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

#ifdef CONFIG_ARM

#define INT_ALI_GIC_BASE			    0

/*Primary Interrupt Controller*/
#define INT_ALI_PRI_BASE			    (INT_ALI_GIC_BASE + 32)
#define INT_ALI_GPIO					(INT_ALI_PRI_BASE + 0)
#define INT_ALI_VE						(INT_ALI_PRI_BASE + 1)
#define INT_ALI_DE						(INT_ALI_PRI_BASE + 2)
#define INT_ALI_SCB4					(INT_ALI_PRI_BASE + 3)
#define INT_ALI_MD						(INT_ALI_PRI_BASE + 4)
#define INT_ALI_AUDIO					(INT_ALI_PRI_BASE + 5)
#define INT_ALI_TSG						(INT_ALI_PRI_BASE + 6)
#define INT_ALI_DEMUX1				    (INT_ALI_PRI_BASE + 7)
#define INT_ALI_DEMUX2				    (INT_ALI_PRI_BASE + 8)
#define INT_ALI_TS_CTRL				    (INT_ALI_PRI_BASE + 9)
#define INT_ALI_SDIO					(INT_ALI_PRI_BASE + 10)
#define INT_ALI_SGDMA2				    (INT_ALI_PRI_BASE + 11)
#define INT_ALI_SGDMA1				    (INT_ALI_PRI_BASE + 12)
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
#define INT_ALI_PANELSCAN			    (INT_ALI_PRI_BASE + 28)
#define INT_ALI_USBDEVICE		    	(INT_ALI_PRI_BASE + 29)
#define INT_ALI_CPU_REQ_TIME_OUT		(INT_ALI_PRI_BASE + 30)
#define INT_ALI_CPU_ADDR_ERR			(INT_ALI_PRI_BASE + 31)

/*Secondary Interrupt Controller*/
#define INT_ALI_SEC_BASE			    (INT_ALI_PRI_BASE + 32)
//#define INT_ALI_TOUCH_CNVDONE	        (INT_ALI_SEC_BASE + 0)
//#define INT_ALI_GPU					(INT_ALI_SEC_BASE + 1)
//#define INT_ALI_TOUCH_DETECT	        (INT_ALI_SEC_BASE + 2)
#define INT_ALI_VIN0                    (INT_ALI_SEC_BASE + 0)
#define INT_ALI_DEMUX4                  (INT_ALI_SEC_BASE + 2)
#define INT_ALI_FLASH_DMA			    (INT_ALI_SEC_BASE + 3)
#define INT_ALI_USB_EHCI			    (INT_ALI_SEC_BASE + 4)
#define INT_ALI_RMII					(INT_ALI_SEC_BASE + 5)
#define INT_ALI_MACINT				    (INT_ALI_SEC_BASE + 6)
#define INT_ALI_MACWOL				    (INT_ALI_SEC_BASE + 7)
#define INT_ALI_OTP_CTRL			    (INT_ALI_SEC_BASE + 8)
#define INT_ALI_GPU_IRQPMU		        (INT_ALI_SEC_BASE + 9)
#define INT_ALI_GPU_IRQGPMMU	        (INT_ALI_SEC_BASE + 10)
#define INT_ALI_GPU_IRQGP			    (INT_ALI_SEC_BASE + 11)
#define INT_ALI_DESCRAMB_INT	        (INT_ALI_SEC_BASE + 12)
#define INT_ALI_IRX 					(INT_ALI_SEC_BASE + 13)
#define INT_ALI_TCF						(INT_ALI_SEC_BASE + 14)
#define INT_ALI_SEMAPHORE		    	(INT_ALI_SEC_BASE + 15)
#define INT_ALI_NFLASH				    (INT_ALI_SEC_BASE + 16)
#define INT_ALI_JPG						(INT_ALI_SEC_BASE + 17)
#define INT_ALI_USB_OHCI0			    (INT_ALI_SEC_BASE + 18)
#define INT_ALI_QAM						(INT_ALI_SEC_BASE + 19)
#define INT_ALI_PMU_KEY_PRES	        (INT_ALI_SEC_BASE + 20)
#define INT_ALI_PMU_RTC				    (INT_ALI_SEC_BASE + 21)
#define INT_ALI_SEE_REQS_INT	        (INT_ALI_SEC_BASE + 22)
#define INT_ALI_SEE_BUS_ERROR_INT		(INT_ALI_SEC_BASE + 23)
#define INT_ALI_MBX3					(INT_ALI_SEC_BASE + 24)
#define INT_ALI_MBX2					(INT_ALI_SEC_BASE + 25)
#define INT_ALI_MBX1					(INT_ALI_SEC_BASE + 26)
#define INT_ALI_MBX0					(INT_ALI_SEC_BASE + 27)

/*Tertiary Interrupt Controller*/
#define INT_ALI_TRI_BASE			    (INT_ALI_SEC_BASE + 32)
#define INT_ALI_SEMA_N1_INT		        (INT_ALI_TRI_BASE + 0)
#define INT_ALI_MCU_INT				    (INT_ALI_TRI_BASE + 2)
#define INT_ALI_SWITCH_WOL		        (INT_ALI_TRI_BASE + 3)
#define INT_ALI_SWITCH_INT		        (INT_ALI_TRI_BASE + 4)
#define INT_ALI_DEMUX3				    (INT_ALI_TRI_BASE + 5)
#define INT_ALI_USB1_OHCI0		        (INT_ALI_TRI_BASE + 6)
#define INT_ALI_USB1_EHCI_INTJ		    (INT_ALI_TRI_BASE + 7)
#define INT_ALI_IRQPPMMU1			    (INT_ALI_TRI_BASE + 8)
#define INT_ALI_IRQPP1			    	(INT_ALI_TRI_BASE + 9)
#define INT_ALI_IRQPPMMU0		    	(INT_ALI_TRI_BASE + 10)
#define INT_ALI_IRQPP0			    	(INT_ALI_TRI_BASE + 11)
#define INT_ALI_IRQPMU				    (INT_ALI_TRI_BASE + 12)
#define INT_ALI_IRQGPMMU			    (INT_ALI_TRI_BASE + 13)
#define INT_ALI_IRQGP					(INT_ALI_TRI_BASE + 14)
/*New added interrupt for C3921*/
#define INT_ALI_RTC_TM0					(INT_ALI_TRI_BASE + 16)
#define INT_ALI_RTC_TM1					(INT_ALI_TRI_BASE + 17)
#define INT_ALI_RTC_TM2					(INT_ALI_TRI_BASE + 18)
#define INT_ALI_RTC_TM3					(INT_ALI_TRI_BASE + 19)
#define INT_ALI_RTC_TM4					(INT_ALI_RTC)  //for compatible with S3921 IC 
#define INT_ALI_RTC_TM5					(INT_ALI_TRI_BASE + 21)
#define INT_ALI_RTC_TM6					(INT_ALI_TRI_BASE + 22)
#define INT_ALI_RTC_TM7					(INT_ALI_TRI_BASE + 23)

//#define NR_IRQS						(INT_ALI_TRI_BASE)

#else                                   //for CONFIG_MIPS
/* decide the irq block assignment */
#define	M36_NUM_CPU_IRQ         8
#ifdef	CONFIG_M6304
#define	M36_NUM_SYS_IRQ	        20
#define	M36_NUM_GPIO_IRQ	    20
#elif defined CONFIG_M6303
#define	M36_NUM_SYS_IRQ	        64    /* Now Total 34 */
#define	M36_NUM_GPIO_IRQ	    32
#endif

#define	M36_IRQ_BASE		     0

#define	M36_CPU_IRQ_BASE	     M36_IRQ_BASE
#define	M36_SYS_IRQ_BASE	     (M36_CPU_IRQ_BASE + M36_NUM_CPU_IRQ)
#define	M36_GPIO_IRQ_BASE	     (M36_SYS_IRQ_BASE + M36_NUM_SYS_IRQ)

/* CPU interrupts */

/* 
   IP0 - Software interrupt 
   IP1 - Software interrupt 
   IP2 - All but battery, high speed modem, and real time clock 
   IP3 - RTC Long1 (system timer) 
   IP4 - RTC Long2 
   IP5 - High Speed Modem (unused on M3602) 
   IP6 - Unused 
   IP7 - Timer interrupt from CPO_COMPARE 
*/
#define M36_IRQ_SW1             (M36_CPU_IRQ_BASE + 0)  
#define M36_IRQ_SW2             (M36_CPU_IRQ_BASE + 1)  
#define M36_IRQ_INT0            (M36_CPU_IRQ_BASE + 2)
#define M36_IRQ_INT1            (M36_CPU_IRQ_BASE + 3)
#define M36_IRQ_INT2            (M36_CPU_IRQ_BASE + 4)
#define M36_IRQ_INT3            (M36_CPU_IRQ_BASE + 5)
#define M36_IRQ_INT4            (M36_CPU_IRQ_BASE + 6)
#define M36_IRQ_TIMER           (M36_CPU_IRQ_BASE + 7)

/* Cascaded from M36_IRQ_INT0 (ICU mapped interrupts) */

/* 
   IP3 - same as M36_IRQ_INT1
   IP8 - This is a cascade to GPIO IRQ's. Do not use.
   IP16 - same as M36_IRQ_INT2
   IP18 - CompactFlash
*/
#define INT_ALI_GPIO            (M36_SYS_IRQ_BASE + 0)  //M36_IRQ_SYSGPIO
#define INT_ALI_VE              (M36_SYS_IRQ_BASE + 1)  //M36_IRQ_VE
#define INT_ALI_DE              (M36_SYS_IRQ_BASE + 2)  //M36_IRQ_DE
#define M36_IRQ_TS              (M36_SYS_IRQ_BASE + 3)
#define M36_IRQ_GE              (M36_SYS_IRQ_BASE + 4)
#define INT_ALI_AUDIO           (M36_SYS_IRQ_BASE + 5)  //M36_IRQ_AUDIO
#define INT_ALI_TSG             (M36_SYS_IRQ_BASE + 6)  //M36_IRQ_TSG
#define INT_ALI_DEMUX1          (M36_SYS_IRQ_BASE + 7)  //M36_IRQ_DMX1
#define INT_ALI_DEMUX2          (M36_SYS_IRQ_BASE + 8)  //M36_IRQ_DMX2
#define M36_IRQ_CI              (M36_SYS_IRQ_BASE + 9)
#define M36_IRQ_ATA             (M36_SYS_IRQ_BASE + 10)
#define M36_IRQ_PCI             (M36_SYS_IRQ_BASE + 11)
#define M36_IRQ_SG              (M36_SYS_IRQ_BASE + 12)
#define INT_ALI_HDMI            (M36_SYS_IRQ_BASE + 13)  //M36_IRQ_HDMI
#define M36_IRQ_EXT             (M36_SYS_IRQ_BASE + 14)
#define M36_IRQ_DEO             (M36_SYS_IRQ_BASE + 15)
#define INT_ALI_UART1           (M36_SYS_IRQ_BASE + 16)  //M36_IRQ_UART1
#define INT_ALI_UART2           (M36_SYS_IRQ_BASE + 17)  //M36_IRQ_UART2
#define INT_ALI_SCB1            (M36_SYS_IRQ_BASE + 18)  //M36_IRQ_SCB
#define INT_ALI_IRC             (M36_SYS_IRQ_BASE + 19)  //M36_IRQ_IR
#define INT_ALI_SCR1            (M36_SYS_IRQ_BASE + 20)  //M36_IRQ_SCR1
#define INT_ALI_SCR2            (M36_SYS_IRQ_BASE + 21)  //M36_IRQ_SCR2
#define INT_ALI_RTC             (M36_SYS_IRQ_BASE + 22)  //M36_IRQ_RTC
#define INT_ALI_WDT             (M36_SYS_IRQ_BASE + 23)  //M36_IRQ_WDT
#define M36_IRQ_IRTX            (M36_SYS_IRQ_BASE + 24)
#define INT_ALI_SCB2            (M36_SYS_IRQ_BASE + 25)  //M36_IRQ_SCB2
#define INT_ALI_SCB3            (M36_SYS_IRQ_BASE + 26)  //M36_IRQ_SCB3
#define INT_ALI_DRAM            (M36_SYS_IRQ_BASE + 27)  //M36_IRQ_DRAM
#define INT_ALI_PANELSCAN       (M36_SYS_IRQ_BASE + 28)  //M36_IRQ_PANEL
#define INT_ALI_USBDEVIC        (M36_SYS_IRQ_BASE + 29)  //EM36_IRQ_USB
#define M36_IRQ_CPUILB          (M36_SYS_IRQ_BASE + 30)
#define M36_IRQ_BUSSERR         (M36_SYS_IRQ_BASE + 31)
#define M36_IRQ_CI2             (M36_SYS_IRQ_BASE + 32)
#define M36_IRQ_VIN             (M36_SYS_IRQ_BASE + 33)

#define INT_ALI_DEMUX3          NULL
#define INT_ALI_DEMUX4          NULL
#define INT_ALI_SWITCH_INT      NULL

/* Cascaded from M36_IRQ_GIU */
#define M36_IRQ_GPIO0           (M36_GPIO_IRQ_BASE + 0)
#define M36_IRQ_GPIO1           (M36_GPIO_IRQ_BASE + 1)
#define M36_IRQ_GPIO2           (M36_GPIO_IRQ_BASE + 2)
#define M36_IRQ_GPIO3           (M36_GPIO_IRQ_BASE + 3)
#define M36_IRQ_GPIO4           (M36_GPIO_IRQ_BASE + 4)
#define M36_IRQ_GPIO5           (M36_GPIO_IRQ_BASE + 5)
#define M36_IRQ_GPIO6           (M36_GPIO_IRQ_BASE + 6)
#define M36_IRQ_GPIO7           (M36_GPIO_IRQ_BASE + 7)
#define M36_IRQ_GPIO8           (M36_GPIO_IRQ_BASE + 8)
#define M36_IRQ_GPIO9           (M36_GPIO_IRQ_BASE + 9)
#define M36_IRQ_GPIO10          (M36_GPIO_IRQ_BASE + 10)
#define M36_IRQ_GPIO11          (M36_GPIO_IRQ_BASE + 11)
#define M36_IRQ_GPIO12          (M36_GPIO_IRQ_BASE + 12)
#define M36_IRQ_GPIO13          (M36_GPIO_IRQ_BASE + 13)
#define M36_IRQ_GPIO14          (M36_GPIO_IRQ_BASE + 14)
#define M36_IRQ_GPIO15          (M36_GPIO_IRQ_BASE + 15)
#define M36_IRQ_GPIO16          (M36_GPIO_IRQ_BASE + 16)
#define M36_IRQ_GPIO17          (M36_GPIO_IRQ_BASE + 17)
#define M36_IRQ_GPIO18          (M36_GPIO_IRQ_BASE + 18)
#define M36_IRQ_GPIO19          (M36_GPIO_IRQ_BASE + 19)
#define M36_IRQ_GPIO20          (M36_GPIO_IRQ_BASE + 20)
#define M36_IRQ_GPIO21          (M36_GPIO_IRQ_BASE + 21)
#define M36_IRQ_GPIO22          (M36_GPIO_IRQ_BASE + 22)
#define M36_IRQ_GPIO23          (M36_GPIO_IRQ_BASE + 23)
#define M36_IRQ_GPIO24          (M36_GPIO_IRQ_BASE + 24)
#define M36_IRQ_GPIO25          (M36_GPIO_IRQ_BASE + 25)
#define M36_IRQ_GPIO26          (M36_GPIO_IRQ_BASE + 26)
#define M36_IRQ_GPIO27          (M36_GPIO_IRQ_BASE + 27)
#define M36_IRQ_GPIO28          (M36_GPIO_IRQ_BASE + 28)
#define M36_IRQ_GPIO29          (M36_GPIO_IRQ_BASE + 29)
#define M36_IRQ_GPIO30          (M36_GPIO_IRQ_BASE + 30)
#define M36_IRQ_GPIO31          (M36_GPIO_IRQ_BASE + 31)

// Alternative to above GPIO IRQ defines
#define M36_IRQ_GPIO(pin)       ((M36_IRQ_GPIO0) + (pin))
#define M36_IRQ_MAX             (M36_IRQ_BASE + M36_NUM_CPU_IRQ + M36_NUM_SYS_IRQ)

#endif

#endif

