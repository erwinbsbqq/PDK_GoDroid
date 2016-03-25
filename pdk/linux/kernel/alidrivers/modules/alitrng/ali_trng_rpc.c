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

#include "ali_trng_lld.h"

#if defined(CONFIG_ALI_RPCNG)
#include <ali_rpcng.h>
#endif

#define TRNG_NPARA(x) ((HLD_DSC_MODULE<<24)|(x<<16))

int ali_trng_generate_byte( __u8 *data )
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 desc[] =
	{
	    1, DESC_OUTPUT_STRU(0, sizeof(__u8)), 1, DESC_P_PARA(0, 0, 0), 0, 0, 
	};
	jump_to_func(NULL, ali_rpc_call, data, TRNG_NPARA(1) | FUNC_TRNG_GENERATE_BYTE, desc);
#else
	RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_UCHAR, sizeof(__u8), (void *)data);
	return RpcCallCompletion(RPC_trng_generate_byte,&p1,NULL);
#endif
}

int ali_trng_generate_64bits( __u8 *data )
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 desc[] =
	{
	    1, DESC_OUTPUT_STRU(0, ALI_TRNG_64BITS_SIZE), 1, DESC_P_PARA(0, 0, 0), 0, 0, 
	};
	jump_to_func(NULL, ali_rpc_call, data, TRNG_NPARA(1) | FUNC_TRNG_GENERATE_64BITS, desc);
#else
	int ret = -1;
	Trng_data_rpc trng_data;
	Param p1;
	
	memset((void *)&trng_data, 0x00, sizeof(trng_data));
	
	RPC_PARAM_UPDATE(p1, PARAM_OUT, PARAM_Trng_data_rpc, ALI_TRNG_64BITS_SIZE, (void *)&trng_data);
    
	ret = RpcCallCompletion(RPC_trng_generate_64bits,&p1,NULL);
	memcpy(data, trng_data.data, ALI_TRNG_64BITS_SIZE);
	return ret;
#endif
}

int ali_trng_get_64bits_ex(__u8 *data, __u32 n)
{
#if !defined(CONFIG_ALI_RPCNG)
	__u32 desc[] =
	{
	    1, DESC_OUTPUT_STRU(0, (ALI_TRNG_64BITS_SIZE*n)), 1, DESC_P_PARA(0, 0, 0), 0, 0, 
	};
	jump_to_func(NULL, ali_rpc_call, data, TRNG_NPARA(2) | FUNC_TRNG_SEE_GET64BIT, desc);
#else
	int ret = -1;
	Trng_data_rpc trng_data;
	Param p1;
	Param p2;

	memset((void *)&trng_data, 0x00, sizeof(trng_data));
	
	RPC_PARAM_UPDATE(p1, PARAM_OUT, PARAM_Trng_data_rpc, ALI_TRNG_64BITS_SIZE*n, (void *)&trng_data);
	RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, sizeof(__u32), (void *)&n);

	ret = RpcCallCompletion(RPC_trng_get_64bits,&p1,&p2,NULL);

	memcpy(data, trng_data.data, ALI_TRNG_64BITS_SIZE*n);
	return ret;
#endif
}


int ali_trng_get_64bits(__u8 *data, __u32 n)
{
	int ret = RET_FAILURE;
	__u32 trng_c = 0;
	__u32 deal_c = 0;
	
	while(deal_c < n)
	{
		trng_c = (n-deal_c)>ALI_TRNG_MAX_GROUP?ALI_TRNG_MAX_GROUP:(n-deal_c);
		ret = ali_trng_get_64bits_ex(data+ALI_TRNG_64BITS_SIZE*deal_c,(__u32)trng_c);
		if (0 != ret)
		{
			ALI_TRNG_RPC_DEBUG("Err:ali_trng_get_64bits_ex() failed, ret:%d\n", ret);
			return ret;
		}  
		deal_c += trng_c;
	}
	
	return ret;
}



