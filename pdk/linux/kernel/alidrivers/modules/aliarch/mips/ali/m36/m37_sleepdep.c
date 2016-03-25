/*
 * sleep dependency.c
 * This file contains all the sleep hardware dependencies implementation.
 *
 */

#include "sleepdep.h"
#include "m37_sleep.h"

// #define PM_IOMAP_USED

/*
 * Variable to let the idle thread to pass
 * its action, this is just a work around
 */
static int ali_pm_noirq_opt = 0;

/**
 * UART
 */
#define DISABLE_UART_IRQ

/**
 * Back up the register value
 * It's from M3603, need to modify
 * according to the official standby
 * flow of C3701C
 */
typedef struct _pm_reg {
    unsigned int vdac_pd_b8008084;
    unsigned int adac_pd_b80020f8;
    unsigned int usb_pd_b803d810;
    /* unsigned int usb_pd_b80324f0; */
    unsigned int hdmi_pd_b800006c;
    unsigned int sata_pd_b80000a0;
    unsigned int rxadc_pd_b8028420;
}pm_reg_t;

static pm_reg_t __pm_reg_value;

/**
 * Prerequisite of standby
 */
static inline void operate_vdac(int power_down)
{
    if (power_down)
    {
        __pm_reg_value.vdac_pd_b8008084 = *(unsigned int *)0xb8008084;
        *(unsigned int *)0xb8008084 |= 0x00000f00;
    }
    else
        *(unsigned int *)0xb8008084 = __pm_reg_value.vdac_pd_b8008084;
}

static inline void operate_adac(int power_down)
{
    if (power_down)
    {
        __pm_reg_value.adac_pd_b80020f8 = *(unsigned int *)0xb80020f8;
        *(unsigned int *)0xb80020f8 |= 0x00007fd0;
    }
    else
        *(unsigned int *)0xb80020f8 = __pm_reg_value.adac_pd_b80020f8;
}

static inline void operate_usb(int power_down)
{
	static unsigned int val_usb_resetj = 0;
    if (power_down)
    {
        __pm_reg_value.usb_pd_b803d810 = *(unsigned int *)0xb803d810;
		val_usb_resetj = *(unsigned int *)0xb8000080;
        *(unsigned int *)0xb803d810 |= 0x00018000;
		// *(unsigned int *)0xb8000080 |= 0x10000000;
    }
    else
    {
        *(unsigned int *)0xb803d810 = __pm_reg_value.usb_pd_b803d810;
		// *(unsigned int *)0xb8000080 = val_usb_resetj;
    }
}

static inline void operate_hdmi(int power_down)
{
    if (power_down)
    {
        __pm_reg_value.hdmi_pd_b800006c = *(unsigned int *)0xb800006c;
        *(unsigned int *)0xb800006c |= 0x00070000;
    }
    else
        *(unsigned int *)0xb800006c = __pm_reg_value.hdmi_pd_b800006c;
}

static inline void operate_sata(int power_down)
{
    if (power_down)
    {
        __pm_reg_value.sata_pd_b80000a0 = *(unsigned int *)0xb80000a0;
        *(unsigned int *)0xb80000a0 |= 0x10000000;
    }
    else
        *(unsigned int *)0xb80000a0 = __pm_reg_value.sata_pd_b80000a0;
}

	static inline void operate_rxadc(int power_down)
	{
	    if (power_down)
	    {
	        __pm_reg_value.rxadc_pd_b8028420 = *(unsigned int *)0xb8028420;
	         *(unsigned int *)0xb8028420 |= 0xf00;
	    }
	    else
	        *(unsigned int *)0xb8028420 = __pm_reg_value.rxadc_pd_b8028420;
	}
inline void pm_standby_prepare(int enter)
{

    operate_vdac(enter);
    operate_adac(enter);
    operate_usb(enter);
    operate_hdmi(enter);
   // operate_sata(enter);
    operate_rxadc(enter);

}

/* 
 * We have to split device clock gating and
 * IRQ disable steps due to UART have some
 * bugs and we want some information from
 * UART before we enter cache
 */

static inline void operate_uart(int enable)
{
#ifdef PM_IOMAP_USED
    void __iomem *p = (void __iomem *)ioremap(IO_BASE_ADDR + 0x30, 16);
#else
    void *p = (void *)0xb8000030;
#endif
    int i = 100;
    
    if (!enable)
#ifdef PM_IOMAP_USED
        writel(readl(p + 8) & ~0x00010000, p + 8);
#else
        *(unsigned int *)(p + 8) &= ~0x00010000;
#endif
    else
#ifdef PM_IOMAP_USED
        writel(readl(p + 8) | 0x00010000, p + 8);
#else
        *(unsigned int *)(p + 8) |= 0x00010000;
#endif

#ifdef PM_IOMAP_USED
    iounmap(p);
#endif

    while (i-- > 0) ;
}

inline void operate_device(int enable)
{
#ifdef DISABLE_UART_IRQ
    unsigned int val60 = 0xeffffff2;
#else
    unsigned int val60 = 0xeff8fff2;
#endif
    /* DSC clock remain for SHA
       Bit 16: VIDEO MEM CLOCK, 
       we can't disable it currently, I don't know why */
    unsigned int val64 = 0x0fffffff;
#ifdef PM_IOMAP_USED
    void __iomem *p = NULL;
#else
    void *p = NULL;
#endif
    int i = 100;
    
    if (enable)
        val60 = val64 = 0x0;

#ifdef PM_IOMAP_USED
    p = (void __iomem *)ioremap(IO_BASE_ADDR + 0x60, 8);
    writel(val60, p);
    writel(val64, p + 4);
#else
    p = (void *)0xb8000060;
    *(unsigned int *)p = val60;
    *(unsigned int *)(p + 4) = val64;
#endif    

#ifdef PM_IOMAP_USED
    iounmap(p);
#endif

    while (i-- > 0) ;
    
    return;
}

/*
 * Set the sleep timeout value
 */
inline void operate_time_count(unsigned long timeout)
{
    unsigned long *p = (unsigned long *)PM_SLEEP_TIMEOUT;
    *p = timeout;
    p = (unsigned long *)PM_SLEEP_TIMEOUT_CONST;
    *p = timeout;
}

/*
 * Disable irq: val_3* & irq_disable_mask_3*
 * Enable irq: restore the old value
 *     @ Just enable the UART / IR interrupt
 * Device should know to free the irq they required
 * But as a confirmation, we disable the unneeded
 */
static unsigned long irq_disable_mask_38 = 0x00000000;
static unsigned long irq_disable_mask_3c = 0x00100000;

static inline void operate_extern_irqs(int enable)
{
    static unsigned long irq_flags_38 = 0;
    static unsigned long irq_flags_3c = 0;
    void __iomem *p = (void __iomem *)ioremap(IO_BASE_ADDR + 0x30, 16);
    int i = 100;
    
    if (!enable)
    {
        irq_flags_38 = readl(p + 8);
        irq_flags_3c = readl(p + 12);
        writel(readl(p + 8) & irq_disable_mask_38, p + 8);
        writel((readl(p + 12) & irq_disable_mask_3c) | irq_disable_mask_3c, p + 12);
    }
    else
    {
        if (0 == irq_flags_38 || 0 == irq_flags_3c)
        {
            printk(KERN_ERR "Err: no restore irq value.\n");
            return;
        }
        writel(irq_flags_38, p + 8);
        writel(irq_flags_3c, p + 12);
        irq_flags_38 = 0;
        irq_flags_3c = 0;
    }

    while (i-- > 0) ;
    
    iounmap(p);
    return;
}

static inline void operate_nonboot_cpu(int standby)
{
#ifdef CONFIG_DVB_ALI_M3602_DMX
    extern unsigned long sed_standby(unsigned long status);
#endif
#ifdef CONFIG_ALI_AS
	SET_AS_FLAG_TO_SEE;
#endif
#ifdef CONFIG_DVB_ALI_M3602_DMX
    sed_standby(standby);
#endif
    if (standby)
        mdelay(100);
}

noinline void arch_suspend_disable_irqs(void)
{
    ali_pm_noirq_opt = 1;    
    operate_nonboot_cpu(1);
    preempt_disable();
    operate_extern_irqs(0);
    operate_uart(0);
    local_irq_disable();
    return;
}

noinline void arch_suspend_enable_irqs(void)
{
    local_irq_enable();
    operate_uart(1);
    operate_extern_irqs(1);
    preempt_enable();
    //operate_nonboot_cpu(0);    
    ali_pm_noirq_opt = 0;
    return;
}

