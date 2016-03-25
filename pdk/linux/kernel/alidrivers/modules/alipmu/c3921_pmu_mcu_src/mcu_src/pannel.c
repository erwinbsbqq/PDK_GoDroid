#include <dp8051xp.h>
#include <stdio.h>
#include <intrins.h>
#include "sys.h"
#include "pannel.h"
#include "uart.h"

//=====================================================================================//
#if PMU_MCU_M3503
#define PANNEL_KEY                                                          0x4
#else
	#if PMU_MCU_M3821
		#define PANNEL_KEY                                            0x44
	#else
		#define PANNEL_KEY                                            0x47
	#endif
#endif

#define EXTERNAL_PULL_HIGH                                           TRUE
#define SETING_ADDR	                                                  0x48
#define DIG0_ADDR	                                                        0x68
#define DIG1_ADDR	                                                        0x6a
#define DIG2_ADDR	                                                        0x6c
#define DIG3_ADDR	                                                        0x6e
#define KEY_ADDR	                                                        0x4f

#define ERR_I2C_SCL_LOCK                                               1
#define ERR_I2C_SDA_LOCK                                              1
#define I2C_GPIO_TIMES_OUT		                                  10
#define ERR_TIME_OUT                                                     -34 /* Waiting time out */
#define ERR_FAILURE	       	                                         -9 /* Common error, operation not success */

#ifdef PMU_MCU_M3821
#define SYS_I2C_SDA                                                        XPMU_GPIO_1
#define SYS_I2C_SCL                                                         XPMU_GPIO_0
#else
	#ifdef PMU_MCU_M3503
		#define SYS_I2C_SDA                                          XPMU_GPIO_0
		#define SYS_I2C_SCL                                           XPMU_GPIO_1
	#else
		#define SYS_I2C_SDA                                          XPMU_GPIO_0
		#define SYS_I2C_SCL                                           XPMU_GPIO1_0
	#endif
#endif

//=====================================================================================//
static void  set_sda_out(void)
{
	hal_gpio_bit_dir_set(SYS_I2C_SDA, HAL_GPIO_O_DIR);
}

static void set_sda_in(void)
{
	hal_gpio_bit_dir_set(SYS_I2C_SDA, HAL_GPIO_I_DIR);
}

#ifdef EXTERNAL_PULL_HIGH 
static void set_sda_hi(void)
{
	set_sda_in();
}

static void set_sda_lo(void)
{
	set_sda_out();
	hal_gpio_bit_set(SYS_I2C_SDA, HAL_GPIO_SER_LOW);
}
#else
static void set_sda_hi(void)
{
	hal_gpio_bit_set(SYS_I2C_SDA, HAL_GPIO_SET_HI);
}

static void set_sda_lo(void)
{
	hal_gpio_bit_set(SYS_I2C_SDA, HAL_GPIO_SER_LOW);
}
#endif

static void set_scl_out(void)
{
	hal_gpio_bit_dir_set(SYS_I2C_SCL, HAL_GPIO_O_DIR);
}

static void set_scl_in(void)
{
	hal_gpio_bit_dir_set(SYS_I2C_SCL, HAL_GPIO_I_DIR);
}

#ifdef EXTERNAL_PULL_HIGH
static void set_scl_hi(void)
{
	set_scl_in();
}

static void set_scl_lo(void)
{
	set_scl_out();
	hal_gpio_bit_set(SYS_I2C_SCL, HAL_GPIO_SER_LOW);
}
#else
static void set_scl_hi(void)
{
	hal_gpio_bit_set(SYS_I2C_SCL, HAL_GPIO_SET_HI);
}

static void set_scl_lo(void)
{
      hal_gpio_bit_set(SYS_I2C_SCL, HAL_GPIO_SER_LOW);
}
#endif
static UINT8  get_scl(void)
{
	UINT8 ret = 0;
	
	ret = hal_gpio_bit_get(SYS_I2C_SCL);
	return ret;
}

static UINT8  get_sda(void)
{
	UINT8 ret = 0;
	
	ret = hal_gpio_bit_get(SYS_I2C_SDA);
	return ret;
}

void OneNop()
{
	_nop_();_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();_nop_();
}

void delay_2us()
{
	_nop_();_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();_nop_();
}

static void  delay_ms( UINT8 ms )
{
	UINT16 i = 0;

	while ( ms -- )
	{
		for( i = 0; i != 300; i++ );
	}
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
static UINT8 i2c_gpio_phase_start(void)
{
#ifdef EXTERNAL_PULL_HIGH 
	set_sda_out();
	set_scl_out();
#endif

	set_sda_hi();
	if(!get_scl())
	{
		OneNop();
	}
	
	set_scl_hi();
	OneNop();
	if(!get_scl())
	{
		return ERR_I2C_SCL_LOCK;
	}

	if(!get_sda())
	{
		return ERR_I2C_SDA_LOCK;
	}

	set_sda_lo();
	OneNop();
	set_scl_lo();
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
static INT8 i2c_gpio_phase_stop(void)
{
#ifdef EXTERNAL_PULL_HIGH 
	set_sda_out();
	set_scl_out();
#endif

	set_sda_lo();
	OneNop();
	set_scl_hi();
	OneNop();
	if(!get_scl())
	{
		return ERR_I2C_SCL_LOCK;
	}

	/*Send I2C bus transfer end signal.*/
	set_sda_hi();
	delay_2us();
	OneNop();

	if(!get_sda())
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
static void i2c_gpio_phase_set_bit(int val)
{
#ifdef EXTERNAL_PULL_HIGH 
	set_sda_out();
	set_scl_out();
#endif

	set_scl_lo();
	if(val)
	{
		set_sda_hi();
	}
	else
	{
		set_sda_lo();
	}
	
	OneNop();
	set_scl_hi();
	OneNop();
	set_scl_lo();
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
static INT8 i2c_gpio_phase_get_bit(void)
{
	int ret = 0;

	set_sda_in();

	/* Hi Ind */
	set_sda_hi();
	OneNop();

	set_scl_hi();
	OneNop();

	ret = get_sda();
	set_scl_lo();

	return ret;
}

/*---------------------------------------------------
int i2c_gpio_phase_set_byte(UINT32 id, UINT8 data);
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
static UINT8 i2c_gpio_phase_set_byte(UINT8 dat)
{
	UINT8 i = 0;

	for (i = 0; i < 8; i++)
	{
		if (dat & 0x80)
		{
			i2c_gpio_phase_set_bit(1);
		}
		else
		{
	   	    i2c_gpio_phase_set_bit(0);
		}

		dat <<= 1;
	}
	
	return(i2c_gpio_phase_get_bit());
}

/*---------------------------------------------------
char i2c_gpio_phase_get_byte(UINT32 id, int ack);
	Perform a byte read process
			by Charlemagne Yue
	SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\
	SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-(Ack)
	Clock Low: 4.7u, High: 4.0u.
    Data exchanged at CLK Low, ready at SCL High
----------------------------------------------------*/
static UINT8 i2c_gpio_phase_get_byte( int ack)
{
	UINT8 ret = 0;
	UINT8 i = 0;

	for (i = 0; i < 8; i++)
	{
		ret <<= 1;
		ret |= i2c_gpio_phase_get_bit();
	}
	
	i2c_gpio_phase_set_bit(ack);

	return ret;
}

/*---------------------------------------------------
INT32 i2c_gpio_read_no_stop(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
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
static INT8 i2c_gpio_read_no_stop( UINT8 slv_addr, UINT8 *dat, int len)
{
	UINT8 i = I2C_GPIO_TIMES_OUT;

	slv_addr |= 1;/* Read */
	while (--i)/* Ack polling !! */
	{
		i2c_gpio_phase_start();
		/* has /ACK => i2c_gpio_phase_start transfer */
		if(!i2c_gpio_phase_set_byte( slv_addr))
		{
			break;
		}
		
		/* device is busy, issue i2c_gpio_phase_stop and chack again later */
		i2c_gpio_phase_stop();
		delay_ms(1);	/* wait for 1mS */
	}
	
	if (i == 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = 0; i < (len - 1); i++)
	{
		/*with no /ack to stop process */
		dat[i] = i2c_gpio_phase_get_byte( 0);
	}
	
	dat[len - 1] = i2c_gpio_phase_get_byte(1);
	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_write_no_stop(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
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
static INT8 i2c_gpio_write_no_stop( UINT8 slv_addr, UINT8 *dat, int len)
{
	UINT8 i = I2C_GPIO_TIMES_OUT;

	slv_addr &= 0xFE;					/*Write*/
	while (--i)							/* Ack polling !! */
	{
		i2c_gpio_phase_start();
		/* has /ACK => i2c_gpio_phase_start transfer */
		if(!i2c_gpio_phase_set_byte(slv_addr))
		{
			//PMU_PRINTF("i2c_gpio_write_no_stop Ack polling OK!\n");
			break;
		}
		
		/* device is busy, issue i2c_gpio_phase_stop and chack again later */
		i2c_gpio_phase_stop();
		delay_ms(1);/* wait for 1mS */
		//PMU_PRINTF("i2c_gpio_write_no_stop Ack polling !\n");
	}

	if (i == 0)
	{
		//PMU_PRINTF("i2c_gpio_write_no_stop err ERR_TIME_OUT!\n");
		return ERR_TIME_OUT;
	}

	for (i = 0; i < len; i++)
	{
		i2c_gpio_phase_set_byte(dat[i]);
	}

	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_read(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
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
static UINT8 i2c_gpio_read(UINT8 slv_addr, UINT8 *dat, int len)
{
	INT8 ret = 0;

	if(SUCCESS != i2c_gpio_read_no_stop(slv_addr, dat, len))
	{
		return ret;
	}
	
	i2c_gpio_phase_stop();
	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_write(UINT8 slv_addr, UINT8 *data, UINT32 len);
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
static INT8 i2c_gpio_write( UINT8 slv_addr, UINT8 *dat, int len)
{
	INT8 ret = 0;
    
	if ((ret = i2c_gpio_write_no_stop(slv_addr, dat, len)) != SUCCESS)
	{
		return ret;
	}
    
	i2c_gpio_phase_stop();
    
    return SUCCESS;
}

 //    ch455 led map
//	    /* Let's put the dot bitmap into the table */
//		{'.', 0x80}, 
//		{'0', 0x3f}, {'1', 0x06}, {'2', 0x5b}, {'3', 0x4f}, 
//		{'4', 0x66}, {'5', 0x6d}, {'6', 0x7d}, {'7', 0x07}, 
//		{'8', 0x7f}, {'9', 0x6f}, {'a', 0x77}, {'A', 0x77}, 
//		{'b', 0x7c}, {'B', 0x7c}, {'c', 0x39}, {'C', 0x39}, 
//		{'d', 0x5e}, {'D', 0x5e}, {'e', 0x79}, {'E', 0x79}, 
//		{'f', 0x71}, {'F', 0x71}, {'g', 0x6f}, {'G', 0x3d}, 
//		{'h', 0x76}, {'H', 0x76}, {'i', 0x04}, {'I', 0x30}, 
//		{'j', 0x0e}, {'J', 0x0e}, {'l', 0x38}, {'L', 0x38}, 
//		{'n', 0x54}, {'N', 0x37}, {'o', 0x5c}, {'O', 0x3f}, 
//		{'p', 0x73}, {'P', 0x73}, {'q', 0x67}, {'Q', 0x67}, 
//		{'r', 0x50}, {'R', 0x77}, {'s', 0x6d}, {'S', 0x6d}, 
//		{'t', 0x78}, {'T', 0x31}, {'u', 0x3e}, {'U', 0x3e}, 
//		{'y', 0x6e}, {'Y', 0x6e}, {'z', 0x5b}, {'Z', 0x5b}, 
//		{':', 0x80}, {'-', 0x40}, {'_', 0x08}, {' ', 0x00}

static void show_off(void)
{
	UINT8 led_map[4]={0x00, 0x3f, 0x71, 0x80};

	i2c_gpio_write(DIG3_ADDR, &led_map[2], 1); //F
	i2c_gpio_write(DIG2_ADDR, &led_map[2], 1); //F
	i2c_gpio_write(DIG1_ADDR, &led_map[1], 1); //O
	i2c_gpio_write(DIG0_ADDR, &led_map[0], 1); //
}

static void show_bank(void)
{
	UINT8 led_map[4]={0x3f, 0x71, 0x00, 0x5c};

	i2c_gpio_write(DIG0_ADDR, &led_map[2], 1);
	i2c_gpio_write(DIG1_ADDR, &led_map[2], 1);
	i2c_gpio_write(DIG2_ADDR, &led_map[2], 1);
	i2c_gpio_write(DIG3_ADDR, &led_map[2], 1);
}

static void show_time(pRTC_TIMER rtc)
{
	UINT8  min_ge = 0;
	UINT8  min_shi = 0;
	UINT8  hour_ge = 0;
	UINT8  hour_shi = 0;
	UINT8  show_colon = 0;

	/*pan ch455 1,2 3...9 ,:*/
	UINT8 led_map[10]={0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};

	min_ge  = rtc->min%10;
	min_shi = rtc->min/10;
	hour_ge = rtc->hour%10;
	hour_shi= rtc->hour/10;
	show_colon = led_map[hour_ge]|0x80; //show : code

	i2c_gpio_write(DIG0_ADDR, &led_map[hour_shi], 1);
	i2c_gpio_write(DIG1_ADDR, &led_map[hour_ge], 1);
    
	if(rtc->sec%2 == 0)
	{
		i2c_gpio_write(DIG1_ADDR, &show_colon, 1);// show :
	}
        
	i2c_gpio_write(DIG2_ADDR, &led_map[min_shi], 1);
	i2c_gpio_write(DIG3_ADDR, &led_map[min_ge], 1);
}  

void pannel_init(void)
{
	hal_mcu_gpio_en(SYS_I2C_SDA);
	hal_mcu_gpio_en(SYS_I2C_SCL);
}

void show_pannel(enum SHOW_TYPE show_type, pRTC_TIMER rtc)
{    
	if(show_type == SHOW_BANK)
	{
		show_bank();
	}
	else if(show_type == SHOW_TIME)
	{
		show_time(rtc);
	}
	else
	{
		show_off();
	}
}

INT8 pannel_scan(void)
{
	UINT8 pan_key = 0;
	i2c_gpio_read(KEY_ADDR, &pan_key, 1);

#if 0//Add only for debug.
	if((0x0 != pan_key) && (PANNEL_KEY != pan_key))
	{
		PMU_WRITE_BYTE(0x3f00, pan_key);
		PMU_WRITE_BYTE(0x3f01, pan_key);
		PMU_WRITE_BYTE(0x3f02, pan_key);
		PMU_WRITE_BYTE(0x3f03, pan_key);
	}
#endif
	
#ifdef PMU_MCU_DEBUG
	printf("pan_key=%0bx\n", pan_key);
#endif

	if(PANNEL_KEY == pan_key)
	{
		return SUCCESS;
	}

	return ERROR ;      
 }
