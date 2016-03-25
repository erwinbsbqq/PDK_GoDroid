#ifndef __PORTING_S3281_LINUX_H__
#define __PORTING_S3281_LINUX_H__



#include "../porting_linux_header.h"

#include "../basic_types.h"
#include "../tun_common.h"

#include "../nim_device.h"



#ifdef __cplusplus
extern "C" {
#endif


#define nim_print  	printk



#define SYS_FUNC_ON				0x00000001	/* Function on */
#define SYS_FUNC_OFF			0x00000000	/* Function disable */


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


#define FAST_TIMECST_AGC	1
#define SLOW_TIMECST_AGC	0
#define _1ST_I2C_CMD		0
#define _2ND_I2C_CMD		1


#define tuner_chip_sanyo        9
#define TUNER_CHIP_CD1616LF_GIH	8
#define tuner_chip_nxp		7
#define tuner_chip_maxlinear	6
#define tuner_chip_microtune	5
#define tuner_chip_quantek	4
#define tuner_chip_rfmagic  3
#define tuner_chip_alps		2	//60120-01Angus
#define tuner_chip_philips	1
#define tuner_chip_infineon	0






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

struct DEMOD_CONFIG_ADVANCED
{
    UINT32  qam_config_advanced; //bit0: demode_mode 0:j83b 1:j83ac;
                                 //bit1: ad sample clock 0: 27m, 1:54m;
    UINT32 qam_buffer_len;
    UINT32 qam_buffer_addr;
};





struct  nim_s3281_private
{

    /* struct for QAM Configuration */
    struct QAM_TUNER_CONFIG_DATA tuner_config_data;

	/* Tuner Initialization Function */
	dvbc_tuner_init_callback  nim_tuner_init;

	/* Tuner Parameter Configuration Function */
	tuner_control_callback nim_tuner_control;//since there will no bandwidth demand, so pass "sym" for later use.

	/* Get Tuner Status Function */
	tuner_status_callback nim_tuner_status;

	/* Close Function. */
    tuner_close_callback nim_tuner_close;


	

    /* Extension struct for Tuner Configuration */
    struct QAM_TUNER_CONFIG_EXT	tuner_config_ext;

    //struct QAM_TUNER_CONFIG_API TUNER_PRIV;
    UINT32 i2c_type;
    UINT32 tuner_id;
    UINT32 qam_mode;
    UINT32 qam_buffer_len;
    UINT32 qam_buffer_addr;
    struct mutex  		i2c_mutex;
    struct workqueue_struct	*workqueue;
    struct work_struct 		work;


};


typedef struct NIM_CHANNEL_CHANGE    NIM_CHANNEL_CHANGE_T;
               

#ifdef __cplusplus
}
#endif

#endif

