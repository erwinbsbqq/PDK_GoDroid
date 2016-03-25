
/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *  (C)
 *  File: adr_ipc.c
 *  (I)
 *  Description: speic IPC module for adr hld library
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2012.11.24			Sam			Create
 ****************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <linux/sem.h>
#include <linux/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <hld_cfg.h>
#include <hld/misc/adr_ipc.h>

#ifdef ADR_IPC_ENABLE
#define IPC_DBG_ENABLE	0
#define IPC_PRF(fmt, args...)  \
			do { \
				if (IPC_DBG_ENABLE) \
				{ \
					ADR_DBG_PRINT(IPC, "%s %s L%d: "fmt"\n", __FILE__, \
						__FUNCTION__, __LINE__, ##args); \
				} \
			} while(0)

#define ENTRY()		IPC_PRF(" in\n")
#define EXIT()		IPC_PRF(" out\n")

#define IPC_PRIV_KEY			(ADR_IPC_KEY_BASE)
#define IPC_SEM_KEY				(ADR_IPC_KEY_BASE + 1)
#define IPC_DEV_KEY_BASE		(ADR_IPC_KEY_BASE + 10)

#define IPC_DEV_KEY_NUM		(ADR_IPC_DEV_SEM_MAX + ADR_IPC_DEV_SHM_MAX)

struct ipc_priv
{
	int shm_id;	
	int sem_id;

	int dbg_on;

	int dev_shm_id[ADR_IPC_MAX][ADR_IPC_DEV_SHM_MAX];
	int dev_sem_id[ADR_IPC_MAX][ADR_IPC_DEV_SEM_MAX];
};

static unsigned int m_dev_shm_buf[ADR_IPC_MAX][ADR_IPC_DEV_SHM_MAX];

static struct ipc_priv *p_ipc_priv = NULL;
static int *p_ipc_dbg_on = NULL;

static int sem_lock(int semid)
{
	struct sembuf buf;

	if(semid <= 0)
	{
		IPC_PRF("fail\n");
		return -1;
	}

	buf.sem_num = 0;
	buf.sem_op = -1; 
	buf.sem_flg = SEM_UNDO;

	if(semop(semid, &buf, 1) < 0)
	{
		IPC_PRF("fail\n");
		return -1;
	}

	return 1;
}

static int sem_unlock(int semid)
{
	struct sembuf buf;

	if(semid <= 0)
	{
		IPC_PRF("fail\n");
		return -1;
	}
	
	buf.sem_num = 0;
	buf.sem_op = 1; 
	buf.sem_flg = SEM_UNDO;

	if(semop(semid, &buf, 1) < 0)
	{
		IPC_PRF("fail\n");
		return -1;
	}

	return 1;
}

static int init_ipc(void)
{
	int shmid = -1;
	int semid = -1;
	
	shmid = shmget(IPC_PRIV_KEY, sizeof(struct ipc_priv), 0);
	if(shmid < 0)
	{
		union semun option;

		semid = semget(IPC_SEM_KEY,1, IPC_CREAT | IPC_EXCL | 0x777);
		if(semid < 0)
		{
			IPC_PRF("create ipc sem fail\n");
			return -1;
		}

		option.val = 1;
		semctl(semid, 0, SETVAL, option);

		sem_lock(semid);

		shmid = shmget(IPC_PRIV_KEY, sizeof(struct ipc_priv), IPC_CREAT | IPC_EXCL | 0x777);
		if(shmid < 0)
		{			
			sem_unlock(semid);

			IPC_PRF("create ipc shm fail\n");			
			return -1;
		}

		p_ipc_priv = (struct ipc_priv *)shmat(shmid, NULL, 0);
		if((int)p_ipc_priv == -1)
		{
			sem_unlock(semid);
			
			IPC_PRF("get initied ipc fail\n");
			return -1;
		}	

		memset((void *)p_ipc_priv, 0, sizeof(*p_ipc_priv));
		p_ipc_priv->shm_id = shmid;
		p_ipc_priv->sem_id = semid;	

		p_ipc_priv->dbg_on = 0;// just for debug

		// IPC_PRF("init ipc shm id %d sem id %d", shmid, semid);
	}
	else
	{
		p_ipc_priv = (struct ipc_priv *)shmat(shmid, NULL, 0);
		if((int)p_ipc_priv == -1)
		{
			IPC_PRF("get initied ipc fail\n");
			return -1;
		}	

		semid = p_ipc_priv->sem_id;

		sem_lock(semid);			
	}

	memset((void *)m_dev_shm_buf, 0, sizeof(m_dev_shm_buf));
	
	p_ipc_dbg_on = &(p_ipc_priv->dbg_on);		
	
	sem_unlock(semid);

	IPC_PRF("done sem id %d shm id %d\n", semid ,shmid);	
	return 1;
}

/* adr ipc semphore operation */
int adr_ipc_semget(enum adr_ipc_dev dev, int idx, int init_val)
{
	int semid = 0;

	if((dev > ADR_IPC_MAX)
		|| (idx > ADR_IPC_DEV_SEM_MAX))
	{
	
		IPC_PRF("%s : line %d par fail dev %d idx %d\n", __FUNCTION__
			, __LINE__, dev, idx);
		return -1;
	}
	
	ENTRY();
	if(p_ipc_priv == NULL)
	{
		if(init_ipc() < 0)
		{
			IPC_PRF("init ipc fail\n");
			goto FAIL;
		}
			
	}
	
	sem_lock(p_ipc_priv->sem_id);

	semid = p_ipc_priv->dev_sem_id[dev][idx];
	if(semid == 0)
	{
		union semun option;
		unsigned int key;

		key = IPC_DEV_KEY_BASE + (dev * IPC_DEV_KEY_NUM) + idx;
		semid = semget(key,1, IPC_CREAT | IPC_EXCL | 0x777);
		if(semid < 0)
		{
			sem_unlock(p_ipc_priv->sem_id);
			
			IPC_PRF("create sem fail\n");
			goto FAIL;
		}

		option.val = init_val;
		semctl(semid, 0, SETVAL, option);

		p_ipc_priv->dev_sem_id[dev][idx] = semid;
		IPC_PRF("create dev sem %d dev %d idx %d\n", semid, dev, idx);
	}
	
	sem_unlock(p_ipc_priv->sem_id);
	
	IPC_PRF("done dev semid %d\n", semid);
	EXIT();
	return semid;

FAIL:
	IPC_PRF("fail\n");
	EXIT();
	return -1;
}

int adr_ipc_semlock(int sem_id)
{	
	int ret = 0;
	
	ENTRY();
	
	ret = sem_lock(sem_id);


	EXIT();
	return ret;
}

int adr_ipc_semunlock(int sem_id)
{
	int ret = 0;
	
	ENTRY();
	
	ret = sem_unlock(sem_id);


	EXIT();
	return ret;
}

/* adr ipc share memory operation */
int adr_ipc_shmget(enum adr_ipc_dev dev, int idx, void **buffer, int size)
{
	int shmid = 0;
	void *shmbuf = NULL;
	int init = 0;

	if((dev > ADR_IPC_MAX)
		|| (idx > ADR_IPC_DEV_SHM_MAX))
	{
	
		IPC_PRF("%s : line %d par fail dev %d idx %d\n", __FUNCTION__
			, __LINE__, dev, idx);
		return -1;
	}
	
	ENTRY();
	if(p_ipc_priv == NULL)
	{
		if(init_ipc() < 0)
		{
			IPC_PRF("init ipc fail\n");
			goto FAIL;
		}
			
	}
	
	sem_lock(p_ipc_priv->sem_id);

	shmid = p_ipc_priv->dev_shm_id[dev][idx];
	
	if(shmid <= 0)
	{	
		if(size > 0)
		{
			unsigned int key = 0;

			key = IPC_DEV_KEY_BASE + (dev * IPC_DEV_KEY_NUM) + ADR_IPC_DEV_SEM_MAX + idx;
			shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0x777);
			if(shmid < 0)
			{		
				sem_unlock(p_ipc_priv->sem_id);
				
				IPC_PRF("create dev shm fail\n");			
				goto FAIL;
			}

			init = 1;
			
			IPC_PRF("init dev shm %d dev %d idx %d\n", shmid, dev, idx);
	
			p_ipc_priv->dev_shm_id[dev][idx] = shmid;			
		}
		else
		{
			sem_unlock(p_ipc_priv->sem_id);
			
			IPC_PRF("create dev shm fail size error %d\n", size);			
			goto FAIL;
		}
	}
	
	shmbuf = (void *)m_dev_shm_buf[dev][idx];

	if(shmbuf == NULL)
	{
		shmbuf = (void *)shmat(shmid, NULL, 0);
		if((int)shmbuf == -1)
		{
			sem_unlock(p_ipc_priv->sem_id);
			
			IPC_PRF("shmat dev buf fail\n");
			goto FAIL;

		}

		if(init)
			memset((void *)shmbuf, 0, size);

		m_dev_shm_buf[dev][idx] = (unsigned int)shmbuf;
	}
	
	*buffer = shmbuf;
		
	sem_unlock(p_ipc_priv->sem_id);
	
	IPC_PRF("done dev shmid %d\n", shmid);
	EXIT();
	return shmid;

FAIL:
	IPC_PRF("fail dev %d idx %d\n", dev, idx);
	EXIT();
	return -1;
}
#endif

