//#include "flash_verify.h"
//#include "flash_enc.h"
#include "lib_rsa.h"
#include "basic_types.h"
#include "NN.h"
#include <common.h>
#include "rsa_verify.h"
#include "ali_simple_dsc.h"

//#define RSA_DEBUG_EN 1
//#define RSA_DATA_DEBUG_EN 1
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
#define RSA_PRINTF          printf
#else
#define RSA_PRINTF(...)     do{}while(0)
#endif
#if RSA_DATA_DEBUG_EN
#define RSA_DEBUG           printf
#define RSA_DUMP(data,len) { const int l=(len); int i;\
                             for(i=0 ; i<l ; i++) {RSA_DEBUG("0x%02x,",(*(data+i))); \
                             if((i+1)%16==0) RSA_DEBUG("\n");}\
                             RSA_DEBUG("\n"); }
#else
#define RSA_DEBUG(...)           do{}while(0)
#define RSA_DUMP(data,len)  do{}while(0)
#endif

/*
Verify the signature of input data, if pass verification, return RET_SUCCESS, otherwise, return error NO.
UINT8 *addr: the start address for the check data, it's must 4bytes align
UINT8 *sign: the start address for the signature, the length of the signature should be 256bytes
const UINT32 length:  data length for verify
UINT8 *pub_address: the public key address 
*/

int ali_rsa_verification(unsigned char *addr, unsigned char *sign, \
                    const unsigned int length, unsigned char *pub_address)
{
    UINT32 addr_outputlen = 0;
    UINT8  *rsa_buf_out;   
    UINT8  sha_signature[32];
    R_RSA_PUBLIC_KEY *g_rsa_Public_Key;
    RET_CODE  ret = RET_FAILURE;

    if((NULL == addr) || (NULL == sign) || (NULL == pub_address))
    {
        ret = -1;
        goto err;
    } 
    rsa_buf_out = (UINT8 *)malloc(256);
    if(rsa_buf_out == NULL)
    {
        RSA_PRINTF("rsa_buf_out malloc fail\n");
        ret = RET_FAILURE;
        goto err;
    }
    g_rsa_Public_Key = (R_RSA_PUBLIC_KEY *)malloc(sizeof(R_RSA_PUBLIC_KEY));
    if(g_rsa_Public_Key== NULL)
    {
        RSA_PRINTF("g_rsa_Public_Key malloc fail\n");
        ret = RET_FAILURE;
        goto err;
    } 
    memset(sha_signature, 0, 32);
    memset((void *)g_rsa_Public_Key, 0, sizeof(R_RSA_PUBLIC_KEY));
    memcpy((UINT8 *)g_rsa_Public_Key, pub_address, OTP_PUBKEY_SIZE-LAST_EXP_SIZE);
    memcpy((UINT8 *)g_rsa_Public_Key+OTP_LAST_EXP_OFFSET_FULL,pub_address+OTP_LAST_EXP_OFFSET,LAST_EXP_SIZE);
    RSA_PRINTF("first 32 bytes rsa buf in: (%x,%x,%x)\n", (unsigned int)addr, (unsigned int)sign,length);
    RSA_DUMP(((UINT8 *)addr),32);
    RSA_PRINTF("Last 32 bytes rsa buf: \n");
    RSA_DUMP(((UINT8 *)addr + length - 32), 32);
    if (RET_SUCCESS!= (ret = ali_rsapublicfunc(rsa_buf_out, &addr_outputlen, sign, ALIASIX_RSA_COMPARE_LEN, g_rsa_Public_Key)))
    {
        RSA_PRINTF("ali_rsapublicfunc() failed!(%x)\n",ret);
        ret = -2;
        goto err;
    }
    RSA_DUMP(((UINT8 *)addr),3);
    RSA_PRINTF("Start Sha count: len : %d\n", length);

    // calc signature
    if (RET_SUCCESS != create_sha_ramdrv((UINT8 *)addr, length, sha_signature))
    {
        RSA_PRINTF("create_sha_ramdrv() failed!\n");
        ret = -4;
        goto err;
    }
    //compare the result
    RSA_PRINTF("SHA digest is:\n");
    RSA_DUMP(sha_signature,sizeof(sha_signature));
    RSA_PRINTF("rsa result is:\n");
    RSA_DUMP(rsa_buf_out,ALIASIX_RSA_COMPARE_LEN);    
    ret = ali_rsa_result_compare(rsa_buf_out,(void *)sha_signature);
    if(ret!=RET_SUCCESS)
    {
        RSA_PRINTF("result compare error!\n");
        ret = -5;
        goto err;
    } 

    ret = RET_SUCCESS;
err:
    if(rsa_buf_out != NULL)
        free(rsa_buf_out);
    if(g_rsa_Public_Key != NULL)
        free(g_rsa_Public_Key);
    printf("ali_rsa_verification over\n");
    return ret;
}
