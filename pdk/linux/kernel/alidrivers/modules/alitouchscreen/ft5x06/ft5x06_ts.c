/* drivers/input/touchscreen/ft5x06_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/input.h>
#include <ft5x06_ts.h>
//#include <linux/earlysuspend.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>


#define DEBUG    1
#include <linux/device.h>	
//#define FTS_CTL_IIC
#define FTS_APK_DEBUG
#define SYSFS_DEBUG
#ifdef FTS_CTL_IIC
#include "focaltech_ctl.h"
#endif
#ifdef SYSFS_DEBUG
#include "ft5x06_ex_fun.h"
#endif
struct ts_event {
	u16 au16_x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];	/*touch event:
					0 -- down; 1-- contact; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure;
	u8 touch_point;
};

struct ft5x0x_ts_data {
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	struct ft5x0x_platform_data *pdata;
#ifdef CONFIG_PM
	struct early_suspend *early_suspend;
#endif
    struct hrtimer timer;
    struct work_struct work;
};

#define FTS_POINT_UP		0x01
#define FTS_POINT_DOWN		0x00
#define FTS_POINT_CONTACT	0x02

static struct workqueue_struct *ft5x06_wq;

extern int ali_i2c_read(u32 id, u8 slv_addr, u8 *data, int len);
extern int ali_i2c_write(u32 id, u8 slv_addr, u8 *data, int len);
extern int ali_i2c_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);

/*******************************************************
Description:
	Read data from the i2c slave device;
	This operation consisted of 2 i2c_msgs,the first msg used
	to write the operate address,the second msg used to read data.

Parameter:
	client:	i2c device.
	buf[0]:operate address.
	buf[1]~buf[len]:read data buffer.
	len:operate length.

return:
	numbers of i2c_msgs to transfer
*********************************************************/
static int i2c_read_bytes(struct i2c_client *client, uint8_t *buf, int len)
{
	uint8_t data[16] = {0};
	unsigned int rd = 0;
	int ret = 0;	

    static int first = 1;
    if(client) {
        if(first){
            printk("1, %s,%d:addr=0x%x,len=%d,buf[0]=0x%x\n",__FUNCTION__,__LINE__,client->addr,len,buf[0]);
            first = 0;
        }        
    }
    else{
        printk("%s,%d:addr=0x%x,len=%d,client=null!\n",__FUNCTION__,__LINE__,client->addr,len);
        return -1;
    }

   client->addr = 0x70;

    if(len <= 2){
        return 0;
    }
    
    len -= 2;    
    while((rd+14)<len)
	{
	    memset(data, 0, sizeof(data));
	    
		ret += ali_i2c_read(1, 0x71, data, 14);
		memcpy(&buf[rd+2], data, 14);
		rd+=14;		
		if(0 != ret)
		{
			return ret;
		}	
	}

	memset(data, 0, sizeof(data));
	
	ret += ali_i2c_read(1, 0x71, data, len-rd);
	memcpy(&buf[rd+2], data, len - rd);
	
	return ret;
}

/*******************************************************
Description:
	write data to the i2c slave device.

Parameter:
	client:	i2c device.
	buf[0]:operate address.
	buf[1]~buf[len]:write data buffer.
	len:operate length.

return:
	numbers of i2c_msgs to transfer.
*********************************************************/
static int i2c_write_bytes(struct i2c_client *client,uint8_t *buf,int len)
{
	uint8_t data[16] = {0};
	unsigned int wt = 0;
	int ret = 0;

    static int first = 1;
    
    if(client) {
        if(first){
            printk("1, %s,%d:addr=0x%x,len=%d\n",__FUNCTION__,__LINE__,client->addr,len);
            first = 0;
        }
    }
    else{
        printk("%s,%d:addr=0x%x,len=%d,client = null!\n",__FUNCTION__,__LINE__,client->addr,len);
        return -1;
    }

    //0xba/0xbb 0x6e/0x6f 0x28/0x29 ?
    client->addr = 0x70;

    if(len <= 2 && len > 0){
        ret = ali_i2c_write(1, client->addr, buf, len);
        return ret;
    }
    
    len -= 2;
    data[0] = buf[0];
	while((wt+14)<len) {
	    data[1] = buf[1] + wt;
	    memcpy(&data[2], &buf[wt + 2], 14);
		ret = ali_i2c_write(1, client->addr, data, 16);
		wt+=14;		
		if(0 != ret)
        {            
			return ret;
        }    
	}

	data[1] = buf[1] + wt;
    memcpy(&data[2], &buf[wt + 2], len-wt);

    ret = ali_i2c_write(1, client->addr, data, len-wt+2);
    
	return ret;
}

/*
*ft5x0x_i2c_Read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*
*
*/
int ft5x0x_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;

       //printk("ft5x0x_i2c_Read, 3, writelen: %d, readlen: %d\n", writelen, readlen);

	if (writelen > 0) {
		ret = i2c_write_bytes(client, writebuf, writelen);
              if(ret != 0)
              {
                    printk("ft5x0x_i2c_Read, i2c write error, ret=%d\n", ret);
                    return ret;
              }
              mdelay(10);
              ret = i2c_read_bytes(client, readbuf, readlen);
              if(ret != 0)
              {
                    printk("ft5x0x_i2c_Read, i2c read error after write, ret=%d\n", ret);
                    return ret;
              }
	} else {
		ret = i2c_read_bytes(client, readbuf, readlen);
              if(ret != 0)
              {
                    printk("ft5x0x_i2c_Read, i2c read error, ret=%d\n", ret);
                    return ret;
              }
	}
    
	return ret;
}

/*write data by i2c*/
int ft5x0x_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	ret = i2c_write_bytes(client, writebuf, writelen);
        if(ret != 0)
       {
                    printk("ft5x0x_i2c_Write, i2c write error, ret=%d\n", ret);
                    return ret;
       }

	return ret;
}

/*release the point*/
static void ft5x0x_ts_release(struct ft5x0x_ts_data *data)
{
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_sync(data->input_dev);
}

u8 s_buf[POINT_READ_BUF];
int repeat_cnt = 0;

static bool compare_buf(u8 *buf) 
{
    int i = 0;
    for(i = 0; i < POINT_READ_BUF; i++) {

        if(buf[i] != s_buf[i])
            break;

    }


    if(i < POINT_READ_BUF)
        return false;

    
    return true;
}

/*Read touch point information when the interrupt  is asserted.*/
static int ft5x0x_read_Touchdata(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	int ret = -1;
	int i = 0;
	u8 pointid = FT_MAX_ID;

	ret = ft5x0x_i2c_Read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}

        
        bool c_ret = compare_buf(buf);

        if(c_ret == true) {
            return -1;
        }
        
        memcpy(s_buf, buf, POINT_READ_BUF);

        printk("\n\n");

        for(i = 0; i < POINT_READ_BUF; i++) {

            printk("%02x  ", buf[i]);

        }

        printk("\n\n");
    
	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
			event->touch_point++;
		event->au16_x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->au16_y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];

             if(event->au16_y[i] < data->y_max)
                event->au16_y[i] = data->y_max - event->au16_y[i];

        
		event->au8_touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->au8_finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
	}

	event->pressure = FT_PRESS;

	return 0;
}

/*
*report the point information
*/
static void ft5x0x_report_value(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	int i = 0;
	int up_point = 0;
	//int touch_point = 0;

        printk("ft5x0x_report_value, touch point: %d, x max: %d, y max: %d\n", event->touch_point, data->x_max, data->y_max);

	for (i = 0; i < event->touch_point; i++) {

            printk("i: %d, x: %d, y: %d\n", i, event->au16_x[i], event->au16_y[i]);
     
		/* LCD view area */
		if (event->au16_x[i] < data->x_max
		    && event->au16_y[i] < data->y_max) {

			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					 event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					 event->au16_y[i]);
			input_report_abs(data->input_dev, ABS_MT_PRESSURE,
					 event->pressure);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID,
					 event->au8_finger_id[i]);

            
			if (event->au8_touch_event[i] == FTS_POINT_DOWN || event->au8_touch_event[i] == FTS_POINT_CONTACT){
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR,
						 event->pressure);
                
                
                     input_report_abs(data->input_dev,ABS_MT_PRESSURE,event->pressure);//its only need for android 4.0
			input_report_key(data->input_dev,BTN_TOUCH,1);//its only need for android 4.0
		}		
		else 
		{
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR, 0);
                                 input_report_abs(data->input_dev,ABS_MT_PRESSURE,0);//its only need for android 4.0  ABS_MT_TOUCH_PRESSURE-->ABS_PRESSURE
				input_report_key(data->input_dev,BTN_TOUCH,0);//its only need for android 4.0
                                //printk("event->au8_touch_event[i] !=0\n");
				//up_point++;
		}
			//touch_point ++;
	}

		input_mt_sync(data->input_dev);
	}
	input_sync(data->input_dev);

	if (event->touch_point == 0)
		ft5x0x_ts_release(data);

}


/*The ft5x0x device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
	struct ft5x0x_ts_data *ft5x0x_ts = dev_id;
	int ret = 0;
	//disable_irq_nosync(irq_no);
	printk("ft5x0x_ts_interrupt [irq] = %d\n", irq);//20130320
	ret = ft5x0x_read_Touchdata(ft5x0x_ts);
	if (ret == 0)
		ft5x0x_report_value(ft5x0x_ts);

	//enable_irq(irq_no);

	return IRQ_HANDLED;
}

static void ft5x06_ts_work_func(struct work_struct * work)
{
    struct ft5x0x_ts_data *data = container_of(work, struct ft5x0x_ts_data, work);
    int ret = 0;
    //disable_irq_nosync(irq_no);
    //printk("ft5x0x_ts_interrupt [irq] = %d\n", irq);//20130320
#if 0
    ret = ft5x0x_read_Touchdata(ft5x0x_ts);
    if (ret == 0)
    {
        //printk("%s, %d\n",  __FUNCTION__, __LINE__);
        ft5x0x_report_value(ft5x0x_ts);
    }
    //enable_irq(irq_no);

    //return IRQ_HANDLED;
#else

    //struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	
	int i = 0;
	u8 pointid = FT_MAX_ID;

      unsigned int X, Y;
	unsigned int x, y, event, id;

	ret = ft5x0x_i2c_Read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return;
	}

        
        bool c_ret = compare_buf(buf);

        if(c_ret == true) {
            return;
        }
        
        memcpy(s_buf, buf, POINT_READ_BUF);

        printk("\n\n");

        for(i = 0; i < POINT_READ_BUF; i++) {

            printk("%02x  ", buf[i]);

        }
  
        printk("\n\n");

         for(i = 0; i < CFG_MAX_TOUCH_POINTS; i++)
	{
		X = (buf[i*6 + 5])<<8 | buf[i*6 + 6];
		Y = (buf[i*6 + 7])<<8 | buf[i*6 + 8];

		x = X & 0xfff;
		y = Y & 0xfff;

                //x = x*720/800;   //   800 --> 720 for vpo display issue

              if(x > data->x_max || y > data->y_max)
                continue;
        
              if(y < data->y_max)
		    y = data->y_max - y;
	
		event = (X >> 14) & 0x3;
		id = (Y >> 12) & 0xf;

              printk("x: %d, y: %d, event: %d, id: %d\n", x, y, event, id);

		if(id >= 0 && id <= CFG_MAX_TOUCH_POINTS-1)
		{
			data->event.au16_x[id] = x;
			data->event.au16_y[id] = y;

			if((event == 0) || (event == 0x02))
			{
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0x7F);
				input_report_abs(data->input_dev, ABS_MT_POSITION_X, x);
				input_report_abs(data->input_dev, ABS_MT_POSITION_Y, y);
				input_report_abs(data->input_dev, ABS_MT_PRESSURE, 0x7F);
				input_report_key(data->input_dev, BTN_TOUCH, 1);
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, id);
				input_mt_sync(data->input_dev);

				// printk("[%d]down: x = %4d, y = %4d\n", id, ts->node[id].x, ts->node[id].y);
			}
			else if(event == 0x01)
			{
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(data->input_dev, ABS_MT_POSITION_X, x);
				input_report_abs(data->input_dev, ABS_MT_POSITION_Y, y);
				input_report_abs(data->input_dev, ABS_MT_PRESSURE, 0);
				input_report_key(data->input_dev, BTN_TOUCH, 0);
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, id);
				input_mt_sync(data->input_dev);

				// printk("[%d]up: x = %4d, y = %4d\n", id, ts->node[id].x, ts->node[id].y);
			}
		}
	}

	input_sync(data->input_dev);
    

#endif
}

/*******************************************************
Description:
	Timer interrupt service routine.

Parameter:
	timer:	timer struct pointer.

return:
	Timer work mode. HRTIMER_NORESTART---not restart mode
*******************************************************/
static enum hrtimer_restart ft5x0x_ts_timer_func(struct hrtimer *timer)
{
	struct ft5x0x_ts_data *ts = container_of(timer, struct ft5x0x_ts_data, timer);
	queue_work(ft5x06_wq, &ts->work);
	hrtimer_start(&ts->timer, ktime_set(0, (POLL_TIME+6)*1000000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static int ft5x0x_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct ft5x0x_platform_data *pdata =
	    (struct ft5x0x_platform_data *)client->dev.platform_data;
	struct ft5x0x_ts_data *ft5x0x_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char uc_reg_value;
	unsigned char uc_reg_addr;
 unsigned int irq_no;

    int i = 0;
    for(i = 0; i < POINT_READ_BUF; i++) {
        s_buf[i] = 0;
    }
 
#if 0
 int i;
 for(i =0; i < 255; i++) {

    int ret = ali_i2c_write(1, i, &uc_reg_value, 1);
    if(ret == 0)
    {
        printk("ft5x0x_ts_probe, i: 0x%02x\n", i);
    }
    else
        printk("error, i: %d\n", i);

    ret = ali_i2c_read(1, i, &uc_reg_value, 1);
    if(ret == 0)
    {
        printk("ft5x0x_ts_probe, read success i: 0x%02x\n", i);
    }
    else
        printk("read error, i: %d\n", i);
 }

 return 0;
#endif
     //add by zhangq
        	/*s3c_gpio_cfgpin(S5PV210_GPH1(6), S3C_GPIO_SFN(0xf));
		s3c_gpio_setpull(S5PV210_GPH1(6), S3C_GPIO_PULL_UP);
		*/
              irq_no = 72+(11);
		printk("irq_no = %d\n", irq_no);
              enable_irq(irq_no);


	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ft5x0x_ts = kzalloc(sizeof(struct ft5x0x_ts_data), GFP_KERNEL);

	if (!ft5x0x_ts) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

    INIT_WORK(&ft5x0x_ts->work, ft5x06_ts_work_func);
	
	i2c_set_clientdata(client, ft5x0x_ts);
	//ft5x0x_ts->irq = client->irq;
        ft5x0x_ts->irq = irq_no;	
	ft5x0x_ts->client = client;
	ft5x0x_ts->pdata = pdata;
	ft5x0x_ts->x_max = pdata->x_max - 1;
	ft5x0x_ts->y_max = pdata->y_max - 1;
///#ifdef CONFIG_PM
//	err = gpio_request(pdata->reset, "ft5x0x reset");
//	if (err < 0) {
//		dev_err(&client->dev, "%s:failed to set gpio reset.\n",
//			__func__);
//		goto exit_request_reset;
//	}
//#endif
	
	ft5x06_wq = create_singlethread_workqueue("ft5x06_wq");		//create a work queue and worker thread
	if (!ft5x06_wq) {
		printk(KERN_ALERT "creat workqueue failed\n");
		goto err_ft5x06_is_not_exist;
	}
      

	/*err = request_threaded_irq(irq_no, NULL, ft5x0x_ts_interrupt,
				   IRQF_TRIGGER_FALLING, client->dev.driver->name,
				   ft5x0x_ts);
	*/

    #if 0
	 err = request_irq(irq_no, (irq_handler_t)ft5x0x_ts_interrupt, 0, client->dev.driver->name, NULL);  
	if (err < 0) {
		dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}
	disable_irq(irq_no);
#endif

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ft5x0x_ts->input_dev = input_dev;

	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_PRESSURE, input_dev->absbit);

	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_X, 0, ft5x0x_ts->x_max, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_Y, 0, ft5x0x_ts->y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_TRACKING_ID, 0, CFG_MAX_TOUCH_POINTS, 0, 0);

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);

	input_dev->name = FT5X0X_NAME;
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
			"ft5x0x_ts_probe: failed to register input device: %s\n",
			dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}
	/*make sure CTP already finish startup process */
	msleep(150);

	/*get some register information */
	uc_reg_addr = FT5x0x_REG_FW_VER;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] Firmware version = 0x%x\n", uc_reg_value);
       printk("[FTS] Firmware version = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT5x0x_REG_POINT_RATE;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] report rate is %dHz.\n",
		uc_reg_value * 10);
       printk("[FTS] report rate is %dHz.\n",
		uc_reg_value * 10);

	uc_reg_addr = FT5X0X_REG_THGROUP;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] touch threshold is %d.\n",
		uc_reg_value * 4);

       printk("[FTS] touch threshold is %d.\n",
		uc_reg_value * 4);
    
#ifdef SYSFS_DEBUG
	ft5x0x_create_sysfs(client);
#endif
#ifdef FTS_APK_DEBUG
	ft5x0x_create_apk_debug_channel(client);
#endif

#ifdef FTS_CTL_IIC
	if (ft_rw_iic_drv_init(client) < 0)
		dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",
				__func__);
#endif

        hrtimer_init(&ft5x0x_ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	 ft5x0x_ts->timer.function = ft5x0x_ts_timer_func;
	 hrtimer_start(&ft5x0x_ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	printk("a1111111111111111111dd succssss\n");
	//enable_irq(irq_no);
	return 0;

exit_input_register_device_failed:
	input_free_device(input_dev);

exit_input_dev_alloc_failed:
	free_irq(client->irq, ft5x0x_ts);
#ifdef CONFIG_PM
exit_request_reset:
	gpio_free(ft5x0x_ts->pdata->reset);
#endif

err_ft5x06_is_not_exist:
exit_irq_request_failed:
	i2c_set_clientdata(client, NULL);
	kfree(ft5x0x_ts);

exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

#ifdef CONFIG_PM
static void ft5x0x_ts_suspend(struct early_suspend *handler)
{
	struct ft5x0x_ts_data *ts = container_of(handler, struct ft5x0x_ts_data,
						early_suspend);

	dev_dbg(&ts->client->dev, "[FTS]ft5x0x suspend\n");
        printk("ft5x0x_ts_suspend\n");
	//disable_irq(ts->pdata->irq);
}

static void ft5x0x_ts_resume(struct early_suspend *handler)
{
	struct ft5x0x_ts_data *ts = container_of(handler, struct ft5x0x_ts_data,
						early_suspend);

	dev_dbg(&ts->client->dev, "[FTS]ft5x0x resume.\n");
	gpio_set_value(ts->pdata->reset, 0);
	msleep(20);
	gpio_set_value(ts->pdata->reset, 1);

    printk("ft5x0x_ts_resume\n");
	//enable_irq(ts->pdata->irq);
}
#else
#define ft5x0x_ts_suspend	NULL
#define ft5x0x_ts_resume		NULL
#endif

static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	ft5x0x_ts = i2c_get_clientdata(client);
	input_unregister_device(ft5x0x_ts->input_dev);
	#ifdef CONFIG_PM
	gpio_free(ft5x0x_ts->pdata->reset);
	#endif
	#ifdef FTS_APK_DEBUG
	ft5x0x_release_apk_debug_channel();
	#endif
	#ifdef SYSFS_DEBUG
	ft5x0x_release_sysfs(client);
	#endif
	#ifdef FTS_CTL_IIC
	ft_rw_iic_drv_exit();
	#endif
	free_irq(client->irq, ft5x0x_ts);
	kfree(ft5x0x_ts);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{FT5X0X_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver ft5x0x_ts_driver = {
	.probe = ft5x0x_ts_probe,
	.remove = __devexit_p(ft5x0x_ts_remove),
	.id_table = ft5x0x_ts_id,
	.suspend = ft5x0x_ts_suspend,
	.resume = ft5x0x_ts_resume,
	.driver = {
		   .name = FT5X0X_NAME,
		   .owner = THIS_MODULE,
		   },
};

static int __init ft5x0x_ts_init(void)
{
	int ret;
        printk("ft5x0x_ts_init: suspend: 0x%08x\n", ft5x0x_ts_driver.suspend);
	ret = i2c_add_driver(&ft5x0x_ts_driver);
	if (ret) {
		printk(KERN_WARNING "Adding ft5x0x driver failed "
		       "(errno = %d)\n", ret);
	} else {
		pr_info("Successfully added driver %s\n",
			ft5x0x_ts_driver.driver.name);
	}
	return ret;
}

static void __exit ft5x0x_ts_exit(void)
{
	i2c_del_driver(&ft5x0x_ts_driver);
}

//ft5x06_ex_fun.c
/*
 *drivers/input/touchscreen/ft5x06_ex_fun.c
 *
 *FocalTech ft5x0x expand function for debug.
 *
 *Copyright (c) 2010  Focal tech Ltd.
 *
 *This software is licensed under the terms of the GNU General Public
 *License version 2, as published by the Free Software Foundation, and
 *may be copied, distributed, and modified under those terms.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *Note:the error code of EIO is the general error in this file.
 */


#include "ft5x06_ex_fun.h"
//#include <linuxft5x06_ts.h>
#include <linux/mount.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>

struct Upgrade_Info {
	u16 delay_aa;		/*delay of write FT_UPGRADE_AA */
	u16 delay_55;		/*delay of write FT_UPGRADE_55 */
	u8 upgrade_id_1;	/*upgrade id 1 */
	u8 upgrade_id_2;	/*upgrade id 2 */
	u16 delay_readid;	/*delay of read id */
	u16 delay_earse_flash; /*delay of earse flash*/
};


int fts_ctpm_fw_upgrade(struct i2c_client *client, u8 *pbt_buf,
			  u32 dw_lenth);

static unsigned char CTPM_FW[] = {
//	#include "ft_app.i"
	#include "Ft5x06_Lib.i"
};

static struct mutex g_device_mutex;

int ft5x0x_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue)
{
	unsigned char buf[2] = {0};
	buf[0] = regaddr;
	buf[1] = regvalue;

	return ft5x0x_i2c_Write(client, buf, sizeof(buf));
}


int ft5x0x_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue)
{
	return ft5x0x_i2c_Read(client, &regaddr, 1, regvalue, 1);
}


int fts_ctpm_auto_clb(struct i2c_client *client)
{
	unsigned char uc_temp = 0x00;
	unsigned char i = 0;

	/*start auto CLB */
	msleep(200);

	ft5x0x_write_reg(client, 0, FTS_FACTORYMODE_VALUE);
	/*make sure already enter factory mode */
	msleep(100);
	/*write command to start calibration */
	ft5x0x_write_reg(client, 2, 0x4);
	msleep(300);
	for (i = 0; i < 100; i++) {
		ft5x0x_read_reg(client, 0, &uc_temp);
		/*return to normal mode, calibration finish */
		if (0x0 == ((uc_temp & 0x70) >> 4))
			break;
	}

	//msleep(200);
	/*calibration OK */
	msleep(300);
	ft5x0x_write_reg(client, 0, FTS_FACTORYMODE_VALUE);	/*goto factory mode for store */
	msleep(100);	/*make sure already enter factory mode */
	ft5x0x_write_reg(client, 2, 0x5);	/*store CLB result */
	msleep(300);
	ft5x0x_write_reg(client, 0, FTS_WORKMODE_VALUE);	/*return to normal mode */
	msleep(300);

	/*store CLB result OK */
	return 0;
}

/*
upgrade with *.i file
*/
int fts_ctpm_fw_upgrade_with_i_file(struct i2c_client *client)
{
	u8 *pbt_buf = NULL;
	int i_ret;
	int fw_len = sizeof(CTPM_FW);

	/*judge the fw that will be upgraded
	* if illegal, then stop upgrade and return.
	*/
	if (fw_len < 8 || fw_len > 32 * 1024) {
		dev_err(&client->dev, "%s:FW length error\n", __func__);
		return -EIO;
	}

	if ((CTPM_FW[fw_len - 8] ^ CTPM_FW[fw_len - 6]) == 0xFF
		&& (CTPM_FW[fw_len - 7] ^ CTPM_FW[fw_len - 5]) == 0xFF
		&& (CTPM_FW[fw_len - 3] ^ CTPM_FW[fw_len - 4]) == 0xFF) {
		/*FW upgrade */
		pbt_buf = CTPM_FW;
		/*call the upgrade function */
		i_ret = fts_ctpm_fw_upgrade(client, pbt_buf, sizeof(CTPM_FW));
		if (i_ret != 0)
			dev_err(&client->dev, "%s:upgrade failed. err.\n",
					__func__);
#ifdef AUTO_CLB
		else
			fts_ctpm_auto_clb(client);	/*start auto CLB */
#endif
	} else {
		dev_err(&client->dev, "%s:FW format error\n", __func__);
		return -EBADFD;
	}

	return i_ret;
}

u8 fts_ctpm_get_i_file_ver(void)
{
	u16 ui_sz;
	ui_sz = sizeof(CTPM_FW);
	if (ui_sz > 2)
		return CTPM_FW[ui_sz - 2];

	return 0x00;	/*default value */
}

/*update project setting
*only update these settings for COB project, or for some special case
*/
int fts_ctpm_update_project_setting(struct i2c_client *client)
{
	u8 uc_i2c_addr;	/*I2C slave address (7 bit address)*/
	u8 uc_io_voltage;	/*IO Voltage 0---3.3v;	1----1.8v*/
	u8 uc_panel_factory_id;	/*TP panel factory ID*/
	u8 buf[FTS_SETTING_BUF_LEN];
	u8 reg_val[2] = {0};
	u8 auc_i2c_write_buf[10] = {0};
	u8 packet_buf[FTS_SETTING_BUF_LEN + 6];
	u32 i = 0;
	int i_ret;

	uc_i2c_addr = client->addr;
	uc_io_voltage = 0x0;
	uc_panel_factory_id = 0x5a;


	/*Step 1:Reset  CTPM
	*write 0xaa to register 0xfc
	*/
	ft5x0x_write_reg(client, 0xfc, 0xaa);
	msleep(50);

	/*write 0x55 to register 0xfc */
	ft5x0x_write_reg(client, 0xfc, 0x55);
	msleep(30);

	/*********Step 2:Enter upgrade mode *****/
	auc_i2c_write_buf[0] = 0x55;
	auc_i2c_write_buf[1] = 0xaa;
	do {
		i++;
		i_ret = ft5x0x_i2c_Write(client, auc_i2c_write_buf, 2);
		msleep(5);
	} while (i_ret <= 0 && i < 5);


	/*********Step 3:check READ-ID***********************/
	auc_i2c_write_buf[0] = 0x90;
	auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =
			0x00;

	ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);

	if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
		dev_dbg(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
			 reg_val[0], reg_val[1]);
	else
		return -EIO;

	auc_i2c_write_buf[0] = 0xcd;
	ft5x0x_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);
	dev_dbg(&client->dev, "bootloader version = 0x%x\n", reg_val[0]);

	/*--------- read current project setting  ---------- */
	/*set read start address */
	buf[0] = 0x3;
	buf[1] = 0x0;
	buf[2] = 0x78;
	buf[3] = 0x0;

	ft5x0x_i2c_Read(client, buf, 4, buf, FTS_SETTING_BUF_LEN);
	dev_dbg(&client->dev, "[FTS] old setting: uc_i2c_addr = 0x%x,\
			uc_io_voltage = %d, uc_panel_factory_id = 0x%x\n",
			buf[0], buf[2], buf[4]);

	 /*--------- Step 4:erase project setting --------------*/
	auc_i2c_write_buf[0] = 0x63;
	ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(100);

	/*----------  Set new settings ---------------*/
	buf[0] = uc_i2c_addr;
	buf[1] = ~uc_i2c_addr;
	buf[2] = uc_io_voltage;
	buf[3] = ~uc_io_voltage;
	buf[4] = uc_panel_factory_id;
	buf[5] = ~uc_panel_factory_id;
	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;
	packet_buf[2] = 0x78;
	packet_buf[3] = 0x0;
	packet_buf[4] = 0;
	packet_buf[5] = FTS_SETTING_BUF_LEN;

	for (i = 0; i < FTS_SETTING_BUF_LEN; i++)
		packet_buf[6 + i] = buf[i];

	ft5x0x_i2c_Write(client, packet_buf, FTS_SETTING_BUF_LEN + 6);
	msleep(100);

	/********* reset the new FW***********************/
	auc_i2c_write_buf[0] = 0x07;
	ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);

	msleep(200);
	return 0;
}

int fts_ctpm_auto_upgrade(struct i2c_client *client)
{
	u8 uc_host_fm_ver = FT5x0x_REG_FW_VER;
	u8 uc_tp_fm_ver;
	int i_ret;

	ft5x0x_read_reg(client, FT5x0x_REG_FW_VER, &uc_tp_fm_ver);
	uc_host_fm_ver = fts_ctpm_get_i_file_ver();

	if (/*the firmware in touch panel maybe corrupted */
		uc_tp_fm_ver == FT5x0x_REG_FW_VER ||
		/*the firmware in host flash is new, need upgrade */
	     uc_tp_fm_ver < uc_host_fm_ver
	    ) {
		msleep(100);
		dev_dbg(&client->dev, "[FTS] uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x\n",
				uc_tp_fm_ver, uc_host_fm_ver);
		i_ret = fts_ctpm_fw_upgrade_with_i_file(client);
		if (i_ret == 0)	{
			msleep(300);
			uc_host_fm_ver = fts_ctpm_get_i_file_ver();
			dev_dbg(&client->dev, "[FTS] upgrade to new version 0x%x\n",
					uc_host_fm_ver);
		} else {
			pr_err("[FTS] upgrade failed ret=%d.\n", i_ret);
			return -EIO;
		}
	}

	return 0;
}

/*
*get upgrade information depend on the ic type
*/
static void fts_get_upgrade_info(struct Upgrade_Info *upgrade_info)
{
	switch (DEVICE_IC_TYPE) {
	case IC_FT5X06:
		upgrade_info->delay_55 = FT5X06_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5X06_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5X06_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5X06_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5X06_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT5X06_UPGRADE_EARSE_DELAY;
		break;
	case IC_FT5606:
		upgrade_info->delay_55 = FT5606_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5606_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5606_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5606_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5606_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT5606_UPGRADE_EARSE_DELAY;
		break;
	case IC_FT5316:
		upgrade_info->delay_55 = FT5316_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5316_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5316_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5316_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5316_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT5316_UPGRADE_EARSE_DELAY;
		break;
	case IC_FT6208:
		upgrade_info->delay_55 = FT6208_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT6208_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT6208_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT6208_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT6208_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT6208_UPGRADE_EARSE_DELAY;
		break;
	default:
		break;
	}
}
void delay_qt_ms(unsigned long  w_ms)
{
	unsigned long i;
	unsigned long j;

	for (i = 0; i < w_ms; i++)
	{
		for (j = 0; j < 1000; j++)
		{
			 udelay(1);
		}
	}
}
int fts_ctpm_fw_read_app(struct i2c_client *client, u8 *pbt_buf,
			  u32 dw_lenth)
{
	u32 packet_number;
	u32 j = 0;
	u32 temp;
	u32 lenght = 0;
	u8 *pReadBuf = NULL;
	u8 auc_i2c_write_buf[10];
	int i_ret;

	dw_lenth = dw_lenth - 2;

	pReadBuf = kmalloc(dw_lenth + 1, GFP_ATOMIC);
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	auc_i2c_write_buf[0] = 0x03;
	auc_i2c_write_buf[1] = 0x00;

	/*Read flash*/
	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		auc_i2c_write_buf[2] = (u8) (temp >> 8);
		auc_i2c_write_buf[3] = (u8) temp;
		
		i_ret = ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, 
			pReadBuf+lenght, FTS_PACKET_LENGTH);
		if (i_ret < 0)
			return -EIO;
		msleep(FTS_PACKET_LENGTH / 6 + 1);
		lenght += FTS_PACKET_LENGTH;
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		auc_i2c_write_buf[2] = (u8) (temp >> 8);
		auc_i2c_write_buf[3] = (u8) temp;
		temp = (dw_lenth) % FTS_PACKET_LENGTH;

		i_ret = ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, 
			pReadBuf+lenght, temp);
		if (i_ret < 0)
			return -EIO;
		msleep(FTS_PACKET_LENGTH / 6 + 1);
		lenght += temp;
	}

	/*read the last six byte */
	temp = 0x6ffa + j;
	auc_i2c_write_buf[2] = (u8) (temp >> 8);
	auc_i2c_write_buf[3] = (u8) temp;
	temp = 6;
	i_ret = ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, 
		pReadBuf+lenght, temp);
	if (i_ret < 0)
		return -EIO;
	msleep(FTS_PACKET_LENGTH / 6 + 1);
	lenght += temp;


	/*read app from flash and compart*/
	for (j=0; j<dw_lenth-2; j++) {
		if(pReadBuf[j] != pbt_buf[j]) {
			kfree(pReadBuf);
			return -EIO;
		}
	}

	kfree(pReadBuf);
	return 0;
}

int fts_ctpm_fw_upgrade(struct i2c_client *client, u8 *pbt_buf,
			  u32 dw_lenth)
{
	u8 reg_val[2] = {0};
	u32 i = 0;
	u32 packet_number;
	u32 j;
	u32 temp;
	u32 lenght;
	u8 packet_buf[FTS_PACKET_LENGTH + 6];
	u8 auc_i2c_write_buf[10];
	u8 bt_ecc;
	int i_ret;
	struct Upgrade_Info upgradeinfo;

	fts_get_upgrade_info(&upgradeinfo);
	
	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*********Step 1:Reset  CTPM *****/
		/*write 0xaa to register 0xfc */
		if (DEVICE_IC_TYPE == IC_FT6208)
			ft5x0x_write_reg(client, 0xbc, FT_UPGRADE_AA);
		else
			ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_AA);
		msleep(upgradeinfo.delay_aa);

		/*write 0x55 to register 0xfc */
		if (DEVICE_IC_TYPE == IC_FT6208)
			ft5x0x_write_reg(client, 0xbc, FT_UPGRADE_55);
		else
			ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_55);

		msleep(upgradeinfo.delay_55);
		/*********Step 2:Enter upgrade mode *****/
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		do {
			i++;
			i_ret = ft5x0x_i2c_Write(client, auc_i2c_write_buf, 2);
			msleep(5);
		} while (i_ret <= 0 && i < 5);


		/*********Step 3:check READ-ID***********************/
		msleep(upgradeinfo.delay_readid);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =
			0x00;
		ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);


		if (reg_val[0] == upgradeinfo.upgrade_id_1
			&& reg_val[1] == upgradeinfo.upgrade_id_2) {
			//dev_dbg(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
				//reg_val[0], reg_val[1]);
			DBG("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
				reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
				reg_val[0], reg_val[1]);
		}
	}
	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;
	auc_i2c_write_buf[0] = 0xcd;

	ft5x0x_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);


	/*Step 4:erase app and panel paramenter area*/
	DBG("Step 4:erase app and panel paramenter area\n");
	auc_i2c_write_buf[0] = 0x61;
	ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);	/*erase app area */
	msleep(upgradeinfo.delay_earse_flash);
	/*erase panel parameter area */
	auc_i2c_write_buf[0] = 0x63;
	ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(100);

	/*********Step 5:write firmware(FW) to ctpm flash*********/
	bt_ecc = 0;
	DBG("Step 5:write firmware(FW) to ctpm flash\n");

	dw_lenth = dw_lenth - 8;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;

	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		lenght = FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (lenght >> 8);
		packet_buf[5] = (u8) lenght;

		for (i = 0; i < FTS_PACKET_LENGTH; i++) {
			packet_buf[6 + i] = pbt_buf[j * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}
		
		ft5x0x_i2c_Write(client, packet_buf, FTS_PACKET_LENGTH + 6);
		msleep(FTS_PACKET_LENGTH / 6 + 1);
		//DBG("write bytes:0x%04x\n", (j+1) * FTS_PACKET_LENGTH);
		//delay_qt_ms(FTS_PACKET_LENGTH / 6 + 1);
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (temp >> 8);
		packet_buf[5] = (u8) temp;

		for (i = 0; i < temp; i++) {
			packet_buf[6 + i] = pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}

		ft5x0x_i2c_Write(client, packet_buf, temp + 6);
		msleep(20);
	}


	/*send the last six byte */
	for (i = 0; i < 6; i++) {
		temp = 0x6ffa + i;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		temp = 1;
		packet_buf[4] = (u8) (temp >> 8);
		packet_buf[5] = (u8) temp;
		packet_buf[6] = pbt_buf[dw_lenth + i];
		bt_ecc ^= packet_buf[6];
		ft5x0x_i2c_Write(client, packet_buf, 7);
		msleep(20);
	}


	/*********Step 6: read out checksum***********************/
	/*send the opration head */
	DBG("Step 6: read out checksum\n");
	auc_i2c_write_buf[0] = 0xcc;
	ft5x0x_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);
	if (reg_val[0] != bt_ecc) {
		dev_err(&client->dev, "[FTS]--ecc error! FW=%02x bt_ecc=%02x\n",
					reg_val[0],
					bt_ecc);
		return -EIO;
	}

	/*read app from flash and compare*/
	/*	
	DBG("Read flash and compare\n");
	if(fts_ctpm_fw_read_app(client, pbt_buf, dw_lenth+6) < 0) {
		dev_err(&client->dev, "[FTS]--app from flash is not equal to app.bin\n");
		return -EIO;
	}
	*/	
	/*********Step 7: reset the new FW***********************/
	DBG("Step 7: reset the new FW\n");
	auc_i2c_write_buf[0] = 0x07;
	ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(300);	/*make sure CTP startup normally */

	return 0;
}

/*sysfs debug*/

/*
*get firmware size

@firmware_name:firmware name
*note:the firmware default path is sdcard.
	if you want to change the dir, please modify by yourself.
*/
static int ft5x0x_GetFirmwareSize(char *firmware_name)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize = 0;
	char filepath[128];
	memset(filepath, 0, sizeof(filepath));

	sprintf(filepath, "%s", firmware_name);

	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);

	if (IS_ERR(pfile)) {
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}

	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	filp_close(pfile, NULL);
	return fsize;
}



/*
*read firmware buf for .bin file.

@firmware_name: fireware name
@firmware_buf: data buf of fireware

note:the firmware default path is sdcard.
	if you want to change the dir, please modify by yourself.
*/
static int ft5x0x_ReadFirmware(char *firmware_name,
			       unsigned char *firmware_buf)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize;
	char filepath[128];
	loff_t pos;
	mm_segment_t old_fs;

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s", firmware_name);
	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);
	if (IS_ERR(pfile)) {
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}

	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_read(pfile, firmware_buf, fsize, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);

	return 0;
}



/*
upgrade with *.bin file
*/

int fts_ctpm_fw_upgrade_with_app_file(struct i2c_client *client,
				       char *firmware_name)
{
	u8 *pbt_buf = NULL;
	int i_ret;
	int fwsize = ft5x0x_GetFirmwareSize(firmware_name);

	if (fwsize <= 0) {
		dev_err(&client->dev, "%s ERROR:Get firmware size failed\n",
					__func__);
		return -EIO;
	}

	if (fwsize < 8 || fwsize > 32 * 1024) {
		dev_dbg(&client->dev, "%s:FW length error\n", __func__);
		return -EIO;
	}

	/*=========FW upgrade========================*/
	pbt_buf = kmalloc(fwsize + 1, GFP_ATOMIC);

	if (ft5x0x_ReadFirmware(firmware_name, pbt_buf)) {
		dev_err(&client->dev, "%s() - ERROR: request_firmware failed\n",
					__func__);
		kfree(pbt_buf);
		return -EIO;
	}
	
	if ((pbt_buf[fwsize - 8] ^ pbt_buf[fwsize - 6]) == 0xFF
		&& (pbt_buf[fwsize - 7] ^ pbt_buf[fwsize - 5]) == 0xFF
		&& (pbt_buf[fwsize - 3] ^ pbt_buf[fwsize - 4]) == 0xFF) {
		/*call the upgrade function */
		i_ret = fts_ctpm_fw_upgrade(client, pbt_buf, fwsize);
		if (i_ret != 0)
			dev_dbg(&client->dev, "%s() - ERROR:[FTS] upgrade failed..\n",
						__func__);
		else {
#ifdef AUTO_CLB
			fts_ctpm_auto_clb(client);	/*start auto CLB*/
#endif
		 }
		kfree(pbt_buf);
	} else {
		dev_dbg(&client->dev, "%s:FW format error\n", __func__);
		kfree(pbt_buf);
		return -EIO;
	}

	return i_ret;
}

static ssize_t ft5x0x_tpfwver_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	ssize_t num_read_chars = 0;
	u8 fwver = 0;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);

	mutex_lock(&g_device_mutex);

	if (ft5x0x_read_reg(client, FT5x0x_REG_FW_VER, &fwver) < 0)
		num_read_chars = snprintf(buf, PAGE_SIZE,
					"get tp fw version fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%02X\n", fwver);

	mutex_unlock(&g_device_mutex);

	return num_read_chars;
}

static ssize_t ft5x0x_tpfwver_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	/*place holder for future use*/
	return -EPERM;
}



static ssize_t ft5x0x_tprwreg_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	/*place holder for future use*/
	return -EPERM;
}

static ssize_t ft5x0x_tprwreg_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	int retval;
	long unsigned int wmreg = 0;
	u8 regaddr = 0xff, regvalue = 0xff;
	u8 valbuf[5] = {0};

	memset(valbuf, 0, sizeof(valbuf));
	mutex_lock(&g_device_mutex);
	num_read_chars = count - 1;

	if (num_read_chars != 2) {
		if (num_read_chars != 4) {
			pr_info("please input 2 or 4 character\n");
			goto error_return;
		}
	}

	memcpy(valbuf, buf, num_read_chars);
	retval = strict_strtoul(valbuf, 16, &wmreg);

	if (0 != retval) {
		dev_err(&client->dev, "%s() - ERROR: Could not convert the "\
						"given input to a number." \
						"The given input was: \"%s\"\n",
						__func__, buf);
		goto error_return;
	}

	if (2 == num_read_chars) {
		/*read register*/
		regaddr = wmreg;
		if (ft5x0x_read_reg(client, regaddr, &regvalue) < 0)
			dev_err(&client->dev, "Could not read the register(0x%02x)\n",
						regaddr);
		else
			pr_info("the register(0x%02x) is 0x%02x\n",
					regaddr, regvalue);
	} else {
		regaddr = wmreg >> 8;
		regvalue = wmreg;
		if (ft5x0x_write_reg(client, regaddr, regvalue) < 0)
			dev_err(&client->dev, "Could not write the register(0x%02x)\n",
							regaddr);
		else
			dev_err(&client->dev, "Write 0x%02x into register(0x%02x) successful\n",
							regvalue, regaddr);
	}

error_return:
	mutex_unlock(&g_device_mutex);

	return count;
}

static ssize_t ft5x0x_fwupdate_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	/* place holder for future use */
	return -EPERM;
}

/*upgrade from *.i*/
static ssize_t ft5x0x_fwupdate_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct ft5x0x_ts_data *data = NULL;
	u8 uc_host_fm_ver;
	int i_ret;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);

	data = (struct ft5x0x_ts_data *)i2c_get_clientdata(client);

	mutex_lock(&g_device_mutex);

	disable_irq(client->irq);
	i_ret = fts_ctpm_fw_upgrade_with_i_file(client);
	if (i_ret == 0) {
		msleep(300);
		uc_host_fm_ver = fts_ctpm_get_i_file_ver();
		pr_info("%s [FTS] upgrade to new version 0x%x\n", __func__,
					 uc_host_fm_ver);
	} else
		dev_err(&client->dev, "%s ERROR:[FTS] upgrade failed.\n",
					__func__);

	enable_irq(client->irq);
	mutex_unlock(&g_device_mutex);

	return count;
}

static ssize_t ft5x0x_fwupgradeapp_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	/*place holder for future use*/
	return -EPERM;
}


/*upgrade from app.bin*/
static ssize_t ft5x0x_fwupgradeapp_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	char fwname[128];
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);

	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count - 1] = '\0';

	mutex_lock(&g_device_mutex);
	disable_irq(client->irq);

	fts_ctpm_fw_upgrade_with_app_file(client, fwname);

	enable_irq(client->irq);
	mutex_unlock(&g_device_mutex);

	return count;
}

/*sysfs */
/*get the fw version
*example:cat ftstpfwver
*/
static DEVICE_ATTR(ftstpfwver, S_IRUGO | S_IWUSR, ft5x0x_tpfwver_show,
			ft5x0x_tpfwver_store);

/*upgrade from *.i
*example: echo 1 > ftsfwupdate
*/
static DEVICE_ATTR(ftsfwupdate, S_IRUGO | S_IWUSR, ft5x0x_fwupdate_show,
			ft5x0x_fwupdate_store);

/*read and write register
*read example: echo 88 > ftstprwreg ---read register 0x88
*write example:echo 8807 > ftstprwreg ---write 0x07 into register 0x88
*
*note:the number of input must be 2 or 4.if it not enough,please fill in the 0.
*/
static DEVICE_ATTR(ftstprwreg, S_IRUGO | S_IWUSR, ft5x0x_tprwreg_show,
			ft5x0x_tprwreg_store);


/*upgrade from app.bin
*example:echo "*_app.bin" > ftsfwupgradeapp
*/
static DEVICE_ATTR(ftsfwupgradeapp, S_IRUGO | S_IWUSR, ft5x0x_fwupgradeapp_show,
			ft5x0x_fwupgradeapp_store);


/*add your attr in here*/
static struct attribute *ft5x0x_attributes[] = {
	&dev_attr_ftstpfwver.attr,
	&dev_attr_ftsfwupdate.attr,
	&dev_attr_ftstprwreg.attr,
	&dev_attr_ftsfwupgradeapp.attr,
	NULL
};

static struct attribute_group ft5x0x_attribute_group = {
	.attrs = ft5x0x_attributes
};

/*create sysfs for debug*/
int ft5x0x_create_sysfs(struct i2c_client *client)
{
	int err;
	err = sysfs_create_group(&client->dev.kobj, &ft5x0x_attribute_group);
	if (0 != err) {
		dev_err(&client->dev,
					 "%s() - ERROR: sysfs_create_group() failed.\n",
					 __func__);
		sysfs_remove_group(&client->dev.kobj, &ft5x0x_attribute_group);
		return -EIO;
	} else {
		mutex_init(&g_device_mutex);
		pr_info("ft5x0x:%s() - sysfs_create_group() succeeded.\n",
				__func__);
	}
	return err;
}

void ft5x0x_release_sysfs(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &ft5x0x_attribute_group);
	mutex_destroy(&g_device_mutex);
}

/*create apk debug channel*/

#define PROC_UPGRADE			0
#define PROC_READ_REGISTER		1
#define PROC_WRITE_REGISTER	2
#define PROC_RAWDATA			3
#define PROC_AUTOCLB			4

#define PROC_NAME	"ft5x0x-debug"
static unsigned char proc_operate_mode = PROC_RAWDATA;
static struct proc_dir_entry *ft5x0x_proc_entry;
/*interface of write proc*/
static int ft5x0x_debug_write(struct file *filp, 
	const char __user *buff, unsigned long len, void *data)
{
	struct i2c_client *client = (struct i2c_client *)ft5x0x_proc_entry->data;
	unsigned char writebuf[FTS_PACKET_LENGTH];
	int buflen = len;
	int writelen = 0;
	int ret = 0;
	
	if (copy_from_user(&writebuf, buff, buflen)) {
		dev_err(&client->dev, "%s:copy from user error\n", __func__);
		return -EFAULT;
	}
	proc_operate_mode = writebuf[0];
	
	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		{
			char upgrade_file_path[128];
			memset(upgrade_file_path, 0, sizeof(upgrade_file_path));
			sprintf(upgrade_file_path, "%s", writebuf + 1);
			upgrade_file_path[buflen-1] = '\0';
			DBG("%s\n", upgrade_file_path);
			disable_irq(client->irq);

			ret = fts_ctpm_fw_upgrade_with_app_file(client, upgrade_file_path);

			enable_irq(client->irq);
			if (ret < 0) {
				dev_err(&client->dev, "%s:upgrade failed.\n", __func__);
				return ret;
			}
		}
		break;
	case PROC_READ_REGISTER:
		writelen = 1;
		DBG("%s:register addr=0x%02x\n", __func__, writebuf[1]);
		ret = ft5x0x_i2c_Write(client, writebuf + 1, writelen);
		if (ret < 0) {
			dev_err(&client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_WRITE_REGISTER:
		writelen = 2;
		ret = ft5x0x_i2c_Write(client, writebuf + 1, writelen);
		if (ret < 0) {
			dev_err(&client->dev, "%s:write iic error\n", __func__);
			return ret;
		}
		break;
	case PROC_RAWDATA:
		break;
	case PROC_AUTOCLB:
		fts_ctpm_auto_clb(client);
		break;
	default:
		break;
	}
	

	return len;
}

/*interface of read proc*/
static int ft5x0x_debug_read( char *page, char **start,
	off_t off, int count, int *eof, void *data )
{
	struct i2c_client *client = (struct i2c_client *)ft5x0x_proc_entry->data;
	int ret = 0, err = 0;
	u8 tx = 0, rx = 0;
	int i, j;
	unsigned char buf[PAGE_SIZE];
	int num_read_chars = 0;
	int readlen = 0;
	u8 regvalue = 0x00, regaddr = 0x00;
	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		/*after calling ft5x0x_debug_write to upgrade*/
		regaddr = 0xA6;
		ret = ft5x0x_read_reg(client, regaddr, &regvalue);
		if (ret < 0)
			num_read_chars = sprintf(buf, "%s", "get fw version failed.\n");
		else
			num_read_chars = sprintf(buf, "current fw version:0x%02x\n", regvalue);
		break;
	case PROC_READ_REGISTER:
		readlen = 1;
		ret = ft5x0x_i2c_Read(client, NULL, 0, buf, readlen);
		if (ret < 0) {
			dev_err(&client->dev, "%s:read iic error\n", __func__);
			return ret;
		} else
			DBG("%s:value=0x%02x\n", __func__, buf[0]);
		num_read_chars = 1;
		break;
	case PROC_RAWDATA:
		break;
	default:
		break;
	}
	
	memcpy(page, buf, num_read_chars);

	return num_read_chars;
}
int ft5x0x_create_apk_debug_channel(struct i2c_client * client)
{
	ft5x0x_proc_entry = create_proc_entry(PROC_NAME, 0666, NULL);
	if (NULL == ft5x0x_proc_entry) {
		dev_err(&client->dev, "Couldn't create proc entry!\n");
		return -ENOMEM;
	} else {
		dev_info(&client->dev, "Create proc entry success!\n");
		ft5x0x_proc_entry->data = client;
		ft5x0x_proc_entry->write_proc = ft5x0x_debug_write;
		ft5x0x_proc_entry->read_proc = ft5x0x_debug_read;
	}
	return 0;
}

void ft5x0x_release_apk_debug_channel(void)
{
	if (ft5x0x_proc_entry)
		remove_proc_entry(PROC_NAME, NULL);
}


module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<luowj>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");

