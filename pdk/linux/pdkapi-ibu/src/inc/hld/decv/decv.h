/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: decv.h
 *
 *  Description: head file for video decoder device management.
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2005.10.21  Rachel		     0.N.000   Support VDEC27_SUPPORT_HW_COMSUME_RUBBISH
 *  2.  2006.5.16    Rachel				   	Add IO control to support return several free buffer instead of only one label"rachel:support_return_multi_freebuffer"
 ****************************************************************************/
#ifndef  _DECV_H_
#define  _DECV_H_

#include <basic_types.h>
#include <mediatypes.h>

#include <sys_config.h>
#include <alidefinition/adf_decv.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <rpc_hld/ali_rpc_hld_decv.h>

#if 0
//#define TEST_VE_TIME_INFO
#define TEST_TOTAL_TIME (5)


//8/17
#ifdef DGS_AVSYNC_METHOD	//only for the project of dongqi
#define	VDEC27_LM_SHIFTTHRESHOLD			4	
#else
#define	VDEC27_LM_SHIFTTHRESHOLD			2	
#endif

#define	VDEC27_SDRAM_SIZE		SYS_SDRAM_SIZE

//#define	VDEC27_SUPPORT_DROPPIC_AFTER_ERROR
#if (VDEC27_SDRAM_SIZE != 2)
#define	VDEC27_SUPPORT_COLORBAR
#endif

#if(SYS_PROJECT_FE != PROJECT_FE_DVBT || VDEC27_SDRAM_SIZE != 2)
#define VDEC27_SUPPORT_SWSCALE_FILLBLACK
#endif


#define	VDEC27_DVIEW_MAP_2M			0
#define	VDEC27_DVIEW_MAP_N2M		1
#if( VDEC27_SDRAM_SIZE == 2)
#define	VDEC27_DVIEW_MAP				VDEC27_DVIEW_MAP_2M
#define	VDEC27_DVIEW_V_MP_WIDTH				304

#define	VDEC27_DVIEW_HW_SCALE_H			176
#define	VDEC27_DVIEW_HW_SCALE_V			144
#else
#define	VDEC27_DVIEW_MAP				VDEC27_DVIEW_MAP_N2M
#define	VDEC27_DVIEW_V_MP_WIDTH				608
#define	VDEC27_DVIEW_HW_SCALE_H				176
#define	VDEC27_DVIEW_HW_SCALE_V				144
#endif

#define	VDEC27_DVIEW_V_MP_HEIGHT			288

//3module config

#define VDEC27_SML_FRM_ON   0x01
#define VDEC27_SML_FRM_OFF   0x02

#define VDEC27_H_PRECISION_2    (0x02<<16)
#define VDEC27_H_PRECISION_4    (0x04<<16)
#define VDEC27_H_PRECISION_8    (0x08<<16)

#define VDEC27_V_PRECISION_2    0x02
#define VDEC27_V_PRECISION_4    0x04
#define VDEC27_V_PRECISION_8    0x08


#define VDEC27_DVIEW_OPT_2   2
#define VDEC27_DVIEW_OPT_4   4
#define VDEC27_DVIEW_OPT_8   8
#define VDEC27_DVIEW_OPT_H2_V4  12
#define VDEC27_DVIEW_OPT_H4_V2  13

#define	VDEC27_PREVIEW_VE_SCALE	0x01
#define	VDEC27_PREVIEW_DE_SCALE	0x02


#if( VDEC27_SDRAM_SIZE >= 4)
#define	VDEC27_SUPPORT_RETURN_MULTI_FREEBUF
#endif

#ifdef DVR_PVR_SUPPORT
#define	VDEC_ADVANCE_PLAY_MODE
#endif



#if(SYS_PROJECT_FE != PROJECT_FE_DVBT || VDEC27_SDRAM_SIZE != 2)
#define	VDEC27_STOP_REST_HW_DIRECTLY
#endif

#ifndef __ATTRIBUTE_REUSE_
#define	__ATTRIBUTE_REUSE_ __attribute__((section(".reuse")))
#endif

#ifdef SYS_ADJUST_PREVIEW_ACCORD_SRC_RATE
//pelease open this macrodefinition for M3327+2M project+120M memory clock to remove the tear at the bottom of preview window when P->N or P->P 
#define	VDEC27_ADJUST_PREVIEW_ACCORD_SRC_RATE
#endif

#ifdef	HDT_M3329_MODEL1
#define	VDEC27_DROP_FREERUN_BEFORE_FIRSTSHOW
#endif
#define VDEC27_DROP_THRESHOLD 20


#define VDEC27_SUPPORT_HW_COMSUME_RUBBISH
#ifdef VDEC27_SUPPORT_HW_COMSUME_RUBBISH
#define	VDEC27_HEADER_FL_MAX	1
#define	VDEC27_LITTERBIN_ADDR	((__MM_MAF_START_ADDR+0x400-0xFF)&0xFFFFFFF0)
#endif
 
#define VIDEO_ADPCM_ONOFF           VIDEO_ADPCM_OFF   
#define VDEC27_SML_FRM_ONOFF        VDEC27_SML_FRM_OFF
#define	VDEC27_SML_FRM_SIZE		0
#define VDEC27_DVIEW_PRECISION      (VDEC27_H_PRECISION_4|VDEC27_V_PRECISION_4)
//#define VDEC27_DVIEW_ADJUST_2
//#define	VDEC27_USE_RE_ADPCM
#define VDEC27_PREVIEW_PRECISION    (VDEC27_H_PRECISION_2|VDEC27_V_PRECISION_2)
#if 1//((defined HDTV_SUPPORT) || ((SYS_CHIP_MODULE == ALI_S3602) && (SYS_CPU_MODULE == CPU_M6303) && (SYS_PROJECT_FE == PROJECT_FE_DVBT)))//for 3105
#define	VDEC27_PREVIEW_SOLUTION	VDEC27_PREVIEW_VE_SCALE
#else
#define	VDEC27_PREVIEW_SOLUTION	VDEC27_PREVIEW_DE_SCALE
#endif

//SDK config parameter
#define VDEC_VALID_USR_DATA 				 (1<<0)
#define VDEC_VALID_DROP_FIRST_FREE_PIC 	 (1<<1)
#define VDEC_VALID_RESET_HW_D_WHEN_STOP 	 (1<<2)
#define VDEC_VALID_SHOW_HD_SERVICE		 (1<<3)
#define VDEC_VALID_STILL_FRM_IN_CC          	 (1<<4)
#define VDEC_VALID_EXTRA_DV_WINDOW             (1<<5)
#define VDEC_VALID_NOT_SHOW_MOSAIC		 (1<<6)
#define VDEC_VALID_ADPCM					 (1<<7)
#define VDEC_VALID_SML_FRM 					 (1<<8)
#define VDEC_VALID_LM_SHIFTTHRESHOLD				 (1<<9)
#define VDEC_VALID_VP_INIT_PHASE					 (1<<10)
#define VDEC_VALID_PREV_SOLUTION					 (1<<11)
#define VDEC_VALID_MEM_ADDR						 (1<<12)

//num of features
#define VDEC_MAX_NUM_OF_FEATURES			  (13)

//3API parameter

//4vdec_m3327_get_frm
#define 	VDEC_DISPLAY		0x01
#define	VDEC_UN_DISPLAY	0x02
#define   VDEC_FRM_REF		0x03

//4vdec_set_sync_level
#define	VDEC_SYNC_FIRSTFREERUN	0x01
#define	VDEC_SYNC_I				0x02
#define	VDEC_SYNC_P				0x04
#define	VDEC_SYNC_B				0x08
#define VDEC_SYNC_HIGH_LEVEL    0x10

#define VIDEO_OPEN_PIP_WIN_FLAG   0X80 

//changed for s3601-6/5
//4vdec_set_profile_level
#define	SP_ML				0x58
#define	MP_LL				0x4A
#define	MP_ML				0x48
#define	MP_H14				0x46
#define	MP_HL				0x44	
#define	SNR_LL				0x3A
#define	SNR_ML				0x38
#define	Spatial_H14			0x26
#define	HP_ML				0x18
#define	HP_H14				0x16
#define	HP_HL				0x14
//- changed end

#define	VDEC_DETACH		10
#define	VDEC_CLOSED		11
#define	VDEC_DECODING		VDEC27_STARTED
#define	VDEC_REVERSING	12
#define	VDEC_PAUSED		VDEC27_PAUSED
#define	VDEC_STOPPED		VDEC27_STOPPED	

enum VDECDecoderMode
{
	VDEC_FULL_MODE,
	VDEC_QUICK_MODE
};

//4Register_Callback
typedef void (*VDecCBFunc)(UINT32 uParam1, UINT32 uParam2);
enum VDecCBType
{
	VDEC_CB_SETTING_CHG = 0,
	VDEC_CB_REQ_DATA,
	VDEC_CB_STOP,
	VDEC_CB_FINISH_CUR_PIC,
	VDEC_CB_FINISH_I_FRAME,
	VDEC_CB_FINISH_TARGET_FRAME,
	VDEC_CB_FIRST_SHOWED,
	VDEC_CB_MODE_SWITCH_OK,
	VDEC_CB_BACKWARD_RESTART_GOP,
	VDEC_CB_OUTPUT_MODE_CHECK,
	VDEC_CB_FIRST_HEAD_PARSED,
	VDEC_CB_MONITOR_FRAME_VBV,
    VDEC_CB_FIRST_I_DECODED,
};

struct vdec_callback
{
    VDecCBFunc	pcb_first_showed;
	VDecCBFunc	pcb_mode_switch_ok;
    VDecCBFunc	pcb_backward_restart_gop;
    VDecCBFunc  pcb_first_head_parsed;
    VDecCBFunc  pcb_first_i_decoded;
};

struct vdec_frm_output_format
{
	// VE config
	BOOL h_scale_enable;
	UINT32 h_scale_factor;//0:reserved, 1: Y h_scale only, 2: Y,C h_scale

	BOOL dview_enable;
	UINT32 dv_h_scale_factor;//0:no dview, 1: 1/2 dview, 2: 1/4 dview, 3: 1/8 dview
	UINT32 dv_v_scale_factor;//0:no dview, 1: 1/2 dview, 2: 1/4 dview, 3: 1/8 dview
	
	UINT32 dv_mode; 

	//DE config
	UINT32 field_src;//0: both fields, 1:top only field
	UINT32 scaler_src;//0: frame base, 1: field base
	UINT32 vpp_effort;//0:high, 1: middle, 2: low, 3:very low
};


//4Get_frm
struct vdec_frm_info_api
{
	UINT32 uY_Addr;
	UINT32 uC_Addr;
	UINT16 uWidth;
	UINT16 uHeight;
	UINT32 uY_Size;//rachel:support_return_multi_freebuffer
	UINT32 uC_Size;//rachel:support_return_multi_freebuffer
};


//4Set_Output	to be removed
/*
typedef enum Output_Frame_Ret_Code (* VDecMPRequest)(struct Display_Info *);
typedef BOOL (* VDecMPRelease)(UINT8, UINT8 );
typedef enum Output_Frame_Ret_Code (* VDecPIPRequest)(struct PIP_Dislay_Info *);
typedef BOOL (* VDecPIPRelease)(UINT8);

struct VDec_MP_CallBack
{
	VDecMPRequest	RequestCallback;
	VDecMPRelease	ReleaseCallback;
};

struct VDec_PIP_CallBack
{
	VDecPIPRequest	RequestCallback;
	VDecPIPRelease	ReleaseCallback;
};
*/

#define	VDec_MP_CallBack MPSource_CallBack		//for compatible with application that had used the VDec_MP_Callback
#define	VDec_PIP_CallBack PIPSource_CallBack

typedef void (* VDec_PullDown_Func)(UINT32);
struct VDec_PullDown_Opr
{
	VDec_PullDown_Func	OpenDeinterlace;
	VDec_PullDown_Func	CloseDeinterlace;
};
struct vdec_start_para
{
	enum VDecOutputMode eMode;
	BOOL hd_support;
	void* reserved;
};

 typedef void (* VDEC_BEYOND_LEVEL)();
 

struct vdec_mem_map
{
	//frm addr
	UINT32	frm0_Y_size;
	UINT32	frm0_C_size;
	UINT32	frm1_Y_size;
	UINT32	frm1_C_size;
	UINT32	frm2_Y_size;
	UINT32	frm2_C_size;
	
	UINT32	frm0_Y_start_addr;
	UINT32	frm0_C_start_addr;
	UINT32	frm1_Y_start_addr;
	UINT32	frm1_C_start_addr;
	UINT32	frm2_Y_start_addr;
	UINT32	frm2_C_start_addr;

	UINT32	dvw_frm_size;
	UINT32	dvw_frm_start_addr;

	//MAF addr
	UINT32	maf_size;
	UINT32	maf_start_addr;

	//VBV addr
	UINT32	vbv_size;
	UINT32	vbv_start_addr;
	UINT32	vbv_end_addr;

	//frame buffer 3 allocated for pip application
	UINT32	frm3_Y_size;
	UINT32 	frm3_C_size;
	UINT32 	frm3_Y_start_addr;
	UINT32 	frm3_C_start_addr;

	UINT32   frm_num;//3 // 3 or 4 frames can be choosed for the main device. It is unactive for the second device
	UINT32   res_bits;
	UINT32  *res_pointer;
};

struct vdec_adpcm
{
	UINT8 adpcm_mode;
	UINT8 adpcm_ratio;
};

struct vdec_sml_frm
{
	UINT8 sml_frm_mode;
	UINT32 sml_frm_size;
};

enum vdec_nonshow_mosaic_threshold
{
	MOST_NON_SHOW,
	MORE_NON_SHOW,
	COMMON_NON_SHOW,
	SOME_MOSAIC_SHOW,
	MANY_MOSAIC_SHOW
};
struct vdec_config_par
{	
	//feature entrys
	UINT8 user_data_parsing;
	UINT8 dtg_afd_parsing;
	UINT8 drop_freerun_pic_before_firstpic_show;
	UINT8 reset_hw_directly_when_stop;
	UINT8 show_hd_service;
	UINT8 still_frm_in_cc;//only work in 2M mode
	UINT8 extra_dview_window;
	UINT8 not_show_mosaic;// now's feature just work rightly at Dview & MP mode
	UINT8 return_multi_freebuf;
	UINT8 advance_play;
	
	struct vdec_adpcm adpcm;
	struct vdec_sml_frm sml_frm;
	
	UINT8 lm_shiftthreshold;////only for the project of dongqi( digisat)
	UINT8 vp_init_phase;//according to macro DEFAULT_MP_FILTER_ENABLE
	UINT8 preview_solution;
	
	struct vdec_mem_map mem_map;
	UINT32 res_bits;
	UINT32 *res_pointer;
};


//#ifdef USE_NEW_VDEC
static char decv_m3327_name[] = {"DECV_M3327"};
struct vdec_device
{
	struct vdec_device  *next;  /*next device */
    	UINT32 type;
	INT8 name[32];
	UINT8  flags;

	UINT8 index;
	void *top_info;
	void *priv;		/* Used to be 'private' but that upsets C++ */

	RET_CODE	(*open)(struct vdec_device *);
	RET_CODE   	(*close)(struct vdec_device *);
	RET_CODE   	(*start)(struct vdec_device *);
	RET_CODE   	(*stop)(struct vdec_device *,BOOL,BOOL);
	//RET_CODE   	(*pause)(struct vdec_device *);
	//RET_CODE   	(*resume)(struct vdec_device *);
	RET_CODE 	(*vbv_request)(struct vdec_device *, UINT32, void **, UINT32 *, struct control_block *);
	void 		(*vbv_update)(struct vdec_device *, UINT32);
	RET_CODE 	(*set_output)(struct vdec_device *,  enum VDecOutputMode, struct VDecPIPInfo *, struct MPSource_CallBack *, struct PIPSource_CallBack *);
	RET_CODE 	(*switch_pip)(struct vdec_device *, struct Position *);
	RET_CODE 	(*switch_pip_ext)(struct vdec_device *, struct Rect*);
	RET_CODE 	(*sync_mode)(struct vdec_device *,  UINT8,UINT8);
	RET_CODE   	(*extrawin_store_last_pic)(struct vdec_device *, struct Rect *);
	RET_CODE   	(*ioctl)(struct vdec_device *, UINT32 , UINT32);
	/* for advanced play */
	RET_CODE 	(*playmode)(struct vdec_device *, enum VDecDirection , enum VDecSpeed );
	RET_CODE 	(*step)(struct vdec_device *);
	RET_CODE 	(*dvr_pause)(struct vdec_device *);
	RET_CODE 	(*dvr_resume)(struct vdec_device *);
	RET_CODE 	(*profile_level)(struct vdec_device *,  UINT8,VDEC_BEYOND_LEVEL);	
	RET_CODE 	(*dvr_set_param)(struct vdec_device *, struct VDec_DvrConfigParam );
        /* end */
	/*
	void 		(*sync_level)(struct vdec_device *, UINT8);
	void 		(*fill_all_frm)(struct vdec_device *, struct YCbCrColor *);
	RET_CODE 	(*register_cb)(struct vdec_device *, enum VDecCBType, VDecCBFunc);
	void 		(*get_status)(struct vdec_device *, struct VDec_StatusInfo *);
	void 		(*dbg_why_idle)(struct vdec_device *);
	RET_CODE 	(*get_src_mode)(struct vdec_device *, enum TVSystem *);
	void 		(*get_frm)(struct vdec_device *, UINT8 , struct vdec_frm_info_api *);
	*/
	RET_CODE	(*internal_set_output)(struct vdec_device *,  enum VDecOutputMode, struct VDecPIPInfo *, struct MPSource_CallBack *, struct PIPSource_CallBack *);
	RET_CODE	(*internal_set_frm_output_format)(struct vdec_frm_output_format *);
	void 		(*de_hw_using)(struct vdec_device *, UINT8, UINT8, UINT8);

};

#define VDEC_DEV_MAX	4
struct vdec_control
{
	struct vdec_device *dev[VDEC_DEV_MAX];
	UINT8 cur_idx;
};


/*
RET_CODE decv_request_write_2(struct vdec_device *dev, UINT32 uSizeRequested, 
	                                        void ** ppVData, UINT32 * puSizeGot,
	                                        struct control_block * ctrl_blk);
void decv_update_write_2(struct vdec_device *dev,UINT32 uDataSize);
*/
BOOL vdec_m3327_get_mem_info(UINT32 *pdest);
enum Output_Frame_Ret_Code vdec_m3327_de_mp_request(struct Display_Info *pDisplay_info);
BOOL vdec_m3327_de_mp_release(UINT8 utop_idx,UINT8 frm_array_idx);
enum Output_Frame_Ret_Code vdec_m3327_de_bg_request(struct Display_Info *pDisplay_info);
BOOL vdec_m3327_de_bg_release(UINT8 utop_idx,UINT8 ubot_idx);
enum Output_Frame_Ret_Code vdec_m3327_de_pip_request(struct PIP_Dislay_Info *pDisplay_info);
BOOL vdec_m3327_de_pip_release(UINT8 pip_idx);
enum Output_Frame_Ret_Code vdec_m3327_de_mp_request_ext(void *dev,void *parameter);
BOOL vdec_m3327_de_mp_release_ext(void *dev,UINT8 utop_idx,UINT8 frm_array_idx);
enum Output_Frame_Ret_Code vdec_m3327_de_bg_request_ext(void *dev,void *parameter);
BOOL vdec_m3327_de_bg_release_ext(void *dev,UINT8 uidx,UINT8 ubot_idx);
enum Output_Frame_Ret_Code vdec_m3327_de_pip_request_ext(void *dev,void *parameter);
BOOL vdec_m3327_de_pip_release_ext(void *dev,UINT8 uidx,UINT8 frm_array_idx);
enum Output_Frame_Ret_Code vdec_m3327_de_pip_request_without_extra_dv(struct vdec_device *dev,struct PIP_Dislay_Info *pDisplay_info);
enum Output_Frame_Ret_Code vdec_m3327_de_pip_request_with_extra_dv(struct vdec_device *dev,struct PIP_Dislay_Info *pDisplay_info);

enum Output_Frame_Ret_Code vdec_s3601_de_request(void *dev,struct Display_Info *pDisplay_info,struct Request_Info *pRequest_Info);
INT32 vdec_s3601_de_release(void *dev,struct Release_Info *pRelease_Info);
enum Output_Frame_Ret_Code vdec_m33_de_request(void * dev, struct Display_Info *pDisplay_info);
BOOL vdec_m33_de_release(void * dev, UINT8 param1, UINT8 param2);


struct vdec_io_reg_callback_para
{
	enum VDecCBType eCBType;
	VDecCBFunc pCB;
};

struct vdec_io_get_frm_para
{
	UINT8 ufrm_mode;
	struct vdec_frm_info_api tFrmInfo;
};
//rachel:support_return_multi_freebuffer
#define	VDEC_GET_FRM_MAX_NUM 3
struct vdec_io_get_frm_para_advance
{
	UINT8 ufrm_mode;
	UINT8 request_frm_number;
	struct vdec_frm_info_api tFrmInfo[VDEC_GET_FRM_MAX_NUM];
	UINT8 return_frm_number;
};

#define	VDEC_FREEBUF_USER_DB			0x01
#define	VDEC_FREEBUF_USER_OSD		0x02
#define VDEC_HIGH_PRIORITY_STOP 0x01
#define VDEC_HIGH_PRIORITY_SML  0x02
struct vdec_io_get_freebuf_para
{
	UINT8	request_user_id;
	UINT32	request_size;
	UINT32	got_free_addr;
	UINT32	got_free_size;
};

enum vdec_io_decode_speed
{
	VDEC_SPEED_NORMAL = 0,
	VDEC_SPEED_FORWARD_2,
	VDEC_SPEED_FORWARD_4
};
struct vdec_io_get_frm_statistic
{
	UINT16    conti_drop_cnt;
	UINT16    conti_hold_cnt;
	UINT16    conti_freerun_cnt;
	UINT16	reserved16;
};
//changed for s3601-6/5
struct vdec_adpcm_cmd
{
	BOOL onoff;
	UINT8 ratio;
};
struct vdec_maf_cmd
{
	BOOL onoff;	
};
struct vdec_dview_cmd
{
	BOOL 	adaptive_precision;
    	UINT8   	h_precision;
    	UINT8   	v_precision;
};
struct vdec_err_conceal_cmd
{
	BOOL onoff;
	UINT8 method;
	UINT8 threshold;
	UINT8 next_mb_x;
	UINT8 next_mb_y;
};
struct vdec_abs_cmd
{
	BOOL onoff;
	BOOL adaptive_threshold;
	UINT8 threshold;
};

struct vdec_io_extra_dview_info
{
	BOOL	bonoff;
	struct Rect	extra_dview_win_rect;
	UINT8		smooth_zoom_vector;
	UINT8		step_max_num;	//from 1,....
	
};

//- changed end

void vdec_m3327_attach(void);
void vdec_m3327_dettach(struct vdec_device *dev);

void HLD_vdec_attach(void);
void HLD_vdec_detach(void);

RET_CODE vdec_open(struct vdec_device *dev);
RET_CODE vdec_close(struct vdec_device *dev);
RET_CODE vdec_start(struct vdec_device *dev);
RET_CODE vdec_stop(struct vdec_device *dev,BOOL bclosevp,BOOL bfillblack);
RET_CODE vdec_pause(struct vdec_device *dev);
RET_CODE vdec_resume(struct vdec_device *dev);
RET_CODE vdec_set_output(struct vdec_device *dev, enum VDecOutputMode eMode,struct VDecPIPInfo *pInitInfo, 	struct MPSource_CallBack *pMPCallBack, struct PIPSource_CallBack *pPIPCallBack);
RET_CODE vdec_vbv_request(void *dev, UINT32 uSizeRequested, void ** ppVData, UINT32 * puSizeGot, struct control_block * ctrl_blk);
void vdec_vbv_update(void *dev, UINT32 uDataSize);
RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param);
RET_CODE vdec_switch_pip(struct vdec_device *dev, struct Position *pPIPPos);
RET_CODE vdec_switch_pip_ext(struct vdec_device *dev, struct Rect *pPIPWin);
RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 uSyncMode,UINT8 uSyncLevel);
RET_CODE vdec_extrawin_store_last_pic(struct vdec_device *dev, struct Rect *pPIPWin);
/* for advanced play */
RET_CODE vdec_dvr_pause(struct vdec_device *dev);
RET_CODE vdec_dvr_resume(struct vdec_device *dev);
RET_CODE vdec_playmode(struct vdec_device *dev, enum VDecDirection direction, enum VDecSpeed speed);
RET_CODE vdec_step(struct vdec_device *dev);
RET_CODE vdec_enable_advance_play(struct vdec_device *dev);
 
RET_CODE vdec_profile_level(struct vdec_device *dev, UINT8 uProfileLevel,VDEC_BEYOND_LEVEL cb_beyond_level);
struct vdec_device * get_selected_decoder(void);
void h264_decoder_select(int select, BOOL in_preivew);
BOOL is_cur_decoder_avc(void);
void set_avc_output_mode_check_cb(VDecCBFunc pCB); 
 /* end */
RET_CODE vdec_decore_ioctl(void *phandle, int cmd, void *param1, void *param2);
/*
void decv_sync_level(struct vdec_device *dev, UINT8 uSyncLevel);
void decv_fill_all_frm(struct vdec_device *dev, struct YCbCrColor *pcolor);
RET_CODE decv_register_cb(struct vdec_device *dev, enum VDecCBType eCBType,VDecCBFunc pCB);
void decv_get_status(struct vdec_device *dev, struct VDec_StatusInfo *pCurStatus);
void decv_dbg_why_idle(struct vdec_device *dev);
RET_CODE decv_get_src_mode(struct vdec_device *dev, enum TVSystem *peTVSys);
void decv_get_frm(struct vdec_device *dev, UINT8 ufrm_mode,struct vdec_frm_info_api *pFrmInfo);
*/



//#define	VDec_Attach(x) 		vdec_m3327_attach()
//#define	VDec_Detach(x1)		vdec_m3327_dettach(x1)

//#define	VDec_Open(x1, x2) 	vdec_open(x1)
//#define	VDec_Close(x1)		vdec_close(x1)

//#define	VDec_Decode(x1,x2,x3,x4,x5,x6,x7) vdec_start(x1)
//#define	VDec_Stop(x1,x2,x3,x4,x5) vdec_stop(x1)
//#define	VDec_Pause(x1)		vdec_pause(x1)
//#define	VDec_Resume(x1)		vdec_resume(x1)

//#define	VDec_SetOutput(x1,x2,x3,x4,x5,x6) vdec_set_output(x1,x2,x3,x4)
//#define	VDec_SwitchPIPWin(x1)		vdec_switch_pip(0,x1)

//#define	VDec_Set_Sync_Mode(x1, x2) vdec_sync_mode(x1, x2)
//#define	VDec_Set_Sync_Level(x1, x2) decv_sync_level(x1, x2)

//#define	VDec_FillDisplayFrm(x1,x2) 	decv_fill_all_frm(x1, x2)
//#define	VDec_RegisterCB(x1,x2,x3)	decv_register_cb(x1,x2,x3)
//#define	VDec_GetStatusInfo(x1,x2)	decv_get_status(x1,x2)	
//#define	VDec_Dbg_WhyUIdle()		decv_dbg_why_idle(0)
//#define	VDec_GetSourceMode(x1,x2)	decv_get_src_mode(x1,x2)
//#define	VDec_Get_Frm(x1,x2)			decv_get_frm(0,x1,x2)

//show logo call this function 
//#define	decv_request_write(x1,x2,x3,x4,x5) 		vdec_vbv_request(x1,x2,x3,x4,x5)
//#define	decv_update_write(x1,x2)					vdec_vbv_update(x1,x2)

/*
#define	VDec_SetOutput(x1,x2,x3,x4,x5,x6) vdec_set_output(x1,x2,x3,x4)
#define	VDec_SwitchPIPWin(x1)		vdec_switch_pip(0,x1)

#define	VDec_Set_Sync_Mode(x1, x2) vdec_sync_mode(x1, x2)
#define	VDec_Set_Sync_Level(x1, x2) decv_sync_level(x1, x2)

#define	VDec_FillDisplayFrm(x1,x2) 	decv_fill_all_frm(x1, x2)
#define	VDec_RegisterCB(x1,x2,x3)	decv_register_cb(x1,x2,x3)
#define	VDec_GetStatusInfo(x1,x2)	decv_get_status(x1,x2)	
#define	VDec_Dbg_WhyUIdle()		decv_dbg_why_idle(0)
#define	VDec_GetSourceMode(x1,x2)	decv_get_src_mode(x1,x2)
#define	VDec_Get_Frm(x1,x2)		decv_get_frm(x1,x2)

//show logo call this function 
#define	decv_request_write(x1,x2,x3,x4,x5) 		vdec_vbv_request(x1,x2,x3,x4,x5)
#define	decv_update_write(x1,x2)				vdec_vbv_update(x1,x2)
*/
//#endif

#endif

#endif  /* _DECV_H_*/


