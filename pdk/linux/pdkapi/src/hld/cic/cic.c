/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    cic.c
*
*    Description:    This file contains all functions definition
*		             of CI controler driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Mar.08.2004      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#include <adr_basic_types.h>
#include <adr_retcode.h>
#include <osal/osal.h>
#include <hld/adr_hld_dev.h>

#include <hld/cic/adr_cic.h>

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


#if 1
#define CIC_PRINTF(...)	do{}while(0)
#else
#define CIC_PRINTF(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(CIC, fmt, ##args); \
			} while(0)

#endif


static char cic_m3602_name[HLD_MAX_NAME_SIZE] = "CIC_m3602_0";

static ca_msg_t cic_msg;
static ca_slot_info_t cic_slot_info;

/*
 * 	Name		:   cic_open()
 *	Description	:   Open a cic device
 *	Parameter	:	struct cic_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 cic_open(struct cic_device *dev, void (*callback)(int slot))
{
	//INT32 result = SUCCESS;
	int cic_hdl=0;

	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		CIC_PRINTF("cic_open: warning - device %s openned already!\n", dev->name);
		return SUCCESS;
	}

	/* Open this device */
	cic_hdl= open("/dev/ali_m36_cic", O_RDWR);
	if(cic_hdl < 0)
	{
		CIC_PRINTF("open CI controller handle fail\n");
		return !RET_SUCCESS;
	}
	else
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
		dev->priv=cic_hdl;
	}

	return SUCCESS;
}

/*
 * 	Name		:   cic_close()
 *	Description	:   Close a cic device
 *	Parameter	:	struct cic_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 cic_close(struct cic_device *dev)
{
	INT32 result =  SUCCESS;

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		CIC_PRINTF("cic_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}

	/* Close device */
	close((int)dev->priv);

	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	return result;
}

/*
 * 	Name		:   cic_read()
 *	Description	:   Read data from CI interface
 *	Parameter	:	struct cic_device *dev		: Device structuer
 *					UINT16 size					: Numer of read in data
 *					UINT8 *buffer				: Buffer pointer
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 cic_read(struct cic_device *dev, int slot, UINT16 size, UINT8 *buffer)
{
	INT32 ret;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	cic_msg.index=slot;
	cic_msg.type=CIC_BLOCK<<16;
	cic_msg.length=size;

	ret=ioctl((int)dev->priv, CA_GET_MSG, &cic_msg);
	memcpy(buffer, cic_msg.msg, size);

	return ret;
}

/*
 * 	Name		:   cic_write()
 *	Description	:   Write data to CI interface
 *	Parameter	:	struct cic_device *dev		: Device structuer
 *					UINT16 size					: Numer of write out data
 *					UINT8 *buffer				: Buffer pointer
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 cic_write(struct cic_device *dev, int slot, UINT16 size, UINT8 *buffer)
{
	INT32 ret;
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	cic_msg.index=slot;
	cic_msg.type=CIC_BLOCK<<16;
	cic_msg.length=size;
	memcpy(cic_msg.msg, buffer, size);

	ret=ioctl((int)dev->priv, CA_SEND_MSG, &cic_msg);

	return SUCCESS;
}

/*
 * 	Name		:   cic_io_control()
 *	Description	:   Panel IO control command
 *	Parameter	:	struct sto_device *dev		: Device
 *					INT32 cmd					: IO command
 *					UINT32 param				: Param
 *	Return		:	INT32 						: Result
 *
 */
INT32 cic_io_control(struct cic_device *dev, INT32 cmd, UINT32 param)
{
	struct cic_io_command_memrw *memrw;
	struct cic_io_command_iorw *iorw;
	struct cic_io_command_signal *signal;
	INT32 ret=ERR_FAILURE;
	/* If device not running, exit */
	if ( dev == NULL)
 	  return ERR_DEV_ERROR;
	if ((CIC_DRIVER_REQMUTEX!=cmd)&&((dev->flags & HLD_DEV_STATS_UP) == 0))
	{
		CIC_PRINTF("%s, error\n", __FUNCTION__);
		return ERR_DEV_ERROR;
	}

	switch(cmd)
	{
		case CIC_DRIVER_READMEM:	/* Read attribute memory command*/
			memrw=(struct cic_io_command_memrw *)param;
			cic_msg.index=memrw->slot;
			cic_msg.type=(CIC_MEMORY<<16)|(memrw->addr);
			cic_msg.length=memrw->size;
			ret=ioctl((int)dev->priv, CA_GET_MSG, &cic_msg);
			if(SUCCESS==ret)
			{
				memcpy(memrw->buffer, cic_msg.msg, cic_msg.length);
				CIC_PRINTF("cic CIC_DRIVER_READMEM,slot:%d, addr:%d.\n",memrw->slot,memrw->addr);
			}
			break;
		case CIC_DRIVER_WRITEMEM:	/* Write attribute memory command */
			memrw=(struct cic_io_command_memrw *)param;
			cic_msg.index=memrw->slot;
			cic_msg.type=(CIC_MEMORY<<16)|(memrw->addr);
			cic_msg.length=memrw->size;
			memcpy(cic_msg.msg, memrw->buffer, cic_msg.length);
			CIC_PRINTF("cic CIC_DRIVER_WRITEMEM,slot:%d, addr:%d.\n",memrw->slot,memrw->addr);
			ret=ioctl((int)dev->priv, CA_SEND_MSG, &cic_msg);
			break;
		case CIC_DRIVER_READIO:		/* Read byte from I/O command */
			iorw=(struct cic_io_command_iorw *)param;
			cic_msg.index=iorw->slot;
			cic_msg.type=iorw->reg<<16;
			cic_msg.length=1;
			ret=ioctl((int)dev->priv, CA_GET_MSG, &cic_msg);
			if(SUCCESS==ret)
			{
				//memcpy(iorw->buffer, cic_msg.msg, cic_msg.length);
				iorw->buffer[0]=cic_msg.msg[0];
				CIC_PRINTF("cic CIC_DRIVER_READIO,slot:%d, reg:%d, data:0x%02x.\n",iorw->slot,iorw->reg, iorw->buffer[0]);
			}
			break;
			break;
		case CIC_DRIVER_WRITEIO:	/* Write byte to I/O command */
			iorw=(struct cic_io_command_iorw *)param;
			cic_msg.index=iorw->slot;
			cic_msg.type=iorw->reg<<16;
			cic_msg.length=1;
			cic_msg.msg[0]=iorw->buffer[0];
			CIC_PRINTF("cic CIC_DRIVER_WRITEIO,slot:%d, reg:%d, data:0x%02x.\n",iorw->slot,iorw->reg, iorw->buffer[0]);
			ret=ioctl((int)dev->priv, CA_SEND_MSG, &cic_msg);
			break;
		case CIC_DRIVER_TSIGNAL:	/* Test a hardware PC card signal */
			signal=(struct cic_io_command_signal *)param;
			cic_slot_info.num=signal->slot;
			cic_slot_info.type=signal->signal;
			ret=ioctl((int)dev->priv, CA_GET_SLOT_INFO, &cic_slot_info);
			if(SUCCESS==ret)
			{
				signal->status=cic_slot_info.flags;
				//CIC_PRINTF("cic CIC_DRIVER_TSIGNAL, %d, status:0x%02x\n", signal->signal, signal->status);
			}
			break;
		case CIC_DRIVER_SSIGNAL:	/* Set or clear a PC card pin */
			signal=(struct cic_io_command_signal *)param;
			cic_slot_info.num=signal->slot;
			cic_slot_info.type=signal->signal;
			cic_slot_info.flags=signal->status;
			CIC_PRINTF("cic CIC_DRIVER_SSIGNAL,slot:%d, signal:%d, status:%d.\n",signal->slot,signal->signal, signal->status);
			ret=ioctl((int)dev->priv, CA_SET_SLOT_INFO, &cic_slot_info);
			break;
		default:
			return ERR_FAILURE;
			break;
	}

	return ret;
}

INT32 cic_m3602_attach(void)
{
	struct cic_device *dev;

	dev = dev_alloc(cic_m3602_name, HLD_DEV_TYPE_CIC,
		 			sizeof(struct cic_device));
	if (dev == NULL)
	{
//		CIC_PRINTF("Error: Alloc CI controler device error!\n");
		return ERR_NO_MEM;
	}
	//cic_m3602_name[STRLEN(cic_m3602_name) - 1]++;

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
//		CIC_PRINTF("Error: Register CI controler device error!\n");
		dev_free(dev);
		return ERR_FAILURE;
	}
	memset(&cic_msg, 0, sizeof(ca_msg_t));
	memset(&cic_slot_info, 0, sizeof(ca_slot_info_t));
	
	return SUCCESS;
}

INT32 cic_m3602_dettach(struct cic_device *dev)
{
	if(dev->flags & (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING))
	{
		CIC_PRINTF("Error, CI controller device still running.\n");
		return !SUCCESS;
	}
	dev_free(dev);
	return SUCCESS;
}
