/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_suspend.h
 *  (I)
 *  Description: ALi power management implementation
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2011.03.07				Owen			Creation
 ****************************************************************************/
 
#ifndef __INCLUDE_KERNEL_ALI_SUSPEND_H____
#define __INCLUDE_KERNEL_ALI_SUSPEND_H____

// #include "ali_pm.h"

#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
//#include <linux/smp_lock.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/crc32.h>
#include <linux/ioport.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/compiler.h>
#include <linux/version.h>

#include <ali_pm_common.h>

#include <asm/system.h>
#include <asm/processor.h>
#include <asm/cacheflush.h>
#include <asm/gpio.h>

#include <asm/mach-ali/sleep.h>

extern void ali_suspend_register_ops(void);
extern void ali_suspend_set_resume_key(pm_key_t *pm_key);
extern void ali_suspend_set_standby_param(pm_param_t *p_standby_param);

#endif
