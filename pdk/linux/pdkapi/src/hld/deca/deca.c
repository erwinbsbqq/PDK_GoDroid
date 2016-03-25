#include <hld_cfg.h>
#include <adr_basic_types.h>
#include <adr_mediatypes.h>
#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld/deca/adr_deca_dev.h>
#include <hld/deca/adr_deca.h>
#include <hld/snd/adr_snd_dev.h>
#include <hld/snd/adr_snd.h>

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

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <dvb_audio.h>


#ifdef ADR_IPC_ENABLE
#include <hld/misc/adr_ipc.h>
#endif

#define DECA_DBG_ENABLE	0
UINT8 deca_hld_dbg = 0;
#define DECA_PRINTF(fmt, args...)  \
			do { \
				if (deca_hld_dbg) \
				{ \
					ADR_DBG_PRINT(DECA, "%s->%s: L %d: "fmt"\n", __FILE__, \
						__FUNCTION__, __LINE__, ##args); \
				} \
			} while(0)

//struct snd_device * g_snd_deca_ref ;
void (*fp_deca_pcm_process_func)(UINT32, UINT8 *, UINT32, UINT32, UINT32) = NULL;
INT32 m_deca_mutex_id;

extern struct snd_device * g_snd_stc_ref;

#ifdef ADR_IPC_ENABLE
extern struct snd_hld_private *psnd_hld_priv;
#else
extern struct snd_hld_private snd_hld_priv;
#endif

extern int g_audio_attached;
#ifdef ADR_IPC_ENABLE
extern int g_audio_mutex_id; 

#define MUTEX_LOCK()   adr_ipc_semlock(g_audio_mutex_id)
#define MUTEX_UNLOCK() adr_ipc_semunlock(g_audio_mutex_id)
#else

#define MUTEX_LOCK()	do{}while(0)
#define MUTEX_UNLOCK() do{}while(0)

#define DECA_MUTEX_LOCK()   osal_mutex_lock(m_deca_mutex_id, OSAL_WAIT_FOREVER_TIME)
#define DECA_MUTEX_UNLOCK() osal_mutex_unlock(m_deca_mutex_id)
#endif

static int m_deca_opened = 0;
static int m_deca_file_handle = 0;
static struct deca_device *g_deca_info = NULL;

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

	//	if(DECA_STATE_ATTACH!=dev->flags)
	//	{
	//		DECA_PRINTF("deca_open: warning- device is not in ATTACH status! cur state:%d\n", dev->flags);
	//		return !RET_SUCCESS;
	//	}
#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		deca_m36_attach(NULL);
#endif
	
	if(m_deca_opened == 1) 
	{
		DECA_PRINTF("deca device already opened!!\n");
		return RET_SUCCESS;
	}
	
	MUTEX_LOCK();
	
	m_deca_file_handle = open("/dev/ali_m36_audio0", O_RDWR | O_CLOEXEC);
	if(m_deca_file_handle <= 0)
	{
		MUTEX_UNLOCK();
		
		DECA_PRINTF("open audio handle fail\n");
		return !RET_SUCCESS;
	}	
	
	DECA_PRINTF("deca file handler : 0x%08x\n", audio_hdl);
	dev->flags = DECA_STATE_IDLE;
	DECA_PRINTF("open audio handle done\n");

	g_deca_info = dev;

	m_deca_opened = 1;
	
#ifdef ADR_IPC_ENABLE

#else
	m_deca_mutex_id = osal_mutex_create();
	if(m_deca_mutex_id == OSAL_INVALID_ID)
	{
		DECA_PRINTF("vdec create mutex fail\n");
	}
#endif

	MUTEX_UNLOCK();
	
	DECA_PRINTF("%s success!\n", __FUNCTION__);
	return RET_SUCCESS;
}

RET_CODE deca_close(struct deca_device * dev)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		return RET_SUCCESS;
#endif

	if(m_deca_opened == 0) 
	{
		DECA_PRINTF("deca device already closed!!\n");
		return RET_SUCCESS;
	}

	MUTEX_LOCK();
	
	close(m_deca_file_handle);

#ifdef ADR_IPC_ENABLE

#else
	dev->priv=NULL;
#endif

	dev->flags=DECA_STATE_ATTACH;

	m_deca_opened = 0;

#ifdef ADR_IPC_ENABLE

#else
	if(m_deca_mutex_id != OSAL_INVALID_ID)
		osal_mutex_delete(m_deca_mutex_id);
#endif

	MUTEX_UNLOCK();

	DECA_PRINTF("%s success!\n", __FUNCTION__);
	return RET_SUCCESS;
}

RET_CODE deca_set_sync_mode(struct deca_device * dev, enum ADecSyncMode mode)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

	 MUTEX_LOCK();
	 
	if(DECA_STATE_ATTACH==dev->flags)
	{
		MUTEX_UNLOCK();
		
		DECA_PRINTF("deca_set_sync_mode: warning- device is not in right status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	if(0==ioctl(m_deca_file_handle, AUDIO_SET_AV_SYNC, mode))
	{
		MUTEX_UNLOCK();
		
		return RET_SUCCESS;
	}

	MUTEX_UNLOCK();
	
	DECA_PRINTF("%s failed!\n", __FUNCTION__);
	return !RET_SUCCESS;
}

RET_CODE deca_set_dbg_level(struct deca_device * dev,UINT32 option)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

	 MUTEX_LOCK();
	 
	if(DECA_STATE_ATTACH==dev->flags)
	{
		MUTEX_UNLOCK();
		
		DECA_PRINTF("deca_set_dbg_level: warning- device is not in right status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	if(0==ioctl(m_deca_file_handle, AUDIO_DECA_SET_DBG_LEVEL, option))
	{
		MUTEX_UNLOCK();
		
		return RET_SUCCESS;
	}

	MUTEX_UNLOCK();
	
	DECA_PRINTF("%s failed!\n", __FUNCTION__);
	return !RET_SUCCESS;
}

RET_CODE deca_start(struct deca_device * dev, UINT32 high32_pts)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

	 MUTEX_LOCK();
	 
	if((DECA_STATE_IDLE!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	{
		MUTEX_UNLOCK();
			
		DECA_PRINTF("deca_start: warning- device is not in right status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	if(0==ioctl(m_deca_file_handle, AUDIO_PLAY, 0))
	{
		dev->flags=DECA_STATE_PLAY;
		
		MUTEX_UNLOCK();		
		return RET_SUCCESS;
	}

	MUTEX_UNLOCK();

	DECA_PRINTF("%s failed!\n", __FUNCTION__);
	return !RET_SUCCESS;
}

RET_CODE deca_stop(struct deca_device * dev, UINT32 high32_pts, enum ADecStopMode mode)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

	 MUTEX_LOCK();
	 
	if((DECA_STATE_PLAY!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	{
		MUTEX_UNLOCK();
		
		DECA_PRINTF("deca_stop: warning- device is not in right status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	if(0==ioctl(m_deca_file_handle, AUDIO_STOP, 0))
	{
		dev->flags=DECA_STATE_IDLE;

		MUTEX_UNLOCK();
		return RET_SUCCESS;
	}

	MUTEX_UNLOCK();	
	DECA_PRINTF("%s failed!\n", __FUNCTION__);
	return !RET_SUCCESS;
}

RET_CODE deca_pause(struct deca_device * dev)
{
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

	MUTEX_LOCK();
	
	if(DECA_STATE_PLAY!=dev->flags)
	{
		MUTEX_UNLOCK();
		
		DECA_PRINTF("deca_pause: warning- device is not in PLAY status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	if(0==ioctl(m_deca_file_handle, AUDIO_PAUSE, 0))
	{
		MUTEX_UNLOCK();
		
		dev->flags=DECA_STATE_PAUSE;
		return RET_SUCCESS;
	}

	MUTEX_UNLOCK();
	
	DECA_PRINTF("%s failed!\n", __FUNCTION__);
	return !RET_SUCCESS;
}

RET_CODE deca_decore_ioctl(struct deca_device * dev, UINT32 cmd, void *param1, void *param2)
{
    struct ali_audio_rpc_pars rpc_pars;	
    RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
    MUTEX_LOCK();
#else
    DECA_MUTEX_LOCK();
#endif

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
        case DECA_DECORE_GET_CUR_TIME:
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
#ifdef ADR_IPC_ENABLE
		MUTEX_UNLOCK();
#else
		DECA_MUTEX_UNLOCK();
#endif
            return RET_FAILURE;
    }
    
	ret = ioctl(m_deca_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars);

#ifdef ADR_IPC_ENABLE
	MUTEX_UNLOCK();
#else
    	DECA_MUTEX_UNLOCK();
#endif

    return ret;
}

RET_CODE deca_io_control(struct deca_device * dev, UINT32 cmd, UINT32 param)
{
	struct ali_audio_ioctl_command io_param;
	
	RET_CODE relt = RET_SUCCESS;
	if(NULL==dev)return !RET_SUCCESS;
	DECA_PRINTF("%s enter, dev name:%s, cmd:%d.\n", __FUNCTION__, dev->name, cmd);

	MUTEX_LOCK();
	
	if(DECA_STATE_ATTACH==dev->flags)
	{
		MUTEX_UNLOCK();
		
		DECA_PRINTF("deca_io_control: warning- device is not in right status! cur state:%d\n", dev->flags);
		return !RET_SUCCESS;
	}

	switch(cmd)
	{
        case DECA_BEEP_INTERVAL:
        {  
            ioctl(m_deca_file_handle,AUDIO_ASE_DECA_BEEP_INTERVAL,param); 
            break;
        }
        
        case DECA_STR_STOP: 
        {
            ioctl(m_deca_file_handle,AUDIO_ASE_STR_STOP,param);
            break;
        }
        
        case DECA_STR_PLAY:
        {
            struct ali_audio_rpc_pars rpc_pars; 
            memset((void *)&rpc_pars, 0, sizeof(rpc_pars)); 
            rpc_pars.arg_num = 1;
            rpc_pars.arg[0].arg = (void *)param;
            rpc_pars.arg[0].arg_size = sizeof(struct ase_str_play_param); 
            ioctl(m_deca_file_handle,AUDIO_ASE_STR_PLAY,&rpc_pars); 
            break;
        }

        case DECA_SET_STR_TYPE:
        {    relt=ioctl(m_deca_file_handle, AUDIO_SET_STREAMTYPE, param);
             break;
        }
        
        case DECA_IO_GET_INPUT_CALLBACK_ROUTINE:
        {
            relt=ioctl(m_deca_file_handle, AUDIO_GET_INPUT_CALLBACK_ROUTINE, param);
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
        case DECA_INDEPENDENT_DESC_ENABLE:
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
        case DECA_PCM_SIGNED_SET:
        case DECA_SET_CACHE_INVALID_FLAG:
 	    case DECA_SET_QUICK_PLAY_MODE:
        case DECA_DTS_FALL_BACK:
        case DECA_CHANGE_AUD_TRACK:
	    {
            io_param.ioctl_cmd=cmd;
            io_param.param=param;
            relt=ioctl(m_deca_file_handle, AUDIO_DECA_IO_COMMAND, &io_param);
            break;
        }
          
        case DECA_GET_PLAY_PARAM: 
        {
		struct ali_audio_rpc_pars rpc_pars;
		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.API_ID = AUDIO_DECA_IO_COMMAND_ADV;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)&cmd;
		rpc_pars.arg[0].arg_size = sizeof(cmd);			
		rpc_pars.arg[1].arg = (void *)param;
		rpc_pars.arg[1].arg_size = sizeof(struct cur_stream_info);
		rpc_pars.arg[1].out = 1;
		relt = ioctl(m_deca_file_handle,AUDIO_RPC_CALL_ADV , &rpc_pars);
		if(relt < 0)
			relt = !RET_SUCCESS;	
		break;
        }
        case DECA_GET_ES_BUFF_STATE:
        {
		struct ali_audio_rpc_pars rpc_pars;
		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.API_ID = AUDIO_DECA_IO_COMMAND_ADV;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)&cmd;
		rpc_pars.arg[0].arg_size = sizeof(cmd);			
		rpc_pars.arg[1].arg = (void *)param;
		rpc_pars.arg[1].arg_size = sizeof(struct deca_buf_info);
		rpc_pars.arg[1].out = 1;
		relt = ioctl(m_deca_file_handle,AUDIO_RPC_CALL_ADV , &rpc_pars);
		if(relt < 0)
			relt = !RET_SUCCESS;	
		break;
        }
        case DECA_GET_AUDIO_INFO:
        {
		struct ali_audio_rpc_pars rpc_pars;
		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.API_ID = AUDIO_DECA_IO_COMMAND_ADV;
		rpc_pars.arg_num = 2;
		rpc_pars.arg[0].arg = (void *)&cmd;
		rpc_pars.arg[0].arg_size = sizeof(cmd);			
		rpc_pars.arg[1].arg = (void *)param;
		rpc_pars.arg[1].arg_size = sizeof(struct AUDIO_INFO);
		rpc_pars.arg[1].out = 1;
		relt = ioctl(m_deca_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars);
		if(relt < 0)
			relt = !RET_SUCCESS;	
		break;
        }
	default:
		relt=!RET_SUCCESS;
		break;
    }

    MUTEX_UNLOCK();
	
    DECA_PRINTF("%s finished, ret:%d\n", __FUNCTION__, relt);
    return relt;
}

void deca_init_ase(struct deca_device * dev)
{ 
	if(NULL==dev)return;

	 MUTEX_LOCK();
	 
	if(DECA_STATE_IDLE!=dev->flags)
	{
		MUTEX_UNLOCK();
		return;
	}

	if(0==ioctl(m_deca_file_handle, AUDIO_ASE_INIT, 0))
	{
		MUTEX_UNLOCK();
		
		return;
	}

	MUTEX_UNLOCK();
	return;
}

UINT8 *req_buf = NULL;
struct control_block audio_cb;
UINT8 cb_avail = 0;
RET_CODE deca_request_write(void  * device, UINT32 req_size, 
	                                        void ** ret_buf, UINT32 * ret_buf_size,
	                                        struct control_block * ctrl_blk)
{
	struct deca_device *dev = (struct deca_device *)device;
	UINT8 ret;
      if(req_size == 0)
         return RET_SUCCESS;

	  MUTEX_LOCK();
	if(ctrl_blk)
	{
		memcpy(&audio_cb,ctrl_blk,sizeof(struct control_block));
		cb_avail = 1;
	}

	  if(req_size > 188)
	  	req_size = 188;
	  
       req_buf = (UINT8 *)malloc(req_size);
	if(req_buf == NULL)
	{
		MUTEX_UNLOCK();
		
		DECA_PRINTF("malloc request buffer fail\n");
		return RET_STA_ERR;
	}

      *ret_buf = req_buf;
	*ret_buf_size = req_size;

	MUTEX_UNLOCK();
	return RET_SUCCESS;   
}

void deca_update_write(void * device, UINT32 size)
{
	struct deca_device *dev = (struct deca_device*)device;
	INT32 write_size = 0;

	MUTEX_LOCK();
    	UINT32 update_size= 0;
	
	 if(cb_avail)
	{
		ioctl(m_deca_file_handle, AUDIO_SET_CTRL_BLK_INFO, &audio_cb);
		cb_avail = 0;
	}

	 write_size = write(m_deca_file_handle, (void *)(req_buf + update_size),size);
	if(write_size == size)
	{
	free((void *)req_buf);
	 req_buf = NULL;
	}

		MUTEX_UNLOCK();
	
	return;
}

void deca_tone_voice(struct deca_device * dev, UINT32 SNR, UINT8 init)  //tone voice// clear waring 050907 ming yi
{
	if(NULL==dev)return;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

	 MUTEX_LOCK();
	 
	if((DECA_STATE_PLAY!=dev->flags)&&(DECA_STATE_IDLE!=dev->flags))
	{
		MUTEX_UNLOCK();
		
		DECA_PRINTF("%s: warning- device is not in right status! cur state:%d\n", __FUNCTION__, dev->flags);
		return;
	}

	if(0==ioctl(m_deca_file_handle, AUDIO_GEN_TONE_VOICE, SNR))
	{
		MUTEX_UNLOCK();
		
		return;
	}
	
	MUTEX_UNLOCK();
	
	DECA_PRINTF("%s failed!\n", __FUNCTION__);
	return;
}

void deca_stop_tone_voice(struct deca_device * dev)  //tone voice // clear waring 050907 ming yi
{
	if(NULL==dev)return;
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__, dev->name);

	MUTEX_LOCK();
	
	if((DECA_STATE_PLAY!=dev->flags)&&(DECA_STATE_IDLE!=dev->flags))
	{
		MUTEX_UNLOCK();
		
		DECA_PRINTF("%s: warning- device is not in right status! cur state:%d\n", __FUNCTION__, dev->flags);
		return;
	}

	if(0==ioctl(m_deca_file_handle, AUDIO_STOP_TONE_VOICE, 0))
	{
		MUTEX_UNLOCK();
		
		return;
	}

	MUTEX_UNLOCK();
	
	DECA_PRINTF("%s failed!\n", __FUNCTION__);
	return;
}

void deca_process_pcm_samples(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, UINT32 sample_rate, UINT32 channel_num, UINT32 sample_precision)
{
	DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__);

	MUTEX_LOCK();

	if(DECA_STATE_PLAY== g_deca_info->flags)
	{
		MUTEX_UNLOCK();

		DECA_PRINTF("%s: cann't play pcm and ts at the same time!!%d\n", __FUNCTION__, g_deca_info->flags);
		return;
	}

	UINT8 ret =0;
	struct ali_audio_rpc_pars rpc_pars;
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = AUDIO_DECA_PROCESS_PCM_SAMPLES;
	rpc_pars.arg_num = 5;

	rpc_pars.arg[0].arg = (void *)&pcm_bytes_len;
	rpc_pars.arg[0].arg_size = sizeof(pcm_bytes_len);	
	rpc_pars.arg[0].out = 0; 

	rpc_pars.arg[1].arg = (void *)&pcm_raw_buf;
	rpc_pars.arg[1].arg_size = 4;
	rpc_pars.arg[1].out = 0; 

	rpc_pars.arg[2].arg = (void *)&sample_rate;
	rpc_pars.arg[2].arg_size = sizeof(sample_rate);	
	rpc_pars.arg[2].out = 0;     

	rpc_pars.arg[3].arg = (void *)&channel_num;
	rpc_pars.arg[3].arg_size = sizeof(channel_num);	
	rpc_pars.arg[3].out = 0;     

	rpc_pars.arg[4].arg = (void *)&sample_precision;
	rpc_pars.arg[4].arg_size = sizeof(sample_precision);	
	rpc_pars.arg[4].out = 0; 
	ret = ioctl(m_deca_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars);     

	MUTEX_UNLOCK();
}

void deca_process_pcm_bitstream(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, UINT32 bs_length, UINT8 *un_processed_bs, UINT32 sample_rate, UINT32 channel_num, UINT32 sample_precision)
{
     DECA_PRINTF("%s enter, dev name:%s.\n", __FUNCTION__);

     MUTEX_LOCK();

    if(DECA_STATE_PLAY== g_deca_info->flags)
    {
       MUTEX_UNLOCK();
	   
       DECA_PRINTF("%s: cann't play pcm and ts at the same time!!%d\n", __FUNCTION__, g_deca_info->flags);
        return;
    }
	
    UINT8 ret =0;
    struct ali_audio_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = AUDIO_DECA_PROCESS_PCM_BITSTREAM;
    rpc_pars.arg_num = 7;

    rpc_pars.arg[0].arg = (void *)&pcm_bytes_len;
    rpc_pars.arg[0].arg_size = sizeof(pcm_bytes_len);	
    rpc_pars.arg[0].out = 0; 

    rpc_pars.arg[1].arg = (void *)&pcm_raw_buf;
    rpc_pars.arg[1].arg_size = 4;
    rpc_pars.arg[1].out = 0; 

    rpc_pars.arg[2].arg = (void *)&bs_length;
    rpc_pars.arg[2].arg_size = sizeof(bs_length);	
    rpc_pars.arg[2].out = 0; 

    rpc_pars.arg[3].arg = (void *)&un_processed_bs;
    rpc_pars.arg[3].arg_size = 4;
    rpc_pars.arg[3].out = 0; 
    
    rpc_pars.arg[4].arg = (void *)&sample_rate;
    rpc_pars.arg[4].arg_size = sizeof(sample_rate);	
    rpc_pars.arg[4].out = 0;     

    rpc_pars.arg[5].arg = (void *)&channel_num;
    rpc_pars.arg[5].arg_size = sizeof(channel_num);	
    rpc_pars.arg[5].out = 0;     

    rpc_pars.arg[6].arg = (void *)&sample_precision;
    rpc_pars.arg[6].arg_size = sizeof(sample_precision);	
    rpc_pars.arg[6].out = 0; 
    ret = ioctl(m_deca_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars);      

    MUTEX_UNLOCK();	
}

void deca_m36_attach(struct deca_feature_config* config)
{
	struct deca_device*dev = NULL;
	struct snd_device*snd_dev = NULL;
		
#ifdef ADR_IPC_ENABLE	
	int shmid = 0;
#endif

	if(g_audio_attached)
	{
		DECA_PRINTF("auidio is already attached\n");
		return;
	}

#ifdef ADR_IPC_ENABLE
	g_audio_mutex_id = adr_ipc_semget(ADR_IPC_DECA, 0, 1);
	if(g_audio_mutex_id < 0)
	{
		DECA_PRINTF("audio create mutex fail\n");
		return;
	}
#endif

	{
		DECA_PRINTF("%s enter.\n", __FUNCTION__);
		dev=dev_alloc("DECA_S3601", HLD_DEV_TYPE_DECA, sizeof(struct deca_device));
		if(dev==NULL)
		{
			DECA_PRINTF("%s Alloc deca device error!\n", __FUNCTION__);
			return;
		}

#ifdef ADR_IPC_ENABLE

#else
		/* Init deca_device element */
		dev->next = NULL ;
#endif

		/* Add this device to queue */
		if(dev_register(dev)!=RET_SUCCESS)
		{
			dev_free(dev);
			DECA_PRINTF("%s Register deca device failed!!!!!!!\n", __FUNCTION__);
			return;
		}
		
		MUTEX_LOCK();		
		
		dev->flags = DECA_STATE_ATTACH;
		
		MUTEX_UNLOCK();
		
		DECA_PRINTF("%s dev: %s Success!\n", __FUNCTION__, dev->name);

      		decadbg_module_register();

	}

	{		
		snd_dev=dev_alloc("SND_S3601", HLD_DEV_TYPE_SND, sizeof(struct snd_device));
		if(snd_dev==NULL)
		{
			DECA_PRINTF("Alloc snd device error!\n");
			return;
		}

		/* Add this device to queue */
		if(dev_register(snd_dev)!=SUCCESS)
		{
			dev_free(snd_dev);
			DECA_PRINTF("Register sound device failed!!!!!!!\n");
			return;
		}

		MUTEX_LOCK();
		
#ifdef ADR_IPC_ENABLE
		if(shmid = adr_ipc_shmget(ADR_IPC_SND, 0
			, (void **)&psnd_hld_priv, sizeof(*psnd_hld_priv)) < 0)
		{
			MUTEX_UNLOCK();
			
			DECA_PRINTF("get snd shm fail\n");
			return;			
		}

		DECA_PRINTF("get the shmd %d\n", shmid);
		
		if(psnd_hld_priv->inited == 0)
		{
			memset((void *)psnd_hld_priv, 0, sizeof(*psnd_hld_priv));
			psnd_hld_priv->shmid = shmid;
			psnd_hld_priv->stc_divisor=299;
			psnd_hld_priv->spdif_type=SND_OUT_SPDIF_INVALID;
			psnd_hld_priv->inited = 1;
			
			snd_dev->priv = (void *)0;
						
			DECA_PRINTF("init snd shm\n");	
			
		}		
		
		g_snd_stc_ref=snd_dev;
#else
		snd_dev->priv=NULL;

		g_snd_stc_ref=snd_dev;
		memset(&snd_hld_priv, 0, sizeof(struct snd_hld_private));
		snd_hld_priv.stc_divisor=299;
		snd_hld_priv.spdif_type=SND_OUT_SPDIF_INVALID;
#endif

		MUTEX_UNLOCK();
		
		DECA_PRINTF("\n snd_s3601_attach:\t Sound attach success! \n");

		snddbg_module_register();
	}

	g_audio_attached = 1;

	DECA_PRINTF("audio attached done\n");
			
#ifdef ADR_IPC_ENABLE
	deca_open(dev, 0, 0, 0, 0, 0);
	snd_open(snd_dev);
#endif		
	return;
}

void deca_m36_dettach(struct deca_device*dev)
{
	return;
}

