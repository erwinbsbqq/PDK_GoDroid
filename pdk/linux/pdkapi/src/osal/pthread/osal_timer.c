#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>

#include "queue.h"
#include "timer.h"
#include "task.h"
#include "select_timer.h"
//typedef Int32 int;
//typedef UInt32 unsigned int;

/*
int SetTimer(int tmr_id, unsigned int interval);
int ActTimer(int timer_id, BOOL bVal);
int StartTimer(int timer_id, unsigned int interval, TimerCallback callback, void *param);
int StartTimerAlarm(int timer_id, unsigned int interval, TimerCallback callback, void *param);
int StopTimer(int timer_id);
*/
int total_timer = 0;
#define TIM_EXIST 0x10000

Timer Timers[TIMER_ID_COUNT] = {{0,0,OBJ_NONEXIST,0, 0, 0,0},};
pthread_mutex_t timer_mutex;
pthread_t timer_pthread_id;

static void timer_task(void)
{
	Timer *tm_info;
	int i = 0;

	while(1)
	{
		pthread_mutex_lock(&timer_mutex);
		for(i = 0;i < TIMER_ID_COUNT;i++)
		{
			tm_info = Timers + i;
			if(tm_info->atr == TIM_EXIST)
			{
				if(tm_info->left_time > 100)
				{
					tm_info->left_time -= 100;
					continue;
				}

				pthread_mutex_unlock(&timer_mutex);
				tm_info->cb(tm_info->param);
				pthread_mutex_lock(&timer_mutex);

				//if(tm_info->atr == TIM_EXIST)
				//{
					if(tm_info->type == TIMER_ALARM)
						tm_info->atr = OBJ_NONEXIST;
					else
						tm_info->left_time = tm_info->interval;
				//}
			}
		}
		pthread_mutex_unlock(&timer_mutex);

		usleep(100000);
	}

	pthread_exit(NULL);
}

ID OS_CreateTimer (T_TIMER* pk_dalm)
{
	Timer *tmc_cb;
	ID tmo_id = INVALID_ID;

	if(NULL == pk_dalm)
	{
		return INVALID_ID;
	}
	else if(pk_dalm->callback == NULL)
		return INVALID_ID;

	if(total_timer == 0)
	{
		for(tmo_id=0; tmo_id<TIMER_ID_COUNT; tmo_id++)
		{
			Timers[tmo_id].running = 0;
			Timers[tmo_id].interval = 0;
			Timers[tmo_id].atr = OBJ_NONEXIST;
			Timers[tmo_id].starttime = 0;
			Timers[tmo_id].cb = 0;
			Timers[tmo_id].param = 0;
			Timers[tmo_id].timer_id = 0;
			Timers[tmo_id].type = 0;
			Timers[tmo_id].status = 0;
		}
		pthread_mutex_init(&timer_mutex,NULL);

		{
			const pthread_attr_t cur_attr;

			pthread_attr_init((pthread_attr_t *)&cur_attr);
			pthread_attr_setstacksize((pthread_attr_t *)&cur_attr, (size_t)0x4000);
			pthread_attr_setdetachstate((pthread_attr_t *)&cur_attr, PTHREAD_CREATE_DETACHED);
			pthread_create((pthread_t *)&timer_pthread_id, (pthread_attr_t *)&cur_attr, (void *)timer_task, NULL);
			pthread_attr_destroy((pthread_attr_t *)&cur_attr);
		}

		total_timer = 1;
	}

	pthread_mutex_lock(&timer_mutex);

	for(tmo_id=0; tmo_id<TIMER_ID_COUNT; tmo_id++)
	{
		if(OBJ_NONEXIST== Timers[tmo_id].atr
			&& 0 == Timers[tmo_id].status)
		{
			Timers[tmo_id].atr = TIM_EXIST;
			break;
		}
	}

	if(tmo_id >= TIMER_ID_COUNT)
	{
		pthread_mutex_unlock(&timer_mutex);
		return INVALID_ID;
	}

	tmc_cb = &Timers[tmo_id];
	tmc_cb->left_time = pk_dalm->time;
	tmc_cb->interval = pk_dalm->time;
	tmc_cb->cb = (TimerCallback)pk_dalm->callback;
	tmc_cb->param = pk_dalm->param;
	tmc_cb->type = pk_dalm->type;

	pthread_mutex_unlock(&timer_mutex);

#if 0
	if(pk_dalm->type == TIMER_ALARM)
	{
		StartTimerAlarm(tmo_id, tmc_cb);
	}
	else if(pk_dalm->type == TIMER_CYCLIC)
	{
		StartTimer(tmo_id, tmc_cb);
	}
	else
	{
		tmo_id= INVALID_ID;
	}
#endif

	return tmo_id;
}

ER OS_DeleteTimer(ID tmrID)
{
	int ret = 0;
	Timer *tmc_cb;

	if (tmrID>=TIMER_ID_COUNT)
		return (E_FAILURE);

	tmc_cb = &Timers[tmrID];

#if 0
	ret = StopTimer(tmc_cb);
#endif

	pthread_mutex_lock(&timer_mutex);

	tmc_cb->atr = OBJ_NONEXIST;

	pthread_mutex_unlock(&timer_mutex);

	return E_OK;
}

ER OS_SetTimer(ID tmrID,UINT32 newVal)
{
	ER ercd = E_OK;
	int ret = 0;
	Timer *tmc_cb;

	if (tmrID>=TIMER_ID_COUNT)
		return (E_FAILURE);
	//if(tmrID < 0)
		//return (E_FAILURE);
	if(newVal<1)
		return E_FAILURE;

	pthread_mutex_lock(&timer_mutex);
	if(Timers[tmrID].atr == OBJ_NONEXIST)
	{
		pthread_mutex_unlock(&timer_mutex);
		return E_FAILURE;
	}

	if(Timers[tmrID].type == TIMER_CYCLIC)
	{

		tmc_cb = &Timers[tmrID];
		tmc_cb->interval = newVal;
		tmc_cb->left_time = newVal;
	}
	else
		ercd = E_FAILURE;

	pthread_mutex_unlock(&timer_mutex);

	return E_OK;
}

ER OS_ActivateTimer(ID tmrID,int bVal)
{
	ER ercd = E_OK;
	Timer *tmc_cb;

	if (tmrID>=TIMER_ID_COUNT)
		return (E_FAILURE);
	//if(tmrID < 0)
		//return (E_FAILURE);

	pthread_mutex_lock(&timer_mutex);

	if(Timers[tmrID].atr == (unsigned long)OBJ_NONEXIST)
	{
		pthread_mutex_unlock(&timer_mutex);
		return E_FAILURE;
	}

	if(Timers[tmrID].type == TIMER_CYCLIC)
	{
		tmc_cb = &Timers[tmrID];
		tmc_cb->running = bVal;
	}
	else
		ercd = E_FAILURE;

	pthread_mutex_unlock(&timer_mutex);

	return ercd;
}

DWORD OS_GetTime(void)
{
#if 0
	return times(NULL) / sysconf(_SC_CLK_TCK);
#else
	struct timespec time;

	clock_gettime(CLOCK_REALTIME, &time);
	return(time.tv_sec);
#endif
}


/*--------------------------------------------------------------------
 * Function_Name: OS_GetTickCount
 *
 * Description:Return milliseconds from system boot up.
 *
 * Return Value:DWORD
 *
 *------------------------------------------------------------------*/
/*
DWORD OS_GetTickCount(void)
{
	DWORD cur_tick = 0;
	cur_tick = (DWORD)((times(NULL)*1000)/sysconf(_SC_CLK_TCK));
	//return (times(NULL)*1000)/sysconf(_SC_CLK_TCK);
	return cur_tick;
}
*/
DWORD OS_GetTickCount(void)
{
#if 0
	DWORD cur_tick = 0;
	if((sysconf(_SC_CLK_TCK)/1000) == 0)
		cur_tick = (DWORD)((times(NULL)*1000)/sysconf(_SC_CLK_TCK));
	else
		cur_tick = (DWORD)(times(NULL)/(sysconf(_SC_CLK_TCK)/1000));

	//return (times(NULL)*1000)/sysconf(_SC_CLK_TCK);
	return cur_tick * 3;
#else
	DWORD cur_tick;
	struct timespec time;

	clock_gettime(CLOCK_REALTIME, &time);

	cur_tick = (time.tv_sec * 1000)+ (time.tv_nsec / 1000000);

	return(cur_tick);
#endif
}

/*--------------------------------------------------------------------
 * Function_Name:
 *
 * Description:
 *
 * Return Value:
 *
 *------------------------------------------------------------------*/


//void OS_Delay(WORD us)
void OS_Delay(unsigned short us)
{
	//unsigned long tick, old_tick;
	return delay_time(us);
}

ER dly_tsk(DLYTIME dlytim)
{
	ER  ercd = E_OK;

	// If the delay time is zero, the function do nothing.
	if(dlytim==0 )
	{
		return ercd;
	}

	if (dlytim > 0)
	{
		usleep(dlytim*1000);
	}
	else
	{
		ercd = E_FAILURE;
	}

	return ercd;
}

