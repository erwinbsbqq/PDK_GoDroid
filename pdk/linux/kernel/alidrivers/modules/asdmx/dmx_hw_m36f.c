#include "dmx_hw.h"
#include "dmx_internal.h"

#define M36F_DMX_HW_TOTAL_FILTERS              48

/* Counted in bytes.
 */
#define DMX_FILTER_HW_MASK_LEN             4 

enum dmx_m33_reg_offset
{
    /* DEMUX control register */
    OFFSET_GLOBAL_CONTROL               = 0x00,    

    /* Filter Fifo Control Register */  
    OFFSET_FILTER_FIFO_CONTROL          = 0x10,

    /* DMA FIFO 0 control register */          
    OFFSET_DMA_FIFO_0_CONTROL           = 0x14,    

    /* DMA FIFO 1 control register */   
    OFFSET_DMA_FIFO_1_CONTROL           = 0x18,

    /* PCR content Register0*/      
    OFFSET_PCR_CONTENT_0                = 0x30,    
    /* PCR content Register1*/  
    OFFSET_PCR_CONTENT_1                = 0x34,

    /* Channel mask registers */     
    OFFSET_FILTER_EN_MASK_0             = 0x40,   
    OFFSET_FILTER_EN_MASK_1             = 0x44,   

    /* Channel de-scrambler mask register */      
    OFFSET_FILTER_DESCRAM_EN_MASK_0     = 0x50,
    OFFSET_FILTER_DESCRAM_EN_MASK_1     = 0x54,
 
    /* Filter mask & value & CW control register */      
    OFFSET_M_V_C_PORT_CONTROL           = 0x60,    
    /* Filter mask & value & CW data register 0 */  
    OFFSET_M_V_C_PORT_DATA_0            = 0x64,
    /* Filter mask & value & CW data register 1 */      
    OFFSET_M_V_C_PORT_DATA_1            = 0x68,    
    /* Filter mask & value & CW data register 2 */  
    OFFSET_M_V_C_PORT_DATA_2            = 0x6C,
    /* Filter mask & value & CW data register 3 */      
    OFFSET_M_V_C_PORT_DATA_3            = 0x70, 
   
    /* TODO: channel CW select registers. */

    /* Default CW data register 0 */    
    OFFSET_DEFULT_CW_DATA_0             = 0xA0,    
    /* Default CW data register 1 */    
    OFFSET_DEFULT_CW_DATA_1             = 0xA4,

    /* DMA segment start address register */    
    OFFSET_TS_BUF_BASE                  = 0xB0,    

    /* DMA channel 0 start address register */  
    OFFSET_TS_BUF_START_OFFSET          = 0xC0,    

    /* DMA channel 0 buffer length & Interrupt threshold register */    
    OFFSET_TS_BUF_LEN_THRESHOLD         = 0x140,
    /* DMA channel 0 write & read point regiter */      
    OFFSET_TS_BUF_W_R_PTR               = 0x1C0,      
    /* DMA threshold interrupt mask register */    
    OFFSET_TS_BUF_THRESHOLD_INT_EN_MASK = 0x300,    
    /* DMA threshold interrupt event register */      
    OFFSET_TS_BUF_THRESHOLD_INT         = 0x310,       
    /* DMA overlap interrupt mask register */
    OFFSET_TS_BUF_OVERFLOW_INT_EN_MASK  = 0x320,   
    /* DMA overlap interrupt event register */  
    TS_BUF_OVERFLOW                     = 0x330,   
    
    /* DMA FIFO overflow interrupt mask register */
    OFFSET_DMA_OVERFLOW_INT_EN_MASK     = 0x340,       

    /* DMA FIFO overflow interrupt event register */
    DMA_OVERFLOW                        = 0x344, 

    /* PCR detect enable register. */ 
    PCR_DETECT_EN                       = 0x350,
    /* PCR hit register. */
    PCR_HIT                             = 0x354     
};


enum dmx_mvc_type
{
    DMX_MVC_TYPE_MASK  = 0x00,
    DMX_MVC_TYPE_VALUE = 0x40,
    DMX_MVC_TYPE_CW    = 0x80
};

//#define   dmx_assert(level, warning, ...)    dmx_assert(level, warning, ...)


#define DMX_HW_TS_BUF_GET_RD_PTR(base_addr) \
        DMX_GET_WORD(base_addr + OFFSET_TS_BUF_W_R_PTR + 2) & 0x1FFF

#define DMX_HW_TS_BUF_SET_RD_PTR(base_addr, data) \
        DMX_SET_WORD((base_addr + OFFSET_TS_BUF_W_R_PTR + 2), (__u16)data)

#define DMX_HW_TS_BUF_GET_WR_PTR(base_addr) \
        DMX_GET_WORD(base_addr + OFFSET_TS_BUF_W_R_PTR) & 0x1FFF
         
#define DMX_HW_INT_IS_PCR(base_addr) \
        DMX_GET_DWORD(base_addr + PCR_HIT) & 0x1

#define DMX_HW_INT_IS_DMA_FIFO_OVERFLOW(base_addr) \
        DMX_GET_DWORD(base_addr + DMA_OVERFLOW) & 0x3

#define DMX_HW_INT_IS_TS_BUF_OVERFLOW(base_addr) \
        DMX_GET_DWORD(base_addr + TS_BUF_OVERFLOW) & 0x1

#define DMX_HW_PCR_GET_VALUE(base_addr) \
        DMX_GET_DWORD(base_addr + OFFSET_PCR_CONTENT_0)


#define DMX_HW_PCR_SET_FLT(base_addr, flt_idx) \
        DMX_SET_BYTE(base_addr + OFFSET_PCR_CONTENT_1 + 2, flt_idx)

#define DMX_HW_PCR_INT_ENABLE(base_addr) \
        DMX_SET_BYTE(base_addr + PCR_DETECT_EN, 0x1)

#define DMX_HW_PCR_INT_DISABLE(base_addr) \
        DMX_SET_BYTE(base_addr + PCR_DETECT_EN, 0x0)

#define DMX_HW_PCR_DETECT_ENABLE(base_addr) \
        DMX_SET_BYTE(base_addr + OFFSET_GLOBAL_CONTROL, \
        DMX_GET_BYTE(base_addr + OFFSET_GLOBAL_CONTROL) | 0x80)

#define DMX_HW_PCR_DETECT_DISABLE(base_addr) \
        DMX_SET_BYTE(base_addr + OFFSET_GLOBAL_CONTROL, \
        DMX_GET_BYTE(base_addr + OFFSET_GLOBAL_CONTROL) & (~0x80))

__s32 dmx_hw_mvc_port_set_data
(
    __u32              base_addr,
    __u8               flt_idx,
    enum dmx_mvc_type  mvc_type,
    __u32             *data
)
{
    __u8  *byte_ptr;
    __u32  port_addr;
    __u32  i;

    byte_ptr = (__u8 *)data;
    port_addr = base_addr + OFFSET_M_V_C_PORT_DATA_0;

    for (i = 0; i < 4; i++) 
    {
    	DMX_SET_BYTE(port_addr++, byte_ptr[3]);
    	DMX_SET_BYTE(port_addr++, byte_ptr[2]);
    	DMX_SET_BYTE(port_addr++, byte_ptr[1]);
    	DMX_SET_BYTE(port_addr++, byte_ptr[0]);

        byte_ptr += 4;
    }

    /* Tell dmx we want operate on one specified hw mvc_type and filter. */
    DMX_SET_BYTE(base_addr + OFFSET_M_V_C_PORT_CONTROL, mvc_type | flt_idx);

    /* Tell dmx we want write operation. */
    DMX_SET_BYTE(base_addr + OFFSET_M_V_C_PORT_CONTROL + 2, 0x2);

    /* Wait until dmx hw write is done. */
    for(i = 0; i < 20000; i++)
    {
        if(0 == (DMX_GET_BYTE(base_addr + OFFSET_M_V_C_PORT_CONTROL + 2) & 0x2))
        {
            return(RET_SUCCESS);
        }
    }

    return(RET_FAILURE);
}


__s32 m36f_dmx_hw_flt_pid_set
(
    __u32 base_addr,
    __u8  flt_idx,
    __u16 pid
)
{
    __u32 mvc_port_data[4];

    if (flt_idx >= M36F_DMX_HW_TOTAL_FILTERS)
    {
        //printk("Illegal flt idx");

        return(RET_FAILURE);
    }

    mvc_port_data[0] = 0x1FFF0000;
    mvc_port_data[1] = 0x00000000;
    mvc_port_data[2] = 0x00000000;
    mvc_port_data[3] = 0x00000000;

    dmx_hw_mvc_port_set_data(base_addr, flt_idx, DMX_MVC_TYPE_MASK,
                             mvc_port_data);

    mvc_port_data[0] = pid << 16;
    mvc_port_data[1] = 0x00000000;
    mvc_port_data[2] = 0x00000000;
    mvc_port_data[3] = 0x00000000;

    dmx_hw_mvc_port_set_data(base_addr, flt_idx, DMX_MVC_TYPE_VALUE,
                             mvc_port_data);

    return(RET_SUCCESS);
}



__s32 m36f_dmx_hw_flt_enable
(
    __u32 base_addr,
    __u8  flt_idx
)
{
    __u32 en_mask;
    __u32 this_flt_mask;
    __u32 filt_en_reg_offset;

    if (flt_idx >= M36F_DMX_HW_TOTAL_FILTERS)
    {
        return(RET_FAILURE);
    }

    if (flt_idx < 32)
    {
         this_flt_mask = (1 << flt_idx);

        filt_en_reg_offset = OFFSET_FILTER_EN_MASK_0;
    }
    else
    {
        this_flt_mask = (1 << (flt_idx - 32));

        filt_en_reg_offset = OFFSET_FILTER_EN_MASK_1;
    }
    
    en_mask = DMX_GET_DWORD(base_addr + filt_en_reg_offset);

    /* Channel already enabled. */
    if (0 != (en_mask & this_flt_mask))
    {
        return(RET_SUCCESS);
    }

    en_mask |= this_flt_mask;

    DMX_SET_DWORD(base_addr + filt_en_reg_offset, en_mask);

    return(RET_SUCCESS);
}


__s32 m36f_dmx_hw_flt_disable
(
    __u32 base_addr,
    __u8  flt_idx
)
{
    __u32 en_mask;
    __u32 this_flt_mask;
    __u32 filt_en_reg_offset;

    if (flt_idx >= M36F_DMX_HW_TOTAL_FILTERS)
    {
        return(RET_FAILURE);
    }

    if (flt_idx < 32)
    {
        this_flt_mask = (1 << flt_idx);

        filt_en_reg_offset = OFFSET_FILTER_EN_MASK_0;
    }
    else
    {
        this_flt_mask = (1 << (flt_idx - 32));

        filt_en_reg_offset = OFFSET_FILTER_EN_MASK_1;
    }

    en_mask = DMX_GET_DWORD(base_addr + filt_en_reg_offset);

    /* Channel already disabled. */
    if (0 == (en_mask & this_flt_mask))
    {
        return(RET_SUCCESS);
    }

    en_mask &= (~this_flt_mask);

    DMX_SET_DWORD(base_addr + filt_en_reg_offset, en_mask);

    return(RET_SUCCESS);
}



__u32 m36f_dmx_hw_buf_rd_get
(
    __u32 dmx_base
)
{
    return(DMX_HW_TS_BUF_GET_RD_PTR(dmx_base));
}

__u32 m36f_dmx_hw_buf_rd_set
(
    __u32 dmx_base,
    __u32 rd
)
{
    //return(DMX_HW_TS_BUF_SET_RD_PTR(dmx_base, rd));
    DMX_SET_WORD((dmx_base + OFFSET_TS_BUF_W_R_PTR + 2), (__u16)rd);

    return(0);
}


__u32 m36f_dmx_hw_buf_wr_get
(
    __u32 dmx_base
)
{
    return(DMX_HW_TS_BUF_GET_WR_PTR(dmx_base));
}


__s32 m36f_dmx_hw_pcr_get
(
    __u32  dmx_base,
    __u32 *pcr
)
{
    __u32 pcr_int;

    /* Handle pcr int. */
    pcr_int = DMX_HW_INT_IS_PCR(dmx_base);

    if (0 == pcr_int)
    {
        return(-1);
    }

    *pcr = DMX_HW_PCR_GET_VALUE(dmx_base);

    DMX_SET_BYTE(dmx_base + PCR_HIT, 0x1);
    //stc_valid();

    //set_stc(pcr, 0);

    return(0);
}

__s32 m36f_dmx_hw_fifo_overflow_get
(
    __u32 dmx_base
)
{
    __u32 fifo_overflow;

    /* Handle dma fifo overflow int. */
    fifo_overflow = DMX_HW_INT_IS_DMA_FIFO_OVERFLOW(dmx_base);

    return(fifo_overflow);
}


__s32 m36f_dmx_hw_reg_reset
(
    __u32 base_addr,
    __u32 ts_buf_addr,
    __u32 ts_buf_pkt_cnt
)
{
    __u32 i;

    /* Rest dmx hw. */
    DMX_SET_BYTE(base_addr + OFFSET_GLOBAL_CONTROL + 3, 0x01);

    /* Wait 16+ clocks. */
    for (i = 0; i < 100; i++);

    /* Init DMA fifo control regs. */
    DMX_SET_WORD(base_addr + OFFSET_DMA_FIFO_0_CONTROL + 2, 0xC);

    DMX_SET_WORD(base_addr + OFFSET_DMA_FIFO_1_CONTROL + 2, 0xC);

    /* Init TS buffer control regs. */
    /* Use united buffer mode. */
    DMX_SET_WORD(base_addr + OFFSET_TS_BUF_BASE + 2, 1);

    /* TS buffer 0 start address. */
    DMX_SET_BYTE(base_addr + OFFSET_TS_BUF_BASE, ts_buf_addr >> 23);

    DMX_SET_DWORD(base_addr + OFFSET_TS_BUF_START_OFFSET, ts_buf_addr);

    /* Set TS buffer 0 len. */
    DMX_SET_WORD(base_addr + OFFSET_TS_BUF_LEN_THRESHOLD, (__u16)ts_buf_pkt_cnt);

    /* Threthold int is not used. */

    /* Enable TS buffer 0 overflow interrupt. */
    //DMX_SET_DWORD(base_addr + OFFSET_TS_BUF_OVERFLOW_INT_EN_MASK, 0x1);

    /* Enable dma fifo 0 & 1 overflow interrupt. */
    DMX_SET_BYTE(base_addr + OFFSET_DMA_OVERFLOW_INT_EN_MASK, 0x3);

    /* Init global congtrol reg. */
    /* external sync mode, PCR disabled, input enabled, bypass CSA, 
     * drop erro pkt, input clk polar inverse of standard.
     */
    //DMX_SET_DWORD(base_addr + OFFSET_GLOBAL_CONTROL, 0x44722);
    DMX_SET_DWORD(base_addr + OFFSET_GLOBAL_CONTROL, 0xC4722);

    return(0);
}


__s32 m36f_dmx_hw_pcr_set_flt(base_addr, flt_idx)
{
    DMX_HW_PCR_SET_FLT(base_addr, flt_idx);
    return(0);
}    
__s32 m36f_dmx_hw_pcr_int_enable(base_addr)
{  
    DMX_HW_PCR_INT_ENABLE(base_addr);
    return(0);
}     
      
__s32 m36f_dmx_hw_pcr_int_disable(base_addr)
{  
    DMX_HW_PCR_INT_DISABLE(base_addr);
    return(0);
}     
      
__s32 m36f_dmx_hw_pcr_detect_enable(base_addr)
{  
    DMX_HW_PCR_DETECT_ENABLE(base_addr);
    return(0);
}     
      
__s32 m36f_dmx_hw_pcr_detect_disable(base_addr)
{
    DMX_HW_PCR_DETECT_DISABLE(base_addr);
    return(0);
}

__s32 m36f_dmx_hw_set_bypass_mode(__u32 base_addr)
{
    
    DMX_SET_DWORD(base_addr + OFFSET_TS_BUF_OVERFLOW_INT_EN_MASK, 0x0);
    DMX_SET_DWORD(base_addr + OFFSET_GLOBAL_CONTROL, 0x04726);
    
    return(0);
}

__s32 m36f_dmx_hw_clear_bypass_mode(__u32 base_addr)
{
    __u32 regAddr = base_addr + OFFSET_GLOBAL_CONTROL;
    
    DMX_SET_BYTE(regAddr, DMX_GET_BYTE(regAddr)&(~0x04));  
    return(0);
}

irqreturn_t m36f_dmx_hw_isr
(
    int   irq,
    void *dev
)
{
    struct dmx_device *dmx; 
	__u32              ts_buf_overflow;
    __u32              ts_fifo_overflow;
    __u32              wr_idx;
    __u32              rd_idx;
    __s32              ret=0;
    __u32              pcr=0;

#if 1

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    dmx = (struct dmx_device *)dev;
    /* Handle pcr int.
     */
    ret = m36f_dmx_hw_pcr_get(dmx->base_addr, &pcr);

    /* Got PCR vaulue from HW. 
     */
    if (0 == ret)
    {
        printk("%s,%d,pcr:%x\n", __FUNCTION__, __LINE__, pcr);

        /* TODO: PCR proccess. 
         */
    }


    /* Handle ts buffer overflow int.
     */
    ts_buf_overflow = DMX_GET_BYTE(dmx->base_addr + TS_BUF_OVERFLOW);

    if (0 != ts_buf_overflow)
    {
        wr_idx = m36f_dmx_hw_buf_wr_get(dmx->base_addr);

        rd_idx = m36f_dmx_hw_buf_rd_get(dmx->base_addr);

        printk("%s,%s,rd:%d,wr:%d,last parsed pid:%d\n",
               __FUNCTION__, dmx->name, rd_idx, wr_idx, dmx->last_parsed_pid);

        DMX_SET_BYTE(dmx->base_addr + TS_BUF_OVERFLOW, 0x1);
    }

    ts_fifo_overflow = DMX_GET_BYTE(dmx->base_addr + DMA_OVERFLOW);

    if (0 != ts_fifo_overflow)
    {
        DMX_SET_BYTE(dmx->base_addr + DMA_OVERFLOW, 0x3);

        printk("%s,%s,rd:%d,wr:%d,last parsed pid:%d\n",
               __FUNCTION__, dmx->name, rd_idx, wr_idx, dmx->last_parsed_pid);

        /* TODO: HW fifo overflow proccess. 
         */
    }
#endif

	return(IRQ_HANDLED);
}
