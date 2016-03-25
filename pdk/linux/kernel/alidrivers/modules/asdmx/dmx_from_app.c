#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/freezer.h>


#include <asm/io.h>

#if 1

MODULE_LICENSE("GPL");



#include "dmx_internal.h"

#define DMX_PLAYBACK_BUF_SIZE 48128 /* 256 * 188 */

#define DMX_PLAYBACK_WORKQUEUE_0_NAME "ali_dmx_pb_0_q"

#define DMX_PLAYBACK_IN_0_NAME "ali_dmx_pb_0_in"

#define DMX_PLAYBACK_OUT_0_NAME "ali_dmx_pb_0_out"




struct dmx_device g_dmx_playback_devices[1];




extern struct class *g_ali_m36_dmx_class;


struct file_operations g_ali_m36_dmx_playback_out_fops =
{
	.owner =    THIS_MODULE,
	//.llseek =   scull_llseek,
	.read =     dmx_channel_read,
	//.write =    dmx_write,
	.unlocked_ioctl =    dmx_ioctl,
	.open =     dmx_channel_open,
	.release =  dmx_channel_release,
    .poll = dmx_channel_poll,
};





struct file_operations g_ali_m36_dmx_playback_in_fops = 
{
	.owner =    THIS_MODULE,
	//.llseek =   scull_llseek,
	//.read =     dmx_channel_read,
	.write =    dmx_playback_buf_usr_wr,
	.unlocked_ioctl =    dmx_playback_ioctl,
	.open =     dmx_playback_in_open,
	.release =  dmx_playback_in_release,
    //.poll = dmx_channel_poll,
};











DMX_INT32 dmx_playback_in_release
(
    struct inode *inode,
    struct file  *file
)
{
    struct dmx_device *dmx;

    DMX_PLAYBACK_DEBUG("%s, go\n", __FUNCTION__);

    dmx = (struct dmx_device *)file->private_data;

    if (DMX_PLAYBACK_IN_IDLE == dmx->playback_in_status)
    {
        return(0);
    }

	if (mutex_lock_interruptible(&dmx->io_mutex))
    {
        DMX_PLAYBACK_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    }

    dmx_data_buf_kern_flush_all_pkt(dmx, &dmx->playback_in_buf);

    dmx->playback_in_status = DMX_PLAYBACK_IN_IDLE;

    mutex_unlock(&dmx->io_mutex);

    return(0);
}





DMX_INT32 dmx_playback_in_open
(
    struct inode *inode,
    struct file  *file
)
{
    DMX_INT32          ret;
    struct dmx_device *dmx;

    DMX_PLAYBACK_DEBUG("%s, go\n", __FUNCTION__);

    dmx = container_of(inode->i_cdev, struct dmx_device, playback_in_cdev);

    ret = 0;

    if (dmx->playback_in_status != DMX_PLAYBACK_IN_IDLE)
    {
		ret = (-EMFILE);
    }

	if (mutex_lock_interruptible(&dmx->io_mutex))
    {
        return(-ERESTARTSYS);
    }

    file->private_data = dmx;

    dmx->playback_in_status = DMX_PLAYBACK_IN_RUN;

    mutex_unlock(&dmx->io_mutex);

    return(ret);
}



ssize_t dmx_playback_buf_usr_wr
(
    struct file       *file, 
    const char __user *src,
	size_t             count,
    loff_t            *ppos
)
{
     __s32                    byte_cnt;
    struct dmx_device       *dmx;
    struct ali_dmx_data_buf *dest_buf;
    
    DMX_PLAYBACK_DEBUG("%s, %d, count:%d\n", __FUNCTION__, __LINE__, count);

    dmx = (struct dmx_device *)file->private_data;

    if (DMX_PLAYBACK_IN_IDLE == dmx->playback_in_status)
    {
        return(-EPERM);
    }

    dest_buf = &(dmx->playback_in_buf);
    
    byte_cnt = dmx_data_buf_usr_wr_data(dmx, file, dest_buf, src, count, 0);
    DMX_PLAYBACK_DEBUG("%s, %d, count:%d\n", __FUNCTION__, __LINE__, byte_cnt);
    return(byte_cnt);
}






void dmx_playback_parse_task
(
	struct work_struct *work
)
{
    __u32                     i;
    struct dmx_device        *dmx;
    struct dmx_ts_pkt_inf     ts_pkt_inf;
    struct ali_dmx_data_buf  *data_buf;
    __u32                     partial_ts_size;
   __u8                       partial_ts[188];
    struct ali_dmx_data_pkt  *data_pkt;
    struct ali_dmx_data_node *cur_data_node;
    struct ali_dmx_data_node *next_data_node;
    __s32 ret = 0;
    dmx = container_of((void *)work, struct dmx_device, work); 

    data_buf = &(dmx->playback_in_buf);

    partial_ts_size = 0;

    set_freezable();

    for (;;)
    {
        try_to_freeze();
        mutex_lock_interruptible(&dmx->io_mutex);
        if(dmx->playback_in_status != DMX_PLAYBACK_IN_RUN)
        {
//            printk("dmx playback Clear partial_ts_size\n");
            partial_ts_size = 0;
            if(dmx->playback_in_status == DMX_PLAYBACK_IN_FLUSH)
                dmx->playback_in_status = DMX_PLAYBACK_IN_RUN;
        }
        data_pkt = dmx_data_buf_kern_force_unlink_first_pkt(dmx, data_buf);
        mutex_unlock(&dmx->io_mutex);

        if (NULL != data_pkt)
        {
            list_for_each_entry_safe(cur_data_node, next_data_node,
                                     &(data_pkt->data_node), link)
            {
                if (partial_ts_size > 0)
                {
                    memcpy(&(partial_ts[partial_ts_size]),
                           (__u8 *)cur_data_node + cur_data_node->rd,
                           188 - partial_ts_size);
        
                    ts_pkt_inf.pkt_addr = &(partial_ts[0]);
                    if (mutex_lock_interruptible(&dmx->io_mutex))
                      continue;
                    dmx_ts_service_parse(dmx, &ts_pkt_inf);
                    mutex_unlock(&dmx->io_mutex);
                    cur_data_node->rd += (188 - partial_ts_size);
    
                    partial_ts_size = 0;
                }
    
                for (i = cur_data_node->rd; (i + 188) <= cur_data_node->wr;
                     i += 188)
                {
                    dmx->pkt_total_in_cnt++;

                    ts_pkt_inf.pkt_addr = (__u8 *)cur_data_node + i;

                    if (mutex_lock_interruptible(&dmx->io_mutex))
                      continue;
                    ret = dmx_ts_service_parse(dmx, &ts_pkt_inf);
                    mutex_unlock(&dmx->io_mutex);
                    
                    cur_data_node->rd +=188;
                }
        
                if (cur_data_node->rd < cur_data_node->wr)
                {
                    partial_ts_size = cur_data_node->wr - cur_data_node->rd;
        
                    memcpy(&(partial_ts[0]), ((__u8 *)cur_data_node) + cur_data_node->rd,
                           partial_ts_size);
                }

                /* Remove current data node from data node list. */
                list_del(&(cur_data_node->link));
            
                dmx_data_node_put(dmx, cur_data_node);
            }
        }

        msleep(5);
    }

    return;
}



void ali_dmx_playback_cleanup
(
    struct dmx_device *dmx,
    __u32              error_code
)
{
    DMX_PLAYBACK_DEBUG("%s\n", __FUNCTION__);

    /* Close playback in device. */
    if (0x1 != (error_code & 0x1))
    {
        cdev_del(&dmx->playback_in_cdev);
    }

    if (0x2 != (error_code & 0x2))
    {
        unregister_chrdev_region(dmx->playback_in_dev_id, 1);
    }

    /* Close playback out device. */
    if (0x4 != (error_code & 0x4))
    {
        cdev_del(&dmx->cdev);
    }

    if (0x8 != (error_code & 0x8))
    {
        unregister_chrdev_region(dmx->dev_id, 1);
    }

    /* Destroy workqueue. */
    if (0x10 != (error_code & 0x10))
    {
        flush_workqueue(dmx->workqueue);

    	destroy_workqueue(dmx->workqueue);
	}

    /* Release memory. */
    if (0x20 != (error_code & 0x20))
    {
        dmx_data_pool_release(dmx);
    }

    /* Distroy sysfs entry. */
    if (0x40 != (error_code & 0x40))
    {
        device_destroy(g_ali_m36_dmx_class, dmx->playback_in_dev_id);
    }

    if (0x80 != (error_code & 0x80))
    {
        device_destroy(g_ali_m36_dmx_class, dmx->dev_id);
    }

    return;
}




int dmx_playback_ioctl
(

    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
{
    struct dmx_device *dmx;
    int                ret;

    ret = 0;

    dmx = filp->private_data;

    switch(cmd)
    {
        case ALI_DMX_CHANNEL_FLUSH_BUF:
        {
            DMX_PLAYBACK_DEBUG("%s,%d ALI_DMX_CHANNEL_FLUSH_BUF\n", __FUNCTION__, __LINE__);

            if (mutex_lock_interruptible(&dmx->io_mutex))
            {
                return(-ERESTARTSYS);
            }
            dmx_data_buf_kern_flush_all_pkt(dmx, &dmx->playback_in_buf);
                
            if(dmx->playback_in_status == DMX_PLAYBACK_IN_RUN)
            {
                dmx->playback_in_status = DMX_PLAYBACK_IN_FLUSH;
            }
            mutex_unlock(&dmx->io_mutex);

            printk("%s,%d\n", __FUNCTION__, __LINE__);
            break;
        }
        case ALI_DMX_IO_SET_PLAYBACK_SRC_TYPE:
        {
            dmx->isRadio_playback = arg;
            printk("%s,%d\n", __FUNCTION__, __LINE__);
            break;
        }
        case ALI_DMX_PLAYBACK_FSTCPY:
        {

            __s32                    byte_cnt;
           struct ali_dmx_data_buf * dest_buf;
           struct dmx_fast_copy_param   param;
           if (DMX_PLAYBACK_IN_IDLE == dmx->playback_in_status)
           {
               return(-EPERM);
           }
           copy_from_user((void *)&param, arg, sizeof(struct dmx_fast_copy_param));
           
           dest_buf = &(dmx->playback_in_buf);
           dest_buf->fst_cpy_size = 0;
		   dest_buf->exp_cpy_size = param.len;
           byte_cnt = dmx_data_buf_usr_wr_data(dmx, filp, dest_buf, param.data, param.len, 1);
           ret = byte_cnt;
           break;

        }
      default:  
        return(-ENOTTY);
    }

    return(ret);    
}





static void __exit ali_dmx_playback_exit
(
    void
)
{
    struct dmx_device *dmx;

    dmx = &(g_dmx_playback_devices[0]);

    ali_dmx_playback_cleanup(dmx, 0xFFFFFFFF);

    return;
}






static __s32 __init ali_dmx_playback_init
(
    void
)
{
    DMX_INT32           result;
    struct dmx_device  *dmx;
    struct attribute  **attrs;

    result = 0;

    dmx = &(g_dmx_playback_devices[0]);

    memset(dmx, 0, sizeof(struct dmx_device));

    dmx->dmx_type = DMX_TYPE_SW;

    dmx->name[0] = 'D';

    dmx->name[1] = 'M';

    dmx->name[2] = 'X';

    dmx->name[3] = 'P';

    dmx->name[4] = 'L';

    dmx->name[5] = 'A';

    dmx->name[6] = 'Y';

    dmx->name[7] = 'B';

    dmx->name[8] = 'A';

    dmx->name[9] = 'C';

    dmx->name[10] = 'K';

    dmx->name[11] = '0';

    DMX_INIT_DEBUG("DMX name: %s\n", dmx->name);

    /* Init sw services. */
    dmx_ts_service_init(dmx);

    dmx_ts2pes_service_init(dmx);

    dmx_ts2sec_service_init(dmx);

    //dmx_pcr_service_init(dmx);

    dmx_channel_init(dmx);

    /* Step 6: Create Mutex. */
	mutex_init(&(dmx->io_mutex));

    /* Step 7: Init buffer structure. */
    dmx_data_buf_setup(dmx, &dmx->playback_in_buf, 0xBC00, 0x1000); /* 47K, 4K */

    /* Step 8: Create memory pool for data buffer. */
    dmx_data_pool_init(dmx, DMX_DATA_POOL_NODE_SIZE, DMX_DATA_POOL_NODE_CNT);


    /* Step 8: Init TS retrieving task. */
	dmx->workqueue = create_workqueue(DMX_PLAYBACK_WORKQUEUE_0_NAME);

	if (!(dmx->workqueue))
    {
		DMX_INIT_DEBUG("Failed to allocate work queue\n");

        ali_dmx_playback_cleanup(dmx, 0x20);

        return(-1);
	}

	INIT_WORK(&dmx->work, dmx_playback_parse_task);

	queue_work(dmx->workqueue, &dmx->work);

    /* Init playback in dev.*/
	result = alloc_chrdev_region(&dmx->playback_in_dev_id, 0, 1, 
                                 DMX_PLAYBACK_IN_0_NAME);

	if (result < 0) 
    {
        DMX_PLAYBACK_DEBUG("%s,%d.\n", __FUNCTION__, __LINE__);

        ali_dmx_playback_cleanup(dmx, 0x20 | 0x10);

        return(-1);
	}

    DMX_PLAYBACK_DEBUG("%s,%d.\n", __FUNCTION__, __LINE__);

    /* Create char device entry. */
	cdev_init(&(dmx->playback_in_cdev), &g_ali_m36_dmx_playback_in_fops);

	dmx->playback_in_cdev.owner = THIS_MODULE;

	result = cdev_add(&dmx->playback_in_cdev, dmx->playback_in_dev_id, 1);

	/* Fail gracefully if need be. */
	if (result)
    {
        DMX_PLAYBACK_DEBUG("%s,%d.\n", __FUNCTION__, __LINE__);

        ali_dmx_playback_cleanup(dmx, 0x20 | 0x10 | 0x2);

        return(-1);
    }

    /* Create entry for user space. 
     * Creates a device and registers it with sysfs.
     */
	dmx->playback_in_ddevice = device_create(g_ali_m36_dmx_class, NULL, 
                                             dmx->playback_in_dev_id, 
                                             dmx, DMX_PLAYBACK_IN_0_NAME);

	if (IS_ERR(dmx->playback_in_ddevice))
    {
		result = PTR_ERR(dmx->playback_in_ddevice);

        DMX_PLAYBACK_DEBUG("%s,%d,err:%d\n", __FUNCTION__, __LINE__, result);

        ali_dmx_playback_cleanup(dmx, 0x20 | 0x10 | 0x2 | 0x1);

        return(-1);
	}

    /* Init playback out dev.*/
	result = alloc_chrdev_region(&dmx->dev_id, 0, 1, DMX_PLAYBACK_OUT_0_NAME);

	if (result < 0) 
    {
        DMX_PLAYBACK_DEBUG("%s,%d.\n", __FUNCTION__, __LINE__);

        ali_dmx_playback_cleanup(dmx, 0x20 | 0x10 | 0x2 | 0x1 | 0x40);

        return(-1);
	}

    DMX_PLAYBACK_DEBUG("%s,%d.\n", __FUNCTION__, __LINE__);

	cdev_init(&(dmx->cdev), &g_ali_m36_dmx_playback_out_fops);

	dmx->cdev.owner = THIS_MODULE;

	result = cdev_add(&dmx->cdev, dmx->dev_id, 1);

	/* Fail gracefully if need be. */
	if (result)
    {
        DMX_PLAYBACK_DEBUG("%s,%d.\n", __FUNCTION__, __LINE__);

        ali_dmx_playback_cleanup(dmx, 0x20 | 0x10 | 0x2 | 0x1 | 0x40 | 0x4);

        return(-1);
    }

	dmx->device = device_create(g_ali_m36_dmx_class, NULL, dmx->dev_id, dmx, 
                                DMX_PLAYBACK_OUT_0_NAME);

	if (IS_ERR(dmx->device))
    {
		result = PTR_ERR(dmx->device);

        DMX_PLAYBACK_DEBUG("%s,%d,err:%d\n", __FUNCTION__, __LINE__, result);

        ali_dmx_playback_cleanup(dmx, 0x20 | 0x10 | 0x2 | 0x1 | 0x40 | 0x4 | 
                                 0x8);

        return(-1);
	}

    dmx->attr_ts_in_cnt.attr.mode = S_IRUGO;

    dmx->attr_ts_in_cnt.attr.name = "global";

    dmx->attr_ts_in_cnt.show = dmx_global_stat_show;

    dmx->attr_ts_in_cnt.store = NULL;


    dmx->attr_ts_services.attr.mode = S_IRUGO;

    dmx->attr_ts_services.attr.name = "ts_services";

    dmx->attr_ts_services.show = dmx_ts_services_show;

    dmx->attr_ts_services.store = NULL;


    attrs = kmalloc(GFP_KERNEL, sizeof(*attrs) * 12);

    if (NULL == attrs)
    {
        DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

        ali_dmx_playback_cleanup(dmx, 0x20 | 0x10 | 0x2 | 0x1 | 0x40 | 0x4 | 
                                 0x8);

        return(-1);
    }

    memset(attrs, 0, sizeof(*attrs) * 12);

    attrs[0] = &(dmx->attr_ts_in_cnt.attr);

    attrs[1] = &(dmx->attr_ts_services.attr);

    attrs[2] = NULL;

    result = dmx_sysfs_entry_create(dmx, attrs);

	if (result)
    {
		dev_warn(dmx->device, "create sysfs group err: %d\n", result);

        kfree(attrs);

        ali_dmx_playback_cleanup(dmx, 0x20 | 0x10 | 0x2 | 0x1 | 0x40 | 0x4 | 
                                 0x8);

        return(-1);
    }

    return(0);
}






//module_init(ali_dmx_see_init);
device_initcall_sync(ali_dmx_playback_init);
module_exit(ali_dmx_playback_exit);

#endif

















