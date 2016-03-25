/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_pm.c
 *  (I)
 *  Description: ALi power management implementation
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2011.03.07				Owen			Creation
 ****************************************************************************/

#ifndef __INCLUDE_KERNEL_ALI_PM_H____
#define __INCLUDE_KERNEL_ALI_PM_H____

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
#include <ali_pm_common.h>
#include <linux/version.h>

//#include <asm/system.h>
#include <asm/processor.h>
#include <asm/cacheflush.h>
#include <asm/gpio.h>

#include <asm/mach-ali/sleep.h>

#define ALI_SUSPEND_DBG
#ifdef ALI_SUSPEND_DBG
#define ALI_SUSPEND_LOG(fmt, args...) \
	do { \
		printk("ALI POWER MNGT: %s "fmt, __func__, ##args); \
	} while (0)
#else
#define ALI_SUSPEND_LOG(...)
#endif

struct pm_private
{
    unsigned long reserved;
};

struct pm_device
{
    /* Common */
    char   *dev_name;

	/* Hardware privative structure */
	struct pm_private *priv;	
    struct device *pm_device_node;
    struct cdev pm_cdev;
    struct class *pm_class;
    dev_t dev_no;
};

#endif
