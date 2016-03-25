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

#include "sys_define.h"
#include "mips.h"
#include "machine.h"

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


 #--------------------------------------------------------------------
 #
 #   void sys_ic_enter_standby(UINT32 expect_time, UINT32 current_time)
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



#if 0
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


 #--------------------------------------------------------------------
 #
 #   void sys_watchdog_reboot()
 #
 #--------------------------------------------------------------------
        .globl  sys_watchdog_reboot
        .ent    sys_watchdog_reboot

sys_watchdog_reboot:

#ifdef SEE_CPU
		.word   0x7000003f              # SEE not reboot, for debug
#endif
#ifdef _DEBUG_PRINTF_
		.word   0x7000003f              # SDBBP opcode
#endif		
		li		s8, 0xdeadbead			# Set S8 to make it rebootup but standby
		move	s7, zero
		jal		sys_ic_reboot
		move	s5, zero

        .end    sys_watchdog_reboot


 #--------------------------------------------------------------------
 #
 #   void sys_ic_enter_standby(UINT32 expect_time, UINT32 current_time)
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
##M3101 Configuration  Parameter Send To BootLoader.All other projects just don't care.
		move	gp, a2
	 	
		# mapping 0xAFC00000 to Nor flash start address
		lbu   t9, 0x223(t0)
		andi  t9, 0xfe
		sb    t9, 0x223(t0)

		li		t2, 0x10
2:		bnez	t2,2b
		addiu	t2,-1

	 	li		t0, SYS_FLASH_BASE_ADDR
	 	jr		t0
		nop

        jr      ra
        nop
		.end	sys_ic_enter_standby


 #--------------------------------------------------------------------
 #
 #   void sys_ic_power_save(int on_off, UINT32 dev_mask)
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_power_save
        .ent    sys_ic_power_save
sys_ic_power_save:
		lui		t0, SYS_IC_NB_BASE_H
		lw		t1, SYS_IC_NB_LDCR(t0)
		beq		a0, zero, 1f
		nop
		or		t1, a1
		b		2f
		nop
1:		not		t2, a1
		and		t1, t2
2:		sw		t1, SYS_IC_NB_LDCR(t0)
#if 0
		.set mips3
		nop
		wait
		nop
		.set mips1
#endif
        jr      ra
        nop
		.end	sys_ic_power_save


 #--------------------------------------------------------------------
 #
 #   void sys_ic_exit_standby()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_exit_standby
        .ent    sys_ic_exit_standby
sys_ic_exit_standby:
		lui		t0, SYS_IC_NB_BASE_H
		move	t1, zero				# Start all device clock
		sw		t1, SYS_IC_NB_LDCR(t0)
        jr      ra
        nop

		.end	sys_ic_exit_standby


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
		li            AT,0x3811                   #0x3811
		beq        t1,AT,6f
		nop
		b		8f						# Other chip
		nop
1:		li		v0, ALI_M3327
		b		9f
		nop
2:		li		v0, ALI_M3327C
		b		9f
		nop
3:		li		v0, ALI_M3329E
		b		9f
		nop
4:		li		v0, ALI_S3602
		b		9f
		nop	
5:		li		v0, ALI_S3602F
		b		9f
		nop
6:           li           v0, ALI_S3811                     #ALI_S3811   
		b    9f
		nop
8:		li		v0, SYS_DEFINE_NULL
9:		jr      ra
        nop
		.set    at
		.end	sys_ic_get_chip_id

        .set    noat


 #--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_is_M3101()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_is_M3101
        .ent    sys_ic_is_M3101
sys_ic_is_M3101:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lh		t1, 2(t0)
		li		AT, 0x3101
		beq		t1, AT, 2f				# M3101 Serial: 0x3101, look as M3327C
		nop
1:		li		v0, 0
		b		3f
		nop
2:		li		v0, 1
		b		3f
		nop
3:		jr      ra
        nop
		.set    at
		.end	sys_ic_is_M3101

        .set    noat

 #--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_is_M3202()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_is_M3202
        .ent    sys_ic_is_M3202
sys_ic_is_M3202:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lh		t1, 2(t0)
		li		AT, 0x3202
		beq		t1, AT, 2f				# M3202 Serial: 0x3202, look as M3327C
		nop
1:		li		v0, 0
		b		3f
		nop
2:		li		v0, 1
		b		3f
		nop
3:		jr      ra
        nop
		.set    at
		.end	sys_ic_is_M3202

        .set    noat
#--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_is_M3329D()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_is_M3329D
        .ent    sys_ic_is_M3329D
sys_ic_is_M3329D:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lh		t1, 2(t0)
		li		AT, 0x3329
		bne		t1, AT, 2f				# M3202 Serial: 0x3202, look as M3327C
		nop
		lbu		t1, 0(t0)
		sltiu		t1, t1, 0x5
		bne		zero, t1, 2f
		nop
		lw		t1, 0(t0)
		andi		t1, t1, 0x8000
		beq		zero, t1, 2f
		nop
1:		li		v0, 1
		b		3f
		nop
2:		li		v0, 0
		b		3f
		nop
3:		jr      ra
        nop
		.set    at
		.end	sys_ic_is_M3329D


 #--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_M3101_rev_id()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_M3101_rev_id
        .ent    sys_ic_M3101_rev_id
sys_ic_M3101_rev_id:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lb		v0, (t0)
		jr      ra
        nop
		.set    at
		.end	sys_ic_M3101_rev_id

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
		li		t2, 0x3811                          #if 0x3811
		beq		t1, t2, check_s3811_id
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
check_s3811_id:
		lbu        t1, 0(t0)
		andi      t1,0xff                            
		beq       t1,zero,1f                    #0x3811_0X00                    
		nop
		b          2f                                
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
#if ((SYS_CHIP_MODULE == ALI_S3602) && (SYS_CPU_MODULE != CPU_M6303))
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
		
##add S3811
		li AT,ALI_S3811
		beq AT,v0, f38
		nop
##		
		lui		t0, SYS_IC_NB_BASE_H
		lw		t1, 0x70(t0)
		srl		t1, t1, 7

		andi	t1, 0x3
		beq		t1, 0, 1f				# CPU CLOCK 396M
		li		AT, 1
		beq		t1, AT, 2f				# CPU CLOCK 486M
		li		AT, 2
		beq		t1, AT, 3f				# CPU CLOCK 324M
		nop
													# CPU CLOCK 396M	
1:	li		v0, 396
		b		9f
		nop
2:		li		v0, 486
		b		9f
		nop
3:		li		v0, 324
		b 9f
		nop
f36:	
		lui		t0, SYS_IC_NB_BASE_H
	#	lw		t1, 0x74(t0)
		lw          t1, 0x70(t0)
		srl		t1, t1, 7

		andi	t1, 0x3
		beq		t1, 0, 1f				# CPU CLOCK 396M
		li		AT, 3
		beq		t1, AT, 2f				# CPU CLOCK 486M
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
 ##add S3811
f38:
		lui		t0, SYS_IC_NB_BASE_H
		lw		t1, 0x70(t0)
		srl		t1, t1, 7
		andi		t1, 0x3
		beq		t1, 0, 1f				# CPU CLOCK 675M
		nop
		li		AT, 3
		beq		t1, AT, 2f				# CPU CLOCK 297M
		nop
		li            AT,2
		beq       	t1, AT,3f                     #CPU CLOCK 396M
		nop
		b	4f										# CPU CLOCK 594M	
		nop
1:	li	v0, 675
		b		9f
		nop
2:		
		li		v0, 297
		b 	9f
		nop
3:
		li           v0,396
		b  9f
		nop
4:		
		li		v0, 594
9:		
		move ra,t3
		jr      ra
        nop
 ##
		.set    at
		.end	sys_ic_get_cpu_clock
#else
        .globl  sys_ic_get_cpu_clock
        .ent    sys_ic_get_cpu_clock
sys_ic_get_cpu_clock:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lw		t1, 0x70(t0)
		lh		t2, 2(t0)

		li		t3, 0x3329
		bne		t2, t3, check_clk_3202			# If isn't M3329e, goto clock check for 3202
		nop
		lbu		t2, 0(t0)
		sltiu		t2, t2, 0x5
		beq		zero, t2, check_clk_3329e5
		nop
		andi	t1, 0x3
		beq		t1, 0, 1f				# CPU CLOCK 270M
		li		AT, 1
		beq		t1, AT, 2f				# CPU CLOCK 240M
		li		AT, 2
		beq		t1, AT, 3f				# CPU CLOCK 216M
		nop
		b		4f				# CPU CLOCK 180M
		nop
check_clk_3329e5:
		srl		t1, t1, 5
		andi	t1, 0x3
		beq		t1, 0, 1f				# CPU CLOCK 270M
		li		AT, 1
		beq		t1, AT, 2f				# CPU CLOCK 240M
		li		AT, 2
		beq		t1, AT, 3f				# CPU CLOCK 216M
		nop
		b		4f				# CPU CLOCK 180M
		nop		
check_clk_3202:
		andi	t1, 0x7
		li	t3, 0x3202
		bne	t2, t3, begin_chk #check 3101 and m3327c
		nop
		andi	t1, 0x1
		beqz t1, 1f
		nop
		b	3f
		nop
begin_chk:	
		li  t3, 0x3101
		bne t2,t3, check_other #check is  3105 or not
		nop
check_3105:
		andi    t1, 0x3
		beqz    t1, 7f                  # CPU CLOCK 307M
		li      AT, 1
		beq     t1, AT, 1f              # CPU CLOCK 270M
		li      AT, 2
		beq     t1, AT, 6f              # CPU CLOCK 230M
		nop
		b       3f                      # CPU CLOCK 216M
		nop
check_other:		
		beq		t1, 0, 1f				# CPU CLOCK 270M
		li		AT, 1
		beq		t1, AT, 2f				# CPU CLOCK 240M
		li		AT, 2
		beq		t1, AT, 2f				# CPU CLOCK 240M
		li		AT, 3
		beq		t1, AT, 3f				# CPU CLOCK 216M
		li		AT, 4
		beq		t1, AT, 4f				# CPU CLOCK 180M
		li		AT, 5
		beq		t1, AT, 5f				# CPU CLOCK 154M
		li		AT, 6
		beq		t1, AT, 5f				# CPU CLOCK 154M
		li		AT, 7
		beq		t1, AT, 4f				# CPU CLOCK 180M
		nop
	
1:		li		v0, 270
		b		9f
		nop
2:		li		v0, 240
		b		9f
		nop
3:		li		v0, 216
		b		9f
		nop
4:		li		v0, 180
		b		9f
		nop
5:		li		v0, 154
		b       9f
		nop
6:		li      v0, 230
		b       9f
		nop		
7:		li		v0, 307
		b       9f
		nop
9:		jr      ra
        nop
		.set    at
		.end	sys_ic_get_cpu_clock
#endif

 #--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_get_dram_clock()
 #
 #   Note:
 #	M3327 uses three bits MEM_CLK[2: 0] to config MEM_CLK,
 #	M3101 ONLY uses one bit MEM_CLK[0] to config MEM_CLK.
 #
 #--------------------------------------------------------------------
#if (SYS_CHIP_MODULE == ALI_S3602)
        .globl  sys_ic_get_dram_clock
        .ent    sys_ic_get_dram_clock
sys_ic_get_dram_clock:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lh           t1, 2(t0)
		li            AT,0x3811                   #0x3811
		beq        t1,AT,10f
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
10:
		lw		t1, 0x70(t0)
		srl		t1, t1, 4
		andi		t1, 0x7
		beq		t1, 0, 1f				# MEM CLOCK 19.8M
		li		AT, 1
		beq		t1, AT, 2f				# MEM CLOCK 264M
		li		AT, 2
		beq		t1, AT, 3f				# MEM CLOCK 316.8M
		li		AT, 3
		beq		t1, AT, 4f				# MEM CLOCK 369.6M
		li		AT, 4
		beq		t1, AT, 5f				# MEM CLOCK 514.8M
		li		AT, 5
		beq		t1, AT, 6f				# MEM CLOCK 396M
		li		AT, 6
		beq		t1, AT, 7f				# MEM CLOCK 475.2M
		li		AT, 7
		beq		t1, AT, 8f				# MEM CLOCK 554.4M

1:		li		v0, 20
		b		9f
		nop
2:		li		v0, 264
		b		9f
		nop
3:		li		v0, 317
		b           9f
		nop
4:		li		v0, 370
		b		9f
		nop
5:		li		v0, 515
		b		9f
		nop
6:		li		v0, 396
		b           9f
		nop
7:		li		v0, 475
		b		9f
		nop
8:		li		v0, 554

9:		jr      ra
        nop
		.set    at
		.end	sys_ic_get_dram_clock
#else
        .globl  sys_ic_get_dram_clock
        .ent    sys_ic_get_dram_clock
sys_ic_get_dram_clock:
		.set    noat
		lui		t0, SYS_IC_NB_BASE_H
		lw		t1, 0x70(t0)
		lh		t2, 2(t0)
		li		t3, 0x3329
		beq		t2, t3, check_mclk_29e
		nop
		li		t3, 0x3202
		beq		t2, t3, check_mclk_3202
		nop
		b		5f					# DRAM CLOCK 0M
		nop
check_mclk_3202:
		srl		t1, t1, 4
		andi		t1, 0x3
		beq		t1, 0, 1f				# DRAM CLOCK 135M
		li		AT, 1
		beq		t1, AT, 2f				# DRAM CLOCK 120M
		li		AT, 2
		beq		t1, AT, 3f				# DRAM CLOCK 154M
		nop
		b		4f				# DRAM CLOCK 166M
		nop
check_mclk_29e:
		lbu		t2, 0(t0)
		sltiu		t2, t2, 0x5
		beq		zero, t2, check_mclk_29e5
		nop
		srl		t1, t1, 2
		andi		t1, 0x3
		beq		t1, 0, 1f				# DRAM CLOCK 135M
		li		AT, 1
		beq		t1, AT, 2f				# DRAM CLOCK 120M
		li		AT, 2
		beq		t1, AT, 4f				# DRAM CLOCK 166M
		nop
		b		3f					# DRAM CLOCK 154M
		nop
check_mclk_29e5:
		srl		t1, t1, 7
		andi		t1, 0x3
		beq		t1, 0, 3f				# DRAM CLOCK 154M
		li		AT, 1
		beq		t1, AT, 1f				# DRAM CLOCK 135M
		li		AT, 2
		beq		t1, AT, 6f				# DRAM CLOCK 90M
		nop
		b		7f					# DRAM CLOCK 45M
		nop		
1:		li		v0, 135
		b		9f
		nop
2:		li		v0, 120
		b		9f
		nop
3:		li		v0, 154
		b		9f
		nop
4:		li		v0, 166
		b		9f
		nop
5:		li		v0, 0
		b		9f
		nop
6:		li		v0, 90
		b		9f
		nop
7:		li		v0, 45
9:		jr      ra
        nop
		.set    at
		.end	sys_ic_get_dram_clock
#endif

 #--------------------------------------------------------------------
 #
 #   UINT32 sys_ic_get_chip_mode()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_chip_mode
        .ent    sys_ic_get_chip_mode

sys_ic_get_chip_mode:
        	lui		t0, SYS_IC_NB_BASE_H
		lw		t1, 0x70(t0)
		lh		t2, 2(t0)
		li		t3, 0x3329
		beq		t2, t3, check_mode_29e
		nop
		li		t3, 0x3202
		beq		t2, t3, check_mode_3202
		nop
		li		t1, 0
		b 		check_mode
		nop
check_mode_29e:		
		lbu		t2, 0(t0)
		sltiu		t2, t2, 0x5
		beq		zero, t2, check_mode_29e5
		nop
		srl		t1, t1, 4
		andi		t1, 0x7
		b		check_mode
		nop
check_mode_29e5:
		andi		t1, 0x7
		b		check_mode
		nop
check_mode_3202:
		srl		t1, t1,  8
		andi		t1, 0x7
		b		check_mode
		nop
check_mode:
		move	v0, t1
		nop
		jr      ra
		nop

        .end    sys_ic_get_chip_mode


#if(SYS_CHIP_MODULE == ALI_S3602)
 #--------------------------------------------------------------------
 #
 #   void sys_ic_get_half_mode_strap_pin()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_half_mode_strap_pin
        .ent    sys_ic_get_half_mode_strap_pin
        
sys_ic_get_half_mode_strap_pin:
		li		t3,0xb8000070
		li		t6, 0xa0000000
		lw		t4,(t3)
		srl	t0,t4,0x11
		la t2,half_mode_registor_value
		and t1,t0,0x01
		or t2, t6
		sw t1,(t2)
        	jr		ra
		nop
        .end    sys_ic_get_half_mode_strap_pin


 #--------------------------------------------------------------------
 #
 #   void sys_ic_set_half_mode_strap_pin()
 #
 #--------------------------------------------------------------------
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
 #   void sys_ic_get_bonding()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_bonding
        .ent    sys_ic_get_bonding

sys_ic_get_bonding:

		move t9, ra
		jal sys_ic_is_M3101
		nop
		beq v0, zero, 1f
		nop
		jal M3105_get_bonding
		nop
		move ra, t9
		b 4f
		nop
1:		
		move ra, t9

		li		t4, 0xa0000000
		li		t0, 0xafc000a0
		lbu     t1, (t0)
		andi	t2, t1, 0x04
		li      t0, 0x04
		beq     t0, t2, 2f      
		nop
		andi	t2, t1, 0x02
		li		t0, 0x02
		beq     t0, t2, 1f      
		nop

		# for M3601c old IC, without bonding option
		li		t0, 1
		la		t1, hd_is_enabled
		or		t1, t4
		sw		t0, (t1)
		la		t1, mv_is_enabled
		or		t1, t4
		sw		t0, (t1)
		la		t1, ac3_is_enabled
		or		t1, t4
		sw		t0, (t1)
		# ddplus is always disabled
		li		t0, 0
		la		t1, ddplus_is_enabled
		or		t1, t4
		sw		t0, (t1)
		jr		ra
		nop

		# for M3601c/M3381c	with bonding option
1:
		# boot.cfg (0ffset 0x20, bit 0) (0 - hd, 1 - sd)
		# XGPIO69 (0 - AC3 enable, 1 - AC3 disable)
		# set GPIO as input mode before reading
		li		t0, 0xb80000ec
		lbu		t1, (t0)
		andi	t1, 0xdf
		sb		t1, (t0)

		li		t0, 0xb80000e4
		lbu		t1, 0x00(t0)
		andi	t2, t1, 0x20
		srl		t2, 5
		xori	t2, 1
		la		t3, ac3_is_enabled
		or		t3, t4
		sw		t2, (t3)

		li		t0, 0xafc000a0
		lbu     t1, (t0)
		andi	t2, t1, 0x01
		xori	t2, 1
		la		t3, hd_is_enabled
		or		t3, t4
		sw		t2, (t3)

		# MV is always enabled
		li		t2, 1
		la		t3, mv_is_enabled
		or		t3, t4
		sw		t2, (t3)

		# ddplus is always disabled
		li		t2, 0
		la		t3, ddplus_is_enabled
		or		t3, t4
		sw		t2, (t3)

		jr		ra
		nop

		# for M3602/M3381
2:
		# set hd_is_enabled according to bonding option
		lui		t0, SYS_IC_NB_BASE_H
		lbu		t1, 0x00(t0)
		andi	t2, t1, 0x80
		srl		t2, 7
		la		t3, hd_is_enabled
		or		t3, t4
		sw		t2, (t3)
		nop

		# get mv bonding option
		andi	t2, t1, 0x40
		srl		t2, 6
		move	t5, t2

		# always set XGPIO71 to 0: (0 - enabled, 1 - disabled)
		# 1. for old IC, AC3 is always enabled
		# 2. for New IC, it can't be set successfully, so it depends on bonding
		li		t0, 0xb80000ec
		lw		t1, (t0)
		ori		t1, 0x80
		sw		t1, (t0)
		li		t0, 0xb80000e8
		lw		t1, (t0)
		li		t2, 0xffffff7f
		and		t1, t2
		sw		t1, (t0)

		li		t0, 0xb80000e4
		lbu		t1, 0x00(t0)
		andi	t1, 0x80
		srl		t1, 7
		move	t6, t1
		xori	t2, t1, 1
		la		t3, ac3_is_enabled
		or		t3, t4
		sw		t2, (t3)
		nop

		# set mv_is_enabled.
		# 0 (mv_bit = 0, (mv_bit = 1) & (ac3_bit = 1))
		# 1 (mv_bit = 1 & ac3_bit = 0)
		move	t2, t5
		beqz	t2, 3f
		nop
		move	t2, t6
		xori	t2, 1
3:
		la		t3, mv_is_enabled
		or		t3, t4
		sw		t2, (t3)
		nop

		# set ddplus_is_enabled:
		# 1: (mv_bit = 1 && ac3_bit = 1), otherwise 0
		and		t2, t5, t6
		la		t3, ddplus_is_enabled
		or		t3, t4
		sw		t2, (t3)
4:
		jr		ra
		nop

        .end    sys_ic_get_bonding

 #--------------------------------------------------------------------
 #
 #   void M3105_get_bonding()
 #
 #--------------------------------------------------------------------
        .globl  M3105_get_bonding
        .ent    M3105_get_bonding

M3105_get_bonding:
		lui		t0, SYS_IC_NB_BASE_H
		lh		t1, 2(t0)
		li		AT, 0x3101
		bne		t1, AT, 2f				# M3101 Serial: 0x3101
		nop
		
		lh		t1, (t0)
		nop
		and    t1,t1,0x0000FFFF
		srl    t1, t1, 8
1:	                     
		la    t3, ac3_is_enabled
		li    t4, 0x40 #bit14
		and   t2, t4, t1
		srl   t2, t2, 6
		sw    t2, (t3)
		
		la    t3, h264_is_enabled
		li    t4, 0x8 #bit11
		and   t2, t4, t1
		srl   t2, t2, 3
		sw    t2, (t3)
		
		la    t3, XD_is_enabled
		li    t4, 0x1 #bit8
		and   t2, t4, t1
		sw    t2, (t3)
		
		la    t3, mp4_is_enabled
		li    t4, 0x4 #bit10
		and   t2, t4, t1
		srl   t2, t2, 2
		sw    t2, (t3)
2:		
		jr		ra
		nop
        .end    M3105_get_bonding
        

#--------------------------------------------------------------------
 #
 #   void s3602f_get_bonding()
 #
 #--------------------------------------------------------------------
        .globl  s3602f_get_bonding
        .ent    s3602f_get_bonding

s3602f_get_bonding:
		li		t0, 0xb8000100
		li    t1, 0x04000008
		lui   t4, 0xa000
		sw    t1, 0x4(t0)    #Trigger otp read
1:	                     #Check read finished
    lw    t1, 0x4(t0)
		srl   t1, 27
		andi  t1, 1
		bnez  t1, 1b
		nop
		lw    t1, 0x8(t0)
		srl   t2, t1, 16
		andi  t2, 1
		la    t3, mv_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t2, t1, 25
		andi  t2, 1
		la    t3, ac3_is_enabled
		or    t3, t4
		sw    t2, (t3)
    srl   t2, t1, 26
		andi  t2, 1
		la    t3, ddplus_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t2, t1, 2
		andi  t2, 1
		la    t3, XD_is_enabled
		or    t3, t4
		sw    t2, (t3)
		la    t3, XDplus_is_enabled
		or    t3, t4
		sw    t2, (t3)
		srl   t2, t1, 24
		andi  t2, 1
		la    t3, aac_is_enabled
		or    t3, t4
		sw    t2, (t3)

		jr		ra
		nop
        .end    s3602f_get_bonding

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
		la    t3, mv_is_enabled
		or    t3, t4
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
 #   void s3811_get_bonding()
 #
 #--------------------------------------------------------------------
        .globl  s3811_get_bonding
        .ent    s3811_get_bonding

s3811_get_bonding:
#   li		t0, 0xb8000100
#   li    t1, 0x04000008
#   lui   t4, 0xa000
#   sw    t1, 0x4(t0)    #Trigger otp read
		lui 	t4,	0xa000
		li     t2,   1
		la    t3, mv_is_enabled
		or    t3, t4
		sw    t2, (t3)
		la    t3, ac3_is_enabled
		or    t3, t4
		sw    t2, (t3)
		la    t3, ddplus_is_enabled
		or    t3, t4
		sw    t2, (t3)
		la    t3, XD_is_enabled
		or    t3, t4
		sw    t2, (t3)
		la    t3, XDplus_is_enabled
		or    t3, t4
		sw    t2, (t3)
	      li      t2,    1
	      la     t3,    aac_is_enabled
	      or     t3,   t4
		sw    t2,  (t3)

	      la     t3,  ejtag_is_enabled  
	      or     t3,   t4
		sw    t2,  (t3)
		 la     t3, uart_is_enabled   
	      or     t3,   t4
		sw    t2,  (t3)	
		la     t3, ms10_is_enabled   
	      or     t3,   t4
		sw    t2,  (t3)
	      la     t3,  ms11_is_enabled  
	      or     t3,   t4
		sw    t2,  (t3)

	      la     t3,  hd_is_enabled  
	      or     t3,   t4
		sw    t2,  (t3)

	     la     t3,  h264_is_enabled  
	      or     t3,   t4
		sw    t2,  (t3)

	     la     t3, mp4_is_enabled  
	      or     t3,   t4
		sw    t2,  (t3)
	      jr		ra
		nop
        .end    s3811_get_bonding
        
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
 #   int sys_ic_uart_disabled()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_uart_disabled
        .ent    sys_ic_uart_disabled

sys_ic_uart_disabled:
		la		t0, uart_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_uart_disabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_ejtag_enabled()
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_ejtag_disabled
        .ent    sys_ic_ejtag_disabled

sys_ic_ejtag_disabled:
		la		t0, ejtag_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_ejtag_disabled


 #--------------------------------------------------------------------
 #
 #   int sys_ic_get_hd_enabled()	// 0: enable SD only, 1: enable SD/HD
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_hd_enabled
        .ent    sys_ic_get_hd_enabled

sys_ic_get_hd_enabled:
		la		t0, hd_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_get_hd_enabled


 #--------------------------------------------------------------------
 #
 #   int sys_ic_mv_is_enabled()	// 0/1: microvision disable/enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_mv_is_enabled
        .ent    sys_ic_mv_is_enabled

sys_ic_mv_is_enabled:
		la		t0, mv_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_mv_is_enabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_ac3_is_enabled()	// 0: AC3 disable, only set AC3 for spdif with bypass mode, 1: AC3 enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_ac3_is_enabled
        .ent    sys_ic_ac3_is_enabled

sys_ic_ac3_is_enabled:
		la		t0, ac3_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_ac3_is_enabled


 #--------------------------------------------------------------------
 #
 #   int sys_ic_ddplus_is_enabled()	// 0: dd+ disable, 1: dd+ enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_ddplus_is_enabled
        .ent    sys_ic_ddplus_is_enabled

sys_ic_ddplus_is_enabled:
		la		t0, ddplus_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_ddplus_is_enabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_XD_is_enabled()	// 0: dd+ disable, 1: dd+ enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_XD_is_enabled
        .ent    sys_ic_XD_is_enabled

sys_ic_XD_is_enabled:
		la		t0, XD_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_XD_is_enabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_XDplus_is_enabled()	// 0: XDplus disable, 1: XDplus enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_XDplus_is_enabled
        .ent    sys_ic_XDplus_is_enabled

sys_ic_XDplus_is_enabled:
		la		t0, XDplus_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_XDplus_is_enabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_aac_is_enabled()	// 0: dd+ disable, 1: dd+ enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_aac_is_enabled
        .ent    sys_ic_aac_is_enabled

sys_ic_aac_is_enabled:
		la		t0, aac_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_aac_is_enabled

 #--------------------------------------------------------------------
 #
 #   int sys_ic_h264_is_enabled()	// 0: h264 disable, 1: h264 enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_h264_is_enabled
        .ent    sys_ic_h264_is_enabled

sys_ic_h264_is_enabled:
		la		t0, h264_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_h264_is_enabled    


#--------------------------------------------------------------------
 #
 #   int sys_ic_mp4_is_enabled()	// 0: mp4 disable, 1: mp4 enable
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_mp4_is_enabled
        .ent    sys_ic_mp4_is_enabled

sys_ic_mp4_is_enabled:
		la		t0, mp4_is_enabled
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_mp4_is_enabled    

 #--------------------------------------------------------------------
 #
 #   int sys_ic_get_product_id()	// get C3603 product name id
 #
 #--------------------------------------------------------------------
        .globl  sys_ic_get_product_id
        .ent    sys_ic_get_product_id

sys_ic_get_product_id:
		la		t0, product_name_id
		lw		v0, (t0)
		jr		ra
		nop
        .end    sys_ic_get_product_id

        
        .set    at
        .set    reorder


.data
dram_sramble_is_enabled: .word	0
split_is_enabled: .word	0
io_security_is_enabled: .word 0
ejtag_is_enabled: .word	0
uart_is_enabled: .word	0
ac3_is_enabled: .word	0
ddplus_is_enabled: .word	0
ms10_is_enabled: .word 0
ms11_is_enabled: .word	0
mv_is_enabled: .word	0
XDplus_is_enabled: .word	0
XD_is_enabled: .word	0
hd_is_enabled: .word	0
aac_is_enabled: .word	0
half_mode_registor_value: .word	0
h264_is_enabled: .word	 0
mp4_is_enabled: .word	 0
product_name_id: .word	0
.globl  product_feature
product_feature: .word	0, 0
#endif

#endif
