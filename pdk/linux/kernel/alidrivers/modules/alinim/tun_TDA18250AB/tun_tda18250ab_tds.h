#ifndef __LLD_TUN_TDA18250_TDS_H__
#define __LLD_TUN_TDA18250_TDS_H__

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_tuner.h>
#include <bus/i2c/i2c.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>

#include <api/libdiseqc/lib_diseqc.h>
#include <api/libsi/si_tdt.h>


#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"
#include "tmddTDA18250.h"
#include "tmbslTDA18250.h"
#include "tun_tda18250.h"


#ifdef __cplusplus
extern "C"
{
#endif







#define nim_print  		libc_printf



#define comm_malloc 			MALLOC
#define comm_memset				MEMSET
#define comm_free				FREE
#define comm_delay				nim_comm_delay
#define comm_sleep				osal_task_sleep
#define comm_memcpy				MEMCPY

#define nim_i2c_read			i2c_read
#define nim_i2c_write			i2c_write
#define nim_i2c_write_read		i2c_write_read


#define osal_mutex_create			os_create_mutex
#define osal_mutex_delete			os_delete_mutex
#define osal_mutex_lock				os_lock_mutex
#define osal_mutex_unlock			os_unlock_mutex



void 		nim_comm_delay(UINT32 us);







#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_TDA18250_H__ */


