/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved.
 *  (C)
 *  File: ali_avsync_procfs.c
 *  (I)
 *  Description: debug fs for aliavsync
 *  (S)
 *  History:(M)
 *  Version     Date        		Author         		Comment
 *  ======   	=======        	======			=======
 * 	0.		2014.07.31		Andreaw			Create
 ****************************************************************************/
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
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/poll.h>

#include <linux/ali_rpc.h>
#include <ali_avsync_common.h>
#include <rpc_hld/ali_rpc_hld_avsync.h>


/*Proc based contron intertace*/
#define AVSYNC_DEBUG_PROC_DIR "aliavsync"
#define AVSYNC_DEBUG_PROC_LEVEL "debuglevel"
#define AVSYNC_DEBUG_PROC_INFO "debuginfo"
#define MAX_BUF_LEN 1000 //do not bigger than one page size 1024 bytes

static struct proc_dir_entry *avsync_proc_dir = NULL;
static struct proc_dir_entry *avsync_proc_debug_file = NULL;
static struct proc_dir_entry *avsync_proc_dbginfo_file = NULL;

static struct avsync_device *avsync_dbg_dev = NULL;
static int avsync_debug_level =0;

static int avsync_read_debug_info(char * buffer)
{
	int len = 0;
	AVSYNC_MODE_E mode = 0;
	AVSYNC_SRCTYPE_E src_type = 0 ;
	avsync_cfg_param_t cfg_params;
	avsync_adv_param_t adv_cfg_params;
	avsync_status_t status;
	avsync_statistics_t statistics;
	struct avsync_smoothly_play_cfg_t cfg;

	if(avsync_dbg_dev == NULL)
	{
		return 0;
	}

	/*Local debug info*/
	len += sprintf(&buffer[len],"************Avsync Info List***********\n");

	if(RET_SUCCESS == avsync_get_syncmode(avsync_dbg_dev,&mode))
	{
		len += sprintf(&buffer[len],"avsync sync mode : %d\n",mode);
	}
	
	if(RET_SUCCESS == avsync_get_sourcetype(avsync_dbg_dev,&src_type))
	{	
		len += sprintf(&buffer[len],"avsync src type : %d\n\n",src_type);
	}
	
	
	if(RET_SUCCESS == avsync_get_params(avsync_dbg_dev,&cfg_params))
	{
		len += sprintf(&buffer[len],"[cfg params list]\n");
		len += sprintf(&buffer[len],"vhold_thres : %d\n",(int)(cfg_params.vhold_thres));
		len += sprintf(&buffer[len],"vdrop_thres : %d\n",(int)(cfg_params.vdrop_thres));
		len += sprintf(&buffer[len],"ahold_thres : %d\n",(int)(cfg_params.ahold_thres));
		len += sprintf(&buffer[len],"adrop_thres : %d\n",(int)(cfg_params.adrop_thres));
		len += sprintf(&buffer[len],"sync mode : %d\n",(int)(cfg_params.sync_mode));
		len += sprintf(&buffer[len],"src_type : %d\n\n",(int)(cfg_params.src_type));
	}	

	if(RET_SUCCESS == avsync_get_advance_params(avsync_dbg_dev,&adv_cfg_params))
	{	
		len += sprintf(&buffer[len],"[advanced cfg params list]\n");
		len += sprintf(&buffer[len],"afreerun_thres : %d\n",(int)(adv_cfg_params.afreerun_thres));
		len += sprintf(&buffer[len],"vfreerun_thres : %d\n",(int)(adv_cfg_params.vfreerun_thres));
		len += sprintf(&buffer[len],"disable_first_video_freerun : %d\n",(int)(adv_cfg_params.disable_first_video_freerun));
		len += sprintf(&buffer[len],"dual_output_sd_delay : %d\n",(int)(adv_cfg_params.dual_output_sd_delay));
		len += sprintf(&buffer[len],"pts_adjust_threshold : %d\n\n",(int)(adv_cfg_params.pts_adjust_threshold));
	}	
	
	if(RET_SUCCESS == avsync_get_status(avsync_dbg_dev,&status))
	{	
		len += sprintf(&buffer[len],"[avsync status list]\n");
		len += sprintf(&buffer[len],"device_status : %d\n",(int)(status.device_status));
		len += sprintf(&buffer[len],"vpts_offset : %d\n",(int)(status.vpts_offset));
		len += sprintf(&buffer[len],"apts_offset : %d\n",(int)(status.apts_offset));
		len += sprintf(&buffer[len],"v_sync_flg : %d\n",(int)(status.v_sync_flg));
		len += sprintf(&buffer[len],"a_sync_flg : %d\n",(int)(status.a_sync_flg));
		len += sprintf(&buffer[len],"cur_vpts : 0x%x\n",(int)(status.cur_vpts));
		len += sprintf(&buffer[len],"cur_apts : 0x%x\n\n",(int)(status.cur_apts));		
	}	
	
	if(RET_SUCCESS == avsync_get_statistics(avsync_dbg_dev,&statistics))
	{	
		len += sprintf(&buffer[len],"[avsync statistics list]\n");
		len += sprintf(&buffer[len],"total_v_play_cnt : %d\n",(int)(statistics.total_v_play_cnt));
		len += sprintf(&buffer[len],"total_v_drop_cnt : %d\n",(int)(statistics.total_v_drop_cnt));
		len += sprintf(&buffer[len],"total_v_hold_cnt : %d\n",(int)(statistics.total_v_hold_cnt));
		len += sprintf(&buffer[len],"total_v_freerun_cnt : %d\n",(int)(statistics.total_v_freerun_cnt));
		len += sprintf(&buffer[len],"total_a_play_cnt : %d\n",(int)(statistics.total_a_play_cnt));
		len += sprintf(&buffer[len],"total_a_drop_cnt : %d\n",(int)(statistics.total_a_drop_cnt));
		len += sprintf(&buffer[len],"total_a_hold_cnt : %d\n",(int)(statistics.total_a_hold_cnt));		
		len += sprintf(&buffer[len],"total_a_freerun_cnt : %d\n\n",(int)(statistics.total_a_freerun_cnt));	
	}
	
	if(RET_SUCCESS == avsync_ioctl(avsync_dbg_dev,AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG,(int)&cfg))
	{
		len += sprintf(&buffer[len],"[video smooth play feature list]\n");
		len += sprintf(&buffer[len],"onoff : %d\n",(int)(cfg.onoff));		
		len += sprintf(&buffer[len],"interval :%d\n",(int)(cfg.interval));		
		len += sprintf(&buffer[len],"level : %d\n\n",(int)(cfg.level));		
	}
#if 0
	printk("mode %d\n",mode);
	printk("src_type %d\n",src_type);
	printk("vhold_thres : %d\n",cfg_params.vhold_thres);
	printk("vdrop_thres : %d\n",cfg_params.vdrop_thres);
	printk("ahold_thres : %d\n",cfg_params.ahold_thres);
	printk("adrop_thres : %d\n",cfg_params.adrop_thres);
	printk("sync mode : %d\n",cfg_params.sync_mode);
	printk("src_type : %d\n\n",cfg_params.src_type);
	printk("afreerun_thres : %d\n",adv_cfg_params.afreerun_thres);
	printk("vfreerun_thres : %d\n",adv_cfg_params.vfreerun_thres);
	printk("disable_first_video_freerun : %d\n",adv_cfg_params.disable_first_video_freerun);
	printk("dual_output_sd_delay : %d\n",adv_cfg_params.dual_output_sd_delay);
	printk("pts_adjust_threshold : %d\n\n",adv_cfg_params.pts_adjust_threshold);
	printk("device_status : %d\n",status.device_status);
	printk("vpts_offset : %d\n",status.vpts_offset);
	printk("apts_offset : %d\n",status.apts_offset);
	printk("v_sync_flg : %d\n",status.v_sync_flg);
	printk("a_sync_flg : %d\n",status.a_sync_flg);
	printk("cur_vpts : 0x%x\n",status.cur_vpts);
	printk("cur_apts : 0x%x\n\n",status.cur_apts);
	printk("[avsync statistics list]\n");
	printk("total_v_play_cnt : %d\n",statistics.total_v_play_cnt);
	printk("total_v_drop_cnt : %d\n",statistics.total_v_drop_cnt);
	printk("total_v_hold_cnt : %d\n",statistics.total_v_hold_cnt);
	printk("total_v_freerun_cnt : %d\n",statistics.total_v_freerun_cnt);
	printk("total_a_play_cnt : %d\n",statistics.total_a_play_cnt);
	printk("total_a_drop_cnt : %d\n",statistics.total_a_drop_cnt);
	printk("total_a_hold_cnt : %d\n",statistics.total_a_hold_cnt);		
	printk("total_a_freerun_cnt : %d\n\n",statistics.total_a_freerun_cnt);
	printk("[video smooth play feature list]\n");
	printk("onoff : %d\n",cfg.onoff);		
	printk("interval :%d\n",cfg.interval);		
	printk("level : %d\n\n",cfg.level); 
#endif
	return len;
}

/*Process debug info*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static ssize_t avsync_dbginfo_procfile_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
	int len = 0;
	char buffer[MAX_BUF_LEN] = {0,};

	len = avsync_read_debug_info(buffer);

	return simple_read_from_buffer(ubuf, size, ppos, buffer, len);
}

static ssize_t avsync_dbginfo_procfile_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	return count;
}
#else
static int avsync_dbginfo_procfile_read(char*buffer, char**buffer_localation, off_t offset,int buffer_length,int* eof, void *data )
{
	int len = 0;
	len =  avsync_read_debug_info(buffer);
	*eof = 1;
        return len;
}

static int avsync_dbginfo_procfile_write(struct file *filp, const char *buffer,unsigned long count,void *data)
{
        return count;
}
#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static ssize_t avsync_debuglevel_procfile_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
	char buffer[MAX_BUF_LEN] = {0,};
	int len = 0;
	len += sprintf(&buffer[len],"debuglevel=0x%08x\n",avsync_debug_level);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_DEFAULT=0x%08x\n",AVSYNC_DBG_PRINT_DEFAULT);
	
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_PCR=0x%08x\n",AVSYNC_DBG_PRINT_PCR);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_APTS=0x%08x\n",AVSYNC_DBG_PRINT_APTS);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_VPTS=0x%08x\n",AVSYNC_DBG_PRINT_VPTS);
	
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_A_SYNC=0x%08x\n",AVSYNC_DBG_PRINT_A_SYNC);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_A_UNSYNC_PTS_STC=0x%08x\n",AVSYNC_DBG_PRINT_A_UNSYNC_PTS_STC);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_A_UNSYNC_STCID_OFFSET=0x%08x\n",AVSYNC_DBG_PRINT_A_UNSYNC_STCID_OFFSET);

	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_V_SYNC=0x%08x\n",AVSYNC_DBG_PRINT_V_SYNC);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_V_UNSYNC_PTS_STC=0x%08x\n",AVSYNC_DBG_PRINT_V_UNSYNC_PTS_STC);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_V_UNSYNC_STCID_OFFSET=0x%08x\n",AVSYNC_DBG_PRINT_V_UNSYNC_STCID_OFFSET);

	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_PTS_OFFSET=0x%08x\n",AVSYNC_DBG_PRINT_PTS_OFFSET);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_STREAM_LOOP=0x%08x\n",AVSYNC_DBG_PRINT_STREAM_LOOP);

	len += sprintf(&buffer[len],"AVSYNC_DBG_FORCE_SYNCMODE_FREERUN=0x%08x\n",AVSYNC_DBG_FORCE_SYNCMODE_FREERUN);
	len += sprintf(&buffer[len],"AVSYNC_DBG_FORCE_SYNCMODE_PCR=0x%08x\n",AVSYNC_DBG_FORCE_SYNCMODE_PCR);
	len += sprintf(&buffer[len],"AVSYNC_DBG_FORCE_SYNCMODE_AUDIO=0x%08x\n",AVSYNC_DBG_FORCE_SYNCMODE_AUDIO);
	len += sprintf(&buffer[len],"AVSYNC_DBG_FORCE_SYNCMODE_INVALID=0x%08x\n",AVSYNC_DBG_FORCE_SYNCMODE_INVALID);

	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_API=0x%08x\n",AVSYNC_DBG_PRINT_API);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_LOG=0x%08x\n",AVSYNC_DBG_PRINT_LOG);

	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_ERR=0x%08x\n",AVSYNC_DBG_PRINT_ERR);	
	return simple_read_from_buffer(ubuf, size, ppos, buffer, len);
}

static ssize_t avsync_debuglevel_procfile_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	char buf[MAX_BUF_LEN] = {0,};

	if(count<=0 || count > MAX_BUF_LEN)
	  return 0;
	
	if (copy_from_user(buf, buffer, count))  
	  return -EFAULT;  

	if (sscanf(buf, "debuglevel=0x%08x",&avsync_debug_level) != 1)
	{
		return 0;
	}
	
	avsync_dbg_set_print_option(avsync_dbg_dev, avsync_debug_level);
	
	return count;

}
#else
static int avsync_debuglevel_procfile_read(char*buffer, char**buffer_localation, off_t offset,int buffer_length,int* eof, void *data )
{
	int len = 0;
	len += sprintf(&buffer[len],"debuglevel=0x%08x\n",avsync_debug_level);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_DEFAULT=0x%08x\n",AVSYNC_DBG_PRINT_DEFAULT);
	
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_PCR=0x%08x\n",AVSYNC_DBG_PRINT_PCR);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_APTS=0x%08x\n",AVSYNC_DBG_PRINT_APTS);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_VPTS=0x%08x\n",AVSYNC_DBG_PRINT_VPTS);
	
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_A_SYNC=0x%08x\n",AVSYNC_DBG_PRINT_A_SYNC);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_A_UNSYNC_PTS_STC=0x%08x\n",AVSYNC_DBG_PRINT_A_UNSYNC_PTS_STC);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_A_UNSYNC_STCID_OFFSET=0x%08x\n",AVSYNC_DBG_PRINT_A_UNSYNC_STCID_OFFSET);

	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_V_SYNC=0x%08x\n",AVSYNC_DBG_PRINT_V_SYNC);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_V_UNSYNC_PTS_STC=0x%08x\n",AVSYNC_DBG_PRINT_V_UNSYNC_PTS_STC);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_V_UNSYNC_STCID_OFFSET=0x%08x\n",AVSYNC_DBG_PRINT_V_UNSYNC_STCID_OFFSET);

	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_PTS_OFFSET=0x%08x\n",AVSYNC_DBG_PRINT_PTS_OFFSET);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_STREAM_LOOP=0x%08x\n",AVSYNC_DBG_PRINT_STREAM_LOOP);

	len += sprintf(&buffer[len],"AVSYNC_DBG_FORCE_SYNCMODE_FREERUN=0x%08x\n",AVSYNC_DBG_FORCE_SYNCMODE_FREERUN);
	len += sprintf(&buffer[len],"AVSYNC_DBG_FORCE_SYNCMODE_PCR=0x%08x\n",AVSYNC_DBG_FORCE_SYNCMODE_PCR);
	len += sprintf(&buffer[len],"AVSYNC_DBG_FORCE_SYNCMODE_AUDIO=0x%08x\n",AVSYNC_DBG_FORCE_SYNCMODE_AUDIO);
	len += sprintf(&buffer[len],"AVSYNC_DBG_FORCE_SYNCMODE_INVALID=0x%08x\n",AVSYNC_DBG_FORCE_SYNCMODE_INVALID);

	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_API=0x%08x\n",AVSYNC_DBG_PRINT_API);
	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_LOG=0x%08x\n",AVSYNC_DBG_PRINT_LOG);

	len += sprintf(&buffer[len],"AVSYNC_DBG_PRINT_ERR=0x%08x\n",AVSYNC_DBG_PRINT_ERR);
	*eof  = 1;
	return len;
}

static int avsync_debuglevel_procfile_write(struct file *filp, const char *buffer,unsigned long count,void *data)
{
	char buf[MAX_BUF_LEN] = {0,};

	if(count<=0 || count > MAX_BUF_LEN)
	  return 0;
	
	if (copy_from_user(buf, buffer, count))  
	  return -EFAULT;  

	if (sscanf(buf, "debuglevel=0x%08x",&avsync_debug_level) != 1)
	{
		return 0;
	}
	
	avsync_dbg_set_print_option(avsync_dbg_dev, avsync_debug_level);
	
	return count;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static const struct file_operations aliavsync_debuglevel_fops = {
	.read = avsync_debuglevel_procfile_read,
	.write = avsync_debuglevel_procfile_write,
	.llseek = default_llseek,
};


static const struct file_operations aliavsync_debuginfo_fops = {
	.read = avsync_dbginfo_procfile_read,
	.write = avsync_dbginfo_procfile_write,
	.llseek = default_llseek,
};
#endif

int  avsync_debug_procfs_init(void)
{
     	avsync_proc_dir = proc_mkdir(AVSYNC_DEBUG_PROC_DIR, NULL);  

     	if (avsync_proc_dir == NULL) {  
		printk("avsync_debug_procfs_init create dir aliavsync failed!!\n");
		return -1;
     	} 

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
        avsync_proc_debug_file = proc_create(AVSYNC_DEBUG_PROC_LEVEL,0644,avsync_proc_dir, &aliavsync_debuglevel_fops);
#else
        avsync_proc_debug_file = create_proc_entry(AVSYNC_DEBUG_PROC_LEVEL,0644,avsync_proc_dir);
#endif

        if(avsync_proc_debug_file == NULL)
        {
                remove_proc_entry(AVSYNC_DEBUG_PROC_DIR, NULL);
                printk("Error:could not initialize /proc/%s/%s\n",AVSYNC_DEBUG_PROC_DIR,AVSYNC_DEBUG_PROC_LEVEL);
                return -1;
        }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))        
        avsync_proc_debug_file->read_proc = avsync_debuglevel_procfile_read;
        avsync_proc_debug_file->write_proc = avsync_debuglevel_procfile_write;
#endif


#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 10, 0))     
	/*For Debug info*/
        avsync_proc_dbginfo_file = proc_create(AVSYNC_DEBUG_PROC_INFO,0644,avsync_proc_dir, &aliavsync_debuginfo_fops);
#else
/*For Debug info*/
	avsync_proc_dbginfo_file = create_proc_entry(AVSYNC_DEBUG_PROC_INFO,0644,avsync_proc_dir);
#endif

        if(avsync_proc_dbginfo_file == NULL)
        {
		remove_proc_entry(AVSYNC_DEBUG_PROC_LEVEL, avsync_proc_dir);
                remove_proc_entry(AVSYNC_DEBUG_PROC_LEVEL, NULL);
                printk("Error:could not initialize /proc/%s/%s\n",AVSYNC_DEBUG_PROC_DIR,AVSYNC_DEBUG_PROC_INFO);
                return -1;
        }
		
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))        
        avsync_proc_dbginfo_file->read_proc = avsync_dbginfo_procfile_read;
        avsync_proc_dbginfo_file->write_proc = avsync_dbginfo_procfile_write;
#endif

	avsync_dbg_dev = (struct avsync_device*)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_AVSYNC);
        return 0;
}


void  avsync_debug_procfs_exit(void)
{
        remove_proc_entry(AVSYNC_DEBUG_PROC_LEVEL, avsync_proc_dir);
        remove_proc_entry(AVSYNC_DEBUG_PROC_INFO, avsync_proc_dir);
	 remove_proc_entry(AVSYNC_DEBUG_PROC_DIR, NULL);
}
