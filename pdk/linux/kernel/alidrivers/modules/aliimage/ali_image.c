/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: ali_image.c
 *  (I)
 *  Description: ali image player
 *  (S)
 *  History:(M)
 *      	Date        				Author        	Comment
 *      	====        			======		=======
 ****************************************************************************/
#include <linux/version.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/version.h>
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
#include <linux/fb.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/ali_transport.h>
#include <rpc_hld/ali_rpc_image.h>

#include <ali_cache.h>

#include "ali_image.h"

extern unsigned long __G_ALI_MM_IMAGE_DECODER_MEM_SIZE;
extern unsigned long __G_ALI_MM_IMAGE_DECODER_MEM_START_ADDR;

extern unsigned long __G_ALI_MM_VIDEO_SIZE;
extern unsigned long __G_ALI_MM_VIDEO_START_ADDR;
unsigned long g_ali_image_display_addr = 0;
unsigned long g_ali_image_display_size = 0x400000;

volatile unsigned long *g_ali_image_arg[MAX_IMG_ARG_NUM];
volatile int g_ali_image_arg_size[MAX_IMG_ARG_NUM];

static int debug_flag=0;

static ssize_t ali_image_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	return -1;
}

static ssize_t ali_image_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct ali_image_info *info = (struct ali_image_info *)file->private_data;
    int ret = 0;
    int length = count;
    unsigned long copy_ret = 0;

    UINT32 buf_addr = ((info->mem_addr + info->mem_size) & 0x1FFFFFFF) | 0x80000000;
    void *tmp_buf = NULL;
    tmp_buf = kmalloc(length, GFP_KERNEL);
	
    copy_ret = copy_from_user(tmp_buf, buf, length);
    if (copy_ret != 0)
    {
        IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
    }
    __CACHE_FLUSH_ALI((unsigned long)tmp_buf, length);

    /* Main CPU should use below mempy to transfer the data to private memory */
	hld_dev_memcpy((void *)buf_addr, tmp_buf, length);
		
	kfree(tmp_buf);

	return ret;
}

static int ali_image_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long pos;

	if(size > g_ali_image_display_size)
	{
		IMAGE_PRF("fail size %x mem_len %x\n", (int)size, (int)g_ali_image_display_size);
		return -EINVAL;
	}

	pos = (unsigned long)(g_ali_image_display_addr & 0x1FFFFFFF);
    IMAGE_PRF("POS: 0x%lx\n", pos);

	vma->vm_flags |= VM_IO;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if ( remap_pfn_range(vma,vma->vm_start, (pos>>PAGE_SHIFT), vma->vm_end - vma->vm_start, vma->vm_page_prot) )
	{
		IMAGE_PRF("remap pfn fail\n");
		return -EINVAL;
	}
	
	IMAGE_PRF("virtual add %x phy addr %x\n", (int)start, (int)pos);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
#else	
	vma->vm_flags |= VM_RESERVED | VM_IO;
#endif
    return 0;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_image_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static long ali_image_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	struct ali_image_info *info = (struct ali_image_info *)file->private_data;
    int sw_hw_flag;
	int i, ret = 0;

    unsigned long copy_ret = 0;

	IMAGE_PRF("<0>""Enter %s cmd %x arg %lx\n",__FUNCTION__, cmd, arg);
	
	switch(cmd) {
		case IMAGEDEC_CMD_INIT:
			{
				struct image_init_config img_init_cfg;
				
				copy_ret = copy_from_user((void *)&img_init_cfg, (void *)arg, sizeof(struct image_init_config));
			    if (copy_ret != 0)
                {
                    IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                }
				ret = image_init(&img_init_cfg);
				
				break;
			}
		case IMAGEDEC_CMD_RELEASE:
			{
				IMAGE_PRF("imagedec release CMD\n");
				
				ret = image_release();

				IMAGE_PRF("imagedec release CMD --rpc over\n");
				break;
			}
		case IMAGEDEC_CMD_STOP:
			{
				ret = image_stop();
				
				break;
			}
		case IMAGEDEC_CMD_DECODE:
			{				 				
				struct image_engine_config img_cfg;
				
				copy_ret = copy_from_user((void *)&img_cfg, (void *)arg, sizeof(struct image_engine_config));
                if (copy_ret != 0)
                {
                    IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                }
				
				//ret = ali_rpc_image_decode(img_cfg);
				sw_hw_flag = 0;
				ret = image_decode(sw_hw_flag, &img_cfg);				
				
				break;
			}
        case IMAGEDEC_CMD_ROTATE:
            {
                unsigned char rotate;
                copy_ret = copy_from_user((void *)&rotate, (void *)arg, sizeof(unsigned char));
                if (copy_ret != 0)
                {
                    IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                }
                
                IMAGE_PRF("IMAGEDEC_CMD_ROTATE angle: %d\n", rotate);
                ret = image_rotate(rotate);
                break;
            }
        case IMAGEDEC_CMD_ZOOM:
            {
                struct ali_image_pars pars;
                copy_ret = copy_from_user((void *)&pars, (void *)arg, sizeof(struct ali_image_pars));
                if (copy_ret != 0)
                {
                    IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                }

                for(i = 0;i < pars.arg_num;i++)
			    {
				    g_ali_image_arg_size[i] = pars.arg[i].arg_size;				
				    if(g_ali_image_arg_size[i] > 0)
				    {
					    if(g_ali_image_arg[i] == NULL)
					    {
						    IMAGE_PRF("allocate rpc arg buf fail\n");
						    ret = -ENOMEM;
						    //goto RPC_EXIT;			
					    }
					
					    copy_ret = copy_from_user((void *)(g_ali_image_arg[i]), pars.arg[i].arg, g_ali_image_arg_size[i]);
                        if (copy_ret != 0)
                        { 
                            IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                        }
				    }
			    }
                IMAGE_PRF("size1: %d, size2: %d\n", g_ali_image_arg_size[0], g_ali_image_arg_size[1]);
                image_zoom((struct Rect *)g_ali_image_arg[0], (struct Rect *)g_ali_image_arg[1]);
                break;
            }
        case IMAGEDEC_CMD_DISPLAY:
            {
                struct image_display_t img_display;
                //struct image_engine_config img_cfg;
               copy_ret =  copy_from_user((void *)&img_display, (void *)arg, sizeof(struct image_display_t));
               if (copy_ret != 0)
               {
                   IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
               }

                sw_hw_flag = 1;
                ret = image_display(sw_hw_flag, &img_display);
                break;
            }
		case IMAGEDEC_GET_MEM_INFO:
			{
				struct ali_image_mem_info mem_info;
				
				mem_info.mem_start = (void*)(info->mem_addr & 0x1FFFFFFF);
				mem_info.mem_size = info->mem_size - 0x400000;
				mem_info.priv_mem_start = (void*)((info->mem_addr + mem_info.mem_size) & 0x1FFFFFFF);
				mem_info.priv_mem_size = 0x400000;
				
				copy_ret = copy_to_user((void *)arg, (void *)&mem_info, sizeof(struct ali_image_mem_info));
                if (copy_ret != 0)
                {
                    IMAGE_PRF("<0>""Enter %s copy_to_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                }
				
				break;
			}
		case IMAGEDEC_CMD_GET_MAP_INFO:
		{
				struct ali_image_map_info map_info;
				
				map_info.mem_start = (void *)(__G_ALI_MM_IMAGE_DECODER_MEM_START_ADDR & 0x1FFFFFFF) ;
				map_info.mem_size = __G_ALI_MM_IMAGE_DECODER_MEM_SIZE;
				
				copy_ret = copy_to_user((void *)arg, (void *)&map_info, sizeof(struct ali_image_map_info));
                if (copy_ret != 0)
                {
                    IMAGE_PRF("<0>""Enter %s copy_to_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                }
			
			break;
		}
        case IMAGEDEC_CMD_GET_HWINFO:
            {
                struct image_hw_info hw_info;
                int flag = 1;
				memset(&hw_info, 0, sizeof(struct image_hw_info));
				
                ret = image_get_hw_addr(flag, &hw_info);
				//dma_cache_inv(&hw_info, sizeof(struct image_hw_info));
				//IMAGE_PRF("0x%x-----0x%x: %d,  0x%x: %d, %d, %d, %d\n", &hw_info, hw_info.y_addr, hw_info.y_size, hw_info.c_addr, hw_info.c_size, hw_info.width, hw_info.height,  hw_info.sample_format);
                copy_ret = copy_to_user((void *)arg, (void *)&hw_info, sizeof(struct image_hw_info));
                if (copy_ret != 0)
                {
                    IMAGE_PRF("<0>""Enter %s copy_to_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                }
                //g_ali_image_display_addr = hw_info.c_addr;
                __CACHE_FLUSH_ALI( (hw_info.y_addr | 0x80000000), hw_info.y_size);
                __CACHE_FLUSH_ALI( (hw_info.c_addr | 0x80000000), hw_info.c_size);

                break;
            }
		case IMAGEDEC_CMD_SET_MADDR:
            {
                unsigned long addr_map;
                copy_ret = copy_from_user((void *)&addr_map, (void *)arg, sizeof(unsigned long));
                if (copy_ret != 0)
                {
                    IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                }
                g_ali_image_display_addr = addr_map;
                break;
            }
        case IMAGEDEC_CMD_GET_IMG_INFO:
            {
                struct ali_image_pars pars;
               copy_ret =  copy_from_user((void *)&pars, (void *)arg, sizeof(struct ali_image_pars));
               if (copy_ret != 0)
               {
                   IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
               }

                for(i = 0;  i < pars.arg_num; i++)
			    {
				    g_ali_image_arg_size[i] = pars.arg[i].arg_size;				
				    if(g_ali_image_arg_size[i] > 0)
				    {
					    if(g_ali_image_arg[i] == NULL)
					    {
						    IMAGE_PRF("allocate rpc arg buf fail\n");
						    ret = -ENOMEM;
						    //goto RPC_EXIT;			
					    }
					
					   copy_ret =  copy_from_user((void *)(g_ali_image_arg[i]), pars.arg[i].arg, g_ali_image_arg_size[i]);
                       if (copy_ret != 0)
                       {
                           IMAGE_PRF("<0>""Enter %s copy_from_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                        }
				    }
			    }
                ret = image_get_info((int)(*g_ali_image_arg[0]), (struct image_info *)g_ali_image_arg[1]);
                for(i = 0; i < pars.arg_num; i++)
                {
                    if(g_ali_image_arg_size[i] > 0)
                    {
                        if(pars.arg[i].out)
                        {
                            copy_ret = copy_to_user(pars.arg[i].arg, (void *)g_ali_image_arg[i], g_ali_image_arg_size[i]);
                            if (copy_ret != 0)
                            {
                                IMAGE_PRF("<0>""Enter %s copy_to_user error,  ret = %lx \n",__FUNCTION__, copy_ret);
                            }
                        }
                    }
                }
                break;
            }
		default:
			ret = -EINVAL;
			break;		
	}

	IMAGE_PRF("<0>""Exit %s\n",__FUNCTION__);

	return ret;
}

static int ali_image_open(struct inode *inode, struct file *file)
{
	struct ali_image_info *info;
    int i, ret = 0;

	/* Get the per-device structure that contains this cdev */
	info = container_of(inode->i_cdev, struct ali_image_info, cdev);

	 /* Easy access to sbm_devp from rest of the entry points */
	file->private_data = info;

    if(g_ali_image_arg[0] == NULL){
	    for(i = 0;i < MAX_IMG_ARG_NUM;i++){
			g_ali_image_arg[i] = kmalloc(MAX_IMG_ARG_SIZE, GFP_KERNEL);
			if(g_ali_image_arg[i] == NULL){
				IMAGE_PRF("ali image malloc arg buf fail\n");
				ret = -ENOMEM;
				//goto EXIT;
			}
		}
	}

	return ret;
}

static int ali_image_release(struct inode *inode, struct file *file)
{
    int i;

    if(g_ali_image_arg[0] != NULL){
		for(i = 0;i < MAX_IMG_ARG_NUM;i++){
			if(g_ali_image_arg[i] != NULL){
				kfree((void *)g_ali_image_arg[i]);
				g_ali_image_arg[i] = NULL;
			}
		}
	}

	return 0;
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations ali_image_fops = {

    .owner    =   THIS_MODULE,           /* Owner */

    .open     =   ali_image_open,        /* Open method */

    .release  =   ali_image_release,     /* Release method */

    .read     =   ali_image_read,        /* Read method */

    .write    =   ali_image_write,       /* Write method */

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
     .unlocked_ioctl = ali_image_ioctl,       /* Ioctl method */
#else
    .ioctl    =   ali_image_ioctl,       /* Ioctl method */
#endif    

    .mmap     =   ali_image_mmap,        /* Mmap method  */
};

#define DEVICE_NAME                      "ali_image"

#define DEVICE_NUM                       1

struct ali_image_info *ali_image_priv;

static dev_t ali_image_dev_t;            /* Allotted device number */

static struct class *ali_image_class;    /* Tie with the device model */

static struct device *ali_image_device;

static int __init ali_image_init(void)
{
	int ret;

	/* Allocate memory for the per-device structure */
	ali_image_priv = kmalloc(sizeof(struct ali_image_info), GFP_KERNEL);
	if(!ali_image_priv) {
		printk("Image bad Kmalloc\n");
		return -ENOMEM;
	}
	memset(ali_image_priv, 0, sizeof(struct ali_image_info));

	//ali_image_priv->c
	
	
       ali_image_priv->mem_addr = __G_ALI_MM_VIDEO_START_ADDR;
	ali_image_priv->mem_size = __G_ALI_MM_VIDEO_SIZE;
	ali_image_priv->priv_mem_addr = 0;
	ali_image_priv->priv_mem_size = 0;
	g_ali_image_display_addr = __G_ALI_MM_VIDEO_START_ADDR + __G_ALI_MM_VIDEO_SIZE - g_ali_image_display_size;

	/* Request dynamic allocation of a device major number */
	if(alloc_chrdev_region(&ali_image_dev_t, 0, DEVICE_NUM, DEVICE_NAME) < 0) {
		
		printk(KERN_DEBUG "Can't register device\n"); 
		return -1;

	}

	 /* Populate sysfs entries */

    	ali_image_class = class_create(THIS_MODULE, DEVICE_NAME);

	if(ali_image_class == NULL) {
		printk(KERN_DEBUG "Image create class fail\n");
		goto fail;
	}

	 /* Connect the file operations with the cdev */
	 cdev_init(&ali_image_priv->cdev, &ali_image_fops);
	 ali_image_priv->cdev.owner = THIS_MODULE;
	 kobject_set_name(&(ali_image_priv->cdev.kobj), "%s", "ali_image");

	 /* Connect the major/minor number to the cdev */
	 ret = cdev_add(&ali_image_priv->cdev, ali_image_dev_t, 1);
	 if(ret) {
	 	printk("Image bad cdev\n");
		goto fail;
	 }

	 ali_image_device = device_create(ali_image_class, NULL, MKDEV(MAJOR(ali_image_dev_t), 0), 
	 				NULL, "ali_image%d", 0);
	 if(ali_image_device == NULL) {
	 	printk("Image create device fail\n");
		goto fail;
	 }
	 return 0;

fail:
	if(ali_image_dev_t != 0)
		unregister_chrdev_region(ali_image_dev_t, DEVICE_NUM);
	if(ali_image_device != NULL)
		device_del(ali_image_device);
	if(ali_image_class != NULL)
		class_destroy(ali_image_class);

	if(ali_image_priv != NULL )
		kfree(ali_image_priv);
	printk("image init fail\n");
	return -1;
}

/* Driver Exit */
void __exit ali_image_exit(void)
{
	/* Release the major number */
	unregister_chrdev_region(ali_image_dev_t, DEVICE_NUM);

	device_destroy(ali_image_class, MKDEV(MAJOR(ali_image_dev_t), 0));
	/* Remove the cdev */
	cdev_del(&ali_image_priv->cdev);
	/*Destroy ali_image_class */
	class_destroy(ali_image_class);

	kfree(ali_image_priv);

	ali_image_dev_t = 0;
	ali_image_device = NULL;
	ali_image_class = NULL;
	ali_image_priv = NULL;

	return;
}

module_init(ali_image_init);
module_exit(ali_image_exit);

module_param(debug_flag, int, 0664);
MODULE_PARM_DESC(debug_flag,"debug_flag, 1=output debug information, 0=do not output debug information");

MODULE_AUTHOR("ALi (Shanghai) Corporation");
MODULE_DESCRIPTION("ali image player driver");
MODULE_LICENSE("GPL");
