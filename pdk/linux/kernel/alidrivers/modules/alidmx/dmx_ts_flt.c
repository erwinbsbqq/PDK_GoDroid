#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/delay.h>

#include "dmx_stack.h"

extern struct Ali_DmxKernGlobalStatInfo g_stat_info;
struct dmx_ts_flt_module ali_dmx_ts_flt_module;

__s32 dmx_ts_flt_chk_ts_pkt
(
    struct dmx_ts_flt     *ts_flt,
    struct dmx_ts_pkt_inf *pkt_inf
)
{
    __u32              conti_cnt_expect;

    if (pkt_inf->scramble_flag != 0)
    {
    	ts_flt->stat_info.TsScrmbCnt++;
    }

    pkt_inf->continuity = DMX_TS_PKT_CONTINU_OK;   

    /* Check TS pakcet continuty.
    */
    if (((TS_PAYLOAD_ONLY == pkt_inf->adapt_ctrl)      || 
        (TS_ADAPT_AND_PAYLOAD == pkt_inf->adapt_ctrl)) &&
        (0 == pkt_inf->disconti_flag))
    {
        /* Check duplicated TS packets.
        */
        if ((pkt_inf->conti_cnt == ts_flt->last_ts_conti_cnt) &&
            (pkt_inf->adapt_ctrl == ts_flt->last_ts_adapt_ctrl))
        
        {
	    	ts_flt->stat_info.TsDupCnt++;

            pkt_inf->continuity = DMX_TS_PKT_CONTINU_DUPLICATE; 
        }
        /* Check TS packet continuty only if current TS pakcet contains no PUSI
         * (Packet Unit Start Indicator) bit. This restriction is to keep 
         * compartibility with some streams which does not follow the
         * iso-13818.1 stricly.
         */
        else if ((0 == pkt_inf->unit_start) && (0 == pkt_inf->disconti_flag))
        {
            conti_cnt_expect = ts_flt->last_ts_conti_cnt + 1;
        
            if (conti_cnt_expect > 0xF)
            {
                conti_cnt_expect = 0;
            }
        
            if (conti_cnt_expect != pkt_inf->conti_cnt)
            {
		    	ts_flt->stat_info.TsLostCnt++;

                pkt_inf->continuity = DMX_TS_PKT_CONTINU_LOST;

				if (g_stat_info.RealTimePrintEn)
				{
					printk("%s,dmx_id:%d,pid:%d,exp:%d,cur:%d,hw_rd:%d,hw_wr:%d,j:%x\n", 
						__FUNCTION__, ts_flt->dmx_id, pkt_inf->pid, conti_cnt_expect, pkt_inf->conti_cnt, 
						dmx_hw_buf_rd_get(ts_flt->dmx_id), dmx_hw_buf_wr_get(ts_flt->dmx_id), (__u32)jiffies);
				}
            }
        }
    }
    
    /* Prepare for next TS packet checking. 
    */
    ts_flt->last_ts_adapt_ctrl = pkt_inf->adapt_ctrl;
    ts_flt->last_ts_conti_cnt = pkt_inf->conti_cnt;

    return(DMX_ERR_OK);
}



__s32 dmx_ts_flt_pkt_info_retrieve
(
    struct dmx_ts_flt     *ts_flt,
    struct dmx_ts_pkt_inf *pkt_inf
)
{  
    if ((pkt_inf->pkt_addr[0] != 0x47) && (pkt_inf->pkt_addr[0] != 0xAA))
    {     
		ts_flt->stat_info.TsSyncByteErrCnt++;

        printk("%s, %d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_TS_FLT_SYNC_BYTE_ERR);
    }

    /* TS pakcet error indicator.
     */
    if (((pkt_inf->pkt_addr[1] & 0x80) >> 7) != 0)
    { 
		ts_flt->stat_info.TsErrCnt++;

//        printk("%s(), %d, error indicator. \n", __FUNCTION__, __LINE__);

        return(DMX_ERR_TS_FLT_PKT_ERR);
    }

    pkt_inf->unit_start = (pkt_inf->pkt_addr[1] & 0x40) >> 6;
 
    pkt_inf->pid = ((pkt_inf->pkt_addr[1] & 0x1F) << 8) | pkt_inf->pkt_addr[2];

    pkt_inf->scramble_flag = (pkt_inf->pkt_addr[3] & 0xC0) >> 6;

    pkt_inf->conti_cnt = pkt_inf->pkt_addr[3] & 0x0F;

    /* TS pakcet scramble flag.
     */
    pkt_inf->adapt_ctrl = (pkt_inf->pkt_addr[3] & 0x30) >> 4;

    /* Locate adaptation field and payload.
     */
    if ((TS_ADAPT_ONLY        ==  pkt_inf->adapt_ctrl) ||
        (TS_ADAPT_AND_PAYLOAD == pkt_inf->adapt_ctrl))
    {
        pkt_inf->adapt_addr = pkt_inf->pkt_addr + 4;

        pkt_inf->adapt_len = pkt_inf->pkt_addr[4] + 1;

        pkt_inf->payload_addr = pkt_inf->adapt_addr + pkt_inf->adapt_len;

        pkt_inf->payload_len = 184 - pkt_inf->adapt_len;

        if (0 != pkt_inf->adapt_len)
        {
            pkt_inf->disconti_flag = ((pkt_inf->adapt_addr[1] & 0x80) >> 7);
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

    return(DMX_ERR_OK);
}






__s32 dmx_ts_flt_parse
(
    struct dmx_ts_pkt_inf *pkt_inf,
    __u32                  flt_idx
)
{
    __s32              ret;
    struct dmx_ts_flt *ts_flt;

    ts_flt = &(ali_dmx_ts_flt_module.ts_flt[flt_idx]);

	ts_flt->stat_info.TsInCnt++;

    ret = dmx_ts_flt_pkt_info_retrieve(ts_flt, pkt_inf);

    if (ret < 0)
    {
        return(ret);
    }

    dmx_ts_flt_chk_ts_pkt(ts_flt, pkt_inf);

    ret = ts_flt->ts_pkt_cb(pkt_inf, ts_flt->cb_param);

    return(ret);
}







__s32 dmx_ts_flt_register
(
    __u32  dmx_id,
    __u16  pid,
    __s32  (*ts_pkt_cb)(struct dmx_ts_pkt_inf *pkt_inf,
                        __u32                  cb_param),
    __u32  cb_param
)
{
    __s32              ret;
    __u32              idx;
    struct dmx_ts_flt *ts_flt;
	
    /* Find an idle ts ts_service slot.
     */
    for (idx = 0; idx < DMX_TS_FLT_CNT; idx++)
    {
        ts_flt = &(ali_dmx_ts_flt_module.ts_flt[idx]);

        if (DMX_TS_FLT_STATE_IDLE == ts_flt->state)
        {
            break;
        }
    }

    /* All ts ts_service slot occupied.
     */
    if (idx >= DMX_TS_FLT_CNT)
    {
        return(DMX_ERR_TS_FLT_EXHAUST);
    }

    ret = dmx_pid_flt_register(dmx_id, pid, dmx_ts_flt_parse, idx);

    /* All hw pid filter occupied. 
     */
    if (ret < 0)
    {
        return(ret);
    }

    memset(ts_flt, 0, sizeof(struct dmx_ts_flt));

    /* Keep track of ts filter for this ts filters.
     */        
    ts_flt->dmx_id = dmx_id;
    
    ts_flt->pid = pid;

    ts_flt->pid_flt_idx = ret;

    ts_flt->ts_pkt_cb = ts_pkt_cb;

    ts_flt->cb_param = cb_param;

    ts_flt->stat_info.TsFltIdx = idx;

    ts_flt->state = DMX_TS_FLT_STATE_STOP;  

    return(idx); 
}





__s32 dmx_ts_flt_start
(
    __s32 flt_idx
)
{
    __s32              ret;
    struct dmx_ts_flt *ts_flt;

    ts_flt = &(ali_dmx_ts_flt_module.ts_flt[flt_idx]);

    if (DMX_TS_FLT_STATE_STOP != ts_flt->state)
    {
        return(DMX_ERR_TS_FLT_OPERATION_DENIED);
    }

    ret = dmx_pid_flt_start(ts_flt->pid_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    ts_flt->state = DMX_TS_FLT_STATE_RUN;   

    return(0);
}





__s32 dmx_ts_flt_stop
(
    __s32 flt_idx
)
{
    __s32              ret;
    struct dmx_ts_flt *ts_flt;

    ts_flt = &(ali_dmx_ts_flt_module.ts_flt[flt_idx]);

    if (DMX_TS_FLT_STATE_RUN != ts_flt->state)
    {
        return(DMX_ERR_TS_FLT_OPERATION_DENIED);
    }

    ret = dmx_pid_flt_stop(ts_flt->pid_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    ts_flt->state = DMX_TS_FLT_STATE_STOP;  

    return(0);
}



__s32 dmx_ts_flt_unregister
(
    __s32 flt_idx
)
{
    __s32              ret;
    struct dmx_ts_flt *ts_flt;

    ts_flt = &(ali_dmx_ts_flt_module.ts_flt[flt_idx]);

    if (DMX_TS_FLT_STATE_IDLE == ts_flt->state)
    {
        return(DMX_ERR_TS_FLT_OPERATION_DENIED);
    }

    ret = dmx_pid_flt_unregister(ts_flt->pid_flt_idx);

    if (ret < 0)
    {
        return(ret);
    }

    ts_flt->state = DMX_TS_FLT_STATE_IDLE;  

    return(0);
}



__u32 dmx_ts_flt_link_pid_flt_idx
(
    __s32 ts_flt_idx
)
{
	return(ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].pid_flt_idx);
}


__s32 dmx_ts_flt_module_init
(
    void
)
{   
    memset(&ali_dmx_ts_flt_module, 0, sizeof(struct dmx_ts_flt_module));

    return(0);
}



