/*
 * ali-cache.S
 *
 * Copyright (C) 2013 ALi, Inc.
 *
 * Author:
 *	Tony Zhang <tony.zhang@alitech.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/linkage.h>
#include <linux/init.h>

__INIT

ENTRY(ALI_L2X0_Init)

	/*
	* L2_TAG_RAM_CLK/L2_DATA_RAM_CLK
	* r0->0x18000094
	* r1->0x1BF02000
	*/
	stmfd   sp!, {r0 - r4, lr}
	mov  r3, r0
	ldr r4, [r3]
	bic r4, r4, #0xc000000
	orr r4, r4, #0x8000000 @ L2_TCLK_RATIO_10
	str r4, [r3]
	ldr r4, [r3]
	bic r4, r4, #0x3000000
	orr r4, r4, #0x2000000 @ L2_DCLK_RATIO_10
	str r4, [r3]

	/*L2_RAM_Latency*/

      @MOVW r1, #0x2000
      @MOVT r1, #0x1BF0

       LDR r2,[r1,#0x108]    @ TAG RAM control register            
       BIC r2,r2,#0x7<<8     @ Clear
       BIC r2,r2,#0x7<<4     @ Clear
       BIC r2,r2,#0x7<<0     @ Clear
       ORR r2,r2,#0x0<<8     @ Write latency
       ORR r2,r2,#0x1<<4     @ Read latency
       ORR r2,r2,#0x0<<0     @ Setup latency
       STR r2,[r1,#0x108]
        
	/* ******************************************************************************** */

       LDR r2,[r1,#0x10C]    @ DATA RAM control register
        
       BIC r2,r2,#0x7<<8     @ Clear
       BIC r2,r2,#0x7<<4     @ Clear
       BIC r2,r2,#0x7<<0     @ Clear
       ORR r2,r2,#0x0<<8     @ Write latency
       ORR r2,r2,#0x1<<4     @ Read latency
       ORR r2,r2,#0x0<<0     @ Setup latency
       STR r2,[r1,#0x10C]
	ldmfd   sp!, {r0 - r4, pc}

ENDPROC(ALI_L2X0_Init)

