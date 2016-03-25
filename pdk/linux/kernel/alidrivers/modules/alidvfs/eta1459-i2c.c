/*
 * eta1459-i2c.c  --  Generic I2C driver for ETA1459 PMIC
 *
 * Copyright (C) 2013 ALI Tech, Inc.
 *
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "eta1459-core.h"

#include <ali_board_config.h>

#include <linux/dvb/ali_i2c_scb_gpio.h>




extern int ali_i2c_read(u32 id, u8 slv_addr, u8 *data, int len);

extern int ali_i2c_write(u32 id, u8 slv_addr, u8 *data, int len);

extern int ali_i2c_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);

extern int i2c_gpio_mode_set(u32 id, int bps, int en);

extern void  i2c_gpio_set(u32 id, u8 sda, u8 scl);

extern int i2c_gpio_attach(int dev_num);


#ifdef CONFIG_REGULATOR_ETA1459

/* Define the ETA1459 regulator constraints */
static struct regulator_init_data eta1459_dcdc1_data = {
	.constraints = {
		.name = "vddarm_s3921",
		.min_uV = 800000,
		.max_uV = 1600000,
		.always_on = 1,
		.apply_uV = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask  = REGULATOR_CHANGE_VOLTAGE, /* */
	},
};


/* Define the ETA1459 regulator object */
static struct {
	int regulator;
	struct regulator_init_data *initdata;
} eta1459_regulators[] = {
	{ ETA1459_DCDC_1, &eta1459_dcdc1_data },
};

/* ETA1459( voltage adjustment chip) initialization interface for the eta1459 regulator registration */
static int __init s3921_eta1459_init(struct eta1459 *eta1459)
{
    int i = 0;
    int ret = 0;
    unsigned short us_data = 0;

#if 1
    ret = eta1459->read_dev(eta1459, 0x01, 0x01, (char *)&us_data);

    //printk(KERN_ALERT"\ns3921_eta1459_init, eta1459 data = %d, ret = %d\n", us_data, ret);

    /* Eta1459 voltage regulator register 0x01,  default value = 0x5c */
    if ((ret != 0) || (us_data != 0x5c))
    {
        return -ENXIO;
    }
#endif

    /* Instantiate the regulators */
    for (i = 0; i < ARRAY_SIZE(eta1459_regulators); i++)
    {
        eta1459_register_regulator(eta1459, eta1459_regulators[i].regulator, eta1459_regulators[i].initdata);
    }
	return 0;
}

/* Define eta1459( voltage adjustment chip) platform data for the eta1459 initialization */
static struct eta1459_platform_data __initdata s3921_eta1459_pdata = 
{
    .init = s3921_eta1459_init,
};

#if 0
/* Define I2C  board information for the I2C salve device registration with the appointed I2C bus */
static struct i2c_board_info i2c_devs0[] __initdata = 
{
    { I2C_BOARD_INFO("eta1459", 0x48),
        .platform_data = &s3921_eta1459_pdata,
    },
};
#endif

#if 1

#define I2C_TYPE_SCB			0x00000000
#define I2C_TYPE_GPIO		0x00010000
#define I2C_TYPE_SCB_RM		0x00020000

// For TI2C_ISR
#define	ISR_TDI			0x01	// Transaction Done Interrupt
// For TI2C_ISR
#define ISR1_TRIG	    0x01
#define I2C_TYPE_SCB0			(I2C_TYPE_SCB|0)
#define I2C_TYPE_SCB1			(I2C_TYPE_SCB|1)
#define I2C_TYPE_SCB2			(I2C_TYPE_SCB|2)
#define I2C_TYPE_SCB3			(I2C_TYPE_SCB|3)

#define I2C_TYPE_GPIO0			(I2C_TYPE_GPIO|0)
#define I2C_TYPE_GPIO1			(I2C_TYPE_GPIO|1)
#define I2C_TYPE_GPIO2			(I2C_TYPE_GPIO|2)
#define I2C_TYPE_GPIO3			(I2C_TYPE_GPIO|3)

#endif


static struct eta1459 eta1459_object;

static unsigned char uc_eta1459_i2c_slave_addr = (0x48 << 1);

static int eta1459_i2c_read_device(struct eta1459 *eta1459, char reg,
				  int bytes, void *dest)
{
    int ret = 0;    
    struct i2c_adapter *adapter;
    struct i2c_msg msgs[2] = {    { .addr = uc_eta1459_i2c_slave_addr >> 1, .flags = 0,                .len = 1,  .buf = (unsigned char *)&reg      },
                                                 { .addr = uc_eta1459_i2c_slave_addr >> 1,  .flags = I2C_M_RD, .len = 1,  .buf = (unsigned char *)dest    } 
                                            };
    int result = 0;



    //printk("\n\neta1459_i2c_read_device:reg:0x%x  zqs ",reg);

    if ((g_dvfs_i2c_device_id & 0x0f) <= (I2C_TYPE_SCB3 & 0xf))
    {
        /*use ALI i2c interface*/
        
        *(char *)dest = reg;
        ret = ali_i2c_write_read(g_dvfs_i2c_device_id, uc_eta1459_i2c_slave_addr, (unsigned char *)dest, 1, 1);    
    }
    else
    {
        /*use Linux kernel i2c interface*/
        adapter = i2c_get_adapter(g_dvfs_i2c_device_id);		

        if (adapter != NULL)
        {						
            if ((result = i2c_transfer(adapter, msgs, 2)) != 2)
            {
                printk("[ %s %d ], i2c_transfer fail, result = %d, name = %s, id = %d, nr = %d, class = 0x%08x\n", 
			    __FUNCTION__, __LINE__, result, adapter->name, adapter->nr, adapter->nr, adapter->class);
                
                ret = -EINVAL;
            }    
            else
            {
                ret = 0;
	    }	
        }
        else
        {            
            printk("[ %s %d ], adapter is NULL, i2c_id = %ld\n", __FUNCTION__, __LINE__, g_dvfs_i2c_device_id);
            ret = -EINVAL;
        }             
    }	
    
    return ret;
}

static int eta1459_i2c_write_device(struct eta1459 *eta1459, char reg,
				   int bytes, void *src)
{
    /* we add 1 byte for device register */
    u8 msg[(ETA1459_MAX_REGISTER << 1) + 1];
    int ret = 0;

    struct i2c_adapter *adapter;
    struct i2c_msg msgs = { .addr = uc_eta1459_i2c_slave_addr >> 1, .flags = 0, .len = 1 + bytes,  .buf = msg};
    int result = 0;

    //printk("\n\neta1459_i2c_write_device:reg:0x%x,data:0x%x zqs\n\n",reg,*(char*)src);

    if (bytes > ((ETA1459_MAX_REGISTER << 1) + 1))
    {
        return -EINVAL;
    }

    /* Fill the message bytes content */
    msg[0] = reg;
    memcpy(&msg[1], src, bytes);

    if ((g_dvfs_i2c_device_id & 0x0f) <= (I2C_TYPE_SCB3 & 0xf))
    {
        /*use ALI i2c interface*/
        ret = ali_i2c_write(g_dvfs_i2c_device_id, uc_eta1459_i2c_slave_addr, msg, bytes + 1);    
    }
    else
    {
        /*use Linux kernel i2c interface*/
        adapter = i2c_get_adapter(g_dvfs_i2c_device_id);		
	if (adapter != NULL)
        {						
            if ((result = i2c_transfer(adapter, &msgs, 1)) != 1)
            {
                printk("[ %s %d ], i2c_transfer fail, result = %d, name = %s, id = %d, nr = %d, class = 0x%08x\n", 
			   __FUNCTION__, __LINE__, result, adapter->name, adapter->nr, adapter->nr, adapter->class);
                
                ret = -EINVAL;
            }    
            else
            {
                ret = 0;
	    }	
        }
        else
        {            
            printk("[ %s %d ], adapter is NULL, i2c_id = %ld\n", __FUNCTION__, __LINE__, g_dvfs_i2c_device_id);
            ret = -EINVAL;
        }
    }
    
    return ret;
}


int register_eta1459_i2c(void)
{
    int ret = 0;

    //printk(KERN_ALERT"\neta1459_i2c_probe:%s,%d zqs\n",__FILE__,__LINE__);


#if 0
    if ((g_dvfs_i2c_device_id & I2C_TYPE_GPIO) != 0)
    { 
        //gpio i2c         
	    i2c_gpio_attach(3);

        printk(KERN_ALERT"\n\n\n\neta1459_i2c_probe:%s,%d\n\n\n",__FILE__,__LINE__);
        
       	i2c_gpio_set((g_dvfs_i2c_device_id & 0x0f), ALI_I2C_GPIO_DVFS_SDA_QFP_PIN, ALI_I2C_GPIO_DVFS_SCL_QFP_PIN);

	    i2c_gpio_mode_set((g_dvfs_i2c_device_id & 0x0f), 100000, 1);
    }
    else
    {
        //scb i2c
        ;
    }
#endif
    
    eta1459_object.read_dev = eta1459_i2c_read_device;
    eta1459_object.write_dev = eta1459_i2c_write_device;

    ret = eta1459_device_init(&eta1459_object, 1, &s3921_eta1459_pdata);
    if (ret < 0)
    {
        goto err;
    }
    return ret;

err:
    return ret;
}


#endif



#if 0

static int eta1459_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct eta1459 *eta1459;
	int ret = 0;

	//printk(KERN_ALERT"\neta1459_i2c_probe:%s,%d zqs\n",__FILE__,__LINE__);

	eta1459 = devm_kzalloc(&i2c->dev, sizeof(struct eta1459), GFP_KERNEL);
	if (eta1459 == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, eta1459);
	eta1459->dev = &i2c->dev;
	eta1459->i2c_client = i2c;
	eta1459->read_dev = eta1459_i2c_read_device;
	eta1459->write_dev = eta1459_i2c_write_device;

	ret = eta1459_device_init(eta1459, i2c->irq, i2c->dev.platform_data);
	if (ret < 0)
		goto err;

	return ret;

err:
	return ret;
}

static int eta1459_i2c_remove(struct i2c_client *i2c)
{
	struct eta1459 *eta1459 = i2c_get_clientdata(i2c);

	eta1459_device_exit(eta1459);

	return 0;
}

static const struct i2c_device_id eta1459_i2c_id[] = {
       { "eta1459", 0 },
       { }
};
MODULE_DEVICE_TABLE(i2c, eta1459_i2c_id);


static struct i2c_driver eta1459_i2c_driver = {
	.driver = {
		   .name = "eta1459",
		   .owner = THIS_MODULE,
	},
	.probe = eta1459_i2c_probe,
	.remove = eta1459_i2c_remove,
	.id_table = eta1459_i2c_id,
};

static int __init eta1459_i2c_init(void)
{
    //printk(KERN_ALERT"\n eta1459_i2c_init:%s,%d zqs\n",__FILE__,__LINE__);
    
    return i2c_add_driver(&eta1459_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(eta1459_i2c_init);

static void __exit eta1459_i2c_exit(void)
{
	i2c_del_driver(&eta1459_i2c_driver);
}
module_exit(eta1459_i2c_exit);

MODULE_DESCRIPTION("I2C support for the ETA1459 PMIC");
MODULE_LICENSE("GPL");
#endif

