/*****************************************************************************
 *    Copyright (C)2007 Ali Corporation. All Rights Reserved.
 *
 *    File:    tun_mxl203rf.c
 *
 *    Description:    Source file of MAXLINEAR mxl203rf TUNER.
 *    History:
 *           Date          Author                 Version          Reason
 *	    ============	=============	=========	=================
 *   4.17.2008	     David.Deng	      Ver 0.1	     Create file.
 *****************************************************************************/

#include "tun_mxl203rf.h"

#include <linux/version.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

//#define MXL_DEBUG

#define NIM_MXL203RF_PRINTF(...) do{}while(0)
//#define NIM_MXL203RF_PRINTF printk

static MxL203RF_TunerConfigS mxl203rf_TunerConfig[2];
static BOOL bMxl_Tuner_Inited[2]={FALSE,FALSE};

#ifdef MXL_DEBUG
UINT8 reg_dump[256];
#endif

static struct QAM_TUNER_CONFIG_EXT * mxl203rf_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 mxl203rf_tuner_cnt = 0;

//static struct mutex g_lock;
static UINT32 mxl203_tuner_i2c = 0;
static UINT8  mxl203_i2c_addr = 0;

static struct tuner_handle *mxl203_model = NULL;


INT32 tun_mxl203rf_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	//struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (mxl203rf_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
	{
		NIM_MXL203RF_PRINTF("%s,%d\n",__FUNCTION__,__LINE__);
		return ERR_FAILUE;
	}
	/*
	   tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	   if((void*)tuner_dev_ptr == NULL)
	   return ERR_FAILUE;

	   MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	

	   mxl203rf_dev_id[mxl203rf_tuner_cnt] = tuner_dev_ptr;
	 *tuner_id = mxl203rf_tuner_cnt;
	 mxl203rf_TunerConfig[mxl203rf_tuner_cnt].tuner_id = mxl203rf_tuner_cnt;
	 bMxl_Tuner_Inited[mxl203rf_tuner_cnt] = FALSE;
	 */
	*tuner_id = mxl203rf_tuner_cnt;
	mxl203rf_TunerConfig[mxl203rf_tuner_cnt].tuner_id = mxl203rf_tuner_cnt;
	bMxl_Tuner_Inited[mxl203rf_tuner_cnt] = FALSE;
	mxl203_tuner_i2c = ptrTuner_Config->i2c_type_id;
	mxl203_i2c_addr = ptrTuner_Config->c_tuner_base_addr;
	mxl203rf_dev_id[mxl203rf_tuner_cnt] = ptrTuner_Config;
	mxl203rf_tuner_cnt ++;

	NIM_MXL203RF_PRINTF("\nmxl203_tuner_i2c %d,mxl203_i2c_addr %d,mxl203rf_tuner_cnt %d\n",\
			mxl203_tuner_i2c, mxl203_i2c_addr,mxl203rf_tuner_cnt);
	return SUCCESS;
}

/*****************************************************************************
 * INT32 tun_mxl203rf_status(UINT8 *lock)
 *
 * Tuner read operation
 *
 * Arguments:
 *  Parameter1: UINT8 *lock		: Phase lock status
 *
 * Return Value: INT32			: Result
 *****************************************************************************/

INT32 tun_mxl203rf_status(UINT32 tuner_id, UINT8 *lock)
{
	BOOL IfLock;
	INT32 ret = SUCCESS;

	if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
		return ERR_FAILUE;

	if(MxL_OK == MxL_RFSynth_Lock_Status(&mxl203rf_TunerConfig[tuner_id], &IfLock))
	{
		//*lock = 1;
		*lock = ( IfLock == TRUE ) ? 1 : 0; 
	}
	else
	{
		*lock = 0;        
		ret = ERR_FAILUE;
		NIM_MXL203RF_PRINTF("%s err,tuner_id %d\n",__FUNCTION__,tuner_id);
	}
	return ret;
}

/*****************************************************************************
 * INT32 nim_mxl203rf_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
 *
 * Tuner write operation
 *
 * Arguments:
 *  Parameter1: UINT32 freq		: Synthesiser programmable divider
 *  Parameter2: UINT8 bandwidth		: channel bandwidth
 *  Parameter3: UINT8 AGC_Time_Const	: AGC time constant
 *  Parameter4: UINT8 *data		: 
 *
 * Return Value: INT32			: Result
 *****************************************************************************/
INT32 tun_mxl203rf_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)
{
	unsigned long dwRF_Freq_KHz;
	MxL203RF_BW_MHz mxl_BW;
	MxL_ERR_MSG Status;

	NIM_MXL203RF_PRINTF("%s:tuner_id %d,freq %ld, sym %ld,AGC_Time_Const %d,_i2c_cmd %d\n",__FUNCTION__,tuner_id,freq,sym,AGC_Time_Const,_i2c_cmd);

	if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
		return ERR_FAILUE;

	if ( bMxl_Tuner_Inited[tuner_id] == FALSE )
	{
		NIM_MXL203RF_PRINTF("MXL203RF CTL_INIT,ADDR=%x\n",mxl203rf_dev_id[tuner_id]->c_tuner_base_addr);
		mxl203rf_TunerConfig[tuner_id].I2C_Addr = (MxL203RF_I2CAddr)(mxl203rf_dev_id[tuner_id]->c_tuner_base_addr);//MxL_I2C_ADDR_96;
		mxl203rf_TunerConfig[tuner_id].Mode = MxL_MODE_CAB_STD;
		mxl203rf_TunerConfig[tuner_id].Xtal_Freq=MxL_XTAL_48_MHZ;		
		mxl203rf_TunerConfig[tuner_id].IF_Freq = MxL_IF_7_2_MHZ; //MxL_IF_6_MHZ;// MxL_IF_4_57_MHZ;//MxL_IF_36_15_MHZ;
		mxl203rf_TunerConfig[tuner_id].IF_Spectrum = MxL_NORMAL_IF;
		mxl203rf_TunerConfig[tuner_id].ClkOut_Setting = MxL_CLKOUT_DISABLE;
		mxl203rf_TunerConfig[tuner_id].ClkOut_Amp = MxL_CLKOUT_AMP_10;
		mxl203rf_TunerConfig[tuner_id].Xtal_Cap = MxL_XTAL_CAP_12_PF;

		if((Status = MxL_Tuner_Init(&mxl203rf_TunerConfig[tuner_id])) != MxL_OK )
		{
			return ERR_FAILUE;
		}

		bMxl_Tuner_Inited[tuner_id] = TRUE;
	}

	dwRF_Freq_KHz = freq;
	mxl_BW = MxL_BW_8MHz; //not use 'sym'

	if(  (Status = MxL_Tuner_RFTune(&mxl203rf_TunerConfig[tuner_id], dwRF_Freq_KHz*1000, mxl_BW)) != MxL_OK )
	{
		return ERR_FAILUE;
	}

	//#define MXL_DEBUG
#ifdef MXL_DEBUG
	UINT8 i;
	for(i = 0; i < 255; i++)
	{
		//printk(&mxl203rf_TunerConfig[tuner_id], i, &reg_dump[i]);
	}
#endif	
	return SUCCESS;	
}

/******************************************************************************
 **
 **  Name: MxL_I2C_Write
 **
 **  Description:    I2C write operations
 **
 **  Parameters:    	
 **					DeviceAddr	- mxl203rf Device address
 **					pArray		- Write data array pointer
 **					count		- total number of array
 **
 **  Returns:        0 if success
 **
 **  Revision History:
 **
 **   SCR      Date      Author  Description
 **  -------------------------------------------------------------------------
 **   N/A   12-16-2007   khuang initial release.
 **
 ******************************************************************************/
#if defined(BURST_PAIR_SZ)
#error " BURST_PAIR_SZ already defined!"
#else
//#define BURST_PAIR_SZ   6
#define BURST_SZ 12
#endif
UINT32 MxL_I2C_Write(UINT8 DeviceAddr, UINT8* pArray, UINT32 count)
{
	int status = 0;
	UINT32 i,j;

	//UINT32 tuner_id = 0;

	UINT8 szBuffer[BURST_SZ];
	UINT16 BurstNum, RemainLen;

	NIM_MXL203RF_PRINTF("Write count =%d\n",count);

	for (i=0;i<count; i++)
	{
		NIM_MXL203RF_PRINTF("Write MXL203RF[%x]=%x\n",i, pArray[i]);
	}
	//	mxl203rf_dev_id[tuner_id]->i2c_type_id=0;
	//	mxl203rf_dev_id[tuner_id]->c_tuner_base_addr=MxL_I2C_ADDR_96;

	if ( !count ) return 0;

	RemainLen = count % BURST_SZ; 
	if ( RemainLen )
	{
		BurstNum = count / BURST_SZ; 
	}
	else
	{
		BurstNum = (count / BURST_SZ) - 1;
		RemainLen = BURST_SZ;
	}

	for ( i = 0 ; i < BurstNum; i ++ )
	{
		for ( j = 0 ; j < BURST_SZ ; j++  )
		{
			szBuffer[j]   = pArray[i * BURST_SZ + j ];             // addr
			//szBuffer[j+1] = pArray[i * BURST_PAIR_SZ + j +1];	// value

		}

		//status+= i2c_write(mxl203rf_dev_id[tuner_id]->i2c_type_id, mxl203rf_dev_id[tuner_id]->c_tuner_base_addr, szBuffer, BURST_SZ);
		status += ali_i2c_scb_write(mxl203_tuner_i2c, mxl203_i2c_addr, szBuffer, BURST_SZ);
		if(status!=0)
		{	
			NIM_MXL203RF_PRINTF("MXL203RF WR1 err=%d\n",status);
			;
		}
	}

	for ( i = 0 ; i < RemainLen ; i++ )
	{
		szBuffer[i]   = pArray[BurstNum * BURST_SZ + i ];         // addr 
	}

	//status += i2c_write(mxl203rf_dev_id[tuner_id]->i2c_type_id, mxl203rf_dev_id[tuner_id]->c_tuner_base_addr, szBuffer, RemainLen);
	status += ali_i2c_scb_write(mxl203_tuner_i2c, mxl203_i2c_addr, szBuffer, RemainLen);
	if(status!=0)
	{		
		NIM_MXL203RF_PRINTF("MXL203RF WR2 err=%d\n",status);
	}

	return status;
}

/******************************************************************************
 **
 **  Name: MxL_I2C_Read
 **
 **  Description:    I2C read operations
 **
 **  Parameters:    	
 **					DeviceAddr	- mxl203rf Device address
 **					Addr		- register address for read
 **					*Data		- data return
 **
 **  Returns:        0 if success
 **
 **  Revision History:
 **
 **   SCR      Date      Author  Description
 **  -------------------------------------------------------------------------
 **   N/A   12-16-2007   khuang initial release.
 **
 ******************************************************************************/
UINT32 MxL_I2C_Read(UINT8 DeviceAddr, UINT8 Addr, UINT8* mData)
{
	int status = 0;

	UINT8 Read_Cmd[2]; 

	//UINT32 tuner_id = 0;

	/* read step 1. accroding to mxl203rf driver API user guide. */
	Read_Cmd[0] = 0xFB;
	Read_Cmd[1] = Addr;

	//status = i2c_write(mxl203rf_dev_id[tuner_id]->i2c_type_id, mxl203rf_dev_id[tuner_id]->c_tuner_base_addr, Read_Cmd, 2);
	status = ali_i2c_scb_write(mxl203_tuner_i2c, mxl203_i2c_addr, Read_Cmd, 2);

	//status += i2c_read(mxl203rf_dev_id[tuner_id]->i2c_type_id, mxl203rf_dev_id[tuner_id]->c_tuner_base_addr, mData, 1);
	status += ali_i2c_scb_read(mxl203_tuner_i2c, mxl203_i2c_addr, mData, 1);

	NIM_MXL203RF_PRINTF("Read MXL203RF[%x]=%x,status=%d\n",Addr, *mData, status);

	return status;
}

/******************************************************************************
 **
 **  Name: MxL_Delay
 **
 **  Description:    Delay function in milli-second
 **
 **  Parameters:    	
 **					mSec		- milli-second to delay
 **
 **  Returns:        0
 **
 **  Revision History:
 **
 **   SCR      Date      Author  Description
 **  -------------------------------------------------------------------------
 **   N/A   12-16-2007   khuang initial release.
 **
 ******************************************************************************/
void MxL_Delay(UINT32 mSec)
{
	//osal_task_sleep(mSec);
	msleep(mSec);
}

INT32 tun_mxl203rf_release(void)
{
	mxl203rf_tuner_cnt = 0;
	return SUCCESS;
}

static int __devinit ali_tuner_mxl203_init(void)
{
	mxl203_model = kmalloc(sizeof(struct tuner_handle), GFP_KERNEL);
	if (NULL == mxl203_model)
	{
		NIM_MXL203RF_PRINTF("Alloc mxl203 tuner memory failed.\n");
		return -ENOMEM;
	}

	printk("[NOTE] mount %s module...\n", STR_MXL203);
	strcpy(mxl203_model->tun_name, STR_MXL203);
	mxl203_model->seq_num       = 0;
	mxl203_model->tuner_init    = tun_mxl203rf_init;
	mxl203_model->tuner_control = tun_mxl203rf_control;
	mxl203_model->tuner_status  = tun_mxl203rf_status;
	mxl203_model->tuner_rflevel = NULL;
	mxl203_model->tuner_release = tun_mxl203rf_release;
	mxl203_model->tuner_active  = NULL;
    mxl203_model->tuner_standby = NULL;

	if (ali_nim_tuner_register(mxl203_model))
	{
		return -ENODEV;
	}

	return RET_SUCCESS;
}

static void __exit ali_tuner_mxl203_exit(void)
{
	printk("[NOTE] unmount %s module...\n", STR_MXL203);
	ali_nim_tuner_unregister(STR_MXL203);
	kfree(mxl203_model);
	mxl203_model = NULL;
}

module_init(ali_tuner_mxl203_init);
module_exit(ali_tuner_mxl203_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bobby");
MODULE_DESCRIPTION("MXL203 tuner driver for Ali M3200");

