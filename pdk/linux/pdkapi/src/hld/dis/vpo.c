/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: vpo.c
 *
 *  Description: Hld video processer and tv encoder driver
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2003.04.11  David Wang     0.1.000   Initial
 *  2.  2010.03.11  Sam				4.0		Support Linux Driver
 ****************************************************************************/

#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld_cfg.h>
#include <adr_mediatypes.h>
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

#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>

#include "adr_vpo_dbg.h"

#ifdef ADR_IPC_ENABLE
#include <hld/misc/adr_ipc.h>

#define VPO_PRF(fmt, args...)  \
			do { \
				if(p_vpo_dbg_on != NULL){ \
					if(*p_vpo_dbg_on){ \
						ADR_DBG_PRINT(VPO, "%s->%s: L %d: "fmt"\n", __FILE__, \
							__FUNCTION__, __LINE__, ##args); \
					} \
				} \
			} while(0)


#define MUTEX_LOCK()	adr_ipc_semlock(m_vpo_mutex_id)
#define MUTEX_UNLOCK() adr_ipc_semunlock(m_vpo_mutex_id)
#else
#define VPO_PRF(fmt, args...)  \
			do { \
				if(g_vpo_dbg_on) \
				{ \
					ADR_DBG_PRINT(VPO, "%s->%s: L%d: "fmt"\n", __FILE__, \
						__FUNCTION__, __LINE__, ##args); \
				} \
			} while(0)

#define MUTEX_LOCK()	osal_mutex_lock(m_vpo_mutex_id, OSAL_WAIT_FOREVER_TIME)
#define MUTEX_UNLOCK() osal_mutex_unlock(m_vpo_mutex_id)
#endif

#define ENTRY()		VPO_PRF("in\n")
#define EXIT()		VPO_PRF("out\n")

#define MAX_VPO_NUM				2

#ifdef ADR_IPC_ENABLE
/* ipc key index */
#define IPC_VPO_SEM_KEY_IDX				0
#define IPC_VPO_PRIV_KEY_IDX				0
#endif

struct vpo_private
{
	int handle;
	char *file_path;
	int idx;

	/* whether it is the hd output */
	int hd;
	
	/* layer on or not*/
	int on;

	/*win zoom pars*/
	struct Rect src_rect;
	struct Rect dst_rect;

	/*aspect pars*/
	enum TVMode aspect_mode;
	enum DisplayMode display_mode;

	/*tvsys pars*/
	int progressive;
	enum TVSystem tv_sys;

#ifdef ADR_IPC_ENABLE
	int opened;
	int inited;
	int dbg_on;

	int shmid;
#endif	
};

#ifdef ADR_IPC_ENABLE
static struct vpo_private *m_vpo_priv[MAX_VPO_NUM];

int *p_vpo_dbg_on = NULL;
#else
int g_vpo_dbg_on = 0;
#endif

static char m_vpo_name_sd[] = {"VPO_S3602F_SD"};
static char m_vpo_name_hd[] = {"VPO_S3602F_HD"};
static struct vpo_device *m_vpo_dev[MAX_VPO_NUM];
static int m_vpo_opened[MAX_VPO_NUM];
static int m_vpo_attached = 0;
#ifdef ADR_ALIDROID
static char m_fb1_path[] = "/dev/graphics/fb1";
#else
static char m_fb1_path[] = "/dev/fb1";
#endif
static int m_fb1_handle = 0;

static int m_vpo_mutex_id = 0;

RET_CODE vpo_open(struct vpo_device *dev,struct VP_InitInfo *pInitInfo)
{
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif
	RET_CODE ret = RET_SUCCESS;
	
	ENTRY();

#ifdef ADR_IPC_ENABLE
	if(m_vpo_attached == 0)
		HLD_vpo_attach();
#endif	
		
	if(m_vpo_opened[priv->idx] > 0)
	{
		VPO_PRF("vpo dev is already opend\n");
		return RET_SUCCESS;
	}
		
	if(m_fb1_handle == 0)
	{
		m_fb1_handle = open(m_fb1_path, O_RDWR | O_CLOEXEC);
		if(m_fb1_handle <= 0)
		{
			VPO_PRF("%s : open file \n", __FUNCTION__);
			return RET_FAILURE;
		}	
	}

#ifdef ADR_IPC_ENABLE

#else
	priv->handle = m_fb1_handle;
#endif


#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();

	if(priv->opened == 0)
#endif
	{
		struct ali_fb_rpc_pars rpc_pars;			
		struct vpo_io_get_info info;
		UINT32 io_cmd = VPO_IO_GET_INFO;

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		memset((void *)&info, 0, sizeof(info));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_IOCTL;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)&io_cmd;
		rpc_pars.arg[0].arg_size = sizeof(io_cmd);	
		rpc_pars.arg[1].arg = (void *)(&info);
		rpc_pars.arg[1].arg_size = sizeof(info);		
		rpc_pars.arg[1].out = 1;	

		ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
		{
			VPO_PRF("get info fail\n");
			ret = RET_FAILURE;
			goto FAIL;
		}

		priv->tv_sys = info.tvsys;
		priv->progressive = info.bprogressive;		
	}

#ifdef ADR_IPC_ENABLE
	priv->opened++;	
	if(priv->opened > 0x10000)
		priv->opened = 1;

	MUTEX_UNLOCK();
#endif
	
	m_vpo_opened[priv->idx] = 1;
	
	VPO_PRF("vpo hd %d default tv %d\n", priv->hd, priv->tv_sys);

FAIL:	
	EXIT();
	
	return RET_SUCCESS;
}

RET_CODE vpo_close(struct vpo_device *dev)
{	
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif
	RET_CODE ret = RET_SUCCESS;
	
	ENTRY();

#ifdef ADR_IPC_ENABLE
	if(m_vpo_attached == 0)
		HLD_vpo_attach();
#endif	
	
	if(m_vpo_opened[priv->idx] == 0)
	{
		VPO_PRF("vpo dev is already closed\n");
		return RET_SUCCESS;
	}

#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();

	priv->opened--;	
	if(priv->opened < 0)
	{			
		VPO_PRF("fail opened count %d\n", priv->opened);
		
		priv->opened = 0;
	}

	MUTEX_UNLOCK();
#endif

	if(m_fb1_handle != 0)
	{	
		close(m_fb1_handle);	
		m_fb1_handle = 0;
	}

	m_vpo_opened[priv->idx] = 0;
	
	EXIT();
	
	return RET_SUCCESS;
}

RET_CODE vpo_win_onoff(struct vpo_device *dev,BOOL bOn)
{
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vpo_attached == 0)
		HLD_vpo_attach();
#endif	

	if(m_vpo_opened[priv->idx] <= 0)
	{
		VPO_PRF("vpo dev is don't opend\n");
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();	
	
	{
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;		
		rpc_pars.API_ID = RPC_FB_WIN_ON_OFF;
		rpc_pars.arg_num = 1;
		rpc_pars.arg[0].arg = (void *)&bOn;
		rpc_pars.arg[0].arg_size = sizeof(bOn);			
		ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;
	}

	priv->on = bOn;
	VPO_PRF("win on %d\n", bOn);	

	EXIT();
	
	MUTEX_UNLOCK();

	return ret;
}

RET_CODE vpo_win_mode(struct vpo_device *dev, BYTE bWinMode, struct MPSource_CallBack *pMPCallBack,struct PIPSource_CallBack *pPIPCallBack)
{
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif
	RET_CODE ret = RET_SUCCESS;
	UINT32 winmode = bWinMode;

#ifdef ADR_IPC_ENABLE
	if(m_vpo_attached == 0)
		HLD_vpo_attach();
#endif	

	if(m_vpo_opened[priv->idx] <= 0)
	{
		VPO_PRF("vpo dev is don't opend\n");
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();	
	
	{
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;		
		rpc_pars.API_ID = RPC_FB_WIN_MODE;
		rpc_pars.arg_num = 3;
		rpc_pars.arg[0].arg = (void *)&winmode;
		rpc_pars.arg[0].arg_size = sizeof(winmode);		
		rpc_pars.arg[1].arg = (void *)pMPCallBack;
		rpc_pars.arg[1].arg_size = sizeof(*pMPCallBack);	
		rpc_pars.arg[2].arg = (void *)pPIPCallBack;
		rpc_pars.arg[2].arg_size = sizeof(*pPIPCallBack);			
		ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;
	}

	VPO_PRF("win mode setting\n");

	EXIT();

	MUTEX_UNLOCK();

	return ret;
}

RET_CODE vpo_zoom(struct vpo_device *dev, struct Rect *pSrcRect , struct Rect *pDstRect)
{
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vpo_attached == 0)
		HLD_vpo_attach();
#endif	

	if(m_vpo_opened[priv->idx] <= 0)
	{
		VPO_PRF("vpo dev is don't opend\n");
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();	

	ENTRY();
	
	priv->src_rect = *pSrcRect;
	priv->dst_rect = *pDstRect;

	VPO_PRF("src x %d y %d w %d h %d \n", pSrcRect->uStartX, pSrcRect->uStartY
		, pSrcRect->uWidth, pSrcRect->uHeight);
	VPO_PRF("dst x %d y %d w %d h %d \n", pDstRect->uStartX, pDstRect->uStartY
		, pDstRect->uWidth, pDstRect->uHeight);	
	
	{
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_ZOOM;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)pSrcRect;
		rpc_pars.arg[0].arg_size = sizeof(*pSrcRect);		
		rpc_pars.arg[1].arg = (void *)pDstRect;
		rpc_pars.arg[1].arg_size = sizeof(*pDstRect);			
		ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;
	}
	
	EXIT();	
	
	MUTEX_UNLOCK();	
	
	return ret;
}

RET_CODE vpo_aspect_mode(struct vpo_device *dev, enum TVMode eTVAspect, enum DisplayMode e169DisplayMode)
{
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif	
	RET_CODE ret = RET_SUCCESS;	

#ifdef ADR_IPC_ENABLE
	if(m_vpo_attached == 0)
		HLD_vpo_attach();
#endif	

	if(m_vpo_opened[priv->idx] <= 0)
	{
		VPO_PRF("vpo dev is don't opend\n");
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();	

	ENTRY();	
	
	priv->aspect_mode = eTVAspect;
	priv->display_mode = e169DisplayMode;

	{
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_ASPECT_MODE;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)&eTVAspect;
		rpc_pars.arg[0].arg_size = sizeof(eTVAspect);		
		rpc_pars.arg[1].arg = (void *)&e169DisplayMode;
		rpc_pars.arg[1].arg_size = sizeof(e169DisplayMode);			
		ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;
	}

	EXIT();
	
	MUTEX_UNLOCK();		
	
	return ret;
}

RET_CODE vpo_tvsys(struct vpo_device *dev, enum TVSystem eTVSys)
{
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vpo_attached == 0)
		HLD_vpo_attach();
#endif	

	if(m_vpo_opened[priv->idx] <= 0)
	{
		VPO_PRF("vpo dev is don't opend\n");
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();	
	
	ENTRY();	
	
	{
		priv->progressive = 0;
		priv->tv_sys = eTVSys;

		{
			struct ali_fb_rpc_pars rpc_pars;			

			memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
			rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
			rpc_pars.API_ID = RPC_FB_TVSYS;
			rpc_pars.arg_num = 1;
			rpc_pars.arg[0].arg = (void *)&eTVSys;
			rpc_pars.arg[0].arg_size = sizeof(eTVSys);				
			ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
			if(ret < 0)
				ret = RET_FAILURE;
		}
	}

	EXIT();
	
	MUTEX_UNLOCK();	
	
	return ret;
}

RET_CODE vpo_tvsys_ex(struct vpo_device *dev, enum TVSystem eTVSys, BOOL bProgressive)
{
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vpo_attached == 0)
		HLD_vpo_attach();
#endif	

	if(m_vpo_opened[priv->idx] <= 0)
	{
		VPO_PRF("vpo dev is don't opend\n");
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();	
	
	ENTRY();	
	
	{
		priv->progressive = bProgressive;
		priv->tv_sys = eTVSys;
		
		{
			struct ali_fb_rpc_pars rpc_pars;			

			memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
			rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
			rpc_pars.API_ID = RPC_FB_TVSYS_EX;
			rpc_pars.arg_num = 2;
			rpc_pars.arg[0].arg = (void *)&eTVSys;
			rpc_pars.arg[0].arg_size = sizeof(eTVSys);		
			rpc_pars.arg[1].arg = (void *)&bProgressive;
			rpc_pars.arg[1].arg_size = sizeof(bProgressive);			
			ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
			if(ret < 0)
				ret = RET_FAILURE;
		}
	}
	
	EXIT();
	
	MUTEX_UNLOCK();		

	return ret;
}

RET_CODE vpo_ioctl(struct vpo_device *dev, UINT32 dwCmd, UINT32 dwParam)
{
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif
	struct ali_fb_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;	

#ifdef ADR_IPC_ENABLE
	if(m_vpo_attached == 0)
		HLD_vpo_attach();
#endif	

	if(m_vpo_opened[priv->idx] <= 0)
	{
		VPO_PRF("vpo dev is don't opend\n");
		return RET_FAILURE;
	}
	
	MUTEX_LOCK();
	
	ENTRY();	

	VPO_PRF("io cmd %x param %x\n", dwCmd, dwParam);
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
	rpc_pars.API_ID = RPC_FB_IOCTL;
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)&dwCmd;
	rpc_pars.arg[0].arg_size = sizeof(dwCmd);		
	switch(dwCmd)
	{
		case VPO_IO_SET_BG_COLOR:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct  YCbCrColor);			
			break;
		}
		case VPO_IO_WRITE_TTX:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct  vpo_io_ttx);
			break;
		}
		case VPO_IO_REG_DAC:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct vp_io_reg_dac_para);			
			break;
		}		
		case VPO_IO_SET_PARAMETER:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct vpo_io_set_parameter);						
			break;
		}
		case VPO_IO_VIDEO_ENHANCE:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct  vpo_io_video_enhance);					
			break;			
		}
		case VPO_IO_GET_OUT_MODE:
		{
			*(enum TVSystem *)(dwParam) = priv->tv_sys;
			
			rpc_pars.arg_num = 0;

			goto OUT;
		}			
        	case VPO_IO_GET_INFO:		
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct vpo_io_get_info);					
			rpc_pars.arg[1].out = 1;
			break;	
		}
		case VPO_IO_SET_CGMS_INFO:
        	{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct vpo_io_cgms_info);					
			break;	
        	}
	        case VPO_IO_AFD_CONFIG:
	        {
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct vp_io_afd_para);					
			break;	
	        }
		case VPO_IO_GET_CURRENT_DISPLAY_INFO:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct vpo_io_get_picture_info);
			break;
		}
		case VPO_IO_BACKUP_CURRENT_PICTURE:
        	{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct vpo_io_get_picture_info);
            		rpc_pars.arg[1].out = 1;
            		break;
        	}
        	case VPO_IO_FREE_BACKUP_PICTURE:
        	{
			//rpc_pars.arg[1].arg = (void *)dwParam;				
			//rpc_pars.arg[1].arg_size = sizeof(struct vpo_io_get_picture_info);
            		//rpc_pars.arg[1].out = 1;
            		break;
        	}
        	case VPO_IO_SET_OSD_SHOW_TIME:
        	{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(vpo_osd_show_time_t);
            		break;
        	}
        	case VPO_IO_GET_MP_SCREEN_RECT:
        	{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct Rect);
            		rpc_pars.arg[1].out = 1;
            		break;
        	}
        	case VPO_IO_GET_REAL_DISPLAY_MODE:	
		case VPO_IO_GET_TV_ASPECT:				
		case VPO_IO_GET_SRC_ASPECT:			
		case VPO_IO_GET_BEST_DESUB:
		case VPO_IO_GET_DISPLAY_MODE:
        	case VPO_IO_GET_OSD0_SHOW_TIME:
        	case VPO_IO_GET_OSD1_SHOW_TIME:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = 4;	
			rpc_pars.arg[1].out = 1;			
			break;
		}
		case VPO_IO_SET_VBI_OUT:
		case VPO_IO_WRITE_WSS:
		case VPO_IO_UNREG_DAC:
		case VPO_IO_MHEG_SCENE_AR:			
		case VPO_IO_MHEG_IFRAME_NOTIFY:   	
		case VPO_IO_DISAUTO_WIN_ONOFF:   	
		case VPO_IO_ENABLE_VBI:			
		case VPO_IO_PLAYMODE_CHANGE:
        	case VPO_IO_DIT_CHANGE:			
		case VPO_IO_SWAFD_ENABLE:			
	       case VPO_IO_704_OUTPUT:
		case VPO_IO_SET_PREVIEW_MODE:
		case VPO_IO_HDMI_OUT_PIC_FMT:
		case VPO_IO_SET_LAYER_ORDER:
		case VPO_IO_SET_VBI_STARTLINE:
		case VPO_IO_SET_DISPLAY_STYLE:
		case VPO_IO_ALWAYS_OPEN_CGMS_INFO:
        	case VPO_IO_TVESDHD_SOURCE_SEL:
        	case VPO_IO_SD_CC_ENABLE:
        	case VPO_IO_SET_PREVIEW_SAR_MODE: 
        	case VPO_FULL_SCREEN_CVBS: 
		{
			rpc_pars.arg[1].arg = (void *)&dwParam;					
			rpc_pars.arg[1].arg_size = 4;					
			break;
		}
		case VPO_IO_CLOSE_HW_OUTPUT:
		{
			rpc_pars.API_ID = RPC_FB_CLOSE;
			rpc_pars.arg_num = 0;			
			break;
		}
		case VPO_IO_SET_CUTLINE:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct  vpo_io_cutline_pars);			
			break;
		}
		case VPO_IO_DISPLAY_3D_ENABLE:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct  alifbio_3d_pars);			
			break;
		}
		case VPO_IO_SET_3D_ENH:
		{
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct  alifbio_3d_enhance_pars);			
			break;
		}
		case VPO_IO_CLEAN_CURRENT_STILL_PICTURE:
		{
			rpc_pars.arg_num = 1;	
			break;
		}
		default:			
		{
			ret = RET_FAILURE;
			VPO_PRF("don't supported vpo io cmd %d\n", dwCmd);
			// don't supported. 
			// the related function has been moved into the kernel
			// or deleted
			break;
		}
	}
	
	if(ret != RET_FAILURE)
	{
		ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;	
	}

OUT:	
	EXIT();
	
	MUTEX_UNLOCK();		
	
	return ret;
}

#if 0
RET_CODE vpo_config_source_window(struct vpo_device *dev, struct vp_win_config_para *pwin_para)
{
	struct vpo_private *priv = (struct vpo_private *)dev->priv;		
	RET_CODE ret = RET_SUCCESS;

	MUTEX_LOCK();	
	
	ENTRY();	
	
	{
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_CONFIG_SOURCE_WINDOW;
		rpc_pars.arg_num = 1;
		rpc_pars.arg[0].arg = (void *)pwin_para;
		rpc_pars.arg[0].arg_size = sizeof(struct vp_win_config_para);				
		ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;
	}

	EXIT();
	
	MUTEX_UNLOCK();		
	
	return ret;
}
#endif

RET_CODE vpo_set_progres_interl(struct vpo_device *dev, BOOL bProgressive)
{
#ifdef ADR_IPC_ENABLE
	struct vpo_private *priv = m_vpo_priv[(int)dev->priv];
#else
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
#endif
	RET_CODE ret = RET_SUCCESS;
		
	MUTEX_LOCK();	
	
	ENTRY();	
	
	{
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_CONFIG_SOURCE_WINDOW;
		rpc_pars.arg_num = 1;
		rpc_pars.arg[0].arg = (void *)&bProgressive;
		rpc_pars.arg[0].arg_size = sizeof(bProgressive);				
		ret = ioctl(m_fb1_handle, FBIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;
	}

	EXIT();
	
	MUTEX_UNLOCK();		
	
	return ret;
}


void HLD_vpo_attach(void)
{
	struct vpo_device *vpo_dev;
	struct vpo_private *priv;
	int i = 0;
#ifdef ADR_IPC_ENABLE	
	int shmid = 0;
#endif

	if(m_vpo_attached > 0)
		return;

	ENTRY();
	
#ifdef ADR_IPC_ENABLE
	m_vpo_mutex_id = adr_ipc_semget(ADR_IPC_VPO, IPC_VPO_SEM_KEY_IDX, 1);
	if(m_vpo_mutex_id < 0)
	{
		VPO_PRF("vpo create mutex fail\n");
		return;
	}
#else
	m_vpo_mutex_id = osal_mutex_create();
	if(m_vpo_mutex_id == OSAL_INVALID_ID)
	{
		VPO_PRF("vpo create mutex fail\n");
		return;
	}
#endif

	for(i = 0;i < MAX_VPO_NUM;i++)
	{
		vpo_dev = dev_alloc(i == 0 ? m_vpo_name_hd : m_vpo_name_sd
			,HLD_DEV_TYPE_DIS,sizeof(struct vpo_device));
		if(vpo_dev == NULL)
		{
			VPO_PRF("malloc vpo dev fail\n");
			return;
		}

#ifdef ADR_IPC_ENABLE
		MUTEX_LOCK();

		if(shmid = adr_ipc_shmget(ADR_IPC_VPO, IPC_VPO_PRIV_KEY_IDX + i
			, (void **)&priv, sizeof(*priv)) < 0)
		{
			MUTEX_UNLOCK();
					
			VPO_PRF("get vpo shm fail\n");
			return;			
		}

		VPO_PRF("get the vpo shmd %d\n", shmid);
		
		if(priv->inited == 0)
		{
			memset((void *)priv, 0, sizeof(*priv));
			// priv->file_path = m_fb1_path;
			priv->hd = i == 0 ? 1 : 0;
			priv->idx = i;
			priv->shmid = shmid;
#ifdef HLD_DBG_ENABLE
			priv->dbg_on = 1;// just for debug
#else
			priv->dbg_on = 0;// just for debug
#endif
			priv->inited = 1;

			vpo_dev->priv = (void *)i;

			VPO_PRF("init vpo shm\n");			
		}

		m_vpo_priv[i] = priv;
		p_vpo_dbg_on = &(priv->dbg_on);

		MUTEX_UNLOCK();
#else
		priv = malloc(sizeof(*priv));
		if(priv == NULL)
		{
			VPO_PRF("malloc osd priv fail\n");
		}
		
		memset((void *)priv, 0, sizeof(*priv));
		priv->file_path = m_fb1_path;
		priv->hd = i == 0 ? 1 : 0;
		priv->idx = i;

		vpo_dev->priv = (void *)priv;
		vpo_dev->next = NULL;		
#endif
		
		if(dev_register(vpo_dev) != RET_SUCCESS)
		{
			VPO_PRF("register vpo dev fail\n");
			return;
		}
		
		m_vpo_dev[i] = vpo_dev;
		m_vpo_opened[i] = 0;

	}

	ioctl(m_fb1_handle, FBIO_SET_DE_LAYER, DE_LAYER0);

	VPO_PRF("vpo create the mutex %d done\n", m_vpo_mutex_id);

	disdbg_register();
	
	m_vpo_attached = 1;
	VPO_PRF("attach hld vpo ok\n");	
	EXIT();

#if 0
	vpo_open(m_vpo_dev[0], NULL);
#else
	for(i = 0;i < MAX_VPO_NUM;i++)	
		vpo_open(m_vpo_dev[i], NULL);
#endif	
}

void HLD_vpo_dettach(void)
{
	int i = 0;

	ENTRY();
	
	if(m_vpo_attached != 0)
	{
		for(i = 0;i < MAX_VPO_NUM;i++)
		{
			vpo_close(m_vpo_dev[i]);
			m_vpo_opened[i] = 0;
#ifdef ADR_IPC_ENABLE

#else
			if(m_vpo_dev[i]->priv != NULL)
				free(m_vpo_dev[i]->priv);
#endif			
			dev_free(m_vpo_dev[i]);
			m_vpo_dev[i] = NULL;
		}

#ifdef ADR_IPC_ENABLE

#else
		osal_mutex_delete(m_vpo_mutex_id);
#endif

		m_vpo_attached = 0;		
	}

	EXIT();
}

