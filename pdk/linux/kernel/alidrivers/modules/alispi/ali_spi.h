#ifndef _ALI_SPI_
#define _ALI_SPI_

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
#define   SPI_CONTINUE_READ_CTRL    (1<<14)
#define   SPI_CFG_DEFAULT  0xc0
#define   SPI_MODE_DEFAULT  0x00
#define   SPI_MODE_3  0x03
#define   SPI_FMT_DEFAULT  0x0d
#define   SPI_INS_DEFAULT  0x03

#define   SPI_ONE_BYTES_ADDRESS   0x10
#define   SPI_IO_MODE_1 0x1
#define   SPI_IO_MODE_5 0x5
#define   ALI_SPI_RX_DUAL    0x3b
#define   ALI_SPI_RX_QUAD   0x6b
#define   ALI_SPI_NOR_RX_DUAL    0x3c
#define   ALI_SPI_NOR_RX_QUAD   0x6c
#define   ALI_SPI_TX_QUAD   0x32
#define   SPI_DMA_TX_CTRL   0x805E00A0
#define   SPI_DMA_RX_CTRL   0x805E0020
#define   SPI_DMA_INT_CLEAN   0x02000001
#define   DMA_MIN_LEN    0x100
#define   CHIP_SELECT_MAX_NUM 2
#define   DMA_BUF_SIZE 4096

#define  SOC_DRAM_CTL 0x1000000
#define  SOC_DRAM_SET  0x10000000
#define  ALIGN_LEN 0x1f
#define  SPI_CS1_ENABLE (1<<12)
#define  PINMUX20_ENABLE (1<<20)
#define  INVALID_DMA_ADDRESS	 0xffffffff
#define  MEM_CLK 108

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
	void __iomem  *cap210_soc_dram_ctl_addr;
	void __iomem  *cap210_bootrom_space_change_addr;
	void __iomem  *cap210_soc_reg_addr;
	void __iomem  *cap210_pinmux_addr;
	u32 xfer_len;       /* Transfer length.  */
	bool spi_dma_flag;
	bool spi_dma_start;
	bool spi_cmd_flag;
	bool dma_align_flag;
	bool is_dma_mapped;
	dma_addr_t tx_dma;
	dma_addr_t rx_dma;
	dma_addr_t buf_dma;
	u8 *buf;
	int dma_xfer;
	u32 tx_dma_len;
	u32 rx_dma_len;	
	bool spi_rx_dual;   /* DUAL X2 */
	bool spi_rx_quad;  /* QUAL X4 */
	bool spi_tx_quad;  /* QUAL X4 */
	u32 flash_ctrl_clk_select;
	u32 dma_mode;
	struct completion spi_dma_completion;
	struct spi_message *ali_msg;
};

enum spi_dma_transfer {
			 SPI_DMA_TX,
			 SPI_DMA_RX
};

enum spi_cs_mode {
			 CS_ENABLE,
			 CS_DISABLE
};

enum spi_multi_io_mode {
			MULTI_IO_0,
			MULTI_IO_1,
			MULTI_IO_5
};

#endif
