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
#define FLG_LIMIT_CNT 256

UINT32 total_flag = 0;

typedef struct flag_control_block
{
    ID			    flgid;
    UINT32     		flgatr;
    UINT32    		flgptn;//current flag pattern
    pthread_mutex_t uni_mutex;
    pthread_cond_t	flg_cond;
} FLG_CB;
FLG_CB flg_st[FLG_LIMIT_CNT] = {{0,OBJ_NONEXIST, 0},};


ID  OS_CreateFlag(UINT32 flgptn)
{
	struct timespec to;
	ID flgid;
	FLG_CB   *flgcb;
	UINT32 i = 0;
	UINT32 j = 0;

  	if(total_flag ==0)
	{
		total_flag = 1;
		for( i = 0; i < FLG_LIMIT_CNT; i++)
		{
			flg_st[i].flgid = INVALID_ID;
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
	UINT32 i = 0;
	FLG_CB   *flgcb;
	ER      ercd = E_OK;

	//if (flgid < 0) return(E_FAILURE);
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


ER OS_SetFlag(ID flgid, UINT32 setptn)
{
	UINT32 i = 0;
	FLG_CB   *flgcb;
	ER      ercd = E_OK;

	//if (flgid < 0) return(E_FAILURE);
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



ER OS_ClearFlag(ID flgid, UINT32 clrptn)
{
	FLG_CB   *flgcb;
   	ER      ercd = E_OK;

	//if (flgid<0) return(E_FAILURE);
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


static inline BOOL flag_cond_satis(UINT32 flag_ptn, UINT32 waiptn, UINT32 wfmode)
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
ER OS_WaitFlagTimeOut(UINT32 *p_flgptn, ID flgid, UINT32 waiptn, UINT32 wfmode, TMO tmout, char *func, int line)
#else
ER OS_WaitFlagTimeOut(UINT32* p_flgptn, ID flgid, UINT32 waiptn, UINT32 wfmode, TMO tmout)
#endif /****APP_LOCK_DEBUG****/
{
	struct timespec old_ts;
	struct timespec new_ts;
	struct timespec ts;
	UINT32 flg_ptn;
	int err_ret = -1;
	ER	ercd = E_FAILURE;
	UINT32 i = 0;
	UINT32 twf_or_flg = 0;
	UINT32 time_ms = 0;

	FLG_CB   *flgcb;

//20150527 modified to resolve logic issues.
//if user not set *p_flgptn, and OS_WaitFlagTimeOut() timeout,
//the *p_flgptn will have a wild value.
	if (p_flgptn != NULL)
	{
		*p_flgptn = 0;
	}

	//if (flgid<0) return(E_FAILURE);
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
			//osal_printf("wait msg2 %d\n", tmout);
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

