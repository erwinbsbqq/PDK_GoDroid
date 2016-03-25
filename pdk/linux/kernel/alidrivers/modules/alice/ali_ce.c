#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/semaphore.h>

#include "ali_ce_lld.h"

long ali_m36_ce_ioctl(struct file *filp, __u32 cmd, unsigned long arg);

int ali_m36_ce_open(struct inode *inode, struct file  *file);

int ali_m36_ce_release(struct inode *inode, struct file  *file);

static struct ali_m36_ce_dev g_ali_ce_devices[1];

struct file_operations g_ali_ce_fops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = ali_m36_ce_ioctl,
    .open = ali_m36_ce_open,
    .release = ali_m36_ce_release,
};

static void ali_ce_record_key_pos(struct file *filp, __u32 pos, __u32 cmd)
{
	struct ali_m36_ce_dev *dev = (struct ali_m36_ce_dev *)filp->private_data;
	__u32 i = 0;
	__u32 pos_exist = 0;

	if(pos >= ALI_CE_REC_MAX_ITEM)
	{
		return;
	}

	for(i=0;i<ALI_CE_REC_MAX_ITEM;i++)
	{
		if(dev->rec[i].pos == (__u32)pos)
		{
			dev->rec[i].filp = (__u32)filp;
			pos_exist = 1;
			break;
		}
	}
	
	if(ALI_CE_RESOURCE_BUSY == cmd)
	{
		if(pos_exist)
		{
			return;
		}
		
		for(i=0;i<ALI_CE_REC_MAX_ITEM;i++)
		{
			if(!dev->rec[i].filp)
			{
				dev->rec[i].filp = (__u32)filp;
				dev->rec[i].pos = pos;
				break;
			}
		}
	}
	else if(ALI_CE_RESOURCE_IDLE == cmd)
	{
		if(i<ALI_CE_REC_MAX_ITEM)
		{
			dev->rec[i].filp = 0;
		}
	}
}


long ali_m36_ce_ioctl
(
    struct file   *filp,
    __u32   cmd,
    unsigned long  arg
)
{
	int                       ret = RET_SUCCESS;
	struct ali_m36_ce_dev   *ce_dev = NULL;
	ALI_CE_GEN_CW ce_gen_cw;
	__u8 cipher_cw[16];
	CE_FOUND_FREE_POS_PARAM kpos_param;
	
	ce_dev = (struct ali_m36_ce_dev *)filp->private_data;

	if(NULL == ce_dev || down_interruptible(&ce_dev->ioctl_sem))
	{
		ALI_CE_DEBUG("get ioctl_sem failed\n");
		return(-EINTR);
	}

	ALI_CE_DEBUG("filp=0x%p, cmd = 0x%x\n",filp,cmd);
	
	switch(CE_IO_CMD(cmd)) 
	{
		case CE_IO_CMD(IO_OTP_ROOT_KEY_GET):
		case CE_IO_CMD(IO_CRYPT_DEBUG_GET_KEY):
		case CE_IO_CMD(IO_CRYPT_POS_IS_OCCUPY):
		case CE_IO_CMD(IO_DECRYPT_PVR_USER_KEY):
		{
			ret = ali_ce_ioctl(ce_dev->see_ce_id, CE_IO_CMD(cmd), (__u32)arg);
			break;
		}
		
		case CE_IO_CMD(IO_CRYPT_FOUND_FREE_POS):
		{
			ret = ali_ce_umemcpy((void *)&kpos_param, (void __user *)arg, \
									sizeof(CE_FOUND_FREE_POS_PARAM));
			if (0 != ret)
			{
				ALI_CE_DEBUG("Err:ali_ce_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			ret = ali_ce_ioctl(ce_dev->see_ce_id, (CE_IO_CMD(cmd)), (__u32)&kpos_param);
			if (0 != ret)
			{
				ALI_CE_DEBUG("Err:ali_ce_ioctl() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			ali_ce_record_key_pos(filp, (__u32)(kpos_param.pos), ALI_CE_RESOURCE_BUSY);
			if(2 == kpos_param.number)
			{
				ali_ce_record_key_pos(filp, (__u32)(kpos_param.pos + 1), ALI_CE_RESOURCE_BUSY);
			}
			
			ret = ali_ce_umemcpy((void __user *)arg, (void *)&kpos_param.pos, \
									sizeof(kpos_param.pos));
			if (0 != ret)
			{
				ALI_CE_DEBUG("Err:ali_ce_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			break;
		}
		
		case CE_IO_CMD(IO_CRYPT_POS_SET_USED):
		case CE_IO_CMD(IO_CRYPT_POS_SET_IDLE):
		{
			ret = ali_ce_ioctl(ce_dev->see_ce_id, CE_IO_CMD(cmd), (__u32)arg);
			if(0 == ret)
			{
				ali_ce_record_key_pos(filp, (__u32)arg, \
				(CE_IO_CMD(IO_CRYPT_POS_SET_IDLE)==CE_IO_CMD(cmd))?ALI_CE_RESOURCE_IDLE:ALI_CE_RESOURCE_BUSY);
			}
			break;
		}
		
		case CE_IO_CMD(IO_CE_KEY_GENERATE):	 
		{
			ret = ali_ce_key_generate((pCE_DEVICE)ce_dev->see_ce_id, (pCE_DATA_INFO)arg);
			break;
		}
		
		case CE_IO_CMD(IO_CE_KEY_LOAD):
		{
			ret = ali_ce_key_load((pCE_DEVICE)ce_dev->see_ce_id, (pOTP_PARAM)arg);    
			break;
		}
				
		case CE_IO_CMD(IO_CE_GENERATE_CW_KEY):  
		{
			ret = ali_ce_umemcpy((void *)&ce_gen_cw, (void __user *)arg, \
									sizeof(ALI_CE_GEN_CW));
			if (0 != ret)
			{
				ALI_CE_DEBUG("Err:ali_ce_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			ret = ali_ce_umemcpy((void *)cipher_cw, (void __user *)ce_gen_cw.in, \
									sizeof(cipher_cw));
			if (0 != ret)
			{
				ALI_CE_DEBUG("Err:ali_ce_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			ret = ali_ce_generate_cw_key(cipher_cw, \
										(__u8)ce_gen_cw.aes_or_des, \
										(__u8)ce_gen_cw.lowlev_pos, \
										(__u8)ce_gen_cw.highlev_pos);
			break;
		}
		
		case CE_IO_CMD(IO_CE_GENERATE_SINGLE_LEVEL_KEY):
		{
			ret = ali_ce_generate_single_level_key((pCE_DEVICE)ce_dev->see_ce_id, \
											 	(pCE_DATA_INFO)arg);
			break;
		}
		
		case CE_IO_CMD(IO_CE_GET_DEV_HLD):
		{
			ret = ali_ce_umemcpy((void __user *)arg, &ce_dev->see_ce_id, \
								sizeof(ce_dev->see_ce_id)); 
			if (0 != ret)
			{
				ALI_CE_DEBUG("Err:ali_ce_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			break;
		}
		 
		default:
		{
			ALI_CE_DEBUG("CE kernel error:Error parameter, cmd=0x%x\n",cmd);
			ret = -EINVAL;
			break;
		}
		
	}
	
DONE:	
	up(&ce_dev->ioctl_sem);
	return(ret);
}



int ali_m36_ce_open
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_ce_dev *ce = NULL;

	ce = container_of(inode->i_cdev, struct ali_m36_ce_dev, cdev);

	if (NULL == ce || down_interruptible(&ce->ioctl_sem))
	{
		ALI_CE_DEBUG("get ioctl_sem failed\n");
		return(-EINTR);
	}

	file->private_data = (void *)ce;

	up(&ce->ioctl_sem);

	return(0);
}


int ali_m36_ce_release
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_ce_dev *dev = NULL;
	__u32 i = 0;

	dev = file->private_data;
	//ce = container_of(inode->i_cdev, struct ali_m36_ce_dev, cdev);

	if(NULL == dev)
	{
		ALI_CE_DEBUG("Err: get ce dev failed\n");
		return(-EIO);
	}

	down(&dev->ioctl_sem);

	for(i=0;i<ALI_CE_REC_MAX_ITEM;i++)
	{
		if(dev->rec[i].filp == (__u32)file)
		{
			ali_ce_ioctl(dev->see_ce_id, IO_CRYPT_POS_SET_IDLE, (__u32)dev->rec[i].pos);
			ALI_CE_DEBUG_ERR("dev->rec[%d].pos = %d\n",i,dev->rec[i].pos);
			dev->rec[i].filp = 0;
		}
	}

	file->private_data = NULL;
	up(&dev->ioctl_sem);
	return(0);
}


static int __init ali_m36_ce_init
(
    void
)
{
	int            result = -1;
	struct device  *clsdev = NULL;
	struct ali_m36_ce_dev *ce = &g_ali_ce_devices[0];
	struct class *ali_ce_class = NULL;

	memset(ce, 0, sizeof(struct ali_m36_ce_dev));

	sema_init(&ce->ioctl_sem, 1);

	result = alloc_chrdev_region(&ce->devt, 0, 1, ALI_CE_DEV);
	if (result < 0) 
	{
		ALI_CE_DEBUG("CE kernel error\n");
		goto fail;
	}

	cdev_init(&(ce->cdev), &g_ali_ce_fops);

	ce->cdev.owner = THIS_MODULE;

	result = cdev_add(&ce->cdev, ce->devt, 1);
	if (result < 0)
	{
		ALI_CE_DEBUG("CE kernel error:cdev_add(), result:%d\n", result);
		goto fail;
	}
	
	ali_ce_class = class_create(THIS_MODULE, "ali_ce_class");
	if (IS_ERR(ali_ce_class))
	{
		result = PTR_ERR(ali_ce_class);
		ALI_CE_DEBUG("CE kernel error:class_create(), result:%d\n", result);
		goto fail;
	}
	ce->dev_class = ali_ce_class;

	clsdev = device_create(ali_ce_class, NULL, ce->devt, ce, ALI_CE_DEV);

	if (IS_ERR(clsdev))
	{
		ALI_CE_DEBUG("ali_ce_0 device_create() failed!\n");
		result = PTR_ERR(clsdev);
		goto fail;
	}
	
	//open see module for ce
	ali_m36_ce_see_init();

	ce->see_ce_id = hld_dev_get_by_type(NULL,HLD_DEV_TYPE_CE);
	if (NULL == ce->see_ce_id)
	{
		ALI_CE_DEBUG("CE kernel error\n");
		return(-ENXIO);
	}
	
	return(0);

fail:
	return(-1);
}



static void __exit ali_m36_ce_exit(void)
{
	struct ali_m36_ce_dev *ce = g_ali_ce_devices;
	ALI_CE_DEBUG("CE kernel exit\n");

	unregister_chrdev_region(ce->devt, 1);
	device_destroy(ce->dev_class, ce->devt);
	cdev_del(&ce->cdev);    
	class_destroy(ce->dev_class);
}

device_initcall(ali_m36_ce_init);
module_exit(ali_m36_ce_exit);

MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("Key Ladder for Advanced Security");
MODULE_LICENSE("GPL");

