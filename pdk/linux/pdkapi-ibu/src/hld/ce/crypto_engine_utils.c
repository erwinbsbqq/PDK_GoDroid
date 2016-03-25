#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <hld/crypto/crypto.h>

#ifndef ALI_CE_HLD_UTILS_DBG
#define ALI_CE_HLD_UTILS_PRF(...) do{}while(0)
#define ALI_CE_HLD_UTILS_PRF(fmt, arg...) \
    do { \
        printf("CE_UTILS: In %s "fmt, __func__, ##arg); \
    } while (0)
#endif


/* This function is used to load otp root key from fixed otp address
key_pos:  OTP key pos
KEY_0_0,KEY_0_1,KEY_0_2,KEY_0_3
*/
int ce_load_otp_key(UINT32 key_pos)
{
	CE_DEVICE *pCeDev = (CE_DEVICE *)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	OTP_PARAM opt_info;
	
	memset((UINT8 *)&opt_info, 0x00, sizeof(OTP_PARAM));
	opt_info.otp_addr = OTP_ADDESS_1+4*key_pos;//AES_HW_M2M2_KEY_OTP_ADDR;
	opt_info.otp_key_pos = key_pos; //AES_HW_M2M2_KEY_POS;
	
	if (RET_SUCCESS != ce_key_load(pCeDev, (OTP_PARAM *)(&opt_info)))
	{
		ALI_CE_HLD_UTILS_PRF("load OTP key failed!");
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
int ce_generate_key_by_aes(UINT8 *key, UINT32 first_key_pos, UINT32 second_key_pos, UINT32 ce_crypt_select)
{
	pCE_DEVICE pCeDev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO ce_data_info;

	memset(&ce_data_info, 0x00, sizeof(CE_DATA_INFO));
	memcpy(ce_data_info.data_info.crypt_data, key, AES_CE_KEY_LEN);	
	ce_data_info.data_info.data_len 			= AES_CE_KEY_LEN;	
	ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_AES; 	
	ce_data_info.des_aes_info.crypt_mode 		= ce_crypt_select; 	
	ce_data_info.des_aes_info.des_low_or_high	= 0;			
	ce_data_info.key_info.first_key_pos 		= first_key_pos;
	ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	ce_data_info.key_info.second_key_pos 		= second_key_pos; 
	if (ce_generate_single_level_key(pCeDev, (CE_DATA_INFO *)(&ce_data_info)) != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("generate AES Key fail!\n");
		return -1;
	}
	return 0;
}


 /* 
 	AES 128bit decrypt 3 level (3x16Bytes) data to key ladder's secure SRAM
	input -> data to be decrypt
	root_pos -> root pos in key ladder(OTP_KEY_0_0 OTP_KEY_0_1 OTP_KEY_0_2 OTP_KEY_0_3)
	key_pos -> level_three key_pos in key ladder, will return to caller for other use
	if this key_pos won't be used anymore, need to set it idle: 
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 */
int aes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT8 *key_pos)
{
	int ret;
	pCE_DEVICE pCeDev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO ce_data_info;
	CE_FOUND_FREE_POS_PARAM key_pos_param;

	//generate Level 1 key with decrypt raw data 1
	ret = ce_generate_key_by_aes(input,root_pos,KEY_1_0+root_pos,CE_IS_DECRYPT);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: level one failed!\n");
		return RET_FAILURE;
	}

	//generate Level 2 key with decrypt raw data 2
	ret = ce_generate_key_by_aes(&input[16],root_pos+KEY_1_0,(KEY_1_0+root_pos)*2,CE_IS_DECRYPT);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: level two failed!\n");
		return RET_FAILURE;
	}


	//find a free level 3 key_pos
	memset(&key_pos_param, 0x00, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = THREE_LEVEL;
	key_pos_param.pos = INVALID_ALI_CE_KEY_POS;
	key_pos_param.root = root_pos;
	ret = ce_ioctl(pCeDev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: find free key pos fail!\n");
		return RET_FAILURE;
	}
	
	//generate Level 3 key with decrypt raw data 3
	ret = ce_generate_key_by_aes(&input[32],(KEY_1_0+root_pos)*2,key_pos_param.pos,CE_IS_DECRYPT);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: level three failed!\n");
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return RET_FAILURE;
	}

	*key_pos = key_pos_param.pos;

	ALI_CE_HLD_UTILS_PRF("key_pos is %d\n",*key_pos);
	return RET_SUCCESS;
}


 /* 
 	TDES 128bit decrypt eck to key ladder's secure SRAM  -> to get ck
	eck -> data to be decrypt
	level_one -> key in CE
	level_gen -> pos of ck in CE
 */
int tdes_decrypt_key_to_ce_one_level(UINT8 *eck, UINT8 level_one, enum CE_CRYPT_TARGET level_gen)
{
	CE_DATA_INFO ce_data_info;

	pCE_DEVICE pCeDev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);

	memset(&ce_data_info, 0x00, sizeof(ce_data_info));
	ce_data_info.data_info.data_len             = 8;			/* aes is 16 bytes des/tdes is 8 bytes*/
	ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_DES; 	/* select AES or DES module*/
	ce_data_info.des_aes_info.crypt_mode 		= CE_IS_DECRYPT; 	
	ce_data_info.des_aes_info.des_low_or_high 	= 1;	            //HIGH_ADDR, then LOW_ADDR;  /* for AES it should be LOW_ADDR */
	ce_data_info.key_info.first_key_pos         = level_one ;
	ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	ce_data_info.key_info.second_key_pos 		= level_gen;
	memcpy(ce_data_info.data_info.crypt_data,&eck[0],8);

	if(RET_SUCCESS != ce_generate_single_level_key(pCeDev, &ce_data_info))
	{
		ALI_CE_HLD_UTILS_PRF("ce_generate_single_level_key failed high\n");
		return RET_FAILURE;
	}

	ce_data_info.des_aes_info.des_low_or_high 	= 0;	//LOW_ADDR				
	memcpy(ce_data_info.data_info.crypt_data,&eck[8],8);
	if(RET_SUCCESS != ce_generate_single_level_key(pCeDev, &ce_data_info))
	{
		ALI_CE_HLD_UTILS_PRF("ce_generate_single_level_key failed low\n");
		return RET_FAILURE;
	}

	return RET_SUCCESS;
}

int tdes_decrypt_key_to_ce_64bit(UINT8 *eck, UINT8 level_one, enum CE_CRYPT_TARGET level_gen, UINT8 addr)
{
	CE_DATA_INFO ce_data_info;

	pCE_DEVICE pCeDev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);

	memset(&ce_data_info, 0, sizeof(ce_data_info));
	ce_data_info.data_info.data_len             = 8;				/* aes is 16 bytes des/tdes is 8 bytes */
	ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_DES; 	/* select AES or DES module */
	ce_data_info.des_aes_info.crypt_mode 		= CE_IS_DECRYPT; 	
	ce_data_info.des_aes_info.des_low_or_high 	= addr;	            //HIGH_ADDR, then LOW_ADDR; 
	ce_data_info.key_info.first_key_pos         = level_one ;
	ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	ce_data_info.key_info.second_key_pos 		= level_gen;
	memcpy(ce_data_info.data_info.crypt_data,&eck[0],8);

	if(RET_SUCCESS != ce_generate_single_level_key(pCeDev, &ce_data_info))
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
int tdes_decrypt_to_ce_two_level(UINT8 *input,UINT8 level_root,UINT8 *key_pos)
{
	int ret;
	pCE_DEVICE pCeDev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO ce_data_info;
	CE_FOUND_FREE_POS_PARAM key_pos_param;

	//generate Level 1 key with decrypt raw data 1
	ret = tdes_decrypt_key_to_ce_one_level(input,level_root,KEY_1_0+level_root);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: level one failed!\n");
		return RET_FAILURE;
	}

	//find level two pos
	memset(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = TWO_LEVEL;
	key_pos_param.pos = INVALID_ALI_CE_KEY_POS;
	key_pos_param.root = level_root;
	ret = ce_ioctl(pCeDev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: find free key pos fail!\n");
		return RET_FAILURE;
	}
	
	//generate Level 2 key with decrypt raw data 2
	ret = tdes_decrypt_key_to_ce_one_level(&input[16],KEY_1_0+level_root,key_pos_param.pos);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: level two failed!\n");
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return RET_FAILURE;
	}

	*key_pos = key_pos_param.pos;

	ALI_CE_HLD_UTILS_PRF("key_pos is %d\n",*key_pos);
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
int tdes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT8 *key_pos)
{
	int ret;
	pCE_DEVICE pCeDev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO ce_data_info;
	CE_FOUND_FREE_POS_PARAM key_pos_param;

	//generate Level 1 key with decrypt raw data 1
	ret = tdes_decrypt_key_to_ce_one_level(input,root_pos,KEY_1_0+root_pos);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: level one failed!\n");
		return RET_FAILURE;
	}

	//generate Level 2 key with decrypt raw data 2
	ret = tdes_decrypt_key_to_ce_one_level(&input[16],root_pos+KEY_1_0,(KEY_1_0+root_pos)*2);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: level two failed!\n");
		return RET_FAILURE;
	}


	//find a free level 3 key_pos
	memset(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	key_pos_param.ce_key_level = THREE_LEVEL;
	key_pos_param.pos = INVALID_ALI_CE_KEY_POS;
	key_pos_param.root = root_pos;
	ret = ce_ioctl(pCeDev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: find free key pos fail!\n");
		return RET_FAILURE;
	}

	//generate Level 3 key with decrypt raw data 3
	ret = tdes_decrypt_key_to_ce_one_level(&input[32],(KEY_1_0+root_pos)*2,key_pos_param.pos);
	if (ret != RET_SUCCESS)
	{
		ALI_CE_HLD_UTILS_PRF("Error: level three failed!\n");
		ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		return RET_FAILURE;
	}

	*key_pos = key_pos_param.pos;

	ALI_CE_HLD_UTILS_PRF("key_pos is %d\n",*key_pos);
	return RET_SUCCESS;
}

