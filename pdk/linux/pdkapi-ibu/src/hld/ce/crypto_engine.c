#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <hld/crypto/crypto.h>

#ifndef ALI_CE_HLD_BASIC_DBG
#define ALI_CE_HLD_PRF(...) do{}while(0)
#else
#define ALI_CE_HLD_PRF(fmt, arg...) \
    do { \
        printf("CE_HLD: In %s "fmt, __func__, ##arg); \
    } while (0)
#endif

int ce_key_generate(pCE_DEVICE pCeDev,pCE_DATA_INFO pCe_data_info)
{
	int ret = RET_FAILURE;
	
	if(NULL == pCeDev)
	{
		ALI_CE_HLD_PRF("CE hld error:pCeDev is NULL\n"); 
		return RET_FAILURE;
	}
		
	ret = ioctl(pCeDev->fd, IO_CE_KEY_GENERATE, (UINT32)pCe_data_info);

	return ret;
}

int ce_key_load(pCE_DEVICE pCeDev,pOTP_PARAM pCe_opt_info)
{
	int ret = RET_FAILURE;
	
	if(NULL == pCeDev)
	{
		ALI_CE_HLD_PRF("CE hld error:pCeDev is NULL\n"); 
		return RET_FAILURE;
	}    

	ret = ioctl(pCeDev->fd, IO_CE_KEY_LOAD, (UINT32)pCe_opt_info);
	return ret;
}

int ce_ioctl(pCE_DEVICE pCeDev,UINT32 cmd,UINT32 param)
{
	int ret = RET_FAILURE;
	
	if(NULL == pCeDev)
	{
		ALI_CE_HLD_PRF("CE hld error:pCeDev is NULL\n"); 
		return RET_FAILURE;
	}

	ret = ioctl(pCeDev->fd, cmd, (UINT32)param);
	
	return ret; 
}

int ce_generate_cw_key(UINT8 *in_cw_data, 
							UINT8 mode,UINT8 first_pos, 
							UINT8 second_pos)
{
	int ret = RET_FAILURE;
	ALI_CE_GEN_CW ce_gen_cw;

	CE_DEVICE *pCeDev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	if(NULL == pCeDev)
	{
		ALI_CE_HLD_PRF("Err: pCeDev is NULL\n");
		return RET_FAILURE;
	}

	ce_gen_cw.in = in_cw_data;
	ce_gen_cw.aes_or_des = mode;
	ce_gen_cw.lowlev_pos = first_pos;
	ce_gen_cw.highlev_pos = second_pos;
	ret = ioctl(pCeDev->fd, IO_CE_GENERATE_CW_KEY, (UINT32)&ce_gen_cw);

	return ret;
}

int ce_generate_single_level_key(pCE_DEVICE pCeDev,
									pCE_DATA_INFO pCe_data_info)
{
	int ret = RET_FAILURE;

	if(NULL == pCeDev)
	{
		ALI_CE_HLD_PRF("CE hld error:pCeDev is NULL\n");  
		return RET_FAILURE;
	}

	ret = ioctl(pCeDev->fd, IO_CE_GENERATE_SINGLE_LEVEL_KEY, (UINT32)pCe_data_info);

	return ret;
}



