/*
 *  arch/arm/mach-ali3921/include/mach/uncompress.h
 *
 *  Copyright (C) 2003 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include <mach/ali-s3921.h>
#ifndef __MACH_ALI_S3921_UNCOMPRESS_H
#define __MACH_ALI_S3921_UNCOMPRESS_H

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

#define SCI_PARITY_NONE				0x0000
#define SCI_PARITY_EVEN				0x0001
#define SCI_PARITY_ODD				0x0002
                            		
#define SCI_WORDLEN_8				0x0000
#define SCI_WORDLEN_7				0x0010
#define SCI_WORDLEN_6				0x0020
#define SCI_WORDLEN_5				0x0030
#define SCI_STOPBIT_1				0x0000
#define SCI_STOPBIT_2				0x0100

#define SCI_READ8(reg)			(*((volatile u8 *)(0x18018300 + reg)))
#define SCI_WRITE8(reg, data)	(*((volatile u8 *)(0x18018300 + reg)) = (data))

static void sci_16550uart_set_mode(u32 bps, int parity);
static void sci_16550uart_write(u8 ch);

static void sci_16550uart_set_mode(u32 bps, int parity)
{
	unsigned int div;
	SCI_WRITE8(SCI_16550_UIER, 0);
	/* Set baud rate and transmit format */
	if (bps > 2000000)
	{
		SCI_WRITE8(SCI_16550_DEVC, SCI_READ8(SCI_16550_DEVC) | 0x08);
		div = (bps / 90000) * 1843;
	} else if (bps > 115200)
	{
		SCI_WRITE8(SCI_16550_DEVC, SCI_READ8(SCI_16550_DEVC) | 0x08);
		div = (bps / 32000) * 1843;
	} 
	div = 115200 / bps;
	SCI_WRITE8(SCI_16550_ULCR, 0x9b);	/* Enable setup baud rate */
	SCI_WRITE8(SCI_16550_UDLL, (div & 0xff));
	SCI_WRITE8(SCI_16550_UDLM, ((div >> 8) & 0xff));

	div = (((parity >> 6) & 0x04) | ((~(parity >> 4)) & 0x03));
	switch (parity & 0x03)
	{
		case SCI_PARITY_EVEN:
			SCI_WRITE8(SCI_16550_ULCR, 0x18 | div);	/* even parity */
			break;
		case SCI_PARITY_ODD:
			SCI_WRITE8(SCI_16550_ULCR, 0x08 | div);	/* odd parity */
			break;
		default :
			SCI_WRITE8(SCI_16550_ULCR, 0x00 | div);	/* none parity */
			break;
	};

	SCI_WRITE8(SCI_16550_UFCR, 0x41);		/* Reset FIFO */
	SCI_WRITE8(SCI_16550_ULSR, 0x00);		/* Reset line status */
	SCI_WRITE8(SCI_16550_UMCR, 0x03);		/* Set modem control */
}

static void sci_16550uart_write(u8 ch)
{
	int i;
	int n;
	int retry = 3;

	while(retry)
	{
		// Send character. 
		SCI_WRITE8(SCI_16550_UTBR, ch);
		//wait for transmission finished 
		i = 20000;
		n = 2;
		while (--i)
		{
			if (SCI_READ8(SCI_16550_ULSR) & 0x20)
			{
				// When CPU is too fast, UART output will have repeated character,
				// Add a delay is a temporary solution, wait for IC team check it.
				while(--n)
				{
					//soc_printf("delay");				
				}
				break;
			}
		}
		if (0 != i)
			break;
		// Timeout, reset XMIT FIFO 
		SCI_WRITE8(SCI_16550_UFCR, SCI_READ8(SCI_16550_UFCR) | 0x04);
		n = 2;
		while(--n)
		{
			//soc_printf("delay 2");				
		}
		retry --;
	}
	return;
}

/*
 * The following code assumes the serial port has already been
 * initialized by the bootloader.  If you didn't setup a port in
 * your bootloader then nothing will appear (which might be desired).
 *
 * This does not append a newline
 */
static void putc(int c)
{
	sci_16550uart_write(c);
}

static inline void flush(void)
{
	/* nothing */
}

/*
 * Setup before decompression.  This is where we do UART selection for
 * earlyprintk and init the uart_base register.
 */
static inline void arch_decomp_setup(void)
{
	sci_16550uart_set_mode(115200, 1);
}

#define arch_decomp_wdog()

#endif
