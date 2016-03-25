/*
 * Driver for ALi SPI Controllers
 *
 * Copyright (C) 2015 ALi Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <mtd/mtd-abi.h>
#include <linux/device.h>
#include <linux/of.h>
#include "ali_spi.h"

#include <ali_reg.h>

/*
 * The core SPI transfer engine just talks to a register bank to set up
 * DMA transfers; transfer queue progress is driven by IRQs.  The clock
 * framework provides the base clock, subdivided for each spi_device.
 *
 * Newer controllers, marked with "new_1" flag, have:
 *  - CR.LASTXFER
 *  - SPI_MR.DIV32 may become FDIV or must-be-zero (here: always zero)
 *  - SPI_SR.TXEMPTY, SPI_SR.NSSR (and corresponding irqs)
 *  - SPI_CSRx.CSAAT
 *  - SPI_CSRx.SBCR allows faster clocking
 */

static u8 sfreg_read8(struct ali_spi *as, u32 reg)
{
	return	ioread8(as->ali_sflash_base_addr  + reg);
}

static void sfreg_write8(struct ali_spi *as, u8 val, u32 reg)
{
	iowrite8(val, as->ali_sflash_base_addr + reg);
}

static u32 sfreg_read32(struct ali_spi *as, u32 reg)
{
	return ioread32(as->ali_sflash_base_addr + reg);
}

static void sfreg_write32(struct ali_spi *as, u32 val, u32 reg)
{
	iowrite32(val, as->ali_sflash_base_addr + reg);
}

static u32 soc_read32(void __iomem  *reg_addr)
{
	return ioread32(reg_addr);
}

static void soc_write32(void __iomem  *reg_addr, u32 val)
{
	iowrite32(val, reg_addr);
}

#if defined(CONFIG_ALI_CHIP_M3921)
#define ALI_SOC_REG_BASE_PHYS 0x18000000
#define CPU_CTRL_DMA_REG 0x1802e000
static u32 reg_data_store_8c;
static u32 reg_data_store_88;
static u32 reg_data_store_cpu_ctrl;
extern struct mutex ali_sto_mutex ; // For S3921, use mutex and register to select NF/SD/SF/EMMC 
static void strapin_2_pinmux(void)
{
	u32 value1;

	reg_data_store_8c = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c);
	reg_data_store_88 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c);
	value1 = (value1 & (~(0x01<<31)));
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c) = value1;
}

static void nand_2_nor(void)
{
	u32 value2 ;
	
	value2 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	value2 = (value2 & (~(0x01<<3)));
	value2 = (value2 |(0x01<<19));
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88) = value2;
}

static void pinmux88_2_default(void)
{
	u32 value1;
	
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	value1 = reg_data_store_88;
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88) = value1;		
}

static void pinmux8c_2_default(void)
{
	u32 value1;
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c);
	value1 = reg_data_store_8c;
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c) = value1;
}

static void pinmux_2_default(void)
{
	pinmux88_2_default();
	pinmux8c_2_default();
}

void ali_spinor_pinmux_release(void) // For S3921, use register to select NF/SD/SF/EMMC  
{
	pinmux_2_default();
	//printk("SPI unlock\n");
	mutex_unlock(&ali_sto_mutex);	
 
}

void ali_spinor_pinmux_set(void)  // For S3921, use register to select NF/SD/SF/EMMC 
{
 
	mutex_lock(&ali_sto_mutex);
 
	strapin_2_pinmux();
	nand_2_nor();	
}
u8 spimode;
void cpu_ctrl_modeset()
{
	u32 value,mask;
	reg_data_store_cpu_ctrl= __REG32ALI(CPU_CTRL_DMA_REG);
	if(spimode==3)
	{
		value=reg_data_store_cpu_ctrl |(1<<9);
		
	}else
	{
		mask=(1<<9);
		value=reg_data_store_cpu_ctrl &(~mask);
	}
	__REG32ALI(CPU_CTRL_DMA_REG) = value;
}
void cpu_ctrl_mode_default()
{
	u32 value1;
	value1 = __REG32ALI(CPU_CTRL_DMA_REG);
	value1 = reg_data_store_cpu_ctrl;
	__REG32ALI(CPU_CTRL_DMA_REG) = value1;
}
static void pinmux_2_spi( )
{
	cpu_ctrl_modeset();
 	ali_spinor_pinmux_set();
}
 static void pinmux_2_nand( )
{
 	ali_spinor_pinmux_release();
	cpu_ctrl_mode_default();
}
 #endif

static int
spi_set_flash_ctrl_clk(struct ali_spi *as)
{
	u32 i;

	/* 4 bytes to select, if not found return default 0*/
	for (i = 0; i < 16; i++) {
		/* SCLK = MEM_CLK / [2 * (CLK_SEL + 1)] */
		if (as->flash_ctrl_clk_select == (MEM_CLK / (2 * (i + 1))))
			return i;
	}
	return 0;
}

static int ali_spi_setup(struct spi_device *spi)
{
	struct ali_spi  *as;
	u32 tmp = 0;
	u32 clk_sel = 0;

	dev_dbg(&spi->dev, "%s.\n", __func__);
	as = spi_master_get_devdata(spi->master);
	sfreg_write8(as, SPI_CFG_DEFAULT, ALI_SPI_CFG);
	u8 mode=spi->mode;
	printk("setup mode=%d\n",mode);
#if defined(CONFIG_ALI_CHIP_M3921)	
	if((mode&0x03)==3)
		spimode=3;
	else if((mode&0x03)==0)
		spimode=0;
	else{
		printk("failed %s (%d), mode %d not support\n", __func__, __LINE__,mode);
		return -EFAULT;
	}
#endif	
	sfreg_write8(as, SPI_MODE_DEFAULT, ALI_SPI_MODE);
	sfreg_write8(as, SPI_FMT_DEFAULT, ALI_SPI_FMT);
	sfreg_write8(as, SPI_INS_DEFAULT, ALI_SPI_INS);

#if 0
	/* set bootrom space_change */
	soc_write32(as->cap210_bootrom_space_change_addr, 1);


	/* set soc reg [12] to enable CS1 */
	tmp = soc_read32(as->cap210_soc_reg_addr);
	tmp |= SPI_CS1_ENABLE;
	soc_write32(as->cap210_soc_reg_addr, tmp);

	/* set pinmux to enable CS1 */
	tmp = soc_read32(as->cap210_pinmux_addr);
	tmp &=  ~(PINMUX20_ENABLE);
	soc_write32(as->cap210_pinmux_addr, tmp);
#endif
	/* set ctrl clk select */
	u32 speed_hz=spi->max_speed_hz;
	if(speed_hz<=3000000)
		as->flash_ctrl_clk_select = 3;//54;
	else if(speed_hz>3000000&&speed_hz<300000000)
		as->flash_ctrl_clk_select = speed_hz/1000000;
	else 
		as->flash_ctrl_clk_select =300;
	clk_sel = spi_set_flash_ctrl_clk(as);
	tmp = sfreg_read8(as, ALI_SPI_CFG);
	tmp |= clk_sel;
	sfreg_write8(as, tmp, ALI_SPI_CFG);

	/* set dma mode */
	as->dma_mode = 0;

	return 0;
}

static int
spi_dma_map(struct ali_spi *as, void *buf, u32 tmp_len, int xfter)
{
	struct device	*dev = &as->pdev->dev;

	as->tx_dma = as->rx_dma = INVALID_DMA_ADDRESS;
	if (buf) {
		if (xfter == SPI_DMA_TX) {
			as->tx_dma = dma_map_single(dev,
					buf, tmp_len,
					DMA_TO_DEVICE);
			if (dma_mapping_error(dev, as->tx_dma))
				return -ENOMEM;
		} else if (xfter == SPI_DMA_RX) {
			as->rx_dma = dma_map_single(dev,
					buf, tmp_len,
					DMA_FROM_DEVICE);
			if (dma_mapping_error(dev, as->rx_dma))
				return -ENOMEM;
		}
	}
	return 0;
}

static void
spi_dma_unmap(struct ali_spi *as, dma_addr_t buf, u32 tmp_len, int xfter)
{
	struct device	*dev = &as->pdev->dev;

	if (buf != INVALID_DMA_ADDRESS) {
		if (xfter == SPI_DMA_TX)
			dma_unmap_single(dev, as->tx_dma,
						tmp_len, DMA_TO_DEVICE);
		else if (xfter == SPI_DMA_RX)
			dma_unmap_single(dev, as->rx_dma,
						tmp_len, DMA_FROM_DEVICE);
	}
}

static void
spi_dma_transfer(void *buf, u32 tmp_len, struct ali_spi *as , int xfter)
{
	u32 value;
	int ret;

	as->dma_align_flag = 1;
	as->spi_dma_start = 1;

	/*set reg BC*/
	sfreg_write32(as, tmp_len, ALI_SPI_FLASH_COUNTER);

	/* set reg 5C*/
	sfreg_write32(as, 0, ALI_SPI_DMA_FLASH_ADDR);

	/* set reg 60*/
	sfreg_write32(as, tmp_len, ALI_SPI_DMA_LEN);

	if (xfter == SPI_DMA_TX) {

		as->dma_xfer = xfter;
		as->tx_dma_len = tmp_len;

		/* set the DMA buffer map */
		if (as->is_dma_mapped)
			as->tx_dma = as->current_transfer->tx_dma;
		 else {
			ret = spi_dma_map(as, buf, tmp_len, SPI_DMA_TX);
			if (ret < 0) {
				dev_err(&as->pdev->dev, "DMA map Tx fail\n");
				return;
			}

			/*buf map but addr not align*/
			if (as->tx_dma & ALIGN_LEN) {
				as->dma_align_flag = 0;
				as->tx_dma = as->buf_dma;
			}
		}

		if (!as->dma_align_flag)
			memcpy(as->buf, buf, tmp_len);

		/* set reg 58*/
		sfreg_write32(as, ((as->tx_dma) & 0x3fffffff),
					ALI_SPI_DMA_MEM_ADDR);

		/*set reg 98*/
		value  = sfreg_read32(as, ALI_SPI_INS);
		value |= (SPI_CONTINUE_COUNT | SPI_CONTINUE_WRITE_CTRL);
		sfreg_write32(as, value, ALI_SPI_INS);

		 /*set reg 64*/
		sfreg_write32(as, (SPI_DMA_TX_CTRL | as->stay->chip_select),
					ALI_SPI_DMA_CTRL);

	} else {
		/* SPI_DMA_RX  */

		as->dma_xfer = xfter;
		as->rx_dma_len = tmp_len;

		/* set the DMA buffer map */
		if (as->is_dma_mapped)
			as->rx_dma = as->current_transfer->rx_dma;
		else {
			ret = spi_dma_map(as, buf, tmp_len, SPI_DMA_RX);
			if (ret < 0) {
				dev_err(&as->pdev->dev, "DMA map Rx fail\n");
				return;
			}

			/*buf map but addr not align*/
			if (as->rx_dma & ALIGN_LEN) {
				as->dma_align_flag = 0;
				as->rx_dma = as->buf_dma;
			}
		}

		/* set reg 58*/
		sfreg_write32(as, ((as->rx_dma) & 0x3fffffff),
					ALI_SPI_DMA_MEM_ADDR);

		/*set reg 98*/
		value = sfreg_read32(as, ALI_SPI_INS);
		value |= (SPI_CONTINUE_COUNT | SPI_CONTINUE_READ_CTRL);
		sfreg_write32(as, value, ALI_SPI_INS);

		 /*set reg 64*/
		sfreg_write32(as, (SPI_DMA_RX_CTRL | as->stay->chip_select),
						ALI_SPI_DMA_CTRL);

	}
}

static void set_multi_io_reg(struct ali_spi *as, int set_mode)
{
	u32 value;

	if (set_mode == MULTI_IO_0) {
		value = sfreg_read8(as, ALI_SPI_MODE);
		value &= ~(0x7);
		sfreg_write8(as, value, ALI_SPI_MODE);
	} else if (set_mode == MULTI_IO_1) {
		value = sfreg_read8(as, ALI_SPI_MODE);
		value |= SPI_IO_MODE_1;
		sfreg_write8(as, value, ALI_SPI_MODE);
	} else if (set_mode == MULTI_IO_5) {
		value = sfreg_read8(as, ALI_SPI_MODE);
		value |= SPI_IO_MODE_5;
		sfreg_write8(as, value, ALI_SPI_MODE);
	} else {
		dev_err(&as->pdev->dev, "not set multi io reg\n");
	}
}

static void set_multi_io_mode(struct ali_spi *as, u8 cmd)
{
	if (as->spi_cmd_flag && (cmd == ALI_SPI_RX_DUAL
		|| cmd == ALI_SPI_NOR_RX_DUAL
		|| cmd == ALI_SPI_RX_QUAD
		|| cmd == ALI_SPI_NOR_RX_QUAD
		|| cmd == ALI_SPI_TX_QUAD)) {
		switch (cmd) {
		case ALI_SPI_RX_DUAL:
		case ALI_SPI_NOR_RX_DUAL:
				as->spi_rx_dual = 1;
				break;

		case ALI_SPI_RX_QUAD:
		case ALI_SPI_NOR_RX_QUAD:
				as->spi_rx_quad = 1;
				break;

		case ALI_SPI_TX_QUAD:
				as->spi_tx_quad = 1;
				break;

		default:
				dev_err(&as->pdev->dev, "not set io mode\n");
				break;
		}
	}
}

static void set_cs_reg(struct ali_spi *as, int set_mode)
{
	u32 value;

	if (set_mode == CS_ENABLE) {
		value = sfreg_read32(as, ALI_SPI_CS);
		value |= (SPI_CS_CTRL_ENABLE | SPI_CS_ENABLE);
		sfreg_write32(as, value,  ALI_SPI_CS);
	} else if (set_mode == CS_DISABLE) {
		value = sfreg_read32(as, ALI_SPI_CS);
		value &= ~(SPI_CS_CTRL_ENABLE);
		sfreg_write32(as, value,  ALI_SPI_CS);
	} else {
		dev_err(&as->pdev->dev, "not set chip select reg\n");
	}
}

static void ali_spi_bedug(void *buf, u32 len)
{
	int i;

	if (len == 1) {
		printk("cmd %02x\n", *(u8*)buf);
	} else {
		printk("Data: \n");
		for (i = 0; i < len; i++) {
			if ((i%16) == 0)
				printk("\n");
			printk("%02x", *(u8*)(buf + i));
		}
		printk("\n");
	}
}

static void transfer_data_tx(void *tx_buf, u32 len, struct ali_spi *as)
{
	u32 tmp_len;
	u32 ali_flash_cs_addr;
	u32 addr, i;
	u32 tmp = 0;
#if 0
	tmp = soc_read32(as->cap210_soc_dram_ctl_addr);
#endif
	//ali_spi_bedug(tx_buf, len);

	ali_flash_cs_addr =
			(u32)as->ali_sflash_cs_addr[as->stay->chip_select];
	if (tmp & SOC_DRAM_CTL)
		ali_flash_cs_addr |= SOC_DRAM_SET;
	tmp_len = len;
	sfreg_write8(as, SPI_HIT_DATA, ALI_SPI_FMT);
	set_multi_io_mode(as, *(u8 *)tx_buf);
	set_multi_io_reg(as, MULTI_IO_0);

	/* if cmd 6b to set quad so len == 1, but addr is 6b than len > 1 */
	if (as->spi_tx_quad && len >= DMA_MIN_LEN) {
		set_multi_io_reg(as, MULTI_IO_5);
		as->spi_tx_quad = 0;
	}

	if (as->spi_dma_flag && (tmp_len >= DMA_MIN_LEN) &&
						(!(tmp_len & ALIGN_LEN))) {
		spi_dma_transfer(tx_buf, tmp_len, as, SPI_DMA_TX);
	} else {
		i = 0;
		while (tmp_len) {
			addr = *(u8 *)((u32) tx_buf + i);
			*(u8 *) (ali_flash_cs_addr + addr) = addr;
			tmp_len--;
			i++;
		}
	}
}

static void transfer_data_rx(void *rx_buf, u32 len, struct ali_spi *as)
{
	u32 tmp_len;
	u32 ali_flash_cs_addr;
	int i;
	u32 tmp = 0;

#if 0
	tmp = soc_read32(as->cap210_soc_dram_ctl_addr);
#endif
	sfreg_write8(as, SPI_HIT_DATA, ALI_SPI_FMT);
	set_multi_io_reg(as, MULTI_IO_0);

	if (as->spi_rx_dual) {
		set_multi_io_reg(as, MULTI_IO_1);
		as->spi_rx_dual = 0;
	}
	if (as->spi_rx_quad) {
		set_multi_io_reg(as, MULTI_IO_5);
		as->spi_rx_quad = 0;
	}

	ali_flash_cs_addr =
			(u32)as->ali_sflash_cs_addr[as->stay->chip_select];
	if (tmp & SOC_DRAM_CTL)
		ali_flash_cs_addr |= SOC_DRAM_SET;
	tmp_len = len;

	if (as->spi_dma_flag && (tmp_len >= DMA_MIN_LEN) &&
						(!(tmp_len & ALIGN_LEN))) {
		spi_dma_transfer(rx_buf, tmp_len, as, SPI_DMA_RX);
	} else {
		i = 0;

		while (tmp_len) {
			/* align */
			if (((((u32) rx_buf + i) & 3) == 0) && (tmp_len >= 4)) {
				*(u32 *)((u32) rx_buf + i) =
					*(u32 *)(ali_flash_cs_addr);
				tmp_len -= 4;
				i += 4;
			} else {
				while ((((((u32)rx_buf + i) & 3) != 0) ||
						(tmp_len < 4)) && tmp_len) {
					*(u8 *)((u32)rx_buf + i) =
						*(u8 *)(ali_flash_cs_addr);
					i++;
					tmp_len--;
				}
			}
		}
	}
	//ali_spi_bedug(rx_buf, len);
}


static int ali_spi_transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct ali_spi  *as;
	struct spi_transfer *parse_xfer;
	struct spi_transfer *xfer;
	int  status = 0;
	bool cs_flag = 0;

	as = spi_master_get_devdata(spi->master);
	as->stay = spi;
	as->ali_msg = msg;
	msg->actual_length = 0;

	/* dma_start not set==> cmd or pio  */
	as->spi_dma_start = 0;

	/* cmd to set multi-io mode  */
	as->spi_cmd_flag = 1;

	/* set the dma mode by device tree */
	if (as->dma_mode)
		as->spi_dma_flag = as->dma_mode;
	else
		as->spi_dma_flag = 0;

#if defined(CONFIG_ALI_CHIP_M3921)
	pinmux_2_spi();
#endif

	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		/* set pinmux to spi reg C8*/
		set_cs_reg(as, CS_ENABLE);
		parse_xfer = xfer;
		as->current_transfer = xfer;
		as->is_dma_mapped = msg->is_dma_mapped;
		if (parse_xfer->tx_buf != NULL) {
				cs_flag = 1;
				msg->actual_length += parse_xfer->len;
				transfer_data_tx((void *)parse_xfer->tx_buf,
						(u32) parse_xfer->len, as);
				as->spi_cmd_flag = 0;
		}
		if (parse_xfer->rx_buf != NULL) {
				msg->actual_length += parse_xfer->len;
				transfer_data_rx(parse_xfer->rx_buf,
						(u32) parse_xfer->len, as);
				cs_flag = 0;
		}
		
		/*if (parse_xfer->rx_buf != NULL) {
				msg->actual_length += parse_xfer->len;
				transfer_data_rx(parse_xfer->rx_buf,
						(u32) parse_xfer->len, as);
				cs_flag = 0;
		} else  if (parse_xfer->tx_buf != NULL) {
				cs_flag = 1;
				msg->actual_length += parse_xfer->len;
				transfer_data_tx((void *)parse_xfer->tx_buf,
						(u32) parse_xfer->len, as);
				as->spi_cmd_flag = 0;
		}*/
		if (!cs_flag && !as->spi_dma_start) {
			/* set pinmux to default reg C8*/
			set_cs_reg(as, CS_DISABLE);
		}
	}
	if (!as->spi_dma_start) {
		/* set pinmux to default reg C8*/
		set_cs_reg(as, CS_DISABLE);
		msg->status = status;
		msg->complete(msg->context);
	}

#if defined(CONFIG_ALI_CHIP_M3921)
	pinmux_2_nand();
#endif

	return 0;

}

static void ali_spi_cleanup(struct spi_device *spi)
{
	dev_dbg(&spi->dev, "%s.\n", __func__);
}

/*-------------------------------------------------------------------------*/

/*
 *  alidev_udc_irq - interrupt handler
 */

static irqreturn_t ali_spi_irq(int dummy, void *as)
{
	struct ali_spi *irq_as = (struct ali_spi *)as;
	u32 value;

	sfreg_write32(irq_as,
				SPI_DMA_INT_CLEAN, ALI_SPI_DMA_INT_STATUS);
	sfreg_write32(irq_as, 0x0, ALI_SPI_DMA_CTRL);

	if (irq_as->dma_xfer == SPI_DMA_TX) {
		value = sfreg_read32(irq_as, ALI_SPI_INS);
		value &= ~(SPI_CONTINUE_COUNT | SPI_CONTINUE_WRITE_CTRL);
		sfreg_write32(irq_as, value, ALI_SPI_INS);

		if (!irq_as->is_dma_mapped)
			spi_dma_unmap(irq_as, irq_as->tx_dma,
					irq_as->tx_dma_len, SPI_DMA_TX);
	} else {
		value = sfreg_read32(irq_as, ALI_SPI_INS);
		value &= ~(SPI_CONTINUE_COUNT | SPI_CONTINUE_READ_CTRL);
		sfreg_write32(irq_as, value, ALI_SPI_INS);

		if (!irq_as->is_dma_mapped)
			spi_dma_unmap(irq_as, irq_as->rx_dma,
					irq_as->rx_dma_len, SPI_DMA_RX);
		if (!irq_as->dma_align_flag)
			memcpy(irq_as->current_transfer->rx_buf,
					irq_as->buf, irq_as->rx_dma_len);
	}

	/* set pinmux to default reg C8*/
	set_cs_reg(irq_as, CS_DISABLE);

	irq_as->ali_msg->status = 0;
	irq_as->ali_msg->complete(irq_as->ali_msg->context);

	return IRQ_HANDLED;
}

static int ali_spi_probe(struct platform_device *pdev)
{
	int     ret;
	struct spi_master *master;
	struct ali_spi  *as;
	int     irq, i;

	/*add for device tree*/
	struct device   *dev = &pdev->dev;
	struct resource *res;
	void __iomem    *base;

	dev_info(&pdev->dev, "%s: line# %d\n",  __func__, __LINE__);
	/* setup spi core then ali-specific driver state */
	ret = -ENOMEM;
	master = spi_alloc_master(&pdev->dev, sizeof(*as));
	if (!master)
		goto out_free;

	master->bus_num = 1;      /* pdev->id  not set*/
	master->num_chipselect = CHIP_SELECT_MAX_NUM;
	master->setup = ali_spi_setup;
	master->transfer = ali_spi_transfer;
	master->cleanup = ali_spi_cleanup;
	master->mode_bits =SPI_MODE_3;
	platform_set_drvdata(pdev, master);

	as = spi_master_get_devdata(master);

	/* get sflash reg */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
		ret = -ENXIO;
		goto out_free;
	}
	dev_info(&pdev->dev,
		"---[%s]-[%d] mem res[0].start:0x%08x, res[0].end:0x%08x\n"
		, __func__, __LINE__, res->start, res->end);
	as->ali_sflash_base_addr = ioremap(res->start,
						res->end - res->start + 1);
	if (!as->ali_sflash_base_addr) {
		dev_err(&pdev->dev, "could not ioremap I/O port range\n");
		ret = -EFAULT;
		goto out_free;
	}
#if 0
	/* add for device tree */
	base = devm_ioremap_resource(dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);
#endif

	/* get chip select reg */
	for (i = 0; i < CHIP_SELECT_MAX_NUM; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i + 1);
		if (!res) {
			dev_err(&pdev->dev, "failed %s (%d)\n",
			__func__, __LINE__);
			ret = -ENXIO;
			goto out_free;
		}
		dev_info(&pdev->dev,
		"---[%s]-[%d] mem res[0].start:0x%08x, res[0].end:0x%08x\n"
		, __func__, __LINE__, res->start, res->end);
		as->ali_sflash_cs_addr[i] = ioremap(res->start,
						res->end - res->start + 1);
		if (!as->ali_sflash_cs_addr[i]) {
			dev_err(&pdev->dev, "could not ioremap I/O port range\n");
			ret = -EFAULT;
			goto out_free;
		}
	}

	/* get irq */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
			dev_err(&pdev->dev, "failed %s (%d)\n",
			__func__, __LINE__);
			goto out_free;
	}
	dev_info(&pdev->dev, "---[%s]-[%d]---irq:%d\n",
	__func__, __LINE__, irq);
	ret = request_irq(irq, ali_spi_irq, 0, dev_name(&pdev->dev), as);
	if (ret != 0) {
			dev_err(&pdev->dev, "failed %s (%d)\n",
			__func__, __LINE__);
			goto out_free;
	}
#if 0
	/* get SoC, pinmux reg to set CS1 & pinmux, Dram reg ctl */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	if (!res) {
		dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
		ret = -ENXIO;
		goto out_free;
	}
	dev_info(&pdev->dev,
		"---[%s]-[%d] mem res[0].start:0x%08x, res[0].end:0x%08x\n"
		, __func__, __LINE__, res->start, res->end);
	as->cap210_soc_dram_ctl_addr = ioremap(res->start,
						res->end - res->start + 1);
	if (!as->cap210_soc_dram_ctl_addr) {
		dev_err(&pdev->dev, "could not ioremap I/O port range\n");
		ret = -EFAULT;
		goto out_free;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 4);
	if (!res) {
		dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
		ret = -ENXIO;
		goto out_free;
	}
	dev_info(&pdev->dev,
		"---[%s]-[%d] mem res[0].start:0x%08x, res[0].end:0x%08x\n"
		, __func__, __LINE__, res->start, res->end);
	as->cap210_bootrom_space_change_addr = ioremap(res->start,
						res->end - res->start + 1);
	if (!as->cap210_bootrom_space_change_addr) {
		dev_err(&pdev->dev, "could not ioremap I/O port range\n");
		ret = -EFAULT;
		goto out_free;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 5);
	if (!res) {
		dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
		ret = -ENXIO;
		goto out_free;
	}
	dev_info(&pdev->dev,
		"---[%s]-[%d] mem res[0].start:0x%08x, res[0].end:0x%08x\n"
		, __func__, __LINE__, res->start, res->end);
	as->cap210_soc_reg_addr = ioremap(res->start,
						res->end - res->start + 1);
	if (!as->cap210_soc_reg_addr) {
		dev_err(&pdev->dev, "could not ioremap I/O port range\n");
		ret = -EFAULT;
		goto out_free;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 6);
	if (!res) {
		dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
		ret = -ENXIO;
		goto out_free;
	}
	dev_info(&pdev->dev,
		"---[%s]-[%d] mem res[0].start:0x%08x, res[0].end:0x%08x\n"
		, __func__, __LINE__, res->start, res->end);
	as->cap210_pinmux_addr = ioremap(res->start,
						res->end - res->start + 1);
	if (!as->cap210_pinmux_addr) {
		dev_err(&pdev->dev, "could not ioremap I/O port range\n");
		ret = -EFAULT;
		goto out_free;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "ctrl_clk_select",
						&as->flash_ctrl_clk_select);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
		ret = -ENXIO;
		goto out_free;
	}
	dev_info(&pdev->dev, "flash cctrl clock select %d\n",
						as->flash_ctrl_clk_select);

	ret = of_property_read_u32(pdev->dev.of_node, "dma_mode",
							&as->dma_mode);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
		ret = -ENXIO;
		goto out_free;
	}
	dev_info(&pdev->dev, "SPI dma mode %d\n", as->dma_mode);
#endif
	res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
		ret = -ENOMEM;
		goto out_free;
	}
	/*
	* Set default dma-mask to 32 bit. Drivers are expected to setup
	* the correct supported dma_mask.
	*/
	dev->coherent_dma_mask = DMA_BIT_MASK(32);

	/*
	* Set it to coherent_dma_mask by default if the architecture
	* code has not set it.
	*/
	if (!dev->dma_mask)
		dev->dma_mask = &dev->coherent_dma_mask;

	as->buf = dma_alloc_coherent(&pdev->dev, DMA_BUF_SIZE,
					&as->buf_dma, GFP_KERNEL);
	if (!as->buf) {
			dev_err(&pdev->dev, "failed %s (%d)\n",
			__func__, __LINE__);
			ret = -ENOMEM;
			goto out_free;
	}

	spin_lock_init(&as->lock);
	INIT_LIST_HEAD(&as->queue);
	as->pdev = pdev;
	ret = spi_register_master(master);
	if (ret) {
		dev_err(&pdev->dev, "failed %s (%d)\n",
		__func__, __LINE__);
		goto out_free;
	}

	return 0;

out_free:
	spi_master_put(master);
	return ret;
}

static int __exit ali_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master = platform_get_drvdata(pdev);
	struct ali_spi  *as = spi_master_get_devdata(master);
	struct spi_message  *msg;

	/* reset the hardware and block queue progress */
	as->stopping = 1;

	/* Terminate remaining queued transfers */
	list_for_each_entry(msg, &as->queue, queue) {
		/* REVISIT unmapping the dma is a NOP on ARM and AVR32
		 * but we shouldn't depend on that...
		 */
		msg->status = -ESHUTDOWN;
		msg->complete(msg->context);
	}
	spi_unregister_master(master);
	platform_device_unregister(pdev);

	return 0;
}

/* add for device tree */
static const struct of_device_id ali_spi_of_match[] = {
{ .compatible = "alitech,spictrl", },
	 {},
};
MODULE_DEVICE_TABLE(of, ali_spi_of_match)

static struct platform_driver ali_spi_driver = {
	.probe = ali_spi_probe,
	.remove = __exit_p(ali_spi_remove),
	.suspend = NULL,
	.resume = NULL,
	.driver = {
		.name = "ali_spi_bus",
		.owner  = THIS_MODULE,
		.of_match_table = ali_spi_of_match,
	},
};
static int __init ali_spi_init(void)
{
	int ret;
	ret = platform_driver_register(&ali_spi_driver);
	return ret;
}

static void __exit ali_spi_exit(void)
{
	platform_driver_unregister(&ali_spi_driver);
}


module_init(ali_spi_init);
module_exit(ali_spi_exit);

MODULE_DESCRIPTION("Ali SPI Controller driver");
MODULE_AUTHOR("Barry Chang");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ali_spi_bus");

