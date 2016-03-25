#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/dvb/frontend.h>
#include <asm/irq.h>
#include <linux/ali_transport.h>
#include <linux/i2c.h>
#include <linux/wait.h>

#include <asm/mach-ali/typedef.h>


#include "sharp_vz7306.h"
#include <linux/dvb/ali_i2c_scb_gpio.h>

#ifndef I2C_FOR_VZ7306
#define I2C_FOR_VZ7306 	0
#endif
#if 0
#define VZ7306_PRINTF(x...) printk(KERN_INFO x)
#else
#define VZ7306_PRINTF(...) 
#endif

#define I2cWriteRegs(Addr, rAddr, lpbData, bLen) i2c_write(I2C_FOR_VZ7306, Addr, lpbData, bLen)
#define I2cReadReg(Addr, rAddr, lpbData) i2c_read(I2C_FOR_VZ7306, Addr, lpbData, 1)

//#define TWO_TUNER_SUPPORT
#define SHARP_7306_NAME "SHARP_7306"
#define MAX_TUNER_SUPPORT_NUM 2//1




static unsigned char init_data1[] = { 0x44, 0x7e, 0xe1, 0x42 };
static unsigned char init_data2[] = { 0xe5 };
static unsigned char init_data3[] = { 0xfd, 0x0d };

struct QPSK_TUNER_CONFIG_EXT * vz7306_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL, NULL};
static UINT32 vz7306_tuner_cnt = 0;



INT32 ali_nim_vz7306_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	INT32 result;
	struct QPSK_TUNER_CONFIG_EXT * vz7306_ptr = NULL;
	VZ7306_PRINTF("[kangzh]line=%d,%s 1!\n",__LINE__, __FUNCTION__);
	if (ptrTuner_Config == NULL||vz7306_tuner_cnt>=MAX_TUNER_SUPPORT_NUM)
		return RET_FAILURE;
	/*vz7306_ptr = (struct QPSK_TUNER_CONFIG_EXT *)kmalloc(sizeof(struct QPSK_TUNER_CONFIG_EXT), GFP_KERNEL);
	if(!vz7306_ptr)
	      return RET_FAILURE;
	memcpy(vz7306_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
	*/
	vz7306_ptr=ptrTuner_Config;
	vz7306_dev_id[vz7306_tuner_cnt] = vz7306_ptr;
	*tuner_id = vz7306_tuner_cnt;
	vz7306_tuner_cnt++;
	VZ7306_PRINTF("[kangzh]line=%d,%s 2!\n",__LINE__, __FUNCTION__);
	VZ7306_PRINTF("%s: i2c addr:0x%x, id: %d\n",__FUNCTION__, vz7306_ptr->c_tuner_base_addr, vz7306_ptr->i2c_type_id);
	if ((result = ali_i2c_write(vz7306_ptr->i2c_type_id, vz7306_ptr->c_tuner_base_addr, init_data1, sizeof(init_data1))) != RET_SUCCESS)
		
	{
		VZ7306_PRINTF("nim_vz7306_init: I2C write error\n");
		return result;
	}
	VZ7306_PRINTF("[kangzh]line=%d,%s 3!\n",__LINE__, __FUNCTION__);
	if ((result = ali_i2c_write(vz7306_ptr->i2c_type_id, vz7306_ptr->c_tuner_base_addr, init_data2, sizeof(init_data2))) != RET_SUCCESS)
	{
		VZ7306_PRINTF("nim_vz7306_init %d: I2C write error\n", __LINE__);
		return result;
	}
	VZ7306_PRINTF("[kangzh]line=%d,%s 4!\n",__LINE__, __FUNCTION__);
	udelay(10000);
	
	if ((result = ali_i2c_write(vz7306_ptr->i2c_type_id, vz7306_ptr->c_tuner_base_addr, init_data3, sizeof(init_data3))) != RET_SUCCESS)
	{
		VZ7306_PRINTF("nim_vz7306_init %d: I2C write error\n", __LINE__);
		return result;
	}
	VZ7306_PRINTF("[kangzh]line=%d,%s 5!\n",__LINE__, __FUNCTION__);
	return RET_SUCCESS;
}

INT32 ali_nim_vz7306_close(void)
{
#ifndef TWO_TUNER_SUPPORT
	vz7306_tuner_cnt = 0;
#else
	if (0 >= vz7306_tuner_cnt)
		return;
	vz7306_tuner_cnt--;
#endif

   return RET_SUCCESS;
   
}
/*****************************************************************************
* INT32 nim_vz7306_control(UINT32 freq, UINT8 sym, UINT8 cp)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq		: Synthesiser programmable divider
*  Parameter2: UINT8 sym		: symbol rate
*
* Return Value: INT32			: Result
*****************************************************************************/
INT32 ali_nim_vz7306_control(UINT32 tuner_id,UINT32 freq, UINT32 sym)
{
	UINT8 data[4];
	UINT16 Npro,tmp;
	UINT32 Rs, BW;
	UINT8 Nswa;
	UINT8 LPF = 15;
	UINT8 BA = 1;
	UINT8 DIV = 0;
	INT32 result;

	struct QPSK_TUNER_CONFIG_EXT * vz7306_ptr = NULL;

	VZ7306_PRINTF("[kangzh]line=%d,%s enter!\n",__LINE__, __FUNCTION__);
	
	if(tuner_id>=vz7306_tuner_cnt||tuner_id>=MAX_TUNER_SUPPORT_NUM)
		return RET_FAILURE;
	vz7306_ptr = vz7306_dev_id[tuner_id];
#if 1
	UINT32 ratio = 135;	
#endif
	
	
	Rs = sym;
	if (Rs == 0)
		Rs = 45000;
#if 1
	BW = Rs*135/200;                
	BW = BW*130/100;                
	if (Rs < 6500)
		BW = BW + 3000;				
	BW = BW + 2000;                 
	BW = BW*108/100;                
#else
	if (ratio == 0)
		BW = 34000;
	else
		BW = Rs * ratio / 100;		
#endif
	if (BW < 10000)
		BW = 10000;					
	if (BW > 34000)
		BW = 34000;
	
	if (BW <= 10000)
		LPF = 3;
	else if (BW <= 12000)
		LPF = 4;
	else if (BW <= 14000)
		LPF = 5;
	else if (BW <= 16000)
		LPF = 6;
	else if (BW <= 18000)
		LPF = 7;
	else if (BW <= 20000)
		LPF = 8;
	else if (BW <= 22000)
		LPF = 9;
	else if (BW <= 24000)
		LPF = 10;
	else if (BW <= 26000)
		LPF = 11;
	else if (BW <= 28000)
		LPF = 12;
	else if (BW <= 30000)
		LPF = 13;
	else if (BW <= 32000)
		LPF = 14;
	else
		LPF = 15;
	

	if (freq <= 1154)
		DIV = 1;
	else
		DIV = 0;

	if (freq <= 986)
		BA = 5;
	else if (freq <= 1073)
		BA = 6;
	else if (freq <= 1154)
		BA = 7;
	else if (freq <= 1291)
		BA = 1;
	else if (freq <= 1447)
		BA = 2;
	else if (freq <= 1615)
		BA = 3;
	else if (freq <= 1791)
		BA= 4;
	else if (freq <= 1972)
		BA= 5;
	else //if (freq <= 2150)
		BA = 6;


	tmp = freq * 1000 * 8 / REF_OSC_FREQ;	
	Nswa = tmp % 32;						
	Npro = tmp / 32;
	
	
	data[0] = (UINT8) ((Npro >> 3) & 0x1F);

	data[0] = data[0] | 0x40;		
	data[1] = Nswa | (((UINT8)Npro & 0x07) << 5);
	data[2] = 0xE1;								
	data[3] = (BA<<5) | (DIV<<1);

	if ((result = ali_i2c_write(vz7306_ptr->i2c_type_id, vz7306_ptr->c_tuner_base_addr, data+0, 4)) != RET_SUCCESS)
	{
		VZ7306_PRINTF("nim_vz7306_control: I2C write error\n");
		return result;
	}

	VZ7306_PRINTF("[kangzh]line=%d,%s 1!\n",__LINE__, __FUNCTION__);
	
	data[2] = 0xE5;								
	if ((result = ali_i2c_write(vz7306_ptr->i2c_type_id, vz7306_ptr->c_tuner_base_addr, data+2, 1)) != RET_SUCCESS)
		return result;
	udelay(10000);

	VZ7306_PRINTF("[kangzh]line=%d,%s 2!\n",__LINE__, __FUNCTION__);
	
	data[2] |= ((LPF & 0x01) << 4) | ((LPF & 0x02) << 2);
	data[3] |= ((LPF & 0x04) << 1) | ((LPF & 0x08) >> 1);
	if ((result = ali_i2c_write(vz7306_ptr->i2c_type_id, vz7306_ptr->c_tuner_base_addr, data+2, 2)) != RET_SUCCESS)
		return result;

	VZ7306_PRINTF("[kangzh]line=%d,%s end!\n",__LINE__, __FUNCTION__);
	return RET_SUCCESS;
}

/*****************************************************************************
* INT32 nim_vz7306_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock		: Phase lock status
*
* Return Value: INT32			: Result
*****************************************************************************/
INT32 ali_nim_vz7306_status(UINT32 tuner_id,UINT8 *lock)
{
	INT32 result;
	UINT8 data;

	struct QPSK_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	VZ7306_PRINTF("[kangzh]line=%d,%s enter!\n",__LINE__, __FUNCTION__);
	
	if(tuner_id>=vz7306_tuner_cnt||tuner_id>=MAX_TUNER_SUPPORT_NUM)
	{
		*lock = 0;
		VZ7306_PRINTF("[kangzh]line=%d,%s back!\n",__LINE__, __FUNCTION__);
		
		return RET_FAILURE;
	}
	tuner_dev_ptr = vz7306_dev_id[tuner_id];
	if ((result = ali_i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1)) == RET_SUCCESS)
	{
		VZ7306_PRINTF("nim_vz7306_status: data = 0x%x\n", data);
		*lock = ((data & 0x40) >> 6);
	}

	VZ7306_PRINTF("[kangzh]line=%d,%s end!\n",__LINE__, __FUNCTION__);
	return result;
}

#ifndef TWO_TUNER_SUPPORT
static int __devinit ali_tuner_vz7306_init(void)
{
	
	return RET_SUCCESS;
	
}

static void __exit ali_tuner_vz7306_exit(void)
{

}
#else
static int __devinit ali_tuner_vz7306_init(void)
{

	return RET_SUCCESS;
	
}

static void __exit ali_tuner_vz7306_exit(void)
{

}

#endif

module_init(ali_tuner_vz7306_init);
module_exit(ali_tuner_vz7306_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Li");
MODULE_DESCRIPTION("Sharp VZ7306 tuner driver for Ali M3501");


