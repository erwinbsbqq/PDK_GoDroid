
#ifndef _ALI_DMX_HAL_H_
#define _ALI_DMX_HAL_H_

#include <linux/types.h>
#include <asm/mach-ali/typedef.h>
#include <ali_basic_common.h>
#include <linux/interrupt.h>


typedef char			DMX_INT8;
typedef unsigned char	DMX_UINT8;

typedef short			DMX_INT16;
typedef unsigned short	DMX_UINT16;

typedef int			    DMX_INT32;
typedef unsigned int	DMX_UINT32;


#define DMX_GET_DWORD(i)       (*(volatile __u32 *)(i))
#define DMX_SET_DWORD(i, d) do {(*(volatile __u32 *)(i)) = (d);} while(0)

#define DMX_GET_WORD(i)        (*(volatile __u16 *)(i))
#define DMX_SET_WORD(i, d)  do {(*(volatile __u16 *)(i)) = (d);} while(0)

#define DMX_GET_BYTE(i)        (*(volatile __u8 *)(i))
#define DMX_SET_BYTE(i, d)  do {(*(volatile __u8 *)(i)) = (d);} while(0)


#define DMX0_BASE_ADDR          0xb8022000
#define DMX1_BASE_ADDR          0xb8024000
#define DMX3_BASE_ADDR          0xb8028000

#define DMX_HW_TOTAL_UNITS                2
#define DMX_HW_MAX_TS_BUF_CNT             0x2000 /* 8K for M3606 series. */

#define DMX_HW_TOTAL_FILTERS              96


typedef struct _DMX_HW_FUNC
{
    __s32 (*dmx_hw_flt_pid_set)(__u32 base_addr, __u8  flt_idx, __u16 pid);
    __s32 (*dmx_hw_flt_enable)(__u32 base_addr, __u8 flt_idx);
    __s32 (*dmx_hw_flt_disable)(__u32 base_addr, __u8 flt_idx);
    __u32 (*dmx_hw_buf_rd_get)(__u32 dmx_base);
    __u32 (*dmx_hw_buf_rd_set)(__u32 dmx_base, __u32 rd);
    __u32 (*dmx_hw_buf_wr_get)(__u32 dmx_base);
    __s32 (*dmx_hw_pcr_get)(__u32 dmx_base, __u32 *pcr);
    __s32 (*dmx_hw_fifo_overflow_get)(__u32 dmx_base);
    __s32 (*dmx_hw_reg_reset)(__u32 base_addr, __u32 ts_buf_addr, __u32 ts_buf_pkt_cnt);
    __s32 (*dmx_hw_pcr_set_flt)(__u32 base_addr, __u8 flt_idx);
    __s32 (*dmx_hw_pcr_int_enable)(__u32 base_addr);
    __s32 (*dmx_hw_pcr_int_disable)(__u32 base_addr);
    __s32 (*dmx_hw_pcr_detect_enable)(__u32 base_addr);
    __s32 (*dmx_hw_pcr_detect_disable)(__u32 base_addr);
    __s32 (*dmx_hw_set_bypass_mode)(__u32 base_addr);
    __s32 (*dmx_hw_clear_bypass_mode)(__u32 base_addr);
    irqreturn_t (*dmx_hw_isr)(__s32 irq, __u8 *dev);
}T_DMX_HW_FUNC;

int dmx_hw_get_func(T_DMX_HW_FUNC* p_dmx_hw_func);

irqreturn_t m36g_dmx_hw_isr(int irq, void *dev);

irqreturn_t m36f_dmx_hw_isr(int irq, void *dev);
#endif

