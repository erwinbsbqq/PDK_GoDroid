/*
 *  Cache-handling routined for MIPS CPUs
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>

#ifndef CONFIG_SYS_MIPS_CACHE_MODE
#define CONFIG_SYS_MIPS_CACHE_MODE CONF_CM_CACHABLE_NONCOHERENT
#endif

#define RA		t8

/*
 * 16kB is the maximum size of instruction and data caches on MIPS 4K,
 * 64kB is on 4KE, 24K, 5K, etc. Set bigger size for convenience.
 *
 * Note that the above size is the maximum size of primary cache. U-Boot
 * doesn't have L2 cache support for now.
 */
#define MIPS_MAX_CACHE_SIZE	0x10000

#define INDEX_BASE	CKSEG0

	.macro	cache_op op addr
	.set	push
	.set	noreorder
	.set	mips3
	cache	\op, 0(\addr)
	.set	pop
	.endm

	.macro	f_fill64 dst, offset, val
	LONG_S	\val, (\offset +  0 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  1 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  2 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  3 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  4 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  5 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  6 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  7 * LONGSIZE)(\dst)
#if LONGSIZE == 4
	LONG_S	\val, (\offset +  8 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  9 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 10 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 11 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 12 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 13 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 14 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 15 * LONGSIZE)(\dst)
#endif
	.endm

/*
 * mips_init_icache(uint PRId, ulong icache_size, unchar icache_linesz)
 */
LEAF(mips_init_icache)
	blez		a1, 9f
	mtc0		zero, CP0_TAGLO
	/* clear tag to invalidate */
	PTR_LI		t0, INDEX_BASE
	PTR_ADDU	t1, t0, a1
1:	cache_op	Index_Store_Tag_I t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
	/* fill once, so data field parity is correct */
	PTR_LI		t0, INDEX_BASE
2:	cache_op	Fill t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 2b
	/* invalidate again - prudent but not strictly neccessary */
	PTR_LI		t0, INDEX_BASE
1:	cache_op	Index_Store_Tag_I t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
9:	jr		ra
	END(mips_init_icache)

/*
 * mips_init_dcache(uint PRId, ulong dcache_size, unchar dcache_linesz)
 */
LEAF(mips_init_dcache)
	blez		a1, 9f
	mtc0		zero, CP0_TAGLO
	/* clear all tags */
	PTR_LI		t0, INDEX_BASE
	PTR_ADDU	t1, t0, a1
1:	cache_op	Index_Store_Tag_D t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
	/* load from each line (in cached space) */
	PTR_LI		t0, INDEX_BASE
2:	LONG_L		zero, 0(t0)
	PTR_ADDU	t0, a2
	bne		t0, t1, 2b
	/* clear all tags */
	PTR_LI		t0, INDEX_BASE
1:	cache_op	Index_Store_Tag_D t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
9:	jr		ra
	END(mips_init_dcache)

/*
 * mips_cache_reset - low level initialisation of the primary caches
 *
 * This routine initialises the primary caches to ensure that they have good
 * parity.  It must be called by the ROM before any cached locations are used
 * to prevent the possibility of data with bad parity being written to memory.
 *
 * To initialise the instruction cache it is essential that a source of data
 * with good parity is available. This routine will initialise an area of
 * memory starting at location zero to be used as a source of parity.
 *
 * RETURNS: N/A
 *
 */
NESTED(mips_cache_reset, 0, ra)
	move	RA, ra
	li	t2, CONFIG_SYS_ICACHE_SIZE
	li	t3, CONFIG_SYS_DCACHE_SIZE
	li	t4, CONFIG_SYS_CACHELINE_SIZE
	move	t5, t4

	li	v0, MIPS_MAX_CACHE_SIZE

	/*
	 * Now clear that much memory starting from zero.
	 */
	PTR_LI		a0, CKSEG1
	PTR_ADDU	a1, a0, v0
2:	PTR_ADDIU	a0, 64
	f_fill64	a0, -64, zero
	bne		a0, a1, 2b

	/*
	 * The caches are probably in an indeterminate state,
	 * so we force good parity into them by doing an
	 * invalidate, load/fill, invalidate for each line.
	 */

	/*
	 * Assume bottom of RAM will generate good parity for the cache.
	 */

	/*
	 * Initialize the I-cache first,
	 */
	move	a1, t2
	move	a2, t4
	PTR_LA	t7, mips_init_icache
	jalr	t7

	/*
	 * then initialize D-cache.
	 */
	move	a1, t3
	move	a2, t5
	PTR_LA	t7, mips_init_dcache
	jalr	t7

	jr	RA
	END(mips_cache_reset)

	
#define Index_Invalidate_I              0x00		/* 0	0 */
#define Index_Writeback_Inv_D           0x01		/* 0	1 */
#define InvAllScache					0x03		/* 0	3 */
#define Index_Load_Tag_I                0x04		/* 1	0 */
#define Index_Load_Tag_D                0x05		/* 1	1 */
#define IndexLoadTagScache		 		0x07		/* 1	3 */
#define Index_Store_Tag_I               0x08		/* 2	0 */
#define Index_Store_Tag_D               0x09		/* 2	1 */
#define IndexStoreTagScache		 		0x0b		/* 2	3 */
#define Create_Dirty_Exc_D              0x0D		/* 3	1 */
#define Hit_Invalidate_I                0x10		/* 4	0 */
#define Hit_Invalidate_D                0x11		/* 4	1 */
#define Fill_I                          0x14		/* 5	0 */
#define Hit_Writeback_Inv_D             0x15		/* 5	1 */
#define PageInvScache			 		0x17		/* 5	3 */
#define Hit_Writeback_I                 0x18		/* 6	0 */
#define Hit_Writeback_D                 0x19		/* 6	1 */

        .globl  cache_flush
        .ent    cache_flush
        .set noreorder
cache_flush:
		blez	a1, FLUSH_END
		li		t2, 0xfffffff0			# Get start address
		and		t0, a0, t2
		add		t1, a0, a1				# Get end address
		addu	t1, -1
		and		t1, t2
FLUSH_D:
        .set    mips3
        cache   Hit_Writeback_D, 0(t0)  # CacheOP instr:
                                        #   hi_writeback
        .set    mips0
        bne     t0, t1, FLUSH_D         # if flushed data len. < len
        addiu   t0, 16                  # next cache line
FLUSH_END:
        jr      ra
        nop

        .end    cache_flush


/*
 * dcache_status - get cache status
 *
 * RETURNS: 0 - cache disabled; 1 - cache enabled
 *
 */
LEAF(dcache_status)
	mfc0	t0, CP0_CONFIG
	li	t1, CONF_CM_UNCACHED
	andi	t0, t0, CONF_CM_CMASK
	move	v0, zero
	beq	t0, t1, 2f
	li	v0, 1
2:	jr	ra
	END(dcache_status)

/*
 * dcache_disable - disable cache
 *
 * RETURNS: N/A
 *
 */
LEAF(dcache_disable)
	mfc0	t0, CP0_CONFIG
	li	t1, -8
	and	t0, t0, t1
	ori	t0, t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG
	jr	ra
	END(dcache_disable)

/*
 * dcache_enable - enable cache
 *
 * RETURNS: N/A
 *
 */
LEAF(dcache_enable)
	mfc0	t0, CP0_CONFIG
	ori	t0, CONF_CM_CMASK
	xori	t0, CONF_CM_CMASK
	ori	t0, CONFIG_SYS_MIPS_CACHE_MODE
	mtc0	t0, CP0_CONFIG
	jr	ra
	END(dcache_enable)


#define Index_Invalidate_I              0x00		/* 0	0 */
#define Index_Writeback_Inv_D           0x01		/* 0	1 */
#define InvAllScache					0x03		/* 0	3 */
#define Index_Load_Tag_I                0x04		/* 1	0 */
#define Index_Load_Tag_D                0x05		/* 1	1 */
#define IndexLoadTagScache		 		0x07		/* 1	3 */
#define Index_Store_Tag_I               0x08		/* 2	0 */
#define Index_Store_Tag_D               0x09		/* 2	1 */
#define IndexStoreTagScache		 		0x0b		/* 2	3 */
#define Create_Dirty_Exc_D              0x0D		/* 3	1 */
#define Hit_Invalidate_I                0x10		/* 4	0 */
#define Hit_Invalidate_D                0x11		/* 4	1 */
#define Fill_I                          0x14		/* 5	0 */
#define Hit_Writeback_Inv_D             0x15		/* 5	1 */
#define PageInvScache			 		0x17		/* 5	3 */
#define Hit_Writeback_I                 0x18		/* 6	0 */
#define Hit_Writeback_D                 0x19		/* 6	1 */

#if 0
        .globl  cache_flush
        .ent    cache_flush
        .set noreorder
cache_flush:
		blez	a1, FLUSH_END
		li		t2, 0xfffffff0			# Get start address
		and		t0, a0, t2
		add		t1, a0, a1				# Get end address
		addu	t1, -1
		and		t1, t2
FLUSH_D:
        .set    mips3
        cache   Hit_Writeback_D, 0(t0)  # CacheOP instr:
                                        #   hi_writeback
        .set    mips0
        bne     t0, t1, FLUSH_D         # if flushed data len. < len
        addiu   t0, 16                  # next cache line
FLUSH_END:
        jr      ra
        nop

        .end    cache_flush

#endif


#define MMFC0(rt,rd,sel)		\
	.word	0x40000000|(rt<<16)|(rd<<11)|sel

#define MMTC0(rt,rd,sel)		\
	.word	0x40800000|(rt<<16)|(rd<<11)|sel
	

        .align  2
        .globl  _invalidate_cache
        .ent    _invalidate_cache
_invalidate_cache:
        #--- transition to kseg1 from undetermined kernel segment
        la      t1, 1f
        or      t1, 0xa0000000
        jr      t1
        nop
1:
 	# Determine how big the I$ is

	.word 0x400a8001	#mfc0	$10, $16, 1		

	bnez    a0,10f           	# if not i-cache, invalidate d-cache
	nop
	# Isolate I$ Line Size
	sll	$11, $10, 10
	srl	$11, 29

	# Skip ahead if No I$
	beq	$11, $0, 10f
	nop

	li	$14, 2
	sllv	$11, $14, $11		# Now have true I$ line size in bytes

	sll	$12, $10, 7
	srl	$12, 29
	li	$14, 64
	sllv	$12, $14, $12		# I$ Sets per way

	sll	$13, $10, 13
	srl	$13, 29			# I$ Assoc (-1)
	add	$13, 1
	mul	$12, $12, $13		# Total number of sets

	lui	$14, 0x8000		# Get a KSeg0 address for cacheops

	# Clear TagLo/TagHi registers
	mtc0	$0, $28
	mtc0	$0, $29

	move	$15, $12	

	# Index Store Tag Cache Op
	# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
1:	
	.set mips3
	cache	0x8, 0($14)
	.set mips1
	addiu	$15, -1			# Decrement set counter

	bne	$15, $0, 1b
	add	$14, $11		# Get next line address

	# Now go through and invalidate the D$
	# Now that the I$ has been flushed, the rest of the code can be
	# moved to kseg0 and run from the cache to go faster
10:	
	addi    a0,-1
        bnez    a0,10f
	
	# Isolate D$ Line Size
	sll	$11, $10, 19
	srl	$11, 29

	# Skip ahead if No D$
	beq	$11, $0, 10f
	nop

	li	$14, 2
	sllv	$11, $14, $11		# Now have true D$ line size in bytes

	sll	$12, $10, 16
	srl	$12, 29
	li	$14, 64
	sllv	$12, $14, $12		# D$ Sets per way

	sll	$13, $10, 22
	srl	$13, 29			# D$ Assoc (-1)
	add	$13, 1

	mul	$12, $12, $13		# Get total number of sets
	
	lui	$14, 0x8000		# Get a KSeg0 address for cacheops

	# Clear TagLo/TagHi registers
	mtc0	$0, $28
	mtc0	$0, $29
#	mtc0	$0, $28, 2
	MMTC0(0,28,2)
#	mtc0	$0, $29, 2
	MMTC0(0,29,2)
	
	move	$15, $12	

	# Index Store Tag Cache Op
	# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
1:	
	.set mips3
	cache	0x9, 0($14)
	.set mips1
	addiu	$15, -1			# Decrement set counter

	bne	$15, $0, 1b
	add	$14, $11		# Get next line address
10:
        jr       ra
        nop

        .end    _invalidate_cache




 #--------------------------------------------------------------------
 #
 #   Function_Name: cache_invalidate
 #
 #--------------------------------------------------------------------
        .globl  cache_invalidate
        .ent    cache_invalidate
        .set noreorder
cache_invalidate:
		blez	a1, INVALIDATE_END
		li		t2, 0xfffffff0			# Get start address
		and		t0, a0, t2
		add		t1, a0, a1				# Get end address
		addu	t1, -1
		and		t1, t2
INVALIDATE_D:
        .set    mips3
        cache   Hit_Invalidate_D, 0(t0) # CacheOP instr:
                                        #   hi_invalidate
        .set    mips0
        bne     t0, t1, INVALIDATE_D    # if invalidated data len. < len
        addiu   t0, 16                  # next cache line
INVALIDATE_END:
        jr      ra
        nop

        .end    cache_invalidate

       
