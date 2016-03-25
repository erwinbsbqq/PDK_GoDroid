
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/delay.h>

#include "dmx_see_interface.h"

#include "dmx_stack.h"

struct dmx_pid_flt_module ali_dmx_pid_flt_module;



__s32 dmx_pid_flt_parse
(
    __u32  src_hw_interface_id,
    __u8  *ts_pkt_addr
)
{
    __s32                              ret;
    struct dmx_ts_pkt_inf              pkt_inf;
    struct dmx_pid_flt_hw_abstraction *hw_abstraction;
    struct dmx_pid_flt_hw             *flt_hw;
    struct dmx_pid_flt_sw             *flt_sw;
    //__s32                            ret;
	memset(&pkt_inf, 0, sizeof(struct dmx_ts_pkt_inf));
    
    pkt_inf.pkt_addr = ts_pkt_addr;

    /* Handle sepecial flag TS pakcet for FF/FB in PVR.
     */
    if (0xAA == pkt_inf.pkt_addr[0])
    {
        dmx_see_buf_wr_ts(&pkt_inf, src_hw_interface_id);

        //printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_OK);
    }   

    pkt_inf.pid = ((ts_pkt_addr[1] & 0x1F) << 8) | ts_pkt_addr[2];

    hw_abstraction = &(ali_dmx_pid_flt_module.hw_abstraction[src_hw_interface_id]);

    /* Loop through all alive TP filters to retrieve all TS pakcet for each
     * TP filter.
     */  
    list_for_each_entry(flt_hw, &(hw_abstraction->tp_run_list), link)
    {
		list_for_each_entry(flt_sw, &(flt_hw->sw_run_list), link)
		{
			if (DMX_PID_FLT_SW_STATE_RUN == flt_sw->state)
			{
				ret = flt_sw->pid_flt_cb(&pkt_inf, flt_sw->cb_param);
				
				if (ret < 0)
				{
					//printk("%s,%d\n", __FUNCTION__, __LINE__);
				}
		
				/* Statistics.
				*/
				flt_sw->ts_in_cnt++;
			}
		}
	}

    /* Loop through all alive pid filters to find a march with
     * current TS pakcet PID.
     */            
    list_for_each_entry(flt_hw, &(hw_abstraction->hw_run_list), link)
    {       
        if (pkt_inf.pid == flt_hw->pid)
        {
            list_for_each_entry(flt_sw, &(flt_hw->sw_run_list), link)
            {
                if (DMX_PID_FLT_SW_STATE_RUN == flt_sw->state)
                {
					ret = flt_sw->pid_flt_cb(&pkt_inf, flt_sw->cb_param);
					
					/* Caution: No ts packet handling proccess is allowed to 
					 * return DMX_ERR_BUF_BUSY except dmx_see_buf_wr_ts(), 
					 * because in PVR case, SEE may need to hold dmx by no
					 * longer retrieving data from main2see buffer.
					 */
					if (DMX_ERR_BUF_BUSY == ret)
					{
						return(DMX_ERR_BUF_BUSY);
					}

                    /* Statistics.
					*/
					flt_sw->ts_in_cnt++;
				}
            }

            /* No need to check the rest hw filters since demux hw
             * does not support multi hit, so there should be no hw filter with
             * the same pid after current one.
             * We provide multi hit by the concept of sw pid filter.
             * If this 
             */
			return(DMX_ERR_OK);
        }
    } 

    /* Noboday cares this TS packet, must be somthing wrong.
     * TODO: error handling.
     */
    return(DMX_ERR_PID_FLT_SUSPICIOUS_TS);
}



/* pid_flt == hw filter in our stack.
*/
__s32 dmx_pid_flt_register
(
    __u32 src_hw_interface_id,
    __u32 pid,
    __s32 (*pid_flt_cb)(struct dmx_ts_pkt_inf *pkt_inf,
                        __u32                  cb_param),
    __u32 cb_param                    
)
{
    __s32                              flt_sw_idx;
    __s32                              flt_hw_idx;
    struct dmx_pid_flt_sw             *flt_sw;
    struct dmx_pid_flt_hw             *flt_hw;
    __s32                              total_hw_flt_cnt;
    struct dmx_pid_flt_hw_abstraction *hw_abstraction;

    /* Find a IDLE sw pid filter slot.
     */
    for (flt_sw_idx = 0; flt_sw_idx < DMX_PID_FLT_MAX_CNT; flt_sw_idx++)
    {
        flt_sw = &(ali_dmx_pid_flt_module.flt_sw[flt_sw_idx]);

        if (DMX_PID_FLT_SW_STATE_IDLE == flt_sw->state)
        {
            break;//return(idx);
        }
    }

    if (flt_sw_idx >= DMX_PID_FLT_MAX_CNT)
    {
        return(DMX_ERR_PID_FLT_EXHAUST);
    }

    flt_sw = &(ali_dmx_pid_flt_module.flt_sw[flt_sw_idx]);

    /* Find a RUN hw pid filter with the same PID.
     */
    hw_abstraction = &(ali_dmx_pid_flt_module.hw_abstraction[src_hw_interface_id]);

    total_hw_flt_cnt = dmx_hw_flt_total_cnt_get(src_hw_interface_id);

    if ((total_hw_flt_cnt >= DMX_PID_FLT_MAX_HW_FLT_EACH_DEV) ||
		(total_hw_flt_cnt <= 0))
    {
        panic("%s,%d,total_hw_flt_cnt:%d > DMX_PID_FLT_MAX_HW_FLT_EACH_DEV:%d",
              __FUNCTION__, __LINE__, total_hw_flt_cnt, 
              DMX_PID_FLT_MAX_HW_FLT_EACH_DEV);
    }

	flt_hw = NULL;

    for (flt_hw_idx = 0; flt_hw_idx < total_hw_flt_cnt; flt_hw_idx++)
    {
        flt_hw = &(hw_abstraction->flt_hw[flt_hw_idx]);

        if ((DMX_PID_FLT_HW_STATE_RUN == flt_hw->state) &&
			(flt_hw->pid == pid))
        {
            break;
        }
    }

    if (flt_hw_idx >= total_hw_flt_cnt)
    {
        /* Otherwise find an idle hw filter.
         */
        for (flt_hw_idx = 0; flt_hw_idx < total_hw_flt_cnt; flt_hw_idx++)
        {
            flt_hw = &(hw_abstraction->flt_hw[flt_hw_idx]);
        
            if (DMX_PID_FLT_HW_STATE_IDLE == flt_hw->state)
            {    
                flt_hw->pid = pid;
                
                flt_hw->state = DMX_PID_FLT_HW_STATE_RUN;

                INIT_LIST_HEAD(&(flt_hw->sw_run_list));

				/* Is this PID filter a special filter that collects all pakcet
				 * in HW buffer (for TP recording)?
				*/
                if (DMX_WILDCARD_PID == pid)
                {
                    /* Yes, bypass all HW filtering.
					*/                
					list_add_tail(&(flt_hw->link), &(hw_abstraction->tp_run_list));

					dmx_hw_bypass_enable(src_hw_interface_id);
				}
				else
				{
				    /* No, it is a normal pid filter.
			         */
					list_add_tail(&(flt_hw->link), &(hw_abstraction->hw_run_list));
			         
					/* Set pid to hw filter.
					 */
					dmx_hw_flt_pid_set(src_hw_interface_id, flt_hw_idx, pid);
					
					/* Enable hw filter.
					*/
					dmx_hw_flt_enable(src_hw_interface_id, flt_hw_idx);
				}

                break;
            }
        }
    }

    if (flt_hw_idx >= total_hw_flt_cnt)
    {
        return(DMX_ERR_PID_FLT_HW_EXHAUST);
    }

    list_add_tail(&(flt_sw->link), &(flt_hw->sw_run_list));
	
    flt_sw->pid_flt_cb = pid_flt_cb;
    flt_sw->cb_param = cb_param;
    flt_sw->src_hw_interface_id = src_hw_interface_id;
    flt_sw->flt_hw_id = flt_hw_idx;
	flt_sw->ts_in_cnt = 0;
    flt_sw->state = DMX_PID_FLT_SW_STATE_STOP;
    
    return(flt_sw_idx);
}




__s32 dmx_pid_flt_start
(
    __s32 flt_idx                 
)
{
    struct dmx_pid_flt_sw            *flt_sw;
    
    flt_sw = &(ali_dmx_pid_flt_module.flt_sw[flt_idx]);

    if (flt_sw->state != DMX_PID_FLT_SW_STATE_STOP)
    {
        return(DMX_ERR_PID_FLT_OPERATION_DENIED);
    }

    flt_sw->state = DMX_PID_FLT_SW_STATE_RUN;

    return(0);
}



__s32 dmx_pid_flt_stop
(
    __s32 flt_idx                 
)
{
    struct dmx_pid_flt_sw            *flt_sw;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    flt_sw = &(ali_dmx_pid_flt_module.flt_sw[flt_idx]);

    if (flt_sw->state != DMX_PID_FLT_SW_STATE_RUN)
    {        
        return(DMX_ERR_PID_FLT_OPERATION_DENIED);
    }

    flt_sw->state = DMX_PID_FLT_SW_STATE_STOP;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);
}



__s32 dmx_pid_flt_unregister
(
    __s32 flt_idx 
)
{
    struct dmx_pid_flt_sw            *flt_sw;
    struct dmx_pid_flt_hw            *flt_hw;
    struct dmx_pid_flt_hw_abstraction *hw_abstraction;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    flt_sw = &(ali_dmx_pid_flt_module.flt_sw[flt_idx]);

    if (DMX_PID_FLT_SW_STATE_IDLE == flt_sw->state)
    {
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        return(DMX_ERR_PID_FLT_OPERATION_DENIED);
    }

    list_del(&(flt_sw->link));

    hw_abstraction = &(ali_dmx_pid_flt_module.hw_abstraction[flt_sw->src_hw_interface_id]);

    flt_hw = &(hw_abstraction->flt_hw[flt_sw->flt_hw_id]);

    /* Disable hw filter if no sw filter linked to it.
    */
    if (list_empty(&(flt_hw->sw_run_list)))
    {
		/* Is this PID filter a special filter that collects all pakcet in
		 * HW buffer (for TP recording)?
		*/
        if (DMX_WILDCARD_PID == flt_hw->pid)
        {
            /* Yes, disable HW filtering bypassing.
			*/
		    dmx_hw_bypass_disable(flt_sw->src_hw_interface_id);
		} 
		else
		{
		    /* No, it is a normal pid filter, just clost HW filter.
			*/
			dmx_hw_flt_disable(flt_sw->src_hw_interface_id, flt_sw->flt_hw_id);	
		}
            
        /* Remove hw filter from active hw filter list.
        */
        list_del(&(flt_hw->link));
        
        flt_hw->state = DMX_PID_FLT_HW_STATE_IDLE;
    }
    
    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    flt_sw->state = DMX_PID_FLT_SW_STATE_IDLE;

    return(0);
} 



__u32 dmx_pid_flt_ts_in_cnt_get
(
    __s32 flt_idx                 
)
{
    return(ali_dmx_pid_flt_module.flt_sw[flt_idx].ts_in_cnt);
}





__s32 dmx_pid_flt_module_init
(
    void
)
{
    __s32                             hw_idx;
    struct dmx_pid_flt_hw_abstraction *hw_abstraction;
    __u32                             hw_flt_idx;
    struct dmx_pid_flt_hw            *hw_flt;
    __u32                             sw_flt_idx;
    struct dmx_pid_flt_sw            *sw_flt;
    
    memset(&ali_dmx_pid_flt_module, 0, sizeof(struct dmx_pid_flt_module));
    
    for (hw_idx = 0; hw_idx < DMX_PID_FLT_MAX_DEV_CNT; hw_idx++)
    {
        hw_abstraction = &(ali_dmx_pid_flt_module.hw_abstraction[hw_idx]);
        
        INIT_LIST_HEAD(&(hw_abstraction->hw_run_list));

        INIT_LIST_HEAD(&(hw_abstraction->tp_run_list));

        for (hw_flt_idx = 0; hw_flt_idx < DMX_PID_FLT_MAX_HW_FLT_EACH_DEV; 
             hw_flt_idx++)
        {
            hw_flt = &(hw_abstraction->flt_hw[hw_flt_idx]);

            hw_flt->state = DMX_PID_FLT_HW_STATE_IDLE;

            INIT_LIST_HEAD(&(hw_flt->sw_run_list));
        }
    }

    for (sw_flt_idx = 0; sw_flt_idx < DMX_PID_FLT_MAX_CNT; sw_flt_idx++)
    {
        sw_flt = &(ali_dmx_pid_flt_module.flt_sw[sw_flt_idx]);
    
        sw_flt->state = DMX_PID_FLT_SW_STATE_IDLE;
    }

    return(0);
}





