
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h> 
#include <asm/io.h>
#include <ali_interrupt.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/delay.h>

#include <linux/mm.h>


#include "dmx_see_interface.h"
#include "dmx_stack.h"


struct dmx_stream_module ali_dmx_stream_module;
extern struct dmx_ts_flt_module ali_dmx_ts_flt_module;
extern struct dmx_pcr_flt_module ali_dmx_pcr_flt_module;
extern struct dmx_sec_flt_module ali_dmx_sec_flt_module;
extern struct dmx_see_device ali_dmx_see_dev[1];
extern struct Ali_DmxKernGlobalStatInfo g_stat_info;
extern void dmx_internal_init(void);

__s32 dmx_sec_stream_write
(
    __u8                     *src,
    __s32                     len,
    enum DMX_SEC_FLT_CB_TYPE  cb_type,
    __u32                     param
)
{
    __u32                    pkt_len;
    __s32                    ret;
    struct ali_dmx_data_buf *dest_buf;
    struct dmx_stream       *stream;
	struct Ali_DmxDrvSecStrmStatInfo *stat_info;

    ret = 0;

    stream = (struct dmx_stream *)param;
	
    dest_buf = &(stream->data_buf);

	stat_info = &stream->detail.sec_stream.stat_info;

    if (DMX_SEC_FLT_CB_TYPE_ERR == cb_type)
    {
    	stat_info->CbTypeErrCnt++;

    	DMX_API_DBG("%s,%d,PID:%d,stream:%d\n", __FUNCTION__, __LINE__, 
    		        stream->detail.sec_stream.param.Pid,stream->stream_id);	
		
        dmx_data_buf_drop_incomplete_pkt(dest_buf);

        return(0);
    }
    else if (DMX_SEC_FLT_CB_TYPE_PKT_DATA == cb_type)
    {
        ret = dmx_data_buf_wr_data(dest_buf, src, len, DMX_DATA_SRC_TYPE_KERN);
    
        if (ret < len)
        {
        	stat_info->WrByteErrCnt++;
			
            printk("%s,%d,streamid:%d,pid:%d,wr len:%d,buf len:%d,ret:%d\n",
				   __FUNCTION__, __LINE__, stream->stream_id, 
				   stream->detail.sec_stream.param.Pid, len, dest_buf->cur_len,
				   ret);
            
            //dmx_data_buf_drop_incomplete_pkt(dest_buf);
            
            /* Must be buffer overflow, flush all section data of this stream 
             * to free memory.
			 */
			dmx_data_buf_flush_all_pkt(dest_buf);
    
            return(ret);
        }
    }
    /*DMX_SEC_FLT_CB_TYPE_PKT_END == cb_type 
    */
    else 
    {   
       pkt_len = dmx_data_buf_first_pkt_len(dest_buf);
       
       ret = dmx_data_buf_wr_pkt_end(dest_buf);

       /* Wakeup waiting poll if following conditions are all true:
       *  1, Section data has been successfully stored in buffer;
       *  2, we are storing in an empty buffer(to offload CPU loading by 
       *     decreasing number of calling to wake_up_interruptible()).
       */
       if ((0 >= ret) && (pkt_len <= 0))
       {
           wake_up_interruptible(&(dest_buf->rd_wq));
       }
    }

    return(ret);
}


__s32 dmx_sec_stream_read
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    __s32                     flags,
    char __user              *usr_buf,
    size_t                    usr_rd_len
)
{
    __u32 pkt_len;
	struct Ali_DmxDrvSecStrmStatInfo *stat_info; 

	stat_info = &stream->detail.sec_stream.stat_info;

    if (stream->type != DMX_STREAM_TYPE_SEC)
    {
    	stat_info->StrTypeErrCnt++;
		
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }

    if (0 == usr_rd_len)
    {
        return(0);
    }    

    for (;;)
    {
        pkt_len = dmx_data_buf_first_pkt_len(&(stream->data_buf));

        if (pkt_len > 0)
        {
            if (pkt_len > usr_rd_len)
            {
                /* Do nothing if user buffer is shorter than pkt len.
                 */
		    	stat_info->RdBufShortCnt++;

                return(-EFBIG);
            }

            /* Else data could be read out. 
			*/
            break;
        }  

        /* pkt_len <= 0, then nothing could be read out.
        */
        if (flags & O_NONBLOCK)
        {
            return(-EAGAIN);
        }    

        #if 0
        dmx_mutex_output_unlock(6);
        #endif
		
        dmx_mutex_output_unlock(dev->src_hw_interface_id);

        if (wait_event_interruptible(stream->data_buf.rd_wq, 
            dmx_data_buf_first_pkt_len(&(stream->data_buf)) > 0))
        {
            return(-ERESTARTSYS);
        }

        if (dmx_mutex_output_lock(dev->src_hw_interface_id))
        {    
            return(-ERESTARTSYS);
        }    

        #if 0
        if (dmx_mutex_output_lock(6))
        {    
            return(-ERESTARTSYS);
        }   
		#endif
    }

    dmx_data_buf_rd(usr_buf, &(stream->data_buf), pkt_len, 
                    DMX_DATA_CPY_DEST_USER);

    return(pkt_len);
}






__s32 dmx_sec_stream_close
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvSecStrmStatInfo *stat_info; 

	DMX_API_DBG("%s,%d,PID:%d\n", __FUNCTION__, __LINE__, 
		        stream->detail.sec_stream.param.Pid);

	stat_info = &stream->detail.sec_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_SEC)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (DMX_STREAM_STATE_CFG == stream->state)
    {
        return(0);
    }

    /* Need Discrambling?
	*/
    if (1 == stream->detail.sec_stream.param.NeedDiscramble)
	{
		DMX_API_DBG("%s,%d,PID:%d\n", __FUNCTION__, __LINE__, 
		        stream->detail.sec_stream.param.Pid);
		
		ret = dmx_ts_flt_unregister(stream->detail.sec_stream.ts_flt_id2see);
		
		if (ret < 0)
		{
			DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
			
			return(ret);
		}

    	Sed_DmxSee2mainPidDel(stream->detail.sec_stream.param.Pid);
	}

    ret = dmx_sec_flt_unregister(stream->detail.sec_stream.sec_flt_id);

    if (ret < 0)
    {
    	stat_info->CloseFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

        return(ret);
    }

    dmx_data_buf_flush_all_pkt(&(stream->data_buf));

	DMX_API_DBG("%s,%d,PID:%d\n", __FUNCTION__, __LINE__, 
		        stream->detail.sec_stream.param.Pid);

    return(0);
}




__s32 dmx_sec_stream_cfg
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32 flt_id;
    __s32 ret;
    __u32 sec_src_hw_interface_id;
    __s32 ts_flt_id2see;
	struct Ali_DmxDrvSecStrmStatInfo *stat_info; 

	DMX_API_DBG("%s,%d\n", __FUNCTION__, __LINE__);

	stat_info = &stream->detail.sec_stream.stat_info;

	memset(stat_info, 0, sizeof(struct Ali_DmxDrvSecStrmStatInfo));
	
    /* Stream status validation. 
     */
    if ((DMX_STREAM_STATE_CFG  != stream->state) &&
        (DMX_STREAM_STATE_STOP != stream->state))
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }	

    if (DMX_STREAM_STATE_STOP == stream->state)
    {
        if (stream->type != DMX_STREAM_TYPE_SEC)
        {
        	stat_info->StrTypeErrCnt++;
    
            return(-EPERM);
        }    
		
        ret = dmx_sec_stream_close(dev, stream);

        if (ret < 0)
        {
	    	stat_info->CloseFailCnt++;

            DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
            return(ret);
        }
    }

    ret = copy_from_user(&(stream->detail.sec_stream.param),
                         (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
    	stat_info->CopyErrCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    dmx_data_buf_list_init(&(stream->data_buf));

    /* Need Discrambling?
	*/
    if (1 != stream->detail.sec_stream.param.NeedDiscramble)
    {
        /* No
		*/
		DMX_API_DBG("%s,%d,PID:%d,stream:%d\n", __FUNCTION__, __LINE__, 
					stream->detail.sec_stream.param.Pid,stream->stream_id); 
		
		sec_src_hw_interface_id = dev->src_hw_interface_id;
	}
	else
	{
	    /* Yes
		*/
		DMX_API_DBG("%s,%d,PID:%d,stream:%d\n", __FUNCTION__, __LINE__, 
					stream->detail.sec_stream.param.Pid,stream->stream_id); 
		
    	Sed_DmxSee2mainPidAdd(stream->detail.sec_stream.param.Pid);
		
		ts_flt_id2see = dmx_ts_flt_register(dev->src_hw_interface_id,
			                                stream->detail.sec_stream.param.Pid, 
								            dmx_see_buf_wr_ts, dev->src_hw_interface_id);
		
		if (ts_flt_id2see < 0)
		{
			DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
			
			return(ret);
		}

		stream->detail.sec_stream.ts_flt_id2see = ts_flt_id2see;  

    	sec_src_hw_interface_id = 6;
	}

	flt_id = dmx_sec_flt_register(sec_src_hw_interface_id, 
								  stream->detail.sec_stream.param.Pid, 
								  &(stream->detail.sec_stream.param.SecMask),
								  dmx_sec_stream_write,
								  (__u32)(stream));
	
	if (flt_id < 0)
	{
    	stat_info->CfgFailCnt++;

		DMX_API_DBG("%s,%d,flt_id:%d\n", __FUNCTION__, __LINE__, flt_id);
	
		return(ret);
	}	

    stream->detail.sec_stream.sec_flt_id = flt_id;  
	
    stream->type = DMX_STREAM_TYPE_SEC;

    stream->state = DMX_STREAM_STATE_STOP;

	DMX_API_DBG("%s,%d,PID:%d,stream:%d\n", __FUNCTION__, __LINE__, 
		        stream->detail.sec_stream.param.Pid,stream->stream_id);	

    return(0);
}






__s32 dmx_sec_stream_start
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32                             ret;
	struct Ali_DmxDrvSecStrmStatInfo *stat_info; 

	DMX_API_DBG("%s,%d,PID:%d,stream:%d\n", __FUNCTION__, __LINE__, 
		        stream->detail.sec_stream.param.Pid,stream->stream_id);	


	stat_info = &stream->detail.sec_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_SEC)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_STOP)
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }

    /* Need Discrambling?
	*/
    if (1 == stream->detail.sec_stream.param.NeedDiscramble)
	{
		ret = dmx_ts_flt_start(stream->detail.sec_stream.ts_flt_id2see);
		
		if (ret < 0)
		{
			DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
			
			return(ret);
		}
	}

    ret = dmx_sec_flt_start(stream->detail.sec_stream.sec_flt_id);

    if (ret < 0)
    {
	   	stat_info->StartFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

        return(ret);
    }

    stream->state = DMX_STREAM_STATE_RUN;

	DMX_API_DBG("%s,%d,PID:%d,stream:%d\n", __FUNCTION__, __LINE__, 
		        stream->detail.sec_stream.param.Pid,stream->stream_id);	

    return(0);
}






__s32 dmx_sec_stream_stop
(
    struct dmx_output_device  *dev,
    struct dmx_stream *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvSecStrmStatInfo *stat_info; 
	
	DMX_API_DBG("%s,%d,PID:%d\n", __FUNCTION__, __LINE__, 
		        stream->detail.sec_stream.param.Pid);

	stat_info = &stream->detail.sec_stream.stat_info;

    if (stream->type != DMX_STREAM_TYPE_SEC)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }

    /* Need Discrambling?
	*/
    if (1 == stream->detail.sec_stream.param.NeedDiscramble)
	{
		DMX_API_DBG("%s,%d,PID:%d,stream:%d\n", __FUNCTION__, __LINE__, 
					stream->detail.sec_stream.param.Pid,stream->stream_id); 

		ret = dmx_ts_flt_stop(stream->detail.sec_stream.ts_flt_id2see);
		
		if (ret < 0)
		{
			DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
			
			return(ret);
		}
	}

    ret = dmx_sec_flt_stop(stream->detail.sec_stream.sec_flt_id);

    if (ret < 0)
    {
	   	stat_info->StopFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

        return(ret);
    }

    dmx_data_buf_flush_all_pkt(&(stream->data_buf));

    stream->state = DMX_STREAM_STATE_STOP;

	DMX_API_DBG("%s,%d,PID:%d,stream:%d\n", __FUNCTION__, __LINE__, 
		        stream->detail.sec_stream.param.Pid,stream->stream_id);	

    return(0);
}






__s32 dmx_sec_stream_poll
(
    struct file              *filp,
    struct poll_table_struct *wait,
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __u32 pkt_len;
    __s32 mask;
    
    poll_wait(filp, &(stream->data_buf.rd_wq), wait);
      
    mask = 0;

    pkt_len = dmx_data_buf_first_pkt_len(&stream->data_buf);
    
    if (pkt_len > 0)
    {    
        mask |= (POLLIN | POLLRDNORM); 
    }

    return(mask);
}


__s32 dmx_sec_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                            ret;
	__s32                            sec_flt_idx;
	__u32                            ts_flt_idx;
	__u32                            pid_flt_idx;
	struct dmx_sec_flt               *sec_flt;
	struct dmx_ts_flt                *ts_flt;
	struct Ali_DmxDrvSecStrmStatInfo *p_StatInfo;

	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_SEC)
    {
        return(-EPERM);
    }	

    sec_flt_idx = stream->detail.sec_stream.sec_flt_id;

	ts_flt_idx = dmx_sec_flt_link_ts_flt_idx(sec_flt_idx);

	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	sec_flt = &ali_dmx_sec_flt_module.sec_flt[sec_flt_idx];

	ts_flt = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx];

	p_StatInfo = &stream->detail.sec_stream.stat_info;
	
	p_StatInfo->TsInCnt = ts_flt->stat_info.TsInCnt;

	p_StatInfo->SecInCnt = sec_flt->stat_info.SecInCnt;

	p_StatInfo->SecOutCnt = sec_flt->stat_info.SecOutCnt;
	
    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }
    
    return(0);
}


__s32 dmx_sec_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                           ret;
	__s32                           sec_flt_idx;
	__u32                           ts_flt_idx;
	__u32                           pid_flt_idx;
	struct dmx_sec_flt              *sec_flt;
	struct Ali_DmxDrvSecFltStatInfo *p_StatInfo;

	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_SEC)
    {
        return(-EPERM);
    }

	sec_flt_idx = stream->detail.sec_stream.sec_flt_id;

	ts_flt_idx = dmx_sec_flt_link_ts_flt_idx(sec_flt_idx);

	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	sec_flt = &ali_dmx_sec_flt_module.sec_flt[sec_flt_idx];

	p_StatInfo = &sec_flt->stat_info;
	
    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}



__s32 dmx_ts_stream_write
(
    struct dmx_ts_pkt_inf *pkt_inf,
    __u32                  param
)
{
    __u32                    data_len;
    __s32                    byte_cnt;
    struct ali_dmx_data_buf *dest_buf;
    struct dmx_stream        *stream;	
	struct Ali_DmxDrvTsStrmStatInfo *stat_info; 
	
	stream = (struct dmx_stream *)param;

    dest_buf = &(stream->data_buf);

    data_len = dmx_data_buf_total_len(dest_buf);

    byte_cnt = dmx_data_buf_wr_data(dest_buf, pkt_inf->pkt_addr, 188, 
                                    DMX_DATA_SRC_TYPE_KERN);

	stat_info = &stream->detail.ts_stream.stat_info;

	if (byte_cnt < 0)
	{
		stat_info->WrByteErrCnt++;
		
        printk("%s,%d,streamid:%d,pid:%d,wr len:%d,buf len:%d,ret:%d,pkt_inf:%x\n",
               __FUNCTION__, __LINE__, stream->stream_id, 
               stream->detail.ts_stream.param.PidList[0], 188, 
               dest_buf->cur_len, byte_cnt, (__u32)pkt_inf);

        /* Must be buffer overflow, flush all data of this stream 
         * to free memory.
		 */
		dmx_data_buf_flush_all_pkt(dest_buf);		
	}

    /* Wakeup waiting poll only if we are storing in an empty buffer to offload
     * CPU loading.
    */
    if (data_len <= 0)
    {
        //printk("%s,%d,data_len:%d\n", __FUNCTION__, __LINE__, data_len);
    	
        wake_up_interruptible(&(dest_buf->rd_wq));
    }

    return(byte_cnt);
}


__s32 dmx_ts_stream_read
(
    struct dmx_output_device  *dev,
    struct dmx_stream         *stream,
    __s32                      flags,
    char __user               *usr_buf,
    size_t                     usr_rd_len
)
{
    __s32 byte_cnt;
    __u32 ready_len;
	struct Ali_DmxDrvTsStrmStatInfo *stat_info; 

	stat_info = &stream->detail.ts_stream.stat_info;

    if (stream->type != DMX_STREAM_TYPE_TS)
    {
		stat_info->StrTypeErrCnt++;
		
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
		stat_info->StatErrCnt++;
		
        return(-EPERM);
    }

    if (0 == usr_rd_len)
    {
        return(0);
    } 

    for (;;)
    {
    	ready_len = dmx_data_buf_total_len(&(stream->data_buf));
    
    	if (ready_len > 0)
    	{
    		/* Data could be read out. 
    		*/
    		break;
    	}  
    
    	/* pkt_len <= 0, then nothing could be read out.
    	*/
    	if (flags & O_NONBLOCK)
    	{		   
    		return(-EAGAIN);
    	}	 

        #if 0
    	dmx_mutex_output_unlock(6);
        #endif
		
    	dmx_mutex_output_unlock(dev->src_hw_interface_id);
		
    	if (wait_event_interruptible(stream->data_buf.rd_wq, 
    		dmx_data_buf_total_len(&(stream->data_buf)) > 0))
    	{
    		return(-ERESTARTSYS);
    	}
			
    	if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    	{	 
    		return(-ERESTARTSYS);
    	}	

        #if 0
    	if (dmx_mutex_output_lock(6))
    	{	 
    		return(-ERESTARTSYS);
    	}	 
		#endif
    }
    
    /* byte_cnt may be less than need_cpy_len if data_buf contains less
     * data than need_cpy_len required.
     */
    byte_cnt = dmx_data_buf_rd(usr_buf, &(stream->data_buf), usr_rd_len, 
    						   DMX_DATA_CPY_DEST_USER); 
    
    if (byte_cnt < 0)
    {
		stat_info->RdByteErrCnt++;

    	printk("%s,%d,byte_cnt:%d\n", __FUNCTION__, __LINE__,
    				  byte_cnt);
    }
	    
    return(byte_cnt);
}

__s32 dmx_ts_stream_close
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32  i;
    __s32  ret;
    __s32 *ts_flt_id;
    __s32 *ts_flt_id2see;
	__u32 *pid_list;
	struct Ali_DmxDrvTsStrmStatInfo *stat_info; 

	stat_info = &stream->detail.ts_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_TS)
    {
		stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (DMX_STREAM_STATE_CFG == stream->state)
    {
        return(0);
    }

	ts_flt_id = stream->detail.ts_stream.ts_flt_id;

	ts_flt_id2see = stream->detail.ts_stream.ts_flt_id2see;

	pid_list = stream->detail.ts_stream.param.PidList;

    for (i = 0; i < stream->detail.ts_stream.param.PidCnt; i++) 
    {
        ret = dmx_ts_flt_unregister(ts_flt_id[i]);
    
        if (ret < 0)
        {
			stat_info->CloseFailCnt++;

            DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
            return(ret);
        }

		/* Need Discrambling?
		*/
		if (1 == stream->detail.ts_stream.param.NeedDiscramble[i])
		{
        	dmx_ts_flt_unregister(ts_flt_id2see[i]);
			
		    Sed_DmxSee2mainPidDel(pid_list[i]);
		}		
    }

    dmx_data_buf_flush_all_pkt(&stream->data_buf);

    return(0);
}


__s32 dmx_ts_stream_cfg
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32  ret;
    __s32  i;
    __s32  j;
    __s32 *ts_flt_id;
    __s32 *ts_flt_id2see;
    __u32  ts_src_hw_interface_id;
	__u32 *pid_list;
	struct Ali_DmxDrvTsStrmStatInfo *stat_info; 

	stat_info = &stream->detail.ts_stream.stat_info;

	memset(stat_info, 0, sizeof(struct Ali_DmxDrvTsStrmStatInfo));
	
	DMX_API_DBG("%s,%d,src_hw_interface_id:%d\n", __FUNCTION__, __LINE__, dev->src_hw_interface_id);

    if ((DMX_STREAM_STATE_CFG  != stream->state) &&
        (DMX_STREAM_STATE_STOP != stream->state))
    {
		stat_info->StatErrCnt++;

        return(-EPERM);
    }

    if (DMX_STREAM_STATE_STOP == stream->state)
    {
        if (stream->type != DMX_STREAM_TYPE_TS)
        {
    		stat_info->StrTypeErrCnt++;
    
            return(-EPERM);
        } 
		
        ret = dmx_ts_stream_close(dev, stream);

        if (ret < 0)
        {
            DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
            return(ret);
        }
    }

    ret = copy_from_user(&(stream->detail.ts_stream.param), (void __user *)arg,
                         _IOC_SIZE(cmd));

    if (0 != ret)
    {
		stat_info->CopyErrCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    if (0 == stream->detail.ts_stream.param.PidCnt)
    {
		stat_info->NoPidCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    DMX_API_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    dmx_data_buf_list_init(&(stream->data_buf));

	ts_flt_id = stream->detail.ts_stream.ts_flt_id;

	ts_flt_id2see = stream->detail.ts_stream.ts_flt_id2see;

	pid_list = stream->detail.ts_stream.param.PidList;

    for (i = 0; i < stream->detail.ts_stream.param.PidCnt; i++)
    {
		if (1 != stream->detail.ts_stream.param.NeedDiscramble[i])
		{
			ts_src_hw_interface_id = dev->src_hw_interface_id;
		}
		else
		{
		    Sed_DmxSee2mainPidAdd(pid_list[i]);

			ts_flt_id2see[i] = dmx_ts_flt_register(dev->src_hw_interface_id, pid_list[i], 
									               dmx_see_buf_wr_ts,
									               dev->src_hw_interface_id);
			
			if (ts_flt_id2see[i] < 0)
			{
				DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

				stat_info->CfgFailCnt++;				

				goto err_exit;
			}
		
			ts_src_hw_interface_id = 6;
		}

    	ts_flt_id[i] = dmx_ts_flt_register(ts_src_hw_interface_id, pid_list[i],
    		                               dmx_ts_stream_write, (__u32)stream);

		stream->detail.ts_stream.param.TsFltId[i] = ts_flt_id[i];
    	
    	if (stream->detail.ts_stream.ts_flt_id[i] < 0)
    	{
    		DMX_API_DBG("%s,%d\n", __FUNCTION__, __LINE__);

			stat_info->CfgFailCnt++;

			goto err_exit;
    	}
    }

    stream->type = DMX_STREAM_TYPE_TS;

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);	

err_exit:

    /* Clean up if error happens.
	*/
	for (j = 0; j < i; j++)
	{
		if (1 == stream->detail.ts_stream.param.NeedDiscramble[j])
		{
        	dmx_ts_flt_unregister(ts_flt_id2see[j]);
			
		    Sed_DmxSee2mainPidDel(pid_list[j]);
		}
		
    	dmx_ts_flt_unregister(ts_flt_id[j]);
	}
	
	return(-EMFILE);
}




__s32 dmx_ts_stream_start
(
    struct dmx_output_device  *dev,
    struct dmx_stream         *stream
)
{
    __s32  i;
    __s32  ret;
    __s32 *ts_flt_id2see;
    __s32 *ts_flt_id;
	struct Ali_DmxDrvTsStrmStatInfo *stat_info; 

	stat_info = &stream->detail.ts_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_TS)
    {
		stat_info->StrTypeErrCnt++;
    
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_STOP)
    {
		stat_info->StatErrCnt++;

        return(-EPERM);
    }

	ts_flt_id = stream->detail.ts_stream.ts_flt_id;

	ts_flt_id2see = stream->detail.ts_stream.ts_flt_id2see;

    for (i = 0; i < stream->detail.ts_stream.param.PidCnt; i++)
    {
        ret = dmx_ts_flt_start(ts_flt_id[i]);

        if (ret < 0)
        {
			stat_info->StartFailCnt++;

            DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
            return(ret);
        }

		if (1 == stream->detail.ts_stream.param.NeedDiscramble[i])
		{
        	dmx_ts_flt_start(ts_flt_id2see[i]);
		}			
    }

    stream->state = DMX_STREAM_STATE_RUN;

    return(0);
}





__s32 dmx_ts_stream_stop
(
    struct dmx_output_device  *dev,
    struct dmx_stream         *stream
)
{
    __s32  i;
    __s32  ret;
    __s32 *ts_flt_id2see;
    __s32 *ts_flt_id;
	struct Ali_DmxDrvTsStrmStatInfo *stat_info; 

	stat_info = &stream->detail.ts_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_TS)
    {
		stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
		stat_info->StatErrCnt++;
    
        return(-EPERM);
    }

	ts_flt_id = stream->detail.ts_stream.ts_flt_id;

	ts_flt_id2see = stream->detail.ts_stream.ts_flt_id2see;

    for (i = 0; i < stream->detail.ts_stream.param.PidCnt; i++) 
    {
        ret = dmx_ts_flt_stop(stream->detail.ts_stream.ts_flt_id[i]);
        
        if (ret < 0)
        {
			stat_info->StopFailCnt++;

            DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
            return(ret);
        }

		if (1 == stream->detail.ts_stream.param.NeedDiscramble[i])
		{
        	dmx_ts_flt_stop(ts_flt_id2see[i]);
		}		
    }

    dmx_data_buf_flush_all_pkt(&stream->data_buf);

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}



__s32 dmx_ts_stream_poll
(
    struct file              *filp,
    struct poll_table_struct *wait,
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __u32 pkt_len;
    __s32 mask;
    
    poll_wait(filp, &(stream->data_buf.rd_wq), wait);
      
    mask = 0;

    pkt_len = dmx_data_buf_total_len(&stream->data_buf);

    /* Return readable if there is data could be read out.
    */
    if (pkt_len > 0)
    {    
        mask |= (POLLIN | POLLRDNORM); 
    }

    DMX_API_DBG("%s,%d,mask:0x%x\n", __FUNCTION__, __LINE__, mask);
    
    return(mask);
}


__s32 dmx_ts_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                           ret;
	__u32                           ts_flt_idx;
	__u32                           pid_flt_idx;
	struct Ali_DmxDrvTsStrmStatInfo *p_StatInfo;
	struct Ali_DmxDrvTsStrmStatInfo *p_UsrInfo;

	p_UsrInfo = (struct Ali_DmxDrvTsStrmStatInfo *)arg;

	if (NULL == p_UsrInfo || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_TS)
    {
        return(-EPERM);
    }

    ts_flt_idx = p_UsrInfo->TsFltIdx;

	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);
	
	p_StatInfo = &stream->detail.ts_stream.stat_info;

	p_StatInfo->TsInCnt = ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info.TsInCnt;

    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}


__s32 dmx_ts_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                          ret;
	__u32                          ts_flt_idx;
	__u32                          pid_flt_idx;
	struct Ali_DmxDrvTsFltStatInfo *p_StatInfo;
	struct Ali_DmxDrvTsFltStatInfo *p_UsrInfo;

	p_UsrInfo = (struct Ali_DmxDrvTsFltStatInfo *)arg;

	if (NULL == p_UsrInfo || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

	switch (stream->type)
	{
		case DMX_STREAM_TYPE_TS:
		{
			ts_flt_idx = p_UsrInfo->TsFltIdx;
		
			p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;
		}
		break;
		
		case DMX_STREAM_TYPE_VIDEO:
		{
			ts_flt_idx = stream->detail.video_stream.ts_flt_id;

			p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;
		}
		break;
		
		case DMX_STREAM_TYPE_AUDIO:
		{
			ts_flt_idx = stream->detail.audio_stream.ts_flt_id;

			p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;
		}
		break;

		case DMX_STREAM_TYPE_PCR:
		{
			ts_flt_idx = dmx_pcr_flt_link_ts_flt_idx(stream->detail.pcr_stream.pcr_flt_id);

			p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;
		}
		break;

		case DMX_STREAM_TYPE_SEC:
		{
			ts_flt_idx = dmx_sec_flt_link_ts_flt_idx(stream->detail.sec_stream.sec_flt_id);

			p_StatInfo = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info;
		}
		break;

		default:
		{
			ts_flt_idx = -1;

			p_StatInfo = NULL;
		}
		break;
	}

	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}





__s32 dmx_tp_stream_write
(
    struct dmx_ts_pkt_inf *pkt_inf,
    __u32                  param
)
{
    __u32                            data_len;
    __s32                            byte_cnt;
    struct ali_dmx_data_buf         *dest_buf;
    struct dmx_stream               *stream;	
	struct Ali_DmxDrvTpStrmStatInfo *stat_info; 
	
	stream = (struct dmx_stream *)param;

    dest_buf = &(stream->data_buf);

    data_len = dmx_data_buf_total_len(dest_buf);

    byte_cnt = dmx_data_buf_wr_data(dest_buf, pkt_inf->pkt_addr, 188, 
                                    DMX_DATA_SRC_TYPE_KERN);

	stat_info = &stream->detail.tp_stream.stat_info;

	if (byte_cnt < 0)
	{
		stat_info->WrByteErrCnt++;
		
        printk("%s,%d,streamid:%d,wr len:%d,buf len:%d,ret:%d,pkt_inf:%x\n",
               __FUNCTION__, __LINE__, stream->stream_id, 188, 
               dest_buf->cur_len, byte_cnt, (__u32)pkt_inf);

        /* Must be buffer overflow, flush all section data of this stream 
         * to free memory.
		 */
		dmx_data_buf_flush_all_pkt(dest_buf);		
	}

    /* Wakeup waiting poll only if we are storing into an empty buffer,
     * to offload CPU loading.
    */
    if (data_len <= 0)
    {
        //printk("%s,%d,data_len:%d\n", __FUNCTION__, __LINE__, data_len);
    	
        wake_up_interruptible(&(dest_buf->rd_wq));
    }

    return(byte_cnt);
}




__s32 dmx_tp_stream_read
(
    struct dmx_output_device  *dev,
    struct dmx_stream         *stream,
    __s32                      flags,
    char __user               *usr_buf,
    size_t                     usr_rd_len
)
{
    __s32                            byte_cnt;
    __u32                            ready_len;
	struct Ali_DmxDrvTpStrmStatInfo *stat_info; 

	//printk("%s, line:%d\n", __FUNCTION__, __LINE__);

	stat_info = &stream->detail.tp_stream.stat_info;

    if (stream->type != DMX_STREAM_TYPE_TP)
    {
		stat_info->StrTypeErrCnt++;
		
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
		stat_info->StatErrCnt++;
		
        return(-EPERM);
    }

    if (0 == usr_rd_len)
    {
        return(0);
    } 

    for (;;)
    {
    	ready_len = dmx_data_buf_total_len(&(stream->data_buf));
    
    	if (ready_len > 0)
    	{
    		/* Data could be read out. 
    		*/
    		break;
    	}  
    
    	/* pkt_len <= 0, then nothing could be read out.
    	*/
    	if (flags & O_NONBLOCK)
    	{		   
    		return(-EAGAIN);
    	}	 
    
    	dmx_mutex_output_unlock(dev->src_hw_interface_id);
		
    	if (wait_event_interruptible(stream->data_buf.rd_wq, 
    		dmx_data_buf_total_len(&(stream->data_buf)) > 0))
    	{
    		return(-ERESTARTSYS);
    	}
			
    	if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    	{	 
    		return(-ERESTARTSYS);
    	}	
    }
    
    /* byte_cnt may be less than need_cpy_len if data_buf contains less
     * data than need_cpy_len required.
     */
    byte_cnt = dmx_data_buf_rd(usr_buf, &(stream->data_buf), usr_rd_len, 
    						   DMX_DATA_CPY_DEST_USER); 
    
    if (byte_cnt < 0)
    {
		stat_info->RdByteErrCnt++;

    	printk("%s,%d,byte_cnt:%d\n", __FUNCTION__, __LINE__, byte_cnt);
    }

	//printk("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(byte_cnt);
}






__s32 dmx_tp_stream_close
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32                            ret;
	struct Ali_DmxDrvTpStrmStatInfo *stat_info; 

	stat_info = &stream->detail.tp_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_TP)
    {
		stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (DMX_STREAM_STATE_CFG == stream->state)
    {
        return(0);
    }

	/* Disable bypasse all control in HW.
	*/

    ret = dmx_pid_flt_unregister(stream->detail.tp_stream.pid_flt_id);

    if (ret < 0)
    {
		stat_info->CloseFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    dmx_data_buf_flush_all_pkt(&stream->data_buf);

    return(0);
}



__s32 dmx_tp_stream_cfg
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                            ret;
    __s32                            flt_id;
	struct Ali_DmxDrvTpStrmStatInfo *stat_info; 

	stat_info = &stream->detail.tp_stream.stat_info;

	memset(stat_info, 0, sizeof(struct Ali_DmxDrvTpStrmStatInfo));
	
	DMX_API_DBG("%s,%d,src_hw_interface_id:%d\n", __FUNCTION__, __LINE__, dev->src_hw_interface_id);

    if ((DMX_STREAM_STATE_CFG  != stream->state) &&
        (DMX_STREAM_STATE_STOP != stream->state))
    {
		stat_info->StatErrCnt++;

        return(-EPERM);
    }

    if (DMX_STREAM_STATE_STOP == stream->state)
    {
        if (stream->type != DMX_STREAM_TYPE_TP)
        {
    		stat_info->StrTypeErrCnt++;
    
            return(-EPERM);
        }    
		
        ret = dmx_tp_stream_close(dev, stream);

        if (ret < 0)
        {
            DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
            return(ret);
        }
    }

    ret = copy_from_user(&(stream->detail.tp_stream.param), (void __user *)arg,
                         _IOC_SIZE(cmd));

    if (0 != ret)
    {
		stat_info->CopyErrCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    dmx_data_buf_list_init(&(stream->data_buf));

	flt_id = dmx_pid_flt_register(dev->src_hw_interface_id, DMX_WILDCARD_PID, 
								  dmx_tp_stream_write, (__u32)(stream));
	
	if (flt_id < 0)
	{
    	stat_info->CfgFailCnt++;

		DMX_API_DBG("%s,%d,flt_id:%d\n", __FUNCTION__, __LINE__, flt_id);
	
		return(ret);
	}	

	stream->detail.tp_stream.pid_flt_id = flt_id;

    stream->type = DMX_STREAM_TYPE_TP;

    stream->state = DMX_STREAM_STATE_STOP;

    DMX_API_DBG("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);	
}




__s32 dmx_tp_stream_start
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32                            ret;
	struct Ali_DmxDrvTpStrmStatInfo *stat_info; 

	DMX_API_DBG("%s,%d,stream:%d\n", __FUNCTION__, __LINE__, stream->stream_id);	

	stat_info = &stream->detail.tp_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_TP)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_STOP)
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }

    ret = dmx_pid_flt_start(stream->detail.tp_stream.pid_flt_id);

    if (ret < 0)
    {
	   	stat_info->StartFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

        return(ret);
    }

    stream->state = DMX_STREAM_STATE_RUN;

	DMX_API_DBG("%s,%d,stream:%d\n", __FUNCTION__, __LINE__,stream->stream_id);	

    return(0);
}




__s32 dmx_tp_stream_stop
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32                            ret;
	struct Ali_DmxDrvTpStrmStatInfo *stat_info; 
	
	DMX_API_DBG("%s,%d\n", __FUNCTION__, __LINE__);

	stat_info = &stream->detail.tp_stream.stat_info;

    if (stream->type != DMX_STREAM_TYPE_TP)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }

    ret = dmx_pid_flt_stop(stream->detail.tp_stream.pid_flt_id);

    if (ret < 0)
    {
	   	stat_info->StopFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

        return(ret);
    }

    dmx_data_buf_flush_all_pkt(&(stream->data_buf));

    stream->state = DMX_STREAM_STATE_STOP;

	DMX_API_DBG("%s,%d,stream:%d\n", __FUNCTION__, __LINE__, stream->stream_id);	

    return(0);
}



__s32 dmx_tp_stream_poll
(
    struct file              *filp,
    struct poll_table_struct *wait,
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __u32 pkt_len;
    __s32 mask;
    
    poll_wait(filp, &(stream->data_buf.rd_wq), wait);
      
    mask = 0;

    pkt_len = dmx_data_buf_total_len(&stream->data_buf);

    /* Return readable if there is data could be read out, otherwise sleep on
     * data_buf.rd_wq waiting for awakened by dmx_tp_stream_write.
    */
    if (pkt_len > 0)
    {    
        mask |= (POLLIN | POLLRDNORM); 
    }

    DMX_API_DBG("%s,%d,mask:0x%x\n", __FUNCTION__, __LINE__, mask);
    
    return(mask);
}




__s32 dmx_video_stream_close
(
    struct dmx_output_device  *dev,
    struct dmx_stream *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvVideoStrmStatInfo *stat_info; 

	stat_info = &stream->detail.video_stream.stat_info;
	
    /* Stream state validation. 
    */
    if (stream->type != DMX_STREAM_TYPE_VIDEO)
    {
    	stat_info->StrTypeErrCnt++;
		
        return(-EPERM);
    }

    DMX_API_DBG("%s, line:%d,state:%d,ts_flt_id:%d\n", __FUNCTION__, __LINE__,
                  stream->state, stream->detail.video_stream.ts_flt_id);

    if (DMX_STREAM_STATE_CFG == stream->state)
    {
        return(0);
    }

    ret = dmx_ts_flt_unregister(stream->detail.video_stream.ts_flt_id);

    if (ret < 0)
    {
    	stat_info->CloseFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    dmx_see_video_stream_stop(0);

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}



__s32 dmx_video_stream_cfg
(
    struct dmx_output_device  *dev,
    struct dmx_stream         *stream,
    unsigned int               cmd,
    unsigned long              arg
)
{
    __s32 ret;
	struct Ali_DmxDrvVideoStrmStatInfo *stat_info; 

	stat_info = &stream->detail.video_stream.stat_info;

	memset(stat_info, 0, sizeof(struct Ali_DmxDrvVideoStrmStatInfo));

	memset(&g_stat_info, 0, sizeof(g_stat_info));
	
    /* Stream state validation. 
    */
    if ((DMX_STREAM_STATE_CFG  != stream->state) &&
        (DMX_STREAM_STATE_STOP != stream->state))
    {
    	stat_info->StatErrCnt++;
		
        return(-EPERM);
    }

    if (DMX_STREAM_STATE_STOP == stream->state)
    {
        /* Stream state validation. 
        */
        if (stream->type != DMX_STREAM_TYPE_VIDEO)
        {
        	stat_info->StrTypeErrCnt++;
    		
            return(-EPERM);
        }  
		
        ret = dmx_video_stream_close(dev, stream);

        if (ret < 0)
        {
            DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
            return(ret);
        }
    }

    ret = copy_from_user(&(stream->detail.video_stream.param), 
                         (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
    	stat_info->CopyErrCnt++;
		    
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    ret = dmx_ts_flt_register(dev->src_hw_interface_id,
                              stream->detail.video_stream.param.Pid & 0x1FFF, 
                              dmx_see_buf_wr_ts, dev->src_hw_interface_id);

    if (ret < 0)
    {
    	stat_info->CfgFailCnt++;
		
        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    stream->detail.video_stream.ts_flt_id = ret;

    stream->type = DMX_STREAM_TYPE_VIDEO;

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}



__s32 dmx_video_stream_start
(
    struct dmx_output_device  *dev,
    struct dmx_stream *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvVideoStrmStatInfo *stat_info; 

	stat_info = &stream->detail.video_stream.stat_info;
	
    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    if (stream->type != DMX_STREAM_TYPE_VIDEO)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    /* Stream state validation. 
    */
    if (stream->state != DMX_STREAM_STATE_STOP)
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }

    dmx_see_video_stream_start(stream->detail.video_stream.param.Pid);

    ret = dmx_ts_flt_start(stream->detail.video_stream.ts_flt_id);

    if (ret < 0)
    {
    	stat_info->StartFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    stream->state = DMX_STREAM_STATE_RUN;

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}




__s32 dmx_video_stream_stop
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvVideoStrmStatInfo *stat_info; 

	stat_info = &stream->detail.video_stream.stat_info;
	
    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    if (stream->type != DMX_STREAM_TYPE_VIDEO)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    /* Stream state validation. 
    */
    if (stream->state != DMX_STREAM_STATE_RUN)
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }

    ret = dmx_ts_flt_stop(stream->detail.video_stream.ts_flt_id);

    if (ret < 0)
    {
    	stat_info->StopFailCnt++;
    
        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    dmx_see_video_stream_stop(0);

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}


__s32 dmx_video_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                              ret;
	__u32                              ts_flt_idx;
	__u32                              pid_flt_idx;
	struct Ali_DmxDrvVideoStrmStatInfo *p_StatInfo;

	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_VIDEO)
    {
        return(-EPERM);
    }	

    ts_flt_idx = stream->detail.video_stream.ts_flt_id;

	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	p_StatInfo = &stream->detail.video_stream.stat_info;

	p_StatInfo->TsInCnt = ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info.TsInCnt;

    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    return(0);
}

__s32 dmx_video_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_VIDEO)
    {
        return(-EPERM);
    }

    return dmx_ts_filter_info_get(dev, stream, cmd, arg);
}


__s32 dmx_video_see_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                                   ret;
	volatile struct Ali_DmxSeePlyChStatInfo *p_StatInfo;

	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_VIDEO)
    {
        return(-EPERM);
    }
	
	p_StatInfo = &ali_dmx_see_dev[0].see_buf_init.PlyChStatInfo[0];

    ret = copy_to_user((void __user *)arg, (const void *)p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

	return(0);
}


__s32 dmx_audio_stream_close
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvAudioStrmStatInfo *stat_info; 

	stat_info = &stream->detail.audio_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_AUDIO)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (DMX_STREAM_STATE_CFG == stream->state)
    {
        return(0);
    }

    ret = dmx_ts_flt_unregister(stream->detail.audio_stream.ts_flt_id);

    if (ret < 0)
    {
    	stat_info->CloseFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    dmx_see_audio_stream_stop(0);

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}



__s32 dmx_audio_stream_cfg
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32 ret;
	struct Ali_DmxDrvAudioStrmStatInfo *stat_info; 

	stat_info = &stream->detail.audio_stream.stat_info;

	memset(stat_info, 0, sizeof(struct Ali_DmxDrvAudioStrmStatInfo));
	
	memset(&g_stat_info, 0, sizeof(g_stat_info));

    /* Stream state validation. 
    */
    if ((DMX_STREAM_STATE_CFG != stream->state) &&
        (DMX_STREAM_STATE_STOP != stream->state))
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    } 

    if (DMX_STREAM_STATE_STOP == stream->state)
    {
        if (stream->type != DMX_STREAM_TYPE_AUDIO)
        {
        	stat_info->StrTypeErrCnt++;
    
            return(-EPERM);
        }
		
        ret = dmx_audio_stream_close(dev, stream);

        if (ret < 0)
        {
            DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
            return(ret);
        }
    }

    ret = copy_from_user(&(stream->detail.audio_stream.param), 
                         (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
    	stat_info->CopyErrCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    ret = dmx_ts_flt_register(dev->src_hw_interface_id,
                              stream->detail.audio_stream.param.Pid & 0x1FFF, 
                              dmx_see_buf_wr_ts, dev->src_hw_interface_id);

    if (ret < 0)
    {
    	stat_info->CfgFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    stream->detail.audio_stream.ts_flt_id = ret;

    stream->type = DMX_STREAM_TYPE_AUDIO;

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}




__s32 dmx_audio_stream_start
(
    struct dmx_output_device  *dev,
    struct dmx_stream *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvAudioStrmStatInfo *stat_info; 

	stat_info = &stream->detail.audio_stream.stat_info;
	
    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    /* Stream state validation. 
    */
    if (stream->type != DMX_STREAM_TYPE_AUDIO)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_STOP)
    {
    	stat_info->StatErrCnt++;
    
        return(-EPERM);
    }

    dmx_see_audio_stream_start(stream->detail.audio_stream.param.Pid);

    ret = dmx_ts_flt_start(stream->detail.audio_stream.ts_flt_id);

    if (ret < 0)
    {
    	stat_info->StartFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    stream->state = DMX_STREAM_STATE_RUN;

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}


__s32 dmx_audio_stream_stop
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvAudioStrmStatInfo *stat_info; 

	stat_info = &stream->detail.audio_stream.stat_info;
	
    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    if (stream->type != DMX_STREAM_TYPE_AUDIO)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
    	stat_info->StatErrCnt++;
    
        return(-EPERM);
    }

    ret = dmx_ts_flt_stop(stream->detail.audio_stream.ts_flt_id);

    if (ret < 0)
    {
    	stat_info->StopFailCnt++;

        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    dmx_see_audio_stream_stop(0);

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}



__s32 dmx_audio_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                                ret;
	__u32                                ts_flt_idx;
	__u32                                pid_flt_idx;
	struct Ali_DmxDrvAudioStrmStatInfo *p_StatInfo;

	DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);
	
	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_AUDIO)
    {
        DMX_API_DBG("%s,%d,stream->type:%d\n", __FUNCTION__, __LINE__, stream->type);
    
        return(-EPERM);
    }

	ts_flt_idx = stream->detail.audio_stream.ts_flt_id;

	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);

	p_StatInfo = &stream->detail.audio_stream.stat_info;

	p_StatInfo->TsInCnt = ali_dmx_ts_flt_module.ts_flt[ts_flt_idx].stat_info.TsInCnt;

    ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

	DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}



__s32 dmx_audio_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_AUDIO)
    {
        return(-EPERM);
    }

    return dmx_ts_filter_info_get(dev, stream, cmd, arg);
}


__s32 dmx_audio_see_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                                   ret;
	volatile struct Ali_DmxSeePlyChStatInfo *p_StatInfo;

	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_AUDIO)
    {
        return(-EPERM);
    }

	p_StatInfo = &ali_dmx_see_dev[0].see_buf_init.PlyChStatInfo[1];

    ret = copy_to_user((void __user *)arg, (const void *)p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

	return(0);
}

/* Note: Called in inerrupt context, so this function may not sleep.
*/
__s32 dmx_pcr_stream_write
(
    __u32 pcr,
    __u32 param
)
{
    __s32                    ret;
    struct ali_dmx_data_buf *dest_buf;
    struct dmx_pcr_stream   *pcr_stream;

    ret = 0;

    dmx_see_set_pcr(pcr);

    dest_buf = &(((struct dmx_stream *)param)->data_buf);
    
    pcr_stream = &(((struct dmx_stream *)param)->detail.pcr_stream);

    pcr_stream->latest_pcr = pcr;
    
    wake_up_interruptible(&(dest_buf->rd_wq));

    return(ret);
}




__s32 dmx_pcr_stream_read
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    __s32                     flags,
    char __user              *usr_buf,
    size_t                    usr_rd_len
)
{
    struct dmx_pcr_stream *pcr_stream;
	struct Ali_DmxDrvPcrStrmStatInfo *stat_info; 
    __s32                  copy_len;

	stat_info = &stream->detail.pcr_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_PCR)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }

    if (usr_rd_len < 4)
    {
    	stat_info->RdBufShortCnt++;

        return(-EFBIG);
    }    

    pcr_stream = &(stream->detail.pcr_stream);

    copy_len = sizeof(pcr_stream->latest_pcr);

    for (;;)
    {
        /* 0 == pcr_stream->latest_pcr means there is nothing to be read.
        */
        if (0 != pcr_stream->latest_pcr)
        {
            if (copy_to_user(usr_buf, &pcr_stream->latest_pcr, copy_len))
            {
		    	stat_info->RdByteErrCnt++;
            
                return(-EFAULT);     
            }
        
            /* Clear pcr for next read.
            */
            pcr_stream->latest_pcr = 0;
            
            return(copy_len);
        }
        
        /* Nothing could be read out.
        */
        if (flags & O_NONBLOCK)
        {
            return(-EAGAIN);
        }   
        
        dmx_mutex_output_unlock(6);
		
        dmx_mutex_output_unlock(dev->src_hw_interface_id);

        if (wait_event_interruptible(stream->data_buf.rd_wq, 
            (0 != pcr_stream->latest_pcr)))
        {
            return(-ERESTARTSYS);
        }
        
        if (dmx_mutex_output_lock(dev->src_hw_interface_id))
        {    
            return(-ERESTARTSYS);
        }    

        #if 0
        if (dmx_mutex_output_lock(6))
        {    
            return(-ERESTARTSYS);
        }    
		#endif
    }

    return(copy_len);
}





__s32 dmx_pcr_stream_close
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvPcrStrmStatInfo *stat_info; 

	stat_info = &stream->detail.pcr_stream.stat_info;
	
    if (stream->type != DMX_STREAM_TYPE_PCR)
    {
    	stat_info->StrTypeErrCnt++;
    
        return(-EPERM);
    }

    if (DMX_STREAM_STATE_CFG == stream->state)
    {
        return(0);
    }
    
    ret = dmx_pcr_flt_unregister(stream->detail.pcr_stream.pcr_flt_id);

    if (ret < 0)
    {
    	stat_info->CloseFailCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(ret);
    }

    stream->detail.pcr_stream.latest_pcr = 0;

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}



__s32 dmx_pcr_stream_cfg
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                         ret;
    struct Ali_DmxPcrStreamParam *param;
	struct Ali_DmxDrvPcrStrmStatInfo *stat_info; 

	stat_info = &stream->detail.pcr_stream.stat_info;	

	memset(stat_info, 0, sizeof(struct Ali_DmxDrvVideoStrmStatInfo));

    /* Channel startus validation.
    */
    if ((DMX_STREAM_STATE_CFG  != stream->state) &&
        (DMX_STREAM_STATE_STOP != stream->state))
    {
    	stat_info->StatErrCnt++;

        return(-EPERM);
    }

    if (DMX_STREAM_STATE_STOP == stream->state)
    {
        if (stream->type != DMX_STREAM_TYPE_PCR)
        {
        	stat_info->StrTypeErrCnt++;
        
            return(-EPERM);
        }    
		
        ret = dmx_pcr_stream_close(dev, stream);

        if (ret < 0)
        {
            DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
            
            return(ret);
        }
    }

    param = &(stream->detail.pcr_stream.param);

    ret = copy_from_user(param, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
    	stat_info->CopyErrCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    dmx_data_buf_list_init(&(stream->data_buf));

#if 0
    /* Temparay path for AV sync.
    */

    sed_set_pcr_pid(stream->detail.pcr_stream.param.Pid);

    ret = dmx_pcr_flt_register(dev->src_hw_interface_id, param->Pid, dmx_pcr_stream_write,
                               (__u32)stream, dmx_see_buf_wr_ts, dev->src_hw_interface_id);

#else
    ret = dmx_pcr_flt_register(dev->src_hw_interface_id, param->Pid, dmx_pcr_stream_write,
                               (__u32)stream, NULL, 0);
#endif  

    if (0 != ret)
    {
    	stat_info->CfgFailCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(ret);
    }
    
    stream->detail.pcr_stream.pcr_flt_id = ret;

    stream->type = DMX_STREAM_TYPE_PCR;

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}





__s32 dmx_pcr_stream_start
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvPcrStrmStatInfo *stat_info; 

	stat_info = &stream->detail.pcr_stream.stat_info;	
	
    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    if (stream->type != DMX_STREAM_TYPE_PCR)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    /* stream state validation. 
     */
    if (stream->state != DMX_STREAM_STATE_STOP)
    {
    	stat_info->StatErrCnt++;
    
        return(-EPERM);
    }
    
    ret = dmx_pcr_flt_start(stream->detail.pcr_stream.pcr_flt_id);

    if (ret < 0)
    {
    	stat_info->StartFailCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(ret);
    }

    stream->state = DMX_STREAM_STATE_RUN;

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}




__s32 dmx_pcr_stream_stop
(
    struct dmx_output_device  *dev,
    struct dmx_stream *stream
)
{
    __s32 ret;
	struct Ali_DmxDrvPcrStrmStatInfo *stat_info; 

	stat_info = &stream->detail.pcr_stream.stat_info;	
	
    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    if (stream->type != DMX_STREAM_TYPE_PCR)
    {
    	stat_info->StrTypeErrCnt++;

        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
    	stat_info->StatErrCnt++;
    
        return(-EPERM);
    }

    ret = dmx_pcr_flt_stop(stream->detail.pcr_stream.pcr_flt_id);

    if (ret < 0)
    {
    	stat_info->StopFailCnt++;

        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(ret);
    }

#if 0
    /* Temparay path for AV sync.
    */
    sed_set_pcr_pid(0x1FFF);
#endif

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);
    
    stream->detail.pcr_stream.latest_pcr = 0;

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}


__s32 dmx_pcr_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                            ret;
	__u32                            pcr_flt_idx;
	__u32                            ts_flt_idx;
	__u32                            pid_flt_idx;
	struct dmx_pcr_flt               *pcr_flt;
	struct dmx_ts_flt                *ts_flt;
	struct Ali_DmxDrvPcrStrmStatInfo *p_StatInfo;

	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_PCR)
    {
        return(-EPERM);
    }

	pcr_flt_idx = stream->detail.pcr_stream.pcr_flt_id;

    ts_flt_idx = dmx_pcr_flt_link_ts_flt_idx(pcr_flt_idx);

	pid_flt_idx = dmx_ts_flt_link_pid_flt_idx(ts_flt_idx);
	
	pcr_flt = &ali_dmx_pcr_flt_module.pcr_flt[pcr_flt_idx];

	ts_flt = &ali_dmx_ts_flt_module.ts_flt[ts_flt_idx];

	p_StatInfo = &stream->detail.pcr_stream.stat_info;

	p_StatInfo->TsInCnt = ts_flt->stat_info.TsInCnt;

	p_StatInfo->LastPcrVal = stream->detail.pcr_stream.latest_pcr;
	
	ret = copy_to_user((void __user *)arg, p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }
    
    return(0);
}

__s32 dmx_pcr_filter_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_PCR)
    {
        return(-EPERM);
    }

    return dmx_ts_filter_info_get(dev, stream, cmd, arg);
}



__s32 dmx_hw_reg_stream_close
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    if (stream->type != DMX_STREAM_TYPE_HW_REG)
    {
        return(-EPERM);
    }

    return(0);
}


__s32 dmx_hw_reg_stream_cfg
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    /* stream state validation. 
     */
    if (stream->state != DMX_STREAM_STATE_CFG &&
		stream->state != DMX_STREAM_STATE_STOP)
    {
        return(-EPERM);
    }

    stream->type = DMX_STREAM_TYPE_HW_REG;

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}




__s32 dmx_hw_reg_stream_start
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{    
	
    if (stream->type != DMX_STREAM_TYPE_HW_REG)
    {
        return(-EPERM);
    }

    /* stream state validation. 
     */
    if (stream->state != DMX_STREAM_STATE_STOP)
    {
        return(-EPERM);
    }

    stream->state = DMX_STREAM_STATE_RUN;

    return(0);
}



__s32 dmx_hw_reg_stream_stop
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

    if (stream->type != DMX_STREAM_TYPE_HW_REG)
    {
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
        return(-EPERM);
    }

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}



__s32 dmx_hw_reg_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32 ret;
	__u32 HwRegTable[18][5];
	__u32 i, j, k, DmxBaseAddr;
	extern __u32 dmx_hw_id2base_m37(__u32);
	extern __u32 AliRegGet32(__u32);

    if (stream->type != DMX_STREAM_TYPE_HW_REG)
    {
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
        return(-EPERM);
    }

	DmxBaseAddr = dmx_hw_id2base_m37(0);
	
	memset(HwRegTable, 0, sizeof(HwRegTable));

	for (i = 0, k = 0; i < 0x370; i += 16)
	{
		if (i > 0x64 && i < 0x300)
		{
			if (i != 0xB0 && i != 0xC0 && i != 0x140 && i != 0x1C0)
			{
				continue;
			}
		}

		if (!(i & 0xf))
		{
			HwRegTable[k][0] = DmxBaseAddr + i;
		}

		for (j = 0; j < 4; j++)
		{
			HwRegTable[k][j + 1] = AliRegGet32(HwRegTable[k][0] + j * 4);
		}

		k++;
	}

    ret = copy_to_user((void __user *)arg, &HwRegTable, sizeof(HwRegTable));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);
    
    return(0);
}



__s32 dmx_kern_glb_stream_close
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_KERN_GLB)
    {
        return(-EPERM);
    }

    return(0);
}


__s32 dmx_kern_glb_stream_cfg
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* stream state validation. 
     */
    if ((DMX_STREAM_STATE_CFG != stream->state) &&
        (DMX_STREAM_STATE_STOP != stream->state))
    {
        return(-EPERM);
    }

    stream->type = DMX_STREAM_TYPE_KERN_GLB;

    stream->state = DMX_STREAM_STATE_STOP;

	memset(&g_stat_info, 0, sizeof(struct Ali_DmxKernGlobalStatInfo));

    return(0);
}



__s32 dmx_kern_glb_stream_start
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{    
	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_KERN_GLB)
    {
        return(-EPERM);
    }

    /* stream state validation. 
     */
    if (stream->state != DMX_STREAM_STATE_STOP)
    {
        return(-EPERM);
    }

    stream->state = DMX_STREAM_STATE_RUN;

    return(0);
}



__s32 dmx_kern_glb_stream_stop
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_KERN_GLB)
    {
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
        return(-EPERM);
    }

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}



__s32 dmx_kern_glb_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32 ret;

	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }
	
    if (stream->type != DMX_STREAM_TYPE_KERN_GLB)
    {
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
        return(-EPERM);
    }

	ret = copy_to_user((void __user *)arg, &g_stat_info, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }
    
    return(0);
}


__s32 dmx_kern_glb_stream_realtime_set
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* stream state validation. 
     */
    if (DMX_STREAM_STATE_IDLE == stream->state)
    {
        return(-EPERM);
    }

	g_stat_info.RealTimePrintEn = arg;

    return(0);
}


__s32 dmx_see_glb_stream_close
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_SEE_GLB)
    {
        return(-EPERM);
    }

    return(0);
}


__s32 dmx_see_glb_stream_cfg
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
	volatile struct Ali_DmxSeeGlobalStatInfo *p_StatInfo;

	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* stream state validation. 
     */
    if ((DMX_STREAM_STATE_CFG != stream->state) &&
        (DMX_STREAM_STATE_STOP != stream->state))
    {
        return(-EPERM);
    }

    stream->type = DMX_STREAM_TYPE_SEE_GLB;

    stream->state = DMX_STREAM_STATE_STOP;

	p_StatInfo = ali_dmx_see_dev[0].see_buf_init.GlobalStatInfo;

	memset((void *)p_StatInfo, 0, sizeof(struct Ali_DmxSeeGlobalStatInfo));

    return(0);
}



__s32 dmx_see_glb_stream_start
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{    
	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_SEE_GLB)
    {
        return(-EPERM);
    }

    /* stream state validation. 
     */
    if (stream->state != DMX_STREAM_STATE_STOP)
    {
        return(-EPERM);
    }

    stream->state = DMX_STREAM_STATE_RUN;

    return(0);
}



__s32 dmx_see_glb_stream_stop
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream
)
{
	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    if (stream->type != DMX_STREAM_TYPE_SEE_GLB)
    {
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
        return(-EPERM);
    }

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}



__s32 dmx_see_glb_stream_info_get
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
    __s32                                     ret;
	volatile struct Ali_DmxSeeGlobalStatInfo *p_StatInfo;

	if (0 == arg || NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }
	
    if (stream->type != DMX_STREAM_TYPE_SEE_GLB)
    {
        return(-EPERM);
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
        return(-EPERM);
    }

	p_StatInfo = ali_dmx_see_dev[0].see_buf_init.GlobalStatInfo;
			
	ret = copy_to_user((void __user *)arg, (const void *)p_StatInfo, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }
    
    return(0);
}

__s32 dmx_see_glb_stream_realtime_set
(
    struct dmx_output_device *dev,
    struct dmx_stream        *stream,
    unsigned int              cmd,
    unsigned long             arg
)
{
	if (NULL == stream)
    {
        DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    /* stream state validation. 
     */
    if (DMX_STREAM_STATE_IDLE == stream->state)
    {
        return(-EPERM);
    }

	ali_dmx_see_dev[0].see_buf_init.statistics->RealTimePrintEn = arg;

    return(0);
}


__u32 dmx_stream_poll
(
    struct file              *filp,
    struct poll_table_struct *wait
)
{
    __s32                     mask;
    struct dmx_stream        *stream; 
    struct dmx_output_device *dev;

    stream = filp->private_data;

    if (DMX_STREAM_STATE_RUN != stream->state)
    {
        printk("%s,%d,state:%d\n", __FUNCTION__, __LINE__, stream->state);

        return(0);
    }

    dev = (struct dmx_output_device *)stream->dmx_output_device;

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }

    #if 0
    /* Also need to lock see to main buffer.
	*/
    if (dmx_mutex_output_lock(6))
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }	
	#endif

    mask = 0;

    switch(stream->type)
    {
        case DMX_STREAM_TYPE_SEC:
        {
            mask = dmx_sec_stream_poll(filp, wait, dev, stream);
        }
        break;

        case DMX_STREAM_TYPE_TS:
        {
            mask = dmx_ts_stream_poll(filp, wait, dev, stream);
        }
        break;

        case DMX_STREAM_TYPE_TP:
        {
            mask = dmx_tp_stream_poll(filp, wait, dev, stream);
        }
        break;		

        default:
        {
            //printk("Invalid output_format! return fail.\n");

            //return(-EINVAL);
        }
        break;
    }

    #if 0
    dmx_mutex_output_unlock(6);
    #endif
	
    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    return(mask);
}




__s32 dmx_stream_close
(
    struct inode *inode,
    struct file  *file
)
{
    __s32                       ret;
    struct dmx_output_device *dev;
    struct dmx_stream        *stream;

    stream = file->private_data;

    if (DMX_STREAM_STATE_IDLE == stream->state)
    {
        return(-EPERM);
    }	

    dev = (struct dmx_output_device *)stream->dmx_output_device;

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        return(-ERESTARTSYS);
    }

    #if 0
    /* Also need to lock see to main buffer.
	*/
    if (dmx_mutex_output_lock(6))
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }	
	#endif
	

    switch(stream->type)
    {
        case DMX_STREAM_TYPE_SEC:
        {
            ret = dmx_sec_stream_close(dev, stream);
        }
        break; 

        case DMX_STREAM_TYPE_VIDEO:
        {
            ret = dmx_video_stream_close(dev, stream);
        }
        break; 

        case DMX_STREAM_TYPE_AUDIO:
        {
            ret = dmx_audio_stream_close(dev, stream);
        }
        break; 

        case DMX_STREAM_TYPE_TS:
        {
            ret = dmx_ts_stream_close(dev, stream);
        }
        break; 

        case DMX_STREAM_TYPE_TP:
        {
            ret = dmx_tp_stream_close(dev, stream);
        }
        break; 		

        case DMX_STREAM_TYPE_PCR:
        {
            ret = dmx_pcr_stream_close(dev, stream);
        }
        break;

        case DMX_STREAM_TYPE_HW_REG:
        {
            ret = dmx_hw_reg_stream_close(dev, stream);
        }
        break;

        case DMX_STREAM_TYPE_KERN_GLB:
        {
            ret = dmx_kern_glb_stream_close(dev, stream);
        }
        break;

        case DMX_STREAM_TYPE_SEE_GLB:
        {
            ret = dmx_see_glb_stream_close(dev, stream);
        }
        break;

        default:
        {
            ret = -EPERM;
        }
        break;  
    }
    
    stream->dmx_output_device = NULL;

    file->private_data = NULL;

    stream->state = DMX_STREAM_STATE_IDLE;

    #if 0
    dmx_mutex_output_unlock(6);
    #endif
	
    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    return(ret);
}




__s32 dmx_stream_open
(
    struct inode *inode,
    struct file  *file
)
{
    __s32                     i;
    struct dmx_output_device *dev;
    struct dmx_stream        *stream;

    DMX_LINUX_API_DEBUG("%s, go\n", __FUNCTION__);

    /* For standby test. Root cause still unkonwn.
     * Date:2014.11.13
     */
    //dmx_see_init();
     dmx_internal_init();

    dev = container_of(inode->i_cdev, struct dmx_output_device, cdev);

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        return(-ERESTARTSYS);
    }

    #if 0
    /* Also need to lock see to main buffer.
	*/
    if (dmx_mutex_output_lock(6))
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }	
    #endif
	
    for (i = 0; i < DMX_STREAM_CNT; i++)
    {
        stream = &(ali_dmx_stream_module.stream[i]);

        if (DMX_STREAM_STATE_IDLE == stream->state)
        {
            stream->state = DMX_STREAM_STATE_CFG;

            stream->dmx_output_device = (void *)dev;

            file->private_data = stream;

            DMX_LINUX_API_DEBUG("Got idle stream %d\n", i);

            break;
        }
    }

    dmx_mutex_output_unlock(6);

    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    if (i >= DMX_STREAM_CNT)
    {
        DMX_LINUX_API_DEBUG("No idle stream!\n");

        return(-EMFILE);
    }

    return(0);
}



#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_stream_ioctl
(
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#else
__s32 dmx_stream_ioctl
(
    struct inode *inode,
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#endif
{
    struct dmx_stream         *stream; 
    struct dmx_output_device  *dev;
    __s32                        ret;

    ret = 0;

    stream = filp->private_data;

    dev = (struct dmx_output_device *)stream->dmx_output_device;

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        return(-ERESTARTSYS);
    }

    #if 0
    /* Also need to lock see to main buffer.
	*/
    if (dmx_mutex_output_lock(6))
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }	
	#endif
	
    switch(cmd)
    {
        case ALI_DMX_SEC_STREAM_CFG:
        {
            ret = dmx_sec_stream_cfg(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_SEC_STREAM_START:
        {
            ret = dmx_sec_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_SEC_STREAM_STOP:
        {
            ret = dmx_sec_stream_stop(dev, stream);
        }
        break;

        case ALI_DMX_SEC_STREAM_INFO_GET:
        {
            ret = dmx_sec_stream_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_SEC_FILTER_INFO_GET:
        {
            ret = dmx_sec_filter_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_TS_STREAM_CFG:
        {
            ret = dmx_ts_stream_cfg(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_TS_STREAM_START:
        {
            ret = dmx_ts_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_TS_STREAM_STOP:
        {
            ret = dmx_ts_stream_stop(dev, stream);
        }
        break;

        case ALI_DMX_TS_FILTER_INFO_GET:
        {
            ret = dmx_ts_filter_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_TP_STREAM_CFG:
        {
            ret = dmx_tp_stream_cfg(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_TP_STREAM_START:
        {
            ret = dmx_tp_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_TP_STREAM_STOP:
        {
            ret = dmx_tp_stream_stop(dev, stream);
        }
        break;		

        /* Need to be implmented.
		*/
		#if 0
        case ALI_DMX_TP_STREAM_INFO_GET:
        {
            ret = dmx_tp_stream_info_get(dev, stream, cmd, arg);
        }
        break;
		#endif

        case ALI_DMX_VIDEO_STREAM_CFG:
        {
            ret = dmx_video_stream_cfg(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_VIDEO_STREAM_START:
        {
            ret = dmx_video_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_VIDEO_STREAM_STOP:
        {
            ret = dmx_video_stream_stop(dev, stream);
        }
        break;

        case ALI_DMX_VIDEO_STREAM_INFO_GET:
        {
            ret = dmx_video_stream_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_VIDEO_FILTER_INFO_GET:
        {
            ret = dmx_video_filter_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_VIDEO_SEE_INFO_GET:
        {
            ret = dmx_video_see_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_AUDIO_STREAM_CFG:
        {
            ret = dmx_audio_stream_cfg(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_AUDIO_STREAM_START:
        {
            ret = dmx_audio_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_AUDIO_STREAM_STOP:
        {
            ret = dmx_audio_stream_stop(dev, stream);
        }
        break;

        case ALI_DMX_AUDIO_STREAM_INFO_GET:
        {
            ret = dmx_audio_stream_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_AUDIO_FILTER_INFO_GET:
        {
            ret = dmx_audio_filter_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_AUDIO_SEE_INFO_GET:
        {
            ret = dmx_audio_see_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_PCR_STREAM_CFG:
        {
            ret = dmx_pcr_stream_cfg(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_PCR_STREAM_START:
        {
            ret = dmx_pcr_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_PCR_STREAM_STOP:
        {
            ret = dmx_pcr_stream_stop(dev, stream);
        }
        break;

        case ALI_DMX_PCR_STREAM_INFO_GET:
        {
            ret = dmx_pcr_stream_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_PCR_FILTER_INFO_GET:
        {
            ret = dmx_pcr_filter_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_HW_REG_STREAM_CFG:
        {
            ret = dmx_hw_reg_stream_cfg(dev, stream);
        }
        break;

        case ALI_DMX_HW_REG_STREAM_START:
        {
            ret = dmx_hw_reg_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_HW_REG_STREAM_STOP:
        {
            ret = dmx_hw_reg_stream_stop(dev, stream);
        }
        break;

        case ALI_DMX_HW_REG_STREAM_INFO_GET:
        {
            ret = dmx_hw_reg_stream_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_KERN_GLB_STREAM_CFG:
        {
            ret = dmx_kern_glb_stream_cfg(dev, stream);
        }
        break;

        case ALI_DMX_KERN_GLB_STREAM_START:
        {
            ret = dmx_kern_glb_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_KERN_GLB_STREAM_STOP:
        {
            ret = dmx_kern_glb_stream_stop(dev, stream);
        }
        break;

        case ALI_DMX_KERN_GLB_STREAM_INFO_GET:
        {
            ret = dmx_kern_glb_stream_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_KERN_GLB_STREAM_REALTIME_SET:
        {
            ret = dmx_kern_glb_stream_realtime_set(dev, stream, cmd, arg);
        }
		break;

        case ALI_DMX_SEE_GLB_STREAM_CFG:
        {
            ret = dmx_see_glb_stream_cfg(dev, stream);
        }
        break;

        case ALI_DMX_SEE_GLB_STREAM_START:
        {
            ret = dmx_see_glb_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_SEE_GLB_STREAM_STOP:
        {
            ret = dmx_see_glb_stream_stop(dev, stream);
        }
        break;

        case ALI_DMX_SEE_GLB_STREAM_INFO_GET:
        {
            ret = dmx_see_glb_stream_info_get(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_SEE_GLB_STREAM_REALTIME_SET:
        {
            ret = dmx_see_glb_stream_realtime_set(dev, stream, cmd, arg);
        }
		break;

        default: 
        {
            ret = -ENOTTY;
        } 
        break;
    }

    #if 0
    dmx_mutex_output_unlock(6);
    #endif
	
    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    return(ret);
}





__s32 dmx_stream_read
(
    struct file *file,
    char __user *buf,
    size_t       count,
    loff_t      *ppos
)
{
    struct dmx_output_device *dev;
    struct dmx_stream        *stream; 
    __s32                     ret;

    //printk("%s, %d\n", __FUNCTION__, __LINE__);

    stream = file->private_data;

    dev = (struct dmx_output_device *)stream->dmx_output_device;

    if (dmx_mutex_output_lock(dev->src_hw_interface_id))
    {
        return(-ERESTARTSYS);
    }

    #if 0
    /* Also need to lock see to main buffer.
	*/
    if (dmx_mutex_output_lock(6))
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);
    }		
    #endif
	
    ret = 0;

    //printk("%s, %d\n", __FUNCTION__, __LINE__);

    switch(stream->type)
    {
        //printk("%s, %d\n", __FUNCTION__, __LINE__);

        case DMX_STREAM_TYPE_SEC:
        {
            //printk("%s, %d\n", __FUNCTION__, __LINE__);
			
            ret = dmx_sec_stream_read(dev, stream, file->f_flags, buf, count);
        }
        break; 

        case DMX_STREAM_TYPE_TS:
        {
            //printk("%s, %d\n", __FUNCTION__, __LINE__);
			
            ret = dmx_ts_stream_read(dev, stream, file->f_flags, buf, count);
        }
        break; 

        case DMX_STREAM_TYPE_PCR:
        {
            //printk("%s, %d\n", __FUNCTION__, __LINE__);
			
            ret = dmx_pcr_stream_read(dev, stream, file->f_flags, buf, count);
        }
        break; 

        case DMX_STREAM_TYPE_TP:
        {
            //printk("%s, %d\n", __FUNCTION__, __LINE__);
			
            ret = dmx_tp_stream_read(dev, stream, file->f_flags, buf, count);
        }
        break; 		

        default:
        {
            printk("%s, %d\n", __FUNCTION__, __LINE__);
			
            ret = -EPERM;
        }
        break;  
    }

    //printk("%s, %d\n", __FUNCTION__, __LINE__);
    #if 0
    dmx_mutex_output_unlock(6);
    #endif
	
    dmx_mutex_output_unlock(dev->src_hw_interface_id);

    //printk("%s, %d\n", __FUNCTION__, __LINE__);

    return(ret);
}




__s32 dmx_stream_module_init
(
    void
)
{
    __u32 i;

    for (i = 0; i < DMX_STREAM_CNT; i++)
    {
        ali_dmx_stream_module.stream[i].state = DMX_STREAM_STATE_IDLE;
		
        ali_dmx_stream_module.stream[i].stream_id = i;
    }

    return(0);
}


