/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *
 *  File: otp.h
 *
 *  Description: This file provide common otp interface.
 *
 *  History:
 *      Date        	Author      	Version  		Comment
 *      ====        	======      	=======  	=======
 *  1.  2009.8.25   	Goliath Peng   0.1.000  		Initial
 ****************************************************************************/

#ifndef	__OTP_H__
#define __OTP_H__

#include <ali/basic_types.h>

#define OTP_DW_LEN                      0x4
#define OTP_ERR_NOSUPPORT		(-1)
#define OTP_ERR_LOCKTMO		(-2)

#define OTP_VOLTAGE_6V5 65    //OTP voltage 6.5V
#define OTP_VOLTAGE_1V8 18    //OTP voltage 1.8V

/*voltage control callback function, OTP driver will tell APP the correct voltage by 
* OTP_VOLTAGE_6V5 or OTP_VOLTAGE_6V5*/
typedef void(*OTP_VOLTAGE_CONTROL)(UINT32 voltage);

/* APP must make OTP driver to control programming voltage to guarantee program timming.
* So App can choose to register GPIO to OTP driver or register voltage control callback function to OTP driver*/
typedef struct {
	UINT16	vpp_by_gpio: 1;		/*1: we need use one GPIO control vpp voltage.*/
								/*0: we use Jumper to switch vpp voltage.*/
	UINT16	reserved1: 15;		/*reserved for future usage*/
	UINT16	vpp_polar	: 1;		/* Polarity of GPIO, 0 or 1 active to set VPP to 6.5V*/
	UINT16	vpp_io		: 1;		/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16	vpp_position: 14;	/* GPIO Number*/
	OTP_VOLTAGE_CONTROL volctl_cb;		/*OTP program voltage control callback*/
										/*OTP_VOLTAGE_6V5 for 6.5V,OTP_VOLTAGE_1V8 for 1.8V*/
}OTP_CONFIG;


#if (1)

#define otp_init(cfg)				otp_m37_init(cfg)	//cfg: Pointer of OTP_CONFIG
#define otp_read(offset, buf, len)	otp_m37_read(offset, buf, len)
#define otp_write(buf, offset, len)	otp_m37_write(buf, offset, len)
#define otp_lock(offset, len)		otp_m37_lock(offset, len)
UINT32 m37_otp_read(UINT32 addr);
#else

#define otp_init()							do{}while(0)
#define otp_read(offset, buf, len)				do{}while(0)
#define otp_write(buf, offset, len)				do{}while(0)
#define otp_lock(offset, len)					do{}while(0)
#endif

///The function name is: IS_UART_FUSE_ENABLE 
#define OTP_CLOSE_UART 11    
#define OTP_SEEROM_ENABLE 8  

#define OTP_CONFIG_ADDR_03 0x18042044
/*
0: uart is not fused by OTP
1: uart is fused by OTP
*/
#define IS_UART_FUSE_ENABLE ((*(UINT32 *)(OTP_CONFIG_ADDR_03)>>OTP_CLOSE_UART)&0x1)
/*
0: SEEROM is not enabled
1: SEEROM is enabled
*/
#define IS_SEEROM_ENABLE ((*(UINT32 *)(OTP_CONFIG_ADDR_03)>>OTP_SEEROM_ENABLE)&0x1)

#endif	/* __OTP_H__ */

