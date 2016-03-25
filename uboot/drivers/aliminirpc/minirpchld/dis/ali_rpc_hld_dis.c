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
#include <linux/ali_hdmi.h>

#include <rpc_hld/ali_rpc_hld_dis.h>

//#include "../ali_rpc.h"
#include <ali_rpcng.h>
static struct vpo_callback m_vpo_cb;

static void vpo_hdmi_cb(UINT32 type, UINT32 uParam)
{
	if(m_vpo_cb.phdmi_callback)
		m_vpo_cb.phdmi_callback(uParam);
}

void  vpo_m36f_attach(struct VP_Feature_Config * vp_cfg, struct Tve_Feature_Config*tvec_cfg)
{
    Vp_feature_config_rpc vp_f;
    Tve_feature_config_rpc tve_f;

    memcpy(&vp_f,vp_cfg,sizeof(Vp_feature_config_rpc));

    memset(&tve_f,0,sizeof(Tve_feature_config_rpc));
    tve_f.config = tvec_cfg->config;
    tve_f.val_tve_tbl_all = tvec_cfg->tve_tbl_all;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Vp_feature_config_rpc, sizeof(Vp_feature_config_rpc), &vp_f);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Tve_feature_config_rpc, sizeof(Tve_feature_config_rpc), &tve_f);   

    RpcCallCompletion(RPC_m36g_vpo_attach,&p1,&p2,NULL);
}

void vpo_m36f_sd_attach(struct VP_Feature_Config * vp_cfg, struct Tve_Feature_Config*tvec_cfg)
{
    Vp_feature_config_rpc vp_f;
    Tve_feature_config_rpc tve_f;

    memcpy(&vp_f,vp_cfg,sizeof(Vp_feature_config_rpc));
    memset(&tve_f,0,sizeof(Tve_feature_config_rpc));
    tve_f.config = tvec_cfg->config;
    tve_f.index_tve_tbl_all = tvec_cfg->tve_adjust;
    tve_f.index_tve_tbl_all_1 = tvec_cfg->tve_adjust_adv;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Vp_feature_config_rpc, sizeof(Vp_feature_config_rpc), &vp_f);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Tve_feature_config_rpc, sizeof(Tve_feature_config_rpc), &tve_f);   

    RpcCallCompletion(RPC_m36g_vpo_sd_attach,&p1,&p2,NULL);
}

void vcap_m36f_attach(const vcap_attach_t *vcap_param)
{
    UINT32 param1 = vcap_param->fb_addr;
    UINT32 param2 = vcap_param->fb_size;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &param1);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &param2);   
    
    RpcCallCompletion(RPC_m36g_vcap_attach,&p1,&p2,NULL);   
}

void tve_write_wss(UINT16 Data)
{ 
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT16, sizeof(UINT16), &Data);   
    
    RpcCallCompletion(RPC_tve_write_wss,&p1,NULL);   
}

void tve_write_cc(UINT8 FieldParity , UINT16 Data)
{ 
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &FieldParity);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT16, sizeof(UINT16), &Data);   
    
    RpcCallCompletion(RPC_tve_write_cc,&p1,&p2,NULL);   
}

void tve_write_ttx (UINT8 LineAddr, UINT8 Addr, UINT8 Data)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &LineAddr);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &Addr);   
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &Data);   
    
    RpcCallCompletion(RPC_tve_write_ttx,&p1,&p2,&p3,NULL);
}

void tve_set_vbi_startline(UINT8 line)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &line);
    
    RpcCallCompletion(RPC_tve_set_vbi_startline,&p1,NULL);
}
// Not use  add poe.cao
void osd_m36f_attach(char *name, OSD_DRIVER_CONFIG *attach_config)
{
    //jump_to_func(NULL, ali_rpc_call, name, (LLD_VP_M36F_MODULE<<24)|(2<<16)|FUNC_OSD_M36F_ATTACH, desc_osd_m36f_attach);
    do{}while(0);
}

RET_CODE vpo_open(struct vpo_device *dev,struct VP_InitInfo *pInitInfo)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Vp_initinfo_rpc, sizeof(Vp_initinfo_rpc), pInitInfo);
    ret = RpcCallCompletion(RPC_vpo_open, &p1, &p2, NULL);

    return ret;   
}
EXPORT_SYMBOL(vpo_open);

RET_CODE vpo_close(struct vpo_device *dev)
{	
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    ret = RpcCallCompletion(RPC_vpo_close, &p1, NULL);

    return ret;   
}
EXPORT_SYMBOL(vpo_close);
	
RET_CODE vpo_win_onoff(struct vpo_device *dev,BOOL bOn)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_BOOL, sizeof(BOOL), &bOn);

    ret = RpcCallCompletion(RPC_vpo_win_onoff, &p1, &p2, NULL);

    return ret;   
}
EXPORT_SYMBOL(vpo_win_onoff);

RET_CODE vpo_win_mode(struct vpo_device *dev, BYTE bWinMode, struct MPSource_CallBack *pMPCallBack,struct PIPSource_CallBack *pPIPCallBack)
{
    Int32 ret;
    Mpsource_callback_rpc mp_call;
    Pipsource_callback_rpc pip_call;

    mp_call.handler = pMPCallBack->handler;
    mp_call.ReleaseCallback = pMPCallBack->ReleaseCallback;
    mp_call.RequestCallback = pMPCallBack->RequestCallback;
    mp_call.vblanking_callback = pMPCallBack->vblanking_callback;

    pip_call.ReleaseCallback = pPIPCallBack->ReleaseCallback;
    pip_call.RequestCallback = pPIPCallBack->RequestCallback;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_CHAR, sizeof(UINT8), &bWinMode);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_Mpsource_callback_rpc, sizeof(Mpsource_callback_rpc), &mp_call);   
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_Pipsource_callback_rpc, sizeof(Pipsource_callback_rpc), &pip_call);  
    ret = RpcCallCompletion(RPC_vpo_win_mode, &p1, &p2, &p3, &p4, NULL);

    return ret;
}
EXPORT_SYMBOL(vpo_win_mode);

RET_CODE vpo_zoom(struct vpo_device *dev, struct Rect *pSrcRect , struct Rect *pDstRect)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Rect_rpc, sizeof(Rect_rpc), pSrcRect);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_Rect_rpc, sizeof(Rect_rpc), pDstRect);   

    ret = RpcCallCompletion(RPC_vpo_zoom, &p1, &p2, &p3, NULL);

    return ret;
}
EXPORT_SYMBOL(vpo_zoom);

RET_CODE vpo_aspect_mode(struct vpo_device *dev, enum TVMode eTVAspect, enum DisplayMode e169DisplayMode)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum TVMode), &eTVAspect);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_ENUM, sizeof(enum DisplayMode), &e169DisplayMode);   

    ret = RpcCallCompletion(RPC_vpo_aspect_mode, &p1, &p2, &p3, NULL);

    return ret;   
}
EXPORT_SYMBOL(vpo_aspect_mode);

RET_CODE vpo_tvsys(struct vpo_device *dev, enum TVSystem eTVSys)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum TVSystem), &eTVSys);

    ret = RpcCallCompletion(RPC_vpo_tvsys, &p1, &p2, NULL);

    return ret;   
}
EXPORT_SYMBOL(vpo_tvsys);

RET_CODE vpo_tvsys_ex(struct vpo_device *dev, enum TVSystem eTVSys, BOOL bProgressive)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum TVSystem), &eTVSys);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_BOOL, sizeof(BOOL), &bProgressive);   

    ret = RpcCallCompletion(RPC_vpo_tvsys_ex, &p1, &p2, &p3, NULL);

    return ret;   
}
EXPORT_SYMBOL(vpo_tvsys_ex);

RET_CODE vpo_ioctl(struct vpo_device *dev, UINT32 dwCmd, UINT32 dwParam)
{
	Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dwCmd);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dwParam);   

	switch(dwCmd)
	{
		case VPO_IO_SET_BG_COLOR:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Ycbcrcolor_rpc, sizeof(Ycbcrcolor_rpc), dwParam);
			break;
		case VPO_IO_WRITE_TTX:
                     RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Vpo_io_ttx_rpc, sizeof(Vpo_io_ttx_rpc), dwParam);
			break;
		case VPO_IO_WRITE_CC:
                     RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Vpo_io_cc_rpc, sizeof(Vpo_io_cc_rpc), dwParam);
			break;
		case VPO_IO_REG_DAC:
                     RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Vp_io_reg_dac_para_rpc, sizeof(Vp_io_reg_dac_para_rpc), dwParam);
			break;	
		case VPO_IO_REG_CB_GE: // de_n only
			return RET_FAILURE;
		case VPO_IO_SET_PARAMETER:  // de_n only
		       RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Vpo_io_set_parameter_rpc, sizeof(Vpo_io_set_parameter_rpc), dwParam);
			break;		
		case VPO_IO_VIDEO_ENHANCE: //de_n only
		       RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Vpo_io_video_enhance_rpc, sizeof(Vpo_io_video_enhance_rpc), dwParam);
			break;		
		case VPO_IO_GET_OUT_MODE:
                     RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(Uint32), &dwParam);
			break;
		case VPO_IO_GET_SRC_ASPECT:
                     RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(Uint32), &dwParam);
			break;
		case VPO_IO_GET_INFO:
                     RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Vpo_io_get_info_rpc, sizeof(Vpo_io_get_info_rpc), dwParam);
			break;
		case VPO_IO_GET_REAL_DISPLAY_MODE:
                     RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(UINT32), &dwParam);
			break;        	
		case VPO_IO_GET_TV_ASPECT:
                     RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(UINT32), &dwParam);
			break;        	        
		case VPO_IO_GET_DE2HDMI_INFO:
                     RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_De2Hdmi_video_infor_rpc, sizeof(De2Hdmi_video_infor_rpc), dwParam);
			break;
		case VPO_IO_REG_CB_SRCMODE_CHANGE:
		case VPO_IO_REG_CB_SRCASPECT_CHANGE:
			return RET_FAILURE;
		case VPO_IO_REG_CB_HDMI: //de_n only		
			m_vpo_cb.phdmi_callback = (OSAL_T_HSR_PROC_FUNC_PTR)dwParam;
			break;
        	case VPO_IO_SET_CGMS_INFO:
                     RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Vpo_io_cgms_info_rpc, sizeof(Vpo_io_cgms_info_rpc), dwParam); 	
			break;			
        case VPO_IO_AFD_CONFIG:
               RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Vp_io_afd_para_rpc, sizeof(Vp_io_afd_para_rpc), dwParam);     
			break;
        case VPO_IO_GET_CURRENT_DISPLAY_INFO:
        case VPO_IO_BACKUP_CURRENT_PICTURE:
               RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Vpo_io_get_picture_info_rpc, sizeof(Vpo_io_get_picture_info_rpc), dwParam);  
            break;
        case VPO_IO_SET_OSD_SHOW_TIME:
               RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Vpo_osd_show_time_rpc, sizeof(Vpo_osd_show_time_rpc), dwParam);  
            break;
        case VPO_IO_GET_OSD0_SHOW_TIME:
        case VPO_IO_GET_OSD1_SHOW_TIME:
        case VPO_IO_GET_DISPLAY_MODE:
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(UINT32), dwParam);    
			break;
        case VPO_IO_GET_MP_SCREEN_RECT:
               RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Rect_rpc, sizeof(Rect_rpc), dwParam);
            break;
	case VPO_IO_SET_CUTLINE:
              RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Vpo_io_cutline_pars_rpc, sizeof(Vpo_io_cutline_pars_rpc), dwParam);
		break;
	case VPO_IO_DISPLAY_3D_ENABLE:
              RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Alifbio_3d_pars_rpc, sizeof(Alifbio_3d_pars_rpc), dwParam);
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
		case VPO_IO_SET_MG_APS_INFO:
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
			break;       
	}   
    ret = RpcCallCompletion(RPC_vpo_ioctl, &p1, &p2, &p3, NULL);
    return ret;   
}
EXPORT_SYMBOL(vpo_ioctl);

RET_CODE vpo_config_source_window(struct vpo_device *dev, struct vp_win_config_para *pwin_para)
{
    Vp_win_config_para_rpc vp_win;
    
    vp_win.source_number = pwin_para->source_number;

    UINT8 i = 0;
    
    for(i = 0; i < 4; i++)
    {
        vp_win.source_info[i].src_module_devide_handle = pwin_para->source_info[i].src_module_devide_handle;
        vp_win.source_info[i].handler = pwin_para->source_info[i].src_callback.handler;
        vp_win.source_info[i].request_callback = pwin_para->source_info[i].src_callback.request_callback;
        vp_win.source_info[i].release_callback = pwin_para->source_info[i].src_callback.release_callback;      
        vp_win.source_info[i].vblanking_callback = pwin_para->source_info[i].src_callback.vblanking_callback;
        vp_win.source_info[i].src_path_index = pwin_para->source_info[i].src_path_index;
        vp_win.source_info[i].attach_source_index = pwin_para->source_info[i].attach_source_index;    
    }
    vp_win.control_source_index = pwin_para->control_source_index;
    vp_win.mainwin_source_index = pwin_para->mainwin_source_index;
    vp_win.window_number = pwin_para->window_number;

    for(i = 0; i < 4; i++)
    {
        vp_win.window_parameter[i].source_index = pwin_para->window_parameter[i].source_index;
        vp_win.window_parameter[i].display_layer = pwin_para->window_parameter[i].display_layer;
        vp_win.window_parameter[i].src_module_devide_handle = pwin_para->window_parameter[i].src_module_devide_handle;
        vp_win.window_parameter[i].handler = pwin_para->window_parameter[i].src_callback.handler;
        vp_win.window_parameter[i].request_callback = pwin_para->window_parameter[i].src_callback.request_callback;
        vp_win.window_parameter[i].release_callback = pwin_para->window_parameter[i].src_callback.release_callback;
        vp_win.window_parameter[i].vblanking_callback = pwin_para->window_parameter[i].src_callback.vblanking_callback;      
        memcpy(&vp_win.window_parameter[i].rect, &pwin_para->window_parameter[i].rect, sizeof(struct vp_win_config));
    }
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_BOOL, sizeof(BOOL), &vp_win);   

    ret = RpcCallCompletion(RPC_vpo_config_source_window,&p1,&p2,NULL);

    return ret;
}
EXPORT_SYMBOL(vpo_config_source_window);

RET_CODE vpo_set_progres_interl(struct vpo_device *dev, BOOL bProgressive)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_BOOL, sizeof(BOOL), &bProgressive);   

    ret = RpcCallCompletion(RPC_vpo_set_progres_interl,&p1,&p2,NULL);

    return ret;   
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
	register RET_CODE ret asm("$2");	
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_TVENC_OPEN, NULL);

	return ret;
}

RET_CODE tvenc_close(struct tve_device *dev)
{
	register RET_CODE ret asm("$2");	
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_TVENC_CLOSE, NULL);

	return ret;
}

RET_CODE tvenc_set_tvsys(struct tve_device *dev,enum TVSystem eTVSys)
{
	register RET_CODE ret asm("$2");		
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_TVENC_SET_TVSYS, NULL);

	return ret;
}

RET_CODE tvenc_set_tvsys_ex(struct tve_device *dev,enum TVSystem eTVSys, BOOL bProgressive)
{
	register RET_CODE ret asm("$2");	
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_TVENC_SET_TVSYS_EX, NULL);

	return ret;
}

RET_CODE tvenc_register_dac(struct tve_device *dev,enum DacType eDacType, struct VP_DacInfo *pInfo)
{
	register RET_CODE ret asm("$2");		

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_TVENC_REGISTER_DAC, desc_tvenc_register_dac);

	return ret;
}

RET_CODE tvenc_unregister_dac(struct tve_device *dev,enum DacType eDacType)
{
	register RET_CODE ret asm("$2");		

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_TVENC_UNREGISTER_DAC, NULL);

	return ret;
}

RET_CODE tvenc_write_wss(struct tve_device *dev,UINT16 Data)
{
	register RET_CODE ret asm("$2");	
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(2<<16)|FUNC_TVENC_WRITE_WSS, NULL);

	return ret;
}

RET_CODE tvenc_write_cc(struct tve_device *dev,UINT8 FieldParity, UINT16 Data)
{
	register RET_CODE ret asm("$2");		
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(3<<16)|FUNC_TVENC_WRITE_CC, NULL);

	return ret;
}

RET_CODE tvenc_write_ttx(struct tve_device *dev,UINT8 LineAddr, UINT8 Addr, UINT8 Data)
{
	register RET_CODE ret asm("$2");	
	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_VP_MODULE<<24)|(4<<16)|FUNC_TVENC_WRITE_TTX, NULL);

	return ret;
}

void tve_advance_config(struct tve_adjust *tve_adj_adv)
{
    jump_to_func(NULL, ali_rpc_call, tve_adj_adv, (HLD_VP_MODULE<<24)|(1<<16)|FUNC_TVE_ADVANCE_CONFIG, NULL);
}

*/

