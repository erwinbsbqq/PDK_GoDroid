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
//#define IR_RLC_BUFFER_START     0x88000000
#define IR_RLC_BUFFER_START     0x87000000
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

#define IR_CTL_REG_SAVE            (IR_CACHE_END + 4)


#define DDR_ALL_END               IR_CTL_REG_SAVE

#define PLL_PM_VALUE_a0          (DDR_ALL_END + 4 * 1)
#define PLL_PM_VALUE_b0          (DDR_ALL_END + 4 * 2)
#define PLL_PM_VALUE_b4          (DDR_ALL_END + 4 * 3)
#define PLL_PM_VALUE_c0          (DDR_ALL_END + 4 * 4)

/* Store the timeout value */
#define PM_SLEEP_TIMEOUT        (PLL_PM_VALUE_c0 + 4 * 1)
#define PM_SLEEP_TIMEOUT_CONST  (PLL_PM_VALUE_c0 + 4 * 2)

/* Store the ir wakeup key */
#define PM_IR_WAKEUP_KEY1  (PM_SLEEP_TIMEOUT_CONST + 4 *1)
#define PM_IR_WAKEUP_KEY2  (PM_SLEEP_TIMEOUT_CONST + 4 *2)
#define PM_IR_WAKEUP_KEY3  (PM_SLEEP_TIMEOUT_CONST + 4 *3)
#define PM_IR_WAKEUP_KEY4  (PM_SLEEP_TIMEOUT_CONST + 4 *4)
#define PM_IR_WAKEUP_KEY5  (PM_SLEEP_TIMEOUT_CONST + 4 *5)
#define PM_IR_WAKEUP_KEY6  (PM_SLEEP_TIMEOUT_CONST + 4 *6)
#define PM_IR_WAKEUP_KEY7  (PM_SLEEP_TIMEOUT_CONST + 4 *7)
#define PM_IR_WAKEUP_KEY8  (PM_SLEEP_TIMEOUT_CONST + 4 *8)

#define SCB_REG0          (PM_IR_WAKEUP_KEY8 + 4 * 1)
#define SCB_REG4          (PM_IR_WAKEUP_KEY8 + 4 * 2)
#define SCB_REG8          (PM_IR_WAKEUP_KEY8 + 4 * 3)

#define STANDBY_TIMER          (SCB_REG8 + 4 * 1)
#define MONTH_ARRAY		(SCB_REG8 + 4 * 2)

#define CONFIGDATA_BASE  (MONTH_ARRAY + 4*4)

/* REG back up buffer ---- in memory */
#define REG_BAK_BUFFER_START    0x88004000
#define REG_BAK_0               (REG_BAK_BUFFER_START + 4 * 0)

#define US_STANDBY_TICKS    (27000000 / 2000000)
#define US_PER_SEC          (1000000)
#define SUSPEND_TICKS2SEC   (1 / (US_STANDBY_TICKS * US_PER_SEC))

//#define TIME_COMPARE_1S     (27000000 / 4)
#define TIME_COMPARE_1S     (27000000 /(4*8))	//SRC_LP_SEL 11: 1/64 CRYSTAL Clock
#define PWRDM_POWER_OFF		0x0
#define PWRDM_POWER_RET		0x1
#define PWRDM_POWER_INACTIVE	0x2
#define PWRDM_POWER_ON		0x3

#endif
