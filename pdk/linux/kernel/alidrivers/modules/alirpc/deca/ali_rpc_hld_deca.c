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
#include <rpc_hld/ali_rpc_hld_deca.h>
#include <dvb_audio.h>

#include "../ali_rpc.h"

volatile enum AudioStreamType g_deca_stream_type;
UINT32 desc_deca_m36_attach[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct deca_feature_config)),
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};
UINT32 desc_deca_m36_ext_dec_enable[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct deca_feature_config)),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};

void deca_m36_attach(struct deca_feature_config* config)
{
    jump_to_func(NULL, ali_rpc_call, config, (LLD_DECA_M36_MODULE<<24)|(1<<16)|FUNC_DECA_M36_ATTACH, desc_deca_m36_attach);
}

void deca_m36_dvr_enable(struct deca_device*dev)
{
    jump_to_func(NULL, ali_rpc_call, dev, (LLD_DECA_M36_MODULE<<24)|(1<<16)|FUNC_DECA_M36_DVR_ENABLE, NULL);
}
void deca_m36_ext_dec_enable(struct deca_device*dev, struct deca_feature_config * config)
{      
    jump_to_func(NULL, ali_rpc_call, dev, (LLD_DECA_M36_MODULE<<24)|(2<<16)|FUNC_DECA_M36_EXT_DEC_ENABLE, desc_deca_m36_ext_dec_enable);
}
void deca_m36_init_tone_voice(struct deca_device * dev)
{
    jump_to_func(NULL, ali_rpc_call, dev, (LLD_DECA_M36_MODULE<<24)|(1<<16)|FUNC_DECA_M36_INIT_TONE_VOICE, NULL);
}


UINT32 desc_deca_p_play_param[] =
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct ase_str_play_param)),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_deca_p_uint32[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};
UINT32 desc_deca_p_ioparam[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct io_param)),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_deca_p_ioparam2[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct io_param)), DESC_STATIC_STRU(1, 4), 
  1, DESC_P_PARA(0, 2, 0), DESC_P_STRU(1, 0, 1, offsetof(struct io_param, io_buff_out)),
  //desc of pointer ret
  0,                          
  0,
};
/*
UINT32 desc_deca_callback[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct soft_decoder_callback)),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_deca_callback2[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct soft_decoder_callback2)),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};
*/
UINT32 desc_deca_get_play_param[] = 
{
  1, DESC_OUTPUT_STRU(0, sizeof(struct cur_stream_info)),  
  1, DESC_P_PARA(0, 2, 0),
  0,                          
  0,    
};
UINT32 desc_deca_get_es_buf_state[] = 
{
  1, DESC_OUTPUT_STRU(0, sizeof(struct deca_buf_info)),  
  1, DESC_P_PARA(0, 2, 0),
  0,                          
  0,      
};
UINT32 desc_deca_io_get_audio_info[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct AUDIO_INFO)),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};
UINT32 desc_deca_io_set_reverb[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct reverb_param)),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};
UINT32 desc_deca_io_set_pl_ii[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pl_ii_param)),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};

void deca_pcm_buf_resume(UINT32 val)
{
    jump_to_func(NULL, ali_rpc_call, val, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_PCM_BUF_RESUME, NULL);
}

RET_CODE deca_open(struct deca_device * dev,
                                enum AudioStreamType stream_type, 
                                enum AudioSampleRate samp_rate, 
                                enum AudioQuantization quan,
                                UINT8 channel_num,
                                UINT32 info_struct)
{
	
    g_deca_stream_type = stream_type;
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(6<<16)|FUNC_DECA_OPEN, NULL);
	
}

RET_CODE deca_close(struct deca_device * dev)
{
	
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_CLOSE, NULL);
	
}

RET_CODE deca_set_sync_mode(struct deca_device * dev, enum ADecSyncMode mode)
{
	
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_SET_SYNC_MODE, NULL);
    
}

RET_CODE deca_start(struct deca_device * dev, UINT32 high32_pts)
{
	
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_START, NULL);
	
}

RET_CODE deca_stop(struct deca_device * dev, UINT32 high32_pts, enum ADecStopMode mode)
{
	
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(3<<16)|FUNC_DECA_STOP, NULL);
    
}

UINT32 deca_standby(struct deca_device * dev, UINT32 status)
{
    
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_STANDBY, NULL);
    
}
EXPORT_SYMBOL(deca_standby);

RET_CODE deca_pause(struct deca_device * dev)
{
	
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_PAUSE, NULL);
	
}

static UINT32 desc_deca_decore_init[] = 
{   //desc of pointer para
	2, DESC_STATIC_STRU(0, sizeof(struct audio_config)), DESC_OUTPUT_STRU(1, 4),
	2, DESC_P_PARA(0, 1, 0), DESC_P_PARA(1, 2, 1),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_deca_decore_uint32[] = 
{   //desc of pointer para
	2, DESC_OUTPUT_STRU(0, 4), DESC_OUTPUT_STRU(1, 4),
	2, DESC_P_PARA(0, 1, 0), DESC_P_PARA(1, 2, 1),
	//desc of pointer ret
	0,                          
	0,
};

static UINT32 desc_deca_decore_get_status[] = 
{   //desc of pointer para
	1, DESC_OUTPUT_STRU(0, sizeof(struct audio_decore_status)),
	1, DESC_P_PARA(0, 1, 0),
	//desc of pointer ret
	0,                          
	0,
};


RET_CODE deca_decore_ioctl(struct deca_device *dev, UINT32 cmd, void *param1, void *param2)
{
	
	UINT32 *desc = NULL;

    switch(cmd)
    {
        case DECA_DECORE_INIT:
            desc = desc_deca_decore_init;
            break;
        case DECA_DECORE_RLS:
        case DECA_DECORE_FLUSH:
            break;
        case DECA_DECORE_SET_BASE_TIME:
        case DECA_DECORE_GET_PCM_TRD:
        case DECA_DECORE_PAUSE_DECODE:
        case DECA_DECORE_GET_CUR_TIME:
	case DECA_DECORE_SET_QUICK_MODE:    
	case DECA_DECORE_SET_SYNC_MODE:      
            desc = desc_deca_decore_uint32;
            break;
        case DECA_DECORE_GET_STATUS:
            desc = desc_deca_decore_get_status;
            break;
        default:
            return RET_FAILURE;
    }

	jump_to_func(NULL, ali_rpc_call, cmd, (HLD_DECA_MODULE<<24)|(3<<16)|FUNC_DECA_DECORE_IOCTL, desc);

	
}
RET_CODE deca_io_control(struct deca_device * dev, UINT32 cmd, UINT32 param)
{
	
    UINT32 *desc = NULL;
    switch(cmd)
    {
        case DECA_EMPTY_BS_SET:
        case DECA_ADD_BS_SET:
        case DECA_DEL_BS_SET:
        case DECA_IS_BS_MEMBER:
        case SET_PASS_CI:
        case DECA_SET_DOLBY_ONOFF:
        case DECA_HDD_PLAYBACK:
        case DECA_SET_PLAY_SPEED:
        case DECA_SET_DECODER_COUNT:
        case DECA_SET_AC3_MODE:
        case DECA_DOLBYPLUS_CONVERT_ONOFF:
        case DECA_SYNC_BY_SOFT:            
        case DECA_SYNC_NEXT_HEADER:
        case DECA_SOFTDEC_JUMP_TIME2:
        case DECA_SOFTDEC_IS_PLAY_END2:
        case DECA_BEEP_INTERVAL:
        case DECA_INDEPENDENT_DESC_ENABLE:
        case DECA_STR_STOP:
        case DECA_RESET_BS_BUFF:
        case DECA_DOLBYPLUS_DEMO_ONOFF:
        case DECA_SET_BUF_MODE:
        case DECA_DO_DDP_CERTIFICATION:
        case DECA_DYNAMIC_SND_DELAY:
        case DECA_SET_AC3_COMP_MODE:
        case DECA_SET_AC3_STEREO_MODE:
        case DECA_CONFIG_BS_BUFFER:
        case DECA_CONFIG_BS_LENGTH:
        case DECA_BS_BUFFER_RESUME:
        case DECA_DOLBY_SET_VOLUME_DB: 
        case DECA_SET_CACHE_INVALID_FLAG:
		case DECA_SET_QUICK_PLAY_MODE:
		case DECA_PCM_SIGNED_SET:
            desc = NULL;
            break;
        case DECA_SET_STR_TYPE:
            g_deca_stream_type = (enum AudioStreamType)param;
            break;
                
        case DECA_GET_STR_TYPE:
        case DECA_GET_HIGHEST_PTS:
        case DECA_GET_AC3_BSMOD:
        case DECA_CHECK_DECODER_COUNT:
        case DECA_GET_DESC_STATUS:
        case DECA_GET_DECODER_HANDLE:
        case DECA_GET_DECA_STATE:
        case DECA_SOFTDEC_GET_ELAPSE_TIME2:
        case DECA_DOLBYPLUS_CONVERT_STATUS:
        case DECA_GET_BS_FRAME_LEN:
        case DECA_GET_DDP_INMOD: 
            desc = desc_deca_p_uint32;
            break;
        case DECA_GET_AUDIO_INFO:
            desc = desc_deca_io_get_audio_info;
            break;
        case DECA_SET_REVERB:
            desc = desc_deca_io_set_reverb;
            break;
        case DECA_SET_PL_II:
            desc = desc_deca_io_set_pl_ii;
            break;
        case DECA_SOFTDEC_INIT:
            g_deca_stream_type = AUDIO_MP3;
            break;
        case DECA_SOFTDEC_CLOSE:
            g_deca_stream_type = AUDIO_INVALID;
            break;
        case DECA_SOFTDEC_SET_TIME:
        case DECA_SOFTDEC_JUMP_TIME:
        case DECA_SOFTDEC_IS_PLAY_END:
            break;
        case DECA_SOFTDEC_INIT2:
        	  g_deca_stream_type = AUDIO_WMA;
            desc = desc_deca_p_ioparam;
            break;
        case DECA_SOFTDEC_CLOSE2:
        	  g_deca_stream_type = AUDIO_INVALID;
            break;
        case DECA_SOFTDEC_CAN_DECODE2:
            desc = desc_deca_p_ioparam2;
            break;
        case DECA_GET_PLAY_PARAM:
            desc = desc_deca_get_play_param;
            break;
		case DECA_STR_PLAY:
            desc = desc_deca_p_play_param;
        case DECA_GET_ES_BUFF_STATE:
            desc = desc_deca_get_es_buf_state;
            break;
        /*case DECA_SOFTDEC_REGISTER_CB:
            desc = desc_deca_callback;
            break;
        case DECA_SOFTDEC_REGISTER_CB2:
            desc = desc_deca_callback2;        
            break;*/
        default:
            break;
	  };

	  jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(3<<16)|FUNC_DECA_IO_CONTROL, desc);
	  
}

UINT32 desc_deca_request_write[] = 
{ //desc of pointer para
	3, DESC_OUTPUT_STRU(0, 4), DESC_OUTPUT_STRU(1, 4), DESC_STATIC_STRU(2, sizeof(struct control_block)),
	3, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1), DESC_P_PARA(2, 4, 2),
  //desc of pointer ret
  0,                          
  0,
};

RET_CODE deca_request_write(void  * device, UINT32 req_size, 
	                                        void ** ret_buf, UINT32 * ret_buf_size,
	                                        struct control_block * ctrl_blk)
{
	
    jump_to_func(NULL, ali_rpc_call, device, (HLD_DECA_MODULE<<24)|(5<<16)|FUNC_DECA_REQUEST_WRITE, desc_deca_request_write);
    
}

void deca_update_write(void * device, UINT32 size)
{
    jump_to_func(NULL, ali_rpc_call, device, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_UPDATE_WRITE, NULL);
}

void deca_tone_voice(struct deca_device * dev, UINT32 SNR, UINT8 init)  //tone voice// clear waring 050907 ming yi
{
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(3<<16)|FUNC_DECA_TONE_VOICE, NULL);
}

void deca_stop_tone_voice(struct deca_device * dev)  //tone voice // clear waring 050907 ming yi
{
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_STOP_TONE_VOICE, NULL);
}

RET_CODE deca_request_desc_write(void  * device, UINT32 req_size, 
	                                        void ** ret_buf, UINT32 * ret_buf_size,
	                                        struct control_block * ctrl_blk)
{
    jump_to_func(NULL, ali_rpc_call, device, (HLD_DECA_MODULE<<24)|(5<<16)|FUNC_DECA_REQUEST_WRITE, NULL);
}

void deca_update_desc_write(void * device, UINT32 size)
{
    jump_to_func(NULL, ali_rpc_call, device, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_UPDATE_WRITE, NULL);
}

void deca_init_ase(struct deca_device * device)
{
    jump_to_func(NULL, ali_rpc_call, device, (HLD_DECA_MODULE<<24)|(1<<16)|FUNC_DECA_INIT_ASE_VOICE, NULL);
}

void deca_process_pcm_samples(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, UINT32 sample_rate, UINT32 channel_num, UINT32 sample_precision)
{
    jump_to_func(NULL, ali_rpc_call, pcm_bytes_len, (HLD_DECA_MODULE<<24)|(5<<16)|FUNC_DECA_PROCESS_PCM_SAMPLES, NULL);
}

void deca_process_pcm_bitstream(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf,
    UINT32 bs_length, UINT8 *un_processed_bs, UINT32 sample_rate, UINT32 channel_num, UINT32 sample_precision)
{
    jump_to_func(NULL, ali_rpc_call, pcm_bytes_len, (HLD_DECA_MODULE<<24)|(7<<16)|FUNC_DECA_PROCESS_PCM_BITSTREAM, NULL);
}

RET_CODE deca_set_dbg_level(struct deca_device * dev, UINT32 option)
{
    	
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_SET_DBG_LEVEL, NULL);    
    
}

