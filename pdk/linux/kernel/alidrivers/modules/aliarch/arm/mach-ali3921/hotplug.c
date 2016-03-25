/*
 * Copyright (C) STMicroelectronics 2009
 * Copyright (C) ST-Ericsson SA 2010
 *
 * License Terms: GNU General Public License v2
 *	Based on ARM realview platform
 *
 * Author: Sundar Iyer <sundar.iyer@stericsson.com>
 *
 */
#include <linux/version.h> 
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/smp.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
#include <asm/cacheflush.h>
#endif
#include <asm/smp_plat.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
extern volatile int pen_release;
#endif
static inline void platform_do_lowpower(unsigned int cpu)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))	
	flush_cache_all();
#endif
	/* we put the platform to just WFI */
	for (;;) {
		__asm__ __volatile__("dsb\n\t" "wfi\n\t"
				: : : "memory");
		if (pen_release == cpu_logical_map(cpu)) {
			/*
			 * OK, proper wakeup, we're done
			 */
			break;
		}
	}
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)) 
int platform_cpu_kill(unsigned int cpu)
{
	return 1;
}
#endif

/*
 * platform-specific code to shutdown a CPU
 *
 * Called with IRQs disabled
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)) 
void platform_cpu_die(unsigned int cpu)
#else
void __ref ali_cpu_die(unsigned int cpu)
#endif
{
	/* directly enter low power state, skipping secure registers */
	platform_do_lowpower(cpu);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)) 
int platform_cpu_disable(unsigned int cpu)
{
	/*
	 * we don't allow CPU 0 to be shutdown (it is still too special
	 * e.g. clock tick interrupts)
	 */
	return cpu == 0 ? -EPERM : 0;
}
#endif