#ifndef __ALI_VBI_COMMON_H
#define __ALI_VBI_COMMON_H

#include "ali_basic_common.h"

#define  VBI_IO		0x00000000
#define  VBI_CALL		0x00010000


#define IO_VBI_WORK_MODE_SEPERATE_TTX_SUBT              (VBI_IO + 1)
#define IO_VBI_WORK_MODE_HISTORY                        (VBI_IO + 2)
#define IO_VBI_ENGINE_OPEN                              (VBI_IO + 3)
#define IO_VBI_ENGINE_CLOSE                             (VBI_IO + 4)
#define IO_VBI_ENGINE_UPDATE_PAGE                       (VBI_IO + 5)
#define IO_VBI_ENGINE_SHOW_ON_OFF                       (VBI_IO + 6)
#define IO_VBI_ENGINE_SEND_KEY                          (VBI_IO + 7)
#define IO_VBI_ENGINE_GET_STATE                         (VBI_IO + 8)
#define IO_VBI_ENGINE_UPDATE_INIT_PAGE                  (VBI_IO + 9)
#define IO_VBI_ENGINE_UPDATE_SUBT_PAGE                  (VBI_IO + 10)
#define IO_VBI_ENGINE_SET_CUR_LANGUAGE                  (VBI_IO + 11)
#define IO_TTX_USER_DSG_FONT              				(VBI_IO + 12)
#define IO_VBI_SELECT_OUTPUT_DEVICE                     (VBI_IO + 13)

#define CALL_VBI_SETOUTPUT				(VBI_CALL+1)
#define CALL_VBI_START					(VBI_CALL+2)
#define CALL_VBI_STOP					(VBI_CALL+3)
#define CALL_TTX_DEFAULT_G0_SET		(VBI_CALL+4)
#define CALL_TTXENG_INIT				(VBI_CALL+5)
#define CALL_TTXENG_ATTACH			(VBI_CALL+6)
#define CALL_ENABLE_VBI_TRANSFER		(VBI_CALL+7)	
#define CALL_GET_INITAL_PAGE			(VBI_CALL+8)	
#define CALL_GET_INITAL_PAGE_STATUS	(VBI_CALL+9)	
#define CALL_GET_FIRST_TTX_PAGE		(VBI_CALL+10)	


#endif //__DRIVERS_ALI_RPC_HLD_SDEC_H

