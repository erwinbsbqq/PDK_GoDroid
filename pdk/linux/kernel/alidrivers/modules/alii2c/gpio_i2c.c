

#include "gpio_i2c.h"
#include <linux/slab.h> 
#include <linux/delay.h> #include <linux/ali_reg.h>


#ifdef SUCCESS
	#undef SUCCESS
	#define SUCCESS 0
#else
	#define SUCCESS 0
#endif

#ifdef ERR_FAILURE
	#undef ERR_FAILURE
	#define ERR_FAILURE -1
#else
	#define ERR_FAILURE -1
#endif

#define I2C_GPIO_TIMES_OUT		10

#define I2C_CLOCK_PERIOD        5

#if defined(CONFIG_ARM)
#define SYS_I2C_SDA 17
#define SYS_I2C_SCL 18
#else
#define SYS_I2C_SDA 7
#define SYS_I2C_SCL 6


#endif

#define TRUE 1
#define EXTERNAL_PULL_HIGH		TRUE
#define ERR_FAILUE -1
#define ERR_TIME_OUT -9
#define HAL_GPIO_I_DIR		0 //0
#define HAL_GPIO_O_DIR		1   //0 is out ; 1

#define ERR_I2C_SCL_LOCK	1
#define ERR_I2C_SDA_LOCK	2
#define OSAL_WAIT_FOREVER_TIME 0xFFFFFFFF
//#define I2C_TYPE_GPIO			0x00010000

#define GPIO_GROUP_MAX	5
#define GPIO_PORT_MAX		169

#define HAL_GPIO_IER_REG	__REGALIRAW(0x18000044)
#define HAL_GPIO_REC_REG	__REGALIRAW(0x18000048)
#define HAL_GPIO_FEC_REG	__REGALIRAW(0x1800004c)
#define HAL_GPIO_ISR_REG	__REGALIRAW(0x1800005c)
#define HAL_GPIO_DIR_REG	__REGALIRAW(0x18000058)
#define HAL_GPIO_DI_REG	__REGALIRAW(0x18000050)
#define HAL_GPIO_DO_REG	__REGALIRAW(0x18000054)
#define HAL_GPIO_EN_REG    __REGALIRAW(0x18000430)

#define HAL_GPIO1_IER_REG	__REGALIRAW(0x180000c4)
#define HAL_GPIO1_REC_REG	__REGALIRAW(0x180000c8)
#define HAL_GPIO1_FEC_REG	__REGALIRAW(0x180000cc)
#define HAL_GPIO1_ISR_REG	__REGALIRAW(0x180000dc)
#define HAL_GPIO1_DIR_REG	__REGALIRAW(0x180000d8)
#define HAL_GPIO1_DI_REG	__REGALIRAW(0x180000d0)
#define HAL_GPIO1_DO_REG	__REGALIRAW(0x180000d4)
#define HAL_GPIO1_EN_REG   __REGALIRAW(0xb8000434)

#define HAL_GPIO2_IER_REG	__REGALIRAW(0x180000e4)
#define HAL_GPIO2_REC_REG	__REGALIRAW(0x180000e8)
#define HAL_GPIO2_FEC_REG	__REGALIRAW(0x180000ec)
#define HAL_GPIO2_ISR_REG	__REGALIRAW(0x180000fc)
#define HAL_GPIO2_DIR_REG	__REGALIRAW(0x180000f8)
#define HAL_GPIO2_DI_REG	__REGALIRAW(0x180000f0)
#define HAL_GPIO2_DO_REG	__REGALIRAW(0x180000f4)
#define HAL_GPIO2_EN_REG   __REGALIRAW(0x18000438)


#define HAL_GPIO3_IER_REG	__REGALIRAW(0x18000344)
#define HAL_GPIO3_REC_REG	__REGALIRAW(0x18000348)
#define HAL_GPIO3_FEC_REG	__REGALIRAW(0x1800034c)
#define HAL_GPIO3_ISR_REG	__REGALIRAW(0x1800035c)
#define HAL_GPIO3_DIR_REG	__REGALIRAW(0x18000358)
#define HAL_GPIO3_DI_REG	__REGALIRAW(0x18000350)
#define HAL_GPIO3_DO_REG	__REGALIRAW(0x18000354)
#define HAL_GPIO3_EN_REG   __REGALIRAW(0x1800043c)
//added by wizephen wang
#define HAL_GPIO4_IER_REG	__REGALIRAW(0x18000444)
#define HAL_GPIO4_REC_REG	__REGALIRAW(0x18000448)
#define HAL_GPIO4_FEC_REG	__REGALIRAW(0x1800044c)
#define HAL_GPIO4_ISR_REG	__REGALIRAW(0x1800045c)
#define HAL_GPIO4_DIR_REG	__REGALIRAW(0x18000458)
#define HAL_GPIO4_DI_REG	__REGALIRAW(0x18000450)
#define HAL_GPIO4_DO_REG	__REGALIRAW(0x18000454)
#define HAL_GPIO4_EN_REG   __REGALIRAW(0x18000440)

#define HAL_GPIO_INT_EN	    0
#define HAL_GPIO_INT_DIS	1
#define HAL_GPIO_EDG_EN	    1
#define HAL_GPIO_EDG_DIS	0
#define HAL_GPIO_ENABLE     1

#define HAL_GPIO_READ()			(*(volatile unsigned long *)HAL_GPIO_DI_REG)
#define HAL_GPIO_WRITE(val)		(*(volatile unsigned long *)HAL_GPIO_DO_REG = (val))
#define HAL_GPIO_DIR_GET()			(*(volatile unsigned long *)HAL_GPIO_DIR_REG)
#define HAL_GPIO_DIR_SET(mode)		(*(volatile unsigned long *)HAL_GPIO_DIR_REG = (mode))
#define HAL_GPIO_IER_SET(val)		(*(volatile unsigned long *)HAL_GPIO_IER_REG = (val))
#define HAL_GPIO_RER_SET(val)		(*(volatile unsigned long *)HAL_GPIO_REC_REG = (val))
#define HAL_GPIO_FER_SET(val)		(*(volatile unsigned long *)HAL_GPIO_FEC_REG = (val))
#define HAL_GPIO_ISR_GET()			(*(volatile unsigned long *)HAL_GPIO_ISR_REG)
#define HAL_GPIO_ISR_SET(val)		(*(volatile unsigned long *)HAL_GPIO_ISR_REG = (val))
#define HAL_GPIO_EN_SET(val)       (*(volatile unsigned long *)HAL_GPIO_EN_REG =(val))

#define HAL_GPIO1_READ()			(*(volatile unsigned long *)HAL_GPIO1_DI_REG)
#define HAL_GPIO1_WRITE(val)		(*(volatile unsigned long *)HAL_GPIO1_DO_REG = (val))
#define HAL_GPIO1_DIR_GET()		(*(volatile unsigned long *)HAL_GPIO1_DIR_REG)
#define HAL_GPIO1_DIR_SET(mode)	(*(volatile unsigned long *)HAL_GPIO1_DIR_REG = (mode))
#define HAL_GPIO1_IER_SET(val)		(*(volatile unsigned long *)HAL_GPIO1_IER_REG = (val))
#define HAL_GPIO1_RER_SET(val)		(*(volatile unsigned long *)HAL_GPIO1_REC_REG = (val))
#define HAL_GPIO1_FER_SET(val)		(*(volatile unsigned long *)HAL_GPIO1_FEC_REG = (val))
#define HAL_GPIO1_ISR_GET()		(*(volatile unsigned long *)HAL_GPIO1_ISR_REG)
#define HAL_GPIO1_ISR_SET(val)		(*(volatile unsigned long *)HAL_GPIO1_ISR_REG = (val))
#define HAL_GPIO1_EN_SET(val)      (*(volatile unsigned long *)HAL_GPIO1_EN_REG =(val))

#define HAL_GPIO2_READ()			(*(volatile unsigned long *)HAL_GPIO2_DI_REG)
#define HAL_GPIO2_WRITE(val)		(*(volatile unsigned long *)HAL_GPIO2_DO_REG = (val))
#define HAL_GPIO2_DIR_GET()		(*(volatile unsigned long *)HAL_GPIO2_DIR_REG)
#define HAL_GPIO2_DIR_SET(mode)	(*(volatile unsigned long *)HAL_GPIO2_DIR_REG = (mode))
#define HAL_GPIO2_IER_SET(val)		(*(volatile unsigned long *)HAL_GPIO2_IER_REG = (val))
#define HAL_GPIO2_RER_SET(val)		(*(volatile unsigned long *)HAL_GPIO2_REC_REG = (val))
#define HAL_GPIO2_FER_SET(val)		(*(volatile unsigned long *)HAL_GPIO2_FEC_REG = (val))
#define HAL_GPIO2_ISR_GET()		(*(volatile unsigned long *)HAL_GPIO2_ISR_REG)
#define HAL_GPIO2_ISR_SET(val)		(*(volatile unsigned long *)HAL_GPIO2_ISR_REG = (val))
#define HAL_GPIO2_EN_SET(val)      (*(volatile unsigned long *)HAL_GPIO2_EN_REG =(val))
/*

#define HAL_GPIO3_READ()			(*(volatile unsigned long *)HAL_GPIO3_DI_REG)
#define HAL_GPIO3_WRITE(val)		(*(volatile unsigned long *)HAL_GPIO3_DO_REG = (val))
#define HAL_GPIO3_DIR_GET()		(*(volatile unsigned long *)HAL_GPIO3_DIR_REG)
#define HAL_GPIO3_DIR_SET(mode)	(*(volatile unsigned long *)HAL_GPIO3_DIR_REG = (mode))
*/

#define HAL_GPIO3_READ()			(*(volatile unsigned long *)HAL_GPIO3_DI_REG)
#define HAL_GPIO3_WRITE(val)		(*(volatile unsigned long *)HAL_GPIO3_DO_REG = (val))
#define HAL_GPIO3_DIR_GET()		(*(volatile unsigned long *)HAL_GPIO3_DIR_REG)
#define HAL_GPIO3_DIR_SET(mode)	(*(volatile unsigned long *)HAL_GPIO3_DIR_REG = (mode))
#define HAL_GPIO3_IER_SET(val)		(*(volatile unsigned long *)HAL_GPIO3_IER_REG = (val))
#define HAL_GPIO3_RER_SET(val)		(*(volatile unsigned long *)HAL_GPIO3_REC_REG = (val))
#define HAL_GPIO3_FER_SET(val)		(*(volatile unsigned long *)HAL_GPIO3_FEC_REG = (val))
#define HAL_GPIO3_ISR_GET()			(*(volatile unsigned long *)HAL_GPIO3_ISR_REG)
#define HAL_GPIO3_ISR_SET(val)		(*(volatile unsigned long *)HAL_GPIO3_ISR_REG = (val))
#define HAL_GPIO3_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO3_EN_REG =(val))

#define HAL_GPIO4_READ()			(*(volatile unsigned long *)HAL_GPIO4_DI_REG)
#define HAL_GPIO4_WRITE(val)		(*(volatile unsigned long *)HAL_GPIO4_DO_REG = (val))
#define HAL_GPIO4_DIR_GET()		(*(volatile unsigned long *)HAL_GPIO4_DIR_REG)
#define HAL_GPIO4_DIR_SET(mode)	(*(volatile unsigned long *)HAL_GPIO4_DIR_REG = (mode))
#define HAL_GPIO4_IER_SET(val)		(*(volatile unsigned long *)HAL_GPIO4_IER_REG = (val))
#define HAL_GPIO4_RER_SET(val)		(*(volatile unsigned long *)HAL_GPIO4_REC_REG = (val))
#define HAL_GPIO4_FER_SET(val)		(*(volatile unsigned long *)HAL_GPIO4_FEC_REG = (val))
#define HAL_GPIO4_ISR_GET()			(*(volatile unsigned long *)HAL_GPIO4_ISR_REG)
#define HAL_GPIO4_ISR_SET(val)		(*(volatile unsigned long *)HAL_GPIO4_ISR_REG = (val))
#define HAL_GPIO4_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO4_EN_REG =(val)


#define HAL_GPIO_BIT_DIR_SET(pos, val)	\
		do { \
			((pos < 32) ? HAL_GPIO_DIR_SET((HAL_GPIO_DIR_GET() & ~(1 << (pos))) | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_DIR_SET((HAL_GPIO1_DIR_GET() & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_DIR_SET((HAL_GPIO2_DIR_GET() & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			: ((pos < 128) ? HAL_GPIO3_DIR_SET((HAL_GPIO3_DIR_GET() & ~(1 << (pos - 96))) | ((val) << (pos - 96))) \
			: HAL_GPIO4_DIR_SET((HAL_GPIO4_DIR_GET() & ~(1 << (pos - 128))) | ((val) << (pos - 128)))))));\
		} while (0)

#define HAL_GPIO_BIT_SET(pos, val)		\
		do { \
			((pos < 32)	? HAL_GPIO_WRITE(((*(volatile unsigned long *)HAL_GPIO_DO_REG) & ~(1 << (pos))) | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_WRITE(((*(volatile unsigned long *)HAL_GPIO1_DO_REG) & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_WRITE(((*(volatile unsigned long *)HAL_GPIO2_DO_REG) & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			: ((pos < 128) ? HAL_GPIO3_WRITE(((*(volatile unsigned long *)HAL_GPIO3_DO_REG) & ~(1 << (pos - 96))) | ((val) << (pos - 96))) \
			: HAL_GPIO4_WRITE(((*(volatile unsigned long *)HAL_GPIO4_DO_REG) & ~(1 << (pos - 128))) | ((val) << (pos - 128))))))); \
		} while (0)

#define HAL_GPIO_BIT_GET(pos)			\
			((pos < 32) ? ((HAL_GPIO_READ() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_READ() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_READ() >> (pos - 64)) & 1) \
			: ((pos < 128) ? ((HAL_GPIO3_READ() >> (pos - 96)) & 1) \
			: ((HAL_GPIO4_READ() >> (pos - 128)) & 1)))))

#define HAL_GPIO_BIT_DIR_GET(pos)		\
			((pos < 32) ? ((HAL_GPIO_DIR_GET() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_DIR_GET() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_DIR_GET() >> (pos - 64)) & 1) \
			: ((pos < 128) ? ((HAL_GPIO3_DIR_GET() >> (pos - 96)) & 1) \
			: ((HAL_GPIO4_DIR_GET() >> (pos - 128)) & 1)))))
static struct i2c_gpio_st
{
	u32 mutex_id;
	u16 clock_period;
} *i2c_gpio = NULL;

#define I2C_GPIO_NUM			2
static struct
{
	int sda;
	int scl;
} i2c_gpio_reg[I2C_GPIO_NUM] = {{SYS_I2C_SDA, SYS_I2C_SCL}, {SYS_I2C_SDA, SYS_I2C_SCL}}; 
#define SET_SDA_OUT(id)	HAL_GPIO_BIT_DIR_SET(i2c_gpio_reg[id].sda, HAL_GPIO_O_DIR)
#define SET_SDA_IN(id)	    HAL_GPIO_BIT_DIR_SET(i2c_gpio_reg[id].sda, HAL_GPIO_I_DIR)
#if( EXTERNAL_PULL_HIGH == TRUE)
#define SET_SDA_HI(id)	SET_SDA_IN(id)
#define SET_SDA_LO(id)	SET_SDA_OUT(id);  HAL_GPIO_BIT_SET(i2c_gpio_reg[id].sda, 0)
#else
#define SET_SDA_HI(id)	HAL_GPIO_BIT_SET(i2c_gpio_reg[id].sda, 1)
#define SET_SDA_LO(id)	HAL_GPIO_BIT_SET(i2c_gpio_reg[id].sda, 0)
#endif
#define GET_SDA(id)		HAL_GPIO_BIT_GET(i2c_gpio_reg[id].sda)

#define SET_SCL_OUT(id)	HAL_GPIO_BIT_DIR_SET(i2c_gpio_reg[id].scl, HAL_GPIO_O_DIR)
#define SET_SCL_IN(id)	    HAL_GPIO_BIT_DIR_SET(i2c_gpio_reg[id].scl, HAL_GPIO_I_DIR)
#if( EXTERNAL_PULL_HIGH == TRUE)
#define SET_SCL_HI(id)	SET_SCL_IN(id)
#define SET_SCL_LO(id)	SET_SCL_OUT(id); HAL_GPIO_BIT_SET(i2c_gpio_reg[id].scl, 0)
#else
#define SET_SCL_HI(id)	HAL_GPIO_BIT_SET(i2c_gpio_reg[id].scl, 1)
#define SET_SCL_LO(id)	HAL_GPIO_BIT_SET(i2c_gpio_reg[id].scl, 0)
#endif
#define GET_SCL(id)		HAL_GPIO_BIT_GET(i2c_gpio_reg[id].scl)

#define I2C_TOTAL_TYPE_NUM		3
struct i2c_device
{
	int	(*mode_set)(u32 id, int bps, int en);

	int	(*read)(u32 id, u8 slv_addr, u8 *data, int len);

	int	(*write)(u32 id, u8 slv_addr, u8 *data, int len);

	int	(*write_read)(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);

	int	(*write_read_std)(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);

	int	(*write_write_read)(u32 id, u8 segment_ptr, u8 slv1_addr,u8 slv2_addr, u8 *data, int rlen);
} i2c_dev[I2C_TOTAL_TYPE_NUM];
struct i2c_device_ext
{
       int	(*write_plus_read)(u32 id, u8 slv_addr, u8*data, int wlen, int rlen);
} i2c_dev_ext[I2C_TOTAL_TYPE_NUM];

#define I2C_TYPE_MASK			0xffff0000
#define I2C_ID_MASK				0x0000ffff


struct mutex lock1;

int i2c_gpio_mode_set(u32 id, int bps, int en);
int i2c_gpio_read(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_gpio_write(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_gpio_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
int i2c_gpio_write_write_read(u32 id, u8 segment_ptr, u8 slv1_addr, u8 slv2_addr, u8 *data, int rlen);
int i2c_gpio_write_read_std(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
int i2c_gpio_write_plus_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);


void  i2c_gpio_set(u32 id, u8 sda, u8 scl)
{
	if(id<I2C_GPIO_NUM)
	{
		i2c_gpio_reg[id].sda = sda;
		i2c_gpio_reg[id].scl = scl;
	}
}

int i2c_gpio_attach(int dev_num)
{
	int i;

	if (i2c_gpio)
	{
		kfree(i2c_gpio);
		i2c_gpio = NULL;
	}

	if (dev_num > 0)
	{
		dev_num = (dev_num > I2C_GPIO_NUM) ? I2C_GPIO_NUM : dev_num;
		i2c_gpio = (struct i2c_gpio_st *)kzalloc(sizeof(struct i2c_gpio_st) * dev_num,GFP_KERNEL);
		if (i2c_gpio == NULL)
			return ERR_FAILUE;
		memset(i2c_gpio, 0, sizeof(struct i2c_gpio_st) * dev_num);

		for (i = 0; i < dev_num; i++)
		{
			//i2c_gpio[i].mutex_id = mutex_init(&lock1);
			mutex_init(&lock1);
			i2c_gpio[i].clock_period =  I2C_CLOCK_PERIOD ;    //0;
		}

		i2c_dev[I2C_TYPE_GPIO>>16].mode_set = i2c_gpio_mode_set;
		i2c_dev[I2C_TYPE_GPIO>>16].read = i2c_gpio_read;
		i2c_dev[I2C_TYPE_GPIO>>16].write = i2c_gpio_write;
		i2c_dev[I2C_TYPE_GPIO>>16].write_read = i2c_gpio_write_read;
		i2c_dev[I2C_TYPE_GPIO>>16].write_read_std = i2c_gpio_write_read_std;	
		i2c_dev[I2C_TYPE_GPIO>>16].write_write_read = i2c_gpio_write_write_read;
		i2c_dev_ext[I2C_TYPE_GPIO>>16].write_plus_read = i2c_gpio_write_plus_read;
	}
	return SUCCESS;
}

/* I2C band rate: 1/i2c_gpio[id].clock_period(uS). */

/*---------------------------------------------------
int i2c_gpio_phase_start(id);
	Generate i2c_gpio_phase_start Condition:
	Stream Format:
		SCL   _____/--------\___
		SDA   =---------\_____
		width (4.7u)4.7u|4.7u|
	Arguments:
		NONE
	Return value:
		int SUCCESS				0
		int ERR_I2C_SCL_LOCK	1
		int ERR_I2C_SDA_LOCK	2
----------------------------------------------------*/
static int i2c_gpio_phase_start(u32 id)
{
	/* Make sure is out */
#if( EXTERNAL_PULL_HIGH != TRUE)	
	SET_SDA_OUT(id);
	SET_SCL_OUT(id);
#endif

	SET_SDA_HI(id);		/* Set SDA high */
	if (!GET_SCL(id))
	{        //printk("%s:%d\n",__func__,__LINE__);
		udelay(i2c_gpio[id].clock_period);
	}

	SET_SCL_HI(id);		/* Set SCL high */
	udelay(i2c_gpio[id].clock_period);
	if(!GET_SCL(id))
	{        printk("%s:%d\n",__func__,__LINE__);
		return ERR_I2C_SCL_LOCK;
	}

	if(!GET_SDA(id))
	{        printk("%s:%d\n",__func__,__LINE__);
		return ERR_I2C_SDA_LOCK;
	}

	SET_SDA_LO(id);    //printk("Low!\n");    //mdelay(20000);
	udelay(i2c_gpio[id].clock_period);
	SET_SCL_LO(id);

	return SUCCESS;
}

/*---------------------------------------------------
int i2c_gpio_phase_stop(id);
	Generate i2c_gpio_phase_stop Condition:
	Stream Format:
		SCL   _____/-------------------------------
		SDA   __________/--------------------------
		width  4.7u|4.7u|4.7u from next i2c_gpio_phase_start bit
	Arguments:
		NONE
	Return value:
		int SUCCESS				0
		int ERR_I2C_SCL_LOCK	1
		int ERR_I2C_SDA_LOCK	2
----------------------------------------------------*/
static int i2c_gpio_phase_stop(u32 id)
{
	/* Make sure is out */
#if( EXTERNAL_PULL_HIGH != TRUE)	
	SET_SDA_OUT(id);
	SET_SCL_OUT(id);
#endif
	SET_SDA_LO(id);
	udelay(i2c_gpio[id].clock_period);
	SET_SCL_HI(id);
	udelay(i2c_gpio[id].clock_period);
	if (!GET_SCL(id))
	{
		return ERR_I2C_SCL_LOCK;
	}

	SET_SDA_HI(id);
	mdelay(2);
	if (!GET_SDA(id))
	{
		return ERR_I2C_SDA_LOCK;
	}

	return SUCCESS;
}

/*---------------------------------------------------
void i2c_gpio_phase_set_bit(id, int val);
	Set a BIT (Hi or Low)
	Stream Format:
		SCL   _____/---\
		SDA   ??AAAAAAAA
		width  4.7u| 4u|
	Arguments:
		int i	: Set(1) or Clear(0) this bit on iic bus
	Return value:
		NONE
----------------------------------------------------*/
static void i2c_gpio_phase_set_bit(u32 id, int val)
{
	/* Make sure is out */
#if( EXTERNAL_PULL_HIGH != TRUE)	
	SET_SDA_OUT(id);
	SET_SCL_OUT(id);
#endif
	SET_SCL_LO(id);
	if(val)
	{
		SET_SDA_HI(id);
	}
	else
	{
		SET_SDA_LO(id);
	}
	udelay(i2c_gpio[id].clock_period);

	SET_SCL_HI(id);
	udelay(i2c_gpio[id].clock_period);
	SET_SCL_LO(id);

	return;
}

/*---------------------------------------------------
int i2c_gpio_phase_get_bit(id);
	Set a BIT (Hi or Low)
	Stream Format:
		SCL   _____/---\
		SDA   ??AAAAAAAA
		width  4.7u| 4u|
	Arguments:
		NONE
	Return value:
		int i	: Set(1) or Clear(0) this bit on iic bus
----------------------------------------------------*/
static int i2c_gpio_phase_get_bit(u32 id)
{
	int ret = 0;

	SET_SDA_IN(id);

	SET_SDA_HI(id);				/* Hi Ind */
	udelay(i2c_gpio[id].clock_period);
	SET_SCL_HI(id);
	udelay(i2c_gpio[id].clock_period);
	ret = GET_SDA(id);
	SET_SCL_LO(id);

	return ret;
}

/*---------------------------------------------------
int i2c_gpio_phase_set_byte(u32 id, u8 data);
	Perform a byte write process
	Stream Format:
		SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\__/-\
		SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-Check
		Clock Low: 4.7u, High: 4.0u.                            Ack
    	Data exchanged at CLK Low, ready at SCL High
	Arguments:
		char data	- Data to send on iic bus
	return value:
		The /ack signal returned from slave
----------------------------------------------------*/
static int i2c_gpio_phase_set_byte(u32 id, u8 data)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		if (data & 0x80)
		{
			i2c_gpio_phase_set_bit(id, 1);
		}
		else
		{
			i2c_gpio_phase_set_bit(id, 0);
		}

		data <<= 1;
	}

	return(i2c_gpio_phase_get_bit(id));
}

/*---------------------------------------------------
char i2c_gpio_phase_get_byte(u32 id, int ack);
	Perform a byte read process
			by Charlemagne Yue
	SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\
	SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-(Ack)
	Clock Low: 4.7u, High: 4.0u.
    Data exchanged at CLK Low, ready at SCL High
----------------------------------------------------*/
static u8 i2c_gpio_phase_get_byte(u32 id, int ack)
{
	u8 ret = 0;
	int i;

	for (i = 0; i < 8; i++)
	{
		ret <<= 1;
		ret |= i2c_gpio_phase_get_bit(id);
	}
	i2c_gpio_phase_set_bit(id, ack);

	return ret;
}


/*---------------------------------------------------
int i2c_gpio_read_no_stop(u32 id, u8 slv_addr, u8 *data, u32 len);
	Perform bytes read process but no stop
	Stream Format:
		S<SLV_R><Read>
		S		: Start
		<SLV_R>	: Set Slave addr & Read Mode
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE reg_addr - Data address
	Return value:
		Data returned
----------------------------------------------------*/
static int i2c_gpio_read_no_stop(u32 id, u8 slv_addr, u8 *data, int len)
{
	int i = I2C_GPIO_TIMES_OUT;

	slv_addr |= 1;						/* Read */
	while (--i)							/* Ack polling !! */
	{
		i2c_gpio_phase_start(id);
		/* has /ACK => i2c_gpio_phase_start transfer */
		if(!i2c_gpio_phase_set_byte(id, slv_addr))
		{
			break;
		}
		/* device is busy, issue i2c_gpio_phase_stop and chack again later */
		i2c_gpio_phase_stop(id);
		mdelay(1); 			/* wait for 1mS */
	}

	if (i == 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = 0; i < (len - 1); i++)
	{
		/*with no /ack to stop process */
		data[i] = i2c_gpio_phase_get_byte(id, 0);
	}
	data[len - 1] = i2c_gpio_phase_get_byte(id, 1);

	return SUCCESS;
}

/*---------------------------------------------------
int i2c_gpio_write_no_stop(u32 id, u8 slv_addr, u8 *data, u32 len);
	Perform bytes write process but no stop
	Stream Format:
		S<SLV_W><Write>
		S		: Start
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write
	Return value:
		NONE
----------------------------------------------------*/
int i2c_gpio_write_no_stop(u32 id, u8 slv_addr, u8 *data, int len)
{
	int i = I2C_GPIO_TIMES_OUT;

	slv_addr &= 0xFE;					/*Write*/
	while (--i)							/* Ack polling !! */
	{
		i2c_gpio_phase_start(id);
		/* has /ACK => i2c_gpio_phase_start transfer */
		if(!i2c_gpio_phase_set_byte(id, slv_addr))
		{
			break;
		}
		/* device is busy, issue i2c_gpio_phase_stop and chack again later */
		i2c_gpio_phase_stop(id);
		mdelay(1); 			/* wait for 1mS */
	}

	if (i == 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = 0; i < len; i++){
		i2c_gpio_phase_set_byte(id, data[i]);
	}

	return SUCCESS;
}

static int i2c_gpio_write_no_stop_no_ack(u32 id, u8 slv_addr, u8 *data, int len)
{
	int i = I2C_GPIO_TIMES_OUT;

	slv_addr &= 0xFE;					/*Write*/
//	while (--i)							/* Ack polling !! */
//	{
		i2c_gpio_phase_start(id);
		/* has /ACK => i2c_gpio_phase_start transfer */
//		if(!i2c_gpio_phase_set_byte(id, slv_addr))
		i2c_gpio_phase_set_byte(id, slv_addr);
		{
//			break;
		}
		/* device is busy, issue i2c_gpio_phase_stop and chack again later */
		//i2c_gpio_phase_stop(id);
		//msleep(1); 			/* wait for 1mS */
//	}

	//if (i == 0)    //if(I2C_GPIO_TIMES_OUT != i)
	//{
		//return ERR_TIME_OUT;
	//}

	for (i = 0; i < len; i++){
		i2c_gpio_phase_set_byte(id, *data);
	}

	return SUCCESS;
}
/*---------------------------------------------------
int i2c_gpio_mode_set(int bps, int en);
----------------------------------------------------*/
int i2c_gpio_mode_set(u32 id, int bps, int en)
{
	if (bps <= 0)
	{
		i2c_gpio[id].clock_period = 5;	/* 100K bps */
		return ERR_FAILUE;
	}
	i2c_gpio[id].clock_period = 500000 / bps;
	SET_SDA_HI(id);
	SET_SCL_HI(id);
#if( EXTERNAL_PULL_HIGH != TRUE)
	if (en)
	{
		SET_SCL_OUT(id);
	} else
	{
		SET_SCL_IN(id);
	}
#endif
	return SUCCESS;
}

/*---------------------------------------------------
int i2c_gpio_read(u32 id, u8 slv_addr, u8 *data, u32 len);
	Perform a byte read process
	Stream Format:
		S<SLV_R><Read>P
		S		: Start
		P		: Stop
		<SLV_R>	: Set Slave addr & Read Mode
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE reg_addr - Data address
	Return value:
		Data returned
----------------------------------------------------*/
int i2c_gpio_read(u32 id, u8 slv_addr, u8 *data, int len)
{
	int ret;

	mutex_lock(&lock1);
	if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, len)) != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}
	i2c_gpio_phase_stop(id);
	mutex_unlock(&lock1);

	return SUCCESS;
}

/*---------------------------------------------------
int i2c_gpio_write(u8 slv_addr, u8 *data, u32 len);
	Perform bytes write process
	Stream Format:
		S<SLV_W><Write>P
		S		: Start
		P		: Stop
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write
	Return value:
		NONE
----------------------------------------------------*/
int i2c_gpio_write(u32 id, u8 slv_addr, u8 *data, int len)
{
	int ret;

	mutex_lock(&lock1);
	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, len)) != SUCCESS)
	{        printk("%s:%d i2c_gpio_write_no_stop,error!\n",__func__,__LINE__);
		mutex_unlock(&lock1);
		return ret;
	}    //printk("%s:%d\n",__func__,__LINE__);
	i2c_gpio_phase_stop(id);
	mutex_unlock(&lock1);

	return SUCCESS;
}

/*---------------------------------------------------
int i2c_gpio_write_read(u8 slv_addr, u8 *data, u32 wlen, u32 rlen);
	Perform bytes write-read process
	Stream Format:
		S<SLV_W><Write>S<SLV_W><Read>
		S		: Start
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
int i2c_gpio_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{
	int  ret;

	if (wlen == 0)
	{
		return i2c_gpio_read(id, slv_addr, data, rlen);
	}

	mutex_lock(&lock1);
	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen)) != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}
#ifdef SPECIAL_I2C_REQUEST
	if (id == 2)
	{
		for (i = 0; i < (rlen - 1); i++)
		{
			/*with ack to stop process */
			data[i] = i2c_gpio_phase_get_byte(id, 0);
		}
		data[rlen - 1] = i2c_gpio_phase_get_byte(id, 1);

	}	
	else
#endif		
	{
		if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen)) != SUCCESS)
		{
			mutex_unlock(&lock1);
			return ret;
		}
	}
	i2c_gpio_phase_stop(id);
	mutex_unlock(&lock1);

	return SUCCESS;
}

/*---------------------------------------------------
int i2c_gpio_write_read(u8 slv_addr, u8 *data, u32 wlen, u32 rlen);
	Perform bytes write-read process
	Stream Format:
		S<SLV_W><Write>S<SLV_W><Read>
		S		: Start
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
int i2c_gpio_mbyte_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{
	int ret;
	u8 msection[2];

	if (wlen == 0)
	{
		return i2c_gpio_read(id, slv_addr, data, rlen);
	}

	mutex_lock(&lock1);
	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen)) != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}	

	msection[0]=0xFE;		
	msection[1]=0xC0|0x1;

	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, msection, 2)) != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}	
	
	//{
		if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen)) != SUCCESS)
		{
			mutex_unlock(&lock1);
			return ret;
		}
	//}
	i2c_gpio_phase_stop(id);
	mutex_unlock(&lock1);

	return SUCCESS;
}


/*---------------------------------------------------
int i2c_gpio_write_write_read(u8 slv_addr, u8 *data, u32 wlen, u32 rlen);
	Perform bytes write-write-read process, specailly for E-DDC
	Stream Format:
		S<SLV_W><Write>S<SLV_W><Read>
		S		: Start
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
int i2c_gpio_write_write_read(u32 id,u8 segment_ptr, u8 slv1_addr,u8 slv2_addr, u8 *data, int rlen)
{
	int ret;

	mutex_lock(&lock1);
#if 0
	if ((ret = i2c_gpio_write_no_stop_no_ack(id, slv1_addr, &segment_ptr, 1)) != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}
#else
	i2c_gpio_write_no_stop_no_ack(id, slv1_addr, &segment_ptr, 1);
#endif
	if ((ret = i2c_gpio_write_no_stop(id, slv2_addr, data, 1)) != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}

	if ((ret = i2c_gpio_read_no_stop(id, slv2_addr, data, rlen)) != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}
	i2c_gpio_phase_stop(id);
	mutex_unlock(&lock1);

	return SUCCESS;
}

int i2c_gpio_write_read_std(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{
	int ret;

	if (wlen == 0)
	{
		return i2c_gpio_read(id, slv_addr, data, rlen);
	}
	mutex_lock(&lock1);
	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen)) != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}
	i2c_gpio_phase_stop(id);
	if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen)) != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}
	i2c_gpio_phase_stop(id);
	mutex_unlock(&lock1);

	return SUCCESS;
}

int i2c_gpio_write_plus_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{	
	int ret;
	
	mutex_lock(&lock1);
	ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen);
	if(ret != SUCCESS)
	{
		mutex_unlock(&lock1);
		return ret;
	}
	ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen);
	mutex_unlock(&lock1);

	return ret;
}	


int i2c_mode_set(u32 id, int bps, int en)
{
	return i2c_dev[(id&I2C_TYPE_MASK)>>16].mode_set((id&I2C_ID_MASK), bps, en);
}


int i2c_read(u32 id, u8 slv_addr, u8 *data, int len)
{
	return i2c_dev[(id&I2C_TYPE_MASK)>>16].read((id&I2C_ID_MASK), slv_addr, data, len);
}


int i2c_write(u32 id, u8 slv_addr, u8 *data, int len)
{
	return i2c_dev[(id&I2C_TYPE_MASK)>>16].write((id&I2C_ID_MASK), slv_addr, data, len);
}

int i2c_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen) {
	return i2c_dev[(id&I2C_TYPE_MASK)>>16].write_read((id&I2C_ID_MASK), slv_addr, data, wlen , rlen);    
}



