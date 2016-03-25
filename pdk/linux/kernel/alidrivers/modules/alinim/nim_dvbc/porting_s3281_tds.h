#ifndef _PORTING_S3281_TDS_H_
#define _PORTING_S3281_TDS_H_

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
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


#define nim_print          libc_printf


#define comm_malloc 			MALLOC
#define comm_memset				MEMSET
#define comm_free				FREE
#define comm_delay				nim_comm_delay
#define comm_sleep				osal_task_sleep
#define comm_memcpy				memcpy

#define nim_i2c_read			i2c_read
#define nim_i2c_write			i2c_write
#define nim_i2c_write_read		i2c_write_read

#define div_u64(a,b)			a=(a/b)
//div_u64(ulHigh, pObj->sConfig.uSamplingClock);

void                             nim_comm_delay(UINT32 us);



#define NIM_MUTEX_ENTER(priv)      os_lock_mutex(priv->i2c_mutex, OSAL_WAIT_FOREVER_TIME)
#define NIM_MUTEX_LEAVE(priv)     os_unlock_mutex(priv->i2c_mutex)




struct nim_tsk_status
{
    UINT32                    m_lock_flag;
    ID                        m_task_id;
    UINT32                    m_sym_rate;
    UINT8                     m_work_mode;
    UINT8                     m_map_type;
    UINT8                     m_code_rate;
    UINT8                     m_info_data;
};
struct nim_s3281_private
{

    /* struct for QAM Configuration */
    struct QAM_TUNER_CONFIG_DATA tuner_config_data;

    /* Tuner Initialization Function */
    INT32                         (*nim_tuner_init)(UINT32 *ptr_tun_id, struct QAM_TUNER_CONFIG_EXT *ptr_tuner_config);

    /* Tuner Parameter Configuration Function */
    //since there will no bandwidth demand, so pass "sym" for later use.
    INT32 (*nim_tuner_control)(UINT32 tun_id, UINT32 freq, UINT32 sym);

    /* Get Tuner Status Function */
    INT32 (*nim_tuner_status)(UINT32 tun_id, UINT8 *lock);

    /* Extension struct for Tuner Configuration */
    struct QAM_TUNER_CONFIG_EXT   tuner_config_ext;

    //struct QAM_TUNER_CONFIG_API TUNER_PRIV;
    struct nim_tsk_status          tsk_status;


    UINT32 i2c_type;
    UINT32 tuner_id;
    UINT32 qam_mode;
    UINT32 qam_buffer_len;
    UINT32 qam_buffer_addr;
    UINT32 i2c_mutex;


};



typedef struct NIM_CHANNEL_CHANGE    NIM_CHANNEL_CHANGE_T;




#ifdef __cplusplus
}
#endif

#endif

