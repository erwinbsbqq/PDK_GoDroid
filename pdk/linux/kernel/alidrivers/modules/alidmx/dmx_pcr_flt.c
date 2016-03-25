
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 

#include "dmx_see_interface.h"
#include "dmx_stack.h"

struct dmx_pcr_flt_module ali_dmx_pcr_flt_module;



__s32 dmx_pcr_parse
(
    struct dmx_ts_pkt_inf *ts_pkt_inf,
    __u32                  param 
)
{
    struct dmx_pcr_flt *pcr_flt; 

#if 0
    /* TODO: parse PCR using SW for DMX_TYPE_SW since SW DMX doest not
     * have HW PCR interrupt..
     */
    if (DMX_TYPE_SW == dmx->dmx_type)
    {

    }
#endif

    pcr_flt = (struct dmx_pcr_flt *)param;

    /* Send PCR TS pakcet to SEE since SEE may need PCR pakcet for doing 
     * a/v sync by SW.
     * SEE interface may drop this packet if it is the same PID with audio
     * or video PID.
     */
    if (NULL != pcr_flt->pcr_pkt_cb)
    {
        pcr_flt->pcr_pkt_cb(ts_pkt_inf, pcr_flt->pcr_pkt_param);
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
__s32 dmx_pcr_flt_register
(
    __u32 dmx_id,
    __u16 pid,
    __s32 pcr_value_cb(__u32 pcr,
                       __u32 value_cb_param),
    __u32 value_cb_param,

    __s32 pcr_pkt_cb(struct dmx_ts_pkt_inf *pkt_inf,
                     __u32                  pkt_cb_param),
    __u32 pkt_cb_param
)
{
    __s32               ret;
    struct dmx_pcr_flt *pcr_flt; 

    pcr_flt = &ali_dmx_pcr_flt_module.pcr_flt[0];

    /* Check if we have idle pcr service slot for this request. 
    */
    if (DMX_PCR_FLT_STATE_IDLE != pcr_flt->state)
    {
        return(DMX_ERR_PCR_FLT_EXHAUST);
    }

    ret = dmx_ts_flt_register(dmx_id, pid, dmx_pcr_parse, (__u32)pcr_flt);

    if (ret < 0)
    {
        return(ret);
    }

    /* Keep track of parameters for this pcr service.
    */
    pcr_flt->dmx_id = dmx_id;

    pcr_flt->pid = pid;

    pcr_flt->ts_flt_idx = ret;

    pcr_flt->pcr_value_cb = pcr_value_cb;

    pcr_flt->pcr_cb_param = value_cb_param;

    pcr_flt->pcr_pkt_cb = pcr_pkt_cb;

    pcr_flt->pcr_pkt_param = pkt_cb_param;

    pcr_flt->state = DMX_PCR_FLT_STATE_STOP;

    return(DMX_ERR_OK); 
}




__s32 dmx_pcr_flt_start
(
    __s32 flt_idx
)
{
    __s32               ret;
    struct dmx_pcr_flt *pcr_flt; 

    pcr_flt = &ali_dmx_pcr_flt_module.pcr_flt[flt_idx];

    if (DMX_PCR_FLT_STATE_STOP != pcr_flt->state)
    {
        return(DMX_ERR_PCR_FLT_OPERATION_DENIED);
    }

    ret = dmx_ts_flt_start(pcr_flt->ts_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    ret = dmx_hw_pcr_detect_enable(pcr_flt->dmx_id, pcr_flt->pid, 
                                   pcr_flt->pcr_value_cb,
                                   pcr_flt->pcr_cb_param);

    if (ret < 0)
    {
        return(ret);
    }

    pcr_flt->state = DMX_PCR_FLT_STATE_RUN; 

    return(0);
}



__s32 dmx_pcr_flt_stop
(
    __s32 flt_idx
)
{
    __s32               ret;
    struct dmx_pcr_flt *pcr_flt; 

    pcr_flt = &ali_dmx_pcr_flt_module.pcr_flt[flt_idx];

    if (DMX_PCR_FLT_STATE_RUN != pcr_flt->state)
    {
        return(DMX_ERR_PCR_FLT_OPERATION_DENIED);
    }

    ret = dmx_hw_pcr_detect_disable(pcr_flt->dmx_id, pcr_flt->pid);

    if (ret < 0)
    {
        return(ret);
    }

    ret = dmx_ts_flt_stop(pcr_flt->ts_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    pcr_flt->state = DMX_PCR_FLT_STATE_STOP;    

    return(0);
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
__s32 dmx_pcr_flt_unregister
(
    __s32 flt_idx
)
{
    __s32               ret;
    struct dmx_pcr_flt *pcr_flt; 

    pcr_flt = &ali_dmx_pcr_flt_module.pcr_flt[flt_idx];

    if (DMX_PCR_FLT_STATE_IDLE == pcr_flt->state)
    {
        return(DMX_ERR_PCR_FLT_OPERATION_DENIED);
    }

    if (DMX_PCR_FLT_STATE_RUN == pcr_flt->state)
    {
        ret = dmx_hw_pcr_detect_disable(pcr_flt->dmx_id, pcr_flt->pid);
        
        if (ret < 0)
        {
            return(ret);
        }
    }

    ret = dmx_ts_flt_unregister(pcr_flt->ts_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    pcr_flt->state = DMX_PCR_FLT_STATE_IDLE;    

    return(0);
}


__u32 dmx_pcr_flt_link_ts_flt_idx
(
    __s32 pcr_flt_idx
)
{
	return(ali_dmx_pcr_flt_module.pcr_flt[pcr_flt_idx].ts_flt_idx);
}


__s32 dmx_pcr_flt_module_init
(
    void
)
{
    __u32 flt_idx;

    memset(&ali_dmx_pcr_flt_module, 0, sizeof(struct dmx_pcr_flt_module));

    for (flt_idx = 0; flt_idx < DMX_PCR_FLT_CNT; flt_idx++)
    {
        ali_dmx_pcr_flt_module.pcr_flt[flt_idx].state = DMX_PCR_FLT_STATE_IDLE;
    }
    
    return(0);
}



