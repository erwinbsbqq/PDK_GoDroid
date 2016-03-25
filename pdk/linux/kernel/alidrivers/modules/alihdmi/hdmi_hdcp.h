/***************************************************************************************************
*    ALi Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_hdcp.h
*
*    Description:
*		ALi HDMI HDCP Process
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#ifndef	_HDMI_HDCP_H_
#define	_HDMI_HDCP_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#if defined(CONFIG_ALI_M39)
#include <video/ali_hdmi.h>
//#else
//#include <linux/ali_hdmi.h>
#endif

#define	HDCP_RX_PRI_BKSV        0x00
#define	HDCP_RX_PRI_RI          0x08
#define	HDCP_RX_PRI_PJ          0x0A
#define	HDCP_RX_PRI_AKSV        0x10
#define	HDCP_RX_PRI_AINFO		0x15
#define	HDCP_RX_PRI_AN			0x18
#define	HDCP_RX_PRI_VH0			0x20
#define	HDCP_RX_PRI_VH1			0x24
#define	HDCP_RX_PRI_VH2			0x28
#define	HDCP_RX_PRI_VH3			0x2C
#define	HDCP_RX_PRI_VH4			0x30
#define	HDCP_RX_PRI_BCAPS		0x40
#define	HDCP_RX_PRI_BSTATUS		0x41
#define	HDCP_RX_PRI_KSV_FIFO	0x43
#define	HDCP_RX_PRI_DBG			0xC0

typedef struct
{
	unsigned char fast_reauthenication          :1;     // [0]      FAST_REAUTHENTICATION
	unsigned char advanced_cipher_support   	:1;		// [1]      1.1_FEATURES
	unsigned char reserved_2_3              	:2;		// [3:2]    Reserved
	unsigned char fast_i2c_support				:1;		// [4]      HDCP Receiver support 400KHz I2C Bus
	unsigned char ready                         :1; 	// [5]      READY, KSV FIFO ready.
	unsigned char repeater                  	:1;		// [6]      REPEATER
	unsigned char reserved_7        			:1; 	// [7]      HDMI_RESERVED.
}HDCP_BCAPS;

typedef struct
{
	unsigned char device_count                  :7;     // [6:0]    Total number of attached downstream devices.
	unsigned char max_devs_exceeded            	:1;		// [7]      Topology error indicator.
	unsigned char depth                         :3;		// [10:8]   Three-bit repeater cascade depth.
	unsigned char max_cascade_exceeded			:1;		// [11]     Topology error indicator.
	unsigned char hdmi_mode                     :1; 	// [12]     HDMI Mode
	unsigned char reserved_15_13               	:3;		// [15:13]  reserved
}HDCP_BSTATUS;

bool hdmi_hdcp_1st_establish_share_value(void);
bool hdmi_hdcp_2nd_verify_repeater_ksvlist(void);
bool hdmi_hdcp_3rd_ri_link_integrity_check(void);
bool hdmi_hdcp_3rd_pj_link_integrity_check(void);

#endif

