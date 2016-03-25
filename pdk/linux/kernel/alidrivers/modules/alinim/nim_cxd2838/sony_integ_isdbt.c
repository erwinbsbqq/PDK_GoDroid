/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : 2014-09-02
  File Revision : 1.0.9.0
------------------------------------------------------------------------------*/

#include "sony_integ_isdbt.h"
#include "sony_demod_isdbt_monitor.h"

#define sony_demod_TuneEnd            cxd2838_demod_TuneEnd
#define sony_demod_I2cRepeaterEnable  cxd2838_demod_I2cRepeaterEnable
#define sony_stopwatch_start          cxd2838_stopwatch_start
#define sony_stopwatch_sleep          cxd2838_stopwatch_sleep
#define sony_stopwatch_elapsed        cxd2838_stopwatch_elapsed


/*------------------------------------------------------------------------------
  Static Function Prototypes
------------------------------------------------------------------------------*/
/*
 @brief Tune process called by sony_integ_isdbt_Tune, sony_integ_isdbt_EWSTune
        and sony_integ_isdbt_Scan.
*/
static sony_result_t isdbt_Tune (sony_demod_t * pDemod, sony_isdbt_tune_param_t* pTuneParam,
                                 uint8_t isEWSTune, uint8_t isWaitTSLock);

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_isdbt_Tune(sony_demod_t* pDemod, sony_isdbt_tune_param_t* pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_integ_isdbt_Tune");

    if ((!pDemod) || (!pTuneParam)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS) &&
        (pDemod->state != SONY_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancellation flag. */
    //sony_atomic_set (&(pInteg->cancel), 0);

    /* Normal tune, wait for TS lock */
    result = isdbt_Tune(pDemod, pTuneParam, 0, 1);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);    
}

sony_result_t sony_integ_isdbt_EWSTune(sony_integ_t* pInteg, sony_isdbt_tune_param_t* pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_integ_isdbt_Tune");

    if ((!pInteg) || (!pTuneParam)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_EWS) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancellation flag. */
    //sony_atomic_set (&(pInteg->cancel), 0);

    /* EWS tune, wait for Demod lock */
    result = isdbt_Tune(pInteg, pTuneParam, 1, 0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);    
}

sony_result_t sony_integ_isdbt_Scan(sony_integ_t*                    pInteg,
                                    sony_integ_isdbt_scan_param_t*   pScanParam,
                                    sony_integ_isdbt_scan_callback_t callback)
{
    uint32_t currentFrequency;
    sony_integ_isdbt_scan_result_t scanResult;

    SONY_TRACE_ENTER("sony_integ_isdbt_Scan");

    if ((!pInteg) || (!pScanParam) || (!callback)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_EWS) &&
        (pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pScanParam->startFrequencyKHz > pScanParam->endFrequencyKHz) || (pScanParam->stepFrequencyKHz == 0)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Clear cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 0);

    currentFrequency = pScanParam->startFrequencyKHz;
    scanResult.tuneParam.bandwidth = pScanParam->bandwidth;

    while (currentFrequency <= pScanParam->endFrequencyKHz) {
        /* Check cancellation. */
        //if (sony_atomic_read(&(pInteg->cancel)) != 0) {
        //    SONY_TRACE_RETURN(SONY_RESULT_ERROR_CANCEL);
        //}

        scanResult.centerFreqKHz = currentFrequency;
        scanResult.tuneParam.centerFreqKHz = currentFrequency;
        /* Normal tune, wait for demod lock */
        scanResult.tuneResult = isdbt_Tune(pInteg, &scanResult.tuneParam, 0, 0);

        switch (scanResult.tuneResult) {
        case SONY_RESULT_OK:
            /* Channel found, callback to application */
            callback(pInteg, &scanResult, pScanParam);
            break;
        case SONY_RESULT_ERROR_UNLOCK:
        case SONY_RESULT_ERROR_TIMEOUT:
            /* Channel not found, callback to applicaiton for progress updates */
            callback(pInteg, &scanResult, pScanParam);
            break;
        default:
            SONY_TRACE_RETURN(scanResult.tuneResult);
        }
        currentFrequency += pScanParam->stepFrequencyKHz;
    }
    
    SONY_TRACE_RETURN(SONY_RESULT_OK);
}


sony_result_t sony_integ_isdbt_WaitDemodLock(sony_demod_t *pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOT_DETECT;
    uint32_t timeout = 0;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;

    SONY_TRACE_ENTER ("sony_integ_isdbt_WaitDemodLock");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_EWS)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Wait for demod lock */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    timeout = SONY_ISDBT_WAIT_DEMOD_LOCK;

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsed >= timeout) {
            continueWait = 0;
        }

        result = sony_demod_isdbt_CheckDemodLock(pDemod, &lock);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        switch (lock) {
        case SONY_DEMOD_LOCK_RESULT_LOCK:
            SONY_TRACE_RETURN(SONY_RESULT_OK);
        case SONY_DEMOD_LOCK_RESULT_UNLOCK:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_UNLOCK);
        default:
            /* continue waiting... */
            break;              
        }
        //if (sony_atomic_read (&(pInteg->cancel)) != 0) {
        //    SONY_TRACE_RETURN(SONY_RESULT_ERROR_CANCEL);
        //}

        if (continueWait) {
            result = sony_stopwatch_sleep(&timer, SONY_ISDBT_WAIT_LOCK_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN(result);
            }
        } else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }
    
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_isdbt_WaitTSLock(sony_demod_t *pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOT_DETECT;
    uint32_t timeout = 0;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;

    SONY_TRACE_ENTER ("sony_integ_isdbt_WaitTSLock");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* In EWS state, TS lock flag is invalid. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Wait for TS lock */
    result = sony_stopwatch_start(&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    timeout = SONY_ISDBT_WAIT_TS_LOCK;

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsed >= timeout) {
            continueWait = 0;
        }

        result = sony_demod_isdbt_CheckTSLock(pDemod, &lock);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        switch (lock) {
        case SONY_DEMOD_LOCK_RESULT_LOCK:
            SONY_TRACE_RETURN(SONY_RESULT_OK);
        case SONY_DEMOD_LOCK_RESULT_UNLOCK:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_UNLOCK);
        default:
            /* continue waiting... */
            break;              
        }

        /* Check cancellation. */
        //if (sony_atomic_read(&(pInteg->cancel)) != 0) {
       //     SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
       // }

        if (continueWait) {
            result = sony_stopwatch_sleep(&timer, SONY_ISDBT_WAIT_LOCK_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }
    
    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t isdbt_Tune(sony_demod_t* pDemod, sony_isdbt_tune_param_t* pTuneParam,
                                uint8_t isEWSTune, uint8_t isWaitTSLock)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("isdbt_Tune");

    if ((!pDemod) || (!pTuneParam)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Tune the demodulator */
    if (isEWSTune) {
        result = sony_demod_isdbt_EWSTune(pDemod, pTuneParam);
    } else {
        result = sony_demod_isdbt_Tune(pDemod, pTuneParam);
    }

    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    if (pDemod) {
#ifndef SONY_DISABLE_I2C_REPEATER
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable(pDemod, 1);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
#endif
        #if 0
        /* Tune the RF part */
        result = pInteg->pTuner->Tune(pInteg->pTuner,
                                      pTuneParam->centerFreqKHz,
                                      pTuneParam->bandwidth);
        #endif
		if(NULL != pDemod->tuner_control.nim_tuner_control){
		result = pDemod->tuner_control.nim_tuner_control(pDemod->tuner_id,  \
		                                           pTuneParam->centerFreqKHz,         \
		                                           pTuneParam->bandwidth,             \
		                                           0,(UINT8*)&(pDemod->system),0);
		
		}
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }

#ifndef SONY_DISABLE_I2C_REPEATER
        /* Disable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable(pDemod, 0);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN(result);
        }
#endif
    }

    /* Reset the demod to enable acquisition */
    result = sony_demod_TuneEnd(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    if (isWaitTSLock) {
        /* Wait for TS lock */
        result = sony_integ_isdbt_WaitTSLock(pDemod);
    } else {
        /* Wait for Demod lock */
        result = sony_integ_isdbt_WaitDemodLock(pDemod);
    }

    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);    
}
sony_result_t sony_integ_isdbt_monitor_RFLevel(sony_integ_t * pInteg, int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("sony_integ_isdbt_monitor_RFLevel");

    if ((!pInteg) || (!pInteg->pDemod) || (!pRFLeveldB)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
 
	if ( tun_cxd_ascot3_command(pInteg->pDemod->tuner_id, NIM_TUNER_GET_RF_POWER_LEVEL, pRFLeveldB) == ERR_FAILUE )
	{
		CXD2838_PRINTF("error: Tuner does not support command 'NIM_TUNER_GET_RF_POWER_LEVEL'.\r\n"); 
		return ERR_FAILUE;
	}

#if 0
    if (pInteg->pTuner->ReadRFLevel) {
#ifndef SONY_DISABLE_I2C_REPEATER
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
#endif

        result = pInteg->pTuner->ReadRFLevel (pInteg->pTuner, pRFLeveldB);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

#ifndef SONY_DISABLE_I2C_REPEATER
        /* Disable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
#endif
    } else if (pInteg->pTuner->CalcRFLevelFromAGC) {
        uint32_t ifAgc;

        result = sony_demod_isdbt_monitor_IFAGCOut(pInteg->pDemod, &ifAgc);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = pInteg->pTuner->CalcRFLevelFromAGC (pInteg->pTuner, ifAgc, pRFLeveldB);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }
#endif
    SONY_TRACE_RETURN (result);
}

