#ifndef _FLASH_ENC_
#define _FLASH_ENC_
#include <ali/basic_types.h>
#include "rsa_public.h"

#define RSA_DEBUG_EN 0
#define RSA_DATA_DEBUG_EN 0

#define CIPHER_DEBUG_EN                   1
#define CIPHER_DATA_DEBUG_EN        1

#define FLASH_SIZE              0x400000    // limit to 4MB by default
#define BL_UK_OFFEST_M3281    0x200
#define BL_UK_OFFEST_M3701    0x600
#define GET_FLASH_DATA(p)  ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3])
#define BL_UK_OFFEST          (((CHIP_IS_3701)||(CHIP_IS_3503))?\
                              (BL_UK_OFFEST_M3281):(BL_UK_OFFEST_M3701))


#define SYS_SW_KEY_SIZE     (4*ALIASIX_SW_UK_LEN) 
#define SYS_SW_PUB_KEY_OFFSET_M3281     0x400
#define SYS_SW_PUB_KEY_OFFSET_M3701     0x800 //uboot pk 
#define SYS_KERNEL_PUB_KEY_OFFSET 0xb00

#define TRANS_PARAMS_MAX_SIZE 0x20000

#define FLASH_BASE SYS_FLASH_BASE_ADDR

//===================================================
#define GET_DWORD(addr)                  (*(volatile UINT32 *)(addr))
#define SET_DWORD(addr, d)              (*(volatile UINT32 *)(addr)) = (d)
#define SETB_DWORD(addr, bit)          *(volatile UINT32 *)(addr) |= (bit)
#define SETB_BYTE(addr, bit)              *(volatile UINT8 *)(addr) |= (bit)
#define GET_BYTE(addr)                     (*(volatile UINT8 *)(addr))
#define SET_BYTE(addr, bit)                (*(volatile UINT8 *)(addr)) = (bit)

enum FLASH_SELECT
{
    FLASH_READ=0,
    FLASH_WRITE=1
};

typedef struct ddr_bl_key
{
    UINT32  reserved;                       //for 8 bytes align

    /*cipher_flag=0xadeadbee means BL UK is clear*/
    UINT32  cipher_flag;         

    UINT8   clear_bl_uk[ALIASIX_BL_UK_LEN];
    UINT8   key6_cipher_uk[ALIASIX_BL_UK_LEN];
    UINT8   r1[ALIASIX_BL_UK_LEN];
    UINT8   key7_cipher_uk[ALIASIX_BL_UK_LEN];
}DDR_BL_KEY, *p_ddr_bl_key; 




/*if the bootloader use 2nd level public key 
  there will be the 2nd public key and its signature 
  in the tial of bootloader otherwise, the length of 
  them is zero
*/
typedef struct tail_of_bl_param
{
    R_RSA_PUBLIC_KEY sw_pub_key;  
    UINT8   padding[ALIASIX_RSA_COMPARE_LEN-4];
    UINT8   pub_key_2nd[0x300];                                  
    UINT8   signature_of_pub_key_2nd[ALIASIX_RSA_COMPARE_LEN];   
    UINT8   bl_signature[ALIASIX_RSA_COMPARE_LEN];
}TAIL_OF_BL_PARAM, *p_tail_of_bl_param; 

#define UNCHACE(x) (((UINT32)x) | (0xa << 28))
#ifdef M3606_SECURE_ENABLE
#define DSC_BASE_ADDR 0xb803a000
#define DSC_READ_WORD(offset)  (*(volatile UINT32 *)(DSC_BASE_ADDR + offset))
#define DSC_WRITE_WORD(offset,val)   \
            (*(volatile UINT32 *)(DSC_BASE_ADDR + offset) = (val))
#endif

#define OTP_PUBKEY_ADDR    (0x4*4)
#define OTP_PUBKEY_SIZE    (4+256+32)
#define OTP_PUBKEY_SIZE_FULL 516
#define LAST_EXP_SIZE    32
#define OTP_LAST_EXP_OFFSET        (OTP_PUBKEY_SIZE-LAST_EXP_SIZE)
#define OTP_LAST_EXP_OFFSET_FULL (OTP_PUBKEY_SIZE_FULL-LAST_EXP_SIZE)

/*Clear Bootloader in DRAM after Bootrom*/
#define FLASH_MIRROR_BASE        0xA0500000

/* Stage1 pass parametes to next by this 
*/
#define STAGE_TRANSFER_ADDR0        0x84000018
#define STAGE_TRANSFER_ADDR1        0x8400001C
#define ALI_TRANSFER_MAGIC_NUM    0xABCD5AA5




#define MEMORY_START 0x80000000
#define MEMORY_END   0xC0000000
#define LEN_ZERO     0x0
#define LEN_32M      0x2000000

#define INVALID_CE_KEY_POS                0xff
#define INVALID_DSC_SUB_DEV_ID            0xff
#define INVALID_DSC_STREAM_ID            0xff

#define VSC_UK_ADDR		0x64	//0x60
#define VSC_CK_LEN		16
#define VSC_ECK_OTP_ADDR 0x7C
#define VSC_ECK_STO_ADDR (128 + 384 - 16)

INT32 access_ok(const void *addr, UINT32 size);
INT32 length_ok(const UINT32 size);

#endif
