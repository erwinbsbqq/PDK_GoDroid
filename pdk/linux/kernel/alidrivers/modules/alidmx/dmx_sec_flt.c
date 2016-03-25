
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 



#include "dmx_stack.h"


struct dmx_sec_flt_module ali_dmx_sec_flt_module;



__s32 dmx_sec_flt_is_mask_hit
(
    struct dmx_sec_flt *sec_flt,
    __u8               *sec_data
)
{
    __u32 Idx;
    __u8  value;
    
    for (Idx = 0; Idx < sec_flt->MaskInfo.MatchLen; Idx++)
    {
        value = sec_data[Idx];

        if (0 == sec_flt->MaskInfo.Negate[Idx])
        {
            if ((sec_flt->MaskInfo.Mask[Idx] & value) !=
                (sec_flt->MaskInfo.Mask[Idx] & sec_flt->MaskInfo.Match[Idx]))
            {
				sec_flt->stat_info.SecMaskMismatchCnt++;

                return(DMX_ERR_SEC_FLT_MASK_MISSMATCH);
            }
        }
        else
        {
            if ((sec_flt->MaskInfo.Mask[Idx] & value) ==
                (sec_flt->MaskInfo.Mask[Idx] & sec_flt->MaskInfo.Match[Idx]))
            {
				sec_flt->stat_info.SecMaskMismatchCnt++;

                return(DMX_ERR_SEC_FLT_MASK_MISSMATCH);
            }
        }
    }

    return(DMX_ERR_OK);
}


__s32 dmx_sec_flt_mask_match
(
    struct dmx_sec_flt *sec_flt,
    __u8               *src,
    __s32               len
)
{
    __s32  ret;
    __u32  match_len;
    __u8  *temp_match_addr;
    __u32  cpy_len;

    /* 1, Do section mask match directly in DMX HW buffer to offload CPU 
     *    loading if possible. 
     * 2, Do section match only if we have got section length.
     * 3, If MatchLen is longer than sec_len, return error.
     */
    if ((sec_flt->sec_len > 0) && (sec_flt->sec_len < sec_flt->MaskInfo.MatchLen))
    {
		sec_flt->stat_info.SecMaskTooLongCnt++;

        return(DMX_ERR_SEC_FLT_MASK_TOO_LONG);
    }
    
    match_len = sec_flt->MaskInfo.MatchLen;
    
    if (DMX_SEC_FLT_STATE_FIRST_TS == sec_flt->state)
    {
        /* We do sectin mask match direcly in HW buffer to offload CPU
         * loading.
         * If section data contained in HW buffer is not long enough to do 
         * section mask match, store data into a temp buffer until it is
         * long enough, then, in this case, we do mask match in this temp
         * buffer.
         */
        if (len >= match_len)
        {
            ret = dmx_sec_flt_is_mask_hit(sec_flt, src);
        
            /* Drop current section if mask not match.
             */
            return(ret);
        }
        else 
        {
            memcpy(&(sec_flt->mask_sec_data[0]), src, len);
    
            sec_flt->mask_sec_data_wr += len;
        }
    }
    else if (DMX_SEC_FLT_STATE_REMAIN_TS == sec_flt->state)
    {
        if (sec_flt->mask_sec_data_wr > 0)
        {
            temp_match_addr = &(sec_flt->mask_sec_data[0]);
            
            if ((len + sec_flt->mask_sec_data_wr) < match_len)
            {               
                memcpy(temp_match_addr + sec_flt->mask_sec_data_wr, src,
                       len);
    
                sec_flt->mask_sec_data_wr += len;
            }
            else 
            {
                cpy_len = match_len - sec_flt->mask_sec_data_wr;
                
                memcpy(temp_match_addr + sec_flt->mask_sec_data_wr, src,
                       cpy_len);
    
                sec_flt->mask_sec_data_wr += cpy_len;       
                
                ret = dmx_sec_flt_is_mask_hit(sec_flt, temp_match_addr);
    
                sec_flt->mask_sec_data_wr = 0;
    
                return(ret);
            }
        }
    }

    return(DMX_ERR_OK);
}


__s32 dmx_sec_flt_wr_pkt_data
(
    struct dmx_sec_flt *sec_flt,
    __u8               *src,
    __s32               len
)
{
    __s32  ret;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    ret = dmx_sec_flt_mask_match(sec_flt, src, len);

    if (ret < 0)
    {
        return(ret);
    }

    ret = sec_flt->sec_data_cb(src, len, DMX_SEC_FLT_CB_TYPE_PKT_DATA, 
                               sec_flt->cb_param);

    if (ret < len)
    {
        /* Return error if data is not sent out completly(ret < len), upper 
         * layer should clear all data received in this section. 
         * This section filter will be reset to DMX_SEC_FLT_STATE_FIRST_TS
         * state and a new round of data collectoin will be started.
         */
		sec_flt->stat_info.SecBufOverflowCnt++;

        //printk("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, sec_flt->pid);

        return(ret);
    }
    else
    {
        sec_flt->sec_copied_len += ret;
    }

    return(ret);
}




__s32 dmx_sec_flt_wr_pkt_end
(
    struct dmx_sec_flt *sec_flt,
    __s32 ret_val
)
{
    sec_flt->sec_data_cb(NULL, 0, DMX_SEC_FLT_CB_TYPE_PKT_END,
                         sec_flt->cb_param);

	if (ret_val >= 0)
	{
		sec_flt->stat_info.SecOutCnt++;
	}
	
    return(DMX_ERR_OK);
}





__s32 dmx_sec_flt_state_set
(
    struct dmx_sec_flt     *sec_flt,
    enum DMX_SEC_FLT_STATE  new_state,
    enum DMX_ERR_CODE       err_code
)
{
    if (err_code < 0)
    {
        //printk("%s,%d,err_code:%d\n", __FUNCTION__, __LINE__, err_code);

        sec_flt->sec_data_cb(NULL, err_code, DMX_SEC_FLT_CB_TYPE_ERR,
                             sec_flt->cb_param);
    }

    if (DMX_SEC_FLT_STATE_FIRST_TS == new_state)
    {
        sec_flt->mask_sec_data_wr = 0;
    
        sec_flt->partial_hdr_wr = 0;

        sec_flt->sec_len = 0;

        sec_flt->sec_copied_len = 0;
    }

    sec_flt->state = new_state;

    return(0);
}






__s32 dmx_sec_flt_parse
(
    struct dmx_ts_pkt_inf *ts_pkt_inf,
    __u32                  flt_idx
)
{
    struct dmx_sec_flt *sec_flt;
    __u8                sec_pointer;
    __u32               ts_remain_len;
    __u32               sec_remain_len;
    __u8               *ts_payload;
    __u8               *ts_end_addr;
    __u8               *rd_addr;
    __u8               *sec_start;
    __u32               sec_data_len;
    __s32               ret;

	sec_flt = &(ali_dmx_sec_flt_module.sec_flt[flt_idx]);

    if (0 == ts_pkt_inf->payload_len)
    {
		sec_flt->stat_info.SecTsNoPayloadCnt++;
        return(DMX_ERR_OK);
    }

    sec_flt = &(ali_dmx_sec_flt_module.sec_flt[flt_idx]);

    if (0 != ts_pkt_inf->scramble_flag)
    {
    	sec_flt->stat_info.SecTsScrmbCnt++;
		
        dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS,
                              DMX_ERR_SEC_FLT_TS_SCRAMBLED);

        return(DMX_ERR_SEC_FLT_TS_SCRAMBLED);
    }

    if (DMX_TS_PKT_CONTINU_DUPLICATE == ts_pkt_inf->continuity)
    {
		sec_flt->stat_info.SecTsDupCnt++;
		
        return(DMX_ERR_SEC_FLT_TS_DUPLICATE);
    }

    if (DMX_TS_PKT_CONTINU_LOST == ts_pkt_inf->continuity)
    {
		sec_flt->stat_info.SecTsLostCnt++;

        dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS,
                              DMX_ERR_SEC_FLT_TS_LOST);

        return(DMX_ERR_SEC_FLT_TS_LOST);
    }

    ret = 0;

    ts_payload = ts_pkt_inf->payload_addr;

    /* In some rare cases sec data in privious TS pakcet is not
     * long enough to contain section length filed.
     * Note: if service->partial_hdr_wr > 0 then 
     * service->status must be DMX_TS2SEC_SERVICE_STATUS_REMAIN_TS
     * in our code logic.
     */
    if (sec_flt->partial_hdr_wr > 0)
    {
        if (0 == ts_pkt_inf->unit_start)
        {
            sec_start = ts_pkt_inf->payload_addr;

            sec_data_len = ts_pkt_inf->payload_len;
        }
        else
        {
            sec_start = ts_pkt_inf->payload_addr + 1;

            /* Pointer field. 
             */
            sec_data_len = ts_pkt_inf->payload_addr[0];
        }

        /* Sec data still not long enough, continue wait for enough data.
         * According to iso-13818-1, each section pakcet should be at least 
         * 3 bytes in length.
         */
        if (sec_data_len + sec_flt->partial_hdr_wr < 3)
        {
            if (0 == ts_pkt_inf->unit_start)
            {
                ret = dmx_sec_flt_wr_pkt_data(sec_flt, sec_start, 
                                             sec_data_len);

                /* Error. 
                 */
                if (ret < 0)
                {
                    dmx_sec_flt_state_set(sec_flt, 
                                          DMX_SEC_FLT_STATE_FIRST_TS, 
                                          ret);
                    return(ret);
                }

                /* Otherwise OK, copy data into partial_hdr buffer.
                 */
                memcpy(sec_flt->partial_hdr + sec_flt->partial_hdr_wr, 
                       sec_start, sec_data_len);
    
                sec_flt->partial_hdr_wr += sec_data_len;
    
                return(ret);
            }
            /* Otherwise error.
             */
            else
            {
				sec_flt->stat_info.SecHdrTooShortCnt++;

                dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS,
                                      DMX_ERR_SEC_FLT_SEC_UNFINISHED);
            }
        }
        else
        {
            /* Enough data retrieved, now get sec len.
             */
            memcpy(sec_flt->partial_hdr + sec_flt->partial_hdr_wr, sec_start,
                   3 - sec_flt->partial_hdr_wr);
    
            sec_flt->sec_len = (((sec_flt->partial_hdr[1] & 0x0F) << 8) | 
                                  sec_flt->partial_hdr[2]) + 3;

            /* Section length should not exceed 0x3FD or less than 1.
             * See iso-13818-1, 2.4.4.13.
             * But in reality, 0xFFF(4K), is commonly used.
             */
            if (sec_flt->sec_len <= 3)
            {
				sec_flt->stat_info.SecDataTooShortCnt++;

                dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS,
                                      DMX_ERR_SEC_FLT_BAD_SEC_LEN);
        
                return(DMX_ERR_SEC_FLT_BAD_SEC_LEN);
            }
    
            /* Clear flag.
             */
            sec_flt->partial_hdr_wr = 0;
        }
    }

    /* TS packet with PUSI(payload_unit_start_indicator) == 0. 
     * This means this TS packet contains no new section.
     */
    if (0 == ts_pkt_inf->unit_start)
    {
        if (DMX_SEC_FLT_STATE_REMAIN_TS == sec_flt->state)
        {
            sec_remain_len = sec_flt->sec_len - sec_flt->sec_copied_len;

            /* Case 1: Current section is finished within this TS pakcet.
             * No other secions coexists in this TS packet, 
             * since PUSI == 0 means there should be no new section starts
             * in this TS packet.
             */
            if(sec_remain_len < ts_pkt_inf->payload_len)
            {
                ret = dmx_sec_flt_wr_pkt_data(sec_flt, ts_payload, 
                                              sec_remain_len);
                
                dmx_sec_flt_wr_pkt_end(sec_flt, ret);
    
                dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS, ret);

                return(ret);
            }

            /* Case 2: Current section is not finished within this TS packet,
             * copy all TS pakcet payload into section buffer and wait for
             * next TS packet.
             */
            ret = dmx_sec_flt_wr_pkt_data(sec_flt, ts_payload, 
                                         ts_pkt_inf->payload_len);

            /* Error. 
             */
            if (ret < 0)
            {
                dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS,
                                      ret);
            }

            return(ret);
        }

        return(0);
    }

    /* Otherwise TS packet with PUSI(payload_unit_start_indicator) == 1,
     * this means there should be at least one new section starts in this TS
     * packet.
     */
    sec_pointer = ts_payload[0];

    /* (sec_pointer + 1) must be less than ts_pkt_inf->payload_len, according 
     * to iso-13818-1.
     */
    if ((sec_pointer + 1) > ts_pkt_inf->payload_len)
    {
		sec_flt->stat_info.SecPtErrCnt++;

        dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS,
                              DMX_ERR_SEC_FLT_BAD_POINTER_FIELD);

        return(DMX_ERR_SEC_FLT_BAD_POINTER_FIELD);
    }

    /* If privous secion has not yet been finished, finish it.
     */
    if (DMX_SEC_FLT_STATE_REMAIN_TS == sec_flt->state)
    {
        rd_addr = ts_payload + 1;

        /* Retrieve remaining data of privious section.
         */
        sec_remain_len = sec_flt->sec_len - sec_flt->sec_copied_len;

        /* sec_remain_len must equal to sec_pointer.
         */
        if(sec_remain_len != sec_pointer)
        {
			sec_flt->stat_info.SecDataNotFullCnt++;

            dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS,
                                  DMX_ERR_SEC_FLT_BAD_SEC_LEN);
        }
        else
        {
            ret = dmx_sec_flt_wr_pkt_data(sec_flt, rd_addr, sec_remain_len);
            
            dmx_sec_flt_wr_pkt_end(sec_flt, ret);

            dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS, ret);
        }
    }

    if (DMX_SEC_FLT_STATE_FIRST_TS == sec_flt->state)
    { 
		sec_flt->stat_info.SecInCnt++;

        ts_end_addr = ts_payload + ts_pkt_inf->payload_len;
       
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
                /* return byte cnt that is acturally copied.
                 */
                return(rd_addr - (ts_payload + 1 + sec_pointer));
            }

            ts_remain_len = ts_end_addr - rd_addr;

            /* Wait for next TS pakcet if this ts pakcet does not contain
             * enough data for retrieving section length field. 
             */
            if (ts_remain_len < 3)
            {
                ret = dmx_sec_flt_wr_pkt_data(sec_flt, rd_addr, ts_remain_len);

                /* Error. 
                 */
                if (ret < 0)
                {
                    dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS, ret);

                    return(ret);
                }

                /* OK, wait for next TS pakcet.
                 */
                memcpy(sec_flt->partial_hdr, rd_addr, ts_remain_len);

                sec_flt->partial_hdr_wr = ts_remain_len;

                dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_REMAIN_TS, ret);
                
                /* return byte cnt that is acturally copied.
                 */
                return(rd_addr - (ts_payload + 1 + sec_pointer));
            }

            /* Get section length.
             */
            sec_flt->sec_len = (((rd_addr[1] & 0x0F) << 8) | rd_addr[2]) + 3;

            /* Section length should not exceed 0x3FD or less than 1.
             * See iso-13818-1, 2.4.4.13.
             * But in reality, 0xFFF(4K), is used.
             */
            if (sec_flt->sec_len <= 3)
            {
				sec_flt->stat_info.SecDataTooShortCnt++;

                dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS,
                                     DMX_ERR_SEC_FLT_BAD_SEC_LEN);
        
                return(DMX_ERR_SEC_FLT_BAD_SEC_LEN);
            }
        
            /* Case 1: Section just finished within this TS packet.
             */
            if (ts_remain_len == sec_flt->sec_len)
            {
                ret = dmx_sec_flt_wr_pkt_data(sec_flt, rd_addr, ts_remain_len);
                
                dmx_sec_flt_wr_pkt_end(sec_flt, ret);

                dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS, ret);

                /* Error. 
                 */
                if (ret < 0)
                {
                    return(ret);
                }

                return(rd_addr - (ts_payload + 1 + sec_pointer));
            }

            /* Case 2: Section longer than this TS palyload, wait for next TS
             * pakcet.
             */
            if (ts_remain_len < sec_flt->sec_len)
            {
                ret = dmx_sec_flt_wr_pkt_data(sec_flt, rd_addr, ts_remain_len);

                /* Error. 
                 */
                if (ret < 0)
                {
                    dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS, ret);

                    return(ret);
                }

                /* OK, wait for next TS pakcet.
                 */
                dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_REMAIN_TS, ret);

                return(rd_addr - (ts_payload + 1 + sec_pointer));
            }

            /* Case 3: Section length is shorter than this TS playload,
             * continue to process next section within this TS paylaod.
             * (ts_remain_len > sec_len)
             */
            ret = dmx_sec_flt_wr_pkt_data(sec_flt, rd_addr, sec_flt->sec_len);

            dmx_sec_flt_wr_pkt_end(sec_flt, ret);

            rd_addr += sec_flt->sec_len;

            dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS, ret);
        }
    }

    return(DMX_ERR_OK);
}




__s32 dmx_sec_flt_register
(
    __u32                      dmx_id,
    __u16                      pid,
    struct Ali_DmxSecMaskInfo *mask,
    __s32 sec_data_cb          (__u8                     *src,
                                __s32                     len,
                                enum DMX_SEC_FLT_CB_TYPE  cb_type,
                                __u32                     cb_param),
    __u32                       cb_param
)
{
    __s32               sec_flt_idx;
    __s32               ts_flt_idx;
    struct dmx_sec_flt *sec_flt;

    /* Find a free sec filter. 
     */
    for (sec_flt_idx = 0; sec_flt_idx < DMX_SEC_FLT_CNT; sec_flt_idx++)
    {
        sec_flt = &(ali_dmx_sec_flt_module.sec_flt[sec_flt_idx]);

        if (DMX_SEC_FLT_STATE_IDLE == sec_flt->state)
        {
            break;
        }
    }

    if (sec_flt_idx >= DMX_SEC_FLT_CNT)
    {
        return(DMX_ERR_SEC_FLT_EXHAUST);
    }

    /* Keep track of parameters for this sec filter.
     */
    memset(sec_flt, 0, sizeof(struct dmx_sec_flt));

    sec_flt->pid = pid;

    sec_flt->sec_data_cb = sec_data_cb;

    sec_flt->cb_param = cb_param;

    if (NULL != mask)
    {
        memcpy(&sec_flt->MaskInfo, mask, sizeof(struct Ali_DmxSecMaskInfo));
    }

    /* Check if we have free ts filter for this sec filter.
     */
    ts_flt_idx = dmx_ts_flt_register(dmx_id, pid, dmx_sec_flt_parse, 
                                     sec_flt_idx);

    if (ts_flt_idx < 0)
    {
        return(ts_flt_idx);
    }

    sec_flt->dmx_id = dmx_id;

    sec_flt->ts_flt_idx = ts_flt_idx;

	sec_flt->stat_info.SecFltIdx = sec_flt_idx;

    dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_STOP, 0);

    return(sec_flt_idx); 
}





__s32 dmx_sec_flt_start
(
    __s32 flt_idx
)
{
    __s32               ret;
    struct dmx_sec_flt *sec_flt;

    sec_flt = &(ali_dmx_sec_flt_module.sec_flt[flt_idx]);

    if (DMX_SEC_FLT_STATE_STOP != sec_flt->state)
    {
        return(DMX_ERR_SEC_FLT_OPERATION_DENIED);
    }
    
    ret = dmx_ts_flt_start(sec_flt->ts_flt_idx);
    
    if (ret < 0)
    {
        return(ret);
    }
    
    dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_FIRST_TS, 0);
    
    return(DMX_ERR_OK);
}



__s32 dmx_sec_flt_stop
(
    __s32 flt_idx
)
{
    __s32               ret;
    struct dmx_sec_flt *sec_flt;

    sec_flt = &(ali_dmx_sec_flt_module.sec_flt[flt_idx]);

    if ((DMX_SEC_FLT_STATE_FIRST_TS  != sec_flt->state) &&
        (DMX_SEC_FLT_STATE_REMAIN_TS != sec_flt->state))
    {
        return(DMX_ERR_SEC_FLT_OPERATION_DENIED);
    }

    ret = dmx_ts_flt_stop(sec_flt->ts_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_STOP, 0);

    return(0);
}





__s32 dmx_sec_flt_unregister
(
    __s32 flt_idx
)
{
    __s32               ret;
    struct dmx_sec_flt *sec_flt;

    sec_flt = &(ali_dmx_sec_flt_module.sec_flt[flt_idx]);

    if (DMX_SEC_FLT_STATE_IDLE == sec_flt->state)
    {
        return(DMX_ERR_SEC_FLT_OPERATION_DENIED);
    }

    ret = dmx_ts_flt_unregister(sec_flt->ts_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    dmx_sec_flt_state_set(sec_flt, DMX_SEC_FLT_STATE_IDLE, 0);

    return(0);
}


__u32 dmx_sec_flt_link_ts_flt_idx
(
    __s32 sec_flt_idx
)
{
	return(ali_dmx_sec_flt_module.sec_flt[sec_flt_idx].ts_flt_idx);
}


__s32 dmx_sec_flt_module_init
(
    void
)
{
    memset(&ali_dmx_sec_flt_module, 0, sizeof(struct dmx_sec_flt_module));

    return(DMX_ERR_OK);
}



