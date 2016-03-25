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
 
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/delay.h>
//#include <linux/mc146818rtc.h>

#include <asm/mipsregs.h>
#include <asm/mipsmtregs.h>
#include <asm/hardirq.h>
//#include <asm/i8253.h>
#include <asm/irq.h>
#include <asm/div64.h>
#include <asm/cpu.h>
#include <asm/time.h>

#include <asm/mach-ali/prom.h>
#include <asm/mach-ali/m36_irq.h>
#include <ali_soc_common.h>

/*
 * Estimate CPU frequency.  Sets mips_hpt_frequency as a side-effect
 */
static unsigned int __init estimate_cpu_frequency(void)
{
	unsigned int prid = read_c0_prid() & 0xffff00;
	unsigned int count;

#if 1
	count = US_TICKS * 1 * 1000 * 1000;
#else
	#define RD_RTC_ADDRESS		0xB8018A04
	#define RTC_SEC_MASK		0x0000003F		// 5 bits for sec
	unsigned long flags;
	unsigned int start/*, end*/;
	unsigned char sec;
    
	local_irq_save(flags);
	sec = (*((volatile unsigned char *)(RD_RTC_ADDRESS)) & RTC_SEC_MASK);

	// printk("current sec: %d\n", sec);
	/* Start counter exactly on RTC_MS = 0 */
	while ((*((volatile unsigned char *)(RD_RTC_ADDRESS)) & RTC_SEC_MASK) != sec + 1);

	/* Start r4k counter. */
	start = read_c0_count();

	while ((*((volatile unsigned char *)(RD_RTC_ADDRESS)) & RTC_SEC_MASK) != sec + 2);

	count = read_c0_count() - start;
	// end = read_c0_count();
	// printk("start at count: %d, end at count: %d\n", start, end);

	/* restore interrupts */
	local_irq_restore(flags);
    
#endif

	mips_hpt_frequency = count;
	if ((prid != (PRID_COMP_MIPS | PRID_IMP_20KC)) &&
	    (prid != (PRID_COMP_MIPS | PRID_IMP_25KF)))
		count *= 2;

	count += 5000;    /* round */
	count -= count%10000;

	return count;
}

void __init plat_time_init(void)
{
	unsigned int est_freq;

	est_freq = estimate_cpu_frequency();	/* est_freq = 396000000; */
	
	printk("CPU frequency %d.%02d MHz\n", est_freq/1000000,
	       (est_freq%1000000)*100/1000000);

	/* plat_perf_setup(est_freq); */
}


