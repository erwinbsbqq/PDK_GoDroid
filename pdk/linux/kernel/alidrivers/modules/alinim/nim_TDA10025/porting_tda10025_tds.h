#ifndef _PORTING_TDA10025_TDS_H_
#define _PORTING_TDA10025_TDS_H_

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>
#include <osal/osal.h> 
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <bus/i2c/i2c.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim.h>
#include <bus/tsi/tsi.h>

#include "nim_tda10025.h"

#if 0
#define TDA10025_PRINTF nim_print   //(x...) printk(KERN_INFO x)
#else
#define TDA10025_PRINTF(...) do{}while(0)
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

#define div_u64(a,b)			a=(a/b)
//div_u64(ulHigh, pObj->sConfig.uSamplingClock); 

void 							nim_comm_delay(UINT32 us);



#define NIM_MUTEX_ENTER(priv)  	os_lock_mutex(priv->i2c_mutex, OSAL_WAIT_FOREVER_TIME)
#define NIM_MUTEX_LEAVE(priv) 	os_unlock_mutex(priv->i2c_mutex)








struct nim_tda10025_private
{

	/* struct for QAM Configuration */
	struct   QAM_TUNER_CONFIG_DATA tuner_config_data;

	/* Tuner Initialization Function */
	INT32 (*nim_tuner_init)(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

	/* Tuner Parameter Configuration Function */
	INT32 (*nim_tuner_control)(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);

	/* Get Tuner Status Function */
	INT32 (*nim_tuner_status)(UINT32 Tun_id, UINT8 *lock);


	/* Close Function. */
	INT32 (*nim_tuner_close)(UINT32 Tun_id);
	/* END */

	/* Extension struct for Tuner Configuration */
	struct QAM_TUNER_CONFIG_EXT tuner_config_ext;
	struct EXT_DM_CONFIG        ext_dem_config;
	UINT32 tuner_id;
	UINT32 qam_mode;
	
	UINT32 i2c_type_id;
	
	UINT32 i2c_mutex;
};


typedef struct NIM_CHANNEL_CHANGE    NIM_CHANNEL_CHANGE_T;

INT32 	nim_tda10025_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner);
INT32 	nim_tda10025_channel_change(struct nim_device *dev, NIM_CHANNEL_CHANGE_T *pstChl_Change );

#endif

