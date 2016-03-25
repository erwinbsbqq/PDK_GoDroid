/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-08-10 10:34:51 #$
  File Revision : $Revision:: 5923 $
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_integ.h

          This file provides the integration layer control interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_INTEG_H
#define SONY_DEMOD_INTEG_H

#include "sony_demod.h"

#include "sony_demod_dvbt.h"
#include "sony_demod_dvbt2.h"
#include "sony_demod_dvbc.h"
#include "sony_demod_dvbt2_monitor.h"
#include "sony_demod_dvbt_monitor.h"
#include "sony_demod_dvbc_monitor.h"
#include "sony_math.h"




#ifndef DEMOD_TUNE_POLL_INTERVAL
#define DEMOD_TUNE_POLL_INTERVAL    10
#endif


/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

sony_result_t sony_integ_dvbt_Tune(sony_demod_t * pDemod, sony_dvbt_tune_param_t * pTuneParam, BOOL NeedToConfigTuner);

sony_result_t sony_integ_dvbt2_Tune(sony_demod_t * pDemod, sony_dvbt2_tune_param_t * pTuneParam,BOOL NeedToConfigTuner);

sony_result_t sony_integ_dvbt2_BlindTune(sony_demod_t * pDemod, sony_dvbt2_tune_param_t * pTuneParam, BOOL NeedToConfigTuner, UINT8 t2_lite_support_flag);

sony_result_t sony_integ_CalcSSI(sony_demod_t * pDemod, uint32_t *pSSI, uint32_t rfLevel);

sony_result_t sony_integ_CalcSQI(sony_demod_t * pDemod, uint32_t *pSQI);

sony_result_t sony_integ_CalcBER(sony_demod_t * pDemod, uint32_t *pBER);

sony_result_t sony_integ_demod_CheckTSLock (sony_demod_t * pDemod, sony_demod_lock_result_t * pLock);

#endif

