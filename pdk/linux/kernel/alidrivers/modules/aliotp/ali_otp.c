#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <ali_otp_common.h>
#include "ali_otp.h"


#if 1
#define ALI_OTP_DEBUG(...)  do{}while(0)
#else
#define ALI_OTP_DEBUG printk
#endif


struct class *g_ali_otp_class;


__s32 ali_otp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

__s32 ali_otp_open(struct inode *inode, struct file  *file);

__s32 ali_otp_release(struct inode *inode, struct file  *file);

struct file_operations g_ali_otp_fops = 
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = ali_otp_ioctl,
    .open = ali_otp_open,
    .release = ali_otp_release,
};

struct ali_otp_dev g_ali_otp_device ;


__s32 ali_otp_ioctl
(

    struct file   *filp,
    unsigned int   cmd,
    unsigned long  arg
)
{
    __s32                       ret;
    unsigned long               paras[MAX_ALI_OTP_PARAS];
   
    ret = 0;

    ret = copy_from_user(&paras, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        return(-EFAULT);
    }

    switch(cmd) 
    {
        case ALI_OTP_READ:
        {
            struct otp_read_paras *par = (struct otp_read_paras *)paras;        
            ALI_OTP_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

                ret = ali_otp_read(par->offset, par->buf, par->len);
        }
        break;

        case ALI_OTP_WRITE:
        {
          	struct otp_write_paras *par = (struct otp_write_paras *)paras;
            ALI_OTP_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

           	    ret = ali_otp_write(par->buf, par->offset, par->len);
        }
        break;
        default:
        {
            ALI_OTP_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);
            ret = -EINVAL;
        }
        break;
    }

    return(ret);
}



__s32 ali_otp_open
(

   struct inode *inode,
    struct file  *file
)
{

    ALI_OTP_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    return(0);
}



__s32 ali_otp_release
(
    struct inode *inode,
    struct file  *file
)
{
    int                         ret;
    ALI_OTP_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);
    ret = 0; 
    return(ret);
}

static int __init ali_otp_init
(
    void
)
{
    int            			result;
    struct device          *clsdev;
    struct ali_otp_dev     *otp;

    ALI_OTP_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    /* Enable CSA module in SEE by PRC. */
    //ali_m36_csa_see_init();

    otp = &g_ali_otp_device;

    mutex_init(&otp->ioctl_mutex);
    
    result = alloc_chrdev_region(&otp->dev_id, 0, 1, "ali_otp");

    if (result < 0) 
    {
        ALI_OTP_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        goto fail;
    }

    ALI_OTP_DEBUG("%s, dev_id:%d.\n", __FUNCTION__, otp->dev_id);

    cdev_init(&(otp->cdev), &g_ali_otp_fops);

    otp->cdev.owner = THIS_MODULE;

    result = cdev_add(&otp->cdev, otp->dev_id, 1);

    /* Fail gracefully if need be. */
    if (result)
    {
        ALI_OTP_DEBUG("cdev_add() failed, result:%d\n", result);

        goto fail;
    }

    g_ali_otp_class = class_create(THIS_MODULE, "ali_otp_class");

    if (IS_ERR(g_ali_otp_class))
    {
        result = PTR_ERR(g_ali_otp_class);

        goto fail;
    }

    ALI_OTP_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    clsdev = device_create(g_ali_otp_class, NULL, otp->dev_id, 
                           otp, "ali_otp");

    if (IS_ERR(clsdev))
    {
        ALI_OTP_DEBUG(KERN_ERR "device_create() failed!\n");

        result = PTR_ERR(clsdev);

        goto fail;
    }

    ali_otp_hw_init();

    #ifdef CONFIG_ALI_CHIP_M3912
		ali_otp_read(0x84 * 4, (unsigned char *)(&result), 4);
		ALI_OTP_DEBUG("[ %s, %d ], Chip ID = 0x%08x, [0x84] = 0x%08x\n", 
			__FUNCTION__, __LINE__, *(unsigned int *)0xB8000000, result);		
		if ((0x00200000 != result) && (0x00200038 != result))		
		{			
			ALI_OTP_DEBUG("[ %s %d ], reboot!\n", __FUNCTION__, __LINE__);			
			kernel_restart(NULL);					
		}		
    #endif

	#ifdef CONFIG_ALI_CHIP_M3901C
		ali_otp_read(0x84 * 4, (unsigned char *)(&result), 4);
		ALI_OTP_DEBUG("[ %s, %d ], Chip ID = 0x%08x, [0x84] = 0x%08x\n", 
			__FUNCTION__, __LINE__, *(unsigned int *)0xB8000000, result);		
		
		if ((0x00300000 != result) && (0x00300038 != result))		
		{			
			ALI_OTP_DEBUG("[ %s %d ], reboot!\n", __FUNCTION__, __LINE__);			
			kernel_restart(NULL);				
		}		
    #endif
    
    ALI_OTP_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    return(0);

fail:
    return(-1);
}



static void __exit ali_otp_exit(void)
{
    printk("%s\n", __FUNCTION__);
}

module_init(ali_otp_init);
module_exit(ali_otp_exit);


