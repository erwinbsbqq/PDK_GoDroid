/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: ali_stc.c
 *
 *  Description: ali system time clock
 *
 *  History:
 *      Date             Author         Version      Comment
 *      ======           ======          =====       =======
 *  1.  2011.09.14       Dylan.Yang     0.1.000     First version Created
 ****************************************************************************/
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vt.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/fb.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/version.h>

#include <linux/ali_transport.h>
#include <ali_reg.h>
#include <ali_stc_common.h>

#if 0
#define STC_PRF(arg, value...)  \
			{\
				printk(KERN_EMERG "kernel debug : file : %s function : %s line %d\n", __FILE__, __FUNCTION__, __LINE__);\
				printk(KERN_EMERG arg, ##value);\
				printk(KERN_EMERG "kernel debug done\n\n");\
			}
#else
#define STC_PRF(...) 	do{}while(0)
#endif

#define STC_CLOSED          0
#define STC_OPENED          1

#ifdef CONFIG_ALI_CHIP_M3921
#define STC_BASE_ADDR       0x18002000
#define STC0_DIVISOR        0xB0
#define STC0_CTRL           0xB2
#define STC0_VALUE          0xB4
#define STC1_DIVISOR        0xB8
#define STC1_CTRL           0xB2
#define STC1_VALUE          0xBC 
#else
#define STC_BASE_ADDR       0xb8002000
#define STC0_DIVISOR        0xE4
#define STC0_CTRL           0xE6
#define STC0_VALUE          0xE8
#define STC1_DIVISOR        0xEC
#define STC1_CTRL           0xEE
#define STC1_VALUE          0xF0
#endif

#define readb(addr) 					__REG8ALI(addr)
#define writeb(value, addr)  		__REG8ALI(addr) = value
#define readw(addr) 					__REG16ALI(addr)
#define writew(value, addr)  	__REG16ALI(addr) = value
#define readl(addr) 					__REG32ALI(addr)
#define writel(value, addr)  		__REG32ALI(addr) = value


/* Per-device (per-stc) structure */
struct stc_dev {
	struct cdev cdev;
	int stc_number;
	char name[10];
	int open_count;
	unsigned int stc_status;
	unsigned int stc_valid;
	unsigned int stc_divisor;

	unsigned int stc0_div_last;
	unsigned int stc1_div_last;
};

static struct semaphore m_stc_sem;

static int stc_pause(int stc_num)
{
	unsigned int stc_addr;
	unsigned char value;

	if(stc_num == 0) {
		stc_addr = STC_BASE_ADDR + STC0_CTRL;
	} else {
		stc_addr = STC_BASE_ADDR + STC1_CTRL;
	}

	value = readb(stc_addr);
	value |= (0x01<<stc_num);
	writeb(value, stc_addr);

	return 0;
}

static int stc_resume(int stc_num)
{
	unsigned int stc_addr;
	unsigned char value;

	if(stc_num == 0) {
		stc_addr = STC_BASE_ADDR + STC0_CTRL;
	} else {
		stc_addr = STC_BASE_ADDR + STC1_CTRL;
	}

	value = readb(stc_addr);
	value &= ~(0x01<<stc_num);
	writeb(value, stc_addr);

	return 0;
}

static int stc_set_divisor(int stc_num, unsigned int divisor)
{
	unsigned int stc_addr;

	if(stc_num == 0) {
		stc_addr = STC_BASE_ADDR + STC0_DIVISOR;
	} else {
		stc_addr = STC_BASE_ADDR + STC1_DIVISOR;
	}

	writew((unsigned short)divisor, stc_addr);

	return 0;
}

static int stc_get_divisor(int stc_num, unsigned int *divisor)
{
	unsigned int stc_addr;

	if(stc_num == 0) {
		stc_addr = STC_BASE_ADDR + STC0_DIVISOR;
	} else {
		stc_addr = STC_BASE_ADDR + STC1_DIVISOR;
	}

	*(unsigned short *)divisor = readw(stc_addr);

	return 0;
}

static int stc_open(struct inode *inode, struct file *file)
{
	struct stc_dev *stc_devp;

	if(down_interruptible(&m_stc_sem))
	{
		STC_PRF("ali stc down sem fail\n");
		return -EINVAL;	
	}

	/* Get the per-device structure that contains this cdev */
	stc_devp = container_of(inode->i_cdev, struct stc_dev, cdev);

	/* Easy access to stc_devp from rest of the entry points */
	file->private_data = stc_devp;

	if(stc_devp->open_count == 0) {
		/* Initialize some fields */
		stc_devp->stc_status = STC_OPENED;
		stc_devp->stc_divisor = 299;// 13499;
		stc_devp->stc_valid = 0;
		
		stc_get_divisor(0, &stc_devp->stc0_div_last);
		stc_get_divisor(1, &stc_devp->stc1_div_last);
		
		stc_set_divisor(0, stc_devp->stc_divisor);		
		stc_set_divisor(1, stc_devp->stc_divisor);   	
	}

	stc_devp->open_count++;

	STC_PRF("open count %d\n", stc_devp->open_count);

	up(&m_stc_sem);
	return 0;
}

static int stc_release(struct inode *inode, struct file *file)
{
	struct stc_dev *stc_devp = file->private_data;

	if(down_interruptible(&m_stc_sem))
	{
		STC_PRF("ali stc down sem fail\n");
		return -EINVAL;	
	}

	stc_devp->open_count--;

	if(stc_devp->open_count == 0) {
		stc_devp->stc_status = STC_CLOSED;
		stc_devp->stc_valid = 0;

		stc_set_divisor(0, stc_devp->stc0_div_last);
		stc_set_divisor(1, stc_devp->stc0_div_last); 		
	}

	up(&m_stc_sem);

	STC_PRF("open count %d\n", stc_devp->open_count);

	return 0;
}

static ssize_t stc_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct stc_dev *stc_devp = file->private_data;
	unsigned int stc_addr, stc_value = 0;

	if(down_interruptible(&m_stc_sem))
	{
		STC_PRF("ali stc down sem fail\n");
		return -EINVAL;	
	}

	if(count > sizeof(unsigned int) || !stc_devp->stc_valid) {
		up(&m_stc_sem);		
		return 0;
	}

	if(stc_devp->stc_number == 0) {
		stc_addr = STC_BASE_ADDR + STC0_VALUE;
	} else {
		stc_addr = STC_BASE_ADDR + STC1_VALUE;
	}

	stc_value = readl(stc_addr);

	STC_PRF("stc value %x\n", stc_value);

	copy_to_user((void *)buf, (void *)&stc_value, count);

	up(&m_stc_sem);

	return count;
}

static ssize_t stc_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	struct stc_dev *stc_devp = file->private_data;
	unsigned int stc_addr, stc_value = 0;

	if(down_interruptible(&m_stc_sem))
	{
		STC_PRF("ali stc down sem fail\n");
		return -EINVAL;	
	}

	if(count > sizeof(unsigned int)) {
		up(&m_stc_sem);		
		return 0;
	}

	copy_from_user((void *)&stc_value, (void *)buf, count);

	if(stc_devp->stc_number == 0) {
		stc_addr = STC_BASE_ADDR + STC0_VALUE;
	} else {
		stc_addr = STC_BASE_ADDR + STC1_VALUE;
	}

	STC_PRF("stc value %x\n", stc_value);

	stc_pause(stc_devp->stc_number);
	writel(stc_value, stc_addr);
	stc_resume(stc_devp->stc_number);

	up(&m_stc_sem);

	return count;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long stc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int stc_ioctl(struct inode *node, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	struct stc_dev *stc_devp = file->private_data;
	int ret = 0;

	if(down_interruptible(&m_stc_sem))
	{
		STC_PRF("ali stc down sem fail\n");
		return -EINVAL;	
	}

	switch(cmd) {
		case STCIO_SET_DIVISOR:
		{
			stc_devp->stc_divisor = arg;
			STC_PRF("stc divisor %d\n", arg);
			ret = stc_set_divisor(stc_devp->stc_number, stc_devp->stc_divisor);
			break;
		}
		case STCIO_SET_VALID:
		{
			stc_devp->stc_valid = arg;
			STC_PRF("stc valid %d\n", arg);
			break;
		}
		case STCIO_PAUSE_STC:
		{
			STC_PRF("stc pause %d\n", arg);
			if(arg) {
				ret = stc_pause(stc_devp->stc_number);
			} else {
				ret = stc_resume(stc_devp->stc_number);
			}
			break;
		}
		default:
		{
			ret = -ENOIOCTLCMD;
			break;
		}
	}

	up(&m_stc_sem);

	return ret;
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations stc_fops = {
    .owner    =   THIS_MODULE,     /* Owner */
    .open     =   stc_open,        /* Open method */
    .release  =   stc_release,     /* Release method */
    .read     =   stc_read,        /* Read method */
    .write    =   stc_write,       /* Write method */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = stc_ioctl,/* Ioctl method */
#else
    .ioctl    =   stc_ioctl,       /* Ioctl method */
#endif    
};

#define DEVICE_NAME                "ali_stc"

static dev_t stc_dev_number;       /* Allotted device number */
static struct class *stc_class;    /* Tie with the device model */
static struct device *stc_device;
static struct stc_dev *stc_priv[STC_NUM];

static int __init stc_init(void)
{
    int i, ret;

    /* Request dynamic allocation of a device major number */
    if (alloc_chrdev_region(&stc_dev_number, 0,
                            STC_NUM, DEVICE_NAME) < 0) {
        printk(KERN_DEBUG "Can't register device\n"); return -1;
    }

    /* Populate sysfs entries */
    stc_class = class_create(THIS_MODULE, DEVICE_NAME);
  
    for (i=0; i<STC_NUM; i++) {
        /* Allocate memory for the per-device structure */
        stc_priv[i] = kmalloc(sizeof(struct stc_dev), GFP_KERNEL);
        if (!stc_priv[i]) {
            printk("Bad Kmalloc\n"); return -ENOMEM;
        }
        memset(stc_priv[i], 0, sizeof(struct stc_dev));
    
        sprintf(stc_priv[i]->name, "ali_stc%d", i);
    
        /* Fill in the stc number to correlate this device
           with the corresponding stc */
        stc_priv[i]->stc_number = i;
    
        /* Connect the file operations with the cdev */
        cdev_init(&stc_priv[i]->cdev, &stc_fops);
        stc_priv[i]->cdev.owner = THIS_MODULE;
    
        /* Connect the major/minor number to the cdev */
        ret = cdev_add(&stc_priv[i]->cdev, (stc_dev_number + i), 1);
        if (ret) {
            printk("Bad cdev\n");
            return ret;
        }
    	
    	stc_device = device_create(stc_class, NULL, MKDEV(MAJOR(stc_dev_number), i), 
                                   NULL, "ali_stc%d", i);
    	if(stc_device == NULL) {
    		printk("stc create device fail\n");
    		return 1;
    	}
    }

    sema_init(&m_stc_sem, 1);

    return 0;
}

/* Driver Exit */
void __exit stc_exit(void)
{
    int i;
  
    /* Release the major number */
    unregister_chrdev_region(stc_dev_number, STC_NUM);

    for (i=0; i<STC_NUM; i++) {
        device_destroy(stc_class, MKDEV(MAJOR(stc_dev_number), i));
        /* Remove the cdev */
        cdev_del(&stc_priv[i]->cdev);
        kfree(stc_priv[i]);
    }
    
    /* Destroy cmos_class */
    class_destroy(stc_class);
    
    return;
}

module_init(stc_init);
module_exit(stc_exit);
 
MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("ali system time clock");
MODULE_LICENSE("GPL");

