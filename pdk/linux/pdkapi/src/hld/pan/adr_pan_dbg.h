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

#ifndef __ADR_HLD_PAN_DBG_H__
#define __ADR_HLD_PAN_DBG_H__


#ifndef __ASSEMBLER__

#include <hld/pan/adr_pan_dev.h>
#include <ali_front_panel_common.h>

#ifdef __cplusplus
extern "C"
{
#endif


/*!@struct pan_dbg
   @brief 前面板调试级别。
*/

typedef enum pan_dbg_level
{	
	PAN_DBG_LEVEL_DEFAULT = 0x00,
	PAN_DBG_LEVEL_HLD 	  = 0x01,
    	PAN_DBG_LEVEL_KERNEL = 0x02,
}pan_dbg_level_e;


/*!@struct pan_dbg
   @brief 前面板按键信息。
*/
struct pan_dbg
{
	UINT8 repeat_enable;
	pan_device_status_e stats;

	/* ir */
	INT32 ir_handle;	
	INT32 input_handle_ir;	
	UINT32 ir_last_key_high;
	UINT32 ir_last_key_low;	
	UINT32 ir_format;		
	UINT32 auto_change;					/* auto change channel */
	UINT32 interval;						/* auto change channel interval */
	pan_dbg_level_e  ir_level;	
	struct ali_fp_key_map_cfg ir_key_map;		/* ir key map */

	/* panel */
	INT32 panel_handle;
	INT32 input_handle_panel;	
	UINT32 panel_last_key_high;
	UINT32 panel_last_key_low;	
	pan_dbg_level_e  panel_level;	
	struct ali_fp_key_map_cfg panel_key_map;		/* panel key map */
};

#ifdef __cplusplus
}
#endif
#endif

/*!
 @}
 */
#endif /* __ADR_HLD_PAN_DBG_H__ */
