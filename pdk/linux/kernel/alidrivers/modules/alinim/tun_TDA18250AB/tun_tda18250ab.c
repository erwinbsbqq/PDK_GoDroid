/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_tda18250.c
*
*    Description:    This file contains tda18250 basic function in LLD. 
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20080520		Trueve Hu		 Ver 0.1	Create file.
*	
*****************************************************************************/

/*
  Copyright (C) 2006-2009 NXP B.V., All Rights Reserved.
  This source code and any compilation or derivative thereof is the proprietary
  information of NXP B.V. and is confidential in nature. Under no circumstances
  is this software to be  exposed to or placed under an Open Source License of
  any type without the expressed written permission of NXP B.V.
 *
 * \file          main.c
 *
 *                3
 *
 * \date          %modify_time%
 *
 * \brief         Sample application code of the NXP TDA18250 driver.
 *
 * REFERENCE DOCUMENTS :
 *                TDA18250A_Driver_User_Guide.pdf
 *
 * Detailed description may be added here.
 *
 * \section info Change Information
 *
*/

//*--------------------------------------------------------------------------------------
//* Include Standard files
//*--------------------------------------------------------------------------------------
#if defined(__NIM_LINUX_PLATFORM__)
#include "tun_tda18250ab_linux.h"
#elif defined(__NIM_TDS_PLATFORM__)
#include "tun_tda18250ab_tds.h"
#endif


#define TDA18250AB_PRINTF(...)
//#define TDA18250AB_PRINTF		printk

#define MAX_TUNER_SUPPORT_NUM 4


static struct QAM_TUNER_CONFIG_EXT * tda18250ab_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 tda18250ab_tuner_cnt = 0;

static UINT32 tda18250ab_tuner_i2c = 0;//I2C_TYPE_SCB0;
static UINT8  tda18250ab_i2c_addr = 0;
static UINT8  t_val=0x00;
static struct tuner_handle *tda18250ab_model = NULL;

//*--------------------------------------------------------------------------------------
//* Prototype of function to be provided by customer
//*--------------------------------------------------------------------------------------
// I2C functions, must be provided by user
static tmErrorCode_t UserWrittenRead_18250ab(tmUnitSelect_t tUnit, UInt32 AddrSize, UInt8* pAddr, UInt32 ReadLen, UInt8* pData);
static tmErrorCode_t UserWrittenWrite_18250ab(tmUnitSelect_t tUnit, UInt32 AddrSize, UInt8* pAddr, UInt32 WriteLen, UInt8* pData);
static tmErrorCode_t UserWrittenWait_18250ab(tmUnitSelect_t FrontEndUnit, UInt32 tms);
static tmErrorCode_t UserWrittenPrint_18250ab(UInt32 level, const char* format, ...);

//*--------------------------------------------------------------------------------------
//* Template of function to be provided by customer
//*--------------------------------------------------------------------------------------

#define BURST_SZ 14

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenI2CRead
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 AddrSize,
//* 						UInt8* pAddr,
//* 						UInt32 ReadLen,
//* 						UInt8* pData
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
static tmErrorCode_t UserWrittenRead_18250ab(tmUnitSelect_t tUnit,	UInt32 AddrSize, UInt8* pAddr,
UInt32 ReadLen, UInt8* pData)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/

	INT32 result = 0;
	UINT8 data[BURST_SZ]; // every time, write 14 byte..

	INT32 RemainLen, BurstNum;
	INT32 i,j;

	if (AddrSize != 1)
	{
		TDA18250AB_PRINTF("TDA18250AB error 2 !\n");
	}

	RemainLen = ReadLen % BURST_SZ; 
	if (RemainLen)
	{
		BurstNum = ReadLen / BURST_SZ; 
	}
	else
	{
		BurstNum = ReadLen / BURST_SZ - 1;
		RemainLen = BURST_SZ;
	}

	for (i = 0 ; i < BurstNum; i++)
	{
		data[0] = pAddr[0]+i*BURST_SZ;

		//result |= i2c_write_read(tda18250ab_tuner_i2c, u8_add, data, 1, BURST_SZ);
		result |= nim_i2c_write_read(tda18250ab_tuner_i2c, tda18250ab_i2c_addr, data, 1, BURST_SZ);

		//copy to the destination array.
		for (j = 0 ; j < BURST_SZ ; j++)
		{
			pData[i * BURST_SZ + j ] = data[j];
		}
	}

	data[0] = pAddr[0]+BurstNum*BURST_SZ;
	//result |= i2c_write_read(tda18250ab_tuner_i2c, u8_add, data, 1, RemainLen);
	result |= nim_i2c_write_read(tda18250ab_tuner_i2c, tda18250ab_i2c_addr, data, 1, RemainLen);

	for (i = 0 ; i < RemainLen ; i++)
	{
		pData[BurstNum * BURST_SZ + i ] = data[i];
	}

	if (result == SUCCESS)
	{
		err = TM_OK;
	}
	else
	{
		err = !TM_OK;
	}

/* ...*/
/* End of Customer code here */

   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenI2CWrite
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 AddrSize,
//* 						UInt8* pAddr,
//* 						UInt32 WriteLen,
//* 						UInt8* pData
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
static tmErrorCode_t UserWrittenWrite_18250ab (tmUnitSelect_t tUnit, 	UInt32 AddrSize, UInt8* pAddr,
UInt32 WriteLen, UInt8* pData)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/
	INT32 result = 0;
	UINT8 data[BURST_SZ+1]; // every time, write 14 byte..

	INT32 RemainLen, BurstNum;
	INT32 i,j;

	if (AddrSize != 1)
	{
		TDA18250AB_PRINTF("TDA18250AB error 1 !\n");
	}
	
	RemainLen = WriteLen % BURST_SZ; 
	if (RemainLen)
	{
		BurstNum = WriteLen / BURST_SZ; 
	}
	else
	{
		BurstNum = WriteLen / BURST_SZ - 1;
		RemainLen = BURST_SZ;
	}

	for (i = 0 ; i < BurstNum; i++)
	{
		for (j = 0 ; j < BURST_SZ ; j++)
		{
			data[j+1]   = pData[i * BURST_SZ + j ];
		}
		data[0] = pAddr[0]+i*BURST_SZ;

		//result |= i2c_write(tda18250ab_tuner_i2c, u8_add, data, BURST_SZ+1);
		result |= nim_i2c_write(tda18250ab_tuner_i2c, tda18250ab_i2c_addr, data, BURST_SZ+1);
	}

	for (i = 0 ; i < RemainLen ; i++)
	{
		data[i+1]   = pData[BurstNum * BURST_SZ + i ];
	}
	data[0] = pAddr[0]+BurstNum*BURST_SZ;
	
	//result |= i2c_write(tda18250ab_tuner_i2c, u8_add, data, RemainLen+1);
	result |= nim_i2c_write(tda18250ab_tuner_i2c, tda18250ab_i2c_addr, data, RemainLen+1);

	if (result == SUCCESS)
	{
		err = TM_OK;
	}
	else
	{
		err = !TM_OK;
	}

/* ...*/
/* End of Customer code here */

   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenWait_18250ab
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 tms
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
static tmErrorCode_t UserWrittenWait_18250ab(tmUnitSelect_t tUnit, UInt32 tms)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/
	//osal_task_sleep(tms);
	comm_sleep(tms);
/* ...*/
/* End of Customer code here */

   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenPrint_18250ab
//* Object              : 
//* Input Parameters    : 	UInt32 level, const char* format, ...
//* 						
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
static tmErrorCode_t UserWrittenPrint_18250ab(UInt32 level, const char* format, ...)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/

/* ...*/
/* End of Customer code here */

   return err;
}

INT32 tun_tda18250ab_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (tda18250ab_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
	{
		return ERR_FAILUE;
	}

	tda18250ab_tuner_i2c = ptrTuner_Config->i2c_type_id;
	tda18250ab_i2c_addr = ptrTuner_Config->c_tuner_base_addr;
	tda18250ab_dev_id[tda18250ab_tuner_cnt] = ptrTuner_Config;
	*tuner_id = tda18250ab_tuner_cnt;

	tda18250ab_tuner_cnt ++;

	TDA18250AB_PRINTF("%s start !\n", __FUNCTION__);
	TDA18250AB_PRINTF("tda18250ab_tuner_i2c /tda18250ab_i2c_addr is %d / 0x%x\n",tda18250ab_tuner_i2c,tda18250ab_i2c_addr);
	return SUCCESS;
}

INT32 tun_tda18250ab_release(void)
	 {
		     tda18250ab_tuner_cnt = 0;
		     return SUCCESS;
		 }

INT32 tun_tda18250ab_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result = 0;
	tmErrorCode_t err;
	tmbslFrontEndState_t PLLLockStatus = tmbslFrontEndStateUnknown;	
	
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=tda18250ab_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = tda18250ab_dev_id[tuner_id];

   /* Get TDA18250AB Master PLL Lock status */
	err = tmbslTDA18250A_GetPLLState(tuner_id, &PLLLockStatus);
	if (err != TM_OK)
	{
		*lock = 0;
		result = !SUCCESS;
	}
	else
	{
		if (tmbslFrontEndStateLocked == PLLLockStatus)
		{
			*lock = 1;
		}
		else
		{
			*lock = 0;
		}
		result = SUCCESS;
	}
   
	return result;
}

INT32 tun_tda18250ab_control(UINT32 tuner_id, UINT32 freq, UINT32 sym)	
{	

	INT32   result;
	tmErrorCode_t err;
	static INT8 tun_status=0;
    struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	UInt32 uRF_Hz = freq*1000;

	tuner_dev_ptr = tda18250ab_dev_id[tuner_id];
TDA18250AStandardMode_t stdmodemaster = TDA18250A_QAM_8MHz;

	if(tuner_dev_ptr->c_tuner_freq_param !=0x00)
	{
		if(t_val!=tuner_dev_ptr->c_tuner_freq_param)
		{
			t_val=tuner_dev_ptr->c_tuner_freq_param;
			tun_status=0x00;
		}
	}

	if (0==tun_status)
	{
	   /* Variable declarations */
		tmbslFrontEndDependency_t  sSrvFunc;
		tmErrorCode_t err = TM_OK;


	   switch(tuner_dev_ptr->c_tuner_freq_param)
	   	{
	   		case 0x09:
				stdmodemaster=TDA18250A_QAM_8MHz;
				break;
			case 0x0C:
				stdmodemaster=TDA18250A_QAM_6MHz;
				break;
			default:
				stdmodemaster = TDA18250A_QAM_8MHz;
				break;
	   	}
	  	 tuner_dev_ptr->c_tuner_freq_param =0x00;
       	memset(&sSrvFunc, 0, sizeof(sSrvFunc));
		sSrvFunc.sIo.Write = UserWrittenWrite_18250ab;
		sSrvFunc.sIo.Read = UserWrittenRead_18250ab;
		sSrvFunc.sTime.Wait = UserWrittenWait_18250ab;
		sSrvFunc.sDebug.Print = UserWrittenPrint_18250ab;
		sSrvFunc.sMutex.Init = Null;
		sSrvFunc.sMutex.DeInit = Null;
		sSrvFunc.sMutex.Acquire = Null;
		sSrvFunc.sMutex.Release = Null;

		/* Tuner init */
		sSrvFunc.dwAdditionalDataSize = 0;
		sSrvFunc.pAdditionalData = Null;
		err = tmbslTDA18250A_Open(tuner_id, &sSrvFunc);

		//joey, 20130415, for 27M crystal support, in future, should be from the top setting.
/*		
		if (err == TM_OK)
		{
			TDA18250AXtalFreq_t tmp_crystal = TDA18250A_XtalFreq_Unknown;
			
			struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

			if ((tuner_id>=tda18250ab_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
				return ERR_FAILUE;
			tuner_dev_ptr = tda18250ab_dev_id[tuner_id];
			
			if (16 == tuner_dev_ptr->cTuner_Crystal) // tuner crystal selection.
			{
				tmp_crystal = TDA18250A_XtalFreq_16000000;
			}
			else if (24 == tuner_dev_ptr->cTuner_Crystal)
			{
				tmp_crystal = TDA18250A_XtalFreq_24000000;
			}
			else if (25 == tuner_dev_ptr->cTuner_Crystal)
			{
				tmp_crystal = TDA18250A_XtalFreq_25000000;
			}
			else if (27 == tuner_dev_ptr->cTuner_Crystal)
			{
				tmp_crystal = TDA18250A_XtalFreq_27000000;
			}
			else if (30 == tuner_dev_ptr->cTuner_Crystal)
			{
				tmp_crystal = TDA18250A_XtalFreq_30000000;
			}
			else
			{
				tmp_crystal = TDA18250A_XtalFreq_16000000;
			}
		
			err = tmbslTDA18250A_SetXtal(tuner_id, tmp_crystal);
		}
*/
		if (err == TM_OK)
		{
			err = tmbslTDA18250A_HwInit(tuner_id);
		}
		if (err == TM_OK)
		{
			err = tmbslTDA18250A_SetPowerState(tuner_id, tmPowerOn);
		}
		
		if(err != TM_OK)
			return err;

		tun_status = 1;
	
	
	err = tmbslTDA18250A_SetStandardMode(tuner_id, stdmodemaster);
	TDA18250AB_PRINTF("2---the err is %d\n",err);

	}
	
	
	if (err == TM_OK)
	{
		err = tmbslTDA18250A_SetRF(tuner_id, uRF_Hz);
	}


	if (err !=TM_OK) 
	{
		result= ERR_FAILED;
	}
	else
	{
		result = SUCCESS;
	}

	return result;
}

INT32 tun_tda18250ab_get_rf_level(UINT32 tuner_id, UINT32 *rf_level)// return level in dbuV.
{
	tmErrorCode_t   err = TM_OK;
	INT32 tmp_val = 0;

	err = tmbslTDA18250A_GetPowerLevel(0, &tmp_val);

	if (TM_OK != err)
	{
		*rf_level = 0;
		return ERR_FAILURE;
	}

	if (tmp_val < -50)
	{
		*rf_level = 0;
	}
	else
	{
		*rf_level = (UINT32)((tmp_val + 50)/100);
	}
	
	return SUCCESS;
}

#if 0
static int __devinit ali_tuner_tda18250_init(void)
{
	tda18250ab_model = kmalloc(sizeof(struct tuner_handle), GFP_KERNEL);
	if (NULL == tda18250ab_model)
	{
		TDA18250AB_PRINTF("Alloc tuner memory failed.\n");
		return -ENOMEM;
	}

	printk("[NOTE] mount %s module...\n", STR_TDA18250AB);
	strcpy(tda18250ab_model->tun_name, STR_TDA18250AB);
	tda18250ab_model->seq_num       = 0;
	tda18250ab_model->tuner_init    = tun_tda18250ab_init;
	tda18250ab_model->tuner_control = tun_tda18250ab_control;
	tda18250ab_model->tuner_status  = tun_tda18250ab_status;
	tda18250ab_model->tuner_rflevel = NULL;  //tun_tda18250ab_get_rf_level;
	tda18250ab_model->tuner_release = tun_tda18250ab_release;
	tda18250ab_model->tuner_active  = NULL;
    tda18250ab_model->tuner_standby = NULL;

	if (ali_nim_tuner_register(tda18250ab_model))
	{
		return -ENODEV;
	}

	return RET_SUCCESS;
}

static void __exit ali_tuner_tda18250_exit(void)
{
	printk("[NOTE] unmount %s module...\n", STR_TDA18250AB);
	ali_nim_tuner_unregister(STR_TDA18250AB);
	kfree(tda18250ab_model);
	tda18250ab_model = NULL;
}

module_init(ali_tuner_tda18250_init);
module_exit(ali_tuner_tda18250_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Corey");
MODULE_DESCRIPTION("TDA18250 tuner driver for Ali M3200");

#endif

