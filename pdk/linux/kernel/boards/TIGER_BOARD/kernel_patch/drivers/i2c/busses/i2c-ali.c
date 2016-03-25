/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/of_i2c.h>


enum ali_i2c_state {
	STATE_DONE,
	STATE_ERROR,
	STATE_START
};
#define DRIVER_NAME		"i2c-ali"
/**
 * struct ali_i2c
 * @base:	Memory base of the HW registers
 * @wait:	Wait queue for callers
 * @adap:	Kernel adapter representation
 * @tx_msg:	Messages from above to be sent
 * @lock:	Mutual exclusion
 * @tx_pos:	Current pos in TX message
 * @nmsgs:	Number of messages in tx_msg
 * @state:	See ali_i2c_state
 * @rx_msg:	Current RX message
 * @irq:	irq number of I2C
 * @rx_pos:	Position within current RX message
 */
struct ali_i2c {
	struct device		*dev;
	void __iomem		*base;
	wait_queue_head_t	wait;
	struct i2c_adapter	adap;
	struct i2c_msg		*tx_msg;
	spinlock_t		lock;
	unsigned int		tx_pos;
	unsigned int		nmsgs;
	enum ali_i2c_state	state;
	struct i2c_msg		*rx_msg;
	int			rx_pos;
	int			irq;
	u32			bus_clk_rate;
};

/*
 * Register offsets in bytes from RegisterBase.
 */
/* Host Control Register			*/
#define I2C_HCR_REG_OFFSET				(0x00)
/* Host Status Register				*/
#define I2C_HSR_REG_OFFSET				(0x01)
/* Interrupt Enable Register			*/
#define I2C_IER_REG_OFFSET				(0x02)
/* Interrupt Status Register			*/
#define I2C_ISR_REG_OFFSET				(0x03)
/* Slave Address Register			*/
#define I2C_SAR_REG_OFFSET				(0x04)
/* Target serial device address			*/
#define I2C_SSAR_REG_OFFSET				(0x05)
/* High Period Clock Counter Register		*/
#define I2C_HPCC_REG_OFFSET				(0x06)
/* Low Period Clock Counter Register		*/
#define I2C_LPCC_REG_OFFSET				(0x07)
/* Stop Setup Time Register			*/
#define I2C_PSUR_REG_OFFSET				(0x08)
/* Stop Hold Time Register			*/
#define I2C_PHDR_REG_OFFSET				(0x09)
/* Restart Setup Time Register			*/
#define I2C_RSUR_REG_OFFSET				(0x0A)
/* Start Hold Time Register			*/
#define I2C_SHDR_REG_OFFSET				(0x0B)
/* FIFO Control Register			*/
#define I2C_FCR_REG_OFFSET				(0x0C)
/* Device Control Register			*/
#define I2C_DCR_REG_OFFSET				(0x0D)
/* E-DDC Address  Register			*/
#define I2C_DAR_REG_OFFSET				(0x0E)
/* SEG_POINTER Address  Register		*/
#define I2C_SPR_REG_OFFSET				(0x0F)
/* FIFO Data(Tx/Rx) Register			*/
#define I2C_FDR_REG_OFFSET				(0x10)
/* Slave Address(SA) Got Register		*/
#define I2C_AGR_REG_OFFSET				(0x11)
/* Second Slave Address BYTE (SA) Got Register	*/
#define I2C_SGR_REG_OFFSET				(0x12)
/* Slave Device State Register			*/
#define I2C_SSR_REG_OFFSET				(0x13)
/* FIFO Trigger Interrupt Enable Register	*/
#define I2C_IER1_REG_OFFSET				(0x20)
/* FIFO Trigger Interrupt Status Register	*/
#define I2C_ISR1_REG_OFFSET				(0x21)
/* FIFO Dept Register				*/
#define I2C_FFDR_REG_OFFSET				(0x22)
/* Byte Number Expect Transfer One Time Register*/
#define I2C_BCR_REG_OFFSET				(0x23)
/* Enable x Sub-slave Address Byte Register	*/
#define I2C_SSAR_EN_REG_OFFSET				(0x24)
/* The 2nd Sub-slave Address Byte Register	*/
#define I2C_SSAR2_REG_OFFSET				(0x25)
/* The 3rd Sub-slave Address Byte Register	*/
#define I2C_SSAR3_REG_OFFSET				(0x26)
/* The 4th Sub-slave Address Byte Register	*/
#define I2C_SSAR4_REG_OFFSET				(0x27)

/*
*Control Register masks
*/
/* Master Start Transaction Bit			*/
#define I2C_HCR_START_TRANSCTION_MASK			BIT(0)
/* Command Protocol Bit				*/
#define I2C_HCR_COMMAND_PROTOCOL_MASK			(BIT(1)|BIT(2)|BIT(3))
/* Repeated start				*/
#define I2C_HCR_REPEATED_START_MASK			BIT(4)
/* EDDC_OPERATION				*/
#define I2C_HCR_EDDC_OPERATION_MASK			BIT(5)
/* Device Not Exist Enable/Disable		*/
#define I2C_HCR_DEVICE_NOT_EXIST_MASK			BIT(6)
/* Host Controller Enable/Disable		*/
#define I2C_HCR_HOST_CONCTOLLER_MASK			BIT(7)

/* 1.843M/12M clock for I2C interface Bit*/
#define I2C_DCR_CLOCK_MASK				BIT(3)
/* FIFO Flush Bit				*/
#define I2C_FCR_FIFO_MASK				BIT(7)
/* Byte Count (BC) Bit				*/
#define I2C_FCR_BC_MASK					0x1F
/* FIFO Bit when the data are read from the FIFO*/
#define I2C_HSR_RFIFO_MASK				BIT(0)
/* FIFO Bit when the data are written from the FIFO*/
#define I2C_HSR_WFIFO_MASK				BIT(1)
/* Host Busy Bit				*/
#define I2C_HSR_HB_MASK					BIT(5)
/* Data for Cacluating Period Counter When Using 12M Clock */
#define I2C_12M_CLOCK_CONSTANT				6000000


/* FIFO_EMPTY Interrupt Enable/Disable Bit	*/
#define I2C_IER_FEIE_MASK				BIT(7)
/* FIFO_FULL Interrupt Enable/Disable Bit	*/
#define I2C_IER_FFIE_MASK				BIT(6)
/* S_P Interrupt Enable/Disable Bit		*/
#define I2C_IER_SPIE_MASK				BIT(5)
/* SLAVE_SELECTED Interrupt Enable/Disable Bit	*/
#define I2C_IER_SSIE_MASK				BIT(4)
/* Arbiter Lost Interrupt Enable/Disable Bit*/
#define I2C_IER_ARBLIE_MASK				BIT(3)
/* Device Busy Interrupt Enable/Disable Bit	*/
#define I2C_IER_DBIE_MASK				BIT(2)
/* Device Not Exist Interrupt Enable/Disable Bit*/
#define I2C_IER_DNEE_MASK				BIT(1)
/* Transaction Done Interrupt Enable/Disable Bit*/
#define I2C_IER_TDIE_MASK				BIT(0)
/* FIFO Trigger Interrupt Enable/Disable Bit	*/
#define I2C_IER1_TRIGGER_MASK				BIT(0)
/* Enable x sub-slave address byte Mask		*/
#define I2C_SSAR_EN_REG_MASK				(BIT(0)|BIT(1))

/* Write Operation				*/
#define I2C_HCR_WT					0x00
/* Current Address Read Operation		*/
#define I2C_HCR_CAR					0x04
/* Sequential Read Operation			*/
#define I2C_HCR_SER					0x08
/* Standard Read Operation			*/
#define I2C_HCR_STR					0x0c


/* The following constants specify the depth of the FIFOs */

/* Rx fifo capacity				*/
#define ALI_RX_FIFO_DEPTH				1
/* Tx fifo capacity				*/
#define ALI_TX_FIFO_DEPTH				8
/* Tx fifo Interrupt Dept			*/
#define ALI_TX_INT_DEPTH				8


#define ALI_READ_OPERATION				1
#define ALI_WRITE_OPERATION				0

#define ali_tx_space(i2c) ((i2c)->tx_msg->len - (i2c)->tx_pos)
#define ali_rx_space(i2c) ((i2c)->rx_msg->len - (i2c)->rx_pos)

static void ali_start_xfer(struct ali_i2c *i2c);
static void __ali_start_xfer(struct ali_i2c *i2c);

static inline u32 ali_reg_read(struct ali_i2c *i2c, unsigned long reg)
{
	return ioread8(i2c->base + reg);
}

static inline void ali_reg_write(struct ali_i2c *i2c, unsigned long reg,
	u32 val)
{
	iowrite8(val, i2c->base + reg);
}

static inline void ali_irq_dis(struct ali_i2c *i2c, u32 mask)
{
	u32 val = ali_reg_read(i2c, I2C_IER_REG_OFFSET);
	ali_reg_write(i2c, I2C_IER_REG_OFFSET, val & (~mask));
}

static inline void ali_irq_en(struct ali_i2c *i2c, u32 mask)
{
	u32 val = ali_reg_read(i2c, I2C_IER_REG_OFFSET);
	ali_reg_write(i2c, I2C_IER_REG_OFFSET, val | mask);
}

static inline void ali_irq_clr(struct ali_i2c *i2c, u32 mask)
{
	u32 val = ali_reg_read(i2c, I2C_ISR_REG_OFFSET);
	ali_reg_write(i2c, I2C_ISR_REG_OFFSET, val | mask);
}

static inline void ali_all_irq_clr(struct ali_i2c *i2c)
{
	u32 val;

	/* Disable All interrupt */
	val = ali_reg_read(i2c, I2C_IER_REG_OFFSET);
	val &= ~0xFF;
	ali_reg_write(i2c, I2C_IER_REG_OFFSET, val);

	/* Disable Trigger interrupt */
	val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
	val &= ~I2C_IER1_TRIGGER_MASK;
	ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val);
}

static inline void ali_irq_clr_en(struct ali_i2c *i2c, u32 mask)
{
	ali_irq_clr(i2c, mask);
	ali_irq_en(i2c, mask);
}

static void ali_clear_rx_fifo(struct ali_i2c *i2c)
{
	u32 val;

	for (val = ali_reg_read(i2c, I2C_HSR_REG_OFFSET);
		!(val & I2C_HSR_RFIFO_MASK);
		val = ali_reg_read(i2c, I2C_HSR_REG_OFFSET))
		ali_reg_read(i2c, I2C_FDR_REG_OFFSET);
}

static void ali_i2c_init(struct ali_i2c *i2c)
{
	u32 val;
	/* Host Controller Enable */
	val = ali_reg_read(i2c, I2C_HCR_REG_OFFSET);
	val |= I2C_HCR_HOST_CONCTOLLER_MASK;
	ali_reg_write(i2c, I2C_HCR_REG_OFFSET, val);

	/*Using 12M clock for I2C interface */
	val = ali_reg_read(i2c, I2C_DCR_REG_OFFSET);
	val &= I2C_IER_ARBLIE_MASK;
	ali_reg_write(i2c, I2C_DCR_REG_OFFSET, val);

	/* Set I2C Clock and Timing Regrister */
	val = I2C_12M_CLOCK_CONSTANT / i2c->bus_clk_rate;
	ali_reg_write(i2c, I2C_HPCC_REG_OFFSET, val);
	ali_reg_write(i2c, I2C_LPCC_REG_OFFSET, val);
	ali_reg_write(i2c, I2C_PSUR_REG_OFFSET, val);
	ali_reg_write(i2c, I2C_PHDR_REG_OFFSET, val);
	ali_reg_write(i2c, I2C_RSUR_REG_OFFSET, val);
	ali_reg_write(i2c, I2C_SHDR_REG_OFFSET, val);

	/* Flushes FIFO and Resets The FIFO Point */
	val = ali_reg_read(i2c, I2C_FCR_REG_OFFSET);
	val |= I2C_FCR_FIFO_MASK | I2C_IER_SSIE_MASK;
	ali_reg_write(i2c, I2C_FCR_REG_OFFSET, val);

	/* Make Sure RX FIFO is Empty */
	ali_clear_rx_fifo(i2c);

	/* Disable interrupt */
	ali_all_irq_clr(i2c);
}


static void ali_deinit(struct ali_i2c *i2c)
{
	u32 val;

	/* Flushes FIFO and Resets The FIFO Point */
	val = ali_reg_read(i2c, I2C_FCR_REG_OFFSET);
	val |= I2C_FCR_FIFO_MASK;
	ali_reg_write(i2c, I2C_FCR_REG_OFFSET, val);

	ali_all_irq_clr(i2c);
}

static void ali_read_rx(struct ali_i2c *i2c)
{
	u8 bytes_in_fifo;
	int i;

	bytes_in_fifo = ali_reg_read(i2c, I2C_FCR_REG_OFFSET) & I2C_FCR_BC_MASK;
	if (!(ali_reg_read(i2c, I2C_HSR_REG_OFFSET) & I2C_HSR_RFIFO_MASK)) {
		for (i = 0; i < bytes_in_fifo; i++)
			i2c->rx_msg->buf[i2c->rx_pos++] =
				ali_reg_read(i2c, I2C_FDR_REG_OFFSET);
	}

}

static int ali_tx_fifo_space(struct ali_i2c *i2c)
{
	return ALI_TX_FIFO_DEPTH;
}

static void ali_fill_tx_fifo(struct ali_i2c *i2c)
{
	u8 fifo_space = ali_tx_fifo_space(i2c);
	int len = ali_tx_space(i2c);

	len = (len > fifo_space) ? fifo_space : len;

	while (len--) {
		u16 data = i2c->tx_msg->buf[i2c->tx_pos++];
		ali_reg_write(i2c, I2C_FDR_REG_OFFSET, data);
	}
}

static void ali_wakeup(struct ali_i2c *i2c, int code)
{
	i2c->tx_msg = NULL;
	i2c->rx_msg = NULL;
	i2c->nmsgs = 0;
	i2c->state = code;
	wake_up(&i2c->wait);
}

static inline void ali_process(struct ali_i2c *i2c)
{
	u32 pend, isr, ier;
	u32 pend1, isr1, ier1;
	u32 clr = 0, clr1 = 0;
	u32 val;

	/* Get the interrupt Status from the ISR. There is no clearing of
	 * interrupts in the IPIF. Interrupts must be cleared at the source.
	 * To find which interrupts are pending; AND interrupts pending with
	 * interrupts masked.
	 */
	isr = ali_reg_read(i2c, I2C_ISR_REG_OFFSET);
	ier = ali_reg_read(i2c, I2C_IER_REG_OFFSET);
	pend = isr & ier;

	isr1 = ali_reg_read(i2c, I2C_ISR1_REG_OFFSET);
	ier1 = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
	pend1 = isr1 & ier1;
	/* Do not processes a devices interrupts if the device has no
	 * interrupts pending
	 */
	if (!pend && !pend1)
		return;
	/* Service requesting interrupt */
	if ((pend & I2C_IER_ARBLIE_MASK) ||
		(pend & I2C_IER_DNEE_MASK) ||
		(pend & I2C_IER_DBIE_MASK)) {
		/* bus arbritration lost, or...
		 * Transmit error,TX error
		 */
		dev_err(i2c->dev, "[line:%d] i2c IRQ error\n", __LINE__);
		ali_i2c_init(i2c);
		ali_wakeup(i2c, STATE_ERROR);
	} else if (pend1 & I2C_IER1_TRIGGER_MASK) {

		clr1 = I2C_IER1_TRIGGER_MASK;

		if (i2c->tx_msg->flags & I2C_M_RD) {
			if (!i2c->rx_msg) {
				dev_dbg(i2c->dev, "rx_msg is null\n");
				ali_clear_rx_fifo(i2c);
				goto out;
			}

			ali_read_rx(i2c);
			if (ali_rx_space(i2c) == 0) {
				val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
				ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val &
					~I2C_IER1_TRIGGER_MASK);
			}
		} else {
			if (!i2c->tx_msg) {
				dev_err(i2c->dev, "tx_msg is null\n");
				goto out;
			}

			ali_fill_tx_fifo(i2c);

			if (ali_tx_space(i2c) == 0) {
				val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
				ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val &
					~I2C_IER1_TRIGGER_MASK);
			}
		}
	} else if (pend & I2C_IER_TDIE_MASK) {
		/* IIC bus has transitioned to not busy */
		clr = I2C_IER_TDIE_MASK;
		/*Completing the current transaction,Disable IRQ*/
		ali_irq_dis(i2c, I2C_IER_TDIE_MASK);
		if (!i2c->tx_msg)
			goto out;
		if ((i2c->nmsgs == 1) && (ali_tx_space(i2c) == 0) &&
			(ali_tx_space(i2c) == 0))
			ali_wakeup(i2c, STATE_DONE);
		else
			ali_wakeup(i2c, STATE_ERROR);

	} else {
		/* got IRQ which is not acked */
		dev_dbg(i2c->dev, "[line:%d] get unexpected IRQ\n", __LINE__);
		clr = pend;
		clr1 = pend1;
	}
out:
	ali_reg_write(i2c, I2C_ISR_REG_OFFSET, clr);
	ali_reg_write(i2c, I2C_ISR1_REG_OFFSET, clr1);
}

static int ali_i2c_busy(struct ali_i2c *i2c)
{
	int tries = 3;
	u32 val;

	if (i2c->adap.retries)
		tries = i2c->adap.retries;
	/* for instance if previous transfer was terminated due to TX error
	 * it might be that the bus is on it's way to become available
	 * give it at most 3 ms to wake
	 */
	while (val && tries--) {
		val = ali_reg_read(i2c, I2C_HSR_REG_OFFSET);
		val &= I2C_HSR_HB_MASK;
		mdelay(1);
	}

	return (val) ? -EBUSY : 0;
}
static void ali_start_recv(struct ali_i2c *i2c)
{
	int val;
	struct i2c_msg *msg = i2c->rx_msg = i2c->tx_msg;

	dev_dbg(i2c->dev, "[line:%d] entry,want rx data len:%d\n",
		__LINE__, msg->len);

	if (!(msg->flags & I2C_M_NOSTART)) {
		val = ((msg->addr << 1) | ALI_READ_OPERATION);
		ali_reg_write(i2c, I2C_SAR_REG_OFFSET, val);
		
		val = ali_reg_read(i2c, I2C_HCR_REG_OFFSET);
		
		val &= ~I2C_HCR_COMMAND_PROTOCOL_MASK;
		ali_reg_write(i2c, I2C_HCR_REG_OFFSET, val | I2C_HCR_SER);

		ali_reg_write(i2c, I2C_BCR_REG_OFFSET, msg->len &
			(~I2C_FCR_BC_MASK));
		ali_reg_write(i2c, I2C_FCR_REG_OFFSET, (msg->len &
			I2C_FCR_BC_MASK) | I2C_FCR_FIFO_MASK);
	}

	ali_irq_clr_en(i2c, I2C_IER_TDIE_MASK | I2C_IER_DNEE_MASK |
		I2C_IER_DBIE_MASK | I2C_IER_ARBLIE_MASK);
	val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
	ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val | I2C_IER1_TRIGGER_MASK);
	
	ali_reg_write(i2c, I2C_FFDR_REG_OFFSET, ALI_RX_FIFO_DEPTH);
	val = ali_reg_read(i2c, I2C_HCR_REG_OFFSET);
	val |= I2C_HCR_START_TRANSCTION_MASK | I2C_HCR_DEVICE_NOT_EXIST_MASK;
	ali_reg_write(i2c, I2C_HCR_REG_OFFSET, val);
	
	/* tx message is end */
	i2c->tx_pos = i2c->tx_msg->len;
}

static void ali_start_send(struct ali_i2c *i2c)
{
	struct i2c_msg *msg = i2c->tx_msg;
	int val, i;
	ali_irq_clr(i2c, I2C_IER_FEIE_MASK);

	dev_dbg(i2c->dev, "[line:%d] HSR: 0x%x, HCR: 0x%x\n",
		__LINE__, ali_reg_read(i2c, I2C_HSR_REG_OFFSET),
		ali_reg_read(i2c, I2C_HCR_REG_OFFSET));

	if (i2c->nmsgs > 1) {
		/* write the address */
		val = ((msg->addr << 1) | ALI_WRITE_OPERATION);
		ali_reg_write(i2c, I2C_SAR_REG_OFFSET, val);
		dev_dbg(i2c->dev, "[line:%d] dev addr:0x%x\n", __LINE__,
				ali_reg_read(i2c, I2C_SAR_REG_OFFSET));
		
		ali_reg_write(i2c, I2C_SSAR_REG_OFFSET, msg->buf[0]);
		dev_dbg(i2c->dev, "[line:%d] word addr[0]:0x%x\n", __LINE__,
				ali_reg_read(i2c, I2C_SSAR_REG_OFFSET));
		
		val = (msg->len - 1) & I2C_SSAR_EN_REG_MASK;
		ali_reg_write(i2c, I2C_SSAR_EN_REG_OFFSET, val);

		for (i = 0; i < val; i++) {
			val = msg->buf[1 + i];
			ali_reg_write(i2c, I2C_SSAR2_REG_OFFSET + i, val);
			dev_dbg(i2c->dev, "[line:%d] word addr[%d]:0x%x\n",
				__LINE__, 1 + i, val);
			
		}
		val = ali_reg_read(i2c, I2C_HCR_REG_OFFSET);
		val &= ~I2C_HCR_COMMAND_PROTOCOL_MASK;
		ali_reg_write(i2c, I2C_HCR_REG_OFFSET, val | I2C_HCR_SER);
	} else {
		ali_reg_write(i2c, I2C_BCR_REG_OFFSET, msg->len &
			(~I2C_FCR_BC_MASK));
		ali_reg_write(i2c, I2C_FCR_REG_OFFSET,
			I2C_FCR_FIFO_MASK | (msg->len & I2C_FCR_BC_MASK));

		if (!(msg->flags & I2C_M_NOSTART)) {
			/* write the address */
			val = ((msg->addr << 1) | ALI_WRITE_OPERATION);
			ali_reg_write(i2c, I2C_SAR_REG_OFFSET, val);
			dev_dbg(i2c->dev, "[line:%d]dev addr:0x%x\n", __LINE__,
				ali_reg_read(i2c, I2C_SAR_REG_OFFSET));
			
			val = ali_reg_read(i2c, I2C_HCR_REG_OFFSET);
			val &= ~I2C_HCR_COMMAND_PROTOCOL_MASK;
			val |= I2C_HCR_WT;
			ali_reg_write(i2c, I2C_HCR_REG_OFFSET, val);
		}

		for (i = 0; i < i2c->tx_msg->len; i++) {
			dev_dbg(i2c->dev, "[line:%d] tx[%d]:0x%x\n", __LINE__,
				i, i2c->tx_msg->buf[i]);
			
		}

		ali_fill_tx_fifo(i2c);

		/* Clear any pending Tx empty,Error and then enable them. */
		ali_irq_clr_en(i2c, I2C_IER_TDIE_MASK | I2C_IER_DNEE_MASK
			| I2C_IER_DBIE_MASK | I2C_IER_ARBLIE_MASK);
		val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
		val |= I2C_IER1_TRIGGER_MASK;
		ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val);
		ali_reg_write(i2c, I2C_FFDR_REG_OFFSET, ALI_TX_INT_DEPTH);
		val = ali_reg_read(i2c, I2C_HCR_REG_OFFSET);
		val |= I2C_HCR_START_TRANSCTION_MASK;
		val |= I2C_HCR_DEVICE_NOT_EXIST_MASK;
		ali_reg_write(i2c, I2C_HCR_REG_OFFSET, val);
	}
}

static irqreturn_t ali_isr(int irq, void *dev_id)
{
	struct ali_i2c *i2c = dev_id;
	ali_process(i2c);

	return IRQ_HANDLED;
}

static void __ali_start_xfer(struct ali_i2c *i2c)
{
	int first = 1;

	if (!i2c->tx_msg)
		return;

	i2c->rx_pos = 0;
	i2c->tx_pos = 0;
	i2c->state = STATE_START;
	while (first || (i2c->nmsgs > 1)) {
		if (!first) {
			i2c->nmsgs--;
			i2c->tx_msg++;
			i2c->tx_pos = 0;
			i2c->rx_pos = 0;
		} else
			first = 0;

		if (i2c->tx_msg->flags & I2C_M_RD) {
			dev_dbg(i2c->dev, "[line:%d] ali start recv\n",
				__LINE__);
			
			ali_start_recv(i2c);
		} else {
			dev_dbg(i2c->dev, "[line:%d] i2c start send\n",
				__LINE__);
			
			ali_start_send(i2c);
		}
	}
}

static void ali_start_xfer(struct ali_i2c *i2c)
{
	spin_lock(&i2c->lock);
	__ali_start_xfer(i2c);
	spin_unlock(&i2c->lock);
}

static int ali_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	struct ali_i2c *i2c = i2c_get_adapdata(adap);
	int err;
	
	dev_dbg(i2c->dev, "[line:%d] entry %s SR: 0x%x\n", __LINE__, __func__,
		ali_reg_read(i2c, I2C_HSR_REG_OFFSET));
	
	err = ali_i2c_busy(i2c);
	if (err) {
		dev_err(i2c->dev, "[line:%d] ali_i2c_busy:%d\n", __LINE__, err);
		return err;
	}
	i2c->tx_msg = msgs;
	i2c->nmsgs = num;
	
	ali_start_xfer(i2c);

	if (wait_event_timeout(i2c->wait, (i2c->state == STATE_ERROR) ||
		(i2c->state == STATE_DONE), HZ*3)) {
		return (i2c->state == STATE_DONE) ? num : -EIO;
	} else {
		dev_err(i2c->dev, "[line:%d] wait i2c event timeout\n",
			__LINE__);
		i2c->tx_msg = NULL;
		i2c->rx_msg = NULL;
		i2c->nmsgs = 0;
		return -ETIMEDOUT;
	}
}

static u32 ali_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm ali_algorithm = {
	.master_xfer	= ali_xfer,
	.functionality	= ali_func,
};

static struct i2c_adapter ali_adapter = {
	.owner		= THIS_MODULE,
	.name		= DRIVER_NAME,
	.class		= I2C_CLASS_HWMON | I2C_CLASS_SPD,
	.algo		= &ali_algorithm,
};
struct ali_i2c_platform_data {
	u8				num_devices;
	struct i2c_board_info const	*devices;
};
static int ali_i2c_probe(struct platform_device *pdev)
{
	struct ali_i2c *i2c;
	struct resource *res;
	struct ali_i2c_platform_data  *pdata;
	int ret,i;
	i2c = devm_kzalloc(&pdev->dev, sizeof(*i2c), GFP_KERNEL);
	if (!i2c) {
		dev_err(&pdev->dev, "Could not allocate struct ali_i2c");
		return -ENOMEM;
	}
	i2c->dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		ret = -ENOENT;
		goto resource_missing;
	}
	i2c->irq = res->start;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENOENT;

		goto resource_missing;
	}
	i2c->base = devm_request_and_ioremap(&pdev->dev, res);//devm_ioremap_resource(&pdev->dev, res);//francis
	if (IS_ERR(i2c->base)) {
		ret = PTR_ERR(i2c->base);
		goto fail_ioremap;
	}

	ret = of_property_read_u32(i2c->dev->of_node, "clock-frequency",
					&i2c->bus_clk_rate);
	if (ret)
		i2c->bus_clk_rate = 100000;	/* default clock rate */

	/* hook up driver to tree */
	platform_set_drvdata(pdev, i2c);
	i2c->adap = ali_adapter;
	i2c_set_adapdata(&i2c->adap, i2c);
	i2c->adap.dev.parent = &pdev->dev;
	i2c->adap.dev.of_node = pdev->dev.of_node;

	ali_i2c_init(i2c);

	spin_lock_init(&i2c->lock);
	init_waitqueue_head(&i2c->wait);

	ret = devm_request_irq(&pdev->dev, i2c->irq, ali_isr,
		0, dev_name(&pdev->dev), i2c);
	if (ret) {
		dev_err(i2c->dev, "Cannot claim IRQ\n");
		goto request_irq_failed;
	}

	/* add i2c adapter to i2c tree */
	ret = i2c_add_adapter(&i2c->adap);
	if (ret) {
		dev_err(i2c->dev, "Failed to add adapter\n");
		goto add_adapter_failed;
	}
	/*pdata = (struct ali_i2c_platform_data *) pdev->dev.platform_data;
	
	if (pdata) {
		for (i = 0; i < pdata->num_devices; i++)
			i2c_new_device(&i2c->adap, pdata->devices + i);
	}*/
	of_i2c_register_devices(&i2c->adap);
	
	return 0;

add_adapter_failed:
	devm_free_irq(&pdev->dev, i2c->irq, i2c);
request_irq_failed:
	ali_deinit(i2c);
fail_ioremap:
resource_missing:
	devm_kfree(&pdev->dev, i2c);
	dev_err(i2c->dev, "IRQ or Memory resource is missing\n");
	return ret;
}

static int ali_i2c_remove(struct platform_device *pdev)
{
	struct ali_i2c *i2c = platform_get_drvdata(pdev);

	/* remove adapter & data */
	i2c_del_adapter(&i2c->adap);
	ali_deinit(i2c);
	devm_free_irq(&pdev->dev, i2c->irq, i2c);
	devm_kfree(&pdev->dev, i2c);
	return 0;
}

static const struct of_device_id ali_of_match[] = {
	{ .compatible = "alitech,i2c", },
	{},
};
MODULE_DEVICE_TABLE(of, ali_of_match);

static struct platform_driver ali_i2c_driver = {
	.probe   = ali_i2c_probe,
	.remove  = ali_i2c_remove,
	.driver  = {
		.owner = THIS_MODULE,
		.name = DRIVER_NAME,
		.of_match_table = ali_of_match,
	},
};

module_platform_driver(ali_i2c_driver);
MODULE_AUTHOR("Dennis Dai");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ALi i2c driver");
MODULE_VERSION("1.0.0");

