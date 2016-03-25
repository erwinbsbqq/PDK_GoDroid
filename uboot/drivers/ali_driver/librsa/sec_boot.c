#include <ali/basic_types.h>
#include <ali/sys_parameters.h>
#include <ali/osal.h>
#include <ali/string.h>
#include "./nn.h"
#include "./sec_boot.h"


UINT32 daemon_time = 50; // 5s

UINT32 g_copy_done_flg =0;
UINT32 g_ack_flg = 0;
static void clean_sw_ack_flag(void)
{
    g_ack_flg = 0;  
}

UINT32 try_sw_ack_flag(void)
{
    UINT32 ret ;
    ret = g_ack_flg ;
    return ret ;
}

static void set_sw_ack_flag(void)
{    
     g_ack_flg =1;    
}

static void clean_sw_copy_flag(void)
{
    g_copy_done_flg = 0; 
}

static UINT32 try_sw_copy_flag(void)
{
    UINT32 ret ;
    ret = g_copy_done_flg ;  
    return ret ;
}

static void set_sw_copy_flag(void)
{    
     g_copy_done_flg =1;  
}


struct SEEROM_PARAM g_see_param;
extern R_RSA_PUBLIC_KEY g_rsa_public_key;
extern R_RSA_PUBLIC_KEY g_u_rsa_public_key;

static RET_CODE set_see_parameter(const struct SEEROM_PARAM * see_param )
{
    *(volatile UINT32 *)(SEEROM_SEE_REAL_RUN_ADDR) = see_param->see_run_addr;
    *(volatile UINT32 *)(SEEROM_SEE_CODE_LEN_ADDR) = see_param->see_code_len ;
    *(volatile UINT32 *)(SEEROM_SEE_CODE_LOCATION_ADDR) = see_param->see_code_addr ;
    *(volatile UINT32 *)(SEEROM_SEE_SIGN_LOCATION_ADDR) = see_param->see_sign_location ;
//    *(volatile SIGN_FORMAT *)(SEEROM_SEE_SIGN_FORMAT_ADDR) = see_param->see_sign_format;
    *(volatile UINT32 *)(SEEROM_DEBUG_KEY_ADDR) = see_param->debug_key_addr;    
    return RET_SUCCESS;
}
static RET_CODE clean_see_parameter(struct SEEROM_PARAM * see_param)
{
    memset(see_param,0x0,sizeof(struct SEEROM_PARAM));
    return set_see_parameter(see_param);    
}

static int destroy_see_code(struct SEEROM_PARAM * see_param )
{
    return memset(see_param->see_code_addr, 0x0, see_param->see_code_len);
}
void set_see_code_info( UINT32 code_addr, UINT32 len, UINT32 run_addr)
{
    memset(&g_see_param, 0,sizeof(struct SEEROM_PARAM));
    g_see_param.see_code_addr = code_addr|(0xa0000000);
    g_see_param.see_code_len = len;
    g_see_param.see_run_addr = run_addr|(0xa0000000);
    osal_cache_flush(code_addr&0x8fffffff, len);
    return ;
}
void set_see_sig_info(UINT32 sig_location, UINT32 sig_len )
{
    UINT32 format = RSASSA_PKCS1_V1_5;
    g_see_param.see_sign_location = sig_location|(0xa0000000);
    g_see_param.see_sign_format.length = sig_len;
    g_see_param.see_sign_format.format = format;
    osal_cache_flush(sig_location&0x8fffffff, sig_len);
    return ;
}
SEE_KEY tmp_see_key;
RET_CODE set_see_key_info( UINT32 key_pos, UINT32 flag)
{
    SEE_KEY * see_key = (SEE_KEY*)&tmp_see_key;
#if 0
    SEE_KEY * see_key = (SEE_KEY*)malloc(sizeof(SEE_KEY));
    if(see_key == NULL){
        BOOT_ERR("%s :out of memory\n", __FUNCTION__);
        return RET_FAILURE;            
    }
#endif
    //fetch_sys_pub_key(0);
	if(flag){
	    osal_cache_flush(&g_rsa_public_key,sizeof(g_rsa_public_key));
	    see_key->see_sig_key =((UINT32)(&g_rsa_public_key))|(0xa0000000);
	}else{
		osal_cache_flush(&g_u_rsa_public_key,sizeof(g_u_rsa_public_key));
	    see_key->see_sig_key =((UINT32)(&g_u_rsa_public_key))|(0xa0000000);
	}
		
    see_key->uk_pos = key_pos ;
   // printf("g_see_param : 0x%x, g_see_param.debug_key_addr : 0x%x\n",g_see_param,g_see_param.debug_key_addr);
    g_see_param.debug_key_addr=((UINT32)see_key)|(0xa0000000) ;
    osal_cache_flush(see_key,sizeof(SEE_KEY));
    return RET_SUCCESS ;
}

/* main get sycn flag from see and then notify see 
* type : wait hardware sync type
* tmo  : timeout 
*/
UINT32 main_hw_ack_sync(UINT32 type,UINT32 tmo)
{
    UINT32 delay_100ms = tmo * 10;
    RET_CODE ret = RET_FAILURE;
    while(delay_100ms--){        
        /*try see sync reboot flag */  
        if (is_hw_reboot_true())
        {
            BOOT_DEBUG("[main_hw_ack_sync] : CSTM id/BL_version failed \n ");  
            hw_watchdog_reboot();
        }    
		if(is_hw_ack_fail_flag())
		{
			ret = RET_FAILURE;
			break;
		}
        if( is_hw_ack_flag_true(type))
		{
            ret = RET_SUCCESS;
            break;
        }
        osal_task_sleep(10);
    }
    if(ret == RET_FAILURE){
        BOOT_DEBUG("[main_hw_ack_sync] : wait hw ack flag : 0x%x failed \n ",type);  
        hw_watchdog_reboot();
    }            
    set_main_get_ack_flag(type);
    set_sw_ack_flag();
    osal_task_sleep(100);
    return RET_SUCCESS;
}


/*main cpu wait see_bl sync ack info*/ 
RET_CODE main_notify_see_boot(UINT32 param )
{
    RET_CODE ret = RET_FAILURE;

	seeboot_type type = param & 0xffff;
    struct SEEROM_PARAM * see_param ;
	
	BOOT_DEBUG("main_notify_see_boot: param = 0x%x ",param);
    see_param = &g_see_param;
    
    if((type == SEEROM_BOOT) ||\
       (type == SEE_SW_VERIFY) ){
        BOOT_DEBUG("step1 : set see parameter ...");            
        set_see_parameter(see_param);

        BOOT_DEBUG("main set see_boot parameter:");
        BOOT_DEBUG("see_run_addr : 0x%x\n", see_param->see_run_addr);    
        BOOT_DEBUG("see_code_addr : 0x%x\n", see_param->see_code_addr & 0x8<<28);
        BOOT_DUMP((UINT8*)(see_param->see_code_addr & 0x8<<28),0x20);
        BOOT_DEBUG("see_code_len : 0x%x\n", see_param->see_code_len);
        BOOT_DEBUG("see_sig_location : 0x%x\n", see_param->see_sign_location);
        BOOT_DUMP((UINT8*)(see_param->see_sign_location & 0x8<<28),0x20);
        BOOT_DEBUG("see_sig_format : 0x%x\n", see_param->see_sign_format);
		
    }else if( type == SEE_STATUS_RESET ){
		clean_sw_ack_flag();
        clean_sw_copy_flag();
		BOOT_DEBUG("step2 : see status reset \n");
		ret = hld_main_notify_see_trig(param); 
        if(ret != RET_SUCCESS){
             BOOT_ERR("BOOT_ERR: main wait see copy timeout\n ");
             goto reboot;
        }
		while(1) {
			if( get_hw_ack_flag() == 0)
				break;
		}		
		return RET_SUCCESS;	
		
    }

        clean_sw_ack_flag();
        clean_sw_copy_flag();
        BOOT_DEBUG("step2 : start see software boot \n"); 

        /*
        * sync see_sw verify status 
        */        
        if(type == SEE_SW_RUN){            
            ret = main_hw_ack_sync(SEE_ACK_BIT,daemon_time);
            if(ret != RET_SUCCESS){
                BOOT_ERR("bootloader check see_sw verify status failed\n");
                goto reboot;
            }            
        }        
        ret = hld_main_notify_see_trig(param); 
        if(RET_FAILURE == ret){
             BOOT_ERR("BOOT_ERR: main wait see copy timeout\n ");
             goto reboot;
        }
        
        if(type == SEE_SW_RUN){                      
             clean_see_parameter(see_param);
             while(1){
                if(is_hw_ack_flag_true(SEE_ROM_RUN_BIT))
                    break;
             }
             BOOT_DEBUG("step3 : see software boot-up done \n");    
             return RET_SUCCESS ;
        }
  
    
    /* polling see code copy done or timeout */
    UINT32 delay_for_see = daemon_time  ; 
    ret= RET_FAILURE ;
    while(delay_for_see --){        
        if( is_hw_copy_done() || try_sw_copy_flag())    
        {         
            ret =RET_SUCCESS;
            BOOT_DEBUG("main polling see copy-done flag pass\n");            
            clean_sw_copy_flag();
            clean_cpu_ack_register();
            break;
        }
        osal_task_sleep(100);        
    }
    if(ret != RET_SUCCESS){
        BOOT_ERR("BOOT_ERR: main wait see copy-done timeout\n ");
        goto reboot;
    }
    destroy_see_code(see_param);
    clean_see_parameter(see_param);

    BOOT_DEBUG("step3 : notify see boot-up done, need to check see status ...\n ");
           
    return ret ;
reboot:
    hw_watchdog_reboot();    //goto reboot;
    
}


/*
* Bootloader main cpu clean bootloader_SEE hardware status          
*        
*/
static RET_CODE bl_clean_see_status()
{
    RET_CODE ret=RET_FAILURE;
    
    ret = main_notify_see_boot( SEE_STATUS_RESET );
    if(ret != RET_SUCCESS){
        BOOT_ERR("bootloader sw_see boot-up failed\n");
        return !RET_SUCCESS;
    } 	
    return RET_SUCCESS ;
}

/*[API]: Bootloader main cpu starts SEE software. 
*        This funciton will notify SEE CPU jump from bootloader_SEE to software
*/
RET_CODE bl_run_SW_see()
{
    RET_CODE ret = RET_FAILURE;

    printf("Try to run see software\n");        
    ret = main_notify_see_boot( SEE_SW_RUN);
    if(ret != RET_SUCCESS){
        BOOT_ERR("bootloader bl_see boot-up failed, goto reboot...\n");
        return !RET_SUCCESS;
    }
    printf("Run see software boot-up work done \n");
    return ret ;
}

/*[API]: Bootloader main cpu trig bootloader_SEE to decrypt/verify/decompress 
         SEE software 
*        
*/
RET_CODE bl_verify_SW_see(UINT32 param)
{
    RET_CODE ret=RET_FAILURE;

	if(RET_SUCCESS != bl_clean_see_status()){
	    BOOT_ERR( "loader clean see status failed\n" );
	    return !RET_SUCCESS;
	}    
	
    ret = main_notify_see_boot( param|SEE_SW_VERIFY);
    if(ret != RET_SUCCESS){
        BOOT_ERR("bootloader sw_see boot-up failed, goto reboot...\n");
        hw_watchdog_reboot();
    }
    return ret ;
}