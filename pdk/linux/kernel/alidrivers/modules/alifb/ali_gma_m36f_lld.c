#ifndef CONFIG_RPC_HLD_GMA

#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
//#include <linux/smp_lock.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vt.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/fb.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld_dis.h>
#include <ali_soc_common.h>

#include "ali_gma_m36f_lld.h"
#include "ali_gma_filtgen.h"
#include "ali_fb.h"

#define osal_task_sleep     msleep
#define MEMCPY              memcpy

#ifndef MEMSET
#define MEMSET              memset
#endif

#ifndef FREE
#define FREE              kfree
#endif

#undef ASSERT
#define ASSERT(...) do{}while(0)

static UINT32 m_gma_chip_id;

//static struct ge_device *m_ge_dev;

static struct gma_m36f_private *m_gma_priv[GMA_M36F_LAYER_NUM];
static struct gma_m36f_private *gp_gma_m36f_private0;
static struct gma_m36f_private *gp_gma_m36f_private1;

static struct mutex m_gma_m36f_mutex;

#define ENTER_GMA_API()  mutex_lock(&m_gma_m36f_mutex)
#define LEAVE_GMA_API()  mutex_unlock(&m_gma_m36f_mutex)

// internal use
static RET_CODE gma_m36f_create_region_param (UINT32 layer_id, UINT32 region_id, pcgma_region_t pregion_param);
static RET_CODE gma_m36f_move_region_param (UINT32 layer_id, UINT32 region_id, pcgma_region_t pregion_param);
static RET_CODE _gma_m36f_set_region_pos (UINT32 layer_id, UINT32 region_id, gma_rect_t * rect);
static RET_CODE _gma_m36f_move_region_param (UINT32 layer_id, UINT32 region_id, pcgma_region_t pregion_param);
static RET_CODE gma_m36f_region_fill (UINT32 layer_id, UINT32 region_id, const gma_rect_t * rect, UINT32 uColorData);
static void gma_m36f_update_block_header (UINT32 layer_id, BOOL bUpdated);
static void gma_m36f_sync_block_header (struct gma_m36f_private *priv, BOOL wait_irq);

//static UINT8 m_osd_layer_attach_flag;
static const gma_scale_param_t m_gma_scale_off = { PAL, 1, 1, 1, 1 };
static const gma_scale_param_t m_gma_scale_p2n = { NTSC, 720, 576, 720, 480 };

//static const alifb_gma_region_t m_osd_default_mode = {GE_PF_CLUT8, FALSE, 0xff, 0};
static const gma_pal_attr_t m_gma_pal_attr_default = { GE_PAL_YCBCR, GE_RGB_ORDER_ACrCbY, GE_ALPHA_RANGE_0_15, GE_ALPHA_POLARITY_0 };


#define SYS_WriteDWord(uAddr, dwVal) \
							do{*(volatile UINT32 *)(uAddr) = (dwVal);}while(0)

#define SYS_ReadDWord(uAddr) \
							({ \
								volatile UINT32 dwVal; \
								dwVal = (*(volatile UINT32 *)(uAddr)); \
								dwVal; \
							})

static const UINT32 m_mask_nbit[] = {
    0x0, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
    0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff,
    0x1ffff, 0x3ffff, 0x7ffff, 0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
    0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff,
};

UINT32 set_dword_nbit (UINT32 data, UINT32 bit_num, UINT32 start_bit, UINT32 value)
{
    UINT32 mask = m_mask_nbit[bit_num];

    value &= mask;
    value <<= start_bit;
    data &= ~(mask << start_bit);

    return (data | value);
}

UINT32 get_dword_nbit (UINT32 data, UINT32 bit_num, UINT32 start_bit)
{
    UINT32 mask = m_mask_nbit[bit_num];

    data >>= start_bit;

    return (data & mask);
}

#define set_1bit(data, bit_pos, value)  set_dword_nbit(data, 1, bit_pos, value)
#define get_1bit(data, bit_pos)         get_dword_nbit(data, 1, bit_pos)

#define set_2bit(data, bit_pos, value)  set_dword_nbit(data, 2, bit_pos, value)
#define get_2bit(data, bit_pos)         get_dword_nbit(data, 2, bit_pos)

#define set_3bit(data, bit_pos, value)  set_dword_nbit(data, 3, bit_pos, value)
#define get_3bit(data, bit_pos)         get_dword_nbit(data, 3, bit_pos)

#define set_4bit(data, bit_pos, value)  set_dword_nbit(data, 4, bit_pos, value)
#define get_4bit(data, bit_pos)         get_dword_nbit(data, 4, bit_pos)

#define set_5bit(data, bit_pos, value)  set_dword_nbit(data, 5, bit_pos, value)
#define get_5bit(data, bit_pos)         get_dword_nbit(data, 5, bit_pos)

#define set_6bit(data, bit_pos, value)  set_dword_nbit(data, 6, bit_pos, value)
#define get_6bit(data, bit_pos)         get_dword_nbit(data, 6, bit_pos)

#define set_7bit(data, bit_pos, value)  set_dword_nbit(data, 7, bit_pos, value)
#define get_7bit(data, bit_pos)         get_dword_nbit(data, 7, bit_pos)

#define set_8bit(data, bit_pos, value)  set_dword_nbit(data, 8, bit_pos, value)
#define get_8bit(data, bit_pos)         get_dword_nbit(data, 8, bit_pos)

#define set_10bit(data, bit_pos, value)  set_dword_nbit(data, 10, bit_pos, value)
#define get_10bit(data, bit_pos)         get_dword_nbit(data, 10, bit_pos)

#define set_12bit(data, bit_pos, value) set_dword_nbit(data, 12, bit_pos, value)
#define get_12bit(data, bit_pos)        get_dword_nbit(data, 12, bit_pos)

#define set_16bit(data, bit_pos, value) set_dword_nbit(data, 16, bit_pos, value)
#define get_16bit(data, bit_pos)        get_dword_nbit(data, 16, bit_pos)

#define set_28bit(data, value)          set_dword_nbit(data, 28, 0, value)
#define get_28bit(data)                 get_dword_nbit(data, 28, 0)

UINT32 ge_m36f_get_hw_pf (enum GMA_PIXEL_FORMAT pf)
{
    UINT32 i = 0;
    UINT32 format = 0;

    for (i = 0; i < ARRAY_SIZE (m_gma_expf2inpf_36f); i += 2)
    {
        if (pf == m_gma_expf2inpf_36f[i])
            break;
    }

    if (i >= ARRAY_SIZE (m_gma_expf2inpf_36f))
        format = 0xFF;
    else
        format = m_gma_expf2inpf_36f[i + 1];

    return format;
}
enum GMA_PIXEL_FORMAT ge_m36f_get_sw_pf (UINT32 format)
{
    UINT32 i = 0;
    UINT32 pf = 0;

    for (i = 0; i < ARRAY_SIZE (m_gma_expf2inpf_36f); i += 2)
    {
        if (format == m_gma_expf2inpf_36f[i + 1])
            break;
    }

    if (i >= ARRAY_SIZE (m_gma_expf2inpf_36f))
        pf = 0xFF;
    else
        pf = m_gma_expf2inpf_36f[i];

    return (enum GMA_PIXEL_FORMAT)pf;
}

#define HW_HAL

static const UINT16 GMA_CTRL[] = { VPOST_GMA_F_CTRL, VPOST_GMA_S_CTRL };
static const UINT16 GMA_DMBA[] = { VPOST_DMBA_GMA_F, VPOST_DMBA_GMA_S };
static const UINT16 GMA_DMBA_HW[] = { VPOST_DMBA_GMA_F_HW, VPOST_DMBA_GMA_S_HW };
static const UINT16 GMA_K[] = { VPOST_GMA_F_K, VPOST_GMA_S_K };
static const UINT16 GMA_CP_INDEX[] = { VPOST_GMA_F_CP_INDEX, VPOST_GMA_S_CP_INDEX };
static const UINT16 GMA_CP[] = { VPOST_GMA_F_CP, VPOST_GMA_S_CP };
static const UINT16 GMA_H_COFF_INDEX[] = { VPOST_GMA_F_H_COFF_INDEX, VPOST_GMA_S_H_COFF_INDEX };
static const UINT16 GMA_H_COFF_DMSB[] = { VPOST_GMA_F_H_COFF_DMSB, VPOST_GMA_S_H_COFF_DMSB };
static const UINT16 GMA_H_COFF_DLSB[] = { VPOST_GMA_F_H_COFF_DLSB, VPOST_GMA_S_H_COFF_DLSB };
static const UINT16 GMA_V_COFF_INDEX[] = { VPOST_GMA_F_V_COFF_INDEX, VPOST_GMA_S_V_COFF_INDEX };
static const UINT16 GMA_V_COFF[] = { VPOST_GMA_F_V_COFF, VPOST_GMA_S_V_COFF };

static const UINT8 M32_GMA_DMBA[] = { M32_VPOST_OSD_DMBA, M32_VPOST_SUBT_DMBA };
static const UINT8 M32_GMA_CP_DATA[] = { M32_VPOST_OSD_CP_DATA, M32_VPOST_SUBT_CP_DATA };
static const UINT8 M32_GMA_FILTER[] = { M32_VPOST_OSD_FILTER, M32_VPOST_SUBT_FILTER };
static const UINT8 M32_GMA_OSD_X[] = { M32_VPOST_OSD_X, M32_VPOST_SUBT_X };
static const UINT8 M32_GMA_OSD_Y[] = { M32_VPOST_OSD_Y, M32_VPOST_SUBT_Y };

#ifndef _WINDOWS
#define gma_write_reg(base, offset, value) SYS_WriteDWord((base) + (offset), (value))
#define gma_read_reg(base, offset)         SYS_ReadDWord((base) + (offset))

#define GMA_PAL_STATIC

#else

#define GMA_PAL_STATIC

extern UINT8 m_gevm_pattern_type;

static void gma_write_reg (UINT32 base, UINT32 offset, UINT32 value)
{
    if (base == S3602F_GMA_REG_BASE || base == M3202C_GMA_REG_BASE)
    {
        if (m_gevm_pattern_type == 2)
            Hal_WriteIoSpaceD (IOBASE_VPOS, offset, value);
        //Ge_Hal_WriteIoD(offset, value);
    }
    else
    {
        base += offset;
        *(UINT32 *) (base) = value;
    }
}

static UINT32 gma_read_reg (UINT32 base, UINT32 offset)
{
    if (base == S3602F_GMA_REG_BASE || base == M3202C_GMA_REG_BASE)
    {
        if (m_gevm_pattern_type == 2)
        {
            UINT32 data = Hal_ReadIoSpaceD (IOBASE_VPOS, offset);
            //UINT32 data2 = Ge_Hal_ReadIoD(offset);
            return (data);
        }
        else
        {
            return 0;
            //return Ge_Hal_ReadIoD(offset);
        }
    }
    else
    {
        base += offset;
        return *(UINT32 *) (base);
    }
}

#endif

static UINT32 g_gma_reg_base_addr = 0;

static INLINE void gma_WriteDword (UINT32 uOffset, UINT32 uValue)
{
    gma_write_reg (g_gma_reg_base_addr, uOffset, uValue);
}

static INLINE UINT32 gma_ReadDword (UINT32 uOffset)
{
    return gma_read_reg (g_gma_reg_base_addr, uOffset);
}

static void gma_hal_SetMask (int gma_layer_id)
{
    if (m_gma_chip_id < ALI_S3602)
    {
        UINT32 mp_ctrl = gma_ReadDword (M32_VPOST_MP_CTRL);
        mp_ctrl = set_1bit (mp_ctrl, 0, 1); // SW is configuring VPOST
        gma_WriteDword (M32_VPOST_MP_CTRL, mp_ctrl);
    }
    
    if(m_gma_chip_id == ALI_C3701 || m_gma_chip_id == ALI_S3503)
    {
	    UINT32 data = 0;
	    UINT32 reg_offset = 0;

	    reg_offset = gma_layer_id == 0 ? 0x250 : 0x2D0;
	    data = gma_ReadDword(reg_offset);
	    data |= 1;
	    gma_WriteDword(reg_offset, data);
    }    
}

static void gma_hal_ClearMask (int gma_layer_id)
{
    if (m_gma_chip_id < ALI_S3602)
    {
        UINT32 mp_ctrl = gma_ReadDword (M32_VPOST_MP_CTRL);
        mp_ctrl = set_1bit (mp_ctrl, 0, 0); // SW config VPOST is ready
        gma_WriteDword (M32_VPOST_MP_CTRL, mp_ctrl);
    }
    if(m_gma_chip_id == ALI_C3701 || m_gma_chip_id == ALI_S3503)
    {
	    UINT32 data = 0;
	    UINT32 reg_offset = 0;

	    reg_offset = gma_layer_id == 0 ? 0x250 : 0x2D0;
	    data = gma_ReadDword(reg_offset);
	    data &= ~1;
	    gma_WriteDword(reg_offset, data);

    }
}

//bit0
static void gma_hal_OnOff (UINT8 layer, UINT8 uOn)
{
    UINT32 data, pos;

    if (m_gma_chip_id == ALI_M3202)
    {
        data = gma_ReadDword (M32_VPOST_GMA_CTRL);

        if (layer == 2)
            pos = 14;
        else if (layer == 1)
            pos = 16;
        else
            pos = 0;

        data = set_1bit (data, pos, uOn);
        gma_WriteDword (M32_VPOST_GMA_CTRL, data);
    }
    else
    {
        UINT32 data = gma_ReadDword (GMA_CTRL[layer]);

        if (uOn)
            data = set_1bit (data, 0, 1);
        else
            data = set_1bit (data, 0, 0);
        gma_WriteDword (GMA_CTRL[layer], data);
    }
}

static void gma_hal_filter_tap (UINT8 layer, UINT8 uTap)
{
    if (m_gma_chip_id == ALI_S3602)
    {
        UINT32 data = gma_ReadDword (GMA_CTRL[layer]);

        if (uTap == 3)
            data = set_1bit (data, 16, 1);
        else
            data = set_1bit (data, 16, 0);

        gma_WriteDword (GMA_CTRL[layer], data);
    }
    else if (m_gma_chip_id == ALI_S3811)
    {
        struct gma_m36f_private *priv = m_gma_priv[layer];
        struct gma_block_t *pHead = &priv->tBlockHead;
        struct gma_block_t *pBlock = pHead->next;
        //UINT32 header_id = priv->new_header_id;
        
        for (; pBlock && pBlock != pHead; pBlock = pBlock->next)
        {
            if (pBlock->uStatus == BLOCK_HIDE)
                continue;
            
            if (uTap == 3)
            {
                pBlock->block_head.filter_select = 1;
            }
            else
            {
                pBlock->block_head.filter_select = 0;
            }
        }
    }
}

static void gma_hal_monochrome (UINT8 layer, UINT8 uMono)
{
    if (m_gma_chip_id == ALI_S3602 || m_gma_chip_id == ALI_C3701 || m_gma_chip_id == ALI_S3811)
    {
        UINT32 data = gma_ReadDword (GMA_CTRL[layer]);

        if (uMono == 1)
            data = set_1bit (data, 17, 1);
        else
            data = set_1bit (data, 17, 0);

        gma_WriteDword (GMA_CTRL[layer], data);
    }
}

static UINT32 gma_hal_get_hw_BaseAddr (UINT8 layer)
{
    UINT32 uBaseAddr = 0;
    if (m_gma_chip_id == ALI_C3701 || m_gma_chip_id == ALI_S3503)
        uBaseAddr = gma_ReadDword (GMA_DMBA_HW[layer]);
    return uBaseAddr;
} 

static void gma_hal_BaseAddr (UINT8 layer, UINT32 uBaseAddr)
{
    if (m_gma_chip_id == ALI_M3202)
        gma_WriteDword (M32_GMA_DMBA[layer], uBaseAddr & 0x0fffffff );
    else
        gma_WriteDword (GMA_DMBA[layer], uBaseAddr & 0x0fffffff);
}

static UINT32 gma_hal_get_BaseAddr (UINT8 layer)
{
    UINT32 uBaseAddr;
    if (m_gma_chip_id == ALI_M3202)
        uBaseAddr = gma_ReadDword (M32_GMA_DMBA[layer]);
    else
        uBaseAddr = gma_ReadDword (GMA_DMBA[layer]);

    return uBaseAddr;
}

static void gma_hal_SetPalette (UINT8 layer, UINT8 uIndex, UINT8 uY, UINT8 uCb, UINT8 uCr)
{
    UINT32 uReg;
    if (m_gma_chip_id == ALI_M3202)
    {
        uReg = (uIndex << 24) | (uY << 16) | (uCb << 8) | uCr;
        gma_WriteDword (M32_GMA_CP_DATA[layer], uReg);
    }
}

static void gma_hal_VFilter (UINT8 layer, UINT32 uData)
{
    if (m_gma_chip_id == ALI_M3202)
    {
        gma_WriteDword (M32_GMA_FILTER[layer], uData);
    }
}

static void gma_hal_Pos (UINT8 layer, UINT16 uXStart, UINT16 uXEnd, UINT16 uYStart, UINT16 uYEnd)
{
    UINT32 data;

    if (m_gma_chip_id == ALI_M3202)
    {
        data = uXStart << 16;
        data |= uXEnd;
        gma_WriteDword (M32_GMA_OSD_X[layer], data);

        data = uYStart << 16;
        data |= uYEnd;
        gma_WriteDword (M32_GMA_OSD_Y[layer], data);
    }
}

static void gma_hal_VFilterOnOff (UINT8 layer, UINT8 uOn)
{
    UINT32 data;

    if (m_gma_chip_id == ALI_M3202 && layer == 0)
    {
        data = gma_ReadDword (M32_VPOST_GMA_CTRL);

        data = set_1bit (data, 25, uOn);

        gma_WriteDword (M32_VPOST_GMA_CTRL, data);
    }
}

static void gma_hal_GlobalAlpha_M32C (UINT8 layer, UINT8 uAlpha)
{
    UINT32 pos_en = 4;
    UINT32 pos_gk = 0;

    UINT32 mp_ctrl = gma_ReadDword (M32_VPOST_MP_CTRL);
    UINT32 global_k = gma_ReadDword (M32_VPOST_GLOBAL_K);

    if (layer == 1)
    {
        pos_en = 5;
        pos_gk = 8;
    }
    else if (layer == 2)
    {
        pos_en = 6;
        pos_gk = 16;
    }

    global_k = set_8bit (global_k, pos_gk, uAlpha);

    if (uAlpha < 0xff)
        mp_ctrl = set_1bit (mp_ctrl, pos_en, 1); // enable global alpha
    else
        mp_ctrl = set_1bit (mp_ctrl, pos_en, 0); // disable global alpha

    gma_WriteDword (M32_VPOST_GLOBAL_K, global_k);
    gma_WriteDword (M32_VPOST_MP_CTRL, mp_ctrl);
}

static void gma_hal_GlobalAlpha (UINT8 layer, UINT8 uAlpha)
{
    //3701 3606 3503

    UINT32 data = gma_ReadDword (GMA_K[layer]);

    data &= 0xfffeff00;
    data |= uAlpha;
    if (uAlpha < 0xff)
    {
        data |= 0x00010000; // enable global alpha
        gma_WriteDword (GMA_K[layer], data);
    }
    else
    {
        gma_WriteDword (GMA_K[layer], data);
    }
    
}

static void gma_hal_anti_flicker_onoff (UINT8 layer, UINT8 uOn)
{
    UINT32 data, pos;
    if(m_gma_chip_id == ALI_S3503)
    {
        if(uOn == 1)
        {
            data = gma_ReadDword(VPOST_MIX_CTRL);            
            data = set_3bit(data, 12, 7);            
            //data = set_1bit(data, 14 - layer, 0);
            gma_WriteDword(VPOST_MIX_CTRL, data); 
            
            SYS_WriteDWord(0xb8006070, 0x01000000);
            SYS_WriteDWord(0xb8006074, 0x01000000);
            SYS_WriteDWord(0xb80061d8, 0x00010010);
            
        }
        else if(uOn == 0)
        {
            data = gma_ReadDword(VPOST_MIX_CTRL);            
            data = set_3bit(data, 12, 0);            
            //data = set_1bit(data, 14 - layer, 0);
            gma_WriteDword(VPOST_MIX_CTRL, data); 
            
            SYS_WriteDWord(0xb8006070, 0x0);
            SYS_WriteDWord(0xb8006074, 0x0);
            SYS_WriteDWord(0xb80061d8, 0x0);
        }
    }
    else//for 3606/3603
    {
        if(uOn == 1)
        {
            data = gma_ReadDword(VPOST_MIX_CTRL);
            data = set_3bit(data, 12, 6);
            //data = set_1bit(data, 14 - layer, 1);           
            //data = set_1bit(data, 14 - layer, 0);
            gma_WriteDword(VPOST_MIX_CTRL, data); 
            
            SYS_WriteDWord(0xb8006070, 0x01000000);
            SYS_WriteDWord(0xb8006074, 0x01000000);
            SYS_WriteDWord(0xb80061d8, 0x00000001);
            
        }
        else if(uOn == 0)
        {
            data = gma_ReadDword(VPOST_MIX_CTRL);  
            data = set_3bit(data, 12, 0);
            //data = set_1bit(data, 14 - layer, 0);          
            //data = set_1bit(data, 14 - layer, 0);
            gma_WriteDword(VPOST_MIX_CTRL, data); 
            
            SYS_WriteDWord(0xb8006070, 0x0);
            SYS_WriteDWord(0xb8006074, 0x0);
            SYS_WriteDWord(0xb80061d8, 0x0);
        }
    }

   
}
void set_gma_line_buf_par(UINT32 value)
{
	gma_WriteDword(0x2B8, value);
}

void set_gma_csc_bypass(int gma_layer_id, int bypass)
{
    UINT32 data = gma_ReadDword (GMA_CTRL[gma_layer_id]);
    if (bypass)
        data = set_1bit (data, 19, 1);
    else
        data = set_1bit (data, 19, 0);
    gma_WriteDword (GMA_CTRL[gma_layer_id], data);
}

void set_gma_enhance_sharpness(int gma_layer_id, int enable)
{
    UINT32 data = gma_ReadDword (GMA_CTRL[gma_layer_id]);
    if (enable)
        data = set_1bit (data, 18, 1);
    else
        data = set_1bit (data, 18, 0);
    gma_WriteDword (GMA_CTRL[gma_layer_id], data);
}


#define HW_BLOCKS

static struct gma_block_t m_osd_block[GMA_MAX_BLOCK];
static UINT8 m_block_status[GMA_MAX_BLOCK];
static UINT8 m_block_inited;
ATTR_ALIGN_32 static UINT32 m_osd_block_head[(GMA_MAX_BLOCK + 1) * GMA_HEADER_QUEUE_SIZE * HEADER_DW_NUM];
static UINT32 m_gma_block_head_start_addr;

#ifdef GMA_PAL_STATIC
static UINT8 m_osd_pal_buf0[GMA_M36F_LAYER_NUM*2*1024+32+256];
static UINT8 *m_osd_pal_buf1;
#endif

#ifdef _WINDOWS

#define OSD_BLOCK_HEAD_SIZE         0x1000
//#define m_osd_block_head            (const void *)(MAX_FPGA_MEM_SIZE-OSD_BLOCK_HEAD_SIZE)
#define GMA_BLOCK_HEAD_FPGA_ADDR    (MAX_FPGA_MEM_SIZE-OSD_BLOCK_HEAD_SIZE)

static gevm_fpga_data_t2 m_gevm_gma_header2 = { m_osd_block_head, sizeof (m_osd_block_head), GMA_BLOCK_HEAD_FPGA_ADDR, 32 };
static const gevm_fpga_data_t m_gevm_gma_header = { m_osd_block_head, sizeof (m_osd_block_head) };
static const gevm_fpga_data_t m_gevm_pal_buf = { m_osd_pal_buf0, sizeof (m_osd_pal_buf0) };

static void write_gma_header_to_fpga (void)
{
    //gevm_write_data_to_fpga (&m_gevm_pal_buf, 1);
    gevm_write_data_to_fpga (&m_gevm_gma_header, 1);
    gevm_write_data_to_fpga2 (&m_gevm_gma_header2, 1);
}

static void write_gma_pal_to_fpga(void)
{
    gevm_write_data_to_fpga(&m_gevm_pal_buf, 1);
}

static UINT8 *get_cache_addr (const void *ptr)
{
    return (UINT8 *) ptr;
}

static UINT8 *get_none_cache_addr (const void *ptr)
{
    return (UINT8 *) ptr;
}

#else
#define write_gma_header_to_fpga(...)
#define write_gma_pal_to_fpga(...)

static UINT8 *get_cache_addr (const void *ptr)
{
    UINT32 addr = (UINT32) ptr;

    addr &= 0x8fffffff;

    return (UINT8 *) addr;
}

static UINT8 *get_none_cache_addr (const void *ptr)
{
    UINT32 addr = (UINT32) ptr;

    addr &= 0x8fffffff;
    addr |= 0xa0000000;

    return (UINT8 *) addr;
}

#endif

static void gma_m36f_init_block (void)
{
    UINT32 i;
    UINT32 addr;
    struct gma_block_t *block = NULL;
    
    if (m_block_inited)
        return;
    
    addr = (UINT32) m_osd_block_head;
    block = m_osd_block;
    addr = (addr + 31) & 0xffffffe0; // 32 bytes aligned
    m_gma_block_head_start_addr = addr;
    addr = (UINT32) get_none_cache_addr ((void *) addr);
    for (i = 0; i < GMA_MAX_BLOCK; i++, block++)
    {
        block->puHeadBuf[0] = addr;
        addr += HEADER_SIZE;
        block->puHeadBuf[1] = addr;
        addr += HEADER_SIZE;
    }
    MEMSET (m_block_status, 0, sizeof (m_block_status));
    m_block_inited = TRUE;
#ifdef _WINDOWS
    m_gevm_gma_header2.data_addr = (const void *)m_gma_block_head_start_addr;
#endif
}

static struct gma_block_t *gma_m36f_get_block (int size)
{
    UINT32 i;
    struct gma_block_t *p = m_osd_block;
    for (i = 0; i < GMA_MAX_BLOCK; i++, p++)
    {
        if (m_block_status[i] == 0)
        {
            m_block_status[i] = 1;
            return p;
        }
    }
    return NULL;
}

static void gma_m36f_release_block (struct gma_block_t *block)
{
    UINT32 i;
    struct gma_block_t *p = m_osd_block;
    for (i = 0; i < GMA_MAX_BLOCK; i++, p++)
    {
        if (p == block)
            break;
    }
    if (i == GMA_MAX_BLOCK)
        return;
    m_block_status[i] = 0;
}

static void gma_m36f_init_head (gma_head_m36f_t * phead)
{
    MEMSET (phead, 0, sizeof (gma_head_m36f_t));

    phead->ep_avg_thr = 170;
    phead->ep_reduce_thr = 32;
    phead->gma_mode = GMA36F_PF_ARGB8888;
    phead->clut_mode = 1;       // DMA mode

    phead->last_block = 1;
    phead->global_alpha = 0xff;
    phead->color_by_color = 1;
    phead->pre_mul = 1;
}

static void gma_fill_hw_head (UINT8 uOSDLayer, struct gma_block_t *Block_Head, struct gma_block_t *blk)
{
    struct gma_m36f_private *priv = NULL;
    UINT32 i;
    UINT32 next_addr;
    struct gma_block_t *next_blk;
    gma_head_m36f_t *phead = &blk->block_head;

    for (i = 0; i < GMA_M36F_LAYER_NUM; i++)
    {
        //if (m_gma_priv[i]->uOSDLayer == uOSDLayer)
        {
            priv = m_gma_priv[uOSDLayer];
            break;
        }
    }

    if (priv == NULL)
        return;

    next_blk = blk->next;
    while (next_blk != Block_Head)
    {
        if (next_blk->uStatus == BLOCK_SHOW)
            break;
        next_blk = next_blk->next;
    }
    
    if (next_blk == Block_Head)
    {
        next_addr = 0;
        phead->last_block = 1;
    }
    else
    {
        phead->last_block = 0;
        next_addr = (UINT32) next_blk->puHeadBuf[priv->new_header_id];
    }
#ifdef _WINDOWS
    if (next_addr != 0)
    {
        UINT32 addr0 = (UINT32)m_gma_block_head_start_addr;
        next_addr = GMA_BLOCK_HEAD_FPGA_ADDR + (next_addr - addr0);
    }
    phead->bitmap_addr = get_fpga_addr(blk->puBuf);
#endif
    phead->next_head = next_addr & 0x0fffffff;

    if (blk != Block_Head)
    {
        UINT32 blk_addr = (UINT32) blk->puHeadBuf[priv->new_header_id];
#ifndef _WINDOWS
        // now we use none-cache address, not need flush cache
#if 0
        blk_addr &= 0x8fffffe0; // block head addr must be 32 bytes aligned
        osal_cache_invalidate (blk_addr, HEADER_SIZE);
        blk_addr |= 0xa0000000;
#endif
#endif
        MEMCPY ((UINT8 *) blk_addr, phead, sizeof (gma_head_m36f_t));
        MEMCPY ((UINT8 *) blk_addr+576, priv->enhance_coef_table, 52); //modefied for 3701c
    }
    else if (next_addr)
    {
        if (m_gma_chip_id < ALI_S3602)
        {
            //ASSERT(next_blk != Block_Head);
            phead = &next_blk->block_head;
            gma_hal_SetMask (uOSDLayer);
            gma_hal_Pos (uOSDLayer, phead->start_x, phead->end_x, phead->start_y, phead->end_y);
            gma_hal_BaseAddr (uOSDLayer, next_addr);
            gma_hal_ClearMask (uOSDLayer);
        }
        else
        {
            priv->first_block = next_blk;
            priv->first_block_changed = TRUE;
        }
    }
}


#define ROUND(x) (UINT32)(x)
#define ROUND2(x) (UINT32)(x+0.5)

static UINT32 gma_round (float fvalue)
{
    return ROUND2 (fvalue);
}

#if 0
static void scale_rect (const gma_rect_t * src_rect, gma_rect_t * dst_rect, UINT32 h_div, UINT32 v_div, UINT32 h_mul, UINT32 v_mul)
{
    float x, y, w, h;
    float dx, dy, dx_end, dy_end;
    unsigned int ix, iy, ix_end, iy_end;

    x = src_rect->left;
    y = src_rect->top;
    w = src_rect->width;
    h = src_rect->height;

    dx = x * (float) h_mul / (float) h_div;
    dx_end = (x + w) * (float) h_mul / (float) h_div - 1;

    dy = y * (float) v_mul / (float) v_div;
    dy_end = (y + h) * (float) v_mul / (float) v_div - 1;

    ix = gma_round (dx);
    iy = gma_round (dy);
    ix_end = gma_round (dx_end)+1;
    iy_end = gma_round (dy_end)+1;

    if (ix & 1)
    {
        ix--;
        ix_end--;
        if (ix_end & 1)
            ix_end++;
    }

    if (iy & 1)
    {
        iy--;
        iy_end--;
        if (iy_end & 1)
            iy_end++;
    }

    dst_rect->left = ix;
    dst_rect->top = iy;
    dst_rect->width = ix_end-1;
    dst_rect->height = iy_end-1;

}
#endif
#if 0
UINT32 gma_test_scale_rect(UINT16 src_h, UINT16 dst_h, UINT32 h_div, UINT32 v_div, UINT32 h_mul, UINT32 v_mul)
{
    UINT16 y0, end_y0;
    gma_rect_t src_rect0, dst_rect0;
    gma_rect_t src_rect1, dst_rect1;
    UINT32 bad = 0;

    src_rect0.left = 100;
    src_rect0.width = 300;

    src_rect1.left = 100;
    src_rect1.width = 300;

    for (y0 = 0; y0 < src_h; y0++)
    {
        src_rect0.top = y0;
        for (end_y0 = y0 + 2; end_y0 < src_h; end_y0++)
        {
            src_rect0.height = end_y0 - y0;
            src_rect1.top = end_y0;
            src_rect1.height = src_h - src_rect1.top;
            if (src_rect1.height < 2)
                continue;
            scale_rect(&src_rect0, &dst_rect0, h_div, v_div, h_mul, v_mul);
            scale_rect(&src_rect1, &dst_rect1, h_div, v_div, h_mul, v_mul);

            if ((dst_rect0.height + 1) != dst_rect1.top)
            {
                bad++;
            }
        }
    }

    for (y0 = 0; y0 < src_h; y0++)
    {
        src_rect0.top = y0;
        src_rect0.height = src_h - y0;
        scale_rect(&src_rect0, &dst_rect0, h_div, v_div, h_mul, v_mul);

        if ((dst_rect0.height + 1) != dst_h)
        {
            bad++;
        }
    }

    return bad;
}
#endif

static void gma_m36f_set_head_rect (struct gma_m36f_private *priv, gma_head_m36f_t * phead, const gma_rect_t * prect)
{
    UINT32 x, y, w, h;
    UINT8 scale_dir = priv->scale_dir;
    gma_rect_t dis_rect;

    x = prect->left;
    y = prect->top;
    w = prect->width;
    h = prect->height;

    phead->source_width = w;
    phead->source_height = h;

#if 1
    phead->start_x = x;//ROUND (x * GMA_SCALE_FACTOR_H);
    phead->start_y = y;//ROUND (y * GMA_SCALE_FACTOR_V);
    phead->end_x   = x + ROUND(w * GMA_SCALE_FACTOR_H) - 1;//ROUND ((x + w) * GMA_SCALE_FACTOR_H) - GMA_SCALE_FACTOR_H;
    phead->end_y   = y + ROUND(h * GMA_SCALE_FACTOR_V) - 1;//ROUND ((y + h) * GMA_SCALE_FACTOR_V) - GMA_SCALE_FACTOR_V;
#else
    scale_rect (prect, &dis_rect, priv->h_div, priv->v_div, priv->h_mul, priv->v_mul);
    phead->start_x = dis_rect.left;
    phead->start_y = dis_rect.top;
    phead->end_x   = dis_rect.width;
    phead->end_y   = dis_rect.height;
#endif

    phead->scale_mode = priv->scale_mode;
    if (scale_dir == GMA_SCALE_OFF)
    {
        phead->scale_on = 0;
    }
    else if (scale_dir == GMA_SCALE_UP)
    {
        phead->scale_on = 1;
        phead->ep_on = priv->ep_on;
        phead->incr_h_fra = get_12bit (priv->h_scale_param, 0);
        phead->incr_h_int = get_4bit (priv->h_scale_param, 12);
        phead->incr_h_fra = get_12bit (priv->h_scale_param, 0);
        phead->incr_h_int = get_4bit (priv->h_scale_param, 12);
    }
    else if (scale_dir == GMA_SCALE_DOWN)
    {
        phead->scale_on = 1;
        phead->ep_on = 0;
    }
    if (m_gma_chip_id == ALI_M3202)
    {
        if (priv->h_mul == priv->h_div * 2)
            phead->scale_on = 1;
        else
            phead->scale_on = 0;
    }
    if (scale_dir != GMA_SCALE_OFF)
    {
        phead->incr_h_fra = get_12bit (priv->h_scale_param, 0);
        phead->incr_h_int = get_4bit (priv->h_scale_param, 12);
        phead->incr_v_fra = get_12bit (priv->v_scale_param, 0);
        phead->incr_v_int = get_4bit (priv->v_scale_param, 12);
    }
}

static UINT8 *m_last_buffer;
static struct gma_block_t *gma_block_create (UINT32 layer_id, gma_rect_t * r, UINT8 * puBuf, UINT32 uSize, const alifb_gma_region_t * pregion_info)
{
    struct gma_block_t *block_cur, *block_last, *block_next, *block;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    struct gma_block_t *Block_Head = &(priv->tBlockHead);
    gma_head_m36f_t *phead;
    UINT32 addr;

    GMA_PRINTF ("gma_block_create :\n");

    if ((block_cur = gma_m36f_get_block (sizeof (struct gma_block_t))) == NULL)
    {
        ASSERT (0);             //return RET_FAILURE;
    }

    MEMCPY (&(block_cur->tRect), r, sizeof (gma_rect_t));

    phead = &block_cur->block_head;
    gma_m36f_init_head (phead);

    gma_m36f_set_head_rect (priv, phead, r);
    //MEMCPY((void *)phead + 576, priv->enhance_coef_table, 52);//modified for 3701c
    phead->gma_mode = pregion_info->color_format;
    addr = (UINT32) puBuf;
    phead->bitmap_addr = addr & 0x0fffffff; // must and it
    phead->pitch = uSize / r->height;
     
    if (phead->gma_mode >= GMA_PF_CLUT2 && phead->gma_mode <= GMA_PF_ACLUT88)
    {
        addr = (UINT32) priv->puPalletteHW;
        ASSERT ((addr & 0x1f) == 0); // CLUT must be 32 bytes aligned
        phead->clut_base = addr & 0x0fffffff; // must and it
        phead->clut_segment = pregion_info->pallette_sel;
        phead->clut_update = 1;
    }
    else
    {
        phead->clut_base = 0;
        phead->clut_update = 0;
    }

#if 1
    phead->global_alpha = pregion_info->global_alpha;
    phead->color_by_color = 1 - (pregion_info->galpha_enable & 1);

    block_cur->puBuf = puBuf;
    block_cur->uSize = uSize;
#endif

    block_cur->uStatus = BLOCK_SHOW;

    if (priv->uBlockInitialized == 0)
    {
        Block_Head->next = Block_Head;
        Block_Head->prev = Block_Head;
        MEMSET (&(Block_Head->tRect), 0, sizeof (gma_rect_t));
        priv->uBlockInitialized = 1;
    }


    block = Block_Head->prev;
    while (block != Block_Head)
    {
        if ((block->tRect.top) > (block_cur->tRect.top))
            block = block->prev;
        else
            break;
    }

    block_last = block;
    block_next = block->next;

    block_cur->next = block_next;
    block_cur->prev = block_last;
    block_last->next = block_cur;
    block_next->prev = block_cur;

    // Clear display buffer before it takes effect -- Jerry Long
    if (puBuf == m_last_buffer)
    {
        //libc_printf("%s, %d: delaying in osd_m36f\n", __FUNCTION__, __LINE__);
        //dly_tsk(50);
        osal_task_sleep (1);
    }

#if 0                           // We will do the following steps after returning to osd_create_region()

    UINT32 color = 0;
    if (uColorMode == GE36F_PF_CLUT8)
        color = (UINT8) priv->trans_color;
    else if (uColorMode == GE36F_PF_CLUT4 || uColorMode == GE36F_PF_CLUT2)
        color = 0xff;
    //MEMSET(puBuf, color, uSize);

    gma_rect_t fill_rect = *r;
    fill_rect.left = 0;
    fill_rect.top = 0;

    gma_m36f_region_fill32 (layer_id, 0, &fill_rect, color);

#if 0
    gma_fill_hw_head (priv->uOSDLayer, Block_Head, block_last);
    gma_fill_hw_head (priv->uOSDLayer, Block_Head, block_cur);
#else
    // write OSD memory first, write register later
    gma_fill_hw_head (priv->uOSDLayer, Block_Head, block_cur);
    gma_fill_hw_head (priv->uOSDLayer, Block_Head, block_last);
#endif

    write_gma_header_to_fpga ();
#endif

    return block_cur;
}

static void gma_block_delete (UINT32 layer_id, struct gma_block_t *block_cur)
{
    struct gma_block_t *block_last, *block_next;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    struct gma_block_t *Block_Head = &(priv->tBlockHead);

    GMA_PRINTF ("gma_block_delete : 0x%x \n", block_cur);

    block_last = block_cur->prev;
    block_next = block_cur->next;
    block_last->next = block_next;
    block_next->prev = block_last;
    block_cur->prev = NULL;
    block_cur->next = NULL;

    gma_fill_hw_head (priv->uOSDLayer, Block_Head, block_last);

    gma_m36f_release_block (block_cur);
    block_cur = NULL;

    write_gma_header_to_fpga ();
}
static void gma_block_update (UINT32 layer_id, struct gma_block_t *block_cur)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    struct gma_block_t *Block_Head = &(priv->tBlockHead);


    gma_fill_hw_head (priv->uOSDLayer, Block_Head, Block_Head);
    gma_fill_hw_head (priv->uOSDLayer, Block_Head, block_cur);

    write_gma_header_to_fpga ();
}


 static INT32 gma_block_show (UINT32 layer_id, struct gma_block_t *block_cur)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    struct gma_block_t *Block_Head = &(priv->tBlockHead);
    struct gma_block_t *block_prev;

    if (block_cur->uStatus == BLOCK_SHOW)
        return RET_FAILURE;

    block_cur->uStatus = BLOCK_SHOW;

    block_prev = block_cur->prev;
    while (block_prev != Block_Head)
    {
        if (BLOCK_SHOW == block_prev->uStatus)
            break;
        block_prev = block_prev->prev;
    }
    gma_fill_hw_head (priv->uOSDLayer, Block_Head, block_cur);
    gma_fill_hw_head (priv->uOSDLayer, Block_Head, block_prev); //block_cur->prev

    write_gma_header_to_fpga ();
    return RET_SUCCESS;
}

 static INT32 gma_block_hide (UINT32 layer_id, struct gma_block_t *block_cur)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    struct gma_block_t *Block_Head = &(priv->tBlockHead);
    struct gma_block_t *block_prev;

    if (block_cur->uStatus == BLOCK_HIDE)
        return RET_FAILURE;

    block_cur->uStatus = BLOCK_HIDE;

    block_prev = block_cur->prev;
    while (block_prev != Block_Head)
    {
        if (BLOCK_SHOW == block_prev->uStatus)
            break;
        block_prev = block_prev->prev;
    }
    gma_fill_hw_head (priv->uOSDLayer, Block_Head, block_prev); //block_cur->prev

    write_gma_header_to_fpga ();

    return RET_SUCCESS;
}

#define HW_PIXEL_FORMAT
static const UINT8 m_ge_to_gma_mode[] = {
    GMA36F_PF_RGB888,     GMA_PF_RGB888,     4, M36F_V|M32C_V, 0,
    GMA36F_PF_ARGB8888,   GMA_PF_ARGB8888,   4, M36F_V|M32C_V, 0,
    GMA36F_PF_RGB444,     GMA_PF_RGB444,     2, M36F_V|M32C_V, 0,
    GMA36F_PF_ARGB4444,   GMA_PF_ARGB4444,   2, M36F_V|M32C_V, 0,
    GMA36F_PF_RGB555,     GMA_PF_RGB555,     2, M36F_V|M32C_V, 0,
    GMA36F_PF_ARGB1555,   GMA_PF_ARGB1555,   2, M36F_V|M32C_V, 0,
    GMA36F_PF_RGB565,     GMA_PF_RGB565,     2, M36F_V|M32C_V, 0,
    GMA36F_PF_CLUT2,      GMA_PF_CLUT2,      1, 0,             M36F_V,
    GMA36F_PF_CLUT4,      GMA_PF_CLUT4,      1, M36F_V|M32C_V, M36F_V|M32C_V,
    GMA36F_PF_ACLUT44,    GMA_PF_ACLUT44,    1, 0, 0,
    GMA36F_PF_CLUT8,      GMA_PF_CLUT8,      1, M36F_V|M32C_V, M36F_V|M32C_V,
    GMA36F_PF_ACLUT88,    GMA_PF_ACLUT88,    2, M36F_V|M32C_V, 0,

    GMA36F_PF_YCbCr444,   GMA_PF_YCbCr444,   4, 0,             0,
    GMA36F_PF_YCbCr422,   GMA_PF_YCbCr422,   2, 0,             0,
    GMA36F_PF_YCbCr420,   GMA_PF_YCbCr420,   2, 0,             0,
    GMA36F_PF_AYCbCr8888, GMA_PF_AYCbCr8888, 4, 0,             0,
};

static UINT8 gma_get_byte_per_pixel (UINT8 osd_mode)
{
    UINT32 i;
    UINT8 bpp = 1;
    for (i = 0; i < ARRAY_SIZE (m_ge_to_gma_mode); i += sizeof (ge_to_gma_mode_t))
    {
        if (m_ge_to_gma_mode[i] == osd_mode)
        {
            bpp = m_ge_to_gma_mode[i + BPP_INDEX];
            break;
        }
    }
    return bpp;
}

static UINT16 gma_get_pitch (UINT8 color_mode, UINT16 width)
{
    UINT8 bpp = gma_get_byte_per_pixel (color_mode);

    if (color_mode == GMA36F_PF_CLUT2)
        return (width >> 2);
    else if (color_mode == GMA36F_PF_CLUT4)
        return (width >> 1);
    else if (color_mode == GMA36F_PF_CLUT8)
        return width;
    else if (bpp == 2)
        return (width << 1);
    else if (bpp == 4)
        return (width << 2);

    return width;
}

static BOOL gma_color_mode_ic_support_flag (UINT8 hw_layer_id, UINT8 osd_mode)
{
    UINT32 i;
    UINT8 bpp = 0;
    for (i = 0; i < ARRAY_SIZE (m_ge_to_gma_mode); i += sizeof (ge_to_gma_mode_t))
    {
        if (m_ge_to_gma_mode[i] == osd_mode)
        {
            bpp = m_ge_to_gma_mode[i + IC_INDEX + hw_layer_id];
            break;
        }
    }

    if (m_gma_chip_id == ALI_S3602 || m_gma_chip_id == ALI_C3701 || m_gma_chip_id == ALI_S3503)
        bpp &= M36F_V;
	else
        bpp &= M32C_V;

    return (bpp ? TRUE : FALSE);
}

static BOOL gma_is_clut8 (UINT8 color_mode)
{
    return (color_mode == GMA36F_PF_CLUT2 || color_mode == GMA36F_PF_CLUT4 || color_mode == GMA36F_PF_CLUT8);
}

static UINT32 get_screen_pitch (UINT32 region_width)
{
    if (region_width <= 720)
        return 720;

    if (region_width <= 1280)
        return 1280;

    return 1920;
}

static UINT32 get_screen_height (UINT32 screen_width)
{
    if (screen_width <= 720)
        return 576;

    if (screen_width <= 1280)
        return 720;

    return 1080;
}

static enum TVSystem osd_m36f_get_tv_system(void)
{
    enum TVSystem eTVMode = PAL;
    struct vpo_device* dis = (struct vpo_device*) hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 0);

    if(dis != NULL)
    {
		RET_CODE ret = vpo_ioctl(dis, VPO_IO_GET_OUT_MODE, (UINT32)&eTVMode);
        if (ret == RET_SUCCESS)
        {
            if (eTVMode == LINE_1080_24 || eTVMode == LINE_1152_ASS || eTVMode == LINE_1080_ASS || eTVMode == LINE_1080_50)
                eTVMode = LINE_1080_25;
            else if (eTVMode == LINE_1080_60)
                eTVMode = LINE_1080_30;
        }
	}
   
    return eTVMode;
}

#define QUEUE_OPERATION

#define GMA_BUF_MAX 4
struct OsdMem
{
    UINT8 *puBuf;
    UINT32 uSize;
};

struct OsdMemSlice
{
    struct OsdMemSlice *next;
    struct OsdMemSlice *prev;
    UINT8 busy;
    UINT8 *ptr;
    UINT32 size;
};

// these 4 variables are for both layers.
static struct OsdMem m_gma_mem[GMA_BUF_MAX];
static struct OsdMemSlice m_gma_mem_head;
static UINT8 m_gma_mem_slice_init; // = 0;
static UINT8 m_gma_mem_index;   // = 0;

 static void q_insert (struct OsdMemSlice *last, struct OsdMemSlice *next, struct OsdMemSlice *slice)
{
    last->next = slice;
    next->prev = slice;
    slice->next = next;
    slice->prev = last;
}

 static void q_delete (struct OsdMemSlice *last, struct OsdMemSlice *next, struct OsdMemSlice *slice)
{
    last->next = next;
    next->prev = last;
    slice->next = NULL;
    slice->prev = NULL;
}

static BOOL get_buffer (struct gma_m36f_private *priv, UINT32 request_size, UINT8 uBufIndex, UINT8 ** asd_buf, UINT32 * ask_size)
{
    if (uBufIndex == 0)
    {
        if (m_config.bCacheable)
            *asd_buf = get_cache_addr ((void *) m_config.mem_base);
        else
            *asd_buf = get_none_cache_addr ((void *) m_config.mem_base);
        *ask_size = m_config.mem_size;
        return TRUE;
    }
    return FALSE;
}

static void malloc_me (struct gma_m36f_private *priv, UINT32 request_size, UINT8 ** req_buf, UINT32 * req_size, UINT8 contiguous)
{
    struct OsdMemSlice *slice, *new_slice;
    UINT8 *ask_buf;
    UINT32 ask_size;

    GMA_PRINTF ("malloc_me :request_size =%d\n", request_size);

    if (m_gma_mem_slice_init == 0)
    {
        m_gma_mem_head.busy = 1; //MUST
        m_gma_mem_head.ptr = NULL; //MUST
        m_gma_mem_head.next = &m_gma_mem_head;
        m_gma_mem_head.prev = &m_gma_mem_head;
        m_gma_mem_slice_init = 1;
    }

    slice = m_gma_mem_head.next;
    while (1)
    {
        if (slice == &m_gma_mem_head)
        {
            if (get_buffer (priv, request_size, m_gma_mem_index, &ask_buf, &ask_size) == FALSE)
                ASSERT (0);
            new_slice = (struct OsdMemSlice *) kmalloc(sizeof (struct OsdMemSlice),GFP_KERNEL);
            new_slice->busy = 0;
            if ((m_gma_mem_index != 0) && (ask_buf == m_gma_mem[m_gma_mem_index - 1].puBuf + m_gma_mem[m_gma_mem_index - 1].uSize))
            {
                m_gma_mem[m_gma_mem_index - 1].uSize += ask_size;
                new_slice->ptr = ask_buf;
                new_slice->size = ask_size;
            }
            else
            {
                if (m_gma_mem_index >= GMA_BUF_MAX)
                    ASSERT (0);
                new_slice->ptr = m_gma_mem[m_gma_mem_index].puBuf = ask_buf;
                new_slice->size = m_gma_mem[m_gma_mem_index].uSize = ask_size;
                m_gma_mem_index++;
            }
            q_insert (slice->prev, &m_gma_mem_head, new_slice);
            slice = new_slice;
            ASSERT (!contiguous || (slice->size >= request_size));
            if (slice->size <= request_size)
            {
                *req_buf = slice->ptr;
                *req_size = slice->size;
                slice->busy = 1;
                return;
            }
            else
            {
                new_slice = (struct OsdMemSlice *) kmalloc(sizeof (struct OsdMemSlice),GFP_KERNEL);
                new_slice->busy = 1;
                new_slice->ptr = slice->ptr;
                new_slice->size = request_size;
                slice->ptr += request_size;
                slice->size -= request_size;
                q_insert (slice->prev, slice, new_slice);
                *req_buf = new_slice->ptr;
                *req_size = new_slice->size;
                return;
            }

        }
        else                    //(slice != &m_gma_mem_head)
        {
            if (slice->busy || (contiguous && slice->size < request_size))
            {
//                if(!slice->busy && (contiguous && slice->size < request_size))
//                    libc_printf("uncontiguous free memory\n");
                slice = slice->next;
                continue;
            }
            if (slice->size <= request_size)
            {
                *req_buf = slice->ptr;
                *req_size = slice->size;
                slice->busy = 1;
                return;
            }
            else
            {
                new_slice = (struct OsdMemSlice *) kmalloc(sizeof (struct OsdMemSlice),GFP_KERNEL);
                new_slice->busy = 1;
                new_slice->ptr = slice->ptr;
                new_slice->size = request_size;
                slice->ptr += request_size;
                slice->size -= request_size;
                q_insert (slice->prev, slice, new_slice);
                *req_buf = new_slice->ptr;
                *req_size = new_slice->size;
                return;
            }
        }
    }
}

 static void free_me (UINT8 * buf, UINT32 size)
{
    struct OsdMemSlice *slice, *slice_last, *slice_next;

    GMA_PRINTF ("free_me : buf = 0x%x,size =%d\n", buf, size);
    m_last_buffer = buf;

    slice = m_gma_mem_head.next;
    while (slice != &m_gma_mem_head)
    {
        if (slice->ptr != buf)
        {
            slice = slice->next;
            continue;
        }
        if (slice->busy == 0)
        {
            GMA_PRINTF ("ERROR : This mem slice is already free!\n");
            ASSERT (0);
        }
        slice->busy = 0;
        slice_last = slice->prev;
        if (slice->prev->busy == 0 && (slice->prev->ptr + slice->prev->size == slice->ptr))
        {
            slice_last = slice->prev->prev;
            slice->ptr = slice->prev->ptr;
            slice->size = slice->size + slice->prev->size;
            FREE (slice->prev);
        }
        slice_next = slice->next;
        if (slice->next->busy == 0 && (slice->ptr + slice->size == slice->next->ptr))
        {
            slice_next = slice->next->next;
            slice->size = slice->size + slice->next->size;
            FREE (slice->next);
        }
        q_insert (slice_last, slice_next, slice);
        while (slice != &m_gma_mem_head && slice->next == &m_gma_mem_head && slice->busy == 0)
        {
            slice_last = slice->prev;
            if (slice->ptr != m_gma_mem[m_gma_mem_index - 1].puBuf) //this mem free
            {
                return;         //  EXIT POINT : NORMAL
            }
            m_gma_mem_index--;
            q_delete (slice->prev, slice->next, slice);
            FREE (slice);
            slice = slice_last;
        }
        return;                 //  EXIT POINT  :NO SLICE
    }
    GMA_PRINTF ("ERROR : No such a mem slice !\n");
    ASSERT (0);
}

#if 0
enum GE_INTERSECT_RESULT ge_rect_intersect_rect (const gma_rect_t * ra, const gma_rect_t * rb, gma_rect_t * r)
{
    UINT32 x, ex, y, ey;
    UINT32 ax, aex, ay, aey;
    UINT32 bx, bex, by, bey;
    ax = ra->left;
    aex = ax + ra->width;
    ay = ra->top;
    aey = ay + ra->height;

    bx = rb->left;
    bex = bx + rb->width;
    by = rb->top;
    bey = by + rb->height;

    enum GE_INTERSECT_RESULT ret = GE_INTERSECT_PART;

    if (aey <= by || bey <= ay || aex <= bx || bex <= ax)
    {
        x = ex = ax;
        y = ey = ay;
        ret = GE_INTERSECT_NULL;
        goto LABEL_RETURN;
    }

    if (ax == bx && aex == bex && ay == by && aey == bey)
    {
        x = ax;
        ex = aex;
        y = ay;
        ey = aey;
        ret = GE_INTERSECT_EQUAL;
        goto LABEL_RETURN;
    }

    if (ax >= bx && aex <= bex && ay >= by && aey <= bey)
    {
        x = ax;
        ex = aex;
        y = ay;
        ey = aey;
        ret = GE_INTERSECT_SMALL;
        goto LABEL_RETURN;
    }

    if (bx >= ax && bex <= aex && by >= ay && bey <= aey)
    {
        x = bx;
        ex = bex;
        y = by;
        ey = bey;
        ret = GE_INTERSECT_BIG;
        goto LABEL_RETURN;
    }

    x = (ax >= bx) ? ax : bx;
    y = (ay >= by) ? ay : by;

    ex = (aex <= bex) ? aex : bex;
    ey = (aey <= bey) ? aey : bey;

  LABEL_RETURN:
    r->left = x;
    r->top = y;
    r->width = ex - x;
    r->height = ey - y;

    return ret;

}

BOOL ge_rect_is_inside_rect (const gma_rect_t * ra, const gma_rect_t * rb)
{
    ge_rect_t r;

    enum GE_INTERSECT_RESULT intersect = ge_rect_intersect_rect (ra, rb, &r);

    return (intersect == GE_INTERSECT_EQUAL || intersect == GE_INTERSECT_SMALL);
}
#endif

BOOL ge_rect_is_inside_size (const gma_rect_t * rect, UINT32 width, UINT32 height)
{
    UINT32 end_x = rect->left + rect->width;
    UINT32 end_y = rect->top + rect->height;

    return (end_x <= width && end_y <= height);
}

static void enable_display (UINT8 uLayer, UINT8 uOn)
{
    gma_hal_SetMask (uLayer);
    gma_hal_OnOff (uLayer, uOn);
    gma_hal_ClearMask (uLayer);
    osal_task_sleep (20);
}

#define REGION_OFF      -1
#define REGION_CHECK    0
#define REGION_ON       1
static UINT8 region_monitor (struct gma_m36f_private *priv, INT8 uOp)
{
    UINT8 i, cnt;

    for (i = 0, cnt = 0; i < GMA_REGION_MAX; i++)
    {
        if (priv->tRegion[i].bValid && priv->tRegion[i].uStatus == BLOCK_SHOW)
            cnt++;
    }

    if (!cnt && (REGION_OFF == uOp))
        enable_display (priv->uOSDLayer, FALSE);
    else if ((1 == cnt) && priv->bOnOff && (REGION_ON == uOp))
        enable_display (priv->uOSDLayer, TRUE);

    return cnt;
}

#define PALLETE_OPERATION
static RET_CODE gma_m36f_malloc_pal (struct gma_m36f_private *priv)
{
    UINT8 *ptr;
    UINT32 addr;
    if (priv->puPalBase != NULL)
        return RET_SUCCESS;

#ifndef GMA_PAL_STATIC

    ptr = (UINT8 *)kmalloc(1024 * 2 + 32,GFP_KERNEL);
    ASSERT(ptr);
#else
    ptr = m_osd_pal_buf0;
    if (m_osd_pal_buf1 == NULL)
    {
        addr = (UINT32)ptr;
        addr &= (32 - 1);
        if (addr)
        {
            ptr += (32 - addr);
        }
        m_osd_pal_buf1 = ptr + 2048;
    }

    if (priv->uOSDLayer == 1)
        ptr = m_osd_pal_buf1;
    else if (priv->uOSDLayer == 2)
        ptr = m_osd_pal_buf1 + 1024 * 2;
#endif

    priv->puPalBase = ptr;

    // GMA pal must be 32 bytes aligned
    // GE  pal must be 8 bytes aligned
    addr = (UINT32)ptr;
    addr &= (32 - 1);
    if (addr)
    {
        ptr += (32 - addr);
    }
    priv->puPallette = get_none_cache_addr(ptr);

    priv->puPalletteHW = priv->puPallette + 1024;

    return RET_SUCCESS;
}

static RET_CODE gma_m36f_free_pal (struct gma_m36f_private *priv)
{
#ifndef GMA_PAL_STATIC
    if (priv->puPalBase != NULL)
    {
        FREE (priv->puPalBase);
        priv->puPalBase = NULL;
    }
#endif
    return RET_SUCCESS;
}

static void gma_m36f_update_pal_entry (struct gma_m36f_private *priv, UINT8 index, BOOL bAllEntry)
{
    UINT8 uK, uY, uCb, uCr;
    UINT32 pos;

    if (bAllEntry)
    {
        pos = index;
        pos <<= 2;

        uY = priv->puPallette[pos++];
        uCb = priv->puPallette[pos++];
        uCr = priv->puPallette[pos++];
        uK = priv->puPallette[pos++];
    }
    else
    {
        pos = priv->color;

        uK = pos;
        uCr = pos >> 8;
        uCb = pos >> 16;
        uY = pos >> 24;
    }

    if (priv->region_info.galpha_enable == 0 && priv->region_info.color_format != GMA36F_PF_ACLUT88)
    {
        uCb = (uCb & 0xFC) | ((uK & 0x0C) >> 2);
        uCr = (uCr & 0xFC) | (uK & 0x03);
    }
    //GMA_PRINTF("index = %x, Y=%x,Cb=%x,Cr=%x \n",i,uY,uCb,uCr);
    if (bAllEntry == FALSE || index == 0)
        gma_hal_SetMask (priv->uOSDLayer);
    gma_hal_SetPalette (priv->uOSDLayer, index, uY, uCb, uCr);
    if (bAllEntry == FALSE || index == 0xff)
        gma_hal_ClearMask (priv->uOSDLayer);
}

static RET_CODE gma_m36f_update_pal (UINT32 layer_id, BOOL bUpdate)
{
    gma_head_m36f_t *phead = NULL;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    if (m_gma_chip_id == ALI_M3202)
    {
        UINT32 i;

        for (i = 0; (i < priv->uPalletteColor) && bUpdate; i++)
            gma_m36f_update_pal_entry (priv, i, TRUE);
        priv->bPalletteUpdated = TRUE;
    }
    else
    {
        struct gma_block_t *pHead = &priv->tBlockHead;
        struct gma_block_t *pBlock = pHead->next;
        UINT32 header_id = priv->new_header_id;

        if (bUpdate == FALSE)
        {
            header_id = 1 - header_id;
        }
        for (; pBlock && pBlock != pHead; pBlock = pBlock->next)
        {
            if (pBlock->uStatus == BLOCK_HIDE)
                continue;

            phead = (gma_head_m36f_t *) pBlock->puHeadBuf[header_id];
            if (bUpdate == FALSE)
            {
                phead->clut_update = 0;
                continue;
            }
            if (phead->clut_base == 0)
            {
                if (phead->gma_mode >= GMA36F_PF_CLUT2 && phead->gma_mode <= GMA36F_PF_ACLUT88)
                    phead->clut_base = ((UINT32)priv->puPalletteHW) & 0x0fffffff;
            }
            if (phead->clut_base)
            {
                // only update the CLUT for the 1st block because all blocks share one CLUT
                phead->clut_update = 1;
                bUpdate = FALSE; // continue to clear other block's update flag.
            }

        }
        priv->bPalletteUpdated = TRUE;
    }
    return RET_SUCCESS;
}


static int osd_clip(int x, int lower, int upper) 
{
    return (x<lower) ? lower : (x>upper) ? upper : x;
}

static UINT8 ptn_ptr0[256];
static void osd_yuv2rgb_convert(UINT8 *bgra, const UINT8 *yuva, BOOL bVideoMatrix, BOOL bBt709)
{
    int uR,uG,uB;
    UINT8 Y, U, V;

    Y = yuva[2];
    U = yuva[1];
    V = yuva[0];

    if(!bVideoMatrix)
    {
        Y = osd_clip(Y,16,235);
        U = osd_clip(U,16,240);
        V = osd_clip(V,16,240);
        if(!bBt709)
        {
            uR = (298*(Y-16)+0*(U-128)+409*(V-128)+128)/256;
            uG= (298*(Y-16)-100*(U-128)-208*(V-128)+128)/256;
            uB= (298*(Y-16)+517*(U-128)+0*(V-128)+128)/256;
        }
        else
        {
            uR = (298*(Y-16)+0*(U-128)+459*(V-128)+128)/256;
            uG= (298*(Y-16)-54*(U-128)-136*(V-128)+128)/256;
            uB= (298*(Y-16)+541*(U-128)+0*(V-128)+128)/256;
        }
    }
    else
    {
        // Y = osd_clip(Y,0,255);
        // U = osd_clip(U,0,255);
        // V = osd_clip(V,0,255);
        if(!bBt709)
        {
            uR = (256*(Y)+0*(U-128)+351*(V-128)+128)/256;
            uG= (256*(Y)-86*(U-128)-179*(V-128)+128)/256;
            uB= (256*(Y)+444*(U-128)+0*(V-128)+128)/256;
        }
        else
        {
            uR = (256*(Y)+0*(U-128)+394*(V-128)+128)/256;
            uG= (256*(Y)-47*(U-128)-117*(V-128)+128)/256;
            uB= (256*(Y)+464*(U-128)+0*(V-128)+128)/256;
        }
    }

    bgra[2] = osd_clip(uR,0,255);
    bgra[1] = osd_clip(uG,0,255);
    bgra[0] = osd_clip(uB,0,255);
    bgra[3] = (yuva[3] << 4) + yuva[3];

}

static RET_CODE gma_m36f_pal_YCbCr_to_RGB (UINT32 layer_id)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    UINT32 i;

    if (priv->uPalFormatHW == priv->uPalFormatSW)
    {
        MEMCPY (priv->puPalletteHW, priv->puPallette, 1024);
        return RET_SUCCESS;
    }

    if (priv->uPalFormatHW == GE_PAL_YCBCR)
    {
        GMA_PRINTF ("ERROR : NOT support RGB -> YCbCr!\n");
        return RET_FAILURE;
    }

#if 0
    UINT8 *ptn_ptr = get_none_cache_addr (ptn_ptr0);
    for (i = 0; i < 256; i++)
        ptn_ptr[i] = i;

    write_gma_header_to_fpga ();

    ge_cmd_list_hdl cmd_list = priv->ge_cmd_list;
    ge_m36f_cmd_list_new (m_ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);

    ge_operating_entity base_addr;

    base_addr.color_format = GE_PF_CLUT8;
    base_addr.base_address = (UINT32) ptn_ptr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = 16;
    base_addr.modify_flags = GE_BA_FLAG_ADDR | GE_BA_FLAG_FORMAT | GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_m36f_cmd_begin (m_ge_dev, cmd_list, GE_DRAW_BITMAP);
    ge_m36f_set_operating_entity (m_ge_dev, cmd_hdl, GE_PTN, &base_addr);

    base_addr.color_format = GE_PF_ARGB8888;
    base_addr.base_address = (UINT32) priv->puPalletteHW;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = 16;
    base_addr.modify_flags = GE_BA_FLAG_ADDR | GE_BA_FLAG_FORMAT | GE_BA_FLAG_PITCH;
    ge_m36f_set_operating_entity (m_ge_dev, cmd_hdl, GE_DST, &base_addr);

    ge_m36f_set_xy (m_ge_dev, cmd_hdl, GE_DST_PTN, 0, 0);
    ge_m36f_set_wh (m_ge_dev, cmd_hdl, GE_DST_PTN, 16, 16);

    ge_m36f_set_clut_mode (m_ge_dev, cmd_hdl, GE_CLUT_COLOR_EXPANSION, TRUE);
    ge_m36f_set_clut_addr (m_ge_dev, cmd_hdl, (UINT32) priv->puPallette);
    ge_m36f_set_clut_rgb_order (m_ge_dev, cmd_hdl, GE_RGB_ORDER_AYCbCr);
    ge_m36f_set_clut_color_cvt (m_ge_dev, cmd_hdl, GE_COLOR_CVT_BT601, GE_COLOR_SPACE_GRAPHIC_MATRIX);
    ge_m36f_set_clut_update (m_ge_dev, cmd_hdl, TRUE);

    ge_m36f_cmd_end (m_ge_dev, cmd_hdl);
    ge_m36f_cmd_list_end (m_ge_dev, cmd_list);
#else
    for (i = 0; i < priv->uPalletteColor; i++)
    {
        osd_yuv2rgb_convert(priv->puPalletteHW + i * 4, priv->puPallette + i * 4, TRUE, FALSE);
    }
    return RET_SUCCESS;
#endif

    return RET_SUCCESS;
}

#define GMA_SUB

static RET_CODE gma_m36f_set_region_param (UINT8 hw_layer_id, alifb_gma_region_t * region_info, const alifb_gma_region_t * ptPara)
{
    if (ptPara == NULL)
        return RET_FAILURE;

    MEMCPY (region_info, ptPara, sizeof (alifb_gma_region_t));
    //region_info->color_format = ge_m36f_get_hw_pf((enum GMA_PIXEL_FORMAT)ptPara->color_format);

    if (gma_color_mode_ic_support_flag (hw_layer_id, ptPara->color_format) == 0)
    {
        GMA_ERR_PRINTF ("chip not support OSD color mode %d\n", ptPara->color_format);
        return RET_FAILURE;     // ge not support this mode
    }

    return RET_SUCCESS;
}

RET_CODE gma_m36f_set_region_pos (UINT32 layer_id, UINT32 region_id, gma_rect_t * rect)
{
    //UINT32 i;
    UINT32 time_out = 50;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);

    if (region->bValid == FALSE)
    {
        GMA_PRINTF ("ERROR : region NOT created!\n");
        return RET_FAILURE;     //ASSERT(0);
    }

    if (rect->width != region->tRegionRect.width || rect->height != region->tRegionRect.height)
    {
        GMA_PRINTF ("ERROR : Size Changed\n");
        ASSERT (0);
    }

#if 0
    for (i = 0; i < MAX_BLOCK_IN_REGION; i++)
    {
        if (region->pBlock[i] != NULL)
        {
            region->pBlock[i]->tRect.left = region->pBlock[i]->tRect.left + rect->left - region->tRegionRect.left;
            region->pBlock[i]->tRect.top = region->pBlock[i]->tRect.top + rect->top - region->tRegionRect.top;

            region->pBlock[i]->tDisRect.left = region->pBlock[i]->tRect.left;
            region->pBlock[i]->tDisRect.top = region->pBlock[i]->tRect.top;

            gma_m36f_set_head_rect (priv, &region->pBlock[i]->block_head, &region->pBlock[i]->tRect);

            gma_block_update (layer_id, region->pBlock[i]);
        }
    }
    MEMCPY (&(region->tRegionRect), rect, sizeof (gma_rect_t));
#else

    // Update block header in DE vblanking
    while (TRUE == priv->bSetRegionPos)
    {
        if (0 == time_out)
            break;
        time_out--;
        osal_task_sleep (20);
    }

    ENTER_GMA_API ();

    gma_m36f_sync_block_header(priv, TRUE);

    priv->reg_rect_new = *rect;
    priv->region_id = region_id;
    priv->bSetRegionPos = TRUE;

#ifdef GMA_HEADER_REALTIME_UPDATE
    gma_m36f_update_block_header(layer_id, FALSE);
#endif

    LEAVE_GMA_API ();

    time_out = 50;

    while (TRUE == priv->bSetRegionPos)
    {
        if (0 == time_out)
            break;
        time_out--;
        osal_task_sleep (20);
    }

#endif

    return RET_SUCCESS;
}

static RET_CODE _gma_m36f_set_region_pos (UINT32 layer_id, UINT32 region_id, gma_rect_t * rect)
{
    UINT32 i;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);

    //ENTER_GMA_API ();

    for (i = 0; i < MAX_BLOCK_IN_REGION; i++)
    {
        if (region->pBlock[i] != NULL)
        {
            region->pBlock[i]->tRect.left = region->pBlock[i]->tRect.left + rect->left - region->tRegionRect.left;
            region->pBlock[i]->tRect.top = region->pBlock[i]->tRect.top + rect->top - region->tRegionRect.top;

            gma_m36f_set_head_rect (priv, &region->pBlock[i]->block_head, &region->pBlock[i]->tRect);

            gma_block_update (layer_id, region->pBlock[i]);
        }
    }
    MEMCPY (&(region->tRegionRect), rect, sizeof (gma_rect_t));
    region->region_info.region_x = rect->left;
    region->region_info.region_y = rect->top;

    //LEAVE_GMA_API ();

    return RET_SUCCESS;
}

static RET_CODE _gma_m36f_set_display_rect (UINT32 layer_id, UINT32 region_id, gma_rect_t *posd_rect0)
{   
    struct vpo_device *dis_dev = (struct vpo_device*)hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    struct vpo_io_get_info dis_info;
    gma_head_m36f_t *phead = NULL;
    enum TVSystem tvsys;
    int max_width;
    int max_height;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);
    gma_rect_t dis_rect = *posd_rect0;
    gma_rect_t *posd_rect = &dis_rect;
    RET_CODE ret = RET_FAILURE;
    BOOL use_default;

    if (region_id >= GMA_REGION_MAX || !region->bValid)
        use_default = TRUE;
    else
        use_default = FALSE;

    if (use_default)
        return RET_FAILURE;

    phead = &(region->pBlock[0]->block_head);

    if(dis_dev != NULL)
    {
        memset(&dis_info, 0, sizeof(dis_info));
	    ret = vpo_ioctl(dis_dev, VPO_IO_GET_INFO, (UINT32)&dis_info);
        if(ret == RET_SUCCESS)
        {
            tvsys      = dis_info.tvsys;
            max_width  = dis_info.des_width;
            max_height = dis_info.des_height;

            if((max_width == 3840) || (max_width == 4096))
            {
                max_width /= 2;
            }
        }
        else
        {
            return ret;
        }
    }
    else
    {
        return ret;
    }

    posd_rect->width = posd_rect->left + posd_rect->width - 1;
    if (posd_rect->width > max_width - 1)
        posd_rect->width = max_width - 1;

    posd_rect->height = posd_rect->top + posd_rect->height - 1;
    if (posd_rect->height > max_height - 1)
        posd_rect->height = max_height - 1;

    phead->start_x = posd_rect->left;
    phead->end_x = posd_rect->width;

    phead->start_y = posd_rect->top;
    phead->end_y = posd_rect->height;

    phead = (gma_head_m36f_t *)(region->pBlock[0]->puHeadBuf[0]);
    phead->start_x = posd_rect->left;
    phead->end_x = posd_rect->width;

    phead->start_y = posd_rect->top;
    phead->end_y = posd_rect->height;

    phead = (gma_head_m36f_t *)(region->pBlock[0]->puHeadBuf[1]);
    phead->start_x = posd_rect->left;
    phead->end_x = posd_rect->width;

    phead->start_y = posd_rect->top;
    phead->end_y = posd_rect->height;

    return RET_SUCCESS;
}

static RET_CODE gma_m36f_create_region_param (UINT32 layer_id, UINT32 region_id, pcgma_region_t pregion_param)
{
    UINT32 i;
    RET_CODE ret;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);
    UINT8 *block_buf;
    UINT32 block_size;
    UINT32 line_size;
    UINT32 request_height;
    UINT32 height;
    alifb_gma_region_t gma_region;
    gma_rect_t r, block_rect;
    gma_rect_t fill_rect;
    UINT32 fill_color;
    UINT8 color_mode;

#if 0
    //TODO: check it overlap with other regions
    if (!ge_rect_is_inside_rect (rect, &priv->screen_rect))
    {

        GMA_PRINTF ("ERROR : region rect must be inside SCREEN RECT, please adjust SCREEN RECT!\n");
        return RET_FAILURE;
    }
#endif

    r.left = pregion_param->region_x;
    r.top = pregion_param->region_y;
    r.width = pregion_param->region_w;
    r.height = pregion_param->region_h;

    if ((UINT32) r.width > pregion_param->bitmap_w - pregion_param->bitmap_x)
        r.width = pregion_param->bitmap_w - pregion_param->bitmap_x;

    if ((UINT32) r.height > pregion_param->bitmap_h - pregion_param->bitmap_y)
        r.height = pregion_param->bitmap_h - pregion_param->bitmap_y;

    if ((UINT16) r.width >= 4096 || (UINT16) r.height >= 4096)
    {
        GMA_ERR_PRINTF ("ERROR : invalid region rect!\n");
        return RET_FAILURE;
    }

    ENTER_GMA_API ();

    gma_m36f_sync_block_header(priv, TRUE);

    MEMCPY (&(region->tRegionRect), &r, sizeof (gma_rect_t));
    MEMCPY ((void *) &gma_region, (void *) pregion_param, sizeof (alifb_gma_region_t));
    gma_region.color_format = ge_m36f_get_hw_pf ((enum GMA_PIXEL_FORMAT)pregion_param->color_format);
    ret = gma_m36f_set_region_param (priv->uOSDLayer, &region->region_info, &gma_region);
    if (ret != RET_SUCCESS)
    {
        LEAVE_GMA_API ();
        return ret;
    }

    region->uStatus = BLOCK_SHOW;
    region->bUseMemSlice = FALSE;
    region->bUseExternalMem = TRUE;

    color_mode = region->region_info.color_format;
    fill_rect.left = 0;
    fill_rect.top = 0;
    fill_rect.width = pregion_param->region_w;
    fill_rect.height = pregion_param->region_h;

    fill_color = 0;
    //fill_color = 0xffffff00;
    if (color_mode == GMA36F_PF_CLUT8)
        fill_color = (UINT8) priv->trans_color;
    else if (color_mode == GMA36F_PF_CLUT4 || color_mode == GMA36F_PF_CLUT2)
        fill_color = 0xff;

    line_size = gma_get_pitch (region->region_info.color_format, pregion_param->pixel_pitch);
    request_height = r.height;

    for (i = 0; i < MAX_BLOCK_IN_REGION; i++)
    {
        //Alex Wu 061003
        if (request_height <= 1)
        {
            if (request_height == 1)
                request_height = 2;
            else
            {
                region->pBlock[i] = NULL;
                continue;
            }
        }

        {
            UINT32 addr = pregion_param->bitmap_addr;

            addr += line_size * pregion_param->bitmap_y;
            addr += gma_get_pitch (region->region_info.color_format, pregion_param->bitmap_x);
            block_buf = (UINT8 *) addr;
            block_size = request_height * line_size;

            if (pregion_param->bitmap_addr == priv->layer_config.mem_base)
            {
                if (addr + block_size > (priv->layer_config.mem_base + priv->layer_config.mem_size))
                    break;  // memory not enough
            }
        }

        GMA_PRINTF ("Region[%d]->block[%d]: block_buf = 0x%x,block_size = %d, end addr = 0x%x\n", region_id, i, block_buf, block_size,
                    block_buf + block_size);

        if (block_size != 0)
        {
            block_rect.left = r.left;
            block_rect.width = r.width;
            block_rect.top = r.top;
            height = block_size / line_size;
            //            height &= ~1;   // block height must be even
            if (height)
            {
                block_rect.height = height;

                region->pBlock[i] = gma_block_create (layer_id, &block_rect, block_buf, height * line_size, &region->region_info);

                r.top += height;
                request_height -= height;

                //MEMSET(block_buf, fill_color, height * line_size);

                fill_rect.height = height;
                region->bValid = TRUE;
                if (priv->auto_clear_region)
                    gma_m36f_region_fill (layer_id, region_id, &fill_rect, fill_color);

                // write OSD memory first, write register later
                gma_fill_hw_head (priv->uOSDLayer, &priv->tBlockHead, region->pBlock[i]);
                gma_fill_hw_head (priv->uOSDLayer, &priv->tBlockHead, region->pBlock[i]->prev);
            }
            else
            {
                break;
            }
        }
    }

    if (request_height)
    {
        GMA_PRINTF ("ERROR : malloc failure!\n");
        LEAVE_GMA_API ();
        return RET_FAILURE;     //ASSERT(0);
    }

    gma_m36f_update_block_header(layer_id, TRUE);

    write_gma_header_to_fpga();

    region->bValid = TRUE;
    region_monitor (priv, REGION_ON);

    LEAVE_GMA_API ();
    return RET_SUCCESS;
}

static RET_CODE _gma_m36f_move_region_param (UINT32 layer_id, UINT32 region_id, pcgma_region_t pc_region_param)
{
    RET_CODE ret;
    struct gma_m36f_private *priv = NULL;
    gma_region_t *region;
    alifb_gma_region_t gma_region;
    alifb_gma_region_t *pregion_param;
    gma_rect_t r, block_rect;
    struct gma_block_t *pblock;
    gma_head_m36f_t *pblock_head;
    alifb_gma_region_t *pregion_info;
    UINT8 color_mode;
    UINT32 pixel_pitch;
    UINT32 byte_pitch;
    UINT32 addr;
    
    if (region_id >= GMA_REGION_MAX)
    {
        GMA_PRINTF ("ERROR : invalid region id!\n");
        return RET_FAILURE;
    }
    priv = m_gma_priv[layer_id];
    region = &(priv->tRegion[region_id]);
    pregion_param = &gma_region;

    if (region->bValid == FALSE)
    {
        GMA_PRINTF ("ERROR : invalid region id!\n");
        return RET_FAILURE;
    }

    gma_region = *pc_region_param;

    if (region->bUseExternalMem == FALSE)
    {
        pregion_param->bitmap_x = 0;
        pregion_param->bitmap_y = 0;
        pregion_param->bitmap_w = pregion_param->region_w;
        pregion_param->bitmap_h = pregion_param->region_h;
        pregion_param->pixel_pitch = pregion_param->bitmap_w;
    }

    r.left = pregion_param->region_x;
    r.top = pregion_param->region_y;
    r.width = pregion_param->region_w;
    r.height = pregion_param->region_h;

    if ((UINT32) r.width > pregion_param->bitmap_w - pregion_param->bitmap_x)
        r.width = pregion_param->bitmap_w - pregion_param->bitmap_x;

    if ((UINT32) r.height > pregion_param->bitmap_h - pregion_param->bitmap_y)
        r.height = pregion_param->bitmap_h - pregion_param->bitmap_y;

    if ((UINT16) r.width >= 4096 || (UINT16) r.height >= 4096)
    {
        GMA_PRINTF ("ERROR : invalid region rect!\n");
        return RET_FAILURE;
    }

    //ENTER_GMA_API ();

    pblock = region->pBlock[0];
    pregion_info = &region->region_info;

    gma_region.color_format = ge_m36f_get_hw_pf ((enum GMA_PIXEL_FORMAT)pregion_param->color_format);
    ret = gma_m36f_set_region_param (priv->uOSDLayer, &region->region_info, &gma_region);
    if (ret != RET_SUCCESS)
    {
        //LEAVE_GMA_API ();
        return ret;
    }

    MEMCPY (&(region->tRegionRect), &r, sizeof (gma_rect_t));
    MEMCPY (&(pblock->tRect), &r, sizeof (gma_rect_t));

    color_mode = pregion_info->color_format;
    pixel_pitch = pregion_info->pixel_pitch;

    if (pixel_pitch == 0)
        pixel_pitch = pregion_info->bitmap_w;

    pregion_info->pixel_pitch = pixel_pitch;

    byte_pitch = gma_get_pitch (color_mode, pixel_pitch);
    addr = pregion_info->bitmap_addr;
    pblock_head = &pblock->block_head;

    pblock_head->gma_mode = color_mode;
    pblock_head->clut_segment = pregion_info->pallette_sel;

    if (m_gma_chip_id == ALI_M3202)
    {
        if (pregion_info->galpha_enable == 0)
            pregion_info->global_alpha = 0xff;
        pblock_head->color_by_color = pregion_info->galpha_enable ? 0 : 1;
        pblock_head->global_alpha = pregion_info->global_alpha;
    }

    addr += byte_pitch * pregion_info->bitmap_y;
    addr += gma_get_pitch (color_mode, pregion_info->bitmap_x);

    pblock_head->bitmap_addr = addr & 0x0fffffff;
    pblock_head->pitch = byte_pitch;//pregion_info->pixel_pitch;
    gma_m36f_set_head_rect (priv, pblock_head, &r);

    gma_block_update (layer_id, pblock);

    pblock->puBuf = (UINT8 *) addr;
    pblock->uSize = byte_pitch * r.height;

    //LEAVE_GMA_API ();

    return RET_SUCCESS;
}

static RET_CODE gma_m36f_move_region_param (UINT32 layer_id, UINT32 region_id, pcgma_region_t pregion_param)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    // Update block header in DE vblanking
    UINT32 time_out = 50;
    while (TRUE == priv->bMoveRegion)
    {
        if (0 == time_out)
            break;
        time_out--;
        osal_task_sleep (20);
    }

    ENTER_GMA_API ();

    gma_m36f_sync_block_header(priv, TRUE);

    priv->region_id = region_id;
    priv->region_param = *pregion_param;
    priv->bMoveRegion = TRUE;

#ifdef GMA_HEADER_REALTIME_UPDATE
    gma_m36f_update_block_header(layer_id, FALSE);
#endif

    LEAVE_GMA_API ();

    time_out = 50;

    while (TRUE == priv->bMoveRegion)
    {
        if (0 == time_out)
            break;
        time_out--;
        osal_task_sleep (20);
    }

    return RET_SUCCESS;
}

static RET_CODE gma_m36f_region_fill (UINT32 layer_id, UINT32 region_id, const gma_rect_t * rect, UINT32 uColorData)
{
#if 0    
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);

    if (region->bValid == FALSE)
    {
        GMA_ERR_PRINTF ("ERROR : region NOT created!\n");
        return RET_FAILURE;
    }

    if (!ge_rect_is_inside_size (rect, region->tRegionRect.width, region->tRegionRect.height))
    {
        GMA_ERR_PRINTF ("ERROR : Rect is out of region size !\n");
        return RET_FAILURE;
    }

    //osal_mutex_lock(dev->sema_opert_osd, OSAL_WAIT_FOREVER_TIME);

    ge_cmd_list_hdl cmd_list = priv->ge_cmd_list;
    ge_m36f_cmd_list_new (m_ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
    gma_m36f_set_region_to_ge (layer_id, region_id, cmd_list);

    UINT32 cmd_hdl = ge_m36f_cmd_begin (m_ge_dev, cmd_list, GE_FILL_RECT_BACK_COLOR);
    if ((priv->region_info.pallette_sel & 0x80) && priv->puPalletteHW)
    {
        UINT32 *pal = (UINT32 *) priv->puPalletteHW;
        uColorData = pal[uColorData];
    }
    ge_m36f_set_back_color (m_ge_dev, cmd_hdl, uColorData);
    ge_m36f_set_xy (m_ge_dev, cmd_hdl, GE_DST, rect->left, rect->top);
    ge_m36f_set_wh (m_ge_dev, cmd_hdl, GE_DST, rect->width, rect->height);
    ge_m36f_cmd_end (m_ge_dev, cmd_hdl);
    ge_m36f_cmd_list_end (m_ge_dev, cmd_list);
    //osal_mutex_unlock(dev->sema_opert_osd);
#endif
    return RET_SUCCESS;
}

// Should call this function before modifying any data in block header
// wait_irq == true, wait until the previous modify take effect(show on the screen)
// wait_irq == false, the previous modify will take effect(show on the screen) later
static void gma_m36f_sync_block_header (struct gma_m36f_private *priv, BOOL wait_irq)
{
    UINT32 hw_gma_base;
    UINT32 sw_gma_base;
    gma_head_m36f_t *phead;
    struct gma_block_t *pHead;
    struct gma_block_t *pBlock;
    UINT32 header_id_hw;
    UINT32 header_id_sw;
    gma_head_m36f_t *phead_hw;
    gma_head_m36f_t *phead_sw;
    UINT32 phead_hw_next;
    UINT32 phead_sw_next;
    UINT32 offset;
    
    if (priv->header_need_sync == 0)
        return;

    local_irq_disable();
    sw_gma_base = gma_hal_get_BaseAddr(priv->uOSDLayer);
    local_irq_enable(); 

    if (sw_gma_base != (priv->first_block->puHeadBuf[priv->new_header_id] & 0x0fffffff))
        return;
    
   if (ali_sys_ic_get_chip_id() <= ALI_S3602F)
   {
        if(wait_irq)
            osal_task_sleep(50);
   }
   
    while (priv->bOnOff)
    {
        local_irq_disable();
        hw_gma_base = gma_hal_get_hw_BaseAddr(priv->uOSDLayer);
        local_irq_enable();
  
        if (hw_gma_base == sw_gma_base) // can do sync
            break;

        if (wait_irq)
        {
            osal_task_sleep(5);
        }
        else
        {
            // cancel the update
            priv->header_need_sync = 0;
            phead = &(priv->first_block->block_head);
            gma_hal_SetMask (priv->uOSDLayer);
            gma_hal_Pos (priv->uOSDLayer, phead->start_x, phead->end_x, phead->start_y, phead->end_y);
            gma_hal_BaseAddr(priv->uOSDLayer, priv->first_block->puHeadBuf[1 - priv->new_header_id]);
            gma_hal_ClearMask (priv->uOSDLayer);
            break;
        }
    }

    if (priv->header_need_sync == 1)
    {
        pHead = &priv->tBlockHead;
        pBlock = pHead->next;
        header_id_hw = priv->new_header_id;
        header_id_sw = 1 - priv->new_header_id;
        for (; pBlock && pBlock != pHead; pBlock = pBlock->next)
        {
            phead_hw = (gma_head_m36f_t *) pBlock->puHeadBuf[header_id_hw];
            phead_sw = (gma_head_m36f_t *) pBlock->puHeadBuf[header_id_sw];
	        //MEMCPY((void *)phead_hw + 576, priv->enhance_coef_table, 52);
            // copy the hw header to sw header
             MEMCPY(phead_sw, phead_hw, sizeof(gma_head_m36f_t));
            //MEMCPY(phead_sw, phead_hw, HEADER_SIZE);//modified for 3701c
            
            // correct the next_header address, be sure we use static block header
            phead_hw_next = phead_hw->next_head;
            offset = phead_hw_next - (((UINT32)phead_hw) & 0x0fffffff);
            phead_sw_next = (UINT32)phead_sw + offset;
            phead_sw->next_head = phead_sw_next & 0x0fffffff; // must and it
        }
        priv->header_need_sync = 0;
        priv->new_header_id = header_id_sw;
    }
}

// Should call this function to modify any data in block header
// bUpdate == true, will update priv->first_block to gma register
// bUpdate == false, will update other changes(if any) to gma register
static void gma_m36f_update_block_header (UINT32 layer_id, BOOL bUpdated)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    if (priv == NULL)
        return;

    if (TRUE == priv->bPalMdfUpdated) //for pallete modify
    {
        gma_m36f_update_pal_entry (priv, priv->color_idx, FALSE);
        priv->bPalMdfUpdated = FALSE;
    }
    if (FALSE == priv->bPalletteUpdated)
    {
        gma_m36f_update_pal (layer_id, TRUE);
        priv->bPalletteUpdated = TRUE;
        bUpdated = TRUE;
    }
    if (TRUE == priv->bSetRegionPos)
    {
        _gma_m36f_set_region_pos (layer_id, priv->region_id, &priv->reg_rect_new);
        priv->bSetRegionPos = FALSE;
        bUpdated = TRUE;
    }
    if (TRUE == priv->bMoveRegion)
    {
        _gma_m36f_move_region_param (layer_id, priv->region_id, &priv->region_param);
        priv->bMoveRegion = FALSE;
        bUpdated = TRUE;
    }
    if (TRUE == priv->bSetDisplayRect)
    {
        _gma_m36f_set_display_rect (layer_id, priv->region_id, &priv->display_rect_new);
        priv->bSetDisplayRect = FALSE;
        bUpdated = TRUE;
    }

    if (priv->first_block == NULL)
        return;

    if (bUpdated || priv->first_block_changed)
    {
        gma_head_m36f_t *phead = &(priv->first_block->block_head);
		if (priv->first_block->puHeadBuf[priv->new_header_id])
		{
        	gma_hal_SetMask (priv->uOSDLayer);
        	gma_hal_Pos (priv->uOSDLayer, phead->start_x, phead->end_x, phead->start_y, phead->end_y);
            MEMCPY ((UINT8 *) priv->first_block->puHeadBuf[priv->new_header_id]+576, priv->enhance_coef_table, 52); //modefied for 3701c
        	gma_hal_BaseAddr(priv->uOSDLayer, priv->first_block->puHeadBuf[priv->new_header_id]);
        	gma_hal_ClearMask (priv->uOSDLayer);
			priv->header_need_sync = TRUE;

            if (GMA_HW_LAYER1 != priv->uOSDLayer) 
            {
                while(gma_hal_get_hw_BaseAddr(priv->uOSDLayer) != (0x0fffffff &priv->first_block->puHeadBuf[priv->new_header_id])) 
                {   
                    ;
                }   
            }   
		}
        priv->first_block_changed = FALSE;
        if (priv->bOnOff == 0)
            gma_m36f_sync_block_header(priv, FALSE);
    }
}

#define GMA_SCALE

static const gma_scale_param_t m_osd_scale_map_576[] = {
    {PAL, 1, 1, 1, 1 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576} */ },
    {PAL_N, 1, 1, 1, 1 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576} */ },
    {PAL_NC, 1, 1, 1, 1 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576} */ },

    {NTSC, 1, 6, 1, 5 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },
    {NTSC_443, 1, 6, 1, 5 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },
    {PAL_60, 1, 6, 1, 5 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },
    {PAL_M, 1, 6, 1, 5 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },

    {LINE_720_25, 9, 4, 16, 5 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 } */ },
    {LINE_720_30, 9, 4, 16, 5 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720} */ },

    {LINE_1080_25, 3, 8, 8, 15 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080} */ },
    {LINE_1080_30, 3, 8, 8, 15 /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080} */ },
};

static const gma_scale_param_t m_osd_scale_map_720[] = {
    {PAL, 16, 5, 9, 4, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576} */ },
    {PAL_N, 16, 5, 9, 4, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576} */ },
    {PAL_NC, 16, 5, 9, 4, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576} */ },

    {NTSC, 16, 3, 9, 2, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },
    {NTSC_443, 16, 3, 9, 2, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },
    {PAL_60, 16, 3, 9, 2, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },
    {PAL_M, 16, 3, 9, 2, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },

    {LINE_720_25, 1, 1, 1, 1},
    {LINE_720_30, 1, 1, 1, 1},

    {LINE_1080_25, 2, 2, 3, 3, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080} */ },
    {LINE_1080_30, 2, 2, 3, 3, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080} */ },
};

static const gma_scale_param_t m_osd_scale_map_1080[] = {
    {PAL, 8 * 2, 15, 3 * 2, 8, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576} */ },
    {PAL_N, 8 * 2, 15, 3 * 2, 8, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576} */ },
    {PAL_NC, 8 * 2, 15, 3 * 2, 8, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576} */ },

    {NTSC, 8 * 2, 9, 3 * 2, 4, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },
    {NTSC_443, 8 * 2, 9, 3 * 2, 4, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },
    {PAL_60, 8 * 2, 9, 3 * 2, 4, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },
    {PAL_M, 8 * 2, 9, 3 * 2, 4, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480} */ },

    {LINE_720_25, 3, 3, 2, 2, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 } */ },
    {LINE_720_30, 3, 3, 2, 2, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720} */ },

    {LINE_1080_25, 1, 1, 1, 1, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080} */ },
    {LINE_1080_30, 1, 1, 1, 1, /*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080} */ },
};

#if (GMA_SCALE_TAP_H4V3 == 3)
static BOOL gma_m36f_set_scale_coeff (struct gma_m36f_private *priv)
{
    short tmp0, tmp1, tmp2, tmp3;
    short coeff_table[4 * 16 + 1];
    short coeff_table_v[4 * 16 + 1];
    short *coeff_start_addr;
    struct gma_block_t *pHead;
    struct gma_block_t *pBlock;
    gma_head_m36f_t *phead;
    UINT32 header_id;
    UINT32 reg_offset;
    UINT32 i;
    BOOL ret;

    if (m_gma_chip_id == ALI_S3811 || m_gma_chip_id == ALI_C3701)
    {
        //short *coeff_start_addr = short *(m_osd_block_head + 16);
        pHead = &priv->tBlockHead;
        pBlock = pHead->next;
        header_id = priv->new_header_id;
              
        ret = vpo_hal_generate_scale_coeff(coeff_table, priv->h_div, priv->h_mul, 4, 16, FALSE);
        if (!ret)
            return FALSE;
        
        ret = vpo_hal_generate_scale_coeff(coeff_table_v, priv->v_div, priv->v_mul, 3, 16, FALSE);
        if (!ret)
            return FALSE;
        
        for (; pBlock && pBlock != pHead; pBlock = pBlock->next)
        {
            if (pBlock->uStatus == BLOCK_HIDE)
                continue;

            phead = (gma_head_m36f_t *) pBlock->puHeadBuf[header_id];
            coeff_start_addr = (short *)phead + 8  * 4;//8Qword + header_base
            
            //fill the block header one depth(128bit,2Qword) after depth,total 16 depth
            for (i=0; i<16; i++)
            {
                // hor, tap = 4
                tmp3 = coeff_table[16*(3-3)+i];
                tmp2 = coeff_table[16*(3-2)+i];
                tmp1 = coeff_table[16*(3-1)+i];
                tmp0 = coeff_table[16*(3-0)+i];

                *coeff_start_addr = *coeff_start_addr | (tmp0 & 0x0FF);
                coeff_start_addr++;
                *coeff_start_addr = *coeff_start_addr | (tmp1 & 0x1FF);
                coeff_start_addr++;
                *coeff_start_addr = *coeff_start_addr | (tmp2 & 0x1FF);
                coeff_start_addr++;
                *coeff_start_addr = *coeff_start_addr | (tmp3& 0x0FF);
                coeff_start_addr += 5;
            }

            //fill the block header one depth(128bit,2Qword) after depth,total 16 depth 
            for (i=0; i<16; i++)
            {
                // hor, tap = 3
                tmp2 = coeff_table_v[16*(2-2)+i];
                tmp1 = coeff_table_v[16*(2-1)+i];
                tmp0 = coeff_table_v[16*(2-0)+i];

                *coeff_start_addr = *coeff_start_addr | (tmp0 & 0x1FF);
                coeff_start_addr++;
                *coeff_start_addr = *coeff_start_addr | (tmp1 & 0x1FF);
                coeff_start_addr++;
                *coeff_start_addr = *coeff_start_addr | (tmp2 & 0x1FF);
                coeff_start_addr += 6;
            }     
        }
        
        return TRUE;
    }
    else
    {
        ret = vpo_hal_generate_scale_coeff(coeff_table, priv->h_div, priv->h_mul, 4, 16, FALSE);
        if (!ret)
            return FALSE;

        if (priv->uOSDLayer == GMA_HW_LAYER0)
            reg_offset = 0x214;
        else
            reg_offset = 0x294;

        for (i=0; i<16; i++)
        {
            // hor, tap = 4
            tmp3 = coeff_table[16*(3-3)+i];
            tmp2 = coeff_table[16*(3-2)+i];
            tmp1 = coeff_table[16*(3-1)+i];
            tmp0 = coeff_table[16*(3-0)+i];
            gma_WriteDword(reg_offset, i); // bit16 == 0, write coeff
            gma_WriteDword(reg_offset+4, ((tmp3&0x0FF)<<16)|(tmp2&0x1FF) );
            gma_WriteDword(reg_offset+8, ((tmp1&0x1FF)<<16)|(tmp0&0x0FF) );
        }

        ret = vpo_hal_generate_scale_coeff(coeff_table, priv->v_div, priv->v_mul, 3, 16, FALSE);
        if (!ret)
            return FALSE;

        if (priv->uOSDLayer == GMA_HW_LAYER0)
            reg_offset = 0x220;
        else
            reg_offset = 0x2a0;

        for (i=0; i<16; i++)
        {
            // hor, tap = 3
            tmp2 = coeff_table[16*(2-2)+i];
            tmp1 = coeff_table[16*(2-1)+i];
            tmp0 = coeff_table[16*(2-0)+i];
            gma_WriteDword(reg_offset, i); // bit16 == 0, write coeff
            gma_WriteDword(reg_offset+4, ((tmp2&0x1FF)<<18)|((tmp1&0x1FF)<<9)|(tmp0&0x1FF) );
        }

        return TRUE;
    }
}
#endif

static UINT32 gma_m36f_get_screen_pitch (struct gma_m36f_private *priv)
{
    UINT32 i;
    UINT32 pitch = 720;

    for (i = 0; i < GMA_REGION_MAX; i++)
    {
        gma_region_t *region = &(priv->tRegion[i]);
        if (region->bValid)
        {
            if (pitch < (UINT32)(region->tRegionRect.left + region->tRegionRect.width))
            {
                pitch = region->tRegionRect.left + region->tRegionRect.width;
            }
        }
    }

    return get_screen_pitch(pitch);
}

static pcgma_scale_param_t osd_s3601_get_scale_param (enum TVSystem eTVMode, UINT32 nScreenWidth)
{
    UINT32 i, size_576, size_720, size_1080, size;
    const gma_scale_param_t *p_ui_map;

    if (eTVMode == LINE_1080_24 || eTVMode == LINE_1152_ASS || eTVMode == LINE_1080_ASS || eTVMode == LINE_1080_50)
        eTVMode = LINE_1080_25;
    else if (eTVMode == LINE_1080_60)
        eTVMode = LINE_1080_30;

    size_720 = ARRAY_SIZE (m_osd_scale_map_720);
    size_1080 = ARRAY_SIZE (m_osd_scale_map_1080);
    size_576 = ARRAY_SIZE (m_osd_scale_map_576);
    if (nScreenWidth == 1280)
    {
        size = size_720;
        p_ui_map = m_osd_scale_map_720;
    }
    else if (nScreenWidth == 1920)
    {
        size = size_1080;
        p_ui_map = m_osd_scale_map_1080;
    }
    else                        // if(nScreenWidth == 720)
    {
        //ASSERT(0);
        size = size_576;
        p_ui_map = m_osd_scale_map_576;
    }
    for (i = 0; i < size; i++)
    {
        if (p_ui_map[i].tv_sys == eTVMode)
        {
            return &(p_ui_map[i]);
        }
    }
    // default to PAL
    return &(p_ui_map[0]);
}

static void gma_m36f_set_scale_param (struct gma_m36f_private *priv, pcgma_scale_param_t pscale_param)
{
    UINT32 h_div, v_div, h_mul, v_mul;
    UINT8 scale_dir_h, scale_dir_v;
    UINT32 scale_param_h, scale_param_v;
    UINT8 scale_mode = GMA_SCALE_FILTER, scale_ep_on = GMA_SCALE_EP_OFF;

    if (pscale_param == NULL)
        return;

    h_div = pscale_param->h_div;
    v_div = pscale_param->v_div;
    h_mul = pscale_param->h_mul;
    v_mul = pscale_param->v_mul;

    if (m_gma_chip_id == ALI_M3202) // 32c only support vertical scale
    {
        h_div = 1;
        h_mul = 1;
    }
    if (!m_config.bScaleFilterEnable)
        scale_mode = GMA_SCALE_DUPLICATE;

    if (h_mul == h_div)
    {
        h_mul = 1;
        h_div = 1;
        scale_dir_h = GMA_SCALE_OFF;
    }
    else if (h_mul > h_div)
    {
        scale_dir_h = GMA_SCALE_UP;
    }
    else
    {
        scale_dir_h = GMA_SCALE_DOWN;
    }

    if (v_mul == v_div)
    {
        v_mul = 1;
        v_div = 1;
        scale_dir_v = GMA_SCALE_OFF;
    }
    else if (v_mul > v_div)
    {
        scale_dir_v = GMA_SCALE_UP;
    }
    else
    {
        scale_dir_v = GMA_SCALE_DOWN;
    }

    scale_dir_h |= scale_dir_v;

    if (scale_dir_h == GMA_SCALE_OFF)
        priv->scale_dir = GMA_SCALE_OFF;
    else if (scale_dir_h == GMA_SCALE_DOWN)
    {
        priv->scale_dir = GMA_SCALE_DOWN;
        //gma_hal_filter_tap (priv->uOSDLayer, GMA_SCALE_TAP_H4V3);
    }
    else
    {
        // scale up factor >= 1.5, turn EP on. need to tune
        // if (h_mul * 2 >= h_div * 3 || v_mul * 2 >= v_div * 3)
        scale_ep_on = GMA_SCALE_EP_ON;

        priv->scale_dir = GMA_SCALE_UP;
        //if (scale_ep_on)
        //    gma_hal_filter_tap (priv->uOSDLayer, GMA_SCALE_TAP_H2V2);
        //else
        //    gma_hal_filter_tap (priv->uOSDLayer, GMA_SCALE_TAP_H4V3);
    }

    priv->tv_sys = (enum TVSystem) (pscale_param->tv_sys & 0xff);
    priv->h_div = h_div;
    priv->h_mul = h_mul;
    priv->v_div = v_div;
    priv->v_mul = v_mul;

    priv->ep_on = scale_ep_on;
    if (scale_mode == GMA_SCALE_DUPLICATE)
        priv->scale_mode = 1;
    else
        priv->scale_mode = 0;

    scale_param_h = h_div * 4096 / h_mul;
    scale_param_v = v_div * 4096 / v_mul;
    priv->h_scale_param = scale_param_h;
    priv->v_scale_param = scale_param_v;

    if (m_gma_chip_id == ALI_M3202)
    {
        UINT32 int_val = 0, fra_val = 0;

        int_val = ((v_div / v_mul) & 0x07) << 16;
        fra_val = v_div % v_mul;
        fra_val = (((fra_val * 4096) / v_mul) & 0xFFF) << 20;
        if (scale_mode == GMA_SCALE_DUPLICATE || scale_dir_v == GMA_SCALE_OFF)
        {
            gma_hal_VFilterOnOff (priv->uOSDLayer, 0);
        }
        else
        {
            gma_hal_VFilterOnOff (priv->uOSDLayer, 1);
            int_val |= 0x8000;
        }
        gma_hal_VFilter (priv->uOSDLayer, fra_val | int_val);
    }
    else // S3602F or S3811
    {
        if (priv->scale_dir == GMA_SCALE_DOWN || priv->scale_dir == GMA_SCALE_UP)
        {
#if (GMA_SCALE_TAP_H4V3 == 3)
            gma_m36f_set_scale_coeff (priv);
#endif
            if (priv->ep_on == GMA_SCALE_EP_OFF)
            {
                if (v_mul * 2 < v_div) // scale down > 2, use H4V3 mode
                    gma_hal_filter_tap (priv->uOSDLayer, GMA_SCALE_TAP_H4V3);
                else
                    gma_hal_filter_tap (priv->uOSDLayer, GMA_SCALE_TAP_H2V2);
            }
            else
                gma_hal_filter_tap (priv->uOSDLayer, GMA_SCALE_TAP_H2V2);
        }
        else
        {
            gma_hal_filter_tap (priv->uOSDLayer, GMA_SCALE_TAP_H2V2);
        }
    }
}

#define DEBUG_INFO

#if (OSDDRV_BASIC_DEBUG)
void print_rect (gma_rect_t * rect)
{
    GMA_PRINTF ("{ %d, %d, %d, %d }\n", rect->left, rect->top, rect->width, rect->height);
}

void gma_m36f_print_region (UINT32 layer_id)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    UINT32 i, j;
    gma_region_t *region = &(priv->tRegion[0]);
    UINT8 *block_buf;
    UINT32 block_size;
    UINT32 request_size;
    struct gma_block_t *block;
    gma_rect_t r, block_rect;

    GMA_PRINTF ("%s: \n", __FUNCTION__);
    for (i = 0; i < GMA_REGION_MAX; i++, region++)
    {
        if (region->bValid == TRUE)
        {
            GMA_PRINTF ("region[%d]: Color = %d, ", i, ge_m36f_get_sw_pf (region->region_info.color_format));
            print_rect (&(region->tRegionRect));
            for (j = 0; j < MAX_BLOCK_IN_REGION; j++)
            {
                block = region->pBlock[j];
                if (block)
                {
                    GMA_PRINTF ("block[%d], buf = 0x%08x, size = 0x%08x\n", j, block->puBuf, block->uSize);
                    // print_rect(&(block->tDisRect));
                    // print_rect(&(block->tRect));
                }
            }
        }
    }
}

static void gma_m36f_print_block_head (UINT32 addr)
{
    if (addr == 0)
        return;
    UINT32 header_padding = 0;
    UINT32 addr0 = (UINT32) addr;
    addr0 += header_padding;
    addr0 &= 0xfffffff0;
    UINT32 *buf32 = (UINT32 *) addr0;
    GMA_PRINTF ("head addr = 0x%08x\n", addr0);
    UINT32 i;
    for (i = 0; i < HEADER_DW_NUM; i++)
        GMA_PRINTF ("head[%d] = 0x%08x\n", i, buf32[i]);
}

void gma_m36f_print_block (UINT32 layer_id)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    GMA_PRINTF ("********** %s: *********\n", __FUNCTION__);
    struct gma_block_t *head = &(priv->tBlockHead);
    struct gma_block_t *block = head->next;
    UINT32 i = 0;
    UINT32 size = 0;
    while (block != head)
    {
        GMA_PRINTF ("block[%d], addr = 0x%08x, buf = 0x%08x, size = 0x%08x\n", i, block, block->puBuf, block->uSize);
        print_rect (&(block->tRect));
        size += block->uSize;
        gma_m36f_print_block_head (block->puHeadBuf[priv->new_header_id]);
        gma_m36f_print_block_head (block->block_head.clut_base);

        block = block->next;
        i++;
    }
    GMA_PRINTF ("Total OSD buf size = 0x%08x, %d\n\n", size, size);
}
#else
#ifdef _WINDOWS
#define gma_m36f_print_region
#define gma_m36f_print_block
#define print_rect
#else
#define gma_m36f_print_region(...)
#define gma_m36f_print_block(...)
#define print_rect(...)
#endif
#endif

#define GMA_API

RET_CODE gma_m36f_create_region (UINT32 layer_id, UINT32 region_id, const alifb_gma_region_t * ptPara)
{
    UINT8 *block_buf;
    UINT8 color_mode;
    UINT32 pixel_pitch;
    UINT32 i;
    UINT32 block_size;
    UINT32 fill_color;
    UINT32 line_size;
    UINT32 request_height;
    UINT32 addr;
    UINT32 height;
    gma_rect_t rect0;
    gma_rect_t *rect;
    gma_rect_t r, block_rect;
    gma_rect_t fill_rect;
    struct gma_m36f_private *priv = NULL;
    gma_region_t *region = NULL;
    alifb_gma_region_t gma_region;    
    RET_CODE ret;

    if (region_id >= GMA_REGION_MAX)
    {
        GMA_ERR_PRINTF ("ERROR : invalid region id!\n");
        return RET_FAILURE;
    }

    priv = m_gma_priv[layer_id];
    region = &(priv->tRegion[region_id]);
    if (region->bValid == TRUE)
    {
        GMA_PRINTF ("ERROR : region already created!\n");
        return RET_SUCCESS;     //ASSERT(0);
    }

    if (ptPara != NULL && ptPara->bitmap_addr != 0)
    {
        return gma_m36f_create_region_param (layer_id, region_id, ptPara);
    }

    ENTER_GMA_API ();

    gma_m36f_sync_block_header(priv, TRUE);

    if (ptPara == NULL)
    {
        ptPara = &priv->region_info;
    }    
    else
    {
        MEMCPY ((void *) &gma_region, (void *) ptPara, sizeof (alifb_gma_region_t));
        gma_region.color_format = ge_m36f_get_hw_pf ((enum GMA_PIXEL_FORMAT)ptPara->color_format);
        ptPara = &gma_region;
    }

    rect = &rect0;
    rect->left = ptPara->region_x;
    rect->top = ptPara->region_y;
    rect->width = ptPara->region_w;
    rect->height = ptPara->region_h;

    GMA_PRINTF ("%s: layer %d format %d region[%d] = ", __FUNCTION__, priv->uOSDLayer, ptPara->color_format, region_id);
    GMA_PRINTF ("{ %d, %d, %d, %d }\n", rect->left, rect->top, rect->width, rect->height);

    MEMCPY (&r, rect, sizeof (gma_rect_t));
    MEMCPY (&(region->tRegionRect), rect, sizeof (gma_rect_t));

    ret = gma_m36f_set_region_param (priv->uOSDLayer, &region->region_info, ptPara);
    if (ret != RET_SUCCESS)
    {
        LEAVE_GMA_API ();
        return ret;
    }

    color_mode = ptPara->color_format;
    pixel_pitch = ptPara->pixel_pitch;

    if (pixel_pitch == 0)
        pixel_pitch = rect->width;

    region->region_info.pixel_pitch = pixel_pitch;
    region->uStatus = BLOCK_SHOW;
    region->bUseMemSlice = TRUE;
    region->bUseExternalMem = FALSE;

    fill_rect.left = 0;
    fill_rect.top = 0;
    fill_rect.width = rect->width;
    fill_rect.height = rect->height;

    fill_color = 0;
    //fill_color = 0xffffff00;
    if (color_mode == GMA36F_PF_CLUT8)
        fill_color = (UINT8) priv->trans_color;
    else if (color_mode == GMA36F_PF_CLUT4 || color_mode == GMA36F_PF_CLUT2)
        fill_color = 0xff;

    line_size = gma_get_pitch (color_mode, pixel_pitch);
    request_height = rect->height;

    for (i = 0; i < MAX_BLOCK_IN_REGION; i++)
    {
        //Alex Wu 061003
        if (request_height <= 1)
        {
            if (request_height == 1)
                request_height = 2;
            else
            {
                region->pBlock[i] = NULL;
                continue;
            }
        }
        if (priv->uOSDLayer == 0)
        {
            malloc_me (priv, request_height * line_size, &block_buf, &block_size, 1);
        }
        else
        {
            addr = priv->layer_config.mem_base;
#ifdef ALI_FB_CREATE_NEW
            //addr += line_size * r.top;
#else
            addr += line_size * r.top;
#endif
            block_buf = (UINT8 *) addr;
            block_size = request_height * line_size;
            region->bUseMemSlice = FALSE;
            if (addr + block_size > (priv->layer_config.mem_base + priv->layer_config.mem_size))
                break;  // memory not enough
        }

        GMA_PRINTF ("Region[%d]->block[%d]: block_buf = 0x%x,block_size = %d, end addr = 0x%x\n", region_id, i, block_buf, block_size,
                    block_buf + block_size);
        if (block_size != 0)
        {
            block_rect.left = r.left;
            block_rect.width = r.width;
            block_rect.top = r.top;
            height = block_size / line_size;
            //            height &= ~1;   // block height must be even
            if (height)
            {
                block_rect.height = height;

                region->pBlock[i] = gma_block_create (layer_id, &block_rect, block_buf, height * line_size, &region->region_info);

                r.top += height;
                request_height -= height;

#if 1
                MEMSET(block_buf, fill_color, height * line_size);

                fill_rect.height = height;
                region->bValid = TRUE;
                //if (priv->auto_clear_region)
                //    gma_m36f_region_fill (layer_id, region_id, &fill_rect, fill_color);

                // write OSD memory first, write register later
                gma_fill_hw_head (priv->uOSDLayer, &priv->tBlockHead, region->pBlock[i]);
                gma_fill_hw_head (priv->uOSDLayer, &priv->tBlockHead, region->pBlock[i]->prev);
#endif

            }
            else
            {
                break;
            }
        }
        else
        {
            GMA_PRINTF ("ERROR : malloc failure!\n");
            //ASSERT (0);
            break;  // memory not enough
        }
    }

    if (request_height)
    {
        GMA_PRINTF ("ERROR : malloc failure!\n");
        LEAVE_GMA_API ();
        return RET_FAILURE;     //ASSERT(0);
    }

    gma_m36f_update_block_header(layer_id, TRUE);

    write_gma_header_to_fpga();

    region->bValid = TRUE;
    region_monitor (priv, REGION_ON);

    LEAVE_GMA_API ();
    return RET_SUCCESS;
}

 RET_CODE gma_m36f_delete_region (UINT32 layer_id, UINT32 region_id)
{
    UINT32 i;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);

#if (OSDDRV_BASIC_DEBUG)
    //libc_printf("%s: region[%d] = ", __FUNCTION__, region_id);
    //libc_printf("{ %d, %d, %d, %d }\n", region->tRegionRect.left, region->tRegionRect.top, region->tRegionRect.width, region->tRegionRect.height);
#endif
    if (region->bValid == FALSE)
    {
        GMA_PRINTF ("ERROR : region NOT created!\n");
        return RET_SUCCESS;     //ASSERT(0);
    }

    ENTER_GMA_API ();

    gma_m36f_sync_block_header(priv, TRUE);

    region->bValid = FALSE;
    region_monitor (priv, REGION_OFF);
    for (i = 0; i < MAX_BLOCK_IN_REGION; i++)
    {
        if (region->pBlock[i] != NULL)
        {
            if (region->bUseMemSlice)
                free_me (region->pBlock[i]->puBuf, region->pBlock[i]->uSize);
            gma_block_delete (layer_id, region->pBlock[i]);
            region->pBlock[i] = NULL;
        }
    }

    gma_m36f_update_block_header(layer_id, TRUE);

    LEAVE_GMA_API ();

    return RET_SUCCESS;
}

RET_CODE gma_m36f_get_region_info (UINT32 layer_id, UINT32 region_id, alifb_gma_region_t * ptPara)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);
    alifb_gma_region_t *pregion_info;
    BOOL use_default;

    if (region_id >= GMA_REGION_MAX || !region->bValid)
        use_default = TRUE;
    else
        use_default = FALSE;

    if (use_default)
        pregion_info = &priv->region_info;
    else
        pregion_info = &region->region_info;

    MEMCPY (ptPara, pregion_info, sizeof (alifb_gma_region_t));
    ptPara->color_format = ge_m36f_get_sw_pf (pregion_info->color_format);
    if (ptPara->bitmap_addr == 0)
    {
        if (region->bValid)
            ptPara->bitmap_addr = (UINT32)region->pBlock[0]->puBuf;
        else
            ptPara->bitmap_addr = (UINT32)priv->layer_config.mem_base;
    }

    return RET_SUCCESS;
}

RET_CODE gma_m36f_move_region (UINT32 layer_id, UINT32 region_id, pcgma_region_t pregion_param)
{
    struct gma_m36f_private *priv = NULL;
    gma_region_t *region;
    gma_rect_t rect;
    alifb_gma_region_t *pregion_info; 
    
    if (pregion_param == NULL)
        return RET_FAILURE;

    if (region_id >= GMA_REGION_MAX)
    {
        GMA_PRINTF ("ERROR : invalid region id!\n");
        return RET_FAILURE;
    }
    priv = m_gma_priv[layer_id];
    region = &(priv->tRegion[region_id]);

    if (region->bValid == FALSE)
    {
        GMA_PRINTF ("ERROR : invalid region id!\n");
        if (pregion_param && region_id == 0)
        {
            pregion_info = &priv->region_info;
            MEMCPY (pregion_info, pregion_param, sizeof (alifb_gma_region_t));
            pregion_info->color_format = ge_m36f_get_hw_pf (pregion_param->color_format);
            return RET_SUCCESS;
        }
        return RET_FAILURE;
    }

    if ((pregion_param->region_w != region->region_info.region_w)
        || (pregion_param->region_h != region->region_info.region_h))
    {
        return gma_m36f_move_region_param (layer_id, region_id, pregion_param);
    }

    if (!region->bUseExternalMem || pregion_param->bitmap_addr == 0)
    {       
        if (pregion_param->region_x == region->region_info.region_x && pregion_param->region_y == region->region_info.region_y)
            return RET_SUCCESS;

        rect.left = pregion_param->region_x;
        rect.top = pregion_param->region_y;
        rect.width = region->region_info.region_w;
        rect.height = region->region_info.region_h;

        return gma_m36f_set_region_pos (layer_id, region_id, &rect);
    }

    return gma_m36f_move_region_param (layer_id, region_id, pregion_param);
}

RET_CODE gma_m36f_set_region_info (UINT32 layer_id, UINT32 region_id, alifb_gma_region_t * pregion_param)
{
    struct gma_m36f_private *priv = NULL;
    gma_region_t *region;
    gma_head_m36f_t *phead;

   if (pregion_param == NULL)
        return RET_FAILURE;

    if (region_id >= GMA_REGION_MAX)
    {
        GMA_PRINTF ("ERROR : invalid region id!\n");
        return RET_FAILURE;
    }
    priv = m_gma_priv[layer_id];
    region = &(priv->tRegion[region_id]); 
    phead = &(priv->first_block->block_head);

    return gma_m36f_move_region_param (layer_id, region_id, pregion_param);
}
 RET_CODE gma_m36f_region_showonoff (UINT32 layer_id, UINT32 region_id, BOOL bOn)
{
    UINT32 i;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);

    if (region->bValid == FALSE)
    {
        GMA_PRINTF ("ERROR : region NOT created!\n");
        return RET_FAILURE;     //ASSERT(0);
    }

    ENTER_GMA_API ();

    gma_m36f_sync_block_header(priv, TRUE);

    region->uStatus = bOn ? BLOCK_SHOW : BLOCK_HIDE;
    if (!bOn)
        region_monitor (priv, REGION_OFF);

    for (i = 0; i < MAX_BLOCK_IN_REGION; i++)
    {
        struct gma_block_t *pBlock = region->pBlock[i];
        if (pBlock != NULL)
        {
            if (bOn == TRUE)
                gma_block_show (layer_id, pBlock);
            else
                gma_block_hide (layer_id, pBlock);
        }
    }

    if (bOn)
        region_monitor (priv, REGION_ON);

    gma_m36f_update_block_header(layer_id, TRUE);

    LEAVE_GMA_API ();

    return RET_SUCCESS;
}

RET_CODE gma_m36f_set_clip_rect (UINT32 layer_id, UINT32 x, UINT32 y, UINT32 w, UINT32 h)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    ENTER_GMA_API ();

    priv->Clip_Rect.left = x;
    priv->Clip_Rect.top = y;
    priv->Clip_Rect.width = w;
    priv->Clip_Rect.height = h;

    LEAVE_GMA_API ();

    return RET_SUCCESS;
}

RET_CODE gma_m36f_set_clip_mode (UINT32 layer_id, enum GE_CLIP_MODE clip_mode)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    ENTER_GMA_API ();
    priv->eClipMode = clip_mode;
    LEAVE_GMA_API ();

    return RET_SUCCESS;
}
#if 0
RET_CODE gma_m36f_set_region_to_ge (UINT32 layer_id, UINT32 region_id, ge_cmd_list_hdl cmd_list)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);
    struct gma_block_t *block = region->pBlock[0];
    gma_rect_t *rect = NULL;
    gma_head_m36f_t *phead;
    UINT32 cmd_base;
    ge_operating_entity base_addr;

    if (region_id >= GMA_REGION_MAX || region->bValid == FALSE)
    {
        //GMA_PRINTF("ERROR : region[%d] NOT created!\n", region_id);
        return RET_FAILURE;
    }

    if (!ge_m36f_cmd_list_is_valid (m_ge_dev, cmd_list))
    {
        //GMA_PRINTF("ERROR : cmd_list[0x%08x] is invalid!\n", cmd_list);
        return RET_FAILURE;
    }

    cmd_base = cmd_list;

    phead = &block->block_head;

    base_addr.color_format = ge_m36f_get_sw_pf (region->region_info.color_format);
    base_addr.base_address = phead->bitmap_addr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = region->region_info.pixel_pitch;
    base_addr.modify_flags = GE_BA_FLAG_ADDR | GE_BA_FLAG_FORMAT | GE_BA_FLAG_PITCH;

    ge_m36f_set_operating_entity (m_ge_dev, cmd_base, GE_DST, &base_addr);
    ge_m36f_set_operating_entity (m_ge_dev, cmd_base, GE_SRC, &base_addr);

    if (phead->gma_mode >= GE36F_PF_CLUT2 && phead->gma_mode <= GE36F_PF_ACLUT88)
    {
        ge_m36f_set_clut_addr (m_ge_dev, cmd_base, phead->clut_base);
        ge_m36f_set_color_format (m_ge_dev, cmd_base, base_addr.color_format);
    }
    else
    {
        ge_m36f_set_color_format (m_ge_dev, cmd_base, GE_PF_ARGB8888);
    }

    ge_m36f_set_rgb_order (m_ge_dev, cmd_base, GE_SRC, GE_RGB_ORDER_ARGB);
    ge_m36f_set_byte_endian (m_ge_dev, cmd_base, GE_SRC, GE_BYTE_ENDIAN_LITTLE);
    ge_m36f_set_subbyte_endian (m_ge_dev, cmd_base, GE_SRC, GE_SUBBYTE_RIGHT_PIXEL_LSB);

    ge_m36f_set_rgb_order (m_ge_dev, cmd_base, GE_PTN, GE_RGB_ORDER_ARGB);
    ge_m36f_set_byte_endian (m_ge_dev, cmd_base, GE_PTN, GE_BYTE_ENDIAN_LITTLE);
    ge_m36f_set_subbyte_endian (m_ge_dev, cmd_base, GE_PTN, GE_SUBBYTE_RIGHT_PIXEL_LSB);

    ge_m36f_set_subbyte_endian (m_ge_dev, cmd_base, GE_MSK, GE_SUBBYTE_RIGHT_PIXEL_LSB);

    //ge_m36f_set_global_alpha(m_ge_dev, cmd_base, 0xff);
    //ge_m36f_set_global_alpha_sel(m_ge_dev, cmd_base, GE_USE_GALPHA_MULTIPLY_PTN_ALPHA);

    ge_m36f_set_xy (m_ge_dev, cmd_base, GE_DST_SRC, 0, 0);
    ge_m36f_set_wh (m_ge_dev, cmd_base, GE_DST_SRC, phead->source_width, phead->source_height);

    enum GE_CLIP_MODE ge_clip_mode = priv->eClipMode;

    ge_m36f_set_clip_mode (m_ge_dev, cmd_base, ge_clip_mode);

    if (ge_clip_mode != GE_CLIP_DISABLE)
    {
        ge_m36f_set_clip_rect (m_ge_dev, cmd_base, priv->Clip_Rect.left, priv->Clip_Rect.top, priv->Clip_Rect.width, priv->Clip_Rect.height);
    }

    if (region->region_info.color_format < GE36F_PF_CLUT2) // RGB
    {
        if ((region->region_info.pallette_sel & 0x80) && priv->puPalletteHW) // do color expansion
        {
            ge_m36f_set_clut_addr (m_ge_dev, cmd_base, (UINT32) priv->puPalletteHW);
            ge_m36f_set_clut_color_cvt (m_ge_dev, cmd_base, GE_COLOR_CVT_BT601, GE_COLOR_SPACE_VIDEO_MATRIX);
            ge_m36f_set_clut_rgb_order (m_ge_dev, cmd_base, GE_RGB_ORDER_ARGB);

            ge_m36f_set_clut_mode (m_ge_dev, cmd_base, GE_CLUT_COLOR_EXPANSION, FALSE);

            ge_m36f_set_clut_update (m_ge_dev, cmd_base, TRUE);
        }
    }

    return RET_SUCCESS;
}
#endif

RET_CODE gma_m36f_set_display_rect(UINT32 layer_id, UINT8 region_id, gma_rect_t *rect)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);
    UINT32 time_out = 50;
    
    if(region->bValid == FALSE)
    {
        GMA_PRINTF("ERROR : region NOT created!\n");
        return RET_FAILURE;//ASSERT(0);
    }

    // Update block header in DE vblanking
    while (TRUE == priv->bSetDisplayRect)
    {
        if(0 == time_out)
            break;
        time_out--;
        osal_task_sleep(20);
    }

    ENTER_GMA_API();

    gma_m36f_sync_block_header(priv, TRUE);
    
    priv->display_rect_new = *rect;
    priv->region_id = region_id;
    priv->bSetDisplayRect = TRUE;

#ifdef GMA_HEADER_REALTIME_UPDATE
    gma_m36f_update_block_header(layer_id, FALSE);
#endif

    LEAVE_GMA_API();

    time_out = 50;

    while (TRUE == priv->bSetDisplayRect)
    {
        if(0 == time_out)
            break;
        time_out--;
        osal_task_sleep(20);
    }

    return RET_SUCCESS;
}

static RET_CODE gma_m36f_get_display_rect (UINT32 layer_id, UINT32 region_id, gma_rect_t *posd_rect)
{
    BOOL use_default;
    gma_head_m36f_t *phead;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    gma_region_t *region = &(priv->tRegion[region_id]);
    
    if (region_id >= GMA_REGION_MAX || !region->bValid)
        use_default = TRUE;
    else
        use_default = FALSE;

    if (use_default)
        return RET_FAILURE;

    phead = &(region->pBlock[0]->block_head);
    posd_rect->left = phead->start_x;
    posd_rect->width = phead->end_x;
    posd_rect->width = posd_rect->width + 1 - posd_rect->left;

    posd_rect->top = phead->start_y;
    posd_rect->height = phead->end_y;
    posd_rect->height = posd_rect->height + 1 - posd_rect->top;

    return RET_SUCCESS;
}

#ifdef ALI_FB_CREATE_NEW
RET_CODE gma_m36f_store_scale_info(UINT32 layer_id, gma_scale_param_t *scale_param)
{
	struct gma_m36f_private *priv = m_gma_priv[layer_id];
	priv->h_div = scale_param->h_div;
	priv->v_div = scale_param->v_div;
	priv->h_mul = scale_param->h_mul;
	priv->v_mul = scale_param->v_mul;

	return RET_SUCCESS;
}
#endif

RET_CODE gma_m36f_scale (UINT32 layer_id, UINT32 uScaleCmd, UINT32 uScaleParam)
{
    UINT32 i, j;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    struct gma_m36f_private *priv_ui;
    gma_region_t *region;
    enum TVSystem tv_system = PAL;
    pcgma_scale_param_t pscale_param = NULL;
    gma_scale_param_t scale_param;
    struct gma_block_t *pBlock;
    
    ENTER_GMA_API ();

    gma_m36f_sync_block_header(priv, TRUE);

    switch (uScaleCmd)
    {
        case GE_VSCALE_OFF:
            if (m_config.bP2NScaleInNormalPlay)
            {
                tv_system = *((enum TVSystem *) uScaleParam);
                if (tv_system == PAL)
                {
                    pscale_param = &m_gma_scale_off;
                }
                else if (tv_system == NTSC)
                {
                    pscale_param = &m_gma_scale_p2n;
                }
            }
            else
            {
                pscale_param = &m_gma_scale_off;
            }
            break;
        case GE_VSCALE_TTX_SUBT:
            gp_gma_m36f_private0->screen_width = gma_m36f_get_screen_pitch(gp_gma_m36f_private0);
            gp_gma_m36f_private1->screen_width = gma_m36f_get_screen_pitch(gp_gma_m36f_private1);

            if (priv == gp_gma_m36f_private0)
            {
                priv_ui = gp_gma_m36f_private1;
            }
            else
            {
                priv_ui = gp_gma_m36f_private0;
            }

            if (priv->screen_width == 720 && priv_ui->screen_width == 720)
            {
                priv->tv_sys = priv_ui->tv_sys = osd_m36f_get_tv_system();
                priv->h_mul = priv_ui->h_mul;
                priv->h_div = priv_ui->h_div;
                priv->v_mul = priv_ui->v_mul;
                priv->v_div = priv_ui->v_div;

                if (priv->layer_config.bP2NScaleInSubtitlePlay)
                {
                    if (!priv_ui->layer_config.bP2NScaleInNormalPlay)
                    {
                        if (priv_ui->tv_sys == LINE_720_30)
                        {
                            priv->v_div = 4;
                            priv->v_mul = 5;
                        }
                        else if (priv_ui->tv_sys == LINE_1080_30)
                        {
                            priv->v_div = 8;
                            priv->v_mul = 15;
                        }
                        else if (priv_ui->tv_sys == NTSC)
                        {
                            priv->v_mul = 5;
                            priv->v_div = 6;
                        }
                    }
                }
                scale_param.tv_sys = priv->tv_sys;
                scale_param.h_mul = priv->h_mul;
                scale_param.h_div = priv->h_div;
                scale_param.v_mul = priv->v_mul;
                scale_param.v_div = priv->v_div;
                pscale_param = &scale_param;
                GMA_PRINTF ("scaling GMA layer 2: h_mul, h_div, v_mul, v_div: %d, %d, %d, %d\n", priv->h_mul, priv->h_div, priv->v_mul, priv->v_div);
            }
            else
            {
                priv->tv_sys = priv_ui->tv_sys = osd_m36f_get_tv_system();
                pscale_param = osd_s3601_get_scale_param (priv->tv_sys, priv->screen_width);
            }
            break;
        case GE_H_DUPLICATE_ON_OFF:
            break;
        case GE_SET_SCALE_PARAM:
            pscale_param = (pcgma_scale_param_t) uScaleParam;
            if (pscale_param == NULL)
            {
                LEAVE_GMA_API ();
                return RET_FAILURE;
            }
            if (pscale_param->h_mul == 0 || pscale_param->h_div == 0 || pscale_param->v_mul == 0 || pscale_param->v_div == 0)
            {
                LEAVE_GMA_API ();
                return RET_FAILURE;
            }
            break;
        case GE_SET_SCALE_MODE:
            m_config.bScaleFilterEnable = (uScaleParam == GE_SCALE_FILTER);
            break;
        default:
            break;
    }

    if (pscale_param)
    {
        gma_hal_SetMask (layer_id);
        gma_m36f_set_scale_param (priv, pscale_param);
        gma_hal_ClearMask (layer_id);
    }

    for (j = 0; j < GMA_REGION_MAX; j++)
    {
        region = &(priv->tRegion[j]);

        if (region->bValid == FALSE)
        {
            continue;
        }
        for (i = 0; i < MAX_BLOCK_IN_REGION; i++)
        {
            pBlock = region->pBlock[i];
            if (pBlock != NULL)
            {
                gma_m36f_set_head_rect (priv, &pBlock->block_head, &pBlock->tRect);
                gma_block_update (layer_id, pBlock);
            }
        }
    }

    gma_m36f_update_block_header(layer_id, TRUE);

    LEAVE_GMA_API ();
    return RET_SUCCESS;
}


RET_CODE gma_m36f_set_pallette (UINT32 layer_id, const UINT8 * puPallette, UINT16 uColorNum, const gma_pal_attr_t * pattr)
{
    UINT8 uType;
    UINT8 alpha_range;
    UINT8 rgb_order;
    UINT8 alpha = 0, R = 0, G = 0, B = 0;
    UINT8 *pdst;
    UINT32 i;
    struct gma_m36f_private *priv = NULL;
    RET_CODE ret;
    
    if (puPallette == NULL)
        return RET_FAILURE;

    if (pattr == NULL)
        pattr = &m_gma_pal_attr_default;

    priv = m_gma_priv[layer_id];
    uType = pattr->pal_type;
    alpha_range = pattr->alpha_range;
    rgb_order = pattr->rgb_order;

    ENTER_GMA_API();

    gma_m36f_malloc_pal (priv);

    gma_m36f_sync_block_header(priv, FALSE);

    gma_m36f_update_pal(layer_id, FALSE);   // invalidate the current pallete

    uColorNum <<= 2;

    pdst = (UINT8 *) priv->puPallette;

    for (i = 0; i < 1024; i += 4, puPallette += 4, pdst += 4)
    {
        if (i < uColorNum)
        {
            switch (rgb_order)
            {
                case GE_RGB_ORDER_ARGB:
                    alpha = puPallette[3];
                    R = puPallette[2];
                    G = puPallette[1];
                    B = puPallette[0];
                    break;
                case GE_RGB_ORDER_ABGR:
                    alpha = puPallette[3];
                    B = puPallette[2];
                    G = puPallette[1];
                    R = puPallette[0];
                    break;
                case GE_RGB_ORDER_RGBA:
                    R = puPallette[3];
                    G = puPallette[2];
                    B = puPallette[1];
                    alpha = puPallette[0];
                    break;
                case GE_RGB_ORDER_BGRA:
                    B = puPallette[3];
                    G = puPallette[2];
                    R = puPallette[1];
                    alpha = puPallette[0];
                    break;
            }
        }
        else
        {
            alpha = 0;
            if (uType == GE_PAL_YCBCR)
            {
                R = 0x10;
                B = 0x80;
                G = 0x80;       // Y Cb Cr
            }
            else
            {
                R = 0;
                B = 0;
                G = 0;
            }
        }

        if (priv->uPalFormatHW == GE_PAL_RGB)
        {
            pdst[3] = alpha;
            pdst[2] = R;
            pdst[1] = G;
            pdst[0] = B;
        }
        else
        {
            pdst[3] = alpha;
            pdst[0] = R;
            pdst[1] = G;
            pdst[2] = B;
        }
    }

    // Adjust RGB alpha range to 256
    if (priv->uPalFormatHW == GE_PAL_RGB && alpha_range != GE_ALPHA_RANGE_0_255)
    {
        pdst = (UINT8 *) priv->puPallette;

        for (i = 0; i < 1024; i += 4, pdst += 4)
        {
            alpha = pdst[3];
            if (alpha_range == GE_ALPHA_RANGE_0_127)
            {
                alpha <<= 1;
                alpha |= (pdst[3] >> 6);
            }
            else                // alpha range 16
            {
                alpha <<= 4;
                alpha |= pdst[3];
            }
            pdst[3] = alpha;
        }
    }

    priv->uPalFormatSW = uType;
    priv->uPalRGBOrder = rgb_order;
    priv->uPalAlphaRange = alpha_range;
    priv->uPalletteColor = 256;
    priv->bPalletteUpdated = FALSE;

    ret = gma_m36f_pal_YCbCr_to_RGB (layer_id);

#ifdef GMA_HEADER_REALTIME_UPDATE
    gma_m36f_update_block_header(layer_id, FALSE);
#endif

#ifdef _WINDOWS
    write_gma_pal_to_fpga();
#endif

    // Update block header in DE vblanking
    LEAVE_GMA_API();

    return ret;
}

RET_CODE gma_m36f_get_pallette (UINT32 layer_id, UINT8 * puPallette, UINT16 uColorNum, const gma_pal_attr_t * pattr)
{
    UINT8 uType;
    UINT8 alpha_range ;
    UINT8 rgb_order;

    UINT32 i;
    UINT8 alpha, R, G, B, temp;
    UINT8 *psrc, *pdst;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    if (priv->puPallette == NULL)
        return RET_FAILURE;

    if (pattr == NULL)
        pattr = &m_gma_pal_attr_default;

    uType = pattr->pal_type;
    alpha_range = pattr->alpha_range;
    rgb_order = pattr->rgb_order;

    if (uType == priv->uPalFormatHW)
        psrc = (UINT8 *) priv->puPalletteHW;
    else if (uType == priv->uPalFormatSW)
        psrc = (UINT8 *) priv->puPallette;
    else
        return RET_FAILURE;

    if (uColorNum > 256)
        uColorNum = 256;

    uColorNum <<= 2;

    pdst = (UINT8 *) puPallette;

    for (i = 0; i < uColorNum; i += 4, psrc += 4, pdst += 4)
    {
        alpha = psrc[3];
        R = psrc[2];
        G = psrc[1];
        B = psrc[0];

        if (priv->uPalFormatHW == GE_PAL_RGB)
        {
            // Adjust alpha range to user specified
            if (alpha_range == GE_ALPHA_RANGE_0_15)
            {
                alpha >>= 4;
            }
            else if (alpha_range == GE_ALPHA_RANGE_0_127)
            {
                alpha >>= 1;
            }
            switch (rgb_order)
            {
                case GE_RGB_ORDER_ARGB:
                    break;
                case GE_RGB_ORDER_ABGR:
                    // swap B <--> R
                    temp = B;
                    B = R;
                    R = temp;
                    break;
                case GE_RGB_ORDER_RGBA:
                    temp = alpha;
                    alpha = R;
                    R = G;
                    G = B;
                    B = alpha;
                    break;
                case GE_RGB_ORDER_BGRA:
                    temp = alpha;
                    alpha = B;
                    B = temp;

                    temp = R;
                    R = G;
                    G = temp;
                    break;
            }
        }
        pdst[3] = alpha;
        pdst[2] = R;
        pdst[1] = G;
        pdst[0] = B;
    }

    return RET_SUCCESS;
}

    RET_CODE gma_m36f_modify_pallette (UINT32 layer_id, UINT8 uIndex, UINT8 uK, UINT8 uY, UINT8 uCb, UINT8 uCr, const gma_pal_attr_t * pattr)
{
    RET_CODE ret;
    UINT8 pal_type;
    UINT8 alpha_range;
    UINT8 *pdst;
    UINT32 i;
    UINT8 alpha;
    UINT32 time_out = 5;
    
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    if (priv->puPallette == NULL)
        return RET_FAILURE;

    if (pattr == NULL)
        pattr = &m_gma_pal_attr_default;

    pal_type = pattr->pal_type;
    alpha_range = pattr->alpha_range;
    //UINT8 rgb_order = priv->uPalRGBOrder;

    if (pal_type != priv->uPalFormatSW)
        return RET_FAILURE;

    ENTER_GMA_API ();

    gma_m36f_sync_block_header(priv, FALSE);

    gma_m36f_update_pal(layer_id, FALSE); // invalidate the current pallette

    i = uIndex;
    i <<= 2;
    pdst = (UINT8 *) priv->puPallette + i;

    alpha = uK;

    // Adjust alpha range to 256
    if (priv->uPalFormatHW == GE_PAL_RGB && alpha_range != GE_ALPHA_RANGE_0_255)
    {
        if (alpha_range == GE_ALPHA_RANGE_0_127)
        {
            alpha <<= 1;
            alpha |= (uK >> 6);
        }
        else                    // alpha range 16
        {
            alpha <<= 4;
            alpha |= uK;
        }
    }

    
    if (priv->uPalFormatHW == GE_PAL_RGB)
    {
        if (priv->uPalFormatSW == GE_PAL_RGB)
        {
            pdst[3] = alpha;
            pdst[2] = uY;
            pdst[1] = uCb;
            pdst[0] = uCr;
            pdst = (UINT8 *) priv->puPalletteHW + i;
            pdst[3] = alpha;
            pdst[2] = uY;
            pdst[1] = uCb;
            pdst[0] = uCr;
            ret = RET_SUCCESS;
        }
        else
        {
            pdst[3] = alpha;
            pdst[2] = uY;
            pdst[1] = uCb;
            pdst[0] = uCr;
            ret = gma_m36f_pal_YCbCr_to_RGB (layer_id);
        }
        priv->bPalletteUpdated = FALSE;

#ifdef GMA_HEADER_REALTIME_UPDATE
        gma_m36f_update_block_header(layer_id, FALSE);
#endif

#ifdef _WINDOWS
        write_gma_pal_to_fpga();
#endif

        LEAVE_GMA_API ();
        return ret;
    }
    else
    {
        pdst[3] = alpha;
        pdst[0] = uY;
        pdst[1] = uCb;
        pdst[2] = uCr;
        pdst = (UINT8 *) priv->puPalletteHW + i;
        pdst[3] = alpha;
        pdst[0] = uY;
        pdst[1] = uCb;
        pdst[2] = uCr;
        ret = RET_SUCCESS;
    }

//for pallete modify
#if 0
    gma_hal_SetMask ();
    gma_hal_SetPalette (priv->uOSDLayer, uIndex, uY, uCb, uCr);
    gma_hal_ClearMask ();
#else

    // Update block header in DE vblanking
    priv->color_idx = uIndex;
    priv->color = (uY << 24) | (uCb << 16) | (uCr << 8) | uK;

    priv->bPalMdfUpdated = TRUE;

#ifdef GMA_HEADER_REALTIME_UPDATE
    gma_m36f_update_block_header(layer_id, FALSE);
#endif

#ifdef _WINDOWS
    write_gma_pal_to_fpga();
#endif

    while (TRUE == priv->bPalMdfUpdated)
    {
        if (0 == time_out)
            break;
        time_out--;
        osal_task_sleep (20);
    }
#endif

    LEAVE_GMA_API ();
    return RET_SUCCESS;

}

RET_CODE gma_m36f_show_onoff (UINT32 layer_id, BOOL uOnOff)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    ENTER_GMA_API ();

    priv->bOnOff = uOnOff;
    if (TRUE == uOnOff && !region_monitor (priv, REGION_CHECK))
    {
        LEAVE_GMA_API ();
        return RET_SUCCESS;
    }
    enable_display (priv->uOSDLayer, uOnOff);

    LEAVE_GMA_API ();
    return RET_SUCCESS;
}

 RET_CODE gma_m36f_ioctl (UINT32 layer_id, UINT32 dwCmd, UINT32 dwParam)
{
    RET_CODE ret = RET_SUCCESS;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    UINT32 *pOnOff;
    gma_rect_t *posd_rect;
    gma_region_t *region;
    int region_pending =0;
    int i,j = 0;

    if (layer_id >= GMA_M36F_LAYER_NUM || priv == NULL)
        return RET_FAILURE;

    switch (dwCmd)
    {
        case GE_IO_UPDATE_PALLETTE:
            {
#ifndef GMA_HEADER_REALTIME_UPDATE
                ENTER_GMA_API ();
                gma_m36f_update_block_header(layer_id, FALSE);
                LEAVE_GMA_API ();
#endif
                break;
            }
#if 0
        case GE_IO_SET_VFILTER:
            {
                if (m_gma_chip_id != ALI_M3202)
                    break;
                gma_hal_SetMask ();
                UINT32 uReg = gma_ReadDword (M32_GMA_FILTER[priv->uOSDLayer]);
                uReg = (uReg & 0xFFFF00FF) | (dwParam & 0xFF00);
                gma_hal_VFilterOnOff (priv->uOSDLayer, dwParam ? 1 : 0);
                gma_hal_VFilter (priv->uOSDLayer, uReg);
                gma_hal_ClearMask ();
                m_config.bScaleFilterEnable = dwParam ? 1 : 0;
                break;
            }
#endif
        case GE_IO_SET_TRANS_COLOR:
            {
                priv->trans_color = dwParam;
                break;
            }
#if 0
        case GMA_IO_SET_ANTI_FLICK_THRE:
            {
                UINT8 max_k = 0;

                max_k = (UINT8) dwParam;
                if (max_k > 15)
                    max_k = 15;
                // set the anti-flick threshold
                //max_k = osd_m33_get_max_k();
                gma_hal_SetMask ();
                gma_WriteDword (0xC0, (UINT32) max_k);
                gma_hal_ClearMask ();
                break;
            }
#endif
        case GE_IO_ENABLE_ANTIFLICK:
            gma_hal_SetMask (priv->uOSDLayer);
            gma_hal_anti_flicker_onoff (priv->uOSDLayer, dwParam);
            gma_hal_ClearMask (priv->uOSDLayer);
            break;
        case GE_IO_SET_GLOBAL_ALPHA:
            dwParam &= 0xff;
            gma_hal_SetMask (priv->uOSDLayer);
            gma_hal_GlobalAlpha (priv->uOSDLayer, dwParam);
            gma_hal_ClearMask (priv->uOSDLayer);
            break;
        case GE_IO_GET_LAYER_ON_OFF:
            pOnOff = (UINT32 *) dwParam;
            if (pOnOff == NULL)
            {
                ret = RET_FAILURE;
                break;
            }
            *pOnOff = priv->bOnOff;
            break;
        case GE_IO_SET_AUTO_CLEAR_REGION:
            priv->auto_clear_region = dwParam;
            break;
        case GE_IO_GET_DISPLAY_RECT:
            posd_rect = (gma_rect_t *)dwParam;
            ret = gma_m36f_get_display_rect(layer_id, 0, (gma_rect_t *)dwParam);
            break;
        case GE_IO_SET_DISPLAY_RECT:
            posd_rect = (gma_rect_t *)dwParam;
            ret = gma_m36f_set_display_rect(layer_id, 0, (gma_rect_t *)dwParam);
            break;
        case GE_IO_SET_ENHANCE_PAR:
		{	
			struct gma_enhance_pars *penhance_pars
				= (struct gma_enhance_pars *)dwParam;

			if(penhance_pars->enhance_grade > 100)
				penhance_pars->enhance_grade = 100;
			else if(penhance_pars->enhance_grade < 0)
				penhance_pars->enhance_grade = 0;	

			GMA_PRINTF("flag %d grade %d\n", penhance_pars->enhance_flag, penhance_pars->enhance_grade);
			
			region_pending = 1;
			switch(penhance_pars->enhance_flag)
			{
				case GMA_ENHANCE_BRIGHTNESS:					
					priv->brightness_grade = penhance_pars->enhance_grade;
					break;
				case GMA_ENHANCE_CONTRAST:					
					priv->contrast_grade = penhance_pars->enhance_grade;
					break;
				case GMA_ENHANCE_SATURATION:
					priv->saturation_grade = penhance_pars->enhance_grade;
					break;
				case GMA_ENHANCE_SHARPNESS:				
					priv->sharpness_grade = penhance_pars->enhance_grade;
					break;
				case GMA_ENHANCE_HUE:
					priv->hue_grade = penhance_pars->enhance_grade;
					break;
				default:
					region_pending = 0;
					break;
			}

			if(region_pending == 1)
			{
			    gma_m36f_enhance_coef_gen((unsigned int *)priv->enhance_coef_table, 
                priv->brightness_grade, priv->contrast_grade, priv->saturation_grade, 
                priv->hue_grade, priv->sharpness_grade);	
                for (j = 0; j < GMA_REGION_MAX; j++)
                {
                    region = &(priv->tRegion[j]);

                    if (region->bValid == FALSE)
                    {
                        continue;
                    }
                    for (i = 0; i < MAX_BLOCK_IN_REGION; i++)
                    {
                        struct gma_block_t *pBlock = region->pBlock[i];

                        if (pBlock != NULL)
                        {
                            gma_m36f_set_head_rect (priv, &pBlock->block_head, &pBlock->tRect);
                            gma_block_update (layer_id, pBlock);
                        }
                    }
                }

			}
        }	
	        break;
        default:
            break;
    }

    return ret;
}

RET_CODE gma_m36f_open (UINT32 layer_id)
{
    UINT32 i;
    UINT8 pal[1024];
    struct gma_m36f_private *priv = m_gma_priv[layer_id];
    alifb_gma_region_t *pregion_info;
    gma_layer_config_t *pconfig;
    
    for (i = 0; i < GMA_REGION_MAX; i++)
        priv->tRegion[i].bValid = FALSE;

    priv->uBlockInitialized = 0;
    priv->first_block = NULL;
    priv->first_block_changed = 0;
    priv->new_header_id = 0;
    priv->header_need_sync = 0;

    priv->eClipMode = GE_CLIP_DISABLE;
    priv->auto_clear_region = FALSE;
    priv->bPalletteUpdated = TRUE;

    pregion_info = &priv->region_info;
    pconfig = &priv->layer_config;

    pregion_info->bitmap_addr = 0;
    pregion_info->galpha_enable = 0;
    pregion_info->global_alpha = 0xff;
    pregion_info->pallette_sel = 0;

    priv->filter_select = 0;
	priv->brightness_grade = 50;
	priv->contrast_grade = 50;
	priv->saturation_grade = 50;
	priv->hue_grade = 50;
	priv->sharpness_grade = 50;
    
	gma_m36f_enhance_coef_gen((int *)priv->enhance_coef_table
		, priv->brightness_grade, priv->contrast_grade, priv->saturation_grade
		, priv->hue_grade, priv->sharpness_grade);
	set_gma_csc_bypass(layer_id, 0);
	set_gma_enhance_sharpness(layer_id, 1);
	gma_hal_GlobalAlpha(layer_id, 0xFF);
	if(layer_id == 0)
	{
		if(pconfig->color_format <= GE_GMA_PF_RGB565)
		set_gma_line_buf_par(0x01);
	}
    
    if((pconfig->color_format <= GE_GMA_PF_ARGB4444) && (0 == layer_id ) 
       && ((ALI_S3811 == m_gma_chip_id)||(ALI_C3701 == m_gma_chip_id))) // enable one line buffer
    {
            gma_WriteDword (VPOST_GMA_LINEBUFFER_CTR, 0x1);
    }
    pregion_info->color_format = ge_m36f_get_hw_pf ((enum GMA_PIXEL_FORMAT)pconfig->color_format);
    pregion_info->pixel_pitch = pconfig->pixel_pitch;
    pregion_info->bitmap_w = pconfig->width;
    pregion_info->bitmap_h = pconfig->height;

    if (0xFF == pregion_info->color_format)
    {
        GMA_ERR_PRINTF ("%s : Color format error\n", __FUNCTION__);
        return RET_FAILURE;
    }

    if (gma_is_clut8 (pregion_info->color_format))
        priv->trans_color = GMA_TRANSPARENT_COLOR;
    else
        priv->trans_color = 0;

    priv->screen_width = get_screen_pitch (pregion_info->pixel_pitch);
    pregion_info->bitmap_x = (priv->screen_width - pregion_info->bitmap_w) / 2;
    pregion_info->bitmap_y = (get_screen_height (priv->screen_width) - pregion_info->bitmap_h) / 2;
    pregion_info->region_x = pregion_info->bitmap_x;
    pregion_info->region_y = pregion_info->bitmap_y;
    pregion_info->region_w = pregion_info->bitmap_w;
    pregion_info->region_h = pregion_info->bitmap_h;

    gma_m36f_set_scale_param (priv, &m_gma_scale_off);
    gma_m36f_init_block ();
    gma_m36f_show_onoff (layer_id, FALSE);
   
    MEMSET(pal, 0, 1024);
    gma_m36f_set_pallette(layer_id, pal, 256, NULL);

    //priv->ge_cmd_list = ge_m36f_cmd_list_create (m_ge_dev, 1);
    return RET_SUCCESS;
}

RET_CODE gma_m36f_close (UINT32 layer_id)
{
    UINT32 i, j;
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    for (j = 0; j < GMA_REGION_MAX; j++)
    {
        gma_region_t *region = &(priv->tRegion[j]);

        if (region->bValid == FALSE)
        {
            GMA_PRINTF ("ERROR : region NOT created!\n");
            continue;
        }

        for (i = 0; i < MAX_BLOCK_IN_REGION; i++)
        {
            if (region->pBlock[i] != NULL)
            {
                if (region->bUseMemSlice)
                    free_me (region->pBlock[i]->puBuf, region->pBlock[i]->uSize);
                gma_block_delete (layer_id, region->pBlock[i]);
                region->pBlock[i] = NULL;
            }
        }
        region->bValid = FALSE;
    }

    gma_m36f_free_pal (priv);
    //ge_m36f_cmd_list_destroy (m_ge_dev, priv->ge_cmd_list);
    //priv->ge_cmd_list = 0;	
    return RET_SUCCESS;
}

void gma_m36f_attach (UINT32 layer_id, const gma_layer_config_t * attach_config)
{
    struct gma_m36f_private *priv;
    void *priv_mem;

    m_gma_chip_id = ali_sys_ic_get_chip_id ();
    if (m_gma_chip_id == ALI_S3602F)
        m_gma_chip_id = ALI_S3602;
    //m_ge_dev = ge_dev;

    /* Alloc structure space of private */
    priv_mem = (void *) kmalloc (sizeof (struct gma_m36f_private),GFP_KERNEL);
    if (NULL == priv_mem)
    {
        GMA_ERR_PRINTF ("Alloc osd device private memory error!/n");
        return;
    }

    MEMSET (priv_mem, 0, sizeof (struct gma_m36f_private));
    priv = (struct gma_m36f_private *) priv_mem;

    /*Global variables initialization, specific features */
    if (NULL == attach_config)
    {
        ASSERT (0);
    }
    else
    {
        MEMCPY (&priv->layer_config, attach_config, sizeof (gma_layer_config_t));
    }

    if (attach_config->hw_layer_id == GMA_HW_LAYER0)
    {
        priv->uOSDLayer = GMA_HW_LAYER0;
        gp_gma_m36f_private0 = priv; // just for debug
    }
    else if (attach_config->hw_layer_id == GMA_HW_LAYER1)
    {
        priv->uOSDLayer = GMA_HW_LAYER1;
        gp_gma_m36f_private1 = priv; // just for debug
    }
    else
    {
        return;
    }
    m_gma_priv[layer_id] = priv;

    if (ALI_C3701 != m_gma_chip_id && ALI_S3602 != m_gma_chip_id && ALI_S3503 != m_gma_chip_id)
    {
        priv->uPalFormatHW = GE_PAL_YCBCR;
        if ((ALI_M3329E == m_gma_chip_id))
        {
            m_gma_chip_id = ALI_M3202;
            g_gma_reg_base_addr = M3202C_GMA_REG_BASE;
        }
        else
        {
            ASSERT (0);
        }
    }
    else
    {
        //m_gma_chip_id = ALI_S3602;
        priv->uPalFormatHW = GE_PAL_RGB;
        g_gma_reg_base_addr = S3602F_GMA_REG_BASE;
    }

    //if (m_osd_layer_attach_flag == 0)
        mutex_init(&m_gma_m36f_mutex);

    //m_osd_layer_attach_flag |= 1 << priv->uOSDLayer;
    return;
}

RET_CODE gma_m36f_detach (UINT32 layer_id)
{
    struct gma_m36f_private *priv = m_gma_priv[layer_id];

    //m_osd_layer_attach_flag &= ~(1 << priv->uOSDLayer);

    FREE (priv);
    priv = NULL;

    //m_block_inited = FALSE;
    //m_last_buffer = 0;

    //if (m_osd_layer_attach_flag == 0)
    //    osal_mutex_delete (m_gma_m36f_mutex);

    return RET_SUCCESS;
}
#endif

