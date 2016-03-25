#include <ali/retcode.h>
#include <ali/sys_define.h>
#include <ali/basic_types.h>
#include <ali/sys_config.h>
#include <ali_nand.h>
#include "ali_pmi_stbid.h"
#include <ali/sys_parameters.h>
#include <ali/osal.h>

//#include <configs/ali-stb.h>

#define FIXED_PRINTF printf


extern UINT32 __USEE_ROM_START;
extern UINT32 __USEE_ROM_END;

extern unsigned long load_addr ;	/* Default Load Address */
extern unsigned long see_loadaddr ;

extern struct PMI   _gxPMI;



static int ali_uboot_load_kernel()
{
    UINT32 cpu_mc_len=0, see_mc_len=0;
    UINT8 *kernel_buf, *see_tmp_buf, *tmp_buf;

    /*
    * Partition_1           upg loader kernel
    * Partition_2           upg loader see
    */
    /* load kernel */
    kernel_buf = (UINT8 *)load_addr ;
    FIXED_PRINTF("CPU Main from 0x%x,0x%x \n",(UINT32)kernel_buf, (UINT32)&kernel_buf);
    if (nand_load_data(tmp_buf, &kernel_buf, &cpu_mc_len, ALI_NAND_PART_1, MEM_TO_RUN) != 0) {
        FIXED_PRINTF("kernel load fail, return \n");
        goto exit;
    }

    /* load see */   
    ali_see_stop();
    see_tmp_buf = (UINT8 *)see_loadaddr ;
    FIXED_PRINTF("See from 0x%x,0x%x \n",(UINT32)see_tmp_buf, (UINT32)&see_tmp_buf);

    if (nand_load_data(tmp_buf, &see_tmp_buf, &see_mc_len, ALI_NAND_PART_2, MEM_TO_RUN) != 0) {
        FIXED_PRINTF("see load fail, return \n");
        goto exit;
    }

    return TRUE;
exit:
    FIXED_PRINTF("ERROR : uboot load kernel failed ...");
    return FALSE;
}

static int ali_uboot_load_upg_kernel()
{
    UINT32 cpu_mc_len=0, see_mc_len=0;
    UINT8 *kernel_buf, *see_tmp_buf, *tmp_buf;

    /*
    * Partition_last-1           upg loader kernel
    * Partition_last             upg loader see
    */
    /* load upg loader kernel */
    kernel_buf = (UINT8 *)load_addr ;
    FIXED_PRINTF("UPG loader main from 0x%x,0x%x \n",(UINT32)kernel_buf, (UINT32)&kernel_buf);
    if (nand_load_data(tmp_buf, &kernel_buf, &cpu_mc_len, _gxPMI.Partation_Num-2, MEM_TO_RUN) != 0)
    {
        FIXED_PRINTF("UPG loader main load fail, return \n");
        goto exit;
    }

    /* load upg loader see */
    ali_see_stop();
    see_tmp_buf = (UINT8 *)see_loadaddr ;
    FIXED_PRINTF("UPG loader see from 0x%x,0x%x \n",(UINT32)see_tmp_buf, (UINT32)&see_tmp_buf);
    if (nand_load_data(tmp_buf, &see_tmp_buf, &see_mc_len, _gxPMI.Partation_Num-1, MEM_TO_RUN) != 0)
    {
        FIXED_PRINTF("UPG loader see load fail, return \n");
        goto exit;
    }

    return TRUE;
exit:
    FIXED_PRINTF("ERROR : uboot load upg kernel failed ...");
    return FALSE;
}

static int ali_uboot_main_ex()
{
    UINT32 stmach_buf_len=0, cpu_mc_len=0, see_mc_len=0;
    struct state_machine_t stmach;
    UINT8 *logo_buf=NULL, *stmach_buf = NULL, *tmp_buf = NULL;   
    UINT32 logo_loadlen = 0, stmach_loadlen = 0, stmach_len = 0;
    UINT32 usee_start, usee_end, usee_len;

    /* Stop ASee, then run USee */
    ali_see_stop();
    usee_start = &__USEE_ROM_START;
    usee_end = &__USEE_ROM_END;
    usee_len = usee_end - usee_start;
	//FIXED_PRINTF("usee_start = 0x%x, usee_end = 0x%x, usee_len = 0x%x\n", usee_start, usee_end, usee_len);
    memcpy((UINT8 *)USEE_ADDR, (UINT8 *)&__USEE_ROM_START, usee_len);
    osal_cache_flush((UINT8 *)USEE_ADDR, usee_len);
    ali_see_boot(USEE_ADDR);
    FIXED_PRINTF("USee boot\n");

    /* init RPC */
//    ali_rpc_init();

    /* show logo */
    if(nand_load_data(tmp_buf, &logo_buf, &logo_loadlen, ALI_NAND_LOGO, MEM_TO_DEC) != 0)
    {
        FIXED_PRINTF("Logo load fail, return \n");
        goto exit;
    }
    else
    {
    #ifdef UBOOT_LOGO
        /* show logo */
        if (bl_show_logo(logo_buf, logo_loadlen) != RET_SUCCESS)
    	{
    		FIXED_PRINTF("Show logo fail\n");
            goto exit ;
    	}
	#endif	
    }
    free(tmp_buf);

    /* load state machine */
    if(nand_load_data(tmp_buf, &stmach_buf, &stmach_loadlen, ALI_NAND_STMACH, MEM_TO_DEC) != 0)
    {
        FIXED_PRINTF("State machine load fail, return \n");
        goto exit;
    }
    else
    {
        memcpy((UINT8 *)&stmach, stmach_buf, stmach_loadlen);

        FIXED_PRINTF("State Machine:\n");
        FIXED_PRINTF("b_boot_status = 0x%x;\n", stmach.b_boot_status);
        FIXED_PRINTF("b_lowlevel_status = 0x%x;\n", stmach.b_lowlevel_status);
        FIXED_PRINTF("b_application_status = 0x%x;\n", stmach.b_application_status);
        FIXED_PRINTF("b_bootloader_upgrade = 0x%x;\n", stmach.b_bootloader_upgrade);
        FIXED_PRINTF("b_lowlevel_upgrade = 0x%x;\n", stmach.b_lowlevel_upgrade);
        FIXED_PRINTF("b_application_upgrade = 0x%x;\n", stmach.b_application_upgrade);
        FIXED_PRINTF("b_bootloader_run_cnt = 0x%x;\n", stmach.b_bootloader_run_cnt);
        FIXED_PRINTF("b_lowlevel_run_cnt = 0x%x;\n", stmach.b_lowlevel_run_cnt);
        FIXED_PRINTF("b_application_run_cnt = 0x%x;\n", stmach.b_application_run_cnt);
        #if 0
        FIXED_PRINTF("b_need_upgrade = 0x%x;\n", stmach.b_need_upgrade);
        FIXED_PRINTF("b_backup_exist = 0x%x;\n", stmach.b_backup_exist);
        FIXED_PRINTF("b_lowlevel_backup_exist = 0x%x;\n", stmach.b_lowlevel_backup_exist);
        FIXED_PRINTF("b_boot_backup_exist = 0x%x;\n", stmach.b_boot_backup_exist);
        FIXED_PRINTF("b_nor_upgrade = 0x%x;\n", stmach.b_nor_upgrade);
        FIXED_PRINTF("b_nor_reserved = 0x%x;\n", stmach.b_nor_reserved);
        FIXED_PRINTF("b_nor_reserved_upgrade = 0x%x;\n", stmach.b_nor_reserved_upgrade);
        FIXED_PRINTF("b_nand_reserved = 0x%x;\n", stmach.b_nand_reserved);
        FIXED_PRINTF("b_nand_reserved_upgrade = 0x%x;\n", stmach.b_nand_reserved_upgrade);
        FIXED_PRINTF("b_nand_whole_upgrade = 0x%x;\n", stmach.b_nand_whole_upgrade);
        FIXED_PRINTF("\n");
        #endif
        FIXED_PRINTF("b_cur_uboot = 0x%x;\n", stmach.b_cur_uboot);
    }
    free(tmp_buf);

    if ((stmach.b_bootloader_upgrade == UPG_DESC_BOOT_UPG_NONE || \
        stmach.b_bootloader_upgrade == UPG_DESC_BOOT_UPG_NO || \
        stmach.b_bootloader_upgrade == UPG_DESC_BOOT_UPG_OVER || \
        stmach.b_bootloader_upgrade == UPG_DESC_BOOT_UPG_RUN) \
        && (stmach.b_lowlevel_upgrade == UPG_DESC_LOWLEVEL_UPG_NONE || \
        stmach.b_lowlevel_upgrade == UPG_DESC_LOWLEVEL_UPG_NO || \
        stmach.b_lowlevel_upgrade == UPG_DESC_LOWLEVEL_UPG_OVER) \
        && (stmach.b_application_upgrade == UPG_DESC_APP_UPG_NONE || \
        stmach.b_application_upgrade == UPG_DESC_APP_UPG_NO || \
        stmach.b_application_upgrade == UPG_DESC_APP_UPG_OVER))
    {
        goto no_upgrade;
    }
    else
    {
        goto load_upg_loader;
    }

no_upgrade:
    FIXED_PRINTF("no_upgrade\n");
    if ((stmach.b_bootloader_run_cnt < UPG_DESC_BOOT_UPG_MAX) && \
        (stmach.b_lowlevel_run_cnt < UPG_DESC_LOWLEVEL_UPG_MAX) && \
        (stmach.b_application_run_cnt < UPG_DESC_APP_UPG_MAX))
    {
        if (ali_uboot_load_kernel() != TRUE)
        {
            goto exit;
        }

        if (stmach.b_bootloader_upgrade || stmach.b_lowlevel_upgrade)
        {
            stmach.b_boot_status = UPG_DESC_BOOT_RUN_OVER;
            stmach.b_bootloader_run_cnt++;

            if (stmach.b_bootloader_upgrade == UPG_DESC_BOOT_UPG_RUN)
            {//finish uboot upgrading
                stmach.b_bootloader_upgrade = UPG_DESC_BOOT_UPG_NO;
                stmach.b_cur_uboot = ((stmach.b_cur_uboot == UPG_DESC_UBOOT2) ? UPG_DESC_UBOOT1 : UPG_DESC_UBOOT2);
            }

            if (stmach.b_lowlevel_upgrade)
            {//upgraded kernel and rootfs
                stmach.b_lowlevel_status = UPG_DESC_LOWLEVEL_RUN_ENTER;
            }

            stmach_len = sizeof(struct state_machine_t);
            /* write the state machine to nand flash*/
            if (nand_save_data((UINT8 *)&stmach, stmach_len, &stmach_loadlen, ALI_NAND_STMACH) != 0)
            {
                FIXED_PRINTF("State machine save fail, return\n");
                goto exit;
            }
        }
    }
    else
    {
load_upg_loader:
        FIXED_PRINTF("load_upg_loader\n");
        if (ali_uboot_load_upg_kernel() != TRUE)
        {
            goto exit;
        }
    }

    return TRUE;
exit:
    FIXED_PRINTF("ERROR : uboot load failed ...");
    return FALSE;
}

void ali_uboot_main() 
{
#ifndef _CAS9_CA_ENABLE_
    return ali_uboot_main_ex();
#else
	return ali_as_uboot_main();
#endif
}

