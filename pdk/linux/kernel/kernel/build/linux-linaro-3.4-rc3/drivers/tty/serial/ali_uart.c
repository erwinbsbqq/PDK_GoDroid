/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2003 Copyright (C)
*
*	 Driver for ALi serial ports
*
*    File:    linux/drivers/serial/ali_uart.c
*
*    Description:    This file contains all globe micros and functions declare
*		             of 16550 UART.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Sep.1.2009     Taipei SDK Team  Ver 0.1    Create file.
*
*  	Copyright 2009 ALi Limited
*  	Copyright (C) 2009 ALi Corp.
*****************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_reg.h>
#include <linux/serial_core.h>
#include <linux/serial.h>

#include <linux/delay.h>
#include <linux/clk.h>

#include <asm/irq.h>
#include <asm/io.h>

#include <mach/ali-s3921.h>
//#include <mach/m36_irq.h>
#include <ali_reg.h>
#include <ali_interrupt.h>

//////////////////////////////////////////////////////////////////////////////////////////
/* Parity setting */
#define SCI_PARITY_NONE				0x0000
#define SCI_PARITY_EVEN				0x0001
#define SCI_PARITY_ODD				0x0002
                            		
/* Word length setting */   		
#define SCI_WORDLEN_8				0x0000
#define SCI_WORDLEN_7				0x0010
#define SCI_WORDLEN_6				0x0020
#define SCI_WORDLEN_5				0x0030
                            		
/* Stop bit setting */      		
#define SCI_STOPBIT_1				0x0000
#define SCI_STOPBIT_2				0x0100

/* Device ID */
#define SCI_FOR_RS232				0
#define SCI_FOR_MDM					1

#define SCI_16550_URBR				0
#define SCI_16550_UTBR				0
#define SCI_16550_UIER				1
#define SCI_16550_UIIR				2
#define SCI_16550_UFCR				2
#define SCI_16550_UDLL				0
#define SCI_16550_UDLM				1
#define SCI_16550_ULCR				3
#define SCI_16550_UMCR				4
#define SCI_16550_ULSR				5
#define SCI_16550_UMSR				6
#define SCI_16550_USCR				7
#define SCI_16550_DEVC				8
#define SCI_16550_RCVP				9
#define SCI_16550UART_RX_BUF_SIZE	256

/* For M3327/M3327C serial chip */
#define SCI_16550_NUM				2

#define UINT8	unsigned char;
#define UINT32	unsigned int;

#define PORT_ALI_UART				18 //<-- define in kernel\include\linux\serial_core.h

#define ALI_UART1_BASE    			0x18018300 /* Memory base for ALI_UART1 */
#define ALI_UART2_BASE    			0x18018600 /* Memory base for ALI_UART2 */
#define ALI_UART_REGISTER_SPACE  	0x0A


//#define ALI_UART1_IRQ				24
//#define ALI_UART2_IRQ				25

//#define ALI_UART1_IRQ				M36_IRQ_UART1
//#define ALI_UART2_IRQ				M36_IRQ_UART2

#define ALI_UART1_IRQ				INT_ALI_UART1
#define ALI_UART2_IRQ				INT_ALI_UART2
#define ALI_UART_FIFO_SIZE			16

static struct
{
	unsigned int reg_base;
	int    irq;
	unsigned int strap_ctrl;
} sci_16550_reg[SCI_16550_NUM] = {{ALI_UART1_BASE, ALI_UART1_IRQ, 0}, {ALI_UART2_BASE, ALI_UART2_IRQ, 0}};

#define SCI_READ8(port, reg)			(__REG8ALI((port)->membase + reg))
#define SCI_WRITE8(port, reg, data)		((__REG8ALI((port)->membase + reg)) = (data))

//////////////////////////////////////////////////////////////////////////////////////////

/* Use device name ttyS, major 4, minor 64-68.  This is the usual serial port
 * name, but it is legally reserved for the 8250 driver. 						*/
#define ALI_UART_MAJOR				4 		/* or Low-density serial ports (alternate device) */
#define MINOR_START					64		// 64-68
#define ALI_UART_DRIVER_NAME 		"ali_uart"
#define ALI_UART_DEVICE_NAME		"ttyS"
#define ALI_UART_NR					SCI_16550_NUM


#ifdef CONFIG_ALI_UART_IO
extern char uart_buffer[];
extern unsigned int uart_header;
extern unsigned int uart_tail;
extern unsigned int uart_sci_mode;
#endif


/******************************************
 * Port information
 ******************************************/
struct ali_uart_port {
	struct uart_port	uart;		/* uart */
	unsigned int		strap_ctrl;
	unsigned char  		rx_buf[SCI_16550UART_RX_BUF_SIZE];
	unsigned int 		rx_buf_head;
	unsigned int 		rx_buf_tail;
	unsigned char  		loopback_flag;
};
static struct ali_uart_port ali_uart_ports[ALI_UART_NR];


/*	ali_uart_interrupt
 	UART Interrupt Handler
 */
static irqreturn_t ali_uart_interrupt(int irq, void *dev_id)
{	
	struct ali_uart_port *ali_port = dev_id;
	struct uart_port *port = &ali_port->uart;
	struct tty_struct *tty = ali_port->uart.state->port.tty;
	unsigned char interrupt_source, line_status;	
	char flag = TTY_NORMAL;
    char ch = 0;
	
	/* It seems not need read UIIR but LSR for interrupt processing, but need
	 * to read UIIR for clear interrupt. If ULSR error ocured, read ULSR and
	 * clear it. */
	while (((interrupt_source = (SCI_READ8(port, SCI_16550_UIIR) & 0x0f)) & 1) == 0)
	{
		switch (interrupt_source)
		{
			case 0x06:	/* LSR error: OE, PE, FE, or BI */
				line_status = SCI_READ8(port, SCI_16550_ULSR);
				if (line_status & 0x9e)
				{
					
					//printk("sci_16550uart_interrupt: lstatus error!\n");
					/* Normal, overrun, parity, frame error? */					
					if( line_status & 0x10 )
						flag |= TTY_BREAK;					
					if( line_status & 0x08 )
						flag |= TTY_FRAME;					
					if( line_status & 0x04 )
						flag |= TTY_PARITY;
					if( line_status & 0x02 )	
						flag |= TTY_OVERRUN;
				}
				/* We continue receive data at this condition */
			case 0x0c:	/* Character Timer-outIndication */
			case 0x04:	/* Received Data Available */
			//	printk("read char\n");
				while (SCI_READ8(port, SCI_16550_ULSR) & 1)
				{
					/* Read data & Dispatch to the tty layer */
					ch = SCI_READ8(port, SCI_16550_URBR);
					//tty_insert_flip_char(tty, SCI_READ8(port, SCI_16550_URBR), flag);
#ifdef CONFIG_ALI_UART_IO
					if (!uart_sci_mode)
					{
						tty_insert_flip_char(tty, ch, flag);										
					}
					else
					{
						uart_buffer[uart_header++] = ch;
						uart_header %= SCI_16550UART_RX_BUF_SIZE;
					}
					//sci_16550[id].rx_buf[sci_16550[id].rx_buf_head++] = SCI_READ8(port->line, SCI_16550_URBR);
					//sci_16550[id].rx_buf_head %= SCI_16550UART_RX_BUF_SIZE;
#else
					tty_insert_flip_char(tty, ch, flag);
#endif
				}
				break;
			case 0x02:	/* TransmitterHoldingRegister Empty */
			case 0x00:	/* Modem Status */
			default:
				break;
		}
	}
	tty_flip_buffer_push(tty);
	
	return IRQ_HANDLED;
}


/*	ali_uart_write_char
 */
void ali_uart_write_char(struct uart_port *port, unsigned char ch)
{
#if 0
	/* Wait Tx empty */
	while( SCI_READ8(port, SCI_16550_ULSR) & 0x20);
	
	/* Write a character to the ALI_UART port */
	SCI_WRITE8(port, SCI_16550_UTBR, ch);
#endif	
    /* Wait Tx empty */
    unsigned char status;
    while (1)
    {
        status = SCI_READ8(port, SCI_16550_ULSR);
        if (status & 0x40)
            break;
    }

    /* Send character. */
    SCI_WRITE8(port, SCI_16550_UTBR, ch);

    return;
}


/*	ali_uart_tx_empty 
	This function tests whether the transmitter fifo and shifter for the port described by 'port' is empty.
	If it is empty,	this function should return TIOCSER_TEMT, otherwise return 0.
	If the port does not support this operation, then it should	return TIOCSER_TEMT.
	
	Locking: none.
	Interrupts: caller dependent.
	This call must not sleep	
*/
static unsigned int ali_uart_tx_empty(struct uart_port *port)
{
	return (SCI_READ8(port, SCI_16550_ULSR) & 0x20) ? TIOCSER_TEMT : 0;			
}


/*	ali_uart_set_mctrl
	This function sets the modem control lines for port described by 'port' to the state described by mctrl.  
	The relevant bits of mctrl are:
		- TIOCM_RTS	RTS signal.
		- TIOCM_DTR	DTR signal.
		- TIOCM_OUT1	OUT1 signal.
		- TIOCM_OUT2	OUT2 signal.
		- TIOCM_LOOP	Set the port into loopback mode.
	If the appropriate bit is set, the signal should be driven active.  
	If the bit is clear, the signal should be driven inactive.

	Locking: port->lock taken.
	Interrupts: locally disabled.
	This call must not sleep
 */	
static void ali_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{	

}	


/*	ali_uart_get_mctrl
	Returns the current state of modem control inputs.  The state of the outputs should not be returned, 
	since the core keeps track of their state.  The state information should include:
		- TIOCM_DCD	state of DCD signal
		- TIOCM_CTS	state of CTS signal
		- TIOCM_DSR	state of DS  R signal
		- TIOCM_RI	state of RI signal
	The bit is set if the signal is currently driven active.  
	If the port does not support CTS, DCD or DSR, the driver should	indicate that the signal is permanently active.
	If RI is not available, the signal should not be indicated as active.

	Locking: port->lock taken.
	Interrupts: locally disabled.
	This call must not sleep 
 */
static unsigned int ali_uart_get_mctrl(struct uart_port *port)
{
#if 0
	unsigned char status;
	unsigned int ret = 0;
	
	status = SCI_READ8(port, SCI_16550_UMSR);
	
	if (status & 0x80)	// DCD#
		ret |= TIOCM_CAR;
	if (status & 0x40)	// RI#
		ret |= TIOCM_RNG;
	if (status & 0x20)	// DSR#
		ret |= TIOCM_DSR;
	if (status & 0x10)
		ret |= TIOCM_CTS;
	return ret;
#endif
	return 0;
}	


/*	ali_uart_stop_tx
	Stop transmitting characters.  This might be due to the CTS
	line becoming inactive or the tty layer indicating we want
	to stop transmission due to an XOFF character.
	The driver should stop transmitting characters as soon as
	possible.
	
	Locking: port->lock taken.
	Interrupts: locally disabled.
	This call must not sleep	
 */
static void ali_uart_stop_tx(struct uart_port *port)
{
}


/*	ali_uart_start_tx
	Start transmitting characters.

	Locking: port->lock taken.
	Interrupts: locally disabled.
	This call must not sleep
 */
static void ali_uart_start_tx(struct uart_port *port)
{
#ifdef CONFIG_ALI_DISABLE_PRINTF
    return;
#endif


#ifdef CONFIG_ALI_UART_IO
	if (uart_sci_mode)
	{
		printk("ali_uart_start_tx,%d,uart_sci_mode=1,port=%d,return\n", __LINE__,port == &ali_uart_ports[1].uart?1:0);
		return;
	}
#endif
	
	while (1) {
		/* Get the data from the UART circular buffer and
			write it to the USB_UART's WRITE_DATA register */

		ali_uart_write_char(port, port->state->xmit.buf[port->state->xmit.tail]);		
		
		/* Adjust the tail of the UART buffer */
    	port->state->xmit.tail = (port->state->xmit.tail + 1) &
                            (UART_XMIT_SIZE - 1);
		/* Statistics */
    	port->icount.tx++;
    	
		/* Finish if no more data available in the UART buffer */
    	if (uart_circ_empty(&port->state->xmit)) break;
    }
    /* ... */	   
	
}


/*	ali_uart_stop_rx
	Stop receiving characters; the port is in the process of
	being closed.

	Locking: port->lock taken.
	Interrupts: locally disabled.
	This call must not sleep
 */
static void ali_uart_stop_rx(struct uart_port *port)
{
}


/*	ali_uart_enable_ms
	Enable the modem status interrupts.

	This method may be called multiple times.  Modem status
	interrupts should be disabled when the shutdown method is
	called.

	Locking: port->lock taken.
	Interrupts: locally disabled.
	This call must not sleep
 */	
static void ali_uart_enable_ms(struct uart_port *port)
{
}


/*	ali_uart_break_ctl
	Control the transmission of a break signal.  If ctl is
	nonzero, the break signal should be transmitted.  The signal
	should be terminated when another call is made with a zero
	ctl.

	Locking: none.
	Interrupts: caller dependent.
	This call must not sleep
 */
static void ali_uart_break_ctl(struct uart_port *port, int break_state)
{
}


/*	ali_uart_startup
	Grab any interrupt resources and initialise any low level driver
	state.  Enable the port for reception.  It should not activate
	RTS nor DTR; this will be done via a separate call to set_mctrl.

	This method will only be called when the port is initially opened.

	Locking: port_sem taken.
	Interrupts: globally disabled. 
 */
static int ali_uart_startup(struct uart_port *port)
{
	struct tty_struct *tty = port->state->port.tty;
	int retval;
	
	/*
	 * Allocate the IRQ
	 */
	 
	retval = request_irq(port->irq, ali_uart_interrupt, 0 /*IRQF_SHARED*/, tty ? tty->name : ALI_UART_DRIVER_NAME, port);
	if (retval) {
		printk("ali_uart: atmel_startup - Can't get irq\n");
		return retval;
	}
	return retval;
} 


/*	ali_uart_shutdown
	Disable the port, disable any break condition that may be in
	effect, and free any interrupt resources.  It should not disable
	RTS nor DTR; this will have already been done via a separate
	call to set_mctrl.

	Drivers must not access port->state once this call has completed.

	This method will only be called when there are no more users of
	this port.

	Locking: port_sem taken.
	Interrupts: caller dependent.
 */
static void ali_uart_shutdown(struct uart_port *port)
{
	/* ... */
	/* Free IRQ */
	free_irq(port->irq, port);

	/* Disable interrupts by writing to appropriate	registers */
	/* ... */	
} 


/*	ali_uart_flush_buffer
	Flush any write buffers, reset any DMA state and stop any
	ongoing DMA transfers.

	This will be called whenever the port->state->xmit circular
	buffer is cleared.

	Locking: port->lock taken.
	Interrupts: locally disabled.
	This call must not sleep
  */
static void ali_uart_flush_buffer(struct uart_port *port)
{
	
} 


/*	ali_uart_set_termios
	Change the port parameters, including word length, parity, stop
	bits.  Update read_status_mask and ignore_status_mask to indicate
	the types of events we are interested in receiving.  Relevant
	termios->c_cflag bits are:
		CSIZE	- word size
		CSTOPB	- 2 stop bits
		PARENB	- parity enable
		PARODD	- odd parity (when PARENB is in force)
		CREAD	- enable reception of characters (if not set,
	still receive characters from the port, but throw them away.
		CRTSCTS	- if set, enable CTS status change reporting
		CLOCAL	- if not set, enable modem status change reporting.
	Relevant termios->c_iflag bits are:
		INPCK	- enable frame and parity error events to be passed to the TTY layer.
		BRKINT
		PARMRK	- both of these enable break events to be passed to the TTY layer.
		IGNPAR	- ignore parity and framing errors
		IGNBRK	- ignore break errors,  If IGNPAR is also set, ignore overrun errors as well.
	The interaction of the iflag bits is as follows (parity error given as an example):
	Parity error	INPCK	IGNPAR
	n/a		0		n/a		character received, marked as TTY_NORMAL
	None			1		n/a	character received, marked as TTY_NORMAL
	Yes		1		0		character received, marked as TTY_PARITY
	Yes		1		1	character discarded

	Other flags may be used (eg, xon/xoff characters) if your hardware supports hardware "soft" flow control.

	Locking: none.
	Interrupts: caller dependent.
	This call must not sleep
 */
static void ali_uart_set_termios(struct uart_port *port, struct ktermios *termios, struct ktermios *old)
{
#if 1	/* init at ali_init_port() */
	unsigned char ulcr = 0;
	unsigned long flags;	
	unsigned int baud, div;	

	/* Word Length Select */
	switch (termios->c_cflag & CSIZE) {
	case CS5:
		ulcr = (ulcr & (~0x03)) | 0x00;	// 5 bits
		break;
	case CS6:
		ulcr = (ulcr & (~0x03)) | 0x01;	// 6 bits
		break;
	case CS7:
		ulcr = (ulcr & (~0x03)) | 0x02;	// 7 bits
		break;
	case CS8:		
	default:
		ulcr = (ulcr & (~0x03)) | 0x03;	// 8 bits
		break;
	}

	/* Stop Bit Select */
	if (termios->c_cflag & CSTOPB)
		ulcr = (ulcr & (~0x04)) | 0x04;
		
	/* Parity Enable */	
	if (termios->c_cflag & PARENB) {
		ulcr = (ulcr & (~0x08)) | 0x08; // Enable Parity Bit		
		if (termios->c_cflag & PARODD)
			ulcr = (ulcr & (~0x10)) | 0x00; // Odd Parity Bit
		else
			ulcr = (ulcr & (~0x10)) | 0x10; // Even Parity Bit
	} else
		ulcr = (ulcr & (~0x08)) | 0x00; // Non Parity Bit
		 
	/* Ask the core to calculate the divisor for us. */
	//baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk/16);
	baud = uart_get_baud_rate(port, termios, old, 0, 921600);
	if (0 == baud)
	{
		baud = 115200;
	}
	
	spin_lock_irqsave(&port->lock, flags);	
	
	/* Disable all interrupt */
	SCI_WRITE8(port, SCI_16550_UIER, 0);

	if (baud > 115200)
		SCI_WRITE8(port, SCI_16550_DEVC, SCI_READ8(port, SCI_16550_DEVC) | 0x08);			
	
	div = 115200 / baud;
	SCI_WRITE8(port, SCI_16550_ULCR, ulcr|0x80);		/* Enable setup baud rate */
	SCI_WRITE8(port, SCI_16550_UDLL, (div & 0xff));
	SCI_WRITE8(port, SCI_16550_UDLM, ((div >> 8) & 0xff));
	SCI_WRITE8(port, SCI_16550_ULCR, ulcr&(~0x80));	/* Disable setup baud rate */
	
	/* Enable FIFO, threshold is 4 bytes */
	SCI_WRITE8(port, SCI_16550_UFCR, 0x47);		/* 0x00: no FIFO, 0x47:Reset FIFO */
	SCI_WRITE8(port, SCI_16550_ULSR, 0x00);		/* Reset line status */
	SCI_WRITE8(port, SCI_16550_UMCR, 0x03);		/* Set modem control */

	/* Enable receiver interrupt */
	SCI_WRITE8(port, SCI_16550_UIER, 0x05);		/* Enable RX & timeout interrupt */

	/* signon message or measure TIMEOUT */	
	spin_unlock_irqrestore(&port->lock, flags);	
	
	if (tty_termios_baud_rate(termios))
		tty_termios_encode_baud_rate(termios, baud, baud);
#endif
}  


/*	ali_uart_type
	Return a pointer to a string constant describing the specified
	port, or return NULL, in which case the string 'unknown' is
	substituted.

	Locking: none.
	Interrupts: caller dependent.
 */        
static const char *ali_uart_type(struct uart_port *port)
{
	return (port->type == PORT_ALI_UART) ? "ALI_UART" : NULL;	
}


/*	ali_uart_release_port
	Release any memory and IO region resources currently in use by
	the port.

	Locking: none.
	Interrupts: caller dependent.
 */
static void ali_uart_release_port(struct uart_port *port)
{
  	release_mem_region(port->mapbase, ALI_UART_REGISTER_SPACE);	
}  


/*	ali_uart_request_port
	Request any memory and IO region resources required by the port.
	If any fail, no resources should be registered when this function
	returns, and it should return -EBUSY on failure.

	Locking: none.
	Interrupts: caller dependent.
 */
static int ali_uart_request_port(struct uart_port *port)
{
	if (!request_mem_region(port->mapbase, ALI_UART_REGISTER_SPACE,
                          "ali_uart")) {
		return -EBUSY;
	}
	return 0;	
} 


/*	ali_uart_config_port
	Perform any autoconfiguration steps required for the port.  `type`
	contains a bit mask of the required configuration.  UART_CONFIG_TYPE
	indicates that the port requires detection and identification.
	port->type should be set to the type found, or PORT_UNKNOWN if
	no port was detected.

	UART_CONFIG_IRQ indicates autoconfiguration of the interrupt signal,
	which should be probed using standard kernel autoprobing techniques.
	This is not necessary on platforms where ports have interrupts
	internally hard wired (eg, system on a chip implementations).

	Locking: none.
	Interrupts: caller dependent.
 */
static void ali_uart_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE && ali_uart_request_port(port) == 0)
	{
		port->type = PORT_ALI_UART;
	}	
} 


/*	ali_uart_verify_port
	Verify the new serial port information contained within serinfo is
	suitable for this port type.

	Locking: none.
	Interrupts: caller dependent.
 */
static int ali_uart_verify_port(struct uart_port *port, struct serial_struct *serinfo)
{
	return 0;
}  


/*	ali_uart_pm
	Perform any power management related activities on the specified
	port.  State indicates the new state (defined by ACPI D0-D3),
	oldstate indicates the previous state.  Essentially, D0 means
	fully on, D3 means powered down.

	This function should not be used to grab any resources.

	This will be called when the port is initially opened and finally
	closed, except when the port is also the system console.  This
	will occur even if CONFIG_PM is not set.

	Locking: none.
	Interrupts: caller dependent.
 */          
static void ali_uart_pm(struct uart_port *port, unsigned int state, unsigned int oldstate)
{
}  


/******************************************
* The UART operations structure 		  *
 ******************************************/
static struct uart_ops ali_uart_pops = {
	.tx_empty		= ali_uart_tx_empty,		/* Transmitter busy empty */	
	.set_mctrl		= ali_uart_set_mctrl,		/* Set modem control */
	.get_mctrl		= ali_uart_get_mctrl,		/* Get modem control */
	.stop_tx		= ali_uart_stop_tx,     	/* Stop transmission */    
	.start_tx		= ali_uart_start_tx,    	/* Start transmitting */
	.stop_rx		= ali_uart_stop_rx,     	/* Stop reception */
	.enable_ms		= ali_uart_enable_ms,   	/* Enable modem status signals */
	.break_ctl		= ali_uart_break_ctl,		/* Control the transmission of a break signal */ 
	.startup		= ali_uart_startup,     	/* App opens ALI_UART */
	.shutdown		= ali_uart_shutdown,		/* App closes ALI_UART */
	.flush_buffer	= ali_uart_flush_buffer,	/* Flush any write buffers */  
	.set_termios	= ali_uart_set_termios,		/* Set termios */    
	.type			= ali_uart_type,        	/* Set UART type */
	.release_port	= ali_uart_release_port,	/* Release resources associated with a ALI_UART port */ 
	.request_port	= ali_uart_request_port,	/* Claim resources associated with a ALI_UART port */   
	.config_port	= ali_uart_config_port, 	/* Configure when driver adds a ALI_UART port */
	.verify_port	= ali_uart_verify_port,		/* Verify the new serial port information */
	.pm				= ali_uart_pm,				/* Perform any power management relatd activites on the spedified port */
};	


/*	ali_init_port
 	Configure the port from the platform device resource info.
 */
static void __devinit ali_init_port(void)
{
	struct ali_uart_port *up;
	static int first = 1;
	int i;	
	
	if (!first)
		return; // Port has been init
	first = 0;	

	for (i = 0; i < ALI_UART_NR; i++) {
		struct ali_uart_port *ali_port = &ali_uart_ports[i];
		struct uart_port *port = &(ali_port->uart);
		
		spin_lock_init(&ali_port->uart.lock);		
		port->mapbase	= sci_16550_reg[i].reg_base;		/* Memory mapped */ 	
		port->membase	= sci_16550_reg[i].reg_base;		/* Memory mapped */ 		
		port->iotype	= UPIO_MEM;
		port->irq		= sci_16550_reg[i].irq;				/* IRQ */				
		port->fifosize	= ALI_UART_FIFO_SIZE;				/* Size of the FIFO */
		port->ops		= &ali_uart_pops;	
		port->flags		= UPF_BOOT_AUTOCONF;
		port->line		= i;								/* UART port number */
		ali_port->strap_ctrl	= sci_16550_reg[i].strap_ctrl;
		ali_port->rx_buf_head 	= 0;
		ali_port->rx_buf_tail 	= 0;
		ali_port->loopback_flag = 0;
		memset(&ali_port->rx_buf, 0, sizeof(ali_port->rx_buf));
		
//        if (ali_port->strap_ctrl != 0)
//            __REG32ALI(0x18000074) = ali_port->strap_ctrl;

        /* Disable all interrupt */
        SCI_WRITE8(port, SCI_16550_UIER, 0x00);
        /* Enable setup baud rate */
        SCI_WRITE8(port, SCI_16550_ULCR, 0x9b);
        /* set divsor latch */
        SCI_WRITE8(port, SCI_16550_UDLL, 0x01);
        SCI_WRITE8(port, SCI_16550_UDLM, 0x00);

#if 0		
        /* clear divsor latch (SCI_16550_ULCR_Bit7) & set parity */
        if (0 == i)
            SCI_WRITE8(port, SCI_16550_ULCR, 0x03); /* none parity */
        else
            SCI_WRITE8(port, SCI_16550_ULCR, 0x1b); /* even parity */
#else
        SCI_WRITE8(port, SCI_16550_ULCR, 0x1b); /* even parity */
#endif

        SCI_WRITE8(port, SCI_16550_UFCR, 0x47); /* 0x00: no FIFO, 0x47: Reset FIFO */
        SCI_WRITE8(port, SCI_16550_ULSR, 0x00);   /* Reset Line Status */
        SCI_WRITE8(port, SCI_16550_UMCR, 0x03);   /* Set modem control */

        /* Enable receiver interrupt */
        SCI_WRITE8(port, SCI_16550_UIER, 0x05);   /* Enable RX & timeout interrupt */
	}
	

	return;
}


static void ali_console_putchar(struct uart_port *port, int ch)
{
#if 0
	/* Wait Tx empty */
	while( SCI_READ8(port, SCI_16550_ULSR) & 0x20);
	
	/* Write a character to the ALI_UART port */
	SCI_WRITE8(port, SCI_16550_UTBR, ch);
#endif	
    /* Wait Tx empty */
    volatile unsigned char status;
    int retry=3,i;
    while(retry)
    {
        i =10000;
        while(--i)
        {
            status = SCI_READ8(port, SCI_16550_ULSR);
            if (status & 0x40)
                break;
            udelay(10);
        }
        if(0!=i)
        {
            break;
        }
        mdelay(10);
        retry --;
    }
    if(0!=retry)
    {
        /* Send character. */
        SCI_WRITE8(port, SCI_16550_UTBR, ch);
    }
    return;
}


/*	ali_console_write
 */
static void ali_console_write(struct console *co, const char *s, unsigned int count)
{
	struct ali_uart_port *up = &ali_uart_ports[co->index];	

	uart_console_write(&up->uart, s, count, ali_console_putchar);
	
}


/*
 * If the port was already initialised (eg, by a boot loader),
 * try to determine the current setup.
 */
static void __init ali_console_get_options(struct uart_port *port, int *baud,
					     int *parity, int *bits, int *flow)
{
	return ;
}


/*	ali_console_setup
 */
static int __init ali_console_setup(struct console *co, char *options)
{

	struct uart_port *port;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
	
	co->index = 0;
	port = &ali_uart_ports[co->index].uart;
	
	if (port->membase == NULL) {
		/* Port not initialized yet - delay setup */
		return -ENODEV;
	}

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	
	return uart_set_options(port, co, baud, parity, bits, flow);					
}

/*****************************************
 * Console information
 ******************************************/
static struct uart_driver ali_uart_reg;
static struct console ali_console = {
	.name		= ALI_UART_DEVICE_NAME,
	.write		= ali_console_write,
	.device		= uart_console_device,
	.setup		= ali_console_setup,
	//.early_setup= ali_console_early_setup,	
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &ali_uart_reg,
};


/*
 * Early console initialization (before VM subsystem initialized).
 */
static int __init ali_console_init(void)
{
	ali_init_port();
	register_console(&ali_console);
	return 0;
}

console_initcall(ali_console_init);


/*	ali_uart_probe
 */
static int __init ali_uart_probe(struct platform_device *dev)
{

	struct ali_uart_port *port = &ali_uart_ports[0];
	//void *data;
	int ret;
  	
  	/* Initial Port Information */
	ali_init_port();
	
 	/* uart_add_one_port */
	ret = uart_add_one_port(&ali_uart_reg, &port->uart);
	if (ret)
	{
		return ret;	/* add one port fail */
	}

	platform_set_drvdata(dev, port);

	port = &ali_uart_ports[1];

	/* uart_add_one_port */
	ret = uart_add_one_port(&ali_uart_reg, &port->uart);
	if (ret)
	{
		return ret;	/* add one port fail */
	}

	//platform_set_drvdata(dev, port);
	
	return 0;
}


/*	ali_uart_remove
 */
static int __devexit ali_uart_remove(struct platform_device *dev)
{
	
	struct ali_uart_port *port = &ali_uart_ports[dev->id];
	int ret = 0;
	
  	platform_set_drvdata(dev, NULL);

  	/* Remove the ALI_UART port from the serial core */
  	ret = uart_remove_one_port(&ali_uart_reg, &port->uart);
  	//kfree(port->rx_buf.buf);
  		
  	return 0;		
}


//#define ali_uart_suspend 	NULL
//#define ali_uart_resume 	NULL
static int ali_uart_suspend(struct platform_device *pdev,
				pm_message_t state)
{
	return 0;
}


static int ali_uart_resume(struct platform_device *pdev)
{
	return 0;
}


static struct uart_driver ali_uart_reg = {
	.owner		= THIS_MODULE,			/* Owner */
	.driver_name	= ALI_UART_DRIVER_NAME,	/* Driver name */
	.dev_name	= ALI_UART_DEVICE_NAME,	/* Node name */
	.major		= ALI_UART_MAJOR,		/* Major number */
	.minor		= MINOR_START,			/* Minor number start */
	.nr			= ALI_UART_NR,			/* Number of UART ports */
	.cons		= &ali_console,			/* Pointer to the console structure. */
};
static struct platform_driver ali_serial_driver = {
	.probe      = ali_uart_probe,		// Probe method 
	.remove     = ali_uart_remove,		// Detach method 
	.suspend	= ali_uart_suspend,		// Power suspend 
	.resume		= ali_uart_resume,		// Resume after a suspend 
	.driver		= {
		.name	= ALI_UART_DRIVER_NAME,	// Driver name 
		.owner	= THIS_MODULE,
	},
};


/* Driver Initialization */
static int __init ali_uart_init(void)
{
	int retval;
	
	printk(KERN_INFO "Serial: ALi UART driver\n");
	
	/* Register the ALI_UART driver with the serial core */
	if ((retval = uart_register_driver(&ali_uart_reg))) 
		return retval;	

	/* Announce a matching driver for the platform devices registered above */
	if ((retval = platform_driver_register(&ali_serial_driver))) 
		uart_unregister_driver(&ali_uart_reg);
	
	return retval;	

}


/* Driver Exit */
static void __exit ali_uart_exit(void)
{
	/* The order of unregistration is important. Unregistering the
	UART driver before the platform driver will crash the system */
	
	/* Unregister the platform driver */
	platform_driver_unregister(&ali_serial_driver);

	/* Unregister the ALi_UART driver */
	uart_unregister_driver(&ali_uart_reg);
}

module_init(ali_uart_init);
module_exit(ali_uart_exit);

MODULE_AUTHOR("ALi Corp Taipei SDK Team");
MODULE_DESCRIPTION("ALi STB serial port driver");
MODULE_ALIAS("platform:ali_uart");
MODULE_LICENSE("GPL");

/*****************************************************************************
	End of Driver 
******************************************************************************/

