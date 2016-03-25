/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:    ali_pan_hwscan.h
*
*    Description:    This file contains head file definition
*		             of HW scan front panel driver.
*
*           Date            Athor        Version          Reason
*	    ======= ======== ========   =========	=================
*	1.   2010.1.8     Martin Xia   Ver 0.1          Create file.
*	2.	 2010.1.13	  Elliott Hsu  Ver 0.2			Update for pan work.
*****************************************************************************/
#ifndef _PAN_CH455_H_
#define _PAN_CH455_H_



/* IO control command */
enum pan_device_ioctrl_command
{
	PAN_DRIVER_SET_HW_INFO	= 0,		/* Front panel driver set hw info command */
	PAN_DRIVER_GET_HW_INFO	= 1,		/* Front panel driver get hw info command */
	PAN_DRIVER_SUSPEND	= 2,		/* Front panel driver suspend command */
	PAN_DRIVER_RESUME	= 3,		/* Front panel driver resume command */	
	PAN_DRIVER_READ_LNB_POWER = 4,	/* Front panel driver NIM LNB power protect status */
	PAN_DRIVER_WRITE_LED_ONOFF =5,
	PAN_DRIVER_UART_SELECT = 6,
	PAN_DRIVER_SET_GPIO	= 7,		/* Front panel driver set gpio command */
	PAN_DRIVER_GET_GPIO = 8,		/* Front panel driver get gpio value */
	PAN_DRIVER_STANDBY  = 9,        /* Front panel driver enter low power mode */
	PAN_DRIVER_SK_DETECT_POLAR = 10,/* Front panel driver set key detect polor, only enable for shadow scan */
	PAN_DRIVER_WRITE_REG	= 11,		//!<Front panel driver write register command 
	PAN_DRIVER_READ_REG	= 12,		//!<Front panel driver read register command 
	PAN_DRIVER_SET_I2C	= 13,		//!<Front panel driver set command, byte0, I2C ID; byte1, 1 gpio_i2c, 0 i2c byte2, SDA; byte3, SCL; 
};


enum pan_key_press
{
	PAN_KEY_RELEASE		= 0,
	PAN_KEY_PRESSED		= 1,
	PAN_KEY_REPEAT		= 2
};



#endif	/* _PAN_CH455_H_ */

