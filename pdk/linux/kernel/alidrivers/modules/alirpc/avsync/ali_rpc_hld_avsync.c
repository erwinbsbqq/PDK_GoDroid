/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_hld_decv.c
 *  (I)
 *  Description: hld decv remote process call api
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.04.08			Sam			Create
 ****************************************************************************/
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
//#include <linux/smp_lock.h>
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
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld_avsync.h>
#include "../ali_rpc.h"

static UINT32 desc_avsync_p_uint32[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, 4),  
	1, DESC_P_PARA(0, 1, 0),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_avsync_io_p_uint32[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, 4),  
	1, DESC_P_PARA(0, 2, 0),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_avsync_params[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(avsync_cfg_param_t)),  
	1, DESC_P_PARA(0, 1, 0),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_avsync_advparams[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(avsync_adv_param_t)),  
	1, DESC_P_PARA(0, 1, 0),
	//desc of pointer ret
	0,                          
	0,
};


static UINT32 desc_avsync_get_status[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(avsync_status_t)),  
	1, DESC_P_PARA(0, 1, 0),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_avsync_get_statistics[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(avsync_statistics_t)),  
	1, DESC_P_PARA(0, 1, 0),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_avsync_get_smoothly_play_cfg[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(avsync_smoothly_play_cfg_t)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_avsync_get_stc_en[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(avsync_get_stc_en_t)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

RET_CODE avsync_open(struct avsync_device *dev)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_OPEN, NULL/*desc_avsync_open*/);
	
}

RET_CODE avsync_close(struct avsync_device *dev)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_CLOSE, NULL);
	
}

RET_CODE avsync_start(struct avsync_device *dev)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_START, NULL);
	
}

RET_CODE avsync_stop(struct avsync_device *dev)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_STOP, NULL);
	
}

RET_CODE avsync_ioctl(struct avsync_device *dev, UINT32 io_code, UINT32 param)
{
	UINT32 *desc = NULL;
	
		
	switch(io_code)
	{
		case AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG:
			desc = desc_avsync_get_smoothly_play_cfg;		
			break;

		case AVSYNC_IO_ENABLE_GET_STC:
			desc = desc_avsync_get_stc_en;		
			break;

		case AVSYNC_IO_GET_CURRENT_PLAY_PTS:
			desc = desc_avsync_io_p_uint32;		
			break;
			
		case AVSYNC_IO_ENABLE_DDP_CERTIFICATION:
		case AVSYNC_IO_SET_PTS_UNIT_HZ:
		case AVSYNC_IO_REG_CALLBACK:
		case AVSYNC_IO_UNREG_CALLBACK:
		case AVSYNC_IO_STC_VALID:
		case AVSYNC_IO_SET_VPTS_SHM_ADDR:
			break;
			
		default:
			return RET_FAILURE;
	}
	
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(3<<16)|FUNC_AVSYNC_IOCTL, desc);	
	
}

RET_CODE avsync_reset(struct avsync_device *dev)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_RESET, NULL);	
	
}

RET_CODE avsync_set_syncmode(struct avsync_device *dev, AVSYNC_MODE_E mode)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_SET_SYNCMODE, NULL);
	
}

RET_CODE avsync_get_syncmode(struct avsync_device *dev, AVSYNC_MODE_E *pmode)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_SYNCMODE, desc_avsync_p_uint32);
	
}

RET_CODE avsync_config_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_CFG_PARAMS, desc_avsync_params);
	
}

RET_CODE avsync_get_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_PARAMS, desc_avsync_params);
	
}

RET_CODE avsync_config_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_CFG_ADV_PARAMS, desc_avsync_advparams);	
	
}

RET_CODE avsync_get_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_ADV_PARAMS, desc_avsync_advparams);	
	
}

RET_CODE avsync_set_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E type)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_SET_SRCTYPE, NULL);
	
}
RET_CODE avsync_get_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E *ptype)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_SRCTYPE, desc_avsync_p_uint32);
	
}

RET_CODE avsync_get_status(struct avsync_device *dev, avsync_status_t *pstatus)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_STATUS, desc_avsync_get_status);
	
}

RET_CODE avsync_get_statistics(struct avsync_device *dev, avsync_statistics_t *pstatistics)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_STATISTICS, desc_avsync_get_statistics);
	
}

RET_CODE avsync_video_smoothly_play_onoff(struct avsync_device *dev, UINT8 onoff, AVSYNC_VIDEO_SMOOTH_LEVEL level, UINT8 interval)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(4<<16)|FUNC_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF, NULL);
		
}

RET_CODE avsync_dbg_set_print_option(struct avsync_device *dev, UINT32 option)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_DBG_SET_PRINT_OPTION, NULL);
		
}

RET_CODE avsync_dbg_polling_onoff(struct avsync_device *dev, UINT32 on_off)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_DBG_SET_POLL_ONOFF, NULL);
		
}

RET_CODE avsync_dbg_set_polling_option(struct avsync_device *dev, UINT32 option)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_DBG_SET_POLL_OPTION, NULL);
		
}

RET_CODE avsync_dbg_set_polling_interval(struct avsync_device *dev, UINT32 interval_ms)
{
		
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_DBG_SET_POLL_INTERVAL, NULL);
		
}


#if 0
UINT32 desc_avsync_attach[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(avsync_cfg_param_t)),
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};
#endif

RET_CODE avsync_attach(void)
{
    jump_to_func(NULL, ali_rpc_call, null, (LLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_ATTACH, NULL/*desc_avsync_attach*/);
}


