/**
* Copyright (c) 2011,Ali Corp.
* All rights reserved.
*
* FileName     : m36_sleep.S
* Verison      : 1.0
* Author       : Zhao Owen
* Date         : 2011-07-28
* Description  : The file is to make standby to support ddr self refresh.
*/

#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>
#include <asm/cacheops.h>
#include "mips.h"
#include "m35_sleep.h"

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

/* IR bit type define */
#define IR_LEADING              0
#define IR_SPECIAL              1
#define IR_DATA                 2
#define IR_REPEAT_LEADING       3
#define IR_REPEAT_DATA          4
#define IR_STOP                 5
#define IR_END                  6
#define IR_NULL                 0xff

/* bit pattern */
IR_ROM_PATTERN:
# NEC
    .byte \
        IR_LEADING, IR_DATA:32, IR_END
# LAB
    .byte \
        IR_LEADING, IR_SPECIAL, IR_DATA:9, IR_END
# LOGIC
    .byte \
        IR_LEADING, IR_DATA:12, IR_END
# KF
    .byte \
        IR_LEADING, IR_DATA:48, IR_END
# JZ
    .byte \
        IR_LEADING, IR_DATA:24, IR_LEADING, IR_DATA:16, IR_END
IR_ROM_PATTERN_END:

/* pattern count */
IR_ROM_PATTERN_CNT_LIST:
# NEC: 34, LAB: 12, LOGIC: 14, KF: 50, JZ: 43
    .byte \
        0, 34, 46, 60, 110

/* 
 * pulse attribute 
 */
#define BIT_MSB_FIRST           (1 << 0)
#define BYTE_MSB_FIRST          (1 << 1)
#define PULSE_INVERT            (1 << 2)
#define REPEAT_ENABLE           (1 << 3)

#define PULSE_TYPE              0
#define PULSE_LOW               2
#define PULSE_HIGH              4
#define PULSE_TORL              6

#define PULSE_PREC              2
#define PULSE_MAX_WIDTH         4
#define PULSE_LEADING           8
#define PULSE_SPECIAL           16
#define PULSE_DATA0             24
#define PULSE_DATA1             32
#define PULSE_R_LEADING         40
#define PULSE_R_DATA            48
#define PULSE_STOP              56

IR_ROM_ATTR:
# NEC
    .half \
        (BIT_MSB_FIRST | REPEAT_ENABLE), 4, 210, 0, \
        IR_LEADING, 123, 61, 15, \
        IR_NULL, 0, 0, 0, \
        IR_DATA, 8, 8, 4, \
        IR_DATA, 8, 23, 7, \
        IR_REPEAT_LEADING, 123, 31, 15, \
        IR_REPEAT_DATA, 8, 280, 7, \
        IR_STOP, 7, 280, 14
# LAB
    .half \
        BIT_MSB_FIRST, 2, 210, 0, \
        IR_LEADING, 8, 99, 8, \
        IR_SPECIAL, 4, 83, 20, \
        IR_DATA, 4, 65, 10, \
        IR_DATA, 4, 99, 10, \
        IR_NULL, 0, 0, 0, \
        IR_NULL, 0, 0, 0, \
        IR_STOP, 7, 280, 14
# LOGIC
    .half \
        0, 2, 210, 0, \
        IR_LEADING, 70, 70, 10, \
        IR_NULL, 0, 0, 0, \
        IR_DATA, 4, 7, 4, \
        IR_DATA, 7, 14, 4, \
        IR_NULL, 0, 0, 0, \
        IR_NULL, 0, 0, 0, \
        IR_STOP, 7, 280, 14
# KF
    .half \
        BYTE_MSB_FIRST, 2, 280, 0, \
        IR_LEADING, 51, 25, 7, \
        IR_NULL, 0, 0, 0, \
        IR_DATA, 5, 5, 4, \
        IR_DATA, 5, 19, 4, \
        IR_NULL, 0, 0, 0, \
        IR_NULL, 0, 0, 0, \
        IR_STOP, 7, 280, 14
# JZ
    .half \
        (BIT_MSB_FIRST | REPEAT_ENABLE), 2, 840, 0, \
        IR_LEADING, 51, 25, 7, \
        IR_NULL, 0, 0, 0, \
        IR_DATA, 8, 8, 4, \
        IR_DATA, 8, 16, 6, \
        IR_REPEAT_LEADING, 51, 51, 7, \
        IR_REPEAT_DATA, 8, 280, 14, \
        IR_STOP, 7, 280, 14
IR_ROM_ATTR_END:

#define FIRST_HALF_GOT          0
#define DECODE_STEP             1
#define KEY_BIT_CNT             2
#define LAST_PULSE_WIDTH        4
#define KEY_CODE                8
#define LAST_KEY_CODE           24

/*
 * Debug options
 */
#ifdef CONFIG_ALI_AS
#define DDR_INTEGRITY_CHECK     1 
#define SYNC_WITH_SEE_ENABLE    1
#else
#define DDR_INTEGRITY_CHECK     0 
#endif
#define DDR_SELF_REFRESH        1
#define CPU_FREQ_ADJUST         0
#define CODE_IN_CACHE           1
#define DATA_IN_CACHE           1
#define ENABLE_DEEP_STANDBY     1 /* It's not a real deep standby, just lower CPU freq */
#define ENABLE_PM_DEBUG         1
#define ENABLE_LOOP_LOG         0
#define ENABLE_PRE_ENTER_DEBUG  0
#define ENABLE_SEE_RESET        0
#define ENABLE_SEE_CLOCKGATE    1   
#define UART_NOT_OFF            0
#define ENTER_STEP_DEBUG        0
#define ENABLE_MEMORY_TEST      0
#define MEMORY_SHUTDOWN         1   
#define DDRPHY_BACKUP_MEMORY	0
#define REG_BACKUP_MEMORY       0
#define REG_BACKUP_IN_PMU       1
#define ENABLE_MEM_BYPASS       0  /* This will be enable when real standby */
#define ENABLE_PLL_PM           1
#define ENABLE_BOARD_PM         0
#define ENABLE_IR_CLOCK         1
#define ENABLE_TIMER_SUPPORT	1
#define ENABLE_MEM_STANDBY		1
#define ENABLE_GPIO_IIC_SUPPORT	1
#define DOWN_PD                 0
#if ENABLE_BOARD_PM
#undef UART_NOT_OFF
#undef ENABLE_PM_DEBUG         
#undef ENABLE_LOOP_LOG         
#undef ENABLE_PRE_ENTER_DEBUG  
#define UART_NOT_OFF            0
#define ENABLE_PM_DEBUG         0
#define ENABLE_LOOP_LOG         0
#define ENABLE_PRE_ENTER_DEBUG  0
#endif

#if DDR_SELF_REFRESH
#undef CODE_IN_CACHE
#undef DATA_IN_CACHE
#define CODE_IN_CACHE           1
#define DATA_IN_CACHE           1
#else
#undef MEMORY_SHUTDOWN
#define MEMORY_SHUTDOWN         0
#endif

#if ENABLE_DEEP_STANDBY
#undef ENABLE_NONDEEP_IR
#undef CPU_FREQ_ADJUST
#undef CODE_IN_CACHE
#undef DATA_IN_CACHE
#define CODE_IN_CACHE           1
#define DATA_IN_CACHE           1
#define CPU_FREQ_ADJUST         0
#define ENABLE_NONDEEP_IR       0
#else
#undef ENABLE_PLL_PM
#undef ENABLE_NONDEEP_IR
#define ENABLE_PLL_PM			0
#define ENABLE_NONDEEP_IR       1
#endif


/*
 * Useful MACROs --To avoid the crash of the function (don't know why)--
 */
.macro SLEEP_TIME_CACULATE_M
    mfc0    t0, C0_COUNT
    nop
    mfc0    t1, C0_COMPARE
    nop
    subu    t0, t1, t0
    bgez    t0, next_loop
    nop
    
    li      t2, TIME_COMPARE_1S
    addu    t1, t2
    mtc0    t1, C0_COMPARE
    nop

    # add sec 
    li      t0, PM_SLEEP_TIME_CONST
    lw      t1, 0x0(t0)
    addiu   t1, 1
    sw      t1, 0x0(t0)
    # time out 
    li      t0, PM_SLEEP_TIMEOUT
    lw      t1, 0x0(t0)
    addiu   t1, -1
    sw      t1, 0x0(t0)

next_loop:
    nop
    .endm
 
.macro SUSPEND_STEP_EX_M step
#if (ENTER_STEP_DEBUG)
    li      t8, \step
#else
#endif
    .endm
    
.macro SUSPEND_STEP_M step
#if (ENTER_STEP_DEBUG && UART_NOT_OFF)
    li      s4, \step
#else
#endif
    .endm

.macro BOARD_POWER_DOWN_M
#if ENABLE_BOARD_PM
    addiu   s4, 1
    beqz    s4, 1f
    subu    s4, 65
    
    lui     t0, 0xb800

    lw      t1, 0x438(t0)
    li      t2, 0x1
    sll     t2, s4
    or      t1, t2
    sw      t1, 0x438(t0)

    lw      t1, 0xf8(t0)
    li      t2, 0x1
    sll     t2, s4
    or      t1, t2
    sw      t1, 0xf8(t0)
    
    lw      t1, 0xf4(t0)
    li      t2, 0x1
    sll     t2, s4
    or      t1, t2
    sw      t1, 0xf4(t0)

    addiu   s4, 64
1:
    nop
#endif
    .endm

.macro BOARD_POWER_UP_M
#if ENABLE_BOARD_PM
    addiu   s4, 1
    beqz    s4, 1f
    subu    s4, 65
    
    lui     t0, 0xb800

    lw      t1, 0xf4(t0)
    li      t2, 0x1
    sll     t2, s4
    not     t2, t2
    and     t1, t2
    sw      t1, 0xf4(t0)
1:
    nop
#endif
    .endm

.macro  PLL_POWER_DOWN_M
#if ENABLE_PLL_PM
    .set    at
    lui     t0, 0xb800          # b0
    lw      t1, 0xb0(t0)
    li      t2, PLL_PM_VALUE_0
    sw      t1, 0(t2)
    or      t1, (1<<20)
    sw      t1, 0xb0(t0)

    lw      t1, 0xa0(t0)         # a0
    li      t2, PLL_PM_VALUE_1
    sw      t1, 0(t2)
    or      t1, (1<<15)
    sw      t1, 0xa0(t0)

    lw      t1, 0xc0(t0)        # c0
    li      t2, PLL_PM_VALUE_2
    sw      t1, 0(t2)
    or      t1, (1<<0)|(1<<16)
    sw      t1, 0xc0(t0)

    lw      t1, 0xb4(t0)        # b4
    li      t2, PLL_PM_VALUE_3
    sw      t1, 0(t2)
    or      t1, (1<<12)
    sw      t1, 0xb4(t0)
    lui      t0, 0xb805
    lw      t1, 0xc0c0(t0)        # pmu
    li      t2, PLL_PM_VALUE_4
    sw      t1, 0(t2)
    and     t1, 0xfffffff8
    sw      t1, 0xc0c0(t0)
    .set    noat
#endif
    .endm

.macro  PLL_POWER_UP_M
#if ENABLE_PLL_PM
    .set    at
    lui     t0, 0xb805         
    li      t2, PLL_PM_VALUE_4
    lw      t1, 0(t2)
    sw      t1, 0xc0c0(t0)  
    lui     t0, 0xb800

    li      t2, PLL_PM_VALUE_3
    lw      t1, 0(t2)
    sw      t1, 0xb4(t0)  
    
    li      t2, PLL_PM_VALUE_2
    lw      t1, 0(t2)
    sw      t1, 0xc0(t0)

    li      t2, PLL_PM_VALUE_1
    lw      t1, 0(t2)
    sw      t1, 0xa0(t0)
    
    li      t2, PLL_PM_VALUE_0
    lw      t1, 0(t2)
    sw      t1, 0xb0(t0)  
    .set    noat
#endif
    .endm


.macro  GATE_SEE_CLK
    #if ENABLE_SEE_CLOCKGATE
    /* close slave CPU clock */
    lui     t0, 0xb800
    lbu     t1, 0x67(t0)
    or      t1, 0x80
    sb      t1, 0x67(t0)
    #endif
    nop
   .set    noat
.endm


.macro  RESUME_SEE_CLK
    /* open slave CPU clock */
    #if ENABLE_SEE_CLOCKGATE
    lui     t0, 0xb800
    lbu     t1, 0x67(t0)
    and     t1, 0x7f
    sb      t1, 0x67(t0) 
    #endif   
    nop
   .set    noat
.endm


.macro  SYS_ENTER_REAL_STANDBY
#if ENABLE_DEEP_STANDBY
    .set    at
    lui		t0, 0xb800
    lw		t1, 0x68(t0)
	ori		t1, 0x05
	sw		t1, 0x68(t0)
#endif
    nop
	.set    noat
.endm


.macro  SYS_EXIT_REAL_STANDBY
#if ENABLE_DEEP_STANDBY
    .set    at
     lui		t0, 0xb800
	lw		t1, 0x68(t0)
	and  	t1, ~0x05
	sw		t1, 0x68(t0)
	nop
#endif
    nop
	.set    noat
.endm
.macro	MEM_ENTER_STANDBY_M
#if	ENABLE_MEM_STANDBY
    .set    at
	lui		t0, 0xb800
# DRAM entry self refresh mode
		lw		t1, 0x1030(t0)
		or		t1, 0x8000
		sw		t1, 0x1030(t0)
		nop
		nop
		and     t1, 0x7fffffff
		sw      t1, 0x1030(t0)
		nop
		nop
		or      t1, 0x01000000
		sw      t1, 0x1030(t0)
		nop
    .set    noat
#else
	nop
#endif
    .endm

.macro MEM_ENTER_BYPASS_M
#if ENABLE_MEM_BYPASS
    .set    at
    lui     t0, 0xb800
    lw      t1, 0x74(t0)
    or      t1, 0x00200000
    and     t1, 0xffffff8f
    sw      t1, 0x74(t0)
    .set    noat
#else
    nop
#endif
    .endm

.macro MEM_EXIT_BYPASS_M
#if ENABLE_MEM_BYPASS
    .set    at
    lui     t0, 0xb800
    lw      t1, 0x74(t0)
    and     t1 , ~((1<<4)|(1<<5)|(1<<6)) # clear bit[6:4]  
    
    li      t2, PM_MEM_CLK
    lw      t3 ,(t2)    
    and     t3 ,(1<<4)|(1<<5)|(1<<6)    # get mem clk seting
    or      t3 ,(1<<21)                 # trig
    or      t3 , t1
    sw      t3 , 0x74(t0)
    nop
    nop
    nop
    .set    noat
#else
    nop
#endif
    .endm

/*
    this function used to make pad set all io pad drive from 5mA to 2.5mA  
    when exit standy it must resume to 5mA
*/

.macro PD_DOWN_ELECTRI
#if DOWN_PD
    .set    at
    lui  t0 , 0xb800
    sw	zero,0x410(t0)
    sw	zero,0x414(t0)
    sw	zero,0x418(t0)
    sw	zero,0x41c(t0)
    sw	zero,0x420(t0)	
    sw	zero,0x424(t0)
    .set    noat
#else
    nop
#endif
    .endm

.macro PD_UP_ELECTRI
 #if DOWN_PD
    .set    at
    // set all io pad drive from 5mA to 2.5mA
	li	t1,0x55555555	
	lui t0,0xb800
	sw	t1,0x410(t0)	
	sw	t1,0x414(t0)	
	sw	t1,0x418(t0)	
	sw	t1,0x41c(t0)	
	sw	t1,0x420(t0)	
	sw	t1,0x424(t0)
    .set    noat
#else
    nop
#endif
    .endm


/*
 *	1.	Before enter standby mode, please store 
 *		DDR3PHY_1 / DDR3PHY_2 in PMU SRAM.
 *	2.	Use a Flag (STANDBY_FLAG) to record "enter 
 *		standby mode" for distinguish cold boot 
 *		or exit standy mode.
 *	3.	DMCTRL assert "Enter Self Refresh" command.
 */
.macro	MEM_ENTER_STANDBY_EX_M
#if MEMORY_SHUTDOWN
	.set    at

        li      t0, 0xb8001000
        li      t2, DDR_DM_REG1
        li      t3, 64                 ##the total register is 16
##store all the memory control register into NF stram register.
##need recove when exit standby
1:
		/* Store 0xb8001000 -0xb8001030 */
		lw      t1, 0x0(t0)
		sw      t1, 0x0(t2)
		addiu   t0, 4
		addiu   t2, 4
		addiu   t3, -4
		bnez    t3, 1b
		nop

		li      t0, 0xb8001000
		lw		t1, 0x78(t0)			# 0xb8001078
		sw		t1, 0x0(t2)
		addiu   t2, 4
		lw		t1, 0x7c(t0)			# 0xb800107c
		sw		t1, 0x0(t2)
		addiu   t2, 4
		lw		t1, 0x80(t0)			# 0xb8001080
		sw		t1, 0x0(t2)
		addiu   t2, 4
		lw		t1, 0x68(t0)			# 0xb8001068
		sw		t1, 0x0(t2)
		addiu   t2, 4
		lw		t1, 0x94(t0)			# 0xb8001094
		sw		t1, 0x0(t2)
		addiu   t2, 4
		lw		t1, 0x98(t0)			# 0xb8001098
		sw		t1, 0x0(t2)
		addiu   t2, 4
		lw		t1, 0x9c(t0)			# 0xb800109c
		sw		t1, 0x0(t2)
		addiu   t2, 4

		li      t0, 0xb8001200

		li		t4, 0x00011000
		lw		t1, 0x7c(t0)
		sw		t1, 0x0(t2)
		sw		t4, 0x7c(t0)
		li		t4, 0x0
		addiu	t2, 4

		lw		t1, 0x80(t0)
		sw		t1, 0x0(t2)
		sw		t4, 0x80(t0)
		addiu	t2, 4
		lw		t1, 0x84(t0)
		sw		t1, 0x0(t2)
		sw		t4, 0x84(t0)
		addiu	t2, 4
		lw		t1, 0x8c(t0)
		sw		t1, 0x0(t2)
		sw		t4, 0x8c(t0)
		addiu	t2, 4
		lw		t1, 0x9c(t0)
		sw		t1, 0x0(t2)
		sw		t4, 0x9c(t0)
		addiu	t2, 4	
	.set    noat
#else
    nop
#endif
    .endm

/*
 *	Exit Standby mode
 *	1.	PMU distinguishes this time is wakeup from 
 *      standy mode via a pre-defined flag (STANDBY_FLAG).
 *  2.	Disable unused DDR3PHY.
 *  3.	Restore DDR3PHY_1 / DDR3PHY_2 register.
 *      Please refer to sdram_fast_resume_for_3606k.s for the detail.
 *	4.	DMCTR assert "Exit Self Refresh" command.
 */
.macro	MEM_EXIT_STANDBY_EX_M    
#if MEMORY_SHUTDOWN
	.set	at
	    li      t0, 0xb8001000
        li      t2, DDR_DM_REG1
        li      t3, 64                 ##the total register is 16
##store all the memory control register from NF stram register.
1:
		/* Store 0xb8001000 -0xb8001030 */
		lw      t1, 0x0(t2)
		sw      t1, 0x0(t0)
		addiu   t0, 4
		addiu   t2, 4
		addiu   t3, -4
		bnez    t3, 1b
		nop

		li      t0, 0xb8001000
		lw		t1, 0x0(t2)
		sw		t1, 0x78(t0)			# 0xb8001078
		addiu   t2, 4
		lw		t1, 0x0(t2)
		sw		t1, 0x7c(t0)			# 0xb800107c
		addiu   t2, 4
		lw		t1, 0x0(t2)
		sw		t1, 0x80(t0)			# 0xb8001080
		addiu   t2, 4
		lw		t1, 0x0(t2)
		sw		t1, 0x68(t0)			# 0xb8001068
		addiu   t2, 4
		lw		t1, 0x0(t2)
		sw		t1, 0x94(t0)			# 0xb8001094
		addiu   t2, 4
		lw		t1, 0x0(t2)
		sw		t1, 0x98(t0)			# 0xb8001098
		addiu   t2, 4
		lw		t1, 0x0(t2)
		sw		t1, 0x9c(t0)			# 0xb800109c
		addiu   t2, 4

		li      t0, 0xb8001200
/*		lw		t1, 0x0(t2)
		sw		t1, 0x7c(t0)
		addiu	t2, 4 */
		lw		t1, 0x0(t2)
		sw		t1, 0x7c(t0)
		addiu	t2, 4
		
		lw		t1, 0x0(t2)
		sw		t1, 0x80(t0)
		addiu	t2, 4
		lw		t1, 0x0(t2)
		sw		t1, 0x84(t0)
		addiu	t2, 4
		lw		t1, 0x0(t2)
		sw		t1, 0x8c(t0)
		addiu	t2, 4
		lw		t1, 0x0(t2)
		sw		t1, 0x9c(t0)
#else
    nop
#endif
	.set	noat
    .endm
    
.macro	MEM_EXIT_STANDBY_M
#if	ENABLE_MEM_STANDBY
    .set    at
  # DRAM exit self refresh mode
		lui		t0, 0xb800
		lw		t1, 0x1030(t0)
		and     t1, ~0x01000000
		sw      t1, 0x1030(t0)
		nop
		nop

		and		t1, ~0x8000
		sw		t1, 0x1030(t0)
		nop
		
         jal     delay_time
         li      a0, 0x5000    
		or      t1, 0x80000000
		sw      t1, 0x1030(t0)
    .set    noat
#else
	nop
#endif
    .endm

    .text
    .set    noreorder
    .set    noat
    .align  4

.macro	RESET_SEE_M
    lui     t0, 0xb800	
    li      t1, 0xb8000280		
    sw      t1, 0x200(t0)
    lbu     t1, 0x220(t0)
    andi    t1, 0xfd       	
    sb      t1, 0x220(t0)
    .endm

.macro	BOOT_SEE_M
    .set    at
    lui     t0, 0xb800			
    lbu     t1, 0x220(t0)   	    
    ori     t1, 0x02       	
    sb      t1, 0x220(t0)
    li      t1, SEE_RUN_ADDR
    and     t1, 0x0fffffff
    or      t1, 0x80000000
    sw      t1, 0x200(t0)
    .set    noat
    .endm

    .macro	SAVE_MEM_CLOCK
    .set    at
     lui    t1, 0xb800
     lw     t2, 0x70(t1)
     li     t0, PM_MEM_CLK
     sw     t2 ,(t0)
    .set    noat
    .endm

    .macro	RESUME_MEM_CLOCK
    .set    at
    lui     t0, 0xb800
    lw      t1, 0x74(t0)
    and     t1 , ~((1<<4)|(1<<5)|(1<<6)) # clear bit[6:4]  
    
    li      t2, PM_MEM_CLK
    lw      t3 ,(t2)    
    and     t3 ,(1<<4)|(1<<5)|(1<<6)    # get mem clk seting
    or      t3 ,(1<<21)                 # trig
    or      t3 , t1
    sw      t3 , 0x74(t0)
    nop
    nop
    nop
    .set    noat
    .endm
    
/*
2013-9-23 mike
fix pm buring crash .
atcion:
make (volatile UINT32 *)0xb8000200 =0xabcXXXXX
to tell see it is pm send mailbox interrupt flag,
so see will return when enter mailbox interupt,
and not do remote call process.
*/
.macro	WAKE_UP_SEE
# enable mailbox interrupt
    .set    at 
    
    li      t0, 0xb800003f
    lbu     t1, (t0)
    ori     t1, 0xc0
    sb      t1, (t0)
    # li      t0, 0xb800003f
    # lbu     t1, (t0)
    # ori     t1, 0xc0
    # sb      t1, (t0)
# fill oxb800020c

    lui     t1 , 0xb800 
    li      t3 , PM_FLAG 
    sw      t3 , 0x20c(t1)

# send interupt to wake up see
    li      t0, 0xb8000037
    li      t1, 0x08
    sb      t1, (t0)   
    nop
    .set    noat
.endm

/*
    fucntion name   : cache_code(standby_key, ir_power, )
    used register   : t0,t1,t2,a0,a1
    global used     : s4,s5,s6,s7
    subroutine used : t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,s0,s1,s2,s3,a0,a1,a2,a3,v0,v1
    This function is to enter ddr self refresh, and wait for keypad or ir power.
    if down the standby key or ir power, leave ddr self refresh.
*/

LEAF(cache_code)  
#if REG_BACKUP_MEMORY
    li      t6, REG_BAK_0
#else
#if REG_BACKUP_IN_PMU    
    li      t6, REG_BAK_CPU
#else
    subu	sp, 256
    move    t6, sp
#endif
#endif
	sync
    	sw      $1, 1*4(t6)
	sw      $2, 2*4(t6)
	sw      $3, 3*4(t6)
	sw      $4, 4*4(t6)
	sw      $5, 5*4(t6)
	sw      $6, 6*4(t6)
	sw      $7, 7*4(t6)

	sw      $16, 16*4(t6)
	sw      $17, 17*4(t6)
	sw      $18, 18*4(t6)
	sw      $19, 19*4(t6)
	sw      $20, 20*4(t6)
	sw      $21, 21*4(t6)
	sw      $22, 22*4(t6)
	sw      $23, 23*4(t6)
	sw      $24, 24*4(t6)
	sw      $25, 25*4(t6)
	sw      $26, 26*4(t6)
	sw      $27, 27*4(t6)
	sw      $28, 28*4(t6)
	sw      $29, 29*4(t6)
	sw      $30, 30*4(t6)
	sw      $31, 31*4(t6)	

    move    s7, ra              # save ra to s7
    move    s6, a0              # save standby keypad to s6
    move    s5, a1              # save ir power key to s5
    move    s4, a2              # save the buffer address

#if (!UART_NOT_OFF && (ENABLE_PM_DEBUG || ENABLE_PRE_ENTER_DEBUG || ENABLE_LOOP_LOG))
    .set    at
    li      t1, 0xb8000060
    lw      s4, (t1)
    lw      t2, (t1)
    and     t2, 0xfffafff0
    sw      t2, (t1)
    jal     delay_time
    li      a0, 0x1000
    .set    noat
#endif

    /* Initialize time count */
    li      k0, TIME_COMPARE_1S
    mtc0	zero, C0_COUNT			# Setup CP0 counter register
    nop
    mtc0	k0, C0_COMPARE
    nop
#if CODE_IN_CACHE
    /* fill 16k code into i-cache */
    li	    t1, 0x4000
    la      t0, enter
1:
    .set    mips3
    cache   0x14, 0x0(t0)
    cache   0x14, 0x20(t0)
    sync
    .set    mips1
    addiu   t1, -64
    addiu   t0, 64
    bnez    t1, 1b
    nop
#endif

#if DATA_IN_CACHE
    li      t0, IR_RLC_BUFFER_START
    addiu   t1, t0, 0x2000          # Start addess + dcache_set_size
READ_ALL_D:
    lw      t2, 0x0(t0)
    .set    mips1
    addiu   t0, 4
    slt     t2, t0, t1
    bnez    t2, READ_ALL_D
    nop
#endif

#if CODE_IN_CACHE
    /* run in the i-cache */
    la      t0, enter
    sll     t0, 4
    srl     t0, 4
    lui     t1, 0x8000
    or      t0, t1
    jr      t0
    nop
#endif
    
enter:
    /* initialize the ir & clear the d-cache*/
    jal     init_ir
    nop

    jal     i2c_gpio_init_s   # I set gpio 134 for sda ,gpio 135 for scl ,but it will get gpio num from ui for next version
    nop 

    # init time count to zero
    li      t0, PM_SLEEP_TIME_CONST
    sw      zero ,0x0(t0)

#if DDR_INTEGRITY_CHECK

#if ENABLE_LOOP_LOG
    li      a0, 0x1
    jal     pm_uart_debug
    nop
#endif   

#if SYNC_WITH_SEE_ENABLE 
/*send flag to SEE, SEE will calculate the digest of main&see memory*/
    li      t6,  REG_BAK_CPU
    li      t5,  0xadead
    sw      t5,  (t6)
/*wait see caculate the digest of main&see when enter standby*/
2:
    lw      t5,  (t6)
    li      t4,  0xbdead
    bne     t5,  t4, 2b
    nop  
#endif
#endif

    SAVE_MEM_CLOCK
#if DDR_SELF_REFRESH
    MEM_ENTER_STANDBY_M
    MEM_ENTER_STANDBY_EX_M
    nop

    jal     delay_time
    li      a0, 0x20000
#endif



    GATE_SEE_CLK

    jal     delay_time
    li      a0, 0x1000

   #  PD_DOWN_ELECTRI
    jal     delay_time
    li      a0, 0x100
    
    SYS_ENTER_REAL_STANDBY
    /* change mem clk to 19.8M */
    MEM_ENTER_BYPASS_M      
    jal     delay_time
    li      a0, 0x100
    PLL_POWER_DOWN_M
    jal     delay_time
    li      a0, 0x100
    BOARD_POWER_DOWN_M

#if ENABLE_IR_CLOCK
	/* Enable IR clock */
    lui     t0, 0xb800
    lhu     t1, 0x62(t0)
    and     t1, 0xf7
    sb      t1, 0x62(t0)
#endif
    
loop:    
    /* delay some time : 40us */
    jal     delay_time
    li      a0, 0x1000

#if ENABLE_LOOP_LOG
    li      a0, 0x2
    jal     pm_uart_debug
    nop
#endif
    /* get the ir power key: if down the power, jump the loop */   
    jal     get_ir
    nop
    beq     v0, s5, leave
    nop

#if ENABLE_LOOP_LOG
    li      a0, 0x3
    jal     pm_uart_debug
    nop
#endif


#if  CH455_MODE
  li      a2, KEY_ADDR       #  it will get the value from ui later.
  or      a2 ,0x01    
  jal     i2c_gpio_read_s   
  nop  
         
  and     v0,  v0, 0xff    
  li      t0 , KEY_PRESS     
  beq     v0,  t0 , leave    
  nop   
#else
    /* Power key */
     li      a0, 0x9
    jal     pm_uart_debug
    nop
    jal     check_key_panel
    nop
    and     v0, v0, 0xff
    bnez    v0, leave
    nop
#endif


    SLEEP_TIME_CACULATE_M

#if ENABLE_TIMER_SUPPORT
    /* If timeout */
    li      t0, PM_SLEEP_TIMEOUT_CONST
    lw      t1, 0(t0)
    beqz    t1, loop
    nop
    # SLEEP_TIME_CACULATE_M
    li      t0, PM_SLEEP_TIMEOUT
    lw      t1, 0(t0)
    bnez    t1, loop
    nop
#endif

leave:
#if ENABLE_IR_CLOCK
    lui     t0, 0xb800
    lhu     t1, 0x62(t0)
    or      t1, 0x08
    sb      t1, 0x62(t0)
#endif
    # PD_UP_ELECTRI
    jal     delay_time
    li      a0, 0x100

#if ENABLE_LOOP_LOG
    jal     pm_uart_debug
    li      a0, 0x31
#endif
    /* disable IR interrupt */
    li      t0, SYS_IC_SB_BASE_H    
    li      t1, 0x00
    sb      t1, SYS_IC_SB_IRC_ISR(t0)
    li      t1, 0x00
    sb      t1, SYS_IC_SB_IRC_IER(t0)

    BOARD_POWER_UP_M

  

    jal     delay_time
    li      a0, 0x1000

    PLL_POWER_UP_M
    jal     delay_time
    li      a0, 0x100
    /* resume mem clk to normal */
    MEM_EXIT_BYPASS_M
    jal     delay_time
    li      a0, 0x10
    
 
    SYS_EXIT_REAL_STANDBY
   

    RESUME_SEE_CLK
    jal     delay_time
    li      a0, 0x10000

#if ENABLE_LOOP_LOG
    jal     pm_uart_debug
    li      a0, 0x33
#endif
#if DDR_SELF_REFRESH
    MEM_EXIT_STANDBY_EX_M
    MEM_EXIT_STANDBY_M
    nop
    jal     delay_time
    li      a0, 0x1000
#endif  
 
 
  /*
    s3503  send mail box interrupt to wake up see
  */
   WAKE_UP_SEE

#if SYNC_WITH_SEE_ENABLE
/*wait see caculate the digest of main&see when exit from standby*/
2:
    li      t6, REG_BAK_CPU
    lw      t0, (t6)
    li      t1, 0xcdead
    bne     t0, t1, 2b
    nop
#endif

#if ENABLE_LOOP_LOG
    jal     pm_uart_debug
    li      a0, 0x5
#endif

   jal pm_get_cost_time
   nop
    /* invalidate 4k d-cache all */
    jal     inval_dcache_all
    nop
    move    ra, s7


#if REG_BACKUP_MEMORY
    li      t6, REG_BAK_0
#else
#if REG_BACKUP_IN_PMU    
    li      t6, REG_BAK_CPU
#else
    move    t6, sp
#endif
#endif
        lw      $1, 1*4(t6)
	lw      $2, 2*4(t6)
	lw      $3, 3*4(t6)
	lw      $4, 4*4(t6)
	lw      $5, 5*4(t6)
	lw      $6, 6*4(t6)
	lw      $7, 7*4(t6)

	lw      $16, 16*4(t6)
	lw      $17, 17*4(t6)
	lw      $18, 18*4(t6)
	lw      $19, 19*4(t6)
	lw      $20, 20*4(t6)
	lw      $21, 21*4(t6)
	lw      $22, 22*4(t6)
	lw      $23, 23*4(t6)
	lw      $24, 24*4(t6)
	lw      $25, 25*4(t6)
	lw      $26, 26*4(t6)
	lw      $27, 27*4(t6)
	lw      $28, 28*4(t6)
	lw      $29, 29*4(t6)
	lw      $30, 30*4(t6)
	lw      $31, 31*4(t6)	
        jr      ra
#if ( (!REG_BACKUP_MEMORY)&&(!REG_BACKUP_IN_PMU) )
	addiu   sp, 256
#endif
    nop
END(cache_code)

LEAF(delay_time)
1:
    nop
    nop
    addiu   a0, -1
    bnez    a0, 1b
    nop

    jr	    ra
    nop
END(delay_time)

/*
    fucntion name   : flush_dcache_all()
    used register   : t0,t1,t2
    global used     : 
    subroutine used : 
*/
LEAF(flush_dcache_all)
    li      t0, IR_RLC_BUFFER_START
    addiu   t1, t0, 0x1000          # Start addess + dcache_set_size
FLUSH_ALL_D:
    .set    mips3
    cache   Index_Writeback_Inv_D, 0x0(t0)
    .set    mips1
    addiu   t0, 32
    slt     t2, t0, t1
    bnez    t2, FLUSH_ALL_D
    nop
    jr      ra
    nop

END(flush_dcache_all)

/*
    fucntion name   : inval_dcache_all()
    used register   : t0,t1,t2
    global used     : 
    subroutine used : 
*/
LEAF(inval_dcache_all)

    li      t0, IR_RLC_BUFFER_START
    addiu   t1, t0, 0x2000          # Start addess + dcache_set_size
INVAL_ALL_D:
    .set    mips3
    cache   Hit_Invalidate_D, 0x0(t0)
    .set    mips1
    addiu   t0, 32
    slt     t2, t0, t1
    bnez    t2, INVAL_ALL_D
    nop
    jr      ra
    nop

END(inval_dcache_all)

/*
    fucntion name: init_ir()
    used register: t0,t1,t2,t3,t4
    global used  : s2,s3
    This function is initialize the ir for the ir code run in the cache.
*/
LEAF(init_ir)
    /* init ir related system registers */
    li      t0, SYS_IC_SB_BASE_H
    sb      zero, SYS_IC_SB_IRC_CFG(t0)     # Clear CR

#if ENABLE_NONDEEP_IR
    li      t1, 0x9c
#else
    li      t1, 0x84                        # Enable IRC : 0x84/0x9c
#endif
    sb      t1, SYS_IC_SB_IRC_CFG(t0)
    li      t1, 0xa0                        # Init FIFO, threshould is 32 bytes
    sb      t1, SYS_IC_SB_IRC_FCTRL(t0)

    li      t1, 0x05                        # Timeout: 6*128*CLKwork
    sb      t1, SYS_IC_SB_IRC_TTHR(t0)
    li      t1, 0x01                        # Noise filer: 1*CLKwork
    sb      t1, SYS_IC_SB_IRC_NTHR(t0)

    li      t1, 0x03                        # Enable all interrupt
    sb      t1, SYS_IC_SB_IRC_IER(t0)
    li      t1, 0x03                        # Clear all interrupt
    sb      t1, SYS_IC_SB_IRC_ISR(t0)
    
    /* clear d-cache buffer for ir decoder */
    li      t0, IR_RLC_BUFFER_START
    li      t1, IR_CACHE_END
1:
    sb      zero, (t0)
    sltu    t4, t0, t1
    bnez    t4, 1b
    addiu   t0, 1
    
    /* copy ir pattern to d-cache */
    la      t0, IR_ROM_PATTERN
    la      t3, IR_ROM_PATTERN_END
    li      t1, IR_PATTERN_START
1:
    lbu     t2, (t0)
    sb      t2, (t1)
    addiu   t0, 1
    addiu   t1, 1
    sltu    t4, t0, t3
    bnez    t4, 1b
    nop

    /* copy ir pattern count to d-cache */
    la      t0, IR_ROM_PATTERN_CNT_LIST
    li      t3, IR_TYPE_NUM
    addu    t3, t0
    li      t1, IR_PATTERN_CNT_START
1:
    lbu     t2, (t0)
    sb      t2, (t1)
    addiu   t0, 1
    addiu   t1, 1
    sltu    t4, t0, t3
    bnez    t4, 1b
    nop

    /* copy ir attribute to d-cache */
    la      t0, IR_ROM_ATTR
    la      t3, IR_ROM_ATTR_END
    li      t1, IR_ATTR_START
1:
    lbu     t2, (t0)
    sb      t2, (t1)
    addiu   t0, 1
    addiu   t1, 1
    sltu    t4, t0, t3
    bnez    t4, 1b
    nop

    /* init ir module global used registers */
    li      s2, 0               # RLC data buffer write pointer
    li      s3, 0               # RLC data buffer read pointer

    jr      ra
    nop
END(init_ir)

/*
    fucntion name   : get_ir()
    v0 return key code
    used register   : t0,t1,t2,t3,t4,t5,t6,t7,t8,t9
    global used     : s0,s1,s2,s3,v0,v1
    subroutine used : a0,a1,a2,a3

    s2: RLC data in cache write pointer
    s3: RLC data in cache read pointer
    s1: save decoder module such as RC6 / NEC etc. 
    s0 : save ra value when call noleaf function
    This function is to get the code from ir.
*/

LEAF(get_ir) 
    /* init regs needed */ 
    li      t0, 0
    li      t1, 0
    li      t2, 0
    li      t3, 0
    li      t4, 0
    li      t5, 0
    li      t6, 0
    li      t7, 0
    li      t8, 0
    li      t9, 0
    nop

    move    v1, ra                      # Save ra to v1

    li      t2, IR_RLC_BUFFER_WTMRK
    addu    t2, s3                      # t2 is the write mark

    li      t0, SYS_IC_SB_BASE_H
    lbu     t5, SYS_IC_SB_IRC_ISR(t0)   # Store current interrupt status to t5
    sb      t5, SYS_IC_SB_IRC_ISR(t0)   # Clear interrupt

fetch_fifo:
    lbu     t1, SYS_IC_SB_IRC_FCTRL(t0) # Get IR RLC data number in FIFO 
    and     t1, 0x7f                    # t1 is the fifo data counter
    bnez    t1, 1f                      # If no IR RLC data, check timeout
    nop
    and     t5, 0x02                    # Check timeout?
    bnez    t5, 1f
    nop
 
    li      v0, 0                       # no timeout & fifo is empty, return 0
    move    ra, v1
    jr      ra
    nop

1:
    li      t3, IR_RLC_BUFFER_START     # if fifo is not empty
    addu    t3, s2
    lbu     t4, SYS_IC_SB_IRC_DATA(t0)  # Read data in FIFO & save to RAM (cache)
    sb      t4, (t3)
    addiu   s2, 1
    addiu   t1, -1

    li      t4, IR_RLC_BUFFER_SIZE
    sltu    k1, s2, t4
    bnez    k1, 2f    
    nop
    addiu   s2, -IR_RLC_BUFFER_SIZE     # if write pointer > buffer size 
2:                                      # if write pointer < buffer size
    beqz    t1, fetch_fifo              # if t1 is empty, goto fetch_fifo
    nop

    move    t4, s2                      # Check whether reach watermark?
    slt     k1, s3, t4
    bnez    k1, 3f      
    nop
    addiu   t4, IR_RLC_BUFFER_SIZE
3:
    slt     k1, t2, t4
    bnez    k1, accum_pulse         # if write pointer < watermark, wait
    nop
    
    bnez    t1, 1b
    nop

    b       fetch_fifo                  # Re-check RLC fifo
    nop

accum_pulse: 
    li      v0 , 0 
    bne     s3, s2, 1f
    nop
    
    li      v0, 0                       # if s2 == s3, return 0
    move    ra, v1
    jr      ra
    nop

1:
    li      t1, IR_RLC_BUFFER_START
    addu    t1, s3
    lbu     t2, (t1)                    # t2 is the data

    addiu   s3, 1
    li      t1, IR_RLC_BUFFER_SIZE
    slt     k1, s3, t1
    bnez    k1, 1f
    nop
    addiu   s3, -IR_RLC_BUFFER_SIZE     

1:
    and     t3, t2, 0x7F           # clear FIFO bit 7 save to t3 , so t3 is the pulse width!
    li      t1, IR_PLS_SUM   # Load pulse width sum from cache
    lh      t4, (t1)
    and     t1, t4, 0xFFFF
    addu    t1, t3
    li      t5, 0x10000                 # max pulse width is 0x10000 , save in t5
    slt     k1, t1, t5
    bnez    k1, 1f    
    nop
    #sdbbp                               # Only support no more than 2^16 RLC sample
1:
    addu    t4, t3
    li      t1, IR_PLS_SUM              # save pulse width sum to d-cache
    sh      t4, (t1)                    # t4 is the pulse width sum
    
    beq     s2, s3, start_decode
    nop

    and     t2, 0x80
    li      t1, IR_RLC_BUFFER_START
    addu    t1, s3
    lbu     t3, (t1)
    and     t3, 0x80
    xor     t2, t3                      # Check current/next RLC polarity same?
    beqz    t2, accum_pulse             # if it is the same, goto start_decode
    nop

start_decode:
    li      t0, IR_PATTERN_START
    li      t1, IR_PATTERN_CNT_START
    addu    t1, t9                      # t9 is the pattern type: NEC
    lbu     t2, (t1)
    addu    t6, t0, t2                  # t6 is the current pattern

    li      t1, IR_ATTR_START
    li      t2, IR_ATTR_SIZE
    multu   t9, t2
    mflo    t2
    addu    t7, t1, t2                  # t7 is the current attribute
    
    li      t2, IR_DECODER_START
    li      t3, IR_DECODER_SIZE
    multu   t9, t3
    mflo    t3
    addu    t8, t2, t3                  # t8 is the currnt decoder
    
    lbu     t1, DECODE_STEP(t8)
    addu    t1, t6
    lbu     t5, (t1)                    # t5 is the decoder step

    lbu     t0, FIRST_HALF_GOT(t8)
    bnez    t0, 2f
    nop

    li      t0, IR_LEADING              # For leading pulse should check first half
    bne     t0, t5, 1f                  # if first half is 0, check the step
    nop

    and     a0, t4, 0xFFFF              # if in leading step

    addiu   t0, t7, PULSE_LEADING       # handle the pulse leading
    lhu     a1, PULSE_LOW(t0)
    lhu     a2, PULSE_TORL(t0)
    jal     in_range
    nop
    beqz    v0, err_clear
    nop

1:
    and     t4, 0xFFFF                  # if not in leading step
    sw      t4, LAST_PULSE_WIDTH(t8)

    li      t0, 1
    sb      t0, FIRST_HALF_GOT(t8)
    b       end_decode
    nop

2:
    sb      zero, FIRST_HALF_GOT(t8)    # if first half is not 0

    lw      a0, LAST_PULSE_WIDTH(t8)    
    and     t4, 0xFFFF
    addu    a0, t4                      # a0 is the pulse width sum

    move    t1, t5
    li      t0, IR_REPEAT_LEADING       
    slt     k1, t1, t0
    bnez    k1, 1f     
    nop
    addiu   t1, 1                       # if step >= repeat leading

1:
    sll     t0, t1, 3                   # see the struct ir_attr before
    addu    t0, t7
    addiu   t0, PULSE_LEADING
    lhu     t1, PULSE_LOW(t0)
    lhu     t2, PULSE_HIGH(t0)
    addu    a1, t1, t2
    lhu     a2, PULSE_TORL(t0)

    jal     in_range
    nop

    li      t0, IR_DATA                
    beq     t0, t5, 1f
    nop

    beqz    v0, err_clear               # if not in data step
    nop
    b       2f
    nop

1:
    li      t0, 0
    bnez    v0, 1f                      # if in data step & v0 not equal 0, t0=0 and jump
    nop

    addiu   t0, t7, PULSE_DATA1
    lhu     t1, PULSE_LOW(t0)
    lhu     t2, PULSE_HIGH(t0)
    addu    a1, t1, t2
    lhu     a2, PULSE_TORL(t0)

    jal     in_range
    nop

    beqz    v0, err_clear
    nop

    li      t0, 1                       # if in data step & v0 equal 0, t0=1

1:
    lbu     t2, KEY_BIT_CNT(t8)         # t2 is the key bit count
    beqz    t0, 1f                      
    nop
    and     t3, t2, 0x7
    sllv    t0, t0, t3
    srl     t4, t2, 3
    addiu   t4, KEY_CODE
    addu    t4, t8
    lbu     t3, (t4)
    or      t0, t3
    sb      t0, (t4)
1:
    addiu   t2, 1
    sb      t2, KEY_BIT_CNT(t8)

2:
    lbu     t0, DECODE_STEP(t8)
    addiu   t0, 1
    sb      t0, DECODE_STEP(t8)
    addu    t0, t6
    lbu     t1, (t0)
    li      t2, IR_END
    bne     t1, t2, end_decode
    nop

    /* new key code got */
    lhu     t0, (t7)
    and     t1, t0, BIT_MSB_FIRST
    beqz    t1, 1f
    nop
    
    addiu   a0, t8, KEY_CODE            # if it is bit msb first
    addiu   a1, t8, LAST_KEY_CODE
    lbu     a2, KEY_BIT_CNT(t8)

    jal     reverse_code_bit
    nop

    addiu   a0, t8, LAST_KEY_CODE
    addiu   a1, t8, KEY_CODE
    lbu     a2, KEY_BIT_CNT(t8)
    addiu   a2, 7
    srl     a2, 3
    jal     copy_code
    nop

1:
    lhu     t0, (t7)
    and     t1, t0, BYTE_MSB_FIRST
    beqz    t1, 1f
    nop

    addiu   a0, t8, KEY_CODE            # if it is byte msb first
    addiu   a1, t8, LAST_KEY_CODE
    lbu     a2, KEY_BIT_CNT(t8)

    jal     reverse_code_byte
    nop

    addiu   a0, t8, LAST_KEY_CODE
    addiu   a1, t8, KEY_CODE
    lbu     a2, KEY_BIT_CNT(t8)
    addiu   a2, 7
    srl     a2, 3
    jal     copy_code
    nop

1:
    addiu   t0, t8, KEY_CODE
    lw      t1, (t0)

    move    a0, t8
    li      a1, IR_DECODER_SIZE
    jal     clear_buffer
    nop

    move    v0, t1    
return_to:
    li      s2, 0
    li      s3, 0
    move    ra, v1
    jr      ra
    nop

err_clear:
    move    a0, t8
    li      a1, IR_DECODER_SIZE
    jal     clear_buffer
    nop

end_decode:
    addiu   t9, 1
    li      t0, IR_TYPE_NUM
    slt     k1, t9, t0
    bnez    k1, start_decode
    nop
    li      t9, 0
    li      t1, IR_PLS_SUM
    sh      zero, (t1)
    b       accum_pulse   # Re-start acc next segment of waveform
    nop

END(get_ir)


/*
    fucntion name   : in_range(a0, a1, a2)
    a0  input value
    a1  absolute value
    a2  tolerance
    used register   : v0,a0,a1,a2,a3
*/
LEAF(in_range)
    slt     v0, a1, a0              # v0 = (a1 < a0) ? 1 : 0
    bnez    v0, 1f
    nop
    b       2f
    sub     a3, a1, a0              # a1 > a0
1:
    sub     a3, a0, a1              # a1 < a0
2:
    jr      ra
    slt     v0, a3, a2              # If |a1 - a0| < a2, reture 1, else 0
END(in_range)

/*
    function name   : copy_code(a0, a1, a2)
    a0  input key code (pointer)
    a1  output key code (pointer)
    a2  byte count
    used register   : v0,a0,a1,a2
*/
LEAF(copy_code)
1:
    lbu     v0, (a0)
    sb      v0, (a1)
    addiu   a0, 1
    addiu   a1, 1
    addiu   a2, -1
    bnez    a2, 1b
    nop
    jr      ra
    nop
END(copy_code)

/*
    function name   : clear_buffer(a0, a1)
    a0  buffer addresss
    a1  length in byte
    used register   : a0,a1
*/
LEAF(clear_buffer)
1:
    sb      zero, (a0)
    addiu   a0, 1
    addiu   a1, -1
    bnez    a1, 1b
    nop
    jr      ra
    nop
END(clear_buffer)

/*
    function name   : reverse_code_bit(a0, a1, a2)
    a0  input key code (pointer)
    a1  output key code (pointer)
    a2  bit count
    used register   : v0,a0,a1,a2,a3,t0,t1,t2
*/
LEAF(reverse_code_bit)
    sw      zero, (a1)  # Clear output key code buffer
    addiu   a1, 4
    sw      zero, (a1)
    addiu   a1, 4
    sw      zero, (a1)
    addiu   a1, 4
    sw      zero, (a1)
    addiu   a1, 4
    sw      zero, (a1)
    addiu   a1, -16

    move    t0, zero
1:
    addiu   a2, -1
    srl     a3, a2, 3
    and     v0, a2, 0x07
    addu    a3, a0
    lbu     t2, (a3)
    li      t1, 1
    sllv    v0, t1, v0
    and     v0, t2
    beqz    v0, 2f
    nop
    srl     a3, t0, 3
    and     v0, t0, 0x07
    addu    a3, a1
    lbu     t2, (a3)
    li      t1, 1
    sllv    v0, t1, v0
    or      v0, t2
    sb      v0, (a3)
2:
    bnez    a2, 1b
    addiu   t0, 1
    jr      ra
    nop
END(reverse_code_bit)

/*
    function name   : reverse_code_byte(a0, a1, a2)
    a0  input key code (pointer)
    a1  output key code (pointer)
    a2  bit count
    used register   : v0,a0,a1,a2,a3
*/
LEAF(reverse_code_byte)
    sw      zero, (a1)  # Clear output key code buffer
    addiu   a1, 4
    sw      zero, (a1)
    addiu   a1, 4
    sw      zero, (a1)
    addiu   a1, 4
    sw      zero, (a1)
    addiu   a1, 4
    sw      zero, (a1)
    addiu   a1, -16

    addiu   a2, 7
    srl     a2, 3
1:
    addiu   a2, -1
    addu    v0, a2, a0
    lbu     a3, (v0)
    sb      a3, (a1)
    bnez    a2, 1b
    addiu   a1, 1
    jr      ra
    nop
END(reverse_code_byte)

/*
 *  Get the panel key
 */
LEAF(check_key_panel)
    .set    at
	lui     t0, 0xb800
	lbu		t1, 0x36(t0)
	and		v0, t1, 0x10

	# clear key status
	lui		t0, 0xb805
	lbu		t1, 0xe012(t0)
	sb		t1, 0xe012(t0)
	nop
	nop
	nop

	jr		ra
	nop
	.set    noat
END(check_key_panel)

/***************************************************************
*Function Name:	_i2c_gpio_bit_set
*Arguments: 	a0: gpio num
*			a1: 0->low, 1->high
*			v0: 0->success, -1->fail
*Description:	Set a bit while gpio dir's out.
*Notice:		Useing t0-t2, v0, a0, a1, no sub-functions.
***************************************************************/
	.global	_i2c_gpio_bit_set
	.ent		_i2c_gpio_bit_set
	.set		at
_i2c_gpio_bit_set:                   /******dont set gpio dir*************/
	li		t0, 32
	bge		a0, t0, 64f
	nop
32:
	li		t0, HAL_GPIO_DO_REG  # process 0-31
	lw		t1, (t0)
	b		1f
	nop
	
64:
	li		t0, 64
	bge		a0, t0, 96f
	nop

	li		t0, HAL_GPIO1_DO_REG  # process 32-63
	lw		t1, (t0)
	b		1f
	nop
	
96:
	li		t0, 96
	bge		a0, t0, 128f
	nop
	
	li		t0, HAL_GPIO2_DO_REG  # process 64-95
	lw		t1, (t0)
	b       1f
	nop

128:
	li		t0, 128
	bge		a0, t0, 160f
	nop
	
	li		t0, HAL_GPIO3_DO_REG  # process 96-127
	lw		t1, (t0)
	b       1f	
	nop

160:
	li		t0, 160
	bge		a0, t0, 192f
	nop
	
	li		t0, HAL_GPIO4_DO_REG  # process 128-159
	lw		t1, (t0)
1:
	li		    t2, 0x1
	sllv		t2, t2, a0					##shift left logically by variable

	bnez		a1, 2f
	nop
	
	not		t2, t2					##set 0
	and		t1, t2
	sw		t1, (t0)	
	b		3f
	nop
	
2:									##set 1
	or		t1, t2
	sw		t1, (t0)
	
3:									
	jr		ra
	li		v0, 0
	
192:
	jr		ra
	li		v0, -1

	.set noat
	.end _i2c_gpio_bit_set


/***************************************************************
*Function Name:	_i2c_gpio_set_enable
*Arguments: 	a0: gpio num
*			a1: 0->dis, 1->enable
*			v0: 0->success, -1->fail
*Description:	Set a bit while gpio dir's out.
*Notice:		Useing t0-t2, v0, a0, a1, no sub-functions.
***************************************************************/
	.global	_i2c_gpio_set_enable
	.ent		_i2c_gpio_set_enable
	.set		at
_i2c_gpio_set_enable:                   /******dont set gpio dir*************/
	li		t0, 32
	bge		a0, t0, 64f
	nop
32:
	li		t0, HAL_GPIO_EN_REG
	lw		t1, (t0)
	b		1f
	nop
	
64:
	li		t0, 64
	bge		a0, t0, 96f
	nop

	li		t0, HAL_GPIO1_EN_REG
	lw		t1, (t0)
	b		1f
	nop
	
96:
	li		t0, 96
	bge		a0, t0, 128f
	nop
	
	li		t0, HAL_GPIO2_EN_REG
	lw		t1, (t0)
	
1:
	li		    t2, 0x1
	sllv		t2, t2, a0					##shift left logically by variable

	bnez		a1, 2f
	nop
	
	not		t2, t2					##set 0
	and		t1, t2
	sw		t1, (t0)	
	b		3f
	nop
	
2:									##set 1
	or		t1, t2
	sw		t1, (t0)
	
3:									
	jr		ra
	li		v0, 0
	
128:
	jr		ra
	li		v0, -1

	.set    noat
	.end _i2c_gpio_set_enable	


/***************************************************************
*Function Name:	_i2c_gpio_bit_get
*Arguments: 	a0: gpio num
*			v0: 0->low, 1->high, -1->fail
*Description:	get a bit while gpio dir's in.
*Notice:		useing t0-t1, v0, a0, no sub-functions.
***************************************************************/
	.global	_i2c_gpio_bit_get
	.ent		_i2c_gpio_bit_get
	.set		at
_i2c_gpio_bit_get:                /******dont set gpio dir*************/
	li		t0, 32
	bge		a0, t0, 64f
	nop
32:	
	li		t0, HAL_GPIO_DI_REG
	lw		t1, (t0)
	b		1f
	nop
	
64:
	li		t0, 64
	bge		a0, t0, 96f
	nop

	li		t0, HAL_GPIO1_DI_REG
	lw		t1, (t0)
	b		1f
	nop
	
96:
	li		t0, 96
	bge		a0, t0, 128f
	nop
	li		t0, HAL_GPIO2_DI_REG
	lw		t1, (t0)
    b       1f
    nop
    
128:
	li		t0, 128
	bge		a0, t0, 160f
	nop
	li		t0, HAL_GPIO3_DI_REG
	lw		t1, (t0)
    b       1f
    nop   
    
160:
    li		t0, 160
    bge		a0, t0, 192f
    nop
    li		t0, HAL_GPIO4_DI_REG
    lw		t1, (t0)

1:
	srlv	t1, t1, a0
	and		v0, t1, 0x01
	jr		ra
	nop
	
192:
	jr		ra
	li		v0, -1

	.set     noat
	.end _i2c_gpio_bit_get

/***************************************************************
*Function Name:	_i2c_gpio_dir_set
*Arguments: 	a0: gpio num
*			a1: 0->in, 1->out
*			v0: 0->success, -1->fail			
*Description:	set GPIO direction.
*Notice:		useing t0-t2, v0, a0, a1, no sub-functions.
a1:dir
a0,gpio_num
***************************************************************/
    .global	_i2c_gpio_dir_set
	.ent		_i2c_gpio_dir_set
	.set		at               /****************c3701 is error dir***************/
# only M37XX will go this way 
# because the dir reg is opposide	                                    
_i2c_gpio_dir_set:
    and     a1, 0x1
    lw      t0, 0xb8000000
    and     t0, 0xffff0000
    bne     t0, 0x37010000, 2f
    nop
    li      t0, 1
    xor     a1, t0, a1
2:
	li		t0, 32
	bge		a0, t0, 64f
	nop
	
32:	
	li		t0, HAL_GPIO_DIR_REG  # process 0-31
	lw		t1, (t0)
	b		1f
	nop

64:
	li		t0, 64
	bge		a0, t0, 96f
	nop
	
	li		t0, HAL_GPIO1_DIR_REG  # process 32-63 
	lw		t1, (t0)
	b		1f
	nop

96:
	li		t0, 96
	bge		a0, t0, 128f
	nop

	li		t0, HAL_GPIO2_DIR_REG  # process 64-95
	lw		t1, (t0)
	b       1f
	nop


128:
	li		t0, 128
	bge		a0, t0, 160f
	nop
	
	li		t0, HAL_GPIO3_DIR_REG  # process 96-127 
	lw		t1, (t0)
	b		1f
	nop

160:
	li		t0, 160
	bge		a0, t0, 192f
	nop
	
	li		t0, HAL_GPIO4_DIR_REG  # process 128-159 
	lw		t1, (t0)

1:
	li		    t2, 0x1
	sllv		t2, t2, a0					##shift left logically by variable
	bnez		a1, 2f
	nop
 	not		t2, t2					##set 0
	and		t1, t2
	sw		t1, (t0)
	b		3f
	nop
2:									##set 1
	or		t1, t2
	sw		t1, (t0)
3:									
	jr		ra
	li		v0, 0

192:
	jr		ra
	li		v0, -1

	.set noat
	.end _i2c_gpio_dir_set
/***************************************************************
*Function Name:	_i2c_gpio_start_phase
*Arguments:  	v0: 0->success, -1->fail
*
*Description:	Generate Start Condition
*Stream Format:
*			SCL	____/------\_
*			SDA	_/------\____
*Notice:		Store ra in t3, so not use t3 in sub-functions.
***************************************************************/
	.global	_i2c_gpio_start_phase
	.ent		_i2c_gpio_start_phase
	.set		at
_i2c_gpio_start_phase:
	move	v1, ra     			##store ra in v1
	
	li		a1, 1					##set SDA output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SDA
	
	li		a1, 1					##set SCL output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SCL

	li		a1, 1					##set SDA high
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SDA

1:
	li		a1, 1					##set SCL high
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL

    jal     delay_time
    li      a0, 0x200
    
	jal		_i2c_gpio_bit_get			##get SDA
	li		a0, I2C_GPIO_SDA			
	beqz	v0, 2f
	nop	

	li		a1, 0					##set SDA low
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SDA

	jal     delay_time
    li      a0, 0x200				##delay half period
    
	li		a1, 0					    ##set SCL low
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL

	jal     delay_time
    li      a0, 0x200				##delay half period

	move	ra, v1					    ##fetch ra from v1
	jr		ra
	li		v0, 0					    ##success

2:	
	move	ra, v1					    ##fetch ra from v1
	jr		ra
	li		v0, -1					     ##fail
	
	.set    noat
	.end _i2c_gpio_start_phase

/***************************************************************
*Function Name:	_i2c_gpio_stop_phase
*Arguments:	v0: 0->success, -1->fail
*
*Description:	Generate Stop Condition
*Stream Format:
*			SCL	____/-------
*			SDA	_______/----
*Notice:		Store ra in t3, so not use t3 in sub-functions.
***************************************************************/
	.global	_i2c_gpio_stop_phase
	.ent		_i2c_gpio_stop_phase
	.set		at
_i2c_gpio_stop_phase:
	move	v1, ra					##store ra in v1.
	
	li		a1, 1					##set SDA output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SDA
	
	li		a1, 1					##set SCL output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SCL
	
	li		a1, 0					##set SDA low
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SDA
	
	jal     delay_time
    li      a0, 0x200				##delay half period

	li		a1, 1					##set SCL high
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL

	jal     delay_time
    li      a0, 0x200				##delay half period

	li		a1, 1					##set SDA high
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SDA
									##may need to delay here.
									
	jal		_i2c_gpio_bit_get			##get SDA
	li		a0, I2C_GPIO_SDA			
	beqz	v0, 1f
	nop	

	move	ra, v1					##fetch ra from v1
	jr		ra
	li		v0, 0					##success

1:	
	move	ra, v1					##fetch ra from v1
	jr		ra
	li		v0, -1					##fail
	
	.set		noat
	.end		_i2c_gpio_stop_phase


/***************************************************************
*Function Name:	_i2c_gpio_set_byte
*Arguments: 
*			a2:	data
*			v0:	ack from slave(0->success, 1->fail)
*Description:	send a byte to i2c bus.
*			Using t3, t5, t7 as global variables:
*				t3->store ret addr			
*				t5->store shifted data
*				t7->store shifting times(8 times)
***************************************************************/
	.global	_i2c_gpio_set_byte
	.ent		_i2c_gpio_set_byte
	.set		at
_i2c_gpio_set_byte:
	move	v1, ra

##Send a byte bit after bit.	
	move	t5, a2
	
	li		t7, 7						##bit7-0
	
	li		a1, 1					##set SDA output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SDA
	
	li		a1, 1					##set SCL output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SCL
1:	
	li		a1, 0					##set SCL low
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL	
	
	andi		t2, t5, 0x80
	srl		    t2, 7
	bnez		t2, 2f
	nop

	b		3f
	li		a1, 0 					##set 0
2:	
	li		a1, 1					##set 1
3:	
	jal		_i2c_gpio_bit_set
	li		a0, I2C_GPIO_SDA

	li		a1, 1					##set SCL high
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL	

	jal     delay_time
    li      a0, 0x200				##delay half period

	li		a1, 0					##set SCL low
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL	

	sll		t5, 1
	bnez	t7, 1b
	sub		t7, 1


##Get ack.
    li		a1, 1					##set SDA high
	jal		_i2c_gpio_bit_set
	li		a0, I2C_GPIO_SDA
	
	li		a1, 0					##set SDA input
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SDA
	
	jal     delay_time
    li      a0, 0x200				##delay half period

	li		a1, 1					  ##set SCL high
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL	

	jal     delay_time
    li      a0, 0x200				##delay half period

	jal		_i2c_gpio_bit_get			##get SDA
	li		a0, I2C_GPIO_SDA

	move 	t7, v0
	
	li		a1, 0					##set SCL low
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL	

    li		a1, 1					##set SDA output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SDA

	move	v0,	t7
	move	ra,	v1
	jr		ra
	nop
	
	.set		noat
	.end		_i2c_gpio_set_byte
/***************************************************************
*Function Name:	_i2c_gpio_get_byte
*Arguments: 
*			v0:	data
*Description:	receive a byte to i2c bus.
*			Using t3, t5, t7 as global variables:
*				t3->store ret addr			
*				t5->store shifted data
*				t7->store shifting times(8 times)
***************************************************************/
	.global	_i2c_gpio_get_byte
	.ent		_i2c_gpio_get_byte
	.set		at
_i2c_gpio_get_byte:
	move	v1, ra

	li		a1, 0					##set SDA input
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SDA

	li		a1, 1					##set SCL output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SCL
	
##get data
	li		t7, 7						##bit7-0
	li      t5, 0                       # clear get date temp regist
1:
	li		a1, 1					##set SDA high
	jal		_i2c_gpio_bit_set
	li		a0, I2C_GPIO_SDA	
	
	jal     delay_time
    li      a0, 0x200				##delay half period

	li		a1, 1					##set SCL high
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL	

	jal     delay_time
    li      a0, 0x200				##delay half period

	jal		_i2c_gpio_bit_get			##get SDA
	li		a0, I2C_GPIO_SDA

	or	 	t5, v0

	li		a1, 0					##set SCL low
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL	

	sll		t5, 1
	bnez		t7, 1b
	sub		t7, 1	
	srl		t5, 1						##shift left 8 times right 1 time
	
##send ack
	li		a1, 1					##set SDA output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SDA
	
	li		a1, 0					##set SCL low
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL		

	li		a1, 0 					##ack: set SDA low
	jal		_i2c_gpio_bit_set
	li		a0, I2C_GPIO_SDA

	jal     delay_time
    li      a0, 0x200				##delay half period

	li		a1, 1					##set SCL high
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL	

	jal     delay_time
    li      a0, 0x200				##delay half period

	li		a1, 0					##set SCL low
	jal		_i2c_gpio_bit_set			
	li		a0, I2C_GPIO_SCL	

	move	v0, t5
	move	ra, v1
	jr		ra
	nop

	.set		noat
	.end		_i2c_gpio_get_byte


/***************************************************************
*Function Name:	i2c_gpio_init_s
*Arguments: 	
*Description:	
*Notice:		Store ra in t3, so not use t3 in sub-functions.
***************************************************************/
	.global	i2c_gpio_init_s
	.ent		i2c_gpio_init_s
	.set		at
i2c_gpio_init_s:	
	move	t3, ra					##store ra in t3.

    lw      t0 , HAL_GPIO4_EN_REG   ## enable gpiob [7,6]
	or      t0 , (1<<6)|(1<<7)
   	
	sw      t0 , HAL_GPIO4_EN_REG  

    li		a1, 1					##set SCL output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SCL

	li		a1, 1					##set SDA output
	jal		_i2c_gpio_dir_set			
	li		a0, I2C_GPIO_SDA

	move	ra, t3					##fetch ra from t3
	jr		ra
	nop
	
	.set		noat
	.end		i2c_gpio_init_s


/***************************************************************
*Function Name:	i2c_gpio_read_s
*Arguments: 	a2:	7 it slave address   
*			    v0:	data 
*Description:	read a byte per time
*			Store ra in t4, retry count in t6, So not use t4 and t6 in this func or its sub-funcs.
*			Should not use t3, t5, t7 as global variables in this function.
***************************************************************/
	.global	i2c_gpio_read_s
	.ent		i2c_gpio_read_s
	.set		at
i2c_gpio_read_s:
	move	t4, ra
1:
	jal		_i2c_gpio_start_phase		##START
	nop
	or       a2 ,0x01               ##set read mode 7 bit slave|1  bit 0 =1 means will read this address
	jal		_i2c_gpio_set_byte		##set addr in a2
	nop
	beqz	v0, 2f              #means ack ok
	nop

	jal		_i2c_gpio_stop_phase		##STOP
	nop

	jal     delay_time
    li      a0, 0x200				##delay half period
2:
	jal		_i2c_gpio_get_byte		    ##get byte
	nop
	move	t7, v0
	jal		_i2c_gpio_stop_phase		##STOP
	nop
	move	v0, t7

	b		4f
	nop
	
3:
	li		v0, -1
	
4:
	move	ra, t4
	jr		ra
	nop
	.set		noat
	.end		i2c_gpio_read_s


/*
 *	void pm_uart_debug(UINT32 data)
 *  a0: print data
 *	print debug information
 *  be careful: the value of t0 will be corrupted since wen don't have stack to use now
 */
    .globl  pm_uart_debug
    .ent    pm_uart_debug
pm_uart_debug:	
    .set    at
	
#if ENABLE_LOOP_LOG
    addiu   a0, 0x30
    li      t6, 0xb8018300
    sb      a0, 0(t6)
#endif
    nop

    jr	    ra
    nop
    .set    noat
	.end	pm_uart_debug

    .globl  ddr_uart_debug
    .ent    ddr_uart_debug
ddr_uart_debug:	
    .set    at
    move      t0, a0
    li      t1, 0x4
    li      t6, 0xb8018300
write_all:
    lw      t2, (t0)
    li      t3, 28
    li      t4, 8
write_w: 
    li      t7, 0x2000
1:
    nop
    nop
    addiu   t7, -1
    bnez    t7, 1b
    nop 	
    
    srl     t5, t2,  t3
    andi    t5, 0xf
    bgeu    t5, 0xa, 2f
    nop
    addiu   t5, 0x30
    b       3f
    nop
2:
    addiu   t5, 0x57
3:
    sb      t5, 0(t6)
    nop
    addiu   t3, -4
    addiu   t4, -1
    bnez    t4, write_w
    nop
    li      t7, 0x2000
1:
    nop
    nop
    addiu   t7, -1
    bnez    t7, 1b
    nop 	
    li      t2, 0x0D
    sb      t2, 0(t6)
    li      t2, 0x0A
    sb      t2, 0(t6)
    addiu   t0, 4
    addiu   t1, -4
    bnez    t1, write_all
    nop       
    jr	    ra
    nop
    .set    noat
	.end	ddr_uart_debug

    .globl  pm_get_cost_time
    .ent    pm_get_cost_time
pm_get_cost_time:	
    .set    at
    la      t0 , pm_cost_sec


    li      t1 , PM_SLEEP_TIME_CONST
    lw      t2 , 0x0(t1)

    sw      t2 , 0x0(t0)
    
    jr	    ra
    nop
    .set    noat
	.end	pm_get_cost_time

    .globl  pm_standby_cost_sec
    .ent    pm_standby_cost_sec
pm_standby_cost_sec:
    .set    at
    la      t0 , pm_cost_sec
    lw      t1 , 0(t0) 

    move    v0 , t1
    
    jr	    ra
    nop
    .set    noat
	.end	pm_standby_cost_sec

.data
pm_cost_sec:.word 0	
