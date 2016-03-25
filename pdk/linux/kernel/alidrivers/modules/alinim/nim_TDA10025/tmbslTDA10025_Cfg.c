/**
 * Copyright (C) 2005 Koninklijke Philips Electronics N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of Koninklijke Philips Electronics N.V. and is confidential in
 * nature. Under no circumstances is this software to be  exposed to or placed
 * under an Open Source License of any type without the expressed written
 * permission of Koninklijke Philips Electronics N.V.
 *
 * \file          tmbslTDA10025_.c
 *
 * \date          26-Nov-2007
 *
 * \brief         -
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
                |           |       |       | 
   -------------|-----------|-------|-------|-----------------------------------
                |           |       |       |
   -------------|-----------|-------|-------|-----------------------------------
   \endverbatim
 *
*/


/*============================================================================*/
/*                   STANDARD INCLUDE FILES                                   */
/*============================================================================*/


/*============================================================================*/
/*                   PROJECT INCLUDE FILES                                    */
/*============================================================================*/
#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"

#include "tmbslTDA10025.h"
#include "tmbslTDA10025_Cfg.h"
#include "tmbslTDA10025local.h"




/*============================================================================*/
/*                       MACRO DEFINITION                                     */
/*============================================================================*/
#define TDA10025_Cfg_COMPATIBILITY_NB 8

#if TDA10025_Cfg_VERSION != TDA10025_Cfg_COMPATIBILITY_NB
    #error "Wrong cfg files version !!!!"
#endif

/*============================================================================*/
/*                   PUBLIC FUNCTIONS DEFINITIONS                             */
/*============================================================================*/

/*============================================================================*/
/*                     END OF FILE                                            */
/*============================================================================*/

