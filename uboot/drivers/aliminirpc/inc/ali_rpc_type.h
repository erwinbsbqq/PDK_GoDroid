#ifndef __ALI_RPC_TYPE_H
#define __ALI_RPC_TYPE_H

/*Here we reuse the mcapi header defination codes*/
#include "pr_types.h"

#define NULL 0

#define  RPCST_IDLE         0
#define  RPCST_RUNNING      1
#define  RPCST_INITIALING   2
#define  RPCST_STOPPING     3

#define  RPCST_MINIRPC_IDLE         10UL
#define  RPCST_MINIRPC_INITIALING   11UL
#define  RPCST_MINIRPC_RUNNING      12UL
#define  RPCST_MINIRPC_STOPPING     13UL





typedef struct _RpcSymbol RpcSymbol;
struct _RpcSymbol {
	char *name;
	Uint32 hash;
	unsigned long func;
#if defined(__ALI_LINUX__)   /*for x86 64bit CPU debug usage*/
	Uint32 allign[2]; /*just for 64bit cpu alligned usage*/
#endif
};

typedef void  Void;

#if 0
typedef void  Void;
typedef Int32 Enum;
typedef char  *Opaque;
typedef char *Bytes;
typedef char  Char;
typedef Uint8	 Uchar;
typedef char  *Array;
typedef char  *String;
typedef char  *Wrapstring;
typedef float  Float;
typedef double Double;
typedef char *Reference;
typedef char *Pointer;
typedef char **Strarray;
#endif

typedef struct _Param Param;
struct _Param {
        Long                            type;
	Long			paramId;
        Size_t                  len;
        void*                   pData;
};

/*typedef Int32 RpcFunc(Param *, Param *, Param *, Param *, Param *, Param *, Param *, Param *);
typedef Int32 (*PRpcFunc)( Param *, Param *, Param *, Param *, Param *, Param *, Param *, Param *);
*/
typedef Int32 RpcFunc();
typedef Int32 (*PRpcFunc)();

typedef enum _ParamType ParamType;
enum _ParamType {
        PARAM_IN            = 1,
        PARAM_INOUT,
        PARAM_OUT,
        PARAM_RET,
        PARAM_CONTINUE,
        PARAM_BUFFER,
        PARAM_INVALID
};



#endif

