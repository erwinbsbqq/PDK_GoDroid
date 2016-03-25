/***************************************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_infoframe.h
*
*    Description:
*		This file contains all read/write functions in register level with HDMI
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#ifndef	_HDMI_INFOFRAME_H_
#define	_HDMI_INFOFRAME_H_

#pragma pack (1)
/*******************************************************************
*						InfoFrame Header						   *
*******************************************************************/
enum HDMI_PKT_IFM_TYPE
{
	VSI_IFM_TYPE	 		= 0x81,
	AVI_IFM_TYPE	 		= 0x82,
	SPD_IFM_TYPE 			= 0x83,
	AUDIO_IFM_TYPE 			= 0x84,
	MPEG_IFM_TYPE 			= 0x85,
	GCP_TYPE				= 0x03,
};

enum HDMI_PKT_IFM_VER
{
	VSI_IFM_VER 			= 0x01,
	AVI_IFM_VER 			= 0x02,
	SPD_IFM_VER 			= 0x01,
	AUDIO_IFM_VER 			= 0x01,
	MPEG_IFM_VER 			= 0x01,
	GCP_VER				= 0x00
};

enum HDMI_PKT_IFM_LEN
{
	GENERIC_IFM_LEN			= 0x1C,	// Length = 28 Bytes
	VSI_IFM_LEN			=0x05,	// Length = 28 Bytes, 3921 temp
	AVI_IFM_LEN 			= 0x0D,	// Length = 13 Bytes
	SPD_IFM_LEN 			= 0x19,	// Length = 25 Bytes
	AUDIO_IFM_LEN 			= 0x0A,	// Length = 10 Bytes
	MPEG_IFM_LEN 			= 0x0A,	// Length = 10 Bytes
	GCP_LEN				= 0x07,
};

/*******************************************************************
*		Each parameter definition for VSI InfoFrame 			   *
*******************************************************************/
typedef struct{
	unsigned char check_sum;			// PB0
	unsigned char IEEE_reg_id[3];		// PB1, PB2, PB3, 24bits IEEE Registeration Identifier(0x000C03)
	unsigned char ext_video_format;		// PB4[7:5], [3bits] extended video formats.
	unsigned char _4K_VIC_3D_structure;		// (PB5) HDMI proprietary Video Format Identification Code.
	unsigned char ext_data;			// (PB6)depends on the 3D_Structure value.
	unsigned char reserve[22];			// depends on the 3D_Structure value.
}VSI_PAYLOAD;

enum VSI_3D_STRUCTURE
{
	VSI_FRAME_PACKING	    = 0,
	VSI_FIELD_ALTERNATIVE   = 1,
	VSI_LINE_ALTERNATIVE    = 2,
	VSI_SIDE_BY_SIDE_FULL   = 3,
	VSI_L_DEPTH             = 4,
	VSI_L_DEPTH_GRAPHICS    = 5,
	VSI_TOP_AND_BOTTOM      = 6,
	VSI_SIDE_BY_SIDE_HALF   = 8,
};

enum VSI_EXT_VIDEO_FORMAT
{
	VSI_NO_ADDITIONAL_VIDEO_FORMAT_PRESENTED	= 0,
	VSI_EXT_RESOLUTION_FORMAT_PRESENTED			= 1,
	VSI_3D_FORMAT_INDICATION_PRESENTED			= 2,
};

enum VSI_EXT_DATA_FIELD
{
	VSI_HORIZONTAL_SUB_SAMPLING	= 0,
	VSI_ODD_LEFT_ODD_RIGHT	= 4,
	VSI_ODD_LEFT_EVEN_RIGHT	= 5,
	VSI_EVEN_LEFT_ODD_RIGHT	= 6,
	VSI_EVEN_LEFT_EVEN_RIGHT	= 7,
};

/*******************************************************************
*		Each parameter definition for AVI InfoFrame 			   *
*******************************************************************/
typedef struct
{
	unsigned char check_sum;							// PB0
	unsigned char scan_information				:2;		// PB1[1:0] S1-S0	Scan Information
	unsigned char bar_information				:2;		// PB1[3:2] B1-B0	Bar Info
	unsigned char acitve_format_info_present	:1;		// PB1[4] 	A0		Active Format Information Present
	unsigned char rgb_ycbcr						:2;		// PB1[6:5] Y1-Y0	RGB/YCbCr
	unsigned char reserved_1					:1; 	// PB1[7]	F17		Future Use, All zeros
	unsigned char active_format_aspect_ratio	:4;		// PB2[3:0] R3-R0	Active Format Aspect Ratio
	unsigned char picture_aspect_ratio			:2; 	// PB2[5:4] M1-M0	Picture Aspect Ratio
	unsigned char colorimetry					:2; 	// PB2[7:6] C1-C0	Colormetry
	unsigned char non_uniform_picture_scaling	:2; 	// PB3[1:0] SC1-SC0	Non-Uniform Picture Scaling
	unsigned char rgb_quantization_range		:2; 	// PB3[3:2] Q1-Q0	RGB Quantization Range
	unsigned char extended_colorimetry			:3; 	// PB3[6:4] EC2-EC0 Extended Colorimetry
	unsigned char it_content					:1; 	// PB3[7]   ITC		IT Content
	unsigned char video_identfy_code;					// PB4		VIC		Video Identification Code.
	unsigned char pixel_repetition_factor		:4;		// PB5[3:0]	PR3-PR0	Pixel Repetition Factor
	unsigned char reserved_2					:4;		// PB5[7:4]	F57-F54	Future Use, All zeros
	unsigned char ln_etb_lower;							// PB6	Line Number of End of Top Bar (lower 8 bits)
	unsigned char ln_etb_upper;							// PB7	Line Number of End of Top Bar (upper 8 bits)
	unsigned char ln_sbb_lower;							// PB8	Line Number of Start of Bottom Bar (lower 8 bits)
	unsigned char ln_sbb_upper;							// PB9	Line Number of Start of Bottom Bar (upper 8 bits)
	unsigned char pn_elb_lower;							// PB10	Pixel Number of End of Left Bar (lower 8 bits)
	unsigned char pn_elb_upper;							// PB11	Pixel Number of End of Left Bar (upper 8 bits)
	unsigned char pn_srb_lower;							// PB12	Pixel Number of Start of Right Bar (lower 8 bits)
	unsigned char pn_srb_upper;							// PB13	Pixel Number of Start of Right Bar (upper 8 bits)
}AVI_PAYLOAD;

// PB1[1:0] S1-S0	Scan Information
enum AVI_IFM_SCAN_INFO
{
	SCAN_INFO_NO_DATA 		= 0x00, 	// No Data
	SCAN_INFO_OVERSCANED 	= 0x01,		// Composed for an overscanned display, where some active pixels & lines at the edges are not displayed.
	SCAN_INFO_UNDERSCANNED 	= 0x02,		// Composed for an underscanned display, where all active pixels & lines are displayed.
	SCAN_INFO_FUTURE 		= 0x03,
};

// PB1[3:2] B1-B0	Bar Info
enum AVI_IFM_BAR_INFO
{
	BAR_INFO_NOT_VALID 			= 0x00,	// Bar Data not valid
	VERT_BAR_INFO_VALID			= 0x01,	// Vert. Bar Info valid
	HORIZ_BAR_INFO_VALID		= 0x02, // Horiz. Bar Info valid
	VERT_HORIZ_BAR_INFO_VALID	= 0x03, // Vert. and Horiz. Bar Info valid
};

// PB1[4] 	A0		Active Format Information Present
enum AVI_IFM_AFI_PRESENT
{
	AFI_PRESENT_NO_DATA 		= 0x00,	// No Data
	AFI_PRESENT_VALID			= 0x01,	// Active Format (R3-R0) Information valid
};

// PB1[6:5] Y1-Y0	RGB/YCbCr
enum AVI_IFM_FORMAT
{
	AVI_FORMAT_RGB 				= 0x00,	// RGB (default)
	AVI_FORMAT_YCBCR_422		= 0x01,	// YCbCr 4:2:2
	AVI_FORMAT_YCBCR_444		= 0x02,	// YCbCr 4:4:4
	AVI_FORMAT_FUTURE			= 0x03,	// Future
};

// PB2[3:0] R3-R0	Active Format Aspect Ratio
enum AVI_IFM_AFD
{
	AFD_16_9_TOP 				= 0x02, // ETSI: 16:9 active picture (top aligned)
	AFD_14_9_TOP	 			= 0x03,	// ETSI: 14:9 active picture (top aligned)
	AFD_GREATER_16_9_CENTER		= 0x04, // ETSI: box > 16:9 (center): wider than 16:9 active picture.
										// The aspect ratio of the source area is not given, and the size of the top/bottom bars is not indicated.
	AFD_AS_CODE_FRAME			= 0x08,	// Full Frame image, same as the frame (4:3 or 16:9). Same as picture aspect ratio
	AFD_4_3_CENTER				= 0x09,	// 4:3 Image: Full Frame in 4:3 frame, Pillarbox in 16:9 frame. 4:3 (Center)
	AFD_16_9_CENTER				= 0x0A, // 16:9 Image: Letterbox in 4:3 frame, Full Frame in 16:9 frame. 16:9 (Center)
	AFD_14_9_CENTER				= 0x0B, // 14:9 Pillarbox/Letterbox image. 14:9 (Center)
	AFD_4_3						= 0x0D, // 4:3 with shoot and protect 14:9 centre.
	AFD_16_9_PROTECT_14_9		= 0x0E, // 16:9 with shoot and protect 14:9 centre.
	AFD_16_9_PROTECT_4_3		= 0x0F, // 16:9 with shoot and protect 4:3 centre.
};

// PB2[5:4] M1-M0	Picture Aspect Ratio
enum AVI_IFM_PICTURE_ASPECT_RATIO
{
	AR_NO_DATA 					= 0x00,	// No Data
	AR_4_3 						= 0x01,	// 4:3
	AR_16_9 					= 0x02,	// 16:9
	AR_FUTURE					= 0x03,	// Future
};

// PB2[7:6] C1-C0	Colormetry
enum AVI_IFM_COLORIMETRY
{
	COLORIMETRY_NO_DATA 			= 0x00,	// No Data
	COLORIMETRY_ITU601 			    = 0x01,	// SMPTE170M/ITU601 481i(p),576i(p),240p and 288p
	COLORIMETRY_ITU709 				= 0x02,	// ITU709 1080i(p) AND 720p
	COLORIMETRY_EXT_CLRMETRY_VALID 	= 0x03, // Extended Colorimetry Information Valid (EC2-EC0)
};

// PB3[1:0] SC1-SC0	Non-Uniform Picture Scaling
enum AVI_IFM_SCALING
{
	NO_KNOWN_SCALING 			= 0x00,	// No Known non-uniform Scaling
	HORIZ_SCALED 				= 0x01,	// Picture has been scaled horizontally
	VERT_SCALED 				= 0x02, // Picture has been scaled vertically
	HORIZ_VERT_SCALED 			= 0x03, // Picture has been scaled horizontally and vertically
};

// PB3[3:2] Q1-Q0	RGB Quantization Range
enum AVI_IFM_RGB_QUANT_RANGE
{
	RGB_QUANT_RANGE_DEFAULT 	= 0x00,	// Default (depends on video format)
	RGB_QUANT_RANGE_LIMITED 	= 0x01,	// Limited Range (220 levels, 16 to 235)
	RGB_QUANT_RANGE_FULL 		= 0x02,	// Full Range (256 levels, 0 to 255)
	RGB_QUANT_RANGE_RESERVED	= 0x03,	// Reserved
};

// PB3[6:4] EC2-EC0 Extended Colorimetry
enum AVI_IFM_EXT_CLRMETRY
{
	EXT_CLRMETRY_XVYCC601 		= 0x00,	// xvYCC601
	EXT_CLRMETRY_XVYCC709 		= 0x01,	// xvYCC709
	EXT_CLRMETRY_SYCC601 		= 0x02,	// sYCC601
	EXT_CLRMETRY_ADOBEYCC601	= 0x03,	// AdobeYCC601
	EXT_CLRMETRY_ADOBERGB		= 0x04,	// AdobeRGB
};

// PB3[7]   ITC		IT Content
enum AVI_IFM_IT
{
	IT_NO_DATA 		= 0x00,	//
	IT_CONTENT 		= 0x01,	//
};

// PB4		VIC		Video Identification Code.
enum VIDEO_ID_CODE
{
	VIC_640x480P_60HZ 			= 1,
	VIC_720x480P_60HZ_4_3		= 2,
	VIC_720x480P_60HZ_16_9		= 3,
	VIC_1280x720P_60HZ			= 4,
	VIC_1920x1080I_60HZ			= 5,
	VIC_1440x480I_60HZ_4_3		= 6,
	VIC_1440x480I_60HZ_16_9		= 7,
	VIC_1440x240P_60HZ_4_3		= 8,
	VIC_1440x240P_60HZ_16_9		= 9,
	VIC_2880x480I_60HZ_4_3		= 10,
	VIC_2880x480I_60HZ_16_9		= 11,
	VIC_2880x240P_60HZ_4_3		= 12,
	VIC_2880x240P_60HZ_16_9		= 13,
	VIC_1440x480P_60HZ_4_3		= 14,
	VIC_1440x480P_60HZ_16_9		= 15,
	VIC_1920x1080P_60HZ			= 16,
	VIC_720x576P_50HZ_4_3		= 17,
	VIC_720x576P_50HZ_16_9		= 18,
	VIC_1280x720P_50HZ			= 19,
	VIC_1920x1080I_50HZ			= 20,
	VIC_1440x576I_50HZ_4_3		= 21,
	VIC_1440x576I_50HZ_16_9		= 22,
	VIC_1440x288P_50HZ_4_3		= 23,
	VIC_1440x288P_50HZ_16_9		= 24,
	VIC_2880x576I_50HZ_4_3		= 25,
	VIC_2880x576I_50HZ_16_9		= 26,
	VIC_2880x288P_50HZ_4_3		= 27,
	VIC_2880x288P_50HZ_16_9		= 28,
	VIC_1440x576P_50HZ_4_3		= 29,
	VIC_1440x576P_50HZ_16_9		= 30,
	VIC_1920x1080P_50HZ			= 31,
	VIC_1920x1080P_24HZ			= 32,
	VIC_1920x1080P_25HZ			= 33,
	VIC_1920x1080P_30HZ			= 34,
	VIC_2880x480P_60HZ_4_3		= 35,
	VIC_2880x480P_60HZ_16_9		= 36,
	VIC_2880x576P_50HZ_4_3		= 37,
	VIC_2880x576P_50HZ_16_9		= 38,
	VIC_1920x1080I_100HZ		= 40,
	VIC_1280x720P_100HZ			= 41,
	VIC_720x576P_100HZ_4_3		= 42,
	VIC_720x576P_100HZ_16_9		= 43,
	VIC_1440x576I_100HZ_4_3		= 44,
	VIC_1440x576I_100HZ_16_9	= 45,
	VIC_1920x1080I_120HZ		= 46,
};

// PB5[3:0]	PR3-PR0	Pixel Repetition Factor
enum AVI_IFM_PIX_REP
{
	PIX_REP_PIX_NO_REPETITION 		= 0x00,	//
	PIX_REP_PIX_SENT_TWO_TIMES		= 0x01,	//
	PIX_REP_PIX_SENT_THREE_TIMES	= 0x02,	//
};
/*******************************************************************
*		Each parameter definition for Audio InfoFrame 			   *
*******************************************************************/
typedef struct
{
	unsigned char check_sum;							// PB0
	unsigned char audio_channel_count			:3;		// PB1[2:0]	CC2-CC0	Audio Channel Count
	unsigned char reserved_1					:1;		// PB1[3]	F13		Future Use, All zeros
	unsigned char audio_coding_type				:4;		// PB1[7:4]	CT3-CT0	Audio Coding Type
	unsigned char sample_size					:2;		// PB2[1:0]	SS1-SS0	Sample Size
	unsigned char sampling_frequency			:3;		// PB2[4:2]	SF2-SF0	Sampling Frequency
	unsigned char reserved_2					:3;		// PB2[7:5]	F27-F25	Future Use, All zeros
	unsigned char reserved_3;							// PB3		F37-F30 Future Use, All zeros
	unsigned char speaker_channel_allocation;			// PB4		CA7-CA0 Speaker Allocation
	unsigned char reserved_4					:3;		// PB5[2:0]	F52-F50 Future Use, All zeros
	unsigned char level_shift_value				:4;		// PB5[6:3]	LSV3-LSV0 Level Shift Value
	unsigned char down_mix_inhibit_flag			:1;		// PB5[7]	DMINH	Describes whether the down mixed stereo output is permitted or not.
	unsigned char reserved_5;							// PB6		F67-F60 Future Use, All zeros
	unsigned char reserved_6;							// PB7		F77-F70 Future Use, All zeros
	unsigned char reserved_7;							// PB8		F87-F80 Future Use, All zeros
	unsigned char reserved_8;							// PB9		F97-F90 Future Use, All zeros
	unsigned char reserved_9;							// PB10		F107-F100 Future Use, All zeros
}AUDIO_PAYLOAD;

// PB1[2:0]	CC2-CC0	Audio Channel Count
enum AUD_IFM_CH_COUNT
{
	CH_CNT_REF_STREAM 	= 0x00,		// Refer to Stream Header
	CH_COUNT_2CH 		= 0x01,		// 2ch
	CH_COUNT_3CH 		= 0x02,		// 3ch
	CH_COUNT_4CH 		= 0x03,		// 4ch
	CH_COUNT_5CH 		= 0x04,		// 5ch
	CH_COUNT_6CH 		= 0x05,		// 6ch
	CH_COUNT_7CH		= 0x06,		// 7ch
	CH_COUNT_8CH 		= 0x07,		// 8ch
};

// PB1[7:4]	CT3-CT0	Audio Coding Type
enum AUD_IFM_CODING_TYPE
{
	AUD_CODE_CT_REF_STREAM		= 0x00,		// Refer to Stream Header
	AUD_CODE_IEC60958_PCM		= 0x01,		// IEC 60958 PCM
	AUD_CODE_AC3				= 0x02,		// AC-3
	AUD_CODE_MPEG1_LAYER_1_2	= 0x03,		// MPEG1 (Layer 1&2)
	AUD_CODE_MPEG1_LAYER_3		= 0x04,		// MP3 (MPEG1 Layer3)
	AUD_CODE_MPEG2_MULTI_CH		= 0x05,		// MPEG2 (multichannel)
	AUD_CODE_AAC				= 0x06,		// AAC
	AUD_CODE_DTS				= 0x07,		// DTS
	AUD_CODE_ATRAC				= 0x08,		// ATRAC
	AUD_CODE_OneBitAudio		= 0x09,		// One Bit Audio
	AUD_CODE_DD_PLUS			= 0x0A,		// Dolby Digital +
	AUD_CODE_DTS_HD				= 0x0B,		// DTS-HD
	AUD_CODE_MAT				= 0x0C,		// MAT (MLP)
	AUD_CODE_DST				= 0x0D,		// DST
	AUD_CODE_WMA_PRO			= 0x0E,		// WMA Pro
	AUD_CODE_RESERVED			= 0x0F,		// Reserved
};

// PB2[1:0]	SS1-SS0	Sample Size
enum AUD_IFM_SAMPLE_SIZE
{
	SAMPLE_SIZE_REF_STREAM 		= 0x00,		// Refer to Stream Header
	WORD_LENGTH_16BIT			= 0x01,		// 16 bit
	WORD_LENGTH_20BIT			= 0x02,		// 20 bit
	WORD_LENGTH_24BIT			= 0x03,		// 24 bit
};

// PB2[4:2]	SF2-SF0	Sampling Frequency
enum AUD_IFM_SAMPLE_FREQ
{
	SAMPLE_FREQ_REF_STREAM 		= 0x00,		// Refer to Stream Header
	SAMPLE_FREQ_32000 			= 0x01,		// 32KHz
	SAMPLE_FREQ_44100 			= 0x02,		// 44.1KHz (CD)
	SAMPLE_FREQ_48000 			= 0x03,		// 48KHz
	SAMPLE_FREQ_88200 			= 0x04,		// 88.2KHz
	SAMPLE_FREQ_96000 			= 0x05,		// 96KHz
	SAMPLE_FREQ_17640 			= 0x06,		// 176.4KHz
	SAMPLE_FREQ_19200 			= 0x07,		// 192KHz
};

// PB4		CA7-CA0 Speaker Allocation
/*
 * CEA speaker placement:
 *
 *        	FLH       FCH        FRH
 *  FLW		FL	FLC   FC   FRC   FR   FRW
 *
 *                    LFE
 *                    TC
 *
 *          RL  RLC   RC   RRC   RR
 *
 * The Left/Right Surround channel _notions_ LS/RS in SMPTE 320M corresponds to
 * CEA RL/RR; The SMPTE channel _assignment_ C/LFE is swapped to CEA LFE/FC.
 */
enum cea_speaker_placement {
	FL  = (1 <<  0),	/* Front Left           */
	FC  = (1 <<  1),	/* Front Center         */
	FR  = (1 <<  2),	/* Front Right          */
	FLC = (1 <<  3),	/* Front Left Center    */
	FRC = (1 <<  4),	/* Front Right Center   */
	RL  = (1 <<  5),	/* Rear Left            */
	RC  = (1 <<  6),	/* Rear Center          */
	RR  = (1 <<  7),	/* Rear Right           */
	RLC = (1 <<  8),	/* Rear Left Center     */
	RRC = (1 <<  9),	/* Rear Right Center    */
	LFE = (1 << 10),	/* Low Frequency Effect */
};

// PB5[6:3]	LSV3-LSV0 Level Shift Value
enum AUD_IFM_LEVEL_SHIFT_VAL
{
	LSV_0dB = 0x00,
	LSV_1dB,
	LSV_2dB,
	LSV_3dB,
	LSV_4dB,
	LSV_5dB,
	LSV_6dB,
	LSV_7dB,
	LSV_8dB,
	LSV_9dB,
	LSV_10dB,
	LSV_11dB,
	LSV_12dB,
	LSV_13dB,
	LSV_14dB,
	LSV_15dB
};

// PB5[7]	DMINH	Describes whether the down mixed stereo output is permitted or not.
enum AUD_IFM_DM_INHIBIT
{
	DOWN_MIX_PERMITTED 	= 0x00,	// Permitted or no information about any assertion of this
	DOWN_MIX_PROHIBITED = 0x01, // Prohibited
};

/*******************************************************************
*		Each parameter definition for MPEG InfoFrame 			   *
*******************************************************************/
typedef struct
{
	unsigned char check_sum;							// PB0
	unsigned char mb_0;									// PB1 		MB#0 	MPEG Bit Rate: Hz Lower Byte
	unsigned char mb_1;									// PB2 		MB#1
	unsigned char mb_2;									// PB3 		MB#2
	unsigned char mb_3;									// PB4 		MB#3 	MPEG Bit Rate: Hz Upper Byte
	unsigned char mpeg_frame					:2;		// PB5[1:0]	MF1-MF0	MPEG Frame
	unsigned char reserved_1					:2;		// PB5[3:2]	F53-F52	Future Use, All zeros
	unsigned char field_repeat					:1;		// PB5[4]	FR0		Field Repeat
	unsigned char reserved_2					:3;		// PB5[7:5]	F57-F55	Future Use, All zeros
	unsigned char reserved_3;							// PB6		F67-F60 Future Use, All zeros
	unsigned char reserved_4;							// PB7		F77-F70 Future Use, All zeros
	unsigned char reserved_5;							// PB8		F87-F80 Future Use, All zeros
	unsigned char reserved_6;							// PB9		F97-F90 Future Use, All zeros
	unsigned char reserved_7;							// PB10		F107-F100 Future Use, All zeros
}MPEG_PAYLOAD;

// PB5[1:0]	MF1-MF0	MPEG Frame
enum MPEG_IFM_FRAME
{
	MF_UNKNOWN = 0x00,
	MF_I_PICTURE,
	MF_B_PICTURE,
	MF_P_PICTURE
};

// PB5[4]	FR0		Field Repeat
enum MPEG_IFM_FR
{
	FR_NEW_FIELD = 0x00,
	FR_REPEATED_FIELD,
};

/*********************************************************************************
*		Each parameter definition for Source Product Description InfoFrame 	     *
**********************************************************************************/
typedef struct
{
	unsigned char check_sum;							// PB0
	unsigned char vendor_name[8];						// PB1-PB8 	VN1-VN8 Vendor Name Character (7-bit ASCII Code)
	unsigned char product_desc[16];						// PB9-PB24	PD1-PD16	Product Description Character (7-bit ASCII Code)
	unsigned char source_device_information;			// PB25		Source Device Information
}SPD_PAYLOAD;

// PB25		Source Device Information
enum SPD_IFM_SDI
{
	SDI_UNKNOWN 		= 0x00, // unknown
	SDI_DIGITAL_STB		= 0x01, // Digital STB
	SDI_DVD				= 0x02,	// DVD Player
	SDI_D_VHS			= 0x03, // D-VHS
	SDI_HDD_VIDEO		= 0x04, // HDD Videorecoder
	SDI_DVC				= 0x05,	// DVC
	SDI_DSC				= 0x06, // DSC
	SDI_VIDEO_CD		= 0x07, // Video CD
	SDI_GAME			= 0x08, // Game
	SDI_PC_GENERAL		= 0x09,	// PC General
	SDI_BLU_RAY_DISC	= 0x0A,	// Blu-Ray Disc (BD)
	SDI_SUPER_AUDIO_CD	= 0x0B,	// Super Audio CD
};

/*********************************************************************************
*		Each parameter definition for General Control Packet 	  			   *
**********************************************************************************/
typedef struct
{
	unsigned char set_avmute		: 1;        //SB0[0]	
	unsigned char reserved_0		: 3;	
	unsigned char clean_avmute		: 1;        //SB0[4]	
	unsigned char reserved_1		: 3;	
	unsigned char color_depth		: 4;        //SB1[0:3]	
	unsigned char packing_phase	: 4;        //SB1[4:7]	
	unsigned char default_phase		: 1;        //SB2[0]	
	unsigned char reserved_2		: 7;	
	unsigned char sub_byte3;        //SB3	
	unsigned char sub_byte4;        //SB4
	unsigned char sub_byte5;        //SB5
	unsigned char sub_byte6;        //SB6
}GCP_PAYLOAD;

enum GCP_PKT_COLOR_DEPTH
{    
	COLOR_DEPTH_NOT_INDICATED   = 0x00,   
	COLOR_DEPTH_24              = 0x04,   
	COLOR_DEPTH_30              = 0x05,  
	COLOR_DEPTH_36              = 0x06,    
	COLOR_DEPTH_48              = 0x07
};
enum GCP_PKT_PACKING_PHASE
{   
	PACKING_PHASE4  = 0x00,   
	PACKING_PHASE1  = 0x01,    
	PACKING_PHASE2  = 0x02,    
	PACKING_PHASE3  = 0x03
};

typedef struct
{
	/* Infor Frame Packet Header */
	unsigned char Type;			// HB0
	unsigned char Version;		// HB1
	unsigned char Length;		// HB2

	/* Infor Frame Packet Content */
	// PB0 			Checksum
	// PB1 - PB27 	Data Byte1 - Data Byte27
	union
	{
		AVI_PAYLOAD 	avi;
		AUDIO_PAYLOAD	audio;
		SPD_PAYLOAD		spd;
		VSI_PAYLOAD		vsi;
		MPEG_PAYLOAD	mpeg;
		GCP_PAYLOAD		gcp;
		unsigned char   databyte[28];
	};
}INFOFRAME;
#pragma pack (0)
#endif

