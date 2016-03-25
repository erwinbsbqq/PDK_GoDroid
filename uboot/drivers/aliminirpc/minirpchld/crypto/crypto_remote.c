#include <ali/basic_types.h>
#include <ali/sys_define.h>
#include <ali/hld_dev.h>
#include <ali/crypto.h>


#include <ali/flash_cipher.h>
#include <ali/osal.h>
#include <ali/string.h>

#include <ali_rpc.h>
#include <ali_minirpc_service.h>
#include <ali_rpcng.h>


#define CONFIG_ALI_RPCNG

#define INVALID_CE_KEY_POS 0xff

#define CE_REMOTE_API_DEBUG
#ifdef CE_REMOTE_API_DEBUG
#ifdef CE_REMOTE_API_PRINTF
#undef CE_REMOTE_API_PRINTF
#endif
#ifdef CE_REMOTE_API_DUMP
#undef CE_REMOTE_API_DUMP
#endif
#define CE_REMOTE_API_PRINTF printf
#define CE_REMOTE_API_DUMP(data,len) \
    do{ \
        int i, l=(len); \
        for(i=0; i<l; i++){ \
            CE_REMOTE_API_PRINTF("0x%x,",*(data+i)); \
            if((i+1)%16==0) \
                CE_REMOTE_API_PRINTF("\n");\
        }\
    }while(0)
#else
#define CE_REMOTE_API_PRINTF(...)  do{}while(0)
#define CE_REMOTE_API_DUMP(...)  do{}while(0)
#endif


#define offsetof(type, f) ((unsigned long) \
((char *)&((type *)0)->f - (char *)(type *)0))

#if 1
#define ALI_CE_RPC_DEBUG(...)
#else
#define ALI_CE_RPC_DEBUG(fmt, arg...) \
    do { \
        printf("ALI_CE_RPC: In %s %d "fmt, __func__, __LINE__, ##arg); \
    } while (0)
#endif


#if 1 //#ifdef DUAL_ENABLE

//#include <ali/modules.h>

enum HLD_CE_FUNC
{
	FUNC_CE_ATTACH = 0,
	FUNC_CE_SET_AKSV,
	FUNC_PATCH_HDMI,
	FUNC_CE_DETACH,	
	FUNC_CE_GENERATE,
	FUNC_CE_LOAD,
	FUNC_CE_IOCTL,
	FUNC_CE_GENERATE_CW_KEY,
	FUNC_CE_GENERATE_SINGLE_LEVEL_KEY,
};

#if 1 //#ifndef _HLD_CRYPTO_REMOTE
UINT32 hld_ce_entry[] = 
{
	(UINT32)ce_api_attach,
	(UINT32)hdmi_set_aksv,
	(UINT32)patch_write_bksv2hdmi,
	(UINT32)ce_api_detach,	
	(UINT32)ce_key_generate,
	(UINT32)ce_key_load,
	(UINT32)ce_ioctl,
	(UINT32)ce_generate_cw_key,
	(UINT32)ce_generate_single_level_key,
};
UINT32 ce_m36_entry[] =
{
};

int ali_ce_umemcpy(void *dest, const void *src, __u32 n)
{
	int ret = 0;
	
	memcpy(dest, src, n);
	
	return ret;
}

void hld_crypto_callee(UINT8 *msg)
{
//	OS_hld_callee(hld_ce_entry, msg);
}
void lld_crypto_m36f_callee( UINT8 *msg )
{
//	OS_hld_callee(ce_m36_entry, msg);
}

#endif  /*_HLD_CE_REMOTE*/

#if 1 //ifdef _HLD_CRYPTO_REMOTE
#define CE_NPARA(x) ((HLD_CRYPTO_MODULE<<24)|(x<<16))

UINT32 ce_io_control[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, 0),
  1, DESC_P_PARA(0, 2, 0), 
  //desc of pointer ret
  0,                          
  0,
};

RET_CODE ce_api_attach_ex( void )
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, NULL, CE_NPARA(0) | FUNC_CE_ATTACH, NULL);
#else 
    RpcCallCompletion(RPC_ce_api_attach, NULL);
#endif
}

RET_CODE ce_api_detach_ex( void )
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, NULL, CE_NPARA(0) | FUNC_CE_DETACH, NULL);
#else 
    RpcCallCompletion(RPC_ce_api_detach, NULL);
#endif
}

RET_CODE  ce_attach_dev_register(void)
{
	hld_dev_get_remote("CE_M3602_0");
}
RET_CODE ce_api_attach( void )
{
	RET_CODE ret = RET_SUCCESS;
	CE_DEVICE *pCeDev = (CE_DEVICE *)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);
	if(!pCeDev)
	{
		ret = ce_api_attach_ex();
		ce_attach_dev_register();
		return ret;
	}
	return ALI_CRYPTO_WARNING_DRIVER_ALREADY_INITIALIZED;
}

RET_CODE ce_api_detach( void )
{
	return ce_api_detach_ex();
}

void hdmi_set_aksv(void)
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, NULL, CE_NPARA(0) | FUNC_CE_SET_AKSV, NULL);
#else 
    RpcCallCompletion(RPC_hdmi_set_aksv, NULL);
#endif
}

void patch_write_bksv2hdmi(UINT8 *data)
{
#if !defined(CONFIG_ALI_RPCNG)
	UINT32 desc[] =
	{
		1, DESC_OUTPUT_STRU(0, 5), 1, DESC_P_PARA(0, 0, 0), 0, 0, 
	};
	jump_to_func(NULL, ali_rpc_call, data, CE_NPARA(1) | FUNC_PATCH_HDMI, desc);
#else 

	Param p1;

    MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UCHAR, sizeof(UINT8), data);
    RpcCallCompletion(RPC_patch_write_bksv2hdmi, &p1, NULL);
#endif
}


RET_CODE ce_key_generate(CE_DEVICE *pCeDev,CE_DATA_INFO *pCe_data_info)
{
#if !defined(CONFIG_ALI_RPCNG)
	UINT32 desc[] = 
	{ //desc of pointer para
		1, DESC_STATIC_STRU(0, sizeof(CE_DATA_INFO)),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,                          
		0,
	};
    jump_to_func(NULL, ali_rpc_call, pCeDev, CE_NPARA(2)|FUNC_CE_GENERATE, desc);

#else
	Param dev;
	Param data_info;
	
	MINIRPC_PARAM_UPDATE(dev, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), &pCeDev);
	MINIRPC_PARAM_UPDATE(data_info, PARAM_IN, PARAM_MINIRPC_Ce_data_info_rpc, sizeof(Ce_data_info_rpc), pCe_data_info);

	return RpcCallCompletion(RPC_ce_key_generate,&dev,&data_info,NULL);
#endif
}

RET_CODE ce_key_load(CE_DEVICE *pCeDev,pOTP_PARAM pCe_opt_info)
{
#if !defined(CONFIG_ALI_RPCNG)
    UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_STATIC_STRU(0, sizeof(OTP_PARAM)),
      1, DESC_P_PARA(0, 1, 0),
      //desc of pointer ret
      0,                          
      0,
    };
    jump_to_func(NULL, ali_rpc_call, pCeDev, CE_NPARA(2)|FUNC_CE_LOAD, desc);
#else
	Param dev;
	Param otp_info;

	MINIRPC_PARAM_UPDATE(dev, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), &pCeDev);
	MINIRPC_PARAM_UPDATE(otp_info, PARAM_IN, PARAM_MINIRPC_Otp_param_rpc, sizeof(Otp_param_rpc), pCe_opt_info);

	return RpcCallCompletion(RPC_ce_key_load,&dev,&otp_info,NULL);
#endif
}


RET_CODE ce_ioctl(CE_DEVICE *pCeDev,UINT32 cmd,UINT32 param)
{
#if !defined(CONFIG_ALI_RPCNG)
    UINT32 i;
    UINT32 common_desc[sizeof(ce_io_control)];
    UINT32 *desc = (UINT32 *)common_desc;
    UINT32 *b = (UINT32 *)ce_io_control;
    
    for(i = 0; i < sizeof(ce_io_control)/sizeof(UINT32); i++)
    {
        desc[i] = b[i];
    }
    switch (cmd)
    {
        case IO_OTP_ROOT_KEY_GET:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(OTP_PARAM));
        break;

        case IO_CRYPT_DATA_INPUT:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DATA_PARAM));
        break;

        case IO_CRYPT_PARAM_SET:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DES_PARAM));
        break;

        case IO_CRYPT_SECOND_KEY_CONFIG:
           DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CE_KEY_PARAM));    	
        break;

        case IO_SECOND_KEY_GENERATE:
            desc = NULL ;
        break;

        case IO_CRYPT_DEBUG_GET_KEY:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(CE_DEBUG_KEY_INFO)); 
            break;
        case IO_CRYPT_POS_IS_OCCUPY:
        {  
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(CE_POS_STS_PARAM));  
        }
        break;
        case IO_CRYPT_POS_SET_USED:
        {
            desc=NULL;
        }
        break;
        case IO_CRYPT_POS_SET_IDLE:
        {           
            desc=NULL;
        }
        break;
        case IO_CRYPT_FOUND_FREE_POS:
        {
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
        }
        break;
        case IO_DECRYPT_PVR_USER_KEY:
        {
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CE_PVR_KEY_PARAM));
        }
        break;
        default:
            SDBBP();
        break;
        
    }
    jump_to_func(NULL, ali_rpc_call, pCeDev, CE_NPARA(3)|FUNC_CE_IOCTL, desc);
#else
	RET_CODE ret = RET_FAILURE;
	CE_DEBUG_KEY_INFO kdebug_param;
	CE_FOUND_FREE_POS_PARAM kpos_param;
	CE_POS_STS_PARAM ksts_param;
	CE_DEVICE *dev_in = (CE_DEVICE *)pCeDev;
	__u32 cmd_in = cmd;
	void *param_in = (void *)param;
	Param p1;
	Param p2;
	Param p3;
	
	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(long), (void *)&dev_in);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(long), (void *)&cmd_in);
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(long), (void *)&param_in);

	switch (cmd)
	{
		case IO_OTP_ROOT_KEY_GET:
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Otp_param_rpc, \
								sizeof(Otp_param_rpc), param_in);          
			break;

		case IO_CRYPT_DATA_INPUT:
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Data_param_rpc, \
								sizeof(Data_param_rpc), param_in);
			break;

		case IO_CRYPT_PARAM_SET:
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Des_param_rpc, \
								sizeof(Des_param_rpc), param_in);
			break;

		case IO_CRYPT_SECOND_KEY_CONFIG:
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Ce_key_param_rpc, \
								sizeof(Ce_key_param_rpc), param_in);
			break;

		case IO_SECOND_KEY_GENERATE:
			break;

		case IO_CRYPT_DEBUG_GET_KEY:
			ali_ce_umemcpy(&kdebug_param, param_in, sizeof(kdebug_param));
			MINIRPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_MINIRPC_Ce_debug_key_info_rpc, \
								sizeof(Ce_debug_key_info_rpc), &kdebug_param);
			break;
			
		case IO_CRYPT_POS_IS_OCCUPY:
			ali_ce_umemcpy(&ksts_param, param_in, sizeof(ksts_param));
			MINIRPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_MINIRPC_Ce_pos_status_param_rpc, \
								sizeof(Ce_pos_status_param_rpc), &ksts_param);
			break;
			
		case IO_CRYPT_POS_SET_USED:
			break;
		
		case IO_CRYPT_POS_SET_IDLE:         
			break;
		
		case IO_CRYPT_FOUND_FREE_POS:
			ali_ce_umemcpy(&kpos_param, param_in, sizeof(kpos_param));
			MINIRPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_MINIRPC_Ce_found_free_pos_param_rpc, \
								sizeof(Ce_found_free_pos_param_rpc), &kpos_param);            
			break;
			
		case IO_DECRYPT_PVR_USER_KEY:
			MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_Ce_pvr_key_param_rpc, 
								sizeof(Ce_pvr_key_param_rpc), param_in);
			break;
		
		default:
		{
			ALI_CE_RPC_DEBUG("CE rpc Error: invalid cmd=0x%x\n",cmd_in);
			break;            
		}
	}

	ret =  RpcCallCompletion(RPC_ce_ioctl,&p1,&p2,&p3,NULL);

	if(IO_CRYPT_DEBUG_GET_KEY == cmd_in)
	{
		ali_ce_umemcpy(param_in, &kdebug_param, sizeof(kdebug_param));
	}
	
	if(IO_CRYPT_FOUND_FREE_POS == cmd_in)
	{
		ali_ce_umemcpy(param_in, &kpos_param, sizeof(kpos_param));
	}
	
	if(IO_CRYPT_POS_IS_OCCUPY == cmd_in)
	{
		ali_ce_umemcpy(param_in, &ksts_param, sizeof(ksts_param));
	}

	return ret;
#endif

}


RET_CODE ce_generate_cw_key(const UINT8 *in_cw_data,UINT8 mode,UINT8 first_pos,UINT8 second_pos)
{
#if !defined(CONFIG_ALI_RPCNG)
     UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_STATIC_STRU(0, 16),
      1, DESC_P_PARA(0, 0, 0),
      //desc of pointer ret
      0,                          
      0,
    }; 
    jump_to_func(NULL, ali_rpc_call, in_cw_data, CE_NPARA(4)|FUNC_CE_GENERATE_CW_KEY, desc);
#else
	Data_param_rpc data_param;
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	
	ali_ce_umemcpy(&data_param.crypt_data[0], in_cw_data, sizeof(data_param.crypt_data));
	MINIRPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_MINIRPC_Data_param_rpc, sizeof(Data_param_rpc), &data_param);
	MINIRPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_MINIRPC_UCHAR, sizeof(__u8), &mode);
	MINIRPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_MINIRPC_UCHAR, sizeof(__u8), &first_pos);
	MINIRPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_MINIRPC_UCHAR, sizeof(__u8), &second_pos);

	return RpcCallCompletion(RPC_ce_generate_cw_key,&p1,&p2,&p3,&p4,NULL);
#endif

}

RET_CODE ce_generate_single_level_key(CE_DEVICE *pCeDev,CE_DATA_INFO *pCe_data_info)
{	
#if !defined(CONFIG_ALI_RPCNG)
    UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_STATIC_STRU(0, sizeof(CE_DATA_INFO)),
      1, DESC_P_PARA(0, 1, 0),
      //desc of pointer ret
      0,                          
      0,
    };
    jump_to_func(NULL, ali_rpc_call, pCeDev, CE_NPARA(2)|FUNC_CE_GENERATE_SINGLE_LEVEL_KEY, desc);
#else
	Param dev;
	Param ce_info;

	MINIRPC_PARAM_UPDATE(dev, PARAM_IN, PARAM_MINIRPC_UINT32, sizeof(__u32), &pCeDev);
	MINIRPC_PARAM_UPDATE(ce_info, PARAM_IN, PARAM_MINIRPC_Ce_data_info_rpc, sizeof(Ce_data_info_rpc), pCe_data_info);

	return RpcCallCompletion(RPC_ce_generate_single_level_key,&dev,&ce_info,NULL);
#endif
}

#endif  /*_HLD_CE_REMOTE*/

#endif  /*DUAL_ENABLE*/


/*add some common API for CE*/
/* This function is used to load key from otp to keyladder
key_pos:  OTP key pos
KEY_0_0,KEY_0_1,KEY_0_2,KEY_0_3
*/
int ce_load_otp_key(UINT32 key_pos)
{
	CE_DEVICE *pCeDev0 = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	OTP_PARAM opt_info;
	
	memset(&opt_info, 0, sizeof(OTP_PARAM));
	opt_info.otp_addr = OTP_ADDESS_1+4*key_pos;//AES_HW_M2M2_KEY_OTP_ADDR;
	opt_info.otp_key_pos = key_pos; //AES_HW_M2M2_KEY_POS;
	
	if (RET_SUCCESS != ce_key_load((UINT32)pCeDev0, (UINT32)(&opt_info)))
	{
		CE_REMOTE_API_PRINTF("load OTP key failed!");
		return -1;	
	}
	return 0;
}
/* This function is used to generate key use keyladder
*key:          input key data
first_key_pos: the root key for generated
second_key_pos:the target key position
ce_crypt_select: select the ce is decrypted or encrypted
*/
int ce_generate_key_by_aes(const UINT8 *key, UINT32 first_key_pos, UINT32 second_key_pos, UINT32 ce_crypt_select)
{
	CE_DEVICE *pCeDev0 = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO Ce_data_info;

	memset(&Ce_data_info, 0, sizeof(CE_DATA_INFO));
	memcpy(Ce_data_info.data_info.crypt_data, key, AES_CE_KEY_LEN);	
	Ce_data_info.data_info.data_len 			= AES_CE_KEY_LEN;	
	Ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_AES; 	
	Ce_data_info.des_aes_info.crypt_mode 		= ce_crypt_select; 	
	Ce_data_info.des_aes_info.des_low_or_high	= 0;			
	Ce_data_info.key_info.first_key_pos 		= first_key_pos;
	Ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	Ce_data_info.key_info.second_key_pos 		= second_key_pos; 
	if (ce_generate_single_level_key((UINT32)pCeDev0, (UINT32)(&Ce_data_info)) != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("generate AES Key fail!\n");
		return -1;
	}
	return 0;
}

/* This function is used to generate key use multi keyladder
pAES_KEY_LADDER_BUF_PARAM:
u32 key_ladder;      how many key ladder want to use
u32 root_key_pos;    root key pos
u8 r[256];           random data for each key ladder
*key_pos:            the output key pos
*/
RET_CODE aes_generate_key_with_multi_keyladder(pAES_KEY_LADDER_BUF_PARAM pCeAesparam, UINT8 *key_pos)
{
    UINT32 i=0;
    static CE_DEVICE *pCeDev = NULL;
	pCeDev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    CE_FOUND_FREE_POS_PARAM key_pos_param;
    UINT32 f_key_pos=0,s_key_pos=0; 
    int ret=-1; 
    OTP_PARAM opt_info;

    if((pCeAesparam==NULL)||(pCeAesparam->root_key_pos>OTP_KEY_0_3)||(pCeAesparam->key_ladder>=8))
    {
        CE_REMOTE_API_PRINTF("input parameter error (0x%x,0x%x,0x%x)!!\n",pCeAesparam,pCeAesparam->root_key_pos,\
                            pCeAesparam->key_ladder);
        return -1;
    }
    if(!pCeDev)
    {
        CE_REMOTE_API_PRINTF("crypto engine not avaliable!!\n");
        return -2;
    }    
    f_key_pos = pCeAesparam->root_key_pos;
    s_key_pos = pCeAesparam->root_key_pos+4;

	//load OTP key
	ret = ce_load_otp_key(f_key_pos);
    if(ret){
        CE_REMOTE_API_PRINTF("Load OTP key fail!!\n");
        return -3;
    }    
    //first level key
    ret = ce_generate_key_by_aes(&(pCeAesparam->r[0]),f_key_pos,s_key_pos,CE_IS_DECRYPT);
    if(ret){
        CE_REMOTE_API_PRINTF("generate 1 level key fail!!\n");
        return -4;
    }
    f_key_pos = s_key_pos;    
    for(i=0;i<pCeAesparam->key_ladder-1;i++)
    {
    	memset(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
    	key_pos_param.ce_key_level = TWO_LEVEL+i;
        if(key_pos_param.ce_key_level>THREE_LEVEL)
            key_pos_param.ce_key_level=THREE_LEVEL;
    	key_pos_param.pos = INVALID_ALI_CE_KEY_POS;
	key_pos_param.root = pCeAesparam->root_key_pos;
    	ret = ce_ioctl(pCeDev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
    	if (ret)
    	{
    		CE_REMOTE_API_PRINTF("find free key pos fail!\n");
    		return -5;
    	}
        s_key_pos = key_pos_param.pos;
    	ret = ce_generate_key_by_aes(&pCeAesparam->r[AES_CE_KEY_LEN*(i+1)],f_key_pos,s_key_pos,CE_IS_DECRYPT);

        if(ret){
            CE_REMOTE_API_PRINTF("generate %d level key fail!!\n",(i+2));
        }
#ifdef CE_REMOTE_API_DEBUG
    	CE_DEBUG_KEY_INFO param;
    	param.len = 4 ; 
    	param.sel = CE_KEY_READ ;
    	ce_ioctl(pCeDev, IO_CRYPT_DEBUG_GET_KEY, &param);  
        CE_REMOTE_API_PRINTF("the %d times level key:\n", (i+2));
        CE_REMOTE_API_DUMP(param.buffer, 4);
        CE_REMOTE_API_PRINTF("\n");
#endif
        ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, f_key_pos);
    	if (ret)
    	{
    		CE_REMOTE_API_PRINTF("CE generate key by AES fail\n");
    		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, s_key_pos);
    		return -6;
    	}
        f_key_pos = s_key_pos;
    }
    *key_pos = s_key_pos;
    CE_REMOTE_API_PRINTF("Key pos is 0x%x\n",*key_pos);
    return RET_SUCCESS;
}


 /* 
 	AES 128bit decrypt 3 level (3x16Bytes) data to key ladder's secure SRAM
	input -> data to be decrypt
	root_pos -> root pos in key ladder(OTP_KEY_0_0 OTP_KEY_0_1 OTP_KEY_0_2 OTP_KEY_0_3)
	key_pos -> level_three key_pos in key ladder, will return to caller for other use
	if this key_pos won't be used anymore, need to set it idle: 
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 */
RET_CODE aes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT8 *key_pos)
{
	RET_CODE ret;
	CE_DEVICE *pCeDev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO Ce_data_info;
	CE_FOUND_FREE_POS_PARAM key_pos_param;

	//generate Level 1 key with decrypt raw data 1
	ret = ce_generate_key_by_aes(input,root_pos,KEY_1_0+root_pos,CE_IS_DECRYPT);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: level one failed!\n");
		return RET_FAILURE;
	}

	//generate Level 2 key with decrypt raw data 2
	ret = ce_generate_key_by_aes(&input[16],root_pos+KEY_1_0,(KEY_1_0+root_pos)*2,CE_IS_DECRYPT);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: level two failed!\n");
		return RET_FAILURE;
	}


	//find a free level 3 key_pos
	memset(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = THREE_LEVEL;
	key_pos_param.pos = INVALID_CE_KEY_POS;
	key_pos_param.root = root_pos;
	ret = ce_ioctl(pCeDev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: find free key pos fail!\n");
		return RET_FAILURE;
	}
	
	//generate Level 3 key with decrypt raw data 3
	ret = ce_generate_key_by_aes(&input[32],(KEY_1_0+root_pos)*2,key_pos_param.pos,CE_IS_DECRYPT);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: level three failed!\n");
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return RET_FAILURE;
	}

	*key_pos = key_pos_param.pos;

    CE_REMOTE_API_PRINTF("key_pos is %d\n",*key_pos);
	return RET_SUCCESS;
}


 /* 
 	TDES 128bit decrypt eck to key ladder's secure SRAM  -> to get ck
	eck -> data to be decrypt
	level_one -> key in CE
	level_gen -> pos of ck in CE
 */
RET_CODE tdes_decrypt_key_to_ce_one_level(UINT8 *eck, UINT8 level_one, enum CE_CRYPT_TARGET level_gen)
{
	CE_DATA_INFO Ce_data_info;

    CE_DEVICE *pCeDev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	
	memset(&Ce_data_info, 0, sizeof(CE_DATA_INFO));
	Ce_data_info.data_info.data_len             = 8;			/* aes is 16 bytes des/tdes is 8 bytes*/
	Ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_DES; 	/* select AES or DES module*/
	Ce_data_info.des_aes_info.crypt_mode 		= CE_IS_DECRYPT; 	
	Ce_data_info.des_aes_info.des_low_or_high 	= 1;	            //HIGH_ADDR, then LOW_ADDR;  /* for AES it should be LOW_ADDR */
	Ce_data_info.key_info.first_key_pos         = level_one ;
	Ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	Ce_data_info.key_info.second_key_pos 		= level_gen;
	memcpy(Ce_data_info.data_info.crypt_data,&eck[0],8);
	
	if(RET_SUCCESS != ce_generate_single_level_key(pCeDev, &Ce_data_info))
	{
		CE_REMOTE_API_PRINTF("ce_generate_single_level_key failed high\n");
		return RET_FAILURE;
	}
    
	Ce_data_info.des_aes_info.des_low_or_high 	= 0;	//LOW_ADDR				
	memcpy(Ce_data_info.data_info.crypt_data,&eck[8],8);
	if(RET_SUCCESS != ce_generate_single_level_key(pCeDev, &Ce_data_info))
	{
		CE_REMOTE_API_PRINTF("ce_generate_single_level_key failed low\n");
		return RET_FAILURE;
	}

	return RET_SUCCESS;
}

RET_CODE tdes_decrypt_key_to_ce_64bit(UINT8 *eck, UINT8 level_one, enum CE_CRYPT_TARGET 
level_gen, UINT8 addr)
{
	CE_DATA_INFO Ce_data_info;

    CE_DEVICE *pCeDev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	
	MEMSET(&Ce_data_info, 0, sizeof(CE_DATA_INFO));
	Ce_data_info.data_info.data_len             = 8;				/* aes is 16 bytes des/tdes is 8 bytes */
	Ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_DES; 	/* select AES or DES module */
	Ce_data_info.des_aes_info.crypt_mode 		= CE_IS_DECRYPT; 	
	Ce_data_info.des_aes_info.des_low_or_high 	= addr;	            //HIGH_ADDR, then LOW_ADDR;  /* for AES it should be LOW_ADDR */
	Ce_data_info.key_info.first_key_pos         = level_one ;
	Ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	Ce_data_info.key_info.second_key_pos 		= level_gen;
	MEMCPY(Ce_data_info.data_info.crypt_data,&eck[0],8);
	
	if(RET_SUCCESS != ce_generate_single_level_key(pCeDev, &Ce_data_info))
	{
		return RET_FAILURE;
	}
	
	return RET_SUCCESS;
}

 /* 
 	TDES 128bit decrypt 2 level data to key ladder's secure SRAM 
	input -> data to be decrypt
	level_root -> root pos in CE(OTP_KEY_0_0 OTP_KEY_0_1 OTP_KEY_0_2 OTP_KEY_0_3)
	key_pos -> level_two key_pos in CE, will return to caller for other use
	after used the key_pos, need to set idle: ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 */
RET_CODE tdes_decrypt_to_ce_two_level(UINT8 *input,UINT8 level_root,UINT8 *key_pos)
{
	RET_CODE ret;
	CE_DEVICE *pCeDev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO Ce_data_info;
	CE_FOUND_FREE_POS_PARAM key_pos_param;

	//generate Level 1 key with decrypt raw data 1
	ret = tdes_decrypt_key_to_ce_one_level(input,level_root,KEY_1_0+level_root);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: level one failed!\n");
		return RET_FAILURE;
	}

	//find level two pos
	memset(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = TWO_LEVEL;
	key_pos_param.pos = INVALID_CE_KEY_POS;
	key_pos_param.root = level_root;
	ret = ce_ioctl(pCeDev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: find free key pos fail!\n");
		return RET_FAILURE;
	}
	
	//generate Level 2 key with decrypt raw data 2
	ret = tdes_decrypt_key_to_ce_one_level(&input[16],KEY_1_0+level_root,key_pos_param.pos);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: level two failed!\n");
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return RET_FAILURE;
	}

	*key_pos = key_pos_param.pos;

    CE_REMOTE_API_PRINTF("key_pos is %d\n",*key_pos);
	return RET_SUCCESS;
}


 /* 
 	TDES 128bit decrypt 3 level (3x16Bytes) data to key ladder's secure SRAM
	input -> data to be decrypt
	root_pos -> root pos in key ladder(OTP_KEY_0_0 OTP_KEY_0_1 OTP_KEY_0_2 OTP_KEY_0_3)
	key_pos -> level_three key_pos in key ladder, will return to caller for other use
	if this key_pos won't be used anymore, need to set it idle: 
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 */
RET_CODE tdes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT8 *key_pos)
{
	RET_CODE ret;
	CE_DEVICE *pCeDev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO Ce_data_info;
	CE_FOUND_FREE_POS_PARAM key_pos_param;

	//generate Level 1 key with decrypt raw data 1
	ret = tdes_decrypt_key_to_ce_one_level(input,root_pos,KEY_1_0+root_pos);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: level one failed!\n");
		return RET_FAILURE;
	}

	//generate Level 2 key with decrypt raw data 2
	ret = tdes_decrypt_key_to_ce_one_level(&input[16],root_pos+KEY_1_0,(KEY_1_0+root_pos)*2);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: level two failed!\n");
		return RET_FAILURE;
	}


	//find a free level 3 key_pos
	memset(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = THREE_LEVEL;
	key_pos_param.pos = INVALID_CE_KEY_POS;
	key_pos_param.root = root_pos;
	ret = ce_ioctl(pCeDev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: find free key pos fail!\n");
		return RET_FAILURE;
	}
	
	//generate Level 3 key with decrypt raw data 3
	ret = tdes_decrypt_key_to_ce_one_level(&input[32],(KEY_1_0+root_pos)*2,key_pos_param.pos);
	if (ret != RET_SUCCESS)
	{
		CE_REMOTE_API_PRINTF("Error: level three failed!\n");
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return RET_FAILURE;
	}

	*key_pos = key_pos_param.pos;

    CE_REMOTE_API_PRINTF("key_pos is %d\n",*key_pos);
	return RET_SUCCESS;
}
