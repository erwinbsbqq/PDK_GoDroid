/*******************************************************************************
 *
 * FILE NAME          : MxL241SF_PhyCtrlApi.h
 * 
 * AUTHOR             : Brenndon Lee
 * DATE CREATED       : 6/24/2009
 *
 * DESCRIPTION        : This file is header file for MxL241SF_PhyCtrlApi.cpp
 *
 *******************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __MXL241SF_PHY_CTRL_API_H__
#define __MXL241SF_PHY_CTRL_API_H__

#include "ali_nim_mxl241.h"


/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/




//#include "MxlBasicType.h"
/******************************************************************************
    Macros
******************************************************************************/
#define n_MXL_FPGA_

#define MAX_241SF_DEVICES   8

/* The following macro enables integer calculation */
/* If you are using floating point calculation, please comment it out */
#define __MXL_INTEGER_CALC_STATISTICS__

/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/
/* Command Types  */
typedef enum
{
	MXL_DEV_SOFT_RESET_CFG = 0,
	MXL_DEV_XTAL_SETTINGS_CFG,
	MXL_DEV_POWER_MODE_CFG,
	MXL_DEV_OVERWRITE_DEFAULT_CFG,

	MXL_DEV_ID_VERSION_REQ,

	MXL_DEMOD_SYMBOL_RATE_CFG,
	MXL_DEMOD_MPEG_OUT_CFG,
	MXL_DEMOD_ANNEX_QAM_TYPE_CFG,
	MXL_DEMOD_MISC_SETTINGS_CFG,
	MXL_DEMOD_INTR_MASK_CFG,
	MXL_DEMOD_INTR_CLEAR_CFG,
	MXL_DEMOD_RESET_STAT_COUNTER_CFG, 
	MXL_DEMOD_ADC_IQ_FLIP_CFG,
	MXL_DEMOD_EQUALIZER_FILTER_CFG, 
	MXL_DEMOD_DBG_AGC_SELECT_CFG,   
	MXL_DEMOD_DBG_AGC_SETPOINT_CFG, 
	MXL_DEMOD_QAM_BURST_FREEZE_CFG,
	MXL_DEMOD_INVERT_CARRIER_OFFSET_CFG,
	MXL_DEMOD_UNLOCK_MONITOR_THREAD_CFG,

	MXL_DEMOD_INTR_STATUS_REQ,
	MXL_DEMOD_FEC_LOCK_REQ,
	MXL_DEMOD_MPEG_LOCK_REQ,
	MXL_DEMOD_QAM_LOCK_REQ,
	MXL_DEMOD_SNR_REQ,
	MXL_DEMOD_BER_UNCODED_BER_CER_REQ,
	MXL_DEMOD_ANNEX_QAM_TYPE_REQ,
	MXL_DEMOD_CARRIER_OFFSET_REQ,
	MXL_DEMOD_INTERLEAVER_DEPTH_REQ,
	MXL_DEMOD_EQUALIZER_FILTER_REQ,
	MXL_DEMOD_STAT_COUNTERS_REQ,
	MXL_DEMOD_TIMING_OFFSET_REQ,
	MXL_DEMOD_RETUNE_INDICATOR_REQ,
	MXL_DEMOD_CONFIRMED_LOCK_STATUS_REQ,
	MXL_DEMOD_DBG_CURRENT_BW_REQ,   

	MXL_TUNER_RSSI_THRESHOLDS_CFG,
	MXL_TUNER_AGC_SETTINGS_CFG,
	MXL_TUNER_TOP_MASTER_CFG,
	MXL_TUNER_CHAN_TUNE_CFG,
	MXL_TUNER_EXTENDED_CHAN_TUNE_CFG,
	MXL_TUNER_CHAN_DEPENDENT_TUNE_CFG,
	MXL_TUNER_AGC_LOCK_SPEED_CFG,

	MXL_TUNER_DBG_IQOUT_CFG,        
	MXL_TUNER_DBG_AGC_SETPOINT_CFG, 
	MXL_TUNER_DBG_GAIN_FREEZE_CFG, 

	MXL_TUNER_LOCK_STATUS_REQ, 
	MXL_TUNER_DBG_LOCK_STATUS_REQ,  
	MXL_TUNER_RF_RX_PWR_REQ,
	MXL_TUNER_AGC_SETTINGS_REQ,
	MXL_TUNER_DBG_SENSOR_TEMP_REQ,  

	MAX_NUM_CMD
} MXL_CMD_TYPE_E;

/* Ds equalizer parameters */
typedef enum 
{
	FFE_MAIN_TAP_LOCATION_1 = 5,
	FFE_MAIN_TAP_LOCATION_2,
	FFE_MAIN_TAP_LOCATION_3,
	FFE_MAIN_TAP_LOCATION_4
} FFE_LEADING_TAP_LOCATION_E;

typedef struct
{
	UINT8 I2cSlaveAddr;       /* IN, I2C Address of MxL Device */
} MXL_RESET_CFG_T, *PMXL_RESET_CFG_T;
/* MXL_DEV_OVERWRITE_DEFAULT_CFG  */
typedef struct
{
	UINT8 I2cSlaveAddr;       /* IN, I2C Address of MxL Device */
} MXL_OVERWRITE_DEFAULT_CFG_T, *PMXL_OVERWRITE_DEFAULT_CFG_T;

/* MXL_DEV_ID_VERSION_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;      /* IN, I2C Address of MxL Device */
	UINT8 DevId;             /* OUT, Device Id of MxL Device */
	UINT8 DevVer;            /* OUT, Device Version of MxL Device */
	UINT8 MxLWareVer[4];     /* OUT, MxLWare Version */
} MXL_DEV_INFO_T, *PMXL_DEV_INFO_T;

/* MXL_DEV_XTAL_SETTINGS_CFG */
typedef enum
{
	XTAL_16MHz = 0,
	XTAL_20MHz,
	XTAL_20_25MHz,
	XTAL_20_48MHz,
	XTAL_24MHz,
	XTAL_25MHz,
	XTAL_25_14MHz,
	XTAL_27MHz,
	XTAL_28_8MHz,
	XTAL_32MHz,
	XTAL_40MHz,
	XTAL_44MHz,
	XTAL_48MHz,
	XTAL_49_3811MHz
} MXL_XTAL_FREQ_E;

typedef enum
{
	I124uA = 0,
	I239uA,
	I351uA,
	I462uA,
	I572uA,
	I680uA,
	I788uA,
	I894uA
} MXL_XTAL_BIAS_E;

typedef struct
{
	UINT8 I2cSlaveAddr;                /* IN, I2C Address of MxL Device */
	MXL_BOOL XtalEnable;               /* IN,  */
	MXL_XTAL_FREQ_E DigXtalFreq;       /* IN,  */
	MXL_XTAL_BIAS_E XtalBiasCurrent;   /* IN,  */
	UINT8 XtalCap;                     /* IN,  */
	MXL_BOOL XtalClkOutEnable;         /* IN,  */
	UINT8 XtalClkOutGain;              /* IN,  */
	MXL_BOOL  LoopThruEnable;          /* IN,  */
} MXL_XTAL_CFG_T, *PMXL_XTAL_CFG_T;

/* MXL_TUNER_AGC_SETTINGS_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;            /* IN, I2C Address of MxL Device */
	MXL_BOOL FreezeAgcGainWord;    /* IN */
} MXL_AGC_T, *PMXL_AGC_T;

/* MXL_TUNER_AGC_SETTINGS_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;            /* IN, I2C Address of MxL Device */
	MXL_BOOL FreezeAgcGainWord;    /* OUT */
	UINT16 AgcSetPoint;            /* OUT */
} MXL_AGC_INFO_T, *PMXL_AGC_INFO_T;

/* MXL_TUNER_TOP_MASTER_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;            /* IN, I2C Address of MxL Device */
	MXL_BOOL TopMasterEnable;      /* IN */
} MXL_TOP_MASTER_CFG_T, *PMXL_TOP_MASTER_CFG_T;

/* MXL_DEMOD_SYMBOL_RATE_CFG */
typedef enum
{
	SYM_TYPE_J83A = 0,
	SYM_TYPE_J83B,
	SYM_TYPE_USER_DEFINED_J83A,
	SYM_TYPE_USER_DEFINED_J83B,
	SYM_TYPE_OOB
} MXL_SYM_TYPE_E;

typedef enum
{
	SYM_RATE_0_772MHz = 0, 
	SYM_RATE_1_024MHz,
	SYM_RATE_1_544MHz
} MXL_OOB_SYM_RATE_E;

#ifdef __MXL_INTEGER_CALC_STATISTICS__
typedef struct
{
	UINT8 I2cSlaveAddr;               /* IN, I2C Address of MxL Device */
	MXL_SYM_TYPE_E SymbolType;        /* IN */
	UINT16 SymbolRate;	            /* IN, For user defined A/B symbol rate kSps*/
	UINT16 SymbolRate256;             /* IN, For user defined B symbol rate kSps*/
	MXL_OOB_SYM_RATE_E OobSymbolRate; /* IN, For OOB Symbol rate */
} MXL_SYMBOL_RATE_T, *PMXL_SYMBOL_RATE_T;
#else
typedef struct
{
	UINT8 I2cSlaveAddr;               /* IN, I2C Address of MxL Device */
	MXL_SYM_TYPE_E SymbolType;        /* IN */
	REAL32 SymbolRate;                /* IN, For user defined A/B symbol rate */
	REAL32 SymbolRate256;             /* IN, For user defined B symbol rate */
	MXL_OOB_SYM_RATE_E OobSymbolRate; /* IN, For OOB Symbol rate */
} MXL_SYMBOL_RATE_T, *PMXL_SYMBOL_RATE_T;
#endif

/* MXL_DEMOD_MPEG_OUT_CFG */
typedef enum
{
	MPEG_DATA_SERIAL  = 0,
	MPEG_DATA_PARALLEL,

	MPEG_SERIAL_LSB_1ST = 0,
	MPEG_SERIAL_MSB_1ST,

	MPEG_SYNC_WIDTH_BIT = 0,
	MPEG_SYNC_WIDTH_BYTE
} MPEG_DATA_FMT_E;

typedef enum
{
	MPEG_ACTIVE_LOW = 0,
	MPEG_ACTIVE_HIGH,

	MPEG_CLK_POSITIVE  = 0,
	MPEG_CLK_NEGATIVE,

	MPEG_CLK_IN_PHASE = 0,
	MPEG_CLK_INVERTED,

	MPEG_CLK_EXTERNAL = 0, 
	MPEG_CLK_INTERNAL,
} MPEG_CLK_FMT_E;

typedef enum
{
	MPEG_CLK_57MHz  = 0,
	MPEG_CLK_38MHz,
	MPEG_CLK_28_5MHz,
	MPEG_CLK_19MHz,
	MPEG_CLK_9_5MHz,
	MPEG_CLK_7_125MHz,
	MPEG_CLK_4_75MHz,
	MPEG_CLK_3_5625MHz,
} MPEG_CLK_RATE_E;

typedef struct
{
	UINT8           I2cSlaveAddr;        /* IN, I2C Address of MxL Device   */   
	MPEG_DATA_FMT_E SerialOrPar;         /* IN, Parallel or Serial */ 
	MPEG_DATA_FMT_E LsbOrMsbFirst;       /* IN  */
	MPEG_DATA_FMT_E MpegSyncPulseWidth;  /* IN  */
	MPEG_CLK_FMT_E  MpegValidPol;        /* IN  */ 
	MPEG_CLK_FMT_E  MpegClkPol;          /* IN  */
	MPEG_CLK_FMT_E  MpegSyncPol;         /* IN  */
	MPEG_CLK_FMT_E  MpegClkSource;       /* IN, */
#ifdef  _MXL_FPGA_ 
	MPEG_CLK_FMT_E  ExtMpegClkPhase;     /* IN, Internal  */
#endif
	MPEG_CLK_RATE_E MpegClkFreq;         /* IN  */
	MXL_BOOL        MpegErrorIndication; /* IN  */
} MXL_MPEG_OUT_CFG_T, *PMXL_MPEG_OUT_CFG_T;

/* MXL_DEMOD_ANNEX_QAM_TYPE_CFG */
typedef enum
{
	ANNEX_B = 0,
	ANNEX_A
} MXL_ANNEX_TYPE_E;

typedef enum
{
	MXL_QAM16 = 0,
	MXL_QAM64,
	MXL_QAM256,
	MXL_QAM1024,
	MXL_QAM32,
	MXL_QAM128,
	MXL_QPSK
} MXL_QAM_TYPE_E;

typedef struct
{
	UINT8 I2cSlaveAddr;           /* IN, I2C Address of MxL Device */
	MXL_ANNEX_TYPE_E AnnexType;   /* IN, Annex-B or A */
	MXL_BOOL AutoDetectQamType;   /* IN, Enable or disable of Auto QAM detect */
	MXL_BOOL AutoDetectMode;      /* IN, Enable or disable of Auto Annex mode detet */
	MXL_QAM_TYPE_E QamType;       /* IN, manual mode, set QAM type directly */
} MXL_ANNEX_CFG_T, *PMXL_ANNEX_CFG_T;

/* MXL_DEMOD_MISC_SETTINGS_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address o	f MxL Device */
	MXL_BOOL EnergyFreezeEnable;
	UINT8 EquFreqSweepLimit;
} MXL_DEMOD_MISC_T, *PMXL_DEMOD_MISC_T;

/* MXL_DEV_POWER_MODE_CFG */
typedef enum
{
	STANDBY_ON = 0,
	SLEEP_ON,
	STANDBY_OFF,
	SLEEP_OFF,
} MXL_PWR_MODE_E; 

typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	MXL_PWR_MODE_E PowerMode;   /* IN  */
} MXL_PWR_MODE_CFG_T, *PMXL_PWR_MODE_CFG_T;

#ifdef __MXL_INTEGER_CALC_STATISTICS__
/* MXL_TUNER_CHAN_TUNE_INT_CFG */
typedef struct
{
	UINT8  I2cSlaveAddr;      /* IN, I2C Address of MxL Device */
	UINT8  BandWidth;         /* IN, Channel Bandwidth in MHz */
	UINT32 Frequency;         /* IN, Radio Frequency in Hz */
} MXL_RF_TUNE_CFG_T, *PMXL_RF_TUNE_CFG_T;
#else
/* MXL_TUNER_CHAN_TUNE_CFG */
typedef struct
{
	UINT8  I2cSlaveAddr;      /* IN, I2C Address of MxL Device */
	UINT8  BandWidth;         /* IN, Channel Bandwidth in MHz */
	REAL32 Frequency;         /* IN, Radio Frequency in Hz */
} MXL_RF_TUNE_CFG_T, *PMXL_RF_TUNE_CFG_T;
#endif

#define MAX_FREQ_OFFSET_CNT    3 
typedef struct
{
	UINT8  I2cSlaveAddr;
	UINT8  BandWidth;
	UINT32 Frequency;
	SINT32 FreqOffsetsArray[MAX_FREQ_OFFSET_CNT];
	UINT8  FreqOffsetsCnt;

	SINT32 InitialFreqOffset;  /* Returned value of initial freq offset. Use it for calculation final frequency offset */

	UINT16 qamLockTimeoutMs;
	UINT16 fecLockTimeoutMs;
	UINT16 mpegLockTimeoutMs;
	MXL_BOOL iqFlip;
} MXL_RF_EXTENDED_TUNE_CFG_T, *PMXL_RF_EXTENDED_TUNE_CFG_T;

typedef struct
{
	UINT8 I2cSlaveAddr;
	UINT8 rssiThresholdLo;
	UINT8 rssiThresholdHi;
} MXL_RF_TUNER_RSSI_THRESHOLDS_T, *PMXL_RF_TUNER_RSSI_THRESHOLDS_T;
/* MXL_DEMOD_INTR_MASK_CFG */
#define INTR_FEC_LOST_EN          0x0001
#define INTR_FEC_LOCK_EN          0x0002
#define INTR_MPEG_LOST_EN         0x0004
#define INTR_MPEG_LOCK_EN         0x0008
#define INTR_QAM_LOCK_GAINED_EN   0x0010
#define INTR_QAM_LOCK_LOST_EN     0x0020

typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	UINT16 IntrMask;            /* IN, */
} MXL_INTR_CFG_T, *PMXL_INTR_CFG_T;

/* MXL_DEMOD_INTR_CLEAR_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	UINT16 IntrStatus;          /* OUT, */
	UINT16 IntrMask;            /* OUT, */
} MXL_INTR_STATUS_T, *PMXL_INTR_STATUS_T;

/* MXL_DEMOD_INTR_CLEAR_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	UINT16 IntrClear;           /* IN, */
} MXL_INTR_CLEAR_T, *PMXL_INTR_CLEAR_T;

/* MXL_DEMOD_FEC_LOCK_REQ, MXL_DEMOD_MPEG_LOCK_REQ, MXL_DEMOD_QAM_LOCK_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	MXL_BOOL Status;            /* OUT */
} MXL_DEMOD_LOCK_STATUS_T, *PMXL_DEMOD_LOCK_STATUS_T;

/* MXL_DEMOD_CONFIRMED_LOCK_STATUS_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	UINT16   FagcGainThreshold; /* IN, Fagc gain threshold */
	MXL_BOOL QamLockStatus;     /* OUT, Demod Qam lock status, MXL_LOCKED=lock, MXL_UNLOCKED=unlock  */
	MXL_BOOL FecLockStatus;     /* OUT, Demod Fec lock status, MXL_LOCKED=lock, MXL_UNLOCKED=unlock  */
	MXL_BOOL MpegLockStatus;    /* OUT, Demod Mpeg lock status,MXL_LOCKED=lock, MXL_UNLOCKED=unlock  */
	MXL_STATUS FalseLockDetected; /* OUT, False lock was detected and demod is restarted */
} MXL_DEMOD_CONFIRMED_LOCK_STATUS_T, *PMXL_DEMOD_CONFIRMED_LOCK_STATUS_T;

#ifdef __MXL_INTEGER_CALC_STATISTICS__
/* MXL_DEMOD_SNR_INT_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	SINT32 SNR;                 /* OUT, */
} MXL_DEMOD_SNR_INFO_T, *PMXL_DEMOD_SNR_INFO_T;
#else
/* MXL_DEMOD_SNR_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	REAL32 SNR;                 /* OUT, */
	UINT32 MseSize;             /* OUT, */
} MXL_DEMOD_SNR_INFO_T, *PMXL_DEMOD_SNR_INFO_T;
#endif

#ifdef __MXL_INTEGER_CALC_STATISTICS__
/* MXL_DEMOD_BER_UNCODED_BER_CER_INT_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	UINT64 BER;                 /* OUT, */
	UINT64 UncodedBER;          /* OUT, */
	UINT64 CER;                 /* OUT, */
	/*add by allen for ATE*/
	UINT64 TotalByte;
	UINT64 CorrectedByte;
	UINT64 UncorrectedByte;  
} MXL_DEMOD_BER_INFO_T, *PMXL_DEMOD_BER_INFO_T;
#else
/* MXL_DEMOD_BER_UNCODED_BER_CER_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	REAL64 BER;                 /* OUT, */
	REAL64 UncodedBER;          /* OUT, */
	REAL64 CER;                 /* OUT, */
	/*add by allen for ATE*/
	UINT64 TotalByte;
	UINT64 CorrectedByte;
	UINT64 UncorrectedByte;
} MXL_DEMOD_BER_INFO_T, *PMXL_DEMOD_BER_INFO_T;
#endif

/* MXL_DEMOD_STAT_COUNTERS_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	UINT64 AccCwCorrCount;      /* OUT, Accumulated corrected code words */
	UINT64 AccCwErrCount;       /* OUT, Accumulated uncorrected code words */
	UINT64 AccCwReceived;       /* OUT, Accumulated total received code words */
	UINT64 AccCwUnerrCount;     /* OUT, Accumulated counter for code words received */
	UINT64 AccCorrBits;         /* OUT, Accumulated counter for corrected bits */
	UINT64 AccErrMpeg;          /* OUT, Accumulated counter for erred mpeg frames */
	UINT64 AccReceivedMpeg;     /* OUT, Accumulated counter for received mpeg frames */
} MXL_DEMOD_STAT_COUNT_T, *PMXL_DEMOD_STAT_COUNT_T;

/* MXL_DEMOD_RESET_STAT_COUNTER_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;       /* IN, I2C Address of MxL Device */
} MXL_RESET_COUNTER_T, *PMXL_RESET_COUNTER_T;

/* MXL_DEMOD_EQUALIZER_FILTER_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;          /* IN, I2C Address of MxL Device */
	MXL_BOOL EqualizerSetting;   /* IN, Enable or disable Equalizer Bypass */
} MXL_EQUALIZER_CFG_T, *PMXL_EQUALIZER_CFG_T;

/* MXL_DEMOD_ANNEX_QAM_TYPE_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	MXL_ANNEX_TYPE_E AnnexType; /* OUT */
	MXL_QAM_TYPE_E QamType;     /* OUT */
} MXL_DEMOD_ANNEXQAM_INFO_T, *PMXL_DEMOD_ANNEXQAM_INFO_T;

typedef enum
{
	SYM_RATE_ANNEX_A_6_89MHz = 0,
	SYM_RATE_ANNEX_B64_5_0569MHz,
	SYM_RATE_ANNEX_B256_5_3605MHz,
	SYM_RATE_OOB_0_772MHz, 
	SYM_RATE_OOB_1_024MHz,
	SYM_RATE_OOB_1_544MHz
}MXL_SYM_RATE_E;

#ifdef __MXL_INTEGER_CALC_STATISTICS__
typedef struct
{
	UINT8 I2cSlaveAddr;          /* IN, I2C Address of MxL Device */
	MXL_SYM_RATE_E SymbolRate;   /* IN, current symbol rate*/
	SINT64 CarrierOffset ;       /* OUT, OUT / 1E9 = MHz unit*/
} MXL_DEMOD_CARRIEROFFSET_INFO_T, *PMXL_DEMOD_CARRIEROFFSET_INFO_T;
#else
/* MXL_DEMOD_CARRIER_OFFSET_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;          /* IN, I2C Address of MxL Device */
	REAL64 CarrierOffset ;       /* OUT, MHz unit*/
} MXL_DEMOD_CARRIEROFFSET_INFO_T, *PMXL_DEMOD_CARRIEROFFSET_INFO_T;
#endif

/* MXL_DEMOD_DBG_CURRENT_BW_REQ */
typedef enum
{ 
	UNKNOWN = 0,
	BW_6MHz = 0x49,
	BW_8MHz = 0x6F
} MXL_BW_INFO_E;

typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	MXL_BW_INFO_E CurrnetBW ;   /* OUT  */ 
} MXL_BW_INFO_T, *PMXL_BW_INFO_T;

/* MXL_DEMOD_INTERLEAVER_DEPTH_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	UINT8 InterDepthI ;         /* OUT  */ 
	UINT8 InterDepthJ ;         /* OUT   */
} MXL_INTERDEPTH_INFO_T, *PMXL_INTERDEPTH_INFO_T;

#ifdef __MXL_INTEGER_CALC_STATISTICS__
/* MXL_DEMOD_EQUALIZER_FILTER_INT_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;        /* IN, I2C Address of MxL Device */
	SINT64 FfeInfo[16];        /* OUT, OUT / 1E9 = FFE filter */
	SINT64 SpurInfo[32];       /* OUT, OUT / 1E9 = Spur filter */
	SINT64 DfeInfo[56];        /* OUT, OUT / 1E9 = DFE filter */
	UINT8 DsEqMainLocation;    /* OUT, Location of leading tap (in FFE)*/
	UINT8 DsEqDfeTapNum;       /* OUT, Number of taps in DFE*/
} MXL_DEMOD_EQUALFILTER_INFO_T, *PMXL_DEMOD_EQUALFILTER_INFO_T;
#else
/* MXL_DEMOD_EQUALIZER_FILTER_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;        /* IN, I2C Address of MxL Device */
	REAL32 FfeInfo[16];        /* OUT, FFE filter */
	REAL32 SpurInfo[32];       /* OUT, Spur filter */
	REAL32 DfeInfo[56];        /* OUT, DFE filter */
	UINT8 DsEqMainLocation;    /* OUT, Location of leading tap (in FFE)*/
	UINT8 DsEqDfeTapNum;       /* OUT, Number of taps in DFE*/
} MXL_DEMOD_EQUALFILTER_INFO_T, *PMXL_DEMOD_EQUALFILTER_INFO_T;
#endif

/* MXL_DEMOD_TIMING_OFFSET_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;
#ifndef __MXL_INTEGER_CALC_STATISTICS__
	REAL64 TimingOffset;
#else
	UINT64 TimingOffset;   // Hz unit
#endif
} MXL_DEMOD_TIMINGOFFSET_INFO_T, *PMXL_DEMOD_TIMINGOFFSET_INFO_T;

/* MXL_DEMOD_ADC_IQ_FLIP_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;        /* IN, I2C Address of MxL Device */
	MXL_BOOL AdcIqFlip;        /* IN, Enable or disable Flip the I/Q path after ADC */
} MXL_ADCIQFLIP_CFG_T, *PMXL_ADCIQFLIP_CFG_T;

/* MXL_TUNER_DBG_LOCK_STATUS_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	MXL_BOOL AgcLockStatus;     /* OUT, Status of AGC lock */
	MXL_BOOL TunerDoneStatus;   /* OUT, Status of Tuner done */
	MXL_BOOL TunerLockStatus;   /* OUT, Status of Tuner Lock */
#ifdef __MXL241_DEBUG__
	MXL_BOOL RfSynthStatus;     /* OUT, Status of RF Synthesizer, Internal */
	MXL_BOOL RefSynthStatus;    /* OUT, Status of REF Synthesizer, Internal */
#endif
} MXL_TUNER_LOCK_STATUS_T, *PMXL_TUNER_LOCK_STATUS_T;

#ifdef __MXL_INTEGER_CALC_STATISTICS__
/* MXL_TUNER_RF_RX_PWR_INT_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	SINT32 RxPwr;               /* OUT, OUT / 1E3 = RF Input Power in dBm */
} MXL_TUNER_RX_PWR_T, *PMXL_TUNER_RX_PWR_T;
#else
/* MXL_TUNER_RF_RX_PWR_REQ */
typedef struct
{
  UINT8 I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
  REAL32 RxPwr;               /* OUT, RF Input Power in dBm */
} MXL_TUNER_RX_PWR_T, *PMXL_TUNER_RX_PWR_T;
#endif

/* MXL_TUNER_AGC_SETTINGS_REQ */
/* Refer to MXL_AGC_T above */

typedef struct
{
	UINT8  I2cSlaveAddr;
	UINT64 AccCwCorrCount;
	UINT64 AccCwErrCount;
	UINT64 AccCwUnerrCount;
	UINT64 AccCwReceived;
	UINT64 AccCorrBits;
	UINT64 AccErrMpeg;
	UINT64 AccReceivedMpeg;
} ACC_STAT_COUNTER_T, *PACC_STAT_COUNTER_T;

/*MXL_DEMOD_DBG_AGC_SELECT_CFG*/
typedef struct
{
	UINT8  I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	MXL_BOOL AgcMode;            /* IN, Self or Demod agc mode */
} MXL_AGC_SELECT_CFG_T, *PMXL_AGC_SELECT_CFG_T;

/* MXL_TUNER_DBG_IQOUT_CFG */
typedef struct
{
	UINT8  I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	MXL_BOOL IqOutCfg;           /* IN, IQ Out enable/disable */
} MXL_IQ_OUT_CFG_T, *PMXL_IQ_OUT_CFG_T;

/* MXL_TUNER_DBG_SENSOR_TEMP_REQ */
typedef struct
{
	UINT8  I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
	UINT8  ReadbackTemp;         /* OUT, Index for Sensor temperature */
} MXL_TEMP_SENSOR_T, *PMXL_TEMP_SENSOR_T;

/* MXL_DEMOD_DBG_AGC_SETPOINT_CFG */
typedef struct
{
	UINT8  I2cSlaveAddr;         /* IN, I2C Address of MxL Device */
#ifndef __MXL_INTEGER_CALC_STATISTICS__
	REAL32 DemodAgcSetPoint;     /* IN, Demod AGC set point */
#else
	UINT32 DemodAgcSetPoint;     /* IN, Demod AGC set point */
#endif
} MXL_DEMOD_AGC_SETPOINT_CFG_T, *PMXL_DEMOD_AGC_SETPOINT_CFG_T;

/* MXL_TUNER_DBG_AGC_SETPOINT_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;            /* IN, I2C Address of MxL Device */
	UINT16 AgcSetPoint;            /* IN, Internal */
} MXL_TUNER_AGC_SETPOINT_CFG_T, *PMXL_TUNER_AGC_SETPOINT_CFG_T;

/* MXL_TUNER_AGC_LOCK_SPEED_CFG */
typedef enum
{
	FAST_AGC_LOCK,
	NORMAL_AGC_LOCK
} MXL_AGC_LOCK_SPEED_E;

typedef struct
{
	UINT8 I2cSlaveAddr;                /* IN, I2C Address of MxL Device */
	MXL_AGC_LOCK_SPEED_E AgcSpeedMode; /* IN, AGC lock speed */
} MXL_AGC_CTRL_SPEED_T, *PMXL_AGC_CTRL_SPEED_T;

/* MXL_TUNER_CHAN_DEPENDENT_TUNE_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;                /* IN, I2C Address of MxL Device */
	MXL_BOOL ChanDependentCfg;         /* IN */
	UINT32 RfFreqHz;                   /* IN */
} MXL_CHAN_DEPENDENT_T, *PMXL_CHAN_DEPENDENT_T;

/* MXL_DEMOD_QAM_BURST_FREEZE_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;                /* IN, I2C Address of MxL Device */
	MXL_BOOL QamFreezeEnable;          /* IN */
	MXL_BOOL QamFreezeMode;            /* IN */
} MXL_QAM_BURST_FREEZE_T, *PMXL_QAM_BURST_FREEZE_T;

/* MXL_TUNER_DBG_GAIN_FREEZE_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;                /* IN, I2C Address of MxL Device */
} MXL_GAIN_FREEZE_T, *PMXL_GAIN_FREEZE_T;

/* MXL_DEMOD_RETUNE_INDICATOR_REQ */
typedef struct
{
	UINT8 I2cSlaveAddr;                /* IN, I2C Address of MxL Device */
	MXL_STATUS ReTuneInd;              /* OUT, Indicate retune required */
} MXL_RETUNE_IND_T, *PMXL_RETUNE_IND_T;

/* MXL_DEMOD_INVERT_CARRIER_OFFSET_CFG */
typedef struct
{
	UINT8 I2cSlaveAddr;                /* IN, I2C Address of MxL Device */
} MXL_INVERT_CARR_OFFSET_T, *PMXL_INVERT_CARR_OFFSET_T;

/******************************************************************************
    Global Variable Declarations
******************************************************************************/
extern const UINT8 MxLWareDrvVersion[];

/* Candidate index */
extern const UINT8 MxLWareCandidateIndex;

/******************************************************************************
    Prototypes
******************************************************************************/

#ifndef __MXL_GUI__
MXL_STATUS MxLWare_API_ConfigDevice(MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_GetDeviceStatus(MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_ConfigDemod(MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_GetDemodStatus(MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_ConfigTuner(MXL_CMD_TYPE_E CmdType, void *ParamPtr);
MXL_STATUS MxLWare_API_GetTunerStatus(MXL_CMD_TYPE_E CmdType, void *ParamPtr);
#endif

#endif /* __MXL241SF_PHY_CTRL_API_H__*/




