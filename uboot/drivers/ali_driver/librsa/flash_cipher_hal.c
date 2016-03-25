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

extern UINT32 g_bl_uk_offset;

/*
*  otp_transfer_to_version() - transfer OTP value to version count.
*
* @buf:       OTP value from hardware .
* @version:   Version count (output)
*/
void otp_transfer_to_version(const UINT32 *buf, UINT32 *version)
{
    UINT32 i=0, 
           j=0;

    *version = 0;
    for(i=4;i>0;i--)
    {
        for(j=32;j>0;j--)
        {
            if(buf[i-1]&(0x1<<(j-1)))
            {
                *version = 32*(i-1)+j;
                break;
            }
        }
    }
}

/*
*  version_transfer_to_otp() - transfer version count to OTP value .
*
* @buf:       OTP value to hardware .
* @version:   Version count (input)
*/
void version_transfer_to_otp(UINT32 *buf, const UINT32 version)
{
    UINT32 row=0;
    UINT32 bits=0;

    MEMSET(buf, 0, OTP_VER_MAX_LEN);
    for(row=version/32;row>0;row--)
    {
        buf[row-1] = 0xFFFFFFFF;
    }
    for(bits=version%32;bits>0;bits--)
    {
        buf[row] |= (1<<(bits-1));
    }
}

INT32 access_ok(const void *addr, UINT32 size)
{
    UINT32 val = (UINT32) addr;

    return (val >= MEMORY_START) && ((val + size )<MEMORY_END);
}

INT32 length_ok(const UINT32 size)
{   
    return (size > LEN_ZERO) && ( size <LEN_32M);
}

/*
* This function used to return the bl universal key offset 
* in the flash mapping
*/
UINT32 get_bl_uk_offset(void)
{
    if(sys_ic_get_chip_id()==ALI_S3281)
    {
        g_bl_uk_offset = BL_UK_OFFEST_M3281;
    }
    else if( (sys_ic_get_chip_id()>=ALI_C3701 ) )
    {
        g_bl_uk_offset = BL_UK_OFFEST_M3701;
    }
    return g_bl_uk_offset;
}
/*
* This function used to judge the flash data is all 0xff or not
* @UINT8 *test_data: the point of the data
* @UINT32 len:       the length of the data
*/
INT32 is_flash_data_valid(UINT8 *test_data, UINT32 len)
{
    UINT32 i = 0;
        
    for(i=0;i<len;i++)
    {
        if(test_data[i]!=0xff)
        {
            return -1;
        }
    }
    if(i==len)
    {
        return 0;
    }
    else
    {
        return -1 ;
    }
}

/*
* wr_flash_by_id() - read and write universial key in extern flash
* @key_id:      the chunk id need to access
* @key_offest:  the offset in the chunk
* @data:        the input or output buffer point
* @length:      data length
* @wr :         FLASH_READ or FLASH_WRITE
*
*
* Internal Interface to deal with the universal key in flash. 
* Encrypt the clear universial key or decrypt the cipher universial
* key shall both need this function.
*/
RET_CODE wr_flash_by_id(UINT32 key_id, UINT32 key_offest, UINT8 *data, 
                    UINT32 length, enum FLASH_SELECT wr)
{
    UINT32 key_block_addr = 0;
    UINT32 key_block_len = 0;
    RET_CODE ret = RET_FAILURE;
    UINT32 ret_len = 0;

    struct sto_device *flash_dev = NULL;
               
           
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);

    if (NULL == flash_dev)
    {
        CIPHER_PRINTF("Can't find FLASH device!\n");
        return -1;
    }

    ret = sto_get_chunk_len(key_id, &key_block_addr, &key_block_len);
    if(RET_SUCCESS != ret)
    {
        return -2;
    }
    key_block_addr += key_offest;
    if(FLASH_READ == wr) //read from flash
    {
        ret_len = sto_get_data(flash_dev, data, key_block_addr, length);   
        if(ret_len!=length)
        {
            return -3;
        }
    }
    else if(FLASH_WRITE == wr) //write to flash
    {
        ret_len = sto_put_data(flash_dev,key_block_addr,data,length);
        if(ret_len!=length)
        {
            return -3;
        }
    }
    return RET_SUCCESS;
}