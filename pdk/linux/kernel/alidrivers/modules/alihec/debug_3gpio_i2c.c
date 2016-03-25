/*
* ALi GPIO Driver.
*/
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/irq.h>

#include <asm/errno.h>

#include <linux/time.h>
#include <linux/delay.h>
#include <linux/mutex.h>

#include <asm/mach-ali/m36_gpio.h>
#include <asm/mach-ali/m6303.h>

#define NEW_INTERFACE

#define HAL_GPIO_ENABLE  1
#define GPIO_GROUP_MAX 	 5

#define HAL_GPIO_EN_REG     0xb8000430
#define HAL_GPIO1_EN_REG    0xb8000434
#define HAL_GPIO2_EN_REG    0xb8000438
#define HAL_GPIO3_EN_REG    0xb800043c
#define HAL_GPIO4_EN_REG    0xb8000440

#define HAL_GPIO_EN_SET(val)         (*(volatile unsigned long *)HAL_GPIO_EN_REG =(val))
#define HAL_GPIO1_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO1_EN_REG =(val))
#define HAL_GPIO2_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO2_EN_REG =(val))
#define HAL_GPIO3_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO3_EN_REG =(val))
#define HAL_GPIO4_EN_SET(val)        (*(volatile unsigned long *)HAL_GPIO4_EN_REG =(val))

#define HAL_GPIO_BIT_ENABLE(pos,en)   \
    do {\
        ((pos < 32)           \
		? HAL_GPIO_EN_SET(((*(volatile unsigned long *)HAL_GPIO_EN_REG)&~(1<<(pos)))|((en)<<(pos)))\
		:((pos < 64)          \
		? HAL_GPIO1_EN_SET(((*(volatile unsigned long *)HAL_GPIO1_EN_REG)&~(1<<(pos - 32)))|((en)<<(pos - 32)))\
		:((pos < 96)          \
		? HAL_GPIO2_EN_SET(((*(volatile unsigned long *)HAL_GPIO2_EN_REG)&~(1<<(pos - 64)))|((en)<<(pos - 64)))\
		:((pos < 128)         \
		? HAL_GPIO3_EN_SET(((*(volatile unsigned long *)HAL_GPIO3_EN_REG)&~(1<<(pos - 96)))|((en)<<(pos - 96)))\
		: HAL_GPIO4_EN_SET(((*(volatile unsigned long *)HAL_GPIO4_EN_REG)&~(1<<(pos - 128)))|((en)<<(pos - 128)))))));\
    } while(0)

static volatile int i2c_bus_release_flag;
static struct mutex gpio_mutex;
static int gpio_udelay;
static spinlock_t gpio_lock;

#ifdef NEW_INTERFACE
//#define bit_dbg printk
#define bit_dbg(...) do{} while(0)

struct i2c_msg {
    __u16 addr;     /* slave address            */
    __u16 flags;
#define I2C_M_TEN           0x0010  /* this is a ten bit chip address */
#define I2C_M_RD            0x0001  /* read data, from slave to master */
#define I2C_M_NOSTART       0x4000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR  0x2000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK    0x1000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK     0x0800  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN      0x0400  /* length will be first received byte */
    __u16 len;      /* msg length               */
	__u8 *buf;      /* pointer to msg data          */
};

struct i2c_gpio_platform_data {
	unsigned int scl;
	unsigned int sda;
	unsigned int sda_pd;
	int          udelay;    /* half clock cycle time in us,
							   minimum 2 us for fast-mode I2C,
							   minimum 5 us for standard-mode I2C and SMBus,
							   maximum 50 us for SMBus */
	int          timeout;   /* in jiffies */
	unsigned char sda_is_open_drain;
	unsigned char scl_is_open_drain;
	unsigned char scl_is_output_only;
} hec_i2c_gpio_config = {14, 12, 11, 5, 0, 0, 0, 0};

struct i2c_algo_bit_data {
	void (*setsda) (unsigned pin, int state);
	void (*setscl) (unsigned pin, int state);
	int  (*getsda) (unsigned pin);
	int  (*getscl) (unsigned pin);
} hec_i2c_gpio_algo;

static int m36_direction_input(unsigned offset)
{
	unsigned long flags;
	
	spin_lock_irqsave(&gpio_lock, flags);
	/* fix IC bug */
	if ((*(unsigned short *)0xB8000002 == 0x3701) 
		&& ((68 == offset) || (69 == offset) || (70 == offset) || (145 == offset) || (146 == offset)))
	{
		HAL_GPIO_BIT_DIR_SET(offset, HAL_GPIO_O_DIR);		
	}
	else
	{
		HAL_GPIO_BIT_DIR_SET(offset, HAL_GPIO_I_DIR);
	}

	HAL_GPIO_BIT_ENABLE(offset, HAL_GPIO_ENABLE);
	
	spin_unlock_irqrestore(&gpio_lock, flags);

	return 0;
}

static int m36_direction_output(unsigned int offset, int value)
{
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);
	/* fix IC bug */
	if ((*(unsigned short *)0xB8000002 == 0x3701) 
		&& ((68 == offset) || (69 == offset) || (70 == offset) || (145 == offset) || (146 == offset)))
	{
		HAL_GPIO_BIT_DIR_SET(offset, HAL_GPIO_I_DIR);		
	}
	else
	{
		HAL_GPIO_BIT_DIR_SET(offset, HAL_GPIO_O_DIR);
	}

	HAL_GPIO_BIT_ENABLE(offset, HAL_GPIO_ENABLE);

	//if (13 == offset)
	    //HAL_GPIO_BIT_SET(offset, value?0:1);
	//else
	    HAL_GPIO_BIT_SET(offset, value?1:0);
	
	spin_unlock_irqrestore(&gpio_lock, flags);
	
	return 0;
}

static int m36_gpio_get(unsigned offset)
{	
	unsigned long flags;
	int val;
	
	//printk("%s->%d\n", __FUNCTION__, offset);

	spin_lock_irqsave(&gpio_lock, flags);

	/* fix IC bug */
	if ((*(unsigned short *)0xB8000002 == 0x3701) && (69 == offset))
	{		
		*((volatile unsigned long *)0xb805c057) |= (1<<1);		/*enable XPMU_GPIO[1] as PMU internal GPIO */
		val = (*((volatile unsigned long *)0xb805c054) >> 1) & 1;	/* read bit 1 */
		*((volatile unsigned long *)0xb805c057) &= ~(1<<1);		/*enable XPMU_GPIO[1] as PMU internal GPIO */		
	}
	else
	{
		val = HAL_GPIO_BIT_GET(offset);
	}

	spin_unlock_irqrestore(&gpio_lock, flags);

	return val;
}

static void m36_gpio_set(unsigned offset, int val)
{
	unsigned long flags;

	//printk("%s->%d val->%d\n", __FUNCTION__, offset,val);

	spin_lock_irqsave(&gpio_lock, flags);

	HAL_GPIO_BIT_SET(offset, val?1:0);
		
	spin_unlock_irqrestore(&gpio_lock, flags);
}

//	.direction_input	= m36_direction_input,
//	.direction_output	= m36_direction_output,
//	.set			    = m36_gpio_set,
//	.get			    = m36_gpio_get,

/* Toggle SDA by changing the direction of the pin */
static void i2c_gpio_setsda_dir(unsigned sda_pin, int state)
{
	if (state)
		m36_direction_input(sda_pin);
	else
		m36_direction_output(sda_pin, 0);
}

/*
 * Toggle SDA by changing the output value of the pin. This is only
 * valid for pins configured as open drain (i.e. setting the value
 * high effectively turns off the output driver.)
 */
static void i2c_gpio_setsda_val(unsigned sda_pin, int state)
{
	m36_gpio_set(sda_pin, state);
}

/* Toggle SCL by changing the direction of the pin. */
static void i2c_gpio_setscl_dir(unsigned scl_pin, int state)
{
	if (state)
		m36_direction_input(scl_pin);
	else
		m36_direction_output(scl_pin, 0);
}

/*
 * Toggle SCL by changing the output value of the pin. This is used
 * for pins that are configured as open drain and for output-only
 * pins. The latter case will break the i2c protocol, but it will
 * often work in practice.
 */
static void i2c_gpio_setscl_val(unsigned scl_pin, int state)
{
	m36_gpio_set(scl_pin, state);
}

static int i2c_gpio_getsda(unsigned sda_pin)
{
	return m36_gpio_get(sda_pin);
}

static int i2c_gpio_getscl(unsigned scl_pin)
{
	return m36_gpio_get(scl_pin);
}

#define hec_setsda(val)   hec_i2c_gpio_algo.setsda(hec_i2c_gpio_config.sda, val)
#define hec_setscl(val)   hec_i2c_gpio_algo.setscl(hec_i2c_gpio_config.scl, val)
#define hec_getsda()      hec_i2c_gpio_algo.getsda(hec_i2c_gpio_config.sda)
#define hec_getscl()      hec_i2c_gpio_algo.getscl(hec_i2c_gpio_config.scl)

#define hec_setsdapd(val) hec_i2c_gpio_algo.setsda(hec_i2c_gpio_config.sda_pd, val)
#define hec_getsdapd()    hec_i2c_gpio_algo.getsda(hec_i2c_gpio_config.sda_pd)

static inline void sdalo(void)
{
	//HEC_SET_SDA_LO();
	hec_setsda(0);
	udelay((gpio_udelay + 1) / 2); 
}

static inline void sdahi(void)
{
	//HEC_SET_SDA_HI();
	hec_setsda(1);
	udelay((gpio_udelay + 1) / 2); 
}

static inline void sdapdlo(void)
{
	//HEC_SET_SDAPD_LO();
	hec_setsdapd(0);
	//i2c_gpio_setsda_val(11, 0);
	udelay((gpio_udelay + 1) / 2); 
}

static inline void sdapdhi(void)
{
	//HEC_SET_SDAPD_HI();
	hec_setsdapd(1);
	//i2c_gpio_setsda_val(11, 1);
	udelay((gpio_udelay + 1) / 2); 
}

static inline void scllo(void)
{
	//HEC_SET_SCL_LO();
	hec_setscl(0);
	udelay(gpio_udelay / 2); 
}

static inline void sclhi(void)
{
	//HEC_SET_SCL_HI();
	hec_setscl(1);
	udelay(gpio_udelay);
}

static void i2c_start(void)
{
	/* assert: scl, sda are high */
	//HEC_SET_SDA_LO();
	hec_setsda(0);
	udelay(gpio_udelay);
	scllo();
}

static void i2c_repstart(void)
{
	/* assert: scl is low */
	sdahi();
	sclhi();
	//HEC_SET_SDA_LO();
	hec_setsda(0);
	udelay(gpio_udelay);
	scllo();
}

static void i2c_stop(void)
{
	/* assert: scl is low */
	sdalo();
	sclhi();
	//HEC_SET_SDA_HI();
	hec_setsda(1);
	udelay(gpio_udelay);
}

static int i2c_outb(unsigned char c)
{
	int i;
	int sb;
	int ack;

	/* assert: scl is low */
	for (i = 7; i >= 0; i--) {
		sb = (c >> i) & 1;
		/*if (sb)
		{
			HEC_SET_SDA_HI();
		}
		else
		{
			HEC_SET_SDA_LO();
		}*/
		hec_setsda(sb);
		udelay(5);        //wait sda steable
		sclhi();
		udelay((gpio_udelay + 1) / 2);
	 	scllo();
	}
	sdahi();
	sdapdhi();
	sclhi();
	/* read ack: SDA should be pulled down by slave, or it may
	 * NAK (usually to report problems with the data we wrote).
	 */
	ack = !hec_getsdapd();  //HEC_GET_SDAPD();//!getsda(adap);    /* ack: sda is pulled low -> success */
	bit_dbg("i2c_outb: 0x%02x %s\n", (int)c, ack ? "A" : "NA");
    //udelay(4);        //wait sda steable
	scllo();
	return ack;
	/* assert: scl is low (sda undef) */
}

static int i2c_inb(void)
{
	/* read byte via i2c port, without start/stop sequence  */
	/* acknowledge is sent in i2c_read.         */
	int i;
	unsigned char indata = 0;

	/* assert: scl is low */
	sdahi();
	sdapdhi();
	for (i = 0; i < 8; i++) {
		sclhi();
		indata *= 2;
		if (/*HEC_GET_SDAPD()*/hec_getsdapd())
			indata |= 0x01;
		//HEC_SET_SCL_LO();
		hec_setscl(0);
		udelay(i == 7 ? gpio_udelay / 2 : gpio_udelay);
	}
	/* assert: scl is low */
	return indata;
}

static int try_address(unsigned char addr, int retries)
{
	int i, ret = 0;

	for (i = 0; i <= retries; i++) {
		ret = i2c_outb(addr);
		if (ret == 1 || i == retries)
			break;
		bit_dbg("emitting stop condition\n");
		i2c_stop();
		udelay(gpio_udelay);
		//yield();
		mdelay(1);
		bit_dbg("emitting start condition\n");
		i2c_start();
	}
	if (i && ret)
		bit_dbg("Used %d tries to %s client at "
				"0x%02x: %s\n", i + 1,
				addr & 1 ? "read from" : "write to", addr >> 1,
				ret == 1 ? "success" : "failed, timeout?");
	return ret;
}

static int sendbytes(struct i2c_msg *msg)
{
	const unsigned char *temp = msg->buf;
	int count = msg->len;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	int retval;
	int wrcount = 0;

	while (count > 0) {
		retval = i2c_outb(*temp);

		/* OK/ACK; or ignored NAK */
		if ((retval > 0) || (nak_ok && (retval == 0))) {
			count--;
			temp++;
			wrcount++;

			/* A slave NAKing the master means the slave didn't like
			 * something about the data it saw.  For example, maybe
			 * the SMBus PEC was wrong.
			 */
		} else if (retval == 0) {
			bit_dbg("sendbytes: NAK bailout.\n");
			return -EIO;

			/* Timeout; or (someday) lost arbitration
			 *
			 * FIXME Lost ARB implies retrying the transaction from
			 * the first message, after the "winning" master issues
			 * its STOP.  As a rule, upper layer code has no reason
			 * to know or care about this ... it is *NOT* an error.
			 */
		} else {
			bit_dbg("sendbytes: error %d\n", retval);
			return retval;
		}
	}
	return wrcount;
}

static int acknak(int is_ack)
{
	/* assert: sda is high */
	if (is_ack)     /* send ack */
	    hec_setsda(0);  //HEC_SET_SDA_LO();
	udelay((gpio_udelay + 1) / 2);
	sclhi();
	scllo();
	return 0;
}

static int readbytes(struct i2c_msg *msg)
{
	int inval;
	int rdcount = 0;    /* counts bytes read */
	unsigned char *temp = msg->buf;
	int count = msg->len;
	const unsigned flags = msg->flags;

	while (count > 0) {
		inval = i2c_inb();
		if (inval >= 0) {
			*temp = inval;
			rdcount++;
		} else {   /* read timed out */
			break;
		}

		temp++;
		count--;

		bit_dbg("readbytes: 0x%02x %s\n",
				inval,
			    (flags & I2C_M_NO_RD_ACK) 
				    ? "(no ack/nak)"
				    : (count ? "A" : "NA"));

		if (!(flags & I2C_M_NO_RD_ACK)) {
			inval = acknak(count);
			if (inval < 0)
			    return inval;
		}
	}
	return rdcount;
}

static int bit_doAddress(struct i2c_msg *msg)
{
	unsigned short flags = msg->flags;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;

	unsigned char addr;
	int ret, retries;
	
	retries = nak_ok ? 0 : 3;//i2c_adap->retries;

	addr = msg->addr << 1;
	if (flags & I2C_M_RD)
		addr |= 1;
	if (flags & I2C_M_REV_DIR_ADDR)
		addr ^= 1;
	ret = try_address(addr, retries);
	if ((ret != 1) && !nak_ok)
		return -ENXIO;
	return 0;
}

static int bit_xfer(struct i2c_msg msgs[], int num)
{
	struct i2c_msg *pmsg;
	int i, ret;
	unsigned short nak_ok;

	bit_dbg("emitting start condition\n");
	i2c_start();
	for (i = 0; i < num; i++) {
		pmsg = &msgs[i];
		nak_ok = pmsg->flags & I2C_M_IGNORE_NAK;
		if (!(pmsg->flags & I2C_M_NOSTART)) {
			if (i) {
				bit_dbg("emitting "
						"repeated start condition\n");
				i2c_repstart();
			}
			ret = bit_doAddress(pmsg);
			if ((ret != 0) && !nak_ok) {
				printk("  NAK from "
						"device addr 0x%02x msg #%d\n",
						msgs[i].addr<<1, i);
				goto bailout;
			}
		}
		if (pmsg->flags & I2C_M_RD) {
			/* read bytes into buffer*/
			ret = readbytes(pmsg);
			if (ret >= 1)
				printk("  read %d byte%s\n",
						ret, ret == 1 ? "" : "s");
			if (ret < pmsg->len) {
				if (ret >= 0)
					ret = -EREMOTEIO;
				goto bailout;
			}
		} else {
			/* write bytes from buffer */
			ret = sendbytes(pmsg);
			if (ret >= 1)
				printk("  wrote %d byte%s\n",
						ret, ret == 1 ? "" : "s");
			if (ret < pmsg->len) {
				if (ret >= 0)
					ret = -EREMOTEIO;
				goto bailout;
			}
		}
	}
	ret = i;

bailout:
	bit_dbg("emitting stop condition\n");
	i2c_stop();
	return ret;
}

static int test_bus(char *name)
{
	int scl, sda;

	//if (adap->getscl == NULL)
	//pr_info("%s: Testing SDA only, SCL is not readable\n", name);

	sda = hec_getsda();
	scl = hec_getscl();//(adap->getscl == NULL) ? 1 : getscl(adap);
	if (!scl || !sda) {
		printk("%s: bus seems to be busy: %d %d\n", name, sda, scl);
		//goto bailout;
	}   

	sdalo();
	sda = hec_getsda();
	scl = hec_getscl();//(adap->getscl == NULL) ? 1 : getscl(adap);
	if (sda) {
		printk("%s: SDA stuck high!\n", name);
		//goto bailout;
	}   
	if (!scl) {
		printk("%s: SCL unexpected low "
				"while pulling SDA low!\n", name);
		//goto bailout;
	}   

	sdahi();
	sda = hec_getsda();
	scl = hec_getscl();//(adap->getscl == NULL) ? 1 : getscl(adap);
	if (!sda) {
		printk("%s: SDA stuck low!\n", name);
		//goto bailout;
	}   
	if (!scl) {
		printk("%s: SCL unexpected low "
				"while pulling SDA high!\n", name);
		//goto bailout;
	}
	scllo();
	sda = hec_getsda();
	scl = hec_getscl();//(adap->getscl == NULL) ? 0 : getscl(adap);
	if (scl) {
		printk("%s: SCL stuck high!\n", name);
		//goto bailout;
	}
	if (!sda) {
		printk("%s: SDA unexpected low "
				"while pulling SCL low!\n", name);
		//goto bailout;
	}

	sclhi();
	sda = hec_getsda();
	scl = hec_getscl();//(adap->getscl == NULL) ? 1 : getscl(adap);
	if (!scl) {
		printk("%s: SCL stuck low!\n", name);
		//goto bailout;
	}
	if (!sda) {
		printk("%s: SDA unexpected low "
				"while pulling SCL high!\n", name);
		//goto bailout;
	}
	printk("%s: Test OK\n", name);
	return 0;
bailout:
	sdahi();
	sclhi();
	return -ENODEV;
}

void my_i2c_gpio_probe(void)
{
    struct i2c_gpio_platform_data *pdata;
	struct i2c_algo_bit_data *palgo;

	hec_i2c_gpio_config.scl = 13;
    hec_i2c_gpio_config.sda = 12;
    hec_i2c_gpio_config.sda_pd = 11;
	hec_i2c_gpio_config.udelay = 5;
	hec_i2c_gpio_config.sda_is_open_drain = 0;
	hec_i2c_gpio_config.scl_is_open_drain = 0;
	hec_i2c_gpio_config.scl_is_output_only = 0;
	
	pdata = &hec_i2c_gpio_config;
	palgo = &hec_i2c_gpio_algo;

	mutex_init(&gpio_mutex);
	gpio_udelay = pdata->udelay;
	gpio_lock = __SPIN_LOCK_UNLOCKED(gpio_lock);

	if (pdata->sda_is_open_drain) {
		m36_direction_output(pdata->sda, 1);
		palgo->setsda = i2c_gpio_setsda_val;
	} else {
		m36_direction_input(pdata->sda);
	    m36_direction_input(pdata->sda_pd);
		palgo->setsda = i2c_gpio_setsda_dir;
	}

	if (pdata->scl_is_open_drain || pdata->scl_is_output_only) {
		m36_direction_output(pdata->scl, 1);
		palgo->setscl = i2c_gpio_setscl_val;
	} else {
		m36_direction_input(pdata->scl);
		palgo->setscl = i2c_gpio_setscl_dir;
	}

	//if (!pdata->scl_is_output_only)
		palgo->getscl = i2c_gpio_getscl;
	
	palgo->getsda = i2c_gpio_getsda;

	test_bus("[Three-line I2C]");
	sdahi();
	sdapdhi();
    sclhi();
}

#else
static unsigned int osal_get_tick(void)
{
    struct timeval tv;
    do_gettimeofday(&tv);
    return (tv.tv_sec*1000) + (tv.tv_usec / 1000);
}

#define HEC_I2C_GPIO_TIMES_OUT  10 
#define HEC_I2C_BPS             100000    //100KHz
#define I2C_BYTE_TIME1          (75000)
#define I2C_BYTE_TIME2          (75000)

struct i2c_gpio_platform_data {
	unsigned int scl;
	unsigned int sda;
	unsigned int sda_pd;
	int          udelay;    /* half clock cycle time in us,
							   minimum 2 us for fast-mode I2C,
							   minimum 5 us for standard-mode I2C and SMBus,
							   maximum 50 us for SMBus */
} hec_i2c_gpio_config = {14, 12, 11, 5};

#define HEC_SDA_ENABLE()   HAL_GPIO_BIT_ENABLE(hec_i2c_gpio_config.sda, HAL_GPIO_ENABLE)
#define HEC_SET_SDA_OUT()  HAL_GPIO_BIT_DIR_SET(hec_i2c_gpio_config.sda, HAL_GPIO_O_DIR)
#define HEC_SET_SDA_HI()   HAL_GPIO_BIT_SET(hec_i2c_gpio_config.sda, 1)
#define HEC_SET_SDA_LO()   HAL_GPIO_BIT_SET(hec_i2c_gpio_config.sda, 0)
#define HEC_GET_SDA()      HAL_GPIO_BIT_GET(hec_i2c_gpio_config.sda)

#define HEC_SDAPD_ENABLE() HAL_GPIO_BIT_ENABLE(hec_i2c_gpio_config.sda_pd, HAL_GPIO_ENABLE)
#define HEC_SET_SDAPD_IN() HAL_GPIO_BIT_DIR_SET(hec_i2c_gpio_config.sda_pd, HAL_GPIO_I_DIR)
#define HEC_SET_SDAPD_HI() HAL_GPIO_BIT_SET(hec_i2c_gpio_config.sda_pd, 1)
#define HEC_SET_SDAPD_LO() HAL_GPIO_BIT_SET(hec_i2c_gpio_config.sda_pd, 0)
#define HEC_GET_SDAPD()    HAL_GPIO_BIT_GET(hec_i2c_gpio_config.sda_pd)

#define HEC_SCL_ENABLE()   HAL_GPIO_BIT_ENABLE(hec_i2c_gpio_config.scl, HAL_GPIO_ENABLE)
#define HEC_SET_SCL_OUT()  HAL_GPIO_BIT_DIR_SET(hec_i2c_gpio_config.scl, HAL_GPIO_O_DIR)
#define HEC_SET_SCL_HI()   HAL_GPIO_BIT_SET(hec_i2c_gpio_config.scl, 1)
#define HEC_SET_SCL_LO()   HAL_GPIO_BIT_SET(hec_i2c_gpio_config.scl, 0)
#define HEC_GET_SCL()      HAL_GPIO_BIT_GET(hec_i2c_gpio_config.scl)

static int hec_i2c_gpio_phase_start(void)
{
	HEC_SET_SDA_HI();
	HEC_SET_SCL_HI();
	udelay(gpio_udelay);
	udelay(gpio_udelay);
	
	HEC_SET_SDA_LO();
	udelay(gpio_udelay);
	udelay(gpio_udelay);
	HEC_SET_SCL_LO();
	udelay(2);

	return 0;
}

static int hec_i2c_gpio_phase_stop(void)
{
	HEC_SET_SDA_LO();
	udelay(gpio_udelay);
	HEC_SET_SCL_HI();
	udelay(gpio_udelay);
	
	HEC_SET_SDA_HI();
	udelay(2);
	return 0;
}

static void hec_i2c_gpio_phase_set_bit(int val)
{
	HEC_SET_SCL_LO();
	if (val)
	{
		HEC_SET_SDA_HI();
	}
	else
	{
		HEC_SET_SDA_LO();
	}
	udelay(gpio_udelay);

	HEC_SET_SCL_HI();
	udelay(gpio_udelay);
	HEC_SET_SCL_LO();

	return;
}

static int hec_i2c_gpio_phase_get_bit(void)
{
	int ret = 0;

	HEC_SET_SDAPD_IN();
	//HEC_SET_SDAPD_HI();
	udelay(gpio_udelay);
	HEC_SET_SCL_HI();
	udelay(gpio_udelay);
	ret = HEC_GET_SDAPD();
	HEC_SET_SCL_LO();

	return ret;
}

static int hec_i2c_gpio_phase_set_byte(unsigned char data)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		if (data & 0x80)
		{
			hec_i2c_gpio_phase_set_bit(1);
		}
		else
		{
			hec_i2c_gpio_phase_set_bit(0);
		}

		data <<= 1;
	}

	return (hec_i2c_gpio_phase_get_bit());
}

static unsigned char hec_i2c_gpio_phase_get_byte(int ack)
{
	unsigned char data = 0;
	int i;

	for (i = 0; i < 8; i++)
	{
		data <<= 1;
		data |= hec_i2c_gpio_phase_get_bit();
	}
    hec_i2c_gpio_phase_set_bit(ack);

	return data;
}

static int hec_i2c_gpio_read_no_stop(unsigned char slv_addr, unsigned char *data, int len)
{
	unsigned int tick1, tick2;
	int i = HEC_I2C_GPIO_TIMES_OUT;

	slv_addr |= 1;                      /* Read */
	while (--i)                         /* Ack polling !! */
	{
		hec_i2c_gpio_phase_start();
		/* has /ACK => sino_i2c_gpio_phase_start transfer */
		tick1 = osal_get_tick();
		if (!hec_i2c_gpio_phase_set_byte(slv_addr))
		{
			tick2 = osal_get_tick();
			if (((tick2-tick1)) < I2C_BYTE_TIME1)
				break;
		}
		/* device is busy, issue sino_i2c_gpio_phase_stop and chack again later */
		hec_i2c_gpio_phase_stop();
		mdelay(1);             /* wait for 1mS */
	}

	if (i == 0)
	{
		return 0xFF;
	}

	for (i = 0; i < (len - 1); i++)
	{
		/*with no /ack to stop process */
		tick1 = osal_get_tick();
		data[i] = hec_i2c_gpio_phase_get_byte(0);
		tick2 = osal_get_tick();
		if (((tick2-tick1)) > I2C_BYTE_TIME2)
		{
			hec_i2c_gpio_phase_stop();
			//printk("[note] read i2c time exceeds = %d ticks!\n", ((tick2-tick1)));
			return 0xFF;
		}
	}
	tick1 = osal_get_tick();
	data[len - 1] = hec_i2c_gpio_phase_get_byte(1);
	tick2 = osal_get_tick();
	if (((tick2-tick1)) > I2C_BYTE_TIME2)
	{
		hec_i2c_gpio_phase_stop();
		//printk("[note] read i2c time exceeds = %d ticks!\n", ((tick2-tick1)));
		return 0xFF;
	}

	return 0;
}

static int hec_i2c_gpio_write_no_stop(unsigned char slv_addr, unsigned char *data, int len)
{
	unsigned int tick1, tick2;
	int i = HEC_I2C_GPIO_TIMES_OUT;

	slv_addr &= 0xFE;                   /*Write*/
	while (--i)                         /* Ack polling !! */
	{
		hec_i2c_gpio_phase_start();
		/* has /ACK => sino_i2c_gpio_phase_start transfer */
		tick1 = osal_get_tick();
		if (!hec_i2c_gpio_phase_set_byte(slv_addr))
		{
			tick2 = osal_get_tick();
			if (((tick2-tick1)) < I2C_BYTE_TIME1)
				break;
		}
		/* device is busy, issue sino_i2c_gpio_phase_stop and chack again later */
		hec_i2c_gpio_phase_stop();
		mdelay(1);             /* wait for 1mS */
	}

	if (i == 0)
	{
		return 0xFF;
	}

	for (i = 0; i < len; i++)
	{
		tick1 = osal_get_tick();
		hec_i2c_gpio_phase_set_byte(data[i]);
		tick2 = osal_get_tick();
		if (((tick2-tick1)) > I2C_BYTE_TIME2)
		{
			hec_i2c_gpio_phase_stop();
			//printk("[note] write i2c time exceeds = %d ticks!\n", ((tick2-tick1)));
			return 0xFF;
		}
	}

	return 0;
}

static int hec_i2c_gpio_read(unsigned char slv_addr, unsigned char *data, int len)
{
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);
	if ((ret = hec_i2c_gpio_read_no_stop(slv_addr, data, len)) != 0)
	{
		//sino_i2c_gpio_phase_stop(id);
		spin_unlock_irqrestore(&gpio_lock, flags);
		return ret;
	}
	hec_i2c_gpio_phase_stop();
	spin_unlock_irqrestore(&gpio_lock, flags);

	return 0;
}

static int hec_i2c_gpio_write(unsigned char slv_addr, unsigned char *data, int len)
{
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);
	if ((ret = hec_i2c_gpio_write_no_stop(slv_addr, data, len)) != 0)
	{
		//sino_i2c_gpio_phase_stop(id);
		spin_unlock_irqrestore(&gpio_lock, flags);
		return ret;
	}
	hec_i2c_gpio_phase_stop();
	spin_unlock_irqrestore(&gpio_lock, flags);

	return 0;
}

static int hec_i2c_gpio_write_read(unsigned char slv_addr, unsigned char *data, int wlen, int rlen)
{
	int ret;
	unsigned long flags;

	if (wlen == 0)
	{
		return hec_i2c_gpio_read(slv_addr, data, rlen);
	}

	spin_lock_irqsave(&gpio_lock, flags);
	if ((ret = hec_i2c_gpio_write_no_stop(slv_addr, data, wlen)) != 0)
	{
		//sino_i2c_gpio_phase_stop(id);
		spin_unlock_irqrestore(&gpio_lock, flags);
		return ret;
	}

	if ((ret = hec_i2c_gpio_read_no_stop(slv_addr, data, rlen)) != 0)
	{
		//sino_i2c_gpio_phase_stop(id);
		spin_unlock_irqrestore(&gpio_lock, flags);
		return ret;
	}

	hec_i2c_gpio_phase_stop();
	spin_unlock_irqrestore(&gpio_lock, flags);

	return 0;
}

void my_i2c_gpio_probe(void)
{
	hec_i2c_gpio_config.scl = 13;
	hec_i2c_gpio_config.sda = 12;
	hec_i2c_gpio_config.sda_pd = 11;
	hec_i2c_gpio_config.udelay = 500000 / HEC_I2C_BPS;
	
	gpio_lock = __SPIN_LOCK_UNLOCKED(gpio_lock);
	gpio_udelay = hec_i2c_gpio_config.udelay;

    HEC_SCL_ENABLE();
	HEC_SDA_ENABLE();
	HEC_SDAPD_ENABLE();

    HEC_SET_SDA_OUT();
	HEC_SET_SDAPD_IN();
	HEC_SET_SCL_OUT();
}
#endif

//* interface *//
int HEC_READ(unsigned char base, unsigned char *p, int lw, int lr)
{
	int ret = 1;

	if (i2c_bus_release_flag == 0)
		i2c_bus_release_flag = 1;
	else
		return ret;

	mutex_lock(&gpio_mutex);
#ifdef NEW_INTERFACE
	struct i2c_msg msgs[] = {
	    { .addr = base>>1, .flags = 0,        .len = lw, .buf = &p[0] },
	    { .addr = base>>1, .flags = I2C_M_RD, .len = lr, .buf = p     },
	};
	if (2 == bit_xfer(msgs, 2))
		ret = 0;
#else
	ret = hec_i2c_gpio_write_read(base, p, lw, lr);
#endif

	mutex_unlock(&gpio_mutex);
	i2c_bus_release_flag = 0;

	return ret;
}

int HEC_WRITE(unsigned char base, unsigned char *p, int l)
{
	int ret = 1;

	if (i2c_bus_release_flag == 0)
		i2c_bus_release_flag = 1;
	else
		return ret;

	mutex_lock(&gpio_mutex);
#ifdef NEW_INTERFACE
	struct i2c_msg msgs = { .addr = base>>1, .flags = 0, .len = l, .buf = p };
    if (1 == bit_xfer(&msgs, 1))
		ret = 0;
#else
	ret = hec_i2c_gpio_write(base, p, l);
#endif

	mutex_unlock(&gpio_mutex);
	i2c_bus_release_flag = 0;

	return ret;
}
