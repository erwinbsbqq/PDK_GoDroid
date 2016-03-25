/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *
 *  File: ali_rpc.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 ****************************************************************************/
#include <stdarg.h>
#include "openmcapi.h"
#include <ali_rpcng.h>

#include <../mbx/shm_comm_hw.h>


unsigned int g_local_id = 0;
unsigned int g_remote_id = 0;


/*share mem operation function*/
SHM_OPS *g_shm_ops = NULL;

/*Global Variable Defination*/
global_param g_param=
{
    /*golobal flag and info*/
	.g_RPCOS = RPC_OS_TAG,          /*0->means Linux, 1->means TDS*/
	.g_RPCRunning = RPCST_MINIRPC_IDLE,     /* 0->Idle, 1->Running, 2->Initialing, 3->Stopping*/
	.g_RPCVersion = MINIRPC_VERSION,
	.g_dbglevel = DEBUG_LEVEL,
	.g_BigEndian = 0,
	.g_local_node_id=0,
	.g_remote_node_id=0,

	/*For debug usage variables*/
	.g_TotalSendMsgCall = 0,
	.g_TotalRcvMsgCall = 0,
	.g_TotalRcvMsgRet = 0,
	.g_TotalRcvBadMsgPacket = 0,
	.g_TotalSendPktCall = 0,
	.g_TotalRcvPktCall = 0,
	.g_TotalRcvPktRet = 0,
	.g_TotalRcvBadPktPacket = 0,
	.g_TotalRcvBadFuncIDCall = 0,
	.g_TotalService = 0,
};


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
    pinfo->OS = g_param.g_RPCOS;
    pinfo->RunningStatus = g_param.g_RPCRunning;
    pinfo->rpcversion = g_param.g_RPCVersion;
    pinfo->totalRegisteredService = g_param.g_TotalService;
    pinfo->totalsendmsgcall = g_param.g_TotalSendMsgCall;
    pinfo->totalsendpktcall = g_param.g_TotalSendPktCall;
    pinfo->totalrcvmsgcall = g_param.g_TotalRcvMsgCall;
    pinfo->totalrcvpktcall = g_param.g_TotalRcvPktCall;
    pinfo->totalrcvmsgret = g_param.g_TotalRcvMsgRet;
    pinfo->totalrcvpktret = g_param.g_TotalRcvPktRet;
    pinfo->totalrcvbadmsgpacket = g_param.g_TotalRcvBadMsgPacket;
    pinfo->totalrcvbadpktpacket = g_param.g_TotalRcvBadPktPacket;
    pinfo->totalrcvbadfuncidcall = g_param.g_TotalRcvBadFuncIDCall;
}

/*
* For Convinent Debug RPC Internal Usage Service
* Return: 0->Success, other value->Failed.
*/
Int32 RPC_DBGService(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
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
        if (g_param.g_RPCRunning != RPCST_RUNNING)
        {
            Log(LOG_DEBUG, "RpcGetDetailInfo, doesn't be initialed!\n");
            return RPC_ERROR_VALUE;
        }

        //TBD...
    }

    return  RPC_SUCCESS_VALUE;
}


/*Get Rpc Version*/
Int32 RpcVersion(void)
{
    return g_param.g_RPCVersion;
}

/*
*
*Set debug level
*
*/
void RpcDbgLevel(Int32 level)
{
    /*Set debug level*/
    if (level >= LOG_EMERG && level <= LOG_DEBUG)
    {
        g_param.g_dbglevel = level;
    }
}



/*-----------------------------------------------------------
* MiniRpcConnect
* 
*
* Return:  RPC_ERROR_VALUE->Failed,  other value->Success or timeout
*------------------------------------------------------------*/
Int32 MiniRpcConnect(void)
{  
   if(RPCST_MINIRPC_RUNNING == g_param.g_RPCRunning)
   {
   	   Log(LOG_ERR, "[%s] already connected to see\n",__func__);
	   return RPC_ERROR_VALUE;
   }
   
   Log(LOG_DEBUG, "[%s] now is connecting to see ... \n",__func__);
   
   if(CtrlMsgWrite(CONNECT_REQ_E))
   {
      Log(LOG_DEBUG, "[%s] CtrlMsgWrite fail\n",__func__);
	  return RPC_ERROR_VALUE;
   }

   while(RPCST_MINIRPC_RUNNING != g_param.g_RPCRunning)
   {
      ;//Log(LOG_DEBUG, ".");
   }
   
   Log(LOG_DEBUG, "[%s] Connect to see successful!\n",__func__);
   
   return RPC_SUCCESS_VALUE;
}


/*-----------------------------------------------------------
* MiniRpcDisConnect
* 
*
* Return:  RPC_ERROR_VALUE->Failed,  other value->Success or timeout
*------------------------------------------------------------*/
Int32 MiniRpcDisConnect(void)
{
   if(RPCST_MINIRPC_STOPPING == g_param.g_RPCRunning)
   {
   	   Log(LOG_ERR, "[%s] already disconnect from see\n",__func__);
	   return RPC_ERROR_VALUE;
   }

   Log(LOG_DEBUG, "[%s] now is disconnecting from see ... \n",__func__);
	  
   if(CtrlMsgWrite(DISCONN_REQ_E))
   {
      Log(LOG_ERR, "[%s] CtrlMsgWrite fail\n",__func__);
	  return RPC_ERROR_VALUE;
   }


   while(RPCST_MINIRPC_STOPPING != g_param.g_RPCRunning)
   {
      ; //Log(LOG_DEBUG, ".");
   }
   
   Log(LOG_DEBUG, "MiniRpc Exit!\n");
   
   return RPC_SUCCESS_VALUE;
}



/*-----------------------------------------------------------
* RPC Init Function, user must call this before call any other RPC Function.
* This function will initializes the MCAPI node, return buffer initialization,
* Invoker Manager initialization, default service listeners initialization, and
* other resource initialization.
*
* Return: 0->Success,  other value->Failed.
*------------------------------------------------------------*/
Int32 MiniRpcInit()
{ 	
    g_param.g_BigEndian = checkCPUendian();

    /*For TDS Debug*/
    Log(LOG_DEBUG, "+++++[%s] Entered, g_BigEndian:%d+++++\n",__func__,g_param.g_BigEndian);

    if (g_param.g_RPCRunning != RPCST_MINIRPC_IDLE)
    {
        Log(LOG_DEBUG, "[%s] Had been Initialized in other place, g_RPCRunning:%d\n",__func__,g_param.g_RPCRunning);
        return  RPC_ERROR_VALUE;
    }

    /*Set rpc overall status*/
    g_param.g_RPCRunning = RPCST_MINIRPC_INITIALING;

    
    g_local_id =  MAIN_CPU_ID;
    g_remote_id = SEE_CPU_ID;
 

    /*minirpc shm init*/
	if(minirpc_shm_init()!=0)
	{
	   Log(LOG_ERR, "[%s] minirpc_shm_init Failed!\n",__func__);
       return  RPC_ERROR_VALUE;
	}

    /*Configure Init*/
    if (CfgInit() < 0)
    {
        Log(LOG_ERR, "[%s] CfgInit Failed!\n",__func__);
        return  RPC_ERROR_VALUE;
    }

	/*Return buffer initialization*/
    if (RetInit() < 0)
    {
        Log(LOG_ERR, "[%s] CfgInit Failed!\n",__func__);
        CfgDeinit();
        return  RPC_ERROR_VALUE;
    }
	
    Log(LOG_INFO, "[%s] completed!\n",__func__);

    return  RPC_SUCCESS_VALUE;
}


/*-----------------------------------------------------------
* release MCAPI resource and return buffer, close all connection if there is
* any MCAPI connection opened.
* Return: 0->Success,  other value->Failed.
*------------------------------------------------------------*/
Int32 MiniRpcDeinit(void)
{
    Int32 i = 0;

    g_param.g_RPCRunning = RPCST_MINIRPC_STOPPING;
    RetDeinit();
    CfgDeinit();
    g_param.g_RPCRunning = RPCST_MINIRPC_IDLE;

    return  RPC_SUCCESS_VALUE;
}


/*-----------------------------------------------------------
* RPC Call function, this is a synchronous call, it will wait the
* remote service returning before returnning to the caller.
*
* Return:  RPC_ERROR_VALUE->Failed,  other value->Success or timeout
*------------------------------------------------------------*/
Int32 __rpc_call_completion(Uint32 funcid, ...)
{
    Int32 rid = 0, encodedlen = 0, ret = 0, i = 0;
    static Uint8 buf[RPC_MAX_SEND_BUF] = {0};
    Param *pargs[8];
    va_list Va;
	Int32 buf_idx = 0;

    if (g_param.g_RPCRunning != RPCST_MINIRPC_RUNNING)
    {
        Log(LOG_ERR, "[%s] !!the RpcInit() doesn't be called! g_RPCRunning:%d\n", __func__,g_param.g_RPCRunning);
        return RPC_ERROR_VALUE;
    }

    memset(buf, 0, RPC_MAX_SEND_BUF);
 
    /*Parse the incoming Param* arguments*/
    memset(pargs, 0, sizeof(pargs));
    va_start(Va, funcid);
    for (i = 0; i < 8; i++)
    {
        pargs[i] = va_arg(Va, Param *);
        if (pargs[i] == NULL)
        {
            Log(LOG_DEBUG, "[%s] parsed input arguemnt received NULL terminated flag! i:%d\n",__func__, i);
            break;
        }
    }
    va_end(Va);

    Log(LOG_DEBUG, "[%s] pargs[0]:0x%x, pargs[1]:0x%x, pargs[2]:0x%x, pargs[3]:0x%x, pargs[4]:0x%x, pargs[5]:0x%x, pargs[6]:0x%x,pargs[7]:0x%x\n", 
    __func__, pargs[0], pargs[1], pargs[2], pargs[3], pargs[4], pargs[5], pargs[6], pargs[7]);
	
    if ((rid = RetAlloc()) < 0)
    {
        Log(LOG_ERR, "[%s] !!The simultaneously calling is over the limitaion!!BITS_PER_RETMAP:%d,nrfree:%d\n", 
        __func__,BITS_PER_RETMAP, returnBufferManager.nrFree);
        return  RPC_ERROR_VALUE;
    }
	
    Log(LOG_DEBUG, "[%s] RetAlloc rid:%d\n",__func__, rid);

    /*Encode the arguments*/
    if ((encodedlen = FormatCallerArgsEncode(buf, RPC_MAX_SEND_BUF, funcid, rid, XFLG_SYN, pargs)) < 0)
    {
        Log(LOG_ERR, "[%s] !!Encode arguments failed!\n",__func__);
        RpcFreeRetBuf(rid);
        return  RPC_ERROR_VALUE;
    }
    Log(LOG_DEBUG, "[%s] encodedlen:%d, rid:%d\n", __func__,encodedlen, rid);

	/*Set returned arguments address to g_pParams for blocking rpccall*/
    SetRetArgsAddress(rid, pargs);

	/*Added by Kinson 2013/09/02*/
	if(encodedlen != RpcMsgWrite(buf,encodedlen))
	{
	   Log(LOG_DEBUG, "[%s] RpcMsgWrite error\n", __func__);
	}

	g_param.g_TotalSendPktCall++;

    ret = PR_CondVarWait_Safe(&returnBufferManager.retCond[rid], RPCCALL_TIMEOUT_VALUE);
    if (ret != RPC_OS_OK)
    {
        Log(LOG_ERR, "[%s] !! CondVarWait happened error or timeout, rid:%d, ret:%d!\n",__func__, rid, ret);
        RpcFreeRetBuf(rid);
        return  ret;
    }

	/*Try to get the peer result*/
    ret = RpcExtractRet(rid);

	Log(LOG_DEBUG, "[%s], CondVarWait return ok, rid:%d, ret:%d\n",__func__, rid, ret);

    RpcFreeRetBuf(rid);

    return  ret;

}


/*-----------------------------------------------------------
*XDR_Rpcdbginfo
*------------------------------------------------------------*/
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

/*-----------------------------------------------------------
*Demo Services for test
*------------------------------------------------------------*/
static Int32 Demo_Service1(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
        Log(LOG_INFO, "<MAIN> Entered Demo_Service1 execution!\n");

        return  0;
}

static Int32 Demo_Service2(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
        Log(LOG_INFO, "<MAIN> Entered Demo_Service2 execution!\n");

        return  0;
}

static Int32 Demo_Service3(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
        Log(LOG_INFO, "<MAIN> Entered Demo_Service3 execution!\n");

        return  0;
}

Int32 Demo_Service_Test0(void)
{  
	Param_Minirpc_Test test_pp1;
#if 1
	Param_Minirpc_Test test_pp2;
	Param_Minirpc_Test test_pp3;
	Param_Minirpc_Test test_pp4;
	
	Param_Minirpc_Test test_pp5;
	Param_Minirpc_Test test_pp6;
	Param_Minirpc_Test test_pp7;
	Param_Minirpc_Test test_pp8;
#endif
	Int32 ret;
	
	int i=0;

	test_pp1.x0=0xa0;
	test_pp1.x1=0xb1;
	test_pp1.x2=0xc2;
	test_pp1.x3=0xd3;
	for(i=0;i<sizeof(test_pp1.array)/sizeof(test_pp1.array[0]);i++)
	{
	   test_pp1.array[i]=i;
	}
#if 1
	test_pp2.x0=0x12;
	test_pp2.x1=0x34;
	test_pp2.x2=0x56;
	test_pp2.x3=0x78;
	for(i=0;i<sizeof(test_pp2.array)/sizeof(test_pp2.array[0]);i++)
	{
	   test_pp2.array[i]=i;
	}

	test_pp3.x0=0xa0;
	test_pp3.x1=0xb1;
	test_pp3.x2=0xc2;
	test_pp3.x3=0xd3;
	for(i=0;i<sizeof(test_pp3.array)/sizeof(test_pp3.array[0]);i++)
	{
	   test_pp3.array[i]=i;
	}

	test_pp4.x0=0x12;
	test_pp4.x1=0x34;
	test_pp4.x2=0x56;
	test_pp4.x3=0x78;
	for(i=0;i<sizeof(test_pp4.array)/sizeof(test_pp4.array[0]);i++)
	{
	   test_pp4.array[i]=i;
	}



	test_pp5.x0=0xa0;
	test_pp5.x1=0xb1;
	test_pp5.x2=0xc2;
	test_pp5.x3=0xd3;
	for(i=0;i<sizeof(test_pp5.array)/sizeof(test_pp5.array[0]);i++)
	{
	   test_pp5.array[i]=i;
	}

	test_pp6.x0=0x12;
	test_pp6.x1=0x34;
	test_pp6.x2=0x56;
	test_pp6.x3=0x78;
	for(i=0;i<sizeof(test_pp6.array)/sizeof(test_pp6.array[0]);i++)
	{
	   test_pp6.array[i]=i;
	}

	test_pp7.x0=0xa0;
	test_pp7.x1=0xb1;
	test_pp7.x2=0xc2;
	test_pp7.x3=0xd3;
	for(i=0;i<sizeof(test_pp7.array)/sizeof(test_pp7.array[0]);i++)
	{
	   test_pp7.array[i]=i;
	}

	test_pp8.x0=0x12;
	test_pp8.x1=0x34;
	test_pp8.x2=0x56;
	test_pp8.x3=0x78;
	for(i=0;i<sizeof(test_pp8.array)/sizeof(test_pp8.array[0]);i++)
	{
	   test_pp8.array[i]=i;
	}
#endif	
	MINIRPC_PARAM_CREATE(p1, PARAM_INOUT, PARAM_MINIRPC_TEST, sizeof(Param_Minirpc_Test), &test_pp1);
#if 1
	MINIRPC_PARAM_CREATE(p2, PARAM_INOUT, PARAM_MINIRPC_TEST, sizeof(Param_Minirpc_Test), &test_pp2);
	MINIRPC_PARAM_CREATE(p3, PARAM_INOUT, PARAM_MINIRPC_TEST, sizeof(Param_Minirpc_Test), &test_pp3);
	MINIRPC_PARAM_CREATE(p4, PARAM_INOUT, PARAM_MINIRPC_TEST, sizeof(Param_Minirpc_Test), &test_pp4);

	MINIRPC_PARAM_CREATE(p5, PARAM_INOUT, PARAM_MINIRPC_TEST, sizeof(Param_Minirpc_Test), &test_pp5);
	MINIRPC_PARAM_CREATE(p6, PARAM_INOUT, PARAM_MINIRPC_TEST, sizeof(Param_Minirpc_Test), &test_pp6);
	MINIRPC_PARAM_CREATE(p7, PARAM_OUT, PARAM_MINIRPC_TEST, sizeof(Param_Minirpc_Test), &test_pp7);
	MINIRPC_PARAM_CREATE(p8, PARAM_INOUT, PARAM_MINIRPC_TEST, sizeof(Param_Minirpc_Test), &test_pp8);
#endif
    ret = RpcCallCompletion(Demo_Minirpc_Service_Test0,&p1,&p2,&p3,&p4,&p5,&p6,&p7,&p8)
    //ret = RpcCallCompletion(Demo_Minirpc_Service_Test0,&p1,NULL)
	if(!ret)
    {
    #if 1
        printf("return value:test_pp7.x0=0x%x\n",test_pp7.x0);
		printf("return value:test_pp7.x1=0x%x\n",test_pp7.x1);
		printf("return value:test_pp7.x2=0x%x\n",test_pp7.x2);
		printf("return value:test_pp7.x3=0x%x\n",test_pp7.x3);
		
        printf("return value:test_pp8.x0=0x%x\n",test_pp8.x0);
		printf("return value:test_pp8.x1=0x%x\n",test_pp8.x1);
		printf("return value:test_pp8.x2=0x%x\n",test_pp8.x2);
		printf("return value:test_pp8.x3=0x%x\n",test_pp8.x3);
	#endif
	printf("ret 0\n");
    }
	else
	{
	    printf("Demo_Service_Test0 error!\n");
	}
}

EXPORT_RPC(Demo_Service1);
EXPORT_RPC(Demo_Service2);
EXPORT_RPC(Demo_Service3);

 
