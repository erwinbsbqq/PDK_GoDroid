/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: ali_video_rpc.c
 *  (I)
 *  Description: rpc hld decv opeartion
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.04.09				Sam		Create
 ****************************************************************************/
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/div64.h>
#include <asm/mach-ali/typedef.h>
#include "ali_video.h"

extern struct ali_video_info *ali_video_priv;
extern struct vdec_device *g_decv_dev0;
extern struct vdec_device *g_decv_avc_dev0;
extern struct vdec_device *g_decv_avs_dev0;

/*
vdec type definition:
0 -- mpeg2
1 -- h.264
2 -- avs
3 -- vc1
4 -- mpeg4
5 -- vp8
6 -- rv
7 -- mjpeg
*/

/*
	Name : hld_decv_set_cur_dev
	Description : set the current device for the decv
	Argement :
		int dev --> dev index
			0 : mpg decv0
			1 : avc decv0
			...
*/
void hld_decv_set_cur_dev(struct ali_video_info *info, int dev)
{
	switch(dev)
	{
		case 0:
			info->cur_dev = g_decv_dev0;
			info->vdec_type = MPEG2_DECODER;
			break;
		case 1:
			info->cur_dev = g_decv_avc_dev0;
            info->vdec_type = H264_DECODER;
			break;
		case 2:
			info->cur_dev = g_decv_avs_dev0;
            info->vdec_type = AVS_DECODER;
		default:
			break;
	}
	VDEC_PRINTF("video set device 0x%x id %d\n", (int)info->cur_dev, dev);
}

int hld_decv_request_buf(struct ali_video_info *info, void *instance,
                            void **buf_start, int *buf_size, struct ctrl_blk *blk)
{
	RET_CODE ret = 0;
	UINT32 size_request = (UINT32)(*buf_size);

	ret = vdec_vbv_request(info->cur_dev, size_request, buf_start, (UINT32 *)buf_size, (struct control_block *)blk);
	
	if(ret == RET_SUCCESS)
		ret = ALI_DECV_REQ_RET_OK;
	else if(ret == RET_FAILURE)
		ret = ALI_DECV_REQ_RET_FAIL;
	else 
		ret = ALI_DECV_REQ_RET_ERROR;
	
	return ret;
}

void hld_decv_update_buf(struct ali_video_info *info, void *instance, int buf_size)
{
	vdec_vbv_update(info->cur_dev, buf_size);
}

void hld_decv_get_out_info(struct ali_video_info *info, struct ali_video_out_info_pars *pars)
{
	struct VDec_StatusInfo status_info;

	memset((void *)pars, 0, sizeof(*pars));
	if(RET_SUCCESS == vdec_io_control(info->cur_dev, VDEC_IO_GET_STATUS, (UINT32)&status_info)) {
		pars->started = (status_info.uCurStatus == VDEC27_STARTED) ? 1 : 0;
		if(pars->started == 0) {
			return;
		}
		
		pars->flag |= status_info.uFirstPicShowed == TRUE ? ALI_VIDEO_OUT_INFO_FLAG_FIRST_SHOWED : 0;
		if(status_info.uFirstPicShowed == TRUE) {
			pars->width = status_info.pic_width;
			pars->height = status_info.pic_height;
			pars->frame_rate = status_info.frame_rate;
			pars->display_idx = status_info.display_idx;
			pars->is_support = status_info.is_support;
            #if 0					
			pars->read_p_offset = 0;
			pars->write_p_offset = 0;
			pars->valid_size = 0;
            #endif				
		}	
	
		return;
	}

	pars->started = 0;
	return;
}

static void decv_pcb_first_showed(UINT32 uParam1, UINT32 uParam2)
{	
    INT32 dst_pid = ali_video_priv->dst_pid;
	UINT8 msg[4];

	msg[0] = MSG_FIRST_SHOWED;
	msg[1] = 2;
	msg[2] = uParam1;
	msg[3] = uParam2;
	ali_transport_send_msg(dst_pid, (void *)msg, 4);

    VDEC_PRINTF("video first picture showed\n");
}

static void decv_pcb_mode_switch_ok(UINT32 uParam1, UINT32 uParam2)
{
    INT32 dst_pid = ali_video_priv->dst_pid;
	UINT8 msg[4];

	msg[0] = MSG_MODE_SWITCH_OK;
	msg[1] = 2;
	msg[2] = uParam1;
	msg[3] = uParam2;
	ali_transport_send_msg(dst_pid, (void *)msg, 4);
}

static void decv_pcb_backward_reset_gop(UINT32 uParam1, UINT32 uParam2)
{
    INT32 dst_pid = ali_video_priv->dst_pid;
	UINT8 msg[4];

	msg[0] = MSG_BACKWARD_RESTART_GOP;
	msg[1] = 2;
	msg[2] = uParam1;
	msg[3] = uParam2;
	ali_transport_send_msg(dst_pid, (void *)msg, 4);
}

static void decv_pcb_first_header_parsed(UINT32 uParam1, UINT32 uParam2)
{
    INT32 dst_pid = ali_video_priv->dst_pid;
	UINT8 msg[4];

	msg[0] = MSG_FIRST_HEADRE_PARSED;
	msg[1] = 2;
	msg[2] = uParam1;
	msg[3] = uParam2;
	ali_transport_send_msg(dst_pid, (void *)msg, 4);

    VDEC_PRINTF("video first header parsed\n");
}

static void decv_pcb_first_i_decoded(UINT32 uParam1, UINT32 uParam2)
{	
    INT32 dst_pid = ali_video_priv->dst_pid;
	UINT8 msg[4];

	msg[0] = MSG_FIRST_I_DECODED;
	msg[1] = 2;
	msg[2] = uParam1;
	msg[3] = uParam2;
	ali_transport_send_msg(dst_pid, (void *)msg, 4);

    VDEC_PRINTF("video first i decoded\n");
}

static void decv_pcb_user_data_parsed(UINT32 param1, UINT32 param2)
{	
    INT32 dst_pid = ali_video_priv->dst_pid;
	UINT8 msg[4+DTVCC_USER_DATA_LENGTH_MAX];
    UINT32 size = param1;
    UINT8 *buf = (UINT8 *)param2;

    if(size > DTVCC_USER_DATA_LENGTH_MAX)
    {
        return;
    }

	msg[0] = MSG_USER_DATA_PARSED;
	msg[1] = 2;
	msg[2] = (UINT8)size;
	msg[3] = (UINT8)(size>>8);
    memcpy(&msg[4], buf, (UINT16)size);
	ali_transport_send_msg(dst_pid, (void *)msg, 4 + size);
}

static void vdec_cb_spec_routine(UINT32 param1, UINT32 param2)
{
    struct vdec_callback *vdec_cb = &(ali_video_priv->call_back);

    if(vdec_cb->pcb_vdec_user_data_parsed)
    {
        vdec_cb->pcb_vdec_user_data_parsed(param1, param2);
    }
}

static void vdec_cb_routine(UINT32 res, UINT32 uParam)
{
    struct vdec_callback *vdec_cb = &(ali_video_priv->call_back);
	UINT8 type = uParam&0xff;
	UINT8 uParam1 = (uParam>>8)&0xff;
	UINT8 uParam2 = (uParam>>16)&0xff;

	VDEC_PRINTF("video callback type %u\n", type);	
    
	switch(type)
	{
		case VDEC_CB_FIRST_SHOWED:
			if(vdec_cb->pcb_first_showed)
				vdec_cb->pcb_first_showed(uParam1, uParam2);
			break;
		case VDEC_CB_MODE_SWITCH_OK:
			if(vdec_cb->pcb_mode_switch_ok)
				vdec_cb->pcb_mode_switch_ok(uParam1, uParam2);
			break;
		case VDEC_CB_BACKWARD_RESTART_GOP: // used for advance play
			if(vdec_cb->pcb_backward_restart_gop)
				vdec_cb->pcb_backward_restart_gop(uParam1, uParam2);
			break;
		case VDEC_CB_FIRST_HEAD_PARSED:
			if(vdec_cb->pcb_first_head_parsed)
				vdec_cb->pcb_first_head_parsed(uParam1, uParam2);
			break;
        case VDEC_CB_FIRST_I_DECODED:
			if(vdec_cb->pcb_first_i_decoded)
				vdec_cb->pcb_first_i_decoded(uParam1, uParam2);
			break; 
		default:
			break;
	}
}

static int vdec_capture_frame(struct ali_video_info *info, struct vdec_picture *ppic)
{
    struct vdec_capture_frm_info frm_info;
    unsigned char *yuv_y = NULL;
    unsigned char *yuv_u = NULL;
    unsigned char *yuv_v = NULL;
    int ret = RET_FAILURE;

    if(!info->capture_mem_addr || !info->capture_mem_size)
    {
        return ret;
    }

    memset(&frm_info, 0, sizeof(frm_info));

    if(info->work_mode)
    {
        ret = vdec_decore_ioctl((void *)&info->codec_tag, VDEC_GET_CAPTURE_FRAME_INFO, (void*)&frm_info, NULL);
    }
    else
    {
        ret = vdec_io_control(info->cur_dev, VDEC_IO_GET_CAPTURE_FRAME_INFO, (UINT32)&frm_info);
    }
    
    if(ret == RET_SUCCESS)
    {
        yuv_y = (unsigned char *)__VSTMALI(info->capture_mem_addr);
        yuv_u = yuv_y + frm_info.y_buf_size;
        yuv_v = yuv_u + (frm_info.y_buf_size>>2);
    
        if(frm_info.de_map_mode)
        {
            convert_h264_de2yuv((unsigned char *)__VSTMALI(frm_info.y_buf_addr), \
                                (unsigned char *)__VSTMALI(frm_info.c_buf_addr), \
                                frm_info.pic_width, frm_info.pic_height, \
                                yuv_y, yuv_u, yuv_v, \
                                frm_info.pic_stride, frm_info.pic_stride>>1);
        }
        else
        {
            convert_mpeg2_de2yuv((unsigned char *)__VSTMALI(frm_info.y_buf_addr), \
                                 (unsigned char *)__VSTMALI(frm_info.c_buf_addr), \
                                 frm_info.pic_width, frm_info.pic_height, \
                                 yuv_y, yuv_u, yuv_v, \
                                 frm_info.pic_stride, frm_info.pic_stride>>1);
        }

        ppic->type = VDEC_PIC_YV12;
        ppic->pic_width  = frm_info.pic_width;
        ppic->pic_height = frm_info.pic_height;
        ppic->out_data_valid_size = frm_info.y_buf_size + (frm_info.y_buf_size>>1);
        if(ppic->out_data_buf && ppic->out_data_valid_size \
           && (ppic->out_data_buf_size >= ppic->out_data_valid_size)) 
        {
            if(copy_to_user((char *)ppic->out_data_buf, (char *)yuv_y, ppic->out_data_valid_size))
            {
                ret = RET_FAILURE;
            }
        } 
        else 
        {
            ret = RET_FAILURE;
        }

        VDEC_PRINTF("video capture frame width %lu height %lu map %u\n", 
            ppic->pic_width, ppic->pic_height, frm_info.de_map_mode);
    }

    return ret;
}

static int vdec_get_multiview_buf(struct ali_video_info *info)
{
#if 0
    struct multiview_info muti_info;
    int ret = RET_FAILURE;
    multiview_buf_attr_t *multiview_buf_attr = (multiview_buf_attr_t *)request_attr(MULTIVIEW_BUF);
    
    muti_info.full_screen_height = 1080;
	muti_info.full_screen_width = 1920;
	muti_info.multiview_buf = multiview_buf_attr->multiview_buf_start_addr;
		
	ret = vdec_io_control(g_decv_dev0, VDEC_IO_GET_MULTIVIEW_BUF,(unsigned long)&muti_info);
    
	ret = vdec_io_control(g_decv_avc_dev0, VDEC_IO_GET_MULTIVIEW_BUF,(unsigned long)&muti_info);

    return ret;
#endif
    return 0;
}

static int io_control(struct ali_video_info *info)
{
    int ret = RET_FAILURE;

    switch(*(unsigned long *)(info->rpc_arg[0]))
    {
        case VDEC_IO_GET_STATUS:
        case VDEC_IO_GET_DECORE_STATUS:
        case VDEC_IO_FILL_FRM:
        case VDEC_IO_SET_OUTPUT_RECT:
        case VDEC_IO_RESERVE_MEMORY:
        case VDEC_IO_FILL_BG_VIDEO:
            ret = vdec_io_control(info->cur_dev, *(unsigned long *)(info->rpc_arg[0]), (unsigned long)(info->rpc_arg[1]));
            break;
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
        case VDEC_IO_BG_FILL_BLACK:
        case VDEC_IO_SET_MULTIVIEW_WIN:
            ret = vdec_io_control(info->cur_dev, *(unsigned long *)(info->rpc_arg[0]), *(unsigned long *)(info->rpc_arg[1]));
            break;		
        case VDEC_IO_CAPTURE_DISPLAYING_FRAME:
            ret = vdec_capture_frame(info, (struct vdec_picture *)info->rpc_arg[1]);
            break;
        case VDEC_IO_GET_MULTIVIEW_BUF:
            ret = vdec_get_multiview_buf(info);
            break;
        default:
            break;			
    }

    return ret;
}

static int avc_callback_init = 0;
static int avs_callback_init = 0;
static void init_avc_callback(void)
{
    struct vdec_callback *vdec_cb = &(ali_video_priv->call_back);
	struct vdec_io_reg_callback_para para;
		
	if(avc_callback_init == 0)
	{
		avc_callback_init = 1;

		para.eCBType = VDEC_CB_FIRST_SHOWED;
		para.pCB = vdec_cb->pcb_first_showed;
		vdec_io_control(g_decv_avc_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);

		para.eCBType = VDEC_CB_BACKWARD_RESTART_GOP;
		para.pCB = vdec_cb->pcb_backward_restart_gop;		
		vdec_io_control(g_decv_avc_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);

        para.eCBType = VDEC_CB_FIRST_I_DECODED;
		para.pCB = vdec_cb->pcb_first_i_decoded;
		vdec_io_control(g_decv_avc_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);
	}
}

static void init_avs_callback(void)
{
    struct vdec_callback *vdec_cb = &(ali_video_priv->call_back);
	struct vdec_io_reg_callback_para para;
		
	if(avs_callback_init == 0)
	{
		avs_callback_init = 1;

		para.eCBType = VDEC_CB_FIRST_SHOWED;
		para.pCB = vdec_cb->pcb_first_showed;
		vdec_io_control(g_decv_avs_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);

		para.eCBType = VDEC_CB_BACKWARD_RESTART_GOP;
		para.pCB = vdec_cb->pcb_backward_restart_gop;		
		vdec_io_control(g_decv_avs_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);		
	}
}

static void switch_working_mode(struct ali_video_info *info)
{
    enum video_decoder_type decoder = get_current_decoder();
    
    if(info->work_mode) {
        vdec_decore_ioctl((void *)&info->codec_tag, VDEC_CMD_RELEASE, 0, 0);
        
        if(info->cur_dev == g_decv_dev0 && decoder != MPEG2_DECODER)
        {
            decoder = MPEG2_DECODER;
            video_decoder_select(decoder, 0);
            hld_decv_set_cur_dev(info, decoder);
        }
        else if(info->cur_dev == g_decv_avc_dev0 && decoder != H264_DECODER)
        {
            decoder = H264_DECODER;
            video_decoder_select(decoder, 0);
            hld_decv_set_cur_dev(info, decoder);
        }
        else if(info->cur_dev == g_decv_avs_dev0 && decoder != AVS_DECODER)
        {
            decoder = AVS_DECODER;
            video_decoder_select(decoder, 0);
            hld_decv_set_cur_dev(info, decoder);
        }
            
        info->work_mode = 0;
        info->codec_tag = 0;
        info->pause_decode  = 0;
        info->pause_display = 0;

        VDEC_PRINTF("video %u switch working mode to live play\n", decoder);
    }
}

static void rpc_set_dbg_flag(unsigned int *par)
{
	struct vdec_io_dbg_flag_info flag_info;

	memset((void *)&flag_info, 0, sizeof(flag_info));

	flag_info.dbg_flag = par[0];			
	
	if(par[1])
		flag_info.active_flag = TRUE;
	else
		flag_info.active_flag = FALSE;

	if(par[2])
		flag_info.unique_flag = TRUE;
	else
		flag_info.unique_flag = FALSE;
	
	vdec_io_control(ali_video_priv->cur_dev, VDEC_IO_SET_DBG_FLAG, (UINT32)&flag_info);
}

int hld_decv_rpc_operation(struct ali_video_info *info, int API_idx)
{
        enum video_decoder_type decoder = MPEG2_DECODER;
        int ret = RET_SUCCESS;

        //VDEC_PRINTF("hld decv rpc operation api %d\n\n", API_idx);
        
        switch(API_idx)
        {
        	case RPC_VIDEO_START:
                switch_working_mode(info);
        		ret = vdec_start(info->cur_dev);
        		break;
        	case RPC_VIDEO_STOP:
        		ret = vdec_stop(info->cur_dev, *(unsigned long *)(info->rpc_arg[0]), *(unsigned long *)(info->rpc_arg[1]));
        		break;
        	case RPC_VIDEO_SET_OUT:
        		ret = vdec_set_output(info->cur_dev, *(unsigned long *)(info->rpc_arg[0]),
        			                  (struct VDecPIPInfo *)(info->rpc_arg[1]), 
        			                  (struct MPSource_CallBack *)(info->rpc_arg[2]),
        			                  (struct PIPSource_CallBack *)(info->rpc_arg[3]));
        		break;
        	case RPC_VIDEO_SYNC_MODE:
        		ret = vdec_sync_mode(info->cur_dev, *(unsigned long *)(info->rpc_arg[0]), 
                                     *(unsigned long *)(info->rpc_arg[1]));
        		break;
        	case RPC_VIDEO_IO_CONTROL:	
        		ret = io_control(info);
        		break;
        	case RPC_VIDEO_PLAY_MODE:
        		ret = vdec_playmode(info->cur_dev, *(unsigned long *)(info->rpc_arg[0]), 
                                    *(unsigned long *)(info->rpc_arg[1]));
        		break;
        	case RPC_VIDEO_DVR_SET_PAR:
        		ret = vdec_dvr_set_param(info->cur_dev, *(struct VDec_DvrConfigParam *)(info->rpc_arg[0]));
        		break;
        	case RPC_VIDEO_DVR_PAUSE:
        		ret = vdec_dvr_pause(info->cur_dev);
        		break;
        	case RPC_VIDEO_DVR_RESUME:
        		ret = vdec_dvr_resume(info->cur_dev);
        		break;
            case RPC_VIDEO_DVR_STEP:
                ret = vdec_step(info->cur_dev);
                break;
        	case RPC_VIDEO_SELECT_DEC:
                hld_decv_set_cur_dev(info, *(enum video_decoder_type *)(info->rpc_arg[0]));
        		h264_decoder_select(*(int *)(info->rpc_arg[0]), *(BOOL *)(info->rpc_arg[1]));
        		if(*(unsigned long *)(info->rpc_arg[0])) {
        			init_avc_callback();
        		}
        		break;
            case RPC_VIDEO_DECODER_SELECT_NEW:
                hld_decv_set_cur_dev(info, *(enum video_decoder_type *)(info->rpc_arg[0]));
    			video_decoder_select(*(enum video_decoder_type *)(info->rpc_arg[0]), *(BOOL *)(info->rpc_arg[1]));
    			if(*(unsigned long *)(info->rpc_arg[0]) == 1)
    				init_avc_callback();
    			else if(*(unsigned long *)(info->rpc_arg[0]) == 2)
    				init_avs_callback();
    			break;
        	case RPC_VIDEO_IS_AVC:
        		ret = is_cur_decoder_avc();
        		if(ret == TRUE && info->cur_dev == g_decv_dev0) {
                    hld_decv_set_cur_dev(info, H264_DECODER);
                    VDEC_PRINTF("switch to H264 decoder\n");
        		}
        		else if(ret != TRUE && info->cur_dev == g_decv_avc_dev0) {
        			hld_decv_set_cur_dev(info, MPEG2_DECODER);
                    VDEC_PRINTF("switch to mpeg2 decoder\n");
        		}
        			
        		return ret;
            case RPC_VIDEO_GET_DECODER:
    			ret = get_current_decoder();
    			if(ret == H264_DECODER && info->cur_dev != g_decv_avc_dev0)
    			{
    				VDEC_PRINTF("%s : switch to H264 decoder\n", __FUNCTION__);
    				hld_decv_set_cur_dev(info, H264_DECODER);
    			}
    			else if(ret == MPEG2_DECODER && info->cur_dev != g_decv_dev0)
    			{
    				VDEC_PRINTF("%s : switch to mpeg2 decoder\n", __FUNCTION__);
    				hld_decv_set_cur_dev(info, MPEG2_DECODER);
    			}
    			else if(ret == AVS_DECODER && info->cur_dev != g_decv_avs_dev0)
    			{
    				VDEC_PRINTF("%s : switch to AVS decoder\n", __FUNCTION__);
    				hld_decv_set_cur_dev(info, AVS_DECODER);
    			}

    			return ret;
            case RPC_VIDEO_DECORE_IOCTL:
                if(*(int *)info->rpc_arg[1] == VDEC_CFG_VIDEO_SBM_BUF
                   || *(int *)info->rpc_arg[1] == VDEC_CFG_DISPLAY_SBM_BUF
                   || *(int *)info->rpc_arg[1] == VDEC_CMD_PAUSE_DECODE) {
                    ret = vdec_decore_ioctl((void*)info->rpc_arg[0], *(int *)info->rpc_arg[1],
                                            (void*)*(int *)info->rpc_arg[2], (void*)*(int *)info->rpc_arg[3]);
                    
                    if(*(int *)info->rpc_arg[1] == VDEC_CMD_PAUSE_DECODE) {
                        if(*(int *)info->rpc_arg[2] > 0)
                            info->pause_decode = 1;
                        else if(*(int *)info->rpc_arg[2] == 0)
                            info->pause_decode = 0;

                        if(*(int *)info->rpc_arg[3] > 0)
                            info->pause_display = 1;
                        else if(*(int *)info->rpc_arg[3] == 0)
                            info->pause_display = 0;
                    }
                } else if(*(int *)info->rpc_arg[1] == VDEC_CMD_EXTRA_DADA) {
                    ret = vdec_decore_ioctl((void*)info->rpc_arg[0], *(int *)info->rpc_arg[1],
                                            (void*)info->rpc_arg[2], (void*)*(int *)info->rpc_arg[3]);
                    return ret;
                } else if(*(int *)info->rpc_arg[1] == VDEC_CMD_SW_RESET) {
                    ret = vdec_decore_ioctl((void*)info->rpc_arg[0], *(int *)info->rpc_arg[1],
                                            (void*)*(int *)info->rpc_arg[2], (void*)info->rpc_arg[3]);
                } else if(*(int *)info->rpc_arg[1] == VDEC_CAPTURE_FRAME) {
                    ret = vdec_capture_frame(info, (struct vdec_picture *)info->rpc_arg[2]);
                } else {
                    if(*(int *)info->rpc_arg[1] == VDEC_CMD_INIT) {
                        info->work_mode = 1;
                        info->codec_tag = *(unsigned int *)info->rpc_arg[0];
                        info->pause_decode  = 0;
                        info->pause_display = 0;

                        switch(info->codec_tag)
                        {
                            case mpg2:
                                info->vdec_type = MPEG2_DECODER;
                                break;
                            case h264:
                                info->vdec_type = H264_DECODER;
                                break;
                            case vc1:
                                info->vdec_type = 3;
                                break;
                            case xvid:
                                info->vdec_type = 4;
                                break;
                            case vp8:
                                info->vdec_type = 5;
                                break;
                            case rmvb:
                                info->vdec_type = 6;
                                break;
                            case mjpg:
                                info->vdec_type = 7;
                                break;
                            default:
                                break;							
                        }
                    }
                    ret = vdec_decore_ioctl((void*)info->rpc_arg[0], *(int *)info->rpc_arg[1],
                                            (void*)info->rpc_arg[2], (void*)info->rpc_arg[3]);

                    if(*(int *)info->rpc_arg[1] == VDEC_CMD_RELEASE) {
                        info->work_mode = 0;
                        info->codec_tag = 0;
                        info->pause_decode  = 0;
                        info->pause_display = 0;
                        decoder = get_current_decoder();
                        hld_decv_set_cur_dev(info, decoder);                        
                    }
                }
                break;
            case RPC_VIDEO_SET_DBG_FLAG:
    		{
    			rpc_set_dbg_flag((unsigned int *)info->rpc_arg[0]);
    			break;
    		}
        	default:
        		ret = -1;
        		break;
        }

        if(ret != RET_SUCCESS) {
        	ret = -1;
        }
        return ret;
}

void hld_decv_rpc_suspend(struct ali_video_info *info)
{
	if(info->work_mode) {
		vdec_dvr_pause(info->cur_dev);		
	} else {
	    vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CMD_PAUSE_DECODE, (void *)1, (void *)1);
	}
}

void hld_decv_rpc_resume(struct ali_video_info *info)
{
	if(info->pause_decode)
		return;
	
	if(info->work_mode) {
		vdec_dvr_resume(info->cur_dev);		
	} else {
	    vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CMD_PAUSE_DECODE, (void *)0, (void *)0);
	}		
}

void hld_decv_rpc_release(struct ali_video_info *info, int force)
{
	if((info->work_mode == 1) || (force == 1))
	{
		info->work_mode = 0;
		info->pause_decode = 0;
        
		if((info->vdec_type == MPEG2_DECODER) || (info->vdec_type == H264_DECODER) 
           || (info->vdec_type == AVS_DECODER))
		{
			vdec_stop(info->cur_dev, FALSE, FALSE);
		}

		vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CMD_RELEASE, 0, 0);

		if((info->vdec_type == MPEG2_DECODER) || (info->vdec_type == H264_DECODER) 
           || (info->vdec_type == AVS_DECODER))
		{
			video_decoder_select(MPEG2_DECODER, FALSE);			
			hld_decv_set_cur_dev(info, 0);
		}
	}
}

void hld_decv_rpc_init(struct ali_video_info *info)
{
	ali_rpc_register_callback(ALI_RPC_CB_VDEC, vdec_cb_routine);
    ali_rpc_register_callback(ALI_RPC_CB_VDEC_SPEC, vdec_cb_spec_routine);

	info->call_back.pcb_first_showed = decv_pcb_first_showed;
	info->call_back.pcb_mode_switch_ok = decv_pcb_mode_switch_ok;
	info->call_back.pcb_backward_restart_gop = decv_pcb_backward_reset_gop;
	info->call_back.pcb_first_head_parsed = decv_pcb_first_header_parsed;	
    info->call_back.pcb_first_i_decoded = decv_pcb_first_i_decoded;
    info->call_back.pcb_vdec_user_data_parsed = decv_pcb_user_data_parsed;

	{
		struct vdec_io_reg_callback_para para;

		para.eCBType = VDEC_CB_FIRST_SHOWED;
		para.pCB = info->call_back.pcb_first_showed;
		vdec_io_control(g_decv_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);

        #if 0		
		para.eCBType = VDEC_CB_MODE_SWITCH_OK;
		para.pCB = info->call_back.pcb_mode_switch_ok;					
		vdec_io_control(g_decv_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);	
        #endif		

		para.eCBType = VDEC_CB_BACKWARD_RESTART_GOP;
		para.pCB = info->call_back.pcb_backward_restart_gop;
		vdec_io_control(g_decv_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);

        #if 0
		para.eCBType = VDEC_CB_FIRST_HEAD_PARSED;
		para.pCB = info->call_back.pcb_first_showed;				
		vdec_io_control(g_decv_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);	
        #endif	

        para.eCBType = VDEC_CB_FIRST_I_DECODED;
		para.pCB = info->call_back.pcb_first_i_decoded;
		vdec_io_control(g_decv_dev0, VDEC_IO_REG_CALLBACK, (UINT32)&para);
	}
		
	return;
}

INT32 hld_decv_start(struct ali_video_info *info)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if(info == NULL)
    {
        return res;
    }

    switch_working_mode(info);
    ret_code = vdec_start(info->cur_dev);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);
    
    VDEC_PRINTF("video %u start, ret %ld\n", info->vdec_type, res);

    return res;
}

INT32 hld_decv_stop(struct ali_video_info *info, struct vdec_stop_param *stop_param)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (stop_param == NULL))
    {
        return res;
    }
    
    ret_code = vdec_stop(info->cur_dev, stop_param->close_display, stop_param->fill_black);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video %u stop <%ld %ld> ret %ld\n", 
        info->vdec_type, stop_param->close_display, stop_param->fill_black, res);

    return res;
}

INT32 hld_decv_pause(struct ali_video_info *info)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if(info == NULL)
    {
        return res;
    }
    
    ret_code = vdec_dvr_pause(info->cur_dev);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video pause %ld\n", res);

    return res;
}

INT32 hld_decv_resume(struct ali_video_info *info)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if(info == NULL)
    {
        return res;
    }
    
    ret_code = vdec_dvr_resume(info->cur_dev);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video resume %ld\n", res);

    return res;
}

INT32 hld_decv_step(struct ali_video_info *info)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if(info == NULL)
    {
        return res;
    }
    
    ret_code = vdec_step(info->cur_dev);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video step %ld\n", res);

    return res;
}

enum vdec_type hld_decv_get_cur_decoder(struct ali_video_info *info)
{
    enum video_decoder_type type = get_current_decoder();
    
	if((type == H264_DECODER) && (info->cur_dev != g_decv_avc_dev0)) 
    {
		hld_decv_set_cur_dev(info, H264_DECODER);
        VDEC_PRINTF("video switch to H264 decoder\n");
	}
	else if((type == AVS_DECODER) && (info->cur_dev != g_decv_avs_dev0)) 
    {
		hld_decv_set_cur_dev(info, AVS_DECODER);
        VDEC_PRINTF("video switch to avs decoder\n");
	}
    else if((type == MPEG2_DECODER) && (info->cur_dev != g_decv_dev0)) 
    {
		hld_decv_set_cur_dev(info, MPEG2_DECODER);
        VDEC_PRINTF("video switch to mpeg2 decoder\n");
    }

    return type;
}

INT32 hld_decv_set_sync_mode(struct ali_video_info *info, struct vdec_sync_param *sync_param)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (sync_param == NULL))
    {
        return res;
    }
    
    ret_code = vdec_sync_mode(info->cur_dev, sync_param->sync_mode, VDEC_SYNC_I|VDEC_SYNC_P|VDEC_SYNC_B);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video set sync mode %u ret %ld\n", sync_param->sync_mode, res);

    return res;
}

INT32 hld_decv_set_play_mode(struct ali_video_info *info, struct vdec_playback_param *playback_param)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (playback_param == NULL))
    {
        return res;
    }
    
    ret_code = vdec_playmode(info->cur_dev, playback_param->direction, playback_param->rate);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video set play mode %u %u ret %ld\n", playback_param->direction, playback_param->rate, res);

    return res;
}

INT32 hld_decv_set_pvr_param(struct ali_video_info *info, struct vdec_pvr_param *pvr_param)
{
    struct VDec_DvrConfigParam param;
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (pvr_param == NULL))
    {
        return res;
    }

    param.is_scrambled = pvr_param->is_scrambled;
    ret_code = vdec_dvr_set_param(info->cur_dev, param);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video set pvr param %d ret %ld\n", param.is_scrambled, res);

    return res;
}

INT32 hld_decv_select_decoder(struct ali_video_info *info, struct vdec_codec_param *codec_param)
{
    INT32 res = -1;

    if((info == NULL) || (codec_param == NULL))
    {
        return res;
    }

    VDEC_PRINTF("video select decoder %u preview %ld\n", codec_param->type, codec_param->preview);

    hld_decv_set_cur_dev(info, (int)(codec_param->type));
    video_decoder_select((int)(codec_param->type), codec_param->preview);
    if(codec_param->type == VDEC_AVC)
    {
        init_avc_callback();
    }

    res = 0;
    return res;
}

INT32 hld_decv_set_output(struct ali_video_info *info, struct vdec_output_param *output_param)
{
    struct VDecPIPInfo pip_info;
    enum VDecOutputMode mode = MP_MODE;
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (output_param == NULL))
    {
        return res;
    }

    memset(&pip_info, 0, sizeof(pip_info));

    if(output_param->output_mode == VDEC_PREVIEW)
    {
        mode = PREVIEW_MODE;
    }
    else if(output_param->output_mode == VDEC_SW_PASS)
    {
        mode = SW_PASS_MODE;
    }

    pip_info.adv_setting.init_mode = (mode == PREVIEW_MODE) ? 1 : 0;
    pip_info.adv_setting.bprogressive = output_param->progressive;
    pip_info.adv_setting.out_sys = output_param->tv_sys;
    ret_code = vdec_set_output(info->cur_dev, mode, &pip_info, &output_param->mp_callback, &output_param->pip_callback);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video set output %u ret %ld\n", output_param->output_mode, res);

    return res;
}

INT32 hld_decv_ioctl(struct ali_video_info *info, UINT32 cmd, UINT32 param)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    switch(cmd)
    {
        case VDEC_IO_GET_STATUS:
        {
            struct VDec_StatusInfo cur_status;
            struct vdec_status_info *pvdec_stat = (struct vdec_status_info *)param;
            
            memset(&cur_status, 0, sizeof(cur_status));
            ret_code = vdec_io_control(info->cur_dev, cmd, (UINT32)(&cur_status));
            if(ret_code == RET_SUCCESS)
            {
                pvdec_stat->status = cur_status.uCurStatus;
                pvdec_stat->first_header_parsed = cur_status.bFirstHeaderGot;
                pvdec_stat->first_pic_showed    = cur_status.uFirstPicShowed;
                pvdec_stat->first_pic_decoded   = cur_status.first_pic_decoded;
                pvdec_stat->pic_width        = cur_status.pic_width;
                pvdec_stat->pic_height       = cur_status.pic_height;
                pvdec_stat->aspect_ratio     = cur_status.aspect_ratio;
                pvdec_stat->frames_displayed = cur_status.display_idx;
                pvdec_stat->frames_decoded   = cur_status.frames_decoded;
                pvdec_stat->frame_last_pts   = cur_status.frame_last_pts;
                pvdec_stat->show_frame   = cur_status.display_frm;
                pvdec_stat->queue_count  = cur_status.top_cnt;
                pvdec_stat->buffer_size  = cur_status.vbv_size;
                pvdec_stat->buffer_used  = cur_status.valid_size;
                pvdec_stat->frame_rate   = cur_status.frame_rate;
                pvdec_stat->interlaced_frame = cur_status.progressive ? 0 : 1;
                pvdec_stat->top_field_first  = cur_status.top_field_first;
                pvdec_stat->hw_dec_error = cur_status.hw_dec_error;
                pvdec_stat->is_support   = cur_status.is_support;
                pvdec_stat->playback_param.direction     = cur_status.play_direction;
                pvdec_stat->playback_param.rate          = cur_status.play_speed;
                pvdec_stat->api_playback_param.direction = cur_status.api_play_direction;
                pvdec_stat->api_playback_param.rate      = cur_status.api_play_speed;

                if(cur_status.output_mode == MP_MODE)
                {
                    pvdec_stat->output_mode = VDEC_FULL_VIEW;
                }
                else if(cur_status.output_mode == PREVIEW_MODE)
                {
                    pvdec_stat->output_mode = VDEC_PREVIEW;
                }
            }
            break;
        }

        case VDEC_IO_CAPTURE_DISPLAYING_FRAME:
        {
            ret_code = vdec_capture_frame(info, (struct vdec_picture *)param);
            break;
        }
        
        default:
            ret_code = vdec_io_control(info->cur_dev, cmd, param);
            break;
    }
    
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    return res;
}

INT32 hld_decv_mp_init(struct ali_video_info *info, struct vdec_mp_init_param *init_param)
{
    VdecInit vdec_init;
    RET_CODE ret_code = RET_SUCCESS;
    UINT32 codec_id;
    INT32 res = -1;

    if((info == NULL) || (init_param == NULL))
    {
        return res;
    }
    codec_id = init_param->codec_tag;
    memset(&vdec_init, 0, sizeof(vdec_init));

    vdec_init.decode_mode  = init_param->decode_mode;
    vdec_init.decoder_flag = init_param->decoder_flag;
    vdec_init.preview      = init_param->preview;
    vdec_init.pfrm_buf     = (VdecFBConfig *)(init_param->dec_buf_addr);
    vdec_init.phw_mem_cfg  = (VdecHWMemConfig *)(init_param->dec_buf_size);
    vdec_init.fmt_in.fourcc         = init_param->codec_tag;
    vdec_init.fmt_in.frame_rate     = init_param->frame_rate;
    vdec_init.fmt_in.pic_width      = init_param->pic_width;
    vdec_init.fmt_in.pic_height     = init_param->pic_height;
    vdec_init.fmt_in.pixel_aspect_x = init_param->pixel_aspect_x;
    vdec_init.fmt_in.pixel_aspect_y = init_param->pixel_aspect_y;
    memcpy(&(vdec_init.fmt_out), &(vdec_init.fmt_in), sizeof(vdec_init.fmt_in));

    if((codec_id == wvc1) || (codec_id == wx3))
    {
        codec_id = vc1;
    }

    ret_code = vdec_decore_ioctl((void*)&codec_id, VDEC_CMD_INIT, (void*)&codec_id, (void*)&vdec_init);
    if(ret_code == RET_SUCCESS)
    {
        info->codec_tag = codec_id;
        info->work_mode = 1;
        info->pause_decode  = 0;
        info->pause_display = 0;
        res = 0;
    }

    VDEC_PRINTF("video %c%c%c%c init resolution <%ldx%ld> frame rate %lu aspect <%ldx%ld> ret %ld\n",
        (UINT8)init_param->codec_tag, (UINT8)(init_param->codec_tag>>8), 
        (UINT8)(init_param->codec_tag>>16), (UINT8)(init_param->codec_tag>>24),
        init_param->pic_width, init_param->pic_height, init_param->frame_rate, 
        init_param->pixel_aspect_x, init_param->pixel_aspect_y, res);

    return res;
}

INT32 hld_decv_mp_rls(struct ali_video_info *info, struct vdec_mp_rls_param *rls_param)
{
    enum video_decoder_type decoder = MPEG2_DECODER;
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (rls_param == NULL))
    {
        return res;
    }

    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CMD_RELEASE, NULL, NULL);
    decoder = get_current_decoder();
    hld_decv_set_cur_dev(info, decoder);
    info->codec_tag = 0;
    info->work_mode = 0;
    info->pause_decode  = 0;
    info->pause_display = 0;

    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video release %ld\n", res);

    return res;
}

INT32 hld_decv_mp_flush(struct ali_video_info *info, struct vdec_mp_flush_param *flush_param)
{
    struct OutputFrmManager output_frm_info;
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (flush_param == NULL))
    {
        return res;
    }

    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CMD_SW_RESET, (void*)&flush_param->flush_flag, (void*)&output_frm_info);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video flush %lu ret %ld\n", flush_param->flush_flag, res);

    return res;
}

INT32 hld_decv_mp_extra_data(struct ali_video_info *info, struct vdec_mp_extra_data *extra_data_param)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (extra_data_param == NULL))
    {
        return res;
    }

    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CMD_EXTRA_DADA, \
                                 (void*)extra_data_param->extra_data, (void*)extra_data_param->extra_data_size);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video decode extra data %ld\n", res);

    return res;
}

INT32 hld_decv_mp_get_status(struct ali_video_info *info, struct vdec_decore_status *decore_status)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (decore_status == NULL))
    {
        return res;
    }

    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CMD_GET_STATUS, (void*)decore_status, NULL);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    return res;
}

INT32 hld_decv_mp_pause(struct ali_video_info *info, struct vdec_mp_pause_param *pause_param)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 pause_deocde = -1;
    INT32 pause_display = -1;
    INT32 res = -1;

    if((info == NULL) || (pause_param == NULL))
    {
        return res;
    }
    
    pause_deocde  = (pause_param->pause_decode == 0xFF) ? -1 : pause_param->pause_decode;
    pause_display = (pause_param->pause_display == 0xFF) ? -1 : pause_param->pause_display;
    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CMD_PAUSE_DECODE, \
                                 (void*)pause_deocde, (void*)pause_display);
    if(ret_code == RET_SUCCESS)
    {
        if(pause_deocde > 0)
        {
            info->pause_decode = 1;
        }
        else if(pause_deocde == 0)
        {
            info->pause_decode = 0;
        }

        if(pause_display > 0)
        {
            info->pause_display = 1;
        }
        else if(pause_display == 0)
        {
            info->pause_display = 0;
        }
    }
    
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video pause <%u %u> ret %ld\n", pause_param->pause_decode, pause_param->pause_display, res);

    return res;
}

INT32 hld_decv_mp_set_sbm(struct ali_video_info *info, struct vdec_mp_sbm_param *sbm_param)
{
    RET_CODE ret_code = RET_SUCCESS;
    UINT32 decode_input = 0;
    UINT32 decode_output = (sbm_param->decode_output == 0xFF) ? -1 : sbm_param->decode_output;
    UINT32 display_input = (sbm_param->display_input == 0xFF) ? -1 : sbm_param->display_input;
    INT32 res = -1;

    if((info == NULL) || (sbm_param == NULL))
    {
        return res;
    }

    decode_input = ((sbm_param->packet_header<<16) | sbm_param->packet_data);
    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CFG_VIDEO_SBM_BUF, (void*)decode_input, (void*)decode_output);
    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CFG_DISPLAY_SBM_BUF, (void*)display_input, NULL);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video set sbm <%u %u %u %u> ret %ld\n", 
        sbm_param->packet_header, sbm_param->packet_data,
        sbm_param->decode_output, sbm_param->display_input, res);

    return res;
}

INT32 hld_decv_mp_set_sync_mode(struct ali_video_info *info, struct vdec_mp_sync_param *sync_param)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if((info == NULL) || (sync_param == NULL))
    {
        return res;
    }

    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CFG_SYNC_MODE, \
                                 (void*)&sync_param->sync_mode, (void*)&sync_param->sync_unit);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video set sync mode <%u %u> ret %ld\n", sync_param->sync_mode, sync_param->sync_unit, res);

    return res;
}

INT32 hld_decv_mp_set_display_rect(struct ali_video_info *info, struct vdec_display_rect *display_rect)
{
    RET_CODE ret_code = RET_SUCCESS;
    struct Video_Rect src_rect;
    struct Video_Rect dst_rect;
    INT32 res = -1;

    if((info == NULL) || (display_rect == NULL))
    {
        return res;
    }

    src_rect.x = display_rect->src_x;
    src_rect.y = display_rect->src_y;
    src_rect.w = display_rect->src_w;
    src_rect.h = display_rect->src_h;
    dst_rect.x = display_rect->dst_x;
    dst_rect.y = display_rect->dst_y;
    dst_rect.w = display_rect->dst_w;
    dst_rect.h = display_rect->dst_h;

    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CFG_DISPLAY_RECT, (void*)&src_rect, (void*)&dst_rect);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video set display rect <%ld %ld %ld %ld> => <%ld %ld %ld %ld> ret %ld\n",
        src_rect.x, src_rect.y, src_rect.w, src_rect.h,
        dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h, res);

    return res;
}

INT32 hld_decv_mp_set_quick_mode(struct ali_video_info *info, UINT32 quick_mode)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if(info == NULL)
    {
        return res;
    }

    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CFG_QUICK_MODE, (void*)&quick_mode, NULL);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video set quick mode %lu ret %ld\n", quick_mode, res);

    return res;
}

INT32 hld_decv_mp_capture_frame(struct ali_video_info *info, struct vdec_picture *picture)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if(info == NULL)
    {
        return res;
    }
    
    ret_code = vdec_capture_frame(info, picture);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    return res;
}

INT32 hld_decv_mp_set_dec_frm_type(struct ali_video_info *info, UINT32 frm_type)
{
    RET_CODE ret_code = RET_SUCCESS;
    UINT32 decode_mode = frm_type;
    INT32 res = -1;

    if(info == NULL)
    {
        return res;
    }

    ret_code = vdec_decore_ioctl((void*)&info->codec_tag, VDEC_CFG_DECODE_MODE, (void*)&decode_mode, NULL);
    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    VDEC_PRINTF("video set frame type %lu ret %ld\n", decode_mode, res);

    return res;
}

INT32 hld_decv_mp_ioctl(struct ali_video_info *info, UINT32 cmd, UINT32 param)
{
    RET_CODE ret_code = RET_SUCCESS;
    INT32 res = -1;

    if(info == NULL)
    {
        return res;
    }   

    switch(cmd)
    {
        case VDEC_DYNAMIC_FRAME_ALLOC:
            ret_code = vdec_decore_ioctl((void*)&info->codec_tag, cmd, (void *)&param, NULL);
            VDEC_PRINTF("video dynamic frame alloc %lu ret %ld\n", param, ret_code);
            break;
            
        default:
            break;
    }

    res = ((ret_code == RET_SUCCESS) ? 0 : -1);

    return res;
}

void hld_decv_fb_cost_down(struct ali_video_info *info, int num)
{
	vdec_io_control(info->cur_dev, VDEC_IO_SET_FB_COST_DOWN_NUM, num);

    VDEC_PRINTF("video cost down fb num %d\n", num);
}