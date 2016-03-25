
#ifndef _ALI_DMX_COMMON_H_
#define _ALI_DMX_COMMON_H_

#include <linux/types.h>
#include "ali_basic_common.h"

/*! @addtogroup DeviceDriver
 *  @{
 */

/*! @addtogroup ALiDMX
 *  @{
 */


#define DMX_SEC_MASK_MAX_LEN     32	//!< Max filter depth of section
#define DMX_REC_PID_LIST_MAX_LEN 32	//!< Max length of demux record pid list 

/*! @enum dmx_channel_output_space
@brief Output space of demux channel
*/
enum dmx_channel_output_space
{
    DMX_OUTPUT_SPACE_USER,		//!< Demux channel output user space
    DMX_OUTPUT_SPACE_KERNEL,	//!< Demux channel output kernel space
    DMX_OUTPUT_SPACE_USER_EX,	//!< Demux channel output user space for extension. Not used now.
};

/*! @enum dmx_channel_output_format
@brief Output format of demux channel
*/
enum dmx_channel_output_format
{
    DMX_CHANNEL_OUTPUT_FORMAT_TS,	//!< Demux channel output TS format
    DMX_CHANNEL_OUTPUT_FORMAT_PES,	//!< Demux channel output PES format
    DMX_CHANNEL_OUTPUT_FORMAT_SEC,	//!< Demux channel output SEC format
    DMX_CHANNEL_OUTPUT_FORMAT_PCR	//!< Demux channel output PCR format
};

/*! @enum dmx_channel_state
@brief State of demux channel
*/
enum dmx_channel_state
{
    DMX_CHANNEL_IDLE,		//!< Idle state of demux channel 
    DMX_CHANNEL_READY,		//!< Ready state of demux channel 
    DMX_CHANNEL_RUN,		//!< Run state of demux channel 
    DMX_CHANNEL_OVERFLOW	//!< Overflow state of demux channel 
};

/*! @enum dmx_see_av_sync_mode
@brief Video and audio sync mode of SEE CPU.
*/
enum dmx_see_av_sync_mode
{
    DMX_SEE_AV_SYNC_MODE_LIVE = 0,				//!< Live play mode
    DMX_SEE_AV_SYNC_MODE_PLAYBACK = 1,			//!< Playback mode
    DMX_SEE_AV_SYNC_MODE_TSG_TIMESHIT = 2,		//!< Tsg timeshift mode
    DMX_SEE_AV_SYNC_MODE_TSG_LIVE = 3,  		//!< Tsg live play mode
    DMX_SEE_AV_SYNC_MODE_FREERUN = 4,  			//!< Freerun mode
    DMX_SEE_AV_SYNC_MODE_PLAYBACK_LEGACY = 5,	//!< Playebakc legacy mode
};

/*! @struct dmx_sec_channel_param
@brief Parameter of demux section channel
*/
struct dmx_sec_channel_param
{
    __u16 pid;							//!< Pid of demux section channel
    __u8  mask[DMX_SEC_MASK_MAX_LEN];	//!< Mask of demux section channel
    __u8  value[DMX_SEC_MASK_MAX_LEN];	//!< Value of demux section channel
    __u32 mask_len;						//!< Mask length of demux section channel 
    __u32 timeout;						//!< Timeout of demux section channel. Not used now.
    __u32 option;						//!< Option of demux section channel. Not used now.
};

/*! @struct dmx_pes_channel_param
@brief Parameter of demux pes channel
*/
struct dmx_pes_channel_param
{
    __u16 pid;													//!< Pid of demux PES channel
    __u32 timeout;												//!< Timeout of demux PES channel

    /* For kernel space only. */
    struct ali_dec_input_callback_routine_pars callback_para;	//!< Input callback routine of demux PES channel
};

/*! @struct dmx_pcr_channel_param
@brief Parameter of demux PCR channel
*/
struct dmx_pcr_channel_param
{
    __u16 pid;	//!< Pid of demux pcr channel
};


/*! @struct dmx_ts_kern_recv_info
@brief Ts receive info in kernel
*/
struct dmx_ts_kern_recv_info
{
    __u32 kern_recv_routine;		//!< Receive routine
    
    __u32 kern_recv_routine_para;	//!< Receive routine parameter
};

/*! @struct dmx_ts_channel_param
@brief Parameter of demux TS channel
*/
struct dmx_ts_channel_param
{  
    __u32 pid_list_len;								//!< Pid list length of demux TS channel
    __u16 pid_list[DMX_REC_PID_LIST_MAX_LEN];		//!< Pid list array of demux TS channel

	__u8 needdiscramble[DMX_REC_PID_LIST_MAX_LEN];	//!< Pid descramble attribute array. pid_list array element and NeedDiscramble array element is one to one correspondence.1-need to descramble 0-not need.

    /* Kernel space only. */
    struct dmx_ts_kern_recv_info kern_recv_info;	//!< Ts receive info of demux TS channel
};

/*! @struct dmx_channel_param
@brief Parameter of demux channel
*/
struct dmx_channel_param
{
    enum dmx_channel_output_format output_format;	//!< Output format of demux channel

    enum dmx_channel_output_space output_space;		//!< Output space of demux channel

    struct dmx_ts_channel_param ts_param;			//!< Parameter of demux TS channel

    struct dmx_sec_channel_param sec_param;			//!< Parameter of demux section channel

    struct dmx_pes_channel_param pes_param;			//!< Parameter of demux PES channel

	struct dmx_pcr_channel_param pcr_param;			//!< Parameter of demux PCR channel

	
    __u32 enc_para;									//!< Encrypt parameter of demux channel 
    __u32 rec_whole_tp;								//!< Record whole transport stream
    
    __s32 fst_cpy_slot;								//!< Not use now
    
    __u32 buf_size;									//!< Buffer size of demux channel, not use now
    
    __u32  fe;										//!< Frontend which connect to demux channel, not usd now
    __u32 nim_chip_id;								//!< Frontend which connect to demux channel, this is chip id of the frontend, not use now.
	__u32 dec_para;									//!< Dencrypt parameter of demux channel 
};

/*! @struct dmx_see_av_pid_info
@brief video, audio, pcr, audio description pid info in SEE CPU
*/
struct dmx_see_av_pid_info
{
    __u16 v_pid;	//!< Video pid info
    __u16 a_pid;	//!< Audio pid info
    __u16 p_pid;	//!< Pcr pid info
    __u16 ad_pid;	//!< Audio description pid info
};

/*! @struct dmx_see_av_scram_info
@brief Not used now
*/
struct dmx_see_av_scram_info
{
     __u16 pid[2];
     __u8 scram_status;
};

/*! @struct dmx_fast_copy_param
@brief Not used now
*/
struct dmx_fast_copy_param
{
#ifdef __CHECKER__
    const char __user  *data;
#else
	const char *data;
#endif
    __s32 len;
};


#define ALI_DMX_IOC_MAGIC  0xA1	//!< Magic for demux ioctl 


/* Legacy IO control command, handled by device "ali_m36_dmx_0".
 */

/*!
  \def ALI_DMX_CHANNEL_START
  Demux channel start with demux channel parameter. 
  struct dmx_channel_param-start paremeter of demux channel.
*/ 
#define ALI_DMX_CHANNEL_START _IOW(ALI_DMX_IOC_MAGIC, 11, struct dmx_channel_param)

/*!
  \def ALI_DMX_CHANNEL_STOP
  Demux channel stop.
*/ 
#define ALI_DMX_CHANNEL_STOP  _IO(ALI_DMX_IOC_MAGIC, 12)

/*!
  \def ALI_DMX_CHANNEL_GET_CUR_PKT_LEN
  Get data buffer info of demux channel. Section or PES channel get first package length. TS channel get data buffer length. 
  __u32-length pointer paremeter.
*/
#define ALI_DMX_CHANNEL_GET_CUR_PKT_LEN  _IOR(ALI_DMX_IOC_MAGIC, 13, __u32)

/*!
  \def ALI_DMX_CHANNEL_GET_PKT_LEN
  The same to ALI_DMX_CHANNEL_GET_CUR_PKT_LEN.
*/
#define ALI_DMX_CHANNEL_GET_PKT_LEN  ALI_DMX_CHANNEL_GET_CUR_PKT_LEN//_IOR(ALI_DMX_IOC_MAGIC, 15, __u32)

/*!
  \def ALI_DMX_CHANNEL_FLUSH_BUF
  Demux hardware dma buffer flush. 
*/
#define ALI_DMX_CHANNEL_FLUSH_BUF  _IO(ALI_DMX_IOC_MAGIC, 14)

/*!
  \def ALI_DMX_CHANNEL_GET_STARTCODE_OFFSET
  Not use now.
*/
#define ALI_DMX_CHANNEL_GET_STARTCODE_OFFSET  _IOR(ALI_DMX_IOC_MAGIC, 15, __u32)

/*!
  \def ALI_DMX_CHANNEL_FLUSH_STARTCODE_OFFSET
  Not use now.
*/
#define ALI_DMX_CHANNEL_FLUSH_STARTCODE_OFFSET  _IOR(ALI_DMX_IOC_MAGIC, 16, __u32)

/*!
  \def ALI_DMX_HW_GET_FREE_BUF_LEN
 Get number of ts package in demux hardware dma buffer(188bytes ts package per unit).
  __u32-length pointer paremeter.
*/
#define ALI_DMX_HW_GET_FREE_BUF_LEN  _IOR(ALI_DMX_IOC_MAGIC, 17, __u32)

/*!
  \def ALI_DMX_GET_PROG_BITRATE
  Get demux bitrate. This is virtual.
  __u32-bitrate pointer paremeter.
*/
#define ALI_DMX_GET_PROG_BITRATE  _IOR(ALI_DMX_IOC_MAGIC, 18, __u32)

/*!
  \def ALI_DMX_PLAYBACK_FSTCPY
  Write data to demux for playback.
  __u32-struct dmx_fast_copy_param pointer paremeter.
*/
#define ALI_DMX_PLAYBACK_FSTCPY   _IOW(ALI_DMX_IOC_MAGIC, 19, __u32)

/*!
  \def ALI_DMX_IO_SET_BYPASS_MODE
  Start to bypass all ts package.
  __u32-ts package buffer address.
*/
#define ALI_DMX_IO_SET_BYPASS_MODE  _IOW(ALI_DMX_IOC_MAGIC, 40, __u32)

/*!
  \def ALI_DMX_IO_BYPASS_GETDATA
  Get data in demux bypass mode.
  __u32-not use now.
*/
#define ALI_DMX_IO_BYPASS_GETDATA  _IOW(ALI_DMX_IOC_MAGIC, 41, __u32)

/*!
  \def ALI_DMX_IO_CLS_BYPASS_MODE
  Stop to bypass all ts package.
  __u32-not use now.
*/
#define ALI_DMX_IO_CLS_BYPASS_MODE  _IOW(ALI_DMX_IOC_MAGIC, 42, __u32)

/*!
  \def ALI_DMX_IO_SET_PLAYBACK_SRC_TYPE
  Not use now.
*/
#define ALI_DMX_IO_SET_PLAYBACK_SRC_TYPE  _IOW(ALI_DMX_IOC_MAGIC, 43, __u32)

/*!
  \def ALI_DMX_BYPASS_ALL
  Not use now.
*/
#define ALI_DMX_BYPASS_ALL  _IOW(ALI_DMX_IOC_MAGIC, 44, __u32)

/*!
  \def ALI_DMX_RESET_BITRATE_DETECT
  Not use now.
*/
#define ALI_DMX_RESET_BITRATE_DETECT  _IOW(ALI_DMX_IOC_MAGIC, 45, __u32)

/*!
  \def ALI_DMX_GET_CHANNEL_STATE
  Not use now.
*/
#define ALI_DMX_GET_CHANNEL_STATE  _IOR(ALI_DMX_IOC_MAGIC, 46, __u32)

/*!
  \def ALI_DMX_SEE_AV_START
  Start to play video and audio in SEE CPU. 
  struct dmx_see_av_pid_info-video, audio, pcr, audio description pid info in SEE CPU.
*/
#define ALI_DMX_SEE_AV_START _IOW(ALI_DMX_IOC_MAGIC, 70, struct dmx_see_av_pid_info)

/*!
  \def ALI_DMX_SEE_AV_STOP
  Stop play video and audio in SEE CPU.
*/
#define ALI_DMX_SEE_AV_STOP  _IO(ALI_DMX_IOC_MAGIC, 71)

/*!
  \def ALI_DMX_SEE_A_CHANGE_PID
  Change audio description in see cpu.
  struct dmx_see_av_pid_info-video, audio, pcr, audio description pid info in SEE CPU.
*/
#define ALI_DMX_SEE_A_CHANGE_PID _IOW(ALI_DMX_IOC_MAGIC, 72, struct dmx_see_av_pid_info)

/*!
  \def ALI_DMX_SEE_TELETXT_START
  Start teletext in SEE CPU.
  __u16-teletext pid.
*/
#define ALI_DMX_SEE_TELETXT_START _IOW(ALI_DMX_IOC_MAGIC, 73, __u16)

/*!
  \def ALI_DMX_SEE_SUBTITLE_START
  Start subtitle in SEE CPU.
  __u16-subtitle pid.
*/
#define ALI_DMX_SEE_SUBTITLE_START _IOW(ALI_DMX_IOC_MAGIC, 74, __u16)

/*!
  \def ALI_DMX_SEE_AV_SYNC_MODE_SET
  Set video and audio sync mode in SEE CPU.
  enum dmx_see_av_sync_mode-video and audio sync mode in SEE CPU.
*/
#define ALI_DMX_SEE_AV_SYNC_MODE_SET _IOW(ALI_DMX_IOC_MAGIC, 75, enum dmx_see_av_sync_mode)

/*!
  \def ALI_DMX_SEE_CRYPTO_TYPE_SET
  Set crypto type in SEE CPU.
  struct dec_parse_param-crypto param.
*/
#define ALI_DMX_SEE_CRYPTO_TYPE_SET  _IOW(ALI_DMX_IOC_MAGIC, 76, struct dec_parse_param)

/*!
  \def ALI_DMX_SEE_CRYPTO_START
  Start crypto in SEE CPU.
*/
#define ALI_DMX_SEE_CRYPTO_START _IO(ALI_DMX_IOC_MAGIC, 77)

/*!
  \def ALI_DMX_SEE_CRYPTO_STOP
  Stop crypto in SEE CPU.
*/
#define ALI_DMX_SEE_CRYPTO_STOP _IO(ALI_DMX_IOC_MAGIC, 78)

/*!
  \def ALI_DMX_SEE_AV_SCRAMBLE_STATUS
  Not use now.
*/
#define ALI_DMX_SEE_AV_SCRAMBLE_STATUS _IOR(ALI_DMX_IOC_MAGIC, 79, __u8)

/*!
  \def ALI_DMX_SEE_GET_DISCONTINUE_COUNT
  Get discontinue package count in SEE CPU.
  __u32-discontinue package count.
*/
#define ALI_DMX_SEE_GET_DISCONTINUE_COUNT   _IOR(ALI_DMX_IOC_MAGIC, 80, __u32)

/*!
  \def ALI_DMX_SEE_GET_MAIN2SEEBUF_REAMIN_DATA_LEN
  Get data remain length in share buffer of Main and SEE CPU.
  __u32-data remain length in share buffer of Main and SEE CPU.
*/
#define ALI_DMX_SEE_GET_MAIN2SEEBUF_REAMIN_DATA_LEN   _IOR(ALI_DMX_IOC_MAGIC, 81, __u32)

/*!
  \def ALI_DMX_SEE_AV_SCRAMBLE_STATUS_EXT
  Not use now.
*/
#define ALI_DMX_SEE_AV_SCRAMBLE_STATUS_EXT _IOWR(ALI_DMX_IOC_MAGIC, 90, struct dmx_see_av_scram_info)

/*!
  \def ALI_DMX_SEE_SET_PLAYBACK_SPEED
  Set playback speed in SEE CPU.
  __u32-playback speed.
*/
#define ALI_DMX_SEE_SET_PLAYBACK_SPEED _IOW(ALI_DMX_IOC_MAGIC, 91, __u32)

/*!
  \def ALI_DMX_SEE_SET_UPDATE_REMAIN
  Update remain data in SEE CPU.
  __u32-zero is reset pes parameter.
*/
#define ALI_DMX_SEE_SET_UPDATE_REMAIN _IOW(ALI_DMX_IOC_MAGIC, 92, __u32)

/*!
  \def ALI_DMX_SEE_GET_TS_INPUT_ROUTINE
  Get ts input routine.
  struct dmx_ts_kern_recv_info-ts receive info in kernel.
*/
#define ALI_DMX_SEE_GET_TS_INPUT_ROUTINE _IOR(ALI_DMX_IOC_MAGIC, 93, struct dmx_ts_kern_recv_info)

/*!
  \def ALI_DMX_GET_LATEST_PCR
  Not use now.
*/
#define ALI_DMX_GET_LATEST_PCR _IOR(ALI_DMX_IOC_MAGIC, 94, __u32)

/*!
  \def ALI_DMX_SEE_PLAYBACK_PAUSE
  Not use now.
*/
#define ALI_DMX_SEE_PLAYBACK_PAUSE          _IOW(ALI_DMX_IOC_MAGIC, 95, __u32)

/*!
  \def ALI_DMX_SEE_PLAYBACK_PRE_PAUSE
  Not used now.
*/
#define ALI_DMX_SEE_PLAYBACK_PRE_PAUSE      _IOW(ALI_DMX_IOC_MAGIC, 96, __u32)

/*!
  \def ALI_DMX_SEE_CLEAR_DISCONTINUE_COUNT
  clear discontinue package count in SEE CPU.
  __u32-Not used now.
*/
#define ALI_DMX_SEE_CLEAR_DISCONTINUE_COUNT _IOW(ALI_DMX_IOC_MAGIC, 97, __u32)

/*!
  \def ALI_DMX_SEE_DO_DDP_CERTIFICATION
  Not used now.
*/
#define ALI_DMX_SEE_DO_DDP_CERTIFICATION    _IOW(ALI_DMX_IOC_MAGIC, 98, __u32)

/*!
  \def ALI_DMX_SET_HW_INFO
  Not used now.
*/
#define ALI_DMX_SET_HW_INFO                 _IO(ALI_DMX_IOC_MAGIC, 99)

//#define ALI_DMX_CHANNEL_SET_TO_PES     _IOW(ALI_DMX_IOC_MAGIC, 44, struct dmx_pes_channel_params)
//#define ALI_DMX_CHANNEL_GET_TEST_PES_PARAM   _IOR(ALI_DMX_IOC_MAGIC, 45, struct ali_video_input_callback_routine_pars)

/*!
  \def ALI_DMX_CHANNEL_ADD_PID
  Not used now.
*/
#define ALI_DMX_CHANNEL_ADD_PID         _IOW(ALI_DMX_IOC_MAGIC, 100, struct dmx_channel_param)

/*!
  \def ALI_DMX_CHANNEL_DEL_PID
  Not used now.
*/
#define ALI_DMX_CHANNEL_DEL_PID         _IOW(ALI_DMX_IOC_MAGIC, 101, struct dmx_channel_param)

/*!
  \def ALI_DMX_SEE_SET_SAT2IP
  Not used now.
*/
#define ALI_DMX_SEE_SET_SAT2IP          _IOW(ALI_DMX_IOC_MAGIC, 102, __u32)

/*!
  \def ALI_DMX_SEE_SET_SEE2MAIN
  Not used now.
*/
#define ALI_DMX_SEE_SET_SEE2MAIN		_IOW(ALI_DMX_IOC_MAGIC, 103, __u32)  

/*!
  \def ALI_DMX_SEE_DCII_SUBT_START
  Start subtitle.
  __u32-subtitle pid.
*/
#define ALI_DMX_SEE_DCII_SUBT_START _IOW(ALI_DMX_IOC_MAGIC, 104, __u32)  

/* New DMX IO control commands.
*/

/*!
  \def ALI_DMX_SEE_GET_STATISTICS
  Get demux statistics info in SEE CPU.
  struct Ali_DmxSeeStatistics-demux statistics info in SEE CPU.
*/
#define ALI_DMX_SEE_GET_STATISTICS _IOR(ALI_DMX_IOC_MAGIC, 1085, struct Ali_DmxSeeStatistics)

/*!
  \def ALI_DMX_CHAN_SEC_INFO_GET
  Get stream info of demux section channel.
  struct Ali_DmxDrvSecStrmStatInfo-stream info of demux section channel.
*/
#define ALI_DMX_CHAN_SEC_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1086, struct Ali_DmxDrvSecStrmStatInfo)

/*!
  \def ALI_DMX_CHAN_SEC_FILTER_INFO_GET
  Get filter info of demux section channel.
  struct Ali_DmxDrvSecFltStatInfo-filter info of demux section channel.
*/
#define ALI_DMX_CHAN_SEC_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1087, struct Ali_DmxDrvSecFltStatInfo)

/*!
  \def ALI_DMX_CHAN_TS_INFO_GET
  Get stream info of demux TS channel.
  struct Ali_DmxDrvTsStrmStatInfo-stream info of demux TS channel.
*/
#define ALI_DMX_CHAN_TS_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1088, struct Ali_DmxDrvTsStrmStatInfo)

/*!
  \def ALI_DMX_CHAN_TS_FILTER_INFO_GET
  Get filter info of demux TS channel.
  struct Ali_DmxDrvTsFltStatInfo-filter info of demux TS channel.
*/
#define ALI_DMX_CHAN_TS_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1089, struct Ali_DmxDrvTsFltStatInfo)

//Used in the Section
/*!
  \def ALI_DMX_CHAN_SEC_TS_FILTER_INFO_GET
  Get ts filter info of demux section channel.
  struct Ali_DmxDrvTsFltStatInfo-ts filter info of demux section channel.
*/
#define ALI_DMX_CHAN_SEC_TS_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1090, struct Ali_DmxDrvTsFltStatInfo)

/*!
  \def ALI_DMX_CHAN_AUDIO_INFO_GET
  Get stream info of demux audio channel.
  struct Ali_DmxDrvAudioStrmStatInfo-stream info of demux audio channe.
*/
#define ALI_DMX_CHAN_AUDIO_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1091, struct Ali_DmxDrvAudioStrmStatInfo)

/*!
  \def ALI_DMX_CHAN_AUDIO_FILTER_INFO_GET
  Get filter info of demux audio channel.
  struct Ali_DmxDrvTsFltStatInfo-filter info of demux audio channel.
*/
#define ALI_DMX_CHAN_AUDIO_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1092, struct Ali_DmxDrvTsFltStatInfo)

/*!
  \def ALI_DMX_CHAN_AUDIO_SEE_INFO_GET
  Get info of demux audio channel in SEE CPU.
  struct Ali_DmxSeePlyChStatInfo-info of demux audio channel in SEE CPU.
*/
#define ALI_DMX_CHAN_AUDIO_SEE_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1093, struct Ali_DmxSeePlyChStatInfo)

/*!
  \def ALI_DMX_CHAN_VIDEO_INFO_GET
  Get stream info of demux video channel.
  struct Ali_DmxDrvVideoStrmStatInfo-stream info of demux video channel.
*/
#define ALI_DMX_CHAN_VIDEO_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1094, struct Ali_DmxDrvVideoStrmStatInfo)

/*!
  \def ALI_DMX_CHAN_VIDEO_FILTER_INFO_GET
  Get filter info of demux video channel.
  struct Ali_DmxDrvTsFltStatInfo-filter info of demux video channel.
*/
#define ALI_DMX_CHAN_VIDEO_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1095, struct Ali_DmxDrvTsFltStatInfo)

/*!
  \def ALI_DMX_CHAN_VIDEO_SEE_INFO_GET
  Get info of demux video channel in SEE CPU.
  struct Ali_DmxSeePlyChStatInfo-info of demux video channel in SEE CPU.
*/
#define ALI_DMX_CHAN_VIDEO_SEE_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1096, struct Ali_DmxSeePlyChStatInfo)

/*!
  \def ALI_DMX_CHAN_PCR_INFO_GET
  Get stream info of demux pcr channel.
  struct Ali_DmxDrvPcrStrmStatInfo-stream info of demux pcr channel.
*/
#define ALI_DMX_CHAN_PCR_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1097, struct Ali_DmxDrvPcrStrmStatInfo)

/*!
  \def ALI_DMX_CHAN_PCR_FILTER_INFO_GET
  Get filter info of demux pcr channel.
  struct Ali_DmxDrvTsFltStatInfo-filter info of demux pcr channel.
*/
#define ALI_DMX_CHAN_PCR_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1098, struct Ali_DmxDrvTsFltStatInfo)

/*!
  \def ALI_DMX_CHAN_TS_IN_RAM_INFO_GET
  Not use now.
*/
#define ALI_DMX_CHAN_TS_IN_RAM_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 1099, struct Ali_DmxDrvTsInRamStrmStatInfo)

/*!
  \def ALI_DMX_CHAN_HW_REG_CFG
  Config to get hardware register of demux channel.
*/
#define ALI_DMX_CHAN_HW_REG_CFG 		_IO(ALI_DMX_IOC_MAGIC, 2000)

/*!
  \def ALI_DMX_CHAN_HW_REG_START
  Start to get hardware register of demux channel.
*/
#define ALI_DMX_CHAN_HW_REG_START 		_IO(ALI_DMX_IOC_MAGIC, 2001)

/*!
  \def ALI_DMX_CHAN_HW_REG_STOP
  Stop to get hardware register of demux channel.
*/
#define ALI_DMX_CHAN_HW_REG_STOP 		_IO(ALI_DMX_IOC_MAGIC, 2002)

/*!
  \def ALI_DMX_CHAN_HW_REG_INFO_GET
  Get register info of demux hardware.
  __u32-hardware register buffer pointer, size must be sizof(__u32 HwRegTable[18][5]).
*/
#define ALI_DMX_CHAN_HW_REG_INFO_GET 	_IOR(ALI_DMX_IOC_MAGIC, 2003, __u32)

/*!
  \def ALI_DMX_CHAN_KERN_GLB_CFG
  Config to get kernel global info of demux channel.
*/
#define ALI_DMX_CHAN_KERN_GLB_CFG 		_IO(ALI_DMX_IOC_MAGIC, 2010)

/*!
  \def ALI_DMX_CHAN_KERN_GLB_START
  Start to get kernel global info of demux channel.
*/
#define ALI_DMX_CHAN_KERN_GLB_START 	_IO(ALI_DMX_IOC_MAGIC, 2011)

/*!
  \def ALI_DMX_CHAN_KERN_GLB_STOP
  Stop to get kernel global info of demux channel.
*/
#define ALI_DMX_CHAN_KERN_GLB_STOP 		_IO(ALI_DMX_IOC_MAGIC, 2012)

/*!
  \def ALI_DMX_CHAN_KERN_GLB_INFO_GET
  Get kernel global info of demux channel.
  struct Ali_DmxKernGlobalStatInfo-kernel global info.
*/
#define ALI_DMX_CHAN_KERN_GLB_INFO_GET 	_IOR(ALI_DMX_IOC_MAGIC, 2013, struct Ali_DmxKernGlobalStatInfo)

/*!
  \def ALI_DMX_CHAN_KERN_GLB_REALTIME_SET
  For get kernel global info of demux channel, set interval realtime.
  struct Ali_DmxKernGlobalStatInfo-interval realtime(regard as __U32).
*/
#define ALI_DMX_CHAN_KERN_GLB_REALTIME_SET 	_IOW(ALI_DMX_IOC_MAGIC, 2014, struct Ali_DmxKernGlobalStatInfo)

/*!
  \def ALI_DMX_CHAN_SEE_GLB_CFG
  Config to get see global info of demux channel.
*/
#define ALI_DMX_CHAN_SEE_GLB_CFG 		_IO(ALI_DMX_IOC_MAGIC, 2020)

/*!
  \def ALI_DMX_CHAN_SEE_GLB_START
  Start to get see global info of demux channel.
*/
#define ALI_DMX_CHAN_SEE_GLB_START 		_IO(ALI_DMX_IOC_MAGIC, 2021)

/*!
  \def ALI_DMX_CHAN_SEE_GLB_STOP
  Stop to get see global info of demux channel.
*/
#define ALI_DMX_CHAN_SEE_GLB_STOP 		_IO(ALI_DMX_IOC_MAGIC, 2022)

/*!
  \def ALI_DMX_CHAN_SEE_GLB_INFO_GET
  Get see global info of demux channel.
  struct Ali_DmxSeeGlobalStatInfo-see global info.
*/
#define ALI_DMX_CHAN_SEE_GLB_INFO_GET 	_IOR(ALI_DMX_IOC_MAGIC, 2023, struct Ali_DmxSeeGlobalStatInfo)

/*!
  \def ALI_DMX_CHAN_SEE_GLB_REALTIME_SET
  For get see global info of demux channel, set interval realtime .
  struct Ali_DmxSeeGlobalStatInfo-interval realtime(regard as __U32).
*/
#define ALI_DMX_CHAN_SEE_GLB_REALTIME_SET 	_IOW(ALI_DMX_IOC_MAGIC, 2024, struct Ali_DmxSeeGlobalStatInfo)

/*!
  \def ALI_DMX_CHAN_TP_INFO_GET
  Not use now.
*/
#define ALI_DMX_CHAN_TP_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 2030, struct Ali_DmxDrvTpStrmStatInfo)

/*!
  \def ALI_DMX_CHAN_PES_INFO_GET
  Get stream info of demux PES channel.
  struct Ali_DmxDrvPesStrmStatInfo-stream info of demux pes channel.
*/
#define ALI_DMX_CHAN_PES_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 2040, struct Ali_DmxDrvPesStrmStatInfo)
/*!
  \def ALI_DMX_CHAN_PES_FILTER_INFO_GET
  Get filter info of demux PES channel.
  struct Ali_DmxDrvPesFltStatInfo-filter info of demux PES channel.
*/
#define ALI_DMX_CHAN_PES_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 2041, struct Ali_DmxDrvPesFltStatInfo)

/*!
  \def ALI_DMX_CHAN_PES_TS_FILTER_INFO_GET
  Get ts filter info of demux PES channel.
  struct Ali_DmxDrvTsFltStatInfo-ts filter info of demux PES channel.
*/
#define ALI_DMX_CHAN_PES_TS_FILTER_INFO_GET _IOR(ALI_DMX_IOC_MAGIC, 2042, struct Ali_DmxDrvTsFltStatInfo)

/*!
@}
*/

/*!
@}
*/
#endif


