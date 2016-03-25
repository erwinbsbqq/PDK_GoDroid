

#include <adr_basic_types.h>
#include <hld/soc/adr_soc.h>



int sys_reboot_get_timer(unsigned long *time_exp, unsigned long *time_cur)
{
    return soc_reboot_get_timer(time_exp, time_cur);
}


void sys_ic_enter_standby(unsigned int time_exp, unsigned int time_cur)
{
	soc_enter_standby(time_exp, time_cur);
}


void ali_sys_ic_get_bonding(void)
{
	soc_get_bonding();
}


unsigned long ali_sys_ic_get_product_id(void)
{
	return soc_get_product_id();
}


unsigned long ali_sys_ic_get_c3603_product(void)
{
    return soc_get_c3603_product();
}


unsigned long ali_sys_ic_get_chip_id(void)
{
    return soc_get_chip_id();
}


unsigned long ali_sys_ic_get_rev_id(void)
{
    return soc_get_rev_id();
}


unsigned long ali_sys_ic_get_cpu_clock(void)
{
    return soc_get_cpu_clock();
}


unsigned long ali_sys_ic_get_dram_clock(void)
{
    return soc_get_dram_clock();
}


int ali_sys_ic_get_usb_num(void)
{
    return soc_get_usb_num();
}

int ali_sys_ic_get_ci_num(void)
{
    return soc_get_ci_num();
}

int ali_sys_ic_get_tuner_num(void)
{
    return soc_get_tuner_num();
}

int ali_sys_ic_get_mac_num(void)
{
    return soc_get_mac_num();
}


int ali_sys_ic_dram_scramble_enabled(void)
{
    return soc_dram_scramble_enabled();
}

int ali_sys_ic_io_security_enabled(void)
{
    return soc_io_security_enabled();
}

int ali_sys_ic_split_enabled(void)
{
    return soc_split_enabled();
}

int aii_sys_ic_uart_enabled(void)
{
    return soc_uart_enabled();
}

int ali_sys_ic_ejtag_enabled(void)
{
    return soc_ejtag_enabled();
}

int ali_sys_ic_mv_is_enabled(void)
{
   return soc_mv_is_enabled();
}

int ali_sys_ic_ac3_is_enabled(void)
{
     return soc_ac3_is_enabled();
}

int ali_sys_ic_ddplus_is_enabled(void)
{
    return soc_ddplus_is_enabled();
}

int ali_sys_ic_XD_is_enabled(void)
{
    return soc_XD_is_enabled();
}

int ali_sys_ic_XDplus_is_enabled(void)
{
     return soc_XDplus_is_enabled();
}

int ali_sys_ic_aac_is_enabled(void)
{
     return soc_aac_is_enabled();
}


int ali_sys_ic_h264_is_enabled(void)
{
     return soc_h264_is_enabled();
}

int ali_sys_ic_mp4_is_enabled(void)
{
    return soc_mp4_is_enabled();
}


int ali_sys_ic_get_hd_enabled(void)
{
    return soc_get_hd_enabled();
}

int ali_sys_ic_hd_is_enabled(void)
{
    return soc_hd_is_enabled();
}

int ali_sys_ic_usb_port_enabled(unsigned long port)
{    
    return soc_usb_port_enabled(port);
}

int ali_sys_ic_sata_enable(void)
{
    return soc_sata_enable();
}

int ali_sys_ic_nim_support(void)
{
    return soc_nim_support();
}

int ali_sys_ic_nim_m3501_support(void)
{
    return soc_nim_m3501_support();
}

INT32 ali_soc_read(UINT8 * to, UINT8 * from, UINT32 len)
{
	return soc_read32(*from, to, len);
}

INT32 ali_soc_write(UINT8 * to, UINT8 * from, UINT32 len)
{
	return soc_write32(*to, from, len);
}

int ali_soc_reboot()
{
	return soc_reboot();
}

int ali_sys_ic_dsc_access_ce_disable(void)
{
   return  soc_dsc_access_ce_disable();
}









