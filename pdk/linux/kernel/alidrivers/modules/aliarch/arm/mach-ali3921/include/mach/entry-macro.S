/*
 * arch/arm/mach-ali3921/include/mach/entry-macro.S
 *
 * Low-level IRQ helper macros for Marvell Dove platforms
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */


#include <mach/ali-3921.h>

#define SOC_IRQ_BASE_ADDR 0xFF800030

	.macro  disable_fiq
	.endm

	.macro  arch_ret_to_user, tmp1, tmp2
	.endm

	.macro  get_irqnr_preamble, base, tmp
	.endm
	
	.macro  get_irqnr_and_base, irqnr, irqstat, base, tmp
	ldr   \base, =(BASE_ADDR+0x100)
	ldr    \irqnr, [\base, #0x0C] 
	.endm
