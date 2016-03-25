#ifndef _DECA_H_
#define _DECA_H_

#include <mediatypes.h>
#include <hld/deca/deca_dev.h>
#include <dvb_audio.h>
//corei
#include <alidefinition/adf_decv.h>
#include <rpc_hld/ali_rpc_hld_deca.h>

#define DECA_STATE_DETACH   0
#define DECA_STATE_ATTACH   1
#define DECA_STATE_IDLE        2
#define DECA_STATE_PLAY        4
#define DECA_STATE_PAUSE     8
//corei

//#define DEBUG_DECA 1
#ifdef DEBUG_DECA
#define DECA_PRINTF   PRINTF
#else
#define DECA_PRINTF(...)	do{}while(0)
#endif
#ifdef AC3DEC
#define DVB_AUDIO_EXT //SUPPORT DVB AUDIO EXTENTION, INCLUDE: AAC, AC3 AND DTS
#endif

#if 0
#define DECA_STATE_DETACH   0
#define DECA_STATE_ATTACH   1
#define DECA_STATE_IDLE        2
#define DECA_STATE_PLAY        4
#define DECA_STATE_PAUSE     8

#define DECA_SUB_STATE_BUSY           1
#define DECA_SUB_STATE_NO_DATA    2
#define DECA_SUB_STATE_NO_BUFF     4


struct deca_feature_config
{
	UINT8 detect_sprt_change;/*=1: if sample rate changed, audio decoder can detected it and re-config sound HW.*/
	UINT8 bs_buff_size  :3;    // power of bs buffer size = (1024 * 8) * (2^n)
	UINT8 support_desc  :1;
	UINT8 reserved      :4;
	UINT16 reserved16;
};

enum ADecDescChannelEnable
{
    ADEC_DESC_CHANNEL_DISABLE = 0,
    ADEC_DESC_CHANNEL_ENABLE
};
enum ADecBufMode
{
	ADEC_LIVE_MODE = 0,
	ADEC_PS_MODE
};

void deca_m36_attach(struct deca_feature_config* config);
void deca_m36_dettach(struct deca_device*dev);
RET_CODE deca_open(struct deca_device * dev,
                                  enum AudioStreamType stream_type,
                                  enum AudioSampleRate samp_rate,
                                  enum AudioQuantization quan,
                                  UINT8 channel_num, UINT32 info_struct);
RET_CODE deca_close(struct deca_device * dev);
RET_CODE deca_start(struct deca_device * dev, UINT32 high32_pts);
RET_CODE deca_stop(struct deca_device * dev, UINT32 high32_pts, enum ADecStopMode mode);
RET_CODE deca_pause(struct deca_device * dev);
RET_CODE deca_io_control(struct deca_device * dev, UINT32 cmd, UINT32 param);
RET_CODE deca_set_sync_mode(struct deca_device * dev, enum ADecSyncMode mode);
RET_CODE deca_request_write(void * dev, UINT32 req_size,
	                                 void * * ret_buf, UINT32 * ret_buf_size,
	                                 struct control_block * ctrl_blk);
void deca_update_write(void * dev, UINT32 size);
void deca_tone_voice(struct deca_device * dev, UINT32 SNR, UINT8 init);  //tone voice
void deca_stop_tone_voice(struct deca_device * dev);  //tone voice
void deca_init_ase(struct deca_device * dev);
RET_CODE deca_decore_ioctl(struct deca_device *dev, UINT32 cmd, void *param1, void *param2);
#endif

#endif /*_DECA_H_*/

