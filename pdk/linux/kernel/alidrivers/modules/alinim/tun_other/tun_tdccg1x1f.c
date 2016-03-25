/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_tdccg1x1f.c
*
*    Description:    This file contains tdcc_g1x1f basic function in LLD. 
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20081028		Trueve Hu		 Ver 0.1	Create file.
*	
*****************************************************************************/


#include "tun_tdccg1x1f.h"

#include <linux/version.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define TDCCG_PRINTF(...)	do{}while(0) //libc_printf

static struct QAM_TUNER_CONFIG_EXT * tdccg1x1f_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 tdccg1x1f_tuner_cnt = 0;

static struct tuner_handle *tdccg1x1f_model = NULL;


INT32 tun_tdccg1x1f_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	/* Check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (tdccg1x1f_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	
	tdccg1x1f_dev_id[tdccg1x1f_tuner_cnt] = ptrTuner_Config;
	*tuner_id = tdccg1x1f_tuner_cnt;

	tdccg1x1f_tuner_cnt++;

	return SUCCESS;
}

INT32 tun_tdccg1x1f_release(void)
{
	tdccg1x1f_tuner_cnt = 0;
	return SUCCESS;
}

INT32 tun_tdccg1x1f_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result;
	UINT8 data =0;	
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=tdccg1x1f_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = tdccg1x1f_dev_id[tuner_id];

	//result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1);
	result = ali_i2c_scb_read(tuner_dev_ptr->i2c_type_id, (tuner_dev_ptr->c_tuner_base_addr) | 0x01, &data, 1);
	*lock = (UINT8)((data>>6)&0x01);

	return result;
}

INT32 tun_tdccg1x1f_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)	
{	
	INT32  result;
	UINT16 Npro;
	UINT32 OscFreq;
	UINT8  StepFreq,RefDivRatio,TunCrystal;
	UINT8  RS, ATP, BS; 
	UINT8  data[5];
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=tdccg1x1f_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = tdccg1x1f_dev_id[tuner_id];

	TDCCG_PRINTF("%s: feq, sym:  %d, %d !\n", __FUNCTION__, freq, sym);

    //==========Calculate control bytes=============
	OscFreq = freq + tuner_dev_ptr->w_tuner_if_freq;
	StepFreq = tuner_dev_ptr->cTuner_Step_Freq;
	RefDivRatio = tuner_dev_ptr->cTuner_Ref_DivRatio;
	TunCrystal = tuner_dev_ptr->c_tuner_crystal;

	Npro=OscFreq * RefDivRatio / (TunCrystal * 1000);	

	if (RefDivRatio == 24) //166.667
		RS = 0x00;
	else if (RefDivRatio == 28)//142.857
		RS = 0x01;
	else if (RefDivRatio == 32) //125
		RS = 0x02;
	else if (RefDivRatio == 64) // 62.5
		RS = 0x03;
	else if (RefDivRatio == 128) // 31.25
		RS = 0x04;
	else if (RefDivRatio == 80) //50
		RS = 0x05;
	else
		RS = 0x03;

	//ATP = 0x02; //Normal op
	//ATP = 0x07; //Dual AGC
	ATP = tuner_dev_ptr->cTuner_AGC_TOP;

	if (freq < 146000)
	{
		BS = 0x01;
	}
	else if ((freq >= 146000) && (freq < 430000)) 
	{
		BS = 0x02;
	}
	else
	{
		BS = 0x08;
	}

    //==========Evaluated to control bytes=============
	data[0] = (UINT8)((Npro>>8)&0x7F);
	data[1] = (UINT8)(Npro&0xFF); 
	data[2] = 0x80 | ((ATP<<3)&0x38) | (RS&0x07);
	data[3] = BS;
	data[4] = 0xd1; // mode = 1.

    result = ERR_FAILUE;

	if (_i2c_cmd == _1st_i2c_cmd)
	{
		//result = i2c_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, data,5);
		result = ali_i2c_scb_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, data, 5);
	}

  	//osal_task_sleep(50);
	msleep(50);

	return result;
}


static int __devinit ali_tuner_tdccg1x1f_init(void)
{
	tdccg1x1f_model = kmalloc(sizeof(struct tuner_handle), GFP_KERNEL);
	if (NULL == tdccg1x1f_model)
	{
		TDCCG_PRINTF("Alloc tuner memory failed.\n");
		return -ENOMEM;
	}

	printk("[NOTE] mount %s module...\n", STR_TDCCG1X1F);
	strcpy(tdccg1x1f_model->tun_name, STR_TDCCG1X1F);
	tdccg1x1f_model->seq_num       = 0;
	tdccg1x1f_model->tuner_init    = tun_tdccg1x1f_init;
	tdccg1x1f_model->tuner_control = tun_tdccg1x1f_control;
	tdccg1x1f_model->tuner_status  = tun_tdccg1x1f_status;
	tdccg1x1f_model->tuner_rflevel = NULL;
	tdccg1x1f_model->tuner_release = tun_tdccg1x1f_release;
	tdccg1x1f_model->tuner_active  = NULL;
    tdccg1x1f_model->tuner_standby = NULL;
	
	if (ali_nim_tuner_register(tdccg1x1f_model))
	{
		return -ENODEV;
	}

	return RET_SUCCESS;
}

static void __exit ali_tuner_tdccg1x1f_exit(void)
{
	printk("[NOTE] unmount %s module...\n", STR_TDCCG1X1F);
	ali_nim_tuner_unregister(STR_TDCCG1X1F);
	kfree(tdccg1x1f_model);
	tdccg1x1f_model = NULL;
}

module_init(ali_tuner_tdccg1x1f_init);
module_exit(ali_tuner_tdccg1x1f_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Corey");
MODULE_DESCRIPTION("MXL603 tuner driver for Ali M3200");

