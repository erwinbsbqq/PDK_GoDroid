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
#include <asm/uaccess.h>

#include "ali_trng_lld.h"

long ali_m36_trng_ioctl(struct file *filp, __u32 cmd, unsigned long arg);

int ali_m36_trng_open(struct inode *inode, struct file  *file);

int ali_m36_trng_release(struct inode *inode, struct file  *file);


extern int ali_trng_generate_byte( __u8 *data );
extern int ali_trng_generate_64bits( __u8 *data );
extern int ali_trng_get_64bits(__u8 *data, __u32 n);

static struct ali_m36_trng_dev g_ali_trng_devices[1];

struct file_operations g_ali_trng_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = ali_m36_trng_ioctl,
	.open = ali_m36_trng_open,
	.release = ali_m36_trng_release,
};

long ali_m36_trng_ioctl
(
	struct file   *filp,
	__u32   cmd,
	unsigned long  arg
)
{
	int                       ret = RET_FAILURE;
	struct ali_m36_trng_dev   *trng_dev = NULL;
	ALI_TRNG_GET_64BITS trng_group;

	trng_dev=(struct ali_m36_trng_dev *)filp->private_data;

	if(NULL == trng_dev || down_interruptible(&trng_dev->ioctl_sem))
	{
		ALI_TRNG_DEBUG_ERR("Err: get ioctl_sem failed\n");
		return(-EBUSY);
	}

	switch(TRNG_IO_CMD(cmd)) 
	{
		case TRNG_IO_CMD(ALI_TRNG_GENERATE_BYTE):
		{
			ret = ali_trng_generate_byte(trng_dev->kernel_output_buffer);
			if (0 != ret)
			{
				ALI_TRNG_DEBUG("Err:ali_trng_generate_byte() failed, ret:%d\n", ret);
				goto DONE;
			}			
			ret = copy_to_user((void __user *)(arg), \
							trng_dev->kernel_output_buffer, \
							sizeof(char)); 
			if (0 != ret)
			{
				ALI_TRNG_DEBUG_ERR("Err:copy_to_user() failed, ret:%d\n", ret);
				goto DONE;
			}      
			break;
		}
				
		case TRNG_IO_CMD(ALI_TRNG_GENERATE_64bits):
		{
			ret = ali_trng_generate_64bits(trng_dev->kernel_output_buffer);
			if (0 != ret)
			{
				ALI_TRNG_DEBUG_ERR("Err:ali_trng_generate_64bits() failed, ret:%d\n", ret);
				goto DONE;
			}			
			ret = copy_to_user((void __user *)(arg), \
						trng_dev->kernel_output_buffer, \
						ALI_TRNG_64BITS_SIZE); 
			if (0 != ret)
			{
				ALI_TRNG_DEBUG_ERR("Err:copy_to_user() failed, ret:%d\n", ret);
				goto DONE;
			}
			break;
		}
		
		case TRNG_IO_CMD(ALI_TRNG_GET_64bits):
		{
			ret = copy_from_user((void *)&trng_group, \
						(void __user *)arg, \
						sizeof(ALI_TRNG_GET_64BITS));
			if (0 != ret)
			{
				ALI_TRNG_DEBUG_ERR("Err:copy_from_user() failed, ret:%d\n", ret);
				goto DONE;
			}
			if (ALI_TRNG_64BITS_SIZE*((__u32)trng_group.n_group) > sizeof(trng_dev->kernel_output_buffer))
			{
				ALI_TRNG_DEBUG_ERR("Err:requested size outof bounds\n");
				ret = EINVAL;
				goto DONE;
			}
			
			ret = ali_trng_get_64bits(trng_dev->kernel_output_buffer,(__u32)trng_group.n_group);
			if (0 != ret)
			{
				ALI_TRNG_DEBUG_ERR("Err:ali_trng_get_64bits() failed, ret:%d\n", ret);
				goto DONE;
			}  
			ret = copy_to_user((void __user *)(trng_group.data), \
							trng_dev->kernel_output_buffer, \
							ALI_TRNG_64BITS_SIZE*((__u32)trng_group.n_group)); 
			if (0 != ret)
			{
				ALI_TRNG_DEBUG_ERR("Err:copy_to_user() failed, ret:%d\n", ret);
				goto DONE;
			}  
			break;  
		}
		      
		default:
		{
			ALI_TRNG_DEBUG_ERR("ioctl not supported\n");
			ret = -EINVAL;
			goto DONE;
		}
	}

	ret = RET_SUCCESS;

DONE:
	up(&trng_dev->ioctl_sem);
	return(ret);
}


int ali_m36_trng_open
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_trng_dev     *trng;

	trng = container_of(inode->i_cdev, struct ali_m36_trng_dev, cdev);

	if (NULL == trng || down_interruptible(&trng->ioctl_sem))
	{
		ALI_TRNG_DEBUG_ERR("Err: get ioctl_sem failed\n");
		return(-EBUSY);
	}

	file->private_data = trng;

	up(&trng->ioctl_sem);

	return(0);
}


int ali_m36_trng_release
(
	struct inode *inode,
	struct file  *file
)
{
	struct ali_m36_trng_dev *trng = NULL;

	trng = file->private_data;
	//trng = container_of(inode->i_cdev, struct ali_m36_trng_dev, cdev);

	if (NULL == trng || down_interruptible(&trng->ioctl_sem))
	{
		return(-EBUSY);
	}

	file->private_data = NULL;
	up(&trng->ioctl_sem);

	return(0);
}


static int __init ali_m36_trng_init
(
	void
)
{
	int            result = -1;
	struct device          *clsdev = NULL;
	struct ali_m36_trng_dev *trng = &g_ali_trng_devices[0];
	struct class *ali_trng_class = NULL;
 
	memset(trng, 0, sizeof(struct ali_m36_trng_dev));

	sema_init(&trng->ioctl_sem, 1);

	result = alloc_chrdev_region(&trng->devt, 0, 1, ALI_TRNG_DEV);

	if (result < 0) 
	{
		ALI_TRNG_DEBUG_ERR("Err TRNG\n");
		goto fail;
	}

	cdev_init(&(trng->cdev), &g_ali_trng_fops);

	trng->cdev.owner = THIS_MODULE;

	result = cdev_add(&trng->cdev, trng->devt, 1);
	if (result < 0)
	{
		ALI_TRNG_DEBUG_ERR("cdev_add() failed, result:%d\n", result);
		goto fail;
	}

	ali_trng_class = class_create(THIS_MODULE, "ali_trng_class");
	if (IS_ERR(ali_trng_class))
	{
		result = PTR_ERR(ali_trng_class);
		goto fail;
	}
	trng->dev_class = ali_trng_class;

	clsdev = device_create(ali_trng_class, NULL, trng->devt, trng, ALI_TRNG_DEV);
	if (IS_ERR(clsdev))
	{
		ALI_TRNG_DEBUG_ERR("ali_trng_0 device_create() failed!\n");
		result = PTR_ERR(clsdev);
		goto fail;
	}
	
	return(0);

fail:
	return(-1);
}


static void __exit ali_m36_trng_exit(void)
{
	struct ali_m36_trng_dev *trng = g_ali_trng_devices;
	ALI_TRNG_DEBUG("TRNG kernel exit\n");

	unregister_chrdev_region(trng->devt, 1);
	device_destroy(trng->dev_class, trng->devt);
	cdev_del(&trng->cdev);    
	class_destroy(trng->dev_class);
}

late_initcall_sync(ali_m36_trng_init);
module_exit(ali_m36_trng_exit);

MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("True Random Data Generator");
MODULE_LICENSE("GPL");

