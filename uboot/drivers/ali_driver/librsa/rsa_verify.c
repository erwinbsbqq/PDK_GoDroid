
#include <ali/basic_types.h>
#include <ali/retcode.h>
#include <ali/sys_define.h>

#include <ali/chunk.h>

#include <ali/dsc.h>
#include <ali/otp.h>
#include <ali/rsa_verify.h>
#include <ali/flash_cipher.h>
#include "flash_enc.h"
#include <ali/string.h>

#include <ali/sto_flash.h>
//#include <ali/ali_param.h>

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

extern R_RSA_PUBLIC_KEY g_rsa_public_key;

/*
* Test_RSA_Ram() - Verify the signature of input data , if pass verification, 
*                  return RET_SUCCESS, otherwise, return error NO.
*@addr: the start address for the check data, the data format need 
*        follow ALi format,use ALi sign tools can generate the special format
*@len:  data length for verify
*@Return :   
*        RET_SUCCESS means signature verfication passing otherwise fail. 
*/
RET_CODE test_rsa_ram(const UINT32 addr, const UINT32 len)
{
    UINT32 tmp_addr = 0;
    UINT32 addr_outputlen = 0;    
    UINT32 tmp_sha_len = 0;
    UINT8  *sha_signature;
    UINT8 ras_buf_in[292] = {0};
    UINT8 ras_buf_out[256] = {0};    
    RET_CODE  ret = RET_FAILURE;

    if( !length_ok(len) )
    {
        RSA_PRINTF("Invalid length!\n");
        return -1;   
    }

    if(!access_ok((void *)addr,len))
    {
        RSA_PRINTF("Invalid input buffer!\n");
        return -1;   
    } 
    sha_signature = malloc(64); // allocate 32bytes  of extra memory  to fix malloc issue
    if(sha_signature == NULL)
    {
        RSA_PRINTF("%s: malloc error\n", __FUNCTION__);
    }

    RSA_PRINTF("First 32 bytes SW public is:\n");
    RSA_DUMP(((UINT8*)&g_rsa_public_key),32);

    /* get signature from block tail*/
    tmp_addr = (addr + len - ALIASIX_RSA_COMPARE_LEN);
    MEMCPY(ras_buf_in, (UINT8 *)tmp_addr, ALIASIX_RSA_COMPARE_LEN);
    
    RSA_PRINTF("first 32 bytes rsa buf in: (%x,%x,%x)\n", addr, tmp_addr,len);
    RSA_DUMP(((UINT8 *)addr),32);
    
    ret = ali_rsapublicfunc(ras_buf_out, &addr_outputlen, ras_buf_in, ALIASIX_RSA_COMPARE_LEN, &g_rsa_public_key);
    if (RET_SUCCESS!= ret )
    {
        RSA_PRINTF("ali_rsapublicfunc() failed!(%x)\n",ret);
        return -1;
    }
    
    /* calc signature*/
    tmp_addr -= ALIASIX_RSA_COMPARE_LEN;
    tmp_sha_len = tmp_sha_len = len - ALIASIX_RSA_COMPARE_LEN;;
    if(tmp_sha_len>len)
    {
        RSA_PRINTF("Input data length error!\n");
        return -3;
    }
    ret = create_sha_ramdrv((UINT8 *)addr, tmp_sha_len, (UINT32 *)sha_signature);
    if (RET_SUCCESS != ret )
    {
        RSA_PRINTF("create_sha_ramdrv() failed!\n");
        return -4;
    }

    /*compare the result*/
    RSA_PRINTF("SHA digest is:\n");
    RSA_DUMP(sha_signature,sizeof(sha_signature));
    RSA_PRINTF("rsa result is:\n");
    RSA_DUMP(ras_buf_out,ALIASIX_RSA_COMPARE_LEN);    
    ret = ali_rsa_result_compare(ras_buf_out,(void *)sha_signature);
    if(ret!=RET_SUCCESS)
    {
        RSA_PRINTF("result compare error!\n");
        return -5;
    } 
    free(sha_signature);
    return RET_SUCCESS;
}

static void illegal_rsa_func4(void) 
{    
    hw_watchdog_reboot();
    do{}while(1);
}

RET_CODE test_vsc_rsa_ram(const UINT32 addr, const UINT32 len)
{
	UINT32 tmp_addr = 0;
	UINT32 addr_outputlen = 0;
	UINT32 tmp_sha_len = 0;
    UINT8  sha_signature[32];
    UINT8 ras_buf_in[292] = {0};
    UINT8 ras_buf_out[256] = {0};    
	RET_CODE  ret = RET_FAILURE;

	ret=get_sys_pub_key(PK_FROM_OTP,NULL,0);
    if(RET_SUCCESS != ret)
    {
		RSA_PRINTF("Fetch OTP public key fail!(%x)\n",ret);
		ret= -2;
	    reset_rsa_pub_key();
	    return ret;
    }

    RSA_PRINTF("First 32 bytes SW public is:\n");
	RSA_DUMP(((UINT8*)&g_rsa_public_key),32);

    // get signature from block tail
	tmp_addr = (addr + len - ALIASIX_RSA_COMPARE_LEN);
    MEMCPY(ras_buf_in, (UINT8 *)tmp_addr, ALIASIX_RSA_COMPARE_LEN);
    
    RSA_PRINTF("first 32 bytes rsa buf in: (%x,%x,%x)\n", addr, tmp_addr,len);
	RSA_DUMP(((UINT8 *)addr),32);
	ret = ali_rsapublicfunc(ras_buf_out, &addr_outputlen, ras_buf_in, ALIASIX_RSA_COMPARE_LEN, &g_rsa_public_key);
	if (RET_SUCCESS!= ret)
	{
		RSA_PRINTF("ali_rsapublicfunc() failed!(%x)\n",ret);
		ret=-1;
	    reset_rsa_pub_key();
	    return ret;
	}
	// calc signature

	tmp_sha_len=len-ALIASIX_RSA_COMPARE_LEN;
	ret = create_sha_ramdrv((UINT8 *)addr, tmp_sha_len, (UINT32 *)sha_signature);
    if (RET_SUCCESS != ret)
	{
		RSA_PRINTF("create_sha_ramdrv() failed!\n");
		ret=-4;
	    reset_rsa_pub_key();
	    return ret;
	}

    //compare the result
    RSA_PRINTF("SHA digest is:\n");
	RSA_DUMP(sha_signature,sizeof(sha_signature));
    RSA_PRINTF("rsa result is:\n");
	RSA_DUMP(ras_buf_out,ALIASIX_RSA_COMPARE_LEN);    
    ret = ali_rsa_result_compare(ras_buf_out,(void *)sha_signature);
    if(RET_SUCCESS != ret)
    {
        RSA_PRINTF("result compare error!\n");
        ret=-5;
	    reset_rsa_pub_key();
	    return ret;
    } 
	reset_rsa_pub_key();
    return RET_SUCCESS;
}
