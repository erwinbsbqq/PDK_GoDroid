 /**********************************************************************
 *
 *  Copyright (C) 2003 ALi (Shanghai) Corporation.  All Rights Reserved.
 *
 *  File:	mediatypes.h
 *
 *  Contents: definition of general A/V data structure of the system
 *			including
 *
 *			Position (x,y),
 *			Rectangle
 *
 *			Audio properties:   audio stream type,
 *							bit rate,
 *							sample frequency,
 *							quantization word length
 *
 *			Video properties:	video stream type
 *							TV system,
 *							frame rate,
 *							aspect ratio,
 *							picture format,
 *							YUV	color,
 *							YCbCr address,
 *							highlight information
 *
 * 			MPEG-1/2 basic syntax/semantic unit :  	SCR & PTS,
 *												start code ,
 *												stream/sub-stream ID
 *			DVD/VCD PE   	A/V settings
 *
 *  History:
 *   Date                  By                Reason
 *   =========    =======    ====================
 *   10/17/2003	  Mengdong Lin       create
 *
 **********************************************************************/
#ifndef 	_MEDIA_TYPES_H_
#define 	_MEDIA_TYPES_H_

#include <basic_types.h>
//corei#include <ali_video_common.h>
#include <alidefinition/adf_media.h>

#if 0    //corei
//#include <alitypes.h>
#include <sys_config.h>
#include <basic_types.h>
#include <ali_video_common.h>

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


/********************************************************************
							audio properties
*********************************************************************/
#define MAX_AUDIO_CH_NUM  8 //maximum audio channel number




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

//highlight information
struct HLInfo
{
	UINT32	uColor;	// Selection color or action color code definition:reference DVD-VIDEO P VI4-122
	UINT32	uStartTime;	// start pts of hight light.
	struct Rect	rect;	// position and size of this hightlight area.
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

/*
enum AspRatio 
{
	DAR_FORBIDDEN, 
	DAR_4_3,
	DAR_16_9,
};
*/

#define	VIDEO_OPEN_WIN_FLAG			0x01
#define	VIDEO_RESET_SRC_DST_FLAG		0x02
#define VIDEO_INTO_SML_MODE             0x04
#define VIDEO_LEAVE_SML_MODE            0x08
#define	VIDEO_FIRST_PIC_SYNC_FLAG	0x10
#define VIDEO_OPEN_PIP_WIN_FLAG     0X80

/***Add some definition for STB solution***/
struct control_block{
       UINT8 stc_id_valid:1; // 1: valid, 0: invalid  
       UINT8 pts_valid:1; // 1:valid, 0:invalid  
       UINT8 data_continue:1; // 1:not continue, 0: continue
       UINT8 ctrlblk_valid:1; // 1:valid, 0: invalid	
       UINT8 instant_update:1; //provided by dec, 1:need update instantly, 0: NOT	
       UINT8 vob_start: 1;
	UINT8 reserve:2;
	UINT8 stc_id;
	UINT32 pts;
};

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

#endif

#endif

