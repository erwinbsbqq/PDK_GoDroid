/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *
 *  File: ali_rpc.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#if defined(__ALI_LINUX__)
#include <stdarg.h>
#endif

/* If we are compiling linux kernel, export symbols for modules.
 * Date:2014.06.17 by Jingang Chu.
*/
#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)
#include <linux/module.h>
#include <linux/init.h>
#endif

#include <ali_rpcng.h>

#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
#include <../mbx/shm_comm_hw.h>

Int32 g_local_id = 0;
Int32 g_remote_id = 0;
Semaphore  g_ExitMinirpcSem;
#endif

/*Global Variable Defination*/
Int32   g_local_cpu_id = 0; 
Int32   g_remote_cpu_id = 0; 
Int32   g_RPCOS = RPC_OS_TAG;           /*0->means Linux, 1->means TDS*/

#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
Int32   g_RPCRunning = RPCST_MINIRPC_IDLE;    
#else
Int32   g_RPCRunning = RPCST_IDLE;      /* 0->Idle, 1->Running, 2->Initialing, 3->Stopping*/
#endif

Int32   g_RPCVersion = RPC_VERSION;
Int32   g_dbglevel = DEBUG_LEVEL;
McapiConn   g_mcapiconn_rcv[RPC_CHANNEL_MAX];   /*For receiving from peer*/
McapiConn   g_mcapiconn_send[RPC_CHANNEL_MAX];  /*For sending to peer*/
Mutex   g_APIMutex;   /*RPC API mutex*/
Int32   g_BigEndian = 0;

/*For debug usage variables*/
Uint32  g_TotalSendMsgCall = 0;
Uint32  g_TotalRcvMsgCall = 0;
Uint32  g_TotalRcvMsgRet = 0;
Uint32  g_TotalRcvBadMsgPacket = 0;

Uint32  g_TotalSendPktCall = 0;
Uint32  g_TotalRcvPktCall = 0;
Uint32  g_TotalRcvPktRet = 0;
Uint32  g_TotalRcvBadPktPacket = 0;

Uint32  g_TotalRcvBadFuncIDCall = 0;
Uint32  g_TotalRunningInvoker = 0;
Uint32  g_TotalSleepInvoker = 0;

Uint32  g_TotalService = 0;
Uint32  g_TotalserviceQtask = 0;
mcapi_version_t g_McapiVersion = 0;

extern Int32    g_svcListenNum;
extern ReturnBuffer returnBufferManager;

/*1->Big endian, 0->little endian*/
static int checkCPUendian(void)
{
	  /*comment*/
    union
    {
        unsigned long int i;
        unsigned char s[4];
    } c;
    
    c.i = 0x12345678;
    return (0x12 == c.s[0]);
}


static void PrepareDebugInfo(RPCDBGINFO *pinfo)
{
    pinfo->OS = g_RPCOS;
    pinfo->RunningStatus = g_RPCRunning;
    pinfo->rpcversion = g_RPCVersion;
    pinfo->mcapiversion = g_McapiVersion;
    pinfo->totalrunningInvoker = g_TotalRunningInvoker;
    pinfo->totalsleepInvoker = g_TotalSleepInvoker;
    pinfo->totalserviceQtask = g_TotalserviceQtask;
    pinfo->totalRegisteredService = g_TotalService;
    pinfo->totalsendmsgcall = g_TotalSendMsgCall;
    pinfo->totalsendpktcall = g_TotalSendPktCall;
    pinfo->totalrcvmsgcall = g_TotalRcvMsgCall;
    pinfo->totalrcvpktcall = g_TotalRcvPktCall;
    pinfo->totalrcvmsgret = g_TotalRcvMsgRet;
    pinfo->totalrcvpktret = g_TotalRcvPktRet;
    pinfo->totalrcvbadmsgpacket = g_TotalRcvBadMsgPacket;
    pinfo->totalrcvbadpktpacket = g_TotalRcvBadPktPacket;
    pinfo->totalrcvbadfuncidcall = g_TotalRcvBadFuncIDCall;

}

/*
* For Convinent Debug RPC Internal Usage Service
* Return: 0->Success, other value->Failed.
*/
Int32 RPC_DBGService(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
    //TBD...

    return  RPC_SUCCESS_VALUE;
}


/*
* Get RPC Local or Remote side detail running status info.
* Return: 0->Success, else value->Failed
*/
Int32 RpcGetDetailInfo(RPCDBGINFO *pinfo, Bool ifremote)
{
    if (!pinfo)
    {
        Log(LOG_ERR, "RpcGetDetailInfo argument is invalid!\n");
        return RPC_ERROR_VALUE;
    }
    if (!ifremote)
    {
        /*Get local debug info*/
        PrepareDebugInfo(pinfo);
        return RPC_SUCCESS_VALUE;
    }
    else
    {
        /*Get remote debug info*/
        if (g_RPCRunning != RPCST_RUNNING)
        {
            Log(LOG_DEBUG, "RpcGetDetailInfo, doesn't be initialed!\n");
            return RPC_ERROR_VALUE;
        }

        //TBD...
    }

    return  RPC_SUCCESS_VALUE;
}


/*
* Below are RPC Run-time Interface
*/
Int32 RpcSend(RpcCd *cd, Void *buf, Size_t size)
{
    //Doesn't implemented now.

    return  RPC_SUCCESS_VALUE;
}

/*Get Rpc Version*/
Int32 RpcVersion(void)
{
    return g_RPCVersion;
}

/*Set debug level*/
void RpcDbgLevel(Int32 level)
{
    /*Set debug level*/
    if (level >= LOG_EMERG && level <= LOG_DEBUG)
    {
        g_dbglevel = level;
    }
}


/*-----------------------------------------------------------
* RPC Init Function, user must call this before call any other RPC Function.
* This function will initializes the MCAPI node, return buffer initialization,
* Invoker Manager initialization, default service listeners initialization, and
* other resource initialization.
*
* Return: 0->Success,  other value->Failed.
*------------------------------------------------------------*/
#if defined(__ALI_LINUX__)   /*for x86 64bit CPU linux user space debug usage*/
Int32 RpcInit(McapiNode node, McapiNode rnode)
#else
Int32 RpcInit(McapiNode node)
#endif
{
    McapiNode localNode;
    McapiNode remoteNode;
    
    /*!!Now we will ignore the argument "node", we will use constant predefine node value for main and see cpu*/

    g_BigEndian = checkCPUendian();

    /*For TDS Debug*/
    Log(LOG_DEBUG, "\n\n********Entered RpcInit, g_BigEndian:%d ******\n", g_BigEndian);


#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
    if (g_RPCRunning != RPCST_MINIRPC_IDLE)
#else
    if (g_RPCRunning != RPCST_IDLE)
#endif
    {
        Log(LOG_DEBUG, "[RpcInit] Had been Initialized in other place, g_RPCRunning:%d\n", g_RPCRunning);
        return  RPC_ERROR_VALUE;
    }

#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
    g_RPCRunning = RPCST_MINIRPC_INITIALING;
#else
    /*Set rpc overall status*/
    g_RPCRunning = RPCST_INITIALING;
#endif

#if defined(__ALI_LINUX__)   /*for x86 64bit CPU debug usage*/
    localNode = node;
    remoteNode = rnode;
#else
    if (g_RPCOS == RPC_OS_LINUX)
    {
        localNode = RPC_LINUX_NODE_ID;
        remoteNode = RPC_TDS_NODE_ID;
    }
    else
    {
        localNode = RPC_TDS_NODE_ID;
        remoteNode = RPC_LINUX_NODE_ID;

#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
		g_local_id = SEE_CPU_ID;
        g_remote_id = MAIN_CPU_ID;
#endif

    }
#endif

    Log(LOG_DEBUG, "[RpcInit] g_RPCOS:%d, localNode:%d, remoteNode:%d\n", g_RPCOS, localNode, remoteNode);

    /*Init Mcapi lib*/
    if (McapiInit(localNode, &g_McapiVersion) < 0)
    {
        Log(LOG_ERR, "[RpcInit] McapiInit Failed!\n");
        g_RPCRunning = RPCST_IDLE;
        return  RPC_ERROR_VALUE;
    }
    /*Configure Init*/
    if (CfgInit() < 0)
    {
        Log(LOG_ERR, "[RpcInit] CfgInit Failed!\n");
        McapiDeinit();
        return  RPC_ERROR_VALUE;
    }
    Log(LOG_DEBUG, "[RPcInit] will start RetInit()\n");
    /*Return buffer initialization*/
    if (RetInit() < 0)
    {
        Log(LOG_ERR, "[RpcInit] CfgInit Failed!\n");
        CfgDeinit();
        McapiDeinit();
        return  RPC_ERROR_VALUE;
    }

    if (PR_MutexInit(&g_APIMutex) != RPC_OS_OK)
    {
        Log(LOG_ERR, "RPCInit g_APIMutex Init Failed I!\n");
        CfgDeinit();
        McapiDeinit();
        return RPC_OS_ERR_MUTEX;
    }


    Log(LOG_DEBUG, "[RPcInit] will start InvokerManagerInit()\n");
    /*Invoker Manager initialization*/
    if (InvokerManagerInit() < 0)
    {
        Log(LOG_ERR, "[RpcInit] InvokerManagerInit Failed!\n");
        RetDeinit();
        CfgDeinit();
        McapiDeinit();
        return  RPC_INVOKER_ERR;
    }


#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
	/*MiniRpc Stage Entry*/
	if (MiniRpcEntry() < 0)
    {
        Log(LOG_ERR, "[RpcInit] InvokerManagerInit Failed!\n");
        RetDeinit();
        CfgDeinit();
        McapiDeinit();
		InvokerManagerDeinit();
        return  RPC_INVOKER_ERR;
    }

	//Log(LOG_EMERG, "g_TotalRcvMsgCall=%d\n",g_TotalRcvMsgCall);
	//Log(LOG_EMERG, "g_TotalRcvMsgRet=%d\n",g_TotalRcvMsgRet);
	//Log(LOG_EMERG, "g_TotalRcvBadMsgPacket=%d\n",g_TotalRcvBadMsgPacket);
	
	 g_TotalRcvMsgCall = 0;
	 g_TotalRcvMsgRet = 0;
	 g_TotalRcvBadMsgPacket = 0;
#endif
   


    /*here already finish minirpc, enter normal rpcng stage*/
    Log(LOG_ERR, "[RPcInit] will Enter Normal RPCng Stage\n");
	
    /*Mcapi Packet Channel Receive channel create*/
    memset(&g_mcapiconn_rcv[MCAPI_PKTCHAN], 0 , sizeof(McapiConn));
    g_mcapiconn_rcv[MCAPI_PKTCHAN].block = True;
    g_mcapiconn_rcv[MCAPI_PKTCHAN].conn = MCAPI_PKTCHAN;
    g_mcapiconn_rcv[MCAPI_PKTCHAN].local.node = localNode;
    g_mcapiconn_rcv[MCAPI_PKTCHAN].local.port = RPC_RECV_PORT2;
    g_mcapiconn_rcv[MCAPI_PKTCHAN].remote.node = remoteNode;
    g_mcapiconn_rcv[MCAPI_PKTCHAN].remote.port = RPC_SEND_PORT2;
    g_mcapiconn_rcv[MCAPI_PKTCHAN].dir = 1;
    if (PR_MutexInit(&g_mcapiconn_rcv[MCAPI_PKTCHAN].mutex) != RPC_OS_OK)
    {
        Log(LOG_ERR, "RpcInit MutexInit Failed III!\n");
        return RPC_OS_ERR_MUTEX;
    }
#if defined(CONFIG_ALI_MBX_AS) 
    ;
#else

    if (McapiOpen_Pre(&g_mcapiconn_rcv[MCAPI_PKTCHAN], True, 0) < 0)
    {
        return    RPC_MCAPI_ERR;
    }
#endif
    /*Mcapi Packet Channel Send channel create*/
    memset(&g_mcapiconn_send[MCAPI_PKTCHAN], 0 , sizeof(McapiConn));
    g_mcapiconn_send[MCAPI_PKTCHAN].block = True;
    g_mcapiconn_send[MCAPI_PKTCHAN].conn = MCAPI_PKTCHAN;
    g_mcapiconn_send[MCAPI_PKTCHAN].local.node = localNode;
    g_mcapiconn_send[MCAPI_PKTCHAN].local.port = RPC_SEND_PORT2;
    g_mcapiconn_send[MCAPI_PKTCHAN].remote.node = remoteNode;
    g_mcapiconn_send[MCAPI_PKTCHAN].remote.port = RPC_RECV_PORT2;
    g_mcapiconn_send[MCAPI_PKTCHAN].dir = 0;
    if (PR_MutexInit(&g_mcapiconn_send[MCAPI_PKTCHAN].mutex) != RPC_OS_OK)
    {
        Log(LOG_ERR, "RpcInit MutexInit Failed III!\n");
        return RPC_OS_ERR_MUTEX;
    }
 
#if defined(CONFIG_ALI_MBX_AS) 
    Log(LOG_DEBUG, "RpcInit CONFIG_ALI_MBX_AS rule!\n");

#else

   if (McapiOpen_Pre(&g_mcapiconn_send[MCAPI_PKTCHAN], True, 0) < 0)
    {
        return  RPC_MCAPI_ERR;
    }

    PR_uSleep(20000);
    if (g_RPCOS == RPC_OS_LINUX)
    {
    	if (McapiOpen(&g_mcapiconn_send[MCAPI_PKTCHAN], True, 0) < 0)
    	{
        	return  RPC_MCAPI_ERR;
    	}
    	Log(LOG_ERR,"mcapiopen send waitting ... \n");
    	McapiWait(&g_mcapiconn_send[MCAPI_PKTCHAN], 0xffffffff);
    	Log(LOG_ERR,"mcapiopen send wait return\n");
       PR_uSleep(20000);
   	if (McapiOpen(&g_mcapiconn_rcv[MCAPI_PKTCHAN], True, 0) < 0)
    	{
        	return  RPC_MCAPI_ERR;
    	}
    	Log(LOG_ERR,"mcapiopen rcv waitting ...\n");
    	McapiWait(&g_mcapiconn_rcv[MCAPI_PKTCHAN], 0xffffffff);
    	Log(LOG_ERR,"mcapiopen rcv wait return\n");
    }
    else
    {
	if (McapiOpen(&g_mcapiconn_rcv[MCAPI_PKTCHAN], True, 0) < 0)
        {
                return  RPC_MCAPI_ERR;
        }
        Log(LOG_ERR,"mcapiopen rcv waitting ... \n");
        McapiWait(&g_mcapiconn_rcv[MCAPI_PKTCHAN], 0xffffffff);
        Log(LOG_ERR,"mcapiopen rcv wait return\n");
        PR_uSleep(20000);
	if (McapiOpen(&g_mcapiconn_send[MCAPI_PKTCHAN], True, 0) < 0)
        {
                return  RPC_MCAPI_ERR;
        }
        Log(LOG_ERR,"mcapiopen send waitting ...\n");
        McapiWait(&g_mcapiconn_send[MCAPI_PKTCHAN], 0xffffffff);
        Log(LOG_ERR,"mcapiopen send wait return\n");
    }
#endif

    if (SvcListenCreate(localNode, RPC_RECV_PORT2, MCAPI_PKTCHAN) < 0)
    {
        Log(LOG_ERR, "[RpcInit] SvcListenCreate MCAPI_PKTCHAN Failed!\n");
        return  RPC_SERVICE_ERR;
    }

    
#if defined(CONFIG_ALI_MBX_AS)
    McapiOpen(&g_mcapiconn_send[MCAPI_PKTCHAN], True, 0);
    g_RPCRunning = RPCST_RUNNING;
    PR_uSleep(10000); //10ms delay
#else
    g_RPCRunning = RPCST_RUNNING;
    PR_uSleep(50000);
#endif
    Log(LOG_ERR, "RpcInit completed!\n");
    
    return  RPC_SUCCESS_VALUE;
}


/*
* release MCAPI resource and return buffer, close all connection if there is
* any MCAPI connection opened.
* Return: 0->Success,  other value->Failed.
*/
Int32 RpcDeinit(void)
{
    Int32 i = 0;

    g_RPCRunning = RPCST_STOPPING;
    /*Sleep 0.5 second or so for threads exit safely*/
    PR_Sleep(500);
    /*Release resource*/
    for (i = 0; i < g_svcListenNum; i++)
    {
        SvcListenDestroy(i);
    }
    InvokerManagerDeinit();
    RetDeinit();
    CfgDeinit();
    McapiDeinit();

    g_RPCRunning = RPCST_IDLE;

    return  RPC_SUCCESS_VALUE;
}


/*-----------------------------------------------------------
* This function will initialize service listener and preare other resource for
* MCAPI layer communication.  Now, we just support the default port,
* so, this function will ignor these port arguments, this just for later
* expansion usage.
*
* Return: 0->Success,  other value->Failed.
*------------------------------------------------------------*/
Int32 RpcOpen(McapiNode localNode, McapiPort localSendPort, McapiPort localRecvPort,
              McapiNode remoteNode, McapiPort remoteSendPort, McapiPort remoteRecvPort,
              Int32 connType, RpcCd *cd)
{
    PR_Lock(&g_APIMutex);


    if (g_RPCRunning != RPCST_RUNNING)
    {
        Log(LOG_ERR, "RpcInit() doesn't be called, g_RPCRunning:%d\n", g_RPCRunning);
        PR_Unlock(&g_APIMutex);
        return RPC_ERROR_VALUE;
    }

    /*!!Now we use the constant predefined node and port id and had been initializd when call "RpcInit()
           so, we donn't need to do too much thing.*/
    if (connType < MCAPI_MSG || connType > MCAPI_PKTCHAN)
    {
        Log(LOG_ERR, "[RpcOpen] connType is invalid!!\n");
        PR_Unlock(&g_APIMutex);
        return  RPC_ERROR_VALUE;
    }
    cd->connHandle = connType;  /*Doesn't be used now.*/
    cd->connType = connType;

    PR_Unlock(&g_APIMutex);

    return  RPC_SUCCESS_VALUE;
}

/*
* Will release the connection related resource.
* Return: 0->Success,  other value->Failed.
*/
Int32 RpcClose(RpcCd *rpcd)
{
    /*!!Now we use the constant predefined node and port id and had been initializd when call "RpcInit()
           so, we donn't need to do anything.*/

    return  RPC_SUCCESS_VALUE;
}


/*-----------------------------------------------------------
* RPC Call function, this is a nonsynchronous call, doesn't need to wait the
* remote service returning, the user must call RpcWait() to get the status.
*
* Return:  RPC_ERROR_VALUE ->Failed,  other value->Success got rid
*------------------------------------------------------------*/
//Int32 __rpc_call(RpcCd *rcd, Uint32 funcid, Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
Int32 __rpc_call(RpcCd *rcd, Uint32 funcid, ...)
{
    Int32 rid = 0, encodedlen = 0, i = 0;
    static Uint8 buf[RPC_MAX_SEND_BUF] = {0};
    Param *pargs[8];
    va_list Va;
    RpcCd df_rcd;

    PR_Lock(&g_APIMutex);

    if (g_RPCRunning != RPCST_RUNNING)
    {
        Log(LOG_ERR, "[__rpc_call] !!the RpcInit() doesn't be called! g_RPCRunning:%d\n", g_RPCRunning);
        PR_Unlock(&g_APIMutex);
        return RPC_ERROR_VALUE;
    }

    memset(&df_rcd, 0, sizeof(RpcCd));
    df_rcd.connType = MCAPI_PKTCHAN;
    if (!rcd)
    {
        Log(LOG_DEBUG, "[__rpc_call]Will use default Pkt channel to do communication!\n");
        rcd = &df_rcd;
    }


    /*Parse the incoming Param* arguments*/
    memset(pargs, 0, sizeof(pargs));
    va_start(Va, funcid);
    for (i = 0; i < 8; i++)
    {
        pargs[i] = va_arg(Va, Param *);
        if (pargs[i] == NULL)
        {
            Log(LOG_DEBUG, "__rpc_call parsed input arguemnt received NULL terminated flag! i:%d\n", i);
            break;
        }
    }
    va_end(Va);

    Log(LOG_DEBUG, "[__rpc_call] pargs[0]:0x%x, pargs[1]:0x%x, pargs[2]:0x%x, pargs[3]:0x%x, pargs[4]:0x%x, pargs[5]:0x%x, pargs[6]:0x%x,pargs[7]:0x%x\n", 
    pargs[0], pargs[1], pargs[2], pargs[3], pargs[4], pargs[5], pargs[6], pargs[7]);

    if ((rid = RetAlloc()) < 0)
    {
        Log(LOG_ERR, "[__rpc_call] !!The simultaneously calling is over the limitaion!!\n");
        PR_Unlock(&g_APIMutex);
        return  RPC_ERROR_VALUE;
    }

    /*Encode the arguments*/
    if ((encodedlen = FormatCallerArgsEncode(buf, RPC_MAX_SEND_BUF, funcid, rid, XFLG_NONSYN, pargs)) < 0)
    {
        Log(LOG_ERR, "[__rpc_call] !!Encode arguments failed!\n");
        RpcFreeRetBuf(rid);
        PR_Unlock(&g_APIMutex);
        return  RPC_ERROR_VALUE;
    }
    /*Send packet to peer*/
    if (rcd->connType == MCAPI_MSG)
    {
#if defined(CONFIG_ALI_MBX_AS) 
         ;
#else
        if (McapiMsgWrite(&g_mcapiconn_send[rcd->connType], buf, encodedlen) <= 0)
        {
            Log(LOG_ERR, "[__rpc_call] !! McapiMsgWrite failed!\n");
            RpcFreeRetBuf(rid);
            PR_Unlock(&g_APIMutex);
            return  RPC_ERROR_VALUE;
        }
#endif
        /*Record the debug variable status*/
        g_TotalSendMsgCall++;
    }
    else if (rcd->connType == MCAPI_PKTCHAN)
    {
        if (McapiPktWrite(&g_mcapiconn_send[rcd->connType], buf, encodedlen) <= 0)
        {
            Log(LOG_ERR, "[__rpc_call] !! McapiMsgWrite failed!\n");
            RpcFreeRetBuf(rid);
            PR_Unlock(&g_APIMutex);
            return  RPC_ERROR_VALUE;
        }
        /*Record the debug variable status*/
        g_TotalSendPktCall++;
    }
    else
    {
        Log(LOG_ERR, "[__rpc_call] !!Doesn't support this connet type:%d \n", rcd->connType);
        RpcFreeRetBuf(rid);
        PR_Unlock(&g_APIMutex);
        return  RPC_ERROR_VALUE;
    }


    /*Donn't need to wait the peer response*/

    PR_Unlock(&g_APIMutex);

    return  rid;

}


/*-----------------------------------------------------------
* RPC Call function, this is a synchronous call, it will wait the
* remote service returning before returnning to the caller.
*
* Return:  RPC_ERROR_VALUE->Failed,  other value->Success or timeout
*------------------------------------------------------------*/
//Int32 __rpc_call_completion(RpcCd *rcd, Int32 *result, Uint32 funcid, Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, 
//Param *arg7, Param *arg8)
Int32 __rpc_call_completion(RpcCd *rcd, Uint32 funcid, ...)
{
    Int32 rid = 0, encodedlen = 0, ret = 0, i = 0;
    static Uint8 buf[RPC_MAX_SEND_BUF] = {0};
    Param *pargs[8];
    va_list Va;
    RpcCd df_rcd;

    PR_Lock(&g_APIMutex);

    if (g_RPCRunning != RPCST_RUNNING)
    {
        Log(LOG_ERR, "[__rpc_call_completion] !!the RpcInit() doesn't be called! g_RPCRunning:%d\n", g_RPCRunning);
        PR_Unlock(&g_APIMutex);
        return RPC_ERROR_VALUE;
    }

    memset(buf, 0, RPC_MAX_SEND_BUF);
    memset(&df_rcd, 0, sizeof(RpcCd));
    df_rcd.connType = MCAPI_PKTCHAN;
    if (!rcd)
    {
        Log(LOG_DEBUG, "[__rpc_call_completion]Will use default Pkt channel to do communication!\n");
        rcd = &df_rcd;
    }

    /*Parse the incoming Param* arguments*/
    memset(pargs, 0, sizeof(pargs));
    va_start(Va, funcid);
    for (i = 0; i < 8; i++)
    {
        pargs[i] = va_arg(Va, Param *);
        if (pargs[i] == NULL)
        {
            Log(LOG_DEBUG, "__rpc_call parsed input arguemnt received NULL terminated flag! i:%d\n", i);
            break;
        }
    }
    va_end(Va);

    Log(LOG_DEBUG, "[%s] pargs[0]:0x%x, pargs[1]:0x%x, pargs[2]:0x%x, pargs[3]:0x%x, pargs[4]:0x%x, pargs[5]:0x%x, pargs[6]:0x%x,pargs[7]:0x%x\n", 
    __func__, pargs[0], pargs[1], pargs[2], pargs[3], pargs[4], pargs[5], pargs[6], pargs[7]);

    if ((rid = RetAlloc()) < 0)
    {
        Log(LOG_ERR, "[__rpc_call_completion] !!The simultaneously calling is over the limitaion!!BITS_PER_RETMAP:%d,nrfree:%d\n", 
        BITS_PER_RETMAP, returnBufferManager.nrFree);
        PR_Unlock(&g_APIMutex);
        return  RPC_ERROR_VALUE;
    }
    Log(LOG_DEBUG, "__rpc_call_completion RetAlloc rid:%d\n", rid);

    /*Encode the arguments*/
    if ((encodedlen = FormatCallerArgsEncode(buf, RPC_MAX_SEND_BUF, funcid, rid, XFLG_SYN, pargs)) < 0)
    {
        Log(LOG_ERR, "[__rpc_call_completion] !!Encode arguments failed!\n");
        RpcFreeRetBuf(rid);
        Log(LOG_ERR, "[__rpc_call_completion] !!Encode arguments failed II\n");
        PR_Unlock(&g_APIMutex);
        Log(LOG_ERR, "[__rpc_call_completion] !!Encode arguments failed III\n");
        return  RPC_ERROR_VALUE;
    }
    Log(LOG_DEBUG, "[__rpc_call_completion] encodedlen:%d, rid:%d\n", encodedlen, rid);

    /*Set returned arguments address to g_pParams for blocking rpccall*/
    SetRetArgsAddress(rid, pargs);

    /*Send packet to peer*/
    if (rcd->connType == MCAPI_MSG)
    {
        Log(LOG_DEBUG, "__rpc_call_completion will do McapiMsgWrite!\n");
#if defined(CONFIG_ALI_MBX_AS) 
       ;
#else 
       if (McapiMsgWrite(&g_mcapiconn_send[rcd->connType], buf, encodedlen) <= 0)
        {
            Log(LOG_ERR, "[__rpc_call_completion] !! McapiMsgWrite failed!\n");
            RpcFreeRetBuf(rid);
            PR_Unlock(&g_APIMutex);
            return  RPC_ERROR_VALUE;
        }
#endif
        /*Record the debug variable status*/
        g_TotalSendMsgCall++;
    }
    else if (rcd->connType == MCAPI_PKTCHAN)
    {
        Log(LOG_DEBUG, "__rpc_call_completion, call McapiPktWrite!\n");
        if (McapiPktWrite(&g_mcapiconn_send[rcd->connType], buf, encodedlen) <= 0)
        {
            Log(LOG_ERR, "[__rpc_call_completion] !! McapiPktWrite failed!\n");
            RpcFreeRetBuf(rid);
            PR_Unlock(&g_APIMutex);
            return  RPC_ERROR_VALUE;
        }
        /*Record the debug variable status*/
        g_TotalSendPktCall++;
    }
    else
    {
        Log(LOG_ERR, "[__rpc_call_completion] !!Doesn't support this connet type:%d \n", rcd->connType);
        RpcFreeRetBuf(rid);
        PR_Unlock(&g_APIMutex);
        return  RPC_ERROR_VALUE;
    }

    /*Must need to wait the peer response*/
    Log(LOG_DEBUG, "\n__rpc_call_completion will wait rid:%d condition varialbe!\n", rid);

    PR_Unlock(&g_APIMutex);


    ret = PR_CondVarWait_Safe(&returnBufferManager.retCond[rid], RPCCALL_TIMEOUT_VALUE);
    if (ret != RPC_OS_OK)
    {
        Log(LOG_ERR, "[__rpc_call_completion] !! CondVarWait happened error or timeout, rid:%d, ret:%d!\n", rid, ret);
        RpcFreeRetBuf(rid);
        return  ret;
    }
    Log(LOG_DEBUG, "__rpc_call_completion, CondVarWait return, rid:%d, ret:%d\n", rid, ret);
    /*Try to get the peer result*/
    ret = RpcExtractRet(rid);

    RpcFreeRetBuf(rid);

    return  ret;

}
/* If we are compiling linux kernel, export symbols for modules.
 * Date:2014.06.17 by Jingang Chu.
*/
#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)
EXPORT_SYMBOL(__rpc_call_completion);
#endif



Bool XDR_Rpcdbginfo(XDR *xdrs, void *cp, Uint32 cnt)
{
    RPCDBGINFO *pdbg = (RPCDBGINFO *)cp;
    
    if (XDR_Int32(xdrs, &pdbg->OS) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->RunningStatus) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->rpcversion) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->mcapiversion) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalrunningInvoker) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalsleepInvoker) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalserviceQtask) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalRegisteredService) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalsendmsgcall) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalsendpktcall) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalrcvmsgcall) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalrcvpktcall) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalrcvmsgret) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalrcvpktret) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalrcvbadmsgpacket) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalrcvbadpktpacket) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &pdbg->totalrcvbadfuncidcall) == False)
    {
        return False;
    }

    return True;
}


#if defined(__ALI_TDS__) && defined(CONFIG_ALI_RPCNG) && defined(CONFIG_ALI_MINIRPC)
int MiniRpcEntry()
{  
    /*Begin:Now is in the MiniRpc stage*/
	g_RPCRunning = RPCST_MINIRPC_INITIALING;

	see_notify_main_minirpc();
	
    PR_SemInit(&g_ExitMinirpcSem, 0);
	
	PR_SemWait(&g_ExitMinirpcSem, 0xFFFFFFFF);

	Log(LOG_ERR, "\n\n+++[%s] MiniRPC Stage has finished!!!+++\n\n", __func__);

	PR_SemDestroy(&g_ExitMinirpcSem);

	g_RPCRunning = RPCST_INITIALING;
	/*End:Now is in the MiniRpc stage*/

    return  RPC_SUCCESS_VALUE;
}
#endif
