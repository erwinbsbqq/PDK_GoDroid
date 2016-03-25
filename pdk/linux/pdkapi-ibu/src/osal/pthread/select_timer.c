
/*
* CTimer.h
*
* Created on: 2009-7-13
*      Author: DEAN
*/

//////////////////////////////////////////////////////////////////////////
// This class provide a timer to finish some works.
// Call SetTimer() to set the timer_interval. Call StartTimer()
// to enable it and call StopTimer() to stop it.
// The work you want to do should be written on OnTimer
// function.
//////////////////////////////////////////////////////////////////////////




/*
* CTimer.c
*
* Created on: 2009-7-13
*      Author: DEAN
*/

//#include "CTimer.h"
#include <time.h>
//#include <iostream.h>
#include <sys/select.h>
#include <pthread.h>
#include "select_timer.h"

#if 0
#define osal_printf printf
#else
#define osal_printf(...)	do{}while(0)
#endif

#define OK (0)
#define ERROR (-1)


#define TRUE 1
#define FALSE 0

#define E_OK               0
#define E_FAILURE		(-1)
#define E_TIMEOUT	(-2)

/*
typedef enum{
	TIMER_ID_NULL = -1,
	TIMER_ID_GUI = 0,
	//add new TIMER after this line
	TIMER_ID_COUNT,
}TIMER_ID_ENUM;
*/
//typedef  Int (*TimerCallback)(void *param);
 

int m_second = 0;
int m_microsecond = 0;
//pthread_t thread_timer;

#if 0
#define TRUE 1
#define FALSE 0
#endif

extern Timer Timers[TIMER_ID_COUNT];
extern pthread_mutex_t timer_mutex;

#if 0
//////////////////////////private methods//////////////////////////
void *thread_proc(int timer_id)
{
	TimerCallback callback;
	int running=FALSE;
	unsigned long param;
	struct timeval tempval;
	unsigned short cur_cyc_id = 0;
	cur_cyc_id = (unsigned short)timer_id;
	while (TRUE)
	{
		//pthread_testcancel();
		
		pthread_mutex_lock(&timer_mutex);		
		tempval.tv_sec = /*m_second;*/Timers[cur_cyc_id].interval/1000;
		tempval.tv_usec =/* m_microsecond;*/(Timers[cur_cyc_id].interval%1000) * 1000;
		callback = Timers[cur_cyc_id].cb;
		running = Timers[cur_cyc_id].running;
		param = Timers[cur_cyc_id].param;
		pthread_mutex_unlock(&timer_mutex);
	
		select(0, NULL, NULL, NULL, &tempval);
		if(running)
			callback(param);
	}
}

void *thread_proc_alarm(int timer_id)
{
	struct timeval tempval;
	unsigned short cur_alarm_id = 0;
	cur_alarm_id = (unsigned short)timer_id;
	TimerCallback callback;
	unsigned long param;
	tempval.tv_sec = /*m_second;*/Timers[cur_alarm_id].interval/1000;
	tempval.tv_usec =/* m_microsecond;*/(Timers[cur_alarm_id].interval%1000) * 1000;
	param = Timers[cur_alarm_id].param;
	callback = Timers[cur_alarm_id].cb;
	
	select(0, NULL, NULL, NULL, &tempval);
	
	pthread_mutex_lock(&timer_mutex);
	Timers[cur_alarm_id].status = 1;
	pthread_mutex_unlock(&timer_mutex);
		
	callback(param);

	pthread_mutex_lock(&timer_mutex);
	if(Timers[cur_alarm_id].status == 2)
		Timers[cur_alarm_id].status = 0;
	pthread_mutex_unlock(&timer_mutex);	

	osal_printf("exit alarm %d\n", timer_id);	
	
	pthread_exit(NULL);
}

/*
int SetTimer(int tmr_id, unsigned int interval)
{
	//m_second = second;
	//m_microsecond = microsecond;
	Timers[tmr_id].interval = interval;
	return 0;
}

int ActTimer(int timer_id, BOOL bVal)
{
	Timers[timer_id].running = bVal;
	return 0;
}
*/
unsigned short tmp_alram_id = 0;
unsigned short tmp_cyc_id = 0;

int StartTimer(unsigned short timer_id, Timer *tmr_cb)
{
	//SetTimer(m_sec, m_micro_sec);
	tmr_cb->running = FALSE;
	tmr_cb->status = 0;

	osal_printf("start timer %d\n", timer_id);
	pthread_create(&tmr_cb->timer_id, NULL, &thread_proc, (void *)timer_id);
	//Timers[timer_id].timer_id = thread_timer;
	
	return 0;
}

int StartTimerAlarm(unsigned short timer_id, Timer *tmr_cb)
{
	int ret = 0;
	
	tmr_cb->status = 0;	
	//SetTimer(m_sec, m_micro_sec);
	//Timers[timer_id].timer_id = thread_timer;
	//Timers[timer_id].cb = tmr_cb->cb;
	//tmr_cb->running = FALSE;
	//Timers[timer_id].interval = tmr_cb->interval;
	//Timers[timer_id].param = tmr_cb->param;
	printf("start alarm id %d\n", timer_id);
	ret = pthread_create(&tmr_cb->timer_id, NULL, &thread_proc_alarm, (void *)timer_id);
	if(ret == 0)
		pthread_detach(tmr_cb->timer_id);
	
	
	return 0;
}

int StopTimer(Timer *tmc_cb)
{
	int ret = E_FAILURE;
	
	pthread_t thread_timer;
	thread_timer =tmc_cb->timer_id;
	if(thread_timer != 0)
	{
		pthread_mutex_lock(&timer_mutex);
		
		if(tmc_cb->status == 1)
		{
			tmc_cb->status = 2;
			
			pthread_mutex_unlock(&timer_mutex);			
			return E_OK;
		}
		
		tmc_cb->status = 0;
		pthread_mutex_unlock(&timer_mutex);

		osal_printf("stop timer\n");		
		ret = pthread_cancel(thread_timer);
		ret = pthread_join(thread_timer, NULL); //wait the thread stopped
	}
	
	return ret;
}
#endif

void delay_time(unsigned short us)
{
	struct timeval ts;
	struct timeval old_ts;
	gettimeofday(&old_ts,0);

	gettimeofday(&ts,0);
	while((ts.tv_usec - old_ts.tv_usec) < (unsigned int)us)
	{
		gettimeofday(&ts,0);
	}
	return;
}

/*
void OnTimer()
{
    cout<<"Timer once..."<<endl;
}
*/


/*
* main.cpp
*
* Created on: 2009-7-19
*      Author: DEAN


#include <iostream>
#include "CTimer.h"

using namespace std;

int main()
{
	CTimer t1(1,0),t2(1,0);    //构造函数，设两个定时器，以1秒为触发时间。参数1是秒，参数2是微秒。
	t1.StartTimer();
	t2.StartTimer();
	sleep(10);
	return 0;
}

*/
 

