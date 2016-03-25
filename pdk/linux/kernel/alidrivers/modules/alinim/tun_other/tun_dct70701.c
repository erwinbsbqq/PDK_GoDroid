/*****************************************************************************
 *    Copyright (C)2008 Ali Corporation. All Rights Reserved.
 *
 *    File:    tun_dct70701.c
 *
 *    Description:    This file contains dct70701 basic function in LLD. For Tuner DCT-70701.
 *    History:
 *           Date            Athor        Version          Reason
 *	    ============	=============	=========	=================
 *	1.  20080111	  Penghui		 Ver 0.1	Create file.
 *	
 *****************************************************************************/

#include "../porting_linux_header.h"

#include "tun_dct70701.h"




#if 0
#define DCT70701_PRINTF(x...) printk(KERN_INFO x)
#else
#define DCT70701_PRINTF(...) 
#endif

#define MAX_TUNER_SUPPORT_NUM 2//1

//static UINT32 tun_dct70701_mutex_id;

static struct QAM_TUNER_CONFIG_EXT *stDCT70701_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 stDCT70701_tuner_cnt = 0;

//static struct tuner_handle *dct70701_model = NULL;


INT32 tun_dct70701_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config)
{
	if ((ptrTuner_Config == NULL) || (stDCT70701_tuner_cnt >= MAX_TUNER_SUPPORT_NUM))
	{
		return ERR_FAILUE;
	}

	stDCT70701_dev_id[stDCT70701_tuner_cnt] = ptrTuner_Config;
	*tuner_id = stDCT70701_tuner_cnt;

	stDCT70701_tuner_cnt++;

	return SUCCESS;
}

INT32 tun_dct70701_release(void)
{
	stDCT70701_tuner_cnt = 0;
	return SUCCESS;
}

INT32 tun_dct70701_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result;
	UINT8 data = 0;	

	struct QAM_TUNER_CONFIG_EXT *tuner_dev_ptr = NULL;

	if ((tuner_id >= stDCT70701_tuner_cnt) || (tuner_id >= MAX_TUNER_SUPPORT_NUM))
	{
		return ERR_FAILUE;
	}
	tuner_dev_ptr = stDCT70701_dev_id[tuner_id];

	result = ali_i2c_read(tuner_dev_ptr->i2c_type_id, (tuner_dev_ptr->c_tuner_base_addr) | 0x01, &data, 1);
	*lock = (UINT8)((data>>6)&0x01);	

	return result;
}

INT32 tun_dct70701_control(UINT32 tuner_id, UINT32 freq, UINT32 sym)	
{	
	INT32  result;
	UINT16 Npro;
	UINT32 OscFreq;
	UINT8  StepFreq,RefDivRatio,TunCrystal;
	UINT8  data[5];

	UINT8 ATP2_0, RS2_0, BS4_1;

	struct QAM_TUNER_CONFIG_EXT *tuner_dev_ptr = NULL;

	if ((tuner_id >= stDCT70701_tuner_cnt) || (tuner_id >= MAX_TUNER_SUPPORT_NUM))
	{
		DCT70701_PRINTF("[%s]line=%d,error,back!\n", __FUNCTION__, __LINE__);
		return ERR_FAILUE;
	}
	tuner_dev_ptr = stDCT70701_dev_id[tuner_id];

	DCT70701_PRINTF("Tuner feq, sym: %d, %d.\n", freq, sym);
	OscFreq = freq + tuner_dev_ptr->w_tuner_if_freq;
	StepFreq = tuner_dev_ptr->c_tuner_step_freq;
	RefDivRatio = tuner_dev_ptr->c_tuner_ref_divratio;
	TunCrystal = tuner_dev_ptr->c_tuner_crystal ;

	//==========DATA[1:0] for Npro==============
	// StepFreq also equal to (TunCrystal * 1000 / RefDivRatio)
	Npro = (OscFreq+StepFreq/2)*RefDivRatio/(TunCrystal*1000);	

	//==========DATA[2]  for ATP2_0, RS2_0=========
	if (RefDivRatio == 24) //166.667
		RS2_0 = 0x00;
	else if (RefDivRatio == 28)//142.857
		RS2_0 = 0x01;
	else if (RefDivRatio == 50) //80
		RS2_0 = 0x02;
	else if (RefDivRatio == 64) // 62.5
		RS2_0 = 0x03;
	else if (RefDivRatio == 128) // 31.25
		RS2_0 = 0x04;
	else if (RefDivRatio == 80) // 50
		RS2_0 = 0x05;
	else
		RS2_0 = 0x03;

	//ATP2_0 = 0x01;
	//ATP2_0 = 0x07; // two AGC
	//joey 20080504. for ATP value from app layer.
	ATP2_0 = tuner_dev_ptr->c_tuner_agc_top;

	//==========DATA[3]  for BS4_1=================
	if (OscFreq >= 85000 && OscFreq < 185000)
		BS4_1 = 0x01;
	else if (OscFreq >= 185000 && OscFreq < 465000)
		BS4_1 = 0x06;
	else if (OscFreq >= 465000 && OscFreq <= 896000)
		BS4_1 = 0x0C;
	else 
		BS4_1 = 0x0C;

	//==========Evaluated to control byte=============
	data[0] = (UINT8)((Npro>>8)&0x7F);
	data[1] = (UINT8)(Npro&0xFF); 
	data[2] = (UINT8)(0x80|(ATP2_0<<3)|RS2_0);
	data[3] = (UINT8)(0x00|BS4_1);
	data[4] = (UINT8)(0xC3);

	//==========Write to tuner register===============
	//if(AGC_Time_Const == SLOW_TIMECST_AGC)
	//    osal_task_sleep(20);

	result = ERR_FAILUE;

	//if (_i2c_cmd == _1st_i2c_cmd)
	{
		result = ali_i2c_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, data, 5);
	}
    DCT70701_PRINTF("[%s]line=%d,end,result=%d!\n", __FUNCTION__, __LINE__,result);
	return result;
}

static int __devinit ali_tuner_dct70701_init(void)
{
/*	
	dct70701_model = kmalloc(sizeof(struct tuner_handle), GFP_KERNEL);
	if (NULL == dct70701_model)
	{
		printk("Alloc tuner memory failed.\n");
		return -ENOMEM;
	}

	printk("[NOTE] mount %s module...\n", STR_DCT70701);
	strcpy(dct70701_model->tun_name, STR_DCT70701);
	dct70701_model->seq_num       = 0;
	dct70701_model->tuner_init    = tun_dct70701_init;
	dct70701_model->tuner_control = tun_dct70701_control;
	dct70701_model->tuner_status  = tun_dct70701_status;
	dct70701_model->tuner_rflevel = NULL;
	dct70701_model->tuner_release = tun_dct70701_release;
	dct70701_model->tuner_active  = NULL;
	dct70701_model->tuner_standby = NULL;

	if (ali_nim_tuner_register(dct70701_model))
	{
		return -ENODEV;
	}
*/
	return RET_SUCCESS;
}

static void __exit ali_tuner_dct70701_exit(void)
{
/*	
	printk("[NOTE] unmount %s module...\n", STR_DCT70701);
	ali_nim_tuner_unregister(STR_DCT70701);
	kfree(dct70701_model);
	dct70701_model = NULL;
*/	
}

module_init(ali_tuner_dct70701_init);
module_exit(ali_tuner_dct70701_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Li");
MODULE_DESCRIPTION("DCT70701 tuner driver for Ali M3200");

