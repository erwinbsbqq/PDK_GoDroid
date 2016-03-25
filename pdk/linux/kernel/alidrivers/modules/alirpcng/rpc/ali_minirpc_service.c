/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *
 *  File: ali_rpc_service.c
 *
 *  Description: XDR new type implementation functions.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_xdr.h>
#include <ali_minirpc_service.h>
#include <ali_rpc_service.h>

Bool XDR_Hld_device_rpc(XDR *xdrs, Hld_device_rpc *cp, Uint32 cnt);

/********* CE *************/
Bool XDR_Ce_data_info_rpc(XDR *xdrs, Ce_data_info_rpc *cp, Uint32 cnt);
Bool XDR_Otp_param_rpc(XDR *xdrs, Otp_param_rpc *cp, Uint32 cnt);
Bool XDR_Data_param_rpc(XDR *xdrs, Data_param_rpc *cp, Uint32 cnt);
Bool XDR_Des_param_rpc(XDR *xdrs, Des_param_rpc *cp, Uint32 cnt);
Bool XDR_Ce_key_param_rpc(XDR *xdrs, Ce_key_param_rpc *cp, Uint32 cnt);
Bool XDR_Ce_debug_key_info_rpc(XDR *xdrs, Ce_debug_key_info_rpc *cp, Uint32 cnt);
Bool XDR_Ce_pos_status_param_rpc(XDR *xdrs, Ce_pos_status_param_rpc *cp, Uint32 cnt);
Bool XDR_Ce_found_free_pos_param_rpc(XDR *xdrs, Ce_found_free_pos_param_rpc *cp, Uint32 cnt);
Bool XDR_Ce_pvr_key_param_rpc(XDR *xdrs, Ce_pvr_key_param_rpc *cp, Uint32 cnt);

/********* dsc ************/
Bool XDR_DeEncrypt_config_rpc(XDR *xdrs, DeEncrypt_config_rpc *cp, Uint32 cnt);
Bool XDR_Sha_init_param_rpc(XDR *xdrs, Sha_init_param_rpc *cp, Uint32 cnt);
Bool XDR_Aes_init_param_rpc(XDR *xdrs, Aes_init_param_rpc *cp, Uint32 cnt);
Bool XDR_Des_init_param_rpc(XDR *xdrs, Des_init_param_rpc *cp, Uint32 cnt);
Bool XDR_Pid_param_rpc(XDR *xdrs, Pid_param_rpc *cp, Uint32 cnt);
Bool XDR_Csa_init_param_rpc(XDR *xdrs, Csa_init_param_rpc *cp, Uint32 cnt);
Bool XDR_Dsc_pvr_key_param_rpc(XDR *xdrs, Dsc_pvr_key_param_rpc *cp, Uint32 cnt);
Bool XDR_Key_param_rpc(XDR *xdrs, Key_param_rpc *cp, Uint32 cnt);
Bool XDR_Trng_data_rpc(XDR *xdrs, Trng_data_rpc *cp, Uint32 cnt);
Bool XDR_Sha_hash_rpc(XDR *xdrs, Sha_hash_rpc *cp, Uint32 cnt);


/*A sample new struct XDR process function*/
Bool XDR_MiniRPC_TestStruct(XDR *xdrs, Minirpc_TestStruct *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->ii) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->uii) == False)
    {
        return False;
    }
    if (XDR_Char(xdrs, &cp->cc) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ucc) == False)
    {
        return False;
    }
    if (XDR_Long(xdrs, &cp->ll) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->ull) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bb) == False)
    {
        return False;
    }
    return True;
}


Bool XDR_Minirpc_Test(XDR *xdrs, Param_Minirpc_Test *cp, Uint32 cnt)
{
    int i=0;

	for(i=0;i<sizeof(cp->array)/sizeof(cp->array[0]);i++)
    {
	    if (XDR_Uint32(xdrs, &cp->array[i]) == False)
	    {
	        return False;
	    }
	}

	if (XDR_Uint32(xdrs, &cp->x0) == False)
    {
        return False;
    }
	
	if (XDR_Uint32(xdrs, &cp->x1) == False)
	{
		return False;
	}
	if (XDR_Uint32(xdrs, &cp->x2) == False)
	{
		return False;
	}
	if (XDR_Uint32(xdrs, &cp->x3) == False)
	{
		return False;
	}
	
    return True;
}




XdrOpTable_minirpc gXdrOpTable_minirpc[] =
{
    {PARAM_MINIRPC_VOID, XDR_Void},
    {PARAM_MINIRPC_INT32, XDR_Int32},
    {PARAM_MINIRPC_UINT32, XDR_Uint32},
    {PARAM_MINIRPC_LONG, XDR_Long},
    {PARAM_MINIRPC_ULONG, XDR_Ulong},
    {PARAM_MINIRPC_INT16, XDR_Int16},
    {PARAM_MINIRPC_UINT16, XDR_UInt16},
    {PARAM_MINIRPC_BOOL, XDR_Bool},
    {PARAM_MINIRPC_ENUM, XDR_Enum},
    {PARAM_MINIRPC_ARRAY, XDR_Array},
    {PARAM_MINIRPC_BYTES, XDR_Bytes},
    {PARAM_MINIRPC_OPAQUE, XDR_Opaque},
    {PARAM_MINIRPC_STRING, XDR_String},
    {PARAM_MINIRPC_UNION, XDR_Union},
    {PARAM_MINIRPC_CHAR, XDR_Char},
    {PARAM_MINIRPC_UCHAR, XDR_Uchar},
    {PARAM_MINIRPC_VECTOR, XDR_Vector},
    {PARAM_MINIRPC_FLOAT, XDR_Float},
    {PARAM_MINIRPC_DOUBLE, XDR_Double},
    {PARAM_MINIRPC_REFERENCE, XDR_Reference},
    {PARAM_MINIRPC_POINTER, XDR_Pointer},
    {PARAM_MINIRPC_WRAPSTRING, XDR_Wrapstring},
    {PARAM_MINIRPC_STRARRAY, XDR_Strarray},

    /********* CE *****************************/
    {PARAM_MINIRPC_Ce_data_info_rpc, XDR_Ce_data_info_rpc},
    {PARAM_MINIRPC_Otp_param_rpc, XDR_Otp_param_rpc},
    {PARAM_MINIRPC_Data_param_rpc, XDR_Data_param_rpc},
    {PARAM_MINIRPC_Des_param_rpc, XDR_Des_param_rpc},
    {PARAM_MINIRPC_Ce_key_param_rpc, XDR_Ce_key_param_rpc},
    {PARAM_MINIRPC_Ce_debug_key_info_rpc, XDR_Ce_debug_key_info_rpc},
    {PARAM_MINIRPC_Ce_pos_status_param_rpc, XDR_Ce_pos_status_param_rpc},
    {PARAM_MINIRPC_Ce_found_free_pos_param_rpc, XDR_Ce_found_free_pos_param_rpc},
    {PARAM_MINIRPC_Ce_pvr_key_param_rpc, XDR_Ce_pvr_key_param_rpc},

    /********* dsc ****************************/
    {PARAM_MINIRPC_DeEncrypt_config_rpc, XDR_DeEncrypt_config_rpc},
    {PARAM_MINIRPC_Sha_init_param_rpc, XDR_Sha_init_param_rpc},
    {PARAM_MINIRPC_Aes_init_param_rpc, XDR_Aes_init_param_rpc},
    {PARAM_MINIRPC_Des_init_param_rpc, XDR_Des_init_param_rpc},
    {PARAM_MINIRPC_Pid_param_rpc, XDR_Pid_param_rpc},
    {PARAM_MINIRPC_Csa_init_param_rpc, XDR_Csa_init_param_rpc},
    {PARAM_MINIRPC_Dsc_pvr_key_param_rpc, XDR_Dsc_pvr_key_param_rpc},
    {PARAM_MINIRPC_Key_param_rpc, XDR_Key_param_rpc},
    {PARAM_MINIRPC_Sha_hash_rpc, XDR_Sha_hash_rpc},    
    {PARAM_MINIRPC_Trng_data_rpc, XDR_Trng_data_rpc},
		
    {PARAM_MINIRPC_Hld_device_rpc, XDR_Hld_device_rpc},
		
	/*User specific data type proc table: START*/
	{PARAM_MINIRPC_TESTSTRUCT, XDR_MiniRPC_TestStruct}, /*Sample for new struct*/
	
	/*minirpc struct start*/
    {PARAM_MINIRPC_TEST, XDR_Minirpc_Test},

    /*User specific data type proc table: END*/
    {PARAM_MINIRPC_ID_MAX, NULL}
};
