
#include "dmx_stack.h"

#include <linux/vmalloc.h>

#define DMX_HW_FROM_USER_DEV_CNT         3

#define DMX_HW_FROM_USER_PER_DEV_FLT_CNT 96

/* Mulitple for both 188 and 204.
*/
#define DMX_HW_FROM_USER_BUF_LEN        (9588 * 64) 

extern __u32 __G_SEE_DMX_SRC_BUF_START;

struct dmx_hw_from_user_flt
{
    __u32 pid;
    __u32 enable;
};


struct dmx_hw_from_user_dev
{
    struct dmx_hw_from_user_flt flt[DMX_HW_FROM_USER_PER_DEV_FLT_CNT];

    __u32 buf;
    __u32 buf_rd_idx;
    __u32 buf_wr_idx;
};


struct dmx_hw_from_user_dev ali_dmx_hw_from_user_dev[DMX_HW_FROM_USER_DEV_CNT];


static __inline__ struct dmx_hw_from_user_dev *dmx_hw_from_user_dev_get_by_id
(
    __u32 dev_id
)
{
	return(&(ali_dmx_hw_from_user_dev[dev_id]));
}



__s32 dmx_hw_from_user_flt_pid_set
(
    __u32 dev_id,
    __u32 flt_idx,
    __u32 pid
)
{
    struct dmx_hw_from_user_dev *hw_dev;

    hw_dev = dmx_hw_from_user_dev_get_by_id(dev_id);
    
    hw_dev->flt[flt_idx].pid = pid;

    return(0);
}



__s32 dmx_hw_from_user_flt_enable
(
    __u32 dev_id,
    __u32 flt_idx
)
{
    struct dmx_hw_from_user_dev *hw_dev;

    hw_dev = dmx_hw_from_user_dev_get_by_id(dev_id);
    
    hw_dev->flt[flt_idx].enable = 1;

    return(0);
}



__s32 dmx_hw_from_user_flt_disable
(
    __u32 dev_id,
    __u32 flt_idx
)
{
    struct dmx_hw_from_user_dev *hw_dev;

    hw_dev = dmx_hw_from_user_dev_get_by_id(dev_id);
    
    hw_dev->flt[flt_idx].enable = 0;

    return(0);
}



__u32 dmx_hw_from_user_flt_total_cnt_get
(
    __u32 dev_id
)
{
    return(DMX_HW_FROM_USER_PER_DEV_FLT_CNT);
}



__u32 dmx_hw_from_user_buf_rd_get
(
    __u32 dev_id
)
{
    struct dmx_hw_from_user_dev *hw_dev;

    hw_dev = dmx_hw_from_user_dev_get_by_id(dev_id);
    
    return(hw_dev->buf_rd_idx);
}



__s32 dmx_hw_from_user_buf_rd_set
(
    __u32 dev_id,
    __u32 rd
)
{
    struct dmx_hw_from_user_dev *hw_dev;

    hw_dev = dmx_hw_from_user_dev_get_by_id(dev_id);

	//printk("%s,%d,rd:%x,gap:%x,align:%d\n", __FUNCTION__, __LINE__, rd, rd - hw_dev->buf_rd_idx, (rd - hw_dev->buf_rd_idx) / 204);
	
    hw_dev->buf_rd_idx = rd;

    return(0);
}


__u32 dmx_hw_from_user_buf_wr_get
(
    __u32 dev_id
)
{
    struct dmx_hw_from_user_dev *hw_dev;

    hw_dev = dmx_hw_from_user_dev_get_by_id(dev_id);
    
    return(hw_dev->buf_wr_idx);
}


__s32 dmx_hw_from_user_buf_wr_set
(
    __u32 dev_id,
    __u32 wr
)
{
    struct dmx_hw_from_user_dev *hw_dev;

    hw_dev = dmx_hw_from_user_dev_get_by_id(dev_id);

    hw_dev->buf_wr_idx = wr;

    return(0);
}



__u32 dmx_hw_from_user_buf_end_get
(
    __u32 dev_id
)
{
    return(DMX_HW_FROM_USER_BUF_LEN);
}



__u32 dmx_hw_from_user_buf_start_addr_get
(
    __u32 dev_id
)
{
    struct dmx_hw_from_user_dev *hw_dev;

    hw_dev = dmx_hw_from_user_dev_get_by_id(dev_id);
	    
    return(hw_dev->buf);
}



__s32 dmx_hw_from_user_pcr_enable
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




__s32 dmx_hw_from_user_pcr_disable
(
    __u32 dev_id,
    __u32 pid
)
{
    return(DMX_ERR_HW_PCR_NO_LIVE_FLT);
}




__s32 dmx_hw_from_user_init
(
    __u32                    dev_id,
    struct dmx_hw_interface *interface
)
{  
    struct dmx_hw_from_user_dev *hw_dev;
    __u8                        *data_buf;

    hw_dev = dmx_hw_from_user_dev_get_by_id(dev_id);

    memset(hw_dev, 0, sizeof(struct dmx_hw_from_user_dev));

    printk("%s,%d,hw_dev->buf:%x,dev_id:%d\n", __FUNCTION__, __LINE__, 
           hw_dev->buf, dev_id);

    data_buf = vmalloc(DMX_HW_FROM_USER_BUF_LEN);

    if (NULL == data_buf)
    {
        panic("%s,%d\n", __FUNCTION__, __LINE__);
    }

    hw_dev->buf = (__u32)data_buf;

    //printk("%s,%d,hw_dev->buf:%x,dev_id:%d\n", __FUNCTION__, __LINE__, 
             //hw_dev->buf, dev_id);

    interface->hw_flt_pid_set = dmx_hw_from_user_flt_pid_set;
    interface->hw_flt_enable = dmx_hw_from_user_flt_enable;
    interface->hw_flt_disable = dmx_hw_from_user_flt_disable;
    interface->hw_flt_total_cnt_get = dmx_hw_from_user_flt_total_cnt_get;
    
    interface->hw_buf_rd_get = dmx_hw_from_user_buf_rd_get;
    interface->hw_buf_rd_set = dmx_hw_from_user_buf_rd_set;
    interface->hw_buf_wr_get = dmx_hw_from_user_buf_wr_get;
    interface->hw_buf_wr_set = dmx_hw_from_user_buf_wr_set;
    interface->hw_buf_end_get = dmx_hw_from_user_buf_end_get;
    interface->hw_buf_start_addr_get = dmx_hw_from_user_buf_start_addr_get;

    interface->hw_pcr_enable = dmx_hw_from_user_pcr_enable;
    interface->hw_pcr_disable = dmx_hw_from_user_pcr_disable;

    printk("%s,%d\n", __FUNCTION__, __LINE__);

    return(0); 
}












