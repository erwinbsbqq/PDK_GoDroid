
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
#include <linux/version.h>
#include <linux/mm.h>

#include "dmx_stack.h"

struct dmx_linux_interface_module ali_dmx_linux_interface_module;


struct file_operations ali_dmx_output_fops =
{
    .owner   = THIS_MODULE,
    .read    = dmx_stream_read,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = dmx_stream_ioctl,
#else
    .ioctl   = dmx_stream_ioctl,
#endif   
    .open    = dmx_stream_open,
    .release = dmx_stream_close,
    .poll    = dmx_stream_poll,
};




struct file_operations ali_dmx_intput_fops =
{
    .owner   = THIS_MODULE,
    .write   = dmx_ts_in_ram_stream_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = dmx_ts_in_ram_stream_ioctl,
#else    
    .ioctl   = dmx_ts_in_ram_stream_ioctl,
#endif    
    .open    = dmx_ts_in_ram_stream_open,
    .release = dmx_ts_in_ram_stream_close,
    .poll    = dmx_ts_in_ram_stream_poll,
};



__s32 dmx_linux_output_interface_init
(
    __u32  src_hw_interface_id,
    __u8  *interface_name
)
{
    __s32                     ret;
    struct dmx_output_device *output_dev;
    struct class             *output_class;

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    /* Step 1: create dev file in /dev.
    */
    output_class = ali_dmx_linux_interface_module.output_device_class;
    
    if (NULL == output_class)
    {
        output_class = class_create(THIS_MODULE, "ali_dmx_output_class");
        
        if (IS_ERR(output_class))
        {       
            return(PTR_ERR(output_class));
        }

        ali_dmx_linux_interface_module.output_device_class = output_class;
    }

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    if (src_hw_interface_id > DMX_LINUX_OUTPUT_DEV_CNT)
    {
        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    output_dev = &(ali_dmx_linux_interface_module.output_device[src_hw_interface_id]);
    
    memset(output_dev, 0, sizeof(struct dmx_output_device));

    ret = alloc_chrdev_region(&output_dev->linux_dev_id, 0, 1, interface_name);

    if (ret < 0) 
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    cdev_init(&(output_dev->cdev), &ali_dmx_output_fops);

    output_dev->cdev.owner = THIS_MODULE;

    ret = cdev_add(&output_dev->cdev, output_dev->linux_dev_id, 1);

    /* Fail gracefully if need be.
     */
    if (ret)
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    output_dev->device = device_create(output_class, NULL, output_dev->linux_dev_id,
                                       output_dev, interface_name);

    if (IS_ERR(output_dev->device))
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);

        return(PTR_ERR(output_dev->device));
    }   

    output_dev->src_hw_interface_id = src_hw_interface_id;

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    return(0);
}




__s32 dmx_linux_input_interface_init
(
    __u32  dest_dev_id,
    __u8  *interface_name
)
{
    __s32                    ret;
    struct dmx_input_device *input_dev;
    struct class            *input_class;

    DMX_INIT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    /* Step 1: create dev file in /dev.
    */
    input_class = ali_dmx_linux_interface_module.input_device_class;
    
    if (NULL == input_class)
    {
        input_class = class_create(THIS_MODULE, "ali_dmx_input_class");
        
        if (IS_ERR(input_class))
        {                
            return(PTR_ERR(input_class));
        }

        ali_dmx_linux_interface_module.input_device_class = input_class;
    }

    if (dest_dev_id > DMX_LINUX_INPUT_DEV_CNT)
    {
        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    input_dev = &(ali_dmx_linux_interface_module.input_device[dest_dev_id]);
    
    memset(input_dev, 0, sizeof(struct dmx_input_device));

    ret = alloc_chrdev_region(&input_dev->linux_dev_id, 0, 1, interface_name);

    if (ret < 0) 
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    cdev_init(&(input_dev->cdev), &ali_dmx_intput_fops);

    input_dev->cdev.owner = THIS_MODULE;

    ret = cdev_add(&input_dev->cdev, input_dev->linux_dev_id, 1);

    /* Fail gracefully if need be.
     */
    if (ret)
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    input_dev->device = device_create(input_class, NULL, input_dev->linux_dev_id,
                                      input_dev, interface_name);

    if (IS_ERR(input_dev->device))
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);

        return(PTR_ERR(input_dev->device));
    }   

    input_dev->dest_dev_id = dest_dev_id;

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    return(0);
}





