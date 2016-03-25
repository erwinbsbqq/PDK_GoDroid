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
#include <linux/types.h>
#include <asm/uaccess.h>
#include <ali_cache.h>

#include "ali_dsc_lld.h"

#if defined(CONFIG_ALI_RPCNG)
#include <ali_rpcng.h>
#endif

static void dsc_deal_cache_before_dma(__u8 *in, __u8 *out, __u32 data_length)
{
	__CACHE_FLUSH_ALI(CACHE_A_ALIGN(in),  CACHE_L_ALIGN(data_length));

	if ((__u32)in != (__u32)out)
	{
		__CACHE_CLEAN_ALI(CACHE_A_ALIGN(out),  CACHE_L_ALIGN(data_length));
	}
}

static void dsc_deal_cache_after_dma(__u8 *out, UINT32 data_length)
{
	__CACHE_INV_ALI(CACHE_A_ALIGN(out),  CACHE_L_ALIGN(data_length));
}


int ali_dsc_umemcpy(void *dest, const void *src, __u32 n)
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

void ali_dsc_record_handle_id(struct file *filp, __u32 handle, __u32 cmd)
{
	struct ali_m36_dsc_dev *dev = (struct ali_m36_dsc_dev *)g_ali_dsc_devices;
	__u32 i = 0;
	__u32 handle_exist = 0;

	if(handle >= ALI_DSC_LLD_MAX_ITEM)
	{
		ALI_DSC_DEBUG_ERR("handle out of bound, handle:%d\n", handle);
		return;
	}

	for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
	{
		if((dev->rec[i].handle_id.handle == (__u32)handle)\
			&& dev->rec[i].handle_id.filp)
		{
			dev->rec[i].handle_id.filp = (__u32)filp;
			handle_exist = 1;
			break;
		}
	}
	
	if(ALI_DSC_RESOURCE_BUSY == cmd)
	{
		if(handle_exist)
		{
			return;
		}
		
		for(i=0;i<ALI_DSC_LLD_MAX_ITEM;i++)
		{
			if(!dev->rec[i].handle_id.filp)
			{
				dev->rec[i].handle_id.filp = (__u32)filp;
				dev->rec[i].handle_id.handle = handle;
				break;
			}
		}
	}
	else if(ALI_DSC_RESOURCE_IDLE == cmd)
	{
		if(i<ALI_DSC_LLD_MAX_ITEM)	
		{
			dev->rec[i].handle_id.filp = 0;
		}
	}
}

#if !defined(CONFIG_ALI_RPCNG)
#define DSC_NPARA(x) ((HLD_DSC_MODULE << 24)|(x << 16))

__u32 des_io_control[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, 0),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

__u32 aes_io_control[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, 0),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

__u32 csa_io_control[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, 0),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

__u32 sha_io_control[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, 0),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

__u32 dsc_io_control[] = 
{ //desc of pointer para
	1, DESC_STATIC_STRU(0, 0),
	1, DESC_P_PARA(0, 2, 0), 
	//desc of pointer ret
	0,                          
	0,
};

__u32 dsc_io_control_1[] = 
{ //desc of pointer para
	9, DESC_OUTPUT_STRU(0, sizeof(KEY_PARAM)), DESC_STATIC_STRU(1, 0),\
		 DESC_STATIC_STRU(2, 4*sizeof(AES_KEY_PARAM)), \
		 DESC_STATIC_STRU(3, 4*sizeof(CSA_KEY_PARAM)), \
		 DESC_STATIC_STRU(4, 4*sizeof(DES_KEY_PARAM)), \
		 DESC_STATIC_STRU(5, 4*sizeof(AES_IV_INFO)),\
		 DESC_STATIC_STRU(6, 4*sizeof(DES_IV_INFO)), \
		 DESC_STATIC_STRU(7, 16), DESC_STATIC_STRU(8, 16), 
	9, DESC_P_PARA(0, 2, 0), DESC_P_STRU(1, 0, 1, offsetof(KEY_PARAM, pid_list)),\
		DESC_P_STRU(1, 0, 2, offsetof(KEY_PARAM, p_aes_key_info)), \
		DESC_P_STRU(1, 0, 3, offsetof(KEY_PARAM, p_csa_key_info)), \
		DESC_P_STRU(1, 0, 4, offsetof(KEY_PARAM, p_des_key_info)), \
		DESC_P_STRU(1, 0, 5, offsetof(KEY_PARAM, p_aes_iv_info)), \
		DESC_P_STRU(1, 0, 6, offsetof(KEY_PARAM, p_des_iv_info)), \
		DESC_P_STRU(1, 0, 7, offsetof(KEY_PARAM, init_vector)), \
		DESC_P_STRU(1, 0, 8, offsetof(KEY_PARAM, ctr_counter)),
	//desc of pointer ret
	0,                          
	0,
};

__u32 sha_crypt_control[] = 
{ //desc of pointer para
	1, DESC_OUTPUT_STRU(0, 64),
	1, DESC_P_PARA(0, 2, 0),
	//desc of pointer ret
	0,                          
	0,
};
#endif

int ali_des_decrypt_ex(pDES_DEV pDesDev,__u16 stream_id, 
						__u8 *input, __u8 *output, 
						__u32 total_length)
{
#if !defined(CONFIG_ALI_RPCNG)    
	jump_to_func(NULL, ali_rpc_call, pDesDev, \
				DSC_NPARA(5)|FUNC_DES_DECRYPT, \
				NULL);
#else
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pDesDev);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT16, sizeof(__u16), (void *)&stream_id);   
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&input);
	RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&output);
	RPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&total_length);   

	return RpcCallCompletion(RPC_des_decrypt,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

int ali_des_encrypt_ex(pDES_DEV pDesDev,__u16 stream_id, 
						__u8 *input, __u8 *output, 
						__u32 total_length)
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, pDesDev, DSC_NPARA(5)|FUNC_DES_ENCRYPT, NULL);
#else
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pDesDev);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT16, sizeof(__u16), (void *)&stream_id);   
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&input);
	RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&output);
	RPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_UINT32, sizeof(total_length), (void *)&total_length);   

	return RpcCallCompletion(RPC_des_encrypt,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

int ali_des_decrypt(pDES_DEV pDesDev,__u16 stream_id, 
					__u8 *input, __u8 *output, 
					__u32 total_length)
{
	int ret=RET_FAILURE;

	if((stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_before_dma(input,output,(total_length*TS_PACKET_SIZE));
	}
	else
	{ 
		dsc_deal_cache_before_dma(input,output,(total_length));
	}
	
	ret=ali_des_decrypt_ex(pDesDev,stream_id,input,output,total_length);
	
	if((stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_after_dma(output,(total_length*TS_PACKET_SIZE));
	}
	else
	{
		dsc_deal_cache_after_dma(output,(total_length));
	}
	
	return ret;
}

int ali_des_encrypt(pDES_DEV pDesDev,__u16 stream_id, 
					__u8 *input, __u8 *output,
					__u32 total_length)
{
	int ret = RET_FAILURE;
	if((stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{     
		dsc_deal_cache_before_dma(input,output,(total_length*TS_PACKET_SIZE));
	}
	else
	{ 
		dsc_deal_cache_before_dma(input,output,(total_length));
	}
	
	ret=ali_des_encrypt_ex(pDesDev,stream_id,input,output,total_length);
	if((stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_after_dma(output,(total_length*TS_PACKET_SIZE));
	}
	else
	{
		dsc_deal_cache_after_dma(output,(total_length));
	}
	return ret;
}


int ali_des_ioctl( DES_DEV *pDesDev ,__u32 cmd , __u32 param)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 i;
	__u32 common_desc[40];
	__u32 *desc = (__u32 *)common_desc;
	__u32 *b = (__u32 *)des_io_control;
	KEY_PARAM *param_tmp = (KEY_PARAM *)param;
	
	for(i = 0; i < sizeof(des_io_control)/sizeof(__u32); i++)
	{
		desc[i] = b[i];
	}
	
	switch(DSC_IO_CMD(cmd)) 
	{
		case DSC_IO_CMD(IO_INIT_CMD):
		{
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DES_INIT_PARAM));
			break;
		}
		
		case DSC_IO_CMD(IO_CREAT_CRYPT_STREAM_CMD):
		case DSC_IO_CMD(IO_KEY_INFO_UPDATE_CMD):
		{
			b = (__u32 *)dsc_io_control_1;
			for(i = 0; i < sizeof(dsc_io_control_1)/sizeof(__u32); i++)
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
			DESC_STATIC_STRU_SET_SIZE(desc, 1, sizeof(__u16)*param_tmp->pid_len);
			break;		  
		}
		
		
		case DSC_IO_CMD(IO_DELETE_CRYPT_STREAM_CMD):
		{
			desc = NULL;
			break;
		}
		
		default:
			return RET_FAILURE;
	}
	
	jump_to_func(NULL, ali_rpc_call, pDesDev, DSC_NPARA(3)|FUNC_DES_IOCTL, desc);
#else
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	KEY_PARAM *key_param = (KEY_PARAM *)param;
	DSC_CLR_KEY *dsc_key = (DSC_CLR_KEY *)dsc->dsc_key;
	DES_INIT_PARAM des_param;
	int ret = -1;
	Param p1;
	Param p2;
	Param p3;
	
	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pDesDev);                     
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&cmd);         
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&param);
	
	switch(DSC_IO_CMD(cmd)) 
	{
		case DSC_IO_CMD(IO_INIT_CMD):
		{
			ali_dsc_umemcpy(&des_param, (void *)param, sizeof(DES_INIT_PARAM));
			RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Des_init_param_rpc, sizeof(Des_init_param_rpc), (void *)&des_param);
			break;
		}
		
		case DSC_IO_CMD(IO_CREAT_CRYPT_STREAM_CMD):
		case DSC_IO_CMD(IO_KEY_INFO_UPDATE_CMD):
		{
			if(!key_param->pid_len || key_param->pid_len > ALI_DSC_LLD_MAX_ITEM)
			{
				key_param->pid_len = 1;
			}
			
			if(key_param->pid_list)
			{
				ali_dsc_umemcpy((dsc_key->pid_ptr),key_param->pid_list,sizeof(__u16)*(key_param->pid_len));
				key_param->pid_list = UC(dsc_key->pid_ptr);
			}
			
			if(key_param->p_des_key_info)
			{
				ali_dsc_umemcpy((dsc_key->key_ptr),key_param->p_des_key_info, \
								2*(key_param->pid_len)*key_param->key_length/8);
				key_param->p_des_key_info = UC(dsc_key->key_ptr);				
			}
			
			if(key_param->p_des_iv_info)
			{
				ali_dsc_umemcpy((dsc_key->iv_ptr),key_param->p_des_iv_info,2*(key_param->pid_len)*DES_BLOCK_LEN);
				key_param->p_des_iv_info = UC(dsc_key->iv_ptr);
			}
			
			if(key_param->init_vector)
			{
				ali_dsc_umemcpy((dsc_key->iv_ptr),key_param->init_vector,DES_BLOCK_LEN);
				key_param->init_vector = UC(dsc_key->iv_ptr);
			}
			
			RPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_Key_param_rpc, sizeof(Key_param_rpc), (void *)key_param);      
			break;
		}		

		case DSC_IO_CMD(IO_DELETE_CRYPT_STREAM_CMD):
			break;

		default:
		    ALI_DSC_DEBUG_ERR("Dsc rpc error: %d, invalid cmd=%d\n", __LINE__, cmd);
		    break;            
	}
	
	ret = RpcCallCompletion(RPC_des_ioctl,&p1,&p2,&p3,NULL);
	
	return ret;
#endif
}


int ali_sha_ioctl( SHA_DEV *pShaDev ,__u32 cmd, __u32 param)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 i;
	__u32 common_desc[sizeof(sha_io_control)];
	__u32 *desc = (__u32 *)common_desc;
	__u32 *b = (__u32 *)sha_io_control;

	for(i = 0; i < sizeof(sha_io_control)/sizeof(__u32); i++)
	{
		desc[i] = b[i];
	}

	switch(DSC_IO_CMD(cmd))
	{
		case DSC_IO_CMD(IO_INIT_CMD):
		{
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(SHA_INIT_PARAM));
			break;
		}

		default:
		{
			return RET_FAILURE;
		}
	}

	jump_to_func(NULL, ali_rpc_call, pShaDev, \
				DSC_NPARA(3)|FUNC_SHA_IOCTL, \
				desc);
#else
	Param p1;
	Param p2;
	Param p3;
	SHA_INIT_PARAM sha_param;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pShaDev);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&cmd);
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&param);

	switch(DSC_IO_CMD(cmd))
	{
		case DSC_IO_CMD(IO_INIT_CMD):
		{
			ali_dsc_umemcpy(&sha_param, (void *)param, sizeof(sha_param));
			RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Sha_init_param_rpc, sizeof(Sha_init_param_rpc), (void *)&sha_param);
			break;
		}
		
		default:
		{
			ALI_DSC_DEBUG_ERR("sha rpc error: invalid cmd %d\n", cmd);
			return RET_FAILURE;
		}
	}
	return RpcCallCompletion(RPC_sha_ioctl,&p1,&p2,&p3,NULL);
#endif    
				
}

int ali_sha_digest_ex(SHA_DEV *pShaDev, __u8 *input, 
						__u8 *output,__u32 data_length)
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, pShaDev, \
				DSC_NPARA(4)|FUNC_SHA_DIGEST, \
				sha_crypt_control);
#else
	int ret = -1;
	Sha_hash_rpc hash_out;
	Param p1;
	Param p2;
	Param p3;
	Param p4;

	memset((void *)&hash_out, 0x00, sizeof(hash_out));

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pShaDev);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&input);
	RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Sha_hash_rpc, sizeof(Sha_hash_rpc), (void *)&hash_out);
	RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&data_length);   

	ret = RpcCallCompletion(RPC_sha_digest,&p1,&p2,&p3,&p4,NULL);
	memcpy(output, hash_out.hash, sizeof(hash_out));
	return ret;
#endif
}

int ali_sha_digest(SHA_DEV *pShaDev, __u8 *input, 
					__u8 *output,__u32 data_length)
{
	int ret = RET_FAILURE;

	dsc_deal_cache_before_dma(input,input,(data_length));
	ret=ali_sha_digest_ex(pShaDev,input,output,data_length);   
	return ret;
}

int ali_aes_encrypt_ex(pAES_DEV pAesDev,__u16 stream_id,
						__u8 *input, __u8 *output,
						__u32 total_length)
{
 #if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, pAesDev, \
				DSC_NPARA(5)|FUNC_AES_ENCRYPT, \
				NULL);
#else
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pAesDev);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT16, sizeof(__u16), (void *)&stream_id);   
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&input);
	RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&output);
	RPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&total_length);
    
    return RpcCallCompletion(RPC_aes_encrypt,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

int ali_aes_decrypt_ex(pAES_DEV pAesDev, __u16 stream_id, 
						__u8 *input, __u8 *output, 
						__u32 total_length)
{
#if !defined(CONFIG_ALI_RPCNG)    
	jump_to_func(NULL, ali_rpc_call, pAesDev, \
				DSC_NPARA(5)|FUNC_AES_DECRYPT, \
				NULL);
#else
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pAesDev);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT16, sizeof(__u16), (void *)&stream_id);   
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&input);
	RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&output);
	RPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&total_length);   

	return RpcCallCompletion(RPC_aes_decrypt,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

int ali_aes_decrypt(pAES_DEV pAesDev, __u16 stream_id, 
					__u8 *input, __u8 *output,
					__u32 total_length)
{
	int ret = RET_FAILURE;
	
	if((stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_before_dma(input,output,(total_length*TS_PACKET_SIZE));
	}
	else
	{
		dsc_deal_cache_before_dma(input,output,(total_length));
	}
	
	ret = ali_aes_decrypt_ex(pAesDev,stream_id,input,output,total_length);

	if((stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_after_dma(output,(total_length*TS_PACKET_SIZE));
	}
	else
	{
		dsc_deal_cache_after_dma(output,(total_length));
	}

	return ret;
}

int ali_aes_encrypt(pAES_DEV pAesDev, __u16 stream_id,
					__u8 *input, __u8 *output,
					__u32 total_length)
{
	int ret = RET_FAILURE;
	
	if((stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_before_dma(input,output,(total_length*TS_PACKET_SIZE));
	}
	else
	{    
		dsc_deal_cache_before_dma(input,output,(total_length));
	}
	
	ret= ali_aes_encrypt_ex(pAesDev,stream_id,input,output,total_length);
	
	if((stream_id<=3) || (stream_id>=8 && stream_id<=0xf))
	{
		dsc_deal_cache_after_dma(output,(total_length*TS_PACKET_SIZE));
	}
	else
	{
		dsc_deal_cache_after_dma(output,(total_length));
	}
	return ret;
}

int ali_aes_ioctl
( 
AES_DEV *pAesDev ,
__u32 cmd ,
__u32 param
)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 i;
	__u32 common_desc[40];
	__u32 *desc = (__u32 *)common_desc;
	__u32 *b = (__u32 *)aes_io_control;
	KEY_PARAM *param_tmp = (KEY_PARAM *)param;
	
	for(i = 0; i < sizeof(aes_io_control)/sizeof(__u32); i++)
	{
		desc[i] = b[i];
	}
	
	switch(DSC_IO_CMD(cmd)) 
	{
		case DSC_IO_CMD(IO_INIT_CMD):
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(AES_INIT_PARAM));
			break;
			
		case DSC_IO_CMD(IO_CREAT_CRYPT_STREAM_CMD):
		case DSC_IO_CMD(IO_KEY_INFO_UPDATE_CMD):
		{
			b = (__u32 *)dsc_io_control_1;
			for(i = 0; i < sizeof(dsc_io_control_1)/sizeof(__u32); i++)
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
			DESC_STATIC_STRU_SET_SIZE(desc, 1, sizeof(__u16)*param_tmp->pid_len);
			break;
		}

		case DSC_IO_CMD(IO_DELETE_CRYPT_STREAM_CMD):
			desc = NULL;
			break;

		default:
			return RET_FAILURE;
	}
	
	jump_to_func(NULL, ali_rpc_call, pAesDev, DSC_NPARA(3)|FUNC_AES_IOCTL, desc);
#else
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	KEY_PARAM *key_param = (KEY_PARAM *)param;
	DSC_CLR_KEY *dsc_key = (DSC_CLR_KEY *)dsc->dsc_key;
	AES_INIT_PARAM aes_param;
	int ret = -1;
	Param p1;
	Param p2;
	Param p3;
	
	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pAesDev);                     
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&cmd);         
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&param);
		
	switch(DSC_IO_CMD(cmd))
	{
		case DSC_IO_CMD(IO_INIT_CMD):
			ali_dsc_umemcpy(&aes_param, (void *)param, sizeof(AES_INIT_PARAM));
			RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Aes_init_param_rpc, sizeof(Aes_init_param_rpc), (void *)&aes_param);
			break;

		case DSC_IO_CMD(IO_CREAT_CRYPT_STREAM_CMD) :
		case DSC_IO_CMD(IO_KEY_INFO_UPDATE_CMD):
		{
			if(!key_param->pid_len || key_param->pid_len > ALI_DSC_LLD_MAX_ITEM)
			{
				key_param->pid_len = 1;
			}
			
			if(key_param->pid_list)
			{
				ali_dsc_umemcpy((dsc_key->pid_ptr),key_param->pid_list,sizeof(__u16)*(key_param->pid_len));
				key_param->pid_list = UC(dsc_key->pid_ptr);
			}
			
			if(key_param->p_aes_key_info)
			{
				ali_dsc_umemcpy((dsc_key->key_ptr),key_param->p_aes_key_info, \
								2*(key_param->pid_len)*key_param->key_length/8);
				key_param->p_aes_key_info = UC(dsc_key->key_ptr);				
			}
			
			if(key_param->p_aes_iv_info)
			{
				ali_dsc_umemcpy((dsc_key->iv_ptr),key_param->p_aes_iv_info,2*(key_param->pid_len)*AES_BLOCK_LEN);
				key_param->p_aes_iv_info = UC(dsc_key->iv_ptr);
			}
			
			if(key_param->init_vector)
			{
				ali_dsc_umemcpy((dsc_key->iv_ptr),key_param->init_vector,AES_BLOCK_LEN);
				key_param->init_vector = UC(dsc_key->iv_ptr);
			}
			
			if(key_param->ctr_counter)
			{
				ali_dsc_umemcpy((dsc_key->ctr_ptr),key_param->ctr_counter,AES_BLOCK_LEN);
				key_param->ctr_counter = UC(dsc_key->ctr_ptr);
			}
			
			RPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_Key_param_rpc, sizeof(Key_param_rpc), (void *)key_param);      
			break;
		}
		
		case DSC_IO_CMD(IO_DELETE_CRYPT_STREAM_CMD):
			break;        

		default:
		    	ALI_DSC_DEBUG_ERR("Dsc rpc error: %d, invalid cmd=%d\n", __LINE__,cmd);
			break;
	}
	
	ret = RpcCallCompletion(RPC_aes_ioctl,&p1,&p2,&p3,NULL);
	
	return ret;
#endif
}


int ali_dsc_ioctl
( 
DSC_DEV *pDscDev ,
__u32 cmd , 
__u32 param
)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 i;
	__u32 common_desc[40];
	__u32 *desc = (__u32 *)common_desc;
	__u32 *b = (__u32 *)dsc_io_control;

	for(i = 0; i < sizeof(dsc_io_control)/sizeof(__u32); i++)
	{
		desc[i] = b[i];
	}

    	switch(DSC_IO_CMD(cmd))
	{
		case DSC_IO_CMD(IO_PARSE_DMX_ID_GET_CMD):
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(__u32));
			break;
		case DSC_IO_CMD(IO_PARSE_DMX_ID_SET_CMD):
			desc = NULL;
			break;
		case DSC_IO_CMD(IO_DSC_GET_DES_HANDLE):
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(__u32));
			break;
		case DSC_IO_CMD(IO_DSC_GET_AES_HANDLE):
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(__u32));
			break;
		case DSC_IO_CMD(IO_DSC_GET_CSA_HANDLE):
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(__u32));
			break;
		case DSC_IO_CMD(IO_DSC_GET_SHA_HANDLE):
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(__u32));
			break;
		case DSC_IO_CMD(IO_DSC_SET_PVR_KEY_PARAM):
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_PVR_KEY_PARAM));
			break;
		case DSC_IO_CMD(IO_DSC_SET_PVR_KEY_IDLE):
			desc = NULL;
			break;
		case DSC_IO_CMD(IO_DSC_SET_ENCRYPT_PRIORITY):                 
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_EN_PRIORITY));
			break;         
		case DSC_IO_CMD(IO_DSC_GET_DRIVER_VERSION):
			DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, 20);
			break;
		case DSC_IO_CMD(IO_DSC_SET_CLR_CMDQ_EN):
			desc = NULL;
			break;
		case DSC_IO_CMD(IO_DSC_DELETE_HANDLE_CMD):
			desc = NULL;
			break;	
		case DSC_IO_CMD(IO_DSC_FIXED_DECRYPTION):			   
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_FIXED_CRYPTION));
			break;				
		case DSC_IO_CMD(IO_DSC_SYS_UK_FW):			   
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_SYS_UK_FW));
			break;		
		case DSC_IO_CMD(IO_DSC_VER_CHECK):
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DSC_VER_CHK_PARAM));
			break;     			
		default:
			return RET_FAILURE;
	}

	jump_to_func(NULL, ali_rpc_call, pDscDev, \
				DSC_NPARA(3)|FUNC_DSC_IOCTL, \
				desc);
#else    
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	DSC_PVR_KEY_PARAM *pvr_key = (DSC_PVR_KEY_PARAM *)param;
	Param p1;
	Param p2;
	Param p3;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pDscDev);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&cmd); 
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&param);

	switch(DSC_IO_CMD(cmd))
	{
		case DSC_IO_CMD(IO_PARSE_DMX_ID_GET_CMD):
			RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(__u32), (void *)param);            
			break;
		case DSC_IO_CMD(IO_PARSE_DMX_ID_SET_CMD):
			break;
		case DSC_IO_CMD(IO_DSC_GET_DES_HANDLE):
			RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(__u32), (void *)param);            
			break;
		case DSC_IO_CMD(IO_DSC_GET_AES_HANDLE):
			RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(__u32), (void *)param);
			break;
		case DSC_IO_CMD(IO_DSC_GET_CSA_HANDLE):
			RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(__u32), (void *)param);            
			break;
		case DSC_IO_CMD(IO_DSC_GET_SHA_HANDLE):
			RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_UINT32, sizeof(__u32), (void *)param);
			break;
		case DSC_IO_CMD(IO_DSC_SET_PVR_KEY_PARAM):
			if(pvr_key->input_addr)
			{
				ali_dsc_umemcpy(dsc->dsc_key, (const void *)pvr_key->input_addr, \
								pvr_key->valid_key_num*pvr_key->pvr_key_length/8);
				pvr_key->input_addr = (__u32)UC(dsc->dsc_key);
			}
			else
			{
				return RET_FAILURE;
			}
			RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Dsc_pvr_key_param_rpc, \
							sizeof(Dsc_pvr_key_param_rpc), (void *)pvr_key);
			break;
		case DSC_IO_CMD(IO_DSC_SET_ENCRYPT_PRIORITY):
			RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Dsc_deen_parity_rpc, \
							sizeof(Dsc_deen_parity_rpc), (void *)param);
			break;
		case DSC_IO_CMD(IO_DSC_GET_DRIVER_VERSION):
			RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Dsc_drv_ver_rpc, \
							sizeof(Dsc_drv_ver_rpc), (void *)param);
			break;
		case DSC_IO_CMD(IO_DSC_VER_CHECK):
			RPC_PARAM_UPDATE(p3, PARAM_OUT, PARAM_Dsc_ver_chk_param_rpc, \
							sizeof(Dsc_ver_chk_param_rpc), (void *)param);
			break;
		case DSC_IO_CMD(IO_DSC_SET_PVR_KEY_IDLE):	
		case DSC_IO_CMD(IO_DSC_SET_CLR_CMDQ_EN):
		case DSC_IO_CMD(IO_DSC_DELETE_HANDLE_CMD):
			break;
		case DSC_IO_CMD(IO_DSC_FIXED_DECRYPTION):			   			
		case DSC_IO_CMD(IO_DSC_SYS_UK_FW):	
		default:
			return ALI_DSC_ERROR_OPERATION_NOT_SUPPORTED;
	}

	return RpcCallCompletion(RPC_dsc_ioctl,&p1,&p2,&p3,NULL); 
#endif				
}

int ali_trig_ram_mon(__u32 start_addr,__u32 end_addr, 
						__u32 interval, __u32 sha_mode, 
						int DisableOrEnable)
{
#if !defined(CONFIG_ALI_RPCNG)  
	jump_to_func(NULL, ali_rpc_call, start_addr, \
				DSC_NPARA(5)|FUNC_TRIG_RAM_MON, \
				NULL);
#else
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&start_addr);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&end_addr);   
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&interval);   
	RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32,sizeof(__u32), (void *)&sha_mode);   
	RPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_INT32,sizeof(int), (void *)&DisableOrEnable);   

	return RpcCallCompletion(RPC_trig_ram_mon,&p1,&p2,&p3,&p4,&p5,NULL);
#endif
}

int ali_DeEncrypt_ex(pDEEN_CONFIG p_DeEn,__u8 *input,
						__u8 *output , __u32 total_length)
{
#if !defined(CONFIG_ALI_RPCNG)  
	__u32 desc[] = 
	{
		1,DESC_STATIC_STRU(0, sizeof(DEEN_CONFIG)),1, DESC_P_PARA(0,0,0),0,0,
	};

	jump_to_func(NULL, ali_rpc_call, p_DeEn, \
				DSC_NPARA(4)|FUNC_DEENCRYPT, \
				desc);
#else     
	Param p1;
	Param p2;
	Param p3;
	Param p4;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_DeEncrypt_config_rpc, sizeof(DeEncrypt_config_rpc), (void *)p_DeEn);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&input);   
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&output);   
	RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&total_length);   

	return RpcCallCompletion(RPC_DeEncrypt,&p1,&p2,&p3,&p4,NULL);
#endif				
}

int ali_DeEncrypt(DEEN_CONFIG *p_deen,__u8 *input,
					__u8 *output , __u32 total_length)
{
	int ret = RET_FAILURE;
	DEEN_CONFIG deen;

	if(!p_deen || !input || !output || !total_length)
	{
		return ALI_DSC_ERROR_INVALID_PARAMETERS;
	}
	
	ali_dsc_umemcpy(&deen, p_deen, sizeof(DEEN_CONFIG));
	if((__u32)deen.dec_dev < VIRTUAL_DEV_NUM)
	{
		if(CSA == deen.Decrypt_Mode)
		{
			deen.dec_dev = (void *)hld_dev_get_by_id(HLD_DEV_TYPE_CSA, (__u32)deen.dec_dev);
		}
		else if(AES == deen.Decrypt_Mode)
		{
			deen.dec_dev = (void *)hld_dev_get_by_id(HLD_DEV_TYPE_AES, (__u32)deen.dec_dev);
		}
		else if((DES == deen.Decrypt_Mode) || (TDES == deen.Decrypt_Mode))
		{
			deen.dec_dev = (void *)hld_dev_get_by_id(HLD_DEV_TYPE_DES, (__u32)deen.dec_dev);
		}
	}
	if((__u32)deen.enc_dev < VIRTUAL_DEV_NUM)
	{
		 if(AES == deen.Encrypt_Mode)
		{
			deen.enc_dev = (void *)hld_dev_get_by_id(HLD_DEV_TYPE_AES, (__u32)deen.enc_dev);
		}
		else if((DES == deen.Decrypt_Mode) || (TDES == deen.Decrypt_Mode))
		{
			deen.enc_dev = (void *)hld_dev_get_by_id(HLD_DEV_TYPE_DES, (__u32)deen.enc_dev);
		}
	}
	
	if(!deen.dec_dev || !deen.enc_dev)
	{
		ALI_DSC_DEBUG_ERR("Invalid dec/enc dev, should be see pointer/ID\n");
		return ALI_DSC_ERROR_DRIVER_NOT_INITIALIZED;
	}
	
	dsc_deal_cache_before_dma(input,output,(total_length*TS_PACKET_SIZE));
	ret = ali_DeEncrypt_ex(&deen,input,output,total_length);
	dsc_deal_cache_after_dma(output,(total_length*TS_PACKET_SIZE));

	return ret;
}
EXPORT_SYMBOL(ali_DeEncrypt);

__u16 ali_dsc_get_free_stream_id(enum DMA_MODE dma_mode)
{
#if !defined(CONFIG_ALI_RPCNG)        
	 jump_to_func(NULL, ali_rpc_call, dma_mode, \
	 			  DSC_NPARA(1)|FUNC_GET_FREE_STREAM_ID, \
	 			  NULL);
#else
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_ENUM, sizeof(enum DMA_MODE), &dma_mode);
	return RpcCallCompletion(RPC_dsc_get_free_stream_id,&p1,NULL);
#endif				  
}
__u32 ali_dsc_get_free_sub_device_id(__u8 sub_mode)
{
#if !defined(CONFIG_ALI_RPCNG)  
	 jump_to_func(NULL, ali_rpc_call, sub_mode, \
	 			  DSC_NPARA(1)|FUNC_GET_FREE_SUB_DEVICE_ID, \
	 			  NULL);
#else
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(__u8), &sub_mode);

	return RpcCallCompletion(RPC_dsc_get_free_sub_device_id,&p1,NULL);
#endif
}

int ali_dsc_set_sub_device_id_idle
(
__u8 sub_mode,
__u32 device_id
)
{
#if !defined(CONFIG_ALI_RPCNG)    
	 jump_to_func(NULL, ali_rpc_call, sub_mode, \
	 			  DSC_NPARA(2)|FUNC_SET_SUB_DEVICE_ID_IDLE, \
	 			  NULL);
#else
	Param p1;
	Param p2;
	
	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(__u8), (void *)&sub_mode);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&device_id);

	return RpcCallCompletion(RPC_dsc_set_sub_device_id_idle,&p1,&p2,NULL);
#endif
}

int ali_dsc_set_stream_id_idle(__u32 pos) 
{
#if !defined(CONFIG_ALI_RPCNG)  
	 jump_to_func(NULL, ali_rpc_call, pos, \
	 			  DSC_NPARA(1)|FUNC_SET_STREAM_ID_IDLE, \
	 			  NULL);
#else
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), &pos);

	return RpcCallCompletion(RPC_dsc_set_stream_id_idle,&p1,NULL);
#endif
}

void ali_dsc_set_stream_id_used(__u32 pos) 
{
#if !defined(CONFIG_ALI_RPCNG)     
	 jump_to_func(NULL, ali_rpc_call, pos, \
	 			  DSC_NPARA(1)|FUNC_SET_STREAM_ID_USED, \
	 			  NULL);
#else
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), &pos);

	RpcCallCompletion(RPC_dsc_set_stream_id_used,&p1,NULL);
#endif
} 
int ali_dsc_set_sub_device_id_used(enum WORK_SUB_MODULE sub_mode, 
										__u32 device_id)
{
#if !defined(CONFIG_ALI_RPCNG)  
	 jump_to_func(NULL, ali_rpc_call, sub_mode, \
	 			  DSC_NPARA(2)|FUNC_SET_SUB_DEVICE_ID_USED, \
	 			  NULL);
#else
	Param p1;
	Param p2;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_ENUM, sizeof(enum WORK_SUB_MODULE), (void *)&sub_mode);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&device_id);

	return RpcCallCompletion(RPC_dsc_set_sub_device_id_used,&p1,&p2,NULL);
#endif
}

int ali_dsc_deal_quantum_for_mixed_ts(pDEEN_CONFIG p_DeEn, 
											__u32 temp_length)
{
#if !defined(CONFIG_ALI_RPCNG)  
	 jump_to_func(NULL, ali_rpc_call, p_DeEn, \
				  DSC_NPARA(2)|FUNC_DEAL_QUANTUM_MIXED_TS, \
	 			  NULL);
#else
	Param p1;
	Param p2;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_DeEncrypt_config_rpc, sizeof(DeEncrypt_config_rpc), (void *)p_DeEn);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&temp_length);   

	return RpcCallCompletion(RPC_dsc_deal_quantum_for_mixed_ts,&p1,&p2,NULL);
#endif				  
}

//CSA function
int csa_decrypt_ex(pCSA_DEV pCsaDev, __u16 stream_id, 
					__u8 *input, __u8 *output, 
					__u32 total_length)
{
#if !defined(CONFIG_ALI_RPCNG)        
	jump_to_func(NULL, ali_rpc_call, pCsaDev, \
				 DSC_NPARA(5)|FUNC_CSA_DECRYPT, \
				 NULL);
#else
	int ret = -1;
	Param p1;
	Param p2;
	Param p3;
	Param p4;
	Param p5;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pCsaDev);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT16, sizeof(__u16), (void *)&stream_id);   
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&input);   
	RPC_PARAM_UPDATE(p4, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&output);   
	RPC_PARAM_UPDATE(p5, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&total_length);   

	ret = RpcCallCompletion(RPC_csa_decrypt,&p1,&p2,&p3,&p4,&p5,NULL);
	return ret;
#endif
}

int ali_csa_decrypt(pCSA_DEV pCsaDev, __u16 stream_id,
					__u8 *input, __u8 *output,
					__u32 total_length)
{
	int ret = RET_FAILURE;

	dsc_deal_cache_before_dma(input,output,(total_length*TS_PACKET_SIZE));
	
	ret = csa_decrypt_ex(pCsaDev,stream_id,input,output,total_length);

	dsc_deal_cache_after_dma(output,(total_length*TS_PACKET_SIZE));

	return ret;
}

int ali_csa_ioctl
(
    CSA_DEV *pCsaDev,
    __u32 cmd,
    __u32 param
)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 i;
	__u32 common_desc[40];
	__u32 *desc = (__u32 *)common_desc;
	__u32 *b = (__u32 *)csa_io_control;
	KEY_PARAM *param_tmp = (KEY_PARAM *)param;
	
	for(i = 0; i < sizeof(csa_io_control)/sizeof(__u32); i++)
	{
		desc[i] = b[i];
	}
	
	switch(DSC_IO_CMD(cmd)) 
	{
		case DSC_IO_CMD(IO_INIT_CMD):
			DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(CSA_INIT_PARAM ));
			break;
			
		case DSC_IO_CMD(IO_CREAT_CRYPT_STREAM_CMD) :
		case DSC_IO_CMD(IO_KEY_INFO_UPDATE_CMD):
		{
			b = (__u32 *)dsc_io_control_1;
			for(i = 0; i < sizeof(dsc_io_control_1)/sizeof(__u32); i++)
			{
				desc[i] = b[i];
			}

			if(0 == param_tmp->pid_len)
				param_tmp->pid_len = 1; //at least transfer - one set of key

			for(i=0; i<3; i++)
			{
				DESC_STATIC_STRU_SET_SIZE(desc, (i+2), 2*param_tmp->pid_len*param_tmp->key_length/8);
			}

			DESC_STATIC_STRU_SET_SIZE(desc, 1, sizeof(__u16)*param_tmp->pid_len);
			break;
		}
		

		case DSC_IO_CMD(IO_DELETE_CRYPT_STREAM_CMD):
			desc = NULL;
			break;
		  
		default:
			return RET_FAILURE;
	}
	jump_to_func(NULL, ali_rpc_call, pCsaDev, DSC_NPARA(3)|FUNC_CSA_IOCTL, desc);
#else
	struct ali_m36_dsc_dev *dsc = g_ali_dsc_devices;
	KEY_PARAM *key_param = (KEY_PARAM *)param;
	DSC_CLR_KEY *dsc_key = (DSC_CLR_KEY *)dsc->dsc_key;
	CSA_INIT_PARAM csa_param;
	int ret = -1;
	Param p1;
	Param p2;
	Param p3;

	RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&pCsaDev);                 
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&cmd);         
	RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&param);

	switch(DSC_IO_CMD(cmd)) 
	{
		case DSC_IO_CMD(IO_INIT_CMD):
		{
			ali_dsc_umemcpy(&csa_param, (void *)param, sizeof(csa_param));
			RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Csa_init_param_rpc, sizeof(Csa_init_param_rpc), (void *)&csa_param);
			break;
		}
		
		case DSC_IO_CMD(IO_CREAT_CRYPT_STREAM_CMD):
		case DSC_IO_CMD(IO_KEY_INFO_UPDATE_CMD):
		{
			if(!key_param->pid_len || key_param->pid_len > ALI_DSC_LLD_MAX_ITEM)
			{
				key_param->pid_len = 1;
			}
			
			if(key_param->pid_list)
			{
				ali_dsc_umemcpy((dsc_key->pid_ptr),key_param->pid_list,sizeof(__u16)*(key_param->pid_len));
				key_param->pid_list = UC(dsc_key->pid_ptr);
			}
			
			if(key_param->p_csa_key_info)
			{
				ali_dsc_umemcpy((dsc_key->key_ptr),key_param->p_csa_key_info, \
								2*(key_param->pid_len)*key_param->key_length/8);
				key_param->p_csa_key_info = UC(dsc_key->key_ptr);				
			}
	
			RPC_PARAM_UPDATE(p3, PARAM_INOUT, PARAM_Key_param_rpc, sizeof(Key_param_rpc), (void *)key_param);      
			break;
		}
		

		case DSC_IO_CMD(IO_DELETE_CRYPT_STREAM_CMD):
			break;

		default:
		    ALI_DSC_DEBUG_ERR("Dsc rpc error: %d, invalid cmd=%d\n", __LINE__, cmd);
		    break;            
	}
	
	ret = RpcCallCompletion(RPC_csa_ioctl,&p1,&p2,&p3,NULL);
	
	return ret;

#endif
}


void ali_m36_dsc_see_init(void)
{
	pDSC_DEV pDscDev = ( pDSC_DEV ) hld_dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
	if(NULL == pDscDev)
	{
		ALI_DSC_DEBUG("ali_m36_dsc_see_init jump to see dsc_api_attach\n\n");
#if !defined(CONFIG_ALI_RPCNG)		
		jump_to_func(NULL, ali_rpc_call, NULL, \
					DSC_NPARA(0)|FUNC_DSC_ATTACH, \
					NULL);
#else
		RpcCallCompletion(RPC_dsc_api_attach,NULL);
#endif
	}
}
void ali_m36_dsc_see_uninit(void)
{
	pDSC_DEV pDscDev = ( pDSC_DEV ) hld_dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );
	if(NULL != pDscDev)
	{
		ALI_DSC_DEBUG("ali_m36_dsc_see_uninit jump to see dsc_api_detach\n\n");
#if !defined(CONFIG_ALI_RPCNG)		
		jump_to_func(NULL, ali_rpc_call, NULL, \
					DSC_NPARA(0)|FUNC_DSC_DETACH, \
					NULL);
#else
		RpcCallCompletion(RPC_dsc_api_detach,NULL);
#endif
	}
}


/* This function is used to decrypt or encrypt the puredata use key from crypto engine
__u8 *input,                the input data
__u8 *output,               the output data
__u32 length,               the data length
__u32 key_pos,              the key pos in secure key ladder
enum CRYPT_SELECT sel    decrypt/encrypt select (DSC_DECRYPT/DSC_ENCRYPT)
*/
int ali_aes_crypt_puredata_with_ce_key(__u8 *input, 
											__u8 *output, 
											__u32 length, 
											__u32 key_pos, 
											enum CRYPT_SELECT sel)
{
	__u32 AesDevId = ALI_INVALID_DSC_SUB_DEV_ID;
	pAES_DEV  pAesDev = NULL;
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	int ret = -1;
	__u32 stream_id = ALI_INVALID_CRYPTO_STREAM_ID;
	__u16 pid[1] = {0x1234};    
	AES_IV_INFO iv;
	
	if ((AesDevId = ali_dsc_get_free_sub_device_id(AES)) == \
		ALI_INVALID_DSC_SUB_DEV_ID)
	{
		ALI_DSC_DEBUG_ERR("ali_dsc_get_free_sub_device_id() failed\n");
		return -1;
	}
	
	if ((pAesDev = hld_dev_get_by_id(HLD_DEV_TYPE_AES, AesDevId)) == NULL)
	{
		ALI_DSC_DEBUG_ERR("Get AES device %d failed!\n", AesDevId);
		ali_dsc_set_sub_device_id_idle(AES, AesDevId);
		return -1;
	}
	
	if ((stream_id = ali_dsc_get_free_stream_id(PURE_DATA_MODE)) == \
		ALI_INVALID_CRYPTO_STREAM_ID)
	{
		ALI_DSC_DEBUG_ERR("ali_dsc_get_free_stream_id() failed!\n");
		goto DONE1;
	}
	
	memset(iv.even_iv, 0, sizeof(iv.even_iv));
	memset(iv.odd_iv, 0, sizeof(iv.odd_iv));   	
	memset(&aes_param, 0, sizeof(struct aes_init_param));
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_CRYPTO;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_CBC;
	ret = ali_aes_ioctl(pAesDev ,IO_INIT_CMD , (__u32)&aes_param);
	if(ret != RET_SUCCESS)
	{
		ALI_DSC_DEBUG_ERR("aes IO_INIT_CMD failed!\n");
		goto DONE2;
	}
	
	memset(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL ; 
	key_param.init_vector = (void *)&iv ;
	key_param.key_length = 128;  
	key_param.pid_len = 1;
	key_param.pid_list = pid;
	key_param.p_aes_iv_info = (void *)&iv ;
	key_param.p_aes_key_info = NULL;
	key_param.stream_id = stream_id;
	key_param.pos = key_pos;
	ret = ali_aes_ioctl(pAesDev, IO_CREAT_CRYPT_STREAM_CMD, \
						(__u32)&key_param);
	if(ret != RET_SUCCESS)
	{
		ALI_DSC_DEBUG_ERR("aes IO_CREAT_CRYPT_STREAM_CMD failed!\n");
		goto DONE2;
	}
		
	if(sel==DSC_ENCRYPT)
		ret = ali_aes_encrypt(pAesDev, stream_id, input, output, length);
	else
		ret = ali_aes_decrypt(pAesDev, stream_id, input, output, length);

	if(ret != RET_SUCCESS)
	{
		ALI_DSC_DEBUG_ERR("aes crypt fail!\n");
		ret = -1;
	}

    ali_aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
    ali_dsc_set_stream_id_idle(stream_id);
DONE1:
    ali_dsc_set_sub_device_id_idle(AES, AesDevId);	
    return ret;
}


/* This function is used to decrypt date by des with key from CE .
key_pos:      CE key pos 
input:        data in des
output:        data out des
data_len:        data len
*/
int ali_aes_cbc_decrypt_ramdata(__u8 key_pos,
									__u8 *input,
									__u8 *output, 
									__u32 data_len)
{
	struct aes_init_param aes_param;
	KEY_PARAM key_param;
	__u16 test_pid[1] = {0x1234};
	__u32 stream_id = ALI_INVALID_CRYPTO_STREAM_ID;
	int ret = -1;
	 __u32 AesDevId = ALI_INVALID_DSC_SUB_DEV_ID; 
	AES_IV_INFO iv;
	pAES_DEV pAES = NULL;
	
	memset(iv.even_iv, 0, sizeof(iv.even_iv));
	memset(iv.odd_iv, 0, sizeof(iv.odd_iv));

	AesDevId = ali_dsc_get_free_sub_device_id(AES);
	if( AesDevId == 0xff)
	{
		ALI_DSC_DEBUG_ERR("Error:Free AES not avaliable!!\n");
		return ret;
	}
        
	pAES = hld_dev_get_by_id(HLD_DEV_TYPE_AES, AesDevId);
	if(!pAES)
	{
	    ALI_DSC_DEBUG_ERR("Error:AES not avaliable!!\n");
	    return ret;
	}

	if ((stream_id = ali_dsc_get_free_stream_id(PURE_DATA_MODE)) == \
											ALI_INVALID_CRYPTO_STREAM_ID)
	{
		ALI_DSC_DEBUG_ERR("ali_dsc_get_free_stream_id() failed!\n");
		goto DONE1;
	}

	ali_aes_ioctl(pAES ,IO_DELETE_CRYPT_STREAM_CMD , 0);
	memset(&aes_param, 0, sizeof(struct aes_init_param));  
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_CRYPTO;
	aes_param.key_mode = AES_128BITS_MODE;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	aes_param.scramble_control = 0 ;
	aes_param.stream_id = stream_id; 
	aes_param.work_mode = WORK_MODE_IS_CBC;
	aes_param.cbc_cts_enable = 0;
	ret = ali_aes_ioctl(pAES,IO_INIT_CMD,(__u32)&aes_param);
	if(ret != RET_SUCCESS){
		ALI_DSC_DEBUG_ERR("aes IO_INIT_CMD failed!\n");
		goto DONE2;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));   
	key_param.handle = 0xff; 
	key_param.ctr_counter = NULL ;
	key_param.init_vector =NULL; 
	key_param.pid_len = 1; 
	key_param.pid_list = test_pid;
	key_param.p_aes_iv_info = &iv ;
	key_param.p_aes_key_info = NULL;
	key_param.stream_id = stream_id; 
	key_param.force_mode = 1;    
	key_param.pos = key_pos;  
	key_param.key_length = 128;

	ret = ali_aes_ioctl(pAES, IO_CREAT_CRYPT_STREAM_CMD , (__u32)&key_param);
	if(ret != RET_SUCCESS)
	{
		ALI_DSC_DEBUG_ERR("aes IO_CREAT_CRYPT_STREAM_CMD failed!\n");
		goto DONE2;
	}

	ret = ali_aes_decrypt(pAES, stream_id, input, output, data_len);
	if(ret != RET_SUCCESS )
	{
	    ALI_DSC_DEBUG_ERR("Error: dec decrypt fail!\n");
	    ret = -1;         
	}
	

	ali_aes_ioctl(pAES, IO_DELETE_CRYPT_STREAM_CMD, key_param.handle);
DONE2:
	ali_dsc_set_stream_id_idle(stream_id);
DONE1:
	ali_dsc_set_sub_device_id_idle(AES, AesDevId);	
	return ret;
}



/*
The function is xor the data from dst and src, the output in the dst
dst: the target buffer for xor
src: the xor data
len: data length
*/
static void ali_xor(__u8 *dst, __u8 *src, __u32 len)
{
    __u32 i;
    for(i=0;i<len;i++)
    {
        dst[i] = dst[i]^src[i];
    }
}
/*
note: KEY_1_2 is the PVR tmp. this function is only used in PVR now!
The function use the hmac arithmetic to verify the input text.
please refer to the fips-198a.pdf
*/
int ali_calculate_hmac(__u8 *input, __u32 length, 
						__u8 *output, __u8 *key)
{
	__u32 ShaDevID = ALI_INVALID_DSC_SUB_DEV_ID;
	pSHA_DEV pShaDev = NULL;
	SHA_INIT_PARAM param;
	int ret = RET_FAILURE;
	__u32 ipad = 0x36, opad = 0x5c;
	HMAC_PARAM k_buffer __attribute__((  aligned( 16 )));
	__u8 *temp_buff = NULL, *temp = NULL;
	__u32 chip_id;

	// 1, prepare k0, use the dsc decrypt the R2 to get the k0
	memset(&k_buffer,0,sizeof(HMAC_PARAM));
	memcpy(k_buffer.k0,key,FIRST_KEY_LENGTH);
	if(sizeof(chip_id) != ali_otp_read(0, (__u8 *)&chip_id, sizeof(chip_id)))
	{
		ALI_DSC_DEBUG_ERR("get chip_id fail!\n");
		return RET_FAILURE;
	}
	memcpy(&k_buffer.k0[FIRST_KEY_LENGTH], &chip_id,sizeof(chip_id));

	ALI_DSC_DEBUG("K0:\n");
	ALI_DSC_DUMP(k_buffer.k0,HASH_BLOCK_LENGTH);    

	// 2, k0 xor ipad 
	memset(k_buffer.ipad,ipad,sizeof(k_buffer.ipad));
	ali_xor(k_buffer.k0,k_buffer.ipad,HASH_BLOCK_LENGTH);
	ALI_DSC_DEBUG("K0:\n");
	ALI_DSC_DUMP(k_buffer.k0,HASH_BLOCK_LENGTH);

	// 3, (k0 xor ipad) || text
	temp_buff = (__u8*)((__u32)kzalloc(length+0xf+ \
							HASH_BLOCK_LENGTH,GFP_KERNEL));
	temp = (__u8 *)(0xFFFFFFF8 & (__u32)temp_buff);	
	memcpy(temp,k_buffer.k0,HASH_BLOCK_LENGTH);
	memcpy(&temp[HASH_BLOCK_LENGTH],input,length);

	// 4, Hash((k0 xor ipad) || text)
	if (ALI_INVALID_DSC_SUB_DEV_ID == \
	(ShaDevID = ali_dsc_get_free_sub_device_id(SHA)))
	{
		ALI_DSC_DEBUG_ERR("get free SHA device failed!\n");
		kfree(temp_buff);
		return RET_FAILURE;
	}
	if (NULL == (pShaDev = (pSHA_DEV)hld_dev_get_by_id(HLD_DEV_TYPE_SHA, ShaDevID)))
	{
		ALI_DSC_DEBUG_ERR("get SHA device %d failed!\n", ShaDevID);
		ali_dsc_set_sub_device_id_idle(SHA, ShaDevID);
		kfree(temp_buff);
		return RET_FAILURE;
	}
	memset(&param, 0, sizeof(SHA_INIT_PARAM));
	param.sha_data_source = SHA_DATA_SOURCE_FROM_DRAM;
	param.sha_work_mode = SHA_SHA_256;
	ali_sha_ioctl(pShaDev, IO_INIT_CMD, (__u32)&param);
	ret = ali_sha_digest(pShaDev, temp, k_buffer.hout, \
						length+HASH_BLOCK_LENGTH);
	if (ret != RET_SUCCESS)
	{
		ALI_DSC_DEBUG_ERR("sha_digest fail!\n");
		ali_dsc_set_sub_device_id_idle(SHA, ShaDevID);
		kfree(temp_buff);
		return RET_FAILURE;
	}
	// 5, k0 xor opad
	ali_xor(k_buffer.k0,k_buffer.ipad,HASH_BLOCK_LENGTH);  
	memset(k_buffer.opad,opad,sizeof(k_buffer.opad));
	ali_xor(k_buffer.k0,k_buffer.opad,HASH_BLOCK_LENGTH);    

	// 6, (k0 xor opad) || Hash((k0 xor ipad) || text)
	memcpy(temp, k_buffer.k0, HASH_BLOCK_LENGTH);
	memcpy(&temp[HASH_BLOCK_LENGTH],k_buffer.hout,HASH_BLOCK_LENGTH);

	// 7, Hash((k0 xor opad) || Hash((k0 xor ipad) || text))
	ret = ali_sha_digest(pShaDev, temp, k_buffer.hout, 2*HASH_BLOCK_LENGTH);
	if (ret != RET_SUCCESS)
	{
		ALI_DSC_DEBUG_ERR("sha_digest fail!\n");
		ali_dsc_set_sub_device_id_idle(SHA, ShaDevID);
		kfree(temp_buff);
		return RET_FAILURE;
	}
	if(ali_dsc_set_sub_device_id_idle(SHA, ShaDevID)!=RET_SUCCESS)
	{
		ALI_DSC_DEBUG_ERR("dsc_set_sub_device_id_idle fail!\n");
		kfree(temp_buff);
		return RET_FAILURE;        
	}
	memcpy(output,k_buffer.hout,HMAC_OUT_LENGTH);
	kfree(temp_buff);

	ALI_DSC_DEBUG("HMAC output : \n");
	ALI_DSC_DUMP(output,HMAC_OUT_LENGTH);
	ALI_DSC_DEBUG("HMAC done, ret: %d!\n", ret);
	return ret;
}


int ali_create_sha_ramdrv(char *input,__u32 len, 
							__u8 *output)
{
  	__u32 ShaDevID = ALI_INVALID_DSC_SUB_DEV_ID;
	pSHA_DEV pShaDev = NULL;
	SHA_INIT_PARAM param;
	int ret = RET_FAILURE;

	if (ALI_INVALID_DSC_SUB_DEV_ID == \
		(ShaDevID = ali_dsc_get_free_sub_device_id(SHA)))
	{
		ALI_DSC_DEBUG_ERR("get free SHA device failed!\n");
		goto err_exit;
	}
	if (NULL == \
		(pShaDev = (pSHA_DEV)hld_dev_get_by_id(HLD_DEV_TYPE_SHA, ShaDevID)))
	{
		ALI_DSC_DEBUG_ERR("get SHA device %d failed!\n", ShaDevID);
		ali_dsc_set_sub_device_id_idle(SHA, ShaDevID);
		goto err_exit;
	}
	memset(&param, 0, sizeof(SHA_INIT_PARAM));
	param.sha_data_source = SHA_DATA_SOURCE_FROM_DRAM;
	param.sha_work_mode = SHA_SHA_256;
	ali_sha_ioctl(pShaDev, IO_INIT_CMD, (__u32)&param);
	ret = ali_sha_digest(pShaDev, input, output, len);
	if (ret != RET_SUCCESS)
	{
		ALI_DSC_DEBUG_ERR("sha_digest fail!\n");
	        ali_dsc_set_sub_device_id_idle(SHA, ShaDevID);
		goto err_exit;		
	}	
	
	ali_dsc_set_sub_device_id_idle(SHA, ShaDevID);
err_exit:
	return ret ;
}
/* This function is used to decrypt and verify the MAC address,if verify fail, the system will reboot
__u8 *mac:  the cipher mac address with its HMAC result 
//16B = 6 Bytes mac addr + 10 bytes pedding+ 32B random value+16byte cipher mac +32byte HMAC result
unsigned long len:  the total lengh of the MAC address with its HMAC result, it is 48 bytes now
__u8 *clear_mac  the output plain text mac address
*
**    mac address output format :
*        mac address (6 bytes + 10 bytes null) 
*      + magic number(4 bytes )
*      + encrypt mac hash (32bytes)
*      + R5+R6 (16 bytes + 16 bytes)   
*/
#define HASH_OUT_LENGTH (32)
#define MAC_TOTAL_LENGTH (6+10+4+32+32)
#define MAC_HASH_OFFSET (20)
int ali_verify_mac_addr(__u8 *mac, __u32 len, 
						__u8 *clear_mac)
{
	int ret=-1;
	pCE_DEVICE pCeDev = (pCE_DEVICE)hld_dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	__u8 key_pos;
	AES_KEY_LADDER_BUF_PARAM aes_ce_param;
	__u8 *t_buf1=NULL,*t_buf2=NULL,*input,*output;
	__u8 hash_buf[HASH_OUT_LENGTH],decrypt_buf[HASH_OUT_LENGTH];	    

	t_buf1 = (__u8*)kzalloc(MAC_TOTAL_LENGTH+0x20, GFP_KERNEL);    
	t_buf2 = (__u8*)kzalloc(MAC_TOTAL_LENGTH+0x20, GFP_KERNEL);  

	input = (__u8 *)((__u32)(t_buf1+0x1f)&(~0xf));		 
	output = (__u8 *)((__u32)(t_buf2+0x1f)&(~0xf));   

	/*1.generate key pos from ce*/	
	memset(&aes_ce_param ,0, sizeof(AES_KEY_LADDER_BUF_PARAM));
	aes_ce_param.key_ladder=2;
	aes_ce_param.root_key_pos=KEY_0_1;
	memcpy(&aes_ce_param.r[0],&mac[16+4+HASH_OUT_LENGTH],32);
	if((ret = ali_aes_generate_key_with_multi_keyladder(&aes_ce_param, \
													&key_pos)) != RET_SUCCESS)
	{
		ALI_DSC_DEBUG_ERR("generate key error!\n");
		ret = RET_FAILURE;
		goto err_exit;
	}
	ALI_DSC_DEBUG("mac load result:\n");
	ALI_DSC_DUMP(mac, len);
	/*2. decrypt hash result*/ 
	memcpy(input, mac+MAC_HASH_OFFSET,HASH_OUT_LENGTH);
	ret=ali_aes_crypt_puredata_with_ce_key(input,\
					 output, \
					HASH_OUT_LENGTH, \
					key_pos,      \
					DSC_DECRYPT);
	if(ret != RET_SUCCESS)
	{
	    ALI_DSC_DEBUG_ERR("decrypt error! %s\n",__FUNCTION__);
	    ali_ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos);	
	    ret = RET_FAILURE;
	    goto err_exit;
	}
	//ALI_DSC_DEBUG_ERR_DUMP((__u8*)((__u32)output|0xa0000000),HASH_OUT_LENGTH);
	memcpy(decrypt_buf,output,HASH_OUT_LENGTH);    
	ALI_DSC_DEBUG("decrypt mac hash result:\n");
	ALI_DSC_DUMP(decrypt_buf, HASH_OUT_LENGTH);

	/*3. calc pure mac address hash*/
	memcpy(input,mac,6);    
	if (RET_SUCCESS != ali_create_sha_ramdrv(input, 6, output))
	{
		ALI_DSC_DEBUG_ERR("create_sha_ramdrv() failed!\n");
		ret = RET_FAILURE ;
	    goto err_exit ;
	}
	memcpy(hash_buf,output,HASH_OUT_LENGTH);
	ALI_DSC_DEBUG("calc mac hash result:\n");
	ALI_DSC_DUMP(hash_buf, HASH_OUT_LENGTH); 

	/*4. compare result*/ 
	if(memcmp(hash_buf,decrypt_buf,HASH_OUT_LENGTH))
	{
	    ALI_DSC_DUMP(hash_buf,HASH_OUT_LENGTH);        
	    ALI_DSC_DUMP(decrypt_buf,HASH_OUT_LENGTH);               
	    goto err_exit;
	}

	kfree(t_buf1);
	kfree(t_buf2);
	ali_ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos);
	return RET_SUCCESS;

err_exit:    
	 ALI_DSC_DEBUG_ERR("mac address verify fail, reboot\n");
	  return RET_FAILURE;
	//    hw_watchdog_reboot();
}


