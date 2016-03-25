#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/semaphore.h>

#include "ali_dsc_lld.h"

long ali_m36_aes_ioctl(struct file *filp, __u32 cmd, unsigned long arg);

int ali_m36_aes_open(struct inode *inode, struct file  *file);

int ali_m36_aes_release(struct inode *inode, struct file  *file);


static struct ali_m36_aes_dev g_ali_aes_devices[1];

static struct file_operations g_ali_aes_fops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = ali_m36_aes_ioctl,
    .open = ali_m36_aes_open,
    .release = ali_m36_aes_release,
};

long ali_m36_aes_ioctl
(
    struct file *filp,
    __u32 cmd,
    unsigned long  arg
)
{
	int  ret = 0;
	struct ali_m36_aes_dev   *aes_dev = NULL;
	struct dsc_see_dev_hld see_dev_hld; 
	ALI_DSC_IO_PARAM ioc_param;
	ALI_DSC_CRYPTION dsc_cryption;
	KEY_PARAM key_param;
	
	aes_dev=(struct ali_m36_aes_dev *)filp->private_data;
	if(NULL == aes_dev || down_interruptible(aes_dev->ioctl_sem))
	{
		ALI_DSC_DEBUG("Err: get ioctl_sem failed\n");
		return(-EINTR);
	}

	ALI_DSC_DEBUG("filp=0x%x, cmd = 0x%x\n",(__u32)filp,cmd);
	
	switch(DSC_IO_CMD(cmd))
	{
		case DSC_IO_CMD(IO_GET_DEV_HLD):
		{
			ret = ali_dsc_umemcpy(&see_dev_hld, (void __user *)arg, \
								sizeof(struct dsc_see_dev_hld));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			if(see_dev_hld.dsc_dev_id >= VIRTUAL_DEV_NUM)
			{
				ALI_DSC_DEBUG_ERR("Err:dev_id out of bound, ID:%d\n", see_dev_hld.dsc_dev_id);
				ret = ALI_DSC_ERROR_INVALID_PARAMETERS;
				goto DONE;
			}
			
			ret = ali_dsc_umemcpy((void __user *)arg, \
							&aes_dev->see_aes_id[see_dev_hld.dsc_dev_id], \
							sizeof(see_dev_hld.dsc_dev_hld));
			if (0 != ret)
			{
				ALI_DSC_DEBUG("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			break;
		}
		
		case DSC_IO_CMD(IO_INIT_CMD):
		case DSC_IO_CMD(IO_DELETE_CRYPT_STREAM_CMD):
		{
			ret = ali_dsc_umemcpy(&ioc_param, (void __user *)arg, \
									sizeof(ALI_DSC_IO_PARAM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			if((__u32)ioc_param.dev < VIRTUAL_DEV_NUM)
			{
				ioc_param.dev = aes_dev->see_aes_id[(__u32)ioc_param.dev];
			}
			ret = ali_aes_ioctl(ioc_param.dev, DSC_IO_CMD(cmd),(__u32)ioc_param.ioc_param);
			if((DSC_IO_CMD(IO_DELETE_CRYPT_STREAM_CMD) == DSC_IO_CMD(cmd)) \
				&& (RET_SUCCESS == ret))
			{
				ali_dsc_record_handle_id(filp, (__u32)ioc_param.ioc_param, ALI_DSC_RESOURCE_IDLE);
			}
			break;
		}
		
		case DSC_IO_CMD(IO_KEY_INFO_UPDATE_CMD):
		case DSC_IO_CMD(IO_CREAT_CRYPT_STREAM_CMD):
		{
			ret = ali_dsc_umemcpy(&ioc_param, (void __user *)arg, \
									sizeof(ALI_DSC_IO_PARAM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			ret = ali_dsc_umemcpy(&key_param,ioc_param.ioc_param,sizeof(KEY_PARAM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			if((__u32)ioc_param.dev < VIRTUAL_DEV_NUM)
			{
				ioc_param.dev = aes_dev->see_aes_id[(__u32)ioc_param.dev];
			}
			ret = ali_aes_ioctl(ioc_param.dev, DSC_IO_CMD(cmd), (__u32)&key_param);
			if(RET_SUCCESS == ret)
			{
				ali_dsc_record_handle_id(filp, (__u32)key_param.handle, ALI_DSC_RESOURCE_BUSY);
			}
			else
			{
				ALI_DSC_DEBUG_ERR("Err:ali_aes_ioctl() failed, ret:%d\n", ret);
				goto DONE;
			}

			ret = ali_dsc_umemcpy(ioc_param.ioc_param, &key_param.handle, sizeof(key_param.handle));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			break;
		}
		
		case DSC_IO_CMD(ALI_DSC_IO_DECRYPT):
		case DSC_IO_CMD(ALI_DSC_IO_ENCRYPT):
		{
			ret = ali_dsc_umemcpy((void *)&dsc_cryption, (void __user *)arg, \
								sizeof(ALI_DSC_CRYPTION));    
			if (0 != ret)
			{
				ALI_DSC_DEBUG("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			if( (dsc_cryption.stream_id<=3) || \
				(dsc_cryption.stream_id >=8 && dsc_cryption.stream_id<=0xf) )
			{
				if(dsc_cryption.length*TS_PACKET_SIZE>PURE_DATA_MAX_SIZE)
				{
					ALI_DSC_DEBUG_ERR("Err: length outside\n");
					ret = ALI_DSC_ERROR_INVALID_PARAMETERS;
					goto DONE;
				}
				ret = ali_dsc_umemcpy(aes_dev->kernel_input_buffer, \
									(void __user *)(dsc_cryption.input), \
									(dsc_cryption.length*TS_PACKET_SIZE));            
			}
			else
			{
				if(dsc_cryption.length>PURE_DATA_MAX_SIZE)
				{
					ALI_DSC_DEBUG_ERR("Err: length outside\n");
					ret = ALI_DSC_ERROR_INVALID_PARAMETERS;
					goto DONE;
				}
				ret = ali_dsc_umemcpy(aes_dev->kernel_input_buffer, \
									(void __user *)(dsc_cryption.input), \
									(dsc_cryption.length));
			}
			if (0 != ret)
			{
				ALI_DSC_DEBUG("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			if((__u32)dsc_cryption.dev < VIRTUAL_DEV_NUM)
			{
				dsc_cryption.dev = aes_dev->see_aes_id[(__u32)dsc_cryption.dev];
			}
			
			if(DSC_IO_CMD(ALI_DSC_IO_DECRYPT) == DSC_IO_CMD(cmd))
			{
				ret = ali_aes_decrypt((AES_DEV *)dsc_cryption.dev, \
								dsc_cryption.stream_id, aes_dev->kernel_input_buffer, \
								aes_dev->kernel_output_buffer, dsc_cryption.length);
			}
			else
			{
				ret = ali_aes_encrypt((AES_DEV *)dsc_cryption.dev, \
								dsc_cryption.stream_id, aes_dev->kernel_input_buffer, \
								aes_dev->kernel_output_buffer, dsc_cryption.length);
			}
			if (0 != ret)
			{
				ALI_DSC_DEBUG("Err:ali_aes crypt failed, ret=%d, cmd=%d\n", ret, cmd);
				goto DONE;
			}

			if( (dsc_cryption.stream_id<=3) || \
				(dsc_cryption.stream_id >=8 && dsc_cryption.stream_id<=0xf) )
			{
				ret = ali_dsc_umemcpy((void __user *)(dsc_cryption.output), \
									aes_dev->kernel_output_buffer, \
									(dsc_cryption.length*TS_PACKET_SIZE));
			}
			else
			{
				ret = ali_dsc_umemcpy((void __user *)(dsc_cryption.output), \
									aes_dev->kernel_output_buffer, \
									(dsc_cryption.length)); 
			}

			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				ret = -ENOTTY;
			}   
			break;
		}
		
		default:
		{
			ALI_DSC_DEBUG_ERR("Err:Parameter Error, cmd=0x%x\n",cmd);
			ret = ALI_DSC_ERROR_OPERATION_NOT_SUPPORTED;
			break;
		}
	}

DONE:
	up(aes_dev->ioctl_sem);
	return(ret);
}



int ali_m36_aes_open
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_aes_dev *aes = NULL;

	aes = container_of(inode->i_cdev, struct ali_m36_aes_dev, cdev);

	if (NULL == aes || down_interruptible(aes->ioctl_sem))
	{
		ALI_DSC_DEBUG("Err: get ioctl_sem failed\n");
		return(-EINTR);
	}

	file->private_data = (void *)aes;

//	ALI_DSC_DEBUG("file=0x%08X\n",(__u32)file);
	up(aes->ioctl_sem);

	return(0);
}



int ali_m36_aes_release
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_aes_dev *aes = NULL;
	struct ali_m36_dsc_dev *dsc = NULL;
	__u32 i = 0;
	
	aes = file->private_data;
	//aes = container_of(inode->i_cdev, struct ali_m36_aes_dev, cdev);

	if(NULL == aes)
	{
		ALI_DSC_DEBUG("Err: get aes dev failed\n");
		return(-EIO);
	}
	
	dsc = (struct ali_m36_dsc_dev *)aes->g_dev;

	down(aes->ioctl_sem);
	
	for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
	{
		if(dsc->rec[i].handle_id.filp == (__u32)file)
		{
			ali_dsc_ioctl(dsc->see_dsc_id, IO_DSC_DELETE_HANDLE_CMD, \
						(__u32)dsc->rec[i].handle_id.handle);
			dsc->rec[i].handle_id.filp = 0;
			ALI_DSC_DEBUG_ERR("dsc->rec[%d].handle_id.handle = %d\n",i,dsc->rec[i].handle_id.handle);
		}
	}

	file->private_data = NULL;
	up(aes->ioctl_sem);

	return(0);
}


static int __init ali_m36_aes_init
(
    void
)
{
	int	result = -1;
	__u32 i = 0;
	struct device	*clsdev = NULL;
	struct ali_m36_aes_dev *aes = &g_ali_aes_devices[0];
	struct ali_m36_dsc_dev *dsc = &g_ali_dsc_devices[0];
	struct class *ali_aes_class = NULL;

	memset(aes, 0, sizeof(struct ali_m36_aes_dev));

	aes->ioctl_sem = &dsc->ioctl_sem;

	aes->g_dev = (void *)dsc;

	result = alloc_chrdev_region(&aes->devt, 0, 1, ALI_AES_DEV);
	if (result < 0) 
	{
		ALI_DSC_DEBUG("Dsc kernel error\n");
		goto fail;
	}
	cdev_init(&(aes->cdev), &g_ali_aes_fops);

	aes->cdev.owner = THIS_MODULE;

	result = cdev_add(&aes->cdev, aes->devt, 1);
	if (result < 0)
	{
		ALI_DSC_DEBUG("Dsc kernel error:cdev_add() failed,result:%d\n", result);
		goto fail;
	}

	ali_aes_class = class_create(THIS_MODULE, "ali_aes_class");
	if (IS_ERR(ali_aes_class))
	{
		result = PTR_ERR(ali_aes_class);
		goto fail;
	}
	aes->dev_class = ali_aes_class;

	clsdev = device_create(ali_aes_class, NULL, aes->devt, aes, ALI_AES_DEV);
	if (IS_ERR(clsdev))
	{
		ALI_DSC_DEBUG("ali_aes_0 device_create() failed!\n");
		result = PTR_ERR(clsdev);
		goto fail;
	}

	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		aes->see_aes_id[i] = hld_dev_get_by_id(HLD_DEV_TYPE_AES, i);           
		if (NULL == aes->see_aes_id[i])
		{
			ALI_DSC_DEBUG_ERR("Dsc kernel error\n");
			return (ALI_DSC_ERROR_INVALID_DEV);
		}
	}
	
	aes->kernel_input_buffer = dsc->kernel_input_buffer;
	if ((NULL == aes->kernel_input_buffer))
	{
		ALI_DSC_DEBUG("Dsc kernel error:DSC not been initialized\n");
		return(-ENOMEM);
	}
	
	aes->kernel_output_buffer = dsc->kernel_output_buffer;
	if ((NULL == aes->kernel_output_buffer))
	{
		ALI_DSC_DEBUG("Dsc kernel error:DSC not been initialized\n");
		return(-ENOMEM);
	}

	return(0);
fail:    
	return(-1);
}


static void __exit ali_m36_aes_exit(void)
{
	struct ali_m36_aes_dev *aes = g_ali_aes_devices;
	ALI_DSC_DEBUG("AES kernel exit\n");

	unregister_chrdev_region(aes->devt, 1);
	device_destroy(aes->dev_class, aes->devt);
	cdev_del(&aes->cdev);    
	class_destroy(aes->dev_class);

}

late_initcall_sync(ali_m36_aes_init);
module_exit(ali_m36_aes_exit);

MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("AES in DSC");
MODULE_LICENSE("GPL");
