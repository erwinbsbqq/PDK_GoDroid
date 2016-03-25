#ifndef __ALI_RPC_NG_H
#define __ALI_RPC_NG_H

#include "ali_rpc_type.h"
#include "ali_rpc_errno.h"
#include "ali_rpc_cfg.h"
#include "ali_rpc_osdep.h"
#include "ali_rpc_debug.h"
#include "ali_rpc_util.h"
#include "ali_rpc_xdr.h"
#include "ali_minirpc_service.h"
#include "ali_rpc_argsformat.h"
#include "ali_rpc_mcapi.h"
#include "ali_rpc_invoker.h"
#include "ali_rpc_servicelisten.h"
#include "ali_rpc_ctrl.h"
#include "ali_shm.h"
#include "../mbx/shm.h"


#define RPC_MAX_SEND_BUF   		        16*1024
#define RPC_MAX_SEND_BUF_PER_MSG   		256

extern SHM_OPS *g_shm_ops;


typedef enum {
	RPC_ACCEPTED,
	RPC_NOT_EXISTED,
	RPC_PARAM_ERR,
	RPC_STATUS_END
}RpcStatusCode;


Int32 MiniRpcInit(void);

Int32 MiniRpcDeinit();

/*The below definitions are to call a RPC service,*/
Int32 __rpc_call(Uint32, ...);
#define RpcCall( FUNC, ARG...) \
		__rpc_call(HASH_STR(FUNC), ##ARG);

Int32 __rpc_call_completion(Uint32, ...);
#define RpcCallCompletion( FUNC, ARG...) \
		__rpc_call_completion(HASH_STR(FUNC), ##ARG);

Int32 RpcWait(Int32 rid, Uint32 timeout);

Int32 RpcFreeRetBuf(Int32 rid);

Int32 RpcExtractRet(Int32 rid);

Int32 RpcMsgWrite(Uint8* buf, Uint32 bufsize);

#define HASH_STR(x) hash(#x)


#if defined(__ALI_LINUX_KERNEL__)
  #define EXPORT_RPC(sym)                               \
        extern typeof(sym) sym;                                 \
        struct _RpcSymbol __rpctab_##sym __attribute__((unused, section(".___rpctab"),aligned(4)))= {  #sym, 0, (unsigned long)sym }
#else
  #define EXPORT_RPC(sym)					\
	static const struct _RpcSymbol __rpctab_##sym		\
	__attribute__((unused, section(".___rpctab")))		\
	= { .name = #sym, \
	    .func = (unsigned long)sym }	
#endif

/*For RPC Debug Usage Interface*/
typedef struct _RPCDBGINFO{
	Int32	OS;      	   /*linux or TDS*/
	Int32	RunningStatus;  /*Idle, Initializing, stopping, running*/
	Int32	rpcversion;	   /*RPC version*/
        Int32   mcapiversion;      /*Mcapi version*/
	Int32	totalrunningInvoker;    /*Total num of Running invoker*/
	Int32	totalsleepInvoker;      /*Total num of sleeping invoker*/
	Int32	totalserviceQtask;      /*Total task num of Service Q*/
	Int32	totalRegisteredService; /*Total num of local registered Service*/
	Uint32  totalsendmsgcall;
	Uint32  totalsendpktcall;
	Uint32  totalrcvmsgcall;
	Uint32  totalrcvpktcall;
	Uint32  totalrcvmsgret;
	Uint32  totalrcvpktret;
	Uint32  totalrcvbadmsgpacket;
	Uint32  totalrcvbadpktpacket;
	Uint32  totalrcvbadfuncidcall;	
	//maybe other info if needed ...	
}RPCDBGINFO;

Int32 RpcGetDetailInfo(RPCDBGINFO *pinfo, Bool ifremote);

#endif
