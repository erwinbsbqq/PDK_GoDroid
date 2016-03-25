/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2008 Copyright (C)
*
*    File:	slot.S
*
*    Description:	This file contains all code in standby mode.
*    History:
*	Date 		Athor		Version		Reason
*	============	=======================================
*	1.07.04.2008	Mao Feng 	Ver 0.1		Create file.
*	
*****************************************************************************/
//#include <hal/mips.h>
//#include <hld/pan/pan.h>
//#include <sys_config.h>

#include "m36_sleep.h"
#include "mips.h"

/***************************************************************
*Function Name:	sys_prestandby
*Arguments: 	
*		
*Description:	Initialize IR & I2C before goes into standby mode.
*
***************************************************************/
		.globl	sys_prestandby
		.ent	sys_prestandby
		.set	noreorder
sys_prestandby:
		jr	ra
		nop
		.end sys_prestandby


		.globl	sys_poststandby
		.ent	sys_poststandby
		
sys_poststandby:
		jr		ra
		nop
		.end sys_poststandby

/***************************************************************
*Function Name:	sys_standby_process
*		
*Description:			
*
***************************************************************/
		.globl	sys_standby_process
		.ent	sys_standby_process
		.set	noreorder	
sys_standby_process:
		jr	ra
		nop
		.end sys_standby_process



