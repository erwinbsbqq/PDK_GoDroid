
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/delay.h>


#include "dmx_stack.h"


/************************** Constant Definitions ****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/


struct dmx_pes_flt_module ali_dmx_pes_flt_module;

__s32 dmx_pes_flt_wr_pkt_data(struct dmx_pes_flt *pes_flt,__u8 *src,__s32 len);
__s32 dmx_pes_flt_wr_pkt_end(struct dmx_pes_flt *pes_flt,__s32 ret_val);

__s32 dmx_pes_flt_parse(struct dmx_ts_pkt_inf *ts_pkt_inf, __u32 flt_idx);

__s32 dmx_pes_flt_state_set(struct dmx_pes_flt *pes_flt,
    						enum DMX_PES_FLT_STATE  new_state, 
    						enum DMX_ERR_CODE err_code);

__s32 dmx_pes_packet_info_retrieve(struct dmx_ts_pkt_inf *ts_pkt_inf, struct dmx_pes_flt *pes_flt);


/*****************************************************************************
*
* Judge the first PES packet
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
* - DMX_PES_OTHER_PKT: the first PES pakcet is illegal and should be dropped.
*
* - DMX_PES_FIRST_PKT: the PES pakcet is legal and the header info is retrieved to
*   serv.
*
******************************************************************************/

__inline __u8 dmx_pes_packet_is_start
(
    struct dmx_ts_pkt_inf *ts_pkt_inf,
    struct dmx_pes_flt		*pes_flt
)
{
    __u8 *pes_addr;

    pes_addr = ts_pkt_inf->payload_addr;

    if ((1 == ts_pkt_inf->unit_start) && (0x00 == pes_addr[0]) &&
        (0x00 == pes_addr[1])         && (0x01 == pes_addr[2]))
    {
		pes_flt->stat_info.PesInCnt++;

        return(DMX_PES_FIRST_PKT);
    }

	pes_flt->stat_info.PesHdrErrCnt++;

    return(DMX_PES_OTHER_PKT);
}


/*****************************************************************************
*
* Retrieve PES header information from a ts buffer.
*
* @param
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
__s32 dmx_pes_packet_info_retrieve
(
    struct dmx_ts_pkt_inf     *ts_pkt_inf,
    struct dmx_pes_flt		*pes_flt
)
{
    __u8 *pes_addr;

    /* PES header check.
     */
    pes_addr = ts_pkt_inf->payload_addr;

    pes_flt->pkt_len = ((pes_addr[4] << 8) | pes_addr[5]) + 6;

    pes_flt->pkt_copied_len = 0;

    return(RET_SUCCESS);
}


/*****************************************************************************
* Use callback function to copy PES Data 
*
* @param
*
* @note
*
* @return
* - 
******************************************************************************/

__s32 dmx_pes_flt_wr_pkt_data
(
    struct dmx_pes_flt *pes_flt,
    __u8               *src,
    __s32               len
)
{
    __s32  ret;

//    printk("%s(),L[%d]\n", __FUNCTION__, __LINE__);
    
    ret = pes_flt->pes_data_cb(src, len, DMX_PES_FLT_CB_TYPE_PKT_DATA, 
                               pes_flt->cb_param);

    if (ret < len)
    {
        /* Return error if data is not sent out completly(ret < len), upper 
         * layer should clear all data received in this PES pkt. 
         * This PES filter will be reset to DMX_PES_FLT_STATE_FIRST_TS
         * state and a new round of data collectoin will be started.
         */
		pes_flt->stat_info.PesBufOverflowCnt++;

        //printk("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, pes_flt->pid);

        return(ret);
    }
    else
    {
//        printk("%s(),L[%d], cpy_len[%d], ret[%d]\n", __FUNCTION__, __LINE__,pes_flt->pkt_copied_len,ret);
        
        pes_flt->pkt_copied_len += ret;
    }

    return(ret);
}


/*****************************************************************************
* To finish data copy 
*
* @param
*
* @note
*
* @return
* - 
******************************************************************************/
__s32 dmx_pes_flt_wr_pkt_end
(
    struct dmx_pes_flt *pes_flt,
    __s32 ret_val
)
{
//    printk("%s(): L[%d],\n",__FUNCTION__,__LINE__);
    
    ret_val = pes_flt->pes_data_cb(NULL, 0, DMX_PES_FLT_CB_TYPE_PKT_END,
                         pes_flt->cb_param);

	if (ret_val >= 0)
	{
		pes_flt->stat_info.PesOutCnt++;
	}
	
    return(DMX_ERR_OK);
}


/*****************************************************************************
*
* Copy TS packet into PES buffer, update PES buffer to upper layer if
* neccessary.
*
* @param
*
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
__s32 dmx_pes_flt_parse
(
    struct dmx_ts_pkt_inf *ts_pkt_inf,
    __u32                  flt_idx
)
{
    __s32               ret;
    __u8                is_pes_start;
    struct dmx_pes_flt *pes_flt;

    pes_flt = &(ali_dmx_pes_flt_module.pes_flt[flt_idx]);   

    if (0 == ts_pkt_inf->payload_len)
    {
		pes_flt->stat_info.PesTsNoPayloadCnt++;
		
        printk("%s(): L[%d], payload_len[%d] \n",\
            	__FUNCTION__,__LINE__,ts_pkt_inf->payload_len);
		
        return(DMX_ERR_OK);
    }

    
	if ((DMX_PES_FLT_STATE_FIRST_TS  != pes_flt->state) &&
		(DMX_PES_FLT_STATE_REMAIN_TS != pes_flt->state))
    {
        printk("%s(): L[%d], state[%d] \n",\
            	__FUNCTION__,__LINE__ ,pes_flt->state);
        
        return(DMX_ERR_PES_FLT_TS_BAD_STATE);
    }

    if (DMX_TS_PKT_CONTINU_DUPLICATE == ts_pkt_inf->continuity)
    {
		pes_flt->stat_info.PesTsDupCnt++;

		printk("%s(): L[%d], continuity[%d] \n",\
			__FUNCTION__,__LINE__,ts_pkt_inf->continuity);

        return(DMX_ERR_PES_FLT_TS_DUMPLICATE);
        
    }

    if (DMX_TS_PKT_CONTINU_LOST == ts_pkt_inf->continuity)
    {
		pes_flt->stat_info.PesTsLostCnt++;

		printk("%s(): L[%d], continuity[%d] \n",\
			__FUNCTION__,__LINE__,ts_pkt_inf->continuity);

        return(DMX_ERR_PES_FLT_TS_LOST);
    }

    if (0 != ts_pkt_inf->scramble_flag)
    {
		pes_flt->stat_info.PesTsScrmbCnt++;

        if (DMX_PES_FLT_STATE_REMAIN_TS == pes_flt->state)
        {
            pes_flt->state = DMX_PES_FLT_STATE_FIRST_TS;
        }

		printk("%s(): L[%d], scramble_flag[%d] \n",\
				__FUNCTION__,__LINE__,ts_pkt_inf->scramble_flag);

        return(DMX_ERR_PES_FLT_TS_SCRAMBLED);
    }

    is_pes_start = dmx_pes_packet_is_start(ts_pkt_inf, pes_flt);

    if (DMX_PES_FIRST_PKT == is_pes_start)
    {
        if (DMX_PES_FLT_STATE_REMAIN_TS == pes_flt->state)
        {
            //printk("%s(),%d\n", __FUNCTION__, __LINE__);
			dmx_pes_flt_wr_pkt_end(pes_flt, 0);
            
        }
        /* (DMX_PES_FLT_STATE_FIRST_TS == serv->state)  */
        else
        {
            pes_flt->state = DMX_PES_FLT_STATE_REMAIN_TS;
        }

        dmx_pes_packet_info_retrieve(ts_pkt_inf, pes_flt);

        ret = dmx_pes_flt_wr_pkt_data( pes_flt,ts_pkt_inf->payload_addr, 
                                       ts_pkt_inf->payload_len);       
        /* overflow. */
        if (ret < ts_pkt_inf->payload_len)
        {            
            pes_flt->state = DMX_PES_FLT_STATE_FIRST_TS;

            printk("%s():L[%d], overflow! \n",__FUNCTION__, __LINE__);

            return(ret);
        }

//	    printk("%s():L[%d],cpy_len[%d], payload_len[%d]\n",
//	           __FUNCTION__,__LINE__,pes_flt->pkt_copied_len,ts_pkt_inf->payload_len);
    }
    /* is_pes_start != 1
     * Copy remaining bytes of a PES packet.
     */
    else 
    {
        if (DMX_PES_FLT_STATE_REMAIN_TS == pes_flt->state)
        {
            ret = dmx_pes_flt_wr_pkt_data( pes_flt,ts_pkt_inf->payload_addr, 
                                           ts_pkt_inf->payload_len);    
            /* overflow. */
            if (ret < ts_pkt_inf->payload_len)
            {  
				printk("%s():%d,--overlap-- payload_len[%d], ret[%d]\n",\
                    	__FUNCTION__,__LINE__,ts_pkt_inf->payload_len, ret);
                
                pes_flt->state = DMX_PES_FLT_STATE_FIRST_TS;
    
                return(ret);
            }

//			printk("%s():L[%d],cpy_len[%d], payload_len[%d]\n",
//			       __FUNCTION__,__LINE__,pes_flt->pkt_copied_len,ts_pkt_inf->payload_len);
        }
    }


    /* serv->pes_total_len == 6 means the pes length field
     * of pes header is 0, this could happen to video PES. 
     */
    if ((pes_flt->state == DMX_PES_FLT_STATE_REMAIN_TS) &&
        (pes_flt->pkt_len != 6)&&
        (pes_flt->pkt_copied_len >= pes_flt->pkt_len))
    {

//        printk("%s(): L[%d], cpy_len[%d], pkt_len[%d] \n",
//            	__FUNCTION__,__LINE__,pes_flt->pkt_copied_len,pes_flt->pkt_len);

		dmx_pes_flt_wr_pkt_end(pes_flt, 0);

        pes_flt->state = DMX_PES_FLT_STATE_FIRST_TS;
    }

    return(ts_pkt_inf->payload_len);
}


/*****************************************************************************
*
* SET a pes filter status.
*
* @param
*
* - flt_idx: The index of the pes service to be started.
*
* @note
*
* @return
* - DMX_ERR_OK: the service is legal and was successfully enalbed.
*
******************************************************************************/
__s32 dmx_pes_flt_state_set
(
    struct dmx_pes_flt     *pes_flt,
    enum DMX_PES_FLT_STATE  new_state,
    enum DMX_ERR_CODE       err_code
)
{
    if (err_code < 0)
    {
        //printk("%s,%d,err_code:%d\n", __FUNCTION__, __LINE__, err_code);

        pes_flt->pes_data_cb(NULL, err_code, DMX_PES_FLT_CB_TYPE_ERR,
                             pes_flt->cb_param);
    }

    pes_flt->state = new_state;

    return(DMX_ERR_OK);
}





/*****************************************************************************
*
* Start a pes service identified by idx.
*
* @param
*
* - flt_idx: The index of the pes service to be started.
*
* @note
*
* @return
* - DMX_ERR_OK: the service is legal and was successfully enalbed.
*
* - < 0 : The corresponding error.
*
******************************************************************************/
__s32 dmx_pes_flt_start
(
    __s32 flt_idx
)
{
    __s32               ret;
    struct dmx_pes_flt *pes_flt;

    pes_flt = &(ali_dmx_pes_flt_module.pes_flt[flt_idx]);

    if (DMX_PES_FLT_STATE_STOP != pes_flt->state)
    {
        return(DMX_ERR_PES_FLT_OPERATION_DENIED);
    }
    
    ret = dmx_ts_flt_start(pes_flt->ts_flt_idx);
    
    if (ret < 0)
    {   	
        return(ret);
    }
    
    dmx_pes_flt_state_set(pes_flt, DMX_PES_FLT_STATE_FIRST_TS, 0);
    
    return(DMX_ERR_OK);
}



/*****************************************************************************
*
* Stop a pes service identified by idx.
*
* @param
*
* - flt_idx: The index of the pes service to be started.
*
* @note
*
* @return
* - DMX_ERR_OK: the service is legal and was successfully enalbed.
*
* - < 0 : The corresponding error.
*
******************************************************************************/
__s32 dmx_pes_flt_stop
(
    __s32 flt_idx
)
{
    __s32               	ret;
    struct dmx_pes_flt 		*pes_flt;

    pes_flt = &(ali_dmx_pes_flt_module.pes_flt[flt_idx]);

    if ((DMX_PES_FLT_STATE_FIRST_TS  != pes_flt->state) &&
        (DMX_PES_FLT_STATE_REMAIN_TS != pes_flt->state))
    {
        return(DMX_ERR_PES_FLT_OPERATION_DENIED);
    }

    ret = dmx_ts_flt_stop(pes_flt->ts_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    dmx_pes_flt_state_set(pes_flt, DMX_PES_FLT_STATE_STOP, 0);

    return(DMX_ERR_OK);
}


/*****************************************************************************
*
* Register a PES service to a demux driver.
*
* @param
** - cb_param: 
*        A function pointer used by pes service layer to get PES buffer from upper layer. 
*   This function takes 4 parameters:
*
* - ret_buf: A function pointer used by pes service layer to send PES pakcet
*   to upper layer.
*
* @note
*
* @return
*
******************************************************************************/
__s32 dmx_pes_flt_register
(
    __u32                      dmx_id,
    __u16                      pid,
    __s32        (*pes_data_cb)(__u8                     *src,
                                __s32                     len,
                                enum DMX_PES_FLT_CB_TYPE  cb_type,
                                __u32                     cb_param),
    __u32                       cb_param
)
{
    __s32               pes_flt_idx;
    __s32               ts_flt_idx;
    struct dmx_pes_flt *pes_flt;

    /* Find a free pes filter. 
     */
    for (pes_flt_idx = 0; pes_flt_idx < DMX_PES_FLT_CNT; pes_flt_idx++)
    {
        pes_flt = &(ali_dmx_pes_flt_module.pes_flt[pes_flt_idx]);

        if (DMX_PES_FLT_STATE_IDLE == pes_flt->state)
        {
            break;
        }
    }

    if (pes_flt_idx >= DMX_PES_FLT_CNT)
    {
        return(DMX_ERR_PES_FLT_EXHAUST);
    }

    /* Keep track of parameters for this pes filter.
     */
    memset(pes_flt, 0, sizeof(struct dmx_pes_flt));

    pes_flt->pid = pid;

    pes_flt->pes_data_cb = pes_data_cb;

    pes_flt->cb_param = cb_param;

    /* Check if we have free ts filter for this pes filter.
     */
    ts_flt_idx = dmx_ts_flt_register(dmx_id, pid, dmx_pes_flt_parse, 
                                     pes_flt_idx);

    if (ts_flt_idx < 0)
    {
        return(ts_flt_idx);
    }

    pes_flt->dmx_id = dmx_id;

    pes_flt->ts_flt_idx = ts_flt_idx;

    pes_flt->stat_info.PesFltIdx = pes_flt_idx;

    dmx_pes_flt_state_set(pes_flt, DMX_PES_FLT_STATE_STOP, 0);

    return(pes_flt_idx); 
}


/*****************************************************************************
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
__s32 dmx_pes_flt_unregister
(
    __s32 flt_idx
)
{
    __s32               ret;
    struct dmx_pes_flt *pes_flt;

    pes_flt = &(ali_dmx_pes_flt_module.pes_flt[flt_idx]);

    if (DMX_PES_FLT_STATE_IDLE == pes_flt->state)
    {
        return(DMX_ERR_PES_FLT_OPERATION_DENIED);
    }

    ret = dmx_ts_flt_unregister(pes_flt->ts_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    dmx_pes_flt_state_set(pes_flt, DMX_PES_FLT_STATE_IDLE, 0);

    return(0);
}


/*****************************************************************************
*
* Return the Filter IDX to catch the PES Data
*
******************************************************************************/

__u32 dmx_pes_flt_link_ts_flt_idx
(
    __s32 pes_flt_idx
)
{
	return(ali_dmx_pes_flt_module.pes_flt[pes_flt_idx].ts_flt_idx);
}



__s32 dmx_pes_flt_module_init
(
    void
)
{
    memset(&ali_dmx_pes_flt_module, 0, sizeof(struct dmx_pes_flt_module));

    return(DMX_ERR_OK);
}


