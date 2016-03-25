

#include "dmx_stack.h"

#include "dmx_see_interface.h"

#define DMX_HW_FROM_SEE_DEV_CNT         3

#define DMX_HW_FROM_SEE_PER_DEV_FLT_CNT 96



struct dmx_hw_from_see_flt
{
    __u32 pid;
    __u32 enable;
};


struct dmx_hw_from_see_dev
{
    struct dmx_hw_from_see_flt flt[DMX_HW_FROM_SEE_PER_DEV_FLT_CNT];
};


struct dmx_hw_from_see_dev ali_dmx_hw_from_see_dev[DMX_HW_FROM_SEE_DEV_CNT];





static __inline__ struct dmx_hw_from_see_dev *dmx_hw_from_see_dev_get_by_id
(
    __u32 dev_id
)
{
	return(&(ali_dmx_hw_from_see_dev[dev_id]));
}



__s32 dmx_hw_from_see_flt_pid_set
(
    __u32 dev_id,
    __u32 flt_idx,
    __u32 pid
)
{
    struct dmx_hw_from_see_dev *hw_dev;

    hw_dev = dmx_hw_from_see_dev_get_by_id(dev_id);
    
    hw_dev->flt[flt_idx].pid = pid;

    return(0);
}



__s32 dmx_hw_from_see_flt_enable
(
    __u32 dev_id,
    __u32 flt_idx
)
{
    struct dmx_hw_from_see_dev *hw_dev;

    hw_dev = dmx_hw_from_see_dev_get_by_id(dev_id);
    
    hw_dev->flt[flt_idx].enable = 1;

    return(0);
}





__s32 dmx_hw_from_see_flt_disable
(
    __u32 dev_id,
    __u32 flt_idx
)
{
    struct dmx_hw_from_see_dev *hw_dev;

    hw_dev = dmx_hw_from_see_dev_get_by_id(dev_id);
    
    hw_dev->flt[flt_idx].enable = 0;

    return(0);
}



__u32 dmx_hw_from_see_flt_total_cnt_get
(
    __u32 dev_id
)
{
    return(DMX_HW_FROM_SEE_PER_DEV_FLT_CNT);
}



__u32 dmx_hw_from_see_buf_rd_get
(
    __u32 dev_id
)
{
    return(dmx_see_see2main_buf_rd_get(dev_id));
}



__s32 dmx_hw_from_see_buf_rd_set
(
    __u32 dev_id,
    __u32 rd
)
{
    return(dmx_see_see2main_buf_rd_set(dev_id, rd));
}


__u32 dmx_hw_from_see_buf_wr_get
(
    __u32 dev_id
)
{
    return(dmx_see_see2main_buf_wr_get(dev_id));
}



__s32 dmx_hw_from_see_buf_wr_set
(
    __u32 dev_id,
    __u32 wr
)
{
    return(dmx_see_see2main_buf_wr_set(dev_id, wr));
}



__u32 dmx_hw_from_see_buf_end_get
(
    __u32 dev_id
)
{
    return(dmx_see_see2main_buf_end_idx_get(dev_id));
}



__u32 dmx_hw_from_see_buf_start_addr_get
(
    __u32 dev_id
)
{
    return(dmx_see_see2main_buf_start_addr_get(dev_id));
}



__s32 dmx_hw_from_see_pcr_enable
(
    __u32 dev_id,
    __u32 pid,
    __s32 pcr_value_cb(__u32 pcr,
                       __u32 param),
    __u32 param
)
{
    return(DMX_ERR_HW_PCR_NO_LIVE_FLT);
}




__s32 dmx_hw_from_see_pcr_disable
(
    __u32 dev_id,
    __u32 pid
)
{
    return(DMX_ERR_HW_PCR_NO_LIVE_FLT);
}



__s32 dmx_hw_from_see_init
(
    __u32                    dev_id,
    struct dmx_hw_interface *interface
)
{  
    struct dmx_hw_from_see_dev *hw_dev;

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    hw_dev = dmx_hw_from_see_dev_get_by_id(dev_id);

    memset(hw_dev, 0, sizeof(struct dmx_hw_from_see_dev));

    interface->hw_flt_pid_set = dmx_hw_from_see_flt_pid_set;
    interface->hw_flt_enable = dmx_hw_from_see_flt_enable;
    interface->hw_flt_disable = dmx_hw_from_see_flt_disable;
    interface->hw_flt_total_cnt_get = dmx_hw_from_see_flt_total_cnt_get;
    
    interface->hw_buf_rd_get = dmx_hw_from_see_buf_rd_get;
    interface->hw_buf_rd_set = dmx_hw_from_see_buf_rd_set;
    interface->hw_buf_wr_get = dmx_hw_from_see_buf_wr_get;
    interface->hw_buf_wr_set = dmx_hw_from_see_buf_wr_set;
    interface->hw_buf_end_get = dmx_hw_from_see_buf_end_get;
    interface->hw_buf_start_addr_get = dmx_hw_from_see_buf_start_addr_get;

    interface->hw_pcr_enable = dmx_hw_from_see_pcr_enable;
    interface->hw_pcr_disable = dmx_hw_from_see_pcr_disable;

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    return(0); 
}





