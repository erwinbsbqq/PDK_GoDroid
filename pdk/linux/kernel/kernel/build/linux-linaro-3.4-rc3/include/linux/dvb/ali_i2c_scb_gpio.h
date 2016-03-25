#ifndef _ALI_I2C_SCB_GPIO_H_
#define _ALI_I2C_SCB_GPIO_H_

#define I2C_TYPE_MASK			0xffff0000
#define I2C_ID_MASK				0x0000ffff

#define I2C_TYPE_SCB			0x00000000
#define I2C_TYPE_GPIO			0x00010000
#define I2C_TYPE_SCB_RM		    0x00020000

#define I2C_TYPE_SCB0			(I2C_TYPE_SCB|0)
#define I2C_TYPE_SCB1			(I2C_TYPE_SCB|1)
#define I2C_TYPE_SCB2			(I2C_TYPE_SCB|2)

#define I2C_TYPE_GPIO0			(I2C_TYPE_GPIO|0)
#define I2C_TYPE_GPIO1			(I2C_TYPE_GPIO|1)
#define I2C_TYPE_GPIO2			(I2C_TYPE_GPIO|2)

int ali_i2c_read(u32 id, u8 slv_addr, u8 *data, int len);
int ali_i2c_write(u32 id, u8 slv_addr, u8 *data, int len);
int ali_i2c_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);


#endif

