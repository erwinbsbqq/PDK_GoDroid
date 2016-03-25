#ifndef _SND_H_
#define _SND_H_

#include <hld/snd/snd_dev.h>
#include <osal/osal_int.h>

#include <dvb_audio.h>
//#include <alidefinition/adf_snd.h>  //corei
#include <rpc_hld/ali_rpc_hld_snd.h>

#define DEBUG_SND 1
#ifdef DEBUG_SND
#define SND_PRINTF   PRINTF
#else
#define SND_PRINTF(...)	do{}while(0)
#endif

#if 0    //corei
#define MUTE_BY_GPIO	0
#define MUTE_BY_SCART	1

#define SND_STATE_DETACH   0
#define SND_STATE_ATTACH   1
#define SND_STATE_IDLE        2
#define SND_STATE_PLAY       4

/*Keep for compile*/
typedef struct{
	void (*spec_call_back)(INT32 *);
	UINT32 collumn_num;
}spec_param;

struct snd_feature_config
{
	struct snd_output_cfg output_config;
	UINT8 support_spdif_mute;
	UINT8 swap_lr_channel;
	UINT8 conti_clk_while_ch_chg;
    UINT8 support_desc;
};

struct snd_callback
{
    OSAL_T_HSR_PROC_FUNC_PTR	phdmi_snd_cb;
    void (*spec_call_back)(INT32 *);
};

RET_CODE snd_open(struct snd_device * dev);
RET_CODE snd_close(struct snd_device * dev);
RET_CODE snd_set_mute(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 enable);
RET_CODE snd_set_volume(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 volume);
UINT8 snd_get_volume(struct snd_device * dev);
RET_CODE snd_io_control(struct snd_device * dev, UINT32 cmd, UINT32 param);
RET_CODE snd_config(struct snd_device * dev, UINT32 sample_rate, UINT16 sample_num, UINT8 precision);
RET_CODE snd_set_spdif_type(struct snd_device * dev, enum ASndOutSpdifType type);
void snd_start(struct snd_device*dev);
void snd_stop(struct snd_device*dev);

RET_CODE get_stc(UINT32 * stc_msb32, UINT8 stc_num);
void set_stc(UINT32 stc_msb32, UINT8 stc_num);
void get_stc_divisor(UINT16 * stc_divisor, UINT8 stc_num);
void set_stc_divisor(UINT16 stc_divisor, UINT8 stc_num);
void stc_invalid(void);
void stc_valid(void);

#ifdef DVBT_BEE_TONE
void snd_gen_tone_voice(struct snd_device * dev, struct pcm_output*pcm, UINT8 init); //tone voice
void snd_stop_tone_voice(struct snd_device * dev);  //tone voice
#endif

#endif

#endif /*_SND_H_*/
