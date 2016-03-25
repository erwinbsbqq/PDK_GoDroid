/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: decv.c
 *
 *  Description: Hld vdec driver
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2003.04.11  David Wang     0.1.000   Initial
 *  2.  2010.03.11  Sam			4.0		Support Linux Driver
 ****************************************************************************/

#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld_cfg.h>
#include <adr_mediatypes.h>
#include <hld/decv/adr_decv.h>
#include <hld/dis/adr_vpo.h>

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

#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>

#include <hld/misc/rfk.h>

#ifdef ADR_IPC_ENABLE
#include <hld/misc/adr_ipc.h>

#define MUTEX_LOCK()	adr_ipc_semlock(m_vdec_mutex_id)
#define MUTEX_UNLOCK() adr_ipc_semunlock(m_vdec_mutex_id)
#else
#define MUTEX_LOCK() osal_mutex_lock(priv->mutex_id, OSAL_WAIT_FOREVER_TIME)			
#define MUTEX_UNLOCK() osal_mutex_unlock(priv->mutex_id)
#endif

#define VDEC_PRF(fmt, args...)  \
			do { \
				if (g_decv_dbg_on) \
				{ \
					ADR_DBG_PRINT(DECV, "%s->%s: L %d: "fmt"\n", __FILE__, \
						__FUNCTION__, __LINE__, ##args); \
				} \
			} while(0)


#define ENTRY()		VDEC_PRF("in\n")
#define EXIT()		VDEC_PRF("out\n")


#define VDEC_MAX_DEV_NUM			3

#define VDEC_MPG_DEV				0
#define VDEC_AVC_DEV				1
#define VDEC_AVS_DEV				2

struct vdec_private
{
	int handle; 
	char *file_path;
	
	volatile int open; /* 0 : device is closed; 1 : device is opened */
	volatile int start; /* 0 : device is idle; 1 : device is working*/	

	volatile void *req_buf;
	int req_size;
	int update_size;

	VDecCBFunc	pcb_first_showed;
	VDecCBFunc	pcb_mode_switch_ok;
	VDecCBFunc	pcb_backward_restart_gop;
	VDecCBFunc pcb_first_head_parsed;
	VDecCBFunc pcb_ff_fb_showed;
       VDecCBFunc  pcb_first_i_deocded;
	
	int task_id;
	int mutex_id;
	int rfk_port;
	
	UINT32 first_show:1;
	UINT32 res:31;
	
	struct ali_video_out_info_pars out_info;

	struct control_block ctrl_blk;
	int ctrl_blk_enable;
};

#ifdef HLD_DBG_ENABLE
int g_decv_dbg_on = 1;
#else
int g_decv_dbg_on = 0;
#endif
 
static char decv_avc_name[] = {"DECV_AVC_0"};
static char decv_mpg_name[] = {"DECV_S3601_0"};
static char decv_avs_name[] = {"DECV_AVS_0"};

static struct vdec_device *m_vdec_dev[VDEC_MAX_DEV_NUM] = {NULL, NULL, NULL};
static struct vdec_private *m_vdec_priv[VDEC_MAX_DEV_NUM] = {NULL, NULL, NULL};

static struct vdec_private m_vdec_priv_data;
static int m_vdec_attached = 0;

#ifdef ADR_IPC_ENABLE
static int m_vdec_mutex_id = 0;
#endif

static char video_path[] = "/dev/ali_video0";

/* 0 : mpeg2 
    1 : avc
*/
static int cur_decv_type = 0;
static BOOL cur_decv_mode = 0;

static int vdec_attach(char *file_path, struct vdec_device **pdev, int mpg)
{
	struct vdec_device *vdec_dev;
	struct vdec_private *priv;
	char *name;

	switch(mpg)
	{
		case 0:
			name = decv_avc_name;
			break;
		case 1:
			name = decv_mpg_name;
			break;
		case 2:
			name = decv_avs_name;
			break;
		default:
			return 0;
	}
	
	*pdev = NULL;

	
#ifdef ADR_IPC_ENABLE	
	adr_hld_global_semlock();
#endif

	vdec_dev = dev_alloc(name,HLD_DEV_TYPE_DECV,sizeof(struct vdec_device));
	if(vdec_dev == NULL)
	{
#ifdef ADR_IPC_ENABLE	
		adr_hld_global_semunlock();
#endif	
		VDEC_PRF("malloc vdec dev fail\n");
		return 0;
	}

	priv = &m_vdec_priv_data;	
	memset((void *)priv, 0, sizeof(*priv));	
    priv->handle = -1;
#ifdef ADR_IPC_ENABLE	

#else
	priv->file_path = file_path;
	vdec_dev->next = NULL;
	vdec_dev->priv = (void *)priv;
#endif

	vdec_dev->flags = 0;
	
	if(dev_register(vdec_dev) != RET_SUCCESS)
	{
#ifdef ADR_IPC_ENABLE	
		adr_hld_global_semunlock();
#endif		
		VDEC_PRF("register vdec dev fail\n");
		return 0;
	}
	
#ifdef ADR_IPC_ENABLE	
	adr_hld_global_semunlock();
#endif	

	*pdev = vdec_dev;
	return 1;
}

static void vdec_moniter_task(UINT32 upara1,UINT32 upara2)
{
#define VDEC_MONITER_TIME_OUT				(10)	

#define MSG_FIRST_SHOWED		1
#define MSG_MODE_SWITCH_OK	2
#define MSG_BACKWARD_RESTART_GOP	3
#define MSG_FIRST_HEADRE_PARSED		4
#define MSG_UPDATE_OUT_INFO			5
#define MSG_FIRST_I_DECODED       6
#define MSG_FF_FB_SHOWED            7

	struct vdec_device *dev = (struct vdec_device *)upara1;
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;
	uint8 *msg_data = NULL;
	uint32 msg_type = 0; 
	uint32 msg_len = 0;
	uint32 flags = 0;

	int ret = 0;

	VDEC_PRF("vdec enter the moniter task\n");
	osal_task_save_thread_info("DCV");

	while(1)
	{
		VDEC_PRF("start to wait new message\n");

		msg_data = rfk_receive_msg(priv->rfk_port);
        if(msg_data == NULL)
        {
            continue;
        }
        
		msg_type = msg_data[0];
		msg_len = msg_data[1];
		VDEC_PRF("receive msg type %d len %d\n", msg_type, msg_len);

		MUTEX_LOCK();			
		
		switch(msg_type)
		{
			case MSG_FIRST_SHOWED:
				if(priv->first_show == 0)
				{	
					struct ali_video_out_info_pars pars;						

					VDEC_PRF("get the MSG_FIRST_SHOWED\n");
					ioctl(priv->handle, ALIVIDEOIO_GET_OUT_INFO, &pars);
					VDEC_PRF("first show pic w %d h %d\n"
						, pars.width, pars.height);
					
					memcpy((void *)&(priv->out_info), (void *)&pars, sizeof(pars));
					
					if(priv->pcb_first_showed)
						priv->pcb_first_showed(0, 0);
					
					priv->first_show = 1;
				}
				break;
			case MSG_MODE_SWITCH_OK:
				if(priv->pcb_mode_switch_ok)
					priv->pcb_mode_switch_ok(msg_data[2], msg_data[3]);
				break;
			case MSG_BACKWARD_RESTART_GOP:
				if(priv->pcb_backward_restart_gop)
					priv->pcb_backward_restart_gop(msg_data[2], msg_data[3]);
				break;
			case MSG_FIRST_HEADRE_PARSED:
				if(priv->pcb_first_head_parsed)
					priv->pcb_first_head_parsed(msg_data[2], msg_data[3]);
				break;
			case MSG_UPDATE_OUT_INFO:
				if(msg_len == sizeof(priv->out_info))
					memcpy((void *)&priv->out_info, msg_data + 2, msg_len);
				break;
            		case MSG_FIRST_I_DECODED:
                		if(priv->pcb_first_i_deocded)
                    			priv->pcb_first_i_deocded(msg_data[2], msg_data[3]);
                		break;
			case MSG_FF_FB_SHOWED:
				if ( priv->pcb_ff_fb_showed )
					priv->pcb_ff_fb_showed(0,0);
				break;
			default:
				break;
		}
		
		MUTEX_UNLOCK();	
			
		if(!priv->start)			
			osal_task_sleep(VDEC_MONITER_TIME_OUT);
	}
}

RET_CODE vdec_open(struct vdec_device *dev)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;
	enum ali_decv_format format; 
	enum ALIFB_OUTPUT_FRAME_PATH out_frm_path;	
	void *fb_info = NULL;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(priv->open)
	{
		VDEC_PRF("vdec %s is already opened\n", dev->name);	
		return RET_SUCCESS;
	}
	
	priv->handle = open(video_path, O_RDWR | O_CLOEXEC);
	if(priv->handle < 0)
	{
		VDEC_PRF("%s : open file fail %s\n", __FUNCTION__, priv->file_path);
		return RET_FAILURE;
	}	

	VDEC_PRF("vdec handle %d %s\n", priv->handle, priv->file_path);

	/* get the free rfk port */
	priv->rfk_port = rfk_get_port();
	if(priv->rfk_port <= 0)
	{
		VDEC_PRF("%s : rfk get port fail\n", __FUNCTION__);
		goto FAIL;
	}
	
	/* create a task to moniter the status of ali video */	
	{
		OSAL_T_CTSK t_ctsk;

#ifdef ADR_IPC_ENABLE

#else
		priv->mutex_id = osal_mutex_create();
		if(priv->mutex_id == OSAL_INVALID_ID)
		{
			VDEC_PRF("vdec create mutex fail\n");
			goto FAIL;
		}
		VDEC_PRF("vdec create the mutex %d done\n", priv->mutex_id);
#endif		

		t_ctsk.stksz = 0x1000;
		t_ctsk.quantum = 15;
		t_ctsk.itskpri = OSAL_PRI_NORMAL;
		t_ctsk.para1 = (UINT32)(dev);
		t_ctsk.para2 = 0;
		t_ctsk.name[0] = 'D';
		t_ctsk.name[1] = 'C';
		t_ctsk.name[2] = 'V';
		t_ctsk.task = (FP)vdec_moniter_task;
		priv->task_id = osal_task_create(&t_ctsk);
		if(priv->task_id == OSAL_INVALID_ID)
		{
			VDEC_PRF("vdec create the moniter task fail\n");
			goto FAIL;
		}
		VDEC_PRF("vdec create the moniter task %d done\n", priv->task_id);

		osal_task_sleep(5);
		ioctl(priv->handle, ALIVIDEOIO_SET_SOCK_PORT_ID, priv->rfk_port);
	}
		
	priv->open = 1;
	priv->start = 0;	
	VDEC_PRF("open the vdec %s ok\n", dev->name);
	
	return RET_SUCCESS;

FAIL:
	if(priv->rfk_port > 0)
		rfk_free_port(priv->rfk_port);

	if(priv->task_id != OSAL_INVALID_ID)
		osal_task_delete(priv->task_id);

#ifdef ADR_IPC_ENABLE

#else
	if(priv->mutex_id != OSAL_INVALID_ID)
		osal_mutex_delete(priv->mutex_id);
#endif

	//if(priv->handle >= 0)
		close(priv->handle);		
	
	return RET_FAILURE;
}

RET_CODE vdec_close(struct vdec_device *dev)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(priv->start)
		vdec_stop(dev, 0, 0);

	if(priv->open)
	{		
		osal_task_delete(priv->task_id);
		rfk_free_port(priv->rfk_port);		

#ifdef ADR_IPC_ENABLE

#else		
		osal_mutex_delete(priv->mutex_id);
#endif

		close(priv->handle);
		priv->handle = -1;
		priv->open = 0;
		priv->start = 0;
	}
	
	return RET_SUCCESS;
}

RET_CODE vdec_start(struct vdec_device *dev)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }
#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
		return RET_FAILURE;
	}

	MUTEX_LOCK();
	
	if(priv->start)
	{
		MUTEX_UNLOCK();	
		
		VDEC_PRF("vdec is already started\n");
		return RET_SUCCESS;
	}

	ENTRY();

	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_START;
	rpc_pars.arg_num = 0;
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0)
		ret = RET_FAILURE;
	
	/* init some info */
	priv->req_buf = NULL;
	priv->req_size = priv->update_size = 0;
	priv->first_show = 0;
	priv->ctrl_blk_enable = 0;
	memset((void *)&priv->ctrl_blk, 0, sizeof(priv->ctrl_blk));
	memset((void *)&priv->out_info, 0, sizeof(priv->out_info));

	ioctl(priv->handle, ALIVIDEOIO_SET_SOCK_PORT_ID, priv->rfk_port);
		
	ioctl(priv->handle, ALIVIDEOIO_VIDEO_PLAY, 0);	

	priv->start = 1;

	EXIT();
	
	MUTEX_UNLOCK();
	
	return ret;
}

RET_CODE vdec_stop(struct vdec_device *dev,BOOL bclosevp,BOOL bfillblack)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;
	struct ali_video_rpc_pars rpc_pars;		
	RET_CODE ret = RET_SUCCESS;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }
#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
		return RET_FAILURE;
	}

	MUTEX_LOCK();

	if(!priv->start)
	{	
		MUTEX_UNLOCK();	
		
		VDEC_PRF("vdec is already stopped\n");
		return RET_SUCCESS;
	}	

	ENTRY();

	if(priv->req_buf)
	{
		free((void *)(priv->req_buf));
		priv->req_buf = NULL;
	}

	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_STOP;
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)&bclosevp;
	rpc_pars.arg[0].arg_size = sizeof(bclosevp);			
	rpc_pars.arg[1].arg = (void *)&bfillblack;
	rpc_pars.arg[1].arg_size = sizeof(bfillblack);
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0)
		ret = RET_FAILURE;
	
	ioctl(priv->handle, ALIVIDEOIO_VIDEO_STOP, 0);	

	priv->start = 0;

	EXIT();
	
	MUTEX_UNLOCK();	
	
	return ret;
}

RET_CODE vdec_vbv_request(void *dev, UINT32 uSizeRequested, void ** ppVData, UINT32 * puSizeGot, struct control_block * ctrl_blk)
{
	struct vdec_device *decv_dev = (struct vdec_device *)dev;	
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, decv_dev->name);
		return RET_STA_ERR;
	}

	MUTEX_LOCK();
	
	if(!priv->start)
	{	
		MUTEX_UNLOCK();		
		
		VDEC_PRF("vdec %s is stopped or closed\n", decv_dev->name);
		return RET_STA_ERR;
	}
	
	ENTRY();

	priv->ctrl_blk_enable = 0;	
	if(ctrl_blk)
	{
		memcpy((void *)&priv->ctrl_blk, ctrl_blk, sizeof(*ctrl_blk));
		priv->ctrl_blk_enable = 1;
	}
	
	priv->req_buf = malloc(uSizeRequested);
	if(priv->req_buf == NULL)
	{	
		MUTEX_UNLOCK();	
		
		VDEC_PRF("malloc request buffer fail\n");
		return RET_STA_ERR;
	}
	
	priv->req_size = uSizeRequested;
	priv->update_size = 0;
	*ppVData = (void *)(priv->req_buf);
	*puSizeGot = priv->req_size;

	EXIT();

	MUTEX_UNLOCK();	
	
	return RET_SUCCESS;
	
}

void vdec_vbv_update(void *dev, UINT32 uDataSize)
{
	struct vdec_device *decv_dev = (struct vdec_device *)dev;	
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;
	int write_size = 0;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, decv_dev->name);
		return;
	}

	MUTEX_LOCK();

	if(!priv->start)
	{	
		MUTEX_UNLOCK();	
				
		VDEC_PRF("vdec %s is stopped or closed\n", decv_dev->name);
		return;
	}
	
	ENTRY();

	if(priv->ctrl_blk_enable)
	{
		ioctl(priv->handle, ALIVIDEOIO_SET_CTRL_BLK_INFO, &priv->ctrl_blk);
		priv->ctrl_blk_enable = 0;
	}
		
	while(1)
	{
		write_size = write(priv->handle, (void *)(priv->req_buf + priv->update_size), uDataSize);
		if(write_size > 0)
		{
			uDataSize -= write_size;
			priv->update_size += write_size;
			if(uDataSize == 0)
				break;
		}	
		osal_task_sleep(10);			
	}

	if(priv->update_size >= priv->req_size)
	{
		// VDEC_PRF("1 %x ", (int)priv->req_buf);		
		free((void *)(priv->req_buf));
		priv->req_buf = NULL;
	}
	
	EXIT();
	
	MUTEX_UNLOCK();
}

RET_CODE vdec_set_output(struct vdec_device *dev, enum VDecOutputMode eMode,struct VDecPIPInfo *pInitInfo, 	struct MPSource_CallBack *pMPCallBack, struct PIPSource_CallBack *pPIPCallBack)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;
	struct ali_video_rpc_pars rpc_pars;		
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
		return RET_FAILURE;
	}

	MUTEX_LOCK();

	ENTRY();
			
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_SET_OUT;
	rpc_pars.arg_num = 3;
	rpc_pars.arg[0].arg = (void *)&eMode;
	rpc_pars.arg[0].arg_size = sizeof(eMode);			
	rpc_pars.arg[1].arg = (void *)pInitInfo;
	rpc_pars.arg[1].arg_size = sizeof(*pInitInfo);		
	rpc_pars.arg[2].arg = (void *)pMPCallBack;
	rpc_pars.arg[2].arg_size = sizeof(*pMPCallBack);		
	rpc_pars.arg[2].out = 1;		
	rpc_pars.arg[3].arg = (void *)pPIPCallBack;
	rpc_pars.arg[3].arg_size = sizeof(*pPIPCallBack);				
	rpc_pars.arg[3].out = 1;		
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0)
		ret = RET_FAILURE;

	EXIT();
	
	MUTEX_UNLOCK();
	
	return ret;
}

RET_CODE vdec_switch_pip_ext(struct vdec_device *dev, struct Rect *pPIPWin)
{
    return RET_FAILURE;
}
RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 uSyncMode,UINT8 uSyncLevel)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;
	struct ali_video_rpc_pars rpc_pars;		
	UINT32 mode = uSyncMode, level = uSyncLevel;
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();	

	ENTRY();
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_SYNC_MODE;
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)&mode;
	rpc_pars.arg[0].arg_size = sizeof(mode);			
	rpc_pars.arg[1].arg = (void *)&level;
	rpc_pars.arg[1].arg_size = sizeof(level);			
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0)
		ret = RET_FAILURE;

	EXIT();
	
	MUTEX_UNLOCK();
	
	return ret;
}
RET_CODE vdec_extrawin_store_last_pic(struct vdec_device *dev, struct Rect *pPIPWin)
{
    return RET_FAILURE;
}
RET_CODE vdec_profile_level(struct vdec_device *dev, UINT8 uProfileLevel,VDEC_BEYOND_LEVEL cb_beyond_level)
{
    return RET_FAILURE;
}

RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
	RET_CODE ret = RET_SUCCESS;
	struct ali_video_rpc_pars rpc_pars;	
       if(NULL == dev)
       {
            VDEC_PRF("%s : don't open dev\n", __FUNCTION__);
            return RET_FAILURE;
       }
#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
		return RET_FAILURE;
	}

	VDEC_PRF("io cmd %d param %d\n", io_code, param);
	
	MUTEX_LOCK();

	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));	
	switch(io_code)
	{
		case VDEC_IO_GET_STATUS:
		{
			if(0)//dev == m_vdec_dev[cur_decv_type])
			{
				struct VDec_StatusInfo *pCurStatus = (struct VDec_StatusInfo *)(param);
				struct ali_video_out_info_pars *out_info = &priv->out_info;

				memset((void *)pCurStatus, 0, sizeof(*pCurStatus));
				pCurStatus->uCurStatus = (priv->start) ? VDEC27_STARTED : VDEC27_STOPPED;
				pCurStatus->uFirstPicShowed = out_info->flag & ALI_VIDEO_OUT_INFO_FLAG_FIRST_SHOWED
					? TRUE : FALSE;
				//pCurStatus->bFirstHeaderGot = out_info->flag & ALI_VIDEO_OUT_INFO_FLAG_FIRST_HEADER
				//	? TRUE : FALSE;
				pCurStatus->bFirstHeaderGot = out_info->flag & ALI_VIDEO_OUT_INFO_FLAG_FIRST_SHOWED
					? TRUE : FALSE;			
				pCurStatus->pic_width = out_info->width;
				pCurStatus->pic_height = out_info->height;
				pCurStatus->frame_rate = out_info->frame_rate;
				pCurStatus->display_idx = out_info->display_idx;
				pCurStatus->read_p_offset = out_info->read_p_offset;
				pCurStatus->write_p_offset = out_info->write_p_offset;
				pCurStatus->valid_size = out_info->valid_size;
				pCurStatus->is_support = out_info->is_support;
			}
			else
			{
				rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
				rpc_pars.arg_num = 2;
				rpc_pars.arg[0].arg = (void *)&io_code;
				rpc_pars.arg[0].arg_size = sizeof(io_code);			
				rpc_pars.arg[1].arg = (void *)param;
				rpc_pars.arg[1].arg_size = sizeof(struct VDec_StatusInfo);
				rpc_pars.arg[1].out = 1;
				ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
				if(ret < 0)
					ret = RET_FAILURE;				
			}
			
			break;
		}		
		case VDEC_IO_GET_MODE:
		{	
			struct ali_video_out_info_pars *out_info = &priv->out_info;			
			int frame_rate;
			int source_width;

			if(!(out_info->flag & ALI_VIDEO_OUT_INFO_FLAG_FIRST_SHOWED))
			{
				ret = RET_FAILURE;
				break;
			}
			
			frame_rate = priv->out_info.frame_rate;
			source_width = priv->out_info.width;
			
			switch((frame_rate + 999) / 1000)
			{
				case 25:
				case 50:
					if(source_width <= 720)
						*(enum TVSystem *)(param) =PAL;
					else if(source_width <= 1280)
						*(enum TVSystem *)(param) =LINE_720_25;
					else 
				            *(enum TVSystem *)(param) = LINE_1080_25;
					break;
				case 24:
				case 30:
				case 60:
					if(source_width<=720)
						*(enum TVSystem *)(param) =NTSC;
					else if(source_width<=1280)
						*(enum TVSystem *)(param) =LINE_720_30;
					else
						*(enum TVSystem *)(param) = LINE_1080_30;
					break;
				default:
					ret = RET_FAILURE;
					break;
			}
			
			break;
		}		
		case VDEC_IO_GET_OUTPUT_FORMAT:
		{
			struct ali_video_out_info_pars *out_info = &priv->out_info;	
			
		
			if(!(out_info->flag & ALI_VIDEO_OUT_INFO_FLAG_FIRST_SHOWED))
			{					
				ret = RET_FAILURE;				
				break;
			}		
			
			*(BOOL *)param = out_info->progressive;
			
			break;
		}
		case VDEC_IO_FILL_FRM:
		{			
			rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
			rpc_pars.arg_num = 2;
			rpc_pars.arg[0].arg = (void *)&io_code;
			rpc_pars.arg[0].arg_size = sizeof(io_code);			
			rpc_pars.arg[1].arg = (void *)param;
			rpc_pars.arg[1].arg_size = sizeof(struct YCbCrColor);
			ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
			if(ret < 0)
				ret = RET_FAILURE;	
			
			break;
		}
		case VDEC_IO_REG_CALLBACK:
		{
			struct vdec_io_reg_callback_para *ppara = (struct vdec_io_reg_callback_para *)(param);
			
			switch(ppara->eCBType)
			{
				case VDEC_CB_FIRST_SHOWED:
					priv->pcb_first_showed = ppara->pCB;
					break;
				case VDEC_CB_MODE_SWITCH_OK:
					priv->pcb_mode_switch_ok = ppara->pCB;
					break;
				case VDEC_CB_BACKWARD_RESTART_GOP: // used for advance play
					priv->pcb_backward_restart_gop = ppara->pCB;
					break;
				case VDEC_CB_FIRST_HEAD_PARSED:
					priv->pcb_first_head_parsed=ppara->pCB;
					break;
                		case VDEC_CB_FIRST_I_DECODED:
                    			priv->pcb_first_i_deocded = ppara->pCB;
                    			break;
				case VDEC_CB_FF_FB_SHOW:
					priv->pcb_ff_fb_showed = ppara->pCB;
					break;
				default:
					break;
			}
			
			break;
		}
		case VDEC_IO_SET_OUTPUT_RECT:
		{
			rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
			rpc_pars.arg_num = 2;
			rpc_pars.arg[0].arg = (void *)&io_code;
			rpc_pars.arg[0].arg_size = sizeof(io_code);			
			rpc_pars.arg[1].arg = (void *)param;
			rpc_pars.arg[1].arg_size = sizeof(struct VDecPIPInfo);
			ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
			if(ret < 0)
				ret = RET_FAILURE;				
			break;
		}
		case VDEC_IO_COLORBAR:  //only valid when vp had got one framebuffer from ve and vp.mp layer is opened.
		case VDEC_IO_PULLDOWN_ONOFF:
		case VDEC_IO_SWITCH:
		case VDEC_IO_DONT_RESET_SEQ_HEADER: 
		case VDEC_IO_SET_DROP_FRM:
		case VDEC_SET_DMA_CHANNEL:
		case VDEC_DTVCC_PARSING_ENABLE:
		case VDEC_VBV_BUFFER_OVERFLOW_RESET:
		case VDEC_IO_SET_DMA_CHANNEL_NUMBER:
		case VDEC_IO_SET_BOOTLOGO_ADDR:
		case VDEC_IO_CONTINUE_ON_ERROR:
		case VDEC_IO_SET_QUICK_PLAY_MODE:
		case VDEC_IO_SET_STILL_FRAME_MODE:
		case VDEC_IO_SET_SMOOTH_PREVIEW:
		case VDEC_IO_CLEAN_STILL_FRAME:	
		case VDEC_IO_SET_FB_COST_DOWN_NUM:
		case VDEC_IO_SET_RGB_OUTPUT_FLAG:
		case VDEC_IO_SET_SYNC_DELAY:
		case VDEC_IO_SAR_ENABLE:
		case VDEC_IO_FIRST_I_FREERUN:
		case VDEC_IO_SET_MODULE_INFO:
		case VDEC_IO_SET_DEC_FRM_TYPE:
		{		
			memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
			rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
			rpc_pars.arg_num = 2;
			rpc_pars.arg[0].arg = (void *)&io_code;
			rpc_pars.arg[0].arg_size = sizeof(io_code);			
			rpc_pars.arg[1].arg = (void *)&param;
			rpc_pars.arg[1].arg_size = 4;
			ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
			if(ret < 0)
				ret = RET_FAILURE;
			
			break;
		}	
		case VDEC_IO_CAPTURE_DISPLAYING_FRAME:
		{
			rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
			rpc_pars.arg_num = 2;
			rpc_pars.arg[0].arg = (void *)&io_code;
			rpc_pars.arg[0].arg_size = sizeof(io_code);			
			rpc_pars.arg[1].arg = (void *)param;
			rpc_pars.arg[1].arg_size = sizeof(struct vdec_picture);
			rpc_pars.arg[1].out = 1;
			ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
			if(ret < 0)
				ret = RET_FAILURE;
			
			break;
		}
		case VDEC_IO_GET_DECORE_STATUS:
		{
			rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
			rpc_pars.arg_num = 2;
			rpc_pars.arg[0].arg = (void *)&io_code;
			rpc_pars.arg[0].arg_size = sizeof(io_code);			
			rpc_pars.arg[1].arg = (void *)param;
			rpc_pars.arg[1].arg_size = sizeof(struct vdec_decore_status);
			rpc_pars.arg[1].out = 1;
			ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);			
			break;
		}
#if 0		
		case VDEC_IO_GET_INPUT_CALLBACK_ROUTINE:
		{
			ioctl(priv->handle, ALIVIDEOIO_GET_INPUT_CALLBACK_ROUTINE, param);
			break;
		}
#endif		
		default:
			ret = RET_FAILURE;
			
			break;
	}

	MUTEX_UNLOCK();		
			
	return ret;
}

RET_CODE vdec_playmode(struct vdec_device *dev, enum VDecDirection direction, enum VDecSpeed speed)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_PLAY_MODE;
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)&direction;
	rpc_pars.arg[0].arg_size = sizeof(direction);			
	rpc_pars.arg[1].arg = (void *)&speed;
	rpc_pars.arg[1].arg_size = sizeof(speed);
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0)
		ret = RET_FAILURE;

	EXIT();
	
	MUTEX_UNLOCK();	

	return ret;
}

RET_CODE vdec_dvr_set_param(struct vdec_device *dev, struct VDec_DvrConfigParam param)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
	struct ali_video_rpc_pars rpc_pars;			
	int ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_DVR_SET_PAR;
	rpc_pars.arg_num = 1;
	rpc_pars.arg[0].arg = (void *)&param;
	rpc_pars.arg[0].arg_size = sizeof(param);			
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0)
		ret = RET_FAILURE;
	
	EXIT();
	
	MUTEX_UNLOCK();	

	return ret;
}

RET_CODE vdec_dvr_pause(struct vdec_device *dev)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev \n", __FUNCTION__);
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_DVR_PAUSE;
	rpc_pars.arg_num = 0;		
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0)
		ret = RET_FAILURE;

	EXIT();
	
	MUTEX_UNLOCK();	

	return ret;
}

RET_CODE vdec_dvr_resume(struct vdec_device *dev)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_DVR_RESUME;
	rpc_pars.arg_num = 0;	
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0)
		ret = RET_FAILURE;

	EXIT();
	
	MUTEX_UNLOCK();	

	return ret;
}

RET_CODE vdec_step(struct vdec_device *dev)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_DVR_STEP;
	rpc_pars.arg_num = 0;	
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0)
		ret = RET_FAILURE;

	EXIT();
	
	MUTEX_UNLOCK();	

	return ret;
}

BOOL is_cur_decoder_avc(void)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open\n", __FUNCTION__);
		return FALSE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_IS_AVC;
	rpc_pars.arg_num = 0;	
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	
	cur_decv_type = (ret == TRUE) ? 1 : 0;
	
	EXIT();
	
	MUTEX_UNLOCK();

    	return (cur_decv_type == 1 ? TRUE : FALSE);
}

struct vdec_device * get_selected_decoder(void)
{
	return get_current_decoder_device();
}

void set_avc_output_mode_check_cb(VDecCBFunc pCB)
{
    do{}while(0);
}
/*
	select : 0 : mpg decoder; 1 : H264 decoder
	in_preview : 0 : main pic mode; 1 : preview mode
*/
void h264_decoder_select(int select, BOOL in_preview)
{
	struct vdec_private *priv = &m_vdec_priv_data;
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open\n", __FUNCTION__);
		return;
	}
	
	MUTEX_LOCK();
	
	ENTRY();
	
	VDEC_PRF("in select %d in preview %d curret select %d current preview %d \n", select, in_preview
		, cur_decv_type, cur_decv_mode);

	// if((select != cur_decv_type) || (in_preview != cur_decv_mode))
	{
		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.API_ID = RPC_VIDEO_SELECT_DEC;
		rpc_pars.arg_num = 2;	
		rpc_pars.arg[0].arg = (void *)&select;
		rpc_pars.arg[0].arg_size = sizeof(select);			
		rpc_pars.arg[1].arg = (void *)&in_preview;
		rpc_pars.arg[1].arg_size = sizeof(in_preview);	
		ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;

		cur_decv_type = (select == 0) ? 0 : 1;
		cur_decv_mode = in_preview;

		ioctl(priv->handle, ALIVIDEOIO_VIDEO_STOP, 0);	

		priv->start = 0;
	}
	
	EXIT();
	
	MUTEX_UNLOCK();
}

/*
	select : 0 : mpg decoder; 1 : H264 decoder; 2 : avs decoder
	in_preview : 0 : main pic mode; 1 : preview mode
*/
void video_decoder_select(enum video_decoder_type select, BOOL in_preview)
{
	struct vdec_private *priv = &m_vdec_priv_data;
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open\n", __FUNCTION__);
		return;
	}
	
	MUTEX_LOCK();
	
	ENTRY();
	
	VDEC_PRF("video_decoder_select in select %d in preview %d curret select %d current preview %d \n", select, in_preview
		, cur_decv_type, cur_decv_mode);

	// if((select != cur_decv_type) || (in_preview != cur_decv_mode))
	{
		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.API_ID = RPC_VIDEO_DECODER_SELECT_NEW;
		rpc_pars.arg_num = 2;	
		rpc_pars.arg[0].arg = (void *)&select;
		rpc_pars.arg[0].arg_size = sizeof(select);			
		rpc_pars.arg[1].arg = (void *)&in_preview;
		rpc_pars.arg[1].arg_size = sizeof(in_preview);	
		ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;

		cur_decv_mode = in_preview;
		if(select == MPEG2_DECODER)
			cur_decv_type = 0;
		else if(select == H264_DECODER)
			cur_decv_type = 1;
		else if(select == AVS_DECODER)
			cur_decv_type = 2;

		ioctl(priv->handle, ALIVIDEOIO_VIDEO_STOP, 0);	

		priv->start = 0;
	}
	
	EXIT();
	
	MUTEX_UNLOCK();
}

enum video_decoder_type get_current_decoder(void)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open\n", __FUNCTION__);
		return FALSE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_GET_DECODER;
	rpc_pars.arg_num = 0;	
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	
	if(ret == MPEG2_DECODER)
		cur_decv_type = 0;
	else if(ret == H264_DECODER)
		cur_decv_type = 1;
	else if(ret == AVS_DECODER)
		cur_decv_type = 2;
	
	EXIT();
	
	MUTEX_UNLOCK();

    	return ret;
}

struct vdec_device * get_current_decoder_device(void)
{
	struct vdec_device *dev = NULL;
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open\n", __FUNCTION__);
		return NULL;
	}
	
	MUTEX_LOCK();

	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_GET_DECODER;
	rpc_pars.arg_num = 0;	
	ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);

	if(ret == MPEG2_DECODER)
		cur_decv_type = 0;
	else if(ret == H264_DECODER)
		cur_decv_type = 1;
	else if(ret == AVS_DECODER)
		cur_decv_type = 2;
	
	MUTEX_UNLOCK();

	if(cur_decv_type == 0)
		dev = m_vdec_dev[VDEC_MPG_DEV];
	else if(cur_decv_type == 1)
		dev = m_vdec_dev[VDEC_AVC_DEV];
	else if(cur_decv_type == 2)
		dev = m_vdec_dev[VDEC_AVS_DEV];
	
	return dev;
}

RET_CODE vdec_decore_ioctl(void *phandle, int cmd, void *param1, void *param2)
{
    struct vdec_private *priv = &m_vdec_priv_data;
    struct ali_video_rpc_pars rpc_pars;
    RET_CODE ret = RET_SUCCESS;

    MUTEX_LOCK();

    VDEC_PRF("In ======%s====== 0x%x\n", __FUNCTION__,(unsigned int)priv);

    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));

    rpc_pars.API_ID = RPC_VIDEO_DECORE_IOCTL;
    rpc_pars.arg_num = 4;

    switch(cmd)
    {
        case VDEC_CMD_INIT:
            rpc_pars.arg[0].arg = (void *)phandle;
            rpc_pars.arg[0].arg_size = sizeof(phandle);
            rpc_pars.arg[1].arg = (void *)&cmd;
            rpc_pars.arg[1].arg_size = sizeof(cmd);
            rpc_pars.arg[2].arg = (void *)param1;
            rpc_pars.arg[2].arg_size = sizeof(param1);
            rpc_pars.arg[3].arg = (void *)param2;
            rpc_pars.arg[3].arg_size = sizeof(VdecInit);
            break;
        case VDEC_CMD_EXTRA_DADA:
        case VDEC_CMD_RELEASE:
        case VDEC_CMD_SW_RESET:
        case VDEC_CMD_HW_RESET:
        case VDEC_CMD_PAUSE_DECODE:
        case VDEC_CFG_VIDEO_SBM_BUF:
        case VDEC_CFG_DISPLAY_SBM_BUF:
        case VDEC_CFG_SYNC_MODE:
        case VDEC_CFG_SYNC_THRESHOLD:
            rpc_pars.arg[0].arg = (void *)phandle;
            rpc_pars.arg[0].arg_size = sizeof(phandle);
            rpc_pars.arg[1].arg = (void *)&cmd;
            rpc_pars.arg[1].arg_size = sizeof(cmd);
            rpc_pars.arg[2].arg = (void *)param1;
            rpc_pars.arg[2].arg_size = sizeof(param1);
            rpc_pars.arg[3].arg = (void *)param2;
            rpc_pars.arg[3].arg_size = sizeof(param2);
            break;
        case VDEC_CFG_DISPLAY_RECT:
            rpc_pars.arg[0].arg = (void *)phandle;
            rpc_pars.arg[0].arg_size = sizeof(phandle);
            rpc_pars.arg[1].arg = (void *)&cmd;
            rpc_pars.arg[1].arg_size = sizeof(cmd);
            rpc_pars.arg[2].arg = (void *)param1;
            rpc_pars.arg[2].arg_size = sizeof(struct Video_Rect);
            rpc_pars.arg[3].arg = (void *)param2;
            rpc_pars.arg[3].arg_size = sizeof(struct Video_Rect);
            break;
        case VDEC_CMD_GET_STATUS:
            rpc_pars.arg[0].arg = (void *)phandle;
            rpc_pars.arg[0].arg_size = sizeof(phandle);
            rpc_pars.arg[1].arg = (void *)&cmd;
            rpc_pars.arg[1].arg_size = sizeof(cmd);
            rpc_pars.arg[2].arg = (void *)param1;
            rpc_pars.arg[2].arg_size = sizeof(struct vdec_decore_status);
            rpc_pars.arg[2].out = 1;
            rpc_pars.arg[3].arg = (void *)param2;
            rpc_pars.arg[3].arg_size = sizeof(struct vdec_decore_status);
            rpc_pars.arg[3].out = 1;
            break;
        default:
            return RET_FAILURE;
    }

    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    if(ret < 0)
    {
        VDEC_PRF("decore ioctl fail: %d %d\n", priv->handle, ret);
        ret = RET_FAILURE;
    }

    VDEC_PRF("Out ======%s======\n\n", __FUNCTION__);

    MUTEX_UNLOCK();

    return ret;
}

static void HLD_vdec_attach_avc(void)
{
	if(m_vdec_dev[VDEC_AVC_DEV])
	{
		VDEC_PRF("avc decoder is already attached\n");
		return;
	}

	if(!vdec_attach(video_path, &m_vdec_dev[VDEC_AVC_DEV], 0))
	{
		VDEC_PRF("attach avc decoder fail\n");
		return;
	}
	
	//m_vdec_priv[VDEC_AVC_DEV] = (struct vdec_private *)m_vdec_dev[VDEC_AVC_DEV]->priv;
	VDEC_PRF("attach hld avc decoder ok\n");
}

static void HLD_vdec_attach_mpg(void)
{
	if(m_vdec_dev[VDEC_MPG_DEV])
	{
		VDEC_PRF("mpg decoder is already attached\n");
		return;
	}

	if(!vdec_attach(video_path, &m_vdec_dev[VDEC_MPG_DEV], 1))
	{
		VDEC_PRF("attach mpg decoder fail\n");
		return;
	}
	
	// m_vdec_priv[VDEC_MPG_DEV] = (struct vdec_private *)m_vdec_dev[VDEC_MPG_DEV]->priv;
	VDEC_PRF("attach hld mpg decoder ok\n");	
}

static void HLD_vdec_attach_avs(void)
{
	if(m_vdec_dev[VDEC_AVS_DEV])
	{
		VDEC_PRF("avs decoder is already attached\n");
		return;
	}

	if(!vdec_attach(video_path, &m_vdec_dev[VDEC_AVS_DEV], 2))
	{
		VDEC_PRF("attach avs decoder fail\n");
		return;
	}
	
	//m_vdec_priv[VDEC_AVS_DEV] = (struct vdec_private *)m_vdec_dev[VDEC_AVS_DEV]->priv;
	VDEC_PRF("attach hld avs decoder ok\n");
}

void HLD_vdec_attach(void)
{
	if(m_vdec_attached == 1)
	{
		VDEC_PRF("vdec is already attached\n");
		return;
	}
	
	HLD_vdec_attach_mpg();
	HLD_vdec_attach_avc();
	HLD_vdec_attach_avs();

#ifdef ADR_IPC_ENABLE
	m_vdec_mutex_id = adr_ipc_semget(ADR_IPC_DECV, 0, 1);
	if(m_vdec_mutex_id < 0)
	{
		VDEC_PRF("vdec create mutex fail\n");
		return;
	}
#endif
	decvdbg_register();

	m_vdec_attached = 1;
	
	vdec_open(m_vdec_dev[0]);
}

void HLD_vdec_detach(void)
{
	int i = 0;

	if(m_vdec_attached == 0)
	{
		VDEC_PRF("vdec is already detached\n");
		return;
	}
	
	if(m_vdec_dev[0])
		vdec_close(m_vdec_dev[0]);
	
	for(i = 0;i < VDEC_MAX_DEV_NUM;i++)
	{
		if(m_vdec_dev[i])
		{
			dev_free(m_vdec_dev[i]);
			m_vdec_dev[i] = NULL;
		}
	}

	m_vdec_attached = 0;
}

void vdec_set_dbg_level(int hld, int kernel, int see)
{
	struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;	
    int video_fd = priv->handle;

	MUTEX_LOCK();

    if(video_fd < 0)
    {
        video_fd = open(video_path, O_RDWR|O_CLOEXEC);
        if(video_fd < 0)
        {
            VDEC_PRF("video device open fail\n");
            return;
        }
    }

	if(hld)
		g_decv_dbg_on = 1;
	else
		g_decv_dbg_on = 0;

	if(kernel)
		ioctl(video_fd, ALIVIDEOIO_ENABLE_DBG_LEVEL, 1);
	else
		ioctl(video_fd, ALIVIDEOIO_DISABLE_DBG_LEVEL, 1);

	if(see)
		ioctl(video_fd, ALIVIDEOIO_ENABLE_DBG_LEVEL, 2);
	else
		ioctl(video_fd, ALIVIDEOIO_DISABLE_DBG_LEVEL, 2);	

    if(priv->handle < 0)
    {
        if(video_fd >= 0)
        {
            close(video_fd);
        }
    }

	MUTEX_UNLOCK();		
}


