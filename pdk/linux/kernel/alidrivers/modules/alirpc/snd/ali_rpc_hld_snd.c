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

#include <rpc_hld/ali_rpc_hld_snd.h>

#include "../ali_rpc.h"

static struct snd_callback g_snd_cb;

static void snd_hdmi_cb(UINT32 type, UINT32 uParam)
{
	if(g_snd_cb.phdmi_snd_cb)
		g_snd_cb.phdmi_snd_cb(uParam);
}

UINT32 desc_snd_m36_attach[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct snd_feature_config)),
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};

void snd_m36_attach(struct snd_feature_config * config)
{
    jump_to_func(NULL, ali_rpc_call, config, (LLD_SND_M36_MODULE<<24)|(1<<16)|FUNC_SND_M36_ATTACH, desc_snd_m36_attach);
}

void snd_m36_init_tone_voice(struct snd_device * dev)
{
    jump_to_func(NULL, ali_rpc_call, dev, (LLD_SND_M36_MODULE<<24)|(1<<16)|FUNC_SND_M36_INIT_TONE_VOICE, NULL);
}

void snd_init_spectrum(struct snd_device * dev)
{
    jump_to_func(NULL, ali_rpc_call, dev, (LLD_SND_M36_MODULE<<24)|(1<<16)|FUNC_SND_INIT_SPECTRUM, NULL);
}

UINT32 stc_delay = STC_DELAY;

RET_CODE get_stc(UINT32 * stc_msb32, UINT8 stc_num)
{


    UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_OUTPUT_STRU(0, 4),
      1, DESC_P_PARA(0, 0, 0), 
      //desc of pointer ret
      0,                          
      0,
    };
    jump_to_func(NULL, ali_rpc_call, stc_msb32, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_GET_STC, desc);

}

void set_stc(UINT32 stc_msb32, UINT8 stc_num)
{
    jump_to_func(NULL, ali_rpc_call, stc_msb32, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SET_STC, NULL);
}

void get_stc_divisor(UINT16 * stc_divisor, UINT8 stc_num)
{
    UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_OUTPUT_STRU(0, 4),
      1, DESC_P_PARA(0, 0, 0), 
      //desc of pointer ret
      0,                          
      0,
    };
    jump_to_func(NULL, ali_rpc_call, stc_divisor, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_GET_STC_DIVISOR, desc);
}

void set_stc_divisor(UINT16 stc_divisor, UINT8 stc_num)
{
    jump_to_func(NULL, ali_rpc_call, stc_divisor, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SET_STC_DIVISOR, NULL);
}

void stc_pause(UINT8 pause, UINT8 stc_num)
{
	jump_to_func(NULL, ali_rpc_call, null, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_STC_PAUSE, NULL);
}

void stc_invalid(void)
{
	jump_to_func(NULL, ali_rpc_call, null, (HLD_SND_MODULE<<24)|(0<<16)|FUNC_STC_INVALID, NULL);
}

void stc_valid(void)
{
    jump_to_func(NULL, ali_rpc_call, null, (HLD_SND_MODULE<<24)|(0<<16)|FUNC_STC_VALID, NULL);
}

void snd_output_config(struct snd_device * dev, struct snd_output_cfg * cfg_param)
{
    UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_STATIC_STRU(0, sizeof(struct snd_output_cfg)),
      1, DESC_P_PARA(0, 1, 0), 
      //desc of pointer ret
      0,                          
      0,
    };
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_OUTPUT_CONFIG, desc);
}
RET_CODE snd_open(struct snd_device * dev)
{
	//register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_OPEN, NULL);
   // return ret;
}

RET_CODE snd_close(struct snd_device * dev)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_CLOSE, NULL);

}

RET_CODE snd_set_mute(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 enable)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_MUTE, NULL);

}

RET_CODE snd_set_volume(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 volume)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_VOLUME, NULL);

}

UINT8 snd_get_volume(struct snd_device * dev)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_GET_VOLUME, NULL);

}

RET_CODE snd_data_enough(struct snd_device * dev)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_DATA_ENOUGH, NULL);

}

RET_CODE snd_request_pcm_buff(struct snd_device * dev, UINT32 size)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_REQUEST_PCM_BUFF, NULL);

}

void snd_write_pcm_data(struct snd_device*dev,struct pcm_output*pcm,UINT32*frame_header)
{
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_WRITE_PCM_DATA, NULL);
}

void snd_write_pcm_data2(struct snd_device * dev, UINT32 * frame_header,
	                                                UINT32 * left, UINT32 * right, UINT32 number, UINT32 ch_num)
{
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(6<<16)|FUNC_SND_WRITE_PCM_DATA2, NULL);
}

static UINT32 snd_spdif_scms_desc[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct snd_spdif_scms)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 snd_sync_param_desc[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(snd_sync_param)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};
static UINT32 snd_spec_param[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(spec_param)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 snd_spec_step_table[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(spec_step_table)),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 req_rem_desc[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_snd_p_uint32[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};

 UINT32 desc_snd_get_status[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct snd_dev_status)),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};  
 
RET_CODE snd_io_control(struct snd_device * dev, UINT32 cmd, UINT32 param)
{
//	register RET_CODE ret asm("$2");

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
        case FORCE_SPDIF_TYPE:
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
        case SND_I2S_OUT:
        case SND_SPDIF_OUT:
        case SND_HDMI_OUT:
        default:
            jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, NULL);
            break;
        case SND_REG_HDMI_CB:
            g_snd_cb.phdmi_snd_cb = (OSAL_T_HSR_PROC_FUNC_PTR)(param);
            jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, NULL);
            break;
        case SND_GET_RAW_PTS:
        case SND_GET_STATUS:
            jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, desc_snd_get_status);
            break;
        case SND_REQ_REM_DATA:
        case SND_GET_TONE_STATUS:
        case SND_CHK_PCM_BUF_DEPTH:
        case SND_GET_SAMPLES_REMAIN:
        case SND_REQ_REM_PCM_DATA:
        case SND_REQ_REM_PCM_DURA:
        case SND_GET_SPDIF_TYPE:
        case SND_GET_MUTE_TH:
            jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, req_rem_desc);
            break;
        case SND_SPECTRUM_START:
            g_snd_cb.spec_call_back = ((spec_param*)param)->spec_call_back;
            jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_spec_param);
            break;
		case SND_SPECTRUM_STEP_TABLE:
            jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_spec_step_table);
            break;
        case SND_SPECTRUM_STOP:
        case SND_SPECTRUM_CLEAR:
        case SND_SPECTRUM_VOL_INDEPEND:
		case SND_SPECTRUM_CAL_COUNTER:
            jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, NULL);
            break;
        case SND_SET_SPDIF_SCMS:
            jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_spdif_scms_desc);
            break;
        case SND_SET_SYNC_PARAM:
            jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_IO_CONTROL, snd_sync_param_desc);
            break;
    }
   // return ret;
}

RET_CODE snd_set_sub_blk(struct snd_device * dev, UINT8 sub_blk, UINT8 enable)
{
//	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_SUB_BLK, NULL);
  //  return ret;
}

RET_CODE snd_set_duplicate(struct snd_device * dev, enum SndDupChannel channel)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_DUPLICATE, NULL);

}

RET_CODE snd_set_spdif_type(struct snd_device * dev, enum ASndOutSpdifType type)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_SPDIF_TYPE, NULL);

}

RET_CODE snd_config(struct snd_device * dev, UINT32 sample_rate, UINT16 sample_num, UINT8 precision)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(4<<16)|FUNC_SND_CONFIG, NULL);

}

void snd_start(struct snd_device * dev)
{
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_START, NULL);
}

void snd_stop(struct snd_device*dev)
{
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_STOP, NULL);
}

static UINT32 gen_tone_voice_desc[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pcm_output)),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};
	
void snd_gen_tone_voice(struct snd_device * dev, struct pcm_output*pcm, UINT8 init)  //tone voice
{
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_GEN_TONE_VOICE, gen_tone_voice_desc);
}

void snd_stop_tone_voice(struct snd_device * dev)  //tone voice
{
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_STOP_TONE_VOICE, NULL);
}

RET_CODE snd_ena_pp_8ch(struct snd_device * dev, UINT8 enable)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_ENA_PP_8CH, NULL);

}

RET_CODE snd_set_pp_delay(struct snd_device * dev, UINT8 delay)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_PP_DELAY, NULL);

}

RET_CODE snd_enable_virtual_surround(struct snd_device * dev, UINT8 enable)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_ENABLE_VIRTUAL_SURROUND, NULL);

}

RET_CODE snd_enable_eq(struct snd_device * dev, UINT8 enable,enum EQ_TYPE type)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_ENABLE_EQ, NULL);

}

RET_CODE snd_enable_bass(struct snd_device * dev, UINT8 enable)
{


    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_ENABLE_BASS, NULL);

}

void snd_register_cb_routine(void)
{
	ali_rpc_register_callback(ALI_RPC_CB_SND_HDMI, snd_hdmi_cb);
}
EXPORT_SYMBOL(snd_register_cb_routine);

RET_CODE snd_set_dbg_level(struct snd_device * dev, UINT32 option)
{

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_DBG_LEVEL, NULL);

}


