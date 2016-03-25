/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: nim_cxd2838.c
*
*    Description: cxd2838 nim driver for linux api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/



#include "porting_cxd2838_linux.h"
#include "sony_demod.h"
//#include "sony_demod_integ.h"
#include "nim_cxd2838.h"
//#include "sony_demod_integ.h"
#include "sony_demod_isdbt.h"
#include "sony_integ_isdbt.h"

#define sony_demod_GPIOSetConfig cxd2838_demod_GPIOSetConfig
#define sony_demod_SetConfig    cxd2838_demod_SetConfig

void cxd2838_log_i2c(sony_demod_t* param, UINT8 err, UINT8 write, UINT8 slv_addr, UINT8 *data, int len)
{
    int i = 0;
   // if ( ! (param->output_buffer && param->fn_output_string) )
   //     return;

    if (write)
        CXD2838_PRINTF("I2C_Write,0x%02X", (slv_addr&0xFE));
    else
        CXD2838_PRINTF("I2C_Read,0x%02X", (slv_addr|1));

    for ( i=0; i<len; ++i )
    {
        CXD2838_PRINTF(",0x%02X", data[i]);
    }

    if (err)
        CXD2838_PRINTF("\terror");
    CXD2838_PRINTF("\r\n");
}
static INT32 config_tuner(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, UINT8 *ptr_lock)
{
	sony_demod_t * priv = (sony_demod_t *)dev->priv;
	UINT32 tuner_id = priv->tuner_id;
	UINT8 lock = 0, Tuner_Retry = 0;

	//set_bypass_mode(dev, TRUE);
	do
	{
		Tuner_Retry++;

		if(priv->tuner_control.nim_tuner_control(tuner_id, freq, bandwidth, 0, (UINT8*)&(priv->system),0) == ERR_FAILUE)
		{
			CXD2838_PRINTF("Config tuner failed, I2c failed!\r\n");
		}

		if(priv->tuner_control.nim_tuner_status(tuner_id, &lock) != SUCCESS)
		{
			//if i2c read failure, no lock state can be report.
			lock = 0;
			CXD2838_PRINTF("Config tuner failed, I2c failed!\r\n");
		}
		CXD2838_PRINTF("Tuner Lock Times=0x%d,Status=0x%d\r\n",Tuner_Retry,lock);

		if(Tuner_Retry > 5)
			break;
	}while(0 == lock);

	if(ptr_lock != NULL)
		*ptr_lock = lock;

	if(Tuner_Retry > 5)
	{
		CXD2838_PRINTF("ERROR! Tuner Lock Fail\r\n");
		return ERR_FAILUE;
	}
	return SUCCESS;
}

static BOOL need_to_config_tuner(struct nim_device *dev, UINT32 freq, UINT32 bandwidth)
{
    sony_demod_t * param = (sony_demod_t *)dev->priv;
    UINT32 tuner_id = param->tuner_id;
    UINT8 lock = 0;
		
    return ! ( param->Frequency == freq \
            && param->bandwidth == bandwidth \
            && param->tuner_control.nim_tuner_status(tuner_id, &lock) == SUCCESS);
}

BOOL need_to_lock_ISDBT_signal(struct nim_device *dev, NIM_CHANNEL_CHANGE_T *change_para, BOOL NeedToConfigTuner)
{
    sony_demod_t * param = (sony_demod_t *)dev->priv;

    if ( change_para->usage_type == USAGE_TYPE_AUTOSCAN \
        || change_para->usage_type == USAGE_TYPE_CHANSCAN \
        || change_para->usage_type == USAGE_TYPE_AERIALTUNE )
        return TRUE;    //Auto detect signal type for Auto Scan and Channel Scan.

    if ( change_para->usage_type == USAGE_TYPE_NEXT_PIPE_SCAN )
        return FALSE;
    param->system = SONY_DTV_SYSTEM_ISDBT;
    return NeedToConfigTuner;
}

INT32 try_to_lock_ISDBT_signal(struct nim_device *dev, BOOL NeedToInitSystem,BOOL NeedToConfigTuner,NIM_CHANNEL_CHANGE_T *change_para)
{
	sony_demod_t * pDemod = (sony_demod_t *)dev->priv;
	sony_isdbt_tune_param_t TuneParam;
	sony_result_t tuneResult;

	//memcpy(&pInteg.pDemod,(sony_demod_t *)dev->priv,sizeof(pInteg.pDemod));

	memset(&TuneParam,0,sizeof(TuneParam));
	TuneParam.centerFreqKHz = change_para->freq;
	TuneParam.bandwidth = change_para->bandwidth;
	//TuneParam.profile = change_para->priority; //SONY_DVBT_PROFILE_HP;

	if( NeedToInitSystem || (pDemod->system != SONY_DTV_SYSTEM_ISDBT))
	{
		pDemod->t2_signal = 2;
		pDemod->system = SONY_DTV_SYSTEM_ISDBT;
		//to do...
	}
	pDemod->system = SONY_DTV_SYSTEM_ISDBT;

	tuneResult = sony_integ_isdbt_Tune(pDemod, &TuneParam);
	switch(tuneResult){
	case SONY_RESULT_OK:
		 CXD2838_PRINTF("[SONY]ISDBT TS Locked.\n");
		return SUCCESS;
	case SONY_RESULT_ERROR_UNLOCK:
		CXD2838_PRINTF("[SONY]ISDBT TS Unlocked.\n");
		return ERR_FAILUE;
	case SONY_RESULT_ERROR_TIMEOUT:
		CXD2838_PRINTF(("[SONY]ISDBT Wait TS Lock but Timeout.\n"));
		return ERR_FAILUE;
	default:
		CXD2838_PRINTF("[SONY] default: in sony_integ_isdbt_Tune,tuneResult = %d\n",tuneResult);
		return ERR_FAILUE;
	}
}

INT32 nim_cxd2838_channel_change_smart(struct nim_device *dev,NIM_CHANNEL_CHANGE_T *change_para)
{	
	INT32 result = SUCCESS;

	sony_demod_t * param = (sony_demod_t *)dev->priv;
	BOOL play_program, NeedToInitSystem = FALSE, NeedToConfigTuner = FALSE;

	if ((change_para->freq <= 40000) || (change_para->freq >= 900000))
	{
		return ERR_FAILUE;
	}
	
	mutex_lock(&param->demodMode_mutex_id);//mutex_lock() puts the calling thread to sleep.
	
	CXD2838_PRINTF("[%s]:line=%d,system:%d\r\n",__FUNCTION__,__LINE__,param->system);

	if ( change_para->bandwidth != SONY_DEMOD_BW_6_MHZ && change_para->bandwidth != SONY_DEMOD_BW_7_MHZ
        && change_para->bandwidth != SONY_DEMOD_BW_8_MHZ)
	{
	    CXD2838_PRINTF("[%s]:line=%d error:bandwidth=%d \n", __FUNCTION__,__LINE__,change_para->bandwidth);
        mutex_unlock(&param->demodMode_mutex_id);
		return ERR_FAILUE;
	}
	
	#if 0
	result = osal_flag_wait(&flgptn, param->flag_id, NIM_SCAN_END, OSAL_TWF_ANDW|OSAL_TWF_CLR, 2000); //OSAL_WAIT_FOREVER_TIME
	if(OSAL_E_OK != result)
	{
		return ERR_FAILUE;
	}
    #endif
	CXD2838_PRINTF("%s: usage_type %d, freq %d, bandwidth %d, priority %d, t2_signal %d, plp_index %d, plp_id  %d, profile %d\r\n",
                __FUNCTION__,change_para->usage_type, change_para->freq,change_para->bandwidth, change_para->priority, 
                change_para->t2_signal, change_para->plp_index, change_para->plp_id, change_para->t2_profile);

	change_para->usage_type = 0;

	do
	{
		//osal_mutex_lock(param->demodMode_mutex_id, OSAL_WAIT_FOREVER_TIME);
		param->autoscan_stop_flag = 0;
		//param->do_not_wait_t2_signal_locked = ( change_para->usage_type == USAGE_TYPE_AERIALTUNE ? 1:0 );

		CXD2838_PRINTF("[%s]:line=%d,system:%d\r\n",__FUNCTION__,__LINE__,param->system);

		if ( need_to_config_tuner(dev, change_para->freq, change_para->bandwidth) )
		{
		    
			CXD2838_PRINTF("[%s]:line=%d,need_to_config_tuner!\r\n",__FUNCTION__,__LINE__);

			if(param->bandwidth != change_para->bandwidth)
				NeedToInitSystem = TRUE;
			param->Frequency = change_para->freq;
			param->bandwidth = change_para->bandwidth;
			NeedToConfigTuner = TRUE;
		}
		if(need_to_lock_ISDBT_signal(dev, change_para, NeedToConfigTuner ))
		{
			CXD2838_PRINTF("[%s]:line=%d,try_to_lock_ISDBT_signal play_program =%d \n",__FUNCTION__,__LINE__,play_program);
			
			//param->priority = change_para->priority;
			result = try_to_lock_ISDBT_signal(dev, NeedToInitSystem, NeedToConfigTuner, change_para);
			if (result == SUCCESS)
			{
			    break;
			}
			else
			{
				CXD2838_PRINTF("[%s]:line=%d,try_to_lock_ISDBT_signal failed!\n",__FUNCTION__,__LINE__);
			}

		}
		
    }while (0);
	CXD2838_PRINTF("[%s]:line=%d,system:%d\r\n",__FUNCTION__,__LINE__,param->system);
    mutex_unlock(&param->demodMode_mutex_id);

    return result;
}

INT32 nim_cxd2838_get_agc(struct nim_device *dev, UINT8 *agc)
{
	sony_demod_t * priv = (sony_demod_t *)dev->priv;
	sony_result_t result = SONY_RESULT_OK;
	static uint32_t Preagc =0;

    mutex_lock(&priv->demodMode_mutex_id);
	
    //result =   sony_demod_isdbt_monitor_IFAGCOut(priv, agc);
    result = sony_integ_isdbt_monitor_RFLevel(priv, agc);
	if(SONY_RESULT_OK != result)
        *agc = Preagc;
	else 
	 	 Preagc = *agc;
	mutex_unlock(&priv->demodMode_mutex_id);
	
	return SUCCESS;	
}
INT32 nim_cxd2838_get_modulation(struct nim_device *dev, UINT8 *modulation)
{

    sony_result_t result;
    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_UNLOCK;
	UINT8 guard = SONY_ISDBT_GUARD_UNKNOWN;
	
	mutex_lock(&priv->demodMode_mutex_id);
    result = sony_demod_isdbt_monitor_ModeGuard(priv, modulation, &guard);
	mutex_unlock(&priv->demodMode_mutex_id);
    return SUCCESS;
}

INT32 nim_cxd2838_get_guard(struct nim_device *dev,UINT8 *guard)
{
    sony_result_t result;
    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_UNLOCK;
	UINT8 modulation = SONY_ISDBT_MODE_UNKNOWN;
	
	mutex_lock(&priv->demodMode_mutex_id);
	result = sony_demod_isdbt_monitor_ModeGuard(priv, &modulation, guard);
	mutex_unlock(&priv->demodMode_mutex_id);
    return SUCCESS;

}
INT32 nim_cxd2838_get_snr(struct nim_device *dev, UINT8 *snr)
{
	sony_demod_t * priv = (sony_demod_t *)dev->priv;
	sony_result_t result = SONY_RESULT_OK;
	static uint32_t presnr =0;  

    mutex_lock(&priv->demodMode_mutex_id);
    result =  sony_demod_isdbt_monitor_SNR(priv, snr);
	if(SONY_RESULT_OK != result)
		*snr = presnr;
	else
		presnr = *snr;
	mutex_unlock(&priv->demodMode_mutex_id);
	return SUCCESS;
}

INT32 nim_cxd2838_get_ber(struct nim_device *dev, UINT32 *ber)
{
	sony_demod_t * priv = (sony_demod_t *)dev->priv;
	sony_result_t result = SONY_RESULT_OK;
	static UINT32 preber_A;
	static UINT32 preber_B;
	static UINT32 preber_C;
	UINT32 ber_A = 0;
	UINT32 ber_B = 0;
	UINT32 ber_C = 0;
	
	mutex_lock(&priv->demodMode_mutex_id);

	result = sony_demod_isdbt_monitor_PreRSBER(priv,SONY_ISDBT_MONITOR_TARGET_LAYER_A,&ber_A);
	if(SONY_RESULT_OK != result)
		ber_A = preber_A;
	else
		preber_A = ber_A;
	
   	result = sony_demod_isdbt_monitor_PreRSBER(priv,SONY_ISDBT_MONITOR_TARGET_LAYER_B,&ber_B);
	if(SONY_RESULT_OK != result)
		ber_B = preber_B;
	else
		preber_B = ber_B;

	result = sony_demod_isdbt_monitor_PreRSBER(priv,SONY_ISDBT_MONITOR_TARGET_LAYER_C,&ber_C);
	if(SONY_RESULT_OK != result)
		ber_C = preber_C;
	else
		preber_C = ber_C;
	
	mutex_unlock(&priv->demodMode_mutex_id);

    *ber = (ber_A + ber_B + ber_C)/3;
	return SUCCESS;
}

INT32 nim_cxd2838_get_per(struct nim_device *dev, UINT32 *per)
{
	sony_demod_t * priv = (sony_demod_t *)dev->priv;
	sony_result_t result = SONY_RESULT_OK;
	static UINT32 preper_A;
	static UINT32 preper_B;
	static UINT32 preper_C;
	UINT32 per_A = 0;
	UINT32 per_B = 0;
	UINT32 per_C = 0;

	mutex_lock(&priv->demodMode_mutex_id);

	result = sony_demod_isdbt_monitor_PER(priv,SONY_ISDBT_MONITOR_TARGET_LAYER_A,&per_A);
	if(SONY_RESULT_OK != result)
		per_A = preper_A;
	else
		preper_A = per_A;

	result = sony_demod_isdbt_monitor_PER(priv,SONY_ISDBT_MONITOR_TARGET_LAYER_A,&per_B);
	if(SONY_RESULT_OK != result)
		per_B = preper_B;
	else
		preper_B = per_B;

	result = sony_demod_isdbt_monitor_PER(priv,SONY_ISDBT_MONITOR_TARGET_LAYER_A,&per_C);
	if(SONY_RESULT_OK != result)
		per_C = preper_C;
	else
		preper_C = per_C;

	*per = (per_A + per_B + per_C)/3;

	mutex_unlock(&priv->demodMode_mutex_id);

	return SUCCESS;
}

INT32 nim_cxd2838_get_lock(struct nim_device *dev, UINT8 *lock)
{
    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    INT32 i;
    sony_demod_lock_result_t demod_lock = SONY_DEMOD_LOCK_RESULT_UNLOCK;
	*lock = 0;
	
    mutex_lock(&priv->demodMode_mutex_id);
	sony_demod_isdbt_CheckDemodLock(priv,&demod_lock);
    if ( SONY_DEMOD_LOCK_RESULT_LOCK == demod_lock )
    {
        *lock = 1;
    }
    mutex_unlock(&priv->demodMode_mutex_id);

    return SUCCESS;
}
/*****************************************************************************
* INT32 nim_cxd2834_close(struct nim_device *dev)
* Description: cxd2834 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_cxd2838_close(struct nim_device *dev)
{
    //UINT8 data;
    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    INT32 result = SONY_RESULT_OK;

    // tuner power off
    tun_cxd_ascot3_command(priv->tuner_id, NIM_TUNER_POWER_CONTROL, TRUE);

    result = cxd2838_demod_Shutdown(priv);//sony_dvb_demod_Finalize
    if (result != SONY_RESULT_OK)
        result = ERR_FAILUE;

    //nim_cxd2838_switch_lock_led(dev, FALSE);
    return result;
}

INT32 nim_cxd2838_open(struct nim_device *dev)
{

    sony_demod_t * priv = (sony_demod_t *)dev->priv;
    struct COFDM_TUNER_CONFIG_API * config_info = &(priv->tuner_control);
    //sony_dvbt2_tune_param_t  TuneParam;
    sony_result_t result = SONY_RESULT_OK;
	
    if((NULL == dev) || (NULL == config_info))
    {
		CXD2838_PRINTF("nim_device is null\n");
		return SONY_RESULT_ERROR_ARG;
	}

    /* Initialize Demod */
    result = sony_demod_Initialize(priv);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }
	// Initialize the tuner.
    result = tun_cxd_ascot3_command(priv->tuner_id, NIM_TUNER_POWER_CONTROL, FALSE); //tuner power on
	if (result != SUCCESS) {
		   SONY_TRACE_RETURN(result);
	 }
       	//Config TS output mode: SSI/SPI. Default is SPI.	
	//result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_PARALLEL_SEL, ((NIM_COFDM_TS_SSI == priv->tc.ts_mode)?0:1) );
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_PARALLEL_SEL, 0 );
	if (result != SONY_RESULT_OK)
	{
		CXD2838_PRINTF("Error: Unable to configure DEMOD_CONFIG_PARALLEL_SEL. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
    	//Confif Data output pin: 0:TS0(pin13), 1:TS7(pin20), default:1.
	//result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_SER_DATA_ON_MSB, ((NIM_COFDM_TS_SSI == priv->tc.ts_mode)?0:1) );
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_SER_DATA_ON_MSB, 0 );
	if (result != SONY_RESULT_OK)
	{
		CXD2838_PRINTF("Error: Unable to configure DEMOD_CONFIG_PARALLEL_SEL. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}

    	//TS Error output.
	result = sony_demod_SetConfig(priv,SONY_DEMOD_CONFIG_TSERR_ACTIVE_HI, 1);//DEMOD_CONFIG_TSERR_ENABLE
	if (result != SONY_RESULT_OK)
	{
		CXD2838_PRINTF("Error: Unable to configure DEMOD_CONFIG_TSERR_ENABLE. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}

	/* IFAGC setup. Modify to suit connected tuner. */
	/* IFAGC: 0 for positive and 1 for negtive*/
#ifdef TUNER_IFAGCPOS
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_IFAGCNEG, 0);
	if (result != SONY_RESULT_OK)
	{
		CXD2838_PRINTF("Error: Unable to configure IFAGCNEG. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
#else
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_IFAGCNEG, 1);
	if (result != SONY_RESULT_OK)
	{
		CXD2838_PRINTF("Error: Unable to configure IFAGCNEG. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
#endif

	/*IFAGC ADC range[0-2]     0 : 1.4Vpp, 1 : 1.0Vpp, 2 : 0.7Vpp*/   
	result = sony_demod_SetConfig(priv,SONY_DEMOD_CONFIG_IFAGC_ADC_FS, 0);//DEMOD_CONFIG_TSERR_ENABLE
	if (result != SONY_RESULT_OK)
	{
		CXD2838_PRINTF("Error: Unable to configure SONY_DEMOD_CONFIG_IFAGC_ADC_FS. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}

	//Ben Debug 140221#1
	//add by AEC for TS error enable 2013-09-09
	 // TSERR output enable from GPIO2 pin
	result = sony_demod_GPIOSetConfig(priv, 2, 1, SONY_DEMOD_GPIO_MODE_TS_ERROR);
	if(result != SONY_RESULT_OK)
	{
		CXD2838_PRINTF("Error in sony_demod_GPIOSetConfig for TS error.\n");
		return result;
	}
	//end for TS error enable 2013-09-09  
	
	/* Spectrum Inversion setup. Modify to suit connected tuner. */
	/* Spectrum inverted, value = 1. */
#ifdef TUNER_SPECTRUM_INV
	result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_SPECTRUM_INV, 1);
	if (result != SONY_RESULT_OK)
	{
		CXD2838_PRINTF("Error: Unable to configure SPECTRUM_INV. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
#endif

    /* RFAIN ADC and monitor enable/disable. */
    /* Default is disabled. 1: Enable, 0: Disable. */
#ifdef RFAIN_ADC_ENABLE
        result = sony_demod_SetConfig(priv, SONY_DEMOD_CONFIG_RFAIN_ENABLE, 0);
        if (result == SONY_RESULT_OK) {
            CXD2838_PRINTF("Demodulator configured to enable RF level monitoring.\n");
        }
        else {
            CXD2838_PRINTF("Error: Unable to configure RFLVMON_ENABLE. (result = %d)\n", result);
            return -1;
        }
#endif

	/* RF level monitoring (RFAIN/RFAGC) enable/disable. */
	/* Default is enabled. 1: Enable, 0: Disable. */
#ifdef TUNER_RFLVLMON_DISABLE
	result = sony_demod_SetConfig(priv, DEMOD_CONFIG_RFLVMON_ENABLE, 0);
	if (result != SONY_RESULT_OK)
	{
		CXD2838_PRINTF("Error: Unable to configure RFLVMON_ENABLE. (status=%d, %s)\r\n", result, FormatResult(result));
		return result;
	}
#endif
    return SUCCESS;
}

