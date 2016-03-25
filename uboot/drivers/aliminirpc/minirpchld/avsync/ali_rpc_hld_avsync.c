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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
#include <linux/smp_lock.h>
#endif
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
//#include "../ali_rpc.h"
#include <ali_rpcng.h>

RET_CODE avsync_open(struct avsync_device *dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
    ret = RpcCallCompletion(RPC_avsync_open,&p1,NULL);
    return ret;

}

RET_CODE avsync_close(struct avsync_device *dev)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
    ret = RpcCallCompletion(RPC_avsync_close,&p1,NULL);   
    return ret;
    
}

RET_CODE avsync_start(struct avsync_device *dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
   ret = RpcCallCompletion(RPC_avsync_start,&p1,NULL); 
   return ret;
    
}

RET_CODE avsync_stop(struct avsync_device *dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
    ret = RpcCallCompletion(RPC_avsync_stop,&p1,NULL);   
    return ret;   
}

RET_CODE avsync_ioctl(struct avsync_device *dev, UINT32 io_code, UINT32 param)
{
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &io_code);    
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);     
    
    Int32 ret;
	switch(io_code)
	{
		case AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG:
			//desc = desc_avsync_get_smoothly_play_cfg;
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Avsync_smoothly_play_cfg_t_rpc, sizeof(Avsync_smoothly_play_cfg_t_rpc), param);           
			break;

		case AVSYNC_IO_ENABLE_GET_STC:
			//desc = desc_avsync_get_stc_en;		
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Avsync_get_stc_en_t_rpc, sizeof(Avsync_get_stc_en_t_rpc), param);
			break;

		case AVSYNC_IO_GET_CURRENT_PLAY_PTS:
			//desc = desc_avsync_io_p_uint32;		
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, 4, param);
			break;
			
		case AVSYNC_IO_ENABLE_DDP_CERTIFICATION:
		case AVSYNC_IO_SET_PTS_UNIT_HZ:
		case AVSYNC_IO_REG_CALLBACK:
		case AVSYNC_IO_UNREG_CALLBACK:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(4), &param);        
			break;
			
		default:
			return RET_FAILURE;
	}    
    ret = RpcCallCompletion(RPC_avsync_ioctl,&p1,&p2,&p3,NULL);
    return ret;
    
    
}

RET_CODE avsync_reset(struct avsync_device *dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
    ret = RpcCallCompletion(RPC_avsync_reset,&p1,NULL);
    return ret;

}

RET_CODE avsync_set_syncmode(struct avsync_device *dev, AVSYNC_MODE_E mode)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(AVSYNC_MODE_E), &mode);
    
    ret = RpcCallCompletion(RPC_avsync_set_syncmode,&p1,&p2,NULL);   
    return ret;
    
}

RET_CODE avsync_get_syncmode(struct avsync_device *dev, AVSYNC_MODE_E *pmode)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_ENUM, sizeof(AVSYNC_MODE_E), pmode);
    
    ret = RpcCallCompletion(RPC_avsync_get_syncmode,&p1,&p2,NULL);
    return ret;
}
RET_CODE avsync_config_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Avsync_cfg_param_t_rpc, sizeof(Avsync_cfg_param_t_rpc), pcfg_params);

    ret = RpcCallCompletion(RPC_avysnc_config_params,&p1,&p2,NULL);
    return ret;
}

RET_CODE avsync_get_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Avsync_cfg_param_t_rpc, sizeof(Avsync_cfg_param_t_rpc), pcfg_params);

    ret = RpcCallCompletion(RPC_avysnc_get_params,&p1,&p2,NULL);
    return ret;
    
}

RET_CODE avsync_config_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Avsync_adv_param_t_rpc, sizeof(Avsync_adv_param_t_rpc), pcfg_params);

    ret = RpcCallCompletion(RPC_avysnc_config_advance_params,&p1,&p2,NULL);
    return ret;
    
}

RET_CODE avsync_get_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Avsync_adv_param_t_rpc, sizeof(Avsync_adv_param_t_rpc), pcfg_params);

    ret = RpcCallCompletion(RPC_avysnc_get_advance_params,&p1,&p2,NULL);
    return ret;
    
}

RET_CODE avsync_set_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E type)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(AVSYNC_SRCTYPE_E), &type);

    ret = RpcCallCompletion(RPC_avsync_set_sourcetype,&p1,&p2,NULL);
    return ret;
    
}
RET_CODE avsync_get_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E *ptype)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_ENUM, sizeof(AVSYNC_SRCTYPE_E), ptype);

    ret = RpcCallCompletion(RPC_avsync_get_sourcetype,&p1,&p2,NULL);
    return ret;
    
}

RET_CODE avsync_get_status(struct avsync_device *dev, avsync_status_t *pstatus)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Avsync_status_t_rpc, sizeof(Avsync_status_t_rpc), pstatus);

    ret = RpcCallCompletion(RPC_avsync_get_status,&p1,&p2,NULL);
    return ret;
    
}

RET_CODE avsync_get_statistics(struct avsync_device *dev, avsync_statistics_t *pstatistics)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Avsync_statistics_t_rpc, sizeof(Avsync_statistics_t_rpc), pstatistics);

    ret = RpcCallCompletion(RPC_avsync_get_statistics,&p1,&p2,NULL);
    return ret;

}

RET_CODE avsync_video_smoothly_play_onoff(struct avsync_device *dev, UINT8 onoff, AVSYNC_VIDEO_SMOOTH_LEVEL level, UINT8 interval)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &onoff);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_ENUM, sizeof(AVSYNC_VIDEO_SMOOTH_LEVEL), &level);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &interval);

    ret = RpcCallCompletion(RPC_avsync_video_smoothly_play_onoff,&p1,&p2,&p3,&p4,NULL);
    return ret;
    
}
RET_CODE avsync_dbg_set_print_option(struct avsync_device *dev, UINT32 option)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &option);
    
    ret = RpcCallCompletion(RPC_avsync_dbg_set_print_option,&p1,&p2,NULL);
    return ret;    
}

RET_CODE avsync_dbg_polling_onoff(struct avsync_device *dev, UINT32 on_off)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &on_off);
    
    ret = RpcCallCompletion(RPC_avsync_dbg_polling_onoff,&p1,&p2,NULL);
    return ret;    
}

RET_CODE avsync_dbg_set_polling_option(struct avsync_device *dev, UINT32 option)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &option);
    
    ret = RpcCallCompletion(RPC_avsync_dbg_set_polling_option,&p1,&p2,NULL);
    return ret;    
    
}

RET_CODE avsync_dbg_set_polling_interval(struct avsync_device *dev, UINT32 interval_ms)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &interval_ms);
    
    ret = RpcCallCompletion(RPC_avsync_dbg_set_polling_interval,&p1,&p2,NULL);
    return ret;    
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
    RpcCallCompletion(RPC_avsync_attach,NULL);
}


