/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_suspend.c
 *  (I)
 *  Description: ALi power management implementation
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2011.03.07				Owen			Creation
 ****************************************************************************/

#include "ali_suspend.h"
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/version.h>

//extern unsigned long cache_code(unsigned long standby_key, unsigned long ir_power,unsigned long time );

/*
 * Spinlock for suspend
 */
static DEFINE_SPINLOCK(ali_suspend_lock);

/*
 * Default value of resume key
 */
#define IR_POWER_VALUE        0x60df708f		// ali demo stb power key
#define IR_POWER_VALUE1        0x10efeb14		// nmp power key 1
#define IR_POWER_VALUE2        0x00ff00ff		// nmp power key 2

/*
 * Debug the suspend state
 */
#ifdef PM_SUSPEND_STATUS_DEBUG
volatile e_pm_state __pm_state = -1;
#endif

/*
 * Enable debug feature
 */
// #define PM_THREAD_STATUS

#ifdef PM_THREAD_STATUS
static char *pm_task_status[] = { \
    "Running", "Interruptible", "Uninterruptible", \
    "Stopped", "Traced", "exit Zombie", "exit Dead", \
    "Dead", "Wakekill", "Unknown"
};
static inline int ali_suspend_get_status_bit(int state)
{
    switch (state)
    {
        case TASK_RUNNING:
            return 0;
        case TASK_INTERRUPTIBLE:
            return 1;
        case TASK_UNINTERRUPTIBLE:
            return 2;
        case __TASK_STOPPED:
            return 3;
        case __TASK_TRACED:
            return 4;
        case EXIT_ZOMBIE:
            return 5;
        case EXIT_DEAD:
            return 6;
        case TASK_DEAD:
            return 7;
        case TASK_WAKEKILL:
            return 8;
        default:
            printk("Task status is 0x%x\n", state);
            return 9;
    }
}
#endif

/*
 * Accept the key
 */
static pm_key_t resume_key = {
    .standby_key = 0,
    .ir_power[0] = IR_POWER_VALUE,
    .ir_power[1] = IR_POWER_VALUE1,
    .ir_power[2] = IR_POWER_VALUE2,
    .ir_power[3] = IR_POWER_VALUE,
    .ir_power[4] = IR_POWER_VALUE,
    .ir_power[5] = IR_POWER_VALUE,
    .ir_power[6] = IR_POWER_VALUE,
    .ir_power[7] = IR_POWER_VALUE,
};

/*
 * Accept the parameter
 */
static pm_param_t standby_param = {
    .board_power_gpio = -1,
    .timeout = 0,
    .reboot = 0,
};

/*
 * Set the resume key value
 */
void ali_suspend_set_resume_key(pm_key_t *pm_key)
{
	unsigned long i;
    if (NULL != pm_key)
    {
        resume_key.standby_key = pm_key->standby_key;
	for(i=0;i<8;i++)
	{
		if(( pm_key->ir_power[i] != 0)&&( pm_key->ir_power[i] != 0xffffffff))
		{
        		resume_key.ir_power[i] = pm_key->ir_power[i];
		}else{
        		resume_key.ir_power[i] = 0x5a5a55aa;
		}
		printk("ir power key[%ld]:%lx \n",i,resume_key.ir_power[i] );
	}
    }
}

/*
 * Set the resume key value
 */
void ali_suspend_set_standby_param(pm_param_t *p_standby_param)
{
    if (NULL != p_standby_param)
    {
        standby_param.board_power_gpio = p_standby_param->board_power_gpio;
        standby_param.timeout = p_standby_param->timeout;
        standby_param.reboot = p_standby_param->reboot;
    }
}


/*
 * To valie the status
 */
static int ali_suspend_state_valid(suspend_state_t suspend_state)
{
	switch (suspend_state) {
	case PM_SUSPEND_ON:
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		return 1;
	default:
		return 0;
	}

	return 0;
}

static inline void output_char(unsigned char c)
{
	while(((*(volatile unsigned char *)0xb8018305)&0x40) == 0x00);	//wait empty
	*(volatile unsigned char *)0xb8018300 = c;
}

void standby_dump_reg(unsigned long addr,unsigned long len)
{
	unsigned long i,j;
	char* ascii;
	unsigned char index;
	
	ascii = "0123456789ABCDEF";
	for(i=0;i<len;i++)
	{
		if(i%16 == 0)
		{
			output_char(0x0d);
			output_char(0x0a);
			for(j=0;j<8;j++)
			{
				output_char(ascii[((addr+i)>>(4*(7-j)))&0xF]);
			}
			output_char(':');
		}
		index = *(unsigned char *)(addr+i);
		output_char(ascii[(index>>4)&0xF]);
		output_char(ascii[index&0xF]);
	}
	output_char(0x0d);
	output_char(0x0a);
	return;
}


int ali_suspend_enter(suspend_state_t suspend_state)
{    
//    int wq_ret = 0;
//    wait_queue_head_t suspend_wq;
//    unsigned long flags;
//	struct task_struct *p = NULL;
	struct timeval cur_time; 
	struct timespec tv;
	struct rtc_time utc_time;
	unsigned long time,sleep_sec_count;
	unsigned long	power_key_count;
	unsigned long cpu_clk;
	volatile unsigned long i;

//	31...26 25...22 21...17 16...12 11...6 5...0
//	  year   mon    date     hour    min   sec
	unsigned long chip_id,chip_bonding,reg_00;
	unsigned long gpio_enable_save_430,gpio_ioctl_save_58,gpio_io_save_54;

	reg_00 = *((volatile unsigned long *)(0xb8000000));
	chip_id = (reg_00 >> 16);
	chip_bonding = (reg_00 >> 8)&0x07;
    __asm__(".set noreorder");
    printk("Info, suspend enter\n");
	
	/* We don't need to wait here, thanks to the loop in the cache */
    /* init_waitqueue_head(&suspend_wq);
    wait_event_interruptible(suspend_wq, (1 == wq_ret)); */

	/* This process must be atomic, or we'll be unknown status */
	/* But we don't care about that, due to IRQ is disable when enter state */

#ifdef PM_THREAD_STATUS
	for_each_process(p) {
		printk("name: %s   status: %d %s task struct %p\n", \
               p->comm, p->state, \
               pm_task_status[ali_suspend_get_status_bit(p->state)], p);
	}
#endif
    printk("Info, run in cache\n");
#ifdef PM_SUSPEND_STATUS_DEBUG
    __pm_state = PM_IRQ_DISABLE;
#endif
    suspend_step('a');
    pm_standby_prepare(PM_ENTER_STANDBY);
    operate_device(PM_DISABLE_DEVICE);
    suspend_step('b');
	do_gettimeofday(&cur_time); 
	rtc_time_to_tm(cur_time.tv_sec,&utc_time);
	printk("UTC time :%d-%d-%d %d:%d:%d \n" ,utc_time.tm_year+1900,utc_time.tm_mon, utc_time.tm_mday,utc_time.tm_hour,utc_time.tm_min,utc_time.tm_sec);
	time= (utc_time.tm_sec & 0x3F ) | ((utc_time.tm_min & 0x3F )<<6)  | ((utc_time.tm_hour & 0x1F )<<12) | ((utc_time.tm_mday & 0x1F)<<17)
			| ((utc_time.tm_mon & 0xF) << 22) | (((utc_time.tm_year % 100) & 0x3F)<<26);
#ifdef PM_SUSPEND_STATUS_DEBUG
    __pm_state = PM_ENTER_CACHE;
#endif
	standby_param.board_power_gpio = 68;
//	standby_dump_reg(0xb805E000,0x20);
	*(volatile unsigned long *) 0xb8000084 |= (1<<24);		//  reset pmu ip:  if pmu gpio is output , IR Rx will be 2.5v
	mdelay(10);
	*(volatile unsigned long *) 0xb8000084 &= ~((1<<24));		//   pmu ip reset
	mdelay(10);
	if((chip_id == 0x3701)&&(chip_bonding == 0x04 ))	// 144PIN for NMP 
	{
		*(volatile unsigned char *)0xb8018300 = 'i';
		gpio_enable_save_430 = (*(volatile unsigned long *)0xB8000430);
		gpio_ioctl_save_58 = (*(volatile unsigned long *)0xB8000058);
		gpio_io_save_54 = (*(volatile unsigned long *)0xB8000058);
		(*(volatile unsigned long *)0xB8000430) |= 0x7FF9FFE0; //3912 nand share pin GPIO[5:16] GPIO[19:20] XRMII share pin GPIO[21:30]
		//(*(volatile unsigned long *)0xB8000058)  &= ~(0x7FF9FFE0);	// input
		(*(volatile unsigned long *)0xB8000058)  |= 0x7FF9FFE0;	// output
		(*(volatile unsigned long *)0xB8000054)  &= ~(0x7FF9FFE0);	// output 0 for STD3V3 1.3V -> 0.69V,  if STD3V3 is 1.3V wifi can not work after standby
	}
    sleep_sec_count = cache_code(resume_key.standby_key, (unsigned long)resume_key.ir_power, \
               time,standby_param.timeout);
	if((chip_id == 0x3701)&&(chip_bonding == 0x04 ))	// 144PIN for NMP 
	{
		(*(volatile unsigned long *)0xB8000058) = gpio_ioctl_save_58;
		(*(volatile unsigned long *)0xB8000054) = gpio_io_save_54;
		(*(volatile unsigned long *)0xB8000430) = gpio_enable_save_430;
		*(volatile unsigned char *)0xb8018300 = 'o';
	}
#ifdef PM_SUSPEND_STATUS_DEBUG
    __pm_state = PM_LEAVE_CACHE;
#endif
	while(((*(volatile unsigned char *)0xb8018305)&0x40) == 0x00);	//wait uart empty
	*(volatile unsigned char *)0xb8018300 = 'r';
	if(standby_param.reboot == 1)
	{
		(*(volatile unsigned long *)0xB8000430) |= 0x40;	//GPIO[2] enable	0xB8000430[2]
		power_key_count = 0;
		while(1)		// wait for power key release before reboot, if power key is pressed, bootrom will jump to usb upgrade
		{
			if(((*(volatile unsigned long *)0xb8000050)&0x04) == 0x00)	//GPIO[2] 0:normal 1: upgrade
			{
				power_key_count  ++;
				if(power_key_count > 50)
					break;
			}else{
				power_key_count = 0;
			}
		}		
		while(((*(volatile unsigned char *)0xb8018305)&0x40) == 0x00);	//wait uart empty
		*( volatile unsigned char *)0xb8018300 = 'w';
		*(volatile unsigned long *) 0xb8018500 = (0xFFFFFFFF-0x0001FFF);	// watchdog count
		*(volatile unsigned char *) 0xb8018504 = 0x67 ;		//enable watch dog
		cpu_clk = *((volatile unsigned long *)(0xb8000070));
		cpu_clk = (cpu_clk>>7)&0x07;
		if(cpu_clk == 0x04)	// cpu clk is pll clk, pll clk have no out put  after watch dog reboot
		{
			*((volatile unsigned long *)(0xb8000068)) |= (1<<31);	// cpu clk swith enable
			for(i=0;i<0x1ff;i++);
			*((volatile unsigned long *)(0xb8000074)) = (1<<31)|(0x00<<7);	// cpu clk 600MHz, strap pin control bit[7:9]
			for(i=0;i<0x1ff;i++);
			*((volatile unsigned long *)(0xb8000068)) &= ~(1<<31);	// cpu clk swith disable
		}

		while(1);
	}
	while(((*(unsigned char *)0xb8018305)&0x40) == 0x00);	//wait uart empty
	*(unsigned char *)0xb8018300 = 'b';
//	err = rtc_read_time(rtc, &tm);
//	rtc_tm_to_time(&tm, &tv.tv_sec);
	tv.tv_sec = cur_time.tv_sec + sleep_sec_count;
	tv.tv_nsec = 0;
	do_settimeofday(&tv);
    suspend_step('c');
	*(unsigned char *)0xb8018300 = 'c';
    operate_device(PM_ENABLE_DEVICE);
    pm_standby_prepare(PM_EXIT_STANDBY);
    suspend_step('d');
#ifdef PM_SUSPEND_STATUS_DEBUG
    __pm_state = PM_IRQ_ENABLE;
#endif
    printk("Info, run in memory\n");
#ifdef PM_THREAD_STATUS
	for_each_process(p) {
		printk("name: %s   status: %d %s task struct %p\n", \
               p->comm, p->state, \
               pm_task_status[ali_suspend_get_status_bit(p->state)], p);
	}
#endif
    
    printk("Info, suspend leave\n");
    
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)

extern int ali_see_enter_standby(void);
extern void ali_see_exit_standby(void);

static struct platform_suspend_ops ali_suspend_ops = {
	.valid = ali_suspend_state_valid,
	.prepare = ali_see_enter_standby,
	.enter = ali_suspend_enter,
	.wake = ali_see_exit_standby,
};
#else
static struct platform_suspend_ops ali_suspend_ops = {
	.valid = ali_suspend_state_valid,
	.enter = ali_suspend_enter,
};
#endif


void ali_suspend_register_ops(void)
{
	printk("Info, register suspend operation\n");
	spin_lock_init(&ali_suspend_lock);
	suspend_set_ops(&ali_suspend_ops);
}
