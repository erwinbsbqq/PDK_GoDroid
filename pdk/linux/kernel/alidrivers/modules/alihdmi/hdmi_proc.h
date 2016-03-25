/***************************************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_proc.h
*
*    Description:
*		This file define private data type for hdmi driver processing.
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#ifndef	_HDMI_PROC_H_
#define	_HDMI_PROC_H_


#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fb.h>


#if defined(CONFIG_ALI_M39)
#include <video/ali_hdmi.h>
#else
//#include <linux/ali_hdmi.h>
#include <linux/poll.h>
#endif

#include "hdmi_infoframe.h"
#include "hdmi_edid.h"



// defined by menuconfig
//debug message
//#define CONFIG_HDMI_PROC_DEBUG_ALI 1
//#define CONFIG_HDMI_EDID_DEBUG_ALI 1
//#define CONFIG_HDMI_HDCP_DEBUG_ALI 1
//#define CONFIG_HDMI_CEC_DEBUG_ALI 1

//menu config params
#define CONFIG_HDCP_ENABLE_ALI 1
//#define CONFIG_CEC_ENABLE_ALI 1
//#define CONFIG_HDCP_SOURCE_MAX_KSV_ALI 9
//#define CONFIG_HDCP_SOURCE_AUTHE_COUNT_ALI 1
#define CONFIG_ALI_M3701 1

//#define AVMUTE_AT_VBLK

#if defined(CONFIG_HDMI_PROC_DEBUG_ALI)
//#define HDMI_DEBUG(fmt, args...) if(hdmi_debug) printk(KERN_DEBUG fmt, ## args)
#define HDMI_DEBUG(fmt, args...) printk(fmt, ## args)
#else
#define HDMI_DEBUG(fmt, args...) do{}while(0)
#endif

#if defined(CONFIG_HDMI_EDID_DEBUG_ALI)
//#define EDID_DEBUG(fmt, args...) if(edid_debug) printk(KERN_DEBUG fmt, ## args)
#define EDID_DEBUG(fmt, args...) printk(fmt, ## args)
#else
#define EDID_DEBUG(fmt, args...) do{}while(0)
#endif

#if defined(CONFIG_HDMI_HDCP_DEBUG_ALI)
//#define HDCP_DEBUG(fmt, args...) if(hdcp_debug) printk(KERN_DEBUG fmt, ## args)
#define HDCP_DEBUG(fmt, args...) printk(fmt, ## args)
#else
#define HDCP_DEBUG(fmt, args...) do{}while(0)
#endif

#if defined(CONFIG_HDMI_CEC_DEBUG_ALI)
//#define CEC_DEBUG(fmt, args...) if(cec_debug) printk(KERN_DEBUG fmt, ## args)
#define CEC_DEBUG(fmt, args...) printk(fmt, ## args)
#else
#define CEC_DEBUG(fmt, args...) do{}while(0)
#endif

#define MAX_EDID_BLOCKS_SUPPORT		8
#define EDID_BLOCK_SIZE				128
#define HDMI_FLAG_VIDEO_UPDATE		0x00000001
#define HDMI2USR_EDID_READY_MSG		0x00000001
#define HDMI2USR_HOTPLUG_IN_MSG		0x00000002
#define HDMI2USR_HOTPLUG_OUT_MSG	0x00000004
#define HDMI2USR_CEC_MSG			0x00000008

struct ali_hdmi_device_data
{
	struct	cdev cdev;
	void 	*priv;
};

// HDMI State
enum HDMI_STATE
{
	HDMI_STATE_IDLE = 0,		//hot plug has been detached
	HDMI_STATE_READY,			//hot plug has been attached but not transmit A/V data
	HDMI_STATE_PLAY,			//hdmi has transmitted a/v data
	HDMI_STATE_AV_CHANGED		//in hdmi A/V change tempory state.
};

enum OUTPUT_MODE
{
	DVI_MODE    = 0,
	HDMI_MODE   = 1,
};

enum HDMI_TMDS_DRV_CURRENT   //TMDS driver current 9mA,9.5mA,10mA,10.5mA,11mA,11.5mA.
{
    HDMI_DRV_CUR_9MA 	= 0,
    HDMI_DRV_CUR_95MA 	= 1,
    HDMI_DRV_CUR_10MA 	= 2,
    HDMI_DRV_CUR_105MA 	= 3,
    HDMI_DRV_CUR_11MA 	= 4,
    HDMI_DRV_CUR_115MA 	= 5,
};

enum HDMI_TMDS_CLK_RATE     //TMDS clock rate according resolution and deep color.
{	
	HDMI_TMDS_CLK_27000 = 0,
	HDMI_TMDS_CLK_27000_125,    
	HDMI_TMDS_CLK_27000_15,    
	HDMI_TMDS_CLK_27000_2,    
	HDMI_TMDS_CLK_108000,    
	HDMI_TMDS_CLK_74250,    
	HDMI_TMDS_CLK_74250_125,    
	HDMI_TMDS_CLK_74250_15,
	HDMI_TMDS_CLK_74250_2,
	HDMI_TMDS_CLK_148500,   
	HDMI_TMDS_CLK_148500_125,   
	HDMI_TMDS_CLK_148500_15,
	HDMI_TMDS_CLK_148500_2,
	HDMI_TMDS_CLK_297000,
};

typedef struct
{
	unsigned int 	chip_id;
	bool			bVCXO;
	unsigned char	bonding_option;
	unsigned char	chip_version;
	unsigned char	chip_version2;
	unsigned char	hw_version;
	
} CHIP_INFO;

typedef struct
{
	bool						hdmi_boot_cfg_valid;
	bool			            hdmi_enable;			// TRUE: enable HDMI output, FALSE: disable HDMI output
	bool                           hdmi_audio_enable;    // TRUE: enable HDMI audio output, FALSE: disable HDMI audio output
	bool			            hdcp_enable;			// TRUE: enable HDCP Encryption FALSE: disable HDCP Encryption
	bool			            cec_enable;				// TRUE: enable CEC function FALSE: disable cec function
	enum OUTPUT_MODE	        hdmi_dvi_mode;			// HDMI Mode / DVI Mode
	bool 			            hot_plug_in;		    // TRUE: HDMI is Plug In, FALSE: HDMI is Plug Out
	bool			            edid_ready;				// TRUE: Got valid EDID Data, FALSE: no valid EDID Data
	bool                        valid_avi_info;         // TRUE: Has Valid AVI Infoframe, FALSE: Invalid AVI Infoframe
	bool                        valid_aud_info;         // TRUE: Has Valid AVI Infoframe, FALSE: Invalid AVI Infoframe
	bool			            av_mute_state;			// TRUE: Set AVMUTE Sate, FALSE: Clear AVMUTE Sate
	bool			            need_reauth;            // Set True if Need Re-authentication
	bool						load_key_from_ce;		// 
	enum HDMI_STATE	            hdmi_state;				// Current HDMI State
	enum HDMI_API_DEEPCOLOR	deep_color;			// Current HDMI Depp Color
	unsigned char				pp_value;				// Current HDMI pp value 
	enum HDMI_API_COLOR_SPACE	color_space;		// Current HDMI Color Space
	bool				aksv_fail;		// For ALiDroid boot media resource issue
	int                         eddc_adapter_id;
	unsigned char               hdmi_int_status;		// For Interrupt Service Routine use.
	unsigned char               hdcp_int_status;
	unsigned char               cec_int_status;
	unsigned char		phy_int_status;		// 3821 phy clk,[0] : PCG INT, [1] : CMU INT
	unsigned char		hdmi_int2_status;
    wait_queue_head_t           wait_queue;
    struct workqueue_struct*    interrupt_work_queue;
	struct workqueue_struct*    hdcp_work_queue;
} CONTROL_DATA;

typedef struct
{
	INFOFRAME				infoframe;
}VSI_DATA;

typedef struct
{
    FB2HDMI_INFO        	config;
    INFOFRAME           	infoframe;
}VIDEO_DATA;

typedef struct
{
    ALSA2HDMI_INFO      	config;
	unsigned int  	    	n;
	unsigned int  	    	cts;
	INFOFRAME           	infoframe;
}AUDIO_DATA;

typedef struct
{
	INFOFRAME           	infoframe;
}GCP_DATA;

typedef struct
{
	unsigned char 			vendor_name[8];		// Vendor Name Character (7-bit ASCII Code)
	unsigned char 			vendor_name_len;
	unsigned char 			product_desc[16];	// Product Description Character (7-bit ASCII Code)
	unsigned char 			product_desc_len;
	INFOFRAME           	infoframe;
}SPD_DATA;

typedef struct
{
	unsigned char       	number_of_extension_block;
	unsigned char       	block[MAX_EDID_BLOCKS_SUPPORT][EDID_BLOCK_SIZE];
	struct fb_monspecs  	mon_specs;
	unsigned int	 		resolution_support[14];
	unsigned int			native_res_index;
	unsigned int        	cea_version;
	bool                	found_IEEE_reg_code;
	bool 			support_AI;
	bool                	underscan;         // true: if DTV monitor underscans IT video formats by default
	bool                	support_basic_audio;
	bool                	support_ycbcr444;
	bool                	support_ycbcr422;
	unsigned int       		number_native_dtds; // total number of native DTDs
	unsigned int   			speaker_allocation;
	
	unsigned short			physical_address;
	bool                    support_3d;
	unsigned short			deep_color;
}EDID_DATA;

typedef struct
{
	unsigned char       	source_max_ksv;     // Maximum number of downstream devices listed in KSV list which the DUT support (1,2,...up to 127)
	unsigned int        	source_authe_count;	// Number of times the DUT attempts authentication before it transitions into the authenticated state.
	bool                	source_out_onlyrep; // Does DUT output contents to a repeater to which no downstream device connected
	bool                	authenticated;
    bool                	sink_is_repeater;
	struct timer_list   	repeater_ready_timer;
	bool 		        	repeater_ready_timeout;
	unsigned char       	key[286];
}HDCP_DATA;

typedef struct
{
	unsigned char  	    	data_status;
	unsigned short 			physical_address;
	unsigned char  	    	buffer_length;
	unsigned char  	    	receive_buffer[16];// For cec receive message
	unsigned char	    	logical_address;
}CEC_DATA;

typedef struct
{
	unsigned long parg1;
	unsigned long parg2;
}EDID_READY_MSG;

typedef struct
{
	unsigned long parg1;
	unsigned long parg2;
}HOT_PLUGIN_MSG;

typedef struct
{
	unsigned long parg1;
	unsigned long parg2;
}HOT_PLUGOUT_MSG;

typedef struct
{
	unsigned long *data;	// buffer 
	unsigned long len;	// length
}CEC_MSG;

typedef struct
{
	struct class 			*ali_hdmi_class;
	struct device			*ali_hdmi_node;
	struct proc_dir_entry	*proc_hdmi_dir;
	struct proc_dir_entry	*proc_hdmi_file;
	struct proc_dir_entry	*proc_debg_file;
}HDMI_FILE_SYSTEM;

typedef struct
{
	EDID_READY_MSG			edid_notify;
	HOT_PLUGIN_MSG			plgin_notify;
	HOT_PLUGOUT_MSG			plgout_notify;
	CEC_MSG					cec_notify;
	int 					port_id[10];
	UINT32					flag;
}HDMI_CALLBACK_MSG;

typedef struct 
{
	CHIP_INFO				chip_info;
	CONTROL_DATA 	    	control;		// Control Data
	VSI_DATA				vsi;			// VSI Data
	VIDEO_DATA		    	video;			// Video Data
	AUDIO_DATA		    	audio;			// Audio Data
	GCP_DATA			gcp;			// GCP Data
	SPD_DATA				spd;			// SPD Data
	EDID_DATA		   		edid;			// EDID Data
	HDCP_DATA		    	hdcp;			// HDCP Data
	CEC_DATA		    	cec;			// CEC Data
	HDMILinkStatus_E		link_status;	// HDMI link status
	HDMI_CALLBACK_MSG		hdmi2usr;
	HDMI_FILE_SYSTEM		hdmi_file_sys;
	struct task_struct 		*hdmi_thread;	// For hdcp thread
	wait_queue_head_t 		wait_que;
	unsigned int 			wait_flag;
	wait_queue_head_t		poll_plug_wq;
	unsigned int 			plug_status;	// 0x5a5a5a5a is plug in 0xa5a5a5a5 is plugout.
	enum HDMI_TMDS_CLK_RATE	tmds_clock;			// Current TMDS clock
}HDMI_PRIVATE_DATA;

void hdmi_proc_driver_init(void);
void hdmi_proc_audio_n_cts_update(void);
void hdmi_proc_audio_interface_config(void);
void hdmi_proc_audio_infoframe_update(void);
void hdmi_proc_avi_infoframe_update(void);
void hdmi_proc_gcp_update(void);
void hdmi_proc_vsi_infoframe_update(void);
void hdmi_proc_transmit_infoframe(INFOFRAME *infoframe);
void hdmi_proc_state_update(void);
void hdmi_proc_get_parse_edid(void);
void hdmi_proc_set_avmute(bool set_avmute, unsigned char cnt);
bool hdmi_proc_get_avmute_state(void);
void hdmi_proc_set_phy_onoff(bool turn_on);
void hdmi_proc_3d_phy_output(void);
void hdmi_proc_south_bridge_config(void);
void hdmi_proc_3d_phy_config(unsigned char address, unsigned short value);
void hdmi_proc_tmds_clk_update(void);
void hdmi_proc_config_digital_value(void);
INT32 hdmi_proc_config_deep_color(enum HDMI_API_DEEPCOLOR deep_color);
void hdmi_proc_config_color_space(enum HDMI_API_COLOR_SPACE color_space);
#ifdef CONFIG_HDCP_ENABLE_ALI
void hdmi_proc_hdcp_stop(void);
#endif

#endif

