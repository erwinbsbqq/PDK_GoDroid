/*
 * ali_interrupt.h
 *
 * Copyright (C) 2013 ALi, Inc.
 *
 * Author:
 *	Tony Zhang <tony.zhang@alitech.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __ALI_INT_H
#define __ALI_INT_H

#define    NR_IRQS     1020

#define INT_ALI_GIC_BASE			0

/*Primary Interrupt Controller*/
#define INT_ALI_PRI_BASE			(INT_ALI_GIC_BASE + 32)
#define INT_ALI_GPIO					(INT_ALI_PRI_BASE + 0)
#define INT_ALI_VE						(INT_ALI_PRI_BASE + 1)
#define INT_ALI_DE						(INT_ALI_PRI_BASE + 2)
#define INT_ALI_SCB4					(INT_ALI_PRI_BASE + 3)
#define INT_ALI_MD						(INT_ALI_PRI_BASE + 4)
#define INT_ALI_AUDIO					(INT_ALI_PRI_BASE + 5)
#define INT_ALI_TSG						(INT_ALI_PRI_BASE + 6)
#define INT_ALI_DEMUX1				(INT_ALI_PRI_BASE + 7)
#define INT_ALI_DEMUX2				(INT_ALI_PRI_BASE + 8)
#define INT_ALI_TS_CTRL				(INT_ALI_PRI_BASE + 9)
#define INT_ALI_SDIO					(INT_ALI_PRI_BASE + 10)
#define INT_ALI_SGDMA2				(INT_ALI_PRI_BASE + 11)
#define INT_ALI_SGDMA1				(INT_ALI_PRI_BASE + 12)
#define INT_ALI_HDMI					(INT_ALI_PRI_BASE + 13)
#define INT_ALI_BDMA					(INT_ALI_PRI_BASE + 14)
#define INT_ALI_PPV						(INT_ALI_PRI_BASE + 15)
#define INT_ALI_UART1					(INT_ALI_PRI_BASE + 16)
#define INT_ALI_UART2					(INT_ALI_PRI_BASE + 17)
#define INT_ALI_SCB1					(INT_ALI_PRI_BASE + 18)
#define INT_ALI_IRC						(INT_ALI_PRI_BASE + 19)
#define INT_ALI_SCR1					(INT_ALI_PRI_BASE + 20)
#define INT_ALI_SCR2					(INT_ALI_PRI_BASE + 21)
#define INT_ALI_RTC						(INT_ALI_PRI_BASE + 22)
#define INT_ALI_WDT						(INT_ALI_PRI_BASE + 23)
#define INT_ALI_VENC					(INT_ALI_PRI_BASE + 24)
#define INT_ALI_SCB2					(INT_ALI_PRI_BASE + 25)
#define INT_ALI_SCB3					(INT_ALI_PRI_BASE + 26)
#define INT_ALI_DRAM					(INT_ALI_PRI_BASE + 27)
#define INT_ALI_PANELSCAN			(INT_ALI_PRI_BASE + 28)
#define INT_ALI_USBDEVICE			(INT_ALI_PRI_BASE + 29)
#define INT_ALI_CPU_REQ_TIME_OUT				(INT_ALI_PRI_BASE + 30)
#define INT_ALI_CPU_ADDR_ERR						(INT_ALI_PRI_BASE + 31)

/*Secondary Interrupt Controller*/
#define INT_ALI_SEC_BASE			(INT_ALI_PRI_BASE + 32)
//#define INT_ALI_TOUCH_CNVDONE	(INT_ALI_SEC_BASE + 0)
//#define INT_ALI_GPU						(INT_ALI_SEC_BASE + 1)
//#define INT_ALI_TOUCH_DETECT	(INT_ALI_SEC_BASE + 2)
#define INT_ALI_VIN0                (INT_ALI_SEC_BASE + 0)
#define INT_ALI_DEMUX4              (INT_ALI_SEC_BASE + 2)
#define INT_ALI_FLASH_DMA			(INT_ALI_SEC_BASE + 3)
#define INT_ALI_USB_EHCI			(INT_ALI_SEC_BASE + 4)
#define INT_ALI_RMII					(INT_ALI_SEC_BASE + 5)
#define INT_ALI_MACINT				(INT_ALI_SEC_BASE + 6)
#define INT_ALI_MACWOL				(INT_ALI_SEC_BASE + 7)
#define INT_ALI_OTP_CTRL			(INT_ALI_SEC_BASE + 8)
#define INT_ALI_GPU_IRQPMU		(INT_ALI_SEC_BASE + 9)
#define INT_ALI_GPU_IRQGPMMU	(INT_ALI_SEC_BASE + 10)
#define INT_ALI_GPU_IRQGP			(INT_ALI_SEC_BASE + 11)
#define INT_ALI_DESCRAMB_INT	(INT_ALI_SEC_BASE + 12)
#define INT_ALI_IRX 					(INT_ALI_SEC_BASE + 13)
#define INT_ALI_TCF						(INT_ALI_SEC_BASE + 14)
#define INT_ALI_SEMAPHORE			(INT_ALI_SEC_BASE + 15)
#define INT_ALI_NFLASH				(INT_ALI_SEC_BASE + 16)
#define INT_ALI_JPG						(INT_ALI_SEC_BASE + 17)
#define INT_ALI_USB_OHCI0			(INT_ALI_SEC_BASE + 18)
#define INT_ALI_QAM						(INT_ALI_SEC_BASE + 19)
#define INT_ALI_PMU_KEY_PRES	(INT_ALI_SEC_BASE + 20)
#define INT_ALI_PMU_RTC				(INT_ALI_SEC_BASE + 21)
#define INT_ALI_SEE_REQS_INT	(INT_ALI_SEC_BASE + 22)
#define INT_ALI_SEE_BUS_ERROR_INT		(INT_ALI_SEC_BASE + 23)
#define INT_ALI_MBX3					(INT_ALI_SEC_BASE + 24)
#define INT_ALI_MBX2					(INT_ALI_SEC_BASE + 25)
#define INT_ALI_MBX1					(INT_ALI_SEC_BASE + 26)
#define INT_ALI_MBX0					(INT_ALI_SEC_BASE + 27)

/*Tertiary Interrupt Controller*/
#define INT_ALI_TRI_BASE			(INT_ALI_SEC_BASE + 32)
#define INT_ALI_SEMA_N1_INT		    (INT_ALI_TRI_BASE + 0)
#define INT_ALI_MCU_INT				(INT_ALI_TRI_BASE + 2)
#define INT_ALI_SWITCH_WOL		    (INT_ALI_TRI_BASE + 3)
#define INT_ALI_SWITCH_INT		(INT_ALI_TRI_BASE + 4)
#define INT_ALI_DEMUX3				(INT_ALI_TRI_BASE + 5)
#define INT_ALI_USB1_OHCI0		(INT_ALI_TRI_BASE + 6)
#define INT_ALI_USB1_EHCI_INTJ		(INT_ALI_TRI_BASE + 7)
#define INT_ALI_IRQPPMMU1			(INT_ALI_TRI_BASE + 8)
#define INT_ALI_IRQPP1				(INT_ALI_TRI_BASE + 9)
#define INT_ALI_IRQPPMMU0			(INT_ALI_TRI_BASE + 10)
#define INT_ALI_IRQPP0				(INT_ALI_TRI_BASE + 11)
#define INT_ALI_IRQPMU				(INT_ALI_TRI_BASE + 12)
#define INT_ALI_IRQGPMMU			(INT_ALI_TRI_BASE + 13)
#define INT_ALI_IRQGP					(INT_ALI_TRI_BASE + 14)


//#define NR_IRQS								(INT_ALI_TRI_BASE)

#define A9_MPCORE_GIC_CPU	(0x1BF00100)
#define A9_MPCORE_GIC_DIST	(0x1BF01000)


#define NULL    ((void *)0)

/****************************************************************************/
struct irq_desc;

typedef	void (*irq_flow_handler_t)(unsigned int irq,
					    struct irq_desc *desc);

enum irqreturn {
	IRQ_NONE		= (0 << 0),
	IRQ_HANDLED		= (1 << 0),
	IRQ_WAKE_THREAD	= (1 << 1),
};

typedef enum irqreturn irqreturn_t;

typedef  irqreturn_t  (*irq_handler_t)(int, void *);

struct irq_data;
struct irq_chip {
	const char  *name;
	void  (*irq_ack)(struct irq_data *data);    /*中断清除函数*/
	void  (*irq_mask)(struct irq_data *data);   /*中断屏蔽函数*/
	void  (*irq_unmask)(struct irq_data *data); /*取消中断屏蔽函数*/
	void  (*irq_eoi)(struct irq_data *data);     /*中断结束函数*/
	int	  (*irq_set_type)(struct irq_data *data, unsigned int flow_type); /*中断类型设置函数*/
	void  (*irq_enable)(struct irq_data *data);
	void  (*irq_disable)(struct irq_data *data);
};

struct irq_data {
	unsigned int		irq;  /*中断号*/
	struct irq_chip		*chip; /*低层次的硬件操作*/
	void			    *chip_data;
};

struct irqaction {
	irq_handler_t		handler;    /*存放用户注册的中断处理函数*/
	unsigned long		flags;      /*flag*/
	void			    *dev_id;   /*传给中断处理函数的指针参数*/
	int			        irq;        /*中断号*/
	const char		    *name;     /*request_irq穿过来的name*/
};


struct irq_desc {
	struct irq_data		  irq_data;
	irq_flow_handler_t	  handle_irq;   /*高层次的中断处理函数*/	
    struct irqaction      *action;	    /*描述用户注册的中断处理函数 */
	unsigned  int		  irq_count;    /*中断发生的次数*/
	const     char		  *name;        /*用户注册时传递来的name*/
};

struct gic_chip_data {
	unsigned long dist_base;
	unsigned long cpu_base;
	unsigned int  gic_irqs;
};

extern struct irq_desc *irq_to_desc(unsigned int irq);

int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	    const char *name, void *dev);


/****************************************************************************/


#endif

