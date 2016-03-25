/**
  Copyright (C) 2008 NXP B.V., All Rights Reserved.
  This source code and any compilation or derivative thereof is the proprietary
  information of NXP B.V. and is confidential in nature. Under no circumstances
  is this software to be  exposed to or placed under an Open Source License of
  any type without the expressed written permission of NXP B.V.
 *
 * \file          tmbslTDA10025.h
 *                %version: CFR_FEAP#17 %
 *
 * \date          %date_modified%
 *
 * \brief         Describe briefly the purpose of this file.
 *
 * REFERENCE DOCUMENTS :
 *
 * Detailled description may be added here.
 *
 * \section info Change Information
 *
 * \verbatim
   Date          Modified by CRPRNr  TASKNr  Maintenance description
   -------------|-----------|-------|-------|-----------------------------------
   9-JULY-2007  | A.TANT    |       |       | CREATION OF TDA10025 ARCHITECTURE 2.0.0
   -------------|-----------|-------|-------|-----------------------------------
                |           |       |       |
   -------------|-----------|-------|-------|-----------------------------------
   \endverbatim
 *
*/

#ifndef _tmbslTDA10025_H //-----------------
#define _tmbslTDA10025_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/


/*============================================================================*/
/*                       MACRO DEFINITION                                     */
/*============================================================================*/
#define TMBSL_TDA10025_NBREG        100

/* SW Error codes */
#define TDA10025_ERR_BASE               (CID_COMP_DEMOD | CID_LAYER_BSL)
#define TDA10025_ERR_COMP               (CID_COMP_DEMOD | CID_LAYER_BSL | TM_ERR_COMP_UNIQUE_START)

#define TDA10025_ERR_BAD_UNIT_NUMBER    (TDA10025_ERR_BASE + TM_ERR_BAD_UNIT_NUMBER)
#define TDA10025_ERR_NOT_INITIALIZED    (TDA10025_ERR_BASE + TM_ERR_NOT_INITIALIZED)
//#define TDA10025_ERR_INIT_FAILED        (TDA10025_ERR_BASE + TM_ERR_INIT_FAILED)
#define TDA10025_ERR_BAD_PARAMETER      (TDA10025_ERR_BASE + TM_ERR_BAD_PARAMETER)
#define TDA10025_ERR_NOT_SUPPORTED      (TDA10025_ERR_BASE + TM_ERR_NOT_SUPPORTED)
//#define TDA10025_ERR_ERR_NO_INSTANCES   (TDA10025_ERR_BASE + TM_ERR_NO_INSTANCES)
//#define TDA10025_ERR_ALREADY_SETUP      (TDA10025_ERR_BASE + TM_ERR_ALREADY_SETUP)
#define TDA10025_ERR_NULL_CONTROLFUNC   (TDA10025_ERR_BASE + TM_ERR_NULL_CONTROLFUNC)

//#define TDA10025_ERR_HW_FAILED          (TDA10025_ERR_COMP + 0x0001)
#define TDA10025_ERR_NOT_READY          (TDA10025_ERR_COMP + 0x0002)
#define TDA10025_ERR_BAD_VERSION        (TDA10025_ERR_COMP + 0x0003)
//#define TDA10025_ERR_STD_NOT_SET        (TDA10025_ERR_COMP + 0x0004)
//#define TDA10025_ERR_RF_NOT_SET         (TDA10025_ERR_COMP + 0x0005)
#define TDA10025_ERR_DRIVER_RETURN      (TDA10025_ERR_COMP + 0x0006)

typedef struct _TDA10025_LockInd_t
{
    Bool bCAGCSat;
    Bool bDAGCSat;
    Bool bAagc;
    Bool bStl;
    Bool bCtl;
    Bool bSctl;
    Bool bDemod;
    Bool bFEC;
    UInt16 uDemodLockTime;
    UInt16 uFecLockTime;
}TDA10025_LockInd_t;

typedef enum _TDA10025_BERWindow_t
{
    TDA10025_BERWindowUnknown = 0,
    TDA10025_BERWindow1E5, /*1e5 bits*/
    TDA10025_BERWindow1E6, /*1e6 bits*/
    TDA10025_BERWindow1E7, /*1e7 bits*/
    TDA10025_BERWindow1E8, /*1e8 bits*/
    TDA10025_BERWindowMax
}TDA10025_BERWindow_t;

typedef enum _TDA10025_ChannelSelection_t
{
    TDA10025_ChannelSelection_tUnknown,
    TDA10025_ChannelSelectionIpA,
    TDA10025_ChannelSelectionIpB,
    TDA10025_ChannelSelectionIpA_SinglePath,
    TDA10025_ChannelSelectionMax
}TDA10025_ChannelSelection_t;

typedef struct _TDA10025_AdditionnalData_t
{
    tmUnitSelect_t tUnitCommon; /* tUnit of the instance containing the common parameters of the TDA10025 */
    TDA10025_ChannelSelection_t eChannelSel; /* Type of the channel */
}TDA10025_AdditionnalData_t;

typedef enum _TDA10025_TsmfMode_t
{
    TDA10025_TsmfModeDisabled,
    TDA10025_TsmfModeForced,
    TDA10025_TsmfModeAuto,
    TDA10025_TsmfModeInvalid
}TDA10025_TsmfMode_t;

typedef enum _TDA10025_TsmfStatusError_t
{
    TDA10025_TsmfStatusNoError,
    TDA10025_TsmfStatusError,
    TDA10025_TsmfStatusErrorInvalid
}TDA10025_TsmfStatusError_t;

typedef enum _TDA10025_TsmfEmergency_t
{
    TDA10025_TsmfEmergencyInactive,
    TDA10025_TsmfEmergencyActive,
    TDA10025_TsmfEmergencyNA,
}TDA10025_TsmfEmergency_t;

typedef enum _TDA10025_TsmfReceiveStatus_t
{
    TDA10025_TsmfReceiveStatusGood,
    TDA10025_TsmfReceiveStatusMedium,
    TDA10025_TsmfReceiveStatusBad,
    TDA10025_TsmfReceiveStatusNA
}TDA10025_TsmfReceiveStatus_t;

typedef enum _TDA10025_TsmfTsStatus_t
{
    TDA10025_TsmfTsStatusValid,
    TDA10025_TsmfTsStatusInvalid
}TDA10025_TsmfTsStatus_t;

typedef enum _TDA10025_TsmfStatusLock_t
{
    TDA10025_TsmfStatusLockNotlocked,
    TDA10025_TsmfStatusLockLocked,
    TDA10025_TsmfStatusLockInvalid
}TDA10025_TsmfStatusLock_t;

typedef struct _TDA10025_TsmfStatus_t
{
    TDA10025_TsmfStatusLock_t           eLockStatus;
    TDA10025_TsmfStatusError_t          eError;
    TDA10025_TsmfEmergency_t            eEmergency;
    TDA10025_TsmfReceiveStatus_t        eCurrentReceiveStatus;
    UInt16                              uVersionNumber;
    UInt16                              uSelectedTsId;
    UInt16                              uSelectedOnId;
}TDA10025_TsmfStatus_t;

typedef struct _TDA10025_TsmfAvailableTs_t
{
    UInt16                       TsId;
    UInt16                       OnId;
    TDA10025_TsmfTsStatus_t      eTsStatus;
    TDA10025_TsmfReceiveStatus_t eRcvStatus;
}TDA10025_TsmfAvailableTs_t;

typedef enum _TDA10025_XtalFreq_t
{
    TDA10025_XtalFreq_16MHz,
    TDA10025_XtalFreq_27MHz,
    TDA10025_XtalFreq_25MHz,
    TDA10025_XtalFreq_Invalid
}TDA10025_XtalFreq_t;

#define TDA10025_TSMF_AVAILABLE_TS_NUMBER 15

/*============================================================================*/
/* Types and defines:                                                         */
/*============================================================================*/

/**
    \brief  Create an instance of the TDA10025 demodulator

    \param  tUnit Demodulator unit number
    \param  psSrvFunc Structure containing the Hardware Access, the Time and the Debug functions.

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED Error while initialisation
    \return TDA10025_ERR_BAD_PARAMETER Parameter is not correctly filled

    \note   No hardware access is done in this function
    
    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_Open
(
    tmUnitSelect_t             tUnit,
    tmbslFrontEndDependency_t *psSrvFunc
);

/**
    \brief  Destroy the instance of the TDA10025 demodulator

    \param  tUnit Demodulator unit number

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
             
    \note   NA

    \sa     NA
 */
extern tmErrorCode_t 
tmbslTDA10025_Close 
(
    tmUnitSelect_t tUnit
);

/**
    \brief  Get the software version

    \param  pSWVersion Pointer to the software version

    \return TM_OK Succeed
             
    \note   ptmSWVersion_t is defined in the tmnxtypes.h file

    \sa     NA
 */
extern tmErrorCode_t   
tmbslTDA10025_GetSWVersion 
(
    ptmSWVersion_t pSWVersion
);

/**
    \brief  Get the version of the configuration file

    \param  pSWSettingsVersion Pointer to the configuration file version

    \return TM_OK Succeed

    \note   Values defined in the tmnxtypes.h file

    \sa     NA
 */
extern tmErrorCode_t     
tmbslTDA10025_GetSWSettingsVersion
(
    ptmSWSettingsVersion_t  pSWSettingsVersion
);

/**
    \brief  Perform the hardware initialisation 

    \param  pSWSettingsVersion Demodulator unit number

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_VERSION This hardware version is not supported
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)

    \note   To be performed once tmbslTDA10025_Open has been called

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_HwInit
(
    tmUnitSelect_t tUnit
);

/**
    \brief  Set the power state

    \param  tUnit Demodulator unit number

    \param  ePowerState Power state (tmPowerOn or tmPowerStandby)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_PARAMETER Parameter is not correctly filled
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)

    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_SetPowerState
(
    tmUnitSelect_t tUnit,
    tmPowerState_t ePowerState
);

/**
    \brief  Get the power state

    \param  tUnit Demodulator unit number
    \param  pePowerState Pointer to the current power state (tmPowerOn or tmPowerStandby)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
             
    \note   NA

    \sa     NA
 */

extern tmErrorCode_t
tmbslTDA10025_GetPowerState
(
    tmUnitSelect_t  tUnit,
    tmPowerState_t *pePowerState
);

/**
    \brief  Start the lock

    \param  tUnit demodulator unit number
    
    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
    \return TDA10025_ERR_NOT_READY  The action can not be performed yet

    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_StartLock 
(
    tmUnitSelect_t tUnit
);

/**
    \brief  Set symbol rate

    \param  tUnit Demodulator unit number
    \param  uSR Symbol rate (symbols per second)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_PARAMETER Parameter is not correctly filled
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
             
    \note   The symbol rate value must be between 1000000 and 7200000

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_SetSR 
(
    tmUnitSelect_t tUnit,
    UInt32         uSR
);

/**
    \brief  Get current symbol rate

    \param  tUnit Demodulator unit number
    \param  puCS Pointer to the symbol rate (symbols per second)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
         
    \note   Due to the symbol rate tracking, this symbol rate differs from the initial value

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetSR 
(
    tmUnitSelect_t  tUnit,
    UInt32         *puCS
);

/**
    \brief  Get configured symbol rate

    \param  tUnit Demodulator unit number
    \param  puCS Pointer to the configured symbol rate (symbols per second)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
             
    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetConfiguredSR 
(
    tmUnitSelect_t tUnit,
    UInt32         *puCS
);

/**
    \brief  Set spectral inversion

    \param  tUnit Demodulator unit number
    \param  eSI Spectral Inversion (On, Off or Auto)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_PARAMETER Parameter is not correctly filled
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)

    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_SetSI 
(
    tmUnitSelect_t      tUnit,
    tmFrontEndSpecInv_t eSI
);

/**
    \brief  Get spectral inversion

    \param  tUnit Demodulator unit number
    \param  peSI Pointer to the actual Spectral Inversion (On, Off)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
             
    \note   The returned value makes sense only when the demodulator is locked

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetSI 
(
    tmUnitSelect_t       tUnit,
    tmFrontEndSpecInv_t *peSI
);

/**
    \brief  Get configured spectral inversion

    \param  tUnit Demodulator unit number
    \param  peSI Pointer to the spectral inversion (On, Off or Auto)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
             
    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetConfiguredSI 
(
    tmUnitSelect_t       tUnit,
    tmFrontEndSpecInv_t *peSI
);

/**
    \brief  Set the modulation

    \param  tUnit Demodulator unit number
    \param  eMod Modulation scheme (16QAM to 256QAM)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_PARAMETER Parameter is not correctly filled
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
             
    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_SetMod 
(
    tmUnitSelect_t         tUnit,
    tmFrontEndModulation_t eMod
);

/**
    \brief  Get the current modulation

    \param  tUnit Demodulator unit number
    \param  peMod Pointer to the modulation type

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
             
    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetMod 
(
    tmUnitSelect_t          tUnit,
    tmFrontEndModulation_t *peMod
);

/**
    \brief  Set the BER window size

    \param  tUnit Demodulator unit number
    \param  eBERWindow BER window size (1E5, 1E6, 1E7 or 1E8 bits)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     tmbslTDA10025_GetBER
    \sa     tmbslTDA10025_ClearUncor
 */
extern tmErrorCode_t
tmbslTDA10025_SetBERWindow
(
    tmUnitSelect_t       tUnit,
    TDA10025_BERWindow_t eBERWindow
);


/**
    \brief  Get BER value (Bit Error Rate)

    \param  tUnit Demodulator unit number
    \param  psBER Pointer to the BER value
    \param  puUncors Pointer to the number of uncorrected packets

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     tmbslTDA10025_SetBERWindow
    \sa     tmbslTDA10025_ClearUncor
 */
extern tmErrorCode_t
tmbslTDA10025_GetBER
(
    tmUnitSelect_t        tUnit,
    tmFrontEndFracNb32_t *psBER,
    UInt32               *puUncors
);

/**
    \brief  Resets Uncors

    \param  tUnit Demodulator unit number
    
    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     tmbslTDA10025_SetBERWindow
    \sa     tmbslTDA10025_GetBER
 */
extern tmErrorCode_t
tmbslTDA10025_ClearUncor
(
    tmUnitSelect_t tUnit
);

/**
    \brief  Get the BER window size

    \param  tUnit Demodulator unit number
    \param  peBERWindow  Pointer to the BER window size (1e5, 1e6, 1e7 or 1e8 bits)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetBERWindow
(
    tmUnitSelect_t        tUnit,
    TDA10025_BERWindow_t *peBERWindow
);

/**
    \brief  Get lock status

    \param  tUnit Demodulator unit number
    \param  peLockStatus Pointer to the the lock status (Searching, Locked or NotLocked)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetLockStatus
(
    tmUnitSelect_t        tUnit,
    tmbslFrontEndState_t *peLockStatus
);

/**
    \brief  Set the FEC mode

    \param  tUnit Demodulator unit number
    \param  eMode FEC mode (AnnexA, AnnexB)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_PARAMETER Parameter is not correctly filled
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   AnnexA is DVB-C (or J83A)
    \note   AnnexB is J83B

    \sa     tmbslTDA10025_GetFECMode
 */
extern tmErrorCode_t
tmbslTDA10025_SetFECMode
(
    tmUnitSelect_t      tUnit,
    tmFrontEndFECMode_t eMode
);

/**
    \brief  Get the FEC mode

    \param  tUnit Demodulator unit number
    \param  peMode Pointer to the FEC mode (AnnexA, AnnexB)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                 
    \note   AnnexA is DVB-C (or J83A)
    \note   AnnexB is J83B

    \sa     tmbslTDA10025_SetFECMode
 */
extern tmErrorCode_t
tmbslTDA10025_GetFECMode
(
    tmUnitSelect_t       tUnit,
    tmFrontEndFECMode_t *peMode
);

/**
    \brief  Get the AGC control value used to set the amplifier gain

    \param  tUnit Demodulator unit number
    \param  puIFAGC Pointer to the IF AGC value
    \param  puRFAGC Pointer to the RF AGC value

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   Those values are automatically updated by the hardware
    \note   while controlling the tuner AGC loops

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetAGC
(
    tmUnitSelect_t  tUnit,
    UInt16         *puIFAGC,
    UInt16         *puRFACC
);

/**
    \brief  Set the IF AGC threshold

    \param  tUnit Demodulator unit number
    \param  uIFAGCthreshold IF AGC threshold

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   This threshold is used to optimise the ADC input dynamic
    \note   in order to avoid signal data loss due to saturation
    \note   in case of sudden amplitude increase (crest factor)

    \sa     tmbslTDA10025_GetIFAGCThreshold
 */
extern tmErrorCode_t
tmbslTDA10025_SetIFAGCThreshold
(
    tmUnitSelect_t tUnit,
    UInt16         uIFAGCthreshold
);

/**
    \brief  Gets the IF AGC threshold

    \param  tUnit Demodulator unit number
    \param  puIFAGCthreshold Pointer to the IF AGC threshold

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
    \return TDA10025_ERR_NOT_READY  The action can not be performed yet

    \note   NA

    \sa     tmbslTDA10025_SetIFAGCThreshold
 */
extern tmErrorCode_t
tmbslTDA10025_GetIFAGCThreshold
(
    tmUnitSelect_t  tUnit,
    UInt16         *puIFAGCthreshold
);

/**
    \brief  Set the RF AGC threshold

    \param  tUnit Demodulator unit number
    \param  uRFAGCthreshold RF AGC threshold

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   This threshold is used to optimise the ADC input dynamic
    \note   in order to avoid signal data loss due to saturation
    \note   in case of sudden amplitude increase (crest factor)

    \sa     tmbslTDA10025_GetRFAGCThreshold
 */
extern tmErrorCode_t
tmbslTDA10025_SetRFAGCThreshold
(
    tmUnitSelect_t tUnit,
    UInt16         uRFAGCthreshold
);

/**
    \brief  Get the RF AGC threshold

    \param  tUnit Demodulator unit number
    \param  puRFAGCthreshold Pointer to RF AGC threshold

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     tmbslTDA10025_SetRFAGCThreshold
 */
extern tmErrorCode_t
tmbslTDA10025_GetRFAGCThreshold
(
    tmUnitSelect_t  tUnit,
    UInt16         *puRFAGCthreshold
);

/**
    \brief  Set the IF (Intermediate Frequency)

    \param  tUnit Demodulator unit number
    \param  uIF IF (in Hz)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   The IF value must match the expected IF frequency coming from the tuner

    \sa     tmbslTDA10025_GetConfiguredIF
    \sa     tmbslTDA10025_GetIFOffset
 */
extern tmErrorCode_t
tmbslTDA10025_SetIF
(
    tmUnitSelect_t tUnit,
    UInt32         uIF
);

/**
    \brief  Get the programmed IF

    \param  tUnit Demodulator unit number
    \param  puIF Pointer to the programmed IF (in Hz)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     tmbslTDA10025_SetIF
    \sa     tmbslTDA10025_GetIFOffset
 */
extern tmErrorCode_t
tmbslTDA10025_GetConfiguredIF
(
    tmUnitSelect_t  tUnit,
    UInt32         *puIF
);

/**
    \brief  Get the IF offset between the actual IF and the configured IF value

    \param  tUnit Demodulator unit number
    \param  plIFOffset Pointer IF offset (in Hz)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     tmbslTDA10025_SetIF
    \sa     tmbslTDA10025_GetConfiguredIF
 */
extern tmErrorCode_t
tmbslTDA10025_GetIFOffset
(
    tmUnitSelect_t  tUnit,
    Int32          *plIFOffset
);

/**
    \brief  Get lock indicators (for debug purpose)

    \param  tUnit Demodulator unit number
    \param  psLockInd Pointer lock indicators

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   the indicators are:
    \note   CAGC Sat lock,
    \note   DAGC Sat lock,
    \note   AAGC lock,
    \note   STL lock,
    \note   CTL lock,
    \note   SCTL lock,
    \note   Demod lock,
    \note   FEC sync,
    \note   Demod lock time,
    \note   FEC lock time

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetLockInd
(
    tmUnitSelect_t      tUnit,
    TDA10025_LockInd_t *psLockInd
);

/**
    \brief  Get the signal quality indicator

    \param  tUnit Demodulator unit number
    \param  plSignalQuality Pointer to the signal quality indicator (0 to 100)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   Signal quality = 0: signal is unlocked
    \note   Signal quality = 10: uncor are present
    \note   Signal quality = 20: BER >  10 exp -2
    \note   Signal quality = 100: BER equals 0 (best quality)

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetSignalQuality
(
    tmUnitSelect_t  tUnit,
    UInt32         *pulSignalQuality
);

/**
    \brief  Get the estimation of the signal to noise ratio (Es/No)

    \param  tUnit Demodulator unit number
    \param  psEsno Pointer to the channel Es/No

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetChannelEsno
(
    tmUnitSelect_t        tUnit,
    tmFrontEndFracNb32_t *psEsno
);

/**
    \brief  Get an estimation of the signal to inter symbol interference ratio

    \param  tUnit Demodulator unit number    
    \param  psSignalToISI Pointer to the signal to ISI ratio (10 x value in in dB)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
    \return TDA10025_ERR_DRIVER_RETURN an error has been reported while starting the action

    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetSignalToISI
(
    tmUnitSelect_t        tUnit,
    tmFrontEndFracNb32_t *psSignalToISI
);

/**
    \brief  Get an estimation of the signal to inband interferer ratio

    \param  tUnit Demodulator unit number
    \param  psSignalToInterference Pointer to the signal to interference ratio (in dB)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
    \return TDA10025_ERR_DRIVER_RETURN an error has been reported while starting the action

    \note   NA

    \sa     NA
 */
extern tmErrorCode_t
tmbslTDA10025_GetSignalToInterference
(
    tmUnitSelect_t        tUnit,
    tmFrontEndFracNb32_t *psSignalToInterference
);

/**
    \brief  Set the TSMF mode

    \param  tUnit Demodulator unit number    
    \param  sTsmfMode TSMF mode (Disable, Forced or Auto)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_VERSION This hardware version is not supported
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     
 */
extern tmErrorCode_t
tmbslTDA10025_SetTsmfMode
(
    tmUnitSelect_t       tUnit,
    TDA10025_TsmfMode_t  eTsmfMode
);

/**
    \brief  Get the TSMF mode

    \param  tUnit Demodulator unit number    
    \param  psTsmfMode Pointer to the TSMF mode (Disable, Forced or Auto)

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_VERSION This hardware version is not supported
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   NA

    \sa     
 */
extern tmErrorCode_t
tmbslTDA10025_GetTsmfMode
(
    tmUnitSelect_t       tUnit,
    TDA10025_TsmfMode_t *psTsmfMode
);

/**
    \brief  Get TSMF status

    \param  tUnit Demodulator unit number    
    \param  psTsmfStatus Pointer to the TSMF status

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_VERSION This hardware version is not supported
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   status matches to register names register 0x5

    \sa     
 */
extern tmErrorCode_t
tmbslTDA10025_GetTsmfStatus
(
    tmUnitSelect_t       tUnit,
    TDA10025_TsmfStatus_t *psTsmfStatus
);

/**
    \brief  Get available TS IDs

    \param  tUnit Demodulator unit number    
    \param  psTsmfAvailableTs Pointer to the available TS (15 elements) 

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_VERSION This hardware version is not supported
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   avalaible TS is made of TS_ID, ON_ID (Original network ID), TS_Status, Rcv_Status
    \note   WARNING psTsmfAvailableTs[0] matches with TS number 1

    \sa     
 */
extern tmErrorCode_t
tmbslTDA10025_GetTsmfAvailableTsId
(
    tmUnitSelect_t              tUnit,
    TDA10025_TsmfAvailableTs_t *psTsmfAvailableTs
);

/**
    \brief  Select one of the available TS IDs

    \param  tUnit Demodulator unit number    
    \param  uTsID Index of the selected TsId (between 1 and 15) 

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_VERSION This hardware version is not supported
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   

    \sa     
 */
extern tmErrorCode_t
tmbslTDA10025_SelectTsmfTsId
(
    tmUnitSelect_t   tUnit,
    UInt32           uTsId
);

/**
    \brief  Configure the XTAL frequency

    \param  tUnit Demodulator unit number    
    \param  eXtalFrequency Xtal frequency

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_VERSION This hardware version is not supported
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   

    \sa     
 */
extern tmErrorCode_t
tmbslTDA10025_SetPllConfig
(
    tmUnitSelect_t   tUnit,
    TDA10025_XtalFreq_t eXtalFrequency
);

/**
    \brief  Retrieve the XTAL frequency

    \param  tUnit Demodulator unit number    
    \param  peXtalFrequency Xtal frequency

    \return TM_OK Succeed
    \return TDA10025_ERR_NOT_INITIALIZED This unit is not initialised
    \return TDA10025_ERR_BAD_VERSION This hardware version is not supported
    \return error codes of the functions registered in tmbslTDA10025_Open
    \return TDA10025_ERR_NULL_CONTROLFUNC External functions are not properly registered (tmbslTDA10025_Open)
                     
    \note   

    \sa     
 */
extern tmErrorCode_t
tmbslTDA10025_GetPllConfig
(
    tmUnitSelect_t   tUnit,
    TDA10025_XtalFreq_t *peXtalFrequency
);

/* backward compatibility */
#define tmbslTDA10025_Init   tmbslTDA10025_Open
#define tmbslTDA10025_DeInit tmbslTDA10025_Close
#define tmbslTDA10025_Reset  tmbslTDA10025_HwInit

#ifdef __cplusplus
}
#endif

#endif // TM<MODULE>_H //---------------
