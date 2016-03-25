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


long ali_m36_csa_ioctl(struct file *filp, __u32 cmd, unsigned long arg);

int ali_m36_csa_open(struct inode *inode, struct file  *file);

int ali_m36_csa_release(struct inode *inode, struct file  *file);


static struct ali_m36_csa_dev g_ali_csa_devices[1];

static struct file_operations g_ali_csa_fops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = ali_m36_csa_ioctl,
    .open = ali_m36_csa_open,
    .release = ali_m36_csa_release,
};

long ali_m36_csa_ioctl
(
    struct file   *filp,
    __u32   cmd,
    unsigned long  arg
)
{
	int  ret = 0;
	struct ali_m36_csa_dev   *csa_dev = NULL;
	struct dsc_see_dev_hld see_dev_hld; 
	ALI_DSC_IO_PARAM ioc_param;
	ALI_DSC_CRYPTION dsc_cryption;
	KEY_PARAM key_param;

	csa_dev=(struct ali_m36_csa_dev *)filp->private_data;
	if(NULL == csa_dev || down_interruptible(csa_dev->ioctl_sem))
	{
		ALI_DSC_DEBUG("Err: get ioctl_sem failed\n");
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
				ALI_DSC_DEBUG("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			if(see_dev_hld.dsc_dev_id >= VIRTUAL_DEV_NUM)
			{
				ALI_DSC_DEBUG_ERR("Err:dev_id out of bound, ID:%d\n", see_dev_hld.dsc_dev_id);
				ret = ALI_DSC_ERROR_INVALID_PARAMETERS;
				goto DONE;
			}

			ret = ali_dsc_umemcpy((void __user *)arg, &csa_dev->see_csa_id[see_dev_hld.dsc_dev_id],
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
				ioc_param.dev = csa_dev->see_csa_id[(__u32)ioc_param.dev];
			}
			ret = ali_csa_ioctl(ioc_param.dev, DSC_IO_CMD(cmd), \
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
				ioc_param.dev = csa_dev->see_csa_id[(__u32)ioc_param.dev];
			}
			ret = ali_dsc_umemcpy(&key_param,ioc_param.ioc_param,sizeof(KEY_PARAM));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			ret = ali_csa_ioctl(ioc_param.dev, DSC_IO_CMD(cmd), (__u32)&key_param);
			if(RET_SUCCESS == ret)
			{
				ali_dsc_record_handle_id(filp, (__u32)key_param.handle, ALI_DSC_RESOURCE_BUSY);
			}
			else
			{
				ALI_DSC_DEBUG_ERR("Err:ali_csa_ioctl() failed, ret:%d\n", ret);
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
				dsc_cryption.dev = csa_dev->see_csa_id[(__u32)dsc_cryption.dev];
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

				ret = ali_dsc_umemcpy(csa_dev->kernel_input_buffer, \
									(void __user *)(dsc_cryption.input), \
									(dsc_cryption.length*TS_PACKET_SIZE));      
			}
			else
			{
				if(dsc_cryption.length>PURE_DATA_MAX_SIZE)
				{
					ALI_DSC_DEBUG("Err: length outside\n");
					ret = ALI_DSC_ERROR_INVALID_PARAMETERS;
					goto DONE;
				}
				ret = ali_dsc_umemcpy(csa_dev->kernel_input_buffer, \
									(void __user *)(dsc_cryption.input), \
									(dsc_cryption.length));
			}
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}

			ret = ali_csa_decrypt((pCSA_DEV)dsc_cryption.dev, \
								dsc_cryption.stream_id, csa_dev->kernel_input_buffer, \
								csa_dev->kernel_output_buffer, dsc_cryption.length);
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_csa_decrypt() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			if( (dsc_cryption.stream_id<=3) || \
				(dsc_cryption.stream_id >=8 && dsc_cryption.stream_id<=0xf) )
			{
				ret = ali_dsc_umemcpy((void __user *)(dsc_cryption.output), 
									csa_dev->kernel_output_buffer, \
									(dsc_cryption.length*TS_PACKET_SIZE));
			}
			else
			{
				ret = ali_dsc_umemcpy((void __user *)(dsc_cryption.output), \
									csa_dev->kernel_output_buffer, \
									(dsc_cryption.length)); 
			}

			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
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
	up(csa_dev->ioctl_sem);
	return(ret);
}



int ali_m36_csa_open
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_csa_dev *csa = NULL;

	csa = container_of(inode->i_cdev, struct ali_m36_csa_dev, cdev);
	if (NULL == csa || down_interruptible(csa->ioctl_sem))
	{
		ALI_DSC_DEBUG("Err: get ioctl_sem failed\n");
		return(-EINTR);
	}

	file->private_data = (void *)csa;
	
	up(csa->ioctl_sem);

	return(0);
}



int ali_m36_csa_release
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_csa_dev *csa = NULL;
	struct ali_m36_dsc_dev *dsc = NULL;
	__u32 i = 0;
	
	csa = file->private_data;
	//csa = container_of(inode->i_cdev, struct ali_m36_csa_dev, cdev);

	if(NULL == csa)
	{
		ALI_DSC_DEBUG("Err: get csa dev failed\n");
		return(-EIO);
	}
	
	dsc = (struct ali_m36_dsc_dev *)csa->g_dev;
		
	down(csa->ioctl_sem);

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
	up(csa->ioctl_sem);

	return(0);
}

static int __init ali_m36_csa_init
(
	void
)
{
	int result = -1;
	__u32 i = 0;
	struct device          *clsdev = NULL;
	struct ali_m36_csa_dev *csa = &g_ali_csa_devices[0];
	struct ali_m36_dsc_dev *dsc = &g_ali_dsc_devices[0];
	struct class *ali_csa_class = NULL;
	
	memset(csa, 0, sizeof(struct ali_m36_csa_dev));

	csa->ioctl_sem = &dsc->ioctl_sem;
	csa->g_dev = (void *)dsc;
	
	result = alloc_chrdev_region(&csa->devt, 0, 1, ALI_CSA_DEV);
	if (result < 0) 
	{
		ALI_DSC_DEBUG("Dsc kernel error\n");
		goto fail;
	}

	cdev_init(&(csa->cdev), &g_ali_csa_fops);

	csa->cdev.owner = THIS_MODULE;

	result = cdev_add(&csa->cdev, csa->devt, 1);
	if (result < 0)
	{
		ALI_DSC_DEBUG("Dsc kernel error:cdev_add() failed,result:%d\n", result);
		goto fail;
	}

	ali_csa_class = class_create(THIS_MODULE, "ali_csa_class");
	if (IS_ERR(ali_csa_class))
	{
		result = PTR_ERR(ali_csa_class);
		goto fail;
	}
	csa->dev_class = ali_csa_class;

	clsdev = device_create(ali_csa_class, NULL, csa->devt, csa, ALI_CSA_DEV);
	if (IS_ERR(clsdev))
	{
		ALI_DSC_DEBUG("ali_csa_0 device_create() failed!\n");
		result = PTR_ERR(clsdev);
		goto fail;
	}

	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		if (NULL == csa->see_csa_id[i])
		{
			csa->see_csa_id[i] = hld_dev_get_by_id(HLD_DEV_TYPE_CSA, i);
			if (NULL == csa->see_csa_id[i])
			{
				ALI_DSC_DEBUG("Dsc kernel error\n");
				return(-ENXIO);
			}
		}
	}
	csa->kernel_input_buffer = dsc->kernel_input_buffer;
	if ((NULL == csa->kernel_input_buffer))
	{
		ALI_DSC_DEBUG("Dsc kernel error:DSC not been initialized\n");
		return(-ENOMEM);
	}
	csa->kernel_output_buffer = dsc->kernel_output_buffer;
	if ((NULL == csa->kernel_output_buffer))
	{
		ALI_DSC_DEBUG("Dsc kernel error:DSC not been initialized\n");
		return(-ENOMEM);
	}
	return(0);

fail:
	return(-1);
}


static void __exit ali_m36_csa_exit(void)
{
	struct ali_m36_csa_dev *csa = g_ali_csa_devices;
	ALI_DSC_DEBUG("CSA kernel exit\n");

	unregister_chrdev_region(csa->devt, 1);
	device_destroy(csa->dev_class, csa->devt);
	cdev_del(&csa->cdev);    
	class_destroy(csa->dev_class);

}

late_initcall_sync(ali_m36_csa_init);
module_exit(ali_m36_csa_exit);

MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("CSA in DSC");
MODULE_LICENSE("GPL");
