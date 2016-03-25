#ifndef _GPIO_I2C_H_
#define _GPIO_I2C_H_


#include <linux/types.h>

#define I2C_TYPE_GPIO			0x00010000




int i2c_gpio_attach(int dev_num);
void  i2c_gpio_set(u32 id, u8 sda, u8 scl);
int i2c_mode_set(u32 id, int bps, int en);
int i2c_read(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_write(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);


#endif



