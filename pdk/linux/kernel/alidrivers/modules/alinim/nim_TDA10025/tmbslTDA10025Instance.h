/**
  Copyright (C) 2006 NXP B.V., All Rights Reserved.
  This source code and any compilation or derivative thereof is the proprietary
  information of NXP B.V. and is confidential in nature. Under no circumstances
  is this software to be  exposed to or placed under an Open Source License of
  any type without the expressed written permission of NXP B.V.
 *
 * \file          tmbslTDA10025Instance.h
 *                %version: 1 %
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
#ifndef _TMBSLTDA10025_INSTANCE_H
#define _TMBSLTDA10025_INSTANCE_H

tmErrorCode_t iTDA10025_AllocInstance (tmUnitSelect_t tUnit, ppTDA10025Object_t ppDrvObject);
tmErrorCode_t iTDA10025_DeAllocInstance (tmUnitSelect_t tUnit);
tmErrorCode_t iTDA10025_GetInstance (tmUnitSelect_t tUnit, ppTDA10025Object_t ppDrvObject);
tmErrorCode_t iTDA10025_ResetInstance(pTDA10025Object_t pDrvObject);

extern TDA10025PllConfig_t ES1PllSettings[];
extern TDA10025PllConfig_t ES2PllSettings[];

#endif /* _TMBSLTDA10025_INSTANCE_H */

