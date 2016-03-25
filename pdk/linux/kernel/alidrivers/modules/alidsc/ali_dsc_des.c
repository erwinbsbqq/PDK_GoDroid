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

long ali_m36_des_ioctl(struct file *filp, __u32 cmd, unsigned long arg);
int ali_m36_des_open(struct inode *inode, struct file  *file);
int ali_m36_des_release(struct inode *inode, struct file  *file);


static struct ali_m36_des_dev g_ali_des_devices[1];

static struct file_operations g_ali_des_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = ali_m36_des_ioctl,
	.open = ali_m36_des_open,
	.release = ali_m36_des_release,
};

long ali_m36_des_ioctl
(
	struct file   *filp,
	__u32   cmd,
	unsigned long  arg
)
{
	int                       ret = 0;
	struct ali_m36_des_dev   *des_dev = NULL;
	struct dsc_see_dev_hld see_dev_hld; 
	ALI_DSC_IO_PARAM ioc_param;
	ALI_DSC_CRYPTION dsc_cryption;
	KEY_PARAM key_param;

	des_dev=(struct ali_m36_des_dev *)filp->private_data;
	if(des_dev == NULL || down_interruptible(des_dev->ioctl_sem))
	{
		ALI_DSC_DEBUG_ERR("Err: get ioctl_sem failed\n");
		return(-EINTR);
	}

	ALI_DSC_DEBUG("cmd = %x\n",cmd);

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
							&des_dev->see_des_id[see_dev_hld.dsc_dev_id], \
							sizeof(see_dev_hld.dsc_dev_hld));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
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
				ioc_param.dev = des_dev->see_des_id[(__u32)ioc_param.dev];
			}
			
			ret = ali_des_ioctl(ioc_param.dev, DSC_IO_CMD(cmd), \
							(__u32)ioc_param.ioc_param);
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
			
			if((__u32)ioc_param.dev < VIRTUAL_DEV_NUM)
			{
				ioc_param.dev = des_dev->see_des_id[(__u32)ioc_param.dev];
			}
			
			ret = ali_dsc_umemcpy(&key_param,ioc_param.ioc_param,sizeof(KEY_PARAM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			ret = ali_des_ioctl(ioc_param.dev, DSC_IO_CMD(cmd), (__u32)&key_param);
			if(RET_SUCCESS == ret)
			{
				ali_dsc_record_handle_id(filp, (__u32)key_param.handle, ALI_DSC_RESOURCE_BUSY);
			}
			else
			{
				ALI_DSC_DEBUG_ERR("Err:ali_des_ioctl() failed, ret:%d\n", ret);
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
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			if((__u32)dsc_cryption.dev < VIRTUAL_DEV_NUM)
			{
				dsc_cryption.dev = des_dev->see_des_id[(__u32)dsc_cryption.dev];
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
				ret = ali_dsc_umemcpy(des_dev->kernel_input_buffer, \
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
				ret = ali_dsc_umemcpy(des_dev->kernel_input_buffer, \
									(void __user *)(dsc_cryption.input), \
									(dsc_cryption.length));
			}
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			if(DSC_IO_CMD(ALI_DSC_IO_DECRYPT) == DSC_IO_CMD(cmd))
			{
				ret = ali_des_decrypt((DES_DEV *)dsc_cryption.dev, \
								dsc_cryption.stream_id, des_dev->kernel_input_buffer, \
								des_dev->kernel_output_buffer, dsc_cryption.length);
			}
			else
			{
				ret = ali_des_encrypt((DES_DEV *)dsc_cryption.dev, \
								dsc_cryption.stream_id, des_dev->kernel_input_buffer, \
								des_dev->kernel_output_buffer, dsc_cryption.length);
			}
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_des crypt failed, ret=%d, cmd=%d\n", ret, cmd);
				goto DONE;
			}

			if( (dsc_cryption.stream_id<=3) || \
				(dsc_cryption.stream_id >=8 && dsc_cryption.stream_id<=0xf) )
			{
				ret = ali_dsc_umemcpy((void __user *)(dsc_cryption.output), \
									des_dev->kernel_output_buffer, \
									(dsc_cryption.length*TS_PACKET_SIZE));
			}
			else
			{
				ret = ali_dsc_umemcpy((void __user *)(dsc_cryption.output), \
									des_dev->kernel_output_buffer, \
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
	up(des_dev->ioctl_sem);
	return(ret);
}



int ali_m36_des_open
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_des_dev    *des = NULL;
	
	des = container_of(inode->i_cdev, struct ali_m36_des_dev, cdev);
	if (NULL == des ||  down_interruptible(des->ioctl_sem))
	{
		ALI_DSC_DEBUG_ERR("Err: get ioctl_sem failed\n");
		return(-EINTR);
	}

	file->private_data = (void *)des;
	
	up(des->ioctl_sem);

	return(0);
}


int ali_m36_des_release
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_des_dev *des = NULL;
	struct ali_m36_dsc_dev *dsc = NULL;
	__u32 i = 0;
	
	des = file->private_data;
	//des = container_of(inode->i_cdev, struct ali_m36_des_dev, cdev);

	if(NULL == des)
	{
		ALI_DSC_DEBUG_ERR("Err: get des dev failed\n");
		return(-EIO);
	}
	
	dsc = (struct ali_m36_dsc_dev *)des->g_dev;
		
	down(des->ioctl_sem);
		
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
	up(des->ioctl_sem);

	return(0);
}

static int __init ali_m36_des_init
(
    void
)
{
	int result = -1;
	__u32 i = 0;
	struct device  *clsdev = NULL;
	struct ali_m36_des_dev *des = &g_ali_des_devices[0];
	struct ali_m36_dsc_dev *dsc = &g_ali_dsc_devices[0];
	struct class *ali_des_class = NULL;

	memset(des, 0x00, sizeof(struct ali_m36_des_dev));

	des->ioctl_sem = &dsc->ioctl_sem;
	des->g_dev = (void *)dsc;

	result = alloc_chrdev_region(&des->devt, 0, 1, ALI_DES_DEV);

	if (result < 0) 
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error\n");
		goto fail;
	}

	cdev_init(&(des->cdev), &g_ali_des_fops);

	des->cdev.owner = THIS_MODULE;

	result = cdev_add(&des->cdev, des->devt, 1);
	if (result < 0)
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error:cdev_add() failed,result:%d\n", result);
		goto fail;
	}

	ali_des_class = class_create(THIS_MODULE, "ali_des_class");
	if (IS_ERR(ali_des_class))
	{
		result = PTR_ERR(ali_des_class);
		ALI_DSC_DEBUG_ERR("Dsc kernel error\n");
		goto fail;
	}
	des->dev_class = ali_des_class;

	clsdev = device_create(ali_des_class, NULL, des->devt, des, ALI_DES_DEV);
	if (IS_ERR(clsdev))
	{
		ALI_DSC_DEBUG_ERR("ali_des_0 device_create() failed!\n");
		result = PTR_ERR(clsdev);
		goto fail;
	}

	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{	
		des->see_des_id[i] = hld_dev_get_by_id(HLD_DEV_TYPE_DES, i);
		if (NULL == des->see_des_id[i])
		{
			ALI_DSC_DEBUG_ERR("Dsc kernel error\n");
			return(-ENXIO);
		}
	}
	des->kernel_input_buffer = dsc->kernel_input_buffer;
	if ((NULL == des->kernel_input_buffer))
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error:DSC not been initialized\n");
		return(-ENOMEM);
	}
	des->kernel_output_buffer = dsc->kernel_output_buffer;
	if ((NULL == des->kernel_output_buffer))
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error:DSC not been initialized\n");
		return(-ENOMEM);
	}
	return(0);

fail:
	return(-1);
}


static void __exit ali_m36_des_exit(void)
{
	struct ali_m36_des_dev *des = g_ali_des_devices;
	ALI_DSC_DEBUG("DES kernel exit\n");

	unregister_chrdev_region(des->devt, 1);
	device_destroy(des->dev_class, des->devt);
	cdev_del(&des->cdev);    
	class_destroy(des->dev_class);
}

late_initcall_sync(ali_m36_des_init);
module_exit(ali_m36_des_exit);

MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("DES/TDES in DSC");
MODULE_LICENSE("GPL");

