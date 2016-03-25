 #--------------------------------------------------------------------
#
#  Copyright (C) 2005 Acer Laboratories Inc.  All Rights Reserved.
#
#  File:   chip.S
#
#  Contents: About chip support function. Those function should running in RAM.
#
#  History:
#      Date          By         Reason
#   ===========    =======   ========================
#   1/17/2006      Justin Wu     Split from boot.S
#--------------------------------------------------------------------

#include "mips.h"
#include "machine.h"
#include <alidefinition/adf_sysdef.h>


# -------------------------------------------------------------------

#define SYS_FLASH_BASE_ADDR 0xafc00000
#define SYS_CHIP_MODULE ALI_S3602
#define SYS_CPU_MODULE  CPU_M6303

#if (SYS_CHIP_MODULE == ALI_S3601)
#define SYS_IC_NB_BASE_H		0xb800
#define SYS_IC_NB_LDCR			0x60
#define SYS_IC_NB_LDRCR			0x6c
#define SYS_IC_NB_SMER			0x68
#define SYS_IC_NB_FMCR			0x90
#define SYS_IC_NB_EISR			0x30
#define SYS_IC_NB_EIMR			0x34

#define SYS_IC_SB_BASE_H		0xb801
#define SYS_IC_SB_IRC_CFG		0x8100
#define SYS_IC_SB_IRC_FCTRL		0x8101
#define SYS_IC_SB_IRC_TTHR		0x8102
#define SYS_IC_SB_IRC_NTHR		0x8103
#define SYS_IC_SB_IRC_IER		0x8106
#define SYS_IC_SB_IRC_ISR		0x8107
#define SYS_IC_SB_IRC_DATA		0x8108
#elif ((SYS_CHIP_MODULE == ALI_S3602) && (SYS_CPU_MODULE != CPU_M6303))
#define SYS_IC_NB_BASE_H		0xb800
#define SYS_IC_NB_LDCR			0x60
#define SYS_IC_NB_LDRCR			0x80
#define SYS_IC_NB_SMER			0x68
#define SYS_IC_NB_FMCR			0x90
#define SYS_IC_NB_EISR1			0x30
#define SYS_IC_NB_EISR2			0x34
#define SYS_IC_NB_EIMR1			0x38
#define SYS_IC_NB_EIMR2			0x3c

#define SYS_IC_SEE_NB_BASE_H	0xb804

#define SYS_IC_SB_BASE_H		0xb801
#define SYS_IC_SB_IRC_CFG		0x8100
#define SYS_IC_SB_IRC_FCTRL		0x8101
#define SYS_IC_SB_IRC_TTHR		0x8102
#define SYS_IC_SB_IRC_NTHR		0x8103
#define SYS_IC_SB_IRC_IER		0x8106
#define SYS_IC_SB_IRC_ISR		0x8107
#define SYS_IC_SB_IRC_DATA		0x8108
#else
#define SYS_IC_NB_BASE_H		0xb800
#define SYS_IC_NB_LDCR			0x60
#define SYS_IC_NB_LDRCR			0x60
#define SYS_IC_NB_SMER			0x68
#define SYS_IC_NB_FMCR			0x90
#define SYS_IC_NB_EISR			0x38
#define SYS_IC_NB_EIMR			0x3c

#define SYS_IC_SB_BASE_H		0xb800
#define SYS_IC_SB_IRC_CFG		0x1100
#define SYS_IC_SB_IRC_FCTRL		0x1101
#define SYS_IC_SB_IRC_TTHR		0x1102
#define SYS_IC_SB_IRC_NTHR		0x1103
#define SYS_IC_SB_IRC_IER		0x1106
#define SYS_IC_SB_IRC_ISR		0x1107
#define SYS_IC_SB_IRC_DATA		0x1108
#endif

# -------------------------------------------------------------------

//Defintions specified for C3603 24KE 
#define SYS_IC_NB_BASE_H		0xb800
#define SYS_IC_NB_LDCR			0x60
# #define SYS_IC_NB_LDRCR			0x80
#define SYS_IC_NB_SMER			0x68
#define SYS_IC_NB_FMCR			0x90
#define SYS_IC_NB_EISR1			0x30
#define SYS_IC_NB_EISR2			0x34
#define SYS_IC_NB_EIMR1			0x38
#define SYS_IC_NB_EIMR2			0x3c

# #define SYS_IC_SB_BASE_H		0xb801
# #define SYS_IC_SB_IRC_CFG		0x8100
# #define SYS_IC_SB_IRC_FCTRL		0x8101
# #define SYS_IC_SB_IRC_TTHR		0x8102
# #define SYS_IC_SB_IRC_NTHR		0x8103
# #define SYS_IC_SB_IRC_IER		0x8106
# #define SYS_IC_SB_IRC_ISR		0x8107
# #define SYS_IC_SB_IRC_DATA		0x8108

/********************************************** 
*******  CHIP MACRO FROM sys_define.h  ********
**********************************************/

#define HW_TYPE_CHIP			  0x00010000
#define HW_TYPE_CHIP_REV		0x00020000

#define ALI_S3602				  (HW_TYPE_CHIP + 70)
#define ALI_S3602F				(HW_TYPE_CHIP + 71)
#define ALI_M3327				  (HW_TYPE_CHIP + 4)
#define ALI_M3329E				(HW_TYPE_CHIP + 6)
#define ALI_M3327C				(HW_TYPE_CHIP + 7)

#define IC_REV_0				(HW_TYPE_CHIP_REV + 1)
#define IC_REV_1				(HW_TYPE_CHIP_REV + 2)
#define IC_REV_2				(HW_TYPE_CHIP_REV + 3)
#define IC_REV_3				(HW_TYPE_CHIP_REV + 4)
#define IC_REV_4				(HW_TYPE_CHIP_REV + 5)
#define IC_REV_5				(HW_TYPE_CHIP_REV + 6)
#define IC_REV_6				(HW_TYPE_CHIP_REV + 7)
#define IC_REV_7				(HW_TYPE_CHIP_REV + 8)
#define IC_REV_8				(HW_TYPE_CHIP_REV + 9)

#define SYS_DEFINE_NULL			0x00000000	/* NULL define */


# -----------------------------///////////------------------------------------


# --------------------------------------------------------------------
#   Function_Name: IRQ_DisableInterrupt
#   Description: Disable interrupts
#   Arguments: None
#   Return Value: None
# --------------------------------------------------------------------
        .set	noreorder
		.globl  IRQ_DisableInterrupt
        .ent	IRQ_DisableInterrupt
# disable all maskable interrupt
IRQ_DisableInterrupt:

       # li      t0, 1                   # Set this flag to avoid task
       # sw      t0, bInSys              # switch before finished.

        mfc0    t0, C0_STATUS            # Get CP0 Status Register
        nop
        nop
        nop
        li      t1, 6

        and     t1, t0                   # Check if ECL or EXL is set
        bne     t1, zero, no_disable

        li      t1, ~SR_IEC
        and     t0, t1
        mtc0    t0, C0_STATUS            # Clear IEC bit in C0_SR
        nop
        nop

        mtc0    t0, C0_STATUS            # Clear IEC bit in C0_SR again! to avoid a interrupt is raised  
        nop                              # when disable interrupt is not working right now in S3602
        nop
no_disable:
       # sw      zero, bInSys            # Clear the flag
        jr      ra
        nop

        .end    IRQ_DisableInterrupt


# --------------------------------------------------------------------
#   Function_Name: IRQ_EnableInterrupt
#   Description: Enable interrupts
#   Arguments: None
#   Return Value: None
# --------------------------------------------------------------------
	.globl  IRQ_EnableInterrupt
	.ent	IRQ_EnableInterrupt
IRQ_EnableInterrupt:
	    # lw      t0, g_cIntMask          # need removed
	    # bne     t0, zero, no_enable 	# needed by cycle timer
	    # nop

		mfc0    t0, C0_STATUS           # Get CP0 Status Register
        nop
        nop
        nop
        li      t1, 6

        and     t1, t0                  # Check if ECL or EXL is set
        bne     t1, zero, no_enable

        or      t0, SR_IEC              # Set IEC bit in C0_SR
        mtc0    t0, C0_STATUS
        nop
        nop
        nop

no_enable:
        jr      ra
        nop

       .end    IRQ_EnableInterrupt




        .set    noreorder

 #--------------------------------------------------------------------
 #
 #   void sys_ic_reboot()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_reboot
        .ent    sys_ic_reboot

sys_ic_reboot:
#if(SYS_CHIP_MODULE == ALI_S3602)
        jal sys_ic_set_half_mode_strap_pin
        nop
        li	t0, 0xb8000000              #reset serial flash register to protect serial flash
        lhu t1, 2(t0)           
        li	t3, 0x3603
        bne t1, t3, 1f
        nop
        li  t0, 0xb802e000
1:
        li	t1, 0xc2000d03
        sw	t1, 0x98(t0)
#else
        li	t0, 0xb8000000              #reset serial flash register to protect serial flash
        lh	t1, 2(t0)
        li	t2, 0x3329
        bne	t1, t2, 1f
        nop
        lbu	t1, 0(t0)
        sltiu		t1, t1, 0x5
        bne		zero, t1, 1f
        nop
        ori		t0, 0x8000
1:        
        li	t1, 0x02000d03
        sw	t1, 0x98(t0)
#endif        
        mfc0    t0, C0_STATUS			# Get CP0 Status Register
        nop
        nop
        nop
        li      t1, ~SR_IEC
        and     t0, t1
        mtc0    t0, C0_STATUS           # Clear IEC bit in C0_SR
	lui	t0, SYS_IC_NB_BASE_H
#if ((SYS_CHIP_MODULE == ALI_S3602) && (SYS_CPU_MODULE != CPU_M6303))
	sw	zero, SYS_IC_NB_EIMR1(t0)
	sw	zero, SYS_IC_NB_EIMR2(t0)
		lui     t1, SYS_IC_SEE_NB_BASE_H
	sw		$0, SYS_IC_NB_EIMR1(t1)
	sw		$0, SYS_IC_NB_EIMR2(t1)		

	
#else
	sw	zero, SYS_IC_NB_EIMR(t0)
#endif
#if(SYS_CHIP_MODULE==ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602 )
	#if (SYS_CPU_MODULE != CPU_M6303)
		li		t1, 0xffbfffff			# Reset all device except RTC
	#else
		li		t1, 0x000001ff			# Reset all device
	#endif
#else
		li		t1, 0x0fffffff			# Reset all device
#endif
		sw		t1, SYS_IC_NB_LDRCR(t0)
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		move	t1, zero
		sw		t1, SYS_IC_NB_LDRCR(t0)
		sw		zero, SYS_IC_NB_LDCR(t0)	# Enable IC device clock

	# mapping 0xAFC00000 to Nor flash start address
	lbu   t9, 0x223(t0)
	andi  t9, 0xfe
	sb    t9, 0x223(t0)

		li		t2, 0x10
2:		bnez	t2,2b
		addiu	t2,-1

        li      v0, SYS_FLASH_BASE_ADDR
        jr      v0
        nop

        .end    sys_ic_reboot


            .globl  sys_ic_set_half_mode_strap_pin
        .ent    sys_ic_set_half_mode_strap_pin
sys_ic_set_half_mode_strap_pin:
		la t5, half_mode_registor_value
		li t0,0x01
		lw t7,(t5)
		bne t7,t0,1f
		li t1,0x20010000
		li t3,0xb8000070
		lw t4,(t3)
		or t4,t4,t1
		li t3,0xb8000074
		sw t4,(t3)
 1:       
 		jr		ra
		nop
        .end    sys_ic_set_half_mode_strap_pin

 
#--------------------------------------------------------------------
 #
 #   void soc_sys_ic_enter_standby(UINT32 expect_time, UINT32 current_time)
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_enter_standby
        .ent    sys_ic_enter_standby
sys_ic_enter_standby:
		
#if(SYS_CHIP_MODULE == ALI_S3602)
        li	t0, 0xb8000000              #reset serial flash register to protect serial flash
        lhu t1, 2(t0)           
        li	t3, 0x3603
        bne t1, t3, 1f
        nop
        li  t0, 0xb802e000
1:
        li	t1, 0xc2000d03
        sw	t1, 0x98(t0)
#endif

	    mfc0    t0, C0_STATUS           # Get CP0 Status Register
        nop
        nop
        nop
        li      t1, ~SR_IEC
        and     t0, t1
        mtc0    t0, C0_STATUS           # Clear IEC bit in C0_SR
		lui		t0, SYS_IC_NB_BASE_H
#if ((SYS_CHIP_MODULE == ALI_S3602) && (SYS_CPU_MODULE != CPU_M6303))
		sw		$0, SYS_IC_NB_EIMR1(t0)
		sw		$0, SYS_IC_NB_EIMR2(t0)
		
				lui     t1, SYS_IC_SEE_NB_BASE_H
		sw		$0, SYS_IC_NB_EIMR1(t1)
		sw		$0, SYS_IC_NB_EIMR2(t1)	

#else
		sw		$0, SYS_IC_NB_EIMR(t0)
#endif
		nop
		li		t2, 0x200
1:		addiu	t2,-1
		nop
		bnez	t2,1b
		nop
		li		s8, 0xdeadbeef			# IC standby func ID
		move	s7, a0
		move	s5, a1		
##M3101 Configuration  Parameter Send To BootLoader.All other projects just do not care.
		move	gp, a2
	 	nop
	 	nop
	 	nop
		# mapping 0xAFC00000 to Nor flash start address
	#	lbu   t9, 0x223(t0)
	#	andi  t9, 0xfe
	#	sb    t9, 0x223(t0)

	#	li		t2, 0x10
2:	#	bnez	t2,2b
	#	addiu	t2,-1

		li 	t0,	0xb8000223
		li  t1,    0xFE
		sw  t1, 0(t0)
		nop
		lw  t1,	0(t0)
	 	li		t0, SYS_FLASH_BASE_ADDR
	 	jr		t0
		nop

        jr      ra
        nop
		.end	sys_ic_enter_standby


# /////////////////////////////////////////

        .set    noreorder
 
 #--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_get_chip_id()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_chip_id
        .ent    sys_ic_get_chip_id
sys_ic_get_chip_id:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lh		t1, 2(t0)
		beq		t1, 0, 1f				# M3327A/B Serial: 0x0000
		li		AT, 0x3327
		beq		t1, AT, 2f				# M3327C/N/D/T/E Serial: 0x3327
		li		AT, 0x3101
		beq		t1, AT, 2f
		li		AT, 0x3202
		beq		t1, AT, 2f				# M3101 Serial: 0x3101, look as M3327C
		li		AT, 0x3329
		beq		t1, AT, 3f				# M3101 Serial: 0x3101, look as M3327C
		li		AT, 0x3602
		beq		t1, AT, 4f				#
		nop
		li		AT, 0x3603
		beq		t1, AT, 5f				#
		nop
		li		AT, 0x3901
		beq		t1, AT, 6f				#
		nop		
		li		AT, 0x3701
		beq		t1, AT, 7f				#
		nop	
		li		AT, 0x3503
		beq		t1, AT, 8f				#
		nop	
		b		9f						# Other chip
		nop
1:		li		v0, ALI_M3327 
		b		10f
		nop
2:		li		v0, ALI_M3327C
		b		10f
		nop
3:		li		v0, ALI_M3329E
		b		10f
		nop
4:		li		v0, ALI_S3602
		b		10f
		nop	
5:		li		v0, ALI_S3602F
		b		10f
		nop
6:		li		v0, ALI_S3901
		b		10f
		nop
7:		li		v0, ALI_C3701
		b		10f
		nop	
8:		li		v0, ALI_S3503
		b		10f
		nop
9:		li		v0, SYS_DEFINE_NULL
10:		jr      ra
        nop
		.set    at
		.end	sys_ic_get_chip_id

        .set    noat

 #--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_get_rev_id()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_rev_id
        .ent    sys_ic_get_rev_id
sys_ic_get_rev_id:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lh		t1, 2(t0)
		beq		t1, 0x0000, check_27_id	# If isn't M3327C, goto M3327 check
		nop
		li		t2, 0x3329
		beq		t1, t2, check_29e_id
		nop
		li		t2, 0x3202
		beq		t1, t2, check_3202_id
		nop
		li		t2, 0x3602
		beq		t1, t2, check_3602_id
		nop
		li		t2, 0x3603
		beq		t1, t2, check_3603_id
		nop
		li              t2, 0x3701
		beq             t1, t2, check_3701_id
		nop
		li              t2, 0x3503
		beq             t1, t2, check_3503_id
		nop
		beq		t1, 0x0000, check_27c_id	# If isn't M3601, goto M3327c check
		nop
		li		v0, 0xffffffff
		jr		ra
		nop

check_27c_id:		
		lbu		t1, 0(t0)
		li		AT, 0x60
		beq		t1, AT, 1f				# M3327C: 0x60
		li		AT, 0x63
		beq		t1, AT, 1f				# S3327C: 0x63
		li		AT, 0x70
		beq		t1, AT, 2f				# S3327N, M3327D: 0x70
		li		AT, 0x30
		beq		t1, AT, 2f				# S3327D1: 0x30
		li		AT, 0xf0
		beq		t1, AT, 3f				# S3327D2, S3327T: 0xf0
		li		AT, 0xf1
		beq		t1, AT, 4f				# S3327E1: 0xf1;
		nop
		b		5f						# M3327E: 0xf2. or higher
check_29e_id:		
		lbu		t1, 0(t0)
		li		v0, IC_REV_0			
		add		v0, v0, t1				
		b		9f					
		nop
check_3202_id:		
		lbu		t1, 0(t0)
		li		v0, IC_REV_0			
		add		v0, v0, t1				
		b		9f					
		nop
check_27_id:
		lh		t1, 0x86(t0)
		andi	t1, 0xC280
		beq		t1, 0, 1f				# M3327 A0A: 0x0000
		li		AT, 0x0080
		beq		t1, AT, 2f				# M3327 A0B: 0x0080
		li		AT, 0x8080
		beq		t1, AT, 3f				# M3327 B0A: 0x8080
		li		AT, 0x0280
		beq		t1, AT, 4f				# M3327 A0C: 0x0280
		nop
		b		5f						# M3327 B0B: 0xC080. or higher
		nop
check_3602_id:
		lbu		t1, 0(t0)
		andi	t1, 0x3f
		li		v0, IC_REV_0			
		add		v0, v0, t1				
		b		9f					
		nop

check_3603_id:
		lbu		t1, 0(t0)
		andi	t1, 0xff
		beq		t1, zero, 1f			# S3602F A0: 0x3603XX00
		nop
		b		2f
		nop
		
check_3701_id:
        lbu		t1, 0(t0)
		andi	t1, 0xff
		beq		t1, zero, 1f			# S3602F A0: 0x3701XX00
		nop
		b		2f
		nop

check_3503_id:
                lbu		t1, 0(t0)
		andi    	t1, 0xff
		beq		t1, zero, 1f			# S3503 A0: 0x3503XX00
		nop
		b		2f
		nop
        
1:		li		v0, IC_REV_0
		b		9f
		nop
2:		li		v0, IC_REV_1
		b		9f
		nop
3:		li		v0, IC_REV_2
		b		9f
		nop
4:		li		v0, IC_REV_3
		b		9f
		nop
5:		li		v0, IC_REV_4
9:		jr      ra
        nop
		.set    at
		.end	sys_ic_get_rev_id

        .set    noat


 #--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_get_cpu_clock()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_cpu_clock
        .ent    sys_ic_get_cpu_clock
sys_ic_get_cpu_clock:

		.set    noat

		move t3,ra
		
		jal sys_ic_get_chip_id
		nop
		li AT,ALI_S3602F
		beq AT,v0, f36
		nop
		
		lui		t0, SYS_IC_NB_BASE_H
		lw		t1, 0x70(t0)
		srl		t1, t1, 7

		andi	t1, 0x7
		beq		t1, 0, 1f				# CPU CLOCK 600M
		li		AT, 1
		beq		t1, AT, 2f				# CPU CLOCK 450M
		li		AT, 2
		beq		t1, AT, 3f				# CPU CLOCK 396M
		nop
		li		v0, 297				# CPU CLOCK 297M	
		b		9f
		nop
1:		li		v0, 600
		b		9f
		nop
2:		li		v0, 450
		b		9f
		nop
3:		li		v0, 396
		b 		9f
		nop
f36:	
		lui		t0, SYS_IC_NB_BASE_H
		lw		t1, 0x70(t0)        ## get cpu clock of M3603
		srl		t1, t1, 7

		andi	t1, 0x3
		beq		t1, 0, 1f				# CPU CLOCK 396M
		li		AT, 3
		beq		t1, AT, 2f				# CPU CLOCK 198M
		nop
		b	3f										# CPU CLOCK 296M	
		nop
1:	li		v0, 396
		b		9f
		nop
2:		
		li		v0, 198
		b 	9f
		nop
3:		
		li		v0, 298
9:		
		move ra,t3
		jr      ra
        nop
		.set    at
		.end	sys_ic_get_cpu_clock

 #--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_get_dram_clock()
 #
 #   Note:
 #	M3327 uses three bits MEM_CLK[2: 0] to config MEM_CLK,
 #	M3101 ONLY uses one bit MEM_CLK[0] to config MEM_CLK.
 #
 #--------------------------------------------------------------------
 
        .globl  sys_ic_get_dram_clock
        .ent    sys_ic_get_dram_clock
sys_ic_get_dram_clock:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lw		t1, 0x70(t0)
		srl		t1, t1, 5

		andi	t1, 0x3
		beq		t1, 0, 1f				# MEM CLOCK 198M
		li		AT, 1
		beq		t1, AT, 2f				# MEM CLOCK 162M
		li		AT, 2
		beq		t1, AT, 3f				# MEM CLOCK 148.5M
		nop
													# MEM CLOCK 198M	
1:	li		v0, 198
		b		9f
		nop
2:		li		v0, 162
		b		9f
		nop
3:		li		v0, 148
9:		jr      ra
        nop
		.set    at
		.end	sys_ic_get_dram_clock


#define CODEC_SUPPORT_LIST_SIZE 14
c3603_codec_support_list:
#####  A   B   C   D   E   F    G    H    I    J    K    L    M    N
.byte 0x0,0x1,0x3,0x7,0xf,0x10,0x11,0x13,0x17,0x1f,0x31,0x71,0x30,0x70
		.align  2
        .globl  c3603_get_bonding
        .ent    c3603_get_bonding
c3603_get_bonding:
		li		t0, 0xb8000100
		lui   t4, 0xa000
		la    t5, product_feature
		or    t5, t4
		li    t2, 0
2:
		li    t1, 0x04000008
		addu  t3, t1, t2 
		sw    t3, 0x4(t0)    #Trigger otp read
1:	                     
		lw    t3, 0x4(t0)
		srl   t3, 27
		andi  t3, 1
		bnez  t3, 1b         #Check read finished
		nop
		lw    t1, 0x8(t0)    #Read product feature
		addu  t3, t5, t2
		sw    t1, (t3)       #Save features 
		addiu t2, 4
		li    t3, 8
		bne   t2, t3, 2b
		nop
        lw    t1, 4(t5)
        li    t2, 0x1
        andi  t3, t1, 0x8
        beqz  t3, 4f
	nop
        la    t3, dram_sramble_is_enabled
		or    t3, t4		
		sw    t2, (t3)
4:        
        andi  t3, t1, 0x20
        beqz  t3, 5f
	nop
        la    t3, io_security_is_enabled
		or    t3, t4
		sw    t2, (t3)
5:
        andi  t3, t1, 0x40
        beqz  t3, 6f
	nop
        la    t3, split_is_enabled
		or    t3, t4
		sw    t2, (t3)
6:
        srl   t1, 8
        andi  t3, t1, 0x8
        beqz  t3, 7f
	nop
        la    t3, ejtag_is_enabled
		or    t3, t4
		sw    t2, (t3)
7:
        andi  t3, t1, 0x10
        beqz  t3, 8f
	nop
        la    t3, uart_is_enabled
		or    t3, t4
		sw    t2, (t3)		
8:		
		lw    t1, (t5)
		srl   t1, 20
		andi  t1, 0x1f      # bits[24..20]
		la    t3, product_name_id
		or    t3, t4
		sw    t1, (t3)

# read feature list
		lw    t1, (t5)
		srl   t1, 6
		andi  t1, 0x7f       #Get feature bits[6..12]
		li    t2, 0x7f
		beq   t1, t2, 2f
		nop		
		la    t2, c3603_codec_support_list
		li    t3, CODEC_SUPPORT_LIST_SIZE
		bgeu  t1, t3, 2f
		nop
		addu  t2, t1
		lbu   t1, (t2)       #Get codec support bits
2:		
		andi  t2, t1, 1
		la    t3, ac3_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, ddplus_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, ms10_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, ms11_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1	
		la    t3, mg_is_enabled
		or    t3, t4
		li    t2 , 0
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, XD_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, XDplus_is_enabled
		or    t3, t4
		sw    t2, (t3)
3:
		jr		ra
		nop
		.end    c3603_get_bonding
       
 #--------------------------------------------------------------------
 #
 #   int s3701_get_bonding()
 #
 #--------------------------------------------------------------------
#define C3701_CODEC_SUPPORT_LIST_SIZE 14
C3701_codec_support_list:
#####  A   B   C   D   E   F    G    H    I    J    K    L    M    N
.byte 0x0,0x1,0x3,0x7,0xf,0x10,0x11,0x13,0x17,0x1f,0x31,0x71,0x30,0x70
 
	    .align  2
        .globl  c3701_get_bonding
        .ent    c3701_get_bonding
c3701_get_bonding:
        lui   t4, 0xa000
		la    t5, product_feature
		or    t5, t4

        move  v1, ra
        li    a0, 0x8
        jal     _get_otp_asm_
        nop
        move    t1, v0
        move ra,v1
        sw  t1,0(t5)

        li    a0, 0xC
        move   v1, ra
        jal     _get_otp_asm_
        nop
        move    ra, v1
        move    t1, v0
        sw      t1,4(t5)

        li    a0, 0x210         # dword 0x84
        move   v1, ra
        jal     _get_otp_asm_
        nop
        move    ra, v1
        move    t1, v0
        sw      t1,8(t5)



		/* check otp 0xc content*/
        lw    t1, 4(t5)
        li    t2, 0x1
        andi  t3, t1, 0x8
        beqz  t3, 4f
	    nop
        la    t3, dram_sramble_is_enabled
		or    t3, t4		
		sw    t2, (t3)
4:        
        andi  t3, t1, 0x20
        beqz  t3, 5f
	    nop
        la    t3, io_security_is_enabled
		or    t3, t4
		sw    t2, (t3)
5:
        andi  t3, t1, 0x40
        beqz  t3, 6f
	    nop
        la    t3, split_is_enabled
		or    t3, t4
		sw    t2, (t3)
6:
        srl   t1, 8
        andi  t3, t1, 0x8
        beqz  t3, 7f
	    nop
        la    t3, ejtag_is_enabled
		or    t3, t4
		sw    t2, (t3)
7:
        andi  t3, t1, 0x10
        beqz  t3, 8f
	    nop
        la    t3, uart_is_enabled
		or    t3, t4
		sw    t2, (t3)

8:		srl   t1, 16
		andi  t3, t1,0x4
		beqz  t3,9f
		nop 
		la    t3, dsc_access_ce_is_dis
		or    t3, t4
		sw    t2,(t3)      		

9:                          # c3701_check
        lw    t1, 8(t5)
		srl   t1, 20
		andi  t1, 0x1f      # bits[24..20]
		la    t3, product_name_id
		or    t3, t4
		sw    t1, (t3)

        lw    t1, 8(t5)
		srl   t1, 6
		andi  t1, 0x7f       # Get feature bits[6...12]
		li    t2, 0x7f
		beq   t1, t2, 2f
		nop		
		la    t2, C3701_codec_support_list
		li    t3, C3701_CODEC_SUPPORT_LIST_SIZE
		bgeu  t1, t3, 2f
		nop
		addu  t2, t1
		lbu   t1, (t2)       #Get codec support bits
2:		
		andi  t2, t1, 1
		la    t3, ac3_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, ddplus_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, ms10_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, ms11_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1	
		la    t3, mg_is_enabled
		or    t3, t4
		li    t2 , 0
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, XD_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t1, 1
		andi  t2, t1, 1
		la    t3, XDplus_is_enabled
		or    t3, t4
		sw    t2, (t3)
        nop

        /*check otp hw 0x8 */	    	    
        lw    t1, (t5)				       # Get feature bits[0..15] from OTP
        andi  t1, 0xffff    
2:		
       	andi  t2, t1, 1                    # bits 0
		la    t3, mpge2_is_enabled          		
		or    t3, t4		
		seq   t2, t2,0
		sw    t2, (t3)
        
        srl   t1, 1                         # bits 1
		andi  t2, t1, 1
		la    t3, h264_is_enabled
		seq   t2, t2,0
		or    t3, t4
		sw    t2, (t3)
        

        srl   t1, 1                         # bits 2
		andi  t2, t1, 1
		la    t3, mpge4_is_enabled
		or    t3, t4		
		seq   t2, t2,0
		sw    t2, (t3)


        srl   t1, 1                         # bit 3
		andi  t2, t1, 1                     
		la    t3, rmvb_is_enabled
		or    t3, t4
		seq   t2, t2,0
		sw    t2, (t3)

		srl   t1, 1                         # bit 4
        andi  t2, t1, 1                     
		la    t3, vc1_is_enabled
		or    t3, t4	
		seq   t2, t2,0
		sw    t2, (t3)

		
		srl   t1, 1                         # bit 5        
		andi  t2, t1, 1
		la    t3, avs_is_enabled
		or    t3, t4		
		seq   t2, t2,0
		sw    t2, (t3)                      
                                         
		srl   t1, 1                          # bit 6  
		andi  t2, t1, 1
		la    t3, vp8_is_enabled
		or    t3, t4
		seq   t2, t2,0
		sw    t2, (t3)
		
		srl   t1, 6                         # bit 12
		andi  t2, t1, 1
		la    t3, hd_is_enabled
		or    t3, t4
		seq   t2, t2,0
		sw    t2, (t3)

        srl   t1, 1                         # bit 13 & 14 also should check with feature list
		andi  t2, t1, 1
		la    t3, mg_is_enabled
		or    t3, t4		
		seq   t2, t2,0
		lw    t0, (t3)		
	    and   t2, t0
	    li    t2 ,0
		sw    t2, (t3)

        srl   t1, 1                         # bit 14
		andi  t2, t1, 1
		la    t3, XD_is_enabled
		or    t3, t4		
		seq   t2, t2,0
		lw    t0, (t3)		
	    and   t2, t0
		sw    t2, (t3)		
		

		srl   t1, 1                         # bit 15
		andi  t2, t1, 1
		la    t3, flv_is_enabled
		or    t3, t4
		seq   t2, t2,0
		sw    t2, (t3)               		                                     
        
3:
		jr		ra
		nop
		.end    c3701_get_bonding			
  
 #--------------------------------------------------------------------
 #
 #   int sys_ic_dram_scramble_enabled()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_dram_scramble_enabled
        .ent    sys_ic_dram_scramble_enabled

sys_ic_dram_scramble_enabled:
		la		t0, dram_sramble_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_dram_scramble_enabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_io_security_enabled()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_io_security_enabled
        .ent    sys_ic_io_security_enabled

sys_ic_io_security_enabled:
		la		t0, io_security_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_io_security_enabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_split_enabled()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_split_enabled
        .ent    sys_ic_split_enabled

sys_ic_split_enabled:
		la		t0, split_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_split_enabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_uart_enabled()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_uart_enabled
        .ent    sys_ic_uart_enabled

sys_ic_uart_enabled:
		la		t0, uart_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_uart_enabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_ejtag_enabled()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_ejtag_enabled
        .ent    sys_ic_ejtag_enabled

sys_ic_ejtag_enabled:
		la		t0, ejtag_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_ejtag_enabled
  
 #--------------------------------------------------------------------
 #
 #   int sys_ic_get_hd_enabled_ex()	// 0: enable SD only, 1: enable SD/HD
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_hd_enabled_ex
        .ent    sys_ic_get_hd_enabled_ex

sys_ic_get_hd_enabled_ex:
		la		t0, hd_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_get_hd_enabled_ex

 #--------------------------------------------------------------------
 #
 #   int sys_ic_mg_is_enabled_ex()	// 0/1: microvision disable/enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_mg_is_enabled_ex
        .ent    sys_ic_mg_is_enabled_ex

sys_ic_mg_is_enabled_ex:
		la		t0, mg_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_mg_is_enabled_ex

 #--------------------------------------------------------------------
 #
 #   int sys_ic_ac3_is_enabled_ex()	// 0: AC3 disable, only set AC3 for spdif with bypass mode, 1: AC3 enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_ac3_is_enabled_ex
        .ent    sys_ic_ac3_is_enabled_ex

sys_ic_ac3_is_enabled_ex:
		la		t0, ac3_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_ac3_is_enabled_ex


 #--------------------------------------------------------------------
 #
 #   int sys_ic_ddplus_is_enabled_ex()	// 0: dd+ disable, 1: dd+ enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_ddplus_is_enabled_ex
        .ent    sys_ic_ddplus_is_enabled_ex

sys_ic_ddplus_is_enabled_ex:
		la		t0, ddplus_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_ddplus_is_enabled_ex

 #--------------------------------------------------------------------
 #
 #   int sys_ic_XD_is_enabled_ex()	 // 0: dd+ disable, 1: dd+ enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_XD_is_enabled_ex
        .ent    sys_ic_XD_is_enabled_ex

sys_ic_XD_is_enabled_ex:
		la		t0, XD_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_XD_is_enabled_ex

 #--------------------------------------------------------------------
 #
 #   int sys_ic_XDplus_is_enabled_ex()	// 0: XDplus disable, 1: XDplus enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_XDplus_is_enabled_ex
        .ent    sys_ic_XDplus_is_enabled_ex

sys_ic_XDplus_is_enabled_ex:
		la		t0, XDplus_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_XDplus_is_enabled_ex

 #--------------------------------------------------------------------
 #
 #   int sys_ic_aac_is_enabled_ex()	// 0: dd+ disable, 1: dd+ enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_aac_is_enabled_ex
        .ent    sys_ic_aac_is_enabled_ex

sys_ic_aac_is_enabled_ex:
		la		t0, aac_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_aac_is_enabled_ex

 #--------------------------------------------------------------------
 #
 #   int sys_ic_h264_is_enabled_ex()	// 0: h264 disable, 1: h264 enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_h264_is_enabled_ex
        .ent    sys_ic_h264_is_enabled_ex

sys_ic_h264_is_enabled_ex:
		la		t0, h264_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_h264_is_enabled_ex   

#--------------------------------------------------------------------
#
#   int sys_ic_mp4_is_enabled_ex()	// 0: mp4 disable, 1: mp4 enable
#
#--------------------------------------------------------------------
        .globl  sys_ic_mp4_is_enabled_ex
        .ent    sys_ic_mp4_is_enabled_ex

sys_ic_mp4_is_enabled_ex:
		la		t0, mp4_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_mp4_is_enabled_ex    

#--------------------------------------------------------------------
#
#   int sys_ic_dsc_access_ce_is_dis()	// 0: access enable, 1: access dissable
#
#--------------------------------------------------------------------
        .globl  sys_ic_dsc_access_ce_is_dis
        .ent    sys_ic_dsc_access_ce_is_dis

sys_ic_dsc_access_ce_is_dis:
		la		t0, dsc_access_ce_is_dis		
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_dsc_access_ce_is_dis


# ---------------------------------------------------------------------------
# UINT32 _get_otp_asm_(UINT32 addr)
# This function to read otp value for 3701 in asm file
# addr : otp offset , a0  
# @used		t0,t1,t2,t3,t4,t5,t6,t7,t9
# ---------------------------------------------------------------------------

#define OTP_BASE            0xb8042000
#define OTP_READ_ADDR       0x4
#define OTP_READ_TRIG       0xc
#define OTP_READ_STATUS     0x10
#define OTP_READ_DATA       0x18

#define OTP_READ_TRIG_BIT   0x100
#define OTP_READ_STATUS_BIT   0x100


 		.globl	_get_otp_asm_
		.ent	_get_otp_asm_
_get_otp_asm_ :

        la	t0, dummy_data              ## save temp reg
     	sw	ra,24(t0)
     	sw	s4,20(t0)
     	sw	s3,16(t0)
     	sw	s2,12(t0)
     	sw	s1,8(t0)
     	sw	s0,4(t0)
  

        li  s0, 0xb8000000
        lw  s1, 0(s0)
        srl s1, 16
        li  s2, 0x3701
        beq s1, s2, otp_37
        nop
        li  s2, 0x3503
        beq s1, s2, otp_35
        nop
        j   otp_36
        nop

otp_37:                             # get_otp of 3701
otp_35:				    # get_otp of 3503
        li     v0, 0
        li     s3, OTP_BASE             

        sw  a0, OTP_READ_ADDR(s3)
        
        lw  s0, OTP_READ_TRIG(s3)   # read trig
        li  s1, OTP_READ_TRIG_BIT
        or  s0, s1
        sw  s0, OTP_READ_TRIG(s3)

        li  s2, 0x100000
        li  s0, OTP_READ_STATUS_BIT     # check read status
1:      
        subu  s2, 0x1
        beqz  s2,  2f
        nop        
        lw  s1, OTP_READ_STATUS(s3)
        and s1,s0
        bnez  s1, 1b 
        nop
2:      
		lw    v0, OTP_READ_DATA(s3)
		j       out
		nop        

otp_36:                              # get_otp of 3603/3281		
        
        li	    s2, 0xb8000100
    	li      s3, 0x04000000
    	or      s3, a0
    	sw   	s3, 0x4(s2)    		# Trigger otp read
1:
    	lw    	s1, 0x4(s2)
    	srl   	s1, 27
    	andi  	s1, 1
    	bnez  	s1, 1b				# Check read finished
    	nop
        lw		v0, 0x8(s2) 
out:
        la	t0, dummy_data
    	lw	ra,24(t0)
    	lw	s4,20(t0)
    	lw	s3,16(t0)
    	lw	s2,12(t0)
    	lw	s1,8(t0)
    	lw	s0,4(t0)

    	jr	ra
    	nop

		jr		ra
		nop
		.end	_get_otp_asm_	
		
        .set    reorder



 #--------------------------------------------------------------------
 #
 #   int sys_ic_get_product_id_ex()	// get C3603 product name id
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_product_id_ex
        .ent    sys_ic_get_product_id_ex

sys_ic_get_product_id_ex:
		la		t0, product_name_id
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_get_product_id_ex

        
        .set    at
        .set    reorder

.data
dsc_access_ce_is_dis:.word 0
dram_sramble_is_enabled: .word	0
split_is_enabled: .word	0
io_security_is_enabled: .word 0
ejtag_is_enabled: .word	0
uart_is_enabled: .word	0
ac3_is_enabled: .word	0
ddplus_is_enabled: .word	0
ms10_is_enabled: .word 0
ms11_is_enabled: .word	0
mg_is_enabled: .word	0
XDplus_is_enabled: .word	0
XD_is_enabled: .word	0
hd_is_enabled: .word	0
aac_is_enabled: .word	0
half_mode_registor_value: .word	0
h264_is_enabled: .word	 0
mp4_is_enabled: .word	 0
mpge2_is_enabled:.word   0
mpge4_is_enabled:.word  0
rmvb_is_enabled:.word 0
vc1_is_enabled:.word 0
avs_is_enabled:.word 0
vp8_is_enabled:.word 0
flv_is_enabled:.word 0
product_name_id: .word	0
.globl  product_feature
product_feature: .word	0, 0
dummy_data:
.byte 0:48

