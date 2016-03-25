/*
  Copyright (C) 2006-2009 NXP B.V., All Rights Reserved.
  This source code and any compilation or derivative thereof is the proprietary
  information of NXP B.V. and is confidential in nature. Under no circumstances
  is this software to be  exposed to or placed under an Open Source License of
  any type without the expressed written permission of NXP B.V.
 *
 * \file          System_Manage_HPMTO.c
 *
 *                3
 *
 * \date          %modify_time%
 *
 * \brief         Describe briefly the purpose of this file.
 *
 * REFERENCE DOCUMENTS :
 *                TDA18250_Driver_User_Guide.pdf
 *
 * Detailed description may be added here.
 *
 * \section info Change Information
 *
*/

/*============================================================================*/
/* Standard include files:                                                    */
/*============================================================================*/
#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"
#include "tmUnitParams.h"

/*============================================================================*/
/* Project include files:                                                     */
/*============================================================================*/
#include "System_Manage_HPMTO.h"
#include "tmddTDA18250.h"
#include "tmbslTDA18250.h"
#include "tmbslTDA18250InstanceCustom.h"
#include "tmbslTDA18250local.h"


/*============================================================================*/
/* FUNCTION:    tmSystem_Manage_HPMTO:                                        */
/*                                                                            */
/* DESCRIPTION: Set the HP MTO on the Master if Slave is analogue             */
/*              and its RF is superior to 250 Mhz                             */
/*                                                                            */
/* RETURN:      TM_OK if no error                                             */
/*                                                                            */
/* NOTES:                                                                     */
/*============================================================================*/
tmErrorCode_t tmSystem_Manage_HPMTO
(
    tmUnitSelect_t  TunerUnit   /* I: Tuner unit number */
)
{
    tmErrorCode_t               err  = TM_OK;
    UInt32                      uIndex0,uIndex2,uIndex3,uIndex;
    UInt32                      uRF;
    tmTDA18250StandardMode_t    StandardMode;
    UInt32                      TunerType;

    TunerType = UNIT_PATH_TYPE_GET (TunerUnit);
    uIndex0 = GET_INDEX_TUNIT(0)|UNIT_PATH_TYPE_VAL(TunerType);
    uIndex2 = GET_INDEX_TUNIT(2)|UNIT_PATH_TYPE_VAL(TunerType);
    uIndex3 = GET_INDEX_TUNIT(3)|UNIT_PATH_TYPE_VAL(TunerType);

    /* stop AGC1 loop */
    if (err == TM_OK)
    {
        err = TDA18250SetAGC1_loop_off(uIndex0, tmTDA18250_ON);
    }

    /* Manage HP MTO on Master if SEtRF for Slave2&3 in analogue with RF >250 MHz */
    if (err == TM_OK)
    {
        for (uIndex=uIndex2; uIndex<=uIndex3; uIndex++)
        {
            err = tmbslTDA18250GetRf (uIndex,&uRF);
            
            if (err == TM_OK)
            {
                err = tmbslTDA18250GetStandardMode (uIndex,&StandardMode);
            }

            if (err == TM_OK)
            {
                if (  (uRF >= 250000000) )   //kent:(StandardMode<=tmTDA18250_ANA_9MHz) &&
                {
                    err = TDA18250SetMTOClose(uIndex0, (UInt8) GET_INDEX_TUNIT(uIndex) );
                }
                else
                {
                    err = TDA18250SetMTOOpen(uIndex0, (UInt8) GET_INDEX_TUNIT(uIndex) );
                }
            }
        }
    }

    /* restart AGC1 loop */
    if (err == TM_OK)
    {
        err = TDA18250SetAGC1_loop_off(uIndex0, tmTDA18250_OFF);
    }

    return err;
}

