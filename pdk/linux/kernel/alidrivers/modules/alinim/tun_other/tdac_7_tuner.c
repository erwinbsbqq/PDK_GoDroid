#include "tdac_7_tuner.h"
#include "atbm886x.h"

#define STR_DCT70701     "TDAC7"
#define MAX_TUNER_SUPPORT_NUM 6

static unsigned char Tuner_Addr;
static unsigned char gConfig_buf[6];
static struct QAM_TUNER_CONFIG_EXT * stDCT70701_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 stDCT70701_tuner_cnt = 0;
static struct tuner_handle *dct70701_model = NULL;

//Alps TDAC2-C02A Tuner, configure DTMB 
void Set_Tuner_TDAC2_DTMB(int channel_frequency_KHz)
{
    	int config_data = (channel_frequency_KHz*6)/1000 + 216; //36M IF, Check demod config
	Tuner_Addr=0xC0;   	
	gConfig_buf[0]=(config_data>>8)&0x7F;
	gConfig_buf[1]=config_data&0xFF;
	gConfig_buf[2]=0xa8;
	
	if(channel_frequency_KHz<125000)
		gConfig_buf[3]=0xA0;
	else if(channel_frequency_KHz<174000)
		gConfig_buf[3]=0xA2;
	else if(channel_frequency_KHz<366000)
		gConfig_buf[3]=0xE2;
	else if(channel_frequency_KHz<470000)
		gConfig_buf[3]=0xa8;
	else if(channel_frequency_KHz<766000)
		gConfig_buf[3]=0xe8;
	else		
		gConfig_buf[3]=0x28;
	
	gConfig_buf[4]=0xC6;

	TUNER_PRINTF("write tuner data DTMB 0x%x 0x%x 0x%x 0x%x 0x%x\n\n",gConfig_buf[0],gConfig_buf[1],gConfig_buf[2],gConfig_buf[3],gConfig_buf[4]);
	Tuner_I2C_Write(Tuner_Addr, gConfig_buf, 5);	
}

#if 0
//Alps TDAC2-C02A Tuner, configure DVB-C 
static void Set_Tuner_TDAC2_DVBC(int channel_frequency_KHz)
{
	
    // int config_data = (channel_frequency_KHz*16)/1000 + 578; //36.125M IF
    int config_data = (channel_frequency_KHz*16)/1000 + 576; //36M
    Tuner_Addr=0xC0; 	
	
	gConfig_buf[0]=(config_data>>8)&0x7F;
	gConfig_buf[1]=config_data&0xFF;
	gConfig_buf[2]=0xab;
	
    if(channel_frequency_KHz<125000)
		gConfig_buf[3]=0xA0;
	else if(channel_frequency_KHz<174000)
		gConfig_buf[3]=0xA2;
	else if(channel_frequency_KHz<366000)
		gConfig_buf[3]=0xE2;
	else if(channel_frequency_KHz<470000)
		gConfig_buf[3]=0xa8;
	else if(channel_frequency_KHz<766000)
		gConfig_buf[3]=0xe8;
	else		
		gConfig_buf[3]=0x28;
	
	gConfig_buf[4]=0xC6;	

	TUNER_PRINTF("write tuner data dvbc 36M IF 0x%x 0x%x 0x%x 0x%x 0x%x\n",gConfig_buf[0],gConfig_buf[1],gConfig_buf[2],gConfig_buf[3],gConfig_buf[4]);
	Tuner_I2C_Write(Tuner_Addr, gConfig_buf, 5);
}
#endif

void Tuner_I2C_Write(unsigned char  tuner_addr, unsigned char * data, unsigned char length)
{	
	int ret,i;
	//ret = ali_i2c_scb_write(1,tuner_addr,data,length);
	ret = demo_I2c_write(tuner_addr, data,length);
	if(ret == SUCCESS)
	{
		for(i=0;i<length;i++)
			TUNER_PRINTF("[%s][success] tuner i2c write data[%d]=%d\n",__FUNCTION__,i,*(data+i));
	}
	else
		NIM_LOG_ERROR("[%s][ERROR] \n",__FUNCTION__);

}

void Tuner_I2C_Read(unsigned char  tuner_addr, unsigned char * data, unsigned char length)
{
	int ret,i;
	ret = demo_I2c_read(tuner_addr,data,length);
	if(ret == SUCCESS)
	{
		for(i=0;i<length;i++)
			TUNER_PRINTF("[%s][success] tuner i2c read data[%d] = %d \n",__FUNCTION__,i,*(data+i));
	}
	else
		NIM_LOG_ERROR("[%s][ERROR] \n",__FUNCTION__);
}

INT32 tun_dct70701_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	//struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (stDCT70701_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	/*************************************************************************************
	tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));
	**************************************************************************************/
	stDCT70701_dev_id[stDCT70701_tuner_cnt] = ptrTuner_Config;
	*tuner_id = stDCT70701_tuner_cnt;
	stDCT70701_tuner_cnt ++;
	return SUCCESS;
}

INT32 tun_dct70701_release(void)
{
	TUNER_PRINTF("[NOTE] TUNER RELEASE ..... \n");
	stDCT70701_tuner_cnt = 0;
	return SUCCESS;
}

INT32 tun_dct70701_status(UINT32 tuner_id, UINT8 *lock)
{
	UINT8 data =0;
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	TUNER_PRINTF("[NOTE] get tun status .....   \n");

	if ((tuner_id>=stDCT70701_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = stDCT70701_dev_id[tuner_id];
	tuner_dev_ptr->c_tuner_base_addr |= 0x01;
	Tuner_I2C_Read(tuner_dev_ptr->c_tuner_base_addr, &data, 1);
	*lock = (UINT8)((data>>6)&0x01);	
	TUNER_PRINTF("[NOTE] get lock value:%d \n",*lock);
	return RET_SUCCESS;
}

INT32 tun_dct70701_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)	
{	
	TUNER_PRINTF("[NOTE] tun_control ...............\n");
	if ((tuner_id>=stDCT70701_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
	{
		 DCT70701_PRINTF("[%s]line=%d,error,back!\n", __FUNCTION__, __LINE__);
		return ERR_FAILUE;
	}
	
	Set_Tuner_TDAC2_DTMB(freq);
	return RET_SUCCESS;
}


static int __devinit ali_tuner_dct70701_init(void)
{
	dct70701_model = kmalloc(sizeof(struct tuner_handle), GFP_KERNEL);
	if (NULL == dct70701_model)
	{
		NIM_LOG_ERROR("Alloc tuner memory failed.\n");
		return -ENOMEM;
	}

	TUNER_PRINTF("[NOTE] mount %s module...\n", STR_DCT70701);
	strcpy(dct70701_model->tun_name, STR_DCT70701);
	dct70701_model->seq_num       = 0;
	dct70701_model->tuner_init    = tun_dct70701_init;
	dct70701_model->tuner_control = tun_dct70701_control;
	dct70701_model->tuner_status  = NULL;		//tun_dct70701_status;
	dct70701_model->tuner_rflevel = NULL;
	dct70701_model->tuner_release = tun_dct70701_release;
	
/*	if (ali_nim_tuner_register(dct70701_model))
	{
		return -ENODEV;
	}
*/	
	return RET_SUCCESS;
}

static void __exit ali_tuner_dct70701_exit(void)
{
	TUNER_PRINTF("[NOTE] unmount %s module...\n", STR_DCT70701);
	//ali_nim_tuner_unregister(STR_DCT70701);
	kfree(dct70701_model);
	dct70701_model = NULL;
}

module_init(ali_tuner_dct70701_init);
module_exit(ali_tuner_dct70701_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Li");
MODULE_DESCRIPTION("DCT70701 tuner driver for Ali M3200");

