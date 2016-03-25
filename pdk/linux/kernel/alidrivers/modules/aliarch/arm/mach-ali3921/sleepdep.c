/*
 * sleep dependency.c 
 * This file contains all the sleep hardware dependencies implementation.
 *
 */
#include <linux/version.h> 
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/slab.h>

#include <asm/system_misc.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <asm/smp_scu.h>
#include <asm/pgalloc.h>
#include <asm/suspend.h>
#include <asm/hardware/cache-l2x0.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#else
#include <asm/hardware/gic.h>
#endif
#include "sleepdep.h"
#include "m3921_sleep.h"
#include "ali_standby_bin.h"
#include <ali_reg.h>
#include<asm/cacheflush.h>



int ali_3921_enter_lowpower(unsigned int cpu, unsigned int power_state);
	
// #define PM_IOMAP_USED

/*
 * Variable to let the idle thread to pass
 * its action, this is just a work around
 */
static int ali_pm_noirq_opt = 0;

/*
 * UART
 */
#define DISABLE_UART_IRQ

/**
 * Back up the register value
 */
typedef struct _pm_reg {
    unsigned int vdac_pd_b8008084;
    unsigned int adac_pd_b80020d4;
    unsigned int adac_pd_b80020d8;
    unsigned int usb_pd_b803d810;
    unsigned int hdmi_pd_b800006c;
    unsigned int rxadc_pd_b8028421;
    unsigned int pmu_pad_pd_b805c0c0;
}pm_reg_t;

static pm_reg_t __pm_reg_value;


static inline void output_char(unsigned char c)
{
	while((__REG8ALI(0x18018305)&0x40) == 0x00);	//wait empty
	__REG8ALI(0x18018300) = c;
}

/**
 * Prerequisite of standby
 */
static inline void operate_vdac(int power_down)
{
    if (power_down)
    {
        __pm_reg_value.vdac_pd_b8008084 = __REG32ALI(0x18008084);
        __REG32ALI(0x18008084) |= 0x00000f00;
    }
    else
        *(unsigned int *)0xb8008084 = __pm_reg_value.vdac_pd_b8008084;
}

static inline void operate_adac(int power_down)
{
    if (power_down)
    {
        __pm_reg_value.adac_pd_b80020d4 = __REG32ALI(0x180020d4);
        __REG32ALI(0x180020d4) = 0x6f30ffc1; //|= 0x00073f0d;
        __pm_reg_value.adac_pd_b80020d8 = __REG32ALI(0x180020d8);
        __REG32ALI(0x180020d8) = 0x00000a00; //|= 0x00073f0d;
    }
    else
    {
        __REG32ALI(0x180020d4) = __pm_reg_value.adac_pd_b80020d4;
        __REG32ALI(0x180020d8) = __pm_reg_value.adac_pd_b80020d8;
    }
}

static inline void operate_usb(int power_down)
{
	if (power_down)
	{
		__pm_reg_value.usb_pd_b803d810 = __REG32ALI(0x1803D810);
		__REG32ALI(0x1803D810) |= (1<<16)|(1<<15);
		__REG32ALI(0x18000080) |= (1<<28);	// usb reset
	}
	else
	{
		__REG32ALI(0x18000080) &= ~(1<<28);	// usb reset
		mdelay(10);
		__REG32ALI(0x1803D810) = __pm_reg_value.usb_pd_b803d810;
		mdelay(10); 
	}
}

static inline void operate_hdmi(int power_down)
{
    if (power_down)
    {
        __pm_reg_value.hdmi_pd_b800006c = __REG32ALI(0x1800006c);
        __REG32ALI(0x1800006c) |= (1<<16)|(1<<17)|(1<<18);	// bit16-18 hdmi phy
    }
    else
        __REG32ALI(0x1800006c) = __pm_reg_value.hdmi_pd_b800006c;
}

static inline void operate_rxadc(int power_down)
{
    if (power_down)
    {
        __pm_reg_value.rxadc_pd_b8028421 = __REG32ALI(0x18028421);
        __REG32ALI(0x18028421) |= (1<<1)|(1<<2)|(1<<3);	// 
    }
    else
        __REG32ALI(0x18028421) = __pm_reg_value.rxadc_pd_b8028421;
}

static inline void operate_pmu(int power_down)
{
    if (power_down)
    {
        __pm_reg_value.pmu_pad_pd_b805c0c0 = __REG32ALI(0x1805c0c0);
        __REG32ALI(0x1805c0c0) &= ~(7<<0);	// BIT[2:0] set to 3'b000 to reduce pad driving
    }
    else
        __REG32ALI(0x1805c0c0) = __pm_reg_value.pmu_pad_pd_b805c0c0;
}

 void pm_standby_prepare(int enter)
{
//	operate_vdac(enter);
//	operate_adac(enter);
//	operate_usb(enter);
//	operate_hdmi(enter);
//	operate_rxadc(enter); 
//	operate_pmu(enter);	// pmu register sometime is B805C050:1FFFFFFF1FFFFFFF0000000000000000
}

/* 
 * We have to split device clock gating and
 * IRQ disable steps due to UART have some
 * bugs and we want some information from
 * UART before we enter cache
 */

static inline void operate_uart(int enable)
{
/*

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
*/    
}

void operate_device(int enable)
{
/*
#ifdef DISABLE_UART_IRQ
    unsigned int val60 = 0x6dfffff0;
#else
    unsigned int val60 = 0x6df8fff0;
#endif
    // Bit 16: VIDEO MEM CLOCK, 
    //   we can't disable it currently, I don't know why 
    unsigned int val64 = 0x1ffeffff;
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
    */
    return;
}

void standby_in_pmu_ram(void);
	
int ali_3921_pm_suspend(void)
{
//	struct power_state *pwrst;
//	int state, ret = 0;
	u32 cpu_id = smp_processor_id();


	/*
	 * For MPUSS to hit power domain retention(CSWR or OSWR),
	 * CPU0 and CPU1 power domains need to be in OFF or DORMANT state,
	 * since CPU power domain CSWR is not supported by hardware
	 * Only master CPU follows suspend path. All other CPUs follow
	 * CPU hotplug path in system wide suspend. On OMAP4, CPU power
	 * domain CSWR is not supported by hardware.
	 * More details can be found in OMAP4430 TRM section 4.3.4.2.
	 */
	ali_3921_enter_lowpower(cpu_id, PWRDM_POWER_OFF);


	return 0;
}


void ali_s3921_timer_enable(void);
/**
 * omap4_enter_lowpower: OMAP4 MPUSS Low Power Entry Function
 * The purpose of this function is to manage low power programming
 * of OMAP4 MPUSS subsystem
 * @cpu : CPU ID
 * @power_state: Low power state.
 *
 * MPUSS states for the context save:
 * save_state =
 *	0 - Nothing lost and no need to save: MPUSS INACTIVE
 *	1 - CPUx L1 and logic lost: MPUSS CSWR
 *	2 - CPUx L1 and logic lost + GIC lost: MPUSS OSWR
 *	3 - CPUx L1 and logic lost + GIC + L2 lost: DEVICE OFF
 */
int ali_3921_enter_lowpower(unsigned int cpu, unsigned int power_state)
{
	unsigned int save_state = 0;
//	unsigned int wakeup_cpu;

	unsigned long reg38_save,reg3c_save;
	unsigned long  reg60_save;
	unsigned long reg64_save;	
	unsigned long reg88_save;
	unsigned long reg90_save;
	unsigned long reg94_save;
	unsigned long reg68_save;
	unsigned long reg6c_save;
	unsigned long reg80_save;
	unsigned long reg8c_save;
	unsigned long regb0_save;
	unsigned long reg20d4_save;
	unsigned long reg20d8_save;
	unsigned long reg28421_save;
	unsigned long reg8084_save;
	unsigned long reg3d810_save;
	unsigned long reg410_save;
	unsigned long reg414_save;
	unsigned long reg418_save;
	unsigned long reg41c_save;

//	return 0;
	
	reg38_save =__REG32ALI(0x18000038) ;
	reg3c_save = __REG32ALI(0x1800003c) ;

	reg60_save = __REG32ALI(0x18000060) ;
	printk("0x18000060:%08lx \n",reg60_save);
	reg64_save = __REG32ALI(0x18000064) ;	
	printk("0x18000064:%08lx \n",reg64_save);
	reg90_save = __REG32ALI(0x18000090) ;
	printk("0x18000090:%08x \n",reg90_save);
	reg88_save = __REG32ALI(0x18000088) ;
	printk("0x18000088:%08lx \n",reg88_save);
	reg94_save = __REG32ALI(0x18000094) ;
	printk("0x18000094:%08lx \n",reg94_save);
	reg68_save = __REG32ALI(0x18000068) ;
	printk("0x18000068:%08lx \n",reg68_save);
	reg6c_save = __REG32ALI(0x1800006c) ;
	printk("0x1800006c:%08lx \n",reg6c_save);
	reg80_save = __REG32ALI(0x18000080) ;
	printk("0x18000080:%08lx \n",reg80_save);
	reg8c_save = __REG32ALI(0x1800008c) ;
	printk("0x1800008c:%08lx \n",reg8c_save);
	regb0_save = __REG32ALI(0x180000b0) ;
	printk("0x180000b0:%08lx \n",regb0_save);
	reg20d4_save = __REG32ALI(0x180020d4) ;
	printk("0x180020d4:%08lx \n",reg20d4_save);
	reg20d8_save = __REG32ALI(0x180020d8) ;
	printk("0x180020d8:%08lx \n",reg20d8_save);
	reg28421_save = __REG32ALI(0x18028421) ;
	printk("0x18028421:%08lx \n",reg28421_save);
	reg8084_save = __REG32ALI(0x18008084) ;
	printk("0x18008084:%08lx \n",reg8084_save);
	reg3d810_save = __REG32ALI(0x1803d810) ;
	printk("0x1803d810:%08lx \n",reg3d810_save);
	reg410_save = __REG32ALI(0x18000410) ;
	reg414_save = __REG32ALI(0x18000414) ;
	reg418_save = __REG32ALI(0x18000418) ;
	reg41c_save = __REG32ALI(0x1800041c) ;

		save_state = 2;

	__REG32ALI(0x180020d4) |= (7<<24);	 //  alg dac mute pattern
	__REG32ALI(0x180020d4) |= (3<<8);    // disable l/r chl clock
	__REG32ALI(0x180020d4) |= (3<<6);    // disable l/r chl data
	__REG32ALI(0x180020d4) |= (0X1F<<10); // power down dac

	__REG32ALI(0x180020d4) &= ~(7<<24);
	__REG32ALI(0x180020d4) |= (8<<24);   //DAC_ALG_REG
	

	__REG32ALI(0x180020d8) &= ~(1<<7);   // disable dig dac
	

	__REG32ALI(0x180020d8) &= ~(1<<21); // disable data input
            
        
	cpu_suspend(save_state, standby_in_pmu_ram);


	__REG32ALI(0x18006010) |= (0x01<<0);	// fb register  VHD plane
	__REG32ALI(0x18006014) |= (0x01<<0);	// fb register VSD
	
	output_char('5');
	__REG32ALI(0x18000410) = reg410_save;
	__REG32ALI(0x18000414) = reg414_save;
	__REG32ALI(0x18000418) = reg418_save;
	__REG32ALI(0x1800041c) = reg41c_save;
	
	__REG32ALI(0x18000060) =  reg60_save;
	printk("0x18000060:%08lx \n",__REG32ALI(0x18000060));
	__REG32ALI(0x18000064) = reg64_save ;	
	printk("0x18000064:%08lx \n",__REG32ALI(0x18000064));
	__REG32ALI(0x18000090) = reg90_save ;
	printk("0x18000090:%08lx \n",__REG32ALI(0x18000090));
	__REG32ALI(0x18000088) = reg88_save ;
	printk("0x18000088:%08lx \n",__REG32ALI(0x18000088));
	__REG32ALI(0x1800008c) = reg8c_save ;
	printk("0x1800008c:%08lx \n",__REG32ALI(0x1800008c));
	__REG32ALI(0x18000094) = reg94_save ;
	printk("0x18000094:%08lx \n",__REG32ALI(0x18000094));
	__REG32ALI(0x18000068) = reg68_save ;
	printk("0x18000068:%08lx \n",__REG32ALI(0x18000068));
	__REG32ALI(0x1800006c) = reg6c_save;
	printk("0x1800006c:%08lx \n",__REG32ALI(0x1800006c));
	__REG32ALI(0x18000080) = reg80_save;
	printk("0x18000080:%08lx \n",__REG32ALI(0x18000080));
	__REG32ALI(0x180000b0) = regb0_save ;
	printk("0x180000b0:%08lx \n",__REG32ALI(0x180000b0));
	__REG32ALI(0x180020d4) = reg20d4_save ;
	printk("0x180020d4:%08lx \n",__REG32ALI(0x180020d4));
	__REG32ALI(0x180020d8) = reg20d8_save ;
	printk("0x180020d8:%08lx \n",__REG32ALI(0x180020d8));
	__REG32ALI(0x18008084) = reg8084_save; // 0x14800000;//reg8084_save ;
	printk("0x18008084:%08lx \n",__REG32ALI(0x18008084));	
	__REG32ALI(0x18028421) = reg28421_save;
	printk("0x18028421:%08lx \n",__REG32ALI(0x18028421));
	 __REG32ALI(0x1803d810) = reg3d810_save;
	printk("0x1803d810:%08lx \n",__REG32ALI(0x1803d810));

	
	output_char('6');
	__REG32ALI(0x18000038)  = reg38_save; //close all interrupt
	output_char('7');
	__REG32ALI(0x1800003c)  = reg3c_save; //close all interrupt
	output_char('8');
	
	ali_s3921_timer_enable();
	return 0;
}


/*
 * Set the sleep timeout value
 */
void operate_time_count(unsigned long timeout)
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
static unsigned long irq_disable_mask_3c = 0x00000000;

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
        writel(readl(p + 12) & irq_disable_mask_3c, p + 12);
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

//extern unsigned int  enter_standby_stop_remote_call ;
//extern unsigned int  enter_in_remote_call;
static inline void operate_nonboot_cpu(int standby)
{
	extern unsigned long see_standby(unsigned long status);
	see_standby(standby);
/*	if(standby)
	{
		while( enter_in_remote_call == 1);			
		enter_standby_stop_remote_call = 1;
		mdelay(1000); // wait remote call
	}else{
		enter_standby_stop_remote_call = 0;
	}
*/	if (standby){
		mdelay(400);
	}
}

    unsigned long ali_3921_irq_flags_38 = 0;
     unsigned long ali_3921_irq_flags_3c = 0;
int ali_see_enter_standby(void)
{
    operate_nonboot_cpu(1);

	printk("ali_see_enter_standby \n");
	ali_3921_irq_flags_38 = __REG32ALI(0x18000038);
	ali_3921_irq_flags_3c = __REG32ALI(0x1800003c);
	__REG32ALI(0x18000038)  = 0; //close all interrupt
	__REG32ALI(0x1800003c)  = 0; //close all interrupt
	return 0;
}

int ali_see_exit_standby(void)
{
	printk("ali_see_exit_standby \n");
    operate_nonboot_cpu(0);

	__REG32ALI(0x18000038)  = ali_3921_irq_flags_38; //close all interrupt
	__REG32ALI(0x1800003c)  = ali_3921_irq_flags_3c; //close all interrupt
	
	return 0;
}

noinline void arch_suspend_disable_irqs(void)
{
//	ali_see_enter_standby();
 //   return;
	output_char('r');
    ali_pm_noirq_opt = 1;   
  //  operate_nonboot_cpu(1);
    preempt_disable();
    operate_extern_irqs(0);
    operate_uart(0);
    local_irq_disable();
    return;
}


#define PMU_RAM_ADDR 0x18050000
#define PMU_STANDBY_SAVE_OFFSET 0x3D00

#define DDR_PHY1_BASE_ADDR 0x1803e000
#define DDR_PHY2_BASE_ADDR 0x1803f000
#define DDR_DM_CTRL_BASE_ADDR 0x18001000


static unsigned long wakeup_power_key[8] = {0x60df708f,0,0,0,0,0,0,0};
static unsigned  char g_panel_key_en = 0;

unsigned long  mips_to_mcs51(unsigned long  tmp)
{
	return (((tmp&0xff)<<24)|(((tmp>>8)&0xff)<<16)|(((tmp>>16)&0xff)<<8)|(((tmp>>24)&0xff)));
}

void pmu_mcu_wakeup_ir_power_key(unsigned long *pmu_ir_key)
{
	unsigned long i;
	unsigned long pmu_ir_key_51[8];

	for(i=0;i<8;i++)
	{
		if(pmu_ir_key[i] == 0xFFFFFFFF)	//#define PAN_KEY_INVALID	0xFFFFFFFF
		{
			pmu_ir_key_51[i] = 0x5a5a5a5a;
		}else{
			pmu_ir_key_51[i] = mips_to_mcs51(pmu_ir_key[i]);
		}
//		printk("pmu_ir_key_51[%ld]:%08x \n",i,pmu_ir_key_51[i]);
	}
	for(i=0;i<8;i++)
	{
		__REG32ALI(0x18050000+0x37d0+i*4) =pmu_ir_key_51[i];
//		printk("set pmu ir_key[%ld]:%08x \n",i,pmu_ir_key[i]);
	
	}
}

#define  MAILBOX_SET_PANEL_KEY_EN 0x1805d209
void pmu_mcu_panel_power_key_en(unsigned char key_en)
{
	__REG8ALI(MAILBOX_SET_PANEL_KEY_EN) = (unsigned char)(key_en & 0xff);
//	printk("MAILBOX_SET_PANEL_KEY_EN ==>>>%x\n",__REG8ALI(MAILBOX_SET_PANEL_KEY_EN));
}

void standby_in_pmu_ram(void)
{
	unsigned long i,len;
	unsigned long save_addr,reg_addr;
	unsigned char temp;
	volatile unsigned long stop;

	u32 val;

	stop = 1;
//	while(stop == 1);
	// init pmu
#if 0   //uart print
	__REG32ALI(0x1800008c) |= 1 << 31;
	__REG32ALI(0x18000088) &= ~(1 << 10);
	__REG32ALI(0x1805c0a4) &= ~(1 << 0);
	__REG32ALI(0x1805c0a4) |= 1 << 24;
	mdelay(10);
	__REG32ALI(0x1805c0a4) &= ~(1 << 24);
#endif
//	__REG32ALI(0x18000038)  = 0; //close all interrupt
//	__REG32ALI(0x1800003c)  = 0; //close all interrupt
	

	len = sizeof(ali_standby_bin);
	printk("copy code to pmu ram,len:%ld \n",len);
	for(i=0;i<len;i++)
	{
		__REG8ALI(PMU_RAM_ADDR+i) = *(ali_standby_bin+i);
	}

	__REG32ALI(0x18000320) &= ~(1 << 17); //reset pmu MCU
	__REG32ALI(0x1805d100) &= ~(1 << 0); //ram switch
	__REG32ALI(0x18000320) |= 1 << 17;	// release pmu mcu
	pmu_mcu_wakeup_ir_power_key(wakeup_power_key);
	pmu_mcu_panel_power_key_en(g_panel_key_en);
	
	// 1	save MEM_CLK (0xb8000070[7:5])
	//	Save ODT Value 0xb8001033[7] 	
	save_addr = PMU_RAM_ADDR + PMU_STANDBY_SAVE_OFFSET;
	reg_addr = DDR_DM_CTRL_BASE_ADDR;
	__REG32ALI(save_addr) = 0xdeadbeef;
	save_addr +=4;	
	printk("return addr save at :0x%08lx\n",save_addr);
	//__REG32ALI(save_addr) = virt_to_phys(&&wakeup_return);
	__REG32ALI(save_addr) = (&&wakeup_return);
	save_addr +=4;
	printk("mem clk save at :0x%08lx\n",save_addr);
	__REG8ALI(save_addr) = __REG8ALI(0x18000070)&(0x07<<5);	
	save_addr ++;
	printk("ODT save at :0x%08lx\n",save_addr);
	__REG8ALI(save_addr) = __REG8ALI(0x18001033)&0x80;	
//	__cpuc_flush_icache_all();
	// 2	0xb8001031[7] =0x1 (Disable Auto-Refresh)
	//	0xb8001033[7] =0x0 (Tie low ODT Pin)
	//	Nop for 100ns ~ 1000ns
	//	0xb8001033[0] =0x1 (Issue Self-Fresh Command
#if 0
	__REG8ALI(0x18001031) |= 0x80;	//(Disable Auto-Refresh)
	__REG8ALI(0x18001033) &= 0x7f;		//(Tie low ODT Pin)
	mdelay(10);
	__REG8ALI(0x18001033) |= 0x01;	//(Issue Self-Fresh Command
#endif	
	// 3	Save Dram Controller Register Value 0xb8001000~0xb8001114 if Chip will Power down or Reset. If Chip always Power On not reset, this step is not need
	save_addr = PMU_RAM_ADDR + PMU_STANDBY_SAVE_OFFSET+0x10;// reserved
//	save_addr = PMU_RAM_ADDR + PMU_STANDBY_SAVE_OFFSET;
	reg_addr = DDR_DM_CTRL_BASE_ADDR;
//	printk("DDR_DM_CTRL start:0x%08x\n",save_addr);
	for(i=0;i<0x114;i++)
	{
		if(0x18001031 == reg_addr)
		{
			
			__REG8ALI(save_addr) = __REG8ALI(reg_addr) |= 0x80;	//(Disable Auto-Refresh)
		}
		else if(0x18001033 == reg_addr)
		{
			__REG8ALI(save_addr) = __REG8ALI(reg_addr) |0x01;		//(Issue Self-Fresh Command
		}else{
			__REG8ALI(save_addr) = __REG8ALI(reg_addr);
		}
		save_addr ++;
		reg_addr ++;
	}
	save_addr = PMU_RAM_ADDR + PMU_STANDBY_SAVE_OFFSET+0x10+0x120;// reserved
	// 4	Save DDR3PHY Register Value 0xb803e000~0xb803e07f ;  0xb803f000~0xb803f07f if Chip will Power down or Reset. If Chip always Power On not reset, this step is not need
	reg_addr = DDR_PHY1_BASE_ADDR;
//	printk("DDR_PHY1 start:0x%08x \n",save_addr);
	for(i=0;i<0x80;i++)
	{
		__REG8ALI(save_addr) = __REG8ALI(reg_addr);
		save_addr ++;
		reg_addr ++;
	}
	
	save_addr = PMU_RAM_ADDR + PMU_STANDBY_SAVE_OFFSET+0x10+0x120 + 0x80;// reserved
	reg_addr = DDR_PHY2_BASE_ADDR;
//	printk("DDR_PHY2 start:0x%08x \n",save_addr);
	for(i=0;i<0x80;i++)
	{
		__REG8ALI(save_addr) = __REG8ALI(reg_addr);
		save_addr ++;
		reg_addr ++;
	}
	// 5	Enter Retention Mode B805_D0D0[0]=1
//	__REG8ALI(0x1805D0D0) |=0x01;		// do it in pmu code, mcu will power off ddr phy

	save_addr = PMU_RAM_ADDR + PMU_STANDBY_SAVE_OFFSET+0x10+0x120 + 0x80 + 0x80;// reserved
	printk("cp15 save at :0x%08lx\n",save_addr);
	//asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r" (val));
/*
MRC p15,0,<Rt>,c13,c0,1 ; Read CP15 Context ID Register
MCR p15,0,<Rt>,c13,c0,1 ; Write CP15 Context ID Register
MRC p15, 0, <Rt>, c13, c0, 2 ; Read CP15 User Read/Write Thread ID Register
MCR p15, 0, <Rt>, c13, c0, 2 ; Write CP15 User Read/Write Thread ID Register
MRC p15, 0, <Rt>, c13, c0, 3 ; Read CP15 User Read-only Thread ID Register
MCR p15, 0, <Rt>, c13, c0, 3 ; Write CP15 User Read-only Thread ID Register
MRC p15, 0, <Rt>, c13, c0, 4 ; Read CP15 Privileged Only Thread ID Register
MCR p15, 0, <Rt>, c13, c0, 4 ; Write CP15 Privileged Only Thread ID Register
*/
	asm volatile("mrc p15, 0, %0, c13, c0, 1" : "=r" (val));	//	 Read CP15 Context ID Register
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c13, c0, 2" : "=r" (val));	//	 Read CP15 User Read/Write Thread ID Register
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c13, c0, 3" : "=r" (val));	//	 Read CP15 User Read-only Thread ID Register
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c13, c0, 4" : "=r" (val));	//	 Read CP15 Privileged Only Thread ID Register
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c13, c0, 0" : "=r" (val));	//	 FCSE/PID
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c13, c0, 3" : "=r" (val));	//	 User r/o thread ID
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c3, c0, 0" : "=r" (val));	//	 Domain ID
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c2, c0, 0" : "=r" (val));	//	 TTB 0
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c2, c0, 1" : "=r" (val));	//	 TTB 1
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c2, c0, 2" : "=r" (val));	//	TTB control register
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r" (val));	//	Control register
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c1, c0, 1" : "=r" (val));	//	Auxiliary control register
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mrc p15, 0, %0, c1, c0, 2" : "=r" (val));	//	Co-processor access control
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
		
	save_addr = PMU_RAM_ADDR + PMU_STANDBY_SAVE_OFFSET+0x10+0x120 + 0x80 + 0x80 + 0x38 ;// reserved
	printk("general reg save at :0x%08lx\n",save_addr);
	asm volatile("mov %0, r9" : "=r" (val));	//
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mov %0, r10" : "=r" (val));	//
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mov %0, r11" : "=r" (val));	//
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mov %0, r12" : "=r" (val));	//
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mov %0, r13" : "=r" (val));	//
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	asm volatile("mov %0, r14" : "=r" (val));	//
	__REG32ALI(save_addr) = val;
	save_addr = save_addr+4;
	
	// 6	Reset/Chip Core Power off
		
//	__REG32ALI(0x18018500) = (0xffffffff - 0x1fffffff);	//watch dog count
//	__REG8ALI(0x18018504) = 0x67;	//enable watch dog


	output_char('1');
	__REG32ALI(0x18000038)  = 0; //close all interrupt
	output_char('2');
	__REG32ALI(0x1800003c)  = 0; //close all interrupt
	output_char('3');
	temp = __REG8ALI(0x1805d211);
	temp |= 0x01;
	__REG8ALI(0x1805d211) = temp; //cpu->mcu IE int enable
	output_char('4');
//	temp = __REG8ALI(0x1805d210);
//	temp |= 0x01;
//	__REG8ALI(0x1805d210) = temp; //cpu->mcu IE int enable, mcu will power off mai cpu
	output_char('5');
#if 0	
	__REG8ALI(0x18001031) |= 0x80;	//(Disable Auto-Refresh)
	__REG8ALI(0x18001033) &= 0x7f;		//(Tie low ODT Pin)
	delay_count = 0x100;
	while(delay_count > 1)
		delay_count --;
	__REG8ALI(0x18001033) |= 0x01;	//(Issue Self-Fresh Command
#endif

	__REG32ALI(0x18006010) &= ~(0x01<<0);	// fb register  VHD plane
	__REG32ALI(0x18006014) &= ~(0x01<<0);	// fb register VSD
	ali_3921_finish_suspend(3);

//	__REG8ALI(0x1805c101) = 0x01; //mcu will power off mai cpu
//	__REG8ALI(0x1A000000) = 0x00;		//remap
//	output_char('6');

//	__REG8ALI(0x1805d0d0) = 0x01; //mcu will power off ddr phy
//	stop = 0xfffff;
//	while(stop >1) stop --;
	
 	
wakeup_return:
	stop = 0;

	
	return;
}





noinline void arch_suspend_enable_irqs(void)
{	
	output_char('v');
//    gic_init(0, GIC_PPI_START, A9_MPCORE_GIC_DIST, A9_MPCORE_GIC_CPU);
	output_char('v');
    local_irq_enable();
    operate_uart(1);
    operate_extern_irqs(1);
    preempt_enable();
  //  operate_nonboot_cpu(0);    
    ali_pm_noirq_opt = 0;
    return;
}

