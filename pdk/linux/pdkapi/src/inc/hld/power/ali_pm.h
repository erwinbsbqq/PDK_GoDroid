/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_pm.h
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of power management ioctl commands.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#ifndef _ALI_POWER_MANAGEMENT___H_
#define _ALI_POWER_MANAGEMENT___H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
@brief setup fast standby ir wakeup key
@param[in] ip_power_key : fast standby ir wakeup key
@return void
*/
void set_ali_fast_standby_ir_power_key(unsigned long * ir_power_key);


/*!
@brief setup fast standby wakeup timer
@param[in] timout : how many second the fast stanby will wakeup
@return void
*/
void set_ali_fast_standby_timeout(unsigned long timout);

/*!
@brief setup fast standby wakeup 
@param[in] reboot : reboot = 1, 
@retval  reboot = 1, the platform will reboot after exit standby
@retval  reboot = 0,  the platform do not reboot after exit standby
@return void
*/
void set_ali_fast_standby_reboot(unsigned long reboot);

/*!
@brief enter fast standby
*/
void enter_ali_fast_standby(void);


#ifdef __cplusplus
}
#endif

#endif
