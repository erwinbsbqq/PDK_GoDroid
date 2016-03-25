/*
 *  ali_nand.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Info:
 *	Contains standard defines and IDs for NAND flash devices
 *
 * Changelog:
 *	See git changelog.
 */
#ifndef _ALI_NAND_H
#define _ALI_NAND_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------*/
/* tmp modify */
/* ------------------------------------------------------------------------------*/
#define SOC_BASE			0x18000000
#define ALI_NANDREG_BASE	0x18032000
#define ALI_NANDREG_LEN		0x60
#define ALI_NANDREG_SRAM 	host->regs + 0x1000

#define ali_soc_read(reg)		__REG32ALI(reg + SOC_BASE)
#define ali_soc_write(val, reg) __REG32ALI(reg + SOC_BASE) = val


#define probe_info printk
//#define probe_info(args...) do{}while(0)
#define ALI_NAND_ERROR printk
#define ALI_NAND_DEBUG(args...)  do{}while(0)
//#define ALI_NAND_DEBUG  printk

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 2, 0))
	#define MTD_OPS_PLACE_OOB MTD_OOB_PLACE
#endif


#ifdef CONFIG_ALI_AS   // temp for compile
#undef CONFIG_ALI_AS
#endif

extern int add_mtd_device(struct mtd_info *mtd);
extern int del_mtd_device (struct mtd_info *mtd);
extern int add_mtd_partitions(struct mtd_info *master, const struct mtd_partition *parts, int nbparts);

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t

#define ALI_NAND_DRIVER_VERSION "ali_nand_ver_2014_1223_k2"
#define DRIVER_NAME "ali_nand"

#define PMI_PATTERN_POS		12
#define PMI_PATTERN_LEN		4
#define CONFIG_OFS			16
#define PARTITIONS_NUM		256
#define PARTITIONS_OFS		260
#define MAX_PARTITION_NUM	31
#define PARTITIONS_NAME		512	//partition name 512-1023

#define BLANK_DETECT_60ECC (0x37<<16)
#define BLANK_DETECT_48ECC (0x2F<<16)
#define BLANK_DETECT_40ECC (0x27<<16)
#define BLANK_DETECT_24ECC (0x17<<16)
#define BLANK_DETECT_16ECC (0x0F<<16)

#define NAND_PARM_START			0x290
#define NAND_PARM_START_3921B	0x2A0
#define NAND_PARM_OFFSET		0x2D0
#define NAND_RR_MODE_PARM_OFFSET	0x2E0	//mlc nand retry mode parameters
#define NAND_CHIP_ID_OFFSET		0x2F0
#define SCAN_START_PAGE			0//75264//0 arthurc3921 test
#define SCAN_RANGE				(SCAN_START_PAGE + 4096)
#define NEXT_SCAN_PAGE			64

//for IBU
#define NOR_WHOLD_PARTIION	0x0
#define NOR_FLASH_SIZE		(0x400000) 
#define CHUNKID_KEY			0x22DD0100
#define CHUNK_HEADER_SIZE	128
#define PMI_SIZE        	(512)   
#define PMI_HMAC_OFFSET		(8192)
#define MAC_EN_MAGIC		(0xdeadbeef)
#define MAC_START_IN_PMI	(160)
#define STMACH_START_IN_PMI	(184)
#define STMACH_OFFSET_IN_SYSINFOPART 4
#define MAC_OFFSET_IN_SYSINFOPART 1
#define KEY_OFFSET_IN_SYSINFOPART 2

/*
 * Useful MACROs
 */
#define KEY_ATOH(result, str, condition, idx) \
    do { \
        result <<= 4; \
        if ((str[idx + 2] - '0') <= 9) \
	{ \
            result |= ((str[idx + 2] - '0') & 0x0000000F); \
	} \
        else \
	{ \
            result |= ((str[idx + 2] - 'a' + 10) & 0x0000000F); \
	} \
        idx++; \
    } while (condition)

//arthurc3921 add++

#ifdef CONFIG_ALI_CRYPTO_NAND
  #define SUPPORT_CRYPTO
    #ifdef SUPPORT_CRYPTO
      //#define DEBUG_CRYPTO
  #endif
#endif

struct nandflash_parameter
{
    u32 bytes_perpage;
    u32 pages_perblock;
    u32 blocks_perchip;	
    u32 eccsec_size;
    u32 eccredu_size;
    u32 rowaddr_cycle;
    u32 ecctype;
    u32 read_timing;
    u32 write_timing;
    u32 eccsec_perpage;
    u32 pageshift_perblock;    
    u8 data_scramble;
    u8 rr_mode;
};

/* boot area nand flash parrameter */
struct boot_area_nand_parameter	
{
	u32 wBlksPerChip;	//Blocks per chip
	u32 wBytesPerPage;	//Bytes per page (2K,4K,8K etc)  
	u32 wPagesPerBlock;	//Pages per Block(64,128, 192,256 etc) 
	u32 wReserved0[6];	//Reserved for compatibility, filled with 0xffffffff
	u32 wReadClock;		//Read clock setting 
	u32 wWriteClock;	//Write clock setting
	u32 wReserved1[5];	//Reserved for compatibility, filled with 0xffffffff
};



/*! @addtogroup DeviceDriver
 *  @{
 */
 
/*! @addtogroup ALiNAND
 *  @{
*/
#define ALI_NAND_REG_LEN 0x5C
/*! @struct ali_nand_host
@brief ali nand host struct
*/
struct ali_nand_host {
	struct mtd_info		mtd;		//!< mtd information.
	struct nand_chip	nand;		//!< nand chip information.
	struct mtd_partition	*parts;	//!< mtd partitions information.
	struct device		*dev;		//!< ali nand drvice.
	void __iomem		*regs;		//!< io memory mapping of ali nand driver.
	u8					*dma_buf;	//!< DMA buffer virtual address of ali nand driver.
	dma_addr_t			hw_dma_addr;	//!< DMA buffer physical address of ali nand driver.
	int chip_id;
	int chip_pkg;
	int chip_ver;
	void __iomem 		*ali_nand_reg;
	struct nandflash_parameter nf_parm;
	u32 nr_parts;
	u8 pin_mux_lock_flag;
	u8 data_all_ff;
	u32 ali_nand_regs[ALI_NAND_REG_LEN/4];
};

/*chip id*/
#define C3603 	0x3603		
/*package*/
#define M3601E	0x1
#define M3701F	0x5
#define M3701E	0x7
#define M3606B	0x9
#define M3606	0x0B
#define M3603	0x0C
/*io*/
#define M36xx_BGA_CLE	(89%32)	//(67%32) // 3
#define	M36xx_BGA_CEJ	(90%32)	//(68%32) // 4
#define	M36xx_BGA_ALE	(88%32)	// 24
#define M36xx_BGA_WEJ	(65%32	// 1
#define M36xx_BGA_REJ	(85%32)	// 21
#define M36xx_BGA_DATA0 (77%32)	// 13
#define M36xx_BGA_DATA1 (78%32)	// 14
#define M36xx_BGA_DATA2 (79%32)	// 15
#define M36xx_BGA_DATA3 (80%32)	// 16
#define M36xx_BGA_DATA4 (81%32)	// 17
#define M36xx_BGA_DATA5 (82%32)	// 18
#define M36xx_BGA_DATA6 (86%32)	// 22
#define M36xx_BGA_DATA7 (87%32)	// 23

#define	M36xx_QFP_CLE	(102%32)	//(67%32) // 3
#define	M36xx_QFP_CEJ	(79%32)	//(68%32) // 4
//#define M36xx_QFP_ALE	(88%32)	// 24
//#define M36xx_QFP_WEJ	(65%32)	// 1
#define M36xx_QFP_WPJ	(101%32)	// 1
//#define M36xx_QFP_REJ (85%32) // 21
#define M36xx_QFP_DATA0 (91%32)	// 13
#define M36xx_QFP_DATA1 (92%32)	// 14
#define M36xx_QFP_DATA2 (93%32)	// 15
#define M36xx_QFP_DATA3 (69%32)	// 16
#define M36xx_QFP_DATA4 (70%32)	// 17
#define M36xx_QFP_DATA5 (71%32)	// 18
#define M36xx_QFP_DATA6 (72%32)	// 22
#define M36xx_QFP_DATA7 (73%32)	// 23 

/*chip id*/
#define C3503			0x3503		//!< Chip ID: 3503
#define C3821			0x3821		//!< Chip ID: 3821

/*chip id*/
#define C3901			0x3901		//!< Chip ID: 3901
/*package*/
#define PIN256_NMP		0x0			//!< IC package: PIN256_NMP

/*chip id*/
#define C3701			0x3701		//!< Chip ID: 3701
/*package*/
#define M3701H			0x2			//!< IC package id: M3701H
#define M3701C			0x3			//!< IC package id: M3701C
#define M3701NMP		0x4			//!< IC package id: M3701NMP

/*chip id*/
#define C3921			0x3921		//!< Chip ID: 3921
/*package*/
#define QFP256			0x0			//!< IC package: LQFP256
#define BGA445			0x1			//!< IC package: BGA445

//#define SYSINFO_DBG_PRINTK ALI_NAND_DEBUG
#define SYSINFO_DBG_PRINTK(...)  do{}while(0)


/* ALi nand flash registers define */
#define NF_dwREADREDU0	0x00		//!< NF Register index: read OOB.
#define NF_dwREADREDU1	0x04		//!< NF Register index: read OOB.
#define NF_dwWRITEREDU0	0x08		//!< NF Register index: write OOB.
#define NF_dwWRITEREDU1	0x0C		//!< NF Register index: write OOB.
#define NF_wECCERROCR	0x10		//!< NF Register index: ECC error occur.
#define NF_wECCCURSEC	0x14		//!< NF Register index: ECC correctable sector.
#define NF_bPIODATA		0x18		//!< NF Register index: NF PIO data.
#define NF_bCTRL		0x1C		//!< NF Register index: NF control.

/*! @enum E_NF_Control
@brief NAND Flash control signal.
*/
enum E_NF_Control
{
	NF_CEJ	= (1<<7),				//!< Chip Enable to PAD
	NF_ALE	= (1<<5),				//!< Address Latch Enable to PAD
	NF_CLE	= (1<<4),				//!< Command Latch Enable to PAD
};

#define NF_bMODE		0x20		//!< NF Register index for NF Mode.
/*! @enum E_NF_Mode
@brief NAND Flash Mode.
*/
enum E_NF_Mode
{
	NF_EN			= (1<<7),		//!< NF Control Engine Enable.
	NF_WRITE_PROTECT= (1<<6),		//!< NF Write Protect. for 3921/3821
	NF_EDO_EN		= (1<<3),		//!< N/A. Don't use.
	NF_EF_MODE_EN	= (1<<2),		//!< NF EF Mode Enable.
	NF_CRC_MODE_EN	= (1<<0),		//!< NF CRC Mode Enable.
};	

#define NF_bREADCYC		0x24		//!< NF Register index: read cycle clock.
#define NF_bWRITECYC	0x28 		//!< NF Register index: write cycle clock.
#define NF_bEDORBCYC	0x2C		//!< NF Register index: EDO R/B Cycle Length.

/*! @enum E_NF_CRYPTO
@brief NAND Flash Crypro Engine setting.
*/
enum E_NF_CRYPTO
{
	NF_CRYPTO_EN 	= (1<<7),		//!< NF Crypto Enable.
};

#define NF_bDMACTRL		0x30		//!< NF Register index: DMA ctrl.
/*! @enum E_NF_DMA_Control
@brief NAND Flash DMA Control.
*/
enum E_NF_DMA_Control
{
	NF_DMA_IMB_EN	= (1<<2),		//!< IMB Mode Enable.
	NF_DMA_OUT		= (1<<1),		//!< DMA Data Direction.
	NF_DMA_EN		= (1<<0),		//!< DMA Mode Enable.
};

#define NF_bDMALEN		0x34		//!< NF Register index: DMA Length Control.
/*! @enum E_NF_DMA_Length_Control
@brief NAND Flash DMA Length Control.
*/
enum E_NF_DMA_Length_Control
{
	NF_FW_RED_0		= (0<<5),		//!< Redundant byte length
	NF_FW_RED_1		= (1<<5),		//!< Redundant byte length
	NF_FW_RED_2		= (2<<5),		//!< Redundant byte length
	NF_FW_RED_3		= (3<<5),		//!< Redundant byte length
	NF_FW_RED_4		= (4<<5),		//!< Redundant byte length
	NF_DMA_1_SEC	= (1<<0),		//!< DMA Transfer length control
	NF_DMA_2_SEC	= (1<<1),		//!< DMA Transfer length control
	NF_DMA_4_SEC	= (1<<2),		//!< DMA Transfer length control
	NF_DMA_8_SEC	= (1<<3),		//!< DMA Transfer length control
	NF_DMA_16_SEC	= (1<<4),		//!< Not Supported
};

#define NF_bECCCTRL		0x38		//!< NF Register index: ECC ctrl
/*! @enum E_NF_ECCCTRL
@brief NAND Flash ECC Control.
*/
enum E_NF_ECCCTRL
{
	NF_ECC_EN		= (1<<7),
	NF_BCH_MODE		= (1<<6),		//!< BCH algorithm selection, refer to bit3 and bit2
	NF_ECC_AUTO_STOP	= (0<<4),	//!< Do ecc, auto stop when uncorrectable error occurs
	NF_ECC_AUTO_STOP_FF = (1<<4),	//!< Do ecc, auto stop when uncorrectable error occurs,except ALL FF
	NF_ECC_NON_STOP	= (2<<4),		//!< Do ecc, no auto stop
	NF_ECC_BYPASS	= (3<<4),		//!< Do not do ECC
	NF_BCH_16B_MODE = (0x20 << 2),	//!< 16bits ECC for 2K/4K page NF, raw area of LBA NF
	NF_BCH_24B_MODE = (0x30 << 2),	//!< 24bits ECC for 8K pate NF
	NF_BCH_40B_MODE = (0x21 << 2),	//!< 40bits ECC for 8K pate NF
	NF_BCH_48B_MODE = (0x31 << 2),	//!< 48bits ECC for 8K pate NF
	NF_BCH_60B_MODE = (0x22 << 2),	//!< 60bits ECC for 8K pate NF
	NF_ALL_FF_FLAG	= (1<<1),		//!< A blank block.
	NF_ECC_INI		= (1<<0),		//!< ECC soft reset
	NF_ECC_1B		= 0,			//!< 1bit ECC, N/A
};		

#define NF_bECCSTS		0x3C		//!< NF Register index: NF ECC status
#define NF_dwINTFLAG	0x40		//!< NF Register index: INT flag
#define NF_bINTFLAG		0x40		//!< NF Register index: INT flag
/*! @enum E_NF_INTFLAG
@brief NAND Flash interrupt flag.
*/
enum E_NF_INTFLAG
{
	NF_DMA_FLAG = (1<<7),			//!< NF read or write data finish in DMA mode,
	NF_ECC_FLAG = (1<<5),			//!< NF ECC finish during reading data from NF.
	IMB_RD_FSH_FLAG	= (1<<17),		//!< IMB read operation finish flag.
	IMB_WR_FSH_FLAG	= (1<<16),		//!< IMB write operation finish flag.
	IMB_RD_FSH_INT_EN = (1<<27),	//!< IMB read operation finish interrupt enable.
	IMB_WR_FSH_INT_EN = (1<<26),	//!< IMB write operation finish interrupt enable.
	NF_DMA_INT_EN = (1<<25),		//!< NF DMA interrupt enable.
	NF_ECC_INT_EN = (1<<24),		//!< NF ECC interrupt enable.
};

#define NF_bCLKCTRL		0x44		//!< NF Register index: NAND Flash Gated Clock Control.
/*! @enum E_NF_CLKCTRL
@brief NAND Flash Gated Clock Control.
*/
enum E_NF_CLKCTRL
{
	REDUR_GCLK_EN	= (1<<7),		//!< Redundant Read Gating Clock Enable.
	CPUW_GCLK_EN	= (1<<6),		//!< MCU Write Gating Clock Enable.
	ECC_GCLK_EN		= (1<<5),		//!< ECC Block Gating Clock Enable.
	COR_GCLK_EN		= (1<<4),		//!< COR Block Gating Clock Enable
	NF_ECC_INT_EN3	= (1<<3),		//!< 4th Layer Power Control.
	NF_ECC_INT_EN2	= (1<<2),		//!< 3rd Layer Power Control.
	NF_ECC_INT_EN1	= (1<<1),		//!< 2nd Layer Power Control.
	NF_ECC_INT_EN0	= (1<<0),		//!< 1st Layer Power Control.
};

#define NF_dwDMADATA	0x48		//!< NF Register index:  NF DMA data
#define NF_dwDMAADDR	0x4C		//!< NF Register index:  NF DMA address
#define NF_dwDMACONFIG	0x50		//!< NF Register index:  NF DMA config
/*! @enum E_NF_DMACONFIG
@brief NAND Flash DMA Configure.
*/
enum E_NF_DMACONFIG
{
	INIT_DMA_IMB	=(1<<18),		//!< Toggle INT DMA IMB before DMA.
	INIT_DMA_SRAM	=(1<<17),		//!< Toggle INT DMA DM before DMA.
	NF_HI_PRIORITY	=(1<<16),		//!< NF HI Priority.
};

#define NF_dwVERSIONID		0x54	//!< NF Register index: NF Version ID.
#define NF_dwDETECTBLANK	0x58	//!< NF Register index: NF Detect Sectors Content.

#define HW_DMA_READ 		0		//!< NF DMA directory, Read.
#define HW_DMA_WRITE 		1		//!< NF DMA directory, Write.


// ALi SOC Registers define
#define SOC_STRAP_CTRL_REG	0x74	//!< Strap Control Register of ALi SOC. 

/*! @enum E_SOC_STRAP_CTRL
@brief SOC Strap pin control.
*/
enum E_SOC_STRAP_CTRL
{
	BOOT_SEL_TRIG_NF	= (1<<23),	//!< Boot Select Triger: for NAND Flash.
	STRAP_PIN_EN_NF		= (1<<18),	//!< Boot Strap Pin: for NAND Flash.
};

#define SOC_INT_POLARITY		0x2C
#define SOC_INT_ENABLE			0x3C

#define C3921_SOC_PINMUX_REG1	0x88	//!< SOC Register index: NF Pinmux Configure1.
#define C3921_NF_PIN_SEL		(1<<3)	//@pinmux 1 NF Pinmux Select.
#define C3921_SF_PIN_SEL		(1<<19)	//@pinmux 1 SF Pinmux Select.
#define C3921_SDIO_SEL			(1<<5)	//@pinmux 1
#define C3921_EMMC_SEL2			(1<<28)	//@pinmux2
#define C3921_EMMC_SEL			(1<<5)	//@pinmux2
#define C3921_SOC_PINMUX_REG2	0x8C	//!< SOC Register index: NF Pinmux Configure2.
#define	C3921_STRAPIN_SEL_ENABLE	(1<<31)	//!< NF Strap Pin Select.
#define C3921_SOC_SYSTEM_CLOCK	0x7C
#define SEL_54M		0
#define SEL_74M		(1<<18)
#define SEL_118M	(2<<18)



#define C3503_SOC_PINMUX_REG1	0x88	//!< SOC Register index: NF Pinmux Configure1.
#define C3503_STRAP_PIN			(1<<8) 	//0 : by PINMUX(490h.3), 1: by STRAP PIN 
#define C3503_SEL_SFALSH		(1<<17)	//1 : select sflash
#define C3503_SEL_256_SQI		(1<<16)	//1 : sel SQI
#define C3503_SOC_PINMUX_REG2	0x8C	//!< SOC Register index: NF Pinmux Configure1.
#define NF_CE0_CE1				(1<<21)	//0 : CE0 enable
#define C3503_SEL_EMMC 			(1<<19)	//1 : select eMMC
#define C3503_SOC_PINMUX_REG3	0xA4	//!< SOC Register index: NF Pinmux Configure1.
#define NF_FUNC_EN				(1<<31)	//1: enable NF, while (88h.8)=0
#define PK292_NAGRA_SEL			(1<<8)	
#define C3503_SOC_SYSTEM_CLOCK	0x7C
#define SEL_74M_54M				(1<<30)	//0:74M 1:54M

#define C3821_SOC_SYSTEM_CLOCK	0x78
#define C3821_SOC_PINMUX_REG3	0x490
#define BGA_NF_SDIO_STRAP_PIN	(1<<26) //0 : by STRAP PIN, 1: by PINMUX(490.3)
#define BGA_NF_CI_STRAP_PIN		(1<<25) //0 : by STRAP PIN, 1: by PINMUX(490.4)
#define QFP_SF_NF_STRAP_PIN		(1<<24) //0 : by STRAP PIN, 1: by PINMUX(490.1)
#define NF_256_PKT_CS			(1<<11)
#define QFP_156_SEL_CI			(1<<4)	//0 : pinmux select NF, 1: pinmux select CI
#define BGA_SEL_NF				(1<<3)				//1 : enable NF pinmux
#define QFP_SEL_NF				(1<<1)			//1:enable NF pinmux


#define SOC_STRAP_PIN_CTRL_REG	0x74
#define NF_BOOT_EN				(1<<30)
#define NF_BOOT_VAL				(1<<18)

#define C3701_SOC_SYSTEM_CLOCK	0x78
#define C3701_SEL_74M_54M		(1<<7)	//0:74M 1:54M
#define C3701_SOC_PINMUX_REG1	0x88	//!< SOC Register index: NF Pinmux Configure1.
#define C3701_BGA_WP_XGPIO		41
#define C3701_QFP_WP_XGPIO		14
#define NF128_SEL				(1<<7)




/*!
@brief	ali nand driver initialize and register to kernel.
@param[in] void.
@return int
@retval  0: SUCCESS
@retval  -1: FAIL
*/
static int __init ali_nand_driver_init(void);

/*!
@brief	ali nand driver cleanup and unregister to kernel.
@param[in] void.
@return void
@retval None
@retval None
*/
static void __exit ali_nand_driver_cleanup(void);

/*!
@brief	ali nand driver suspend.
@param[in] pdev : A structure pointer to a platform device.
@param[in] state: power status of platform device.
@return int
@retval  0: SUCCESS
@retval  -1: FAIL
*/
static int ali_nand_suspend(struct platform_device *pdev, pm_message_t state);

/*!
@brief	ali nand driver resume.
@param[in] pdev : A structure pointer to a platform device.
@return int
@retval  0: SUCCESS
@retval  -1: FAIL
*/
static int ali_nand_resume(struct platform_device *pdev);

/* Debugging macro and defines */
#define MTD_DEBUG_LEVEL0	(0)	/* Quiet   */
#define MTD_DEBUG_LEVEL1	(1)	/* Audible */
#define MTD_DEBUG_LEVEL2	(2)	/* Loud    */
#define MTD_DEBUG_LEVEL3	(3)	/* Noisy   */
//#define CONFIG_MTD_DEBUG//arthur add
//#define CONFIG_MTD_DEBUG_VERBOSE 99
#ifdef CONFIG_MTD_DEBUG
#define DEBUG(n, args...)					\
	do {									\
		if (n <= CONFIG_MTD_DEBUG_VERBOSE)	\
			printk(KERN_INFO args);			\
	} while(0)
#else /* CONFIG_MTD_DEBUG */
	#define DEBUG(n, args...)				\
			do {							\
				if (0)						\
					printk(KERN_INFO args);	\
			} while(0)
#endif /* CONFIG_MTD_DEBUG */

/*!
@}
*/

/*!
@}
*/

#ifdef __cplusplus
}
#endif

#endif  /* _ALI_NAND_H */

