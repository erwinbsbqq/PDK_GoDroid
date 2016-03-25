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


Bool XDR_Hld_device_rpc(XDR *xdrs, Hld_device_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    if (XDR_Uint32(xdrs, &cp->HLD_DEV) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->type) == False)
    {
        return False;
    }
    for (i = 0; i < 16; i++)
    {
        if (XDR_Uchar(xdrs, &cp->name[i]) == False)
        {
            return False;
        }
    }
    return True;
}

/************************** ce *******************************/
Bool XDR_Ce_data_info_rpc(XDR *xdrs, Ce_data_info_rpc *cp, Uint32 cnt)
{
    Uint32 i = 0;
    if (XDR_Char(xdrs, &cp->otp_info.otp_addr) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->otp_info.otp_key_pos) == False)
    {
        return False;
    }

    for (i = 0; i < 4; i++)
    {
        if (XDR_Uint32(xdrs, &cp->data_info.crypt_data[i]) == False)
        {
            return False;
        }
    }
    if (XDR_Uint32(xdrs, &cp->data_info.data_len) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->des_aes_info.crypt_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->des_aes_info.aes_or_des) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->des_aes_info.des_low_or_high) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->key_info.first_key_pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_info.second_key_pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_info.hdcp_mode) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Otp_param_rpc(XDR *xdrs, Otp_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Char(xdrs, &cp->otp_addr) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->otp_key_pos) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Data_param_rpc(XDR *xdrs, Data_param_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    for (i = 0; i < 4; i++)
    {
        if (XDR_Uint32(xdrs, &cp->crypt_data[i]) == False)
        {
            return False;
        }
    }
    if (XDR_Uint32(xdrs, &cp->data_len) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Des_param_rpc(XDR *xdrs, Des_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->crypt_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->aes_or_des) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->des_low_or_high) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_key_param_rpc(XDR *xdrs, Ce_key_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->first_key_pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->second_key_pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->hdcp_mode) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_debug_key_info_rpc(XDR *xdrs, Ce_debug_key_info_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;

    if (XDR_Enum(xdrs, &cp->sel) == False)
    {
        return False;
    }

    for (i = 0; i < 4; i++)
    {
        if (XDR_Uint32(xdrs, &cp->buffer[i]) == False)
        {
            return False;
        }
    }
    if (XDR_Uint32(xdrs, &cp->len) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_pos_status_param_rpc(XDR *xdrs, Ce_pos_status_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->pos) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->status) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_found_free_pos_param_rpc(XDR *xdrs, Ce_found_free_pos_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->ce_key_level_r) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->number) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->root) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_pvr_key_param_rpc(XDR *xdrs, Ce_pvr_key_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->input_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->second_pos) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->first_pos) == False)
    {
        return False;
    }

    return True;
}

/**************** dsc *************************/
Bool XDR_DeEncrypt_config_rpc(XDR *xdrs, DeEncrypt_config_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->do_encrypt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->dec_dev) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->Decrypt_Mode) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dec_dmx_id) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->do_decrypt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->enc_dev) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->Encrypt_Mode) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->enc_dmx_id) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Sha_init_param_rpc(XDR *xdrs, Sha_init_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->sha_work_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->sha_data_source) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sha_buf) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Aes_init_param_rpc(XDR *xdrs, Aes_init_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->parity_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_from) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->scramble_control) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->stream_id) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->dma_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->residue_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->work_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cbc_cts_enable) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Des_init_param_rpc(XDR *xdrs, Des_init_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->parity_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_from) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->scramble_control) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->stream_id) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->dma_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->residue_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->work_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cbc_cts_enable) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Pid_param_rpc(XDR *xdrs, Pid_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->dmx_id) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pid) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pos) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->key_addr) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Csa_init_param_rpc(XDR *xdrs, Csa_init_param_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    if (XDR_Enum(xdrs, &cp->version) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->dma_mode) == False)
    {
        return False;
    }
    for (i = 0; i < 4; i++)
    {
        if (XDR_Uint32(xdrs, &cp->Dcw[i]) == False)
        {
            return False;
        }
    }
    if (XDR_Uint32(xdrs, &cp->pes_en) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->parity_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_from) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->scramble_control) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->stream_id) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Dsc_pvr_key_param_rpc(XDR *xdrs, Dsc_pvr_key_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->input_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->valid_key_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->current_key_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pvr_key_length) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pvr_user_key_pos) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->total_quantum_number) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->current_quantum_number) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ts_packet_number) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pvr_key_change_enable) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->stream_id) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Key_param_rpc(XDR *xdrs, Key_param_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    Uchar j = 0;
    if (XDR_Uint32(xdrs, &cp->handle) == False)
    {
        return False;
    }
    
    if (XDR_Uint32(xdrs, &cp->pid_list) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->pid_len) == False)
    {
        return False;
    }
    
    if (XDR_Uint32(xdrs, &cp->p_aes_key_info) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->p_csa_key_info) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->p_des_key_info) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->key_length) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->p_aes_iv_info) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->p_des_iv_info) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->stream_id) == False)
    {
        return False;
    }
    
    if (XDR_Uint32(xdrs, &cp->init_vector) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ctr_counter) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->force_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pos) == False)
    {
        return False;
    }
    
    if (XDR_Uchar(xdrs, &cp->no_even) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->no_odd) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->not_refresh_iv) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Trng_data_rpc(XDR *xdrs, Trng_data_rpc *cp, Uint32 cnt)
{
    Uint32 i=0;
    
    for (i = 0; i < sizeof(Trng_data_rpc); i++)
    {
        if (XDR_Uchar(xdrs, &cp->data[i]) == False)
        {
            return False;
        }
    }
    
    return True;
}

Bool XDR_Sha_hash_rpc(XDR *xdrs, Sha_hash_rpc *cp, Uint32 cnt)
{
    Uint32 i=0;
    
    for (i = 0; i < sizeof(Sha_hash_rpc); i++)
    {
        if (XDR_Uchar(xdrs, &cp->hash[i]) == False)
        {
            return False;
        }
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
		
    /********* hld ****************************/
    {PARAM_MINIRPC_Hld_device_rpc, XDR_Hld_device_rpc},
	/*User specific data type proc table: START*/
	{PARAM_MINIRPC_TESTSTRUCT, XDR_MiniRPC_TestStruct}, /*Sample for new struct*/
	
	/*minirpc struct start*/
    {PARAM_MINIRPC_TEST, XDR_Minirpc_Test},

    /*User specific data type proc table: END*/
    {PARAM_MINIRPC_ID_MAX, NULL}
};
