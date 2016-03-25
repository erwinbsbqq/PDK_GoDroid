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
#include <rpc_hld/ali_rpc_hld_decv.h>

#include <ali_video_common.h>

//#include "../ali_rpc.h"
#include <ali_rpcng.h>
static unsigned int m_vdec_XD = 0;//add by phil for XD311
//add by phil for boot-media
void boot_media_start(UINT32 addr, UINT32 len)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &addr);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &len);

    RpcCallCompletion(RPC_boot_media_start,&p1,&p2,NULL);      
}

void vdec_m36_attach(struct vdec_config_par *pconfig_par)
{
    Vdec_adpcm_rpc   vdec_adp;
    Vdec_sml_frm_rpc vdec_sml;
    Vdec_mem_map_rpc vdec_mem;
    Vdec_config_par_rpc vdec_con;

    memcpy(&vdec_adp, &pconfig_par->adpcm, sizeof(Vdec_adpcm_rpc));
    memcpy(&vdec_sml, &pconfig_par->sml_frm, sizeof(Vdec_sml_frm_rpc));
    memcpy(&vdec_mem, &pconfig_par->mem_map, sizeof(Vdec_mem_map_rpc));

    vdec_con.user_data_parsing = pconfig_par->user_data_parsing;
    vdec_con.dtg_afd_parsing   = pconfig_par->dtg_afd_parsing;
    vdec_con.drop_freerun_pic_before_firstpic_show = pconfig_par->drop_freerun_pic_before_firstpic_show;
    vdec_con.reset_hw_directly_when_stop = pconfig_par->reset_hw_directly_when_stop;
    vdec_con.show_hd_service = pconfig_par->show_hd_service;
    vdec_con.still_frm_in_cc = pconfig_par->still_frm_in_cc;
    vdec_con.extra_dview_window = pconfig_par->extra_dview_window;
    vdec_con.not_show_mosaic = pconfig_par->not_show_mosaic;
    vdec_con.return_multi_freebuf = pconfig_par->return_multi_freebuf;
    vdec_con.advance_play = pconfig_par->advance_play;
    vdec_con.lm_shiftthreshold = pconfig_par->lm_shiftthreshold;
    vdec_con.vp_init_phase = pconfig_par->vp_init_phase;
    vdec_con.preview_solution = pconfig_par->preview_solution;
    vdec_con.res_bits = pconfig_par->res_bits;
    vdec_con.res_pointer = pconfig_par->res_pointer;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Vdec_adpcm_rpc, sizeof(Vdec_adpcm_rpc), &vdec_adp);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Vdec_sml_frm_rpc, sizeof(Vdec_sml_frm_rpc), &vdec_sml);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_Vdec_mem_map_rpc, sizeof(Vdec_mem_map_rpc), &vdec_mem);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_Vdec_config_par_rpc, sizeof(Vdec_config_par_rpc), &vdec_con);

    RpcCallCompletion(RPC_vdec_m36_attach,&p1,&p2,&p3,&p4,NULL);   
}

RET_CODE vdec_enable_advance_play(struct vdec_device *dev)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);

    ret = RpcCallCompletion(RPC_vdec_enable_advance_play,&p1,NULL);
    
    return ret;
}

void vdec_disable_dview(struct vdec_device *dev)
{
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);

    RpcCallCompletion(RPC_vdec_disable_dview,&p1,NULL);    
}

void vdec_avc_attach(struct vdec_avc_config_par *pconfig_par)
{
    Vdec_avc_memmap_rpc vdec_avc;
    Vdec_avc_config_par_rpc vdec_avc_con;
    
    memcpy(&vdec_avc, &pconfig_par->memmap, sizeof(Vdec_avc_memmap_rpc));
    vdec_avc_con.max_additional_fb_num = pconfig_par->max_additional_fb_num;
    vdec_avc_con.dtg_afd_parsing = pconfig_par->dtg_afd_parsing;
#ifdef VIDEO_SEAMLESS_SWITCHING
	vdec_avc_con.seamless_enable = pconfig_par->seamless_enable;
#endif

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Vdec_avc_memmap_rpc, sizeof(Vdec_avc_memmap_rpc), &vdec_avc);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Vdec_avc_config_par_rpc, sizeof(Vdec_avc_config_par_rpc), &vdec_avc_con);
    
    RpcCallCompletion(RPC_vdec_avc_attach,&p1,&p2,NULL);    

}

RET_CODE vdec_open(struct vdec_device *dev)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);

    ret = RpcCallCompletion(RPC_vdec_open,&p1,NULL);
    
    return ret;

}

RET_CODE vdec_close(struct vdec_device *dev)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);

    ret = RpcCallCompletion(RPC_vdec_close,&p1,NULL);

    return ret;

}

RET_CODE vdec_start(struct vdec_device *dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);

    ret = RpcCallCompletion(RPC_vdec_start,&p1,NULL);
    return ret;

}

RET_CODE vdec_stop(struct vdec_device *dev,BOOL bclosevp,BOOL bfillblack)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_BOOL, sizeof(BOOL), &bclosevp);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_BOOL, sizeof(BOOL), &bfillblack);

    ret = RpcCallCompletion(RPC_vdec_stop,&p1,&p2,&p3,NULL);
    return ret;

    
}

RET_CODE vdec_vbv_request(void *dev, UINT32 uSizeRequested, void ** ppVData, UINT32 * puSizeGot, struct control_block * ctrl_blk)
{    
    INT32 ret;
    Control_block_rpc con_blk;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &uSizeRequested);
    RPC_PARAM_CREATE(p3, PARAM_OUT, PARAM_UINT32, 4, ppVData);
    RPC_PARAM_CREATE(p4, PARAM_OUT, PARAM_UINT32, 4, puSizeGot);
    RPC_PARAM_CREATE(p5, PARAM_IN, PARAM_Control_block_rpc, sizeof(Control_block_rpc), &con_blk);

    if(ctrl_blk != NULL)
    {        
        con_blk.stc_id_valid = ctrl_blk->stc_id_valid;
        con_blk.pts_valid    = ctrl_blk->pts_valid;
        con_blk.data_continue= ctrl_blk->data_continue;
        con_blk.ctrlblk_valid= ctrl_blk->ctrlblk_valid;
        con_blk.instant_update=ctrl_blk->instant_update;
        con_blk.vob_start   = ctrl_blk->vob_start;
        con_blk.bstream_run_back= ctrl_blk->bstream_run_back;
        con_blk.reserve     = ctrl_blk->reserve;
        con_blk.stc_id      = ctrl_blk->stc_id;
        con_blk.stc_offset_idx= ctrl_blk->stc_offset_idx;
        con_blk.pts         = ctrl_blk->pts;        
    }
    else
    {
        RPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_Control_block_rpc, sizeof(Control_block_rpc), NULL);
    }
    if(ppVData == NULL)
    {
         RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 0, NULL);
    }
    if(puSizeGot == NULL)
    {
         RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, 0, NULL);
    }
        
    
    ret = RpcCallCompletion(RPC_vdec_vbv_request,&p1,&p2,&p3,&p4,&p5,NULL);
    return ret; 
 
}

void vdec_vbv_update(void *dev, UINT32 uDataSize)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &uDataSize);
    
    RpcCallCompletion(RPC_vdec_vbv_update,&p1,&p2,NULL);

}

RET_CODE vdec_set_output(struct vdec_device *dev, enum VDecOutputMode eMode,struct VDecPIPInfo *pInitInfo, 	struct MPSource_CallBack *pMPCallBack, struct PIPSource_CallBack *pPIPCallBack)
{
    Int32 ret;
    Mpsource_callback_rpc mps_cb;
    Pipsource_callback_rpc pips_cb;

    memset(&mps_cb, 0, sizeof(mps_cb));
    memset(&pips_cb, 0, sizeof(pips_cb));
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum VDecOutputMode), &eMode);
    RPC_PARAM_CREATE(p3, PARAM_IN,PARAM_Vdecpipinfo_rpc, sizeof(Vdecpipinfo_rpc), pInitInfo);    
    RPC_PARAM_CREATE(p4, PARAM_OUT,PARAM_Mpsource_callback_rpc, sizeof(Mpsource_callback_rpc), &mps_cb);
    RPC_PARAM_CREATE(p5, PARAM_OUT,PARAM_Pipsource_callback_rpc, sizeof(Pipsource_callback_rpc), &pips_cb);

    ret = RpcCallCompletion(RPC_vdec_set_output,&p1,&p2,&p3,&p4,&p5,NULL);

    pMPCallBack->handler = (struct vdec_device *)(mps_cb.handler);
    pMPCallBack->RequestCallback = (T_MPRequest)(mps_cb.RequestCallback);
    pMPCallBack->ReleaseCallback = (T_MPRelease)(mps_cb.ReleaseCallback);
    pMPCallBack->vblanking_callback = (T_VBlanking)(mps_cb.vblanking_callback);
    pPIPCallBack->RequestCallback = (T_Request_ext)(pips_cb.RequestCallback);
    pPIPCallBack->ReleaseCallback = (T_Release_ext)(pips_cb.ReleaseCallback);

    return ret;
        
}

RET_CODE vdec_switch_pip(struct vdec_device *dev, struct Position *pPIPPos)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Position_rpc, sizeof(Position_rpc), pPIPPos);

    ret = RpcCallCompletion(RPC_vdec_switch_pip,&p1,&p2,NULL);
    return ret;
    
}

RET_CODE vdec_switch_pip_ext(struct vdec_device *dev, struct Rect *pPIPWin)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Rect_rpc, sizeof(Rect_rpc), pPIPWin);

    ret =RpcCallCompletion(RPC_vdec_switch_pip_ext,&p1,&p2,NULL);
    return ret;
    
}

RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 uSyncMode,UINT8 uSyncLevel)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &uSyncMode);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &uSyncLevel);

    ret = RpcCallCompletion(RPC_vdec_sync_mode,&p1,&p2,&p3,NULL);
    return ret;    
}

RET_CODE vdec_extrawin_store_last_pic(struct vdec_device *dev, struct Rect *pPIPWin)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dev);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Rect_rpc, sizeof(Rect_rpc), pPIPWin);

    ret = RpcCallCompletion(RPC_vdec_extrawin_store_last_pic,&p1,&p2,NULL);
    return ret;
}

RET_CODE vdec_profile_level(struct vdec_device *dev, UINT8 uProfileLevel,VDEC_BEYOND_LEVEL cb_beyond_level)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &uProfileLevel);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &cb_beyond_level);   

    ret = RpcCallCompletion(RPC_vdec_profile_level,&p1,&p2,&p3,NULL);
    return ret;
}

RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param)
{
    Vdec_io_reg_callback_para_rpc vdec_io;
    struct vdec_io_reg_callback_para *tmp = (struct vdec_io_reg_callback_para *)param;

    Vdec_picture_rpc vdec_pic;
    struct vdec_picture *tmp1 = (struct vdec_picture *)param;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &io_code);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32,sizeof(UINT32), &param);      
    
	switch(io_code)
	{
		case VDEC_IO_GET_STATUS:
            RPC_PARAM_UPDATE(p3, PARAM_OUT,PARAM_Vdec_statusinfo_rpc,sizeof(Vdec_statusinfo_rpc), param);
			break;
        case VDEC_IO_GET_DECORE_STATUS:
            RPC_PARAM_UPDATE(p3, PARAM_OUT,PARAM_Vdec_decore_status_rpc,sizeof(Vdec_decore_status_rpc), param);            
            break;
		case VDEC_IO_GET_MODE:
            RPC_PARAM_UPDATE(p3, PARAM_OUT,PARAM_UINT32,4, param);            
			break;
		case VDEC_IO_GET_OUTPUT_FORMAT:
            RPC_PARAM_UPDATE(p3, PARAM_OUT,PARAM_UINT32,4, param);           
			break;
		case VDEC_IO_GET_FRM:
            RPC_PARAM_UPDATE(p3, PARAM_OUT,PARAM_Vdec_io_get_frm_para_rpc,sizeof(Vdec_io_get_frm_para_rpc), param);            
			break;	
		case VDEC_IO_FILL_FRM:
		case VDEC_IO_FILL_FRM2:
            RPC_PARAM_UPDATE(p3, PARAM_IN,PARAM_Ycbcrcolor_rpc,sizeof(Ycbcrcolor_rpc), param);
			break;
		case VDEC_IO_REG_CALLBACK:		
            vdec_io.eCBType = tmp->eCBType;
            vdec_io.pCB = tmp->pCB;
            RPC_PARAM_UPDATE(p3, PARAM_IN,PARAM_Vdec_io_reg_callback_para_rpc,sizeof(Vdec_io_reg_callback_para_rpc), &vdec_io);             
			break;
    	case VDEC_IO_SET_OUTPUT_RECT:
            RPC_PARAM_UPDATE(p3, PARAM_IN,PARAM_Vdecpipinfo_rpc,sizeof(Vdecpipinfo_rpc), param);            
    		break;
		case VDEC_IO_CAPTURE_DISPLAYING_FRAME:
            vdec_pic.type = tmp1->type;
            vdec_pic.out_data_buf = tmp1->out_data_buf;
            vdec_pic.out_data_buf_size = tmp1->out_data_buf_size;
            vdec_pic.out_data_valid_size = tmp1->out_data_valid_size;
            vdec_pic.pic_width = tmp1->pic_width;
            vdec_pic.pic_height = tmp1->pic_height;
            RPC_PARAM_UPDATE(p3, PARAM_IN,PARAM_Vdec_picture_rpc,sizeof(Vdec_picture_rpc), &vdec_pic);            
			break;
		case VDEC_IO_SET_DBG_FLAG:
            RPC_PARAM_UPDATE(p3, PARAM_IN,PARAM_Vdec_io_dbg_flag_info_rpc,sizeof(Vdec_io_dbg_flag_info_rpc), param);
			break;
        case VDEC_IO_SET_TRICK_MODE:
            RPC_PARAM_UPDATE(p3, PARAM_IN,PARAM_Vdec_playback_param_rpc,sizeof(Vdec_playback_param_rpc), param);
            break;
        case VDEC_IO_GET_CAPTURE_FRAME_INFO:
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Vdec_capture_frm_info_rpc, sizeof(Vdec_capture_frm_info_rpc), param);
            break;
		case VDEC_IO_ADPCM_CMD:
		case VDEC_IO_ABS_CMD:
		case VDEC_IO_ERR_CONCEAL_CMD:
		case VDEC_IO_DVIEW_CMD:
		case VDEC_IO_MAF_CMD:
			return RET_FAILURE;
		case VDEC_IO_WHY_IDLE:
		case VDEC_IO_GET_FREEBUF:
		case VDEC_IO_PRIORITY_STOP_SML:
		case VDEC_IO_FILLRECT_INBG:
		case VDEC_IO_COLORBAR:  //only valid when vp had got one framebuffer from ve and vp.mp layer is opened.
		case VDEC_IO_PULLDOWN_ONOFF:
		case VDEC_IO_SWITCH:
		case VDEC_IO_DONT_RESET_SEQ_HEADER: 
		case VDEC_IO_SET_DROP_FRM:
		case VDEC_SET_DMA_CHANNEL:
		case VDEC_DTVCC_PARSING_ENABLE:
		case VDEC_VBV_BUFFER_OVERFLOW_RESET:	
		case VDEC_IO_SET_SYNC_DELAY:
		case VDEC_IO_SAR_ENABLE:
		case VDEC_IO_FIRST_I_FREERUN:
		case VDEC_IO_CONTINUE_ON_ERROR:
		case VDEC_IO_SET_FREERUN_THRESHOLD:
		case VDEC_IO_DBLOCK_ENABLE:
		case VDEC_IO_SET_DEC_FRM_TYPE:
		case VDEC_IO_SET_DMA_CHANNEL_NUMBER:
		case VDEC_IO_SET_BOOTLOGO_ADDR:
		case VDEC_IO_SET_SMOOTH_PREVIEW:
		case VDEC_IO_SET_QUICK_PLAY_MODE:
		case VDEC_IO_SET_STILL_FRAME_MODE:
		case VDEC_IO_CLEAN_STILL_FRAME:	
		case VDEC_IO_SET_FB_COST_DOWN_NUM:
		case VDEC_IO_SET_RGB_OUTPUT_FLAG:
        case VDEC_IO_SET_SIMPLE_SYNC:
			RPC_PARAM_UPDATE(p3, PARAM_IN,PARAM_UINT32,sizeof(UINT32), &param);      			
			break;       	     
		default:
			return RET_FAILURE;
	};   
    Int32 ret;    
    ret = RpcCallCompletion(RPC_vdec_io_control,&p1,&p2,&p3,NULL);
    return ret;
}
EXPORT_SYMBOL(vdec_io_control);


RET_CODE vdec_playmode(struct vdec_device *dev, enum VDecDirection direction, enum VDecSpeed speed)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum VDecDirection), &direction);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_ENUM, sizeof(enum VDecSpeed), &speed);   

    ret = RpcCallCompletion(RPC_vdec_playmode,&p1,&p2,&p3,NULL);
    return ret;   
}

RET_CODE vdec_dvr_set_param(struct vdec_device *dev, struct VDec_DvrConfigParam param)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Vdec_dvrconfigparam_rpc, sizeof(Vdec_dvrconfigparam_rpc), &param);

    ret = RpcCallCompletion(RPC_vdec_dvr_set_param,&p1,&p2,NULL);
    return ret;    
}

RET_CODE vdec_dvr_pause(struct vdec_device *dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    ret = RpcCallCompletion(RPC_vdec_dvr_pause,&p1,NULL);
    return ret;  
}

RET_CODE vdec_dvr_resume(struct vdec_device *dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    ret = RpcCallCompletion(RPC_vdec_dvr_resume,&p1,NULL);
    return ret;   
}

RET_CODE vdec_step(struct vdec_device *dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    ret = RpcCallCompletion(RPC_vdec_step,&p1,NULL);
    return ret;   
}

void h264_decoder_select(int select, BOOL in_preivew)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, 4, &select);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_BOOL, sizeof(BOOL), &in_preivew);

    RpcCallCompletion(RPC_h264_decoder_select,&p1,&p2,NULL);    
}
void video_decoder_select(enum video_decoder_type select, BOOL in_preview)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_ENUM, sizeof(enum video_decoder_type), &select);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_BOOL, sizeof(BOOL), &in_preview);

    RpcCallCompletion(RPC_video_decoder_select,&p1,&p2,NULL);
}
struct vdec_device * get_selected_decoder(void)
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_get_selected_decoder,NULL);
    return (struct vdec_device *)ret;    
}
enum video_decoder_type get_current_decoder(void )
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_get_current_decoder,NULL);

    return (enum video_decoder_type)ret;    
}

BOOL is_cur_decoder_avc(void)
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_is_cur_decoder_avc,NULL);
    return ret;  
}

void set_avc_output_mode_check_cb(VDecCBFunc pCB)
{
    	//jump_to_func(NULL, ali_rpc_call, pCB, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_SET_AVC_OUTPUT_MODE_CHECK_CB, NULL);
    	
    	do{}while(0);
}
#if 0
RET_CODE  vdec_decore_ioctl(void *phandle, int cmd, void *param1, void *param2)
{
	if (VDEC_CMD_INIT == cmd)
	{
		VdecInit *pinit_par = (VdecInit *)param2;
		m_vdec_XD = pinit_par->fmt_in.fourcc;
	}
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &phandle);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &cmd);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param1);            
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param2);            
    Vdecinit_rpc vdec_r;
    VdecInit *p = (VdecInit *)param2;

    switch(cmd)
    {
        case VDEC_CMD_INIT:
            memcpy(&vdec_r.fmt_out, &p->fmt_out, sizeof(Vdecformatinfo_rpc));
            memcpy(&vdec_r.fmt_in, &p->fmt_in, sizeof(Vdecformatinfo_rpc));
            if(p->pfrm_buf != NULL)
            {
                vdec_r.pfrm_buf.FBStride = p->pfrm_buf;          
            }
            else
            {
                memset(&vdec_r.pfrm_buf,0xffff,sizeof(Vdecfbconfig_rpc));
            }
            if(p->phw_mem_cfg != NULL)
            {
                 vdec_r.phw_mem_cfg.fb_max_height = p->phw_mem_cfg;
            }
            else
            {
                memset(&vdec_r.phw_mem_cfg,0,sizeof(Vdechwmemconfig_rpc));
                vdec_r.phw_mem_cfg.vbv_len = 0xffffffff;
            }

            vdec_r.on_decode_event = p->on_decode_event;
            vdec_r.pfn_decore_de_request = p->pfn_decore_de_request;
            vdec_r.pfn_decore_de_release = p->pfn_decore_de_release;
            vdec_r.decoder_flag = p->decoder_flag;
            vdec_r.decode_mode = p->decode_mode;
            vdec_r.preview = p->preview;
            
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param1);            
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_Vdecinit_rpc, sizeof(Vdecinit_rpc), &vdec_r);            
                        
            break;
        case VDEC_CMD_EXTRA_DADA:
            RPC_PARAM_UPDATE(p3, PARAM_BUFFER, PARAM_OPAQUE, (UINT32)param2, param1);            
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param2);            
            break;
        case VDEC_CMD_RELEASE:
        case VDEC_CMD_HW_RESET:
        case VDEC_CMD_PAUSE_DECODE:
        case VDEC_CFG_VIDEO_SBM_BUF:
        case VDEC_CFG_DISPLAY_SBM_BUF:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param1);            
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param2);            
            break;
        case VDEC_CMD_SW_RESET:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param1);            
            RPC_PARAM_UPDATE(p4, PARAM_OUT, PARAM_OutputFrmManager_rpc, sizeof(OutputFrmManager_rpc), param2);
            break;
        case VDEC_CFG_SYNC_MODE:
        case VDEC_CFG_SYNC_THRESHOLD:
        case VDEC_CFG_DECODE_MODE:
            //desc = desc_vdec_decore_uint32;
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_INT32, 4, param1);
            RPC_PARAM_UPDATE(p4, PARAM_OUT, PARAM_INT32, 4, param2);
            break;
        case VDEC_CFG_DISPLAY_RECT:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Video_rect_rpc, sizeof(Video_rect_rpc), param1);
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_Video_rect_rpc, sizeof(Video_rect_rpc), param2);
            break;
        case VDEC_CMD_GET_STATUS:
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Vdec_decore_status_rpc, sizeof(Vdec_decore_status_rpc), param1);
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param2);
            break;
        default:
            return RET_FAILURE;
    }
    
    switch(*(unsigned int *)phandle)
    {
        case h264:
            ret = RpcCallCompletion(RPC_vdec_avc_decode,&p1,&p2,&p3,&p4,NULL);            
            break;
        case xvid:
			if (XD == m_vdec_XD)
			{
				ret = RpcCallCompletion(RPC_vdec_mpeg4_plus_decode,&p1,&p2,&p3,&p4,NULL);
			}
			else
			{
				ret = RpcCallCompletion(RPC_vdec_mpeg4_decode,&p1,&p2,&p3,&p4,NULL);     
            }
			break;
        case mpg2:
		    ret = RpcCallCompletion(RPC_vdec_mpeg2_decode,&p1,&p2,&p3,&p4,NULL);                 
            break;
        case mjpg:
            ret = RpcCallCompletion(RPC_vdec_mjpg_decode,&p1,&p2,&p3,&p4,NULL);     

	   break;
	   case rmvb:
            ret = RpcCallCompletion(RPC_vdec_rv_decode,&p1,&p2,&p3,&p4,NULL);
            break;
	   case vp8:
            ret = RpcCallCompletion(RPC_vdec_vp8_decode,&p1,&p2,&p3,&p4,NULL);
            break;
	//add by phil for support vc1
	   case vc1:
            ret = RpcCallCompletion(RPC_vdec_vc1_decode,&p1,&p2,&p3,&p4,NULL);
            break;
	//phil add end
        default:
            return RET_FAILURE;
    }
    return ret;
    
}
#endif

RET_CODE vdec_decore_ioctl(void *phandle, int cmd, void *param1, void *param2)
{
	if (VDEC_CMD_INIT == cmd)
	{
		VdecInit *pinit_par = (VdecInit *)param2;
		m_vdec_XD = pinit_par->fmt_in.fourcc;
	}
    
    Int32 ret;
    unsigned int codec_tag = *(unsigned int *)phandle;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &codec_tag);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &cmd);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param1);            
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param2);            
    Vdecinit_rpc vdec_r;
    VdecInit *p = (VdecInit *)param2;
    
    switch(cmd)
    {
        case VDEC_CMD_INIT:
            memcpy(&vdec_r.fmt_out, &p->fmt_out, sizeof(Vdecformatinfo_rpc));
            memcpy(&vdec_r.fmt_in, &p->fmt_in, sizeof(Vdecformatinfo_rpc));
            if(p->pfrm_buf != NULL)
            {
                vdec_r.pfrm_buf.FBStride = p->pfrm_buf;          
            }
            else
            {
                memset(&vdec_r.pfrm_buf,0xffff,sizeof(Vdecfbconfig_rpc));
            }
            if(p->phw_mem_cfg != NULL)
            {
                 vdec_r.phw_mem_cfg.fb_max_height = p->phw_mem_cfg;
            }
            else
            {
                memset(&vdec_r.phw_mem_cfg,0,sizeof(Vdechwmemconfig_rpc));
                vdec_r.phw_mem_cfg.vbv_len = 0xffffffff;
            }

            vdec_r.on_decode_event = p->on_decode_event;
            vdec_r.pfn_decore_de_request = p->pfn_decore_de_request;
            vdec_r.pfn_decore_de_release = p->pfn_decore_de_release;
            vdec_r.decoder_flag = p->decoder_flag;
            vdec_r.decode_mode = p->decode_mode;
            vdec_r.preview = p->preview;
            
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param1);            
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_Vdecinit_rpc, sizeof(Vdecinit_rpc), &vdec_r);            
                        
            break;
        case VDEC_CMD_EXTRA_DADA:
            RPC_PARAM_UPDATE(p3, PARAM_BUFFER, PARAM_OPAQUE, (UINT32)param2, param1);            
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param2);            
            break;
        case VDEC_CMD_RELEASE:
        case VDEC_CMD_HW_RESET:
        case VDEC_CMD_PAUSE_DECODE:
        case VDEC_CFG_VIDEO_SBM_BUF:
        case VDEC_CFG_DISPLAY_SBM_BUF:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param1);            
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param2);            
            break;
        case VDEC_CMD_SW_RESET:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param1);            
            RPC_PARAM_UPDATE(p4, PARAM_OUT, PARAM_OutputFrmManager_rpc, sizeof(OutputFrmManager_rpc), param2);
            break;
        case VDEC_ROTATE_FRAME:
        case VDEC_CFG_SYNC_MODE:
        case VDEC_CFG_SYNC_THRESHOLD:
        case VDEC_CFG_DECODE_MODE:
        case VDEC_CFG_FREERUN_TRD:
        case VDEC_CFG_QUICK_MODE:
        case VDEC_DYNAMIC_FRAME_ALLOC:
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_INT32, 4, param1);
            RPC_PARAM_UPDATE(p4, PARAM_OUT, PARAM_INT32, 4, param2);
            break;
        case VDEC_CFG_DISPLAY_RECT:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Video_rect_rpc, sizeof(Video_rect_rpc), param1);
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_Video_rect_rpc, sizeof(Video_rect_rpc), param2);
            break;
        case VDEC_CMD_GET_STATUS:
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Vdec_decore_status_rpc, sizeof(Vdec_decore_status_rpc), param1);
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param2);
            break;
        case VDEC_GET_CAPTURE_FRAME_INFO:
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Vdec_capture_frm_info_rpc, sizeof(Vdec_capture_frm_info_rpc), param1);
            RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &param2);
            break;
        default:
            return RET_FAILURE;
    }
    ret = RpcCallCompletion(RPC_vdec_decore_ioctl,&p1,&p2,&p3,&p4,NULL);  
    return ret;
    
}

void vdec_avs_attach(struct vdec_avs_config_par *pconfig_par)
{
    Vdec_avs_memmap_rpc vdec_avs;

    memcpy(&vdec_avs, &pconfig_par->memmap,sizeof(Vdec_avs_memmap_rpc));

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Vdec_avs_memmap_rpc, sizeof(Vdec_avs_memmap_rpc), &vdec_avs);


    RpcCallCompletion(RPC_vdec_avs_attach,&p1,NULL);

}

