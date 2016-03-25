/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    nim_alpstdae.h
*
*    Description:    Header file for alpstdae.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20080520		Trueve Hu	Ver 0.1		Create file.
*****************************************************************************/

#ifndef __TUN_RT810_H__
#define __TUN_RT810_H__

#include "../basic_types.h"
#include "../porting_linux_header.h"

#ifdef __cplusplus
extern "C"
{
#endif


INT32 ali_nim_tuner_register(struct tuner_handle *model);
INT32 ali_nim_tuner_unregister(const UINT8 *model_name);


INT32 tun_rt810_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_rt810_control(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
INT32 tun_rt810_status(UINT32 Tun_id, UINT8 *lock);
INT32 tun_rt810_release(void);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_RT810_H__ */

