#ifndef __ADF_AVSYNC__
#define __ADF_AVSYNC__

#include "adf_basic.h"
#include "adf_media.h"


/*! @addtogroup Devicedriver
 *  @{
     */


/*!  @addtogroup ALiAVSYNC
 *  @{
  */

#ifdef __cplusplus
extern "C" {
#endif

#define AVSYNC_IO_ENABLE_DDP_CERTIFICATION 0x01           //!< Enable DDP certificaiton
#define AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG   0x02     //!<  Configure video smoothly playback
#define AVSYNC_IO_ENABLE_GET_STC 0x03	                  //!<  Get STC for Main CPU
#define AVSYNC_IO_SET_PTS_UNIT_HZ   0x04               //!< Set PTS unit
#define AVSYNC_IO_UNREG_CALLBACK   0x05	              //!<  Unregister callback function
#define AVSYNC_IO_REG_CALLBACK   0x06                //!<  Register callback function
#define AVSYNC_IO_GET_CURRENT_PLAY_PTS   0x07     	//!<Get current PTS
#define AVSYNC_IO_PAUSE   0x08	                   //!< Pause AVSYN module
#define AVSYNC_IO_RESUME   0x09	                  //!< Resume AVSYN module
#define AVSYNC_IO_GET_CURRENT_STC	0x0A              //!< Get current STC
#define AVSYNC_IO_CHANGE_AUDIO_TRACK            0x0B  //!<Change auido track resync
#define AVSYNC_IO_STC_VALID            0x0C  //!<Notify SEE CPU STC is valid
#define AVSYNC_IO_GET_VIDEO_PTS            0x0D  //!<Get current video pts


#define AVSYNC_IO_SET_VPTS_SHM_ADDR            0xF0  //!<Get current video pts

/*! @enum LLD_AVSYNC_FUNC
@brief  This enmu is used for remoting call.
*/
enum LLD_AVSYNC_FUNC{
    FUNC_AVSYNC_ATTACH = 0,
};

 
 /*! @enum HLD_AVSYNC_FUNC
 @brief  This enmu is used for remoting call.
 */
enum HLD_AVSYNC_FUNC{
    FUNC_AVSYNC_OPEN = 0,   
    FUNC_AVSYNC_CLOSE,
    FUNC_AVSYNC_START,   
    FUNC_AVSYNC_STOP,
    FUNC_AVSYNC_RESET,
    FUNC_AVSYNC_IOCTL,
    FUNC_AVSYNC_SET_SYNCMODE,
    FUNC_AVSYNC_GET_SYNCMODE,
    FUNC_AVSYNC_CFG_PARAMS,
    FUNC_AVSYNC_GET_PARAMS,
    FUNC_AVSYNC_CFG_ADV_PARAMS,
    FUNC_AVSYNC_GET_ADV_PARAMS,
    FUNC_AVSYNC_SET_SRCTYPE,
    FUNC_AVSYNC_GET_SRCTYPE,
    FUNC_AVSYNC_GET_STATUS,
    FUNC_AVSYNC_GET_STATISTICS,
    FUNC_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF,
    FUNC_AVSYNC_DBG_SET_PRINT_OPTION,
    FUNC_AVSYNC_DBG_SET_POLL_ONOFF,
    FUNC_AVSYNC_DBG_SET_POLL_OPTION,
    FUNC_AVSYNC_DBG_SET_POLL_INTERVAL,
};

/*! @struct avsync_get_stc_en_t
@brief  This structure is used for getting STC clock by Main CPU.
*/

typedef struct 
{
	void *sharem_addr;
	UINT8 enable;
}avsync_get_stc_en_t;

/*! @enum AVSYNC_PTS_MODE
@brief  Current PTS is valid or not.
*/
typedef enum  
{
	AVSYNC_PTS_MODE_FREE,          //!<  No PTS 
	AVSYNC_PTS_MODE_CNT,          //!< Not used
	AVSYNC_PTS_MODE_VALID,       //!<  Valid PTS
	AVSYNC_PTS_MODE_INVALID,	  //!< No valid PTS
}AVSYNC_PTS_MODE;


/*! @enum AVSYNC_FRAME_SYNCFLAG_E
@brief  Current synchronization flag.
*/
typedef enum 
{
	AVSYNC_FRAME_PLAY,	           //!<  Play Audio/Video frame
	AVSYNC_FRAME_DROP,            //!<  Drop Audio/Video frame
	AVSYNC_FRAME_HOLD,           //!<  Hold Audio by software
	AVSYNC_FRAME_FREERUN,       //!< Free run frame
	AVSYNC_FRAME_HW_HOLD,      //!< Hold Audio frame by hardware
}AVSYNC_FRAME_SYNCFLAG_E;

/*! @enum AVSYNC_MODE_E
@brief Audio/video synchronization mode.The default configuration is AVSYNC_MODE_PCR in AVSYNC module, AVSYNC_MODE_VIDEO not supported temporarily.
*/
typedef enum 
{
	AVSYNC_MODE_PCR,       //!<Based on PCR.
	AVSYNC_MODE_AUDIO,     //!<Based on audio.
	AVSYNC_MODE_VIDEO,     //!< Based on video, not supported temporarily.
	AVSYNC_MODE_V_FREERUN, //!<Video is not synchronous,play directly.
	AVSYNC_MODE_A_FREERUN, //!< Audio is not synchronous,play directly.
	AVSYNC_MODE_AV_FREERUN, //!< Audio & video is not synchronous,play directly.
}AVSYNC_MODE_E;

/*! @ enum AVSYNC_SRCTYPE_E
@brief Audio/video data source type. The default configuration of AVSYNC module is #AVSYNC_SRC_TURNER.
}AVSYNC_MODE_E;
*/
typedef enum 
{
	AVSYNC_SRC_TURNER,           //!< Data comes from turner.
	AVSYNC_SRC_SWDMX,           //!<Data comes from software DMX.
	AVSYNC_SRC_HDD_MP,         //!< Data comes from local media play.
	AVSYNC_SRC_NETWORK_MP,    //!< Data comes from network media play.
}AVSYNC_SRCTYPE_E;
typedef enum 
{
	AVSYNC_H264,           //!<Decoder type is h.264
	AVSYNC_MPEG2,         //!< Decoder type is MPEG2
	AVSYNC_MPEG4,        //!<Decoder type is MPEG4
	AVSYNC_AVS,         //!< Decoder type is AVS
}AVSYNC_CODEC_TYPE;

/*! @ enum AVSYNC_DEVICE_STATUS
@brief  AVSYNC device status.
*/
typedef enum 
{
	AVSYNC_ATTACHED,         //!< Device attached
	AVSYNC_OPENED,          //!< Device opened
	AVSYNC_CLOSED,         //!< Device closed
	AVSYNC_STARTED,       //!< Device started
	AVSYNC_STOPPED,	     //!<Device stoppedd
}AVSYNC_DEV_STATUS;

/*! @ enum AVSYNC_VIDEO_SMOOTH_LEVEL
@brief The level of video playback smoothly.
*/
typedef enum 
{
	AVSYNC_VIDEO_SMOOTH_LEVEL1, 
	AVSYNC_VIDEO_SMOOTH_LEVEL2, 
}AVSYNC_VIDEO_SMOOTH_LEVEL;

/*! @ struct avsync_cfg_param_t
@brief Basic synchronization parameter. For the default configuration, please refer to below descriptions.
*/
typedef struct
{
	UINT32 vhold_thres;               //!< Video repeat threadhold, default value is 80ms.
	UINT32 vdrop_thres;              //!<Video drop threadhold, default value is 80ms.
	UINT32 ahold_thres;             //!< Audio repeat threadhold, default value is 40ms.
	UINT32 adrop_thres;            //!< Audio drop threadhold, default value is 64ms.
	AVSYNC_MODE_E sync_mode;      //!< Audio/video synchronization mode, default value is AVSYNC_MODE_PCR  
	AVSYNC_SRCTYPE_E src_type;   //!< Audio/video data source type, default value is AVSYNC_SRC_TURNER 
}avsync_cfg_param_t;

/*! @ struct avsync_adv_param_t
@brief  Advanced synchronization parameter. For the default configuration, please refer to below descriptions. 
*/
typedef struct
{
	UINT32 afreerun_thres;                   //!< Audio free run threadhold, default value is 10s.
	UINT32 vfreerun_thres;                  //!< Video free run threadhold, default value is 10s.
	UINT8  disable_monitor;                //!<Disable monitor function, default value is 1.
	UINT8 disable_first_video_freerun;    //!<Disable free run function of the video first frame, default value is 0.
	UINT16  dual_output_sd_delay;        //!<standard definition output delay of video dual output.
	UINT32  pts_adjust_threshold;
	UINT32 rsvd2; 
	UINT32 rsvd3;
}avsync_adv_param_t;


/*! @ struct avsync_vsync_info_t
@brief The structure description of video synchronization information.
*/
struct avsync_vsync_info_t
{
	UINT32 vpts;                                   //!< PTS of video frame
	AVSYNC_PTS_MODE	vpts_mode;                    //!< PTS mode of video frame
	UINT32 display_dly;                          //!< Display delay from AVSYNC flag request of real display
	UINT32 remain_esbuf_size;                   //!< Remained VBV buffer size
	UINT32 esbuf_size;                         //!<VBV buffer size
	UINT32 frame_duration;                    //!<Video frame duration
	UINT32 bitrate;                          //!< Video bit rate
	UINT8   stc_id;					                //!< SCT ID used
	UINT8   bstream_run_back;              //!< Stream run back flag
	UINT8   stc_offset_idx;			          //!< Offset index of STC used
	UINT8   decode_error;			           //!< ID of STC used
	AVSYNC_CODEC_TYPE   CODEC_type;	    //!< Video codec used
	BOOL pause_state;				           //!<In pause state
	UINT8 trick_state;              // in trick mode
};

 /*! @ struct avsync_vsync_info_t
@brief  The structure description of audio synchronization information.
*/
struct  avsync_async_info_t
{
	UINT32 cur_apts;                             //!<< PTS of audio frame
	UINT32 next_apts;                           //!< Next valid audio PTS
	UINT32 frm_num_two_pts;                    //!< Frame numbers between two PTS
	UINT32 remain_esbuf_size;                 //!< Remained audio ES buffer size
	UINT32 esbuf_size;                       //!< Audio ES buffer size
	UINT32 remain_pcm_time;                 //!< Play time in PCM buffer
	UINT32 frame_duration;                 //!<Audio frame duration
	UINT32 bitrate;                       //!<Video bit rate
	UINT8   stc_id;				               //!< STC ID used
	UINT8   bstream_run_back;           //!< Stream run back flag
	UINT8   pts_valid;			           //!< PTS valid flag@~
	UINT8   stc_offset_idx;		        //!<Offset index of STC used
	UINT32 total_remain_pcm_time;	   //!<PCM time remained in buffer
};


/*! @ struct avsync_status_t
@brief Module status
*/
typedef struct 
{
	AVSYNC_DEV_STATUS device_status;         //!<Device status~
	UINT32 vpts_offset;                     //!< Video PTS offset
	UINT32 apts_offset;                    //!< Audio PTS offset
	UINT8 v_sync_flg;                     //!< Video synchronization flag
	UINT8 a_sync_flg;                    //!< Audio synchronization flag
	UINT8 rsvd0;                        //!< Reserved
	UINT8 rsvd1;                       //!< Reserved

	UINT32 cur_vpts;                 //!< Current video PTS
	UINT32 cur_apts;                //!< Current audio PTS
}avsync_status_t;

/*! @ struct AVSYNC_SRCTYPE_E
@brief Synchronization statistical information@~
*/
typedef struct 
{
	UINT32 total_v_play_cnt;                 //!<Synchronization video frame numbers
	UINT32 total_v_drop_cnt;                //!< Dropped video frame numbers
	UINT32 total_v_hold_cnt;               //!< Repeated video frame numbers
	UINT32 total_v_freerun_cnt;           //!<Free-run video frame numbers
	UINT32 total_a_play_cnt;             //!< Synchronization audio frame numbers
	UINT32 total_a_drop_cnt;            //!< Dropped audio frame numbers
	UINT32 total_a_hold_cnt;           //!<Repeated audio frame numbers
	UINT32 total_a_freerun_cnt;       //!< Free-run audio frame numbers
}avsync_statistics_t;

/*! @ struct avsync_video_cb_t
@brief  A callback function , for registering to video driver.
*/
struct avsync_video_cb_t
{
	void *get_vync_flg_cb;           //!< Get video synchronization flag
	void *vbv_full_cb;              //!< Video buffer overflow event notification
	void *avsync_start;            //!< Used for starting AVSYNC driver
	void *avsync_stop;            //!<Used for stopping AVSYNC driver
};

/*! @ struct avsync_video_cb_t
@brief  A callback function , for registering to audio driver.
*/

struct avsync_audio_cb_t
{
	void *get_async_flg_cb;    //!< Get audio synchronization flag
};

/*! @ struct avsync_smoothly_play_cfg_t
@brief  Configuration for video smoothly play.
*/

struct  avsync_smoothly_play_cfg_t
{
	UINT8 onoff;		                       //!<1:Open; 0:Close
	UINT8 interval;	                      //!< Interval time
	AVSYNC_VIDEO_SMOOTH_LEVEL level;     //!< Level
};

#define AVSYNC_CB_MAX_DEV_NUM 4      //!< Maximum AVSYNC device numbers

/*! @ struct avsync_cb
@brief  Device pointer group
*/

struct avsync_cb
{
	void *dmx[AVSYNC_CB_MAX_DEV_NUM];       //!< The number of DMX device Pointer is up to AVSYNC_CB_MAX_DEV_NUM
	void *decv[AVSYNC_CB_MAX_DEV_NUM];     //!< The number of DECV device Pointer is up to AVSYNC_CB_MAX_DEV_NUM
	void *deca[AVSYNC_CB_MAX_DEV_NUM];    //!< The number of DECA device Pointer is up to AVSYNC_CB_MAX_DEV_NUM
	void *snd[AVSYNC_CB_MAX_DEV_NUM];    //!< The number of SND device Pointer is up to AVSYNC_CB_MAX_DEV_NUM
};

/*! @struct avsync_device
@brief Device type definition, includes member variables and provided operation interfaces.
*/
struct avsync_device
{
	struct avsync_device  *next;   //!< Next device 
	/*struct module *owner;*/
	INT32 type;
	INT8 name[32];
	INT32 flags;

	void *priv;		            /* Used to be 'private' but that upsets C++ */
	void      (*attach)(void *);
	void      (*detach)(struct avsync_device **);
	RET_CODE	(*open)(struct avsync_device *);
	RET_CODE   	(*close)(struct avsync_device *);
	RET_CODE	(*start)(struct avsync_device *);
	RET_CODE   	(*stop)(struct avsync_device *);
	RET_CODE   	(*ioctl)(struct avsync_device *, UINT32 , UINT32);	
	RET_CODE     (*reset)(struct avsync_device*);
	RET_CODE     (*set_syncmode)(struct avsync_device*, AVSYNC_MODE_E);
	RET_CODE     (*get_syncmode)(struct avsync_device*, AVSYNC_MODE_E *);
	RET_CODE     (*set_sourcetype)(struct avsync_device*, AVSYNC_SRCTYPE_E);
	RET_CODE     (*get_sourcetype)(struct avsync_device*, AVSYNC_SRCTYPE_E *);
	RET_CODE     (*config_params)(struct avsync_device*, avsync_cfg_param_t *);
	RET_CODE     (*get_params)(struct avsync_device*, avsync_cfg_param_t *);
	RET_CODE     (*config_adv_params)(struct avsync_device*, avsync_adv_param_t *);
	RET_CODE     (*get_adv_params)(struct avsync_device*, avsync_adv_param_t *);
	RET_CODE     (*get_status)(struct avsync_device*, avsync_cfg_param_t *);
	RET_CODE     (*get_statistics)(struct avsync_device*, avsync_cfg_param_t *);
	RET_CODE     (*video_smoothly_play_onoff)(struct avsync_device *, UINT8, AVSYNC_VIDEO_SMOOTH_LEVEL, UINT8);
	RET_CODE		(*dbg_set_print_option)(struct avsync_device *, UINT32);
	RET_CODE		(*dbg_poll_onoff)(struct avsync_device *, UINT32);
	RET_CODE		(*dbg_set_poll_option)(struct avsync_device *, UINT32);
	RET_CODE		(*dbg_set_poll_interval)(struct avsync_device *, UINT32);
};

typedef RET_CODE (*AVSYNC_GET_VIDEO_CTRLLK_CB)(struct avsync_device *, UINT32 , struct control_block *);
typedef RET_CODE (*AVSYNC_APTS_EXTRACT_CB)(struct avsync_device *, UINT32 , struct control_block *);
typedef RET_CODE (*AVSYNC_PROCESS_PCR_CB)(struct avsync_device *, UINT32);


/*! @ struct avsync_dmx_cb
@brief  A callback function, for registering to DMX.
*/
struct avsync_dmx_cb
{
    void *avsync_dev;		                       //!<Video control block callback function. 
	AVSYNC_GET_VIDEO_CTRLLK_CB pvpts_cb; //!< Video control block callback fucntion
	AVSYNC_APTS_EXTRACT_CB papts_cb;           //!<  Audio control block callback function. 
	AVSYNC_PROCESS_PCR_CB ppcr_cb;            //!<  PCR callback function. 
};

typedef RET_CODE (*AVSYNC_GET_VSYNC_FLAG)(void *dev, struct avsync_vsync_info_t *, AVSYNC_FRAME_SYNCFLAG_E *, BOOL *);
typedef RET_CODE (*AVSYNC_VBV_BUFF_FULL)(void *dev, UINT8);
typedef RET_CODE (*AVSYNC_START_CB)(void *dev);
typedef RET_CODE (*AVSYNC_STOP_CB)(void *dev);
typedef RET_CODE (*AVSYNC_GET_ASYNC_FLAG)(void *dev, struct avsync_async_info_t *, AVSYNC_FRAME_SYNCFLAG_E *);
typedef RET_CODE(*AVSYNC_GET_PAUSE_FLAG)(void*dev, UINT32 *, UINT32 *);


/*! @ struct avsync_callback
@brief   Callback function of AVSYNC module
*/

struct avsync_callback
{
	struct avsync_device *pavsync_dev;                    //!< AVSYNC device pointer
	AVSYNC_GET_VSYNC_FLAG pget_vync_flg_cb;              //!<Video synchronization callback function
	AVSYNC_VBV_BUFF_FULL pvbv_full_cb;                  //!<Video buffer overfull callback function
	AVSYNC_START_CB pavsync_start;                     //!< Start AVSYNC callbck function
	AVSYNC_STOP_CB pavsync_stop;	                    //!< Stop AVSYNC callbck function
	AVSYNC_GET_ASYNC_FLAG pget_async_flg_cb;	       //!<Callback function of audio synchronization
	AVSYNC_GET_PAUSE_FLAG pget_avsync_pause_flag;   //!< Pause audio callback function
};

/*!
@brief Register AVSYNC device and allocate resource.
@note: This function must be called first before calling other interfaces of AVSYNC module.
*/
RET_CODE avsync_attach(void);

/*!
@brief Remove AVSYNC device and destroy resource.
@param[in] dev  Pointer to AVSYNC device. 
*/
void avsync_dettach(struct avsync_device*dev);

/*!
@brief Open AVSYNC module.
@param[in] dev Pointer to AVSYNC device.
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/
RET_CODE avsync_open(struct avsync_device *dev);

/*!
@brief Close AVSYNC module.
@param[in] dev Pointer to AVSYNC device.
@return RET_CODE
@retval  RET_SUCCESS       Operation succeeded.
@retval  !RET_SUCCESS     Operation failed, with parameter error or status error.
*/
RET_CODE avsync_close(struct avsync_device *dev);
/*!
@brief  Start AVSYNC module. 
@param[in] dev  Pointer to AVSYNC device. 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/

RET_CODE avsync_start(struct avsync_device *dev);

/*!
@brief Stop AVSYNC module. 
@param[in] dev  Pointer to AVSYNC device. 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/

RET_CODE avsync_stop(struct avsync_device *dev);

/*!
@brief  ioctl function interface provided by AVSYNC device. 
@param[in] dev  Pointer to AVSYNC device. 
@param[in] io_code   Command type 
@param[in] param   Command parameter 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/
RET_CODE avsync_ioctl(struct avsync_device *dev, UINT32 io_code, UINT32 param);

/*!
@brief  Reset synchronization mode and re-synchronization.
@param[in] dev  Pointer to AVSYNC device. 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/RET_CODE avsync_reset(struct avsync_device *dev);

/*!
@brief Set synchronization mode.
@param[in] dev  Pointer to AVSYNC device. 
@param[in] mode  Synchronization mode, please refer to #AVSYNC_MODE_E definition.
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
@note: If this interface is not called by application level, default configuration is used in AVSYNC module internal. Please refer to #AVSYNC_MODE_E definition.
*/
RET_CODE avsync_set_syncmode(struct avsync_device *dev, AVSYNC_MODE_E mode);

/*!
@brief Get synchronization mode.
@param[in] dev  Pointer to AVSYNC device. 
@param[out] pmode  Synchronization mode
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/
RET_CODE avsync_get_syncmode(struct avsync_device *dev, AVSYNC_MODE_E *pmode);

/*!
@brief  Set data source type.
@param[in] dev  Pointer to AVSYNC device. 
@param[in] type   Data source type, please refer to #AVSYNC_SRCTYPE_E definition.
@return RET_CODE
@@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
@note: If this interface is not called by application level, default configuration is used in AVSYNC module internal. Please refer to #AVSYNC_SRCTYPE_E definition.
*/
RET_CODE avsync_set_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E type);

/*!
@brief Get data source type. 
@param[in] dev  Pointer to AVSYNC device. 
@param[out] ptype   Data source type  
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/
RET_CODE avsync_get_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E *ptype);

/*!
@brief Set basic synchronization parameters. 
@param[in] dev  Pointer to AVSYNC device. 
@param[in] pcfg_params Basic synchronization configuration. 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
@note:  If this interface is not called by application level, default configuration is used in AVSYNC module internal. Please refer to #avsync_cfg_param_t definition. 
*/
RET_CODE avsync_config_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params);

/*!
@brief  Get basic synchronization parameters. 
@param[in] dev  Pointer to AVSYNC device. 
@param[out] pcfg_params Basic synchronization configuration. 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/
RET_CODE avsync_get_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params);

/*!
@brief  Set advanced synchronization parameters. 
@param[in] dev  Pointer to AVSYNC device. 
@param[in] pcfg_params  Advanced synchronization configuration. 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
@note: If this interface is not called by application level, default configuration is used in AVSYNC module internal. Please refer to #avsync_adv_param_t definition. 
*/
RET_CODE avsync_config_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params);

/*!
@brief Get advanced synchronization parameters. 
@param[in] dev  Pointer to AVSYNC device. 
@param[out] pcfg_params   Advanced synchronization parameters. 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/
RET_CODE avsync_get_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params);

/*!
@brief Get mode status. 
@param[in] dev  Pointer to AVSYNC device. 
@param[out] pstatus    Mode status. 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/
RET_CODE avsync_get_status(struct avsync_device *dev, avsync_status_t *pstatus);

/*!
@brief  Get synchronization statistical information. 
@param[in] dev  Pointer to AVSYNC device. 
@param[out] pstatistics  Synchronization statistical information.  
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/
RET_CODE avsync_get_statistics(struct avsync_device *dev, avsync_statistics_t *pstatistics);

/*!
@brief Switch on/off the function of video smooth play.  
@param[in] dev  Pointer to AVSYNC device. 
@param[in] onoff   On/off video smooth playback function.  
@param[in] level  Smooth play level.@~ 
@param[in] interval  Smooth play interval (unit in frame numbers).  
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS       Operation failed, with parameter error or status error. 
*/
RET_CODE avsync_video_smoothly_play_onoff(struct avsync_device *dev, UINT8 onoff, AVSYNC_VIDEO_SMOOTH_LEVEL level, UINT8 interval);

/*==========================================
 * AVSYNC Debug Start
 *==========================================*/
 
//  Real print options@~
#define AVSYNC_DBG_PRINT_DEFAULT 1                                    //!<  Print STC adjustment and audio/vdieo unsynchronization information. 
#define AVSYNC_DBG_PRINT_PCR	2                                      //!< Print PCR and PCR interval. 
#define AVSYNC_DBG_PRINT_APTS	4                                     //!< Print audio PTS and PTS interval. 
#define AVSYNC_DBG_PRINT_VPTS 8                                    //!<  Print video PTS and PTS interval. 
#define AVSYNC_DBG_PRINT_A_SYNC 0x10                              //!< Print PTS/STC value when audio frames are synchronous. 
#define AVSYNC_DBG_PRINT_A_UNSYNC_PTS_STC 0x20                   //!< Print PTS/STC value when audio frames are unsynchronous. 
#define AVSYNC_DBG_PRINT_A_UNSYNC_STCID_OFFSET 0x40             //!<  Print STC offsets when audio frames are unsynchronous. 
#define AVSYNC_DBG_PRINT_A_UNSYNC_ESBUF 0x80                   //!<  Print avaliable ES buffer length when audio frames are unsynchronous. 
#define AVSYNC_DBG_PRINT_V_SYNC 0x100                         //!< Print PTS/STC value when video frames are synchronous. 
#define AVSYNC_DBG_PRINT_V_UNSYNC_PTS_STC 0x200              //!< Print PTS/STC value when video frames are unsynchronous.  
#define AVSYNC_DBG_PRINT_V_UNSYNC_STCID_OFFSET 0x400        //!< Print STC offsets when video frames are unsynchronous. 
#define AVSYNC_DBG_PRINT_V_UNSYNC_VBVBUF 0x800             //!< Print avaliable VBV buffer length when video frames are unsynchronous. 
#define AVSYNC_DBG_PRINT_PTS_OFFSET  0x1000               //!< Print PTS offsets adjustment information. 
#define AVSYNC_DBG_PRINT_STREAM_LOOP 0x2000              //!< Print when detecting streams run back. 
#define AVSYNC_DBG_PRINT_VESBUF_OF_UR 0x4000            //!< Print video when ES buffer void or full. 
#define AVSYNC_DBG_PRINT_AESBUF_OF_UR 0x8000           //!< Print when audio ES buffer void or full. 

#define AVSYNC_DBG_FORCE_SYNCMODE_FREERUN 0x10000     //!< Force synchronization mode to AVSYNC_MODE_AV_FREERUNR; 
#define AVSYNC_DBG_FORCE_SYNCMODE_PCR 0x20000        //!<  Force synchronization mode to AAVSYNC_MODE_AV_PCR; 
#define AVSYNC_DBG_FORCE_SYNCMODE_AUDIO 0x40000     //!<  Force synchronization mode to AVSYNC_MODE_AUDIO; 
#define AVSYNC_DBG_FORCE_SYNCMODE_INVALID 0x80000  //!<  Debugging synchronization mode invalid 

#define AVSYNC_DBG_PRINT_API	0x10000000         //!<  Print API call 
#define AVSYNC_DBG_PRINT_LOG	0x20000000	      //!<  Print LOG 
#define AVSYNC_DBG_PRINT_ERR	0x40000000       //!<  Print module error information 

//
#define AVSYNC_DBG_POLL_V_SYNC_STATISTICS  0x01	 //!<  Open video statistical information 
#define AVSYNC_DBG_POLL_A_SYNC_STATISTICS  0x02  //!<  Open audio statistical information 
#define AVSYNC_DBG_POLL_PCR_STATISTICS  0x04//!<  Open PCR statistical information 
#define AVSYNC_DBG_POLL_VBV_FULL_CNT  0x08 //!< video buffer overflow numbers 
#define AVSYNC_DBG_POLL_PTS_OFFSET  0x10	//!< PTS offset statistical information 
#define AVSYNC_DBG_POLL_SYNC_MODE 0x20	//!< Synchronization mode information 


/*!
@brief  Set real debugging print options. 
@param[in] dev  Pointer to AVSYNC device. 
@param[out] option Print options 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS      Operation failed.
*/
RET_CODE avsync_dbg_set_print_option(struct avsync_device *dev, UINT32 option);

/*!
@brief Switch on/off polling debugging.
@param[in] dev  Pointer to AVSYNC device. 
@param[out] on_off  On/off polling debugging.
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS      Operation failed.
*/
RET_CODE avsync_dbg_polling_onoff(struct avsync_device *dev, UINT32 on_off);

/*!
@brief Set polling debugging options.
@param[in] dev  Pointer to AVSYNC device. 
@param[out] option  Polling debugging options.
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS      Operation failed.
*/
RET_CODE avsync_dbg_set_polling_option(struct avsync_device *dev, UINT32 option);

/*!
@brief Set polling debugging print interval. 
@param[in] dev  Pointer to AVSYNC device. 
@param[out] interval_ms   Polling debugging print interval, unit in ms. 
@return RET_CODE
@retval  RET_SUCCESS        Operation succeeded.
@retval  !RET_SUCCESS      Operation failed.
*/
RET_CODE avsync_dbg_set_polling_interval(struct avsync_device *dev, UINT32 interval_ms);


#ifdef __cplusplus
}
#endif

/*!
@}
*/

/*!
@}
*/

#endif


