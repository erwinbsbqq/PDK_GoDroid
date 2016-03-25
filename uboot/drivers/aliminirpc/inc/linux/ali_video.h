/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: ali_video.h
 *  (I)
 *  Description: ali video header
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2009.12.22				Sam		Create
 ****************************************************************************/

 #ifndef __ALI_VIDEO_H
#define __ALI_VIDEO_H

#include "ali_basic.h"

/* video output dac information */
#define MAX_VIDEO_DAC_TYPENUM	23
#define VIDEO_DAC0		0x01
#define VIDEO_DAC1		0x02
#define VIDEO_DAC2		0x04
#define VIDEO_DAC3		0x08
#define VIDEO_DAC4		0x10
#define VIDEO_DAC5		0x20

enum Video_DacType
{
	Video_CVBS_1 = 0,
	Video_CVBS_2,
	Video_CVBS_3,
	Video_CVBS_4,
	Video_CVBS_5,
	Video_CVBS_6,	
	Video_SVIDEO_1,
	Video_SVIDEO_2,
	Video_SVIDEO_3,	
	Video_YUV_1,
	Video_YUV_2,	
	Video_RGB_1,
	Video_RGB_2,
	Video_SVIDEO_Y_1,
	Video_SECAM_CVBS1,
	Video_SECAM_CVBS2,
	Video_SECAM_CVBS3,
	Video_SECAM_CVBS4,
	Video_SECAM_CVBS5,
	Video_SECAM_CVBS6,	
	Video_SECAM_SVIDEO1,
	Video_SECAM_SVIDEO2,
	Video_SECAM_SVIDEO3,
};

enum Video_VgaMode
{
	Video_VGA_NOT_USE = 0,
	Video_VGA_640_480,
	Video_VGA_800_600
};

struct Video_DacIndex
{
	uint8 uDacFirst;     // for all   (CVBS , YC_Y ,YUV_Y,RGB_R)
	uint8 uDacSecond;   //for SVideo & YUV & RGB	(YC_C ,YUV_U,RGB_G)
	uint8 uDacThird;   //for YUV & RGB	(YUV_V,RGB_B)
};

struct Video_DacInfo
{
	int enable;
	enum Video_DacType type;
	enum Video_VgaMode mode;
	struct Video_DacIndex index;
	int progressive;
};

enum Video_TVSystem
{
	Video_PAL		= 0	, //	PAL4.43(==PAL_BDGHI)		(Fh=15.625,fv=50)
	Video_NTSC		, //	NTSC3.58					(Fh=15.734,Fv=59.94)
	Video_PAL_M		, //	PAL3.58					(Fh=15.734,Fv=59.94)
	Video_PAL_N		, //	PAL4.43(changed PAL mode)	(Fh=15.625,fv=50)	
	Video_PAL_60		, //							(Fh=15.734,Fv=59.94)
	Video_NTSC_443	, //	NTSC4.43					(Fh=15.734,Fv=59.94)
	Video_SECAM		,
	Video_MAC			,
	Video_LINE_720_25,	//added for s3601
	Video_LINE_720_30,	//added for s3601
	Video_LINE_1080_25,	//added for s3601
	Video_LINE_1080_30,	//added for s3601_
	Video_LINE_1080_50,	//added for s3602f
	Video_LINE_1080_60,   //added for s3602f
	Video_LINE_1080_24,	//added for s3602f	
	Video_LINE_1152_ASS,  //added for s3602f
	Video_LINE_1080_ASS,  //added for s3602f
	Video_PAL_NC,


	Video_LINE_800_600_VESA,
	Video_LINE_1024_768_VESA,
	Video_LINE_1360_768_VESA,	
};

//TVAspect
enum Video_TVAspect
{
	Video_TV_4_3 = 0,
	Video_TV_16_9,
	Video_TV_AUTO	//060517 yuchun for GMI Aspect Auto
};

//DisplayMode
enum Video_DisplayMode
{
	Video_PANSCAN = 0,
	Video_PANSCAN_NOLINEAR,	//non-linear pan&scan
	Video_LETTERBOX,
	Video_TWOSPEED, //add by t2
	Video_PILLBOX,
	Video_VERTICALCUT,
	Video_NORMAL_SCALE,
	Video_LETTERBOX149,
	Video_DONT_CARE,
	Video_AFDZOOM,
	//BOTH
};


#define VIDEO_PIC_WIDTH 720
#define VIDEO_PIC_HEIGHT 2880	//2880 is the lease common multiple of screen height of Pal and ntsc 
#define VIDEO_SCR_WIDTH 720
#define VIDEO_SCR_HEIGHT 2880

struct Video_Rect
{
	int32 x;	// Horizontal start point.
	int32 y;	// Vertical start point.
	int32 w;	// Horizontal size.
	int32 h;	// Vertical size.
};

struct Video_Pos
{
	int32 x;
	int32 y;
    int32 w;
    int32 h;
};

//YUV color
struct  Video_YCbCrColor
{
	uint8 Y;
	uint8 Cb;
	uint8 Cr;
};

enum Video_PicFmt
{
	// YCbCr Format
	Video_PicFmt_YCBCR_411,
	Video_PicFmt_YCBCR_420,
	Video_PicFmt_YCBCR_422,
	Video_PicFmt_YCBCR_444,
	
	//RGB format
	Video_PicFmt_RGB_MODE1,		//rgb (16-235)
	Video_PicFmt_RGB_MODE2,		//rgb (0-255)
};

#define DIS_FORMAT_CLUT8						0

#define DIS_FORMAT_MC420						10
#define DIS_FORMAT_MC422						(DIS_FORMAT_MC420 + 1)
#define DIS_FORMAT_MC444						(DIS_FORMAT_MC420 + 2)
#define DIS_FORMAT_AYCBCR8888					(DIS_FORMAT_MC444 + 3)

#define DIS_FORMAT_ARGB1555					20
#define DIS_FORMAT_RGB565						(DIS_FORMAT_ARGB1555 + 1)

#define DIS_FORMAT_ARGB8888					40
#define DIS_FORMAT_RGB888					41

#define VIDEO_MAX_GMA_REGION_NUM			6
#define VIDEO_MAX_GMA_WIDTH					4095
#define VIDEO_MAX_GMA_HEIGHT					4095

enum ALIFB_DE_LAYER
{
	DE_LAYER0, // it is for Ali Video Module
	DE_LAYER1, // it is the Ali Video PIP module or other
	DE_LAYER2, // GMA1
	DE_LAYER3, // GMA2
	DE_LAYER4, // GMA3
	DE_LAYER5, // GMA4
};

enum ALIFB_COLOR_TYPE
{
	ALIFB_COLOR_YUV,
	ALIFB_COLOR_RGB,
};

/* IO Command of Ali FB driver */
#define FBIO_GET_FBINFO								0x460001
#define FBIO_SET_OUTPUT_FRAME_PATH					0x460002
#define FBIO_OUTPUT_FRAME								0x460003
#define FBIO_CHECK_FRAME_FREE							0x460004
#define FBIO_GET_FBINFO_DATA							0x460005

#define FBIO_SET_TVSYS									0x460010
#define FBIO_GET_TVSYS									0x460011
#define FBIO_WIN_ZOOM									0x460012
#define FBIO_SET_ASPECT_MODE							0x460013
#define FBIO_GET_ASPECT_MODE							0x460014
#define FBIO_UNREG_DAC									0x460015
#define FBIO_REG_DAC									0x460016
#define FBIO_WIN_ONOFF									0x460017
#define FBIO_SET_PALLETTE								0x460018
#define FBIO_GET_PALLETTE								0x460019

#define FBIO_DELETE_GMA_REGION						0x460020
#define FBIO_CREATE_GMA_REGION						0x460021
#define FBIO_SET_GMA_SCALE_INFO						0x460022
#define FBIO_GET_GMA_REG_INFO							0x460023
#define FBIO_FLUSH_GMA_RECT							0x460024
#define FBIO_MOVE_REGION								0x460025

#define FBIO_SET_BG_COLOR								0x460030
#define FBIO_REG_HDMI_CALLBACK_ROUTINE				0x460031
#define FBIO_SET_HDMI_PIC_FRMT						0x460032
#define FBIO_FILL_COLOR									0x460033

#define FBIO_SET_DE_LAYER								0x460040
#define FBIO_RPC_OPERATION								0x460041

#define FBIO_SET_GLOBAL_ALPHA							0x460042

#define FBIO_DISABLE_GMA_ENHANCE						0x460043
#define FBIO_SET_REGION_SIDE_BY						0x460044

#define FBIO_SET_GE_CMDQ_BUF						    0x460100

#define FBIO_GET_UI_RSC_MAP						        0x460101
#define FBIO_GET_DISPLAY_RECT                           0X460102
#define FBIO_SET_DISPLAY_RECT                           0X460103
#define FBIO_SET_UI_CACHE_FLUSH						    0x460104

#define FBIO_SET_DBG_LEVEL							0x460105

struct alifbio_cache_flush
{
	unsigned long mem_start;
	int mem_size;
};

/* add by qj
 */
#define FBIO_REGISTER_ISR                       0x460200
#define FBIO_UNREGISTER_ISR                     0x460201
#define FBIO_FLAG_WAIT                          0x460202

/*FBIO_SET_OUTPUT_FRAME_PATH*/
enum ALIFB_OUTPUT_FRAME_PATH
{
	FRAME_FROM_KERNEL,
	FRAME_FROM_USER,		
};

/*FBIO_GET_FBINFO_DATA*/
struct alifbio_fbinfo_data_pars
{
	unsigned long mem_start;
	int mem_size;

	int xres_virtual;
	int yres_virtual;
	int line_length;
};

/*FBIO_SET_TVSYS*/
struct alifbio_tvsys_pars
{
	int progressive;
	enum Video_TVSystem tvsys;
};

/*FBIO_WIN_ZOOM*/
struct alifbio_zoom_pars
{
	struct Video_Rect src_rect;
	struct Video_Rect dst_rect;
};

/*FBIO_ASPECT_MODE*/
struct alifbio_aspect_pars
{
	enum Video_TVAspect aspect;
	enum Video_DisplayMode display_mode;
};

/*FBIO_SET_PALLETTE*/
enum ALIFB_PLT_TYPE
{
	PLT_RGB, //  0~7 : B; 8 ~ 15 : G; 16 ~ 23 : R; 24 ~ 31 : Alpha
	PLT_YUV, // 0~7 : V; 8 ~ 15 : Y 16 ~ 23 : U; ; 24 ~ 31 : Alpha
};

enum ALIFB_PLT_ALPHA_LEVEL
{
	PLT_ALPHA_16,
	PLT_ALPHA_128,
	PLT_ALPHA_256,
};

struct alifbio_plt_pars
{
	enum ALIFB_PLT_TYPE type;
	enum ALIFB_PLT_ALPHA_LEVEL level;

	void *pallette_buf;
	int color_num;
    
    unsigned char rgb_order;        // enum GE_RGB_ORDER
	unsigned char alpha_range;      // enum GE_ALPHA_RANGE
	unsigned char alpha_pol;        // enum GE_ALPHA_POLARITY
};

/*FBIO_CREATE_GMA_REGION & FBIO_GET_GMA_REG_INFO*/
struct alifbio_reg_pars
{
	int index;
	struct Video_Rect rect;
	uint32 dis_format;

	void *mem_start;
	int mem_size;
	int pitch;			
};

/*FBIO_SET_GMA_SCALE_INFO*/
enum ALIFB_GMA_SCALE_MODE
{
	GMA_RSZ_DISABLE =0,
	GMA_RSZ_DIRECT_RESIZE=4,  /* copy pixels directly */ 
	GMA_RSZ_ALPHA_ONLY =5,     /* filter active only on alpha channel */
	GMA_RSZ_COLOR_ONLY =6,     /* filter active only on color channel */
	GMA_RSZ_ALPHA_COLOR =7   /* filter active on alpha and color channel */
};

struct alifbio_gma_scale_info_pars
{
	enum ALIFB_GMA_SCALE_MODE scale_mode;
	
	/* scale parameters */
	int h_dst;
	int h_src;
	
	int v_dst;
	int v_src;
    int tv_sys;
    unsigned long uScaleCmd;
    unsigned long uScaleParam;
};

/* FBIO_FLUSH_GMA_RECT */
struct alifbio_flush_GMA_rect_pars
{
	int region_id;
	void *in_start;
	int in_pitch;

	struct Video_Rect rect;
};

/*  FBIO_MOVE_REGION */
struct alifbio_move_region_pars
{
	int region_id;
	struct Video_Pos pos;
};

/* FBIO_FILL_COLOR */
struct alifbio_fill_color_pars
{
	enum ALIFB_COLOR_TYPE type;
	uint32 color;
};
struct alifbio_cmdq_buf
{
    int cmdq_index;     // 0 - HQ, 1 - LQ
    uint32 *cmdq_buf;   // should be virtual address
    int cmdq_size;      // in bytes
};
	
/* FBIO_RPC_OPERATION :
it do Remote Process Call operation to the dis */
/*RPC API index definition */
#define RPC_FB_OPEN					1
#define RPC_FB_CLOSE				2
#define RPC_FB_WIN_ON_OFF			3
#define RPC_FB_WIN_MODE			4
#define RPC_FB_ZOOM					5
#define RPC_FB_ASPECT_MODE			6
#define RPC_FB_TVSYS				7
#define RPC_FB_TVSYS_EX				8
#define RPC_FB_IOCTL					9
#define RPC_FB_CONFIG_SOURCE_WINDOW	10
#define RPC_FB_SET_PROGRES_INTERL	11

#define MAX_FB_RPC_ARG_NUM		4

#define MAX_FB_RPC_ARG_SIZE		512

struct ali_fb_rpc_arg
{
	void *arg;
	int arg_size;
	int out;
};

struct ali_fb_rpc_pars
{
	int hd_dev; /* 0 : SD output 1 : HD output*/
	int API_ID;
	struct ali_fb_rpc_arg arg[MAX_FB_RPC_ARG_NUM];
	int arg_num;
};

struct ali_fb_rsc_mem_map
{
	void *mem_start;
	unsigned long mem_size;
};

/* IO Command of Ali Video Drvier */
/* magic num 0x55*/
#define ALIVIDEOIO_VIDEO_STOP							0x550001
#define ALIVIDEOIO_VIDEO_PLAY							0x550002
#define ALIVIDEOIO_VIDEO_PAUSE							0x550003
#define ALIVIDEOIO_VIDEO_RESUME						0x550004
#define ALIVIDEOIO_VIDEO_STEP							0x550005
#define ALIVIDEOIO_VIDEO_BLANK							0x550006
#define ALIVIDEOIO_VIDEO_DIRECTION					0x550007
#define ALIVIDEOIO_VIDEO_SPEED							0x550008
#define ALIVIDEOIO_VIDEO_SYNC_MODE					0x550009

#define ALIVIDEOIO_SET_DECODER_FORMAT				0x550020
#define ALIVIDEOIO_GET_INPUT_CALLBACK_ROUTINE		0x550021
#define ALIVIDEOIO_SET_FBINFO_HANDLE					0x550022
#define ALIVIDEOIO_SET_DMA_INFO						0x550023
#define ALIVIDEOIO_GET_OUT_INFO						0x550024
#define ALIVIDEOIO_OUTPUT_FIRST_FRAME				0x550025
#define ALIVIDEOIO_SET_PREVIEW_PARS					0x550026

#define ALIVIDEOIO_RPC_OPERATION						0x550040
#define ALIVIDEOIO_SET_SOCK_PORT_ID					0x550041
#define ALIVIDEOIO_GET_MEM_INFO                         			0x550042
#define ALIVIDEOIO_SET_CTRL_BLK_INFO                                0x550043
#define ALIALIVIDEOIO_GET_BOOTMEDIA_INFO                       0x550044

#define ALIVIDEOIO_DBG_BASE							0x55f000		
#define ALIVIDEOIO_ENABLE_DBG_LEVEL					(ALIVIDEOIO_DBG_BASE + 0x0001)
#define ALIVIDEOIO_DISABLE_DBG_LEVEL					(ALIVIDEOIO_DBG_BASE + 0x0002)
#define ALIVIDEOIO_SET_SEE_TEST_MODE					(ALIVIDEOIO_DBG_BASE + 0x0003)
#define ALIVIDEOIO_SET_APE_DBG_MODE					(ALIVIDEOIO_DBG_BASE + 0x0004)
#define ALIVIDEOIO_GET_APE_DBG_MODE					(ALIVIDEOIO_DBG_BASE + 0x0005)

/* ALIVIDEOIO_GET_INPUT_CALLBACK_ROUTINE parameters */
#define ALI_VIDEO_REQ_RET_FAIL 	ALI_DECV_REQ_RET_FAIL
#define ALI_VIDEO_REQ_RET_OK 		ALI_DECV_REQ_RET_OK
#define ALI_VIDEO_REQ_RET_ERROR	ALI_DECV_REQ_RET_ERROR

typedef struct ctrl_blk ali_vdeo_ctrl_blk;
typedef ali_dec_request_buf ali_video_request_buf;
typedef ali_dec_update_buf ali_video_update_buf;
	
struct ali_video_mem_info
{
	void *mem_start;
	unsigned long mem_size;
    void *priv_mem_start;
    unsigned long priv_mem_size;
};
/* ALIVIDEOIO_VIDEO_DIRECTION parameters */
/* forward or reverse mode */
enum ALI_VIDEO_DIRECTION
{
	ALI_VIDEO_FORWARD,
	ALI_VIDEO_BACKWARD
};

/* ALIVIDEOIO_VIDEO_SPEED parameters */
/* fast or slow play mode */
enum ALI_VIDEO_SPEED
{
	ALI_VIDEO_SPEEDX1,
	ALI_VIDEO_SPEEDX2,
	ALI_VIDEO_SPEEDX4,
	ALI_VIDEO_SPEEDX8,
	ALI_VIDEO_SPEEDX16,
	ALI_VIDEO_SPEEDX32,

	ALI_VIDEO_SPEED_1_2 = 0x11,
	ALI_VIDEO_SPEED_1_4,
	ALI_VIDEO_SPEED_1_8,
	ALI_VIDEO_SPEED_1_16,
	ALI_VIDEO_SPEED_1_32,	
	
	ALI_VIDEO_SPEED_STEP = 0x101
};

#define ALI_VIDEO_SPEED_UP_MAX			ALI_VIDEO_SPEEDX32
#define ALI_VIDEO_SPEED_DOWN_MAX			ALI_VIDEO_SPEED_1_32

/* ALIVIDEOIO_VIDEO_SYNC_MODE parameters */
enum ALI_VIDEO_SYNC_MODE
{
	ALI_VIDEO_SYNC_MODE_ENABLE,
	ALI_VIDEO_SYNC_MODE_DISABLE,
};

#define	ALI_VIDEO_SYNC_FIRSTFREERUN		0x0001
#define	ALI_VIDEO_SYNC_I					0x0002
#define	ALI_VIDEO_SYNC_P					0x0004
#define	ALI_VIDEO_SYNC_B					0x0008

#define	ALI_VIDEO_SYNC_ALL				0xFFFF

struct ali_video_sync_pars
{
	enum ALI_VIDEO_SYNC_MODE sync_mode;
	uint32 sync_flags;
};

/* ALIVIDEOIO_GE_OUT_INFO */
#define ALI_VIDEO_OUT_INFO_FLAG_FIRST_SHOWED		0x00000001
#define ALI_VIDEO_OUT_INFO_FLAG_FIRST_HEADER		0x00000002

struct ali_video_out_info_pars
{
	int dev_idx;
	
	uint32 flag;

	int started;
	int width;
	int height;
	int frame_rate; // frames per 1000seconds
	int progressive;	
	int display_idx;

	/* vbv buf paras */
	uint32 read_p_offset;
	uint32 write_p_offset;
	int valid_size;

	int is_support;
};

/* ALIVIDEOIO_SET_PREVIEW_PARS : 
it means the needed size of the decoded frames.
output the full size if width or height is 0*/
struct ali_video_preview_pars
{
	int preview_width;
	int preview_height;
};

/* ALIVIDEOIO_SET_SEE_TEST_MODE parameters */
enum ali_video_see_test_mode
{
	VDEC_TEST_NONE,
	VDEC_TEST_Check_NAL,
	VDEC_TEST_Check_HEADER,
};

/* ALIVIDEOIO_RPC_OPERATION :
it do Remote Process Call operation to the decv */
/*RPC API index definition */
#define RPC_VIDEO_OPEN				1
#define RPC_VIDEO_CLOSE			2
#define RPC_VIDEO_START			3
#define RPC_VIDEO_STOP				4
#define RPC_VIDEO_VBV_REQUEST		5
#define RPC_VIDEO_VBV_UPDATE		6
#define RPC_VIDEO_SET_OUT			7
#define RPC_VIDEO_SYNC_MODE		8
#define RPC_VIDEO_PROFILE_LEVEL	9
#define RPC_VIDEO_IO_CONTROL		10
#define RPC_VIDEO_PLAY_MODE		11
#define RPC_VIDEO_DVR_SET_PAR		12
#define RPC_VIDEO_DVR_PAUSE		13
#define RPC_VIDEO_DVR_RESUME		14
#define RPC_VIDEO_DVR_STEP			15
#define RPC_VIDEO_SELECT_DEC		16
#define RPC_VIDEO_GET_DECODER		17
#define RPC_VIDEO_IS_AVC			18
#define RPC_VIDEO_SWITCH_DEC		19
#define RPC_VIDEO_GET_CURRENT_DEC 	20
#define RPC_VIDEO_DECORE_IOCTL  21
#define RPC_VIDEO_DECODER_SELECT_NEW		22

#define RPC_VIDEO_INTENAL				0xF0000000
#define RPC_VIDEO_SET_DBG_FLAG		(RPC_VIDEO_INTENAL + 0x0001)

#define MAX_VIDEO_RPC_ARG_NUM		4
#define MAX_VIDEO_RPC_ARG_SIZE		2048

struct ali_video_rpc_arg
{
	void *arg;
	int arg_size;
	int out;
};

struct ali_video_rpc_pars
{
	int API_ID;
	struct ali_video_rpc_arg arg[MAX_VIDEO_RPC_ARG_NUM];
	int arg_num;
};

#define MSG_FIRST_SHOWED		  1
#define MSG_MODE_SWITCH_OK	      2
#define MSG_BACKWARD_RESTART_GOP  3
#define MSG_FIRST_HEADRE_PARSED	  4
#define MSG_UPDATE_OUT_INFO		  5

// decode status machine
#define VDEC_NEW_VIDEO_PACKET     1     // ready to decode new video packet(picture)
#define VDEC_WAIT_NEW_FB          2     // waiting for free buffer to decode video
#define VDEC_NEW_SLICE            3     // ready to decode new video slice
#define VDEC_ON_GOING             4     // video decoding is on going
#define VDEC_POST_PROCESS         5     // last picture decoding is done, ready to do some post process
#define VDEC_CONTINUE             6     // decore needs to push last decoded P VOP into DQ in B-VOP decoding and continue B-VOP decoding

// decode mode
#define VDEC_MODE_NORMAL          0     // normal decode
#define VDEC_MODE_VOL             1     // parse all headers above (including) VOP level without VOP reconstuction
#define VDEC_MODE_HEADER          2     // parse header to get current frame's prediction type
#define VDEC_MODE_SKIP_B_FRAME    3     // skip b frame
#define VDEC_MODE_SKIP_B_P_FRAME  4     // only decode i frame
#define VDEC_MODE_SBM             5     //

// decoder command
#define VDEC_CMD_INIT             0     // Initialize the decoder
#define VDEC_CMD_RELEASE          1     // Release the decoder
#define VDEC_CMD_DECODE_FRAME     2     // Decode a frame
#define VDEC_CMD_SW_RESET         3     // Reset the decoder sw status.
#define VDEC_CMD_HW_RESET         4     // Reset the HW to be ready to decode next frame
#define VDEC_CMD_EXTRA_DADA       5     // Decode extra data before decode frame
#define VDEC_CMD_GET_STATUS       6     // Get decoder status
#define VDEC_CMD_PAUSE_DECODE     7     // Pause decode task running

// display queue operations (void* pHandle, int decOpt, void* pParam1, void* pParam2)
// *pParam1 --(uint32)de_index if not specified, *pParam2 -- (uint32)frame_id if available.
// frame_id == (frame_array_idx << 8) | frame_idx. For single output, frame_array_idx == 0
#define VDEC_DQ_INIT              20    // *pParam1 -- de_index, *pParam2 --, int DQ for de_index
#define VDEC_DQ_ADD_NULL_FRAME    21    // *pParam1 -- de_index, *pParam2 --, Add an null frame into DQ
#define VDEC_DQ_PEEK_FRAME        22    // *pParam1 -- de_index, *pParam2 -- the next frame_id in DQ
#define VDEC_DQ_POP_FRAME         23    // *pParam1 -- de_index, *pParam2 -- the frame_id to be poped
#define VDEC_DQ_GET_COUNT         24    // *pParam1 -- de_index, *pParam2 -- the frame number in DQ
#define VDEC_FRAME_PEEK_FREE      25    // *pParam1 -- the next free frm_idx, *pParam2 -- the next free pip_idx
#define VDEC_FRAME_GET_INFO       26    // *pParam2 -- frame_id, pParam1 -- struct DisplayInfo *,
#define VDEC_FRAME_GET_POC        27    // *pParam2 -- frame_id, pParam1 -- int *p_poc,
#define VDEC_FRAME_SET_FREE       28    // *pParam2 -- frame_id, bit16 must be set or clear, see DEC_LAST_REQ_FRAME_FLAG.
#define VDEC_FRAME_REQUESTING     29    // *pParam2 -- frame_id, pParam1 -- struct OutputFrmManager *,
#define VDEC_FRAME_REQUEST_OK     30    // *pParam2 -- frame_id, *pParam1 -- de_index,
#define VDEC_FRAME_RELEASE_OK     31    // *pParam2 -- frame_id, *pParam1 -- de_index,
#define VDEC_CLEAR_SCREEN         32    // *pParam2 -- flag,     *pParam1 -- de_index,
#define VDEC_START_OUTPUT         33    // *pParam2 -- flag,     *pParam1 -- de_index,
#define VDEC_FRAME_GET_PTS        34    // *pParam2 -- frame_id, pParam1 -- uint32 *pts
#define VDEC_CFG_VIDEO_SBM_BUF    35    // *pParam1 -- smb idx for video pkt buf, *pParam2 -- sbm idx for video output queue
#define VDEC_CFG_DISPLAY_SBM_BUF  36    // *pParam1 -- smb idx for display queue
#define VDEC_CFG_SYNC_MODE        37    // *pParam1 -- av sync mode
#define VDEC_CFG_SYNC_THRESHOLD   38    // *pParam1 -- hold threshold, *pParam2 -- drop threshold
#define VDEC_CFG_SYNC_DELAY       39    // *pParam1 -- av sync delay [-500ms, 500ms]
#define VDEC_CFG_DISPLAY_RECT     40    // *pParam1 -- source rect, *pParam2 destination rect
#define VDEC_CFG_DECODE_MODE      41    // *pParam1 -- decode mode

// decode return values
#define VDEC_SUCCESS              0
#define VDEC_FAILURE              1
#define VDEC_FORBIDDEN_DECODE     2     // some reason cannot decode
#define VDEC_INVALID_CMD          3
#define VDEC_FRAME_SKIPED         4     // frame skiped according to decode mode
#define VDEC_NO_MEMORY            5     // memory not enough
#define VDEC_BAD_FORMAT           6
#define VDEC_NOT_IMPLEMENTED      7     // new feature not supported
#define VDEC_PATCH_PSUDO_MB       8     // Patch IC issue
#define VDEC_CURRENT_CONTINUE     9     // continue decode current frame

// decode error type (not supported features)
#define VDEC_ERR_NONE             0
#define VDEC_ERR_SHAPE           (1 << 0)  // shape coding, unsupported
#define VDEC_ERR_SPRITE          (1 << 1)  // static sprite, unsupported
#define VDEC_ERR_NOT8BIT         (1 << 2)  // video data precision N-bit, unsupported
#define VDEC_ERR_NEWPRED         (1 << 3)  // newpred mode, unsupported
#define VDEC_ERR_SCALAB          (1 << 4)  // scalability, unsupported
#define VDEC_ERR_REDUCEDRES      (1 << 5)  // reduced resolution vop, unsupported
#define VDEC_ERR_3POINTGMC       (1 << 6)  // 3-point gmc, video may appear distorted
#define VDEC_ERR_DATA_PARTITION  (1 << 7)  // data partition, unsupported
#define VDEC_ERR_RESOLUTION      (1 << 8)  // resolution unsupported
#define VDEC_ERR_CODEC           (1 << 9)  // codec unsupported
#define VDEC_ERR_NOMEMORY        (1 << 10) // not enough memory

// VdecInit.decoder_flag is a set of bit flags
#define VDEC_FLAG_HAS_LICENSED   (1 << 0)  // Has full license
#define VDEC_FLAG_AVC1_FORMAT    (1 << 1)  // AVC nalu has nalu_size on the head
#define VDEC_FLAG_MPEG4_DECODER  (1 << 2)  // I'm a general mpeg4 decoder
// other flag
#define VDEC_FLAG_LAST_REQ_FRAME (1 << 16) // An flag to free the last requested frame

#define VDEC_FRM_IDX_NOT_DISPLAY  0xfe     // An indicator for not display frame
#define VDEC_FRM_IDX_INVALID      0xff

#define VDEC_MIN_FRAME_BUF_NUM    4
#define VDEC_MAX_FRAME_BUF_NUM    6

#define MKTAG(a, b, c, d)	      (a | (b << 8) | (c << 16) | (d << 24))
#define h264	                  MKTAG('h','2','6','4')
#define xvid                      MKTAG('x','v','i','d')
#define mpg2                      MKTAG('m','p','g','2')
#define flv1                      MKTAG('f','l','v','1')
#define mjpg					  MKTAG('m','j','p','g')
#define rmvb                      MKTAG('r','m','v','b')
#define vp8 					  MKTAG('v','p','8',' ')
#define vc1 					  MKTAG('v','c','1',' ')
#define XD 					  	  MKTAG('x','d',' ',' ')
typedef unsigned long FourCC;

typedef struct _VdecHWMemConfig
{
    uint32 fb_y[VDEC_MAX_FRAME_BUF_NUM];
    uint32 fb_c[VDEC_MAX_FRAME_BUF_NUM];
    uint32 dv_y[VDEC_MAX_FRAME_BUF_NUM];
    uint32 dv_c[VDEC_MAX_FRAME_BUF_NUM];
    uint32 fb_max_stride;
    uint32 fb_max_height;
    uint32 dv_max_stride;
    uint32 dv_max_height;
    uint32 fb_num;
    uint32 neighbor_mem;
    uint32 colocate_mem;
    uint32 cmdq_base;
    uint32 cmdq_len;
    uint32 vbv_start;
    uint32 vbv_len;
} VdecHWMemConfig;

typedef struct _DViewCtrl
{
    uint8 dv_enable;
    uint8 dv_h_scale_factor;
    uint8 dv_v_scale_factor;
    uint8 interlaced:1;
    uint8 bottom_field_first:1;
}DViewCtrl;

// Structure for HW memory config
typedef struct _VdecFBConfig
{
    uint32 FBStride;
    uint32 FBMaxHeight;
    uint8  *FrmBuf;     //used only for SW_DECODE
    uint8  *DispBuf;    //In HW_DISPLAY mode, first 4 bytes store frame buffer addr, last 4 byte stor dv buffer addr
} VdecFBConfig;

/// Describes a compressed or uncompressed video format.
typedef struct _VdecFormatInfo
{
    FourCC fourcc;
    int32 bits_per_pixel;
    int32 pic_width;
    int32 pic_height;
    int32 pic_inverted;
    int32 pixel_aspect_x;
    int32 pixel_aspect_y;
    int32 frame_rate;
    int32 frame_period;
    int32 frame_period_const;
} VdecFormatInfo;

// decore notify event to video player engine
enum VdecEventType
{
    VDEC_EVENT_FRAME_FINISHED,   // one frame finished by VE
    VDEC_EVENT_FRAME_OUTPUT,     // one frame put into DQ
    VDEC_EVENT_DE_RELEASED,      // one frame released by DE
    VDEC_EVENT_DE_REQUESTING,    // Requesting frame by DE
    VDEC_EVENT_SLICE_FINISHED,   // one slice finished by VE
    VDEC_EVENT_FB_MALLOC_DONE,   // frame buffer malloc done
    VDEC_EVENT_FIELD_PIC_FLAG,   // 0 -- 1 frame 1 pts, 1 -- 1 field 1 pts
};

typedef int32 (* VdecDecoreEven)(enum VdecEventType event, uint32 param);
typedef int32 (VdecDecodeFunction)(void *pHandle, int vdecCmd, void *pParam1, void *pParam2);
typedef int32 (* DisplayRequest)(void *, void *, void *);
typedef int32 (* DisplayRelease)(void *, void *);

// Structure with parameters used to instantiate a decoder.
typedef struct _VdecInit
{
    VdecFormatInfo fmt_out; ///< Desired output video format.
    VdecFormatInfo fmt_in; ///< Given input video format
    const VdecFBConfig *pfrm_buf;
    const VdecHWMemConfig *phw_mem_cfg;
    VdecDecoreEven on_decode_event;
    DisplayRequest pfn_decore_de_request;
    DisplayRelease pfn_decore_de_release;
    uint32 decoder_flag;
    uint32 decode_mode;
    uint32 preview;
}VdecInit;

// Structure containing compressed video data.
typedef struct _VdecVideoPacket
{
    void *buffer;
    uint32 size;
    uint8 decoded;
}VdecVideoPacket;

typedef struct _VdecFrmBuf
{
    uint8 *buf_addr; //In HW_DISPLAY mode, first 4 bytes store frame buffer addr, last 4 byte stor dv buffer addr
    uint8 buf_id;
    uint8 prediction_type;
    DViewCtrl dv_ctrl;
}VdecFrmBuf;

// Structure containing input bitstream and decoder frame buffer.
typedef struct _VdecFrameConfig
{
    int32 decode_buf_id;
    int32 decode_finish; ///< Non-zero value means that a frame was successfully decoded.  Set by decoder.
    int32 decode_mode;
    int32 decode_status;
    int64 pts;
    int64 dts;
    VdecVideoPacket video_packet; ///< Input video packet to be decoded.
    DViewCtrl dview_ctrl;
    int64 pos;
}VdecFrameConfig;

// Structure that can be used to get extended information about a decoded frame
typedef struct _VdecFrameInfo
{
    uint32 frame_length;
    uint32 frame_num;
    uint32 vop_coded;

    uint32 stride_y;
    uint32 stride_uv;

    int32 decode_status;
    int32 decode_error;

    uint8 disp_buf_id;
    uint8 recon_buf_id;
    uint8 ref_for_buf_id;
    uint8 ref_back_buf_id;

    uint32 disp_buf_prediction_type;
    uint32 recon_buf_prediction_type;
    uint32 for_buf_prediction_type;
    uint32 bak_buf_prediction_type;

    uint32 cur_frame_pts;   // presentation timestamp of the decoded frame.

    DViewCtrl disp_buf_dv_ctrl;
    DViewCtrl for_buf_dv_ctrl;
    DViewCtrl bak_buf_dv_ctrl;
}VdecFrameInfo;

// Structure used by display queue management
enum VdecFrmArrayType
{
	VDEC_FRM_ARRAY_MP = 0x01,
	VDEC_FRM_ARRAY_DV = 0x02,

	VDEC_FRM_ARRAY_INVALID = 0xff,
};

struct OutputFrmManager
{
	uint32	display_index[2];

	enum VdecFrmArrayType de_last_request_frm_array[2];
	enum VdecFrmArrayType de_last_release_frm_array[2];
	//enum VdecFrmArrayType	de_using_frm_array[2];
	uint8  de_last_request_idx[2];
	uint8  de_last_release_idx[2];

	//uint8	de_using_frm_idx[2];

    //uint8	de_using_laf_idx;
	//uint8	de_last_request_laf_idx;

	int32 last_output_pic_released[2];
	int32 de_last_release_poc[2];
	int32 de_last_request_poc[2];

	uint8 frm_number;
	uint8 pip_frm_number;
};

struct vdec_decore_status
{
    uint32 decode_status;
    uint32 pic_width;
    uint32 pic_height;
    uint32 sar_width;
    uint32 sar_heigth;
    uint32 frame_rate;
    int32 interlaced_frame;
    int32 top_field_first;
    int32 first_header_got;
    int32 first_pic_showed;
    uint32 frames_decoded;
    uint32 frames_displayed;
    int64 frame_last_pts;
    uint32 buffer_size;
    uint32 buffer_used;
    uint32 decode_error;
    uint32 decoder_feature;
};

struct av_config
{
   int32 av_sync_mode;
   int32 hold_threshold;
   int32 drop_threshold;
   int32 av_sync_delay;
   int32 av_sync_unit;
   int32 preview;
   struct Video_Rect src_rect;
   struct Video_Rect dst_rect;
   int32 pause_decode;
   int32 pause_output;
};

enum {
    AV_SYNC_NONE,
    AV_SYNC_AUDIO,
    AV_SYNC_VIDEO,
    AV_SYNC_EXTERNAL, /* synchronize to an external clock */
};

enum av_picture_type 
{
    PICTURE_TYPE_I = 1, ///< Intra
    PICTURE_TYPE_P,     ///< Predicted
    PICTURE_TYPE_B,     ///< Bi-dir predicted
    PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG4
    PICTURE_TYPE_SI,    ///< Switching Intra
    PICTURE_TYPE_SP,    ///< Switching Predicted
    PICTURE_TYPE_BI,    ///< BI type
};

/**
 * Pan Scan area.
 * This specifies the area which should be displayed.
 * Note there may be multiple such areas for one frame.
 */
struct av_panscan{
    /**
     * id
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    int32 id;

    /**
     * width and height in 1/16 pel
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    int32 width;
    int32 height;

    /**
     * position of the top left corner in 1/16 pel for up to 3 fields/frames
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    int16 position[3][2];
};

/**
 * rational number numerator/denominator
 */
struct av_rational
{
    int32 num; ///< numerator
    int32 den; ///< denominator
};

struct av_packet 
{
    /**
     * Presentation timestamp in AVStream->time_base units; the time at which
     * the decompressed packet will be presented to the user.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     * pts MUST be larger or equal to dts as presentation cannot happen before
     * decompression, unless one wants to view hex dumps. Some formats misuse
     * the terms dts and pts/cts to mean something different. Such timestamps
     * must be converted to true pts/dts before they are stored in AVPacket.
     */
    int64 pts;
    /**
     * Decompression timestamp in AVStream->time_base units; the time at which
     * the packet is decompressed.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     */
    int64 dts;
    uint8 *data;
    int32 size;
    int32 stream_index;
    int32 flags;
    void *side_data;
    int32 side_data_elems;

    /**
     * Duration of this packet in AVStream->time_base units, 0 if unknown.
     * Equals next_pts - this_pts in presentation order.
     */
    int32 duration;
    void  (*destruct)(struct av_packet *);
    void  *priv;
    int64 pos;                            ///< byte position in stream, -1 if unknown
    int64 convergence_duration;
};

struct av_frame 
{
     /**
     * pointer to the picture planes.
     * This might be different from the first allocated byte
     * - encoding: 
     * - decoding: 
     */
    uint8 *data[4];
    int32 linesize[4];
    /**
     * pointer to the first allocated byte of the picture. Can be used in get_buffer/release_buffer.
     * This isn't used by libavcodec unless the default get/release_buffer() is used.
     * - encoding: 
     * - decoding: 
     */
    uint8 *base[4];
    /**
     * 1 -> keyframe, 0-> not
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    int32 key_frame;
    /**
     * Picture type of the frame, see ?_TYPE below.
     * - encoding: Set by libavcodec. for coded_picture (and set by user for input).
     * - decoding: Set by libavcodec.
     */
    enum av_picture_type pict_type;
    /**
     * presentation timestamp in time_base units (time when frame should be shown to user)
     * If AV_NOPTS_VALUE then frame_rate = 1/time_base will be assumed.
     * - encoding: MUST be set by user.
     * - decoding: Set by libavcodec.
     */
    int64 pts;
    /**
     * picture number in bitstream order
     * - encoding: set by
     * - decoding: Set by libavcodec.
     */
    int32 coded_picture_number;
    /**
     * picture number in display order
     * - encoding: set by
     * - decoding: Set by libavcodec.
     */
    int32 display_picture_number;
    /**\
     * quality (between 1 (good) and FF_LAMBDA_MAX (bad)) 
     * - encoding: Set by libavcodec. for coded_picture (and set by user for input).
     * - decoding: Set by libavcodec.
     */
    int32 quality; 
    /**
     * buffer age (1->was last buffer and dint change, 2->..., ...).
     * Set to INT_MAX if the buffer has not been used yet.
     * - encoding: unused
     * - decoding: MUST be set by get_buffer().
     */
    int32 age;
    /**
     * is this picture used as reference
     * The values for this are the same as the MpegEncContext.picture_structure
     * variable, that is 1->top field, 2->bottom field, 3->frame/both fields.
     * Set to 4 for delayed, non-reference frames.
     * - encoding: unused
     * - decoding: Set by libavcodec. (before get_buffer() call)).
     */
    int32 reference;
    /**
     * QP table
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    int8 *qscale_table;
    /**
     * QP store stride
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    int32 qstride;
    /**
     * mbskip_table[mb]>=1 if MB didn't change
     * stride= mb_width = (width+15)>>4
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    uint8 *mbskip_table;
    /**
     * motion vector table
     * @code
     * example:
     * int mv_sample_log2= 4 - motion_subsample_log2;
     * int mb_width= (width+15)>>4;
     * int mv_stride= (mb_width << mv_sample_log2) + 1;
     * motion_val[direction][x + y*mv_stride][0->mv_x, 1->mv_y];
     * @endcode
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    uint16 (*motion_val[2])[2];
    /**
     * macroblock type table
     * mb_type_base + mb_width + 2
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    uint32 *mb_type;
    /**
     * log2 of the size of the block which a single vector in motion_val represents: 
     * (4->16x16, 3->8x8, 2-> 4x4, 1-> 2x2)
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    uint8 motion_subsample_log2;
    /**
     * for some private data of the user
     * - encoding: unused
     * - decoding: Set by user.
     */
    void *opaque;
    /**
     * error
     * - encoding: Set by libavcodec. if flags&CODEC_FLAG_PSNR.
     * - decoding: unused
     */
    int64 error[4];
    /**
     * type of the buffer (to keep track of who has to deallocate data[*])
     * - encoding: Set by the one who allocates it.
     * - decoding: Set by the one who allocates it.
     * Note: User allocated (direct rendering) & internal buffers cannot coexist currently.
     */
    int32 type;
    /**
     * When decoding, this signals how much the picture must be delayed.
     * extra_delay = repeat_pict / (2*fps)
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    int32 repeat_pict;
    /**
     * 
     */
    int32 qscale_type;
    /**
     * The content of the picture is interlaced.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec. (default 0)
     */
    int32 interlaced_frame;
    /**
     * If the content is interlaced, is top field displayed first.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    int32 top_field_first;
    /**
     * Pan scan.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    struct av_panscan *pan_scan;
    /**
     * Tell user application that palette has changed from previous frame.
     * - encoding: ??? (no palette-enabled encoder yet)
     * - decoding: Set by libavcodec. (default 0).
     */
    int32 palette_has_changed;
    /**
     * codec suggestion on buffer type if != 0
     * - encoding: unused
     * - decoding: Set by libavcodec. (before get_buffer() call)).
     */
    int32 buffer_hints;
    /**
     * DCT coefficients
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    short *dct_coeff;
    /**
     * motion reference frame index
     * the order in which these are stored can depend on the codec.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    int8 *ref_index[2];
    /**
     * reordered opaque 64bit (generally an integer or a double precision float
     * PTS but can be anything). 
     * The user sets AVCodecContext.reordered_opaque to represent the input at
     * that time,
     * the decoder reorders values as needed and sets AVFrame.reordered_opaque
     * to exactly one of the values provided by the user through AVCodecContext.reordered_opaque
     * @deprecated in favor of pkt_pts
     * - encoding: unused
     * - decoding: Read by user.
     */
    int64 reordered_opaque;
    /**
     * hardware accelerator private data (FFmpeg allocated)
     * - encoding: unused
     * - decoding: Set by libavcodec
     */
    void *hwaccel_picture_private;
    /**
     * reordered pts from the last AVPacket that has been input into the decoder
     * - encoding: unused
     * - decoding: Read by user.
     */
    int64 pkt_pts;
    /**
     * dts from the last AVPacket that has been input into the decoder
     * - encoding: unused
     * - decoding: Read by user.
     */
    int64 pkt_dts;
    /**
     * the AVCodecContext which ff_thread_get_buffer() was last called on
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    //struct AVCodecContext *owner;
    void *owner;
    /**
     * used by multithreading to store frame-specific info
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    void *thread_opaque;
    /**
     * frame timestamp estimated using various heuristics, in stream time base
     * - encoding: unused
     * - decoding: set by libavcodec, read by user.
     */
    int64 best_effort_timestamp;
    /**
     * reordered pos from the last AVPacket that has been input into the decoder
     * - encoding: unused
     * - decoding: Read by user.
     */
    int64 pkt_pos;
    /**
     * reordered sample aspect ratio for the video frame, 0/1 if unknown\unspecified
     * - encoding: unused
     * - decoding: Read by user.
     */
    struct av_rational sample_aspect_ratio;
    /**
     * width and height of the video frame
     * - encoding: unused
     * - decoding: Read by user.
     */
    int32 width, height;
    /**
     * format of the frame, -1 if unknown or unset
     * It should be cast to the corresponding enum (enum PixelFormat
     * for video, enum AVSampleFormat for audio)
     * - encoding: unused
     * - decoding: Read by user.
     */
    int32 format;
};

#ifdef KERNEL_GMA_TEST

// define pixel format
enum GE_GMA_PIXEL_FORMAT
{
	// used by 3602
	GE_GMA_PF_RGB565		=0x00,
	GE_GMA_PF_RGB888		=0x01,
	GE_GMA_PF_RGB555		=0x02,
	GE_GMA_PF_RGB444		=0x03,
	GE_GMA_PF_ARGB565		=0x04,
	GE_GMA_PF_ARGB8888	=0x05,
	GE_GMA_PF_ARGB1555	=0x06,
	GE_GMA_PF_ARGB4444	=0x07,
	GE_GMA_PF_CLUT1		=0x08,
	GE_GMA_PF_CLUT2		=0x09,
	GE_GMA_PF_CLUT4		=0x0A,	
	GE_GMA_PF_CLUT8		=0x0B,
	GE_GMA_PF_ACLUT88		=0x0C,
	GE_GMA_PF_YCBCR888	=0x10,
	GE_GMA_PF_YCBCR422	=0x12,
	GE_GMA_PF_YCBCR422MB	=0x13,
	GE_GMA_PF_YCBCR420MB	=0x14,
	GE_GMA_PF_AYCBCR8888	=0x15,
	GE_GMA_PF_A1			=0x18,
	GE_GMA_PF_A8			=0x19,
	GE_GMA_PF_CK_CLUT2       =0x89,
	GE_GMA_PF_CK_CLUT4       =0x8A,
	GE_GMA_PF_CK_CLUT8       =0x8B,
	GE_GMA_PF_ABGR1555       =0x90, /* GE not support, but DE support in M3202*/
	GE_GMA_PF_ABGR4444       =0x91, /* GE not support, but DE support in M3202*/
	GE_GMA_PF_BGR565           =0x92, /* GE not support, but DE support in M3202*/

	// new added for the 36f
	GE_GMA_PF_ACLUT44        = 0x93,
	GE_GMA_PF_YCBCR444       = 0x94,
	GE_GMA_PF_YCBCR420 	= 0x95,
	GE_GMA_PF_MASK_A1		= GE_GMA_PF_A1,
	GE_GMA_PF_MASK_A8		= GE_GMA_PF_A8,
};

typedef struct _gma_pal_attr_t
{
	unsigned char pal_type;         // GE_PAL_RGB or GE_PAL_YCBCR
	unsigned char rgb_order;        // enum GE_RGB_ORDER
	unsigned char alpha_range;      // enum GE_ALPHA_RANGE
	unsigned char alpha_pol;        // enum GE_ALPHA_POLARITY
} gma_pal_attr_t, *pgma_pal_attr_t;

typedef const gma_pal_attr_t *pcgma_pal_attr_t;

typedef struct _gma_scale_param_t
{
	unsigned short tv_sys;       // enum TVSystem
	unsigned short h_div;
	unsigned short v_div;
	unsigned short h_mul;
	unsigned short v_mul;
} gma_scale_param_t, *pgma_scale_param_t;
typedef const gma_scale_param_t *pcgma_scale_param_t;
// End ge_gma_scale(dev, layer_id, scale_cmd, scale_param)

// ge_driver_config_t for ge_attach(dev, ge_driver_config_t *, layer_number);
typedef struct _gma_layer_config_t
{
	unsigned long mem_base;
	unsigned long mem_size;
	unsigned char  hw_layer_id;         // application can switch hw layer id
	unsigned char  color_format;        // default region color format, enum GMA_PIXEL_FORMAT
	unsigned short width, height;       // default region width and height
	unsigned short pixel_pitch;         // default region pixel pitch

	unsigned long bScaleFilterEnable       :1;  // enable/disable GMA scale filter
	unsigned long bP2NScaleInNormalPlay    :1;  // enable/disable PAL/NTSC scale in normal play mode
	unsigned long bP2NScaleInSubtitlePlay  :1;	 // enable/disable PAL/NTSC scale in subtitle play mode
	unsigned long bDirectDraw              :1;  // for CPU direct draw, no GE draw
	unsigned long bCacheable               :1;  // for CPU direct draw, no GE draw
	unsigned long reserved                 :29; // reserved for future use
} gma_layer_config_t, *pgma_layer_config_t;

// for create/move/get region
typedef struct _alifb_gma_region_t
{
	unsigned char   color_format;   // enum GMA_PIXEL_FORMAT
	unsigned char   galpha_enable;  // 0 - use color by color alpha; 1 - enable global alpha for this region
	unsigned char   global_alpha;   // If global alpha enable, please set global_alpha [0x00, 0xff]
	unsigned char   pallette_sel;   // pallette index for CLUT4

	unsigned short  region_x;       // x offset of the region, from screen top_left pixel
	unsigned short  region_y;       // y offset of the region, from screen top_left pixel
	unsigned short  region_w;
	unsigned short  region_h;

	unsigned long  bitmap_addr;    // 0 - use uMemBase(internal memory) which is set by ge_attach(gma_layer_config_t *);
	                        // bitmap_addr not 0 - use external memory address as region bitmap addr
	unsigned long  pixel_pitch;    // pixel pitch(not byte pitch) for internal memory or bitmap_addr

	                        // ge_create_region(): bitmap_addr and pixel_pitch determines the region bitmap address, total 4 cases:
	                        // Case 1: if bitmap_addr is 0, and pixel_pitch is 0, it will use region_w as pixel_pitch,
	                        //     and region bitmap addr will be allocated from uMemBase dynamically.
	                        // Case 2: if bitmap_addr is 0, and pixel_pitch is not 0, the region bitmap addr will be fixed:
	                        //     uMemBase + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel

	                        // Case 3: if bitmap_addr is not 0, and pixel_pitch is 0, the region bitmap addr will be:
	                        //     bitmap_addr + (bitmap_w * bitmap_y + bitmap_x) * byte_per_pixel
	                        // Case 4: if bitmap_addr is not 0, and pixel_pitch is not 0, the region bitmap addr will be:
	                        //     bitmap_addr + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel

	                        // ge_move_region(): region using internal memory can only change region_x, region_y, pal_sel;
	                        // ge_move_region(): region using external memory can change everyting in ge_region_t;

	unsigned long  bitmap_x;       // x offset from the top_left pixel in bitmap_addr or internal memory
	unsigned long  bitmap_y;       // y offset from the top_left pixel in bitmap_addr or internal memory
	unsigned long  bitmap_w;       // bitmap_w must >= bitmap_x + region_w, both for internal memory or external memory
	unsigned long  bitmap_h;       // bitmap_h must >= bitmap_y + region_h, both for internal memory or external memory
	unsigned long  region_index;
} alifb_gma_region_t, *pgma_gma_region_t;
typedef const alifb_gma_region_t *pcgma_region_t;

 #endif
 #endif
 
