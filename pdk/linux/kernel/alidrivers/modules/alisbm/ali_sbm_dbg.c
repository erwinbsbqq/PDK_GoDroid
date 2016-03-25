#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
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
#include <ali_sbm_common.h>
#include <linux/version.h>
#include <rpc_hld/ali_rpc_sbm.h>
#include <linux/ali_rpc.h>
#include <ali_cache.h>
#include <ali_shm.h>
#include <linux/debugfs.h>
#include "ali_sbm.h"
#include "ali_sbm_dbg.h"

extern volatile struct sbm_desc *sbm_info[SBM_NUM];
extern volatile struct sbm_desc_pkt *sbm_info_pkt[SBM_NUM];
extern  struct sbm_dev *sbm_priv[SBM_NUM];

static struct dentry *ali_sbm_debugfs_root;
static __u32 ali_sbm_status_show_cnt = 0;
static struct task_struct *ali_sbm_status_show_task_hdl = NULL;
static struct mutex ali_sbm_status_show_mutex;
static __u32 ali_sbm_api_show_en;

__s32 ali_sbm_api_show
(
    char *fmt, ...
)
{
    va_list args;
    __s32 r;

    if (1 != ali_sbm_api_show_en)
    {
        return(-__LINE__);

    }
 
    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);

    return(r);
}


__s32 ali_sbm_show_skip_zero
(
    __u8 *fmt_str,
    __u32 value
)
{
    if (0 != value)
    {
        printk(fmt_str, value);
	}

    return(0);
}


__s32 ali_sbm_stats_write_go_cnt
(
    struct sbm_dev *dev,
    __u32 inc_num
)
{
    dev->wirte_go_cnt += inc_num;

    return(inc_num);
}

__s32 ali_sbm_stats_write_ill_status_cnt
(
    struct sbm_dev *dev,
    __u32 inc_num
)
{
    dev->write_ill_status_cnt += inc_num;

    return(inc_num);
}

__s32 ali_sbm_stats_write_mutex_fail_cnt
(
    struct sbm_dev *dev,
    __u32 inc_num
)
{
    dev->write_mutex_fail_cnt += inc_num;

    return(inc_num);
}

__s32 ali_sbm_stats_write_ok_cnt
(
    struct sbm_dev *dev,
    __u32 inc_num
)
{
    dev->write_ok_cnt += inc_num;

    return(inc_num);
}



static __s32 ali_sbm_status_show
(
    void
)
{
	__u32 idx;

    #if 0
	sbm_info = ali_sbm_info_array_get();
	sbm_info_pkt = ali_sbm_info_pkt_array_get();
	sbm_priv = ali_sbm_priv_array_get();
	#endif
			
    for (idx = 0; idx < SBM_NUM; idx++) 
	{
        if ((NULL == sbm_info[idx]) && (NULL == sbm_info_pkt[idx]))
        {
            continue;
		}

		printk("\n<=====================\n"); 
		
		printk("IDX:%u,%s:\n", idx, sbm_priv[idx]->name);
		
    	printk("===Configration:\n");
    	printk("buffer_addr:%u\n", sbm_priv[idx]->sbm_cfg.buffer_addr);
    	printk("buffer_size:%u\n", sbm_priv[idx]->sbm_cfg.buffer_size);
    	printk("block_size:%u\n", sbm_priv[idx]->sbm_cfg.block_size);
    	printk("reserve_size:%u\n", sbm_priv[idx]->sbm_cfg.reserve_size);
    	printk("wrap_mode:%u\n", sbm_priv[idx]->sbm_cfg.wrap_mode);
    	printk("lock_mode:%u\n", sbm_priv[idx]->sbm_cfg.lock_mode);		
		
    	printk("===Status:\n");
    	ali_sbm_show_skip_zero("status:%u\n", sbm_priv[idx]->status);
    	ali_sbm_show_skip_zero("open_count:%u\n", sbm_priv[idx]->open_count);
    	ali_sbm_show_skip_zero("write_ill_status_cnt:%u\n", sbm_priv[idx]->sbm_number);	
		
    	printk("===Global statistics:\n");
    	ali_sbm_show_skip_zero("wirte_go_cnt:%u\n", sbm_priv[idx]->wirte_go_cnt);
    	ali_sbm_show_skip_zero("write_mutex_fail_cnt:%u\n", sbm_priv[idx]->write_mutex_fail_cnt);
    	ali_sbm_show_skip_zero("write_ill_status_cnt:%u\n", sbm_priv[idx]->write_ill_status_cnt);
    	ali_sbm_show_skip_zero("write_ok_cnt:%u\n", sbm_priv[idx]->write_ok_cnt);

		if (NULL != sbm_info[idx])
		{
 		    printk("===sbm_info status:\n");
			
            printk("read_pos:%u\n", sbm_info[idx]->read_pos);	
            printk("write_pos:%u\n", sbm_info[idx]->write_pos);	
            printk("valid_size:%u\n", sbm_info[idx]->valid_size);
			
			ali_sbm_show_skip_zero("request_write_total_cnt:%u\n", sbm_info[idx]->request_write_total_cnt);
			ali_sbm_show_skip_zero("request_write_status_err_cnt:%u\n", sbm_info[idx]->request_write_status_err_cnt);
			ali_sbm_show_skip_zero("request_write_full_cnt:%u\n", sbm_info[idx]->request_write_full_cnt);
			ali_sbm_show_skip_zero("request_write_ok_cnt:%u\n", sbm_info[idx]->request_write_ok_cnt);
			ali_sbm_show_skip_zero("update_write_total_cnt:%u\n", sbm_info[idx]->update_write_total_cnt);
			ali_sbm_show_skip_zero("update_write_status_err_cnt:%u\n", sbm_info[idx]->update_write_status_err_cnt);
			ali_sbm_show_skip_zero("update_write_ok_cnt:%u\n", sbm_info[idx]->update_write_ok_cnt);
			
			ali_sbm_show_skip_zero("request_read_total_cnt:%u\n", sbm_info[idx]->request_read_total_cnt);
			ali_sbm_show_skip_zero("request_read_status_err_cnt:%u\n", sbm_info[idx]->request_read_status_err_cnt);
			ali_sbm_show_skip_zero("request_read_ok_cnt:%u\n", sbm_info[idx]->request_read_ok_cnt);			
			ali_sbm_show_skip_zero("request_read_empty_cnt:%u\n", sbm_info[idx]->request_read_empty_cnt);
			ali_sbm_show_skip_zero("update_read_total_cnt:%u\n", sbm_info[idx]->update_read_total_cnt);
			ali_sbm_show_skip_zero("update_read_status_err_cnt:%u\n", sbm_info[idx]->update_read_status_err_cnt);	
			ali_sbm_show_skip_zero("update_read_ok_cnt:%u\n", sbm_info[idx]->update_read_ok_cnt);
			
			ali_sbm_show_skip_zero("pkt_write_total_cnt:%u\n", sbm_info[idx]->pkt_write_total_cnt);
			ali_sbm_show_skip_zero("pkt_write_status_copy_from_user_err_cnt:%u\n", sbm_info[idx]->pkt_write_status_copy_from_user_err_cnt);			
			ali_sbm_show_skip_zero("pkt_write_full_cnt:%u\n", sbm_info[idx]->pkt_write_full_cnt);
			ali_sbm_show_skip_zero("pkt_write_ok_cnt:%u\n", sbm_info[idx]->pkt_write_ok_cnt);
		}
		else
		{
 		    printk("===sbm_info:NULL\n");  
		}

		if (NULL != sbm_info_pkt[idx])
		{
 		    printk("===sbm_info_pkt status:\n");
			
            printk("read_pos:%u\n", sbm_info_pkt[idx]->read_pos);	
            printk("write_pos:%u\n", sbm_info_pkt[idx]->write_pos);	
            printk("valid_size:%u\n", sbm_info_pkt[idx]->valid_size);

			ali_sbm_show_skip_zero("request_write_total_cnt:%u\n", sbm_info_pkt[idx]->request_write_total_cnt);
			ali_sbm_show_skip_zero("request_write_status_err_cnt:%u\n", sbm_info_pkt[idx]->request_write_status_err_cnt);
			ali_sbm_show_skip_zero("request_write_full_cnt:%u\n", sbm_info_pkt[idx]->request_write_full_cnt);
			ali_sbm_show_skip_zero("request_write_ok_cnt:%u\n", sbm_info_pkt[idx]->request_write_ok_cnt);
			ali_sbm_show_skip_zero("update_write_total_cnt:%u\n", sbm_info_pkt[idx]->update_write_total_cnt);
			ali_sbm_show_skip_zero("update_write_status_err_cnt:%u\n", sbm_info_pkt[idx]->update_write_status_err_cnt);
			ali_sbm_show_skip_zero("update_write_ok_cnt:%u\n", sbm_info_pkt[idx]->update_write_ok_cnt);
			
			ali_sbm_show_skip_zero("request_read_total_cnt:%u\n", sbm_info_pkt[idx]->request_read_total_cnt);
			ali_sbm_show_skip_zero("request_read_status_err_cnt:%u\n", sbm_info_pkt[idx]->request_read_status_err_cnt);
			ali_sbm_show_skip_zero("request_read_ok_cnt:%u\n", sbm_info_pkt[idx]->request_read_ok_cnt);			
			ali_sbm_show_skip_zero("request_read_empty_cnt:%u\n", sbm_info_pkt[idx]->request_read_empty_cnt);
			ali_sbm_show_skip_zero("update_read_total_cnt:%u\n", sbm_info_pkt[idx]->update_read_total_cnt);
			ali_sbm_show_skip_zero("update_read_status_err_cnt:%u\n", sbm_info_pkt[idx]->update_read_status_err_cnt);	
			ali_sbm_show_skip_zero("update_read_ok_cnt:%u\n", sbm_info_pkt[idx]->update_read_ok_cnt);
			
			ali_sbm_show_skip_zero("pkt_write_total_cnt:%u\n", sbm_info_pkt[idx]->pkt_write_total_cnt);
			ali_sbm_show_skip_zero("pkt_write_status_copy_from_user_err_cnt:%u\n", sbm_info_pkt[idx]->pkt_write_status_copy_from_user_err_cnt);			
			ali_sbm_show_skip_zero("pkt_write_full_cnt:%u\n", sbm_info_pkt[idx]->pkt_write_full_cnt);
			ali_sbm_show_skip_zero("pkt_write_ok_cnt:%u\n", sbm_info_pkt[idx]->pkt_write_ok_cnt);			
		}
		else
		{
 		    printk("===sbm_info_pkt:NULL\n");   
		}

		printk("=====================>\n"); 
    }

	return(0);
}


static __s32 ali_sbm_status_show_task
(
    void *param
)
{	
    for (;;)
    {
        if (mutex_lock_interruptible(&ali_sbm_status_show_mutex))
        {
            return(-ERESTARTSYS);
        }

        if (ali_sbm_status_show_cnt > 0)
        {
            ali_sbm_status_show();
		}

    	ali_sbm_status_show_cnt--;

		if (ali_sbm_status_show_cnt > 0)
		{
            mutex_unlock(&ali_sbm_status_show_mutex);

            /* Check dmx buffer every DMX_DATA_ENG_OUTPUT_INTERVAL miliseconds.
             * This should be enough to meet all time reqirments.
             */
            msleep_interruptible(3000);				
		}
		else
		{
            mutex_unlock(&ali_sbm_status_show_mutex);
			
			break;
		}
    }

	return(0);
}




static __s32 ali_sbm_status_show_cnt_set
(
    /* Passed in by debugfs_create_file().
    */
    void *show_cnt_ptr, 

    /*set by user
    */
    __u64 setting
)
{
    printk("%s,%d,setting:%u,ali_sbm_status_show_cnt:%u\n",
		__FUNCTION__, __LINE__, (__u32)setting, ali_sbm_status_show_cnt);

    if (mutex_lock_interruptible(&ali_sbm_status_show_mutex))
    {
        return(-ERESTARTSYS);
    }
		
    /* If debug task does not exist, create it.
	*/
    if (ali_sbm_status_show_cnt <= 0)
    {
        ali_sbm_status_show_cnt = (__u32)setting;
	
        printk("%s,%d,setting:%u\n", __FUNCTION__, __LINE__, (__u32)setting);
    
        ali_sbm_status_show_task_hdl = kthread_create(
			ali_sbm_status_show_task, NULL, "ali_sbm_status");
    
        if (IS_ERR(ali_sbm_status_show_task_hdl))
        {
            return(PTR_ERR(ali_sbm_status_show_task_hdl)); 
        }
    
        wake_up_process(ali_sbm_status_show_task_hdl);   
	}
	else
	{
        ali_sbm_status_show_cnt = setting;
	}

    mutex_unlock(&ali_sbm_status_show_mutex);
		
    printk("%s,%d,setting:%u,ali_sbm_status_show_cnt:%u\n",
		__FUNCTION__, __LINE__, (__u32)setting, ali_sbm_status_show_cnt);	
	
    return(0);
}








static __s32 ali_sbm_status_show_cnt_get
(
    /* Passed in by debugfs_create_file().
    */
    void *data,

    /*set by user
    */
    u64 *val
)
{
    *val = *(u32 *)data;

    return 0;
}



DEFINE_SIMPLE_ATTRIBUTE(ali_sbm_status_show_cnt_fops, 
	ali_sbm_status_show_cnt_get,
    ali_sbm_status_show_cnt_set, "%llu\n");


/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_sbm_dbg_exit
(
    void 
)
{
    #if 0
    debugfs_remove_recursive(ali_alsa_dbg_debugfs_root);
    ali_alsa_dbg_debugfs_root = NULL;
    ali_alsa_capture_dump_en = 0;
	ali_alsa_playback_dump_en = 0;

    ali_alsa_playback_dump_cnt = 0;
    ali_alsa_capture_dump_cnt = 0;	
    #endif
	
    ali_sbm_status_show_cnt_set(&ali_sbm_status_show_cnt, 0);	
	ali_sbm_status_show_cnt = 0;

	#if 0
	ali_alsa_reg_show_task_hdl = NULL;
	ali_alsa_reg_show_interval = 3000;
	#endif
	
    return(0);
}

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
__s32 ali_sbm_dbg_init
(
    void 
)
{
    struct dentry *fs_entry;

    printk("%s,%d,Go\n", __FUNCTION__, __LINE__);

	mutex_init(&ali_sbm_status_show_mutex);
	
    ali_sbm_debugfs_root = debugfs_create_dir("ali_sbm", NULL);
    
    if (!ali_sbm_debugfs_root)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        return(-ENOENT);
    }

    /* For ali audio sbm api show enable/disable.
	*/
    fs_entry = debugfs_create_u32("api_show_en", 0644, ali_sbm_debugfs_root,
                                  &ali_sbm_api_show_en);
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    } 
    
    printk("%s,%d,succeed.\n", __FUNCTION__, __LINE__);
	
    fs_entry = debugfs_create_file("status_show_cnt", 0644,
	    ali_sbm_debugfs_root, &ali_sbm_status_show_cnt, 
	    &ali_sbm_status_show_cnt_fops);
	
	if (!fs_entry)
	{
	    printk("%s,%d\n", __FUNCTION__, __LINE__);
		goto Fail;	
	}
	
	
    printk("%s,%d,succeed.\n", __FUNCTION__, __LINE__);
    return(0);

Fail:
    printk("%s,%d,failed\n", __FUNCTION__, __LINE__);
    ali_sbm_dbg_exit();
    return(-ENOENT);    
}
