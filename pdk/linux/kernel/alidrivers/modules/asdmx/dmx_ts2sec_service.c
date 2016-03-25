
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/delay.h>



#include "dmx_internal.h"


DMX_INT32 dmx_ts2sec_is_mask_hit
(
    struct dmx_device         *dmx,
    struct dmx_ts2sec_service *service
)
{
    DMX_UINT32 i;

    for (i = 0; i < service->mask_len; i++)
    {
        if ((service->mask[i] & service->buf[i]) != 
            (service->mask[i] & service->value[i]))
        {
            return(0);
        }
    }

    /* Do not do mask compartion in other cases. */
    return(1);
}




DMX_INT32 dmx_ts2sec_drop_cur_pkt
(
    struct dmx_device         *dmx,
    struct dmx_ts2sec_service *service,
    struct ali_dmx_data_buf   *dest_buf
)
{
    //printk("%s,%d,err:%d\n", __FUNCTION__, __LINE__, service->err_code);

    if (DMX_TS2SEC_SERVICE_STATUS_REMAIN_TS == service->status)
    {
        dmx_data_buf_kern_flush_last_pkt(dmx, dest_buf);
    }

    return(RET_SUCCESS);
}

__s32 dmx_ts2sec_wr_pkt_data
(
    struct dmx_device         *dmx,
    struct dmx_ts2sec_service *service,
    __u8                      *src,
    __s32                      len
)
{
    __s32                    ret;
    struct ali_dmx_data_buf *dest_buf;

    dest_buf = (struct ali_dmx_data_buf *)service->param;

    //printk("%s,%d,sev:%x,dst:%x,src:%x,len:%d\n", 
           //__FUNCTION__, __LINE__, service, dest_buf, src, service->sec_len);

    ret = dmx_data_buf_kern_wr_data(dmx, dest_buf, src, len);

    if (ret < len)
    {
        dmx_data_buf_kern_flush_all_pkt(dmx, dest_buf);

        printk("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, service->pid);
    }
    else
    {
        service->sec_copied_len += ret;
    }

    return(ret);
}

__s32 dmx_ts2sec_wr_pkt_end
(
    struct dmx_device         *dmx,
    struct dmx_ts2sec_service *service,
    __u8                      *src,
    __s32                      len
)
{
    __s32 ret;

    struct ali_dmx_data_buf *dest_buf;

    ret = dmx_ts2sec_wr_pkt_data(dmx, service, src, len);

    /* If pakcet is successfully stored in memory. */
    if (ret == len)
    {
       dest_buf = (struct ali_dmx_data_buf *)service->param;
   
       dmx_data_buf_kern_wr_pkt_end(dmx, dest_buf);
    }

    return(ret);
}

/*****************************************************************************/
/**
*
* Copy TS packet into PES buffer, update PES buffer to upper layer if
* neccessary.
*
* @param
* - dmx: A pointer to the demux to which the pes service is registered.
*
* - ts_buf: A pointer to a struct dmx_ts_buf_t which contains all the
*           information of the current TS pakcet.
*
* - param1: A pointer to struct dmx_ts2es_service which repsents a PES data path.
*
* - parm_2: Not used.
*
* @note
* This function was implemented as an TS service call back. This function 
* receive TS pakcet from TS service layer and copy it into PES buffer provided
* by upper layer. 
*
* @return
* - NONE
*
******************************************************************************/
__s32 dmx_ts2sec_parse
(
    void                  *dmx,
    struct dmx_ts_pkt_inf *ts_pkt_inf,
    DMX_UINT32             param 
)
{
    DMX_UINT8                  sec_pointer;
    DMX_UINT32                 ts_remain_len;
    DMX_UINT32                 sec_remain_len;
    DMX_UINT8                 *ts_payload;
    DMX_UINT8                 *ts_end_addr;
    DMX_UINT8                 *rd_addr;
    struct dmx_ts2sec_service *service;
    DMX_UINT8                 *sec_start;
    DMX_UINT32                 sec_data_len;
    struct ali_dmx_data_buf   *dest_buf;
    __s32                      ret;

    ret = 0;

    service = (struct dmx_ts2sec_service *)param;

    if ((DMX_TS2SEC_SERVICE_STATUS_REMAIN_TS != service->status) &&
        (DMX_TS2SEC_SERVICE_STATUS_FIRST_TS  != service->status))
    {
        return(-1);
    }

    if ((DMX_TS_PKT_CONTINU_DUPLICATE == ts_pkt_inf->continuity) ||
        (0 == ts_pkt_inf->payload_len))
    {
        return(-2);
    }

    dest_buf = (struct ali_dmx_data_buf *)service->param;

    service->err_code = DMX_SEC_ERR_CODE_OK;

    /* ALI_DMX_DATA_PKT_DEBUG(" ====> %s %d %s \n", __func__, \
        ts_pkt_inf->scramble_flag, ((struct dmx_device *)dmx)->name); */
    
    if (0 != ts_pkt_inf->scramble_flag && \
        0 != strncmp(((struct dmx_device *)dmx)->name,"DMXPLAYBACK", 10))
    {
        service->err_code = DMX_SEC_ERR_CODE_TS_SCRAMBLED;

		dmx_ts2sec_drop_cur_pkt(dmx, service, dest_buf);

        service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

        return(-3);
    }

    if (DMX_TS_PKT_CONTINU_LOST == ts_pkt_inf->continuity)
    {
        service->err_code = DMX_SEC_ERR_CODE_DATA_LOST;

        dmx_ts2sec_drop_cur_pkt(dmx, service, dest_buf);

        service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

        return(-4);
    }

    ts_payload = ts_pkt_inf->payload_addr;

    /* In some rare cases sec data in privious TS pakcet is not
     * long enough to contain section length filed.
     */
    if ((service->partial_hdr_wr > 0) && 
        (DMX_TS2SEC_SERVICE_STATUS_REMAIN_TS == service->status))
    {
        if (0 == ts_pkt_inf->unit_start)
        {
            sec_start = ts_pkt_inf->payload_addr;

            sec_data_len = ts_pkt_inf->payload_len;
        }
        else
        {
            sec_start = ts_pkt_inf->payload_addr + 1;

            /* Pointer field. */
            sec_data_len = ts_pkt_inf->payload_addr[0];
        }

        /* sec data still not long enough, continue wait for enough data. */
        if (sec_data_len + service->partial_hdr_wr < 3)
        {
            if (0 == ts_pkt_inf->unit_start)
            {
                memcpy(service->partial_hdr + service->partial_hdr_wr, 
                       sec_start, sec_data_len);
			
                service->partial_hdr_wr += sec_data_len;
				
 				ret = dmx_ts2sec_wr_pkt_data(dmx, service, sec_start, sec_data_len);
                if (ret < sec_data_len)
                {
                    service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

                    return(-6);
                }

                return(sec_data_len);
            }
            else
            {
                service->err_code = DMX_SEC_ERR_CODE_SEC_UNFINISHED;

                dmx_ts2sec_drop_cur_pkt(dmx, service, dest_buf);
    
                service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

                return(-5);
            }
        }

        /* Enough data retrieved, now get sec len. */
        memcpy(service->partial_hdr + service->partial_hdr_wr, sec_start,
               3 - service->partial_hdr_wr);

        service->sec_len = (((service->partial_hdr[1] & 0x0F) << 8) | 
                              service->partial_hdr[2]) + 3;
        

        service->partial_hdr_wr = 0;

        //printk("%s,%d,sec len:%d\n", __FUNCTION__, __LINE__, service->sec_len);
    }

    /* TS packet with PUSI(payload_unit_start_indicator) == 0. */
    if (0 == ts_pkt_inf->unit_start)
    {
        if (DMX_TS2SEC_SERVICE_STATUS_REMAIN_TS == service->status)
        {
            sec_remain_len = service->sec_len - service->sec_copied_len;

            /* Case 1: Current section is finished within this TS pakcet. */
            if(sec_remain_len <= ts_pkt_inf->payload_len)
            {
                //printk("%s,%d,sl:%d,l:%d\n", __FUNCTION__, __LINE__, service->sec_len, sec_remain_len);

                ret = dmx_ts2sec_wr_pkt_end(dmx, service, ts_payload, 
                                            sec_remain_len);

                service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;
				
                /* No other secions coexists in this TS packet, 
                 * since PUSI == 0 means there should be no new section started
                 * in this TS packet.
                 */
                if (ret < sec_remain_len)
                {
                    return(-6);
                }

                return(ret);
            }
            /* Case 2: Current section is not finished within this TS packet,
             * copy all TS pakcet payload to section buffer and wait for next
             * TS packet.
             */
            else
            {
                //printk("%s,%d,sl:%d,l:%d\n", __FUNCTION__, __LINE__, service->sec_len, ts_pkt_inf->payload_len);

                ret = dmx_ts2sec_wr_pkt_data(dmx, service, ts_payload, 
                                             ts_pkt_inf->payload_len);

                /* Overflow. */
                if (ret < ts_pkt_inf->payload_len)
                {
                    service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

                    return(-6);
                }
            }
        }

        return(ret);
    }

    /* Otherwise TS packet with PUSI(payload_unit_start_indicator) == 1. */
    sec_pointer = ts_payload[0];

    /* Pointer field must be less than 182. */
    if (sec_pointer > 182)
    {
        service->err_code = DMX_SEC_ERR_CODE_BAD_POINTER_FIELD;

        dmx_ts2sec_drop_cur_pkt(dmx, service, dest_buf);

        service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

        return(-7);
    }

    /* If privous secion has not yet finished, finish it. */
    if (DMX_TS2SEC_SERVICE_STATUS_REMAIN_TS == service->status)
    {
        rd_addr = ts_payload + 1;

        /* Retrieve remaining data of privious section. */
        sec_remain_len = service->sec_len - service->sec_copied_len;

        /* sec_remain_len must equal to sec_pointer. */
        if(sec_remain_len != sec_pointer)
        {
			service->err_code = DMX_SEC_ERR_CODE_BAD_POINTER_FIELD;
            dmx_ts2sec_drop_cur_pkt(dmx, service, dest_buf);

            service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

            return(-8);
        }

        //printk("%s,%d,sl:%d,l:%d\n", __FUNCTION__, __LINE__, service->sec_len, sec_remain_len);

        ret = dmx_ts2sec_wr_pkt_end(dmx, service, rd_addr, sec_remain_len);

        service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

        /* Overflow. */
        if (ret < sec_remain_len)
        {
            //dmx_data_buf_kern_flush_all_pkt(dmx, dest_buf);

            return(-6);
        }
    }

    if (DMX_TS2SEC_SERVICE_STATUS_FIRST_TS == service->status)
    { 
		ts_end_addr = ts_payload + ts_pkt_inf->payload_len;
       	service->sec_copied_len = 0;

        rd_addr = ts_payload + 1 + sec_pointer;

        /* Loop to process sections contained in this TS pakcet if 
         * they exist.
         */
        while (rd_addr < ts_end_addr)
        {
            /* table_id == 0xFF means no more sections in this TS pakcet.
             * (iso-138188-1, c.3).
             */
            if (0xFF == rd_addr[0])
            {
                //printk("%s,%d,sl:%d,l:%d\n", __FUNCTION__, __LINE__, service->sec_len, 0);

                //dmx_ts2sec_wr_pkt_end(service, rd_addr, 0);
   
                /* return byte cnt that is acturally copied. */
                return(rd_addr - (ts_payload + 1 + sec_pointer));
            }

            ts_remain_len = ts_end_addr - rd_addr;

            //printk("end:%x,s:%x,rem:%x\n", ts_end_addr, rd_addr, ts_remain_len);

            /* Wait for next TS pakcet if this ts pakcet does not contain
             * enough data for retrieving section length field. 
             */
            if (ts_remain_len < 3)
            {
                memcpy(service->partial_hdr, rd_addr, ts_remain_len);

                service->partial_hdr_wr = ts_remain_len;

                /* OK, wait for next TS pakcet. */
                service->status = DMX_TS2SEC_SERVICE_STATUS_REMAIN_TS;

                //printk("%s,%d,sl:%d,l:%d\n", __FUNCTION__, __LINE__, service->sec_len, ts_remain_len);

                ret = dmx_ts2sec_wr_pkt_data(dmx, service, rd_addr, 
                                             ts_remain_len);

                /* Overflow. */
                if (ret < ts_remain_len)
                {
                    //dmx_data_buf_kern_flush_all_pkt(dmx, dest_buf);

                    service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;
                }

                return(ret);
            }

            /* Initialize sec service parameters. */
            service->sec_len = (((rd_addr[1] & 0x0F) << 8) | rd_addr[2]) + 3;
            service->sec_copied_len = 0;

            //printk("%s,%d,sec len:%d\n", __FUNCTION__, __LINE__, service->sec_len);
        
            /* Case 1: Section just finished within this TS packet. */
            if (ts_remain_len == service->sec_len)
            {
                //printk("%s,%d,sec len:%d\n", __FUNCTION__, __LINE__, service->sec_len);

                ret = dmx_ts2sec_wr_pkt_end(dmx, service, rd_addr, 
                                            ts_remain_len);

#if 0
                /* Overflow. */
                if (ret < ts_remain_len)
                {
                    dmx_data_buf_kern_flush_all_pkt(dmx, dest_buf);
                }
#endif

                service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

                return(ret);
            }

            /* Case 2: Section longer than this TS palyload, wait for next TS
             * pakcet.
             */
            if (ts_remain_len < service->sec_len)
            {
                //printk("%s,%d,sl:%d,l:%d\n", __FUNCTION__, __LINE__, service->sec_len, ts_remain_len);

                ret = dmx_ts2sec_wr_pkt_data(dmx, service, rd_addr, 
                                             ts_remain_len);

                /* Overflow. */
                if (ret < ts_remain_len)
                {
                    //dmx_data_buf_kern_flush_all_pkt(dmx, dest_buf);

                    service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;
                }
                else
                {
                    /* OK, wait for next TS pakcet. */
                    service->status = DMX_TS2SEC_SERVICE_STATUS_REMAIN_TS;
                }

                return(ret);
            }

            /* Case 3: Section length is shorter than this TS playload,
             *         continue to process next section within this
             *         TS paylaod.(ts_remain_len > sec_len)
             */
            //printk("%s,%d,sl:%d,l:%d\n", __FUNCTION__, __LINE__, service->sec_len, service->sec_len);

            ret = dmx_ts2sec_wr_pkt_end(dmx, service, rd_addr,
                                        service->sec_len);

			//have copied a section,so reset the sec_copied_len
			service->sec_copied_len = 0;

			/* Overflow. */
            if (ret < service->sec_len)
            {
                //dmx_data_buf_kern_flush_all_pkt(dmx, dest_buf);

                service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

                printk("%s,%d\n", __FUNCTION__, __LINE__);
            }

            rd_addr += service->sec_len;
			
        }
    }

    return(RET_SUCCESS);
}







/*****************************************************************************/
/**
*
* Register a section service to a demux driver.
*
* @param
* - dmx: A pointer to the demux to which the pes service is to be registered.
*
* - DMX_UINT16 pid: The PID for the pes service.
*
* - device is a pointer identifier to which the PES pakcet should send to.
*
* - request_buf: A function pointer used by pes service layer to get PES buffer
*   from upper layer. 
*   This function takes 4 parameters:
*    device is a pointer identifier to which the PES pakcet should send to.
*    len_req is the lenght of buffer the pes service layer want to get.
*    buf is a pointer to a pointer of PES buffer, upper layer should fill this
*    filed when pes service layer call request_buf().
*    len_got is the pes buffer length the pes service layer actualy got.
*    ctrl_blk is pointer to struct control_block which contains PES pakcet
*    information provided by pes service layer.
*
* - return_buf: A function pointer used by pes service layer to send PES pakcet
*   to upper layer.
*
* @note
*
* @return
* - DMX_INVALID_TS2ES_SERVICE_IDX: the PES service was not registered successfully
*   due to invalid parameter or insufficent TS service.
*
* - Otherwise: the service was successfully registered and the pes service index
*   is returned.
*
******************************************************************************/
DMX_UINT32 dmx_ts2sec_service_register
(
    struct dmx_device    *dmx,
    DMX_UINT16            pid,
    DMX_UINT32            mask_len,
    DMX_UINT8            *mask,
    DMX_UINT8            *value,
    DMX_UINT32            param 
)
{
    DMX_UINT32                 idx;
    DMX_UINT32                 ts_serv_idx;
    struct dmx_services       *services;
    struct dmx_ts2sec_service *serv;

    if (NULL == dmx)
    {
        return(DMX_INVALID_IDX);
    }

    if (NULL == mask)
    {
        return(DMX_INVALID_IDX);
    }

    if (NULL == value)
    {
        return(DMX_INVALID_IDX);
    }

    if (mask_len > DMX_SEC_MASK_MAX_LEN)
    {
        return(DMX_INVALID_IDX);
    }

    services = &dmx->services;

    /* Find a free sec service node */
    for (idx = 0; idx < DMX_TOTAL_TS2SEC_SERVICE; idx++)
    {
        serv = &(services->ts2sec_service[idx]);

        if (DMX_TS2SEC_SERVICE_STATUS_IDLE == serv->status)
        {
            break;
        }
    }

    if (idx >= DMX_TOTAL_TS2SEC_SERVICE)
    {
		return(DMX_INVALID_IDX);
    }

    /* Keep track of parameters for this sec service */
    memset(serv, 0, sizeof(struct dmx_ts2sec_service));

    serv->pid = pid;

    serv->param = param;

    serv->mask_len = mask_len;

    memcpy(serv->mask, mask, mask_len);

    memcpy(serv->value, value, mask_len);

    /* Check if we have free ts service slot for this sec service */
    ts_serv_idx = dmx_ts_service_register(dmx, pid, dmx_ts2sec_parse,
                                          (DMX_UINT32)serv);

    if (DMX_INVALID_IDX == ts_serv_idx)
    {
        //priv->ts2sec_service[idx].status = DMX_TS2SEC_SERVICE_STATUS_IDLE;

        return(DMX_INVALID_IDX);
    }

    services->ts2sec_service[idx].ts_serv_idx = ts_serv_idx;

    serv->status = DMX_TS2SEC_SERVICE_STATUS_READY;

    return(idx); 
}





/*****************************************************************************/
/**
*
* Enable a pes service identified by idx.
*
* @param
* - dmx: A pointer to the demux to which the pes service is to be enabled.
*
* - idx: The index of the pes service to be enalbed.
*
* @note
*
* @return
* - RET_SUCCESS: the service is legal and was successfully enalbed.
*
* - RET_FAILURE: the service is illegal and nothing was done.
*
******************************************************************************/
DMX_INT32 dmx_ts2sec_service_enable
(
    struct dmx_device *dmx,
    DMX_UINT32         idx
)
{
    struct dmx_ts2sec_service *service;

    if (NULL == dmx)
    {
        return(RET_FAILURE);
    }

    if (idx >= DMX_TOTAL_TS2SEC_SERVICE)
    {
        return(RET_FAILURE);
    }

    service = &(dmx->services.ts2sec_service[idx]);

    if (DMX_TS2SEC_SERVICE_STATUS_READY == service->status)
    {
        service->status = DMX_TS2SEC_SERVICE_STATUS_FIRST_TS;

        /* Enable ts service.
         */
        dmx_ts_service_enable(dmx, service->ts_serv_idx);
    }

    return(RET_SUCCESS); 
}




/*****************************************************************************/
/**
*
* Unregister a pes service identified by idx.
*
* @param
* - dmx: A pointer to the demux to which the pes service is to be enabled.
*
* - idx: The index of the pes service to be unregistered.
*
* @note
*
* @return
* - RET_SUCCESS: the service is legal and was successfully unregistered.
*
* - RET_FAILURE: the service is illegal and nothing was done.
*
******************************************************************************/
DMX_INT32 dmx_ts2sec_service_unregister
(
    struct dmx_device *dmx,
    DMX_UINT32         idx
)
{
    struct dmx_ts2sec_service *service;

    if (NULL == dmx)
    {
        return(RET_FAILURE);
    }

    if (idx >= DMX_TOTAL_TS2SEC_SERVICE)
    {
        return(RET_FAILURE);
    }

    service = &(dmx->services.ts2sec_service[idx]);

    if (DMX_TS2SEC_SERVICE_STATUS_IDLE != service->status)
    {
        /* Unregister ts service.
         */
        dmx_ts_service_unregister(dmx, service->ts_serv_idx);

        service->status = DMX_TS2SEC_SERVICE_STATUS_IDLE;
    }

    return(RET_SUCCESS); 
}







/*****************************************************************************/
/**
*
* Init the data structures which repsents the pes service.
*
* @param
* - dmx: A pointer to the demux to be initilaized.
*
* @note
*
* @return
* - RET_SUCCESS
*
******************************************************************************/
DMX_INT32 dmx_ts2sec_service_init
(
    struct dmx_device *dmx
)
{
    DMX_UINT32                 idx;
    struct dmx_ts2sec_service *service;

    for (idx = 0; idx < DMX_TOTAL_TS2SEC_SERVICE; idx++)
    {
        service = &(dmx->services.ts2sec_service[idx]);

        service->status = DMX_TS2SEC_SERVICE_STATUS_IDLE;

        service->cpy_type = DMX_MEM_CPY_CPU;
    }

    return(RET_SUCCESS);
}



