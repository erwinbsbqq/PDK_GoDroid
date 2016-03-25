/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan.h
*
*    Description:    This file contains all functions definition
*		             of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Apr.21.2003      Justin Wu       Ver 0.1    Create file.
* 	2.  Dec.19.2003		 Justin Wu		 Ver 0.2    Add ESC CMD macros.
*	3.  Sep.23.2005		 Justin Wu		 Ver 0.3    Add pan information.
*****************************************************************************/

#ifndef __ADR_HLD_SMC_PRIV_H__
#define __ADR_HLD_SMC_PRIV_H__


#ifndef __ASSEMBLER__

#include <hld_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum smc_dbg_level
{	
	SMC_DBG_LEVEL_DEFAULT = 0x00,
	SMC_DBG_LEVEL_HLD 	  = 0x01,
	SMC_DBG_LEVEL_KERNEL = 0x02,
}smc_dbg_level_e;

struct smc_dbg
{		
	smc_dbg_level_e level;		
};

struct smc_hld_private
{
	INT32 smc_hdl;
	UINT32 rfk_port;
	OSAL_ID task_id;
};


#ifdef __cplusplus
}
#endif
#endif

/*!
 @}
 */
#endif /* __ADR_HLD_SMC_PRIV_H__ */
