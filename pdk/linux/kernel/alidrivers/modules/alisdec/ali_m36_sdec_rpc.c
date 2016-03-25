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
//#include <linux/smp_lock.h>
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


#include <linux/ali_rpc.h>

#include <rpc_hld/ali_rpc_hld_sdec.h>

#define ALI_SDEC_INFO   "ALi SDEC Driver"

extern struct sdec_feature_config g_sdec_config;
extern struct sdec_device *g_sdec_device;

static int ali_sdec_open(struct inode *inode, struct file *file)
{
    //printk("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

    if(NULL == g_sdec_device)
    {
        printk("SDEC: g_sdec_config is NULL\n");
        return -ENODEV;
    }
    else
        file->private_data = g_sdec_device;
    
    sdec_open(g_sdec_device);
    
    return 0;
}

static int  ali_sdec_release(struct inode *inode, struct file *file)
{
    struct sdec_device *p_sdec_dev = (struct sdec_device *)file->private_data;

    printk("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

    sdec_close(p_sdec_dev);

    return 0;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_sdec_ioctl(struct file *file, unsigned int cmd, unsigned long parg)
#else
static int ali_sdec_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long parg)
#endif
{
    struct sdec_device *p_sdec_dev = (struct sdec_device *)file->private_data;
    unsigned long arg = (unsigned long) parg;
    INT32 ret = 0;
    UINT16 low16, high16;
    struct subt_config_par config_par;

    //printk("%s->%s()->%d->cmd(0x%08x)->parg(0x%08x).\n", __FILE__, __FUNCTION__, __LINE__, cmd, parg);
    
    switch(cmd)
    {
        case CALL_SDEC_START:
            low16 = (UINT16)arg; 
            high16 = (UINT16)(arg >> 16); 
            ret = sdec_start(p_sdec_dev, low16, high16);
            break;
            
        case CALL_SDEC_STOP:
            ret = sdec_stop(p_sdec_dev);
            break;

        case CALL_SDEC_PAUSE:
            ret = sdec_pause(p_sdec_dev);
            break;
            
        case CALL_LIB_SUBT_ATTACH:
            ret = copy_from_user(&config_par, (void *)arg, sizeof(struct subt_config_par));

            if (0 != ret)
            {
                printk("%s,%d,copy_from_user() failed, ret:%d\n", __FUNCTION__, __LINE__, ret);
        
                return(-EFAULT);
            }
            
            lib_subt_attach(&config_par);

            break;
            
        case CALL_OSD_SUBT_ENTER:
            osd_subt_enter();
            break;
            
        case CALL_OSD_SUBT_LEAVE:
            osd_subt_leave();
            break;
        case CALL_LIB_ISDBT_CC_INIT:
            lib_isdbtcc_init();
            break;
        case CALL_OSD_ISDBT_CC_ENTER:
            osd_isdbtcc_enter();
            break;
        case CALL_OSD_ISDBT_CC_LEASE:
            osd_isdbtcc_leave();
            break;
#if 0           
        case CALL_LIB_ISDBT_CC_ATTACH:
        {
extern unsigned long __G_MM_SUB_PB_START_ADDR;
#define __MM_ISDBT_PB_LEN           (0x7E900)
            struct isdbtcc_config_par par;

            copy_from_user((void *)&par, arg, sizeof(par)); 
            par.g_buf_addr = (UINT8 *)__G_MM_SUB_PB_START_ADDR;
            par.g_buf_len = __MM_ISDBT_PB_LEN;
            lib_isdbtcc_attach(&par);
            break;
        }
#endif      
        case CALL_ISDBT_CC_DEC_ATTACH:
        {   
            struct sdec_feature_config par;
            
            ret = copy_from_user((void *)&par, arg, sizeof(par));

            if (0 != ret)
            {
                printk("%s,%d,copy_from_user() failed, ret:%d\n", __FUNCTION__, __LINE__, ret);
        
                return(-EFAULT);
            }
            
            ret = isdbtcc_dec_attach(&par);
            break;
        }
        case CALL_ISDBT_CC_DISPLAY_INIT:
            isdbtcc_disply_init(p_sdec_dev);
            break;
        case CALL_ISDBT_CC_GET_CUR_LANG_BY_PID:
        {
            struct isdbt_get_lang_by_pid info;
            unsigned int para;

            ret = copy_from_user((void *)&info, (void *)arg, sizeof(info));
            if (0 != ret)
            {
                printk("%s,%d,copy_from_user() failed, ret:%d\n", __FUNCTION__, __LINE__, ret);
        
                return(-EFAULT);
            }           
            isdbtcc_get_cur_lang_by_pid(info.pid, (UINT32)&para);
            ret = copy_to_user((void *)info.para, (void *)&para, 4);
            if (0 != ret)
            {
                printk("%s,%d,copy_from_user() failed, ret:%d\n", __FUNCTION__, __LINE__, ret);
        
                return(-EFAULT);
            }           
            break;
        }   
        case CALL_SUBT_ATSC_SET_BUF:
        {
            struct atsc_subt_config_par par;
            ret = copy_from_user((void*)&par,arg, sizeof(par));
            if (0 != ret)
            {
                printk("%s,%d,copy_from_user() failed, ret:%d\n", __FUNCTION__, __LINE__, ret);
        
                return(-EFAULT);
            }
            
            lib_subt_atsc_set_buf(&par);

            break;
        }
        case CALL_SUBT_ATSC_CREATE_TASK:
        {
            lib_subt_atsc_create_task();
            break;
        }
        case CALL_SUBT_ATSC_TERMINATE_TASK:
        {
            lib_subt_atsc_terminate_task();
            break;
        }
        case CALL_SUBT_ATSC_STREAM_IDENTIFY:
        {
            #if 0
            struct subt_atsc_stream_identify par;
            extern UINT8 *g_sec_buf;
            UINT8 *data;    
            UINT8 *temp_data;
            ret = copy_from_user((void*)&par,arg, sizeof(par));
            if (0 != ret)
            {
                printk("%s,%d,copy_from_user() failed, ret:%d\n", __FUNCTION__, __LINE__, ret);
        
                return(-EFAULT);
            }
                        
            data = (UINT8*)kmalloc(sizeof(UINT8)*(par.length+64), GFP_KERNEL);
            temp_data = (UINT8 *)(((UINT32)data + 0x1f) & 0xfffffffe0);
            ret = copy_from_user(temp_data,par.data,par.length);
            if (0 != ret)
            {
                printk("%s,%d,copy_from_user() failed, ret:%d\n", __FUNCTION__, __LINE__, ret);
        
                return(-EFAULT);
            }
            
            //dma_cache_wback(temp_data, par.length);
            hld_dev_memcpy(g_sec_buf, temp_data, par.length);
            kfree(data);
            //lib_subt_atsc_stream_identify(par.length, par.data);
            lib_subt_atsc_stream_identify(par.length, g_sec_buf);
            #endif
            break;
        }
        case CALL_SUBT_ATSC_SHOW_ONOFF:
        {
            lib_subt_atsc_show_onoff((BOOL)arg);
            break;
        }
        case CALL_SUBT_ATSC_CLEAN_UP:
        {
            lib_subt_atsc_clean_up();
            break;
        }
        case CALL_SUBT_ATSC_DEL_TIMER:
        {
            lib_subt_atsc_delete_timer();
            break;
        }
        case CALL_SUBT_ATSC_PARSE_CREATE_TASK:
        {
            lib_subt_atsc_section_parse_create_task();
            break;
        }
        case CALL_SUBT_ATSC_PARSE_TERMINATE_TASK:
        {
            lib_subt_atsc_section_parse_terminate_task();
            break;
        }
        default:
            ret = -1;
            //printk("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
            break;
    }

    return (int)ret;
}


static const struct file_operations sdec_fops = {
    .owner      = THIS_MODULE,
    .open       = ali_sdec_open,
    .release        = ali_sdec_release,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = ali_sdec_ioctl,
#else
    .ioctl      = ali_sdec_ioctl,
#endif  
};

static struct miscdevice sdec_misc = {
    .fops       = &sdec_fops,
    .name       = "ali_sdec",
    .minor      = MISC_DYNAMIC_MINOR,
};


static int __init ali_m36_sdec_init(void)
{
    int ret = 0;

    printk("%s\n", ALI_SDEC_INFO);

    ret = misc_register(&sdec_misc);
    if (ret != 0) {
        printk("SDEC: cannot register miscdev(err=%d)\n", ret);
        goto fail_misc;
    }
    
fail_misc:

    return ret;
}

static void __exit ali_m36_sdec_exit(void)
{
    misc_deregister(&sdec_misc);
}


module_init(ali_m36_sdec_init);
module_exit(ali_m36_sdec_exit);
MODULE_LICENSE("GPL");

