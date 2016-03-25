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
 *  2.  2010.03.11  Sam			4.0		Support Linux Driver
 ****************************************************************************/

#include <osal/osal.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <sys_config.h>
#include <mediatypes.h>
#include <hld/dis/vpo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
//corei#include <err/errno.h>

#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>

#if 1
#define VPO_PRF(...)	do{}while(0)
#else
#define VPO_PRF printf
#endif

#define MUTEX_LOCK	osal_mutex_lock(m_vpo_mutex_id, OSAL_WAIT_FOREVER_TIME)
#define MUTEX_UNLOCK osal_mutex_unlock(m_vpo_mutex_id)

#define ENTRY		VPO_PRF("%s : in\n", __FUNCTION__)
#define EXIT			VPO_PRF("%s : out\n", __FUNCTION__)

#define MAX_VPO_NUM				2

struct vpo_private
{
	int handle;
	char *file_path;

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
};

static char m_vpo_name_sd[] = {"VPO_S3602F_SD"};
static char m_vpo_name_hd[] = {"VPO_S3602F_HD"};
static struct vpo_device *m_vpo_dev[MAX_VPO_NUM];
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
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
	RET_CODE ret = RET_SUCCESS;
	
	ENTRY;
	
	if(m_fb1_handle == 0)
	{
		m_fb1_handle = open(priv->file_path, O_RDWR);
		if(m_fb1_handle <= 0)
		{
			VPO_PRF("%s : open file fail %s\n", __FUNCTION__, priv->file_path);
			return RET_FAILURE;
		}
	}
	
	priv->handle = m_fb1_handle;

	{
		#ifdef USE_OLD_RPC_FB_IO
		struct ali_fb_rpc_pars rpc_pars;			
		UINT32 io_cmd = VPO_IO_GET_OUT_MODE;

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_IOCTL;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)&io_cmd;
		rpc_pars.arg[0].arg_size = sizeof(io_cmd);	
		rpc_pars.arg[1].arg = (void *)(&(priv->tv_sys));
		rpc_pars.arg[1].arg_size = sizeof(priv->tv_sys);		
		rpc_pars.arg[1].out = 1;
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else
        struct ali_vpo_ioctrl_pars ioctrl_pars;

        memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
        ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
        ioctrl_pars.param = (UINT32)priv->tv_sys;
        
        ret = ioctl(priv->handle, VPO_GET_OUT_MODE, &ioctrl_pars);

        priv->tv_sys = (enum TVSystem)ioctrl_pars.param;
        #endif
		if(ret < 0)
			ret = RET_FAILURE;
	}

	VPO_PRF("vpo hd %d default tv %d\n", priv->hd, priv->tv_sys);
	
	EXIT;
	
	return RET_SUCCESS;
}

RET_CODE vpo_close(struct vpo_device *dev)
{	
	struct vpo_private *priv = (struct vpo_private *)dev->priv;

	ENTRY;
	
	if(m_fb1_handle != 0)
	{
		close(m_fb1_handle);	
		m_fb1_handle = 0;
	}
	
	EXIT;
	
	return RET_SUCCESS;
}

RET_CODE vpo_win_onoff(struct vpo_device *dev,BOOL bOn)
{
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
	RET_CODE ret = RET_SUCCESS;
		
	MUTEX_LOCK;
	
	ENTRY;	
	
	{
		#ifdef USE_OLD_RPC_FB_IO
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;		
		rpc_pars.API_ID = RPC_FB_WIN_ON_OFF;
		rpc_pars.arg_num = 1;
		rpc_pars.arg[0].arg = (void *)&bOn;
		rpc_pars.arg[0].arg_size = sizeof(bOn);			
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else
        struct ali_vpo_win_status_pars win_status_pars;

        memset((void *)&win_status_pars, 0, sizeof(win_status_pars));
		win_status_pars.hd_dev = (priv->hd == 1) ? 1 : 0;	
        win_status_pars.on = bOn;
        win_status_pars.layer = VPO_LAYER_MAIN;
        ret = ioctl(priv->handle, VPO_SET_WIN_ONOFF, &win_status_pars);
		#endif
		if(ret < 0)
			ret = RET_FAILURE;
	}

	priv->on = bOn;
	VPO_PRF("win on %d\n", bOn);	

	EXIT;
	
	MUTEX_UNLOCK;

	return ret;
}

RET_CODE vpo_win_onoff_ext(struct vpo_device *dev,BOOL bOn,enum vp_display_layer layer)
{
    struct vpo_private *priv = (struct vpo_private *)dev->priv;
	RET_CODE ret = RET_SUCCESS;
		
	MUTEX_LOCK;
	
	ENTRY;	
	
	{
		#ifdef USE_OLD_RPC_FB_IO
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;		
		rpc_pars.API_ID = RPC_FB_WIN_ON_OFF_EX;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)&bOn;
		rpc_pars.arg[0].arg_size = sizeof(bOn);	
        rpc_pars.arg[1].arg = (void *)&layer;
		rpc_pars.arg[1].arg_size = sizeof(layer);	
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else
        struct ali_vpo_win_status_pars win_status_pars;

        memset((void *)&win_status_pars, 0, sizeof(win_status_pars));
		win_status_pars.hd_dev = (priv->hd == 1) ? 1 : 0;	
        win_status_pars.on = bOn;
        win_status_pars.layer = layer;
        ret = ioctl(priv->handle, VPO_SET_WIN_ONOFF_EX, &win_status_pars);
		#endif
		if(ret < 0)
			ret = RET_FAILURE;
	}

	priv->on = bOn;
	VPO_PRF("win on %d\n", bOn);	

	EXIT;
	
	MUTEX_UNLOCK;

	return ret;
}

RET_CODE vpo_win_mode(struct vpo_device *dev, BYTE bWinMode, struct MPSource_CallBack *pMPCallBack,struct PIPSource_CallBack *pPIPCallBack)
{
	struct vpo_private *priv = (struct vpo_private *)dev->priv;
	RET_CODE ret = RET_SUCCESS;
	UINT32 winmode = bWinMode;
		
	MUTEX_LOCK;
	
	ENTRY;	
	
	{
		#ifdef USE_OLD_RPC_FB_IO
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
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else
        struct ali_vpo_winmode_pars win_mode_pars;

        memset((void *)&win_mode_pars, 0, sizeof(win_mode_pars));
		win_mode_pars.hd_dev = (priv->hd == 1) ? 1 : 0;	
        win_mode_pars.win_mode = winmode;
        win_mode_pars.mp_callback = *pMPCallBack;
        win_mode_pars.pip_callback = *pPIPCallBack;
        ret = ioctl(priv->handle, VPO_SET_WIN_MODE, &win_mode_pars);
		#endif
		if(ret < 0)
			ret = RET_FAILURE;
	}

	VPO_PRF("win mode setting\n");

	EXIT;

	MUTEX_UNLOCK;

	return ret;
}

RET_CODE vpo_zoom(struct vpo_device *dev, struct Rect *pSrcRect , struct Rect *pDstRect)
{
	struct vpo_private *priv = (struct vpo_private *)dev->priv;	
	RET_CODE ret = RET_SUCCESS;
		
	MUTEX_LOCK;	

	ENTRY;
	
	priv->src_rect = *pSrcRect;
	priv->dst_rect = *pDstRect;

	VPO_PRF("src x %d y %d w %d h %d \n", pSrcRect->uStartX, pSrcRect->uStartY
		, pSrcRect->uWidth, pSrcRect->uHeight);
	VPO_PRF("dst x %d y %d w %d h %d \n", pDstRect->uStartX, pDstRect->uStartY
		, pDstRect->uWidth, pDstRect->uHeight);	
	
	{
		#ifdef USE_OLD_RPC_FB_IO
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_ZOOM;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)pSrcRect;
		rpc_pars.arg[0].arg_size = sizeof(*pSrcRect);		
		rpc_pars.arg[1].arg = (void *)pDstRect;
		rpc_pars.arg[1].arg_size = sizeof(*pDstRect);			
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else
        struct ali_vpo_zoom_pars zoom_pars;

        memset((void *)&zoom_pars, 0, sizeof(zoom_pars));
		zoom_pars.hd_dev = (priv->hd == 1) ? 1 : 0;	
        zoom_pars.src_rect = *pSrcRect;
        zoom_pars.dst_rect = *pDstRect;
        zoom_pars.layer = VPO_LAYER_MAIN;
        ret = ioctl(priv->handle, VPO_WIN_ZOOM, &zoom_pars);
		#endif
		if(ret < 0)
			ret = RET_FAILURE;
	}
	
	EXIT;	
	
	MUTEX_UNLOCK;	
	
	return ret;
}
RET_CODE vpo_zoom_ext(struct vpo_device *dev, struct Rect *pSrcRect , struct Rect *pDstRect,enum vp_display_layer layer)
{
    struct vpo_private *priv = (struct vpo_private *)dev->priv;	
	RET_CODE ret = RET_SUCCESS;
		
	MUTEX_LOCK;	

	ENTRY;
	
	priv->src_rect = *pSrcRect;
	priv->dst_rect = *pDstRect;

	VPO_PRF("src x %d y %d w %d h %d \n", pSrcRect->uStartX, pSrcRect->uStartY
		, pSrcRect->uWidth, pSrcRect->uHeight);
	VPO_PRF("dst x %d y %d w %d h %d \n", pDstRect->uStartX, pDstRect->uStartY
		, pDstRect->uWidth, pDstRect->uHeight);	
	
	{
		#ifdef USE_OLD_RPC_FB_IO
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_ZOOM_EX;
		rpc_pars.arg_num = 3;
		rpc_pars.arg[0].arg = (void *)pSrcRect;
		rpc_pars.arg[0].arg_size = sizeof(*pSrcRect);		
		rpc_pars.arg[1].arg = (void *)pDstRect;
		rpc_pars.arg[1].arg_size = sizeof(*pDstRect);
        rpc_pars.arg[2].arg = (void *)&layer;
		rpc_pars.arg[2].arg_size = sizeof(layer);		
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else
        struct ali_vpo_zoom_pars zoom_pars;

        memset((void *)&zoom_pars, 0, sizeof(zoom_pars));
		zoom_pars.hd_dev = (priv->hd == 1) ? 1 : 0;	
        zoom_pars.src_rect = *pSrcRect;
        zoom_pars.dst_rect = *pDstRect;
        zoom_pars.layer = layer;
        ret = ioctl(priv->handle, VPO_WIN_ZOOM_EX, &zoom_pars);
		#endif
		if(ret < 0)
			ret = RET_FAILURE;
	}
	
	EXIT;	
	
	MUTEX_UNLOCK;	
	
	return ret;
}

RET_CODE vpo_aspect_mode(struct vpo_device *dev, enum TVMode eTVAspect, enum DisplayMode e169DisplayMode)
{
	struct vpo_private *priv = (struct vpo_private *)dev->priv;		
	RET_CODE ret = RET_SUCCESS;	

	MUTEX_LOCK;	

	ENTRY;	
	
	priv->aspect_mode = eTVAspect;
	priv->display_mode = e169DisplayMode;

	{	
		#ifdef USE_OLD_RPC_FB_IO
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_ASPECT_MODE;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)&eTVAspect;
		rpc_pars.arg[0].arg_size = sizeof(eTVAspect);		
		rpc_pars.arg[1].arg = (void *)&e169DisplayMode;
		rpc_pars.arg[1].arg_size = sizeof(e169DisplayMode);			
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else		
        struct ali_vpo_aspect_pars aspect_par;
		memset((void *)&aspect_par, 0, sizeof(aspect_par));
		aspect_par.hd_dev = (priv->hd == 1) ? 1 : 0;				
		aspect_par.aspect = eTVAspect;
        aspect_par.display_mode = e169DisplayMode;
        ret = ioctl(priv->handle, VPO_SET_ASPECT_MODE, &aspect_par);
		#endif
    	if(ret < 0)
    		ret = RET_FAILURE;
	}

	EXIT;
	
	MUTEX_UNLOCK;		
	
	return ret;
}

RET_CODE vpo_tvsys(struct vpo_device *dev, enum TVSystem eTVSys)
{
	struct vpo_private *priv = (struct vpo_private *)dev->priv;		
	RET_CODE ret = RET_SUCCESS;
		
	MUTEX_LOCK;	
	
	ENTRY;	
	
	priv->progressive = 0;
	priv->tv_sys = eTVSys;

	{
		#ifdef USE_OLD_RPC_FB_IO
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_TVSYS;
		rpc_pars.arg_num = 1;
		rpc_pars.arg[0].arg = (void *)&eTVSys;
		rpc_pars.arg[0].arg_size = sizeof(eTVSys);				
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else
        struct ali_vpo_tvsys_pars tvsys_pars;
        memset((void *)&tvsys_pars, 0, sizeof(tvsys_pars));
		tvsys_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
        tvsys_pars.progressive = 0;
        tvsys_pars.tvsys = eTVSys;
        ret = ioctl(priv->handle, VPO_SET_TVSYS, &tvsys_pars);
		#endif
		if(ret < 0)
			ret = RET_FAILURE;
	}

	EXIT;
	
	MUTEX_UNLOCK;	
	
	return ret;
}

RET_CODE vpo_tvsys_ex(struct vpo_device *dev, enum TVSystem eTVSys, BOOL bProgressive)
{
	struct vpo_private *priv = (struct vpo_private *)dev->priv;		
	RET_CODE ret = RET_SUCCESS;

	MUTEX_LOCK;	
	
	ENTRY;	
	
	priv->progressive = bProgressive;
	priv->tv_sys = eTVSys;
	
	{
		#ifdef USE_OLD_RPC_FB_IO
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_TVSYS_EX;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)&eTVSys;
		rpc_pars.arg[0].arg_size = sizeof(eTVSys);		
		rpc_pars.arg[1].arg = (void *)&bProgressive;
		rpc_pars.arg[1].arg_size = sizeof(bProgressive);			
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else
        struct ali_vpo_tvsys_pars tvsys_pars;
        memset((void *)&tvsys_pars, 0, sizeof(tvsys_pars));
		tvsys_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
        tvsys_pars.progressive = bProgressive;
        tvsys_pars.tvsys = eTVSys;
        ret = ioctl(priv->handle, VPO_SET_TVSYS_EX, &tvsys_pars);
		#endif
		if(ret < 0)
			ret = RET_FAILURE;
	}
	
	EXIT;
	
	MUTEX_UNLOCK;		

	return ret;
}

RET_CODE vpo_ioctl(struct vpo_device *dev, UINT32 dwCmd, UINT32 dwParam)
{
	if ( !m_vpo_attached )
	{
		return RET_FAILURE;
	}
	struct vpo_private *priv = (struct vpo_private *)dev->priv;		
	struct ali_fb_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;	

	MUTEX_LOCK;
	
	ENTRY;
    
    #ifdef USE_OLD_RPC_FB_IO
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
			break;
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
			rpc_pars.arg[1].out = 1;
            break;
        }
        case VPO_IO_BACKUP_CURRENT_PICTURE:
        {
			rpc_pars.arg[1].arg = (void *)dwParam;				
			rpc_pars.arg[1].arg_size = sizeof(struct vpo_io_get_picture_info);
            rpc_pars.arg[1].out = 1;
            printf("------%s------VPO_IO_BACKUP_CURRENT_PICTURE---\n", __FUNCTION__);
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

        case VPO_IO_ALWAYS_OPEN_CGMS_INFO:
        case VPO_IO_SET_LAYER_ORDER:
        case VPO_IO_TVESDHD_SOURCE_SEL:
        case VPO_IO_SD_CC_ENABLE:
        case VPO_IO_SET_PREVIEW_SAR_MODE: 
        case VPO_FULL_SCREEN_CVBS: 
		{
			rpc_pars.arg[1].arg = (void *)&dwParam;					
			rpc_pars.arg[1].arg_size = 4;					
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
	
	if((rpc_pars.arg_num != 0) && (ret != RET_FAILURE))
	{
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;	
	}
	#else
	switch(dwCmd)
	{
		case VPO_IO_SET_BG_COLOR:
		{
			struct ali_vpo_bgcolor_pars bg_color_pars;
            struct YCbCrColor *pcolor = (struct YCbCrColor *)dwParam;
        
            memset((void *)&bg_color_pars, 0, sizeof(bg_color_pars));
            bg_color_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            bg_color_pars.yuv_color.uCb = pcolor->uCb;
            bg_color_pars.yuv_color.uCr = pcolor->uCr;
            bg_color_pars.yuv_color.uY = pcolor->uY;

            ret = ioctl(priv->handle, VPO_SET_BG_COLOR, &bg_color_pars);
			break;
		}
		case VPO_IO_REG_DAC:
		{
            struct ali_vpo_dac_pars dac_pars;
            struct vp_io_reg_dac_para *pdac_para = (struct vp_io_reg_dac_para *)dwParam;
            
            memset((void *)&dac_pars, 0, sizeof(dac_pars));
            dac_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            dac_pars.dac_param.eDacType = pdac_para->eDacType;
            dac_pars.dac_param.DacInfo = pdac_para->DacInfo;
            ret = ioctl(priv->handle, VPO_REG_DAC, &dac_pars);
			break;
		}		
		case VPO_IO_SET_PARAMETER:
		{
            struct ali_vpo_parameter_pars parameter_pars;
            struct vpo_io_set_parameter *para = (struct vpo_io_set_parameter *)dwParam;
            
            memset((void *)&parameter_pars, 0, sizeof(parameter_pars));
            parameter_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            parameter_pars.param.changed_flag = para->changed_flag;
            parameter_pars.param.fetch_mode_api_en = para->fetch_mode_api_en;
            parameter_pars.param.fetch_mode = para->fetch_mode;
            parameter_pars.param.dit_enable = para->dit_enable;
            parameter_pars.param.vt_enable = para->vt_enable;
            parameter_pars.param.vertical_2tap = para->vertical_2tap;
            parameter_pars.param.edge_preserve_enable = para->edge_preserve_enable;

            ret = ioctl(priv->handle, VPO_SET_PARAMETER, &parameter_pars);
			break;
		}
		case VPO_IO_VIDEO_ENHANCE:
		{
            struct ali_vpo_video_enhance_pars video_enhance_pars;
            struct vpo_io_video_enhance *para = (struct vpo_io_video_enhance *)dwParam;
            
            memset((void *)&video_enhance_pars, 0, sizeof(video_enhance_pars));
            video_enhance_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            video_enhance_pars.video_enhance_param.changed_flag = para->changed_flag;
            video_enhance_pars.video_enhance_param.grade = para->grade;

            ret = ioctl(priv->handle, VPO_VIDEO_ENHANCE, &video_enhance_pars);
			break;		
		}
		case VPO_IO_GET_OUT_MODE:
		{
            struct ali_vpo_ioctrl_pars ioctrl_pars;
            enum TVSystem *para = (enum TVSystem *)dwParam;
            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = (UINT32)*para;
            
            ret = ioctl(priv->handle, VPO_GET_OUT_MODE, &ioctrl_pars);

            *para = (enum TVSystem)ioctrl_pars.param;
            
            break; 
		}			
        case VPO_IO_GET_INFO:		
		{
            struct ali_vpo_mp_info_pars mp_info_pars;
            struct vpo_io_get_info *para = (struct vpo_io_get_info *)dwParam;

            memset((void *)&mp_info_pars, 0, sizeof(mp_info_pars));
            mp_info_pars.hd_dev = (priv->hd == 1) ? 1 : 0;

            ret = ioctl(priv->handle, VPO_GET_MP_INFO, &mp_info_pars);
            
			memcpy((void *)para, &mp_info_pars.mp_info, sizeof(struct vpo_io_get_info));  
			break;	
		}
        case VPO_IO_SET_CGMS_INFO:
        {
            struct ali_vpo_cgms_info_pars cgms_info_pars;
            struct vpo_io_cgms_info *para = (struct vpo_io_cgms_info *)dwParam;
            
            memset((void *)&cgms_info_pars, 0, sizeof(cgms_info_pars));
            cgms_info_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            cgms_info_pars.cgms_info.aps = para->aps;
            cgms_info_pars.cgms_info.cgms = para->cgms;

            ret = ioctl(priv->handle, VPO_SET_CGMS_INFO, &cgms_info_pars);			
			break;	
        }
        case VPO_IO_AFD_CONFIG:
        {
            struct ali_vpo_afd_pars afd_pars;
            struct vp_io_afd_para *para = (struct vp_io_afd_para *)dwParam;
            
            memset((void *)&afd_pars, 0, sizeof(afd_pars));
            afd_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            afd_pars.afd_param.bSwscaleAfd = para->bSwscaleAfd;
            afd_pars.afd_param.afd_solution = para->afd_solution;
            afd_pars.afd_param.protect_mode_enable = para->protect_mode_enable;

            ret = ioctl(priv->handle, VPO_AFD_CONFIG, &afd_pars);			
			break;		
        }
        case VPO_IO_GET_CURRENT_DISPLAY_INFO:
        {
            struct ali_vpo_display_info_pars display_info_pars;
            struct vpo_io_get_picture_info *cur_display_info = (struct vpo_io_get_picture_info *)dwParam;

            memset((void *)&display_info_pars, 0, sizeof(display_info_pars));
            display_info_pars.hd_dev = (priv->hd == 1) ? 1 : 0;

            ret = ioctl(priv->handle, VPO_GET_CURRENT_DISPLAY_INFO, &display_info_pars);
            
			memcpy((void *)cur_display_info, &display_info_pars.display_info, sizeof(struct vpo_io_get_picture_info));            
            break;
        }
        case VPO_IO_BACKUP_CURRENT_PICTURE:
        {
            struct ali_vpo_display_info_pars display_info_pars;
            struct vpo_io_get_picture_info *cur_display_info = (struct vpo_io_get_picture_info *)dwParam;

            memset((void *)&display_info_pars, 0, sizeof(display_info_pars));
            display_info_pars.hd_dev = (priv->hd == 1) ? 1 : 0;

            ret = ioctl(priv->handle, VPO_BACKUP_CURRENT_PICTURE, &display_info_pars);
            break;
        }
        case VPO_IO_FREE_BACKUP_PICTURE:
        {
            ret = ioctl(priv->handle, VPO_FREE_BACKUP_PICTURE, NULL);
            break;
        }
        case VPO_IO_SET_OSD_SHOW_TIME:
        {
            struct ali_vpo_osd_show_time_pars osd_show_time_pars;
            vpo_osd_show_time_t *param = (vpo_osd_show_time_t *)dwParam;

            memset((void *)&osd_show_time_pars, 0, sizeof(osd_show_time_pars));
            osd_show_time_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            osd_show_time_pars.osd_show_time.layer_id = param->layer_id;
            osd_show_time_pars.osd_show_time.show_on_off = param->show_on_off;
            osd_show_time_pars.osd_show_time.time_in_ms = param->time_in_ms;
            osd_show_time_pars.osd_show_time.reserved0 = param->reserved0;
            osd_show_time_pars.osd_show_time.reserved1 = param->reserved1;
            ret = ioctl(priv->handle, VPO_SET_OSD_SHOW_TIME, &osd_show_time_pars);
            break;
        }
        case VPO_IO_GET_MP_SCREEN_RECT:
        {
            struct ali_vpo_screem_rect_pars screem_rect_pars;
            struct Rect *video_rect = (struct Rect *)dwParam;

            memset((void *)&screem_rect_pars, 0, sizeof(screem_rect_pars));
            screem_rect_pars.hd_dev = (priv->hd == 1) ? 1 : 0;

            ret = ioctl(priv->handle, VPO_GET_MP_SCREEN_RECT, &screem_rect_pars);
            
			memcpy((void *)video_rect, &screem_rect_pars.mp_screem_rect, sizeof(struct Rect));
            break;
        }
        case VPO_IO_GET_REAL_DISPLAY_MODE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;
            enum DisplayMode *para = (enum DisplayMode *)dwParam;
            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = (UINT32)*para;
            
            ret = ioctl(priv->handle, VPO_GET_REAL_DISPLAY_MODE, &ioctrl_pars);

            *para = (enum DisplayMode)ioctrl_pars.param;
            
            break; 
        }
		case VPO_IO_GET_TV_ASPECT:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;
            UINT32 *para = (UINT32 *)dwParam;
            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = *para;
            
            ret = ioctl(priv->handle, VPO_GET_TV_ASPECT, &ioctrl_pars);

            *para = ioctrl_pars.param;
            
            break; 
		}
		case VPO_IO_GET_SRC_ASPECT:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;
            enum TVMode *para = (enum TVMode *)dwParam;
            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = (UINT32)*para;
            
            ret = ioctl(priv->handle, VPO_GET_SRC_ASPECT, &ioctrl_pars);

            *para = (enum TVMode)ioctrl_pars.param;            
            break; 
		}
        case VPO_IO_GET_DISPLAY_MODE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;
            enum DisplayMode *para = (enum DisplayMode *)dwParam;
            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = (UINT32)*para;
            
            ret = ioctl(priv->handle, VPO_GET_DISPLAY_MODE, &ioctrl_pars);

            *para = (enum DisplayMode)ioctrl_pars.param;
            
            break; 
        }
        case VPO_IO_GET_OSD0_SHOW_TIME:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;
            UINT32 *para = (UINT32 *)dwParam;
            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = *para;
            
            ret = ioctl(priv->handle, VPO_GET_OSD0_SHOW_TIME, &ioctrl_pars);

            *para = ioctrl_pars.param;
            
            break; 
        }
        case VPO_IO_GET_OSD1_SHOW_TIME:
		{
			struct ali_vpo_ioctrl_pars ioctrl_pars;
            UINT32 *para = (UINT32 *)dwParam;
            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = *para;
            
            ret = ioctl(priv->handle, VPO_GET_OSD1_SHOW_TIME, &ioctrl_pars);

            *para = ioctrl_pars.param;
            
            break; 
		}
        case VPO_IO_SET_VBI_OUT:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_SET_VBI_OUT, &ioctrl_pars);
            break; 
        }
		case VPO_IO_WRITE_WSS:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_WRITE_WSS, &ioctrl_pars);
            break; 
		}
		case VPO_IO_UNREG_DAC:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_UNREG_DAC, &ioctrl_pars);
            break; 
		}
		case VPO_IO_MHEG_SCENE_AR:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_MHEG_SCENE_AR, &ioctrl_pars);
            break; 
		}
		case VPO_IO_MHEG_IFRAME_NOTIFY:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_MHEG_IFRAME_NOTIFY, &ioctrl_pars);
            break; 
		}
		case VPO_IO_DISAUTO_WIN_ONOFF: 
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_DISAUTO_WIN_ONOFF, &ioctrl_pars);
            break; 
		}
		case VPO_IO_ENABLE_VBI:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_ENABLE_VBI, &ioctrl_pars);
            break; 
		}
		case VPO_IO_PLAYMODE_CHANGE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_PLAYMODE_CHANGE, &ioctrl_pars);
            break; 
		}
        case VPO_IO_DIT_CHANGE:	
        {  
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_DIT_CHANGE, &ioctrl_pars);
            break; 
        }
		case VPO_IO_SWAFD_ENABLE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_SWAFD_ENABLE, &ioctrl_pars);
            break;
		}
	    case VPO_IO_704_OUTPUT:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_704_OUTPUT, &ioctrl_pars);
            break;
	    }
		case VPO_IO_SET_PREVIEW_MODE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_SET_PREVIEW_MODE, &ioctrl_pars);
            break;
		}
		case VPO_IO_HDMI_OUT_PIC_FMT:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_HDMI_OUT_PIC_FMT, &ioctrl_pars);
            break;
		}
        case VPO_IO_ALWAYS_OPEN_CGMS_INFO:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_ALWAYS_OPEN_CGMS_INFO, &ioctrl_pars);
            break;
        }
        case VPO_IO_SET_LAYER_ORDER:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            if(dwParam > 20)
            {
                dwParam = 0;
            }

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;

            
            
            ret = ioctl(priv->handle, VPO_SET_LAYER_ORDER, &ioctrl_pars);
            break;
        }
        case VPO_IO_TVESDHD_SOURCE_SEL:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_TVESDHD_SOURCE_SEL, &ioctrl_pars);
            break; 
        }
        case VPO_IO_SD_CC_ENABLE:
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_SD_CC_ENABLE, &ioctrl_pars);
            break; 
        }
        case VPO_IO_SET_PREVIEW_SAR_MODE: 
        {
            struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_SET_PREVIEW_SAR_MODE, &ioctrl_pars);
            break; 
        }
        case VPO_FULL_SCREEN_CVBS: 
		{
			struct ali_vpo_ioctrl_pars ioctrl_pars;

            memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
            ioctrl_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
            ioctrl_pars.param = dwParam;
            
            ret = ioctl(priv->handle, VPO_SET_FULL_SCREEN_CVBS, &ioctrl_pars);
            break; 
		}				
		default:			
		{
			ret = RET_FAILURE;
			VPO_PRF("don't supported vpo io cmd %d\n", dwCmd);
			break;
		}
	}

    if(ret < 0)	
    {
        ret = RET_FAILURE;
    }
    #endif
	EXIT;
	
	MUTEX_UNLOCK;		
	
	return ret;
}

RET_CODE vpo_config_source_window(struct vpo_device *dev, struct vp_win_config_para *pwin_para)
{
	struct vpo_private *priv = (struct vpo_private *)dev->priv;		
	RET_CODE ret = RET_SUCCESS;

	MUTEX_LOCK;	
	
	ENTRY;	
	
	{
		#ifdef USE_OLD_RPC_FB_IO
		struct ali_fb_rpc_pars rpc_pars;			

		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.hd_dev = (priv->hd == 1) ? 1 : 0;				
		rpc_pars.API_ID = RPC_FB_CONFIG_SOURCE_WINDOW;
		rpc_pars.arg_num = 1;
		rpc_pars.arg[0].arg = (void *)pwin_para;
		rpc_pars.arg[0].arg_size = sizeof(struct vp_win_config_para);				
		ret = ioctl(priv->handle, FBIO_RPC_OPERATION, &rpc_pars);
		#else
        struct ali_vpo_win_config_pars win_config_pars;

        memset((void *)&win_config_pars, 0, sizeof(win_config_pars));
		win_config_pars.hd_dev = (priv->hd == 1) ? 1 : 0;
        win_config_pars.win_config_para = *pwin_para;
        ret = ioctl(priv->handle, VPO_CONFIG_SOURCE_WINDOW, &win_config_pars);
		#endif
		if(ret < 0)
			ret = RET_FAILURE;
	}

	EXIT;
	
	MUTEX_UNLOCK;		
	
	return ret;
}

void HLD_vpo_attach(void)
{
	struct vpo_device *vpo_dev;
	struct vpo_private *priv;
	int i = 0;

	m_vpo_mutex_id = osal_mutex_create();
	if(m_vpo_mutex_id == OSAL_INVALID_ID)
	{
		VPO_PRF("vpo create mutex fail\n");
		return;
	}
	
	for(i = 0;i < MAX_VPO_NUM;i++)
	{
		vpo_dev = dev_alloc(i == 0 ? m_vpo_name_hd : m_vpo_name_sd
			,HLD_DEV_TYPE_DIS,sizeof(struct vpo_device));
		if(vpo_dev == NULL)
		{
			VPO_PRF("malloc vpo dev fail\n");
			return;
		}

		priv = malloc(sizeof(*priv));
		if(priv == NULL)
		{
			VPO_PRF("malloc osd priv fail\n");
		}
		
		memset((void *)priv, 0, sizeof(*priv));
		priv->file_path = m_fb1_path;
		priv->hd = i == 0 ? 1 : 0;
		vpo_dev->priv = (void *)priv;
		vpo_dev->next = NULL;
		
		if(dev_register(vpo_dev) != RET_SUCCESS)
		{
			VPO_PRF("register vpo dev fail\n");
			return;
		}
		m_vpo_dev[i] = vpo_dev;

		vpo_open(vpo_dev, NULL);
	}

	ioctl(priv->handle, FBIO_SET_DE_LAYER, DE_LAYER0);

	VPO_PRF("vdec create the mutex %d done\n", m_vpo_mutex_id);
	
	m_vpo_attached = 1;
	VPO_PRF("attach hld vpo ok\n");
}

void HLD_vpo_dettach(void)
{
	int i = 0;
	
	if(m_vpo_attached != 0)
	{
		for(i = 0;i < MAX_VPO_NUM;i++)
		{
			vpo_close(m_vpo_dev[i]);
			if(m_vpo_dev[i]->priv != NULL)
				free(m_vpo_dev[i]->priv);
			dev_free(m_vpo_dev[i]);
			m_vpo_dev[i] = NULL;
		}
		osal_mutex_delete(m_vpo_mutex_id);

		m_vpo_attached = 0;		
	}
}

