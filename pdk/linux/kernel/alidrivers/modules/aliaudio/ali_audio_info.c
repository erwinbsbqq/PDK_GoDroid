
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/poll.h>
//#include <linux/byteorder/swabb.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/version.h>

#include <ali_cache.h>
#include <linux/debugfs.h>

//#include <asm/system.h>
//#include <asm/semaphore.h>

#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <ali_rpcng.h>
#include "ali_m36_audio_rpc.h"
#include <ali_shm.h> // add by jacket 2013.7.17
#include <ali_board_config.h> // add by jacket 2013.10.23
#include "ali_audio_info.h"

/* For debug functions.
 * Date:2014/11/4
 */

static struct dentry *ali_audio_debugfs_root;
static struct mutex ali_audio_info_show_mutex;
static __u32 ali_audio_info_show_interval = 3000;
static struct task_struct *ali_audio_info_show_task_hdl;

static __u32 ali_audio_show_en;
static __u32 ali_audio_snd_reg_show_cnt;
static __u32 ali_audio_see_snd_stats_show_cnt;
static __u32 ali_audio_see_ape_show_cnt;

static struct ali_audio_see_ape_info ali_audio_see_ape_info;
static struct ali_audio_see_snd_info ali_audio_see_snd_info;


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
/* For debug functions.
 * Date:2014/11/4
 */
__s32 ali_audio_api_show(char *fmt, ...)
{
    va_list args;
    __s32 r;

    if (1 != ali_audio_show_en)
    {
        return(-__LINE__);

    }
 
    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);

    return(r);
}



__s32 ali_audio_see_snd_info_get
(
    struct ali_audio_see_snd_info *see_snd_info
)
{
    __s32 ret;

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_OPAQUE, sizeof(struct ali_audio_see_snd_info), see_snd_info);
    
    ret = RpcCallCompletion(RPC_ali_audio_see_snd_info_get, &p1, NULL);     

    printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);      
    
    return(ret);
}



__s32 ali_audio_see_snd_info_show
(
    struct ali_audio_see_snd_info *see_snd_info
)
{
    printk("\n\n%s,%d,see_snd_info:\n", __FUNCTION__, __LINE__);

    printk("i2si_rx_samp_int_cnt:%u\n", see_snd_info->i2si_rx_samp_int_cnt);
    printk("i2so_samp_int_cnt:%u\n", see_snd_info->i2so_samp_int_cnt);
    printk("spo_samp_int_cnt:%u\n", see_snd_info->spo_samp_int_cnt);
    printk("ddp_spo_samp_int_cnt:%u\n", see_snd_info->ddp_spo_samp_int_cnt);
    printk("i2so_underrun_int_cnt:%u\n", see_snd_info->i2so_underrun_int_cnt);
    printk("spo_underrun_int_cnt:%u\n", see_snd_info->spo_underrun_int_cnt);
    printk("ddp_spo_underrun_int_cnt:%u\n", see_snd_info->ddp_spo_underrun_int_cnt);
    printk("spo_resume_int_cnt:%u\n", see_snd_info->i2so_resume_int_cnt);
    printk("ddp_spo_resume_int_cnt:%u\n", see_snd_info->ddp_spo_resume_int_cnt);    
    printk("i2so_timing_chk_int_cnt:%u\n", see_snd_info->i2so_timing_chk_int_cnt);  

    printk("new_aavsync_play_cnt:%u\n", see_snd_info->new_avsync_play_cnt);
    printk("new_aavsync_drop_cnt:%u\n", see_snd_info->new_avsync_drop_cnt);
    printk("new_aavsync_hold_cnt:%u\n", see_snd_info->new_avsync_hold_cnt);
    printk("new_aavsync_freerun_cnt:%u\n", see_snd_info->new_avsync_freerun_cnt);
    printk("new_aavsync_hw_hold_cnt:%u\n", see_snd_info->new_avsync_hw_hold_cnt);       

    printk("old_avsync_hold_cnt:%u\n", see_snd_info->old_avsync_hold_cnt);
    printk("old_avsync_drop_cnt:%u\n", see_snd_info->old_avsync_drop_cnt);
    printk("old_avsync_freerun_cnt:%u\n", see_snd_info->old_avsync_freerun_cnt);    

    printk("pcm_sync_buff_empty_cnt:%u\n", see_snd_info->pcm_sync_buff_empty_cnt);
    printk("pcm_sync_buff_full_cnt:%u\n", see_snd_info->pcm_sync_buff_full_cnt);
    printk("es_sync_buff_empty_cnt:%u\n", see_snd_info->es_sync_buff_empty_cnt);
    printk("es_sync_buff_full_cnt:%u\n", see_snd_info->es_sync_buff_full_cnt);  
    printk("es_ddp_sync_buff_empty_cnt:%u\n", see_snd_info->es_ddp_sync_buff_empty_cnt);
    printk("es_ddp_sync_buff_full_cnt:%u\n", see_snd_info->es_ddp_sync_buff_full_cnt);  
    
    printk("pcm_dma_buff_empty_cnt:%u\n", see_snd_info->pcm_dma_buff_empty_cnt);
    printk("pcm_dma_buff_full_cnt:%u\n", see_snd_info->pcm_dma_buff_full_cnt);
    printk("es_dma_buff_empty_cnt:%u\n", see_snd_info->es_dma_buff_empty_cnt);      
    printk("es_dma_buff_full_cnt:%u\n", see_snd_info->es_dma_buff_full_cnt);
    printk("es_ddp_dma_buff_empty_cnt:%u\n", see_snd_info->es_ddp_dma_buff_empty_cnt);
    printk("es_ddp_dma_buff_full_cnt:%u\n", see_snd_info->es_ddp_dma_buff_full_cnt);
    
    printk("snd_lld_input_go_cnt:%u\n", see_snd_info->snd_lld_input_go_cnt);
    printk("snd_lld_input_fail_cnt:%u\n", see_snd_info->snd_lld_input_fail_cnt);    
    printk("snd_lld_input_succ_cnt:%u\n", see_snd_info->snd_lld_input_succ_cnt);        
    printk("snd_lld_check_dma_buff_go_cnt:%u\n", see_snd_info->snd_lld_check_dma_buff_go_cnt);
    printk("snd_lld_check_dma_buff_fail_cnt:%u\n", see_snd_info->snd_lld_check_dma_buff_fail_cnt);
    printk("snd_lld_check_dma_buff_succ_cnt:%u\n", see_snd_info->snd_lld_check_dma_buff_succ_cnt);  
    printk("snd_lld_output_go_cnt:%u\n", see_snd_info->snd_lld_output_go_cnt);
    printk("snd_lld_output_succ_cnt:%u\n", see_snd_info->snd_lld_output_succ_cnt);

    return(0);
}


__s32 ali_audio_see_ape_info_get
(
    struct ali_audio_see_ape_info *see_ape_info
)
{
    __s32 ret;

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_OPAQUE, sizeof(struct ali_audio_see_ape_info), see_ape_info);
      
    ret = RpcCallCompletion(RPC_ali_audio_see_ape_info_get, &p1, NULL);     

    printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);      
    
    return(ret);
}



__s32 ali_audio_see_ape_info_show
(
    struct ali_audio_see_ape_info *see_ape_info
)
{
    printk("\n\n%s,%d,see_ape_info:\n", __FUNCTION__, __LINE__);

    printk("deca_task_status:%u\n", see_ape_info->deca_task_status);
    printk("deca_task_go_cnt:%u\n", see_ape_info->deca_task_go_cnt);

    printk("deca_task_rd_hdr_go_cnt:%u\n", see_ape_info->deca_task_rd_hdr_go_cnt);
    printk("deca_task_rd_hdr_succ_cnt:%u\n", see_ape_info->deca_task_rd_hdr_succ_cnt);
    printk("deca_task_rd_hdr_fail_cnt:%u\n", see_ape_info->deca_task_rd_hdr_fail_cnt);

    printk("deca_task_rd_data_go_cnt:%u\n", see_ape_info->deca_task_rd_data_go_cnt);
    printk("deca_task_rd_data_fail_cnt:%u\n", see_ape_info->deca_task_rd_data_fail_cnt);    
    printk("deca_task_rd_data_succ_cnt:%u\n", see_ape_info->deca_task_rd_data_succ_cnt);
    
    printk("deca_task_ring_busy_cnt:%u\n", see_ape_info->deca_task_ring_busy_cnt);

    printk("deca_task_avysync_go_cnt:%u\n", see_ape_info->deca_task_avysync_go_cnt);    
    printk("deca_task_avysync_drop_cnt:%u\n", see_ape_info->deca_task_avysync_drop_cnt);
    printk("deca_task_avysync_ok_cnt:%u\n", see_ape_info->deca_task_avysync_ok_cnt);

    printk("deca_task_decode_go_cnt:%u\n", see_ape_info->deca_task_decode_go_cnt);  
    printk("deca_task_decode_succ_cnt:%u\n", see_ape_info->deca_task_decode_succ_cnt);
    printk("deca_task_decode_busy_cnt:%u\n", see_ape_info->deca_task_decode_busy_cnt);
    printk("deca_task_decode_wr_pcm_fail_cnt:%u\n", see_ape_info->deca_task_decode_wr_pcm_fail_cnt);    
    printk("deca_task_decode_buffering_cnt:%u\n", see_ape_info->deca_task_decode_buffering_cnt);    
    printk("deca_task_decode_samplerate_chg_cnt:%u\n", see_ape_info->deca_task_decode_samplerate_chg_cnt);
    printk("deca_task_decode_ch_num_chg_cnt:%u\n", see_ape_info->deca_task_decode_ch_num_chg_cnt);
    printk("deca_task_sleep_cnt:%u\n", see_ape_info->deca_task_sleep_cnt);  

    printk("decode_mode:%u\n", see_ape_info->decode_mode);
    printk("sync_mode:%u\n", see_ape_info->sync_mode);
    printk("sync_unit:%u\n", see_ape_info->sync_unit);
    printk("deca_input_sbm:%u\n", see_ape_info->deca_input_sbm);
    printk("deca_output_sbm:%u\n", see_ape_info->deca_output_sbm);
    printk("snd_input_sbm:%u\n", see_ape_info->snd_input_sbm);
    printk("pcm_sbm:%u\n", see_ape_info->pcm_sbm);
    printk("codec_id:%u\n", see_ape_info->codec_id);
    printk("bits_per_coded_sample:%u\n", see_ape_info->bits_per_coded_sample);
    printk("sample_rate:%u\n", see_ape_info->sample_rate);
    printk("channels:%u\n", see_ape_info->channels);
    printk("bit_rate:%u\n", see_ape_info->bit_rate);
    printk("pcm_buf:0x%x\n", see_ape_info->pcm_buf);
    printk("pcm_buf_size:%u\n", see_ape_info->pcm_buf_size);
    printk("block_align:%u\n", see_ape_info->block_align);

    printk("dwRIFF:%u\n", see_ape_info->dwRIFF);
    printk("dwFileLen:%u\n", see_ape_info->dwFileLen);
    printk("dwWAVE:%u\n", see_ape_info->dwWAVE);
    printk("dw_fmt:%u\n", see_ape_info->dw_fmt);
    printk("dwFmtLen:%u\n", see_ape_info->dwFmtLen);
    printk("wDataType:%u\n", see_ape_info->wDataType);
    printk("wNChannels:%u\n", see_ape_info->wNChannels);
    printk("dwSamplingRate:%u\n", see_ape_info->dwSamplingRate);
    printk("dwNBytesPerSec:%u\n", see_ape_info->dwNBytesPerSec);
    printk("wAlignment:%u\n", see_ape_info->wAlignment);
    printk("wNBitsPerSam:%u\n", see_ape_info->wNBitsPerSam);
    printk("wNBitsPerSam:%u\n", see_ape_info->bit_rate);
    printk("wAlignment:0x%x\n", see_ape_info->wAlignment);
    printk("wNBitsPerSam:%u\n", see_ape_info->wNBitsPerSam);
    printk("dwdata:%u\n", see_ape_info->dwdata);
    printk("dwDataLen:%u\n", see_ape_info->dwDataLen);

    printk("pAudioInit:0x%x\n", see_ape_info->pAudioInit);
    printk("pAudioOutput:0x%x\n", see_ape_info->pAudioOutput);
    printk("pAudioStop:0x%x\n", see_ape_info->pAudioStop);
    printk("pAudioReset:0x%x\n", see_ape_info->pAudioReset);
    printk("m_pAdecAvailable:0x%x\n", see_ape_info->m_pAdecAvailable);
    printk("m_pAdecPlayerState:0x%x\n", see_ape_info->m_pAdecPlayerState);
    printk("m_pAdecBufEmpty:0x%x\n", see_ape_info->m_pAdecBufEmpty);
	
    return(0);
}





__s32 ali_audio_info_show_thread_exit
(
    void
)
{
    ali_audio_info_show_task_hdl = NULL;
    return(0);
}



static __s32 ali_audio_info_show_thread
(
    void *param
)
{
    __s32         ret;
    __s32         idx;
    void __iomem *reg_base_snd; 
    void __iomem *reg_base_clk;         

    reg_base_snd = (void __iomem *)(ALI_PHYS2VIRT(0x18002000)); 
    reg_base_clk = (void __iomem *)(ALI_PHYS2VIRT(0x18000148));     

    printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    for (;;)
    {
        if (mutex_lock_interruptible(&ali_audio_info_show_mutex))
        {
            return(-ERESTARTSYS);
        }

        //printk("%s,%d\n", __FUNCTION__, __LINE__);

        if (ali_audio_snd_reg_show_cnt > 0)
        {
            for (idx = 0; idx < 16; idx += 4)
            {
                printk("ADDR:%x,VALUE:%x\n", ALI_VIRT2PHYS(reg_base_clk + idx),
                    ioread32(reg_base_clk + idx));
            }
        
            for (idx = 0; idx < 0x400; idx += 4)
            {
                printk("ADDR:%x,VALUE:%x\n", ALI_VIRT2PHYS(reg_base_snd + idx),
                    ioread32(reg_base_snd + idx));
            }

            ali_audio_snd_reg_show_cnt--;           
        }       

        //printk("%s,%dali_audio_snd_reg_show_cnt:%d\n", __FUNCTION__, __LINE__,
            //ali_audio_snd_reg_show_cnt);        

        if (ali_audio_see_snd_stats_show_cnt > 0)
        {
            ret = ali_audio_see_snd_info_get(&ali_audio_see_snd_info);
            
            if (ret >= 0)
            {
                ali_audio_see_snd_info_show(&ali_audio_see_snd_info); 
            }
            else
            {
                printk("%s,%d,ali_audio_see_snd_info_show() failed,ret:%d\n",
                       __FUNCTION__, __LINE__, ret);        
            }

            ali_audio_see_snd_stats_show_cnt--;     
        }

        if (ali_audio_see_ape_show_cnt > 0)
        {
            ret = ali_audio_see_ape_info_get(&ali_audio_see_ape_info);
            
            if (ret >= 0)
            {
                ali_audio_see_ape_info_show(&ali_audio_see_ape_info); 
            }
            else
            {
                printk("%s,%d,ali_audio_see_snd_info_show() failed,ret:%d\n",
                       __FUNCTION__, __LINE__, ret);        
            }

            ali_audio_see_ape_show_cnt--;           
        }       

        /* Keep going until we run out of upper limit.
        */
        if ((ali_audio_see_snd_stats_show_cnt > 0) || 
            (ali_audio_snd_reg_show_cnt > 0) ||
            (ali_audio_see_ape_show_cnt > 0))
        {
            mutex_unlock(&ali_audio_info_show_mutex);

            //printk("%s,%d\n", __FUNCTION__, __LINE__);

            /* Check dmx buffer every DMX_DATA_ENG_OUTPUT_INTERVAL miliseconds.
             * This should be enough to meet all time reqirments.
             */
            msleep_interruptible(ali_audio_info_show_interval);             
        }
        else
        {
            ali_audio_info_show_thread_exit();

            printk("%s,%d\n", __FUNCTION__, __LINE__);

            mutex_unlock(&ali_audio_info_show_mutex);
            
            break;
        }
    }

    return(0);
}


__s32 ali_audio_info_show_thread_run
(
    void
)
{
    /* If debug task does not exist, create it.
    */
    if (IS_ERR_OR_NULL(ali_audio_info_show_task_hdl))
    {
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
        ali_audio_info_show_task_hdl = kthread_create(
            ali_audio_info_show_thread, NULL, "ali_audio_info");
    
        if (IS_ERR(ali_audio_info_show_task_hdl))
        {
            return(PTR_ERR(ali_audio_info_show_task_hdl)); 
        }
    
        wake_up_process(ali_audio_info_show_task_hdl);   

        //printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);
}



static __s32 ali_audio_see_snd_stats_show_cnt_set
(
    /* Passed in by debugfs_create_file().
    */
    void *show_cnt_ptr, 

    /*set by user
    */
    __u64 setting
)
{
    printk("%s,%d,setting:%u,ali_audio_see_snd_stats_show_cnt:%u\n",
        __FUNCTION__, __LINE__, (__u32)setting, 
        ali_audio_see_snd_stats_show_cnt);

    if (mutex_lock_interruptible(&ali_audio_info_show_mutex))
    {
        return(-ERESTARTSYS);
    }

    ali_audio_see_snd_stats_show_cnt = (__u32)setting;
    
    ali_audio_info_show_thread_run();

    mutex_unlock(&ali_audio_info_show_mutex);
        
    printk("%s,%d,setting:%u,ali_audio_see_snd_stats_show_cnt:%u\n",
        __FUNCTION__, __LINE__, (__u32)setting, 
        ali_audio_see_snd_stats_show_cnt);  
    
    return(0);
}


static __s32 ali_audio_see_snd_stats_show_cnt_get
(
    /* Passed in by debugfs_create_file().
    */
    void *data,

    /* Get by user
    */
    u64 *val
)
{
    if (mutex_lock_interruptible(&ali_audio_info_show_mutex))
    {
        return(-ERESTARTSYS);
    }
    
    *val = ali_audio_see_snd_stats_show_cnt;

    mutex_unlock(&ali_audio_info_show_mutex);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ali_audio_see_snd_stats_show_cnt_fops, 
    ali_audio_see_snd_stats_show_cnt_get, ali_audio_see_snd_stats_show_cnt_set, 
    "%llu\n");



static __s32 ali_audio_snd_reg_show_cnt_set
(
    void *reg_show_cnt_ptr,
    u64   setting /*set by user*/
)
{
    printk("%s,%d,setting:%u,ali_audio_snd_reg_show_cnt:%u\n",
        __FUNCTION__, __LINE__, (__u32)setting, ali_audio_snd_reg_show_cnt);

    if (mutex_lock_interruptible(&ali_audio_info_show_mutex))
    {
        return(-ERESTARTSYS);
    }

    ali_audio_snd_reg_show_cnt = (__u32)setting;
        
    ali_audio_info_show_thread_run();

    mutex_unlock(&ali_audio_info_show_mutex);
        
    printk("%s,%d,setting:%u,ali_audio_snd_reg_show_cnt:%u\n",
        __FUNCTION__, __LINE__, (__u32)setting, ali_audio_snd_reg_show_cnt);    
    
    return(0);  

}


static __s32 ali_audio_snd_reg_show_cnt_get
(
    void *data,
    u64 *val
)
{
    *val = *(u32 *)data;

    return 0;
}


DEFINE_SIMPLE_ATTRIBUTE(ali_audio_reg_show_cnt_fops, 
    ali_audio_snd_reg_show_cnt_get, ali_audio_snd_reg_show_cnt_set, "%llu\n");


static __s32 ali_audio_see_ape_show_cnt_set
(
    void *reg_show_cnt_ptr,
    u64   setting /*set by user*/
)
{
    //printk("%s,%d,setting:%u,ali_audio_snd_reg_show_cnt:%u\n",
        //__FUNCTION__, __LINE__, (__u32)setting, ali_audio_snd_reg_show_cnt);

    if (mutex_lock_interruptible(&ali_audio_info_show_mutex))
    {
        return(-ERESTARTSYS);
    }

    ali_audio_see_ape_show_cnt = (__u32)setting;
        
    ali_audio_info_show_thread_run();

    mutex_unlock(&ali_audio_info_show_mutex);
        
    //printk("%s,%d,setting:%u,ali_audio_snd_reg_show_cnt:%u\n",
        //__FUNCTION__, __LINE__, (__u32)setting, ali_audio_snd_reg_show_cnt);    
    
    return(0);  

}


static __s32 ali_audio_see_ape_show_cnt_get
(
    void *data,
    u64 *val
)
{
    *val = *(u32 *)data;

    return 0;
}


DEFINE_SIMPLE_ATTRIBUTE(ali_audio_see_ape_show_cnt_fops, 
    ali_audio_see_ape_show_cnt_get, ali_audio_see_ape_show_cnt_set, "%llu\n");


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
__s32 ali_audio_info_exit
(
    void 
)
{
    printk("%s,%d\n", __FUNCTION__, __LINE__);

    debugfs_remove_recursive(ali_audio_debugfs_root);
    ali_audio_show_en = 0;

    printk("%s,%d\n", __FUNCTION__, __LINE__);

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
__s32 ali_audio_info_init
(
    void 
)
{
    struct dentry *fs_entry;

    printk("%s,%d,Go\n", __FUNCTION__, __LINE__);

    mutex_init(&ali_audio_info_show_mutex);
    
    ali_audio_debugfs_root = debugfs_create_dir("ali_audio", NULL);
    
    if (!ali_audio_debugfs_root)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        return(-ENOENT);
    }

    /* For ali audio debug show enable/disable.
    */
    fs_entry = debugfs_create_u32("api_show_en", 0644, ali_audio_debugfs_root,
                                  &ali_audio_show_en);
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    } 

    printk("%s,%d,succeed.\n", __FUNCTION__, __LINE__); 

    fs_entry = debugfs_create_file("snd_reg_show_cnt", 0644,
        ali_audio_debugfs_root, &ali_audio_snd_reg_show_cnt, 
        &ali_audio_reg_show_cnt_fops);
    
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }   
    
    printk("%s,%d,succeed.\n", __FUNCTION__, __LINE__);

    fs_entry = debugfs_create_file("see_snd_stats_show_cnt", 0644,
        ali_audio_debugfs_root, &ali_audio_see_snd_stats_show_cnt, 
        &ali_audio_see_snd_stats_show_cnt_fops);
    
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }   

    printk("%s,%d,succeed.\n", __FUNCTION__, __LINE__); 

    fs_entry = debugfs_create_file("see_ape_show_cnt", 0644,
        ali_audio_debugfs_root, &ali_audio_see_ape_show_cnt, 
        &ali_audio_see_ape_show_cnt_fops);
    
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }   

    printk("%s,%d,succeed.\n", __FUNCTION__, __LINE__); 
    
    return(0);

Fail:
    printk("%s,%d,failed\n", __FUNCTION__, __LINE__);
    ali_audio_info_exit();
    return(-ENOENT);    
}



