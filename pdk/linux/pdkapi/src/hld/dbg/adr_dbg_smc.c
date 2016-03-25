/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    adr_smc_dbg.c
*
*    Description:    This file contains all diagnostic functions definition
*		             of Front smc driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Feb.21.2013      John Chen       Ver 0.1    Create file.
*****************************************************************************/

#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <osal/osal_task.h>
#include <adr_basic_types.h>
#include <adr_retcode.h>
#include <hld/adr_hld_dev.h>
#include <hld/dbg/adr_dbg_parser.h>
#include <hld/smc/adr_smc.h>
#include <linux/dvb/ca.h>
#include <hld_cfg.h>


#define SMC_DBG_INFO(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(SMC, fmt, ##args); \
			} while(0)

#define SMC_DBG_ERR(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(SMC, fmt, ##args); \
			} while(0)



#define SMC_DBG_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) SMC_DBG_INFO(" 0x%02x,",*((data)+i)); \
                         SMC_DBG_INFO("\n"); }

#ifndef CARD_DETECT_INVERT
#define CARD_DETECT_INVERT	1
#endif

typedef enum smc_dbg_level
{	
	SMC_DBG_LEVEL_DEFAULT = 0x00,
	SMC_DBG_LEVEL_HLD 	  = 0x01,
    	SMC_DBG_LEVEL_KERNEL = 0x02,
}smc_dbg_level_e;

/*! @enum smc_device_status
    @brief 智能卡设备状态。
*/
typedef enum smc_ca_flag
{
	CA_NONE = 0,
	CA_DVT,				/* 1 */
	CA_DVN,				/* 2 */
	CA_CDCA3,			/* 3 */
	CA_CONAX,			/* 4 */
	CA_NAGRA,			/* 5 */
	CA_IRDETO,			/* 6 */
	CA_TF,				/* 7 */	
	CA_CTI,				/* 8 */			
	CA_MG,				/* 9 */
	CA_GOS,				/* 10 */	
	CA_CRYPTON,		/* 11 */
	CA_JET,				/* 12 */
	CA_VIACCESS, 		/* 13 */
	CA_SE,				/* 14 */	
	CA_TONGDA,			/* 15 */
	CA_CRYPTWORKS,	/* 16 */
	CA_UNKNOW
}smc_ca_flag_e;

/*!@struct smc_dbg
   @brief smc dbg 信息。
*/
struct smc_dbg
{		
	smc_dbg_level_e  level;		
};


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

static UINT8 tonda_atr[] = {0x3b, 0x6d, 0x00, 0x00, 0x57, 0x44, 0x29, 0x46, 0x21, 0x86, 0x93, 0x03/*, 0x24, 0x23, 0x14, 0x09, 0x47*/};
static UINT8 tongfang_atr[] = {0x3b, 0x6c, 0x00, 0x00, 0x4E, 0x54, 0x49, 0x43};
static UINT32 try_times = 0;
static UINT32 atr_error_times = 0;
static UINT32 cmd_error_times = 0;
static UINT8 cmd_answer[256];


static INT32 smc_dag_on_off(struct smc_device *dev,UINT8 level)
{		
	struct smc_dbg *smc_dbg_info = NULL;
	UINT32 max_level = SMC_DBG_LEVEL_HLD | SMC_DBG_LEVEL_KERNEL;		

	
	if(NULL == dev)
	{
		SMC_DBG_ERR("[ %s ] error:  dev is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}	

	if (level > max_level)
	{
		SMC_DBG_ERR("[ %s ] error : Invalid parameters!, level = %d, max_level = %d\n", 
			__FUNCTION__, level, max_level);

		return ERR_FAILURE;
	}	

	smc_dbg_info = (struct smc_dbg *)smc_dbg_get_info(dev);	
	if (NULL == smc_dbg_info)
	{
		SMC_DBG_ERR("[ %s ] error:  smc_dbg_info is NULL!\n", __FUNCTION__);
		return ERR_FAILURE;
	}	

	smc_dbg_info->level = (smc_dbg_level_e)level;	
	
	level &= SMC_DBG_LEVEL_KERNEL;		
	if (SUCCESS != smc_io_control(dev, SMC_DRIVER_SET_DEBUG_LEVEL, (UINT32)level))	
	{
		SMC_DBG_ERR("[ %s %d ], Fail!\n", __FUNCTION__, __LINE__);
		perror("");	
	}		
	
	smc_dbg_set_info(dev, smc_dbg_info);	
	
	
	return SUCCESS;
}


static void smc_dbg_show_help(int argc, char **argv)
{	
	SMC_DBG_INFO("Module Name:\n");	
	SMC_DBG_INFO("\tsmc - smc diagnostic module.\n\n");	

	SMC_DBG_INFO("Synopsis:\n");	
	SMC_DBG_INFO("\tadrdbg smc -cmd [arguments] ...\n\n");
	
	SMC_DBG_INFO("Description:\n");
	SMC_DBG_INFO("\t-h, --help\n");
	SMC_DBG_INFO("\t\tself description of this diagnostic module.\n\n");	
	
	SMC_DBG_INFO("\t-l, --level level\n");
	SMC_DBG_INFO("\t\tset smc debugging printout level (default is 0).\n");
	SMC_DBG_INFO("\t\tlevel 0 \t\t--no printout.\n");
	SMC_DBG_INFO("\t\tlevel 1 \t\t--show HLD printout infomation.\n");
	SMC_DBG_INFO("\t\tlevel 2 \t\t--show Kernel printout infomation.\n");
	SMC_DBG_INFO("\t\tlevel 3 \t\t--show HLD & Kernel printout infomation.\n\n");	
	
	SMC_DBG_INFO("\t-i, --info\n");
	SMC_DBG_INFO("\t\tshow smc info.\n\n");			

	SMC_DBG_INFO("\nUsage example:\n");
	SMC_DBG_INFO("\tadrdbg smc -h\n");	
	SMC_DBG_INFO("\tadrdbg smc -l 3\n");	
	SMC_DBG_INFO("\tadrdbg smc -i\n");	
}


static void smc_dbg_set_level(int argc, char **argv)
{
	INT32 ret = ERR_FAILURE;
	int level = SMC_DBG_LEVEL_DEFAULT;	
	UINT32 max_level = SMC_DBG_LEVEL_HLD | SMC_DBG_LEVEL_KERNEL;	
	struct smc_device *smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);
	struct smc_dbg *smc_dbg_info = NULL;	
	
	
	ret = soc_dbg_get_num(1, argv, &level, "l");	
	if (SUCCESS != ret)
	{		
		return;
	}
	
	smc_dbg_info = (struct smc_dbg *)smc_dbg_get_info(smc_dev);		
	if (NULL == smc_dbg_info)
	{
		SMC_DBG_ERR("[ %s ] error:  smc_dbg_info is NULL!\n", __FUNCTION__);
		return;
	}	
	
	smc_dag_on_off(smc_dev, (UINT8)level);	
}




static void smc_dbg_show_info(int argc, char **argv)
{	
	UINT8 i = 0;	
	UINT32 value = 0;
	ca_msg_t atr_msg;
	struct smc_dbg *smc_dbg_info = NULL;	
	struct smc_device *smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);	
	
	
	smc_dbg_info = (struct smc_dbg *)smc_dbg_get_info(smc_dev);	
	if (NULL == smc_dbg_info)
	{
		SMC_DBG_ERR("[ %s ] error:  smc_dbg_info is NULL!\n", __FUNCTION__);
		return;
	}	

	if (SUCCESS != smc_card_exist(smc_dev))
	{	
		SMC_DBG_ERR("[ %s %d ], No SMC!\n", __FUNCTION__, __LINE__);	
		return;
	}
	

	SMC_DBG_INFO("Ali SMC Info\n");	

	value = 0;
	if (SUCCESS == smc_io_control(smc_dev, SMC_DRIVER_CHECK_STATUS, (UINT32)(&value)))
	{
		if (SMC_STATUS_OK == value)
		{
			SMC_DBG_INFO("status \t\t\t : OK\n");	
		}
		else if (SMC_STATUS_NOT_EXIST == value)
		{
			SMC_DBG_INFO("status \t\t\t : NOT EXIST\n");		
			return;
		}
		else if (SMC_STATUS_NOT_RESET == value)
		{
			SMC_DBG_INFO("status \t\t\t : NOT RESET\n");				
		}
		else
		{
			SMC_DBG_ERR("status \t\t\t : %d, error\n", value);				
		}		
	}

	value = 0;
	if (SUCCESS == smc_io_control(smc_dev, SMC_DRIVER_GET_PROTOCOL, (UINT32)(&value)))
	{
		SMC_DBG_INFO("T \t\t\t : t%d\n", value);	
	}		

	value = 0;
	if (SUCCESS == smc_io_control(smc_dev, SMC_DRIVER_GET_F, (UINT32)(&value)))
	{
		SMC_DBG_INFO("F \t\t\t : %d\n", value);	
	}	

	value = 0;
	if (SUCCESS == smc_io_control(smc_dev, SMC_DRIVER_GET_D, (UINT32)(&value)))
	{
		SMC_DBG_INFO("D \t\t\t : %d\n", value);	
	}	

	value = 0;
	if (SUCCESS == smc_io_control(smc_dev, SMC_DRIVER_GET_ATR_RESULT, (UINT32)(&value)))
	{
		if (SMC_ATR_OK == value)
		{
			memset(&atr_msg, 0, sizeof(atr_msg));
			atr_msg.type=0;  /* 0 for ATR */
			if (SUCCESS == smc_io_control(smc_dev, CA_GET_MSG, (UINT32)(&atr_msg)))	
			{
				SMC_DBG_INFO("ATR \t\t\t : ");	
				for (i=0; i<atr_msg.length; i++)
				{
					SMC_DBG_INFO("%02x ", atr_msg.msg[i]);
				}
				SMC_DBG_INFO("\n");
			}			
		}
		else
		{
			SMC_DBG_INFO("ATR \t\t\t : %d, error\n", value);	
		}
	}		
}


/**************************************************************************
*
*	Name	:	smart_test()
*
**************************************************************************/
static UINT32 send_ca_cmd(struct smc_device *handler, UINT8 *cmd, UINT16 len_c)
{
	UINT16 act_len_c;
	UINT16 act_len_a;
	UINT16 i;
	SMC_DBG_INFO("CMD: ");
	SMC_DBG_DUMP(cmd, len_c);
	if(smc_raw_write(handler, cmd, len_c, &act_len_c) != SUCCESS)
	{
		SMC_DBG_INFO("write cmd fail!\n");
		return !SUCCESS;
	}
	memset(cmd_answer, 0, 256);
	if(smc_raw_read(handler, cmd_answer, 256, &act_len_a) != SUCCESS)
	{
		SMC_DBG_INFO("read card answer fail!\n");
		return !SUCCESS;
	}
	SMC_DBG_INFO("ANSWER: ");
	SMC_DBG_DUMP(cmd_answer, act_len_a);
	return SUCCESS;
}



/* Smart card init */
static void smc_dbg_init(int argc, char **argv)
{
	struct smc_dev_cfg smc_config;
	UINT32 init_clk = 3570000;//6000000;
	int ca_flag = SMC_DBG_LEVEL_DEFAULT;		
	struct smc_device *smc_dev = NULL;		
	UINT8 buffer = 0;
	
	
	if (SUCCESS  != soc_dbg_get_num(1, argv, &ca_flag, "in"))		
	{		
		return;
	}	

	memset(&smc_config, 0, sizeof(struct smc_dev_cfg));
	smc_config.init_clk_trigger= 1;
	smc_config.init_clk_number = 1;
	smc_config.force_tx_rx_trigger = 1;
	smc_config.apd_disable_trigger = 1;

	
	SMC_DBG_INFO("[ %s %d ], ca_flag = %d\n", __FUNCTION__, __LINE__, ca_flag);	

	if (ca_flag ==CA_DVN)  				/* for dvn card,can not do pps. */
	{
		smc_config.disable_pps = 1;
		init_clk = 5000000;
		SMC_DBG_INFO("[ %s %d ], DVN card\n", __FUNCTION__, __LINE__);	
	}		
	else if (ca_flag == CA_IRDETO)
	{
		smc_config.def_etu_trigger = 0;
		smc_config.disable_pps = 1;
		smc_config.parity_disable_trigger = 1;
		init_clk = 6000000;
		SMC_DBG_INFO("[ %s %d ], Irdeto card\n", __FUNCTION__, __LINE__);	
	}
	else if (ca_flag == CA_CONAX)
	{
		smc_config.def_etu_trigger = 1; /*def etu for irdeto card is not 372, use HW detect it*/
		smc_config.default_etu = 372;
	    	smc_config.warm_reset_trigger = 1;
		smc_config.force_tx_rx_cmd = 0xdd;//CONAX command start.
		smc_config.force_tx_rx_cmd_len = 5; //CONAX command length.	
		init_clk = 3600000;
		SMC_DBG_INFO("[ %s %d ], Conax card\n", __FUNCTION__, __LINE__);	
	}
	else if (ca_flag == CA_CDCA3)
	{
		smc_config.def_etu_trigger = 1;
		smc_config.default_etu = 372;
		init_clk = 5000000;
	}
	else if (ca_flag == CA_CRYPTON)//gavin 20101118
	{
		smc_config.def_etu_trigger = 1;
		smc_config.default_etu = 372;
		init_clk = 3570000;
		SMC_DBG_INFO("[ %s %d ], Crypton card\n", __FUNCTION__, __LINE__);	
	}
	else if (ca_flag == CA_DVT)
	{
		smc_config.default_etu = 372;
		smc_config.def_etu_trigger = 1;
		init_clk = 5000000;		
		SMC_DBG_INFO("[ %s %d ], DVT card\n", __FUNCTION__, __LINE__);	
	}	
	
	smc_config.init_clk_array = &init_clk;

	//SMC_DBG_INFO("[ %s %d ], CARD_DETECT_INVERT = %d\n", __FUNCTION__, __LINE__, CARD_DETECT_INVERT);
    smc_config.invert_detect = CARD_DETECT_INVERT;

    if (ERR_FAILURE == soc_read8(0x18000001, &buffer, 1))
    {
    	return;
    }
	
	if ((ALI_M3921 == soc_get_chip_id()) && (1 == (buffer & 0x01)))		/* BGA445 */
	{
		smc_dev_attach(0, &smc_config);	
	}
	else if ((ALI_M3921 == soc_get_chip_id()) && (0 == (buffer & 0x01)))	/* QFP256 */												/* QFP256 */
	{
		smc_dev_attach(1, &smc_config);	
	}
	

	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(smc_dev == NULL)
	{		
		SMC_DBG_ERR("Get Smart card reader failed\n");	
		return;
	}	
	
	if(RET_SUCCESS != smc_open(smc_dev, NULL))
	{
		SMC_DBG_ERR("Get Smart card reader failed\n");	
		return;
	}
}


static void smc_dbg_close(int argc, char **argv)
{	
	struct smc_device *smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);	
	
	smc_deactive(smc_dev);
	smc_close(smc_dev);	
	smc_dev_dettach(0);
}


static void smc_dbg_test(int argc, char **argv)
{
	UINT8 atr_buffer[33];
	UINT16 i = 0;
	UINT16 atr_size = 0;
	UINT8 response_buffer[256];
	UINT8 smart_status[2];
	UINT32 ca_flag = 0;	
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
	struct smc_device *smc_dev = NULL;	
	UINT32 input_data[2];
	UINT32 test_cnt = 1;		
	UINT32 tick = 0;	
	
		
	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(smc_dev == NULL)
	{		
		SMC_DBG_ERR("Get Smart card reader failed\n");
		return;	
	}	

	
	memset(input_data, 0x00, sizeof(input_data));
	if (SUCCESS != soc_dbg_get_num(2, argv, (int *)&input_data, "t"))
	{		
		return;
	}	

	ca_flag = input_data[0];
	test_cnt = input_data[1];	

	SMC_DBG_INFO("[ %s %d ], ca_flag = %d, test_cnt = %d\n", __FUNCTION__, __LINE__, ca_flag, test_cnt);

	while((test_cnt--)> 0)
	{
		if(smc_card_exist(smc_dev) != SUCCESS)
		{
			SMC_DBG_ERR("NO SMC\n");
			return;	
		}
		
		memset(atr_buffer, 0, 33);
		try_times ++;

		tick = osal_get_tick();
		if(RET_SUCCESS != smc_reset(smc_dev, atr_buffer, &atr_size)) 
		{
			atr_error_times ++;
			SMC_DBG_INFO("[ %s %d , try1:%d, atr_error:%d\n", __FUNCTION__, __LINE__, 
				try_times, atr_error_times);
			smc_deactive(smc_dev);
			osal_task_sleep(1000);
			perror("");
			goto test_end;		
		}
		SMC_DBG_INFO("[ %s %d ], get atr cost tick %d\n", __FUNCTION__, __LINE__, osal_get_tick() - tick);
		SMC_DBG_DUMP(atr_buffer, atr_size);		

		//goto test_end;		
		
		if (CA_UNKNOW == ca_flag)
		{
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
				SMC_DBG_ERR("Unknow CA system Card!\n");
				goto test_end;		
			}
		}

		if ((CA_IRDETO == ca_flag) || (irdeto_flag == 1)) {
			SMC_DBG_INFO("Irdeto Card!\n");
			for(i=0; i<10; i++) {
				if(send_ca_cmd(smc_dev, irdeto_cmd[i], 7) != SUCCESS)
				{
					cmd_error_times ++;
					SMC_DBG_ERR("try%d:%d, cmd_error:%d\n", i, try_times, cmd_error_times);
					break;
				}
				osal_task_sleep(1000);
			}
			if(i == 10)
			{
				SMC_DBG_INFO("CMD OK\n");	
			}
		}
		else if ((CA_CTI == ca_flag) || (cti_flag == 1))
		{
			SMC_DBG_INFO("CTI Card!\nCMD:");
			SMC_DBG_DUMP(cti_cmd[0], 9);
			tmp =0;
			
			if (smc_t1_xcv(smc_dev, cti_cmd, 9, cmd_answer,22, &tmp) != 0)
			{
				SMC_DBG_INFO("try%d:%d, cmd_error:%d\n", i, try_times, cmd_error_times);
				goto test_end;
			}
			for (i=0; i<tmp;i++)
				SMC_DBG_INFO(" %02x ",cmd_answer[i]);
			SMC_DBG_INFO("\n");
			
			osal_task_sleep(1000);
		}	
		else if ((CA_JET == ca_flag) || (jetcas_flag == 1)){
			osal_task_sleep(2000);
			SMC_DBG_INFO("JetCAS card!\n");
			if (send_ca_cmd(smc_dev, jetcas_cmd[0],5 ) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			osal_task_sleep(1000);
			
			if (send_ca_cmd(smc_dev, jetcas_cmd[1],5) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			osal_task_sleep(1000);
			if (send_ca_cmd(smc_dev, jetcas_cmd1,41) != SUCCESS)// send data and receive resp.
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			
			osal_task_sleep(1000);
			if (send_ca_cmd(smc_dev, jetcas_cmd[3],5) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			
			if (send_ca_cmd(smc_dev, jetcas_cmd2,168) != SUCCESS)// send data and receive resp.
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}			
		}
		else if ((CA_CONAX == ca_flag) || (conax_flag == 1)) {
			SMC_DBG_INFO("Conax Card!\n");
			if(smc_iso_transfer(smc_dev, conax_cmd[0], 8, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			osal_task_sleep(1000);
			conax_cmd[1][4] = smart_status[1];
			if(smc_iso_transfer(smc_dev, conax_cmd[1], 5, response_buffer, conax_cmd[1][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			osal_task_sleep(1000);
			SMC_DBG_INFO("CMD OK\n");	
		}			
		else if ((CA_VIACCESS == ca_flag) || (viaccess_flag == 1)) {
			SMC_DBG_INFO("Viaccess Card!\n");
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));

			SMC_DBG_INFO("CMD:");
			SMC_DBG_DUMP(viaccess_cmd[0], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[0], 5, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			else
			{	
				SMC_DBG_INFO("cmd OK!\n");
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_DBG_INFO("CMD:");
			SMC_DBG_DUMP(viaccess_cmd[1], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[1], 5, response_buffer, viaccess_cmd[1][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			else
			{
				SMC_DBG_DUMP(response_buffer, viaccess_cmd[1][4]);
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_DBG_INFO("CMD:");
			SMC_DBG_DUMP(viaccess_cmd[2], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[2], 5, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			else
			{	
				SMC_DBG_INFO("cmd OK!\n");
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_DBG_INFO("CMD:");
			SMC_DBG_DUMP(viaccess_cmd[3], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[3], 5, response_buffer, viaccess_cmd[3][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			else
			{
				SMC_DBG_DUMP(response_buffer, viaccess_cmd[3][4]);
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_DBG_INFO("CMD:");
			SMC_DBG_DUMP(viaccess_cmd[4], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[4], 5, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			else
			{	
				SMC_DBG_INFO("cmd OK!\n");
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			SMC_DBG_INFO("CMD:");
			SMC_DBG_DUMP(viaccess_cmd[5], 5);
			if(smc_iso_transfer(smc_dev, viaccess_cmd[5], 5, response_buffer, viaccess_cmd[5][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			else
			{
				SMC_DBG_DUMP(response_buffer, viaccess_cmd[5][4]);
			}
		
			SMC_DBG_INFO("CMD OK\n");			

		}
		else if ((CA_SE == ca_flag) || (seca_flag == 1)) {
			SMC_DBG_INFO("Seca Card!\n");
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));			
			if(smc_iso_transfer(smc_dev, seca_cmd[0], 5, response_buffer, seca_cmd[0][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, seca_cmd[1], 5, response_buffer, seca_cmd[1][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, seca_cmd[2], 5, response_buffer, seca_cmd[2][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, seca_cmd[3], 5, response_buffer, seca_cmd[3][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try4:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, seca_cmd[4], 5, response_buffer, seca_cmd[4][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try5:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}

			SMC_DBG_INFO("CMD OK\n");			
		}
		else if ((CA_NAGRA == ca_flag) || (nagra_flag == 1)) {	/*4M clock, UART baudrate is 115200, can not support ETU=32*/
			SMC_DBG_INFO("Nagra Card!\n");	
				
			SMC_DBG_INFO("CMD:");
			SMC_DBG_DUMP(nagra_cmd[0], 5);
			if (smc_t1_negociate_ifsd(smc_dev, 0x21, 0x58)<0)//for ifsd negociation, only nad and size is need
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try1:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			else
			{
				SMC_DBG_INFO("\nSMC ifsd negociation success!\n");
			}

			osal_task_sleep(1000);
			SMC_DBG_INFO("CMD:");
			SMC_DBG_DUMP(nagra_cmd[1], 12);
			tmp = smc_t1_transfer(smc_dev, 0x21, nagra_cmd_t1[0], 8, cmd_answer, 256);
			if (tmp < 0)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try2:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			else
			{
				SMC_DBG_INFO("ANSWER:");
				SMC_DBG_DUMP(cmd_answer, tmp);
			}
			
			osal_task_sleep(1000);
			SMC_DBG_INFO("CMD:");
			SMC_DBG_DUMP(nagra_cmd[2], 12);
			tmp = smc_t1_transfer(smc_dev, 0x21, nagra_cmd_t1[1], 8, cmd_answer, 256);
			if (tmp < 0)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			else
			{
				SMC_DBG_INFO("ANSWER:");
				SMC_DBG_DUMP(cmd_answer, tmp);
			}
			
			SMC_DBG_INFO("CMD OK\n\n\n");	
			
		}
		else if ((CA_TONGDA == ca_flag) || (tongda_flag == 1)) {
			SMC_DBG_INFO("TongDa Card!\n");			
			if(memcmp(atr_buffer, tonda_atr, sizeof(tonda_atr)) != 0)
			{
				atr_error_times ++;
				SMC_DBG_INFO("try2:%d, atr_error:%d\n", try_times, atr_error_times);
				SMC_DBG_INFO("ATR: ");
				for(i=0; i<atr_size; i++) {
					SMC_DBG_INFO("%02x ", *(atr_buffer+i));
				}
				SMC_DBG_INFO("\n");
			}
			

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			osal_task_sleep(1);
			if(smc_iso_transfer(smc_dev, tongda_cmd[0], 5, response_buffer, 256, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(response_buffer, tongda_cmd[0][4]);
				goto test_end;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, tongda_cmd[1], 7, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try4:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(smart_status, 2);
				goto test_end;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			tongda_cmd[2][4] = smart_status[1];
			if(smc_iso_transfer(smc_dev, tongda_cmd[2], 5, response_buffer, tongda_cmd[2][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try5:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(response_buffer, tongda_cmd[2][4]);
				goto test_end;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, tongda_cmd[3], 7, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;			
				SMC_DBG_INFO("try6:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(smart_status, 2);
				goto test_end;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			tongda_cmd[4][4] = smart_status[1];
			if(smc_iso_transfer(smc_dev, tongda_cmd[4], 5, response_buffer, tongda_cmd[2][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;			
				SMC_DBG_INFO("try7:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(response_buffer, tongda_cmd[4][4]);
				goto test_end;
			}
			osal_task_sleep(1000);			
			SMC_DBG_INFO("CMD OK\n");
		}
		else if ((CA_CDCA3 == ca_flag) || (tongfang_flag == 1)) {
			SMC_DBG_INFO("TongFang Card!\n");
			
			if(memcmp(atr_buffer, tongfang_atr, sizeof(tongfang_atr)) != 0)
			{
				atr_error_times ++;
				SMC_DBG_INFO("try2:%d, atr_error:%d\n", try_times, atr_error_times);
				SMC_DBG_INFO("ATR: ");
				for(i=0; i<atr_size; i++) {
					SMC_DBG_INFO("%02x ", *(atr_buffer+i));
				}
				SMC_DBG_INFO("\n");
			}			

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			osal_task_sleep(1);
			if(smc_iso_transfer(smc_dev, tongfang_cmd[0], 10, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(response_buffer, tongda_cmd[0][4]);
				goto test_end;
			}
			else if((smart_status[0] != 0x90)||(smart_status[1] != 0x00))
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try4:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(smart_status, 2);
				goto test_end;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, tongfang_cmd[1], 9, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try5:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(smart_status, 2);
				goto test_end;
			}
			else if((smart_status[0] != 0x61)||(smart_status[1] != 0x04))
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try6:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(smart_status, 2);
				goto test_end;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, tongfang_cmd[2], 5, response_buffer, tongda_cmd[2][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try7:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(response_buffer, tongfang_cmd[2][4]);
				goto test_end;
			}
			else if((smart_status[0] != 0x90)||(smart_status[1] != 0x00))
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try8:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(smart_status, 2);
				goto test_end;
			}
			osal_task_sleep(1000);
			SMC_DBG_INFO("CMD OK\n");
		}
		else if ((CA_CRYPTWORKS == ca_flag) || (cryptworks_flag == 1))
		{
			SMC_DBG_INFO("Cryptworks Card!\n");

			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			osal_task_sleep(1);
			if(smc_iso_transfer(smc_dev, cryptworks_cmd[0], 7, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try3:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, cryptworks_cmd[1], 6, NULL, 0, (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try5:%d, cmd_error:%d\n", try_times, cmd_error_times);
				goto test_end;
			}
			osal_task_sleep(1);
			memset(response_buffer, 0, sizeof(response_buffer));
			memset(smart_status, 0, sizeof(smart_status));
			if(smc_iso_transfer(smc_dev, cryptworks_cmd[2], 5, response_buffer, cryptworks_cmd[2][4], (INT16 *)smart_status) != SUCCESS)
			{
				cmd_error_times ++;
				SMC_DBG_INFO("try7:%d, cmd_error:%d\n", try_times, cmd_error_times);
				SMC_DBG_DUMP(response_buffer, cryptworks_cmd[2][4]);
				goto test_end;
			}
			osal_task_sleep(1000);
			SMC_DBG_INFO("CMD OK\n");
		}

		test_end:

		SMC_DBG_INFO("test_cnt = %d\n", test_cnt);
	}	

	SMC_DBG_INFO("\nsmc test end!\n");
}


static PARSE_COMMAND_LIST smcdbg_command[] = {
	{ { NULL, NULL }, smc_dbg_show_help, soc_dbg_no_param_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, smc_dbg_set_level, soc_dbg_one_param_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, smc_dbg_show_info, soc_dbg_no_param_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, smc_dbg_init, soc_dbg_one_param_preview, "init", "in", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, smc_dbg_close, soc_dbg_no_param_preview, "close", "c", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, smc_dbg_test, soc_dbg_two_param_preview, "test", "t", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

void smc_module_register(void)
{	
	debug_module_add("smc", &smcdbg_command[0], ARRAY_SIZE(smcdbg_command));	
}

INT32 smc_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &smcdbg_command[0];
	*list_cnt = ARRAY_SIZE(smcdbg_command);	

	return RET_SUCCESS;
}

INT32 smc_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return smc_dbg_cmd_get(cmd_list, list_cnt);
}
