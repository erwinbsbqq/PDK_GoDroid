/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: ali_rpc_ctrl.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_ctrl.h>
#include <ali_shm.h>
#include <ali_rpcng.h>


extern MINIRPC_INTERFACE Minirpc_Interface;
extern unsigned char *rx_buf;


Ctrl_Msg_Status ctrlMsgStatus;
ReturnBuffer returnBufferManager;

TaskDesc Call_TaskDesc;
	
extern HashTable *g_ht;

Param *g_pParams[RETURN_BUFFER_MAX * 8] ={NULL};  /*this is for caller param returnning usage*/

TaskDesc Minirpc_TaskDesc;

#define RPC_MAX_SEND_BUF   		        16*1024


/*
* Initialize the return buffer management module.
* Return: 0->Success, othe value->Failed.
*/
Int32 RetInit(void)
{
    memset(&returnBufferManager, 0, sizeof(returnBufferManager));
    returnBufferManager.nrFree = RETURN_BUFFER_MAX;
    returnBufferManager.lastRid = -1;

	/*ReturnBuffer mutex initialization*/
    //if (PR_MutexInit(&returnBufferManager.mutex) == RPC_OS_ERR)
    //{
        //return    RPC_OS_ERR_MUTEX;
    //}

    return  RPC_SUCCESS_VALUE;
}

/*
* Destroy the return buffer management module and release related resource.
* Return: 0->Success, othe value->Failed.
*/
Int32 RetDeinit(void)
{
    //PR_MutexDestroy(&returnBufferManager.mutex);
    //PR_MutexDestroy(&returnBufferManager.retMutex);

    return  RPC_SUCCESS_VALUE;
}

/*
* Allocate a return buffer for the user, initialize the condition variable for the given service.
* Return: (<0)->Failed, othe value->Allocated rid.
*/
Int32 RetAlloc(void)
{
    int rid = returnBufferManager.lastRid + 1;
    int offset = rid & BITS_PER_RETMAP_MASK;

    //PR_Lock(&returnBufferManager.mutex);

    if (!returnBufferManager.nrFree)
    {
        //PR_Unlock(&returnBufferManager.mutex);
        return RPC_ERROR_VALUE;
    }
    offset = Rid_find_next_zero_bit(&returnBufferManager.retMap, BITS_PER_RETMAP, offset);
    /*Enhanced by tony on 2013/05/31*/
    if (offset >= BITS_PER_RETMAP)
    {
        offset = Rid_find_next_zero_bit(&returnBufferManager.retMap, BITS_PER_RETMAP, 0);
    }
    /*Enhanced end*/
    if (BITS_PER_RETMAP != offset && !Rid_test_and_set_bit(offset, &returnBufferManager.retMap))
    {
        --returnBufferManager.nrFree;
        returnBufferManager.lastRid = offset;

		returnBufferManager.retCond[offset]= -1;
        /*Initialize the condition variable*/
        //if (PR_MutexInit(&returnBufferManager.retMutex[offset]) != RPC_OS_OK)
        //{
            //Log(LOG_ERR, "RetAlloc rid:%d, PR_MutexInit failed!\n", offset);
            //PR_Unlock(&returnBufferManager.mutex);
            //return  RPC_OS_ERR_MUTEX;
        //}
        //if (PR_CondVarInit(&returnBufferManager.retCond[offset], &returnBufferManager.retMutex[offset]) != RPC_OS_OK)
        //{
            //Log(LOG_ERR, "RetAlloc rid:%d, PR_CondVarInit failed!\n", offset);
            //PR_MutexDestroy(&returnBufferManager.retMutex[offset]);
            //PR_Unlock(&returnBufferManager.mutex);
            //return  RPC_OS_ERR_COND;
        //}

        //PR_Unlock(&returnBufferManager.mutex);
        Log(LOG_DEBUG,"[%s] RetAlloc, offset:%d  A\n",__func__,offset);
        return offset;
    }

    //PR_Unlock(&returnBufferManager.mutex);
    Log(LOG_ERR,"[%s] RetAlloc, offset:%d  B\n",__func__, offset);
    return RPC_ERROR_VALUE;

}

/*
* Free the return buffer and destroy the condition variable that indicated by rid.
* Return: Non
*/
void RetFree(Int32 rid)
{
    Int32 offset = rid & BITS_PER_RETMAP_MASK;

    returnBufferManager.nrFree++;
    Rid_clear_bit(offset, &returnBufferManager.retMap);

	returnBufferManager.retCond[offset]= -1;

    /*Release mutex resource*/
    //PR_MutexDestroy(&returnBufferManager.retMutex[offset]);

    /*release the condition variable*/
    //PR_CondVarDestroy(&returnBufferManager.retCond[offset]);

}


/*-----------------------------------------------------------
* user could use this interface to listen to the corresponding RPC service's
* completion. It is definitely a blocking function, till the local has got the
* remote RPC service return data.
*
* Return:  62->timeout, 0->a normal cond return, other value->failed
*------------------------------------------------------------*/
Int32 RpcWait(Int32 rid, Uint32 timeout)
{
    Int32 offset = rid & BITS_PER_RETMAP_MASK;
    Int32 retvalue = 0;

    //retvalue = PR_CondVarWait_Safe(&returnBufferManager.retCond[offset], timeout);

    return  retvalue;
}


/*
* This method frees the given return buffer allocated by the service, returns the
* status of the call. It is called by the user once the nonsynchronous service has finished.
* Return: 0->Success,  other value->Failed.
*/
Int32 RpcFreeRetBuf(Int32 rid)
{
    //PR_Lock(&returnBufferManager.mutex);

    if (!Rid_test_bit(rid, &returnBufferManager.retMap))
    {
        //PR_Unlock(&returnBufferManager.mutex);
        Log(LOG_ERR, "!!RpcFreeRetBuf rid:%d was free!\n", rid);
        return RPC_ERROR_VALUE;
    }

    RetFree(rid);

    //PR_Unlock(&returnBufferManager.mutex);

    return  RPC_SUCCESS_VALUE;
}

/*
* This method extracts the return value of the given service by rid.
* Return: Rpc call return value
*/
Int32 RpcExtractRet(Int32 rid)
{
    Int32  retvalue = -1;
    Int32 offset = rid & BITS_PER_RETMAP_MASK;

    //PR_Lock(&returnBufferManager.mutex);

    retvalue = returnBufferManager.retBuff[offset];

    //PR_Unlock(&returnBufferManager.mutex);

    return  retvalue;
}

/*Set return value to returnBufferManager*/
Int32 RpcUpdateRet(Int32 rid, Int32 result)
{
    Int32 offset = rid & BITS_PER_RETMAP_MASK;

    returnBufferManager.retBuff[offset] = result;

    return  RPC_SUCCESS_VALUE;
}

/*Set returned arguments address to g_pParams for blocking rpccall*/
void  SetRetArgsAddress(Uint32 rid, Param *pargs[])
{
    Int32 index= 0;
    Int32 i = 0;

    if (!pargs)
    {
        return;
    }
    index = rid << 3;
    for (i = 0; i < 8; i++)
    {
        g_pParams[index + i] = pargs[i];
    }
}

static void *GetCallbackF(void *key)
{
    /*Doesn't need mutex lock now because the callback functions table are created staticly*/
    return (void *)HashTableLookupConst(g_ht, key);

}


static void InvokerConvertParams(Param **parg,  Param *constarg,  Uint8 *param)
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

/*-----------------------------------------------------------------------------------------
* [Function]    InvokerManagerEnqueue
*
* [Description]This interface enqueues a task specified by arg to the service queue, returns -1 if the enqueue
*           is failed or the queue is full. Also it notifies the service thread to wake up.
*           This function will also do xdr format(decode) to incoming arg buffer.
*
* [Return]  -1->Failed,   othe value->Success.
*------------------------------------------------------------------------------------------*/
Int32 ProcessCallPacket(Uint8 *arg, Int32 len)
{
	TaskDesc *pcall_TaskDesc = &Minirpc_TaskDesc;
	Int32  freeTaskIndex = -1;
	Ulong *pLong = NULL;
	Param  *pargs[8] = {NULL};
	Param  args[8];
	Uint32	ret = 0;
	Int32  sendsize = 0;
	Uint8 *retbuf = NULL;
	int i=0;

	/*Prepare the return data*/
	if ((retbuf = PR_Malloc(RPC_MAX_SEND_BUF)) == NULL)
    {
        Log(LOG_ERR, "allocate return buffer failed!!!\n");
        return RPC_ERROR_VALUE;
    }
	
	if (FormatCallerArgsDecode(arg, len, pcall_TaskDesc) < 0)
	{
		PR_LOG("!!Decode failed!!Receivedf BAD Task Stream!\n");
		return	RPC_ARGS_ERR;
	}


	pLong = (Ulong *)GetCallbackF((void *)&(pcall_TaskDesc->funcid));
	if (!pLong)
	{
		PR_LOG("ProcessCallPacket entered III, can not find the function! funcid:0x%x\n",pcall_TaskDesc->funcid);
		pcall_TaskDesc->func = 0;
	}
	else
	{
		pcall_TaskDesc->func = (PRpcFunc)(*pLong);
	}

	Minirpc_Interface.minirpc_free_buffer(rx_buf);
	
	if (pcall_TaskDesc->func)
	{
		PR_LOG("ProcessCallPacket will execute callback function !\n");
		memset(pargs, 0, sizeof(pargs));
		/*Convert the ptaskD params to pargs*/
		InvokerConvertParams(&pargs[0],  &args[0], pcall_TaskDesc->param1);
		InvokerConvertParams(&pargs[1],  &args[1], pcall_TaskDesc->param2);
		InvokerConvertParams(&pargs[2],  &args[2], pcall_TaskDesc->param3);
		InvokerConvertParams(&pargs[3],  &args[3], pcall_TaskDesc->param4);
		InvokerConvertParams(&pargs[4],  &args[4], pcall_TaskDesc->param5);
		InvokerConvertParams(&pargs[5],  &args[5], pcall_TaskDesc->param6);
		InvokerConvertParams(&pargs[6],  &args[6], pcall_TaskDesc->param7);
		InvokerConvertParams(&pargs[7],  &args[7], pcall_TaskDesc->param8);

		ret = (pcall_TaskDesc->func)(pargs[0], pargs[1], pargs[2], pargs[3], pargs[4], pargs[5], pargs[6], pargs[7]);

		PR_LOG("ProcessCallPacket  execute callback function return! ret=%d\n",ret);
		
		if ((sendsize = FormatReturnArgsEncode(ret, retbuf, RPC_MAX_SEND_BUF,  pcall_TaskDesc->funcid, pcall_TaskDesc->rid, pcall_TaskDesc->xflag, &pargs)) <= 0)
		{
			PR_LOG("!!!FormatArgsEncodeReturn Happened Error!!!\n");
		}
		else
		{		
			ret = RpcMsgWrite((void *)retbuf, sendsize);
			/*!!Fix me, here I am not sure if the MCAPI would return the real sent size*/
			if (ret != sendsize)
			{
				PR_LOG("[ProcessCallPacket] Deosn't send all the buffer to peer! sendsize:%d, ret:%d\n", sendsize, ret);
			}
		}

		Log(LOG_DEBUG, "ProcessCallPacket execute callback function completed! sendsize:%d\n", sendsize);

	}
	 else
	 {
		PR_LOG("!!!ProcessCallPacket can not find the service functon, ");

		PR_LOG("please check if u had export the service or if the caller input correct function name!!!\n");

		/*Prepare the return data*/
		memset(pargs, 0, sizeof(pargs));
		if ((sendsize = FormatReturnArgsEncode(RPC_INVOKER_ERR_NOFUNC, retbuf, RPC_MAX_SEND_BUF,  pcall_TaskDesc->funcid, pcall_TaskDesc->rid, pcall_TaskDesc->xflag,
			 &pargs)) <= 0)
		{
			PR_LOG("!!!FormatArgsEncodeReturn Happened Error!!!\n");
		}
		else
		{	
			//before write msg, free the merge buffer			 
			ret = RpcMsgWrite((void *)retbuf, sendsize);

		}

	}

	PR_LOG("ProcessCallPacket completed\n");

	return RPC_SUCCESS_VALUE;
}


/*
* Process the returned result packet from peer
* Return: 0->Success, other value->Failed.
*/
Int32 ProcessReturnPacket(Uint8 *buf, Int32 buflen, Uint32 rid)
{
    Int32 result = 0;
    Int32 xflag = 0;
    Uint32 ui = 0;
    int i;
	
    if (!buf)
    {
        return    RPC_ERROR_VALUE;
    }
    if (rid >= RETURN_BUFFER_MAX)
    {
        return    RPC_ERROR_VALUE;
    }

    Log(LOG_DEBUG, "ProcessReturnPacket start, rid:%d buflen:%d\n", rid,buflen);

    /*Check if rid is valid*/
    if (!Rid_test_bit(rid, &returnBufferManager.retMap))
    {
        Log(LOG_ERR, "!!ProcessReturnPacket rid:%d was free!\n", rid);
        return RPC_ERROR_VALUE;
    }

    if (FormatReturnArgsDecode(buf, buflen,  &ui, &result, &xflag, &g_pParams[(rid << 3)]) < 0)
    {
        Log(LOG_ERR, "[ProcessReturnPacket] returnpacket decode failed!\n");
        return  RPC_ARGS_ERR;
    }

	Minirpc_Interface.minirpc_free_buffer(rx_buf);

    /*Notify the CondV*/
    RpcUpdateRet(rid, result);

    if (!Rid_test_bit(rid, &returnBufferManager.retMap))
    {
        Log(LOG_ERR, "!!ProcessReturnPacket rid:%d was free II!\n", rid);
        return RPC_ERROR_VALUE;
    }
    else
    {
        PR_CondVarNotify_Safe(&returnBufferManager.retCond[rid]);
        Log(LOG_DEBUG,"ProcessReturnPacket CondNotify End, rid:%d\n",rid); //debug usage
    }

    Log(LOG_DEBUG, "\nProcessReturnPacket End, rid:%d\n", rid);

    return  RPC_SUCCESS_VALUE;
}


