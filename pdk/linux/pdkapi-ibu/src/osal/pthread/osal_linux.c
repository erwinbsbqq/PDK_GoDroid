#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>
#include <timedoctor_common.h>
#include <os/tds2/itron.h>
#include "queue.h"
#include "timer.h"
#include "task.h"

#define _GNU_SOURCE
//thread
#define PTH_NONEXIST		0xFFFFFFFF
#if 0
#define osal_printf printf
#else
#define osal_printf(...)	do{}while(0)
#endif
#define OSAL_WAIT_FOREVER_TIME		0xFFFFFFFF

#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
typedef struct
{
    int id;
    char *func;
    int line;
} lock_debug;
enum lock_debug_type
{
    lock_debug_semaphore,
    lock_debug_mutex,
    lock_debug_flag,
    lock_debug_msgqueue,
};
static int _lock_debug_init_ = 0;
static void lock_debug_in(int type, int id, char *func, int line);
static void lock_debug_out(int type, int id);
#endif /****SYS_LOCK_DEBUG****/
typedef struct thread_control_block
{
	ID      	th_id;          /* message buffer Identifier */
	BOOL   	state;         /* message buffer attribute */
	pthread_t pthid;
	int		pid;
	char 	name[5];
} PTH_CB;
UINT32 	total_pth= 0;
#define THREAD_MAX_CNT 128
PTH_CB pth_st[THREAD_MAX_CNT];

//////////////////////////////////////////////

#define _SHOW_TASK_PID_

#ifdef _SHOW_TASK_PID_ /* FOR DEBUG  --Doy.Dong, 2011-12-16*/
#include <sys/syscall.h>

typedef struct
{
    UINT32 param1;
    UINT32 param2;
    char   name[8];
    FP     task;
	int	   th_id;
} _temp_task_param;

static int _thread_pid(char *task_name)
{
    int pid = -1;
    pid = syscall(__NR_gettid);
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>task %s -> PID = %d\n", task_name, pid);
    return pid;
}


static char *trim(char *taskname)
{
    char *s = taskname;
    int i =0 ;
    for(i= 0; i< 3;i++)
    {
        if(*(s+i) < 0x20 || *(s+i) > 0x7f)
            *(s+i) = '-';
    }
    return s;
}

static void _task_exit()
{
    UINT32 taskid=0;
    pthread_t pthid = pthread_self();
	for(taskid = 0; taskid < THREAD_MAX_CNT; taskid++)
	{
		if(pth_st[taskid].pthid == pthid)
		{
            pth_st[taskid].state = PTH_NONEXIST;//reset state
            printf("*****task %d exit ...\n", taskid);
            break;
		}
	}
}

static void _show_taskinfo_hook_(UINT32 param1, UINT32 param2)
{
    unsigned int pid = 0;
    char taskname[32];
    char cmd[128];
    _temp_task_param para;
    pthread_detach(pthread_self());
    memcpy(&para,  param1, sizeof(_temp_task_param));
    free((void *)param1);
    UINT32 p1 = para.param1;
    UINT32 p2 = para.param2;
    FP f = para.task;
    sprintf(taskname, "%s(0x%x)", trim(para.name), (UINT32)f);
    pid = _thread_pid(taskname);
	pth_st[para.th_id].pid = pid;
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "echo \"%s\" > /tmp/runtime/task/%d", taskname, pid);
    system(cmd);
    printf("CMD: %s\n", cmd);
    f(p1, p2);
    ///////////////////////////////
    _task_exit();
}
#endif /****FOR DEBUG****/

ID_THREAD OS_CreateThread(T_CTHD * pk_cthd)
{
	int ret = 0;
	pthread_t pth_id;
	//pk_cthd->task;
	const pthread_attr_t cur_attr;
	ID_THREAD 	thdid;
	ER 			ercd=E_OK;
	int stacksize;
	UINT16 i = 0;
	if(pk_cthd == NULL)
		return INVALID_ID;
	if(pk_cthd->task == NULL)
		return INVALID_ID;
	if(total_pth == 0)
	{
		total_pth = 1;
		for(i = 0; i <THREAD_MAX_CNT; i++)
		{
			pth_st[i].state =PTH_NONEXIST;
		}
	}
	
	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		if(pth_st[thdid].state == PTH_NONEXIST)
			break;
	}
	if (thdid>=THREAD_MAX_CNT)
	{
        printf("*** Up to MAX pth_st num!!!\n");
		return INVALID_ID;
	}
	pth_st[thdid].state = 0;
	pth_st[thdid].th_id = thdid;
    memcpy(pth_st[thdid].name, pk_cthd->name, 3);
    pthread_attr_init (&cur_attr);
	stacksize = (int)pk_cthd->stksz;
	//ret = pthread_attr_setstacksize(&cur_attr, stacksize);	
	pthread_attr_setdetachstate (&cur_attr,PTHREAD_CREATE_DETACHED);

#ifdef _SHOW_TASK_PID_ /* FOR DEBUG  --Doy.Dong, 2011-12-16*/
    _temp_task_param *para = (_temp_task_param *)malloc(sizeof(_temp_task_param));
    memset(para, 0, sizeof(_temp_task_param));
    memcpy(para->name, pk_cthd->name, 3);
    para->param1 = pk_cthd->para1;
    para->param2 = pk_cthd->para2;
    para->task = pk_cthd->task;
	para->th_id = thdid;
    pk_cthd->task = _show_taskinfo_hook_;
    pk_cthd->para1 = para;
#endif /****FOR DEBUG****/

	ret =pthread_create( &pth_st[thdid].pthid, &cur_attr, (pk_cthd->task), (void *)pk_cthd->para1);

#if (defined(CONFIG_TIMEDOCTOR) || defined(TD_IN_MODULE))
    {
        char name[4];
        memcpy(name, pk_cthd->name, 3);
        name[3] = 0;
        td_setName(TDI_TASK, getPthreadPid(pth_st[thdid].pthid), name);
    }
#endif

    pthread_attr_destroy (&cur_attr);
    
	//pth_st[thdid].pthid = pth_id;
	if(ret)
	{
		osal_printf("create ret_value: %d\n", ret);
		return INVALID_ID;
	}
	printf("pthread_id create: %u, task[%d]\n", pth_st[thdid].pthid, thdid);
	return (thdid);
}

ER OS_DeleteThread(ID_THREAD thd_id)
{
	pthread_t pth_id;
	ER 	ercd=E_OK;
	int err_ret = 0;

	pth_id = pth_st[thd_id].pthid;
	if(thd_id > THREAD_MAX_CNT)
		return(E_FAILURE);

	#ifndef ADR_ALIDROID
	err_ret = pthread_cancel(pth_id);
	//pthread_join(pth_id, NULL);
	if(err_ret)
		return(E_FAILURE);	
	#endif
	return(ercd);
}

void OS_Exit(ER ExitCode)
{
	pthread_exit(&ExitCode);
}


ID OS_GetCurrentThreadID(void)
{
	UINT16 i = 0;
	pthread_t cur_pth_id;
	cur_pth_id = pthread_self();
	//osal_printf("th_id: %u\n", cur_pth_id);
	for(i = 0; i < THREAD_MAX_CNT; i ++)
	{
		if(cur_pth_id == pth_st[i].pthid)
			break;
	}
	//osal_printf("id: %d\n", i);
	if (i >=THREAD_MAX_CNT)
	{
		return INVALID_ID;
	}
	return (ID)i;
}

//semaphore
#define SEM_LIMIT_CNT 256

typedef struct sem_control_block
{
        ID      semid;          /* semaphore Identifier */
        ATR     sematr;         /* attribute, OBJ_NONEXIST, or SEM_FIFO */
        sem_t psema_id;
} SEM_CB;
UINT32 total_sema = 0;
SEM_CB sema_st[SEM_LIMIT_CNT] = {{0, OBJ_NONEXIST, 0},};
pthread_mutex_t sem_mutex;

ID OS_CreateSemaphore(INT32 semcnt)
{
	SEM_CB   *semcb;
	ID semid;
	UINT i = 0;

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
	sem_init(&semcb->psema_id, 0, semcnt);

	return (semid);
}

ER OS_DelSemaphore(ID semid)
{
	ER ret = E_OK;
	SEM_CB   *semcb;

	if(semid<0)
		return(E_FAILURE);
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

	if(semid<0) return(E_FAILURE);
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

	if(semid<0)
		return(E_FAILURE);
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

//mutex

#define MUT_LIMIT_CNT 256

typedef struct mut_control_block
{
        ID      mutid;          /* semaphore Identifier */
        ATR     mutatr;         /* attribute, OBJ_NONEXIST, or SEM_FIFO */
        pthread_mutex_t pmut_id;
} MUT_CB;

UINT32 total_mutex = 0;
MUT_CB mut_st[MUT_LIMIT_CNT] = {{0, OBJ_NONEXIST, 0},};
pthread_mutex_t mut_id;
ID OS_CreateMutex(void)
{
	MUT_CB *mutcb;
	UINT i =0;
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

	if(meid<0)
		return(E_FAILURE);
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

		
	if((meid<0) || (meid>=MUT_LIMIT_CNT))
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

	if(meid<0) 
	if((meid<0) ||(meid>=SEM_LIMIT_CNT))
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

//flag
#define FLG_LIMIT_CNT 256
UINT32 total_flag = 0;
typedef struct flag_control_block 
{
    ID			flgid;
    ATR     		flgatr;
    UINT    		flgptn;//current flag pattern
    pthread_mutex_t uni_mutex;
    pthread_cond_t	flg_cond;
} FLG_CB;
FLG_CB flg_st[FLG_LIMIT_CNT] = {{0,OBJ_NONEXIST, 0},};


ID  OS_CreateFlag(UINT32 flgptn)
{
	
	struct timespec to;
	ID flgid;
	FLG_CB   *flgcb;
	UINT i = 0;
	UINT j = 0;
	
  	if(total_flag ==0)
	{
		total_flag = 1;
		for( i = 0; i < FLG_LIMIT_CNT; i++)
		{
			flg_st[i].flgid=INVALID_ID;
			flg_st[i].flgatr = OBJ_NONEXIST;
			pthread_mutex_init(&flg_st[i].uni_mutex,NULL);
		}
	}
	for(flgid=0;flgid<FLG_LIMIT_CNT;flgid++)
	{
		if (flg_st[flgid].flgatr==OBJ_NONEXIST)
		{
			break;
		}
	}
	
	flgcb = &(flg_st[flgid]);//get_flgcb(flgid);
	pthread_mutex_lock(&flgcb->uni_mutex);
	if (flgid==FLG_LIMIT_CNT)
	{
		pthread_mutex_unlock(&flgcb->uni_mutex);
		osal_printf("%s failed, not enough flag.\n", __FUNCTION__);
		return INVALID_ID;
	}
	
	
	//for(j = 0; j < 32; j++)
	pthread_cond_init(&(flgcb->flg_cond), NULL);
	
	flgcb->flgatr = TA_WMUL;//pk_cflg->flgatr;
	flgcb->flgptn = flgptn;//pk_cflg->iflgptn;
	flgcb->flgid = flgid;

	pthread_mutex_unlock(&flgcb->uni_mutex);
	osal_printf("%s %d, ptn:0x%08x\n", __FUNCTION__, flgid, flgptn);
	return flgid;
}

ER OS_DelFlag(const ID flgid)
{
	UINT i = 0;
	FLG_CB   *flgcb;
	ER      ercd = E_OK;

	if (flgid < 0) return(E_FAILURE);
	if (flgid >= FLG_LIMIT_CNT) return(E_FAILURE);

	flgcb = &(flg_st[flgid]);
	pthread_mutex_lock(&flgcb->uni_mutex);
	if (flgcb->flgatr == OBJ_NONEXIST)
	{
		osal_printf("%s failed, flg not exist.\n", __FUNCTION__);
		ercd = E_FAILURE;
	}
	else
	{
		flgcb->flgatr = OBJ_NONEXIST;
		flgcb->flgid=INVALID_ID;
		flgcb->flgptn=0;
		//for( i = 0; i < 32; i++)
		{
			pthread_cond_broadcast(&(flgcb->flg_cond));
			pthread_cond_destroy(&(flgcb->flg_cond));
		}
	}
	/*
	for(i = flgid;  i < total_flag; i++)
	{
		flg_st[i] = flg_st[i + 1];
	}
	if(total_flag > 0)
		total_flag--;
	else if(total_flag == 1)
	{
		pthread_mutex_destroy(&uni_mutex);
		pthread_cond_destroy(&uni_cond);
	}
	*/
	pthread_mutex_unlock(&flgcb->uni_mutex);
	osal_printf("%s %d\n", __FUNCTION__, flgid);
	return(ercd);
}


ER OS_SetFlag(ID flgid, UINT setptn)
{
	UINT i = 0;
	FLG_CB   *flgcb;
	ER      ercd = E_OK;

	if (flgid < 0) return(E_FAILURE);
	if (flgid >= FLG_LIMIT_CNT) return(E_FAILURE);

	flgcb = &(flg_st[flgid]);
	pthread_mutex_lock(&flgcb->uni_mutex);
	
	
	if (flgcb->flgatr == OBJ_NONEXIST)
	{
		osal_printf("%s %d, failed, not exist.\n", __FUNCTION__, flgid);
		ercd = E_FAILURE;
	}
	else
	{
		flgcb->flgptn |= setptn;
		osal_printf("%s %d, ptn: 0x%08x\n", __FUNCTION__, flgid, setptn);
		
		pthread_cond_broadcast(&(flgcb->flg_cond));
		//pthread_cond_signal(&(flgcb->flg_cond));
	}

	pthread_mutex_unlock(&flgcb->uni_mutex); 
	return(ercd);
}



ER OS_ClearFlag(ID flgid, UINT clrptn)
{
	FLG_CB   *flgcb;
   	ER      ercd = E_OK;

	if (flgid<0) return(E_FAILURE);
	if (flgid >= FLG_LIMIT_CNT) return(E_FAILURE);

	// Invert flag pattern. Therefore, bit which value is '1' will be cleared.
	clrptn = ~clrptn;

	flgcb = &(flg_st[flgid]);

	pthread_mutex_lock(&flgcb->uni_mutex);

	if (flgcb->flgatr == OBJ_NONEXIST)
	{
		osal_printf("%s %d, failed, not exist.\n", __FUNCTION__, flgid);
		ercd = E_FAILURE;
	}
	else
	{
		flgcb->flgptn &= clrptn;
	}
	
    	pthread_mutex_unlock(&flgcb->uni_mutex); 
    	osal_printf("%s %d, ptn: 0x%08x\n", __FUNCTION__, flgid, clrptn);
	return(ercd);
}


static inline BOOL flag_cond_satis( UINT32 flag_ptn, UINT waiptn, UINT wfmode)
{
	if (wfmode & TWF_ORW)
	{
	    return((BOOL)(flag_ptn & waiptn));
	}
	else
	{
	    return((flag_ptn & waiptn) == waiptn);
	}
}

#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
ER OS_WaitFlagTimeOut(UINT *p_flgptn, ID flgid, UINT waiptn, UINT wfmode, TMO tmout, char *func, int line)
#else
ER OS_WaitFlagTimeOut(UINT *p_flgptn, ID flgid, UINT waiptn, UINT wfmode, TMO tmout)
#endif /****APP_LOCK_DEBUG****/
{
	struct timespec old_ts;
	struct timespec new_ts;
	struct timespec ts;
	UINT32 flg_ptn;
	int err_ret = -1;
	ER	ercd = E_FAILURE;
	UINT i = 0;
	UINT twf_or_flg = 0;
	UINT time_ms = 0;

	FLG_CB   *flgcb;

//20150527 modified to resolve logic issues.
//if user not set *p_flgptn, and OS_WaitFlagTimeOut() timeout,
//the *p_flgptn will have a wild value.
	if (p_flgptn != NULL)
	{
		*p_flgptn = 0;
	}

	if (flgid<0) return(E_FAILURE);
	if (flgid >= FLG_LIMIT_CNT) return(E_FAILURE);
	if(p_flgptn == NULL)
		return(E_FAILURE);

	//clock_gettime(CLOCK_REALTIME, &old_ts );
	
	flgcb = &(flg_st[flgid]);
#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
    lock_debug_in(lock_debug_flag, flgid, func, line);
#endif /****APP_LOCK_DEBUG****/
CHECK_FLAG_PATTERN:
	osal_printf("%s, %d\n",__FUNCTION__, flgid);
	pthread_mutex_lock(&flgcb->uni_mutex);
	if (flgcb->flgatr == OBJ_NONEXIST)
	{
		osal_printf("wait flg %d err, not exist\n", flgid);
		ercd = E_FAILURE;
	}
	else if(flag_cond_satis(flgcb->flgptn, waiptn, wfmode))
	{
		osal_printf("wait flag, id:%d done. ptn:0x%08x\n", flgid, waiptn);
		err_ret = 0;
	}
	else
	{
		if(tmout == OSAL_WAIT_FOREVER_TIME)
		{
			osal_printf("wait flg forever: %d, ptn:0x%08x\n", flgid, waiptn);
			err_ret = pthread_cond_wait(&(flgcb->flg_cond), &flgcb->uni_mutex);
			osal_printf("wait flg forever: %d, wake up\n", flgid);
			if(!err_ret)
			{
				pthread_mutex_unlock(&flgcb->uni_mutex);
				goto CHECK_FLAG_PATTERN;
			}
			else
			{
				ercd = E_FAILURE;
			}
		}
		else
		{
			//printf("wait msg2 %d\n", tmout);			
			clock_gettime(CLOCK_REALTIME, &ts );
			//osal_printf("ts_value_af: %u: %u\n", ts.tv_sec, ts.tv_nsec);
			ts.tv_sec += (tmout / 1000 );
			ts.tv_nsec += ( tmout % 1000 ) * 1000000;
			while(ts.tv_nsec >= 1000000000){
				ts.tv_nsec -= 1000000000;
				ts.tv_sec++;
			}

			osal_printf("wait flg tmo: %d, tmo:%d, ptn:0x%08x\n", flgid, tmout,waiptn);
			clock_gettime(CLOCK_REALTIME, &old_ts );
			err_ret = pthread_cond_timedwait(&(flgcb->flg_cond), &flgcb->uni_mutex, &ts); 
			
			if(!err_ret )
			{
				clock_gettime(CLOCK_REALTIME, &new_ts );
				time_ms = (new_ts.tv_sec - old_ts.tv_sec) * 1000\
					+ (new_ts.tv_nsec - old_ts.tv_nsec) / 1000000;
				if(tmout>time_ms)
					tmout-=time_ms;
				else
					tmout=0;

				pthread_mutex_unlock(&flgcb->uni_mutex);
				goto CHECK_FLAG_PATTERN;
			}
			else
			{
				ercd = E_TIMEOUT;
			}
		}

	}
	
EXIT:	
	if(err_ret == 0)	
	{
		if(p_flgptn != NULL)
			*p_flgptn = flgcb->flgptn;		
		ercd = E_OK;
		if(wfmode&TWF_CLR)
			flgcb->flgptn &= ~(waiptn);
	}
	
	osal_printf("wait flag id:%d done exit ret %x\n",flgid,ercd);	

	pthread_mutex_unlock(&flgcb->uni_mutex);  
#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
    lock_debug_out(lock_debug_flag, flgid);
#endif /****APP_LOCK_DEBUG****/
	return(ercd);

}

//message buffer

typedef struct msgbuf_control_block
{
        ID      mbfid;          /* message buffer Identifier */
        ATR     mbfatr;         /* message buffer attribute */
        INT     bufsz;          /* buffer size */
        INT     maxmsz;         /* message maximum size */
        INT     frbufsz;        /* free buffer size */
        INT     head;           /* first message position in buffer */
        INT     tail;           /* message end position in buffer */
        char*   buffer;         /* message buffer pointer */
        pthread_cond_t msg_cond;	
	 pthread_mutex_t mutex;
} MSG_CB;
#define MSG_LIMIT_CNT 128
#define ROUND_SZ(sz)             (((sz) + (sizeof(INT)-1)) & ~((sizeof(INT)-1)))

//UINT16 total_msg = 0;
static UINT total_msg = 0;
MSG_CB msg_st[MSG_LIMIT_CNT] = {{0,OBJ_NONEXIST,0,0,0,0,0,NULL},};
pthread_mutex_t msg_mutex;

ID OS_CreateMsgBuffer(T_CMBF *pk_cmbf)
{
	MSG_CB   *mbfcb;
	INT     bufsz;
	UINT	mbfid;
	UINT	i = 0;
	
	if(pk_cmbf == NULL)
		return INVALID_ID;
	if(pk_cmbf->bufsz<=0)
		return(INVALID_ID);
	
	if(total_msg == 0)
	{
		total_msg = 1;
		pthread_mutex_init(&msg_mutex,NULL);
		for(i = 0; i <MSG_LIMIT_CNT; i++)
		{
			msg_st[i].mbfatr = OBJ_NONEXIST;
		}
	}

	pthread_mutex_lock(&msg_mutex);
	for(mbfid=0;mbfid<MSG_LIMIT_CNT;mbfid++)
	{
		if (msg_st[mbfid].mbfatr==OBJ_NONEXIST)
		{
			break;
		}
	}

	if (mbfid==MSG_LIMIT_CNT)
	{	
		pthread_mutex_unlock(&msg_mutex);			
		return INVALID_ID;
	}

	mbfcb = &msg_st[mbfid];
	bufsz = ROUND_SZ(pk_cmbf->bufsz);
	
	//mbfcb->buffer = pk_cmbf->buffer;//msgbuf[IDtoINDEX_MBF(mbfid)];
	if ((mbfcb->buffer=(char *)malloc(bufsz))==NULL)
	{
		pthread_mutex_unlock(&msg_mutex);		
		return INVALID_ID;
	}
	else
	{
		mbfcb->mbfatr = TA_TFIFO;//pk_cmbf->mbfatr;
		mbfcb->bufsz = mbfcb->frbufsz = bufsz;
		mbfcb->maxmsz = pk_cmbf->maxmsz;
		mbfcb->head = mbfcb->tail = 0;
		pthread_mutex_init(&mbfcb->mutex, NULL);		
	}
	
	pthread_cond_init(&(mbfcb->msg_cond),NULL);
	
	pthread_mutex_unlock(&msg_mutex);		
	return mbfid;
}


ER OS_ClearMsgBuffer(ID mbfid)
{
	MSG_CB	*mbfcb;
	ER	ercd = E_OK;

	if(mbfid<0)
		return(E_FAILURE);
	if(mbfid >= MSG_LIMIT_CNT)
		return(E_FAILURE);

	mbfcb = &msg_st[mbfid];
	pthread_mutex_lock(&mbfcb->mutex);
	if (mbfcb->mbfatr == OBJ_NONEXIST)
	{
		pthread_mutex_unlock(&mbfcb->mutex);
		 return(E_FAILURE);
	}

	mbfcb->frbufsz = mbfcb->bufsz ;
	mbfcb->head = mbfcb->tail = 0;
	pthread_mutex_unlock(&mbfcb->mutex);
	return(ercd);
}

ER OS_DelMessageBuffer(ID mbfid)
{
	MSG_CB	*mbfcb;
	ER	ercd = E_OK;

	if(mbfid<0)
		return(E_FAILURE);
	if(mbfid>=MSG_LIMIT_CNT)
		return(E_FAILURE);

	mbfcb = &msg_st[mbfid];
	pthread_mutex_lock(&msg_mutex);
	if (mbfcb->mbfatr == OBJ_NONEXIST)
	{
		pthread_mutex_unlock(&msg_mutex);
		 return(E_FAILURE);
	}
	
	pthread_cond_destroy(&(mbfcb->msg_cond));
			
	if ((mbfcb->bufsz > 0) &&(mbfcb->buffer !=NULL))
	{
		free(mbfcb->buffer);
		mbfcb->buffer= NULL;
	}
	mbfcb->mbfatr = OBJ_NONEXIST;
	pthread_mutex_unlock(&msg_mutex);
	return(ercd);
}

BOOL mbf_free(MSG_CB* mbfcb, INT msgsz)
{
    return((long)(sizeof(INT)) + msgsz <= mbfcb->frbufsz);
}


BOOL mbf_empty(MSG_CB* mbfcb)
{
    return(mbfcb->frbufsz == mbfcb->bufsz);
}


static void msg_to_mbf(MSG_CB* mbfcb, VP msg, INT msgsz)
{
    INT     tail = mbfcb->tail;
    VB      *buffer = mbfcb->buffer;
    INT     remsz;

    mbfcb->frbufsz -= sizeof(INT) + ROUND_SZ(msgsz);
    *((INT *) &(buffer[tail])) = msgsz;
    tail += sizeof(INT);
    if (tail >= mbfcb->bufsz)
    {
        tail = 0;
    }

    if ((remsz = mbfcb->bufsz - tail) < msgsz)
    {
        memcpy(&(buffer[tail]), msg, remsz);
        msg = (VP)((VB *)msg + remsz);
        msgsz -= remsz;
        tail = 0;
    }
    memcpy(&(buffer[tail]), msg, msgsz);
    tail += ROUND_SZ(msgsz);
    if (tail >= mbfcb->bufsz)
    {
            tail = 0;
    }
    mbfcb->tail = tail;
}

static INT mbf_to_msg(MSG_CB* mbfcb, VP msg)
{
    INT     head = mbfcb->head;
    VB      *buffer = mbfcb->buffer;
    INT     msgsz, copysz;
    INT     remsz;

    msgsz = *((INT *) &(buffer[head]));
    head += sizeof(INT);
    if (head >= mbfcb->bufsz)
    {
        head = 0;
    }
    mbfcb->frbufsz += sizeof(INT) + ROUND_SZ(msgsz);

    copysz = msgsz;
    if ((remsz = mbfcb->bufsz - head) < copysz)
    {
        memcpy(msg, &(buffer[head]), remsz);
        msg = (VP)((VB *)msg + remsz);
        copysz -= remsz;
        head = 0;
    }
    memcpy(msg, &(buffer[head]), copysz);
    head += ROUND_SZ(copysz);
    if (head >= mbfcb->bufsz)
    {
        head = 0;
    }
    mbfcb->head = head;
    return(msgsz);
}

ER OS_SendMsgTimeOut(ID mbfid, VP msg, INT msgsz, TMO tmout)
{
	struct timespec old_ts;
	struct timespec new_ts;	
	MSG_CB   *mbfcb;
	ER      ercd = E_OK;
	UINT time_ms = 0;	

	clock_gettime(CLOCK_REALTIME, &old_ts );
	
	mbfcb = &msg_st[mbfid];
	
	osal_printf("%s : id %d\n", __FUNCTION__, mbfid);
	
	pthread_mutex_lock(&mbfcb->mutex);			
	
	if (mbfcb->mbfatr == OBJ_NONEXIST)
	{
	    ercd = E_FAILURE;
	}
	else if (msgsz > mbfcb->maxmsz)
	{
		ercd = E_FAILURE;
	}
	else
	{
		while(1)
		{			
			if (mbf_free(mbfcb, msgsz))
			{
				msg_to_mbf(mbfcb, msg, msgsz);
				pthread_cond_signal(&(mbfcb->msg_cond));
				ercd = E_OK;
											
				goto EXIT;
			}			
			
			clock_gettime(CLOCK_REALTIME, &new_ts);
			if(tmout != OSAL_WAIT_FOREVER_TIME)
			{
				time_ms = (new_ts.tv_sec - old_ts.tv_sec) * 1000
					+ (new_ts.tv_nsec - old_ts.tv_nsec) / 1000000;
				//printf("n %d %d o %d %d timeout %d ms %d\n", new_ts.tv_sec, new_ts.tv_nsec
				//	, old_ts.tv_sec, old_ts.tv_nsec, tmout, time_ms);						
				if(tmout < time_ms)
				{
					osal_printf("wait flag time out %d\n", time_ms);						
					ercd = E_TIMEOUT;
					goto EXIT;
				}
			}	

			pthread_mutex_unlock(&mbfcb->mutex);
			OS_TaskSleep(1);
			pthread_mutex_lock(&mbfcb->mutex);				
		}
	}
		
EXIT:	
	pthread_mutex_unlock(&mbfcb->mutex);		
	osal_printf("send msg ret %d\n", ercd);
	return(ercd);
}


#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
ER OS_GetMsgTimeOut(VP msg, INT *p_msgsz, ID mbfid, TMO tmout, char *func, int line)
#else
ER OS_GetMsgTimeOut(VP msg, INT *p_msgsz, ID mbfid, TMO tmout)
#endif /****APP_LOCK_DEBUG****/
{
	struct timespec ts;
	MSG_CB   *mbfcb;
	INT     msgsz;
	ER      ercd = E_OK;
	int err_ret = 0;

	if((p_msgsz == NULL) ||(msg ==NULL))
		return E_FAILURE;
	
	mbfcb = &msg_st[mbfid];	
	
	pthread_mutex_lock(&mbfcb->mutex);
	if (mbfcb->mbfatr == OBJ_NONEXIST)
	{
		pthread_mutex_unlock(&mbfcb->mutex);
		ercd = E_FAILURE;
		return(ercd);
	}

	if (!mbf_empty(mbfcb))
	{
		osal_printf("get msg 1\n");
		*p_msgsz = mbf_to_msg(mbfcb, msg);
		ercd = E_OK;
	}
	else
	{
		//clock_gettime(CLOCK_REALTIME, &ts );
		//osal_printf("ts_value_bf: %u: %u\n", ts.tv_sec, ts.tv_nsec);
		//usleep(100*1000);
#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
        lock_debug_in(lock_debug_msgqueue, mbfid, func, line);
#endif /****APP_LOCK_DEBUG****/
		if(tmout == OSAL_WAIT_FOREVER_TIME)
		{
			osal_printf("wait msg1\n");
			err_ret = pthread_cond_wait(&(mbfcb->msg_cond), &mbfcb->mutex);
		
			if(!err_ret)
				ercd = E_OK;
			else
				ercd = E_FAILURE;
		}
		else
		{
			//printf("wait msg2 %d\n", tmout);			
			clock_gettime(CLOCK_REALTIME, &ts );
			//osal_printf("ts_value_af: %u: %u\n", ts.tv_sec, ts.tv_nsec);
			ts.tv_sec += (tmout / 1000 );
			ts.tv_nsec += ( tmout % 1000 ) * 1000000;
			while(ts.tv_nsec >= 1000000000){
				ts.tv_nsec -= 1000000000;
				ts.tv_sec++;
			}
			err_ret = pthread_cond_timedwait(&(mbfcb->msg_cond), &mbfcb->mutex, &ts); 
			
			if(!err_ret )
				ercd = E_OK;
			else
				ercd = E_TIMEOUT;
		}
		
		if (ercd == E_OK)
		{
			osal_printf("get msg 2\n");	
			if(!mbf_empty(mbfcb))
				*p_msgsz = mbf_to_msg(mbfcb, msg);
			else
			{
				osal_printf("get msg 2 fail\n");				
				ercd = E_FAILURE;
			}
		}
#ifdef APP_LOCK_DEBUG  /* --Doy.Dong, 2013-1-15*/
        lock_debug_out(lock_debug_msgqueue, mbfid);
#endif /****APP_LOCK_DEBUG****/
	}

	pthread_mutex_unlock(&mbfcb->mutex);
	
	return (ercd);
}


#ifdef APP_LOCK_DEBUG /* lock debug  --Doy.Dong, 2013-1-15*/
static lock_debug g_lock_debug[SEM_LIMIT_CNT
                               + MUT_LIMIT_CNT
                               + FLG_LIMIT_CNT
                               + MSG_LIMIT_CNT];
static void lock_debug_in(int type, int id, char *func, int line)
{
    int offset = 0;
    if(_lock_debug_init_ == 0)
    {
        _lock_debug_init_=1;
        memset(g_lock_debug,0, sizeof(g_lock_debug));
    }
    switch (type)
    {
        case lock_debug_semaphore:
            offset = 0;
            break;
        case lock_debug_mutex:
            offset = SEM_LIMIT_CNT;
            break;
        case lock_debug_flag:
            offset = SEM_LIMIT_CNT + MUT_LIMIT_CNT;
            break;
        case lock_debug_msgqueue:
            offset = SEM_LIMIT_CNT + MUT_LIMIT_CNT + FLG_LIMIT_CNT;
            break;
    }

    g_lock_debug[offset + id].id = id;
    g_lock_debug[offset + id].func = func;
    g_lock_debug[offset + id].line = line;
}
static void lock_debug_out(int type, int id)
{
    int offset = 0;
    if(_lock_debug_init_ == 0)
    {
        _lock_debug_init_=1;
        memset(g_lock_debug,0, sizeof(g_lock_debug));
    }
    switch (type)
    {
        case lock_debug_semaphore:
            offset = 0;
            break;
        case lock_debug_mutex:
            offset = SEM_LIMIT_CNT;
            break;
        case lock_debug_flag:
            offset = SEM_LIMIT_CNT + MUT_LIMIT_CNT;
            break;
        case lock_debug_msgqueue:
            offset = SEM_LIMIT_CNT + MUT_LIMIT_CNT + FLG_LIMIT_CNT;
            break;
    }

    g_lock_debug[offset + id].id = -1;
    g_lock_debug[offset + id].func = NULL;
    g_lock_debug[offset + id].line = 0;
}
void lock_debug_print()
{
    int i = 0;

    for (i = 0; i < SEM_LIMIT_CNT + MUT_LIMIT_CNT + MSG_LIMIT_CNT; i++)
    {
        if (g_lock_debug[i].func)
        {
            printf("func:%s,line:%d wait on %d\n", g_lock_debug[i].func,
                   g_lock_debug[i].line,
                   g_lock_debug[i].id);
        }
    }
}
#endif /****lock debug****/


