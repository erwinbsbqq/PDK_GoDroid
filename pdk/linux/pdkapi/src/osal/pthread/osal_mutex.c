#include <hld_cfg.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>

#include "queue.h"
#include "timer.h"
#include "task.h"

#define _GNU_SOURCE

#if 0
#define osal_printf(fmt, args...) ADR_DBG_PRINT(OSAL, fmt, ##args)
#else
#define osal_printf(...)	do{}while(0)
#endif
#define MUT_LIMIT_CNT 256

typedef struct mut_control_block
{
        ID      mutid;          /* semaphore Identifier */
        UINT32     mutatr;         /* attribute, OBJ_NONEXIST, or SEM_FIFO */
        pthread_mutex_t pmut_id;
} MUT_CB;

UINT32 total_mutex = 0;
MUT_CB mut_st[MUT_LIMIT_CNT] = {{0, OBJ_NONEXIST, 0},};
pthread_mutex_t mut_id;
ID OS_CreateMutex(void)
{
	MUT_CB *mutcb;
	UINT32 i =0;
	ID cur_mut_id = 0;
	if(total_mutex ==0)
	{
		total_mutex = 1;

		for( i = 0; i < MUT_LIMIT_CNT; i++)
		{
			mut_st[i].mutatr = OBJ_NONEXIST;
		}
		pthread_mutex_init(&mut_id,NULL);
	}

	// search for unused entry
	for(cur_mut_id=0;cur_mut_id<MUT_LIMIT_CNT;cur_mut_id++)
	{
		if (mut_st[cur_mut_id].mutatr==OBJ_NONEXIST)
		{
			break;
		}
	}
	if (cur_mut_id>=MUT_LIMIT_CNT)
	{ // entry all used
		return INVALID_ID;
	}

	mutcb = &mut_st[cur_mut_id];

	mutcb->mutatr   =TA_TFIFO;// t_csem->sematr;
	mutcb->mutid = cur_mut_id;
	//pthread_mutexattr_init(&mattr);
	//pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_NORMAL);

	pthread_mutex_init(&mutcb->pmut_id, NULL);
	//osal_printf("create_mutex_pid: %u, id: %d\n", mutcb->pmut_id, cur_mut_id);
	return cur_mut_id;
}

ER OS_DeleteMutex(ID meid)
{
	ER ret = E_OK;
	MUT_CB   *mutcb;

	//if(meid<0)
		//return(E_FAILURE);
	if(meid>=MUT_LIMIT_CNT)
		return(E_FAILURE);

	mutcb = &mut_st[meid];
	if(mutcb->mutatr == OBJ_NONEXIST)
	    return(E_FAILURE);

	mutcb->mutatr = OBJ_NONEXIST;
	pthread_mutex_destroy(&mutcb->pmut_id);
	return ret;
}

#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
ER OS_LockMutex(ID meid, TMO tmout, char *func, int line)
#else
ER OS_LockMutex(ID meid, TMO tmout)
#endif /****APP_LOCK_DEBUG****/
{
	struct timespec ts;
	ER ret = E_OK;
	int sts;
	MUT_CB *mutcb;

	if(meid>=MUT_LIMIT_CNT)
	{
		osal_printf("lock:meid out of range\n");
		return(E_FAILURE);
	}

	mutcb = &mut_st[meid];
	if(mutcb->mutatr== OBJ_NONEXIST)
	{
		osal_printf("lock:meid non_exist\n");
    		return(E_FAILURE);
	}
#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
    lock_debug_in(lock_debug_mutex, meid, func, line);
#endif /****APP_LOCK_DEBUG****/
	if(tmout == OSAL_WAIT_FOREVER_TIME)
	{
		pthread_mutex_lock(&mutcb->pmut_id);
	}
	else
	{
		clock_gettime(CLOCK_REALTIME, &ts );
		//osal_printf("bf---ts_second %u, ts_nsecond: %u\n", ts.tv_sec, ts.tv_nsec);
		ts.tv_sec += (tmout / 1000 );
		ts.tv_nsec += ( tmout % 1000 ) * 1000000;
		//osal_printf("af---ts_second %u, ts_nsecond: %u\n", ts.tv_sec, ts.tv_nsec);
		/* Try to lock Mutex */
		#ifdef ADR_ALIDROID
			sts = pthread_mutex_lock_timeout_np(&mutcb->pmut_id, tmout);
		#else
			sts = pthread_mutex_timedlock(&mutcb->pmut_id, &ts);
		#endif
		//osal_printf("lock ret_value: %d, mutex_pthid: %u, mut_id: %d\n", sts, mutcb->pmut_id, meid);
		if(sts)
		{
			ret = E_TIMEOUT;
			osal_printf("lock tim_out: %d, met_id: %d\n", sts, meid);
		}
	}

	return ret;
}

ER OS_UnlockMutex(ID meid)
{
	MUT_CB   *mutcb;
	ER      ercd = E_OK;

	//if ((meid < 0) || (meid>=MUT_LIMIT_CNT))
	if (meid>=MUT_LIMIT_CNT)
	{
		osal_printf("unlock: meid out of range\n");
		return(E_FAILURE);
	}

	mutcb = &mut_st[meid];
	if(mutcb->mutatr == OBJ_NONEXIST)
	{
		osal_printf("unlock:meid non_exist\n");
		return(E_FAILURE);
	}

	pthread_mutex_unlock(&mutcb->pmut_id);
#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
    lock_debug_out(lock_debug_mutex, meid);
#endif /****APP_LOCK_DEBUG****/
	return(ercd);
}

