/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2013 Copyright (C)
*
*	 Driver for ALi POK
*
*    File:
*
*    Description:
*         
*    History:
*           Date            Athor        Version           Reason
*	    ============	=============	=========	=======================
*	1.	Aug.21.2013       corey@SCT      Ver 0.1      Create file for 3921
*
*  	Copyright 2013 ALi Limited
*  	Copyright (C) 2013 ALi Corp.
*****************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sysrq.h>
#include <linux/device.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/mach-ali/m36_gpio.h>
#if defined(CONFIG_ALI_CHIP_M3921)
#include <ali_interrupt.h>
#else
#include <asm/mach-ali/m36_irq.h>
#endif
#include <ali_reg.h>

#if defined(CONFIG_ALI_CHIP_M3921)
#define PGPIO_CTL_REG        0x1800043C	/* GPIOC */
#define PGPIO_IER_REG        0x18000344
#define PGPIO_REC_REG        0x18000348
#define PGPIO_FEC_REG        0x1800034c
#define PGPIO_DI_REG         0x18000350
#define PGPIO_DO_REG         0x18000354
#define PGPIO_DIR_REG        0x18000358
#define PGPIO_ISR_REG        0x1800035c
#else
#define PGPIO_CTL_REG		0x18000440	/* GPIOD */
#define PGPIO_IER_REG		0x18000444
#define PGPIO_REC_REG		0x18000448
#define PGPIO_FEC_REG		0x1800044c
#define PGPIO_DI_REG		0x18000450
#define PGPIO_DO_REG		0x18000454
#define PGPIO_DIR_REG		0x18000458
#define PGPIO_ISR_REG		0x1800045c

#define GPIO_I_DIR		0
#define GPIO_O_DIR		1
#define GPIO_INT_EN	    0
#define GPIO_INT_DIS	1
#define GPIO_EDG_EN	    1
#define GPIO_EDG_DIS	0
#endif

#if defined(CONFIG_ALI_CHIP_M3921)
#define ALI_POK_IRQ           INT_ALI_GPIO//M36_IRQ_SYSGPIO
#define ALI_POK_ALERT_PIN     30//29
#define ALI_POK_DOWN_PIN      29//30
#define ALI_POK_OFFSET        96//belong to the 3th gpio
#else
#define ALI_POK_IRQ              M36_IRQ_SYSGPIO
#define ALI_POK_ALERT_PIN  29
#define ALI_POK_DOWN_PIN   30
#define ALI_POK_OFFSET			128
#endif

#define ALI_POK_DRIVER_NAME   "ali_pok"


extern void _cpu_suspend(void);


struct ali_pok_port {
    int irq;
	int gpio_nu;
	irqreturn_t (*handler)(int nu, void *param);
}; 

static irqreturn_t ali_pok_alert_interrupt(int irq, void *dev_id)
{	
	if (1 != get_gpio_interrupt_status((((struct ali_pok_port *)dev_id)->gpio_nu)+ALI_POK_OFFSET))
	{
		return IRQ_HANDLED; 
	}

    //printk("pOk...\n");
	__REG8ALI(0x18018300) = '$';

	__REG32ALI(0x18000080) |= (1<<14);  //sflash reset assert
	__REG32ALI(0x18000084) |= (1<<20);  //nand reset assert
	__REG32ALI(0x1802A00F) &= 0xfffffffe;

	_cpu_suspend();
	return IRQ_HANDLED;
}

static irqreturn_t ali_pok_down_interrupt(int irq, void *dev_id)
{	
	if (1 != get_gpio_interrupt_status((((struct ali_pok_port *)dev_id)->gpio_nu)+ALI_POK_OFFSET))
	{
		return IRQ_HANDLED; 
	}
	
	//printk("Pok...\n");
	__REG8ALI(0x18018300) = '~';

	__REG32ALI(0x18000080) |= (1<<14);  //sflash reset assert
	__REG32ALI(0x18000084) |= (1<<20);  //nand reset assert
	__REG32ALI(0x1802A00F) &= 0xfffffffe;

	_cpu_suspend();

	return IRQ_HANDLED;
}

static struct ali_pok_port g_pok_port[2] = {
	                                         {ALI_POK_IRQ, ALI_POK_ALERT_PIN, ali_pok_alert_interrupt},
                                             {ALI_POK_IRQ, ALI_POK_DOWN_PIN, ali_pok_down_interrupt},
                                           };

static int ali_pok_startup(struct ali_pok_port *port)
{
	int retval;
	/*
	 * Allocate the IRQ
	 */
	retval = request_irq(port->irq, port->handler, IRQF_SHARED, ALI_POK_DRIVER_NAME, port);
	if (retval) {
		printk("[ERR] ali_pok: pok_startup - Can't get irq\n");
	}
	return retval;
} 

static void ali_pok_cleanup(void)
{
	int pok_gpio, palert_gpio;
	/* Free IRQ */
	free_irq(g_pok_port[1].irq, &g_pok_port[1]);
	free_irq(g_pok_port[0].irq, &g_pok_port[0]);

	/* Disable interrupts by writing to appropriate	registers */
    pok_gpio = g_pok_port[1].gpio_nu;
	palert_gpio = g_pok_port[0].gpio_nu;
	
	__REG32ALI(PGPIO_CTL_REG) &= ~(1<<pok_gpio);
	__REG32ALI(PGPIO_CTL_REG) &= ~(1<<palert_gpio);
	__REG32ALI(0x18000038) &= ~(1<<0);

#if	defined(CONFIG_MIPS)
	write_c0_status(read_c0_status() & (~STATUSF_IP2));
#endif	
} 

static int ali_init_pok(void)
{
	int ret = 0;
	int pok_gpio = g_pok_port[1].gpio_nu, palert_gpio = g_pok_port[0].gpio_nu;
	
	//enable gpio irq mask
	
#if	defined(CONFIG_MIPS)
	write_c0_status(read_c0_status() | STATUSF_IP2);
#endif	

#if defined(CONFIG_ALI_CHIP_M3921)

#else
	*(volatile long *)(0xb8000040) |= (1<<1);
#endif

	__REG32ALI(PGPIO_CTL_REG) |= ((1<<(pok_gpio&0x1f))|(1<<(palert_gpio&0x1f)));

	__REG32ALI(PGPIO_DIR_REG) &= ~(1 << palert_gpio);  //GPIO BIT DIR SET in
	__REG32ALI(PGPIO_IER_REG) |= (1 << palert_gpio);   //GPIO INT SET enable
	__REG32ALI(PGPIO_FEC_REG) |= (1 << palert_gpio);   //GPIO INT FEDG SET fall
	__REG32ALI(PGPIO_ISR_REG) = (1 << palert_gpio);    //GPIO INT CLEAR
	ret = ali_pok_startup(&g_pok_port[0]);

	__REG32ALI(PGPIO_DIR_REG) &= ~(1 << pok_gpio);
	__REG32ALI(PGPIO_IER_REG) |= (1 << pok_gpio);
	__REG32ALI(PGPIO_FEC_REG) |= (1 << pok_gpio);
	__REG32ALI(PGPIO_ISR_REG) = (1 << pok_gpio);
	ret |= ali_pok_startup(&g_pok_port[1]);

    //enable GPIO irq
	__REG32ALI(0x18000038) |= (1<<0);
	
	return ret;
}

int ali_pok_open(void)
{
	return ali_init_pok();
}
EXPORT_SYMBOL(ali_pok_open);

void ali_pok_close(void)
{
	ali_pok_cleanup();
}
EXPORT_SYMBOL(ali_pok_close);


static int __init ali_pok_init(void)
{
	return ali_init_pok();
}

static void __exit ali_pok_exit(void)
{
	ali_pok_cleanup();
}

module_init(ali_pok_init);
module_exit(ali_pok_exit);

MODULE_AUTHOR("ALi Corp SCT: corey");
MODULE_DESCRIPTION("ALi STB POK driver");
MODULE_LICENSE("GPL");

