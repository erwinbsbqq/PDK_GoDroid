/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2008 Copyright (C)
*
*    File:	slot.S
*
*    Description:	This file contains all code in standby mode.
*    History:
*	Date 		Athor		Version		Reason
*	============	=======================================
*	1.07.04.2008	Mao Feng 	Ver 0.1		Create file.
*	
*****************************************************************************/
//#include <hal/mips.h>
//#include <hld/pan/pan.h>
//#include <sys_config.h>

#include "m36_sleep.h"
#include "mips.h"

//#define CONFIGDATA_BASE 0xAFC00000

#define COM_VERSION	(CONFIGDATA_BASE+0x80)
#define SDRAM_ASR	(CONFIGDATA_BASE+0x84)	# 16bits. 0x84-0x85: SDRAM ASR; 0x86-0x87:reserved
#define SDRAM_MSR	(CONFIGDATA_BASE+0x88)	# 16bits. 0x88-0x89: SDRAM MSR; 0x8A-0x8B:reserved
#define FLASH_CTRL1	(CONFIGDATA_BASE+0x8C)	# 32bits. sflash ctrl interface
#define FLASH_MCR1	(CONFIGDATA_BASE+0x90)	# 32bits. flash memory control reg
#define FLASH_CTRL2	(CONFIGDATA_BASE+0x94)	# 32bits. sflash ctrl interface
#define FLASH_MCR2	(CONFIGDATA_BASE+0x98)	# 32bits. flash memory control reg
#define BOOT_LOGO	(CONFIGDATA_BASE+0x9C)	# 16bits. 
			# onoff:bit 15		1:on / 0:off
			# tv format:bit 12~14	0:PAL / 1:NTSC / 2:PAL_M / 3:PAL_N / 4:PAL_60 / 5:NTSC_443
			# source picture height:bit 0~11
			# For example : if open logo in bootload, and display format is PAL, and source picture is 480, then it should be set to 0x81e0.
#define CRYSTAL_TYPE	(CONFIGDATA_BASE+0x9E)	# 8bit. 0:27M; 1:13.5M
#define PAN_TYPE	(CONFIGDATA_BASE+0x9F)	# 3bits.[bit7-bit5] 0:hwscan; 1:compan
#define PAN_TYPE_MASK	0xE0
#define DVBC_SUPPORT	(CONFIGDATA_BASE+0x9F)	# 2bit.[bit4-bit3]. 0:DVBS; 1:DVBC/DVBT;
#define DVBC_SUPPORT_MASK 0x18
#define NEC_INVERT	(CONFIGDATA_BASE+0x9F)	# 1bit.[bit2]. 1:support nec invert ir; 0: not suppurt nec invert ir
#define NEC_INVERT_MASK	0x04


#define EXT_VERSION		(CONFIGDATA_BASE+0x100)	# 2bytes
#define	PAN_HW_INFO_BASE (CONFIGDATA_BASE+0x104)	# 60bytes.	reference struct pan_hw_info
#define STBY_PAN_TYPE	(CONFIGDATA_BASE+0x140)	# 1byte 	1--show timer; 2--show off; 3--show blank
#define BOOT_TYPE	(CONFIGDATA_BASE+0x141) # 1bit.[bit7]	0:bootup; 1:enter into standby after cold boot.
#define BOOT_TYPE_MASK	0x80
#define PAN_KEY_NUM	(CONFIGDATA_BASE+0x141) # 3bits.[bit6-bit4] 0:one panel standby key; 1:two key; other: not define now
#define PAN_KEY_NUM_MASK	0x70
#define IRP_KEY_NUM	(CONFIGDATA_BASE+0x141) # 4bits.[bit3-bit0] 0:one ir standby key; 1:two key; 2:three key; 3: four key; other: not define now
#define IRP_KEY_NUM_MASK	0x0F
#define LED_ARRAY_BASE	(CONFIGDATA_BASE+0x150)	# 16bytes.	'0','1','2','3','4','5','6','7','8','9','o','F',':','-','_',' '
#define IRP_KEY_STANDBY1	(CONFIGDATA_BASE+0x160)	# 32bits.
#define IRP_KEY_STANDBY2	(CONFIGDATA_BASE+0x164)	# 32bits.
#define IRP_KEY_STANDBY3	(CONFIGDATA_BASE+0x168)	# 32bits.
#define IRP_KEY_STANDBY4	(CONFIGDATA_BASE+0x16C)	# 32bits.
#define PAN_KEY_STANDBY1	(CONFIGDATA_BASE+0x170)	# 32bits.
#define PAN_KEY_STANDBY2	(CONFIGDATA_BASE+0x174)	# 32bits.
#define PAN_KEY_STANDBY3	(CONFIGDATA_BASE+0x178)	# 32bits.
#define PAN_KEY_STANDBY4	(CONFIGDATA_BASE+0x17C)	# 32bits.


#define	SCB_NUM		1

#define	SETING_ADDR		0x48
#define	DIG0_ADDR		0x68
#define	DIG1_ADDR		0x6a
#define	DIG2_ADDR		0x6c
#define	DIG3_ADDR		0x6e

#define	KEY_ADDR		0x4f

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

# MACRO: get_led_array(reg dr)
#  reg dr: load led_array to dr.
#
		.macro	get_led_array dr
		li		\dr, LED_ARRAY_BASE
		.endm
/*
# MACRO: get_ir_power_key_num(reg dr)
#  reg dr: load ir power key number to dr.
#
		.macro	get_ir_power_key_num dr
		lb		\dr, IRP_KEY_NUM
		andi		\dr, IRP_KEY_NUM_MASK
		.endm

# MACRO: get_fp_power_key_num(reg dr)
#  reg dr: load fp power key number to dr.
#
		.macro	get_fp_power_key_num dr
		lb		\dr, PAN_KEY_NUM
		andi		\dr, PAN_KEY_NUM_MASK
		.endm
				
# MACRO: get_ir_power_key(reg dr, reg index)
#  reg dr: load No. index ir power key to dr.
#
		.macro	get_ir_power_key dr, index
		beq		\index, 0, 90f
		nop
		beq		\index, 1, 91f
		nop
		beq		\index, 2, 92f
		nop
93:
		lw		\dr, IRP_KEY_STANDBY4	# get ir standby key 4
		b		99f
		nop
92:
		lw		\dr, IRP_KEY_STANDBY3	# get ir standby key 3
		b		99f
		nop
91:
		lw		\dr, IRP_KEY_STANDBY2	# get ir standby key 2
		b		99f
		nop
90:		
		lw		\dr, IRP_KEY_STANDBY1	# get ir standby key 1
99:		
		.endm
		
# MACRO: get_fp_power_key(reg dr, reg index)
#  reg dr: load No. index fp power key to dr.
#
		.macro	get_fp_power_key dr, index
		beq		\index, 0, 90f
		nop
		beq		\index, 1, 91f
		nop
		beq		\index, 2, 92f
		nop
93:
		lw		\dr, PAN_KEY_STANDBY4	# get pan standby key 4
		b		99f
		nop		
92:		
		lw		\dr, PAN_KEY_STANDBY3	# get pan standby key 3
		b		99f
		nop		
91:		
		lw		\dr, PAN_KEY_STANDBY2	# get pan standby key 2
		b		99f
		nop		
90:
		lw		\dr, PAN_KEY_STANDBY1	# get pan standby key 1
99:
		.endm
*/

# MACRO: get_pan_hw_info(reg dr)
#  reg dr: load pan_hw_info to dr.
#
		.macro	get_pan_hw_info dr
		li		\dr, PAN_HW_INFO_BASE
		.endm

# MACRO: gettimerdata(reg sr)
#  reg dr: get timefield to dr
#
		.macro	gettimerdata dr
		li		t1, STANDBY_TIMER
		lw		\dr, 0(t1)
		.endm

# MACRO: settimerdata(reg sr)
#  reg dr: set sr(time) to  timefield.
#
		.macro	settimerdata sr
		li		t1, STANDBY_TIMER
		sw		\sr,0(t1)	
		.endm

# MACRO: get_pan_type(reg dr)
#  reg dr: load pan_type to dr.
#
		.macro	get_pan_type dr
		lb		\dr, PAN_TYPE
		andi		\dr, PAN_TYPE_MASK
		srl		\dr, 5
		.endm
/*
# MACRO: gettimercompare1s(reg dr)
#  reg dr: Output 1s.
	.macro	gettimercompare1s dr
		lb		\dr, CRYSTAL_TYPE		# check crystal
		beq		\dr, 1, 91f 	# 13M crystal
		nop
		beq		\dr, 2, 92f	# 28.8M crystal
		nop
		li		\dr,  6750000			# (27000000/4)
		b		93f
		nop
91:
		li		\dr, 421875			# (13500000/32)
		b 		93f
		nop
92:
		li		\dr,  900000			# (28800000/32)
93:
	.endm
*/
#--------------------------------------------------------------------
#
#   Function_Name: sys_preboot
#     This function used only for board dedicat issue patch, just as
#     turn off LED GPIO, if you want do other patch about chipset,
#     please added into _Init_Chipset in boot.S.
#--------------------------------------------------------------------
		.globl  sys_preboot
		.ent	sys_preboot
		.set noreorder
sys_preboot:
#######force into standby mode
#		li	s8, 0xdeadbeef
		jr		ra
		nop
		.set 	reorder
		.end	sys_preboot

		.globl  sys_postboot
		.ent	sys_postboot
		.set noreorder
sys_postboot:
		jr		ra
		nop
		.set 	reorder
		.end	sys_postboot
				
#--------------------------------------------------------------------
#   Function_Name: sys_get_pinconfig(int chiptype)
#     This function used only for costomer to read in strap pins 
#     config, and do some further config according it.
#	chiptype =	1 : M3329
#							2 : M3329C		
#	Returns: v0: strap pin soft-control info
#					bit2: mem control 	0: disable, 1: enable
#					bit1: mem clock: 	0: 135M, 1: 120M
#					bit0: mem size:		0: 8MB, 1: 16MB
#--------------------------------------------------------------------
		.globl  sys_get_pinconfig
		.ent	sys_get_pinconfig
		.set noreorder		
sys_get_pinconfig:
		jr		ra
		nop				
		.set 	reorder
		.end	sys_get_pinconfig

//LED_ARRAY_BASE:	//# 16bytes.	'0','1','2','3','4','5','6','7','8','9','o','F',':','-','_',' '
//{'0', 0x3f}, {'1', 0x06}, {'2', 0x5b}, {'3', 0x4f}, 
//{'4', 0x66}, {'5', 0x6d}, {'6', 0x7d}, {'7', 0x07}, 
//{'8', 0x7f}, {'9', 0x6f},  {'o', 0x5c},{'F', 0x71},
//{':', 0x80}, {'-', 0x40}, {'_', 0x08}, {' ', 0x00},
#define led_array_0	0x3f
#define led_array_1	0x06
#define led_array_2	0x5b
#define led_array_3	0x4f
#define led_array_4	0x66
#define led_array_5	0x6d
#define led_array_6	0x7d
#define led_array_7	0x07
#define led_array_8	0x7f
#define led_array_9	0x6f
#define led_array_a	0x5c
#define led_array_b	0x71
#define led_array_c	0x80
#define led_array_d	0x40
#define led_array_e	0x08
#define led_array_f	0x00	

		.globl	ini_led_array
		.ent	ini_led_array
		.set	noreorder
ini_led_array:
	li	t0,LED_ARRAY_BASE
	li	t1,led_array_0		// 0
	sb	t1,0x00(t0)
	li	t1,led_array_1
	sb	t1,0x01(t0)
	li	t1,led_array_2
	sb	t1,0x02(t0)
	li	t1,led_array_3
	sb	t1,0x03(t0)
	li	t1,led_array_4
	sb	t1,0x04(t0)
	li	t1,led_array_5
	sb	t1,0x05(t0)
	li	t1,led_array_6
	sb	t1,0x06(t0)
	li	t1,led_array_7
	sb	t1,0x07(t0)
	li	t1,led_array_8
	sb	t1,0x08(t0)
	li	t1,led_array_9
	sb	t1,0x09(t0)
	li	t1,led_array_a
	sb	t1,0x0a(t0)
	li	t1,led_array_b
	sb	t1,0x0b(t0)
	li	t1,led_array_c
	sb	t1,0x0c(t0)
	li	t1,led_array_d
	sb	t1,0x0d(t0)
	li	t1,led_array_e
	sb	t1,0x0e(t0)
	li	t1,led_array_f
	sb	t1,0x0f(t0)
	jr		ra
	nop				
	.set 	reorder
	.end	sys_get_pinconfig
		
/***************************************************************
*Function Name:	sys_prestandby
*Arguments: 	
*		
*Description:	Initialize IR & I2C before goes into standby mode.
*
***************************************************************/
		.globl	sys_prestandby
		.ent	sys_prestandby
		.set	noreorder
sys_prestandby:
		move	k0, ra				
		nop
		
		jal	ini_led_array				# Init IR
		nop
#if(SCB_NUM == 0)		
	li	t0, 0xb8018200 //I2C_SCB_BASE_0
	lw	t1,0x00(t0)
	sw	t1,SCB_REG0
	lw	t1,0x04(t0)
	sw	t1,SCB_REG4
	lw	t1,0x08(t0)
	sw	t1,SCB_REG8
#endif	
#if(SCB_NUM == 1)		
	li	t0, 0xb8018700 //I2C_SCB_BASE_1
	lw	t1,0x00(t0)
	sw	t1,SCB_REG0
	lw	t1,0x04(t0)
	sw	t1,SCB_REG4
	lw	t1,0x08(t0)
	sw	t1,SCB_REG8
#endif	
	
####### Check i2c type(GPIO or SCB) and its information here.
1:
		jal 	i2c_scb_init_s			# Init SCB
		li	a0, SCB_NUM

//ch455 mode
//0	[INTENS]	[7SEG]	[SLEEP]	0	[ENA]
//0	000		0		0		0	1
		li	a1, SETING_ADDR			
		li	a2, ((1<<4)|(1<<0))
		jal	i2c_scb_write_s
		li	a0, SCB_NUM

		b	3f
		nop
		
####### Init GPIO type of I2C.(not supported right now)
2:
#		jal 	i2c_gpio_init_s			# Init GPIOs
		nop
3:		
		move	ra, k0

		b  1f				# 1--show timer
		nop
		lb	t1, STBY_PAN_TYPE		
		beq	t1, 0, 0f				# 0--show "----"	
		nop
		beq	t1, 1, 1f				# 1--show timer
		nop
		beq	t1, 2, 2f				# 2--show "off"
		nop
		beq	t1, 3, 3f				# 3--show blank
		nop
		b	5f
		nop
0:
		li	s6, 0xdddd			# "----" index
		b	5f
		nop
1:
		li	t0, (0x1f<<12)		# Hour in ASCII
		and 	v0, t0, s4
		srl 	v0, 12
		bin2ascii v0, t5
		sll 	t5, 8
		li	t0,(0x3f<<6)			# Minute in ASCII
		and 	v0, t0, s4
		srl 	v0, 6
		bin2ascii v0, t4
		or	s6, t4, t5		
		settimerdata s6
		b	5f
		nop
2:
		li	s6, 0xabbf			# "OFF " index
		b	5f
		nop
3:
		li	s6, 0xffff				# "    " index
5:
		move	ra, k0
		jr	ra
		nop
		.end sys_prestandby


		.globl	sys_poststandby
		.ent	sys_poststandby
		.set noreorder
sys_poststandby:
		move	k1, ra

#		get_pan_type	t1
#		bnez		t1, 2f
#		nop
	
//ch455 mode
//0	[INTENS]	[7SEG]	[SLEEP]	0	[ENA]
//0	000		0		0		0	1
		li	a1, SETING_ADDR			
		li	a2, ((0<<4)|(1<<0))
		jal	i2c_scb_write_s
		li	a0, SCB_NUM
		nop
		
#if(SCB_NUM == 0)		
	li	t0, 0xb8018200 //I2C_SCB_BASE_0
	lw	t1,SCB_REG0
	sw	t1,0x00(t0)
	lw	t1,SCB_REG4
	sw	t1,0x04(t0)
	lw	t1,SCB_REG8
	sw	t1,0x08(t0)
#endif	
#if(SCB_NUM == 1)		
	li	t0, 0xb8018700 //I2C_SCB_BASE_1
	lw	t1,SCB_REG0
	sw	t1,0x00(t0)
	lw	t1,SCB_REG4
	sw	t1,0x04(t0)
	lw	t1,SCB_REG8
	sw	t1,0x08(t0)
#endif	

		move	ra, k1
		jr		ra
		nop
		.set reorder
		.end sys_poststandby

/***************************************************************
*Function Name:	sys_standby_process
*		
*Description:			
*
***************************************************************/
		.globl	sys_standby_process
		.ent	sys_standby_process
		.set	noreorder	
sys_standby_process:
		move	k0, ra

_led_display:	
	//	lb	t1, STBY_PAN_TYPE
	//	bne	t1, 1, 2f					# if not show timer, jump to 2f
		nop
		
		gettimerdata	t0
		move		s6, t0

2:
		
	//	li	s6, 0xabbf			# "OFF " index
#######Display LED 3.
		srl	t0, s6, 0
		andi	t0, 0x0f
		get_led_array	t4				# Turn BCD code to Display bitmap
		addu	t1, t4, t0
		lbu 	t0, (t1)
		
		li	a1, DIG3_ADDR			# Display DIG3
		or	a2, t0, t0
		jal	i2c_scb_write_s
		li	a0, SCB_NUM		
#######Display LED 2.
		srl	t0, s6, 4
		andi	t0, 0x0f	
		get_led_array	t4				# Turn BCD code to Display bitmap
		addu	t1, t4, t0
		lbu 	t0, (t1)
		
		li	a1, DIG2_ADDR			# Display DIG2
		or	a2, t0, t0
		jal	i2c_scb_write_s
		li	a0, SCB_NUM	
#######Display LED 1.
		srl	t0, s6, 8
		andi	t0, 0x0f	
		get_led_array	t4				# Turn BCD code to Display bitmap
		addu	t1, t4, t0
		lbu 	t0, (t1)

		li	t1, 0x01
		and	t1, s4
		beqz	t1, _no_dot
		nop

	//	lb	t1, STBY_PAN_TYPE
	//	bne	t1, 1, _no_dot					# if not show timer, jump to _no_dot
		nop		

		ori	t0, 0x80
		
_no_dot:	
		li	a1, DIG1_ADDR			# Display DIG1
		or	a2, t0, t0
		jal	i2c_scb_write_s
		li	a0, SCB_NUM		
#######Display LED 0.
		srl	t0, s6, 12
		andi	t0, 0x0f	
		get_led_array	t4				# Turn BCD code to Display bitmap
		addu	t1, t4, t0
		lbu 	t0, (t1)

		li	a1, DIG0_ADDR			# Display DIG0
		or	a2, t0, t0
		jal	i2c_scb_write_s
		li	a0, SCB_NUM
		
		move	ra, k0
		jr	ra
		nop


		.set reorder
		.end sys_standby_process



