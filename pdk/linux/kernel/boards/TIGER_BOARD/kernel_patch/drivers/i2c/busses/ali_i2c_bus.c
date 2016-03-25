#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <asm/mach-ali/typedef.h>
#include <asm/mach-ali/m36_irq.h>
//#include <asm/mach-m3602/gpio.h>
#include <ali_reg.h>

#if 0
#define PRINTK_INFO(x...) printk(KERN_INFO x)
#else
#define PRINTK_INFO(x...)
#endif

#define SCB_HCR		 			0x00
#define SCB_HSR					0x01
#define SCB_IER					0x02
#define SCB_ISR					0x03
#define SCB_SAR					0x04
#define SCB_SSAR				0x05
#define SCB_HPCC				0x06
#define SCB_LPCC				       0x07
#define SCB_PSUR				0x08
#define SCB_PHDR				0x09
#define SCB_RSUR				0x0A
#define SCB_SHDR				0x0B
#define SCB_FCR					0x0C
#define SCB_FDR					0x10
#define SCB_DDC_ADDR		       0x0E
#define SCB_SEG_PTR				0x0F
#define SCB_IER1					0x20
#define SCB_ISR1					0x21
#define SCB_FIFO_TRIG_LEVEL	0x22
#define SCB_BC          				0x23
#define SCB_SSAR_EN     			0x24
#define SCB_SSAR1					0x25
#define SCB_SSAR2					0x26
#define SCB_SSAR3					0x27
/* SCB_HCR bit define */
#define SCB_HCE					0x80
#define SCB_DNEE				0x40
#define SCB_CP_WT				0x00
#define SCB_CP_CAR				0x04
#define SCB_CP_SER				0x08
#define SCB_CP_STR				0x0c
#define SCB_ST					0x01
#define SCB_EDDC				0x20
/* SCB_HSR bit define */
#define SCB_DB					0x80
#define SCB_DNE					0x40
#define SCB_HB					0x20
#define SCB_FER					0x10
#define SCB_FO					0x08
#define SCB_FU					0x04
#define SCB_FF					0x02
#define SCB_FE					0x01
/* SCB_FCR bit define */
#define SCB_FLUSH				0x80
#define SCB_BC_VAL				0x1f
/* SCB_IER bit define */
#define SCB_IERE				       0xf0

#define I2C_SCB_TIMES_OUT		100
#define I2C_SCB_RETRY			10

//#define RET_SUCCESS 	0
#define ERR_FAILURE		-1
#define ERR_TIME_OUT	-9

/*static struct i2c_scb_st
{
	UINT32	mutex_id;
} *i2c_scb = NULL;
*/
#define I2C_SCB_NUM				4

static struct mutex ali_i2c_bus_mutex[I2C_SCB_NUM];
static struct
{
	UINT32 reg_base;
	int    irq;
} i2c_scb_reg[I2C_SCB_NUM] = {{0x18018200,26},{0x18018700,33},{0x18018b00,34}, {0x18018d00,-1}};


#define SCB_READ8(id, reg)			(__REG8ALI((i2c_scb_reg[id].reg_base + reg)))
#define SCB_WRITE8(id, reg, data)	(__REG8ALI((i2c_scb_reg[id].reg_base + reg)) = (data))


static INT32 i2c_scb_wait_host_ready(UINT32 id)
{
	int i = I2C_SCB_TIMES_OUT;


	if (id >= I2C_SCB_NUM)
	{
		return ERR_FAILURE;
	}

	while (--i)
	{
		if ((SCB_READ8(id, SCB_HSR) & SCB_HB) == 0)
		{
			return RET_SUCCESS;
		}
		//mdelay(1);
	}

	//0x14 bit[2:0] == 0, master is idle, continue send data/cmd.
	if((SCB_READ8(id, 0x14)&0x07) == 0)
	{
		return RET_SUCCESS;
	}
	
	PRINTK_INFO("i2c_scb_wait_host_ready: time out\n");
	return ERR_TIME_OUT;
}

static INT32 i2c_scb_wait_dev_ready(UINT32 id)
{
	int i = I2C_SCB_TIMES_OUT;


	if (id >= I2C_SCB_NUM)
	{
		return ERR_FAILURE;
	}
	
	while (--i)
	{
		if ((SCB_READ8(id, SCB_HSR) & (SCB_DB | SCB_DNE | SCB_HB)) == 0)
		{
			return RET_SUCCESS;
		} else if ((SCB_READ8(id, SCB_ISR) & 0x0e) != 0)
		{
			PRINTK_INFO("i2c_scb_wait_dev_ready: fail\n");
			return ERR_FAILURE;
		}
		mdelay(1);
	}

	PRINTK_INFO("i2c_scb_wait_dev_ready: time out\n");
	return ERR_TIME_OUT;
}

static INT32 i2c_scb_wait_fifo_ready(UINT32 id, int len)
{
	int i = I2C_SCB_TIMES_OUT;
	

	if (id >= I2C_SCB_NUM)
	{
		return ERR_FAILURE;
	}

	while (--i)
	{
		if ((SCB_READ8(id, SCB_FCR) & SCB_BC_VAL) == (len > 16 ? 16: len))
		{
			return RET_SUCCESS;
		}
		mdelay(1);
	}

	PRINTK_INFO("i2c_scb_wait_fifo_ready: time out\n");
	return ERR_TIME_OUT;
}

static INT32 i2c_scb_wait_trans_done(UINT32 id)
{
	int i = I2C_SCB_TIMES_OUT;
	UINT8 status;


	if (id >= I2C_SCB_NUM)
	{
		return ERR_FAILURE;
	}

	while (--i)
	{
		status = SCB_READ8(id, SCB_ISR);
		if ((status & 1) != 0)
		{
			return RET_SUCCESS;
		} else if ((status & 0x0e) != 0)
		{
			PRINTK_INFO("i2c_scb_wait_trans_done: fail(%x)\n", status);
			return ERR_FAILURE;
		}
		mdelay(1);
	}

	PRINTK_INFO("i2c_scb_wait_trans_done: time out\n");
	return ERR_TIME_OUT;
}

/*---------------------------------------------------
INT32 ali_i2c_scb_read(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
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
INT32 ali_i2c_scb_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i;
	int timeout;


	if (id >= I2C_SCB_NUM)
	{
		return ERR_FAILURE;
	}

	//osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	mutex_lock(&(ali_i2c_bus_mutex[id]));
	if (i2c_scb_wait_host_ready(id) != RET_SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);	

		SCB_WRITE8(id, SCB_BC, ((len>>5) & 0xff));
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_CAR | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == RET_SUCCESS)
		{
			break;
		}
		mdelay(1); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		PRINTK_INFO("ali_i2c_scb_read: %d times i2c_scb_wait_dev_ready  time out\n", I2C_SCB_RETRY);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_fifo_ready(id, len) != RET_SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		return ERR_TIME_OUT;
	}

	for (i = 0; i < len; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
		 	if (SCB_READ8(id, SCB_FCR) & 0x1F)
		 	{
		 		break;
		 	}
			mdelay(1);
		}
		if (timeout == 0)
		{
			//osal_mutex_unlock(i2c_scb[id].mutex_id);
			mutex_unlock(&(ali_i2c_bus_mutex[id]));
			PRINTK_INFO("ali_i2c_scb_read: fail.\n");
			return ERR_FAILURE;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}
	//osal_mutex_unlock(i2c_scb[id].mutex_id);
	mutex_unlock(&(ali_i2c_bus_mutex[id]));

	return RET_SUCCESS;
}
EXPORT_SYMBOL(ali_i2c_scb_read);

/*---------------------------------------------------
INT32 ali_i2c_scb_write(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
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
INT32 ali_i2c_scb_write(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i, j;


	if (id >= I2C_SCB_NUM)
	{
		return ERR_FAILURE;
	}

	//osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	mutex_lock(&(ali_i2c_bus_mutex[id]));
	if (i2c_scb_wait_host_ready(id) != RET_SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		return ERR_TIME_OUT;
	}

	for (j = I2C_SCB_RETRY; j > 0; j--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);
	 
		SCB_WRITE8(id, SCB_BC, ((len>>5) & 0xff));
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));

		for (i = 0; i < len; i++)
		{
			if (SCB_READ8(id, SCB_HSR) & SCB_FF)
			{
				//osal_mutex_unlock(i2c_scb[id].mutex_id);
				mutex_unlock(&(ali_i2c_bus_mutex[id]));
				PRINTK_INFO("ali_i2c_scb_write: fail.\n");
				return ERR_FAILURE;
			}
			SCB_WRITE8(id, SCB_FDR, data[i]);
		}

		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_WT | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == RET_SUCCESS)
		{
			break;
		}
		mdelay(1); 			/* wait for 1mS */
	}
	if (j == 0)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		PRINTK_INFO("ali_i2c_scb_write: %d times i2c_scb_wait_dev_ready  time out\n", I2C_SCB_RETRY);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_trans_done(id) != RET_SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		return ERR_TIME_OUT;
	}
	//osal_mutex_unlock(i2c_scb[id].mutex_id);
	mutex_unlock(&(ali_i2c_bus_mutex[id]));

	return RET_SUCCESS;
}
EXPORT_SYMBOL(ali_i2c_scb_write);

/*---------------------------------------------------
INT32 ali_i2c_scb_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 wlen, UINT32 rlen);
	Perform bytes write-read process
	Stream Format:
		S<SLV_W><Write>S<SLV_W><Read>P
		S		: Start
		P		: Stop
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
INT32 ali_i2c_scb_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	int i, j;
	int timeout;


	if (id >= I2C_SCB_NUM)
	{
		return ERR_FAILURE;
	}

	if (wlen == 0)
	{
		return ali_i2c_scb_read(id, slv_addr, data, rlen);
	}

	//osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	mutex_lock(&(ali_i2c_bus_mutex[id]));
	if (i2c_scb_wait_host_ready(id) != RET_SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);	 

		SCB_WRITE8(id, SCB_SSAR_EN, ((wlen - 1)&0x3));
		for(j = 0; j < ((wlen - 1)&0x3); j++)
			 SCB_WRITE8(id, (SCB_SSAR1 + j), data[j+1]);
		SCB_WRITE8(id, SCB_BC, ((rlen>>5) & 0xff));
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_SER | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == RET_SUCCESS)
		{
			break;
		}
		mdelay(1); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		PRINTK_INFO("ali_i2c_scb_write_read: %d times i2c_scb_wait_dev_ready  time out\n", I2C_SCB_RETRY);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_fifo_ready(id, rlen) != RET_SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		return ERR_TIME_OUT;
	}

	for (i = 0; i < rlen; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
		 	if (SCB_READ8(id, SCB_FCR) & 0x1F)
		 	{
		 		break;
		 	}
			mdelay(1);
		}
		if (timeout == 0)
		{
			//osal_mutex_unlock(i2c_scb[id].mutex_id);
			mutex_unlock(&(ali_i2c_bus_mutex[id]));
			PRINTK_INFO("ali_i2c_scb_write_read: fail.\n");
			return ERR_FAILURE;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}
	//osal_mutex_unlock(i2c_scb[id].mutex_id);
	mutex_unlock(&(ali_i2c_bus_mutex[id]));

	return RET_SUCCESS;
}

EXPORT_SYMBOL(ali_i2c_scb_write_read);

INT32 i2c_scb_write_read_std(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	int i,j;
	int timeout;


	if (id >= I2C_SCB_NUM)
	{
		return ERR_FAILURE;
	}
	
	if (wlen == 0)
	{
		return ali_i2c_scb_read(id, slv_addr, data, rlen);
	}

	mutex_lock(&(ali_i2c_bus_mutex[id]));
	//osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if (i2c_scb_wait_host_ready(id) != RET_SUCCESS)
	{
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		//osal_up(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}
	
	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);

	//if ((ALI_M3329E == g_scb_sys_chip_id)&& (g_scb_sys_chip_ver>=IC_REV_5 )||(ALI_S3602F == g_scb_sys_chip_id))
	// {
		SCB_WRITE8(id, SCB_SSAR_EN, ((wlen - 1)&0x3));
		for(j = 0; j < ((wlen - 1)&0x3); j++)
			 SCB_WRITE8(id, (SCB_SSAR1 + j), data[j+1]);
		SCB_WRITE8(id, SCB_BC, ((rlen>>5) & 0xff));
	// }
		
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
			SCB_CP_STR | SCB_ST);
		
		if (i2c_scb_wait_dev_ready(id) == RET_SUCCESS)
		{
			break;
		}
		//osal_task_sleep(1); 			/* wait for 1mS */
		mdelay(1);
	}
	if (i == 0)
	{
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		//osal_up(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}
	
	if (i2c_scb_wait_fifo_ready(id, rlen) != RET_SUCCESS)
	{
		mutex_unlock(&(ali_i2c_bus_mutex[id]));
		//osal_up(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}
	
	for (i = 0; i < rlen; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
			if (SCB_READ8(id, SCB_FCR) & 0x1F)
			{
				break;
			}
			//osal_task_sleep(1);
			mdelay(1);
		}
		if (timeout == 0)
		{
			mutex_unlock(&(ali_i2c_bus_mutex[id]));
			//osal_up(i2c_scb[id].mutex_id);			
			return ERR_FAILURE;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}
	mutex_unlock(&(ali_i2c_bus_mutex[id]));
	//osal_up(i2c_scb[id].mutex_id);
	
	return RET_SUCCESS;
}

EXPORT_SYMBOL(i2c_scb_write_read_std);


/*
 * Generic i2c master transfer entrypoint.
 *
 * Note: We do not use Atmel's feature of storing the "internal device address".
 * Instead the "internal device address" has to be written using a separate
 * i2c message.
 * http://lists.arm.linux.org.uk/pipermail/linux-arm-kernel/2004-September/024411.html
 */
static int ali_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *pmsg, int num)
{
	int i, ret;

	PRINTK_INFO("%s: processing %d messages:\n", __FUNCTION__, num);

	for (i = 0; i < num; i++) {
		PRINTK_INFO(" #%d: %sing %d byte%s %s 0x%02x\n", i,
			pmsg->flags & I2C_M_RD ? "read" : "writ",
			pmsg->len, pmsg->len > 1 ? "s" : "",
			pmsg->flags & I2C_M_RD ? "from" : "to",	pmsg->addr);


		pmsg->addr = (pmsg->addr << 1);    //for I2C_SLAVE ioctl should <= 0x7f when I2C_M_TEN=0

		if (pmsg->len && pmsg->buf) 
		{	/* sanity check */
			if (pmsg->flags & I2C_M_RD)
			{
				if(pmsg->flags&I2C_M_TEN)
				{
					ret = ali_i2c_scb_write_read(adap->nr, pmsg->addr, pmsg->buf, 1, pmsg->len);
				}
				else if(pmsg->flags&I2C_M_REV_DIR_ADDR)
				{
					ret = i2c_scb_write_read_std(adap->nr, pmsg->addr, pmsg->buf, 2,  pmsg->len);
				}
				else
				{
					ret = ali_i2c_scb_read(adap->nr, pmsg->addr, pmsg->buf, pmsg->len);
				}
			}
			else
				ret = ali_i2c_scb_write(adap->nr, pmsg->addr, pmsg->buf, pmsg->len);

			if (ret)
				return ret;

		}
		PRINTK_INFO("transfer complete\n");
		pmsg++;		/* next message */
	}
	return i;
}

/*
 * Return list of supported functionality.
 */
static UINT32 ali_i2c_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR;
}

static struct i2c_algorithm ali_i2c_algorithm = {
	.master_xfer	= ali_i2c_xfer,
	.functionality	= ali_i2c_func,
};


static int __init ali_i2c_probe(struct platform_device *pdev)
{
	struct i2c_adapter *adapter;
	UINT32 bps;
	INT32 rc;

	adapter = kzalloc(sizeof(struct i2c_adapter), GFP_KERNEL);
	if (adapter == NULL) {
		PRINTK_INFO("can't allocate i2c adapter inteface! %d\n", pdev->id);
		rc = -ENOMEM;
		return rc;
	}
	sprintf(adapter->name, "ALI_I2C_%d", pdev->id);
	adapter->algo = &ali_i2c_algorithm;
	adapter->class = I2C_CLASS_HWMON | I2C_CLASS_SPD;;
	adapter->dev.parent = &pdev->dev;	
	adapter->nr = pdev->id;

	platform_set_drvdata(pdev, adapter);

	
	bps=100000;
	if(pdev->id==1)
	{
		bps=40000;
	}
	PRINTK_INFO("I2C id: %d, bps: %d\n",pdev->id, bps);

	if (pdev->id < I2C_SCB_NUM)
	{		
		/* Disable interrupt */
		SCB_WRITE8(pdev->id, SCB_IER, SCB_READ8(pdev->id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(pdev->id, SCB_ISR, (UINT8)~SCB_IERE);

		/* Timing based on SB_CLK, for SB_CLK is 12MHz */
		SCB_WRITE8(pdev->id, SCB_HPCC, 6000000/bps);
		SCB_WRITE8(pdev->id, SCB_LPCC, 6000000/bps);

		SCB_WRITE8(pdev->id, SCB_PSUR, 6000000/bps);	/* 1/2 clock period */
		SCB_WRITE8(pdev->id, SCB_PHDR, 6000000/bps);	/* 1/2 clock period */
		SCB_WRITE8(pdev->id, SCB_RSUR, 6000000/bps);	/* 1/2 clock period */
		SCB_WRITE8(pdev->id, SCB_SHDR, 6000000/bps);	/* 1/2 clock period */

		SCB_WRITE8(pdev->id, SCB_FCR, 0x80);	/* Clear FIFO */

		if (1)
		{
			SCB_WRITE8(pdev->id, SCB_HCR, SCB_HCE);	/* DNEE can set only when start transmit */
		} 
		else
		{
			SCB_WRITE8(pdev->id, SCB_HCR, 0x00);
		}
	}

	rc=i2c_add_numbered_adapter(adapter);
	if (rc) {
		dev_err(&pdev->dev, "Adapter %s registration failed\n",
				adapter->name);
		goto fail3;
	}

	PRINTK_INFO("ALI i2c adapter inteface %d probed.\n", pdev->id);
	dev_info(&pdev->dev, "ALI i2c bus driver.\n");
	return 0;
	
fail3:
	platform_set_drvdata(pdev, NULL);
	kfree(adapter);

	return rc;
}

static int __exit ali_i2c_remove(struct platform_device *pdev)
{
	INT32 rc;
	struct i2c_adapter *adapter = platform_get_drvdata(pdev);
	
	rc=i2c_del_adapter(adapter);
	platform_set_drvdata(pdev, NULL);
	
	return rc;
}

static struct platform_driver ali_i2c_driver = {
	.driver		= {
		.name	= "ali_i2c_bus",
		.owner	= THIS_MODULE,
	},
	.probe		= ali_i2c_probe,
	.remove		= __devexit_p(ali_i2c_remove),
	.suspend	= NULL, //ali_i2c_suspend,
	.resume		= NULL, //ali_i2c_resume,
};

static int __init ali_i2c_init(void) 
{
	struct platform_device *pd;
	UINT8 i;

	for(i=0;i<I2C_SCB_NUM;i++)
	{
		pd = platform_device_alloc("ali_i2c_bus", i);
		if (pd == NULL) {
			printk(KERN_ERR "ali_i2c_bus: failed to allocate device id %d\n",
			       1);
			return -ENODEV;
		}

		pd->dev.platform_data = NULL;
		platform_device_add(pd);
		mutex_init(&(ali_i2c_bus_mutex[i]));
	}

	return platform_driver_probe(&ali_i2c_driver, ali_i2c_probe);
}

static void __exit ali_i2c_exit(void)
{
	platform_driver_unregister(&ali_i2c_driver);
}

//module_init(ali_i2c_init);
//arch_initcall(ali_i2c_init);
module_exit(ali_i2c_exit);

MODULE_DESCRIPTION("Ali I2C bus Controller driver");
MODULE_AUTHOR("Eric Li");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ali_i2c_bus");


