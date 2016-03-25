/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_pm.c
 *  (I)
 *  Description: ALi power management implementation
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2011.03.07				Owen			Creation
 ****************************************************************************/

#include "ali_pm.h"
//#include "ali_suspend.h"


extern void ali_suspend_register_ops(void);
extern void ali_suspend_set_resume_key(pm_key_t *pm_key);
extern void ali_suspend_set_standby_param(pm_param_t *p_standby_param);

static int ali_pm_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int ali_pm_release(struct inode *inode, struct file *file)
{
    return 0;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_pm_ioctl(struct file *file,
			            unsigned int cmd, unsigned long parg)
#else
static int ali_pm_ioctl(struct inode *inode, struct file *file,
			            unsigned int cmd, unsigned long parg)
#endif			            
{
	unsigned int temp,ret;
	pm_key_t pm_resume_key;
	pm_key_t *p_key = (pm_key_t *)parg;
	pm_param_t pm_standby_param;
	pm_param_t *p_param = (pm_param_t *)parg;

	temp = PM_CMD_SET_RESUME_KEY;
	printk(" PM_CMD_SET_RESUME_KEY:%x\n",temp);

	temp = PM_CMD_SET_STANDBY_PARAM;
	printk(" PM_CMD_SET_STANDBY_PARAM:%x\n",temp);

	printk("ali_pm_ioctl cmd:%x\n",cmd);
    switch (cmd)
    {
        case PM_CMD_SET_RESUME_KEY:        
        {
		printk("ali_pm_ioctl PM_CMD_SET_RESUME_KEY\n");

            if (NULL == p_key) return -EINVAL;
            get_user(pm_resume_key.standby_key, &p_key->standby_key);
		//get_user(pm_resume_key.ir_power, &p_key->ir_power);
		ret = copy_from_user(pm_resume_key.ir_power, p_key->ir_power, 8*sizeof(unsigned long));
		if (0 != ret)
		{
		    printk("PM_CMD_SET_RESUME_KEY copy_from_user() failed, ret:%d\n", ret);
		    return(-EINVAL);
		}
            ali_suspend_set_resume_key(&pm_resume_key);
            
            break;
        }
        case PM_CMD_SET_STANDBY_PARAM:
        {
		printk("ali_pm_ioctl PM_CMD_SET_STANDBY_PARAM\n");

            if (NULL == p_key) return -EINVAL;
            get_user(pm_standby_param.board_power_gpio, &p_param->board_power_gpio);
            get_user(pm_standby_param.timeout, &p_param->timeout);
            get_user(pm_standby_param.reboot, &p_param->reboot);
            ali_suspend_set_standby_param(&pm_standby_param);
            break;
        }
        default:
            break;
    }
    return 0;
}

static struct file_operations ali_pm_fops = {
	.owner		= THIS_MODULE,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_pm_ioctl,
#else
	.ioctl		= ali_pm_ioctl,
#endif	
	.open		= ali_pm_open,
	.release	= ali_pm_release,
};

static struct pm_device pm_dev = {
    .dev_name = "ali_pm",
};

static int __init ali_pm_init(void)
{
    int ret = 0;
    
    ret = alloc_chrdev_region(&pm_dev.dev_no, 0, 1, pm_dev.dev_name);
    if (ret < 0)
        return ret;

    pm_dev.pm_class = class_create(THIS_MODULE, "ali_pm_class");
    if (IS_ERR(pm_dev.pm_class))
	{
		ret = PTR_ERR(pm_dev.pm_class);
        goto err0;
	}

    cdev_init(&pm_dev.pm_cdev, &ali_pm_fops);
	ret = cdev_add(&pm_dev.pm_cdev, pm_dev.dev_no, 1);
    if (ret < 0) goto err1;

    pm_dev.pm_device_node = device_create(pm_dev.pm_class, NULL, \
                                          pm_dev.dev_no, &pm_dev, 
                                          pm_dev.dev_name);
    if (IS_ERR(pm_dev.pm_device_node))
    {
		ret = PTR_ERR(pm_dev.pm_device_node);
		goto err2;
	}
    
	ali_suspend_register_ops();
	return 0;

err2:
    cdev_del(&pm_dev.pm_cdev);
err1:
    class_destroy(pm_dev.pm_class);
err0:
    unregister_chrdev_region(pm_dev.dev_no, 1);
    return ret;
}

static void __exit ali_pm_exit(void)
{
	return;
}

subsys_initcall(ali_pm_init);
module_exit(ali_pm_exit);
 
MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("ALi power management implementation");
MODULE_LICENSE("GPL");

