/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: ali_fake.c
 *
 *  Description: fake interface for KFT
 *
 *  History:
 *      Date             Author         Version      Comment
 *      ======           ======          =====       =======
 *  1.  2011.09.14       Owen.Zhao     0.1.000     First version Created
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
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/version.h>

#include <linux/kft.h>
#include <linux/dvb/ali_fake.h>

#define DEVICE_NAME                "fake_trace"

struct fake_trace_dev {
    struct cdev cdev;
    char name[32];
    int used;
};

static dev_t fake_trace_dev_number;       /* Allotted device number */
static struct class *fake_trace_class;    /* Tie with the device model */
static struct device *fake_trace_device;
static struct fake_trace_dev *fake_trace_priv = NULL;

static int fake_trace_open(struct inode *inode, struct file *file)
{
    struct fake_trace_dev *fake_trace_devp;
  
    /* Get the per-device structure that contains this cdev */
    fake_trace_devp = container_of(inode->i_cdev, struct fake_trace_dev, cdev);

    /* Easy access to fake_trace_devp from rest of the entry points */
    file->private_data = fake_trace_devp;

    if (fake_trace_devp->used) {
    	   printk("Error: fake trace is used\n");
        return -EPERM;
    }
  	fake_trace_devp->used = 1;
  
    return 0;
}

static int fake_trace_release(struct inode *inode, struct file *file)
{
    struct fake_trace_dev *fake_trace_devp = file->private_data;

    fake_trace_devp->used = 0;
  
    return 0;
}

static ssize_t fake_trace_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    return 0;
}

static ssize_t fake_trace_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    return 0;
}

static const char stat_nam[] = "RSDTtZX";

static void fake_trace_show_stack(struct task_struct *p)
{
    unsigned state;
    printk("fake_trace_show_stack entered\n");

    state = p->state ? __ffs(p->state) + 1 : 0;
    printk("%-13.13s %c", p->comm,
        state < sizeof(stat_nam) - 1 ? stat_nam[state] : '?');
#if (BITS_PER_LONG == 32)
    if (state == TASK_RUNNING)
        printk(" running ");
    else
        printk(" %08lX ", thread_saved_pc(p));
#else
    if (state == TASK_RUNNING)
        printk("  running task   ");
    else
        printk(" %016lx ", thread_saved_pc(p));
#endif
    printk("\n");
    if (state != TASK_RUNNING)
        show_stack(p, NULL);
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static int fake_trace_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int fake_trace_ioctl(struct inode *node, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    //struct fake_trace_dev *fake_trace_devp = file->private_data;
    int ret = 0;

    struct task_struct *g, *p;
    int pid_t; 

    switch(cmd) {
        case FAKE_TRACE_SHOW_MEMORY:
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 38))
            show_mem();
#else
            show_mem(0);
#endif
            break;
    	case FAKE_TRACE_GET_TICK:
    	{
    	    put_user((read_32bit_cp0_register($9) / US_TICKS), \
    		     (unsigned long __user *)arg);
    	    break;	
    	}

        case FAKE_TRACE_KFT_START:
        {
            break;
	    }

        case FAKE_TRACE_SHOW_STACK_ALL:
        {
            read_lock(&tasklist_lock);
            do_each_thread(g, p) {
                fake_trace_show_stack(p);
                mdelay(1000);/* Flush the uart buffer */
            } while_each_thread(g, p);
            read_unlock(&tasklist_lock);
            break;
        }
        
	    case FAKE_TRACE_SHOW_STACK_PID:
        {
            int found = 0;
            if (copy_from_user(&pid_t, (int *)arg,  sizeof(int)))
                return -EFAULT;
            read_lock(&tasklist_lock);
            do_each_thread(g, p) {
                if(p->tgid == pid_t)
                {
                    found = 1;
                    fake_trace_show_stack(p);
                }
            } while_each_thread(g, p);
            if (!found) printk("Not a valid task PID.\n");
            read_unlock(&tasklist_lock);
            break;
        }
        
        case FAKE_TRACE_KFT_STOP:
        {
#ifdef CONFIG_KFT
        	kft_to_userspace();
#endif
            break;
	    }

        default:
        {
            break;
        }
    }

    return ret;
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations fake_trace_fops = {
    .owner    =   THIS_MODULE,     /* Owner */
    .open     =   fake_trace_open,        /* Open method */
    .release  =   fake_trace_release,     /* Release method */
    .read     =   fake_trace_read,        /* Read method */
    .write    =   fake_trace_write,       /* Write method */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
     .unlocked_ioctl = fake_trace_ioctl,       /* Ioctl method */
#else
    .ioctl    =   fake_trace_ioctl,       /* Ioctl method */
#endif    
};

static int __init fake_trace_init(void)
{
    int ret;

    /* Request dynamic allocation of a device major number */
    if (alloc_chrdev_region(&fake_trace_dev_number, 0,
                            1, DEVICE_NAME) < 0) {
        printk(KERN_DEBUG "Can't register device\n"); 
        return -1;
    }

    /* Populate sysfs entries */
    fake_trace_class = class_create(THIS_MODULE, DEVICE_NAME);
  
    fake_trace_priv = kmalloc(sizeof(struct fake_trace_dev), GFP_KERNEL);
    if (!fake_trace_priv) {
        printk("Bad Kmalloc\n"); 
        return -ENOMEM;
    }
    memset(fake_trace_priv, 0, sizeof(struct fake_trace_dev));
    strcpy(fake_trace_priv->name, "ali_fake_trace");

    /* Connect the file operations with the cdev */
    cdev_init(&fake_trace_priv->cdev, &fake_trace_fops);
    fake_trace_priv->cdev.owner = THIS_MODULE;

    /* Connect the major/minor number to the cdev */
    ret = cdev_add(&fake_trace_priv->cdev, fake_trace_dev_number, 1);
    if (ret) {
        printk("Bad cdev\n");
        return ret;
    }
	
	  fake_trace_device = device_create(fake_trace_class, NULL, 
	  																	MKDEV(MAJOR(fake_trace_dev_number), 0), 
                               				NULL, "ali_fake_trace");
	  if(fake_trace_device == NULL) {
		    printk("fake trace create device fail\n");
		    return 1;
	  }
  
    return 0;
}

/* Driver Exit */
void __exit fake_trace_exit(void)
{  
    /* Release the major number */
    unregister_chrdev_region(fake_trace_dev_number, 1);

    device_destroy(fake_trace_class, MKDEV(MAJOR(fake_trace_dev_number), 0));
    /* Remove the cdev */
    cdev_del(&fake_trace_priv->cdev);
    kfree(fake_trace_priv);
    
    /* Destroy cmos_class */
    class_destroy(fake_trace_class);
    
    return;
}

module_init(fake_trace_init);
module_exit(fake_trace_exit);
 
MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("ali fake trace control");
MODULE_LICENSE("GPL");

