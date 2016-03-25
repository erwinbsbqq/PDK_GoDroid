#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <hld/dsc/adr_dsc.h>

#ifndef ALI_DSC_HLD_BASIC_DBG
#define ALI_DSC_HLD_PRF(...) do{}while(0)
#else
#define ALI_DSC_HLD_PRF(fmt, args...)  \
					do { \
							ADR_DBG_PRINT(DSC, fmt, ##args); \
					} while(0)
#endif

void *dsc_get_mem(struct ali_dsc_krec_mem *krec_mem)
{
	UINT32 addr = 0;
	UINT32 kaddr = 0;
	DSC_DEV *pDscDev = NULL;
	
	pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	if(NULL == pDscDev)
	{
		return NULL;
	}
	
	if(0 == ioctl(pDscDev->fd, IO_REQUEST_KREC_ADDR, (UINT32)&kaddr))
	{
		if(0 == ioctl(pDscDev->fd, IO_REQUEST_KREC_SPACE, (UINT32)krec_mem))
		{
			addr = (UINT32)pDscDev->user_base + ((UINT32)krec_mem->pa_mem - kaddr);
			krec_mem->va_mem = (void *)addr;
		}
	}
	
	return (void*)(addr);
}

int dsc_put_mem(void *addr, UINT32 size)
{
	struct ali_dsc_krec_mem krec_mem;
	DSC_DEV *pDscDev = NULL;
	UINT32 kaddr = 0;
	
	pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	if(NULL == pDscDev)
	{
		return RET_FAILURE;
	}
		
	if(0 == ioctl(pDscDev->fd, IO_REQUEST_KREC_ADDR, (UINT32)&kaddr))
	{
		krec_mem.size = size;
		krec_mem.pa_mem = (void *)(kaddr + ((UINT32)addr - (UINT32)pDscDev->user_base));
	
		if(0 == ioctl(pDscDev->fd, IO_RELEASE_KREC_SPACE, (UINT32)&krec_mem))
		{
			return 0;
		}
	}
	
	return RET_FAILURE;
}

int dsc_operate_mem(pDSC_DEV pDscDev, UINT32 cmd, UINT32 param)
{
	struct ali_dsc_krec_mem *krec_mem = (struct ali_dsc_krec_mem *)param;
	int ret = RET_FAILURE;

	if(!param)
	{
		ALI_DSC_HLD_PRF("Error: param is NULL\n");
		return ret;
	}
	
	if(IO_RELEASE_KREC_SPACE == cmd)
	{
		ret = dsc_put_mem(krec_mem->va_mem, krec_mem->size);
	}
	
	if(IO_REQUEST_KREC_SPACE == cmd)
	{
		krec_mem->va_mem = dsc_get_mem(krec_mem);
		if (NULL == krec_mem->va_mem)
		{
			ALI_DSC_HLD_PRF("Error: dsc_get_mem\n");
			ret = !RET_SUCCESS;
		}
	}
	return ret;
}


int des_decrypt(pDES_DEV pDesDev,UINT16 stream_id, 
				UINT8 *input, UINT8 *output, 
				UINT32 total_length)
{
	int ret=RET_FAILURE;
	ALI_DSC_CRYPTION dsc_cryption;

	if((NULL==pDesDev) || (NULL == pDesDev->priv))
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDesDev is NULL!\n");
		return ret;    
	}

	if(total_length>PURE_DATA_MAX_SIZE)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: len is out of range!\n");
		return ret;
	}

	dsc_cryption.dev = (void *)pDesDev->priv;
	dsc_cryption.stream_id = (UINT32)stream_id;
	dsc_cryption.input = (UINT8 *)input;
	dsc_cryption.output = (UINT8 *)output;
	dsc_cryption.length = (UINT32)total_length;
	ret = ioctl(pDesDev->fd, ALI_DSC_IO_DECRYPT, (UINT32)&dsc_cryption);

	return ret;
}

int des_encrypt(pDES_DEV pDesDev,UINT16 stream_id, 
				UINT8 *input, UINT8 *output, 
				UINT32 total_length)
{
	int ret=RET_FAILURE;
	ALI_DSC_CRYPTION dsc_cryption;
	
	if((NULL==pDesDev) || (NULL == pDesDev->priv))
	{
	    ALI_DSC_HLD_PRF("Dsc hld error: pDesDev is NULL!\n");
	    return ret;    
	}

	if(total_length>PURE_DATA_MAX_SIZE)
	{
	    ALI_DSC_HLD_PRF("Dsc hld error: len is out of range!\n");
	    return ret;
	}

	dsc_cryption.dev = (void *)pDesDev->priv;
	dsc_cryption.stream_id = (UINT32)stream_id;
	dsc_cryption.input = (UINT8 *)input;
	dsc_cryption.output = (UINT8 *)output;
	dsc_cryption.length = (UINT32)total_length;
	ret = ioctl(pDesDev->fd, ALI_DSC_IO_ENCRYPT, (UINT32)&dsc_cryption);
	
	return ret;
}


int des_ioctl( pDES_DEV pDesDev ,UINT32 cmd , UINT32 param)
{
	int ret=RET_FAILURE;
	ALI_DSC_IO_PARAM ioc_param;
	
	if((NULL==pDesDev) || (NULL == pDesDev->priv))
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDesDev is NULL!\n");
		return ret;
	}
	
	ioc_param.dev = (void *)pDesDev->priv;
	ioc_param.ioc_param = (void *)param;
	ret = ioctl(pDesDev->fd, cmd, (UINT32)&ioc_param);
	
	return ret;
}

int aes_decrypt(pAES_DEV pAesDev,UINT16 stream_id,
				UINT8 *input, UINT8 *output,
				UINT32 total_length)
{
	int ret=RET_FAILURE;
	
	ALI_DSC_CRYPTION dsc_cryption;
	
	if((NULL==pAesDev) || (NULL == pAesDev->priv))
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pAesDev is NULL!\n");
		return ret;    
	}

	if(total_length>PURE_DATA_MAX_SIZE)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: len is out of range!\n");
		return ret;
	}

	dsc_cryption.dev = (void *)pAesDev->priv;
	dsc_cryption.stream_id = (UINT32)stream_id;
	dsc_cryption.input = (UINT8 *)input;
	dsc_cryption.output = (UINT8 *)output;
	dsc_cryption.length = (UINT32)total_length;
	ret = ioctl(pAesDev->fd, ALI_DSC_IO_DECRYPT, (UINT32)&dsc_cryption);
	
	return ret;
}

int aes_encrypt(pAES_DEV pAesDev,UINT16 stream_id,
				UINT8 *input, UINT8 *output,
				UINT32 total_length)
{
	int ret=RET_FAILURE;
	ALI_DSC_CRYPTION dsc_cryption;

	if((NULL==pAesDev) || (NULL == pAesDev->priv))
	{
		    ALI_DSC_HLD_PRF("Dsc hld error: pAesDev is NULL!\n");
		    return ret;    
	}
	if(total_length>PURE_DATA_MAX_SIZE)
	{
		    ALI_DSC_HLD_PRF("Dsc hld error: len is out of range!\n");
		    return ret;
	}

	dsc_cryption.dev = (void *)(pAesDev->priv);
	dsc_cryption.stream_id = (UINT32)stream_id;
	dsc_cryption.input = (UINT8 *)input;
	dsc_cryption.output = (UINT8 *)output;
	dsc_cryption.length = (UINT32)total_length;
	ret = ioctl(pAesDev->fd, ALI_DSC_IO_ENCRYPT, (UINT32)&dsc_cryption);

	return ret;
}

int aes_ioctl( pAES_DEV pAesDev ,UINT32 cmd , UINT32 param)
{
	int ret=RET_FAILURE;
	ALI_DSC_IO_PARAM ioc_param;
	
	if((NULL==pAesDev) || (NULL == pAesDev->priv))
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pAesDev is NULL!\n");
		return ret;
	}
	
	ioc_param.dev = (void *)pAesDev->priv;
	ioc_param.ioc_param = (void *)param;
	ret = ioctl(pAesDev->fd, cmd, (UINT32)&ioc_param);
	return ret;
}

int csa_decrypt(pCSA_DEV pCsaDev ,UINT16 stream_id,
				UINT8 *input, UINT8 *output,
				UINT32 total_length)
{
	int ret=RET_FAILURE;
	ALI_DSC_CRYPTION dsc_cryption;
	
	if((NULL==pCsaDev) || (NULL == pCsaDev->priv))
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pCsaDev is NULL!\n");
		return ret;    
	}
	if(total_length>PURE_DATA_MAX_SIZE)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: len is out of range!\n");
		return ret;
	}
	
	dsc_cryption.dev = (void *)(pCsaDev->priv);
	dsc_cryption.stream_id = (UINT32)stream_id;
	dsc_cryption.input = (UINT8 *)input;
	dsc_cryption.output = (UINT8 *)output;
	dsc_cryption.length = (UINT32)total_length;
	ret = ioctl(pCsaDev->fd, ALI_DSC_IO_DECRYPT, (UINT32)&dsc_cryption);

	return ret;
}

int csa_ioctl(pCSA_DEV pCsaDev,UINT32 cmd , UINT32 param)
{
	int ret=RET_FAILURE;
	ALI_DSC_IO_PARAM ioc_param;
	
	if((NULL==pCsaDev) || (NULL == pCsaDev->priv))
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pCsaDev is NULL!\n");
		return ret;    
	}

	ioc_param.dev = (void *)(pCsaDev->priv);
	ioc_param.ioc_param = (void *)param;
	ret = ioctl(pCsaDev->fd, cmd, (UINT32)&ioc_param);
	return ret;
}

int sha_ioctl( pSHA_DEV pShaDev ,UINT32 cmd , UINT32 param)
{
	int ret=RET_FAILURE;
	ALI_DSC_IO_PARAM ioc_param;
	
	if((NULL==pShaDev) || (NULL == pShaDev->priv))
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pShaDev is NULL!\n");
		return ret;
	}

	ioc_param.dev = (void *)pShaDev->priv;
	ioc_param.ioc_param = (void *)param;
	ret = ioctl(pShaDev->fd, cmd, (UINT32)&ioc_param);

	return ret;
}

UINT32 g_sha_ex_buf=0;
int sha_digest(pSHA_DEV pShaDev, UINT8 *input,
				UINT8 *output,UINT32 data_length)
{
	int ret=RET_FAILURE;
	UINT32 pure_data_len=0;
	ALI_SHA_DIGEST sha_digest;
	
	if((NULL==pShaDev) || (NULL == pShaDev->priv))
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pShaDev is NULL!\n");
		return ret;    
	}

	if(g_sha_ex_buf)
	{
		pure_data_len = PURE_DATA_MAX_SIZE*10; // 10M
	}
	else
	{
		pure_data_len = PURE_DATA_MAX_SIZE;
	}

	if(data_length>pure_data_len)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: len out of range!\n");
		return ret;
	}

	sha_digest.dev = (void *)(pShaDev->priv);
	sha_digest.input = (UINT8 *)input;
	sha_digest.output = (UINT8 *)output;
	sha_digest.length = (UINT32)data_length;
	ret = ioctl(pShaDev->fd, ALI_DSC_IO_SHA_DIGEST, (UINT32)&sha_digest);

	return ret;
}


//DSC function
int dsc_ioctl( pDSC_DEV pDscDev ,UINT32 cmd , UINT32 param)
{
	int ret=RET_FAILURE;
	
	if(NULL==pDscDev)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDscDev is NULL!\n");
		return ret;    
	}
	
	ret = ioctl(pDscDev->fd, cmd, param);

	return ret;
}


int trig_ram_mon(UINT32 start_addr,UINT32 end_addr,
					UINT32 interval, enum SHA_MODE sha_mode,
					int DisableOrEnable)
{
	int ret=RET_FAILURE; 
	ALI_DSC_RAM_MON ram_mon_param;
	DSC_DEV *pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	
	if(NULL == pDscDev)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDscDev is NULL!\n");
		return ret;
	}

	ram_mon_param.start_addr = start_addr;
	ram_mon_param.end_addr = end_addr;
	ram_mon_param.interval = interval;
	ram_mon_param.sha_mode = sha_mode;
	ram_mon_param.dis_or_en = DisableOrEnable;
	ret = ioctl(pDscDev->fd, ALI_DSC_IO_TRIG_RAM_MON, (UINT32)&ram_mon_param); 

	return ret;
}

int DeEncrypt(pDEEN_CONFIG p_DeEn,UINT8 *input,
				UINT8 *output , UINT32 total_length)
{
	int ret=RET_FAILURE;
	ALI_RE_ENCRYPT re_encrypt;
	DSC_DEV *pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	
	if(NULL == pDscDev)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDscDev is NULL!\n");
		return ret;
	}
	
	re_encrypt.p_deen = (pDEEN_CONFIG)p_DeEn;
	re_encrypt.input = (UINT8 *)input;
	re_encrypt.output = (UINT8 *)output;
	re_encrypt.length = total_length;
	ret = ioctl(pDscDev->fd, ALI_DSC_IO_DEENCRYPT, (UINT32)&re_encrypt);

	return ret;
}

UINT16 dsc_get_free_stream_id(enum DMA_MODE dma_mode)
{
	int ret=RET_FAILURE;
	UINT32 stream_id = INVALID_DSC_STREAM_ID;
	ALI_DSC_ID_INFO dsc_id;
	DSC_DEV *pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	
	if(NULL == pDscDev)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDscDev is NULL!\n");
		return ret;
	}

	dsc_id.mode = (UINT32)dma_mode;
	dsc_id.id_number = (UINT32)&stream_id;
	ret = ioctl(pDscDev->fd, ALI_DSC_IO_GET_FREE_STREAM_ID, (UINT32)&dsc_id);  

	if(ret != RET_SUCCESS)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: ioctl\n");
	}
	
	return stream_id;
}

UINT32 dsc_get_free_sub_device_id(enum WORK_SUB_MODULE sub_mode)
{
	int ret=RET_FAILURE;
	UINT32 device_id = INVALID_DSC_SUB_DEV_ID;
	ALI_DSC_ID_INFO dsc_id;
	DSC_DEV *pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	
	if(NULL == pDscDev)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDscDev is NULL!\n");
		return ret;
	}
	

	dsc_id.mode = (UINT32)sub_mode;
	dsc_id.id_number = (UINT32)&device_id;
	ret = ioctl(pDscDev->fd, ALI_DSC_IO_GET_FREE_SUB_DEVICE_ID, (UINT32)&dsc_id);	

	if(ret != RET_SUCCESS)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: ioctl\n");
	}  
	return device_id;      
}

int dsc_set_sub_device_id_idle(enum WORK_SUB_MODULE sub_mode,
									UINT32 device_id)
{
	int ret=RET_FAILURE;
	ALI_DSC_ID_INFO dsc_id;
	DSC_DEV *pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	
	if(NULL == pDscDev)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDscDev is NULL!\n");
		return ret;
	}
	
	dsc_id.mode = (UINT32)sub_mode;
	dsc_id.id_number = (UINT32)device_id;
	ret = ioctl(pDscDev->fd, ALI_DSC_IO_SET_SUB_DEVICE_ID_IDLE, (UINT32)&dsc_id);  

	if(ret != RET_SUCCESS)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: ioctl\n");
	}  
	return ret;
}

int dsc_set_stream_id_idle(UINT32 pos) 
{
	int ret=RET_FAILURE;
	DSC_DEV *pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	
	if(NULL == pDscDev)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDscDev is NULL!\n");
		return ret;
	}
	
	ret = ioctl(pDscDev->fd, ALI_DSC_IO_SET_STREAM_ID_IDLE, (UINT32)pos);
	return ret;
}    

void dsc_set_stream_id_used(UINT32 pos)
{
	int ret=RET_FAILURE;
	DSC_DEV *pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	
	if(NULL == pDscDev)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDscDev is NULL!\n");
		return;
	}
	
	ioctl(pDscDev->fd, ALI_DSC_IO_SET_STREAM_ID_USED, (UINT32)pos); 
} 

int dsc_set_sub_device_id_used(enum WORK_SUB_MODULE sub_mode,
									UINT32 device_id)
{
	int ret=RET_FAILURE;
	ALI_DSC_ID_INFO dsc_id;
	DSC_DEV *pDscDev = (DSC_DEV *)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	
	if(NULL == pDscDev)
	{
		ALI_DSC_HLD_PRF("Dsc hld error: pDscDev is NULL!\n");
		return ret;
	}
	
	dsc_id.mode = (UINT32)sub_mode;
	dsc_id.id_number = (UINT32)device_id;
	ret = ioctl(pDscDev->fd, ALI_DSC_IO_SET_SUB_DEVICE_ID_USED, (UINT32)&dsc_id);  

	return ret;
}

