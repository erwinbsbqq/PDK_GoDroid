#ifndef __DRIVERS_ALI_RPC_HLD_AVSYNC_H
#define __DRIVERS_ALI_RPC_HLD_AVSYNC_H

#include "ali_rpc_hld.h"

#define AVSYNC_IO_ENABLE_DDP_CERTIFICATION 0x01
#define AVSYNC_IO_GET_VIDEO_SMOOTHLY_PLAY_CFG   0x02
#define AVSYNC_IO_ENABLE_GET_STC 0x03
#define AVSYNC_IO_SET_PTS_UNIT_HZ   0x04
#define AVSYNC_IO_UNREG_CALLBACK   0x05
#define AVSYNC_IO_REG_CALLBACK   0x06
#define AVSYNC_IO_GET_CURRENT_PLAY_PTS   0x07

enum LLD_AVSYNC_FUNC{
    FUNC_AVSYNC_ATTACH = 0,
};
 
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


typedef enum 
{
	AVSYNC_MODE_PCR, //synchronize to PCR(only for live stream)
	AVSYNC_MODE_AUDIO, //synchronize to APTS
	AVSYNC_MODE_VIDEO, //synchronize to VPTS
	AVSYNC_MODE_FREERUN, // not do sync
}AVSYNC_MODE_E;

typedef enum 
{
	AVSYNC_SRC_TURNER, // source data from turner
	AVSYNC_SRC_TMS, //source data from timeshift
	AVSYNC_SRC_HDD_PLAYBACK, //source data  from HDD playback
	AVSYNC_SRC_UDP, //source data from UDP
	AVSYNC_SRC_TCP, //source data from TCP
}AVSYNC_SRCTYPE_E;

typedef enum 
{
	AVSYNC_ATTACHED, //!<设备已挂接
	AVSYNC_OPENED,    //!<设备已打开
	AVSYNC_CLOSED,    //!<设备已关闭
	AVSYNC_STARTED,  //!<设备已开始工作
	AVSYNC_STOPPED,	//!<设备已停止工作	
}AVSYNC_DEV_STATUS;

typedef enum 
{
	AVSYNC_VIDEO_SMOOTH_LEVEL1, 
	AVSYNC_VIDEO_SMOOTH_LEVEL2, 
}AVSYNC_VIDEO_SMOOTH_LEVEL;

typedef struct
{
	UINT32 vhold_thres; //video hold threshold
	UINT32 vdrop_thres; // video drop threshold
	UINT32 ahold_thres; //audio hold threshold
	UINT32 adrop_thres; //audio hold threshold	
	AVSYNC_MODE_E sync_mode; // av sync mode
	AVSYNC_SRCTYPE_E src_type; // source data type
}avsync_cfg_param_t;

typedef struct
{
	UINT32 afreerun_thres; // audio free run threshold
	UINT32 vfreerun_thres; // video free run threshold
	UINT8  disable_monitor; // disable avsync monitor 
	UINT8  disable_first_video_freerun; // disable first video frame freerun,show frame until sync
	UINT16  dual_output_sd_delay; // video delay in sd path
	UINT32 rsvd1; 
	UINT32 rsvd2; 
	UINT32 rsvd3;
}avsync_adv_param_t;

typedef struct 
{
	AVSYNC_DEV_STATUS device_status; //!<设备状态
	UINT32 vpts_offset; //!<视频PTS偏移
	UINT32 apts_offset; //!<音频PTS偏移	
	UINT8 v_sync_flg; //!<视频同步标志
	UINT8 a_sync_flg; //!<音频同步标志
	UINT8 rsvd0; //!<预留
	UINT8 rsvd1; //!<预留

	UINT32 cur_vpts;//!< 当前视频PTS
	UINT32 cur_apts;//!< 当前音频PTS		
}avsync_status_t;

typedef struct 
{
	UINT32 total_v_play_cnt;
	UINT32 total_v_drop_cnt;
	UINT32 total_v_hold_cnt;
	UINT32 total_v_freerun_cnt;
	UINT32 total_a_play_cnt;
	UINT32 total_a_drop_cnt;
	UINT32 total_a_hold_cnt;
	UINT32 total_a_freerun_cnt;
}avsync_statistics_t;

typedef struct  
{
	UINT8 onoff;
	UINT8 interval;
	AVSYNC_VIDEO_SMOOTH_LEVEL level; 
}avsync_smoothly_play_cfg_t;

typedef struct 
{
	void *sharem_addr;
	UINT8 enable;
}avsync_get_stc_en_t;

struct avsync_device
{
	struct avsync_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[32];
	INT32 flags;

	void *priv;		/* Used to be 'private' but that upsets C++ */
	void      (*attach)(void);
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
	RET_CODE		(*dbg_set_print_option)(UINT32);
};

RET_CODE avsync_attach(void);

/*
	open avsync device
  */
RET_CODE avsync_open(struct avsync_device *dev);

/*
	close avsync device
  */
RET_CODE avsync_close(struct avsync_device *dev);

/*
	start do audio/video sync
  */
RET_CODE avsync_start(struct avsync_device *dev);

/*
	start do audio/video sync
  */
RET_CODE avsync_stop(struct avsync_device *dev);

RET_CODE avsync_ioctl(struct avsync_device *dev, UINT32 io_code, UINT32 param);

/*
   reset avsync module, all sync information will be cleared
*/
RET_CODE avsync_reset(struct avsync_device *dev);

/*
  set sync mode
*/
RET_CODE avsync_set_syncmode(struct avsync_device *dev, AVSYNC_MODE_E mode);

/*
  get sync mode
*/
RET_CODE avsync_get_syncmode(struct avsync_device *dev, AVSYNC_MODE_E *pmode);

/*
  configure basic sync parameters
*/
RET_CODE avsync_config_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params);

/*
  get basic sync parameters
*/
RET_CODE avsync_get_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params);

/*
	configure advance sync parameters
*/
RET_CODE avsync_config_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params);

/*
	get advance sync parameters
*/
RET_CODE avsync_get_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params);

/*
  set source data type
*/
RET_CODE avsync_set_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E type);

/*
  get source data type
*/
RET_CODE avsync_get_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E *ptype);

/*
  get source data type
*/
RET_CODE avsync_get_status(struct avsync_device *dev, avsync_status_t *pstatus);

/*
  get statistics infomation
*/
RET_CODE avsync_get_statistics(struct avsync_device *dev, avsync_statistics_t *pstatistics);

RET_CODE avsync_video_smoothly_play_onoff(struct avsync_device *dev, UINT8 onoff, AVSYNC_VIDEO_SMOOTH_LEVEL level, UINT8 interval);

/*==========================================
 *  AVSYNC Debug Start
 *==========================================*/
#define AVSYNC_DBG_PRINT_DEFAULT 1 //include STC adjust and av unsync info
#define AVSYNC_DBG_PRINT_PCR	2  // trace audio PCR
#define AVSYNC_DBG_PRINT_APTS	4  // trace audio PTS
#define AVSYNC_DBG_PRINT_VPTS 8  // trace video PTS
#define AVSYNC_DBG_PRINT_A_SYNC 0x10 //printf audio sync info
#define AVSYNC_DBG_PRINT_A_UNSYNC_PTS_STC 0x20 // printf audio sync info
#define AVSYNC_DBG_PRINT_A_UNSYNC_STCID_OFFSET 0x40
#define AVSYNC_DBG_PRINT_V_SYNC 0x80
#define AVSYNC_DBG_PRINT_V_UNSYNC_PTS_STC 0x100
#define AVSYNC_DBG_PRINT_V_UNSYNC_STCID_OFFSET 0x200
#define AVSYNC_DBG_PRINT_V_UNSYNC_VBVBUF 0x400
#define AVSYNC_DBG_PRINT_PTS_OFFSET  0x800
#define AVSYNC_DBG_PRINT_STREAM_LOOP 0x1000

#define AVSYNC_DBG_PRINT_VESBUF_OF_UR 0x2000
#define AVSYNC_DBG_PRINT_AESBUF_OF_UR 0x4000

#define AVSYNC_DBG_PRINT_API	0x10000000
#define AVSYNC_DBG_PRINT_LOG	0x20000000
#define AVSYNC_DBG_PRINT_ERR	0x40000000

#define AVSYNC_DBG_POLL_V_SYNC_CONTROL_FLAG_CNT  0x01
#define AVSYNC_DBG_POLL_A_SYNC_CONTROL_FLAG_CNT  0x02
#define AVSYNC_DBG_POLL_PCR_CNT  0x04
#define AVSYNC_DBG_POLL_VBV_FULL_CNT  0x08
#define AVSYNC_DBG_POLL_PTS_OFFSET  0x10
#define AVSYNC_DBG_POLL_SYNC_MODE 0x20

RET_CODE avsync_dbg_set_print_option(struct avsync_device *dev, UINT32 option);
RET_CODE avsync_dbg_polling_onoff(struct avsync_device *dev, UINT32 on_off);
RET_CODE avsync_dbg_set_polling_option(struct avsync_device *dev, UINT32 option);
RET_CODE avsync_dbg_set_polling_interval(struct avsync_device *dev, UINT32 interval);

/*==========================================
 *  AVSYNC Debug End
 *==========================================*/
#endif
