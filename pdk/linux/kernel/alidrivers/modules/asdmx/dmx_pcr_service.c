
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 


#include "dmx_hw.h"
#include "dmx_internal.h"


__s32 dmx_pcr_parse
(
    void                  *dev,
    struct dmx_ts_pkt_inf *ts_pkt_inf,
    DMX_UINT32             param 
)
{
    struct dmx_device *dmx;

	dmx = (struct dmx_device *)dev;

    if (DMX_TYPE_SW == dmx->dmx_type)
    {
        /* TODO: parse PCR using SW.
		 */
	}

    return(0);
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
DMX_UINT32 dmx_pcr_service_register
(
    struct dmx_device *dmx,
    DMX_UINT16         pid
)
{
    DMX_UINT32           ts_serv_idx;
    struct dmx_services *services;

    services = &dmx->services;

    /* Check if we have idle pcr service slot for this request */
    if (DMX_PCR_SERVICE_STATUS_IDLE != services->pcr_service.status)
    {
        return(DMX_INVALID_IDX);
    }

    /* Check if we have ts service for this pcr service */
    ts_serv_idx = dmx_ts_service_register(dmx, pid, dmx_pcr_parse, 0);

    if (DMX_INVALID_IDX == ts_serv_idx)
    {
        return(DMX_INVALID_IDX);
    }

    /* Keep track of parameters for this pcr service */
    services->pcr_service.ts_serv_idx = ts_serv_idx;

    services->pcr_service.pid = pid;

    services->pcr_service.status = DMX_PCR_SERVICE_STATUS_READY;

    return(0); 
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
DMX_INT32 dmx_pcr_service_enable
(
    struct dmx_device *dmx,
    DMX_UINT32         idx
)
{
    DMX_UINT32           ts_serv_idx;
    DMX_UINT32           pid_filter_idx;
    struct dmx_services *services;

    if (NULL == dmx)
    {
        return(RET_FAILURE);
    }

    services = &dmx->services;

    ts_serv_idx = services->pcr_service.ts_serv_idx;

    /* Validate service */
    if (DMX_PCR_SERVICE_STATUS_READY == services->pcr_service.status)
    {
        services->pcr_service.status = DMX_PCR_SERVICE_STATUS_RUN;
    
        pid_filter_idx = services->ts_service[ts_serv_idx].pid_flt_idx;
    
        if (DMX_TYPE_HW == dmx->dmx_type)
        {
            dmx->pfunc.dmx_hw_pcr_set_flt(dmx->base_addr, (DMX_UINT8)pid_filter_idx);
        
            dmx->pfunc.dmx_hw_pcr_int_enable(dmx->base_addr);
        
            dmx->pfunc.dmx_hw_pcr_detect_enable(dmx->base_addr);
        }

        /* Enalbe ts filter */    
        dmx_ts_service_enable(dmx, ts_serv_idx); 
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
DMX_INT32 dmx_pcr_service_unregister
(
    struct dmx_device *dmx,
    DMX_UINT32             idx
)
{
    struct dmx_services *services;

    if (NULL == dmx)
    {
        return(RET_FAILURE);
    }

    services = &dmx->services;

    if (DMX_PCR_SERVICE_STATUS_IDLE != services->pcr_service.status)
    {
        services->pcr_service.status = DMX_PCR_SERVICE_STATUS_IDLE;

        if (DMX_TYPE_HW == dmx->dmx_type)
        {
            dmx->pfunc.dmx_hw_pcr_int_disable(dmx->base_addr);
        
            dmx->pfunc.dmx_hw_pcr_detect_disable(dmx->base_addr);
        }

        /* Unregister ts service.
         */
        dmx_ts_service_unregister(dmx, services->pcr_service.ts_serv_idx);

        services->pcr_service.ts_serv_idx = DMX_INVALID_IDX;
    
        services->pcr_service.pid = 0x1FFF;
    }

    return(RET_SUCCESS); 
}







DMX_INT32 dmx_pcr_service_init
(
    struct dmx_device *dmx
)
{
    struct dmx_services *services;

    services = &dmx->services;

    services->pcr_service.status = DMX_PCR_SERVICE_STATUS_IDLE;

    //set_stc_divisor(299, 0);

    return(RET_SUCCESS); 
}


