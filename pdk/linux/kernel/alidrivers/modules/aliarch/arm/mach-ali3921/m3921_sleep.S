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

#include <linux/linkage.h>
#include <asm/smp_scu.h>
#include <asm/memory.h>
#include <asm/hardware/cache-l2x0.h>

#include "m3921_sleep.h"



.macro	DO_SMC
	dsb
//	smc	#0
	dsb
.endm

/*
 * =============================
 * == CPU suspend finisher ==
 * =============================
 *
 * void omap4_finish_suspend(unsigned long cpu_state)
 *
 * This function code saves the CPU context and performs the CPU
 * power down sequence. Calling WFI effectively changes the CPU
 * power domains states to the desired target power state.
 *
 * @cpu_state : contains context save state (r0)
 *	0 - No context lost
 * 	1 - CPUx L1 and logic lost: MPUSS CSWR
 * 	2 - CPUx L1 and logic lost + GIC lost: MPUSS OSWR
 *	3 - CPUx L1 and logic lost + GIC + L2 lost: MPUSS OFF
 * @return: This function never returns for CPU OFF and DORMANT power states.
 * Post WFI, CPU transitions to DORMANT or OFF power state and on wake-up
 * from this follows a full CPU reset path via ROM code to CPU restore code.
 * The restore function pointer is stored at CPUx_WAKEUP_NS_PA_ADDR_OFFSET.
 * It returns to the caller for CPU INACTIVE and ON power states or in case
 * CPU failed to transition to targeted OFF/DORMANT state.
 */
ENTRY(ali_3921_finish_suspend)
	stmfd	sp!, {lr}

	bl	v7_flush_dcache_all
	/*
	 * Clear the SCTLR.C bit to prevent further data cache
	 * allocation. Clearing SCTLR.C would make all the data accesses
	 * strongly ordered and would not hit the cache.
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #(1 << 2)		@ Disable the C bit
	mcr	p15, 0, r0, c1, c0, 0
	isb

	/*
	 * Invalidate L1 data cache. Even though only invalidate is
	 * necessary exported flush API is used here. Doing clean
	 * on already clean cache would be almost NOP.
	 */
	bl	v7_flush_dcache_all

	/*
	 * Switch the CPU from Symmetric Multiprocessing (SMP) mode
	 * to AsymmetricMultiprocessing (AMP) mode by programming
	 * the SCU power status to DORMANT or OFF mode.
	 * This enables the CPU to be taken out of coherency by
	 * preventing the CPU from receiving cache, TLB, or BTB
	 * maintenance operations broadcast by other CPUs in the cluster.
	 */
	mrc	p15, 0, r0, c1, c1, 2		@ Read NSACR data
	tst	r0, #(1 << 18)
	mrcne	p15, 0, r0, c1, c0, 1
	bicne	r0, r0, #(1 << 6)		@ Disable SMP bit
	mcrne	p15, 0, r0, c1, c0, 1
	isb
	dsb
	/*
	 * Clean and invalidate the L2 cache.
	 * Common cache-l2x0.c functions can't be used here since it
	 * uses spinlocks. We are out of coherency here with data cache
	 * disabled. The spinlock implementation uses exclusive load/store
	 * instruction which can fail without data cache being enabled.
	 * OMAP4 hardware doesn't support exclusive monitor which can
	 * overcome exclusive access issue. Because of this, CPU can
	 * lead to deadlock.
	 */
	bl	ali_m3921_get_l2cache_base
	mov	r2, r0
	ldr	r0, =0xffff
	str	r0, [r2, #L2X0_CLEAN_INV_WAY]
wait:
	ldr	r0, [r2, #L2X0_CLEAN_INV_WAY]
	ldr	r1, =0xffff
	ands	r0, r0, r1
	bne	wait
l2x_sync:
	bl	ali_m3921_get_l2cache_base
	mov	r2, r0
	mov	r0, #0x0
	str	r0, [r2, #L2X0_CACHE_SYNC]
sync:
	ldr	r0, [r2, #L2X0_CACHE_SYNC]
	ands	r0, r0, #0x1
	bne	sync

do_WFI:
	bl	ali_3921_do_wfi

	/*
	 * CPU is here when it failed to enter OFF/DORMANT or
	 * no low power state was attempted.
	 */
	mrc	p15, 0, r0, c1, c0, 0
	tst	r0, #(1 << 2)			@ Check C bit enabled?
	orreq	r0, r0, #(1 << 2)		@ Enable the C bit
	mcreq	p15, 0, r0, c1, c0, 0
	isb

	/*
	 * Ensure the CPU power state is set to NORMAL in
	 * SCU power state so that CPU is back in coherency.
	 * In non-coherent mode CPU can lock-up and lead to
	 * system deadlock.
	 */
	mrc	p15, 0, r0, c1, c0, 1
	tst	r0, #(1 << 6)			@ Check SMP bit enabled?
	orreq	r0, r0, #(1 << 6)
	mcreq	p15, 0, r0, c1, c0, 1
	isb
	mov	r0, #SCU_PM_NORMAL
	mov	r1, #0x00
	stmfd   r13!, {r4-r12, r14}
//	ldr	r12, =OMAP4_MON_SCU_PWR_INDEX
	DO_SMC
	ldmfd   r13!, {r4-r12, r14}
	isb
	dsb
	ldmfd	sp!, {pc}
ENDPROC(ali_3921_finish_suspend)


ENTRY(ali_3921_do_wfi)
//	stmfd	sp!, {lr}

	/*
	 * Execute an ISB instruction to ensure that all of the
	 * CP15 register changes have been committed.
	 */
	isb

	/*
	 * Execute a barrier instruction to ensure that all cache,
	 * TLB and branch predictor maintenance operations issued
	 * by any CPU in the cluster have completed.
	 */
	dsb
	dmb

	movw r2,#0x3d00
	movt  r2,#0xFEFF	//#0x1805
	ldr	r1,=return
	str	r1,[r2,#0x04]	//return addr save at :0x18053d04
	
	movw	r2,#(0x3eb0+0x80+0x38)
	movt	r2,#0xFEFF		//  reg  save_OFFSET 
	str	r4,[r2]	
	str	r5,[r2,#0x04]	
	str	r6,[r2,#0x08]	
	str	r7,[r2,#0x0c]	
	str	r8,[r2,#0x0c]	
	str	r9,[r2,#0x10]	
	str	r10,[r2,#0x14]	
	str	r11,[r2,#0x18]	
	str	r12,[r2,#0x1c]	
	str	r13,[r2,#0x20]	
	str	r14,[r2,#0x24]	

	//17	Normal Access DDR3
	movw	r2,0x3d04	//return addr
	movt	r2,0x1805
	
	movw	r0,0x0000
	movt	r0,0x1805

jump_to_func:
//    push {r4-r11}
    @ disable mmu
    dsb
    mrc p15, 0, r1, c1, c0, 0
    bic r1, r1, #0x1
    mcr p15, 0, r1, c1, c0, 0
//    isb
	nop
	nop
/*    @ invalidate BHT
    mov r1, #0x1
    mcr p15, 0, r1, c7, c1, 6
    isb */
    @ jump to physical address
    mov lr, pc
    mov pc, r0
    @ enable mmu again
    mrc p15, 0, r1, c1, c0, 0
    orr r1, r1, #0x1
    mcr p15, 0, r1, c1, c0, 0
    isb
    @ invalidate BHT
    mov r1, #0x1
    mcr p15, 0, r1, c7, c1, 6
    isb
    @ return to c code to get next mem init code
//    pop {r4-r11, pc}

return:	
//	b	return
//	ldmfd	sp!, {pc}
// 1:
//	b	1b
//    pop {r4-r11, pc}
	bx      lr
	nop
ENDPROC(ali_3921_do_wfi)
