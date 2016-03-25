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
#include <linux/version.h>
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

#include <rpc_hld/ali_rpc_hld_snd.h>

//#include "../ali_rpc.h"

#include <ali_rpcng.h>
static struct snd_callback g_snd_cb;

static void snd_hdmi_cb(UINT32 type, UINT32 uParam)
{
	if(g_snd_cb.phdmi_snd_cb)
		g_snd_cb.phdmi_snd_cb(uParam);
}
#if !defined(CONFIG_ALI_RPCNG)
UINT32 desc_snd_m36_attach[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct snd_feature_config)),
  1, DESC_P_PARA(0, 0, 0), 
  //desc of pointer ret
  0,                          
  0,
};
#endif
void snd_m36_attach(struct snd_feature_config * config)
{
    #if 0
    jump_to_func(NULL, ali_rpc_call, config, (LLD_SND_M36_MODULE<<24)|(1<<16)|FUNC_SND_M36_ATTACH, desc_snd_m36_attach);
    #endif
    struct snd_feature_config_rpc snd_feature;
    struct snd_feature_config_rpc *tmp1 = &snd_feature;

    struct snd_output_cfg_rpc snd_output;
    struct snd_output_cfg_rpc *tmp2 = &snd_output;
    

    tmp1->support_spdif_mute = config->support_spdif_mute;
    tmp1->swap_lr_channel = config->swap_lr_channel;
    tmp1->conti_clk_while_ch_chg = config->conti_clk_while_ch_chg;
    tmp1->support_desc = config->support_desc;

    tmp2->mute_num = config->output_config.mute_num;
    tmp2->mute_polar = config->output_config.mute_polar;
    tmp2->dac_precision = config->output_config.dac_precision;
    tmp2->dac_format = config->output_config.dac_format;
    tmp2->is_ext_dac = config->output_config.is_ext_dac;
    tmp2->reserved8 = config->output_config.reserved8;
    tmp2->gpio_mute_circuit = config->output_config.gpio_mute_circuit;
    tmp2->ext_mute_mode = config->output_config.ext_mute_mode;
    tmp2->enable_hw_accelerator = config->output_config.enable_hw_accelerator;
    tmp2->chip_type_config = config->output_config.chip_type_config;
    tmp2->reserved = config->output_config.reserved;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Snd_feature_config_rpc, sizeof(struct snd_feature_config_rpc), tmp1);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Snd_output_cfg_rpc, sizeof(struct snd_output_cfg_rpc), tmp2);
   

    RpcCallCompletion(RPC_snd_m36_attach,&p1,&p2,NULL);    
}

void snd_m36_init_tone_voice(struct snd_device * dev)
{
    #if 0
    jump_to_func(NULL, ali_rpc_call, dev, (LLD_SND_M36_MODULE<<24)|(1<<16)|FUNC_SND_M36_INIT_TONE_VOICE, NULL);
    #endif
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);


    RpcCallCompletion(RPC_snd_m36_init_tone_voice,&p1,NULL);    
}

void snd_init_spectrum(struct snd_device * dev)
{
    #if 0
    jump_to_func(NULL, ali_rpc_call, dev, (LLD_SND_M36_MODULE<<24)|(1<<16)|FUNC_SND_INIT_SPECTRUM, NULL);
    #endif

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    
    RpcCallCompletion(RPC_snd_init_spectrum, &p1, NULL);    
}

UINT32 stc_delay = STC_DELAY;

RET_CODE get_stc(UINT32 * stc_msb32, UINT8 stc_num)
{
    #if 0
	register RET_CODE ret asm("$2");

    UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_OUTPUT_STRU(0, 4),
      1, DESC_P_PARA(0, 0, 0), 
      //desc of pointer ret
      0,                          
      0,
    };
    jump_to_func(NULL, ali_rpc_call, stc_msb32, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_GET_STC, desc);
    return ret;
    #endif

    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_UINT32, 4, stc_msb32);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &stc_num);

    ret = RpcCallCompletion(RPC_get_stc, &p1, &p2, NULL);
    return ret;
}

void set_stc(UINT32 stc_msb32, UINT8 stc_num)
{
    #if 0
    jump_to_func(NULL, ali_rpc_call, stc_msb32, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SET_STC, NULL);
    #endif
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &stc_msb32);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &stc_num);

    RpcCallCompletion(RPC_set_stc, &p1, &p2, NULL);
}

void get_stc_divisor(UINT16 * stc_divisor, UINT8 stc_num)
{
    #if 0
    UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_OUTPUT_STRU(0, 4),
      1, DESC_P_PARA(0, 0, 0), 
      //desc of pointer ret
      0,                          
      0,
    };
    jump_to_func(NULL, ali_rpc_call, stc_divisor, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_GET_STC_DIVISOR, desc);
    #endif

    RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_UINT16, 4, stc_divisor);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &stc_num);
    
    RpcCallCompletion(RPC_get_stc_divisor, &p1, &p2, NULL);
}

void set_stc_divisor(UINT16 stc_divisor, UINT8 stc_num)
{
    #if 0
    jump_to_func(NULL, ali_rpc_call, stc_divisor, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SET_STC_DIVISOR, NULL);
    #endif

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT16, sizeof(Uint16), &stc_divisor);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &stc_num);

    RpcCallCompletion(RPC_set_stc_divisor, &p1, &p2, NULL);

}

void stc_pause(UINT8 pause, UINT8 stc_num)
{
    #if 0
    jump_to_func(NULL, ali_rpc_call, null, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_STC_PAUSE, NULL);
    #endif

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &pause);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &stc_num);
    
    RpcCallCompletion(RPC_stc_pause, &p1, &p2, NULL);
}

void stc_invalid(void)
{
#if 0
	jump_to_func(NULL, ali_rpc_call, null, (HLD_SND_MODULE<<24)|(0<<16)|FUNC_STC_INVALID, NULL);
#endif

    RpcCallCompletion(RPC_stc_invalid, NULL);

}

void stc_valid(void)
{
#if 0
    jump_to_func(NULL, ali_rpc_call, null, (HLD_SND_MODULE<<24)|(0<<16)|FUNC_STC_VALID, NULL);
#endif

    RpcCallCompletion(RPC_stc_valid, NULL);
}

void snd_output_config(struct snd_device * dev, struct snd_output_cfg * cfg_param)
{
#if 0
    UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_STATIC_STRU(0, sizeof(struct snd_output_cfg)),
      1, DESC_P_PARA(0, 1, 0), 
      //desc of pointer ret
      0,                          
      0,
    };
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_OUTPUT_CONFIG, desc);
#endif

    struct snd_output_cfg_rpc snd_output_cfg_struct;
    struct snd_output_cfg_rpc *temp1 = &snd_output_cfg_struct;

    temp1->mute_num = cfg_param->mute_num;
    temp1->mute_polar = cfg_param->mute_polar;
    temp1->dac_precision = cfg_param->dac_precision;
    temp1->dac_format = cfg_param->dac_format;
    temp1->is_ext_dac = cfg_param->is_ext_dac;
    temp1->reserved8 = cfg_param->reserved8;
    temp1->gpio_mute_circuit = cfg_param->gpio_mute_circuit;
    temp1->ext_mute_mode = cfg_param->ext_mute_mode;
    temp1->enable_hw_accelerator = cfg_param->enable_hw_accelerator;
    temp1->chip_type_config = cfg_param->chip_type_config;
    temp1->reserved = cfg_param->reserved;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Snd_output_cfg_rpc, sizeof(Snd_output_cfg_rpc), temp1);
    
    RpcCallCompletion(RPC_snd_output_config, &p1, &p2, NULL);
}
RET_CODE snd_open(struct snd_device * dev)
{
    #if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_OPEN, NULL);
    return ret;
    #endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    ret = RpcCallCompletion(RPC_snd_open,&p1,NULL);
    return ret;
}

RET_CODE snd_close(struct snd_device * dev)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_CLOSE, NULL);
    return ret;
#endif
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    ret = RpcCallCompletion(RPC_snd_close, &p1, NULL);
    return ret;
}

RET_CODE snd_set_mute(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 enable)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_MUTE, NULL);
    return ret;
#endif

    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum SndSubBlock), &sub_blk);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &enable);

    ret = RpcCallCompletion(RPC_snd_set_mute, &p1, &p2, &p3, NULL);
    return ret;
}

RET_CODE snd_set_volume(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 volume)
{
    #if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_VOLUME, NULL);
    return ret;
    #endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum SndSubBlock), &sub_blk);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &volume);

    ret = RpcCallCompletion(RPC_snd_set_volume,&p1,&p2,&p3,NULL);    
    return ret;
}

UINT8 snd_get_volume(struct snd_device * dev)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_GET_VOLUME, NULL);
    return ret;
#endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    ret = RpcCallCompletion(RPC_snd_get_volume,&p1, NULL);
    return ret;
}

RET_CODE snd_data_enough(struct snd_device * dev)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_DATA_ENOUGH, NULL);
    return ret;
#endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    ret = RpcCallCompletion(RPC_snd_data_enough, &p1, NULL);
    return ret;
}

RET_CODE snd_request_pcm_buff(struct snd_device * dev, UINT32 size)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_REQUEST_PCM_BUFF, NULL);
    return ret;
#endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &size);

    ret = RpcCallCompletion(RPC_snd_request_pcm_buff, &p1, &p2, NULL);
    return ret;
}

void snd_write_pcm_data(struct snd_device*dev,struct pcm_output*pcm,UINT32*frame_header)
{
#if 0
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_WRITE_PCM_DATA, NULL);
#endif

    struct pcm_output_rpc pcm_output_struct;
    struct pcm_output_rpc *temp1 = &pcm_output_struct;

    temp1->ch_num = pcm->ch_num;
    temp1->ch_mod = pcm->ch_mod;
    temp1->samp_num = pcm->samp_num;
    temp1->sample_rata_id = pcm->sample_rata_id;
    temp1->inmode = pcm->inmode;
    temp1->ch_left = pcm->ch_left;
    temp1->ch_right = pcm->ch_right;
    temp1->ch_sl = pcm->ch_sl;
    temp1->ch_sr = pcm->ch_sr;
    temp1->ch_c = pcm->ch_c;
    temp1->ch_lfe = pcm->ch_lfe;
    temp1->ch_dl = pcm->ch_dl;

    temp1->ch_dr = pcm->ch_dr;
    temp1->raw_data_start = pcm->raw_data_start;
    temp1->raw_data_len = pcm->raw_data_len;
    temp1->iec_pc = pcm->iec_pc;
    temp1->raw_data_ddp_start = pcm->raw_data_ddp_start;
    temp1->raw_data_ddp_len = pcm->raw_data_ddp_len;
    temp1->iec_pc_ddp = pcm->iec_pc_ddp;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Pcm_output_rpc, sizeof(Pcm_output_rpc), temp1);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, 4, &frame_header);

    RpcCallCompletion(RPC_snd_write_pcm_data, &p1, &p2, &p3, NULL);
}

void snd_write_pcm_data2(struct snd_device * dev, UINT32 * frame_header,
	                                                UINT32 * left, UINT32 * right, UINT32 number, UINT32 ch_num)
{
#if 0
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(6<<16)|FUNC_SND_WRITE_PCM_DATA2, NULL);
#endif

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &frame_header);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, 4, &left);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_UINT32, 4, &right);
    RPC_PARAM_CREATE(p5, PARAM_IN, PARAM_UINT32, 4, &number);
    RPC_PARAM_CREATE(p6, PARAM_IN, PARAM_UINT32, 4, &ch_num);

    RpcCallCompletion(RPC_snd_write_pcm_data2, &p1, &p2, &p3, &p4, &p5, p6, NULL);
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
#if 0
	register RET_CODE ret asm("$2");

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
    return ret;
#endif
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &cmd);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);

    switch(cmd)
    {
        case IS_SND_RUNNING:
        case IS_SND_MUTE:
        case IS_PCM_EMPTY:
        case SND_CC_MUTE:
        case SND_DAC_MUTE:
        case SND_CC_MUTE_RESUME:
        case SND_SPO_ONOFF:
        case SND_SET_FADE_SPEED:
        case SND_BYPASS_VCR:
        case FORCE_SPDIF_TYPE:      
        case SND_CHK_SPDIF_TYPE:  
        case SND_BASS_TYPE:
        case SND_PAUSE_MUTE:
        case SND_SET_DESC_VOLUME_OFFSET:
        case SND_SET_BS_OUTPUT_SRC:
        case SND_SECOND_DECA_ENABLE:    
        case SND_DO_DDP_CERTIFICATION:
        case SND_POST_PROCESS_0:
        case SND_SPECIAL_MUTE_REG:
        case SND_SET_SYNC_DELAY:
        case SND_SET_SYNC_LEVEL:
        case SND_SET_MUTE_TH:
        case SND_AUTO_RESUME:
        case SND_I2S_OUT:
        case SND_SPDIF_OUT:
        case SND_HDMI_OUT:
        {
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);
            break;
        }
        
        case SND_REG_HDMI_CB:
        {
            g_snd_cb.phdmi_snd_cb = (OSAL_T_HSR_PROC_FUNC_PTR)(param);

            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);
            break;
        }
        
        case SND_GET_STATUS:
        {
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Snd_dev_status_rpc, sizeof(Snd_dev_status_rpc), param);
            break;
        }

        case SND_CHK_DAC_PREC:
        case SND_GET_RAW_PTS:
        case SND_REQ_REM_DATA:
        case SND_GET_TONE_STATUS:
        case SND_CHK_PCM_BUF_DEPTH:
        case SND_GET_SAMPLES_REMAIN:
        case SND_REQ_REM_PCM_DATA:
        case SND_REQ_REM_PCM_DURA:
        case SND_GET_SPDIF_TYPE:
        case SND_GET_MUTE_TH:
        {
            RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, 4, param);
            break;
        }
        
        case SND_SPECTRUM_START:
        {
            g_snd_cb.spec_call_back = ((spec_param*)param)->spec_call_back;
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Spec_param_rpc, sizeof(Spec_param_rpc), param);
            break;
        }
        
        case SND_SPECTRUM_STEP_TABLE:
        {
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Spec_step_table_rpc, sizeof(Spec_step_table_rpc), param);
            break;
        }
        
        case SND_SPECTRUM_STOP:
        case SND_SPECTRUM_CLEAR:
        case SND_SPECTRUM_VOL_INDEPEND:
        case SND_SPECTRUM_CAL_COUNTER:
        {
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);
            break;
        }
        
        case SND_SET_SPDIF_SCMS:
        {
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Snd_spdif_scms_rpc, sizeof(Snd_spdif_scms_rpc), param);
            break;
        }
        
        case SND_SET_SYNC_PARAM:
        {
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Snd_sync_param_rpc, sizeof(Snd_sync_param_rpc), param);
            break;
        }

        default:
        {
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 4, &param);
            break;
        }
    }
    
    ret = RpcCallCompletion(RPC_snd_io_control, &p1, &p2, &p3, NULL);
    
    return ret;
}
EXPORT_SYMBOL(snd_io_control);


	
RET_CODE snd_set_sub_blk(struct snd_device * dev, UINT8 sub_blk, UINT8 enable)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_SET_SUB_BLK, NULL);
    return ret;
#endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &sub_blk);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &enable);

    ret = RpcCallCompletion(RPC_snd_set_sub_blk, &p1, &p2, &p3, NULL);
    return ret;
}

RET_CODE snd_set_duplicate(struct snd_device * dev, enum SndDupChannel channel)
{
    #if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_DUPLICATE, NULL);
    return ret;
    #endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum SndDupChannel), &channel);

    ret = RpcCallCompletion(RPC_snd_set_duplicate, &p1, &p2, NULL);
    return ret;
}

RET_CODE snd_set_spdif_type(struct snd_device * dev, enum ASndOutSpdifType type)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_SPDIF_TYPE, NULL);
    return ret;
#endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ENUM, sizeof(enum ASndOutSpdifType), &type);

    ret = RpcCallCompletion(RPC_snd_set_spdif_type,&p1,&p2,NULL);
    return ret;
}

RET_CODE snd_config(struct snd_device * dev, UINT32 sample_rate, UINT16 sample_num, UINT8 precision)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(4<<16)|FUNC_SND_CONFIG, NULL);
    return ret;
#endif
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &sample_rate);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT16, sizeof(Uint16), &sample_num);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &precision);

    ret = RpcCallCompletion(RPC_snd_config, &p1, &p2, &p3, &p4, NULL);
    return ret;
}

void snd_start(struct snd_device * dev)
{
#if 0
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_START, NULL);
#endif

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    RpcCallCompletion(RPC_snd_start, &p1, NULL);
}

void snd_stop(struct snd_device*dev)
{
#if 0
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_STOP, NULL);
#endif

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    RpcCallCompletion(RPC_snd_stop, &p1, NULL);
}
#if 0
static UINT32 gen_tone_voice_desc[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pcm_output)),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};
#endif	
void snd_gen_tone_voice(struct snd_device * dev, struct pcm_output*pcm, UINT8 init)  //tone voice
{
#if 0
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_GEN_TONE_VOICE, gen_tone_voice_desc);
#endif

    struct pcm_output_rpc pcm_output_struct;
    struct pcm_output_rpc *temp1 = &pcm_output_struct;

    temp1->ch_num = pcm->ch_num;
    temp1->ch_mod = pcm->ch_mod;
    temp1->samp_num = pcm->samp_num;
    temp1->sample_rata_id = pcm->sample_rata_id;
    temp1->inmode = pcm->inmode;
    temp1->ch_left = pcm->ch_left;
    temp1->ch_right = pcm->ch_right;
    temp1->ch_sl = pcm->ch_sl;
    temp1->ch_sr = pcm->ch_sr;
    temp1->ch_c = pcm->ch_c;
    temp1->ch_lfe = pcm->ch_lfe;
    temp1->ch_dl = pcm->ch_dl;

    temp1->ch_dr = pcm->ch_dr;
    temp1->raw_data_start = pcm->raw_data_start;
    temp1->raw_data_len = pcm->raw_data_len;
    temp1->iec_pc = pcm->iec_pc;
    temp1->raw_data_ddp_start = pcm->raw_data_ddp_start;
    temp1->raw_data_ddp_len = pcm->raw_data_ddp_len;
    temp1->iec_pc_ddp = pcm->iec_pc_ddp;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Pcm_output_rpc, sizeof(Pcm_output_rpc), temp1);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &init);
    
    RpcCallCompletion(RPC_snd_gen_tone_voice, &p1, &p2, &p3, NULL);


}

void snd_stop_tone_voice(struct snd_device * dev)  //tone voice
{
#if 0
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(1<<16)|FUNC_SND_STOP_TONE_VOICE, NULL);
#endif

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);

    RpcCallCompletion(RPC_snd_stop_tone_voice, &p1, NULL);
}

RET_CODE snd_ena_pp_8ch(struct snd_device * dev, UINT8 enable)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_ENA_PP_8CH, NULL);
    return ret;
#endif

    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &enable);

    ret = RpcCallCompletion(RPC_snd_ena_pp_8ch, &p1, &p2, NULL);
    return ret;
}

RET_CODE snd_set_pp_delay(struct snd_device * dev, UINT8 delay)
{
#if 0 
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_PP_DELAY, NULL);
    return ret;
#endif

    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &delay);

    ret = RpcCallCompletion(RPC_snd_set_pp_delay,&p2,NULL);
    return ret;

}

RET_CODE snd_enable_virtual_surround(struct snd_device * dev, UINT8 enable)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_ENABLE_VIRTUAL_SURROUND, NULL);
    return ret;
#endif 

    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &enable);

    ret = RpcCallCompletion(RPC_snd_enable_virtual_surround, &p1, &p2, NULL);
    return ret;
}

RET_CODE snd_enable_eq(struct snd_device * dev, UINT8 enable,enum EQ_TYPE type)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(3<<16)|FUNC_SND_ENABLE_EQ, NULL);
    return ret;
#endif

    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &enable);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_ENUM, sizeof(enum EQ_TYPE), &type);


    ret = RpcCallCompletion(RPC_snd_enable_eq, &p1, &p2, &p3, NULL);
    return ret;

}

RET_CODE snd_enable_bass(struct snd_device * dev, UINT8 enable)
{
#if 0
	register RET_CODE ret asm("$2");

    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_ENABLE_BASS, NULL);
#endif

    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &enable);

    ret = RpcCallCompletion(RPC_snd_enable_bass, &p1, &p2, NULL);
    return ret;
}

void snd_register_cb_routine(void)
{
	ali_rpc_register_callback(ALI_RPC_CB_SND_HDMI, snd_hdmi_cb);
}
EXPORT_SYMBOL(snd_register_cb_routine);

RET_CODE snd_set_dbg_level(struct snd_device * dev, UINT32 option)
{
#if 0
    register RET_CODE ret asm("$2");
    jump_to_func(NULL, ali_rpc_call, dev, (HLD_SND_MODULE<<24)|(2<<16)|FUNC_SND_SET_DBG_LEVEL, NULL);
    return ret;
#endif

    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, 4, &option);

    ret = RpcCallCompletion(RPC_snd_set_dbg_level, &p1, &p2, NULL);
    return ret;
}


