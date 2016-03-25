/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    nim_mxl241.h
*
*    Description:    Header file in alidriver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
* 1.20121018	  Coeus   Ver linux1.0 porting to ali linux system 
*****************************************************************************/

#ifndef __ALi_NIM_MXL241_H__
#define __ALi_NIM_MXL241_H__

#include "../porting_linux_header.h"

#include "MaxLinearDataTypes.h"

#include "MxL241SF_PhyCfg.h"
#include"MxL241SF_PhyCtrlApi.h"






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

//ID f_MXL241_IIC_Sema_ID = OSAL_INVALID_ID;

#define SUCCESS         0       /* Success return */

#define ERR_NO_MEM      -1      /* Not enough memory error */
#define ERR_LONG_PACK   -2      /* Package too long */
#define ERR_RUNT_PACK   -3      /* Package too short */
#define ERR_TX_BUSY     -4      /* TX descriptor full */
#define ERR_DEV_ERROR   -5      /* Device work status error */
#define ERR_DEV_CLASH   -6      /* Device clash for same device in queue */
#define ERR_QUEUE_FULL  -7      /* Queue node count reached the max. val*/
#define ERR_NO_DEV      -8      /* Device not exist on PCI */
#define ERR_FAILURE		-9      /* Common error, operation not success */
/* Compatible with previous written error*/
#define ERR_FAILUE		-9

#define ERR_PARA        -20     /* Parameter is invalid */
#define ERR_ID_FULL     -21     /* No more ID available */
#define ERR_ID_FREE     -22     /* Specified ID isn't allocated yet */

#define ERR_OFF_SCRN    -30     /* Block is out off the screen */
#define ERR_V_OVRLAP    -31     /* Block is overlaped in vertical */
#define ERR_BAD_CLR     -32     /* Invalid Color Mode code */
#define ERR_OFF_BLOCK   -33     /* Bitmap is out off the block */
#define ERR_TIME_OUT    -34     /* Waiting time out */

/* add by Sen */
#define ERR_FAILED		-40
#define ERR_BUSY		-41
#define ERR_ADDRESS		-42
/* end of Sen */

struct ali_nim_device
{
	struct cdev cdev;
	void *priv;
};

#define QAM_ONLY   0
#define QAM_SOC	1
#define ALI_NIM_DEVICE_NAME 	"ali_nim_mxl241"
#define NIM_MXL241_GET_BYTE(i)             (*(volatile UINT8 *)(i))
#define NIM_MXL241_SET_BYTE(i,d)          (*(volatile UINT8 *)(i)) = (d)

#define MXL241_QAM_ONLY_I2C_BASE_ADDR  	99 


#define SWITCH_NIM_MXL241_DEBUG	0

#define NIM_MXL241_ALL_LOCK							0x3F

struct ali_nim_mxl241_private
{

	/* struct for QAM Configuration */
	struct   QAM_TUNER_CONFIG_DATA tuner_config_data;

	/* Tuner Initialization Function */
	INT32 (*nim_tuner_init)(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

	/* Tuner Parameter Configuration Function */
	INT32 (*nim_tuner_control)(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);//since there will no bandwidth demand, so pass "sym" for later use.

	/* Get Tuner Status Function */
	INT32 (*nim_tuner_status)(UINT32 Tun_id, UINT8 *lock);

	/* Extension struct for Tuner Configuration */
	struct QAM_TUNER_CONFIG_EXT tuner_config_ext;
	
	//struct QAM_TUNER_CONFIG_API TUNER_PRIV;	

	UINT32 tuner_id;
	UINT32 qam_mode;
	
	UINT32 i2c_type_id;
	struct mutex i2c_mutex;
	//struct flag scan_flag;
	struct workqueue_struct *workqueue;
	struct work_struct work;
};


struct mxl241_Lock_Info
{	
	UINT32	Frequency;
	UINT32	SymbolRate;
	UINT8	Modulation;
};


#endif	/* __LLD_NIM_MXL241_H__ */




