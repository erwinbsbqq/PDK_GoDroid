/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2007 Copyright (C)
*
*    File:    scart.c
*    
*    Description:    This file contains all functions definition 
*		             of SCART SWITCH  modulator driver.
*    History: 
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Aug.21.2007      Goliath Peng       Ver 0.1    Create file.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
//#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
#include <osal/osal_int.h>
#include <hld/hld_dev.h>
#include <hld/scart/scart.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
//corei#include <err/errno.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>

static char scart_mx9671_name[HLD_MAX_NAME_SIZE]= "SCART_MX9671_0";
static INT32 scart_detach(struct scart_device *dev);

INT32 scart_open(struct scart_device *dev)
{
	INT32 result = SUCCESS;
	int scart_handle = 0;
	
	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		PRINTF("scart_open: warning - device %s openned already!\n", dev->name);
		return SUCCESS;
	}
	
	/* Open this device */
	scart_handle = open("/dev/ali_scart", O_RDWR);
	if(scart_handle == 0)
	{
		PRINTF("open Scart handle fail\n");
		return RET_FAILURE;
	}
	else
	{
			dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
			dev->priv=(void *)scart_handle;
			PRINTF("open Scart handle success\n");
	}
	
	return result;
}


INT32 scart_close(struct scart_device *dev)
{
	int scart_handle = 0;

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		PRINTF("sdec_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}
	
	scart_handle = (int)dev->priv;
	close(scart_handle);
	dev->priv = (int)0;

	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	
	return SUCCESS;
}


INT32 scart_io_control(struct scart_device *dev, INT32 cmd, UINT32 param)
{	
	INT32 result=SUCCESS;
	int scart_handle = 0;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return !SUCCESS;
	}
	
	scart_handle = (int)dev->priv;
	if(SCART_DETACH_DEV == cmd)
		scart_detach(dev);
	else
		result = ioctl(scart_handle, cmd, (void *)param);

	return result;
}

INT32 scart_attach(void)
{	
	struct scart_device *dev;

	dev = dev_alloc(scart_mx9671_name, HLD_DEV_TYPE_SCART, sizeof(struct scart_device));
	if (dev == NULL)
	{
		PRINTF("Error: Alloc SCART switch device ERROR!\n");
		return ERR_NO_MEM;
	}
	
	/* Add this device to queue */
	if(dev_register(dev) != SUCCESS)
	{
		PRINTF("Error: Register RF modulator device ERROR!\n");
	}
	dev->flags = 0;
	
	return SUCCESS;
}

static INT32 scart_detach(struct scart_device *dev)
{  
    if(dev != NULL)
    {
		//It is not necessary to free the space of dev->priv since it is now the handler of this device
		//which means it is not a real pointer
        dev_free(dev);
        dev = NULL;
        return SUCCESS;
    }
    else
        return  ERR_DEV_ERROR;
}

