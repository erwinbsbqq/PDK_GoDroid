#ifndef _ALI_SPI_

#define _ALI_SPI_

#define ALI_SPI_SOC_BASE         0x18000000
#define ali_spi_soc_read(reg)        __REG32ALI(reg + ALI_SPI_SOC_BASE)
#define ali_spi_soc_write(val, reg)  (__REG32ALI(reg + ALI_SPI_SOC_BASE) = val)

#define ALI_SPI_INS   0x98
#define ALI_SPI_FMT   0x99
#define ALI_SPI_MODE  0x9A
#define ALI_SPI_CFG   0x9B
#define SPI_HIT_DATA  0x01
#define SPI_HIT_DUMM     0x02
#define SPI_HIT_ADDR      0x04
#define SPI_HIT_CODE      0x08
#define SPI_CONT_RD     0x40
#define SPI_CONT_WR 0x80

#define ALI_SPI_CS    0xC8
#define SPI_CS_CTRL_ENABLE   (0x3 << 25)
#define SPI_CS_ENABLE   (0x1 << 24)
#define ALI_SOC_28        (1 << 28)

#define ALI_SOC_REG   0x1030
#define ALI_SPI_DMA_MEM_ADDR    0x58
#define ALI_SPI_DMA_FLASH_ADDR  0x5C
#define ALI_SPI_DMA_LEN     0x60
#define ALI_SPI_DMA_CTRL      0x64
#define ALI_SPI_FLASH_COUNTER   0xBC
#define ALI_SPI_DMA_INT_STATUS    0xA0
#define   SPI_DMA_INT_EN    (1<<20)
#define   SPI_DMA_ACCESS    (1<<19)
#define   SPI_DMA_DIR_TX    (1<<7) /* 1: dram to flash, 0: flash to dram(rx) */
#define   SPI_DMA_DIR_RX  (0)     /*1 : dram to flash, 0 : flash to dram(rx) */
#define   SPI_DMA_START   (1<<5)
#define    SPI_DMA_CS_0   0
#define    SPI_DMA_CS_1   1
#define    SPI_DMA_CS_2   2
#define    SPI_DMA_CS_3   3
#define   SPI_DMA_LEN_ERR (1<<25)
#define   SPI_DMA_STOP    (1<<18)
#define   SPI_DMA_INT   (1<<0)
#define   SPI_CONTINUE_COUNT      (1<<21)
#define   SPI_CONTINUE_WRITE_CTRL   (1<<15)
#define   SPI_SFLASH_CTRL_CLK_SEL   (0<<24)
#define   SPI_CONTINUE_READ_CTRL    (1<<14)

#define   SPI_ONE_BYTES_ADDRESS   0x10
#define   SPI_IO_MODE_1 0x1
#define   SPI_IO_MODE_5 0x5
#define   ALI_SPI_RX_DUAL    0x3b
#define   ALI_SPI_RX_QUAD   0x6b
#define   ALI_SPI_TX_QUAD   0x32
#define   SPI_DMA_TX_CTRL   0x005E00A0
#define   SPI_DMA_RX_CTRL   0x005E0020
#define   SPI_DMA_INT_CLEAN   0x02000001
#define   SPI_2K_DATA_LEN   0x840
#define   DMA_MIN_LEN    0x100
#define   CHIP_SELECT_MAX_NUM 2
#define   DMA_BUF_SIZE 4096

struct ali_spi {
	spinlock_t           lock;
	unsigned long           flags;
	struct platform_device *pdev;
	struct spi_device    *stay;
	u8                 stopping;
	struct list_head     queue;
	struct spi_transfer    *current_transfer;
	unsigned long      current_remaining_bytes;
	struct spi_transfer    *next_transfer;
	unsigned long      next_remaining_bytes;
	void __iomem  *ali_sflash_base_addr;
	void __iomem  *ali_sflash_cs_addr[CHIP_SELECT_MAX_NUM];
	u8 spi_ins;         /* SPI instruction. */
	u8 ins_fmt;         /* Ali SPI instruction format. */
	u8 r_or_w;          /* 0: Read; 1: Write */
	u8 spi_active;      /* 0: idle; 1: SPI instruction ready.  */
	u32 offset;         /* SPI access offset.  */
	u32 xfer_len;       /* Transfer length.  */
	u8 *buf;
	dma_addr_t buf_dma;
	bool spi_rx_dual;   /* DUAL X2 */
	bool spi_rx_quad;  /* QUAL X4 */
	bool spi_tx_quad;  /* QUAL X4 */
	struct completion spi_dma_completion;
};

enum spi_dma_transfer {
			 SPI_DMA_TX,
			 SPI_DMA_RX
};

static void ali_spi_lock(struct ali_spi *as) __acquires(&as->lock)
{
				 spin_lock_irqsave(&as->lock, as->flags);
}

static void ali_spi_unlock(struct ali_spi *as) __releases(&as->lock)
{
				 spin_unlock_irqrestore(&as->lock, as->flags);
}

static u8 sfreg_read8(struct ali_spi *as, u32 reg)
{
	return	readb(as->ali_sflash_base_addr  + reg);
}

static void sfreg_write8(struct ali_spi *as, u8 val, u32 reg)
{
	writeb(val, as->ali_sflash_base_addr + reg);
}

static u32 sfreg_read32(struct ali_spi *as, u32 reg)
{
	return readl(as->ali_sflash_base_addr + reg);
}

static void sfreg_write32(struct ali_spi *as, u32 val, u32 reg)
{
	writel(val, as->ali_sflash_base_addr + reg);
}

#endif
