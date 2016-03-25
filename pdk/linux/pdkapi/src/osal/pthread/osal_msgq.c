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
typedef struct msgbuf_control_block
{
        ID      mbfid;          /* message buffer Identifier */
        UINT32    mbfatr;         /* message buffer attribute */
        INT32     bufsz;          /* buffer size */
        INT32     maxmsz;         /* message maximum size */
        INT32     frbufsz;        /* free buffer size */
        INT32     head;           /* first message position in buffer */
        INT32     tail;           /* message end position in buffer */
        char*   buffer;         /* message buffer pointer */
        pthread_cond_t msg_cond;
	 pthread_mutex_t mutex;
} MSG_CB;
#define MSG_LIMIT_CNT 128
#define ROUND_SZ(sz)             (((sz) + (sizeof(INT32)-1)) & ~((sizeof(INT32)-1)))

static UINT32 total_msg = 0;
MSG_CB msg_st[MSG_LIMIT_CNT] = {{0,OBJ_NONEXIST,0,0,0,0,0,NULL},};
pthread_mutex_t msg_mutex;

ID OS_CreateMsgBuffer(T_CMBF *pk_cmbf)
{
	MSG_CB   *mbfcb;
	INT32     bufsz;
	UINT32	mbfid;
	UINT32	i = 0;

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
			msg_st[i].mbfid = 0;
			msg_st[i].mbfatr = OBJ_NONEXIST;
			msg_st[i].bufsz = 0;
			msg_st[i].maxmsz = 0;
			msg_st[i].frbufsz = 0;
			msg_st[i].head = 0;
			msg_st[i].tail = 0;
			msg_st[i].buffer = NULL;
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

	//if(mbfid<0)
		//return(E_FAILURE);
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

	//if(mbfid<0)
		//return(E_FAILURE);
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

BOOL mbf_free(MSG_CB* mbfcb, INT32 msgsz)
{
    return((long)(sizeof(INT32)) + msgsz <= mbfcb->frbufsz);
}


BOOL mbf_empty(MSG_CB* mbfcb)
{
    return(mbfcb->frbufsz == mbfcb->bufsz);
}


static void msg_to_mbf(MSG_CB* mbfcb, VP msg, INT32 msgsz)
{
    INT32     tail = mbfcb->tail;
    char      *buffer = mbfcb->buffer;
    INT32     remsz;

    mbfcb->frbufsz -= sizeof(INT32) + ROUND_SZ(msgsz);
    *((INT32 *) &(buffer[tail])) = msgsz;
    tail += sizeof(INT32);
    if (tail >= mbfcb->bufsz)
    {
        tail = 0;
    }

    if ((remsz = mbfcb->bufsz - tail) < msgsz)
    {
        memcpy(&(buffer[tail]), msg, remsz);
        msg = (VP)((char *)msg + remsz);
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

static INT32 mbf_to_msg(MSG_CB* mbfcb, VP msg)
{
    INT32     head = mbfcb->head;
    char      *buffer = mbfcb->buffer;
    INT32     msgsz, copysz;
    INT32     remsz;

    msgsz = *((INT32 *) &(buffer[head]));
    head += sizeof(INT32);
    if (head >= mbfcb->bufsz)
    {
        head = 0;
    }
    mbfcb->frbufsz += sizeof(INT32) + ROUND_SZ(msgsz);

    copysz = msgsz;
    if ((remsz = mbfcb->bufsz - head) < copysz)
    {
        memcpy(msg, &(buffer[head]), remsz);
        msg = (VP)((char *)msg + remsz);
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

ER OS_SendMsgTimeOut(ID mbfid, VP msg, INT32 msgsz, TMO tmout)
{
	struct timespec old_ts;
	struct timespec new_ts;
	MSG_CB   *mbfcb;
	ER      ercd = E_OK;
	UINT32 time_ms = 0;

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
				//osal_printf("n %d %d o %d %d timeout %d ms %d\n", new_ts.tv_sec, new_ts.tv_nsec
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
ER OS_GetMsgTimeOut(VP msg, INT32 *p_msgsz, ID mbfid, TMO tmout)
#endif /****APP_LOCK_DEBUG****/
{
	struct timespec ts;
	MSG_CB   *mbfcb;
	INT32     msgsz;
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
			//osal_printf("wait msg2 %d\n", tmout);
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
            osal_printf("func:%s,line:%d wait on %d\n", g_lock_debug[i].func,
                   g_lock_debug[i].line,
                   g_lock_debug[i].id);
        }
    }
}
#endif /****lock debug****/
