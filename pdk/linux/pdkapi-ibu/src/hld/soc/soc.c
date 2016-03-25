#include <osal/osal.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <sys_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <ali_soc_common.h>

                                  

static INT32 fd = -1;
static UINT32 soc_relay_func(UINT32 cmd, void *fp)
{
    if(fd < 0)
    {
    	fd = open("/dev/ali_soc", O_RDONLY);
    	if (fd < 0)
    	    return RET_FAILURE;
    }
    return ioctl(fd, cmd, fp);
}                                       

/*
 *
 *The API below specified for USER call!
 *
*/


int sys_reboot_get_timer(unsigned long *time_exp, unsigned long *time_cur)
{
    struct reboot_timer paras={time_exp, time_cur};
    soc_relay_func(ALI_SOC_REBOOT_GET_TIMER, &paras);
}

void sys_ic_enter_standby(unsigned int time_exp, unsigned int time_cur)
{
    struct boot_timer paras={time_exp, time_cur};
    soc_relay_func(ALI_SOC_ENTER_STANDBY, &paras);
}

void ali_sys_ic_get_bonding(void)
{
    soc_relay_func(ALI_SOC_GET_BONDING,NULL);
}

unsigned long ali_sys_ic_get_product_id(void)
{
    soc_relay_func(ALI_SOC_PRODUCT_ID,NULL);
}

unsigned long ali_sys_ic_get_c3603_product(void)
{
    soc_relay_func(ALI_SOC_C3603_PRODUCT,NULL);
}

unsigned long ali_sys_ic_get_chip_id(void)
{
    soc_relay_func(ALI_SOC_CHIP_ID,NULL);
}

unsigned long ali_sys_ic_get_rev_id(void)
{
    soc_relay_func(ALI_SOC_REV_ID,NULL);
}

unsigned long ali_sys_ic_get_cpu_clock(void)
{
    soc_relay_func(ALI_SOC_CPU_CLOCK,NULL);
}

unsigned long ali_sys_ic_get_dram_clock(void)
{
    soc_relay_func(ALI_SOC_DRAM_CLOCK,NULL);
}

int ali_sys_ic_get_usb_num(void)
{
    soc_relay_func(ALI_SOC_USB_NUM,NULL);
}

int ali_sys_ic_get_ci_num(void)
{
    soc_relay_func(ALI_SOC_CI_NUM,NULL);
}

int ali_sys_ic_get_tuner_num(void)
{
    soc_relay_func(ALI_SOC_TUNER_NUM,NULL);
}

int ali_sys_ic_get_mac_num(void)
{
    soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

/////////////////////////////////////////////////////

int ali_sys_ic_dram_scramble_enabled(void)
{
    soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

int ali_sys_ic_io_security_enabled(void)
{
    soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

int ali_sys_ic_split_enabled(void)
{
    soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

int aii_sys_ic_uart_enabled(void)
{
    soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

int ali_sys_ic_ejtag_enabled(void)
{
    soc_relay_func(ALI_SOC_MAC_NUM,NULL); 
}

int ali_sys_ic_mv_is_enabled(void)
{
   soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

int ali_sys_ic_ac3_is_enabled(void)
{
     soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

int ali_sys_ic_ddplus_is_enabled(void)
{
    soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

int ali_sys_ic_aac_is_enabled(void)
{
     soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}


int ali_sys_ic_h264_is_enabled(void)
{
     soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

int ali_sys_ic_mp4_is_enabled(void)
{
    soc_relay_func(ALI_SOC_MAC_NUM,NULL);
}

/////////////////////////////////////////////////////

int ali_sys_ic_get_hd_enabled(void)
{
    soc_relay_func(ALI_SOC_HD_ENABLED,NULL);
}

int ali_sys_ic_hd_is_enabled(void)
{
    soc_relay_func(ALI_SOC_HD_IS_ENABLED,NULL);
}

int ali_sys_ic_usb_port_enabled(unsigned long port)
{
    struct soc_usb_port usb_port = {port};
    soc_relay_func(ALI_SOC_USB_PORT_ENABLED,&usb_port);
}

int ali_sys_ic_sata_enable(void)
{
    soc_relay_func(ALI_SOC_SATA_EANBLE,NULL);
}

int ali_sys_ic_nim_support(void)
{
    soc_relay_func(ALI_SOC_NIM_SUPPORT,NULL);
}

int ali_sys_ic_nim_m3501_support(void)
{
    soc_relay_func(ALI_SOC_NIM_M3501_SUPPORT,NULL);
}

int ali_sys_ic_get_nand_boot(void)
{
    soc_relay_func(ALI_SOC_GET_BOOT_TYPE,NULL);
}

int ali_soc_read(unsigned char * to, unsigned char * from, unsigned int len)
{
      struct soc_op_paras soc_par = {to,from,len};  
      soc_relay_func(ALI_SOC_READ,&soc_par);
}

int ali_soc_write(unsigned char * to, unsigned char * from, unsigned int len)
{
      struct soc_op_paras soc_par = {to,from,len};  
      soc_relay_func(ALI_SOC_WRITE,&soc_par);
}

int ali_soc_reboot()
{

      struct soc_op_paras soc_par = {0,0,0};  
      soc_relay_func(ALI_SOC_REBOOT,&soc_par);
}

int ali_sys_ic_dsc_access_ce_disable(void)
{
   return  soc_relay_func(ALI_DSC_ACC_CE_DIS,NULL);
}
