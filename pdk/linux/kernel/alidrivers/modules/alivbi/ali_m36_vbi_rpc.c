#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/poll.h>
//#include <linux/byteorder/swabb.h>
#include <linux/slab.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <ali_shm.h>  //for __VMTSALI

#include <linux/ali_rpc.h>

#include <rpc_hld/ali_rpc_hld_vbi.h>

#define ALI_VBI_INFO	"ALi VBI Driver"

extern struct vbi_device *g_vbi_config_dev;
extern struct vbi_config_par g_vbi_config;

static int ali_vbi_open(struct inode *inode, struct file *file)
{
	//VBI_PRINT("%s()->%s()->%d\n", __FILE__, __FUNCTION__, __LINE__);

	if(NULL == g_vbi_config_dev)
	{
		VBI_PRINT("VBI: g_vbi_config_dev is NULL\n");
		return -ENODEV;
	}
	else
		file->private_data = g_vbi_config_dev;

	vbi_open(g_vbi_config_dev);
	
	return 0;
}

static int  ali_vbi_release(struct inode *inode, struct file *file)
{
	struct vbi_device *p_vbi_dev = (struct vbi_device *)file->private_data;
	//VBI_PRINT("%s()->%s()->%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	vbi_close(p_vbi_dev);
	return 0;
}

extern unsigned long __G_ALI_MM_TTX_PARAM_BUF_ADDR;

INT32 ttx_show_content
(
    UINT8 *buf
)
{
    INT32 i;
	
    for (i = 0; i < 10; i++)
    {
        printk("%02x ", buf[i]);
	}

    printk("\n");
}


#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
 static long ali_vbi_ioctl(struct file *file, unsigned int cmd, unsigned long parg)
#else
 static int ali_vbi_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long parg)
 #endif
{
	struct vbi_device *p_vbi_dev = (struct vbi_device *)file->private_data;
	unsigned long arg = (unsigned long) parg;
	struct ttx_config_par pconfig_par;
	int ret = 0;
	
	VBI_PRINT("%s()->%s()->%d->cmd(0x%08x)\n", __FILE__, __FUNCTION__, __LINE__,cmd);

	if((cmd & VBI_CALL) == 0)
	{
		if((cmd == IO_VBI_ENGINE_UPDATE_INIT_PAGE) ||(cmd == IO_VBI_ENGINE_UPDATE_SUBT_PAGE))	
		{
			struct ttx_page_info p_user;
			struct ttx_page_info p_kernel;
		
			ret = copy_from_user(&p_user, (void *)parg, sizeof(struct ttx_page_info));	
            if (0 != ret)
            {
        	    printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
                return(-EFAULT);
            }
			
		
			p_kernel.num = p_user.num;
			//p_kernel.page_addr=(UINT32)kmalloc(((p_user.num)*sizeof(struct t_ttx_lang)),GFP_KERNEL);
            p_kernel.page_addr=(UINT32)__G_ALI_MM_TTX_PARAM_BUF_ADDR;
			
            printk("parg(0x%08x) addr(0x%08x)->num(%d)\n", parg, (UINT32)(p_user.page_addr), p_user.num);

			ret = copy_from_user((void *)p_kernel.page_addr, (void *)p_user.page_addr, ((p_user.num)*sizeof(struct t_ttx_lang)));

            if (0 != ret)
            {
        	    printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
                return(-EFAULT);
            }			

			//ttx_show_content((UINT8 *)p_kernel.page_addr);

            //VBI_PRINT("parg(0x%08x) addr(0x%08x)->num(%d)\n", parg, (UINT32)(p_user.page_addr), p_user.num);
            //VBI_PRINT("p_kernel(0x%08x) addr(0x%08x)->num(%d)\n", (UINT32)&p_kernel, (UINT32)(p_kernel.page_addr), p_kernel.num);

        	//printk("%s,%d,page_addr:%x\n", __FUNCTION__, __LINE__, p_kernel.page_addr);

            p_kernel.page_addr = __VMTSALI(p_kernel.page_addr);

        	//printk("%s,%d,page_addr:%x\n", __FUNCTION__, __LINE__, p_kernel.page_addr);			
			
			ret = vbi_ioctl(p_vbi_dev, cmd, (UINT32)&p_kernel);

			//kfree((void *)p_kernel.page_addr);
		}
		else if(cmd == IO_VBI_ENGINE_SET_CUR_LANGUAGE)
		{
			//u8 lang[3];
        	    //printk("%s,%d\n", __FUNCTION__, __LINE__);
			((int *)__G_ALI_MM_TTX_PARAM_BUF_ADDR)[0] = __VMTSALI(__G_ALI_MM_TTX_PARAM_BUF_ADDR + 4);

			copy_from_user((void *)(__G_ALI_MM_TTX_PARAM_BUF_ADDR + 4), (void *)parg, 3);

            //printk("%s,%d,__G_ALI_MM_TTX_PARAM_BUF_ADDR:%x,", __FUNCTION__, __LINE__, __G_ALI_MM_TTX_PARAM_BUF_ADDR);			
        
        	ttx_show_content(__G_ALI_MM_TTX_PARAM_BUF_ADDR + 4);			

        	    printk("%s,%d\n", __FUNCTION__, __LINE__);

			ret = vbi_ioctl(p_vbi_dev, cmd, (__G_ALI_MM_TTX_PARAM_BUF_ADDR));

        	    //printk("%s,%d,__VMTSALI(&__G_ALI_MM_TTX_PARAM_BUF_ADDR):%x\n", __FUNCTION__, __LINE__, __VMTSALI(__G_ALI_MM_TTX_PARAM_BUF_ADDR));
			
            //VBI_PRINT("user(0x%08x) lang(0x%08x)\n", parg, *(UINT32 *)&lang[0]);
		}
		else
			ret = vbi_ioctl(p_vbi_dev, cmd, arg);
	}
	else
	{
		switch(cmd)
		{
			case CALL_VBI_SETOUTPUT:
				vbi_setoutput(p_vbi_dev, (T_VBIRequest *)arg);
				break;
				
			case CALL_VBI_START:
				ret = vbi_start(p_vbi_dev, (t_TTXDecCBFunc)arg);
				break;

			case CALL_VBI_STOP:
				ret = vbi_stop(p_vbi_dev);
				break;		

			case CALL_TTX_DEFAULT_G0_SET:
				ttx_default_g0_set(p_vbi_dev, (u8)arg);
				break;

			case CALL_TTXENG_INIT:
				TTXEng_Init();
				break;

			case CALL_TTXENG_ATTACH:
				ret = copy_from_user(&pconfig_par, (void *)arg, sizeof(struct ttx_config_par));
				
				if(0 == ret)
					TTXEng_Attach(&pconfig_par);
				break;
				
			case CALL_ENABLE_VBI_TRANSFER:
				enable_vbi_transfer((bool)arg);
				break;

			case CALL_GET_INITAL_PAGE:
				ret = get_inital_page();
				break;
			
			case CALL_GET_INITAL_PAGE_STATUS:
				ret = get_inital_page_status();
				break;
				
			case CALL_GET_FIRST_TTX_PAGE:
				get_first_ttx_page();
				break;
			default:	
				break;
		}
	}
	
	return ret;

}


static const struct file_operations vbi_fops = {
	.owner		= THIS_MODULE,
	.open		= ali_vbi_open,
	.release		= ali_vbi_release,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_vbi_ioctl,
#else
	.ioctl		= ali_vbi_ioctl,
#endif	
};

static struct miscdevice vbi_misc = {
	.fops		= &vbi_fops,
	.name		= "ali_vbi",
	.minor		= MISC_DYNAMIC_MINOR,
};

static int __init ali_m36_vbi_init(void)
{
	int ret = 0;

	VBI_PRINT("%s\n", ALI_VBI_INFO);

	ret = misc_register(&vbi_misc);
	if (ret != 0) {
		VBI_PRINT(KERN_ERR "VBI: cannot register miscdev(err=%d)\n", ret);
		goto fail_misc;
	}
	
fail_misc:

	return ret;
}

static void __exit ali_m36_vbi_exit(void)
{
	misc_deregister(&vbi_misc);
}


module_init(ali_m36_vbi_init);
module_exit(ali_m36_vbi_exit);
MODULE_LICENSE("GPL");

