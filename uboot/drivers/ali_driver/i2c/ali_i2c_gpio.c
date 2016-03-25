/*
* ALi I2C GPIO Driver.

*/
#include <common.h>
#include <ali/sys_define.h>
#include <asm-generic/gpio.h>
#include <asm/io.h>
#include <i2c.h>
#include <errno.h>

#define I2C_ERROR_BASE		-200

#define ERR_I2C_SCL_LOCK	(I2C_ERROR_BASE - 1)	/* I2C SCL be locked */
#define ERR_I2C_SDA_LOCK	(I2C_ERROR_BASE - 2)	/* I2C SDA be locked */
#define ERR_I2C_NO_ACK		(I2C_ERROR_BASE - 3)	/* I2C slave no ack */
#define SUCCESS 0
#define ERR_FAILUE		-9

#define ERR_TIME_OUT    -34     /* Waiting time out */
//#define TRUE    (1)
//#define FALSE    (0)

//#define EXTERNAL_PULL_HIGH		FALSE//TRUE    // for I2C_gpio.c
#define I2C_GPIO_NUM			1

#define I2C_GPIO_TIMES_OUT		10
static struct i2c_gpio_st
{
	u16 clock_period;
} i2c_gpio[I2C_GPIO_NUM];

// M3616/M3713 panel I2C GPIO
#define M3616_SYS_I2C_SDA1    68
#define M3616_SYS_I2C_SCL1    69

// M3516/M3715 panel I2C GPIO
#define M3516_SYS_I2C_SDA1    134
#define M3516_SYS_I2C_SCL1    135


#define M3921_SYS_I2C_SDA1    12
#define M3921_SYS_I2C_SCL1    4

static struct
{
	int sda;
	int scl;
} i2c_gpio_reg[I2C_GPIO_NUM] = {{M3616_SYS_I2C_SDA1, M3616_SYS_I2C_SCL1}};	//cmchen


#define SET_SDA_OUT(id)  i2c_gpio_set_sda_out(id)
#define SET_SDA_IN(id)   i2c_gpio_set_sda_in(id)
#if (EXTERNAL_PULL_HIGH == TRUE)
  #define SET_SDA_HI(id)	SET_SDA_IN(id)
  #define SET_SDA_LO(id)	SET_SDA_OUT(id);  gpio_set_value(i2c_gpio_reg[id].sda, 0)
#else
  #define SET_SDA_HI(id)	gpio_set_value(i2c_gpio_reg[id].sda, 1)
  #define SET_SDA_LO(id)	gpio_set_value(i2c_gpio_reg[id].sda, 0)
#endif
#define GET_SDA(id)		gpio_get_value(i2c_gpio_reg[id].sda)

#define SET_SCL_OUT(id)	i2c_gpio_set_scl_out(id);//i2c_gpio_set_scl_out(i)//HAL_GPIO_BIT_DIR_SET(i2c_gpio_reg[id].scl, HAL_GPIO_O_DIR)
#define SET_SCL_IN(id)	i2c_gpio_set_scl_in(id);
#if (EXTERNAL_PULL_HIGH == TRUE)
  #define SET_SCL_HI(id)	SET_SCL_IN(id)
  #define SET_SCL_LO(id)	SET_SCL_OUT(id); gpio_set_value(i2c_gpio_reg[id].scl, 0)
#else
  #define SET_SCL_HI(id)	gpio_set_value(i2c_gpio_reg[id].scl, 1)
  #define SET_SCL_LO(id)	gpio_set_value(i2c_gpio_reg[id].scl, 0)
#endif
#define GET_SCL(id)		gpio_get_value(i2c_gpio_reg[id].scl)

int i2c_gpio_mode_set(u32 id, int bps, int en);
int i2c_gpio_read(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_gpio_write(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_gpio_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
int i2c_gpio_write_write_read(u32 id, u8 segment_ptr, u8 slv1_addr, u8 slv2_addr, u8 *data, int rlen);
int i2c_gpio_write_read_std(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
int i2c_gpio_write_plus_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
int i2c_gpio_init(int speed, int slaveaddr);

static unsigned int m_chip_id;
static unsigned int m_chip_rev;

void i2c_gpio_set_scl_out(u32 id)
{
	if((id == 0)&&(ALI_C3701==sys_ic_get_chip_id()))
    {
        gpio_direction_input(i2c_gpio_reg[id].scl);
	}
	else
	{
        gpio_direction_output(i2c_gpio_reg[id].scl, 2);
	}
}

void i2c_gpio_set_sda_out(u32 id)
{
	if((id == 0)&&(ALI_C3701==sys_ic_get_chip_id()))
    {
        gpio_direction_input(i2c_gpio_reg[id].sda);
	}
	else
	{
        gpio_direction_output(i2c_gpio_reg[id].sda, 2);
	}
}
void i2c_gpio_set_scl_in(u32 id)
{
	if((id == 0)&&(ALI_C3701==sys_ic_get_chip_id()))
    {
        gpio_direction_output(i2c_gpio_reg[id].scl, 2);
	}
	else
	{
        gpio_direction_input(i2c_gpio_reg[id].scl);
	}
}

void i2c_gpio_set_sda_in(u32 id)
{
	if((id == 0)&&(ALI_C3701==sys_ic_get_chip_id()))
    {
        gpio_direction_output(i2c_gpio_reg[id].sda, 2);
	}
	else
	{
        gpio_direction_input(i2c_gpio_reg[id].sda);
	}
}


void  i2c_gpio_set(u32 id, u8 sda, u8 scl)
{
	if(id<I2C_GPIO_NUM)
	{
		i2c_gpio_reg[id].sda = sda;
		i2c_gpio_reg[id].scl = scl;
	}
}

int i2c_gpio_init(int speed, int slaveaddr)
{
    u8 sda[I2C_GPIO_NUM];// = 0;
    u8 scl[I2C_GPIO_NUM];// = 0;

    if (slaveaddr >= I2C_GPIO_NUM)
    {
        return -1;
    }

    m_chip_id = sys_ic_get_chip_id();
    m_chip_rev = sys_ic_get_rev_id();

    if (m_chip_id == ALI_C3701) //BOARD_M3701C_DEMO
    {
        sda[0] = M3616_SYS_I2C_SDA1;
        scl[0] = M3616_SYS_I2C_SCL1;
    }
    else if(m_chip_id == ALI_S3503)
    {
        sda[0] = M3516_SYS_I2C_SDA1;
        scl[0] = M3516_SYS_I2C_SCL1;
    }
    else if(m_chip_id == ALI_S3921)
    {
        sda[0] = M3921_SYS_I2C_SDA1;
        scl[0] = M3921_SYS_I2C_SCL1;
    }

    if (m_chip_id == ALI_C3701)
    {
        gpio_direction_input(sda[slaveaddr]);
        gpio_set_value(sda[slaveaddr], 1);
        gpio_direction_input(scl[slaveaddr]);
        gpio_set_value(scl[slaveaddr], 1);
    }
    else
    {
        gpio_direction_output(sda[slaveaddr], 1);
        gpio_direction_output(scl[slaveaddr], 1);
    }

    i2c_gpio_set(slaveaddr, sda[slaveaddr], scl[slaveaddr]);
	i2c_gpio[slaveaddr].clock_period = 0;

    //这个配置可以减轻linux CH455 panel 有时跳键问题
    i2c_gpio_mode_set(slaveaddr, speed, 1);

	return 0;
}


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
static u32 i2c_gpio_phase_start(u32 id)
{
	/* Make sure is out */
#if( EXTERNAL_PULL_HIGH != TRUE)	
	SET_SDA_OUT(id);
	SET_SCL_OUT(id);
#endif

	SET_SDA_HI(id);		/* Set SDA high */
	if (!GET_SCL(id))
	{
		__udelay(i2c_gpio[id].clock_period);
	}

	SET_SCL_HI(id);		/* Set SCL high */
	__udelay(i2c_gpio[id].clock_period);
	if(!GET_SCL(id))
	{
		return ERR_I2C_SCL_LOCK;
	}

	if(!GET_SDA(id))
	{
		return ERR_I2C_SDA_LOCK;
	}
	SET_SDA_LO(id);
	__udelay(i2c_gpio[id].clock_period);
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
	__udelay(i2c_gpio[id].clock_period);
	SET_SCL_HI(id);
	__udelay(i2c_gpio[id].clock_period);
	if (!GET_SCL(id))
	{
		return ERR_I2C_SCL_LOCK;
	}

	SET_SDA_HI(id);
	__udelay(2);//__udelay(2);

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
	__udelay(i2c_gpio[id].clock_period);

	SET_SCL_HI(id);
	__udelay(i2c_gpio[id].clock_period);
	SET_SCL_LO(id);

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
	__udelay(i2c_gpio[id].clock_period);
	SET_SCL_HI(id);
	__udelay(i2c_gpio[id].clock_period);

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
		//osal_task_sleep(1); 			/* wait for 1mS */
        mdelay(1);
        //M_SLEEP(1);
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
		//osal_task_sleep(1); 			/* wait for 1mS */
        mdelay(1);
        //M_SLEEP(1);
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
		//osal_task_sleep(1); 			/* wait for 1mS */
//	}

	if (i == 0)
	{
		return ERR_TIME_OUT;
	}

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

	if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, len)) != SUCCESS)
	{
		return ret;
	}
	i2c_gpio_phase_stop(id);

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

	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, len)) != SUCCESS)
	{
		return ret;
	}
	i2c_gpio_phase_stop(id);

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
	int ret;

	if (wlen == 0)
	{
		return i2c_gpio_read(id, slv_addr, data, rlen);
	}

	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen)) != SUCCESS)
	{
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
			return ret;
		}
	}
	i2c_gpio_phase_stop(id);

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

	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen)) != SUCCESS)
	{
		return ret;
	}	

	msection[0]=0xFE;		
	msection[1]=0xC0|0x1;

	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, msection, 2)) != SUCCESS)
	{
		return ret;
	}	
	
	//{
		if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen)) != SUCCESS)
		{
			return ret;
		}
	//}
	i2c_gpio_phase_stop(id);

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

	if ((ret = i2c_gpio_write_no_stop_no_ack(id, slv1_addr, &segment_ptr, 1)) != SUCCESS)
	{
		return ret;
	}

	if ((ret = i2c_gpio_write_no_stop(id, slv2_addr, data, 1)) != SUCCESS)
	{
		return ret;
	}

	if ((ret = i2c_gpio_read_no_stop(id, slv2_addr, data, rlen)) != SUCCESS)
	{
		return ret;
	}
	i2c_gpio_phase_stop(id);

	return SUCCESS;
}

int i2c_gpio_write_read_std(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{
	int ret;

	if (wlen == 0)
	{
		return i2c_gpio_read(id, slv_addr, data, rlen);
	}
	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen)) != SUCCESS)
	{
		return ret;
	}
	i2c_gpio_phase_stop(id);
	if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen)) != SUCCESS)
	{
		return ret;
	}
	i2c_gpio_phase_stop(id);

	return SUCCESS;
}

int i2c_gpio_write_plus_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{	
	int ret;

	ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen);
	if(ret != SUCCESS)
	{
		return ret;
	}
	ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen);

	return ret;
}

