/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *
 *  File: ali_rpc_invoker.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_invoker.h>

TaskDesc svcQtask[SVC_QUEUE_SIZE];      /*Change the name from "SvcQ" to "svcQtask"*/
//static TaskDesc InvokerTask[RPC_INVOKER_MAX];  /*!!Doesn't need this now, Masked by tony*/
InvokerManager invokerManager;
extern HashTable *g_ht;
extern Int32   g_RPCRunning;
extern McapiConn       g_mcapiconn_send[RPC_CHANNEL_MAX];
extern Uint32 g_TotalRcvBadFuncIDCall;
extern Uint32  g_TotalRunningInvoker;
extern Uint32  g_TotalSleepInvoker;
extern Int32 PR_ThreadCreate(Thread *tid, ThreadFunc thread, void *arg, Int32 prio, Uint32 stacksize, Int8 *name);
extern Uint32  g_TotalserviceQtask;

void InvokerConvertParams(Param **parg,  Param *constarg,  Uint8 *param)
{
    memcpy((void *)&constarg->type, &param[0], sizeof(Long));
    memcpy((void *)&constarg->paramId, &param[sizeof(Long)], sizeof(Long));
    memcpy((void *)&constarg->len, &param[sizeof(Long) + sizeof(Long)], sizeof(Size_t));
    if (constarg->type < PARAM_IN || constarg->type >= PARAM_INVALID || constarg->len == 0)
    {
        *parg = NULL;
    }
    else
    {
        constarg->pData = &param[sizeof(Long) + sizeof(Long) + sizeof(Size_t)];
        *parg = constarg;
    }

}

static void *InvokerSafeGetCallbackF(void *key)
{
    /*Doesn't need mutex lock now because the callback functions table are created staticly*/
    return (void *)HashTableLookupConst(g_ht, key);

}

static void InvokerSafeAddToList(CList *list, CList *e)
{
    PR_Lock(&invokerManager.mutex);

    LIST_APPEND_LINK(e, list);

    PR_Unlock(&invokerManager.mutex);
}

static Invoker *InvokerSafeGetFromList(CList *list)
{
    CList *e = NULL;

    PR_Lock(&invokerManager.mutex);

    e = LIST_HEAD(list);
    if (e)
    {
        LIST_REMOVE_LINK(e);
    }

    PR_Unlock(&invokerManager.mutex);

    return (Invoker *)e;        /*From Clist* conver to Invoker* because Clist is the first element of Invoker*/
}

static void InvokerSafeRemoveFromList(CList *e)
{
    PR_Lock(&invokerManager.mutex);

    LIST_REMOVE_LINK(e);

    PR_Unlock(&invokerManager.mutex);
}

static Int32 InvokerSafeCheckListEmpty(CList *list)
{
    Int32   ret = 0;

    PR_Lock(&invokerManager.mutex);
    ret = LIST_IS_EMPTY(list);
    PR_Unlock(&invokerManager.mutex);

    return  ret;

}

static Int32 InvokerSafeCheckTaskQEmpty(void)
{
    Int32 ret = 0;

    PR_Lock(&invokerManager.mutex);

    ret = QueueEmpty(&invokerManager.svcQueue);

    PR_Unlock(&invokerManager.mutex);

    return  ret;
}

static Int32 InvokerSafeAddTaskToQueue(TaskDesc *task)
{
    Int32 ret = RPC_ERROR_VALUE;
    TaskDesc *pdesc = NULL;

    //Log(LOG_DEBUG,"InvokerSafeAddTaskToQueue task:0x%x\n", task);
    PR_Lock(&invokerManager.mutex);
    if (task)
    {
        ret = QueuePush(&invokerManager.svcQueue, (void *)task);
        /*for debug*/
        pdesc = (TaskDesc *)QueueFront(&invokerManager.svcQueue);
        //Log(LOG_DEBUG,"InvokerSafeAddTaskToQueue pop task:0x%x\n", pdesc);
    }
    PR_Unlock(&invokerManager.mutex);

    return  ret;
}

static TaskDesc *InvokerSafeGetTaskFromQueue(void)
{
    TaskDesc *pdesc = NULL;

    PR_Lock(&invokerManager.mutex);

    pdesc = (TaskDesc *)QueueFront(&invokerManager.svcQueue);
    //Log(LOG_DEBUG,"InvokerSafeGetTaskFromQueue pdesc_addr:0x%x, pdesc:0x%x\n",&pdesc, pdesc);
    if (pdesc)
    {
        QueuePop(&invokerManager.svcQueue);
    }

    PR_Unlock(&invokerManager.mutex);

    return  pdesc;
}

static Int32 InvokerSafeFindOutFreeTaskDesc(McapiConn *conn)
{
    Int32 i = 0;
    Int32 starttry = 0;
    static Int32 lastkit = 0;  /*for balance the task resource allocated per time*/

    PR_Lock(&invokerManager.mutex);
    if (lastkit >= 0 && lastkit < SVC_QUEUE_SIZE)
    {
        i = lastkit + 1;
        if (i >= SVC_QUEUE_SIZE)
        {
            i = 0;
        }
    }
    else
    {
        i = 0;
    }
    starttry = i;
    for (; i < SVC_QUEUE_SIZE; i++)
    {
        if (svcQtask[i].used == 0)
        {
            break;
        }
    }

    /*Enhanced by tony on 2013/05/31*/
    if (i >= SVC_QUEUE_SIZE)
    {
        for (i = 0; i < starttry; i++)
        {
            if (svcQtask[i].used == 0)
            {
                break;
            }
        }
        if (i >= starttry)
        {
            i = SVC_QUEUE_SIZE;
        }
    }
    /*Enhanced end*/

    if (i < SVC_QUEUE_SIZE)
    {
        svcQtask[i].used = 1;  /*Set it busy*/
        svcQtask[i].conn = conn;
        lastkit = i;
        g_TotalserviceQtask++; /*for debug usage*/
    }
    else
    {
        i = RPC_ERROR_VALUE;
    }

    PR_Unlock(&invokerManager.mutex);

    return i;
}

static void InvokerSafeFreeTaskDesc(Int32 index)
{
    if (index >= 0 && index < SVC_QUEUE_SIZE)
    {
        PR_Lock(&invokerManager.mutex);
        svcQtask[index].used = 0; /*Set it free*/
        if (g_TotalserviceQtask > 0)
        {
            g_TotalserviceQtask--;
        }
        PR_Unlock(&invokerManager.mutex);
    }
}

/*-----------------------------------------------------------------------------------------
* [Function]    InvokerServiceThread
*
* [Description]This is the service thread routine for service of lnvokerManager.
*
* [Return]  Non
*------------------------------------------------------------------------------------------*/
static void InvokerServiceThread(void *contex)
{
    Invoker *pService = (Invoker *)contex;
    TaskDesc *ptaskD = NULL;
    Invoker *pInvoker = NULL;
    Uint32  count = 0;
    const Int32 servicewaittick = 50;  /*the unit is ms,!!!Enhance me! Reduce this value will casue the extreme case
                                 response time is lower!!!*/
    Int32 status = 0;

    Log(LOG_DEBUG, "InvokerServiceThread entered!\n");
    while (g_RPCRunning != RPCST_STOPPING)
    {
        /*Check if there is incoming task*/
        if (InvokerSafeCheckTaskQEmpty())
        {
            /*Suspending this thread*/
            if ((status = PR_CondVarWait_Safe(&pService->suspendCv, servicewaittick)) == RPC_OS_ERR)
            {
                Log(LOG_EMERG, "!!!InvokerServiceThread CondVarWait Happended Fatal Error I !!!\n");
            }
            //Log(LOG_DEBUG,"InvokerServiceThread PR_CondVarWait_Safe status:%d\n",status);
        }
        else
        {
            //Log(LOG_DEBUG,"InvokerServiceThread Entered I\n");
            count = 0;
            while (g_RPCRunning)
            {
                /*Check if there is a sleep invoker thread*/
                if (InvokerSafeCheckListEmpty(&invokerManager.sleep))
                {
                    Log(LOG_DEBUG, "InvokerServiceThread Entered II\n");
                    if (count % 40 == 0)
                    {
                        Log(LOG_INFO, "[InvokerServiceThread] All invoker thread are busy! total wait time:%d ms\n", count * servicewaittick);
                    }
                    //PR_Sleep(servicewaittick);
                    /*Suspending this thread until there is siganl when a invoker added into sleep list, or a new task added into svcQueue*/
                    if (PR_CondVarWait_Safe(&pService->suspendCv, servicewaittick) == RPC_OS_ERR)
                    {
                        Log(LOG_EMERG, "!!!InvokerServiceThread CondVarWait Happended Fatal Error II !!!\n");
                    }
                }
                else
                {
                    /*Assign a task to invoker*/
                    ptaskD = InvokerSafeGetTaskFromQueue();
                    if (ptaskD)
                    {
                        Log(LOG_DEBUG, "InvokerServiceThread will dispatch the ptaskD:0x%x, used:%d to invoker thread\n", ptaskD, ptaskD->used);
                    }
                    /*Get a sleep invoker*/
                    pInvoker = InvokerSafeGetFromList(&invokerManager.sleep);
                    if (InvokerManagerAssignTask(pInvoker, (void *)ptaskD) < 0)
                    {
                        Log(LOG_EMERG, "!!!Service Thread Assign Invoker Happened Fatal Error!!!\n");
                    }
                    break;
                }
                count++;
            }

        }


    }

}

static void SetInvokerTaskIdle(Invoker *pInvoker)
{
    TaskDesc *ptaskD = NULL;

    if (!pInvoker)
    {
        return;
    }
    /*Change invoker task to idle*/
    PR_Lock(&pInvoker->mutex);
    ptaskD = pInvoker->task;
    pInvoker->task = NULL;
    PR_Unlock(&pInvoker->mutex);

    /*free the task*/
    PR_Lock(&invokerManager.mutex);
    if (ptaskD)
    {
        Log(LOG_DEBUG, "SetInvokerTaskIdle, ptaskD->used:%d\n", ptaskD->used);
        ptaskD->used = 0;
        if (g_TotalserviceQtask > 0)
        {
            g_TotalserviceQtask--;
        }
    }
    g_TotalRunningInvoker--;
    g_TotalSleepInvoker++;
    PR_Unlock(&invokerManager.mutex);

    InvokerSafeRemoveFromList(&pInvoker->list);     /*Remove from running list*/

    InvokerSafeAddToList(&invokerManager.sleep, &pInvoker->list);
}

static TaskDesc *GetInvokerTask(Invoker *pInvoker)
{
    TaskDesc *ptaskD = NULL;
    if (!pInvoker)
    {
        return NULL;
    }
    PR_Lock(&pInvoker->mutex);
    ptaskD = pInvoker->task;
    PR_Unlock(&pInvoker->mutex);

    return ptaskD;
}

/*-----------------------------------------------------------------------------------------
* [Function]    InvokerThreadCommon
*
* [Description]This is the common invoker thread routine for invoker[] of lnvokerManager.
*
* [Return]  Non
*------------------------------------------------------------------------------------------*/
static void InvokerThreadCommon(void *contex)
{
    Invoker *pInvoker = (Invoker *)contex;
    TaskDesc *ptaskD = NULL;
    McapiConn *pConn = NULL;
    Param  *pargs[8] = {NULL};
    Param  args[8];
    Uint32  ret = 0;
    const Int32 waittick = 200;  /*the unit is ms*/
    Uint8 *retbuf = NULL;
    Int32  sendsize = 0;
    Int32  status = 0;


    Log(LOG_DEBUG, "InvokerThreadCommon entered!\n");
    if(!pInvoker)
    {
    	Log(LOG_ERR,"InvokerThreadCommon input Contex is NULL!");
    	return;
    }

    if ((retbuf = PR_Malloc(RPC_MAX_SEND_BUF)) == NULL)
    {
        Log(LOG_ERR, "InvokerThreadCommon allocate return buffer failed!!!\n");
        return;
    }

    while (g_RPCRunning != RPCST_STOPPING)
    {
        ptaskD = GetInvokerTask(pInvoker);

        if (ptaskD)
        {
            pConn = ptaskD->conn;
        }
        else
        {
            pConn = NULL;
        }
        if (!ptaskD)
        {
            /*Suspending this thread*/
            if ((status = PR_CondVarWait_Safe(&pInvoker->suspendCv, waittick)) == RPC_OS_ERR)
            {
                Log(LOG_EMERG, "!!!InvokerThreadCommon CondVarWait Happended Fatal Error I !!!\n");
            }
            //Log(LOG_DEBUG,"InvokerThreadCommon PR_CondVarWait_Safe return:%d, pinvoker:0x%x\n",status,pInvoker);
        }
        else
        {
            if (!pConn)
            {
                Log(LOG_ERR, "InvokerThreadCommon pConn of task:0x%x is NULL!\n", ptaskD);
                SetInvokerTaskIdle(pInvoker);
                continue;
            }
            Log(LOG_DEBUG, "InvokerThreadCommon will execute callback function, pinvoker:0x%x,ptaskD:0x%x func:0x%x, funcid:0x%x\n", pInvoker,  ptaskD, 
            ptaskD->func, ptaskD->funcid);
            /*execute the callback function*/
            if (ptaskD->func)
            {
                Log(LOG_DEBUG, "InvokerThreadCommon will execute callback function !\n");
                memset(pargs, 0, sizeof(pargs));
                /*Convert the ptaskD params to pargs*/
                InvokerConvertParams(&pargs[0],  &args[0], ptaskD->param1);
                InvokerConvertParams(&pargs[1],  &args[1], ptaskD->param2);
                InvokerConvertParams(&pargs[2],  &args[2], ptaskD->param3);
                InvokerConvertParams(&pargs[3],  &args[3], ptaskD->param4);
                InvokerConvertParams(&pargs[4],  &args[4], ptaskD->param5);
                InvokerConvertParams(&pargs[5],  &args[5], ptaskD->param6);
                InvokerConvertParams(&pargs[6],  &args[6], ptaskD->param7);
                InvokerConvertParams(&pargs[7],  &args[7], ptaskD->param8);

                ret = (ptaskD->func)(pargs[0], pargs[1], pargs[2], pargs[3], pargs[4], pargs[5], pargs[6], pargs[7]);

                /*Prepare the return data*/
                if ((sendsize = FormatReturnArgsEncode(ret, retbuf, RPC_MAX_SEND_BUF,  ptaskD->funcid, ptaskD->rid, ptaskD->xflag, &pargs)) <= 0)
                {
                    Log(LOG_EMERG, "!!!FormatArgsEncodeReturn Happened Error!!!\n");
                }
                else
                {
                    /*Send back  the data to peer*/
                    if (pConn->conn == MCAPI_MSG)
                    {
                        ret = McapiMsgWrite(pConn, (void *)retbuf, sendsize);
                    }
                    else if (pConn->conn == MCAPI_PKTCHAN)
                    {
                        ret = McapiPktWrite(pConn, (void *)retbuf, sendsize);
                    }
                    else
                    {
                        ret = McapiSclWrite(pConn, (void *)retbuf, sendsize);
                    }

                    /*!!Fix me, here I am not sure if the MCAPI would return the real sent size*/
                    if (ret != sendsize)
                    {
                        Log(LOG_ERR, "[InvokerThreadCommon] Deosn't send all the buffer to peer! sendsize:%d, ret:%d\n", sendsize, ret);
                    }
                }

                Log(LOG_DEBUG, "InvokerThreadCommon execute callback function completed! sendsize:%d\n", sendsize);
            }
            else
            {
                Log(LOG_ERR, "!!!InvokerThreadCommon can not find the service functon, ");
                Log(LOG_ERR, "please check if u had export the service or if the caller input correct function name!!!\n");
                /*Send back the error response packet to peer*/
                g_TotalRcvBadFuncIDCall++;
                /*Prepare the return data*/
                memset(pargs, 0, sizeof(pargs));
                if ((sendsize = FormatReturnArgsEncode(RPC_INVOKER_ERR_NOFUNC, retbuf, RPC_MAX_SEND_BUF,  ptaskD->funcid, ptaskD->rid, ptaskD->xflag,
                	 &pargs)) <= 0)
                {
                    Log(LOG_EMERG, "!!!FormatArgsEncodeReturn Happened Error!!!\n");
                }
                else
                {
                    /*Send back  the data to peer*/
                    if (pConn->conn == MCAPI_MSG)
                    {
                        ret = McapiMsgWrite(pConn, (void *)retbuf, sendsize);
                    }
                    else if (pConn->conn == MCAPI_PKTCHAN)
                    {
                        ret = McapiPktWrite(pConn, (void *)retbuf, sendsize);
                    }
                    else
                    {
                        ret = McapiSclWrite(pConn, (void *)retbuf, sendsize);
                    }

                }

            }
            /*Free Resource and Add itselft to InvokerManager sleep List*/
            SetInvokerTaskIdle(pInvoker);

            /*!!Wakup and Notify the service thread because maybe the service thread scheduler is waiting the idle invoker availabe*/
            PR_CondVarNotify_Safe(&invokerManager.service.suspendCv);


        }
    }

    /*Free allocated buffer*/
    if (retbuf)
    {
        PR_Free(retbuf);
    }

}




/*-----------------------------------------------------------------------------------------
* [Function]    InvokerManagerInit
*
* [Description]This interface initializes the invoker manager unit, ie. initializing all threads suspending condition
*           variables, starting up invoker threads, service thread, creating the service queue to manage the
*           incoming RPC service tasks, initializing the running thread list and sleep thread list.
* [Return]  0->Success,  othe value->Failed.
*------------------------------------------------------------------------------------------*/
Int32 InvokerManagerInit(void)
{
    InvokerManager *pInvM = &invokerManager;
    int i = 0;;
    char name[64] = {0};

    Log(LOG_DEBUG, "[InvokerManagerInit] start!\n");
    memset((void *)&invokerManager, 0, sizeof(invokerManager));

    /*Initializing mutex of InvokerManager*/
    if (PR_MutexInit(&pInvM->mutex) != RPC_OS_OK)
    {
        return  RPC_INVOKER_ERR;
    }
    //Log(LOG_DEBUG,"[InvokerManagerInit] entered 2\n");

    /*Initializing running and sleep Lists of InvokerManager*/
    LIST_INIT(&pInvM->running);
    LIST_INIT(&pInvM->sleep);

    //Log(LOG_DEBUG,"[InvokerManagerInit] entered 3\n");
    /*Initializing svcQueue of InvokerManager*/
    if (QueueInit((void *)&pInvM->svcQueue, sizeof(void *), SVC_QUEUE_SIZE) != 0)
    {
        PR_MutexDestroy(&pInvM->mutex);
        return  RPC_INVOKER_ERR;
    }
    //Log(LOG_DEBUG,"[InvokerManagerInit] entered 4\n");

    /*initializing mutex and CondVar of service, create service thread and suspend it*/
    if (PR_MutexInit(&pInvM->service.suspendMutex) != RPC_OS_OK)
    {
        PR_MutexDestroy(&pInvM->mutex);
        QueueDeinit((void *)&pInvM->svcQueue);
        return  RPC_OS_ERR_MUTEX;
    }
    //Log(LOG_DEBUG,"[InvokerManagerInit] entered 5\n");

    if (PR_CondVarInit(&pInvM->service.suspendCv, &pInvM->service.suspendMutex) != RPC_OS_OK)
    {
        PR_MutexDestroy(&pInvM->mutex);
        QueueDeinit((void *)&pInvM->svcQueue);
        PR_MutexDestroy(&pInvM->service.suspendMutex);
        return  RPC_OS_ERR_COND;
    }
    Log(LOG_DEBUG, "[InvokerManagerInit] Will create service thread!\n");


    if (PR_ThreadCreate(&pInvM->service.tid, &InvokerServiceThread, (void *)&pInvM->service, 0, 0, "rpc_inv_service") != RPC_OS_OK)
    {
        Log(LOG_ERR, "[InvokerManagerInit] PR_ThreadCreate for service thread failed!\n");
        PR_MutexDestroy(&pInvM->mutex);
        QueueDeinit((void *)&pInvM->svcQueue);
        PR_CondVarDestroy(&pInvM->service.suspendCv);
        PR_MutexDestroy(&pInvM->service.suspendMutex);
        return  RPC_OS_ERR_THREAD;
    }

    //Log(LOG_DEBUG,"[InvokerManagerInit] entered 7\n");

    /*initializing mutex and CondVar of invoker, create all invoker threads and suspend its*/
    for (i = 0; i < RPC_INVOKER_MAX; i++)
    {
        //Log(LOG_DEBUG,"[InvokerManagerInit] entered 8_1\n");
        if (PR_MutexInit(&pInvM->invoker[i].mutex) != RPC_OS_OK)
        {
            PR_MutexDestroy(&pInvM->mutex);
            QueueDeinit((void *)&pInvM->svcQueue);
            PR_CondVarDestroy(&pInvM->service.suspendCv);
            PR_MutexDestroy(&pInvM->service.suspendMutex);
            return  RPC_OS_ERR_MUTEX;
        }
        if (PR_MutexInit(&pInvM->invoker[i].suspendMutex) != RPC_OS_OK)
        {
            PR_MutexDestroy(&pInvM->mutex);
            QueueDeinit((void *)&pInvM->svcQueue);
            PR_CondVarDestroy(&pInvM->service.suspendCv);
            PR_MutexDestroy(&pInvM->service.suspendMutex);

            PR_MutexDestroy(&pInvM->invoker[i].mutex);
            return  RPC_OS_ERR_MUTEX;
        }
        //Log(LOG_DEBUG,"[InvokerManagerInit] entered 8_2\n");
        if (PR_CondVarInit(&pInvM->invoker[i].suspendCv, &pInvM->invoker[i].suspendMutex) != RPC_OS_OK)
        {
            PR_MutexDestroy(&pInvM->mutex);
            QueueDeinit((void *)&pInvM->svcQueue);
            PR_CondVarDestroy(&pInvM->service.suspendCv);
            PR_MutexDestroy(&pInvM->service.suspendMutex);

            PR_MutexDestroy(&pInvM->invoker[i].mutex);
            PR_MutexDestroy(&pInvM->invoker[i].suspendMutex);
            return  RPC_OS_ERR_COND;
        }
        //Log(LOG_DEBUG,"[InvokerManagerInit] entered 8_3\n");
        sprintf(name, "rpc_invoker%d", i + 1);
        if (PR_ThreadCreate(&pInvM->invoker[i].tid, &InvokerThreadCommon, (void *)&pInvM->invoker[i], 0, 0, name) != RPC_OS_OK)
        {
            Log(LOG_ERR, "[InvokerManagerInit]PR_ThreadCreate Failed for invoker[%d]!\n", i);
            PR_MutexDestroy(&pInvM->mutex);
            QueueDeinit((void *)&pInvM->svcQueue);
            PR_CondVarDestroy(&pInvM->service.suspendCv);
            PR_MutexDestroy(&pInvM->service.suspendMutex);

            PR_MutexDestroy(&pInvM->invoker[i].mutex);
            PR_CondVarDestroy(&pInvM->invoker[i].suspendCv);
            PR_MutexDestroy(&pInvM->invoker[i].suspendMutex);
            return  RPC_OS_ERR_THREAD;
        }
        //Log(LOG_DEBUG,"[InvokerManagerInit] entered 8_4\n");
        /*Add to sleep list*/
        InvokerSafeAddToList(&invokerManager.sleep, &pInvM->invoker[i].list);
        g_TotalSleepInvoker++;
        //Log(LOG_DEBUG,"[InvokerManagerInit] entered 8_5\n");
    }

    Log(LOG_DEBUG, "[InvokerManagerInit] entered End\n");

    return  RPC_SUCCESS_VALUE;

}

/*-----------------------------------------------------------------------------------------
* [Function]    InvokerManagerDeinit
*
* [Description]This interface exits the invoker manager unit, it does the reverse procedures that InvokerManagerInit
*           has ever done, ie. destroying all the threads, destroying all the condition variables, destroying the service
*           queue, cleaning up the running thread list and sleep thread list.
*
* [Return]  0->Success,  other value->Failed.
*------------------------------------------------------------------------------------------*/
Int32 InvokerManagerDeinit(void)
{
    InvokerManager *pInvM = &invokerManager;
    int i = 0;

    PR_MutexDestroy(&pInvM->mutex);
    QueueDeinit((void *)&pInvM->svcQueue);
    PR_CondVarDestroy(&pInvM->service.suspendCv);
    PR_MutexDestroy(&pInvM->service.suspendMutex);

    for (i = 0; i < RPC_INVOKER_MAX; i++)
    {
        PR_CondVarDestroy(&pInvM->invoker[i].suspendCv);
        PR_MutexDestroy(&pInvM->invoker[i].suspendMutex);
    }

    return  RPC_SUCCESS_VALUE;
}

/*-----------------------------------------------------------------------------------------
* [Function]    InvokerManagerAlloc
*
* [Description]This interface allocates a sleep invoker thread, returns the pointer to the invoker if success,
*           NULL else.
*
* [Return]  NULL->Failed,  other value->Success.
*------------------------------------------------------------------------------------------*/
Invoker *InvokerManagerAlloc(void)
{
    /*!!Doesn't need to be suppotted now*/
    /*!!Maybe Improve me later!!*/
    return  NULL;
}

/*-----------------------------------------------------------------------------------------
* [Function]    InvokerManagerFree
*
* [Description]This interface frees the specific invoker that has just finished the task to the sleep thread list.
*
* [Return]  Non
*------------------------------------------------------------------------------------------*/
void InvokerManagerFree(Invoker *invoker)
{
    /*!!Doesn't need to be suppotted now*/
    /*!!Maybe Improve me later!!*/
}

/*-----------------------------------------------------------------------------------------
* [Function]    InvokerManagerAssignTask
*
* [Description]This interface assigns a task to the specific invoker, the task data stored in the parameter arg.
*                    Only service thread would call this interface.
*
* [Return]  -1->Failed,   othe value->Success.
*------------------------------------------------------------------------------------------*/
Int32 InvokerManagerAssignTask(Invoker *invoker, void *arg)
{
    if (!invoker || !arg)
    {
        return    RPC_INVOKER_ERR;
    }

    Log(LOG_DEBUG, "InvokerManagerAssignTask entered, used:%d\n", ((TaskDesc *)arg)->used);
    /*Add invoker to running list*/
    InvokerSafeAddToList(&invokerManager.running, &invoker->list);

    PR_Lock(&invoker->mutex);
    invoker->task = (TaskDesc *)arg;
    PR_Unlock(&invoker->mutex);

    /*Added mutex lock for debug usage on 2013/05/27*/
    PR_Lock(&invokerManager.mutex);
    g_TotalRunningInvoker++;
    g_TotalSleepInvoker--;
    PR_Unlock(&invokerManager.mutex);
    /*Added end*/
    Log(LOG_DEBUG, "InvokerManagerAssignTask task:0x%x, task->used:%d\n", invoker->task, invoker->task->used);
    /*Wakeup the invoker thread*/
    PR_CondVarNotify_Safe(&invoker->suspendCv);

    return RPC_SUCCESS_VALUE;
}

/*-----------------------------------------------------------------------------------------
* [Function]    InvokerManagerEnqueue
*
* [Description]This interface enqueues a task specified by arg to the service queue, returns -1 if the enqueue
*           is failed or the queue is full. Also it notifies the service thread to wake up.
*           This function will also do xdr format(decode) to incoming arg buffer.
*
* [Return]  -1->Failed,   othe value->Success.
*------------------------------------------------------------------------------------------*/
Int32 InvokerManagerEnqueue(Uint8 *arg, Int32 len, McapiConn *conn)
{
    Int32  freeTaskIndex = -1;
    Ulong *pLong = NULL;

    Log(LOG_DEBUG, "InvokerManagerEnqueue entered!\n");
    if (!arg || !conn)
    {
        return    RPC_INVOKER_ERR;
    }
    if ((freeTaskIndex = InvokerSafeFindOutFreeTaskDesc(conn)) < 0)
    {
        Log(LOG_ERR, "The SvcQueue is full!!! We will discard this task!g_TotalserviceQtask:%d,SVC_QUEUE_SIZE:%d\n", g_TotalserviceQtask, SVC_QUEUE_SIZE);
        return  RPC_INVOKER_ERR_QFULL;
    }

    Log(LOG_DEBUG, "InvokerManagerEnqueue entered I, freeTaskIndex:%d, used:%d\n", freeTaskIndex, svcQtask[freeTaskIndex].used);
    if (FormatCallerArgsDecode(arg, len, &svcQtask[freeTaskIndex]) < 0)
    {
        Log(LOG_ERR, "!!Decode failed!!Receivedf BAD Task Stream!\n");
        InvokerSafeFreeTaskDesc(freeTaskIndex);
        return  RPC_ARGS_ERR;
    }

    //Log(LOG_DEBUG,"InvokerManagerEnqueue II, used:%d\n", svcQtask[freeTaskIndex].used);
    /*Get the callback function pointer from hash table*/
    pLong = (Ulong *)InvokerSafeGetCallbackF((void *)&svcQtask[freeTaskIndex].funcid);
    //Log(LOG_DEBUG,"InvokerManagerEnqueue foud callback func:0x%x\n", pLong);
    if (!pLong)
    {
        Log(LOG_DEBUG, "InvokerManagerEnqueue entered III, can not find the function! funcid:0x%x\n", svcQtask[freeTaskIndex].funcid);
        /*Will let the invoker send back the related error packet to peer*/
        svcQtask[freeTaskIndex].func = 0;
    }
    else
    {
        svcQtask[freeTaskIndex].func = (PRpcFunc)(*pLong);
    }

    //Log(LOG_DEBUG,"InvokerManagerEnqueue svcQtask:0x%x, index:%d, func:0x%x, funcid:0x%x\n",&svcQtask[freeTaskIndex], freeTaskIndex ,
    //svcQtask[freeTaskIndex].func, svcQtask[freeTaskIndex].funcid);
    /*Set the McapiConn for sending result to peer, here we must use the "send" channel*/
    if (conn->conn == MCAPI_MSG || conn->conn == MCAPI_PKTCHAN)
    {
        svcQtask[freeTaskIndex].conn = &g_mcapiconn_send[conn->conn];
    }
    else
    {
        svcQtask[freeTaskIndex].conn = NULL;
        Log(LOG_ERR, "InvokerManagerEnqueue doesn't support conn:%d type\n", conn->conn);
        InvokerSafeFreeTaskDesc(freeTaskIndex);
        return RPC_INVOKER_ERR;
    }
    //Log(LOG_DEBUG,"InvokerManagerEnqueue III, used:%d\n", svcQtask[freeTaskIndex].used);
    /*Append task to svcQueue*/
    InvokerSafeAddTaskToQueue(&svcQtask[freeTaskIndex]);

    //Log(LOG_DEBUG,"InvokerManagerEnqueue IV, used:%d\n", svcQtask[freeTaskIndex].used);
    /*Wakup and Notify the service thread*/
    PR_CondVarNotify_Safe(&invokerManager.service.suspendCv);

    Log(LOG_DEBUG, "InvokerManagerEnqueue completed\n");

    return RPC_SUCCESS_VALUE;
}





