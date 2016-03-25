#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <boot_common.h>
#ifdef ALI_ARM_STB
#include <asm/dma-mapping.h>
#endif
#include <linux/err.h>
#include <nand.h>
#include <ali_nand.h>
#include "ali_nand_priv.h"

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t

#define ALI_NAND_DRIVER_VERSION "ali_nand_ver_2014_0902_u1"
#if defined(DEBUG_NAND_DRV)
#define probe_info printf
#else
#define probe_info(args...) do{}while(0)
#endif

static void __iomem *ali_soc_reg = (void __iomem *)ALI_SOC_BASE;
static void __iomem *ali_nand_reg = (void __iomem *)ALI_NANDREG_BASE;

#define ali_soc_read(reg)		readl(reg)
#define ali_soc_write(val, reg) writel(val, reg)
#define nfreg_read(reg)			readl(ali_nand_reg + reg)
#define nfreg_read8(reg)		readb(ali_nand_reg + reg)
#define nfreg_write(val, reg)	writel(val, (ali_nand_reg + reg))
#define nfreg_write8(val, reg)	writeb(val, (ali_nand_reg + reg))
#define nfreg_read32(reg)		nfreg_read(reg)
#define nfreg_write32(val, reg)	nfreg_write(val,reg)

static u16 chip_id;
static u8 chip_package, chip_ver;	
static u8 bch_mode = 0;
static u8 data_all_ff;
//static dma_addr_t chip_hw_dma_address;
static unsigned long chip_hw_dma_address;
static u8 *chip_hw_dma_buf;
static const char nandname[] = "alidev_nand_reg";

static u8 bbt_pattern[] = {'B', 'b', 't', '0' };
static u8 mirror_pattern[] = {'1', 't', 'b', 'B' };
static u8 scan_ff_pattern[] = { 0xff, 0xff };
static u8 pattern_35[4] = {0x35, 0x03, 0x55, 0xAA,};
static u8 pattern_36[4] = {0x36, 0x03, 0x55, 0xAA,};
static u8 pattern_39[4] = {0x39, 0x01, 0x55, 0xAA,};
static u8 pattern_37[4] = {0x37, 0x01, 0x55, 0xAA,};
static u8 pattern_38[4] = {0x38, 0x11, 0x55, 0xAA,};
static u8 pattern_3821[4] = {0x38, 0x21, 0x55, 0xAA,};
static u8 pattern_3921[4] = {0x39, 0x21, 0x55, 0xAA,};

// PMI ecc pattern base on MPTool
//static u8 ECC_pattern_1b[12]={0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
static u8 ECC_pattern_16b[12]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static u8 ECC_pattern_24b[12]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static u8 ECC_pattern_40b[12]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static u8 ECC_pattern_48b[12]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static u8 ECC_pattern_60b[12]={0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
#define PMI_PATTERN_POS 	12
#define PMI_PATTERN_LEN		4

#define BLANK_DETECT_60ECC (0x37 << 16)
#define BLANK_DETECT_48ECC (0x2F << 16)
#define BLANK_DETECT_40ECC (0x27 << 16)
#define BLANK_DETECT_24ECC (0x17 << 16)
#define BLANK_DETECT_16ECC (0x0F << 16)

static struct nand_ecclayout ali_nand_oob_32 = {
	.eccbytes = 28,
	.eccpos = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
		      19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31},
	.oobfree = {
		{.offset = 0,
		 .length = 8}}
};

static struct nand_bbt_descr largepage_flashbased = {
	.options = NAND_BBT_SCAN2NDPAGE,
	.offs = 0,
	.len = 2,
	.pattern = scan_ff_pattern
};

static struct nand_bbt_descr ali_bbt_main_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	0,
	.len = 4,
	.veroffs = 4,
	.maxblocks = 16,
	.pattern = bbt_pattern
};

static struct nand_bbt_descr ali_bbt_mirror_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	0,
	.len = 4,
	.veroffs = 4,
	.maxblocks = 16,
	.pattern = mirror_pattern
};

struct PMI_descr {
	u8 maxblocks;
	u32 pos[4];
	u8 *pattern;
};

static struct PMI_descr ali_PMI_descr = {
	.maxblocks = 4,
	.pos =  {0x00, 0x100, 0x200, 0x300,},
	.pattern = &pattern_3921[0],
};	

#if 0
struct mtd_partition ali_nand_partitions[] = {
    { .name = "ALI-Private",	.offset = 0,     .size = 0,     .mask_flags = MTD_WRITEABLE,},/* force read-only */     
    { .name = "Partition1",     .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition2",     .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition3",     .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition4",     .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition5",     .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition6",     .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition7",     .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition8",     .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition9",     .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition10",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition11",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition12",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition13",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition14",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition15",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition16",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition17",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition18",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition19",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition20",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition21",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition22",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition23",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition24",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition25",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition26",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition27",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition28",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition29",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
    { .name = "Partition30",    .offset = 0,     .size = 0,     .mask_flags = 0,    },
};
#endif

//static struct nand_chip ali_nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];
#define BLOCK_REWRITE_TBL_SIZE 4096
u8 block_rewrite_tbl[BLOCK_REWRITE_TBL_SIZE];

//------------------------------------------------add for nand flash partion
#define PartCntOff       (0x100)
#define PartOff          (0x104)
#define PartNameOff      (0x200)

#define MAX_PMI_SIZE      2048   //pageSize=2048,  1024
#define NAND_BUFFER_ADD_SIZE           8


//----------------------------------------------------------------------


#define NF_BUFFER_ADDR       (NAND_BASE_ADDR + 0x1000)

/*
 * Constants for hardware specific CLE/ALE/NCE function
 *
 * These are bits which can be or'ed to set/clear multiple
 * bits in one go.
 */
/* Select the chip by setting nCE to low */
#define NAND_NCE		0x01
/* Select the command latch by setting CLE to high */
#define NAND_CLE		0x02
/* Select the address latch by setting ALE to high */
#define NAND_ALE		0x04

#define NAND_CTRL_CLE		(NAND_NCE | NAND_CLE)
#define NAND_CTRL_ALE		(NAND_NCE | NAND_ALE)
#define NAND_CTRL_CHANGE	0x80

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_RNDOUT		5
#define NAND_CMD_PAGEPROG	0x10
#define NAND_CMD_READOOB	0x50
#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_STATUS_MULTI	0x71
#define NAND_CMD_SEQIN		0x80
#define NAND_CMD_RNDIN		0x85
#define NAND_CMD_READID		0x90
#define NAND_CMD_ERASE2		0xd0
#define NAND_CMD_RESET		0xff

/* Extended commands for large page devices */
#define NAND_CMD_READSTART	    0x30
#define NAND_CMD_RNDOUTSTART    0xE0
#define NAND_CMD_CACHEDPROG	    0x15

/* Extended commands for AG-AND device */
/*
 * Note: the command for NAND_CMD_DEPLETE1 is really 0x00 but
 *       there is no way to distinguish that from NAND_CMD_READ0
 *       until the remaining sequence of commands has been completed
 *       so add a high order bit and mask it off in the command.
 */
#define NAND_CMD_DEPLETE1		0x100
#define NAND_CMD_DEPLETE2		0x38
#define NAND_CMD_STATUS_MULTI	0x71
#define NAND_CMD_STATUS_ERROR	0x72
/* multi-bank error status (banks 0-3) */
#define NAND_CMD_STATUS_ERROR0	0x73
#define NAND_CMD_STATUS_ERROR1	0x74
#define NAND_CMD_STATUS_ERROR2	0x75
#define NAND_CMD_STATUS_ERROR3	0x76
#define NAND_CMD_STATUS_RESET	0x7f
#define NAND_CMD_STATUS_CLEAR	0xff

#define NAND_CMD_NONE			-1

/* Status bits */
#define NAND_STATUS_FAIL	    0x01
#define NAND_STATUS_FAIL_N1	    0x02
#define NAND_STATUS_TRUE_READY	0x20
#define NAND_STATUS_READY	    0x40
#define NAND_STATUS_WP		    0x80

#define NF_CS_DIS_ALL

//arthurc3921 add++
enum{
	ECC_16=0,
	ECC_24=1,
	ECC_40=2,
	ECC_48=3,
	ECC_60=4,
/*
    ECC_1=2,
	CRC_MODE=3,
	EF_MODE=4,
	BA_MODE=5,
	ECC_24_SCRB=6,
	LBA_PNP=0,
*/
};

struct NAND_INFO
{
    u32 bytes_perpage;
    u32 pages_perblock;	
    u32 eccsec_size;
    u32 eccredu_size;
    u32 rowaddr_cycle;
    u32 ecctype;
    u32 read_timing;
    u32 write_timing;
    u32 eccsec_perpage;
    u32 pageshift_perblock;
    u32 cfg_addr;
    u32 cfg_len;
    u32 bl_addr;
    u32 bl_len;
    u32 crypto_en;
};

struct BA_NAND_CONFIG
{
	u32 wBlksPerChip;		//Blocks per chip
	u32 wBytesPerPage;		//Bytes per page (2K,4K,8K etc)  
	u32 wPagesPerBlock;		//Pages per Block(64,128, 192,256 etc) 
	u32 wReserved0[6];		//Reserved for compatibility, filled with 0xffffffff
	u32 wReadClock;			//Read clock setting 
	u32 wWriteClock;		//Write clock setting
	u32 wReserved1[5];		//Reserved for compatibility, filled with 0xffffffff
};

static struct NAND_INFO  nf_info;
static void nf_cmd(unsigned int command, int column, int page_addr);
//arthurc3921 add--

//static unsigned long total_nand_size; /* in kiB */

static PMI_t *pPMI = NULL;
static u8 pmi_buf[MAX_PMI_SIZE + NAND_BUFFER_ADD_SIZE];
#ifndef HAVE_PART_TBL_PARTITION
static PART_TBL PartitionInfo;
#else
static PART_TBL *p_partTbl=NULL;
static PART_TBL *p_PMIPartInfo=NULL;
static u8 parttbl_buf[sizeof(PART_TBL)];
static unsigned int offset_parttbl,len_parttbl;
#endif

extern int add_mtd_device(struct mtd_info *mtd);
extern int del_mtd_device (struct mtd_info *mtd);

static char ali_nand_lock_flag = 0;

#define boot_gettime()   get_timer(0)



// support read retry
// for MICRON
/* Extended commands for ONFI */
#define NAND_CMD_MICRON_GET_FEATURES	0xEE
#define NAND_CMD_MICRON_SET_FEATURES	0xEF
/* Feature Address Definitions */
#define FADDR_TIMING_MODE		0x01
#define FADDR_READ_RETRY		0x89

// for HYNIX
// current support:
// H27UBG8T2BTR ID AD DE 94 97 44 45 1xnm 2nd 64GbMLC D-die
// H27UCG8T2ETR ID AD DE 94 A7 42 48 1xnm 2nd 64GbMLC E-die
// H27UCG8T2ATR ID AD DE 94 DA 74 C4 F20 64Gb MLC 20nm A-die
// H27UCG8T2BTR ID AD DE 94 EB 74 44 F20 64Gb MLC 20nm B-die
// H27UBG8T2CTR ID AD D7 94 91 60 44 F20 32Gb MLC 20nm C-die

// H27UBG8T2BTR RRReg 38 39 3A 3B
// H27UCG8T2ETR RRReg 38 39 3A 3B
// H27UCG8T2ATR RRReg CC / BF / AA / AB / CD / AD / AE / AF
// H27UCG8T2BTR RRReg B0 / B1 / B2 / B3 / B4 / B5 / B6 / B7
// H27UBG8T2CTR RRReg B0 / B1 / B2 / B3 / B4 / B5 / B6 / B7

// H27UBG8T2BTR RRSeq FFh ¡V36h ¡V 38h ¡V 52h ¡V 16h ¡V 17h ¡V 04h ¡V 19h ¡V 00h ¡K 30h ¡V Data Out - FFh - 36h ¡V 38h ¡V 00h ¡V 16h ¡K
// H27UCG8T2ETR RRSeq FFh ¡V36h ¡V 38h ¡V 52h ¡V 16h ¡V 17h ¡V 04h ¡V 19h ¡V 00h ¡K 30h ¡V Data Out - FFh - 36h ¡V 38h ¡V 00h ¡V 16h ¡K
// H27UBG8T2CTR RRSeq FFh ¡V36h ¡VFFh¡V40h¡VCCh-4Dh -16h ¡V17h ¡V04h ¡V19h ¡V00h ¡K
// H27UBG8T2CTR RRSeq FFh ¡V36h ¡VAEh¡V00h¡VB0h-4Dh -16h ¡V17h ¡V04h ¡V19h ¡V00h ¡K
// H27UBG8T2CTR RRSeq FFh ¡V36h ¡VAEh¡V00h¡VB0h-4Dh -16h ¡V17h ¡V04h ¡V19h ¡V00h ¡K

#define COPY_SET_1xnm 8
#define RR_TABLE_PER_COPY_SET_1xnm 2
#define RR_PARAM_PER_RR_TABLE_1xnm 8
#define RR_PARAM_SIZE_1xnm 4
#define COPY_SET_20nm 8
#define RR_TABLE_PER_COPY_SET_20nm 2
#define RR_PARAM_PER_RR_TABLE_20nm 8
#define RR_PARAM_SIZE_20nm 8

enum NF_CHIP {
	RR_MODE_NONE = 0,
	RR_MODE_MICRON = 1,
	RR_MODE_HYNIX_20nm_A = 2,
	RR_MODE_HYNIX_20nm_B = 3,
	RR_MODE_HYNIX_20nm_C = 4,
	RR_MODE_HYNIX_1xnm = 5,
};

typedef struct RR_TABLE_1xnm
{
	uint8_t  rr_total_count[8];
	uint8_t  rr_reg_count[8];
	uint8_t  rr_pram_set[COPY_SET_1xnm][RR_TABLE_PER_COPY_SET_1xnm][RR_PARAM_PER_RR_TABLE_1xnm][RR_PARAM_SIZE_1xnm];
}RR_TABLE_1xnm_t;

typedef struct RR_TABLE_20nm
{
    uint8_t  rr_total_count;
	uint8_t  rr_reg_count;
    uint8_t  rr_pram_set[COPY_SET_20nm][RR_TABLE_PER_COPY_SET_20nm][RR_PARAM_PER_RR_TABLE_20nm][RR_PARAM_SIZE_20nm];
}RR_TABLE_20nm_t;

#define RR_TABLE_LEN_1xnm COPY_SET_1xnm*RR_TABLE_PER_COPY_SET_1xnm*RR_PARAM_PER_RR_TABLE_1xnm*RR_PARAM_SIZE_1xnm
#define RR_TABLE_LEN_20nm COPY_SET_20nm*RR_TABLE_PER_COPY_SET_20nm*RR_PARAM_PER_RR_TABLE_20nm*RR_PARAM_SIZE_20nm
uint8_t RR_Table_1xnm[RR_TABLE_LEN_1xnm];
uint8_t RR_Table_20nm[RR_TABLE_LEN_20nm];
uint8_t RR_Params_1xnm[RR_PARAM_PER_RR_TABLE_1xnm][RR_PARAM_SIZE_1xnm];
uint8_t RR_Params_20nm[RR_PARAM_PER_RR_TABLE_20nm][RR_PARAM_SIZE_20nm];


static uint8_t rr_mode = RR_MODE_NONE;
static uint8_t g_retry_options_nb = 0;
static uint8_t hynix_option = 0;

//arthurc3921 add++
#define BA_NAND_CONFIG_OFFSET	0x290
#define BA_NAND_INFO_OFFSET		0x2D0
#define BA_NAND_RR_MODE_OFFSET	0x2E0
#define BA_NAND_CHIP_ID_OFFSET	0x2F0
#define BA_SCAN_START			0//0//75264
#define BA_SCAN_RANGE			(BA_SCAN_START + 4096)
#define BA_PAGE_SKIP			64

#if 1 // arthur: origin version.
static void nf_wait_ready(void)
{
    nf_cmd(NAND_CMD_STATUS, -1, -1);
    while(1)
    {
        udelay(100);
        if(nfreg_read(NF_bPIODATA) & NAND_STATUS_READY)
            break;
    };
}

void nf_set_seed(u32 sec)
{
    u8 tmp;
    if(nf_info.crypto_en)
    {
        tmp = nfreg_read(NF_bEDORBCYC);
        tmp &= ~0x70;
        nfreg_write(tmp, NF_bEDORBCYC);
        if(sec % nf_info.eccsec_perpage)
        {
            tmp |= ((sec % nf_info.eccsec_perpage) << 4);
            nfreg_write(tmp, NF_bEDORBCYC);
        } 
    }
}

static void nf_host_init(void)
{
	u8 tmp;

	nfreg_write(NF_EN, NF_bMODE);    	

	nfreg_write(0x00, NF_bDMACTRL);
	nfreg_write32(0x00, NF_dwINTFLAG);

	if(nf_info.crypto_en)
	{        
		tmp = nfreg_read(NF_bEDORBCYC);
		tmp |= NF_CRYPTO_EN;
		nfreg_write(tmp, NF_bEDORBCYC);
	}
	else
	{
		tmp = nfreg_read(NF_bEDORBCYC);
		tmp &= ~NF_CRYPTO_EN;
		nfreg_write(tmp, NF_bEDORBCYC);
	}	

	switch (nf_info.ecctype)
	{
		case ECC_16:
		    nfreg_write(NF_ECC_EN | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write(NF_FW_RED_4, NF_bDMALEN);
		    break;
		case ECC_24:
		    nfreg_write(NF_ECC_EN | NF_BCH_24B_MODE | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write(NF_FW_RED_4, NF_bDMALEN);
		    break;
		case ECC_40:
		    nfreg_write(NF_ECC_EN | NF_BCH_40B_MODE | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write(NF_FW_RED_4, NF_bDMALEN);
		    break;
		case ECC_48:
		    nfreg_write(NF_ECC_EN | NF_BCH_48B_MODE | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write(NF_FW_RED_4, NF_bDMALEN);
		    break;
		case ECC_60:
		    nfreg_write(NF_ECC_EN | NF_BCH_60B_MODE | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write(NF_FW_RED_4, NF_bDMALEN);
		    break;
	}
}

static void set_dma_length(u8 sectors)
{
    u8 tmp;

    tmp = nfreg_read(NF_bDMALEN);
    tmp &= 0xE0;
    tmp |= sectors;    
    nfreg_write(tmp, NF_bDMALEN);
}

/* set DMA sectors and dma buffer address */
static void set_dma_addr(u32 addr)
{
    u32 tmp;

    tmp = nfreg_read32(NF_dwDMACONFIG);
    tmp |= (INIT_DMA_IMB | INIT_DMA_SRAM);
    nfreg_write32(tmp, NF_dwDMACONFIG);
    tmp &= ~(INIT_DMA_IMB | INIT_DMA_SRAM);
    nfreg_write32(tmp, NF_dwDMACONFIG);

    nfreg_write32(addr, NF_dwDMAADDR);
}

/* set DMA read start*/
static void set_dma_read_start(u32 to_sram)
{
    /*Bryan suggestion, to suppress glitch */
    nfreg_write32(0, NF_dwINTFLAG);
    nfreg_write(NF_DMA_EN | (to_sram ? 0 : NF_DMA_IMB_EN), NF_bDMACTRL);        /* Enable DMA */    
}

static u32 check_read_dma_done(u32 is_sram, u32 secs)
{
	u32 tmp, err;
	u32 timeo = 5000;//boot_gettime();
	u32 mask = NF_ECC_DONE |NF_DMA_DONE;
				
	if (!is_sram)
	{
		mask |= (IMB_WR_FSH_FLAG << 16);
	}
	//while ((boot_gettime() - timeo) < 100*DELAY_1MS)
	while (timeo--)//((boot_gettime() - timeo) < 100*DELAY_1MS)
	{
		tmp = nfreg_read32(NF_dwINTFLAG);
		if (mask == (tmp & mask))
		{
			if (nfreg_read32(NF_bECCCTRL) & NF_ALL_FF_FLAG) 
	  		    return 0x0;				            
			err = 0;
			tmp = nfreg_read32(NF_wECCCURSEC) & 0xffff;
			while(secs)		            
			{	
			    if (!(tmp & 0x1))
				{
				    err ++;
				}
				tmp = tmp >> 1;
				secs--;
			}		            
			return err;						
		}              
		udelay(5);
	}    
	return 0xFF00FF00;
}

static void hw_ecc_init(void)
{
    /* clear HW Ecc status */
    nfreg_write(0, NF_bECCSTS);
    nfreg_write32(0, NF_wECCERROCR);
    nfreg_write32(0xffff, NF_wECCCURSEC);
}

static void hw_ecc_enable(void)
{
    u8 tmp;

    tmp = nfreg_read(NF_bECCCTRL);
    tmp &= ~NF_ECC_BYPASS;
    tmp |= NF_ECC_NON_STOP | NF_ECC_EN;
    nfreg_write(tmp, NF_bECCCTRL);
    hw_ecc_init();
    
}

static void hw_ecc_disable(void)
{
}

static void nf_ctrl(int cmd, unsigned int ctrl)
{
    if (0 == (ctrl & NAND_NCE))
    {
        nfreg_write(NF_CEJ, NF_bCTRL);
        return;
    }

    if (ctrl & NAND_CTRL_CHANGE)
    {
		if (ctrl & NAND_CLE)
		{
			nfreg_write(NF_CLE, NF_bCTRL);
		}
		else if (ctrl & NAND_ALE)
		{
			nfreg_write(NF_ALE, NF_bCTRL);
		}
		else
		{
			nfreg_write(0, NF_bCTRL);
		}
    }

    if (NAND_CMD_NONE != cmd)
	{
		nfreg_write((u8) cmd, NF_bPIODATA);//write command to register 0x1c
	}
}

static void nf_cmd(unsigned int command,
                             int column, int page_addr)
{
//    unsigned long timeo = boot_gettime();
    unsigned long timen;
    timen = 5000;//timeo + DELAY_1MS;


    /* Command latch cycle */
    nf_ctrl(command & 0xff,
                   NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);

    if ((column != -1) || (page_addr != -1))
    {
        int ctrl = NAND_CTRL_CHANGE | NAND_NCE | NAND_ALE;

        /* not support 512 page nand */
        if (column != -1) {
            nf_ctrl(column, ctrl);
            ctrl &= ~NAND_CTRL_CHANGE;
            nf_ctrl(column >> 8, ctrl);
        }
        if (page_addr != -1) {
            if (192 == nf_info.pages_perblock)
			{
                page_addr = (page_addr / 192) * 256 + (page_addr % 192);
			}
            nf_ctrl(page_addr, ctrl);
            nf_ctrl(page_addr >> 8, ctrl);
            if (nf_info.rowaddr_cycle >= 3)
            {
			    nf_ctrl(page_addr>>16, ctrl);
			}
            if (nf_info.rowaddr_cycle == 4)
            {
			    nf_ctrl(page_addr>>24, ctrl);
			}
        }
        
    }
    nf_ctrl(NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

    /*
     * program and erase have their own busy handlers
     * status, sequential in, and deplete1 need no delay
     */
    switch (command)
    {
        case NAND_CMD_STATUS:
            return;
        case NAND_CMD_RESET:
            udelay(1000);
//            timen = timeo + 20*DELAY_1MS;
            nf_ctrl(NAND_CMD_STATUS,
                           NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
            nf_ctrl(NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
            while(timen--)//(timen < timeo)// (time_before(jiffies, timeo))
            {
                if (nfreg_read(NF_bPIODATA) & NAND_STATUS_READY)
				{
                    break;
				}
                udelay(1000);//timeo = boot_gettime();        
            }
            return;

        case NAND_CMD_READID:
            nf_ctrl(NAND_CMD_READ0,
                           NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
            nf_ctrl(NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
            return;

        case NAND_CMD_READ0:
            nf_ctrl(NAND_CMD_READSTART,
                           NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
            nf_ctrl(NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
            //nf_ctrl(mtd, chip);
			nf_wait_ready();
            nf_ctrl(NAND_CMD_READ0,
                           NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
            nf_ctrl(NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
			return;
    }
}

/*
Read from nand to sram or dram by ecc sectors
*/
static u32 nf_dma_read(u32 sec, int len, u8 *buf, u32 unit, u32 to_sram)
{
    u32 col; 
    u32 page;
    u32 secs;
    u32 status;
    struct NAND_INFO *info = &nf_info;

    while(len > 0)
    {
        col  = sec % info->eccsec_perpage;
        if(col + unit > info->eccsec_perpage)
		{
            secs =  info->eccsec_perpage - col;
		}
        else
        {
		    secs = unit;
		}
        if(secs > len)
        {
		    secs = len;
		}
        page = sec / info->eccsec_perpage;
        col *= (info->eccsec_size + info->eccredu_size);
        nf_cmd(NAND_CMD_READ0, col, page);	 
        hw_ecc_enable();
        nfreg_write32(0x00060000, NF_dwDMACONFIG);	// enable reset DMA and SRAM
        nfreg_write32(0x00000000, NF_dwDMACONFIG);	// disable reset DMA and SRAM        	
		hw_ecc_init();
		nf_host_init();
		set_dma_length(secs);
		set_dma_addr((u32)((u32)chip_hw_dma_buf & ~0x80000000));
		set_dma_read_start(to_sram);
		status = check_read_dma_done(to_sram, secs);
        if (status != 0)
        {
			printk("[ERR] %s!\n", __FUNCTION__);
			return !RET_SUCCESS;
        }
        hw_ecc_disable();
        //dma_stop();
		if(to_sram && (buf != NULL))
		{
			memcpy((u8 *)buf, (u8 *)(ali_nand_reg + 0x1000), secs * info->eccsec_size);
		}
		else  if(buf != NULL)
		{
			memcpy((u8 *)buf, (u8 *)chip_hw_dma_buf, len * info->eccsec_size);
		}
		
        sec += secs;
        len -= secs;
        buf += secs * info->eccsec_size;
    }
    return RET_SUCCESS;
}

/*
Read from nand to sram or dram by page alignment, so length must small than one page size.
page: page alignment.
len : sector unit and need < page size (2k)
*/
u32 nf_dma_read2(u32 page, int len, u8 *buf, u32 to_sram)
{
    u32 col = 0;
    u32 status = 0;
	u32 eccsec_size = 1024;

    nf_cmd(NAND_CMD_READ0, col, page);

    hw_ecc_enable();
    nfreg_write32(0x00060000, NF_dwDMACONFIG);	// enable reset DMA and SRAM
    nfreg_write32(0x00000000, NF_dwDMACONFIG);	// disable reset DMA and SRAM
    hw_ecc_init();
    nf_host_init();
    set_dma_length(len);
    set_dma_addr((u32)((u32)chip_hw_dma_buf & ~0x80000000));
    set_dma_read_start(to_sram);
    status = check_read_dma_done(to_sram, 8);
	
    hw_ecc_disable();
	//dma_stop();
	if(to_sram && buf != NULL)
	{
		memcpy((u8 *)buf, (u8 *)(ali_nand_reg + 0x1000), len * eccsec_size);
	}
	else  if(buf != NULL)
	{
		memcpy((u8 *)buf, (u8 *)chip_hw_dma_buf, len * eccsec_size);
	}

    if (status != 0)
    {
		//printk("[ERR] %s!\n", __FUNCTION__);
        return !RET_SUCCESS;
    }
    return RET_SUCCESS;
}
#endif
//arthurc3921 add--

int hynix_check_rr_table_1xnm(uint8_t *table)
{
	int i, j, k, ret = 0;
	RR_TABLE_1xnm_t *rr_table = (RR_TABLE_1xnm_t *)table;

	printk("%s \n", __FUNCTION__);
	for(i=0; i<RR_TABLE_PER_COPY_SET_1xnm; i++)
	{
		ret = 0;
		for(j=0; j<RR_PARAM_PER_RR_TABLE_1xnm; j++)
		{
			for(k=0; k<RR_PARAM_SIZE_1xnm; k++)
			{
				// exclusive-OR success, check each bit of org and inv set is different.
				if(rr_table->rr_pram_set[i][0][j][k] != ~(rr_table->rr_pram_set[i][1][j][k]))
				{
					ret = -1;
					break;
				}
				RR_Params_1xnm[j][k] = rr_table->rr_pram_set[i][0][j][k];
			}
			if(ret == -1)
			{
				break;
			}
		}
		if(ret == 0)
		{
			// got rr_params
			return 0;
		}
	}
	return ret;
}

int hynix_check_rr_table_20nm(uint8_t *table)
{
	int i, j, k, ret = 0;
	RR_TABLE_20nm_t *rr_table = (RR_TABLE_20nm_t *)table;

	printk("%s \n", __FUNCTION__);
	for(i=0; i<RR_TABLE_PER_COPY_SET_20nm; i++)
	{
		ret = 0;
		for(j=0; j<RR_PARAM_PER_RR_TABLE_20nm; j++)
		{
			for(k=0; k<RR_PARAM_SIZE_20nm; k++)
			{
				// exclusive-OR success, check each bit of org and inv set is different.
				if(rr_table->rr_pram_set[i][0][j][k] != ~(rr_table->rr_pram_set[i][1][j][k]))
				{
					ret = -1;
					break;
				}
				RR_Params_20nm[j][k] = rr_table->rr_pram_set[i][0][j][k];
			}
			if(ret == -1)
			{
				break;
			}
		}
		if(ret == 0)
		{
			// got rr_params
			return 0;
		}
	}
	return ret;
}

int hynix_get_rr_table_1xnm(void)// only
{
	int i, ret;

	// Read Retry Table
// [CMD FF 36] [ADDR 38] [WD 52] [CMD 16 17 04 19 00] [ADDR 00 00 00 02 00] [CMD 30] [RD 08 08 ...] [CMD FF 36] [ADDR 38] [WD 00] [CMD 16 00] [ADDR dummy] [CMD 30]
	nf_ctrl(0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x36, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x38, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x52, NAND_NCE | NAND_CTRL_CHANGE);
	nf_ctrl(0x16, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x17, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x04, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x19, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x02, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x30, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	for(i=0; i<RR_TABLE_LEN_1xnm; i++)
	{
		RR_Table_1xnm[i] = nfreg_read(NF_bPIODATA);
	}
	nf_ctrl(0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x36, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x38, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_CTRL_CHANGE);
	nf_ctrl(0x16, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);

	//Dummy Read(Address Don't Care)
	nf_ctrl(0x00, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x30, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	// check RR_Table if correct.
	ret = hynix_check_rr_table_1xnm(RR_Table_1xnm);

	if(ret == -1)
	{
		printk("[ERR] %s !\n",__FUNCTION__);
	}
	
	return ret;
}

int hynix_get_rr_table_20nm(void)
{
	int i, ret;

	// Read Retry Table
	// A-die
	// [CMD FF 36] [ADDR FF] [WD 40] [ADDR CC] [WD 4D] [CMD 16 17 04 19 00] [ADDR 00 00 00 02 00] [CMD 30] [RD 08 08 ...] [CMD FF 38]
	// B-die C-die
    // [CMD FF 36] [ADDR AE] [WD 00] [ADDR B0] [WD 4D] [CMD 16 17 04 19 00] [ADDR 00 00 00 02 00] [CMD 30] [RD 08 08 ...] [CMD FF 38]
	nf_ctrl(0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x36, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	if( rr_mode == RR_MODE_HYNIX_20nm_A)
	{
		nf_ctrl(0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	}
	else
	{
		nf_ctrl(0xAE, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	}
	if( rr_mode == RR_MODE_HYNIX_20nm_A)
	{
		nf_ctrl(0x40, NAND_NCE | NAND_CTRL_CHANGE);
	}
	else
	{
		nf_ctrl(0x00, NAND_NCE | NAND_CTRL_CHANGE);
	}
	if( rr_mode == RR_MODE_HYNIX_20nm_A)
	{
		nf_ctrl(0xCC, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	}
	else
	{
		nf_ctrl(0xB0, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	}
	nf_ctrl(0x4D, NAND_NCE | NAND_CTRL_CHANGE);
	nf_ctrl(0x16, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x17, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x04, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x19, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x02, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	nf_ctrl(0x30, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	for(i=0; i<RR_TABLE_LEN_20nm; i++)
	{
		RR_Table_20nm[i] = nfreg_read(NF_bPIODATA);
	}
	nf_ctrl(0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x38, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	nf_ctrl(0x00, NAND_NCE | NAND_CTRL_CHANGE);

	// check RR_Table if correct.
	ret = hynix_check_rr_table_20nm(RR_Table_20nm);
	if(ret == -1)
	{
		printk("[ERR] %s !\n",__FUNCTION__);
	}
	
	return ret;
}

void hynix_get_param_rr_1xnm(struct mtd_info *mtd, uint8_t *para)
{
	struct nand_chip *chip = mtd->priv;

	printk("%s \n", __FUNCTION__);
// [CMD 37h] [ADDR Reg#1 38h] [RD Para1 ??] [ADDR Reg#2 39h] [RD Para2 ??]
	chip->cmd_ctrl(mtd, 0x37 & 0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, 0x38 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	*(para) = readb(chip->IO_ADDR_R);
	chip->cmd_ctrl(mtd, 0x39 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	*(para + 1) = readb(chip->IO_ADDR_R);
	chip->cmd_ctrl(mtd, 0x3A & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	*(para + 2) = readb(chip->IO_ADDR_R);
	chip->cmd_ctrl(mtd, 0x3B & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	*(para + 3) = readb(chip->IO_ADDR_R);
}

void hynix_set_param_rr_1xnm(struct mtd_info *mtd, uint8_t *params)
{
	struct nand_chip *chip = mtd->priv;

	printk("%s \n", __FUNCTION__);
// [CMD 36h] [ADDR Reg#1 38h] [WD Para1] [ADDR Reg#2 39h] [WD Para2] 
//           [ADDR Reg#3 3Ah] [WD Para3] [ADDR Reg#4 3Bh] [WD Para4] [CMD 16h]
	chip->cmd_ctrl(mtd, 0x36 & 0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, 0x38 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, *params    , NAND_NCE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, 0x39 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, *(params+1), NAND_NCE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, 0x3A & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, *(params+2), NAND_NCE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, 0x3B & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, *(params+3), NAND_NCE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, 0x16 & 0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	
	// check setting
	{
		uint8_t params_rd[4];
		hynix_get_param_rr_1xnm(mtd, params_rd);
		if( (*params != params_rd[0]) || (*(params+1) != params_rd[1]) || (*(params+2) != params_rd[2]) || (*(params+3) != params_rd[3]) )
		{
			printk("[ERR] %s !\n", __FUNCTION__);
		}
	}
}

void hynix_set_param_rr_20nm(struct mtd_info *mtd, uint8_t *params)
{
	struct nand_chip *chip = mtd->priv;

	printk("%s \n", __FUNCTION__);
// [CMD 36h] [ADDR Reg#1 B0h] [WD Para1] [ADDR Reg#2 B1h] [WD Para2] [ADDR Reg#3 B2h] [WD Para3] [ADDR Reg#4 B3h] [WD Para4] 
//           [ADDR Reg#5 B4h] [WD Para5] [ADDR Reg#6 B5h] [WD Para6] [ADDR Reg#7 B6h] [WD Para7] [ADDR Reg#8 B7h] [WD Para8] [CMD 16h]
	if(rr_mode == RR_MODE_HYNIX_20nm_A)
	{
		chip->cmd_ctrl(mtd, 0x36 & 0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xCC & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *params    , NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xBF & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+1), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xAA & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+2), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xAB & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+3), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xCD & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+4), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xAD & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+5), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xAE & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+6), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xAF & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+7), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0x16 & 0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	}
	else //RR_MODE_HYNIX_20nm_B RR_MODE_HYNIX_20nm_C
	{
		chip->cmd_ctrl(mtd, 0x36 & 0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xB0 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *params    , NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xB1 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+1), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xB2 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+2), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xB3 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+3), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xB4 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+4), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xB5 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+5), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xB6 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+6), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0xB7 & 0xFF, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, *(params+7), NAND_NCE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, 0x16 & 0xFF, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	}
	// check setting
	// N/A
}

// N/A
//void hynix_get_param_rr_20nm(void)
//{
//}


//for MICRON
/* 
 * Set feature command for MICRON NAND Flash 
 * 
 */
static int nand_micron_set_feature(struct mtd_info *mtd,  uint8_t feature_addr, uint8_t *option)
{
	struct nand_chip *chip = mtd->priv;
	
	printk("%s FA 0x%x option 0x%x\n", __FUNCTION__, feature_addr, option[0]);
	/* Set Feature command for MICRON */
	chip->cmdfunc(mtd, NAND_CMD_MICRON_SET_FEATURES, feature_addr, -1);
	writeb(option[0], chip->IO_ADDR_W);
	writeb(option[1], chip->IO_ADDR_W);
	writeb(option[2], chip->IO_ADDR_W);
	writeb(option[3], chip->IO_ADDR_W);

	return 0;
}

/* 
 * Set read retry option for MICRON NAND Flash
 * 
 */
static int nand_micron_set_feature_rr(struct mtd_info *mtd, uint8_t option)
{
	uint8_t p[4]={0, 0, 0, 0};
	p[0]=option;
	
	printk("%s 0x%x\n", __FUNCTION__, option);
	nand_micron_set_feature(mtd, FADDR_READ_RETRY, p);

	return 0;
}

/* 
 * Get feature command for MICRON NAND Flash
 * 
 */
static int nand_micron_get_feature(struct mtd_info *mtd, uint8_t feature_addr, uint8_t *option)
{
	struct nand_chip *chip = mtd->priv;

	/* Set Feature command for MICRON */
	chip->cmdfunc(mtd, NAND_CMD_MICRON_GET_FEATURES, feature_addr, -1);
	option[0] = readb(chip->IO_ADDR_R);
	option[1] = readb(chip->IO_ADDR_R);
	option[2] = readb(chip->IO_ADDR_R);
	option[3] = readb(chip->IO_ADDR_R);
//	printk("%s FA 0x%x option 0x%x\n", __FUNCTION__, feature_addr, option[0]);
	
	return 0;
}

/* 
 * Get read retry option for MICRON NAND Flash
 * 
 */
static int nand_micron_get_feature_rr(struct mtd_info *mtd)
{
	uint8_t option[4] = {0};
	
	nand_micron_get_feature(mtd, FADDR_READ_RETRY, option);
//	printk("%s 0x%x\n", __FUNCTION__, option[0]);

	return option[0];
}

/* 
 * Set feature command read retry for NAND Flash supported Read Retry
 * 
 */
static int nand_set_feature_rr(struct mtd_info *mtd, uint8_t option)
{
	if(rr_mode == RR_MODE_NONE)
	{
		return 0;
	}
	else if(rr_mode == RR_MODE_MICRON)
	{
		nand_micron_set_feature_rr(mtd, option);
	}
	if(rr_mode == RR_MODE_HYNIX_1xnm)
	{
		hynix_set_param_rr_1xnm(mtd, &(RR_Params_1xnm[option][0]));
	}
	else if((rr_mode == RR_MODE_HYNIX_20nm_A) || (rr_mode == RR_MODE_HYNIX_20nm_B) || (rr_mode == RR_MODE_HYNIX_20nm_C))
	{
		hynix_set_param_rr_20nm(mtd, &(RR_Params_20nm[option][0]));
	}
	return 0;
}

//arthurc3921 add++
// for get data
char word_to_bit(u16 word)
{
	u8 i, bit0_cnt=0, bit1_cnt=0;

	for (i=0; i<16; i++)
	{
		if((word >> i) & 0x01)
		{
			bit1_cnt++;
		}
		else
		{
			bit0_cnt++;
		}
	}
	
	if(bit1_cnt > 9)
	{
		return 1;
	}
	else if(bit0_cnt > 9)
	{
		return 0;
	}
	
	return -1;
}

u16 ba_chip_id_pattern[4]={0x39, 0x21, 0x55, 0xAA};


int ba_check_chip_id(u8 *buf)
{
	unsigned char i=0, j=0;
	unsigned short *p = (unsigned short *)buf;
	unsigned char data[4];

	p = (u16 *)buf;

	memset((u8 *)data, 0, 4);

	for(i=0; i<4; i++)
	{
		for(j=0; j<8; j++)
		{
			data[i] |= word_to_bit(*(p+j)) << j;
		}
		if(data[i] != ba_chip_id_pattern[i])
		{
			return -1;
		}
		p = p+8;
	}
	return RET_SUCCESS;
}

void ba_get_nand_info(u8 *buf)
{
	volatile unsigned long i = 0;
	volatile unsigned char data = 0;
	unsigned short *p = (unsigned short *)buf;
	volatile unsigned char ECC_Tppe = 0;
	volatile unsigned char Bytes_Perpage = 0;
	volatile unsigned char Pages_PerBlock = 0;
	
	data = 0x00;
	for(i=0; i<8; i++)
	{
		data |= word_to_bit(*(p+i)) << i;
	}

	ECC_Tppe = data & 0x07;
	if(1 == ECC_Tppe)
	{
		nf_info.ecctype       = ECC_16;
		nf_info.eccredu_size  = 32;
		bch_mode = NF_BCH_16B_MODE;
		probe_info("NF_BCH_16B_MODE \n\r");
	}
	else if(2 == ECC_Tppe)
	{
		nf_info.ecctype       = ECC_24;
		nf_info.eccredu_size  = 46;
		bch_mode = NF_BCH_24B_MODE;
		probe_info("NF_BCH_24B_MODE \n\r");
	}
	else if(3 == ECC_Tppe)
	{
		nf_info.ecctype       = ECC_40;
		nf_info.eccredu_size  = 74;
		bch_mode = NF_BCH_40B_MODE;
		probe_info("NF_BCH_40B_MODE \n\r");
	}
	else if(4 == ECC_Tppe)
	{
		nf_info.ecctype       = ECC_48;
		nf_info.eccredu_size  = 88;
		bch_mode = NF_BCH_48B_MODE;
		probe_info("NF_BCH_48B_MODE \n\r");
	}
	else if(5 == ECC_Tppe)
	{
		nf_info.ecctype       = ECC_60;
		nf_info.eccredu_size  = 110;
		bch_mode = NF_BCH_60B_MODE;
		probe_info("NF_BCH_60B_MODE \n\r");
	}
	else
	{
		printf("[ERR] ECC MODE INVALID %d\n\r", ECC_Tppe);
	}

	Bytes_Perpage = (data & 0x18) >> 3;
	if(0 == Bytes_Perpage)
	{
		nf_info.bytes_perpage = 0x4000;
		nf_info.eccsec_perpage = 0x10;
	}
	else if(1 == Bytes_Perpage)
	{
		nf_info.bytes_perpage = 0x2000;
		nf_info.eccsec_perpage = 0x8;
	}
	else if(2 == Bytes_Perpage)
	{
		nf_info.bytes_perpage = 0x1000;
		nf_info.eccsec_perpage = 0x4;
	}
	else if(3 == Bytes_Perpage)
	{
		nf_info.bytes_perpage = 0x800;
		nf_info.eccsec_perpage = 0x2;
	}

	Pages_PerBlock = (data & 0x60) >> 5;
	if(0 == Pages_PerBlock)
	{
		nf_info.pages_perblock = 64;
	}
	else if(1 == Pages_PerBlock)
	{
		nf_info.pages_perblock = 128;
	}
	else if(2 == Pages_PerBlock)
	{
		nf_info.pages_perblock = 192;
	}
	else if(3 == Pages_PerBlock)
	{
		nf_info.pages_perblock = 256;
	}

	nf_info.eccsec_perpage = (nf_info.bytes_perpage)/1024;
}

void ba_get_nand_rr_mode(u8 *buf)
{
	volatile unsigned long i = 0;
	volatile unsigned char data = 0;
	volatile unsigned short *p = (u16 *)buf;
	
	data = 0x00;
	for(i=0; i<8; i++)
	{
		data |= word_to_bit(*(p+i)) << i;
	}

	rr_mode = data & 0x07;

	switch(rr_mode)
	{
		case RR_MODE_MICRON:			// only for MICRON 29F64G08CBABA(2C64444BA9)
										// only for MICRON 29F32G08CBADA(2C44444BA9)
			rr_mode = RR_MODE_MICRON;
			break;
		case RR_MODE_HYNIX_20nm_A:		// only for HYNIX H27UCG8T2ATR-BC(ADDE94DA74C4) F20 64Gb MLC A-die
		case RR_MODE_HYNIX_20nm_B:		// only for HYNIX H27UBG8T2BTR-BC(ADDE94EB7444) F20 64Gb MLC B-die
		case RR_MODE_HYNIX_20nm_C:		// only for HYNIX H27UBG8T2CTR-BC(ADD794916044) F20 64Gb MLC
			hynix_get_rr_table_20nm();
			break;
		case RR_MODE_HYNIX_1xnm:		// only for HYNIX H27UCG8T2DTR(ADDE94974445) 1xnm 1st 64Gb MLC D-die
										// only for HYNIX H27UCG8T2ETR(ADDE94A74248) 1xnm 2nd 64Gb MLC E-die
			hynix_get_rr_table_1xnm();
		default:
			rr_mode = RR_MODE_NONE;
			break;
	}
}

/*
Scan boot area from page_start to page_end page and skip page_skip page.
*/
// page_start:  start page number.
// page_end:    end page number.
// page_skip:   skip page number.
//u8 ba_nand_info_data[2048];
u32 scs_nand_info_scan(u32 page_start)
{
	u8  is_scramble = 0;
	u32 page = 0;
	struct BA_NAND_CONFIG *ba_nand_config = NULL;
	
	//read first 2K data from Nandflash into internal memory
	memset(&nf_info, 0x0, sizeof(struct NAND_INFO));

	//fixed the page size, row count, ECC Type 
	nf_info.rowaddr_cycle = 3;
	nf_info.ecctype       = ECC_40; 
	nf_info.eccsec_size   = 1024;
	nf_info.eccredu_size  = 0;
	nf_info.eccsec_perpage = 1;
	nf_info.crypto_en = 0;
	
	//Enable NF
	nfreg_write(NF_EN, NF_bMODE); 
	nf_host_init();
	//Reset Nand Chip
	nf_cmd(NAND_CMD_RESET, -1, -1);

	//get nand info and id
	//search boot area per 64 pages
	for(page=page_start; page<=BA_SCAN_RANGE; page+=BA_PAGE_SKIP)
	{
		//try scramble on/off
		for(is_scramble=0; is_scramble<2; is_scramble++)
		{
			nf_info.crypto_en  = is_scramble;
			nf_dma_read2(page, 1, NULL, 1);
			printf("page:%x, is_scramble:%x \n",page,is_scramble);
			
			if(ba_check_chip_id((u8 *)(NF_BUFFER_ADDR+BA_NAND_CHIP_ID_OFFSET)) == RET_SUCCESS)
			{
				ba_get_nand_info((u8 *)(NF_BUFFER_ADDR+BA_NAND_INFO_OFFSET));
				//update nand info read again using real ecc type
				if(nf_dma_read(page*(nf_info.bytes_perpage/nf_info.eccsec_size), 2, NULL, 2, 1) == RET_SUCCESS)
				{
					ba_nand_config = (struct BA_NAND_CONFIG *)(NF_BUFFER_ADDR+BA_NAND_CONFIG_OFFSET);
					printk("%s ba_nand_config: wReadClock 0x%x wWriteClock 0x%x wBlksPerChip 0x%x wBytesPerPage 0x%x wPagesPerBlock 0x%x\n", __FUNCTION__,
					ba_nand_config->wReadClock,
					ba_nand_config->wWriteClock,
					ba_nand_config->wBlksPerChip,
					ba_nand_config->wBytesPerPage,
					ba_nand_config->wPagesPerBlock);
					nf_info.read_timing = ba_nand_config->wReadClock;
					nf_info.write_timing = ba_nand_config->wWriteClock;
					nf_info.bytes_perpage = ba_nand_config->wBytesPerPage;
					nf_info.pages_perblock = ba_nand_config->wPagesPerBlock;
					nf_info.eccsec_perpage = (nf_info.bytes_perpage)/1024;
		
					
					ba_get_nand_info((u8 *)(NF_BUFFER_ADDR+BA_NAND_INFO_OFFSET));
					ba_get_nand_rr_mode((u8 *)(NF_BUFFER_ADDR+BA_NAND_RR_MODE_OFFSET));
					
					printf("%s ok\n", __FUNCTION__);
					return page;
				}
			}
		}
	}
	printf("[ERR] %s fail\n", __FUNCTION__);
	return -1;
}

/*
Scan boot area at page 0 for AS.
*/
u32 scs_nand_info_scan_as(void)
{
	u8  is_scramble = 0;
	u32 page = 0;
	struct BA_NAND_CONFIG *ba_nand_config = NULL;
	
	//read first 2K data from Nandflash into internal memory
	memset(&nf_info, 0x0, sizeof(struct NAND_INFO));

	//fixed the page size, row count, ECC Type 
	bch_mode = NF_BCH_16B_MODE;
	nf_info.ecctype       = ECC_16;
	nf_info.eccredu_size  = 32;
	nf_info.rowaddr_cycle = 3;
	nf_info.eccsec_size   = 1024;
	nf_info.eccsec_perpage = 2;
	nf_info.crypto_en = 0;

	//Enable NF
	nfreg_write(NF_EN, NF_bMODE); 
	nf_host_init();
	//Reset Nand Chip
	nf_cmd(NAND_CMD_RESET, -1, -1);

	//get nand info and id
	//update nand info read again using real ecc type
	if(nf_dma_read(page, 2, NULL, 2, 1) == RET_SUCCESS)
	{
		ba_nand_config = (struct BA_NAND_CONFIG *)(NF_BUFFER_ADDR+BA_NAND_CONFIG_OFFSET);
		printf("%s scs_nand_info_scan_as: wReadClock 0x%x wWriteClock 0x%x wBlksPerChip 0x%x wBytesPerPage 0x%x wPagesPerBlock 0x%x\n", __FUNCTION__,
		ba_nand_config->wReadClock,
		ba_nand_config->wWriteClock,
		ba_nand_config->wBlksPerChip,
		ba_nand_config->wBytesPerPage,
		ba_nand_config->wPagesPerBlock);
		
		nf_info.read_timing = ba_nand_config->wReadClock;
		nf_info.write_timing = ba_nand_config->wWriteClock;
		nf_info.bytes_perpage = ba_nand_config->wBytesPerPage;
		nf_info.pages_perblock = ba_nand_config->wPagesPerBlock;
		nf_info.eccsec_perpage = (nf_info.bytes_perpage)/1024;
		
		printf("%s ok\n", __FUNCTION__);
		return 0;
	}

	printf("[ERR] %s fail\n", __FUNCTION__);
	return -1;
}

//arthurc3921 add--

void ali_nand_reg_dump(void)
{
	int i;
	printf( "ali reg\n");
	printf( "reg60 0x%x\n", *(volatile u32 *) (0x18000060));
	printf( "reg70 0x%x\n", *(volatile u32 *) (0x18000070));
	printf( "reg74 0x%x\n", *(volatile u32 *) (0x18000074));
	printf( "reg7c 0x%x\n", *(volatile u32 *) (0x1800007c));
	printf( "reg84 0x%x\n", *(volatile u32 *) (0x18000084));
	printf( "reg88 0x%x\n", *(volatile u32 *) (0x18000088));
	printf( "reg90 0x%x\n", *(volatile u32 *) (0x18000090));

	for (i = 0; i<0x5c ; i++)
	{
		if((i%16) == 0)
			printf( " \n");
		if( (i>=0x48) && (i<0x4c))
			printf(" 0xXX");
		else
			printf(" 0x%02x", *(volatile u8 *) (ali_nand_reg + i));

	}
	printf( "\n");
}

void ali_nand_reg_dump_mips(void)
{
	int i;
	printf( "ali reg\n");
	printf( "reg60 0x%x\n", *(volatile u32 *) (0xb8000060));
	printf( "reg70 0x%x\n", *(volatile u32 *) (0xb8000070));
	printf( "reg74 0x%x\n", *(volatile u32 *) (0xb8000074));
	printf( "reg7c 0x%x\n", *(volatile u32 *) (0xb800007c));
	printf( "reg84 0x%x\n", *(volatile u32 *) (0xb8000084));
	printf( "reg88 0x%x\n", *(volatile u32 *) (0xb8000088));
	printf( "reg90 0x%x\n", *(volatile u32 *) (0xb8000090));

	for (i = 0; i<0x5c ; i++)
	{
		if((i%16) == 0)
			printf( " \n");
		if( (i>=0x48) && (i<0x4c))
			printf(" 0xXX");
		else
			printf(" 0x%02x", *(volatile u8 *) (ali_nand_reg + i));

	}
	printf( "\n");
}

static int ali_check_pattern(u8 *buf, int pos, int pattern_len, u8 *pattern)
{
	int i;
	uint8_t *p = buf;

	p += pos;

	/* Compare the pattern */
	for (i = 0; i < pattern_len; i++) {	
		if (p[i] != pattern[i])
		{
			return -1;
		}
	}
	return 0;
}

void ali_nand_pinmux_release(int callline)
{
	u32 pinmux = 0;

	if (ali_nand_lock_flag < 1)
	{
		return;
	}

 	switch(chip_id)
	{
		case C3921:
			//Step1: allow share
			pinmux = ali_soc_read(ali_soc_reg+SOC_PINMUX_REG2);
			pinmux |= STRAPIN_SEL_ENABLE;
			ali_soc_write(pinmux, ali_soc_reg+SOC_PINMUX_REG2);
			//Step2: forbid nf
			pinmux = ali_soc_read(ali_soc_reg+SOC_PINMUX_REG1);
			pinmux &= ~(NF_PIN_SEL);
			ali_soc_write(pinmux, ali_soc_reg+SOC_PINMUX_REG1);
			break;
			
		case C3503:
			//Step1: allow share
			pinmux = ali_soc_read(ali_soc_reg+SOC_PINMUX_REG2);
			pinmux |= STRAPIN_SEL_ENABLE;
			ali_soc_write(pinmux, ali_soc_reg+SOC_PINMUX_REG2);
			//Step2: forbid nf
			pinmux = ali_soc_read(ali_soc_reg+SOC_PINMUX_REG1);
			pinmux &= ~(NF_PIN_SEL);
			ali_soc_write(pinmux, ali_soc_reg+SOC_PINMUX_REG1);
			
			// XGPIO[56] pin release
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x434) & ~(1 << 24)), ali_soc_reg + 0x434);
			
			//Step2: forbid nf
			switch(chip_package)
			{
				case 0x00: // BGA 292 Pin package
					// pinmux3: PK292_NAGRA_SEL
					pinmux = ali_soc_read(ali_soc_reg + 0xA4);
					pinmux &= ~(1 << 8);
					ali_soc_write(pinmux, ali_soc_reg + 0xA4);
					break;
					
				case 0x01: // 256 LQFP package
				case 0x02: // 128 LQFP package
	            case 0x03: // 144 LQFP package
	            	// pinmux3: NANDFlash_Func_Sel
					pinmux = ali_soc_read(ali_soc_reg + 0xA4);
					pinmux &= ~(1 << 31);
					ali_soc_write(pinmux, ali_soc_reg + 0xA4);
					break;
					
				default:
					printk("[ERR]  N/A chip package");
					break;
			}
			break;
		default:
			//Step1: allow share
			pinmux = ali_soc_read(ali_soc_reg + SOC_PINMUX_REG2);
			pinmux |= STRAPIN_SEL_ENABLE;
			ali_soc_write(pinmux, ali_soc_reg + SOC_PINMUX_REG2);
	        break;
			
	}
	ali_nand_lock_flag--;
}

void ali_nand_pinmux_set(int callline)
{
	u32 pinmux = 0;
	
	ali_nand_lock_flag++;
 	switch(chip_id)
	{
   		case C3921:
			//printk("%s line %d\n", __FUNCTION__, callline);
			//Step1: select nf
			pinmux = ali_soc_read(ali_soc_reg+SOC_PINMUX_REG1);
			pinmux |= NF_PIN_SEL;
			ali_soc_write(pinmux, ali_soc_reg+SOC_PINMUX_REG1);
			//Step2: forbid share
			pinmux = ali_soc_read(ali_soc_reg+SOC_PINMUX_REG2);
			pinmux &= (~STRAPIN_SEL_ENABLE);
			ali_soc_write(pinmux, ali_soc_reg+SOC_PINMUX_REG2);
			break;

		case C3503:
			//printk("%s line %d\n", __FUNCTION__, callline);
			//Step1: select nf
			pinmux = ali_soc_read(ali_soc_reg+SOC_PINMUX_REG1);
			pinmux &= ~((1<<8)|(1<<16)|(1<<17));
			ali_soc_write(pinmux, ali_soc_reg+SOC_PINMUX_REG1);
			//Step2: chip enable select
			pinmux = ali_soc_read(ali_soc_reg+SOC_PINMUX_REG2);
			pinmux &= ~(1<<21);
			ali_soc_write(pinmux, ali_soc_reg+SOC_PINMUX_REG2);

			//write protect disable XGPIO[56]
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x434) | (1 << 24)), ali_soc_reg + 0x434);
			ali_soc_write((ali_soc_read(ali_soc_reg + 0xD8) | (1 << 24)), ali_soc_reg + 0xD8);
			ali_soc_write((ali_soc_read(ali_soc_reg + 0xD4) | (1 << 24)), ali_soc_reg + 0xD4);

			switch(chip_package)
			{
				case 0x00: //BGA 292 Pin package
					// pinmux3: PK292_NAGRA_SEL
					pinmux = ali_soc_read(ali_soc_reg+0xA4);
					pinmux |= (1 << 8);
					ali_soc_write(pinmux, ali_soc_reg+0xA4);
					break;
					
				case 0x01: // 256 LQFP package
				case 0x02: // 128 LQFP package
	            case 0x03: // 144 LQFP package
					pinmux = ali_soc_read(ali_soc_reg+0xA4);
					pinmux |= (1 << 31);
					ali_soc_write(pinmux, ali_soc_reg+0xA4);
					break;
					
				default:
					printk("[ERR] chip package N/A");
					break;
			}		
			break;
		default:
			printk("[ERR] chip id N/A");
			break;
	}
}

u32 ali_nand_get_pagesize(void)
{
	return nf_info.bytes_perpage;
}

static void enable_cs(int n)
{
    u32 tmp;

	switch (chip_id)
	{
		case C3503:
		case C3901:
		case C3701:
		case C3921:
			// TBD
			break;
		default:
			break;	
	}
}
#if 0//arthur unuse temp rm
static void ali_nand_clk_disable(void)
{
	switch(chip_id)
    {
    	case C3603:
    	case C3811:
    		ali_soc_write((ali_soc_read(ali_soc_reg + 0x64) | (1 << 14)), 
    			ali_soc_reg + 0x64); 	
		break;
		
		case C3901:
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x64) | (1 << 6)), 
				ali_soc_reg + 0x64); 	
		break;
		case C3701:
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x60) | (1 << 2)), 
				ali_soc_reg + 0x60); 	
			break;

		case C3921:
//#if 1 // soc spec
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x60) | (1 << 12)), 
				ali_soc_reg + 0x60); 	
//#else // nf spec
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x90) | (1 << 29)), 
				ali_soc_reg + 0x90);
//#endif
			break;
		default:
			break;
    }
}
#endif

static void ali_nand_clk_enable(void)
{
	switch(chip_id)
	{
		case C3603:
		case C3811:
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x64) & ~(1 << 14)), 
				ali_soc_reg + 0x64); 			
			break;
			
		case C3901:
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x64) & ~(1 << 6)), 
				ali_soc_reg + 0x64); 			
			break;
		case C3503:
		case C3701:
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x60) & ~(1 << 2)), 
				ali_soc_reg + 0x60); 			
			break;
		case C3921:
	//#if 1 // soc spec
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x60) & ~(1 << 12)), 
				ali_soc_reg + 0x60); 			
	//#else // nf spec
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x90) & ~(1 << 29)), 
				ali_soc_reg + 0x90);
	//#endif
		
			break;
		default:
			break;
	}
}

/*
 * init DMA
*/
static void init_hw_dma(void)
{
	u32 tmp;	
	nfreg_write(0, NF_bDMACTRL); 
	tmp = nfreg_read(NF_dwDMACONFIG);    
	tmp |= (INIT_DMA_IMB | INIT_DMA_SRAM);    
	nfreg_write(tmp, NF_dwDMACONFIG);  
	tmp &= ~(INIT_DMA_IMB | INIT_DMA_SRAM);    
	nfreg_write(tmp, NF_dwDMACONFIG);  		

	nfreg_write(0, NF_bECCSTS);    
	nfreg_write(0, NF_dwINTFLAG);	  
	nfreg_write(0xffff, NF_wECCCURSEC);
	nfreg_write(NF_ECC_EN | NF_ECC_NON_STOP | bch_mode, NF_bECCCTRL);  
}

/*
* wait DMA finish
*/	
static int wait_dma_finish(int mode)	
{
    unsigned long  timeo = 0x1000;//arthurrm cannot use jiffies;	
    int int_flag;

	if (mode == HW_DMA_READ)
	{
		int_flag = NF_DMA_DONE | NF_ECC_DONE | IMB_WR_FSH_FLAG;
	}
	else
	{
		int_flag = NF_DMA_DONE;
	}
	
	while (timeo--)
	{
		if ((nfreg_read(NF_dwINTFLAG) & int_flag) == int_flag)		
		{            
			nfreg_write(0, NF_dwINTFLAG); 
			return 0;        
		}  
	}	
	
	/* read again, sometimes, outside loop takes too many time*/
	if ((nfreg_read(NF_dwINTFLAG) & int_flag) == int_flag)		
	{            
		nfreg_write(0, NF_dwINTFLAG); 
		return 0;        
	}  	
	printf("[ERR] HW dma time out mode %x %x\n", mode 
				,nfreg_read(NF_dwINTFLAG));
	return -1;
}

/*
 * get ECC status
 */	
static int check_ecc_status(struct mtd_info *mtd, int sectors)
{
	struct nand_chip *chip = mtd->priv;
	u32 tmp, blank_mark, i;

	/*	C3701 C version IC support new blank page detecttion
		0x58 [23:16] : threshold cont
		0x58 [31:24] : detect value (default is 0)
		0x58 [7:0]	: page 0~7 status, 1: blank / 0: non blank 

		detect method:		
		1. read reg 0x14 [7:0] : page 0~7 ECC status, 1: correctable /0: uncorrectable
		2. if ECC uncorrectable, check 0x58 [7:0]		
	 */
	chip->page_all_ff = 0;
	if (((C3701 == chip_id) && (chip_ver >= 0x01)) ||
		(C3921 == chip_id))
	{
		switch(sectors)
		{
			case 1:
				blank_mark = 0x01;
				break;
			case 2:
				blank_mark = 0x03;
				break;
			case 4:
				blank_mark = 0x0F;
				break;
			case 8:
				blank_mark = 0xFF;
				break;
		}

		//check all page is uncorrect and blank detect
		tmp = nfreg_read(NF_wECCCURSEC) & blank_mark;
		if (!tmp)
		{
			tmp = nfreg_read(NF_dwDETECTBLANK) & blank_mark;
			if (tmp == blank_mark) 
			{
				chip->page_all_ff = data_all_ff = 1;
				//probe_info("data_all_ff !!!\n");
				return 0;
			}
		}
	}
	else
	{	
		if (NF_ALL_FF_FLAG == (nfreg_read(NF_bECCCTRL) & NF_ALL_FF_FLAG))  
		{
			chip->page_all_ff = data_all_ff = 1;
			return 0;
		}
	}

	data_all_ff = 0;	
	tmp = nfreg_read(NF_wECCCURSEC);
	i = 0;
	while(sectors)
	{
		if (!(tmp & 0x01))
		{
			mtd->ecc_stats.failed++;
			//probe_info("<%s> ecc error %x\n", __FUNCTION__, sectors);
			printf("[ERR] <%s> ecc error %x\n", __FUNCTION__, i);
		}
		tmp = tmp >> 1;
		sectors--;
		i++;
	}

	if (nfreg_read(NF_wECCERROCR) & 0xFF)
	{
		tmp = nfreg_read(NF_bECCSTS) & 0x3f;
		if (((C3701 == chip_id) && (chip_ver >= 0x01)) ||
			 (C3921 == chip_id))
		{
			if (bch_mode == NF_BCH_16B_MODE)		/* 16bit ecc */
			{
				if (tmp > 13)	
				{
					//mtd->ecc_stats.corrected += tmp;
					printk("too many correctable ecc 0x%x\n", tmp);
					return tmp;
				}
			}
			else if(bch_mode == NF_BCH_24B_MODE)	/* 24bit ecc */
			{
				if (tmp > 20)
				{
					//mtd->ecc_stats.corrected += tmp;
					printk("too many correctable ecc 0x%x\n", tmp);
					return tmp;
				}
			}
			else if(bch_mode == NF_BCH_40B_MODE)	/* 40bit ecc */
			{
				if (tmp > 36)
				{
					//mtd->ecc_stats.corrected += tmp;
					printk("too many correctable ecc 0x%x\n", tmp);
					return tmp;
				}
			}
			else if(bch_mode == NF_BCH_48B_MODE)	/* 48bit ecc */
			{
				if (tmp > 44)
				{
					//mtd->ecc_stats.corrected += tmp;
					printk("too many correctable ecc 0x%x\n", tmp);
					return tmp;
				}
			}
			else if(bch_mode == NF_BCH_60B_MODE)	/* 60bit ecc */
			{
				if (tmp > 56)
				{
					//mtd->ecc_stats.corrected += tmp;
					printk("too many correctable ecc 0x%x\n", tmp);
					return tmp;
				}
			}
		}
		else
		{
			if (bch_mode == 0x00)		/* 16bit ecc */
			{
				if (tmp > 13)	
				{
					//mtd->ecc_stats.corrected += tmp;
					probe_info("too many correctable ecc 0x%x\n", tmp);
					return tmp;
				}
			}
			else if(bch_mode == 0x40)	/* 24 bit ecc */
			{
				if (tmp > 20)
				{
					//mtd->ecc_stats.corrected += tmp;
					printk("too many correctable ecc 0x%x\n", tmp);
					return tmp;
				}
			}
		}
	}
	return 0;
}

/*
 * 
 */
static void ali_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{    
#ifdef CONFIG_MTD_NAND_ALI_M39XX
    if (g_bool_palert)
        return;
#endif
	
	if ((ctrl & NAND_NCE) != NAND_NCE)    
	{     		
		nfreg_write(NF_CEJ, NF_bCTRL);        		
		return;    
	}		
	
	if (ctrl & NAND_CTRL_CHANGE)
	{
		if ((ctrl & NAND_CTRL_CLE) == NAND_CTRL_CLE)
		{
			nfreg_write(NF_CLE, NF_bCTRL);
		}
		else if ((ctrl & NAND_CTRL_ALE) == NAND_CTRL_ALE)
		{
			nfreg_write(NF_ALE, NF_bCTRL);		
		}
		else
		{
			nfreg_write(0, NF_bCTRL);
		}
	}
	
	if (NAND_CMD_NONE != cmd)
	{
		nfreg_write((u8) cmd, NF_bPIODATA);
	}
}

#if 1 //for 16k page and read retry
/* 
 *ECC will be calculated automatically, and errors will be detected in
 * waitfunc.
 */ 
int ali_nand_read_page_hwecc(struct mtd_info *mtd, 
				struct nand_chip *chip,	u8 * buf, int page)
{
    u32 tmp0, tmp1;
    int i;
	uint32_t dma_transfer_len, max_dma_transfer_len = 8192;
	uint8_t read_retry_option=0;
	uint32_t ecc_err_cnt=0;
	uint32_t ecc_stats_corrected=0;

	if(mtd->writesize > max_dma_transfer_len)	// for 16kpage
	{
		dma_transfer_len = max_dma_transfer_len;
	}
	else
	{
		dma_transfer_len = mtd->writesize;
	}

	ecc_err_cnt = mtd->ecc_stats.failed;
	do
	{
		if(read_retry_option)
		{
			chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page);
		}
		chip->waitfunc(mtd, chip);
		chip->cmd_ctrl(mtd, NAND_CMD_READ0,
					NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE); 
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);	
		ecc_stats_corrected = 0;
		for(i=0; i<mtd->writesize; i+=dma_transfer_len) // for 16kpage
		{
			/*
			 * init DMA
			 */
			init_hw_dma();

			/*
			 * set_dma_length, address
			 */
			if(chip_id == C3921)
			{// for arm platform
//				nfreg_write((uint32_t)chip_hw_dma_buf & ~0x80000000, NF_dwDMAADDR);
				nfreg_write((uint32_t)(buf+i) & ~0x80000000, NF_dwDMAADDR);
			}
			else
			{// for mips platform
				nfreg_write((uint32_t)(buf+i) & ~0xE0000000, NF_dwDMAADDR);	
			}

			tmp0 = 0x80;
			tmp0 |= (dma_transfer_len >> 10);
			nfreg_write((u8)tmp0, NF_bDMALEN);

			/*
			 * set HW DMA start (Nand to Ali chip)
			 */	
			nfreg_write(NF_DMA_IMB_EN | NF_DMA_EN, NF_bDMACTRL);

			/*
			 * wait DMA finish
			 */	
			if (wait_dma_finish(HW_DMA_READ))
			{
				printf("[ERR] %s read page %d error\n", __FUNCTION__, page);
				return -1;
			}
			invalidate_dcache_range((unsigned long)(buf+i), (unsigned long)((buf+i) + dma_transfer_len));

			/*
			* check ecc status
			*/
			ecc_stats_corrected = check_ecc_status(mtd, dma_transfer_len >> 10);
			if(ecc_stats_corrected)
			{
				printf("[WRN] page #%d ecc status corrected 0x%x \n", page, ecc_stats_corrected);
			}

			/*
			* copy data to buf
			*/
			if (buf != NULL)
			{
#ifdef SUPPORT_CRYPTO
				ptt_cyp_mask = check_crypto_mask(page);
				/* do decrypto, if needed */
				if (ptt_cyp_mask && !data_all_ff)
				{
#ifdef DEBUG_CRYPTO
					show_crypto_status(1);
#endif
					NAND_Crypto(DECRYPT, chip_hw_dma_buf, crypto_buf, dma_transfer_len);
					memcpy((u8 *)(buf+i), (u8 *)crypto_buf, dma_transfer_len);
				}
				else
				{
					memcpy((u8 *)(buf+i), (u8 *)chip_hw_dma_buf, dma_transfer_len);
				}
#else
//a				memcpy((u8 *)(buf+i), (u8 *)(chip_hw_dma_buf), dma_transfer_len);
#endif

			}

		}
		tmp0 = nfreg_read(NF_dwREADREDU0);
		tmp1 = nfreg_read(NF_dwREADREDU1);

		/*
		 * chip->oob always after dma buf + page lebgth
		 */
		*(u32 *) &chip->oob_poi[0] = tmp0;
		*(u32 *) &chip->oob_poi[4] = tmp1;

#ifdef DUMP_READ
		if (buf == NULL)
		{
			printk("read OOB");	
		}
		else
		{
			printk("read data");	
		}
		for (i=0;i<16;i++)
		{
			printk(" %02x", chip_hw_dma_buf[i]);
		}

		printk("\nRedundant %x %x\n", *(u32 *) &chip->oob_poi[0], *(u32 *) &chip->oob_poi[4]);		
#endif

//arthurretry error test
//		if( (read_retry_option == 0) ||
//			(read_retry_option == 1) ||
//			(read_retry_option == 2) ||
//			(read_retry_option == 3) ||
//			(read_retry_option == 4) ||
//			(read_retry_option == 5) ||
//			(read_retry_option == 6) ||
//			(read_retry_option == 6))
//			(read_retry_option == 7))
//			mtd->ecc_stats.failed = 8;

		if(mtd->ecc_stats.failed > ecc_err_cnt)//arthur read retry if ecc error happened.
		{
			read_retry_option++;
			if (read_retry_option < g_retry_options_nb)
			{
				mtd->ecc_stats.failed = ecc_err_cnt;
				if(rr_mode == RR_MODE_MICRON)
				{
					nand_set_feature_rr(mtd, read_retry_option);
				}
				else
				{
					hynix_option = (hynix_option + 1) & 0x07;
					nand_set_feature_rr(mtd, hynix_option);
				}
			}
		}
		else
		{
			break;
		}
	}while(read_retry_option < g_retry_options_nb);

	if(read_retry_option && (rr_mode == RR_MODE_MICRON))
	{
		nand_set_feature_rr(mtd, 0);
	}
	
	if(mtd->ecc_stats.failed > ecc_err_cnt)
	{
		printf("\n[ERR] %s page = 0x%x \n", __FUNCTION__, page);
	}
	
	if(ecc_stats_corrected)
	{
		printk("[WRN] page #%d ecc status corrected 0x%x \n", page, ecc_stats_corrected);
		block_rewrite_tbl[page*mtd->writesize/mtd->erasesize] = 0xFF;
	}

	return 0;
}
#else
/* 
 *ECC will be calculated automatically, and errors will be detected in
 * waitfunc.
 */ 
int ali_nand_read_page_hwecc(struct mtd_info *mtd, 
				struct nand_chip *chip,	u8 * buf, int page)
{    
    u32 tmp0, tmp1;
//    int i;

	chip->waitfunc(mtd, chip);           
	chip->cmd_ctrl(mtd, NAND_CMD_READ0, 
				NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE); 
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);	

#if 0
//	printf("\n\r");
		udelay(50);
	for (i=0; i<mtd->writesize; i++)
	{
		buf[i] = readb(chip->IO_ADDR_R);
//		printf(" %02x", buf[i]);
		udelay(50);
	}
//	printf("\n\r");

#else
	/*
	* init DMA
	*/
	init_hw_dma();  		
	
	/*
	* set_dma_length, address
	*/
	if(chip_id == C3921)
	{// for arm platform
		nfreg_write((u32)chip_hw_dma_buf & ~0x80000000, NF_dwDMAADDR);
	}
	else
	{// for mips platform
		nfreg_write((u32)chip_hw_dma_buf & ~0xF0000000, NF_dwDMAADDR);	
	}
	
	tmp0 = 0x80;    
	tmp0 |= (mtd->writesize >> 10); 
	nfreg_write((u8)tmp0, NF_bDMALEN);	

	/*
	* set HW DMA start (Nand to Ali chip)
	*/	
	nfreg_write(NF_DMA_IMB_EN | NF_DMA_EN, NF_bDMACTRL); 	

	/*
	* wait DMA finish
	*/	
	if (wait_dma_finish(HW_DMA_READ))
	{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 28))
		DEBUG(MTD_DEBUG_LEVEL0, "read page %x error\n", page);
#endif
		return -1;
	}

	/*
	* check ecc status
	*/	
	check_ecc_status(mtd, mtd->writesize >> 10);	

	/* 
	* copy data to buf 
	*/	
	if (buf != NULL)   
	{
#ifdef SUPPORT_CRYPTO	
		ptt_cyp_mask = check_crypto_mask(page);		
		/* do decrypto, if needed */
		if (ptt_cyp_mask && !data_all_ff)
		{	
#ifdef DEBUG_CRYPTO
			show_crypto_status(1);			
#endif
			NAND_Crypto(DECRYPT, chip_hw_dma_buf, crypto_buf, mtd->writesize);				
			memcpy((u8 *)buf, (u8 *)crypto_buf, mtd->writesize);  
		}	
		else
		{
			memcpy((u8 *)buf, (u8 *)chip_hw_dma_buf, mtd->writesize);    
		}		      
#else	
		memcpy((u8 *)buf, (u8 *)((u32)chip_hw_dma_buf), mtd->writesize);
//debug
//	printf("\n\r");
//	for (i=0; i<mtd->writesize; i++)
//	{
//		printf(" %02x", buf[i]);
//	}
//	printf("\n\r");

#endif
		
	}	
	
	tmp0 = nfreg_read(NF_dwREADREDU0);    
	tmp1 = nfreg_read(NF_dwREADREDU1);    
	
	/*
	* chip->oob always after dma buf + page lebgth
	*/	
	*(u32 *) &chip->oob_poi[0] = tmp0;
	*(u32 *) &chip->oob_poi[4] = tmp1;
	
    #ifdef DUMP_READ
	if (buf == NULL)
	{
		printk("read OOB");	
	}
	else
	{
		printk("read data");	
	}
	for (i=0;i<16;i++)
	{
		printk(" %02x", chip_hw_dma_buf[i]);
	}
	
	printk("\nRedundant %x %x\n", *(u32 *) &chip->oob_poi[0], *(u32 *) &chip->oob_poi[4]);		
    #endif	
#endif	
	return 0;
}
#endif


static int ali_nand_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	return 0;
}

static int ali_nand_read_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
			     int page, int sndcmd)
{
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
	
//#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 28))
	sndcmd = chip->ecc.read_page(mtd, chip, NULL, page);
//#else
//	sndcmd = chip->ecc.read_page(mtd, chip, NULL);
//#endif
	return sndcmd;
}

static void ali_nand_write_page_hwecc(struct mtd_info *mtd,
				struct nand_chip *chip,
				const uint8_t *buf)
{
    u32 tmp0, tmp1;
	/* 
	 * init hw dma 
	 */
	init_hw_dma();	
	if(chip_id == C3921)
	{// for arm platform
		nfreg_write((uint32_t)chip_hw_dma_buf & ~0x80000000, NF_dwDMAADDR);
	}
	else
	{// for mips platform
		nfreg_write((uint32_t)chip_hw_dma_buf & ~0xE0000000, NF_dwDMAADDR);
	}
	
	
	tmp0 = *(u32 *) &chip->oob_poi[0];
	tmp1 = *(u32 *) &chip->oob_poi[4];	
	
	nfreg_write(tmp0, NF_dwWRITEREDU0);    
	nfreg_write(tmp1, NF_dwWRITEREDU1); 
		
	if (buf != NULL) 
	{	
	    #ifdef SUPPORT_CRYPTO 	
		/* do encrypto, if needed */
		if (ptt_cyp_mask)
		{
		    #ifdef DEBUG_CRYPTO
			show_crypto_status(2);				
		    #endif
			memcpy((u8 *)crypto_buf, (u8 *)buf, mtd->writesize); 			
			NAND_Crypto(ENCRYPT, crypto_buf, chip_hw_dma_buf, mtd->writesize);
			
		    #ifdef DEBUG_CRYPTO
		    	dump_crypto_data(struct mtd_info *mtd);			
		    #endif			
		}	
		else	
		{					
			memcpy((u8 *)chip_hw_dma_buf, (u8 *)buf, mtd->writesize);   
		}			      
	    #else	    
			memcpy((u8 *)chip_hw_dma_buf, (u8 *)buf, mtd->writesize);   
	    #endif
	}		
	else
	{
		memset((u8 *)chip_hw_dma_buf, 0xFF, mtd->writesize);		
	}
	
    #ifdef DUMP_WRITE
	if (buf == NULL)
	{
		printk("write OOB ");
	}
	else	
	{
		printk("write data");
	}
	
	for (i=0;i<16;i++)	
	{
		printk(" %x", chip_hw_dma_buf[i]);
	}
	
	printk("\nRedundant %x %x\n", *(u32 *) &chip->oob_poi[0], *(u32 *) &chip->oob_poi[4]);			
    #endif
			
	/*
	* set_dma_length
	*/	
	tmp0 = 0x80;    
	tmp0 |= mtd->writesize >> 10;        
	nfreg_write((u8)tmp0, NF_bDMALEN);
	/*
	* set DMA start (Ali chip to Nand)
	*/	
	nfreg_write(NF_DMA_IMB_EN | NF_DMA_OUT | NF_DMA_EN, NF_bDMACTRL);
	
	/*
	* wait DMA finish
	*/
	wait_dma_finish(HW_DMA_WRITE);
}

static int ali_nand_write_oob_std(struct mtd_info *mtd, struct nand_chip *chip, 
		int page)
{
	int status = 0;	
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0, page);
	chip->ecc.write_page(mtd, chip, NULL);	
	
	/* Send command to program the OOB data */
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);
		
	return status & NAND_STATUS_FAIL ? -EIO : 0;
}


/**
 * nand_write_page - [REPLACEABLE] write one page
 * @mtd:	MTD device structure
 * @chip:	NAND chip descriptor
 * @buf:	the data to write
 * @page:	page number to write
 * @cached:	cached programming
 * @raw:	use _raw version of write_page
 */
static int ali_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			   const uint8_t *buf, int page, int cached, int raw)
{

	// ubifs add space_fixup: fixup free space on first mount
	int status;
#if 0 
	int i;
	//check if data all FF, skip write for UBI FS
    if (0xFFFFFFFF == (*(u32 *) &chip->oob_poi[0]))
    {
    	for (i=0; i<mtd->writesize; i++)
		{
			if (buf[i] != 0xff)	
			break;
		}
			
		if (i == mtd->writesize) 
		{
			probe_info("All FF, skip write\n");
			return 0;	
		}
	}
#endif
    #ifdef SUPPORT_CRYPTO	
	ptt_cyp_mask = check_crypto_mask(page);		    
    #endif
	
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);

	if (unlikely(raw))
	{
		chip->ecc.write_page_raw(mtd, chip, buf);
	}
	else
	{
		chip->ecc.write_page(mtd, chip, buf);
	}

	/*
	 * Cached progamming disabled for now, Not sure if its worth the
	 * trouble. The speed gain is not very impressive. (2.3->2.6Mib/s)
	 */
	cached = 0;

//	if (!cached || !(chip->options & NAND_CACHEPRG)) {

		chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
		/*
		 * See if operation failed and additional status checks are
		 * available
		 */
		if ((status & NAND_STATUS_FAIL) && (chip->errstat))
		{
			status = chip->errstat(mtd, chip, FL_WRITING, status,
					       page);
		}

		if (status & NAND_STATUS_FAIL)
		{
			printf ("[ERR] %s line%d page %d\n", __FUNCTION__, __LINE__, page);
			return -EIO;
		}
//	} else {
//		chip->cmdfunc(mtd, NAND_CMD_CACHEDPROG, -1, -1);
//		status = chip->waitfunc(mtd, chip);
//	}
    #ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/* Send command to read back the data */
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

	if (chip->verify_buf(mtd, buf, mtd->writesize))
	{
		return -EIO;
	}
    #endif

	return 0;
}



/**
 * nand_default_bbt - [NAND Interface] Select a default bad block table for the device
 * @mtd:	MTD device structure
 *
 * This function selects the default bad block table
 * support for the device and calls the nand_scan_bbt function
 *
*/
static int ali_nand_default_bbt(struct mtd_info *mtd)
{
	struct nand_chip *this = mtd->priv;

	this->bbt_td = &ali_bbt_main_descr;
	this->bbt_md = &ali_bbt_mirror_descr;
	if (!this->badblock_pattern) 
	{
		this->badblock_pattern = &largepage_flashbased;
	}

	return nand_scan_bbt(mtd, this->badblock_pattern);
}


static void ali_nand_select_chip(struct mtd_info *mtd, int chips)
{
	switch (chips)
	{
		case 0:
			ali_nand_pinmux_set(__LINE__);
			nfreg_write(~NF_CEJ, NF_bCTRL);
			enable_cs(0);
			break;
		case 1:
			nfreg_write(NF_CEJ, NF_bCTRL);
			enable_cs(1);
			ali_nand_pinmux_release(__LINE__);
			break;
		case -1:
		case 2:
		case 3:
		default:
			nfreg_write(NF_CEJ, NF_bCTRL);
			ali_nand_pinmux_release(__LINE__);
			break;						
	}
}

// Identifying initial invalid block:
// case 1: check col 0 and first byte oob of 1st and last pages
// case 2: first byte oob of 1st and 2nd pages
// case 3: first byte oob of 1st.
int ali_nand_block_bad(struct mtd_info *mtd, loff_t ofs)
{
	int page, res = 0;
	struct nand_chip *chip = mtd->priv;
	
//	ALI_NAND_PRINTF("ali_nand_block_bad entry \n");
	page = (int)(ofs >> chip->page_shift);		
	//nand_get_device(chip, mtd, FL_READING);
	/* Select the NAND device */
	ali_nand_select_chip(mtd, 0);
	// only verify 1st 2nd and last page
	{
		chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
		chip->waitfunc(mtd, chip);
		chip->cmd_ctrl(mtd, NAND_CMD_READ0,	NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE); 
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		if (chip->read_byte(mtd) != 0xff)
		{
			res = 1;
			return  res;
		}		

		chip->cmdfunc(mtd, NAND_CMD_READ0, mtd->writesize, page);
		chip->waitfunc(mtd, chip);
		chip->cmd_ctrl(mtd, NAND_CMD_READ0,	NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE); 
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		if (chip->read_byte(mtd) != 0xff)
		{
			res = 1;
			return res;
		}

		chip->cmdfunc(mtd, NAND_CMD_READ0, mtd->writesize, page+1);
		chip->waitfunc(mtd, chip);
		chip->cmd_ctrl(mtd, NAND_CMD_READ0,	NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE); 
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		if (chip->read_byte(mtd) != 0xff)
		{
			res = 1;
			return res;
		}
		
		page += (mtd->erasesize / mtd->writesize)-1;
		
		chip->badblockpos = 0; 
		chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
		chip->waitfunc(mtd, chip);
		chip->cmd_ctrl(mtd, NAND_CMD_READ0,	NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE); 
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		if (chip->read_byte(mtd) != 0xff)
		{
			res = 1;
			return  res;
		}

		chip->cmdfunc(mtd, NAND_CMD_READOOB, mtd->writesize, page);
		chip->waitfunc(mtd, chip);
		chip->cmd_ctrl(mtd, NAND_CMD_READ0,	NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE); 
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		if (chip->read_byte(mtd) != 0xff)
		{
			res = 1;
			return res;
		}
	}
	ali_nand_select_chip(mtd, -1);
	//nand_release_device(mtd);

	return res;
}

/**
 * unknown ecc type, use PIO read byte 0~11 for judgement 
 *
 **/
static int get_ecc_type(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	int page = 0;
	u8 buf[16], i;
	u32 err_count_bit = 0;
	unsigned long  timeo;

	ali_nand_select_chip(mtd, 0);

pio_read:		
	chip->cmd_ctrl(mtd, NAND_CMD_RESET,	NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);            
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);	
	chip->cmd_ctrl(mtd, NAND_CMD_STATUS,NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);            
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);
	
	timeo = 0x1000;//jiffies;	
//arthurrm	timeo += (HZ * 50) / 1000;	
//arthurrm	while (time_before(jiffies, timeo))
	while(timeo--)
	{
		i = readb(chip->IO_ADDR_R);
		if (i & NAND_STATUS_READY)
		{
			goto rst_ready;
		}
	}

	
	printf("Exit %s pio_read error \n\r", __FUNCTION__);
	ali_nand_select_chip(mtd, -1);
	return -1;
	
rst_ready:	
	/* read page 0,256,512,768 */	
	chip->cmd_ctrl(mtd, NAND_CMD_READ0, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, 0x0,			NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, 0x0, 			NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, (u8) page, 		NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, (u8)(page>>8),  NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, (u8)(page>>16), NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);
	
	chip->cmd_ctrl(mtd, NAND_CMD_READSTART, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);            
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);
	
	/* wait status ready */
	chip->cmd_ctrl(mtd, NAND_CMD_STATUS, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);            
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);
	
	timeo = 0x100;//jiffies;	
//arthurrm	timeo += (HZ * 50) / 1000;	
//arthurrm	while (time_before(jiffies, timeo))
	while(timeo--)
	{
		i = readb(chip->IO_ADDR_R);
		if (i & NAND_STATUS_READY)
		{
			goto rd_start;
		}
	}
	printf("Exit %s rst_ready error \n\r", __FUNCTION__);
	ali_nand_select_chip(mtd, -1);
	return -1;	

rd_start:	
	/* start read */
	chip->cmd_ctrl(mtd, NAND_CMD_READ0, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);	
	
	/* 16 bytes */
//	probe_info("\n\r");
	for (i=0; i<16; i++)
	{
		buf[i] = readb(chip->IO_ADDR_R);
//		probe_info(" %2x", buf[i]);
	}
//	probe_info("\n\r");
	
	/* check pattern &&  count bit0~96 "1" */
	if ((buf[14] == 0x55) && (buf[15] == 0xAA)) 
	{
		err_count_bit = 0;
		for (i=0; i<8*12; i ++)
		{
			if ( (buf[i/8] & (1 << (i%8))) != (ECC_pattern_16b[i/8] & (1 << (i%8))) )
			{
				err_count_bit++;
			}
		}
		if(err_count_bit < 8)
		{
			if (chip_id == C3921)
			{
				bch_mode = NF_BCH_16B_MODE;
			}
			else
			{
				bch_mode = 0x00;	// 16-bits ECC
			}
			probe_info("NF_BCH_16B_MODE \n\r");
			ali_nand_select_chip(mtd, -1);
			return 0;
		}
		err_count_bit = 0;
		for (i=0; i<8*12; i ++)
		{
			if ( (buf[i/8] & (1 << (i%8))) != (ECC_pattern_24b[i/8] & (1 << (i%8))) )
			{
				err_count_bit++;
			}
		}
		if(err_count_bit < 8)
		{
			if (chip_id == C3921)
			{
				bch_mode = NF_BCH_24B_MODE;
			}
			else
			{
				bch_mode = 0x40;	// 24-bits ECC
			}
			probe_info("NF_BCH_24B_MODE \n\r");
			ali_nand_select_chip(mtd, -1);
			return 0;
		}
		err_count_bit = 0;
		for (i=0; i<(8*12); i ++)
		{
			if ( (buf[i/8] & (1 << (i%8))) != (ECC_pattern_40b[i/8] & (1 << (i%8))) )
			{
				err_count_bit++;
			}
		}
		if(err_count_bit < 8)
		{
			bch_mode = NF_BCH_40B_MODE;
			probe_info("NF_BCH_40B_MODE 2\n\r");
			ali_nand_select_chip(mtd, -1);
			return 0;
		}
		err_count_bit = 0;
		for (i=0; i<(8*12); i ++)
		{
			if ( (buf[i/8] & (1 << (i%8))) != (ECC_pattern_48b[i/8] & (1 << (i%8))) )
			{
				err_count_bit++;
			}
		}
		if(err_count_bit < 8)
		{
			bch_mode = NF_BCH_48B_MODE;
			probe_info("NF_BCH_48B_MODE \n\r");
			ali_nand_select_chip(mtd, -1);
			return 0;
		}
		err_count_bit = 0;
		for (i=0; i<8*12; i ++)
		{
			if ( (buf[i/8] & (1 << (i%8))) != (ECC_pattern_60b[i/8] & (1 << (i%8))) )
			{
				err_count_bit++;
			}
		}
		if(err_count_bit < 8)
		{
			bch_mode = NF_BCH_60B_MODE;
			probe_info("NF_BCH_60B_MODE \n\r");
			ali_nand_select_chip(mtd, -1);
			return 0;
		}
		//NF_ECC_1B useless ;
		ali_nand_select_chip(mtd, -1);
		return -1;
	}
	else
	{
		page += 256;
		if (page >= 1024)
		{
			ali_nand_select_chip(mtd, -1);
			return -1;
		}
		goto pio_read;	
	}	
}

static void ali_nand_identify(struct mtd_info *mtd, struct nand_chip *chip)
{
	u8 nand_id[10], i;
	/* Select the device */
	chip->select_chip(mtd, 0);
	
//printf("Entry %s \n\r", __FUNCTION__);

	chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
	/* Send the command for reading device ID */
	chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	for (i=0; i<10; i++)
	{
		nand_id[i] = chip->read_byte(mtd);
	}


	if ((nand_id[0] == 0x2C) && (nand_id[1] == 0x44) &&
		(nand_id[2] == 0x44) && (nand_id[3] == 0x4B) && (nand_id[4] == 0xA9))//2C44444BA9
	{
		printk("RR patch for MT29F32G08CBADA\n");
		g_retry_options_nb = 8;
		rr_mode = RR_MODE_MICRON;
	}
	else if ((nand_id[0] == 0x2C) && (nand_id[1] == 0x64) && 
			 (nand_id[2] == 0x44) && (nand_id[3] == 0x4B) && (nand_id[4] == 0xA9))//2C64444BA9
	{
		printk("RR patch for MICRON 29F64G08CBABA\n");
		g_retry_options_nb = 8;
		rr_mode = RR_MODE_MICRON;
	}
	else if ((nand_id[0] == 0xAD) && (nand_id[1]== 0xDE) &&
			 (nand_id[2] == 0x94) && (nand_id[3] == 0xDA) && (nand_id[4] == 0x74) && (nand_id[5] == 0xC4))
	{
		printk("RR patch for HYNIX H27UCG8T2ATR \n"); // HYNIX H27UCG8T2ATR-BC(ADDE94DA74C4) F20 64Gb MLC A-die
		g_retry_options_nb = 8;
		rr_mode = RR_MODE_HYNIX_20nm_A;
	}
	else if ((nand_id[0] == 0xAD) && (nand_id[1] == 0xDE) &&
			 (nand_id[2] == 0x94) && (nand_id[3] == 0xEB) && (nand_id[4] == 0x74) && (nand_id[5] == 0x44))
	{
		printk("RR patch for HYNIX H27UBG8T2BTR \n"); // HYNIX H27UBG8T2BTR-BC(ADDE94EB7444) F20 64Gb MLC B-die
		g_retry_options_nb = 8;
		rr_mode = RR_MODE_HYNIX_20nm_B;
	}
	else if ((nand_id[0] == 0xAD) && (nand_id[1] == 0xD7) &&
			 (nand_id[2] == 0x94) && (nand_id[3] == 0x91) && (nand_id[4] == 0x60) && (nand_id[5] == 0x44))
	{
		printk("RR patch for HYNIX H27UBG8T2CTR-BC \n"); // HYNIX H27UBG8T2CTR-BC(ADD794916044) F20 32Gb MLC
		g_retry_options_nb = 8;
		rr_mode = RR_MODE_HYNIX_20nm_C;
	}
	else if ((nand_id[0] == 0xAD) && (nand_id[1] == 0xDE) &&
			 (nand_id[2] == 0x94) && (nand_id[3] == 0x97) && (nand_id[4] == 0x44) && (nand_id[5] == 0x45))
	{
		printk("RR patch for HYNIX H27UCG8T2DTR \n"); // HYNIX H27UCG8T2DTR(ADDE94974445) 1xnm 1st 64Gb MLC D-die
		g_retry_options_nb = 8;
		rr_mode = RR_MODE_HYNIX_1xnm;
	}
	else if ((nand_id[0] == 0xAD) && (nand_id[1] == 0xDE) &&
			 (nand_id[2] == 0x94) && (nand_id[3] == 0xA7) && (nand_id[4] == 0x42) && (nand_id[5] == 0x48))
	{
		printk("RR patch for HYNIX H27UCG8T2ETR-BC \n"); // HYNIX H27UCG8T2ETR(ADDE94A74248) 1xnm 2nd 64Gb MLC E-die
		g_retry_options_nb = 8;
		rr_mode = RR_MODE_HYNIX_1xnm;
	}
//printf("Exit %s \n\r", __FUNCTION__);
	chip->select_chip(mtd, -1);
}

//arthurc3921 add
int get_pmi_c3921(struct mtd_info *mtd, struct nand_chip *chip)
{
	ali_nand_identify(mtd, chip);

	/*
	* update mtd_info
	*/
	mtd->oobsize = 8;
	mtd->oobavail = 8;
	mtd->subpage_sft = 0;
	
	probe_info("mtd->erasesize 0x%x\n", mtd->erasesize);
	probe_info("mtd->writesize 0x%x\n", mtd->writesize);
	probe_info("mtd->size 0x%012llx\n", mtd->size);	

	/*
	* update nand_chip
	*/
	// by nand_get_flash_type function.

	return 0;
}


int get_pmi_s3921(struct mtd_info *mtd, struct nand_chip *chip)
{
	struct mtd_oob_ops ops;
	loff_t offs;
	int blocks, len;
	u8 *buf, *buf_tmp;
	u8 *pattern;
	u8 i=0;	PART_INFO *part;
	
	blocks = ali_PMI_descr.maxblocks;
	pattern = ali_PMI_descr.pattern;
	len = mtd->writesize + mtd->oobsize;
	buf_tmp = (u8 *)kmalloc(len+0x20, GFP_KERNEL);
	if (!buf_tmp) {
		printf("[ERR] %s NO Memory\n", __FUNCTION__);
		return -1;
	}
	buf = (u8 *)((u32)(buf_tmp + 0x1F) & (u32)(~0x1F));
	
	ali_nand_identify(mtd, chip);
	
	while(i < blocks)
	{
		probe_info("get_pmi %d\n\r", i);
		ops.mode = MTD_OOB_PLACE;
		// LINUX_VERSION_CODE 2.6.11.8 ~ 2.6.31.13
		ops.ooboffs = 0;
		ops.ooblen = mtd->oobsize;
		ops.oobbuf = buf + mtd->writesize;
		ops.datbuf = buf;
		ops.len = mtd->writesize;
		
		offs = ((loff_t) ali_PMI_descr.pos[i]) << chip->page_shift;
		
		if (mtd->read_oob(mtd, offs, &ops))
		{
			kfree(buf_tmp);
			return -1;
		}
		
		if (!ali_check_pattern(buf, PMI_PATTERN_POS, PMI_PATTERN_LEN, pattern))
		{
			printk(KERN_ERR "PMI pattern found @ page %x\n", ali_PMI_descr.pos[i]);
			break;
		}
		i++;
	}
	
	if (i == blocks)
	{
		printf("Can't find PMI \n");
		kfree(buf_tmp);
		return -1;
	}
	
	memset((u8 *)pmi_buf, 0x0, MAX_PMI_SIZE);
	memcpy((u8 *)pmi_buf, (u8 *)buf, MAX_PMI_SIZE);
	pPMI = (PMI_t *) pmi_buf;
	
	/*
	* update mtd_info
	*/
	mtd->oobsize = 8;
	mtd->oobavail = 8;
	mtd->subpage_sft = 0;
	mtd->erasesize = pPMI->Config.wBytesPerPage * pPMI->Config.wPagesPerBlock;
	mtd->writesize = pPMI->Config.wBytesPerPage;
	probe_info("mtd->erasesize 0x%x\n", mtd->erasesize);
	probe_info("mtd->writesize 0x%x\n", mtd->writesize);
 	mtd->size = (uint64_t)pPMI->Config.wBlksPerChip * (uint64_t)pPMI->Config.wBytesPerPage * 
 				(uint64_t)pPMI->Config.wPagesPerBlock * (uint64_t)chip->numchips;
 	probe_info("mtd->size 0x%012llx\n", mtd->size);
	
    /*
	* update nand_chip
	*/
	chip->page_shift = ffs(mtd->writesize) - 1;
	chip->bbt_erase_shift = chip->phys_erase_shift = ffs(mtd->erasesize) - 1;				
	chip->subpagesize = mtd->writesize;
	chip->chipsize = (u64) pPMI->Config.wBlksPerChip * (u64) pPMI->Config.wBytesPerPage * (u64) pPMI->Config.wPagesPerBlock;
	if (chip->chipsize & 0xffffffff)
	{
		chip->chip_shift = ffs((unsigned)chip->chipsize) - 1;
	}
	else
	{
		chip->chip_shift = ffs((unsigned)(chip->chipsize >> 32)) + 32 - 1;
	}
		
	chip->pagemask = (chip->chipsize >> chip->page_shift) - 1;	
	
	/*
	* setup clock
	*/	
	if (pPMI->Config.bReadClock != 0x0)
	{
		nfreg_write(pPMI->Config.bReadClock, NF_bREADCYC);
	}
	if (pPMI->Config.bWriteClock != 0x0)
	{
		nfreg_write(pPMI->Config.bWriteClock, NF_bWRITECYC);
	}
	
	probe_info("read speed setting 0x%x\n", nfreg_read(NF_bREADCYC));
	probe_info("write speed setting 0x%x\n", nfreg_read(NF_bWRITECYC));
	probe_info("bch_mode 0x%x\n", (u32)(pPMI->Config.bECCType));
	
	/*
	* set partition info
	*/
#ifdef HAVE_PART_TBL_PARTITION
	p_PMIPartInfo = parttbl_buf;
	p_PMIPartInfo->part_num = *(u32 *) &pmi_buf[PartCntOff];
	//printf("part_num: pmi_buf[PartCntOff]=0x%08x,&pmi_buf[PartCntOff]=0x%08x\n", pmi_buf[PartCntOff],&pmi_buf[PartCntOff]);

	for (i=0; i<p_PMIPartInfo->part_num; i++)
	{
		part = &(p_PMIPartInfo->parts[i]);
#else
	PartitionInfo.part_num = *(u32 *) &pmi_buf[PartCntOff];
	//printf("part_num: pmi_buf[PartCntOff]=0x%08x,&pmi_buf[PartCntOff]=0x%08x\n", pmi_buf[PartCntOff],&pmi_buf[PartCntOff]);

	for (i=0; i<PartitionInfo.part_num; i++)
	{
		part = &(PartitionInfo.parts[i]);
#endif
		part->start = *(u32 *) &pmi_buf[PartOff + i * 8];
		part->size = *(u32 *) &pmi_buf[PartOff + i * 8 + 4];
		part->size &= 0x3FFFFFFF;
		part->start = part->start <<10;
		part->size = part->size <<10;

		if ((pmi_buf[PartNameOff+i*16] >= 0x21) && (pmi_buf[PartNameOff+i*16] <= 0x7D))
		{
			memcpy((u8 *)part->name, &pmi_buf[PartNameOff+i*16], 16);
			part->name[15] = 0;
		}

	}
	kfree(buf_tmp);
	return 0;
}

int get_pmi(struct mtd_info *mtd, struct nand_chip *chip)
{
	if(((chip_id == 0x3921) && (chip_ver == 1)) || (chip_id == C3503))
	{
		if (get_pmi_c3921(mtd, chip))
		{
			printk(KERN_ERR "Can't find PMI \n");
			return -1;
		}
	}
	else
	{
		if (get_pmi_s3921(mtd, chip))
		{
			printk(KERN_ERR "Can't find PMI \n");
			return -1;
		}
	}
	return 0;
}

static void ali_nand_crypto_inhibit(struct mtd_info *mtd, int inhibit)
{
//	struct nand_chip *chip = mtd->priv;

	u8 tmp8 = 0;

	ALI_NAND_PRINTF("ali_nand_crypto_inhibit entry %d\n", inhibit);
	
	if (inhibit == 1)
	{
//		if (chip->crypto_mode)
		{
			tmp8 = nfreg_read(NF_bEDORBCYC);
			ALI_NAND_PRINTF("ali_nand_crypto_inhibit0 %d\n", tmp8);
			ALI_NAND_PRINTF("ali reg 0x2c a 0x%x\n", *(volatile u8 *) 0x1803202c);
			nfreg_write(tmp8 & ~NF_CRYPTO_EN, NF_bEDORBCYC);
		}
	}
	else
	{
//		if (chip->crypto_mode)
		{
			tmp8 = nfreg_read(NF_bEDORBCYC);
			ALI_NAND_PRINTF("ali_nand_crypto_inhibit1 %d\n", tmp8);
			ALI_NAND_PRINTF("ali reg 0x2c a 0x%x\n", *(volatile u8 *) 0x1803202c);
			nfreg_write(tmp8 | NF_CRYPTO_EN, NF_bEDORBCYC);
		}
	}

}
#if 0// for u-boot burning
int get_nand_id(u8 *id) 
{
	int i = 0;
	unsigned long  timeo = 0x1000;
	
	/* 
	* chip Enable
	*/
	nfreg_write(NF_EN, NF_bMODE);

	/*
	* Clock seting
	*/
	nfreg_write(0x22, NF_bREADCYC);
	nfreg_write(0x22, NF_bWRITECYC);
	
	/* Select the device */
	ali_nand_select_chip(0, 0);
	ali_nand_reg_dump();;
	
	//chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
	ali_nand_cmd_ctrl(0, NAND_CMD_RESET, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	ali_nand_cmd_ctrl(0, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	ali_nand_cmd_ctrl(0, NAND_CMD_STATUS, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);            
	ali_nand_cmd_ctrl(0, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		
	while(timeo--)
	{
		i = nfreg_read8(NF_bPIODATA);
		if (i & NAND_STATUS_READY)
			break;
	}

	if(timeo == 0)	
	{
		printf("Exit %s pio_read error \n\r", __FUNCTION__);
		ali_nand_select_chip(0, -1);
		return -1;
	}
	
	/* Send the command for reading device ID */
	//chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);
	ali_nand_cmd_ctrl(0, NAND_CMD_READID,  NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	ali_nand_cmd_ctrl(0, 0, NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);
	ali_nand_cmd_ctrl(0, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	printk("NAND ID:");
	for (i=0; i<10; i++)
	{
		*(id + i) = nfreg_read8(NF_bPIODATA);
		printk("0x%02X ", *(id + i));
	}
	printk("\n");
	ali_nand_select_chip(0, -1);
	return 0;
}
#endif

/*****************************************************************
    return Enable/Disable security bootrom.
    0x03[1] BOOT_SECURITY_EN 
    return 1, enable
    return 0, disable
*****************************************************************/
u32 sys_ic_bootrom_is_enabled(void)
{
    u32 OTP_value = 0;
    u32 ret = 0;
#ifdef ALI_ARM_STB
    OTP_value = (*(volatile u32 *)(0x18042044));
#else
    OTP_value = (*(volatile u32 *)(0xb8042044));
#endif
	printf("0x18042044: 0x%08x\n", OTP_value);
	
    ret = (OTP_value & 0x2) >> 1;
    return ret;
}

#if 1
int board_nand_init(struct nand_chip *nand)
{
	int err = 0;
	//arthurrm	struct ali_nand_host *host;
	static int chip_nr = 0;
	struct mtd_info *mtd = NULL;
	int maxchips = CONFIG_SYS_NAND_MAX_CHIPS;

	printf("%s\n", ALI_NAND_DRIVER_VERSION);

	if (maxchips < 1)
	{
		maxchips = 1;
	}

	/* Allocate memory for MTD device structure and private data */
	ali_soc_reg = (void __iomem *)ALI_SOC_BASE;
	chip_id = (u16) (ali_soc_read(ali_soc_reg + 0) >> 16);	
	chip_package = (u8) (ali_soc_read(ali_soc_reg + 0) >> 8) & 0x0F;	
	chip_ver = (u8) (ali_soc_read(ali_soc_reg + 0)) & 0xFF;


	/* structures must be linked */
	mtd = &nand_info[chip_nr++];
	mtd->priv = nand;
	mtd->owner = 0;//THIS_MODULE;
	mtd->name = "ali_nand";

	/*
	* Nand flash reset / clk not gated 
	*/
	ali_nand_clk_enable();
	switch(chip_id)
	{
	    case C3603:
	    	ali_PMI_descr.pattern = &pattern_36[0];			
	    	if (chip_package == M3603)
	    	{
	    	    probe_info("ali_nand_probe:BGA package\n");
	    	    ali_soc_write((ali_soc_read(ali_soc_reg + 0xAC) | (1 << 21)),
								ali_soc_reg + 0xAC); 
	    	}
	    	else if (chip_package == M3606)
	    	{
	    	    probe_info("ali_nand_probe:QFP package\n");	
	    	    ali_soc_write((ali_soc_read(ali_soc_reg + 0xAC) | (1 << 22) | (1 << 24)),
								ali_soc_reg + 0xAC); 
	    	    
	    	    ali_soc_write(((ali_soc_read(ali_soc_reg + 0x2E00) & (~0x1F00)) | (0x1200)),
								ali_soc_reg + 0x2E00); 
	    	    
	    	    /* disable GPIO function*/			
	    	    ali_soc_write((ali_soc_read(ali_soc_reg + 0x43C) 
	    	    	& ~((1<<M36xx_QFP_CLE) |(1<<M36xx_QFP_WPJ)))
	    	    	, ali_soc_reg + 0x43C); 	
	    	}
	    	else
	    	{
	    	    printk(KERN_ERR "C36 Package not support Nand Flash\n");	
	    	    goto eres;
	    	}
	    	break;
	    	
	    case C3901:	
	    	ali_PMI_descr.pattern = &pattern_39[0];
	    	/* ip reset */		
	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x84) | (1 << 20)), ali_soc_reg + 0x84); 
	    	udelay(10);
	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x84) & ~(1 << 20)), ali_soc_reg + 0x84); 		
	    	/* fucntion enale */					
	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x74) | 0x40040000), ali_soc_reg + 0x74);
	    	
	    	if (chip_package == 1)
	    	{			
	    	    /* do not control WP pin, kernel handle it.
	    	    	(demo/customer board)
	    	    	WP / NC GPIO[14]  
	    	    	ALE     GPIO[31]
	    	    	WE      GPIO[30]
	    	    	
	    	    	CLE      GPIO2[0]
	    	    	CE1 / WP GPIO2[1] 
	    	    	CE0      GPIO2[2]
	    	    	RE       GPIO2[3]
	    	    	DATA0~7  GPIO2[4,5,6,7,8,9,10,12]
	    	    */	    	    
	    	    #if 0
	    	        ali_soc_write(((ali_soc_read(ali_soc_reg + 0x430) | (1 << 14)) & ~(1 << 31 | 1 << 30)),
									 ali_soc_reg + 0x430);
	    	        ali_soc_write((ali_soc_read(ali_soc_reg + 0x58) | (1 << 14)), ali_soc_reg + 0x58);
	    	        ali_soc_write((ali_soc_read(ali_soc_reg + 0x54) | (1 << 14)), ali_soc_reg + 0x54);
	    	    #endif
	    	    
	    	    /* GPIO2 */
	    	    ali_soc_write((ali_soc_read(ali_soc_reg + 0x434)  & 
	    	    		~(1<<0 | 1<<2 | 1<<3 | 1<<3 | 1<<4 |
						  1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 |
						  1<<10 | 1<<12)), 						
	    	    		ali_soc_reg + 0x434);
	    	    
	    	    			
	    	}
	    	else
	    	{
	    	    /* 
	    	       WP 		GPIO2[18], 
	    	       DATA 6,7	GPIO2[0,1]
	    	       DATA 5~0	GPIO[31,30,29,38,27,26]
	    	       RE		GPIO[25]
	    	       CE0		GPIO[24]
	    	       CE1		GPIO[23]
	    	       CLE		GPIO[22]
	    	       ALE		GPIO[21]
	    	       WE		GPIO[20]				   
	    	      */
	    	    
	    	    #if 0  
	    	    ali_soc_write(((ali_soc_read(ali_soc_reg + 0x434) | (1 << 18)) & ~((1 << 0) | (1 << 1))), ali_soc_reg + 0x434);
	    	    ali_soc_write((ali_soc_read(ali_soc_reg + 0xD8) | (1 << 18)), ali_soc_reg + 0xD8);
	    	    ali_soc_write((ali_soc_read(ali_soc_reg + 0xD4) | (1 << 18)), ali_soc_reg + 0xD4);			
	    	    #endif
	    	    
	    	    /* GPIO */
	    	    ali_soc_write((ali_soc_read(ali_soc_reg + 0x430)  & 
	    	    		~(1<<31 | 1<<30 | 1<<29 | 1<<28 | 1<<27 | 1<<26 |
						  1<<25 | 1<<24 | 1<<23 | 1<<22 | 1<<21 | 1<<20)), 						
	    	    		ali_soc_reg + 0x430);
	    	}
	    	break;
			
	    case C3701:	
	    	ali_PMI_descr.pattern = &pattern_37[0];
	    	/* ip reset */		
	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x84) | (1 << 20)), ali_soc_reg + 0x84); 
	    	udelay(10);
	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x84) & ~(1 << 20)), ali_soc_reg + 0x84); 		
	    	/* fucntion enale */					
	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x74) | 0x40040000), ali_soc_reg + 0x74);
	    	
	    	switch(chip_package)
	    	{	    		
	    	    case 00:/* 376 pin */				
	    	    case 01:/* 256 pin */
			case M3701H://02:/* 256 pin */					/* WP use XGPIO41 */
	    	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x434) | 0x200), ali_soc_reg + 0x434);
	    	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0xD8)  | 0x200), ali_soc_reg + 0xD8);
	    	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0xD4)  | 0x200), ali_soc_reg + 0xD4);			
	    	    	break;
			case M3701C://03:/* QAM 128 pin */				/* enable 128 pin NF select */
	    	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x88) | (1 << 7)), ali_soc_reg + 0x88); 			
	    	    	/* WP use XGPIO5 */
	    	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x430) | 0x20), ali_soc_reg + 0x430);
	    	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x58)  | 0x20), ali_soc_reg + 0x58);
	    	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x54)  | 0x20), ali_soc_reg + 0x54);			
	    	    	break;
	    	    	
			case M3701NMP://04:/* NMP 128 pin */				/* enable 128 pin NF select */
	    	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x88) | (1 << 7)), ali_soc_reg + 0x88); 			
	    	    	/* WP use HW */
	    	    	break;
	    	    default:	
	    	    	break;
	    	}
	    	break;
	    case C3503:	
	    	ali_PMI_descr.pattern = &pattern_35[0];
	    	/* ip reset */		
			// N/A		
	    	/* fucntion enale */					
	    	ali_soc_write((ali_soc_read(ali_soc_reg + 0x74) | 0x40040000), ali_soc_reg + 0x74);
	    	
	    	switch(chip_package)
	    	{	    		
                case 0x00:  //BGA 292 Pin package
                case 0x04:
					ali_soc_write((ali_soc_read(ali_soc_reg + 0x74) | 0x40040000), ali_soc_reg + 0x74); 
					ali_soc_write(ali_soc_read(ali_soc_reg + 0xA4) | (1 << 8), ali_soc_reg + 0xA4);
					break;     
				case 0x01: //256 Pin package
					ali_soc_write((ali_soc_read(ali_soc_reg + 0x74) | 0x41040000), ali_soc_reg + 0x74);        
					//PAD driving
					ali_soc_write((ali_soc_read(ali_soc_reg + 0x410) & ~((1 << 27)|(1 << 26))),
									ali_soc_reg + 0x410);
					ali_soc_write(ali_soc_read(ali_soc_reg + 0xA4) | (1 <<31), ali_soc_reg + 0xA4);
					break;
				default:
					probe_info("[ERR] Not defined!!\n");
					break;
	    	}
			ali_soc_write(ali_soc_read(ali_soc_reg + 0x88) & (~((1 << 8) | (1 << 16) | (1 << 17))),
							ali_soc_reg + 0x88);
			ali_soc_write(ali_soc_read(ali_soc_reg + 0x8c) & (~(1 << 21)), ali_soc_reg + 0x8c);
			
	    	break;
	    case C3811:
	    	ali_PMI_descr.pattern = &pattern_38[0];
	    	ali_soc_write(0x00800200, ali_soc_reg + 0x74);
	    	break;
		case C3821:
			ali_PMI_descr.pattern = &pattern_3821[0];
			/* ip reset */
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x84) | (1 << 20)), ali_soc_reg + 0x84);
			udelay(10);
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x84) & ~(1 << 20)), ali_soc_reg + 0x84);
			/* fucntion enale */					
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x74) | 0x40040000), ali_soc_reg + 0x74);
			/* NFlash Clock Select  */ // reg0x78 0:74MHz 1:54MHz 
			ali_soc_write(((ali_soc_read(ali_soc_reg + 0x7C) & ~(1 << 30))), ali_soc_reg + 0x7C);
		case C3921:
			ali_PMI_descr.pattern = &pattern_3921[0];
			/* ip reset */
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x84) | (1 << 20)), ali_soc_reg + 0x84);
			udelay(10);
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x84) & ~(1 << 20)), ali_soc_reg + 0x84); 		
			/* fucntion enale */					
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x74) | 0x9840000), ali_soc_reg + 0x74);
			ali_soc_write((ali_soc_read(ali_soc_reg + 0x88) | (1 << 3)), ali_soc_reg + 0x88); 
			/* NFlash Clock Select 118MHz,default 54M */
			ali_soc_write(((ali_soc_read(ali_soc_reg + 0x7C) & ~(3 << 18)) | (2 << 18)), ali_soc_reg + 0x7C);
	    	switch(chip_package)
	    	{
	    	    case M3921_QFP256:/* QFP256 */				
	    	    case M3921_BGA445:/* BGA445 */
					// TODO : GPIO pins mux setting base on IC packeage.
					//ali_soc_write((ali_soc_read(ali_soc_reg + 0x434) | 0x200), ali_soc_reg + 0x434);
	    	    	//ali_soc_write((ali_soc_read(ali_soc_reg + 0xD8) | 0x200), ali_soc_reg + 0xD8);
	    	    	//ali_soc_write((ali_soc_read(ali_soc_reg + 0xD4) | 0x200), ali_soc_reg + 0xD4);			
	    	    	break;

	    	    default:
	    	    	break;
	    	}
	    	break;
	    	
	    default:
	    	break;
	}
	
	/*
	* Register
	*/
	switch(chip_id)
	{
	    case C3603:
	    case C3811:
	    	ali_nand_reg = (void __iomem *)0xB803C000;
	    	break;
	    case C3503:
	    case C3901:
	    case C3701:
	    case C3821:
	    case C3921:
	    	ali_nand_reg = (void __iomem *)ALI_NANDREG_BASE;
	    	break;
	    default:
	    	break;
	}

	nand->IO_ADDR_R = (void __iomem *) (ali_nand_reg+NF_bPIODATA);
	nand->IO_ADDR_W = (void __iomem *) (ali_nand_reg+NF_bPIODATA);

	if (0)//(crypto_mode)//arthurcrypto
	{
		nfreg_write(nfreg_read(NF_bEDORBCYC) | NF_CRYPTO_EN, NF_bMODE);
	}
	else
	{
		nfreg_write(nfreg_read(NF_bEDORBCYC) & ~NF_CRYPTO_EN, NF_bMODE);
	}

    /* 
    * buffer1 for DMA access 
    */
	//adds for cache line size align
#ifdef ALI_ARM_STB
	chip_hw_dma_buf = (u8 *)dma_alloc_coherent(8192+0x20, &chip_hw_dma_address);
#else
	chip_hw_dma_buf = (u8 *)malloc(8192+0x20);
#endif
	chip_hw_dma_buf = (u8 *)(((u32)chip_hw_dma_buf + (u32)0x1F) & (~(u32)0x1F));
	if (!chip_hw_dma_buf)
	{
    	err = -ENOMEM;
	   	goto eres;
	}

	probe_info("chip_hw_dma_buf 0x%x\n", (u32) chip_hw_dma_buf);
	//probe_info("chip_hw_dma_address 0x%x\n", (u32) chip_hw_dma_address);
	
	/* 
	* Reference hardware control function 
	*/
	nand->cmd_ctrl  = ali_nand_cmd_ctrl;
	nand->write_page = ali_nand_write_page;
	nand->ecc.read_page = ali_nand_read_page_hwecc;
	nand->ecc.write_page = ali_nand_write_page_hwecc;
	nand->select_chip = ali_nand_select_chip;
	nand->verify_buf = ali_nand_verify_buf;	
	nand->ecc.read_oob = ali_nand_read_oob_std;
	nand->ecc.write_oob = ali_nand_write_oob_std;
	nand->scan_bbt = ali_nand_default_bbt;
	nand->dev_ready = NULL;
	nand->chip_delay = 1;
	
	probe_info("chip_id %x\n", chip_id);
	probe_info("soc base 0x%x\n", (u32) ali_soc_reg);
	probe_info("ali_nand_regbase 0x%x\n", (u32) ali_nand_reg);
		
	/* 
	* chip Enable
	*/
	nfreg_write(NF_EN, NF_bMODE);
	
	/*
	* Clock seting
	*/
	nfreg_write(0x11, NF_bREADCYC);
	nfreg_write(0x11, NF_bWRITECYC);

	/*
	* data layout, 4 bytes OOB per 1024 bytes
	*/
	nfreg_write(0x80, NF_bDMALEN);
	nfreg_write(0, NF_bDMACTRL);
	nfreg_write((u32)chip_hw_dma_buf & ~0x80000000, NF_dwDMAADDR);

	probe_info("scs_nand_info_scan \n");

	//arthurc3921
	if(((chip_id == C3921) && (chip_ver == 1)) || (chip_id == C3503))
	{

		if(sys_ic_bootrom_is_enabled() && (chip_id == C3921))
		{
			printk(KERN_ERR "3921 as bootrom is enablel\n");
			if(scs_nand_info_scan_as() == -1)
			{
				printk(KERN_ERR "scs_nand_info_scan_as fail\n");
				goto escan;
			}
		}
		else
		{
			if(scs_nand_info_scan(BA_SCAN_START) == -1)
			{
				printk(KERN_ERR "scs_nand_info_scan fail\n");
				goto escan;
			}
		}
		
		printk(KERN_ERR "scs_nand_info_scan ok!\n");
		nfreg_write(nf_info.read_timing, NF_bREADCYC);
        nfreg_write(nf_info.write_timing, NF_bWRITECYC);
	}
	else
	{
		/*
		 * Get ECC type, 16/24/40/48/60 bits BCH mode
		 */
		if (get_ecc_type(mtd))
		{
			printk(KERN_ERR "Can't get ECC type\n");		
			goto escan;
		}
		nfreg_write(NF_ECC_EN | NF_ECC_NON_STOP | bch_mode, NF_bECCCTRL); 	
	}
	/*
	* options
	*/  //arthur??
	nand->options = NAND_NO_SUBPAGE_WRITE | NAND_USE_FLASH_BBT;
	nand->ecc.layout = &ali_nand_oob_32;
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.size = 1024;
	nand->ecc.bytes = 28;
	nand->ecc.layout->oobavail = 8;
	nand->bbt_td = &ali_bbt_main_descr;
	nand->bbt_md = &ali_bbt_mirror_descr;


 	/*	C3701 C version IC support new blank page detecttion
		0x58 [23:16] : threshold cont
		0x58 [31:24] : detect value (default is 0)
		0x58 [7:0]	: page 0~7 status, 1: blank / 0: non blank 
		
		initial set:
		1. 0x58 [23:16] = x , threshold value , for 16 bit ECC 15, 24 bit ECC 23					
	*/	
	if (((C3701 == chip_id) && (chip_ver >= 0x01)) ||
		 (C3921 == chip_id) || (C3503 == chip_id))
	{	
		if (bch_mode == NF_BCH_16B_MODE)
		{
			nfreg_write(BLANK_DETECT_16ECC, NF_dwDETECTBLANK);
		}
		else if (bch_mode == NF_BCH_24B_MODE)
		{
			nfreg_write(BLANK_DETECT_24ECC, NF_dwDETECTBLANK);
		}
		else if (bch_mode == NF_BCH_40B_MODE)
		{
			nfreg_write(BLANK_DETECT_40ECC, NF_dwDETECTBLANK);
		}
		else if (bch_mode == NF_BCH_48B_MODE)
		{
			nfreg_write(BLANK_DETECT_48ECC, NF_dwDETECTBLANK);
		}
		else if (bch_mode == NF_BCH_60B_MODE)
		{
			nfreg_write(BLANK_DETECT_60ECC, NF_dwDETECTBLANK);
		}
		else
		{
			printk(KERN_ERR "bch_mode error \n");
			goto escan;
		}
	}

	memset(block_rewrite_tbl, 0, BLOCK_REWRITE_TBL_SIZE);
		
	/* Register the partitions */
    #ifdef CONFIG_MTD_PARTITIONS
   	if (nr_parts > 0)
	{
		add_mtd_partitions(mtd, host->parts, nr_parts);
	}
   	else
    #endif
   	{
//arthur: undo.  It will be processed by u-boot func of u-boot
//arthurrm        probe_info("Registering %s as whole device\n", mtd->name);
//arthurrm        add_mtd_device(mtd);
	}

#ifdef SUPPORT_CRYPTO
    #ifdef DEBUG_CRYPTO
    force_test_partition();	    
    #endif
	
	if (encrypto)
	{
		printk("Encrypto Enable\n");
		info = (int *) crypt_info;
		if (info == NULL)
		{
			printk("crypto info is empty !!!\n");
			encrypto = 0;			
		}
		else
		{
			for (err=0; err<11; err++)	
			{
				printk("Partition[%d] encrypt %x\n", err, info[err]);
			}
		
#ifdef ALI_ARM_STB
			crypto_buf = dma_alloc_coherent(&pdev->dev, 
    					mtd->writesize,
                                	&crypto_addr, GFP_KERNEL);
#else
			crypto_buf = malloc(mtd->writesize);

#endif
        }
		printk("crypto_buf 0x%x\n", (u32) crypto_buf);
	}
	else
	{
		printk("Encrypto disable\n");
	}
	
    #ifdef DEBUG_CRYPTO
	    test_crypto_read_write(struct mtd_info *mtd, struct nand_chip *nand);	    
    #endif
#endif
escan:
eres:
	return err;
}

#else
int ali_nand_init(void) //arthur: is equal to  board_nand_init
{
}
#endif


int Get_NandPageSize(void)
{
    if (NULL != pPMI)
    {
        return pPMI->Config.wBytesPerPage;
    }

    return 0;
}

