/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    Tun_tda18250.h
*
*    Description:    Header file for alpstdae.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20090219	Magic Yang	Ver 0.1		Create file.
*****************************************************************************/

#ifndef __LLD_TUN_TDA18250_H__
#define __LLD_TUN_TDA18250_H__


#ifdef __cplusplus
extern "C"
{
#endif

INT32 		tun_tda18250_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 		tun_tda18250_control(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
INT32 		tun_tda18250_control_X(UINT32 Tun_id, UINT32 freq, UINT32 sym);
INT32 		tun_tda18250_status(UINT32 Tun_id, UINT8 *lock);
INT32 		tun_tda18250_close(UINT32 tuner_id);


#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_TDA18250_H__ */


