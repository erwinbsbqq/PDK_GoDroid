/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2006 Copyright (C)
*
*    File:    dog_m3327e.c
*
*    Description:    This file contains all globe micros and functions declare
*		             of watchdog timer.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	May.2.2006       Justin Wu      Ver 0.1    Create file.
*	2.
*****************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/uaccess.h>
//#include <asm/mach-ali/chip.h>
//#include "linux/dvb/ali_soc.h"
#include <ali_soc_common.h>
#include <linux/delay.h>

#include <ali_reg.h>

#define PFX "alim36dog: "

static spinlock_t wdt_lock;
static unsigned int wdt_count;
static unsigned int wdt_timeleft;

#define WDT_DEBUG   printk 
//#define WDT_DEBUG   do{}while(0)

#define WATCHDOG_TIMEOUT 60		/* 60 sec default timeout */

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout,
		"Watchdog cannot be stopped once started (default="
				__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");


#define DOG_M3327E_WDTCNT			0
#define DOG_M3327E_WDTCTRL			4

#define DOG_M3327E_NUM				1

typedef unsigned long UINT32;
typedef unsigned char UINT8;
typedef unsigned short UINT16; 

#if defined(CONFIG_ALI_CHIP_M3921)

static struct
{
	UINT32 reg_base;
	int    irq;
	UINT32 timebase;			/* The init value of cnt (we need config dog in us) */
} dog_m3327e_reg[DOG_M3327E_NUM] = {{0x18018500, 31, 0}};

#define DOG_WRITE32(id, reg, data)	((__REG32ALI(dog_m3327e_reg[id].reg_base + reg)) = (data))
#define DOG_READ32(id, reg)			(__REG32ALI(dog_m3327e_reg[id].reg_base + reg))
#define DOG_WRITE8(id, reg, data)		((__REG8ALI(dog_m3327e_reg[id].reg_base + reg)) = (data))
#define DOG_READ8(id, reg)			(__REG8ALI(dog_m3327e_reg[id].reg_base + reg))

#else

static struct
{
	UINT32 reg_base;
	int    irq;
	UINT32 timebase;			/* The init value of cnt (we need config dog in us) */
} dog_m3327e_reg[DOG_M3327E_NUM] = {{0xb8018500, 31, 0}};

#define DOG_WRITE32(id, reg, data)	(*((volatile UINT32 *)(dog_m3327e_reg[id].reg_base + reg)) = (data))
#define DOG_READ32(id, reg)			(*((volatile UINT32 *)(dog_m3327e_reg[id].reg_base + reg)))
#define DOG_WRITE8(id, reg, data)	(*((volatile UINT8 *)(dog_m3327e_reg[id].reg_base + reg)) = (data))
#define DOG_READ8(id, reg)			(*((volatile UINT8 *)(dog_m3327e_reg[id].reg_base + reg)))
#endif


static UINT32 sys_mem_clk = 27;

//extern DEFINE_MUTEX(nor_nand_mutex);
extern nor_nand_mutex;
uint32_t strappin70;
static void wdt_reboot_from_nand(void)
{
	uint32_t tmp;
	mutex_lock(&nor_nand_mutex);       

    if(ali_sys_ic_get_chip_id() == ALI_C3701)    
    {	    
        *(volatile uint32_t *)(0xb8000074) |= 0x40040000;  //NOTE: just for 3701 chip        
    }    
    if(ali_sys_ic_get_chip_id() == ALI_S3503)	
    {        
		tmp = strappin70;
		tmp |= ((1<<30)|(1<<29)|(1<<24));
		*(volatile uint32_t *)(0xb8000074) = tmp;
    }
    if(ali_sys_ic_get_chip_id() == ALI_C3921)	
    {
		__REG32ALI(0x18000074) = (1<<23)|(1<<18);		// strap pin to nand		
    }
     mutex_unlock(&nor_nand_mutex); 
}

static void wdt_reboot_from_nor(void)
{
	uint32_t tmp;
	mutex_lock(&nor_nand_mutex);       
    if(ali_sys_ic_get_chip_id() == ALI_C3701)    
    {	  
		*(volatile uint32_t *)(0xb8000074) &= ~(0x1<<18);  
        *(volatile uint32_t *)(0xb8000074) |= 0x40000000; //NOTE: just for 3701 chip            
    }        
    if(ali_sys_ic_get_chip_id() == ALI_S3503)	
	{
		/*
                           74[30]    74[29]    74[24]    74[18]    74[17]    74[15]
External Nor flash boot       1        /         1          0         0         X
SIP flash boot                1        /         1          0         1         X
Nand flash boot               1        1         1          1         0         0
		*/
		strappin70 = (*(volatile uint32_t *)(0xb8000070));
		tmp = strappin70;
		tmp &= ~(1<<18);
		tmp |= (1<<30);
		*(volatile uint32_t *)(0xb8000074) = tmp;
	}
    if(ali_sys_ic_get_chip_id() == ALI_C3921)	
    {
		__REG32ALI(0x18000074) = (1<<23)|(0<<18);		// strap pin to nor	
    }
     mutex_unlock(&nor_nand_mutex); 
}

static void m36wdt_init(void)
{
#if defined(CONFIG_ALI_CHIP_M3921)
	dog_m3327e_reg[0].reg_base=0x18018500;
	dog_m3327e_reg[0].irq = 23;
	dog_m3327e_reg[0].timebase=0;  
	//sys_mem_clk = ali_sys_ic_get_dram_clock();
	//later will get clock from the right function
	sys_mem_clk = 27;
#else
	dog_m3327e_reg[0].reg_base=0xb8018500;
	dog_m3327e_reg[0].irq = 34;
	dog_m3327e_reg[0].timebase=0;  
	//sys_mem_clk = ali_sys_ic_get_dram_clock();
	//later will get clock from the right function
	sys_mem_clk = 27;
#endif
}

static void m36wdt_get_lefttime(void)
{
	UINT16 div;
	div = DOG_READ8(0, DOG_M3327E_WDTCTRL) & 3;
	div = (1 << (5 + div));
    wdt_timeleft = (0xffffffff - (DOG_READ32(0, DOG_M3327E_WDTCNT)))/sys_mem_clk*div;
}

// 3701c cpu clk from pll clk to 600MHz
static void change_cpu_clk(void)	
{
    int i, ret;

    /* cpu clk swith enable */
	*((volatile unsigned long *)(0xb8000068)) |= (1<<31);	
	for(i=0;i<0x3ff;i++);
    /* cpu clk 600MHz, strap pin control bit[7:9] */
	*((volatile unsigned long *)(0xb8000074)) |= (1<<31);
    *((volatile unsigned long *)(0xb8000074)) &= ~((1<<8)|(1<<9)|(1<<7));
	for(i=0;i<0x3ff;i++);
    /* cpu clk swith disable */
	*((volatile unsigned long *)(0xb8000068)) &= ~(1<<31);	
    for(i=0;i<0x3ff;i++);
    ret = *((volatile unsigned long *)(0xb8000070));
    WDT_DEBUG("[%s] strappin 0x70 = 0x%x \n",__FUNCTION__, ret);
}

static void m36wdt_start(void)
{
    UINT16 div;
	UINT32 a, duration_us; 
    unsigned long cpu_clk;
    
    duration_us = wdt_count; 
	a = 0xffffffff / sys_mem_clk;
	if (duration_us < (a << 5))
		div = 0;
	else if (duration_us < (a << 6))
		div = 1;
	else if (duration_us < (a << 7))
		div = 2;
	else
		div = 3;
    spin_lock(&wdt_lock);

	dog_m3327e_reg[0].timebase = 0xffffffff - (duration_us / (1 << (5 + div)) * sys_mem_clk);

    DOG_WRITE32(0, DOG_M3327E_WDTCNT, dog_m3327e_reg[0].timebase);/* It is watchdog mode */

#if (!defined(CONFIG_ALI_CHIP_M3921))
    if(ALI_C3701 == ali_sys_ic_get_chip_id())/* for M3912 dongle */
    {
        
    	cpu_clk = *((volatile unsigned long *)(0xb8000070));
        WDT_DEBUG("4[%s] 70 =0x%x end.\n",__FUNCTION__,cpu_clk);
    	cpu_clk = (cpu_clk>>7)&0x07;
        /* cpu clk is pll clk, pll clk have no output  after watch dog reboot */
    	if(cpu_clk == 0x04)	
    	{
            /* in order to execute change_cpu_clk() */
            DOG_WRITE32(0, DOG_M3327E_WDTCNT, dog_m3327e_reg[0].timebase-0xfff);/* It is watchdog mode */
            DOG_WRITE8(0, DOG_M3327E_WDTCTRL, 0x64 | div);
            load_to_icache(change_cpu_clk,0x200);
            change_cpu_clk();
        }
    }
#endif

    DOG_WRITE8(0, DOG_M3327E_WDTCTRL, 0x64 | div);
    
    spin_unlock(&wdt_lock);
}

static void m36wdt_stop(void)
{
	spin_lock(&wdt_lock);
	DOG_WRITE32(0, DOG_M3327E_WDTCTRL, 0);
	DOG_WRITE32(0, DOG_M3327E_WDTCNT, 0);
	spin_unlock(&wdt_lock);
}

static void m36wdt_ping(void)
{
	UINT16 div;
    UINT32 us;
    us = wdt_count; 
	spin_lock(&wdt_lock);

	// keeplive: use the old time out
//	div = DOG_READ8(0, DOG_M3327E_WDTCTRL) & 3;
//	div = (1 << (5 + div));
	
//	dog_m3327e_reg[0].timebase = 0xffffffff - (us / div * sys_mem_clk);
	
    DOG_WRITE32(0, DOG_M3327E_WDTCNT,  dog_m3327e_reg[0].timebase);
	spin_unlock(&wdt_lock);
}

static int m36wdt_open(struct inode *inode, struct file *file)
{
	//m36wdt_start();
	m36wdt_init();
	return nonseekable_open(inode, file);
}

static int m36wdt_release(struct inode *inode, struct file *file)
{
	//m36wdt_stop();
	return 0;
}

static ssize_t m36wdt_write(struct file *file, const char __user *data,
				size_t len, loff_t *ppos)
{
    if (len)
    {
        wdt_count = len;
        m36wdt_start();
	    m36wdt_ping();
    }
	return len;
}

extern bool board_is_nand_boot(void);

static long m36wdt_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
   int options, retval = -EINVAL;
   void __user *argp = (void __user *)arg;
   int __user *p = argp;
   unsigned int reboot_sec = 0;
   unsigned int new_margin;
    
   static struct watchdog_info ident = {
		.options		= WDIOF_KEEPALIVEPING |
					  WDIOF_MAGICCLOSE,
		.firmware_version	= 0,
		.identity		= "Hardware Watchdog for ALI M36",
	};

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		if (copy_to_user((struct watchdog_info *)arg,
				 &ident, sizeof(ident)))
			return -EFAULT;
		return 0;
	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, (int *)arg);
	case WDIOC_SETOPTIONS:
	{
		if (get_user(options, (int *)arg))
			return -EFAULT;
		if (options & WDIOS_DISABLECARD) {
			m36wdt_stop();
			retval = 0;
		}
		if (options & WDIOS_ENABLECARD) {
			m36wdt_start();
			retval = 0;
		}
		return retval;
	}
	case WDIOC_KEEPALIVE:
     {		
//        if (get_user(new_margin, p))
//			return -EFAULT;
//        wdt_count = new_margin;
// keeplive:  use the old time out
		m36wdt_ping();
		return 0;
     }
	case WDIOC_GETTIMEOUT:
		return put_user(WATCHDOG_TIMEOUT*1000000, (int *)arg);
        
    case WDIOC_SETTIMEOUT:  //start counting
    {
        if (get_user(new_margin, p))
			return -EFAULT;
        wdt_count = new_margin;
        m36wdt_start();
    }
    
    case WDIOC_GETTIMELEFT:
    {
        m36wdt_get_lefttime();
        return put_user(wdt_timeleft, (unsigned int*)arg);
    }

#if (!defined(CONFIG_ALI_CHIP_M3921))
	case WDIOC_WDT_REBOOT:
    {
        if (get_user(reboot_sec, p))
			return -EFAULT;
        /*
		if(!reboot_sec)
          wdt_reboot_from_nor(); 
        else
          wdt_reboot_from_nand(); 
        */
        if(board_is_nand_boot())
            wdt_reboot_from_nand();
        else
            wdt_reboot_from_nor();

		return 0;
    }
#endif
	default:
		return -ENOTTY;
	}
}

static int m36_notify_sys(struct notifier_block *this,
					unsigned long code, void *unused)
{
	if (code == SYS_DOWN || code == SYS_HALT)
		m36wdt_stop();		/* Turn the WDT off */

	return NOTIFY_DONE;
}

/* kernel interface */
static const struct file_operations m36wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= NULL, //m36wdt_write , it's recommanded to use ioctl
	.unlocked_ioctl	= m36wdt_ioctl,
	.open		= m36wdt_open,
	.release	= m36wdt_release,
};

static struct miscdevice m36wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &m36wdt_fops,
};

static struct notifier_block m36dog_notifier = {
	.notifier_call = m36_notify_sys,
};

static char banner[] __initdata =
	KERN_INFO PFX "Hardware Watchdog Timer for ALI M36 Platform\n";

static int __init watchdog_init(void)
{
	int ret;

	spin_lock_init(&wdt_lock);

	ret = register_reboot_notifier(&m36dog_notifier);
	if (ret) {
		printk(KERN_ERR PFX
			"cannot register reboot notifier (err=%d)\n", ret);
		return ret;
	}

	ret = misc_register(&m36wdt_miscdev);
	if (ret) {
		printk(KERN_ERR PFX
			"cannot register miscdev on minor=%d (err=%d)\n",
							WATCHDOG_MINOR, ret);
		unregister_reboot_notifier(&m36dog_notifier);
		return ret;
	}

	printk(banner);

	return 0;
}

static void __exit watchdog_exit(void)
{
	misc_deregister(&m36wdt_miscdev);
	unregister_reboot_notifier(&m36dog_notifier);
}

module_init(watchdog_init);
module_exit(watchdog_exit);
MODULE_DESCRIPTION("Hardware Watchdog Device for ALI M36 Platform");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
