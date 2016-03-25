/****************************************************************************
 *
 *  ALi (shenzhen) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: pr.h
 *
 *  Description: Portable runtime OS header file.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2013.01.25      David.L         0.1.000         Initial
 ****************************************************************************/

#ifndef  __PR_H__
#define  __PR_H__

#include "pr_types.h"

#define PR_OS_OK                0
#define PR_OS_ERR               -1
#define PR_OS_TIMEOUT           62

#define PR_TIMEOUT_INFINITE     (~0)

#define PR_LOG printf
/*----------------------------------------------------------------------------*/

void *PR_Malloc(unsigned int n);
void PR_Free(void *ptr);

#if 0
void PR_uSleep(Uint32 usec);
void PR_Sleep(Uint32 sec);

Int32 PR_ThreadCreate(Thread *tid, ThreadFunc thread, void *arg, Int32 prio, Uint32 stacksize, Int8 *name);
Int32 PR_ThreadDestroy(Thread *tid);
Bool PR_ThreadTestCancel(void);

Int32 PR_MutexInit(Mutex *mutex);
Int32 PR_MutexDestroy(Mutex *mutex);
Int32 PR_Lock(Mutex *mutex);
Int32 PR_Unlock(Mutex *mutex);

Int32 PR_CondVarInit(CondVar *cvar, Mutex *mutex);
Int32 PR_CondVarDestroy(CondVar *cvar);
Int32 PR_CondVarWait(CondVar *cvar, Uint32 timeout);
Int32 PR_CondVarNotify(CondVar *cvar);

Int32 PR_SemInit(Semaphore *sem, Uint32 value);
Int32 PR_SemDestroy(Semaphore *sem);
Int32 PR_SemWait(Semaphore *sem, Int32 timeoutms);
Int32 PR_SemPost(Semaphore *sem);
#endif

#endif // __PR_H__

