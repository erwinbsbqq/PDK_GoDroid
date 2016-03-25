
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/poll.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "dmx_see_interface.h"
#include "dmx_stack.h"
#include "dmx_subt_if.h"


struct file_operations ali_dmx_subt_fops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = dmx_subt_stream_ioctl,
    .open    = dmx_subt_stream_open,
    .release = dmx_subt_stream_close,
};

static struct class *ali_dmx_subt_if_class = NULL;
static struct dmx_subt_device ali_dmx_subt_devices[DMX_SUBT_DEVICE_CNT];
static struct dmx_subt_stream ali_dmx_subt_streams[DMX_SUBT_STREAM_CNT];





__s32 dmx_subt_stream_open
(
    struct inode *inode,
    struct file  *file
)
{
    __s32                   i;
    struct dmx_subt_device *dev;
    struct dmx_subt_stream *stream;

    printk("%s, line:%d\n", __FUNCTION__, __LINE__);

    dev = container_of(inode->i_cdev, struct dmx_subt_device, linux_cdev);

    if (dmx_mutex_lock(dev->src_hw_if_id))
    {
        return(-ERESTARTSYS);
    }

    for (i = 0; i < DMX_SUBT_STREAM_CNT; i++)
    {
        stream = &(ali_dmx_subt_streams[i]);

        if (DMX_SUBT_STREAM_STATE_IDLE == stream->state)
        {
            stream->state = DMX_SUBT_STREAM_STATE_CFG;

			stream->subt_device = dev;

            file->private_data = stream;

            printk("%s,%d,Got idle subt id:%d!\n", __FUNCTION__, __LINE__, i);

            break;
        }
    }

    dmx_mutex_unlock(dev->src_hw_if_id);

    if (i >= DMX_SUBT_STREAM_CNT)
    {
        printk("%s,%d,No idle subt left!\n", __FUNCTION__, __LINE__);

        return(-EMFILE);
    }

    printk("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}





__s32 dmx_subt_stream_close
(
    struct inode *inode,
    struct file  *file
)
{
    __s32                   ret;
    struct dmx_subt_device *dev;
    struct dmx_subt_stream *stream;

    printk("%s, line:%d\n", __FUNCTION__, __LINE__);

    stream = file->private_data;

    dev = stream->subt_device;

    if (dmx_mutex_lock(dev->src_hw_if_id))
    {
        return(-ERESTARTSYS);
    }

    if (DMX_SUBT_STREAM_STATE_IDLE == stream->state)
    {
		ret = -EPERM;

		goto EXIT;
    }	

    if ((DMX_SUBT_STREAM_STATE_STOP == stream->state) ||
		(DMX_SUBT_STREAM_STATE_RUN == stream->state))
    {   
        ret = dmx_ts_flt_unregister(stream->ts_flt_id);
    
        if (ret < 0)
        {    
            printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

		    goto EXIT;
        }
    }

    if (DMX_SUBT_STREAM_STATE_RUN == stream->state)
    {
        dmx_see_subt_stream_stop();
	}
	
    stream->state = DMX_SUBT_STREAM_STATE_IDLE;

EXIT:
    dmx_mutex_unlock(dev->src_hw_if_id);

    printk("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}



__s32 dmx_subt_stream_cfg
(
    struct dmx_subt_device  *dev,
    struct dmx_subt_stream  *stream,
    unsigned int             cmd,
    unsigned long            arg
)
{
    __s32 ret;

    printk("%s, line:%d\n", __FUNCTION__, __LINE__);
	
    /* Stream state validation. 
    */
    if ((DMX_SUBT_STREAM_STATE_CFG  != stream->state) &&
        (DMX_SUBT_STREAM_STATE_STOP != stream->state))
    {		    
        return(-EPERM);
    }

    if (DMX_SUBT_STREAM_STATE_STOP == stream->state)
    {
        ret = dmx_ts_flt_unregister(stream->ts_flt_id);
    
        if (ret < 0)
        {    
            printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

            return(ret);
        }
    }

    ret = copy_from_user(&(stream->usr_param), (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {		    
        printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);

        return(-EFAULT);
    }

    ret = dmx_ts_flt_register(dev->src_hw_if_id, stream->usr_param.Pid, 
		                      dmx_see_buf_wr_ts, dev->src_hw_if_id);

    if (ret < 0)
    {		
        printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    stream->ts_flt_id = ret;

    stream->state = DMX_SUBT_STREAM_STATE_STOP;

    printk("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}





__s32 dmx_subt_stream_start
(
    struct dmx_subt_device *dev,
    struct dmx_subt_stream *stream
)
{
    __s32 ret;
	
    printk("%s, line:%d\n", __FUNCTION__, __LINE__);
	
    /* Stream state validation. 
    */
    if (stream->state != DMX_SUBT_STREAM_STATE_STOP)
    {
        return(-EPERM);
    }

    dmx_see_subt_stream_start(stream->usr_param.Pid);

    ret = dmx_ts_flt_start(stream->ts_flt_id);

    if (ret < 0)
    {
        DMX_API_DBG("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
        
        return(ret);
    }

    stream->state = DMX_SUBT_STREAM_STATE_RUN;

    printk("%s, line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}







__s32 dmx_subt_stream_stop
(
    struct dmx_subt_device *dev,
    struct dmx_subt_stream *stream
)
{
    __s32 ret;
	
    printk("%s, line:%d\n", __FUNCTION__, __LINE__);
	
    /* Stream state validation. 
    */
    if (stream->state != DMX_SUBT_STREAM_STATE_RUN)
    {
        dmx_mutex_unlock(dev->src_hw_if_id);
    
        return(-EPERM);
    }

    ret = dmx_ts_flt_stop(stream->usr_param.Pid);

    if (ret < 0)
    {    
        printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
		
        return(ret);
    }

    dmx_see_subt_stream_stop();

    printk("%s,line:%d\n", __FUNCTION__, __LINE__);

    stream->state = DMX_SUBT_STREAM_STATE_STOP;

    return(0);
}


__s32 dmx_subt_stream_info_get
(
    struct dmx_subt_device  *dev,
    struct dmx_subt_stream  *stream,
    unsigned int             cmd,
    unsigned long            arg
)
{
    __s32                        ret;
	struct Ali_DmxSubtStreamInfo info;

    printk("%s,line:%d\n", __FUNCTION__, __LINE__);
		
    info.StreamIdx = stream->idx;

    info.StreamState = stream->state;

    ret = copy_to_user((void __user *)arg, &info, _IOC_SIZE(cmd));

    if (ret != 0)
    {
        printk("%s,line:%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    printk("%s,line:%d\n", __FUNCTION__, __LINE__);

    return(0);
}




long dmx_subt_stream_ioctl
(
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
{
    __s32                   ret;
    struct dmx_subt_device *dev;
    struct dmx_subt_stream *stream;

    ret = 0;

    stream = filp->private_data;

    dev = (struct dmx_subt_device *)stream->subt_device;	

    printk("%s,line:%d,cmd:%x\n", __FUNCTION__, __LINE__, cmd);

    if (dmx_mutex_lock(dev->src_hw_if_id))
    {
        return(-ERESTARTSYS);
    }
	
    switch(cmd)
    {
        case ALI_DMX_SUBT_STREAM_CFG:
        {
            ret = dmx_subt_stream_cfg(dev, stream, cmd, arg);
        }
        break;

        case ALI_DMX_SUBT_STREAM_START:
        {
            ret = dmx_subt_stream_start(dev, stream);
        }
        break;

        case ALI_DMX_SUBT_STREAM_STOP:
        {
            ret = dmx_subt_stream_stop(dev, stream);
        }
        break;

        case ALI_DMX_SUBT_STREAM_INFO_GET:
        {
            ret = dmx_subt_stream_info_get(dev, stream, cmd, arg);
        }
        break;		

        default: 
        {
            ret = -ENOTTY;
        } 
        break;
    }

    dmx_mutex_unlock(dev->src_hw_if_id);

    printk("%s, line:%d\n", __FUNCTION__, __LINE__);
	
    return(ret);
}



__s32 dmx_subt_if_init
(
    __u32  src_hw_if_id,
    __u8  *this_if_name
)
{
    __s32                   ret;
	__s32                   i;
    struct dmx_subt_device *dev;

    //printk("%s, %d.\n", __FUNCTION__, __LINE__);

    for (i = 0; i < DMX_SUBT_STREAM_CNT; i++)
    {
        ali_dmx_subt_streams[i].idx = i;
    }
	
    if (NULL == ali_dmx_subt_if_class)
    {
        ali_dmx_subt_if_class = class_create(THIS_MODULE, "ali_dmx_subt_if_class");
        
        if (IS_ERR(ali_dmx_subt_if_class))
        {       
            panic("%s,%d,class_create() failed.\n", __FUNCTION__, __LINE__);
        }
    }

    /* Find a free subtitle device interface to control subtitle stream 
     * retrieving from src_hw_if_id.
	*/
    for (i = 0; i < DMX_SUBT_DEVICE_CNT; i++)
    {
        dev = &(ali_dmx_subt_devices[i]);

        if (DMX_SUBT_DEVICE_STATE_IDLE == dev->state)
        {
            dev->state = DMX_SUBT_DEVICE_STATE_RUN;

            printk("%s,%d,Got idle subt device,id:%d!\n", __FUNCTION__, __LINE__, i);

            break;
        }
	}

    if (i >= DMX_SUBT_DEVICE_CNT)
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);
    }

    memset(dev, 0, sizeof(struct dmx_subt_device));

    ret = alloc_chrdev_region(&dev->linux_dev_id, 0, 1, this_if_name);

    if (ret < 0) 
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);
    }

    //printk("%s, %d.\n", __FUNCTION__, __LINE__);

    cdev_init(&(dev->linux_cdev), &ali_dmx_subt_fops);

    dev->linux_cdev.owner = THIS_MODULE;

    ret = cdev_add(&dev->linux_cdev, dev->linux_dev_id, 1);

    /* Fail gracefully if need be.
     */
    if (ret)
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);
    }

    printk("%s, %d.\n", __FUNCTION__, __LINE__);

    dev->linux_dev = device_create(ali_dmx_subt_if_class, NULL, dev->linux_dev_id,
                                   dev, this_if_name);

    if (IS_ERR(dev->linux_dev))
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);
    }   

    dev->src_hw_if_id = src_hw_if_id;

    //printk("%s, %d.\n", __FUNCTION__, __LINE__);

    return(0);
}






















