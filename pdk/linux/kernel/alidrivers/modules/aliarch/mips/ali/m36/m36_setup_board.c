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

#include <ali_board_config.h>
static bool is_nand_boot = false;

extern void ali_hdmi_set_module_init_hdcp_onoff(bool bOnOff);

void __init board_setup(void)
{	
	//config hdmi hdcp on/off
#ifdef CONFIG_HDMI_ALI	
	ali_hdmi_set_module_init_hdcp_onoff(g_hdmi_hdcp_enable);
#endif
}

static int __init set_nand_boot(char *str)
{
        is_nand_boot = true;
        return 1;
}

__setup("nandboot", set_nand_boot);


bool board_is_nand_boot(void)
{
    return is_nand_boot;
}

