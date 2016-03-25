 #ifndef __ALI_AUDIO_H
#define __ALI_AUDIO_H

#include "ali_basic.h"

#define RPC_AUDIO_DECORE_IOCTL      1

#define MAX_AUDIO_RPC_ARG_NUM		8
#define MAX_AUDIO_RPC_ARG_SIZE		512

#define DECA_CMD_BASE				0x00
#define DECA_SET_STR_TYPE			(DECA_CMD_BASE	+ 1)
#define DECA_GET_STR_TYPE			(DECA_CMD_BASE	+ 2)
#define DECA_SET_DOLBY_ONOFF		(DECA_CMD_BASE	+ 3)
#define DECA_AUDIO_KEY				(DECA_CMD_BASE	+ 4)//Don't support anymore, pls use DECA_STR_PLAY
#define DECA_BEEP_START			(DECA_CMD_BASE	+ 5)//Don't support anymore, pls use DECA_STR_PLAY
#define DECA_BEEP_STOP				(DECA_CMD_BASE	+ 6)//Don't support anymore, pls use DECA_STR_STOP
#define DECA_BEEP_INTERVAL			(DECA_CMD_BASE	+ 7)
#define DECA_SET_PLAY_SPEED		(DECA_CMD_BASE	+ 8)
#define DECA_HDD_PLAYBACK			(DECA_CMD_BASE + 9)
#define DECA_STR_PLAY				(DECA_CMD_BASE + 10)//Play a slice of audio bitstream in SDRAM
#define DECA_SET_MULTI_CH			(DECA_CMD_BASE + 11)//strongly recommend call this command in channel change task.
#define DECA_STR_PAUSE				(DECA_CMD_BASE + 12)
#define DECA_STR_RESUME			(DECA_CMD_BASE + 13)		
#define DECA_STR_STOP				(DECA_CMD_BASE + 14)
#define DECA_GET_AUDIO_INFO		(DECA_CMD_BASE + 15)
#define DECA_GET_HIGHEST_PTS		(DECA_CMD_BASE + 16)

#define DECA_MP3DEC_INIT 			(DECA_CMD_BASE + 17)//Don't support anymore, pls use DECA_SOFTDEC_INIT
#define DECA_MP3DEC_CLOSE 			(DECA_CMD_BASE + 18)//Don't support anymore, pls use DECA_SOFTDEC_CLOSE
#define DECA_MP3_CAN_DECODE 		(DECA_CMD_BASE + 19)//Don't support anymore.
#define DECA_MP3_GET_ELAPSE_TIME 	(DECA_CMD_BASE + 20)//Don't support anymore.
#define DECA_MP3_JUMP_TIME 		(DECA_CMD_BASE + 21)//Don't support anymore, pls use DECA_SOFTDEC_JUMP_TIME
#define DECA_MP3_SET_TIME 			(DECA_CMD_BASE + 22)//Don't support anymore, pls use DECA_SOFTDEC_SET_TIME
#define DECA_MP3_IS_PLAY_END 		(DECA_CMD_BASE + 23)//Don't support anymore, pls use DECA_SOFTDEC_IS_PLAY_END
#define DECA_PCM_FRM_LATE			(DECA_CMD_BASE + 24)
#define DECA_SET_AV_SYNC_LEVEL		(DECA_CMD_BASE + 25)
#define DECA_SOFTDEC_REGISTER_CB 		(DECA_CMD_BASE + 26)
#define DECA_SOFTDEC_INIT				(DECA_CMD_BASE + 27)
#define DECA_SOFTDEC_CLOSE			(DECA_CMD_BASE + 28)
#define DECA_SOFTDEC_JUMP_TIME		(DECA_CMD_BASE + 29)
#define DECA_SOFTDEC_SET_TIME			(DECA_CMD_BASE + 30)
#define DECA_SOFTDEC_IS_PLAY_END		(DECA_CMD_BASE + 31)
#define DECA_SOFTDEC_INIT2			(DECA_CMD_BASE + 32)
#define DECA_SOFTDEC_CLOSE2			(DECA_CMD_BASE + 33)
#define DECA_SOFTDEC_CAN_DECODE2		(DECA_CMD_BASE + 34)
#define DECA_SOFTDEC_GET_ELAPSE_TIME2	(DECA_CMD_BASE + 35)
#define DECA_SOFTDEC_GET_MUSIC_INFO2	(DECA_CMD_BASE + 36)
#define DECA_SOFTDEC_JUMP_TIME2		(DECA_CMD_BASE + 37)
#define DECA_SOFTDEC_IS_PLAY_END2		(DECA_CMD_BASE + 38)
#define DECA_SOFTDEC_REGISTER_CB2		(DECA_CMD_BASE + 39)
#define DECA_PLAY_MEDIA_STR			(DECA_CMD_BASE + 40)
#define DECA_EMPTY_BS_SET				(DECA_CMD_BASE + 41)
#define DECA_ADD_BS_SET					(DECA_CMD_BASE + 42)
#define DECA_DEL_BS_SET					(DECA_CMD_BASE + 43)
#define DECA_IS_BS_MEMBER				(DECA_CMD_BASE + 44)
#define DECA_AUDIO_PTS_SYNC_STC 		(DECA_CMD_BASE + 45)
#define DECA_REG_PCM_PROCESS_FUNC	(DECA_CMD_BASE + 46)
#define DECA_SYNC_BY_SOFT           (DECA_CMD_BASE + 47)
#define DECA_DOLBYPLUS_CONVERT_ONOFF	(DECA_CMD_BASE + 48)
#define DECA_DOLBYPLUS_CONVERT_STATUS	(DECA_CMD_BASE + 49)
#define DECA_RESET_BS_BUFF              (DECA_CMD_BASE + 50)
#define DECA_REG_PCM_BS_PROCESS_FUNC    (DECA_CMD_BASE + 51)
#define DECA_GET_AUDIO_DECORE           (DECA_CMD_BASE + 52)
#define DECA_DOLBYPLUS_DEMO_ONOFF       (DECA_CMD_BASE + 53)
#define DECA_SET_BUF_MODE               (DECA_CMD_BASE + 54)//for M3105 only
#define DECA_GET_BS_FRAME_LEN           (DECA_CMD_BASE + 55)
#define DECA_INDEPENDENT_DESC_ENABLE    (DECA_CMD_BASE + 56)
#define DECA_GET_DESC_STATUS            (DECA_CMD_BASE + 57)
#define DECA_GET_DECODER_HANDLE         (DECA_CMD_BASE + 58)
#define DECA_SYNC_NEXT_HEADER           (DECA_CMD_BASE + 59)
#define DECA_DO_DDP_CERTIFICATION       (DECA_CMD_BASE + 60)
#define DECA_DYNAMIC_SND_DELAY          (DECA_CMD_BASE + 61)
#define DECA_GET_DDP_INMOD              (DECA_CMD_BASE + 62)
#define DECA_GET_DECA_STATE             (DECA_CMD_BASE + 63)
#define DECA_GET_DDP_PARAM              (DECA_CMD_BASE + 64)
#define DECA_SET_DDP_PARAM              (DECA_CMD_BASE + 65)
#define DECA_CONFIG_BS_BUFFER           (DECA_CMD_BASE + 66)
#define DECA_CONFIG_BS_LENGTH           (DECA_CMD_BASE + 67)
#define DECA_BS_BUFFER_RESUME           (DECA_CMD_BASE + 68)
#define DECA_PTS_DELAY                  (DECA_CMD_BASE + 69) //param [0, 200)
#define DECA_DOLBY_SET_VOLUME_DB        (DECA_CMD_BASE + 70)

#define DECA_GET_PLAY_PARAM		        (DECA_CMD_BASE + 71)
#define DECA_ADV_IO					    (DECA_CMD_BASE + 0x200)
#define DECA_SET_REVERB			        (DECA_ADV_IO + 1)
#define DECA_SET_PL_II				    (DECA_ADV_IO+ 2)
#define DECA_SET_AC3_MODE			    (DECA_ADV_IO+ 3)
#define DECA_SET_AC3_STR_MODE		    (DECA_ADV_IO+ 4)
#define DECA_GET_AC3_BSMOD		        (DECA_ADV_IO+ 5)
#define SET_PASS_CI					    (DECA_ADV_IO+ 6)
#define DECA_CHECK_DECODER_COUNT        (DECA_ADV_IO+ 7)
#define DECA_SET_DECODER_COUNT          (DECA_ADV_IO+ 8)
#define DECA_SET_AC3_COMP_MODE          (DECA_ADV_IO+ 9)
#define DECA_SET_AC3_STEREO_MODE        (DECA_ADV_IO+ 10)

#define DECA_IO_GET_INPUT_CALLBACK_ROUTINE (DECA_ADV_IO+ 100)

#define DECA_DECORE_IO					(DECA_CMD_BASE + 0x300)
#define DECA_DECORE_INIT                (DECA_CMD_BASE + 1)
#define DECA_DECORE_RLS                 (DECA_CMD_BASE + 2)
#define DECA_DECORE_SET_BASE_TIME       (DECA_CMD_BASE + 3)
#define DECA_DECORE_GET_PCM_TRD         (DECA_CMD_BASE + 4)
#define DECA_DECORE_PAUSE_DECODE        (DECA_CMD_BASE + 5)
#define DECA_DECORE_FLUSH               (DECA_CMD_BASE + 6)
#define DECA_DECORE_GET_CUR_TIME		(DECA_CMD_BASE + 7)
#define DECA_DECORE_SET_QUICK_MODE      (DECA_CMD_BASE + 8)
#define DECA_DECORE_SET_SYNC_MODE       (DECA_CMD_BASE + 9)

#define ADEC_PLUGIN_NO_EXIST    4

struct audio_config
{
    int32 decode_mode;
    int32 sync_mode;
    int32 sync_unit;
    int32 deca_input_sbm;
    int32 deca_output_sbm;
    int32 snd_input_sbm;
    int32 pcm_sbm;
    int32 codec_id;
    int32 bits_per_coded_sample;
    int32 sample_rate;
    int32 channels;
	int32 bit_rate;
	uint32 pcm_buf;
	uint32 pcm_buf_size;
	int32 block_align;
	int32 codec_frame_size;
	uint8 extradata[100];
	uint32 extradata_size;
	uint8 extradata_mode;
};

struct audio_frame
{
    int64 pts;
    uint32 size;
    uint32 pos;
    uint32 stc_id;
};

struct ali_audio_rpc_arg
{
	void *arg;
	int arg_size;
	int out;
};

struct ali_audio_rpc_pars
{
	int API_ID;
	struct ali_audio_rpc_arg arg[MAX_AUDIO_RPC_ARG_NUM];
	int arg_num;
};
typedef struct control_block ali_audio_ctrl_blk;
#endif
