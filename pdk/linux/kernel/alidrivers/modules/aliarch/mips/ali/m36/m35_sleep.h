/**
* Copyright (c) 2011,Ali Corp.
* All rights reserved.
*
* FileName     : m36_sleep.h
* Verison      : 1.0
* Author       : Zhao Owen
* Date         : 2012-5-14
* Description  : The MACROs in this header is dedicated to fast standby.
*                Do not use for any other process
*/
#ifndef __ASM_ALI_M36_SLEEP_XXXXXXXXXXX______
#define __ASM_ALI_M36_SLEEP_XXXXXXXXXXX______

#define SEE_RUN_ADDR            0xa6000200

/* IR HW registers define */
#define SYS_IC_SB_BASE_H        0xb8018100
#define SYS_IC_SB_IRC_CFG       0x0
#define SYS_IC_SB_IRC_FCTRL     0x1
#define SYS_IC_SB_IRC_TTHR      0x2
#define SYS_IC_SB_IRC_NTHR      0x3
#define SYS_IC_SB_IRC_IER       0x6
#define SYS_IC_SB_IRC_ISR       0x7
#define SYS_IC_SB_IRC_DATA      0x8
#define SYS_IC_SB_IRC_BITIE     0x80000

/* IR pulse width buffer ---- in cache */
#define IR_RLC_BUFFER_START     0x88000000
#define IR_RLC_BUFFER_SIZE      256
#define IR_RLC_BUFFER_WTMRK     192

/* IR decoder related macro */
#define IR_TYPE_NUM             5
#define IR_PATTERN_START        (IR_RLC_BUFFER_START + IR_RLC_BUFFER_SIZE)
#define IR_PATTERN_SIZE         240
#define IR_PATTERN_CNT_START    (IR_PATTERN_START + IR_PATTERN_SIZE)
#define IR_PATTERN_CNT_SIZE     16
#define IR_ATTR_START           (IR_PATTERN_CNT_START + IR_PATTERN_CNT_SIZE)
#define IR_ATTR_SIZE            64
#define IR_DECODER_START        (IR_ATTR_START + (IR_ATTR_SIZE * IR_TYPE_NUM))
#define IR_DECODER_SIZE         40
#define IR_PLS_SUM              (IR_DECODER_START + (IR_DECODER_SIZE * IR_TYPE_NUM))
#define IR_RC6_FLG              (IR_PLS_SUM + 4)
#define IR_CACHE_END            (IR_RC6_FLG + 4)
#define DDR_SET_1               (IR_CACHE_END + 4)
#define DDR_SET_2               (IR_CACHE_END + 4 * 2)
#define DDR_SET_3               (IR_CACHE_END + 4 * 3)
#define DDR_SET_4               (IR_CACHE_END + 4 * 4)

#define DDR_SET_5               (IR_CACHE_END + 4 * 5)
#define DDR_SET_6               (IR_CACHE_END + 4 * 6)
#define DDR_SET_7               (IR_CACHE_END + 4 * 7)
#define DDR_SET_8               (IR_CACHE_END + 4 * 8)
#define DDR_SET_9               (IR_CACHE_END + 4 * 9)
#define DDR_SET_10              (IR_CACHE_END + 4 * 10)
#define DDR_SET_11              (IR_CACHE_END + 4 * 11)

#define DDR_PHY_BASE			DDR_SET_11

#define DDR_PHY1_PHY2_ENABLE	(DDR_PHY_BASE + 4)
#define MEM_CLK_SAVE			(DDR_PHY_BASE + 4 * 2)
#define DDR_PHY1_START			MEM_CLK_SAVE
#define DDR_PHY1_PGCR0			(DDR_PHY1_START + 4)
#define DDR_PHY1_PGCR1			(DDR_PHY1_START + 4 * 2)
#define DDR_PHY1_PLLCR			(DDR_PHY1_START + 4 * 3)
#define DDR_PHY1_PTR0			(DDR_PHY1_START + 4 * 4)
#define DDR_PHY1_PTR1			(DDR_PHY1_START + 4 * 5)
#define DDR_PHY1_PTR2			(DDR_PHY1_START + 4 * 6)
#define DDR_PHY1_PTR3			(DDR_PHY1_START + 4 * 7)
#define DDR_PHY1_PTR4			(DDR_PHY1_START + 4 * 8)
#define DDR_PHY1_ACMDLR			(DDR_PHY1_START + 4 * 9)
#define DDR_PHY1_ACDBLR			(DDR_PHY1_START + 4 * 10)
#define DDR_PHY1_ACIOCR			(DDR_PHY1_START + 4 * 11)
#define DDR_PHY1_DXCCR			(DDR_PHY1_START + 4 * 12)
#define DDR_PHY1_DSGCR			(DDR_PHY1_START + 4 * 13)
#define DDR_PHY1_DCR			(DDR_PHY1_START + 4 * 14)
#define DDR_PHY1_DTPR0			(DDR_PHY1_START + 4 * 15)
#define DDR_PHY1_DTPR1			(DDR_PHY1_START + 4 * 16)
#define DDR_PHY1_DTPR2			(DDR_PHY1_START + 4 * 17)
#define DDR_PHY1_MR0_MR			(DDR_PHY1_START + 4 * 18)
#define DDR_PHY1_MR1_EMR		(DDR_PHY1_START + 4 * 19)
#define DDR_PHY1_MR2			(DDR_PHY1_START + 4 * 20)
#define DDR_PHY1_MR3			(DDR_PHY1_START + 4 * 21)
#define DDR_PHY1_PGCR2			(DDR_PHY1_START + 4 * 22)
#define DDR_PHY1_ZQ0CR0			(DDR_PHY1_START + 4 * 23)
#define DDR_PHY1_ZQ0CR1			(DDR_PHY1_START + 4 * 24)
#define DDR_PHY1_ZQ0SR0			(DDR_PHY1_START + 4 * 25)
#define DDR_PHY1_ZQ0SR1			(DDR_PHY1_START + 4 * 26)
#define DDR_PHY1_DX0GCR			(DDR_PHY1_START + 4 * 27)
#define DDR_PHY1_DX0BDLR0		(DDR_PHY1_START + 4 * 28)
#define DDR_PHY1_DX0BDLR1		(DDR_PHY1_START + 4 * 29)
#define DDR_PHY1_DX0BDLR2		(DDR_PHY1_START + 4 * 30)
#define DDR_PHY1_DX0BDLR3		(DDR_PHY1_START + 4 * 31)
#define DDR_PHY1_DX0BDLR4		(DDR_PHY1_START + 4 * 32)
#define DDR_PHY1_DX0LCDLR0		(DDR_PHY1_START + 4 * 33)
#define DDR_PHY1_DX0LCDLR1		(DDR_PHY1_START + 4 * 34)
#define DDR_PHY1_DX0LCDLR2		(DDR_PHY1_START + 4 * 35)
#define DDR_PHY1_DX0MDLR		(DDR_PHY1_START + 4 * 36)
#define DDR_PHY1_DX0GTR			(DDR_PHY1_START + 4 * 37)
#define DDR_PHY1_DX1GCR			(DDR_PHY1_START + 4 * 38)
#define DDR_PHY1_DX1BDLR0		(DDR_PHY1_START + 4 * 39)
#define DDR_PHY1_DX1BDLR1		(DDR_PHY1_START + 4 * 40)
#define DDR_PHY1_DX1BDLR2		(DDR_PHY1_START + 4 * 41)
#define DDR_PHY1_DX1BDLR3		(DDR_PHY1_START + 4 * 42)
#define DDR_PHY1_DX1BDLR4		(DDR_PHY1_START + 4 * 43)
#define DDR_PHY1_DX1LCDLR0		(DDR_PHY1_START + 4 * 44)
#define DDR_PHY1_DX1LCDLR1		(DDR_PHY1_START + 4 * 45)
#define DDR_PHY1_DX1LCDLR2		(DDR_PHY1_START + 4 * 46)
#define DDR_PHY1_DX1MDLR		(DDR_PHY1_START + 4 * 47)
#define DDR_PHY1_DX1GTR			(DDR_PHY1_START + 4 * 48)
#define DDR_PHY1_END			DDR_PHY1_DX1GTR

#define DDR_PHY2_START			DDR_PHY1_END
#define DDR_PHY2_PGCR0			(DDR_PHY2_START + 4)
#define DDR_PHY2_PGCR1			(DDR_PHY2_START + 4 * 2)
#define DDR_PHY2_PLLCR			(DDR_PHY2_START + 4 * 3)
#define DDR_PHY2_PTR0			(DDR_PHY2_START + 4 * 4)
#define DDR_PHY2_PTR1			(DDR_PHY2_START + 4 * 5)
#define DDR_PHY2_PTR2			(DDR_PHY2_START + 4 * 6)
#define DDR_PHY2_PTR3			(DDR_PHY2_START + 4 * 7)
#define DDR_PHY2_PTR4			(DDR_PHY2_START + 4 * 8)
#define DDR_PHY2_ACMDLR			(DDR_PHY2_START + 4 * 9)
#define DDR_PHY2_ACDBLR			(DDR_PHY2_START + 4 * 10)
#define DDR_PHY2_ACIOCR			(DDR_PHY2_START + 4 * 11)
#define DDR_PHY2_DXCCR			(DDR_PHY2_START + 4 * 12)
#define DDR_PHY2_DSGCR			(DDR_PHY2_START + 4 * 13)
#define DDR_PHY2_DCR			(DDR_PHY2_START + 4 * 14)
#define DDR_PHY2_DTPR0			(DDR_PHY2_START + 4 * 15)
#define DDR_PHY2_DTPR1			(DDR_PHY2_START + 4 * 16)
#define DDR_PHY2_DTPR2			(DDR_PHY2_START + 4 * 17)
#define DDR_PHY2_MR0_MR			(DDR_PHY2_START + 4 * 18)
#define DDR_PHY2_MR1_EMR		(DDR_PHY2_START + 4 * 19)
#define DDR_PHY2_MR2			(DDR_PHY2_START + 4 * 20)
#define DDR_PHY2_MR3			(DDR_PHY2_START + 4 * 21)
#define DDR_PHY2_PGCR2			(DDR_PHY2_START + 4 * 22)
#define DDR_PHY2_ZQ0CR0			(DDR_PHY2_START + 4 * 23)
#define DDR_PHY2_ZQ0CR1			(DDR_PHY2_START + 4 * 24)
#define DDR_PHY2_ZQ0SR0			(DDR_PHY2_START + 4 * 25)
#define DDR_PHY2_ZQ0SR1			(DDR_PHY2_START + 4 * 26)
#define DDR_PHY2_DX0GCR			(DDR_PHY2_START + 4 * 27)
#define DDR_PHY2_DX0BDLR0		(DDR_PHY2_START + 4 * 28)
#define DDR_PHY2_DX0BDLR1		(DDR_PHY2_START + 4 * 29)
#define DDR_PHY2_DX0BDLR2		(DDR_PHY2_START + 4 * 30)
#define DDR_PHY2_DX0BDLR3		(DDR_PHY2_START + 4 * 31)
#define DDR_PHY2_DX0BDLR4		(DDR_PHY2_START + 4 * 32)
#define DDR_PHY2_DX0LCDLR0		(DDR_PHY2_START + 4 * 33)
#define DDR_PHY2_DX0LCDLR1		(DDR_PHY2_START + 4 * 34)
#define DDR_PHY2_DX0LCDLR2		(DDR_PHY2_START + 4 * 35)
#define DDR_PHY2_DX0MDLR		(DDR_PHY2_START + 4 * 36)
#define DDR_PHY2_DX0GTR			(DDR_PHY2_START + 4 * 37)
#define DDR_PHY2_DX1GCR			(DDR_PHY2_START + 4 * 38)
#define DDR_PHY2_DX1BDLR0		(DDR_PHY2_START + 4 * 39)
#define DDR_PHY2_DX1BDLR1		(DDR_PHY2_START + 4 * 40)
#define DDR_PHY2_DX1BDLR2		(DDR_PHY2_START + 4 * 41)
#define DDR_PHY2_DX1BDLR3		(DDR_PHY2_START + 4 * 42)
#define DDR_PHY2_DX1BDLR4		(DDR_PHY2_START + 4 * 43)
#define DDR_PHY2_DX1LCDLR0		(DDR_PHY2_START + 4 * 44)
#define DDR_PHY2_DX1LCDLR1		(DDR_PHY2_START + 4 * 45)
#define DDR_PHY2_DX1LCDLR2		(DDR_PHY2_START + 4 * 46)
#define DDR_PHY2_DX1MDLR		(DDR_PHY2_START + 4 * 47)
#define DDR_PHY2_DX1GTR			(DDR_PHY2_START + 4 * 48)
#define DDR_PHY2_END            DDR_PHY2_DX1GTR

#define DDR_DM_REG1				(DDR_PHY2_END + 4 )
#define DDR_DM_REG2				(DDR_PHY2_END + 4 * 2)
#define	DDR_DM_LEN				0x80

	
#define DDR_ALL_END				(DDR_DM_REG1+DDR_DM_LEN)

/* store pll vaule*/
#define PLL_PM_VAULE_START      DDR_ALL_END
#define PLL_PM_VALUE_0          (PLL_PM_VAULE_START + 4 * 1)
#define PLL_PM_VALUE_1          (PLL_PM_VAULE_START + 4 * 2)
#define PLL_PM_VALUE_2          (PLL_PM_VAULE_START + 4 * 3)
#define PLL_PM_VALUE_3          (PLL_PM_VAULE_START + 4 * 4)
#define PLL_PM_VALUE_4          (PLL_PM_VAULE_START + 4 * 5)
#define PLL_PM_VALUE_END        (PLL_PM_VAULE_START + 4 * 5)


/* Store the timeout value */
#define PM_SLEEP_START          PLL_PM_VALUE_END
#define PM_SLEEP_TIMEOUT        (PM_SLEEP_START+ 4 * 1)
#define PM_SLEEP_TIMEOUT_CONST  (PM_SLEEP_START+ 4 * 2)
#define PM_SLEEP_TIME_CONST     (PM_SLEEP_START+ 4 * 3)
/* Store the mem clk */
#define  PM_MEM_CLK             (PM_SLEEP_TIME_CONST+4) 
/* REG back up buffer ---- in memory */
#define REG_BAK_BUFFER_START    0x88004000
#define REG_BAK_0               (REG_BAK_BUFFER_START + 4 * 0)

/* REG back up buffer ---- in PMU sram */
#define PMU_SRAM_BASE_ADDR      0xb8050000
#define REG_BAK_CPU             (PMU_SRAM_BASE_ADDR + 0x3F00)
#define AS_FLAG_TO_SEE          0xdeadbeef

#define SET_AS_FLAG_TO_SEE      (*(volatile unsigned long *)(REG_BAK_CPU+32) = AS_FLAG_TO_SEE)

#define US_STANDBY_TICKS		(27000000 / 2000000)
#define US_PER_SEC				(1000000)
#define SUSPEND_TICKS2SEC		(1 / (US_STANDBY_TICKS * US_PER_SEC))

#define TIME_COMPARE_1S			(27000000 / 4)

#define ALI_IRC_BASE			0x18018100      /* Memory base for ALI_M3602_IRC */
#define INFRA_IRCCFG			(ALI_IRC_BASE + 0x00)
#define INFRA_FIFOCTRL			(ALI_IRC_BASE + 0x01)
#define INFRA_TIMETHR			(ALI_IRC_BASE + 0x02)
#define INFRA_NOISETHR			(ALI_IRC_BASE + 0x03)
#define INFRA_IER				(ALI_IRC_BASE + 0x06)
#define INFRA_ISR				(ALI_IRC_BASE + 0x07)
#define INFRA_RLCBYTE			(ALI_IRC_BASE + 0x08)
#define INTV_REPEAT				250     /* in mini second */
#define INTV_REPEAT_FIRST		300     /* in mini second */
#define PAN_KEY_INVALID			0xFFFFFFFF

#define VALUE_TOUT			    24000   /* Timeout threshold, in uS */
#define VALUE_CLK_CYC		    8       /* Work clock cycle, in uS */
#define VALUE_NOISETHR		    80      /* Noise threshold, in uS */
#define IR_RLC_SIZE			    256
#define IO_BASE_ADDR			0x18000000
#define AUDIO_IO_BASE_ADDR		0x18002000
#define USB_IO_BASE_ADDR		0x1803d800
#define HDMI_PHY 	            0x1800006c
#define SYS_IC_NB_BASE_H		0x1800
#define SYS_IC_NB_CFG_SEQ0		0x74
#define SYS_IC_NB_BIT1033		0x1033
#define SYS_IC_NB_BIT1031		0x1031

/* ch455*/
#define SETING_ADDR	0x48
#define DIG0_ADDR	    0x68
#define DIG1_ADDR	    0x6a
#define DIG2_ADDR	    0x6c
#define DIG3_ADDR	    0x6e
#define KEY_ADDR	    0x4f
#define KEY_PRESS	    0x55
#define CH455_MODE	    0x01
/* GPIO IIC*/
#define I2C_GPIO_SCL              135
#define I2C_GPIO_SDA		      134
//# #define _HALF_PERIOD_COUNT	  200      # debug cpu 27M
//# #define _HALF_PERIOD_COUNT	  60000000*3 # debug cpu 600M  1s
//# #define _HALF_PERIOD_COUNT	  60*3*15 # debug cpu 600M  15us   right  normal
//# 600M 180 means 1us  
#define  _HALF_PERIOD_COUNT       300//120
//##bps(40K)-->half period(12.5us)
//##crystal(27M)-->1s count(27M/4)-->half period count(27*12.5/4=85)-->CPU clk(170)
//##crystal(28.8M)-->1s count(28.8M/32)-->half period count(28.8*12.5/32=12)-->CPU clk(24)##crystal(13.5M)-->1s count(13.5M/32)-->half period count(13.5*12.5/32=6)-->CPU clk(12)
#define I2C_GPIO_READ_TIMEOUT	  7
//# gpio 0 - 31 
#define HAL_GPIO_EN_REG	        0xb8000430
#define HAL_GPIO_DIR_REG	      0xb8000058
#define HAL_GPIO_DI_REG		      0xb8000050
#define HAL_GPIO_DO_REG		      0xb8000054
//# gpio 32 - 63  gpioA
#define HAL_GPIO1_EN_REG	      0xb8000434
#define HAL_GPIO1_DIR_REG	      0xb80000D8
#define HAL_GPIO1_DI_REG	      0xb80000D0
#define HAL_GPIO1_DO_REG	      0xb80000D4
//# gpio 64 - 95  gpioB
#define HAL_GPIO2_EN_REG	      0xb8000438
#define HAL_GPIO2_DIR_REG  	    0xb80000F8
#define HAL_GPIO2_DI_REG	      0xb80000F0
#define HAL_GPIO2_DO_REG	      0xb80000F4
//# gpio 96 - 127  gpioC
#define HAL_GPIO3_EN_REG   	    0xb800043C
#define HAL_GPIO3_DIR_REG	      0xb8000358
#define HAL_GPIO3_DI_REG	      0xb8000350
#define HAL_GPIO3_DO_REG	      0xb8000354
//# gpio 128 - 159  gpioC
#define HAL_GPIO4_EN_REG   	      0xb8000440
#define HAL_GPIO4_DIR_REG	      0xb8000458
#define HAL_GPIO4_DI_REG	      0xb8000450
#define HAL_GPIO4_DO_REG	      0xb8000454

#define PM_FLAG                   0xabcddcba
#define PM_REMOTE                 0xffffffff
#endif