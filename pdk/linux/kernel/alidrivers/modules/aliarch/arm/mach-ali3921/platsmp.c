/*
 * Copyright (C) 2002 ARM Ltd.
 * Copyright (C) 2008 STMicroelctronics.
 * Copyright (C) 2009 ST-Ericsson.
 * Author: Srinidhi Kasagar <srinidhi.kasagar@stericsson.com>
 *
 * This file is based on arm realview platform
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/version.h>  
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/smp.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#else
#include <asm/hardware/gic.h>
#endif
#include <asm/smp_plat.h>
#include <asm/smp_scu.h>
#include <mach/hardware.h>
#include <mach/ali-s3921.h>
//#include <mach/setup.h>
#include <ali_reg.h>

#define SCU_CPU_STATES_REG      (unsigned*)(A9_MPCORE_SCU + 0x08)
#define SCU_CONTROL_REG         (unsigned*)(A9_MPCORE_SCU + 0x00)
#define SCU_INVALIDATE_ALL_REG  (unsigned*)(A9_MPCORE_SCU + 0x0C)
#define SCU_EN                  0x1 // Enable the SCU

/* This is called from headsmp.S to wakeup the secondary core */
extern void my_secondary_startup(void);

/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen"
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)) 
volatile int pen_release = -1;
#else
extern volatile int pen_release;// = -1;
#endif
volatile unsigned int secondary_cpu_boot_addr = 0;

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
	__cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
	outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}

static void __iomem *scu_base_addr(void)
{
    return (void *)A9_MPCORE_SCU;
}

static DEFINE_SPINLOCK(boot_lock);

/*Added by tony*/

/* Get the number of powered on CPUs in MPCore */
int ali_get_number_of_cpu(void) {
  unsigned cpu_status;
  int num_cpu = 0;
  int i;
  volatile unsigned int *cpu_status_ptr = SCU_CPU_STATES_REG;
  cpu_status = *cpu_status_ptr;
  for(i = 0; i <4; i++)
  	num_cpu += !((cpu_status >> i * 8) & 0x03);
  return num_cpu;
}


void ali_scu_enable(int set) {
  /* Turn on or off the SCU */
  volatile unsigned int* ctl_reg = SCU_CONTROL_REG;
  volatile unsigned int* inv_reg = SCU_INVALIDATE_ALL_REG;
  if (set) {
    /* Invalidates all the tags rams */
    *inv_reg = 0xffff;

    /* Turn on the SCU */
    *ctl_reg |= SCU_EN;
  }

  else *ctl_reg &= ~SCU_EN;
}

/*Added end*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
void __cpuinit platform_secondary_init(unsigned int cpu)
#else
static void s3921_secondary_init(unsigned int cpu)
#endif
{

	/*
	 * if any interrupts are already enabled for the primary
	 * core (e.g. timer irq), then they will not have been enabled
	 * for us: do so
	 */
	printk("enter platform_secondary_init\n");
//	while(true);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#else
	gic_secondary_init(0);
#endif
	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);

}
#define SEE_CFG_ADDR    0x18000220
#define CPU1_KEEP_RESET    4


#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
#else
static int s3921_boot_secondary(unsigned int cpu, struct task_struct *idle)
#endif
{
	unsigned long timeout;
	/*
	 * set synchronisation state between this boot processor
	 * and the secondary one
	 */
	*(volatile unsigned int *)(ioremap(SEE_CFG_ADDR, sizeof(unsigned int)))= (~(1<<CPU1_KEEP_RESET));

	printk("boot_secondary entered, cores:%d\n",ali_get_number_of_cpu());
	spin_lock(&boot_lock);
	printk("boot_secondary##\n");
//	while(1);

	/*
	 * The secondary processor is waiting to be released from
	 * the holding pen - release it, then wait for it to flag
	 * that it has been released by resetting pen_release.
	 */
	write_pen_release(cpu_logical_map(cpu));

    printk("send softirq:%x\n",secondary_cpu_boot_addr);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
	arch_send_wakeup_ipi_mask(cpumask_of(cpu));	
#else	
	gic_raise_softirq(cpumask_of(cpu), 1);
	//gic_raise_softirq(cpumask_of(cpu), 8);
	//dsb_sev();
#endif	

	timeout = jiffies + (1 * HZ);
	timeout *= 100000;
	while (time_before(jiffies, timeout)) {
		if (pen_release == -1)
			break;
		udelay(10);
	}
    printk("boot over:%x %x\n",timeout, pen_release);

//	while(1);

/*	if(pen_release == -1)
	{
		while(true);
	}
*/
	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)) 
void __init smp_init_cpus(void) 
#else
static void __init s3921_smp_init_cpus(void)
#endif
{
	void __iomem *scu_base = scu_base_addr();
	unsigned int i, ncores;

	/*Enhance by tony on 2014/07/25*/
	//ncores = scu_base ? scu_get_core_count(scu_base) : 1;
	ncores = scu_get_core_count(scu_base);

	/* sanity check */
	if (ncores > nr_cpu_ids) {
		pr_warn("SMP: %u cores greater than maximum (%u), clipping\n",
			ncores, nr_cpu_ids);
		ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#else
	set_smp_cross_call(gic_raise_softirq);
#endif
}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
void __init platform_smp_prepare_cpus(unsigned int max_cpus)
#else
static void __init s3921_platform_smp_prepare_cpus(unsigned int max_cpus)
#endif
{
	//scu_enable(scu_base_addr());
    //secondary_cpu_boot_addr = virt_to_phys(my_secondary_startup);
    secondary_cpu_boot_addr = virt_to_phys(my_secondary_startup);
    printk("platform_smp_prepare_cpus:%x %x,scureg:0x%x, 0x18000054_V:0x%x\n",my_secondary_startup,secondary_cpu_boot_addr,*(volatile unsigned int*)A9_MPCORE_SCU,__REGALIRAW(0x18000054));
	//*(unsigned int*)0xfefff054 =  secondary_cpu_boot_addr;
	*(unsigned int*)__REGALIRAW(0x18000054) =  secondary_cpu_boot_addr;
	//*(unsigned int*)(0xc00000f0) =  secondary_cpu_boot_addr; //tony debug
	
	/*Reset the second core*/
	//__REG32ALI(0x18000250) |= 0x40;
	
	scu_enable(scu_base_addr());
	printk("scu enabled, scureg:0x%x\n", *(volatile unsigned int*)A9_MPCORE_SCU);
	/* make sure write buffer is drained */
	mb();
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
extern void ali_cpu_die(unsigned int cpu);
struct smp_operations s3921_smp_ops __initdata = {
	.smp_init_cpus		= s3921_smp_init_cpus,
	.smp_prepare_cpus	= s3921_platform_smp_prepare_cpus,
	.smp_secondary_init	= s3921_secondary_init,
	.smp_boot_secondary	= s3921_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	 .cpu_die		= ali_cpu_die,
#endif
};
#endif
