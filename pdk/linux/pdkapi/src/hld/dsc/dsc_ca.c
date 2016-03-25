#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hld/adr_hld_dev.h>
#include <hld/dsc/adr_dsc.h>

#ifndef ALI_DSC_HLD_CA_DBG
#define ALI_DSC_HLD_CA_PRF(...) do{}while(0)
#else
#define ALI_DSC_HLD_CA_PRF(fmt, args...)  \
					do { \
							ADR_DBG_PRINT(DSC_HLD_CA, fmt, ##args); \
					} while(0)

#endif

static struct csa_key_table
{
   UINT32 handle ;
   UINT32 pid ;
   UINT16 used_status;
   UINT16 reserved; 
   struct CSA_KEY key ;
}csa_table[16];

static UINT32 check_pid_used(UINT16 pid)
{
    UINT8 i ;
    UINT32 ret = 0xFF ;
    for(i = 0;i < 5;i ++)
    {
        if( (pid == csa_table[i].pid) && csa_table[i].used_status)
        {
            ret =i ;
            break ;
        }
    }
    return ret ;
}


static UINT32 check_free_csa_pos(UINT8 *free_pos)
{
    UINT8 i ;
    UINT32 ret = 0 ;
    for(i=0;i<5;i++)
    {
        if(csa_table[i].used_status == 0)
        {
            *free_pos =i ;
            ret= 1; 
            break ;
        }
    }
	
    return ret ;
}


/***
* Clear the configuratiion of the DSC channel by PID
* Input Param: The DSC channel pid
* Output Param:
*  	   ----RET_SUCCESS:  The DSC channel is working
*      ----RET_FAILURE:  The DSC channel is free
*/
RET_CODE clear_dsc_chan_used(UINT16 pid)
{
	static pCSA_DEV pCsaDev = NULL;
	UINT8 idx = 0;
	RET_CODE  ret = 1; //RET_FAILURE;


#ifdef DSC_CA_DEBUG
	UINT8 i =0;

	for( i =0; i<5; i++)
	{
		ALI_DSC_HLD_CA_PRF(" Group_ID[%d], pid[0x%x], Status[%d] \n",\
	      		i,csa_table[i].pid, csa_table[i].used_status);
	} 

	ALI_DSC_HLD_CA_PRF("\n ");
#endif


	idx = check_pid_used(pid);
	if(0xFF != idx)
	{
		if (!pCsaDev)
			pCsaDev = (pCSA_DEV)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);

		if(pCsaDev ==NULL)
		{
			ALI_DSC_HLD_CA_PRF("No pCsaDev\n");    
			ret = RET_FAILURE;  
			goto RETURN;
		}

		ret = csa_ioctl(  pCsaDev ,IO_DELETE_CRYPT_STREAM_CMD , (UINT32)(csa_table[idx].handle));
		if(ret != RET_SUCCESS)
		{
			ALI_DSC_HLD_CA_PRF("CSA Delete CRYPT Stream fail!! idx[%d] \n", idx);
			ret = RET_FAILURE ;
			goto RETURN;
		}
		else
		{
			csa_table[idx].handle = 0xFFFF;
			csa_table[idx].pid = 0xFFFF;
			csa_table[idx].used_status = 0;
			csa_table[idx].reserved = 0;
			memset(&(csa_table[idx].key), 0xff, sizeof(struct CSA_KEY));
		}
	            
		ret = RET_SUCCESS;
		goto RETURN;
	}

RETURN:
	return ret ;
}


/*
	cw: 
		8byte clear cw
	cw_type:
		1.default cw.
		2.even cw.
		3.odd cw.
	stream_id:
		stream id
	pid:
		ts pid.
*/
static int dsc_update_csa_cw(pCSA_DEV pCsaDev, UINT8 stream_id, \
				UINT16 pid, UINT8 cw_type, UINT8 * cw);
int dsc_cfg_cw( UINT16 pid, UINT8 stream_id, \
				UINT8 cw_type, UINT8 * cw)
{
	 static pCSA_DEV pCsaDev = NULL;
	 static UINT8 csa_initialized = 0;
	 CSA_INIT_PARAM param ;
	 int ret = RET_SUCCESS;
	 
	if (!pCsaDev)
		pCsaDev = (pCSA_DEV)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);
	
	if(!pCsaDev)
		return RET_FAILURE;
		
	if (csa_initialized != 1)
	{
		param.dma_mode = TS_MODE ;
		param.key_from = KEY_FROM_SRAM ;
		param.parity_mode = AUTO_PARITY_MODE0 ;
		param.pes_en = 1; 			 /*not used now*/
		param.scramble_control = 0 ; 	/*dont used default CW*/
		param.stream_id = stream_id;
		param.version = CSA2;
		ret = csa_ioctl( pCsaDev ,IO_INIT_CMD , (UINT32)&param);
		if(ret != RET_SUCCESS)
		{
			ALI_DSC_HLD_CA_PRF("IO_INIT_CMD fail\n");
			return ret;
		}

		csa_initialized = 1;
	}

	ret = dsc_update_csa_cw(pCsaDev,stream_id, pid, cw_type, cw);
	if(ret != RET_SUCCESS)
	{
		ALI_DSC_HLD_CA_PRF("update CW failed!\n");
	}

	return ret;
}

/*
	cw: 
		8byte clear even/odd cw
	cw_type:
		1.default cw.
		2.even cw.
		3.odd cw.
	stream_id:
		stream id
	pid:
		ts pid.
*/
int dsc_cfg_csa_cw(pCSA_DEV pCsaDev, UINT16 pid, 
					UINT8 stream_id,UINT8 cw_type,
					UINT8 *cw)
{
	 CSA_INIT_PARAM param ;
	 int ret = RET_SUCCESS;
	 
	if(!pCsaDev)
		return RET_FAILURE;
		
	memset(&param,0x00,sizeof(CSA_INIT_PARAM));
	{
		param.dma_mode = TS_MODE ;
		param.key_from = KEY_FROM_SRAM;
		param.parity_mode = AUTO_PARITY_MODE0 ;
		param.scramble_control = 0 ; 	/*dont used default CW*/
		param.stream_id = stream_id;
		param.version = CSA2;
		ret = csa_ioctl(pCsaDev ,IO_INIT_CMD , (UINT32)&param);
		if(ret != RET_SUCCESS)
		{
			ALI_DSC_HLD_CA_PRF("IO_INIT_CMD fail\n");
			return ret;
		}
	}

	ret = dsc_update_csa_cw(pCsaDev,stream_id, pid, cw_type, cw);
	if(ret != RET_SUCCESS)
	{
		ALI_DSC_HLD_CA_PRF("update CW failed!\n");
	}

	return ret;
}

static int dsc_update_csa_cw(pCSA_DEV pCsaDev,UINT8 stream_id, \
								UINT16 pid, UINT8 cw_type, \
								UINT8 *cw)
{
	UINT8 pos ;
	KEY_PARAM CsaParamList ;
	int  ret = RET_SUCCESS;

	memset(&CsaParamList,0,sizeof(KEY_PARAM));
	pos = check_pid_used(pid) ;
	if(pos != 0xFF) /*used*/
	{
		CsaParamList.handle =  csa_table[pos].handle ;
		CsaParamList.pid_len = 1;
		CsaParamList.force_mode= 0 ;
		CsaParamList.pid_list = &pid;
		CsaParamList.key_length = 64;
		if(cw_type == 2) /*even key*/  
			memcpy(csa_table[pos].key.EvenKey,cw,8);
		if(cw_type == 3) /*odd key*/
			memcpy(csa_table[pos].key.OddKey,cw,8);
		CsaParamList.stream_id = stream_id ;
		CsaParamList.p_csa_key_info = (void *)&csa_table[pos].key ;
		ret = csa_ioctl(pCsaDev,IO_KEY_INFO_UPDATE_CMD,(UINT32)&CsaParamList);
		if(ret != RET_SUCCESS)
		{
			ALI_DSC_HLD_CA_PRF("CSA update key fail\n");
			ret = RET_FAILURE ;
		}
		ALI_DSC_HLD_CA_PRF("CSA update: handle %d\n",CsaParamList.handle);
	}
	else
	{
		if(check_free_csa_pos(&pos)) /*found it*/
		{
			csa_table[pos].used_status = 1 ;
			csa_table[pos].pid = pid & 0x1FFF ;

			CsaParamList.ctr_counter = NULL ;
			CsaParamList.force_mode = 0;
			CsaParamList.handle = 0xFF ;
			CsaParamList.init_vector = NULL ;
			CsaParamList.key_length = 64 ;
			CsaParamList.pid_len = 1 ;
			CsaParamList.pid_list = &pid;
			if(cw_type == 2) /*even key*/  
				memcpy(csa_table[pos].key.EvenKey,cw,8);
			if(cw_type == 3) /*odd key*/
				memcpy(csa_table[pos].key.OddKey,cw,8);
			CsaParamList.stream_id = stream_id ;
			CsaParamList.p_csa_key_info =  (void *)&csa_table[pos].key;
			ret = csa_ioctl(pCsaDev,IO_CREAT_CRYPT_STREAM_CMD, \
							(UINT32)&CsaParamList);
			if(ret != RET_SUCCESS)
			{
				ALI_DSC_HLD_CA_PRF("CSA creat stream fail\n");
				csa_table[pos].used_status = 0;
				ret = RET_FAILURE ;
			}
			else
				csa_table[pos].handle = CsaParamList.handle ;
			ALI_DSC_HLD_CA_PRF("CSA create: handle %d\n",CsaParamList.handle);
		}
		else
		{
			ALI_DSC_HLD_CA_PRF("CSA table is full\n");
			ret = RET_FAILURE;
		}
	}
	
	return ret ;
}

int dsc_delete_csa_cw(pCSA_DEV pCsaDev, UINT16 pid)
{
	UINT8 pos ;
	int  ret = RET_SUCCESS;

	pos = check_pid_used(pid) ;
	if(pos != 0xFF) /*used*/
	{
		ALI_DSC_HLD_CA_PRF("handle %d\n",csa_table[pos].handle);
		ret = csa_ioctl(pCsaDev,IO_DELETE_CRYPT_STREAM_CMD, \
						csa_table[pos].handle);
		if(ret != RET_SUCCESS)
		{
			ALI_DSC_HLD_CA_PRF("CSA IO_DELETE_CRYPT_STREAM_CMD fail\n");
			ret = RET_FAILURE ;
		}
		memset(&csa_table[pos],0x00,sizeof(struct csa_key_table));
	}
	else
	{
		ret = RET_FAILURE ;
		ALI_DSC_HLD_CA_PRF("pid not found in table\n");
	}
	
	return ret ;
}

/* 
   crypt_mode -> DSC_ENCRYPT encrypt
   crypt_mode -> DSC_DECRYPT decrypt 
*/
int aes_pure_ecb_crypt( UINT8 *key, UINT8 *input, \
							UINT8 *output, UINT32 length, \
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
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
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

DONE3:
	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}

/* 
   crypt_mode -> DSC_ENCRYPT encrypt
   crypt_mode -> DSC_DECRYPT decrypt 
*/
int aes_pure_cbc_crypt( UINT8 *key, UINT8 *iv, \
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

DONE3:
	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}

/* 
   crypt_mode -> DSC_ENCRYPT encrypt
   crypt_mode -> DSC_DECRYPT decrypt 
*/
int aes_pure_ctr_crypt( UINT8 *key, UINT8 *ctr, \
							UINT8 *input, UINT8 *output, \
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

DONE3:
	aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
	dsc_set_stream_id_idle(stream_id);
DONE1:
	dsc_set_sub_device_id_idle(AES, device_id);
	return ret;
}

