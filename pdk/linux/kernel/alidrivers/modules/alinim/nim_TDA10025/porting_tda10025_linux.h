#ifndef __PORTING_TDA10025_LINUX_H__
#define __PORTING_TDA10025_LINUX_H__


#include "../porting_linux_header.h"


#include "../basic_types.h"
#include "../tun_common.h"
#include "../nim_device.h"
#include "nim_tda10025.h"




#define MAX_TUNER_SUPPORT_NUM 	3

#if 0
#define TDA10025_PRINTF nim_print   //(x...) printk(KERN_INFO x)
#else
#define TDA10025_PRINTF(...) do{}while(0)
#endif



#define nim_print  	printk







#define comm_malloc(x)				kmalloc((x),GFP_KERNEL)//kmalloc((x), GFP_ATOMIC)
#define comm_memset 				memset
#define comm_free 					kfree
#define comm_delay					udelay
#define comm_sleep 					msleep
#define comm_memcpy 				memcpy
#define nim_i2c_read				ali_i2c_read
#define nim_i2c_write				ali_i2c_write
#define nim_i2c_write_read			ali_i2c_write_read

#define CHANNEL_CHANGE_ASYNC
#define OSAL_TWF_ORW				0
#define OSAL_TWF_ANDW 				0
#define OSAL_TWF_CLR				0

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


/*
struct nim_device
{
	struct cdev cdev;
	void *priv;
};

*/


typedef struct NIM_CHANNEL_CHANGE    NIM_CHANNEL_CHANGE_T;	

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

	/* Open  Function*/
	INT32 (*nim_Tuner_Open)(UINT32 Tun_id);

	/* Close Function. */
	INT32 (*nim_tuner_close)(UINT32 Tun_id);
	/* END */


	/* Extension struct for Tuner Configuration */
	struct QAM_TUNER_CONFIG_EXT tuner_config_ext;
	struct EXT_DM_CONFIG        ext_dem_config;
	//struct QAM_TUNER_CONFIG_API TUNER_PRIV;	

	UINT32 tuner_id;
	UINT32 qam_mode;
	
	UINT32 i2c_type_id;
	struct mutex i2c_mutex;
	//struct flag scan_flag;
	struct workqueue_struct *workqueue;
	struct work_struct work;
	UINT8  dev_idx;
};


INT32 	nim_tda10025_channel_change(struct nim_device *dev, NIM_CHANNEL_CHANGE_T *pstChl_Change );

#endif

