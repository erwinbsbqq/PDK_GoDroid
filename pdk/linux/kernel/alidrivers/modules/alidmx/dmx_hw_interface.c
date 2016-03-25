
#include <ali_soc_common.h>

#include "dmx_stack.h"
 
/* dev_id: 
 * 0=>HW DMX 0; 1=>HW DMX 1; 2=>HW DMX 2; (Receive Data from HW)
 * 3=>SW DMX 0; 4=>SW DMX 1; 5=>SW DMX 2; (Receive Data from User Space)
 * 6=>SW DMX 3; 7=>SW DMX 4; 8=>SW DMX 5; (Receive Data from See)
 */
 
struct dmx_hw_interface_module ali_dmx_hw_interface_module;


__u32 dmx_hw_flt_total_cnt_get
(
    __u32 interface_id
)
{
    __u32                    ret;
    struct dmx_hw_interface *interface;
    
    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_flt_total_cnt_get)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }

    ret = interface->hw_flt_total_cnt_get(interface->hw_dev_id);

    return(ret);
}



__s32 dmx_hw_flt_pid_set
(
    __u32 interface_id,
    __s32 flt_idx,
    __s32 pid
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_flt_pid_set)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }

    ret = interface->hw_flt_pid_set(interface->hw_dev_id, flt_idx, pid);
    
    return(ret);
}


__s32 dmx_hw_flt_enable
(
    __u32 interface_id,
    __u32 flt_idx
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_flt_enable)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }

    ret = interface->hw_flt_enable(interface->hw_dev_id, flt_idx);
    
    return(ret);
}


__s32 dmx_hw_flt_disable
(
    __u32 interface_id,
    __u32 flt_idx
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_flt_disable)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }
    
    ret = interface->hw_flt_disable(interface->hw_dev_id, flt_idx);

    return(ret);
}



__u32 dmx_hw_buf_rd_get
(
    __u32 interface_id
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_buf_rd_get)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }

    ret = interface->hw_buf_rd_get(interface->hw_dev_id);

    return(ret);
}



__s32 dmx_hw_buf_rd_set
(
    __u32 interface_id,
    __u32 rd
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_buf_rd_set)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }

    ret = interface->hw_buf_rd_set(interface->hw_dev_id, rd);
    
    return(ret);
}




__u32 dmx_hw_buf_wr_get
(
    __u32 interface_id
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_buf_wr_get)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }

    ret = interface->hw_buf_wr_get(interface->hw_dev_id);

    return(ret);
}

__s32 dmx_hw_buf_wr_set
(
    __u32 interface_id,
    __u32 wr
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_buf_wr_set)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }   

    ret = interface->hw_buf_wr_set(interface->hw_dev_id, wr);

    return(ret);
}



__u32 dmx_hw_buf_end_get
(
    __u32 interface_id
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_buf_end_get)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }   

    ret = interface->hw_buf_end_get(interface->hw_dev_id);
    
    return(ret);
}





__u32 dmx_hw_buf_start_addr_get
(
    __u32 interface_id
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_buf_start_addr_get)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }   

    ret = interface->hw_buf_start_addr_get(interface->hw_dev_id);

    return(ret);
}



__s32 dmx_hw_pcr_detect_enable
(
    __u32 interface_id,
    __u32 pid,
    __s32 pcr_value_cb(__u32 pcr,
                       __u32 param),
    __u32 param
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_pcr_enable)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }   

    ret = interface->hw_pcr_enable(interface->hw_dev_id, pid, pcr_value_cb, param);

    return(ret);
}




__s32 dmx_hw_pcr_detect_disable
(
    __u32 interface_id,
    __u32 pid
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_pcr_disable)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }   

    ret = interface->hw_pcr_disable(interface->hw_dev_id, pid);

    return(ret);
}


__s32 dmx_hw_bypass_enable
(
    __u32 interface_id
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_bypass_enable)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }   

    ret = interface->hw_bypass_enable(interface->hw_dev_id);

    return(ret);
}


__s32 dmx_hw_bypass_disable
(
    __u32 interface_id
)
{
    __s32                    ret;
    struct dmx_hw_interface *interface;

    interface = &ali_dmx_hw_interface_module.interface[interface_id];

    if (NULL == interface->hw_bypass_disable)
    {
        return(DMX_ERR_HW_NO_FUNCTION);
    }   

    ret = interface->hw_bypass_disable(interface->hw_dev_id);

    return(ret);
}



/* dev_id: 
 * 0=>HW DMX 0; 1=>HW DMX 1; 2=>HW DMX 2; (Receive Data from HW)
 * 3=>SW DMX 0; 4=>SW DMX 1; 5=>SW DMX 2; (Receive Data from User Space)
 * 6=>SW DMX 3; 7=>SW DMX 4; 8=>SW DMX 5; (Receive Data from See)
 */
__s32 dmx_hw_interface_init
(
    /* ID of this HW interface in it's type of interface. 
    */
    __u32                      hw_interface_id,
    enum DMX_HW_INTERFACE_TYPE hw_dev_type,
    __u32                      hw_dev_id
)
{
    __u32                    chip_id;
    __s32                    ret;
    struct dmx_hw_interface *interface;

    ret = 0;

    interface = &(ali_dmx_hw_interface_module.interface[hw_interface_id]);

    memset(interface, 0, sizeof(struct dmx_hw_interface));
    
    interface->self_id = hw_interface_id; 

    interface->hw_dev_type = hw_dev_type; 

    interface->hw_dev_id = hw_dev_id; 
	
    if (hw_dev_type == DMX_HW_INTERFACE_TYPE_HW)
    {
        chip_id = ali_sys_ic_get_chip_id();

        printk("%s,%d,chip id:0x%x\n", __FUNCTION__, __LINE__, chip_id);
		
        if(chip_id == ALI_S3602F)
        {
            printk("%s,%d\n", __FUNCTION__, __LINE__);
                
            return(ret);
        }
        /* 
         * chip_ID == ALI_S3701C
         */
        else if ((ALI_C3921 == chip_id) || (ALI_S3503 == chip_id))
        {
            //printk("%s,%d\n", __FUNCTION__, __LINE__);
        
            ret = dmx_hw_init_m37(hw_dev_id, interface);
        
            return(ret);
        }
		else
		{
            panic("%s,%d,unsupported chip id:0x%x\n", __FUNCTION__, __LINE__, chip_id);

			return(ret);
		}
	}

    if (hw_dev_type == DMX_HW_INTERFACE_TYPE_USR)
    {
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
        ret = dmx_hw_from_user_init(hw_dev_id, interface);

        return(ret);
	}

    if (hw_dev_type == DMX_HW_INTERFACE_TYPE_SEE)
    {
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
        ret = dmx_hw_from_see_init(hw_dev_id, interface);

        return(ret);
    }

    panic("%s,%d\n", __FUNCTION__, __LINE__);

    return(-1);
}





