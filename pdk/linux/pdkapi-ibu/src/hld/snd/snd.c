#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>

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
//#include <dvb_audio.h>


struct deca_device * g_deca_snd_ref ;
struct snd_device * g_snd_stc_ref = NULL;

struct snd_hld_private
{
	UINT8 volume;
	UINT16 stc_divisor;
	enum ASndOutSpdifType spdif_type;
};

static struct snd_hld_private snd_hld_priv;

__ATTRIBUTE_RAM_
RET_CODE get_stc(UINT32 * stc_msb32, UINT8 stc_num)
{
	return RET_FAILURE;
}

__ATTRIBUTE_RAM_
void set_stc(UINT32 stc_msb32, UINT8 stc_num)
{
	return RET_FAILURE;
}

__ATTRIBUTE_RAM_
void get_stc_divisor(UINT16 * stc_divisor, UINT8 stc_num)
{
	*stc_divisor=snd_hld_priv.stc_divisor;
}

__ATTRIBUTE_RAM_
void set_stc_divisor(UINT16 stc_divisor, UINT8 stc_num)
{
	snd_hld_priv.stc_divisor=stc_divisor;
}

__ATTRIBUTE_RAM_
void stc_pause(UINT8 pause, UINT8 stc_num)
{
}

__ATTRIBUTE_RAM_
void stc_invalid(void)
{
}

void stc_valid(void)
{
}

__ATTRIBUTE_REUSE_
RET_CODE snd_open(struct snd_device * dev)
{
	g_deca_snd_ref=NULL;
	g_deca_snd_ref=(struct deca_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_DECA);

	if(NULL==g_deca_snd_ref)
		return  !RET_SUCCESS;
	else
	{
		dev->priv=g_deca_snd_ref->priv;
		return RET_SUCCESS;
	}
}

RET_CODE snd_close(struct snd_device * dev)
{
       dev->priv=NULL;
	return RET_SUCCESS;
}

RET_CODE snd_set_mute(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 enable)
{
	if(dev == NULL)return !RET_SUCCESS;

	if(NULL==dev->priv)
		dev->priv=g_deca_snd_ref->priv;

	if(NULL==dev->priv)
	{
		SND_PRINTF("%s: Audio device status error.\n",__FUNCTION__);
		return !RET_SUCCESS;
	}

	if(0==ioctl((int)dev->priv, AUDIO_SET_MUTE, enable))
	{
		return RET_SUCCESS;
	}
	else
	{
		return !RET_SUCCESS;
	}

	return !RET_SUCCESS;
}

RET_CODE snd_set_volume(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 volume)
{
	if(dev == NULL)return !RET_SUCCESS;

	if(NULL==dev->priv)
		dev->priv=g_deca_snd_ref->priv;

	if(NULL==dev->priv)
	{
		SND_PRINTF("%s: Audio device status error.\n",__FUNCTION__);
		return !RET_SUCCESS;
	}

	snd_hld_priv.volume=volume;

	if(0==ioctl((int)dev->priv, AUDIO_SET_VOLUME, volume))
	{
		return RET_SUCCESS;
	}
	else
	{
		return !RET_SUCCESS;
	}
	
	return !RET_SUCCESS;
}

UINT8 snd_get_volume(struct snd_device * dev)
{
	return snd_hld_priv.volume;
}

RET_CODE snd_data_enough(struct snd_device * dev)
{
	return !RET_SUCCESS;
}

RET_CODE snd_request_pcm_buff(struct snd_device * dev, UINT32 size)
{
	return !RET_SUCCESS;
}


RET_CODE snd_io_control(struct snd_device * dev, UINT32 cmd, UINT32 param)
{
	struct ali_audio_ioctl_command io_param;
	if(NULL==dev)
		return !RET_SUCCESS;

	if(NULL==dev->priv)
		dev->priv=g_deca_snd_ref->priv;

	if(NULL==dev->priv)
	{
		SND_PRINTF("%s: Audio device status error.\n",__FUNCTION__);
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
    case SND_I2S_OUT:
    case SND_SPDIF_OUT:
    case SND_HDMI_OUT:	
		io_param.ioctl_cmd=cmd;
		io_param.param=param;
		return ioctl((int)dev->priv, AUDIO_SND_IO_COMMAND, &io_param);
	case FORCE_SPDIF_TYPE:
		snd_hld_priv.spdif_type=(enum ASndOutSpdifType)param;
		io_param.ioctl_cmd=cmd;
		io_param.param=param;
		return ioctl((int)dev->priv, AUDIO_SND_IO_COMMAND, &io_param);
	case SND_GET_SPDIF_TYPE:
	{
		enum ASndOutSpdifType *p_spdif;
		if (param == 0)
			break;
		p_spdif = (enum ASndOutSpdifType *)param;
		*p_spdif = snd_hld_priv.spdif_type;
		return RET_SUCCESS;
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
	return !RET_SUCCESS;
}

RET_CODE snd_set_sub_blk(struct snd_device * dev, UINT8 sub_blk, UINT8 enable)
{
	return !RET_SUCCESS;
}

RET_CODE snd_set_duplicate(struct snd_device * dev, enum SndDupChannel channel)
{
	if(dev == NULL)return !RET_SUCCESS;

	if(NULL==dev->priv)
		dev->priv=g_deca_snd_ref->priv;

	if(NULL==dev->priv)
	{
		SND_PRINTF("%s: Audio device status error.\n",__FUNCTION__);
		return !RET_SUCCESS;
	}

	if(0==ioctl((int)dev->priv, AUDIO_CHANNEL_SELECT, channel))
	{
		return RET_SUCCESS;
	}
	else
	{
		return !RET_SUCCESS;
	}
}

RET_CODE snd_set_spdif_type(struct snd_device * dev, enum ASndOutSpdifType type)
{
	if(dev == NULL)return !RET_SUCCESS;

	if(NULL==dev->priv)
		dev->priv=g_deca_snd_ref->priv;

	if(NULL==dev->priv)
	{
		SND_PRINTF("%s: Audio device status error.\n",__FUNCTION__);
		return !RET_SUCCESS;
	}
	
	snd_hld_priv.spdif_type=type;
	if(0==ioctl((int)dev->priv, AUDIO_SET_BYPASS_MODE, type))
	{
		return RET_SUCCESS;
	}
	else
	{
		return !RET_SUCCESS;
	}
}

RET_CODE snd_config(struct snd_device * dev, UINT32 sample_rate, UINT16 sample_num, UINT8 precision)
{
	return !RET_SUCCESS;
}

void snd_start(struct snd_device * dev)
{
}

void snd_stop(struct snd_device*dev)
{
	if(NULL==dev)
	{
        return !RET_SUCCESS;
	}
     
    if(NULL==dev->priv)
         dev->priv=g_deca_snd_ref->priv;
	if(0==ioctl((int)dev->priv, AUDIO_STOP, 2))
	{
		dev->flags = SND_STATE_IDLE;
		return RET_SUCCESS;
	}
	return !RET_SUCCESS;
}


void snd_m36_attach(struct snd_feature_config * config)
{
	struct snd_device*dev ;
	dev=dev_alloc("SND_S3601", HLD_DEV_TYPE_SND, sizeof(struct snd_device));
	if(dev==NULL)
	{
		//SND_PRINTF("Alloc snd device error!\n");
		ASSERT(0);
	}

       /* Add this device to queue */
	if(dev_register(dev)!=SUCCESS)
	{
		dev_free(dev);
		//SND_PRINTF("Register sound device failed!!!!!!!\n");
		ASSERT(0);
	}

	dev->priv=NULL;

	g_snd_stc_ref=dev;
	memset(&snd_hld_priv, 0, sizeof(struct snd_hld_private));
	snd_hld_priv.stc_divisor=299;
	snd_hld_priv.spdif_type=SND_OUT_SPDIF_INVALID;

	/*
	if(config)
	{
		snd_mute_num = config->output_config.mute_num;
		snd_mute_polar = config->output_config.mute_polar;
		snd_dac_format = config->output_config.dac_format;
		snd_dac_precision = config->output_config.dac_precision;
		snd_is_ext_dac = config->output_config.is_ext_dac;
		snd_ext_mute_mode = config->output_config.ext_mute_mode;

		if(config->conti_clk_while_ch_chg)
			snd_dis_clk_while_ch_chg = 0;
		else
			snd_dis_clk_while_ch_chg = 1;

		snd_support_spdif_mute = config->support_spdif_mute;

		if(snd_contain_embedded_dac&&0==snd_is_ext_dac)
		{
			snd_support_swap_lr_ch = config->swap_lr_channel;
		}
	}
	*/
	SND_PRINTF("\n snd_s3601_attach:\t Sound attach success! \n");
}

void snd_m36_dettach(struct snd_device *dev)
{
	if(NULL!=dev->priv)
	{
		SND_PRINTF("%s: Audio device status error.\n",__FUNCTION__);
		return;
	}
	dev_free(dev);
}

