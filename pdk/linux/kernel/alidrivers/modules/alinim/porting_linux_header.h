#ifndef __PORTING_LINUX_HEADER_H__
#define __PORTING_LINUX_HEADER_H__

#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <asm/irq.h>
#include <linux/ali_transport.h>
#include <linux/i2c.h>
#include <linux/wait.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/kthread.h>


#include <ali_dmx_common.h>
#include <linux/dvb/ali_i2c_scb_gpio.h>
#include <linux/ali_gpio.h>
#include <asm/mach-ali/typedef.h>
#include <dvb_frontend_common.h>
#include <ali_reg.h>

#include "basic_types.h"
#include "nim_device.h"

#include "../alii2c/gpio_i2c.h"


#define comm_malloc(x)				kmalloc((x),GFP_KERNEL)//kmalloc((x), GFP_ATOMIC)
#define comm_memset 				memset
#define comm_free 					kfree

#define comm_sleep 					msleep
#define comm_memcpy 				memcpy
#define nim_i2c_read				ali_i2c_read
#define nim_i2c_write				ali_i2c_write
#define nim_i2c_write_read			ali_i2c_write_read
#define comm_delay(x)				\
	do \
	{\
	   if(x>1000000)\
	   {\
	   	  mdelay(x/1000000);\
	   }\
	   else if(x>1000)\
	   {\
	   	  ndelay(x/1000);\
	   }\
	   else\
	   {\
	   	  udelay(x);\
	   }\
	}while(0)  
#define NIM_MUTEX_ENTER(priv)  \
	do \
	{ \
		mutex_lock(&priv->i2c_mutex); \
	}while(0)

#define NIM_MUTEX_LEAVE(priv) \
	do\
	{ \
		mutex_unlock(&priv->i2c_mutex);\
	}while(0)
extern UINT32 osal_get_tick(void);

/*!@struct COFDM_TUNER_CONFIG_API
@brief The structure passes the argument of tuner config data to driver from board config (for COFDM).
*/
typedef struct COFDM_TUNER_CONFIG_API
{	
	
	int (*nim_tuner_init) (__u32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrtuner_config);
	int (*nim_tuner_control) (__u32 tuner_id,__u32 freq, __u8 bandwidth,__u8 agc_time_const,__u8 *data,__u8 cmd_type);
	int (*nim_tuner_status) (__u32 tuner_id,__u8 *lock);
	int (*nim_tuner_close) ();
    union
    {
	    int (*nim_tuner_cal_agc) (__u32 tuner_id, __u8 flag, __u16 rf_val, __u16 if_val, __u8 *data);
        int (*nim_tuner_command)(__u32 tuner_id, int cmd, __u32 param);
    };
    void (*nim_lock_cb) (__u8 lock);
	struct COFDM_TUNER_CONFIG_DATA config_data;	
	struct COFDM_TUNER_CONFIG_EXT tuner_config;
	struct EXT_DM_CONFIG ext_dm_config;

    __u32 tuner_type;
    __u32 rev_id           : 8;
    __u32 config_mode      : 1;
    __u32 work_mode        : 1;    // NIM_COFDM_SOC_MODE or NIM_COFDM_ONLY_MODE
    __u32 ts_mode          : 2;    // enum nim_cofdm_ts_mode, only for NIM_COFDM_ONLY_MODE
    __u32 reserved         : 20;

	__u32 demod_index;                           //!<Multi demodulator control,the index of demodulator
	__u32 tuner_id;                              //!<The identifier of tuner
	
}*PCOFDM_TUNER_CONFIG_API;
#endif



