#ifndef _ALI_SOC_H_
#define _ALI_SOC_H_


#include <alidefinition/adf_basic.h>


#define SYS_CPU_CLOCK                   (ali_sys_ic_get_cpu_clock() * 1000 * 1000)

#if defined(CONFIG_ARM)
#define ALI_SOC_BASE            0x18000000 
#elif defined(CONFIG_MIPS)
#define ALI_SOC_BASE            0xb8000000
#endif


void ali_sys_ic_get_bonding(void);
void sys_ic_enter_standby(unsigned int time_exp, unsigned int  time_cur);
void ali_sys_ic_enter_standby(unsigned long time_exp, unsigned long  time_cur);

int ali_sys_ic_get_ci_num(void);
int ali_sys_ic_get_mac_num(void);
int ali_sys_ic_get_usb_num(void);
int ali_sys_ic_get_tuner_num(void);
int ali_sys_ic_nim_m3501_support(void);
int ali_sys_ic_nim_support(void);
int ali_sys_ic_hd_is_enabled(void);
int ali_sys_ic_usb_port_enabled(unsigned long port);
int ali_sys_ic_get_hd_enabled(void);
int ali_sys_ic_sata_enable(void);
int ali_sys_ic_dram_scramble_enabled(void);
int ali_sys_ic_io_security_enabled(void);
int ali_sys_ic_split_enabled(void);
int aii_sys_ic_uart_enabled(void);
int ali_sys_ic_ejtag_enabled(void);
int ali_sys_ic_mv_is_enabled(void);
int ali_sys_ic_ac3_is_enabled(void);
int ali_sys_ic_ddplus_is_enabled(void);
int ali_sys_ic_XD_is_enabled(void);
int ali_sys_ic_XDplus_is_enabled(void);
int ali_sys_ic_aac_is_enabled(void);
int ali_sys_ic_h264_is_enabled(void);
int ali_sys_ic_mp4_is_enabled(void);
int ali_sys_ic_ms10_is_enabled(void);
int ali_sys_ic_ms11_is_enabled(void);
int ali_sys_ic_rmvb_is_enabled(void);
int ali_sys_ic_vc1_is_enabled(void);    
int ali_sys_ic_avs_is_enabled(void);
int ali_sys_ic_vp8_is_enabled(void);
int ali_sys_ic_flv_is_enabled(void);
int ali_sys_reboot_get_timer(unsigned long *time_exp, unsigned long *time_cur);
int ali_sys_ic_mg_is_enabled(void);
unsigned long ali_sys_ic_get_c3603_product(void);
unsigned long ali_sys_ic_get_product_id(void);
unsigned long ali_sys_ic_get_chip_id(void);
unsigned long ali_sys_ic_get_rev_id(void);
unsigned long ali_sys_ic_get_cpu_clock(void);
unsigned long ali_sys_ic_get_dram_clock(void); 
unsigned long ali_sys_ic_get_dram_size(void);
ADF_BOOT_BOARD_INFO * ali_get_user_data(void);
int ali_sys_ic_change_boot_type(unsigned int type);
int ali_sys_ic_revert_boot_type(void);



#endif 
