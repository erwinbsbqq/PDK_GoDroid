/*****************************************************************************
*   Copyrights(C) 2012 ALi corp. All Rights Reserved.
*
*   FILE NAME:      adr_deca_dbg.c
*
*   DESCRIPTION:    Deca debug module.
*
*   HISTORY:
*                      Date      Author      Version      Notes
*                   ==========  =========   =========   ===========
*                   2012-10-24               =.=                   Ver 1.0                Create File
*****************************************************************************/
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include <osal/osal_task.h>

#include <hld/adr_hld_dev.h>

#include <hld_cfg.h>
#include <adr_mediatypes.h>

#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld/deca/adr_deca_dev.h>
#include <hld/deca/adr_deca.h>
#include <hld/snd/adr_snd_dev.h>
#include <hld/snd/adr_snd.h>

#include <hld/dbg/adr_dbg_parser.h>

#define __ADR_DECA_DBG
#ifdef __ADR_DECA_DBG
#define DECA_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DECA, fmt, ##args); \
			} while(0)
#else
#define DECA_DBG_PRINT(...)
#endif

#define DECA_DBG_PRINT_INTRV 5000

typedef struct deca_dbg_stat_ctrl
{
	int      dbg_dev_fd;
	UINT32   dbg_task_id;
	struct deca_device *dbg_dev;
	UINT8    dbg_init;
	UINT8    dbg_enable;
	UINT8    dbg_task_enable;

      UINT8 dbg_get_dev_status_en;
     UINT8 dbg_get_buf_status_en;
    
	UINT32   dbg_show_intv;  //Unit: ms default 5000ms
	UINT32   dbg_show_ims;
} deca_dbg_stat_ctrl;


enum Deca_Test_Case{
	PUSH_ES_DATA = 1,
	PLAY_MP3,
	PLAY_PCM
};
