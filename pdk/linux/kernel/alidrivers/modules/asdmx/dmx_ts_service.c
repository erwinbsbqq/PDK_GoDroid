#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <timedoctor_common.h>
#include <linux/features.h>

#include "dmx_internal.h"

extern struct dmx_see_device g_dmx_see_devices[1];

__u16 g_dmx_check_pid = 0; 
__u32 g_dmx_check_count = 0xffffffff;

/*****************************************************************************/
/**
*
* Delete a hw filter from a hw filter list.
*
* @param
* - list: A pointer to struct dmx_pid_flt_list which contains head and tail
*         information of the hw filter list.
*
* - filter: A pointer to struct dmx_pid_flt to be deleted from the hw 
*           filter list
*
* @note
*
* @return
* - RET_SUCCESS: hw filter was successfully removed from ts ts_service list.
*
* - RET_FAILURE: At least one of the parameters was illegal.
*
******************************************************************************/
DMX_INT32 dmx_pid_flt_list_add_node
(
    struct dmx_services *services,
    struct dmx_pid_flt  *filter
)
{
    struct dmx_pid_flt_list *list;

    if (NULL == filter)
    {
        return(RET_FAILURE);
    }

    list = &(services->pid_flt_run_list);

    /* If empty list */
    if ((NULL == list->head) && (NULL == list->tail))
    {
        filter->next = NULL;
        filter->pri  = NULL;
       
        list->head = filter;
        list->tail = filter;

#if 0
        if (DMX_PARSE_TASK_STATUS_IDLE == priv->parse_task_status)
        {
            priv->parse_task_status = DMX_PARSE_TASK_STATUS_PLAY;

            osal_flag_set(priv->parse_task_ctrl_flag, DMX_PARSE_TASK_CMD_START);
        }
#endif

    }    
    /* Insert as tail node */
    else
    {
        filter->next = NULL;
        filter->pri  = list->tail;

        list->tail->next = filter;

        list->tail = filter;
    }

    return(RET_SUCCESS);
}


/*****************************************************************************/
/**
*
* Delete a hw filter from a hw filter list.
*
* @param
* - list: A pointer to struct dmx_pid_flt_list which contains head and tail
*         information of the hw filter list.
*
* - filter: A pointer to struct dmx_pid_flt to be deleted from the hw 
*           filter list
*
* @note
*
* @return
* - RET_SUCCESS: hw filter was successfully removed from ts ts_service list.
*
* - RET_FAILURE: At least one of the parameters was illegal.
*
******************************************************************************/
DMX_INT32 dmx_pid_flt_list_del_node
(
    struct dmx_services *services,
    struct dmx_pid_flt  *filter
)
{
    struct dmx_pid_flt_list *list;

    if (NULL == filter)
    {
        return(RET_FAILURE);
    }

    list = &(services->pid_flt_run_list);

    /* Del the specified pid_flt in the list */

    /* The last one node left in the queue */
    if ((filter == list->head) && (filter == list->tail))
    {
        list->head = NULL;
        list->tail = NULL;

#if 0
        if (DMX_PARSE_TASK_STATUS_PLAY == priv->parse_task_status)
        {
            priv->parse_task_status = DMX_PARSE_TASK_STATUS_IDLE;
        }
#endif
    }
    /* The head node */
    else if(filter == list->head)
    {
        filter->next->pri = NULL;
        list->head = filter->next;
    }
    /* The tail node */
    else if (filter == list->tail)
    {
        filter->pri->next = NULL;
        list->tail = filter->pri;
    }
    /* In the middle */
    else
    {
        filter->pri->next = filter->next;
        filter->next->pri = filter->pri;
    }

    return(RET_SUCCESS);              
}


/*****************************************************************************/
/**
*
* Add a node to the ts ts_service list in ts ts_service idx increase order.
*
* @param
* - struct dmx_ts_service_list *list is a pointer to struct dmx_ts_service_list
*   which contains head and tail information of the ts ts_service list.
*
* - struct dmx_ts_service *ts_service is a pointer to struct dmx_ts_service which
*   contains all information needed by a ts ts_service.
*
* @note
*
* @return
* - RET_SUCCESS: the ts_service is legal and was successfully inserted into 
*   ts ts_service list.
*
* - RET_FAILURE: the ts_service is illegal and was not inserted into ts ts_service
*   list.
*
******************************************************************************/
DMX_INT32 dmx_ts_service_list_add_node
(
    struct dmx_ts_service_list *list,
    struct dmx_ts_service      *ts_service
)
{
    if (NULL == ts_service)
    {
        return(RET_FAILURE);
    }

    if (NULL == list)
    {
        return(RET_FAILURE);
    }

    /* If empty list */
    if ((NULL == list->head) && (NULL == list->tail))
    {
        ts_service->next = NULL;
        ts_service->pri  = NULL;
       
        list->head = ts_service;
        list->tail = ts_service;
    }    
    /* Insert as tail node */
    else
    {
        ts_service->next = NULL;
        ts_service->pri  = list->tail;

        list->tail->next = ts_service;

        list->tail = ts_service;
    }

    return(RET_SUCCESS);
}




/*****************************************************************************/
/**
*
* Delete a node from the ts ts_service list.
*
* @param
* - struct dmx_ts_service_list *list is a pointer to struct dmx_ts_service_list
*   which contains head and tail information of the ts ts_service list.
*
* - struct dmx_ts_service *ts_service is a pointer to struct dmx_ts_service which
*   contains all information needed by a ts ts_service.
*
* @note
*
* @return
* - RET_SUCCESS: the ts_service is legal and was successfully removed from ts
*   ts_service list.
*
* - RET_FAILURE: the ts_service is illegal and nothing was done to the ts 
*   servic list.
*
******************************************************************************/
DMX_INT32 dmx_ts_service_list_del_node
(
    struct dmx_ts_service_list *list,
    struct dmx_ts_service      *ts_service
)
{
    if (NULL == list)
    {
        return(RET_FAILURE);
    }

    if (NULL == ts_service)
    {
        return(RET_FAILURE);
    }

    /* Del the specified ts_service in the list */

    /* The last node left in the queue */
    if ((ts_service == list->head) && (ts_service == list->tail))
    {
        list->head = NULL;
        list->tail = NULL;
    }
    /* The head node */
    else if(ts_service == list->head)
    {
        ts_service->next->pri = NULL;
        list->head = ts_service->next;
    }
    /* The tail node */
    else if (ts_service == list->tail)
    {
        ts_service->pri->next = NULL;
        list->tail = ts_service->pri;
    }
    /* In the middle */
    else
    {
        ts_service->pri->next = ts_service->next;
        ts_service->next->pri = ts_service->pri;
    }

    return(RET_SUCCESS);              
}


/*****************************************************************************/
/**
*
* Get a struct dmx_ts_service pointer to the head of a ts server list.
*
* @param
* - struct dmx_ts_service_list *list is a pointer to struct dmx_ts_service_list
*   which contains head and tail information of the ts ts_service list.
*
* @note
*
* @return
* - struct dmx_ts_service*: the head node of the ts server list. 
*
* - NULL: the ts server list is empty.
*
******************************************************************************/
struct dmx_ts_service* dmx_ts_service_list_peek_head
(
    struct dmx_ts_service_list *list
)
{
    if (NULL == list)
    {
        return(NULL);
    }

    return(list->head);
}



/*****************************************************************************/
/**
*
* Get a hw filter from a idle hw filter list.
*
* @param
* - dev: A pointer to the demux to which the hw filters is to be got.
*
* - pid: The PID for the hw filter.
*
* @note
*
* @return
* - DMX_INVALID_HW_FILTER_IDX: No free ts filter left for use.
*
* - Otherwise: The index of the new got hw filer.
*
******************************************************************************/
DMX_UINT32 dmx_pid_flt_register
(
    struct dmx_device *dmx,
    DMX_UINT16         pid
)
{
    DMX_UINT32           pid_flt_idx;
    struct dmx_pid_flt  *pid_flt;
    struct dmx_services *services;

    services = &dmx->services;

    /* Find a ready or run hw filter. */
    for (pid_flt_idx = 0; pid_flt_idx < DMX_HW_TOTAL_FILTERS; pid_flt_idx++)
    {
        pid_flt = &(services->pid_flt[pid_flt_idx]);

        if (((DMX_PID_FLT_STATUS_READY == pid_flt->status)  || 
            (DMX_PID_FLT_STATUS_RUN    == pid_flt->status)) &&
            (pid_flt->pid == pid))
        {
            return(pid_flt_idx);
        }
    }

    /* Otherwise find an idle hw filter. */
    for (pid_flt_idx = 0; pid_flt_idx < DMX_HW_TOTAL_FILTERS; pid_flt_idx++)
    {
        pid_flt = &(services->pid_flt[pid_flt_idx]);

        if (DMX_PID_FLT_STATUS_IDLE == pid_flt->status)
        {    
            pid_flt->status = DMX_PID_FLT_STATUS_READY;

            break;
        }
    }
   
    if (pid_flt_idx >= DMX_HW_TOTAL_FILTERS)
    {
        return(DMX_INVALID_IDX);
    }

    pid_flt->pid  = pid;
    pid_flt->next = NULL;
    pid_flt->pri  = NULL;

    return(pid_flt_idx);
}






/*****************************************************************************/
/**
*
* Enable a hw filter.
*
* @param
* - dev: A pointer to the demux to which the hw filters is to be enabled.

*
* - idx: The index the hw filter.
*
* @note
*
* @return
* - DMX_INVALID_HW_FILTER_IDX: No free ts filter left for use.
*
* - Otherwise: The index of the new got hw filer.
*
******************************************************************************/
DMX_INT32 dmx_pid_flt_enable
(
    struct dmx_device     *dmx,
    DMX_UINT32             idx,
    struct dmx_ts_service *ts_service
)
{
    struct dmx_services *services;
    struct dmx_pid_flt  *pid_flt;

    services = &dmx->services;

    pid_flt = &(services->pid_flt[idx]);

    if ((DMX_PID_FLT_STATUS_READY != pid_flt->status)  &&
        (DMX_PID_FLT_STATUS_RUN   != pid_flt->status))
    {
        return(RET_FAILURE);   
    }

    dmx_ts_service_list_add_node(&pid_flt->ts_service_list, ts_service);

    if (DMX_PID_FLT_STATUS_READY == pid_flt->status)
    {
        pid_flt->status = DMX_PID_FLT_STATUS_RUN; 

        dmx_pid_flt_list_add_node(services, pid_flt);

        if (DMX_TYPE_HW == dmx->dmx_type)
        {
            /* Set pid to hw filter(M3603 dmx hw dmx has a a bug that mask
             * can not fit value exactly). 
             */
            //DMX_HW_FLT_PID_SET(dmx->base_addr, idx, pid_flt->pid & 0x1FFF);
            dmx->pfunc.dmx_hw_flt_pid_set(dmx->base_addr, idx, pid_flt->pid & 0x1FFF);
    
            /* Enable hw filter */
            //DMX_HW_FLT_ENABLE(dmx->base_addr, idx);
            dmx->pfunc.dmx_hw_flt_enable(dmx->base_addr, idx);
        }
    }

    return(RET_SUCCESS);   
}




/*****************************************************************************/
/**
*
* Disable a hw filter.
*
* @param
* - dev: A pointer to the demux to which the hw filters is to be disabled.

*
* - idx: The index the hw filter.
*
* @note
*
* @return
* - RET_SUCCESS: hw filter was successfully disabled.
*
* - RET_FAILURE: the hw filter idx is illegal.
*
******************************************************************************/
DMX_INT32 dmx_pid_flt_unregister
(
    struct dmx_device     *dmx,
    DMX_UINT32             idx,
    struct dmx_ts_service *ts_service
)
{
    struct dmx_services *services;
    struct dmx_pid_flt  *pid_flt;

    services = &dmx->services;

    pid_flt = &(services->pid_flt[idx]);

    if (DMX_PID_FLT_STATUS_IDLE != pid_flt->status)
    {
        /* Remove hooked ts filter from running hw filter. */
        dmx_ts_service_list_del_node(&pid_flt->ts_service_list, ts_service);
    
        /* Close hw filter and remove it from running list. */
        if (NULL == dmx_ts_service_list_peek_head(&pid_flt->ts_service_list))
        {    
            dmx_pid_flt_list_del_node(services, pid_flt);

			if (DMX_TYPE_HW == dmx->dmx_type)
            {
                //DMX_HW_FLT_DISABLE(dmx->base_addr, idx);
                dmx->pfunc.dmx_hw_flt_disable(dmx->base_addr, idx);
            }
    
            pid_flt->status = DMX_PID_FLT_STATUS_IDLE;
        }
    }

    return(RET_SUCCESS);   
} 






/*****************************************************************************/
/**
*
* Register a ts ts_service to a demux driver.
*
* @param
* - dev: A pointer to the demux to which the ts ts_service is to be registered.
*
* - DMX_UINT16 pid: The PID for the ts ts_service.
*
* - ali_dmx_ts_cb: A function used by ts ts_service layer to send TS pakcet to
*   upper layer. 
*   This function takes 4 parameters:
*    dev is a pointer to the demux to which the ts ts_service is registered,
*    *ts_buf is a pointer to a struct dmx_ts_buf_t which contains all the
*     information of the current TS pakcet.
*    param_1 is an extra parameter.
*    param_2 is another extra parameter.
*
* - param_1 is an extra parameter which will be passed to ali_dmx_ts_cb.
* - param_2 is another extra parameter which will be passed to ali_dmx_ts_cb.

* @note
*
* @return
* - DMX_INVALID_ts_service_IDX: the TS ts_service was not registered successfully
*   due to invalid parameter or insufficent hw filter.
*
* - Otherwise: the ts_service was successfully registered and the ts ts_service
*   index is returned.
*
******************************************************************************/
DMX_UINT32 dmx_ts_service_register
(
    struct dmx_device  *dmx,
    DMX_UINT16          pid,
    DMX_INT32         (*ali_dmx_ts_cb)(void                  *dmx,
                                       struct dmx_ts_pkt_inf *pkt_inf,
                                       DMX_UINT32             param),
    DMX_UINT32          param
)
{
    DMX_UINT32             pid_flt_idx;
    DMX_UINT32             idx;
    struct dmx_ts_service *ts_service;
    struct dmx_services   *services;

    //printk("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, pid);

    services = &dmx->services;

    /* Find a idle ts ts_service slot. */
    for (idx = 0; idx < DMX_TOTAL_TS_SERVICE; idx++)
    {
        ts_service = &(services->ts_service[idx]);

        if (DMX_TS_SERVICE_STATUS_IDLE == ts_service->status)
        {
            break;
        }
    }

    /* All ts ts_service slot occupied. */
    if (idx >= DMX_TOTAL_TS_SERVICE)
    {
        return(DMX_INVALID_IDX);
    }

    /* All ts ts_service slot occupied. */
    pid_flt_idx = dmx_pid_flt_register(dmx, pid);

    if (DMX_INVALID_IDX == pid_flt_idx)
    {
        return(DMX_INVALID_IDX);
    }

    memset(ts_service, 0, sizeof(struct dmx_ts_service));

    /* Keep track of ts filter for this ts_service. */        
    ts_service->pid = pid;

    ts_service->ali_dmx_ts_cb = ali_dmx_ts_cb;

    ts_service->param = param;

    ts_service->pid_flt_idx = pid_flt_idx;

    ts_service->status = DMX_TS_SERVICE_STATUS_READY;

    return(idx); 
}


/*****************************************************************************/
/**
*
* Enable a ts ts_service identified by idx.
*
* @param
* - struct dmx_device *dev: A pointer to the demux to which the ts ts_service is 
*   registered.
*
* - idx: The index of the ts ts_service to be enalbed.
*
* @note
*
* @return
* - RET_SUCCESS: the ts_service is legal and was successfully enalbed.
*
* - RET_FAILURE: the ts_service is illegal and nothing was done.
*
******************************************************************************/
DMX_INT32 dmx_ts_service_enable
(
    struct dmx_device *dmx,
    DMX_UINT32         idx
)
{
    struct dmx_ts_service *ts_service;
    struct dmx_services   *services;

    services = &dmx->services;

    ts_service = &(services->ts_service[idx]);

    if (DMX_TS_SERVICE_STATUS_READY == ts_service->status)
    {
        ts_service->status = DMX_TS_SERVICE_STATUS_RUN;
    
        dmx_pid_flt_enable(dmx, ts_service->pid_flt_idx, ts_service);
    }

    return(RET_SUCCESS); 
}





/*****************************************************************************/
/**
*
* Unregister a ts ts_service identified by idx.
*
* @param
* - struct dmx_device *dev: A pointer to the demux to which the ts ts_service is 
*   registered.
*
* - idx: The index of the ts ts_service to be unregistered.

* @note
*
* @return
* - RET_SUCCESS: the ts_service is legal and was successfully unregistered.
*
* - RET_FAILURE: the ts_service is illegal and nothing was done.
*
******************************************************************************/
DMX_INT32 dmx_ts_service_unregister
(
    struct dmx_device *dmx,
    DMX_UINT32         idx
)
{
    struct dmx_ts_service *ts_service;
    struct dmx_services   *services;

    services = &dmx->services;

    ts_service = &(services->ts_service[idx]);

    DMX_TS_SERV_DEBUG("%s, idx:%d, pid:%d\n", __FUNCTION__, idx, 
                      ts_service->pid);

    if (DMX_TS_SERVICE_STATUS_IDLE != ts_service->status)
    {
        dmx_pid_flt_unregister(dmx, ts_service->pid_flt_idx, ts_service);
    
        ts_service->status = DMX_TS_SERVICE_STATUS_IDLE;
    }

    return(RET_SUCCESS); 
}







__inline DMX_INT32 dmx_ts_pkt_info_retrieve
(
    struct dmx_device     *dmx,
    struct dmx_ts_pkt_inf *pkt_inf
)
{  
    if ((pkt_inf->pkt_addr[0] != 0x47) && (pkt_inf->pkt_addr[0] != 0xAA))
    {     
        printk("%s, %d\n", __FUNCTION__, __LINE__);

        dmx->pkt_total_sync_erro_cnt++;

        return(RET_FAILURE);
    }

    /* TS pakcet error indicator. */
    if (((pkt_inf->pkt_addr[1] & 0x80) >> 7) != 0)
    { 
        dmx->pkt_total_erro_cnt++;

        return(RET_FAILURE);
    }

    pkt_inf->unit_start = (pkt_inf->pkt_addr[1] & 0x40) >> 6;
 
    pkt_inf->pid = ((pkt_inf->pkt_addr[1] & 0x1F) << 8) | pkt_inf->pkt_addr[2];

    pkt_inf->scramble_flag = (pkt_inf->pkt_addr[3] & 0xC0) >> 6;

    pkt_inf->conti_cnt = pkt_inf->pkt_addr[3] & 0x0F;

    //printk("conti_cnt:%d\n", pkt_inf->conti_cnt);

    /* TS pakcet scramble flag. */
    pkt_inf->adapt_ctrl = (pkt_inf->pkt_addr[3] & 0x30) >> 4;

    /* Locate adaptation field and payload */
    if ((TS_ADAPT_ONLY        ==  pkt_inf->adapt_ctrl) ||
        (TS_ADAPT_AND_PAYLOAD == pkt_inf->adapt_ctrl))
    {
        pkt_inf->adapt_addr = pkt_inf->pkt_addr + 4;

        pkt_inf->adapt_len = pkt_inf->pkt_addr[4] + 1;

        pkt_inf->payload_addr = pkt_inf->adapt_addr + pkt_inf->adapt_len;

        pkt_inf->payload_len = 184 - pkt_inf->adapt_len;

        if (0 != pkt_inf->adapt_len)
        {
            pkt_inf->disconti_flag = ((pkt_inf->adapt_addr[1] & 0x40) >> 6);

			if (feature_is_timedoctor_enable())
			{
				if(pkt_inf->adapt_len > 1) //pkt_inf->adapt_len may be 0
	            {
		            unsigned char discontinuity_indicator;
			        unsigned char PCR_flag;
				    unsigned long curpcr;

					discontinuity_indicator = (unsigned char)((pkt_inf->adapt_addr[1] & 0x80) >> 7);
	                if(discontinuity_indicator) 
		                td_genericEvent(GE_PCR_DISCONTI + (dmx->data_src << 13) + pkt_inf->pid);
			        PCR_flag = (unsigned char)((pkt_inf->adapt_addr[1] & 0x10) >> 4);
				    if(PCR_flag){
					    td_genericEvent(GE_ID_PCR + (dmx->data_src << 13) + pkt_inf->pid);
						//pcr delta checking, pcr convert to ms: program_clock_reference_base:32bits / 45
	                    curpcr = ((pkt_inf->adapt_addr[2] << 24) & 0xff000000) | \
		                        ((pkt_inf->adapt_addr[3] << 16) & 0xff0000) | \
			                    ((pkt_inf->adapt_addr[4] << 8) & 0xff00) | \
				                ((pkt_inf->adapt_addr[5]) & 0xff);
					    curpcr /= 45;
						td_setVal((dmx->data_src << 13) + pkt_inf->pid, curpcr);
	                }
		        }
			}
        }
        else
        {
            pkt_inf->disconti_flag = 0;
        }
    }
    else
    {
        pkt_inf->adapt_addr = NULL;

        pkt_inf->adapt_len = 0;

        pkt_inf->payload_addr = pkt_inf->pkt_addr + 4;

        pkt_inf->payload_len = 184;

        pkt_inf->disconti_flag = 0;
    }

    return(RET_SUCCESS);
}






__inline DMX_INT32 dmx_ts_service_chk_ts_pkt
(
    struct dmx_ts_service *ts_service,
    struct dmx_ts_pkt_inf *pkt_inf,
    struct dmx_device     *dmx
)
{
    DMX_UINT32 conti_cnt_expect;

    ts_service->pkt_in_cnt++;

    if (pkt_inf->scramble_flag != 0)
    {
        ts_service->scram_cnt++;
    }

    pkt_inf->continuity = DMX_TS_PKT_CONTINU_OK;    

    /* Check duplicated TS packets.*/
    if ((pkt_inf->conti_cnt == ts_service->last_ts_conti_cnt)         &&
        (pkt_inf->adapt_ctrl == ts_service->last_ts_adapt_ctrl)       &&
        (pkt_inf->adapt_len == ts_service->last_ts_adapt_len)         &&
        (pkt_inf->payload_len == ts_service->last_ts_payload_len)     &&
        (pkt_inf->scramble_flag == ts_service->last_ts_scramble_flag) &&
        (pkt_inf->priority == ts_service->last_ts_priority)           &&
        (pkt_inf->unit_start == ts_service->last_ts_unit_start))

    {
        pkt_inf->continuity = DMX_TS_PKT_CONTINU_DUPLICATE; 

        ts_service->duplicate_cnt++;

        //printk("%s,%d,pid:%d,conti:%d\n", __FUNCTION__, __LINE__, 
               //pkt_inf->pid, pkt_inf->conti_cnt);
    }

    /* Check TS packet continuty only if current TS pakcet contains no PUSI
     * (Packet Unit Start Indicator) bit. This restriction is to keep 
     * compartibility with some streams which does not follow the
     * iso-13818.1 restricly.
     */
    else if ((0 == pkt_inf->unit_start) && (0 == pkt_inf->disconti_flag))
    {
        conti_cnt_expect = ts_service->last_ts_conti_cnt + 1;
    
        if (conti_cnt_expect > 0xF)
        {
            conti_cnt_expect = 0;
        }
    
        if (conti_cnt_expect != pkt_inf->conti_cnt)
        {
            pkt_inf->continuity = DMX_TS_PKT_CONTINU_LOST;

            ts_service->discon_cnt++;
            if(feature_is_timedoctor_enable() && \
			   ts_service->pid > 0x20) //non si/psi pid
                td_genericEvent(GE_TS_DISCONTI + (dmx->data_src << 13) + ts_service->pid);
#if 0
            if (pkt_inf->conti_cnt != ts_service->last_ts_conti_cnt)
            {
                printk("%s,%d,pid:%d,exp:%d,cur:%d\n", __FUNCTION__, __LINE__,
                       pkt_inf->pid, conti_cnt_expect, pkt_inf->conti_cnt);
            }
#endif
        }
    }

    /* Prepare for next TS packet checking. */
    ts_service->last_ts_unit_start = pkt_inf->unit_start;
    ts_service->last_ts_priority = pkt_inf->priority;
    ts_service->last_ts_scramble_flag = pkt_inf->scramble_flag;
    ts_service->last_ts_adapt_ctrl = pkt_inf->adapt_ctrl;
    ts_service->last_ts_conti_cnt = pkt_inf->conti_cnt;
    ts_service->last_ts_adapt_len = pkt_inf->adapt_len;
    ts_service->last_ts_payload_len = pkt_inf->payload_len;

    return(RET_SUCCESS);
}




__s32 dmx_playback_ts_flag_pkt_handle
(
    struct dmx_device     *dmx,
    struct dmx_ts_pkt_inf *pkt_inf
)
{
    if (0xAA != pkt_inf->pkt_addr[0])
    {
        return(RET_FAILURE);
    }

    dmx_see_buf_wr_ts(dmx, pkt_inf, (DMX_UINT32)(&(g_dmx_see_devices[0])));

    return(RET_SUCCESS);
}



/*****************************************************************************/
/**
*
* Parse a TS pakcet and retrieve its information, locate its adaptation field
* and payload.
*
* @param
* - ts_buf is a struct which contains all neccessary info about a TS pakcet.
*
* @note
*
* @return
* - RET_FAILURE the TS pakcet is not processed due to upper layer delay and
*   should be processed again in next run, the ts buffer should not be released.
*
* - RET_SUCCESS the TS pakcet is processed and the ts buffer can be released.
*
******************************************************************************/
__s32 dmx_ts_service_parse
(
    struct dmx_device     *dmx,
    struct dmx_ts_pkt_inf *pkt_inf
)
{
    DMX_INT32              ret;
    struct dmx_pid_flt    *pid_flt;
    struct dmx_ts_service *ts_service;
    struct dmx_services   *services;

    /* Handle sepecial flag TS pakcet for FF/FB in PVR. */
    ret = dmx_playback_ts_flag_pkt_handle(dmx, pkt_inf);

    if (RET_SUCCESS == ret)
    {
        return(RET_SUCCESS);
    }

    services = &dmx->services;

    //ret = dmx_ts_pkt_info_retrieve(services, pkt_inf);

    ret = dmx_ts_pkt_info_retrieve(dmx, pkt_inf);

    if (RET_SUCCESS != ret)
    {
        return(ret);
    }

    /* Loop through all alive pid filters to find a march with
     * current TS pakcet PID.
     */            
    for(pid_flt = services->pid_flt_run_list.head;
        pid_flt != NULL; pid_flt = pid_flt->next)
    {
        if (pkt_inf->pid == pid_flt->pid)
        {
            /* Send TS packet to upper layer handler. */
            for (ts_service = pid_flt->ts_service_list.head;
                 NULL != ts_service; ts_service = ts_service->next)
            {
                if (NULL != ts_service->ali_dmx_ts_cb)
                {
                    dmx_ts_service_chk_ts_pkt(ts_service, pkt_inf, dmx);

                    dmx->last_parsed_pid = pkt_inf->pid;


                    ts_service->ali_dmx_ts_cb(dmx, pkt_inf, ts_service->param);
                }
            }
            /* No need to check the rest hw filters since demux hw
             * does not support multi hit, we provide multi hit by
             * the for loop above.
             */
            return(RET_SUCCESS);
        }
    } 

    /* No sw TS filer hit, the sw filter for this PID must have
     * been closed or we got an illegal TS packet.
     */
    return(RET_FAILURE);
}



DMX_INT32 dmx_ts_service_init
(
    struct dmx_device *dmx
)
{
    DMX_UINT32           i;
    struct dmx_services *services;
    services = &dmx->services;
    for (i = 0; i < DMX_HW_TOTAL_FILTERS; i++)
    {
        services->pid_flt[i].status = DMX_PID_FLT_STATUS_IDLE;
    }
    for (i = 0; i < DMX_TOTAL_TS_SERVICE; i++)
    {
        services->ts_service[i].status = DMX_TS_SERVICE_STATUS_IDLE;
    }
    return(RET_SUCCESS);
}

void dmx_check_ts_to_user(void *buf, __u32 len)
{
    __u16 pid;
    __u8 *ts_buf = (__u8 *)buf;
    static __u32 last_addr;
    static __u8 cnt = 0;
     int i;
    
    for(i = 0; i < len; i++, ts_buf+=188)
    {
        if(ts_buf[0] != 0x47 && ts_buf[0] != 0xaa)
        {
            printk("%s  sync byte found \n", __FUNCTION__);
        }
        pid = ((ts_buf[1]&0x1f)<<8)|ts_buf[2];
        if(pid == g_dmx_check_pid)
        {
            if(((cnt + 1)&0xf) != (ts_buf[3]&0xf)) 
            {
                printk("%s No continue, cnt = %x %x address = %x %x\n",__FUNCTION__, cnt, (ts_buf[3]&0xf), last_addr, buf);
                g_dmx_check_count--;
                if(g_dmx_check_count == 0)
                    asm(".word 0x7000003f;");
            }
            cnt = (ts_buf[3]&0xf);
            last_addr= buf;
        }
    }
}

void dmx_check_ts_to_kernel(void *buf, __u32 len)
{
    __u16 pid;
    __u8 *ts_buf = (__u8 *)buf;
    static __u32 last_addr;
    static __u8 cnt = 0;
    int i;
    
    for(i = 0; i < len; i++, ts_buf+=188)
    {
        if(ts_buf[0] != 0x47 && ts_buf[0] != 0xaa)
        {
            printk("%s  sync byte found \n", __FUNCTION__);
        }
        pid = ((ts_buf[1]&0x1f)<<8)|ts_buf[2];
        if(pid == g_dmx_check_pid)
        {
            if(((cnt + 1)&0xf) != (ts_buf[3]&0xf))
                printk("%s No continue, cnt = %x %x address = %x %x\n",__FUNCTION__, cnt, (ts_buf[3]&0xf), last_addr, ts_buf);
            cnt = (ts_buf[3]&0xf);
            last_addr= ts_buf;
        }
    }
}