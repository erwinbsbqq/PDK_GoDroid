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

#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
#include "../../lib/libmcapi/shm.h"
extern unsigned char *minirpc_rx_buf;
extern MINIRPC_INTERFACE Minirpc_Interface;
#endif

ReturnBuffer returnBufferManager;
Param *g_pParams[RETURN_BUFFER_MAX * 8] ={NULL};  /*this is for caller param returnning usage*/





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
    if (PR_MutexInit(&returnBufferManager.mutex) == RPC_OS_ERR)
    {
        return    RPC_OS_ERR_MUTEX;
    }

    return  RPC_SUCCESS_VALUE;
}

/*
* Destroy the return buffer management module and release related resource.
* Return: 0->Success, othe value->Failed.
*/
Int32 RetDeinit(void)
{
    PR_MutexDestroy(&returnBufferManager.mutex);
    PR_MutexDestroy(&returnBufferManager.retMutex);

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

    PR_Lock(&returnBufferManager.mutex);

    if (!returnBufferManager.nrFree)
    {
        PR_Unlock(&returnBufferManager.mutex);
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
        /*Initialize the condition variable*/
        if (PR_MutexInit(&returnBufferManager.retMutex[offset]) != RPC_OS_OK)
        {
            Log(LOG_ERR, "RetAlloc rid:%d, PR_MutexInit failed!\n", offset);
            PR_Unlock(&returnBufferManager.mutex);
            return  RPC_OS_ERR_MUTEX;
        }
        if (PR_CondVarInit(&returnBufferManager.retCond[offset], &returnBufferManager.retMutex[offset]) != RPC_OS_OK)
        {
            Log(LOG_ERR, "RetAlloc rid:%d, PR_CondVarInit failed!\n", offset);
            PR_MutexDestroy(&returnBufferManager.retMutex[offset]);
            PR_Unlock(&returnBufferManager.mutex);
            return  RPC_OS_ERR_COND;
        }

        PR_Unlock(&returnBufferManager.mutex);
        return offset;
    }

    PR_Unlock(&returnBufferManager.mutex);
    //Log(LOG_ERR,"RetAlloc, offset:%d\n", offset);
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

    /*Release mutex resource*/
    PR_MutexDestroy(&returnBufferManager.retMutex[offset]);

    /*release the condition variable*/
    PR_CondVarDestroy(&returnBufferManager.retCond[offset]);

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

    retvalue = PR_CondVarWait_Safe(&returnBufferManager.retCond[offset], timeout);

    return  retvalue;
}


/*
* This method frees the given return buffer allocated by the service, returns the
* status of the call. It is called by the user once the nonsynchronous service has finished.
* Return: 0->Success,  other value->Failed.
*/
Int32 RpcFreeRetBuf(Int32 rid)
{
    PR_Lock(&returnBufferManager.mutex);

    if (!Rid_test_bit(rid, &returnBufferManager.retMap))
    {
        PR_Unlock(&returnBufferManager.mutex);
        Log(LOG_ERR, "!!RpcFreeRetBuf rid:%d was free!\n", rid);
        return RPC_ERROR_VALUE;
    }

    RetFree(rid);

    PR_Unlock(&returnBufferManager.mutex);

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

    PR_Lock(&returnBufferManager.mutex);

    retvalue = returnBufferManager.retBuff[offset];

    PR_Unlock(&returnBufferManager.mutex);

    return  retvalue;
}

/*Set return value to returnBufferManager*/
Int32 RpcUpdateRet(Int32 rid, Int32 result)
{
    Int32 offset = rid & BITS_PER_RETMAP_MASK;

    PR_Lock(&returnBufferManager.mutex);

    returnBufferManager.retBuff[offset] = result;

    PR_Unlock(&returnBufferManager.mutex);

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

/*
* Process the returned result packet from peer
* Return: 0->Success, other value->Failed.
*/
Int32 ProcessReturnPacket(Uint8 *buf, Int32 buflen, Uint32 rid)
{
    Int32 result = 0;
    Int32 xflag = 0;
    Uint32 ui = 0;

    if (!buf)
    {
        return    RPC_ERROR_VALUE;
    }
    if (rid >= RETURN_BUFFER_MAX)
    {
        return    RPC_ERROR_VALUE;
    }

    Log(LOG_DEBUG, "\nProcessReturnPacket start, rid:%d\n", rid);

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
	
	
#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
    Minirpc_Interface.minirpc_free_buffer(minirpc_rx_buf);
#endif

    /*Notify the CondV*/
    RpcUpdateRet(rid, result);

    PR_Lock(&returnBufferManager.mutex);
    if (!Rid_test_bit(rid, &returnBufferManager.retMap))
    {
        PR_Unlock(&returnBufferManager.mutex);
        Log(LOG_ERR, "!!ProcessReturnPacket rid:%d was free II!\n", rid);
        return RPC_ERROR_VALUE;
    }
    else
    {
        PR_CondVarNotify_Safe(&returnBufferManager.retCond[rid]);
        //Log(LOG_DEBUG,"ProcessReturnPacket CondNotify End, rid:%d\n",rid); //debug usage
    }
    PR_Unlock(&returnBufferManager.mutex);


    Log(LOG_DEBUG, "\nProcessReturnPacket End, rid:%d\n", rid);

    return  RPC_SUCCESS_VALUE;
}


