#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <ali_cache.h>

#include "ali_ce_lld.h"

#if defined(CONFIG_ALI_RPCNG)
#include <ali_rpcng.h>
#endif

int ali_ce_umemcpy(void *dest, const void *src, __u32 n)
{
	int ret = 0;
	int sflag = access_ok(VERIFY_READ, (void __user *)src, n);
	int dflag = access_ok(VERIFY_WRITE, (void __user *)dest, n);

	if(segment_eq(get_fs(), USER_DS))
	{
		if(sflag && !dflag)
		{
			ret = copy_from_user(dest, (void __user *)src, n);
		}
		else	if(dflag && !sflag)
		{
			ret = copy_to_user(dest, src, n);
		}
		else if(!sflag && !dflag)
		{
			memcpy(dest, src, n);
		}
		else
		{
			return -1; 
		}
	}
	else
	{
		memcpy(dest, src, n);
	}
	return ret;
}


#if !defined(CONFIG_ALI_RPCNG)
#define CE_NPARA(x) ((HLD_CRYPTO_MODULE<<24)|(x<<16))

static __u32 ce_io_control[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, 0),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};
#endif

int ali_ce_ioctl(CE_DEVICE *pCeDev,__u32 cmd,__u32 param)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 i;
	__u32 common_desc[sizeof(ce_io_control)];
	__u32 *desc = (__u32 *)common_desc;
	__u32 *b = (__u32 *)ce_io_control;

	for(i = 0; i < sizeof(ce_io_control)/sizeof(__u32); i++)
	{
		desc[i] = b[i];
	}
	
	switch (CE_IO_CMD(cmd))
	{
		case CE_IO_CMD(IO_OTP_ROOT_KEY_GET):
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(OTP_PARAM));
			break;

		case CE_IO_CMD(IO_CRYPT_DEBUG_GET_KEY):
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(CE_DEBUG_KEY_INFO)); 
			break;
			
		case CE_IO_CMD(IO_CRYPT_POS_IS_OCCUPY):
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(CE_POS_STS_PARAM));  
			break;
			
		case CE_IO_CMD(IO_CRYPT_POS_SET_USED):
			desc=NULL;
			break;
			
		case CE_IO_CMD(IO_CRYPT_POS_SET_IDLE):
			desc=NULL;
			break;
			
		case CE_IO_CMD(IO_CRYPT_FOUND_FREE_POS):
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, \
										sizeof(CE_FOUND_FREE_POS_PARAM));
			break;
			
		case CE_IO_CMD(IO_DECRYPT_PVR_USER_KEY):
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CE_PVR_KEY_PARAM));
			break;
			
		default:
		{
			ALI_CE_RPC_DEBUG("CE rpc Error:invalid cmd\n");
			break;            
		}
	}
	jump_to_func(NULL, ali_rpc_call, pCeDev, CE_NPARA(3)|FUNC_CE_IOCTL, desc);
    
#else
	RET_CODE ret = RET_FAILURE;
	union
	{
		OTP_PARAM otp;
		CE_PVR_KEY_PARAM ce_pvr_key;
		CE_FOUND_FREE_POS_PARAM kpos_param;
		CE_DEBUG_KEY_INFO kdebug_param;
		CE_POS_STS_PARAM ksts_param;
	}tmp;
	
	Param p1;
	Param p2;
	Param p3;
	
	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(long), (void *)&pCeDev);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(long), (void *)&cmd);
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(long), (void *)&param);

	switch (CE_IO_CMD(cmd))
	{
		case CE_IO_CMD(IO_OTP_ROOT_KEY_GET):
			ali_ce_umemcpy(&tmp, (void *)param, sizeof(tmp.otp));
			RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Otp_param_rpc, \
								sizeof(Otp_param_rpc), (void *)&tmp);          
			break;

		case CE_IO_CMD(IO_CRYPT_DEBUG_GET_KEY):
			ali_ce_umemcpy(&tmp, (void *)param, sizeof(tmp.kdebug_param));
			RPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_Ce_debug_key_info_rpc, \
								sizeof(Ce_debug_key_info_rpc), (void *)&tmp);
			break;
			
		case CE_IO_CMD(IO_CRYPT_POS_IS_OCCUPY):
			ali_ce_umemcpy(&tmp, (void *)param, sizeof(tmp.ksts_param));
			RPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_Ce_pos_status_param_rpc, \
								sizeof(Ce_pos_status_param_rpc), (void *)&tmp);
			break;
			
		case CE_IO_CMD(IO_CRYPT_POS_SET_USED):
			break;
		
		case CE_IO_CMD(IO_CRYPT_POS_SET_IDLE):         
			break;
		
		case CE_IO_CMD(IO_CRYPT_FOUND_FREE_POS):
			RPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_Ce_found_free_pos_param_rpc, \
								sizeof(Ce_found_free_pos_param_rpc), (void *)param);            
			break;
			
		case CE_IO_CMD(IO_DECRYPT_PVR_USER_KEY):
			ali_ce_umemcpy(&tmp, (void *)param, sizeof(tmp.ce_pvr_key));
			RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Ce_pvr_key_param_rpc, 
								sizeof(Ce_pvr_key_param_rpc), (void *)&tmp);
			break;
		
		default:
		{
			ALI_CE_RPC_DEBUG("CE rpc Error: invalid cmd=0x%x\n",cmd);
			break;            
		}
	}

	ret =  RpcCallCompletion(RPC_ce_ioctl,&p1,&p2,&p3,NULL);

	if(CE_IO_CMD(IO_CRYPT_DEBUG_GET_KEY) == CE_IO_CMD(cmd))
	{
		ali_ce_umemcpy((void *)param, &tmp, sizeof(tmp.kdebug_param));
	}
	
	if(CE_IO_CMD(IO_CRYPT_POS_IS_OCCUPY) == CE_IO_CMD(cmd))
	{
		ali_ce_umemcpy((void *)param, &tmp, sizeof(tmp.ksts_param));
	}

	return ret;
#endif
}


int ali_ce_key_generate(pCE_DEVICE pCeDev, pCE_DATA_INFO pCe_data_info)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 desc[] = 
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
	CE_DATA_INFO ce_data;

	ali_ce_umemcpy(&ce_data, pCe_data_info, sizeof(ce_data));
	
	RPC_PARAM_UPDATE(dev, PARAM_IN, PARAM_UINT32, sizeof(__u32), &pCeDev);
	RPC_PARAM_UPDATE(data_info, PARAM_IN, PARAM_Ce_data_info_rpc, sizeof(Ce_data_info_rpc), &ce_data);

	return RpcCallCompletion(RPC_ce_key_generate,&dev,&data_info,NULL);
#endif
}

int ali_ce_key_load(pCE_DEVICE pCeDev, pOTP_PARAM pCe_opt_info)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 desc[] = 
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
	OTP_PARAM otp_data;
	
	ali_ce_umemcpy(&otp_data, pCe_opt_info, sizeof(otp_data));
	
	RPC_PARAM_UPDATE(dev, PARAM_IN, PARAM_UINT32, sizeof(__u32), &pCeDev);
	RPC_PARAM_UPDATE(otp_info, PARAM_IN, PARAM_Otp_param_rpc, sizeof(Otp_param_rpc), &otp_data);

	return RpcCallCompletion(RPC_ce_key_load,&dev,&otp_info,NULL);
#endif
}


int ali_ce_generate_cw_key(__u8 *in_cw_data,__u8 mode,
						__u8 first_pos,__u8 second_pos)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 desc[] = 
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
	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_Data_param_rpc, sizeof(Data_param_rpc), &data_param);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(__u8), &mode);
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UCHAR, sizeof(__u8), &first_pos);
	RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UCHAR, sizeof(__u8), &second_pos);

	return RpcCallCompletion(RPC_ce_generate_cw_key,&p1,&p2,&p3,&p4,NULL);
#endif
}

int ali_ce_generate_single_level_key(pCE_DEVICE pCeDev, pCE_DATA_INFO pCe_data_info)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 desc[] = 
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
	CE_DATA_INFO ce_data;

	ali_ce_umemcpy(&ce_data, pCe_data_info, sizeof(ce_data));
	
	RPC_PARAM_UPDATE(dev, PARAM_IN, PARAM_UINT32, sizeof(__u32), &pCeDev);
	RPC_PARAM_UPDATE(ce_info, PARAM_IN, PARAM_Ce_data_info_rpc, sizeof(Ce_data_info_rpc), &ce_data);

	return RpcCallCompletion(RPC_ce_generate_single_level_key,&dev,&ce_info,NULL);
#endif
}

void ali_m36_ce_see_init(void)
{
	pCE_DEVICE pCeDev = ( pCE_DEVICE ) hld_dev_get_by_type ( NULL, HLD_DEV_TYPE_CE );
	if(NULL == pCeDev)
	{
		ALI_CE_RPC_DEBUG("\nali_m36_ce_see_init jump to see ce_api_attach\n\n");
#if !defined(CONFIG_ALI_RPCNG)
		jump_to_func(NULL, ali_rpc_call, NULL, CE_NPARA(0) | FUNC_CE_ATTACH, NULL);
#else
		RpcCallCompletion(RPC_ce_api_attach,NULL);
#endif
	}
}

void ali_m36_ce_see_uninit(void)
{
	pCE_DEVICE pCeDev = ( pCE_DEVICE ) hld_dev_get_by_type ( NULL, HLD_DEV_TYPE_CE );
	if(NULL != pCeDev)
	{
		ALI_CE_RPC_DEBUG("\nali_m36_ce_see_uninit jump to see ce_api_detach\n\n");
#if !defined(CONFIG_ALI_RPCNG)
		jump_to_func(NULL, ali_rpc_call, NULL, CE_NPARA(0) | FUNC_CE_DETACH, NULL);
#else
		RpcCallCompletion(RPC_ce_api_detach,NULL);
#endif
	}
}


/*add some common API for CE*/
/* This function is used to load key from otp to keyladder
key_pos:  OTP key pos
KEY_0_0,KEY_0_1,KEY_0_2,KEY_0_3
*/
int ali_ce_load_otp_key(__u32 key_pos)
{
	pCE_DEVICE pCeDev0 = (pCE_DEVICE)hld_dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	OTP_PARAM opt_info;
	
	memset(&opt_info, 0x00, sizeof(OTP_PARAM));
	opt_info.otp_addr = OTP_ADDESS_1+4*key_pos;//AES_HW_M2M2_KEY_OTP_ADDR;
	opt_info.otp_key_pos = key_pos; //AES_HW_M2M2_KEY_POS;
	
	if (RET_SUCCESS != ali_ce_key_load(pCeDev0, (&opt_info)))
	{
		ALI_CE_RPC_DEBUG("load OTP key failed!");
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
int ali_ce_generate_key_by_aes(__u8 *key,
									__u32 first_key_pos,
									__u32 second_key_pos,
									__u32 ce_crypt_select)
{
	pCE_DEVICE pCeDev0 = (pCE_DEVICE)hld_dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO Ce_data_info;

	memset(&Ce_data_info, 0, sizeof(CE_DATA_INFO));
	memcpy(Ce_data_info.data_info.crypt_data, key, AES_CE_KEY_LEN);	
	Ce_data_info.data_info.data_len 			= AES_CE_KEY_LEN;	
	Ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_AES ; 	
	Ce_data_info.des_aes_info.crypt_mode 		= ce_crypt_select; 	
	Ce_data_info.des_aes_info.des_low_or_high	= 0;			
	Ce_data_info.key_info.first_key_pos 		= first_key_pos ;
	Ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	Ce_data_info.key_info.second_key_pos 		= second_key_pos; 
	if (ali_ce_generate_single_level_key(pCeDev0, &Ce_data_info) != RET_SUCCESS)
	{
		ALI_CE_RPC_DEBUG("generate AES Key fail!\n");
		return -1;
	}
	return 0;
}

/* This function is used to generate key use multi keyladder
pAES_KEY_LADDER_BUF_PARAM:
__u32 key_ladder;      how many key ladder want to use
__u32 root_key_pos;    root key pos
__u8 r[256];           random data for each key ladder
*key_pos:            the output key pos
*/
int ali_aes_generate_key_with_multi_keyladder
(
	AES_KEY_LADDER_BUF_PARAM *pCeAesparam, __u8 *key_pos
)
{
	__u32 i=0;
	pCE_DEVICE pCeDev = NULL;
	CE_FOUND_FREE_POS_PARAM key_pos_param;
	__u32 f_key_pos=0,s_key_pos=0; 
	int ret=0; 

	pCeDev = (pCE_DEVICE)hld_dev_get_by_id(HLD_DEV_TYPE_CE, 0);

	if((pCeAesparam==NULL) \
		||(pCeAesparam->root_key_pos>OTP_KEY_0_3) \
		||(pCeAesparam->key_ladder>=8))
	{
	    ALI_CE_RPC_DEBUG("input parameter error (0x%x,0x%x,0x%x)!!\n", \
						(__u32)pCeAesparam, pCeAesparam->root_key_pos, \
							pCeAesparam->key_ladder);
	    return -1;
	}
	if(!pCeDev)
	{
	    ALI_CE_RPC_DEBUG("crypto engine not avaliable!!\n");
	    return -1;
	}    
	f_key_pos = pCeAesparam->root_key_pos;
	s_key_pos = pCeAesparam->root_key_pos+4;

	//load OTP key
	ret = ali_ce_load_otp_key(f_key_pos);
	if(ret){
	    ALI_CE_RPC_DEBUG("Load OTP key fail!!\n");
	    return -1;
	}    
	//first level key
	ret = ali_ce_generate_key_by_aes(&(pCeAesparam->r[0]), \
									f_key_pos,s_key_pos, \
									CE_IS_DECRYPT);
	if(ret){
	    ALI_CE_RPC_DEBUG("generate 1 level key fail!!\n");
	    return -1;
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
		ret = ali_ce_ioctl(pCeDev, IO_CRYPT_FOUND_FREE_POS, \
							(__u32)&key_pos_param);
		if (ret)
		{
			ALI_CE_RPC_DEBUG("find free key pos fail!\n");
			return INVALID_ALI_CE_KEY_POS;
		}
	    s_key_pos = key_pos_param.pos;
		ret=ali_ce_generate_key_by_aes(&pCeAesparam->r[AES_CE_KEY_LEN*(i+1)], \
											f_key_pos, s_key_pos, \
											CE_IS_DECRYPT);
	    if(ret){
	        ALI_CE_RPC_DEBUG("generate %d level key fail!!\n",(i+2));
	    }


	    ali_ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, f_key_pos);
		if (ret)
		{
			ALI_CE_RPC_DEBUG("CE generate key by AES fail\n");
			ali_ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, s_key_pos);
			return INVALID_ALI_CE_KEY_POS;
		}
	    f_key_pos = s_key_pos;
	}
	*key_pos = s_key_pos;
	ALI_CE_RPC_DEBUG("Key pos is 0x%x\n",*key_pos);
	return RET_SUCCESS;
}

/* This function is used to generate 2 level key by OTP_ADDESS_2 & R1 & R2 use keyladder
*pos:          output key pos of 2 level key
_r1:        the radom data R1 
_r2:        the radom data R2 
*/
int ali_decrypt_2_level_key(__u8 * pos,__u8 * _r1,
							__u8* _r2)
{
	int ret;
	CE_FOUND_FREE_POS_PARAM key_pos_param;
	pCE_DEVICE pCeDev1 ;

	pCeDev1= (pCE_DEVICE)hld_dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	if(!pCeDev1)
	{
	    ALI_CE_RPC_DEBUG("crypto engine not avaliable!!\n");
	    return -1;
	}    
	// step1 : generate level 1 key
	ali_ce_load_otp_key(OTP_ADDESS_2);
	ret = ali_ce_generate_key_by_aes(_r1, KEY_0_1, CRYPT_KEY_1_1,CE_IS_DECRYPT);
	if(ret <0 ){
	     ALI_CE_RPC_DEBUG("ce decrypt level 1 key failed\n");
	     return ret ;
	}
	// step 2: generate level 2 key

	memset(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = TWO_LEVEL;
	key_pos_param.pos = 0xff;
	key_pos_param.root = KEY_0_1;
	// find free pos // CE find a free pos for level 2 key generate
	ret = ali_ce_ioctl(pCeDev1, IO_CRYPT_FOUND_FREE_POS, \
						(__u32)&key_pos_param); 
	if (ret != RET_SUCCESS )
	{
		ALI_CE_RPC_DEBUG("Error: find free key pos fail!\n");
		return ret;
	} 
	ret=ali_ce_generate_key_by_aes(_r2,KEY_1_1,key_pos_param.pos,CE_IS_DECRYPT);
	if(ret <0 )
	{
	    ALI_CE_RPC_DEBUG("Error: ce decrypt 2 level key failed!\n");    
	    return ret ;
	}
	*pos =  key_pos_param.pos;     
	return 0 ;        
}

