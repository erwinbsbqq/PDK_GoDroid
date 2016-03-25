/*******************************************************************************
 *
 * FILE NAME          : MxL241SF_PhyCtrlApi.cpp
 * 
 * AUTHOR             : Brenndon Lee
 * DATE CREATED       : 6/22/2009
 *
 * DESCRIPTION        : This file contains MxL241SF driver APIs
 * 				     Coeus modify to Ali linux platform 2012/10/28
 *                             
 *******************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ******************************************************************************/

#include "MxL241SF_PhyCtrlApi.h"
#include "MxL241SF_PhyCfg.h"

/* MxLWare Driver version for MxL241SF */
const UINT8 MxLWareDrvVersion[] = {2, 1, 20, 2};
const UINT8 MxLWareCandidateIndex = 0;

/* For statistics */
ACC_STAT_COUNTER_T AccStatCounter[MAX_241SF_DEVICES];

static MXL_STATUS MxL_GetDemodLockStatus(MXL_CMD_TYPE_E CmdType, PMXL_DEMOD_LOCK_STATUS_T DemodLockStatusPtr);
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDevReset - MXL_DEV_SOFT_RESET_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : By writing any value into address 0xFFFF(AIC), all control 
--|                 registers are initialized to the default value.
--|                 AIC - Address Initiated Command
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDevReset(PMXL_RESET_CFG_T ResetParamPtr)
{
	UINT8 status = MXL_TRUE;
	UINT8 i;

	/* Stop Tune */
	status |= Ctrl_WriteRegister(ResetParamPtr->I2cSlaveAddr, RF_SEQUENCER_REG, STOP_TUNE);

	/* Power down Tuner */
	status |= Ctrl_WriteRegister(ResetParamPtr->I2cSlaveAddr, TOP_MASTER_CONTROL_REG, DISABLE_TOP_MASTER);

	status |= Ctrl_WriteRegister(ResetParamPtr->I2cSlaveAddr, AIC_RESET_REG, 0x0000);

	/* Statistic variables are also initialized */
	for (i = 0; i < MAX_241SF_DEVICES; i++)
	{
		if (AccStatCounter[i].I2cSlaveAddr == ResetParamPtr->I2cSlaveAddr) break;
	}

	if (i == MAX_241SF_DEVICES)
	{
		/* This Device was never registered */
		/* Find empty slot */
		for (i = 0; i < MAX_241SF_DEVICES; i++)
		{
			if (AccStatCounter[i].I2cSlaveAddr == 0) break;
		}

		/* No empty slot */
		if (i == MAX_241SF_DEVICES)
		{
			return MXL_FALSE;
		}
	}

	/* Register this I2C address */
	AccStatCounter[i].I2cSlaveAddr = ResetParamPtr->I2cSlaveAddr;

	AccStatCounter[i].AccCwCorrCount = 0;
	AccStatCounter[i].AccCwErrCount = 0;
	AccStatCounter[i].AccCwUnerrCount = 0;
	AccStatCounter[i].AccCwReceived = 0;
	AccStatCounter[i].AccCorrBits = 0;
	AccStatCounter[i].AccErrMpeg = 0;
	AccStatCounter[i].AccReceivedMpeg = 0;

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDeviceInfo - MXL_DEV_ID_VERSION_REQ
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : This function returns MxL241SF Chip Id and version information.
--|                 Device Id of MxL241SF is 0x1, Version is unknown yet 
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDeviceInfo(PMXL_DEV_INFO_T DevInfoPtr)
{
	MXL_STATUS status;
	UINT16 readBack;
	UINT8 i = 0;
	status = Ctrl_ReadRegister(DevInfoPtr->I2cSlaveAddr, VERSION_REG, &readBack);
	DevInfoPtr->DevId = (UINT8)(readBack & 0xF);
	DevInfoPtr->DevVer = (UINT8)((readBack >> 4) & 0xF);
	/* MxLWare Driver Version */
	for (i = 0 ; i < sizeof(MxLWareDrvVersion) ; i++)
	{
		DevInfoPtr->MxLWareVer[i] = MxLWareDrvVersion[i];
	}
	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDevXtalSettings - MXL_DEV_XTAL_SETTINGS_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : This function configures XTAL frequency, Clok-out settings, and
--|                 Loop through control for MxL241SF.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDevXtalSettings(PMXL_XTAL_CFG_T XtalParamPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 control;
	/* XTAL Enable or Disable */
	if (XtalParamPtr->XtalEnable == MXL_ENABLE) control = ENABLE_XTAL;
	else control = 0;

	/* Set XTAL Frequency */
	control |= XtalParamPtr->DigXtalFreq;

	status |= Ctrl_WriteRegister(XtalParamPtr->I2cSlaveAddr, DIG_XTAL_FREQ_CTRL_REG, control);

	/* Xtal Capacitor */
	control = (UINT16)XtalParamPtr->XtalCap;
	status |= Ctrl_WriteRegister(XtalParamPtr->I2cSlaveAddr, DIG_XTAL_CAP_CTRL_REG, control);

	/* Xtal Bias */
	control = (UINT16)XtalParamPtr->XtalBiasCurrent << 4;
	status |= Ctrl_WriteRegister(XtalParamPtr->I2cSlaveAddr, DIG_XTAL_BIAS_CTRL_REG, control);

	/* Xtal CLK Out */
	if (XtalParamPtr->XtalClkOutEnable == MXL_ENABLE) control = ENABLE_CLK_OUT;
	else control = 0;

	control |= (UINT16)XtalParamPtr->XtalClkOutGain;
	status |= Ctrl_WriteRegister(XtalParamPtr->I2cSlaveAddr, XTAL_CLK_OUT_CTRL_REG, control);

	/* Loop Through */
	control = 0x14;

	if (XtalParamPtr->LoopThruEnable == MXL_ENABLE) control |= ENABLE_LOOP_THRU;

	status |= Ctrl_WriteRegister(XtalParamPtr->I2cSlaveAddr, LOOP_THROUGH_CTRL_REG, control);

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodQamBurstFreeze - MXL_DEMOD_QAM_BURST_FREEZE_CFG
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 5/25/2010
--|
--| DESCRIPTION   : It sets demod reaction speed against QAM BURST noise.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDemodQamBurstFreeze(PMXL_QAM_BURST_FREEZE_T ParamPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 readData;

	status |= Ctrl_ReadRegister(ParamPtr->I2cSlaveAddr, QAM_BURST_FREEZE, &readData);

	/* Reset */
	readData &= 0x9FFF;

	/* QAM Burst Freeze enable <13> */
	if (ParamPtr->QamFreezeEnable == MXL_ENABLE)
	{
		readData |= 0x2000;
	}

	/* QAM Burst Freeze mode <14> */
	if (ParamPtr->QamFreezeMode == MXL_ENABLE)
	{
		readData |= 0x4000;
	}

	status |= Ctrl_WriteRegister(ParamPtr->I2cSlaveAddr, QAM_BURST_FREEZE, readData);

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodInvertCarrierOffset - MXL_DEMOD_INVERT_CARRIER_OFFSET_CFG
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 4/13/2011
--|
--| DESCRIPTION   : It inverts demod carrier offset.
--|                 When auto spectrum algorithm used, calling this API will able to 
--|                 reduece locking time
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDemodInvertCarrierOffset(PMXL_INVERT_CARR_OFFSET_T ParamPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 regData;

	/* Before readback, register 0x803E needs to be written by data 0x0087  */
	status |= Ctrl_WriteRegister(ParamPtr->I2cSlaveAddr, CARRIER_OFFSET_REG, 0x0087);

	/* Read the current settings. */
	status |= Ctrl_ReadRegister(ParamPtr->I2cSlaveAddr, CARRIER_OFFSET_RB_REG, &regData);

	status |= Ctrl_WriteRegister(ParamPtr->I2cSlaveAddr, CARRIER_OFFSET_RB_REG, (SINT16)regData * (-1));

	return (MXL_STATUS)status;
}
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodSymbolRate - MXL_DEMOD_SYMBOL_RATE_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/19/2009
--|                 6/02/2010
--|
--| DESCRIPTION   : This function configures Symbol Rate for Annex A/B or OOB, and
--|                 supports either float or integer calculation depending on
--|                 conditional compile (__MXL_INTEGER_CALC_STATISTICS__)
--|
--|                 Resample register bank - 0 : Annex-A bank
--|                                          1 : Annex B 64 QAM
--|                                          2 : Annex B 256 QAM
--|                 Interpolator bypass - 0 : bypass off
--|                                       1 : bypass on
--|                 Symbol rate - 0 : 6.89 MHz (A)
--|                               1 : 5.05 MHz (B64)
--|                               2 : 5.36 Mhz (B256)
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

#ifdef __MXL_INTEGER_CALC_STATISTICS__
static MXL_STATUS MxL_ConfigDemodSymbolRate(PMXL_SYMBOL_RATE_T SymbolRatePtr) 
{
	UINT8 status = MXL_TRUE;
	PREG_CTRL_INFO_T OobFilterCoefDataPtr = NULL;
	UINT32 SymbolRate[SYM_NUM_B_BANK];
	UINT32 ResampleRateRatio;
	UINT16 ResampleRegCtrl, control, sweepStep;
	UINT8 i, NumBank, Bank[SYM_NUM_B_BANK];
	MXL_QAM_BURST_FREEZE_T qamBurstFreeze;
	switch (SymbolRatePtr->SymbolType)
	{
		case SYM_TYPE_J83A:
			SymbolRate[0] = 0x0108BB43;
			Bank[0] = 0; /* Annex-A bank */
			NumBank = 1;
			break;

		case SYM_TYPE_J83B:
			NumBank = 2;
			SymbolRate[0] = 0x0168B13F;
			Bank[0] = 1; /* Annex-B 64QAM bank */
			SymbolRate[1] = 0x015443B1;
			Bank[1] = 2; /* Annex-B 256QAM bank */
			break;

		case SYM_TYPE_USER_DEFINED_J83A:
			NumBank = 1;
			Bank[0] = 0; /* Annex-A bank */
		break;

		case SYM_TYPE_USER_DEFINED_J83B:
			NumBank = 2;
			Bank[0] = 1; /* Annex-B 64QAM bank */
			Bank[1] = 2; /* Annex-B 256QAM bank */
			break;

		case SYM_TYPE_OOB:
			NumBank = 1;
			Bank[0] = 0; /* Annex-A bank */
			switch (SymbolRatePtr->OobSymbolRate)
			{
				case SYM_RATE_0_772MHz:
					SymbolRate[0] = 0x03139094;
					OobFilterCoefDataPtr = MxL_OobAciMfCoef_0_772MHz;
					sweepStep = 0x0009;
					break;

				case SYM_RATE_1_024MHz:
					SymbolRate[0] = 0x0251C000;
					OobFilterCoefDataPtr = MxL_OobAciMfCoef_1_024MHz;
					sweepStep = 0x000A;
					break;

				case SYM_RATE_1_544MHz:
					SymbolRate[0] = 0x0189C84A;
					OobFilterCoefDataPtr = MxL_OobAciMfCoef_1_544MHz;
					sweepStep = 0x0009;
					break;

				default:
					return MXL_FALSE;
			}
			break;

		default:
			return MXL_FALSE;
			break;
	}

	/* Configure Resample rate ratio  */
	for (i = 0; i < NumBank; i++)
	{
		/* Read the current settings. */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, SYMBOL_RATE_RESAMP_BANK_REG, &ResampleRegCtrl);
		ResampleRegCtrl &= 0xFFF8;

		/* Select Bank */
		ResampleRegCtrl |= Bank[i]; 

		/* INTERP_BYPASS <4> */
		if (SymbolRatePtr->SymbolType == SYM_TYPE_OOB)
		{
			ResampleRegCtrl |= 0x0010;
		}
		else
		{
			if ((SymbolRatePtr->SymbolType == SYM_TYPE_USER_DEFINED_J83A) ||
			(SymbolRatePtr->SymbolType == SYM_TYPE_USER_DEFINED_J83B))
			{
				UINT32 temp;

				temp = 0x5f000000;
				temp = (temp + (SymbolRatePtr->SymbolRate / 2)) / SymbolRatePtr->SymbolRate;

				if (SymbolRatePtr->SymbolRate < 2375)
				{
					temp *= 100;
					ResampleRegCtrl |= BYPASS_INTERPOLATOR;
				}
				else
				{
					temp *= 300;
					ResampleRegCtrl &= ~BYPASS_INTERPOLATOR;
				}
				temp /= 4;
				SymbolRate[i] = temp;
				/*calculate ResampleRateRatio*/
			}
			ResampleRegCtrl &= 0xFFEF;
		}

		/* Configure Resample_register bank and Interpolator bypass control  */
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, SYMBOL_RATE_RESAMP_BANK_REG, ResampleRegCtrl);

		ResampleRateRatio = SymbolRate[i];
		//MxL_DLL_DEBUG0("ResampleRateRatio = 0x%x\n", ResampleRateRatio);

		/* Configure Resample Rate Ratio register */
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, 
			SYMBOL_RATE_RESAMP_RATE_LO_REG, 
		ResampleRateRatio & 0xFF);
		ResampleRateRatio >>= 8;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, 
			SYMBOL_RATE_RESAMP_RATE_MID_LO_REG, 
		ResampleRateRatio & 0xFF);
		ResampleRateRatio >>= 8;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, 
			SYMBOL_RATE_RESAMP_RATE_MID_HI_REG, 
		ResampleRateRatio & 0xFF);
		ResampleRateRatio >>= 8;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, 
			SYMBOL_RATE_RESAMP_RATE_HI_REG, 
		ResampleRateRatio & 0xFF);
	}

	if (SYM_TYPE_OOB == SymbolRatePtr->SymbolType)
	{
		UINT16 maskTunerDone;
		/* Set Annex-A and QPSK  */
		control = MXL_QPSK;
		control |= ANNEX_A_TYPE;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, control);

		/* Watch dog disable <12> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, QAM_TYPE_AUTO_DETECT_REG, &control);
		control &= 0xEFFF;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, QAM_TYPE_AUTO_DETECT_REG, control);

		/* Enable Custom ACI COEF <5> */
		/* Enable Matched Filter custom coefficients <6> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, CUSTOM_COEF_EN_REG, &control);
		control |= 0x0060;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, CUSTOM_COEF_EN_REG, control);

		/* Set max = 15 Equalizer frequency sweep limit <7:4> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, EQU_FREQ_SWEEP_LIMIT_REG, &control);
		control |= 0x00F0;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, EQU_FREQ_SWEEP_LIMIT_REG, control);

		/* Sweep step <3:0> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, EQU_FREQ_SWEEP_LIMIT_REG, &control);
		control &= 0xFFF0;
		control |= sweepStep;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, EQU_FREQ_SWEEP_LIMIT_REG, control);

		/* Set serial output interface <1> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, MPEGOUT_PAR_CLK_CTRL_REG, &control);
		control &= 0xFFFD;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, MPEGOUT_PAR_CLK_CTRL_REG, control);

		/* Enable 2X drive for MCLK and MDAT <7:0> */
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_PIN_2XDRV_REG, 0x0002);

		/* Configure control before writing filter data */
		status |= Ctrl_ProgramRegisters(SymbolRatePtr->I2cSlaveAddr, MxL_OobAciMfCoef);

		/* Program multiple registers for the specified function */
		status |= Ctrl_ProgramRegisters(SymbolRatePtr->I2cSlaveAddr, OobFilterCoefDataPtr);
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_SAGC_NMAXAVG, 0x0002);
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_SAGC_NMAXHOLDCLK, 0x0045);

		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &maskTunerDone);
		maskTunerDone |= 0x08;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, maskTunerDone);
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, AGC_CTRL_SPEED_REG, FAST_AGC_CTRL_LOCK);
	}
	else
	{
		UINT16 maskTunerDone;
		// Wider blind state frequency sweep range for Annex-B mode
		if (SYM_TYPE_J83B == SymbolRatePtr->SymbolType)
		{
			MxL_EqualizerSpeedUp[0].data = WIDER_FREQ_SWEEP_RANGE;
		}
		else
		{
			MxL_EqualizerSpeedUp[0].data = DEFAULT_FREQ_SWEEP_RANGE;
		}

		status |= Ctrl_ProgramRegisters(SymbolRatePtr->I2cSlaveAddr, MxL_EqualizerSpeedUp);

		/* QAM Burst Freeze cfg only for J.83 A/B */
		qamBurstFreeze.I2cSlaveAddr = SymbolRatePtr->I2cSlaveAddr;
		qamBurstFreeze.QamFreezeEnable = MXL_ENABLE;
		qamBurstFreeze.QamFreezeMode = MXL_ENABLE;
		status |= MxL_ConfigDemodQamBurstFreeze(&qamBurstFreeze);
		/* PG 2.1.18*/
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_SAGC_NMAXAVG, 0x000E);
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_SAGC_NMAXHOLDCLK, 0x0042);

		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &maskTunerDone);
		maskTunerDone &= ~0x08;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, maskTunerDone);
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, AGC_CTRL_SPEED_REG, NORMAL_AGC_CTRL_LOCK);
	}

	return (MXL_STATUS)status;
}
#else
static MXL_STATUS MxL_ConfigDemodSymbolRate(PMXL_SYMBOL_RATE_T SymbolRatePtr) 
{
	UINT8 status = MXL_TRUE;
	PREG_CTRL_INFO_T OobFilterCoefDataPtr = NULL;
	REAL32 SPSin, SymbolRate[SYM_NUM_B_BANK];
	UINT32 ResampleRateRatio;
	UINT16 ResampleRegCtrl, control, sweepStep;
	UINT8 i, NumBank, Bank[SYM_NUM_B_BANK];
	UINT32 tmp_SPSin=0;
	MXL_QAM_BURST_FREEZE_T qamBurstFreeze;
	switch (SymbolRatePtr->SymbolType)
	{
		case SYM_TYPE_J83A:
			SymbolRate[0] = (REAL32)6.89;
			Bank[0] = 0; /* Annex-A bank */
			NumBank = 1;
			break;

		case SYM_TYPE_J83B:
			NumBank = 2;
			SymbolRate[0] = (REAL32)5.056941;
			Bank[0] = 1; /* Annex-B 64QAM bank */
			SymbolRate[1] = (REAL32)5.360537;
			Bank[1] = 2; /* Annex-B 256QAM bank */
			break;

		case SYM_TYPE_USER_DEFINED_J83A:
			NumBank = 1;
			SymbolRate[0] = SymbolRatePtr->SymbolRate;
			Bank[0] = 0; /* Annex-A bank */
			break;

		case SYM_TYPE_USER_DEFINED_J83B:
			NumBank = 2;
			SymbolRate[0] = SymbolRatePtr->SymbolRate;
			Bank[0] = 1; /* Annex-B 64QAM bank */
			SymbolRate[1] = SymbolRatePtr->SymbolRate256;
			Bank[1] = 2; /* Annex-B 256QAM bank */
			break;

		case SYM_TYPE_OOB:
			NumBank = 1;
			Bank[0] = 0; /* Annex-A bank */
			switch (SymbolRatePtr->OobSymbolRate)
			{
				case SYM_RATE_0_772MHz:
					SymbolRate[0] = (REAL32)0.772;
					OobFilterCoefDataPtr = MxL_OobAciMfCoef_0_772MHz;
					sweepStep = 0x0009;
					break;

				case SYM_RATE_1_024MHz:
					SymbolRate[0] = (REAL32)1.024;
					OobFilterCoefDataPtr = MxL_OobAciMfCoef_1_024MHz;
					sweepStep = 0x000A;
					break;

				case SYM_RATE_1_544MHz:
					SymbolRate[0] = (REAL32)1.544;
					OobFilterCoefDataPtr = MxL_OobAciMfCoef_1_544MHz;
					sweepStep = 0x0009;
					break;

				default:
					return MXL_FALSE;
			}
			break;

		default:
			return MXL_FALSE;
			break;
	}
	/* Configure Resample rate ratio  */
	for (i = 0; i < NumBank; i++)
	{
		/* Read the current settings. */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, SYMBOL_RATE_RESAMP_BANK_REG, &ResampleRegCtrl);
		ResampleRegCtrl &= 0xFFF8;

		/* Select Bank */
		ResampleRegCtrl |= Bank[i]; 

		if (0 != SymbolRate[i]) SPSin = (REAL32)9.5 / SymbolRate[i];
		else status |= MXL_FALSE;

		ResampleRateRatio = 1 << 24;	
		SymbolRate[i] = (REAL32)ResampleRateRatio * SPSin;
		//tmp_SPSin=SPSin*100;
		//SymbolRate[i] =(REAL32)((ResampleRateRatio*tmp_SPSin)/100);

		if (SPSin <= 4)
		{
			/* Interpolator X3 should not be bypassed  */
			ResampleRegCtrl &= ~BYPASS_INTERPOLATOR;
			SymbolRate[i] *= 0.75;
		}
		else
		{
			/* Interpolator X3 should be bypassed  */
			ResampleRegCtrl |= BYPASS_INTERPOLATOR;
			SymbolRate[i] /= 4;
		}

		/* Configure Resample_register bank and Interpolator bypass control  */
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, SYMBOL_RATE_RESAMP_BANK_REG, ResampleRegCtrl);

		ResampleRateRatio = (UINT32)SymbolRate[i];
		//MxL_DLL_DEBUG0("SPSin = %f, ResampleRateRatio = 0x%x\n", SPSin, ResampleRateRatio);

		/* Configure Resample Rate Ratio register */
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, 
			SYMBOL_RATE_RESAMP_RATE_LO_REG, 
		ResampleRateRatio & 0xFF);
		//MxL_DLL_DEBUG0("SYMBOL_RATE_RESAMP_RATE_LO_REG: 0x%x \n",ResampleRateRatio);
		ResampleRateRatio >>= 8;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, 
			SYMBOL_RATE_RESAMP_RATE_MID_LO_REG, 
		ResampleRateRatio & 0xFF);
		//MxL_DLL_DEBUG0("SYMBOL_RATE_RESAMP_RATE_MID_LO_REG: 0x%x \n",ResampleRateRatio);
		ResampleRateRatio >>= 8;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, 
			SYMBOL_RATE_RESAMP_RATE_MID_HI_REG, 
		ResampleRateRatio & 0xFF);
		//MxL_DLL_DEBUG0("SYMBOL_RATE_RESAMP_RATE_MID_HI_REG: 0x%x \n",ResampleRateRatio);
		ResampleRateRatio >>= 8;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, 
			SYMBOL_RATE_RESAMP_RATE_HI_REG, 
		ResampleRateRatio & 0xFF);
		//MxL_DLL_DEBUG0("SYMBOL_RATE_RESAMP_RATE_HI_REG: 0x%x \n",ResampleRateRatio);
	}

	if (SYM_TYPE_OOB == SymbolRatePtr->SymbolType)
	{
		UINT16 maskTunerDone;
		/* Set Annex-A and QPSK  */
		control = MXL_QPSK;
		control |= ANNEX_A_TYPE;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, control);

		/* Watch dog disable <12> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, QAM_TYPE_AUTO_DETECT_REG, &control);
		control &= 0xEFFF;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, QAM_TYPE_AUTO_DETECT_REG, control);

		/* Enable Custom ACI COEF <5> */
		/* Enable Matched Filter custom coefficients <6> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, CUSTOM_COEF_EN_REG, &control);
		control |= 0x0060;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, CUSTOM_COEF_EN_REG, control);

		/* Set max = 15 Equalizer frequency sweep limit <7:4> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, EQU_FREQ_SWEEP_LIMIT_REG, &control);
		control |= 0x00F0;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, EQU_FREQ_SWEEP_LIMIT_REG, control);

		/* Sweep step <3:0> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, EQU_FREQ_SWEEP_LIMIT_REG, &control);
		control &= 0xFFF0;
		control |= sweepStep;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, EQU_FREQ_SWEEP_LIMIT_REG, control);

		/* Set serial output interface <1> */
		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, MPEGOUT_PAR_CLK_CTRL_REG, &control);
		control &= 0xFFFD;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, MPEGOUT_PAR_CLK_CTRL_REG, control);

		/* Enable 2X drive for MCLK and MDAT <7:0> */
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_PIN_2XDRV_REG, 0x0002);

		/* Configure control before writing filter data */
		status |= Ctrl_ProgramRegisters(SymbolRatePtr->I2cSlaveAddr, MxL_OobAciMfCoef);

		/* Program multiple registers for the specified function */
		status |= Ctrl_ProgramRegisters(SymbolRatePtr->I2cSlaveAddr, OobFilterCoefDataPtr);
		/* PG 2.1.18*/
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_SAGC_NMAXAVG, 0x0002);
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_SAGC_NMAXHOLDCLK, 0x0045);

		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &maskTunerDone);
		maskTunerDone |= 0x08;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, maskTunerDone);
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, AGC_CTRL_SPEED_REG, FAST_AGC_CTRL_LOCK);
	}
	else
	{
		UINT16 maskTunerDone;
		// Wider blind state frequency sweep range for Annex-B mode
		if (SYM_TYPE_J83B == SymbolRatePtr->SymbolType)
		{
			MxL_EqualizerSpeedUp[0].data = WIDER_FREQ_SWEEP_RANGE;
		}
		else
		{
			MxL_EqualizerSpeedUp[0].data = DEFAULT_FREQ_SWEEP_RANGE;
		}

		status |= Ctrl_ProgramRegisters(SymbolRatePtr->I2cSlaveAddr, MxL_EqualizerSpeedUp);

		/* QAM Burst Freeze cfg only for J.83 A/B */
		qamBurstFreeze.I2cSlaveAddr = SymbolRatePtr->I2cSlaveAddr;
		qamBurstFreeze.QamFreezeEnable = MXL_ENABLE;
		qamBurstFreeze.QamFreezeMode = MXL_ENABLE;
		status |= MxL_ConfigDemodQamBurstFreeze(&qamBurstFreeze);
		/* PG 2.1.18*/
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_SAGC_NMAXAVG, 0x000E);
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, DIG_SAGC_NMAXHOLDCLK, 0x0042);

		status |= Ctrl_ReadRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &maskTunerDone);
		maskTunerDone &= ~0x08;
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, maskTunerDone);
		status |= Ctrl_WriteRegister(SymbolRatePtr->I2cSlaveAddr, AGC_CTRL_SPEED_REG, NORMAL_AGC_CTRL_LOCK);
	}

	return (MXL_STATUS)status;
}
#endif

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerAgc - MXL_TUNER_AGC_SETTINGS_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : This function configures AGC set-point and AGC mode
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigTunerFreezeAgc(PMXL_AGC_T AgcParamPtr) 
{
	MXL_STATUS status;
	UINT16 control;
	if (AgcParamPtr->FreezeAgcGainWord == MXL_FREEZE) control = FREEZE_AGC_GAIN_WORD;
	else control = 0;                        
	status = Ctrl_WriteRegister(AgcParamPtr->I2cSlaveAddr, DIG_HALT_GAIN_CTRL_REG, control);
	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerTopMaster - MXL_TUNER_TOP_MASTER_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/22/2009
--|
--| DESCRIPTION   : Enabling or Disabling Tuner Block
--|                 
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigTunerTopMaster(PMXL_TOP_MASTER_CFG_T TopMasterCfgPtr) 
{
	MXL_STATUS status;
	UINT16 control;

	if (TopMasterCfgPtr->TopMasterEnable == MXL_ENABLE) 
		control = ENABLE_TOP_MASTER;
	else 
		control = DISABLE_TOP_MASTER;

	status = Ctrl_WriteRegister(TopMasterCfgPtr->I2cSlaveAddr, TOP_MASTER_CONTROL_REG, control);

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodAnnexQamType - MXL_DEMOD_ANNEX_QAM_TYPE_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : This function configure QAM_ANNEX_TYPE and QAM type
--|                 Annex-A type coressponds to J.83 A/C QAM, and OOB signal
--|                 Annex-B for J.83 B QAM signal
--|                 QAM Type : 0 - 16 QAM, 1 - 64 QAM, 2 - 256 QAM,
--|                            3 - 1024 QAM, 4 - 32 QAM, 5 - 128 QAM,
--|                            6 - QPSK
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDemodAnnexQamType(PMXL_ANNEX_CFG_T AnnexQamTypePtr) 
{
	UINT8 status = MXL_TRUE;
	UINT16 AnnnexType, AutoDetectCtrl;
	UINT16 godard_kp_step3 = 0x00, mpeg_th = 0x00;

	/* Read the current settings. */
	status |= Ctrl_ReadRegister(AnnexQamTypePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &AnnnexType);

	if (AnnexQamTypePtr->AnnexType == ANNEX_A)
		AnnnexType |= ANNEX_A_TYPE;
	else 
		AnnnexType &= ~ANNEX_A_TYPE; /* Annex-B  */

	status |= Ctrl_ReadRegister(AnnexQamTypePtr->I2cSlaveAddr, QAM_TYPE_AUTO_DETECT_REG, &AutoDetectCtrl);
	/* AutoDetectCtrl = 0x0014; */

	if (AnnexQamTypePtr->AutoDetectQamType == MXL_ENABLE)
	{
		AutoDetectCtrl |= ENABLE_AUTO_DETECT_QAM_TYPE; /* Enable Auto detect QAM Type */

		if (AnnexQamTypePtr->AutoDetectMode == MXL_ENABLE) 
			AutoDetectCtrl |= ENABLE_AUTO_DETECT_MODE; /* Enable Auto-detect mode  */
		else 
			AutoDetectCtrl &= ~ENABLE_AUTO_DETECT_MODE; /* Disbale Auto-detect mode  */
	}
	else
	{ 
		AutoDetectCtrl &= ~ENABLE_AUTO_DETECT_QAM_TYPE; /* Disable Auto detect QAM Type */

		/* Set QAM type manually  */
		/* Reset Qam type <2:0>   */
		AnnnexType &= 0xFFF8;
		AnnnexType |= AnnexQamTypePtr->QamType;
	}

	/* Configure Annex Type  */
	status |= Ctrl_WriteRegister(AnnexQamTypePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, AnnnexType);

	/* Configure Auto detect mode  */
	status |= Ctrl_WriteRegister(AnnexQamTypePtr->I2cSlaveAddr, QAM_TYPE_AUTO_DETECT_REG, AutoDetectCtrl);
	/* PG 2.1.18*/
	status |= Ctrl_ReadRegister(AnnexQamTypePtr->I2cSlaveAddr, GODARD_KP_STEP3, &godard_kp_step3);
	status |= Ctrl_ReadRegister(AnnexQamTypePtr->I2cSlaveAddr, MPEG_LOCK_TH, &mpeg_th);
	godard_kp_step3 &= ~(0x0f00);
	mpeg_th &= ~(0xff00);
	if (AnnexQamTypePtr->AnnexType == ANNEX_A)
	{ 
		godard_kp_step3 |= (0x0b00);
		mpeg_th |= (0x9500);
	}
	if (AnnexQamTypePtr->AnnexType == ANNEX_B)
	{
		godard_kp_step3 |= (0x0700);
		mpeg_th |= (0x5f00);
	}
	status |= Ctrl_WriteRegister(AnnexQamTypePtr->I2cSlaveAddr, GODARD_KP_STEP3, godard_kp_step3);
	status |= Ctrl_WriteRegister(AnnexQamTypePtr->I2cSlaveAddr, MPEG_LOCK_TH, mpeg_th);
	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodMpegOutIface - MXL_DEMOD_MPEG_OUT_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : For Transport stream output through MPEG TS interface
--|                 the following parameters are needed to configure after or before
--|                 demod lock.
--|                  TS output mode : Seral or Parallel
--|                  MSB or LSB first      : In serial output mode
--|                  MPEG Valid Polarity   : Active Low or High
--|                  MPEG Clock Polarity   : Rising or Falling Edge
--|                  MPEG Sync Polarity    : Active Low or High
--|                  MPEG Sync Pulse width : 1 bit or 8 bit, only for serial mode
--|                  MPEG CLK Frequency  : 0 - 57MHz (6x9.5)(default)
--|               (Internally generated) : 1 - 38MHz (4x9.5)
--|                                      : 2 - 28.5MHz (3x9.5)
--|                                      : 3 - 19MHz (2x9.5)
--|                                      : 4 - 9.5MHz
--|                                      : 5 - 7.125MHz (3/4x9.5)
--|                                      : 6 - 4.75MHz  (1/2x9.5)
--|                                      : 7 - 3.5625MHz (3/8x9.5)
--|                  MPEG Clock Source   : External or Internal MPEG Clock
--|                  External MPEG Clock Phase   : In phase or Inverted phase of the externally provided clock
--|
--| RETURN VALUE  : true or false
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDemodMpegOutIface(PMXL_MPEG_OUT_CFG_T MpegOutParamPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 control;
	UINT8 i2cAddr = MpegOutParamPtr->I2cSlaveAddr;

	//  printk("MXL_DEMOD_MPEG_OUT_CFG - SerialOrPar=%d, LsbOrMsbFirst=%d, MpegValidPol=%d, MpegClkPol=%d, MpegSyncPol=%d\n MpegSyncPulseWidth=%d, MpegClkSource = %d, MpegClkFreq=%d, MpegErrorIndication=%d \n",
	//                 MpegOutParamPtr->SerialOrPar,
	//                 MpegOutParamPtr->LsbOrMsbFirst,
	//                 MpegOutParamPtr->MpegValidPol,
	//                 MpegOutParamPtr->MpegClkPol,
	//                 MpegOutParamPtr->MpegSyncPol,
	//                 MpegOutParamPtr->MpegSyncPulseWidth,
	//                 MpegOutParamPtr->MpegClkSource,
	//                 MpegOutParamPtr->MpegClkFreq,
	//                MpegOutParamPtr->MpegErrorIndication);

	/* Clock source  */
	if (MpegOutParamPtr->MpegClkSource == MPEG_CLK_INTERNAL) 
		control = MPEG_CLK_INTERNAL;
	else 
		control = MPEG_CLK_EXTERNAL; /* Use external MPEG Clock */

	/* MPEG Clock Frequency */
	control |= (MpegOutParamPtr->MpegClkFreq << 2);

	if (MpegOutParamPtr->SerialOrPar == MPEG_DATA_PARALLEL)
	{
		/* Parallel */
		control |= (MPEG_DATA_PARALLEL << 1);

		/* MPEG Clock Frequency */
		control |= (MpegOutParamPtr->MpegClkFreq << 2);

		/* MPEG Clock polarity */
		if (MpegOutParamPtr->MpegClkPol == MPEG_CLK_NEGATIVE) 
			control |= (MPEG_CLK_NEGATIVE << 5);

		/* MPEG Data Valid level */
		if (MpegOutParamPtr->MpegValidPol == MPEG_ACTIVE_HIGH) 
			control |= (MPEG_ACTIVE_HIGH << 6);

		/* MPEG Sync Level */
		if (MpegOutParamPtr->MpegSyncPol == MPEG_ACTIVE_HIGH) 
			control |= (MPEG_ACTIVE_HIGH << 7);
	}

	status |= Ctrl_WriteRegister(i2cAddr, MPEGOUT_PAR_CLK_CTRL_REG, control);

	if (MpegOutParamPtr->MpegErrorIndication == MXL_ENABLE)
		control = ENABLE_MPEG_ERROR_IND;
	else 
		control = 0;

	/* If MPEG Data out format is serial, configure for phase and polarity */
	if (MpegOutParamPtr->SerialOrPar == MPEG_DATA_SERIAL)
	{
		/* MPEG Data Valid level */
		if (MpegOutParamPtr->MpegValidPol == MPEG_ACTIVE_LOW) 
			control |= MPEG_ACTIVE_HIGH;

		/* MPEG Sync Level */
		if (MpegOutParamPtr->MpegSyncPol == MPEG_ACTIVE_LOW) 
			control |= (MPEG_ACTIVE_HIGH << 1);

		/* LSB or MSB first */
		if (MpegOutParamPtr->LsbOrMsbFirst == MPEG_SERIAL_MSB_1ST) 
			control |= (MPEG_SERIAL_MSB_1ST << 2);

		/* SYNC Pulse width */
		if (MpegOutParamPtr->MpegSyncPulseWidth == MPEG_SYNC_WIDTH_BYTE) 
			control |= (MPEG_SYNC_WIDTH_BYTE << 3);

		/* MPEG Clock polarity */
		if (MpegOutParamPtr->MpegClkPol == MPEG_CLK_NEGATIVE) 
			control |= (MPEG_CLK_NEGATIVE << 4);
	}
	else
	{
		/* For parallel, DISABLE_FIFO_READ_LIMIT has to be set */
		control |= FIFO_READ_UNLIMIT;
	}
	status |= Ctrl_WriteRegister(i2cAddr, MPEGOUT_SER_CTRL_REG, control);
	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDevPowerSavingMode - MXL_DEV_POWER_MODE_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/19/2009
--|                 2/14/2009
--|                 3/02/2010
--|
--| DESCRIPTION   : This function configures Standby mode and Sleep mode to
--|                 control power consumption.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDevPowerSavingMode(PMXL_PWR_MODE_CFG_T PwrModePtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 control;

	switch (PwrModePtr->PowerMode)
	{
		case STANDBY_ON:
		case SLEEP_ON:
			/* Sequencer settings  */
			status |= Ctrl_WriteRegister(PwrModePtr->I2cSlaveAddr, RF_SEQUENCER_REG, STOP_TUNE); 

			/* Power down Tuner  */
			status |= Ctrl_WriteRegister(PwrModePtr->I2cSlaveAddr, TOP_MASTER_CONTROL_REG, DISABLE_TOP_MASTER);

			/* REG_BIAS = 0 <5:4> */
			status |= Ctrl_ReadRegister(PwrModePtr->I2cSlaveAddr, DN_LNA_BIAS_CTRL_REG, &control);
			control &= 0x00CF;
			status |= Ctrl_WriteRegister(PwrModePtr->I2cSlaveAddr, DN_LNA_BIAS_CTRL_REG, control);

			/* Enable Loop Through  */
			status |= Ctrl_ReadRegister(PwrModePtr->I2cSlaveAddr, LOOP_THROUGH_CTRL_REG, &control);
			control &= ~ENABLE_LOOP_THRU;
			control |= ENABLE_LOOP_THRU;
			status |= Ctrl_WriteRegister(PwrModePtr->I2cSlaveAddr, LOOP_THROUGH_CTRL_REG, control);

			if (PwrModePtr->PowerMode == SLEEP_ON)
			{
				/* Disable Clock Out  */
				status |= Ctrl_ReadRegister(PwrModePtr->I2cSlaveAddr, XTAL_CLK_OUT_CTRL_REG, &control);
				control &= ~ENABLE_CLK_OUT;

				status |= Ctrl_WriteRegister(PwrModePtr->I2cSlaveAddr, XTAL_CLK_OUT_CTRL_REG, control);
			}
			break;

		case STANDBY_OFF:
		case SLEEP_OFF:
			/* REG_BIAS = 2 <5:4> */
			status |= Ctrl_ReadRegister(PwrModePtr->I2cSlaveAddr, DN_LNA_BIAS_CTRL_REG, &control);
			control &= 0x00CF;
			control |= 0x0020;
			status |= Ctrl_WriteRegister(PwrModePtr->I2cSlaveAddr, DN_LNA_BIAS_CTRL_REG, control);
			break;

		default:
			status = MXL_FALSE;
			break;
	}

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDevOverwriteDefault - MXL_DEV_OVERWRITE_DEFAULT_CFG
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 08/27/2009
--|
--| DESCRIPTION   : Overwrite default value
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDevOverwriteDefault(PMXL_OVERWRITE_DEFAULT_CFG_T ParamPtr)
{
	MXL_STATUS status;
	status = Ctrl_ProgramRegisters(ParamPtr->I2cSlaveAddr, MxL_OverwriteDefaults);
	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodAdcIqFlip - MXL_DEMOD_ADC_IQ_FLIP_CFG
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 06/18/2009
--|
--| DESCRIPTION   : Set ADC IQ Flip enable/disable
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDemodAdcIqFlip(PMXL_ADCIQFLIP_CFG_T AdcIqFlipPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 control;

	/* Enable = 1, disable = 0
	* Flip the I/Q path after ADC
	* DIG_ADCIQ_FLIP <4>
	*/
	status |= Ctrl_ReadRegister(AdcIqFlipPtr->I2cSlaveAddr, DIG_ADCIQ_FLIP_REG, &control);
	if (AdcIqFlipPtr->AdcIqFlip == MXL_ENABLE) 
		control |= 0x10;
	else 
		control &= 0xFFEF;
	status |= Ctrl_WriteRegister(AdcIqFlipPtr->I2cSlaveAddr, DIG_ADCIQ_FLIP_REG, control);
	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodAdcIqInvert
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 06/18/2009
--|
--| DESCRIPTION   : Set ADC IQ Flip oposite to the current state 
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/
static MXL_STATUS MxL_ConfigDemodAdcIqInvert(PMXL_ADCIQFLIP_CFG_T AdcIqFlipPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 control;
	status |= Ctrl_ReadRegister(AdcIqFlipPtr->I2cSlaveAddr, DIG_ADCIQ_FLIP_REG, &control);
	control ^= 0x10;
	status |= Ctrl_WriteRegister(AdcIqFlipPtr->I2cSlaveAddr, DIG_ADCIQ_FLIP_REG, control);
	AdcIqFlipPtr->AdcIqFlip = (control & 0x10)?MXL_ENABLE:MXL_DISABLE;
	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerChanDependent - MXL_TUNER_CHAN_DEPENDENT_TUNE_CFG
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 02/01/2010
--|
--| DESCRIPTION   : This function configures channel dependent tuner setting
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigTunerChanDependent(PMXL_CHAN_DEPENDENT_T ParamPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 tmpData, control;

	/* XTAL_EN_VCO_BIASBST <3> */
	status |= Ctrl_ReadRegister(ParamPtr->I2cSlaveAddr, XTAL_EN_VCO_BIASBST, &tmpData);
	tmpData &= 0xFFF7;

	if (ParamPtr->ChanDependentCfg == MXL_ENABLE)
	{
		/* DIG_TEMP_RB <7:4> */
		status |= Ctrl_ReadRegister(ParamPtr->I2cSlaveAddr, DIG_TEMP_RB, &control);
		control = (control & 0x00F0) >> 4;

		/* Channel dependent tuner setting */
		if (control < TEMP_THRESHOLD)
		{
			tmpData |= 0x0008;
		}
		status |= Ctrl_WriteRegister(ParamPtr->I2cSlaveAddr, XTAL_EN_VCO_BIASBST, tmpData);

		/* read RF_CHP_GAIN_LUT_BYP_REG value*/
		status |= Ctrl_ReadRegister(ParamPtr->I2cSlaveAddr, RF_CHP_GAIN_LUT_BYP_REG, &tmpData);
		tmpData &= 0xFFFE;

		/* Increase RF Power within certain range of freq - PG change 2.1.20.1*/
		if ((333000000 <= ParamPtr->RfFreqHz) && (ParamPtr->RfFreqHz <= 444000000))
			tmpData |= 0x01;

		status |= Ctrl_WriteRegister(ParamPtr->I2cSlaveAddr, RF_CHP_GAIN_LUT_BYP_REG, tmpData);

	}
	else
	{
		status |= Ctrl_WriteRegister(ParamPtr->I2cSlaveAddr, XTAL_EN_VCO_BIASBST, tmpData);
	}

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerChanTune - MXL_TUNER_CHAN_TUNE_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/24/2009
--|                 6/02/2010
--|
--| DESCRIPTION   : This API configures RF channel frequency and bandwidth. 
--|                 Radio Frequency unit is Hz and Bandwidth unit is MHz, and
--|                 supports either float or integer calculation depending on
--|                 conditional compile (__MXL_INTEGER_CALC_STATISTICS__)
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

#ifdef __MXL_INTEGER_CALC_STATISTICS__
static MXL_STATUS MxL_ConfigTunerChanTuneInt(PMXL_RF_TUNE_CFG_T TuneParamPtr, MXL_BOOL clearIqFlip)
{
	UINT8 status = MXL_TRUE;
	UINT64 frequency;
	UINT32 freq = 0; 
	UINT16 bandwidth = 0x6F;
	MXL_ADCIQFLIP_CFG_T AdcIqFlipCfg;
	MXL_CHAN_DEPENDENT_T ChanDependentCfg;

	/* Set ADC IQ Flip to 0  */
	AdcIqFlipCfg.I2cSlaveAddr = TuneParamPtr->I2cSlaveAddr;
	AdcIqFlipCfg.AdcIqFlip = MXL_DISABLE;

	//  MxL_DLL_DEBUG0("MXL_TUNER_CHAN_TUNE_CFG - Freq = %dHz, BW = %dMHz\n", TuneParamPtr->Frequency, TuneParamPtr->BandWidth);    

	/* Stop Tune */
	status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_SEQUENCER_REG, STOP_TUNE);

	if (clearIqFlip == MXL_ENABLE)
	{
		/* Set AGC IQ Flip */
		status |= MxL_ConfigDemodAdcIqFlip(&AdcIqFlipCfg);
	}
	if (TuneParamPtr->BandWidth == 6) 
		bandwidth = 0x49;
	else if (TuneParamPtr->BandWidth == 8) 
		bandwidth = 0x6F;
	else 
		status = MXL_FALSE; /* BandWidth is not defined */

	if (status == MXL_TRUE)
	{
		status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_CHAN_BW_REG, bandwidth);

		frequency = TuneParamPtr->Frequency;

		/* Calculate RF Channel = DIV(64*RF(Hz), 1E6) */
		frequency *= 64;

		/* Quotient */
		while (frequency >= 1000000)
		{
			frequency -= 1000000;
			freq++;
		}

		/* Set RF  */
		status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_LOW_REG, (UINT16)(freq & 0x00FF)); /* Fractional part */
		status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_HIGH_REG, (UINT16)((freq >> 8 ) & 0x00FF)); /* Integer part */

		/* Channel dependent tune setting */
		ChanDependentCfg.I2cSlaveAddr = TuneParamPtr->I2cSlaveAddr;
		ChanDependentCfg.ChanDependentCfg = MXL_ENABLE;
		ChanDependentCfg.RfFreqHz = TuneParamPtr->Frequency;
		status |= MxL_ConfigTunerChanDependent(&ChanDependentCfg);

		/* Start Tune */
		status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_SEQUENCER_REG, START_TUNE); 
	}

	return (MXL_STATUS)status;
}
static MXL_STATUS MxL_ConfigTunerChanTune(PMXL_RF_TUNE_CFG_T TuneParamPtr)
{
	return (MXL_STATUS) MxL_ConfigTunerChanTuneInt(TuneParamPtr, MXL_ENABLE);
}
#else
static MXL_STATUS MxL_ConfigTunerChanTuneInt(PMXL_RF_TUNE_CFG_T TuneParamPtr, MXL_BOOL clearIqFlip)
	{
	UINT8 status = MXL_TRUE;
	REAL32 frequency;
	UINT32 freq; 
	UINT16 bandwidth = 0;
	MXL_ADCIQFLIP_CFG_T AdcIqFlipCfg;
	MXL_CHAN_DEPENDENT_T ChanDependentCfg;

	/* Set ADC IQ Flip to 0  */
	AdcIqFlipCfg.I2cSlaveAddr = TuneParamPtr->I2cSlaveAddr;
	AdcIqFlipCfg.AdcIqFlip = MXL_DISABLE;

	//MxL_DLL_DEBUG0("MXL_TUNER_CHAN_TUNE_CFG : Freq = %d Hz, BW = %d MHz \n", (UINT32)TuneParamPtr->Frequency, (UINT32)TuneParamPtr->BandWidth);    

	/* Stop Tune */
	status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_SEQUENCER_REG, STOP_TUNE); 

	/* Set AGC IQ Flip */
	if (clearIqFlip == MXL_ENABLE)
	{
		status |= MxL_ConfigDemodAdcIqFlip(&AdcIqFlipCfg);
	}

	if (TuneParamPtr->BandWidth == 6) 
		bandwidth = 0x49;
	else if (TuneParamPtr->BandWidth == 8) 
		bandwidth = 0x6F;
	else 
		status |= MXL_FALSE; /* BandWidth is not defined */

	status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_CHAN_BW_REG, bandwidth);

	frequency = TuneParamPtr->Frequency;

	/* Calculate RF Channel */
	frequency /= 1000000;
	//MxL_DLL_DEBUG0("MXL_TUNER_CHAN_TUNE_CFG : frequency=%d MHz\n",(UINT32)frequency);
	//frequency=299.0;
	frequency *= 64;

	/* Do round */
	frequency += 0.5;
	freq = (UINT32)frequency;
	//MxL_DLL_DEBUG0("MXL_TUNER_CHAN_TUNE_CFG : freq*64=%d\n",freq);

	/* Set RF  */
	status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_LOW_REG, (UINT16)(freq & 0x00FF)); /* Fractional part */
	status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_HIGH_REG, (UINT16)((freq >> 8 ) & 0x00FF)); /* Integer part */

	/* Channel dependent tune setting */
	ChanDependentCfg.I2cSlaveAddr = TuneParamPtr->I2cSlaveAddr;
	ChanDependentCfg.ChanDependentCfg = MXL_ENABLE;
	ChanDependentCfg.RfFreqHz = (UINT32)TuneParamPtr->Frequency;
	status |= MxL_ConfigTunerChanDependent(&ChanDependentCfg);

	/* Start Tune */
	status |= Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, RF_SEQUENCER_REG, START_TUNE); 

	return (MXL_STATUS)status;
	}
static MXL_STATUS MxL_ConfigTunerChanTune(PMXL_RF_TUNE_CFG_T TuneParamPtr)
{
	return (MXL_STATUS) MxL_ConfigTunerChanTuneInt(TuneParamPtr, MXL_ENABLE);
}
#endif

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_WaitForLockStatus
--| 
--| AUTHOR        : Mariusz Murawski 
--|
--| DATE CREATED  : 5/16/2012
--|
--| DESCRIPTION   : Wait for demod lock for defined amount of time. 
--|
--| RETURN VALUE  : True if demodulator is locked or False if not
--|---------------------------------------------------------------------------*/
static MXL_BOOL MxL_WaitForLockStatus(UINT8 I2cSlaveAddr, MXL_CMD_TYPE_E lockType, UINT16 timeoutMs)
{
	UINT32 startTime;
	MXL_STATUS status = MXL_TRUE;
	MXL_DEMOD_LOCK_STATUS_T lockStatus;

	startTime = osal_get_tick();
	lockStatus.I2cSlaveAddr = I2cSlaveAddr; 
	do
	{
		status = MxL_GetDemodLockStatus(lockType, &lockStatus);
		if (lockStatus.Status != MXL_LOCKED)
		{
			msleep(5);
		}
	} while ((status == MXL_TRUE) && (lockStatus.Status != MXL_LOCKED) && (osal_get_tick() < (startTime + timeoutMs)));
	//  MxL_DLL_DEBUG0("lock:%d (%dms)\n", lockStatus.Status, (osal_get_tick() - startTime));

	return lockStatus.Status;
}

#define MXL241_WAIT_FOR_QAM_TIMEOUT 450
#define MXL241_WAIT_FOR_FEC_TIMEOUT 50
#define MXL241_WAIT_FOR_MPEG_TIMEOUT 50

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerExtendedChanTune - MXL_TUNER_CHAN_EXTENDED_TUNE_CFG
--| 
--| AUTHOR        : Mariusz Murawski 
--|
--| DATE CREATED  : 5/16/2012
--|
--| DESCRIPTION   : This API function tunes tuner to requested frequency and 
--|                 bandwidth. Afterwards, it waits for QAM lock for certain time.
--|                 If QAM lock is not achieved, frequency offsets are applied until
--|                 demodulator reports QAM lock. Then, the function waits for FEC
--|                 and MPEG lock.
--|                 Frequency offsets list is user-defined.
--|
--| RETURN VALUE  : True if demodulator is locked or False if not
--|
--|---------------------------------------------------------------------------*/
static MXL_BOOL MxL_ConfigTunerExtendedRangeChanTune(PMXL_RF_EXTENDED_TUNE_CFG_T TuneParamPtr)
{
	MXL_STATUS done = MXL_FALSE;
	MXL_BOOL status = MXL_UNLOCKED;
	UINT8 currentFreqIndex = 0;
	UINT32 lastTunedFrequency = 0;
	MXL_RF_TUNE_CFG_T regularTuneParams;

	regularTuneParams.I2cSlaveAddr = TuneParamPtr->I2cSlaveAddr;
	regularTuneParams.BandWidth = TuneParamPtr->BandWidth;

	if (TuneParamPtr->qamLockTimeoutMs == 0) TuneParamPtr->qamLockTimeoutMs = MXL241_WAIT_FOR_QAM_TIMEOUT;
	if (TuneParamPtr->fecLockTimeoutMs == 0) TuneParamPtr->fecLockTimeoutMs = MXL241_WAIT_FOR_FEC_TIMEOUT;
	if (TuneParamPtr->mpegLockTimeoutMs == 0) TuneParamPtr->mpegLockTimeoutMs = MXL241_WAIT_FOR_MPEG_TIMEOUT;
	/*      Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, DEMOD_RESTART_REG, 0xFFFF); */


	while ((currentFreqIndex < TuneParamPtr->FreqOffsetsCnt) && (status == MXL_UNLOCKED) && (done == MXL_FALSE))
	{
		SINT32 freqOffset;

		freqOffset = TuneParamPtr->FreqOffsetsArray[currentFreqIndex];
		regularTuneParams.Frequency = (UINT32) ((SINT32) TuneParamPtr->Frequency + freqOffset);

		//    MxL_DLL_DEBUG0("Current Freq index = %d, Attempt frequency=%d\n", currentFreqIndex, regularTuneParams.Frequency);    

		/* if required frequency is the same don't call MxL_ConfigTunerChanTune again */
		/* it may happen if in previos iteration, just I/Q flip was done */
		if ((regularTuneParams.Frequency == lastTunedFrequency) || (MxL_ConfigTunerChanTuneInt(&regularTuneParams, MXL_ENABLE) == MXL_TRUE))
		{
			if (regularTuneParams.Frequency != lastTunedFrequency)
			{
				MXL_ADCIQFLIP_CFG_T MxL241SF_AdcIpFlip;
				msleep(25);
				MxL241SF_AdcIpFlip.I2cSlaveAddr = TuneParamPtr->I2cSlaveAddr;
				MxL241SF_AdcIpFlip.AdcIqFlip = TuneParamPtr->iqFlip; 
				MxL_ConfigDemodAdcIqFlip(&MxL241SF_AdcIpFlip);
			}

			Ctrl_WriteRegister(TuneParamPtr->I2cSlaveAddr, DEMOD_RESTART_REG, 0xFFFF);

			if (MxL_WaitForLockStatus(TuneParamPtr->I2cSlaveAddr, MXL_DEMOD_QAM_LOCK_REQ, TuneParamPtr->qamLockTimeoutMs) == MXL_LOCKED)
			{
				if (MxL_WaitForLockStatus(TuneParamPtr->I2cSlaveAddr, MXL_DEMOD_FEC_LOCK_REQ, TuneParamPtr->fecLockTimeoutMs) == MXL_LOCKED)
				{
					if (MxL_WaitForLockStatus(TuneParamPtr->I2cSlaveAddr, MXL_DEMOD_MPEG_LOCK_REQ, TuneParamPtr->mpegLockTimeoutMs) == MXL_LOCKED)
					{
						//MxL_DLL_DEBUG0(" QAM, FEC and Mpeg are all locked.\n");  
						status = MXL_LOCKED;
						TuneParamPtr->InitialFreqOffset = freqOffset;
					}
					done = MXL_TRUE;
				}
				else
				{
					/* if it is the first approach at this frequency*/
					if (lastTunedFrequency != regularTuneParams.Frequency)
					{
						MXL_ADCIQFLIP_CFG_T AdcIqInvert;

						AdcIqInvert.I2cSlaveAddr = TuneParamPtr->I2cSlaveAddr;
						MxL_ConfigDemodAdcIqInvert(&AdcIqInvert);
						//MxL_DLL_DEBUG0("Invert spectrum (state=%d)\n", AdcIqInvert.AdcIqFlip);    
					} 
					else 
						done = MXL_TRUE; /* otherwise - game over*/
				}
			} else currentFreqIndex++;
			lastTunedFrequency = regularTuneParams.Frequency;
		} else done = MXL_TRUE;
		/* if MxL_ConfigTunerChanTuneInt returns fail it means that I2C error was detected. Then we have to exit the function*/
	}

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerAgcLockSpeed - MXL_TUNER_AGC_LOCK_SPEED_CFG
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 01/06/2009
--|
--| DESCRIPTION   : This function configures AGC lock speed with either 
--|                 fast mode or normal mode
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigTunerAgcLockSpeed(PMXL_AGC_CTRL_SPEED_T AgcCtrlSpeedMode)
{
	MXL_STATUS status;

	switch(AgcCtrlSpeedMode->AgcSpeedMode)
	{
		case FAST_AGC_LOCK:
			status = Ctrl_WriteRegister(AgcCtrlSpeedMode->I2cSlaveAddr, AGC_CTRL_SPEED_REG, FAST_AGC_CTRL_LOCK);
			break;

		case NORMAL_AGC_LOCK:
			status = Ctrl_WriteRegister(AgcCtrlSpeedMode->I2cSlaveAddr, AGC_CTRL_SPEED_REG, NORMAL_AGC_CTRL_LOCK);
			break;

		default:
			status = MXL_FALSE;
			break;
	}

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodInterrupt - MXL_DEMOD_INTR_MASK_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : This function configures Interrupt Mask register 
--|                 to enable or disable it.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDemodInterrupt(PMXL_INTR_CFG_T IntrMaskInfoPtr)
{
	MXL_STATUS status;
	UINT16 IntrMask = IntrMaskInfoPtr->IntrMask;

	status = Ctrl_WriteRegister(IntrMaskInfoPtr->I2cSlaveAddr, INTR_MASK_REG, IntrMask); 

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodInterruptStatus
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : This function retrieves the interrupt status when Interrupt 
--|                 is triggered by demod
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDemodInterruptStatus(PMXL_INTR_STATUS_T IntrStatusInfoPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 IntrMask, IntrStatus;

	status |= Ctrl_ReadRegister(IntrStatusInfoPtr->I2cSlaveAddr, INTR_STATUS_REG, &IntrStatus);
	status |= Ctrl_ReadRegister(IntrStatusInfoPtr->I2cSlaveAddr, INTR_MASK_REG, &IntrMask);

	IntrStatusInfoPtr->IntrStatus = IntrStatus;
	IntrStatusInfoPtr->IntrMask = IntrMask;

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ClearDemodInterrupt - MXL_DEMOD_INTR_CLEAR_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : This function clears the triggered interrupt
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ClearDemodInterrupt(PMXL_INTR_CLEAR_T IntrClearInfoPtr)
{
	MXL_STATUS status;
	UINT16 IntrClear = IntrClearInfoPtr->IntrClear; 

	status = Ctrl_WriteRegister(IntrClearInfoPtr->I2cSlaveAddr, INTR_CLEAR_REG, IntrClear); 

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodLockStatus - MXL_DEMOD_QAM_LOCK_REQ
--|                                          MXL_DEMOD_FEC_LOCK_REQ, 
--|                                          MXL_DEMOD_MPEG_LOCK_REQ, 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/24/2009
--|
--| DESCRIPTION   : This function returns QAM, FEC, or MPEG Lock status of Demod.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDemodLockStatus(MXL_CMD_TYPE_E CmdType, PMXL_DEMOD_LOCK_STATUS_T DemodLockStatusPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 readBack, AnnexType;
	UINT8 lock = 0;

	switch (CmdType)
	{
		case MXL_DEMOD_QAM_LOCK_REQ:
			status = Ctrl_ReadRegister(DemodLockStatusPtr->I2cSlaveAddr, QAM_LOCK_STATUS_REG, &readBack);
			lock = (UINT8)(readBack & 0x0001);
			break;

		case MXL_DEMOD_MPEG_LOCK_REQ:
			status = Ctrl_ReadRegister(DemodLockStatusPtr->I2cSlaveAddr, FEC_MPEG_LOCK_REG, &readBack);
			//printk("lockdata:%04x\n",readBack);
			lock = (UINT8)(readBack & 0x0001);
			break;

		case MXL_DEMOD_FEC_LOCK_REQ:
			/* Check Annex Type and QAM TYPE  */
			status |= Ctrl_ReadRegister(DemodLockStatusPtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &readBack); 
			AnnexType = readBack & ANNEX_A_TYPE;    /* Bit 10 of Reg 0x8001 gives ANNEX Type */

			status |= Ctrl_ReadRegister(DemodLockStatusPtr->I2cSlaveAddr, FEC_MPEG_LOCK_REG, &readBack);

			lock = (UINT8)((readBack >> 1) & 0x0001);

			if (AnnexType == ANNEX_A_TYPE)
			{
				if (lock == 0) 
					lock = 1; /* Locked to FEC */
				else 
					lock = 0;           /* Unlock */
			}
			break;

		default:
			return MXL_FALSE;
			break;
	}

	DemodLockStatusPtr->Status = (MXL_BOOL)lock;

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodSnr - MXL_DEMOD_SNR_REQ
--| 
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/19/2009
--|                 3/02/2010
--|
--| DESCRIPTION   : This function returns SNR(Signal to Noise Ratio), and
--|                 supports either float or integer calculation depending on
--|                 conditional compile (__MXL_INTEGER_CALC_STATISTICS__)
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

#ifdef __MXL_INTEGER_CALC_STATISTICS__
static MXL_STATUS MxL_GetDemodSnr(PMXL_DEMOD_SNR_INFO_T SnrPtr)
{
	UINT8 status = MXL_TRUE, N = 16, T = 0, R = 0;
	UINT16 AnnexType, QamType, Y = 0;
	UINT16 RegData, TmpData, control;
	SINT32 L = 0;

	/* Read QAM and Annex type */
	status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &RegData); 

	AnnexType = (RegData & 0x0400) >> 10;
	QamType = (RegData & 0x0007);

	/* Before reading SNR value, write MSE_AVG_COEF */
	status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, SNR_MSE_AVG_COEF_REG, &control);
	control &= 0x07FF;
	control |= MSE_AVG_COEF;
	status |= Ctrl_WriteRegister(SnrPtr->I2cSlaveAddr, SNR_MSE_AVG_COEF_REG, control);

	status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, SNR_EXT_SPACE_ADDR_REG, &RegData);
	RegData &= 0x0100;

	/* Set command to read MSE data from Extended space data register  */
	RegData |= 0x0089;
	status |= Ctrl_WriteRegister(SnrPtr->I2cSlaveAddr, SNR_EXT_SPACE_ADDR_REG, RegData);

	status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, SNR_EXT_SPACE_DATA_REG, &RegData);

	//MxL_DLL_DEBUG0("QAM Type = %d, Annex type = %d, Reg(0x803f) = 0x%x", QamType, AnnexType, RegData);    

	switch (QamType)
	{
		case 0: /* 16 QAM */
			L = -11072;
			break;

		case 1: /* 64 QAM */
			L = -10860;
			break;

		case 2: /* 256 QAM */
			L = -10809;
			break;

		case 4: /* 32 QAM */
			L = -14082;
			break;

		case 5: /* 128 QAM */
			L = -13975;
			break;

		case 6: /* QPSK */
			L = -8519;
			break;

		default:
			status |= MXL_FALSE;
			break;
	}

	/* T = number of bits occupied in X */
	TmpData = RegData;
	while (TmpData != 0)
	{
		TmpData >>= 1;
		T++;
	}

	/* R = T - 5 if T > 5 or R = 0 */
	/* Y = X right shifted by Z-5 bits if T > 5 or X if T <= 5 */
	/* Y becomes the address of MxL_LutYLookUpTable */
	if (T > 5)
	{
		R = T - 5;
		Y = RegData >> (T - 5);
	}
	else
	{
		R = 0;
		Y = RegData;
	}

	/* 1000 * SNR = (-1) * LUT(Y) + 3010 * (N + 4 - R ) + L */
	SnrPtr->SNR = (-1) * MxL_LutYLookUpTable[Y].lutY + 3010 * (N + 4 - R) + L;

	//printk("Device = 0x%x, SNR = %ddB\n", SnrPtr->I2cSlaveAddr, SnrPtr->SNR);    

	return (MXL_STATUS)status;
}
#else
static MXL_STATUS MxL_GetDemodSnr(PMXL_DEMOD_SNR_INFO_T SnrPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 AnnexType, QamType, Es, M;
	UINT16 RegData, control;
	REAL32 Mse, constant, k;
	UINT32 CalcRes;

	/* Read QAM and Annex type */
	status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &RegData); 

	AnnexType = (RegData & 0x0400) >> 10;
	QamType = (RegData & 0x0007);

	/* Before reading SNR value, write MSE_AVG_COEF */
	status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, SNR_MSE_AVG_COEF_REG, &control);
	control &= 0x07FF;
	control |= MSE_AVG_COEF;
	status |= Ctrl_WriteRegister(SnrPtr->I2cSlaveAddr, SNR_MSE_AVG_COEF_REG, control);

	status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, SNR_EXT_SPACE_ADDR_REG, &RegData);
	RegData &= 0x0100;

	/* Set command to read MSE data from Extended space data register  */
	RegData |= 0x0089;
	status |= Ctrl_WriteRegister(SnrPtr->I2cSlaveAddr, SNR_EXT_SPACE_ADDR_REG, RegData);

	status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, SNR_EXT_SPACE_DATA_REG, &RegData);

	// MxL_DLL_DEBUG0("QAM Type = %d, Annex type = %d, Reg(0x803f) = 0x%x", QamType, AnnexType, RegData);    

	switch (QamType)
	{
		case 0: /* 16 QAM */
			M = 8;  
			Es = 10;
			break;

		case 1: /* 64 QAM */
			M = 16;  
			Es = 42;
			break;

		case 2: /* 256 QAM */
			M = 32;  
			Es = 170;
			break;

		case 4: /* 32 QAM */
			M = 16;  
			Es = 20;
			break;

		case 5: /* 128 QAM */
			M = 32;  
			Es = 82;
			break;

		case 6: /* QPSK */
			M = 8;  
			Es = 18;
			break;

		default:
			M = 1;  
			Es = 1;
			break;
	}

	/*
	* MSE = 10log10(RegData * 2^ -(N + Log2(1/16)) * 8/K)
	* SNR = -1 * MSE
	* N = 16 (No. of Bits)
	* K = Es/(M/2)2
	* E & M change with QAM types
	*/
	constant = 1.0/1048576; /* 2 ^(-N + Log2(1/16); */
	k = (REAL32)pow((M/2), 2.0);
	k = (REAL32)(Es)/k;

	if (RegData != 0)
	{
		Mse = 10 * (REAL32)log10((REAL32)RegData * constant * (8/k));
		SnrPtr->SNR = -1 * Mse;
	}
	else
	{
		SnrPtr->SNR = 0.0;
	}

	/* SNR/MSE size (num of symbols) */
	status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, PHY_EQUALIZER_LLP_CONTROL_1_DEBUG_MSE_REGISTER, &RegData);
	RegData = (RegData & 0xF800) >> 11;
	CalcRes = (RegData + 1)/2 + 5;
	/* pow(2, CalcRes) */
	SnrPtr->MseSize = 1 << CalcRes;

	//printk("Device = 0x%x, SNR = %fdB\n", SnrPtr->I2cSlaveAddr, SnrPtr->SNR);    

	return (MXL_STATUS)status;       
}
#endif

/*------------------------------------------------------------------------------
--| FUNCTION NAME : GetMCNSSD
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/24/2008
--|
--| DESCRIPTION   : Read back Counter registers for BER, CER calculation.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS GetMCNSSD(UINT8 I2cSlaveAddr, UINT8 RegCode, UINT32 *dataPtr)
{
	UINT8 status = MXL_TRUE;
	UINT32 tmp;
	UINT16 lsb;
	UINT16 msb;

	switch (RegCode)
	{
		case CW_CORR_COUNT:
			/* CW_CORR_COUNT = MSB * 2^16 +  LSB */
			/*  Get LSB */
			status |= Ctrl_WriteRegister(I2cSlaveAddr, MCNSSD_SEL_REG, 0x0002);
			status |= Ctrl_ReadRegister(I2cSlaveAddr, MCNSSD_REG, &lsb);
			/* Get MSB */
			status |= Ctrl_WriteRegister(I2cSlaveAddr, MCNSSD_SEL_REG, 0x0003);
			status |= Ctrl_ReadRegister(I2cSlaveAddr, MCNSSD_REG, &msb);
			tmp = msb << 16;
			tmp |= lsb;
			break;

		case CW_ERR_COUNT:
			/* CW_ERR_COUNT = MSB * 2^16 +  LSB */
			/* Get LSB */
			status |= Ctrl_WriteRegister(I2cSlaveAddr, MCNSSD_SEL_REG, 0x0004);
			status |= Ctrl_ReadRegister(I2cSlaveAddr, MCNSSD_REG, &lsb);

			/* Get MSB */
			status |= Ctrl_WriteRegister(I2cSlaveAddr, MCNSSD_SEL_REG, 0x0005);
			status |= Ctrl_ReadRegister(I2cSlaveAddr, MCNSSD_REG, &msb);

			tmp = msb << 16;
			tmp |= lsb;
			break;

		case CW_COUNT:
			/* CW_COUNT = MSB * 2^16 +  LSB */
			/* Get LSB */
			status |= Ctrl_WriteRegister(I2cSlaveAddr, MCNSSD_SEL_REG, 0x0000);
			status |= Ctrl_ReadRegister(I2cSlaveAddr, MCNSSD_REG, &lsb);
			/* Get MSB */
			status |= Ctrl_WriteRegister(I2cSlaveAddr, MCNSSD_SEL_REG, 0x0001);
			status |= Ctrl_ReadRegister(I2cSlaveAddr, MCNSSD_REG, &msb);

			tmp = msb << 16;
			tmp |= lsb;
			break;

		case CORR_BITS:
			status |= Ctrl_ReadRegister(I2cSlaveAddr, NCBL_REG, &lsb);
			status |= Ctrl_ReadRegister(I2cSlaveAddr, NCBH_REG, &msb);
			msb &= 0x00FF; /* Only 8bit is valid */

			tmp = msb << 16;
			tmp |= lsb;
			break;

		default:
			tmp = 0;
			break;
	}

	*dataPtr = tmp;

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodBer - MXL_DEMOD_BER_UNCODED_BER_CER_REQ
--| 
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/19/2009
--|                 3/02/2010
--|                 8/13/2010
--|
--| DESCRIPTION   : This function returns BER, CER, and Uncoded BER, and
--|                 supports either float or integer calculation depending on
--|                 conditional compile (__MXL_INTEGER_CALC_STATISTICS__)
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

#ifdef __MXL_INTEGER_CALC_STATISTICS__
static MXL_STATUS MxL_GetDemodBer(PMXL_DEMOD_BER_INFO_T BerInfoPtr) 
{
	UINT8 status = MXL_TRUE;
	UINT32 cw_corr_count;
	UINT32 cw_err_count;
	UINT32 cw_count;
	UINT32 cw_unerr_count;
	UINT32 corr_bits;        
	UINT32 err_mpeg;
	UINT16 received_mpeg;

	UINT32 kcer = 0, kber = 0, kuber = 0;
	UINT16 RegData, AnnexType, QamType;
	UINT8  Idx;

	/* Find a database id of the target device for statistics */
	for (Idx = 0; Idx < MAX_241SF_DEVICES; Idx++)
	{
	if (AccStatCounter[Idx].I2cSlaveAddr == BerInfoPtr->I2cSlaveAddr) break;
	}

	if (Idx == MAX_241SF_DEVICES) return MXL_FALSE;

	/* Check Annex Type and QAM TYPE */
	status |= Ctrl_ReadRegister(BerInfoPtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &RegData); 
	AnnexType = RegData & ANNEX_A_TYPE;    /* Bit 10 of Reg 0x8001 gives ANNEX Type */
	QamType = RegData & QAM_TYPE;          /* Bit <2:0> of Reg 0x8001 gives QAM Type */

	/* Clear the internal HW counters to avoid overflow */
	status |= Ctrl_WriteRegister(BerInfoPtr->I2cSlaveAddr, STAMP_REG, 0x0055);

	GetMCNSSD(BerInfoPtr->I2cSlaveAddr, CW_CORR_COUNT, &cw_corr_count);
	GetMCNSSD(BerInfoPtr->I2cSlaveAddr, CW_ERR_COUNT, &cw_err_count);
	GetMCNSSD(BerInfoPtr->I2cSlaveAddr, CW_COUNT, &cw_count);
	GetMCNSSD(BerInfoPtr->I2cSlaveAddr, CORR_BITS, &corr_bits);

	cw_unerr_count = cw_count - cw_corr_count - cw_err_count;

	status |= Ctrl_ReadRegister(BerInfoPtr->I2cSlaveAddr, FRCNT_REG, &RegData); 

	/* ERR_MPEG */
	if (AnnexType == ANNEX_A_TYPE) 
	{
	received_mpeg = 0xFFFF - RegData;
	err_mpeg = cw_err_count;
	}
	else
	{
	received_mpeg = RegData;

	status |= Ctrl_ReadRegister(BerInfoPtr->I2cSlaveAddr, MEF_REG, &RegData); 
	err_mpeg = RegData;
	}

	/* Update Statistic counter */
	AccStatCounter[Idx].AccCwCorrCount += cw_corr_count;
	AccStatCounter[Idx].AccCwErrCount += cw_err_count;
	AccStatCounter[Idx].AccCwUnerrCount += cw_unerr_count;
	AccStatCounter[Idx].AccCorrBits += corr_bits;
	AccStatCounter[Idx].AccErrMpeg += err_mpeg;
	AccStatCounter[Idx].AccReceivedMpeg += received_mpeg;
	AccStatCounter[Idx].AccCwReceived = AccStatCounter[Idx].AccCwCorrCount + \
	                          AccStatCounter[Idx].AccCwErrCount  + \
	                          AccStatCounter[Idx].AccCwUnerrCount;

	/* Calculate BER, CER, and Uncoded BER */
	if (AnnexType == ANNEX_A_TYPE) 
	{
		if (QamType == MXL_QAM64)
		{
			kcer = 43844;
			kber = 263;
			kuber = 27;
		}
		else if (QamType == MXL_QAM16)
		{
			kcer = 65788;
			kber = 394;    
			kuber = 40;
		}
		else if (QamType == MXL_QAM32)
		{
			kcer = 52630;
			kber = 315;    
			kuber = 32;
		}
		else if (QamType == MXL_QAM128)
		{
			kcer = 37593;
			kber = 225;    
			kuber = 23;
		}
		else
		{
			kcer = 32894;
			kber = 197;
			kuber = 20;
		}
	}
	else
	{
		if (QamType == MXL_QAM64)
		{
			kcer = 35129;
			kber = 411;
			kuber = 39;
		}
		else
		{
			kcer = 24418;
			kber = 286;
			kuber = 27;
		}
	}

	/* Calculate CER, BER, and Uncoded BER */
	BerInfoPtr->CER = kcer * cw_err_count;
	BerInfoPtr->BER = kber * err_mpeg;
	BerInfoPtr->UncodedBER = kuber * corr_bits;
#if 0  
	BerInfoPtr->TotalByte=AccStatCounter[Idx].AccReceivedMpeg*188;
	BerInfoPtr->UncorrectedByte=AccStatCounter[Idx].AccCwErrCount;
	BerInfoPtr->CorrectedByte=AccStatCounter[Idx].AccCorrBits/8;
#else
	BerInfoPtr->TotalByte=received_mpeg*188;
	BerInfoPtr->UncorrectedByte=cw_err_count;
	BerInfoPtr->CorrectedByte=corr_bits/8;
#endif

	//MxL_DLL_DEBUG0("CER = 0x%08Lx\n", BerInfoPtr->CER);
	//MxL_DLL_DEBUG0("BER = 0x%08Lx\n", BerInfoPtr->BER);
	//MxL_DLL_DEBUG0("Uncoded BER = 0x%08Lx\n", BerInfoPtr->UncodedBER);
	//if (cw_err_count) 
		//printk("cw_err_count=%d\n", cw_err_count);

	return (MXL_STATUS)status;
}
#else
static MXL_STATUS MxL_GetDemodBer(PMXL_DEMOD_BER_INFO_T BerInfoPtr) 
{
	UINT8 status = MXL_TRUE;
	UINT32 cw_corr_count;
	UINT32 cw_err_count;
	UINT32 cw_count;
	UINT32 cw_unerr_count;
	UINT32 corr_bits;        
	UINT32 err_mpeg;
	UINT16 received_mpeg;

	UINT16 RegData, AnnexType;
	UINT8  Idx;

	/* Find a database id of the target device for statistics */
	for (Idx = 0; Idx < MAX_241SF_DEVICES; Idx++)
	{
		if (AccStatCounter[Idx].I2cSlaveAddr == BerInfoPtr->I2cSlaveAddr) 
			break;
	}

	if (Idx == MAX_241SF_DEVICES) return MXL_FALSE;

	/* Check Annex Type and QAM TYPE */
	status |= Ctrl_ReadRegister(BerInfoPtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &RegData); 
	AnnexType = RegData & ANNEX_A_TYPE;    /* Bit 10 of Reg 0x8001 gives ANNEX Type */

	/* Clear the internal HW counters to avoid overflow */
	status |= Ctrl_WriteRegister(BerInfoPtr->I2cSlaveAddr, STAMP_REG, 0x0055);

	GetMCNSSD(BerInfoPtr->I2cSlaveAddr, CW_CORR_COUNT, &cw_corr_count);
	GetMCNSSD(BerInfoPtr->I2cSlaveAddr, CW_ERR_COUNT, &cw_err_count);
	GetMCNSSD(BerInfoPtr->I2cSlaveAddr, CW_COUNT, &cw_count);
	GetMCNSSD(BerInfoPtr->I2cSlaveAddr, CORR_BITS, &corr_bits);    

	cw_unerr_count = cw_count - cw_corr_count - cw_err_count;

	status |= Ctrl_ReadRegister(BerInfoPtr->I2cSlaveAddr, FRCNT_REG, &RegData); 

	/* ERR_MPEG */
	if (AnnexType == ANNEX_A_TYPE) 
	{
		received_mpeg = 0xFFFF - RegData;
		err_mpeg = cw_err_count;
	}
	else
	{
		received_mpeg = RegData;

		status |= Ctrl_ReadRegister(BerInfoPtr->I2cSlaveAddr, MEF_REG, &RegData); 
		err_mpeg = RegData;
	}

	/* Update Statistic counter */
	AccStatCounter[Idx].AccCwCorrCount += cw_corr_count;
	AccStatCounter[Idx].AccCwErrCount += cw_err_count;
	AccStatCounter[Idx].AccCwUnerrCount += cw_unerr_count;
	AccStatCounter[Idx].AccCorrBits += corr_bits;
	AccStatCounter[Idx].AccErrMpeg += err_mpeg;
	AccStatCounter[Idx].AccReceivedMpeg += received_mpeg;
	AccStatCounter[Idx].AccCwReceived = AccStatCounter[Idx].AccCwCorrCount + \
	          AccStatCounter[Idx].AccCwErrCount  + \
	          AccStatCounter[Idx].AccCwUnerrCount;

	/* Check boundary case */
	if (AccStatCounter[Idx].AccCwReceived == 0)
	{
		BerInfoPtr->CER = 0.0;
	}
	else
	{
		/* Calculate CER */
		BerInfoPtr->CER = (REAL64)AccStatCounter[Idx].AccCwErrCount / AccStatCounter[Idx].AccCwReceived;
	}

	/* Calculate BER and Uncoded BER */
	if (AnnexType == ANNEX_A_TYPE) 
	{
		if (AccStatCounter[Idx].AccReceivedMpeg == 0) 
			BerInfoPtr->BER = 0.0;
		else 
			BerInfoPtr->BER = (9.0 * (REAL64)AccStatCounter[Idx].AccErrMpeg) / (188.0 * 8.0 * (REAL64)AccStatCounter[Idx].AccReceivedMpeg);

		if (AccStatCounter[Idx].AccCwReceived == 0) 
			BerInfoPtr->UncodedBER = 0.0;
		else 
			BerInfoPtr->UncodedBER = (REAL64)AccStatCounter[Idx].AccCorrBits / ((REAL64)AccStatCounter[Idx].AccCwReceived * 204.0 * 8.0);
	}
	else
	{
		if (AccStatCounter[Idx].AccReceivedMpeg == 0) 
			BerInfoPtr->BER = 0.0;
		else 
			BerInfoPtr->BER = (10.0 * (REAL64)AccStatCounter[Idx].AccErrMpeg) / (188.0 * 8.0 * (REAL64)AccStatCounter[Idx].AccReceivedMpeg);

		if (AccStatCounter[Idx].AccCwReceived == 0) 
			BerInfoPtr->UncodedBER = 0.0;
		else 
			BerInfoPtr->UncodedBER = (REAL64)AccStatCounter[Idx].AccCorrBits / ((REAL64)AccStatCounter[Idx].AccCwReceived * 128.0 * 7.0);
	}

	BerInfoPtr->TotalByte=AccStatCounter[Idx].AccReceivedMpeg*204;
	BerInfoPtr->UncorrectedByte=AccStatCounter[Idx].AccCwErrCount*204;
	BerInfoPtr->CorrectedByte=AccStatCounter[Idx].AccCorrBits/8;
	//MxL_DLL_DEBUG0("CER = %f\n", BerInfoPtr->CER);    
	//MxL_DLL_DEBUG0("BER = %f\n", BerInfoPtr->BER);    
	//MxL_DLL_DEBUG0("Uncoded BER = %f\n", BerInfoPtr->UncodedBER);    

	return (MXL_STATUS)status;       
}
#endif

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodStatisticCounters - MXL_DEMOD_STAT_COUNTERS_REQ
--| 
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/24/2009
--|
--| DESCRIPTION   : It reads back statistic counters
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDemodStatisticCounters(PMXL_DEMOD_STAT_COUNT_T StatCountPtr)
{
	MXL_STATUS status = MXL_FALSE;
	UINT8 i;

	/* Search for device's statistics database */
	for (i = 0; i < MAX_241SF_DEVICES; i++)
	{
		if (AccStatCounter[i].I2cSlaveAddr == StatCountPtr->I2cSlaveAddr) 
			break;
	}

	if (i < MAX_241SF_DEVICES)
	{
		/* Accumulated counter for corrected code word */
		StatCountPtr->AccCwCorrCount = AccStatCounter[i].AccCwCorrCount;

		/* Accumulated counter for uncorrected code word */
		StatCountPtr->AccCwErrCount = AccStatCounter[i].AccCwErrCount;

		/* Accumulated total received code words */
		StatCountPtr->AccCwReceived = AccStatCounter[i].AccCwReceived;

		/* Accumulated counter for code words received*/
		StatCountPtr->AccCwUnerrCount = AccStatCounter[i].AccCwUnerrCount;

		/* Accumulated counter for corrected bits*/
		StatCountPtr->AccCorrBits = AccStatCounter[i].AccCorrBits;

		/* Accumulated counter for erred mpeg frames*/
		StatCountPtr->AccErrMpeg = AccStatCounter[i].AccErrMpeg;

		/* Accumulated counter for received mpeg frames*/
		StatCountPtr->AccReceivedMpeg = AccStatCounter[i].AccReceivedMpeg;     

		status = MXL_TRUE;
	}

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ResetDemodStatCounters - MXL_DEMOD_RESET_STAT_COUNTER_CFG
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/25/2009
--|
--| DESCRIPTION   : It resets the statistic variales that shall be used to calculate 
--|                 BER, CER, Uncoded BER.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ResetDemodStatCounters(PMXL_RESET_COUNTER_T ResetCountPtr)
{
	MXL_STATUS status = MXL_FALSE;
	UINT8 i;

	/* Find out device's statistics database */
	for (i = 0; i < MAX_241SF_DEVICES; i++)
	{
		if (AccStatCounter[i].I2cSlaveAddr == ResetCountPtr->I2cSlaveAddr) break;
	}

	if (i < MAX_241SF_DEVICES)
	{
		AccStatCounter[i].AccCwCorrCount = 0;
		AccStatCounter[i].AccCwErrCount = 0;
		AccStatCounter[i].AccCwUnerrCount = 0;
		AccStatCounter[i].AccCwReceived = 0;
		AccStatCounter[i].AccCorrBits = 0;
		AccStatCounter[i].AccErrMpeg = 0;
		AccStatCounter[i].AccReceivedMpeg = 0;

		status = MXL_TRUE;
	}

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodAnnexQamType - MXL_DEMOD_ANNEX_QAM_TYPE_REQ
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 06/17/2009
--|
--| DESCRIPTION   : Get Annex (B or A) and Qam type
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDemodAnnexQamType(PMXL_DEMOD_ANNEXQAM_INFO_T AnnexQamTypePtr)
{
	MXL_STATUS status;
	UINT16 RegData;

	status = Ctrl_ReadRegister(AnnexQamTypePtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &RegData); 

	/* Annex type */
	AnnexQamTypePtr->AnnexType = (MXL_ANNEX_TYPE_E)((RegData & 0x0400) >> 10);

	/* QAM type */
	AnnexQamTypePtr->QamType = (MXL_QAM_TYPE_E)(RegData & 0x0007);

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodCarrierOffset - MXL_DEMOD_CARRIER_OFFSET_REQ
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 06/17/2009, 06/02/2010
--|
--| DESCRIPTION   : Get carrier offset
--|                 before reading Carrier Offset, 
--|                 addr:0x803E, data:0x0087 need to be written, and
--|                 supports either float or integer calculation depending on
--|                 conditional compile (__MXL_INTEGER_CALC_STATISTICS__)
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

#ifdef __MXL_INTEGER_CALC_STATISTICS__
static MXL_STATUS MxL_GetDemodCarrierOffset(PMXL_DEMOD_CARRIEROFFSET_INFO_T CarrierOffsetPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 sr = 0;
	UINT16 regData;

	/* Before readback, register 0x803E needs to be written by data 0x0087  */
	status |= Ctrl_WriteRegister(CarrierOffsetPtr->I2cSlaveAddr, CARRIER_OFFSET_REG, 0x0087);

	switch(CarrierOffsetPtr->SymbolRate)
	{
		case SYM_RATE_ANNEX_A_6_89MHz:
			sr = 13142;
			break;

		case SYM_RATE_ANNEX_B64_5_0569MHz:
			sr = 9645;
			break;

		case SYM_RATE_ANNEX_B256_5_3605MHz:
			sr = 10224;
			break;

		case SYM_RATE_OOB_0_772MHz:
			sr = 1377;
			break;

		case SYM_RATE_OOB_1_024MHz:
			sr = 1953;
			break;

		case SYM_RATE_OOB_1_544MHz:
			sr = 2945;
			break;

		default:
			status |= MXL_FALSE;
			break;
	}

	/* Read the current settings. */
	status |= Ctrl_ReadRegister(CarrierOffsetPtr->I2cSlaveAddr, CARRIER_OFFSET_RB_REG, &regData);

	/* 1E9 * Carrier Offset = SR * signed(X) [MHz] */
	CarrierOffsetPtr->CarrierOffset = sr * (SINT16)regData;

	return (MXL_STATUS)status;
}
#else
static MXL_STATUS MxL_GetDemodCarrierOffset(PMXL_DEMOD_CARRIEROFFSET_INFO_T CarrierOffsetPtr)
{
	MXL_DEMOD_ANNEXQAM_INFO_T AnnexQamType;
	UINT8 status = MXL_TRUE;
	UINT8 byPass;
	UINT16 regData;
	REAL32 SymbolRate = 0;
	UINT32 nominalSymRate = 0;

	/* Before readback, register 0x803E needs to be written by data 0x0087  */
	status |= Ctrl_WriteRegister(CarrierOffsetPtr->I2cSlaveAddr, CARRIER_OFFSET_REG, 0x0087);

	/* Check Annex and Qam type */
	AnnexQamType.I2cSlaveAddr = CarrierOffsetPtr->I2cSlaveAddr;
	status |= MxL_GetDemodAnnexQamType(&AnnexQamType);

	/* Read the current settings. */
	status |= Ctrl_ReadRegister(CarrierOffsetPtr->I2cSlaveAddr, SYMBOL_RATE_RESAMP_BANK_REG, &regData);

	/* Save bypass mode*/
	byPass = (UINT8)regData & 0x0010;

	/* Config Bank <2:0> */
	if (AnnexQamType.AnnexType == ANNEX_A)
	{
		/* Annex A bank = 0 */
		regData &= 0xFFF8;
	}
	else
	{
		if (AnnexQamType.QamType == MXL_QAM64)
		{
			/* Annex B and 64QAM = 1 */
			regData &= 0xFFF9;
		}
		else
		{
			/* Annex B and 256QAM = 2 */
			regData &= 0xFFFA;
		}
	}
	status |= Ctrl_WriteRegister(CarrierOffsetPtr->I2cSlaveAddr, SYMBOL_RATE_RESAMP_BANK_REG, regData);

	/* SYMBOL_RATE_RATE_HI_REG_RB <2:0> */
	status |= Ctrl_ReadRegister(CarrierOffsetPtr->I2cSlaveAddr, SYMBOL_RATE_RATE_HI_REG_RB, &regData);
	nominalSymRate = (regData & 0x0003) << 24;

	/* SYMBOL_RATE_RATE_MID_HI_REG_RB <7:0> */
	status |= Ctrl_ReadRegister(CarrierOffsetPtr->I2cSlaveAddr, SYMBOL_RATE_RATE_MID_HI_REG_RB, &regData);
	nominalSymRate |= (regData & 0x00FF) << 16;

	/* SYMBOL_RATE_RATE_MID_LO_REG_RB <7:0> */
	status |= Ctrl_ReadRegister(CarrierOffsetPtr->I2cSlaveAddr, SYMBOL_RATE_RATE_MID_LO_REG_RB, &regData);
	nominalSymRate |= (regData & 0x00FF) << 8;

	/* SYMBOL_RATE_RATE_LO_REG_RB <7:0> */
	status |= Ctrl_ReadRegister(CarrierOffsetPtr->I2cSlaveAddr, SYMBOL_RATE_RATE_LO_REG_RB, &regData);
	nominalSymRate |= (regData & 0x00FF);

	if (byPass == MXL_ENABLE)
		SymbolRate = (REAL32)(nominalSymRate * 4 / (1 << 24));
	else
		SymbolRate = (REAL32)(nominalSymRate / (REAL32)((1 << 24) * 0.75));

	if (0 != SymbolRate) 
		SymbolRate = (REAL32)(9.5 / SymbolRate);
	else 
		status |= MXL_FALSE;

	/* Read the current settings. */
	status |= Ctrl_ReadRegister(CarrierOffsetPtr->I2cSlaveAddr, CARRIER_OFFSET_RB_REG, &regData);

	/* CarrierOffset = SymbolRate * Singed(regData) * 2^(-N+1+log2(1/16)) */
	/* N = number of bits, therefore, N = 16  */
	CarrierOffsetPtr->CarrierOffset = SymbolRate * (SINT16)regData;

	/* pow (2, -19) */
	CarrierOffsetPtr->CarrierOffset *= (REAL64)1 / (1 << 19); 

	return (MXL_STATUS)status;
}
#endif

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodInterleaverDepth - MXL_DEMOD_INTERLEAVER_DEPTH_REQ
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 06/18/2009
--|
--| DESCRIPTION   : Get Interleaver Depth I and J
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDemodInterleaverDepth(PMXL_INTERDEPTH_INFO_T InterDepthPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 regData, annexType;
	PINTERLEAVER_LOOKUP_INFO_T InterDepthLoopUpTable = MxL_InterDepthLookUpTable;

	/* Read the current Annex Type <10> */
	status |= Ctrl_ReadRegister(InterDepthPtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &annexType);
	annexType &= 0x0400;

	/* before readback, register 0x80A3 needs to be written by data 0 <3:0> */
	status |= Ctrl_ReadRegister(InterDepthPtr->I2cSlaveAddr, INTERLEAVER_DEPTH_REG, &regData);

	/* written by data 0 <3:0> */
	regData &= 0xFF87;
	status |= Ctrl_WriteRegister(InterDepthPtr->I2cSlaveAddr, INTERLEAVER_DEPTH_REG, regData);

	/* Read the current settings. */
	status |= Ctrl_ReadRegister(InterDepthPtr->I2cSlaveAddr, INTERLEAVER_DEPTH_REG, &regData);

	/* Interleaver Depth I, J <6:3>
	* regData = Control word(4bits) becomes 
	* the address of InterleaverDepth LookUp Table
	*/
	regData = (regData >> 3) & 0x000F;

	if (annexType == ANNEX_B) /* Annex_B */
	{
		InterDepthPtr->InterDepthI = InterDepthLoopUpTable[regData].interDepthI;
		InterDepthPtr->InterDepthJ = InterDepthLoopUpTable[regData].interDepthJ;
	}
	else           /* Annex_A */
	{
		InterDepthPtr->InterDepthI = 12;
		InterDepthPtr->InterDepthJ = 17;
	}

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodEqualizerFilter - MXL_DEMOD_EQUALIZER_FILTER_REQ
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 06/18/2009, 06/02/2010
--|
--| DESCRIPTION   : Get Equalizer filter, and supports either float or
--|                 integer calculation depending on conditional compile
--|                 (__MXL_INTEGER_CALC_STATISTICS__)
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

#ifdef __MXL_INTEGER_CALC_STATISTICS__
static MXL_STATUS MxL_GetDemodEqualizerFilter(PMXL_DEMOD_EQUALFILTER_INFO_T EqualizerFilterPtr)
{
	UINT8 status = MXL_TRUE;
	UINT8 counter = 0, tmpValue = 0;
	UINT16 regData = 0;
	SINT16 tmpData = 0;

	/* EXTENDED SPACE ADDRESS <7:0>
	* Address auto increment enable <8>
	* FFE filer start 00 to FF
	*/
	status |= Ctrl_WriteRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_REG, 0x0100);

	/* Readback data
	* Oddth data = X, eventh data = Y
	* 1E9* FFE Tap = (signed(X) * 15259) + (j * signed(Y) * 15259)
	*/
	while (counter < 16)
	{
	status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_DATA_RB_REG, &regData);
	EqualizerFilterPtr->FfeInfo[counter] = (SINT16)regData * 15259;

	counter++;
	}

	/* Spur filer start 20 to 3F */
	status |= Ctrl_WriteRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_REG, 0x0120);

	/* Readback data
	* Oddth data = X, eventh data = Y
	* 1E9* Spur Tap = (signed(X) * 488281) + (j * signed(Y) * 488281)
	*/
	counter = 0;
	while (counter < 32)
	{
		status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_DATA_RB_REG, &regData);
		regData &= 0x0FFF;

		/* Check signed or unsigned */
		if (regData & 0x0800) 
			tmpData = (SINT16)(0xF000 | regData);
		else 
			tmpData = (SINT16)regData;

		EqualizerFilterPtr->SpurInfo[counter] = tmpData * 488281;

		counter++;
	}

	/* DFE filer start 40 to 77  */
	status |= Ctrl_WriteRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_REG, 0x0140);

	/* Readback data
	* Oddth data = X, eventh data = Y
	* 1E9* DFE Tap = (signed(X) * 15259) + (j * signed(Y) * 15259)
	*/
	counter = 0;
	while (counter < 56)
	{
		status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_DATA_RB_REG, &regData);
		EqualizerFilterPtr->DfeInfo[counter] = (SINT16)regData * 15259;

		counter++;
	}

	/* Main Tap Location */
	/* Reading register 0x8026, bits (8-9), holds the main tap location with deviation of 5 */
	status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, PHY_EQUALIZER_FFE_FILTER_REGISTER, &regData);
	tmpValue = ((regData >> 8) & 0x0003);

	switch(tmpValue)
	{
		case 0:
			EqualizerFilterPtr->DsEqMainLocation = FFE_MAIN_TAP_LOCATION_1;
			break;

		case 1:
			EqualizerFilterPtr->DsEqMainLocation = FFE_MAIN_TAP_LOCATION_2;
			break;

		case 2:
			EqualizerFilterPtr->DsEqMainLocation = FFE_MAIN_TAP_LOCATION_3;
			break;

		case 3:
			EqualizerFilterPtr->DsEqMainLocation = FFE_MAIN_TAP_LOCATION_4;
			break;

		default:
			//MxL_DLL_DEBUG0("Unsupported main tap location.\n");
			status |= MXL_FALSE;
			break;
	}

	/* Number of DFE Taps */
	/* Reading register 0x8027, bits (0-4) +1, holds the number of reverse taps */
	status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, PHY_EQUALIZER_DFE_FILTER_REGISTER, &regData);
	tmpValue = regData & 0x001F;
	EqualizerFilterPtr->DsEqDfeTapNum = (UINT8)(tmpValue + 1);

	return (MXL_STATUS)status;
}
#else
static MXL_STATUS MxL_GetDemodEqualizerFilter(PMXL_DEMOD_EQUALFILTER_INFO_T EqualizerFilterPtr)
{
	UINT8 status = MXL_TRUE;
	UINT8 counter = 0, tmpValue = 0;
	UINT16 regData = 0;
	SINT16 tmpData = 0;

	/* EXTENDED SPACE ADDRESS <7:0>
	* Address auto increment enable <8>
	* FFE filer start 00 to FF
	*/
	status |= Ctrl_WriteRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_REG, 0x0100);

	/* Readback data
	* Oddth data = X, eventh data = Y
	* FFE Tap = signed(X)*2^(-n+1+log2(1/2)) + j*signed(Y)*2^(-n+1+log2(1/2))
	* n = 16 for FFE
	*/
	while (counter < 16)
	{
		status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_DATA_RB_REG, &regData);
		EqualizerFilterPtr->FfeInfo[counter] = (REAL32)((SINT16)regData * ((REAL32)1 / (1 << 16)));
		counter++;
	}

	/* Spur filer start 20 to 3F */
	status |= Ctrl_WriteRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_REG, 0x0120);

	/* Readback data
	* Oddth data = X, eventh data = Y
	* Spur Tap = signed(X)*2^(-n+1+log2(1)) + j*signed(Y)*2^(-n+1+log2(1))
	* n = 12 for FFE
	*/
	counter = 0;
	while (counter < 32)
	{
		status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_DATA_RB_REG, &regData);
		regData &= 0x0FFF;

		/* Check signed or unsigned */
		if (regData & 0x0800) 
			tmpData = (SINT16)(0xF000 | regData);
		else 
			tmpData = (SINT16)regData;

		EqualizerFilterPtr->SpurInfo[counter] = (REAL32)(tmpData * ((REAL32)1 / (1 << 11)));

		counter++;
	}

	/* DFE filer start 40 to 77  */
	status |= Ctrl_WriteRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_REG, 0x0140);

	/* Readback data
	* Oddth data = X, eventh data = Y
	* DFE Tap = signed(X)*2^(-n+1+log2(1/2)) + j*signed(Y)*2^(-n+1+log2(1/2))
	* n = 16 for DEF
	*/
	counter = 0;
	while (counter < 56)
	{
		status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, EQUALIZER_FILTER_DATA_RB_REG, &regData);
		EqualizerFilterPtr->DfeInfo[counter] = (REAL32)((SINT16)regData * ((REAL32)1 / (1 << 16)));

		counter++;
	}

	/* Main Tap Location */
	/* Reading register 0x8026, bits (8-9), holds the main tap location with deviation of 5 */
	status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, PHY_EQUALIZER_FFE_FILTER_REGISTER, &regData);
	tmpValue = ((regData >> 8) & 0x0003);

	switch(tmpValue)
	{
		case 0:
			EqualizerFilterPtr->DsEqMainLocation = FFE_MAIN_TAP_LOCATION_1;
			break;

		case 1:
			EqualizerFilterPtr->DsEqMainLocation = FFE_MAIN_TAP_LOCATION_2;
			break;

		case 2:
			EqualizerFilterPtr->DsEqMainLocation = FFE_MAIN_TAP_LOCATION_3;
			break;

		case 3:
			EqualizerFilterPtr->DsEqMainLocation = FFE_MAIN_TAP_LOCATION_4;
			break;

		default:
			//MxL_DLL_DEBUG0("Unsupported main tap location.\n");
			status |= MXL_FALSE;
			break;
	}

	/* Number of DFE Taps */
	/* Reading register 0x8027, bits (0-4) +1, holds the number of reverse taps */
	status |= Ctrl_ReadRegister(EqualizerFilterPtr->I2cSlaveAddr, PHY_EQUALIZER_DFE_FILTER_REGISTER, &regData);
	tmpValue = regData & 0x001F;
	EqualizerFilterPtr->DsEqDfeTapNum = (UINT8)(tmpValue + 1);

	return (MXL_STATUS)status;
}
#endif

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodTimingOffset - MXL_DEMOD_TIMING_OFFSET_REQ
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 07/21/2009
--|
--| DESCRIPTION   : Get timing-offset of the receiver
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDemodTimingOffset(PMXL_DEMOD_TIMINGOFFSET_INFO_T TimingOffsetPtr)
{
	UINT8 status = MXL_TRUE;
	UINT8 bankCfgData = 0;
	UINT16 regData;
	SINT16 godardAcc;
	SINT64 extendGodardAcc;
	UINT32 tmpRate = 0;
	MXL_DEMOD_ANNEXQAM_INFO_T AnnexQamType;

	/* Readback current Annex and QAM type first */
	AnnexQamType.I2cSlaveAddr = TimingOffsetPtr->I2cSlaveAddr;
	status = MxL_GetDemodAnnexQamType(&AnnexQamType);

	switch(AnnexQamType.AnnexType)
	{
		case ANNEX_B:
			switch(AnnexQamType.QamType)
			{
				case MXL_QAM64:
					bankCfgData = 1;
					break;

				case MXL_QAM256:
					bankCfgData = 2;
					break;

				default:
					return MXL_FALSE;
			}
			break;

		case ANNEX_A:
			bankCfgData = 0;
			break;
	}

	/* Config bank <2:0> */
	status |= Ctrl_ReadRegister(TimingOffsetPtr->I2cSlaveAddr, RESAMP_BANK_REG, &regData);
	regData = ((regData & 0xFFF8) | bankCfgData);
	status |= Ctrl_WriteRegister(TimingOffsetPtr->I2cSlaveAddr, RESAMP_BANK_REG, regData);

	/* Resample readback: High<10:0>, Low<15:0> */
	status |= Ctrl_ReadRegister(TimingOffsetPtr->I2cSlaveAddr, RATE_RESAMP_RATE_MID_HI_REG, &regData);
	tmpRate = (regData & 0x07FF) << 16;
	status |= Ctrl_ReadRegister(TimingOffsetPtr->I2cSlaveAddr, RATE_RESAMP_RATE_MID_LO_REG, &regData);
	tmpRate = ((tmpRate & 0xFFFF0000) | regData);

	/* Calculate timing offset (ppm unit) */
	status |= Ctrl_ReadRegister(TimingOffsetPtr->I2cSlaveAddr, GODARD_ACC_REG, (UINT16*)&godardAcc);
	if (0 != tmpRate)
	{
#ifndef __MXL_INTEGER_CALC_STATISTICS__
		TimingOffsetPtr->TimingOffset = ((REAL64)godardAcc * 1000000) / tmpRate;
#else
		extendGodardAcc =  (SINT64)godardAcc;
		uint64_t temp = extendGodardAcc * 1000000;
		do_div(temp, tmpRate);
		TimingOffsetPtr->TimingOffset = temp;
#endif
	}
	else
		status |= MXL_FALSE;

	return (MXL_STATUS)status;
}

/*---------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodConfirmedLockStatus - MXL_DEMOD_CONFIRMED_LOCK_STATUS_REQ
--|
--| AUTHOR        : SidS
--|
--| DATE CREATED  : 11/09/2011
--|
--| DESCRIPTION   : This function returns confirmed QAM, FEC, and MPEG Lock statuses of Demod.
--|
--|                 In case of Annex-A:
--|                 - It also checks for a false lock condition with specified FagcGainThreshold.
--|                 - The demod will be restarted in case of unlock (real-unlock or false-lock).
--|
--| NOTE          : If FagcGainThreshold is 0, default threshold 9 will be used (recommended)
--|                 Any non-zero FagcGainThreshold will overwrite the default 9.
--|
--| RETURN VALUE  : MXL_TRUE or MXL_FALSE
--|
--|-------------------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDemodConfirmedLockStatus(PMXL_DEMOD_CONFIRMED_LOCK_STATUS_T cmdPtr)
{
	UINT8  status = MXL_TRUE;
	UINT16 annexType;
	UINT16 readBack;
	UINT16 fagcThreshold = cmdPtr->FagcGainThreshold;
	MXL_STATUS restartDemod = MXL_FALSE;

	if (fagcThreshold == 0)
	{
	/* Use default value of 9*/
	fagcThreshold = 9;
	}

	cmdPtr->FalseLockDetected = MXL_FALSE;

	/* Get QAM lock status*/
	status |= (UINT8)Ctrl_ReadRegister(cmdPtr->I2cSlaveAddr, QAM_LOCK_STATUS_REG , &readBack);
	cmdPtr->QamLockStatus = (MXL_BOOL)(readBack & 0x0001);

	/* Get Annex Type and QAM TYPE*/
	status |= Ctrl_ReadRegister(cmdPtr->I2cSlaveAddr, QAM_ANNEX_TYPE_REG, &readBack); 
	annexType = readBack & ANNEX_A_TYPE;    /* Bit 10 of Reg 0x8001 gives ANNEX Type */

	/* Read FEC and MEPG lock status*/
	status |= Ctrl_ReadRegister(cmdPtr->I2cSlaveAddr, FEC_MPEG_LOCK_REG, &readBack);

	cmdPtr->FecLockStatus = (MXL_BOOL)((readBack >> 1) & 0x0001);

	cmdPtr->MpegLockStatus = (MXL_BOOL)(readBack & 0x0001);

	if (annexType == ANNEX_A_TYPE)
	{
		/* Flip lock bit for ANNEX_A type*/
		cmdPtr->FecLockStatus = (MXL_LOCKED == cmdPtr->FecLockStatus)?MXL_UNLOCKED:MXL_LOCKED;

		if (   MXL_LOCKED != cmdPtr->QamLockStatus
		|| MXL_LOCKED != cmdPtr->FecLockStatus
		|| MXL_LOCKED != cmdPtr->MpegLockStatus)
		{
			/* If any of QAM/FEC/MPEG is unlocked, require demod restart*/
			restartDemod = MXL_TRUE;
		}
		else
		{
			/* If all of QAM/FEC/MPEG are locked, further check EQU_FAGC_GAIN8/
			/* Read EQU_FAGC_GAIN*/
			status |= Ctrl_WriteRegister(cmdPtr->I2cSlaveAddr, SNR_EXT_SPACE_ADDR_REG, (UINT16)0x86);
			status |= Ctrl_ReadRegister(cmdPtr->I2cSlaveAddr, SNR_EXT_SPACE_DATA_REG, &readBack);
			readBack &= 0x0FFF; 

			if (readBack < fagcThreshold)
			{
				/* False lock detected*/
				cmdPtr->QamLockStatus  = MXL_UNLOCKED;
				cmdPtr->FecLockStatus  = MXL_UNLOCKED;
				cmdPtr->MpegLockStatus = MXL_UNLOCKED;
				cmdPtr->FalseLockDetected = MXL_TRUE;
				restartDemod = MXL_TRUE;
			}
		}
		if ((MXL_BOOL) MXL_TRUE == restartDemod) 
		{
			/* Restart demod*/
			status |= Ctrl_WriteRegister(cmdPtr->I2cSlaveAddr, DEMOD_RESTART_REG, 0xFFFF);
		}
	}

	//MxL_DLL_DEBUG0("MXL_DEMOD_CONFIRMED_LOCK_STATUS_REQ - i2cAddr=0x%x, FagcThreshold=%d, QamLockStatus:%d, FecLockStatus:%d, MpegLockStatus:%d, FalseLockDetected=%d\n",
	//    cmdPtr->I2cSlaveAddr,
	//    fagcThreshold,
	//    cmdPtr->QamLockStatus,
	//    cmdPtr->FecLockStatus,
	//    cmdPtr->MpegLockStatus,
	//    cmdPtr->FalseLockDetected);

	return (MXL_STATUS)status;
}

#ifdef __MXL241_DEBUG__
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetCurrentBW - MXL_DEMOD_DBG_CURRENT_BW_REQ
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 06/17/2009
--|
--| DESCRIPTION   : Get current channel bandwidth
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDemodDbgCurrentBW(PMXL_BW_INFO_T CurrentBwPtr)
{
	MXL_STATUS status;
	UINT16 regData;

	/* Read current BandWidth */
	status = Ctrl_ReadRegister(CurrentBwPtr->I2cSlaveAddr, RF_CHAN_BW_REG, &regData);

	switch(regData)
	{
		case BW_6MHz:
			CurrentBwPtr->CurrnetBW = BW_6MHz;
			break;

		case BW_8MHz:
			CurrentBwPtr->CurrnetBW = BW_8MHz;
			break;

		default:
			CurrentBwPtr->CurrnetBW = UNKNOWN;
			break;
	}

	return status;
}
#endif

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetDemodReTuneIndicator - MXL_DEMOD_RETUNE_INDICATOR_REQ
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 04/13/2011
--|
--| DESCRIPTION   : Get retune indicator status
--|                 When both FEC and MPEG locked, calling this API returns
--|                 retune required or not
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetDemodReTuneIndicator(PMXL_RETUNE_IND_T ParamPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 regData;

	/* Before readback, register 0x803E needs to be written by data 0x0086  */
	status = Ctrl_WriteRegister(ParamPtr->I2cSlaveAddr, SNR_EXT_SPACE_ADDR_REG, 0x0086);

	status |= Ctrl_ReadRegister(ParamPtr->I2cSlaveAddr, SNR_EXT_SPACE_DATA_REG, &regData);

	ParamPtr->ReTuneInd = (regData < RETUNE_INDICATOR_THRESHOLD) ? MXL_TRUE : MXL_FALSE;

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodEqualizerSetting - MXL_DEMOD_EQUALIZER_FILTER_CFG
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 06/16/2009
--|
--| DESCRIPTION   : Set Demodulator Equalizer
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDemodEqualizerSetting(PMXL_EQUALIZER_CFG_T EqualSettingPtr)
{  
	UINT8 status = MXL_TRUE;
	UINT16 control;

	//MxL_DLL_DEBUG0("MXL_DEMOD_EQUALIZER_FILTER_CFG - EqualizerSetting = %d", EqualSettingPtr->EqualizerSetting);

	status |= Ctrl_ReadRegister(EqualSettingPtr->I2cSlaveAddr, EQ_SPUR_BYPASS_REG, &control);

	/* EQ_SPUR_BYPASS <9> */
	/* Enable = 1, Disable = 0 */
	if (EqualSettingPtr->EqualizerSetting == MXL_ENABLE) 
		control |= 0x0200;
	else 
		control &= 0xFDFF;

	status |= Ctrl_WriteRegister(EqualSettingPtr->I2cSlaveAddr, EQ_SPUR_BYPASS_REG, control);

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetTunerLockStatus - MXL_TUNER_LOCK_STATUS_REQ
--|
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/25/2009
--|                 11/10/2009
--|                 3/02/2010
--|
--| DESCRIPTION   : This function returns Tuner, AGC Lock, and
--|                 Tuner Done status after tuning.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetTunerLockStatus(PMXL_TUNER_LOCK_STATUS_T TunerLockStatusPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 lock;

	status = Ctrl_ReadRegister(TunerLockStatusPtr->I2cSlaveAddr, RF_LOCK_STATUS_REG, &lock);
#ifdef __MXL241_DEBUG__
	/* RF Sync <3:2> */
	if ((lock & RF_SYN_RDY) == RF_SYN_RDY)
		TunerLockStatusPtr->RfSynthStatus = MXL_LOCKED;
	else
		TunerLockStatusPtr->RfSynthStatus = MXL_UNLOCKED;

	/* RF Sync <1:0> */
	if ((lock & REF_SYN_RDY) == REF_SYN_RDY)
		TunerLockStatusPtr->RefSynthStatus = MXL_LOCKED;
	else
		TunerLockStatusPtr->RefSynthStatus = MXL_UNLOCKED;
#endif
	if ((lock & TUNER_LOCKED) == TUNER_LOCKED)
	{
		TunerLockStatusPtr->TunerLockStatus = MXL_LOCKED;
	}
	else
	{
		TunerLockStatusPtr->TunerLockStatus = MXL_UNLOCKED;
	}

	/* AGC Lock <2> */
	status |= Ctrl_ReadRegister(TunerLockStatusPtr->I2cSlaveAddr, AGC_LOCK_STATUS_REG, &lock);

	if ((lock & AGC_LOCKED) == AGC_LOCKED)
	{
		TunerLockStatusPtr->AgcLockStatus = MXL_LOCKED;
	}
	else
	{
		TunerLockStatusPtr->AgcLockStatus = MXL_UNLOCKED;
	}

	/* Tuner Done <3> */
	if ((lock & TUNER_DONE) == TUNER_DONE)
	{
		TunerLockStatusPtr->TunerDoneStatus = MXL_LOCKED;
	}
	else
	{
		TunerLockStatusPtr->TunerDoneStatus = MXL_UNLOCKED;
	}

	return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerRssiThresholds
--|
--| AUTHOR        : Mariusz Murawski 
--|                
--|
--| DATE CREATED  : 6/25/2012
--|
--| DESCRIPTION   : This function sets required digital rssi value 
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/
static MXL_STATUS MxL_ConfigTunerRssiThresholds(PMXL_RF_TUNER_RSSI_THRESHOLDS_T paramPtr)
{
	MXL_STATUS status;
	UINT16 regValue;

	regValue = ((paramPtr->rssiThresholdLo & 0x0f) << 4);
	regValue |= (paramPtr->rssiThresholdHi & 0x0f);

	status = Ctrl_WriteRegister(paramPtr->I2cSlaveAddr, DIG_RSSI, regValue);

	return (MXL_STATUS) status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetTunerAgcSettings - MXL_TUNER_AGC_SETTINGS_REQ
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/25/2009
--|
--| DESCRIPTION   : This function returns the current AGC settings.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetTunerAgcSettings(PMXL_AGC_INFO_T AgcParamPtr) 
{
	MXL_STATUS status;
	UINT16 control;

	status = Ctrl_ReadRegister(AgcParamPtr->I2cSlaveAddr, DIG_HALT_GAIN_CTRL_REG, &control);
	AgcParamPtr->FreezeAgcGainWord = (MXL_BOOL)((control & FREEZE_AGC_GAIN_WORD) >> 3);

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetTunerRxPower - MXL_TUNER_RF_RX_PWR_REQ
--|
--| AUTHOR        : Brenndon Lee
--|                 Sunghoon Park
--|
--| DATE CREATED  : 6/25/2009
--|                 6/02/2010
--|
--| DESCRIPTION   : This function returns RF input power in dBm, and
--|                 supports either float or integer calculation depending on
--|                 conditional compile (__MXL_INTEGER_CALC_STATISTICS__)
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

#ifdef __MXL_INTEGER_CALC_STATISTICS__
static MXL_STATUS MxL_GetTunerRxPower(PMXL_TUNER_RX_PWR_T TunerRxPwrPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 RegRb, RfpinData;

	/* Enable RF input power readback */
	status |= Ctrl_WriteRegister(TunerRxPwrPtr->I2cSlaveAddr, RF_PIN_RB_EN_REG, ENABLE_RFPIN_RB);

	/* RF power readback 0x0018 <7:0>, 0x0019 <2:0> */
	status |= Ctrl_ReadRegister(TunerRxPwrPtr->I2cSlaveAddr, DIG_RF_PIN_RB_LO_REG, &RegRb);
	RfpinData = RegRb & 0x00FF;

	status |= Ctrl_ReadRegister(TunerRxPwrPtr->I2cSlaveAddr, DIG_RF_PIN_RB_HI_REG, &RegRb);
	RfpinData |= ((RegRb & 0x0007) << 8);

	/* Calculate */
	TunerRxPwrPtr->RxPwr = RfpinData * 125; 
	TunerRxPwrPtr->RxPwr -= 120000;

	return (MXL_STATUS)status;
}
#else
static MXL_STATUS MxL_GetTunerRxPower(PMXL_TUNER_RX_PWR_T TunerRxPwrPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 RegRb, RfpinData;

	/* Enable RF input power readback */
	status |= Ctrl_WriteRegister(TunerRxPwrPtr->I2cSlaveAddr, RF_PIN_RB_EN_REG, ENABLE_RFPIN_RB);

	/* RF power readback 0x0018 <7:0>, 0x0019 <2:0> */
	status |= Ctrl_ReadRegister(TunerRxPwrPtr->I2cSlaveAddr, DIG_RF_PIN_RB_LO_REG, &RegRb);
	RfpinData = RegRb & 0x00FF;

	status |= Ctrl_ReadRegister(TunerRxPwrPtr->I2cSlaveAddr, DIG_RF_PIN_RB_HI_REG, &RegRb);
	RfpinData |= ((RegRb & 0x0007) << 8);

	/* Get Integer part */
	TunerRxPwrPtr->RxPwr = (REAL32)(RfpinData >> 3);

	/* Calculate fractional part */
	RfpinData &= 0x7;  

	if (RfpinData & 0x1) 
		TunerRxPwrPtr->RxPwr += 0.125;

	RfpinData >>= 1;

	if (RfpinData & 0x1) 
		TunerRxPwrPtr->RxPwr += 0.25;

	RfpinData >>= 1;

	if (RfpinData & 0x1) 
		TunerRxPwrPtr->RxPwr += 0.5;

	TunerRxPwrPtr->RxPwr -= 120.0;

	return (MXL_STATUS)status;
}
#endif

#ifdef __MXL241_DEBUG__
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodAgcSelect - MXL_DEMOD_DBG_AGC_SELECT_CFG
--|
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 7/28/2009
--|
--| DESCRIPTION   : This function sets Self or Demod-agc mode
--|                 Disable: Self-AGC mode
--|                 Enable : Demod-AGC mode
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDemodDbgAgcSelect(PMXL_AGC_SELECT_CFG_T AgcSelectPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 RegData;

	status |= Ctrl_ReadRegister(AgcSelectPtr->I2cSlaveAddr, DIG_AGC_SELECT, &RegData);

	/* DIG_AGCSEL <6> */
	/* Set default value*/
	RegData &= 0xFFBF;

	if (AgcSelectPtr->AgcMode == MXL_ENABLE)
		RegData |= 0x0040;

	status |= Ctrl_WriteRegister(AgcSelectPtr->I2cSlaveAddr, DIG_AGC_SELECT, RegData);

	return (MXL_STATUS)status;
}
#endif

#ifdef __MXL241_DEBUG__
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerDbgIqOut - MXL_TUNER_DBG_IQOUT_CFG
--|
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 7/28/2009
--|
--| DESCRIPTION   : This function sets IQ out enable/disable
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigTunerDbgIqOut(PMXL_IQ_OUT_CFG_T IqOutPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 RegData;

	status |= Ctrl_ReadRegister(IqOutPtr->I2cSlaveAddr, DIG_TESTIF_OBV_MODE, &RegData);

	/* DIG_TESTIF_OBV_MODE <3:2> */
	/* Set default value*/
	RegData &= 0xFFF3;

	if (IqOutPtr->IqOutCfg == MXL_ENABLE)
		RegData |= 0x0008;

	status |= Ctrl_WriteRegister(IqOutPtr->I2cSlaveAddr, DIG_TESTIF_OBV_MODE, RegData);

	return (MXL_STATUS)status;
}
#endif

#ifdef __MXL241_DEBUG__
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_GetTunerDbgSensorTemp - MXL_TUNER_DBG_SENSOR_TEMP_REQ
--|
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 7/28/2009
--|
--| DESCRIPTION   : This function returns index for Sensor temperature
--|                 0 : less than -30C
--|                 1 : -25C
--|                 2 : -15C
--|                 3 : -5C
--|                 4 : 5C
--|                 5 : 15C
--|                 6 : 25C
--|                 7 : 35C
--|                 8 : 45C
--|                 9 : 55C
--|                 10 : 65C
--|                 11 : 75C
--|                 12 : 85C
--|                 13 : more than 90C
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_GetTunerDbgSensorTemp(PMXL_TEMP_SENSOR_T TempSensorPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 RegData;

	/* Enable temp sensor first */
	/* RFPIN_RB_EN <0> */
	status |= Ctrl_ReadRegister(TempSensorPtr->I2cSlaveAddr, ENABLE_TEMP_SENSOR_REG, &RegData);
	RegData |= 0x0001;
	status |= Ctrl_WriteRegister(TempSensorPtr->I2cSlaveAddr, ENABLE_TEMP_SENSOR_REG, RegData);

	/* DIG_TEMP_RB <7:4> */
	status |= Ctrl_ReadRegister(TempSensorPtr->I2cSlaveAddr, TEMP_SENSOR_RB_REG, &RegData);
	RegData = (RegData >> 4) & 0x000F;

	TempSensorPtr->ReadbackTemp = (UINT8)RegData;

	return (MXL_STATUS)status;
}
#endif

#ifdef __MXL241_DEBUG__
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigDemodDbgAgcSetPoint - MXL_DEMOD_DBG_AGC_SETPOINT_CFG
--|
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 7/28/2009
--|
--| DESCRIPTION   : This function sets Demod AGC set point
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigDemodDbgAgcSetPoint(PMXL_DEMOD_AGC_SETPOINT_CFG_T AgcSetPointPtr)
{
	//MxL_DLL_DEBUG0("AgcSetPointPtr->DemodAgcSetPoint = %f", AgcSetPointPtr->DemodAgcSetPoint);

	UINT8 status = MXL_TRUE;
	UINT16 RegData;
	REAL32 TmpNum;
	UINT32 TmpData;

	/* CAGC_REFERENCE <11:0> */
	status |= Ctrl_ReadRegister(AgcSetPointPtr->I2cSlaveAddr, DEMOD_AGC_SET_POINT_REG, &RegData);
	RegData &= 0xF000;

	/* RegData = round(10^(dBFS/10) * (2^15)) */
	TmpNum = pow(10, AgcSetPointPtr->DemodAgcSetPoint/10) * (1 << 15);

	/* Do round */
	TmpNum += 0.5;
	TmpData = (UINT32)TmpNum;

	RegData |= (UINT16)TmpData & 0x0FFF;
	status |= Ctrl_WriteRegister(AgcSetPointPtr->I2cSlaveAddr, DEMOD_AGC_SET_POINT_REG, RegData);

	return (MXL_STATUS)status;
}
#endif

#ifdef __MXL241_DEBUG__
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerDbgAgcSetPoint - MXL_TUNER_DBG_AGC_SETPOINT_CFG
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 7/31/2009
--|
--| DESCRIPTION   : This function sets Tuner AGC set point
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigTunerDbgAgcSetPoint(PMXL_TUNER_AGC_SETPOINT_CFG_T AgcParamPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 control, regData;

	//MxL_DLL_DEBUG0("MXL_TUNER_DBG_AGC_SETPOINT_CFG - AgcSetPoint=%d", \
	AgcParamPtr->AgcSetPoint);

	/* AGC Set point */
	/* In 10bit data resolution, upper 6 bits are an integer part and 
	* lower 4 bits are fractional bits 
	*/
	control = AgcParamPtr->AgcSetPoint << 4;

	/* LSB <7:0> */
	status = Ctrl_WriteRegister(AgcParamPtr->I2cSlaveAddr, DIG_AGC_SETPT_CTRL_LO_REG, (control & 0x00FF));

	/* MSB <1:0> AGC setpoint */
	status |= Ctrl_ReadRegister(AgcParamPtr->I2cSlaveAddr, DIG_AGC_SETPT_CTRL_HI_REG, &regData);
	regData &= 0xFFFC;
	control = (control >> 8) & 0x0003;
	control |= regData;
	status |= Ctrl_WriteRegister(AgcParamPtr->I2cSlaveAddr, DIG_AGC_SETPT_CTRL_HI_REG, control);

	return (MXL_STATUS)status;
}
#endif

#ifdef __MXL241_DEBUG__
/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL_ConfigTunerDbgGainFreezeTest - MXL_TUNER_DBG_GAIN_FREEZE_CFG
--| 
--| AUTHOR        : Sunghoon Park
--|
--| DATE CREATED  : 10/28/2009
--|
--| DESCRIPTION   : This function tests Gain Freeze test only for ADB and should be
--|                 called after demod locks
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

static MXL_STATUS MxL_ConfigTunerDbgGainFreezeTest(PMXL_GAIN_FREEZE_T GainParamPtr)
{
	UINT8 status = MXL_TRUE;
	UINT16 regDataLow, regDataHigh;
	UINT16 wDataLow, wDataHigh;

	status |= Ctrl_WriteRegister(GainParamPtr->I2cSlaveAddr, FREEZE_AGC_CFG_REG, 0x0008);

	/*  read <5:0> from 0x00EE & 0x00EF */
	status |= Ctrl_ReadRegister(GainParamPtr->I2cSlaveAddr, 0x00EE, &regDataLow);
	status |= Ctrl_ReadRegister(GainParamPtr->I2cSlaveAddr, 0x00EF, &regDataHigh);
	regDataLow &= 0x003F;
	regDataHigh &= 0x003F;

	/* write same data <5:0> to 0x008C & 0x008D */
	status |= Ctrl_ReadRegister(GainParamPtr->I2cSlaveAddr, 0x008C, &wDataLow);
	status |= Ctrl_ReadRegister(GainParamPtr->I2cSlaveAddr, 0x008D, &wDataHigh);
	wDataLow &= 0xFFC0;
	wDataHigh &= 0xFFC0;
	wDataLow |= regDataLow;
	wDataHigh |= regDataHigh;

	status |= Ctrl_WriteRegister(GainParamPtr->I2cSlaveAddr, 0x008C, regDataLow);
	status |= Ctrl_WriteRegister(GainParamPtr->I2cSlaveAddr, 0x008D, regDataHigh);

	return (MXL_STATUS)status;
}
#endif

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_ConfigDevice 
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : The general device configuration shall be handled 
--|                 through this API
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_ConfigDevice(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
	MXL_STATUS status = MXL_TRUE;

	//MxL_DLL_DEBUG0("MxLWare_API_ConfigDevice - %d\n", CmdType);

	switch (CmdType)
	{
		case MXL_DEV_SOFT_RESET_CFG:
			status = MxL_ConfigDevReset((PMXL_RESET_CFG_T)ParamPtr);
			break;

		case MXL_DEV_XTAL_SETTINGS_CFG:
			status = MxL_ConfigDevXtalSettings((PMXL_XTAL_CFG_T)ParamPtr);
			break;

		case MXL_DEV_POWER_MODE_CFG:
			status = MxL_ConfigDevPowerSavingMode((PMXL_PWR_MODE_CFG_T)ParamPtr);
			break;

		case MXL_DEV_OVERWRITE_DEFAULT_CFG:
			status = MxL_ConfigDevOverwriteDefault((PMXL_OVERWRITE_DEFAULT_CFG_T)ParamPtr);
			break;

		default:
			status = MXL_FALSE;
			break;
	}

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_GetDeviceStatus 
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : The general device inquiries shall be handled 
--|                 through this API
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_GetDeviceStatus(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
	MXL_STATUS status = MXL_TRUE;

	//MxL_DLL_DEBUG0("MxLWare_API_GetDeviceStatus - %d\n", CmdType);

	switch (CmdType)
	{
		case MXL_DEV_ID_VERSION_REQ:
			status = MxL_GetDeviceInfo((PMXL_DEV_INFO_T)ParamPtr);
			break;

		default:
			status = MXL_FALSE;
			break;
	}

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_ConfigDemod 
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : The demod block specific configuration shall be handled 
--|                 through this API
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_ConfigDemod(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
	MXL_STATUS status = MXL_TRUE;

	switch (CmdType)
	{
		case MXL_DEMOD_SYMBOL_RATE_CFG:
			status = MxL_ConfigDemodSymbolRate((PMXL_SYMBOL_RATE_T)ParamPtr);
			break;

		case MXL_DEMOD_MPEG_OUT_CFG:
			status = MxL_ConfigDemodMpegOutIface((PMXL_MPEG_OUT_CFG_T)ParamPtr);
			break;

		case MXL_DEMOD_ANNEX_QAM_TYPE_CFG:
			status = MxL_ConfigDemodAnnexQamType((PMXL_ANNEX_CFG_T)ParamPtr);
			break;

		case MXL_DEMOD_MISC_SETTINGS_CFG:
			break;

		case MXL_DEMOD_INTR_MASK_CFG:
			status = MxL_ConfigDemodInterrupt((PMXL_INTR_CFG_T)ParamPtr);
			break;

		case MXL_DEMOD_INTR_CLEAR_CFG:
			status = MxL_ClearDemodInterrupt((PMXL_INTR_CLEAR_T)ParamPtr);
			break;

		case MXL_DEMOD_RESET_STAT_COUNTER_CFG:
			status = MxL_ResetDemodStatCounters((PMXL_RESET_COUNTER_T)ParamPtr);
			break;

		case MXL_DEMOD_ADC_IQ_FLIP_CFG:
			status = MxL_ConfigDemodAdcIqFlip((PMXL_ADCIQFLIP_CFG_T)ParamPtr);
			break;
#ifdef __MXL241_DEBUG__
		case MXL_DEMOD_DBG_AGC_SELECT_CFG:
			status = MxL_ConfigDemodDbgAgcSelect((PMXL_AGC_SELECT_CFG_T)ParamPtr);
			break;

		case MXL_DEMOD_DBG_AGC_SETPOINT_CFG:
			status = MxL_ConfigDemodDbgAgcSetPoint((PMXL_DEMOD_AGC_SETPOINT_CFG_T)ParamPtr);
			break;
#endif      
		case MXL_DEMOD_QAM_BURST_FREEZE_CFG:
			status = MxL_ConfigDemodQamBurstFreeze((PMXL_QAM_BURST_FREEZE_T)ParamPtr);
			break;

		case MXL_DEMOD_INVERT_CARRIER_OFFSET_CFG:
			status = MxL_ConfigDemodInvertCarrierOffset((PMXL_INVERT_CARR_OFFSET_T)ParamPtr);
			break;

		default:
			status = MXL_FALSE;
			break;
	}

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_GetDemodStatus 
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : The demod specific inquiries shall be handled 
--|                 through this API
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_GetDemodStatus(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
	MXL_STATUS status = MXL_TRUE;

	switch (CmdType)
	{
		case MXL_DEMOD_INTR_STATUS_REQ:
		status = MxL_GetDemodInterruptStatus((PMXL_INTR_STATUS_T)ParamPtr);
		break;

		case MXL_DEMOD_FEC_LOCK_REQ:
		case MXL_DEMOD_MPEG_LOCK_REQ:
		case MXL_DEMOD_QAM_LOCK_REQ:
		status = MxL_GetDemodLockStatus(CmdType,  (PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
		break;

		case MXL_DEMOD_CONFIRMED_LOCK_STATUS_REQ:
			status = MxL_GetDemodConfirmedLockStatus((PMXL_DEMOD_CONFIRMED_LOCK_STATUS_T)ParamPtr);
			break;
			
		case MXL_DEMOD_SNR_REQ:
			status = MxL_GetDemodSnr((PMXL_DEMOD_SNR_INFO_T)ParamPtr);
			break;

		case MXL_DEMOD_BER_UNCODED_BER_CER_REQ:
			status = MxL_GetDemodBer((PMXL_DEMOD_BER_INFO_T)ParamPtr);
			break;

		case MXL_DEMOD_STAT_COUNTERS_REQ:
			status = MxL_GetDemodStatisticCounters((PMXL_DEMOD_STAT_COUNT_T)ParamPtr);
			break;

		case MXL_DEMOD_ANNEX_QAM_TYPE_REQ:
			status = MxL_GetDemodAnnexQamType((PMXL_DEMOD_ANNEXQAM_INFO_T)ParamPtr);
			break;

		case MXL_DEMOD_CARRIER_OFFSET_REQ:
			status = MxL_GetDemodCarrierOffset((PMXL_DEMOD_CARRIEROFFSET_INFO_T)ParamPtr);
			break;

		case MXL_DEMOD_INTERLEAVER_DEPTH_REQ:
			status = MxL_GetDemodInterleaverDepth((PMXL_INTERDEPTH_INFO_T)ParamPtr);
			break;

		case MXL_DEMOD_TIMING_OFFSET_REQ:
			status = MxL_GetDemodTimingOffset((PMXL_DEMOD_TIMINGOFFSET_INFO_T)ParamPtr);
			break;
			
		case MXL_DEMOD_EQUALIZER_FILTER_REQ:
			status = MxL_GetDemodEqualizerFilter((PMXL_DEMOD_EQUALFILTER_INFO_T)ParamPtr);
			break;

#ifdef __MXL241_DEBUG__
		case MXL_DEMOD_DBG_CURRENT_BW_REQ:
			status = MxL_GetDemodDbgCurrentBW((PMXL_BW_INFO_T)ParamPtr);
			break;
#endif

		case MXL_DEMOD_RETUNE_INDICATOR_REQ:
			status = MxL_GetDemodReTuneIndicator((PMXL_RETUNE_IND_T)ParamPtr);
			break;

		default:
			status = MXL_FALSE;
			break;
	}

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_ConfigTuner 
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : The tuner block specific configuration shall be handled 
--|                 through this API
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_ConfigTuner(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
	MXL_STATUS status = MXL_TRUE;

	//MxL_DLL_DEBUG0("MxLWare_API_ConfigTuner : CmdType =%d\n", CmdType);

	switch (CmdType)
	{
		case MXL_TUNER_AGC_SETTINGS_CFG:
			status = MxL_ConfigTunerFreezeAgc((PMXL_AGC_T)ParamPtr);
			break;

		case MXL_TUNER_TOP_MASTER_CFG:
			status = MxL_ConfigTunerTopMaster((PMXL_TOP_MASTER_CFG_T)ParamPtr);
			break;

		case MXL_TUNER_CHAN_TUNE_CFG:
			status = MxL_ConfigTunerChanTune((PMXL_RF_TUNE_CFG_T)ParamPtr);
			break;
			
		case MXL_TUNER_EXTENDED_CHAN_TUNE_CFG:
			status = (MXL_STATUS) MxL_ConfigTunerExtendedRangeChanTune((PMXL_RF_EXTENDED_TUNE_CFG_T)ParamPtr);
			break;

		case MXL_TUNER_CHAN_DEPENDENT_TUNE_CFG:
			status = MxL_ConfigTunerChanDependent((PMXL_CHAN_DEPENDENT_T)ParamPtr);
			break;

		case MXL_TUNER_AGC_LOCK_SPEED_CFG:
			status = MxL_ConfigTunerAgcLockSpeed((PMXL_AGC_CTRL_SPEED_T)ParamPtr);
			break;

		case MXL_TUNER_RSSI_THRESHOLDS_CFG:
			status = MxL_ConfigTunerRssiThresholds((PMXL_RF_TUNER_RSSI_THRESHOLDS_T) ParamPtr);
			break;

#ifdef __MXL241_DEBUG__    
		case MXL_TUNER_DBG_IQOUT_CFG:
			status = MxL_ConfigTunerDbgIqOut((PMXL_IQ_OUT_CFG_T)ParamPtr);
			break;

		case MXL_TUNER_DBG_AGC_SETPOINT_CFG:
			status = MxL_ConfigTunerDbgAgcSetPoint((PMXL_TUNER_AGC_SETPOINT_CFG_T)ParamPtr);
			break;

		/* Gain Freeze Program test*/
		case MXL_TUNER_DBG_GAIN_FREEZE_CFG:
			status = MxL_ConfigTunerDbgGainFreezeTest((PMXL_GAIN_FREEZE_T)ParamPtr);
			break;
#endif      

		default:
			status = MXL_FALSE;
			break;
	}

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_GetTunerStatus 
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 6/19/2009
--|
--| DESCRIPTION   : The tuner specific inquiries shall be handled 
--|                 through this API
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_GetTunerStatus(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
	MXL_STATUS status;

	//MxL_DLL_DEBUG0("MxLWare_API_GetTunerStatus : CmdType =%d\n", CmdType);

	switch (CmdType)
	{
		case MXL_TUNER_DBG_LOCK_STATUS_REQ:
		case MXL_TUNER_LOCK_STATUS_REQ:
			status = MxL_GetTunerLockStatus((PMXL_TUNER_LOCK_STATUS_T)ParamPtr);
			break;

		case MXL_TUNER_RF_RX_PWR_REQ:
			status = MxL_GetTunerRxPower((PMXL_TUNER_RX_PWR_T)ParamPtr);
			break;

		case MXL_TUNER_AGC_SETTINGS_REQ:
			status = MxL_GetTunerAgcSettings((PMXL_AGC_INFO_T)ParamPtr);
			break;

#ifdef __MXL241_DEBUG__
		case MXL_TUNER_DBG_SENSOR_TEMP_REQ:
			status = MxL_GetTunerDbgSensorTemp((PMXL_TEMP_SENSOR_T)ParamPtr);
			break;
#endif      
		default:
			status = MXL_FALSE;
			break;
	}

	return status;
}

//#endif 
