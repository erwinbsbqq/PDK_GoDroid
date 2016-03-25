#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>

#include "queue.h"
#include "timer.h"
#include "task.h"

#define _GNU_SOURCE

#define SEM_LIMIT_CNT 256

typedef struct sem_control_block
{
        ID      semid;          /* semaphore Identifier */
        UINT32     sematr;         /* attribute, OBJ_NONEXIST, or SEM_FIFO */
        sem_t psema_id;
} SEM_CB;
UINT32 total_sema = 0;
SEM_CB sema_st[SEM_LIMIT_CNT] = {{0, OBJ_NONEXIST, 0},};
pthread_mutex_t sem_mutex;

ID OS_CreateSemaphore(INT32 semcnt)
{
	SEM_CB   *semcb;
	ID semid;
	UINT32 i = 0;

	if(total_sema ==0)
	{
		total_sema = 1;
		for( i = 0; i < SEM_LIMIT_CNT; i++)
		{
			sema_st[i].sematr = OBJ_NONEXIST;
		}
		pthread_mutex_init(&sem_mutex,NULL);
	}

	//check para
	if(semcnt<0)
		return INVALID_ID;

	// search for unused entry
	for(semid=0;semid<SEM_LIMIT_CNT;semid++)
	{
		if (sema_st[semid].sematr==OBJ_NONEXIST)
		{
			break;
		}
	}
	if (semid>=SEM_LIMIT_CNT)
	{ // entry all used
		return INVALID_ID;
	}

	semcb = &sema_st[semid];

	semcb->sematr   =TA_TFIFO;// t_csem->sematr;
	semcb->semid = semid;
	/* pshared, 0, share between thread;not 0, share between process */
	sem_init(&semcb->psema_id, 1, semcnt);

	return (semid);
}

ER OS_DelSemaphore(ID semid)
{
	ER ret = E_OK;
	SEM_CB   *semcb;

	//if(semid<0)
		//return(E_FAILURE);
	if(semid>=SEM_LIMIT_CNT)
		return(E_FAILURE);

	semcb = &sema_st[semid];
	if(semcb->sematr == OBJ_NONEXIST)
	    return(E_FAILURE);

	semcb->sematr = OBJ_NONEXIST;
	sem_destroy(&semcb->psema_id);
	return ret;
}

ER OS_FreeSemaphore(ID semid)
{
	SEM_CB   *semcb;
	ER      ercd = E_OK;

	//if(semid<0) return(E_FAILURE);
	if(semid>=SEM_LIMIT_CNT) return(E_FAILURE);

	semcb = &sema_st[semid];
	if(semcb->sematr == OBJ_NONEXIST)
		return(E_FAILURE);

	sem_post(&semcb->psema_id);
	return(ercd);
}


#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
ER OS_AcquireSemaphoreTimeOut(ID semid, TMO tmout, char *func, int line)
#else
ER OS_AcquireSemaphoreTimeOut(ID semid, TMO tmout)
#endif /****APP_LOCK_DEBUG****/
{
	struct timespec ts;
	int sts;
	SEM_CB   *semcb;
	ER      ercd = E_OK;

	//if(semid<0)
		//return(E_FAILURE);
	if(semid>=SEM_LIMIT_CNT)
		return(E_FAILURE);

	semcb = &sema_st[semid];
	if(semcb->sematr == OBJ_NONEXIST)
		return(E_FAILURE);
#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
    lock_debug_in(lock_debug_semaphore, semid, func, line);
#endif /****APP_LOCK_DEBUG****/
	if(OSAL_WAIT_FOREVER_TIME == tmout)
	{
		sem_wait(&semcb->psema_id);
	}
	else
	{
		clock_gettime(CLOCK_REALTIME, &ts );
		ts.tv_sec += (tmout / 1000 );
		ts.tv_nsec += ( tmout % 1000 ) * 1000000;
		/* Try to lock Semaphore */
		sts = sem_timedwait(&semcb->psema_id, &ts);
		if(sts)
			ercd = E_TIMEOUT;
	}
#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
    lock_debug_out(lock_debug_semaphore, semid);
#endif /****APP_LOCK_DEBUG****/
	return(ercd);
}

