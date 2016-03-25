/***************************************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:    
*		hdmi_dev.h
*
*    Description:    
*		This file define data type for hdmi driver processing.
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#include <linux/ioctl.h>

#define HDMI_IOC_MAGIC 'h'

typedef struct  {
	unsigned int hdcp_status;
} HDMI_ioctl_hdcp_state_t;

typedef struct  {
	unsigned int cec_status;
} HDMI_ioctl_cec_state_t;

typedef struct  {
	unsigned char cec_addr;
	bool ret;
} HDMI_ioctl_cec_addr_t;

typedef struct  {
	unsigned char message[16];
	unsigned char message_length;
	bool ret;
} HDMI_ioctl_cec_msg_t;

#define HDMI_IOCINI_HDMIHW			_IO(HDMI_IOC_MAGIC,		0)
#define HDMI_IOCT_NETPORT			_IO(HDMI_IOC_MAGIC,		1)
#define HDMI_IOCT_HDCPONOFF			_IO(HDMI_IOC_MAGIC,		2)
#define HDMI_IOCQ_MUTESTA			_IOR(HDMI_IOC_MAGIC,	3,	int)
#define HDMI_IOCQ_HDCPSTAT			_IOR(HDMI_IOC_MAGIC,	4,	int)
#define HDMI_IOC_SET8MA				_IO(HDMI_IOC_MAGIC,		5)
#define HDMI_IOC_SET85MA			_IO(HDMI_IOC_MAGIC,		6)
#define HDMI_IOC_SET9MA				_IO(HDMI_IOC_MAGIC,		7)
#define HDMI_IOC_SET95MA			_IO(HDMI_IOC_MAGIC,		8)
#define HDMI_IOC_SET10MA			_IO(HDMI_IOC_MAGIC,		9)
#define HDMI_IOC_SET105MA			_IO(HDMI_IOC_MAGIC,		10)
#define HDMI_IOC_SET11MA			_IO(HDMI_IOC_MAGIC,		11)
#define HDMI_IOC_SET115MA			_IO(HDMI_IOC_MAGIC,		12)
#define HDMI_IOC_SET12MA			_IO(HDMI_IOC_MAGIC,		13)
#define HDMI_IOC_SET125MA			_IO(HDMI_IOC_MAGIC,		14)
#define HDMI_IOC_SET13MA			_IO(HDMI_IOC_MAGIC,		15)
#define HDMI_IOC_SET135MA			_IO(HDMI_IOC_MAGIC,		16)
#define HDMI_IOC_SET14MA		 	_IO(HDMI_IOC_MAGIC,		17)
#define HDMI_IOC_SET145MA			_IO(HDMI_IOC_MAGIC,		18)
#define HDMI_IOC_SET15MA	 		_IO(HDMI_IOC_MAGIC,		19)
#define HDMI_IOC_SET155MA			_IO(HDMI_IOC_MAGIC,		20)
#define HDMI_IOC_SET16MA			_IO(HDMI_IOC_MAGIC,		21)
#define HDMI_IOCG_HDMIMODE			_IOR(HDMI_IOC_MAGIC,	22,	int)
#define HDMI_IOCG_HDMIAUDIO			_IOR(HDMI_IOC_MAGIC,	23,	short)
#define HDMI_IOCG_EDIDRDY			_IOR(HDMI_IOC_MAGIC,	24,	int)
#define HDMI_IOCG_NATIVERES			_IOR(HDMI_IOC_MAGIC,	25,	int)
#define HDMI_IOC_DEVCLOSE			_IO(HDMI_IOC_MAGIC,		26)

#define HDMI_IOCT_HDCP_GET_STATUS	_IO(HDMI_IOC_MAGIC, 	27)
#define HDMI_IOCT_HDCP_SET_ONOFF	_IOW(HDMI_IOC_MAGIC, 	28, HDMI_ioctl_hdcp_state_t *)
#define HDMI_IOCT_HDCP_GET_ONOFF	_IOR(HDMI_IOC_MAGIC, 	29, HDMI_ioctl_hdcp_state_t *)
#define HDMI_IOCT_CEC_SET_ONOFF		_IOW(HDMI_IOC_MAGIC, 	30, HDMI_ioctl_cec_state_t *)
#define HDMI_IOCT_CEC_GET_ONOFF		_IOR(HDMI_IOC_MAGIC, 	31, HDMI_ioctl_cec_state_t *)
#define HDMI_IOCT_CEC_GET_PA		_IOR(HDMI_IOC_MAGIC, 	32, HDMI_ioctl_cec_addr_t *)
#define HDMI_IOCT_CEC_SET_LA		_IOW(HDMI_IOC_MAGIC, 	33, HDMI_ioctl_cec_addr_t *)
#define HDMI_IOCT_CEC_GET_LA		_IOR(HDMI_IOC_MAGIC, 	34, HDMI_ioctl_cec_addr_t *)
#define HDMI_IOCT_CEC_TRANSMIT		_IOW(HDMI_IOC_MAGIC, 	35, HDMI_ioctl_cec_msg_t *)


