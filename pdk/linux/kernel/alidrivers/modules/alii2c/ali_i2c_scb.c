#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/types.h>
#include <asm/errno.h>
#include <linux/platform_device.h>
#include <asm/mach-ali/typedef.h>
#include <linux/err.h>
#include <linux/dvb/ali_i2c_scb_gpio.h>
#if 1
int i2c_gpio_read(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_gpio_write(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_gpio_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
#endif
int ali_i2c_scb_write(u32 id, u8 slv_addr, u8 *data, int len);
int ali_i2c_scb_read(u32 id, u8 slv_addr, u8 *data, int len);
int ali_i2c_scb_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int len);

int ali_i2c_read(u32 id, u8 slv_addr, u8 *data, int len)
{
    u32 id_minor = 0;
    int ret = -1;
    id_minor = id & 0x0f;
    if(id & I2C_TYPE_GPIO){ //gpio i2c
        ret = i2c_gpio_read(id_minor, slv_addr, data, len);
    }else{ //scb i2c
        ret = ali_i2c_scb_read(id_minor, slv_addr, data, len);
    } 
	return ret;
}
EXPORT_SYMBOL(ali_i2c_read);

int ali_i2c_write(u32 id, u8 slv_addr, u8 *data, int len)
{
    u32 id_minor = 0;
    int ret = -1;
    id_minor = id & 0x0f;
    if(id & I2C_TYPE_GPIO){ 
        ret = i2c_gpio_write(id_minor, slv_addr, data, len);
    }else{ 
        ret = ali_i2c_scb_write(id_minor, slv_addr, data, len);
    } 
	return ret;
}
EXPORT_SYMBOL(ali_i2c_write);

int ali_i2c_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{
    u32 id_minor = 0;
    int ret = -1;
    id_minor = id & 0x0f;
    if(id & I2C_TYPE_GPIO){ 
        ret = i2c_gpio_write_read(id_minor, slv_addr, data, wlen, rlen);
        //printk("Here!\n");
    }else{
        //printk("Here1!\n");
        ret = ali_i2c_scb_write_read(id_minor, slv_addr, data, wlen, rlen);
    } 
	return ret;
}
EXPORT_SYMBOL(ali_i2c_write_read);


