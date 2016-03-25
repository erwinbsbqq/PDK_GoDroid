#include <hld_cfg.h>
#include <adr_basic_types.h>
#include <adr_mediatypes.h>
#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld/snd/adr_snd_dev.h>
#include <hld/snd/adr_snd.h>
#include <hld/deca/adr_deca_dev.h>
#include <hld/deca/adr_deca.h>

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

#define SND_DBG_ENABLE	0
#define SND_PRINTF(fmt, args...)  \
			do { \
				if (SND_DBG_ENABLE) \
				{ \
					ADR_DBG_PRINT(SND, "%s %s L%d: "fmt"\n", __FILE__, \
						__FUNCTION__, __LINE__, ##args); \
				} \
			} while(0)

#define ENTRY()		SND_PRINTF("in\n")
#define EXIT()		SND_PRINTF("out\n")

struct deca_device * g_deca_snd_ref ;
struct snd_device * g_snd_stc_ref = NULL;

#ifdef ADR_IPC_ENABLE
struct snd_hld_private *psnd_hld_priv;
#else
struct snd_hld_private snd_hld_priv;
#endif

int g_audio_attached = 0;
static int m_snd_opened = 0;
static int m_snd_file_handle = 0;
#ifdef ADR_IPC_ENABLE
int g_audio_mutex_id = -1;
#endif

#ifdef ADR_IPC_ENABLE
#define MUTEX_LOCK()   adr_ipc_semlock(g_audio_mutex_id)
#define MUTEX_UNLOCK() adr_ipc_semunlock(g_audio_mutex_id)
#else
#define MUTEX_LOCK()   do{}while(0)
#define MUTEX_UNLOCK() do{}while(0)
#endif

RET_CODE get_stc(UINT32 * stc_msb32, UINT8 stc_num)
{
	UINT8 ret = RET_SUCCESS;
	struct ali_audio_rpc_pars rpc_pars;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return !RET_SUCCESS;
	}

	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = AUDIO_SND_GET_STC;
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)stc_msb32;
	rpc_pars.arg[0].arg_size = sizeof(stc_msb32);	
	rpc_pars.arg[0].out = 1;
	rpc_pars.arg[1].arg = (void *)&stc_num;
	rpc_pars.arg[1].arg_size = sizeof(stc_num);
	rpc_pars.arg[1].out = 0;
	ret = ioctl(m_snd_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars);        

	EXIT();	
	MUTEX_UNLOCK();

	return ret;
}

void set_stc(UINT32 stc_msb32, UINT8 stc_num)
{    
	UINT8 ret =0;
	struct ali_audio_rpc_pars rpc_pars;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return;
	}

	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = AUDIO_SND_SET_STC;
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)&stc_msb32;
	rpc_pars.arg[0].arg_size = sizeof(stc_msb32);	
	rpc_pars.arg[0].out = 0;
	rpc_pars.arg[1].arg = (void *)&stc_num;
	rpc_pars.arg[1].arg_size = sizeof(stc_num);
	rpc_pars.arg[1].out = 0;
	ret = ioctl(m_snd_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars);        	

	EXIT();	
	MUTEX_UNLOCK();
	
	return;
}

void get_stc_divisor(UINT16 * stc_divisor, UINT8 stc_num)
{
#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

#ifdef ADR_IPC_ENABLE
	*stc_divisor=psnd_hld_priv->stc_divisor;
#else
	*stc_divisor=snd_hld_priv.stc_divisor;
#endif

	EXIT();	
	MUTEX_UNLOCK();	
}

void set_stc_divisor(UINT16 stc_divisor, UINT8 stc_num)
{
#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

#ifdef ADR_IPC_ENABLE
	psnd_hld_priv->stc_divisor=stc_divisor;
#else
	snd_hld_priv.stc_divisor=stc_divisor;
#endif

	EXIT();	
	MUTEX_UNLOCK();	
}

void stc_pause(UINT8 pause, UINT8 stc_num)
{
#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	struct ali_audio_rpc_pars rpc_pars;

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return;
	}

	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = AUDIO_SND_PAUSE_STC;
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)&pause;
	rpc_pars.arg[0].arg_size = sizeof(pause);
	rpc_pars.arg[0].out  = 0;
	rpc_pars.arg[1].arg = (void *)&stc_num;
	rpc_pars.arg[1].arg_size = sizeof(stc_num);
	rpc_pars.arg[1].out  = 0;
	ioctl(m_snd_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars); 

	EXIT();	
	MUTEX_UNLOCK();	
	return;
}

void stc_invalid(void)
{
#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return;
	}

	ioctl(m_snd_file_handle, AUDIO_SND_STC_INVALID, 0);

	EXIT();	
	MUTEX_UNLOCK();	
	return;
}

void stc_valid(void)
{
#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return;
	}

	ioctl(m_snd_file_handle, AUDIO_SND_STC_VALID, 0);    
	
	EXIT();	
	MUTEX_UNLOCK();	
	return;
}

void snd_output_config(struct snd_device * dev, struct snd_output_cfg * cfg_param)
{
/*
	if(dev&&dev->output_config)
		dev->output_config(dev, cfg_param);
*/
}

RET_CODE snd_open(struct snd_device * dev)
{
	int audio_hdl=0;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	if(NULL == dev) 
		return !RET_SUCCESS;

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 1)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd device already opened!!!\n");
		return RET_SUCCESS;
	}
	
	m_snd_file_handle = open("/dev/ali_m36_audio0", O_RDWR | O_CLOEXEC);
	if(m_snd_file_handle <= 0)
	{
		MUTEX_UNLOCK();

		
		SND_PRINTF("open auido dev fail\n");
		return !RET_SUCCESS;
	}

	m_snd_opened = 1;
	
	SND_PRINTF("%s leave, dev name:%s.\n", __FUNCTION__, dev->name);

	EXIT();
	MUTEX_UNLOCK();
		
	return RET_SUCCESS;
}

RET_CODE snd_close(struct snd_device * dev)
{
#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		return RET_SUCCESS;
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd device already closed!!!\n");
		return RET_SUCCESS;
	}

	close(m_snd_file_handle);

	m_snd_opened = 0;
	
	EXIT();
	MUTEX_UNLOCK();
	
	return RET_SUCCESS;
}

RET_CODE snd_set_mute(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 enable)
{
	if(dev == NULL)
		return !RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return !RET_SUCCESS;
	}

	if(0==ioctl(m_snd_file_handle, AUDIO_SET_MUTE, enable))
	{
		EXIT();
		MUTEX_UNLOCK();

		SND_PRINTF("set mute done\n");
		return RET_SUCCESS;
	}

	EXIT();	
	MUTEX_UNLOCK();
	
	return !RET_SUCCESS;
}

RET_CODE snd_set_volume(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 volume)
{
	if(dev == NULL)
		return !RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return !RET_SUCCESS;
	}

#ifdef ADR_IPC_ENABLE
	psnd_hld_priv->volume = volume;
#else
	snd_hld_priv.volume = volume;
#endif

	if(0==ioctl(m_snd_file_handle, AUDIO_SET_VOLUME, volume))
	{
		EXIT();
		MUTEX_UNLOCK();
		
		return RET_SUCCESS;
	}

	EXIT();
	MUTEX_UNLOCK();

	return !RET_SUCCESS;
}

RET_CODE snd_set_dbg_level(struct snd_device * dev, UINT32 option)
{
	if(dev == NULL)
		return !RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return !RET_SUCCESS;
	}
     
	if(0==ioctl(m_snd_file_handle, AUDIO_SND_SET_DBG_LEVEL, option))
	{
		EXIT();
		MUTEX_UNLOCK();
		
		return RET_SUCCESS;
	}

	EXIT();
	MUTEX_UNLOCK();
		
	return !RET_SUCCESS;
}

UINT8 snd_get_volume(struct snd_device * dev)
{	
#ifdef ADR_IPC_ENABLE
	return psnd_hld_priv->volume;
#else
	return snd_hld_priv.volume;
#endif
}

RET_CODE snd_data_enough(struct snd_device * dev)
{
	return !RET_SUCCESS;
}

RET_CODE snd_request_pcm_buff(struct snd_device * dev, UINT32 size)
{
	return !RET_SUCCESS;
}

void snd_write_pcm_data(struct snd_device*dev,struct pcm_output*pcm,UINT32*frame_header)
{
	return;
}

void snd_write_pcm_data2(struct snd_device * dev, UINT32 * frame_header,
	                                                UINT32 * left, UINT32 * right, UINT32 number, UINT32 ch_num)
{
	return;
}

RET_CODE snd_io_control(struct snd_device * dev, UINT32 cmd, UINT32 param)
{
	struct ali_audio_ioctl_command io_param;
	int ret = RET_SUCCESS;
	
	if(NULL==dev)
		return !RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return !RET_SUCCESS;
	}

	/*
	if(NULL!=dev->spectrum_cmd)
	{
		switch(cmd)
		{
			case SND_SPECTRUM_START:
			case SND_SPECTRUM_STOP:
			case SND_SPECTRUM_CLEAR:
			case SND_SPECTRUM_STEP_TABLE:
			case SND_SPECTRUM_VOL_INDEPEND:
			case SND_SPECTRUM_CAL_COUNTER:
				return dev->spectrum_cmd(dev, cmd, param);
			default:
				break;
		}
	}


	if(NULL!=dev->ioctl)
	{
		return dev->ioctl(dev, cmd, param);
	}
	*/
	switch(cmd)
	{
		case IS_SND_RUNNING:
		case IS_SND_MUTE:
		case SND_CC_MUTE:
		case SND_DAC_MUTE:
		case SND_CC_MUTE_RESUME:
		case SND_SPO_ONOFF:
		case SND_SET_FADE_SPEED:
		case IS_PCM_EMPTY:
		case SND_BYPASS_VCR:
		case SND_CHK_SPDIF_TYPE:
		case SND_BASS_TYPE:
		case SND_PAUSE_MUTE:
		case SND_SET_DESC_VOLUME_OFFSET:
		case SND_SET_BS_OUTPUT_SRC:
		case SND_SECOND_DECA_ENABLE:    
		case SND_DO_DDP_CERTIFICATION:
		case SND_CHK_DAC_PREC:
		case SND_POST_PROCESS_0:
		case SND_SPECIAL_MUTE_REG:
		case SND_SET_SYNC_DELAY:
		case SND_SET_SYNC_LEVEL:
		case SND_SET_MUTE_TH:
		case SND_AUTO_RESUME:
		case SND_REG_HDMI_CB:
		case SND_REQ_REM_DATA:
		case SND_GET_TONE_STATUS:
		case SND_CHK_PCM_BUF_DEPTH:
		case SND_GET_SAMPLES_REMAIN:
		case SND_REQ_REM_PCM_DATA:
		case SND_REQ_REM_PCM_DURA:
		case SND_GET_MUTE_TH:
		case SND_SPECTRUM_START:
		case SND_SPECTRUM_STEP_TABLE:
		case SND_SPECTRUM_STOP:
		case SND_SPECTRUM_CLEAR:
		case SND_SPECTRUM_VOL_INDEPEND:
		case SND_SPECTRUM_CAL_COUNTER:
		case SND_SET_SPDIF_SCMS:
		case SND_SET_SYNC_PARAM: 
		case SND_GET_RAW_PTS:
		case SND_I2S_OUT:
		case SND_SPDIF_OUT:
		case SND_HDMI_OUT:
		
    /*Begin:Added by kinson for kakaok*/
	case SND_I2SIN_MIC0_GET_ENABLE:
	case SND_I2SIN_MIC0_SET_ENABLE:
	case SND_I2SIN_MIC0_GET_VOLUME:
	case SND_I2SIN_MIC0_SET_VOLUME:
	case SND_I2SIN_MIC0_START:
	case SND_I2SIN_MIC0_STOP:
		
	case SND_I2SIN_MIC1_GET_ENABLE:
	case SND_I2SIN_MIC1_SET_ENABLE:
	case SND_I2SIN_MIC1_GET_VOLUME:
	case SND_I2SIN_MIC1_SET_VOLUME:
	case SND_I2SIN_MIC1_START:
	case SND_I2SIN_MIC1_STOP:
    /*End:Added by kinson for kakaok*/
	
			io_param.ioctl_cmd=cmd;
			io_param.param=param;		
			ret = ioctl(m_snd_file_handle, AUDIO_SND_IO_COMMAND, &io_param);
			break;
		case FORCE_SPDIF_TYPE:
#ifdef ADR_IPC_ENABLE
			psnd_hld_priv->spdif_type=(enum ASndOutSpdifType)param;
#else
			snd_hld_priv.spdif_type=(enum ASndOutSpdifType)param;
#endif
			io_param.ioctl_cmd=cmd;
			io_param.param=param;
			ret = ioctl(m_snd_file_handle, AUDIO_SND_IO_COMMAND, &io_param);
			break;
		case SND_GET_SPDIF_TYPE:
		{
			enum ASndOutSpdifType *p_spdif;
			if (param == 0)
				break;
			p_spdif = (enum ASndOutSpdifType *)param;
#ifdef ADR_IPC_ENABLE
			*p_spdif = psnd_hld_priv->spdif_type;
#else
			*p_spdif = snd_hld_priv.spdif_type;
#endif
			break;
		}
		case SND_GET_STATUS:
		{
			struct ali_audio_rpc_pars rpc_pars;
			memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
			rpc_pars.API_ID = AUDIO_SND_IO_COMMAND_ADV;
			rpc_pars.arg_num = 2;
			rpc_pars.arg[0].arg = (void *)&cmd;
			rpc_pars.arg[0].arg_size = sizeof(cmd);			
			rpc_pars.arg[1].arg = (void *)param;
			rpc_pars.arg[1].arg_size = sizeof(struct snd_dev_status);
			rpc_pars.arg[1].out  = 1;
			ret = ioctl(m_snd_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars);            
			break;
		}
		/*	
		case SND_REQ_REM_DATA:
			if(param)
				*(UINT32*)param = snd_s3601_data_remain(dev);
			break;
		case SND_GET_SAMPLES_REMAIN:
			if(param)
				*(UINT32*)param = snd_s3601_samples_remain(dev);
			break;
		case SND_CHK_PCM_BUF_DEPTH:
			if(param)
				*((UINT32 *)param) = pcm_buf_depth;
			break;
			}
		*/
		default:
			break;
	}

	EXIT();
	MUTEX_UNLOCK();
	
	return ret;
}

RET_CODE snd_set_sub_blk(struct snd_device * dev, UINT8 sub_blk, UINT8 enable)
{
	UINT8 ret =0;
	struct ali_audio_rpc_pars rpc_pars;
	
	if(dev == NULL)
		return !RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return !RET_SUCCESS;
	}
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = AUDIO_SND_SET_SUB_BLK;
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)&sub_blk;
	rpc_pars.arg[0].arg_size = sizeof(sub_blk);	
	rpc_pars.arg[0].out = 0;
	rpc_pars.arg[1].arg = (void *)&enable;
	rpc_pars.arg[1].arg_size = sizeof(enable);
	rpc_pars.arg[1].out = 0;
	ret = ioctl(m_snd_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars);    

	EXIT();
	MUTEX_UNLOCK();
	
	return ret;
}

RET_CODE snd_set_duplicate(struct snd_device * dev, enum SndDupChannel channel)
{
	if(dev == NULL)
		return !RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return !RET_SUCCESS;
	}

	if(0==ioctl(m_snd_file_handle, AUDIO_CHANNEL_SELECT, channel))
	{
		EXIT();
		MUTEX_UNLOCK();
	
		return RET_SUCCESS;
	}

	EXIT();
	MUTEX_UNLOCK();

	return !RET_SUCCESS;
}

RET_CODE snd_set_spdif_type(struct snd_device * dev, enum ASndOutSpdifType type)
{
	if(dev == NULL)
		return !RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return !RET_SUCCESS;
	}

#ifdef ADR_IPC_ENABLE
	psnd_hld_priv->spdif_type=type;
#else
	snd_hld_priv.spdif_type=type;
#endif

	if(0==ioctl(m_snd_file_handle, AUDIO_SET_BYPASS_MODE, type))
	{
		EXIT();
		MUTEX_UNLOCK();
	
		return RET_SUCCESS;
	}
	
	EXIT();
	MUTEX_UNLOCK();

	return !RET_SUCCESS;
}

RET_CODE snd_config(struct snd_device * dev, UINT32 sample_rate, UINT16 sample_num, UINT8 precision)
{
	return !RET_SUCCESS;
}

void snd_start(struct snd_device * dev)
{
 	if(dev == NULL)
		return;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return;
	}

	ioctl(m_snd_file_handle, AUDIO_SND_START, 0);   

	EXIT();
	MUTEX_UNLOCK();	
}

void snd_stop(struct snd_device*dev)
{
 	if(dev == NULL)return;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return;
	}

	ioctl(m_snd_file_handle, AUDIO_SND_STOP, 0);   

	EXIT();
	MUTEX_UNLOCK();	
}

void snd_gen_tone_voice(struct snd_device * dev, struct pcm_output*pcm, UINT8 init)  //tone voice
{
}

void snd_stop_tone_voice(struct snd_device * dev)  //tone voice
{
}

RET_CODE snd_ena_pp_8ch(struct snd_device * dev, UINT8 enable)
{
	return RET_SUCCESS;
}

RET_CODE snd_set_pp_delay(struct snd_device * dev, UINT8 delay)
{
	return RET_SUCCESS;
}

RET_CODE snd_enable_virtual_surround(struct snd_device * dev, UINT8 enable)
{
	return RET_SUCCESS;
}

RET_CODE snd_enable_bass(struct snd_device * dev, UINT8 enable)
{
	return RET_SUCCESS;	
}

RET_CODE snd_enable_eq(struct snd_device * dev, UINT8 enable, enum EQ_TYPE type)
{
	if(dev == NULL)
		return !RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(g_audio_attached == 0)
		snd_m36_attach(NULL);
#endif

	MUTEX_LOCK();	
	ENTRY();

	if(m_snd_opened == 0)
	{
		MUTEX_UNLOCK();
		
		SND_PRINTF("snd is don't be opened!!!\n");
		return !RET_SUCCESS;
	}
	
	UINT8 ret =0;
	struct ali_audio_rpc_pars rpc_pars;
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = AUDIO_SND_ENABLE_EQ;
	rpc_pars.arg_num = 2;
	rpc_pars.arg[0].arg = (void *)&enable;
	rpc_pars.arg[0].arg_size = sizeof(enable);			
	rpc_pars.arg[1].arg = (void *)&type;
	rpc_pars.arg[1].arg_size = sizeof(type);
	ret = ioctl(m_snd_file_handle, AUDIO_RPC_CALL_ADV, &rpc_pars);
	//if(ret < 0)
		//ret = RET_FAILURE;

	EXIT();
	MUTEX_UNLOCK();
	
	return ret;
}

void snd_m36_attach(struct snd_feature_config * config)
{
	struct deca_device*dev = NULL;
	struct snd_device*snd_dev = NULL;
	
#ifdef ADR_IPC_ENABLE	
	int shmid = 0;
#endif

	if(g_audio_attached)
	{
		SND_PRINTF("auidio is already attached\n");
		return;
	}

#ifdef ADR_IPC_ENABLE
	g_audio_mutex_id = adr_ipc_semget(ADR_IPC_DECA, 0, 1);
	if(g_audio_mutex_id < 0)
	{
		SND_PRINTF("audio create mutex fail\n");
		return;
	}
#endif

	{
		SND_PRINTF("%s enter.\n", __FUNCTION__);
		dev=dev_alloc("DECA_S3601", HLD_DEV_TYPE_DECA, sizeof(struct deca_device));
		if(dev==NULL)
		{
			SND_PRINTF("%s Alloc deca device error!\n", __FUNCTION__);
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
			SND_PRINTF("%s Register deca device failed!!!!!!!\n", __FUNCTION__);
			return;
		}
		
#ifdef ADR_IPC_ENABLE

#else
		dev->flags = DECA_STATE_ATTACH;
#endif
		SND_PRINTF("%s dev: %s Success!\n", __FUNCTION__, dev->name);

      		decadbg_module_register();			
	}

	{		
		snd_dev=dev_alloc("SND_S3601", HLD_DEV_TYPE_SND, sizeof(struct snd_device));
		if(snd_dev==NULL)
		{
			SND_PRINTF("Alloc snd device error!\n");
			return;
		}

		/* Add this device to queue */
		if(dev_register(snd_dev)!=SUCCESS)
		{
			dev_free(snd_dev);
			SND_PRINTF("Register sound device failed!!!!!!!\n");
			return;
		}

		MUTEX_LOCK();
		
#ifdef ADR_IPC_ENABLE
		if(shmid = adr_ipc_shmget(ADR_IPC_SND, 0
			, (void **)&psnd_hld_priv, sizeof(*psnd_hld_priv)) < 0)
		{
			SND_PRINTF("get snd shm fail\n");
			return;			
		}

		SND_PRINTF("get the shmd %d\n", shmid);
		
		if(psnd_hld_priv->inited == 0)
		{
			memset((void *)psnd_hld_priv, 0, sizeof(*psnd_hld_priv));
			psnd_hld_priv->shmid = shmid;
			psnd_hld_priv->stc_divisor=299;
			psnd_hld_priv->spdif_type=SND_OUT_SPDIF_INVALID;
			psnd_hld_priv->inited = 1;

			snd_dev->priv = (void *)0;
			
			SND_PRINTF("init snd shm\n");			
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
		
		SND_PRINTF("\n snd_s3601_attach:\t Sound attach success! \n");

		snddbg_module_register();
	

	}

	g_audio_attached = 1;

	SND_PRINTF("audio attached done\n");

#ifdef ADR_IPC_ENABLE
	deca_open(dev, 0, 0, 0, 0, 0);
	snd_open(snd_dev);
#endif		
	return;
}

void snd_m36_dettach(struct snd_device *dev)
{
	return;
}

