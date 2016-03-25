/*******************************************************************************
 *
 * FILE NAME          : MxL601_OEM_Drv.h
 * 
 * AUTHOR             : Dong Liu 
 *
 * DATE CREATED       : 11/23/2011
 *
 * DESCRIPTION        : Header file for MxL601_OEM_Drv.c
 *
 *******************************************************************************
 *                Copyright (c) 2010, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __TUN_MXL603_H__
#define __TUN_MXL603_H__
/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/

#include "MaxLinearDataTypes.h"
//#include "MxL_Debug.h"

/******************************************************************************
    Macros
******************************************************************************/

/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/
//typedef struct
//{
//	MxL5007_I2CAddr		I2C_Addr;
//	MxL5007_Mode		Mode;
//	SINT32				IF_Diff_Out_Level;
//	MxL5007_Xtal_Freq	Xtal_Freq;
//	MxL5007_IF_Freq	    IF_Freq;
//	MxL5007_IF_Spectrum IF_Spectrum;
//	MxL5007_ClkOut		ClkOut_Setting;
//    MxL5007_ClkOut_Amp	ClkOut_Amp;
//	MxL5007_BW_MHz		BW_MHz;
//	MxL5007_LoopThru	LoopThru;
//	UINT32				RF_Freq_Hz;
//	UINT32              tuner_id;
//} MXL603_PARAMETER;

/******************************************************************************
    Global Variable Declarations
******************************************************************************/
extern void * MxL603_OEM_DataPtr[];
/*****************************************************************************
    Prototypes
******************************************************************************/

INT32 tun_mxl603_init_DVBC(UINT32 *tuner_id , struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_mxl603_control_DVBC(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 agc_time_const, UINT8 _i2c_cmd);
INT32 tun_mxl603_control_DVBC_X(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 tun_mxl603_status(UINT32 tuner_id, UINT8 *lock);


INT32 tun_mxl603_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_mxl603_init_ISDBT(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)  ;
INT32 tun_mxl603_init_ISDBT(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)  ;
INT32 tun_mxl603_init_CDT_MN88472(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config) ;
INT32 tun_mxl603_control(UINT32 tuner_id, UINT32 freq, UINT32 bandwidth,UINT8 AGC_Time_Const,
                        UINT8 *data,UINT8 _i2c_cmd)	;



MXL_STATUS MxLWare603_OEM_WriteRegister(UINT32 tuner_id, UINT8 regAddr, UINT8 regData);
MXL_STATUS MxLWare603_OEM_ReadRegister(UINT32 tuner_id, UINT8 regAddr, UINT8 *regDataPtr);
void MxLWare603_OEM_Sleep(UINT16 delayTimeInMs);  

#endif /* __TUN_MXL603_H__*/




