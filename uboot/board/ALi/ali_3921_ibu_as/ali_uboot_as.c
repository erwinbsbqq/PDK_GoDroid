#include <ali/retcode.h>
#include <ali/osal.h>
#include <ali/sys_config.h>
#include <ali/basic_types.h>
#include <ali/chunk.h>
#include <ali/sys_parameters.h>
//#include <ali/flash.h>
#include <ali/flash_cipher.h>
#include <ali/dsc.h>
#include <ali_nand.h>
#include <ali/sec_boot.h>

#include <ali/string.h>
//#include "ali_pmi_stbid.h"

#include <ali/ticks.h>
#include <ali/ali_param.h>

#define USEE_ADDR           0x87000200
#define SIG_LEN                 256
/*
         .data      :
	{
	  __USEE_ROM_START = .;
	  __USEE_ROM_END = .;
	}
*/
//extern UINT32 __USEE_ROM_START;
//extern UINT32 __USEE_ROM_END;

UINT32 __USEE_ROM_START;
UINT32 __USEE_ROM_END;

extern struct PMI   _gxPMI;

extern unsigned long load_addr ;	/* Default Load Address */
 unsigned long see_loadaddr ;


//#define FIXED_PRINTF(fmt,args...)
#define FIXED_PRINTF printf

extern struct sto_device *flash_dev ;
unsigned char g_uk_pos;

int ali_usee_boot()
{  
	INT32 usee_start, usee_end, usee_len;	

	usee_start = &__USEE_ROM_START;
    usee_end = &__USEE_ROM_END;
    usee_len = usee_end - usee_start;
   // FIXED_PRINTF("usee_start = 0x%x, usee_end = 0x%x, usee_len = 0x%x\n", usee_start, usee_end, usee_len);
#ifdef M3515_SECURE_BOOT_ENABLE  	
	set_see_code_info(usee_start,usee_len-SIG_LEN,USEE_ADDR);
	set_see_sig_info((UINT32)usee_end-SIG_LEN, SIG_LEN);
	set_see_key_info(g_uk_pos,0);
    if(RET_SUCCESS != bl_verify_SW_see(CASE3_SEE)){
        FIXED_PRINTF( "uboot see software verify failed\n" );        
        goto err_exit;
    }
    if(RET_SUCCESS != bl_run_SW_see()){
        FIXED_PRINTF( "uboot see software verify failed\n" );        
        goto err_exit;
    }
#else
	hld_dev_memcpy((UINT8 *)USEE_ADDR, (UINT8 *)&__USEE_ROM_START, usee_len);
    osal_delay(100);
	ali_see_boot(USEE_ADDR);
    FIXED_PRINTF("USee boot\n");
#endif

    dsc_api_attach();
    ce_api_attach();
	return RET_SUCCESS;
err_exit:
	return !RET_SUCCESS;
	
}

UINT32 is_bl_encrypted_enable(void)
{
    UINT32 shadom_reg_03 = 0;
    INT32 ret = 0;

 //   ret = sys_get_otp_from_shadom(0x03*4, &shadom_reg_03);
    if(RET_FAILURE == ret)
        return RET_FAILURE;
    ret = (shadom_reg_03 >> 24) & 0x1;
    return ret;
    
}

/**
*
* @brief     Prepare the universal key.
* @param[in] boot_type  0:nor boot, 1:nand boot
* @param[out] ukey      #UKEY_INFO system software universal key info
* @return RET_CODE
* @retval RET_SUCCESS   Success.
* @retval RET_FAILURE   Failure.
*
*/
RET_CODE bl_prepare_key(struct UKEY_INFO *ukey, struct transfer_params *t_param)
{
    UINT32 offset = 0;
    UINT32 key_pos = 0;
    UINT32 chid = 0;
    UINT32 chip_id=sys_ic_get_chip_id();
    RET_CODE ret = RET_FAILURE;
    CHUNK_HEADER chunk_hdr;
    //struct transfer_params *t_param = NULL;

    if (NULL == ukey)
    {
        FIXED_PRINTF("Error : %s, invalid parameter, pos is NULL!\n", __FUNCTION__);
        return ret;
    }
#if 0    
    t_param = malloc(sizeof(struct transfer_params));
    if(t_param == NULL)
    {
        FIXED_PRINTF("Error : %s, malloc error!\n", __FUNCTION__);
        return RET_FAILURE;
    }
#endif
#if 0    
    ret = get_parameter_from_bl(0, NULL, (void *)t_param);
    if(ret != RET_SUCCESS)
    {
        FIXED_PRINTF("Error : %s, get paramter error!\n", __FUNCTION__);
        free(t_param);
        return ret;
    }
#endif
    ret = decrypt_ukey_from_mem((UINT8 *)&key_pos, (void *)&(t_param->sys_uk), (NEW_HEAD_ID&NEW_HEAD_MASK|FIRST_FLASH_KEY));
    if(RET_SUCCESS != ret)
    {
        return ret;
    }
    ret = RET_SUCCESS;
    get_enc_uk_mode(&ukey->ukey_mode, ukey->enc_uk);
    //memcpy(&(ukey->enc_uk), &(t_param->sys_uk.cipher_sw_uk), ALIASIX_SW_UK_LEN);
   // ukey->ukey_mode = g_sw_uk_mode;
    ukey->kl_key_pos = key_pos;
    //if(t_param != NULL)
    //free(t_param);
    return ret;
}


struct sto_device*   g_sto_dev = NULL;
int ali_as_uboot_main(struct transfer_params *t_param)
{
	UINT32 chid,offset;
	UINT32 decryp_flag = 0xff, decrypt_flag2 = 0xff;	 //decrypt_flag2 for hw dmcrypt key
    UINT8 *logo_buf=NULL, *tmp_buf=NULL;
    UINT32 logo_loadlen =0;
	CHUNK_HEADER chunk_hdr;
    UINT8 *buf;
    UINT8 sys_sw_wrong = 0;

    struct UKEY_INFO ukey;
	

	FIXED_PRINTF("uboot see run success.\n");

	FIXED_PRINTF("Prepare universal Key start.\n");
	MEMSET(&ukey, 0x0, sizeof(UKEY_INFO));
	if(RET_SUCCESS != bl_prepare_key(&ukey, t_param))
	{
		FIXED_PRINTF("Error : %s, prepare key failed!\n", __FUNCTION__);
		goto exit;
	}
	FIXED_PRINTF("Prepare universal Key end.\n");

    g_uk_pos = (unsigned char)(ukey.kl_key_pos);
    if (load_kernel(&ukey) != 0)
    {
		FIXED_PRINTF("state machine error\n");            
		sys_sw_wrong = 1;
        goto exit ;
    }
exit:
    PERF_TRACE("Verify finish: %ld %d\n",read_us(), sys_sw_wrong);
    if(sys_sw_wrong == 1){
        printf("uboot verify failed, go to reboot\n");
        bl_panel_fail();
        hw_watchdog_reboot();
    }
 
    PERF_TRACE("ALi UBoot main exit %ld\n", 
                read_us());
    return TRUE;
}

