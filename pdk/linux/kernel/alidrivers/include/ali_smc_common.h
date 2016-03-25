/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smc_common.h
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of the smart card ioctl commands.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#ifndef _ALI_SMC_COMMON_H_
#define _ALI_SMC_COMMON_H_


/*! @addtogroup Devicedriver
 *  @{
 */
 
/*! @addtogroup ALiSMC
 *  @{
*/

#include <ali_basic_common.h>
#include <alidefinition/adf_smc.h>


#define SMC_CMD_GET_CARD_STATUS     _IOR('o', 526, unsigned short)      //!< Get if the smart card inserted: 0 not; 1 yes 
#define SMC_CMD_SET_TRANSPORT_ID    _IOW('o', 527, uint32)       		//!< Set port for app / kernel communication 
#define SMC_CMD_RESET               _IOWR('o', 528, smc_atr_t)          //!< Reset the smart card 
#define SMC_CMD_DEACTIVE            _IO('o', 529)                       //!< Deactive the smart card 
#define SMC_CMD_ISO_TRANS           _IOWR('o', 530, smc_iso_transfer_t) //!< ISO 7816 transferation command for T0
#define SMC_CMD_ISO_TRANS_T1        _IOWR('o', 531, smc_iso_transfer_t) //!< ISO 7816 transferation command for T1
#define SMC_CMD_SET_IO_ONOFF        _IOW('o', 533, int32)               //!< To do  
#define SMC_CMD_SET_ETU			    _IOW('o', 534, uint32)       		//!< Set working etu 
#define SMC_CMD_SET_WWT			    _IOW('o', 535, uint32)       		//!< Set the first character waiting time, in unit of ms 			
#define SMC_CMD_SET_GUARDTIME	    _IOW('o', 536, int32)               //!< To do 
#define SMC_CMD_SET_BAUDRATE		_IOW('o', 537, int32)               //!<  To do 
#define SMC_CMD_CHECK_STATUS		_IOR('o', 538, int32) 				//!< Check status
#define SMC_CMD_CLKCHG_SPECIAL	    _IOW('o', 539, int32)               //!< To do
#define SMC_CMD_FORCE_SETTING	    _IOW('o', 540, int32)               //!< To do 
#define SMC_CMD_SET_CWT			    _IOW('o', 541, uint32)       		//!< Set character waiting time, in unit of ms 
#define SMC_CMD_GET_F				_IOR('o', 542, uint32)       		//!< Get F factor value 
#define SMC_CMD_GET_D				_IOR('o', 543, uint32)       		//!< Get D factor value 
#define SMC_CMD_GET_ATR_RESULT 	    _IOR('o', 544, int32)               //!< Get ATR status 
#define SMC_CMD_GET_HB				_IOR('o', 545, smc_hb_t)            //!< Get History Bytes 
#define SMC_CMD_GET_PROTOCOL		_IOR('o', 546, uint32)       		//!< Get the smart card current protocol 
#define SMC_CMD_SET_WCLK			_IOW('o', 547, uint32)       		//!< Set the working clock of the smart card,  
									                                    //!< The new setting value will be used from next time reset 
#define SMC_CMD_GET_CLASS			_IOR('o', 548, int32)               //!< Get the currently selected classs 
#define SMC_CMD_SET_CLASS			_IOW('o', 549, int32)               //!< Set new class selection if previous select fail 
#define SMC_CMD_T1_TRANS			_IOWR('o', 550, smc_t1_trans_t)     //!< T1 communication, need to build block frame by driver
#define SMC_CMD_T1_XCV              _IOWR('o', 551, smc_t1_xcv_t)       //!< T1 communication 
#define SMC_CMD_T1_NEGO_IFSD        _IOW('o', 552, smc_t1_nego_ifsd_t)  //!< Set t1 negociate maximun information field size
#define SMC_CMD_CONFIG			    _IOW('o', 553, struct smc_dev_cfg)  //!< Configure the smart card 
#define SMC_CMD_DECONFIG			_IO('o', 554)						//!< De-configure the smart card 
#define SMC_CMD_SEND_PPS			_IOW('o', 555, uint32) 				//!< To do 			
#define SMC_CMD_SET_OPEN_DRAIN		_IOW('o', 556, uint32) 				//!< Enable/disable the open drain mode
#define SMC_CMD_SET_DEBUG_LEVEL		_IOW('o', 557, uint32) 				//!< Set debug level

/*!
@}
*/

/*!
@}
*/


#endif
