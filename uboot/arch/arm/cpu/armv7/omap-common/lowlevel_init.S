/*
 * Board specific setup info
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/arch/omap.h>
#include <configs/ali_3921.h>

.global save_boot_params
save_boot_params:
	/*
	 * See if the rom code passed pointer is valid:
	 * It is not valid if it is not in non-secure SRAM
	 * This may happen if you are booting with the help of
	 * debugger
	 */
	ldr     r2, =NON_SECURE_SRAM_START
	cmp	r2, r0
	bgt	1f
	ldr	r2, =NON_SECURE_SRAM_END
	cmp	r2, r0
	blt	1f

	/*
	 * store the boot params passed from rom code or saved
	 * and passed by SPL
	 */
	cmp	r0, #0
	beq	1f
	ldr	r1, =boot_params
	str	r0, [r1]
#ifdef CONFIG_SPL_BUILD
	/* Store the boot device in omap_boot_device */
	ldrb	r2, [r0, #BOOT_DEVICE_OFFSET]	@ r1 <- value of boot device
	and	r2, #BOOT_DEVICE_MASK
	ldr	r3, =boot_params
	strb	r2, [r3, #BOOT_DEVICE_OFFSET]	@ omap_boot_device <- r1

	/* boot mode is passed only for devices that can raw/fat mode */
	cmp	r2, #2
	blt	2f
	cmp	r2, #7
	bgt	2f
	/* Store the boot mode (raw/FAT) in omap_boot_mode */
	ldr	r2, [r0, #DEV_DESC_PTR_OFFSET]	@ get the device descriptor ptr
	ldr	r2, [r2, #DEV_DATA_PTR_OFFSET]	@ get the pDeviceData ptr
	ldr	r2, [r2, #BOOT_MODE_OFFSET]	@ get the boot mode
	ldr	r3, =omap_bootmode
	str	r2, [r3]
#endif
2:
	ldrb	r2, [r0, #CH_FLAGS_OFFSET]
	ldr	r3, =boot_params
	strb	r2, [r3, #CH_FLAGS_OFFSET]
1:
	bx	lr


.globl lowlevel_init
lowlevel_init:
#ifndef S3921_FT_NOR_TEST
	/*
	 * Setup a temporary stack
	 */
	ldr	sp, =LOW_LEVEL_SRAM_STACK

	/*
	 * Save the old lr(passed in ip) and the current lr to stack
	 */
	push	{ip, lr}

	/*
	 * go setup pll, mux, memory
	 */
	bl	s_init
	pop	{ip, pc}

.globl set_pl310_ctrl_reg
set_pl310_ctrl_reg:
	PUSH	{r4-r11, lr}	@ save registers - ROM code may pollute
				@ our registers
	LDR	r12, =0x102	@ Set PL310 control register - value in R0
	.word	0xe1600070	@ SMC #0 - hand assembled because -march=armv5
				@ call ROM Code API to set control register
	POP	{r4-r11, pc}
#else//source code comes from sdram_3921_512MB_1066Mbps.S
	ldr    r1,  =0xffff0001
delay0:
	subs	r1,r1,#0x01
	bne	delay0

	ldr    r0,  =0x18018300
	ldr    r1,  ='d'
	strb	r1,   [r0]
	
	
sdram_init:
	ldr          r2, =0x1803e000
	ldr          r3, =0x1803f000
	
change_mem_clk:	
	ldr          r0, =0x18000000
	ldr          r1, =0x00200080
	str	       r1, [r0,#0x74]		//Data.Out IO:0x18000074 %Long 0x00200040
	
	ldr          r0, =0x18001000
enable_mem_clk1:
	ldr          r1, =0x603fffff
	str	       r1, [r0,#0x30]		//Data.Out IO:0x18001030 %Long 0x603fffff									;DM_CTRL enable mem clk

	ldr          r1, =0x90900000
	str	       r1, [r2,#0x2c]	// DQS1/DQS0	rx/tx delay chain
	
	ldr          r0, =0x18001000// dm ctrl  base addr 0x18001000
	
set_phy1:
	ldr          r1, =0x90900000
	str	       r1, [r2,#0x2c]		//Data.Out IO:0x1803E02C %Long 0x90900000
	ldr	       r1, [r2,#0x8c]	//temp = 0x1803e08c
	lsl	       r1, r1,#23
	lsr	       r1, r1,#23	//temp = temp&0x000001ff
	lsr	       r1, r1,#02 	//temp = temp>>2
	add	       r1, r1,#0x28	//temp = temp + 0x24
	//lsl	r1,r1,#24	//temp = temp<<24
	strb	       r1, [r2,#0x33]	//byte 0x1803E033 = temp
	//movw	r1,#0x00
	//strb	r1,[r2,#0x33]	//Data.Out IO:0x1803E033 %Byte 0x00

	ldr          r1, =0x00004040
	str	       r1, [r1,#0x34]		//Data.Out IO:0x1803E034 %Long 0x00004040
	ldr    	r1, =0xb010
	strh   	r1, [r2,#0x30]	//	Data.Out IO:0x1803E030 %Word 0xb010

	ldr          r1, =0x809bc318
	str	       r1, [r2,#0x38]	// 0x1803E038 = 0x809b_c318
	ldr          r1, =0x11041104
	str	       r1, [r2,#0x40]	// 0x1803E040 = 0x00050005
	ldr          r1, =0x11041104
	str	       r1, [r2,#0x44]	// 0x1803E044 = 0x00050005
	ldr          r1, =0x22042204
	str	       r1, [r2,#0x48]	// 0x1803E048 = 0x00050005
	ldr          r1, =0x000400c0
	str	       r1, [r2,#0x4c]	// 0x1803E04c = 0x000500c0

set_phy2:
	ldr          r1, =0x90900000
	str	       r1, [r3,#0x2c]		//Data.Out IO:0x1803f02C %Long 0x00000000
	ldr	       r1, [r3,#0x8c]	//temp = 0x1803F08c
	lsl	       r1, r1,#23
	lsr	       r1, r1,#23	//temp = temp&0x000001ff
	lsr	       r1, r1,#02 	//temp = temp>>2
	add	       r1, r1,#0x28	//temp = temp + 0x08
	//lsl	r1,r1,#24	//temp = temp<<24
	strb	       r1, [r3,#0x33]	//byte 0x1803F033 = temp
	//movw	r1,#0x00
	//strb	r1,[r3,#0x33]	//Data.Out IO:0x1803F033 %Byte 0x00

	ldr          r1, =0x00004040
	str	       r1, [r3,#0x34]	//Data.Out IO:0x1803F034 %Long 0x00004040
	ldr     	r1, =0xb010
	strh	       r1, [r3,#0x30]	//	Data.Out IO:0x1803F030 %Word 0xb010
	
//	movw	r1,#0x1818
//	strh	r1,[r3,#0x34]	//	half 0x1803F034	= 0x1818 
	
	ldr          r1, =0x809bc318
	str	       r1, [r3,#0x38]	// 0x1803F038 = 0x809b_c318
	ldr          r1, =0x11041104
	str	       r1, [r3,#0x40]	// 0x1803F040 = 0x00050005
	ldr          r1, =0x11041104
	str	       r1, [r3,#0x44]	// 0x1803F044 = 0x00050005
	ldr          r1, =0x22042204
	str	       r1, [r3,#0x48]	// 0x1803F048 = 0x00050005
	ldr          r1, =0x000400c0
	str	       r1, [r3,#0x4c]	// 0x1803F04c = 0x000500c0


	ldr          r0, =0x18018300
	ldr     	r1, ='d'
	strb	       r1, [r0]
	
//;;;;;;;;;;;;;;;;;;; DM_CTRL initial ;;;;;;;;;;;;;;;;;;;

	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
  	ldr    	r1, =0xb861
  	strh	       r1, [r0,#0x30]	//Data.Out IO:18001030 %Word 0xB861
  	ldr     	r1, =0x89
  	strb	       r1, [r0]		//Data.Out IO:18001000 %Byte 0x89
  	ldr     	r1, =0x0c
  	strb	       r1, [r0,#0x04]	//Data.Out IO:18001004 %Byte 0x0C
  	ldr          r0, =0x800026a0
	ldr		r1, [r0]	//Data.In IO:800026a0 /Long
  
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18000000
  	ldr    	r1, =0x60	
  	strb	       r1, [r0,#0x33]	//Data.Out IO:18001033 %Byte 0x60

set_dm_ctrl1:
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
	ldr    	r1, =0x80
	strb	       r1, [r0,#0x07]		//byte 0x18001007 = 0x80
	ldr          r1, =0x1c80c383
	str	       r1, [r0,#0x00]		//0x18001000 = 0x1C80C383
	ldr    	r1, =0xB861
	strh	       r1, [r0,#0x30]		//half 0x18001030 = 0xB861
	ldr     	r1, =0x60
	strb	       r1, [r0,#0x33]		//byte 0x18001033 = 0x60
	ldr    	r1, =0x20
	strb	       r1, [r0,#0x33]		//byte 0x18001033 = 0x20
	ldr    	r1, =0xc0
	strb	       r1, [r0,#0x33]		//byte 0x18001033 = 0xc0
	ldr    	r1, =0x60
	strb	       r1, [r0,#0x33]		//byte 0x18001033 = 0x60

set_1st_ch:
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
	ldr    	r1, =0x1c
	strb	       r1, [r0,#0x04]	//	byte 0x18001004 = 0x1c
	ldr          r2, =0x80082180
	ldr	       r1, [r2]	// read MR2_ch_1
	ldr          r2, =0x800c0000
	ldr	       r1, [r2]	//read 0xa00C0000
	ldr          r2, =0x80040020
	ldr	       r1, [r2]	//read 0xa0040020
	ldr          r2, =0x8001d700
	ldr	       r1, [r2]	//read MR0_ch_1
	
	// issue ZQCL
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
	ldr    	r1, =0x2c	//ldr	r1,=0x2c
	strb	       r1, [r0,#0x04]		// byte 0x18001004 = 0x2C
	ldr          r2, =0x80004000
	ldr	       r1, [r2]	//read 0x80004000
	
set_2st_ch:
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
	ldr     	r1, =0x1c
	strb	       r1, [r0,#0x04]	//	byte 0x18001004 = 0x1c
	ldr          r2, =0x88082180
	ldr	       r1, [r2]	// read MR2_ch_2
	ldr          r2, =0x880c0000
	ldr	       r1, [r2]	//read 0xa80C0000
	ldr          r2, =0x88040020
	ldr	       r1, [r2]	//read 0xa8040020
	ldr          r2, =0x8801d700
	ldr	       r1, [r2]	//read MR0_ch_2
	
	// issue ZQCL
	ldr    	r1, =0x2c	//ldr	r1,=0x2c
	strb	       r1, [r0,#0x04]		// byte 0x18001004 = 0x2C
	ldr          r2, =0x88004000
	ldr	       r1, [r2]	//read 0x88004000

nop:
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
	ldr     	r1, =0x0c
	strb	       r1, [r0,#0x04]		//byte 0x18001004 = 0x0C
	ldr          r2, =0x800026a0
	ldr	       r1, [r2]			//read 0xa00026a0	
	ldr	       r1, [r2]			//read 0xa00026a0	
	ldr          r3, =0x880026a0
	ldr	       r1, [r3]			//read 0xa80026a0	
	ldr	       r1, [r3]			//read 0xa80026a0

precharge:
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
	ldr    	r1, =0x14
	strb	       r1, [r0,#0x04]		//byte 0x18001004 = 0x14
	ldr	       r1, [r2]			//read 0xa00026a0  ch1
	ldr	       r1, [r3]			//read 0xa80026a0  ch2

auto_refresh:
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
	ldr    	r1, =0x24
	strb	       r1, [r0,#0x04]		//byte 0x18001004 = 0x24
	ldr	       r1, [r2]			//read 0xa00026a0   ch1
	ldr	       r1, [r3]			//read 0xa80026a0   ch2

normal:
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
	ldr    	r1, =0x00
	strb	       r1, [r0,#0x04]		// byte 0x18001004 = 0x00

set_dm_ctrl_step2:
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001100
	ldr          r1, =0x70ad7034
	str	       r1, [r0,#0x00]	//Data.Out IO:0x18001000 %Long 0x70ad7034			
	ldr          r1, =0x80139fc0
	str	       r1, [r0,#0x04]	//	0x18001004 = 0x80139FC0	
	ldr     	r1, =0x50
	strb	       r1, [r0,#0x31]	//	byte 0x18001031 = 0x50	
	ldr    	r1, =0x40
	strb	       r1, [r0,#0x0d]	//	byte 0x1800100d	= 0x40
	ldr    	r1, =0xfaff
	strh	       r1, [r0,#0x0e]	//	half 0x1800100e	= 0xfaff
	ldr          r1, =0xffffffff
	ldr          r2, =0x18000200
	str	       r1, [r2,#0x24]	//	0x18000224 = 0xffffffff	
	str	       r1, [r2,#0x28]	//	0x18000224 = 0xffffffff	
	str	       r1, [r2,#0x2c]	//	0x18000224 = 0xffffffff	
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001100
	str	       r1, [r0,#0x10]	//	0x18001010 = 0xffffffff	
	ldr          r1, =0x88888888
	str	       r1, [r0,#0x18]	//	0x18001018 = 0x88888888	
	ldr          r1, =0x88888888
	str	       r1,[r0,#0x78]	//	0x18001078 = 0x88888888	
	ldr          r1, =0xffffffff
	str	       r1, [r0,#0x7c]	//	0x1800107c = 0xffffffff	
	str	       r1, [r0,#0x80]	//	0x18001080 = 0xffffffff	
	str	       r1, [r0,#0x20]	//	0x18001020 = 0xffffffff	
	str	       r1, [r0,#0x24]	//	0x18001024 = 0xffffffff	
	ldr          r1, =0xffff8022
	str	       r1, [r0,#0x08]	//Data.Out IO:0x18001008 %Long 0xFFFF8022
	ldr          r1, =0x33330f00
	str	       r1, [r0,#0x14]	//	0x18001014 = 0x33330f00	
	ldr          r1, =0xffffffff
	str	       r1, [r0,#0x1c]	//	0x1800101c = 0xffffffff	
	ldr          r1, =0xffff8022
	str	       r1, [r0,#0x34]	//Data.Out IO:0x18001034 %Long 0xFFFF8022
	ldr          r1, =0x33330f00
	str	       r1, [r0,#0x38]	//	0x18001038 = 0x33330f00	
	ldr          r1, =0xffffffff
	str	       r1, [r0,#0x3c]	//	0x1800103c = 0xffffffff	
	ldr          r1, =0xffff8022
	str	       r1, [r0,#0x28]	//Data.Out IO:0x18001028 %Long 0xFFFF8022
	ldr          r1, =0x33330f00
	str	       r1, [r0,#0x2c]	//	0x1800102c = 0x33330f00	
	ldr          r1, =0xffffffff
	str	       r1, [r0,#0x68]	//	0x18001068 = 0xffffffff	
	ldr          r1, =0xffff8022
	str	       r1, [r0,#0x94]	//	0x18001094 = 0xffff8022	
	ldr          r1, =0x33330f00
	str	       r1, [r0,#0x98]	//	0x18001098 = 0x33330f00	
	ldr          r1, =0xffffffff
	str	       r1, [r0,#0x9c]	//	0x1800109c = 0xffffffff	
	ldr          r1, =0xffff8022
	str	       r1, [r0,#0xa0]	//	0x180010A0 = 0xffff8022	
	ldr          r1, =0x33330f00
	str	       r1, [r0,#0xa4]	//	0x180010A4 = 0x33330f00	
	ldr          r1, =0xffffffff
	str	       r1, [r0,#0xa8]	//	0x180010A8 = 0xffffffff	
	ldr          r1, =0xffff8022
	strb	       r1, [r0,#0xe0]	//	0x180010E0 = 0xffff8022	
	ldr          r1, =0x33330f00
	str	       r1, [r0,#0xe4]	//	0x180010E4 = 0x33330f00	
	ldr          r1, =0xffffffff
	str	       r1, [r0,#0xe8]	//	0x180010E8 = 0xffffffff	

	
// CPU CLK settoing
	ldr          r0, =0x18001000	// base addr
	ldr          r1, =0x4fd80000
	str	       r1, [r0,#0x94]	//Data.Out IO:0x18000094 %Long 0x4FD80000

// dram BIST setting
	ldr          r0, =0x18001000	// dm ctrl  base addr 0x18001000
	ldr          r1, =0x55555555
	str	       r1, [r0,#0xb0]	//Data.Out IO:0x180010b0 %Long 0x55555555
	ldr          r1, =0xaaaaaaaa
	str	       r1, [r0,#0xb4]	//Data.Out IO:0x180010b4 %Long 0xAAAAAAAA
	ldr          r1, =0x00f00000
	str	       r1, [r0,#0xb8]	//Data.Out IO:0x180010b8 %Long 0x00f00000
	ldr          r1, =0x0000ffe0
	str	       r1, [r0,#0xc0]	//Data.Out IO:0x180010C0 %Long 0x0000ffe0
 
	ldr    	r1, =0xffff
wait5:
	subs	       r1, r1, #0x01
	bne	       wait5
	ldr          r2, =0x18018300
	ldr    	r1, ='r'
	strb	       r1, [r2]

	ldr          r1, =0x0002ffff
wait6:
	subs	       r1, r1, #0x01
	bne	       wait6
	ldr          r2, =0x18018300
	ldr    	r1, ='1'
	strb	       r1, [r2]
	
	ldr          r1, =0x0002ffff
wait7:
	subs	       r1, r1, #0x01
	bne	       wait7
	ldr          r2, =0x18018300
	ldr    	r1, ='0'
	strb	       r1, [r2]
	
	ldr          r1, =0x0002ffff
wait8:
	subs	       r1, r1, #0x01
	bne	       wait8
	ldr          r2, =0x18018300
	ldr    	r1, ='6'
	strb	       r1, [r2]
	
	ldr          r1, =0x0002ffff
wait9:
	subs	       r1, r1, #0x01
	bne	       wait9
	ldr          r2, =0x18018300
	ldr    	r1, ='6'
	strb	       r1, [r2]
 	
end:
	bx      lr

#endif
