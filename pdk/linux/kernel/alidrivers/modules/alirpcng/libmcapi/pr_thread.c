/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: pr_thead.c
 *
 *  Description: Portable runtime os thread (linux-kernel) implementation file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include "pr.h"


typedef struct {
     ThreadFunc  func;
     void *      arg;
}ThreadDesc_t;

/**
 *  Counter of the kernel threads created by PR_ThreadCreate.
 */
static Uint32 kthdCnter = 0;

static int thd(void *arg)
{
    ThreadDesc_t td, *p = (ThreadDesc_t *)arg;

    td.func = p->func;
    td.arg = p->arg;

    PR_Free(p);

    (td.func)(td.arg);

    return 0;
}

Int32 PR_ThreadCreate(Thread *tid, ThreadFunc thread, void *arg, Int32 prio, Uint32 stacksize, Int8 *name)
{
    ThreadDesc_t *td;

    if (!tid) {
        PR_LOG("Thread Creation thread id error.\n");
        return PR_OS_ERR;
    }

    td = (ThreadDesc_t *)PR_Malloc(sizeof(ThreadDesc_t));
    if (!td) {
        PR_LOG("Thread Creation MALLOC error.\n");
        return PR_OS_ERR;
    }

    td->func = thread;
    td->arg = arg;
    if(name)
	*tid = kthread_run(thd, td, name);
    else
    	*tid = kthread_run(thd, td, "PR_thread%d", ++kthdCnter);

    if (IS_ERR(*tid)) {
        PR_LOG("Thread Creation kthread_run error.\n");
        *tid = NULL;
        PR_Free(td);
        return PR_OS_ERR;
    }
    
    return PR_OS_OK;
}

Int32 PR_ThreadDestroy(Thread *tid)
{
    int rv;

    if (!tid) {
        PR_LOG("Thread Destroy thread id error.\n");
        return PR_OS_ERR;
    }

    rv = kthread_stop(*tid);
    if (rv == 0) {
        return PR_OS_OK;
    }
    else if (rv == -EINTR) {
        PR_LOG("Process EINTR.\n");
        return PR_OS_ERR;
    }

    return PR_OS_ERR;
}

/** Linux-kernel has a test for the running thread, using it to stop a kthread.
 */
Bool PR_ThreadTestCancel(void)
{
    return (Bool)kthread_should_stop();
}


