#ifndef __LLD_TUN_MXL603_LINUX_H__
#define __LLD_TUN_MXL603_LINUX_H__


#include "../porting_linux_header.h"
#include "../basic_types.h"
#include "tun_mxl603.h"


#include "MaxLinearDataTypes.h"
//#include "MxL603_TunerApi.h"
//#include "MxL603_TunerCfg.h"


#ifdef __cplusplus
extern "C"
{
#endif




#define SYS_TUN_MODULE             ANY_TUNER


#define nim_print  					printk

#define comm_malloc(x)				kmalloc((x),GFP_KERNEL)//kmalloc((x), GFP_ATOMIC)
#define comm_memset 				memset
#define comm_free 					kfree
#define comm_delay					udelay
#define comm_sleep 					msleep
#define comm_memcpy 				memcpy

#define nim_i2c_read				ali_i2c_read
#define nim_i2c_write				ali_i2c_write
#define nim_i2c_write_read			ali_i2c_write_read


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


typedef INT32  (*INTERFACE_DEM_WRITE_READ_TUNER)(void * nim_dev_priv, UINT8 tuner_address, \
                 UINT8 *wdata, int wlen, UINT8* rdata, int rlen);
typedef struct
{
    void * nim_dev_priv; //for support dual demodulator.   
    INTERFACE_DEM_WRITE_READ_TUNER  dem_write_read_tuner;
} DEM_WRITE_READ_TUNER;  //Dem control tuner by through mode (it's not by-pass mode).





/******************************************************************************
    Global Variable Declarations
******************************************************************************/
extern void * MxL603_OEM_DataPtr[];
/*****************************************************************************
    Prototypes
******************************************************************************/
#if 0
INT32 tun_mxl603_init_DVBC(UINT32 *tuner_id , struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_mxl603_control_DVBC(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 tun_mxl603_status(UINT32 tuner_id, UINT8 *lock);


INT32 tun_mxl603_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_mxl603_init_ISDBT(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)  ;
INT32 tun_mxl603_init_ISDBT(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)  ;
INT32 tun_mxl603_init_CDT_MN88472(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config) ;
INT32 tun_mxl603_control(UINT32 tuner_id, UINT32 freq, UINT32 bandwidth)	;


MXL_STATUS MxLWare603_OEM_WriteRegister(UINT32 tuner_id, UINT8 regAddr, UINT8 regData);
MXL_STATUS MxLWare603_OEM_ReadRegister(UINT32 tuner_id, UINT8 regAddr, UINT8 *regDataPtr);
void MxLWare603_OEM_Sleep(UINT16 delayTimeInMs);  
#endif




#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_TDA18250_H__ */


