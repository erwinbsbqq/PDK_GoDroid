


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h> 
#include <asm/io.h>
#include <ali_interrupt.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>

#include "dmx_see_interface.h"
#include "dmx_stack.h"
#include "dmx_dbg.h"


static 	struct mutex ali_dmx_dbg_mutex;
wait_queue_head_t ali_dmx_dbg_wq;
static 	struct task_struct *ali_dmx_dbg_show_thread_ptr;
static struct dentry *ali_dmx_dbg_debugfs_root;
static __u32 ali_dmx_dbg_see_show_cnt = 0;
static __u32 ali_dmx_dbg_legacy_ch_api_show_en = 0;
static __u32 ali_dmx_dbg_see_show_interval = 3000;

struct Ali_DmxSeeGlobalStatInfo dmx_see_glb_statistics;
struct Ali_DmxSeePlyChStatInfo dmx_see_ch_statistics[DMX_SEE_CH_CNT];


static INT32 dmx_dbg_see_ch_info_show
(
	struct Ali_DmxSeePlyChStatInfo *dmx_see_ch_stat_Info,
	__s32                           ch_cnt
)
{
	__s32 idx;

    for (idx = 0; idx < ch_cnt; idx++)
    {
	    //dmx_see_ch_stat_Info = &ali_dmx_see_dev[0].see_buf_init.PlyChStatInfo[Idx];
	    dmx_see_ch_stat_Info = &dmx_see_ch_statistics[idx];
    
        if (dmx_see_ch_stat_Info->PlyChPid > 0x1FFF)
        {
	        DMX_STATIS_SHOW("\nSEE Channel[%d] PID not configured, skip it.\n", idx);  

			continue;
		}

    	DMX_STATIS_SHOW("\nSEE Play Channel[%d] Statistics, PID:%u\n", idx, dmx_see_ch_stat_Info->PlyChPid);
    
    	DMX_STATIS_SHOW("------------------------------------\n");
    
    	DMX_STATIS_SHOW("TsInCnt:%u\n", dmx_see_ch_stat_Info->TsInCnt);
    
    	DMX_STATIS_SHOW("PesHdrCnt:%u\n", dmx_see_ch_stat_Info->PesHdrCnt);
    
    	if (dmx_see_ch_stat_Info->TsPlayBusyCnt != 0)
    	{
    		DMX_STATIS_SHOW("TsPlayBusyCnt:%u\n", dmx_see_ch_stat_Info->TsPlayBusyCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->TsScrmbCnt != 0)
    	{
    		DMX_STATIS_SHOW("TsScrmbCnt:%u\n", dmx_see_ch_stat_Info->TsScrmbCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->TsErrCnt != 0)
    	{
    		DMX_STATIS_SHOW("TsErrCnt:%u\n", dmx_see_ch_stat_Info->TsErrCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesTsErrCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesTsErrCnt:%u\n", dmx_see_ch_stat_Info->PesTsErrCnt);
    	}
    			
    	if (dmx_see_ch_stat_Info->PesTsScrmbCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesTsScrmbCnt:%u\n", dmx_see_ch_stat_Info->PesTsScrmbCnt);
    	}
    			
    	if (dmx_see_ch_stat_Info->PesTsDupCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesTsDupCnt:%u\n", dmx_see_ch_stat_Info->PesTsDupCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesTsLostCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesTsLostCnt:%u\n", dmx_see_ch_stat_Info->PesTsLostCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesHdrLenErrCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesHdrLenErrCnt:%u\n", dmx_see_ch_stat_Info->PesHdrLenErrCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesHdrScErrCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesHdrScErrCnt:%u\n", dmx_see_ch_stat_Info->PesHdrScErrCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesStreamIdErrCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesStreamIdErrCnt:%u\n", dmx_see_ch_stat_Info->PesStreamIdErrCnt);
    	}
    			
    	if (dmx_see_ch_stat_Info->PesScrmbCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesScrmbCnt:%u\n", dmx_see_ch_stat_Info->PesScrmbCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesHdrPayloadLenErrCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesHdrPayloadLenErr:%u\n", dmx_see_ch_stat_Info->PesHdrPayloadLenErrCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesCallbackNobufCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesCallbackNobufCnt:%u\n", dmx_see_ch_stat_Info->PesCallbackNobufCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesReqBufBusyCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesReqBufBusyCnt:%u\n", dmx_see_ch_stat_Info->PesReqBufBusyCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesReqDecStateErrCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesReqDecStateErrCnt:%u\n", dmx_see_ch_stat_Info->PesReqDecStateErrCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesTsNoPayloadCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesTsNoPayloadCnt:%u\n", dmx_see_ch_stat_Info->PesTsNoPayloadCnt);
    	}
    
    	if (dmx_see_ch_stat_Info->PesBufOverflowCnt != 0)
    	{
    		DMX_STATIS_SHOW("PesBufOverflowCnt:%u\n", dmx_see_ch_stat_Info->PesBufOverflowCnt);
    	}
    
    	DMX_STATIS_SHOW("PesBufReqCnt:%u\n", dmx_see_ch_stat_Info->PesBufReqCnt);
    
    	DMX_STATIS_SHOW("PesBufUpdateCnt:%u\n", dmx_see_ch_stat_Info->PesBufUpdateCnt);		
	}
	
	return (0);
}



static INT32 dmx_dbg_see_glb_show
(
	struct Ali_DmxSeeGlobalStatInfo *glb_inf
)
{
	DMX_STATIS_SHOW("\nSEE Global:\n");
	
	DMX_STATIS_SHOW("-----------------------------------------------\n");

	DMX_STATIS_SHOW("ParseStatus:%u\n", glb_inf->ParseStatus);
	DMX_STATIS_SHOW("TotalTsInCnt:%u\n", glb_inf->TotalTsInCnt);

	if (glb_inf->TsOddCnt != 0)
	{
	    DMX_STATIS_SHOW("TsOddCnt:%u\n", glb_inf->TsOddCnt);
	}
	
	if (glb_inf->TsSyncErrCnt != 0)
	{
    	DMX_STATIS_SHOW("TsSyncErrCnt:%u\n", glb_inf->TsSyncErrCnt);
	}
	
    DMX_STATIS_SHOW("TsDecrySucCnt:%u\n", glb_inf->TsDecrySucCnt);

	if (glb_inf->TsDecryFailCnt != 0)
	{
	    DMX_STATIS_SHOW("TsDecryFailCnt:%u\n", glb_inf->TsDecryFailCnt);
	}
	
	if (glb_inf->TsDecryEmptyCnt != 0)
	{
	    DMX_STATIS_SHOW("TsDecryEmptyCnt:%u\n", glb_inf->TsDecryEmptyCnt);
	}

	if (glb_inf->Pcr2AvsyncCnt != 0)
	{
	    DMX_STATIS_SHOW("Pcr2AvsyncCnt:%u\n", glb_inf->Pcr2AvsyncCnt);
	}
	
	//if (glb_inf->PcrLastValue != 0)
	{
	    DMX_STATIS_SHOW("PcrLastValue:0x%x\n", glb_inf->PcrLastValue);
	}

	DMX_STATIS_SHOW("TsRetToMainCnt:%u\n", glb_inf->TsRetToMainCnt);
	
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
__s32 dmx_dbg_ch_legacy_api_show(char *fmt, ...)
{
    va_list args;
    __s32   ret;

    if (1 != ali_dmx_dbg_legacy_ch_api_show_en)
    {
        return(-__LINE__);

    }
 
    va_start(args, fmt);
    ret = vprintk(fmt, args);
    va_end(args);

    return(ret);
}




static __s32 ali_dmx_dbg_show_thread
(
    void *param
)
{
    __s32 ret;
	
    for(;;)
    {
        if (mutex_lock_interruptible(&ali_dmx_dbg_mutex))
        {
            return(-ERESTARTSYS);
        }

        /* Wait until we are allowed to show debug info.
		*/
		while (ali_dmx_dbg_see_show_cnt <= 0)
		{
            mutex_unlock(&ali_dmx_dbg_mutex);

	        if (wait_event_interruptible(ali_dmx_dbg_wq, 
                                         (ali_dmx_dbg_see_show_cnt > 0)))
            {
                return(-ERESTARTSYS);
            }
			
            if (mutex_lock_interruptible(&ali_dmx_dbg_mutex))
            {
                return(-ERESTARTSYS);
            }				
		}
		
	    ret = dmx_see_glb_info_get(&dmx_see_glb_statistics);
		
		if (ret >= 0)
		{
    	    dmx_dbg_see_glb_show(&dmx_see_glb_statistics); 
		}
		else
		{
            printk("%s,%d,dmx_see_glb_info_get() failed,ret:%d\n",
				   __FUNCTION__, __LINE__, ret);
		}		
			
	    ret = dmx_see_ch_info_get(&dmx_see_ch_statistics[0], DMX_SEE_CH_CNT);

		if (ret >= 0)
		{
    	    dmx_dbg_see_ch_info_show(&dmx_see_ch_statistics[0], DMX_SEE_CH_CNT); 
		}
		else
		{
            printk("%s,%d,dmx_see_ch_info_get() failed,ret:%d\n",
				   __FUNCTION__, __LINE__, ret);
		}
			
		ali_dmx_dbg_see_show_cnt--;

        mutex_unlock(&ali_dmx_dbg_mutex);
		
        /* Check dmx buffer every DMX_DATA_ENG_OUTPUT_INTERVAL miliseconds.
         * This should be enough to meet all time reqirments.
         */
        msleep_interruptible(ali_dmx_dbg_see_show_interval);				
    }

	return(0);
}




static int dmx_dbg_see_show_cnt_set(void *data, u64 val)
{
    if (mutex_lock_interruptible(&ali_dmx_dbg_mutex))
    {
        return(-ERESTARTSYS);
    }
	
    *(u32 *)data = val;
	
    wake_up_interruptible(&ali_dmx_dbg_wq);
	
	mutex_unlock(&ali_dmx_dbg_mutex);
	
    return(0);

}

static int dmx_dbg_see_show_cnt_get(void *data, u64 *val)
{
    *val = *(u32 *)data;

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(dmx_dbg_see_show_cnt_fops, dmx_dbg_see_show_cnt_get,
	                    dmx_dbg_see_show_cnt_set, "%llu\n");



__s32 dmx_statistic_show_init
(
    void 
)
{

	struct dentry *fs_entry;

	mutex_init(&ali_dmx_dbg_mutex);
	init_waitqueue_head(&ali_dmx_dbg_wq);
	
	ali_dmx_dbg_debugfs_root = debugfs_create_dir("ali_dmx", NULL);
	
	if (!ali_dmx_dbg_debugfs_root)
	{
	    printk("%s,%d\n", __FUNCTION__, __LINE__);
		return(-ENOENT);
	}

	fs_entry = debugfs_create_u32("ch_legacy_api_show_en", 0644, ali_dmx_dbg_debugfs_root,
		                           &ali_dmx_dbg_legacy_ch_api_show_en);
	if (!fs_entry)
	{
	    printk("%s,%d\n", __FUNCTION__, __LINE__);
		goto Fail;	
	}	

	fs_entry = debugfs_create_u32("see_info_show_interval", 0644, ali_dmx_dbg_debugfs_root,
		                           &ali_dmx_dbg_see_show_interval);
	if (!fs_entry)
	{
	    printk("%s,%d\n", __FUNCTION__, __LINE__);
		goto Fail;	
	}
	
    fs_entry = debugfs_create_file("see_info_show_cnt", 0644, ali_dmx_dbg_debugfs_root,
                                   &ali_dmx_dbg_see_show_cnt, &dmx_dbg_see_show_cnt_fops);
	if (!fs_entry)
	{
	    printk("%s,%d\n", __FUNCTION__, __LINE__);
		goto Fail;	
	}
	
	ali_dmx_dbg_show_thread_ptr = kthread_create(ali_dmx_dbg_show_thread, NULL, "dmx_dbg");

	if (IS_ERR(ali_dmx_dbg_show_thread_ptr))
	{
	    printk("%s,%d\n", __FUNCTION__, __LINE__);
		return(PTR_ERR(ali_dmx_dbg_show_thread_ptr));	
	}

	wake_up_process(ali_dmx_dbg_show_thread_ptr);

	return(0);

Fail:
	//printk("%s,%d\n", __FUNCTION__, __LINE__);
	
	debugfs_remove_recursive(ali_dmx_dbg_debugfs_root);
	ali_dmx_dbg_debugfs_root = NULL;
	return(-ENOENT);	
}




