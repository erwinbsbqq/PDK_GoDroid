
#include <ali/basic_types.h>
#include <ali/sys_define.h>
#include <ali/hld_dev.h>
#include <ali/dsc.h>

#include <ali/flash_cipher.h>
#include <ali_rpc.h>
#include <ali_minirpc_service.h>
#include <ali_rpcng.h>
#include <ali/osal.h>
#include <ali/string.h>

#include "ali_dsc_lld.h"

#define _HLD_DSC_REMOTE

struct ali_m36_dsc_dev g_ali_dsc_devices[1];


#define CACHE_LINE_SIZE  32
#define CACHE_L_ALIGN(x) (((x)%CACHE_LINE_SIZE)?((x)+CACHE_LINE_SIZE-((x)%CACHE_LINE_SIZE)):(x))
#define CACHE_A_ALIGN(x) ((const void *)((__u32)x & (~(__u32)(CACHE_LINE_SIZE - 1))))
#undef UC
#define UC(x) ((void *)(((__u32)(x)&0xBFFFFFFF)|0xa0000000)) //Un-cached memory addr for see

//#define DSC_REMOTE_API_DEBUG
#ifdef DSC_REMOTE_API_DEBUG
#define DSC_REMOTE_API_PRINTF libc_printf
#define DSC_REMOTE_API_DUMP(data,len) \
    do{ \
        int i, l=(len); \
        for(i=0; i<l; i++){ \
            DSC_REMOTE_API_PRINTF("0x%x,",*(data+i)); \
            if((i+1)%16==0) \
                DSC_REMOTE_API_PRINTF("\n");\
        }\
    }while(0)
#else
#define DSC_REMOTE_API_PRINTF(...)  do{}while(0)
#define DSC_REMOTE_API_DUMP(...)  do{}while(0)
#endif

#define offsetof(type, f) ((unsigned long) \
	((char *)&((type *)0)->f - (char *)(type *)0))

#define ALI_TRNG_MAX_GROUP 16
#define ALI_TRNG_64BITS_SIZE 8

int ali_dsc_umemcpy(void *dest, const void *src, __u32 n)
{
	int ret = 0;
	
	memcpy(dest, src, n);
	
	return ret;
}


static void dsc_deal_cache_before_dma(UINT32 in, UINT32 out, UINT32 data_length)
{
#ifdef DUAL_ENABLE
		cache_flush ( (void *)in, data_length );
		cache_flush ( (void *)out, data_length );
#endif
}

static void dsc_deal_cache_after_dma(UINT32 out, UINT32 data_length)
{
#ifdef DUAL_ENABLE
		cache_invalidate ( (void *)out, data_length );
#endif
}


#if 1 //#ifdef DUAL_ENABLE
//#include <ali/modules.h>

#ifndef _HLD_DSC_REMOTE
extern RET_CODE des_decrypt_rpc( DES_DEV *p_des_dev, UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length);
extern RET_CODE des_encrypt_rpc( DES_DEV *p_des_dev, UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length);
extern RET_CODE aes_encrypt_rpc( AES_DEV *p_aes_dev, UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length);
extern RET_CODE aes_decrypt_rpc( AES_DEV *p_aes_dev, UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length);
extern RET_CODE csa_decrypt_rpc( CSA_DEV *p_aes_dev, UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length);

UINT32 hld_dsc_entry[] = 
{
	(UINT32)trng_generate_byte, 
	(UINT32)trng_generate_64bits, 
	(UINT32)trng_get_64bits,	
	
	(UINT32)des_decrypt_rpc,
	(UINT32)des_encrypt_rpc,
	(UINT32)des_ioctl,

	(UINT32)aes_decrypt_rpc,
	(UINT32)aes_encrypt_rpc,
	(UINT32)aes_ioctl,

	(UINT32)csa_decrypt_rpc,
	(UINT32)csa_ioctl,

	(UINT32)sha_digest,
	(UINT32)sha_ioctl,

	(UINT32)dsc_api_attach, 
	(UINT32)dsc_api_detach,
	(UINT32)dsc_ioctl,

	(UINT32)trig_ram_mon,
	(UINT32)de_encrypt,
	(UINT32)dsc_get_free_stream_id,
	(UINT32)dsc_set_stream_id_idle, 
	(UINT32)dsc_get_free_sub_device_id,
	(UINT32)dsc_set_sub_device_id_idle,
	(UINT32)dsc_set_stream_id_used,
	(UINT32)dsc_set_sub_device_id_used,
	(UINT32)dsc_deal_quantum_for_mixed_ts,
};

void hld_dsc_callee(UINT8 *msg)
{
	OS_hld_callee((UINT32)hld_dsc_entry, msg);
}

UINT32 dsc_m36_entry[] =
{
};
/* just define the function handle, do not use this */
void lld_dsc_m36f_callee( UINT8 *msg )
{
	OS_hld_callee((UINT32)dsc_m36_entry, msg);
}
void lld_trng_m36f_callee( UINT8 *msg )
{
	OS_hld_callee((UINT32)dsc_m36_entry, msg);
}

/* High level API on SEE side for SHA1/SHA224/SHA256/SHA384/SHA512 digest */
RET_CODE see_ali_sha_digest(UINT8 *input, UINT32 input_len, \
								enum SHA_MODE sha_mode, UINT8 *output)
{
	#define SHA_MODE_CHECK(mode)	(((mode) != SHA_SHA_1) && \
		((mode) != SHA_SHA_224) && ((mode) != SHA_SHA_256) && \
		((mode) != SHA_SHA_384 )&& ((mode) != SHA_SHA_512))

	UINT32 ShaDevID = ALI_INVALID_DSC_SUB_DEV_ID;
	pSHA_DEV pShaDev = NULL;
	SHA_INIT_PARAM param;
	RET_CODE ret = RET_FAILURE;

	if ( !input || ! output || ! input_len || SHA_MODE_CHECK ( sha_mode ) )
	{
		DSC_REMOTE_API_PRINTF( "%s(): Wrong para!\n", __FUNCTION__);
		return RET_FAILURE;
	}

	if(ALI_INVALID_DSC_SUB_DEV_ID == (ShaDevID = dsc_get_free_sub_device_id(SHA)))
	{
		DSC_REMOTE_API_PRINTF("%s() get free SHA device failed!\n", __FUNCTION__);
		return RET_FAILURE;
	}

	if (NULL == (pShaDev = (pSHA_DEV)dev_get_by_id(HLD_DEV_TYPE_SHA, ShaDevID)))
	{
		DSC_REMOTE_API_PRINTF("%s() get pSHA_DEV %d failed!\n", \
								__FUNCTION__, ShaDevID);
		dsc_set_sub_device_id_idle(SHA, ShaDevID);
		return RET_FAILURE;
	}

	memset(&param, 0, sizeof(SHA_INIT_PARAM));
	param.sha_data_source = SHA_DATA_SOURCE_FROM_DRAM;
	param.sha_work_mode = sha_mode;
	sha_ioctl(pShaDev, IO_INIT_CMD, (UINT32)&param);
	ret = sha_digest(pShaDev, input, output, input_len);

	dsc_set_sub_device_id_idle(SHA, ShaDevID);
	return ret;
}

/*
*High level API on SEE side for software version check
*/
RET_CODE see_version_check(UINT32 block_id,UINT32 block_addr,UINT32 block_len)
{    
	RET_CODE ret = RET_FAILURE;
	pDSC_DEV pDscDev = ( pDSC_DEV ) dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
	if(pDscDev!= NULL)
	{
		 DSC_VER_CHK_PARAM ver_param;
		 memset(&ver_param,0,sizeof(DSC_VER_CHK_PARAM));
		 ver_param.chk_id = block_id ;
		 ver_param.input_mem = block_addr|0xa0000000;
		 ver_param.len = block_len ;
		 ver_param.chk_mode =   CHECK_END  ; 
		 
		 ret=dsc_ioctl(pDscDev,IO_DSC_VER_CHECK,(UINT32)&ver_param);
		 return ret ;         
	}
	return RET_SUCCESS;
}

#endif  /*_HLD_DSC_REMOTE*/

#ifdef _HLD_DSC_REMOTE
#define DSC_NPARA(x) ((HLD_DSC_MODULE<<24)|(x<<16))

UINT32 des_io_control[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, 0),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 aes_io_control[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, 0),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 csa_io_control[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, 0),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 sha_io_control[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, 0),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 dsc_io_control[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, 0),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

UINT32 dsc_io_control_1[] = 
{ //desc of pointer para
  9, DESC_OUTPUT_STRU(0, sizeof(KEY_PARAM)), DESC_STATIC_STRU(1, 0),\
  	 DESC_STATIC_STRU(2, 4*sizeof(AES_KEY_PARAM)), DESC_STATIC_STRU(3, 4*sizeof(CSA_KEY_PARAM)),\
  	 DESC_STATIC_STRU(4, 4*sizeof(DES_KEY_PARAM)), DESC_STATIC_STRU(5, 4*sizeof(AES_IV_INFO)),\
  	 DESC_STATIC_STRU(6, 4*sizeof(DES_IV_INFO)), DESC_STATIC_STRU(7, 16), DESC_STATIC_STRU(8, 16),
  9, DESC_P_PARA(0, 2, 0), DESC_P_STRU(1, 0, 1, offsetof(KEY_PARAM, pid_list)),\
     DESC_P_STRU(1, 0, 2, offsetof(KEY_PARAM, p_aes_key_info)), DESC_P_STRU(1, 0, 3, offsetof(KEY_PARAM, p_csa_key_info)),\
     DESC_P_STRU(1, 0, 4, offsetof(KEY_PARAM, p_des_key_info)), DESC_P_STRU(1, 0, 5, offsetof(KEY_PARAM, p_aes_iv_info)),\
     DESC_P_STRU(1, 0, 6, offsetof(KEY_PARAM, p_des_iv_info)), DESC_P_STRU(1, 0, 7, offsetof(KEY_PARAM, init_vector)),\
     DESC_P_STRU(1, 0, 8, offsetof(KEY_PARAM, ctr_counter)),
  //desc of pointer ret
  0,                          
  0,
};

UINT32 sha_crypt_control[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 64),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};

RET_CODE dsc_api_attach_ex( void )
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, NULL, DSC_NPARA(0) | FUNC_DSC_ATTACH, NULL);
#else 
	RpcCallCompletion(RPC_dsc_api_attach,NULL);
#endif
}

RET_CODE dsc_api_detach_ex( void )
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, NULL, DSC_NPARA(0) | FUNC_DSC_DETACH, NULL);
#else 
	RpcCallCompletion(RPC_dsc_api_detach,NULL);
#endif
}

RET_CODE  dsc_attach_dev_register(void)
{
	hld_dev_get_remote("DSC_M3602_0");
	
	hld_dev_get_remote("SHA_M3602_1");
	hld_dev_get_remote("SHA_M3602_2");
	hld_dev_get_remote("SHA_M3602_3");
	hld_dev_get_remote("SHA_M3602_4");

	hld_dev_get_remote("AES_M3602_1");
	hld_dev_get_remote("AES_M3602_2");
	hld_dev_get_remote("AES_M3602_3");
	hld_dev_get_remote("AES_M3602_4");
	
	hld_dev_get_remote("DES_M3602_1");
	hld_dev_get_remote("DES_M3602_2");
	hld_dev_get_remote("DES_M3602_3");
	hld_dev_get_remote("DES_M3602_4");
	
	hld_dev_get_remote("CSA_M3602_1");
	hld_dev_get_remote("CSA_M3602_2");
	hld_dev_get_remote("CSA_M3602_3");
	hld_dev_get_remote("CSA_M3602_4");
	return ;
}
	
RET_CODE dsc_api_attach( void )
{
	RET_CODE ret = RET_SUCCESS;
	pDSC_DEV pDscDev = ( pDSC_DEV ) dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
	if(!pDscDev)
	{
		ret = dsc_api_attach_ex();
	//	run_time_integrity_check();
		dsc_attach_dev_register();
		return ret;
	}
	return ALI_DSC_WARNING_DRIVER_ALREADY_INITIALIZED;
}

RET_CODE dsc_api_detach( void )
{
	return dsc_api_detach_ex();
}

RET_CODE trng_generate_byte( UINT8 *data )
{
    UINT32 desc[] =
    {
        1, DESC_OUTPUT_STRU(0, 1), 1, DESC_P_PARA(0, 0, 0), 0, 0, 
    };
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, data, DSC_NPARA(1) | FUNC_TRNG_GENERATE_BYTE, desc);
#else 
	MINIRPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_MINIRPC_UCHAR, sizeof(__u8), (void *)data);
	return RpcCallCompletion(RPC_trng_generate_byte,&p1,NULL);
#endif
}

RET_CODE trng_generate_64bits( UINT8 *data )
{
    UINT32 desc[] =
    {
        1, DESC_OUTPUT_STRU(0, 8), 1, DESC_P_PARA(0, 0, 0), 0, 0, 
    };
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, data, DSC_NPARA(1) | FUNC_TRNG_GENERATE_64BITS, desc);
#else 
	int ret = -1;
	Trng_data_rpc trng_data;
	Param p1;
	
	memset((void *)&trng_data, 0x00, sizeof(trng_data));
	
	MINIRPC_PARAM_UPDATE(p1, PARAM_OUT, PARAM_MINIRPC_Trng_data_rpc, ALI_TRNG_64BITS_SIZE, (void *)&trng_data);
    
	ret = RpcCallCompletion(RPC_trng_generate_64bits,&p1,NULL);
	memcpy(data, trng_data.data, ALI_TRNG_64BITS_SIZE);
	return ret;
#endif
}

static RET_CODE trng_get_64bits_ex(UINT8 *data, UINT32 n)
{
    UINT32 desc[] =
    {
        1, DESC_OUTPUT_STRU(0, 0), 1, DESC_P_PARA(0, 0, 0), 0, 0,
    };
    DESC_OUTPUT_STRU_SET_SIZE(desc, 0, (ALI_TRNG_64BITS_SIZE * n));
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, data, DSC_NPARA(2) | FUNC_TRNG_SEE_GET64BIT, desc);
#else 
	int ret = -1;
	Trng_data_rpc trng_data;
	Param p1;
	Param p2;

	memset((void *)&trng_data, 0x00, sizeof(trng_data));
	
	MINIRPC_PARAM_UPDATE(p1, PARAM_OUT, PARAM_MINIRPC_Trng_data_rpc, ALI_TRNG_64BITS_SIZE*n, (void *)&trng_data);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&n);

	ret = RpcCallCompletion(RPC_trng_get_64bits,&p1,&p2,NULL);

	memcpy(data, trng_data.data, ALI_TRNG_64BITS_SIZE*n);
	return ret;
#endif
}

RET_CODE trng_get_64bits(UINT8 *data, UINT32 n)
{
	RET_CODE ret = RET_FAILURE;
	UINT32 trng_c = 0;
	UINT32 deal_c = 0;
	
	while(deal_c < n)
	{
		trng_c = (n-deal_c)>ALI_TRNG_MAX_GROUP?ALI_TRNG_MAX_GROUP:(n-deal_c);
		ret = trng_get_64bits_ex(data+ALI_TRNG_64BITS_SIZE*deal_c,(UINT32)trng_c);
		if (RET_SUCCESS != ret)
		{
			return ret;
		}  
		deal_c += trng_c;
	}
	
	return ret;
}

RET_CODE des_decrypt_ex(pDES_DEV pDesDev,UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, pDesDev, DSC_NPARA(5)|FUNC_DES_DECRYPT, NULL);
#else 
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pDesDev);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT16, sizeof(__u16), (void *)&stream_id);   
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&input);
	MINIRPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&output);
	MINIRPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&total_length);   

	return RpcCallCompletion(RPC_des_decrypt,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

RET_CODE des_encrypt_ex(pDES_DEV pDesDev,UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, pDesDev, DSC_NPARA(5)|FUNC_DES_ENCRYPT, NULL);
#else 
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pDesDev);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT16, sizeof(__u16), (void *)&stream_id);   
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&input);
	MINIRPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&output);
	MINIRPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(total_length), (void *)&total_length);   

	return RpcCallCompletion(RPC_des_encrypt,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

RET_CODE des_decrypt(pDES_DEV pDesDev,UINT16 stream_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
	RET_CODE ret=RET_FAILURE;
	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		if(total_length>TS_MAX_SIZE)
		{
			return ret;
		}
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length * 188);
	}
	else
	{
		if(total_length>PURE_DATA_MAX_SIZE)
		{
			return ret;
		}      
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length);
	}
	
	ret=des_decrypt_ex(pDesDev,stream_id,input,output,total_length);
	
	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length * 188);
	}
	else
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length);
	}   

	return ret;
}

RET_CODE des_encrypt(pDES_DEV pDesDev, UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length)
{
	RET_CODE ret = RET_FAILURE;
	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		if(total_length>TS_MAX_SIZE)
		{
			return ret;
		}        
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length * 188);
	}
	else
	{
		if(total_length>PURE_DATA_MAX_SIZE)
		{
			return ret;
		}      
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length);
	}

	ret=des_encrypt_ex(pDesDev,stream_id,input,output,total_length);
	
	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length * 188);
	}
	else
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length);
	}
	return ret;
}


RET_CODE des_ioctl( pDES_DEV pDesDev ,UINT32 cmd , UINT32 param)
{
#if !defined(CONFIG_ALI_RPCNG)
	UINT32 i;
	UINT32 common_desc[40];
	UINT32 *desc = (UINT32 *)common_desc;
	UINT32 *b = (UINT32 *)des_io_control;
	pKEY_PARAM param_tmp = (pKEY_PARAM)param;

	for(i = 0; i < sizeof(des_io_control)/sizeof(UINT32); i++)
	{
		desc[i] = b[i];
	}
	switch(cmd) 
	{
		case IO_INIT_CMD:
		{
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DES_INIT_PARAM));
		}
		break ;
		
		case IO_CREAT_CRYPT_STREAM_CMD :
		case IO_KEY_INFO_UPDATE_CMD:
		{
			/*has return value*/
			b = (UINT32 *)dsc_io_control_1;
			for(i = 0; i < sizeof(dsc_io_control_1)/sizeof(UINT32); i++)
			{
				desc[i] = b[i];
			}

			if(0 == param_tmp->pid_len)
			{
				param_tmp->pid_len = 1; //at least transfer - one set of key
			}
				
			for(i=0; i<3; i++)
			{
				DESC_STATIC_STRU_SET_SIZE(desc, (i+2), 2*param_tmp->pid_len*param_tmp->key_length/8);
			}
			DESC_STATIC_STRU_SET_SIZE(desc, 6, 2*param_tmp->pid_len*8);
			DESC_STATIC_STRU_SET_SIZE(desc, 1, sizeof(UINT16)*param_tmp->pid_len);
		}
		break;

		case IO_DELETE_CRYPT_STREAM_CMD:
		{
			desc = NULL;
		}
		break;

		default:
			if(sys_ic_get_chip_id() >= ALI_S3602F)
			return RET_FAILURE;
	}

	jump_to_func(NULL, ali_rpc_call, pDesDev, DSC_NPARA(3)|FUNC_DES_IOCTL, desc);
#else
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	KEY_PARAM key_param;
	DSC_CLR_KEY *dsc_key = (DSC_CLR_KEY *)dsc->dsc_key;
	int hdl_ret = 0;
	int ret = -1;
	Param p1;
	Param p2;
	Param p3;
	
	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pDesDev);                     
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&cmd);         
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&param);

	memset(&key_param, 0x00, sizeof(KEY_PARAM));
	
	switch(cmd) 
	{
		case IO_INIT_CMD:
		{
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Des_init_param_rpc, sizeof(Des_init_param_rpc), (void *)param);
			break;
		}
		
		case IO_CREAT_CRYPT_STREAM_CMD:
		case IO_KEY_INFO_UPDATE_CMD:
		{
			ali_dsc_umemcpy(&key_param,(pKEY_PARAM)param,sizeof(KEY_PARAM));
			if(!key_param.pid_len || key_param.pid_len > ALI_DSC_LLD_MAX_ITEM)
			{
				key_param.pid_len = 1;
			}
			
			if(key_param.pid_list)
			{
				ali_dsc_umemcpy((dsc_key->pid_ptr),key_param.pid_list,sizeof(__u16)*(key_param.pid_len));
				key_param.pid_list = UC(dsc_key->pid_ptr);
			}
			
			if(key_param.p_des_key_info)
			{
				ali_dsc_umemcpy((dsc_key->key_ptr),key_param.p_des_key_info, \
								2*(key_param.pid_len)*key_param.key_length/8);
				key_param.p_des_key_info = UC(dsc_key->key_ptr);				
			}
			
			if(key_param.p_des_iv_info)
			{
				ali_dsc_umemcpy((dsc_key->iv_ptr),key_param.p_des_iv_info,2*(key_param.pid_len)*DES_BLOCK_LEN);
				key_param.p_des_iv_info = UC(dsc_key->iv_ptr);
			}
			
			if(key_param.init_vector)
			{
				ali_dsc_umemcpy((dsc_key->iv_ptr),key_param.init_vector,DES_BLOCK_LEN);
				key_param.init_vector = UC(dsc_key->iv_ptr);
			}
			
			hdl_ret = 1;
			MINIRPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_MINIRPC_Key_param_rpc, sizeof(Key_param_rpc), &key_param);      
			break;
		}
		

		case IO_DELETE_CRYPT_STREAM_CMD:
			break;

		default:
		    ALI_DSC_RPC_DEBUG("Dsc rpc error: %d, invalid cmd=%d\n", __LINE__, cmd);
		    break;            
	}
	
	ret = RpcCallCompletion(RPC_des_ioctl,&p1,&p2,&p3,NULL);
	
	if(hdl_ret)
	{
		ali_dsc_umemcpy((pKEY_PARAM)param,&key_param,sizeof(key_param.handle));
		ALI_DSC_RPC_DEBUG("handle = %d\n",key_param.handle);
	}
	return ret;
#endif

}


RET_CODE aes_encrypt_ex(pAES_DEV pAesDev,UINT16 stream_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
 #if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, pAesDev, DSC_NPARA(5)|FUNC_AES_ENCRYPT, NULL);
#else
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pAesDev);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT16, sizeof(__u16), (void *)&stream_id);   
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&input);
	MINIRPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&output);
	MINIRPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&total_length);
    
    return RpcCallCompletion(RPC_aes_encrypt,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

RET_CODE aes_decrypt_ex(pAES_DEV pAesDev,UINT16 stream_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
 #if !defined(CONFIG_ALI_RPCNG)
 	jump_to_func(NULL, ali_rpc_call, pAesDev, DSC_NPARA(5)|FUNC_AES_DECRYPT, NULL);
#else
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pAesDev);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT16, sizeof(__u16), (void *)&stream_id);   
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&input);
	MINIRPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&output);
	MINIRPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&total_length);   

	return RpcCallCompletion(RPC_aes_decrypt,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

RET_CODE aes_decrypt(pAES_DEV pAesDev, UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length)
{
	RET_CODE ret=RET_FAILURE;

	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		if(total_length>TS_MAX_SIZE)
		{
			return ret;
		}          
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length * 188);
	}
	else
	{
		if(total_length>PURE_DATA_MAX_SIZE)
		{
			return ret;
		}
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length);
	}
	ret = aes_decrypt_ex(pAesDev,stream_id,input,output,total_length);

	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length * 188);
	}
	else
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length);
	}

	return ret;
}

RET_CODE aes_encrypt(pAES_DEV pAesDev, UINT16 stream_id, UINT8 *input, UINT8 *output, UINT32 total_length)
{
	RET_CODE ret=RET_FAILURE;

	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		if(total_length>TS_MAX_SIZE)
		{
			return ret;
		}          
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length * 188);
	}
	else
	{
		if(total_length>PURE_DATA_MAX_SIZE)
		{
			return ret;
		}  
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length);
	}
	ret = aes_encrypt_ex(pAesDev,stream_id,input,output,total_length);

	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length * 188);
	}
	else
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length);
	}

	return ret;
}

RET_CODE aes_ioctl( pAES_DEV pAesDev ,UINT32 cmd , UINT32 param)
{
#if !defined(CONFIG_ALI_RPCNG)
	UINT32 i;
	UINT32 common_desc[40];
	UINT32 *desc = (UINT32 *)common_desc;
	UINT32 *b = (UINT32 *)aes_io_control;
	pKEY_PARAM param_tmp = (pKEY_PARAM)param;

	for(i = 0; i < sizeof(aes_io_control)/sizeof(UINT32); i++)
	{
		desc[i] = b[i];
	}
	switch(cmd) 
	{
		case IO_INIT_CMD:
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(AES_INIT_PARAM));
			break ;
		case IO_CREAT_CRYPT_STREAM_CMD :
		case IO_KEY_INFO_UPDATE_CMD:
		{
			/*has return value*/
			b = (UINT32 *)dsc_io_control_1;
			for(i = 0; i < sizeof(dsc_io_control_1)/sizeof(UINT32); i++)
			{
				desc[i] = b[i];
			}

			if(0 == param_tmp->pid_len)
			{
				param_tmp->pid_len = 1; //at least transfer - one set of key
			}

			for(i=0; i<3; i++)
			{
				DESC_STATIC_STRU_SET_SIZE(desc, (i+2), 2*param_tmp->pid_len*param_tmp->key_length/8);
			}
			DESC_STATIC_STRU_SET_SIZE(desc, 5, 2*param_tmp->pid_len*16);
			DESC_STATIC_STRU_SET_SIZE(desc, 1, sizeof(UINT16)*param_tmp->pid_len);
		}
		break;

		case IO_DELETE_CRYPT_STREAM_CMD:
			desc = NULL;
			break;

		default:
			if(sys_ic_get_chip_id() >= ALI_S3602F)
			return RET_FAILURE;
	}
	jump_to_func(NULL, ali_rpc_call, pAesDev, DSC_NPARA(3)|FUNC_AES_IOCTL, desc);
#else
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	KEY_PARAM key_param;
	DSC_CLR_KEY *dsc_key = (DSC_CLR_KEY *)dsc->dsc_key;
	int hdl_ret = 0;
	int ret = -1;
	Param p1;
	Param p2;
	Param p3;

	memset(&key_param, 0x00, sizeof(KEY_PARAM));
	
	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pAesDev);                     
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&cmd);         
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&param);
		
	switch(cmd) 
	{
		case IO_INIT_CMD:
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Aes_init_param_rpc, sizeof(Aes_init_param_rpc), (void *)param);
			break;

		case IO_CREAT_CRYPT_STREAM_CMD :
		case IO_KEY_INFO_UPDATE_CMD:
		{
			ali_dsc_umemcpy(&key_param,(pKEY_PARAM)param,sizeof(KEY_PARAM));
			if(!key_param.pid_len || key_param.pid_len > ALI_DSC_LLD_MAX_ITEM)
			{
				key_param.pid_len = 1;
			}
			
			if(key_param.pid_list)
			{
				ali_dsc_umemcpy((dsc_key->pid_ptr),key_param.pid_list,sizeof(__u16)*(key_param.pid_len));
				key_param.pid_list = UC(dsc_key->pid_ptr);
			}
			
			if(key_param.p_aes_key_info)
			{
				ali_dsc_umemcpy((dsc_key->key_ptr),key_param.p_aes_key_info, \
								2*(key_param.pid_len)*key_param.key_length/8);
				key_param.p_aes_key_info = UC(dsc_key->key_ptr);				
			}
			
			if(key_param.p_aes_iv_info)
			{
				ali_dsc_umemcpy((dsc_key->iv_ptr),key_param.p_aes_iv_info,2*(key_param.pid_len)*AES_BLOCK_LEN);
				key_param.p_aes_iv_info = UC(dsc_key->iv_ptr);
			}
			
			if(key_param.init_vector)
			{
				ali_dsc_umemcpy((dsc_key->iv_ptr),key_param.init_vector,AES_BLOCK_LEN);
				key_param.init_vector = UC(dsc_key->iv_ptr);
			}
			
			if(key_param.ctr_counter)
			{
				ali_dsc_umemcpy((dsc_key->ctr_ptr),key_param.ctr_counter,AES_BLOCK_LEN);
				key_param.ctr_counter = UC(dsc_key->ctr_ptr);
			}
			hdl_ret = 1;
			MINIRPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_MINIRPC_Key_param_rpc, sizeof(Key_param_rpc), (void *)&key_param);      
			break;
		}
		
		case IO_DELETE_CRYPT_STREAM_CMD:
			break;        

		default:
		    	ALI_DSC_RPC_DEBUG("Dsc rpc error: %d, invalid cmd=%d\n", __LINE__,cmd);
			break;
	}
	
	ret = RpcCallCompletion(RPC_aes_ioctl,&p1,&p2,&p3,NULL);
	
	if(hdl_ret)
	{
		ali_dsc_umemcpy((pKEY_PARAM)param,&key_param,sizeof(key_param.handle));
		ALI_DSC_RPC_DEBUG("handle = %d\n",key_param.handle);
	}
	return ret;
#endif

}

RET_CODE csa_decrypt_ex(pCSA_DEV pCsaDev,UINT16 stream_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
#if !defined(CONFIG_ALI_RPCNG)   
	jump_to_func(NULL, ali_rpc_call, pCsaDev,DSC_NPARA(5)|FUNC_CSA_DECRYPT, NULL);
#else
	int ret = -1;
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pCsaDev);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT16, sizeof(__u16), (void *)&stream_id);   
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&input);   
	MINIRPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&output);   
	MINIRPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&total_length);   

	ret = RpcCallCompletion(RPC_csa_decrypt,&p1,&p2,&p3,&p4,&p5,NULL);
	return ret;
#endif
}

RET_CODE csa_decrypt(pCSA_DEV pCsaDev,UINT16 stream_id,  UINT8 *input, UINT8 *output, UINT32 total_length)
{
	RET_CODE ret=RET_FAILURE;
	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		if(total_length>TS_MAX_SIZE)
		{
			return ret;
		}
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length * 188);
	}
	else
	{
		if(total_length>PURE_DATA_MAX_SIZE)
		{
			return ret;
		}
		dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length);
	}
	ret=csa_decrypt_ex(pCsaDev,stream_id,input,output,total_length);
	
	if((stream_id>=0 && stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length * 188);
	}
	else
	{
		dsc_deal_cache_after_dma((UINT32)output, total_length);
	}
	return ret;
}

RET_CODE csa_ioctl( pCSA_DEV pCsaDev, UINT32 cmd, UINT32 param)
{
#if !defined(CONFIG_ALI_RPCNG)
	UINT32 i;
	UINT32 common_desc[40];
	UINT32 *desc = (UINT32 *)common_desc;
	UINT32 *b = (UINT32 *)csa_io_control;
	pKEY_PARAM param_tmp = (pKEY_PARAM)param;

	for(i = 0; i < sizeof(csa_io_control)/sizeof(UINT32); i++)
	{
	desc[i] = b[i];
	}
	switch(cmd) 
	{
		case IO_INIT_CMD:
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CSA_INIT_PARAM ));
			break;
		case IO_CREAT_CRYPT_STREAM_CMD :
		case IO_KEY_INFO_UPDATE_CMD:
		{
			/*has return value*/
			b = (UINT32 *)dsc_io_control_1;
			for(i = 0; i < sizeof(dsc_io_control_1)/sizeof(UINT32); i++)
			{
				desc[i] = b[i];
			}

			if(0 == param_tmp->pid_len)
			{
				param_tmp->pid_len = 1; //at least transfer - one set of key
			}

			for(i=0; i<3; i++)
			{
				DESC_STATIC_STRU_SET_SIZE(desc, (i+2), 2*param_tmp->pid_len*param_tmp->key_length/8);
			}
			DESC_STATIC_STRU_SET_SIZE(desc, 1, sizeof(UINT16)*param_tmp->pid_len);
		}
		break;

		case IO_DELETE_CRYPT_STREAM_CMD:
			desc = NULL;
			break;

		default:
			if(sys_ic_get_chip_id() >= ALI_S3602F)
				return RET_FAILURE;
	}
	jump_to_func(NULL, ali_rpc_call, pCsaDev, DSC_NPARA(3)|FUNC_CSA_IOCTL, desc);
#else
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	KEY_PARAM key_param;
	DSC_CLR_KEY *dsc_key = (DSC_CLR_KEY *)dsc->dsc_key;
	int hdl_ret = 0;
	int ret = -1;
	Param p1;
	Param p2;
	Param p3;

	memset(&key_param, 0x00, sizeof(KEY_PARAM));

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pCsaDev);                     
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&cmd);         
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&param);

	switch(cmd) 
	{
		case IO_INIT_CMD:
		{
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Des_init_param_rpc, sizeof(DES_INIT_PARAM), (void *)param);
			break;
		}
		
		case IO_CREAT_CRYPT_STREAM_CMD:
		case IO_KEY_INFO_UPDATE_CMD:
		{
			ali_dsc_umemcpy(&key_param,(pKEY_PARAM)param,sizeof(KEY_PARAM));
			if(!key_param.pid_len || key_param.pid_len > ALI_DSC_LLD_MAX_ITEM)
			{
				key_param.pid_len = 1;
			}
			
			if(key_param.pid_list)
			{
				ali_dsc_umemcpy((dsc_key->pid_ptr),key_param.pid_list,sizeof(__u16)*(key_param.pid_len));
				key_param.pid_list = UC(dsc_key->pid_ptr);
			}
			
			if(key_param.p_csa_key_info)
			{
				ali_dsc_umemcpy((dsc_key->key_ptr),key_param.p_csa_key_info, \
								2*(key_param.pid_len)*key_param.key_length/8);
				key_param.p_csa_key_info = UC(dsc_key->key_ptr);				
			}
	
			hdl_ret = 1;
			MINIRPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_MINIRPC_Key_param_rpc, sizeof(Key_param_rpc), (void *)&key_param);      
			break;
		}
		

		case IO_DELETE_CRYPT_STREAM_CMD:
			break;

		default:
		    ALI_DSC_RPC_DEBUG("Dsc rpc error: %d, invalid cmd=%d\n", __LINE__, cmd);
		    break;            
	}
	
	ret = RpcCallCompletion(RPC_csa_ioctl,&p1,&p2,&p3,NULL);
	
	if(hdl_ret)
	{
		ali_dsc_umemcpy((pKEY_PARAM)param,&key_param,sizeof(key_param.handle));
		ALI_DSC_RPC_DEBUG("handle = %d\n",key_param.handle);
	}
	return ret;

#endif

}


RET_CODE sha_ioctl( pSHA_DEV pShaDev ,UINT32 cmd , UINT32 param)
{
#if !defined(CONFIG_ALI_RPCNG)
	UINT32 i;
	UINT32 common_desc[sizeof(sha_io_control)];
	UINT32 *desc = (UINT32 *)common_desc;
	UINT32 *b = (UINT32 *)sha_io_control;

	for(i = 0; i < sizeof(sha_io_control)/sizeof(UINT32); i++)
	{
		desc[i] = b[i];
	}
	switch(cmd)
	{
		case IO_INIT_CMD:
		{
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(SHA_INIT_PARAM ));
		}
		break;

		default:
		if(sys_ic_get_chip_id() >= ALI_S3602F)
			return RET_FAILURE;
	}
	jump_to_func(NULL, ali_rpc_call, pShaDev, DSC_NPARA(3)|FUNC_SHA_IOCTL, desc);
#else
	Param p1;
	Param p2;
	Param p3;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pShaDev);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&cmd);
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&param);

	switch(cmd)
	{
		case IO_INIT_CMD:
		{
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Sha_init_param_rpc, sizeof(SHA_INIT_PARAM), (void *)param);
			break;
		}
		
		default:
		{
			ALI_DSC_RPC_DEBUG("sha rpc error: invalid cmd %d\n", cmd);
			return RET_FAILURE;
		}
	}
	return RpcCallCompletion(RPC_sha_ioctl,&p1,&p2,&p3,NULL);
#endif    

}

RET_CODE sha_digest_ex(pSHA_DEV pShaDev, UINT8 *input, UINT8 *output,UINT32 data_length)
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, pShaDev, DSC_NPARA(4)|FUNC_SHA_DIGEST, sha_crypt_control);
#else
	int ret = -1;
	Sha_hash_rpc hash_out;
	Param p1;
	Param p2;
	Param p3;
	Param p4;

	memset((void *)&hash_out, 0x00, sizeof(hash_out));

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pShaDev);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&input);
	MINIRPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_MINIRPC_Sha_hash_rpc, sizeof(Sha_hash_rpc), (void *)&hash_out);
	MINIRPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&data_length);   

	ret = RpcCallCompletion(RPC_sha_digest,&p1,&p2,&p3,&p4,NULL);
	memcpy(output, hash_out.hash, sizeof(hash_out));
	return ret;
#endif

}


RET_CODE sha_digest(pSHA_DEV pShaDev, UINT8 *input, UINT8 *output,UINT32 data_length)
{
	RET_CODE ret=RET_FAILURE;
	if(data_length>PURE_DATA_MAX_SIZE)
	{
		return ret;
	}        
	dsc_deal_cache_before_dma((UINT32)input, (UINT32)input, data_length);
	ret=sha_digest_ex(pShaDev,input,output,data_length);   
	return ret;
}

UINT32 dsc_bl_ctrl[] = 
{ //desc of pointer para
	2, DESC_OUTPUT_STRU(0, sizeof(DSC_BL_UK_PARAM)), DESC_OUTPUT_STRU(1, 16),
	2, DESC_P_PARA(0, 2, 0),
	DESC_P_STRU(1, 0, 1, offsetof(DSC_BL_UK_PARAM, output_key)),
	//desc of pointer ret
	0,                          
	0,
};


RET_CODE dsc_ioctl( pDSC_DEV pDscDev ,UINT32 cmd , UINT32 param)
{
#if !defined(CONFIG_ALI_RPCNG)
	UINT32 i;
	UINT32 common_desc[40];
	UINT32 *desc = (UINT32 *)common_desc;
	UINT32 *b = (UINT32 *)dsc_io_control;

	for(i = 0; i < sizeof(dsc_io_control)/sizeof(UINT32); i++)
	{
		desc[i] = b[i];
	}

	switch(cmd)
	{
		case IO_PARSE_DMX_ID_GET_CMD:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
			break;
		case IO_PARSE_DMX_ID_SET_CMD:
			desc = NULL;
			break;
		case IO_DSC_GET_DES_HANDLE:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
			break;
		case IO_DSC_GET_AES_HANDLE:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
			break;
		case IO_DSC_GET_CSA_HANDLE:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
			break;
		case IO_DSC_GET_SHA_HANDLE:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(UINT32));
			break;
		case IO_DSC_SET_PVR_KEY_PARAM:
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_PVR_KEY_PARAM));
			break;
		case IO_DSC_VER_CHECK:                 
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_VER_CHK_PARAM));
			break;                          
		case IO_DSC_ENCRYTP_BL_UK:
			b = (UINT32 *)dsc_bl_ctrl;
			for(i = 0; i < sizeof(dsc_bl_ctrl)/sizeof(UINT32); i++)
			{
				desc[i] = b[i];
			}
			break;   
		case IO_DSC_SET_PVR_KEY_IDLE:
			desc = NULL;
			break;         
		case IO_DSC_SET_ENCRYPT_PRIORITY:                 
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_EN_PRIORITY));
			break;         
		case IO_DSC_GET_DRIVER_VERSION:
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 20);
			break;
		case IO_DSC_SET_CLR_CMDQ_EN:
			desc = NULL;
			break;
		case IO_DSC_DELETE_HANDLE_CMD:
			desc = NULL;
			break;	
		case IO_DSC_FIXED_DECRYPTION:			   
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_FIXED_CRYPTION));
			break;				
		case IO_DSC_SYS_UK_FW:			   
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_SYS_UK_FW));
			break;			
		default:
			break;
	}

	jump_to_func(NULL, ali_rpc_call, pDscDev, DSC_NPARA(3)|FUNC_DSC_IOCTL, desc);
#else    
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	DSC_PVR_KEY_PARAM pvr_key;
	Param p1;
	Param p2;
	Param p3;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&pDscDev);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&cmd); 
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&param);

	switch(cmd)
	{
		case (IO_PARSE_DMX_ID_GET_CMD):
			MINIRPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)param);            
			break;
		case (IO_PARSE_DMX_ID_SET_CMD):
			break;
		case (IO_DSC_GET_DES_HANDLE):
			MINIRPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)param);            
			break;
		case (IO_DSC_GET_AES_HANDLE):
			MINIRPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)param);
			break;
		case (IO_DSC_GET_CSA_HANDLE):
			MINIRPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)param);            
			break;
		case (IO_DSC_GET_SHA_HANDLE):
			MINIRPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)param);
			break;
		case (IO_DSC_SET_PVR_KEY_PARAM):
			ali_dsc_umemcpy(&pvr_key, (void *)param, sizeof(DSC_PVR_KEY_PARAM));
			if(pvr_key.input_addr)
			{
				ali_dsc_umemcpy(dsc->dsc_key, (const void *)pvr_key.input_addr, \
								pvr_key.valid_key_num*pvr_key.pvr_key_length/8);
				pvr_key.input_addr = (__u32)UC(dsc->dsc_key);
			}
			else
			{
				return RET_FAILURE;
			}
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Dsc_pvr_key_param_rpc, \
							sizeof(Dsc_pvr_key_param_rpc), (void *)&pvr_key);
			break;
		case IO_DSC_SET_PVR_KEY_IDLE:
			break;
		default:
			return RET_FAILURE;
	}

	return RpcCallCompletion(RPC_dsc_ioctl,&p1,&p2,&p3,NULL); 
#endif				

}

RET_CODE trig_ram_mon_ex(UINT32 start_addr,UINT32 end_addr, UINT32 interval, enum SHA_MODE sha_mode,BOOL DisableOrEnable)
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, start_addr,DSC_NPARA(5)|FUNC_TRIG_RAM_MON, NULL);
#else
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&start_addr);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&end_addr);   
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&interval);   
	MINIRPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_MINIRPC_UINT32,sizeof(__u32), (void *)&sha_mode);   
	MINIRPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_MINIRPC_INT32,sizeof(int), (void *)&DisableOrEnable);   

	return RpcCallCompletion(RPC_trig_ram_mon,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

RET_CODE trig_ram_mon(UINT32 start_addr,UINT32 end_addr, UINT32 interval, enum SHA_MODE sha_mode,BOOL DisableOrEnable)
{
	return trig_ram_mon_ex(start_addr,end_addr,interval,sha_mode,DisableOrEnable);
}

RET_CODE DeEncrypt_ex(pDEEN_CONFIG p_DeEn,UINT8 *input,UINT8 *output , UINT32 total_length)
{
#if !defined(CONFIG_ALI_RPCNG)
	UINT32 desc[] = {1,DESC_STATIC_STRU(0, sizeof(DEEN_CONFIG)),1,DESC_P_PARA(0,0,0),0,0,};
	jump_to_func(NULL, ali_rpc_call, p_DeEn,DSC_NPARA(4)|FUNC_DEENCRYPT, desc);
#else     
	Param p1;
	Param p2;
	Param p3;
	Param p4;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_DeEncrypt_config_rpc, sizeof(DeEncrypt_config_rpc), (void *)p_DeEn);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&input);   
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&output);   
	MINIRPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&total_length);   

	return RpcCallCompletion(RPC_DeEncrypt,&p1,&p2,&p3,&p4,NULL);
#endif				
}

RET_CODE DeEncrypt(pDEEN_CONFIG p_DeEn,UINT8 *input,UINT8 *output , UINT32 total_length)
{
	RET_CODE ret = RET_FAILURE;
	if(total_length>TS_MAX_SIZE)
	{
		return ret;
	}        
	dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, total_length * 188);	
	ret = DeEncrypt_ex(p_DeEn,input,output,total_length);
	dsc_deal_cache_after_dma((UINT32)output, total_length * 188);	

	return ret;
}
UINT16 dsc_get_free_stream_id(enum DMA_MODE dma_mode)
{
#if !defined(CONFIG_ALI_RPCNG)
	 jump_to_func(NULL, ali_rpc_call, dma_mode,DSC_NPARA(1)|FUNC_GET_FREE_STREAM_ID, NULL);
#else
	MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_ENUM, sizeof(enum DMA_MODE), &dma_mode);
	return RpcCallCompletion(RPC_dsc_get_free_stream_id,&p1,NULL);
#endif				  
}
UINT32 dsc_get_free_sub_device_id(enum WORK_SUB_MODULE sub_mode)
{
#if !defined(CONFIG_ALI_RPCNG)
	 jump_to_func(NULL, ali_rpc_call, sub_mode,DSC_NPARA(1)|FUNC_GET_FREE_SUB_DEVICE_ID, NULL);
#else
	MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_UCHAR, sizeof(__u8), &sub_mode);

	return RpcCallCompletion(RPC_dsc_get_free_sub_device_id,&p1,NULL);
#endif
}
RET_CODE dsc_set_sub_device_id_idle(enum WORK_SUB_MODULE sub_mode,UINT32 device_id)
{
#if !defined(CONFIG_ALI_RPCNG)
	 jump_to_func(NULL, ali_rpc_call, sub_mode,DSC_NPARA(2)|FUNC_SET_SUB_DEVICE_ID_IDLE, NULL);
#else
	Param p1;
	Param p2;
	
	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UCHAR, sizeof(__u8), (void *)&sub_mode);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&device_id);

	return RpcCallCompletion(RPC_dsc_set_sub_device_id_idle,&p1,&p2,NULL);
#endif
}
RET_CODE dsc_set_stream_id_idle(UINT32 pos) 
{
#if !defined(CONFIG_ALI_RPCNG)
	 jump_to_func(NULL, ali_rpc_call, pos,DSC_NPARA(1)|FUNC_SET_STREAM_ID_IDLE, NULL);
#else
	MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), &pos);

	return RpcCallCompletion(RPC_dsc_set_stream_id_idle,&p1,NULL);
#endif
}    

void dsc_set_stream_id_used(UINT32 pos) 
{
#if !defined(CONFIG_ALI_RPCNG)
	 jump_to_func(NULL, ali_rpc_call, pos,DSC_NPARA(1)|FUNC_SET_STREAM_ID_USED, NULL);
#else
	MINIRPC_PARAM_CREATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), &pos);

	RpcCallCompletion(RPC_dsc_set_stream_id_used,&p1,NULL);
#endif
} 
RET_CODE dsc_set_sub_device_id_used(enum WORK_SUB_MODULE sub_mode,UINT32 device_id)
{
#if !defined(CONFIG_ALI_RPCNG)
	 jump_to_func(NULL, ali_rpc_call, sub_mode,DSC_NPARA(2)|FUNC_SET_SUB_DEVICE_ID_USED, NULL);
#else
	Param p1;
	Param p2;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_ENUM, sizeof(enum WORK_SUB_MODULE), (void *)&sub_mode);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&device_id);

	return RpcCallCompletion(RPC_dsc_set_sub_device_id_used,&p1,&p2,NULL);
#endif
}

RET_CODE dsc_deal_quantum_for_mixed_ts(pDEEN_CONFIG p_DeEn, UINT32 temp_length)
{
#if !defined(CONFIG_ALI_RPCNG)
	 jump_to_func(NULL, ali_rpc_call, p_DeEn ,DSC_NPARA(2)|FUNC_DEAL_QUANTUM_MIXED_TS, NULL);
#else
	Param p1;
	Param p2;

	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_DeEncrypt_config_rpc, sizeof(DeEncrypt_config_rpc), (void *)p_DeEn);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), (void *)&temp_length);   

	return RpcCallCompletion(RPC_dsc_deal_quantum_for_mixed_ts,&p1,&p2,NULL);
#endif				  
}


#endif  /*_HLD_DSC_REMOTE*/

#endif  /*DUAL_ENABLE*/


RET_CODE dsc_fixed_cryption(UINT8 *input, UINT32 length, UINT32 pos)
{
	DSC_FIXED_CRYPTION param_fixed;
	pDSC_DEV pDscDev = NULL;
	RET_CODE ret = RET_FAILURE;
	
	pDscDev = ( pDSC_DEV ) dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
	if(NULL == pDscDev)
	{
		return ret;
	}
	
	dsc_deal_cache_before_dma((UINT32)input, (UINT32)input, length);
	param_fixed.input = input;
	param_fixed.length = length;
	param_fixed.pos = pos;
	ret = dsc_ioctl(pDscDev, IO_DSC_FIXED_DECRYPTION, (UINT32)&param_fixed);
	
	return ret;
}

RET_CODE dsc_deal_sys_uk_fw(UINT8 *input, UINT8 *output, UINT8 *key, UINT32 length,
							UINT8 pos, enum CRYPT_SELECT mode)
{
	pDSC_DEV pDscDev = NULL;
	RET_CODE ret = RET_FAILURE;
	DSC_SYS_UK_FW dsc_key;

	pDscDev = ( pDSC_DEV ) dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
	if(NULL == pDscDev)
	{
		return ret;
	}
	
	memset(&dsc_key, 0x00, sizeof(dsc_key));
	
	memcpy(dsc_key.ck,key,sizeof(dsc_key.ck));
	dsc_key.input = input;
	dsc_key.output = output; 
	dsc_key.length = length;
	dsc_key.mode = mode;
	dsc_key.pos = pos;
	dsc_deal_cache_before_dma((UINT32)input, (UINT32)output, length);
	ret = dsc_ioctl(pDscDev, IO_DSC_SYS_UK_FW, (UINT32)&dsc_key);
	dsc_deal_cache_after_dma((UINT32)output, length);
	
	return ret;
}


/*
0 ->Disable
1 -> Enable
*/
RET_CODE dsc_enable_disable_cmdq(UINT32 en_or_dis)
{
	pDSC_DEV pDscDev = dev_get_by_id(HLD_DEV_TYPE_DSC, 0);

	if(pDscDev)
	{
		return dsc_ioctl(pDscDev,IO_DSC_SET_CLR_CMDQ_EN,en_or_dis);
	}
	else
		return RET_FAILURE;
}

RET_CODE ali_sha_digest(UINT8 *input, UINT32 input_len, \
						enum SHA_MODE sha_mode, UINT8 *output)
{
	#define SHA_MODE_CHECK(mode)	(((mode) != SHA_SHA_1) && \
		((mode) != SHA_SHA_224) && ((mode) != SHA_SHA_256) && \
		((mode) != SHA_SHA_384 )&& ((mode) != SHA_SHA_512))

	UINT32 ShaDevID = ALI_INVALID_DSC_SUB_DEV_ID;
	pSHA_DEV pShaDev = NULL;
	SHA_INIT_PARAM param;
	RET_CODE ret = RET_FAILURE;
	
    if ( !input || ! output || ! input_len || SHA_MODE_CHECK ( sha_mode ) )
    {
            DSC_REMOTE_API_PRINTF( "%s(): Wrong para!\n",__FUNCTION__);
            return RET_FAILURE;
    }

	if (ALI_INVALID_DSC_SUB_DEV_ID == (ShaDevID = dsc_get_free_sub_device_id(SHA)))
	{
		DSC_REMOTE_API_PRINTF("%s() get free SHA device failed!\n", __FUNCTION__);
		return RET_FAILURE;
	}

	if (NULL == (pShaDev = (pSHA_DEV)dev_get_by_id(HLD_DEV_TYPE_SHA, ShaDevID)))
	{
		DSC_REMOTE_API_PRINTF("%s() get SHA device %d failed!\n", \
								__FUNCTION__, ShaDevID);
		dsc_set_sub_device_id_idle(SHA, ShaDevID);
		return RET_FAILURE;
	}

	memset(&param, 0, sizeof(SHA_INIT_PARAM));
	param.sha_data_source = SHA_DATA_SOURCE_FROM_DRAM;
	param.sha_work_mode = sha_mode;
	sha_ioctl(pShaDev, IO_INIT_CMD, (UINT32)&param);
	ret = sha_digest(pShaDev, input, (UINT8 *)output, input_len);

	dsc_set_sub_device_id_idle(SHA, ShaDevID);
	return ret;
}


/* This function is used to decrypt or encrypt the puredata use key from crypto engine
u8 *input,                the input data
u8 *output,               the output data
u32 length,               the data length
u32 key_pos,              the key pos in crypto engine
enum CRYPT_SELECT sel     decrypt or encrypt select
*/
RET_CODE aes_crypt_puredata_with_ce_key(UINT8 *input, UINT8 *output, UINT32 length, 
								UINT32 key_pos, enum CRYPT_SELECT sel)
{
	UINT32 AesDevId = INVALID_DSC_SUB_DEV_ID;
	pAES_DEV  pAesDev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	RET_CODE ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	UINT16 pid[1] = {0x1234};    
	AES_IV_INFO iv;

	if ((AesDevId = dsc_get_free_sub_device_id(AES)) == INVALID_DSC_SUB_DEV_ID)
	{
		DSC_REMOTE_API_PRINTF("dsc_get_free_sub_device_id() failed\n");
		return -1;
	}

	if ((pAesDev = dev_get_by_id(HLD_DEV_TYPE_AES, AesDevId)) == NULL)
	{
		DSC_REMOTE_API_PRINTF("%s() get AES device %d failed!\n", __FUNCTION__, AesDevId);
		dsc_set_sub_device_id_idle(AES, AesDevId);
		return -1;
	}

	if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
		DSC_REMOTE_API_PRINTF("%s() get free stream id failed!\n", __FUNCTION__);
		dsc_set_sub_device_id_idle(AES, AesDevId);
		return -1;
	}
	
	MEMSET(iv.even_iv, 0, sizeof(iv.even_iv));
	MEMSET(iv.odd_iv, 0, sizeof(iv.odd_iv));   	
	MEMSET(&aes_param, 0, sizeof(struct aes_init_param));
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_CRYPTO;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	aes_param.scramble_control = 0 ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_CBC ;
	aes_param.cbc_cts_enable = 0;
	aes_ioctl(pAesDev ,IO_INIT_CMD , (UINT32)&aes_param);

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL ; 
	key_param.init_vector = &iv ;
	key_param.key_length = 128;  
	key_param.pid_len = 1;
	key_param.pid_list = pid;
	key_param.p_aes_iv_info = (void *)&iv ;
	key_param.p_aes_key_info = NULL;
	key_param.stream_id = stream_id; //PURE_ID; /*DMx 0*/ /*0-3 for dmx id , 4-7 for pure data mode*/
	key_param.force_mode = 1;
	key_param.pos = key_pos;

	aes_ioctl(pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);	
	
	if(sel==DSC_ENCRYPT)
		ret = aes_encrypt(pAesDev, stream_id, input, output, length);
	else
		ret = aes_decrypt(pAesDev, stream_id, input, output, length);
	
	if(ret!=RET_SUCCESS)
	{
		DSC_REMOTE_API_PRINTF("%s() aes crypt fail!\n", __FUNCTION__);
		aes_ioctl(pAesDev ,IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
		dsc_set_stream_id_idle(stream_id);
		dsc_set_sub_device_id_idle(AES, AesDevId);
		return -1;
	}
	
	aes_ioctl(pAesDev ,IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
	dsc_set_stream_id_idle(stream_id);
	dsc_set_sub_device_id_idle(AES, AesDevId);
	return RET_SUCCESS;
}


/* This function is used to decrypt or encrypt the puredata use key from sram
UINT8 *key,                  the key
UINT32 klen,                 the key length
UINT8 *input,                the input data
UINT8 *output,               the output data
UINT32 length,               the data length
enum CRYPT_SELECT crypt_mode     decrypt or encrypt select
*/
RET_CODE aes_crypt_puredata_with_sram_key(UINT8 *input, UINT8 *output, UINT32 length, \
                                                UINT8 *key, UINT32 klen, enum CRYPT_SELECT crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    pAES_DEV pAesDev = NULL;
    struct aes_init_param aes_param;
    KEY_PARAM key_param;
    AES_KEY_PARAM key_info[1];
    AES_IV_INFO iv[1];
    
    UINT32 device_id = dsc_get_free_sub_device_id(AES);
    if(device_id == INVALID_DSC_SUB_DEV_ID)
    {
        return ret;
    }
    pAesDev = (pAES_DEV)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);
	
    if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
        goto DONE1;
	}
	
    memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
    aes_param.dma_mode = PURE_DATA_MODE;
    aes_param.key_from = KEY_FROM_SRAM;
    aes_param.key_mode = AES_128BITS_MODE ;
    aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE;
    aes_param.scramble_control = 0 ;
    aes_param.work_mode = WORK_MODE_IS_CBC;
	aes_param.stream_id = stream_id;
    aes_param.cbc_cts_enable = 0;
    ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );
    if(ret != RET_SUCCESS)
    {
        goto DONE2;
    }

	memset(&key_param, 0x00, sizeof(KEY_PARAM));
	memset(iv, 0x00, sizeof(iv));

    memset(key_info, 0x00, sizeof(key_info));
    if (16 == klen)
    {
        memcpy(key_info[0].aes_128bit_key.even_key, key, 16);
    }
    else if (24 == klen)
    {
        memcpy(key_info[0].aes_192bit_key.even_key, key, 24);
    }
    else if (32 == klen)
    {
        memcpy(key_info[0].aes_256bit_key.even_key, key, 32);
    }
    else
    {
        goto DONE2;
    }

	key_param.handle = 0xFF;
	key_param.ctr_counter = NULL; 
	key_param.init_vector = iv;
	key_param.key_length = 128;  
	key_param.pid_len = 1; 				
	key_param.p_aes_key_info = key_info;
    key_param.p_aes_iv_info = iv;
	key_param.stream_id = stream_id;
	ret=aes_ioctl (pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if(ret != RET_SUCCESS)
    {
        goto DONE2;
    }
    
    if(crypt_mode == DSC_ENCRYPT)
    {
        ret = aes_encrypt( pAesDev, stream_id, input, output, length );
    }
    else
    {
        ret = aes_decrypt( pAesDev, stream_id, input, output, length );
    }
	

    aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
    dsc_set_stream_id_idle(stream_id);
DONE1:
    dsc_set_sub_device_id_idle(AES, device_id);
    return ret;
}

/* 
	crypt_mode -> AES_128_CBC  DSC_ENCRYPT / DSC_DECRYPT
	Force iv setting to -> 0,0,0, ...
*/

RET_CODE crypt_by_otp_key6( UINT8 *input, UINT8 *output, UINT32 length, UINT8 crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    pAES_DEV pAesDev = NULL;
    struct aes_init_param aes_param;
    KEY_PARAM key_param;
    UINT8 iv[16];
    UINT16 pid = 0;
   
    UINT32 device_id = dsc_get_free_sub_device_id(AES);
    if(device_id == INVALID_DSC_SUB_DEV_ID)
        return ret;
    pAesDev = (pAES_DEV)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);
    
    if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
        goto DONE1;
	}

    MEMSET ( &aes_param, 0, sizeof ( struct aes_init_param ) );
    aes_param.dma_mode = PURE_DATA_MODE;
    aes_param.key_from = KEY_FROM_OTP;
    aes_param.key_mode = AES_128BITS_MODE ;
    aes_param.parity_mode = OTP_KEY_FROM_68;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    aes_param.scramble_control = 0 ;
    aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_CBC ;
    aes_param.cbc_cts_enable = 0;
    ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );
    if(ret != RET_SUCCESS){
        goto DONE2;
    }

    MEMSET ( iv, 0, sizeof (iv) );
    MEMSET ( &key_param, 0, sizeof ( KEY_PARAM ) );
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL ;
	key_param.init_vector = iv ;
	key_param.key_length = 128;
	key_param.pid_len = 1;
	key_param.pid_list = &pid;
	key_param.p_aes_iv_info = iv ;
	key_param.p_aes_key_info = output;
	key_param.stream_id = stream_id;
	key_param.force_mode = 1;
	key_param.pos = 0;
	ret=aes_ioctl (pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if(ret != RET_SUCCESS){
        goto DONE2;
    }
    
    if(crypt_mode == DSC_ENCRYPT)
        ret = aes_encrypt( pAesDev, stream_id, input, output, length );
    else
        ret = aes_decrypt( pAesDev, stream_id, input, output, length );
    
    aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
    dsc_set_stream_id_idle(stream_id);
DONE1:
    dsc_set_sub_device_id_idle(AES, device_id);
    return ret;
}

/* 
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE tdes_ecb_crypt_by_otp_key6( UINT8 *input, UINT8 *output, UINT32 length, UINT8 crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    pDES_DEV pDesDev = NULL;
    struct des_init_param des_param;
    KEY_PARAM key_param;
    UINT16 pid = 0;
   
    UINT32 device_id = dsc_get_free_sub_device_id(DES);
    if(device_id == INVALID_DSC_SUB_DEV_ID)
        return ret;
    pDesDev = (pDES_DEV)dev_get_by_id(HLD_DEV_TYPE_DES, device_id);
    
    if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
        goto DONE1;
	}

    MEMSET ( &des_param, 0, sizeof ( struct des_init_param ) );
    des_param.dma_mode = PURE_DATA_MODE;
    des_param.key_from = KEY_FROM_OTP;
    des_param.key_mode = TDES_ABA_MODE ;
    des_param.parity_mode = OTP_KEY_FROM_68;
    des_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    des_param.scramble_control = 0 ;
    des_param.stream_id = stream_id;
    des_param.work_mode = WORK_MODE_IS_ECB ;
    des_param.cbc_cts_enable = 0;
	des_param.sub_module = TDES;
    ret=des_ioctl ( pDesDev , IO_INIT_CMD , ( UINT32 ) &des_param );
    if(ret != RET_SUCCESS){
        goto DONE2;
    }

    MEMSET ( &key_param, 0, sizeof ( KEY_PARAM ) );
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL ;
	key_param.key_length = 128;
	key_param.pid_len = 1;
	key_param.pid_list = &pid;
	key_param.stream_id = stream_id;
	key_param.force_mode = 1;
	ret=des_ioctl (pDesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if(ret != RET_SUCCESS){
        goto DONE2;
    }
    
    if(crypt_mode == DSC_ENCRYPT)
        ret = des_encrypt( pDesDev, stream_id, input, output, length );
    else
        ret = des_decrypt( pDesDev, stream_id, input, output, length );
    

    des_ioctl( pDesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
    dsc_set_stream_id_idle(stream_id);
DONE1:
    dsc_set_sub_device_id_idle(DES, device_id);
    return ret;
}


/* 
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE aes_pure_ecb_crypt( UINT8 *key, UINT8 *input, 
							UINT8 *output, UINT32 length, 
							UINT8 crypt_mode)
{
	int ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	pAES_DEV pAesDev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	AES_KEY_PARAM key_info[1];
	UINT16 pid[1] = {0x1234};

	UINT32 device_id = dsc_get_free_sub_device_id(AES);
	if(device_id == INVALID_DSC_SUB_DEV_ID)
	    return ret;
	pAesDev = (pAES_DEV)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

	if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == \
		INVALID_DSC_STREAM_ID)
	{
		goto DONE1;
	}

	memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_SRAM;
	aes_param.key_mode = AES_128BITS_MODE;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	aes_param.scramble_control = 0 ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_ECB ;
	ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));
	memcpy(key_info[0].aes_128bit_key.even_key, key, 16);
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL; 
	key_param.init_vector = NULL;
	key_param.key_length = 128;  
	key_param.pid_len = 1; 				
	key_param.pid_list = pid; 			
	key_param.p_aes_key_info = key_info;
	key_param.stream_id = stream_id;
	ret=aes_ioctl (pAesDev,IO_CREAT_CRYPT_STREAM_CMD, (UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	if(crypt_mode == DSC_ENCRYPT)
		ret = aes_encrypt( pAesDev, stream_id, input, output, length );
	else
		ret = aes_decrypt( pAesDev, stream_id, input, output, length );


	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}

/* 
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE aes_pure_cbc_crypt( UINT8 *key, UINT8 *iv, \
							UINT8 *input, UINT8 *output, \
							UINT32 length, UINT8 crypt_mode)
{
	int ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	pAES_DEV pAesDev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	AES_KEY_PARAM key_info[1];
	AES_IV_INFO iv_info;
	UINT16 pid[1] = {0x1234};

	UINT32 device_id = dsc_get_free_sub_device_id(AES);
	if(device_id == INVALID_DSC_SUB_DEV_ID)
	    return ret;
	pAesDev = (pAES_DEV)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

	if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == \
		INVALID_DSC_STREAM_ID)
	{
		goto DONE1;
	}

	memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_SRAM;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	aes_param.scramble_control = 0 ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_CBC ;
	ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );
	if(ret != RET_SUCCESS)
	{
	    goto DONE2;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));
	memcpy(key_info[0].aes_128bit_key.even_key, key, 16);
	memcpy(iv_info.even_iv, iv, 16);
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL; 
	key_param.init_vector = NULL;
	key_param.key_length = 128;  
	key_param.pid_len = 1; 				
	key_param.pid_list = pid; 			
	key_param.p_aes_iv_info = &iv_info;
	key_param.p_aes_key_info = key_info;
	key_param.stream_id = stream_id;
	ret=aes_ioctl (pAesDev,IO_CREAT_CRYPT_STREAM_CMD,(UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	if(crypt_mode == DSC_ENCRYPT)
		ret = aes_encrypt( pAesDev, stream_id, input, output, length );
	else
		ret = aes_decrypt( pAesDev, stream_id, input, output, length );


	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}


/* 
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT (same)
*/
RET_CODE aes_pure_ctr_crypt( UINT8 *key, UINT8 *ctr, 
							UINT8 *input, UINT8 *output, 
							UINT32 length, UINT8 crypt_mode)
{
	int ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	pAES_DEV pAesDev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	AES_KEY_PARAM key_info[1];
	UINT16 pid[1] = {0x1234};

	UINT32 device_id = dsc_get_free_sub_device_id(AES);
	if(device_id == INVALID_DSC_SUB_DEV_ID)
		return ret;
	pAesDev = (pAES_DEV)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

	if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == \
		INVALID_DSC_STREAM_ID)
	{
		goto DONE1;
	}

	memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_SRAM;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_AS_ATSC;
	aes_param.scramble_control = 0 ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_CTR ;
	ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));
	memcpy(key_info[0].aes_128bit_key.even_key, key, 16);
	key_param.handle = 0xFF ;
	key_param.ctr_counter = ctr; 
	key_param.init_vector = NULL;
	key_param.key_length = 128;  
	key_param.pid_len = 1;				
	key_param.pid_list = pid;			
	key_param.p_aes_key_info = key_info;
	key_param.stream_id = stream_id;
	ret=aes_ioctl (pAesDev,IO_CREAT_CRYPT_STREAM_CMD,(UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	if(crypt_mode == DSC_ENCRYPT)
		ret = aes_encrypt( pAesDev, stream_id, input, output, length );
	else
		ret = aes_decrypt( pAesDev, stream_id, input, output, length );


	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}


/* 
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE aes_pure_ctr_crypt_with_sram_key( UINT8 *key, UINT8 *ctr, UINT8 *input, 
	UINT8 *output, UINT32 length, UINT8 crypt_mode)
{
	RET_CODE ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	pAES_DEV pAesDev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	AES_KEY_PARAM key_info[1];


	UINT32 device_id = dsc_get_free_sub_device_id(AES);
	if(device_id == INVALID_DSC_SUB_DEV_ID)
		return ret;
	pAesDev = (pAES_DEV)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

	if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
		goto DONE1;
	}

	memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_SRAM;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_AS_ATSC;
	aes_param.scramble_control = 0 ;
	aes_param.work_mode = WORK_MODE_IS_CTR;
	aes_param.stream_id = stream_id;
	ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));
	memcpy(key_info[0].aes_128bit_key.even_key, key, 16);
	key_param.handle = 0xFF ;
	key_param.ctr_counter = ctr; 
	key_param.init_vector = NULL;
	key_param.key_length = 128;  
	key_param.pid_len = 1; 				
	key_param.p_aes_key_info = key_info;
	key_param.stream_id = stream_id;
	ret=aes_ioctl (pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	if(crypt_mode == DSC_ENCRYPT)
		ret = aes_encrypt( pAesDev, stream_id, input, output, length );
	else
		ret = aes_decrypt( pAesDev, stream_id, input, output, length );


	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}


/* 
   crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE tdes_pure_ecb_crypt( UINT8 *key, UINT8 *input, UINT8 *output, UINT32 length, UINT8 crypt_mode)
{
	RET_CODE ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	pDES_DEV pDesDev = NULL;
	struct des_init_param des_param;
	KEY_PARAM key_param;
	DES_KEY_PARAM key_info[1];
	UINT16 pid[1] = {0x1234};

	UINT32 device_id = dsc_get_free_sub_device_id(DES);
	if(device_id == INVALID_DSC_SUB_DEV_ID)
		return ret;
	pDesDev = (pDES_DEV)dev_get_by_id(HLD_DEV_TYPE_DES, device_id);

	if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
		goto DONE1;
	}


	memset( &des_param, 0, sizeof ( struct des_init_param ) );
	des_param.sub_module = TDES;
	des_param.dma_mode = PURE_DATA_MODE;
	des_param.key_from = KEY_FROM_SRAM;
	des_param.key_mode = TDES_ABA_MODE ;
	des_param.parity_mode = EVEN_PARITY_MODE;
	des_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	des_param.scramble_control = 0 ;
	des_param.stream_id = stream_id;
	des_param.work_mode = WORK_MODE_IS_ECB ;
	ret=des_ioctl ( pDesDev , IO_INIT_CMD , ( UINT32 ) &des_param );
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));
	memcpy(key_info[0].des_128bits_key.EvenKey, key, 16);
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL; 
	key_param.init_vector = NULL;
	key_param.key_length = 128;  
	key_param.pid_len = 1; 				
	key_param.pid_list = pid; 			
	key_param.p_des_key_info = key_info;
	key_param.stream_id = stream_id;
	ret=des_ioctl (pDesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	if(crypt_mode == DSC_ENCRYPT)
		ret = des_encrypt( pDesDev, stream_id, input, output, length );
	else
		ret = des_decrypt( pDesDev, stream_id, input, output, length );


	des_ioctl( pDesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(DES, device_id);
	return ret;
}

/* 
	key_pos -> CE key pos
	length should < 64MB
	crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE tdes_pure_ecb_crypt_with_ce_key( UINT8 key_pos, UINT8 *input, UINT8 *output, UINT32 
length, UINT8 crypt_mode)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 stream_id = INVALID_DSC_STREAM_ID;
    pDES_DEV pDesDev = NULL;
    struct des_init_param des_param;
    KEY_PARAM key_param;
    UINT16 pid[1] = {0x1234};
		
    UINT32 device_id = dsc_get_free_sub_device_id(DES);
    if(device_id == INVALID_DSC_SUB_DEV_ID)
        return ret;
    pDesDev = (pDES_DEV)dev_get_by_id(HLD_DEV_TYPE_DES, device_id);
    
    if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
        goto DONE1;
	}

    memset( &des_param, 0, sizeof ( struct des_init_param ) );
    des_param.sub_module = TDES;
    des_param.dma_mode = PURE_DATA_MODE;
    des_param.key_from = KEY_FROM_CRYPTO;
    des_param.key_mode = TDES_ABA_MODE;
    des_param.parity_mode = EVEN_PARITY_MODE;
    des_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    des_param.scramble_control = 0 ;
    des_param.stream_id = stream_id;
    des_param.work_mode = WORK_MODE_IS_ECB ;
    des_param.cbc_cts_enable = 0;
    ret=des_ioctl ( pDesDev , IO_INIT_CMD , ( UINT32 ) &des_param );
    if(ret != RET_SUCCESS){
        goto DONE2;
    }

	memset(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL; 
	key_param.init_vector = NULL;
	key_param.key_length = 128;  
	key_param.pid_len = 1; 				
	key_param.pid_list = pid; 
	key_param.stream_id = stream_id;
	key_param.force_mode = 1;
    key_param.pos = key_pos;
	ret=des_ioctl (pDesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if(ret != RET_SUCCESS){
        goto DONE2;
    }
    
    if(crypt_mode == DSC_ENCRYPT)
        ret = des_encrypt( pDesDev, stream_id, input, output, length );
    else
        ret = des_decrypt( pDesDev, stream_id, input, output, length );
    

    des_ioctl( pDesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
    dsc_set_stream_id_idle(stream_id);
DONE1:
    dsc_set_sub_device_id_idle(DES, device_id);
    return ret;
}


/* 
	key_pos -> key pos in keyladder 
	ctr -> as the IV/Counter
	length should < 64MB
	crypt_mode -> DSC_ENCRYPT / DSC_DECRYPT
*/
RET_CODE aes_pure_ctr_crypt_with_ce_key( UINT8 key_pos, UINT8 *ctr, UINT8 *input, 
						UINT8 *output, UINT32 length, UINT8 crypt_mode)
{
	RET_CODE ret = RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	pAES_DEV pAesDev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;

	if(0)//(sys_ic_dsc_access_ce_disable())
	{
		DSC_REMOTE_API_PRINTF("Not support this function\n");
		return ret;
	}
		
	UINT32 device_id = dsc_get_free_sub_device_id(AES);
	if(device_id == INVALID_DSC_SUB_DEV_ID)
		return ret;
	pAesDev = (pAES_DEV)dev_get_by_id(HLD_DEV_TYPE_AES, device_id);

	if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
		goto DONE1;
	}

	memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_CRYPTO;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_AS_ATSC;
	aes_param.scramble_control = 0 ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_CTR;
	aes_param.cbc_cts_enable = 0;
	ret=aes_ioctl ( pAesDev , IO_INIT_CMD , ( UINT32 ) &aes_param );
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF ;
	key_param.ctr_counter = ctr; 
	key_param.init_vector = NULL;
	key_param.key_length = 128;  
	key_param.pid_len = 1; 				
	key_param.force_mode = 1;
	key_param.stream_id = stream_id;
	key_param.pos = key_pos;
	ret=aes_ioctl (pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		goto DONE2;
	}

	if(crypt_mode == DSC_ENCRYPT)
		ret = aes_encrypt( pAesDev, stream_id, input, output, length );
	else
		ret = aes_decrypt( pAesDev, stream_id, input, output, length );

	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}

/*
    this function used to encrypt the bootloader universal key
    encrypt_type
    0: encrypt bl uk use key 6
    1: encrypt bl uk use key 6 with r1
    2: encrypt bl uk use key 7
*/
RET_CODE ali_dsc_encrypt_bl_uk(UINT8 *input, UINT8 *r_key, UINT8 *output, UINT32 encrypt_type)
{
	pDSC_DEV  pDscDev = NULL;
    DSC_BL_UK_PARAM bl_uk_param;
    RET_CODE ret = RET_FAILURE;
    pDscDev = dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
    if(NULL==pDscDev)
        return RET_FAILURE;
    MEMSET(&bl_uk_param, 0 ,sizeof(DSC_BL_UK_PARAM));
    bl_uk_param.input_key = input;
    bl_uk_param.r_key = r_key;
    bl_uk_param.output_key = output;
    osal_cache_flush(bl_uk_param.input_key, ALIASIX_BL_UK_LEN);
    osal_cache_flush(bl_uk_param.r_key, ALIASIX_BL_UK_LEN);
    ret = dsc_ioctl(pDscDev,IO_DSC_ENCRYTP_BL_UK,&bl_uk_param);
    return ret;
}
