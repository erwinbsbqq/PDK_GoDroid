#include "dmx_hw.h"

#include "dmx_internal.h"


//#define DMX_HW_TOTAL_FILTERS              96


#define DMX_BIT_CLK_POLAR_START_IDX 0
#define DMX_BIT_CLK_POLAR_END_IDX   0

#define DMX_BIT_ERR_DROP_START_IDX 1
#define DMX_BIT_ERR_DROP_END_IDX   1

#define DMX_BIT_BYPASS_START_IDX 2
#define DMX_BIT_BYPASS_END_IDX   2

#define DMX_BIT_TS_IN_EN_START_IDX 5
#define DMX_BIT_TS_IN_EN_END_IDX   5

#define DMX_BIT_PCR_EN_START_IDX 7
#define DMX_BIT_PCR_EN_END_IDX   7


#define DMX_BIT_SYNC_BYTE_START_IDX 8
#define DMX_BIT_SYNC_BYTE_END_IDX   15

#define DMX_BIT_SYNC_MODE_START_IDX 16
#define DMX_BIT_SYNC_MODE_END_IDX   16

#define DMX_BIT_SYNC_TIME_START_IDX 17
#define DMX_BIT_SYNC_TIME_END_IDX   18

#define DMX_BIT_RESET_START_IDX 24
#define DMX_BIT_RESET_END_IDX   24

#define DMX_BIT_EXCLUDE_MODE_START_IDX 28
#define DMX_BIT_EXCLUDE_MODE_END_IDX   28


#define DMX_BIT_PCR_EN_START_IDX 7
#define DMX_BIT_PCR_EN_END_IDX   7


#define DMX_BIT_PCR_DISCON_INDI_START_IDX 8
#define DMX_BIT_PCR_DISCON_INDI_END_IDX   8


#define DMX_BIT_PCR_HW_FLT_IDX_START_IDX 16
#define DMX_BIT_PCR_HW_FLT_IDX_END_IDX   22



#define DMX_BIT_PCR_VALUE_START_IDX 0
#define DMX_BIT_PCR_VALUE_END_IDX   31

#define DMX_BIT_PCR_INT_EN_START_IDX 0
#define DMX_BIT_PCR_INT_EN_END_IDX   0

#define DMX_BIT_PCR_INT_EVENT_START_IDX 0
#define DMX_BIT_PCR_INT_EVENT_END_IDX   0


#define DMX_BIT_LANCY_START_IDX 0
#define DMX_BIT_LANCY_END_IDX   0


#define DMX_BIT_HW_BUF_ADDR_START_IDX 0
#define DMX_BIT_HW_BUF_ADDR_END_IDX   31

#define DMX_BIT_HW_BUF_LEN_START_IDX 0
#define DMX_BIT_HW_BUF_LEN_END_IDX 15

#define DMX_BIT_HW_BUF_THRESHOLD_START_IDX 16
#define DMX_BIT_HW_BUF_THRESHOLD_END_IDX 31

#define DMX_BIT_HW_BUF_THRESHOLD_INT_EN_START_IDX 0
#define DMX_BIT_HW_BUF_THRESHOLD_INT_EN_END_IDX 0

#define DMX_BIT_HW_BUF_THRESHOLD_INT_EVENT_START_IDX 0
#define DMX_BIT_HW_BUF_THRESHOLD_INT_EVENT_END_IDX 0

#define DMX_REG_HW_BUF_FULL_INT_EN_START_IDX 0
#define DMX_REG_HW_BUF_FULL_INT_EN_END_IDX 0

#define DMX_REG_HW_BUF_FULL_INT_EVENT_START_IDX 0
#define DMX_REG_HW_BUF_FULL_INT_EVENT_END_IDX 0


#define DMX_BIT_PID_FLT_VALUE_START_IDX 0
#define DMX_BIT_PID_FLT_VALUE_END_IDX 15

#define DMX_BIT_PID_FLT_SELECT_START_IDX 0
#define DMX_BIT_PID_FLT_SELECT_END_IDX 6


#define DMX_BIT_PID_FLT_RD_START_IDX 16
#define DMX_BIT_PID_FLT_RD_END_IDX 16

#define DMX_BIT_PID_FLT_WR_START_IDX 17
#define DMX_BIT_PID_FLT_WR_END_IDX 17





enum M3701C_DMX_REG_OFFSET
{
    DMX_REG_CTRL                       = 0x00,
    DMX_REG_PCR_VALUE                  = 0x30,
    DMX_REG_PCR_CFG                    = 0x34,
    DMX_REG_PID_FLT_EN                 = 0x40,
    DMX_REG_PID_FLT_CFG                = 0x60,
    DMX_REG_PID_FLT_VALUE              = 0x64,
    DMX_REG_HW_BUF_ADDR                = 0xC0,
    DMX_REG_HW_THRESHOLD_AND_BUF_LEN   = 0x140,
    DMX_REG_HW_WR_IDX                  = 0x1C0,
    DMX_REG_HW_RD_IDX                  = 0x1C2,
    DMX_REG_HW_BUF_THRESHOLD_INT_EN    = 0x300,
    DMX_REG_HW_BUF_THRESHOLD_INT_EVENT = 0x310,
    DMX_REG_HW_BUF_FULL_INT_EN         = 0x320,
    DMX_REG_HW_BUF_FULL_INT_EVNET      = 0x330,
    DMX_REG_HW_FIFO_FULL_INT_EN        = 0x340,
    DMX_REG_HW_FIFO_FULL_INT_EVNET     = 0x344,
    DMX_REG_PCR_INT_EN                 = 0x350,
    DMX_REG_PCR_INT_EVENT              = 0x354,
    DMX_REG_LANCY                      = 0x360
};

enum DMX_HW_FLT_MATCH_MODE
{
    DMX_HW_FLT_MATCH_MODE_INCLUDE = 0,

    DMX_HW_FLT_MATCH_MODE_EXCLUDE = 1,
};



enum DMX_HW_CLK_POLAR
{
    DMX_HW_CLK_POLAR_POSEDGE = 0,

    DMX_HW_CLK_POLAR_NEGEDGE = 1,
};

enum DMX_HW_SYNC_MODE
{
    DMX_HW_SYNC_MODE_EXTERNAL = 0,
    DMX_HW_SYNC_MODE_INTERNAL = 1,
};


enum DMX_HW_SYNC_TIME
{
    DMX_HW_SYNC_TIME_1 = 0x0,
	DMX_HW_SYNC_TIME_2 = 0x1,
	DMX_HW_SYNC_TIME_3 = 0x2,
	DMX_HW_SYNC_TIME_7 = 0x3
};

enum M3701C_TSG_REG_OFFSET
{
    TSG_REG_SRC_ADDR_AND_READY     = 0x00,
	TSG_REG_SRC_LEN                = 0x08,	
    TSG_REG_TRANS_CTRL_AND_STATUS  = 0x0C,
	TSG_REG_SYNC_BYTE              = 0x20,
	TSG_REG_FEATURE_CFG            = 0x24,	
    TSG_REG_INT_CFG_AND_STATUS     = 0x28,
};

#define ALI_HW_REG_R32(i)       (*(volatile DMX_UINT32 *)(i))
#define ALI_HW_REG_W32(i, d) do {(*(volatile DMX_UINT32 *)(i)) = (d);} while(0)

#define ALI_HW_REG_R16(i)        (*(volatile DMX_UINT16 *)(i))
#define ALI_HW_REG_W16(i, d)  do {(*(volatile DMX_UINT16 *)(i)) = (d);} while(0)


#define ALI_HW_REG_R8(i)        (*(volatile DMX_UINT8 *)(i))
#define ALI_HW_REG_W8(i, d)  do {(*(volatile DMX_UINT8 *)(i)) = (d);} while(0)

static DMX_INT32 AliRegSetBits32
(
    DMX_UINT32 Address,
    DMX_UINT32 StartBitIdx, /* From 0 to 31*/
    DMX_UINT32 EndBitIdx,/* From 0 to 31, should be bigger than StartBitIdx*/
    DMX_UINT32 BitsValue
)
{
    DMX_UINT32 Value;
    DMX_UINT32 Mask;
    DMX_UINT32 Idx;   

    Mask = 0;

    for (Idx = StartBitIdx; Idx <= EndBitIdx; Idx++)
    {
        Mask |= (1 << Idx);
    }

    BitsValue <<= StartBitIdx;

    BitsValue &= Mask;


    /* Clean Value conrresponding bits to 0. */
    Mask = (~Mask);

    Value = ALI_HW_REG_R32(Address);

    Value &= Mask;


    /* Set BitsValue to Value. */
    Value |= BitsValue;

 
    /* Set Value to HW. */
    ALI_HW_REG_W32(Address, Value);    

    return(0);
}



static DMX_UINT32 AliRegGetBits32
(
    DMX_UINT32 Address,
    DMX_UINT32 StartBitIdx, /* From 0 to 31*/
    DMX_UINT32 EndBitIdx/* From 0 to 31, should be bigger than StartBitIdx*/
)
{
    DMX_UINT32 Value;
    DMX_UINT32 Mask;
    DMX_UINT32 Idx;   

    Mask = 0;

    for (Idx = StartBitIdx; Idx <= EndBitIdx; Idx++)
    {
        Mask |= (1 << Idx);
    }

    Value = ALI_HW_REG_R32(Address);

    Value &= Mask;

    Value >>= StartBitIdx;  

    return(Value);
}


DMX_INT32 AliRegSet32
(
    DMX_UINT32 Address,
    DMX_UINT32 Value
)
{
    ALI_HW_REG_W32(Address, Value);    

    return(0);
}

DMX_UINT32 AliRegGet32
(
    DMX_UINT32 Address
)
{
    DMX_UINT32 Value;

    Value = ALI_HW_REG_R32(Address);    

    return(Value);
}

DMX_INT32 AliRegSet16
(
    DMX_UINT32 Address,
    DMX_UINT16 Value
)
{
    ALI_HW_REG_W16(Address, Value);    

    return(0);
}


DMX_UINT16 AliRegGet16
(
    DMX_UINT32 Address
)
{
    DMX_UINT16 Value;

    Value = ALI_HW_REG_R16(Address);    

    return(Value);
}

DMX_INT32 AliRegSet8
(
    DMX_UINT32 Address,
    DMX_UINT16 Value
)
{
    ALI_HW_REG_W8(Address, Value);    

    return(0);
}

DMX_UINT16 AliRegGet8
(
    DMX_UINT32 Address
)
{
    DMX_UINT16 Value;

    Value = ALI_HW_REG_R8(Address);    

    return(Value);
}

DMX_INT32 DmxRegGetHwFltPid
(
    DMX_UINT32 DmxBase,
    DMX_UINT32 HwFltIdx
)
{
    DMX_INT32 CpuPid;
    DMX_INT32 HwPid;
    DMX_INT32 Loop;

    /* Select HW filter. */
    AliRegSetBits32(DmxBase + DMX_REG_PID_FLT_CFG, 
                    DMX_BIT_PID_FLT_SELECT_START_IDX,
                    DMX_BIT_PID_FLT_SELECT_END_IDX,
                    HwFltIdx);

    /* Tell HW start to read PID from internal RAM. */
    AliRegSetBits32(DmxBase + DMX_REG_PID_FLT_CFG, 
                    DMX_BIT_PID_FLT_RD_START_IDX,
                    DMX_BIT_PID_FLT_RD_END_IDX,
                    1);   

    /* Pool unitl HW finished RD operation. */
    for (Loop = 0; Loop < 10000; Loop++)
    {
        if (0 == AliRegGetBits32(DmxBase + DMX_REG_PID_FLT_CFG, 
                                 DMX_BIT_PID_FLT_RD_START_IDX,
                                 DMX_BIT_PID_FLT_RD_END_IDX))
        {
            break;
        }
    } 

	if (Loop >= 10000)
	{
        //SDBBP();

        return(0xFFFFFFFF);
	}

    /* Get PID from HW reg. */
    HwPid = AliRegGetBits32(DmxBase + DMX_REG_PID_FLT_VALUE, 
                            DMX_BIT_PID_FLT_VALUE_START_IDX,
                            DMX_BIT_PID_FLT_VALUE_END_IDX);

    /* Change PID byte order to fit HW need. */
    CpuPid = ((HwPid & 0xFF00) >> 8) | ((HwPid & 0x1F) << 8);

    return(CpuPid);
}







DMX_INT32 DmxRegSetHwFltPid
(
    DMX_UINT32 DmxBase,
    DMX_UINT32 HwFltIdx,
    DMX_UINT32 Pid
)
{
    DMX_INT32 HwPid;
    DMX_INT32 Loop;

    if (Pid > 0x1FFF)
    {
        return(-1);
    }

    /* Change PID byte order to fit HW need. */
    HwPid = (Pid >> 8) | (Pid << 8);

    /* Set PID to HW reg. */
    AliRegSetBits32(DmxBase + DMX_REG_PID_FLT_VALUE, 
                    DMX_BIT_PID_FLT_VALUE_START_IDX,
                    DMX_BIT_PID_FLT_VALUE_END_IDX,
                    HwPid);

    /* Select HW filter. */
    AliRegSetBits32(DmxBase + DMX_REG_PID_FLT_CFG, 
                    DMX_BIT_PID_FLT_SELECT_START_IDX,
                    DMX_BIT_PID_FLT_SELECT_END_IDX,
                    HwFltIdx);

    /* Tell HW start to read PID to internal RAM. */
    AliRegSetBits32(DmxBase + DMX_REG_PID_FLT_CFG, 
                    DMX_BIT_PID_FLT_WR_START_IDX,
                    DMX_BIT_PID_FLT_WR_END_IDX,
                    1);    
    
    /* Pool unitl HW finished WR operation. */
    for (Loop = 0; Loop < 10000; Loop++)
    {
        if (0 == AliRegGetBits32(DmxBase + DMX_REG_PID_FLT_CFG, 
                                 DMX_BIT_PID_FLT_WR_START_IDX,
                                 DMX_BIT_PID_FLT_WR_END_IDX))
        {
            return(0);
        }
    }

    //SDBBP();

    return(-1);
}






DMX_INT32 DmxRegSetHwFltEn
(
    DMX_UINT32 DmxBase,
    DMX_UINT32 HwFltIdx,
    DMX_UINT32 HwFltEn
)
{
    DMX_UINT32 RegOffset;

    if (HwFltIdx < 32)
    {
        RegOffset = DMX_REG_PID_FLT_EN;
    }
    else if (HwFltIdx < 64)
    {
        RegOffset = DMX_REG_PID_FLT_EN + 4; 

        HwFltIdx -= 32;
    }    
    else if (HwFltIdx < 96)
    {
        RegOffset = DMX_REG_PID_FLT_EN + 8; 

        HwFltIdx -= 64;
    }
    else
    {
        return(-1);
    }

    AliRegSetBits32(DmxBase + RegOffset, HwFltIdx, HwFltIdx, HwFltEn);

    return(0);
}




DMX_UINT32 DmxRegGetHwFltEn
(
    DMX_UINT32 DmxBase,
    DMX_UINT32 HwFltIdx
)
{
    DMX_UINT32 Value;
    DMX_UINT32 RegOffset;

    if (HwFltIdx < 32)
    {
        RegOffset = DMX_REG_PID_FLT_EN;
    }
    else if (HwFltIdx < 64)
    {
        RegOffset = DMX_REG_PID_FLT_EN + 4; 

        HwFltIdx -= 32;
    }    
    else if (HwFltIdx < 96)
    {
        RegOffset = DMX_REG_PID_FLT_EN + 8; 

        HwFltIdx -= 64;
    }
    else
    {
        return(0);
    }
   
    Value = AliRegGetBits32(DmxBase + RegOffset, HwFltIdx, HwFltIdx);

    return(Value);
}


__s32 m36g_dmx_hw_flt_pid_set
(
    __u32 base_addr,
    __u8  flt_idx,
    __u16 pid
)
{
    return(DmxRegSetHwFltPid(base_addr, flt_idx, pid));
}



__s32 m36g_dmx_hw_flt_enable
(
    __u32 base_addr,
    __u8  flt_idx
)
{
    return(DmxRegSetHwFltEn(base_addr, flt_idx, 1));
}


__s32 m36g_dmx_hw_flt_disable
(
    __u32 base_addr,
    __u8  flt_idx
)
{
    return(DmxRegSetHwFltEn(base_addr, flt_idx, 0));
}







__u32 m36g_dmx_hw_buf_rd_get
(
    __u32 dmx_base
)
{
    //return(DMX_HW_TS_BUF_GET_RD_PTR(dmx_base));

    return(AliRegGet16(dmx_base + DMX_REG_HW_RD_IDX) & 0x1FFF);
}

__u32 m36g_dmx_hw_buf_rd_set
(
    __u32 dmx_base,
    __u32 rd
)
{
    //return(DMX_HW_TS_BUF_SET_RD_PTR(dmx_base, rd));
    //DMX_SET_WORD((dmx_base + OFFSET_TS_BUF_W_R_PTR + 2), (DMX_UINT16)rd);

    AliRegSet16(dmx_base + DMX_REG_HW_RD_IDX, (__u16)rd);

    return(0);
}


__u32 m36g_dmx_hw_buf_wr_get
(
    __u32 dmx_base
)
{
    //return(DMX_HW_TS_BUF_GET_WR_PTR(dmx_base));
    return(AliRegGet16(dmx_base + DMX_REG_HW_WR_IDX) & 0x1FFF);
}






__s32 m36g_dmx_hw_pcr_get
(
    __u32  dmx_base,
    __u32 *pcr
)
{
    return(0);
}

__s32 m36g_dmx_hw_fifo_overflow_get
(
    __u32 dmx_base
)
{
    return(0);
}




__s32 m36g_dmx_hw_reg_reset
(
    __u32 base_addr,
    __u32 ts_buf_addr,
    __u32 ts_buf_pkt_cnt
)
{
#if 0
    DMX_UINT32 i;

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

    return;
#else
    __u32 i;

    /* Rest dmx hw. */
    AliRegSet8(base_addr + DMX_REG_CTRL + 3, 0x01);

    /* Wait 16+ clocks. */
    for (i = 0; i < 1000; i++);

    /* TS buffer 0 start address. */
    AliRegSet32(base_addr + DMX_REG_HW_BUF_ADDR, ts_buf_addr & 0x1FFFFFFF);

    /* Set TS buffer 0 len. */
    DMX_SET_WORD(base_addr + DMX_REG_HW_THRESHOLD_AND_BUF_LEN, 
                 ((__u16)ts_buf_pkt_cnt << 16) | (__u16)ts_buf_pkt_cnt);

    /* Init global congtrol reg. */
    /* external sync mode, PCR disabled, input enabled, bypass CSA, 
     * drop erro pkt, input clk polar inverse of standard.
     */
    //DMX_SET_DWORD(base_addr + OFFSET_GLOBAL_CONTROL, 0x44722);
    AliRegSet32(base_addr + DMX_REG_CTRL, 0xD4722);

#if 1
    AliRegSet32(base_addr + DMX_REG_HW_BUF_FULL_INT_EN, 0x1);

	AliRegSet32(base_addr + DMX_REG_HW_FIFO_FULL_INT_EN, 0x1);

	AliRegSet32(base_addr + DMX_REG_LANCY, 0x110);
#endif

    /* Moved to m36_setup_board.c
	 */
#if 0
    /* On S3701C, DMX has a low DRAM arbitor priority, it we cause DMX
     * has no opertunity to write data for FIFO to DRAM, we change DMX arbitor priority
     * to a high level here to avoid this problem.*/
	AliRegSetBits32(0xB8001010, 10, 11, 0x0);


	AliRegSetBits32(0xB8001010, 29, 29, 0x0);


	AliRegSetBits32(0xB8001014, 9, 9, 0x0);


	AliRegSetBits32(0xB800101C, 8, 15, 0x10);

	AliRegSetBits32(0xB8001080, 8, 15, 0x10);
#endif

    return(0);
#endif
}

__s32 m36g_dmx_hw_pcr_set_flt
(
    __u32 base_addr, 
    __u32 flt_idx
)
{
    AliRegSet8(base_addr + DMX_REG_PCR_CFG + 2,flt_idx);
    return(0);
}    
__s32 m36g_dmx_hw_pcr_int_enable
(
    __u32 base_addr
)
{  

    AliRegSet8(base_addr + DMX_REG_PCR_INT_EN,1);
    return(0);
}     
      
__s32 m36g_dmx_hw_pcr_int_disable
(
    __u32 base_addr
)
{  
    AliRegSet8(base_addr + DMX_REG_PCR_INT_EN,0);
    return(0);
}     
      
__s32 m36g_dmx_hw_pcr_detect_enable
(
    __u32 base_addr
)
{  
    AliRegSetBits32(base_addr + DMX_REG_CTRL, DMX_BIT_PCR_EN_START_IDX, DMX_BIT_PCR_EN_END_IDX, 1);
	
    return(0);
}     
      
__s32 m36g_dmx_hw_pcr_detect_disable
(
    __u32 base_addr
)
{
    AliRegSetBits32(base_addr + DMX_REG_CTRL, DMX_BIT_PCR_EN_START_IDX, DMX_BIT_PCR_EN_END_IDX, 0);

    return(0);
}

__s32 m36g_dmx_hw_set_bypass_mode
(
    __u32 base_addr
)
{
    
    __u32 regAddr = base_addr + DMX_REG_CTRL;
    AliRegSet32(base_addr + DMX_REG_HW_BUF_FULL_INT_EN, 0x0);
    AliRegSet8(regAddr, AliRegGet8(regAddr)|0x04);  
    return(0);
}

__s32 m36g_dmx_hw_clear_bypass_mode
(
    __u32 base_addr
)
{
    __u32 regAddr = base_addr + DMX_REG_CTRL;
    
    AliRegSet8(regAddr, AliRegGet8(regAddr)&(~0x04));  
	
    return(0);
}


irqreturn_t m36g_dmx_hw_isr
(
    int   irq,
    void *dev
)
{
    struct dmx_device *dmx; 
	__u32              ts_buf_overflow = 0;
    __u32              ts_fifo_overflow = 0;
    __u32              wr_idx = 0;
    __u32              rd_idx = 0;
	__u32              pcr_event = 0;
	__u32              pcr_value = 0;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    dmx = (struct dmx_device *)dev;

#if 1
    /* Handle pcr int.
     */
    pcr_event = AliRegGet32(dmx->base_addr + DMX_REG_PCR_INT_EVENT);

    /* Got PCR vaulue from HW. 
     */
    if (1 == pcr_event)
    {
        pcr_value = AliRegGet32(dmx->base_addr + DMX_REG_PCR_VALUE);

        AliRegSet32(dmx->base_addr + DMX_REG_PCR_INT_EVENT, 0x1);

        //printk("%s,line:%d,%s,pcr:%x\n",
               //__FUNCTION__, __LINE__, dmx->name, pcr_value);

        /* TODO: PCR proccess. 
         */
    }
#endif

#if 1
    /* Handle ts buffer overflow int.
     */
    ts_buf_overflow = AliRegGet32(dmx->base_addr + DMX_REG_HW_BUF_FULL_INT_EVNET);

    if (1 == ts_buf_overflow)
    {
        wr_idx = m36g_dmx_hw_buf_wr_get(dmx->base_addr);

        rd_idx = m36g_dmx_hw_buf_rd_get(dmx->base_addr);

        AliRegSet32(dmx->base_addr + DMX_REG_HW_BUF_FULL_INT_EVNET, 0x1);

        printk("%s,line:%d,%s,rd:%d,wr:%d,last parsed pid:%d\n",
               __FUNCTION__, __LINE__, dmx->name, rd_idx, wr_idx, dmx->last_parsed_pid);
    }

    ts_fifo_overflow = AliRegGet32(dmx->base_addr + DMX_REG_HW_FIFO_FULL_INT_EVNET);

    if (1 == ts_fifo_overflow)
    {
        AliRegSet32(dmx->base_addr + DMX_REG_HW_FIFO_FULL_INT_EVNET, 0x1);

        printk("%s,line:%d,%s,rd:%d,wr:%d,last parsed pid:%d\n",
               __FUNCTION__, __LINE__, dmx->name, rd_idx, wr_idx, dmx->last_parsed_pid);
        /* TODO: HW fifo overflow proccess. 
         */
    }
#endif

	return(IRQ_HANDLED);
}
