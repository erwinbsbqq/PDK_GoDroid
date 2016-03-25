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
#include "m36_sleep.h"

#define ENABLE_PM_DEBUG         1



	.text
	.set    noreorder
	.align  4


/*
fucntion name   : cache_code(standby_key, ir_power, )
used register   : t0,t1,t2,a0,a1
global used     : s4,s5,s6,s7
subroutine used : t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,s0,s1,s2,s3,a0,a1,a2,a3,v0,v1
This function is to enter ddr self refresh, and wait for keypad or ir power.
if down the standby key or ir power, leave ddr self refresh.
*/
LEAF(cache_code)  
	li	t1,0x30
	sb	t1,0xb8018300

	subu		sp, 256
	sw      $1, 1*4(sp)
	sw      $2, 2*4(sp)
	sw      $3, 3*4(sp)
	sw      $4, 4*4(sp)
	sw      $5, 5*4(sp)
	sw      $6, 6*4(sp)
	sw      $7, 7*4(sp)

	sw      $16, 16*4(sp)
	sw      $17, 17*4(sp)
	sw      $18, 18*4(sp)
	sw      $19, 19*4(sp)
	sw      $20, 20*4(sp)
	sw      $21, 21*4(sp)
	sw      $22, 22*4(sp)
	sw      $23, 23*4(sp)
	sw      $24, 24*4(sp)
	sw      $25, 25*4(sp)
	sw      $26, 26*4(sp)
	sw      $27, 27*4(sp)
	sw      $28, 28*4(sp)
	sw      $29, 29*4(sp)
	sw      $30, 30*4(sp)
	sw      $31, 31*4(sp)	


	li	t1,0x31
	sb	t1,0xb8018300
	move    s7, ra              # save ra to s7
	move    s6, a0              # save standby keypad to s6
	move    s5, a1              # save ir power key to s5
	move    s4, a2              # save the time
	move    s3, a3              # save the timeout
	

	lw      t1, 0xb8006000 	// fb register
	and     t1, ~(1<<0)
	sw	t1,0xb8006000 
	lw      t1, 0xb8007804 	// fb register
	and     t1, ~(1<<0)
	sw	t1,0xb8007804 
	
	li      t1, 5000		// delay for DE show a frame before clk gating
1:
	nop
	nop
	jal     delay_time
	nop
	addiu   t1, -1
	bnez    t1, 1b
	nop
	nop
	
/*
Local Device Clock Gating Control Register (60h)
Bit	Description
31	SM 27M CLK Gating
30	SB UART CLK Gating
29	SB 108M CLK Gating
28	SB MEM CLK Gating
27	USB CLK Gating
26	SCB3_DISCLK Serial Control Bus, 3rd SCB interface clock gating
25	SCB2_DISCLK Serial Control Bus, 2nd SCB interface clock gating
24	IRTX_DISCLK IR transmitter clock gating
23	WDT_DISCLK Watch Dog Timer
22	RTC_DISCLK Real Time Clock
21	SCR2_DISCLK ISO7816 interface 
20	SCR1_DISCLK ISO7816 interface
19	IR_DISCLK IR interface
18	SCB_DISCLK Serial Control Bus
17	UART2_DISCLK UART interface
16	UART1_DISCLK UART interface
15	HDMI ID CLK Gating
14	HDMI PHY CLK Gating
13	HDMI_MEM_CLK Gating
12	MAC CLK Gating
11	QAM CLK Gating
10	SDIO CLK Gating
9	TS SWITCH/CI CLK Gating
8	DEMUX2 CLK Gating
7	DEMUX1 CLK Gating
6	TSG_DISCLK Gating
5	AUDIO CLK Gating
4	TouchPanel CLK Gating
3	Reserved
2	NF CLK Gating
1	SB 1843K CLK Gating
0	SB SYS CLK Gating

Local Device Clock Gating Control Register1 (64H)

Bit	Description
31	CPU2 CLK Gating
30	DLL/DDR3X CLK Gating
29	DM CTROL CLK Gating
28	DES MEM CLK Gating
27	CRYPTO MEM CLK Gating 
26	FLASH MEM CLK Gating
25	VCAP_CLK Gating
24	DVI_CLK Gating(two DEO share clock 27M)
23	SUT_RLE_CLK Gating
22	GC_CLK Gating
21	VOU_HD_CLK(EXT) Gating
20	VOU_HD_CLK Gating
19	Digital PLL Clock gating
18	ELB CLK Gating
17	QAM_59M_CLK Gating
16	VIDEO MEM CLK Gating
15	Reserved
14	BDMA CLK Gating
13	SGDMA2 CLK Gating
12	SGDMA1 CLK Gating
11	VDAC4 CLK Gating
10	VDAC3 CLK Gating
9	VDAC2 CLK Gating
8	VDAC1 CLK Gating
7	S/P RGB clock gating 
6	Reserved
5	TVENC_HD_CLK Gating
4	GE_CLK Gating  
3	TVENC_SD_CLK Gating
2	DE_N_CLK Gating
1	VIDEO_CLK Gating 
0	DE_O_1_CLK Gating
*/
dev_clk_gating:
//	li	t1,	~((1<<30)|(1<<29)|(1<<28)|(1<<26)|(1<<25)|(1<<19)|(1<<18)|(1<<16)|(1<<1)|(1<<0))		// not gating
	li	t1,	~((1<<30)|(1<<29)|(1<<28)|(1<<25)|(1<<19)|(1<<16)|(1<<1)|(1<<0))		// not gating
	sw	t1,0xb8000060
//	li	t1,	~((1<<31)|(1<<30)|(1<<29)|(1<<26)|(1<<19))		//not gating
	li	t1,	~((1<<30)|(1<<29)|(1<<26)|(1<<19))		//not gating
	sw	t1,0xb8000064
	jal     delay_time
	nop

	li	t1,0x32
	sb	t1,0xb8018300

	/* Initialize time count */
	li      k0, TIME_COMPARE_1S
	mtc0	zero, C0_COUNT			# Setup CP0 counter register
	nop
	mtc0	k0, C0_COMPARE
	nop

	/* fill 16k code into i-cache */
	li      t1, 0x4000
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

	li      t0, IR_RLC_BUFFER_START
	addiu   t1, t0, 0x2000          # Start addess + dcache_set_size
1:
	lw      t2, 0x0(t0)
	.set    mips1
	addiu   t0, 4
	slt     t2, t0, t1
	bnez    t2, 1b
	nop

1:	
//	jal		sys_prestandby
	nop	
//	jal 	sys_standby_process
	nop
//	b	1b
		
	li	t1,0x33
	sb	t1,0xb8018300

	/* run in the i-cache */
	la      t0, enter
	sll     t0, 4
	srl     t0, 4
	lui     t1, 0x8000
	or      t0, t1
	jr      t0
	nop

enter:
	/* initialize the ir & clear the d-cache*/

	lw	t1, 0xb8018100 	
	sw   t1,IR_CTL_REG_SAVE
	
	lw	t1, 0(a1) 			# save ir power key
	sw   t1,PM_IR_WAKEUP_KEY1
	lw	t1, 4(a1) 			# save ir power key
	sw   t1,PM_IR_WAKEUP_KEY2
	lw	t1, 8(a1) 			# save ir power key
	sw   t1,PM_IR_WAKEUP_KEY3
	lw	t1, 0xc(a1) 			# save ir power key
	sw   t1,PM_IR_WAKEUP_KEY4
	lw	t1, 0x10(a1) 			# save ir power key
	sw   t1,PM_IR_WAKEUP_KEY5
	lw	t1, 0x14(a1) 			# save ir power key
	sw   t1,PM_IR_WAKEUP_KEY6
	lw	t1, 0x18(a1) 			# save ir power key
	sw   t1,PM_IR_WAKEUP_KEY7
	lw	t1, 0x1c(a1) 			# save ir power key
	sw   t1,PM_IR_WAKEUP_KEY8
	nop
/*
	li	t2,	PM_IR_WAKEUP_KEY1
	li      t3,  32
1:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait empty
	nop
	lb      t1, 0x0(t2)
	sb      t1, 0xb8018300	//printf
	addiu   t2, 1
	addiu   t3, -1
	bnez    t3, 1b	//next byte
	nop
*/	
	jal     init_ir
	nop

	jal	mem_enter_standby_save
	nop
	jal mem_enter_standby
	nop

	li	t1,0x34
	sb	t1,0xb8018300

	// set all io pad drive from 5mA to 2.5mA
	sw	zero,0xb8000410
	sw	zero,0xb8000414
	sw	zero,0xb8000418
	sw	zero,0xb800041c
	sw	zero,0xb8000420
	sw	zero,0xb8000424
	
	li      t1, 100
1:
	nop
	nop
	jal     delay_time
	nop
	addiu   t1, -1
	bnez    t1, 1b
	nop
	nop
1:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait empty
	nop
	li	t0,0x35
	sb	t0,0xb8018300
/*
	// down cpu frequence 
	lui     t0, 0xb800
	lhu     t1, 0x74(t0)
	or      t1, 0x0180
	sh      t1, 0x74(t0)

	lui     t0, 0xb800
	lbu     t1, 0x76(t0)
	or      t1, 0x40
	sb      t1, 0x76(t0)
	nop
	nop
	nop
	nop
	nop
*/

/*
Power Mode Control Register (68h)
3	LP_SBM_SEL1
	0: SB_1843_CLK=1.843M
	1: SB_1843_CLK=SRC_CLK/64
2	LP_SBM_SEL
	0: SB_CLK=SRC_CLK/4
	1: SB_CLK=SRC_CLK/16
1	LOWPOWER_EN  Under low power mode, CPU_CLK, MEM_CLK and IR run and clock to external DRAM chip is OFF
	1: when it is set 1 system will begin go into low power mode 
	0: when it is set 0 system will begin go out low power mode
0	STANDBY_EN. Standby mode enable bit, under low power mode, CPU_CLK, MEM_CLK and IR run at lower speed and clock to external DRAM chip is OFF
	1: when it is set 1 system will begin go into standby mode 
	0: when it is set 0 system will begin go out standby mode
*/
	lui		t0, 0xb800
	lw		t1, 0x68(t0)
	ori		t1, (3<<4)	//SRC_LP_SEL 11: 1/64 CRYSTAL Clock
	ori		t1, 0x05
	sw		t1, 0x68(t0)

1:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait empty
	nop
	li	t0,0x36
	sb	t0,0xb8018300
	
	jal	pll_power_down
	nop

1:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait empty
	nop
	li	t0,0x37
	sb	t0,0xb8018300
	
	lui     t0, 0xb800
	lhu     t1, 0x62(t0)
	and     t1, 0xf7	// ir not clk gated
	sb      t1, 0x62(t0)

	li	t0,0x38
	sb	t0,0xb8018300
	
		
	li      t0, PM_SLEEP_TIMEOUT_CONST
	sw	a3, 0(t0)
	li      t0, PM_SLEEP_TIMEOUT
	li	t1,1
	sw	t1, 0(t0)		// 1s , if PM_SLEEP_TIMEOUT_CONST is 0s , never timeout
	
	jal	board_power_down
	nop
	
	jal		sys_prestandby
	nop	
	jal 	sys_standby_process
	nop
	jal	init_month_array
	nop
	
//	li	t1,	~((1<<31)|(1<<26)|(1<<19))		//not gating
	li	t1,	~((1<<19))		//not gating
	sw	t1,0xb8000064
	
	li	t0,0x39
	sb	t0,0xb8018300
	li	a0,0x05		// panel key IR 0x7F count
loop:    
	/* delay some time : 40us */
	li      t1, 0x1000
1:
	nop
	nop
	bnez    t1, 1b
	addiu   t1, -1


	/* get the ir power key: if down the power, jump the loop */   
	jal     get_ir
	nop
	beqz    v0, check_panel	//invalid key
	nop
	li      t1, PM_IR_WAKEUP_KEY1
	li	t0,8
check_ir:	
	lw	s5, 0(t1)
	bne     v0, s5, ir_next
	nop
	nop
	sb		t0, 0xb8018300	// ir_power[i]
	nop
	li      t1, PM_IR_WAKEUP_KEY1
	li	t0,8
print_wakeupkey:	
	lw	s5, 0(t1)
	nop
	nop
	sb		s5, 0xb8018300	//bit[7-0]
	nop
	srl 		s5, 8
	sb		s5, 0xb8018300	//bit[15-8]
	nop
	srl 		s5, 8
	sb		s5, 0xb8018300	//bit[23-16]
	nop
	srl 		s5, 8
	sb		s5, 0xb8018300	//bit[31-24]
	nop 
1:
	lb	s5,0xB8018305;
	and	s5,0x40
	beqz    s5, 1b	//wait uart empty
	nop
	addiu	t1,4
	addiu 	t0,-1
	bnez	  	t0,print_wakeupkey
	nop
	li	t0,'1'
	sb	t0,0xb8018300	// leave from ir
	b leave
	nop
	nop
	
ir_next:	
	addiu	t1,4
	addiu 	t0,-1
	bnez	  t0,check_ir
	nop

check_panel:	
	li	t0,0x7f	// panel power key press
	bne	 v0,t0,check_timeout
	nop
1:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait uart empty
	nop
	li	t0,'2'
	sb	t0,0xb8018300	// leave from panel power key
	b leave
	nop

	
check_timeout:
//	li      a0, 0x2
//	    jal     pm_uart_debug
	nop

	/* If timeout */
	jal	sleep_time_caculate
	nop
	li      t0, PM_SLEEP_TIMEOUT_CONST
	lw      t1, 0(t0)
	nop
	li      t0, PM_SLEEP_TIMEOUT
	lw      t2, 0(t0)
	bne    t1,t2, loop
	nop
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait uart empty
	nop
	li	t0,'3'
	sb	t0,0xb8018300	//leave from time out
	nop

leave:
	li	t0,0x35
	sb	t0,0xb8018300
	
	li	t1,	~((1<<31)|(1<<30)|(1<<29)|(1<<26)|(1<<19))		//not gating
	sw	t1,0xb8000064
	
//	lui     t0, 0xb800
//	lhu     t1, 0x62(t0)
//	or      t1, 0x08		// ir clk gated 
//	sb      t1, 0x62(t0)

	// set all io pad drive from 2.5mA to 5mA
	li	t1,0x55555555
	sw	t1,0xb8000410
	sw	t1,0xb8000414
	sw	t1,0xb8000418
	sw	t1,0xb800041c
	sw	t1,0xb8000420
	sw	t1,0xb8000424
	
	jal		sys_poststandby
	nop	
	
	jal	board_power_up
	nop
/*
	// disable IR interrupt
	li      t0, SYS_IC_SB_BASE_H    
	li      t1, 0x00
	sb      t1, SYS_IC_SB_IRC_ISR(t0)
	li      t1, 0x00
	sb      t1, SYS_IC_SB_IRC_IER(t0)
*/
//	li	t1, 0x0A160083  	
//	sw   t1,0xb8018100
	
	li      t1, 0xa0                        # Init FIFO, threshould is 32 bytes
	sb      t1, 0xb8018100  #SYS_IC_SB_IRC_FCTRL(t0)
	
	lw	t1, IR_CTL_REG_SAVE  	
	sw   t1,0xb8018100
	
	li	t0,0x36
	sb	t0,0xb8018300
	
	/* clear the keypad interrupt */
	lui     t0, 0xb800
	lhu     t1, 0x005c(t0)
	sh      t1, 0x005c(t0)

	jal     delay_time
	nop

	li	t0,0x37
	sb	t0,0xb8018300
	
	jal pll_power_up
	nop

	li	t0,0x38
	sb	t0,0xb8018300

/*
Power Mode Control Register (68h),exit standby
*/
	lui		t0, 0xb800
	lw		t1, 0x68(t0)
	and  	t1, ~(3<<4)
	and  	t1, ~0x05
	sw		t1, 0x68(t0)
	nop


	li	t0,0x39
	sb	t0,0xb8018300
/*
	// up cpu frequence 
	lui     t0, 0xb800
	lhu     t1, 0x74(t0)
	and     t1, 0xfe7f
	sh      t1, 0x74(t0)

	lui     t0, 0xb800
	lbu     t1, 0x76(t0)
	or      t1, 0x40
	sb      t1, 0x76(t0)
	nop
	nop
	nop
	nop
	nop
*/

	li	t0,0x41
	sb	t0,0xb8018300
	
	li      t1, 0x10000
1:
	addiu   t1, -1
	bnez    t1, 1b
	nop

//Local Device Clock Gating Control Register 
	sw	zero,0xb8000060
	sw	zero,0xb8000064
	jal     delay_time
	nop
	
# #jal     delay_time
	nop

	li	t0,0x42
	sb	t0,0xb8018300

	jal     delay_time
	nop

	li      t0, PM_SLEEP_TIMEOUT
	lw     v0, 0(t0)

	jal	mem_exit_standby
	nop

	li	t0,0x43
	sb	t0,0xb8018300

	li      t1, 5000	//delay for pll 
1:
	nop
	nop
	jal     delay_time
	nop
	addiu   t1, -1
	bnez    t1, 1b
	nop
    
	/* invalidate 4k d-cache all */
	jal     inval_dcache_all
	nop
	
/*	
	li      t1, 0xb8000060
	sw      s4, (t1)
	jal     delay_time
	nop
*/
	li	t0,0x44
	sb	t0,0xb8018300
	move    ra, s7

	lw      t1, 0xb8006000 	// fb register
	or     t1, (1<<0)
	sw	t1,0xb8006000 
	lw      t1, 0xb8007804 	// fb register
	or     t1, (1<<0)
	sw	t1,0xb8007804 
	
	lw      $1, 1*4(sp)
//	lw      $2, 2*4(sp)	// vo , standby  sec count
	lw      $3, 3*4(sp)
	lw      $4, 4*4(sp)	
	lw      $5, 5*4(sp)
	lw      $6, 6*4(sp)
	lw      $7, 7*4(sp)

	lw      $16, 16*4(sp)
	lw      $17, 17*4(sp)
	lw      $18, 18*4(sp)
	lw      $19, 19*4(sp)
	lw      $20, 20*4(sp)
	lw      $21, 21*4(sp)
	lw      $22, 22*4(sp)
	lw      $23, 23*4(sp)
	lw      $24, 24*4(sp)
	lw      $25, 25*4(sp)
	lw      $26, 26*4(sp)
	lw      $27, 27*4(sp)
	lw      $28, 28*4(sp)
	lw      $29, 29*4(sp)
	lw      $30, 30*4(sp)
	lw      $31, 31*4(sp)
	addiu   sp, 256

	li	t0,0x45
	sb	t0,0xb8018300
	/*
	li      t0, 0xb8018500	// watchdog reboot
	li      t1, (0xFFFFFFFF-0x000bb0a)//(0xFFFFFFFF-0x017bb0a)  // 1 sec
	sw      t1, 0x00(t0)
	li      t1, 0x67        // enable watchdog
	sw      t1, 0x04(t0)
	
	li		t1, 0x00000FFF					# Delay some time.
1:		bne		t1, zero, 1b
	nop
		*/
//	move    ra, s7              # save ra to s7
	jr	    ra
	nop

END(cache_code)

LEAF(delay_time)
	/* delay some time : 40us */
	li      t3, 0x1000
1:
	nop
	nop
	addiu   t3, -1
	bnez    t3, 1b
	nop

	jr	    ra
	nop
END(delay_time)

/*
* Useful MACROs --To avoid the crash of the function (don't know why)--
*/
LEAF(sleep_time_caculate)
	move	k1, ra
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

	li	t0,0x31
	sb	t0,0xb8018300
	nop
	jal		inc_time				# Increase time
	nop
	li	t0,0x32
	sb	t0,0xb8018300
	nop
	jal 	sys_standby_process
	nop
	li	t0,0x34
	sb	t0,0xb8018300
	nop
		
	li      t0, PM_SLEEP_TIMEOUT
	lw      t1, 0(t0)
	addiu   t1, 1
	sw      t1, 0(t0)

next_loop:
	nop
	move	ra, k1
	jr	    ra
	nop
END(sleep_time_caculate)

//month_array:
//		.byte	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
		.globl	init_month_array
		.ent	init_month_array
		.set	noreorder
init_month_array:
	li	t0,MONTH_ARRAY
	li	t1,31		// 0
	sb	t1,0x00(t0)
	li	t1,28
	sb	t1,0x01(t0)
	li	t1,31
	sb	t1,0x02(t0)
	li	t1,30
	sb	t1,0x03(t0)
	li	t1,31
	sb	t1,0x04(t0)
	li	t1,30
	sb	t1,0x05(t0)
	li	t1,31
	sb	t1,0x06(t0)
	li	t1,31
	sb	t1,0x07(t0)
	li	t1,30
	sb	t1,0x08(t0)
	li	t1,31
	sb	t1,0x09(t0)
	li	t1,30
	sb	t1,0x0a(t0)
	li	t1,31
	sb	t1,0x0b(t0)
	jr		ra
	nop				
	.set 	reorder
	.end	init_month_array
	
# MACRO: add_over_flow(reg in, const ov, const mb, const sv)
#  reg in:   Register to be conter.
#  const ov: Overflow value.
#  const mb: Conter field mask bits.
#  const sv: Shift number of mask bits.
# Internal:
#  t0: Overflow value saver;
#  t1: Mask bits saver;
#  t2: Mask bits shifted saver;
#  t3: Conter target field;
#  v0: Compare result, return == 0 if conter overflow;
#
		.macro	add_over_flow in,ov,mb,sv
		li		t0, \ov;
		li		t1, \mb;				# Get the act field
		sll		t2, t1, \sv;
		and		t3, \in, t2;
		srl		t3, \sv;
		addiu	t3, 1;					# Inc act field
		slt		v0, t0, t3;
		beqz		v0, 91f;				# If don't greate than ov (v0 == 0)
		nop;
		li		t3, 0;					# Carry
91:		or		\in, t2;				# Equal to: (\in & (~t2))
		xor		\in, t2;
		sll		t3, \sv;
		or		\in, t3; 
		.endm

		.macro	gettimerdata dr
		li		t1, STANDBY_TIMER
		lw		\dr, 0(t1)
		.endm

		.macro	settimerdata sr
		li		t1, STANDBY_TIMER
		sw		\sr,0(t1)	
		.endm

# MACRO: bin2ascii(reg b1, reg d1)
#  reg b1: Input binary data.
#  reg d1: Output BCD code data.
# Internal:
#  t0: ;
#  t1: Primary higher 4 digital;
#  t2: Lower 4 bit digital;
#  t3: ;
#
		.macro	bin2ascii b1,d1
		andi	t0, \b1, 0xf0;
		srl		t0, 4;					# 6bits->2bits higher
		srl		t1, t0, 1;
		addu	t1, t0;					# Get the base: 00b=>0d, 01b=>1d, 10b=>3d, 11b=>4d
		sll		t2, t1, 3;
		sll		t3, t1, 1;
		addu	t3, t2;					# 10X: 0=>0, 1=>10, 2=>20, etc...
		subu	t2, \b1, t3;			# Get draft lower value to t2
		li		t0, 10;
		blt		t2, t0, 91f;			# If lower 4bits<10, finished
		nop
		addiu	t1, 1;					# Else higher+1, and lower-10.
		sub		t2, t0;
		blt		t2, t0, 91f;			# If lower 4bits<10, finished
		nop
		addiu	t1, 1;					# Else higher+1, and lower-10.
		sub		t2, t0;
91:		sll		t1, 4;
		or		\d1, t1, t2; 
		.endm
/*
################################################################################
#	inc_time()
#	s4==globl second count
#	31...26 25...22 21...17 16...12 11...6 5...0
#	  year   mon    date     hour    min   sec
#	s6==globl LED display buffer register (in BCD) & LED ID register
#	31...24  23...16  15...8 7...0
#   LED_ID   dot_mask  hour   min
################################################################################
		.globl	inc_time
		.ent	inc_time
		.set	noreorder
inc_time:
*/
LEAF(inc_time)
		add_over_flow s4, 59, 0x3f, 0	# Inc 1 second
		beqz	v0, 2f
		add_over_flow s4, 59, 0x3f, 6	# Inc 1 minute
		beqz	v0, 1f
		add_over_flow s4, 23, 0x1f, 12	# Inc 1 hour
		beqz	v0, 1f
		
		srl 		t0, s1, 22				# t0 month
		andi 	t0, t0, 0x0f
		srl 		t1, s1, 26				# t1 year
		andi 	t1, t1, 0x3f

		li 		t2, 2
		bne 		t0, t2, not_feb			# if(month == 2)
		nop
		
		beqz		t1, 28f				# if(0 == year) feb = 28
		nop
		andi 	t2, t1, 0x03			# if(0 != (year % 4)) feb = 28
		bnez 	t2, 28f
		nop
		b 		29f					# feb = 29
		nop
not_feb:								
		li 		t2, MONTH_ARRAY				
		addi	t0, -1
		addu 	t0, t2
		lb 		t2, 0(t0)
		li 		t3, 30
		subu 	t2, t2, t3
		beqz 	t2, 30f 
		nop	

31:		
		add_over_flow s4, 31, 0x1f, 17	# Inc 1 day
		b check_ov
		nop
30:		
		add_over_flow s4, 30, 0x1f, 17	# Inc 1 day
		b check_ov
		nop
29:		
		add_over_flow s4, 29, 0x1f, 17	# Inc 1 day
		b check_ov
		nop
28:		
		add_over_flow s4, 28, 0x1f, 17	# Inc 1 day
		
check_ov:
		beqz	v0, 1f	

		li		t0, 1
		sll		t0, t0, 17
		or		s4, s4, t0
		
		add_over_flow s4, 12, 0xf, 22	# Inc 1 month
		beqz	v0, 1f

		li		t0, 1
		sll		t0, t0, 22
		or		s4, s4, t0
		
		add_over_flow s4, 50, 0x3f, 26	# Inc 1 year
1:	
		li		t0, (0x3f<<6)			# Minuter in ASCII: t4
		and		v0, t0, s4
		srl		v0, 6
		bin2ascii v0, t4
		li		t0, (0x1f<<12)			# Hour in ASCII: t5
		and		v0, t0, s4
		srl		v0, 12
		bin2ascii v0, t5
		sll		t5, 8

		gettimerdata t0
		srl		t0, 16					
		sll		t0, 16
		or		t0, t5					
		or		t0, t4
		b		3f
		nop
		
2:		
		gettimerdata t0

3:				
		lui		v0, 0xff				# Dot flash
		xor		t0, v0
		settimerdata t0
4:		
		jr		ra
		nop
	//	.end	inc_time
END(inc_time)


/*
//S3701C XPMU_PD share pin with XGPIO[70], board_power_gpio is GPIO[70] 
LEAF(board_power_down)
	lw      t1, 0xb8000438	// enable GPIO
	ori      t1, (1<<(70-64))
	sw	t1,0xb8000438

	lw      t1, 0xb80000F8	// out put mode
	ori      t1, (1<<(70-64))
	sw	t1,0xb80000F8

	lw      t1, 0xb80000F4	// out put 0
	li	t2,~(1<<(70-64))
	and    t1,t1,t2
	sw	t1,0xb80000F4
	jr	    ra
	nop
END(board_power_down)

//S3701C XPMU_PD share pin with XGPIO[70],board_power_gpio is GPIO[70] 
LEAF(board_power_up)
	lw      t1, 0xb8000438		// enable GPIO
	ori      t1, (1<<(70-64))
	sw	t1,0xb8000438

	lw      t1, 0xb80000F8		// out put mode
	ori      t1, (1<<(70-64))
	sw	t1,0xb80000F8

	lw      t1, 0xb80000F4		// out put 1
	ori      t1, (1<<(70-64))
	sw	t1,0xb80000F4
	nop
	jr	    ra
	nop
END(board_power_up)
*/

//C3701C XPMU_PD share pin with GPIOD[18], board_power_gpio is GPIOD[18] 

LEAF(board_power_down)
	lw      t1, 0xb8000440	// enable GPIOD[18]
	or      t1, (1<<18)
	sw	t1,0xb8000440
	nop	
	lw      t1, 0xb8000458	// GPIOD[18] out enable: 0: enable 1:diable
	and     t1, ~(1<<18)
	sw	t1,0xb8000458
	nop
	nop
	lw      t1, 0xb8000454	//  GPIOD[18] out put low
	and      t1, ~(1<<18)
	sw	t1,0xb8000454
	nop	
	jr	    ra
	nop
END(board_power_down)


//C3701C XPMU_PD share pin with XGPIO[70],board_power_gpio is GPIO[70] 

LEAF(board_power_up)
	lw      t1, 0xb8000440	// enable GPIOD[18]
	or      t1, (1<<18)
	sw	t1,0xb8000440
	nop	
	lw      t1, 0xb8000458	// GPIOD[18] out enable: 0: enable 1:diable
	and     t1, ~(1<<18)
	sw	t1,0xb8000458
	nop
	nop
	lw      t1, 0xb8000454	//  GPIOD[18] out put high
	or      t1, (1<<18)
	sw	t1,0xb8000454
	nop	
	jr	    ra
	nop
END(board_power_up)

LEAF(pll_power_down)
	lui     t0, 0xb800
	lw      t1, 0xb0(t0)
	li      t2, PLL_PM_VALUE_b0
	sw      t1, 0(t2)
	or      t1, (1<<20)	// APLL power down
	sw      t1, 0xb0(t0)

	lw      t1, 0xb4(t0)
	li      t2, PLL_PM_VALUE_b4
	sw      t1, 0(t2)
	or      t1, (1<<12)	// RxPLL power down  bit28 ?????
	sw      t1, 0xb4(t0)

	lw      t1, 0xa0(t0)
	li      t2, PLL_PM_VALUE_a0
	sw      t1, 0(t2)
	or      t1, (1<<15)	// DPLL power down  bit28 ?????
	sw      t1, 0xa0(t0)
	
	lw      t1, 0xc0(t0)
	li      t2, PLL_PM_VALUE_c0
	sw      t1, 0(t2)
//	or      t1, (1<<16)	//SPLL  power down uart will no output
	or      t1, (1<<0)	//CPLL power down
	sw      t1, 0xc0(t0)
	jr	    ra
	nop
END(pll_power_down)

LEAF(pll_power_up)
	lui     t0, 0xb800

	li      t2, PLL_PM_VALUE_c0
	lw      t1, 0(t2)
	sw      t1, 0xc0(t0)

	li      t2, PLL_PM_VALUE_b4
	lw      t1, 0(t2)
	sw      t1, 0xb4(t0)

	li      t2, PLL_PM_VALUE_b0
	lw      t1, 0(t2)
	sw      t1, 0xb0(t0)  
	
	li      t2, PLL_PM_VALUE_a0
	lw      t1, 0(t2)
	sw      t1, 0xa0(t0)  
	
	jr	    ra
	nop
END(pll_power_up)

/*
Two steps to assert self-refresh command
1.B8031030[15] set to '1'
2.B8031030[24] set to '1'
*/
LEAF(mem_enter_standby)
	lw	t1,0xb8001030
	or	t1,(1<<15)
	sw	t1,0xb8001030
	lw	t1,0xb8001030
	or	t1,(1<<24)
	sw	t1,0xb8001030
	nop
	jr	    ra
	nop
END(mem_enter_standby)

/*
1.	Before enter standby mode, please store DDR3PHY_1 / DDR3PHY_2 
2.	DMCTRL assert "Enter Self Refresh" command.
Two steps to assert self-refresh command
1.B8031030[15] set to '1'
2.B8031030[24] set to '1'
*/
LEAF(mem_enter_standby_save)
/*	
	//printf phy register
	li      t2, 0xb803e000
	li      t3, 0x300   
1:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait empty
	nop
	lb      t1, 0x0(t2)
	sb      t1, 0xb8018300	//printf
	addiu   t2, 1
	addiu   t3, -1
	bnez    t3, 1b	//next byte
	nop
	li      t2, 0xb803f000
	li      t3, 0x300   
1:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait empty
	nop
	lb      t1, 0x0(t2)
	sb      t1, 0xb8018300	//printf
	addiu   t2, 1
	addiu   t3, -1
	bnez    t3, 1b	//next byte
	nop
	nop
*/
	lw	t1,0xb8000000
	srl	t1,8	
	andi	t1,0x0F

	li	t2,0x00		//reserved
	beq	t1,t2,BGA292
	nop
	li	t2,0x01	// BGA292 right 16bit ddr3/2
	beq	t1,t2,BGA292
	nop
	li	t2,0x02	//	QAM 256, 32bit DDR
	beq	t1,t2,QFP256
	nop
	li	t2,0x03		// QAM 144 : left 16bit ddr3/2
	beq	t1,t2,QFP144
	nop
	li	t2,0x04		//NMP 144:left 16bit ddr3/2
	beq	t1,t2,QFP144
	nop

QFP144:
	li	t1,0x01		//QAM-144 or NMP-144 only use PHY1
	sw	t1,DDR_PHY1_PHY2_ENABLE
	j	save_mem_clk
	nop

QFP256:
	li	t1,0x03		//QAM-256 both PHY1 and PHY2
	sw	t1,DDR_PHY1_PHY2_ENABLE
	j	save_mem_clk
	nop

BGA292:
	li	t1,0x02		//BGA292 only usb PHY2
	sw	t1,DDR_PHY1_PHY2_ENABLE
	j	save_mem_clk
	nop
	
save_mem_clk:	
	lw	t1,0xb8000070
	and	t1,(1<<17)|(3<<5)	//strap pin info bit[5:6] bit[17]
	sw	t1,MEM_CLK_SAVE
	
save_ddr_phy1_param:	
	lw		t1,DDR_PHY1_PHY2_ENABLE
	andi		t1,0x01
	beqz		t1,save_ddr_phy2_param
	nop			
	li	t1,0x61
	sb	t1,0xb8018300
	
	li	t0, 0xb803e000 
	lw	t1, 0x08(t0) 	//08H PGCR0
	sw   t1,DDR_PHY1_PGCR0
	lw	t1, 0x0c(t0) 	//0CH PGCR1
	sw   t1,DDR_PHY1_PGCR1
	lw	t1, 0x18(t0) 	//18H PLLCR
	sw   t1,DDR_PHY1_PLLCR
	lw	t1, 0x1c(t0) 	// 1CH PTR0	
	sw   t1,DDR_PHY1_PTR0	
	lw	t1, 0x20(t0)  	// 20H PTR1
	sw   t1,DDR_PHY1_PTR1
	lw	t1, 0x24(t0) 	// 24H PTR2  
	sw   t1,DDR_PHY1_PTR2
	lw	t1, 0x28(t0)	 // 28H PTR3
	sw   t1,DDR_PHY1_PTR3
	lw	t1, 0x2c(t0) 	 // 2CH PTR4
	sw   t1,DDR_PHY1_PTR4
	lw	t1, 0x30(t0) 	// 30H ACMDLR
	sw   t1,DDR_PHY1_ACMDLR	
	lw	t1, 0x34(t0) 	//34H ACDBLR
	sw   t1,DDR_PHY1_ACDBLR	
	lw	t1, 0x38(t0) 	// 38H ACIOCR
	sw   t1,DDR_PHY1_ACIOCR
	lw	t1, 0x3C(t0) 	// 3CH DXCCR
	sw   t1,DDR_PHY1_DXCCR			
	lw	t1, 0x40(t0) 	// 40H DSGCR
	sw   t1,DDR_PHY1_DSGCR
	lw	t1, 0x44(t0) 	// 44H DCR
	sw   t1,DDR_PHY1_DCR
	lw	t1, 0x48(t0) 	// 48H DTPR0
	sw   t1,DDR_PHY1_DTPR0
	lw	t1, 0x4C(t0) 	// 4CH	DTPR1
	sw   t1,DDR_PHY1_DTPR1
	lw	t1, 0x50(t0) 	// 50H	DTPR2
	sw   t1,DDR_PHY1_DTPR2
	lw	t1, 0x54(t0) 	// 54H MR0/MR
	sw   t1,DDR_PHY1_MR0_MR
	lw	t1, 0x58(t0) 	//58H MR1/EMR
	sw   t1,DDR_PHY1_MR1_EMR
	lw	t1, 0x5C(t0) 	// 5CH MR2
	sw   t1,DDR_PHY1_MR2
	lw	t1, 0x60(t0) 	// 60H MR3
	sw   t1,DDR_PHY1_MR3
	lw	t1, 0x68(t0) 	// 68H DTCR
	sw	t1,DDR_PHY1_DTCR
	lw	t1, 0x8C(t0)	// 8CH PGCR2 //for C3701C used
	sw   t1,DDR_PHY1_PGCR2
	lw	t1, 0x180(t0) //180h	ZQ 0 Impedance Control Register 0 (ZQ0CR0)
	sw   t1,DDR_PHY1_ZQ0CR0		
	lw	t1, 0x184(t0) 	// 184h	ZQ 0 Impedance Control Register 1 (ZQ0CR1)
	sw   t1,DDR_PHY1_ZQ0CR1	
	lw	t1, 0x188(t0) 	// 188h	ZQ 0 Impedance Status Register 0 (ZQ0SR0)
	sw   t1,DDR_PHY1_ZQ0SR0			
	lw	t1, 0x18C(t0) 	// 18Ch	ZQ 0 Impedance Status Register 1 (ZQ0SR1)
	sw   t1,DDR_PHY1_ZQ0SR1			
	lw	t1, 0x1C0(t0) 	// 1C0h	DATX8 0 General Configuration register (DX0GCR)
	sw   t1,DDR_PHY1_DX0GCR	
	lw	t1, 0x1CC(t0) 	// 1CCh	DATX8 0 Bit Delay Line Registers 0 (DX0BDLR0)
	sw   t1,DDR_PHY1_DX0BDLR0	
	lw	t1, 0x1D0(t0) 	//#1D0h	DATX8 0 Bit Delay Line Registers 1 (DX0BDLR1)
	sw   t1,DDR_PHY1_DX0BDLR1		
	lw	t1, 0x1D4(t0) 	// 1D4h	DATX8 0 Bit Delay Line Registers 2 (DX0BDLR2)
	sw   t1,DDR_PHY1_DX0BDLR2	
	lw	t1, 0x1D8(t0) // 1D8h	DATX8 0 Bit Delay Line Registers 3 (DX0BDLR3)
	sw   t1,DDR_PHY1_DX0BDLR3		
	lw	t1, 0x1DC(t0) // 1DCh	DATX8 0 Bit Delay Line Registers 4 (DX0BDLR4)
	sw   t1,DDR_PHY1_DX0BDLR4		
	lw	t1, 0x1E0(t0)  // 1E0h	DATX8 0 Local Calibrated Delay Line Registers 0 (DX0LCDLR0)
	sw   t1,DDR_PHY1_DX0LCDLR0
	lw	t1, 0x1E4(t0) 	// 1E4h	DATX8 0 Local Calibrated Delay Line Registers 1 (DX0LCDLR1)
	sw   t1,DDR_PHY1_DX0LCDLR1		
	lw	t1, 0x1E8(t0) // 1E8h	DATX8 0 Local Calibrated Delay Line Registers 2 (DX0LCDLR2)
	sw   t1,DDR_PHY1_DX0LCDLR2		
	lw	t1, 0x1EC(t0)	// 1ECh	DATX8 0 Master Delay Line register (DX0MDLR)
	sw   t1,DDR_PHY1_DX0MDLR	
	lw	t1, 0x1F0(t0)	// #1F0h	DATX8 0 General Timing register (DX0GTR)
	sw   t1,DDR_PHY1_DX0GTR
#### Step5:  huangcs_20120620, for Byte 1 Delay line |--->		
	lw	t1, 0x200(t0) 	// 200h	DATX8 1 General Configuration register (DX1GCR)
	sw   t1,DDR_PHY1_DX1GCR
	lw	t1, 0x20C(t0) 	// 20Ch	DATX8 1 Bit Delay Line Registers 0 (DX1BDLR0)
	sw   t1,DDR_PHY1_DX1BDLR0
	lw	t1, 0x210(t0) 	// 210h	DATX8 1 Bit Delay Line Registers 1 (DX1BDLR1)
	sw   t1,DDR_PHY1_DX1BDLR1	
	lw	t1, 0x214(t0) 	// 214h	DATX8 1 Bit Delay Line Registers 2 (DX1BDLR2)
	sw   t1,DDR_PHY1_DX1BDLR2	
	lw	t1, 0x218(t0) 	// 218h	DATX8 1 Bit Delay Line Registers 3 (DX1BDLR3)
	sw   t1,DDR_PHY1_DX1BDLR3	
	lw	t1, 0x21C(t0) 	// #21Ch	DATX8 1 Bit Delay Line Registers 4 (DX1BDLR4)
	sw   t1,DDR_PHY1_DX1BDLR4	
	lw	t1, 0x220(t0) 	// 220h	DATX8 1 Local Calibrated Delay Line Registers 0 (DX1LCDLR0)
	sw   t1,DDR_PHY1_DX1LCDLR0	
	lw	t1, 0x224(t0) 	// 224h	DATX8 1 Local Calibrated Delay Line Registers 1 (DX1LCDLR1)
	sw   t1,DDR_PHY1_DX1LCDLR1	
	lw	t1, 0x228(t0) 	// #228h	DATX8 1 Local Calibrated Delay Line Registers 2 (DX1LCDLR2)
	sw   t1,DDR_PHY1_DX1LCDLR2	
	lw	t1, 0x22C(t0) //#22Ch	DATX8 1 Master Delay Line register (DX1MDLR)
	sw   t1,DDR_PHY1_DX1MDLR	
	lw	t1, 0x230(t0) //#230h	DATX8 1 General Timing register (DX1GTR)
	sw   t1,DDR_PHY1_DX1GTR

save_ddr_phy2_param:
	lw		t1,DDR_PHY1_PHY2_ENABLE
	andi		t1,0x02
	beqz		t1,save_dm_ctrl_param
	nop	
	li	t1,0x62
	sb	t1,0xb8018300	
	li	t0, 0xb803f000 
	lw	t1, 0x08(t0) 	//08H PGCR0
	sw   t1,DDR_PHY2_PGCR0
	lw	t1, 0x0c(t0) 	//0CH PGCR1
	sw   t1,DDR_PHY2_PGCR1
	lw	t1, 0x18(t0) 	//18H PLLCR
	sw   t1,DDR_PHY2_PLLCR
	lw	t1, 0x1c(t0) 	// 1CH PTR0	
	sw   t1,DDR_PHY2_PTR0	
	lw	t1, 0x20(t0)  	// 20H PTR1
	sw   t1,DDR_PHY2_PTR1
	lw	t1, 0x24(t0) 	// 24H PTR2  
	sw   t1,DDR_PHY2_PTR2
	lw	t1, 0x28(t0)	 // 28H PTR3
	sw   t1,DDR_PHY2_PTR3
	lw	t1, 0x2c(t0) 	 // 2CH PTR4
	sw   t1,DDR_PHY2_PTR4
	lw	t1, 0x30(t0) 	// 30H ACMDLR
	sw   t1,DDR_PHY2_ACMDLR	
	lw	t1, 0x34(t0) 	//34H ACDBLR
	sw   t1,DDR_PHY2_ACDBLR	
	lw	t1, 0x38(t0) 	// 38H ACIOCR
	sw   t1,DDR_PHY2_ACIOCR
	lw	t1, 0x3C(t0) 	// 3CH DXCCR
	sw   t1,DDR_PHY2_DXCCR			
	lw	t1, 0x40(t0) 	// 40H DSGCR
	sw   t1,DDR_PHY2_DSGCR
	lw	t1, 0x44(t0) 	// 44H DCR
	sw   t1,DDR_PHY2_DCR
	lw	t1, 0x48(t0) 	// 48H DTPR0
	sw   t1,DDR_PHY2_DTPR0
	lw	t1, 0x4C(t0) 	// 4CH	DTPR1
	sw   t1,DDR_PHY2_DTPR1
	lw	t1, 0x50(t0) 	// 50H	DTPR2
	sw   t1,DDR_PHY2_DTPR2
	lw	t1, 0x54(t0) 	// 54H MR0/MR
	sw   t1,DDR_PHY2_MR0_MR
	lw	t1, 0x58(t0) 	//58H MR1/EMR
	sw   t1,DDR_PHY2_MR1_EMR
	lw	t1, 0x5C(t0) 	// 5CH MR2
	sw   t1,DDR_PHY2_MR2
	lw	t1, 0x60(t0) 	// 60H MR3
	sw   t1,DDR_PHY2_MR3
	lw	t1, 0x68(t0) 	// 68H DTCR
	sw	t1,DDR_PHY2_DTCR
	lw	t1, 0x8C(t0)	// 8CH PGCR2 //for C3701C used
	sw   t1,DDR_PHY2_PGCR2
	lw	t1, 0x180(t0) //180h	ZQ 0 Impedance Control Register 0 (ZQ0CR0)
	sw   t1,DDR_PHY2_ZQ0CR0		
	lw	t1, 0x184(t0) 	// 184h	ZQ 0 Impedance Control Register 1 (ZQ0CR1)
	sw   t1,DDR_PHY2_ZQ0CR1	
	lw	t1, 0x188(t0) 	// 188h	ZQ 0 Impedance Status Register 0 (ZQ0SR0)
	sw   t1,DDR_PHY2_ZQ0SR0			
	lw	t1, 0x18C(t0) 	// 18Ch	ZQ 0 Impedance Status Register 1 (ZQ0SR1)
	sw   t1,DDR_PHY2_ZQ0SR1			
	lw	t1, 0x1C0(t0) 	// 1C0h	DATX8 0 General Configuration register (DX0GCR)
	sw   t1,DDR_PHY2_DX0GCR	
	lw	t1, 0x1CC(t0) 	// 1CCh	DATX8 0 Bit Delay Line Registers 0 (DX0BDLR0)
	sw   t1,DDR_PHY2_DX0BDLR0	
	lw	t1, 0x1D0(t0) 	//#1D0h	DATX8 0 Bit Delay Line Registers 1 (DX0BDLR1)
	sw   t1,DDR_PHY2_DX0BDLR1		
	lw	t1, 0x1D4(t0) 	// 1D4h	DATX8 0 Bit Delay Line Registers 2 (DX0BDLR2)
	sw   t1,DDR_PHY2_DX0BDLR2	
	lw	t1, 0x1D8(t0) // 1D8h	DATX8 0 Bit Delay Line Registers 3 (DX0BDLR3)
	sw   t1,DDR_PHY2_DX0BDLR3		
	lw	t1, 0x1DC(t0) // 1DCh	DATX8 0 Bit Delay Line Registers 4 (DX0BDLR4)
	sw   t1,DDR_PHY2_DX0BDLR4		
	lw	t1, 0x1E0(t0)  // 1E0h	DATX8 0 Local Calibrated Delay Line Registers 0 (DX0LCDLR0)
	sw   t1,DDR_PHY2_DX0LCDLR0
	lw	t1, 0x1E4(t0) 	// 1E4h	DATX8 0 Local Calibrated Delay Line Registers 1 (DX0LCDLR1)
	sw   t1,DDR_PHY2_DX0LCDLR1		
	lw	t1, 0x1E8(t0) // 1E8h	DATX8 0 Local Calibrated Delay Line Registers 2 (DX0LCDLR2)
	sw   t1,DDR_PHY2_DX0LCDLR2		
	lw	t1, 0x1EC(t0)	// 1ECh	DATX8 0 Master Delay Line register (DX0MDLR)
	sw   t1,DDR_PHY2_DX0MDLR	
	lw	t1, 0x1F0(t0)	// #1F0h	DATX8 0 General Timing register (DX0GTR)
	sw   t1,DDR_PHY2_DX0GTR
#### Step5:  huangcs_20120620, for Byte 1 Delay line |--->		
	lw	t1, 0x200(t0) 	// 200h	DATX8 1 General Configuration register (DX1GCR)
	sw   t1,DDR_PHY2_DX1GCR
	lw	t1, 0x20C(t0) 	// 20Ch	DATX8 1 Bit Delay Line Registers 0 (DX1BDLR0)
	sw   t1,DDR_PHY2_DX1BDLR0
	lw	t1, 0x210(t0) 	// 210h	DATX8 1 Bit Delay Line Registers 1 (DX1BDLR1)
	sw   t1,DDR_PHY2_DX1BDLR1	
	lw	t1, 0x214(t0) 	// 214h	DATX8 1 Bit Delay Line Registers 2 (DX1BDLR2)
	sw   t1,DDR_PHY2_DX1BDLR2	
	lw	t1, 0x218(t0) 	// 218h	DATX8 1 Bit Delay Line Registers 3 (DX1BDLR3)
	sw   t1,DDR_PHY2_DX1BDLR3	
	lw	t1, 0x21C(t0) 	// #21Ch	DATX8 1 Bit Delay Line Registers 4 (DX1BDLR4)
	sw   t1,DDR_PHY2_DX1BDLR4	
	lw	t1, 0x220(t0) 	// 220h	DATX8 1 Local Calibrated Delay Line Registers 0 (DX1LCDLR0)
	sw   t1,DDR_PHY2_DX1LCDLR0	
	lw	t1, 0x224(t0) 	// 224h	DATX8 1 Local Calibrated Delay Line Registers 1 (DX1LCDLR1)
	sw   t1,DDR_PHY2_DX1LCDLR1	
	lw	t1, 0x228(t0) 	// #228h	DATX8 1 Local Calibrated Delay Line Registers 2 (DX1LCDLR2)
	sw   t1,DDR_PHY2_DX1LCDLR2	
	lw	t1, 0x22C(t0) //#22Ch	DATX8 1 Master Delay Line register (DX1MDLR)
	sw   t1,DDR_PHY2_DX1MDLR	
	lw	t1, 0x230(t0) //#230h	DATX8 1 General Timing register (DX1GTR)
	sw   t1,DDR_PHY2_DX1GTR

save_dm_ctrl_param:
	li	t1,0x63
	sb	t1,0xb8018300	
	li	t0, 0xb8001000 	//# dm_ctrl_param1
	lw	t1, 0x00(t0) 
	sw   t1,DDR_DM_REG1
	lw	t1, 0x04(t0) 
	sw   t1,DDR_DM_REG2
	jr	    ra
	nop
END(mem_enter_standby_save)

/*
4.	Exit Standby mode
2.	Disable unused DDR3PHY.
3.	Restore DDR3PHY_1 / DDR3PHY_2 register.
4.	DMCTR assert "Exit Self Refresh" command.
*/
LEAF(mem_exit_standby)
##--------------change mem clk --------------------
set_mem_clk:
	li	t0, 0xb8000000  
	lw	t1,0x68(t0)		// power mode control reg
	or	t1,(1<<30)
	sw	t1,0x68(t0)		// mem clk swith enable
	lw	t1,MEM_CLK_SAVE		//bit[5:6] bit[17]
	or	t1,(1<<21)	## triger
	sw	t1,0x74(t0)
	lw	t1,0x68(t0)		// power mode control reg
	and	t1,~(1<<30)
	sw	t1,0x68(t0)		// mem clk swith disable
	
##--------------BGA256, disable port 1 --------------------
power_down_phy1:
	lw		t1,DDR_PHY1_PHY2_ENABLE 
	andi		t1,0x01
	bnez		t1,power_down_phy2
	nop
	li	t1,0x64
	sb	t1,0xb8018300	
	li	t0, 0xb803e000      ## DDR3PHY1 base addr 0x1803_E000   DDR3PHY1 base addr 0x1803_F000 
	li	t1,  0x2001C000 
	sw	t1, 0x18(t0)
	li	t1,  0x3BFC3F1A 
	sw	t1, 0x38(t0)
	li	t1,  0x0000189C 
	sw	t1, 0x3c(t0)
	li	t1,  0xFFF0001F 
	sw	t1, 0x40(t0)
	li	t1,  0xC000014A 
	sw	t1, 0x180(t0)
	li	t1,  0x7C020EF0 
	sw	t1, 0x1c0(t0)
	li	t1,  0x7C020EF0 
	sw	t1, 0x200(t0)


##--------------qam144/nmp144, disable port 2 --------------------
power_down_phy2:
	lw		t1,DDR_PHY1_PHY2_ENABLE 
	andi		t1,0x02
	bnez		t1,wait_phy1_init_done
	nop	
	li	t1,0x65
	sb	t1,0xb8018300	
	li	t0, 0xb803f000      ## DDR3PHY1 base addr 0x1803_E000   DDR3PHY2 base addr 0x1803_F000 
	li	t1,  0x2001C000 
	sw	t1, 0x18(t0)
	li	t1,  0x3BFC3F1A 
	sw	t1, 0x38(t0)
	li	t1,  0x0000189C 
	sw	t1, 0x3c(t0)
	li	t1,  0xFFF0001F 
	sw	t1, 0x40(t0)
	li	t1,  0xC000014A 
	sw	t1, 0x180(t0)
	li	t1,  0x7C020EF0 
	sw	t1, 0x1c0(t0)
	li	t1,  0x7C020EF0 
	sw	t1, 0x200(t0)

wait_phy1_init_done:
	lw		t1,DDR_PHY1_PHY2_ENABLE	
	andi		t1,0x01
	beqz		t1,wait_phy2_init_done
	nop	
	li	t1,0x66
	sb	t1,0xb8018300	
	li	t0, 0xb803e000    
##wait until DDR3PHY_2 PGSR0[0] = 1'b1. means PHY init done
1:
	lw      t1, 0x10(t0)	
	andi  t1, 1
	beqz  t1, 1b
	nop

wait_phy2_init_done:
	lw		t1,DDR_PHY1_PHY2_ENABLE
	andi		t1,0x02
	beqz		t1,set_phy1_param
	nop	
	li	t1,0x67
	sb	t1,0xb8018300	
	li	t0, 0xb803f000    
##wait until DDR3PHY_2 PGSR0[0] = 1'b1. means PHY init done
1:
	lw      t1, 0x10(t0)	
	andi  t1, 1
	beqz  t1, 1b
	nop
	
set_phy1_param:
	lw		t1,DDR_PHY1_PHY2_ENABLE
	andi		t1,0x01
	beqz		t1,set_phy2_param
	nop	
	li	t1,0x68
	sb	t1,0xb8018300	
	li	t0, 0xb803e000 
	lw   t1,DDR_PHY1_PGCR0
	sw	t1, 0x08(t0) 	//08H PGCR0
	lw   t1,DDR_PHY1_PGCR1
	sw	t1, 0x0c(t0) 	//0CH PGCR1
	lw   t1,DDR_PHY1_PLLCR
	sw	t1, 0x18(t0) 	//18H PLLCR
	lw   t1,DDR_PHY1_PTR0	
	sw	t1, 0x1c(t0) 	// 1CH PTR0	
	lw   t1,DDR_PHY1_PTR1
	sw	t1, 0x20(t0)  	// 20H PTR1
	lw   t1,DDR_PHY1_PTR2
	sw	t1, 0x24(t0) 	// 24H PTR2  
	lw   t1,DDR_PHY1_PTR3
	sw	t1, 0x28(t0)	 // 28H PTR3
	lw   t1,DDR_PHY1_PTR4
	sw	t1, 0x2c(t0) 	 // 2CH PTR4
	lw   t1,DDR_PHY1_ACMDLR	
	sw	t1, 0x30(t0) 	// 30H ACMDLR
	lw   t1,DDR_PHY1_ACDBLR	
	sw	t1, 0x34(t0) 	//34H ACDBLR
	lw   t1,DDR_PHY1_ACIOCR
	sw	t1, 0x38(t0) 	// 38H ACIOCR
	lw   t1,DDR_PHY1_DXCCR		
	sw	t1, 0x3C(t0) 	// 3CH DXCCR	
	lw   t1,DDR_PHY1_DSGCR
	sw	t1, 0x40(t0) 	// 40H DSGCR
	lw   t1,DDR_PHY1_DCR
	sw	t1, 0x44(t0) 	// 44H DCR
	lw   t1,DDR_PHY1_DTPR0
	sw	t1, 0x48(t0) 	// 48H DTPR0
	lw   t1,DDR_PHY1_DTPR1
	sw	t1, 0x4C(t0) 	// 4CH	DTPR1
	lw   t1,DDR_PHY1_DTPR2
	sw	t1, 0x50(t0) 	// 50H	DTPR2
	lw   t1,DDR_PHY1_MR0_MR
	sw	t1, 0x54(t0) 	// 54H MR0/MR
	lw   t1,DDR_PHY1_MR1_EMR
	sw	t1, 0x58(t0) 	//58H MR1/EMR
	lw   t1,DDR_PHY1_MR2
	sw	t1, 0x5C(t0) 	// 5CH MR2
	lw   t1,DDR_PHY1_MR3
	sw	t1, 0x60(t0) 	// 60H MR3
	lw	t1,DDR_PHY1_DTCR
	sw	t1,0x68(t0)     // 68H DTCR
	lw   t1,DDR_PHY1_PGCR2
	sw	t1, 0x8C(t0)	// 8CH PGCR2 //for C3701C used	
  ## Set ZQ0CR0 to overrride the ODT to 150Ohm
  ##
  ## ZQ0SR0 (0xb803f188)  
  ## ZCTRL[19:15] is used to select the pull-up on-die termination impedance
  ## ZCTRL[14:10] is used to select the pull-down on-die termination impedance
  ## ZCTRL[9:5]   is used to select the pull-up output impedance 
  ## ZCTRL[4:0]   is used to select the pull-down output impedance.
  ##
  ## ZQ0CR0 (0xb803f180)
  ## [31] ZQPD
  ## [30] ZCALEN
  ## [29] ZCALBYP  --> set to 1
  ## [28] ZDEN     --> set to 1
  ## [19:15] is used to select the pull-up on-die termination impedance 
  ## [14:10] is used to select the pull-down on-die termination impedance
  ## [9:5]   is used to select the pull-up output impedance 
  ## [4:0]   is used to select the pull-down output impedance.
  ##
  ## set 180[29:28] to "2'b011" for override ZQ.
  ##
  ##180h	ZQ 0 Impedance Control Register 0 (ZQ0CR0),use ZQ0SR0 to overrride  
	lw   t1,DDR_PHY1_ZQ0CR1	
	sw	t1, 0x184(t0) 	// 184h	ZQ 0 Impedance Control Register 1 (ZQ0CR1)
	lw   t1,DDR_PHY1_ZQ0SR0
	li	t2,0x30000000
	or  t1,t1,t2		
	sw	t1, 0x180(t0) //180h	ZQ 0 Impedance Control Register 0 (ZQ0CR0)
	lw   t1,DDR_PHY1_ZQ0SR0			
##	sw	t1, 0x188(t0) 	// 188h	ZQ 0 Impedance Status Register 0 (ZQ0SR0)
	lw   t1,DDR_PHY1_ZQ0SR1			
##	sw	t1, 0x18C(t0) 	// 18Ch	ZQ 0 Impedance Status Register 1 (ZQ0SR1)
	lw   t1,DDR_PHY1_DX0GCR	
	sw	t1, 0x1C0(t0) 	// 1C0h	DATX8 0 General Configuration register (DX0GCR)
	lw   t1,DDR_PHY1_DX0BDLR0	
	sw	t1, 0x1CC(t0) 	// 1CCh	DATX8 0 Bit Delay Line Registers 0 (DX0BDLR0)
	lw   t1,DDR_PHY1_DX0BDLR1		
	sw	t1, 0x1D0(t0) 	//#1D0h	DATX8 0 Bit Delay Line Registers 1 (DX0BDLR1)
	lw   t1,DDR_PHY1_DX0BDLR2	
	sw	t1, 0x1D4(t0) 	// 1D4h	DATX8 0 Bit Delay Line Registers 2 (DX0BDLR2)
	lw   t1,DDR_PHY1_DX0BDLR3		
	sw	t1, 0x1D8(t0) // 1D8h	DATX8 0 Bit Delay Line Registers 3 (DX0BDLR3)
	lw   t1,DDR_PHY1_DX0BDLR4		
	sw	t1, 0x1DC(t0) // 1DCh	DATX8 0 Bit Delay Line Registers 4 (DX0BDLR4)
	lw   t1,DDR_PHY1_DX0LCDLR0
	sw	t1, 0x1E0(t0)  // 1E0h	DATX8 0 Local Calibrated Delay Line Registers 0 (DX0LCDLR0)
	lw   t1,DDR_PHY1_DX0LCDLR1		
	sw	t1, 0x1E4(t0) 	// 1E4h	DATX8 0 Local Calibrated Delay Line Registers 1 (DX0LCDLR1)
	lw   t1,DDR_PHY1_DX0LCDLR2		
	sw	t1, 0x1E8(t0) // 1E8h	DATX8 0 Local Calibrated Delay Line Registers 2 (DX0LCDLR2)
	lw   t1,DDR_PHY1_DX0MDLR	
	sw	t1, 0x1EC(t0)	// 1ECh	DATX8 0 Master Delay Line register (DX0MDLR)
	lw   t1,DDR_PHY1_DX0GTR
	sw	t1, 0x1F0(t0)	// #1F0h	DATX8 0 General Timing register (DX0GTR)
#### Step5:  huangcs_20120620, for Byte 1 Delay line |--->		
	lw   t1,DDR_PHY1_DX1GCR
	sw	t1, 0x200(t0) 	// 200h	DATX8 1 General Configuration register (DX1GCR)
	lw   t1,DDR_PHY1_DX1BDLR0
	sw	t1, 0x20C(t0) 	// 20Ch	DATX8 1 Bit Delay Line Registers 0 (DX1BDLR0)
	lw   t1,DDR_PHY1_DX1BDLR1	
	sw	t1, 0x210(t0) 	// 210h	DATX8 1 Bit Delay Line Registers 1 (DX1BDLR1)
	lw   t1,DDR_PHY1_DX1BDLR2	
	sw	t1, 0x214(t0) 	// 214h	DATX8 1 Bit Delay Line Registers 2 (DX1BDLR2)
	lw   t1,DDR_PHY1_DX1BDLR3	
	sw	t1, 0x218(t0) 	// 218h	DATX8 1 Bit Delay Line Registers 3 (DX1BDLR3)
	lw   t1,DDR_PHY1_DX1BDLR4	
	sw	t1, 0x21C(t0) 	// #21Ch	DATX8 1 Bit Delay Line Registers 4 (DX1BDLR4)
	lw   t1,DDR_PHY1_DX1LCDLR0	
	sw	t1, 0x220(t0) 	// 220h	DATX8 1 Local Calibrated Delay Line Registers 0 (DX1LCDLR0)
	lw   t1,DDR_PHY1_DX1LCDLR1	
	sw	t1, 0x224(t0) 	// 224h	DATX8 1 Local Calibrated Delay Line Registers 1 (DX1LCDLR1)
	lw   t1,DDR_PHY1_DX1LCDLR2	
	sw	t1, 0x228(t0) 	// #228h	DATX8 1 Local Calibrated Delay Line Registers 2 (DX1LCDLR2)
	lw   t1,DDR_PHY1_DX1MDLR	
	sw	t1, 0x22C(t0) //#22Ch	DATX8 1 Master Delay Line register (DX1MDLR)
	lw   t1,DDR_PHY1_DX1GTR
	lw	t1, 0x230(t0) //#230h	DATX8 1 General Timing register (DX1GTR)

set_phy2_param:
	lw		t1,DDR_PHY1_PHY2_ENABLE
	andi		t1,0x02
	beqz		t1,wait_for_phy1
	nop	
	li	t1,0x69
	sb	t1,0xb8018300	
	li	t0, 0xb803f000 
	lw   t1,DDR_PHY2_PGCR0
	sw	t1, 0x08(t0) 	//08H PGCR0
	lw   t1,DDR_PHY2_PGCR1
	sw	t1, 0x0c(t0) 	//0CH PGCR1
	lw   t1,DDR_PHY2_PLLCR
	sw	t1, 0x18(t0) 	//18H PLLCR
	lw   t1,DDR_PHY2_PTR0	
	sw	t1, 0x1c(t0) 	// 1CH PTR0	
	lw   t1,DDR_PHY2_PTR1
	sw	t1, 0x20(t0)  	// 20H PTR1
	lw   t1,DDR_PHY2_PTR2
	sw	t1, 0x24(t0) 	// 24H PTR2  
	lw   t1,DDR_PHY2_PTR3
	sw	t1, 0x28(t0)	 // 28H PTR3
	lw   t1,DDR_PHY2_PTR4
	sw	t1, 0x2c(t0) 	 // 2CH PTR4
	lw   t1,DDR_PHY2_ACMDLR	
	sw	t1, 0x30(t0) 	// 30H ACMDLR
	lw   t1,DDR_PHY2_ACDBLR	
	sw	t1, 0x34(t0) 	//34H ACDBLR
	lw   t1,DDR_PHY2_ACIOCR
	sw	t1, 0x38(t0) 	// 38H ACIOCR
	lw   t1,DDR_PHY2_DXCCR		
	sw	t1, 0x3C(t0) 	// 3CH DXCCR	
	lw   t1,DDR_PHY2_DSGCR
	sw	t1, 0x40(t0) 	// 40H DSGCR
	lw   t1,DDR_PHY2_DCR
	sw	t1, 0x44(t0) 	// 44H DCR
	lw   t1,DDR_PHY2_DTPR0
	sw	t1, 0x48(t0) 	// 48H DTPR0
	lw   t1,DDR_PHY2_DTPR1
	sw	t1, 0x4C(t0) 	// 4CH	DTPR1
	lw   t1,DDR_PHY2_DTPR2
	sw	t1, 0x50(t0) 	// 50H	DTPR2
	lw   t1,DDR_PHY2_MR0_MR
	sw	t1, 0x54(t0) 	// 54H MR0/MR
	lw   t1,DDR_PHY2_MR1_EMR
	sw	t1, 0x58(t0) 	//58H MR1/EMR
	lw   t1,DDR_PHY2_MR2
	sw	t1, 0x5C(t0) 	// 5CH MR2
	lw   t1,DDR_PHY2_MR3
	sw	t1, 0x60(t0) 	// 60H MR3
	lw	t1,DDR_PHY2_DTCR
	sw	t1,0x68(t0)     // 68H DTCR
	lw   t1,DDR_PHY2_PGCR2
	sw	t1, 0x8C(t0)	// 8CH PGCR2 //for C3701C used
  ## Set ZQ0CR0 to overrride the ODT to 150Ohm
  ##
  ## ZQ0SR0 (0xb803f188)  
  ## ZCTRL[19:15] is used to select the pull-up on-die termination impedance
  ## ZCTRL[14:10] is used to select the pull-down on-die termination impedance
  ## ZCTRL[9:5]   is used to select the pull-up output impedance 
  ## ZCTRL[4:0]   is used to select the pull-down output impedance.
  ##
  ## ZQ0CR0 (0xb803f180)
  ## [31] ZQPD
  ## [30] ZCALEN
  ## [29] ZCALBYP  --> set to 1
  ## [28] ZDEN     --> set to 1
  ## [19:15] is used to select the pull-up on-die termination impedance 
  ## [14:10] is used to select the pull-down on-die termination impedance
  ## [9:5]   is used to select the pull-up output impedance 
  ## [4:0]   is used to select the pull-down output impedance.
  ##
  ## set 180[29:28] to "2'b011" for override ZQ.
  ##
  ##180h	ZQ 0 Impedance Control Register 0 (ZQ0CR0),use ZQ0SR0 to overrride  
	lw   t1,DDR_PHY2_ZQ0CR1	
	sw	t1, 0x184(t0) 	// 184h	ZQ 0 Impedance Control Register 1 (ZQ0CR1)
	lw   t1,DDR_PHY2_ZQ0SR0
	li	t2,0x30000000
	or  t1,t1,t2
	sw	t1, 0x180(t0) //180h	ZQ 0 Impedance Control Register 0 (ZQ0CR0)
	lw   t1,DDR_PHY2_ZQ0SR0			
##	sw	t1, 0x188(t0) 	// 188h	ZQ 0 Impedance Status Register 0 (ZQ0SR0)
	lw   t1,DDR_PHY2_ZQ0SR1			
##	sw	t1, 0x18C(t0) 	// 18Ch	ZQ 0 Impedance Status Register 1 (ZQ0SR1)
	lw   t1,DDR_PHY2_DX0GCR	
	sw	t1, 0x1C0(t0) 	// 1C0h	DATX8 0 General Configuration register (DX0GCR)
	lw   t1,DDR_PHY2_DX0BDLR0	
	sw	t1, 0x1CC(t0) 	// 1CCh	DATX8 0 Bit Delay Line Registers 0 (DX0BDLR0)
	lw   t1,DDR_PHY2_DX0BDLR1		
	sw	t1, 0x1D0(t0) 	//#1D0h	DATX8 0 Bit Delay Line Registers 1 (DX0BDLR1)
	lw   t1,DDR_PHY2_DX0BDLR2	
	sw	t1, 0x1D4(t0) 	// 1D4h	DATX8 0 Bit Delay Line Registers 2 (DX0BDLR2)
	lw   t1,DDR_PHY2_DX0BDLR3		
	sw	t1, 0x1D8(t0) // 1D8h	DATX8 0 Bit Delay Line Registers 3 (DX0BDLR3)
	lw   t1,DDR_PHY2_DX0BDLR4		
	sw	t1, 0x1DC(t0) // 1DCh	DATX8 0 Bit Delay Line Registers 4 (DX0BDLR4)
	lw   t1,DDR_PHY2_DX0LCDLR0
	sw	t1, 0x1E0(t0)  // 1E0h	DATX8 0 Local Calibrated Delay Line Registers 0 (DX0LCDLR0)
	lw   t1,DDR_PHY2_DX0LCDLR1		
	sw	t1, 0x1E4(t0) 	// 1E4h	DATX8 0 Local Calibrated Delay Line Registers 1 (DX0LCDLR1)
	lw   t1,DDR_PHY2_DX0LCDLR2		
	sw	t1, 0x1E8(t0) // 1E8h	DATX8 0 Local Calibrated Delay Line Registers 2 (DX0LCDLR2)
	lw   t1,DDR_PHY2_DX0MDLR	
	sw	t1, 0x1EC(t0)	// 1ECh	DATX8 0 Master Delay Line register (DX0MDLR)
	lw   t1,DDR_PHY2_DX0GTR
	sw	t1, 0x1F0(t0)	// #1F0h	DATX8 0 General Timing register (DX0GTR)
#### Step5:  huangcs_20120620, for Byte 1 Delay line |--->		
	lw   t1,DDR_PHY2_DX1GCR
	sw	t1, 0x200(t0) 	// 200h	DATX8 1 General Configuration register (DX1GCR)
	lw   t1,DDR_PHY2_DX1BDLR0
	sw	t1, 0x20C(t0) 	// 20Ch	DATX8 1 Bit Delay Line Registers 0 (DX1BDLR0)
	lw   t1,DDR_PHY2_DX1BDLR1	
	sw	t1, 0x210(t0) 	// 210h	DATX8 1 Bit Delay Line Registers 1 (DX1BDLR1)
	lw   t1,DDR_PHY2_DX1BDLR2	
	sw	t1, 0x214(t0) 	// 214h	DATX8 1 Bit Delay Line Registers 2 (DX1BDLR2)
	lw   t1,DDR_PHY2_DX1BDLR3	
	sw	t1, 0x218(t0) 	// 218h	DATX8 1 Bit Delay Line Registers 3 (DX1BDLR3)
	lw   t1,DDR_PHY2_DX1BDLR4	
	sw	t1, 0x21C(t0) 	// #21Ch	DATX8 1 Bit Delay Line Registers 4 (DX1BDLR4)
	lw   t1,DDR_PHY2_DX1LCDLR0	
	sw	t1, 0x220(t0) 	// 220h	DATX8 1 Local Calibrated Delay Line Registers 0 (DX1LCDLR0)
	lw   t1,DDR_PHY2_DX1LCDLR1	
	sw	t1, 0x224(t0) 	// 224h	DATX8 1 Local Calibrated Delay Line Registers 1 (DX1LCDLR1)
	lw   t1,DDR_PHY2_DX1LCDLR2	
	sw	t1, 0x228(t0) 	// #228h	DATX8 1 Local Calibrated Delay Line Registers 2 (DX1LCDLR2)
	lw   t1,DDR_PHY2_DX1MDLR	
	sw	t1, 0x22C(t0) //#22Ch	DATX8 1 Master Delay Line register (DX1MDLR)
	lw   t1,DDR_PHY2_DX1GTR
	sw	t1, 0x230(t0) //#230h	DATX8 1 General Timing register (DX1GTR)

wait_for_phy1:
 ##---------------wait for port 1 --------------------
	lw		t1,DDR_PHY1_PHY2_ENABLE
	andi		t1,0x01
	beqz		t1,wait_for_phy2
	nop	
	li	t0, 0xb803e000    

 ##DDR3PHY_1 Write PIR
 ##	li	t1, 0x00000181
	li	t1, 0x00000101
	sw	t1, 0x04(t0)  

 ##wait until DDR3PHY_1 PGSR0[0] = 1'b1. means PHY init done
1:
	lw      t1, 0x10(t0)	
	andi  t1, 1
	beqz  t1, 1b
	nop
								
 ##check if DDR3PHY_1 PGSR0[31] = 1'b1
1:
	lw 	t1, 0x10(t0)
	and	t1, 0x80000000
	beqz		t1, 1b
	nop


wait_for_phy2:
 ##-------------wait for port 2 --------------------
	lw		t1,DDR_PHY1_PHY2_ENABLE
	andi		t1,0x02
	beqz		t1,set_dm_ctrl_parm1
	nop	
	li	t0, 0xb803f000    

 ##DDR3PHY_2 Write PIR
 ##	li	t1, 0x00000181
	li	t1, 0x00000101
	sw	t1, 0x04(t0)  

 ##wait until DDR3PHY_2 PGSR0[0] = 1'b1. means PHY init done
1:
	lw      t1, 0x10(t0)	
	andi  t1, 1
	beqz  t1, 1b
	nop

 ##check if DDR3PHY_2 PGSR0[31] = 1'b1
1:
	lw 	t1, 0x10(t0)
	and	t1, 0x80000000
	beqz		t1, 1b
	nop


set_dm_ctrl_parm1:
	li	t1,0x6a
	sb	t1,0xb8018300	
	li	t0, 0xb8001000 	//# dm_ctrl_param1
	lw   t1,DDR_DM_REG1
	sw	t1, 0x00(t0) 
	lw   t1,DDR_DM_REG2
	sw	t1, 0x04(t0) 

set_dm_ctrl_param2:	
	li	t1,0x6b
	sb	t1,0xb8018300	
##-------------------- DRAM controller ---------------------------------
#Jillian add |---->
##	li	t1, (1<<6)|(2<<3) //bit[5:3]+8bit
	li	t1, (3<<6)|(2<<3) //bit[5:3]+8bit	
#Jillian end <----|

	sb	t1, 0x31(t0) 	##0xb8001030   bit[13:11] Default refresh CFG [2:0]   
	li	t1, (3<<5)  
	sb	t1, 0x32(t0) 	##0xb8001032 bit[7:5] MONITOR_SEL 
#Jillian add |---->
	li	t1, 0x61  
	sb	t1, 0x33(t0) 	##default in self-refresh mode
#Jillian end <----|

	li	t1, 0xfa5f
	sh	t1, 0x0e(t0)   ##0xb800100c -0f :CFG_SEQ [31:0] 
	li	t1, 0x40
	sb	t1, 0x0d(t0)

	li	t0, 0xb8000000 
	li	t1, 0xffffffff
	sh	t1, 0x224(t0)   ## CPU Interface parameter register (224h) The configuration of Main CPU and Slave CPU latency and HI_Priority

	li	t0, 0xb8001000 
	li	t1, 0xfdffcfff  
	sw	t1, 0x10(t0)    ##0xb8001010 -13 :CFG_SEQ [63:32] 
	li	t1, 0x88888888  
	sw	t1, 0x18(t0)    ##0xb8001018 -1b :CFG_SEQ [95:64] 
	li	t1, 0x888888e8  
	sw	t1, 0x78(t0)     ##0xb8001078 -7b :CFG_SEQ [191:160]  
	li	t1, 0xffffffff  
	sw	t1, 0x7C(t0)    ##0xb800107C :CFG_SEQ[223:192]
	li	t1, 0xffffffff  
	sw	t1, 0x80(t0)    ##0xb8001080 :CFG_SEQ[255:224]
	li	t1, 0x0f0f0f0f  
	sw	t1, 0x20(t0)     ##0xb8001020 -23 :CFG_SEQ [127:96] 
	li	t1, 0x0f  
	sb	t1, 0x26(t0)     ##0xb8001024 -27 :CFG_SEQ [159:128] 
	li	t1, 0x80  
	sb	t1, 0x09(t0)    ## 0xb8001008 -0b	CFG_IMB0 [31:0]
	li	t1, 0x33330f00  
	sw	t1, 0x14(t0)    ##0xb8001014 -17	CFG_IMB0 [63:32]
	li	t1, 0xffffffff  
	sw	t1, 0x1c(t0)    ##0xb800101c -1f	CFG_IMB0 [95:64]
	li	t1, 0x80  
	sb	t1, 0x35(t0)    ##0xb8001034 -37	CFG_IMB1 [31:0]
	li	t1, 0x33330700  
	sw	t1, 0x38(t0)    ##0xb8001038 -3b	CFG_IMB1 [63:32]
	li	t1, 0x1fffffff  
	sw	t1, 0x3c(t0)    ##0xb800103c -3f	CFG_IMB1 [95:64]
	li	t1, 0x80  
	sb	t1, 0x29(t0)    ##0xb8001028 -2b	CFG_IMB2 [31:0]
	li	t1, 0x33330f00  
	sw	t1, 0x2c(t0)    ##0xb800102c -2f	CFG_IMB2 [63:32]
	li	t1, 0xffffffff  
	sw	t1, 0x68(t0)     ##0xb8001068 -6b	CFG_IMB2 [95:64]

#huangcs |---->  
#enable DRAM free run clock for DFI_RDATA_EN FIX_DAT_LATENCY issue
	li	t1, 0x01   
	sb	t1, 0x06(t0)  
#huangcs <----| 


#### Step8:  assert "Exit self refresh" command |--->	 	
###-----------------------------------------------------------------
##   b800_1030[24] -> 0;	//Exit Self-refresh
##   b800_1030[31] -> 0/1;	//Disable/Enable ODT
##   b800_1030[15] -> 0;	//Enable auto-refresh

	li	t0, 0xb8001000 
	lb  t2, 0x33(t0)  
	and t2, 0xfe      //b800_1030[24] -> 0;	//Exit Self-refresh
	sb	t2, 0x33(t0)  

	lb  t2, 0x31(t0)  	
	and t2, 0x7f      //b800_1030[15] -> 0;	//Enable auto-refresh
	sb	t2, 0x31(t0)  

###-----------------------------------------------------------------
/*

	//printf phy register
	li      t2, 0xb803e000
	li      t3, 0x300   
1:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait empty
	nop
	lb      t1, 0x0(t2)
	sb      t1, 0xb8018300	//printf
	addiu   t2, 1
	addiu   t3, -1
	bnez    t3, 1b	//next byte
	nop
	li      t2, 0xb803f000
	li      t3, 0x300   
1:
	lb	t1,0xB8018305;
	and	t1,0x40
	beqz    t1, 1b	//wait empty
	nop
	lb      t1, 0x0(t2)
	sb      t1, 0xb8018300	//printf
	addiu   t2, 1
	addiu   t3, -1
	bnez    t3, 1b	//next byte
	nop
	nop
*/	
	jr	    ra
	nop
END(mem_exit_standby)

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
*  Get the panel key
*/
LEAF(check_key_panel)
	lui     t0, 0xb801
	li      t1, 0x4f
	sb      t1, 0x8704(t0)

	li      t1, 0xc5
	sb      t1, 0x8700(t0)

	move    t2, ra
	jal     delay_time
	nop

	move    ra, t2

	lw      v0, 0x8710(t0)

	jr		ra
	nop
END(check_key_panel)


/*
*	void pm_uart_debug(UINT32 data)
*  a0: print data
*	print debug information
*  be careful: the value of t0 will be corrupted since wen don't have stack to use now
*/
.globl  pm_uart_debug
.ent    pm_uart_debug
pm_uart_debug:	

	addiu   a0, 0x30
	li      t6, 0xb8018300
	sb      a0, 0(t6)
	nop
	nop
	nop
	nop
	nop
	nop

	jr	    ra
	nop
.end	pm_uart_debug


