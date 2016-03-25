#include <ali_interrupt.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/delay.h>


#include "dmx_see_interface.h"
#include "dmx_stack.h"

struct dmx_instream_ram_module_legacy ali_dmx_instream_ram_module_legacy;



__s32 dmx_instream_ram_legacy_hw_buf_flush
(
    struct dmx_input_device *dev
)
{
    __u32 dest_buf_wr;
    
    if (dmx_mutex_output_lock(dev->dest_dev_id))
    {
        return(-ERESTARTSYS);
    }
    
    dest_buf_wr = dmx_hw_buf_wr_get(dev->dest_dev_id);

    dmx_hw_buf_rd_set(dev->dest_dev_id, dest_buf_wr);

    dmx_mutex_output_unlock(dev->dest_dev_id);

    return(0);
}



#if 0
ssize_t dmx_instream_ram_legacy_write
(
    struct file       *file,
    const char __user *src_buf,
    size_t             usr_wr_len,
    loff_t            *ppos
)
{
    struct dmx_input_device        *dev;
    struct dmx_instream_ram_legacy *stream; 
    __u32                           free_unit;
    __u32                           free_len;
    __u32                           wr_len;
    __u32                           wr_unit;
    __u32                           dest_buf_end;
    __u32                           dest_buf_rd;
    __u32                           dest_buf_wr;
    __u32                           dest_buf_wr_next;
    __u8                           *dest_buf;

    //printk("%s, %d\n", __FUNCTION__, __LINE__);

    stream = file->private_data;

    dev = (struct dmx_input_device *)stream->dmx_input_device;

    if (dmx_mutex_input_lock(dev->dest_dev_id))
    {
        //printk("%s, %d\n", __FUNCTION__, __LINE__);
        
        return(-ERESTARTSYS);
    }

    if (stream->state != DMX_INSTREAM_RAM_STATE_RUN_LEGACY)
    {
        dmx_mutex_input_unlock(dev->dest_dev_id);

        //printk("%s, %d\n", __FUNCTION__, __LINE__);

        return(-EPERM);
    }

    dest_buf_end = dmx_hw_buf_end_get(dev->dest_dev_id);

    dest_buf_rd = dmx_hw_buf_rd_get(dev->dest_dev_id);

    dest_buf_wr = dmx_hw_buf_wr_get(dev->dest_dev_id);


#if 1
    if (dest_buf_rd <= dest_buf_wr)
    {
#if 1
        if (0 == dest_buf_rd)
        {
            //printk("%s,%d\n", __FUNCTION__, __LINE__);

            free_unit = dest_buf_end - dest_buf_wr - 1;
        }
        else
        {
        
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
            free_unit = dest_buf_end - dest_buf_wr;
        }
#else
        free_unit = dest_buf_end - dest_buf_wr;
#endif
    }
    else
    {
        free_unit = dest_buf_rd - dest_buf_wr - 1;
    }
#else
    if (0 == dest_buf_rd)
    {
        dest_buf_wr_end = dest_buf_end;
    }





    if (dest_buf_rd <= dest_buf_wr)
    {
        if (0 == dest_buf_rd)
        {
            dest_buf_wr_end = dest_buf_end;
        }
        else
        {
            dest_buf_wr_end = dest_buf_end;

        }

    }
#endif

    /* Buffer full, nothing could be write in.
    */
    if (0 == free_unit)
    {
        dmx_mutex_input_unlock(dev->dest_dev_id);

        if (file->f_flags & O_NONBLOCK)
        {
            //printk("%s,%d,rd:%d,wr:%d,end:%d\n", __FUNCTION__, __LINE__,
                     //dest_buf_rd, dest_buf_wr, dest_buf_end);
            
            return(-EAGAIN);
        }
        else
        {
            //printk("%s,%d,rd:%d,wr:%d,end:%d\n", __FUNCTION__, __LINE__,
                     //dest_buf_rd, dest_buf_wr, dest_buf_end);

            return(0);
        }
    }   

    free_len = free_unit * 188;

    if (free_len > usr_wr_len)
    {
        wr_len = usr_wr_len;

        wr_unit = usr_wr_len / 188;
    }
    else
    {
        wr_len = free_len;

        wr_unit = free_unit;
    }

    dest_buf = (__u8 *)dmx_hw_buf_start_addr_get(dev->dest_dev_id);
    
    dest_buf += (dest_buf_wr * 188);

    if (copy_from_user(dest_buf, src_buf, wr_len))
    {    
        dmx_mutex_input_unlock(dev->dest_dev_id);
    
        //printk("%s, %d\n", __FUNCTION__, __LINE__);
    
        return(-EFAULT);
    }
    
    dest_buf_wr_next = dest_buf_wr + wr_unit;
    
    if (dest_buf_wr_next >= dest_buf_end)
    //if (dest_buf_wr_next > dest_buf_end)
    {
       dest_buf_wr_next = 0;
    }
    
    dmx_hw_buf_wr_set(dev->dest_dev_id, dest_buf_wr_next);

    //printk("%s,%d,dest_buf_rd:%d,dest_buf_wr:%d\n", __FUNCTION__, __LINE__, dest_buf_rd, dest_buf_wr);

    dmx_mutex_input_unlock(dev->dest_dev_id);

    //printk("%s, %d\n", __FUNCTION__, __LINE__);

    return(wr_len);
}
#else


ssize_t dmx_instream_ram_legacy_write
(
    struct file       *file,
    const char __user *src_buf,
    size_t             usr_wr_len,
    loff_t            *ppos
)
{
    struct dmx_input_device        *dev;
    struct dmx_instream_ram_legacy *stream; 
	struct Ali_DmxDrvTsInRamStrmStatInfo *stat_info; 
    __u32                           remain_len;
    __u32                           copied_len;
    __u32                           free_len;
    __u32                           wr_len;
    __u32                           dest_buf_end;
    __u32                           dest_buf_rd;
    __u32                           dest_buf_wr;
    __u32                           dest_buf_wr_end;
    __u8                           *dest_buf;
    __s32                           err;

    DMX_API_DBG("%s,%d,usr_wr_len:%d\n", __FUNCTION__, __LINE__, usr_wr_len);

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

    if (stream->state != DMX_INSTREAM_RAM_STATE_RUN_LEGACY)
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

        //printk("%s():%d, dest_buf_rd[%x], dest_buf_wr[%x] \n", __FUNCTION__, __LINE__,dest_buf_rd,dest_buf_wr);

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
            /* copy wr->wr_end
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

            if (copy_from_user(dest_buf, src_buf + copied_len, wr_len))
            {    
                dmx_mutex_input_unlock(dev->dest_dev_id);
            
				stat_info->CopyErrCnt++;

                printk("%s,%d,dest_buf[%x],src_buf[%x],copied_len[%u],wr_len[%u]\n",
                       __FUNCTION__, __LINE__, (__u32)dest_buf, (__u32)src_buf, copied_len,
                       wr_len);

                err = -EFAULT;
                
                goto ERR_EXIT;
            }            

//          dmx_hw_buf_wr_set(dev->dest_dev_id, dest_buf_wr + (wr_len / 188));
            dmx_hw_buf_wr_set(dev->dest_dev_id, dest_buf_wr + wr_len );

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

            /* src_wr_len - remain_len == copied_len
            */
            if (copy_from_user(dest_buf, src_buf + copied_len, wr_len))
            {    
                dmx_mutex_input_unlock(dev->dest_dev_id);
            
				stat_info->CopyErrCnt++;

                printk("%s(),%d, dest_buf[%x],src_buf[%x],copied_len[%d],wr_len[%d],"
					   "dest_buf_rd[%x], dest_buf_wr[%x],buf_end[%x]\n",
                        __FUNCTION__, __LINE__, (__u32)dest_buf, (__u32)src_buf, copied_len,
                        wr_len, dest_buf_rd, dest_buf_wr, dest_buf_end);
            
                err = -EFAULT;
                
                goto ERR_EXIT;
            }

            dmx_hw_buf_wr_set(dev->dest_dev_id, dest_buf_wr + wr_len);

            copied_len += wr_len;

            if (copied_len >= usr_wr_len)
            {
                goto OK_EXIT;
            }

            /* Copy second half. buf_start=>wr_end
            */
            remain_len = usr_wr_len - copied_len;
            
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
            if (copy_from_user(dest_buf, src_buf + copied_len, wr_len))
            {    
                dmx_mutex_input_unlock(dev->dest_dev_id);
            
				stat_info->CopyErrCnt++;

                printk("%s,%d,dest_buf[%x],src_buf[%x],copied_len[%d],wr_len[%d]\n",
                        __FUNCTION__, __LINE__, (__u32)dest_buf, (__u32)src_buf,
                        copied_len, wr_len);
            
                err = -EFAULT;
                
                goto ERR_EXIT;
            }

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

    DMX_API_DBG("%s,%d,usr_wr_len:%d,copied_len:%d\n",
           __FUNCTION__, __LINE__, usr_wr_len, copied_len);
    
    return(copied_len);
}


#endif


__s32 dmx_instream_ram_legacy_open
(
    struct inode *inode,
    struct file  *file
)
{
    __s32                           i;
    struct dmx_input_device        *dev;
    struct dmx_instream_ram_legacy *stream;

    DMX_API_DBG("%s, go\n", __FUNCTION__);

    dev = container_of(inode->i_cdev, struct dmx_input_device, cdev);

    if (dmx_mutex_input_lock(dev->dest_dev_id))
    {
        return(-ERESTARTSYS);
    }

    for (i = 0; i < DMX_LINUX_INPUT_DEV_CNT_LEGACY; i++)
    {
        stream = &(ali_dmx_instream_ram_module_legacy.stream[i]);

        if (DMX_INSTREAM_RAM_STATE_IDLE_LEGACY == stream->state)
        {
            stream->state = DMX_INSTREAM_RAM_STATE_RUN_LEGACY;

            stream->dmx_input_device = (void *)dev;

            file->private_data = stream;

            DMX_API_DBG("Got idle stream %d\n", i);

            break;
        }
    }

    dmx_mutex_input_unlock(dev->dest_dev_id);

    if (i >= DMX_LINUX_INPUT_DEV_CNT_LEGACY)
    {
        DMX_API_DBG("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EMFILE);
    }

    return(0);
}



#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_instream_ram_legacy_ioctl
(
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#else
__s32 dmx_instream_ram_legacy_ioctl
(
    struct inode *inode,
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#endif
{
    struct dmx_instream_ram_legacy 			*stream; 
	struct Ali_DmxDrvTsInRamStrmStatInfo 	*stat_info; 
    struct dmx_input_device        			*dev;
    __s32                             		ret;

    ret = 0;

    stream = filp->private_data;

	stat_info = &stream->stat_info;

    dev = (struct dmx_input_device *)stream->dmx_input_device;

    if (dmx_mutex_input_lock(dev->dest_dev_id))
    {
        return(-ERESTARTSYS);
    }

    switch(cmd)
    {
        case ALI_DMX_CHANNEL_FLUSH_BUF:
        {
            //ret = dmx_instream_ram_legacy_cfg(dev, stream, cmd, arg);
            ret = dmx_instream_ram_legacy_hw_buf_flush(dev);
        }
        break;

        case ALI_DMX_IO_SET_PLAYBACK_SRC_TYPE:
        {
            //ret = dmx_instream_ram_legacy_start(dev, stream);
        }
        break;

        case ALI_DMX_PLAYBACK_FSTCPY:
        {
            //ret = dmx_instream_ram_legacy_stop(dev, stream);
#ifdef CONFIG_FAST_COPY
             __s32                    byte_cnt;
            struct ali_dmx_data_buf * dest_buf;
            struct dmx_fast_copy_param   param;

#if 0
            struct dmx_device        *dmx;

            dmx = (struct dmx_device *)stream->dmx_input_device;
         
            if (DMX_PLAYBACK_IN_IDLE == dmx->playback_in_status)
            {
                return(-EPERM);
            }

            copy_from_user((void *)&param, arg, sizeof(struct dmx_fast_copy_param));

            dest_buf = &(dmx->playback_in_buf);
#endif

            if (copy_from_user((void *)&param, arg, sizeof(struct dmx_fast_copy_param)))
            {    
				stat_info->CopyErrCnt++;

                printk("%s(), %d, copy_from_user() fail! \n", __FUNCTION__, __LINE__);

                return(-EFAULT); 

            }


            dest_buf = (struct ali_dmx_data_buf *)stream->dmx_input_device;

            dest_buf->fst_cpy_size = 0;
            dest_buf->exp_cpy_size = param.len;
//          byte_cnt = dmx_data_buf_usr_wr_data(dmx, filp, dest_buf, param.data, param.len, 1);
            byte_cnt = dmx_data_buf_wr_data(dest_buf, param.data, param.len, DMX_DATA_SRC_TYPE_FAST);


            ret = byte_cnt;
#endif
            
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




__s32 dmx_instream_ram_legacy_close
(
    struct inode *inode,
    struct file  *file
)
{
    struct dmx_input_device *dev;
    struct dmx_instream_ram_legacy *stream;

    stream = file->private_data;

    dev = (struct dmx_input_device *)stream->dmx_input_device;

    if (dmx_mutex_input_lock(dev->dest_dev_id))
    {
        return(-ERESTARTSYS);
    }

    stream->state = DMX_INSTREAM_RAM_STATE_IDLE_LEGACY;

    dmx_mutex_input_unlock(dev->dest_dev_id);

    return(0);
}




__s32 dmx_instream_ram_module_legacy_init
(
    void
)
{
    __u32 i;

    for (i = 0; i < DMX_LINUX_INPUT_DEV_CNT_LEGACY; i++)
    {
        ali_dmx_instream_ram_module_legacy.stream[i].state = 
            DMX_INSTREAM_RAM_STATE_IDLE_LEGACY;
    }

    return(0);
}



