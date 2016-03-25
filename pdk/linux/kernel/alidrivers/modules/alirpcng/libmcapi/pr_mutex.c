/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: pr_mutex.c
 *
 *  Description: Portable runtime os mutex (linux-kernel) implementation file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#include <linux/semaphore.h>
#include "pr.h"


Int32 PR_MutexInit(Mutex *mutex)
{
    if(!mutex)
    	return PR_OS_ERR; /*Enhanced by tony on 2014/03/17*/
    sema_init(mutex, 1);

    return PR_OS_OK;
}

Int32 PR_MutexDestroy(Mutex *mutex)
{
	if(!mutex)
		return PR_OS_ERR;
	else
		return PR_OS_OK;
}

Int32 PR_Lock(Mutex *mutex)
{
    int rv;

    if(!mutex)
    	return PR_OS_ERR;
    /*Changed by tony on 2013/06/24*/
#if 0
    rv = down_interruptible(mutex);

    if (rv == 0) {
        return PR_OS_OK;
    }
    else if (rv == -EINTR) {
        PR_LOG("Process EINTR.\n");
    }

    return PR_OS_ERR;
#endif
    down(mutex);
    return PR_OS_OK;
}

Int32 PR_Unlock(Mutex *mutex)
{
	if(!mutex)
		return PR_OS_ERR;
    up(mutex);
    
    return PR_OS_OK;
}


