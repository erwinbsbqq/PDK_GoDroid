/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: pr_cond.c
 *
 *  Description: Portable runtime os condition variable (linux-kernel) implementation file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#include <linux/completion.h>
#include <linux/jiffies.h>
#include "pr.h"

Int32 PR_CondVarInit(CondVar *cvar, Mutex *mutex)
{
    if (!cvar) {
        PR_LOG("CondVar Init error.\n");
        return PR_OS_ERR;
    }

    if (!mutex) {
        PR_LOG("CondVar Init mutex assignment error.\n");
        return PR_OS_ERR;
    }
    
    cvar->mutex = mutex;    
    /*Added by tony*/
    cvar->flag = 0;
    /*Added end*/

	init_completion(&cvar->cv);

    return PR_OS_OK;
}

Int32 PR_CondVarDestroy(CondVar *cvar)
{
    cvar->mutex = 0;
    
    return PR_OS_OK;
}

/**
 * timeout              - The number of milliseconds to wait.
 */
Int32 PR_CondVarWait(CondVar *cvar, Uint32 timeout)
{
    long rv;
    unsigned long j;

    if (!cvar) {
        PR_LOG("CondVar Wait error.\n");
        return PR_OS_ERR;
    }

    if (!cvar->mutex) {
        PR_LOG("CondVar Wait mutex empty error.\n");
        return PR_OS_ERR;
    }

    /*Added by tony*/
    if(cvar->flag&0x02){
	/*Means this Condnotify was called first! */
	cvar->flag = 0;
	return PR_OS_OK;	
    }
    else{
	/*Set Cond waitting flag*/
	cvar->flag = 0x01;
    }
    /*Added end*/

    PR_Unlock(cvar->mutex);

    if (timeout == PR_TIMEOUT_INFINITE) {
	/*Changed by tony for avoding the interrupt impact*/
        //rv = wait_for_completion_interruptible(&cvar->cv);
        wait_for_completion(&cvar->cv);
	    rv = PR_OS_OK;
	    PR_Lock(cvar->mutex);
	    cvar->flag = 0;
	    return rv;

    /*    if (rv < 0) {
            PR_Lock(cvar->mutex);
            PR_LOG("Process EINTR.\n");
	    cvar->flag = 0;
            return PR_OS_ERR;
        }
        else if (rv == 0) {
            PR_Lock(cvar->mutex);
	    cvar->flag = 0;
            return PR_OS_OK;
        }
     */
    }
    else {
        j = msecs_to_jiffies(timeout);

	/*Changed by tony for avoding the interrupt impact*/
        //rv = wait_for_completion_interruptible_timeout(&cvar->cv, j);
        rv = wait_for_completion_timeout(&cvar->cv, j);

       /* if (rv < 0) {
            PR_LOG("Process EINTR.\n");
            PR_Lock(cvar->mutex);
	    cvar->flag = 0;
            return PR_OS_ERR;
        }*/
        if (rv == 0) {
            PR_Lock(cvar->mutex);
	    cvar->flag = 0;
            return PR_OS_TIMEOUT;
        }
    }

    PR_Lock(cvar->mutex);
    cvar->flag = 0;

    return PR_OS_OK;
}

Int32 PR_CondVarNotify(CondVar *cvar)
{
    if (!cvar) {
        PR_LOG("CondVar Notify error.\n");
        return PR_OS_ERR;
    }


   if(cvar->flag & 0x01){
    /*Changed by tony*/
    //complete(&cvar->cv);
    complete_all(&cvar->cv);
   }

   /*Set Cond Notify flag*/   
   cvar->flag |= 0x02;

    return PR_OS_OK;
}



