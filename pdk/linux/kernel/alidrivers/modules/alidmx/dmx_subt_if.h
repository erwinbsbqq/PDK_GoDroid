#ifndef _ALI_DMX_SUBT_IF_H_
#define _ALI_DMX_SUBT_IF_H_



#define DMX_SUBT_DEVICE_CNT 8
#define DMX_SUBT_STREAM_CNT 1

enum DMX_SUBT_DVICE_STATE
{
    DMX_SUBT_DEVICE_STATE_IDLE = 0,
	DMX_SUBT_DEVICE_STATE_RUN
};

enum DMX_SUBT_STREAM_STATE
{
    DMX_SUBT_STREAM_STATE_IDLE = 0,
	DMX_SUBT_STREAM_STATE_CFG,
	DMX_SUBT_STREAM_STATE_STOP,
	DMX_SUBT_STREAM_STATE_RUN
};


struct dmx_subt_device
{
    enum DMX_SUBT_DVICE_STATE state;

    /* Linux kernel char dev id.
    */
    dev_t linux_dev_id;

    /* Linux kernel device.
    */
    struct device *linux_dev;

    /* Linux kernel char dev.
    */
    struct cdev linux_cdev;	

    /* Data source hw device id of this dmx interface.
    */
    __u32 src_hw_if_id;
};


struct dmx_subt_stream
{
    __s32 idx;
	
    enum DMX_SUBT_STREAM_STATE state;

	/* Point to subtitle device this stream attached to.
	*/
    struct dmx_subt_device *subt_device;

	/* Ts filter ID this stream fetching data from.
	*/
	__s32 ts_flt_id;

	/* Configuration parameters passed by user. 
	*/
	struct Ali_DmxSubtStreamParam usr_param;
};


__s32 dmx_subt_stream_open(struct inode *inode, struct file *file);
__s32 dmx_subt_stream_close(struct inode *inode, struct file *file);
long dmx_subt_stream_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
__s32 dmx_subt_stream_start(struct dmx_subt_device * dev, struct dmx_subt_stream * stream);
__s32 dmx_subt_stream_stop(struct dmx_subt_device * dev, struct dmx_subt_stream * stream);

#endif

