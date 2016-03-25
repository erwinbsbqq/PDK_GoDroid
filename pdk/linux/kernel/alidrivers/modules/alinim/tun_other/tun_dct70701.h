/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    nim_dct70701.h
*
*    Description:    Header file for DCT70701.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20080111		Penghui	Ver 0.1		Create file.
*****************************************************************************/

#ifndef __TUN_DCT70701_H__
#define __TUN_DCT70701_H__

//#include "ali_qam.h"

#include "../basic_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_dct70701_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_dct70701_control(UINT32 Tun_id, UINT32 freq, UINT32 sym);//, UINT8 AGC_Time_Const, UINT8 _i2c_cmd
INT32 tun_dct70701_status(UINT32 Tun_id, UINT8 *lock);
INT32 tun_dct70701_release(void);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_DCT70701_H__ */


