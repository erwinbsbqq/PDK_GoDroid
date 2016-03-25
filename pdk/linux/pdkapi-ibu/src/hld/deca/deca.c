#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>

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

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <dvb_audio.h>


//struct snd_device * g_snd_deca_ref ;
void (*fp_deca_pcm_process_func)(UINT32, UINT8 *, UINT32, UINT32, UINT32) = NULL;
INT32 m_deca_mutex_id;

#define DECA_MUTEX_LOCK   osal_mutex_lock(m_deca_mutex_id, OSAL_WAIT_FOREVER_TIME)
#define DECA_MUTEX_UNLOCK osal_mutex_unlock(m_deca_mutex_id)

__ATTRIBUTE_REUSE_
RET_CODE deca_open(struct deca_device * dev,
                                enum AudioStreamType stream_type, 
                                enum AudioSampleRate samp_rate, 
                                enum AudioQuantization quan,
                                UINT8 channel_num,
                                UINT32 info_struct)
{
	int audio_hdl=0;
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if(DECA_STATE_ATTACH!=dev->flags)
	{
		DECA_PRINTF("deca_open: warning- device is not in ATTACH status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	audio_hdl= open("/dev/ali_m36_audio0", O_RDWR);
	if(audio_hdl == 0)
	{
		DECA_PRINTF("open audio handle fail\n");
		return !RET_SUCCESS;
	}
	DECA_PRINTF("deca file handler : 0x%08x\n", audio_hdl);
	dev->flags=DECA_STATE_IDLE;
	DECA_PRINTF("open audio handle done\n");

	dev->priv=(void *)audio_hdl;

    m_deca_mutex_id = osal_mutex_create();
	if(m_deca_mutex_id == OSAL_INVALID_ID)
	{
		DECA_PRINTF("vdec create mutex fail\n");
	}
	
	DECA_PRINTF("%s success!\n", __FUNCTION__);
	return RET_SUCCESS;
}

RET_CODE deca_close(struct deca_device * dev)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if(DECA_STATE_IDLE!=dev->flags)
	{
		DECA_PRINTF("deca_close: warning- device is not in IDLE status! cur state:%d\n", dev->flags);
		if((DECA_STATE_PLAY==dev->flags)||(DECA_STATE_PAUSE==dev->flags))
			deca_stop(dev, 0, ADEC_STOP_IMM);
		else
			return !RET_SUCCESS;
	}

	close((int)dev->priv);
	dev->priv=NULL;
	dev->flags=DECA_STATE_ATTACH;
    if(m_deca_mutex_id != OSAL_INVALID_ID)
        osal_mutex_delete(m_deca_mutex_id);
	DECA_PRINTF("%s success!\n", __FUNCTION__);
	return RET_SUCCESS;
}

RET_CODE deca_set_sync_mode(struct deca_device * dev, enum ADecSyncMode mode)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if(DECA_STATE_ATTACH==dev->flags)
	{
		DECA_PRINTF("deca_set_sync_mode: warning- device is not in right status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	if(0==ioctl((int)dev->priv, AUDIO_SET_AV_SYNC, mode))
	{
		return RET_SUCCESS;
	}
	DECA_PRINTF("%s success!\n", __FUNCTION__);
	
	return !RET_SUCCESS;
}

RET_CODE deca_start(struct deca_device * dev, UINT32 high32_pts)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if((DECA_STATE_IDLE!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	{
		DECA_PRINTF("deca_start: warning- device is not in right status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	if(0==ioctl((int)dev->priv, AUDIO_PLAY, 0))
	{
		dev->flags=DECA_STATE_PLAY;
		return RET_SUCCESS;
	}

	DECA_PRINTF("%s success!\n", __FUNCTION__);
	
	return !RET_SUCCESS;
}

RET_CODE deca_stop(struct deca_device * dev, UINT32 high32_pts, enum ADecStopMode mode)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if((DECA_STATE_PLAY!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	{
		DECA_PRINTF("deca_stop: warning- device is not in right status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	if(0==ioctl((int)dev->priv, AUDIO_STOP, 0))
	{
		dev->flags=DECA_STATE_IDLE;
		return RET_SUCCESS;
	}
	DECA_PRINTF("%s success!\n", __FUNCTION__);
	return !RET_SUCCESS;
}

RET_CODE deca_pause(struct deca_device * dev)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);
     
	if(DECA_STATE_PLAY!=dev->flags)
	{
		DECA_PRINTF("deca_pause: warning- device is not in PLAY status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	if(0==ioctl((int)dev->priv, AUDIO_PAUSE, 0))
	{
		dev->flags=DECA_STATE_PAUSE;
		return RET_SUCCESS;
	}
	DECA_PRINTF("%s success!\n", __FUNCTION__);
	return !RET_SUCCESS;
}

RET_CODE deca_decore_ioctl(struct deca_device * dev, UINT32 cmd, void *param1, void *param2)
{
    struct ali_audio_rpc_pars rpc_pars;	
    RET_CODE ret = RET_SUCCESS;

    DECA_MUTEX_LOCK;

    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));

    switch(cmd)
    {
        case DECA_DECORE_INIT:
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 3;
            rpc_pars.arg[0].arg = (void *)&cmd;
        	rpc_pars.arg[0].arg_size = sizeof(cmd);	
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = sizeof(struct audio_config);
            rpc_pars.arg[2].arg = (void *)param2; 
            rpc_pars.arg[2].arg_size = 4;
            break;
        case DECA_DECORE_RLS:
        case DECA_DECORE_FLUSH:
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 3;
            rpc_pars.arg[0].arg = (void *)&cmd;
        	rpc_pars.arg[0].arg_size = sizeof(cmd);	
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 0;
            rpc_pars.arg[2].arg = (void *)param2; 
            rpc_pars.arg[2].arg_size = 0;
            break;
        case DECA_DECORE_SET_BASE_TIME:
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 3;
            rpc_pars.arg[0].arg = (void *)&cmd;
        	rpc_pars.arg[0].arg_size = sizeof(cmd);	
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[2].arg = (void *)param2; 
            rpc_pars.arg[2].arg_size = 0;
            break;
        case DECA_DECORE_PAUSE_DECODE:
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 3;
            rpc_pars.arg[0].arg = (void *)&cmd;
        	rpc_pars.arg[0].arg_size = sizeof(cmd);	
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[2].arg = (void *)param2; 
            rpc_pars.arg[2].arg_size = 4;
            break;
        case DECA_DECORE_GET_PCM_TRD:
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 3;
            rpc_pars.arg[0].arg = (void *)&cmd;
        	rpc_pars.arg[0].arg_size = sizeof(cmd);	
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[1].out = 1;
            rpc_pars.arg[2].arg = (void *)param2; 
            rpc_pars.arg[2].arg_size = 4;
            rpc_pars.arg[2].out = 1;
            break;
        default:
            return RET_FAILURE;
    }
    
	ret = ioctl((int)dev->priv, AUDIO_DECORE_COMMAND, &rpc_pars);

    DECA_MUTEX_UNLOCK;
    
    return ret;
}

RET_CODE deca_io_control(struct deca_device * dev, UINT32 cmd, UINT32 param)
{
	struct ali_audio_ioctl_command io_param;
	
	RET_CODE relt = RET_SUCCESS;
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s, cmd:%d.\n", __FUNCTION__, dev->name, cmd);
	if(DECA_STATE_ATTACH==dev->flags)
	{
		DECA_PRINTF("deca_io_control: warning- device is not in right status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}
	/*
	if(NULL!=dev->ase_cmd)
	{
		switch(cmd)
		{
		case DECA_AUDIO_KEY:
		case DECA_BEEP_INTERVAL:
		case DECA_STR_PLAY:
		case DECA_STR_PAUSE:
		case DECA_STR_RESUME:
		case DECA_STR_STOP:
			return dev->ase_cmd(dev, cmd, param);
		default:
			break;
		}
	}
    */
    
	switch(cmd)
	{
//	case DECA_AUDIO_KEY:
	case DECA_BEEP_INTERVAL:
    { 
		return ioctl((int)dev->priv,AUDIO_ASE_DECA_BEEP_INTERVAL,param); 
	}
//    case DECA_STR_PAUSE:
//    case DECA_STR_RESUME:
    case DECA_STR_STOP: 
    {
	    return ioctl((int)dev->priv,AUDIO_ASE_STR_STOP,param);
    }
    case DECA_STR_PLAY:
    {
        struct ali_audio_rpc_pars rpc_pars; 
        memset((void *)&rpc_pars, 0, sizeof(rpc_pars)); 
        rpc_pars.arg_num = 1;
        rpc_pars.arg[0].arg = (void *)param;
        rpc_pars.arg[0].arg_size = sizeof(struct ase_str_play_param); 
		return ioctl((int)dev->priv,AUDIO_ASE_STR_PLAY,&rpc_pars); 
    }
	default:
		break;
	}
    
	switch(cmd)
	{
	case DECA_SET_STR_TYPE:
		relt=ioctl((int)dev->priv, AUDIO_SET_STREAMTYPE, param);
		break;
//	case DECA_GET_STR_TYPE:
//		relt = RET_FAILURE;
//		break;
	case DECA_IO_GET_INPUT_CALLBACK_ROUTINE:
	{
		relt=ioctl((int)dev->priv, AUDIO_GET_INPUT_CALLBACK_ROUTINE, param);
		break;
	}

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
    case DECA_GET_AUDIO_INFO:
    case DECA_SET_REVERB:
    case DECA_SET_PL_II:
    case DECA_SOFTDEC_INIT:
    case DECA_SOFTDEC_CLOSE:
    case DECA_SOFTDEC_SET_TIME:
    case DECA_SOFTDEC_JUMP_TIME:
    case DECA_SOFTDEC_IS_PLAY_END:
    case DECA_SOFTDEC_INIT2:
    case DECA_SOFTDEC_CLOSE2:
    case DECA_SOFTDEC_CAN_DECODE2: 
    case DECA_GET_PLAY_PARAM: 
    case DECA_CHANGE_AUD_TRACK:
		io_param.ioctl_cmd=cmd;
		io_param.param=param;
		relt=ioctl((int)dev->priv, AUDIO_DECA_IO_COMMAND, &io_param);
		break;

	default:
		relt=!RET_SUCCESS;
		break;
    }
    DECA_PRINTF("%s finished, ret:%d\n", __FUNCTION__, relt);
    return relt;
}

void deca_init_ase(struct deca_device * dev)
{ 
	if(NULL==dev)return !RET_SUCCESS;
     
	if(DECA_STATE_IDLE!=dev->flags)
	{
		return !RET_SUCCESS;
	}

	if(0==ioctl((int)dev->priv, AUDIO_ASE_INIT, 0))
	{
		return RET_SUCCESS;
	}
	return !RET_SUCCESS;
}

RET_CODE deca_request_write(void  * device, UINT32 req_size, 
	                                        void ** ret_buf, UINT32 * ret_buf_size,
	                                        struct control_block * ctrl_blk)
{
	return RET_STA_ERR;
}

void deca_update_write(void * device, UINT32 size)
{
	return;
}

void deca_tone_voice(struct deca_device * dev, UINT32 SNR, UINT8 init)  //tone voice// clear waring 050907 ming yi
{
}

void deca_stop_tone_voice(struct deca_device * dev)  //tone voice // clear waring 050907 ming yi
{
}

void deca_process_pcm_samples(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, UINT32 sample_rate, UINT32 channel_num, UINT32 sample_precision)
{
}

void deca_m36_attach(struct deca_feature_config* config)
{
	struct deca_device*dev ;

	DECA_PRINTF("%s enter.\n", __FUNCTION__);
	//DECA_PRINTF("\ndeca_3601_atach:\tBase address: %08x\n",base_addr);
	dev=dev_alloc("DECA_S3601", HLD_DEV_TYPE_DECA, sizeof(struct deca_device));
	if(dev==NULL)
	{
		DECA_PRINTF("%s Alloc deca device error!\n", __FUNCTION__);
		while(1);//DECA_ASSERT(0);
	}


	/* Init deca_device element */
	dev->next=NULL ;

	/* Add this device to queue */
	if(dev_register(dev)!=RET_SUCCESS)
	{
		dev_free(dev);
		DECA_PRINTF("%s Register deca device failed!!!!!!!\n", __FUNCTION__);
		while(1);//DECA_ASSERT(0);
	}
/*
	//dev->minor = DECA_STATE_DETACH;
	if(config)
	{
 		need_detect_sprt_chg = config->detect_sprt_change;
	}
	deca_bs_buf_size=DECA_BS_BUFF_SIZE;
	deca_ctrl_blk_num = CTRBLKNUM;
*/
	dev->flags = DECA_STATE_ATTACH;
//	dev->ase_flags = DECA_STATE_IDLE;
//    deca_s3601_audiostream_decoder_install();
	//FOR DEBUG
	DECA_PRINTF("%s dev: %s Success!\n", __FUNCTION__, dev->name);
}


void deca_m36_dettach(struct deca_device*dev)
{
	if(DECA_STATE_ATTACH!=dev->flags)
	{
		DECA_PRINTF("%s error. Device status error.\n",__FUNCTION__);
		return;
	}
	dev_free(dev);
}


