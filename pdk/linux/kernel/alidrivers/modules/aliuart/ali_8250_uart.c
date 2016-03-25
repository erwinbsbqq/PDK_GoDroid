/*
 *  Based on 8250_boca.
 *
 *  Data taken from include/asm-i386/serial.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/serial_8250.h>

#include <ali_interrupt.h>

#define ALI_UART1_BASE              0x18018300 /* Memory base for ALI_UART1 */
#define ALI_UART2_BASE              0x18018600 /* Memory base for ALI_UART2 */
#define ALI_UART_REGISTER_SPACE     0x0A

#define ALI_UART1_IRQ               INT_ALI_UART1
#define ALI_UART2_IRQ               INT_ALI_UART2

#define PORT(_base,_irq)			\
	{					        	\
		.iobase		= _base,		\
		.mapbase    = _base,        \
		.irq		= _irq,			\
		.uartclk	= 115200 * 16,	\
		.iotype		= UPIO_MEM,		\
		.flags		= UPF_IOREMAP | UPF_FIXED_TYPE, \
		.type       = PORT_16550A   \
	}

static struct plat_serial8250_port ali_8250_data[] = {
	PORT(ALI_UART1_BASE, ALI_UART1_IRQ),
	PORT(ALI_UART2_BASE, ALI_UART2_IRQ),
	{ },
};

static struct platform_device ali_8250_device = {
	.name		= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data	= ali_8250_data,
	},
};

static int __init ali_8250_init(void)
{
	return platform_device_register(&ali_8250_device);
}

module_init(ali_8250_init);

MODULE_AUTHOR("Corey Chi");
MODULE_DESCRIPTION("8250 serial probe module for ALi IC");
MODULE_LICENSE("GPL");
