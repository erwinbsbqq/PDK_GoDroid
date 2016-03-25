/***************************************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_edid.h
*
*    Description:
*		This file define private data type for hdmi driver EDID Parser.
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#ifndef	_HDMI_EDID_H_
#define	_HDMI_EDID_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#if defined(CONFIG_ALI_M39)
#include <video/ali_hdmi.h>
#else
#include <ali_hdmi_common.h>
#endif

#define UNKNOWN_DESCRIPTOR      						-1
#define DETAILED_TIMING_BLOCK   						-2
#define DETAILED_TIMING_DESCRIPTIONS_START         		0x36
#define DETAILED_TIMING_DESCRIPTION_SIZE        		18
#define TOTAL_NUM_DETAILED_TIMING_DESCRIPTIONS          4
#define MONITOR_LIMITS	    							0xFD
#define MONITOR_NAME	    							0xFC

typedef struct SHORT_AUDIO_DESCRIPTOR
{
	unsigned short audio_format_code;
	unsigned char max_num_audio_channels;
	unsigned char audio_sampling_rate;
	unsigned short max_audio_bit_rate;
	struct SHORT_AUDIO_DESCRIPTOR *next;
}SHORT_AUDIO_DESCRIPTOR_t;

typedef struct SHORT_VIDEO_DESCRIPTOR
{
	unsigned char native_indicator;
	unsigned char video_id_code;
	struct SHORT_VIDEO_DESCRIPTOR * next;
}SHORT_VIDEO_DESCRIPTOR_t;

typedef struct DETAILED_TIMING_DESCRIPTOR
{
	unsigned short pixel_clock_div_10000;
	unsigned short h_active;
	unsigned short h_blanking;
	unsigned short v_active;
	unsigned short v_blanking;
	unsigned short h_sync_offset;
	unsigned short h_sync_pulse_w;
	unsigned char v_sync_offset;
	unsigned char v_sync_pulse_w;
	unsigned short h_image_size;
	unsigned short v_image_size;
	unsigned char h_border;
	unsigned char v_border;
	unsigned char interlaced;//0-- non-interlaced; 1 -- interlaced
	struct DETAILED_TIMING_DESCRIPTOR * next;	
}DETAILED_TIMING_DESCRIPTOR_t;

typedef struct LIPSYNC_INFO_DESCRIPTOR
{
	bool interlaced_latency_present;
	short video_latency;
	short audio_latency;
	short interlaced_video_latency;
	short interlaced_audio_latency;	
}LIPSYNC_INFO_DESCRIPTOR_t;

typedef struct VENDOR_PRODUCT_BLOCK
{
    unsigned char manufacturer_name[4];
    unsigned short product_id; 
    unsigned int serial_number; 
    unsigned char week_manufacturer;  
    unsigned short year_manufacturer;
    struct VENDOR_PRODUCT_BLOCK * next;	
}VENDOR_PRODUCT_BLOCK_t;

void hdmi_edid_parse_extension(unsigned char* edid);
void hdmi_edid_parse_std(unsigned char *edid);
void hdmi_edid_clear(void);
bool hdmi_edid_chk_intface(void);
bool supports_AI_inVSDB(void);
void edid_get_max_channel_number(unsigned char *aud_max_ch_num);
void edid_get_prefer_audio_out(unsigned short *aud_fmt_code);
short edid_get_lipsync_audio_delay(bool interlaced_video);
void edid_get_native_resolution(enum HDMI_API_RES *res);
void edid_support_video_format(enum PicFmt *format);
#endif


