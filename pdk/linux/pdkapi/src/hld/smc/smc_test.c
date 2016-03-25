/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: smc_test.c
 *
 *  Description: This file contains all functions to test smart card interface
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 2005.9.8  Gushun Chen     0.1.000    Initial
 *
 ****************************************************************************/

#include <adr_retcode.h>
#include <osal/osal.h>
#include <hld/adr_hld_dev.h>

#include <hld/smc/adr_smc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <linux/dvb/ca.h>


//#define DEBUG_SMC_TEST
#ifdef DEBUG_SMC_TEST
#define SMC_TEST_PRINTF	libc_printf
#define SMC_TEST_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) SMC_TEST_PRINTF(" 0x%02x,",*((data)+i)); \
                         SMC_TEST_PRINTF("\n"); }
#else
#define SMC_TEST_PRINTF(...)	do{}while(0)
#define SMC_TEST_DUMP(data,len) ;
#endif

/**************************************************************************
*
*	Name	:	smart_test()
*
**************************************************************************/
UINT8 cmd_answer[256];
UINT32 send_ca_cmd(struct smc_device *handler, UINT8 *cmd, UINT16 len_c)
{
	UINT16 act_len_c;
	UINT16 act_len_a;
	UINT16 i;
	SMC_TEST_PRINTF("CMD: ");
	SMC_TEST_DUMP(cmd, len_c);
	if(smc_raw_write(handler, cmd, len_c, &act_len_c) != SUCCESS)
	{
		SMC_TEST_PRINTF("write cmd fail!\n");
		return !SUCCESS;
	}
	memset(cmd_answer, 0, 256);
	if(smc_raw_read(handler, cmd_answer, 256, &act_len_a) != SUCCESS)
	{
		SMC_TEST_PRINTF("read card answer fail!\n");
		return !SUCCESS;
	}
	SMC_TEST_PRINTF("ANSWER: ");
	SMC_TEST_DUMP(cmd_answer, act_len_a);
	return SUCCESS;
}

static UINT8 irdeto_cmd[10][7] = {
	{0x01,0x02,0x02,0x03,0x00,0x00,0x3d}, //country code is request
	{0x01,0x02,0x00,0x03,0x00,0x00,0x3f}, //ascll serial number is requset
	{0x01,0x02,0x01,0x03,0x00,0x00,0x3e}, //hex serial number is requested
	{0x01,0x02,0x03,0x03,0x00,0x00,0x3c}, //The provider id for provider 00 is requested
	{0x01,0x02,0x03,0x03,0x01,0x00,0x3c}, //The provider id for provider 10 is requested
	{0x01,0x02,0x08,0x03,0x00,0x00,0x37}, //The card's configuration is requested
	{0x01,0x02,0x03,0x03,0x02,0x00,0x3c}, //The provider id for provider 20 is requested (IRDETO2)
	{0x01,0x02,0x03,0x03,0x03,0x00,0x3c}, //The provider id for provider 30 is requested (IRDETO2)
	{0x01,0x02,0x0e,0x02,0x00,0x00,0x30}, //The card's card file 1 is requested
	{0x01,0x02,0x0e,0x02,0x00,0x00,0x31}, //The card's card file 2 is requested
};

static UINT8 viaccess_cmd[6][5] = {
	{0xca, 0xac, 0xa4, 0x00, 0x00},	 //request unique id
	{0xca, 0xb8, 0x00, 0x00, 0x07},	 //read unique id
	{0xca, 0xa4, 0x00, 0x00, 0x00},	//select issuer	0
	{0xca, 0xc0, 0x00, 0x00, 0x1a},	//show provider properties
	{0xca, 0xac, 0xa5, 0x00, 0x00},	 //request sa
	{0xca, 0xb8, 0x00, 0x00, 0x07},	 //read sa	
};

static UINT8 ViaCommand[3][5] =
{
	{ 0x87, 0x02, 0x00, 0x00, 0x03 }, // VIACCESS_FAC_READ_DATA
	{ 0x87, 0x04, 0x00, 0x00, 0x07 }, // VIACCESS_FAC_READ_SELECT
	{ 0x87, 0x06, 0x00, 0x00, 0x00 }, // VIACCESS_FAC_READ_BLOCK
};

static UINT8 conax_cmd[2][8] = {
	{0xdd, 0x26, 0x00, 0x00, 0x03, 0x10, 0x01, 0x01},//0x26, 0x10, 0x01, 0x01	(00 00 2 9011; 00 40 2 9011)
	{0xdd, 0xca, 0x00, 0x00, 0x00}	//(00 00 2 9011; 00 40 2 9011)
};

static UINT8 seca_cmd[5][5] = {
	{0xC1,0x0E,0x00,0x00,0x08},	 //get serial nr. (UA)
	{0xC1,0x16,0x00,0x00,0x07}, 	//get nr. of providers
	{0xC1,0x12,0x00,0x00,0x19},	//get provider info
	{0xC1,0x34,0x00,0x00,0x03},	//request provider pbm (data=0x00 0x00 0x00)
	{0xC1,0x32,0x00,0x00,0x0A}	//get the pbm data (p1=provider)
};

static UINT8 nagra_cmd[3][12] = {
	{0x21,0xC1,0x01,0x58,0xB9},	//12 E1 01 58 AA Setting Packet size
	{0x21,0x00,0x08,0xA0,0xCA,0x00,0x00,0x02,0xC0,0x00,0x06,0x87},	//12 00 08 B0 04 00 03 00 00 90 00 3D Cam status Cmd C0
	{0x21,0x40,0x08,0xA0,0xCA,0x00,0x00,0x02,0x12,0x00,0x06,0x15}	//12 40 08 92 04 1B 21 8D 0F 90 00 E4 CAM ID Cmd 12
};

static UINT8 nagra_cmd_t1[2][8] = {    //for T1, only the cmd to send
	{0xA0,0xCA,0x00,0x00,0x02,0xC0,0x00,0x06},
	{0xA0,0xCA,0x00,0x00,0x02,0x12,0x00,0x06}
};

static UINT8 tongda_cmd[5][7] = {
	{0x00,0x84,0x00,0x00,0x04},	//get challenge, random
	{0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00},	//select MF
	{0x00,0xC0,0x00,0x00,0x00},	//get responce
	{0x00,0xA4,0x00,0x00,0x02, 0x2F,0x11},	//select file 2F11
	{0x00,0xC0,0x00,0x00,0x00}	//get responce
};

static UINT8 tongda_cmd_1[4][5] = {
	{0x00, 0x84, 0x00, 0x00, 0x04},
	{0x00, 0xa4, 0x00, 0x00, 0x02},
	{0x2F, 0x11},
	{0x00, 0xc0, 0x00, 0x00, 0x28}
};

static UINT8 tongfang_cmd[3][10] = {
	{0x00, 0xA4, 0x04, 0x00, 0x05, 0xF9, 0x5A, 0x54, 0x00, 0x06},
	{0x80, 0x46, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x04},
	{0x00, 0xC0, 0x00, 0x00, 0x04}
};

static UINT8 cryptworks_cmd[3][7] = {
	{0xA4, 0xA4, 0x00, 0x00, 0x02, 0x2F, 0x01},	// select file 2F01
	{0xA4, 0xA2, 0x00, 0x00, 0x01, 0xD1},		// read record
	{0xA4, 0xB2, 0x00, 0x00, 0xD1},	
};

static UINT8 jetcas_cmd[5][6] = {
	{0xff, 0xff, 0xff, 0xff, 0xff}, //invalid command, resp 6E 00 required
	{0x84,0xaa,0x00,0x00,0x27},       //header to send data
	{0x84,0xaa,0x00,0x00,0x2b},	   // header to get data
	{0x84,0xaa,0x00,0x00,0xa6},	    //header to send data 2
	{0x84,0xaa,0x00,0x00,0x95},	    //header to get data 2
};
static UINT8 cti_cmd[2] [9]= {
	{0x00,0x00,0x05,0x81, 0xd2, 0x00, 0x01, 0x10, 0x47},
	{0x81, 0xd2, 0x00, 0x01, 0x10},
};
static UINT8 jetcas_cmd1[] = { 0x01, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
						 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xd9, 0x90, 
						 0x00};	

static UINT8 jetcas_cmd2[] = { 0x02, 0xa0, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
						 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
						 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
						 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
						 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
						 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
						 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33,
						 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
						 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x44,
						 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 
						 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
						 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b,
						 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63,
						 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
						 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
						 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
						 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
						 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b,
						 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93,
						 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
						 0x9c, 0x9d, 0x9e, 0x9f, 0xa8, 0x08, 0x90, 0x00};

UINT8 tonda_atr[] = {0x3b, 0x6d, 0x00, 0x00, 0x57, 0x44, 0x29, 0x46, 0x21, 0x86, 0x93, 0x03/*, 0x24, 0x23, 0x14, 0x09, 0x47*/};
UINT8 tongfang_atr[] = {0x3b, 0x6c, 0x00, 0x00, 0x4E, 0x54, 0x49, 0x43};
UINT32 try_times = 0;
UINT32 atr_error_times = 0;
UINT32 cmd_error_times = 0;

void smart_thread()
{		
	struct smc_device *smc_dev = NULL;
	UINT8 atr_buffer[33];
	UINT16 i = 0;
	UINT16 atr_size = 0;
	UINT8 response_buffer[256];
	UINT8 smart_status[2];
	UINT8 irdeto_flag = 0;
	UINT8 conax_flag = 0;
	UINT8 viaccess_flag = 0;
	UINT8 seca_flag = 0;
	UINT8 nagra_flag = 0;
	UINT8 tongda_flag = 0;
	UINT8 tongfang_flag = 0;
	UINT8 cryptworks_flag = 0;
	INT16 num_size = 0;
	UINT8 jetcas_flag = 0;
	UINT8 cti_flag = 0;
	INT32 tmp = 0;

	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);

	osal_task_save_thread_info("SMO");

	while(1)
	{
		if(smc_card_exist(smc_dev) != SUCCESS)
		{
#if 0
#include <bus/sci/sci.h>
				sci_mode_set(SCI_FOR_MDM,9600, 0x001);
				libc_printf("fifo write1:\n");
				sci_fifowrite(SCI_FOR_MDM, jetcas_cmd1,  39);
				for (i=0; i<39; i++)
				{
					tmp = sci_read_tm(SCI_FOR_MDM, &response_buffer[i], 10000);
					if (tmp != SUCCESS)
						break;
				}
				for (tmp=0; tmp<i; tmp++)
					libc_printf("%02x ", response_buffer[tmp]);
				libc_printf("\n");
				
				osal_task_sleep(10);
				libc_printf("fifo write2:\n");
				sci_fifowrite(SCI_FOR_MDM, jetcas_cmd[2], 5);
				for (i=0; i<5; i++)
				{
					tmp = sci_read_tm(SCI_FOR_MDM, &response_buffer[i], 10000);
					if (tmp != SUCCESS)
						break;
				}
				for (tmp=0; tmp<i; tmp++)
					libc_printf("%02x ", response_buffer[tmp]);
				libc_printf("\n");
				osal_task_sleep(1000);
				//sci_write(SCI_FOR_MDM, 0x99);
				libc_printf("jetcas test:\n");
				sci_fifowrite(SCI_FOR_MDM, jetcas_test1,  6);
				for (i=0; i<6; i++)
				{
					tmp = sci_read_tm(SCI_FOR_MDM, &response_buffer[i], 10000);
					if (tmp != SUCCESS)
						break;
				}
				for (tmp=0; tmp<i; tmp++)
					libc_printf("%02x ", response_buffer[tmp]);
				libc_printf("\n");
			
#endif
			osal_task_sleep(100);
			continue;
		}
		memset(atr_buffer, 0, 33);
		try_times ++;
		if(RET_SUCCESS != smc_reset(smc_dev, atr_buffer, &atr_size)) {
//			SMC_TEST_PRINTF("Smart card reset error!\n");
			atr_error_times ++;
			SMC_TEST_PRINTF("try1:%d, atr_error:%d\n", try_times, atr_error_times);
#if 0
			SMC_TEST_PRINTF("ATR: ");
			for(i=0; i<atr_size; i++) {
				SMC_TEST_PRINTF("%02x ", *(atr_buffer+i));
			}
			SMC_TEST_PRINTF("\n");
#endif			
			smc_deactive(smc_dev);
			osal_task_sleep(1000);
			continue;
		}
		SMC_TEST_DUMP(atr_buffer, atr_size);

		if(0)
		{
			smc_deactive(smc_dev);

			osal_task_sleep(1000);
			continue;
		}

		irdeto_flag = 0;
		conax_flag = 0;
		viaccess_flag = 0;
		seca_flag = 0;
		nagra_flag = 0;
		tongda_flag = 0;
		cryptworks_flag = 0;
		jetcas_flag = 0;
		cti_flag = 0;

		if(atr_buffer[1] == 0x9F) irdeto_flag = 1;
		else if(atr_buffer[1] == 0x24) conax_flag = 1;
		else if((atr_buffer[1] == 0x77) || (atr_buffer[1] == 0x3F)) viaccess_flag = 1;
		else if(atr_buffer[1] == 0xF7) seca_flag = 1;
		else if(atr_buffer[1] == 0xFF) nagra_flag = 1;
		else if(atr_buffer[1] == 0x6D) tongda_flag = 1;
		else if(atr_buffer[1] == 0x6C) tongfang_flag = 1;
		else if(atr_buffer[1] == 0x78) cryptworks_flag = 1;
		else if(atr_buffer[1] == 0x7f) jetcas_flag = 1;
		else if (atr_buffer[1] == 0xe9) cti_flag = 1;
		else {
			SMC_TEST_PRINTF("Unknow CA system Card!\n");
			continue;
		}

		if(irdeto_flag == 1) {
			SMC_TEST_PRINTF("Irdeto Card!\n");
			for(i=0; i<10; i++) {
				if(send_ca_cmd(smc_dev, irdeto_cmd[i], 7) != SUCCESS)
				{
					cmd_error_times ++;
					SMC_TEST_PRINTF("try%d:%d, cmd_error:%d\n", i, try_times, cmd_error_times);
					break;
				}
				osal_task_sleep(1000);
			}
			if(i == 10)
				SMC_TEST_PRINTF("CMD OK\n");	
		}
		else if (cti_flag == 1)
		{
			SMC_TEST_PRINTF("CTI Card!\nCMD:");
			SMC_TEST_DUMP(cti_cmd[0], 9);
			tmp =0;
			#if 1
			if (smc_t1_xcv(smc_dev, cti_cmd, 9, cmd_answer,22, &tmp) != 0)
			{
				SMC_TEST_PRINTF("try%d:%d, cmd_error:%d\n", i, try_times, cmd_error_times);
				continue;
			}
			for (i=0; i<tmp;i++)
				SMC_TEST_PRINTF(" %02x ",cmd_answer[i]);
			SMC_TEST_PRINTF("\n");
			#else
			tmp = smc_t1_transfer(smc_dev, 0x00, cti_cmd[1], 5, cmd_answer, 22);
			if (tmp < 0)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{
				SMC_TEST_PRINTF("ANSWER:");
				SMC_TEST_DUMP(cmd_answer, tmp);
			}
			#endif
			#if 0
			if (send_ca_cmd(smc_dev, cti_cmd,9 ) != SUCCESS)
			{
				cmd_error_times ++;
					SMC_TEST_PRINTF("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
					continue;
			}
			#endif
			osal_task_sleep(1000);
		}
			
		else if (jetcas_flag == 1){
			osal_task_sleep(2000);
			SMC_TEST_PRINTF("JetCAS card!\n");
			if (send_ca_cmd(smc_dev, jetcas_cmd[0],5 ) != SUCCESS)
			{
				cmd_error_times ++;
					SMC_TEST_PRINTF("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
					continue;
			}
			osal_task_sleep(1000);
			
			if (send_ca_cmd(smc_dev, jetcas_cmd[1],5) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			osal_task_sleep(1000);
			if (send_ca_cmd(smc_dev, jetcas_cmd1,41) != SUCCESS)// send data and receive resp.
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			/*osal_task_sleep(1);
			if (send_ca_cmd(smc_dev, jetcas_cmd[2],5) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}*/
			osal_task_sleep(1000);
			if (send_ca_cmd(smc_dev, jetcas_cmd[3],5) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			//osal_task_sleep(1000);
			if (send_ca_cmd(smc_dev, jetcas_cmd2,168) != SUCCESS)// send data and receive resp.
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			/*osal_task_sleep(1000);
			if (send_ca_cmd(smc_dev, jetcas_cmd[4],5) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}*/
			
		}
		else if(conax_flag == 1) {
			SMC_TEST_PRINTF("Conax Card!\n");
			if(smc_iso_transfer(smc_dev, conax_cmd[0], 8, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			osal_task_sleep(1000);
			conax_cmd[1][4] = smart_status[1];
			if(smc_iso_transfer(smc_dev, conax_cmd[1], 5, response_buffer, conax_cmd[1][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			osal_task_sleep(1000);
			SMC_TEST_PRINTF("CMD OK\n");	
		}			
		else if(viaccess_flag == 1) {
			SMC_TEST_PRINTF("Viaccess Card!\n");
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
#if 1
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(viaccess_cmd[0], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[0], 5, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{	
				SMC_TEST_PRINTF("cmd OK!\n");
			}
//				osal_task_sleep(1000);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(viaccess_cmd[1], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[1], 5, response_buffer, viaccess_cmd[1][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{
				SMC_TEST_DUMP(response_buffer, viaccess_cmd[1][4]);
			}
//				osal_task_sleep(1000);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(viaccess_cmd[2], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[2], 5, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{	
				SMC_TEST_PRINTF("cmd OK!\n");
			}
//				osal_task_sleep(1000);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(viaccess_cmd[3], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[3], 5, response_buffer, viaccess_cmd[3][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{
				SMC_TEST_DUMP(response_buffer, viaccess_cmd[3][4]);
			}
//				osal_task_sleep(1000);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(viaccess_cmd[4], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[4], 5, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{	
				SMC_TEST_PRINTF("cmd OK!\n");
			}
//				osal_task_sleep(1000);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(viaccess_cmd[5], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[5], 5, response_buffer, viaccess_cmd[5][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{
				SMC_TEST_DUMP(response_buffer, viaccess_cmd[5][4]);
			}
//				osal_task_sleep(1000);
#endif			
			SMC_TEST_PRINTF("CMD OK\n");			

		}
		else if(seca_flag == 1) {
			SMC_TEST_PRINTF("Seca Card!\n");
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));			
			if(smc_iso_transfer(smc_dev, seca_cmd[0], 5, response_buffer, seca_cmd[0][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
//				osal_task_sleep(1000);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, seca_cmd[1], 5, response_buffer, seca_cmd[1][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
//				osal_task_sleep(1000);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, seca_cmd[2], 5, response_buffer, seca_cmd[2][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
//				osal_task_sleep(1000);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, seca_cmd[3], 5, response_buffer, seca_cmd[3][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try4:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
//				osal_task_sleep(1000);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, seca_cmd[4], 5, response_buffer, seca_cmd[4][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try5:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
//				osal_task_sleep(1000);
			SMC_TEST_PRINTF("CMD OK\n");			
		}
		else if(nagra_flag == 1) {	/*4M clock, UART baudrate is 115200, can not support ETU=32*/
			SMC_TEST_PRINTF("Nagra Card!\n");	
			#if 0
			if(send_ca_cmd(smc_dev, nagra_cmd[0], 5) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			osal_task_sleep(1000);
			if(send_ca_cmd(smc_dev, nagra_cmd[1], 12) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			osal_task_sleep(1000);
			if(send_ca_cmd(smc_dev, nagra_cmd[2], 12) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try4:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			#else
			#if 0          //This branch only test if the read/write function  works fine
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(nagra_cmd[0], 5);
			if (smc_t1_xcv(smc_dev, nagra_cmd[0], 5, cmd_answer,256, &tmp) != 0)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else 
			{
				SMC_TEST_DUMP(cmd_answer, tmp);
			}
			
			osal_task_sleep(2000);
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(nagra_cmd[1], 12);
			if (smc_t1_xcv(smc_dev, nagra_cmd[1], 12, cmd_answer,256, &tmp) !=  0)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else 
			{
				SMC_TEST_DUMP(cmd_answer, tmp);
			}
			
			osal_task_sleep(2000);
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(nagra_cmd[2], 12);
			if (smc_t1_xcv(smc_dev, nagra_cmd[2], 12, cmd_answer,256, &tmp) != 0)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else 
			{
				SMC_TEST_DUMP(cmd_answer, tmp);
			}
			
			#else    //this branch tests the t1 protocol write/read
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(nagra_cmd[0], 5);
			if (smc_t1_negociate_ifsd(smc_dev, 0x21, 0x58)<0)//for ifsd negociation, only nad and size is need
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{
				SMC_TEST_PRINTF("\nSMC ifsd negociation success!\n");
			}

			osal_task_sleep(1000);
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(nagra_cmd[1], 12);
			tmp = smc_t1_transfer(smc_dev, 0x21, nagra_cmd_t1[0], 8, cmd_answer, 256);
			if (tmp < 0)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{
				SMC_TEST_PRINTF("ANSWER:");
				SMC_TEST_DUMP(cmd_answer, tmp);
			}
			
			osal_task_sleep(1000);
			SMC_TEST_PRINTF("CMD:");
			SMC_TEST_DUMP(nagra_cmd[2], 12);
			tmp = smc_t1_transfer(smc_dev, 0x21, nagra_cmd_t1[1], 8, cmd_answer, 256);
			if (tmp < 0)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			else
			{
				SMC_TEST_PRINTF("ANSWER:");
				SMC_TEST_DUMP(cmd_answer, tmp);
			}
			#endif
			#endif
			SMC_TEST_PRINTF("CMD OK\n\n\n");	
			
		}
		else if(tongda_flag == 1) {
			SMC_TEST_PRINTF("TongDa Card!\n");
			if(memcmp(atr_buffer, tonda_atr, sizeof(tonda_atr)) != 0)
			{
				atr_error_times ++;
				SMC_TEST_PRINTF("try2:%d, atr_error:%d\n", try_times, atr_error_times);
				SMC_TEST_PRINTF("ATR: ");
				for(i=0; i<atr_size; i++) {
					SMC_TEST_PRINTF("%02x ", *(atr_buffer+i));
				}
				SMC_TEST_PRINTF("\n");
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			osal_task_sleep(1);
			if(smc_iso_transfer(smc_dev, tongda_cmd[0], 5, response_buffer, 256, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(response_buffer, tongda_cmd[0][4]);
				continue;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, tongda_cmd[1], 7, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try4:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(smart_status, 2);
				continue;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			tongda_cmd[2][4] = smart_status[1];
			if(smc_iso_transfer(smc_dev, tongda_cmd[2], 5, response_buffer, tongda_cmd[2][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try5:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(response_buffer, tongda_cmd[2][4]);
				continue;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, tongda_cmd[3], 7, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;			
				SMC_TEST_PRINTF("try6:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(smart_status, 2);
				continue;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			tongda_cmd[4][4] = smart_status[1];
			if(smc_iso_transfer(smc_dev, tongda_cmd[4], 5, response_buffer, tongda_cmd[2][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;			
				SMC_TEST_PRINTF("try7:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(response_buffer, tongda_cmd[4][4]);
				continue;
			}
			osal_task_sleep(1000);			
			SMC_TEST_PRINTF("CMD OK\n");
		}
		else if(tongfang_flag == 1) {
			SMC_TEST_PRINTF("TongFang Card!\n");
			if(memcmp(atr_buffer, tongfang_atr, sizeof(tongfang_atr)) != 0)
			{
				atr_error_times ++;
				SMC_TEST_PRINTF("try2:%d, atr_error:%d\n", try_times, atr_error_times);
				SMC_TEST_PRINTF("ATR: ");
				for(i=0; i<atr_size; i++) {
					SMC_TEST_PRINTF("%02x ", *(atr_buffer+i));
				}
				SMC_TEST_PRINTF("\n");
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			osal_task_sleep(1);
			if(smc_iso_transfer(smc_dev, tongfang_cmd[0], 10, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(response_buffer, tongda_cmd[0][4]);
				continue;
			}
			else if((smart_status[0] != 0x90)||(smart_status[1] != 0x00))
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try4:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(smart_status, 2);
				continue;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, tongfang_cmd[1], 9, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try5:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(smart_status, 2);
				continue;
			}
			else if((smart_status[0] != 0x61)||(smart_status[1] != 0x04))
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try6:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(smart_status, 2);
				continue;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, tongfang_cmd[2], 5, response_buffer, tongda_cmd[2][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try7:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(response_buffer, tongfang_cmd[2][4]);
				continue;
			}
			else if((smart_status[0] != 0x90)||(smart_status[1] != 0x00))
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try8:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(smart_status, 2);
				continue;
			}
			osal_task_sleep(1000);
			SMC_TEST_PRINTF("CMD OK\n");
		}
		else if(cryptworks_flag == 1)
		{
			SMC_TEST_PRINTF("Cryptworks Card!\n");

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			osal_task_sleep(1);
			if(smc_iso_transfer(smc_dev, cryptworks_cmd[0], 7, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, cryptworks_cmd[1], 6, NULL, 0, smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try5:%d, cmd_error:%d\n", try_times, cmd_error_times);
				continue;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, cryptworks_cmd[2], 5, response_buffer, cryptworks_cmd[2][4], smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_TEST_PRINTF("try7:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_TEST_DUMP(response_buffer, cryptworks_cmd[2][4]);
				continue;
			}
			osal_task_sleep(1000);
			SMC_TEST_PRINTF("CMD OK\n");
		}
		smc_deactive(smc_dev);
	}

}

void smart_test()
{
	UINT32 thread_id;
	OSAL_T_CTSK t_ctsk;

	/* create smart test thread */
	t_ctsk.task = (OSAL_T_TASK_FUNC_PTR)smart_thread;
	t_ctsk.itskpri = OSAL_PRI_LOW;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.name[0] = 'S';
	t_ctsk.name[1] = 'M';
	t_ctsk.name[2] = '0';
	thread_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == thread_id)
	{
		SMC_TEST_PRINTF("Can not create smart test thread!!!\n");
		ASSERT(0);
	}
}

INT32 smc_init()
{
	struct smc_device *smc_dev = NULL;
	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(smc_dev == NULL)
	{		
		PRINTF("Get Smart card reader failed\n");
		ASSERT(0);
	}
	if(RET_SUCCESS != smc_open(smc_dev, NULL))
	{
		PRINTF("smc_open failed!!\n");
		ASSERT(0);
	}
	smart_test();
}

