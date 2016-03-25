/**
Copyright (C) 2008 NXP B.V., All Rights Reserved.
This source code and any compilation or derivative thereof is the proprietary
information of NXP B.V. and is confidential in nature. Under no circumstances
is this software to be  exposed to or placed under an Open Source License of
any type without the expressed written permission of NXP B.V.
*
* \file          tmbslTDA10025Instance.c
*                %version: CFR_FEAP#7 %
*
* \date          %date_modified%
*
* \brief         Describe briefly the purpose of this file.
*
* REFERENCE DOCUMENTS :
*
* Detailed description may be added here.
*
* \section info Change Information
*
* \verbatim
Date          Modified by CRPRNr  TASKNr  Maintenance description
-------------|-----------|-------|-------|-----------------------------------
26-Nov-2007  | V.VRIGNAUD|       |       | COMPATIBILITY WITH NXPFE
             | C.CAZETTES|       |       | 
-------------|-----------|-------|-------|-----------------------------------
             |           |       |       |
-------------|-----------|-------|-------|-----------------------------------
\endverbatim
*
*/

#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmbslFrontEndCfgItem.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"

/* TDA10025 Driver includes */
#include "tmbslHCDP.h"

#include "tmbslTDA10025.h"
#include "tmbslTDA10025local.h"

#include "tmbslTDA10025Instance.h"
#include "tmbslTDA10025_Cfg.h"

/* define default configuration */
#define TDA10025_INSTANCE_DEFAULT \
    (tmUnitSelect_t)(-1),                   /* tUnit           */ \
    (tmUnitSelect_t)(-1),                   /* tUnitW          */ \
    (tmUnitSelect_t)(-1),                   /* tUnitCommon     */ \
    (tmUnitSelect_t)(-1),                   /* tUnitOtherStream */ \
    False,                                  /* init            */ \
    {                                       /* sIo             */ \
        Null, \
        Null, \
    }, \
    {                                       /* sTime           */ \
        Null, \
        Null, \
    }, \
    {                                       /* sDebug          */ \
        Null, \
    }, \
    {                                       /* sMutex          */ \
        Null, \
        Null, \
        Null, \
        Null, \
    }, \
    Null,                                   /* pMutex           */ \
    TDA10025_HwSampleVersion_Unknown,       /* eHwSample        */ \
    {                                       /* sIP for HCDP IP  */ \
        tmPowerStandby,                     /* ePowerState      */ \
        0,                                  /* uIFAGCThreshold  */ \
        0,                                  /* uRFAGCThreshold  */ \
        TDA10025_ConstellationSourceUnkown, /* sConstSource     */ \
        False,                              /* bUncorPresent    */ \
    }, \
    TDA10025_ChannelSelection_tUnknown,     /* eChannelSel      */ \
    tmPowerStandby,                         /* ePowerState      */ \
    {                                       /* sPllConfig       */ \
        0,                                  /* uXtal            */ \
        0,                                  /* uPLLMFactor      */ \
        0,                                  /* lPLLNFactor      */ \
        0,                                  /* bPLLPFactor      */ \
    }

/*----------------------------------------------------------------------------*/
/* Global data:                                                               */
/*----------------------------------------------------------------------------*/
TDA10025Object_t gTDA10025Instance[]= 
{
    {TDA10025_INSTANCE_DEFAULT, {TMBSL_TDA10025_INSTANCE_CUSTOM_0}},
    {TDA10025_INSTANCE_DEFAULT, {TMBSL_TDA10025_INSTANCE_CUSTOM_1}},
    {TDA10025_INSTANCE_DEFAULT, {TMBSL_TDA10025_INSTANCE_CUSTOM_2}},
    {TDA10025_INSTANCE_DEFAULT, {TMBSL_TDA10025_INSTANCE_CUSTOM_3}},
    {TDA10025_INSTANCE_DEFAULT, {TMBSL_TDA10025_INSTANCE_CUSTOM_4}},
    {TDA10025_INSTANCE_DEFAULT, {TMBSL_TDA10025_INSTANCE_CUSTOM_5}}
};

/* PLL configuration table */
TDA10025PllConfig_t ES1PllSettings[TDA18265_ES1_PLL_CONFIG_NB]= { TDA18265_ES1_PLL_CONFIG };
TDA10025PllConfig_t ES2PllSettings[TDA18265_ES2_PLL_CONFIG_NB] = { TDA18265_ES2_PLL_CONFIG };

/*============================================================================*/
/* FUNCTION:    tmbsliTDA10025_AllocInstance:                                   */
/*                                                                            */
/* DESCRIPTION: allocate new instance                                         */
/*                                                                            */
/* RETURN:                                                                    */
/*                                                                            */
/* NOTES:                                                                     */
/*============================================================================*/
tmErrorCode_t
iTDA10025_AllocInstance
(
 tmUnitSelect_t       tUnit,         /* I: Unit number   */
 ppTDA10025Object_t ppDrvObject    /* I: Device Object */
 )
{ 
    tmErrorCode_t       err = TDA10025_ERR_BAD_UNIT_NUMBER;
    pTDA10025Object_t   pObj = Null;
    UInt32              uLoopCounter = 0;    

    /* Find a free instance */
    for(uLoopCounter = 0; uLoopCounter<TDA10025_MAX_UNITS; uLoopCounter++)
    {
        pObj = &gTDA10025Instance[uLoopCounter];
        if(pObj->init == False)
        {
            pObj->tUnit = tUnit;
            pObj->tUnitW = tUnit;

            err = iTDA10025_ResetInstance(pObj);

            *ppDrvObject = pObj;
            err = TM_OK;
            break;
        }
    }

    return err;
}

/*============================================================================*/
/* FUNCTION:    tmbsliTDA10025_DeAllocInstance                                */
/*                                                                            */
/* DESCRIPTION: deallocate instance                                           */
/*                                                                            */
/* RETURN:      always TM_OK                                                  */
/*                                                                            */
/* NOTES:                                                                     */
/*============================================================================*/
tmErrorCode_t
iTDA10025_DeAllocInstance
(
 tmUnitSelect_t  tUnit    /* I: Unit number */
 )
{     
    tmErrorCode_t       err = TDA10025_ERR_BAD_UNIT_NUMBER;
    pTDA10025Object_t pObj = Null;

    /* check input parameters */
    err = iTDA10025_GetInstance(tUnit, &pObj);

    /* check driver state */
    if (err == TM_OK)
    {
        if (pObj == Null || pObj->init == False)
        {
            err = TDA10025_ERR_NOT_INITIALIZED;
        }
    }

    if ((err == TM_OK) && (pObj != Null)) 
    {
        pObj->init = False;
    }

    /* return value */
    return err;
}

/*============================================================================*/
/* FUNCTION:    iTDA10025_GetInstance                                         */
/*                                                                            */
/* DESCRIPTION: get the instance                                              */
/*                                                                            */
/* RETURN:      always True                                                   */
/*                                                                            */
/* NOTES:                                                                     */
/*============================================================================*/
tmErrorCode_t
iTDA10025_GetInstance 
(
 tmUnitSelect_t       tUnit,         /* I: Unit number   */
 ppTDA10025Object_t ppDrvObject    /* I: Device Object */
 )
{     
    tmErrorCode_t       err = TDA10025_ERR_NOT_INITIALIZED;
    pTDA10025Object_t   pObj = Null;
    UInt32              uLoopCounter = 0;    

    /* Find a free instance */
    for(uLoopCounter = 0; uLoopCounter<TDA10025_MAX_UNITS; uLoopCounter++)
    {
        pObj = &gTDA10025Instance[uLoopCounter];
        if(pObj->init == True && pObj->tUnit == tUnit)
        {
            *ppDrvObject = pObj;
            err = TM_OK;
            break;
        }
    }

    return err;
}

/*============================================================================*/
/* FUNCTION:    iTDA10025_ResetInstance:                                      */
/*                                                                            */
/* DESCRIPTION: Resets an instance.                                           */
/*                                                                            */
/* RETURN:      TM_OK if no error                                             */
/*                                                                            */
/* NOTES:                                                                     */
/*============================================================================*/
tmErrorCode_t
iTDA10025_ResetInstance(
    pTDA10025Object_t  pDrvObject  /* I: Driver Object */
)
{
    tmErrorCode_t   err = TM_OK ;

    pDrvObject->sIP.uIFAGCThreshold = 0xD;

    pDrvObject->ePowerState = tmPowerMax; /* identify the power-up state */

    return err;
}

