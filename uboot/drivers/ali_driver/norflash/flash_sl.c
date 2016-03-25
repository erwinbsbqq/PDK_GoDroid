/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_sl.c
 *
 *  Description: Provide local serial flash driver for sto type device.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.4.24   Justin Wu   0.1.000  Initial
 *
 ****************************************************************************/
#include <ali/basic_types.h>
#include <ali/sys_define.h>
#include <ali/retcode.h>
#include <ali/sys_config.h>
#include <ali/sys_parameters.h>
#include <ali/osal.h>

#include <ali/string.h>
//#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
//#include <hal/hal_mem.h>

#include <ali/hld_dev.h>
#include <ali/sto.h>
#include <ali/sto_dev.h>

//#include <bus/flash/flash.h>
//#include <bus/erom/erom.h>
//#include <bus/erom/uart.h>
#include <ali/sto_flash.h>
#include "flash_data.h"
#include "flash.h"

/*
extern void sflash_get_id(UINT32 *result, UINT32 cmdaddr);
extern int sflash_open( void *arg );
extern int sflash_close( void *arg );
extern int sflash_erase_chip(void);
extern int sflash_erase_sector(UINT32 sector_addr);
extern int sflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int sflash_read(void *buffer, void *flash_addr, UINT32 len);
extern int sflash_verify(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern void sflash_set_io(UINT8 io_num, UINT8 board_support_qio);
extern unsigned short sflash_devid;
extern unsigned short sflash_devtp;
*/
static char sto_sflash_local_name[HLD_MAX_NAME_SIZE] = "STO_SFLASH_0";

struct flash_info_st flash_info;

void flash_info_sl_init()
{
	flash_info.flash_deviceid = &sflash_deviceid[0];
	flash_info.flash_deviceid_ex = NULL;	
	flash_info.flash_id_p = &sflash_id[0];
	flash_info.flash_id_p_ex = NULL;	
	flash_info.flash_deviceid_num = sflash_deviceid_num;
	flash_info.flash_deviceid_num_ex = 0;	
	flash_info.flash_io_p = &sflash_io[0];
	flash_info.get_id = sflash_get_id;
	flash_info.erase_chip = sflash_erase_chip;
	flash_info.erase_sector = sflash_erase_sector;
	flash_info.verify = sflash_verify;
	flash_info.write = sflash_copy;
	flash_info.read = sflash_read;
	flash_info.set_io = sflash_set_io;
}


INT32 sto_local_sflash_attach(struct sto_flash_info *param)
{
	struct sto_device *dev;
	struct flash_private *tp;
	unsigned int ret;


	dev = dev_alloc(sto_sflash_local_name, HLD_DEV_TYPE_STO, sizeof(struct sto_device));
	if (dev == NULL)
	{
		PRINTF("Error: Alloc storage device error!\n");
		return ERR_NO_MEM;
	}

	/* Alloc structure space of private */
	if ((tp = MALLOC(sizeof(struct flash_private))) == NULL) {
		dev_free(dev);
		PRINTF("Error: Alloc front panel device priv memory error!\n");
		return ERR_NO_MEM;
	}
	MEMSET(tp, 0, sizeof(struct flash_private));
	dev->priv = tp;

	tp->get_id = sflash_get_id;

	tp->erase_chip = sflash_erase_chip;

	tp->erase_sector = sflash_erase_sector;
	tp->write = sflash_copy;
	tp->read = sflash_read;
	tp->verify = sflash_verify;
	tp->set_io = sflash_set_io;
	tp->open  = NULL;
	tp->close = NULL;
	tp->io_ctrl = NULL;

	if (param == NULL || param->flash_deviceid_num == 0)
	{
		tp->flash_deviceid = &sflash_deviceid[0];
		tp->flash_id_p     	= &sflash_id[0];
		tp->flash_io_p	= &sflash_io[0];
		tp->flash_deviceid_num = sflash_deviceid_num;
	} else
	{
		tp->flash_deviceid = param->flash_deviceid;
		tp->flash_id_p     	= param->flash_id;
		tp->flash_io_p	= param->flash_io;
		tp->flash_deviceid_num = param->flash_deviceid_num;
	}

	/* Current operate address */
	MEMSET(dev->curr_addr, 0, STO_TASK_SUPPORT_NUM_MAX * sizeof(UINT32));

	dev->base_addr = SYS_FLASH_BASE_ADDR;   /* Flash base address */

	//MUTEX_ENTER();
	ret = sto_sflash_identify(dev, 0);
	//MUTEX_LEAVE();

	if (ret == 0) 	{
		PRINTF("Error: Unknow Flash type.\n");
		FREE(tp);
		dev_free(dev);
		return ERR_NO_DEV;
	}

	
	/* Function point init */
	sto_fp_init(dev, sto_local_sflash_attach);

	dev->totol_size = tp->flash_size;

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS) {
		PRINTF("Error: Register Flash storage device error!\n");
		FREE(tp);
		dev_free(dev);
		return ERR_NO_DEV;
	}

	return SUCCESS;
}

/**************************************************************
 * Function:
 * 	sto_flash_identify()
 * Description:
 * 	Identify the parameters of flash.
 * Inpute:
 *	dev --- Device control block.
 *	mode
 *	   0 --- Ideentify the flash attached to dev
 *	   1 --- Identify the local flash and pass the paramters
 *		 to those of dev
 * Return Values:
 *	0 ---  Fail to identify flash
 *	1 ---  Idenfify successfully.
 ***************************************************************/
unsigned int sto_sflash_identify(struct sto_device *dev, UINT32 mode)
{
	unsigned short s,i,j;
	unsigned long id, id_buf[3];
	unsigned char flash_did;
	struct flash_private *tp = (struct flash_private *)dev->priv;

	if( 0 == mode )
		tp->get_id(id_buf, tflash_cmdaddr[0]);
	else {
		/*
		In the mode, one Master STB will upgrade many Slave STBs
		in one time. So we identify local flash and assume that
		all Slave STBs have the same type of flashes. Also, we
		should inform remote_flash driver of the correct flash
		command address.
		*/
		sflash_get_id(id_buf, tflash_cmdaddr[0]);
		/* *below operation only applying to remote_flash* */
		if(tp->io_ctrl!=NULL)
			tp->io_ctrl(FLASH_SET_CMDADDR,sflash_devid);
	}
	for (i = 0; i < (tp->flash_deviceid_num)*2; i += 2)
	{
		s = (tp->flash_deviceid)[i + 1];
		id = id_buf[s >> 5];
		s &= 0x1F;
		flash_did = (tp->flash_deviceid)[i];
		if (((id >> s) & 0xFF) == flash_did)
		{
		      if (0x17==flash_did) 
		     {	  	
				if(((id_buf[0]&0xffffffff)!=0x4d182001));
				else continue;
		     }	
			tp->flash_id = (tp->flash_id_p)[i >> 1];
			tp->flash_io = (tp->flash_io_p)[i >> 1];
			break;
		}
	}
	if (i < (tp->flash_deviceid_num)*2 )
	{
		tp->flash_sectors = (unsigned int) \
			(tflash_sectors[tp->flash_id]);
		tp->flash_size=sto_flash_sector_start(dev,tp->flash_sectors);
		if((id_buf[0]&0xffff) == 0x40EF)//W25Q
			tp->flash_io = 4;		
		else if((id_buf[0]&0xffff) == 0x30EF)//W25X
			tp->flash_io = 1;	// w25x32 can't support 2 line mode
		else if((id_buf[0]&0xffff) == 0x24c2)//MX25L1635D/MX25L3235D
			tp->flash_io = 4;
		else if((id_buf[0]&0xffff) == 0x20c2 && tp->flash_size >= 0x200000) //MX25L1605D/MX25L3205D/MX25L6405D
		{
				tp->flash_io = 1;
		}
		else if((((id_buf[0]>>8)&0xffff) == 0x3037) || (((id_buf[0]>>8)&0xffff) == 0x2037) && (tp->flash_size >= 0x200000))//A25L016,A25L032
			tp->flash_io = 2;
		if(( 2==tp->flash_io || 4 ==tp->flash_io) && sys_ic_get_chip_id()==ALI_S3811&&(*((volatile unsigned long*)0x18000480)&(1<<13)) == 0)
			tp->flash_io = 1;
		if(tp->set_io)
			tp->set_io(tp->flash_io);
	}

	return (i < (tp->flash_deviceid_num)*2);
}
