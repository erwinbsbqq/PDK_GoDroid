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

#ifndef __TUN_TDA18250AB_H__
#define __TUN_TDA18250AB_H__

//#include "../ali_qam.h"

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_tda18250ab_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_tda18250ab_control(UINT32 Tun_id, UINT32 freq, UINT32 sym);
INT32 tun_tda18250ab_status(UINT32 Tun_id, UINT8 *lock);
INT32 tun_tda18250ab_release(void);
unsigned int  tun_tda18250abwrite	(UINT32  uAddress, UINT32  uSubAddress, UINT32  uNbData, UINT32* pDataBuff);
//Bool tun_tda18250writebit(UInt32  uAddress, UInt32  uSubAddress, UInt32  uMaskValue, UInt32  uValue);
unsigned int	tun_tda18250abread(UINT32  uAddress, UINT32  uSubAddress,UINT32  uNbData,UINT32* pDataBuff);

#ifdef __cplusplus
}
#endif

#endif  /* __TUN_TDA18250_H__ */

