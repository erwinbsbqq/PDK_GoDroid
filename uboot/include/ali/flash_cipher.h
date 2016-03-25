/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: flash_cipher.h
*   
*    Description: 
*       
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef  _FLASH_CIPHER_H_
#define  _FLASH_CIPHER_H_

enum VER_TYPE
{
    BL_VER = 0,
    SW_VER = 1    
};

#define  NEW_HEAD_ID 0xaead0000
#define  NEW_HEAD_MASK 0xffff0000
#define  KERNEL_UK_OFFSET_WITH_NEW_HEAD 0xc0
#define  SYS_UK_OFFSET_WITH_NEW_HEAD 0x80
#define  UK_OFFSET_WITH_NEW_HEAD 0x1000
#define  BL_UK_TYPE_MASK   0xFF00
#define  BL_CODE_OFFSET    0x1800


/*
The number of the key in key chunk
---address 0--------
chunk header
---address 0x80-----
security key 1
---address 0x80+0x40-----
security key 2
---address 0x80+0x40*2-----
security key 3
---address 0x80+0x40*3-----
*/
enum FLASH_KEY_TYPE
{
    BL_FLASH_KEY =    0xFF,
    FIRST_FLASH_KEY = 0xFE,
    SECOND_FLASH_KEY = 0xFD,
    THIRD_FLASH_KEY = 0xFC,
    FORTH_FLASH_KEY = 0xFB
};

enum FLASH_DATA_TYPE
{
    DATA_FROM_SRAM = 0,
    DATA_FROM_BLOCK_ADDR = 1,
    DATA_FROM_BLOCK_ID = 2
};

typedef struct SRAM_UKEY
{
    UINT8 *uk;
    UINT32 len;
    UINT32 resv;
}SRAM_UKEY;

typedef struct CE_UKEY
{
    UINT8 pos;
    UINT32 resv;
}CE_UKEY;

typedef struct UKEY_INFO
{
    UINT8   enc_uk[16];
    UINT32 ukey_mode;
    UINT32 kl_key_pos;
    UINT32 resv[2];
}UKEY_INFO;

typedef struct CRYPT_DATA_INFO
{
    UINT8 *buf;
    UINT32 blockid;
    UINT32 addr;
	UINT32 len;
    enum FLASH_DATA_TYPE data_type;
	UINT32 resv[2];
}CRYPT_DATA_INFO;

#define FLASH_CHUCK_ID_MASK   0xFFFF0000
#define FLASH_KEY_ID_MASK     0xFF
#define FLASH_TRI_LADDER_MODE_KEY02     0x8000

#define OTP_VER_MAX_LEN        16
#define OTP_ADDESS_BL_VER      0x74
#define OTP_ADDESS_SW_VER      0x78
#define MAX_VERSION_NUM        128

#define ALIASIX_BL_UK_LEN       			  16
#define ALIASIX_SW_UK_LEN       			  16

#define BL_KEY_CIPHER_MAGIC_NUMBER            0xadeafbee

#define LOW_ADDR  				0
#define HIGH_ADDR 				1


#define PMI_MAC_OFSET 160
#define PMI_OFFSET  (256)  // 256 pages pmi offset 

#define PMI_HASH_OFFSET (8192)
#define PMI_SIZE        (512)    

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
BOOL is_key_encrypted(UINT32 offset, enum FLASH_KEY_TYPE key_type);
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
RET_CODE encrypt_bl_universal_key(UINT32 key_id);
/*
* This function used to decrypt the key by key_block_addr, 
 the decrypted key will store in the crypto engine,
 the position is return in pos
UINT32 key_id
bit31~bit16:  chunk key id
bit15~bit8 :  reserved
bit7 ~bit0 :  which key  
(0xFE, the first key, 0xFD, the second key, 0xFC the third key, etc)
*/
RET_CODE decrypt_bk_universal_key(UINT8 *pos, UINT32 key_block_addr, 
                        UINT32 key_block_len);


/*
*  version_check() - Check the software chunk version  
*
* @blockid:    block ID on flash
* @block_addr:  block address in flash
* @block_len:   block length in flash
*
* This Interface is using to check the software version against to the OTP by
* remoting to SEE. 
*/
RET_CODE version_check(UINT32 block_id,UINT32 block_addr,UINT32 block_len);


void flash_cipher_buf_init(UINT8 *tmp_buf_addr, UINT32 tmp_buf_size);

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
INT32 encrypt_universal_key(UINT32 key_id);

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
INT32 decrypt_ukey_from_mem(UINT8 *pos, void *param, UINT32 key_chunk_id);


/*
* aes_cbc_decrypt_ram_chunk() - decrypt data use key ladder, the data from sram 
* the decrypted key will store in the crypto engine, the position is key_pos.
*
* @key_pos:   the key position in key ladder
* @out:       the output buffer, need 8 bytes align for M3603/M3606/M3383
* @data:      input data
* @data_len:  data length for decrypt
* 
* This interface has nothing about the chunk structure.It it the safe 
* interface for user to decrypt data with key ladder key.
* 
*/
INT32 aes_cbc_decrypt_ram_chunk(UINT8 key_pos, UINT8 *out, 
                UINT8 *data, UINT32 data_len);

/*
* aes_cbc_decrypt_flash_data() - decrypt data use key ladder,the data will 
*   get from nor flash by block_addr the decrypted key will store in the 
*   key ladder,the position is key_pos
*
* @key_pos:    the key position in key ladder
* @out:        the output buffer, need 8 bytes align for M3603/M3606/M3383
* @block_addr: block offset on flash
* @block_len:  data length for decrypt
*/
INT32 aes_cbc_decrypt_flash_data(UINT8 key_pos, UINT32 block_addr,
                 UINT32 block_len, UINT8 *out);

/*
* aes_cbc_decrypt_chunk() - decrypt data use key ladder,the data will 
*   get from nor flash by block_id, the decrypted key store in the 
*   key ladder,the position is key_pos
*
* @key_pos:    the key position in key ladder
* @blockid:    block ID on flash
* @out:        the output buffer, need 8 bytes align for M3603/M3606/M3383
* @len:        data length for decrypt
*/
INT32 aes_cbc_decrypt_chunk(UINT8 key_pos, UINT32 blockid,
                   UINT8 *out, UINT32 *len);


BOOL version_update(UINT32 new_version, enum VER_TYPE flag);
BOOL version_verify(UINT32 new_version, enum VER_TYPE flag);


/*add for three ladder*/

/*
* decrypt_universal_key_ex() - decrypt the key to key-ladder by key id, 
*
* The opposite function to encrypt_universal_key_ext() which shall decrypt the
* universial key from flash to key-ladder. The clear universial key position in
* key-ladder shall be stored to paramerter @plain_key. 
*/
RET_CODE decrypt_universal_key_ext(UINT8 *plain_key, UINT32 key_id);

/*
* encrypt_universal_key_ext() - Extend interface for encrypt_universal_key()
* @key_id :  The universal key chunk id
*            bit31~bit16:  chunk key id
*            bit15~bit8 :  reserved
*            bit7 ~bit0 :  which key (0xFE, the first key, 0xFD, 
*                          the second key, 0xFC the third key, etc) 
* 
* This function is a customizable interface for encrypt_universal_key() with 
* three level key-ladder.
*/
RET_CODE encrypt_universal_key_ext(UINT32 key_id);

/*
*  aes_cbc_decrypt_chunk_ext() - decrypt data use key ladder with 3 level 
*    key ladder,the data will get from nor flash by block_id, the decrypted 
*    key stores in the key ladder.
*
* @key:        the key position in key ladder
* @blockid:    block ID on flash
* @out:        the output buffer, need 8 bytes align for M3603/M3606/M3383
* @len:        data length for decrypt
*/
RET_CODE aes_cbc_decrypt_chunk_ext(UINT8*key , UINT32 blockid,
                 UINT8 *out, UINT32 *len);


RET_CODE aes_cbc_decrypt_flash_data_ext (UINT8 *out,UINT8* plain_key,
                 UINT32 block_addr, UINT32 block_len);

RET_CODE aes_decrypt_data_from_sram_ext(UINT8 *output,UINT8* inputkey, 
                    UINT8* inputdate,UINT32 block_len, enum CRYPT_SELECT sel);

RET_CODE aes_cbc_decrypt_VSC(UINT8 *in, UINT32 len);


#endif /*_FLASH_CIPHER_H_*/
