
#include "dmx_hw.h"
#include "linux/dvb/ali_soc.h" 

extern __s32 m36f_dmx_hw_flt_pid_set(__u32 base_addr, __u8  flt_idx, __u16 pid);
extern __s32 m36f_dmx_hw_flt_enable(__u32 base_addr, __u8 flt_idx);
extern __s32 m36f_dmx_hw_flt_disable(__u32 base_addr, __u8 flt_idx);
extern __u32 m36f_dmx_hw_buf_rd_get(__u32 dmx_base);
extern __u32 m36f_dmx_hw_buf_rd_set(__u32 dmx_base, __u32 rd);
extern __u32 m36f_dmx_hw_buf_wr_get(__u32 dmx_base);
extern __s32 m36f_dmx_hw_pcr_get(__u32 dmx_base, __u32 *pcr);
extern __s32 m36f_dmx_hw_fifo_overflow_get(__u32 dmx_base);
extern __s32 m36f_dmx_hw_reg_reset(__u32 base_addr, __u32 ts_buf_addr, __u32 ts_buf_pkt_cnt);
extern __s32 m36f_dmx_hw_pcr_set_flt(__u32 base_addr, __u8 flt_idx);
extern __s32 m36f_dmx_hw_pcr_int_enable(__u32 base_addr);
extern __s32 m36f_dmx_hw_pcr_int_disable(__u32 base_addr);
extern __s32 m36f_dmx_hw_pcr_detect_enable(__u32 base_addr);
extern __s32 m36f_dmx_hw_pcr_detect_disable(__u32 base_addr);
extern __s32 m36f_dmx_hw_set_bypass_mode(__u32 base_addr);
extern __s32 m36f_dmx_hw_clear_bypass_mode(__u32 base_addr);
extern __s32 m36g_dmx_hw_flt_pid_set(__u32 base_addr, __u8  flt_idx, __u16 pid);
extern __s32 m36g_dmx_hw_flt_enable(__u32 base_addr, __u8 flt_idx);
extern __s32 m36g_dmx_hw_flt_disable(__u32 base_addr, __u8 flt_idx);
extern __u32 m36g_dmx_hw_buf_rd_get(__u32 dmx_base);
extern __u32 m36g_dmx_hw_buf_rd_set(__u32 dmx_base, __u32 rd);
extern __u32 m36g_dmx_hw_buf_wr_get(__u32 dmx_base);
extern __s32 m36g_dmx_hw_pcr_get(__u32 dmx_base, __u32 *pcr);
extern __s32 m36g_dmx_hw_fifo_overflow_get(__u32 dmx_base);
extern __s32 m36g_dmx_hw_reg_reset(__u32 base_addr, __u32 ts_buf_addr, __u32 ts_buf_pkt_cnt);
extern __s32 m36g_dmx_hw_pcr_set_flt(__u32 base_addr, __u8 flt_idx);
extern __s32 m36g_dmx_hw_pcr_int_enable(__u32 base_addr);
extern __s32 m36g_dmx_hw_pcr_int_disable(__u32 base_addr);
extern __s32 m36g_dmx_hw_pcr_detect_enable(__u32 base_addr);
extern __s32 m36g_dmx_hw_pcr_detect_disable(__u32 base_addr);
extern __s32 m36g_dmx_hw_set_bypass_mode(__u32 base_addr);
extern __s32 m36g_dmx_hw_clear_bypass_mode(__u32 base_addr);



int dmx_hw_get_func(T_DMX_HW_FUNC* p_dmx_hw_func)
{
    __u32 chip_ID;

    chip_ID = ali_sys_ic_get_chip_id();

    if(chip_ID == ALI_S3602F)
    {
        p_dmx_hw_func->dmx_hw_flt_pid_set       = m36f_dmx_hw_flt_pid_set       ;
        p_dmx_hw_func->dmx_hw_flt_enable        = m36f_dmx_hw_flt_enable        ;
        p_dmx_hw_func->dmx_hw_flt_disable       = m36f_dmx_hw_flt_disable       ;
        p_dmx_hw_func->dmx_hw_buf_rd_get        = m36f_dmx_hw_buf_rd_get        ;
        p_dmx_hw_func->dmx_hw_buf_rd_set        = m36f_dmx_hw_buf_rd_set        ;
        p_dmx_hw_func->dmx_hw_buf_wr_get        = m36f_dmx_hw_buf_wr_get        ;
        p_dmx_hw_func->dmx_hw_pcr_get           = m36f_dmx_hw_pcr_get           ;
        p_dmx_hw_func->dmx_hw_fifo_overflow_get = m36f_dmx_hw_fifo_overflow_get ;
        p_dmx_hw_func->dmx_hw_reg_reset         = m36f_dmx_hw_reg_reset         ;
        p_dmx_hw_func->dmx_hw_pcr_set_flt       = m36f_dmx_hw_pcr_set_flt       ;
        p_dmx_hw_func->dmx_hw_pcr_int_enable    = m36f_dmx_hw_pcr_int_enable    ;
        p_dmx_hw_func->dmx_hw_pcr_int_disable   = m36f_dmx_hw_pcr_int_disable   ;
        p_dmx_hw_func->dmx_hw_pcr_detect_enable = m36f_dmx_hw_pcr_detect_enable ;
        p_dmx_hw_func->dmx_hw_pcr_detect_disable= m36f_dmx_hw_pcr_detect_disable;
        p_dmx_hw_func->dmx_hw_set_bypass_mode   = m36f_dmx_hw_set_bypass_mode   ;
        p_dmx_hw_func->dmx_hw_clear_bypass_mode = m36f_dmx_hw_clear_bypass_mode ;
        p_dmx_hw_func->dmx_hw_isr               = m36f_dmx_hw_isr;
        return 0;
    }
    //else if(chip_ID == ALI_S3701c)
    else
    {
        p_dmx_hw_func->dmx_hw_flt_pid_set       = m36g_dmx_hw_flt_pid_set       ;
        p_dmx_hw_func->dmx_hw_flt_enable        = m36g_dmx_hw_flt_enable        ;
        p_dmx_hw_func->dmx_hw_flt_disable       = m36g_dmx_hw_flt_disable       ;
        p_dmx_hw_func->dmx_hw_buf_rd_get        = m36g_dmx_hw_buf_rd_get        ;
        p_dmx_hw_func->dmx_hw_buf_rd_set        = m36g_dmx_hw_buf_rd_set        ;
        p_dmx_hw_func->dmx_hw_buf_wr_get        = m36g_dmx_hw_buf_wr_get        ;
        p_dmx_hw_func->dmx_hw_pcr_get           = m36g_dmx_hw_pcr_get           ;
        p_dmx_hw_func->dmx_hw_fifo_overflow_get = m36g_dmx_hw_fifo_overflow_get ;
        p_dmx_hw_func->dmx_hw_reg_reset         = m36g_dmx_hw_reg_reset         ;
        p_dmx_hw_func->dmx_hw_pcr_set_flt       = m36g_dmx_hw_pcr_set_flt       ;
        p_dmx_hw_func->dmx_hw_pcr_int_enable    = m36g_dmx_hw_pcr_int_enable    ;
        p_dmx_hw_func->dmx_hw_pcr_int_disable   = m36g_dmx_hw_pcr_int_disable   ;
        p_dmx_hw_func->dmx_hw_pcr_detect_enable = m36g_dmx_hw_pcr_detect_enable ;
        p_dmx_hw_func->dmx_hw_pcr_detect_disable= m36g_dmx_hw_pcr_detect_disable;
        p_dmx_hw_func->dmx_hw_set_bypass_mode   = m36g_dmx_hw_set_bypass_mode   ;
        p_dmx_hw_func->dmx_hw_clear_bypass_mode = m36g_dmx_hw_clear_bypass_mode ;
        p_dmx_hw_func->dmx_hw_isr               = m36g_dmx_hw_isr;
        return 0;
    }

    return  -1;
}


