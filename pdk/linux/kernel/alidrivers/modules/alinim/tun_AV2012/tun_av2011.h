/*****************************************************************************
*    Copyright (C) 2010 ALi Corp. All Rights Reserved.
*    
*    Company confidential and Properietary information.       
*    This information may not be disclosed to unauthorized  
*    individual.    
*    File: nim_av2011.h
*   
*    Description: 
*    
*    History: 
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2010/4/7      Roman         
*        
*****************************************************************************/
#ifndef __NIM_AV2011_H
#define __NIM_AV2011_H

#include "../basic_types.h"
#include "../porting_linux_header.h"


#ifdef __cplusplus
extern "C"
{
#endif

INT32 ali_nim_av2011_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 ali_nim_av2011_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 ali_nim_av2011_status(UINT32 tuner_id, UINT8 *lock);
INT32 ali_nim_av2011_close(void);

#ifdef __cplusplus
}
#endif

#endif  /* __NIM_AV2011_H */


