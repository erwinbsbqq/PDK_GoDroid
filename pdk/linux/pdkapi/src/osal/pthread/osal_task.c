#include <hld_cfg.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <hld_cfg.h>


#define _GNU_SOURCE        /* or _BSD_SOURCE or _SVID_SOURCE */

#include "queue.h"
#include "timer.h"
#include "task.h"

#define _GNU_SOURCE

#define PTH_NONEXIST		(INT32)0xFFFFFFFF
#if 0
#define OSAL_PRINT(fmt, args...) ADR_DBG_PRINT(OSAL, fmt, ##args)
#else
#define OSAL_PRINT(...)	do{}while(0)
#endif
#define OSAL_WAIT_FOREVER_TIME		0xFFFFFFFF

typedef struct {
	int tgid;            // (special)task id, the POSIX thread ID (see also: tgid)
	char name[64];      // thread or process name
	int ppid,           // 父进程ID
	    pgrp,           // 线程组ID
	    session;        // 会话组ID
	unsigned  pcpu;     // stat (special) %CPU usage (is not filled in by readproc!!!)
	char 	state;		// stat,status single-char code for process state (S=sleeping)
	UINT64 utime,		// stat user-mode CPU time accumulated by process
	       stime,		// stat kernel-mode CPU time accumulated by process// and so on...
	       cutime,		// stat cumulative utime of process and reaped children
	       cstime,		// stat cumulative stime of process and reaped children
	       start_time;	// stat start time of process -- seconds since 1-1-70
	long priority,      // 进程的动态优先级
		 nice;          // 进程的静态优先级
} proc_task_stat_t;

typedef struct
{
   UINT64 user,        // 用户态的运行时间，不包含 nice值为负进程
   		  nice,        // nice值为负的进程所占用的CPU时间
          system,      // 核心态的运行时间
          idle,        // 除IO等待时间以外的其它等待时间
          iowait,      // IO等待时间
          irq,         // 硬中断时间
          softirq,     // 软中断时间
          stealstolen, /* the time spent in other operating systems when running
                        * in a virtualized environment */
          guest; /* the time spent running a virtual CPU for guest operating systems
          		  * under the control of the Linux kernel */
} proc_stat_t;
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
	pthread_t pthid;       /* thread pthread_t */
	pid_t   pid;           /* thread pid */	
	UINT64  cpu_tm_saved;  /* save cpu time of last time */
	UINT64  task_tm_saved; /* save thread time of last time */
	char    pname[3];      /* thread name */
} PTH_CB;
UINT32 	total_pth= 0;
#define THREAD_MAX_CNT 128
PTH_CB pth_st[THREAD_MAX_CNT] = {{PTH_NONEXIST, (pthread_t)0x0, 0, 0, 0, 0},};


//#define _SHOW_TASK_PID_

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
    OSAL_PRINT(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>task %s -> PID = %d\n", task_name, pid);
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
            OSAL_PRINT("*****task %d exit ...\n", taskid);
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
    OSAL_PRINT("CMD: %s\n", cmd);
    f(p1, p2);
    ///////////////////////////////
    _task_exit();
}
#endif /****FOR DEBUG****/
ID_THREAD OS_CreateThread(T_CTHD * pk_cthd)
{
	int ret = 0;
	//pthread_t pth_id;
	pthread_attr_t cur_attr;
	ID_THREAD 	thdid;
	ER 	ercd=E_OK;
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
			pth_st[i].state=PTH_NONEXIST;
			pth_st[i].pthid=INVALID_ID;
			pth_st[i].pid=0;
			memset(pth_st[i].pname, 0, sizeof(pth_st[i].pname));
			pth_st[i].cpu_tm_saved = 0;
			pth_st[i].task_tm_saved = 0;
		}
	}

	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		if(pth_st[thdid].state == PTH_NONEXIST)
			break;
	}
	if (thdid>=THREAD_MAX_CNT)
	{
        OSAL_PRINT("*** Up to MAX pth_st num!!!\n");
		return INVALID_ID;
	}
	pth_st[thdid].state = 0;
	pth_st[thdid].th_id = thdid;
    memcpy(pth_st[thdid].pname, pk_cthd->name, 3);
	pthread_attr_init(&cur_attr);
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
	ret = pthread_create(&pth_st[thdid].pthid, (pthread_attr_t *)&cur_attr, (void *)(pk_cthd->task), (void *)pk_cthd->para1);

#if (defined(CONFIG_TIMEDOCTOR) || defined(TD_IN_MODULE))
    {
        char name[4];
        memcpy(name, pk_cthd->name, 3);
        name[3] = 0;
        td_setName(TDI_TASK, getPthreadPid(pth_st[thdid].pthid), name);
    }
#endif
    pthread_attr_destroy (&cur_attr);
	if(ret)
	{
		OSAL_PRINT("create pthread failed. ret_value: %d\n", ret);
		return INVALID_ID;
	}
	
	OSAL_PRINT("pthread_id create: %u, task[%d]\n", pth_st[thdid].pthid, thdid);	

	return (thdid);
}

ER OS_DeleteThread(ID_THREAD thd_id)
{
	pthread_t pth_id;
	ER 	ercd=E_OK;
	int err_ret = 0;

	OSAL_PRINT("%s: %d\n", __FUNCTION__, thd_id);

	if(thd_id<THREAD_MAX_CNT)
	{
		pth_id = pth_st[thd_id].pthid;
		pth_st[thd_id].pthid=INVALID_ID;
		pth_st[thd_id].state=PTH_NONEXIST;
		pth_st[thd_id].pid=0;
		memset(pth_st[thd_id].pname, 0, sizeof(pth_st[thd_id].pname));
		pth_st[thd_id].cpu_tm_saved = 0;
		pth_st[thd_id].task_tm_saved = 0;
	}
	else
	{
		return(E_FAILURE);
	}
		

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
	UINT16 thdid = 0;
	pthread_t cur_pth_id;

	cur_pth_id = pthread_self();
	OSAL_PRINT("%s pth_id: 0x%08x\n", __FUNCTION__, cur_pth_id);

	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		if((pth_st[thdid].state != PTH_NONEXIST) && (cur_pth_id == pth_st[thdid].pthid))
			break;
	}
	if (thdid < THREAD_MAX_CNT)
	{
		OSAL_PRINT("%s th_id: %d\n", __FUNCTION__, thdid);
		return thdid;
	}

	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		if(pth_st[thdid].state == PTH_NONEXIST)
			break;
	}

	if (thdid >= THREAD_MAX_CNT)
	{
		OSAL_PRINT("Not enough thread control structure.\n");
		return INVALID_ID;
	}

	pth_st[thdid].state=0;
	pth_st[thdid].pthid=cur_pth_id;

	OSAL_PRINT("%s th_id: %d\n", __FUNCTION__, thdid);
	return (ID)thdid;
}

void OS_SaveThreadInfo(const char *thread_name)
{
	unsigned int pid = 0;
	UINT16 thdid = 0;

	/* get thread pid */
	pid = syscall(__NR_gettid);
	if (pid == 0)
		OSAL_PRINT("Get task(%s) pid failed.\n");
	/*
	else // remove ugly print message
		OSAL_PRINT(">>>Task=%s -> PID=%d\n", thread_name, pid);
	*/

	/* get thread pthread_t */
	pthread_t cur_pth_id;
	cur_pth_id = pthread_self();

	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		if((pth_st[thdid].state != PTH_NONEXIST) && (cur_pth_id == pth_st[thdid].pthid))
			break;
	}

	if (thdid < THREAD_MAX_CNT)
	{
		pth_st[thdid].pid = pid;
		strncpy(pth_st[thdid].pname, thread_name, sizeof(pth_st[thdid].pname));
		return;
	}

	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		if(pth_st[thdid].state == PTH_NONEXIST)
			break;
	}

	if (thdid >= THREAD_MAX_CNT)
	{
		OSAL_PRINT("Not enough thread control structure.\n");
		return;
	}

	pth_st[thdid].state=0;
	pth_st[thdid].pthid=cur_pth_id;
	pth_st[thdid].pid=pid;
	strncpy(pth_st[thdid].pname, thread_name, sizeof(pth_st[thdid].pname));

	return;
}

/* Get process pid or main thread in process
 * INPUT:
 *   process_name : NULL -> be invoked within current process
 *                  "STB" -> get linux_stb main thread pid
 *      Notice: Must be invoked after call OS_SaveThreadInfo when is not NULL
 * RETURN:
 *   ID : process pid value */
ID OS_GetProcessPID(const char *process_name)
{
	pid_t pid = 0;
	UINT16 thdid = 0;

	if (!process_name)
	{
		pid = getpid();
		return pid;
	}

	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		if(!strcasecmp(process_name, pth_st[thdid].pname))
			break;
	}

	if (thdid >= THREAD_MAX_CNT)
	{
		OSAL_PRINT("Not found this process(%s).\n", process_name);
		return pid; /* zero means invalid process pid */
	}

	pid = pth_st[thdid].pid;

	return pid;
}

/* Get pthread pid
 * INPUT:
 *   thread_name : NULL -> be invoked within current thread
 *                 "xxx" -> get thread(xxx) pid
 *      Must be invoked after call OS_SaveThreadInfo when is not NULL
 * RETURN:
 *   ID : process pid value */
ID OS_GetThreadPID(const char *thread_name)
{
	pid_t pid = 0;
	UINT16 thdid = 0;

	if (!thread_name)
	{
		pid = syscall(__NR_gettid);
		return pid;
	}

	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		if(!strcasecmp(thread_name, pth_st[thdid].pname))
			break;
	}

	if (thdid >= THREAD_MAX_CNT)
	{
		OSAL_PRINT("Not found this thread(%s).\n", thread_name);
		return pid; /* zero means invalid thread pid */
	}

	pid = pth_st[thdid].pid;

	return pid;
}

/* Must be invoked after call OS_SaveThreadInfo */
char *OS_GetThreadName(pid_t pid)
{
	UINT16 thdid = 0;

	if (pid <= 0)
	{
		OSAL_PRINT("Pid is invalid.\n");
		return NULL;
	}

	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		if(pth_st[thdid].pid == pid)
			break;
	}

	if (thdid >= THREAD_MAX_CNT)
	{
		OSAL_PRINT("Not found this thread(%d).\n", pid);
		return NULL; /* NULL means invalid thread pid */
	}

	return pth_st[thdid].pname;
}

/* Reference: task state in Linux kernel */
#if 0
static const char *task_state_array[] = {
    "R (running)",      /*  0 */
    "S (sleeping)",     /*  1 */
    "D (disk sleep)",   /*  2 */
    "T (stopped)",      /*  4 */
    "T (tracing stop)", /*  8 */
    "Z (zombie)",       /* 16 */
    "X (dead)"      /* 32 */
};
#endif

/* Notice:
 * if pid = 0, then use current thread pid
 * if tgid = 0, then use current process pid
 */
char *OS_GetThreadState(pid_t pid, pid_t tgid)
{
	char status_file_path[32];
	char line[64], state[8], detail[24];
	FILE *pfd = NULL;
	static char result[32];
    unsigned int cmdlen;

	/* if pid is zero means current pthread pid */
	if (pid == 0)
		pid = syscall(__NR_gettid);

	/* group id or linux_stb process pid */
	if (tgid == 0)
		tgid = OS_GetProcessPID(NULL);

	/* Obtain thread status file path */
	sprintf(status_file_path, "/proc/%d/task/%d/status", tgid, pid);

	/* Open thread status file */
	pfd = fopen(status_file_path, "r");
	if (!pfd) {
		OSAL_PRINT("Open file %s failed.\n", status_file_path);
		return NULL;
	}

	/* Read file line by line */
    while (!feof(pfd)) {

        fgets(line, sizeof(line), pfd);
        if (strlen(line) <= sizeof(line))
        {
            cmdlen = strlen(line);
        }
        else
        {
            cmdlen = sizeof(line);
        }

		line[cmdlen-1] = '\0';

		if ((strlen(line) > strlen("State")) &&
				(strncmp(line, "State", strlen("State")) == 0))
		{
			sscanf(line, "State:\t%s (%s", state, detail);
			break;
		}
    }

	fclose(pfd);

	/* Integrate full state information */
	sprintf(result, "%s (%s", state, detail);

	return result;
}

/* Notice:
 * if pid = 0, then use current thread pid
 * if tgid = 0, then use current process pid
 */
INT32 OS_GetThreadPriority(pid_t pid, pid_t tgid)
{
	proc_task_stat_t task_info;
	UINT32 priority;
	FILE *pfd = NULL;
	char stat_file_path[32];
    UINT32 temp32[10];
    UINT64 temp64[10];
	int i = 0, j = 0;

	/* if pid is zero means current pthread pid */
	if (pid == 0)
		pid = syscall(__NR_gettid);

	/* group id or linux_stb process pid */
	if (tgid == 0)
		tgid = OS_GetProcessPID(NULL);

	/* Obtain thread status file path */
	sprintf(stat_file_path, "/proc/%d/task/%d/stat", tgid, pid);

	/* Open thread status file */
	pfd = fopen(stat_file_path, "r");
	if (!pfd) {
		OSAL_PRINT("Open file %s failed.\n", stat_file_path);
		return -1; // invalid priority
	}

	/* Read file with format */
	fscanf(pfd,
        "%d (%s \
        %c %d %d %d \
        %d %d %u \
        %Lu %Lu %Lu %Lu \
        %Lu %Lu %Lu %Lu \
        %d %d",
        &task_info.tgid, &task_info.name,
        &task_info.state, &task_info.ppid, &task_info.pgrp, &task_info.session,
        &temp32[i+1], &temp32[i+2], &temp32[i+3],
        &temp64[j+1], &temp64[j+2], &temp64[j+3], &temp64[j+4],
        &task_info.utime, &task_info.stime, &task_info.cutime, &task_info.cstime,
        &task_info.priority, &task_info.nice);

	fclose(pfd);

	/* get task dynamic priority */
	priority = task_info.priority;

	return priority;
}

/* Notice:
 * if pid = 0, then use current thread pid
 * if tgid = 0, then use current process pid
 */
INT32 OS_SetThreadPriority(pid_t pid, pid_t tgid)
{
	;//TBD
}

/* Parameters:
 * IN : pid - pthread id
 *      tgid - group id
 * OUT: cpu_tm - spend cpu time
 *      task_tm - thread spend time
 * Notice:
 *  if pid = 0, then use current thread pid
 *  if tgid = 0, then use current process pid
 *
 */
INT32 OS_GetThreadCPUTime(pid_t pid, pid_t tgid, UINT64 *cpu_tm, UINT64 *task_tm)
{
	proc_task_stat_t task_info;
	proc_stat_t cpu_info;
	FILE *pfd = NULL;
	char stat_file_path[32], cpu_file_path[32];
    UINT32 temp32[10];
    UINT64 temp64[10];
	int i = 0, j = 0;

	/* if pid is zero means current pthread pid */
	if (pid == 0)
		pid = syscall(__NR_gettid);

	/* group id or linux_stb process pid */
	if (tgid == 0)
		tgid = OS_GetProcessPID(NULL);


	/* Obtain thread status file path */
	sprintf(stat_file_path, "/proc/%d/task/%d/stat", tgid, pid);

	/* Open thread status file */
	pfd = fopen(stat_file_path, "r");
	if (!pfd) {
		OSAL_PRINT("Open file %s failed.\n", stat_file_path);
		return -1; // invalid priority
	}

	/* Read file with format */
	fscanf(pfd,
        "%d (%s \
        %c %d %d %d \
        %d %d %u \
        %Lu %Lu %Lu %Lu \
        %Lu %Lu %Lu %Lu \
        %d %d",
        &task_info.tgid, &task_info.name,
        &task_info.state, &task_info.ppid, &task_info.pgrp, &task_info.session,
        &temp32[i+1], &temp32[i+2], &temp32[i+3],
        &temp64[j+1], &temp64[j+2], &temp64[j+3], &temp64[j+4],
        &task_info.utime, &task_info.stime, &task_info.cutime, &task_info.cstime,
        &task_info.priority, &task_info.nice);

	fclose(pfd);

	/* caculate task spend time */
	*task_tm = task_info.utime + task_info.stime + task_info.cutime + task_info.cstime;


	/* Obtain cpu status file path */
	sprintf(cpu_file_path, "/proc/stat");

	/* Open cpu status file */
	pfd = fopen(cpu_file_path, "r");
	if (!pfd) {
		OSAL_PRINT("Open file %s failed.\n", cpu_file_path);
		return -1; // invalid priority
	}

	/* Read file with format */
	fscanf(pfd,
    	    "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
			&cpu_info.user,
			&cpu_info.nice,
			&cpu_info.system,
			&cpu_info.idle,
			&cpu_info.iowait,
			&cpu_info.irq,
			&cpu_info.softirq,
			&cpu_info.stealstolen,
			&cpu_info.guest);

	fclose(pfd);

	/* caculate task spend time */
	*cpu_tm = cpu_info.user + cpu_info.nice + cpu_info.system +
				cpu_info.idle + cpu_info.iowait + cpu_info.irq +
				cpu_info.softirq + cpu_info.stealstolen + cpu_info.guest;
}

/* show all thread info
 * NOTICE: only show linux_stb application, not a common api
 */
void OS_ShowThreadInfo(void)
{
	UINT16 thdid = 0;
	UINT64 cpu_tm = 0;
	UINT64 task_tm = 0;
	INT32 spend_cpu_tm, spend_task_tm;

	OSAL_PRINT("\tID\tPID\tName\tStatus\tPriority\tCPU\n");

	/* print all thread info */
	for(thdid = 0; thdid < THREAD_MAX_CNT; thdid++)
	{
		/* exclude not existing thread or invalid pid thread */
		if (pth_st[thdid].state == PTH_NONEXIST || pth_st[thdid].pid == 0)
			break;

		/* get current thread spend task time and total cpu time */
		OS_GetThreadCPUTime(pth_st[thdid].pid, 0, &cpu_tm, &task_tm);

		/* caculate act spend time based on last time */
		spend_cpu_tm = (INT32)(cpu_tm - pth_st[thdid].cpu_tm_saved);
		spend_task_tm = (INT32)(task_tm - pth_st[thdid].task_tm_saved);

		/* save current cpu time and task time */
		pth_st[thdid].cpu_tm_saved = cpu_tm;
		pth_st[thdid].task_tm_saved = task_tm;

		/* print out detail message */
		OSAL_PRINT("\t%d\t%d\t%s\t%s\t%d\t%3d.%d%%\n",
				thdid,
				pth_st[thdid].pid, pth_st[thdid].pname,
				OS_GetThreadState(pth_st[thdid].pid, 0),
				OS_GetThreadPriority(pth_st[thdid].pid, 0),
				(1000*spend_task_tm/spend_cpu_tm)/10,
				(1000*spend_task_tm/spend_cpu_tm)%10);
	}
}
