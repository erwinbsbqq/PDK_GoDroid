#include <adr_basic_types.h>
#include <osal/osal.h>

#include <hld/adr_hld_dev.h>
#include <hld/sdec/adr_sdec.h>
#include <hld/sdec/adr_sdec_dev.h>
//#include <api/libsubt/lib_subt.h>
//#include <api/libsubt/subt_osd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#define SDEC_PRINTF(...) do{}while(0)
static int g_sdec_handle = 0;
static char sdec_m36_name[HLD_MAX_NAME_SIZE] = "ali_sdec";
/*
 * 	Name		:   sdec_open()   	
 *	Description	:   Open a sdec device
 *	Parameter	:	struct sdec_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
__ATTRIBUTE_REUSE_
INT32 sdec_open(struct sdec_device *dev)
{
	INT32 result=SUCCESS;
	int sdec_handle = 0;
	
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
	
	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		SDEC_PRINTF("sdec_open: warning - device openned already!\n");
		return SUCCESS;
	}
	
	//strcpy(dev->name, "/dev/ali_sdec");
	
	sdec_handle = open("/dev/ali_sdec", O_RDONLY|O_NONBLOCK);
	if(sdec_handle < 0)
	{
		SDEC_PRINTF("sdec_open: fail to open device %s!\n", dev->name);
		result =  !SUCCESS;
	}
	else
	{
		g_sdec_handle = sdec_handle;
		dev->priv = (void *)sdec_handle;
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	}
	
	return result;

}

/*
 * 	Name		:   sdec_close()   	
 *	Description	:   Close a sdec device
 *	Parameter	:	struct sdec_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 sdec_close(struct sdec_device *dev)
{
	int sdec_handle = 0;
	
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		SDEC_PRINTF("sdec_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}
	
	sdec_handle = (int)dev->priv;
	close(sdec_handle);
	dev->priv = (int)0;
	g_sdec_handle = 0;	
	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	
	return SUCCESS;
}

INT32  sdec_start(struct sdec_device *dev,UINT16 composition_page_id,UINT16 ancillary_page_id)
{
	INT32 result=SUCCESS;
	int sdec_handle = 0;
	UINT32 arg;
	
	SDEC_PRINTF("%s->%s()->%d->(%d)(%d).\n", __FILE__, __FUNCTION__, __LINE__,composition_page_id, ancillary_page_id);

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return !SUCCESS;
	}
	
	sdec_handle = (int)dev->priv;
	arg = (UINT32)((ancillary_page_id << 16) |composition_page_id);
	result = ioctl(sdec_handle, CALL_SDEC_START, (void *)arg);

	return result;
}

INT32  sdec_stop(struct sdec_device *dev)
{
	INT32 result=SUCCESS;
	int sdec_handle = 0;
	
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return !SUCCESS;
	}
	
	sdec_handle = (int)dev->priv;
	result = ioctl(sdec_handle, CALL_SDEC_STOP, (void *)NULL);

	return result;

}
INT32  sdec_pause(struct sdec_device *dev)
{
	INT32 result=SUCCESS;
	int sdec_handle = 0;
	
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
		
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return !SUCCESS;
	}
	
	sdec_handle = (int)dev->priv;
	result = ioctl(sdec_handle, CALL_SDEC_PAUSE, (void *)NULL);

	return result;
}

void  lib_subt_attach(struct subt_config_par *psubt_config_par)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
		
	if (g_sdec_handle == 0)
	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_LIB_SUBT_ATTACH, (void *)psubt_config_par);}



void  osd_subt_enter(void)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)
	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_OSD_SUBT_ENTER, NULL);
}
	
void  osd_subt_leave(void)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
		
	if (g_sdec_handle == 0)
	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_OSD_SUBT_LEAVE, NULL);
}

/* isdbt cc */
void lib_isdbtcc_init(void)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)
	{
		return;
}

	ioctl(g_sdec_handle, CALL_LIB_ISDBT_CC_INIT, NULL);
}

void osd_isdbtcc_enter(void)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)
{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_OSD_ISDBT_CC_ENTER, NULL);
}
	
void osd_isdbtcc_leave(void)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)
	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_OSD_ISDBT_CC_LEASE, NULL);
	}
	
void lib_isdbtcc_attach(struct isdbtcc_config_par *pcc_config_par)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)
	{
		return;
	}

	ioctl(g_sdec_handle, CALL_LIB_ISDBT_CC_ATTACH, (int)pcc_config_par);
}

INT32 isdbtcc_dec_attach(struct sdec_feature_config * cfg_param)
{
	INT32 result=SUCCESS;
	
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
	
	if (g_sdec_handle == 0)
	{
		return !SUCCESS;
	}
	
	result = ioctl(g_sdec_handle, CALL_ISDBT_CC_DEC_ATTACH, (int)cfg_param);

	return result;
}

void isdbtcc_disply_init(struct sdec_device *dev)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)
	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_ISDBT_CC_DISPLAY_INIT, (int)dev);
}

void isdbtcc_get_cur_lang_by_pid(UINT16 pid, UINT32 lang)
{
	struct isdbt_get_lang_by_pid para;

	para.pid = pid;
	para.para = (unsigned int)lang;

	ioctl(g_sdec_handle, CALL_ISDBT_CC_GET_CUR_LANG_BY_PID, (int)&para);
}

#if 0 //this function be done in lib_subt.c
INT32  sdec_showonoff(struct sdec_device *dev,BOOL bOn)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->showonoff)
	{
		return dev->showonoff(dev,bOn);
	}
	return RET_FAILURE;
}
#endif
/*
 * 	Name		:   sdec_io_control()   	
 *	Description	:   sdecel IO control command
 *	Parameter	:	struct sto_device *dev		: Device
 *					INT32 cmd					: IO command
 *					UINT32 param				: Param
 *	Return		:	INT32 						: Result
 *
 */
INT32 sdec_ioctl(struct sdec_device *dev, UINT32 cmd, UINT32 param)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
	return !SUCCESS;
}

INT32 sdec_request_write(void * pdev, UINT32 uSizeRequested,void ** ppuData,UINT32* puSizeGot,struct control_block* pSDataCtrlBlk)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
	return ;
}

void sdec_update_write(void * pdev,UINT32 uDataSize)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
	return ;
}

void sdec_m36_attach(void)
{
	struct sdec_device *dev;

	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
	
	dev = dev_alloc(sdec_m36_name,HLD_DEV_TYPE_SDEC,sizeof(struct sdec_device));
	if (dev == NULL)
	{
		SDEC_PRINTF("%s : Error: Alloc video sdecplay device error!\n",__FUNCTION__);
		ASSERT(0);
	}

	if (dev_register(dev) != RET_SUCCESS)
	{
		SDEC_PRINTF("%s : Error: Register sdecplay device error!\n",__FUNCTION__);
		dev_free(dev);
		ASSERT(0);
	}
	SDEC_PRINTF("%s : SDEC(0x%08x) Attached!\n",__FUNCTION__, dev );
}


void lib_subt_atsc_set_buf(struct atsc_subt_config_par* p_config)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_SUBT_ATSC_SET_BUF, (void*)p_config);
}

UINT16 lib_subt_atsc_stream_identify(UINT16 length,UINT8 *data)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);
	struct subt_atsc_stream_identify arg;
	arg.length = length;
	arg.data = data;
	
	if (g_sdec_handle == 0)	{
		return;
	}
	ioctl(g_sdec_handle, CALL_SUBT_ATSC_STREAM_IDENTIFY, (void *)&arg);
}

BOOL lib_subt_atsc_create_task()
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_SUBT_ATSC_CREATE_TASK, NULL);
}

BOOL lib_subt_atsc_terminate_task()
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_SUBT_ATSC_TERMINATE_TASK, NULL);
}

void lib_subt_atsc_clean_up()
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_SUBT_ATSC_CLEAN_UP, NULL);
}

void lib_subt_atsc_delete_timer()
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_SUBT_ATSC_DEL_TIMER, NULL);
}

void lib_subt_atsc_show_onoff(BOOL onoff)
{
	SDEC_PRINTF("%s->%s()->%d.\n", __FILE__, __FUNCTION__, __LINE__);

	if (g_sdec_handle == 0)	{
		return;
	}
	
	ioctl(g_sdec_handle, CALL_SUBT_ATSC_SHOW_ONOFF, onoff);
}


