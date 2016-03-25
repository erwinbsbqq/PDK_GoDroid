/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: pr_semaphore.c
 *
 *  Description: Portable runtime os semaphore (linux-kernel) implementation file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#include <linux/semaphore.h>
#include "pr.h"
 
 
Int32 PR_SemInit(Semaphore *sem, Uint32 value)
{
    sema_init(sem, value);

    return PR_OS_OK;
}

Int32 PR_SemDestroy(Semaphore *sem)
{
    return PR_OS_OK;
}

Int32 PR_SemWait(Semaphore *sem, Int32 timeoutms)
{
    int rv;

    
    //rv = down_interruptible(sem);
    rv = down_timeout(sem, timeoutms);

    if (rv == 0) {
        return PR_OS_OK;
    }
    else if (rv == -EINTR) {
        PR_LOG("Process EINTR.\n");
    }

    return PR_OS_ERR;
}

Int32 PR_SemPost(Semaphore *sem)
{
    up(sem);

    return PR_OS_OK;
}




