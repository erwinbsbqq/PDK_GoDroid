#--------------------------------------------------------------------
#
#  Copyright (C) 2005 Acer Laboratories Inc.  All Rights Reserved.
#
#  File:   reboot.S
#
#  Contents:    About reboot load and exchange field functions
#
#  History:
#      Date          By         Reason
#   ===========    =======   ========================
#   9/1/2005      Justin Wu     Create
#--------------------------------------------------------------------

#include "../arch/mips/include/asm/regdef.h"
# #include "../../platform/inc/api/libc/string.h"

#define SYS_FLASH_BASE_ADDR   0xafc00000
#define UNPRO_ADDR_UP            SYS_FLASH_BASE_ADDR
#define UNPRO_ADDR_LOW        (SYS_FLASH_BASE_ADDR + 0x400000)

#define SUCCESS 0
#define ERR_FAILUE -9

#define PUSH(x)			\
		subu		sp,	4;	\
		sw		x,	(sp); \
		nop

#define POP(x)			\
		lw	x,	(sp);	\
		addu	sp,	4; \
		nop

        .set    noreorder
#
# void sys_reboot_load_entry(UINT32 chunk_id, UINT32 chunk_mask)
#
		.globl	sys_reboot_load_entry
        .ent    sys_reboot_load_entry
sys_reboot_load_entry:
		li		s8, 0xdeadbeeb			# Multi load func ID
		move	s7, a0					# Save chunk ID
1:		jal		sys_ic_reboot			# Reboot
		move	s5, a1					# Save chunk ID mask
		b		1b
		nop
		jr      ra
        nop
		.end	sys_reboot_load_entry


#
# INT32 sys_reboot_get_timer(UINT32 *expect, UINT32 *current)
#
		.globl	sys_reboot_get_timer
        .ent    sys_reboot_get_timer
sys_reboot_get_timer:
		la		t1, 0x80000220			# Check vailiable flag in REF#
		lw		t2, 0(t1) 
		li		t0, 0xdead1bee
		li		v0, SUCCESS
		beq		t0, t2, 1f
		nop
		li		v0, ERR_FAILUE
		b		2f
		nop
1:		lw		t2, 4(t1)				# Get the expect & current time
		lw		t3, 8(t1)
		sw		t2, 0(a0)
		sw		t3, 0(a1)		
2:		jr      ra
        nop
		.end	sys_reboot_get_timer

#
# void sys_reboot_set_exchange_data(void *buffer, int len)
#
		.globl	sys_reboot_set_exchange_data
        .ent    sys_reboot_set_exchange_data
sys_reboot_set_exchange_data:
		PUSH(ra)
		move	a2, a1					# Set memcpy param len
		move	a1, a0					# Set memcpy param src
		la		a0, 0x80000120			# Set memcpy param dsc
		jal		memcpy
		nop
		POP(ra)
		jr      ra
        nop
		.end	sys_reboot_set_exchange_data


#
# void sys_reboot_get_exchange_data(void *buffer, int len)
#
		.globl	sys_reboot_get_exchange_data
        .ent    sys_reboot_get_exchange_data
sys_reboot_get_exchange_data:
		PUSH(ra)
		move	a2, a1					# Set memcpy param len
		la		a1, 0x80000120			# Set memcpy param src
		jal		memcpy				# Keep memcpy param dsc
		nop
		POP(ra)
		jr      ra
        nop
		.end	sys_reboot_get_exchange_data


#
# void sys_reboot_clear_exchange_data()
#
		.globl	sys_reboot_clear_exchange_data
        .ent    sys_reboot_clear_exchange_data
sys_reboot_clear_exchange_data:
		PUSH(ra)
		li		a0, 0x80000120			# Set memset param dsc
		li		a2, 96					# Set memset param len
		jal		memset
		move	a1, zero				# Set memset param value
		POP(ra)
		jr      ra
        nop
		.end	sys_reboot_clear_exchange_data

        .set    reorder



        .set    noreorder
        .set    noat
#ifndef SEE_CPU
        .globl  sflash_soft_protect_init
        .ent    sflash_soft_protect_init
sflash_soft_protect_init:
	or	t7, ra,ra
	jal	powerdown_test_init
	nop
	or	ra,t7,t7
	
	lw	t5, unpro_addr_low
	lw	t6, unpro_addr_up
	mtc0	t5, $18
	mtc0	t6, $10
	jr	ra
	nop
        .end    sflash_soft_protect_init


        .globl  sflash_write_soft_protect
        .ent    sflash_write_soft_protect

sflash_write_soft_protect:	    
	or	t7, ra,ra
	jal	powerdown_test
	nop
	or	ra,t7,t7

	lui     AT, SYS_FLASH_BASE_ADDR >> 16
        or      a0, AT
    	mfc0	t5, $18
    	mfc0	t6, $10
    	sltu	AT, a0, t5
        beqz    AT, 1f
        nop
        #sdbbp
        .word	0x7000003f
	nop
1:
        sltu	AT, a0, t6
        bnez    AT, 1f
        nop
        #sdbbp
        .word	0x7000003f
	nop
1:
start_copy:
	lui	t0,0xb800	
	sb	a3,0x98(t0)
copy_next_byte:
        beqz    a2, copy_ret_success
        addi    a2, -1
	lbu     t3, 0(a1)
        sb      t3, 0(a0)
        
	addiu   a1, 1
        b       copy_next_byte
        addiu   a0, 1
copy_ret_success:  
        jr      ra
        nop        
        .end    sflash_write_soft_protect


        .globl  pflash_soft_protect_init
        .ent    pflash_soft_protect_init
pflash_soft_protect_init:
	or	t7, ra,ra
	jal	powerdown_test_init
	nop
	or	ra,t7,t7

	lw	t5, unpro_addr_low
	lw	t6, unpro_addr_up
	mtc0	t5, $18
	mtc0	t6, $10
	jr	ra
	nop
        .end    pflash_soft_protect_init


        .globl  pflash_write_soft_protect
        .ent    pflash_write_soft_protect

pflash_write_soft_protect:
	lui     AT, SYS_FLASH_BASE_ADDR >> 16
        or      a0, AT
    	mfc0	t5, $18
    	mfc0	t6, $10
    	sltu	AT, a0, t5
        beqz    AT, 1f
        nop
        #sdbbp
        .word	0x7000003f
	nop
1:
        sltu	AT, a0, t6
        bnez    AT, 1f
        nop
        #sdbbp
        .word	0x7000003f
	nop
1:
        sb      a1, 0(a0)
        jr      ra
        nop        
        .end    pflash_write_soft_protect


##Do sdram check to assure that power supply is stable.
	.globl  powerdown_test_init
	.ent    powerdown_test_init
powerdown_test_init:
 	li	t0,0xa0005a5c
	la	t1,_sdram_check_addr_data_table
	li	t4,0x2030
	li	t5,0xf
2:	lw	t2,(t0)
	la	t3,rl_array
	addu	t3,t5
	lbu	t3,(t3)
	andi	t3,0xf
	sll	t6,t3,3
	sll	t3,2
	addu	t3,t6
	addu	t3,t1
	sw	t0,(t3)
	sw	t2,4(t3)
	not	t2
	sw	t2,8(t3)
	add	t0,t4
	bnez	t5,2b
	addiu	t5,-1
	jr	ra
	nop
	.end	powerdown_test_init
rl_array:
	.byte	0x0,0x8,0x4,0xc,0x2,0xa,0x6,0xe,0x1,0x9,0x5,0xd,0x3,0xb,0x7,0xf

        .globl  powerdown_test
        .ent    powerdown_test

powerdown_test:
	la	t1, _sdram_check_addr_data_table
	li	t5,0x10
2:	lw	t2,(t1)
	lw	t0,4(t1)
	lw	t3,0(t2)
	bne	t3,t0,flash_err_reboot
	addiu	t5,-1
	lw	t3,8(t1)
	not	t3
	bne	t3,t0,flash_err_reboot
	nop
	nop
	addiu	t1,12
3:	bnez	t5,2b
	nop
	jr	ra
	nop
	.end powerdown_test



flash_err_reboot:
	nop
	.word	0x42000020
	b	flash_err_reboot
	nop
_sdram_check_addr_data_table:
	.space	0xc0
#endif

	.set    at
        .set    reorder

.data
unpro_addr_low: .word	UNPRO_ADDR_LOW
unpro_addr_up: .word UNPRO_ADDR_UP
