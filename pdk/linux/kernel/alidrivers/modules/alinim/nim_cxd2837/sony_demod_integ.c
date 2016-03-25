/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-06-12 13:06:10 #$
  File Revision : $Revision:: 5540 $
------------------------------------------------------------------------------*/
#include "sony_demod_integ.h"

#define sony_demod_TuneEnd            cxd2837_demod_TuneEnd
#define sony_demod_I2cRepeaterEnable  cxd2837_demod_I2cRepeaterEnable
#define sony_math_log10 			  cxd2837_math_log10
#define sony_stopwatch_start          cxd2837_stopwatch_start
#define sony_stopwatch_sleep          cxd2837_stopwatch_sleep
#define sony_stopwatch_elapsed        cxd2837_stopwatch_elapsed
sony_result_t sony_integ_demod_monitor_PacketErrorNumber (sony_demod_t * pDemod, uint32_t * pen)
{
	sony_result_t result = SONY_RESULT_OK;
	SONY_TRACE_ENTER ("sony_integ_demod_monitor_PacketErrorNumber");
	
	if (!pDemod || !pen)
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

	/* Software state check */
	if (pDemod->state == SONY_DEMOD_STATE_ACTIVE_T_C)
	{
		switch (pDemod->system)
		{
			case SONY_DTV_SYSTEM_DVBT:
			{
			    result =  sony_demod_dvbt_monitor_PacketErrorNumber(pDemod, pen);//sony_dvb_demodT_CheckTSLock
			    SONY_TRACE_RETURN (result);
			}
			case SONY_DTV_SYSTEM_DVBT2:
			{
			    result =  sony_demod_dvbt2_monitor_PacketErrorNumber(pDemod, pen);//sony_dvb_demodC_CheckTSLock
			    SONY_TRACE_RETURN (result);
			}
			/* Intentional fall through. */
			case SONY_DTV_SYSTEM_UNKNOWN:
			default:
				*pen = 0xffff;
				SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	    }
	}
	else 
	{
	        CXD2837_LOG(pDemod, "%s(): Unlock. pDemod->state = %d .\r\n", __FUNCTION__, pDemod->state);
	        *pen = 0xffff;
	        SONY_TRACE_RETURN (SONY_DEMOD_STATE_INVALID);
    }
}


/*Check DVBC/T/T2 TS Lock status*/
sony_result_t sony_integ_demod_CheckTSLock (sony_demod_t * pDemod, sony_demod_lock_result_t * pLock)
{
	sony_result_t result = SONY_RESULT_OK;
	SONY_TRACE_ENTER ("sony_integ_demod_CheckTSLock");
	
	if (!pDemod || !pLock)
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

	/* Software state check */
	if (pDemod->state == SONY_DEMOD_STATE_ACTIVE_T_C)
	{
		switch (pDemod->system)
		{
			case SONY_DTV_SYSTEM_DVBT:
			{
			    result =  sony_demod_dvbt_CheckTSLock(pDemod, pLock);//sony_dvb_demodT_CheckTSLock
			    SONY_TRACE_RETURN (result);
			}
			case SONY_DTV_SYSTEM_DVBC:
			{
			    result =  sony_demod_dvbc_CheckDemodLock(pDemod, pLock);//sony_dvb_demodC_CheckTSLock
			    SONY_TRACE_RETURN (result);
			}
			case SONY_DTV_SYSTEM_DVBT2:
			{
			    result =  sony_demod_dvbt2_CheckTSLock(pDemod, pLock);//sony_dvb_demodT2_CheckTSLock
			    SONY_TRACE_RETURN (result);
			}
			/* Intentional fall through. */
			case SONY_DTV_SYSTEM_UNKNOWN:
			default:
				*pLock = SONY_DEMOD_LOCK_RESULT_UNLOCKED;
				SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	    }
	}
	else 
	{
	    CXD2837_LOG(pDemod, "%s(): Unlock. pDemod->state = %d .\r\n", __FUNCTION__, pDemod->state);
	    *pLock = SONY_DEMOD_LOCK_RESULT_UNLOCKED;
	    SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
	}
}

static sony_result_t sony_integ_demod_WaitTSLock (sony_demod_t * pDemod,sony_dvbt2_profile_t profile)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    uint16_t timeout = 0;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;
    
    SONY_TRACE_ENTER ("sony_dvb_integration_WaitTSLock");

	/* Argument verification. */
	if (!pDemod )
	{
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	}
	if (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C)
	{
		/* This api is accepted in ACTIVE_T_C state only */
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
	}

    switch (pDemod->system) {
    case SONY_DTV_SYSTEM_DVBC:
        timeout = DTV_DEMOD_DVBC_WAIT_TS_LOCK;
        break;
    case SONY_DTV_SYSTEM_DVBT2:
		if (profile == SONY_DVBT2_PROFILE_BASE) {
			timeout = DTV_DEMOD_DVBT2_WAIT_LOCK;
		}
		else if ((profile == SONY_DVBT2_PROFILE_LITE) || (profile == SONY_DVBT2_PROFILE_ANY)) {
			timeout = DTV_DEMOD_DVBT2_LITE_WAIT_LOCK;
		}
		else {
			SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
		}
		break;
    case SONY_DTV_SYSTEM_DVBT:
        timeout = DTV_DEMOD_DVBT_WAIT_TS_LOCK;
        break;
/*    case SONY_DTV_SYSTEM_DVBC2:
        timeout = DTV_DEMOD_DVBC2_WAIT_TS_LOCK;
        break;*/
	/* Intentional fall-through. */
	case SONY_DTV_SYSTEM_UNKNOWN:
	default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Wait for TS lock */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
    for (;;) {

        // Check cancellation.
        if (pDemod->autoscan_stop_flag)
        {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }
        
        if ((pDemod->do_not_wait_t2_signal_locked) && ( SONY_DTV_SYSTEM_DVBT2== pDemod->system))
        {
           SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }

        result = sony_stopwatch_elapsed(&timer,&elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        if (elapsed >= timeout) {
            continueWait = 0;
        }

        result = sony_integ_demod_CheckTSLock(pDemod, &lock);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        switch (lock) {
        case SONY_DEMOD_LOCK_RESULT_LOCKED:
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        case SONY_DEMOD_LOCK_RESULT_UNLOCKED:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);

            /* Intentional fall-through. */
        case SONY_DEMOD_LOCK_RESULT_NOTDETECT:
        default:
            break;              /* continue waiting... */
        }

        if (continueWait) {
            result = sony_stopwatch_sleep (&timer, DEMOD_TUNE_POLL_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } 
        else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }

    SONY_TRACE_RETURN (result);
}
static sony_result_t sony_tuner_ascot2e_CalcGainFromAGC1 (sony_demod_t * pDemod, uint32_t agcReg, int32_t * pIFGain, int32_t * pRFGain)
{
    SONY_TRACE_ENTER ("sony_tuner_ascot2e_CalcGainFromAGC");

    if (!pDemod || !pIFGain || !pRFGain) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

   
    {
        int8_t if_bpf_gc = 0;
        int32_t agcRegX = (int32_t)agcReg * 227;
		
        //printf("%d,%d\n",pDemod->system,pDemod->Frequency);
        switch(pDemod->system){
        case SONY_DTV_SYSTEM_DVBT:
        case SONY_DTV_SYSTEM_DVBT2:
            if_bpf_gc = 12;
            break;
        case SONY_DTV_SYSTEM_DVBC:
            if_bpf_gc = 4;
            break;
        case SONY_DTV_SYSTEM_DVBC2:
            if_bpf_gc = 0;
            break;
        default:
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_SW_STATE);
        }

        if(agcRegX > 245760){
            *pIFGain = (6 + if_bpf_gc) * 100;
        }else if(agcRegX > 77824){
            *pIFGain = (6 + if_bpf_gc) * 100 + (69 * (245760 - agcRegX) + 2048) / 4096; /* Round */
        }else{
            *pIFGain = (6 + if_bpf_gc) * 100 + 69 * 41;
        }
    }

  
    {
        int32_t agcRegX = (int32_t)agcReg * 227;
        int32_t rfgainmax_x100 = 0;

        if(pDemod->Frequency > 900000){
            rfgainmax_x100 = 4320;
        }else if(pDemod->Frequency > 700000){
            rfgainmax_x100 = 4420;
        }else if(pDemod->Frequency > 600000){
            rfgainmax_x100 = 4330;
        }else if(pDemod->Frequency > 504000){
            rfgainmax_x100 = 4160;
        }else if(pDemod->Frequency > 400000){
            rfgainmax_x100 = 4550;
        }else if(pDemod->Frequency > 320000){
            rfgainmax_x100 = 4400;
        }else if(pDemod->Frequency > 270000){
            rfgainmax_x100 = 4520;
        }else if(pDemod->Frequency > 235000){
            rfgainmax_x100 = 4370;
        }else if(pDemod->Frequency > 192000){
            rfgainmax_x100 = 4190;
        }else if(pDemod->Frequency > 130000){
            rfgainmax_x100 = 4550;
        }else if(pDemod->Frequency > 86000){
            rfgainmax_x100 = 4630;
        }else if(pDemod->Frequency > 50000){
            rfgainmax_x100 = 4350;
        }else{
            rfgainmax_x100 = 4450;
        }

        if(agcRegX < 172032){
            *pRFGain = rfgainmax_x100;
        }else if(agcRegX < 200704){
            *pRFGain = rfgainmax_x100 - (63 * (agcRegX - 172032) + 2048) / 4096; /* Round */
        }else if(agcRegX < 245760){
            *pRFGain = rfgainmax_x100 - 63 * 7;
        }else if(agcRegX < 462848){
            *pRFGain = rfgainmax_x100 - 63 * 7 - (63 * (agcRegX - 245760) + 2048) / 4096; /* Round */
        }else if(agcRegX < 565248){
            *pRFGain = rfgainmax_x100 - 63 * 60 - (45 * (agcRegX - 462848) + 2048) / 4096; /* Round */
        }else{
            int32_t rfgrad = -95;

            *pRFGain = rfgainmax_x100 - 63 * 60 - 45 * 25 + (rfgrad * (agcRegX - 565248) - 2048) / 4096; /* Round */
        }
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}
static sony_result_t sony_integ_dvbt_t2_monitor_RFLevel (sony_demod_t * pDemod, int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_dvbt_t2_monitor_RFLevel");

    if ((!pDemod) || (!pRFLeveldB)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DTV_SYSTEM_DVBT) && (pDemod->system != SONY_DTV_SYSTEM_DVBT2)) {
        /* Not DVB-T or DVB-T2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch (pDemod->tunerOptimize) {
#ifndef SONY_INTEG_DISABLE_ASCOT_TUNER
    case SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D:
	
        {
            uint32_t ifAgc;

            if (pDemod->system == SONY_DTV_SYSTEM_DVBT) {
                result = sony_demod_dvbt_monitor_IFAGCOut(pDemod, &ifAgc);
            }
            else {
                result = sony_demod_dvbt2_monitor_IFAGCOut(pDemod, &ifAgc);
            }

            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            /* Protect against overflow. IFAGC is unsigned 12-bit. */
            if (ifAgc > 0xFFF) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
            }

            /* 2nd order polynomial relationship :
             * RF (dB) = -4E-6 * IFAGC^2 + 0.0594 * IFAGC - 131.24
             * RF (dB*1000) = ((-4 * IFAGC^2) + (59400* IFAGC) - 131240000) / 1000
             */
            *pRFLeveldB = (int32_t) ((-4 * (int32_t) (ifAgc * ifAgc)) + (59400 * (int32_t) ifAgc) - 131240000);
            *pRFLeveldB = (*pRFLeveldB < 0) ? *pRFLeveldB - 500 : *pRFLeveldB + 500;
            *pRFLeveldB /= 1000;
        }

        break;

    case SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2E:		
        {
            uint32_t ifAgc;
            int32_t ifGain;
            int32_t rfGain;


                if (pDemod->system == SONY_DTV_SYSTEM_DVBT) {
                    result = sony_demod_dvbt_monitor_IFAGCOut(pDemod, &ifAgc);
                }
                else {
                    result = sony_demod_dvbt2_monitor_IFAGCOut(pDemod, &ifAgc);
                }

                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                /* Protect against overflow. IFAGC is unsigned 12-bit. */
                if (ifAgc > 0xFFF) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
                }

                result = sony_tuner_ascot2e_CalcGainFromAGC1(pDemod, ifAgc, &ifGain, &rfGain);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                /* RF Level dBm = IFOUT - IFGAIN - RFGAIN
                 * IFOUT is the target IF level for tuner, -4.0dBm
                 */
                *pRFLeveldB = 10 * (-400 - ifGain - rfGain);


        }
        break;

    case SONY_DEMOD_TUNER_OPTIMIZE_ASCOT3:
			
        {

           // int32_t rssi = 0;
	      int32_t rssi=*pRFLeveldB;
#if 		0
                #ifndef SONY_DISABLE_I2C_REPEATER
                    /* Enable the I2C repeater */
                    result = sony_demod_I2cRepeaterEnable (pDemod, 0x01);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }
                #endif
                result = sony_tuner_ascot3_ReadRssi (pTunerTerrCable, &rssi);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                #ifndef SONY_DISABLE_I2C_REPEATER
                    /* Disable the I2C repeater */
                    result = sony_demod_I2cRepeaterEnable (pDemod, 0x00);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }
                #endif
#endif 
                /* RF Level dBm = RSSI + IFOUT
                 * IFOUT is the target IF level for tuner, -4.0dBm
                 */
                *pRFLeveldB = 10 * (rssi - 400);
                
                /* Note : An implementation specific offset may be required
                 * to compensate for component gains / attenuations */
   
        }
 	
        break;
		
#endif

    default:
        /* Please add RF level calculation for non ASCOT tuners. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    SONY_TRACE_RETURN (result);
}
static sony_result_t sony_integ_dvbc_monitor_RFLevel (sony_demod_t * pDemod, int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_dvbc_monitor_RFLevel");

    if ( (!pDemod) || (!pRFLeveldB)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch (pDemod->tunerOptimize) {
#ifndef SONY_INTEG_DISABLE_ASCOT_TUNER
    case SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D:
        {
            uint32_t ifAgc;

            result = sony_demod_dvbc_monitor_IFAGCOut(pDemod, &ifAgc);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            /* Protect against overflow. IFAGC is unsigned 12-bit. */
            if (ifAgc > 0xFFF) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
            }

            /* 2nd order polynomial relationship :
             * RF (dB) = -4E-6 * IFAGC^2 + 0.0594 * IFAGC - 131.24
             * RF (dB*1000) = ((-4 * IFAGC^2) + (59400* IFAGC) - 131240000) / 1000
             */
            *pRFLeveldB = (int32_t) ((-4 * (int32_t) (ifAgc * ifAgc)) + (59400 * (int32_t) ifAgc) - 131240000);
            *pRFLeveldB = (*pRFLeveldB < 0) ? *pRFLeveldB - 500 : *pRFLeveldB + 500;
            *pRFLeveldB /= 1000;
        }
        break;

    case SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2E:
        {
            uint32_t ifAgc;
            int32_t ifGain;
            int32_t rfGain;

                result = sony_demod_dvbc_monitor_IFAGCOut(pDemod, &ifAgc);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                /* Protect against overflow. IFAGC is unsigned 12-bit. */
                if (ifAgc > 0xFFF) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
                }

                result = sony_tuner_ascot2e_CalcGainFromAGC1(pDemod, ifAgc, &ifGain, &rfGain);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                /* RF Level dBm = IFOUT - IFGAIN - RFGAIN
                 * IFOUT is the target IF level for tuner, -3.5dBm
                 */
                *pRFLeveldB = 10 * (-350 - ifGain - rfGain);

                /* Note : An implementation specific offset may be required
                 * to compensate for component gains / attenuations */
        }
        break;

    case SONY_DEMOD_TUNER_OPTIMIZE_ASCOT3:
		
        {
			
          //  int32_t rssi = 0;
               int32_t rssi =*pRFLeveldB ;
#if 0
                #ifndef SONY_DISABLE_I2C_REPEATER
                    /* Enable the I2C repeater */
                    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }
                #endif

                result = sony_tuner_ascot3_ReadRssi (pInteg->pTunerTerrCable, &rssi);
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
#endif
                /* RF Level dBm = RSSI + IFOUT
                 * IFOUT is the target IF level for tuner, -5.5dBm
                 */
                *pRFLeveldB = 10 * (rssi - 550);
                
                /* Note : An implementation specific offset may be required
                 * to compensate for component gains / attenuations */
        }

        break;
		
#endif

    default:
        /* Please add RF level calculation for non ASCOT tuners. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    SONY_TRACE_RETURN (result);
}
static sony_result_t sony_demod_dvbt2_monitor_SSI (sony_demod_t * pDemod, uint8_t * pSSI, uint32_t rfLevel)
{
    //int32_t rfLevel;
    sony_dvbt2_plp_constell_t qam;
    sony_dvbt2_plp_code_rate_t codeRate;
    int32_t prel;
    int32_t tempSSI = 0;
    sony_result_t result = SONY_RESULT_OK;

    static const int32_t pRefdBm1000[4][8] = {
    /*    1/2,    3/5,    2/3,    3/4,    4/5,    5/6,    1/3,    2/5                */
        {-96000, -95000, -94000, -93000, -92000, -92000, -98000, -97000}, /* QPSK    */
        {-91000, -89000, -88000, -87000, -86000, -86000, -93000, -92000}, /* 16-QAM  */
        {-86000, -85000, -83000, -82000, -81000, -80000, -89000, -88000}, /* 64-QAM  */
        {-82000, -80000, -78000, -76000, -75000, -74000, -86000, -84000}, /* 256-QAM */
    };

    SONY_TRACE_ENTER ("sony_demod_dvbt2_monitor_SSI");
		
    if ((!pDemod) || (!pSSI)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBT2) {
        /* Not DVB-T2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    #if 0
    /* Get estimated RF Level */
    result = sony_integ_dvbt_t2_monitor_RFLevel (pDemod, &rfLevel);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
    #endif

    /* Get PLP constellation */
    result = sony_demod_dvbt2_monitor_QAM (pDemod, SONY_DVBT2_PLP_DATA, &qam);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Get PLP code rate */
    result = sony_demod_dvbt2_monitor_CodeRate (pDemod, SONY_DVBT2_PLP_DATA, &codeRate);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Ensure correct plp info. */
    if ((codeRate > SONY_DVBT2_R2_5) || (qam > SONY_DVBT2_QAM256)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    /* prel = prec - pref */
    prel = rfLevel - pRefdBm1000[qam][codeRate];

    /* SSI (Signal Strength Indicator) is calculated from:
     *
     * if (prel < -15dB)              SSI = 0
     * if (-15dB <= prel < 0dB)       SSI = (2/3) * (prel + 15)
     * if (0dB <= prel < 20dB)        SSI = 4 * prel + 10
     * if (20dB <= prel < 35dB)       SSI = (2/3) * (prel - 20) + 90
     * if (prel >= 35dB)              SSI = 100
     */
    if (prel < -15000) {
        tempSSI = 0;
    }
    else if (prel < 0) {
        /* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
        tempSSI = ((2 * (prel + 15000)) + 1500) / 3000;
    }
    else if (prel < 20000) {
        /* Note : prel scaled by 10^3 so divide by 10^3 added */
        tempSSI = (((4 * prel) + 500) / 1000) + 10;
    }
    else if (prel < 35000) {
        /* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
        tempSSI = (((2 * (prel - 20000)) + 1500) / 3000) + 90;
    }
    else {
        tempSSI = 100;
    }

    /* Clip value to 100% */
    *pSSI = (tempSSI > 100)? 100 : (uint8_t)tempSSI;

    SONY_TRACE_RETURN (result);
}


static sony_result_t sony_demod_dvbt_monitor_SSI (sony_demod_t * pDemod, uint8_t * pSSI, uint32_t rfLevel)
{
	//int32_t rfLevel;
	sony_dvbt_tpsinfo_t tps;
	int32_t prel;
	int32_t tempSSI = 0;
	sony_result_t result = SONY_RESULT_OK;

	static const int32_t pRefdBm1000[3][5] = {
		/*    1/2,    2/3,    3/4,    5/6,    7/8,               */
		{-93000, -91000, -90000, -89000, -88000}, /* QPSK    */
		{-87000, -85000, -84000, -83000, -82000}, /* 16-QAM  */
		{-82000, -80000, -78000, -77000, -76000}, /* 64-QAM  */
	};

	SONY_TRACE_ENTER ("sony_demod_dvbt_monitor_SSI");

	if ((!pDemod) || (!pSSI)) {
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	}

	if (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
		/* This api is accepted in ACTIVE state only */
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
	}

	if (pDemod->system != SONY_DTV_SYSTEM_DVBT) {
		/* Not DVB-T */
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
	}
	
	#if 0
	/* Get estimated RF Level */
	result = sony_integ_dvbt_t2_monitor_RFLevel (pDemod, &rfLevel);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}
	#endif

	/* Monitor TPS for Modulation / Code Rate */
	result = sony_demod_dvbt_monitor_TPSInfo (pDemod, &tps);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}

	/* Ensure correct TPS values. */
	if ((tps.constellation >= SONY_DVBT_CONSTELLATION_RESERVED_3) || (tps.rateHP >= SONY_DVBT_CODERATE_RESERVED_5)) {
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
	}

	/* prel = prec - pref */
	prel = rfLevel - pRefdBm1000[tps.constellation][tps.rateHP];

	/* SSI (Signal Strength Indicator) is calculated from:
	*
	* if (prel < -15dB)             SSI = 0
	* if (-15dB <= prel < 0dB)       SSI = (2/3) * (prel + 15)
	* if (0dB <= prel < 20dB)        SSI = (4 * prel) + 10
	* if (20dB <= prel < 35dB)       SSI = (2/3) * (prel - 20) + 90
	* if (prel >= 35dB)              SSI = 100
	*/
	if (prel < -15000) {
		tempSSI = 0;
	}
	else if (prel < 0) {
		/* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
		tempSSI = ((2 * (prel + 15000)) + 1500) / 3000;
	}
	else if (prel < 20000) {
		/* Note : prel scaled by 10^3 so divide by 10^3 added */
		tempSSI = (((4 * prel) + 500) / 1000) + 10;
	}
	else if (prel < 35000) {
		/* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
		tempSSI = (((2 * (prel - 20000)) + 1500) / 3000) + 90;
	}
	else {
		tempSSI = 100;
	}

	/* Clip value to 100% */
	*pSSI = (tempSSI > 100)? 100 : (uint8_t)tempSSI;

	SONY_TRACE_RETURN (result);
}

static sony_result_t sony_demod_dvbc_monitor_SSI (sony_demod_t * pDemod, uint8_t * pSSI, uint32_t rfLevel)
{
	sony_dvbc_constellation_t constellation;
	uint32_t symbolRate;
	int32_t prec;
	int32_t prel;
	int32_t pref;
	int32_t tempSSI = 0;  
	int32_t noiseFigureDB1000;
	sony_result_t result = SONY_RESULT_OK;

	/* Nordig spec C/N (Es/No) minimum performance 
	* Note: 32QAM isn't provided in the Nordig unified specification, so has been
	* Implemented based on interpolation and measurements. 
	*/
	static const int32_t cnrNordigdB1000[] = {
		/*  16QAM   32QAM   64QAM   128QAM  256QAM */
		20000,  23000,  26000,  29000,  32000  };

	SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_SSI");

	if ((!pDemod) || (!pSSI)) {
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	}

	if (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
		/* This api is accepted in ACTIVE state only */
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
	}

	if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
		/* Not DVB-C*/
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
	}

	/* Get estimated RF Level 
	result = sony_integ_dvbc_monitor_RFLevel (pInteg, &prec);
	if (result != SONY_RESULT_OK) {
	SONY_TRACE_RETURN (result);
	}*/
	//prec = rfLevel;	

	/* Monitor constellation */
	result = sony_demod_dvbc_monitor_QAM (pDemod, &constellation);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}

	/* Monitor symbol rate */
	result = sony_demod_dvbc_monitor_SymbolRate(pDemod, &symbolRate);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}    

	/* Modify this to suit the tuner noise figure specification value in dB * 1000 */
	noiseFigureDB1000 = 9000; 

	/* Reference sensitivity limit is calcualted from:
	* RefLevel (dB) = (10 * Log(1.38*10^-23 * 290)) + 30 + NoiseFigure + (10 * Log(symbolRate)) + C/N_Offset
	*  - sony_math_log10(x)     = 100 * Log(x)
	*  - Log(1.38*10^-23 * 290) = -20.3977
	*
	* Therefore:
	* RefLevel (dB * 1000) = -203977 + 30000 + (1000 * NoiseFigure) + (100 * Log(symbolRate)) + (1000 * C/N_Offset)
	*/
	pref = -203977 + 30000 + noiseFigureDB1000 + (100 * sony_math_log10(symbolRate* 1000)) + cnrNordigdB1000[(uint8_t)constellation];

	/* prel = prec - pref */
	prel = prec - pref;

	/* SSI (Signal Strength Indicator) is calculated from:
	*
	* if (prel < -15dB)              SSI = 0
	* if (-15dB <= prel < 0dB)       SSI = (2/3) * (prel + 15)
	* if (0dB <= prel < 20dB)        SSI = 4 * prel + 10
	* if (20dB <= prel < 35dB)       SSI = (2/3) * (prel - 20) + 90
	* if (prel >= 35dB)              SSI = 100
	*/
	if (prel < -15000) {
		tempSSI = 0;
	}
	else if (prel < 0) {
		/* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
		tempSSI = ((2 * (prel + 15000)) + 1500) / 3000;
	}
	else if (prel < 20000) {
		/* Note : prel scaled by 10^3 so divide by 10^3 added */
		tempSSI = (((4 * prel) + 500) / 1000) + 10;
	}
	else if (prel < 35000) {
		/* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
		tempSSI = (((2 * (prel - 20000)) + 1500) / 3000) + 90;
	}
	else {
		tempSSI = 100;
	}

	/* Clip value to 100% */
	*pSSI = (tempSSI > 100)? 100 : (uint8_t)tempSSI;

	SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_CalcSSI(sony_demod_t * pDemod, uint32_t *pSSI, uint32_t rfLevel)
{	
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

    if((pDemod == NULL)||(pSSI == NULL))
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

    result = sony_integ_demod_CheckTSLock(pDemod, &lock);
    if( lock != SONY_DEMOD_LOCK_RESULT_LOCKED)
    {	
        result = SONY_RESULT_ERROR_UNLOCK;
        //CXD2837_LOG(pDemod, "[%s]:TS Unlock!\n", __FUNCTION__);
        return result;		
    }
    else	
    {
        CXD2837_LOG(pDemod, "[%s]:TS lock!\n", __FUNCTION__);

        switch(pDemod->system)
        {
            case SONY_DTV_SYSTEM_DVBT2:
            {
                result = sony_demod_dvbt2_monitor_SSI(pDemod,(uint8_t*)pSSI,rfLevel);//sony_dvb_demod_monitorT2_IFAGCOut
            }	
            break;
            case SONY_DTV_SYSTEM_DVBT:
            {
                result = sony_demod_dvbt_monitor_SSI(pDemod,(uint8_t*)pSSI,rfLevel);
            }	
            break;
            case SONY_DTV_SYSTEM_DVBC:
            {
                result = sony_demod_dvbc_monitor_SSI(pDemod,(uint8_t*)pSSI,rfLevel);//sony_dvb_demod_monitorC_IFAGCOut
            }	
            break;
            default:
                return SONY_RESULT_ERROR_ARG;
        }
    }

    if(result != SONY_RESULT_OK)
    {
    //SONY_SLEEP(10);
    CXD2837_LOG(pDemod, "[sony_integ_CalcSSI]:failed,result=%d\n",result);
    }

    return result;
}

sony_result_t sony_integ_CalcSQI(sony_demod_t * pDemod, uint32_t *pSQI)
{	
	sony_result_t result = SONY_RESULT_OK;
	sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
	
	if((pDemod == NULL)||(pSQI == NULL))
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    
	result = sony_integ_demod_CheckTSLock(pDemod, &lock);
	if( lock != SONY_DEMOD_LOCK_RESULT_LOCKED)
	{
		result = SONY_RESULT_ERROR_UNLOCK;
		//CXD2837_LOG(pDemod, "[%s]:TS Unlock!\n", __FUNCTION__);
		return result;		
	}
	else	
	{
    		switch(pDemod->system)
	    	{
	    		case SONY_DTV_SYSTEM_DVBT2:
	    		{
	    			result = sony_demod_dvbt2_monitor_Quality(pDemod,pSQI);//sony_dvb_demod_monitorT2_Quality
	    		}	
	    		break;
	    		case SONY_DTV_SYSTEM_DVBT:
	    		{
	    			result = sony_demod_dvbt_monitor_Quality(pDemod,pSQI);
	    		}	
	    		break;
	    		case SONY_DTV_SYSTEM_DVBC:
	    		{
	    			result = sony_demod_dvbc_monitor_Quality(pDemod,pSQI);//sony_dvb_demod_monitorC_SNR
	    		}	
	    		break;
	    		default:
	    			return SONY_RESULT_ERROR_ARG;
	    	}
	}
	
	if(result != SONY_RESULT_OK)
	{
		//SONY_SLEEP(10);
		CXD2837_LOG(pDemod, "[sony_integ_CalcSQI]:failed,result=%d\n",result);
	}

	return result;
}

sony_result_t sony_integ_CalcBER(sony_demod_t * pDemod, uint32_t *pBER)
{	
	sony_result_t result = SONY_RESULT_OK;
	sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

	UINT8	cn=0;
	
	if((pDemod == NULL)||(pBER == NULL))
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	
	cn = 0;	
	*pBER = 1;
    
	result = sony_integ_demod_CheckTSLock(pDemod, &lock);
    if( lock != SONY_DEMOD_LOCK_RESULT_LOCKED)
	{
		result = SONY_RESULT_ERROR_UNLOCK;
		//CXD2837_LOG(pDemod, "[%s]:TS Unlock!\n", __FUNCTION__);
		return result;	
	}
	else	
	{
    	switch(pDemod->system)
    	{
    		case SONY_DTV_SYSTEM_DVBT2:
    		{
    			result = sony_demod_dvbt2_monitor_PreBCHBER(pDemod,pBER);//sony_dvb_demod_monitorT2_PreBCHBER
    		}	
    		break;
    		case SONY_DTV_SYSTEM_DVBT:
    		{
    			result = sony_demod_dvbt_monitor_PreRSBER(pDemod,pBER);
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2837_LOG(pDemod, "sony_demod_dvbt_monitor_PreRSBER()=%d, failed!\n", result);
    			}
    			*pBER = (*pBER)*100;
    		}	
    		break;
    		case SONY_DTV_SYSTEM_DVBC:
    		{
    			result = sony_demod_dvbc_monitor_PreRSBER(pDemod,pBER);//sony_dvb_demod_monitorC_PreRSBER
    			if(result != SONY_RESULT_OK)
    			{
    				CXD2837_LOG(pDemod, "[sony_demod_dvbc_monitor_PreRSBER] -> failed!\n");
    			}
    			*pBER = (*pBER)*100;
    		}	
    		break;
    		default:
    			return SONY_RESULT_ERROR_ARG;
    	}
	}

	if(result != SONY_RESULT_OK)
	{
		//SONY_SLEEP(10);
		CXD2837_LOG(pDemod, "[sony_integ_CalcBER]:failed,result=%d!\n", result);
	}

	if(*pBER == 0)
		*pBER = 1;
	while(*pBER/10 >=1)
	{
		cn++;
		(*pBER) /= 10;
	}
	
	return  SONY_RESULT_OK;
}

/*Check DVBC/T/T2 Demod Lock status*/
static sony_result_t sony_integ_demod_CheckLock (sony_demod_t * pDemod, sony_demod_lock_result_t * pLock)
{
	sony_result_t result = SONY_RESULT_OK;
	SONY_TRACE_ENTER ("sony_integ_demod_CheckLock");
	
	if (!pDemod || !pLock)
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

	/* Software state check */
	if (pDemod->state == SONY_DEMOD_STATE_ACTIVE_T_C)
	{
		switch (pDemod->system)
		{
			case SONY_DTV_SYSTEM_DVBT:
			{
			    result =  sony_demod_dvbt_CheckDemodLock(pDemod, pLock);//sony_dvb_demodT_CheckTSLock
			    SONY_TRACE_RETURN (result);
			}
			case SONY_DTV_SYSTEM_DVBC:
			{
			    result =  sony_demod_dvbc_CheckDemodLock(pDemod, pLock);//sony_dvb_demodC_CheckTSLock
			    SONY_TRACE_RETURN (result);
			}
			case SONY_DTV_SYSTEM_DVBT2:
			{
			    result =  sony_demod_dvbt2_CheckDemodLock(pDemod, pLock);//sony_dvb_demodT2_CheckTSLock
			    SONY_TRACE_RETURN (result);
			}
			/* Intentional fall through. */
			case SONY_DTV_SYSTEM_UNKNOWN:
			default:
				*pLock = SONY_DEMOD_LOCK_RESULT_UNLOCKED;
				SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	    }
	}
	else 
	{
        CXD2837_LOG(pDemod, "%s(): Unlock. pDemod->state = %d .\r\n", __FUNCTION__, pDemod->state);
        *pLock = SONY_DEMOD_LOCK_RESULT_UNLOCKED;
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
    }
}

static sony_result_t sony_integ_demod_WaitDemodLock (sony_demod_t * pDemod, sony_dvbt2_profile_t profile)
{
	sony_result_t result = SONY_RESULT_OK;
	sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
	uint16_t timeout = 0;
	sony_stopwatch_t timer;
	uint8_t continueWait = 1;
	uint32_t elapsed = 0;

	SONY_TRACE_ENTER ("sony_integ_demod_WaitDemodLock");

	if (!pDemod) {
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	}

	if (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
		/* This api is accepted in ACTIVE_T_C state only */
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
	}

	switch (pDemod->system) 
	{
		case SONY_DTV_SYSTEM_DVBC:
			timeout = DTV_DEMOD_DVBC_WAIT_LOCK;
			break;
		case SONY_DTV_SYSTEM_DVBT2:
			if (profile == SONY_DVBT2_PROFILE_BASE) {
				timeout = DTV_DEMOD_DVBT2_WAIT_LOCK;
			}
			else if ((profile == SONY_DVBT2_PROFILE_LITE) || (profile == SONY_DVBT2_PROFILE_ANY)) {
				timeout = DTV_DEMOD_DVBT2_LITE_WAIT_LOCK;
			}
			else {
				SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
			}
			break;
		case SONY_DTV_SYSTEM_DVBT:
			timeout = DTV_DEMOD_DVBT_WAIT_LOCK;
			break;
		/* Intentional fall-through. */
		case SONY_DTV_SYSTEM_UNKNOWN:
		default:
			SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	}

	/* Wait for demod lock */
	result = sony_stopwatch_start (&timer);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}

	for (;;) 
	{
		result = sony_stopwatch_elapsed(&timer, &elapsed);
		if (result != SONY_RESULT_OK) {
			SONY_TRACE_RETURN (result);
		}

		if (elapsed >= timeout) {
			continueWait = 0;
		}

		result = sony_integ_demod_CheckLock (pDemod, &lock);
		if (result != SONY_RESULT_OK) {
			SONY_TRACE_RETURN (result);
		}

		switch (lock) 
		{
			case SONY_DEMOD_LOCK_RESULT_LOCKED:
				SONY_TRACE_RETURN (SONY_RESULT_OK);

			case SONY_DEMOD_LOCK_RESULT_UNLOCKED:
				SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);

			default:
				/* continue waiting... */
				break;              
		}

		/* Check cancellation. */
		//if (sony_atomic_read (&(pInteg->cancel)) != 0) {
		//SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
		//}

		if (continueWait) 
		{
			result = sony_stopwatch_sleep (&timer, DTV_DEMOD_WAIT_LOCK_INTERVAL);
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

static sony_result_t sony_demod_dvbt2_WaitL1PostLock (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;
    uint8_t l1PostValid;

    SONY_TRACE_ENTER ("sony_demod_dvbt2_WaitL1PostLock");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
  
        /* Check for timeout condition */
        if (elapsed >= DTV_DEMOD_TUNE_T2_L1POST_TIMEOUT/*SONY_DVBT2_L1POST_TIMEOUT*/) {
            continueWait = 0;
        }

        result = sony_demod_dvbt2_CheckL1PostValid (pDemod, &l1PostValid);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* If L1 Post is valid, return from loop, else continue waiting */
        if (l1PostValid) {
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        }
        
        /* Check cancellation.
        if (sony_atomic_read (&(pInteg->cancel)) != 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }
 		*/
 		
        if (continueWait) {
            result = sony_stopwatch_sleep (&timer, DTV_DEMOD_WAIT_LOCK_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } 
        else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }
    
    SONY_TRACE_RETURN (result);
}


sony_result_t sony_integ_dvbt_Tune(sony_demod_t * pDemod, sony_dvbt_tune_param_t * pTuneParam, BOOL NeedToConfigTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_dvbt_Tune");

    if ((!pDemod) || (!pTuneParam)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP_T_C) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C)) {
        /* This api is accepted in SLEEP_T_C and ACTIVE_T_C states only */
        printk("[%s]:line=%d,pDemod->state = %d,\n",__FUNCTION__,__LINE__,pDemod->state);
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancellation flag. */
    //sony_atomic_set (&(pInteg->cancel), 0);

    /* Check bandwidth validity */
    if ((pTuneParam->bandwidth != SONY_DEMOD_BW_5_MHZ) && (pTuneParam->bandwidth != SONY_DEMOD_BW_6_MHZ) && 
        (pTuneParam->bandwidth != SONY_DEMOD_BW_7_MHZ) && (pTuneParam->bandwidth != SONY_DEMOD_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    /* Set DVB-T profile for acquisition */
    result = sony_demod_dvbt_SetProfile(pDemod, pTuneParam->profile);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Tune the demodulator */
    result = sony_demod_dvbt_Tune (pDemod, pTuneParam);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

   // if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Tune)) {
#ifndef SONY_DISABLE_I2C_REPEATER
        /* Enable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pDemod, 0x01);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
#endif

		/* Tune the RF part */
		//result = pInteg->pTunerTerrCable->Tune (pInteg->pTunerTerrCable, pTuneParam->centerFreqKHz, SONY_DTV_SYSTEM_DVBT, pTuneParam->bandwidth);
		//if (result != SONY_RESULT_OK) {
		//	SONY_TRACE_RETURN (result);
		//}
		if(NeedToConfigTuner){
			if(pDemod->tuner_control.nim_tuner_control(pDemod->tuner_id, pTuneParam->centerFreqKHz, pTuneParam->bandwidth, 0, (UINT8*)&(pDemod->system),0) == ERR_FAILUE)
			{
				result = SONY_RESULT_ERROR_I2C;
				SONY_TRACE_RETURN (result);
			}
		}
#ifndef SONY_DISABLE_I2C_REPEATER
        /* Disable the I2C repeater */
        result = sony_demod_I2cRepeaterEnable (pDemod, 0x00);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
#endif
    //}

    /* Reset the demod to enable acquisition */
    result = cxd2837_demod_TuneEnd (pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }    

    /* Wait for TS lock */
    result = sony_integ_demod_WaitTSLock(pDemod, pTuneParam->profile);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }    

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbt2_Tune(sony_demod_t * pDemod, sony_dvbt2_tune_param_t * pTuneParam,BOOL NeedToConfigTuner)
{
	sony_result_t result = SONY_RESULT_OK;

	SONY_TRACE_ENTER ("sony_integ_dvbt2_Tune");

	 if ((!pTuneParam) || (!pDemod)) {
	     SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	 }
    
	if ((pDemod->state != SONY_DEMOD_STATE_SLEEP_T_C) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C)) {
	  /* This api is accepted in SLEEP_T_C and ACTIVE_T_C states only */
	  SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
	}

	/* Clear cancellation flag. */
	// sony_atomic_set (&(pInteg->cancel), 0);

	/* Check bandwidth validity */
	if ((pTuneParam->bandwidth != SONY_DEMOD_BW_1_7_MHZ) && (pTuneParam->bandwidth != SONY_DEMOD_BW_5_MHZ) && 
	  (pTuneParam->bandwidth != SONY_DEMOD_BW_6_MHZ) && (pTuneParam->bandwidth != SONY_DEMOD_BW_7_MHZ) && 
	  (pTuneParam->bandwidth != SONY_DEMOD_BW_8_MHZ)) {
	  SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
	}
    
	/* Check for valid profile selection */
	if ((pTuneParam->profile != SONY_DVBT2_PROFILE_BASE) && (pTuneParam->profile != SONY_DVBT2_PROFILE_LITE)) {
	  SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	}

	/* Configure for manual PLP selection. */
	result = sony_demod_dvbt2_SetPLPConfig (pDemod, 0x00, pTuneParam->dataPlpId);
	if (result != SONY_RESULT_OK) {
	  SONY_TRACE_RETURN (result);
	}

	/* Configure the DVB-T2 profile without recovery */
	result = sony_demod_dvbt2_SetProfile(pDemod, pTuneParam->profile);
	if (result != SONY_RESULT_OK) {
	  SONY_TRACE_RETURN (result);
	}

	/* Tune the demodulator */
	result = sony_demod_dvbt2_Tune (pDemod, pTuneParam);
	if (result != SONY_RESULT_OK) {
	  SONY_TRACE_RETURN (result);
	}

	//if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Tune)) {
#ifndef SONY_DISABLE_I2C_REPEATER
	/* Enable the I2C repeater */
	result = sony_demod_I2cRepeaterEnable (pDemod, 0x01);
	if (result != SONY_RESULT_OK) {
	   SONY_TRACE_RETURN (result);
	}
#endif

	/* Tune the RF part 
	result = pInteg->pTunerTerrCable->Tune (pInteg->pTunerTerrCable, pTuneParam->centerFreqKHz, SONY_DTV_SYSTEM_DVBT2, pTuneParam->bandwidth);
	if (result != SONY_RESULT_OK) {
	SONY_TRACE_RETURN (result);
	}
	*/
	if(NeedToConfigTuner){
		if(pDemod->tuner_control.nim_tuner_control(pDemod->tuner_id, pTuneParam->centerFreqKHz, pTuneParam->bandwidth, 0, (UINT8*)&(pDemod->system),0) == ERR_FAILUE)
		{
			result = SONY_RESULT_ERROR_I2C;
			SONY_TRACE_RETURN (result);
		}
	}
		
#ifndef SONY_DISABLE_I2C_REPEATER
	/* Disable the I2C repeater */
	result = sony_demod_I2cRepeaterEnable (pDemod, 0x00);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}
#endif
    //}

    /* Reset the demod to enable acquisition */
    result = cxd2837_demod_TuneEnd (pDemod);

    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }    

	/* Wait for demodulator lock */
	result = sony_integ_demod_WaitDemodLock(pDemod, pTuneParam->profile);
	if (result != SONY_RESULT_OK) {
	  SONY_TRACE_RETURN (result);
	}    

	/* Wait for TS lock */
	result = sony_integ_demod_WaitTSLock(pDemod,pTuneParam->profile);

	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	} 
	 
    /* In DVB-T2, L1 Post information may not immediately be valid after acquisition
     * (L1POST_OK bit != 1).  This wait loop handles such cases.  This issue occurs 
     * only under clean signal lab conditions, and will therefore not extend acquistion 
     * time under normal conditions.
     */ 
	result = sony_demod_dvbt2_WaitL1PostLock(pDemod);
		
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}    

    /* Confirm correct PLP selection in acquisition */
    {
		uint8_t plpNotFound;

		result = sony_demod_dvbt2_monitor_DataPLPError (pDemod, &plpNotFound);
		if (result == SONY_RESULT_ERROR_HW_STATE) {
			/* Demod lock is lost causing monitor to fail, return UNLOCK instead of HW STATE */
			SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
		}
		else if (result != SONY_RESULT_OK) {
			/* Serious error, so return result */
			SONY_TRACE_RETURN (result);
		}   

		if (plpNotFound)
		{
			result = SONY_RESULT_OK_CONFIRM;
			pTuneParam->tuneInfo = SONY_DEMOD_DVBT2_TUNE_INFO_INVALID_PLP_ID;
		}  
		else {
			pTuneParam->tuneInfo = SONY_DEMOD_DVBT2_TUNE_INFO_OK;
		}
	}
	SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbt2_BlindTune(sony_demod_t * pDemod, sony_dvbt2_tune_param_t * pTuneParam, BOOL NeedToConfigTuner, UINT8 t2_lite_support_flag)
{
	sony_result_t result = SONY_RESULT_OK;

	SONY_TRACE_ENTER ("sony_integ_dvbt2_BlindTune");

	if ((!pTuneParam) || (!pDemod)) {
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
	}

	if ((pDemod->state != SONY_DEMOD_STATE_SLEEP_T_C) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C)) {
		/* This api is accepted in SLEEP_T_C and ACTIVE_T_C states only */
		SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
	}

#ifdef DVBT2_LITE_SUPPORT
if(t2_lite_support_flag == 1)
	pTuneParam->profile=SONY_DVBT2_PROFILE_ANY;
else
#endif	
	pTuneParam->profile=SONY_DVBT2_PROFILE_BASE;

	/* Clear cancellation flag. */
	// sony_atomic_set (&(pInteg->cancel), 0);

	/* Check bandwidth validity */
	if ((pTuneParam->bandwidth != SONY_DEMOD_BW_1_7_MHZ) && (pTuneParam->bandwidth != SONY_DEMOD_BW_5_MHZ) && 
	  (pTuneParam->bandwidth != SONY_DEMOD_BW_6_MHZ) && (pTuneParam->bandwidth != SONY_DEMOD_BW_7_MHZ) && 
	  (pTuneParam->bandwidth != SONY_DEMOD_BW_8_MHZ)) {
	  SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
	}

	/* Configure for manual PLP selection. */
	result = sony_demod_dvbt2_SetPLPConfig (pDemod, 0x01,0x00);//By SONY AEC 20130701
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}

	/* Configure the DVB-T2 profile without recovery */
	result = sony_demod_dvbt2_SetProfile(pDemod,  pTuneParam->profile);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}

	/* Tune the demodulator */
	result = sony_demod_dvbt2_Tune (pDemod, pTuneParam);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}

	//if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Tune)) {
#ifndef SONY_DISABLE_I2C_REPEATER
	/* Enable the I2C repeater */
	result = sony_demod_I2cRepeaterEnable (pDemod, 0x01);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}
#endif

	/* Tune the RF part 
	result = pInteg->pTunerTerrCable->Tune (pInteg->pTunerTerrCable, pTuneParam->centerFreqKHz, SONY_DTV_SYSTEM_DVBT2, pTuneParam->bandwidth);
	if (result != SONY_RESULT_OK) {
	SONY_TRACE_RETURN (result);
	}
	*/
	if(NeedToConfigTuner){
		if(pDemod->tuner_control.nim_tuner_control(pDemod->tuner_id, pTuneParam->centerFreqKHz, pTuneParam->bandwidth, 0, (UINT8*)&(pDemod->system),0) == ERR_FAILUE)
		{
			result = SONY_RESULT_ERROR_I2C;
			SONY_TRACE_RETURN (result);
		}
	}
		
#ifndef SONY_DISABLE_I2C_REPEATER
	/* Disable the I2C repeater */
	result = sony_demod_I2cRepeaterEnable (pDemod, 0x00);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}
#endif
    //}

	/* Reset the demod to enable acquisition */
	result = cxd2837_demod_TuneEnd (pDemod);
	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	}    

	/* Wait for demodulator lock */
	result = sony_integ_demod_WaitDemodLock(pDemod, pTuneParam->profile);

	/* Wait for TS lock */
	result = sony_integ_demod_WaitTSLock(pDemod,pTuneParam->profile);

	if (result != SONY_RESULT_OK) {
		SONY_TRACE_RETURN (result);
	} 
	//add by AEC 20130710
	switch (result) {
	case SONY_RESULT_OK:
		/* In DVB-T2, L1 Post information may not immediately be valid after acquisition
		* (L1POST_OK bit != 1).  This wait loop handles such cases.  This issue occurs 
		* only under clean signal lab conditions, and will therefore not extend acquistion 
		* time under normal conditions.
		*/    
		result = sony_demod_dvbt2_WaitL1PostLock (pDemod);
		if (result != SONY_RESULT_OK) {
			SONY_TRACE_RETURN (result);
		}  
		{
			sony_dvbt2_profile_t ProfileFound;
			/* Obtain the current profile if detection was automatic. */
			result = sony_demod_dvbt2_monitor_Profile (pDemod, &ProfileFound);
			if (result == SONY_RESULT_ERROR_HW_STATE) {
				/* Demod lock is lost causing monitor to fail, return UNLOCK instead of HW STATE */
				SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
			}
			else if (result != SONY_RESULT_OK) {
				/* Serious error, so return result */
				SONY_TRACE_RETURN (result);
			}
			pDemod->t2_profile = ProfileFound;
			//libc_printf("[%s]ProfileFound:%d\n",__FUNCTION__,ProfileFound);
		}
		break;

	/* Intentional fall-through */
	case SONY_RESULT_ERROR_TIMEOUT:
	case SONY_RESULT_ERROR_UNLOCK:
		break;
	default:
		SONY_TRACE_RETURN(result);
	}
	//end add by AEC 20130710   
	/* Confirm correct PLP selection in acquisition */
	{
		uint8_t plpNotFound;

		result = sony_demod_dvbt2_monitor_DataPLPError (pDemod, &plpNotFound);
		if (result == SONY_RESULT_ERROR_HW_STATE) {
			/* Demod lock is lost causing monitor to fail, return UNLOCK instead of HW STATE */
			SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);
		}
		else if (result != SONY_RESULT_OK) {
			/* Serious error, so return result */
			SONY_TRACE_RETURN (result);
		}

		if (plpNotFound)
		{
			result = SONY_RESULT_OK_CONFIRM;
			pTuneParam->tuneInfo = SONY_DEMOD_DVBT2_TUNE_INFO_INVALID_PLP_ID;
		}  
		else {
			pTuneParam->tuneInfo = SONY_DEMOD_DVBT2_TUNE_INFO_OK;
		}
	}	
   
    SONY_TRACE_RETURN (result);
}
