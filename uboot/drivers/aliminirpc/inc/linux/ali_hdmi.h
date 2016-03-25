/***************************************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:    
*		ali_hdmi.h
*
*    Description:    
*		This file define data type for hdmi driver processing.
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#ifndef	_ALI_HDMI_H_
#define	_ALI_HDMI_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ali_video_types.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <rpc_hld/ali_rpc_hld_dis.h>
#include <rpc_hld/ali_rpc_hld_snd.h>
#include <rpc_hld/ali_rpc_hld_gma.h>

//#include "../../ETree/inc/bus/hdmi/m36/hdmi_api.h"

#define ALI_HDMI_MSG_EDID		1
#define ALI_HDMI_MSG_PLUGIN		2
#define ALI_HDMI_MSG_PLUGOUT	3
#define ALI_HDMI_MSG_CEC		4

/**@brief HDMI Link Status*/
typedef enum
{
	HDMI_STATUS_UNLINK,				/**< HDMI not link */
	HDMI_STATUS_LINK_HDCP_SUCCESSED,
	HDMI_STATUS_LINK_HDCP_FAILED,
	HDMI_STATUS_LINK_HDCP_IGNORED,
	HDMI_STATUS_MAX,
}HDMILinkStatus_E;

typedef enum
{
//SD
	TV_MODE_AUTO = 0,	// switch by source
	TV_MODE_PAL,
	TV_MODE_PAL_M,		// PAL3.58
	TV_MODE_PAL_N,
	TV_MODE_NTSC358,	// NTSC3.58
	TV_MODE_NTSC443,
	TV_MODE_SECAM,
//HD
	TV_MODE_576P,
	TV_MODE_480P,
	TV_MODE_720P_50,
	TV_MODE_720P_60,
	TV_MODE_1080I_25,
	TV_MODE_1080I_30,
	TV_MODE_1080P_50,
	TV_MODE_1080P_60,	
	TV_MODE_1080P_25,
	TV_MODE_1080P_30,	
	TV_MODE_1080P_24,
}TV_OUT_MODE;

typedef struct
{    
	TV_OUT_MODE					resolution;         // Output Resolution
	enum PicFmt					color_format;       // Color Format
	enum TVMode					aspect_ratio;       // 4:3, 16:9
 	bool						afd_present;        //
	unsigned char				afd;                //
//	enum 	EXT_VIDEO_FORMAT	ext_video_format;	//add for 3d function
//	enum	_3D_STRUCTURE		_4K_VIC_3D_structure;
//	unsigned char				_3D_ext_data;
} FB2HDMI_INFO;

enum AUDIO_USER_SETTING{
	AUD_USR_BITSTREAM_OUT       = 0,  
	AUD_USR_LPCM_OUT            = 1,     
	AUD_USR_AUTO                = 2, // By EDID
};

enum AUDIO_CODING_TYPE{
	AUD_CODING_PCM 				= 0x01,
	AUD_CODING_AC3 				= 0x02,
	AUD_CODING_MPEG1_L12 		= 0x03,
	AUD_CODING_MPEG1_L3 		= 0x04,
	AUD_CODING_MEPG2	  		= 0x05,
	AUD_CODING_AAC				= 0x06,
	AUD_CODING_DTS				= 0x07,
	AUD_CODING_ATRAC			= 0x08,
	AUD_CODING_ONEBITAUDIO		= 0x09,
	AUD_CODING_DD_PLUS			= 0x0A,
	AUD_CODING_DTS_HD			= 0x0B,
	AUD_CODING_MAT_MLP			= 0x0C,
	AUD_CODING_DST				= 0x0D,	
	AUD_CODING_WMAPRO			= 0x0E,
};

enum AUDIO_SAMPLE_RATE{
    AUD_SAMPLERATE_32KHZ        = 0,  
	AUD_SAMPLERATE_44_1KHZ      = 1, 
	AUD_SAMPLERATE_48KHZ        = 2,	
	AUD_SAMPLERATE_88_2KHZ      = 3,
	AUD_SAMPLERATE_96KHZ        = 4,	
    AUD_SAMPLERATE_176_4KHZ     = 5,  
	AUD_SAMPLERATE_192KHZ       = 6,
	AUD_SAMPLERATE_768KHZ       = 7,
	AUD_SAMPLERATE_UNKNOWN      = 8,	
};

enum I2S_FMT_TYPE{
	I2S_FMT_LEFT_JUSTIFIED      = 0,  
	I2S_FMT_RIGHT_JUSTIFIED     = 1,     
	I2S_FMT_I2S	                = 2,
};

enum I2S_FMT_WORD_LENGTH{
	I2S_FMT_WLENGTH_24          = 0,  
	I2S_FMT_WLENGTH_16          = 1,     
	I2S_FMT_WLENGTH_28          = 2, // One-bit audio
};

//****************** HDMI deep color definition ***********************
enum HDMI_API_DEEPCOLOR
{	
	HDMI_DEEPCOLOR_24 = 0,	
	HDMI_DEEPCOLOR_30,	
	HDMI_DEEPCOLOR_36,	
	HDMI_DEEPCOLOR_48,
};

typedef union
{    
    struct
    {
        unsigned int    profession_flag     :1;      // [0]		Professional flag
        unsigned int    audio_content_flag  :1;      // [1]		Audio content flag
        unsigned int    copyright           :1;      // [2]		Copyright
        unsigned int    emphasis            :3;      // [5:3]	Emphasis
        unsigned int    mode                :2;      // [7:6]	Mode
        unsigned int    l_and_category      :8;      // [15:8]	L & Category
        unsigned int    source_number       :4;      // [19:16]	Source Numberg
        unsigned int    channel_number      :4;      // [23:20]	Channel Number
        unsigned int    sample_rate         :4;      // [27:24]	Sample rate
        unsigned int    clock_accuracy      :2;      // [29:28]	Clock Accuracy        
        unsigned int    reserved            :2;      // [31:30]	Reserved      	   
    };
    unsigned int data_uint32;
    unsigned char databyte[4];

} I2S_CHANNEL_STATUS;
    
typedef struct
{    
    enum AUDIO_USER_SETTING     user_audio_out;		// User Setting 2: Auto (By EDID) 1: PCM 0: BIT STREAM
	enum AUDIO_CODING_TYPE      coding_type;        // SPO buffer bit stream coding type
	unsigned int                channel_count;		// 2, 6(5.1), 8(down-mixed + 5.1 channel)	    
    enum AUDIO_SAMPLE_RATE      sample_rate;		// 48000, 44100, 32000 ...
    
	enum I2S_FMT_TYPE           i2s_format;	        // I2S, Left Justify and Right Justify			
	bool                        lrck_hi_left;		// true: lrck high send left channel, false: otherwise
	enum I2S_FMT_WORD_LENGTH    word_length;		// 16bits, 24bits, or 28bits(one-bit audio)
    I2S_CHANNEL_STATUS          channel_status;	
} ALSA2HDMI_INFO;

enum HDMI_API_RES
{
	HDMI_RES_INVALID = 0,
	HDMI_RES_480I,
	HDMI_RES_480P,
	HDMI_RES_576I,
	HDMI_RES_576P,
	HDMI_RES_720P_50,
	HDMI_RES_720P_60,
	HDMI_RES_1080I_25,
	HDMI_RES_1080I_30,	
	HDMI_RES_1080P_24,
	HDMI_RES_1080P_25,
	HDMI_RES_1080P_30,
	HDMI_RES_1080P_50,
	HDMI_RES_1080P_60,	
};

enum HDMI_AV_CHG_STE
{
	HDMI_CB_NOTHING 	 = 0x00,
	HDMI_CB_CLK_RDY2CHG  = 0x01,
	HDMI_CB_AV_INFO_CHG  = 0x02,
	HDMI_CB_CLK_CHG_DONE = 0x04,
	HDMI_CB_CLR_RDY2CHG  = 0x08,
};

typedef struct de2Hdmi_video_infor
{
	enum TVSystem			    tv_mode;
	UINT16					    width;
	UINT16					    height;
	enum PicFmt				    format;
	BOOL					    scan_mode;
	BOOL					    afd_present;
	enum TVMode			        output_aspect_ratio;
	UINT8					    active_format_aspect_ratio;
	enum HDMI_AV_CHG_STE	    av_chg_ste;
}DE2HDMI_VIDINF;

typedef struct snd2Hdmi_audio_infor
{
	/* INFO by user setting */
	UINT32 user_def_ch_num;				/* 2 or 8								*/
	UINT32 pcm_out;						/* 2:AUTO  1: PCM 0: BIT STREAM 		*/

	/* AUDIO stream status */
	enum AUDIO_CODING_TYPE coding_type;
	UINT32 max_bit_rate;				/* maximum bit rate 					*/
	UINT32 ch_count;					/* 2, 6(5.1), 8							*/
	UINT32 sample_rate;					/* 48000, 44100, 32000 etc				*/
	UINT32 level_shift;					/* level shift after down-mixing		*/

	/* S/PDIF config dynamic setting */
	UINT32 spdif_edge_clk;				/*0: rising edge latch data, 1: falling edge latch data	*/

	/* I2S config dynamic setting */
	UINT32 ch_status;					/*	31:30	Reserved
											29:28	Clock Accuracy
											27:24	Sample rate
											23:20	Channel Number
											19:16	Source Number
											15:8	L & Category
											7:6		Channel Status Mode
											5:3		Emphasis;
											2		Copyright Information; 0: has copyright; 1: no copyright
											1		Audio content flag; 0:audio PCM; 1: not audio PCM 
											0		Professional flag;  0: consumer mode; 1: professional mode */
	UINT8   ch_position[8];				/* ch_position[i] 
										bit0~bit3: speaker(CH_FL, CH_FC etc)
										bit4~bit6: channel position(0~7, the position in I2S dma buffer)
										bit7: speaker enable(1:enable, 0:disable)				*/
											  
	/* I2S config fixed setting */
	UINT32 bclk_lrck;					/* 32, 64												*/
	UINT32 word_length;					/* 16bits, 24bits										*/
	UINT32 i2s_edge_clk;				/* 0:rising edge latch data, 1:falling edge latch data	*/
	enum I2S_FMT_TYPE i2s_format;		/* I2S, Left Justify and Right Justify					*/
	UINT32 lrck_hi_left;				/* 1: lrck high for left channel, 0: reverse			*/
	enum HDMI_AV_CHG_STE	av_chg_ste;
}SND2HDMI_AUDINF;

extern void ali_hdmi_set_video_info(FB2HDMI_INFO* fb2hdmi_info );
extern void ali_hdmi_set_audio_info(ALSA2HDMI_INFO* alsa2hdmi_info );
extern void set_video_info_to_hdmi(UINT32 param);
extern void set_audio_info_to_hdmi(UINT32 param);
extern void ali_hdmi_set_bootloader_param(void *pParam);
extern void ali_hdmi_set_module_init_hdcp_onoff(bool bOnOff);
extern bool ali_hdmi_get_hdcp_onoff(void);
extern void ali_hdmi_set_hdcp_onoff(bool bOnOff);
extern bool ali_hdmi_get_cec_onoff(void);
extern void ali_hdmi_set_cec_onoff(bool bOnOff);
extern bool ali_hdmi_set_logical_address(unsigned char logical_address);
extern unsigned char ali_hdmi_get_logical_address(void);
extern bool ali_hdmi_cec_transmit(unsigned char* message, unsigned char message_length);
extern void ali_hdmi_audio_switch_onoff(bool turn_on);

#endif

