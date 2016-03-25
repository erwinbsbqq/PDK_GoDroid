/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_hld_dis.c
 *  (I)
 *  Description: hld dis Remote Process Call api
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
#include <ali_hdmi_common.h>

#include <rpc_hld/ali_rpc_hld_dis.h>

#include "../ali_rpc.h"

static struct vpo_callback m_vpo_cb;

static void vpo_hdmi_cb(UINT32 type, UINT32 uParam)
{
	if(m_vpo_cb.phdmi_callback)
		m_vpo_cb.phdmi_callback(uParam);
}

static UINT32 desc_vpo_m36f_attach[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct VP_Feature_Config)), DESC_STATIC_STRU(1, sizeof(struct Tve_Feature_Config)),
  2, DESC_P_PARA(0, 0, 0), DESC_P_PARA(0, 1, 1), 
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_vcap_m36f_attach[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(vcap_attach_t)),
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};

void  vpo_m36f_attach(struct VP_Feature_Config * vp_cfg, struct Tve_Feature_Config*tvec_cfg)
{
	jump_to_func(NULL, ali_rpc_call, vp_cfg, (LLD_VP_M36F_MODULE<<24)|(2<<16)|FUNC_VPO_M36_ATTACH, desc_vpo_m36f_attach);
}

void vpo_m36f_sd_attach(struct VP_Feature_Config * vp_cfg, struct Tve_Feature_Config*tvec_cfg)
{
    	jump_to_func(NULL, ali_rpc_call, vp_cfg, (LLD_VP_M36F_MODULE<<24)|(2<<16)|FUNC_VPO_M36_SD_ATTACH, desc_vpo_m36f_attach);
}

void vcap_m36f_attach(const vcap_attach_t *vcap_param)
{
    	jump_to_func(NULL, ali_rpc_call, vcap_param, (LLD_VP_M36F_MODULE<<24)|(1<<16)|FUNC_VPO_M36_VCAP_ATTACH, desc_vcap_m36f_attach);
}

void tve_write_wss(UINT16 Data)
{ 
    	jump_to_func(NULL, ali_rpc_call, Data, (LLD_VP_M36F_MODULE<<24)|(1<<16)|FUNC_TVE_WRITE_WSS, NULL);
}

void tve_write_cc(UINT8 FieldParity , UINT16 Data)
{ 
    	jump_to_func(NULL, ali_rpc_call, FieldParity, (LLD_VP_M36F_MODULE<<24)|(2<<16)|FUNC_TVE_WRITE_CC, NULL);
}

void tve_write_ttx (UINT8 LineAddr, UINT8 Addr, UINT8 Data)
{
    	jump_to_func(NULL, ali_rpc_call, LineAddr, (LLD_VP_M36F_MODULE<<24)|(3<<16)|FUNC_TVE_WRITE_TTX, NULL);
}

void tve_set_vbi_startline(UINT8 line)
{
    	jump_to_func(NULL, ali_rpc_call, line, (LLD_VP_M36F_MODULE<<24)|(1<<16)|FUNC_TVE_SET_VBI_STARTLINE, NULL);
}

UINT32 desc_osd_m36f_attach[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(OSD_DRIVER_CONFIG)),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};

void osd_m36f_attach(char *name, OSD_DRIVER_CONFIG *attach_config)
{
    jump_to_func(NULL, ali_rpc_call, name, (LLD_VP_M36F_MODULE<<24)|(2<<16)|FUNC_OSD_M36F_ATTACH, desc_osd_m36f_attach);
}

static UINT32 desc_vpo_ioctl[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, 0),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vpo_open[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct VP_InitInfo)),
	1, DESC_P_PARA(0, 1, 0), 
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vpo_config_source_window[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct vp_win_config_para)),
	1, DESC_P_PARA(0, 1, 0), 
	//desc of pointer ret
	0,                          
	0,
};

/*
static UINT32 desc_tvenc_register_dac[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct VP_DacInfo)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};
*/

static UINT32 desc_vpo_win_mode[] = 
{ //desc of pointer para
	2, DESC_STATIC_STRU(0, sizeof(struct MPSource_CallBack)),DESC_STATIC_STRU(1, sizeof(struct PIPSource_CallBack)),
	2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(0, 3, 1),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vpo_zoom[] = 
{ //desc of pointer para
	2, DESC_STATIC_STRU(0, sizeof(struct Rect)),DESC_STATIC_STRU(1, sizeof(struct Rect)),
	2, DESC_P_PARA(0, 1, 0), DESC_P_PARA(0, 2, 1),
	//desc of pointer ret
	0,                          
	0,
};

RET_CODE vpo_open(struct vpo_device *dev,struct VP_InitInfo *pInitInfo)
{
	

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_OPEN, desc_vpo_open);


}
EXPORT_SYMBOL(vpo_open);

RET_CODE vpo_close(struct vpo_device *dev)
{	
	

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_VPO_CLOSE, NULL);


}
EXPORT_SYMBOL(vpo_close);
	
RET_CODE vpo_win_onoff(struct vpo_device *dev,BOOL bOn)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_WIN_ONOFF, NULL);


}
EXPORT_SYMBOL(vpo_win_onoff);

RET_CODE vpo_win_onoff_ext(struct vpo_device *dev,BOOL bOn,enum vp_display_layer layer)
{

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_WIN_ONOFF_EXT, NULL);

}
EXPORT_SYMBOL(vpo_win_onoff_ext);

RET_CODE vpo_win_mode(struct vpo_device *dev, BYTE bWinMode, struct MPSource_CallBack *pMPCallBack,struct PIPSource_CallBack *pPIPCallBack)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(4<<16)|FUNC_VPO_WIN_MODE,desc_vpo_win_mode);


}
EXPORT_SYMBOL(vpo_win_mode);

RET_CODE vpo_zoom(struct vpo_device *dev, struct Rect *pSrcRect , struct Rect *pDstRect)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_ZOOM, desc_vpo_zoom);


}
EXPORT_SYMBOL(vpo_zoom);

RET_CODE vpo_zoom_ext(struct vpo_device *dev, struct Rect *pSrcRect , struct Rect *pDstRect,enum vp_display_layer layer)
{

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(4<<16)|FUNC_VPO_ZOOM_EXT, desc_vpo_zoom);

}
EXPORT_SYMBOL(vpo_zoom_ext);

RET_CODE vpo_aspect_mode(struct vpo_device *dev, enum TVMode eTVAspect, enum DisplayMode e169DisplayMode)
{
	

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_ASPECT_MODE, NULL);


}
EXPORT_SYMBOL(vpo_aspect_mode);

RET_CODE vpo_tvsys(struct vpo_device *dev, enum TVSystem eTVSys)
{
	

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_TVSYS, NULL);


}
EXPORT_SYMBOL(vpo_tvsys);

RET_CODE vpo_tvsys_ex(struct vpo_device *dev, enum TVSystem eTVSys, BOOL bProgressive)
{
	

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_TVSYS_EX, NULL);


}
EXPORT_SYMBOL(vpo_tvsys_ex);

RET_CODE vpo_ioctl(struct vpo_device *dev, UINT32 dwCmd, UINT32 dwParam)
{


	UINT32 i;
	UINT32 common_desc[sizeof(desc_vpo_ioctl)];

	//MEMCPY(common_desc, desc_vpo_ioctl, sizeof(desc_vpo_ioctl));
	UINT32 *desc = (UINT32 *)common_desc;
	UINT32 *b = (UINT32 *)desc_vpo_ioctl;

	for(i = 0; i < sizeof(desc_vpo_ioctl)/sizeof(UINT32); i++)
		desc[i] = b[i];

	switch(dwCmd)
	{
		case VPO_IO_SET_BG_COLOR:
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  YCbCrColor));
			break;
		case VPO_IO_WRITE_TTX:
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_ttx));
			break;
		case VPO_IO_WRITE_CC:
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_cc));
			break;
		case VPO_IO_REG_DAC:
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vp_io_reg_dac_para));
			break;	
		case VPO_IO_REG_CB_GE: // de_n only
			// DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_register_ge_callback));
			// break;		
			return RET_FAILURE;
		case VPO_IO_SET_PARAMETER:  // de_n only
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_set_parameter));
			break;		
		case VPO_IO_VIDEO_ENHANCE: //de_n only
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct  vpo_io_video_enhance));
			break;		
		case VPO_IO_GET_OUT_MODE:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
			break;
		case VPO_IO_GET_SRC_ASPECT:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
			break;
		case VPO_IO_GET_INFO:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(struct vpo_io_get_info));
			break;
		case VPO_IO_GET_REAL_DISPLAY_MODE:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
			break;        	
		case VPO_IO_GET_TV_ASPECT:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
			break;        	        
		case VPO_IO_GET_DE2HDMI_INFO:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(struct de2Hdmi_video_infor));
			break;
		case VPO_IO_REG_CB_SRCMODE_CHANGE:
		case VPO_IO_REG_CB_SRCASPECT_CHANGE:
			return RET_FAILURE;
		case VPO_IO_REG_CB_HDMI: //de_n only		
			m_vpo_cb.phdmi_callback = (OSAL_T_HSR_PROC_FUNC_PTR)dwParam;
			desc = NULL;
			break;
        	case VPO_IO_SET_CGMS_INFO:
            		DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct vpo_io_cgms_info));			
			break;			
        case VPO_IO_AFD_CONFIG:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct vp_io_afd_para));			
			break;
        case VPO_IO_GET_CURRENT_DISPLAY_INFO:
        case VPO_IO_BACKUP_CURRENT_PICTURE:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(struct vpo_io_get_picture_info));
            break;
        case VPO_IO_SET_OSD_SHOW_TIME:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(vpo_osd_show_time_t));
            break;
        case VPO_IO_GET_OSD0_SHOW_TIME:
        case VPO_IO_GET_OSD1_SHOW_TIME:
        case VPO_IO_GET_DISPLAY_MODE:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 4);
			break;
        case VPO_IO_GET_MP_SCREEN_RECT:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(struct Rect));
            break;
	case VPO_IO_SET_CUTLINE:
		DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct vpo_io_cutline_pars));
		break;
	case VPO_IO_DISPLAY_3D_ENABLE:
		DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct alifbio_3d_pars));
		break;
	case VPO_IO_SET_3D_ENH:
		DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(struct alifbio_3d_enhance_pars));
		break;
		case VPO_IO_SET_VBI_OUT:
		case VPO_IO_WRITE_WSS:
		case VPO_IO_UNREG_DAC:
		case VPO_IO_PRINTF_HW_INFO:
		case VPO_IO_PRINTF_HW_SCALE_INIT:
		case VPO_IO_PREFRAME_DETECT_ONOFF:  //not use
		case VPO_IO_HDMI_OUT_PIC_FMT: //de_n only
		case VPO_IO_MHEG_SCENE_AR:
		case VPO_IO_MHEG_IFRAME_NOTIFY:    
		case VPO_IO_DISAUTO_WIN_ONOFF:    
		case VPO_IO_ENABLE_VBI:
		case VPO_IO_PLAYMODE_CHANGE:
		case VPO_IO_DIT_CHANGE:
		case VPO_IO_SWAFD_ENABLE:
		case VPO_IO_704_OUTPUT:
		case VPO_IO_SET_LAYER_ORDER:	
        case VPO_IO_ALWAYS_OPEN_CGMS_INFO:
        case VPO_IO_TVESDHD_SOURCE_SEL:
        case VPO_IO_SD_CC_ENABLE:	
		case VPO_IO_ENABLE_DE_AVMUTE_HDMI:
		case VPO_IO_SET_DE_AVMUTE_HDMI:
		case VPO_IO_GET_DE_AVMUTE_HDMI:
		case VPO_IO_BOOT_UP_DONE:
		case VPO_IO_SET_DISPLAY_STYLE:
		default:
			desc = NULL;
			break;       
	}    
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_VPO_IOCTL, desc);


}
EXPORT_SYMBOL(vpo_ioctl);

RET_CODE vpo_config_source_window(struct vpo_device *dev, struct vp_win_config_para *pwin_para)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_CONFIG_SOURCE_WINDOW, desc_vpo_config_source_window);


}
EXPORT_SYMBOL(vpo_config_source_window);

RET_CODE vpo_set_progres_interl(struct vpo_device *dev, BOOL bProgressive)
{
	

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_VPO_SET_PROGRES_INTERL, NULL);


}
EXPORT_SYMBOL(vpo_set_progres_interl);

void vpo_register_cb_routine(void)
{
	ali_rpc_register_callback(ALI_RPC_CB_VPO_HDMI, vpo_hdmi_cb);
}
EXPORT_SYMBOL(vpo_register_cb_routine);

/*
RET_CODE tvenc_open(struct tve_device *dev)
{
		
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_TVENC_OPEN, NULL);

	
}

RET_CODE tvenc_close(struct tve_device *dev)
{
		
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_TVENC_CLOSE, NULL);

	
}

RET_CODE tvenc_set_tvsys(struct tve_device *dev,enum TVSystem eTVSys)
{
			
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_TVENC_SET_TVSYS, NULL);

	
}

RET_CODE tvenc_set_tvsys_ex(struct tve_device *dev,enum TVSystem eTVSys, BOOL bProgressive)
{
		
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_TVENC_SET_TVSYS_EX, NULL);

	
}

RET_CODE tvenc_register_dac(struct tve_device *dev,enum DacType eDacType, struct VP_DacInfo *pInfo)
{
			

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_TVENC_REGISTER_DAC, desc_tvenc_register_dac);

	
}

RET_CODE tvenc_unregister_dac(struct tve_device *dev,enum DacType eDacType)
{
			

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_TVENC_UNREGISTER_DAC, NULL);

	
}

RET_CODE tvenc_write_wss(struct tve_device *dev,UINT16 Data)
{
		
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_TVENC_WRITE_WSS, NULL);

	
}

RET_CODE tvenc_write_cc(struct tve_device *dev,UINT8 FieldParity, UINT16 Data)
{
			
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_TVENC_WRITE_CC, NULL);

	
}

RET_CODE tvenc_write_ttx(struct tve_device *dev,UINT8 LineAddr, UINT8 Addr, UINT8 Data)
{
		
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(4<<16)|FUNC_TVENC_WRITE_TTX, NULL);

	
}

void tve_advance_config(struct tve_adjust *tve_adj_adv)
{
    jump_to_func(NULL, ali_rpc_call, tve_adj_adv, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_TVE_ADVANCE_CONFIG, NULL);
}

*/

