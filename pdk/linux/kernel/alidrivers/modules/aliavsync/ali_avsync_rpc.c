/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: ali_avsync.c
 *  (I)
 *  Description: ali video player
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2009.12.24			Sam			Create
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


struct sharem_stc_info
{
	UINT8 stc_updated;
	UINT8 stc_id;
	UINT8 stc_offset_idx;
	UINT8 stc_valid;
	UINT32 stc;
};

struct sharem_ctrl_blk_info
{
	UINT8 run_back_flag;
	UINT8 stc_id;
	UINT8 stc_offset_idx;
	UINT8 updating;
};

struct avsync_sharem_info
{
	struct sharem_stc_info stc_info;
	struct sharem_ctrl_blk_info ctrl_blk_info;	
};


#if 0
#define AVSYNC_PRF(arg, value...)  \
			{\
				printk("<0>""kernel debug : file : %s function : %s\n", __FILE__, __FUNCTION__);\
				printk("<0>"arg, ##value);\
				printk("<0>""kernel debug done\n\n");\
			}
#else
#define AVSYNC_PRF(...)					do{}while(0)
#endif

#if 0
#define AVSYNC_ERROR_PRF(arg, value...)  \
			{\
				printk("kernel debug : file : %s function : %s\n", __FILE__, __FUNCTION__);\
				printk(arg, ##value);\
				printk("kernel debug done\n\n");\
			}
#else
#define AVSYNC_ERROR_PRF(...)					do{}while(0)
#endif

static dev_t m_avsync_dev_t = 0;
static struct cdev *m_avsync_cdev = NULL;
static struct class *m_avysnc_class = NULL;
static struct device *m_avysnc_device = NULL;
static int8 g_avsync_main_get_stc_en = 0;
static volatile struct avsync_sharem_info *gp_avsync_sharem_info;
static volatile INT64 *gp_avsync_vpts= NULL;

static struct semaphore m_avsync_sem;
static int m_avsync_open_cnt = 0;

static struct avsync_device *m_avsync_see_dev = NULL;

static ssize_t ali_avsync_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	return -1;
}

static ssize_t ali_avsync_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	return -1;
}

RET_CODE avsync_get_video_pts(struct avsync_device *dev, UINT32 io_addr)
{
	RET_CODE ret=RET_FAILURE;
	RET_CODE copy_ret = 0;
	
	if(gp_avsync_vpts == NULL)
	{
		gp_avsync_vpts = ali_rpc_malloc_shared_mm(sizeof(INT64));
		if(gp_avsync_vpts == NULL)
		{
			printk("<0>""%s error line%d, ALI_AVSYNC_GET_VIDEO_PTS, ali_rpc_malloc_shared_mm() failed.\n", __FUNCTION__, __LINE__);
			return RET_FAILURE;
		}
		
		*gp_avsync_vpts =0;
		
		
		ret = avsync_ioctl(dev, AVSYNC_IO_SET_VPTS_SHM_ADDR, (UINT32)gp_avsync_vpts);
		if(RET_SUCCESS != ret)
		{
			printk("<0>""%s error line%d, AVSYNC_IO_GET_VIDEO_PTS, set share mem address of video pts to SEE failed\n", __FUNCTION__, __LINE__);
			return RET_FAILURE;
		}
	}	

	if((copy_ret = copy_to_user((void *)io_addr, (void *)gp_avsync_vpts, sizeof(INT64))) !=  0)
	{
		printk("%s error line%d\n", __FUNCTION__, __LINE__);
		// Invalid user space address
		return RET_FAILURE;
	}

	return RET_SUCCESS;	
}
RET_CODE avsync_main_enable_get_stc(struct avsync_device *dev, UINT32 enable)
{
	avsync_get_stc_en_t get_stc_en;
	
	if(enable && !g_avsync_main_get_stc_en)
	{
		AVSYNC_PRF("notify see, get stc enabled\n");

		//allocate share memory
		gp_avsync_sharem_info = ali_rpc_malloc_shared_mm(sizeof(struct avsync_sharem_info));
		get_stc_en.enable = enable;
		get_stc_en.sharem_addr = (void *)gp_avsync_sharem_info;
		avsync_ioctl(dev, AVSYNC_IO_ENABLE_GET_STC, (UINT32)&get_stc_en);
		
		g_avsync_main_get_stc_en = 1;		
	}
	else if(!enable && g_avsync_main_get_stc_en)
	{
		get_stc_en.enable = enable;
		avsync_ioctl(dev, AVSYNC_IO_ENABLE_GET_STC, (UINT32)&get_stc_en);

		//free share memory
		ali_rpc_free_shared_mm((void *)gp_avsync_sharem_info, sizeof(struct avsync_sharem_info));
		gp_avsync_sharem_info = NULL;
		
		g_avsync_main_get_stc_en = 0;		
	}		
	return RET_SUCCESS;
	
}

RET_CODE avsync_main_get_stc(UINT8 stc_id, UINT8  stc_offset_idx, UINT32 *pstc)
{
	struct sharem_stc_info *pstc_info = NULL;
	int sleep_cnt=0;
	RET_CODE copy_ret = 0;	
		
	if(NULL == gp_avsync_sharem_info)
	{
		AVSYNC_ERROR_PRF("get stc is disabled\n ");
		return RET_FAILURE;
	}

	pstc_info = (struct sharem_stc_info *)(&gp_avsync_sharem_info->stc_info);
	
	/*use share memory to get stc to avoid remote call issue
	    main cpu write stc_id and stc_offset_idx to share memory, 
	    see cpu write stc value to share memory and set stc_updated flag.
	*/
	
	if(stc_id > 1)
	{
		AVSYNC_ERROR_PRF("stc id %d error, should be 0 or 1\n ", stc_id);
		return RET_FAILURE;
	}

	if(stc_offset_idx > 8)
	{
		AVSYNC_ERROR_PRF("stc offset id %d error, should be <= 8\n ", stc_offset_idx);
		return RET_FAILURE;
	}

	if(!pstc_info->stc_updated)
	{
		return RET_FAILURE;
	}

	// write stc info to share memory
	pstc_info->stc_id = stc_id;
	pstc_info->stc_offset_idx = stc_offset_idx;
	pstc_info->stc_updated = 0;

	while(!pstc_info->stc_updated)
	{ // wait see update stc value
		msleep(1); // sleep 1ms
		sleep_cnt++;
		if(sleep_cnt > 20)
		{
			AVSYNC_ERROR_PRF("get stc timeout, exceed 20ms\n ");
			return RET_FAILURE;
		}
	}

	if(!pstc_info->stc_valid)
	{
		pstc_info->stc = 0;
		AVSYNC_ERROR_PRF("stc is invalid\n");
		return RET_FAILURE;
	}

	if((copy_ret = copy_to_user((void *)pstc, (void *)&pstc_info->stc, 4)) != 0)
	{
		printk("%s error line%d\n", __FUNCTION__, __LINE__);
		// Invalid user space address
		return -EFAULT;
	}		

	return RET_SUCCESS;;
}

RET_CODE avsync_main_get_ctrl_blk(UINT32 pts, struct control_block *pctrl_blk_user)
{
	struct sharem_ctrl_blk_info *pctrl_blk_info = NULL;
	struct control_block ctrl_blk;
	int delay_cnt=0;
	RET_CODE copy_ret = 0;

	if(NULL == gp_avsync_sharem_info)
	{
		AVSYNC_ERROR_PRF("get stc is disabled\n ");
		return RET_FAILURE;
	}

	pctrl_blk_info = (struct sharem_ctrl_blk_info *)(&gp_avsync_sharem_info->ctrl_blk_info);
		
	while(pctrl_blk_info->updating)
	{ // SEE cpu is updating control block
		udelay(200);
		delay_cnt++;
		if(delay_cnt > 5) // exceed 1ms
			return RET_FAILURE;
	}

	//read control block info from share memory
	memset((void *)&ctrl_blk, 0, sizeof(struct control_block));
	ctrl_blk.pts = pts + ((pctrl_blk_info->run_back_flag<<31)&0x80000000);
	ctrl_blk.pts_valid = 1;
	ctrl_blk.stc_id = pctrl_blk_info->stc_id;
	ctrl_blk.stc_id_valid = 1;
	ctrl_blk.stc_offset_idx = pctrl_blk_info->stc_offset_idx;
	
	if((copy_ret = copy_to_user((void *)pctrl_blk_user, (void *)&ctrl_blk, sizeof(struct control_block))) !=0)
	{
		printk("%s error line%d\n", __FUNCTION__, __LINE__);
		// Invalid user space address
		return -EFAULT;
	}		
	return RET_SUCCESS;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_avsync_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int ali_avsync_ioctl(struct inode *node,struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	struct avsync_device *dev = file->private_data;
	uint32 get_param = 0xff;
	RET_CODE ret=RET_FAILURE;
	RET_CODE copy_ret = 0;
		
	if(NULL == dev)
	{
		AVSYNC_PRF("see AVYSNC device attach failed.\n");
		return -ENODEV;
	}

	down(&m_avsync_sem);

	AVSYNC_PRF("io command %d\n", cmd);
	
	switch(cmd)
	{
		case ALI_AVSYNC_RESET:
		{
			ret = avsync_reset(dev);
			break;
		}
		
		case ALI_AVSYNC_START:
		{
			ret = avsync_start(dev);
			break;
		}
		
		case ALI_AVSYNC_STOP:
		{
			ret = avsync_stop(dev);
			break;
		}
		
		case ALI_AVSYNC_SET_SYNC_MODE:
		{
			AVSYNC_PRF("io ALI_AVSYNC_SET_SYNC_MODE %d\n", arg);
			ret = avsync_set_syncmode(dev, (unsigned long)arg);
			break;
		}
		
		case ALI_AVSYNC_GET_SYNC_MODE:
		{
			ret = avsync_get_syncmode(dev, (AVSYNC_MODE_E *)&get_param);
			AVSYNC_PRF("io ALI_AVSYNC_GET_SYNC_MODE %d\n", get_param);
			if((copy_ret = copy_to_user((void *)arg, (void *)&get_param, 4)) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}				
			break;
		}
		
		case ALI_AVSYNC_SET_SOURCE_TYPE:
		{
			ret = avsync_set_sourcetype(dev, (unsigned long)arg);
			break;
		}
		
		case ALI_AVSYNC_GET_SOURCE_TYPE:
		{
			ret = avsync_get_sourcetype(dev, (AVSYNC_SRCTYPE_E*)&get_param);
			if((copy_ret = copy_to_user((void *)arg, (void *)&get_param, 4)) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
			break;
		}
		
		case ALI_AVSYNC_CONFIG_PARAMS:
		{
			avsync_cfg_param_t param;
			if((copy_ret = copy_from_user((void *)&param, (void *)arg, sizeof(avsync_cfg_param_t))) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}

			ret = avsync_config_params(dev, &param);
			break;
		}
		
		case ALI_AVSYNC_GET_PARAMS:
		{
			avsync_cfg_param_t param;
			ret = avsync_get_params(dev, &param);
			if((copy_ret = copy_to_user((void *)arg, (void *)&param, sizeof(avsync_cfg_param_t))) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}			
			break;
		}
		
		case ALI_AVSYNC_CONFIG_ADVANCE_PARAMS:
		{
			avsync_adv_param_t adv_param;
			if((copy_ret = copy_from_user((void *)&adv_param, (void *)arg, sizeof(avsync_adv_param_t))) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
			ret = avsync_config_advance_params(dev, &adv_param);
			break;
		}
		
		case ALI_AVSYNC_GET_ADVANCE_PARAMS:
		{
			avsync_adv_param_t adv_param;
			ret = avsync_get_advance_params(dev, &adv_param);
			if((copy_ret = copy_to_user((void *)arg, (void *)&adv_param, sizeof(avsync_adv_param_t))) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}

			
			break;
		}
		
		case ALI_AVSYNC_GET_STATUS:
		{
			avsync_status_t status;
			ret = avsync_get_status(dev, &status);
			if((copy_ret = copy_to_user((void *)arg, (void *)&status, sizeof(avsync_status_t))) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}
			break;
		}
		
		case ALI_AVSYNC_GET_STATISTICS:
		{
			avsync_statistics_t statis;
			ret = avsync_get_statistics(dev, &statis);
			if((copy_ret = copy_to_user((void *)arg, (void *)&statis, sizeof(avsync_statistics_t))) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}				
			break;
		}
		
		case ALI_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF:
		{
			struct ali_avsync_rpc_pars pars;
			UINT32 tmp[3] = {0,};
			UINT32 i = 0;

			if((copy_ret = copy_from_user((void *)&pars, (void *)arg, sizeof(pars))) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}				
			for(i=0; i<pars.arg_num; i++)
			{
				if((copy_ret = copy_from_user((void *)&tmp[i], (void *)pars.arg[i].arg, pars.arg[i].arg_size)) !=  0)
				{
					printk("%s error line%d\n", __FUNCTION__, __LINE__);
					// Invalid user space address
					return -EFAULT;
				}					
			}

			AVSYNC_PRF("io ALI_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF %d-%d-%d\n", (UINT8)tmp[0], tmp[1], (UINT8)tmp[2]);

			ret = avsync_video_smoothly_play_onoff(dev, tmp[0], tmp[1], tmp[2]);
			break;
		}

		case ALI_AVSYNC_GET_STC:
		{
			struct ali_avsync_rpc_pars pars;
			UINT8 stc_id, stc_offset_idx;
			UINT32 *pstc_user;

			if((copy_ret = copy_from_user((void *)&pars, (void *)arg, sizeof(pars))) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}		
			
			if((copy_ret = copy_from_user((void *)&stc_id, (void *)pars.arg[0].arg, pars.arg[0].arg_size)) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}			
			
			if((copy_ret = copy_from_user((void *)&stc_offset_idx, (void *)pars.arg[1].arg, pars.arg[1].arg_size)) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}				
			pstc_user = (void *)pars.arg[2].arg;
			AVSYNC_PRF("io ALI_AVSYNC_GET_STC, stc id %d, stc_offset_idx %d\n", stc_id, stc_offset_idx);
			ret = avsync_main_get_stc(stc_id, stc_offset_idx, pstc_user);
			break;
		}
				
		case ALI_AVSYNC_GET_CONTROL_BLOCK_OTHERS:
		{
			struct ali_avsync_rpc_pars pars;
			UINT32 pts;
			struct control_block *pctrl_blk_user;

			if((copy_ret = copy_from_user((void *)&pars, (void *)arg, sizeof(pars))) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}				
			if((copy_ret = copy_from_user((void *)&pts, (void *)pars.arg[0].arg, pars.arg[0].arg_size)) !=  0)
			{
				printk("%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}				
				
			pctrl_blk_user = pars.arg[1].arg;
				
			AVSYNC_PRF("io ALI_AVSYNC_GET_CONTROL_BLOCK_OTHERS, pts 0x%x\n", pts);
			ret = avsync_main_get_ctrl_blk(pts, pctrl_blk_user);
			break;
		}		

		case ALI_AVSYNC_IO_COMMAND:
		{
			struct ali_avsync_ioctl_command io_param;

			if((copy_ret = copy_from_user(&io_param, (void *)arg, sizeof(struct ali_avsync_ioctl_command))) !=  0)
			{
				printk("<0>""%s error line%d\n", __FUNCTION__, __LINE__);
				// Invalid user space address
				return -EFAULT;
			}				
			//printk("<0>""%s, io %d\n", __FUNCTION__, io_param.ioctl_cmd);
			if(AVSYNC_IO_GET_VIDEO_PTS == io_param.ioctl_cmd)
			{
				ret = avsync_get_video_pts(dev, io_param.param);
			}
            else if(AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG == io_param.ioctl_cmd)
			{
				struct avsync_smoothly_play_cfg_t cfg;
				ret = avsync_ioctl(dev, AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG, (UINT32)&cfg);	
				AVSYNC_PRF("%s: %d-%d-%d\n", __FUNCTION__, cfg.onoff, cfg.level, cfg.interval);
				if(RET_SUCCESS == ret)
					{
						if((copy_ret = copy_to_user((void *)io_param.param, (void *)&cfg, sizeof(struct avsync_smoothly_play_cfg_t))) !=  0)
						{
							printk("%s error line%d\n", __FUNCTION__, __LINE__);
							// Invalid user space address
							return -EFAULT;
						}							
					}
				else	
					AVSYNC_PRF("%s: get smooth fail\n", __FUNCTION__);
			}
			else if(AVSYNC_IO_GET_CURRENT_PLAY_PTS == io_param.ioctl_cmd
				|| AVSYNC_IO_GET_CURRENT_STC == io_param.ioctl_cmd)
			{
				unsigned int pts;
				ret = avsync_ioctl(dev, io_param.ioctl_cmd, (UINT32)&pts);

				if(RET_SUCCESS == ret)
				{
					if((copy_ret = copy_to_user((void *)io_param.param, (void *)&pts, sizeof(unsigned int))) !=  0)
					{
						printk("%s error line%d\n", __FUNCTION__, __LINE__);
						// Invalid user space address
						return -EFAULT;
					}						
					AVSYNC_PRF("kernel: current play pts %d\n", pts);
				}
			}
			else
				ret = avsync_ioctl(dev, io_param.ioctl_cmd, io_param.param);
			
			break;
		}
#if 0		
		case ALI_AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG:
		{
			avsync_smoothly_play_cfg_t cfg;
			ret = avsync_ioctl(dev, AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG, (UINT32)&cfg);
			AVSYNC_PRF("smooth %d-%d-%d\n", __FUNCTION__, cfg.onoff, cfg.level, cfg.interval);

			copy_to_user((void *)arg, (void *)&cfg, sizeof(avsync_smoothly_play_cfg_t));
			break;
		}
		
		case ALI_AVSYNC_IO_ENABLE_DDP_CERTIFICATION:
		{
			ret = avsync_ioctl(dev, AVSYNC_IO_ENABLE_DDP_CERTIFICATION, arg);
			break;
		}
		
		case ALI_AVSYNC_IO_UNREG_CALLBACK:
		{
			ret = avsync_ioctl(dev, AVSYNC_IO_UNREG_CALLBACK, arg);
			break;
		}
		
		case ALI_AVSYNC_IO_REG_CALLBACK:
		{
			AVSYNC_PRF("io ALI_AVSYNC_IO_ENABLE_GET_STC\n");
			ret = avsync_ioctl(dev, AVSYNC_IO_REG_CALLBACK, arg);
			break;
		}
#endif

		case ALI_AVSYNC_SET_DBG_PRINT_OPTION:
		{
			ret = avsync_dbg_set_print_option(dev, (unsigned long)arg);
			break;
		}
		
		case ALI_AVSYNC_SET_DBG_POLL_ONOFF:
		{
			ret = avsync_dbg_polling_onoff(dev, (unsigned long)arg);
			break;
		}
		
		case ALI_AVSYNC_SET_DBG_POLL_OPTION:
		{
			ret = avsync_dbg_set_polling_option(dev, (unsigned long)arg);
			break;
		}
		
		case ALI_AVSYNC_SET_DBG_POLL_INTERVAL:
		{
			ret = avsync_dbg_set_polling_interval(dev, (unsigned long)arg);
			break;
		}
				
		default:
			up(&m_avsync_sem);
			
			return -1;
	}

	up(&m_avsync_sem);
	
	if(RET_FAILURE == ret)
		return -1;
	
	return 0;
}

/*how to transfer device handle*/
static int ali_avsync_open(struct inode *inode, struct file *file)
{
	struct avsync_device *see_avysnc_dev = m_avsync_see_dev;
	
	if(NULL==see_avysnc_dev)
	{
		AVSYNC_PRF("see AVYSNC device attach failed.\n");
		return -ENODEV;
	}

	down(&m_avsync_sem);

	//printk("<0>""%s : open count %d\n", __FUNCTION__, m_avsync_open_cnt);
	
	if(m_avsync_open_cnt == 0)
		avsync_open(see_avysnc_dev);


	m_avsync_open_cnt++;
	
	file->private_data=(void*)see_avysnc_dev;

	up(&m_avsync_sem);

	return 0;
}

extern void ali_decv_rpc_release(void);
extern void ali_deca_rpc_release(void);

static int ali_avsync_release(struct inode *inode, struct file *file)
{
	struct avsync_device *dev = file->private_data;

	if(NULL==dev)
	{
		AVSYNC_PRF("see AVYSNC device attach failed.\n");
		return -ENODEV;
	}

	down(&m_avsync_sem);
	
	//printk("<0>""%s : open count %d\n", __FUNCTION__, m_avsync_open_cnt);

	m_avsync_open_cnt--;
	
	if(m_avsync_open_cnt == 0)
	{
		ali_decv_rpc_release();
	 	ali_deca_rpc_release();

		avsync_stop(dev);
		avsync_close(dev);

		if(g_avsync_main_get_stc_en)
		{
			ali_rpc_free_shared_mm((void *)gp_avsync_sharem_info, sizeof(struct avsync_sharem_info));
			gp_avsync_sharem_info = NULL;
			
			g_avsync_main_get_stc_en = 0;		
		}

		if(gp_avsync_vpts)
		{
			ali_rpc_free_shared_mm((void *)gp_avsync_vpts, sizeof(INT64));
			gp_avsync_vpts = NULL;			
		}
	}

	up(&m_avsync_sem);
	
	return 0;
}

static const struct file_operations ali_avsync_fops = {
	.owner =	THIS_MODULE,
	.read = ali_avsync_read,
	.write = ali_avsync_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_avsync_ioctl,
#else
	.ioctl = ali_avsync_ioctl,
#endif	
	.open = ali_avsync_open,
	.release = ali_avsync_release,
};

extern void avsync_debug_procfs_init();
extern void avsync_debug_procfs_exit();


static int __init ali_avsync_init(void)
{
	if(alloc_chrdev_region(&m_avsync_dev_t, 0, 1, "ali_avsync") < 0)
	{
		AVSYNC_PRF("allocate ali video dev_t fail\n");
		return 0;
	}

	m_avsync_cdev = cdev_alloc();
	if(m_avsync_cdev == NULL)	
	{
		AVSYNC_PRF("ali avysnc cdev alloc fail\n");
		return 0;
	}
	
	m_avsync_cdev->ops = &ali_avsync_fops;
	m_avsync_cdev->owner = ali_avsync_fops.owner;
	kobject_set_name(&m_avsync_cdev->kobj, "%s", "ali_avsync");
	if(cdev_add(m_avsync_cdev, m_avsync_dev_t, 1) < 0)
	{
		AVSYNC_PRF("ali avysnc cdev add fail\n");
		goto FAIL;
	}

	m_avysnc_class = class_create(THIS_MODULE, "ali_avsync");
	if(m_avysnc_class == NULL)	
	{
		AVSYNC_PRF("ali avysnc create class fail\n");
		goto FAIL;
	}
	
	m_avysnc_device  = device_create(m_avysnc_class, NULL, m_avsync_dev_t, NULL
		, "ali_avsync0");
	if(m_avysnc_device == NULL)
	{
		AVSYNC_PRF("ali avysnc create device fail\n");
		goto FAIL;
	}

	m_avsync_see_dev = (struct avsync_device*)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_AVSYNC);

        sema_init(&m_avsync_sem, 1);
	
	m_avsync_open_cnt = 0;

	avsync_debug_procfs_init();
	
	AVSYNC_PRF("init ali avysnc done major %d\n", MAJOR(m_avsync_dev_t));
	return 0;
	
FAIL:
	if(m_avsync_cdev != NULL)
		cdev_del(m_avsync_cdev);
	if(m_avsync_dev_t != 0)
		unregister_chrdev_region(m_avsync_dev_t, 1);
	if(m_avysnc_device != NULL)
		device_del(m_avysnc_device);
	if(m_avysnc_class != NULL)
		class_destroy(m_avysnc_class);

	AVSYNC_PRF("init ali avysnc fail\n");	
	return 0;
}

 static void __exit ali_avsync_exit(void)
{
	avsync_debug_procfs_exit();
	
	if(m_avysnc_device != NULL)
		device_del(m_avysnc_device);

	if(m_avysnc_class != NULL)
		class_destroy(m_avysnc_class);

	if(m_avsync_cdev != NULL)
		cdev_del(m_avsync_cdev);

	if(m_avsync_dev_t != 0)
		unregister_chrdev_region(m_avsync_dev_t, 1);
}

 module_init(ali_avsync_init);
 module_exit(ali_avsync_exit);
 
MODULE_AUTHOR("ALi (Shanghai) Corporation");
MODULE_DESCRIPTION("ali AV sync driver");
MODULE_LICENSE("GPL");

