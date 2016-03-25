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

long ali_m36_sha_ioctl(struct file *filp, __u32 cmd, unsigned long arg);

int ali_m36_sha_open(struct inode *inode, struct file  *file);

int ali_m36_sha_release(struct inode *inode, struct file  *file);



static struct ali_m36_sha_dev g_ali_sha_devices[1];

static struct file_operations g_ali_sha_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = ali_m36_sha_ioctl,
	.open = ali_m36_sha_open,
	.release = ali_m36_sha_release,
};

long ali_m36_sha_ioctl
(
    struct file   *filp,
    __u32   cmd,
    unsigned long  arg
)
{
	int                       ret=0,i=0;
	struct ali_m36_sha_dev   *sha_dev = NULL;
	struct dsc_see_dev_hld see_dev_hld; 
	SHA_INIT_PARAM sha_param;
	__u32 sha_outlen = 0;
	ALI_DSC_IO_PARAM ioc_param;
	ALI_SHA_DIGEST sha_digest;

	sha_dev = (struct ali_m36_sha_dev *)filp->private_data;
	if(NULL == sha_dev)
	{
		ALI_DSC_DEBUG("Err: get sha_dev failed\n");
		return(-EIO);
	}

	ALI_DSC_DEBUG("cmd = %x\n",cmd);
	
	switch(DSC_IO_CMD(cmd))
	{
		case DSC_IO_CMD(IO_GET_DEV_HLD):
		{
			ret = ali_dsc_umemcpy(&see_dev_hld, (void __user *)arg, sizeof(struct dsc_see_dev_hld));
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
							&sha_dev->see_sha_id[see_dev_hld.dsc_dev_id], \
							sizeof(see_dev_hld.dsc_dev_hld)); 
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}  
			break;
		}
		

		case DSC_IO_CMD(IO_INIT_CMD):
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
				ioc_param.dev = sha_dev->see_sha_id[(__u32)ioc_param.dev];
			}
			ret = ali_dsc_umemcpy(&sha_param,  (void __user *)(ioc_param.ioc_param), sizeof(SHA_INIT_PARAM));            
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE;
			}
			
			if(down_interruptible(sha_dev->ioctl_sem))
			{
				ALI_DSC_DEBUG_ERR("Err: get ioctl_sem failed\n");
				return(-EINTR);
			}
		
			for(i=0;i<VIRTUAL_DEV_NUM;i++)
			{
				if((__u32)ioc_param.dev == (__u32)sha_dev->see_sha_id[i])
				{
					sha_dev->sha_dma_mode[i] = sha_param.sha_work_mode; 
					if( (sha_param.sha_buf & (1 << 31)) && !(sha_param.sha_buf & 0x3) )// align 4bytes kernel space
					{
						sha_dev->sha_ex_buf[i] = sha_param.sha_buf ;
					}
					else
					{
						sha_dev->sha_ex_buf[i] = (__u32)sha_dev->kernel_input_buffer;
					}
					break;
				}
			}		
			
			up(sha_dev->ioctl_sem);

			ret = ali_sha_ioctl(ioc_param.dev, DSC_IO_CMD(cmd), (__u32)(&sha_param));
			break;
		}
		
		case DSC_IO_CMD(ALI_DSC_IO_SHA_DIGEST):
		{	
			if(down_interruptible(sha_dev->ioctl_sem))
			{
				ALI_DSC_DEBUG_ERR("Err: get ioctl_sem failed\n");
				return(-EINTR);
			}
			
			ret = ali_dsc_umemcpy(&sha_digest, (void __user *)arg, \
									sizeof(ALI_SHA_DIGEST));
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE_ERR;
			}

			if(sha_digest.length>PURE_DATA_MAX_SIZE)
			{
				ALI_DSC_DEBUG_ERR("Err: length outside\n");
				ret = ALI_DSC_ERROR_INVALID_PARAMETERS;
				goto DONE_ERR;
			}

			if((__u32)sha_digest.dev < VIRTUAL_DEV_NUM)
			{
				sha_digest.dev = sha_dev->see_sha_id[(__u32)sha_digest.dev];
			}
			for(i=0;i<VIRTUAL_DEV_NUM;i++)
			{
				if((__u32)sha_digest.dev == (__u32)sha_dev->see_sha_id[i])
				{
					break;
				}
			}
			
			if(i >= VIRTUAL_DEV_NUM)
			{
				ret = -EINVAL;
                		goto DONE_ERR;
			}

			ret = ali_dsc_umemcpy((void *)sha_dev->sha_ex_buf[i], \
							(void __user *)(sha_digest.input), \
							(sha_digest.length));
			if (0 != ret)
			{
				ALI_DSC_DEBUG("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE_ERR;
			}

			ret = ali_sha_digest(sha_dev->see_sha_id[i], \
							  	(__u8*)sha_dev->sha_ex_buf[i], \
								sha_dev->kernel_output_buffer, \
								sha_digest.length);
			if(0 != ret)
			{
				ALI_DSC_DEBUG("Err:ali_sha_digest(), ret:%d\n", ret);
				goto DONE_ERR;
			}
			sha_dev->sha_ex_buf[i] = (__u32)sha_dev->kernel_input_buffer;

			if(SHA_SHA_1 == sha_dev->sha_dma_mode[i])
			{
				sha_outlen = SHA1_HASH_SIZE;
			}
			else if(SHA_SHA_224 == sha_dev->sha_dma_mode[i])
			{
				sha_outlen = SHA224_HASH_SIZE;
			}
			else if(SHA_SHA_256 == sha_dev->sha_dma_mode[i])
			{
				sha_outlen = SHA256_HASH_SIZE;
			}
			else if(SHA_SHA_384 == sha_dev->sha_dma_mode[i])
			{
				sha_outlen = SHA384_HASH_SIZE;
			}
			else if(SHA_SHA_512 == sha_dev->sha_dma_mode[i])
			{
				sha_outlen = SHA512_HASH_SIZE;
			}
			else
			{
				sha_outlen = SHA256_HASH_SIZE;
			}

			ret = ali_dsc_umemcpy((void __user *)(sha_digest.output), \
							sha_dev->kernel_output_buffer, \
							sha_outlen);
			if (0 != ret)
			{
				ALI_DSC_DEBUG_ERR("Err:ali_dsc_umemcpy() failed, ret:%d\n", ret);
				goto DONE_ERR;
			}     
			
		DONE_ERR:
			up(sha_dev->ioctl_sem);	
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
	return(ret);
}


int ali_m36_sha_open
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_sha_dev *sha = NULL;

	sha = container_of(inode->i_cdev, struct ali_m36_sha_dev, cdev);

	if (NULL == sha || down_interruptible(sha->ioctl_sem))
	{
		ALI_DSC_DEBUG_ERR("Err: get ioctl_sem failed\n");
		return(-EINTR);
	}

	file->private_data = (void *)sha;

	up(sha->ioctl_sem);

	return(0);
}


int ali_m36_sha_release
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_sha_dev *sha = NULL;

	sha = file->private_data;
	//sha = container_of(inode->i_cdev, struct ali_m36_sha_dev, cdev);

	if(NULL == sha)
	{
		ALI_DSC_DEBUG_ERR("Err: get sha dev failed\n");
		return(-EIO);
	}

	down(sha->ioctl_sem);
		
	file->private_data = NULL;

	up(sha->ioctl_sem);

	return(0);
}


static int __init ali_m36_sha_init
(
	void
)
{
	int  result = -1;
	__u32 i = 0;
	struct device          *clsdev = NULL;
	struct ali_m36_sha_dev *sha = &g_ali_sha_devices[0];
	struct ali_m36_dsc_dev *dsc = &g_ali_dsc_devices[0];
	struct class *ali_sha_class = NULL;

	memset(sha, 0, sizeof(struct ali_m36_sha_dev));

	sha->ioctl_sem = &dsc->ioctl_sem;

	result = alloc_chrdev_region(&sha->devt, 0, 1, ALI_SHA_DEV);

	if (result < 0) 
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error\n");
		goto fail;
	}

	cdev_init(&(sha->cdev), &g_ali_sha_fops);

	sha->cdev.owner = THIS_MODULE;

	result = cdev_add(&sha->cdev, sha->devt, 1);
	if (result < 0)
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error:cdev_add() failed,result:%d\n", result);
		goto fail;
	}

	ali_sha_class = class_create(THIS_MODULE, "ali_sha_class");
	if (IS_ERR(ali_sha_class))
	{
		result = PTR_ERR(ali_sha_class);
		goto fail;
	}
	sha->dev_class = ali_sha_class;

	clsdev = device_create(ali_sha_class, NULL, sha->devt, sha, ALI_SHA_DEV);
	if (IS_ERR(clsdev))
	{
		ALI_DSC_DEBUG_ERR("ali_sha_0 device_create() failed!\n");
		result = PTR_ERR(clsdev);
		goto fail;
	}

	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		sha->see_sha_id[i] = hld_dev_get_by_id(HLD_DEV_TYPE_SHA, i);
		if (NULL == sha->see_sha_id[i])
		{
			ALI_DSC_DEBUG_ERR("Dsc kernel error\n");
			return(-ENXIO);
		}
		sha->sha_dma_mode[i]=SHA_SHA_256;
	}

	sha->kernel_input_buffer = dsc->kernel_input_buffer;
	
	if ((NULL == sha->kernel_input_buffer))
	{
		ALI_DSC_DEBUG_ERR("Dsc kernel error:DSC not been initialized\n");
		return(-ENOMEM);
	}

	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		sha->sha_ex_buf[i] = (__u32)sha->kernel_input_buffer;
	}

	return(0);

fail:
	return(-1);
}



static void __exit ali_m36_sha_exit(void)
{
	struct ali_m36_sha_dev *sha = g_ali_sha_devices;
	ALI_DSC_DEBUG("SHA kernel exit\n");

	unregister_chrdev_region(sha->devt, 1);
	device_destroy(sha->dev_class, sha->devt);
	cdev_del(&sha->cdev);    
	class_destroy(sha->dev_class);
}

late_initcall_sync(ali_m36_sha_init);
module_exit(ali_m36_sha_exit);

MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("SHA in DSC, HW HASH");
MODULE_LICENSE("GPL");

