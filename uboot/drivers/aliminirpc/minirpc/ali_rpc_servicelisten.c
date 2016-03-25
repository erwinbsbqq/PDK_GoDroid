/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: ali_rpc_servicelisten.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_servicelisten.h>

SvcListenDesc svcListen[RPC_CHANNEL_MAX];
Int32   g_svcListenNum = 0;    /*Means total initialized service listen thread number*/
extern Int32    g_RPCRunning;
extern McapiConn    g_mcapiconn_rcv[RPC_CHANNEL_MAX];
extern HashTable *g_ht;
extern Uint32  g_TotalRcvMsgCall;
extern Uint32  g_TotalRcvMsgRet;
extern Uint32  g_TotalRcvBadMsgPacket;
extern Uint32  g_TotalRcvPktCall;
extern Uint32  g_TotalRcvPktRet;
extern Uint32  g_TotalRcvBadPktPacket;

extern Int32 PR_ThreadCreate(Thread *tid, ThreadFunc thread, void *arg, Int32 prio, Uint32 stacksize, Int8 *name);
extern Int32 ProcessReturnPacket(Uint8 *buf, Int32 buflen, Uint32 rid);
extern Int32 InvokerManagerEnqueue(Uint8 *arg, Int32 len, McapiConn *conn);

static void RecordRcvCallDebugInfo()
{
    g_TotalRcvMsgCall++;
}

static void RecordRcvRetDebugInfo(ConnType conn)
{
    if (conn == MCAPI_MSG)
    {
        g_TotalRcvMsgRet++;
    }
    else
    {
        g_TotalRcvPktRet++;
    }
}

static void RecordRcvBadPacketDebugInfo(ConnType conn)
{
    if (conn == MCAPI_MSG)
    {
        g_TotalRcvBadMsgPacket++;
    }
    else
    {
        g_TotalRcvBadPktPacket++;
    }
}

/*-----------------------------------------------------------------------------------------
* [Function]    SvcListenThreadCommon
*
* [Description]This is the Svc listen thread routine for all the channel.
*
* [Return]  Non
*------------------------------------------------------------------------------------------*/
static void SvcListenThreadCommon(void *contex)
{
    SvcListenDesc       *psvcDesc = (SvcListenDesc *)contex;
    McapiConn   *pConn = &g_mcapiconn_rcv[psvcDesc->conn];
    Uint8   *argbuf = NULL, *pargbuf = NULL;
    Int32   ret = 0, len = 0;
    Uint32  rid = 0;

    Log(LOG_DEBUG, "SvcListenThreadCommon entered! conn type:%d\n", psvcDesc->conn);
    if ((argbuf = PR_Malloc(RPC_MAX_RCV_BUF)) == NULL)
    {
        Log(LOG_ERR, "SvcListenThreadCommon allocate received buffer failed!!!\n");
        return;
    }
    while (g_RPCRunning != RPCST_STOPPING)
    {
        /*Assume McapiOpen was called before this routine, will blocking until incoming data from remote node.*/

        /*try to read the incoming data from this conn*/
        if (pConn->conn == MCAPI_MSG)
        {
            pargbuf = argbuf;
            Log(LOG_DEBUG, "SvcListenThreadCommon Try to read data from MCAPI_MSG interface, pargbuf:0x%x!\n", pargbuf);
            len = ret = McapiMsgRead(pConn, (void *)pargbuf, RPC_MAX_RCV_BUF);
            Log(LOG_DEBUG, "SvcListenThreadCommon McapiMsgRead read completed! pargbuf:0x%x\n", pargbuf);
        }
        else if (pConn->conn == MCAPI_PKTCHAN)
        {
            pargbuf = NULL;
            Log(LOG_DEBUG, "SvcListenThreadCommon Try to read data from MCAPI_PKTCHAN interface!\n");
            ret = McapiPktRead(pConn, (void **)&pargbuf, &len);
        }
        else
        {
            Log(LOG_DEBUG, "[SvcListenThreadCommon] Doesn't support Scl Channel!\n");
            PR_uSleep(100000);
            //PR_Sleep(10);
            continue;
        }
        Log(LOG_DEBUG, "[SvcListenThreadCommon] received len:%d, ret:%d, conn type:%d, pargbuf:0x%x\n", len, ret, psvcDesc->conn, pargbuf);
        if (len > 0)
        {
            ret = FormatCheckPacketType(pargbuf, len, &rid);
            if (ret == 0)
            {
                /*It's a caller encoded packet*/
                if (InvokerManagerEnqueue(pargbuf, len, pConn) == RPC_SUCCESS_VALUE)
                {
                    RecordRcvCallDebugInfo(psvcDesc->conn);
                }
                else
                {
                    Log(LOG_EMERG, "###RecordRcvBadPacketDebugInfo, caller packet!\n");
                    RecordRcvBadPacketDebugInfo(psvcDesc->conn);
                }
            }
            else if (ret == 1)
            {
                /*It's a return result packet*/
                if (ProcessReturnPacket(pargbuf, len, rid) == RPC_SUCCESS_VALUE)
                {
                    RecordRcvRetDebugInfo(psvcDesc->conn);
                }
                else
                {
                    Log(LOG_EMERG, "###RecordRcvBadPacketDebugInfo, ret packet!\n");
                    RecordRcvBadPacketDebugInfo(psvcDesc->conn);
                }
            }
            else
            {
                Log(LOG_EMERG, "[SvcListenThreadCommon]  !!Received a bad packet!!\n");
                /*Record the debug variable status*/
                RecordRcvBadPacketDebugInfo(psvcDesc->conn);

            }
            if (pConn->conn == MCAPI_PKTCHAN && pargbuf)
            {
                Log(LOG_DEBUG, "SvcListenThreadCommon, will call McapiPktFree!\n");
                McapiPktFree(pargbuf);
                pargbuf = NULL;
                Log(LOG_DEBUG, "SvcListenThreadCommon, McapiPktFree completed!\n");
            }
        }
        Log(LOG_DEBUG, "[SvcListenThreadCommon] process data end!\n");

    }

    /*Free allocated buffer*/
    if (argbuf)
    {
        PR_Free(argbuf);
    }

}

/*
* This method creates the monitor thread suspending on the specified communication port.
* Return: (<0)->Failed,  other value->Success
*/
SvcListenHandle SvcListenCreate(mcapi_node_t localNode, mcapi_port_t localRecvPort, ConnType conn)
{
    SvcListenDesc       *psvcDesc = NULL;
    char name[64] = {0};

    if (g_svcListenNum >= RPC_CHANNEL_MAX)
    {
        return    -1;
    }
    psvcDesc = &svcListen[g_svcListenNum];
    psvcDesc->node = localNode;
    psvcDesc->port = localRecvPort;
    psvcDesc->conn = conn;
    psvcDesc->connHandle = g_svcListenNum;

    /*!!Do we need to initialize Mutex "tid" ? seems we donn't use it now*/
    if (conn == MCAPI_MSG)
    {
        sprintf(name, "rpc_listen_msg");
    }
    else
    {
        sprintf(name, "rpc_listen_pkt");
    }
    if (PR_ThreadCreate(&psvcDesc->tid, (ThreadFunc)&SvcListenThreadCommon, (void *)psvcDesc, 0, 0, name) != RPC_OS_OK)
    {
        return  RPC_OS_ERR_THREAD;
    }

    g_svcListenNum++;
    return (g_svcListenNum - 1);

}

/*
* This method destroys the specific service manage thread, stopping monitor the corresponding
* communication port.
* Return: 0->Success,  other value->Failed.
*/
Int32 SvcListenDestroy(SvcListenHandle handle)
{
    /*Doesn't need to do anything now.*/

    return  RPC_SUCCESS_VALUE;
}

/*This interface dynamically adds/remove the given symbol into the RPC look up table,
  * but now we donn't support it, maybe next version we will support it. */
Int32 RpcAddService(RpcSymbol *symbol)
{
    /*Doesn't implemented now!!!!!*/
    return  RPC_SUCCESS_VALUE;
}

Int32 RpcRemoveService(Uint32 hash)
{
    /*Doesn't implemented now!!!!!*/
    return  RPC_SUCCESS_VALUE;
}

/*
* Findout the mached hash value service address for calling.
* Return: NULL->Failed,  other value->Success.
*/
PRpcFunc RpcLookup(Uint32 hash)
{
    HashEntry *pentry = HashTableLookup(g_ht, &hash);
    
    if (pentry)
    {
        if (pentry->value)
        {
            return ((PRpcFunc)pentry->value);
        }
        else
        {
            return    NULL;
        }
    }
    else
    {
        return    NULL;
    }
}


 
