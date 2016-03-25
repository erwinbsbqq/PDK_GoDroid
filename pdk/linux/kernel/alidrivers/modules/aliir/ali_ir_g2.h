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
#ifndef _ALI_IR_G2_H_
#define _ALI_IR_G2_H_



typedef enum pan_ir_protocol
{
	IR_TYPE_NEC = 0,
	IR_TYPE_LAB,
	IR_TYPE_50560,
	IR_TYPE_KF,
	IR_TYPE_LOGIC,
	IR_TYPE_SRC,
	IR_TYPE_NSE,
	IR_TYPE_RC5,
	IR_TYPE_RC6,
	IR_TYPE_UNDEFINE
}pan_ir_protocol_e;


/*! @enum pan_device_ir_ioctrl_command
    @brief 前面板IO命令集。
*/
enum pan_device_ir_ioctrl_command
{
	PAN_DRIVER_IR_SET_STANDBY_KEY	= 0xfff0,		//!<Remote controller driver set standby key command 	
	PAN_DRIVER_IR_SET_ENDIAN,			//!<Remote controller driver set endian command 
	PAN_DRIVER_IR_SET_RESUME_KEY,		//!<Remote controller driver set resume key command 
	PAN_DRIVER_IR_SET_EMOUSE,		//!<Remote controller driver set emulate mouse speed...

};

/*!@struct pan_standby_key
   @brief 前面板待机按键信息。
*/
struct pan_standby_key{
	unsigned char enable;
	unsigned long key;
};


/*!@struct pan_ir_endian
   @brief 前面板红外协议大小端模式。
*/
struct pan_ir_endian{
	unsigned int enable;
	pan_ir_protocol_e protocol;
	unsigned int bit_msb_first;
	unsigned int byte_msb_first;	
};



#endif	/* _ALI_IR_G2_H_ */

