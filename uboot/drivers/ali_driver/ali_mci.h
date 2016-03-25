/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2010 - 2013 Copyright (C)
 *
 *  File: alimci.h
 *
 *  Description: SD/MMC Card Host driver for ALI 36XX/39XX platform
 *
 *  History:
 *          Date                    Author          Comment
 *          ====                    ======          =======
 * 0.       2010.6.28               Alex Xia        Create
 * 1.       2013.8.14               David Chen   Re-architecture for S3921
 ****************************************************************************/
#ifndef _ALI_MCI_H
#define _ALI_MCI_H

#include "ali_mci_bsc.h"

#if defined(CONFIG_CMD_MMC) //if used in u-boot
#define FALSE 0
#define TRUE 1
    #define mutex_lock(...)			do { } while (0)
    #define mutex_unlock(...)			do { } while (0)
#define __CTDADDRALI(x)		(x & 0x7fffffff)
//    #define cmd->opcode             cmd->cmdidx
    #define printk				printf
typedef unsigned int bool;

#define MMC_BUS_WIDTH_1		0
#define MMC_BUS_WIDTH_4		2
#define MMC_BUS_WIDTH_8		3

typedef enum
{
    ALI_SDIO_MT_UNKNOWN,     //not detected yet.
    ALI_SDIO_MT_SD20PLUS,     //SD2.0 and plus card
    ALI_SDIO_MT_SD110MINUS,   //SD1.10 and minus card
    ALI_SDIO_MT_MMC,              //MMC Card
    ALI_SDIO_MT_eMMC              //eMMC chip
}ALI_SDIO_MEDIATYPE;

#define RESP_4_TIMEOUT 0x12345678

#define DRV_4_eMMC
#endif

#define ALIMMC_MIN_SPEED	(24000000 / 256)
#define ALIMMC_MAX_SPEED	10000000 //26000000
#define ALIMMC_HOST_CAPS	MMC_MODE_4BIT   //(MMC_MODE_4BIT | MMC_MODE_HS)


#define DETECT_DELAY        500 //ms

#define GPIO_HIGH   1
#define GPIO_LOW    0

#define ALIMCI_FAIL		1
#define ALIMCI_SUCESS	0

#define CLK_GATE_ON     0
#define CLK_GATE_OFF   1

#define ALIMCI_ON     1
#define ALIMCI_OFF   0

#define SD_CTRL_REG             (0x00)      //8 bit
#define     ALI_XFER_MODE           (1 << 7)                /* Response Type */
#define         SD_4BIT                    (1 << 7)
#define         SD_MMC_1BIT           (0 << 7)
#define     ALI_MCI_RSPTYP          (7 << 4)                /* Response Type */
#define         ALI_MCI_RSPTYPE_NONE          (0 << 4)        /* No response for cmd 0,4,15 */
#define         ALI_MCI_RSPTYPE_R1               (1 << 4)        /* R1b for cmd 5,6,12,28,29,38,42 */
#define         ALI_MCI_RSPTYPE_R2               (2 << 4)        /* R2 for cmd 2,9,10 */
#define         ALI_MCI_RSPTYPE_R3               (3 << 4)        /* R3 for cmd 1,58,ACMD41 */
#define         ALI_MCI_RSPTYPE_R4_7           (4 << 4)        /* R4 for cmd 39,      R5 for cmd 40,      R6 for cmd sd_3,       R7 for cmd sd_8  */
#define     ALI_MCI_CMDTYPE       (5 << 1)                /* Command Type */
#define         ALI_MCI_CMDTYPE_BC              (0 << 1)         /* BC (broadcase command) */
#define         ALI_MCI_CMDTYPE_BCR            (1 << 1)              /* BCR (broadcase command with response)  for cmd 1,2,40, sd_3,sd_8,ACMD41 */
#define         ALI_MCI_CMDTYPE_AC              (2 << 1)               /* AC (addressed command) */
#define         ALI_MCI_CMDTYPE_ADTC_RD    (4 << 1)         /* ADTC (addressed data transfer command) */
#define         ALI_MCI_CMDTYPE_ADTC_WR   (5 << 1)
#define     ALI_SEND_CMD                (1 << 0)

#define SD_STATUS_REG           (0x01)      //8 bit
#define     SD_STATUS_OK_MASK    0xe4
#define         DATA_BUSY               (1 << 4)
#define         CMD_BUSY                (1 << 3)
#define         CRC16_ERR               (1 << 1)
#define         CRC7_ERR                (1 << 0)

#define SD_CMD_REG              (0x02)      //8 bit
#define     ALI_XFER_DIR            (1 << 7)                /* Transfer Direction */
#define             RD_FROM_CARD                (0 << 7)
#define             WR_TO_CARD              (1 << 7)
#define         SD_INT_ENABLE                   (1 << 6)        //Enable intterrupt
#define         SD_INT_DISABLE                 (0 << 6)        //Disable intterrupt
#define     SD_CMD_IDX_MASK    (0x3f << 0) /* 6 bits, Command Index */

#define SD_CLKDIV_L_REG         (0x03)
#define SD_ARG_REG              (0x04)
#define SD_BLKLEN_REG           (0x08)
#define SD_BLKNUM_L_REG     (0x0a)
#define SD_CLK_FORCE_EN          (0x0b)     //bit 0  enable CLK output to Slave(Eg SD card)
#define     CLK_ALWAYS_EN                (1 << 0)
#define SD_BITCTRL_REG          (0x0b)      //bit 1,2,3
#define SD_PIO_DATA             (0x0c)
#define SD_PIO_CTRL             (0x0e)
#define     PIO_CLEAN_DATA  (1<<0)
#define     PIO_NO_DATA     (1<<1)
#define     PIO_EN          (1<<2)
#define SD_RESP_REG             (0x10)
#define SD_DMA_R_ADDR           (0x20)
#define SD_DMA_W_ADDR           (0x24)
#define SD_DMA_R_SIZE           (0x28)
#define SD_DMA_W_SIZE           (0x2C)

#define SD_DMA_CTRL         (0x30)
#define     TX_BUS_IDLE_INT_STATUS   (1 << 16)
#define     TX_BUS_IDLE_INT_ENABLE   (1 << 12)
#define     SYS_INT_STATUS           (1 << 6)
#define     DMA_RESET                (1 << 5)
#define     DMA_BUSY        (1<<4)
#define     DMA_WDOING      (1<<3)
#define     DMA_RDOING      (1<<2)
#define     DMA_XFER_DIR            (1 << 1)                /* DMA Transfer Direction */
#define             DMA_RD_FROM_CARD                (0 << 1)
#define             DMA_WR_TO_CARD              (1 << 1)
#define     DMA_START       (1<<0)

#define SD_CLKDIV_H_REG           (0x34)
#define SD_BLKNUM_H_REG     (0x36)
#define SD_CLK_DLY_REG          (0x38)
#define SD_CUR_ADDR         (0x40)
#define SD_CUR_BLK_IFO          (0x44)
#define SD_IP_VER               (0x48)







#define SOC_CHIP_VER            (0x00)


#define BOTH_DIR (MMC_DATA_WRITE | MMC_DATA_READ)


enum alimci_optype
{
    NO_DATA_OP,
    DATA_OP_READ,
    DATA_OP_WRITE_1,
    DATA_OP_WRITE_2,
};

/*
enum alimci_waitfor
{
    COMPLETION_NONE,
    COMPLETION_FINALIZE,
    COMPLETION_CMDSENT,
    COMPLETION_RSPFIN,
    //  COMPLETION_XFERFINISH,
    COMPLETION_XFERFINISH_RSPFIN,
};
*/

struct alimci_host
{
    //u32 id;
//    struct platform_device  *pdev;
    struct mmc     *base_mmc;
	struct mmc_cmd *cmd;
	struct mmc_data *data;
//    struct mmc_request  *mrq;

//    spinlock_t      complete_lock;
    //enum alimci_waitfor    complete_what;

    //void __iomem *baseaddr;
    u32 soc_base;
    u32 sdio_base;
    u16 ChipId; //Eg. 0x3921
    u8   ChipPkg; //Package, Eg. QFP or BGA,...
    u8   ChipVer; //reversion

    u32 pinmux1;
    u32 pinmux2;
    u32 pinmux3;
    u32 pinmux4;

    int         irq;
    int         irq_cd;
    bool       usedma;
    u16         InitClkDiv;
    u16         TranClkDiv;
    unsigned char   bus_width;      /* data bus width */
    int present;
    volatile enum alimci_optype data_op_type;

    /* delay in mS before detecting cards after interrupt */
    u32 detect_delay_ms;
    int gpio_cd;           /* gpio detecting card insertion */
    bool gpio_cd_invert;       /* gpio card detect is inverted, TRUE when gpio_card_detect ==0 means CARD exist  */
    int gpio_card_ro;           /* gpio detecting read only toggle */
    bool gpio_card_ro_invert;       /* gpio ro is inverted */
    int gpio_power;             /* gpio powering up MMC bus */
    bool gpio_power_invert;         /* gpio power is inverted */
    //bool gpio_power_orig;         /* orignal GPIO value before SD driver start */

//    bool       DbgOut;
    bool LstCmd55;         /* last opcode  is MMC_APP_CMD? */
    bool IntMode;
};

static struct alimci_host *p_host = NULL;
extern struct mutex ali_sto_mutex;

static inline u32 ALI_SOC_RD32(const u32 reg)
{
    if (!p_host)
    {
        printk("----p_host == NULL, please check!------\n");
    }

    if (0 == p_host->soc_base)
    {
        printk("----'sdio_base' is not assigned a value yet.------\n");
    }
    return __REG32ALI(p_host->soc_base + reg);
}

static inline void ALI_SOC_WR32(const u32 reg, const u32 data)
{
    if (!p_host)
    {
        printk("----p_host == NULL, please check!------\n");
    }

    if (0 == p_host->soc_base)
    {
        printk("----'sdio_base' is not assigned a value yet.------\n");
    }
    __REG32ALI(p_host->soc_base + reg) = data;
}

static inline u32 ALI_MCI_RD8(const u32 reg)
{
    if (!p_host)
    {
        printk("----p_host == NULL, please check!------\n");
    }

    if (0 == p_host->sdio_base)
    {
        printk("----'sdio_base' is not assigned a value yet.------\n");
    }
    return __REG8ALI(p_host->sdio_base + reg);
}

static inline void ALI_MCI_WR8(const u32 reg, const u32 data)
{
    if (!p_host)
    {
        printk("----p_host == NULL, please check!------\n");
    }

    if (0 == p_host->sdio_base)
    {
        printk("----'sdio_base' is not assigned a value yet.------\n");
    }
    __REG8ALI(p_host->sdio_base + reg) = data;
}
static inline u32 ALI_MCI_RD32(const u32 reg)
{
    if (!p_host)
    {
        printk("----p_host == NULL, please check!------\n");
    }

    if (0 == p_host->sdio_base)
    {
        printk("----'sdio_base' is not assigned a value yet.------\n");
    }
    return __REG32ALI(p_host->sdio_base + reg);
}

static inline void ALI_MCI_WR32(const u32 reg, const u32 data)
{
    if (!p_host)
    {
        printk("----p_host == NULL, please check!------\n");
    }

    if (0 == p_host->sdio_base)
    {
        printk("----'sdio_base' is not assigned a value yet.------\n");
    }
    __REG32ALI(p_host->sdio_base + reg) = data;
}

#define GET_UINT32_UNALIGNED(i)      (((volatile unsigned char *)(i))[0]) \
    |(((volatile unsigned char *)(i))[1]<<8) \
    |(((volatile unsigned char *)(i))[2]<<16) \
    |(((volatile unsigned char *)(i))[3]<<24)





#endif
