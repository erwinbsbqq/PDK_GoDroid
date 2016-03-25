#ifndef __ALI_MINIRPC_SERVICE_H
#define __ALI_MINIRPC_SERVICE_H

#include "ali_rpc_type.h"
#include "ali_rpc_cfg.h"
#include "ali_rpc_osdep.h"
#include "ali_rpc_debug.h"

typedef struct _Minirpc_TestStruct{
        Int32  ii;
        Uint32 uii;
        signed char   cc;
        Uchar  ucc;
        Long   ll;
        Ulong  ull;
        Bool   bb;
}Minirpc_TestStruct;

typedef struct _Param_Minirpc_Test
{
    Uint32 array[400];
    Uint32 x0;
	Uint32 x1;
    Uint32 x2;
    Uint32 x3;
}Param_Minirpc_Test;


typedef enum _ParamId_MiniRpc {
    /****common define****/
	PARAM_MINIRPC_VOID,
	PARAM_MINIRPC_INT32,
	PARAM_MINIRPC_UINT32,
	PARAM_MINIRPC_LONG,
	PARAM_MINIRPC_ULONG,
	PARAM_MINIRPC_INT16,
	PARAM_MINIRPC_UINT16,
	PARAM_MINIRPC_BOOL,
	PARAM_MINIRPC_ENUM,
	PARAM_MINIRPC_ARRAY,
	PARAM_MINIRPC_BYTES,
	PARAM_MINIRPC_OPAQUE,
	PARAM_MINIRPC_STRING,
	PARAM_MINIRPC_UNION,
	PARAM_MINIRPC_CHAR,
	PARAM_MINIRPC_UCHAR,
	PARAM_MINIRPC_VECTOR,
	PARAM_MINIRPC_FLOAT,
	PARAM_MINIRPC_DOUBLE,
	PARAM_MINIRPC_REFERENCE,
	PARAM_MINIRPC_POINTER,
	PARAM_MINIRPC_WRAPSTRING,
	PARAM_MINIRPC_STRARRAY,
	PARAM_MINIRPC_RPCDBGINFO,
	/****common define*****/

/**************** ce *******************/
    PARAM_MINIRPC_Ce_data_info_rpc,
    PARAM_MINIRPC_Otp_param_rpc,
    PARAM_MINIRPC_Data_param_rpc,
    PARAM_MINIRPC_Des_param_rpc,
    PARAM_MINIRPC_Ce_key_param_rpc,
    PARAM_MINIRPC_Ce_debug_key_info_rpc,
    PARAM_MINIRPC_Ce_pos_status_param_rpc,
    PARAM_MINIRPC_Ce_found_free_pos_param_rpc,
    PARAM_MINIRPC_Ce_pvr_key_param_rpc,
    
/**************** dsc ******************/    
    PARAM_MINIRPC_DeEncrypt_config_rpc,
    PARAM_MINIRPC_Sha_init_param_rpc,
    PARAM_MINIRPC_Aes_init_param_rpc,
    PARAM_MINIRPC_Des_init_param_rpc,
    PARAM_MINIRPC_Pid_param_rpc,
    PARAM_MINIRPC_Csa_init_param_rpc,
    PARAM_MINIRPC_Dsc_pvr_key_param_rpc,
    PARAM_MINIRPC_Key_param_rpc,
    PARAM_MINIRPC_Sha_hash_rpc,
    PARAM_MINIRPC_Trng_data_rpc,
    
    PARAM_MINIRPC_Hld_device_rpc,
    
    /*user struct define start*/
	PARAM_MINIRPC_TESTSTRUCT,
	
	PARAM_MINIRPC_TEST,
	
	PARAM_MINIRPC_ID_MAX
}ParamId_minirpc;

typedef Bool (*XdrOpFunc_minirpc)();

typedef struct _XdrOpTable_minirpc XdrOpTable_minirpc;
struct _XdrOpTable_minirpc {
	ParamId_minirpc id;
	XdrOpFunc_minirpc op;
};


/*Create Param usage*/
#define MINIRPC_PARAM_CREATE(param_name, param_type, param_id, param_len, data_ptr) \
			Param param_name; \
			param_name.type = param_type; \
			param_name.paramId = param_id;\
			param_name.len = param_len; \
			param_name.pData = data_ptr;


#define MINIRPC_PARAM_UPDATE(param_name, param_type, param_id, param_len, data_ptr) \
                        param_name.type = param_type; \
                        param_name.paramId = param_id;\
                        param_name.len = param_len; \
                        param_name.pData = data_ptr;




#endif
