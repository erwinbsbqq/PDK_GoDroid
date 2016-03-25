/*
  Copyright (C) 2010 NXP B.V., All Rights Reserved.
  This source code and any compilation or derivative thereof is the proprietary
  information of NXP B.V. and is confidential in nature. Under no circumstances
  is this software to be  exposed to or placed under an Open Source License of
  any type without the expressed written permission of NXP B.V.
 *
 * \file          tmbslTDA10025local.h
 *
 *                %version: 1 %
 *
 * \date          %modify_time%
 *
 * \author        Alexandre TANT
 *
 * \brief         Describe briefly the purpose of this file.
 *
 * REFERENCE DOCUMENTS :
 *               
 *
 * \section info Change Information
 *
 * \par Changelog
 *  -2.2.49 TDA10025/27 updates for FIFO reset issue
 *  -2.2.49 TDA10025/27 updates for FIFO reset issue
 *  -2.2.48 apply reset to ADC FIFOs when exiting the full stand-by mode
 *  -2.2.47 remove redondant call to iTDA10025_ConfigurePLL() in HWinit
 *  -2.2.46 Make XTAL PLL configurable
 *  -2.2.45 allow extended SR recovery in J83B (still selectable in the configuration file)
 *  -2.2.44 Improve error management
 *  -2.2.43 Reduce I2C traffic when TSMF is disabled
 *  -2.2.42 change CtlTrkDirGain back to default in J83B/256QAM due to SNR regression at high RF frequencies
 *
 * \par Version_Definition
 *  VERSION_TAG:TDA10025_BSL_COMP_NUM.TDA10025_BSL_MAJOR_VER.TDA10025_BSL_MINOR_VER
 *
*/
#ifndef _TMBSLTDA10025LOCAL_H //-----------------
#define _TMBSLTDA10025LOCAL_H

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                       MACRO DEFINITION                                     */
/*============================================================================*/
#define TDA10025_BSL_COMP_NUM  2
#define TDA10025_BSL_MAJOR_VER 2
#define TDA10025_BSL_MINOR_VER 49

/* Instance macros */
#define P_OBJ_VALID                             (pObj != Null)

/* I/O Functions macros */
#define P_SIO                                   pObj->sIo
#define P_SIO_READ                              P_SIO.Read
#define P_SIO_WRITE                             P_SIO.Write
#define P_SIO_READ_VALID                        (P_OBJ_VALID && (P_SIO_READ != Null))
#define P_SIO_WRITE_VALID                       (P_OBJ_VALID && (P_SIO_WRITE != Null))

/* Time Functions macros */
#define P_STIME                                 pObj->sTime
#define P_STIME_WAIT                            P_STIME.Wait
#define P_STIME_WAIT_VALID                      (P_OBJ_VALID && (P_STIME_WAIT != Null))

/* Debug Functions macros */
#define P_SDEBUG                                pObj->sDebug
#define P_DBGPRINTEx                            P_SDEBUG.Print
#define P_DBGPRINTVALID                         (P_OBJ_VALID && (P_DBGPRINTEx != Null))

/* Mutex Functions macros */
#define P_SMUTEX                                pObj->sMutex
#ifdef _TVFE_SW_ARCH_V4
 #define P_SMUTEX_OPEN                           P_SMUTEX.Open
 #define P_SMUTEX_CLOSE                          P_SMUTEX.Close
#else
 #define P_SMUTEX_OPEN                           P_SMUTEX.Init
 #define P_SMUTEX_CLOSE                          P_SMUTEX.DeInit
#endif
#define P_SMUTEX_ACQUIRE                        P_SMUTEX.Acquire
#define P_SMUTEX_RELEASE                        P_SMUTEX.Release

#define P_SMUTEX_OPEN_VALID                     (P_OBJ_VALID && (P_SMUTEX_OPEN != Null))
#define P_SMUTEX_CLOSE_VALID                    (P_OBJ_VALID && (P_SMUTEX_CLOSE != Null))
#define P_SMUTEX_ACQUIRE_VALID                  (P_OBJ_VALID && (P_SMUTEX_ACQUIRE != Null))
#define P_SMUTEX_RELEASE_VALID                  (P_OBJ_VALID && (P_SMUTEX_RELEASE != Null))

/* Driver Mutex macros */
#define TDA10025_MUTEX_TIMEOUT                  (5000)
#define P_MUTEX                                 pObj->pMutex
#define P_MUTEX_VALID                           (P_MUTEX != Null)

#ifdef _TVFE_IMPLEMENT_MUTEX
 #define _MUTEX_ACQUIRE(_NAME) \
     if(err == TM_OK) \
     { \
         /* Try to acquire driver mutex */ \
         err = i##_NAME##_MutexAcquire(pObj, _NAME##_MUTEX_TIMEOUT); \
     } \
     if(err == TM_OK) \
     {

 #define _MUTEX_RELEASE(_NAME) \
         (void)i##_NAME##_MutexRelease(pObj); \
     }
#else
 #define _MUTEX_ACQUIRE(_NAME) \
     if(err == TM_OK) \
     {

 #define _MUTEX_RELEASE(_NAME) \
     }
#endif

#define POBJ_SRVFUNC_SIO pObj->sRWFunc
#define POBJ_SRVFUNC_STIME pObj->sTime

#define TDA10025_MAX_UNITS 6

/* PLL configuration */
#define TDA18265_ES1_PLL_CONFIG_NB 1

#define TDA18265_ES1_PLL_CONFIG                 \
    {                                           \
        16000000,             /* uXtal */       \
        30,                   /* uPLLMFactor */ \
        2,                    /* lPLLNFactor */ \
        1,                    /* bPLLPFactor */ \
        0x62,                 /* lPllCtl0Ind */ \
        0x6AAA,               /* lPllCtl1Ind */ \
        0x202,                /* lPllCtl2Ind */ \
        0x0,                  /* lPllCtl3Ind */ \
        TDA10025_PllMode_1c,  /* ePllMode */    \
        480000000             /* uPllValue */   \
    }

#define TDA18265_ES2_PLL_CONFIG_NB 3

#define TDA18265_ES2_PLL_CONFIG                 \
    {                                           \
        16000000,             /* uXtal */       \
        81,                   /* uPLLMFactor */ \
        8,                    /* lPLLNFactor */ \
        1,                    /* bPLLPFactor */ \
        0x62,                 /* lPllCtl0Ind */ \
        0x6963,               /* lPllCtl1Ind */ \
        0x2C,                 /* lPllCtl2Ind */ \
        0x7F0,                /* lPllCtl3Ind */ \
        TDA10025_PllMode_1c,  /* ePllMode */    \
        324000000             /* uPllValue */   \
    },                                          \
    {                                           \
        27000000,             /* uXtal */       \
        48,                   /* uPLLMFactor */ \
        8,                    /* lPLLNFactor */ \
        1,                    /* bPLLPFactor */ \
        0x62,                 /* lPllCtl0Ind */ \
        0x6667,               /* lPllCtl1Ind */ \
        0x2C,                 /* lPllCtl2Ind */ \
        0x674,                /* lPllCtl3Ind */ \
        TDA10025_PllMode_1c,  /* ePllMode */    \
        324000000             /* uPllValue */   \
    },                                          \
    {                                           \
        25000000,             /* uXtal */       \
        162,                  /* uPLLMFactor */ \
        25,                   /* lPLLNFactor */ \
        1,                    /* bPLLPFactor */ \
        0x62,                 /* lPllCtl0Ind */ \
        0x342D,               /* lPllCtl1Ind */ \
        0x3F,                 /* lPllCtl2Ind */ \
        0x7D8,                /* lPllCtl3Ind */ \
        TDA10025_PllMode_1c,  /* ePllMode */    \
        324000000             /* uPllValue */   \
    }

#define TDA10025_ES1_PLL_SAMPLING_CLOCK_DIVIDER 0x8 /* temporary: to be adapted when ES1 is removed */
#define TDA10025_ES1_PLL_D_CLOCK_DIVIDER 0x3 /* temporary: to be adapted when ES1 is removed */
#define TDA10025_ES2_PLL_SAMPLING_CLOCK_DIVIDER 0x6
#define TDA10025_ES2_PLL_D_CLOCK_DIVIDER 0x2

#define TDA10025_CDP_0_ADDRESS 0x0000
#define TDA10025_CDP_1_ADDRESS 0x0800

#define TDA10025_TSMF_0_ADDRESS 0x0000
#define TDA10025_TSMF_1_ADDRESS 0x0800

/*---------*/
/*  INDEX  */
/*---------*/
#define TDA10025_PLL_CTL_0_IND      0x1800
#define TDA10025_PLL_CTL_1_IND      0x1802
#define TDA10025_PLL_CTL_2_IND      0x1804
#define TDA10025_PLL_STATUS_IND     0x1806
#define TDA10025_PLL_CMD_IND        0x1808
#define TDA10025_CLK_EN_IND         0x180A
#define TDA10025_CLK_EXIT_RESET_IND 0x180C
#define TDA10025_CLK_INV_IND        0x180E
#define TDA10025_PLL_CTL_3_IND      0x1814
#define TDA10025_RESET_CONTROL_IND  0x2000
#define TDA10025_ADC0_CFG_IND       0x2800
#define TDA10025_ADC0_CS_IND        0x2802
#define TDA10025_ADC1_CFG_IND       0x2804
#define TDA10025_ADC1_CS_IND        0x2806
#define TDA10025_TS_COMMON_CFG_IND  0x280C
#define TDA10025_DEMOD0_TS_CFG_IND  0x280E
#define TDA10025_DEMOD1_TS_CFG_IND  0x2810
#define TDA10025_OOB_IF_CFG_IND     0x2812
#define TDA10025_PAD_CTRL_IND       0x281C

/*----------------*/
/*  DEFINE MASKS  */
/*----------------*/
#define TDA10025_PLL_CTL_0_PD_MSK   0x0100

#define TDA10025_PLL_CTL_0_DIRECTI_O_BIT  11
#define TDA10025_PLL_CTL_0_DIRECTI_O_MSK  0x1800
#define TDA10025_PLL_CTL_0_DIRECTI_MSK    0x1000
#define TDA10025_PLL_CTL_0_DIRECTO_MSK    0x0800

/* TDA10025_RESET_CONTROL_IND MASKS */
#define TDA10025_CDP_0_RSTN_MSK     0x0001
#define TDA10025_CDP_1_RSTN_MSK     0x0002

#define TDA10025_M_MSB_MSK          0x0080
#define TDA10025_P_MSK              0x007F
#define TDA10025_N_MSK              0x03FF
#define TDA10025_SELP_MSK           0x07C0
#define TDA10025_SELI_MSK           0x003F
#define TDA10025_ADC_ROUTE_MSK      0x0020
#define TDA10025_ADC_PD_MSK         0x0001
#define TDA10025_OUT_CLKDIR_MSK     0x0080
#define TDA10025_OUT_CLKDIV_MSK     0x001F
#define TDA10025_OUT_CLKPOL_MSK     0x0020

/* TDA10025_CLK_INV_IND MASKS */
#define TDA10025_CLK_INV_ADC0_MSK   0x0001
#define TDA10025_CLK_INV_ADC1_MSK   0x0002

/* TDA10025_CLK_EN_IND MASKS */
#define TDA10025_CLK_SER_TS_MSK     0x0200
#define TDA10025_CLK_OOB_MSK        0x0100
#define TDA10025_CLK_OOB_ADC_MSK    0x0080
#define TDA10025_CLK_CDP1_CORE_MSK  0x0040
#define TDA10025_CLK_CDP1_A2D_MSK   0x0020
#define TDA10025_CLK_CDP1_ADC_MSK   0x0010
#define TDA10025_CLK_CDP0_CORE_MSK  0x0008
#define TDA10025_CLK_CDP0_A2D_MSK   0x0004
#define TDA10025_CLK_CDP0_ADC_MSK   0x0002
#define TDA10025_CLK_CTRL_MSK       0x0001

/* TDA10025_OOB_IF_CFG_IND MASKS */
#define TDA10025_OOBIP_IF_CFG_CLK_DIV_MSK 0x001F

/* TDA10025_PAD_CTRL_IND MASKS */
#define TDA10025_OOB_TS_MSK         0x0010
#define TDA10025_OOB_POD_MSK        0x0008
#define TDA10025_OOB_TUNER_MSK      0x0004
#define TDA10025_DEMOD1_TUNER_MSK   0x0002
#define TDA10025_DEMOD0_TUNER_MSK   0x0001

/* TDA10025_TS_COMMON_CFG_IND MASKS */
#define TDA10025_ENA_TS1_MSK        0x0010
#define TDA10025_ENA_TS0_MSK        0x0008
#define TDA10025_TS_IF1_ROUTE_MSK   0x0004
#define TDA10025_TS_IF0_ROUTE_MSK   0x0002
#define TDA10025_OUT_PARASER_MSK    0x0001

/*----------------*/
/* TSMF registers */
/*----------------*/
#define TDA10025_BYPASS                  0x3000
#define TDA10025_BYPASS_BYPASS_MSK       0x1
#define TDA10025_BYPASS_BYPASS_DISABLE   0x0
#define TDA10025_BYPASS_BYPASS_ENABLE    0x1

#define TDA10025_MODE                    0x3002
#define TDA10025_MODE_BYPASS_MSK         0xF
#define TDA10025_MODE_BYPASS_DISABLE     0x0
#define TDA10025_MODE_BYPASS_ENABLE      0x1
#define TDA10025_MODE_BYPASS_AUTOMATIC   0x2

#define TDA10025_REG0x05                 0x3008
#define TDA10025_REG0x05_M_LOCK_MSK      0x01
#define TDA10025_REG0x05_M_LOCK_BIT      0
#define TDA10025_REG0x05_VER_MSK         0x0E
#define TDA10025_REG0x05_VER_BIT         1         
#define TDA10025_REG0x05_CRE_ST_MSK      0x30
#define TDA10025_REG0x05_CRE_ST_BIT      4
#define TDA10025_REG0x05_EMERGENCY_MSK   0x40
#define TDA10025_REG0x05_EMERGENCY_BIT   6
#define TDA10025_REG0x05_ERROR_MSK       0x80
#define TDA10025_REG0x05_ERROR_BIT       7

#define TDA10025_TS_ID                   0x3004
#define TDA10025_ON_ID                   0x3006

#define TDA10025_TS_STATUS               0x300A
#define TDA10025_RE_STATUS_MSB           0x300C
#define TDA10025_RE_STATUS_LSB           0x300E

#define BIT_MSK                          0x01

#define TDA10025_TS_ID_OFFSET            0x4
#define TDA10025_ON_ID_OFFSET            0x4

#define TDA10025_TS_ID1                  0x3010
#define TDA10025_ON_ID1                  0x3012

#define TDA10025_TS_ID2                  0x3014
#define TDA10025_ON_ID2                  0x3016

#define TDA10025_TS_ID3                  0x3018
#define TDA10025_ON_ID3                  0x301A

#define TDA10025_TS_ID4                  0x301C
#define TDA10025_ON_ID4                  0x301E

#define TDA10025_TS_ID5                  0x3020
#define TDA10025_ON_ID5                  0x3022

#define TDA10025_TS_ID6                  0x3024
#define TDA10025_ON_ID6                  0x3026

#define TDA10025_TS_ID7                  0x3028
#define TDA10025_ON_ID7                  0x302A

#define TDA10025_TS_ID8                  0x302C
#define TDA10025_ON_ID8                  0x302E

#define TDA10025_TS_ID9                  0x3030
#define TDA10025_ON_ID9                  0x3032

#define TDA10025_TS_ID10                 0x3034
#define TDA10025_ON_ID10                 0x3036

#define TDA10025_TS_ID11                 0x3038
#define TDA10025_ON_ID11                 0x303A

#define TDA10025_TS_ID12                 0x303C
#define TDA10025_ON_ID12                 0x303E

#define TDA10025_TS_ID13                 0x3040
#define TDA10025_ON_ID13                 0x3042

#define TDA10025_TS_ID14                 0x3044
#define TDA10025_ON_ID14                 0x3046

#define TDA10025_TS_ID15                 0x3048
#define TDA10025_ON_ID15                 0x304A

/*============================================================================*/
/*                       ENUM OR TYPE DEFINITION                              */
/*============================================================================*/
typedef enum _TDA10025_HwSampleVersion_t
{
    TDA10025_HwSampleVersion_Unknown = 0,
    TDA10025_HwSampleVersion_ES1,
    TDA10025_HwSampleVersion_ES2,
    TDA10025_HwSampleVersion_Max
} TDA10025_HwSampleVersion_t;

typedef enum _TDA10025_ConstellationSource_t
{
    TDA10025_ConstellationSourceUnkown = 0,
    TDA10025_ConstellationSourceADC,
    TDA10025_ConstellationSourceFEDR,
    TDA10025_ConstellationSourcePDF,
    TDA10025_ConstellationSourceDAGC,
    TDA10025_ConstellationSourceMF,
    TDA10025_ConstellationSourceCAGC,
    TDA10025_ConstellationSourceEqualizer,
    TDA10025_ConstellationSourceBEDR,
    TDA10025_ConstellationSourceMax
}TDA10025_ConstellationSource_t;

typedef struct _TDA10025IP_t
{
    tmPowerState_t                  ePowerState;
    UInt16                          uIFAGCThreshold;
    UInt16                          uRFAGCThreshold;
    TDA10025_ConstellationSource_t  sConstSource;
    Bool                            bUncorPresent;
} TDA10025IP_t;

typedef enum _TDA10025_Cfg_Err_t
{
    TDA10025_Cfg_NoError_E,
    TDA10025_Cfg_InvalidValue_E
} TDA10025_Cfg_Err_t;

typedef enum _TDA10025_IF_t
{
    TDA10025_IF_Unknown = 0,
    TDA10025_IF_0,
    TDA10025_IF_1,
    TDA10025_IF_Max
} TDA10025_IF_t;

typedef enum _TDA10025_CDP_t
{
    TDA10025_CDP_Unknown = 0,
    TDA10025_CDP_0,
    TDA10025_CDP_1,
    TDA10025_CDP_Max
} TDA10025_CDP_t;

typedef enum _TDA10025_TS_t
{
    TDA10025_TS_Unknown = 0,
    TDA10025_TS_0,
    TDA10025_TS_1,
    TDA10025_TS_Max
} TDA10025_TS_t;

typedef enum _TDA10025_TsMode_t
{
    TDA10025_TsModeSerial = 0,
    TDA10025_TsModeParallel,
    TDA10025_TsModeInvalid
} TDA10025_TsMode_t;

typedef enum _TDA10025_TsCfg_ClkPol_t
{
    TDA10025_TsCfg_ClkPolNormal = 0,
    TDA10025_TsCfg_ClkPolInversion,
    TDA10025_TsCfg_ClkPolInvalid
} TDA10025_TsCfg_ClkPol_t;

typedef enum _TDA10025_TsCfg_ClkDir_t
{
    TDA10025_TsCfg_ClkDirIsOutput = 0,
    TDA10025_TsCfg_ClkDirIsInput,
    TDA10025_TsCfg_ClkDirInvalid
} TDA10025_TsCfg_ClkDir_t;

typedef enum _TDA10025_TsCfg_ClkStdby_t
{
    TDA10025_TsCfg_ClkInStdbyOff = 0,
    TDA10025_TsCfg_ClkInStdbyOn,
    TDA10025_TsCfg_ClkInStdbyInvalid
} TDA10025_TsCfg_ClkStdby_t;


typedef enum _TDA10025_PllMode_t
{
    TDA10025_PllMode_1a = 3,
    TDA10025_PllMode_1b = 2,
    TDA10025_PllMode_1c = 1,
    TDA10025_PllMode_1d = 0,
    TDA10025_PllMode_Invalid = 4
} TDA10025_PllMode_t;

typedef struct _TDA10025PllConfig_t
{
    UInt32                  uXtal;
    UInt32                  uPLLMFactor;
    UInt16                  lPLLNFactor;
    UInt8                   bPLLPFactor;
    UInt16                  lPllCtl0Ind;
    UInt16                  lPllCtl1Ind;
    UInt16                  lPllCtl2Ind;
    UInt16                  lPllCtl3Ind;
    TDA10025_PllMode_t      ePllMode;
    UInt32                  uPllValue;
} TDA10025PllConfig_t;

typedef enum _TDA10025_ExtendSymbolRateMode_t
{
    TDA10025_ExtendSymbolRateModeDisable,
    TDA10025_ExtendSymbolRateModeEnable700ppm,
    TDA10025_ExtendSymbolRateModeEnable1500ppm, /* costly in lock time, till 1 sec */
    TDA10025_ExtendSymbolRateModeInvalid
}TDA10025_ExtendSymbolRateMode_t;

typedef struct _TDA10025_Cfg_t
{
    unsigned int            CompatibilityNb_U;
    /* Internal path selection */
    TDA10025_IF_t           eIF;
    TDA10025_CDP_t          eCDP;
    TDA10025_TS_t           eTS;

    /* TS output */
    TDA10025_TsMode_t       eTsMode;

    TDA10025_TsCfg_ClkDir_t   eTsClkDir;
    TDA10025_TsCfg_ClkPol_t   eTsClkPol;
    TDA10025_TsCfg_ClkStdby_t eTsClkStdby;
    unsigned char           ulTsClkDiv;

    /* Extend Timing Recovery */
    TDA10025_ExtendSymbolRateMode_t eExtendSRMode;
} TDA10025_Cfg_t;

/*============================================================================*/
/* Generic Configuration items                                                */
/*============================================================================*/
typedef struct _TDA10025Object_t
{
    tmUnitSelect_t                  tUnit;
    tmUnitSelect_t                  tUnitW;
    tmUnitSelect_t                  tUnitCommon;
    tmUnitSelect_t                  tUnitOtherStream;
    Bool                            init;
    tmbslFrontEndIoFunc_t           sIo;
    tmbslFrontEndTimeFunc_t         sTime;
    tmbslFrontEndDebugFunc_t        sDebug;
    tmbslFrontEndMutexFunc_t        sMutex;
    ptmbslFrontEndMutexHandle       pMutex;
    /* Device specific part: */
    TDA10025_HwSampleVersion_t      eHwSample;
    TDA10025IP_t                    sIP; /* HCDP IP */
    TDA10025_ChannelSelection_t     eChannelSel; /* Channel selection of this IP */
    tmPowerState_t                  ePowerState;
    TDA10025PllConfig_t             sPllConfig;
    TDA10025_Cfg_t                  sCfg;
} TDA10025Object_t, *pTDA10025Object_t, **ppTDA10025Object_t;

typedef struct _TDA10025_LogTable_t
{
    UInt32 uX;
    UInt32 uLogX;  /*100*log(x)*/
} TDA10025_LogTable_t;

#define TDA10025_LOG_TABLE \
    /* X   ,  1000*LogX */ \
    {1,0}, \
    {2,301}, \
    {3,477}, \
    {4,602}, \
    {5,698}, \
    {6,778}, \
    {7,845}, \
    {8,903}, \
    {9,954}, \
    {10,1000}, \
    {11,1041}, \
    {12,1079}, \
    {13,1113}, \
    {14,1146}, \
    {15,1176}, \
    {16,1204}, \
    {17,1230}, \
    {18,1255}, \
    {19,1278}, \
    {20,1301}, \
    {21,1322}, \
    {22,1342}, \
    {23,1361}, \
    {24,1380}, \
    {25,1397}, \
    {26,1414}, \
    {27,1431}, \
    {28,1447}, \
    {29,1462}, \
    {30,1477}, \
    {31,1491}, \
    {32,1505}, \
    {33,1518}, \
    {34,1531}, \
    {35,1544}, \
    {36,1556}, \
    {37,1568}, \
    {38,1579}, \
    {39,1591}, \
    {41,1612}, \
    {42,1623}, \
    {43,1633}, \
    {45,1653}, \
    {46,1662}, \
    {48,1681}, \
    {49,1690}, \
    {51,1707}, \
    {52,1716}, \
    {54,1732}, \
    {56,1748}, \
    {58,1763}, \
    {60,1778}, \
    {62,1792}, \
    {63,1799}, \
    {66,1819}, \
    {68,1832}, \
    {70,1845}, \
    {72,1857}, \
    {74,1869}, \
    {77,1886}, \
    {79,1897}, \
    {82,1913}, \
    {84,1924}, \
    {87,1939}, \
    {90,1954}, \
    {93,1968}, \
    {96,1982}, \
    {99,1995}, \
    {102,2008}, \
    {105,2021}, \
    {109,2037}, \
    {112,2049}, \
    {116,2064}, \
    {120,2079}, \
    {123,2089}, \
    {127,2103}, \
    {132,2120}, \
    {136,2133}, \
    {140,2146}, \
    {145,2161}, \
    {149,2173}, \
    {154,2187}, \
    {159,2201}, \
    {164,2214}, \
    {169,2227}, \
    {175,2243}, \
    {180,2255}, \
    {186,2269}, \
    {192,2283}, \
    {198,2296}, \
    {205,2311}, \
    {211,2324}, \
    {218,2338}, \
    {225,2352}, \
    {232,2365}, \
    {240,2380}, \
    {247,2392}, \
    {255,2406}, \
    {264,2421}, \
    {272,2434}, \
    {281,2448}, \
    {290,2462}, \
    {299,2475}, \
    {309,2489}, \
    {318,2502}, \
    {329,2517}, \
    {339,2530}, \
    {350,2544}, \
    {361,2557}, \
    {373,2571}, \
    {385,2585}, \
    {397,2598}, \
    {410,2612}, \
    {423,2626}, \
    {437,2640}, \
    {451,2654}, \
    {465,2667}, \
    {480,2681}, \
    {495,2694}, \
    {511,2708}, \
    {528,2722}, \
    {544,2735}, \
    {562,2749}, \
    {580,2763}, \
    {598,2776}, \
    {618,2790}, \
    {637,2804}, \
    {658,2818}, \
    {679,2831}, \
    {701,2845}, \
    {723,2859}, \
    {746,2872}, \
    {770,2886}, \
    {795,2900}, \
    {820,2913}, \
    {846,2927}, \
    {874,2941}, \
    {901,2954}, \
    {930,2968}, \
    {960,2982}, \
    {991,2996}, \
    {1023,3009}, \
    {1055,3023}, \
    {1089,3037}, \
    {1124,3050}, \
    {1160,3064}, \
    {1197,3078}, \
    {1235,3091}, \
    {1275,3105}, \
    {1316,3119}, \
    {1358,3132}, \
    {1401,3146}, \
    {1446,3160}, \
    {1493,3174}, \
    {1540,3187}, \
    {1590,3201}, \
    {1641,3215}, \
    {1693,3228}, \
    {1747,3242}, \
    {1803,3255}, \
    {1861,3269}, \
    {1920,3283}, \
    {1982,3297}, \
    {2045,3310}, \
    {2111,3324}, \
    {2178,3338}, \
    {2248,3351}, \
    {2320,3365}, \
    {2394,3379}, \
    {2471,3392}, \
    {2550,3406}, \
    {2632,3420}, \
    {2716,3433}, \
    {2803,3447}, \
    {2892,3461}, \
    {2985,3474}, \
    {3081,3488}, \
    {3179,3502}, \
    {3281,3516}, \
    {3386,3529}, \
    {3494,3543}, \
    {3606,3557}, \
    {3722,3570}, \
    {3841,3584}, \
    {3964,3598}, \
    {4090,3611}, \
    {4221,3625}, \
    {4356,3639}, \
    {4496,3652}, \
    {4640,3666}, \
    {4788,3680}, \
    {4941,3693}, \
    {5100,3707}, \
    {5263,3721}, \
    {5431,3734}, \
    {5605,3748}, \
    {5784,3762}, \
    {5970,3775}, \
    {6161,3789}, \
    {6358,3803}, \
    {6561,3816}, \
    {6771,3830}, \
    {6988,3844}, \
    {7212,3858}, \
    {7442,3871}, \
    {7680,3885}, \
    {7926,3899}, \
    {8180,3912}, \
    {8442,3926}, \
    {8712,3940}, \
    {8991,3953}, \
    {9278,3967}, \
    {9575,3981}, \
    {9882,3994}, \
    {10198,4008}, \
    {10524,4022}, \
    {10861,4035}, \
    {11209,4049}, \
    {11567,4063}, \
    {11938,4076}, \
    {12320,4090}, \
    {12714,4104}, \
    {13121,4117}, \
    {13541,4131}, \
    {13974,4145}, \
    {14421,4158}, \
    {14883,4172}, \
    {15359,4186}, \
    {15850,4200}, \
    {16357,4213}, \
    {16881,4227}, \
    {17421,4241}, \
    {17979,4254}, \
    {18554,4268}, \
    {19148,4282}, \
    {19760,4295}, \
    {20393,4309}, \
    {21045,4323}, \
    {21719,4336}, \
    {22414,4350}, \
    {23131,4364}, \
    {23871,4377}, \
    {24635,4391}, \
    {25424,4405}, \
    {26237,4418}, \
    {27077,4432}, \
    {27943,4446}, \
    {28837,4459}, \
    {29760,4473}, \
    {30713,4487}, \
    {31695,4500}, \
    {32710,4514}, \
    {33756,4528}, \
    {34837,4542}, \
    {35951,4555}, \
    {37102,4569}, \
    {38289,4583}, \
    {39514,4596}, \
    {40779,4610}, \
    {42084,4624}, \
    {43431,4637}, \
    {44820,4651}, \
    {46255,4665}, \
    {47735,4678}, \
    {49262,4692}, \
    {50839,4706}, \
    {52466,4719}, \
    {54145,4733}, \
    {55877,4747}, \
    {57665,4760}, \
    {59511,4774}, \
    {61415,4788}, \
    {63380,4801}, \
    {65408,4815}, \
    {67501,4829}, \
    {69662,4842}, \
    {71891,4856}, \
    {74191,4870}, \
    {76565,4884}, \
    {79015,4897}, \
    {81544,4911}, \
    {84153,4925}, \
    {86846,4938}, \
    {89625,4952}, \
    {92493,4966}, \
    {95453,4979}, \
    {98508,4993}, \
    {0,0}

/*============================================================================*/
/* Internal Prototypes:                                                       */
/*============================================================================*/
extern tmErrorCode_t iTDA10025_Write(tmUnitSelect_t tUnit, UInt16 uIndex_U, UInt32 uNBytes_U, UInt16* puData_U);
extern tmErrorCode_t iTDA10025_WriteBit(tmUnitSelect_t tUnit, UInt16 uIndex_U, UInt16 uMask_U, UInt16 uData_U);
extern tmErrorCode_t iTDA10025_Read(tmUnitSelect_t tUnit, UInt16 uIndex_U, UInt32 uNBytes_U, UInt16* puData_U);

extern tmErrorCode_t iTDA10025_Wait(pTDA10025Object_t pObj,UInt32 uTime);

#ifdef _TVFE_IMPLEMENT_MUTEX
 extern tmErrorCode_t iTDA10025_MutexAcquire(pTDA10025Object_t pObj, UInt32 timeOut);
 extern tmErrorCode_t iTDA10025_MutexRelease(pTDA10025Object_t pObj);
#endif
#ifdef __cplusplus
}
#endif

#endif /* TMBSLTDA10025LOCAL_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/


