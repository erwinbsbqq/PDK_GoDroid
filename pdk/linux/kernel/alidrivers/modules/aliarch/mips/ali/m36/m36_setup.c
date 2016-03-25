/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation. 2010 Copyright (C)
 *  (C)
 *  File: m36_printf.c
 *  (I)
 *  Description:
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.03			Sam			Create

*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version
* 2 of the License, or (at your option) any later version.
*

 ****************************************************************************/
 
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/screen_info.h>

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/mach-ali/prom.h>
#include <asm/dma.h>
#include <asm/time.h>
#include <asm/traps.h>
#ifdef CONFIG_VT
#include <linux/console.h>
#endif

#include <asm/mach-ali/typedef.h>
#include <asm/mach-ali/m36_gpio.h>
#include <asm/reboot.h>
#include <ali_soc_common.h>
#include <asm/mach-ali/chip.h>

uint32_t strappin_0x70;
extern unsigned long __G_ALI_BOARD_TYPE;

extern void __init board_setup(void);
extern bool board_is_nand_boot(void);
//int coherentio = 0;	/* no DMA cache coherency (may be set by user) */
//int hw_coherentio = 0;	/* init to 0 => no HW DMA cache coherency (reflects real HW) */

void boot_delay(u32 ms)
{
	u32 i;
	for(i=0; i<ms; i++) 
	{
		ali_udelay(1000);
	} 
}

int __init m36_init_gpio(void);
extern void sys_ic_enter_standby(unsigned int , unsigned int);
extern void IRQ_DisableInterrupt(void );

static void prom_halt(void)
{
	printk(KERN_NOTICE "\n** System halted.\n");
    IRQ_DisableInterrupt();
    *(volatile unsigned long *) 0xb8000038 =0 ;
    *(volatile unsigned long *) 0xb800003c =0 ;
    sys_ic_enter_standby(0,0) ;
	while (1)
		asm volatile (
			" mfc0    $8, $12  \t\n"  
			" nop  \t\n" 
			" nop  \t\n" 
			" nop  \t\n" 
			" li      $9, ~0x00000001  \t\n" 
			" and     $8, $9  \t\n" 
			" mtc0    $8, $12   \t\n" 
			" lui		$8, 0xb800   \t\n" 
			" sw		$0, 0x38($8)  \t\n" 
			" sw		$0, 0x3c($8)  \t\n" 
			" nop  \t\n" 
			" li	$9, 0x00001fff \t\n"
			" sw	$9, 0x80($8) \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" sw	$0, 0x80($8) \t\n"
			" sw	$0, 0x60($8) \t\n"
			" li		$10, 0x200  \t\n" 
			" 1:		addiu	$10,-1  \t\n" 
			" nop  \t\n" 
			" bnez	$10,1b  \t\n" 
			" nop  \t\n" 
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			//reset see
			" lui   $8, 0xb800 \t\n"
			" li    $9, 0xb8000280 \t\n"
			" sw    $9, 0x200($8) \t\n"
			" lbu   $9, 0x220($8) \t\n"
			" andi  $9, 0xfd \t\n"
			" sb    $9, 0x220($8) \t\n"
			//jump to nor flash addr
			" li		$30, 0xdeadbeef \t\n"
			" li	$23, 0 \t\n"
			//cur_time = (dt.sec & 0x3F ) | ((dt.min & 0x3F )<<6)  | ((dt.hour & 0x1F )<<12) | ((dt.day & 0x1F)<<17) ((dt.month & 0xF) << 22) | (((dt.year % 100) & 0x3F)<<26);
			" li	$21, 0x20421002 \t\n"	
			" li		$8, 0xbfc00000 \t\n"
			" jr		$8  \t\n"
			" nop \t\n"
		);
}


static void wdt_reboot_from_nand(void)
{
	uint32_t tmp;

    if(ali_sys_ic_get_chip_id() == ALI_C3701)    
    {	    
        *(volatile uint32_t *)(0xb8000074) |= 0x40040000;  //NOTE: just for 3701 chip        
    }    
    if(ali_sys_ic_get_chip_id() == ALI_S3503)	
    {        
		tmp = strappin_0x70;
		tmp |= ((1<<30)|(1<<29)|(1<<24));
		*(volatile uint32_t *)(0xb8000074) = tmp;
    }
}

static void wdt_reboot_from_nor(void)
{
	uint32_t tmp;
    if(ali_sys_ic_get_chip_id() == ALI_C3701)    
    {	  
		*(volatile uint32_t *)(0xb8000074) &= ~(0x1<<18);  
        *(volatile uint32_t *)(0xb8000074) |= 0x40000000; //NOTE: just for 3701 chip            
    }        
    if(ali_sys_ic_get_chip_id() == ALI_S3503)	
	{
		/*
                           74[30]    74[29]    74[24]    74[18]    74[17]    74[15]
External Nor flash boot       1        /         1          0         0         X
SIP flash boot                1        /         1          0         1         X
Nand flash boot               1        1         1          1         0         0
		*/
		strappin_0x70 = (*(volatile uint32_t *)(0xb8000070));
		tmp = strappin_0x70;
		tmp &= ~(1<<18);
		tmp |= (1<<30);
		*(volatile uint32_t *)(0xb8000074) = tmp;
	}
}

static void wdt_reboot(void)
{
    uint16_t div;
	uint32_t a, duration_us; 
    uint32_t mem_clk;
    uint32_t timebase;
    spinlock_t wdt_lock;
//    uint64_t cpu_clk;
    
    //duration_us = wdt_count; 
    duration_us = 1;

    spin_lock(&wdt_lock);
    //mem_clk = ali_sys_ic_get_dram_clock();
    mem_clk = 27;

	a = 0xffffffff / mem_clk;
	if (duration_us < (a << 5))
		div = 0;
	else if (duration_us < (a << 6))
		div = 1;
	else if (duration_us < (a << 7))
		div = 2;
	else
		div = 3;
	timebase = 0xffffffff - (duration_us / (1 << (5 + div)) * mem_clk);

    *(volatile uint32_t *)(0xb8018500) = timebase; 
#if 0
    if(ALI_C3701 == ali_sys_ic_get_chip_id())
    {
    	cpu_clk = *((volatile unsigned long *)(0xb8000070));
    	cpu_clk = (cpu_clk>>7)&0x07;
        /* cpu clk is pll clk, pll clk have no output  after watch dog reboot */
    	if(cpu_clk == 0x04)	
    	{
            /* in order to execute change_cpu_clk() */
            *(volatile uint32_t *)(0xb8018500) = timebase - 0xfff;
            load_to_icache(change_cpu_clk,0x200);
            change_cpu_clk();
    	}
    }
#endif    
    *(volatile uint8_t *)(0xb8018500+4) = 0x64 | div;  
    spin_unlock(&wdt_lock);
}
/*
 * TODO - use definition For M3606 CHIP
 * For other CHIP or CPU may need to modify
 */ 
static void prom_restart(char *command)
{
    printk(KERN_NOTICE "\n** System reboot...\n");

#if 0
	while (1)
		asm volatile (
			//set deadbead flag for aliboot
			" li	$30, 0xdeadbead \t\n"
			" li	$23, 0 \t\n"
			
			//reset ic
			" li	$8, 0xb8000000 \t\n"
			" lh	$9, 2($8) \t\n"
	        " li	$10, 0x3329 \t\n"
	        " bne	$9, $10, 1f \t\n"
			" nop \t\n"
			" lbu	$9, 0($8) \t\n"
			" sltiu	$9, $9, 0x5 \t\n"
			" bne	$0, $9, 1f \t\n"
			" nop \t\n"
			" ori	$8, 0x8000 \t\n"
		    " 1: \t\n"
			" li	$9, 0x02000d03 \t\n"
			" sw	$9, 0x98($8) \t\n"
			" mfc0  $8, $12 \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" li    $9, ~0x00000001 \t\n"
			" and   $8, $9 \t\n"
			" mtc0  $8, $12 \t\n"
			" lui	$8, 0xb800 \t\n"
			" sw	$0, 0x38($8) \t\n"
			" sw	$0, 0x3c($8) \t\n"
			" li	$9, 0xffbfffff \t\n"
			" sw	$9, 0x80($8) \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" nop \t\n"
			" move	$9, $0 \t\n"
			" sw	$9, 0x80($8) \t\n"
			" sw	$0, 0x60($8) \t\n"

			//reset see
			" lui   $8, 0xb800 \t\n"
			" li    $9, 0xb8000280 \t\n"
			" sw    $9, 0x200($8) \t\n"
			" lbu   $9, 0x220($8) \t\n"
			" andi  $9, 0xfd \t\n"
			" sb    $9, 0x220($8) \t\n"

			//jump reset entry
			" li	$21, 0 \t\n"		
			" li	$8, 0xbfc00000 \t\n"
			" jr	$8  \t\n"
			" nop \t\n"
		);
#else
	printk(KERN_EMERG "Watch Dog reset CHIP.\n");

    /*implement*/
    if(board_is_nand_boot())
        wdt_reboot_from_nand();
    else
        wdt_reboot_from_nor();

    wdt_reboot();
#endif

}


#if 0
extern unsigned long __G_MM_VIDEO_TOP_ADDR;
extern unsigned long __G_MM_VIDEO_START_ADDR;

extern unsigned long __G_MM_STILL_FRAME_SIZE;
extern unsigned long __G_MM_STILL_FRAME_ADDR;

extern unsigned long __G_MM_VCAP_FB_SIZE;
extern unsigned long __G_MM_VCAP_FB_ADDR;

extern unsigned long __G_RPC_MM_LEN;
extern unsigned long __G_MM_SHARED_MEM_TOP_ADDR;
extern unsigned long __G_MM_SHARED_MEM_START_ADDR;


extern unsigned long __G_MM_PRIVATE_AREA_TOP_ADDR;
extern unsigned long __G_MM_PRIVATE_AREA_START_ADDR;

extern unsigned long __G_MM_VDEC_VBV_START_ADDR;
extern unsigned long __G_MM_VDEC_CMD_QUEUE_ADDR;
extern unsigned long __G_MM_VDEC_LAF_FLAG_BUF_ADDR;




extern unsigned long __G_MM_MP_MEM_TOP_ADDR;
extern unsigned long __G_MM_MP_MEM_START_ADDR;

extern unsigned long __G_MM_OSD_BK_ADDR;
extern unsigned long __G_MM_TTX_BS_START_ADDR;
extern unsigned long __G_MM_TTX_PB_START_ADDR;
extern unsigned long __G_MM_TTX_SUB_PAGE_BUF_ADDR;
extern unsigned long __G_MM_TTX_P26_NATION_BUF_ADDR;
extern unsigned long __G_MM_TTX_P26_DATA_BUF_ADDR;
extern unsigned long __G_MM_SUB_BS_START_ADDR;
extern unsigned long __G_MM_SUB_HW_DATA_ADDR;
extern unsigned long __G_MM_SUB_PB_START_ADDR;
extern unsigned long __G_MM_VDEC_VBV_LEN;

extern unsigned long __G_MM_BOOTLOGO_DATA_START_ADDR;

#if 0
extern unsigned long __G_MM_SGDMA_MEM_END;
extern unsigned long __G_MM_SGDMA_MEM_START;
#endif

extern unsigned long __G_MM_DMX_MEM_TOP_ADDR;
extern unsigned long __G_MM_DMX_MEM_START_ADDR;

extern unsigned long __G_MM_TSG_BUF_LEN;
extern unsigned long __G_MM_TSG_BUF_START_ADDR;

extern unsigned long __G_GE_CMD_SIZE;

extern unsigned long __G_MM_FB_START_ADDR;

extern unsigned long __G_MM_IMAGE_DECODER_MEM_LEN;
extern unsigned long __G_MM_IMAGE_DECODER_MEM_START_ADDR;	

#if 1	
extern unsigned long __G_MM_NIM_J83B_MEM_LEN;
extern unsigned long __G_MM_NIM_J83B_MEM_START_ADDR;
#endif


void ali_show_sys_memmap(void)
{
    printk("===================MEMORY MAP START===================\n");
	
    printk("__G_MM_VIDEO_TOP_ADDR              :0x%08x\n", (unsigned int)__G_MM_VIDEO_TOP_ADDR);
    printk("__G_MM_VIDEO_START_ADDR            :0x%08x\n", (unsigned int)__G_MM_VIDEO_START_ADDR);
    printk("__G_MM_VCAP_FB_SIZE                :0x%08x\n", (unsigned int)__G_MM_VCAP_FB_SIZE);
    printk("__G_MM_VCAP_FB_ADDR                :0x%08x\n", (unsigned int)__G_MM_VCAP_FB_ADDR);	
    printk("__G_MM_STILL_FRAME_SIZE            :0x%08x\n", (unsigned int)__G_MM_STILL_FRAME_SIZE);
    printk("__G_MM_STILL_FRAME_ADDR            :0x%08x\n", (unsigned int)__G_MM_STILL_FRAME_ADDR);

    printk("__G_MM_VDEC_VBV_LEN                :0x%08x\n", (unsigned int)__G_MM_VDEC_VBV_LEN);	
    printk("__G_MM_VDEC_VBV_START_ADDR         :0x%08x\n", (unsigned int)__G_MM_VDEC_VBV_START_ADDR);
    printk("__G_MM_VDEC_CMD_QUEUE_ADDR         :0x%08x\n", (unsigned int)__G_MM_VDEC_CMD_QUEUE_ADDR);
    printk("__G_MM_VDEC_LAF_FLAG_BUF_ADDR      :0x%08x\n", (unsigned int)__G_MM_VDEC_LAF_FLAG_BUF_ADDR);	

    printk("__G_MM_MP_MEM_TOP_ADDR             :0x%08x\n", (unsigned int)__G_MM_MP_MEM_TOP_ADDR);
    printk("__G_MM_MP_MEM_START_ADDR           :0x%08x\n", (unsigned int)__G_MM_MP_MEM_START_ADDR);

    printk("__G_MM_IMAGE_DECODER_MEM_LEN       :0x%08x\n", (unsigned int)__G_MM_IMAGE_DECODER_MEM_LEN);
    printk("__G_MM_IMAGE_DECODER_MEM_START_ADDR:0x%08x\n", (unsigned int)__G_MM_IMAGE_DECODER_MEM_START_ADDR);
	
    printk("__G_MM_PRIVATE_AREA_TOP_ADDR       :0x%08x\n", (unsigned int)__G_MM_PRIVATE_AREA_TOP_ADDR);
    printk("__G_MM_PRIVATE_AREA_START_ADDR     :0x%08x\n", (unsigned int)__G_MM_PRIVATE_AREA_START_ADDR);
    printk("__G_MM_SHARED_MEM_TOP_ADDR         :0x%08x\n", (unsigned int)__G_MM_SHARED_MEM_TOP_ADDR);
    printk("__G_MM_SHARED_MEM_START_ADDR       :0x%08x\n", (unsigned int)__G_MM_SHARED_MEM_START_ADDR);
    printk("__G_RPC_MM_LEN                     :0x%08x\n", (unsigned int)__G_RPC_MM_LEN);
#if 0
    printk("__G_MM_OSD_BK_ADDR                 :0x%08x\n", (unsigned int)__G_MM_OSD_BK_ADDR);
    printk("__G_MM_TTX_PB_START_ADDR           :0x%08x\n", (unsigned int)__G_MM_TTX_PB_START_ADDR);
    printk("__G_MM_TTX_SUB_PAGE_BUF_ADDR       :0x%08x\n", (unsigned int)__G_MM_TTX_SUB_PAGE_BUF_ADDR);
    printk("__G_MM_TTX_P26_NATION_BUF_ADDR     :0x%08x\n", (unsigned int)__G_MM_TTX_P26_NATION_BUF_ADDR);
    printk("__G_MM_TTX_P26_DATA_BUF_ADDR       :0x%08x\n", (unsigned int)__G_MM_TTX_P26_DATA_BUF_ADDR);
    printk("__G_MM_SUB_BS_START_ADDR           :0x%08x\n", (unsigned int)__G_MM_SUB_BS_START_ADDR);
    printk("__G_MM_SUB_HW_DATA_ADDR            :0x%08x\n", (unsigned int)__G_MM_SUB_HW_DATA_ADDR);
    printk("__G_MM_SUB_PB_START_ADDR           :0x%08x\n", (unsigned int)__G_MM_SUB_PB_START_ADDR);
#endif

    printk("__G_MM_BOOTLOGO_DATA_START_ADDR    :0x%08x\n", (unsigned int)__G_MM_BOOTLOGO_DATA_START_ADDR);

#if 0	
    printk("__G_MM_SGDMA_MEM_END               :0x%08x\n", (unsigned int)__G_MM_SGDMA_MEM_END);
    printk("__G_MM_SGDMA_MEM_START             :0x%08x\n", (unsigned int)__G_MM_SGDMA_MEM_START);
#endif

    printk("__G_MM_DMX_MEM_TOP_ADDR            :0x%08x\n", (unsigned int)__G_MM_DMX_MEM_TOP_ADDR);
    printk("__G_MM_DMX_MEM_START_ADDR          :0x%08x\n", (unsigned int)__G_MM_DMX_MEM_START_ADDR);
    printk("__G_MM_TSG_BUF_LEN                 :0x%08x\n", (unsigned int)__G_MM_TSG_BUF_LEN);
    printk("__G_MM_TSG_BUF_START_ADDR          :0x%08x\n", (unsigned int)__G_MM_TSG_BUF_START_ADDR);
    printk("__G_GE_CMD_SIZE                    :0x%08x\n", (unsigned int)__G_GE_CMD_SIZE);
    printk("__G_MM_FB_START_ADDR               :0x%08x\n", (unsigned int)__G_MM_FB_START_ADDR);

#if 1	
    printk("__G_MM_NIM_J83B_MEM_LEN            :0x%08x\n", (unsigned int)__G_MM_NIM_J83B_MEM_LEN);
    printk("__G_MM_NIM_J83B_MEM_START_ADDR     :0x%08x\n", (unsigned int)__G_MM_NIM_J83B_MEM_START_ADDR);
#endif

    printk("===================MEMORY MAP END===================\n");
	
	return;
}
#endif

void __init plat_mem_setup(void)
{
	m36_init_gpio();
	
	/* board specific setup: PIN MUX ... */
	board_setup();

	/* _machine_restart, _machine_halt, pm_power_off 
	    associated with  arch/mips/kernel/reset.c */
	_machine_restart = prom_restart;
	_machine_halt = prom_halt;
	pm_power_off = prom_halt;

	coherentio = 0;	/* no DMA cache coherency (may be set by user) */
    hw_coherentio = 0;	/* init to 0 => no HW DMA cache coherency (reflects real HW) */
#if 0
	ali_show_sys_memmap();
#endif
	/* IO/MEM resources. */
}
