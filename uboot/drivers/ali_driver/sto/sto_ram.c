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
#include <ali/osal.h>
#include <ali/hld_dev.h>
//#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
#include <ali/string.h>
#include <ali/sto.h>
#include <ali/sto_dev.h>
//#include <ali/flash.h>
#include <ali/sto_flash.h>
//#include "flash_data.h"



void sto_ram_fp_init(struct sto_device *dev,INT32 (*init)());

#if 0
extern void sflash_get_id(UINT32 *result, UINT32 cmdaddr);
extern int sflash_open( void *arg );
extern int sflash_close( void *arg );
extern int sflash_erase_chip(void);
extern int sflash_erase_sector(UINT32 sector_addr);
extern int sflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int sflash_read(void *buffer, void *flash_addr, UINT32 len);
extern int sflash_verify(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int sst26_sf_erase_sector(UINT32 sector_addr);
extern int sst26_sf_copy(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int sst26_sf_read(void *buffer, void *flash_addr, UINT32 len);
extern int sst26_sf_verify(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern void sflash_set_io(UINT8 io_num, UINT8 board_support_qio);
extern unsigned short sflash_devid;
extern unsigned short sflash_devtp;
#endif

static char sto_ram_local_name[HLD_MAX_NAME_SIZE] = "STO_RAM_0";

OSAL_ID sto_mutex_id_ex = OSAL_INVALID_ID;
enum{
    FREE,
    BUSY,
    SUSPEND,
    RESUME,
};
static UINT32 ref_status;
static void sto_mutex_enter_ex(void)
{
#if ( SYS_OS_MODULE == ALI_TDS2 )
	if(OSAL_INVALID_ID == sto_mutex_id_ex)
	{
		sto_mutex_id_ex = osal_mutex_create();
        ref_status = FREE;
		ASSERT(sto_mutex_id_ex != OSAL_INVALID_ID);
	}
	osal_mutex_lock(sto_mutex_id_ex, OSAL_WAIT_FOREVER_TIME);
#endif
    ref_status = BUSY;
}
static void sto_mutex_exit_ex(void)
{
 #if (SYS_OS_MODULE == ALI_TDS2)   
	osal_mutex_unlock(sto_mutex_id_ex);
 #endif 
    ref_status = FREE;
}

static INT32 sto_ram_open(struct sto_device *dev)
{
	return SUCCESS;
}

static INT32 sto_ram_close(struct sto_device *dev)
{
 	return SUCCESS;
}

/*
 * 	Name		:   sto_ram_ioctl()
 *	Description	:   Do miscellaneous routines.
 *	Parameter	:	INT32 cmd, UINT32 param
 *	Return		:	INT32
 *  Current support:
 *     .Chip erase;
 *     .Sector auto erase;
 */
static INT32 sto_ram_ioctl(struct sto_device *dev, INT32 cmd, \
      UINT32 param)
{
	return SUCCESS;
}

static INT32 sto_ram_lseek(struct sto_device *dev, INT32 offset, int origin)
{
    INT32 new_addr;
	int id;

	id = 0;
    if(id>=STO_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		ASSERT(0);
	}
	switch (origin)
	{
		case STO_LSEEK_SET:
			/* Great than totol size, seek to end */
			if (offset >= dev->totol_size)
			{
			    dev->curr_addr[id] = dev->totol_size - 1;
			}
			/* Common seek */
			else if (offset >= 0)
			{
			    dev->curr_addr[id] = offset;
			}
			break;

		case STO_LSEEK_CUR:
			new_addr = dev->curr_addr[id] + offset;
			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
			    dev->curr_addr[id] = 0;
			}
			/* Great than totol size, seek to end */
			else if (new_addr >= dev->totol_size)
			{
			    dev->curr_addr[id] = dev->totol_size - 1;
			}
			/* Common seek */
			else
			{
			    dev->curr_addr[id] = new_addr;
			}
			break;

		case STO_LSEEK_END:
			new_addr = dev->totol_size + offset - 1;
			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
			    dev->curr_addr[id] = 0;
			}
			/* Common seek */
			else if (offset <= 0)
			{
			    dev->curr_addr[id] = new_addr;
			}
			break;
	}

	return dev->curr_addr[id];
}

/*
 * 	Name		:   sto_ram_write()
 *	Description	:   Write data into ram
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT8 *data					: Data to be write
 *					UINT32 len					: Data length
 *	Return		:	INT32 						: Write data length
 *
 */
static INT32 sto_ram_write(struct sto_device *dev, UINT8 *data, INT32 len)
{
    INT32 ret;
	int id;

    if (!data)
    {
        return -1;
    }

    if ((dev->curr_addr[id] + len) <= dev->totol_size)
    {
        ret = len;
    }
    else
    {
        ret = dev->totol_size - dev->curr_addr[id];
    }

	id = 0;
	if(id>=STO_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		ASSERT(0);
	}

    sto_mutex_enter_ex();
    MEMCPY(dev->base_addr + dev->curr_addr[id], data, ret);
	dev->curr_addr[id] += ret;
    sto_mutex_exit_ex();

	return ret;
}

/*
 * 	Name		:   sto_ram_put_data()
 *	Description	:   Write data into storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT32 offset				: Offset
 *					UINT8 *data					: Data to be write
 *					UINT32 len					: Data length
 *	Return		:	INT32 						: Write data length
 *
 */
static INT32 sto_ram_put_data(struct sto_device *dev, UINT32 offset, UINT8 *data, INT32 len)
{
    INT32 ret;

    if (!data)
    {
        return -1;
    }

    if ((offset + len) <= dev->totol_size)
    {
        ret = len;
    }
    else
    {
        ret = dev->totol_size - offset;
    }

    sto_mutex_enter_ex();
    MEMCPY(dev->base_addr + offset, data, ret);
    sto_mutex_exit_ex();

	return ret;
}

/*
 * 	Name		:   sto_ram_get_data()
 *	Description	:   Read data into storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT8 *data					: Data to be read
 *					UINT32 offset				: Offset
 *					UINT32 len					: Data length
 *	Return		:	INT32 						: Read data length
 *
 */
static INT32 sto_ram_get_data(struct sto_device *dev, UINT8 *data, UINT32 offset, INT32 len)
{
    INT32 ret;

    if (!data)
    {
        return -1;
    }

    if ((offset + len) <= dev->totol_size)
    {
        ret = len;
    }
    else
    {
        ret = dev->totol_size - offset;
    }

    sto_mutex_enter_ex();
    MEMCPY(data, dev->base_addr + offset, ret);
    sto_mutex_exit_ex();

	return ret;
}

/*
 * Read "len" number of data from current address point.
 */
static INT32 sto_ram_read(struct sto_device *dev, UINT8 *data, INT32 len)
{
    INT32 ret;
	int id;

    if (!data)
    {
        return -1;
    }

    if ((dev->curr_addr[id] + len) <= dev->totol_size)
    {
        ret = len;
    }
    else
    {
        ret = dev->totol_size - dev->curr_addr[id];
    }

	id = 0;
	if(id>=STO_TASK_SUPPORT_NUM_MAX)
	{
		//Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
		ASSERT(0);
	}

    sto_mutex_enter_ex();
    MEMCPY(data, dev->base_addr + dev->curr_addr[id], ret);
	dev->curr_addr[id] += ret;
    sto_mutex_exit_ex();

	return ret;
}


INT32 sto_local_ram_attach()
{
	struct sto_device *dev;
	struct flash_private *tp;
	unsigned int ret;


    libc_printf("[%s, %d], sto_local_sflash_attach\n", __FUNCTION__, __LINE__);
    dev = dev_alloc(sto_ram_local_name, HLD_DEV_TYPE_STO, sizeof(struct sto_device));
	if (dev == NULL)
	{
		libc_printf("Error: Alloc storage device error!\n");
		return ERR_NO_MEM;
	}


	/* Current operate address */
	MEMSET(dev->curr_addr, 0, STO_TASK_SUPPORT_NUM_MAX * sizeof(UINT32));
	dev->base_addr = RAM_BASE_ADDR;   /* Flash base address */
	dev->totol_size = RAM_LEN;

	/* Function point init */
	sto_ram_fp_init(dev, sto_local_ram_attach);

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS) {
		libc_printf("Error: Register Flash storage device error!\n");
		dev_free(dev);
		return ERR_NO_DEV;
	}

	return SUCCESS;
}

/**************************************************************
 * Function:
 * 	sto_ram_fp_init()
 * Description:
 * 	init common functions for device.
 * Inpute:
 *	None.
 * Return Values:
 *	None.
 ***************************************************************/
void sto_ram_fp_init(struct sto_device *dev,INT32 (*init)())
{
	dev->init 	= init;
	dev->open 	= sto_ram_open;
	dev->close 	= sto_ram_close;
	dev->do_ioctl 	= sto_ram_ioctl;
	dev->lseek 	= sto_ram_lseek;
	dev->write 	= sto_ram_write;
	dev->put_data	= sto_ram_put_data;
	dev->get_data	= sto_ram_get_data;
	dev->read 	= sto_ram_read;
}


