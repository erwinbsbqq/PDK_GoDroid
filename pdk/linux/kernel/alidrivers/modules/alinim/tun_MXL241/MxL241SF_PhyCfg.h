/*******************************************************************************
 *
 * FILE NAME          : MxL241SF_PhyCfg.h
 * 
 * AUTHOR             : Brenndon Lee
 * DATE CREATED       : 5/18/2009
 *
 * DESCRIPTION        : This file contains MxL241SF common control register 
 *                      	     definitions
 * 				     Coeus modify to Ali linux platform 2012/10/28
 *
 *******************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __MXL241SF_PHY_CFG_H__
#define __MXL241SF_PHY_CFG_H__

/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/

#include "ali_nim_mxl241.h"
/******************************************************************************
    Macros
******************************************************************************/
#define CW_COUNT          0
#define CW_CORR_COUNT     1
#define CW_ERR_COUNT      2
#define CORR_BITS         3
#define AIC_RESET_REG                       0xFFFF
#define VERSION_REG                         0x0017
#define SLEEP_MODE_CTRL_REG                 0x0002
#define ENABLE_SLEEP      0x0001
#define DISABLE_SLEEP     0x0000
#define XTAL_CLK_OUT_CTRL_REG               0x0000
#define ENABLE_CLK_OUT   0x10
#define DIG_XTAL_FREQ_CTRL_REG              0x0003
#define ENABLE_XTAL   0x10
#define DIG_XTAL_CAP_CTRL_REG               0x0004
#define DIG_XTAL_BIAS_CTRL_REG              0x00BC
#define LOOP_THROUGH_CTRL_REG               0x0006
#define ENABLE_LOOP_THRU   0x0020
#define TOP_MASTER_CONTROL_REG              0x0001
#define ENABLE_TOP_MASTER   0x0001
#define DISABLE_TOP_MASTER  0x0000
#define DIG_RSSI            0x0084
#define SYMBOL_RATE_RESAMP_BANK_REG         0x0007
#define SYMBOL_RATE_RESAMP_RATE_LO_REG      0x0009
#define SYMBOL_RATE_RESAMP_RATE_MID_LO_REG  0x000A
#define SYMBOL_RATE_RESAMP_RATE_MID_HI_REG  0x000B
#define SYMBOL_RATE_RESAMP_RATE_HI_REG      0x000C
#define SYM_NUM_B_BANK         2
#define BYPASS_INTERPOLATOR    0x0010
#define DIG_HALT_GAIN_CTRL_REG              0x0015
#define FREEZE_AGC_GAIN_WORD    0x000C
#define DIG_AGC_SETPT_CTRL_LO_REG           0x002D
#define DIG_AGC_SETPT_CTRL_HI_REG           0x002E
#define DIG_SAGC_NMAXAVG                    0x002E
#define DIG_SAGC_NMAXHOLDCLK                0x002F
#define STAMP_REG                           0x80A4
#define MCNSSD_SEL_REG                      0x80AD 
#define MCNSSD_REG                          0x80AE 
#define NCBL_REG                            0x80AA
#define NCBH_REG                            0x80A9
#define FRCNT_REG                           0x80A7
#define MEF_REG                             0x80A8
#define ANNEX_A_TYPE  0x0400
#define QAM_TYPE      0x0007
#define ENABLE_AUTO_DETECT_QAM_TYPE 0x8000
#define ENABLE_AUTO_DETECT_MODE     0x4000
#define QAM_LOCK_STATUS_REG                 0x800E  
#define FEC_MPEG_LOCK_REG                   0x80A3
#define DIG_ADCIQ_FLIP_REG                  0x0027
#define QAM_ANNEX_TYPE_REG                  0x8001
#define QAM_TYPE_AUTO_DETECT_REG            0x8005
#define GODARD_KP_STEP3                     0x8048
#define MPEG_LOCK_TH                        0x80A2
#define SNR_MSE_AVG_COEF_REG                0x8036
#define SNR_EXT_SPACE_ADDR_REG              0x803E
#define SNR_EXT_SPACE_DATA_REG              0x803F
#define MSE_AVG_COEF             0xD800
#define MPEGOUT_PAR_CLK_CTRL_REG            0x000D
#define MPEGOUT_EXT_MPEGCLK_INV_REG         0x000E
#define MPEGOUT_SER_CTRL_REG                0x80F2
#define FIFO_READ_UNLIMIT        0x0080
#define ENABLE_MPEG_ERROR_IND    0x0100
#define EQ_SPUR_BYPASS_REG                  0x802F
#define CARRIER_OFFSET_REG                  0x803E
#define RF_CHAN_BW_REG                      0x0010
#define RF_LOW_REG                          0x0011
#define RF_HIGH_REG                         0x0012
#define RF_SEQUENCER_REG                    0x0013
#define STOP_TUNE    0x0000
#define START_TUNE   0x0001
#define RF_LOCK_STATUS_REG                  0x0016
#define REF_SYN_RDY      0x0003
#define RF_SYN_RDY       0x000C
#define TUNER_LOCKED     0x000F
#define AGC_LOCK_STATUS_REG                 0x00F8
#define AGC_LOCKED       0x0004
#define TUNER_DONE       0x0008
#define RF_PIN_RB_EN_REG                    0x0014
#define ENABLE_RFPIN_RB  0x0001
#define DIG_RF_PIN_RB_LO_REG                0x0018
#define DIG_RF_PIN_RB_HI_REG                0x0019
#define INTERLEAVER_DEPTH_REG               0x80A3
#define CUSTOM_COEF_EN_REG                  0x0008
#define EQU_FREQ_SWEEP_LIMIT_REG            0x8021
#define EQUALIZER_FILTER_REG                0x803E
#define EQUALIZER_FILTER_DATA_RB_REG        0x803F
#define CARRIER_OFFSET_RB_REG               0x803F
#define FREEZE_AGC_CFG_REG                  0x0015
#define INTR_MASK_REG                       0x80F3
#define INTR_STATUS_REG                     0x80F4
#define INTR_CLEAR_REG                      0x80F4
#define ENABLE_DEMOD_INT  0x8000
#define DEMOD_RESTART_REG                   0x80FF
#define RESAMP_BANK_REG   0x8040
#define RATE_RESAMP_RATE_MID_LO_REG      0x8043
#define RATE_RESAMP_RATE_MID_HI_REG      0x8042
#define GODARD_ACC_REG 0x804B
#define DIG_AGC_SELECT    0x002B
#define DIG_TESTIF_OBV_MODE                 0x00BE
#define ENABLE_TEMP_SENSOR_REG              0x0014
#define TEMP_SENSOR_RB_REG                  0x0019
#define DEMOD_AGC_SET_POINT_REG             0x804D
#define PHY_EQUALIZER_FFE_FILTER_REGISTER   0x8026
#define PHY_EQUALIZER_DFE_FILTER_REGISTER   0x8027
#define PHY_EQUALIZER_LLP_CONTROL_1_DEBUG_MSE_REGISTER 0x8036
#define OVERWRITE_DEFAULT_REG               0x0072
#define SYMBOL_RATE_RATE_LO_REG_RB          0x0009
#define SYMBOL_RATE_RATE_MID_LO_REG_RB      0x000A
#define SYMBOL_RATE_RATE_MID_HI_REG_RB      0x000B
#define SYMBOL_RATE_RATE_HI_REG_RB          0x000C
#define AGC_CTRL_SPEED_REG     0x0031
#define FAST_AGC_CTRL_LOCK     0x0010
#define NORMAL_AGC_CTRL_LOCK   0x0002
#define XTAL_EN_VCO_BIASBST    0x00BD
#define DIG_TEMP_RB            0x0019
#define TEMP_THRESHOLD         0x8
#define DN_LNA_BIAS_CTRL_REG   0x0099
#define QAM_BURST_FREEZE       0x8002
#define DIG_PIN_2XDRV_REG      0x000F
#define WIDER_FREQ_SWEEP_RANGE   0xD586
#define DEFAULT_FREQ_SWEEP_RANGE 0xD566
#define RF_SPARE_REG           0x00B8
#define RF_CHP_GAIN_LUT_BYP_REG   0x00AE
#define RETUNE_INDICATOR_THRESHOLD 0x9
/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/
typedef struct
{
	UINT16 regAddr;
	UINT16 mask;
	UINT16 data;
} REG_CTRL_INFO_T, *PREG_CTRL_INFO_T;

typedef struct
{
	UINT8 interDepthI;
	UINT8 interDepthJ;
} INTERLEAVER_LOOKUP_INFO_T, *PINTERLEAVER_LOOKUP_INFO_T;

typedef struct
{
	UINT16 lutY;
} LUTY_LOOKUP_INFO_T, *PLUTY_LOOKUP_INFO_T;

/******************************************************************************
    Global Variable Declarations
******************************************************************************/

extern REG_CTRL_INFO_T MxL_OobAciMfCoef[];     
extern REG_CTRL_INFO_T MxL_OobAciMfCoef_0_772MHz[];     
extern REG_CTRL_INFO_T MxL_OobAciMfCoef_1_024MHz[];     
extern REG_CTRL_INFO_T MxL_OobAciMfCoef_1_544MHz[]; 
extern REG_CTRL_INFO_T MxL_OverwriteDefaults[];
extern REG_CTRL_INFO_T MxL_EqualizerSpeedUp[];
extern INTERLEAVER_LOOKUP_INFO_T MxL_InterDepthLookUpTable[];
extern LUTY_LOOKUP_INFO_T MxL_LutYLookUpTable[];
/******************************************************************************
    Prototypes
******************************************************************************/
MXL_STATUS Ctrl_ProgramRegisters(UINT8 I2cSlaveAddr, PREG_CTRL_INFO_T ctrlRegInfoPtr);
// OEM specific APIs
MXL_STATUS Ctrl_ReadRegister(UINT8 I2cSlaveAddr, UINT16 RegAddr, UINT16 *DataPtr);
MXL_STATUS Ctrl_WriteRegister(UINT8 I2cSlaveAddr, UINT16 RegAddr, UINT16 RegData);

#endif /* __MXL241SF_PHY_CFG_H__*/
