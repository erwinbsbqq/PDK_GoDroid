/*****************************************************************************
*	Copyrights(C) 2012 Acer Laboratries inc. All Rights Reserved.
*
*	FILE NAME:		adr_dbg_soc.c
*
*	DESCRIPTION:	Debug SOC module.
*
*	HISTORY:
*						Date 	 Author      Version 	  Notes
*					=========	=========	=========	===========
*					2012-09-30	 Leo.Ma      Ver 1.0	Create File.
*					2012-10-31	 John.CHen   Ver 1.1	Add soc dbg module.
*					2013-05-17	 Leo.Ma      Ver 1.2	Modify the arch.
*****************************************************************************/

#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <hld_cfg.h>
#include <hld/dbg/dbg_parser.h>
#include <version.h>
#include<sys/mman.h>  


//#define __ADR_DBG_SOC

#define MAX_MEM_SIZE	(0x3FFFFFFF)



#define SOC_INFO(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(SOC, fmt, ##args); \
			} while(0)

#define SOC_ERR(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(SOC,fmt, ##args); \
			} while(0)

#ifdef __ADR_DBG_SOC
	#define ADR_DBG_SOC_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(SOC, fmt, ##args); \
			} while(0)
#else
	#define ADR_DBG_SOC_PRINT(...)
#endif

typedef enum socdbg_level
{
	DBG_LEVEL_DEFAULT = 0x00,
	DBG_LEVEL_HLD 	  = 0x01,
    DBG_LEVEL_KERNEL = 0x02,
} socdbg_level_e;
INT32 ir_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt);
INT32 pan_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 *list_cnt);

static DEBUG_MODULE_LIST debug_module[] = {
		{ { NULL, NULL }, "ir", 0, (GET_CMD_FP)ir_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "pan", 0, (GET_CMD_FP)pan_dbg_cmd_get, { NULL, NULL } },
};
