/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: flash_cipher.c
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
#include <ali/chunk.h>
#include <ali/crypto.h>
#include <ali/sto_dev.h>
#include <ali/dsc.h>
#include <ali/otp.h>
#include <ali/flash_cipher.h>
#include <ali/rsa_verify.h>
#include "flash_enc.h"
#include <ali/hld_dev.h>

#include <ali/sto_flash.h>
#include <ali/sys_parameters.h>
#include <ali/sys_define.h>

#include <ali/string.h>

#ifdef CIPHER_PRINTF
#undef CIPHER_PRINTF
#endif
#ifdef CIPHER_DEBUG
#undef CIPHER_DEBUG
#endif
#ifdef CIPHER_DUMP
#undef CIPHER_DUMP
#endif

#if CIPHER_DEBUG_EN
#define CIPHER_PRINTF               libc_printf
#else
#define CIPHER_PRINTF(...)            do{} while(0)
#endif

#define CIPHER_DATA_DEBUG_EN 1
#if CIPHER_DATA_DEBUG_EN
#define CIPHER_DEBUG                libc_printf
#define CIPHER_DUMP(data, len) { const int l = (len); int i;\
                for (i = 0; i < l; i++) {CIPHER_DEBUG(" 0x%02x", *((data) + i)); \
                if((i+1)%16==0) CIPHER_DEBUG("\n");}\
                           CIPHER_DEBUG("\n");}
#else
#define CIPHER_DUMP(data, len)        do{} while(0)
#define CIPHER_DEBUG(...)           do{} while(0)
#endif

static UINT8 g_static_r[ALIASIX_SW_UK_LEN]={
                        0x41,0xf0,0x58,0x12,
                        0x01,0xb2,0xc3,0x0f,
                        0x8c,0x52,0x63,0x4e,
                        0x9d,0x7d,0x8a,0xf4,
                      };
UINT8 g_enc_uk[ALIASIX_SW_UK_LEN]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static UINT8 *cipher_tmp_buf_addr = NULL;
static UINT32 cipher_tmp_buf_size = 0;
UINT32 g_bl_uk_offset = 0;
static UINT32 g_sw_uk_mode=0;   // 1: means using key_0_1, 2: means using key_0_2


void flash_cipher_buf_init(UINT8 *tmp_buf_addr, UINT32 tmp_buf_size)
{
    cipher_tmp_buf_addr = tmp_buf_addr;
    cipher_tmp_buf_size = tmp_buf_size;
}

/*
* is_key_encrypted() :  get the key encrypted status 
* @offset:    The key offset in Flash
* @key_type   enum FLASH_KEY_TYPE key_type:
*                BL_FLASH_KEY =    0xFF,    //bootloader uk
*                FIRST_FLASH_KEY = 0xFE,    //sys sw uk 1
*                SECOND_FLASH_KEY = 0xFD,   //sys sw uk 2
*                THIRD_FLASH_KEY = 0xFC,    //sys sw uk 3
*                FORTH_FLASH_KEY = 0xFB     //sys sw uk 4  
* @Return : The output TRUE means key is encrypted
*            otherwise, FLASE means key is clear
*
* Check whether the universal key in extern flash encrypted or not.
*/
BOOL is_key_encrypted(UINT32 offset, enum FLASH_KEY_TYPE key_type)
{
    UINT32 uk_offset=0;
    UINT32 decrypt_flag=0;
    UINT32 i=0;
    UINT32 boot_type = 0;

    boot_type = sys_ic_current_boot_type();
    if(BL_FLASH_KEY == key_type)
    {
        if (boot_type != 1)
        {
            uk_offset = get_bl_uk_offset()+FLASH_BASE+offset+CHUNK_HEADER_SIZE;
        }
        else
        {
            uk_offset = get_bl_uk_offset()+ RAM_BASE_ADDR+offset+CHUNK_HEADER_SIZE;
        }
        decrypt_flag = *(UINT32*)(uk_offset);
        if ( BL_KEY_CIPHER_MAGIC_NUMBER == decrypt_flag)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }

    }
    else 
    {
        if (boot_type != 1)
        {//Nor boot
            uk_offset = FLASH_BASE+offset+CHUNK_HEADER_SIZE +(FIRST_FLASH_KEY-key_type)*SYS_SW_KEY_SIZE;
        }
        else
        {//Nand boot
            uk_offset = RAM_BASE_ADDR+offset+CHUNK_HEADER_SIZE +(FIRST_FLASH_KEY-key_type)*SYS_SW_KEY_SIZE;
        }
        for(i=0;i<ALIASIX_SW_UK_LEN;i+=4)
        {
            decrypt_flag = *(UINT32*)(uk_offset+i);
            if(decrypt_flag!=0)
            {
                return FALSE;
            }
        }
        return TRUE;
    }   
}

/*Redundant function for secure excuse.
* build a dummy function call.
*/
static void illegal_rsa_func1(void) 
{    
    hw_watchdog_reboot();
    do{}while(1);
}

/*
* ali_encrypt_ukey() - encrypt universal key by key buffer
* @input :  the clear input key which need encrypt
* @output : the cipher output key
* @random_num : the random number is used for generate the immediate key.
                           length of the random num need 16*n byte. if root_key is 6/7, n>=1, otherwise n>=2.
* @ key_type: which is used for root key
*  The ukey len is must be 128 bit
*/
RET_CODE ali_encrypt_ukey(UINT8 *input, UINT8 *output, UINT8 *random_num, enum CE_OTP_KEY_SEL key_type)
{
    RET_CODE ret =RET_FAILURE;
    UINT8 temp_input[ALIASIX_SW_UK_LEN];
    UINT32 key_pos=0;
    AES_KEY_LADDER_BUF_PARAM  key_ladder;   
    UINT32 kl_level = sys_ic_get_kl_key_mode();

    CE_FOUND_FREE_POS_PARAM key_pos_param;
    pCE_DEVICE pcedev = NULL;
    pcedev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);    
    CIPHER_DEBUG("enter %s (%x)\n",__FUNCTION__, key_type);

    if ((NULL == input) || (NULL == output)||(NULL==random_num))
    {
        CIPHER_PRINTF("Invalid parameter!\n");
        return -1;
    }    

     key_ladder.root_key_pos = key_type;
     key_pos=key_type+4;
     g_sw_uk_mode = key_type;


// If root key is key_0_2, use CE to do the last two level.
    if((OTP_KEY_0_2 == g_sw_uk_mode))
    {
        // Load OTP key into KL
        ret = ce_load_otp_key(key_ladder.root_key_pos);
        if(ret)
        {
            CIPHER_PRINTF("Load OTP key fail!!\n");
            return -6;
        }
        //first level key use KL
        MEMCPY(temp_input, &random_num[0], ALIASIX_SW_UK_LEN);
        ret = ce_generate_key_by_aes(temp_input,key_ladder.root_key_pos,key_pos,CE_IS_DECRYPT);
        if(ret)
        {
            CIPHER_PRINTF("generate 1 level key fail!!\n");
            return -7;
        }
        //next key level use CE
        MEMCPY(temp_input, &random_num[ALIASIX_SW_UK_LEN], ALIASIX_SW_UK_LEN);
        ret = dsc_deal_sys_uk_fw(input, output, \
                                    temp_input, ALIASIX_SW_UK_LEN,key_pos,DSC_ENCRYPT);
    }
// If root key is key_0_1, kl_level is 3, use KL do the first 3 level, the last level use CE.
//                                   kl_level is 2, use KL do the first 2 level, the last level use CE.
    else if((OTP_KEY_0_1 == g_sw_uk_mode))
    {
        MEMCPY(&key_ladder.r[0], &random_num[0],ALIASIX_SW_UK_LEN);
        MEMCPY(&key_ladder.r[ALIASIX_SW_UK_LEN], &random_num[ALIASIX_SW_UK_LEN],ALIASIX_SW_UK_LEN);
        if((3==kl_level))
        {
            key_ladder.key_ladder = 3;
            MEMCPY(&key_ladder.r[2*ALIASIX_SW_UK_LEN], g_static_r, ALIASIX_SW_UK_LEN);
        }
        else
        {
            key_ladder.key_ladder = 2;
        }
        CIPHER_DEBUG("random number:\n");
        CIPHER_DUMP(&key_ladder.r[0], 2*ALIASIX_SW_UK_LEN);
        ret = aes_generate_key_with_multi_keyladder(&key_ladder, &key_pos);
        if(RET_SUCCESS != ret)
        {
            CIPHER_PRINTF("generate content key fail!!\n");
            return -7;
        }
        //encrypt the UK.
        ret = aes_crypt_puredata_with_ce_key(input, output, \
                                        ALIASIX_SW_UK_LEN,key_pos,DSC_ENCRYPT);
    }
    else if((OTP_KEY_0_6 == g_sw_uk_mode) ||(OTP_KEY_0_7 == g_sw_uk_mode) ||(OTP_KEY_0_6_R == g_sw_uk_mode) )
    {
        ret = ali_dsc_encrypt_bl_uk(input, &random_num[0], output, (g_sw_uk_mode-OTP_KEY_0_6_R));
    }
    else
    {
        CIPHER_PRINTF("Root Key do not support\n");
    }
    if(RET_SUCCESS!=ret)
    {
        CIPHER_PRINTF("key encrypt fail,%x\n",ret);
        return -3;
    }
    return RET_SUCCESS;
}

/*
* ali_decrypt_ukey() - decrypt universal key by key buffer
* @input :  the cipher input key which need decrypt
* @key_pos : the clear output key pos in KL
* @random_num : the random number is used for generate the immediate key.
                           length of the random num need 16*n byte. if root_key is 6/7, n>=1, otherwise n>=2.
* @ key_type: which is used for root key
*  The ukey len is must be 128 bit
*/
RET_CODE ali_decrypt_ukey(UINT8 *input, UINT32 *key_pos, UINT8 *random_num, enum CE_OTP_KEY_SEL key_type)
{
    RET_CODE ret =RET_FAILURE;
    AES_KEY_LADDER_BUF_PARAM  key_ladder;    
    UINT32 kl_level = sys_ic_get_kl_key_mode();

    if ((NULL == input) || (NULL == key_pos)||(NULL==random_num))
    {
        CIPHER_PRINTF("Invalid parameter!\n");
        return -1;
    }    
    key_ladder.key_ladder = 3;
    key_ladder.root_key_pos = key_type;
    g_sw_uk_mode = key_type;

    MEMCPY(&key_ladder.r[0], &random_num[0],ALIASIX_SW_UK_LEN);
    MEMCPY(&key_ladder.r[ALIASIX_SW_UK_LEN], &random_num[ALIASIX_SW_UK_LEN],ALIASIX_SW_UK_LEN);

    if(((KEY_0_1 == g_sw_uk_mode))
        &&(3==kl_level))
    {
        MEMCPY(&key_ladder.r[2*ALIASIX_SW_UK_LEN], g_static_r, ALIASIX_SW_UK_LEN);
        MEMCPY(g_enc_uk, input, ALIASIX_SW_UK_LEN);        
    }
    else
        MEMCPY(&key_ladder.r[2*ALIASIX_SW_UK_LEN], input, ALIASIX_SW_UK_LEN);

    ret = aes_generate_key_with_multi_keyladder(&key_ladder, key_pos);
    if(RET_SUCCESS!=ret)    
    {
        CIPHER_PRINTF("%s generate key error, ret = %d!\n", __FUNCTION__, ret);
        return -2;
    }        
    return RET_SUCCESS;
}


/*
* encrypt_bl_universal_key() - encrypt bl universal key by key id
* @key_id :  The bl universal key chunk id
*            bit31~bit16:  chunk key id
*            bit15~bit0 :  reserved
* 
* Bootloader shall check the bl universal key in flash and if 
* the bl universial key is clear, Just call this function to encrypt 
* and store it to the flash. This interface do all the jobs in the procedure.
*/
RET_CODE encrypt_bl_universal_key(UINT32 key_id)
{
    RET_CODE ret =RET_FAILURE;
    UINT8 raw_data[ALIASIX_BL_UK_LEN];
    
    p_ddr_bl_key flash_keys = NULL;
    UINT8 *f_temp = NULL;

    CIPHER_DEBUG("enter %s (%x)\n",__FUNCTION__, key_id);
    //generate random data
    MEMSET(raw_data, 0, sizeof(raw_data));
    ret = trng_get_64bits(raw_data,(ALIASIX_BL_UK_LEN/8)); 
    if(RET_SUCCESS != ret)
    {
        CIPHER_PRINTF("%s generate random error!(%x)\n", __FUNCTION__, ret);
        return -1;
    }
    //encrypt with dsc
    f_temp = (UINT8*)((UINT32)MALLOC(0x100));
    if (NULL == f_temp)
    {
        CIPHER_PRINTF("MALLOC Failed!(%x)\n", ret);
        return -9;
    }    
    flash_keys = (p_ddr_bl_key)((0xFFFFFFF8&((UINT32)f_temp)));
    MEMSET(flash_keys, 0x0, sizeof(DDR_BL_KEY));
    ret = wr_flash_by_id((key_id&FLASH_CHUCK_ID_MASK), g_bl_uk_offset,  \
                        (UINT8 *)(&(flash_keys->cipher_flag)), (sizeof(DDR_BL_KEY)-4),FLASH_READ);
    if(RET_SUCCESS!=ret)
    {
        CIPHER_PRINTF("read bl uk fail!(%x)\n", ret);
        FREE(f_temp);
        return -2;
    }
    if(is_flash_data_valid(flash_keys->key6_cipher_uk,ALIASIX_BL_UK_LEN) &&\
       is_flash_data_valid(flash_keys->r1,ALIASIX_BL_UK_LEN) )
    {
        CIPHER_PRINTF("bl uk already encrypt!(%x)\n", ret);
        FREE(f_temp);
        return -3;
    }
    CIPHER_DEBUG("plain bl uk:\n");
    CIPHER_DUMP(flash_keys->clear_bl_uk,ALIASIX_BL_UK_LEN);

    MEMCPY(flash_keys->r1,raw_data,ALIASIX_BL_UK_LEN);
    ret = ali_encrypt_ukey(flash_keys->clear_bl_uk, flash_keys->key6_cipher_uk, flash_keys->r1, OTP_KEY_0_6_R);
    if(RET_SUCCESS!=ret)
    {
        CIPHER_PRINTF("bl uk encrypt fail!(%x)\n", ret);
        FREE(f_temp);
        return -4;
    }
    CIPHER_DEBUG("random data:\n");
    CIPHER_DUMP(flash_keys->r1, ALIASIX_BL_UK_LEN);
    CIPHER_DEBUG("enc bl uk:\n");
    CIPHER_DUMP(flash_keys->key6_cipher_uk, ALIASIX_BL_UK_LEN);

    //clean the plaint key
    flash_keys->cipher_flag = 0;
    MEMSET(flash_keys->clear_bl_uk, 0x0, ALIASIX_BL_UK_LEN);
    ret = wr_flash_by_id((key_id&FLASH_CHUCK_ID_MASK), g_bl_uk_offset, \
                        (UINT8 *)(&(flash_keys->cipher_flag)), (3*ALIASIX_BL_UK_LEN+4),FLASH_WRITE);
    if(RET_SUCCESS!=ret)
    {
        CIPHER_PRINTF("write encrypted bl uk to flash fail!(%x)\n", ret);
        FREE(f_temp);
        return -5;
    }
    FREE(f_temp);
    CIPHER_DEBUG("exit %s\n",__FUNCTION__);
    return RET_SUCCESS;
}

static void illegal_rsa_func2(void) 
{        
    hw_watchdog_reboot();
    do{}while(1);
}

/*
* encrypt_universal_key() - encrypt system/u-boot universal key by key id
* @key_id :  The universal key chunk id
*            bit31~bit16:  chunk key id
*            bit15 :  vsc tri_ladder_mode   
*            bit14~bit8 :  reserved
*            bit7 ~bit0 :  which key (0xFE, the first key, 0xFD, 
*                          the second key, 0xFC the third key, etc) 
* 
* Bootloader checks the universal key for system or u-boot encryption 
* in flash and if the universial key is clear, Just call this function to 
* encrypt with two level key-ladder and store it to the flash. 
* Of coure the clear universial key shall be erased after that. 
* The system and u-boot universial keys are stored in different location
* in the flash. The ENUM FLASH_KEY_TYPE is defined for the location enum.
*/
RET_CODE encrypt_universal_key(UINT32 key_id)
{
    RET_CODE ret=RET_FAILURE;

    UINT8 raw_data[2*ALIASIX_SW_UK_LEN];
    UINT32 key_type=0;
    p_sys_sw_key flash_keys = NULL;
    UINT8 *f_temp = NULL;    
    UINT32 flash_offset = 0;

//Read key from flash
    f_temp = (UINT8*)((UINT32)MALLOC(0x200));
    if (NULL == f_temp)
    {
        CIPHER_PRINTF("MALLOC Failed!\n");
        return -9;
    }
    flash_keys = (p_sys_sw_key)((0xFFFFFFF8&((UINT32)f_temp)));  

    if(SECOND_FLASH_KEY == (key_id&FLASH_KEY_ID_MASK))
    {
        flash_offset = SYS_SW_KEY_SIZE;            
    }
    else if(THIRD_FLASH_KEY == (key_id&FLASH_KEY_ID_MASK))
    {
        flash_offset = 2*SYS_SW_KEY_SIZE; 
    }
    else if(FORTH_FLASH_KEY == (key_id&FLASH_KEY_ID_MASK))
    {
        flash_offset = 3*SYS_SW_KEY_SIZE; 
    }
    else  //FIRST_FLASH_KEY == (key_id&FLASH_KEY_ID_MASK)
    {
        flash_offset = 0; 
    }    
    ret = wr_flash_by_id((key_id&FLASH_CHUCK_ID_MASK),
                          flash_offset,
                          flash_keys->clear_sw_uk,
                          ALIASIX_SW_UK_LEN,FLASH_READ);
    if(RET_SUCCESS!=ret)
    {
        CIPHER_PRINTF("read key fail,%x\n",ret);
        FREE(f_temp);
        return -5;
    }
    CIPHER_DEBUG("plain key:\n");
    CIPHER_DUMP(flash_keys->clear_sw_uk,ALIASIX_SW_UK_LEN);

//Generate the random number
    MEMSET(raw_data, 0, sizeof(raw_data));
    ret = trng_get_64bits(raw_data,(2*ALIASIX_SW_UK_LEN/8));
    //MEMSET(raw_data,0x00, (2*ALIASIX_SW_UK_LEN));
    if(RET_SUCCESS != ret )
    {
        CIPHER_PRINTF("%s generate random data error!\n", __FUNCTION__);
        return -1;
    }

    MEMCPY(flash_keys->r1, raw_data, (2*ALIASIX_SW_UK_LEN));
    
    if(FLASH_TRI_LADDER_MODE_KEY02==(key_id&0xFF00))
    {
        key_type = OTP_KEY_0_2;
    }
    else
    {
        key_type = OTP_KEY_0_1;
    }
    ret = ali_encrypt_ukey(flash_keys->clear_sw_uk, flash_keys->cipher_sw_uk, flash_keys->r1,key_type);
    if(RET_SUCCESS != ret )
    {
        CIPHER_PRINTF("%s encrypt key error!\n", __FUNCTION__);
        return -5;
    }    
    
    CIPHER_DEBUG("cipher key:\n");
    CIPHER_DUMP(flash_keys->cipher_sw_uk, ALIASIX_SW_UK_LEN);
    CIPHER_DEBUG("random number:\n");
    CIPHER_DUMP(flash_keys->r1, 2*ALIASIX_SW_UK_LEN);

    //clear the clear UK
    MEMSET(flash_keys->clear_sw_uk, 0x0, ALIASIX_SW_UK_LEN);
    ret = wr_flash_by_id((key_id&FLASH_CHUCK_ID_MASK), flash_offset, flash_keys->clear_sw_uk,    \
                         4*ALIASIX_SW_UK_LEN, FLASH_WRITE);
    if(RET_SUCCESS!=ret)
    {
        CIPHER_PRINTF("write flash fail,%x\n",ret);
        FREE(f_temp);
        return -4;
    }
    FREE(f_temp);
    CIPHER_DEBUG("exit %s\n",__FUNCTION__);
    return RET_SUCCESS;
}
static void illegal_rsa_func3(void) 
{       
    hw_watchdog_reboot();
    do{}while(1);
}

/*
* decrypt_ukey_from_mem() - decrypt the key to key-ladder by key id, 
* @key_id:
*           bit31~bit16:  chunk key id
*           bit15~bit8 :  reserved
*           bit7 ~bit0 :  which key (0xFE, the first key, 0xFD, 
*                          the second key, 0xFC the third key, etc)
* @kernel_uk   kernel u key from bootloader
* @pos : the clear universial key position in the key-ladder .
*
* The opposite function to encrypt_universal_key() which shall decrypt the
* universial key from flash to key-ladder. The clear universial key position in
* key-ladder shall be stored to paramerter @pos. 
*/
RET_CODE decrypt_ukey_from_mem(UINT8 *pos, void *param, UINT32 key_id)
{
    RET_CODE ret = RET_FAILURE;
    p_sys_sw_key flash_keys = NULL;    
    UINT32 flash_offset = 0;
    UINT32 key_type=0;

    CIPHER_DEBUG("enter %s (%x,%x)\n", __FUNCTION__,pos,key_id);
    flash_keys = param;
    if(FLASH_TRI_LADDER_MODE_KEY02==(key_id&0xFF00))
    {
        key_type = OTP_KEY_0_2;
    }
    else
    {
        key_type = OTP_KEY_0_1;
    }

    ret =  ali_decrypt_ukey(flash_keys->cipher_sw_uk, pos, flash_keys->r1, key_type);
    if(RET_SUCCESS != ret )
    {
        CIPHER_PRINTF("%s decrypt key error!\n", __FUNCTION__);
        return -5;
    }    
    
    CIPHER_DEBUG("cipher key:\n");
    CIPHER_DUMP(flash_keys->cipher_sw_uk, ALIASIX_SW_UK_LEN);
    CIPHER_DEBUG("random number:\n");
    CIPHER_DUMP(flash_keys->r1, 2*ALIASIX_SW_UK_LEN);
    flash_keys = NULL;
    CIPHER_DEBUG("key pos is: 0x%x\n", *pos);
    CIPHER_DEBUG("exit %s \n", __FUNCTION__);
    return RET_SUCCESS;
}

static void illegal_rsa_func4(void) 
{        
    hw_watchdog_reboot();
    do{}while(1);
}

/*
* get_enc_uk_mode() - get the enc_uk and uk mode from CPU.
*
* @uk_mode:   which root key is used for uk encrypt
* @enc_uk:      the cipher uk for 3-level key mode use
* 
* This interface is only be used for CPU part, and it is only active when decrypt_universal_key is used
* 
*/
void get_enc_uk_mode(UINT32 *uk_mode, UINT8 *enc_uk)
{
    if(NULL == enc_uk)
        return;
    *uk_mode = g_sw_uk_mode;
    MEMCPY(enc_uk, g_enc_uk, ALIASIX_SW_UK_LEN);
}

/*
* aes_cbc_decrypt_ram_chunk() - decrypt data use key ladder, the data from sram 
* the decrypted key will store in the crypto engine, the position is key_pos.
*
* @key_pos:   the key position in key ladder
* @out:       the output buffer, need 8 bytes align for M3603/M3606/M3383, other project only need 4 bytes align
* @data:      input data, need 8 bytes align for M3603/M3606/M3383, other project only need 4 bytes align
* @data_len:  data length for decrypt
* 
* This interface has nothing about the chunk structure.It it the safe 
* interface for user to decrypt data with key ladder key.
* 
*/
RET_CODE aes_cbc_decrypt_ram_chunk(UINT8 key_pos, UINT8 *out,
                    UINT8 *data, UINT32 data_len)
{
    RET_CODE ret = RET_FAILURE;
    UINT32 kl_level = sys_ic_get_kl_key_mode();
    CIPHER_DEBUG("enter %s (%x,%x, %x,%x)\n", __FUNCTION__, key_pos, out, data, data_len);
    if ((NULL == out) || (NULL == data) || (0 == data_len) )
    {
        CIPHER_PRINTF("Invalid parameter!\n");
        return -1;
    }

     if(!length_ok(data_len))
    {
        CIPHER_PRINTF("Invalid inpute data length!\n");
        return -1;   
    }

    if((!access_ok(data,data_len)) || \
       (!access_ok(out,data_len)) )
    {
        CIPHER_PRINTF("Invalid memory!\n");
        return -1;   
    }
       
    CIPHER_DEBUG("First 16 bytes input data:\n");
    CIPHER_DUMP(data, 16);

    if((KEY_0_1==g_sw_uk_mode)&&(3==kl_level))
    {
        ret = dsc_deal_sys_uk_fw(data, out, g_enc_uk, data_len,key_pos,DSC_DECRYPT);
    }
    else
    {
        ret = aes_crypt_puredata_with_ce_key(data, out, data_len, key_pos, DSC_DECRYPT);
    }
    CIPHER_DEBUG("First 16 bytes output data:\n");
    CIPHER_DUMP(out, 16);

    if(RET_SUCCESS != ret)
    {
        CIPHER_PRINTF("%s decrypt data error! %x\n", __FUNCTION__, ret);
        return -2;
    }
    CIPHER_DEBUG("exit %s\n", __FUNCTION__);
    return RET_SUCCESS;
}

static void illegal_rsa_func6(void) 
{    
    hw_watchdog_reboot();
    do{}while(1);
}


/*
* aes_cbc_decrypt_flash_data() - decrypt data use key ladder,the data will 
*   get from nor flash by block_addr the decrypted key will store in the 
*   key ladder,the position is key_pos
*
* @key_pos:    the key position in key ladder
* @out:        the output buffer, need 8 bytes align for M3603/M3606/M3383
* @block_addr: block offset on Nor flash
* @block_len:  data length for decrypt
*/
RET_CODE aes_cbc_decrypt_flash_data(UINT8 key_pos, UINT32 block_addr,
                            UINT32 block_len, UINT8 *out)
{
    UINT8 *temp_buffer=cipher_tmp_buf_addr;
    UINT8 *f_temp = NULL;
    RET_CODE ret = RET_FAILURE;
    struct sto_device *flash_dev = NULL;
    UINT32 kl_level = sys_ic_get_kl_key_mode();

    CIPHER_DEBUG("enter %s (%x,%x,%x)\n", __FUNCTION__, key_pos, block_addr, block_len);    

    if ((NULL == out) || (0 == block_addr) || (0 == block_len))
    {
        CIPHER_PRINTF("Invalid parameter!\n");
        return -1;
    }
    if((!access_ok(out,block_len)) || (!length_ok(block_len)))
    {
        CIPHER_PRINTF("Invalid memory!\n");
        return -1;   
    }    
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if (NULL == flash_dev)
    {
        CIPHER_PRINTF("Cannot find flash device!\n");
        return -1;
    }
      
    if(NULL == temp_buffer)
    {
        f_temp = (UINT8 *)MALLOC(block_len + 0xf);
        if (NULL == f_temp)
        {
            CIPHER_PRINTF("MALLOC Failed!\n");
            return -9;
        }
        temp_buffer = (UINT8 *)((0xFFFFFFF8 & (UINT32)f_temp));
    }
    ret = sto_get_data(flash_dev, temp_buffer, block_addr, block_len);
    if(block_len != (UINT32)ret)
    {
        CIPHER_PRINTF("%s get data from flash error! %x\n", __FUNCTION__, ret);
        if(f_temp != NULL)
        {
            FREE(f_temp);
            f_temp = NULL;
            temp_buffer = NULL;
        }
        return -3;
    }
    ret = aes_cbc_decrypt_ram_chunk(key_pos, out, temp_buffer, block_len);
    if(f_temp != NULL)
    {
        FREE(f_temp);
        f_temp = NULL;
        temp_buffer = NULL;
    }
    if(RET_SUCCESS != ret)
    {
        CIPHER_PRINTF("%s decrypt data error! %x\n", __FUNCTION__, ret);
        return -2;
    }
    CIPHER_DEBUG("exit %s \n", __FUNCTION__);
    return RET_SUCCESS;   
}

static void illegal_rsa_func7(void) 
{   
    hw_watchdog_reboot();
    do{}while(1);
}

/*
* aes_cbc_decrypt_chunk() - decrypt data use key ladder,the data will 
*   get from nor flash by block_id, the decrypted key store in the 
*   key ladder,the position is key_pos
*
* @key_pos:    the key position in key ladder
* @blockid:    block ID on Nor flash
* @out:        the output buffer, need 8 bytes align for M3603/M3606/M3383
* @len:        data length for decrypt
*/
RET_CODE aes_cbc_decrypt_chunk(UINT8 key_pos, UINT32 blockid, 
                UINT8 *out, UINT32 *len)
{
   UINT8 *temp_buffer=cipher_tmp_buf_addr;
    UINT8 *f_temp = NULL;
    RET_CODE ret = RET_FAILURE;
    struct sto_device *flash_dev = NULL;
    UINT32 block_addr = 0;
    UINT32 block_len = 0;
    UINT32 kl_level = sys_ic_get_kl_key_mode();

    CIPHER_DEBUG("enter %s (%x,%x,%x)\n", __FUNCTION__, key_pos, blockid, out);

    if ((NULL == out) || (0 == blockid ) )
    {
        CIPHER_PRINTF("Invalid parameter!\n");
        return -1;
    }
    if (RET_SUCCESS != sto_get_chunk_len(blockid, &block_addr, &block_len))
    {
        CIPHER_PRINTF("Cannot find chunk id 0x%08x.\n", blockid);
        return -1;
    }  
    ret = aes_cbc_decrypt_flash_data(key_pos, block_addr, block_len, out);
    *len = block_len;
    CIPHER_DEBUG("exit %s \n", __FUNCTION__);
    return ret;
}

static void illegal_rsa_func8(void) 
{    
    hw_watchdog_reboot();
    do{}while(1);
}

/*
*  version_check() - Check the software chund version  
*
* @blockid:    block ID on flash
* @block_addr:  block address in flash
* @block_len:   block length in flash
*
* This Interface is using to check the software version against to the OTP by
* remoting to SEE. 
*/
RET_CODE version_check(UINT32 block_id,UINT32 block_addr,UINT32 block_len)
{    
    RET_CODE ret = RET_FAILURE;
    DSC_VER_CHK_PARAM ver_param;
    
    pDSC_DEV dsc_dev = ( pDSC_DEV ) dev_get_by_type ( NULL, HLD_DEV_TYPE_DSC );

    if(dsc_dev!= NULL)
    {
         MEMSET(&ver_param,0,sizeof(DSC_VER_CHK_PARAM));
         ver_param.chk_id = block_id;
         osal_cache_flush((void *)block_addr, block_len);
         ver_param.input_mem = block_addr|0xa0000000;
         ver_param.len = block_len ;
         ver_param.chk_mode =   CHECK_END  ; 
         
         ret=dsc_ioctl(dsc_dev,IO_DSC_VER_CHECK,(UINT32)&ver_param);
         if(ret == RET_FAILURE)
         {
               hw_watchdog_reboot();
         }
    }
    else 
    {
        return RET_FAILURE;
    }
    
    return RET_SUCCESS;
}

static void illegal_rsa_func10(void) 
{    
    hw_watchdog_reboot();
    do{}while(1);
}

/*
*  version_update() - update the version OTP value .
*
* @new_version:    version count shall be updated.
* @flag:   define the OTP location in hardware.
*          BL_VER means update the bootloader version value in OTP
*          SW_VER means software.
*/
BOOL version_update(UINT32 new_version, enum VER_TYPE flag)
{
    UINT32 ver[4]={0,0,0,0};
    UINT32 ver_addr = 0;
    UINT32 count=0;
    UINT32 ret_len = 0;
    
    if(BL_VER == flag)
    {
        ver_addr = 4*OTP_ADDESS_BL_VER;
    }
    else if(SW_VER == flag)
    {
        ver_addr = 4*OTP_ADDESS_SW_VER;
    }
    else
    {
        return FALSE;
    }

    ret_len = otp_read(ver_addr,(UINT8 *)ver,OTP_VER_MAX_LEN);
    if(ret_len!=OTP_VER_MAX_LEN)
    {
        return FALSE;
    }

    otp_transfer_to_version(ver, &count);
    
    if(new_version<=count)
    {
        return FALSE;
    }
    else if(new_version>=MAX_VERSION_NUM)
    {
        return TRUE;
    }
    else
    {
        version_transfer_to_otp(ver, new_version);
        ret_len = otp_write((UINT8 *)ver,ver_addr,OTP_VER_MAX_LEN);    
        if(ret_len!=OTP_VER_MAX_LEN)
        {
            return FALSE;
        }
    }

    ret_len = otp_read(ver_addr,(UINT8 *)ver,OTP_VER_MAX_LEN);
    if(ret_len!=OTP_VER_MAX_LEN)
    {
        return FALSE;
    }
    otp_transfer_to_version(ver, &count);
    if(count==new_version)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*
*  version_verify() - verify the OTP version value .
*
* @new_version:    version count shall be updated.
* @flag:   define the OTP location in hardware.
*          BL_VER means update the bootloader version value in OTP
*          SW_VER means software.
* Compare the new_version with the OTP version.
*/
BOOL version_verify(UINT32 new_version, enum VER_TYPE flag)
{
    UINT32 ver[4]={0,0,0,0};
    UINT32 ver_addr = 0;
    UINT32 count=0;
        UINT32 ret_len = 0;    

    if(BL_VER == flag)
    {
        ver_addr = 4*OTP_ADDESS_BL_VER;
    }
    else if(SW_VER == flag)
    {
        ver_addr = 4*OTP_ADDESS_SW_VER;
    }
    else
    {
        return RET_FAILURE;
    }
    ret_len = otp_read(ver_addr,(UINT8 *)ver,OTP_VER_MAX_LEN);
    if(ret_len!=OTP_VER_MAX_LEN)
    {
        return RET_FAILURE;
    }
    otp_transfer_to_version(ver, &count);
	libc_printf("%s: new_version=%d, count=%d\n", __FUNCTION__, new_version, count);
    if((new_version<count))
    {
        return RET_FAILURE;
    }
    else
    {
        return RET_SUCCESS;
    }
}

static void fetch_vsc_lib_key (UINT8 *encrypted_ck)
{
	unsigned long bl_offset = 0;
	UINT8 keydata[VSC_CK_LEN] = {0};
	UINT8 zero_data[VSC_CK_LEN] = {0};
	struct sto_device *flash_dev = NULL;
	INT32 read_length = 0;

	MEMSET ((void *)keydata, 0, VSC_CK_LEN);
	MEMSET ((void *)encrypted_ck, 0, VSC_CK_LEN);
	read_length = otp_read ((VSC_ECK_OTP_ADDR << 2), (UINT8 *)(keydata), VSC_CK_LEN);
	if (VSC_CK_LEN != read_length)
	{
		CIPHER_PRINTF ("%s (%d) %d\n", __FUNCTION__, __LINE__, read_length);
	}
	// ECK in OTP
	if (0 != MEMCMP ((void *)keydata, (void *)zero_data, VSC_CK_LEN))
	{
		MEMCPY ((void *)encrypted_ck, (void *)keydata, VSC_CK_LEN);
	}
	// ECK in Flash
	else
	{
		flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
		if (NULL != flash_dev)
		{
			bl_offset = 0 + VSC_ECK_STO_ADDR;
			sto_get_data(flash_dev, encrypted_ck, bl_offset, VSC_CK_LEN);
		}
	}
}

RET_CODE aes_cbc_decrypt_vsc(UINT8 *in, UINT32 len)
{
	pCE_DEVICE p_ce_dev = NULL;
	OTP_PARAM opt_info;
	UINT8 encrypted_ck[VSC_CK_LEN] = {0};
	RET_CODE ret = RET_SUCCESS;
					
	if (NULL == in)
	{
		CIPHER_PRINTF("Invalid parameter!\n");
		return -9;
	}
	p_ce_dev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);

	MEMSET(&opt_info, 0, sizeof(OTP_PARAM));
	opt_info.otp_addr = VSC_UK_ADDR;
	opt_info.otp_key_pos = KEY_0_3;	
	ret = ce_key_load(p_ce_dev, &opt_info);
	if (RET_SUCCESS != ret)
	{
		CIPHER_PRINTF("load key4 failed!");
		return -1;	
	}
	fetch_vsc_lib_key (encrypted_ck);
	ret = ce_generate_key_by_aes(encrypted_ck,KEY_0_3,KEY_1_3,CE_IS_DECRYPT);
	if(RET_SUCCESS != ret)
	{
		CIPHER_PRINTF("generate 1 level key fail!!\n");
	    return -2;
	}
	ret = dsc_fixed_cryption(in, len, KEY_1_3);
	if(RET_SUCCESS !=ret)
	{
		CIPHER_PRINTF("decrypt fail,%x\n",ret);
		return -3;
	}
	return ret;
}
