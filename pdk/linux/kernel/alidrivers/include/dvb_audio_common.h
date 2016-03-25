#ifndef _DVB_AUDIO_COMMON_H_
#define _DVB_AUDIO_COMMON_H_

#include "ali_basic_common.h"
#include "ali_audio_common.h"

#define AUDIO_SET_VOLUME                                _IOW('o', 21, unsigned char)
#define AUDIO_GET_INPUT_CALLBACK_ROUTINE                _IOR('o', 22, struct ali_dec_input_callback_routine_pars)
#define AUDIO_DECA_IO_COMMAND                           _IOWR('o', 23, struct ali_audio_ioctl_command)
#define AUDIO_SND_IO_COMMAND                            _IOWR('o', 24, struct ali_audio_ioctl_command)
#define AUDIO_DECORE_COMMAND                            _IOWR('o', 25, struct ali_audio_rpc_pars)
#define AUDIO_GET_VOLUME                                _IOR('o', 26, unsigned char)

#define AUDIO_ASE_INIT                                  _IO('o', 27)
#define AUDIO_ASE_STR_STOP                              _IO('o', 28)
#define AUDIO_ASE_DECA_BEEP_INTERVAL                    _IOW('o', 29,unsigned int)
#define AUDIO_ASE_STR_PLAY                              _IOW('o', 30,struct ase_str_play_param)

#define AUDIO_GEN_TONE_VOICE                            _IOW('o', 31, unsigned long)
#define AUDIO_STOP_TONE_VOICE                           _IO('o', 32)
#define AUDIO_SND_ENABLE_EQ                             _IOW('o',33,struct ali_audio_rpc_pars)

#define AUDIO_EMPTY_BS_SET                              _IO('o', 34)
#define AUDIO_ADD_BS_SET                                _IO('o', 35)
#define AUDIO_DEL_BS_SET                                _IO('o', 36)
#define AUDIO_SND_START                                 _IO('o', 37)
#define AUDIO_SND_STOP                                  _IO('o', 38)
#define AUDIO_SND_STC_INVALID                           _IO('o', 39)
#define AUDIO_SND_STC_VALID                             _IO('o', 40)
#define AUDIO_RPC_CALL_ADV                              _IO('o', 41)
#define AUDIO_SND_GET_STC                               _IO('o', 42)
#define AUDIO_SND_SET_STC                               _IO('o', 43)
#define AUDIO_SND_PAUSE_STC                             _IO('o', 44)
#define AUDIO_SND_SET_SUB_BLK                           _IO('o', 45)
#define AUDIO_DECA_IO_COMMAND_ADV                       _IOWR('o', 46, struct ali_audio_rpc_pars)
#define AUDIO_SND_IO_COMMAND_ADV                        _IOWR('o', 47, struct ali_audio_rpc_pars)
#define AUDIO_DECA_PROCESS_PCM_SAMPLES                  _IO('o', 48)
#define AUDIO_DECA_PROCESS_PCM_BITSTREAM                _IO('o', 49)
#define RPC_AUDIO_DECORE_IOCTL                          _IO('o', 50)
#define AUDIO_DECA_SET_DBG_LEVEL                        _IO('o', 51)
#define AUDIO_SND_SET_DBG_LEVEL                         _IO('o', 52)

#define AUDIO_SET_CTRL_BLK_INFO                         _IOW('o',53,ali_audio_ctrl_blk)

#define AUDIO_DECA_IS_BS_MEMBER_CHECK                   _IOR('O',54,unsigned char)
#define AUDIO_DECA_HDD_PLAYBACK                         _IOW('O',55,unsigned char)
#define AUDIO_DECA_PLAY_SPEED_SET                       _IOW('O',56,enum ADecPlaySpeed)
#define AUDIO_DECA_DESC_ENABLE                          _IOW('O',57,unsigned char)
#define AUDIO_DECA_AC3_COMP_MODE_SET                    _IOW('O',58,enum DECA_AC3_COMP_MODE)
#define AUDIO_DECA_AC3_STEREO_MODE_SET                  _IOW('O',59,enum DECA_AC3_STEREO_MODE)
#define AUDIO_DECA_STATE_GET                            _IOR('O',60,unsigned char)
#define AUDIO_DECA_DDP_INMOD_GET                        _IOR('O',61,unsigned char)
#define AUDIO_DECA_FRM_INFO_GET                         _IOR('O',62,struct cur_stream_info)


#define AUDIO_SND_FORCE_SPDIF_TYPE                      _IOR('O',63,enum ASndOutSpdifType)
#define AUDIO_SND_I2S_OUT                               _IOW('O',64,unsigned char)
#define AUDIO_SND_SPDIF_OUT                             _IOW('O',65,unsigned char)
#define AUDIO_SND_HDMI_OUT                              _IOW('O',66,unsigned char)
#define AUDIO_SND_DESC_VOLUME_OFFSET_SET                _IOW('O',67,long)
#define AUDIO_SND_DESC_ENABLE                           _IOW('O',68,unsigned char)
#define AUDIO_SND_SYNC_DELAY_SET                        _IOW('O',69,long)
#define AUDIO_SND_SYNC_LEVEL_SET                        _IOW('O',70,unsigned char)


#define AUDIO_DECA_DECORE_INIT                          _IOW('O',301,struct audio_config)
#define AUDIO_DECA_DECORE_RLS                           _IO('O',302)
#define AUDIO_DECA_DECORE_SET_BASE_TIME                 _IOW('O',303,unsigned long)
#define AUDIO_DECA_DECORE_GET_PCM_TRD                   _IOR('O',304,unsigned long)
#define AUDIO_DECA_DECORE_PAUSE_DECODE                  _IO('O',305)
#define AUDIO_DECA_DECORE_FLUSH                         _IO('O',306)
#define AUDIO_DECA_DECORE_SET_QUICK_MODE                _IOW('O',307,unsigned char)
#define AUDIO_DECA_DECORE_SET_SYNC_MODE                 _IOW('O',308,enum AUDIO_AV_SYNC_MODE)
#define AUDIO_DECA_DECORE_GET_CUR_TIME                  _IOR('O',309,unsigned long)
#define AUDIO_DECA_DECORE_GET_STATUS                    _IOR('O',310,struct audio_decore_status)


struct ali_audio_ioctl_command
{
	unsigned long ioctl_cmd;
	unsigned long param;
};


#endif
