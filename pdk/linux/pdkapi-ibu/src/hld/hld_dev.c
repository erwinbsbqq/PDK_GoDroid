/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    hld_dev.c
*
*    Description:    This file contains all functions that implement
*		             HLD device management.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jan.26.2003      Justin Wu      Ver 0.1    Create file.
*	2.  Jan.17.2006      Justin Wu      Ver 0.2    Add get_dev_by_id().
*****************************************************************************/

#include <types.h>
#include <retcode.h>
//corei#include <api/libc/printf.h>
//#include <api/libc/string.h>
//#include <api/libc/alloc.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>

static struct hld_device *hld_device_base = NULL;

/*
 * 	Name		:   dev_get_by_name()
 *	Description	:   Get a device from device link list by device name.
 *	Parameter	:   INT8 *name					: Device name
 *	Return		:	void *						: Device founded
 *
 */
void *dev_get_by_name(INT8 *name)
{
	struct hld_device *dev;

	for (dev = hld_device_base; dev != NULL; dev = dev->next)
	{
		/* Find the device */
		if (strcmp(dev->name, name) == 0)
		{
			return dev;
		}
	}

	return NULL;
}

/*
 * 	Name		:   dev_get_by_type()
 *	Description	:   Get a device from device link list by device type.
 *	Parameter	:   INT32 type					: Device type
 *					void *sdev					: Start search nod
 *	Return		:	void *						: Device founded
 *
 */
void *dev_get_by_type(void *sdev, UINT32 type)
{
	struct hld_device *dev;

	dev = (sdev == NULL) ? hld_device_base : sdev;

	for (; dev != NULL; dev = dev->next)
	{
		/* Find the device */
		if ((dev->type & HLD_DEV_TYPE_MASK) == type)
		{
			return dev;
		}
	}

	return NULL;
}

/*
 * 	Name		:   dev_get_by_id()
 *	Description	:   Get a device from device link list by device ID.
 *	Parameter	:   UINT32 type					: Device type
 *					UINT16 id					: Device id
 *	Return		:	void *						: Device founded
 *
 */
void *dev_get_by_id(UINT32 type, UINT16 id)
{
	struct hld_device *dev;

	for (dev = hld_device_base; dev != NULL; dev = dev->next)
	{
		/* Find the device */
		if (dev->type == (type | id))
		{
			return dev;
		}
	}

	return NULL;
}

/*
 * 	Name		:   hld_dev_add()
 *	Description	:   Add a device to device link list tail.
 *	Parameter	:	struct hld_device *dev		: Device need queue to tail
 *	Return		:	INT32						: Result value
 *
 */
static INT32 hld_dev_add(struct hld_device *dev)
{
	struct hld_device *dp;
	UINT32 count;

	/* Is a null queue */
	if (hld_device_base == NULL)
	{
		hld_device_base = dev;
		dev->next = NULL;
		return SUCCESS;
	}

	if (dev_get_by_name(dev->name) != NULL)
	{
		PRINTF("hld_dev_add error: device %s same name!\n", dev->name);
		return ERR_DEV_CLASH;
	}

	/* Add this device to device list */
	/* Move to tail */
	for (dp = hld_device_base, count = 0; dp->next != NULL; dp = dp->next)
	{
		count++;
	}

	if (count >= HLD_MAX_DEV_NUMBER)
	{
		return ERR_QUEUE_FULL;
	}

	dp->next = dev;
	dev->next = NULL;

	return SUCCESS;
}

/*
 * 	Name		:   hld_dev_remove()
 *	Description	:   Remove a device from device link list.
 *	Parameter	:	struct hld_device *dev		: Device to be remove
 *	Return		:	INT32						: Result value
 *
 */
static INT32 hld_dev_remove(struct hld_device *dev)
{
	struct hld_device *dp;

	/* If dev in dev_queue, delete it from queue, else free it directly */
	if (hld_device_base != NULL)
	{
		if (strcmp(hld_device_base->name, dev->name) == 0)
		{
			hld_device_base = dev->next;
		} else
		{
			for (dp = hld_device_base; dp->next != NULL; dp = dp->next)
			{
				if (strcmp(dp->next->name, dev->name) == 0)
				{
					dp->next = dev->next;
					break;
				}
			}
		}
	}

	return SUCCESS;
}

/*
 * 	Name		:   dev_alloc()
 *	Description	:   Alloc and add a HLD device to device link list.
 *	Parameter	:   INT8 *name					: Device's name
 *					UINT32 size					: Device structure size in bytes
 *	Return		:	void*				 		: Device need be added
 *
 */

void *dev_alloc(INT8 *name, UINT32 type, UINT32 size)
{
	struct hld_device *dev = (struct hld_device *)malloc(size);
	struct hld_device *dp;
	UINT16 id;

	if (dev == NULL)
	{
		PRINTF("hld_dev_alloc: error - device %s not enough memory: %08x!\n",
			  name, size);
		return NULL;
	}

	/* Name same with some one in HLD device list, error */
	for (id = 0, dp = hld_device_base; dp != NULL; dp = dp->next)
	{
		/* Find the device */
		if (strcmp(dp->name, name) == 0)
		{
			PRINTF("hld_dev_alloc: error - device %s same name!\n", name);
			free(dev);
			return NULL;
		}
		/* Check ID */
		if ((dp->type & HLD_DEV_TYPE_MASK) == type)
		{
			id++;
		}
	}

	/* Clear device structure */
	memset((UINT8 *)dev, 0, size);

	dev->type = (type | id);
	strcpy(dev->name, name);

	return dev;
}

/*
 * 	Name		:   dev_register()
 *	Description	:   Register a HLD device to system.
 *	Parameter	:	void *dev					: Device need be register
 *	Return		:   INT8						: Return value
 *
 */
INT32 dev_register(void *dev)
{
	return hld_dev_add((struct hld_device *)dev);
}

/*
 * 	Name		:   dev_free()
 *	Description	:   Free a HLD device from device link list.
 *	Parameter	:	void *dev					: Device to be free
 *	Return		:
 *
 */
void dev_free(void *dev)
{
	if (dev != NULL)
	{
		hld_dev_remove((struct hld_device *)dev);
		free(dev);
	}

	return;
}

/*
 * 	Name		:   dev_list_all(void *sdev)
 *	Description	:   Print out all device from device link list.
 *	Parameter	:	void *sdev					: Start search nod
 *	Return		:
 *
 */
void dev_list_all(void *sdev)
{
	struct hld_device *dev;

	dev = (sdev == NULL) ? hld_device_base : sdev;

	for (dev = hld_device_base; dev != NULL; dev = dev->next)
	{
		PRINTF("Name: %16s; Type: 0x%08x; Address: 0x%08x\n",
		  dev->name, dev->type, dev);
	}

	return;
}
