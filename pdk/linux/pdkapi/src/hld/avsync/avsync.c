#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld_cfg.h>
#include <adr_mediatypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <hld/avsync/adr_avsync.h>
#include <ali_avsync_common.h>

#ifdef ADR_IPC_ENABLE
#include <hld/misc/adr_ipc.h>
#endif

#define AVSYNC_DBG_ENABLE	0
#define AVSYNC_PRF(fmt, args...)	\
	do { \
		if (AVSYNC_DBG_ENABLE) { \
			ADR_DBG_PRINT(AVSYNC, fmt, ##args); \
		} \
	} while(0)

static int g_avsync_file_handle = 0;
int g_avsync_attached;
int g_avsync_opened;
#ifdef ADR_IPC_ENABLE
int g_avsync_mutex_id; 

#define MUTEX_LOCK()   adr_ipc_semlock(g_avsync_mutex_id)
#define MUTEX_UNLOCK() adr_ipc_semunlock(g_avsync_mutex_id)
#endif

extern RET_CODE avsync_dbg_init();
RET_CODE avsync_open(struct avsync_device *dev)
{
	int avsync_hdl=0;
	
	if(NULL==dev)
		return !RET_SUCCESS;
	
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

#ifdef ADR_IPC_ENABLE
	if(g_avsync_attached == 0)
		avsync_attach();
#endif
	
	if(g_avsync_opened == 1) 
	{
		AVSYNC_PRF("avsync device already opened!!\n");
		return RET_SUCCESS;
	}
	
#ifdef ADR_IPC_ENABLE	
	MUTEX_LOCK();
#endif
	
	g_avsync_file_handle= open("/dev/ali_avsync0", O_RDWR | O_CLOEXEC);
	if(g_avsync_file_handle < 0)
	{
		AVSYNC_PRF("open avsync handle fail\n");
#ifdef ADR_IPC_ENABLE	
		MUTEX_UNLOCK();
#endif
		return !RET_SUCCESS;
	}	
	
	g_avsync_opened = 1;	
	AVSYNC_PRF("%s success!\n", __FUNCTION__);
#ifdef ADR_IPC_ENABLE	
	MUTEX_UNLOCK();
#endif
	return RET_SUCCESS;		
}

RET_CODE avsync_close(struct avsync_device *dev)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

#ifdef ADR_IPC_ENABLE
	if(g_avsync_attached == 0)
		return RET_SUCCESS;
#endif

	if(g_avsync_opened == 0) 
	{
		AVSYNC_PRF("avsync device already closed!!\n");
		return RET_SUCCESS;
	}

#ifdef ADR_IPC_ENABLE	
	MUTEX_LOCK();
#endif
	
	close((int)g_avsync_file_handle);
	
	g_avsync_opened = 0;
#ifdef ADR_IPC_ENABLE
#else
  dev->priv = NULL;   
#endif

#ifdef ADR_IPC_ENABLE
	MUTEX_UNLOCK();
#endif

	AVSYNC_PRF("%s success!\n", __FUNCTION__);
	return RET_SUCCESS;
}

RET_CODE avsync_start(struct avsync_device *dev)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

#ifdef ADR_IPC_ENABLE
    MUTEX_LOCK();
#endif
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_START))
	{
#ifdef ADR_IPC_ENABLE
	    MUTEX_UNLOCK();
#endif
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif	
	return !RET_SUCCESS;
}

RET_CODE avsync_stop(struct avsync_device *dev)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
#ifdef ADR_IPC_ENABLE	
	MUTEX_LOCK();
#endif
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_STOP))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif	
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif	
	return !RET_SUCCESS;
}

RET_CODE avsync_ioctl(struct avsync_device *dev, UINT32 io_code, UINT32 param)
{
	RET_CODE ret=RET_FAILURE;
	struct ali_avsync_ioctl_command io_param;

	if(NULL==dev)
		return !RET_SUCCESS;
	
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
#ifdef ADR_IPC_ENABLE	
	MUTEX_LOCK();
#endif

	io_param.ioctl_cmd = io_code;
	io_param.param = param;
	
	if(0 == ioctl((int)g_avsync_file_handle, ALI_AVSYNC_IO_COMMAND, (UINT32)&io_param))
		ret = RET_SUCCESS;
	else
	{
		ret = RET_FAILURE;
		AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	}
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return ret;
}
RET_CODE avsync_reset(struct avsync_device *dev)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
#ifdef ADR_IPC_ENABLE	
	MUTEX_LOCK();
#endif
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_RESET))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}
RET_CODE avsync_set_syncmode(struct avsync_device *dev, AVSYNC_MODE_E mode)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
 #ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
   
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_SET_SYNC_MODE, mode))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}

RET_CODE avsync_get_syncmode(struct avsync_device *dev, AVSYNC_MODE_E *pmode)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_GET_SYNC_MODE, pmode))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}
RET_CODE avsync_config_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_CONFIG_PARAMS, pcfg_params))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}

RET_CODE avsync_get_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_GET_PARAMS, pcfg_params))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}

RET_CODE avsync_config_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_CONFIG_ADVANCE_PARAMS, pcfg_params))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}

RET_CODE avsync_get_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_GET_ADVANCE_PARAMS, pcfg_params))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}
RET_CODE avsync_set_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E type)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
 #ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
    
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_SET_SOURCE_TYPE, type))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}

RET_CODE avsync_get_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E *ptype)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
 #ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
    
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_GET_SOURCE_TYPE, ptype))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}

RET_CODE avsync_get_status(struct avsync_device *dev, avsync_status_t *pstatus)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
 #ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
    
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_GET_STATUS, pstatus))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}

RET_CODE avsync_get_statistics(struct avsync_device *dev, avsync_statistics_t *pstatistics)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_GET_STATISTICS, pstatistics))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
}

RET_CODE avsync_video_smoothly_play_onoff(struct avsync_device *dev, UINT8 onoff, AVSYNC_VIDEO_SMOOTH_LEVEL level, UINT8 interval)
{
	struct ali_avsync_rpc_pars rpc_pars;		
	
	if(NULL==dev)
		return !RET_SUCCESS;
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
	
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.arg_num = 3;
	rpc_pars.arg[0].arg = (void *)&onoff;
	rpc_pars.arg[0].arg_size = sizeof(onoff);			
	rpc_pars.arg[1].arg = (void *)&level;
	rpc_pars.arg[1].arg_size = sizeof(level);
	rpc_pars.arg[2].arg = (void *)&interval;
	rpc_pars.arg[2].arg_size = sizeof(interval);
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF, &rpc_pars))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
	
}

RET_CODE avsync_get_stc(struct avsync_device *dev,UINT8 stc_id, UINT8  stc_offset_id, UINT32 *pstc)
{
	struct ali_avsync_rpc_pars rpc_pars;		

	if(NULL==dev)
		return !RET_SUCCESS;
	
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.arg_num = 3;
	rpc_pars.arg[0].arg = (void *)&stc_id;
	rpc_pars.arg[0].arg_size = sizeof(stc_id);			
	rpc_pars.arg[1].arg = (void *)&stc_offset_id;
	rpc_pars.arg[1].arg_size = sizeof(stc_offset_id);
	rpc_pars.arg[2].arg = (void *)pstc;
	rpc_pars.arg[2].arg_size = sizeof(pstc);
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_GET_STC, &rpc_pars))
	{
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
    MUTEX_UNLOCK();
#endif		
	return !RET_SUCCESS;
	
	
}

RET_CODE avsync_get_control_block_others(struct avsync_device *dev, UINT32 pts, struct control_block *pctrl_blk)
{
	
	struct ali_avsync_rpc_pars rpc_pars;		

	if(NULL==dev)
		return !RET_SUCCESS;
	
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)&pts;
	rpc_pars.arg[0].arg_size = sizeof(pts);			
	rpc_pars.arg[1].arg = (void *)pctrl_blk;
	rpc_pars.arg[1].arg_size = sizeof(pctrl_blk);
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_GET_CONTROL_BLOCK_OTHERS, &rpc_pars))
	{
#ifdef ADR_IPC_ENABLE
	MUTEX_UNLOCK();
#endif
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
#ifdef ADR_IPC_ENABLE
	MUTEX_UNLOCK();
#endif
	return !RET_SUCCESS;
}
/*
  * For AV SYNC debug
  */
RET_CODE avsync_dbg_set_print_option(struct avsync_device *dev, UINT32 option)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_SET_DBG_PRINT_OPTION, option))
	{
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
	return !RET_SUCCESS;
}

RET_CODE avsync_dbg_polling_onoff(struct avsync_device *dev, UINT32 on_off)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_SET_DBG_POLL_ONOFF, on_off))
	{
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
	return !RET_SUCCESS;	
}

RET_CODE avsync_dbg_set_polling_option(struct avsync_device *dev, UINT32 option)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_SET_DBG_POLL_OPTION, option))
	{
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
	return !RET_SUCCESS;		
}

RET_CODE avsync_dbg_set_polling_interval(struct avsync_device *dev, UINT32 interval_ms)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	
	AVSYNC_PRF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if(0==ioctl((int)g_avsync_file_handle, ALI_AVSYNC_SET_DBG_POLL_INTERVAL, interval_ms))
	{
		return RET_SUCCESS;
	}

	AVSYNC_PRF("%s failed!\n", __FUNCTION__);
	
	return !RET_SUCCESS;	
	
}
RET_CODE avsync_attach()
{
	struct avsync_device*dev ;

	AVSYNC_PRF("%s enter.\n", __FUNCTION__);
	if(g_avsync_attached)
	{
		AVSYNC_PRF("avsync is already attached\n");
		return RET_FAILURE; 
	}

#ifdef ADR_IPC_ENABLE
	g_avsync_mutex_id = adr_ipc_semget(ADR_IPC_AVSYNC, 0, 1);
	if(g_avsync_mutex_id < 0)
	{
		AVSYNC_PRF("avsync create mutex fail\n");
		return RET_FAILURE;
	}
#endif

	dev=dev_alloc("AVSYNC", HLD_DEV_TYPE_AVSYNC, sizeof(struct avsync_device));
	if(dev==NULL)
	{
		AVSYNC_PRF("Alloc avsync device error!\n", __FUNCTION__);
		return !RET_SUCCESS;
	}

#ifdef ADR_IPC_ENABLE
#else
	/* Init avsync_device element */
	dev->next=NULL ;
	dev->priv=NULL ;
#endif

	/* Add this device to queue */
	if(dev_register(dev)!=RET_SUCCESS)
	{
		dev_free(dev);
		AVSYNC_PRF("%s Register avsync device failed!!!!!!!\n", __FUNCTION__);
		return !RET_SUCCESS;
	}
	g_avsync_attached = 1;

	avsync_dbg_init();
#ifdef ADR_IPC_ENABLE
	avsync_open(dev);
#endif			

	AVSYNC_PRF("%s dev: %s Success!\n", __FUNCTION__, dev->name);
	return RET_SUCCESS;
}

void avsync_dettach(struct avsync_device *dev)
{
	struct avsync_device *avsync_dev;
	
	AVSYNC_PRF("%s enter.\n", __FUNCTION__);
	
	avsync_dev = (struct avsync_device *)dev_get_by_id(HLD_DEV_TYPE_AVSYNC,0);
	
	if(NULL == avsync_dev)
	{
		AVSYNC_PRF("avsync device not exist!\n", __FUNCTION__);
		return;
	}

	if(NULL == dev)
	{
		AVSYNC_PRF("detach dev is NULL!\n", __FUNCTION__);
		return;
	}

	dev_free(dev);
	
	AVSYNC_PRF("%s dev: %s Success!\n", __FUNCTION__, dev->name);
	return;
}
