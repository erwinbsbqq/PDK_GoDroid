/**
$Header: 
(C) Copyright 2007 NXP Semiconductors, All rights reserved

This source code and any compilation or derivative thereof is the sole
property of NXP Corporation and is provided pursuant to a Software
License Agreement.    This code is the proprietary information of NXP
Corporation and is confidential in nature.    Its use and dissemination by
any party other than NXP Corporation is strictly limited by the
confidential information provisions of the Agreement referenced above.
-----------------------------------------------------------------------------
FILE NAME:        tmbslTDA10025.c

DESCRIPTION:    Function for the silicon demodulator TDA10025

DOCUMENT REF: <References to specification or other documents related to
              this module>

NOTES:
*/

/*============================================================================*/
/*                   STANDARD INCLUDE FILES                                   */
/*============================================================================*/
/*
tmCompId                (def errors)
nxtypes                 (standard types)
tmflags                 (dependance machine)
tmbslFrontEndCfgItem.h  (enum config)
tmFrontend.h            (common enum)
tmbslfrontend.h         (def struct init)
tmbslFrontEndTypes.h    (bsl commo, types)
*/

#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"

/* TDA10025 Driver includes */
#include "tmbslHCDP.h"
#include "tmbslHCDP_Advanced.h"

#include "tmbslTDA10025.h"
#include "tmbslTDA10025local.h"
#include "tmbslTDA10025_Cfg.h"

#include "tmbslTDA10025Instance.h"

/*============================================================================*/
/*                   PROJECT INCLUDE FILES                                    */
/*============================================================================*/

/*============================================================================*/
/*                   MACRO DEFINITION                                         */
/*============================================================================*/
/*#define NO_FLOAT*/
#define SEND_TRACEFCT1(x,y)
#define SEND_TRACEFCT2(x,y)
#define SEND_TRACEFCT3(x,y)
#define SEND_TRACEFCT4(x,y)
#define SEND_TRACE(x,y) 

#define BITS32_MAX 0xFFFFFFFF
#define NO_FLOAT

/*============================================================================*/
/*                   TYPE DEFINITION                                          */
/*============================================================================*/


/*============================================================================*/
/*                   PUBLIC VARIABLES DEFINITION                              */
/*============================================================================*/


/*============================================================================*/
/*                   STATIC VARIABLES DECLARATIONS                            */
/*============================================================================*/
static TDA10025_LogTable_t gTDA10025LogTable[] = { TDA10025_LOG_TABLE };

/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/


/*============================================================================*/
/*                   STATIC FUNCTIONS DECLARATIONS                            */
/*============================================================================*/
static tmErrorCode_t iTDA10025_HwInit( tmUnitSelect_t tUnit );
static tmErrorCode_t iTDA10025_ResetCdp( tmUnitSelect_t tUnit );
static tmErrorCode_t iTDA10025_InitCdp( tmUnitSelect_t tUnit );

static tmUnitSelect_t iTDA10025_GetUnit( tmUnitSelect_t tUnit );

static tmErrorCode_t iTDA10025_IPRead(tmUnitSelect_t tUnit, UInt32 AddrSize, UInt8* pAddr, UInt32 ReadLen, UInt8* pData);
static tmErrorCode_t iTDA10025_IPWrite(tmUnitSelect_t tUnit, UInt32 AddrSize, UInt8* pAddr, UInt32 WriteLen, UInt8* pData);

static tmErrorCode_t iTDA10025_TsmfWrite(tmUnitSelect_t tUnit, UInt16 uIndex_U, UInt32 uNBytes_U, UInt16* puData_U);
static tmErrorCode_t iTDA10025_TsmfWriteBit(tmUnitSelect_t tUnit, UInt16 uIndex_U, UInt16 uMask_U, UInt16 uData_U);
static tmErrorCode_t iTDA10025_TsmfRead(tmUnitSelect_t tUnit, UInt16 uIndex_U, UInt32 uNBytes_U, UInt16* puData_U);

static tmErrorCode_t iTDA10025_ConfigurePLL(tmUnitSelect_t tUnit);
tmErrorCode_t iTDA10025_CalculateHCDPClockParam(pTDA10025Object_t pObj, UInt32 *puSamplingClock, UInt32 *puDClock);
static tmErrorCode_t iTDA10025_EnablePLL(tmUnitSelect_t tUnit);
static tmErrorCode_t iTDA10025_DisablePLL(tmUnitSelect_t tUnit);
tmErrorCode_t iTDA10025_SetPowerState(tmUnitSelect_t tUnit, tmPowerState_t ePowerState);

static tmErrorCode_t iTDA10025_MutexInitDummy(ptmbslFrontEndMutexHandle *ppMutexHandle);
static tmErrorCode_t iTDA10025_MutexDeInitDummy(ptmbslFrontEndMutexHandle pMutexHandle);
static tmErrorCode_t iTDA10025_MutexAcquireDummy(ptmbslFrontEndMutexHandle pMutexHandle, UInt32 timeOut);
static tmErrorCode_t iTDA10025_MutexReleaseDummy(ptmbslFrontEndMutexHandle pMutexHandle);

static UInt32 iTDA10025_Log (UInt32 ulValue);

static tmErrorCode_t iTDA10025_SetTsmfMode( tmUnitSelect_t tUnit, TDA10025_TsmfMode_t eTsmfMode);
static tmErrorCode_t iTDA10025_GetTsmfMode( tmUnitSelect_t tUnit, TDA10025_TsmfMode_t *peTsmfMode);
static tmErrorCode_t iTDA10025_GetTsmfStatus( tmUnitSelect_t tUnit, TDA10025_TsmfStatus_t *psTsmfStatus );
static tmErrorCode_t iTDA10025_GetTsmfAvailableTsId( tmUnitSelect_t tUnit, TDA10025_TsmfAvailableTs_t *psTsmfAvailableTs );
static tmErrorCode_t iTDA10025_SelectTsmfTsId( tmUnitSelect_t tUnit, UInt32 uTsID );

tmErrorCode_t iTDA10025_IQPreset( tmUnitSelect_t tUnit, TDA10025_ConstellationSource_t eSource );

extern tmErrorCode_t iTDA10025_SetPathConfiguration( tmUnitSelect_t tUnit, TDA10025_IF_t eIF, TDA10025_CDP_t eCDP, TDA10025_TS_t eTS );

#ifdef TMBSL_TDA10025_NO_64BITS
static UInt32 iTDA10025_Division32(UInt32 ulNum1, UInt32 ulNum2, UInt16 ulDenom);
#endif

static tmErrorCode_t iTDA10025SetPllConfig(pTDA10025Object_t pObj, TDA10025_XtalFreq_t uXtal);

/*============================================================================*/
/*                   PUBLIC FUNCTIONS DEFINITIONS                             */
/*============================================================================*/
 
/*============================================================================*/
/* tmbslTDA10025_Open                                                         */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_Open
(
    tmUnitSelect_t             tUnit,
    tmbslFrontEndDependency_t *psSrvFunc
)
{
    pTDA10025Object_t          pObj = Null;
    pTDA10025Object_t          pObjOtherStream = Null;
    tmErrorCode_t              err = TM_OK;
    tmbslFrontEndDependency_t  sHCDPSrvFunc;

    tmUnitSelect_t             tUnitOtherStream;

    /* Test parameter(s) */
    if( (psSrvFunc == Null) || (psSrvFunc->dwAdditionalDataSize != sizeof(TDA10025_AdditionnalData_t)) ) 
    {
        err = TDA10025_ERR_BAD_PARAMETER;
    }

    if (err == TM_OK)
    {
        /* check input parameters */
        err = iTDA10025_GetInstance(tUnit, &pObj);
    }

    /* check driver state */
    if (err == TM_OK || err == TDA10025_ERR_NOT_INITIALIZED)
    {
        if (pObj != Null && pObj->init == True)
        {
            err = TDA10025_ERR_NOT_INITIALIZED;
        }
        else 
        {
            /* initialize the Object */
            if (pObj == Null)
            {
                /* Try to allocate an instance of the driver */
                err = iTDA10025_AllocInstance(tUnit, &pObj);
                if (err != TM_OK || pObj == Null)
                {
                    err = TDA10025_ERR_NOT_INITIALIZED;        
                }
            }

            if (err == TM_OK)
            {
                /* Initialize the Object by default values */
                P_SIO = P_FUNC_SIO(psSrvFunc);
                P_STIME = P_FUNC_STIME(psSrvFunc);
                P_SDEBUG = P_FUNC_SDEBUG(psSrvFunc);

#ifdef _TVFE_IMPLEMENT_MUTEX
                if(    P_FUNC_SMUTEX_OPEN_VALID(psSrvFunc)
                    && P_FUNC_SMUTEX_CLOSE_VALID(psSrvFunc)
                    && P_FUNC_SMUTEX_ACQUIRE_VALID(psSrvFunc)
                    && P_FUNC_SMUTEX_RELEASE_VALID(psSrvFunc) )
                {
                    P_SMUTEX = psSrvFunc->sMutex;

                    err = P_SMUTEX_OPEN(&P_MUTEX);
                    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "Mutex_Open(0x%08X) failed.", tUnit));
                }
#endif

                // initialise common path tUnit
                pObj->tUnitCommon = ((TDA10025_AdditionnalData_t*)psSrvFunc->pAdditionalData)->tUnitCommon;

                // initialise path type
                pObj->eChannelSel = ((TDA10025_AdditionnalData_t*)psSrvFunc->pAdditionalData)->eChannelSel;
            }
 
            if(err == TM_OK)
            {
                /* configure HCDP instance */
                sHCDPSrvFunc.sDebug    = psSrvFunc->sDebug;
                /* mutex are not used in HCDP (TDA10025 mutex are used instead) */
                sHCDPSrvFunc.sMutex.Init = iTDA10025_MutexInitDummy;
                sHCDPSrvFunc.sMutex.DeInit = iTDA10025_MutexDeInitDummy;
                sHCDPSrvFunc.sMutex.Acquire = iTDA10025_MutexAcquireDummy;
                sHCDPSrvFunc.sMutex.Release = iTDA10025_MutexReleaseDummy;
                sHCDPSrvFunc.sTime     = psSrvFunc->sTime;
                sHCDPSrvFunc.sIo.Read  = iTDA10025_IPRead;
                sHCDPSrvFunc.sIo.Write = iTDA10025_IPWrite;

                err = tmbslHCDP_Init(tUnit, &sHCDPSrvFunc);
            }

            tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_Open(0x%08X)", tUnit);

            if(err == TM_OK)
            {
                pObj->init = True;
            }

            if( ( err == TM_OK ) && ((TDA10025_AdditionnalData_t*)psSrvFunc->pAdditionalData)->eChannelSel == TDA10025_ChannelSelectionIpA_SinglePath )
            {
                /* in case of one path only, allocate the other path in order to reset the HW correctly */
                /* this allocation is accessible only internally */
                tUnitOtherStream = tUnit+0x1;

                /* allocate the other path */
                err = iTDA10025_AllocInstance( tUnitOtherStream, &pObjOtherStream );

                if (err != TM_OK || pObjOtherStream == Null)
                {
                    err = TDA10025_ERR_NOT_INITIALIZED;        
                }
                else
                {
                    /* indicate to main path the tUnit of other path */
                    /* this is required to perform the initialisation correctly */
                    pObj->tUnitOtherStream = tUnitOtherStream;
                }

                /* initialise  */
                pObjOtherStream->sIo = psSrvFunc->sIo;
                pObjOtherStream->sTime = psSrvFunc->sTime;
                pObjOtherStream->sDebug = psSrvFunc->sDebug;

#ifdef _TVFE_IMPLEMENT_MUTEX
                if(    P_FUNC_SMUTEX_OPEN_VALID(psSrvFunc)
                    && P_FUNC_SMUTEX_CLOSE_VALID(psSrvFunc)
                    && P_FUNC_SMUTEX_ACQUIRE_VALID(psSrvFunc)
                    && P_FUNC_SMUTEX_RELEASE_VALID(psSrvFunc) )
                {
                    P_SMUTEX = psSrvFunc->sMutex;

                    if( (psSrvFunc->sMutex.Init != Null) &&
                        (psSrvFunc->sMutex.DeInit != Null) &&
                        (psSrvFunc->sMutex.Acquire != Null) &&
                        (psSrvFunc->sMutex.Release != Null) )
                    {
                        pObjOtherStream->sMutex = psSrvFunc->sMutex;
                        err = pObjOtherStream->sMutex.Init( &pObjOtherStream->pMutex);
                        tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "Mutex_Open(0x%08X) failed.", pObj->tUnitOtherStream));
                    }
                }
#endif

                // initialise common path tUnit
                pObjOtherStream->tUnitCommon = ((TDA10025_AdditionnalData_t*)psSrvFunc->pAdditionalData)->tUnitCommon;

                // initialise path type
                pObjOtherStream->eChannelSel = ((TDA10025_AdditionnalData_t*)psSrvFunc->pAdditionalData)->eChannelSel;

                if(err == TM_OK)
                {
                    /* configure HCDP instance */
                    sHCDPSrvFunc.sDebug    = psSrvFunc->sDebug;
                    /* mutex are not used in HCDP (TDA10025 mutex are used instead) */
                    sHCDPSrvFunc.sMutex.Init = iTDA10025_MutexInitDummy;
                    sHCDPSrvFunc.sMutex.DeInit = iTDA10025_MutexDeInitDummy;
                    sHCDPSrvFunc.sMutex.Acquire = iTDA10025_MutexAcquireDummy;
                    sHCDPSrvFunc.sMutex.Release = iTDA10025_MutexReleaseDummy;
                    sHCDPSrvFunc.sTime     = psSrvFunc->sTime;
                    sHCDPSrvFunc.sIo.Read  = iTDA10025_IPRead;
                    sHCDPSrvFunc.sIo.Write = iTDA10025_IPWrite;

                    err = tmbslHCDP_Init(pObj->tUnitOtherStream, &sHCDPSrvFunc);
                }

                tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_Open(0x%08X)", tUnit);

                if(err == TM_OK)
                {
                    pObjOtherStream->init = True;
                }
            }
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_Open(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_Close                                                        */
/*============================================================================*/
tmErrorCode_t 
tmbslTDA10025_Close
(
    tmUnitSelect_t tUnit
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    if(err == TM_OK)
    {
#ifdef _TVFE_IMPLEMENT_MUTEX
        /* Try to acquire driver mutex */
        err = iTDA10025_MutexAcquire(pObj, TDA10025_MUTEX_TIMEOUT);

        if(err == TM_OK)
        {
#endif
            tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_Close(0x%08X)", tUnit);

            if( pObj->eChannelSel == TDA10025_ChannelSelectionIpA_SinglePath )
            {
                /* in case of single path, 2 instances have been allocated */
                /* release the other stream one first */

                /* terminate HCDP */
                err = tmbslHCDP_DeInit( pObj->tUnitOtherStream );

                if (err == TM_OK)
                {
                    err = iTDA10025_DeAllocInstance( pObj->tUnitOtherStream );
                }
            }

            /* terminate HCDP */
            if (err == TM_OK)
            {
                err = tmbslHCDP_DeInit(tUnit);
            }

#ifdef _TVFE_IMPLEMENT_MUTEX
        }

        if(err == TM_OK)
        {
            P_SMUTEX_ACQUIRE = Null;

            /* Release driver mutex */
            (void)iTDA10025_MutexRelease(pObj);

            if(P_SMUTEX_CLOSE_VALID && P_MUTEX_VALID)
            {
                err = P_SMUTEX_CLOSE(P_MUTEX);
            }

            P_SMUTEX_OPEN = Null;
            P_SMUTEX_CLOSE = Null;
            P_SMUTEX_RELEASE = Null;

            P_MUTEX = Null;
        }
#endif
        if (err == TM_OK)
        {
            err = iTDA10025_DeAllocInstance(tUnit);
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_Close(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetSWVersion                                                 */
/*============================================================================*/
tmErrorCode_t     
tmbslTDA10025_GetSWVersion
(
    ptmSWVersion_t      pSWVersion
)
{
    pSWVersion->compatibilityNr = TDA10025_BSL_COMP_NUM;
    pSWVersion->majorVersionNr  = TDA10025_BSL_MAJOR_VER;
    pSWVersion->minorVersionNr  = TDA10025_BSL_MINOR_VER;

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA10025_GetSWSettingsVersion                                         */
/*============================================================================*/
tmErrorCode_t     
tmbslTDA10025_GetSWSettingsVersion
(
    ptmSWSettingsVersion_t      pSWSettingsVersion
)
{
    pSWSettingsVersion->customerNr      = TDA10025_SETTINGS_CUSTOMER_NUM;
    pSWSettingsVersion->projectNr       = TDA10025_SETTINGS_PROJECT_NUM;
    pSWSettingsVersion->majorVersionNr  = TDA10025_SETTINGS_MAJOR_VER;
    pSWSettingsVersion->minorVersionNr  = TDA10025_SETTINGS_MINOR_VER;

    return TM_OK;
}

/*============================================================================*/
/* tmbslTDA10025_HwInit                                                       */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_HwInit
(
    tmUnitSelect_t tUnit    /* I: Unit number */
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_HwInit(0x%08X)", tUnit);
    
    if( err == TM_OK )
    {
        err = iTDA10025_HwInit( tUnit );
    }

    if( (err == TM_OK) && (pObj->tUnitOtherStream != -1) )
    {
        err = iTDA10025_HwInit( pObj->tUnitOtherStream );
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_HwInit(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetPowerState                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetPowerState
(
    tmUnitSelect_t tUnit,
    tmPowerState_t ePowerState
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetPowerState(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        if( ePowerState < tmPowerMax )
        {
            switch (ePowerState)
            {
            case tmPowerOn:
                if (pObj->ePowerState != ePowerState) /* compare to current state */
                {
                    err = iTDA10025_SetPowerState(tUnit, ePowerState);
                }
                /* else nothing to do */
                break;
            case tmPowerStandby:
            case tmPowerSuspend:
            case tmPowerOff:
                if (pObj->ePowerState != ePowerState) /* compare to current state */
                {
                    err = iTDA10025_SetPowerState(tUnit, ePowerState);
                }
                /* else nothing to do */
                break;
            default:
                err = TDA10025_ERR_BAD_PARAMETER;
            }
        }
        else
        {
            err = TDA10025_ERR_NOT_INITIALIZED;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetPowerState(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)
    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetPowerState                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetPowerState
(
    tmUnitSelect_t tUnit,
    tmPowerState_t *pePowerState
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16              uValue = 0;
    UInt16              uMask = 0;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetPowerState(0x%08X)", tUnit);

    if (pObj->ePowerState == tmPowerOn)
    {
        if (pObj->sCfg.eCDP == TDA10025_CDP_0)
        {
            uMask = TDA10025_CLK_CDP0_CORE_MSK;
        }
        else
        {
            uMask = TDA10025_CLK_CDP1_CORE_MSK;
        }

        /* re-check the CDP power state in case of hardware reset or RESET_CONTROL */
        if (err == TM_OK)
        {
            /* check the CDP power state from clk_en */
            err = iTDA10025_Read(tUnit, TDA10025_CLK_EN_IND, 1, &uValue);
        }

        if (err == TM_OK)
        {
            if ((uValue & uMask) == 0x0)
            {
                pObj->ePowerState = tmPowerStandby;
            }
        }
    }

    if (err == TM_OK)
    {
        *pePowerState = pObj->ePowerState;
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetPowerState(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_StartLock                                                    */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_StartLock
(
    tmUnitSelect_t tUnit    /* I: Unit number */
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_StartLock(0x%08X)", tUnit);
    
    if ((err == TM_OK) && (pObj->ePowerState != tmPowerOn))
    {
        err = TDA10025_ERR_NOT_READY;
    }

    if (err == TM_OK)
    {    
        err = tmbslHCDP_StartLock(tUnit);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_StartLock(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetBER                                                       */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetBER
(
    tmUnitSelect_t  tUnit,
    tmFrontEndFracNb32_t *psBER,
    UInt32 *puUncors
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetBER(0x%08X)", tUnit);
    /* check driver state */
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetBER(tUnit, psBER, puUncors);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetBER(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetBERWindow                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetBERWindow
(
    tmUnitSelect_t  tUnit,
    TDA10025_BERWindow_t eBERWindow
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetBERWindow(0x%08X)", tUnit);
    
    /* check driver state */
    if (err == TM_OK)
    {
        err = tmbslHCDP_SetBERWindow(tUnit, (HCDP_BERWindow_t)eBERWindow);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetBERWindow(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetBERWindow                                                 */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetBERWindow
(
    tmUnitSelect_t  tUnit,
    TDA10025_BERWindow_t *peBERWindow
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetBERWindow(0x%08X)", tUnit);
    
    /* check driver state */
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetBERWindow(tUnit, (HCDP_BERWindow_t*)peBERWindow);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetBERWindow(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetLockStatus                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetLockStatus
(
    tmUnitSelect_t       tUnit,
    tmbslFrontEndState_t *peLockStatus
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetLockStatus(0x%08X)", tUnit);

    if (err == TM_OK)
    {
        if (pObj->ePowerState == tmPowerOn)
        {
            err = tmbslHCDP_GetLockStatus(tUnit, peLockStatus);
        }
        else
        {
            *peLockStatus = tmbslFrontEndStateNotLocked;
        }
    }
    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetLockStatus(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetSR                                                        */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetSR
(
    tmUnitSelect_t tUnit,
    UInt32         uSR
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    if( uSR == 0 ) 
    {
        err = TDA10025_ERR_BAD_PARAMETER;
    }

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetSR(0x%08X)", tUnit);

    if (err == TM_OK)
    {
        err = tmbslHCDP_SetSR(tUnit, uSR);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetSR(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetSR                                                        */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetSR
(
    tmUnitSelect_t  tUnit,
    UInt32         *puSR
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetSR(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err  = tmbslHCDP_GetSR(tUnit, puSR);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetSR(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetConfiguredSR                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetConfiguredSR
(
    tmUnitSelect_t tUnit,
    UInt32         *puSR
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetConfiguredSR(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err  = tmbslHCDP_GetConfiguredSR(tUnit, puSR);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetConfiguredSR(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetSI                                                        */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetSI
(
    tmUnitSelect_t      tUnit,
    tmFrontEndSpecInv_t eSI
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetSI(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        /* test the parameter value*/
        if( eSI >= tmFrontEndSpecInvMax )
        {
            err = TDA10025_ERR_BAD_PARAMETER;
        }
    }
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_SetSI(tUnit, eSI);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetSI(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetSI                                                        */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetSI
(
    tmUnitSelect_t      tUnit,
    tmFrontEndSpecInv_t *peSI
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetSI(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetSI(tUnit, peSI);
    }
    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetSI(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetConfiguredSI                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetConfiguredSI
(
    tmUnitSelect_t      tUnit,
    tmFrontEndSpecInv_t *peSI
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetSI(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetConfiguredSI(tUnit, peSI);
    }
    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetSI(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetMod                                                       */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetMod
(
    tmUnitSelect_t         tUnit,
    tmFrontEndModulation_t eMod
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
   
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)
    
    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetMod(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        /* test the parameter value */
        if (eMod >= tmFrontEndModulationMax)
        {
            err = TDA10025_ERR_BAD_PARAMETER;
        }

        if (err == TM_OK)
        {
            if (eMod == tmFrontEndModulationAuto)
            {
                err = TDA10025_ERR_NOT_SUPPORTED;
            }
        }

        if (err == TM_OK)
        {
            err = tmbslHCDP_SetMod(tUnit, eMod);
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetMod(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetMod                                                       */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetMod
(
    tmUnitSelect_t         tUnit,
    tmFrontEndModulation_t *peMod
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetMod(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetMod(tUnit, peMod);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetMod(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetFECMode                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetFECMode
(
    tmUnitSelect_t         tUnit,
    tmFrontEndFECMode_t    eMode
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetMod(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        /* test the parameter value */
        if (eMode >= tmFrontEndFECModeMax)
        {
            err = TDA10025_ERR_BAD_PARAMETER;
        }

        if (err == TM_OK)
        {

            err = tmbslHCDP_SetFECMode(tUnit, eMode);
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetMod(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetFECMode                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetFECMode
(
    tmUnitSelect_t         tUnit,
    tmFrontEndFECMode_t *peMode
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetFECMode(0x%08X)", tUnit);
    
    if( pObj->ePowerState == tmPowerOn )
    {
        if (err == TM_OK)
        {
            err = tmbslHCDP_GetFECMode(tUnit, peMode);
        }

        tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetFECMode(0x%08X) failed.", tUnit));
    }
    else
    {
        // this information is not avalaible when the system is stand-by
        err = TDA10025_ERR_NOT_INITIALIZED;
    }

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetAGC                                                       */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetAGC
(
    tmUnitSelect_t         tUnit,
    UInt16                 *puIFAGC,
    UInt16                 *puRFAGC

)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025GetAGCACC(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetAAGCAcc(tUnit, (UInt16*)puIFAGC, (UInt16*)puRFAGC);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025GetAGCACC(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetIFAGCThreshold                                            */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetIFAGCThreshold
(
    tmUnitSelect_t         tUnit,
    UInt16                 uIFAGCthreshold
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetIFAGCThreshold(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_SetIFAGCThreshold(tUnit, (UInt16)uIFAGCthreshold);
        
        if (err == TM_OK)
        {
            pObj->sIP.uIFAGCThreshold = uIFAGCthreshold;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetIFAGCThreshold(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetIFAGCThreshold                                            */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetIFAGCThreshold
(
    tmUnitSelect_t         tUnit,
    UInt16                 *puIFAGCthreshold
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetIFAGCThreshold(0x%08X)", tUnit);
    
    /* check state */
    if ((err == TM_OK) && (pObj->ePowerState != tmPowerOn))
    {
        err = TDA10025_ERR_NOT_READY;
    }

    if (err == TM_OK)
    {
        err = tmbslHCDP_GetIFAGCThreshold(tUnit, (UInt16*)puIFAGCthreshold);
        
        if (err == TM_OK)
        {
            pObj->sIP.uIFAGCThreshold = *puIFAGCthreshold;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetIFAGCThreshold(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetRFAGCThreshold                                            */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetRFAGCThreshold
(
    tmUnitSelect_t         tUnit,
    UInt16                 uRFAGCthreshold
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetRFAGCThreshold(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        /*
        err = tmbslHCDP_SetIFAGCThreshold(tUnit, (UInt16)uRFAGCthreshold);
        
        if (err == TM_OK)
        {
            pObj->sIP.uRFAGCThreshold = uRFAGCthreshold;
        }
        */
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetRFAGCThreshold(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetRFAGCThreshold                                            */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetRFAGCThreshold
(
    tmUnitSelect_t         tUnit,
    UInt16                 *puIFAGCthreshold
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetRFAGCThreshold(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        *puIFAGCthreshold = 0;  /* pObj->sIP.uIFAGCThreshold; */
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetRFAGCThreshold(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetIF                                                        */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetIF
(
    tmUnitSelect_t tUnit,   /* I: Unit number */
    UInt32         uIF      /* I: If in Herz  */
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetIF(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_SetIF(tUnit, uIF);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetIF(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetConfiguredIF                                              */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetConfiguredIF
(
    tmUnitSelect_t tUnit,   /* I: Unit number */
    UInt32         *puIF    /* I: If in Herz  */
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetConfiguredIF(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetConfiguredIF(tUnit, puIF);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetConfiguredIF(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}



/*============================================================================*/
/* tmbslTDA10025_GetIFOffset                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetIFOffset
(
    tmUnitSelect_t          tUnit,
    Int32                   *plIFOffset
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetIFOffset(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetIFOffset(tUnit, plIFOffset);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetIFOffset(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetLockInd                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetLockInd
(
    tmUnitSelect_t          tUnit,
    TDA10025_LockInd_t      *psLockInd
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    HCDP_LockInd_t     sLockInd;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025GetAFC(0x%08X)", tUnit);

    if (err == TM_OK)
    {
        err = tmbslHCDP_GetLockIndicators(tUnit, &sLockInd);
        if (err == TM_OK)
        {
            psLockInd->bCAGCSat = sLockInd.bCagcSat;
            psLockInd->bDAGCSat = sLockInd.bDagcSat;
            psLockInd->bAagc = sLockInd.bAagc;
            psLockInd->bCtl = sLockInd.bCtl;
            psLockInd->bDemod = sLockInd.bDemod;
            psLockInd->bFEC = sLockInd.bFEC;
            psLockInd->bSctl = sLockInd.bSctl;
            psLockInd->bStl = sLockInd.bStl;
            psLockInd->uDemodLockTime = sLockInd.uDemodLockTime;
            psLockInd->uFecLockTime = sLockInd.uFecLockTime;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025GetAFC(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetSignalQuality                                             */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetSignalQuality
(
    tmUnitSelect_t          tUnit,
    UInt32                 *pulSignalQuality
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    
    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetSignalQuality(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetSignalQuality(tUnit, pulSignalQuality);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetSignalQuality(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetChannelEsno                                               */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetChannelEsno
(
    tmUnitSelect_t          tUnit,
    tmFrontEndFracNb32_t    *psEsno
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetChannelEsno(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err  = tmbslHCDP_GetChannelEsNo(tUnit, psEsno);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetChannelEsno(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetSignalToISI                                               */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetSignalToISI
(
    tmUnitSelect_t          tUnit,
    tmFrontEndFracNb32_t    *psSignalToISI
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16              SignalToISI;
    UInt16              EqCenterTapGain;

    UInt32 ulLogValue = 0;
    Int32  lInteger;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

        tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetSignalToISI(0x%08X)", tUnit);

    if (err == TM_OK)
    {
        if (pObj->ePowerState == tmPowerOn)
        {
            err = tmbslHCDP_GetSignalToISI(tUnit, &SignalToISI, &EqCenterTapGain);

            if (err == TM_OK)
            {
                if(SignalToISI != 0)
                {
                    ulLogValue = iTDA10025_Log( SignalToISI );
                    lInteger = -1 * ulLogValue;
                    lInteger += (602 * EqCenterTapGain);
                    lInteger += 5418;

                    psSignalToISI->lInteger = (Int32)lInteger;
                    psSignalToISI->uDivider = 100;       
                }
                else
                {
                    psSignalToISI->lInteger = 100;
                    psSignalToISI->uDivider = 100;       
                }
            }

            else
            {
                err = TDA10025_ERR_DRIVER_RETURN;
            }
        }
        else
        {
            err = TDA10025_ERR_NOT_READY;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetSignalToISI(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetSignalToInterference                                      */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetSignalToInterference
(
    tmUnitSelect_t          tUnit,
    tmFrontEndFracNb32_t    *psSignalToInterference
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16              SignalToInterference;
    UInt16              EqCenterTapGain;

    UInt32 ulLogValue = 0;
    Int32  lInteger;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetSignalToInterference(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        if (pObj->ePowerState == tmPowerOn)
        {
            err = tmbslHCDP_GetSignalToInterference(tUnit, &SignalToInterference, &EqCenterTapGain);

            if (err == TM_OK) 
            {
                if (SignalToInterference != 0)
                {
                    ulLogValue = iTDA10025_Log( SignalToInterference );
                    lInteger = -1 * ulLogValue;
                    lInteger += (602 * EqCenterTapGain);
                    lInteger += 5418;

                    psSignalToInterference->lInteger = (Int32)lInteger;
                    psSignalToInterference->uDivider = 100;       
                }
                else
                {
                    /* no interferences -> set to max value */
                    psSignalToInterference->lInteger = 100;
                    psSignalToInterference->uDivider = 1;       
                }
            }
            else
            {
                err = TDA10025_ERR_DRIVER_RETURN;
            }
        }
        else
        {
            err = TDA10025_ERR_NOT_READY;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetSignalToInterference(0x%08X) failed.", tUnit));
    
    _MUTEX_RELEASE(TDA10025)

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_ClearUncor                                                   */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_ClearUncor
(
    tmUnitSelect_t       tUnit
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_ClearUncor(0x%08X)", tUnit);

    if (err == TM_OK)
    {
        err = tmbslHCDP_ClearUncor(tUnit);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_ClearUncor(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)
    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetTsmfMode                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SetTsmfMode
(
    tmUnitSelect_t       tUnit,
    TDA10025_TsmfMode_t  eTsmfMode
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetTsmfMode(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        if (pObj->ePowerState == tmPowerOn)
        {
            err = iTDA10025_SetTsmfMode( tUnit, eTsmfMode );
        }
        else
        {
            /* indicate that this API can not be called in standby */
            err = TDA10025_ERR_NOT_INITIALIZED;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetTsmfMode(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)
    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetTsmfMode                                                  */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetTsmfMode
(
    tmUnitSelect_t       tUnit,
    TDA10025_TsmfMode_t *peTsmfMode
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetTsmfMode(0x%08X)", tUnit);

    if (err == TM_OK)
    {
        if (pObj->ePowerState == tmPowerOn)
        {
            err = iTDA10025_GetTsmfMode( tUnit, peTsmfMode);
        }
        else
        {
            /* indicate that this API can not be called in standby */
            err = TDA10025_ERR_NOT_INITIALIZED;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetTsmfMode(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)
    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetTsmfStatus                                                */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetTsmfStatus
(
    tmUnitSelect_t         tUnit,
    TDA10025_TsmfStatus_t *psTsmfStatus
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetTsmfStatus(0x%08X)", tUnit);

    if (err == TM_OK)
    {
        if (pObj->ePowerState == tmPowerOn)
        {
            err = iTDA10025_GetTsmfStatus( tUnit, psTsmfStatus);
        }
        else
        {
            /* indicate that this API can not be called in standby */
            err = TDA10025_ERR_NOT_INITIALIZED;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetTsmfStatus(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)
    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetTsmfAvailableTsId                                         */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_GetTsmfAvailableTsId
(
    tmUnitSelect_t              tUnit,
    TDA10025_TsmfAvailableTs_t *psTsmfAvailableTs
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetPLL(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        if (pObj->ePowerState == tmPowerOn)
        {
            iTDA10025_GetTsmfAvailableTsId( tUnit, psTsmfAvailableTs );
        }
        else
        {
            /* indicate that this API can not be called in standby */
            err = TDA10025_ERR_NOT_INITIALIZED;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetPLL(0x%08X) failed.", tUnit));

    _MUTEX_RELEASE(TDA10025)
    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SelectTsmfTsId                                               */
/*============================================================================*/
tmErrorCode_t
tmbslTDA10025_SelectTsmfTsId
(
    tmUnitSelect_t   tUnit,
    UInt32           uTsId       
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SelectTsmfTsId(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        if (pObj->ePowerState == tmPowerOn)
        {
            iTDA10025_SelectTsmfTsId( tUnit, uTsId);
        }
        else
        {
            /* indicate that this API can not be called in standby */
            err = TDA10025_ERR_NOT_INITIALIZED;
        }
    }

    _MUTEX_RELEASE(TDA10025)

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SelectTsmfTsId(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_SetPllConfig                                                 */
/*============================================================================*/
extern tmErrorCode_t
tmbslTDA10025_SetPllConfig
(
    tmUnitSelect_t   tUnit,
    TDA10025_XtalFreq_t eXtalFrequency
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_SetPllConfig(0x%08X)", tUnit);
    
    if( err == TM_OK )
    {
        /* retrieve state and set to power on if required */
        tmPowerState_t         ePowerState = pObj->ePowerState;

        if( (ePowerState != tmPowerOn) && (ePowerState != tmPowerMax) )
        {
            err = iTDA10025_SetPowerState(tUnit, tmPowerOn);
        }

        /* configure the PLL */
        if( err == TM_OK )
        {
            err = iTDA10025SetPllConfig(pObj, eXtalFrequency);
        }

        /* come back to initial value */
        if ( (ePowerState != tmPowerOn) && (ePowerState != tmPowerMax) )
        {
            err = iTDA10025_SetPowerState(tUnit, tmPowerStandby);
        }
    }

    _MUTEX_RELEASE(TDA10025)

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_SetPllConfig(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* tmbslTDA10025_GetPllConfig                                                 */
/*============================================================================*/
extern tmErrorCode_t
tmbslTDA10025_GetPllConfig
(
    tmUnitSelect_t   tUnit,
    TDA10025_XtalFreq_t *peXtalFrequency
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;
    int index;
    int arraySize;
    Bool freqFound = False;
    TDA10025PllConfig_t *pPllConfig;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);
    _MUTEX_ACQUIRE(TDA10025)

    tmDBGPRINTEx(DEBUGLVL_INOUT, "tmbslTDA10025_GetPllConfig(0x%08X)", tUnit);
    
    if( err == TM_OK )
    {
        /* check that current Xtal freq exist */
        switch( pObj->sPllConfig.uXtal )
        {
        case 16000000:
            *peXtalFrequency = TDA10025_XtalFreq_16MHz;
            break;
        case 27000000:
            *peXtalFrequency = TDA10025_XtalFreq_27MHz;
            break;
        case 25000000:
            *peXtalFrequency = TDA10025_XtalFreq_25MHz;
            break;
        default:
            // if not a predefined value
            *peXtalFrequency = TDA10025_XtalFreq_Invalid;
            break;
        }

        /* check M, N, P parameters are matching */
        if (*peXtalFrequency != TDA10025_XtalFreq_Invalid)
        {
            /* retrieve possible configuration, depending on ES version */
            switch (pObj->eHwSample)
            {
            case TDA10025_HwSampleVersion_ES1:
                arraySize = TDA18265_ES1_PLL_CONFIG_NB;
                pPllConfig = ES1PllSettings;
                break;
            case TDA10025_HwSampleVersion_ES2:
                arraySize = TDA18265_ES2_PLL_CONFIG_NB;
                pPllConfig = ES2PllSettings;
                break;
            default:
                err = TDA10025_ERR_BAD_VERSION;
            }

            if (err == TM_OK)
            {
                index = 0;
                do {
                    if( pPllConfig[index].uXtal == pObj->sPllConfig.uXtal )
                    {
                        freqFound = True;
                    }
                    index ++;
                } while ((index < arraySize) && (freqFound == False));

                if( freqFound == True )
                {
                    index--;

                    if( (pObj->sPllConfig.uPLLMFactor != pPllConfig[index].uPLLMFactor) ||
                        (pObj->sPllConfig.lPLLNFactor != pPllConfig[index].lPLLNFactor) ||
                        (pObj->sPllConfig.bPLLPFactor != pPllConfig[index].bPLLPFactor) ||
                        (pObj->sPllConfig.ePllMode    != pPllConfig[index].ePllMode) )
                    {
                        // parameters does not match
                        *peXtalFrequency = TDA10025_XtalFreq_Invalid;
                    }
                }
                else
                {
                    // configuration has not been found
                    *peXtalFrequency = TDA10025_XtalFreq_Invalid;
                }
            }
        }
    }

    _MUTEX_RELEASE(TDA10025)

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "tmbslTDA10025_GetPllConfig(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/*                   STATIC FUNCTIONS DEFINTIONS                              */
/*============================================================================*/

/*============================================================================*/
/* iTDA10025_HwInit                                                           */
/*============================================================================*/
static tmErrorCode_t
iTDA10025_HwInit
(
    tmUnitSelect_t tUnit
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16               uValue;
    HCDP_HwCdpSample_t   eHwCdpSample;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_HwInit(0x%08X)", tUnit);
    
    /* initialise common path */
    if( pObj->tUnit == pObj->tUnitCommon )
    {
        /* enable PLL */
        if (err == TM_OK)
        {
            err = iTDA10025_EnablePLL(tUnit);
        }

        /* enable all clocks (required for reset) */
        if (err == TM_OK)
        {
            uValue = 0x03FF;
            err = iTDA10025_Write(tUnit, TDA10025_CLK_EN_IND, 1, &uValue);
        }

        if (err == TM_OK)
        {
            /* reset all blocks (except CGU = hw reset only) */
            uValue = 0x0000;
            err = iTDA10025_Write(tUnit, TDA10025_RESET_CONTROL_IND, 1, &uValue);
        }

        /* wait 1ms */
        if (err == TM_OK)
        {
            err = iTDA10025_Wait(pObj, 1);
        }

        if (err == TM_OK)
        {
            /* disable reset except for OOB */
            uValue = 0xFFF7;
            err = iTDA10025_Write(tUnit, TDA10025_RESET_CONTROL_IND, 1, &uValue);
        }

        if (err == TM_OK)
        {
            /* enable clocks except for OOB */
            uValue = 0x027F;
            err = iTDA10025_Write(tUnit, TDA10025_CLK_EN_IND, 1, &uValue);
        }

        if (err == TM_OK)
        {
            /* enable clock exit reset except for OOB */
            uValue = 0x027F;
            err = iTDA10025_Write(tUnit, TDA10025_CLK_EXIT_RESET_IND, 1, &uValue);
        }

        if (err == TM_OK)
        {
            /* workaround: disable all tri-state modes to reduce consumption */
            /* OOB: enable pads */
            /* CDP AGC: enable pads */
            uValue = 0x001F;
            err = iTDA10025_WriteBit(tUnit,
                TDA10025_PAD_CTRL_IND,
                TDA10025_OOB_TS_MSK | TDA10025_OOB_POD_MSK | TDA10025_OOB_TUNER_MSK | TDA10025_DEMOD0_TUNER_MSK | TDA10025_DEMOD1_TUNER_MSK,
                uValue);
        }

        if (err == TM_OK)
        {
            /* OOB clock divider configuration to reduce consumption */
            uValue = 0x1F;
            err = iTDA10025_WriteBit(tUnit,
                TDA10025_OOB_IF_CFG_IND,
                TDA10025_OOBIP_IF_CFG_CLK_DIV_MSK,
                uValue);
        }
    }

    /* initialise path specific */
    if (err == TM_OK)
    {
        /* Activate clocks */
        /* Required for reading the registers of CDP0 or CDP1 */
        if( pObj->sCfg.eCDP == TDA10025_CDP_0 )
        {
            /* Clocks of CDP0 enabled */
            uValue = TDA10025_CLK_CDP0_CORE_MSK | TDA10025_CLK_CDP0_A2D_MSK;
            err = iTDA10025_WriteBit(tUnit, TDA10025_CLK_EN_IND, TDA10025_CLK_CDP0_CORE_MSK | TDA10025_CLK_CDP0_A2D_MSK, uValue);
            /* clk_exit_reset will be set in SetPowerState */
        }
        else
        {
            /* Clocks of CDP1 enabled */
            uValue = TDA10025_CLK_CDP1_CORE_MSK | TDA10025_CLK_CDP1_A2D_MSK;
            err = iTDA10025_WriteBit(tUnit, TDA10025_CLK_EN_IND, TDA10025_CLK_CDP1_CORE_MSK | TDA10025_CLK_CDP1_A2D_MSK, uValue);
            /* clk_exit_reset will be set in SetPowerState */
        }
    }

    /* HCDP reset required to identify the die version ES1 or ES2 from CDP block */
    if (err == TM_OK)
    {
        err = tmbslHCDP_Reset(tUnit);
    }

    /* Differenciate ES1 and ES2 in tmbslTDA10025 to support specific settings in common part */
    if (err == TM_OK)
    {
        err = tmbslHCDP_GetCdpHwSample(tUnit, &eHwCdpSample);
        if (err == TM_OK)
        {
            switch (eHwCdpSample) /* temporary: to be adapted when ES1 is removed */
            {
            case HCDP_SAMPLE_ES1:
                pObj->eHwSample              = TDA10025_HwSampleVersion_ES1;
                break;
            case HCDP_SAMPLE_ES2:
                pObj->eHwSample              = TDA10025_HwSampleVersion_ES2;
                break;
            default:
                pObj->eHwSample = TDA10025_HwSampleVersion_Unknown;
                err = TDA10025_ERR_BAD_VERSION;
            }
        }

        if( err == TM_OK )
        {
            err = iTDA10025SetPllConfig(pObj, TDA10025_DEFAULT_XTAL);
        }
    }

    if ((err == TM_OK) && (pObj->eHwSample == TDA10025_HwSampleVersion_ES1)) /* temporary: to be adapted when ES1 is removed */
    {
        /* Invert ADCs clocks for ES1 */
        uValue = 0x0007;
        err = iTDA10025_Write(tUnit, TDA10025_CLK_INV_IND, 1, &uValue);
    }

    if (err == TM_OK)
    {
        err = iTDA10025_SetPathConfiguration( tUnit,
                                              pObj->sCfg.eIF,
                                              pObj->sCfg.eCDP,
                                              pObj->sCfg.eTS );
    }

    if (err == TM_OK)
    {
        err = iTDA10025_SetPowerState(tUnit, tmPowerStandby);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_HwInit(0x%08X) failed.", tUnit));
    
    return err;
}

/*============================================================================*/
/* iTDA10025_ResetCdp                                                         */
/*============================================================================*/
static tmErrorCode_t
iTDA10025_ResetCdp
(
    tmUnitSelect_t tUnit
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16               uMask = 0;
    UInt16               uValue = 0;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_ResetCdp(0x%08X)", tUnit);

    uMask = TDA10025_CDP_0_RSTN_MSK | TDA10025_CDP_1_RSTN_MSK;

    if (err == TM_OK)
    {
        /* reset CDPs */
        uValue = 0x0000;
        err = iTDA10025_WriteBit(tUnit, TDA10025_RESET_CONTROL_IND, uMask, uValue);
    }

    /* wait 1ms */
    if (err == TM_OK)
    {
        err = iTDA10025_Wait(pObj, 1);
    }

    if (err == TM_OK)
    {
        /* release CDP reset */
        uValue = uMask;
        err = iTDA10025_WriteBit(tUnit, TDA10025_RESET_CONTROL_IND, uMask, uValue);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_ResetCdp(0x%08X) failed.", tUnit));
    
    return err;
}

/*============================================================================*/
/* iTDA10025_InitCdp                                                          */
/*============================================================================*/
static tmErrorCode_t
iTDA10025_InitCdp
(
    tmUnitSelect_t tUnit
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;
    HCDP_ExtendSymbolRateMode_t eExtendSRMode;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_InitCdp(0x%08X)", tUnit);

    /* reapply overwritten settings */
    if (err == TM_OK)
    {
        err = tmbslHCDP_Reset(tUnit);
    }

    if (err == TM_OK)
    {
        err = iTDA10025_IQPreset(tUnit, TDA10025_ConstellationSourceBEDR);
    }

    if (err == TM_OK)
    {
        switch (pObj->sCfg.eExtendSRMode)
        {
        case TDA10025_ExtendSymbolRateModeEnable700ppm:
            eExtendSRMode = HCDP_ExtendSymbolRateModeEnable700ppm;
            break;
        case TDA10025_ExtendSymbolRateModeEnable1500ppm:
            eExtendSRMode = HCDP_ExtendSymbolRateModeEnable1500ppm;
            break;
        case TDA10025_ExtendSymbolRateModeDisable:
            eExtendSRMode = HCDP_ExtendSymbolRateModeDisable;
            break;
        default:
            eExtendSRMode = HCDP_ExtendSymbolRateModeInvalid;
        }

        /* configure config Extended SR mode */
        err = tmbslHCDP_SetExtendSymbolRateRange(tUnit, eExtendSRMode);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_InitCdp(0x%08X) failed.", tUnit));
    
    return err;
}

/*============================================================================*/
/* iTDA10025SetPllConfig                                                      */
/*============================================================================*/
tmErrorCode_t iTDA10025SetPllConfig(pTDA10025Object_t pObj, TDA10025_XtalFreq_t eXtal)
{
    tmErrorCode_t  err = TM_OK;
    int index;
    int arraySize;
    Bool freqFound = False;

    UInt32 uXtal = 0;
    TDA10025PllConfig_t *pPllConfig;

    // retrieve matching xtal frequency
    switch(eXtal)
    {
    case TDA10025_XtalFreq_16MHz:
        uXtal = 16000000;
        break;
    case TDA10025_XtalFreq_27MHz:
        uXtal = 27000000;
        break;
    case TDA10025_XtalFreq_25MHz:
        uXtal = 25000000;
        break;
    default:
        err = TDA10025_ERR_BAD_PARAMETER;
        break;
    }

    /* retrieve possible configuration, depending on ES version */
    switch (pObj->eHwSample)
    {
    case TDA10025_HwSampleVersion_ES1:
        arraySize = TDA18265_ES1_PLL_CONFIG_NB;
        pPllConfig = ES1PllSettings;
        break;
    case TDA10025_HwSampleVersion_ES2:
        arraySize = TDA18265_ES2_PLL_CONFIG_NB;
        pPllConfig = ES2PllSettings;
        break;
    default:
        err = TDA10025_ERR_BAD_VERSION;
    }

    if (err == TM_OK)
    {
        index = 0;
        do {
            if( pPllConfig[index].uXtal == uXtal )
            {
                freqFound = True;
            }
            index ++;
        } while ((index < arraySize) && (freqFound == False));

        if( freqFound == True )
        {
            index--;

            pObj->sPllConfig.uXtal       = uXtal;
            pObj->sPllConfig.uPLLMFactor = pPllConfig[index].uPLLMFactor;
            pObj->sPllConfig.lPLLNFactor = pPllConfig[index].lPLLNFactor;
            pObj->sPllConfig.bPLLPFactor = pPllConfig[index].bPLLPFactor;
            pObj->sPllConfig.lPllCtl0Ind = pPllConfig[index].lPllCtl0Ind;
            pObj->sPllConfig.lPllCtl1Ind = pPllConfig[index].lPllCtl1Ind;
            pObj->sPllConfig.lPllCtl2Ind = pPllConfig[index].lPllCtl2Ind;
            pObj->sPllConfig.lPllCtl3Ind = pPllConfig[index].lPllCtl3Ind;
            pObj->sPllConfig.ePllMode    = pPllConfig[index].ePllMode;
            pObj->sPllConfig.uPllValue   = pPllConfig[index].uPllValue;
        }
        else
        {
            // configuration has not been found
            err = TDA10025_ERR_BAD_VERSION;
        }
    }

    if(err == TM_OK)
    {
        // configure HW
        err = iTDA10025_ConfigurePLL (pObj->tUnit);
    }

    return err;
}

/*============================================================================*/
/* iTDA10025_ConfigurePLL                                                     */
/*============================================================================*/
tmErrorCode_t
iTDA10025_ConfigurePLL
(
    tmUnitSelect_t tUnit
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;
    UInt32               uSamplingClock = 0;
    UInt32               uDClock = 0;
    UInt16               uValue;
    UInt16               uStoreClkExitReset;


    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_ConfigurePLL(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        /* Store config and switch CTL_CLK on the quartz to keep register access */
        err = iTDA10025_Read(tUnit, TDA10025_CLK_EXIT_RESET_IND, 1, &uStoreClkExitReset);
        if (err == TM_OK)
        {
            uValue = 0x0000;
            err = iTDA10025_Write(tUnit, TDA10025_CLK_EXIT_RESET_IND, 1, &uValue);
        }
        
        /* Set the PLL */
        if (err == TM_OK)
        {
            err = iTDA10025_WriteBit(tUnit, TDA10025_PLL_CTL_0_IND, TDA10025_PLL_CTL_0_DIRECTI_O_MSK | TDA10025_M_MSB_MSK | TDA10025_P_MSK, (UInt16)(pObj->sPllConfig.lPllCtl0Ind + (pObj->sPllConfig.ePllMode << TDA10025_PLL_CTL_0_DIRECTI_O_BIT)));
        }

        if (err == TM_OK)
        {
            err = iTDA10025_Write(tUnit, TDA10025_PLL_CTL_1_IND, 1, &pObj->sPllConfig.lPllCtl1Ind);
        }

        if (err == TM_OK)
        {
            err = iTDA10025_WriteBit(tUnit, TDA10025_PLL_CTL_2_IND, TDA10025_N_MSK, pObj->sPllConfig.lPllCtl2Ind);
        }

        if ((err == TM_OK) && (pObj->eHwSample == TDA10025_HwSampleVersion_ES2)) /* temporary: to be adapted when ES1 is removed */
        {
            err = iTDA10025_WriteBit(tUnit, TDA10025_PLL_CTL_3_IND, TDA10025_SELP_MSK | TDA10025_SELI_MSK, pObj->sPllConfig.lPllCtl3Ind);
        }

        /* Load the PLL parameters */
        if (err == TM_OK)
        {
            uValue = 0x1;
            err = iTDA10025_Write(tUnit, TDA10025_PLL_CMD_IND, 1, &uValue);
        }

        /* wait 2ms (max oscillator start-up time = 1.7ms) */
        if (err == TM_OK)
        {
            err = iTDA10025_Wait(pObj, 2);
        }

        /* Restore the clk to previous config */
        if (err == TM_OK)
        {
            err = iTDA10025_Write(tUnit, TDA10025_CLK_EXIT_RESET_IND, 1, &uStoreClkExitReset);
        }

        err = iTDA10025_CalculateHCDPClockParam(pObj, &uSamplingClock, &uDClock);
        if(err == TM_OK)
        {
            err = tmbslHCDP_SetClocks(tUnit, uSamplingClock, uDClock);
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_ConfigurePLL(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_CalculateHCDPClockParam                                          */
/*============================================================================*/
tmErrorCode_t
iTDA10025_CalculateHCDPClockParam
(
    pTDA10025Object_t  pObj,
    UInt32            *puSamplingClock,
    UInt32            *puDClock
)
{
    tmErrorCode_t        err = TM_OK;

    /* PLL = (2*M/N)*Xtal with post-divider bypassed (directo = 1) */
    if (pObj->eHwSample == TDA10025_HwSampleVersion_ES1)
    {
        *puSamplingClock = (UInt32)( pObj->sPllConfig.uPllValue / TDA10025_ES1_PLL_SAMPLING_CLOCK_DIVIDER );
        *puDClock = (UInt32)( pObj->sPllConfig.uPllValue / TDA10025_ES1_PLL_D_CLOCK_DIVIDER );
    }
    else if (pObj->eHwSample == TDA10025_HwSampleVersion_ES2)
    {
        *puSamplingClock = (UInt32)( pObj->sPllConfig.uPllValue / TDA10025_ES2_PLL_SAMPLING_CLOCK_DIVIDER );
        *puDClock = (UInt32)( pObj->sPllConfig.uPllValue / TDA10025_ES2_PLL_D_CLOCK_DIVIDER );
    }
    else
    {
        err = TDA10025_ERR_BAD_VERSION;
    }

    return err;
}

/*============================================================================*/
/* iTDA10025_EnablePLL                                                        */
/*============================================================================*/
tmErrorCode_t
iTDA10025_EnablePLL
(
    tmUnitSelect_t tUnit
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16               uValue;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_EnablePLL(0x%08X)", tUnit);

    if (pObj->eHwSample == TDA10025_HwSampleVersion_ES1) /* temporary: to be adapted when ES1 is removed */
    {
        /* PLL stand-by not supported in ES1 */
        /* PLL always on */
    }
    else if ((pObj->eHwSample == TDA10025_HwSampleVersion_ES2) ||
             (pObj->eHwSample == TDA10025_HwSampleVersion_Unknown))
    {
        /* Unknown used at start-up before reading sample version */
        if (err == TM_OK)
        {
            uValue = 0;
            err = iTDA10025_Read(tUnit, TDA10025_PLL_STATUS_IND, 1, &uValue);
        }

        /* PLL reprogrammed when not locked */
        /* This will be skipped if (Unknown==ES1) */
        if ((err == TM_OK) && (uValue == 0x0))
        {
            /* PLL not locked */
            if (err == TM_OK)
            {
                uValue = (UInt16)~TDA10025_PLL_CTL_0_PD_MSK;
                err = iTDA10025_WriteBit(tUnit,
                    TDA10025_PLL_CTL_0_IND,
                    TDA10025_PLL_CTL_0_PD_MSK,
                    uValue);
            }

            /* Load the PLL parameters (to apply changes on PLL_CTL_n) */
            if (err == TM_OK)
            {
                uValue = 0x1;
                err = iTDA10025_Write(tUnit, TDA10025_PLL_CMD_IND, 1, &uValue);
            }

            /* wait 2ms (max oscillator start-up time = 1.7ms) */
            if (err == TM_OK)
            {
                err = iTDA10025_Wait(pObj, 2);
            }

            if (err == TM_OK)
            {
                uValue = TDA10025_CLK_CTRL_MSK;
                err = iTDA10025_WriteBit(tUnit,
                    TDA10025_CLK_EXIT_RESET_IND,
                    TDA10025_CLK_CTRL_MSK,
                    uValue);
            }
        }
    }
    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_EnablePLL(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_DisablePLL                                                       */
/*============================================================================*/
tmErrorCode_t
iTDA10025_DisablePLL
(
    tmUnitSelect_t tUnit
)
{
    pTDA10025Object_t    pObj = Null;
    tmErrorCode_t        err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16               uValue;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_DisablePLL(0x%08X)", tUnit);
    
    if (pObj->eHwSample == TDA10025_HwSampleVersion_ES1) /* temporary: to be adapted when ES1 is removed */
    {
        /* PLL stand-by not supported in ES1 */
        err = TM_OK;
    }
    else if (pObj->eHwSample == TDA10025_HwSampleVersion_ES2)
    {
        if (err == TM_OK)
        {
            uValue = (UInt16)~TDA10025_CLK_CTRL_MSK;
            err = iTDA10025_WriteBit(tUnit,
                TDA10025_CLK_EXIT_RESET_IND,
                TDA10025_CLK_CTRL_MSK,
                uValue);
        }

        if (err == TM_OK)
        {
            uValue = TDA10025_PLL_CTL_0_PD_MSK;
            err = iTDA10025_WriteBit(tUnit,
                TDA10025_PLL_CTL_0_IND,
                TDA10025_PLL_CTL_0_PD_MSK,
                uValue);
        }

        /* Load the PLL parameters (to apply changes on PLL_CTL_n) */
        if (err == TM_OK)
        {
            uValue = 0x1;
            err = iTDA10025_Write(tUnit, TDA10025_PLL_CMD_IND, 1, &uValue);
        }
    }
    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_DisablePLL(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_SetPowerState                                                    */
/*============================================================================*/
tmErrorCode_t
iTDA10025_SetPowerState(
    tmUnitSelect_t tUnit,
    tmPowerState_t ePowerState
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             uValue = 0;
    UInt16             uMask = 0;
    Bool               bFullStandby = False;
    Bool               bOtherCDPinStandby = False;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    if (ePowerState == tmPowerOn)
    {
        if (err == TM_OK)
        {
            err = iTDA10025_EnablePLL(tUnit);
        }

        /* check whether the other stream is in stand-by state */
        /* in order to trigger the demodX_rstn */
        if (err == TM_OK)
        {
            if (pObj->sCfg.eCDP == TDA10025_CDP_0)
            {
                uMask = TDA10025_CLK_CDP1_CORE_MSK;
            }
            else
            {
                uMask = TDA10025_CLK_CDP0_CORE_MSK;
            }

            uValue = 0;
            err = iTDA10025_Read(tUnit, TDA10025_CLK_EN_IND, 1, &uValue);
            if (err == TM_OK)
            {
                if ((uValue & uMask) == 0x0)
                {
                    /* the other CDP is in stand-by */
                    bOtherCDPinStandby = True;
                }
            }
        }

        if (err == TM_OK)
        {
            /* With the other stream in stand-by, enable all clocks on FIFOs and CDPs */
            /* prior to triggering the demodX_rstn */
            if (bOtherCDPinStandby)
            {
                uMask = TDA10025_CLK_CDP0_ADC_MSK | TDA10025_CLK_CDP1_ADC_MSK |
                        TDA10025_CLK_CDP0_A2D_MSK | TDA10025_CLK_CDP1_A2D_MSK |
                        TDA10025_CLK_CDP0_CORE_MSK | TDA10025_CLK_CDP1_CORE_MSK;
            }
            else
            {
                uMask = 0;
            }

            if (pObj->sCfg.eCDP == TDA10025_CDP_0)
            {
                /* (IP A while IF0 connected to CDP0) or (IP B while IF0 connected to CDP1) then need to set CDP0 */
                /* Start clock core CDP0 */
                uMask |= TDA10025_CLK_CDP0_CORE_MSK | TDA10025_CLK_SER_TS_MSK;
                uValue = uMask;
                err = iTDA10025_WriteBit(tUnit, 
                    TDA10025_CLK_EN_IND, 
                    uMask,
                    uValue);

                if (err == TM_OK)
                {
                    /* Enable clock exit reset CDP0 */
                    err = iTDA10025_WriteBit(tUnit, 
                        TDA10025_CLK_EXIT_RESET_IND, 
                        uMask,
                        uValue);
                }
            }
            else /* pObj->sCfg.eCDP == TDA10025_CDP_1 */
            {
                /* (IP B while IF0 connected to CDP0) or (IP A while IF0 connected to CDP1) then need to set CDP1 */
                /* Start clock core CDP1 */
                uMask |= TDA10025_CLK_CDP1_CORE_MSK | TDA10025_CLK_SER_TS_MSK;
                uValue = uMask;
                err = iTDA10025_WriteBit(tUnit,
                    TDA10025_CLK_EN_IND, 
                    uMask, 
                    uValue);

                if (err == TM_OK)
                {
                    /* Enable clock exit reset CDP1 */
                    err = iTDA10025_WriteBit(tUnit, 
                        TDA10025_CLK_EXIT_RESET_IND, 
                        uMask,
                        uValue);
                }
            }
        }

        /* With the other stream in stand-by, trigger the reset on FIFOs and CDPs */
        /* thus disable the unnecessary clk_cdpX_core */
        if (bOtherCDPinStandby)
        {
            if (err == TM_OK)
            {
                err = iTDA10025_ResetCdp(tUnit);
            }

            if (err == TM_OK)
            {
                if (pObj->sCfg.eCDP == TDA10025_CDP_0)
                {
                    /* (IP A while IF0 connected to CDP0) or (IP B while IF0 connected to CDP1) then need to set CDP0 */
                    /* Disable clock exit reset of CDP1 core = switch to xtal */
                    uMask = TDA10025_CLK_CDP1_CORE_MSK;
                    uValue = (UInt16)~uMask;
                    err = iTDA10025_WriteBit(tUnit,
                        TDA10025_CLK_EXIT_RESET_IND,
                        uMask,
                        uValue);

                    if (err == TM_OK)
                    {
                        /* Disable clock of CDP1 core */
                        err = iTDA10025_WriteBit(tUnit,
                            TDA10025_CLK_EN_IND,
                            uMask,
                            uValue);
                    }
                }
                else /* pObj->sCfg.eCDP == TDA10025_CDP_1 */
                {
                    /* (IP B while IF0 connected to CDP0) or (IP A while IF0 connected to CDP1) then need to set CDP1 */
                    /* Disable clock exit reset of CDP0 core = switch to xtal */
                    uMask = TDA10025_CLK_CDP0_CORE_MSK;
                    uValue = (UInt16)~uMask;
                    err = iTDA10025_WriteBit(tUnit,
                        TDA10025_CLK_EXIT_RESET_IND,
                        uMask,
                        uValue);

                    if (err == TM_OK)
                    {
                        /* Disable clock of CDP0 core */
                        err = iTDA10025_WriteBit(tUnit,
                            TDA10025_CLK_EN_IND,
                            uMask,
                            uValue);
                    }
                }
            }
        }

        /* Re-apply the CDP initialization */
        /* to cope with the potential CDP reset */
        if (err == TM_OK)
        {
            err = iTDA10025_InitCdp(tUnit);
        }

        if (err == TM_OK)
        {
            if (pObj->sCfg.eIF == TDA10025_IF_0)
            {
                /* Start ADC0 */
                uValue = (UInt16)~TDA10025_ADC_PD_MSK;
                err = iTDA10025_WriteBit(tUnit, 
                    TDA10025_ADC0_CFG_IND, 
                    TDA10025_ADC_PD_MSK, 
                    uValue);
            }
            else
            {
                /* Start ADC1 */
                uValue = (UInt16)~TDA10025_ADC_PD_MSK;
                err = iTDA10025_WriteBit(tUnit, 
                    TDA10025_ADC1_CFG_IND, 
                    TDA10025_ADC_PD_MSK, 
                    uValue);
            }
        }

        if (err == TM_OK)
        {
            if( pObj->sCfg.eCDP == TDA10025_CDP_0 )
            {
                /* enable AGC for CDP0 */
                uValue = TDA10025_DEMOD0_TUNER_MSK;

                err = iTDA10025_WriteBit(tUnit, 
                    TDA10025_PAD_CTRL_IND,
                    TDA10025_DEMOD0_TUNER_MSK,
                    uValue);
            }
            else /* pObj->sCfg.eCDP == TDA10025_CDP_1 */
            {
                /* enable AGC for CDP1 */
                uValue = TDA10025_DEMOD1_TUNER_MSK;

                err = iTDA10025_WriteBit(tUnit, 
                    TDA10025_PAD_CTRL_IND,
                    TDA10025_DEMOD1_TUNER_MSK,
                    uValue);
            }
        }

        if ((err == TM_OK) && (pObj->sCfg.eTsMode == TDA10025_TsModeSerial))
        {
            /* restore the TS_CLK frequency value */
            uValue = pObj->sCfg.ulTsClkDiv;
            if (pObj->sCfg.eCDP == TDA10025_CDP_0)
            {
                err = iTDA10025_WriteBit(tUnit, TDA10025_DEMOD0_TS_CFG_IND, TDA10025_OUT_CLKDIV_MSK, uValue);
            }
            else /* pObj->sCfg.eCDP == TDA10025_CDP_1 */
            {
                err = iTDA10025_WriteBit(tUnit, TDA10025_DEMOD1_TS_CFG_IND, TDA10025_OUT_CLKDIV_MSK, uValue);
            }
        }

        if (err == TM_OK)
        {
            if( pObj->sCfg.eTS == TDA10025_TS_0 )
            {
                /* configure TS0 */
                uMask = TDA10025_ENA_TS0_MSK;
                uValue = TDA10025_ENA_TS0_MSK;
            }
            else
            {
                /* configure TS1 */
                uMask = TDA10025_ENA_TS1_MSK;
                uValue = TDA10025_ENA_TS1_MSK;
            }

            /* re-force serial TS mode to overwrite the parallel TS in case both CDPs were in stand-by */
            if (pObj->sCfg.eTsMode == TDA10025_TsModeSerial)
            {
                uMask |= TDA10025_OUT_PARASER_MSK;
                uValue &= (UInt16)~TDA10025_OUT_PARASER_MSK;
            }

            err = iTDA10025_WriteBit(tUnit, 
                TDA10025_TS_COMMON_CFG_IND, 
                uMask,
                uValue);
        }
    }
    else /* ePowerState != tmPowerOn */
    {
        /* stand-by */

        /* check whether full stand-by can be selected or not */
        if (err == TM_OK)
        {
            if (pObj->sCfg.eCDP == TDA10025_CDP_0)
            {
                uMask = TDA10025_CLK_CDP1_CORE_MSK;
            }
            else
            {
                uMask = TDA10025_CLK_CDP0_CORE_MSK;
            }

            uValue = 0;
            err = iTDA10025_Read(tUnit, TDA10025_CLK_EN_IND, 1, &uValue);
            if (err == TM_OK)
            {
                if ((uValue & uMask) == 0x0)
                {
                    /* the other CDP is in stand-by */
                    bOtherCDPinStandby = True;
                    if (((uValue & TDA10025_CLK_OOB_MSK) == 0x0) && (pObj->sCfg.eTsClkStdby == TDA10025_TsCfg_ClkInStdbyOff))
                    {
                        /* both the other CDP and OOB are in stand-by */
                        bFullStandby = True;
                    }
                }
            }
        }

        if (err == TM_OK)
        {
            if( pObj->sCfg.eTS == TDA10025_TS_0 )
            {
                /* configure TS0 */
                uMask = TDA10025_ENA_TS0_MSK;
                /* workaround: disable all tri-state modes to reduce consumption */
                /* uValue = ~TDA10025_ENA_TS0_MSK; */
                uValue = TDA10025_ENA_TS0_MSK;
            }
            else
            {
                /* configure TS1 */
                uMask = TDA10025_ENA_TS1_MSK;
                /* workaround: disable all tri-state modes to reduce consumption */
                /* uValue = ~TDA10025_ENA_TS1_MSK; */
                uValue = TDA10025_ENA_TS1_MSK;
            }

            /* force serial TS mode to parallel TS when both CDPs are in stand-by */
            if (bOtherCDPinStandby && (pObj->sCfg.eTsMode == TDA10025_TsModeSerial) && (pObj->sCfg.eTsClkStdby == TDA10025_TsCfg_ClkInStdbyOff))
            {
                uMask |= TDA10025_OUT_PARASER_MSK;
                uValue |= TDA10025_OUT_PARASER_MSK;
            }

            err = iTDA10025_WriteBit(tUnit, 
                TDA10025_TS_COMMON_CFG_IND,
                uMask,
                uValue);
        }

        if ((err == TM_OK) && (pObj->sCfg.eTsMode == TDA10025_TsModeSerial) && (pObj->sCfg.eTsClkStdby == TDA10025_TsCfg_ClkInStdbyOff))
        {
            /* force the lowest TS_CLK frequency value */
            uValue = 0x1F;
            if (pObj->sCfg.eCDP == TDA10025_CDP_0)
            {
                err = iTDA10025_WriteBit(tUnit, TDA10025_DEMOD0_TS_CFG_IND, TDA10025_OUT_CLKDIV_MSK, uValue);
            }
            else /* pObj->sCfg.eCDP == TDA10025_CDP_1 */
            {
                err = iTDA10025_WriteBit(tUnit, TDA10025_DEMOD1_TS_CFG_IND, TDA10025_OUT_CLKDIV_MSK, uValue);
            }
        }

        /* workaround: disable all tri-state modes to reduce consumption */
        //if (err == TM_OK)
        //{
        //    if( pObj->sCfg.eCDP == TDA10025_CDP_0 )
        //    {
        //        /* configure CDP0 */
        //        uValue = ~TDA10025_DEMOD0_TUNER_MSK;

        //        err = iTDA10025_WriteBit(tUnit, 
        //            TDA10025_PAD_CTRL_IND, 
        //            TDA10025_DEMOD0_TUNER_MSK,
        //            uValue);
        //    }
        //    else
        //    {
        //        /* configure CDP1 */
        //        uValue = ~TDA10025_DEMOD1_TUNER_MSK;

        //        err = iTDA10025_WriteBit(tUnit, 
        //            TDA10025_PAD_CTRL_IND, 
        //            TDA10025_DEMOD1_TUNER_MSK,
        //            uValue);
        //    }
        //}

        if (err == TM_OK)
        {
            if( pObj->sCfg.eIF == TDA10025_IF_0 )
            {
                /* Stop ADC0 */
                uValue = TDA10025_ADC_PD_MSK;
                err = iTDA10025_WriteBit(tUnit, 
                    TDA10025_ADC0_CFG_IND, 
                    TDA10025_ADC_PD_MSK, 
                    uValue);
            }
            else
            {
                /* Stop ADC1 */
                uValue = TDA10025_ADC_PD_MSK;
                err = iTDA10025_WriteBit(tUnit, 
                    TDA10025_ADC1_CFG_IND, 
                    TDA10025_ADC_PD_MSK, 
                    uValue);
            }
        }

        if (err == TM_OK)
        {
            if (bFullStandby)
            {
                uMask = TDA10025_CLK_CDP0_CORE_MSK | TDA10025_CLK_CDP0_A2D_MSK | TDA10025_CLK_CDP0_ADC_MSK |
                        TDA10025_CLK_CDP1_CORE_MSK | TDA10025_CLK_CDP1_A2D_MSK | TDA10025_CLK_CDP1_ADC_MSK |
                        TDA10025_CLK_SER_TS_MSK;
                uValue = (UInt16)~uMask;
            }
            else if (bOtherCDPinStandby)
            {
                uMask = TDA10025_CLK_CDP0_CORE_MSK | TDA10025_CLK_CDP0_A2D_MSK | TDA10025_CLK_CDP0_ADC_MSK |
                        TDA10025_CLK_CDP1_CORE_MSK | TDA10025_CLK_CDP1_A2D_MSK | TDA10025_CLK_CDP1_ADC_MSK;
                uValue = (UInt16)~uMask;
            }
            else if (pObj->sCfg.eCDP == TDA10025_CDP_0)
            {
                /* (IP A while IF0 connected to CDP0) or (IP B while IF0 connected to CDP1) then need to set CDP0 */
                uMask = TDA10025_CLK_CDP0_CORE_MSK /*| TDA10025_CLK_CDP0_A2D_MSK | TDA10025_CLK_CDP0_ADC_MSK*/;
                uValue = (UInt16)~uMask;
            }
            else /* (pObj->sCfg.eCDP == TDA10025_CDP_1) */
            {
                /* (IP B while IF0 connected to CDP0) or (IP A while IF0 connected to CDP1) then need to set CDP1 */
                uMask = TDA10025_CLK_CDP1_CORE_MSK /*| TDA10025_CLK_CDP1_A2D_MSK | TDA10025_CLK_CDP1_ADC_MSK*/;
                uValue = (UInt16)~uMask;
            }
            /* Disable clock exit reset CDPx */
            err = iTDA10025_WriteBit(tUnit, 
                TDA10025_CLK_EXIT_RESET_IND, 
                uMask,
                uValue);

            if (err == TM_OK)
            {
                /* Stop clocks CDPx */
                err = iTDA10025_WriteBit(tUnit, 
                    TDA10025_CLK_EN_IND, 
                    uMask,
                    uValue);
            }
        }

        if ((err == TM_OK) && bFullStandby)
        {
            err = iTDA10025_DisablePLL(tUnit);
        }
    }

    if( err == TM_OK )
    {
        pObj->ePowerState = ePowerState;
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_SetPowerState(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/*                   I2C FUNCTIONS                                            */
/*============================================================================*/

/*============================================================================*/
/* iTDA10025_IPWrite                                                          */
/*============================================================================*/
static tmErrorCode_t
iTDA10025_IPWrite
(
    tmUnitSelect_t tUnit,
    UInt32 AddrSize,
    UInt8* pAddr,
    UInt32 WriteLen,
    UInt8* pData
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             i2c_index = 0;
    
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_BLAB, "iTDA10025_IPWrite(0x%08X)", pObj->tUnit);
    
    if (err == TM_OK)
    {
        switch (AddrSize)
        {
        case 2:
            i2c_index = *((UInt16*)pAddr);
            break;
        default:
            err = TDA10025_ERR_BAD_PARAMETER;
            break;
        }
        
        if (err == TM_OK)
        {
            if (pObj->sCfg.eCDP == TDA10025_CDP_0)
            {
                i2c_index = i2c_index * 2 + TDA10025_CDP_0_ADDRESS;
            }
            else
            {
                i2c_index = i2c_index * 2 + TDA10025_CDP_1_ADDRESS;
            }
        }

        if (err == TM_OK)
        {
            err = P_SIO_WRITE( iTDA10025_GetUnit(tUnit), 2, (UInt8*)&i2c_index, WriteLen, pData);
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_IPWrite(0x%08X) failed.", pObj->tUnit));

    return err;
}



/*============================================================================*/
/* iTDA10025_IPRead                                                           */
/*============================================================================*/
static tmErrorCode_t
iTDA10025_IPRead
(
    tmUnitSelect_t tUnit,
    UInt32 AddrSize,
    UInt8* pAddr,
    UInt32 ReadLen,
    UInt8* pData
)
{
    pTDA10025Object_t   pObj = Null;
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16              i2c_index = 0;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_BLAB, "iTDA10025_IPRead(0x%08X)", pObj->tUnit);
 
    if (err == TM_OK)
    {
        switch (AddrSize)
        {
        case 2:
            i2c_index = *((UInt16*)pAddr);
            break;
        default:
            err = TDA10025_ERR_BAD_PARAMETER;
            break;
        }
        
        if (err == TM_OK)
        {
            if (pObj->sCfg.eCDP == TDA10025_CDP_0)
            {
                i2c_index = i2c_index * 2 + TDA10025_CDP_0_ADDRESS;
            }
            else
            {
                i2c_index = i2c_index * 2 + TDA10025_CDP_1_ADDRESS;
            }
        }

        if (err == TM_OK)
        {
            err = P_SIO_READ( iTDA10025_GetUnit(tUnit), 2, (UInt8*)&i2c_index, ReadLen, pData);
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_IPRead(0x%08X) failed.", pObj->tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_TsmfWrite                                                        */
/*============================================================================*/
static tmErrorCode_t
iTDA10025_TsmfWrite
(
    tmUnitSelect_t tUnit,
    UInt16         uIndex_U,
    UInt32         uNBytes_U,
    UInt16*        puData_U
)
{

    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             i2c_index = uIndex_U;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_BLAB, "iTDA10025_TsmfWrite(0x%08X)", tUnit);

    /* check that address matchs with TSMF */
    if( err == TM_OK )
    {
        if( uIndex_U < TDA10025_BYPASS )
        {
            err = TDA10025_ERR_BAD_PARAMETER;
        }
    }

    /* select add an offset to the register value depending of selected TS */
    if (err == TM_OK)
    {
        if (pObj->sCfg.eCDP == TDA10025_CDP_0)
        {
            i2c_index = i2c_index + TDA10025_TSMF_0_ADDRESS;
        }
        else
        {
            i2c_index = i2c_index + TDA10025_TSMF_1_ADDRESS;
        }
    }
    
    if (err == TM_OK)
    {
        err = P_SIO_WRITE( iTDA10025_GetUnit(tUnit), 2, (UInt8*)&i2c_index, uNBytes_U*2, (UInt8*)puData_U);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_TsmfWrite(0x%08X) failed.", pObj->tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_TsmfRead                                                         */
/*============================================================================*/
static tmErrorCode_t
iTDA10025_TsmfRead
(
    tmUnitSelect_t  tUnit,
    UInt16          uIndex_U,
    UInt32          uNBytes_U,
    UInt16*         puData_U
)
{

    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16              i2c_index = uIndex_U;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_BLAB, "iTDA10025_TsmfRead(0x%08X)", tUnit);

    /* check that address matchs with TSMF */
    if( err == TM_OK )
    {
        if( uIndex_U < TDA10025_BYPASS )
        {
            err = TDA10025_ERR_BAD_PARAMETER;
        }
    }

    /* select add an offset to the register value depending of selected TS */
    if( err == TM_OK )
    {
        if (pObj->sCfg.eCDP == TDA10025_CDP_0)
        {
            i2c_index = i2c_index + TDA10025_TSMF_0_ADDRESS;
        }
        else
        {
            i2c_index = i2c_index + TDA10025_TSMF_1_ADDRESS;
        }
    }

    if( err == TM_OK )
    {
        err = P_SIO_READ( iTDA10025_GetUnit(tUnit), 2, (UInt8*)&i2c_index, uNBytes_U*2, (UInt8*)puData_U);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_TsmfRead(0x%08X) failed.", pObj->tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_TsmfWriteBit                                                     */
/*============================================================================*/
tmErrorCode_t
iTDA10025_TsmfWriteBit(
    tmUnitSelect_t tUnit,
    UInt16 uIndex_U,
    UInt16 uMask_U,
    UInt16 uData_U
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             byte;

    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_BLAB, "iTDA10025_TsmfWriteBit(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = iTDA10025_TsmfRead(tUnit, uIndex_U, 1, &byte);
    }
    
    if (err == TM_OK)
    {
        byte = (~(UInt16)uMask_U & byte) | ((UInt16)uMask_U & (UInt16)uData_U);
        err = iTDA10025_TsmfWrite(tUnit, uIndex_U, 1, &byte);
    }
    
    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_TsmfWriteBit(0x%08X) failed.", pObj->tUnit));

    return err;
}


static tmUnitSelect_t iTDA10025_GetUnit( tmUnitSelect_t tUnit )
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance( tUnit, &pObj );

    if( pObj->eChannelSel == TDA10025_ChannelSelectionIpA_SinglePath )
    {
        return pObj->tUnitCommon;
    }
    else
    {
        return tUnit;
    }
}

/*============================================================================*/
/* iTDA10025_Write                                                              */
/*============================================================================*/
tmErrorCode_t
iTDA10025_Write
(
    tmUnitSelect_t tUnit,
    UInt16         uIndex_U,
    UInt32         uNBytes_U,
    UInt16*        puData_U
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_BLAB, "iTDA10025_Write(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = P_SIO_WRITE( iTDA10025_GetUnit(tUnit), 2, (UInt8*)&uIndex_U, uNBytes_U*2, (UInt8*)puData_U);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_Write(0x%08X) failed.", pObj->tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_Read                                                               */
/*============================================================================*/
tmErrorCode_t
iTDA10025_Read
(
    tmUnitSelect_t  tUnit,
    UInt16          uIndex_U,
    UInt32          uNBytes_U,
    UInt16*         puData_U
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_BLAB, "iTDA10025_Read(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = P_SIO_READ( iTDA10025_GetUnit(tUnit), 2, (UInt8*)&uIndex_U, uNBytes_U*2, (UInt8*)puData_U);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_Read(0x%08X) failed.", pObj->tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_WriteBit                                                           */
/*============================================================================*/
tmErrorCode_t
iTDA10025_WriteBit(
    tmUnitSelect_t tUnit,
    UInt16 uIndex_U,
    UInt16 uMask_U,
    UInt16 uData_U
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             byte;

    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_BLAB, "iTDA10025_WriteBit(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        err = iTDA10025_Read(tUnit, uIndex_U, 1, &byte);
    }
    
    if (err == TM_OK)
    {
        byte = (~(UInt16)uMask_U & byte) | ((UInt16)uMask_U & (UInt16)uData_U);
        err = iTDA10025_Write(tUnit, uIndex_U, 1, &byte);
    }
    
    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_WriteBit(0x%08X) failed.", pObj->tUnit));

    return err;
}

/*============================================================================*/
/* FUNCTION:    iTDA10025_Wait                                                */
/*                                                                            */
/* DESCRIPTION: Waits for requested time.                                     */
/*                                                                            */
/* RETURN:      TM_OK if no error                                             */
/*                                                                            */
/*============================================================================*/
tmErrorCode_t 
iTDA10025_Wait(
    pTDA10025Object_t pObj,   /* I: Driver object */
    UInt32 uTime    /* I: time to wait for (in ms) */
)
{
    tmErrorCode_t   err = TDA10025_ERR_NULL_CONTROLFUNC;

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_Wait(0x%08X)", pObj->tUnit);

    if (P_STIME_WAIT_VALID)
    {
        /* Wait Time ms */
        err = P_STIME_WAIT(pObj->tUnit, uTime);
        tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_Wait(0x%08X, %d) failed.", pObj->tUnit, uTime));
    }

    return err;
}

/*============================================================================*/
/* Open a mutex object                                                        */
/*============================================================================*/
static tmErrorCode_t iTDA10025_MutexInitDummy(ptmbslFrontEndMutexHandle *ppMutexHandle)
{
    tmErrorCode_t err = TM_OK;
    
    *ppMutexHandle = (ptmbslFrontEndMutexHandle)1;

    return err;
}

/*============================================================================*/
/* Close a mutex object                                                       */
/*============================================================================*/
static tmErrorCode_t iTDA10025_MutexDeInitDummy(ptmbslFrontEndMutexHandle pMutexHandle)
{
    tmErrorCode_t err = TM_OK;
    return err;
}

/*============================================================================*/
/* Acquire a mutex object                                                     */
/*============================================================================*/
static tmErrorCode_t iTDA10025_MutexAcquireDummy(ptmbslFrontEndMutexHandle pMutexHandle, UInt32 timeOut)
{
    tmErrorCode_t err = TM_OK;
    return err;
}

/*============================================================================*/
/* Release a mutex object                                                     */
/*============================================================================*/
static tmErrorCode_t iTDA10025_MutexReleaseDummy(ptmbslFrontEndMutexHandle pMutexHandle)
{
    tmErrorCode_t err = TM_OK;
    return err;
}

#ifdef _TVFE_IMPLEMENT_MUTEX
/*============================================================================*/
/* FUNCTION:    iTDA10025_MutexAcquire:                                       */
/*                                                                            */
/* DESCRIPTION: Acquires driver mutex.                                        */
/*                                                                            */
/* RETURN:      TM_OK if no error                                             */
/*                                                                            */
/* NOTES:                                                                     */
/*============================================================================*/
tmErrorCode_t
iTDA10025_MutexAcquire(
    pTDA10025Object_t   pObj,
    UInt32              timeOut
)
{
    tmErrorCode_t   err = TDA10025_ERR_NULL_CONTROLFUNC;

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_MutexAcquire(0x%08X)", pObj->tUnitW);

    if(P_SMUTEX_ACQUIRE_VALID && P_MUTEX_VALID)
    {
        err = P_SMUTEX_ACQUIRE(P_MUTEX, timeOut);
        tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "Mutex_Acquire(0x%08X, %d) failed.", pObj->tUnitW, timeOut));
    }

    return err;
}

/*============================================================================*/
/* FUNCTION:    iTDA10025_MutexRelease:                                       */
/*                                                                            */
/* DESCRIPTION: Releases driver mutex.                                        */
/*                                                                            */
/* RETURN:      TM_OK if no error                                             */
/*                                                                            */
/* NOTES:                                                                     */
/*============================================================================*/
tmErrorCode_t
iTDA10025_MutexRelease(
    pTDA10025Object_t   pObj
)
{
    tmErrorCode_t   err = TDA10025_ERR_NULL_CONTROLFUNC;

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_MutexRelease(0x%08X)", pObj->tUnitW);

    if(P_SMUTEX_RELEASE_VALID && P_MUTEX_VALID)
    {
        err = P_SMUTEX_RELEASE(P_MUTEX);
        tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "Mutex_Release(0x%08X) failed.", pObj->tUnitW));
    }

    return err;
}
#endif

/*============================================================================*/
/* FUNCTION:    iTDA10025_Log                                                 */
/*                                                                            */
/* DESCRIPTION: Releases driver mutex.                                        */
/*                                                                            */
/* RETURN:      TM_OK if no error                                             */
/*                                                                            */
/* NOTES:                                                                     */
/*============================================================================*/
static UInt32
iTDA10025_Log(
    UInt32 ulValue
)
{
    UInt32 ulLogValue;
    UInt32 ulIndex = 0;

    if (ulValue > 0)
    {
        // find out the log value in the table
        while ( (gTDA10025LogTable[ulIndex].uX < ulValue) && (gTDA10025LogTable[ulIndex].uX != 0) )
        {
            ulIndex ++;
        }

        ulLogValue = gTDA10025LogTable[ulIndex].uLogX;
    }
    else
    {
        // error case
        ulLogValue = 0;
    }

    return ulLogValue;
}

/*============================================================================*/
/* iTDA10025_SetTsmfMode                                                      */
/*============================================================================*/
static tmErrorCode_t
iTDA10025_SetTsmfMode
(
    tmUnitSelect_t       tUnit,
    TDA10025_TsmfMode_t  eTsmfMode
 )
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             uValue = 0;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    if( err == TM_OK )
    {
        switch( eTsmfMode )
        {
        case TDA10025_TsmfModeDisabled:
            uValue = TDA10025_BYPASS_BYPASS_DISABLE;

            err = iTDA10025_TsmfWriteBit(
                tUnit, 
                TDA10025_BYPASS, 
                TDA10025_BYPASS_BYPASS_MSK, 
                uValue);

            if( err == TM_OK )
            {
                uValue = TDA10025_MODE_BYPASS_DISABLE;

                err = iTDA10025_TsmfWriteBit(
                    tUnit, 
                    TDA10025_MODE, 
                    TDA10025_MODE_BYPASS_MSK, 
                    uValue);
            }
            break;

        case TDA10025_TsmfModeForced:
            uValue = TDA10025_BYPASS_BYPASS_ENABLE;

            err = iTDA10025_TsmfWriteBit(
                tUnit, 
                TDA10025_BYPASS, 
                TDA10025_BYPASS_BYPASS_MSK, 
                uValue);

            if( err == TM_OK )
            {
                uValue = TDA10025_MODE_BYPASS_ENABLE;

                err = iTDA10025_TsmfWriteBit(
                    tUnit, 
                    TDA10025_MODE, 
                    TDA10025_MODE_BYPASS_MSK, 
                    uValue);
            }
            break;

        case TDA10025_TsmfModeAuto:
            uValue = TDA10025_BYPASS_BYPASS_ENABLE;

            err = iTDA10025_TsmfWriteBit(
                tUnit, 
                TDA10025_BYPASS, 
                TDA10025_BYPASS_BYPASS_MSK, 
                uValue);

            if( err == TM_OK )
            {
                uValue = TDA10025_MODE_BYPASS_AUTOMATIC;

                err = iTDA10025_TsmfWriteBit(
                    tUnit, 
                    TDA10025_MODE, 
                    TDA10025_MODE_BYPASS_MSK, 
                    uValue);
            }
            break;

        default:
            err = TDA10025_ERR_BAD_PARAMETER;
            break;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_SetTsmfMode(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_GetTsmfMode                                                      */
/*============================================================================*/
static tmErrorCode_t 
iTDA10025_GetTsmfMode
( 
    tmUnitSelect_t tUnit, 
    TDA10025_TsmfMode_t *peTsmfMode
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             uValue = 0;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    if( err == TM_OK )
    {
        err = iTDA10025_TsmfRead(
            tUnit, 
            TDA10025_MODE, 
            1, 
            &uValue);

        switch(uValue)
        {
        case TDA10025_MODE_BYPASS_DISABLE:
            *peTsmfMode = TDA10025_TsmfModeDisabled;
            break;

        case TDA10025_MODE_BYPASS_ENABLE:
            *peTsmfMode = TDA10025_TsmfModeForced;
            break;

        case TDA10025_MODE_BYPASS_AUTOMATIC:
            *peTsmfMode = TDA10025_TsmfModeAuto;
            break;

        default:
            err = TDA10025_ERR_BAD_PARAMETER;
            break;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_GetTsmfMode(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_GetTsmfStatus                                                    */
/*============================================================================*/
static tmErrorCode_t 
iTDA10025_GetTsmfStatus
( 
    tmUnitSelect_t          tUnit, 
    TDA10025_TsmfStatus_t  *psTsmfStatus
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             uValue = 0;
    TDA10025_TsmfMode_t eTsmfMode;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    if (err == TM_OK)
    {
        err = iTDA10025_GetTsmfMode( tUnit, &eTsmfMode);
    }

    if (eTsmfMode == TDA10025_TsmfModeDisabled)
    {
        psTsmfStatus->eLockStatus = TDA10025_TsmfStatusLockNotlocked;
        psTsmfStatus->eError = TDA10025_TsmfStatusErrorInvalid;
        psTsmfStatus->eEmergency = TDA10025_TsmfEmergencyNA;
        psTsmfStatus->eCurrentReceiveStatus = TDA10025_TsmfReceiveStatusNA;
        psTsmfStatus->uVersionNumber = 0;
        psTsmfStatus->uSelectedTsId = 0;
        psTsmfStatus->uSelectedOnId = 0;
    }
    else if (eTsmfMode == TDA10025_TsmfModeInvalid)
    {
        psTsmfStatus->eLockStatus = TDA10025_TsmfStatusLockInvalid;
        psTsmfStatus->eError = TDA10025_TsmfStatusErrorInvalid;
        psTsmfStatus->eEmergency = TDA10025_TsmfEmergencyNA;
        psTsmfStatus->eCurrentReceiveStatus = TDA10025_TsmfReceiveStatusNA;
        psTsmfStatus->uVersionNumber = 0;
        psTsmfStatus->uSelectedTsId = 0;
        psTsmfStatus->uSelectedOnId = 0;
    }
    else
    {
        if (err == TM_OK)
        {
            err = iTDA10025_TsmfRead(
                tUnit, 
                TDA10025_REG0x05, 
                1, 
                &uValue);

            if( err == TM_OK )
            {
                /* Lock status */
                if( ((uValue & TDA10025_REG0x05_M_LOCK_MSK) >> TDA10025_REG0x05_M_LOCK_BIT) == 0 )
                {
                    psTsmfStatus->eLockStatus = TDA10025_TsmfStatusLockNotlocked;
                }
                else
                {
                    psTsmfStatus->eLockStatus = TDA10025_TsmfStatusLockLocked;
                }

                /* version number */
                psTsmfStatus->uVersionNumber = (uValue & TDA10025_REG0x05_VER_MSK) >> TDA10025_REG0x05_VER_BIT;

                /* error */
                if( ((uValue & TDA10025_REG0x05_ERROR_MSK) >> TDA10025_REG0x05_ERROR_BIT) == 0 )
                {
                    psTsmfStatus->eError = TDA10025_TsmfStatusNoError;
                }
                else
                {
                    psTsmfStatus->eError = TDA10025_TsmfStatusError;
                }

                if( psTsmfStatus->eError == TDA10025_TsmfStatusNoError )
                {
                    /* emergency */
                    if( ((uValue & TDA10025_REG0x05_EMERGENCY_MSK) >> TDA10025_REG0x05_EMERGENCY_BIT ) == 0 )
                    {
                        psTsmfStatus->eEmergency = TDA10025_TsmfEmergencyInactive;
                    }
                    else
                    {
                        psTsmfStatus->eEmergency = TDA10025_TsmfEmergencyActive;
                    }

                    /* current receive status */
                    switch( (uValue & TDA10025_REG0x05_CRE_ST_MSK) >> TDA10025_REG0x05_CRE_ST_BIT )
                    {
                    case 0:
                        psTsmfStatus->eCurrentReceiveStatus = TDA10025_TsmfReceiveStatusGood;
                        break;
                    case 1:
                        psTsmfStatus->eCurrentReceiveStatus = TDA10025_TsmfReceiveStatusMedium;
                        break;
                    case 2:
                        psTsmfStatus->eCurrentReceiveStatus = TDA10025_TsmfReceiveStatusBad;
                        break;
                    default:
                        psTsmfStatus->eCurrentReceiveStatus = TDA10025_TsmfReceiveStatusNA;
                        break;
                    }
                }
                else
                {
                    psTsmfStatus->eEmergency = TDA10025_TsmfEmergencyNA;
                    psTsmfStatus->eCurrentReceiveStatus = TDA10025_TsmfReceiveStatusNA;
                }
            }

            if( err == TM_OK )
            {
                /* get TS ID matching with the parameter */
                err = iTDA10025_TsmfRead(
                    tUnit, 
                    TDA10025_TS_ID, 
                    1, 
                    &uValue );

                psTsmfStatus->uSelectedTsId = uValue;
            }

            if( err == TM_OK )
            {
                /* get ON ID (original network) matching with the parameter */
                err = iTDA10025_TsmfRead(
                    tUnit, 
                    TDA10025_ON_ID, 
                    1, 
                    &uValue );

                psTsmfStatus->uSelectedOnId = uValue;
            }
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_GetTsmfStatus(0x%08X) failed.", tUnit));

    return err;
}

/*============================================================================*/
/* iTDA10025_GetTsmfAvailableTsId                                             */
/*============================================================================*/
static tmErrorCode_t
iTDA10025_GetTsmfAvailableTsId
(
    tmUnitSelect_t              tUnit,
    TDA10025_TsmfAvailableTs_t *psTsmfAvailableTs
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             uValue = 0;
    UInt16             uTsStatus = 0;
    UInt16             uReStatusMsb = 0;
    UInt16             uReStatusLsb = 0;
    TDA10025_TsmfMode_t eTsmfMode;
    UInt16             uIndex;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    if (err == TM_OK)
    {
        err = iTDA10025_GetTsmfMode( tUnit, &eTsmfMode);
    }

    if ((eTsmfMode == TDA10025_TsmfModeDisabled) || (eTsmfMode == TDA10025_TsmfModeInvalid))
    {
        for( uIndex = 0; uIndex < TDA10025_TSMF_AVAILABLE_TS_NUMBER; uIndex ++ )
        {
            psTsmfAvailableTs[uIndex].eTsStatus = TDA10025_TsmfTsStatusInvalid;
            psTsmfAvailableTs[uIndex].eRcvStatus = TDA10025_TsmfReceiveStatusNA;
            psTsmfAvailableTs[uIndex].TsId = 0;
            psTsmfAvailableTs[uIndex].OnId = 0;
        }
    }
    else
    {
        if( err == TM_OK)
        {
            /* TS status bit */
            err = iTDA10025_TsmfRead(
                tUnit, 
                TDA10025_TS_STATUS, 
                1, 
                &uTsStatus);
        }

        if( err == TM_OK)
        {
            /* RE status MSB */
            err = iTDA10025_TsmfRead(
                tUnit, 
                TDA10025_RE_STATUS_MSB, 
                1, 
                &uReStatusMsb);
        }

        if( err == TM_OK)
        {
            /* RE status LSB */
            err = iTDA10025_TsmfRead(
                tUnit, 
                TDA10025_RE_STATUS_LSB, 
                1, 
                &uReStatusLsb);
        }

        if( err == TM_OK)
        {
            /* there are 15 streams -> uIndex from 0 to 14 */
            for( uIndex = 0; uIndex < TDA10025_TSMF_AVAILABLE_TS_NUMBER; uIndex ++ )
            {
                /* TS status bit */
                /* TS_STATUS[0] for the relative_ts_id[1] */
                uValue = (uTsStatus >> uIndex) & BIT_MSK;

                if( uValue == 1 )
                {
                    psTsmfAvailableTs[uIndex].eTsStatus = TDA10025_TsmfTsStatusValid;
                }
                else
                {
                    psTsmfAvailableTs[uIndex].eTsStatus = TDA10025_TsmfTsStatusInvalid;
                }

                /* check receive status if the TS is valid */
                /* else set it to Not Applicable */
                if( psTsmfAvailableTs[uIndex].eTsStatus == TDA10025_TsmfTsStatusValid )
                {
                    /* RE status */
                    /* RE_STATUS[0] for the relative_ts_id[1] */
                    uValue = ((uReStatusLsb >> uIndex) & BIT_MSK) | (((uReStatusMsb >> uIndex) & BIT_MSK) << 1);

                    switch( uValue )
                    {
                    case 0:
                        psTsmfAvailableTs[uIndex].eRcvStatus = TDA10025_TsmfReceiveStatusGood;
                        break;
                    case 1:
                        psTsmfAvailableTs[uIndex].eRcvStatus = TDA10025_TsmfReceiveStatusMedium;
                        break;
                    case 2:
                        psTsmfAvailableTs[uIndex].eRcvStatus = TDA10025_TsmfReceiveStatusBad;
                        break;
                    default:
                        psTsmfAvailableTs[uIndex].eRcvStatus = TDA10025_TsmfReceiveStatusNA;
                        break;
                    }
                }
                else
                {
                    psTsmfAvailableTs[uIndex].eRcvStatus = TDA10025_TsmfReceiveStatusNA;
                }
            }
        }

        for( uIndex = 0; uIndex < TDA10025_TSMF_AVAILABLE_TS_NUMBER; uIndex ++ )
        {
            if( err == TM_OK)
            {
                err = iTDA10025_TsmfRead(
                    tUnit, 
                    TDA10025_TS_ID1 + uIndex * TDA10025_TS_ID_OFFSET, 
                    1, 
                    &uValue);
            }

            psTsmfAvailableTs[uIndex].TsId = uValue;

            if( err == TM_OK)
            {
                err = iTDA10025_TsmfRead(
                    tUnit, 
                    TDA10025_ON_ID1 + uIndex * TDA10025_ON_ID_OFFSET, 
                    1, 
                    &uValue);
            }

            psTsmfAvailableTs[uIndex].OnId = uValue;
        }
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_GetTsmfAvailableTsId(0x%08X) failed.", tUnit));
    
    return err;
}

/*============================================================================*/
/* iTDA10025_SetTsmfTsId                                                      */
/*============================================================================*/
static tmErrorCode_t 
iTDA10025_SelectTsmfTsId
(
    tmUnitSelect_t   tUnit,
    UInt32           uTsID       
)
{
    pTDA10025Object_t  pObj = Null;
    tmErrorCode_t      err = TDA10025_ERR_NOT_INITIALIZED;
    UInt16             uValue = 0;
    TDA10025_TsmfAvailableTs_t aTsmfAvailableTs[TDA10025_TSMF_AVAILABLE_TS_NUMBER];


    /* check that TS ID parameter is in the right range */
    if( uTsID > 0 && uTsID <= TDA10025_TSMF_AVAILABLE_TS_NUMBER )
    {
        err = TM_OK;
    }
    else
    {
        err = TDA10025_ERR_BAD_PARAMETER;
    }

    /* check input parameters */
    if( err == TM_OK )
    {
        err = iTDA10025_GetInstance(tUnit, &pObj);
    }

    /* retrieve current TS ID and ON ID */
    if( err == TM_OK )
    {
        err = iTDA10025_GetTsmfAvailableTsId( tUnit, aTsmfAvailableTs );
    }

    if( err == TM_OK )
    {
        /* set TS ID matching with the parameter */
        uValue = aTsmfAvailableTs[uTsID - 1].TsId;

        err = iTDA10025_TsmfWrite(
            tUnit, 
            TDA10025_TS_ID, 
            1, 
            &uValue );
    }

    if( err == TM_OK )
    {
        /* set ON ID (original network) matching with the parameter */
        uValue = aTsmfAvailableTs[uTsID - 1].OnId;

        err = iTDA10025_TsmfWrite(
            tUnit, 
            TDA10025_ON_ID, 
            1, 
            &uValue );
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_SetTsmfTsId(0x%08X) failed.", tUnit));

    return err;
}


/*============================================================================*/
/* iTDA10025_IQPreset                                                         */
/*============================================================================*/
tmErrorCode_t
iTDA10025_IQPreset
(
    tmUnitSelect_t                  tUnit,
    TDA10025_ConstellationSource_t  eSource
)
{
    pTDA10025Object_t           pObj = Null;
    tmErrorCode_t               err = TDA10025_ERR_NOT_INITIALIZED;
    HCDP_ConstellationSource_t  sConstellationSource;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    tmDBGPRINTEx(DEBUGLVL_INOUT, "iTDA10025_IQPreset(0x%08X)", tUnit);
    
    if (err == TM_OK)
    {
        pObj->sIP.sConstSource = eSource;
        switch (eSource)
        {
        case TDA10025_ConstellationSourceADC:
            sConstellationSource = HCDP_ConstellationSource_ADC;
            break;
        case TDA10025_ConstellationSourceFEDR:
            sConstellationSource = HCDP_ConstellationSource_FEDR;
            break;
        case TDA10025_ConstellationSourcePDF:
            sConstellationSource = HCDP_ConstellationSource_PDF;
            break;
        case TDA10025_ConstellationSourceDAGC:
            sConstellationSource = HCDP_ConstellationSource_DAGC;
            break;
        case TDA10025_ConstellationSourceMF:
            sConstellationSource = HCDP_ConstellationSource_MF;
            break;
        case TDA10025_ConstellationSourceCAGC:
            sConstellationSource = HCDP_ConstellationSource_CAGC;
            break;
        case TDA10025_ConstellationSourceEqualizer:
            sConstellationSource = HCDP_ConstellationSource_Equalizer;
            break;
        case TDA10025_ConstellationSourceBEDR:
            sConstellationSource = HCDP_ConstellationSource_BEDR;
            break;
        default:
            err = TDA10025_ERR_NOT_SUPPORTED;
            break;
        }
    }

    if (err == TM_OK)
    {
        err = tmbslHCDP_ConstPreset(tUnit, sConstellationSource);
    }

    tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "iTDA10025_IQPreset(0x%08X) failed.", tUnit));

    return err;
}

#ifdef TMBSL_TDA10025_NO_64BITS
/*============================================================================*/
/* iTDA10025_PllFunction32                                                    */
/*============================================================================*/
static UInt32 iTDA10025_Division32(UInt32 ulNum1, UInt32 ulNum2, UInt16 ulDenom)
{
    UInt32 ulRes;
    UInt32 ulRemain;
    UInt32 ulTmp;
    UInt32 ulPart1 = 0;
    UInt32 ulNbPart1 = 0;

    if( (ulNum1 == 0) || (ulNum2 == 0) )
    {
        ulRes = 0;
    }
    else
    {
        ulRes = 0;
        ulRemain = 0;

        /* select the smaller of 2 numerators */
        if( ulNum2 < ulNum1 )
        {
            ulTmp = ulNum1;
            ulNum1 = ulNum2;
            ulNum2 = ulTmp;
        }

        ulPart1 = BITS32_MAX / ulNum1;

        if( ulPart1 < ulNum2 )
        {
            /* calculate number of part 1 */
            ulNbPart1 = ulNum2 / ulPart1;

            ulRes = (ulPart1 * ulNum1) / ulDenom;
            ulRes *= ulNbPart1;

            ulRemain = (ulPart1 * ulNum1) % ulDenom;
            ulRemain *= ulNbPart1;

            /* update  ulNum2 */
            ulNum2 -= (ulNbPart1 * ulPart1);
        }

        /* add the remaining part */
        ulRes += (ulNum2 * ulNum1) / ulDenom;
        ulRemain += (ulNum2 * ulNum1) % ulDenom;
        ulRemain /= ulDenom;

        ulRes += ulRemain;
    }

    return ulRes;
}
#endif

/*============================================================================*/
/* iTDA10025_SetPathConfiguration                                             */
/*============================================================================*/
tmErrorCode_t iTDA10025_SetPathConfiguration(
    tmUnitSelect_t       tUnit,
    TDA10025_IF_t        eIF,
    TDA10025_CDP_t       eCDP,
    TDA10025_TS_t        eTS )
{
    tmErrorCode_t      err = TM_OK;
    pTDA10025Object_t  pObj = Null;
    UInt16             uValue;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    if( err == TM_OK )
    {
        /* store new configuration */
        /* assume that all checks have been done previously */
        pObj->sCfg.eIF = eIF;
        pObj->sCfg.eCDP = eCDP;
        pObj->sCfg.eTS = eTS;

        if( pObj->sCfg.eCDP == TDA10025_CDP_0 ) 
        {
            /* configure the ADC route for CDP0 */
            /* TDA10025_ADC0_CFG_IND:  */
            /*   0: ADC0 - CDP0        */ 
            /*   1: ADC1 - CDP0        */
            if( pObj->sCfg.eIF == TDA10025_IF_0 )
            {
                /* connect ADC0 to CDP0 */
                uValue = (UInt16)~TDA10025_ADC_ROUTE_MSK;
            }
            else
            {
                /* connect ADC1 to CDP0 */
                uValue = TDA10025_ADC_ROUTE_MSK;
            }

            /* ADC 0 */
            err = iTDA10025_WriteBit(tUnit, TDA10025_ADC0_CFG_IND, TDA10025_ADC_ROUTE_MSK, uValue);
        }
        else
        {
            /* configure the ADC route for CDP1 */
            /* TDA10025_ADC1_CFG_IND:  */
            /*   0: ADC1 - CDP1        */
            /*   1: ADC0 - CDP1        */

            if( pObj->sCfg.eIF == TDA10025_IF_0 )
            {
                /* connect ADC0 to CDP1 */
                uValue = TDA10025_ADC_ROUTE_MSK;
            }
            else
            {
                /* connect ADC1 to CDP1  */
                uValue = (UInt16)~TDA10025_ADC_ROUTE_MSK;
            }

            /* ADC 1 */
            err = iTDA10025_WriteBit(tUnit, TDA10025_ADC1_CFG_IND, TDA10025_ADC_ROUTE_MSK, uValue);
            if ((err == TM_OK) && (pObj->eHwSample == TDA10025_HwSampleVersion_ES1))
            {
                /* workaround for ES1 */
                err = iTDA10025_WriteBit(tUnit, TDA10025_ADC0_CFG_IND, TDA10025_ADC_ROUTE_MSK, uValue);
            }
        }

        if (err == TM_OK)
        {
            /* TS mode configuration */
            if (pObj->sCfg.eTsMode == TDA10025_TsModeSerial)
            {
                /* serial configuration */
                uValue = (UInt16)~TDA10025_OUT_PARASER_MSK;
            }
            else
            {
                /* parallel configuration */
                uValue = TDA10025_OUT_PARASER_MSK;
            }

            if ((pObj->sCfg.eTS == TDA10025_TS_0 && pObj->sCfg.eCDP == TDA10025_CDP_0) || 
                (pObj->sCfg.eTS == TDA10025_TS_1 && pObj->sCfg.eCDP == TDA10025_CDP_1)) 
            {
                /* TS are not swapped */
                /* TS 0 and TS 1 */
                uValue &= (UInt16)~(TDA10025_TS_IF1_ROUTE_MSK | TDA10025_TS_IF0_ROUTE_MSK);
                err = iTDA10025_WriteBit(tUnit, TDA10025_TS_COMMON_CFG_IND, TDA10025_TS_IF1_ROUTE_MSK | TDA10025_TS_IF0_ROUTE_MSK | TDA10025_OUT_PARASER_MSK, uValue);
            }
            else /* TS outputs are interchanged */
            {
                /* TS are swapped */
                /* TS 0  and TS 1 */
                uValue |= TDA10025_TS_IF1_ROUTE_MSK | TDA10025_TS_IF0_ROUTE_MSK ;
                err = iTDA10025_WriteBit(tUnit, TDA10025_TS_COMMON_CFG_IND, TDA10025_TS_IF1_ROUTE_MSK | TDA10025_TS_IF0_ROUTE_MSK | TDA10025_OUT_PARASER_MSK, uValue);
            }
        }

        if (err == TM_OK)
        {
            /* Configure TS */
            uValue = pObj->sCfg.ulTsClkDiv;

            if( pObj->sCfg.eTsClkPol == TDA10025_TsCfg_ClkPolNormal)
            {
                /* no clk inversion -> 0 */
                uValue &= (UInt16)~TDA10025_OUT_CLKPOL_MSK;
            }
            else
            {
                /* clk inversion -> 1 */
                uValue |= TDA10025_OUT_CLKPOL_MSK;
            }

            if( pObj->sCfg.eTsClkDir == TDA10025_TsCfg_ClkDirIsOutput )
            {
                /* TS clk configured as output -> 0 */
                uValue &= (UInt16)~TDA10025_OUT_CLKDIR_MSK;
            }
            else
            { 
                /* TS clk configured as input -> 1 */
                uValue |= TDA10025_OUT_CLKDIR_MSK;
            }

            /* Configure TS at CDP output, whatever the TS path routing */
            if (pObj->sCfg.eCDP == TDA10025_CDP_0)
            {
                /* TS at CDP0 output */
                err = iTDA10025_WriteBit(tUnit, TDA10025_DEMOD0_TS_CFG_IND, TDA10025_OUT_CLKDIV_MSK | TDA10025_OUT_CLKPOL_MSK | TDA10025_OUT_CLKDIR_MSK, uValue);
            }
            else
            {
                /* TS at CDP1 output */
                err = iTDA10025_WriteBit(tUnit, TDA10025_DEMOD1_TS_CFG_IND, TDA10025_OUT_CLKDIV_MSK | TDA10025_OUT_CLKPOL_MSK | TDA10025_OUT_CLKDIR_MSK, uValue);
            }

            if (err == TM_OK)
            {
                if (pObj->sCfg.eTS == TDA10025_TS_0)
                {
                    /* enable TS0 */
                    uValue = TDA10025_ENA_TS0_MSK;
                    err = iTDA10025_WriteBit(tUnit, TDA10025_TS_COMMON_CFG_IND, TDA10025_ENA_TS0_MSK, uValue);
                }
                else
                {
                    /* enable TS1 */
                    uValue = TDA10025_ENA_TS1_MSK;
                    err = iTDA10025_WriteBit(tUnit, TDA10025_TS_COMMON_CFG_IND, TDA10025_ENA_TS1_MSK, uValue);
                }
            }
        }

        if (err == TM_OK)
        {
            /* Raise an error if the config is not allowed (although previously applied) */
            if ((pObj->sCfg.eTsMode == TDA10025_TsModeParallel) && (pObj->sCfg.eTsClkStdby == TDA10025_TsCfg_ClkInStdbyOn))
            {
                /* TS parallel mode is NOT allowed TS clock always enabled */
                /* because this mode cannot be detected in the tmbslOOBIP SetPowerState when the PLL is switched off */
                err = TDA10025_ERR_NOT_SUPPORTED;
            }
        }

        if (err == TM_OK)
        {
            /* Raise an error if the config is not allowed (although previously applied) */
            if (((pObj->sCfg.eTS == TDA10025_TS_0 && pObj->sCfg.eCDP == TDA10025_CDP_1) || 
                (pObj->sCfg.eTS == TDA10025_TS_1 && pObj->sCfg.eCDP == TDA10025_CDP_0)) &&
                (pObj->sCfg.eTsClkDir == TDA10025_TsCfg_ClkDirIsInput))
            {
                /* TS clock input mode is NOT compatible with CDP0->TS1 or CDP1->TS0 (hardware limitation) */
                err = TDA10025_ERR_NOT_SUPPORTED;
            }
        }

    }

    return err;
}