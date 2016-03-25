

#include <linux/delay.h>
#include "dmx_stack.h"
#include "dmx_see_interface.h"
#include <ali_soc_common.h>
#include <ali_interrupt.h>

#define DMX_HW_MAX_DEV_CNT         6

#define ALI_DMX_HW_1_CTRL_BASE      0x18022000
#define ALI_DMX_HW_2_CTRL_BASE      0x18024000
#define ALI_DMX_HW_3_CTRL_BASE      0x18030000
#define ALI_DMX_HW_4_CTRL_BASE      0x18031000

/* for S3503B */
#define ALI_DMX_HW_3_CTRL_BASE_S3503B      0x18020000  
#define ALI_DMX_HW_4_CTRL_BASE_S3503B      0x1803e000  


#define ALI_DMX_HW_BUF_MAX_IDX_M37 0xFFFF

#define ALI_DMX_HW_FLT_TOTAL_M37   96

#define ALI_DMX_HW_BUF_LEN_M37   (256 * 1024)

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
    DMX_REG_HW_BUF_LEN                 = 0x140,
    DMX_REG_HW_BUF_THRESHOLD           = 0x142,
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









struct dmx_hw_dev_info_m37
{
    /* Base address for each HW DMX.
    */
    __s32 hw_ctrl_reg_base;

    __u32 hw_buf_addr;

    /* Counted in 188 bytes per unit.
    */
    __u32 hw_buf_end;

    __s32 (*pcr_value_cb)(__u32 pcr, __u32 pcr_cb_param);
    
    __u32 pcr_cb_param;

#if 0
    /* Irq number for each HW DMX in linux.
    */
    __s32 hw_irq_num;

    /* HW filter total cnt for each HW device. 
    */
    __s32 hw_flt_total_cnt;

    __s32 hw_buf_addr;

    /* HW buffer len for each HW device. 
    */
    __s32 hw_buf_max_idx;
#endif
};



extern unsigned long __G_ALI_MM_DMX_MEM_SIZE;
extern unsigned long __G_ALI_MM_DMX_MEM_START_ADDR;



struct dmx_hw_dev_info_m37 ali_dmx_hw_dev_info_m37[DMX_HW_MAX_DEV_CNT];





static __s32 AliRegSetBits32
(
    __u32 Address,
    __u32 StartBitIdx, /* From 0 to 31*/
    __u32 EndBitIdx,/* From 0 to 31, should be bigger than StartBitIdx*/
    __u32 BitsValue
)
{
    __u32 Value;
    __u32 Mask;
    __u32 Idx;   

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



static __u32 AliRegGetBits32
(
    __u32 Address,
    __u32 StartBitIdx, /* From 0 to 31*/
    __u32 EndBitIdx/* From 0 to 31, should be bigger than StartBitIdx*/
)
{
    __u32 Value;
    __u32 Mask;
    __u32 Idx;   

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


__s32 AliRegSet32
(
    __u32 Address,
    __u32 Value
)
{
    ALI_HW_REG_W32(Address, Value);    

    return(0);
}

__u32 AliRegGet32
(
    __u32 Address
)
{
    __u32 Value;

    Value = ALI_HW_REG_R32(Address);    

    return(Value);
}

__s32 AliRegSet16
(
    __u32 Address,
    __u16 Value
)
{
    ALI_HW_REG_W16(Address, Value);    

    return(0);
}


__u16 AliRegGet16
(
    __u32 Address
)
{
    __u16 Value;

    Value = ALI_HW_REG_R16(Address);    

    return(Value);
}

__s32 AliRegSet8
(
    __u32 Address,
    __u16 Value
)
{
    ALI_HW_REG_W8(Address, Value);    

    return(0);
}

__u16 AliRegGet8
(
    __u32 Address
)
{
    __u16 Value;

    Value = ALI_HW_REG_R8(Address);    

    return(Value);
}

__u32 DmxRegGetHwFltPid
(
    __u32 DmxBase,
    __u32 HwFltIdx
)
{
    __s32 CpuPid;
    __s32 HwPid;
    __s32 Loop;

    /* Select HW filter.
    */
    AliRegSetBits32(DmxBase + DMX_REG_PID_FLT_CFG, 
                    DMX_BIT_PID_FLT_SELECT_START_IDX,
                    DMX_BIT_PID_FLT_SELECT_END_IDX,
                    HwFltIdx);

    /* Tell HW start to read PID from internal RAM.
    */
    AliRegSetBits32(DmxBase + DMX_REG_PID_FLT_CFG, 
                    DMX_BIT_PID_FLT_RD_START_IDX,
                    DMX_BIT_PID_FLT_RD_END_IDX,
                    1);   

    /* Pool unitl HW finished RD operation.
    */
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

    /* Get PID from HW reg.
    */
    HwPid = AliRegGetBits32(DmxBase + DMX_REG_PID_FLT_VALUE, 
                            DMX_BIT_PID_FLT_VALUE_START_IDX,
                            DMX_BIT_PID_FLT_VALUE_END_IDX);

    /* Change PID byte order to fit HW need.
    */
    CpuPid = ((HwPid & 0xFF00) >> 8) | ((HwPid & 0x1F) << 8);

    return(CpuPid);
}







__s32 DmxRegSetHwFltPid
(
    __u32 DmxBase,
    __u32 HwFltIdx,
    __u32 Pid
)
{
    __s32 HwPid;
    __s32 Loop;

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






__s32 DmxRegSetHwFltEn
(
    __u32 DmxBase,
    __u32 HwFltIdx,
    __u32 HwFltEn
)
{
    __u32 RegOffset;

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








__u32 DmxRegGetHwFltEn
(
    __u32 DmxBase,
    __u32 HwFltIdx
)
{
    __u32 Value;
    __u32 RegOffset;

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





__u32 dmx_hw_id2base_m37
(
    __u32 dev_id
)
{
    __u32 ctrl_base;
    __u32 chip_id;

	chip_id = ali_sys_ic_get_chip_id();
	
    switch(dev_id)
    {
        case 0:
        {
            ctrl_base = ALI_DMX_HW_1_CTRL_BASE;
        }
        break;

        case 1:
        {
            ctrl_base = ALI_DMX_HW_2_CTRL_BASE;
        }
        break;

        case 2:
        {			
			if (ALI_S3503 == chip_id)
			{
				ctrl_base = ALI_DMX_HW_3_CTRL_BASE_S3503B;
			}
			else
			{
            	ctrl_base = ALI_DMX_HW_3_CTRL_BASE;
			}
        }
        break;

        case 3:
        {			
			if (ALI_S3503 == chip_id)
			{
				ctrl_base = ALI_DMX_HW_4_CTRL_BASE_S3503B;
			}
			else
			{
            	ctrl_base = ALI_DMX_HW_4_CTRL_BASE;
			}
        }
        break;		

        default:
        {
            panic("%s,%d,dev_id:%d, out of range!!\n",__FUNCTION__, __LINE__,
                  dev_id);

            ctrl_base = 0xFFFFFFFF;
        }
        break;
    }

    return(ctrl_base);
}


#include <ali_interrupt.h>



__u32 dmx_hw_id2irq_m37
(
    __u32 dev_id
)
{
    __u32 irq_num;
    
    switch(dev_id)
    {
        case 0:
        {
            irq_num = INT_ALI_DEMUX1;
        }
        break;

        case 1:
        {
            irq_num = INT_ALI_DEMUX2;
        }
        break;

        case 2:
        {
            irq_num = INT_ALI_DEMUX3;
        }
        break;

        case 3:
        {
            irq_num = INT_ALI_DEMUX4;
        }
        break;


        default:
        {
            panic("%s,%d,dev_id:%d, out of range!!\n",__FUNCTION__, __LINE__,
                  dev_id);

            irq_num = 0xFFFFFFFF;
        }
        break;
    }

    return(irq_num);
}



__u32 dmx_hw_id2buf_addr_m37
(
    __u32 dev_id
)
{
    __u32 buf_start;
    __u32 hw_buf_len;
	__u32 cfg_buf_len;

	cfg_buf_len = __G_ALI_MM_DMX_MEM_SIZE;

    /* DMX buffer configration validation.
     * 2 ^ 16 = 65536 unit of TS pakcet. 
	*/
	if (cfg_buf_len >= ((65536 * 188) + (3 * DMX_SEE_BUF_SIZE)))
	{
        panic("%s,%d,DMX Buffer configration error,len:%u\n",
			  __FUNCTION__, __LINE__, cfg_buf_len);

        return(-__LINE__);
	}
	/* Old way to configure DMX buffer.
	 * DMX HW buffers are configured at system memory map,
	 * main2see buffer, see2main buffer, see decrypt buffer are dynamicly
	 * allocated at driver initilization.
	 */
	else if (cfg_buf_len < ((2 * 1024*1024) + (3 * DMX_SEE_BUF_SIZE)))
	{
		hw_buf_len = cfg_buf_len / 2;
	}
	/* New way to configure DMX buffer to keep all buffer below 256M 
	 * ram address.
	 * All buffers, DMX HW buffers , main2see buffer, see2main buffer,
	 * see decrypt buffer are configured at system memory map.
	 */	
	else
	{
		hw_buf_len = (cfg_buf_len - (3 * DMX_SEE_BUF_SIZE) - (2 * DMX_DEENCRYPT_BUF_LEN)) / 4;
	}

    switch(dev_id)
    {
        case 0:
        {
            /* Physical address lies below 512M in MIPS system.
            */
            //buf_start = __G_ALI_MM_DMX_MEM_START_ADDR & 0x0FFFFFFF;
            //buf_start = (__u32)(virt_to_phys(__G_ALI_MM_DMX_MEM_START_ADDR));
            
            /* Change CPU virtual address to HW physical address.
        	*/		
        	#ifdef CONFIG_ARM
            buf_start = __G_ALI_MM_DMX_MEM_START_ADDR - 0xC0000000;
            #else
            buf_start = __G_ALI_MM_DMX_MEM_START_ADDR - ALI_REGS_VIRT_BASE;
            #endif

        }
        break;

        case 1:
        {
            /* Physical address lies below 512M in MIPS system.
            */
            //buf_start = (__G_ALI_MM_DMX_MEM_START_ADDR & 0x0FFFFFFF) + hw_buf_len;
            //buf_start = (__u32)(virt_to_phys(__G_ALI_MM_DMX_MEM_START_ADDR + hw_buf_len));

            /* Change CPU virtual address to HW physical address.
        	*/				
        	#ifdef CONFIG_ARM
            buf_start = __G_ALI_MM_DMX_MEM_START_ADDR - 0xC0000000 + hw_buf_len;
            #else
            buf_start = __G_ALI_MM_DMX_MEM_START_ADDR - ALI_REGS_VIRT_BASE + hw_buf_len;
            #endif

        }
        break;

        case 2:
        {
            /* Physical address lies below 512M in MIPS system.
            */
            //buf_start = __G_ALI_MM_DMX_MEM_START_ADDR & 0x0FFFFFFF;
            //buf_start = (__u32)(virt_to_phys(__G_ALI_MM_DMX_MEM_START_ADDR));
            
            /* Change CPU virtual address to HW physical address.
        	*/		
        	#ifdef CONFIG_ARM
            buf_start =  __G_ALI_MM_DMX_MEM_START_ADDR - 0xC0000000 + (hw_buf_len * 2);
            #else
            buf_start =  __G_ALI_MM_DMX_MEM_START_ADDR - ALI_REGS_VIRT_BASE + (hw_buf_len * 2);
            #endif

        }
        break;

        case 3:
        {
            /* Physical address lies below 512M in MIPS system.
            */
            //buf_start = (__G_ALI_MM_DMX_MEM_START_ADDR & 0x0FFFFFFF) + hw_buf_len;
            //buf_start = (__u32)(virt_to_phys(__G_ALI_MM_DMX_MEM_START_ADDR + hw_buf_len));

            /* Change CPU virtual address to HW physical address.
        	*/		
        	#ifdef CONFIG_ARM
            buf_start =  __G_ALI_MM_DMX_MEM_START_ADDR - 0xC0000000 + (hw_buf_len * 3);
            #else
            buf_start =  __G_ALI_MM_DMX_MEM_START_ADDR - ALI_REGS_VIRT_BASE + (hw_buf_len * 3);
            #endif

        }
        break;		

        default:
        {
            panic("%s,%d,dev_id:%d, out of range!!\n",__FUNCTION__, __LINE__,
                  dev_id);

            buf_start = 0xFFFFFFFF;
        }
        break;
    }

    return(buf_start);
}





__u32 dmx_hw_id2buf_pkt_cnt_m37
(
    __u32 dev_id
)
{
    __u32 hw_buf_len;
    __u32 pkt_cnt;
	__u32 cfg_buf_len;

	cfg_buf_len = __G_ALI_MM_DMX_MEM_SIZE;

    /* DMX buffer configration validation.
     * 2 ^ 16 = 65536 unit of TS pakcet. 
	*/
	if (cfg_buf_len >= ((65536 * 188) + (3 * DMX_SEE_BUF_SIZE)))
	{
        panic("%s,%d,DMX Buffer configration error,len:%u\n",
			  __FUNCTION__, __LINE__, cfg_buf_len);

        return(-__LINE__);
	}
	/* Old way to configure DMX buffer.
	 * DMX HW buffers are configured at system memory map,
	 * main2see buffer, see2main buffer, see decrypt buffer are dynamicly
	 * allocated at driver initilization.
	 */
	else if (cfg_buf_len < ((2 * 1024 * 1024) + (3 * DMX_SEE_BUF_SIZE)))
	{
		hw_buf_len = cfg_buf_len / 2;
	}
	/* New way to configure DMX buffer to keep all buffer below 256M 
	 * ram address.
	 * All buffers, DMX HW buffers , main2see buffer, see2main buffer,
	 * see decrypt buffer are configured at system memory map.
	 */	
	else
	{
		hw_buf_len = (cfg_buf_len - (3 * DMX_SEE_BUF_SIZE)) / 4;
	}	
    
    pkt_cnt = hw_buf_len / 188;
    
    if (pkt_cnt > 0xFFFF)
    {
        panic("%s,%d,ts_buf_pkt_cnt:%d, out of range!!\n",__FUNCTION__,
              __LINE__, pkt_cnt);
        
        pkt_cnt = 0xFFFF;
    }

    return(pkt_cnt);
}




__s32 dmx_hw_flt_pid_set_m37
(
    __u32 dev_id,
    __u32 flt_idx,
    __u32 pid
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    return(DmxRegSetHwFltPid(base_addr, flt_idx, pid));
}



__s32 dmx_hw_flt_enable_m37
(
    __u32 dev_id,
    __u32 flt_idx
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    return(DmxRegSetHwFltEn(base_addr, flt_idx, 1));
}


__s32 dmx_hw_flt_disable_m37
(
    __u32 dev_id,
    __u32 flt_idx
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    return(DmxRegSetHwFltEn(base_addr, flt_idx, 0));
}



__u32 dmx_hw_flt_total_cnt_get_m37
(
    __u32 dev_id
)
{
    return(ALI_DMX_HW_FLT_TOTAL_M37);
}



__u32 dmx_hw_buf_rd_get_m37
(
    __u32 dev_id
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    return(AliRegGet16(base_addr + DMX_REG_HW_RD_IDX) & 0x1FFF);
}



__s32 dmx_hw_buf_rd_set_m37
(
    __u32 dev_id,
    __u32 rd
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    AliRegSet16(base_addr + DMX_REG_HW_RD_IDX, (__u16)rd);

    return(0);
}




__u32 dmx_hw_buf_wr_get_m37
(
    __u32 dev_id
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    return(AliRegGet16(base_addr + DMX_REG_HW_WR_IDX) & 0x1FFF);
}



__s32 dmx_hw_buf_wr_set_m37
(
    __u32 dev_id,
    __u32 wr
)
{
    return(DMX_ERR_HW_NOT_PERMIT);
}


__u32 dmx_hw_buf_end_get_m37
(
    __u32 dev_id
)
{
#if 1
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    /* end_idx = buf_unit_count - 1
	*/
    return(AliRegGet16(base_addr + DMX_REG_HW_BUF_LEN) & 0x1FFF) - 1;
#else
    return(ali_dmx_hw_dev_info_m37[dev_id].hw_buf_end);
#endif
}



__u32 dmx_hw_buf_start_addr_get_m37
(
    __u32 dev_id
)
{
#if 1
    __u32 base_addr;
    __u32 virt_addr;
    __u32 phys_addr;
	
    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    //return(AliRegGet32(base_addr + DMX_REG_HW_BUF_ADDR) & 0x3FFFFFFF);
    
    /* Access dmx hw buffer through cached address to improve pefermance.
    */
    //return(AliRegGet32(base_addr + DMX_REG_HW_BUF_ADDR) | 0x80000000);
    phys_addr = AliRegGet32(base_addr + DMX_REG_HW_BUF_ADDR);

	//virt_addr = (__u32)(phys_to_virt(phys_addr));
	
    /* Change HW physical address to CPU virtual address.
	*/		
	#ifdef CONFIG_ARM
    virt_addr = phys_addr + 0xC0000000;
    #else
    virt_addr = phys_addr | ALI_REGS_VIRT_BASE;
    #endif
	
    //printk("%s,%d,phys_addr:%x,virt_addr:%x\n", __FUNCTION__, __LINE__, phys_addr, virt_addr);
	
    return(virt_addr);
	
#else
    return(ali_dmx_hw_dev_info_m37[dev_id].hw_buf_addr);
#endif
}




__s32 dmx_hw_pcr_set_flt_m37
(
    __u32 dev_id, 
    __u32 flt_idx
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    AliRegSet8(base_addr + DMX_REG_PCR_CFG + 2, flt_idx);
    
    return(0);
}    


__s32 dmx_hw_pcr_int_enable_m37
(
    __u32 dev_id
)
{  
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    AliRegSet8(base_addr + DMX_REG_PCR_INT_EN, 1);
    
    return(0);
}     


      
__s32 dmx_hw_pcr_int_disable_m37
(
    __u32 dev_id
)
{  
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    AliRegSet8(base_addr + DMX_REG_PCR_INT_EN, 0);
    
    return(0);
}     



      
__s32 dmx_hw_pcr_detect_enable_m37
(
    __u32 dev_id
)
{  
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    AliRegSetBits32(base_addr + DMX_REG_CTRL, DMX_BIT_PCR_EN_START_IDX,
                    DMX_BIT_PCR_EN_END_IDX, 1);
    
    return(0);
}     

      
__s32 dmx_hw_pcr_detect_disable_m37
(
    __u32 dev_id
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    AliRegSetBits32(base_addr + DMX_REG_CTRL, DMX_BIT_PCR_EN_START_IDX,
                    DMX_BIT_PCR_EN_END_IDX, 0);

    return(0);
}



__s32 dmx_hw_pcr_enable_m37
(
    __u32 dev_id,
    __u32 pid,
    __s32 pcr_value_cb(__u32 pcr,
                       __u32 param),
    __u32 param
)
{
    __u32 idx;
    __u32 base_addr;
    __u32 en;
    __u32 flt_pid;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;
    
    for (idx = 0; idx < ALI_DMX_HW_FLT_TOTAL_M37; idx++)
    {
        en = DmxRegGetHwFltEn(base_addr, idx);

        if (1 == en)
        {
            flt_pid = DmxRegGetHwFltPid(base_addr, idx);

            if (flt_pid == pid)
            {
                ali_dmx_hw_dev_info_m37[dev_id].pcr_value_cb = pcr_value_cb;

                ali_dmx_hw_dev_info_m37[dev_id].pcr_cb_param = param;
                
                dmx_hw_pcr_set_flt_m37(dev_id, idx);

                dmx_hw_pcr_int_enable_m37(dev_id);

                dmx_hw_pcr_detect_enable_m37(dev_id);

                return(0);
            }
        }
    }

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    return(DMX_ERR_HW_PCR_NO_LIVE_FLT);
}




__s32 dmx_hw_pcr_disable_m37
(
    __u32 dev_id,
    __u32 pid
)
{
    __u32 idx;
    __u32 base_addr;
    __u32 en;
    __u32 flt_pid;
    
    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;
    
    for (idx = 0; idx < ALI_DMX_HW_FLT_TOTAL_M37; idx++)
    {
        en = DmxRegGetHwFltEn(base_addr, idx);
    
        if (1 == en)
        {
            flt_pid = DmxRegGetHwFltPid(base_addr, idx);
    
            if (flt_pid == pid)
            {    
                dmx_hw_pcr_int_disable_m37(dev_id);
    
                dmx_hw_pcr_detect_disable_m37(dev_id);
    
                return(0);
            }
        }
    }
    
    printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    return(DMX_ERR_HW_PCR_NO_LIVE_FLT);
}


__s32 dmx_hw_bypass_enable_m37
(
    __u32 dev_id
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    AliRegSetBits32(base_addr + DMX_REG_CTRL, DMX_BIT_BYPASS_START_IDX,
                    DMX_BIT_BYPASS_END_IDX, 1);
    
    return(0);
};



__s32 dmx_hw_bypass_disable_m37
(
    __u32 dev_id
)
{
    __u32 base_addr;

    base_addr = ali_dmx_hw_dev_info_m37[dev_id].hw_ctrl_reg_base;

    AliRegSetBits32(base_addr + DMX_REG_CTRL, DMX_BIT_BYPASS_START_IDX,
                    DMX_BIT_BYPASS_END_IDX, 0);
    
    return(0);
}



irqreturn_t dmx_hw_isr_m37
(
    int   irq,
    void *dev_id
)
{
    __u32 base_addr;
    __u32 ts_buf_overflow;
    __u32 ts_fifo_overflow;
    __u32 wr_idx;
    __u32 rd_idx;
    __u32 pcr_event;
    __u32 pcr_value;
    __u32 param;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    base_addr = dmx_hw_id2base_m37((__u32)dev_id);

    if (0xFFFFFFFF == base_addr)
    {
        return(DMX_ERR_HW_NOT_EXIST);
    }

    /* Handle pcr int.
     */
    pcr_event = AliRegGet32(base_addr + DMX_REG_PCR_INT_EVENT);

    /* Got PCR vaulue from HW. 
     */
    if (1 == pcr_event)
    {
        pcr_value = AliRegGet32(base_addr + DMX_REG_PCR_VALUE);

        AliRegSet32(base_addr + DMX_REG_PCR_INT_EVENT, 0x1);

        param = ali_dmx_hw_dev_info_m37[(__u32)dev_id].pcr_cb_param;


        if (ali_dmx_hw_dev_info_m37[(__u32)dev_id].pcr_value_cb)
        {        
    		ali_dmx_hw_dev_info_m37[(__u32)dev_id].pcr_value_cb(pcr_value, param);
        }
        

        //printk("%s,line:%d,%x,pcr:%x\n",
                 //__FUNCTION__, __LINE__, base_addr, pcr_value);
    }

    /* Handle ts buffer overflow int.
     */
    ts_buf_overflow = AliRegGet32(base_addr + DMX_REG_HW_BUF_FULL_INT_EVNET);

    if (1 == ts_buf_overflow)
    {
        wr_idx = dmx_hw_buf_wr_get_m37((__u32)dev_id);

        rd_idx = dmx_hw_buf_rd_get_m37((__u32)dev_id);

        AliRegSet32(base_addr + DMX_REG_HW_BUF_FULL_INT_EVNET, 0x1);

        //printk("%s,line:%d,%x,rd:%d,wr:%d\n",
                 //__FUNCTION__, __LINE__, base_addr, rd_idx, wr_idx);
    }

    ts_fifo_overflow = AliRegGet32(base_addr + DMX_REG_HW_FIFO_FULL_INT_EVNET);

    if (1 == ts_fifo_overflow)
    {
        AliRegSet32(base_addr + DMX_REG_HW_FIFO_FULL_INT_EVNET, 0x1);

        printk("%s,line:%d\n", __FUNCTION__, __LINE__);
        
        /* TODO: HW fifo overflow proccess. 
         */
    }

    return(IRQ_HANDLED);
}



__s32 dmx_hw_init_m37
(
    __u32                    dev_id,
    struct dmx_hw_interface *interface
)
{  
    __u32                       ctrl_base;
    __u32                       irq_num;
    __u32                       ts_buf_pkt_cnt;
    __u8                        irq_name[30];
    __s32                       result;
    struct dmx_hw_dev_info_m37 *dev_info;

#if 0
    __u32                       buf_len_order;
    __u32                       phy_addr;
#else
    __u32                       hw_buf_addr;
#endif

    /* Implement following functions according to HW spec.
    */
    interface->hw_flt_pid_set = dmx_hw_flt_pid_set_m37;
    interface->hw_flt_enable = dmx_hw_flt_enable_m37;
    interface->hw_flt_disable = dmx_hw_flt_disable_m37;
    interface->hw_flt_total_cnt_get = dmx_hw_flt_total_cnt_get_m37;
    
    interface->hw_buf_rd_get = dmx_hw_buf_rd_get_m37;
    interface->hw_buf_rd_set = dmx_hw_buf_rd_set_m37;
    interface->hw_buf_wr_get = dmx_hw_buf_wr_get_m37;
    interface->hw_buf_wr_set = dmx_hw_buf_wr_set_m37;
    interface->hw_buf_end_get = dmx_hw_buf_end_get_m37;
    interface->hw_buf_start_addr_get = dmx_hw_buf_start_addr_get_m37;

    interface->hw_pcr_enable = dmx_hw_pcr_enable_m37;
    interface->hw_pcr_disable = dmx_hw_pcr_disable_m37;

    interface->hw_bypass_enable = dmx_hw_bypass_enable_m37;
    interface->hw_bypass_disable = dmx_hw_bypass_disable_m37;	

    ctrl_base = dmx_hw_id2base_m37(dev_id);

    irq_num = dmx_hw_id2irq_m37(dev_id);

    /* Register ISR.
     */
    sprintf(irq_name, "ali_dmx_irq_%d", dev_id);

//    printk("%s,%d,name:%s\n", __FUNCTION__, __LINE__, irq_name);

    result = request_irq(irq_num, dmx_hw_isr_m37, 0, irq_name, (void *)dev_id);

    if (result)
    {
        panic("%s,%d,name:%s\n", __FUNCTION__, __LINE__, irq_name);

        //return(DMX_ERR_HW_REG_ISR_FAIL);
    }

    dev_info = &(ali_dmx_hw_dev_info_m37[dev_id]);

    dev_info->hw_ctrl_reg_base = ctrl_base;

#if 0
    buf_len_order = get_order(ALI_DMX_HW_BUF_LEN_M37);

    dev_info->hw_buf_addr = (__u32)__get_free_pages(GFP_KERNEL, buf_len_order);

    printk("%s,%d,hw_buf_addr:%x\n", __FUNCTION__, __LINE__, dev_info->hw_buf_addr);

    if (0 == dev_info->hw_buf_addr)
    {
        panic("%s,%d\n", __FUNCTION__, __LINE__);

        //return(-ENOMEM);
    }

    phy_addr = virt_to_phys((void *)dev_info->hw_buf_addr);

    printk("%s,%d,phy_addr:%x\n", __FUNCTION__, __LINE__, phy_addr);

    /* HW limitaion.
    */
    if (phy_addr > 0x3FFFFFFF)
    {
        panic("%s,%d,phy_addr:%x\n", __FUNCTION__, __LINE__, phy_addr);
    }
#else
    hw_buf_addr = dmx_hw_id2buf_addr_m37(dev_id); 

    ts_buf_pkt_cnt = dmx_hw_id2buf_pkt_cnt_m37(dev_id);

//    printk("%s,%d,hw_buf_addr:%x,ts_buf_pkt_cnt:%x\n", __FUNCTION__, __LINE__, 
//           hw_buf_addr, ts_buf_pkt_cnt);

#endif
    //dev_info->hw_buf_end = ts_buf_pkt_cnt;

    /* Rest dmx hw. 
    */
    AliRegSet8(ctrl_base + DMX_REG_CTRL + 3, 0x01);

    /* Wait 16+ clocks. 
    */
    msleep(2);

    /* TS buffer 0 start address, mask hw_buf_addr .
    */
    AliRegSet32(ctrl_base + DMX_REG_HW_BUF_ADDR, hw_buf_addr);

    /* Set TS buffer 0 len & data income interrupt threthold.
    */
#if 0
    AliRegSet32(base + DMX_REG_HW_THRESHOLD_AND_BUF_LEN, 
                ((__u16)ts_buf_pkt_cnt << 16) | (__u16)ts_buf_pkt_cnt);
#else
    AliRegSet16(ctrl_base + DMX_REG_HW_BUF_LEN, (__u16)ts_buf_pkt_cnt);

    AliRegSet16(ctrl_base + DMX_REG_HW_BUF_THRESHOLD, (__u16)ts_buf_pkt_cnt);
#endif

    /* Init global congtrol reg.
     */
    AliRegSet32(ctrl_base + DMX_REG_CTRL, 0xD4722);

    AliRegSet32(ctrl_base + DMX_REG_HW_BUF_FULL_INT_EN, 0x1);

    AliRegSet32(ctrl_base + DMX_REG_HW_FIFO_FULL_INT_EN, 0x1);

    AliRegSet32(ctrl_base + DMX_REG_LANCY, 0x110);

    return(0); 
}






