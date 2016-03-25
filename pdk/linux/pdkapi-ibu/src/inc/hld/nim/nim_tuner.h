/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2005 Copyright (C)
*
*    File:    nim_tuner.h
*
*    Description:    This file contains QPSK Tuner Configuration AP Functions
*
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Aug.25.2005       Jun Zhu       Ver 0.1    Create file.
*****************************************************************************/

#ifndef _NIM_TUNER_H_
#define _NIM_TUNER_H_


//Android patch:
// mark the struct which already defined in dvb_frontend_common.h!
// only keep the struct not defined in dvb_frontend_common.h!

#include <dvb_frontend_common.h>

#define MAX_TUNER_SUPPORT_NUM	2

#define COFDM_TUNER_CONFIG_API		ali_nim_mn88436_cfg


struct DEMOD_CONFIG_ADVANCED
{
	UINT32  qam_config_advanced; 	//bit0: demode_mode 0:j83b 1:j83ac;
							  		//bit1: ad sample clock 0: 27m, 1:54m;
	UINT32 qam_buffer_len;
	UINT32 qam_buffer_addr;
};


struct QAM_TUNER_CONFIG_API
{
	/* struct for QAM Configuration */
	struct QAM_TUNER_CONFIG_DATA tuner_config_data;

	/* Tuner Initialization Function */
	INT32 (*nim_Tuner_Init)(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

	/* Tuner Parameter Configuration Function */
	INT32 (*nim_Tuner_Control)(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);//since there will no bandwidth demand, so pass "sym" for later use.

	/* Get Tuner Status Function */
	INT32 (*nim_Tuner_Status)(UINT32 Tun_id, UINT8 *lock);

	/* Extension struct for Tuner Configuration */
	struct QAM_TUNER_CONFIG_EXT tuner_config_ext;
	
	struct EXT_DM_CONFIG ext_dem_config;
	struct DEMOD_CONFIG_ADVANCED dem_config_advanced;
	
	UINT32 tuner_id;// kent, for defferent tuner
};


#endif // _NIM_TUNER_H_
