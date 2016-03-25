
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/delay.h>



#include "dmx_internal.h"

//#include <linux/ali_decv_plugin.h>
//#include <linux/ali_video.h>

/************************** Constant Definitions ****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/



/*****************************************************************************/
/**
*
* Retrieve PES header information from a ts buffer.
*
* @param
* - dmx: A pointer to the demux to which the pes serv is registered.
*
* - serv: A pointer to struct dmx_ts2pes_service which repsent a PES data
*                path.
*
* - ts_buf: A pointer to a struct dmx_ts_buf_t which contains all the
*           information of the current TS pakcet.
*
* @note
*
* @return
* - RET_FAILURE the PES pakcet is illegal and should be dropped.
*
* - RET_SUCCESS the PES pakcet is legal and the header info is retrieved to
*   serv.
*
******************************************************************************/
DMX_INT32 dmx_pes_packet_info_retrieve
(
    struct dmx_device         *dmx,
    struct dmx_ts2pes_service *serv,
    struct dmx_ts_pkt_inf     *ts_pkt_inf
)
{
    DMX_UINT8 *pes_addr;

    /* PES header check */
    pes_addr = ts_pkt_inf->payload_addr;

    serv->pkt_len = ((pes_addr[4] << 8) | pes_addr[5]) + 6;

    serv->pkt_copied_len = 0;

    return(RET_SUCCESS);
}



__inline __u8 dmx_pes_packet_is_start
(
    struct dmx_ts_pkt_inf *ts_pkt_inf
)
{
    DMX_UINT8 *pes_addr;

    pes_addr = ts_pkt_inf->payload_addr;

    if ((1 == ts_pkt_inf->unit_start) && (0x00 == pes_addr[0]) &&
        (0x00 == pes_addr[1])         && (0x01 == pes_addr[2]))
    {
        return(1);
    }

    return(0);
}


__s32 dmx_ts2pes_wr_data
(
    struct dmx_device         *dmx,
    struct dmx_ts2pes_service *serv,
    struct ali_dmx_data_buf   *dest_buf,
    __u8                      *src,
    __s32                      len
)
{
    __s32 ret;

    ret = dmx_data_buf_kern_wr_data(dmx, dest_buf, src, len);

    /* overflow. */
    if (ret < len)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        dmx_data_buf_kern_flush_all_pkt(dmx, dest_buf);
    }
    else
    {
        serv->pkt_copied_len += len;
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
* - dmx: A pointer to the demux to which the pes serv is registered.
*
* - ts_buf: A pointer to a struct dmx_ts_buf_t which contains all the
*           information of the current TS pakcet.
*
* - param_1: A pointer to struct dmx_ts2pes_service which repsents a PES data path.
*
* - parm_2: Not used.
*
* @note
* This function was implemented as an TS serv call back. This function 
* receive TS pakcet from TS serv layer and copy it into PES buffer provided
* by upper layer. 
*
* @return
* - NONE
*
******************************************************************************/
DMX_INT32 dmx_ts2pes_parse
(
    void                  *dmx,
    struct dmx_ts_pkt_inf *ts_pkt_inf,
    DMX_UINT32             param
)
{
    __s32                      ret;
    DMX_UINT8                  is_pes_start;
    struct dmx_ts2pes_service *serv;
    struct ali_dmx_data_buf   *dest_buf;

    serv = (struct dmx_ts2pes_service *)param;

    if ((DMX_TS2PES_SERVICE_STATUS_DATA       != serv->status)  &&
        (DMX_TS2PES_SERVICE_STATUS_START_CODE != serv->status))
    {
        return(RET_SUCCESS);
    }

    if (0 == ts_pkt_inf->payload_len)
    {
        return(RET_SUCCESS);
    }

    if (DMX_TS_PKT_CONTINU_DUPLICATE == ts_pkt_inf->continuity)
    {
        return(RET_SUCCESS);
    }

    if (0 != ts_pkt_inf->scramble_flag)
    {
        if (DMX_TS2PES_SERVICE_STATUS_DATA == serv->status)
        {
            serv->status = DMX_TS2PES_SERVICE_STATUS_START_CODE;
        }

        return(RET_SUCCESS);
    }

    dest_buf = (struct ali_dmx_data_buf *)serv->param;

    is_pes_start = dmx_pes_packet_is_start(ts_pkt_inf);

    if (1 == is_pes_start)
    {
        serv->pes_pkt_in_cnt++;

        if (DMX_TS2PES_SERVICE_STATUS_DATA == serv->status)
        {
            printk("%s,%d\n", __FUNCTION__, __LINE__);

            dmx_data_buf_kern_wr_pkt_end(dmx, dest_buf);
        }
        /* (DMX_TS2PES_SERVICE_STATUS_START_CODE == serv->status) */
        else
        {
            serv->status = DMX_TS2PES_SERVICE_STATUS_DATA;
        }

        dmx_pes_packet_info_retrieve(dmx, serv, ts_pkt_inf);

        ret = dmx_ts2pes_wr_data(dmx, serv, dest_buf, ts_pkt_inf->payload_addr, 
                                 ts_pkt_inf->payload_len);

        /* overflow. */
        if (ret < ts_pkt_inf->payload_len)
        {            
            serv->status = DMX_TS2PES_SERVICE_STATUS_START_CODE;

            return(ret);
        }

        //serv->pkt_copied_len += ts_pkt_inf->payload_len;
    }
    /* is_pes_start != 1
     * Copy remaining bytes of a PES packet.
     */
    else 
    {
        if (DMX_TS2PES_SERVICE_STATUS_DATA == serv->status)
        {
            ret = dmx_ts2pes_wr_data(dmx, serv, dest_buf, 
                                     ts_pkt_inf->payload_addr, 
                                     ts_pkt_inf->payload_len);
    
            /* overflow. */
            if (ret < ts_pkt_inf->payload_len)
            {            
                serv->status = DMX_TS2PES_SERVICE_STATUS_START_CODE;
    
                return(ret);
            }
        }
    }

    /* serv->pes_total_len == 6 means the pes length field
     * of pes header is 0, this could happen to video PES. 
     */
    if ((serv->status == DMX_TS2PES_SERVICE_STATUS_DATA) &&
        (serv->pkt_len != 6)                             &&
        (serv->pkt_copied_len >= serv->pkt_len))
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        dmx_data_buf_kern_wr_pkt_end(dmx, dest_buf);

        serv->status = DMX_TS2PES_SERVICE_STATUS_START_CODE;
    }

    return(ts_pkt_inf->payload_len);
}






/*****************************************************************************/
/**
*
* Register a PES service to a demux driver.
*
* @param
* - dmx: A pointer to the demux to which the pes service is to be registered.
*
* - DMX_UINT16 pid: The PID for the pes service.
*
* - device is a pointer identifier to which the PES pakcet should send to.
*
* - req_buf: A function pointer used by pes service layer to get PES buffer
*   from upper layer. 
*   This function takes 4 parameters:
*    device is a pointer identifier to which the PES pakcet should send to.
*    len_req is the lenght of buffer the pes service layer want to get.
*    buf is a pointer to a pointer of PES buffer, upper layer should fill this
*    filed when pes service layer call req_buf().
*    len_got is the pes buffer length the pes service layer actualy got.
*    ctrl_blk is pointer to struct control_block which contains PES pakcet
*    information provided by pes service layer.
*
* - ret_buf: A function pointer used by pes service layer to send PES pakcet
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
DMX_UINT32 dmx_ts2pes_service_register
(
    struct dmx_device *dmx,
    DMX_UINT16         pid,
    DMX_UINT32         param
)
{
    DMX_UINT32                 idx;
    DMX_UINT32                 ts_serv_idx;
    struct dmx_ts2pes_service *ts2pes_service;
    struct dmx_services       *services;

    services = &dmx->services;

    /* Find an idle pes service slot for this call. */
    for (idx = 0; idx < DMX_TOTAL_TS2PES_SERVICE; idx++)
    {
        ts2pes_service = &(services->ts2pes_service[idx]);

        if (DMX_TS2PES_SERVICE_STATUS_IDLE == 
            services->ts2pes_service[idx].status)
        {
            break;
        }
    }

    if (idx >= DMX_TOTAL_TS2PES_SERVICE)
    {
        return(DMX_INVALID_IDX);
    }

    /* Check if we have ts service for this pes service */
    ts_serv_idx = dmx_ts_service_register(dmx, pid, dmx_ts2pes_parse,
                                          (DMX_UINT32)ts2pes_service);

    if (DMX_INVALID_IDX == ts_serv_idx)
    {
        return(DMX_INVALID_IDX);
    }

    /* Keep track of parameters for this pes service */        
    ts2pes_service->param = param;

    ts2pes_service->pid = pid;

    //ts2pes_service->stream_id = reg_serv->str_type;

    ts2pes_service->ts_serv_idx = ts_serv_idx;

    ts2pes_service->status = DMX_TS2PES_SERVICE_STATUS_READY;

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
DMX_INT32 dmx_ts2pes_service_enable
(
    struct dmx_device *dmx,
    DMX_UINT32             idx
)
{
    struct dmx_ts2pes_service *ts2pes_service;
    struct dmx_services       *services;

    if (NULL == dmx)
    {
        return(RET_FAILURE);
    }

    /* ts2pes_service not found. */
    if (idx >= DMX_TOTAL_TS2PES_SERVICE)
    {
        return(RET_FAILURE);
    }

    services = &dmx->services;

    ts2pes_service = &(services->ts2pes_service[idx]);

    if (DMX_TS2PES_SERVICE_STATUS_READY == ts2pes_service->status)
    {
        ts2pes_service->status = DMX_TS2PES_SERVICE_STATUS_START_CODE;

        /* Enalbe ts service. */
        dmx_ts_service_enable(dmx, ts2pes_service->ts_serv_idx);
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
DMX_INT32 dmx_ts2pes_service_unregister
(
    struct dmx_device *dmx,
    DMX_UINT32             idx
)
{
    DMX_INT32                   ret;
    struct dmx_ts2pes_service *ts2pes_service;
    struct dmx_services       *services;

    if (NULL == dmx)
    {
        return(RET_FAILURE);
    }

    if (idx >= DMX_TOTAL_TS2PES_SERVICE)
    {
        return(RET_FAILURE);
    }

    services = &dmx->services;

    ts2pes_service = &(services->ts2pes_service[idx]);

    if (DMX_TS2PES_SERVICE_STATUS_IDLE != ts2pes_service->status)
    {
        ts2pes_service->status = DMX_TS2PES_SERVICE_STATUS_IDLE;

        /* Unregister ts filter.
         */
        ret = dmx_ts_service_unregister(dmx, ts2pes_service->ts_serv_idx);
    }

    return(RET_SUCCESS); 
}



DMX_INT32 dmx_ts2pes_service_init
(
    struct dmx_device *dmx
)
{
    DMX_UINT32           i;
    struct dmx_services *services;

    services = &dmx->services;

    for (i = 0; i < DMX_TOTAL_TS2PES_SERVICE; i++)
    {
        services->ts2pes_service[i].status = DMX_TS2PES_SERVICE_STATUS_IDLE;
    }

    return(RET_SUCCESS); 
}


