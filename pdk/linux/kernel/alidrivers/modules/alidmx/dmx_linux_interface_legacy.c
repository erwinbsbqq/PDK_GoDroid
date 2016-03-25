
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

#include "dmx_stack.h"

struct dmx_linux_interface_module_legacy ali_dmx_linux_interface_module_legacy;


struct file_operations ali_dmx_output_fops_legacy =
{
    .owner   = THIS_MODULE,
    .read    = dmx_channel_read,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = dmx_channel_ioctl,
#else
    .ioctl   = dmx_channel_ioctl,
#endif    
    .open    = dmx_channel_open,
    .release = dmx_channel_close,
    .poll    = dmx_channel_poll,
};

struct file_operations ali_dmx_input_fops_legacy =
{
    .owner   = THIS_MODULE,
    .write =    dmx_instream_ram_legacy_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = dmx_instream_ram_legacy_ioctl,
#else    
    .ioctl =    dmx_instream_ram_legacy_ioctl,
#endif    
    .open =     dmx_instream_ram_legacy_open,
    .release =  dmx_instream_ram_legacy_close,
};




__s32 dmx_linux_output_interface_legacy_init
(
    __u32  src_hw_interface_id,
    __u8  *interface_name,
    __u32  interface_id
)
{
    __s32                     ret;
    struct dmx_output_device *output_dev;
    struct class             *output_class;

    /* Step 1: create dev file in /dev.
    */
    output_class = ali_dmx_linux_interface_module_legacy.output_device_class;
    
    if (NULL == output_class)
    {
        output_class = class_create(THIS_MODULE, "ali_dmx_output_class_legacy");
        
        if (IS_ERR(output_class))
        {                
            return(PTR_ERR(output_class));
        }

        ali_dmx_linux_interface_module_legacy.output_device_class = output_class;
    }

    if (interface_id > DMX_LINUX_OUTPUT_DEV_CNT)
    {
        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    output_dev = &(ali_dmx_linux_interface_module_legacy.output_device[interface_id]);
    
    memset(output_dev, 0, sizeof(struct dmx_output_device));

    ret = alloc_chrdev_region(&output_dev->linux_dev_id, 0, 1, interface_name);

    if (ret < 0) 
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    cdev_init(&(output_dev->cdev), &ali_dmx_output_fops_legacy);

    output_dev->cdev.owner = THIS_MODULE;

    ret = cdev_add(&output_dev->cdev, output_dev->linux_dev_id, 1);

    /* Fail gracefully if need be.
     */
    if (ret)
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

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




__s32 dmx_linux_input_interface_legacy_init
(
    __u32  dest_dev_id,
    __u8  *interface_name,
    __u32  interface_id
)
{
    __s32                    ret;
    struct dmx_input_device *input_dev;
    struct class            *input_class;

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    /* Step 1: create dev file in /dev.
    */
    input_class = ali_dmx_linux_interface_module_legacy.input_device_class;
    
    if (NULL == input_class)
    {
        input_class = class_create(THIS_MODULE, "ali_dmx_input_class_legacy");
        
        if (IS_ERR(input_class))
        {                
            return(PTR_ERR(input_class));
        }

        ali_dmx_linux_interface_module_legacy.input_device_class = input_class;
    }

    if (interface_id > DMX_LINUX_INPUT_DEV_CNT_LEGACY)
    {
        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    input_dev = &(ali_dmx_linux_interface_module_legacy.input_device[interface_id]);
    
    memset(input_dev, 0, sizeof(struct dmx_input_device));

    ret = alloc_chrdev_region(&input_dev->linux_dev_id, 0, 1, interface_name);

    if (ret < 0) 
    {
        panic("%s, %d\n", __FUNCTION__, __LINE__);

        return(DMX_ERR_LINUX_INTERFACE_INIT_FAIL);
    }

    DMX_INIT_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    cdev_init(&(input_dev->cdev), &ali_dmx_input_fops_legacy);

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




__s32 dmx_linux_interface_module_legacy_init
(
    /* Internal DMX id to be attached to.
     *(each internal DMX has an input interface and an output interface and
     * a data engine and a hw interface, they are binded by src_hw_interface_id).
     */
    __u32                          src_hw_interface_id,
    
    /* Name.
    */
    __u8                          *interface_name,

    /* Input 
    */
    enum DMX_LINUX_INTERFACE_TYPE  interface_type,
    /* Idx in specified interface_type.
    */
    __u32                          interface_id 
)
{
    __s32 ret;
    
    switch(interface_type)
    {
        case DMX_LINUX_INTERFACE_TYPE_OUTPUT:
        {
            ret = dmx_linux_output_interface_legacy_init(src_hw_interface_id, 
                                                         interface_name,
                                                         interface_id);
        }
        break;

        case DMX_LINUX_INTERFACE_TYPE_INPUT:
        {
            ret = dmx_linux_input_interface_legacy_init(src_hw_interface_id,  
                                                        interface_name,
                                                        interface_id);
        }
        break;

        default:
        {
            ret = DMX_ERR_LINUX_INTERFACE_TYPE_ILLEGAL;
            
            panic("%s,%d\n", __FUNCTION__, __LINE__);
        }
        break;
    }

    return(ret);
}




