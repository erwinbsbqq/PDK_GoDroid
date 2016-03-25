

#include <linux/version.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#include <linux/sched.h>
#include <linux/sched/rt.h>
#else
#include <linux/sched.h>
#endif
#include <linux/kthread.h>
#include <ali_cache.h>

#include "dmx_stack.h"


struct dmx_data_engine_module ali_dmx_data_engine_module;
struct Ali_DmxKernGlobalStatInfo g_stat_info;


#if 0
__s32 dmx_hw_buf_invalidate
(
    __u32  hw_buf_base,
    __u32  rd_idx,
    __u32  wr_idx,
    __u32  end_idx
)
{
    __u32 inv_len;

    /* Invalidate cach in HW buffer.
     */
    if (rd_idx > wr_idx)
    {
        /* rd -> end
        */
        inv_len = (end_idx - rd_idx) * 188;
    
        if (0 != inv_len)
        {
            __CACHE_INV_ALI(hw_buf_base + (rd_idx * 188), inv_len);
        }

        /* start -> wr
        */  
        inv_len = wr_idx * 188;
    
        if (0 != inv_len)
        {
            __CACHE_INV_ALI(hw_buf_base, inv_len);
        }
    }
    else
    {
        /* rd -> wr
        */
        inv_len = (wr_idx - rd_idx) * 188;
    
        if (0 != inv_len)
        {
            __CACHE_INV_ALI(hw_buf_base + (rd_idx * 188), inv_len);
        }
    }

    return(0);
}
#endif


/*****************************************************************************/
/**
*
* Retrieve TS packets from demux hw buffer, dispatch them to upper layer.
*
* @param
* - param1: Not used.
*
* - param2: Not used
*
* @note
*
* @return
* - NONE
*
******************************************************************************/
__s32 dmx_data_engine_output_task_from_kern
(
    void *param
)
{
    __s32                          ret;
    __u32                          hw_buf_rd;
    __u32                          hw_buf_wr;
    __u32                          next_hw_buf_wr;
    __u32                          hw_buf_end_idx;
    __u32                          hw_buf_start_addr;
    __u8                          *ts_addr;
    struct dmx_data_engine_output *engine;

    engine = (struct dmx_data_engine_output *)param;

    hw_buf_start_addr = dmx_hw_buf_start_addr_get(engine->src_hw_interface_id);
    
    hw_buf_end_idx = dmx_hw_buf_end_get(engine->src_hw_interface_id);

    for(;;)
    {
        /* Check dmx buffer every DMX_DATA_ENG_OUTPUT_INTERVAL miliseconds.
         * This should be enough to meet all time reqirments.
         */
        msleep_interruptible(DMX_DATA_ENG_OUTPUT_INTERVAL);

        /* Loop to process TS packets, copy the TS packet payload to 
         * upper layer buffers.
         */
        hw_buf_rd = dmx_hw_buf_rd_get(engine->src_hw_interface_id);

        hw_buf_wr = dmx_hw_buf_wr_get(engine->src_hw_interface_id);

        /* Check if buffer is empty.
        */
        if (hw_buf_rd == hw_buf_wr)
        {
            g_stat_info.DmxBufEmptyCnt++;

            continue;
        }

        if (DMX_DATA_ENGINE_SRC_REAL_HW == engine->src_type)
        {
            /* Check if dmx hw buffer is overflow, clean hw buffer if it is.
            */
            next_hw_buf_wr = hw_buf_wr + 1;
        
            if (next_hw_buf_wr > hw_buf_end_idx)
            {
                next_hw_buf_wr = 0;
            }
            
            if (hw_buf_rd == next_hw_buf_wr)
            {
                g_stat_info.OverlapCnt++;

                printk("hw dmx:%d, hw buf overflow,rd:%d,wr:%d.\n",
                       engine->src_hw_interface_id, hw_buf_rd, hw_buf_wr);
                
                dmx_hw_buf_rd_set(engine->src_hw_interface_id, hw_buf_wr);
            
                continue;
            }

            #if 0
            dmx_hw_buf_invalidate(hw_buf_start_addr, hw_buf_rd, hw_buf_wr,
                                  hw_buf_end_idx);
            #endif
        }

        ts_addr = (__u8 *)(hw_buf_start_addr + (hw_buf_rd * 188));

        /* Lock entire DMX until all the data contained in HW buffer has been
         * parsed for safty reason.
         */
        ret = dmx_mutex_output_lock(engine->src_hw_interface_id);

        if (0 != ret)
        {
            printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
            return(ret);
        }  		
        
        /* Parse TS packets from EXTERNAL TO MAIN buffer.
        */
        while(hw_buf_rd != hw_buf_wr)
        { 
            g_stat_info.TotalTsInCnt++;

            ret = dmx_pid_flt_parse(engine->src_hw_interface_id, ts_addr);

            /* Caution: 
             * No ts packet handling proccess is allowed to return
             * DMX_ERR_BUF_BUSY except dmx_see_buf_wr_ts(), because in PVR case,
             * SEE may need to hold dmx by no longer retrieving data from 
             * main2see buffer.
             */
            if (DMX_ERR_BUF_BUSY == ret)
            {
                //printk("%s,%d,hw dmx:%d,rd:%d,wr:%d.\n",__FUNCTION__, __LINE__,
                       //engine->src_hw_interface_id, hw_buf_rd, hw_buf_wr);

                g_stat_info.PlayBusyCnt++;

                break;
            }
            else if (DMX_ERR_OK != ret)
            {
                g_stat_info.NobodyCareCnt++;
            }

            /* Statistics.
            */
            engine->ts_in_cnt++;

            hw_buf_rd++;

            if (hw_buf_rd > hw_buf_end_idx)
            {
                hw_buf_rd = 0;

                ts_addr = (__u8 *)(hw_buf_start_addr);
            }
            else
            {
                ts_addr += 188;
            }

            /* Move hardware rd pointer. 
             */
            dmx_hw_buf_rd_set(engine->src_hw_interface_id, hw_buf_rd);
        }
        
        dmx_mutex_output_unlock(engine->src_hw_interface_id);
    }

    return(-__LINE__);
}





/*****************************************************************************/
/**
*
* Retrieve TS packets from demux hw buffer, dispatch them to upper layer.
*
* @param
* - param1: Not used.
*
* - param2: Not used
*
* @note
*
* @return
* - NONE
*
******************************************************************************/
__s32 dmx_data_engine_locate_ts_pkt
(
    struct dmx_data_engine_output *engine
)
{
    __s32  round;
    __u32  chk_pkt_len;
    __u8  *rd_addr;
    __u8  *wr_addr;
    __u8  *buf_start_addr;
    __u8  *buf_end_addr;
    __u8  *sync_find_ptr_1;
    __u8  *sync_find_ptr_2; 
    __u8  *sync_find_ptr_3;
    __u32  rd_idx;
    __u32  wr_idx;
    __u32  buf_end_idx; 

    /* If pakcet already synced, do nothing.
    */
    if (0 != engine->pkt_synced)
    {
        return(0);
    }

    rd_idx = dmx_hw_buf_rd_get(engine->src_hw_interface_id);

    wr_idx = dmx_hw_buf_wr_get(engine->src_hw_interface_id);

    /* If buffer is empty, do nothing.
    */
    if (rd_idx == wr_idx)
    {
        return(0);
    }

    sync_find_ptr_1 = NULL;
    sync_find_ptr_2 = NULL; 
    sync_find_ptr_3 = NULL;

    buf_start_addr = (__u8 *)dmx_hw_buf_start_addr_get(engine->src_hw_interface_id);

    rd_addr = buf_start_addr + rd_idx;

    wr_addr = buf_start_addr + wr_idx;  

    buf_end_idx = dmx_hw_buf_end_get(engine->src_hw_interface_id);
    
    buf_end_addr = buf_start_addr + buf_end_idx;

    sync_find_ptr_1 = rd_addr;

    for (round = 0; round < 2; round++)
    {
        if (0 == round)
        {
            printk("%s,%d\n", __FUNCTION__, __LINE__);
            
            chk_pkt_len = 188;
        }
        else if (1 == round)
        {
            printk("%s,%d\n", __FUNCTION__, __LINE__);
            
            chk_pkt_len = 204;
        }
        else
        {
            chk_pkt_len = 0;
            
            printk("%s,%d\n", __FUNCTION__, __LINE__);
            
            return(0);
        }

        sync_find_ptr_1 = rd_addr;

        if (rd_addr > wr_addr)
        {
            for (;;)
            {
                sync_find_ptr_2 = sync_find_ptr_1 + chk_pkt_len;
                
                if (sync_find_ptr_2 >= buf_end_addr)
                {
                    sync_find_ptr_2 = sync_find_ptr_2 - buf_end_addr + buf_start_addr;
    
                    if (sync_find_ptr_2 >= wr_addr)
                    {
                        printk("%s,%d,sync_find_ptr_1:%x,sync_find_ptr_2:%x,sync_find_ptr_3:%x,rd_addr:%x,wr_addr:%x\n",
                                __FUNCTION__, __LINE__, (__u32)sync_find_ptr_1, (__u32)sync_find_ptr_2,
                                (__u32)sync_find_ptr_3, (__u32)rd_addr, (__u32)wr_addr);
                        break;
                    }               
                }

                sync_find_ptr_3 = sync_find_ptr_2 + chk_pkt_len;
    
                if (sync_find_ptr_3 >= buf_end_addr)
                {
                    sync_find_ptr_3 = sync_find_ptr_3 - buf_end_addr + buf_start_addr;
    
                    if (sync_find_ptr_3 >= wr_addr)
                    {
                        printk("%s,%d,sync_find_ptr_1:%x,sync_find_ptr_2:%x,sync_find_ptr_3:%x,rd_addr:%x,wr_addr:%x\n",
                                __FUNCTION__, __LINE__, (__u32)sync_find_ptr_1, (__u32)sync_find_ptr_2,
                                (__u32)sync_find_ptr_3, (__u32)rd_addr, (__u32)wr_addr);
        
                        break;
                    }                   
                }
    
                if ((0x47 == *sync_find_ptr_1) && (0x47 == *sync_find_ptr_2) &&
                    (0x47 == *sync_find_ptr_3))
                {
                    printk("%s,%d,sync_find_ptr_1:%x,sync_find_ptr_2:%x,sync_find_ptr_3:%x,rd_addr:%x,wr_addr:%x,chk_pkt_len:%d\n",
                            __FUNCTION__, __LINE__, (__u32)sync_find_ptr_1, (__u32)sync_find_ptr_2,
                            (__u32)sync_find_ptr_3, (__u32)rd_addr, (__u32)wr_addr, chk_pkt_len);
    
                    /* Drop thrash data before the first found TS packet.
                    */
                    dmx_hw_buf_rd_set(engine->src_hw_interface_id, (__u32)(sync_find_ptr_1 - buf_start_addr));
    
                    /* Now we are in sync.
                    */
                    engine->pkt_synced = 1;
    
                    engine->pkt_len = chk_pkt_len;
                    
                    return(1);
                }       

                sync_find_ptr_1++;
            
                if (sync_find_ptr_1 >= buf_end_addr)
                {
                    sync_find_ptr_1 = buf_start_addr;
                } 
            }
        }
        else
        {
            for (;;)
            {
                sync_find_ptr_2 = sync_find_ptr_1 + chk_pkt_len;
    
                if (sync_find_ptr_2 >= wr_addr)
                {
                    printk("%s,%d,sync_find_ptr_1:%x,sync_find_ptr_2:%x,sync_find_ptr_3:%x,rd_addr:%x,wr_addr:%x\n",
                            __FUNCTION__, __LINE__, (__u32)sync_find_ptr_1, (__u32)sync_find_ptr_2,
                            (__u32)sync_find_ptr_3, (__u32)rd_addr, (__u32)wr_addr);
                    break;
                }               

                sync_find_ptr_3 = sync_find_ptr_2 + chk_pkt_len;
    
                if (sync_find_ptr_3 >= wr_addr)
                {
                    printk("%s,%d,sync_find_ptr_1:%x,sync_find_ptr_2:%x,sync_find_ptr_3:%x,rd_addr:%x,wr_addr:%x\n",
                            __FUNCTION__, __LINE__, (__u32)sync_find_ptr_1, (__u32)sync_find_ptr_2,
                            (__u32)sync_find_ptr_3, (__u32)rd_addr, (__u32)wr_addr);
    
                    break;
                }                   
    
                if ((0x47 == *sync_find_ptr_1) && (0x47 == *sync_find_ptr_2) &&
                    (0x47 == *sync_find_ptr_3))
                {
                    printk("%s,%d,sync_find_ptr_1:%x,sync_find_ptr_2:%x,sync_find_ptr_3:%x,rd_addr:%x,wr_addr:%x,chk_pkt_len:%d\n",
                            __FUNCTION__, __LINE__, (__u32)sync_find_ptr_1, (__u32)sync_find_ptr_2,
                            (__u32)sync_find_ptr_3, (__u32)rd_addr, (__u32)wr_addr, chk_pkt_len);
    
                    /* Drop thrash data before the first found TS packet.
                    */
                    dmx_hw_buf_rd_set(engine->src_hw_interface_id, (__u32)(sync_find_ptr_1 - buf_start_addr));
    
                    /* Now we are in sync.
                    */
                    engine->pkt_synced = 1;
    
                    engine->pkt_len = chk_pkt_len;
                    
                    return(1);
                }       

                sync_find_ptr_1++;
            
                if (sync_find_ptr_1 >= buf_end_addr)
                {
                    sync_find_ptr_1 = buf_start_addr;
                } 
            }
        }
    }

    printk("%s,%d,sync_find_ptr_1:%x,sync_find_ptr_2:%x,sync_find_ptr_3:%x,rd_addr:%x,wr_addr:%x,chk_pkt_len:%d\n",
            __FUNCTION__, __LINE__, (__u32)sync_find_ptr_1, (__u32)sync_find_ptr_2,
            (__u32)sync_find_ptr_3, (__u32)rd_addr, (__u32)wr_addr, chk_pkt_len);

    /* Drop thrash data if no packet found.
    */
    dmx_hw_buf_rd_set(engine->src_hw_interface_id, (__u32)(sync_find_ptr_1 - buf_start_addr));   

    return(0);
}









__s32 dmx_data_engine_send_ts_pkt
(
    struct dmx_data_engine_output *engine,
    __u8 *pkt_addr
)
{
    __s32 ret;
    
    /* 0xAA is used for ALI PVR, nasty.
    */
    if ((0x47 != pkt_addr[0]) && (0xAA != pkt_addr[0]))
    {
        engine->pkt_synced = 0;

        printk("%s,%d,rd_addr:%x,rd_addr[0]:%02x %02x %02x %02x %02x\n",
               __FUNCTION__, __LINE__, (__u32)pkt_addr, pkt_addr[0],
               pkt_addr[1], pkt_addr[2], pkt_addr[3], pkt_addr[4]);
        
        return(DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT);
    }
    
    ret = dmx_pid_flt_parse(engine->src_hw_interface_id, pkt_addr);

    return(ret);
}





__s32 dmx_data_engine_output_task_from_usr
(
    void *param
)
{
    __s32                          ret;
    __u8                          *rd_addr;
    __u8                          *wr_addr;
    __u8                          *end_addr;    
    __u8                          *next_rd_addr;
    __u8                          *buf_start_addr;
    __u32                          rd_idx;
    __u32                          wr_idx;
    __u32                          end_idx;
    struct dmx_data_engine_output *engine;
    __u32                          data_len;
    __u32                          runback_need_len;

    engine = (struct dmx_data_engine_output *)param;

    buf_start_addr = (__u8 *)dmx_hw_buf_start_addr_get(engine->src_hw_interface_id);

    end_idx = dmx_hw_buf_end_get(engine->src_hw_interface_id);

    end_addr = buf_start_addr + end_idx;

    printk("%s,%d,buf_start_addr:%x,end_addr:%x\n",
           __FUNCTION__, __LINE__, (__u32)buf_start_addr, (__u32)end_addr);  	
	
    for(;;)
    {
        while (0 == engine->pkt_synced)
        {
            //printk("%s,%d\n", __FUNCTION__, __LINE__);
            
            msleep_interruptible(DMX_DATA_ENG_OUTPUT_INTERVAL);
                        
            dmx_data_engine_locate_ts_pkt(engine);
        }

        /* Lock entire DMX until all the data contained in HW buffer has been
         * parsed for safe.
         */
        ret = dmx_mutex_output_lock(engine->src_hw_interface_id);
		
        if (0 != ret)
        {
            printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
            return(ret);
        }  

        rd_idx = dmx_hw_buf_rd_get(engine->src_hw_interface_id);
        
        wr_idx = dmx_hw_buf_wr_get(engine->src_hw_interface_id);

        rd_addr = buf_start_addr + rd_idx;

        wr_addr = buf_start_addr + wr_idx;

        //printk("%s,%d\n", __FUNCTION__, __LINE__);

        if(rd_addr == wr_addr)
        {           
            goto NEXT_LOOP;
        }

        /* rd ==> end then rd = 0
        */
        if (rd_addr > wr_addr)
        {  
            /* Process run-back pakcet which occupies the end and start of ring
             * buffer.
             */
            if(engine->runback_pkt_buf_wr > 0)
            {
                data_len = wr_addr - end_addr;
                
                runback_need_len = engine->pkt_len - engine->runback_pkt_buf_wr;
                
                if (data_len < runback_need_len)
                {
                    memcpy(engine->runback_pkt_buf + engine->runback_pkt_buf_wr,
                           rd_addr, data_len);
                
                    engine->runback_pkt_buf_wr += data_len;
                
                    rd_addr = buf_start_addr;
                
                    goto NEXT_LOOP;
                }
                /* Send runback pakcet.
                */
                else
                {
                    memcpy(engine->runback_pkt_buf + engine->runback_pkt_buf_wr,
                           rd_addr, runback_need_len);
                    
                    ret = dmx_data_engine_send_ts_pkt(engine, 
                                                      engine->runback_pkt_buf);

                    if (DMX_ERR_BUF_BUSY == ret)
                    {
                        goto NEXT_LOOP;
                    }
                    
                    /* If the TS pakcet in runback buffer is bad, drop this packet.
                     * Doing this may loss at most 188 bytes which should not be dropped, but
                     * the code may be greatly simplified.
                     * In the case of bad TS pakcet, dropping 188 bytes should be affordable.
                    */
                    if (DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret)
                    {
                        printk("%s,%d,buf_start_addr:%x,end_addr:%x,rd_addr:%x,wr_addr:%x\n",
                               __FUNCTION__, __LINE__, (__u32)buf_start_addr, (__u32)end_addr,
                               (__u32)rd_addr, (__u32)wr_addr);  
                    }                   
                
                    engine->runback_pkt_buf_wr = 0;
                    
                    rd_addr += runback_need_len;
                }
            }

            /* Process normal pakcet.
             */
            next_rd_addr = rd_addr + engine->pkt_len;

            for (;;)
            {
                /* send pakcet in the ring buffer.
                 */
                if (next_rd_addr < end_addr)
                {
                    ret = dmx_data_engine_send_ts_pkt(engine, rd_addr);

                    if (DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret)
                    {
                        printk("%s,%d,buf_start_addr:%x,end_addr:%x,rd_addr:%x,wr_addr:%x\n",
                               __FUNCTION__, __LINE__, (__u32)buf_start_addr, (__u32)end_addr,
                               (__u32)rd_addr, (__u32)wr_addr);  
                    }                   

                    if ((DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret) || 
                        (DMX_ERR_BUF_BUSY == ret))
                    {
                        goto NEXT_LOOP;
                    }
                }
                /* pakcet just complete at boundary, send all pakcet and wait
                 * for next rund.
                 */ 
                else if (next_rd_addr == end_addr)
                {
                    ret = dmx_data_engine_send_ts_pkt(engine, rd_addr);

                    if (DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret)
                    {
                        printk("%s,%d,buf_start_addr:%x,end_addr:%x,rd_addr:%x,wr_addr:%x\n",
                               __FUNCTION__, __LINE__, (__u32)buf_start_addr, (__u32)end_addr,
                               (__u32)rd_addr, (__u32)wr_addr);  
                    }
                    
                    if ((DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret) || 
                        (DMX_ERR_BUF_BUSY == ret))
                    {
                        goto NEXT_LOOP;
                    }

                    break;
                }
                /* we have a run-back pakcet which occupies the end and the 
                 * start of the ring buffer, copy it a temporay buffer for 
                 * later sending it.
                 * At most one TS packet in runback_pkt_buf.
                */
                else 
                {
                    engine->runback_pkt_buf_wr = end_addr - rd_addr;
                    
                    memcpy(engine->runback_pkt_buf, rd_addr, 
                           engine->runback_pkt_buf_wr);

                    break;
                }
                
                rd_addr = next_rd_addr;

                next_rd_addr += engine->pkt_len;
            }
            
            rd_addr = buf_start_addr;
        }

        /* rd ==> wr
        */
        if (rd_addr < wr_addr)
        {
            /* Process run-back pakcet which occupies the end and start of ring
             * buffer.
             */
            if(engine->runback_pkt_buf_wr > 0)
            {
                data_len = wr_addr - rd_addr;
                
                runback_need_len = engine->pkt_len - engine->runback_pkt_buf_wr;
                
                if (data_len < runback_need_len)
                {
                    memcpy(engine->runback_pkt_buf + engine->runback_pkt_buf_wr,
                           rd_addr, data_len);
                
                    engine->runback_pkt_buf_wr += data_len;
                
                    rd_addr = wr_addr;
                
                    goto NEXT_LOOP;
                }
                /* Send runback pakcet.
                */
                else
                {
                    memcpy(engine->runback_pkt_buf + engine->runback_pkt_buf_wr,
                           rd_addr, runback_need_len);
                    
                    ret = dmx_data_engine_send_ts_pkt(engine, 
                                                      engine->runback_pkt_buf);

                    if (DMX_ERR_BUF_BUSY == ret)
                    {
                        goto NEXT_LOOP;
                    }
                    
                    /* If the TS pakcet in runback buffer is bad, drop this packet.
                     * Doing this may loss at most 188 bytes which should not be dropped, but
                     * the code may be greatly simplified.
                     * In the case of bad TS pakcet, dropping 188 bytes should be affordable.
                    */
                    if (DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret)
                    {
                        printk("%s,%d,buf_start_addr:%x,end_addr:%x,rd_addr:%x,wr_addr:%x\n",
                               __FUNCTION__, __LINE__, (__u32)buf_start_addr, (__u32)end_addr,
                               (__u32)rd_addr, (__u32)wr_addr);   
                    }

                    engine->runback_pkt_buf_wr = 0;
                    
                    rd_addr += runback_need_len;
                }
            }

            /* Process normal pakcet.
             */     
            next_rd_addr = rd_addr + engine->pkt_len;

            for (;;)
            {
                /* send pakcet in the ring buffer.
                */
                if (next_rd_addr < wr_addr)
                {
                    ret = dmx_data_engine_send_ts_pkt(engine, rd_addr);

                    if (DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret)
                    {
                        printk("%s,%d,buf_start_addr:%x,end_addr:%x,rd_addr:%x,wr_addr:%x\n",
                               __FUNCTION__, __LINE__, (__u32)buf_start_addr, (__u32)end_addr,
                               (__u32)rd_addr, (__u32)wr_addr);  
                    }
                    
                    if ((DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret) || 
                        (DMX_ERR_BUF_BUSY == ret))
                    {
                        goto NEXT_LOOP;
                    }

                    //printk("%s,%d,rd_addr:%x\n", __FUNCTION__, __LINE__, rd_addr);
                }
                /* pakcet just complete at boundary, send all pakcet and wait
                 * for next rund.
                 */             
                else if (next_rd_addr == wr_addr)
                {
                    ret = dmx_data_engine_send_ts_pkt(engine, rd_addr);

                    if (DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret)
                    {
                        printk("%s,%d,buf_start_addr:%x,end_addr:%x,rd_addr:%x,wr_addr:%x\n",
                               __FUNCTION__, __LINE__, (__u32)buf_start_addr, (__u32)end_addr,
                               (__u32)rd_addr, (__u32)wr_addr);  
                    }
                    
                    if ((DMX_ERR_DATA_ENGINE_NEED_RESYNC_PKT == ret) || 
                        (DMX_ERR_BUF_BUSY == ret))
                    {
                        goto NEXT_LOOP;
                    }

                    rd_addr = wr_addr;
                    
                    //printk("%s,%d,rd_addr:%x\n", __FUNCTION__, __LINE__, rd_addr);

                    break;
                }
                /* Pakcet not completely written by user, wait for next round
                 * for pakcet to be completely written by user.
                 */
                else 
                {                   
                    break;
                }
                
                rd_addr = next_rd_addr;

                next_rd_addr += engine->pkt_len;
            }
        }

NEXT_LOOP:
    
        //printk("%s,%d,(__u32)(new_rd_addr - buf_start_addr):%x\n", __FUNCTION__, __LINE__,(__u32)(new_rd_addr - buf_start_addr));

        dmx_hw_buf_rd_set(engine->src_hw_interface_id, (__u32)(rd_addr - buf_start_addr));

        dmx_mutex_output_unlock(engine->src_hw_interface_id);

        //printk("%s,%d\n", __FUNCTION__, __LINE__);

        /* Check dmx buffer every DMX_DATA_ENG_OUTPUT_INTERVAL miliseconds.
         * DMX_DATA_ENG_OUTPUT_INTERVAL should be enough to meet all time
         * reqirments.
         */
        msleep_interruptible(DMX_DATA_ENG_OUTPUT_INTERVAL);  
    }

    return(-__LINE__);
}



__u32 dmx_data_engine_output_ts_in_cnt_get
(
    __u32 src_hw_interface_id
)
{
    return(ali_dmx_data_engine_module.engine_output[src_hw_interface_id].ts_in_cnt);
}


static __s32 dmx_data_engine_thread_run
(
    __s32 (*engine_thread)(void *engine_thread_param),    
    void *engine_thread_param,    
    void *engine_thread_name
)
{
	struct sched_param param = {.sched_priority = MAX_RT_PRIO - 1};

	struct task_struct *p;

	p = kthread_create(engine_thread, engine_thread_param, engine_thread_name);

	if (IS_ERR(p))
	{
		return(PTR_ERR(p));	
	}

	sched_setscheduler(p, SCHED_RR, &param);

	wake_up_process(p);
	
	return(0);
}


__s32 dmx_data_engine_module_init_kern
(
    __u32                     src_hw_interface_id,
    __u8                     *engine_name,
    enum DMX_DATA_ENGINE_SRC  src_type
)
{
    struct dmx_data_engine_output *engine;
    
    if (src_hw_interface_id > DMX_LINUX_OUTPUT_DEV_CNT)
    {
        return(DMX_ERR_DATA_ENGINE_INIT_FAIL);
    }
    
    engine = &ali_dmx_data_engine_module.engine_output[src_hw_interface_id];
    
    if (engine->state != DMX_DATA_ENGINE_TASK_HW_STATE_IDLE)
    {
        return(DMX_ERR_DATA_ENGINE_INIT_FAIL);
    }

    engine->src_type = src_type;

    engine->pkt_synced = 0;

    engine->pkt_len = 0;

    engine->src_hw_interface_id = src_hw_interface_id;

    engine->state = DMX_DATA_ENGINE_TASK_HW_STATE_RUN;

    dmx_data_engine_thread_run(dmx_data_engine_output_task_from_kern, engine, engine_name);

    return(0);
}



__s32 dmx_data_engine_module_init_usr
(
    __u32                     src_hw_interface_id,
    __u8                     *engine_name,
    enum DMX_DATA_ENGINE_SRC  src_type
)
{
    struct dmx_data_engine_output *engine;
    
    if (src_hw_interface_id > DMX_LINUX_OUTPUT_DEV_CNT)
    {
        return(DMX_ERR_DATA_ENGINE_INIT_FAIL);
    }
    
    engine = &ali_dmx_data_engine_module.engine_output[src_hw_interface_id];
    
    if (engine->state != DMX_DATA_ENGINE_TASK_HW_STATE_IDLE)
    {
        return(DMX_ERR_DATA_ENGINE_INIT_FAIL);
    }

    engine->src_type = src_type;

    engine->pkt_synced = 0;

    engine->pkt_len = 0;

    engine->src_hw_interface_id = src_hw_interface_id;

    engine->runback_pkt_buf_len = DMX_DATA_ENG_RUNBACK_BUF_LEN;

    engine->runback_pkt_buf_wr = 0;

    engine->runback_pkt_buf = vmalloc(engine->runback_pkt_buf_len);

    if (NULL == engine->runback_pkt_buf)
    {
        panic("%s,%d\n", __FUNCTION__, __LINE__);
    }

    engine->state = DMX_DATA_ENGINE_TASK_HW_STATE_RUN;

    dmx_data_engine_thread_run(dmx_data_engine_output_task_from_usr, engine, engine_name);

    return(0);
}


