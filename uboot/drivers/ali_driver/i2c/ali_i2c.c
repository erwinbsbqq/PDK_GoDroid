#include <common.h>
#include <i2c.h>
#include <asm/io.h>

extern int i2c_scb_mode_set(u32 id, int bps, int en);
extern int i2c_scb_read(u32 id, u8 slv_addr, u8 *data, int len);
extern int i2c_scb_write(u32 id, u8 slv_addr, u8 *data, int len);
extern int i2c_scb_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
extern int i2c_gpio_mode_set(u32 id, int bps, int en);
extern int i2c_gpio_read(u32 id, u8 slv_addr, u8 *data, int len);
extern int i2c_gpio_write(u32 id, u8 slv_addr, u8 *data, int len);
extern int i2c_gpio_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
extern int i2c_gpio_init(int speed, int slaveaddr);

void i2c_init(int speed, int slaveaddr)
{
    uint id_minor = 0;
    id_minor = slaveaddr & 0x0f;
    if(slaveaddr & I2C_TYPE_GPIO){ //gpio i2c
        i2c_gpio_init(speed, id_minor);
    }else{ //scb i2c
        i2c_scb_mode_set(id_minor, speed, 1);
    }
}

int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
    uint id_minor = 0;
    int ret = -1;
    id_minor = chip & 0x0f;
    if(chip & I2C_TYPE_GPIO){ //gpio i2c
        ret = i2c_gpio_read(id_minor, addr, buffer, len);
    }else{ //scb i2c
        ret = i2c_scb_read(id_minor, addr, buffer, len);
    }
	return ret;
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
    uint id_minor = 0;
    int ret = -1;
    id_minor = chip & 0x0f;
    if(chip & I2C_TYPE_GPIO){ 
        ret = i2c_gpio_write(id_minor, addr, buffer, len);
    }else{ 
        ret = i2c_scb_write(id_minor, addr, buffer, len);
    } 
	return ret;
}

int ali_i2c_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{
    u32 id_minor = 0;
    int ret = -1;
    id_minor = id & 0x0f;
    if(id & I2C_TYPE_GPIO){ 
        ret = i2c_gpio_write_read(id_minor, slv_addr, data, wlen, rlen);
    }else{ 
        ret = i2c_scb_write_read(id_minor, slv_addr, data, wlen, rlen);
    } 
	return ret;
}


