/*
* ALi I2C SCB Driver.

*/
#include <common.h>
#include <ali/sys_define.h>
#include <i2c.h>
#include <errno.h>

#if 0
#define PRINTF_I2C(x...) printf(x)
#else
#define PRINTF_I2C(x...)
#endif

#define SCB_HCR		 			0x00
#define SCB_HSR					0x01
#define SCB_IER					0x02
#define SCB_ISR					0x03
#define SCB_SAR					0x04
#define SCB_SSAR				0x05
#define SCB_HPCC				0x06
#define SCB_LPCC				0x07
#define SCB_PSUR				0x08
#define SCB_PHDR				0x09
#define SCB_RSUR				0x0A
#define SCB_SHDR				0x0B
#define SCB_FCR					0x0C
#define SCB_FDR					0x10
#define SCB_DDC_ADDR		    0x0E
#define SCB_SEG_PTR				0x0F
#define SCB_IER1				0x20
#define SCB_ISR1				0x21
#define SCB_FIFO_TRIG_LEVEL	    0x22
#define SCB_BC				    0x23
#define SCB_SSAR_EN				0x24
#define SCB_SSAR1				0x25
#define SCB_SSAR2				0x26
#define SCB_SSAR3				0x27
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
#define SCB_IERE				0xf0

#define I2C_SCB_TIMES_OUT		100
#define I2C_SCB_RETRY			10

#define I2C_SCB_NUM				3
#define ERR_TIME_OUT    -34     /* Waiting time out */

u32 g_scb_sys_chip_id = 0;
u32 g_scb_sys_chip_ver =0;

#ifdef ALI_ARM_STB
static struct
{
	u32 reg_base;
	int    irq;
} i2c_scb_reg[I2C_SCB_NUM] = {{0x18018200, 18},{0x18018700,25},{0x18018b00,26},{0x18018d00,3}};
#else
static struct
{
	u32 reg_base;
	int    irq;
} i2c_scb_reg[I2C_SCB_NUM] = {{0xB8018200, 26},{0xB8018700,33},{0xB8018b00,34}};
#endif

#define SCB_READ8(id, reg)			(*((volatile u8 *)(i2c_scb_reg[id].reg_base + reg)))
#define SCB_WRITE8(id, reg, data)	(*((volatile u8 *)(i2c_scb_reg[id].reg_base + reg)) = (data))

int i2c_scb_mode_set(u32 id, int bps, int en);
int i2c_scb_read(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_scb_write(u32 id, u8 slv_addr, u8 *data, int len);
int i2c_scb_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
int i2c_scb_write_read_std(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
int i2c_scb_write_write_read(u32 id,u8 segment_ptr, u8 slv1_addr,u8 slv2_addr, u8 *data, int rlen);
int i2c_scb_eddc_read(u32 id,u8 segment_ptr, u8 slv1_addr,u8 slv2_addr, u8 *data, int rlen);
int i2c_scb_write_plus_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);

static unsigned char *databuf = 0;

int i2c_scb_mode_set(u32 id, int bps, int en)
{
	if (bps <= 0)
	{
		return -1;
	}

	/* Disable interrupt */
	SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
	SCB_WRITE8(id, SCB_ISR, (u8)~SCB_IERE);

	/* Timing based on SB_CLK, for SB_CLK is 12MHz */
	SCB_WRITE8(id, SCB_HPCC, 6000000/bps);
	SCB_WRITE8(id, SCB_LPCC, 6000000/bps);

	SCB_WRITE8(id, SCB_PSUR, 6000000/bps);	/* 1/2 clock period */
	SCB_WRITE8(id, SCB_PHDR, 6000000/bps);	/* 1/2 clock period */
	SCB_WRITE8(id, SCB_RSUR, 6000000/bps);	/* 1/2 clock period */
	SCB_WRITE8(id, SCB_SHDR, 6000000/bps);	/* 1/2 clock period */

	SCB_WRITE8(id, SCB_FCR, 0x80);	/* Clear FIFO */

	if (en)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_HCE);	/* DNEE can set only when start transmit */
	} else
	{
		SCB_WRITE8(id, SCB_HCR, 0x00);
	}

	return 0;
}

static int i2c_scb_wait_host_ready(u32 id)
{
	int i = I2C_SCB_TIMES_OUT;

	while (--i)
	{
		if ((SCB_READ8(id, SCB_HSR) & SCB_HB) == 0)
		{
			return 0;
		}
		__udelay(1000);
	}

	//0x14 bit[2:0] == 0, master is idle, continue send data/cmd.
	if((SCB_READ8(id, 0x14)&0x07) == 0)
	{
		return 0;
	}
	
	PRINTF_I2C("i2c_scb_wait_host_ready: time out\n");
	return ERR_TIME_OUT;
}

static int i2c_scb_wait_dev_ready(u32 id)
{
	int i = I2C_SCB_TIMES_OUT;
	while (--i)
	{
		if ((SCB_READ8(id, SCB_HSR) & (SCB_DB | SCB_DNE | SCB_HB)) == 0)
		{
			return 0;
		} else if ((SCB_READ8(id, SCB_ISR) & 0x0e) != 0)
		{
			PRINTF_I2C("i2c_scb_wait_dev_ready: fail\n");
			return -1;
		}
		__udelay(1000);
	}

	PRINTF_I2C("i2c_scb_wait_dev_ready: time out\n");
	return ERR_TIME_OUT;
}

static int i2c_scb_wait_fifo_ready(u32 id, int len)
{
	int i = I2C_SCB_TIMES_OUT;

	while (--i)
	{
		if ((SCB_READ8(id, SCB_FCR) & SCB_BC_VAL) == (len > 16 ? 16: len))
		{
			return 0;
		}
		__udelay(1000);
	}

	PRINTF_I2C("i2c_scb_wait_fifo_ready: time out\n");
	return ERR_TIME_OUT;
}

static int i2c_scb_wait_trans_done(u32 id)
{
	int i = I2C_SCB_TIMES_OUT;
	u8 status;

	while (--i)
	{
		status = SCB_READ8(id, SCB_ISR);
		if ((status & 1) != 0)
		{
			return 0;
		} else if ((status & 0x0e) != 0)
		{
			PRINTF_I2C("i2c_scb_wait_trans_done: fail(%x)\n", status);
			return -1;
		}
		__udelay(1000);
	}

	PRINTF_I2C("i2c_scb_wait_trans_done: time out\n");
	return ERR_TIME_OUT;
}

/*---------------------------------------------------
int i2c_scb_read(u32 id, u8 slv_addr, u8 *data, u32 len);
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
int i2c_scb_read(u32 id, u8 slv_addr, u8 *data, int len)
{
	int i;
	int timeout;

	if (i2c_scb_wait_host_ready(id) != 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (u8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);	

		SCB_WRITE8(id, SCB_BC, ((len>>5) & 0xff));
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_CAR | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == 0)
		{
			break;
		}
		__udelay(1000); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		PRINTF_I2C("ali_i2c_scb_read: %d times i2c_scb_wait_dev_ready  time out\n", I2C_SCB_RETRY);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_fifo_ready(id, len) != 0)
	{
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
			__udelay(1000);
		}
		if (timeout == 0)
		{
			PRINTF_I2C("ali_i2c_scb_read: fail.\n");
			return -1;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}

	return 0;
}

/*---------------------------------------------------
int i2c_scb_write(u32 id, u8 slv_addr, u8 *data, u32 len);
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
int i2c_scb_write(u32 id, u8 slv_addr, u8 *data, int len)
{
	int i, j;

	if (i2c_scb_wait_host_ready(id) != 0)
	{
		return ERR_TIME_OUT;
	}

	for (j = I2C_SCB_RETRY; j > 0; j--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (u8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);
	 
		SCB_WRITE8(id, SCB_BC, ((len>>5) & 0xff));
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));

		for (i = 0; i < len; i++)
		{
			if (SCB_READ8(id, SCB_HSR) & SCB_FF)
			{
				PRINTF_I2C("ali_i2c_scb_write: fail.\n");
				return -1;
			}
			SCB_WRITE8(id, SCB_FDR, data[i]);
		}

		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_WT | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == 0)
		{
			break;
		}
		__udelay(1000); 			/* wait for 1mS */
	}
	if (j == 0)
	{
		PRINTF_I2C("ali_i2c_scb_write: %d times i2c_scb_wait_dev_ready  time out\n", I2C_SCB_RETRY);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_trans_done(id) != 0)
	{
		return ERR_TIME_OUT;
	}

	return 0;
}

/*---------------------------------------------------
int i2c_scb_write_read(u32 id, u8 slv_addr, u8 *data, u32 wlen, u32 rlen);
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
int i2c_scb_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{
	int i, j;
	int timeout;

	if (wlen == 0)
	{
		return i2c_scb_read(id, slv_addr, data, rlen);
	}

	if (i2c_scb_wait_host_ready(id) != 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (u8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);	 

		SCB_WRITE8(id, SCB_SSAR_EN, ((wlen - 1)&0x3));
		for(j = 0; j < ((wlen - 1)&0x3); j++)
			 SCB_WRITE8(id, (SCB_SSAR1 + j), data[j+1]);
		SCB_WRITE8(id, SCB_BC, ((rlen>>5) & 0xff));
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_SER | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == 0)
		{
			break;
		}
		__udelay(1000); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		PRINTF_I2C("ali_i2c_scb_write_read: %d times i2c_scb_wait_dev_ready  time out\n", I2C_SCB_RETRY);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_fifo_ready(id, rlen) != 0)
	{
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
			__udelay(1000);
		}
		if (timeout == 0)
		{
			PRINTF_I2C("ali_i2c_scb_write_read: fail.\n");
			return -1;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}

	return 0;
}

//  Note: the following function i2c_isr() ,i2c_scb_write_multi() and i2c_scb_write_read_multi() are
//for the S3329E5 new feature
static volatile int inx = 0;
static u8 *ubuf;
void i2c_isr(u32 param)
{
	u8 cnt,j;
	u8 id = (u8)param;
	
	if(SCB_READ8(id, SCB_ISR1) & 0x01)
		SCB_WRITE8(id, SCB_ISR1, 0x1);
	else
		SCB_WRITE8(id, SCB_ISR, 0x1);
	cnt = (SCB_READ8(id, SCB_FCR) & 0x1f);
	//static int ii = 0;
	//*(u8 *)(0xa0710000 + ii++) =  cnt;
	//*(u8 *)(0xa0710000 + ii++) =  inx;
	
	for(j = 0; j <cnt; j++)
		ubuf[inx + j] = SCB_READ8(id, SCB_FDR);
	
	inx += cnt;
}


/*---------------------------------------------------
int i2c_scb_write_multi(u32 id, u8 slv_addr, u8 *data, u32 len);
	Perform bytes write process, initially create for S3329E5 new I2C feature,
	can finish more than 16 bytes writing one time
	
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
int i2c_scb_write_multi(u32 id, u8 slv_addr, u8 *data, int len)
{
	int i, j;
	int timeout;

	if (i2c_scb_wait_host_ready(id) != 0)
	{
		return ERR_TIME_OUT;
	}

	for (j = I2C_SCB_RETRY; j > 0; j--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (u8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);
////
    		SCB_WRITE8(id, SCB_BC, ((len>>5) & 0xff));
////    
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));


		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_WT | SCB_ST);

		for (i = 0; i < len; i++)
		{
			for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
			{
				if (!(SCB_READ8(id, SCB_HSR) & SCB_FF))
					break;
				__udelay(1000);
			}
			if (timeout == 0)
			{
				return -1;
			}
			SCB_WRITE8(id, SCB_FDR, data[i]);
			//*((u8 *)(0xa0700000 + i)) =  data[i];
		}

		if (i2c_scb_wait_dev_ready(id) == 0)
		{
			break;
		}
		__udelay(1000); 			/* wait for 1mS */
	}
	if (j == 0)
	{
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_trans_done(id) != 0)
	{
		return ERR_TIME_OUT;
	}

	return 0;
}

/*---------------------------------------------------
int i2c_scb_write_read_multi(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
	Perform bytes write-read process, initially create for S3329E5 to read more than 16 bytes
	one time
	Stream Format:
		S<SLV_W><Write>S<SLV_R><Read>P
		S		: Start
		P		: Stop
		<SLV_W>	: Set Slave addr & Write Mode
		<SLV_R>       : Set Slave addr & Read  Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
int i2c_scb_write_read_multi(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{
	int j;
	int timeout;

	if (wlen == 0)
	{
		return i2c_scb_read(id, slv_addr, data, rlen);
	}

	if (i2c_scb_wait_host_ready(id) != 0)
	{
		return ERR_TIME_OUT;
	}

  	inx = 0;
  	ubuf = data;
	SCB_WRITE8(id, SCB_FIFO_TRIG_LEVEL, 0x8);
	SCB_WRITE8(id, SCB_IER1, 0x1);


	SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
	SCB_WRITE8(id, SCB_HSR, 0x00);
	SCB_WRITE8(id, SCB_IER, (SCB_READ8(id, SCB_IER) & SCB_IERE)|0x01);
	SCB_WRITE8(id, SCB_ISR, (u8)(~(SCB_IERE | 0x01)));
	SCB_WRITE8(id, SCB_SAR, slv_addr);
	SCB_WRITE8(id, SCB_SSAR, data[0]);

	SCB_WRITE8(id, SCB_SSAR_EN, ((wlen - 1)&0x3));
	for(j = 0; j < ((wlen - 1)&0x3); j++)
		 SCB_WRITE8(id, (SCB_SSAR1 + j), data[j+1]);
	SCB_WRITE8(id, SCB_BC, ((rlen>>5) & 0xff));

	SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
	SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
	  SCB_CP_SER | SCB_ST);

	for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
	{
		if(inx >= rlen)
			break;
	
		if ((SCB_READ8(id, SCB_ISR) & 0x0e) != 0)
		{
			SCB_WRITE8(id, SCB_IER1, 0x0);
			return -1;
			//continue;
		}
		__udelay(1000);
	}

	if (timeout == 0)
	{
		SCB_WRITE8(id, SCB_IER1, 0x0);
		return -1;
		//continue;
	}
	SCB_WRITE8(id, SCB_IER1, 0x0);

	return 0;
}


int i2c_scb_write_read_std(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{
	int i,j;
	int timeout;
	
	if (wlen == 0)
	{
		return i2c_scb_read(id, slv_addr, data, rlen);
	}

	if (i2c_scb_wait_host_ready(id) != 0)
	{
		return ERR_TIME_OUT;
	}
	
	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (u8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);

		SCB_WRITE8(id, SCB_SSAR_EN, ((wlen - 1)&0x3));
		for(j = 0; j < ((wlen - 1)&0x3); j++)
			 SCB_WRITE8(id, (SCB_SSAR1 + j), data[j+1]);
		SCB_WRITE8(id, SCB_BC, ((rlen>>5) & 0xff));
		
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
			SCB_CP_STR | SCB_ST);
		
		if (i2c_scb_wait_dev_ready(id) == 0)
		{
			break;
		}
		__udelay(1000); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		return ERR_TIME_OUT;
	}
	
	if (i2c_scb_wait_fifo_ready(id, rlen) != 0)
	{
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
			__udelay(1000);
		}
		if (timeout == 0)
		{			
			return -1;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}
	
	return 0;
}

void scb_get_data(u32 id)
{
	unsigned char count;
	unsigned int i;

	if(databuf == 0)
	{
        PRINTF_I2C("buf address equal zero!\n");
        return ;
	}
	
	count = SCB_READ8(id,SCB_FCR);

	for(i=0;i<count;i++)
	{
		*databuf = SCB_READ8(id,SCB_FDR);
		databuf++;
	}
}
int i2c_scb_eddc_read(u32 id, u8 segment_ptr, u8 slv1_addr, u8 slv2_addr, u8 *data, int rlen)
{
	int i,done_flag,isr_status,isr1_status;

	if (i2c_scb_wait_host_ready(id) != 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_FIFO_TRIG_LEVEL, 0x8);
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (u8)~SCB_IERE);
		SCB_WRITE8(id, SCB_DDC_ADDR, slv1_addr);
		SCB_WRITE8(id, SCB_SEG_PTR, segment_ptr);
		SCB_WRITE8(id, SCB_SAR, slv2_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);

		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
		SCB_WRITE8(id, SCB_BC, (rlen>>5)&0xff);    //set the byte number expect transfer one time
#if 1                                              //fix bug edid cannt be read by C303 DB. open this code and then mask below codes when using to read edid.
		if (i2c_scb_wait_dev_ready(id) == 0)
		{
		    SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		    SCB_CP_SER | SCB_EDDC | SCB_ST);
			break;
		}
#endif

#if 0
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		SCB_CP_SER | SCB_EDDC | SCB_ST);  
		
		if (i2c_scb_wait_dev_ready(id) == 0)
		{
			break;
		}
#endif
		//__udelay(1000); 			// wait for 1mS
	}
	if (i == 0)
	{
		return ERR_TIME_OUT;
	}

	/*if (i2c_scb_wait_fifo_ready(id, rlen) != 0)
	{
		return ERR_TIME_OUT;
	}*/
	databuf = data;
	done_flag = 0;
	while(!done_flag){
		isr_status = SCB_READ8(id,SCB_ISR);
		isr1_status = SCB_READ8(id,SCB_ISR1);
		if(isr_status & ISR_TDI)
		{
			scb_get_data(id);
			SCB_WRITE8(id,SCB_ISR,isr_status);
			done_flag = 1;
		}
		if(isr1_status & ISR1_TRIG)
		{
			scb_get_data(id);
			SCB_WRITE8(id,SCB_ISR1,isr1_status);
		}
 		if ((isr_status & 0x0e)!= 0)
		{
			return -1;
		}
//		delay(1); // delay could make i2c read lost data.
	}
	
/*	for (i = 0; i < rlen; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
		 	if (SCB_READ8(id, SCB_FCR) & 0x1F)
		 	{
		 		break;
		 	}
			__udelay(1000);
		}
		if (timeout == 0)
		{
			return -1;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
		libc_printf("0x%2x",data[i]);
	}*/

	return 0;
}

int i2c_scb_write_write_read(u32 id, u8 segment_ptr, u8 slv1_addr, u8 slv2_addr, u8 *data, int rlen)
{
	u32 count;
	u8 segment_point = segment_ptr;
	u32 word_offset = data[0], r_lens;
	u32 compare_len = 128;
	u8 *data_buffer = data;

	count = rlen;
	
	while(count > 0)
	{
		r_lens = (count > compare_len)? compare_len : count;
		
		if(word_offset == 256)
		{
			segment_point++;
			word_offset = 0;
		}
		else if((word_offset + r_lens) > 256)
		{
			r_lens = 256 - word_offset;
		}

		*data_buffer = word_offset;
				
		if(i2c_scb_eddc_read(id, segment_point, slv1_addr, slv2_addr, data_buffer, r_lens) != 0)
		{
			PRINTF_I2C("read error!\n");
			return -1;
		}
		
		word_offset += r_lens;
		data_buffer += r_lens;
		count -= r_lens;
	}

	return 0;
}
static int i2c_scb_write_internal(u32 id, u8 slv_addr, u8 *data, int len)
{
	int i, j;
	
	if (i2c_scb_wait_host_ready(id) != 0)
	{
		return ERR_TIME_OUT;
	}

	for (j = I2C_SCB_RETRY; j > 0; j--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (u8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);


		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));

		for (i = 0; i < len; i++)
		{
			if (SCB_READ8(id, SCB_HSR) & SCB_FF)
			{
				return -1;
			}
			SCB_WRITE8(id, SCB_FDR, data[i]);
		}

		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_WT | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == 0)
		{
			break;
		}
		__udelay(1000); 			/* wait for 1mS */
	}
	if (j == 0)
	{
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_trans_done(id) != 0)
	{
		return ERR_TIME_OUT;
	}
	return 0;
}


static int i2c_scb_read_internal(u32 id, u8 slv_addr, u8 *data, int len)
{
	int i;
	int timeout;
		
	if (i2c_scb_wait_host_ready(id) != 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (u8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);

		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_CAR | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == 0)
		{
			break;
		}
		__udelay(1000); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_fifo_ready(id, len) != 0)
	{
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
			__udelay(1000);
		}
		if (timeout == 0)
		{
			return -1;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}
	return 0;
}
/*---------------------------------------------------
int i2c_scb_write_plus_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
	Perform bytes read by the following 2 steps: 
	1. write a register address to device which we want to read from
	Start<Write SLV_ADDR><Write register addr>STOP
	2. read bytes
	Start<Write SLV_ADDR><Read byte0><Read byte1>.....<Read byten>STOP

	Arguments:
		slv_addr - Slave Address
		data    - data[0]:register address we want to read from
		rlen - data length we want to read
	Return value:
		0 
		ERR_FAILUE
		ERR_TIME_OUT
----------------------------------------------------*/

int i2c_scb_write_plus_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen)
{	
	int ret;

	ret = i2c_scb_write_internal(id, slv_addr, data, wlen);
	if(ret != 0)
	{
		return ret;
	}
	ret = i2c_scb_read_internal(id, slv_addr, data, rlen);

	return ret;
}

