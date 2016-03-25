
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <asm/io.h>
#include <ali_interrupt.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/delay.h>

#include <linux/mm.h>


#include "dmx_see_interface.h"
#include "dmx_stack.h"

struct dmx_ts_in_ram_module ali_dmx_ts_in_ram_module;




__s32 dmx_ts_in_ram_stream_write
(
    struct file       *file,
    const char __user *src_buf,
    size_t             usr_wr_len,
    loff_t            *ppos
)
{
    struct dmx_input_device     *dev;
    struct dmx_ts_in_ram_stream *stream; 
	struct Ali_DmxDrvTsInRamStrmStatInfo *stat_info; 
    __u32                        remain_len;
    __u32                        copied_len;
    __u32                        free_len;
    __u32                        wr_len;
    __u32                        dest_buf_end;
    __u32                        dest_buf_rd;
    __u32                        dest_buf_wr;
    __u32                        dest_buf_wr_end;
    __u8                        *dest_buf;
	__s32                        err;

#ifdef CONFIG_FAST_COPY
    void  *qbuf = (void *)src_buf;
    __s32  quantum_id = (__s32)(((__u32)src_buf>>16) & 0xffff);
    __s32  offset = (__s32)((__u32)src_buf & 0xffff);

    if (feature_is_fastcopy_enable())
    {
        qbuf = fcp_get_quantum(quantum_id, 1) + offset;         
    }  

#endif


//	DMX_API_DBG("%s(),%d,usr_wr_len:%d\n", __FUNCTION__, __LINE__, usr_wr_len);

    stream = file->private_data;
	
	stat_info = &stream->stat_info;

	err = 0;

    dev = (struct dmx_input_device *)stream->dmx_input_device;

    if (dmx_mutex_input_lock(dev->dest_dev_id))
    {
        printk("%s, %d\n", __FUNCTION__, __LINE__);
        
        err = -ERESTARTSYS;

		goto ERR_EXIT;
    }

    if (stream->state != DMX_STREAM_STATE_RUN)
    {
        dmx_mutex_input_unlock(dev->dest_dev_id);

		stat_info->StatErrCnt++;

        printk("%s, %d\n", __FUNCTION__, __LINE__);

    	err = -EPERM;
    	
    	goto ERR_EXIT;
    }

    dest_buf_end = dmx_hw_buf_end_get(dev->dest_dev_id);

	copied_len = 0;

	//while (copied_len < usr_wr_len)
	for (;;)
	{
        dest_buf_rd = dmx_hw_buf_rd_get(dev->dest_dev_id);
    
        dest_buf_wr = dmx_hw_buf_wr_get(dev->dest_dev_id);

		if (0 == dest_buf_rd)
		{
			dest_buf_wr_end = dest_buf_end - 1;
		}
		else
		{
            dest_buf_wr_end = dest_buf_rd - 1;
		}
    
    	if (dest_buf_wr == dest_buf_wr_end)
        {
    		if (file->f_flags & O_NONBLOCK)
    		{
    			//printk("%s,%d,rd:%d,wr:%d,end:%d\n", __FUNCTION__, __LINE__,
    					//dest_buf_rd, dest_buf_wr, dest_buf_end);
   
				err = -EAGAIN;
				
				goto ERR_EXIT;
    		}
    	}
		else if (dest_buf_wr < dest_buf_wr_end)
		{
			/* copy wr=>wr_end
			*/
		    remain_len = usr_wr_len - copied_len;
			
			//free_len = (dest_buf_wr_end - dest_buf_wr) * 188;
			free_len = dest_buf_wr_end - dest_buf_wr;

			if (free_len > remain_len)
			{
                wr_len = remain_len;
			}
			else
			{
                wr_len = free_len;
			}
		    
		    dest_buf = (__u8 *)dmx_hw_buf_start_addr_get(dev->dest_dev_id);
    
            //dest_buf += (dest_buf_wr * 188);
            dest_buf += dest_buf_wr;
            
#ifdef CONFIG_FAST_COPY
            if (feature_is_fastcopy_enable())
            {
				memcpy(((void *)dest_buf), qbuf + copied_len, wr_len);           
            }
            else
            {
				if (copy_from_user(dest_buf, src_buf + copied_len, wr_len))
				{    
					dmx_mutex_input_unlock(dev->dest_dev_id);

					stat_info->CopyErrCnt++;

				    printk("%s(), %d, copy fail!\n", __FUNCTION__, __LINE__);

				    err = -EFAULT;
				    
				    goto ERR_EXIT;
				}
            }
#else

			if (copy_from_user(dest_buf, src_buf + copied_len, wr_len))
			{	 
				dmx_mutex_input_unlock(dev->dest_dev_id);
			
				stat_info->CopyErrCnt++;

				printk("%s(), %d, copy fail!\n", __FUNCTION__, __LINE__);

				err = -EFAULT;
				
				goto ERR_EXIT;
			}
#endif
           
			//dmx_hw_buf_wr_set(dev->dest_dev_id, dest_buf_wr + (wr_len / 188));
			dmx_hw_buf_wr_set(dev->dest_dev_id, dest_buf_wr + wr_len);

			copied_len += wr_len;

			if (copied_len >= usr_wr_len)
			{
				goto OK_EXIT;
			}
		}
		/* Must be dest_buf_wr > dest_buf_wr_end
		*/
		else
		{
		    /* Copy first half. wr=>buf_end
			*/
		    remain_len = usr_wr_len - copied_len;
			
			//free_len = (dest_buf_end - dest_buf_wr) * 188;
			free_len = dest_buf_end - dest_buf_wr;

			if (free_len > remain_len)
			{
                wr_len = remain_len;
			}
			else
			{
                wr_len = free_len;
			}
		    
		    dest_buf = (__u8 *)dmx_hw_buf_start_addr_get(dev->dest_dev_id);
    
            //dest_buf += (dest_buf_wr * 188);
            dest_buf += dest_buf_wr;

			/* sr_wr_len - remain_len == copied_len
			*/
#ifdef CONFIG_FAST_COPY
			if (feature_is_fastcopy_enable())
			{
			    memcpy(((void *)dest_buf), qbuf + copied_len, wr_len);           
			}
			else
			{
			    if (copy_from_user(dest_buf, src_buf + copied_len, wr_len))
			    {    
			        dmx_mutex_input_unlock(dev->dest_dev_id);

					stat_info->CopyErrCnt++;

			        printk("%s(), %d, copy fail!\n", __FUNCTION__, __LINE__);

			        err = -EFAULT;
			        
			        goto ERR_EXIT;
			    }
			}
#else

			if (copy_from_user(dest_buf, src_buf + copied_len, wr_len))
			{	 
				dmx_mutex_input_unlock(dev->dest_dev_id);
			
				stat_info->CopyErrCnt++;

				printk("%s(), %d, copy fail!\n", __FUNCTION__, __LINE__);
			
    			err = -EFAULT;
    			
    			goto ERR_EXIT;
			}

#endif
		
			if (dest_buf_wr + wr_len >= dest_buf_end)
			{
				dmx_hw_buf_wr_set(dev->dest_dev_id, 0);
			}
			else
			{
				dmx_hw_buf_wr_set(dev->dest_dev_id, dest_buf_wr + wr_len);
			}

			copied_len += wr_len;

			if (copied_len >= usr_wr_len)
			{
				goto OK_EXIT;
			}

		    /* Copy second half. buf_start=>wr_end
			*/
		    remain_len = usr_wr_len - copied_len;
			
			//free_len = dest_buf_wr_end * 188;
			free_len = dest_buf_wr_end;

			if (free_len > remain_len)
			{
                wr_len = remain_len;
			}
			else
			{
                wr_len = free_len;
			}
		    
		    dest_buf = (__u8 *)dmx_hw_buf_start_addr_get(dev->dest_dev_id);
    
			/* sr_wr_len - remain_len == copied_len
			*/
#ifdef CONFIG_FAST_COPY
            if (feature_is_fastcopy_enable())
            {
                memcpy(((void *)dest_buf), qbuf + copied_len, wr_len);           
            }
            else
            {
                if (copy_from_user(dest_buf, src_buf + copied_len, wr_len))
                {    
                    dmx_mutex_input_unlock(dev->dest_dev_id);

					stat_info->CopyErrCnt++;

                    printk("%s(), %d, copy fail!\n", __FUNCTION__, __LINE__);

                    err = -EFAULT;
                    
                    goto ERR_EXIT;
                }
            }
#else
			
			if (copy_from_user(dest_buf, src_buf + copied_len, wr_len))
			{	 
				dmx_mutex_input_unlock(dev->dest_dev_id);
			
				stat_info->CopyErrCnt++;

				printk("%s(), %d, copy fail!\n", __FUNCTION__, __LINE__);
			
    			err = -EFAULT;
    			
    			goto ERR_EXIT;
			}

#endif
			//dmx_hw_buf_wr_set(dev->dest_dev_id, wr_len / 188);
			dmx_hw_buf_wr_set(dev->dest_dev_id, wr_len);

			copied_len += wr_len; 
    		
    		if (copied_len >= usr_wr_len)
    		{
				goto OK_EXIT;
    		}
		}
		
		if (file->f_flags & O_NONBLOCK)
		{
			//printk("%s,%d,rd:%d,wr:%d,end:%d\n", __FUNCTION__, __LINE__,
					//dest_buf_rd, dest_buf_wr, dest_buf_end);
			
			goto OK_EXIT;
		}
		else
		{
		    dmx_mutex_input_unlock(dev->dest_dev_id);
			
            msleep_interruptible(50);

            if (dmx_mutex_input_lock(dev->dest_dev_id))
            {
                printk("%s, %d\n", __FUNCTION__, __LINE__);
                
                err = -ERESTARTSYS;
        
        		goto ERR_EXIT;
            }			
		}
	}

ERR_EXIT:	
    dmx_mutex_input_unlock(dev->dest_dev_id);
	
	return(err);	

OK_EXIT:
    dmx_mutex_input_unlock(dev->dest_dev_id);

	//printk("%s,%d,usr_wr_len:%d,copied_len:%d\n",
		    //__FUNCTION__, __LINE__, usr_wr_len, copied_len);
	
	return(copied_len);
}







__s32 dmx_ts_in_ram_stream_cfg
(
    struct dmx_input_device *dev,
    struct dmx_ts_in_ram_stream *stream,
    unsigned int             cmd,
    unsigned long            arg
)
{
	struct Ali_DmxDrvTsInRamStrmStatInfo *stat_info; 

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

	stat_info = &stream->stat_info;

    /* Stream startus validation. 
     */
    if ((DMX_STREAM_STATE_CFG  != stream->state) &&
        (DMX_STREAM_STATE_STOP != stream->state))		
    {
        dmx_mutex_input_unlock(dev->dest_dev_id);

		stat_info->StatErrCnt++;

        return(-EPERM);
    }

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}




__s32 dmx_ts_in_ram_stream_start
(
    struct dmx_input_device     *dev,
    struct dmx_ts_in_ram_stream *stream
)
{   
	struct Ali_DmxDrvTsInRamStrmStatInfo *stat_info; 

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

	stat_info = &stream->stat_info;

    /* Stream status validation. 
     */
    if (DMX_STREAM_STATE_STOP != stream->state)
    {
        dmx_mutex_input_unlock(dev->dest_dev_id);

		stat_info->StatErrCnt++;

        return(-EPERM);
    }

    stream->state = DMX_STREAM_STATE_RUN;

    return(0);
}



__s32 dmx_ts_in_ram_stream_stop
(
    struct dmx_input_device *dev,
    struct dmx_ts_in_ram_stream *stream
)
{
	struct Ali_DmxDrvTsInRamStrmStatInfo *stat_info; 

    DMX_API_DBG("%s, line:%d\n", __FUNCTION__, __LINE__);

	stat_info = &stream->stat_info;

    /* Stream startus validation. 
     */
    if (DMX_STREAM_STATE_RUN != stream->state)
    {
        dmx_mutex_input_unlock(dev->dest_dev_id);

		stat_info->StatErrCnt++;

        return(-EPERM);
    }

    stream->state = DMX_STREAM_STATE_STOP;

    return(0);
}




__s32 dmx_ts_in_ram_stream_open
(
    struct inode *inode,
    struct file  *file
)
{
    __s32                    i;
    struct dmx_input_device *dev;
    struct dmx_ts_in_ram_stream *stream;

    DMX_API_DBG("%s, go\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, struct dmx_input_device, cdev);

    if (dmx_mutex_input_lock(dev->dest_dev_id))
    {
        return(-ERESTARTSYS);
    }

    for (i = 0; i < DMX_STREAM_CNT; i++)
    {
        stream = &(ali_dmx_ts_in_ram_module.stream[i]);

        if (DMX_STREAM_STATE_IDLE == stream->state)
        {
            stream->state = DMX_STREAM_STATE_CFG;

            stream->dmx_input_device = (void *)dev;

            init_waitqueue_head(&(stream->wr_wq));

            file->private_data = stream;

            DMX_API_DBG("Got idle stream %d\n", i);

            break;
        }
    }

    dmx_mutex_input_unlock(dev->dest_dev_id);

    if (i >= DMX_STREAM_CNT)
    {
        DMX_API_DBG("No idle stream!\n");

        return(-EMFILE);
    }

    return(0);
}



#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_ts_in_ram_stream_ioctl
(
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#else
__s32 dmx_ts_in_ram_stream_ioctl
(
    struct inode *inode,
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#endif
{
    struct dmx_ts_in_ram_stream *stream; 
    struct dmx_input_device *dev;
    __s32                      ret;

    ret = 0;

    stream = filp->private_data;

    dev = (struct dmx_input_device *)stream->dmx_input_device;

    if (dmx_mutex_input_lock(dev->dest_dev_id))
    {
        return(-ERESTARTSYS);
    }

    switch(cmd)
    {
        case ALI_DMX_TS_IN_RAM_STREAM_CFG:
        {
            ret = dmx_ts_in_ram_stream_cfg(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_TS_IN_RAM_STREAM_START:
        {
            ret = dmx_ts_in_ram_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_TS_IN_RAM_STREAM_STOP:
        {
            ret = dmx_ts_in_ram_stream_stop(dev, stream);
        }
        break;

        default: 
        {
            ret = -ENOTTY;
        } 
        break;
    }

    dmx_mutex_input_unlock(dev->dest_dev_id);

    return(ret);
}




__s32 dmx_ts_in_ram_stream_close
(
    struct inode *inode,
    struct file  *file
)
{
    struct dmx_input_device *dev;
    struct dmx_ts_in_ram_stream *stream;

    stream = file->private_data;

    dev = (struct dmx_input_device *)stream->dmx_input_device;

    if (dmx_mutex_input_lock(dev->dest_dev_id))
    {
        return(-ERESTARTSYS);
    }

    stream->state = DMX_STREAM_STATE_IDLE;

    dmx_mutex_input_unlock(dev->dest_dev_id);

    return(0);
}




__u32 dmx_ts_in_ram_stream_poll
(
    struct file              *filp,
    struct poll_table_struct *wait
)
{
    __s32                    mask;
    __u32                    buf_rd;
    __u32                    buf_wr;
    __u32                    buf_wr_next;
    __u32                    buf_end;
    struct dmx_ts_in_ram_stream *stream; 
    struct dmx_input_device *dev;

    stream = filp->private_data;

    mask = 0;

    dev = (struct dmx_input_device *)(stream->dmx_input_device);

    if (dmx_mutex_input_lock(dev->dest_dev_id))
    {
        printk("%s, line:%d\n", __FUNCTION__, __LINE__);

        return(-ERESTARTSYS);
    }
    
    poll_wait(filp, &(stream->wr_wq), wait);
          
    buf_end = dmx_hw_buf_end_get(dev->dest_dev_id);
    
    buf_rd = dmx_hw_buf_rd_get(dev->dest_dev_id);
    
    buf_wr = dmx_hw_buf_wr_get(dev->dest_dev_id);
    
    buf_wr_next = buf_wr++;
    
    if (buf_wr_next >= buf_end)
    {
        buf_wr_next = 0;
    }
    
    /* Buffer not full, there is room could be write in.
    */
    if (buf_wr_next != buf_rd)
    {       
        mask |= (POLLIN | POLLWRNORM); 
    }

    dmx_mutex_input_unlock(dev->dest_dev_id);

    return(mask);
}





__s32 ali_dmx_ts_in_ram_module_init
(
    void
)
{
    __u32 i;

    for (i = 0; i < DMX_INSTREAM_RAM_CNT; i++)
    {
        ali_dmx_ts_in_ram_module.stream[i].state = DMX_STREAM_STATE_IDLE;
    }

    return(0);
}


