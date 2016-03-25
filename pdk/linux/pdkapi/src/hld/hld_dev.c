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

#include <adr_retcode.h>
#include <hld_cfg.h>
#include <hld/adr_hld_dev.h>
#include <osal/osal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hld/misc/adr_ipc.h>

#if 0
#define PRINTF(fmt, args...)                  ADR_DBG_PRINT(DEV, fmt, ##args)
#else
#define PRINTF(...)             do{}while(0)
#endif


#ifdef ADR_IPC_ENABLE
#define IPC_HLD_SEM_BASE	0
#define IPC_HLD_DEV_BASE	0

#define MUTEX_LOCK()	adr_ipc_semlock(m_hld_sem_id)
#define MUTEX_UNLOCK()	adr_ipc_semunlock(m_hld_sem_id)

static int m_hld_sem_id = 0;
static int g_ali_hld_sem_id = 0;
#else
#define MUTEX_LOCK()	do{}while(0)
#define MUTEX_UNLOCK()	do{}while(0)
#endif
static struct hld_device *hld_device_base = NULL;
static int m_hld_init = 0;
#ifdef ADR_IPC_ENABLE
void adr_hld_global_semlock()
{
	g_ali_hld_sem_id = adr_ipc_semget(ADR_IPC_HLD, IPC_HLD_SEM_BASE+1, 1);
	if(g_ali_hld_sem_id < 0)
	{
		PRINTF("################## %s : %d sem fail %d\n", __FUNCTION__
			,__LINE__, g_ali_hld_sem_id);
	
		return -1;
	}
	adr_ipc_semlock(g_ali_hld_sem_id);
}

void adr_hld_global_semunlock()
{
	g_ali_hld_sem_id = adr_ipc_semget(ADR_IPC_HLD, IPC_HLD_SEM_BASE+1, 1);
	if(g_ali_hld_sem_id < 0)
	{
		PRINTF("################## %s : %d sem fail %d\n", __FUNCTION__
			,__LINE__, g_ali_hld_sem_id);
	
		return -1;
	}

	adr_ipc_semunlock(g_ali_hld_sem_id);
}

#endif

static int hld_init(void)
{
		adr_dbg_init();
	
#ifdef ADR_IPC_ENABLE
		if(hld_device_base == NULL)
		{
			int shmid = -1;
	
			m_hld_sem_id = adr_ipc_semget(ADR_IPC_HLD, IPC_HLD_SEM_BASE, 1);
			if(m_hld_sem_id < 0)
			{
				PRINTF("################## %s : %d sem fail %d\n", __FUNCTION__
					,__LINE__, m_hld_sem_id);
	
				return -1;
			}
	
			shmid = adr_ipc_shmget(ADR_IPC_HLD, IPC_HLD_DEV_BASE
				, (void **)&hld_device_base, sizeof(*hld_device_base));
			if(shmid < -1)
			{
				PRINTF("################## %s : %d shm fail %d\n", __FUNCTION__
					,__LINE__, shmid);
	
				return -1;
			}		
		}

		if (hld_device_base == NULL)
		{
			return -1;
		}
		
		MUTEX_LOCK();	
		if(hld_device_base->next == NULL)
		{
			memset((void *)hld_device_base, 0, sizeof(*hld_device_base));
			
			strcpy((void *)hld_device_base->name, "hld_base");
			hld_device_base->next = (struct hld_device *)(IPC_HLD_DEV_BASE<<16);
		}

		MUTEX_UNLOCK();

		PRINTF("hld init device base %x\n", (int)hld_device_base);
		
	
#endif
	
		return 1;
	}

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

	PRINTF("%s : line %d name %s\n", __FUNCTION__, __LINE__, name);
	
#ifdef ADR_IPC_ENABLE
	unsigned int keyid = 0;
	int shmid = -1;
	
	if (!m_hld_init)
	{
		if(hld_init() < 0)
			return NULL;

		m_hld_init = 1;	
	}
#endif

	MUTEX_LOCK();

#ifdef ADR_IPC_ENABLE
	keyid = ((unsigned int)hld_device_base->next & 0xFFFF);
	for(;keyid > 0; keyid = ((unsigned int)dev->next & 0xFFFF))
	{
		shmid = adr_ipc_shmget(ADR_IPC_HLD, (int)keyid, (void *)&dev, 0);	
		if(shmid < 0)
		{
			MUTEX_UNLOCK();
			
			PRINTF("hld key id error %d\n", keyid);
			return NULL;
		}
		/* Find the device */
		if (strcmp(dev->name, name) == 0)
		{
			MUTEX_UNLOCK();
			
			return dev;
		}
	}		
#else
	for (dev = hld_device_base; dev != NULL; dev = dev->next)
	{
		/* Find the device */
		if (strcmp(dev->name, name) == 0)
		{
			MUTEX_UNLOCK();
			
			return dev;
		}
	}	
#endif	

	MUTEX_UNLOCK();
	
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

	PRINTF("%s : line %d type %d\n", __FUNCTION__, __LINE__, type);
	
#ifdef ADR_IPC_ENABLE
	unsigned int keyid = 0;
	int shmid = -1;
	
	if (!m_hld_init)
	{
		if(hld_init() < 0)
			return NULL;

		m_hld_init = 1;	
	}
#endif

	MUTEX_LOCK();

#ifdef ADR_IPC_ENABLE
	dev = (sdev == NULL) ? hld_device_base : sdev;
	keyid = ((unsigned int)dev->next & 0xFFFF);
	for(;keyid > 0; keyid = ((unsigned int)dev->next & 0xFFFF))
	{
		shmid = adr_ipc_shmget(ADR_IPC_HLD, (int)keyid, (void *)&dev, 0);	
		if(shmid < 0)
		{
			MUTEX_UNLOCK();
			
			PRINTF("key id error %d\n", keyid);
			return NULL;
		}
		/* Find the device */
		if ((dev->type & HLD_DEV_TYPE_MASK) == type)
		{
			MUTEX_UNLOCK();
			
			return dev;
		}
	}		
#else
	dev = (sdev == NULL) ? hld_device_base : sdev;

	for (; dev != NULL; dev = dev->next)
	{
		/* Find the device */
		if ((dev->type & HLD_DEV_TYPE_MASK) == type)
		{
			MUTEX_UNLOCK();
			
			return dev;
		}
	}	
#endif	

	MUTEX_UNLOCK();
	
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


	PRINTF("%s : line %d type %d id %d\n", __FUNCTION__, __LINE__, type, id);
	
#ifdef ADR_IPC_ENABLE
	unsigned int keyid = 0;
	int shmid = -1;
	
	if (!m_hld_init)
	{
		if(hld_init() < 0)
			return NULL;

		m_hld_init = 1;	
	}
#endif

	MUTEX_LOCK();

#ifdef ADR_IPC_ENABLE
	keyid = ((unsigned int)hld_device_base->next & 0xFFFF);
	for(;keyid > 0; keyid = ((unsigned int)dev->next & 0xFFFF))
	{
		shmid = adr_ipc_shmget(ADR_IPC_HLD, (int)keyid, (void *)&dev, 0);
		if(shmid < 0)
		{
			MUTEX_UNLOCK();
			
			PRINTF("key id error %d\n", keyid);
			return NULL;
		}
		/* Find the device */
		if (dev->type == (type | id))
		{
			MUTEX_UNLOCK();
			
			return dev;
		}
	}		
#else
	for (dev = hld_device_base; dev != NULL; dev = dev->next)
	{
		/* Find the device */
		if (dev->type == (type | id))
		{
			MUTEX_UNLOCK();
			
			return dev;
		}
	}	
#endif

	MUTEX_UNLOCK();

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

	PRINTF("%s : line %d\n", __FUNCTION__, __LINE__);
	if (NULL == dev)
	{
		PRINTF("%s : line %d\n", __FUNCTION__, __LINE__);
		return ERR_FAILURE;
	}
	
#ifdef ADR_IPC_ENABLE
	unsigned int keyid = 0;
	int shmid = -1;
#else
	/* Is a null queue */
	if (hld_device_base == NULL)
	{
		hld_device_base = dev;
		dev->next = NULL;
		return SUCCESS;
	}
#endif

	if (dev_get_by_name(dev->name) != NULL)
	{
		PRINTF("hld_dev_add error: device %s same name!\n", dev->name);

#ifdef ADR_IPC_ENABLE
		return SUCCESS;
#else
		return ERR_DEV_CLASH;
#endif
	}

	/* Add this device to device list */
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();

	keyid = ((unsigned int)hld_device_base->next & 0xFFFF);
	for(dp = hld_device_base;keyid > 0; keyid = ((unsigned int)dp->next & 0xFFFF))
	{
		shmid = adr_ipc_shmget(ADR_IPC_HLD, (int)keyid, (void *)&dp, 0);	
		if(shmid < 0)
		{
			MUTEX_UNLOCK();
			
			PRINTF("hld key id error %d\n", keyid);
			return ERR_FAILURE;
		}		
	}
	count = (UINT32)dp->next & 0xFFFF0000;
	count |= (((UINT32)dev->next)>>16);
	dp->next = (struct hld_device *)count;
	
	PRINTF("%s : %d cur key %d last %d\n", __FUNCTION__, __LINE__, (UINT32)dev->next>>16, (count>>16));
	
	MUTEX_UNLOCK();
#else	
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
#endif

	return SUCCESS;
}

/*
 * 	Name		:   hld_dev_remove()
 *	Description	:   Remove a device from device link list.
 *	Parameter	:	struct hld_device *dev		: Device to be remove
 *	Return		:	INT32						: Result value
 *
 */
 #ifdef ADR_IPC_ENABLE
 #else
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
 #endif

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
	struct hld_device *dev = NULL;
	struct hld_device *dp;
	UINT16 id;

	PRINTF("%s : line %d type %x size %d\n", __FUNCTION__, __LINE__
		, type, size);
	
#ifdef ADR_IPC_ENABLE
	unsigned int keyid = 0;
	int shmid = -1;
	
	if (!m_hld_init)
	{
		if(hld_init() < 0)
			return NULL;

		m_hld_init = 1;	
	}
#else
	dev = (struct hld_device *)malloc(size);
	if (dev == NULL)
	{
		PRINTF("hld_dev_alloc: error - device %s not enough memory: %08x!\n",
			  name, size);
		return NULL;
	}
#endif

	MUTEX_LOCK();

#ifdef ADR_IPC_ENABLE
	keyid = ((unsigned int)hld_device_base->next & 0xFFFF);
	for(id = 0,dp = hld_device_base;keyid > 0; keyid = ((unsigned int)dp->next & 0xFFFF))
	{
		shmid = adr_ipc_shmget(ADR_IPC_HLD, (int)keyid, (void *)&dp, size);	
		if(shmid < 0)
		{
			MUTEX_UNLOCK();
			
			PRINTF("%s : line %d key id error %d\n", __FUNCTION__, __LINE__, keyid);
			return NULL;
		}
		
		/* Find the device */
		if (strcmp(dp->name, name) == 0)
		{
			MUTEX_UNLOCK();
					
			PRINTF("hld_dev_alloc: error - device %s same name!\n", name);
			return dp;		
		}		

		/* Check ID */
		if ((dp->type & HLD_DEV_TYPE_MASK) == type)
		{
			id++;
		}
	}

	keyid = (((UINT32)dp->next)>>16) + 1;
	shmid = adr_ipc_shmget(ADR_IPC_HLD, (int)keyid, (void *)&dev, size);	
	if(shmid < 0)
	{
		MUTEX_UNLOCK();
		
		PRINTF("%s : line %d key id error %d\n", __FUNCTION__, __LINE__, keyid);
		return NULL;
	}	

	/* Clear device structure */
	memset((UINT8 *)dev, 0, size);
	
	dev->next = (struct hld_device *)(keyid<<16);
#else
	/* Name same with some one in HLD device list, error */
	for (id = 0, dp = hld_device_base; dp != NULL; dp = dp->next)
	{
		/* Find the device */
		if (strcmp(dp->name, name) == 0)
		{
			MUTEX_UNLOCK();	
			
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
#endif

	dev->type = (type | id);
	strcpy(dev->name, name);

	MUTEX_UNLOCK();

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
	if (!m_hld_init)
	{
		if(hld_init() < 0)
			return ERR_FAILURE;

		m_hld_init = 1;	
	}


	PRINTF("%s : line %d\n", __FUNCTION__, __LINE__);
	
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
#ifdef ADR_IPC_ENABLE

#else
	if (dev != NULL)
	{
		hld_dev_remove((struct hld_device *)dev);
		free(dev);
	}
#endif

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

#ifdef ADR_IPC_ENABLE
	unsigned int keyid = 0;
	int shmid = -1;
	
	if (!m_hld_init)
	{
		if(hld_init() < 0)
			return;

		m_hld_init = 1;	
	}
#endif

	MUTEX_LOCK();

#ifdef ADR_IPC_ENABLE
	dev = (sdev == NULL) ? hld_device_base : sdev;
	keyid = ((unsigned int)dev->next & 0xFFFF);
	for(;keyid > 0; keyid = ((unsigned int)dev->next & 0xFFFF))
	{
		shmid = adr_ipc_shmget(ADR_IPC_HLD, (int)keyid, (void *)&dev, 0);	
		if(shmid < 0)
		{
			MUTEX_UNLOCK();
			
			PRINTF("key id error %d\n", keyid);
			return;
		}
		
		PRINTF("Name: %16s; Type: 0x%08x; Address: 0x%08x\n",
		  dev->name, dev->type, dev);
	}
#else	
	dev = (sdev == NULL) ? hld_device_base : sdev;

	for (dev = hld_device_base; dev != NULL; dev = dev->next)
	{
		PRINTF("Name: %16s; Type: 0x%08x; Address: 0x%08x\n",
		  dev->name, dev->type, dev);
	}
#endif

	MUTEX_UNLOCK();

	return;
}

