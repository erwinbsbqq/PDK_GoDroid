/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_alpstdae.c
*
*    Description:    This file contains alpstdae basic function in LLD. 
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20080520		Trueve Hu		 Ver 0.1	Create file.
*	
*****************************************************************************/

#include "tun_alpstdae.h"

#include <linux/version.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define ALPSTDAE_PRINTF(...)	do{}while(0) //libc_printf
//#define ALPSTDAE_PRINTF	PRINTK_INFO

//static UINT32 tun_alpstdae_mutex_id;

static struct QAM_TUNER_CONFIG_EXT * alpstdae_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 alpstdae_tuner_cnt = 0;

static struct tuner_handle *alpstdae_model = NULL;


INT32 tun_alpstdae_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	//struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (alpstdae_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	/*tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));*/
	
	//alpstdae_dev_id[alpstdae_tuner_cnt] = tuner_dev_ptr;
	alpstdae_dev_id[alpstdae_tuner_cnt] = ptrTuner_Config;
	*tuner_id = alpstdae_tuner_cnt;

	alpstdae_tuner_cnt ++;

	return SUCCESS;
}

INT32 tun_alpstdae_release(void)
{
	alpstdae_tuner_cnt = 0;
    return SUCCESS;
}

INT32 tun_alpstdae_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result;
	UINT8 data =0;	

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=alpstdae_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = alpstdae_dev_id[tuner_id];

	//result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1);
	result = ali_i2c_scb_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr | 0x01, &data, 1);
	*lock = (UINT8)((data>>6)&0x01);

	return result;
}

INT32 tun_alpstdae_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)	
{	
	INT32  result;
	UINT16 Npro;
	UINT32 OscFreq;
	UINT8  StepFreq,RefDivRatio,TunCrystal;
	UINT8  RS, AGST, BS_CP; 
	UINT8  data[5];

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=alpstdae_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = alpstdae_dev_id[tuner_id];

	ALPSTDAE_PRINTF("%s: feq, sym:  %d, %d !\n", __FUNCTION__, freq, sym);

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
	else if (RefDivRatio == 80) //50
		RS = 0x02;
	else if (RefDivRatio == 64) // 62.5
		RS = 0x03;
	else if (RefDivRatio == 128) // 31.25
		RS = 0x04;
	else
		RS = 0x03;

	//AGST = 0x03; //Normal op
	//AGST = 0x00; //Dual AGC
	AGST = tuner_dev_ptr->cTuner_AGC_TOP;

	if ((freq >= 47000) && (freq < 125000)) 
		BS_CP = 0xa0;
	else if ((freq >= 125000) && (freq < 366000)) 
	 	BS_CP = 0xa2;
	else if  ((freq >= 366000) && (freq < 622000)) 
	 	BS_CP = 0x68;
	else if  ((freq >= 622000) && (freq < 726000)) 
	 	BS_CP = 0xa8;
	else if  ((freq >= 726000) && (freq < 862000)) 
	 	BS_CP = 0xe8; 
	else
		BS_CP = 0x68;

    //==========Evaluated to control bytes=============
	data[0] = (UINT8)((Npro>>8)&0x7F);
	data[1] = (UINT8)(Npro&0xFF); 
	data[2] = 0x80 | ((AGST&0x07)<<3) | (RS&0x07);
	data[3] = BS_CP;
	data[4] = 0xc6;

    //==========Write to tuner register===============
    if (AGC_Time_Const == SLOW_TIMECST_AGC)
        msleep(50);

    result = ERR_FAILUE;

	if (_i2c_cmd == _1st_i2c_cmd)
	{
		//result = i2c_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, data,5);
		result = ali_i2c_scb_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, data, 5);
	}

    return result;
}


static int __devinit ali_tuner_alpstdae_init(void)
{
	alpstdae_model = kmalloc(sizeof(struct tuner_handle), GFP_KERNEL);
	if (NULL == alpstdae_model)
	{
		ALPSTDAE_PRINTF("Alloc tuner memory failed.\n");
		return -ENOMEM;
	}

	printk("[NOTE] mount %s module...\n", STR_ALPSTDAE);
	strcpy(alpstdae_model->tun_name, STR_ALPSTDAE);
	alpstdae_model->seq_num       = 0;
	alpstdae_model->tuner_init    = tun_alpstdae_init;
	alpstdae_model->tuner_control = tun_alpstdae_control;
	alpstdae_model->tuner_status  = tun_alpstdae_status;
	alpstdae_model->tuner_rflevel = NULL;
	alpstdae_model->tuner_release = tun_alpstdae_release;
	alpstdae_model->tuner_active  = NULL;
    alpstdae_model->tuner_standby = NULL;
	
	if (ali_nim_tuner_register(alpstdae_model))
	{
		return -ENODEV;
	}
	
	return RET_SUCCESS;
}

static void __exit ali_tuner_alpstdae_exit(void)
{
	printk("[NOTE] unmount %s module...\n", STR_ALPSTDAE);
	ali_nim_tuner_unregister(STR_ALPSTDAE);
	kfree(alpstdae_model);
	alpstdae_model = NULL;
}

module_init(ali_tuner_alpstdae_init);
module_exit(ali_tuner_alpstdae_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Li");
MODULE_DESCRIPTION("ALPSTDAE tuner driver for Ali M3200");

