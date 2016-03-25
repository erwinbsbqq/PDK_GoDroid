#ifndef  _FLASH_PARAM_H_
#define  _FLASH_PARAM_H_

#include <ali/flash_cipher.h>

#define ALI_RSA_MODULUS_BITS 2048
#define ALI_RSA_MODULUS_LEN ((ALI_RSA_MODULUS_BITS + 7) / 8)

typedef struct sys_sw_key
{
    UINT8   clear_sw_uk[ALIASIX_SW_UK_LEN];
    UINT8   cipher_sw_uk[ALIASIX_SW_UK_LEN];
    UINT8   r1[ALIASIX_SW_UK_LEN];
    UINT8   r2[ALIASIX_SW_UK_LEN];
  //  UINT8   r3[ALIASIX_SW_UK_LEN];
}SYS_SW_KEY, *p_sys_sw_key; 


typedef struct 
{
  UINT32 bits;                           /* length in bits of modulus */
  UINT8 modulus[ALI_RSA_MODULUS_LEN];  /* modulus */
  UINT8 exponent[ALI_RSA_MODULUS_LEN]; /* public exponent */
} R_RSA_PUBLIC_KEY;
//transfer parameter size less than 128k bytes
typedef struct transfer_params
{
    UINT32 param_len;                 // this struct size
    UINT32 see_len;
    R_RSA_PUBLIC_KEY  public_key1;                           /* public key for u-boot to verify kernel etc*/
    SYS_SW_KEY        sys_uk;                       // sys uk for u-boot to decrypt kernel etc    
} TRANSFER_PARAMS, *p_transfer_params;
#endif //_FLASH_PARAM_H_