#include <linux/version.h> 
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/clockchips.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <mach/ali-s3921.h>
#include <mach/hardware.h>
#include <mach/irqs.h>

#include <asm/mach/time.h>
#include <asm/sched_clock.h>
#include <ali_interrupt.h>
#include <ali_reg.h>

#ifdef CONFIG_GENERIC_CLOCKEVENTS

/*SB timer2 defination*/
#define ALI_SBTIMER_BASE VIRT_SOUTHBRIDGE+0xA00
#define REG_SB_TM0_AIM_SECOND  ALI_SBTIMER_BASE+0x00 //32bit
#define REG_SB_TM0_AIM_MS      ALI_SBTIMER_BASE+0x04 //16bit
#define REG_SB_TM0_TICK_VALUE  ALI_SBTIMER_BASE+0x06 //16bit
#define REG_SB_TM0CTRL ALI_SBTIMER_BASE+0x08   //8bit
#define REG_SB_TM0_CUR_MS      ALI_SBTIMER_BASE+0x0A //16bit
#define REG_SB_TM0_CUR_SECOND  ALI_SBTIMER_BASE+0x0C //32bit

#define REG_SB_TM2CNT	       ALI_SBTIMER_BASE+0x20 //32bit
#define REG_SB_TM2CTRL 		ALI_SBTIMER_BASE+0x28   //8bit

#define REG_SB_TM4CNT		ALI_SBTIMER_BASE+0x40  //32bit
#define REG_SB_TM4CMP		ALI_SBTIMER_BASE+0x44  //32bit
#define REG_SB_TM4CTRL		ALI_SBTIMER_BASE+0x48  //8bit

static unsigned long g_sbtm4cnt = 27000000/(16*HZ);
static unsigned long g_sbtm4freq = 27000000/16;

/* Clockevent device: use one-shot mode */
unsigned int  g_cpufreq = 1000000000;
static int default_clkevt_next(unsigned long evt, struct clock_event_device *ev)
{
	unsigned int next_time;
	unsigned int cycle = g_cpufreq/(HZ*2); 

	writel_relaxed(0,REG_SB_TM4CNT);
	writeb_relaxed(0, REG_SB_TM4CTRL);
        printk("^^^^^^ default_clkevt_next 1, ctrl:0x%x, cmp:0x%x\n", readb_relaxed(REG_SB_TM4CTRL), readb_relaxed(REG_SB_TM4CMP));
        writeb_relaxed((readb_relaxed(REG_SB_TM4CTRL)|(1<<2)|(1<<4)), REG_SB_TM4CTRL);
        printk("^^^^^^ default_clkevt_next end, ctrl:0x%x, cmp:0x%x\n", readb_relaxed(REG_SB_TM4CTRL), readb_relaxed(REG_SB_TM4CMP));

	return 0;
}

/*We must call this after updating the frequency*/
void clkevt_freq_update(unsigned int freq)
{
	//printk("clkevt_freq_update entered!\n");
	g_cpufreq = freq;
}

static void my_clkevt_mode(enum clock_event_mode mode,
			     struct clock_event_device *dev)
{
    printk("clkmode:%x %x %x\n",mode,dev,dev->event_handler);
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		printk("my_clkevt_mode periodic mode\n");
		default_clkevt_next(0x00,dev);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		break;
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_UNUSED:
		break;
	case CLOCK_EVT_MODE_RESUME:
		break;
	}
}
static struct clock_event_device myclkevt = {
	.name		= "default_clk",
	.features	= CLOCK_EVT_FEAT_ONESHOT | CLOCK_EVT_FEAT_PERIODIC,
	.rating		= 300,
	.set_mode	= my_clkevt_mode,
	.set_next_event	= default_clkevt_next,
};

static u32 notrace ali3921_read_sched_clock(void)
{
        return readl(REG_SB_TM4CNT);
}

static struct clocksource myclocksource = {
    .name   = "my_hrtimer_src",
    .rating = 300,
    .read   = ali3921_read_sched_clock,
    .mask   = CLOCKSOURCE_MASK(32),
    .flags  = CLOCK_SOURCE_IS_CONTINUOUS,
};

#endif
/*	ali_s3921_timer */
static irqreturn_t ali_s3921_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = (struct clock_event_device *)dev_id;
#if 0
	static counter = 0;
	
	if(((counter++)%200)==0)
		printk("^^^^^^^SB Timer4 interrupt arrived, counter:0x%x,tm2ctrl:0x%x ctrl:0x%x, cnt:0x%x, cmp:0x%x g_sbtm4cnt:0x%x\n", counter, readb(REG_SB_TM2CTRL),readb(REG_SB_TM4CTRL), readl(REG_SB_TM4CNT),readl(REG_SB_TM4CMP), g_sbtm4cnt);
#endif
	/*clear irq*/
	writeb((readb(REG_SB_TM4CTRL)|(1<<3)), REG_SB_TM4CTRL);	
	/*stop timer*/
	//writeb((readb(REG_SB_TM4CTRL)&(~(1<<2) )), REG_SB_TM4CTRL);

	//writel((readl(REG_SB_TM4CNT)+0)&0xffffffff,REG_SB_TM4CNT);	
	writel((readl(REG_SB_TM4CNT)+g_sbtm4cnt)&0xffffffff,REG_SB_TM4CMP);
	
	//writeb((readb(REG_SB_TM4CTRL)|(1<<2)), REG_SB_TM4CTRL);
	
	if(evt->event_handler)
		evt->event_handler(evt);
	
        return IRQ_HANDLED;
}

static struct irqaction ali3921_timer_irq = {
        .name           = "ali_timer_irq",
        .flags          = IRQF_TIMER | IRQF_IRQPOLL,
        .handler        = ali_s3921_timer_interrupt,
        .dev_id         = &myclkevt,
        .irq            = INT_ALI_RTC,
};

void ali_s3921_timer_enable(void)
{
	/*Init SB timer4*/
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x08); //disable Time0 and Time0 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x18); //disable Time1 and Time1 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x28); //disable Time2 and Time1 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x38); //disable Time3 and Time3 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x58); //disable Time5 and Time5 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x68); //disable Time6 and Time6 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x78); //disable Time7 and Time7 interrupt

	writeb_relaxed(8, ALI_SBTIMER_BASE+0x48); //disable Timer4 interrupt

        writel_relaxed(0,REG_SB_TM4CNT);
        writel_relaxed(g_sbtm4cnt,REG_SB_TM4CMP);

}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
void ali_s3921_timer_init(void)
#else
static void ali_s3921_timer_init(void)
#endif
{
	int res;

        /*Init SB timer4*/
	writeb_relaxed(8, ALI_SBTIMER_BASE+0x08); //disable Time0 and Time0 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x18); //disable Time1 and Time1 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x28); //disable Time2 and Time1 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x38); //disable Time3 and Time3 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x58); //disable Time5 and Time5 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x68); //disable Time6 and Time6 interrupt
        writeb_relaxed(8, ALI_SBTIMER_BASE+0x78); //disable Time7 and Time7 interrupt
                                
        writel_relaxed(0,REG_SB_TM4CNT);
        writel_relaxed(g_sbtm4cnt,REG_SB_TM4CMP);

	setup_sched_clock(ali3921_read_sched_clock, 32, g_sbtm4freq);
#if 1 
	if (clocksource_mmio_init(REG_SB_TM4CNT,
                "timer_us", g_sbtm4freq, 300, 32, clocksource_mmio_readl_up)) {
                printk(KERN_ERR "Failed to register clocksource\n");
        }
        printk("clocksource_mmio_init end\n");
#endif
#if 0 
	clocks_calc_mult_shift(&myclocksource.mult, &myclocksource.shift, NSEC_PER_SEC, g_sbtm4freq, 5);
	clocksource_register(&myclocksource);  
	printk("register clock source end\n");
#endif
	res = setup_irq(ali3921_timer_irq.irq, &ali3921_timer_irq);
        if (res) {
                printk("Failed to register timer IRQ: %d\n", res);
        }
	printk("setup_irq end\n");

        clockevents_calc_mult_shift(&myclkevt, g_sbtm4freq, 5);
        myclkevt.max_delta_ns =
                clockevent_delta2ns(0xffffffff, &myclkevt);
        myclkevt.min_delta_ns =
                clockevent_delta2ns(0x01, &myclkevt);
        myclkevt.cpumask = cpu_all_mask; //cpumask_of(0);
        myclkevt.irq = ali3921_timer_irq.irq;
        clockevents_register_device(&myclkevt);

	printk("ali_s3921_timer_init end\n");

}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#else
struct sys_timer ali_s3921_timer = {
	.init = ali_s3921_timer_init,
};
#endif
