/*
 * max31855  Programmable Controller driver (SPI bus)
 *
 * Copyright 2009-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/input.h>	/* BUS_SPI */
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/pm.h>
#include <linux/types.h>

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/of_gpio.h>
#include <linux/sensors.h>
#include <ali_reg.h>

//#include "max31855.h"

#define MAX31855_SPI_CMD_PREFIX      0xE000   /* bits 15:11 */
#define MAX31855_SPI_READ            BIT(10)


#define MAX31855_DEV_NAME	"max31855"
#define MAX31855_DEFAULT_POLL_INTERVAL_MS   1000// 200
#define MAX31855_MIN_POLL_INTERVAL_MS	10
#define MAX31855_MAX_POLL_INTERVAL_MS	5000
#define MAX31855_MIN_VALUE	-32768
#define MAX31855_MAX_VALUE	32767


struct max31855_sensor {
	struct spi_device *spi;
	struct device *dev;
	struct input_dev *temperature_dev;
	struct sensors_classdev temperature_cdev;
	struct max31855_platform_data *pdata;
	struct mutex op_lock;
//	enum inv_devices chip_type;
	struct delayed_work temperature_poll_work;
	int enabled;
	int wakeup_en;
	int temperature_value;
	int temperature_poll_ms;
	u8 spi_transfer_buf[16];
};

/* Accelerometer information read by HAL */
static struct sensors_classdev max31855_cdev = {
	.name = "max31855",
	.vendor = "Invensense",
	.version = 1,
	.handle = SENSORS_TEMPERATURE_HANDLE,
	.type = SENSOR_TYPE_TEMPERATURE,
	.max_range = "200",	/* m/s^2 */
	.resolution = "0.01",	/* m/s^2 */
	.sensor_power = "0.5",	/* 0.5 mA */
	.min_delay = 10000,
	.delay_msec = 200,
	.fifo_reserved_event_count = 0,
	.fifo_max_event_count = 0,
	.enabled = 0,
	.sensors_enable = NULL,
	.sensors_poll_delay = NULL,
};



#if defined(CONFIG_ALI_CHIP_M3921)
#define ALI_SOC_REG_BASE_PHYS 0x18000000
static u32 reg_data_store_8c;
static u32 reg_data_store_88;

extern struct mutex ali_sto_mutex ; // For S3921, use mutex and register to select NF/SD/SF/EMMC 
static void strapin_2_pinmux(void)
{
	u32 value1;

	reg_data_store_8c = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c);
	reg_data_store_88 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c);
	value1 = (value1 & (~(0x01<<31)));
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c) = value1;
}

static void nand_2_nor(void)
{
	u32 value2 ;
	
	value2 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	value2 = (value2 & (~(0x01<<3)));
	value2 = (value2 |(0x01<<19));
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88) = value2;
}

static void pinmux88_2_default(void)
{
	u32 value1;
	
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	value1 = reg_data_store_88;
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88) = value1;		
}

static void pinmux8c_2_default(void)
{
	u32 value1;
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c);
	value1 = reg_data_store_8c;
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c) = value1;
}

static void pinmux_2_default(void)
{
	pinmux88_2_default();
	pinmux8c_2_default();
}

void ali_spinor_pinmux_release(void) // For S3921, use register to select NF/SD/SF/EMMC  
{
	pinmux_2_default();
	//printk("SPI unlock\n");
	mutex_unlock(&ali_sto_mutex);	
 
}

void ali_spinor_pinmux_set(void)  // For S3921, use register to select NF/SD/SF/EMMC 
{
 
	mutex_lock(&ali_sto_mutex);
 
	strapin_2_pinmux();
	nand_2_nor();	
}
 
static void pinmux_2_spi( )
{
 	ali_spinor_pinmux_set();
}
 static void pinmux_2_nand( )
{
 	ali_spinor_pinmux_release();
}
 #endif
/*
static int max31855_spi_read(struct max31855_sensor *sensor,
			   unsigned short reg, unsigned short *data, size_t len)
{
	struct spi_device *spi = to_spi_device(sensor->dev);
	struct spi_message message;
	struct spi_transfer xfer[2];
	int i;
	int error;

	spi_message_init(&message);
	memset(xfer, 0, sizeof(xfer));

	sensor->xfer_buf[0] = cpu_to_be16(MAX31855_SPI_CMD_PREFIX |
					MAX31855_SPI_READ | reg);
	xfer[0].tx_buf = &sensor->xfer_buf[0];
	xfer[0].len = sizeof(sensor->xfer_buf[0]);
	spi_message_add_tail(&xfer[0], &message);

	xfer[1].rx_buf = &sensor->xfer_buf[1];
	xfer[1].len = sizeof(sensor->xfer_buf[1]) * len;
	spi_message_add_tail(&xfer[1], &message);

	error = spi_sync(spi, &message);
	if (unlikely(error)) {
		dev_err(sensor->dev, "SPI read error: %d\n", error);
		return error;
	}

	for (i = 0; i < len; i++)
		data[i] = be16_to_cpu(sensor->xfer_buf[i + 1]);

	return 0;
}

static int max31855_spi_write(struct max31855_sensor *sensor,
			    unsigned short reg, unsigned short data)
{
	struct spi_device *spi = to_spi_device(sensor->dev);
	int error;

	sensor->xfer_buf[0] = cpu_to_be16(MAX31855_SPI_CMD_PREFIX | reg);
	sensor->xfer_buf[1] = cpu_to_be16(data);

	error = spi_write(spi, (u8 *)sensor->xfer_buf,
			  2 * sizeof(*sensor->xfer_buf));
	if (unlikely(error)) {
		dev_err(sensor->dev, "SPI write error: %d\n", error);
		return error;
	}

	return 0;
}*/
static int max31855_set_enable(struct max31855_sensor *sensor, bool enable)
{
	int ret = 0;
	printk("max31855_set_enable  enable=%d\n",enable);

	mutex_lock(&sensor->op_lock);
	printk("max31855_set_enable 2\n");
	if (enable) {
		sensor->enabled=1;
		schedule_delayed_work(&sensor->temperature_poll_work,
				msecs_to_jiffies(sensor->temperature_poll_ms));
	} else {
		/*ret = max31855_enable(sensor, false);
		if (ret) {
			dev_err(&sensor->spi->dev,
				"Fail to disable gyro engine ret=%d\n", ret);
			ret = -EBUSY;
			goto exit;
		}*/
		sensor->enabled=0;
		cancel_delayed_work_sync(&sensor->temperature_poll_work);

	}

	mutex_unlock(&sensor->op_lock);
	return ret;
}
static ssize_t max31855_attr_get_polling_delay(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int val;
	printk("max31855_attr_get_polling_delay  \n");

	struct max31855_sensor *sensor = dev_get_drvdata(dev);
	if (sensor == NULL)
		return -ENOMEM;
	val = sensor ? sensor->temperature_poll_ms : 0;
	return snprintf(buf, 8, "%d\n", val);
}

/**
 * max31855_attr_set_polling_delay - set the sampling rate
 */
 static int max31855_set_poll_delay(struct max31855_sensor *sensor,
					unsigned long delay)
{
	int ret;
	printk("max31855_set_poll_delay  \n");

	mutex_lock(&sensor->op_lock);
	if (delay < MAX31855_MIN_POLL_INTERVAL_MS)
		delay = MAX31855_MIN_POLL_INTERVAL_MS;
	if (delay > MAX31855_MAX_POLL_INTERVAL_MS)
		delay = MAX31855_MAX_POLL_INTERVAL_MS;

	sensor->temperature_poll_ms = delay;
	
	cancel_delayed_work_sync(&sensor->temperature_poll_work);
	schedule_delayed_work(&sensor->temperature_poll_work,
				msecs_to_jiffies(sensor->temperature_poll_ms));
	
	mutex_unlock(&sensor->op_lock);
	return 0;
}

static ssize_t max31855_attr_set_polling_delay(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct max31855_sensor *sensor = dev_get_drvdata(dev);
	unsigned long interval_ms;
	int ret;
	printk("max31855_attr_set_polling_delay  \n");

	if (sensor == NULL)
		return -ENOMEM;
	if (kstrtoul(buf, 10, &interval_ms))
		return -EINVAL;

	ret = max31855_set_poll_delay(sensor, interval_ms);

	return ret ? -EBUSY : size;
}

static ssize_t max31855_attr_get_enable(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct max31855_sensor *sensor = dev_get_drvdata(dev);
	printk("max31855_attr_get_enable  \n");

	if (sensor == NULL)
		return -ENOMEM;
	return snprintf(buf, 4, "%d\n", sensor->enabled);
}

/**
 * max31855_attr_set_enable -
 *    Set/get enable function is just needed by sensor HAL.
 */
static ssize_t max31855_attr_set_enable(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct max31855_sensor *sensor = dev_get_drvdata(dev);
	unsigned long enable;
	int ret;
	printk("max31855_attr_set_enable  \n");
	if (sensor == NULL)
		return -ENOMEM;
	if (kstrtoul(buf, 10, &enable))
		return -EINVAL;

	if (enable)
		ret = max31855_set_enable(sensor, true);
	else
		ret = max31855_set_enable(sensor, false);

	return ret ? -EBUSY : count;
}

static struct device_attribute temperature_attr[] = {
	__ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
		max31855_attr_get_polling_delay,
		max31855_attr_set_polling_delay),
	__ATTR(enable, S_IRUGO | S_IWUSR,
		max31855_attr_get_enable,
		max31855_attr_set_enable),
};

#ifdef CONFIG_PM
static int max31855_spi_suspend(struct device *device)
{
	struct spi_device *client  = container_of(device,struct spi_device, dev);
	
	struct max31855_sensor *sensor = dev_get_drvdata(&client->dev);
	printk("max31855_spi_suspend  \n");
	if (sensor == NULL)
		return -ENOMEM;

	mutex_lock(&sensor->op_lock);

	if (sensor->enabled)
		cancel_delayed_work_sync(&sensor->temperature_poll_work);
	

	mutex_unlock(&sensor->op_lock);
	dev_dbg(&client->dev, "Suspend completed\n");
	return 0;
}

static int max31855_spi_resume(struct device *device)
{
	struct spi_device *client  = container_of(device,struct spi_device, dev);
	printk("max31855_spi_resume  \n");
	int ret=0;
	struct max31855_sensor *sensor = dev_get_drvdata(&client->dev);
	if (sensor == NULL)
		return -ENOMEM;

	if (sensor->enabled) {
		ret = max31855_set_enable(sensor, true);
		
		schedule_delayed_work(&sensor->temperature_poll_work,
			msecs_to_jiffies(sensor->temperature_poll_ms));
		
	}
	return ret;
}

#endif

//static SIMPLE_DEV_PM_OPS(max31855_spi_pm, max31855_spi_suspend, max31855_spi_resume);
static UNIVERSAL_DEV_PM_OPS(max31855_spi_pm, max31855_spi_suspend, max31855_spi_resume, NULL);

static int create_temperature_sysfs_interfaces(struct device *dev)
{
	int i;
	int err;
	for (i = 0; i < ARRAY_SIZE(temperature_attr); i++) {
		err = device_create_file(dev, temperature_attr + i);
		if (err)
			goto error;
	}
	printk("create_temperature_sysfs_interfaces  \n");

	return 0;

error:
	for (; i >= 0; i--)
		device_remove_file(dev, temperature_attr + i);
	dev_err(dev, "Unable to create interface\n");
	printk("Unable to create interface  \n");

	return err;
}

static int remove_temperature_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(temperature_attr); i++)
		device_remove_file(dev, temperature_attr + i);
	return 0;
}
static void max31855_work_fn(struct work_struct *work)
{
	struct max31855_sensor *sensor;
	u32 shift;
	int ret,val,intTemp=0;
	long tmp,remTemp;
	int temperature=0;
	float temperature_f=0;
	ktime_t timestamp;
	u8* tx_buf;
	u8* rx_buf;
	
	//printk("max31855_work_fn  \n");
	sensor = container_of((struct delayed_work *)work,
				struct max31855_sensor, temperature_poll_work);
	if (sensor == NULL)
		return -ENOMEM;
	timestamp = ktime_get();
	//mpu6050_read_accel_data(sensor, &sensor->temperature_value);
	//mpu6050_remap_accel_data(&sensor->axis, sensor->pdata->place);
//////////////////////////////////////////////////////
	tx_buf = sensor->spi_transfer_buf;  
	rx_buf = sensor->spi_transfer_buf + 8;  

	mutex_lock(&sensor->op_lock);  
#if defined(CONFIG_ALI_CHIP_M3921)
//	pinmux_2_spi();
#endif
	tx_buf[0] = 0;//INSTRUCTION_READ;  
	tx_buf[1] = 0;//reg;  
	//tx_buf[2] = 0;
	//tx_buf[3] = 0;
	rx_buf[0] =0;
	
	ret =spi_read(sensor->spi, rx_buf, 4);// spi_write_then_read(sensor->spi, tx_buf, 1, rx_buf, 4);  
	//	printk("%s: rx_buf = %x  %x   %x  %x  %x \n", __FUNCTION__, tx_buf[0],rx_buf[0],rx_buf[1],rx_buf[2],rx_buf[3]);
	/*ret = spi_write_then_read(sensor->spi, tx_buf, 1, &rx_buf[1], 1);  
		printk("%s: rx_buf = %x  %x \n", __FUNCTION__, tx_buf[0],rx_buf[1]);

	ret = spi_write_then_read(sensor->spi, tx_buf, 1, &rx_buf[2], 1);  
		printk("%s: rx_buf = %x  %x \n", __FUNCTION__, tx_buf[0],rx_buf[2]);

	ret = spi_write_then_read(sensor->spi, tx_buf, 1, &rx_buf[3], 1);  
		printk("%s: rx_buf = %x  %x \n", __FUNCTION__, tx_buf[0],rx_buf[3]);*/

	//ret = spi_write_then_read(sensor->spi, tx_buf, 1, rx_buf, 4); 
	//printk("%s: rx_buf = %x  %x %x %x\n", __FUNCTION__, rx_buf[0],rx_buf[1],rx_buf[2],rx_buf[3]/*, rx_buf[4],rx_buf[5],rx_buf[6],rx_buf[7]*/);
#if defined(CONFIG_ALI_CHIP_M3921)
//	pinmux_2_nand();
#endif	
	if (ret < 0)  
	{  
		dev_dbg(&sensor->dev, "%s: failed: ret = %d\n", __FUNCTION__, ret);  
		val = 0;  
		printk("%s: failed: ret = %d\n", __FUNCTION__, ret);
		goto error;
	}  
	else  
		val = 1;  
	mutex_unlock(&sensor->op_lock);  

	if(val==1)
	{
		if((rx_buf[1] & 0x01) == 0x01){          // Fault detection
		  	printk("spi_write_then_read  Error!");
			if((rx_buf[3] & 0x01) == 0x01){        // Open circuit fault?
			 	printk("Open circuit");           // Write text in first row
			}
			//
			if((rx_buf[3] & 0x02) == 0x02){        // Short to GND fault?
				printk("Short to GND");           // Write text in first row
			}
			//
			if((rx_buf[3] & 0x04) == 0x04){        // Short to Vcc fault?
				printk("Short to Vcc");           // Write text in first row
			}
			goto error;
		}
		tmp = rx_buf[0];
		tmp = tmp << 8;
		tmp = tmp | rx_buf[1];
		remTemp = tmp /4;//tmp >> 2;
		remTemp = remTemp & 0x03;                     // Decimal part of temperature value
		temperature =remTemp *25;//* 0.25*100;
		intTemp = tmp /16;  //tmp >> 4;
		intTemp=intTemp*100;// Intiger part of temperature value
		temperature += intTemp;                       // Temperature value
		sensor->temperature_value=temperature*100;  //temperature =temperature*100, for report  int.  so  in android HAL need to temperature/100
	}
////////////////////////////////////////////////////////
	//printk("%s: temperature = %d  \n", __FUNCTION__, sensor->temperature_value);
	input_report_abs(sensor->temperature_dev, ABS_THROTTLE,
		sensor->temperature_value);
	
	input_event(sensor->temperature_dev,
			EV_SYN, SYN_TIME_SEC,
			ktime_to_timespec(timestamp).tv_sec);
	input_event(sensor->temperature_dev, EV_SYN,
		SYN_TIME_NSEC,
		ktime_to_timespec(timestamp).tv_nsec);
	input_sync(sensor->temperature_dev);
error:
	schedule_delayed_work(&sensor->temperature_poll_work,msecs_to_jiffies(sensor->temperature_poll_ms));
}


static int max31855_cdev_enable(struct sensors_classdev *sensors_cdev,
			unsigned int enable)
{
	struct max31855_sensor *sensor = container_of(sensors_cdev,
			struct max31855_sensor,temperature_cdev);
	printk("max31855_cdev_enable  enable=%d\n",enable);
	if (sensor == NULL)
		return -ENOMEM;
	return max31855_set_enable(sensor, enable);
}

static int max31855_cdev_poll_delay(struct sensors_classdev *sensors_cdev,
			unsigned int delay_ms)
{
	struct max31855_sensor *sensor = container_of(sensors_cdev,
			struct max31855_sensor, temperature_cdev);
	printk("max31855_cdev_poll_delay  delay_ms=%d\n",delay_ms);
	if (sensor == NULL)
		return -ENOMEM;
	return max31855_set_poll_delay(sensor, delay_ms);
}

static int max31855_cdev_enable_wakeup(
			struct sensors_classdev *sensors_cdev,
			unsigned int enable)
{
	struct max31855_sensor *sensor = container_of(sensors_cdev,
			struct max31855_sensor, temperature_cdev);
	printk("max31855_cdev_enable_wakeup  enable=%d\n",enable);
	if (sensor == NULL)
		return -ENOMEM;
	sensor->enabled= enable;
	return 0;
}


static int  max31855_spi_probe(struct spi_device *spi)
{

	int ret,err;
	struct max31855_sensor *sensor;
	printk("max31855_spi_probe  \n");
	err = spi_setup(spi);
	if (err < 0)
		return err;
	printk("max31855_spi_probe1  \n");

	sensor = devm_kzalloc(&spi->dev, sizeof(*sensor), GFP_KERNEL);
	if (sensor == NULL)
		return -ENOMEM;

	spi_set_drvdata(spi, sensor);

/*	ret = sysfs_create_group(&spi->dev.kobj, &ad7314_group);
	if (ret < 0)
		return ret;
*/
	mutex_init(&sensor->op_lock);
	sensor->spi = spi;
	sensor->dev= &spi->dev;
	printk("max31855_spi_probe2  name=%s\n",dev_name(sensor->dev));

	sensor->temperature_dev = input_allocate_device();
	if (!sensor->temperature_dev ) {
		dev_err(&spi->dev,
			"Failed to allocate temperature input device\n");
		ret = -EINVAL;
		goto err_free_devmem;
	}

	sensor->temperature_dev->name = MAX31855_DEV_NAME;
	sensor->temperature_dev->id.bustype = BUS_SPI;
	sensor->temperature_poll_ms= MAX31855_DEFAULT_POLL_INTERVAL_MS;

	input_set_capability(sensor->temperature_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(sensor->temperature_dev, ABS_THROTTLE,
			MAX31855_MIN_VALUE, MAX31855_MAX_VALUE,
			0, 0);
	sensor->temperature_dev->dev.parent = &spi->dev;
	input_set_drvdata(sensor->temperature_dev, sensor);

	INIT_DELAYED_WORK(&sensor->temperature_poll_work,
			max31855_work_fn);
	printk("max31855_spi_probe3  \n");

	ret = input_register_device(sensor->temperature_dev);
	if (ret) {
		dev_err(&spi->dev, "Failed to register input device\n");
		goto err_free_input;
	}

	printk("max31855_spi_probe4  \n");

	ret = create_temperature_sysfs_interfaces(&sensor->temperature_dev->dev);
	if (ret < 0) {
		dev_err(&spi->dev, "failed to create sysfs for gyro\n");
		goto err_unregister_temperature;
	}
	
	sensor->temperature_cdev= max31855_cdev;
	sensor->temperature_cdev.delay_msec = sensor->temperature_poll_ms;
	sensor->temperature_cdev.sensors_enable = max31855_cdev_enable;
	sensor->temperature_cdev.sensors_poll_delay = max31855_cdev_poll_delay;
	sensor->temperature_cdev.sensors_enable_wakeup =
					max31855_cdev_enable_wakeup;
	printk("max31855_spi_probe5  \n");

	ret = sensors_classdev_register(&spi->dev, &sensor->temperature_cdev);
	if (ret) {
		dev_err(&spi->dev,
			"create accel class device file failed!\n");
		ret = -EINVAL;
		goto err_remove_temperature_sysfs;
	}

	return 0;
err_remove_temperature_sysfs:
	remove_temperature_sysfs_interfaces(&sensor->temperature_dev->dev);
err_unregister_temperature:
	input_unregister_device(sensor->temperature_dev);
err_free_input:
	input_free_device(sensor->temperature_dev);
err_free_devmem:
	devm_kfree(&spi->dev, sensor);
	dev_err(&spi->dev, "Probe device return error%d\n", ret);
	return ret;
}

static int  max31855_spi_remove(struct spi_device *spi)
{
	struct max31855_sensor *sensor = spi_get_drvdata(spi);
	printk("max31855_spi_remove  \n");

	sensors_classdev_unregister(&sensor->temperature_cdev);
	remove_temperature_sysfs_interfaces(&sensor->temperature_dev->dev);
	input_unregister_device(sensor->temperature_dev);
	if (spi->irq > 0)
		free_irq(spi->irq, sensor);
	input_free_device(sensor->temperature_dev);
	
	devm_kfree(&spi->dev, sensor);

	spi_set_drvdata(spi, NULL);

	return 0;
}


static const struct spi_device_id max31855_ids[] = {
	{ "max31855", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, max31855_ids);
/*
static const struct of_device_id max31855_of_match[] = {
	{ .compatible = "invn,max31855", },
	{ },
};
MODULE_DEVICE_TABLE(of, max31855_of_match);
*/
static struct spi_driver max31855_spi_driver = {
	.driver = {
		.name	= "max31855",
		.owner	= THIS_MODULE,
		.pm	= &max31855_spi_pm,
	},
	.probe		= max31855_spi_probe,
	.remove		= max31855_spi_remove,//__devexit_p(max31855_spi_remove),
	.id_table	= max31855_ids,
};

static int __init max31855_init(void)
{
	int ret;
	printk("max31855_init  \n");
	ret=spi_register_driver(&max31855_spi_driver);
	return ret;
}

static void __exit max31855_exit(void)
{
	printk("max31855_init  \n");
	spi_unregister_driver(&max31855_spi_driver);
}


module_init(max31855_init);
module_exit(max31855_exit);
//module_spi_driver(max31855_spi_driver);

MODULE_DESCRIPTION("max31855 Sensor SPI Bus Driver");
MODULE_AUTHOR("Francis Cheng ");
MODULE_LICENSE("GPL");

