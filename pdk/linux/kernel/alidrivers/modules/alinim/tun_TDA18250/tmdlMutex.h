/**
  Copyright (C) 2008 NXP B.V., All Rights Reserved.
  This source code and any compilation or derivative thereof is the proprietary
  information of NXP B.V. and is confidential in nature. Under no circumstances
  is this software to be  exposed to or placed under an Open Source License of
  any type without the expressed written permission of NXP B.V.
 *
 * \file          tmdlI2C.h
 *                %version: 2 %
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
   9-JULY-2007  | A.TANT    |       |       | CREATION OF I2C ARCHITECTURE 2.0.0
   -------------|-----------|-------|-------|-----------------------------------
                |           |       |       |
   -------------|-----------|-------|-------|-----------------------------------
   \endverbatim
 *
*/


#ifndef TMDLMUTEX_H
#define TMDLMUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/*                       INCLUDE FILES                                        */
/*============================================================================*/

/*============================================================================*/
/*                       MACRO DEFINITION                                     */
/*============================================================================*/

/* SW Error codes */
#define MUTEX_ERR_BASE               (CID_COMP_TIMER | CID_LAYER_HWAPI)
#define MUTEX_ERR_COMP               (CID_COMP_TIMER | CID_LAYER_HWAPI | TM_ERR_COMP_UNIQUE_START)

#define MUTEX_ERR_INIT_FAILED        (MUTEX_ERR_BASE + TM_ERR_INIT_FAILED)
#define MUTEX_ERR_BAD_PARAMETER      (MUTEX_ERR_BASE + TM_ERR_BAD_PARAMETER)
#define MUTEX_ERR_NOT_SUPPORTED      (MUTEX_ERR_BASE + TM_ERR_NOT_SUPPORTED)
#define MUTEX_ERR_NOT_INSTALLED      (MUTEX_ERR_COMP + 0x0001)
#define MUTEX_ERR_ABANDONED          (MUTEX_ERR_COMP + 0x0002)


/*============================================================================*/
/*                       ENUM OR TYPE DEFINITION                              */
/*============================================================================*/


/*============================================================================*/
/*                       EXTERN DATA DEFINITION                               */
/*============================================================================*/



/*============================================================================*/
/*                       EXTERN FUNCTION PROTOTYPES                           */
/*============================================================================*/

extern tmErrorCode_t
tmdlMutexInit
(
    ptmbslFrontEndMutexHandle *ppMutex
);

extern tmErrorCode_t
tmdlMutexDeInit
(
    ptmbslFrontEndMutexHandle pMutex
);

extern tmErrorCode_t
tmdlMutexAcquire
(
    ptmbslFrontEndMutexHandle	pMutex,
    UInt32									 	timeOut    
);

extern tmErrorCode_t
tmdlMutexRelease
(
    ptmbslFrontEndMutexHandle pMutex
);

#ifdef __cplusplus
}
#endif

#endif /* TMDLMUTEX_H */
/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/

