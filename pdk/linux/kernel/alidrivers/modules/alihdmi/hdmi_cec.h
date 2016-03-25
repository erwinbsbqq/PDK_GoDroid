/******************************************************************************
* Copyright (C) 2008 ALi Corp. All Rights Reserved.All rights reserved.
*
* File: hdmi_cec.h
*
* Description¡G
*
* History:
*     Date         By          Reason           Ver.
*   ==========  =========    ================= ======
*******************************************************************************/

#ifndef HDMI_CEC_H
#define HDMI_CEC_H
/* body of  hdmi_cec.h  file */

/*********************
* EXPORTED TYPEDEFS  *
**********************/
//typedef and enum
enum CEC_LOGIC_ADDR
{
	LOGADDR_TV				= 0,
	LOGADDR_RECORD_DEV1		= 1,
	LOGADDR_RECORD_DEV2		= 2,
	LOGADDR_TUNER1			= 3,
	LOGADDR_PLAYBACK_DEV1	= 4,
	LOGADDR_AUDIO_SYSTEM	= 5,
	LOGADDR_TUNER2			= 6,
	LOGADDR_TUNER3			= 7,
	LOGADDR_PLAYBACK_DEV2	= 8,
	LOGADDR_RECORD_DEV3		= 9,
	LOGADDR_TUNER4			= 10,
	LOGADDR_PLAYBACK_DEV3	= 11,
	LOGADDR_RESERVED1		= 12,
	LOGADDR_RESERVED2		= 13,
	LOGADDR_FREE_USE		= 14,
	LOGADDR_UNREG_BRDCST	= 15
};

/********************************
* EXPORTED FUNCTIONS PROTOTYPES *
*********************************/
void hdmi_cec_init(void);
void hdmi_set_cec_logic_address( unsigned char cec_address);
unsigned char hdmi_get_cec_logic_address(void);
unsigned char hdmi_get_cec_status(void);
void hdmi_clear_cec_status(void);
unsigned char hdmi_get_cec_datastatus(void);
void hdmi_clear_cec_datastatus(void);
bool hdmi_cec_transmit(unsigned char* Data, unsigned char Data_length);
unsigned char hdmi_cec_receive(unsigned char* buf);
void hdmi_set_biu_frequency( void);
void hdmi_set_cec_rst( bool cec_reset);
bool hdmi_cec_polling_message(unsigned char dest_addr);
void hdmi_cec_report_physicalAddr(void);
#endif // HDMI_CEC_H

