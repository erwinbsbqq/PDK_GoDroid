/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: rsa_key_api.c
*   
*    Description: 
*       
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#include <ali/basic_types.h>
#include <ali/retcode.h>
#include <ali/sys_define.h>
#include <ali/chunk.h>
#include <ali/dsc.h>
#include <ali/otp.h>
#include <ali/hld_dev.h>
#include <ali/sto_flash.h>
#include <ali/string.h>
#include <ali/flash_cipher.h>
#include <ali/rsa_verify.h>
#include "flash_enc.h"
#include <ali/ali_param.h>

#define RSA_PRINTF 1
#define RSA_DEBUG_EN 1
#define RSA_DATA_DEBUG_EN 1

#ifdef RSA_PRINTF
#undef RSA_PRINTF
#endif


#ifdef RSA_DEBUG
#undef RSA_DEBUG
#endif

#ifdef RSA_DUMP
#undef RSA_DUMP
#endif

#if RSA_DEBUG_EN
#define RSA_PRINTF          libc_printf
#else
#define RSA_PRINTF(...)     do{}while(0)
#endif

#if RSA_DATA_DEBUG_EN
#define RSA_DEBUG           libc_printf
#define RSA_DUMP(data,len) { const int l=(len); int i;\
                             for(i=0 ; i<l ; i++) {RSA_DEBUG("0x%02x,",(*(data+i))); \
                             if((i+1)%16==0) RSA_DEBUG("\n");}\
                             RSA_DEBUG("\n"); }
#else
#define RSA_DEBUG(...)           do{}while(0)
#define RSA_DUMP(data,len)  do{}while(0)
#endif

R_RSA_PUBLIC_KEY g_rsa_public_key;
R_RSA_PUBLIC_KEY g_u_rsa_public_key;
static UINT32 get_flag = 0;
static R_RSA_PUBLIC_KEY g_rsa_public_key_bak;

/*align for hmac calculate buffer */
static HMAC_PARAM k_buffer __attribute__((  aligned( 16 )));

/*
*get_parameter_from_bl() - get the parameter from Bootloader. It should be 
* called in Firmware initialization phase.
*@type_flag:
*      0:  get system software public key, 
*           store the public key in the globe paramter
*   others: reserved
*@get_mode: reseved
*/
RET_CODE get_parameter_from_bl(UINT32 type_flag, UINT32 *get_mode, void *param_from_bl) 
{
    UINT32 magic_num=0;
    UINT32 params_addr=0;
    TRANSFER_PARAMS *p_trans;
    magic_num=readl(STAGE_TRANSFER_ADDR0);
    params_addr=readl(STAGE_TRANSFER_ADDR1);

    RSA_DEBUG("%s (%x,%x)\n", __FUNCTION__, magic_num, params_addr); 
    p_trans = (TRANSFER_PARAMS *)params_addr;
    if(0 == get_flag)
    {
        if(ALI_TRANSFER_MAGIC_NUM == magic_num)
        {
            MEMCPY(param_from_bl, (UINT8 *)(p_trans), sizeof(TRANSFER_PARAMS));
            MEMCPY((UINT8 *)(&g_rsa_public_key), (UINT8 *)((&p_trans->public_key1)), sizeof(R_RSA_PUBLIC_KEY));
        		get_flag = 1;
        }        
    }
    if(0x800==g_rsa_public_key.bits)
        return RET_SUCCESS;    
    else
        return RET_FAILURE;
}

/*set_parameter_from_bl() - Store software PK to firmware by specify DRAM space.
* This function should be called by Bootloader.
UINT32 flag:     reserved
UINT32 *param: reserved
*/
RET_CODE set_parameter_from_bl(UINT32 flag, UINT32 *param)
{
    UINT32 ret = RET_FAILURE;
    UINT8 *temp_buff=NULL;
    TRANSFER_PARAMS *p_trans=NULL;

    ret = fetch_sys_pub_key(0);
    if(RET_SUCCESS!=ret)
        return RET_FAILURE;

    temp_buff = (UINT8 *)(MALLOC(TRANS_PARAMS_MAX_SIZE));
    p_trans = (TRANSFER_PARAMS *)(temp_buff);

    RSA_DEBUG("%s (%x,%x)\n", __FUNCTION__, &g_rsa_public_key, temp_buff); 
    if((NULL!=&p_trans->public_key1)&&(0x800== g_rsa_public_key.bits))
    {
        writel(STAGE_TRANSFER_ADDR0, ALI_TRANSFER_MAGIC_NUM);
        writel(STAGE_TRANSFER_ADDR1, temp_buff);
        MEMCPY((UINT8 *)((&p_trans->public_key1)),(UINT8 *)(&g_rsa_public_key), sizeof(R_RSA_PUBLIC_KEY));
    }
    else
    {
        RSA_PRINTF("pass key to next stage fail!\n"); 
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

/*Redundant function for secure excuse.
*/
static void illegal_rsa_func1(void) 
{   
    hw_watchdog_reboot();
    do{}while(1);
}


/*fetch_sys_kernel_pub_key() - Store the linux system software public key
* to the target memory space. BL pass the pk to the u-boot
* by calling this function before entering u-boot.
*
*@target: The target memory address by 0x8xxx_xxxx
*@Return: 0:  success
*         -1  fail
*/
RET_CODE fetch_sys_kernel_pub_key(UINT32 target)
{
    struct sto_device *flash_dev =  NULL;
    UINT32 bl_offset = 0;
    UINT32 key_offset = 0 ;
    UINT32 offset = 0 ;
    INT32 ret = -1 ;     

    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if (NULL == flash_dev)
    {
        return -1;
    }
    bl_offset = sto_fetch_long(0 + CHUNK_OFFSET);
    if(is_bl_encrypted_enable())
    {
        key_offset = FLASH_MIRROR_BASE + bl_offset-SYS_KERNEL_PUB_KEY_OFFSET ;
        MEMCPY((UINT8 *)target, (UINT8 *)key_offset, sizeof(R_RSA_PUBLIC_KEY));   
        key_offset = (FLASH_MIRROR_BASE + bl_offset-SYS_SW_PUB_KEY_OFFSET_M3701) ;
        MEMCPY((UINT8 *)(target +sizeof(R_RSA_PUBLIC_KEY) ), (UINT8 *)key_offset, sizeof(R_RSA_PUBLIC_KEY));        
    }    
    else      
    {   
        ret = sto_get_data(flash_dev, (UINT8*)target,(bl_offset - SYS_KERNEL_PUB_KEY_OFFSET),sizeof(R_RSA_PUBLIC_KEY));
        
        if(sizeof(R_RSA_PUBLIC_KEY) !=ret )
        {
            RSA_PRINTF("%s sys kernel pk failed!\n",__FUNCTION__);    
            return -1 ;
        }
        offset = (bl_offset - SYS_SW_PUB_KEY_OFFSET_M3701);
        ret = sto_get_data(flash_dev, (UINT8*)(target+sizeof(R_RSA_PUBLIC_KEY)), offset, sizeof(R_RSA_PUBLIC_KEY) );
        if(sizeof(R_RSA_PUBLIC_KEY) != ret)
        {
             RSA_PRINTF("%s sys sw pk failed!\n",__FUNCTION__);    
             return -1 ;
        }
    }
    return 0;    
}

static void illegal_rsa_func2(void) 
{   
    hw_watchdog_reboot();
    do{}while(1);
}

RET_CODE reset_rsa_pub_key(void)
{
	MEMCPY((UINT8 *)(&g_rsa_public_key),(UINT8 *)(&g_rsa_public_key_bak), sizeof(R_RSA_PUBLIC_KEY));
	if(0 == g_rsa_public_key.bits) //don't get the correct public key
	{
            return RET_FAILURE;
	}
	else
	{
	     return RET_SUCCESS;
	}
}

/*
* fetch_sys_pub_key() -  get the system sw public key from target.
* The bootloader shall call it before signature verification.
*@target_flag:
*       0:  from bootloader
*       1:  from OTP 0x4~0x4C
*/
RET_CODE fetch_sys_pub_key(UINT32 target_flag) 
{
    UINT32 boot_type      = 0;
    UINT32 bl_offset      = 0;
    UINT32 flash_offset   = 0;
    UINT32 pub_key_addr   = 0;
    UINT32 store_key_addr = 0;
    INT32 ret = -1 ;
    UINT8 temp_key[OTP_PUBKEY_SIZE] = {0};
    UINT32 pub_key_off = 0;	
    UINT16 i = 0;
    UINT16 zerokey=TRUE;
    UINT32 mem_start = 0;

    if(1 == target_flag)
    {
        if(1 == get_flag)	//backup sys sw rsa public key
        {
        	MEMCPY((UINT8 *)(&g_rsa_public_key_bak),(UINT8 *)(&g_rsa_public_key), sizeof(R_RSA_PUBLIC_KEY));
        	RSA_PRINTF("backup sys sw public key\n");
        }
        ret=otp_init(NULL);
        if(ret < 0)
        {
            RSA_PRINTF("otp init fail!\n");
            return RET_FAILURE; 
        }    
        ret = otp_read(OTP_PUBKEY_ADDR, (UINT8 *)(&temp_key), OTP_PUBKEY_SIZE);
        if(ret != OTP_PUBKEY_SIZE)
        {
            RSA_PRINTF("otp read fail!\n");
            return RET_FAILURE; 
        }    
        for(i=0;i<OTP_PUBKEY_SIZE;i++) 
        {
            if(temp_key[i])
            {
                zerokey=FALSE;
                break;
            }
        }
        if(zerokey)
        {
            return RET_FAILURE;
        }
        MEMSET((UINT8 *)&g_rsa_public_key,0x00,sizeof(R_RSA_PUBLIC_KEY));
        MEMCPY((UINT8 *)&g_rsa_public_key,temp_key, OTP_PUBKEY_SIZE-LAST_EXP_SIZE);
        MEMCPY((UINT8 *)&g_rsa_public_key+OTP_LAST_EXP_OFFSET_FULL, temp_key+OTP_LAST_EXP_OFFSET, LAST_EXP_SIZE);
        get_flag = 1;
    }
    else if(0 == target_flag)
    {
        if(0 == get_flag)
        {
            if(sys_ic_get_chip_id()== ALI_S3281)
                pub_key_off = SYS_SW_PUB_KEY_OFFSET_M3281;
            else
                pub_key_off = SYS_SW_PUB_KEY_OFFSET_M3701;      

            if(sys_ic_get_secure_flash_map(0) == 2)
            {
                mem_start = sys_ic_get_boot_map_addr(0);
                bl_offset = *(UINT32 *)(mem_start+0x8); //offset of the scs_total_area
                pub_key_addr =  mem_start + bl_offset-pub_key_off;
                MEMCPY((UINT8 *)&g_rsa_public_key, (UINT8 *)(pub_key_addr), sizeof(R_RSA_PUBLIC_KEY));
            }
            else
            {
                boot_type = sys_ic_current_boot_type();
                RSA_PRINTF("fetch_sys_pub_key, boot_type = %d\n", boot_type);
                if (boot_type)
                { /* Nand Boot */
                    flash_offset = FLASH_MIRROR_BASE+CHUNK_OFFSET;
                    bl_offset = GET_FLASH_DATA(((UINT8 *)flash_offset));
                }
                else
                { /* Nor Boot */
                    flash_offset = FLASH_BASE+CHUNK_OFFSET;
                    bl_offset = GET_FLASH_DATA(((UINT8 *)flash_offset));
                }

                if(is_bl_encrypted_enable())
                {
                    pub_key_addr =  FLASH_MIRROR_BASE + bl_offset-pub_key_off;
                    MEMCPY((UINT8 *)&g_rsa_public_key, (UINT8 *)(pub_key_addr), sizeof(R_RSA_PUBLIC_KEY));
                }
                else      
                {
                    pub_key_addr =  FLASH_BASE+bl_offset-pub_key_off;
                    store_key_addr = FLASH_MIRROR_BASE + bl_offset-pub_key_off;

                    MEMCPY((UINT8 *)&g_rsa_public_key,(UINT8 *)(pub_key_addr), sizeof(R_RSA_PUBLIC_KEY));
                    /*store the public key into memory, 
                        pass the key to system software*/
                    MEMCPY((UINT8 *)(FLASH_MIRROR_BASE), (UINT8 *)(FLASH_BASE), CHUNK_HEADER_SIZE);
                    
                    MEMCPY((UINT8 *)(store_key_addr),(UINT8 *)(pub_key_addr), sizeof(R_RSA_PUBLIC_KEY));
                }
            }
            /*don't get the correct public key*/
            if(0 == g_rsa_public_key.bits)
            {
                return RET_FAILURE;
            }
            get_flag = 1;
        }
    }
    return RET_SUCCESS;    
}

/*fetch and system public key and copy it to the argument PK
*@target_flag:
*       0:  from bootloader
*       1:  from OTP 0x4~0x4C
*/
RET_CODE get_sys_pub_key(PK_POS target, UINT8 *pk,UINT32 len)
{
    RET_CODE ret = RET_FAILURE ;
       
    ret = fetch_sys_pub_key(target);
    if(ret != RET_SUCCESS )
    {
        RSA_DEBUG("fetch_pub_key failed\n");
        return RET_FAILURE;
    }
    if( (pk != NULL) && (len >= sizeof(g_rsa_public_key)))
      MEMCPY(pk, (UINT8 *)(&g_rsa_public_key),sizeof(g_rsa_public_key));
    return RET_SUCCESS ;
}

/*
*create_sha_ramdrv() - calculate the input digest. 
*@input:  the point of input buffer
*@length: the length of input data
*@output: the output digest
*/
RET_CODE create_sha_ramdrv(UINT8 *input, UINT32 length, UINT32 *output)
{
    RET_CODE ret = RET_FAILURE;

    ret = ali_sha_digest(input,length,SHA_SHA_256,(UINT8 *)output);
    if(ret!=RET_SUCCESS)
        RSA_PRINTF("RSA ERROR: sha digest error!\n");
    return ret;
}

/*
*ali_xor() - xor the data from dst and src, the output in the dst
*@dst: the target buffer for xor
*@src: the xor data
*@len: data length
*/
static void ali_xor(UINT8 *dst, UINT8 *src, UINT32 len)
{
    UINT32 i = 0;

    for(i=0;i<len;i++)
    {
        dst[i] = dst[i]^src[i];
    }
}
/*
* calculate_hmac() - calculate the input data HMAC
* @input : input data buffer
* @length: input data length
* @output : output the HMAC value
* @key : HAMC salt key
*
* note: KEY_1_2 is the PVR tmp. this function is only used in PVR now!
* The function use the hmac arithmetic to verify the input text.
* please refer to the fips-198a.pdf
*/
RET_CODE calculate_hmac(unsigned char *input, unsigned long length, unsigned char *output, unsigned char *key)
{
    RET_CODE ret = RET_FAILURE;
    const UINT32 ipad = 0x36;
    const UINT32 opad = 0x5c;
    UINT8 *temp_buff = NULL; 
    UINT8 *temp = NULL;
    UINT32 chip_id = 0;

    if((!access_ok(input,length)) || (!access_ok(output,HMAC_OUT_LENGTH)) )
    {
        RSA_PRINTF("Invalid memory!\n");
        return -1;   
    }  

    /*1, prepare k0, use the dsc decrypt the R2 to get the k0*/
    MEMSET(&k_buffer,0,sizeof(HMAC_PARAM));
    MEMCPY(k_buffer.k0,key,FIRST_KEY_LENGTH);
    if(OTP_DW_LEN!=otp_read(0, (UINT8 *)(&chip_id), OTP_DW_LEN))
    {
        RSA_PRINTF("get chip_id fail!\n");
        return RET_FAILURE;
    }
    MEMCPY(&k_buffer.k0[FIRST_KEY_LENGTH], &chip_id,OTP_DW_LEN);
    RSA_PRINTF("K0:\n");
    RSA_DUMP(k_buffer.k0,HASH_BLOCK_LENGTH);    
    // 2, k0 xor ipad 
    MEMSET(k_buffer.ipad,ipad,HASH_BLOCK_LENGTH);
    ali_xor(k_buffer.k0,k_buffer.ipad,HASH_BLOCK_LENGTH);
    RSA_PRINTF("K0:\n");
    RSA_DUMP(k_buffer.k0,HASH_BLOCK_LENGTH);
    // 3, (k0 xor ipad) || text
    temp_buff = (UINT8*)((UINT32)MALLOC(length+0xf+HASH_BLOCK_LENGTH));
    if(NULL == temp_buff)
    {
        RSA_PRINTF("%s: out of memory\n",__FUNCTION__);
        return RET_FAILURE;
    }
    temp = (UINT8 *)(0xFFFFFFF8 & (UINT32)temp_buff);    
    MEMCPY(temp,k_buffer.k0,HASH_BLOCK_LENGTH);
    MEMCPY(&temp[HASH_BLOCK_LENGTH],input,length);

    // 4, Hash((k0 xor ipad) || text)
    ret = ali_sha_digest(temp,(length+HASH_BLOCK_LENGTH), SHA_SHA_256,k_buffer.hout);
    if(ret!=RET_SUCCESS)
    {
        RSA_PRINTF("RSA ERROR: sha digest error!\n");
        FREE(temp_buff);
        return RET_FAILURE;
    }
    // 5, k0 xor opad
    ali_xor(k_buffer.k0,k_buffer.ipad,HASH_BLOCK_LENGTH);  
    MEMSET(k_buffer.opad,opad,HASH_BLOCK_LENGTH);
    ali_xor(k_buffer.k0,k_buffer.opad,HASH_BLOCK_LENGTH);    

    // 6, (k0 xor opad) || Hash((k0 xor ipad) || text)
    MEMCPY(temp, k_buffer.k0, HASH_BLOCK_LENGTH);
    MEMCPY(&temp[HASH_BLOCK_LENGTH],k_buffer.hout,HASH_BLOCK_LENGTH);

    // 7, Hash((k0 xor opad) || Hash((k0 xor ipad) || text))
    ret = ali_sha_digest(temp,(2*HASH_BLOCK_LENGTH)
                            ,SHA_SHA_256,k_buffer.hout);
    if(ret!=RET_SUCCESS)
    {
        RSA_PRINTF("RSA ERROR: sha digest error!\n");
        FREE(temp_buff);
        return RET_FAILURE;
    }
    MEMCPY(output,k_buffer.hout,HMAC_OUT_LENGTH);
    FREE(temp_buff);
    RSA_DUMP(output,HMAC_OUT_LENGTH);
    RSA_PRINTF("HMAC done, ret: %d!\n", ret);
    return ret;
}

static void illegal_rsa_func3(void) 
{  
    hw_watchdog_reboot();
    do{}while(1);
}
