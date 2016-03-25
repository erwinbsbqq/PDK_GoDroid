/*
 * Copyright (c) 2010, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "openmcapi.h"

#if defined(__ALI_TDS__)
#include <pr.h>
#elif defined(__ALI_LINUX__) || defined(__ALI_LINUX_KERNEL__)
#include "pr.h"
#endif

MCAPI_MUTEX MCAPI_Mutex;
MCAPI_MUTEX MCAPI_SendMutex;
Thread      MCAPI_Control_Task_TCB;
Thread      MCAPI_Control_Task_Proc_TCB;
MCAPI_COND_STRUCT       mcapi_ctrl_msg_cond;
MCAPI_BUF_QUEUE         ctrl_msg_rx_queue;
extern MCAPI_MUTEX      MCAPI_MsgMutex; /*Added by tony*/

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Init_OS
*
*   DESCRIPTION
*
*       Initializes OS specific data structures.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       MCAPI_SUCCESS
*       MCAPI_ERR_GENERAL
*
*************************************************************************/
mcapi_status_t MCAPI_Init_OS(void)
{
    Int32 rv;

     /*Add cond initial by tony*/
     ctrl_msg_rx_queue.head = ctrl_msg_rx_queue.tail = MCAPI_NULL;
     /*Enhanced by tony on 2013/10/31*/
     rv = PR_CondVarInit(&(mcapi_ctrl_msg_cond.mcapi_cond), &MCAPI_MsgMutex);

     if (rv != PR_OS_OK)
	return MCAPI_ERR_GENERAL;


    /* Create the task that will be used for receiving status
     * messages.
     */

    rv = PR_ThreadCreate(&MCAPI_Control_Task_TCB,
                        mcapi_process_ctrl_msg, NULL, 0, 0, "mcapi_msg_rvd");

    if (rv)
        return MCAPI_ERR_GENERAL;

    PR_uSleep(50000);
	
    /*Added by tony on 2013/05/23*/
    rv = PR_ThreadCreate(&MCAPI_Control_Task_Proc_TCB,
                        mcapi_process_ctrl_msg_proc, NULL, 0, 0, "mcapi_msg_proc");

    if (rv)
        return MCAPI_ERR_GENERAL;


    return MCAPI_SUCCESS;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Exit_OS
*
*   DESCRIPTION
*
*       Release OS specific data structures.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
void MCAPI_Exit_OS(void)
{
    PR_ThreadDestroy(&MCAPI_Control_Task_TCB);
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Resume_Task
*
*   DESCRIPTION
*
*       Resumes a suspended system task.
*
*   INPUTS
*
*       *request                The request structure on which the
*                               suspended task is suspended.
*
*   OUTPUTS
*
*       MCAPI_SUCCESS
*       MCAPI_ERR_GENERAL
*
*************************************************************************/
mcapi_status_t MCAPI_Resume_Task(mcapi_request_t *request)
{
    Int32 rv;

    /* If multiple requests are suspending on the same condition, we use
     * mcapi_cond_ptr. */
    if (request->mcapi_cond.mcapi_cond_ptr)
        rv = PR_CondVarNotify(request->mcapi_cond.mcapi_cond_ptr);
    else
        rv = PR_CondVarNotify(&request->mcapi_cond.mcapi_cond);

    if (rv)
        return MCAPI_ERR_GENERAL;

    return MCAPI_SUCCESS;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Suspend_Task
*
*   DESCRIPTION
*
*       Suspends a system task.
*
*   INPUTS
*
*       *node_data              A pointer to the global node data
*                               structure.
*       *request                A pointer to the request associated
*                               with the thread being suspended.
*       *mcapi_os               A pointer to the OS specific structure
*                               containing suspend/resume data.
*       timeout                 The number of milliseconds to suspend
*                               pending completion of the request.
*
*   OUTPUTS
*
*       MCAPI_SUCCESS
*       MCAPI_ERR_GENERAL
*
*************************************************************************/
mcapi_status_t MCAPI_Suspend_Task(MCAPI_GLOBAL_DATA *node_data,
                                  mcapi_request_t *request,
                                  MCAPI_COND_STRUCT *condition,
                                  mcapi_timeout_t timeout)
{
    Int32 rv = PR_OS_OK;



    /* If a request structure was passed into the routine. */
    if (request)
    {
        /* Initialize the condition variable with default parameters. */
        rv = PR_CondVarInit(&request->mcapi_cond.mcapi_cond, &MCAPI_Mutex);

        if (rv == PR_OS_OK)
        {
            /* Add the request to the queue of pending requests. */
            mcapi_enqueue(&node_data->mcapi_local_req_queue, request);
        }
    }

    if (rv == PR_OS_OK)
    {
        /* If no timeout value was specified. */
        if (timeout == (unsigned int)MCAPI_TIMEOUT_INFINITE)
        {
            rv = PR_CondVarWait(&condition->mcapi_cond, PR_TIMEOUT_INFINITE);
        }
        else
        {
            rv = PR_CondVarWait(&condition->mcapi_cond, timeout);
        }

        /* Uninitialize the condition variable. */
        rv = PR_CondVarDestroy(&condition->mcapi_cond);
    }

    if (rv)
        return MCAPI_ERR_GENERAL;

    return MCAPI_SUCCESS;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Cleanup_Task
*
*   DESCRIPTION
*
*       Terminates the current thread.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
void MCAPI_Cleanup_Task(void)
{
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Init_Condition
*
*   DESCRIPTION
*
*       Sets an OS specific condition for resuming a task in the future.
*
*   INPUTS
*
*       *os_cond                A pointer to the OS specific structure
*                               containing suspend/resume data.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
void MCAPI_Init_Condition(MCAPI_COND_STRUCT *condition)
{
    /* Initialize the condition variable with default parameters. */
    PR_CondVarInit(&condition->mcapi_cond, &MCAPI_Mutex);
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Set_Condition
*
*   DESCRIPTION
*
*       Sets an OS specific condition for resuming a task in the future.
*
*   INPUTS
*
*       *request                A request structure that will trigger
*                               a future task resume.
*       *os_cond                The condition to use for resuming the
*                               task.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
void MCAPI_Set_Condition(mcapi_request_t *request, MCAPI_COND_STRUCT *condition)
{
    request->mcapi_cond.mcapi_cond_ptr = &condition->mcapi_cond;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Clear_Condition
*
*   DESCRIPTION
*
*       Clears a previously set OS condition.
*
*   INPUTS
*
*       *request                A request structure that will trigger
*                               a future task resume.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
void MCAPI_Clear_Condition(mcapi_request_t *request)
{
    request->mcapi_cond.mcapi_cond_ptr = MCAPI_NULL;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Create_Mutex
*
*   DESCRIPTION
*
*       Creates a system mutex.
*
*   INPUTS
*
*       *mutex                  Pointer to the mutex control block.
*       *name                   Name of the mutex.
*
*   OUTPUTS
*
*       MCAPI_SUCCESS
*
*************************************************************************/
mcapi_status_t MCAPI_Create_Mutex(MCAPI_MUTEX *mutex, char *name)
{
    Int32 rv;

    rv = PR_MutexInit(mutex);

    if (rv)
        return MCAPI_ERR_GENERAL;

    return MCAPI_SUCCESS;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Delete_Mutex
*
*   DESCRIPTION
*
*       Destroyes a system mutex.
*
*   INPUTS
*
*       *mutex                  Pointer to the mutex control block.
*
*   OUTPUTS
*
*       MCAPI_SUCCESS
*       MCAPI_ERR_GENERAL
*
*************************************************************************/
mcapi_status_t MCAPI_Delete_Mutex(MCAPI_MUTEX *mutex)
{
    Int32 rv;

    rv = PR_MutexDestroy(mutex);

    if (rv)
        return MCAPI_ERR_GENERAL;

    return MCAPI_SUCCESS;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Obtain_Mutex
*
*   DESCRIPTION
*
*       Obtains a system mutex.
*
*   INPUTS
*
*       *mutex              Pointer to the mutex control block.
*
*   OUTPUTS
*
*       MCAPI_SUCCESS
*       MCAPI_ERR_GENERAL
*
*************************************************************************/
mcapi_status_t MCAPI_Obtain_Mutex(MCAPI_MUTEX *mutex)
{
    Int32 rv;

    rv = PR_Lock(mutex);

    if (rv)
        return MCAPI_ERR_GENERAL;

    return MCAPI_SUCCESS;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Release_Mutex
*
*   DESCRIPTION
*
*       Releases a system mutex.
*
*   INPUTS
*
*       *mutex              Pointer to the mutex control block.
*
*   OUTPUTS
*
*       MCAPI_SUCCESS
*       MCAPI_ERR_GENERAL
*
*************************************************************************/
mcapi_status_t MCAPI_Release_Mutex(MCAPI_MUTEX *mutex)
{
    Int32 rv;

    rv = PR_Unlock(mutex);

    if (rv)
        return MCAPI_ERR_GENERAL;

    return MCAPI_SUCCESS;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Set_RX_Event
*
*   DESCRIPTION
*
*       Sets an event indicating that data is ready to be received.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       MCAPI_SUCCESS
*
*************************************************************************/
mcapi_status_t MCAPI_Set_RX_Event(void)
{
    mcapi_rx_data();

    return MCAPI_SUCCESS;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Lock_RX_Queue
*
*   DESCRIPTION
*
*       Protect RX queue from concurrent accesses.  No work is required
*       in this routine since Linux receives data from the context of
*       the driver RX thread.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       Unused.
*
*************************************************************************/
mcapi_int_t MCAPI_Lock_RX_Queue(void)
{
    return 0;
}

/*************************************************************************
*
*   FUNCTION
*
*       MCAPI_Unlock_RX_Queue
*
*   DESCRIPTION
*
*       Protect RX queue from concurrent accesses.  No work is required
*       in this routine since Linux receives data from the context of
*       the driver RX thread.
*
*   INPUTS
*
*       cookie                  Unused.
*
*   OUTPUTS
*
*       None.
*
*************************************************************************/
void MCAPI_Unlock_RX_Queue(mcapi_int_t cookie)
{
}

void MCAPI_Sleep(unsigned int secs)
{
    PR_Sleep(secs);
}

void MCAPI_uSleep(unsigned int usecs)
{
    PR_uSleep(usecs);
}
