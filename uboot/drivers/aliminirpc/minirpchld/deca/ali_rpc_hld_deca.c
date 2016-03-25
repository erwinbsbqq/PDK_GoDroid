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
#include <linux/ali_audio.h>
#include <rpc_hld/ali_rpc_hld_deca.h>

//#include "../ali_rpc.h"
#include <ali_rpcng.h>

volatile enum AudioStreamType g_deca_stream_type;

void deca_m36_attach(struct deca_feature_config* config)
{
    struct deca_feature_config_rpc daca_feature;
    struct deca_feature_config_rpc *tmp = &daca_feature;
    
    tmp->detect_sprt_change = config->detect_sprt_change;
    tmp->bs_buff_size = config->bs_buff_size;
    tmp->support_desc = config->support_desc;
    tmp->reserved = config->reserved;
    tmp->reserved16 = config->reserved16;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Deca_feature_config_rpc, sizeof(struct deca_feature_config_rpc), tmp);
    RpcCallCompletion(RPC_deca_m36_attach, &p1, NULL);
}

void deca_m36_dvr_enable(struct deca_device*dev)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    RpcCallCompletion(RPC_deca_m36_dvr_enable,&p1,NULL);
    
}
void deca_m36_ext_dec_enable(struct deca_device*dev, struct deca_feature_config * config)
{      
    struct deca_feature_config_rpc daca_feature;
    struct deca_feature_config_rpc *tmp = &daca_feature;
    
    tmp->detect_sprt_change = config->detect_sprt_change;
    tmp->bs_buff_size = config->bs_buff_size;
    tmp->support_desc = config->support_desc;
    tmp->reserved = config->reserved;
    tmp->reserved16 = config->reserved16;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Deca_feature_config_rpc, sizeof(struct deca_feature_config_rpc), tmp);
    
    RpcCallCompletion(RPC_deca_m36_ext_dec_enable,&p1,&p2,NULL);
    
}
void deca_m36_init_tone_voice(struct deca_device * dev)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    RpcCallCompletion(RPC_deca_m36_init_tone_voice,&p1,NULL);

}

/*#define offsetof(type, f) ((unsigned long) \
((char *)&((type *)0)->f - (char *)(type *)0))
*/

void deca_pcm_buf_resume(UINT32 val)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &val);

    RpcCallCompletion(RPC_deca_pcm_buf_resume, &p1, NULL);
}

RET_CODE deca_open(struct deca_device * dev,
                                enum AudioStreamType stream_type, 
                                enum AudioSampleRate samp_rate, 
                                enum AudioQuantization quan,
                                UINT8 channel_num,
                                UINT32 info_struct)
{
    g_deca_stream_type = stream_type;

    Int32 ret;    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum AudioStreamType), &stream_type);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_ENUM, sizeof(enum AudioSampleRate), &samp_rate);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_ENUM, sizeof(enum AudioQuantization), &quan);
    RPC_PARAM_CREATE(p5, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &channel_num);
    RPC_PARAM_CREATE(p6, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &info_struct);

    ret = RpcCallCompletion(RPC_deca_open,&p1,&p2,&p3,&p4,&p5,&p6,NULL);
    return ret;
}

RET_CODE deca_close(struct deca_device * dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    ret = RpcCallCompletion(RPC_deca_close, &p1, NULL);
    return ret;
}

RET_CODE deca_set_sync_mode(struct deca_device * dev, enum ADecSyncMode mode)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum ADecSyncMode), &mode);

    ret = RpcCallCompletion(RPC_deca_set_sync_mode,&p1,&p2,NULL);
    return ret;
}

RET_CODE deca_start(struct deca_device * dev, UINT32 high32_pts)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &high32_pts);

    ret = RpcCallCompletion(RPC_deca_start, &p1, &p2, NULL);
    return ret;
}

RET_CODE deca_stop(struct deca_device * dev, UINT32 high32_pts, enum ADecStopMode mode)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &high32_pts);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_ENUM, sizeof(enum ADecStopMode), &mode);

    ret = RpcCallCompletion(RPC_deca_stop, &p1, &p2, &p3, NULL);
    return ret;
}

UINT32 deca_standby(struct deca_device * dev, UINT32 status)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &status);

    ret = RpcCallCompletion(RPC_deca_standby, &p1, &p2, NULL);
    return ret;
}
EXPORT_SYMBOL(deca_standby);

RET_CODE deca_pause(struct deca_device * dev)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    ret = RpcCallCompletion(RPC_deca_pause, &p1, NULL);
    return ret;
}

int deca_decore_ioctl(UINT32 cmd, void *param1, void *param2)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &cmd);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &param1);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, 4, &param2);

    switch(cmd)
    {
        case DECA_DECORE_INIT:
            RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_Audio_config_rpc, sizeof(Audio_config_rpc), param1);
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, 4, param2);

            ret = RpcCallCompletion(RPC_deca_decore_ioctl, &p1, &p2, &p3, NULL);
            break;
        case DECA_DECORE_RLS:
        case DECA_DECORE_FLUSH:
            RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &param1);
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 4, &param2);

            ret = RpcCallCompletion(RPC_deca_decore_ioctl, &p1, &p2, &p3, NULL); 
            break;
        case DECA_DECORE_SET_BASE_TIME:
        case DECA_DECORE_GET_PCM_TRD:
        case DECA_DECORE_PAUSE_DECODE:
        case DECA_DECORE_GET_CUR_TIME:
        case DECA_DECORE_SET_QUICK_MODE:    
	    case DECA_DECORE_SET_SYNC_MODE:     
            RPC_PARAM_UPDATE(p2, PARAM_OUT, PARAM_UINT32, sizeof(Uint32), param1);
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, 4, param2);
            ret = RpcCallCompletion(RPC_deca_decore_ioctl, &p1, &p2, &p3, NULL); 
            break;
        default:
            return RET_FAILURE;
    }

    return ret;
}
RET_CODE deca_io_control(struct deca_device * dev, UINT32 cmd, UINT32 param)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &cmd);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);

	Log(LOG_ERR," !!! Enter %s  : dev = %0x   cmd = %0x  param = %0x\n", __FUNCTION__, dev, cmd, param);
    
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
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, 4, param);
            break;
        case DECA_GET_AUDIO_INFO:
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_AUDIO_INFO_rpc, sizeof(AUDIO_INFO_rpc), param);
            break;
        case DECA_SET_REVERB:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Reverb_param_rpc, sizeof(Reverb_param_rpc), param);
            break;
        case DECA_SET_PL_II:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Pl_ii_param_rpc, sizeof(Pl_ii_param_rpc), param);
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
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Io_param_rpc, sizeof(Io_param_rpc), param);
            break;
        case DECA_SOFTDEC_CLOSE2:
        	  g_deca_stream_type = AUDIO_INVALID;
            break;
        case DECA_SOFTDEC_CAN_DECODE2:
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Io_param_rpc, sizeof(Io_param_rpc), param);
            break;
        case DECA_GET_PLAY_PARAM:
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Cur_stream_info_rpc, sizeof(Cur_stream_info_rpc), param);
            break;
        case DECA_GET_ES_BUFF_STATE:
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Deca_buf_info_rpc, sizeof(Deca_buf_info_rpc), param);
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

     ret = RpcCallCompletion(RPC_deca_io_control, &p1, &p2, &p3, NULL);
    return ret;
}

RET_CODE deca_request_write(void  * device, UINT32 req_size, 
	                                        void ** ret_buf, UINT32 * ret_buf_size,
	                                        struct control_block * ctrl_blk)
{
    INT32 ret;
    struct control_block_rpc control_block_rpc_struct;
    struct control_block_rpc *temp1 = &control_block_rpc_struct;

    if (NULL != ctrl_blk)
    {
        temp1->stc_id_valid = ctrl_blk->stc_id_valid;
        temp1->pts_valid = ctrl_blk->pts_valid;
        temp1->data_continue = ctrl_blk->data_continue;
        temp1->ctrlblk_valid = ctrl_blk->ctrlblk_valid;

        temp1->instant_update = ctrl_blk->instant_update;
        temp1->vob_start = ctrl_blk->vob_start;
        temp1->bstream_run_back = ctrl_blk->bstream_run_back;
        temp1->reserve = ctrl_blk->reserve;
        
        temp1->stc_id = ctrl_blk->stc_id;
        temp1->stc_offset_idx = ctrl_blk->stc_offset_idx;
        temp1->pts = ctrl_blk->pts;
    }

    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &device);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &req_size);
    RPC_PARAM_CREATE(p3, PARAM_OUT, PARAM_UINT32, 4, ret_buf);
    RPC_PARAM_CREATE(p4, PARAM_OUT, PARAM_UINT32, 4, ret_buf_size);
    RPC_PARAM_CREATE(p5, PARAM_IN, PARAM_Control_block_rpc, sizeof(struct control_block_rpc), temp1);

    ret = RpcCallCompletion(RPC_deca_request_write,&p1,&p2,&p3,&p4,&p5,NULL);
    return ret;
}

void deca_update_write(void * device, UINT32 size)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &device);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &size);
    
    RpcCallCompletion(RPC_deca_update_write, &p1, &p2, NULL);
}

void deca_tone_voice(struct deca_device * dev, UINT32 SNR, UINT8 init)  //tone voice// clear waring 050907 ming yi
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &SNR);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &init);
    
    RpcCallCompletion(RPC_deca_tone_voice, &p1, &p2, &p3, NULL);
}

void deca_stop_tone_voice(struct deca_device * dev)  //tone voice // clear waring 050907 ming yi
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    RpcCallCompletion(RPC_deca_stop_tone_voice, &p1, NULL);
}

RET_CODE deca_request_desc_write(void  * device, UINT32 req_size, 
	                                        void ** ret_buf, UINT32 * ret_buf_size,
	                                        struct control_block * ctrl_blk)
{
    //jump_to_func(NULL, ali_rpc_call, device, (HLD_DECA_MODULE<<24)|(5<<16)|FUNC_DECA_REQUEST_WRITE, NULL);//?
    struct control_block_rpc control_block_rpc_struct;
    struct control_block_rpc *temp1 = &control_block_rpc_struct;

    temp1->stc_id_valid = ctrl_blk->stc_id_valid;
    temp1->pts_valid = ctrl_blk->pts_valid;
    temp1->data_continue = ctrl_blk->data_continue;
    temp1->ctrlblk_valid = ctrl_blk->ctrlblk_valid;

    temp1->instant_update = ctrl_blk->instant_update;
    temp1->vob_start = ctrl_blk->vob_start;
    temp1->bstream_run_back = ctrl_blk->bstream_run_back;
    temp1->reserve = ctrl_blk->reserve;
    
    temp1->stc_id = ctrl_blk->stc_id;
    temp1->stc_offset_idx = ctrl_blk->stc_offset_idx;
    temp1->pts = ctrl_blk->pts;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &device);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &req_size);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, 4, &ret_buf);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_UINT32, 4, &ret_buf_size);
    RPC_PARAM_CREATE(p5, PARAM_IN, PARAM_Control_block_rpc, sizeof(struct control_block_rpc), temp1);

    RpcCallCompletion(RPC_deca_request_write, &p1, &p2, &p3, &p4, &p5, NULL);
}

void deca_update_desc_write(void * device, UINT32 size)
{
    //jump_to_func(NULL, ali_rpc_call, device, (HLD_DECA_MODULE<<24)|(2<<16)|FUNC_DECA_UPDATE_WRITE, NULL);  //?
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &device);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &size);
    
    RpcCallCompletion(RPC_deca_update_write, &p1, &p2, NULL);
}

void deca_init_ase(struct deca_device * device)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &device);
    
    RpcCallCompletion(RPC_deca_init_ase, &p1, NULL);
}

void deca_process_pcm_samples(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, UINT32 sample_rate, UINT32 channel_num, UINT32 sample_precision)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pcm_bytes_len);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &pcm_raw_buf);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &sample_rate);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &channel_num);
    RPC_PARAM_CREATE(p5, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &sample_precision);
    
    RpcCallCompletion(RPC_deca_process_pcm_samples, &p1, &p2, &p3, &p4, &p5, NULL);
}

void deca_process_pcm_bitstream(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf,
    UINT32 bs_length, UINT8 *un_processed_bs, UINT32 sample_rate, UINT32 channel_num, UINT32 sample_precision)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pcm_bytes_len);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &pcm_raw_buf);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &bs_length);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &un_processed_bs);
    
    RPC_PARAM_CREATE(p5, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &sample_rate);
    RPC_PARAM_CREATE(p6, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &channel_num);
    RPC_PARAM_CREATE(p7, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &sample_precision);
    
    RpcCallCompletion(RPC_deca_process_pcm_bitstream, &p1, &p2, &p3, &p4, &p5, &p6, &p7, NULL);
}

RET_CODE deca_set_dbg_level(struct deca_device * dev, UINT32 option)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &option);

    ret = RpcCallCompletion(RPC_deca_set_dbg_level, &p1, &p2, NULL);
    return ret;
}

