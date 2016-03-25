/*
 * Driver for Atmel AT32 and AT91 SPI Controllers
 *
 * Copyright (C) 2006 Atmel Corporation
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
//#include <linux/mtd/spi-nand.h>
#include <ali_reg.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <mtd/mtd-abi.h>
#include <linux/device.h>
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

//#define SPI_DMA_XFER 


#if defined(CONFIG_ALI_CHIP_M3921)
#define ALI_SOC_REG_BASE_PHYS 0x18000000
static u32 reg_data_store_8c;
static u32 reg_data_store_88;

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
 
static void pinmux_2_spi( )
{
 	ali_spinor_pinmux_set();
}
 static void pinmux_2_nand( )
{
 	ali_spinor_pinmux_release();
}
 #endif
 
static int ali_spi_setup(struct spi_device *spi)
{
	struct ali_spi  *as;
	
	dev_dbg(&spi->dev, "%s.\n", __func__);
	as = spi_master_get_devdata(spi->master);
	sfreg_write8(as, 0xc2, ALI_SPI_CFG);
	sfreg_write8(as, 0x00, ALI_SPI_MODE);
	sfreg_write8(as, 0x0d, ALI_SPI_FMT);
	sfreg_write8(as, 0x03, ALI_SPI_INS);
	return 0;
}

void
spi_dma_transfer(const void *buf, u32 tmp_len, struct ali_spi *as , int xfter)
{
	u32 value;
	int ret;

	init_completion(&as->spi_dma_completion);
	/* set pinmux 1030 28bit 1 */
	value = ali_spi_soc_read(ALI_SOC_REG);
	value |= (ALI_SOC_28);
	ali_spi_soc_write(value, ALI_SOC_REG);

	sfreg_write32(as, tmp_len, ALI_SPI_FLASH_COUNTER);  /*reg BC*/
	sfreg_write8(as, 0x0, ALI_SPI_CFG);  /* set clk to 54M */
	sfreg_write32(as, ((as->buf_dma) & 0x3fffffff),
					ALI_SPI_DMA_MEM_ADDR);   /*reg 58*/
	sfreg_write32(as, 0, ALI_SPI_DMA_FLASH_ADDR);/*reg 5C*/
	sfreg_write32(as, tmp_len, ALI_SPI_DMA_LEN); /*reg 60*/

	if (xfter == SPI_DMA_TX) {
		memcpy(as->buf, buf, tmp_len);
		value  = sfreg_read32(as, ALI_SPI_INS);
		value |= (SPI_CONTINUE_COUNT | SPI_CONTINUE_WRITE_CTRL);
		value |=  SPI_SFLASH_CTRL_CLK_SEL;
		sfreg_write32(as, value, ALI_SPI_INS);  /*reg 98*/
		sfreg_write32(as, SPI_DMA_TX_CTRL,
					ALI_SPI_DMA_CTRL); /*reg 64*/
		ret = wait_for_completion_timeout(&as->spi_dma_completion,
						msecs_to_jiffies(1000));

		if (!ret) {
			dev_err(&as->pdev->dev, "DMA Tx time out\n");
			return;
		}
		value = sfreg_read32(as, ALI_SPI_INS);
		value &= ~(SPI_CONTINUE_COUNT | SPI_CONTINUE_WRITE_CTRL);
		sfreg_write32(as, value, ALI_SPI_INS);

	} else {
		value = sfreg_read32(as, ALI_SPI_INS);
		value |= (SPI_CONTINUE_COUNT | SPI_CONTINUE_READ_CTRL);
		value |=  SPI_SFLASH_CTRL_CLK_SEL;
		sfreg_write32(as, value, ALI_SPI_INS); /*reg 98*/
		sfreg_write32(as, SPI_DMA_RX_CTRL,
						ALI_SPI_DMA_CTRL);  /*reg 64*/
		ret = wait_for_completion_timeout(&as->spi_dma_completion,
						msecs_to_jiffies(1000));

		if (!ret)
		{
			dev_err(&as->pdev->dev, "DMA Rx time out\n");
			return;
		}
		memcpy(buf, as->buf, tmp_len);
		value = sfreg_read32(as, ALI_SPI_INS);
		value &= ~(SPI_CONTINUE_COUNT | SPI_CONTINUE_READ_CTRL);
		sfreg_write32(as, value, ALI_SPI_INS);
	}

	/* set pinmux 1030 28bit 0 */
	value = ali_spi_soc_read(ALI_SOC_REG);
	value &= ~(ALI_SOC_28);
	ali_spi_soc_write(value, ALI_SOC_REG);
}

void set_io_mode(u8 cmd, struct ali_spi *as)
{
	switch(cmd) {
		case ALI_SPI_RX_DUAL:
			as->spi_rx_dual = 1;
			break;
		case ALI_SPI_RX_QUAD:
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

void transfer_data_tx(const void *tx_buf , u32 len, struct ali_spi *as)
{
	u32 tmp_len;
	u32 ali_flash_base_addr;
	u32 addr, i, value;
	int ret;

	ali_flash_base_addr =
			(u32)as->ali_sflash_cs_addr[as->stay->chip_select];
	tmp_len = len;
	if(*(u8 *)tx_buf == ALI_SPI_RX_DUAL 
		|| *(u8 *)tx_buf == ALI_SPI_RX_QUAD 
		|| *(u8 *)tx_buf == ALI_SPI_TX_QUAD)
		set_io_mode( *(u8 *)tx_buf, as);
	sfreg_write8(as, SPI_HIT_DATA, ALI_SPI_FMT);
	value = sfreg_read8(as, ALI_SPI_MODE);
	value &= ~(0x7);
	sfreg_write8(as, value, ALI_SPI_MODE);

	if (as->spi_tx_quad) {
		value = sfreg_read8(as, ALI_SPI_MODE);
		value |= SPI_IO_MODE_5;
		sfreg_write8(as, value, ALI_SPI_MODE);
		as->spi_tx_quad = 0;
	}

#ifdef SPI_DMA_XFER

	if ((tmp_len >= DMA_MIN_LEN) && ( !(tmp_len & 0x0f))) {
		spi_dma_transfer(tx_buf, tmp_len, as, SPI_DMA_TX);
	} else {
		i = 0;
		while (tmp_len) {
			addr = *(u8 *)((u32) tx_buf + i);
			*(u8 *) (ali_flash_base_addr + addr) = addr;
			tmp_len--;
			i++;
		}
	}
#else
	i = 0;
	while (tmp_len) {
		addr = *(u8 *)((u32) tx_buf + i);
		*(u8 *) (ali_flash_base_addr + addr) = (u8) addr;
		tmp_len--;
		i++;
	}
#endif
}

void transfer_data_rx(void *rx_buf, u32 len, struct ali_spi *as)
{
	u32 tmp_len;
	u32 ali_flash_base_addr;
	int i;
	u32 value;
	int ret;

	sfreg_write8(as, SPI_HIT_DATA, ALI_SPI_FMT);
	value = sfreg_read8(as, ALI_SPI_MODE);
	value &= ~(0x7);
	sfreg_write8(as, value, ALI_SPI_MODE);

	if (as->spi_rx_dual) {
		value = sfreg_read8(as, ALI_SPI_MODE);
		value |= SPI_IO_MODE_1;
		sfreg_write8(as, value, ALI_SPI_MODE);
		as->spi_rx_dual = 0;
	}
	if (as->spi_rx_quad) {
		value = sfreg_read8(as, ALI_SPI_MODE);
		value |= SPI_IO_MODE_5;
		sfreg_write8(as, value, ALI_SPI_MODE);
		as->spi_rx_quad = 0;
	}

	ali_flash_base_addr =
			(u32)as->ali_sflash_cs_addr[as->stay->chip_select];
	tmp_len = len;

#ifdef SPI_DMA_XFER

	if ((tmp_len >= DMA_MIN_LEN) && ( !(tmp_len & 0x0f))){
		spi_dma_transfer(rx_buf, tmp_len, as, SPI_DMA_RX);
	} else {
		i = 0;

		while (tmp_len) {
			/* align */
			if (((((u32) rx_buf + i) & 3) == 0) && (tmp_len >= 4)) {
				*(u32 *)((u32) rx_buf + i) =
					*(u32 *)(ali_flash_base_addr);
				tmp_len -= 4;
				i += 4;
			} else {
				while ((((((u32) rx_buf + i) & 3) != 0) ||
						(tmp_len < 4)) && tmp_len) {
					*(u8 *)((u32) rx_buf + i) =
						*(u8 *)(ali_flash_base_addr);
					i++;
					tmp_len--;
				}
			}
		}
	}
#else
	i = 0;

	while (tmp_len) {
		/* align  */
		if (((((u32) rx_buf + i) & 3) == 0) && (tmp_len >= 4)) {
			*(u32 *)((u32) rx_buf + i) =
						*(u32 *)(ali_flash_base_addr);
			tmp_len -= 4;
			i += 4;
		} else {
			while ((((((u32) rx_buf + i) & 3) != 0) ||
						(tmp_len < 4)) && tmp_len) {
				*(u8 *)((u32) rx_buf + i) =
						*(u8 *)(ali_flash_base_addr);
				i++;
				tmp_len--;
			}
		}
	}

#endif
}


static int ali_spi_transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct ali_spi  *as;
	struct spi_transfer *parse_xfer;
	struct spi_transfer *xfer;
	int  status = 0;
	u8 cmd;
	u32 addr = 0;
	u32 value;
	bool cs_flag = 0;

	as = spi_master_get_devdata(spi->master);
	as->stay = spi;
	msg->actual_length = 0;

#if defined(CONFIG_ALI_CHIP_M3921)
	pinmux_2_spi();
#endif
	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		/* set pinmux to spi reg C8*/
		value = sfreg_read32(as, ALI_SPI_CS);
		value |= (SPI_CS_CTRL_ENABLE | SPI_CS_ENABLE);
		sfreg_write32(as, value,  ALI_SPI_CS);
		parse_xfer = xfer;
		
		if (parse_xfer->rx_buf != NULL) {
				msg->actual_length += parse_xfer->len;
				transfer_data_rx(parse_xfer->rx_buf,
						(u32) parse_xfer->len, as);
				cs_flag = 0;
		}
		else  if (parse_xfer->tx_buf != NULL) {
				cs_flag = 1;
				msg->actual_length += parse_xfer->len;
				transfer_data_tx(parse_xfer->tx_buf,
						(u32) parse_xfer->len, as);
		}
		if(!cs_flag) {
		/* set pinmux to default reg C8*/
			value = sfreg_read32(as, ALI_SPI_CS);
			value &= ~(SPI_CS_CTRL_ENABLE);
			sfreg_write32(as, value,  ALI_SPI_CS);
		}
	}
	value = sfreg_read32(as, ALI_SPI_CS);
	value &= ~(SPI_CS_CTRL_ENABLE);
	sfreg_write32(as, value,  ALI_SPI_CS);
	msg->status = status;
	msg->complete(msg->context);
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
	sfreg_write32(as, SPI_DMA_INT_CLEAN, ALI_SPI_DMA_INT_STATUS);
	sfreg_write32(as, 0x0, ALI_SPI_DMA_CTRL);
	complete(&(((struct ali_spi *)as)->spi_dma_completion));
	return IRQ_HANDLED;
}

static int __init ali_spi_probe(struct platform_device *pdev)
{
	int     ret;
	struct spi_master *master;
	struct ali_spi  *as;
	int     irq, i;

	/*add for device tree*/
	struct device   *dev = &pdev->dev;
	struct resource *res;
	void __iomem    *base;
//	printk("ali_spi_probe ### 1\n");
	dev_info(&pdev->dev, "%s: line# %d\n",  __func__, __LINE__);
	/* setup spi core then atmel-specific driver state */
	ret = -ENOMEM;
	master = spi_alloc_master(&pdev->dev, sizeof(*as));
	if (!master)
		goto out_free;

	master->bus_num = pdev->id;
	master->num_chipselect = CHIP_SELECT_MAX_NUM;
	master->setup = ali_spi_setup;
	master->transfer = ali_spi_transfer;
	master->cleanup = ali_spi_cleanup;
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

	/* add for device tree */
	/*base = devm_ioremap_resource(dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);*/

	/* get chip select reg */
	for (i = 0; i < CHIP_SELECT_MAX_NUM; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i + 1);
		if (!res) {
			dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
			ret = -ENXIO;
			goto out_free;
		}
		dev_info(&pdev->dev,
		"---[%s]-[%d] mem res[0].start:0x%08x, res[0].end:0x%08x\n"
		, __func__, __LINE__, res->start, res->end);
		as->ali_sflash_cs_addr[i] = ioremap(res->start,
						res->end - res->start + 1);
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
			dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
			goto out_free_buffer;
	}
	dev_info(&pdev->dev, "---[%s]-[%d]---irq:%d\n", __func__, __LINE__, irq);
	ret = request_irq(irq, ali_spi_irq, 0, dev_name(&pdev->dev), as);
	if (ret != 0) {
			dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
			goto out_free_buffer;
	}
	//printk("ali_spi_probe ### 2\n");
	if (dev->of_node) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
		if (res != 0) {
			ret = -ENODEV;
			goto out_free;
		}

	} else {
		/*
		* buffer for DMA access
		*/
		res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
		if (!res) {
			dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
			ret = -ENOMEM;
			goto out_free;
		}
	}

	as->buf = dma_alloc_coherent(&pdev->dev, DMA_BUF_SIZE,
					&as->buf_dma, GFP_KERNEL);
	if (!as->buf) {
			dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
			ret = -ENOMEM;
			goto out_free;
	}
	//printk("ali_spi_probe ### 3\n");
	spin_lock_init(&as->lock);
	INIT_LIST_HEAD(&as->queue);
	as->pdev = pdev;
	init_completion(&as->spi_dma_completion);
	ret = spi_register_master(master);
	if (ret) {
		dev_err(&pdev->dev, "failed %s (%d)\n", __func__, __LINE__);
		goto out_free_buffer;
	}
	//printk("ali_spi_probe ### ok\n");
	return 0;

out_free_buffer:
	dma_free_coherent(&pdev->dev, DMA_BUF_SIZE, as->buf, as->buf_dma);
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
{ .compatible = "ali_tech,spictrl", },
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
MODULE_AUTHOR("Eric Li");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ali_spi_bus");

