#ifndef __DRIVERS_ALI_RPC_HLD_H
#define __DRIVERS_ALI_RPC_HLD_H

#include <ali/basic_types.h>
//#include <boot_common.h>
//#include <asm/mach-ali/typedef.h>
/*
typedef char			INT8;
typedef unsigned char	UINT8;

typedef short			INT16;
typedef unsigned short	UINT16;

typedef long			INT32;
typedef unsigned long	UINT32;

typedef unsigned long long UINT64;
typedef long long INT64;
*/
#define LOW_PRI             31    // Min number, but the highest level
#define HIGH_PRI            0     // Max number,but the lowest level
#define DEF_PRI             20    // default level for normal thread
#define HSR_PRI             10    // HSR level, higher than normal
#define DYN_PRI             18    // Dynamic level, only  once.

enum
{
	OSAL_PRI_LOW		= LOW_PRI,			/* Lowest,  for idle task */
	OSAL_PRI_NORMAL		= DEF_PRI,			/* Normal,  for user task */
	OSAL_PRI_HIGH		= 17,				/* High,    for system task */
	OSAL_PRI_CRITICL	= HSR_PRI,			/* Highest, for HSR scheduler */
};


#if 0
#define RET_CODE		int
#define RET_SUCCESS		((INT32)0)
#define RET_FAILURE		((INT32)1)
#define RET_BUSY		((INT32)2)
#define RET_STA_ERR		((INT32)3)
#define BOOL	 			int
#endif

#if 0
#define	USE_NEW_VDEC
#define VIRTUAL_DUAL_VIDEO_DECODER
#ifndef _BUILD_OTA_E_
#define DMX_XFER_V_ES_BY_DMA 	//added for s3601. transfer video ES by DMA, others by CPU copy
#endif

#if(SYS_PROJECT_FE == PROJECT_FE_ATSC)
#define SC_SEARCH_BY_DMA		// because M3602/M3601c have fixed IC bug, so re-enable it
#endif

//#define	VIDEO_SECOND_B_MONITOR

#ifdef VIDEO_SECOND_B_MONITOR
#define	VIDEO_2B_MONITOR_MAX 50
#define	VIDEO_LINEMEET_1 0x01
#define	VIDEO_LINEMEET_2 0x02
#define	VIDEO_VBLANK	 0x03
#define	VIDEO_FINISH	 0x04
struct second_b_monitor
{
	UINT8	process_idx;
	UINT32	vbi_index;
	UINT32	de_scan_line;
	UINT32	decoding_head_idx;
	UINT32	mb_y;
	UINT32	mb_x;
};
#endif


#define VIDEO_ADPCM_ON     0x00
#define VIDEO_ADPCM_OFF    0x01
#define VIDEO_ADPCM_OPT_75 0x01
#define VIDEO_ADPCM_OPT_66 0x02
#define VIDEO_ADPCM_OPT_50 0x03

//#define VIDEO_FOR_SDRAM_PRJ
#ifdef VIDEO_FOR_SDRAM_PRJ
#define	IC_REV_0	0x01
#define	IC_REV_1	0x02
#define	IC_REV_2	0x03
#endif
#endif

//Position   (x,y)
struct Position
{
	UINT16	uX;	// Horizontal position .
	UINT16	uY;	// Vertical position.
};

//rectangle size
struct RectSize
{
	UINT16	uWidth; 	// Horizontal size.
	UINT16	uHeight;// Vertical size.
};

//Rectangle
struct Rect
{
#if (defined(_MHEG5_ENABLE_) || defined(_MHEG5_V20_ENABLE_) )
	INT16	uStartX;	// Horizontal start point.
	INT16	uStartY;	// Vertical start point.
#else
	UINT16	uStartX;	// Horizontal start point.
	UINT16	uStartY;	// Vertical start point.
#endif
	UINT16	uWidth;	// Horizontal size.
	UINT16	uHeight;	// Vertical size.
};


/********************************************************************
							Play Mode & Speed

*********************************************************************/
struct PlayMode
{
	UINT8	uMode;
	INT16 	nSpeed;
};

//for uMode
#define 	PLAYMODE_INVALID		0   //For error detection, should not send down
#define 	PLAYMODE_FORWARD 	1
#define	PLAYMODE_BACKWARD	2
#define	PLAYMODE_PAUSE		3


//for nSpeed-----RATE_2X ~ RATE_64X value should not changed!
#define RATE_1_2X 	-2
#define RATE_1_4X  	-4
#define RATE_1_8X	-8
#define RATE_1_16X 	-16
#define RATE_1_32X 	-32
#define RATE_1_64X	-64

#define RATE_DEFAULT		0 //if  not to change rate
#define RATE_1X 		1

#define RATE_2X  	2
#define RATE_4X  	4
#define RATE_8X	 	8
#define RATE_16X	16
#define RATE_32X	32
#define RATE_64X      64
#define RATE_100X    100


#if 0
/********************************************************************
							audio properties
*********************************************************************/
#define MAX_AUDIO_CH_NUM  8 //maximum audio channel number

// Audio stream type
enum AudioStreamType
{
	AUDIO_MPEG1,		// MPEG I
	AUDIO_MPEG2,		// MPEG II
	AUDIO_MPEG_AAC,
	AUDIO_AC3,			// AC-3
	AUDIO_DTS,		//DTS audio for DVD-Video
	AUDIO_PPCM,		//Packet PCM for DVD-Audio
	AUDIO_LPCM_V,		//Linear PCM audio for DVD-Video
	AUDIO_LPCM_A,		//Linear PCM audio for DVD-Audio
	AUDIO_PCM,			//PCM audio
	AUDIO_WMA,		//WMA audio
	AUDIO_RA8,			//Real audio 8
	AUDIO_MP3,			//MP3 audio
	AUDIO_INVALID,
	AUDIO_MPEG_ADTS_AAC,
	AUDIO_OGG,
	AUDIO_EC3
};	//reserved : A_DV,	A_SACD,	A_WMA, 	A_SVD, A_RA, 	A_HDCD


//audio bit rate, kbps
enum	AudioBitRate
{
	A_BITRATE_INVALID,
	A_BITRATE_32,	//32k bps	,
	A_BITRATE_40,
	A_BITRATE_48,
	A_BITRATE_56,
	A_BITRATE_64,
	A_BITRATE_80,
	A_BITRATE_96,
	A_BITRATE_112,	//bit rate 112 kbps
	A_BITRATE_128,
	A_BITRATE_160,
	A_BITRATE_192,
	A_BITRATE_224,
	A_BITRATE_256,
	A_BITRATE_320,
	A_BITRATE_384,
	A_BITRATE_416,
	A_BITRATE_448,
	A_BITRATE_512,
	A_BITRATE_576,
	A_BITRATE_640,
	A_BITRATE_768  //typical for DVD DTS

};

// Audio Sample Rate
enum AudioSampleRate
{
	AUDIO_SAMPLE_RATE_INVALID = 1,	// Invalid sample rate
	AUDIO_SAMPLE_RATE_8,		// 8 KHz
	AUDIO_SAMPLE_RATE_11,		// 11.025 KHz
	AUDIO_SAMPLE_RATE_12,		// 12 KHz
	AUDIO_SAMPLE_RATE_16,		// 16 KHz
	AUDIO_SAMPLE_RATE_22,		// 22.05 KHz
	AUDIO_SAMPLE_RATE_24,		// 24 KHz
	AUDIO_SAMPLE_RATE_32,  		// 32 KHz
	AUDIO_SAMPLE_RATE_44,  		// 44.1 KHz
	AUDIO_SAMPLE_RATE_48,		// 48 KHz
	AUDIO_SAMPLE_RATE_64,		// 64 KHz
	AUDIO_SAMPLE_RATE_88,		// 88.2 KHz
	AUDIO_SAMPLE_RATE_96,		// 96 KHz
	AUDIO_SAMPLE_RATE_128,		// 128 KHz
	AUDIO_SAMPLE_RATE_176,		// 176.4 KHz
	AUDIO_SAMPLE_RATE_192		// 192 KHz
};


// Audio Quantization
enum AudioQuantization
{
	AUDIO_QWLEN_INVALID = 1,
	AUDIO_QWLEN_8,			// 8 Bits
	AUDIO_QWLEN_12,			// 12 Bits
	AUDIO_QWLEN_16,			// 16 Bits
	AUDIO_QWLEN_20,			// 20 Bits
	AUDIO_QWLEN_24,			// 24 Bits
	AUDIO_QWLEN_32			// 32 Bits
};

//channel assignment
enum ADecChannel
{
	ADEC_CHANNEL_NONE,	//not used
	ADEC_CHANNEL_L,		// left, including down-mixed L
	ADEC_CHANNEL_R,		// right, including down-mixed R
	ADEC_CHANNEL_LF,		// front left
	ADEC_CHANNEL_RF,		// front right
	ADEC_CHANNEL_C,		// center
	ADEC_CHANNEL_LFE,		// low frequence
	ADEC_CHANNEL_LS,		// left surround
	ADEC_CHANNEL_RS		// right surround
};


enum AC3BitStreamMode
{
	AC3_BSMOD_COMPLETE_MAIN = 0,
	AC3_BSMOD_MUSIC_EFFECTS,
	AC3_BSMOD_VISUALLY_IMPAIRED,
	AC3_BSMOD_HEARING_IMPAIRED,
	AC3_BSMOD_DIALOGUE,
	AC3_BSMOD_COMMENTARY,
	AC3_BSMOD_EMERGENCY,
	AC3_BSMOD_VOICE_OVER,
	AC3_BSMOD_KARAOKE,
};
#endif

/********************************************************************
							video & color properties

*********************************************************************/

// Video spec format (mpeg1/2/4),
enum VidSpecFmt
{
	MPEG1_ES = 0,	// MPEG1 Video ES stream
	MPEG2_ES, 	// MPEG2 Video ES stream
	MPEG4_ES 	// MPEG4 Video stream
};



// TV system
enum TVSystem
{
	PAL		= 0	, //	PAL4.43(==PAL_BDGHI)		(Fh=15.625,fv=50)
	NTSC		, //	NTSC3.58					(Fh=15.734,Fv=59.94)
	PAL_M		, //	PAL3.58					(Fh=15.734,Fv=59.94)
	PAL_N		, //	PAL4.43(changed PAL mode)	(Fh=15.625,fv=50)	
	PAL_60		, //							(Fh=15.734,Fv=59.94)
	NTSC_443	, //	NTSC4.43					(Fh=15.734,Fv=59.94)
	SECAM		,
	MAC			,
	LINE_720_25,	//added for s3601
	LINE_720_30,	//added for s3601
	LINE_1080_25,	//added for s3601
	LINE_1080_30,	//added for s3601

	LINE_1080_50,	//added for s3602f
	LINE_1080_60,   //added for s3602f
	LINE_1080_24,	//added for s3602f	
	LINE_1152_ASS,  //added for s3602f
	LINE_1080_ASS,  //added for s3602f
	PAL_NC,

	LINE_576P_50_VESA,
	LINE_720P_60_VESA,
	LINE_1080P_60_VESA,
};


// Frame rate
enum FrameRate
{
	FRAME_RATE_0 = 0, 	//	0 f/s, forbidden in MPEG
	FRAME_RATE_23976, //	23.976 f/s
	FRAME_RATE_24 ,	//	24 f/s
	FRAME_RATE_25,	//	25 f/s
	FRAME_RATE_2997,	//	29.97 f/s
	FRAME_RATE_30,	//	30 f/s
	FRAME_RATE_50,	//	50 f/s
	FRAME_RATE_5994,	//	59.94 f/s
	FRAME_RATE_60,	//	60 f/s
	FRAME_RATE_15,	//	15 f/s
	FRAME_RATE_INVALID //	invalid frame rate value
};

//Aspect ratio
enum AspRatio
{
	DAR_FORBIDDEN = 0, //aspect ratio forbidden
	SAR, 				//sample aspect ratio
	DAR_4_3,			//Display aspect ratio 3/4
	DAR_16_9,			//Display aspect ratio 9/16
	DAR_221_1			//Display aspect ratio 1/2.11
}; //P126, value must NOT change!


//highlight information
struct HLInfo
{
	UINT32	uColor;	// Selection color or action color code definition:reference DVD-VIDEO P VI4-122
	UINT32	uStartTime;	// start pts of hight light.
	struct Rect	rect;	// position and size of this hightlight area.
};


//TVMode
enum TVMode
{
	TV_4_3 = 0,
	TV_16_9,
	TV_AUTO	//060517 yuchun for GMI Aspect Auto
};

//DisplayMode
/*
enum DisplayMode
{
	PANSCAN = 0,
	PANSCAN_NOLINEAR,	//non-linear pan&scan
	LETTERBOX
	//BOTH
};
*/

//DisplayMode
enum DisplayMode
{
	PANSCAN = 0,		// default panscan is 16:9 source on 4:3 TV.
	PANSCAN_NOLINEAR,	//non-linear pan&scan
	LETTERBOX,
	TWOSPEED, //add by t2
	PILLBOX,
	VERTICALCUT,
	NORMAL_SCALE,
	LETTERBOX149,
	AFDZOOM,
	PANSCAN43ON169,		// 4:3 source panscan on 16:9 TV.
	COMBINED_SCALE,
	DONT_CARE,
	
	//BOTH
};

/*
enum RGBSubType
{
	RGB_555,	// Each component is 5 bits
	RGB_888		// Each component is 8 bits
};
*/

// picture format
enum PicFmt
{
	// YCbCr Format
	YCBCR_411,
	YCBCR_420,
	YCBCR_422,
	YCBCR_444,
	//RGB format
	RGB_MODE1,		//rgb (16-235)
	RGB_MODE2		//rgb (0-255)
};

//YUV color
struct  YCbCrColor
{
	UINT8 	uY;
	UINT8	uCb;
	UINT8	uCr;
};

//Y, Cb, Cr address
struct YCbCrAddr
{
	UINT8*	pY_Addr; //address of the  Y valure array of a picture
	UINT8*	pCb_Addr;
	UINT8*	pCr_Addr;
};

// Picture types for MPEG
enum   PicType
{
	UNKNOWN_PIC = 0,
	I_PIC =	1,
	P_PIC =	2,
	B_PIC =	3,
       D_PIC =    4
};

enum PicMemMapMode
{
    MEMMAP_16X16,
    MEMMAP_32X16,
};

enum VideoColorPrimaries
{
    CPRIM_FORBIDDEN = 0,
    CPRIM_BT709     = 1,
    CPRIM_UNKWOWN   = 2,
    CPRIM_RESEVERD3 = 3,
    CPRIM_BT470M    = 4,
    CPRIM_BT470BG   = 5,
    CPRIM_SMPTE170M = 6,
    CPRIM_SMPTE240M = 7,
    CPRIM_GENERIC_FILM = 8,
};

enum VideoTransferCharacter    // Video Transfer Characteristics
{
    TRANSC_FORBIDDEN = 0,
    TRANSC_BT709     = 1,
    TRANSC_UNKWOWN   = 2,
    TRANSC_RESEVERD3 = 3,
    TRANSC_BT470M    = 4,
    TRANSC_BT470BG   = 5,
    TRANSC_SMPTE170M = 6,
    TRANSC_SMPTE240M = 7,
    TRANSC_LINEAR    = 8,
    TRANSC_LOG_100   = 9,
    TRANSC_LOG_316   = 10,
};

enum VideoMatrixCoeff       // Video Matrix Coefficients
{
    MCOEF_FORBIDDEN = 0,
    MCOEF_BT709     = 1,
    MCOEF_UNKWOWN   = 2,
    MCOEF_RESEVERD3 = 3,
    MCOEF_FCC       = 4,
    MCOEF_BT470BG   = 5,
    MCOEF_SMPTE170M = 6,
    MCOEF_SMPTE240M = 7,
};

enum VP_SOURCE_TYPE
{
    VP_SRC_VIDEO = 0,       // For displaying video pictures from TS, etc.
    VP_SRC_JPEG  = 1,       // For displaying computer pictures, such as JPEG, BMP, etc.
};

/******************added by rachel ****************************/

enum StillMode
{
	STILLMODE_AUTO = 0,
	STILLMODE_FRAME = 1,
	STILLMODE_FIELD = 2
};



struct VPO_YCbCrAddress
{
	UINT32		uYAddress;
	UINT32		uCbAddress;
	UINT32		uCrAddress;
};


/******************end by rachel ****************************/




/********************************************************************
			time information of  MPEG 1/2

*********************************************************************/
//system clock reference
struct  SCR_T
{
	UINT8 	uScr0;		// lsb
	UINT32 	uScr;		// msb 32 bits
	UINT16 	uScr_ext;	// 9 bits in MPEG2 (MPEG1 doesn't have this field)
};

//time stamp, for PTS/DTS
struct PTS_T
{
	UINT8 	uTs0;	//lsb
	UINT32 	uTs;	//high 32 bits of time stamp
};


/**************************************************************
	start code name in MPEG 1/2, NOT stream ID!

***************************************************************/
//system start codes
//#define ISO_11172_END_CODE  0x000001B9
#define PACK_START_CODE  0x000001BA
#define SYSTEM_HEADER_START_CODE  0x000001BB

//packet start codes (including those used in PS and TS)
#define PROGRAM_STREAM_MAP 	0x000001BC
#define PRIVATE_STREAM_1  0x000001BD
#define PADDING_STREAM		0x000001BE
#define PRIVATE_STREAM_2  0x000001BF

#define AUDIO_STREAM_0     	0x000001C0
//...										//successive in ascending order
#define AUDIO_STREAM_31     0x000001DF

#define VIDEO_STREAM_0		 0x000001E0
//...										//successive in ascending order
#define VIDEO_STREAM_15	 0x000001EF

#define RESERVED_STREAM_0		 0x000001F0
//...										//successive in ascending order
#define RESERVED_STREAM_15	 0x000001FF


#define V_SEQUENCE_START 	0x000001b3
#define V_SEQUENCE_END		0x000001b7
#define PICTURE_START		0x00000100
#define GOP_START			0x000001b8

#define MPEG_AUDIO_SYNCWORD 0xfff

/**************************************************************
	stream ID of MPEG 1/2

***************************************************************/
//system level
#define  	ISO_11172_END_CODE 	0xB9
#define 	PACK_HEAD_CODE		0xBA
#define	SYSTEM_HEAD_CODE  	0xBB

#define 	STREAM_ID_PROGRAM_MAP 	0xBC
#define 	STREAM_ID_PRIVATE_1  		0xBD
#define 	STREAM_ID_PADDING		0xBE
#define 	STREAM_ID_PRIVATE_2  		0xBF


#define 	STREAM_ID_AUDIO_0     	0xC0
#define 	STREAM_ID_AUDIO_1     	0xC1
//...										//successive in ascending order
#define 	STREAM_ID_AUDIO_31     0xDF

#define 	STREAM_ID_VIDEO_0		 0xE0
#define 	STREAM_ID_VIDEO_1		 0xE1	//SVCD still picture
#define 	STREAM_ID_VIDEO_2		 0xE2	//SVCD high-definition still picture
//...										//successive in ascending order
#define 	STREAM_ID_VIDEO_15	 	0xEF

#define 	STREAM_ID_RESERVEDM_0		 0xF0
//...										//successive in ascending order
#define 	STREAM_ID_RESERVEDM_15	 0xFF
//reserved stream used
#define   STREAM_ID_ECM				0XF0
#define 	STREAM_ID_EMM				0XF1
#define   STREAM_ID_DSM_CC			0XF2



/**************************************************************
	sub-stream ID of MPEG 1/2

***************************************************************/
//sub_stream_id for private stream 2, only NV_VR support(not parse now)
#define 	SUB_STREAM_ID_RDI	0x50

//NV-video
#define 	SUB_STREAM_ID_SUB_PICTURE_0 		0x20   //NV_VR also support
#define 	SUB_STREAM_ID_SUB_PICTURE_31 	0x3f

#define 	SUB_STREAM_ID_AC3_0  		0x80 //NV_VR also support
#define 	SUB_STREAM_ID_AC3_1  		0x81//NV_VR also support
#define 	SUB_STREAM_ID_AC3_7  		0x87

#define 	SUB_STREAM_ID_DTS_0  		0x88
#define 	SUB_STREAM_ID_DTS_7  		0x8f

#define 	SUB_STREAM_ID_SDDS_0  	0x90
#define 	SUB_STREAM_ID_SDDS_7  	0x97

#define 	SUB_STREAM_ID_LPCM_0  	0xa0//NV_VR also support
#define 	SUB_STREAM_ID_LPCM_1  	0xa1//NV_VR also support
#define 	SUB_STREAM_ID_LPCM_7  	0xa7

#define 	INVALID_MPEG_STREAM_ID      0
/**************************************************************
	DVD/HDD/VCD PE   stream ID

***************************************************************/

//default A/V settings of PE
//#define  	PE_INVALID_STREAM_ID		INVALID_MPEG_STREAM_ID	//when there is no audio/sub-picture stream exists
										//if A/SP stream not exist, don't use API to give settings

//#define  	PE_DEFALT_ID_VIDEO  		STREAM_ID_VIDEO_0

#define 	DVD_SECTOR_SIZE 			2048

//Rachel From M3357 DRV_Common.h
enum DeinterlacingAlg
{
	NORMAL_WEAVE = 0,
	HALF_PHASE_WEAVE,
	NORMAL_BOB,
	SINGLE_FIELD_BOB,
	NORMAL_MAF,
	SINGLE_FIELD_MAF,
	NORMAL_BOB_CONV
};

enum DitAlgSingleOption
{
	DIT_SINGLE_TOP = 0,
	DIT_SINGLE_BOT
};
/*
enum AspRatio 
{
	DAR_FORBIDDEN, 
	DAR_4_3,
	DAR_16_9,
};
*/
enum RepeatField
{
	Repeat_Top_Field,
	Repeat_Bottom_Field,
	Repeat_None
};

enum Output_Frame_Ret_Code
{
	Ret_Output_Success,
//	Ret_Decision_Not_Ready,	//VPO Decision process not finish
//	Ret_Time_Not_Reach,		//decided by A/V sync, the display time is not reached
	Ret_No_Decoded_Frame	//there is no decoded frame in display queue
};

struct PanScanInfo
{
	UINT32 uNumberOfOffset;
	short HorizontalOffsets[3];
};

struct Video_Info
{
	int width;		//picture size in horizontal
	int height;		//picture size in vertical
	int frame_rate;	//video source frame rate
	//enum AspRatio eAspectRatio; //aspect ratio for display
	UINT32 eAspectRatio;
	UINT8  active_format;
    int sar_width; //Added for H264, this information is from decoder
    int sar_height; //Added for H264, this information is from decoder
    enum TVSystem src_tv_sys; //Added for H264, this information is from decoder
};


enum top_to_bottom_phase_diff
{
    vertical_phase_diff_1_2=0,
    vertical_phase_diff_1_4,
    vertical_phase_diff_1_8,
    vertical_phase_diff_1_16,
    vertical_phase_diff_1_32
};

enum field_sourcce
{
    top_field_source_only=0,
    bottom_field_source_only,
    both_fields_source
};

enum scaler_source
{
    scaler_field_based=0,
    scaler_frame_based
};

enum deinterlace_effort
{
   dit_effort_very_low = 0,
   dit_effort_low,
   dit_effort_medium,
   dit_effort_high
};

#define	VIDEO_OPEN_WIN_FLAG         0x01
#define	VIDEO_RESET_SRC_DST_FLAG    0x02
#define VIDEO_INTO_SML_MODE         0x04
#define VIDEO_LEAVE_SML_MODE        0x08
#define	VIDEO_FIRST_PIC_SYNC_FLAG   0x10
#define VIDEO_OPEN_PIP_WIN_FLAG     0X80

struct Display_Info
{
//#ifdef H264_SUPPORT	
	struct FrameStore *fs;
//#endif
	BOOL bNewVideoInfo;	//the flag indicates the structure pVideoInfo has changed or not
	
	BOOL bProgressiveSource;// Indicating the source is progressive or not
	BOOL bTopFieldFirst;	// VPO should only check this flag when bProgressiveSource is FALSE;
	enum RepeatField eRepeatField;   // Only check it when frame rate is 24 FPS

	UINT8 top_index;	//frame buffer index containing top field
	UINT8 *top_y;	//the address of frame buffer, which contains top field luminance
	UINT8 *top_c;	//the address of frame buffer, which contains top field chrominance
	UINT8 bottom_index;	//frame buffer index containing bottom field
	UINT8 *bottom_y;	//the address of frame buffer, which contains bottom field luminance
	UINT8 *bottom_c;	//the address of frame buffer, which contains bottom field chrominance
	UINT8 *pbMAFBuffer; //the address of flag buffer, which contains motion adaptive deinterlacing flags.
	BOOL bh264_mapping;
	int stride;		//marco block numbers in a marco block line
	
	struct Video_Info *pVideoInfo;	//a pointer to structure stored video information
	struct PanScanInfo *pPanScanInfo; //
	BOOL zoomflag;	//the flag indicates the accomplishment of zoom coefficients calculations

	BOOL	bDitAlgValid;
	enum DeinterlacingAlg eDeinterlacingAlg; //selection of deinterlacing algorithm
	enum DitAlgSingleOption eDitSingleOpt;
	BOOL	bbase_single_field;
	BOOL	bbase_single_top;
	
	UINT8	src_output_mode;
	BOOL	src_adpcm_onoff;
	UINT8	src_adpcm_ratio;
	BOOL	src_top_fieldstructure;
	BOOL	src_top_sml;
	UINT8	src_top_sml_height;
	BOOL	src_bot_fieldstructure;
	BOOL	src_bot_sml;
	UINT8	src_bot_sml_height;
	BOOL	bshow_one_field_time;
	struct Rect		src_rect;
	struct Rect		dst_rect;
	UINT8 	init_phase;

	UINT8	bvp_act_flag;
	BOOL	bhalf_mode;//addedf for s3601

    //vpo->vdec
	UINT8	uCurTopBot;
	//vdec->vpo accordint to the uCurTopBot and size changed
	BOOL	bwaitnextfield;

    UINT8   type;       // enum PicType
    UINT32  display_idx;
    UINT32  head_idx;
    UINT32 *pvld_err;

	UINT8 colour_primaries;	// 1:BT709, 0:BT601, see enum VideoColorPrimaries
	BOOL	bReleaseInLM;//if true,release frame in linemeet,else release frame in vblanking.
	BOOL     bH264Decoder;//temporarily use
	BOOL   bDView;


    enum top_to_bottom_phase_diff   eTop_to_bottom_phase_diff_luma;
    enum top_to_bottom_phase_diff   eTop_to_bottom_phase_diff_chroma;
    enum field_sourcce   eField_source;
    enum scaler_source   eScaler_source;
    enum deinterlace_effort   eDeinterlace_effort;

    BOOL 	b_maf_enable;
	UINT8   transfer_characteristics;   // enum VideoTransferCharacter
	UINT8	matrix_cofficients;         // enum VideoMatrixCoeff
    UINT8   sample_format;              // enum PicFmt
    UINT8   source_type;                // enum VP_SOURCE_TYPE

};

//wl add
struct PIP_Dislay_Info
{
	
	BOOL bNewVideoInfo;	//the flag indicates the structure pVideoInfo has changed or not

	BOOL bProgressiveSource;// Indicating the source is progressive or not
	BOOL bTopFieldFirst;	// VPO should only check this flag when bProgressiveSource is FALSE;
	enum RepeatField eRepeatField;   // Only check it when frame rate is 24 FPS

	
	UINT8 buf_index;	
	UINT8 *puYAddr;	
	UINT8 *puCAddr;	

	int stride;		//marco block numbers in a marco block line
	
	struct Video_Info *pVideoInfo;	//a pointer to structure stored video information
	enum DeinterlacingAlg eDeinterlacingAlg; //selection of deinterlacing algorithm
	BOOL bSingleFieldIsTopField;	

	// wl add
	UINT16 uXStaPos;
	UINT16 uYStaPos;
// ~wl add
	BOOL	bFirstPIP;

////only for  VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW 
	BOOL	bpip_layer_on;
	UINT32	pip_y_addr;
	UINT32	pip_c_addr;
	struct Rect	pip_rect;
	UINT16	pip_buffer_stride;
//#endif
};
//End Rachel

/***Add some definition for STB solution***/
struct control_block{
       UINT8 stc_id_valid:1; // 1: valid, 0: invalid  
       UINT8 pts_valid:1; // 1:valid, 0:invalid  
       UINT8 data_continue:1; // 1:continue, 0:not continue
       UINT8 ctrlblk_valid:1; // 1:valid, 0: invalid	
       UINT8 instant_update:1; //provided by dec, 1:need update instantly, 0: NOT	
       UINT8 vob_start: 1;
	UINT8 bstream_run_back:1;
	UINT8 reserve:1;
	UINT8 stc_id;
	UINT8 stc_offset_idx;
	UINT32 pts;
};

typedef enum Output_Frame_Ret_Code(* T_MPRequest)(struct Display_Info *);
typedef BOOL (* T_MPRelease)(UINT8, UINT8);
typedef enum Output_Frame_Ret_Code(* T_PIPRequest)(struct PIP_Dislay_Info *);
typedef BOOL (* T_PIPRelease)(UINT8);

typedef enum Output_Frame_Ret_Code(* T_Request_ext)(void *,void *);
typedef BOOL (* T_Release_ext)(void*,UINT8, UINT8);
//added for s3601
struct Release_Info
{
	UINT8	src_path;
	UINT8 	src_frm_type_index;
	UINT8	src_frm_index;
	struct FrameStore *fs;
};

struct Request_Info
{
	UINT8	src_path;
	UINT8	case_index;
	UINT8	drop_frame_count;   // if drop_frame_count > 0, vdec should drops some frame follows the current frame
	UINT8	reserved;           // reserved for the future
	BOOL      b_ve_adjust_FrameRate;//when 60frames/s to 50frames/s(or 50 fields/s) case,need to adjust frame rate convert in vdec module
};

struct vblanking_info
{
	UINT8	src_path;
	UINT8 	src_frm_type_index;
	UINT8	src_frm_index;	
};

typedef enum Output_Frame_Ret_Code(* T_Request)(void *,struct Display_Info *,struct Request_Info *);
typedef RET_CODE (* T_Release)(void *, struct Release_Info *);
typedef RET_CODE (* T_VBlanking)(void *, struct vblanking_info *);
typedef RET_CODE (* T_GEResize)(UINT32);
//add-end
struct MPSource_CallBack
{
	struct vdec_device *handler;
	T_MPRequest	RequestCallback;
	T_MPRelease	ReleaseCallback;
	T_VBlanking vblanking_callback;
};

struct PIPSource_CallBack
{
	T_PIPRequest	RequestCallback;
	T_PIPRelease	ReleaseCallback;
};
struct Source_CallBack_ext
{
	struct vdec_device *handler;
	T_Request_ext	RequestCallback;
	T_Release_ext	ReleaseCallback;
	T_VBlanking vblanking_callback;
};
struct source_callback
{
	struct vdec_device *handler;
	T_Request request_callback;
	T_Release release_callback;
	T_VBlanking vblanking_callback;
};

//end
//Steve add for connection between VBI & VPO
typedef void (* T_VBIRequest)(UINT8 FieldPolar);
//end

#define TTX_VBI	0
#define TTX_OSD	1
//added for s3601. information between DE and HDMI
struct s3601ToHdmi_video_infor
{
	enum TVSystem		tv_mode;
	UINT16			width;
	UINT16			height;
	enum PicFmt		format;
	BOOL			scan_mode;
	enum TVMode	output_aspect_ratio;

};

enum MHEG5SceneAR
{
    /* Set display aspect ratio to 16:9 - wide screen */
    SCENE_ASPECT_RATIO_16x9,

    /* Set display aspect ratio to 4:3 - normal TV mode  with CCO*/
    SCENE_ASPECT_RATIO_4x3_CCO,

    /* Set display aspect ratio to 4:3 - normal TV mode  with LB*/
    SCENE_ASPECT_RATIO_4x3_LB,

    /* Scene aspect ratio is not specified */
    SCENE_ASPECT_RATIO_UNSEPCIFIED
};

typedef void (*OSAL_T_LSR_PROC_FUNC_PTR)(UINT32);
typedef void (*OSAL_T_HSR_PROC_FUNC_PTR)(UINT32);

#define HLD_DEV_TYPE_TVE		0x01010000	/* TV encoder device */
#define HLD_DEV_TYPE_DMX		0x01020000	/* Stream de-multiplex device */
#define HLD_DEV_TYPE_OSD		0x01030000	/* OSD device */
#define HLD_DEV_TYPE_NET		0x01040000	/* Network device */
#define HLD_DEV_TYPE_NIM		0x01050000	/* NIM (Demodulator + tuner) */
#define HLD_DEV_TYPE_IRC		0x01060000	/* IR controler */
#define HLD_DEV_TYPE_PAN		0x01070000	/* Front panel device */
#define HLD_DEV_TYPE_SMC		0x01080000	/* Smart-card reader device */
#define HLD_DEV_TYPE_SND		0x01090000	/* Sound card device */
#define HLD_DEV_TYPE_DECA		0x010A0000	/* Audio decoder device */
#define HLD_DEV_TYPE_DECV		0x010B0000	/* Video decoder device */
#define HLD_DEV_TYPE_STO		0x010C0000	/* Character storage device */
#define HLD_DEV_TYPE_RFM		0x010D0000	/* RF modulator device */
#define HLD_DEV_TYPE_MST		0x010E0000	/* Block storage device */
#define HLD_DEV_TYPE_DIS		0x010F0000	/* Display device */
#define HLD_DEV_TYPE_CIC		0x01100000	/* DVB CI controler device */
#define HLD_DEV_TYPE_TTX		0x01110000	/* Teletext decoder */
#define HLD_DEV_TYPE_SDEC		0x01120000	/* Subtitle decoder */
#define HLD_DEV_TYPE_VBI		0x01130000	/* VBI decoder device */
#define HLD_DEV_TYPE_VENC		0x01140000	/* Video encoder */
#define HLD_DEV_TYPE_SGDMA		0x01150000	/* Scatter Gather DMA Engine */
#define HLD_DEV_TYPE_GE			0x01160000	/* Graphic Engine */
#define HLD_DEV_TYPE_HDMI		0x01170000	/* HDMI device */
#define HLD_DEV_TYPE_USB_HOST		0x01180000	/*USB Host Controller*/
#define HLD_DEV_TYPE_USB_HUB		0x01190000	/*USB HUB Device*/
#define HLD_DEV_TYPE_USB_MASS		0x011a0000	/*USB Mass Storage Device*/

#define HLD_DEV_TYPE_ACI		0x011b0000	/* ACI device */
#define HLD_DEV_TYPE_SCART		0x011c0000	/*Scart Switch device*/
#define HLD_DEV_TYPE_SD		0x011d0000	/*M33 SD device Controller*/

#define HLD_DEV_TYPE_DSC        0x011e0000   /*Des module*/
#define HLD_DEV_TYPE_CE         0x011f0000   /*CE module*/
#define HLD_DEV_TYPE_DES         0x01200000   /*CE module*/
#define HLD_DEV_TYPE_AES         0x01210000   /*CE module*/
#define HLD_DEV_TYPE_SHA         0x01220000   /*CE module*/
#define HLD_DEV_TYPE_CSA         0x01230000   /*CE module*/
#define HLD_DEV_TYPE_SATA         0x01240000   /*SATA Host*/
#define HLD_DEV_TYPE_GMA		0x01250000 /*Graphic Memory Access module*/
#define HLD_DEV_TYPE_NAND		0x01260000  /*nand flash*/
#define HLD_DEV_TYPE_WIFI		0x01270000  /* WiFi Network Device */

#define HLD_DEV_TYPE_PMU        0x01280000 /*PMU device*/
#define HLD_DEV_TYPE_AVSYNC		0x01290000  /* AV sync Device */

#define HLD_DEV_TYPE_MASK		0xFFFF0000	/* Device type mask */
#define HLD_DEV_ID_MASK			0x0000FFFF	/* Device id mask */

#define HLD_DEV_STATS_UP		0x01000001	/* Device is up */
#define HLD_DEV_STATS_RUNNING	0x01000002	/* Device is running */

#define HLD_MAX_DEV_NUMBER		64 		/* Max device number */
#define HLD_MAX_NAME_SIZE		16			/* Max device name length */

struct hld_device
{
	struct hld_device   *next;						/* Next device structure */
	UINT32			type;						/* Device type */
	INT8				name[HLD_MAX_NAME_SIZE];	/* Device name */
};

//#ifdef DUAL_ENABLE
struct remote_hld_device
{
	struct hld_device   *next;						/* Next device structure */
	UINT32				type;						/* Device type */
	INT8				name[HLD_MAX_NAME_SIZE];	/* Device name */
    	struct hld_device   *remote;						/* remote device structure */
};
//#endif

/*
  * this struct is used to save device IO parameter
  */
struct io_param
{
     UINT8 *io_buff_in;
     UINT32 buff_in_len;
     UINT8 *io_buff_out;
     UINT32 buff_out_len;
};

struct io_param_ex
{
     UINT8 *io_buff_in;
     UINT32 buff_in_len;
     UINT8 *io_buff_out;
     UINT32 buff_out_len;
	 UINT32 hnd;
	 UINT8 h264_flag;
	 UINT8 is_scrambled;
     UINT8 record_all;
};

#if 0
#ifdef __cplusplus
extern "C"
{
#endif

void *dev_get_by_name(INT8 *name);
void *dev_get_by_type(void *sdev, UINT32 type);
void *dev_get_by_id(UINT32 type, UINT16 id);
void dev_list_all(void *sdev);

void *dev_alloc(INT8 *name, UINT32 type, UINT32 size);
INT32 dev_register(void *dev);
void  dev_free(void *dev);

#ifdef SEE_CPU
void hld_cpu_callback(UINT32 para);
#endif

#ifdef __cplusplus
}
#endif
#endif

enum ALI_RPC_CB_TYPE
{
	ALI_RPC_CB_VDEC,
	ALI_RPC_CB_VPO,
	ALI_RPC_CB_VPO_HDMI,
	ALI_RPC_CB_SND_HDMI,
	ALI_RPC_CB_SNC_SPC,
	ALI_RPC_CB_IMG,
	ALI_RPC_CB_VDE,
	ALI_RPC_CB_MUS,
};

typedef void (*ali_rpc_cb_routine)(UINT32 type, UINT32 para);

void *hld_dev_get_by_name(INT8 *name);
void *hld_dev_get_by_type(void *sdev, UINT32 type);
void *hld_dev_get_by_id(UINT32 type, UINT16 id);
void ali_rpc_hld_base_callee(UINT8 *msg);
INT32 hld_dev_add_remote(struct hld_device *dev, UINT32 dev_addr);
INT32 hld_dev_remove_remote(struct hld_device *dev);
void hld_dev_memcpy(void *dest, const void *src, unsigned int len);
void hld_dev_see_init(void *addr);

void ali_rpc_register_callback(enum ALI_RPC_CB_TYPE type, void *cb_func);
UINT32 hld_get_see_version(UINT8 *dest);
void hld_disable_see_printf(unsigned long disable);
void hld_hit_see_heart(void);
void hld_enable_see_exception(void);
#endif
