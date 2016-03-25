/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation. 2010 Copyright (C)
 *  (C)
 *  File: init.c
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
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/bootmem.h>
#include <linux/string.h>
#include <linux/kernel.h>

#include <asm/addrspace.h>
#include <asm/bootinfo.h>

#include <asm/mach-ali/prom.h>
#include "board_config.h"
#include <ali_board_config.h>
#include <linux/memblock.h>

#define KB 1024
#define MB (1024 * KB)
#define ALI_OVG_MEM		"ovg_mem="

int prom_argc;
char **prom_argv;
char **prom_envp;

const char *get_system_type(void)
{
	return "ALi M36";
}

char *prom_getenv(char *envname)
{
	/*
	 * Return a pointer to the given environment variable.
	 * In 64-bit mode: we're using 64-bit pointers, but all pointers
	 * in the PROM structures are only 32-bit, so we need some
	 * workarounds, if we are running in 64-bit mode.
	 */

/*
	int i, index=0;

	i = strlen(envname);

	while (prom_envp(index)) {
		if(strncmp(envname, prom_envp(index), i) == 0) {
			return(prom_envp(index+1));
		}
		index += 2;
	}
*/

	return NULL;
}



#if 0
extern unsigned long __G_MM_OVG_MEM_LEN;
extern unsigned long __G_MM_OVG_MEM_START_ADDR;


int set_ali_mem_map(char *cmdline)
{
	char *ptr = NULL;

	if (cmdline)
	{
		/* Check the command line for ovg size */		
		ptr = strstr(cmdline, ALI_OVG_MEM);	
		if (ptr) 
		{
			__G_MM_OVG_MEM_LEN = memparse(ptr + strlen(ALI_OVG_MEM), &ptr);
			
		}	
	}	
	return 0;
}
#endif

extern void kernel_fromboot(int boot);



void __init prom_init(void)
{
	unsigned char *memsize_str;
	//unsigned long memsize;
	//int i = 0;
	//Disable mbx interrupt firstly
	*((unsigned long *) 0xb804003c) &= 0x0fffffff;
	//*((volatile unsigned long *)0xb8000074) = (1<<30)|(1<<18);		// set starp pin to nand falsh
	//*(unsigned long *) 0xb8018500 = (0xFFFFFFFF-0x00200000);        // watchdog count : total= 2^32*clk_div*(1/27)ns= 5084s (clk_div=32)
	//*(unsigned char *) 0xb8018504 = 0x67 ;                //enable watch dog bit[0:1] clk_div 11 mem_clk/256 bit[2]:1 count start bit[5]: timer enable bit[6] watch dog enable
	//printk("watch dog enable \n");
	   
#if (defined(CONFIG_ALI_M3701G) || defined(CONFIG_ALI_M3701C))
	*((volatile unsigned long *)(0xB8000110)) |= 0x01;
#endif
	*((volatile unsigned long *)(0xB8000220)) |= 0x01000000;
/*Ehanced by tony on 2014-08-25*/
	//if (fw_arg0 >= CKSEG0 || fw_arg1 < CKSEG0 || fw_arg2 != 0 ||fw_arg3 < CKSEG0) {
	if (fw_arg0 >= CKSEG0 || fw_arg1 < CKSEG0 || fw_arg3!= 0 ||fw_arg2 < CKSEG0) {
		/* fw_arg0 is not a valid number, or fw_arg1 is not a valid pointer */
		prom_argc = 0;
		prom_argv = prom_envp = NULL;
		printk("CKSEG0 0x%08x fw_arg0 0x%lx fw_arg1 0x%lx fw_arg2 0x%lx fw_arg3 0x%lx\n",CKSEG0,fw_arg0,fw_arg1,fw_arg2,fw_arg3);
	}else {
		prom_argc = (int) fw_arg0;
		prom_argv = (char **) fw_arg1;
		prom_envp = (char **) fw_arg2;
	}


	mips_machtype = MACH_ALI_M36;

#if 1
	prom_init_cmdline();
	memsize_str = prom_getenv("memsize");
#endif	

	#if 0
	for (i=0; i<prom_argc; i++)
	{				
		set_ali_mem_map(prom_argv[i]);		
	}
	#endif
	printk("%s,%d\n", __FUNCTION__, __LINE__);
	customize_board_setting();
	//ali_reserve_mem();	

	#if 0
	if (1){//!memsize_str) {
        extern unsigned long __G_MM_MAIN_MEM[][2];
        extern unsigned long __G_MM_MAIN_MEM_NUM;
		unsigned long start = 0;
		unsigned long end = 0;
		int i  = 0;
	
		for(i = 0;i < __G_MM_MAIN_MEM_NUM;i++)
		{
            start = __G_MM_MAIN_MEM[i][0] - 0xA0000000;
		    end = __G_MM_MAIN_MEM[i][1] - 0xA0000000;
				
		    //printk("%s,%d,start:%x,end:%x\n", __FUNCTION__, __LINE__, (unsigned int)start, (unsigned int)end);
			
			add_memory_region(start, end - start, BOOT_MEM_RAM);	
		}
	} else {
		memsize = simple_strtol(memsize_str, NULL, 0);
		add_memory_region(0, memsize, BOOT_MEM_RAM);		
	}
	#endif

	//System configuration
/*****************************************************
			UART FIFO work mode
******************************************************/	
	*((volatile unsigned char *)(0xB8018303)) = 0x83;
	*((volatile unsigned char *)(0xB8018300)) = 0x01;	  	
	*((volatile unsigned char *)(0xB8018301)) = 0x00;	  		
	*((volatile unsigned char *)(0xB8018303)) &= 0x7F;	
	*((volatile unsigned char *)(0xB8018305)) = 0x00;		//Reset Line Status
	*((volatile unsigned char *)(0xB8018302)) = 0x47;	
	*((volatile unsigned char *)(0xB8018304)) = 0x03;	
}

