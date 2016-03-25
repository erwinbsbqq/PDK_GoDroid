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
//#include <linux/ali_video.h>
#include <rpc_hld/ali_rpc_hld_decv.h>

#include "../ali_rpc.h"
static unsigned int m_vdec_XD = 0;//add by phil for XD311
//add by phil for boot-media
void boot_media_start(UINT32 addr, UINT32 len)
{
	 jump_to_func(NULL, ali_rpc_call, addr, (BOOT_MEDIA_MODULE<<24)|(2<<16)|FUNC_BOOT_MEDIA_START, NULL);
}

static UINT32 desc_vdec_m36_attach[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct vdec_config_par)),
	1, DESC_P_PARA(0, 0, 0), 
	//desc of pointer ret
	0,                          
	0,
};

void vdec_m36_attach(struct vdec_config_par *pconfig_par)
{
    	jump_to_func(NULL, ali_rpc_call, pconfig_par, (LLD_DECV_M36_MODULE<<24)|(1<<16)|FUNC_VDEC_M36_ATTACH, desc_vdec_m36_attach);
}

RET_CODE vdec_enable_advance_play(struct vdec_device *dev)
{
	
    	jump_to_func(NULL, ali_rpc_call, dev, (LLD_DECV_M36_MODULE<<24)|(1<<16)|FUNC_VDEC_ENABLE_ADVANCE_PLAY, NULL);

}

void vdec_disable_dview(struct vdec_device *dev)
{
    	jump_to_func(NULL, ali_rpc_call, dev, (LLD_DECV_M36_MODULE<<24)|(1<<16)|FUNC_VDEC_DISABLE_DVIEW, NULL);
}

static UINT32 desc_vdec_avc_attach[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct vdec_avc_config_par)),
	1, DESC_P_PARA(0, 0, 0), 
	//desc of pointer ret
	0,                          
	0,
};

void vdec_avc_attach(struct vdec_avc_config_par *pconfig_par)
{
    jump_to_func(NULL, ali_rpc_call, pconfig_par, (LLD_DECV_AVC_MODULE<<24)|(1<<16)|FUNC_VDEC_AVC_ATTACH, desc_vdec_avc_attach);
}

static UINT32 desc_vdec_p_uint32[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, 4),  
	1, DESC_P_PARA(0, 2, 0),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_get_status[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(struct VDec_StatusInfo)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_get_decore_status[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(struct vdec_decore_status)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_get_frm[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(struct vdec_io_get_frm_para)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_fill_frm[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct YCbCrColor)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

UINT32 desc_vdec_reg_callback[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct vdec_io_reg_callback_para)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

/*
static UINT32 desc_vdec_reg_callback[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct vdec_io_reg_callback_para)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};
*/

static UINT32 desc_vdec_vbv_request[] = 
{ //desc of pointer para
	3, DESC_OUTPUT_STRU(0, 4), DESC_OUTPUT_STRU(1, 4), DESC_STATIC_STRU(2, sizeof(struct control_block)),
	3, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1), DESC_P_PARA(2, 4, 2),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_set_output[] = 
{ //desc of pointer para
	3, DESC_STATIC_STRU(0, sizeof(struct VDecPIPInfo)), DESC_OUTPUT_STRU(1, sizeof(struct MPSource_CallBack)), DESC_OUTPUT_STRU(2, sizeof(struct PIPSource_CallBack)), 
	3, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1), DESC_P_PARA(2, 4, 2),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_switch_pip[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct Position)),
	1, DESC_P_PARA(0, 1, 0), 
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_switch_pip_ext[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct Rect)),
	1, DESC_P_PARA(0, 1, 0), 
	//desc of pointer ret
	0,                          
	0,
};

/*
static UINT32 desc_vdec_dvr_set_param[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct VDec_DvrConfigParam)),
	1, DESC_P_PARA(0, 1, 0), 
	//desc of pointer ret
	0,                          
	0,
};
*/

static UINT32 desc_vdec_set_output_rect[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct VDecPIPInfo)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};


static UINT32 desc_vdec_capture_display[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(struct vdec_picture)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_io_dbg_flag_info[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct vdec_io_dbg_flag_info)),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

RET_CODE vdec_open(struct vdec_device *dev)
{

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_OPEN, NULL);


}

RET_CODE vdec_close(struct vdec_device *dev)
{
	

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_CLOSE, NULL);  


}

RET_CODE vdec_start(struct vdec_device *dev)
{
	

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_START, NULL);


}

RET_CODE vdec_stop(struct vdec_device *dev,BOOL bclosevp,BOOL bfillblack)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_STOP, NULL);


}

RET_CODE vdec_vbv_request(void *dev, UINT32 uSizeRequested, void ** ppVData, UINT32 * puSizeGot, struct control_block * ctrl_blk)
{
	

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(5<<16)|FUNC_VDEC_VBV_REQUEST, desc_vdec_vbv_request);


}

void vdec_vbv_update(void *dev, UINT32 uDataSize)
{
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_VDEC_VBV_UPDATE, NULL);
}

RET_CODE vdec_set_output(struct vdec_device *dev, enum VDecOutputMode eMode,struct VDecPIPInfo *pInitInfo, 	struct MPSource_CallBack *pMPCallBack, struct PIPSource_CallBack *pPIPCallBack)
{


    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(5<<16)|FUNC_VDEC_SET_OUTPUT, desc_vdec_set_output);


}

RET_CODE vdec_switch_pip(struct vdec_device *dev, struct Position *pPIPPos)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_VDEC_SWITCH_PIP, desc_vdec_switch_pip);


}

RET_CODE vdec_switch_pip_ext(struct vdec_device *dev, struct Rect *pPIPWin)
{
	

	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_VDEC_SWITCH_PIP_EXT, desc_vdec_switch_pip_ext);


}

RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 uSyncMode,UINT8 uSyncLevel)
{

	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_SYNC_MODE, NULL);


}

RET_CODE vdec_extrawin_store_last_pic(struct vdec_device *dev, struct Rect *pPIPWin)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_VDEC_EXTRAWIN_STORE_LAST_PIC, desc_vdec_switch_pip_ext);


}

RET_CODE vdec_profile_level(struct vdec_device *dev, UINT8 uProfileLevel,VDEC_BEYOND_LEVEL cb_beyond_level)
{
	

    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_PROFILE_LEVEL, NULL);


}

RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param)
{

	UINT32 *desc = NULL;

	switch(io_code)
	{
		case VDEC_IO_GET_STATUS:
			desc = desc_vdec_get_status;
			break;
        case VDEC_IO_GET_DECORE_STATUS:
            desc = desc_vdec_get_decore_status;
            break;
		case VDEC_IO_GET_MODE:
			desc = desc_vdec_p_uint32;
			break;
		case VDEC_IO_GET_OUTPUT_FORMAT:
			desc = desc_vdec_p_uint32;
			break;
		case VDEC_IO_GET_FRM:
			desc = desc_vdec_get_frm;
			break;	
		case VDEC_IO_FILL_FRM:
		case VDEC_IO_FILL_FRM2:
			desc = desc_vdec_fill_frm;
			break;
		case VDEC_IO_REG_CALLBACK:		
			desc = desc_vdec_reg_callback;
			break;
        	case VDEC_IO_SET_OUTPUT_RECT:
            		desc = desc_vdec_set_output_rect;
            		break;
		case VDEC_IO_CAPTURE_DISPLAYING_FRAME:
			desc = desc_vdec_capture_display;
			break;
		case VDEC_IO_SET_DBG_FLAG:
			desc = desc_vdec_io_dbg_flag_info;
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
			break;       	     
		default:
			return RET_FAILURE;
	};
	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_IO_CONTROL, desc);
	
}

RET_CODE vdec_playmode(struct vdec_device *dev, enum VDecDirection direction, enum VDecSpeed speed)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(3<<16)|FUNC_VDEC_PLAYMODE, NULL);


}

RET_CODE vdec_dvr_set_param(struct vdec_device *dev, struct VDec_DvrConfigParam param)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_VDEC_DVR_SET_PARAM, NULL);


}

RET_CODE vdec_dvr_pause(struct vdec_device *dev)
{


	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_DVR_PAUSE, NULL);


}

RET_CODE vdec_dvr_resume(struct vdec_device *dev)
{

	
    	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_DVR_RESUME, NULL);


}

RET_CODE vdec_step(struct vdec_device *dev)
{

	
	jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_VDEC_STEP, NULL);


}

void h264_decoder_select(int select, BOOL in_preivew)
{
    	jump_to_func(NULL, ali_rpc_call, select, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_H264_DECODER_SELECT, NULL);
}
void video_decoder_select(enum video_decoder_type select, BOOL in_preview)
{
    	jump_to_func(NULL, ali_rpc_call, select, (HLD_DECV_MODULE<<24)|(2<<16)|FUNC_VIDEO_DECODER_SELECT, NULL);
}
struct vdec_device * get_selected_decoder(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (HLD_DECV_MODULE<<24)|(0<<16)|FUNC_GET_SELECTED_DECODER, NULL);


}
enum video_decoder_type get_current_decoder(void )
{


	jump_to_func(NULL, ali_rpc_call, null, (HLD_DECV_MODULE<<24)|(0<<16)|FUNC_GET_CURRENT_DECODER, NULL);


}	
BOOL is_cur_decoder_avc(void)
{

	
    jump_to_func(NULL, ali_rpc_call, null, (HLD_DECV_MODULE<<24)|(0<<16)|FUNC_IS_CUR_DECODER_AVC, NULL);
	

}

void set_avc_output_mode_check_cb(VDecCBFunc pCB)
{
    	//jump_to_func(NULL, ali_rpc_call, pCB, (HLD_DECV_MODULE<<24)|(1<<16)|FUNC_SET_AVC_OUTPUT_MODE_CHECK_CB, NULL);
    	
    	do{}while(0);
}
static UINT32 desc_vdec_cmd_init[] = 
{   //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(VdecInit)),
	1, DESC_P_PARA(0, 3, 0), 
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_cmd_sw_reset[] = 
{   //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(struct OutputFrmManager)),
	1, DESC_P_PARA(0, 3, 0),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_display_rect[] = 
{   //desc of pointer para
	2, DESC_STATIC_STRU(0, sizeof(struct Video_Rect)), DESC_STATIC_STRU(1, sizeof(struct Video_Rect)),
	2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_cmd_get_status[] = 
{   //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(struct vdec_decore_status)),
	1, DESC_P_PARA(0, 2, 0),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_decore_uint32[] = 
{   //desc of pointer para
	2, DESC_OUTPUT_STRU(0, 4), DESC_OUTPUT_STRU(0, 4),
	2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_vdec_cmd_extra_data[] = 
{   //desc of pointer para
	1, DESC_OUTPUT_STRU(0, MAX_VIDEO_RPC_ARG_SIZE),
	1, DESC_P_PARA(0, 2, 0),
	//desc of pointer ret
	0,                          
	0,
};

RET_CODE vdec_decore_ioctl(void *phandle, int cmd, void *param1, void *param2)
{

    UINT32 func_desc;
	UINT32 *desc = NULL;
	if (VDEC_CMD_INIT == cmd)
	{
		VdecInit *pinit_par = (VdecInit *)param2;
		m_vdec_XD = pinit_par->fmt_in.fourcc;
	}
    switch(*(unsigned int *)phandle)
    {
        case h264:
            func_desc = ((LLD_DECV_AVC_MODULE<<24)|(4<<16)|FUNC_VDEC_AVC_DECODE);
            break;
        case xvid:
			if (XD == m_vdec_XD)
			{
				func_desc = ((LIB_MP4DEC_MODULE<<24)|(4<<16)|FUNC_VDEC_MPEG4_PLUS_DECODE);
				//printk("it is XD311!!\n");
			}
			else
			{
            	func_desc = ((LIB_MP4DEC_MODULE<<24)|(4<<16)|FUNC_VDEC_MPEG4_DECODE);
            }
			break;
        case mpg2:
            func_desc = ((LLD_DECV_M36_MODULE<<24)|(4<<16)|FUNC_VDEC_MPEG2_DECODE);
            break;
        case mjpg:
	   func_desc = ((LIB_MJPGDEC_MODULE<<24)|(4<<16)|FUNC_VDEC_MJPG_DECODE);
	   break;
	   case rmvb:
            func_desc = ((LIB_RVDEC_MODULE<<24)|(4<<16)|FUNC_VDEC_RV_DECODE);
            break;
	   case vp8:
            func_desc = ((LIB_VP8DEC_MODULE<<24)|(4<<16)|FUNC_VDEC_VP8_DECODE);
            break;
	//add by phil for support vc1
	   case vc1:
	   		//printk("it is vc1 decoder!!\n");
	   	 	func_desc = ((LIB_VC1DEC_MODULE<<24)|(4<<16)|FUNC_VDEC_VC1_DECODE);
            break;
	//phil add end
        default:
            return RET_FAILURE;
    }
    
    switch(cmd)
    {
        case VDEC_CMD_INIT:
            desc = desc_vdec_cmd_init;
            break;
        case VDEC_CMD_EXTRA_DADA:
            desc = desc_vdec_cmd_extra_data;
            DESC_OUTPUT_STRU_SET_SIZE(desc, 0, (UINT32)param2);
            break;
        case VDEC_CMD_RELEASE:
        case VDEC_CMD_HW_RESET:
        case VDEC_CMD_PAUSE_DECODE:
        case VDEC_CFG_VIDEO_SBM_BUF:
        case VDEC_CFG_DISPLAY_SBM_BUF:
            break;
        case VDEC_CMD_SW_RESET:
            desc = desc_vdec_cmd_sw_reset;
            break;
        case VDEC_CFG_SYNC_MODE:
        case VDEC_CFG_SYNC_THRESHOLD:
        case VDEC_CFG_DECODE_MODE:
            desc = desc_vdec_decore_uint32;
            break;
        case VDEC_CFG_DISPLAY_RECT:
            desc = desc_vdec_display_rect;
            break;
        case VDEC_CMD_GET_STATUS:
            desc = desc_vdec_cmd_get_status;
            break;
        default:
            return RET_FAILURE;
    }

    jump_to_func(NULL, ali_rpc_call, phandle, func_desc, desc);  


}

static UINT32 desc_vdec_avs_attach[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, sizeof(struct vdec_avs_config_par)),
	1, DESC_P_PARA(0, 0, 0), 
	//desc of pointer ret
	0,                          
	0,
};
void vdec_avs_attach(struct vdec_avs_config_par *pconfig_par)
{
    jump_to_func(NULL, ali_rpc_call, pconfig_par, (LLD_DECV_AVS_MODULE<<24)|(1<<16)|FUNC_VDEC_AVS_ATTACH, desc_vdec_avs_attach);
}

