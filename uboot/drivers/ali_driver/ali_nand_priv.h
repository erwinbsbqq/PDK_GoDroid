#ifndef _ALI_NAND_PRIV_H
#define _ALI_NAND_PRIV_H
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
// NAND Flash Controller
#define ALI_NANDREG_BASE      (ALI_SOC_BASE+0x32000)
#define ALI_NANDREG_LEN       0x60
#define ALI_NANDSRAM_BASE     (ALI_NANDREG_BASE+0x1000)
#define ALI_NANDSRAM_LEN      0x800
#define ALI_NANDBUF_LEN       0x2000 + 0x400

#ifdef HAVE_PART_TBL_PARTITION
#define MAX_STBPART_NUM		64
#else
#define MAX_STBPART_NUM		31
#endif

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
#define M36xx_BGA_CLE (89%32)   //(67%32) // 3
#define M36xx_BGA_CEJ (90%32)   //(68%32) // 4
#define M36xx_BGA_ALE (88%32)   // 24
#define M36xx_BGA_WEJ (65%32)   // 1
#define M36xx_BGA_REJ (85%32)   // 21
#define M36xx_QFP_CLE (102%32)  //(67%32) // 3
#define M36xx_QFP_CEJ (79%32)   //(68%32) // 4
#define M36xx_QFP_WPJ (101%32) 	// 1

/*chip id*/
#define C3811		0x3811	
#define C3503		0x3503
#define C3821		0x3821
/*chip id*/
#define C3901		0x3901	    
/*package*/
#define PIN256_NMP	0x0

/*chip id*/
#define C3701		0x3701
/*package*/
#define M3701H		0x2
#define M3701C		0x3
#define M3701NMP	0x4

/* ALi nand flash registers define */
#define NF_dwREADREDU0	0x00	/* read OOB */
#define NF_dwREADREDU1	0x04
#define NF_dwWRITEREDU0	0x08	/* write OOB */
#define NF_dwWRITEREDU1	0x0C
#define NF_wECCERROCR	0x10	/* ECC error occur */
#define NF_wECCCURSEC	0x14	/* ECC correctable sector */
#define NF_bPIODATA		0x18	/* NF PIO data reg. */
#define NF_bCTRL		0x1C	/* NF control reg. */
enum
{
	NF_CEJ	= (1<<7),
	NF_ALE	= (1<<5),
	NF_CLE	= (1<<4),
};	
#define NF_bMODE		0x20
enum
{
	NF_EN			= (1<<7),
	NF_EDO_EN 		= (1<<3),
	NF_EF_MODE_EN	= (1<<2),
	NF_CRC_MODE_EN	= (1<<0),
};

#define NF_bREADCYC		0x24	 /* read cycle clk */
#define NF_bWRITECYC	0x28 	 /* write cycle clk */
#define NF_bEDORBCYC	0x2C	
enum
{
	NF_CRYPTO_EN = (1<<7),	
};	
#define NF_bDMACTRL		0x30	/* DMA ctrl */
enum
{
	NF_DMA_IMB_EN = (1<<2),
	NF_DMA_OUT	  = (1<<1),
	NF_DMA_EN	  = (1<<0),
};		
#define NF_bDMALEN		0x34	/* DMA Length Control */
enum
{
	NF_FW_RED_0		=(0<<5),	/* 0 byte per 512/1024 bytes, such as BA Nand */
	NF_FW_RED_1		=(1<<5),		
	NF_FW_RED_2		=(2<<5),		
	NF_FW_RED_3		=(3<<5),		
	NF_FW_RED_4		=(4<<5),	/* 4 byte per 512/1024 bytes, such as row Nand */
	NF_DMA_1_SEC	=(1<<0),	/* DMA transfer 1 sector 1024 for 16/24 bit ECC, 512 for others */
	NF_DMA_2_SEC	=(1<<1),	/* DMA transfer 2 sector 2048 for 16/24 bit ECC */
	NF_DMA_4_SEC	=(1<<2),	/* DMA transfer 4 sector 4096 for 16/24 bit ECC */
	NF_DMA_8_SEC	=(1<<3),	/* DMA transfer 8 sector 8192 for 16/24 bit ECC */
	NF_DMA_16_SEC	=(1<<4),		
};
#define NF_bECCCTRL		0x38	/* ECC ctrl */
enum
{
	NF_ECC_EN 		= (1<<7),
	NF_BCH_MODE		= (1<<6),		/* BCH algorithm selection, refer to bit3 and bit2 */
	NF_ECC_AUTO_STOP = (0<<4),		/* Do ecc, auto stop when uncorrect error occurs */
	NF_ECC_AUTO_STOP_FF = (1<<4),	/* Do ecc, auto stop when uncorrect error occurs,except ALL FF */
	NF_ECC_NON_STOP	= (2<<4),		/* Do ecc, no auto stop */
	NF_ECC_BYPASS   = (3<<4),		/* Do not do ECC */
	NF_BCH_16B_MODE = (0x20 << 2),	/* 16bit ECC for 2K/4K page NF, raw area of LBA NF*/
	NF_BCH_24B_MODE = (0x30 << 2),	/* 24bit ECC for 8K pate NF */
	NF_BCH_40B_MODE = (0x21 << 2),	/* 40bit ECC for 8K pate NF */
	NF_BCH_48B_MODE = (0x31 << 2),	/* 48bit ECC for 8K pate NF */
	NF_BCH_60B_MODE = (0x22 << 2),	/* 60bit ECC for 8K pate NF */
	NF_ALL_FF_FLAG	= (1<<1),		/* 1: blank block */
	NF_ECC_INI		= (1<<0),		/* ECC soft reset */	
	NF_ECC_1B		= 0,
};

#define NF_bECCSTS			0x3C	/* NF ECC status */
#define NF_dwINTFLAG		0x40	/* INT flag */
enum
{
	NF_DMA_DONE = (1<<7),		/* NF read or write data finish in DMA mode, 1: Finish */
	NF_ECC_DONE = (1<<5),		/* NF ECC finish during reading data from NF */
	IMB_RD_FSH_FLAG	= (1<<17),
	IMB_WR_FSH_FLAG	= (1<<16),	/* 1: Finish */
};						
#define NF_bCLKCTRL			0x44
enum
{
	REDUR_GCLK_EN  = (1<<7),	/*1: disable low power, 0: enable low power*/
	CPUW_GCLK_EN   = (1<<6),
	ECC_GCLK_EN    = (1<<5),
	COR_GCLK_EN	   = (1<<4),
	NF_ECC_INT_EN3 = (1<<3),
	NF_ECC_INT_EN2 = (1<<2),
	NF_ECC_INT_EN1 = (1<<1),
	NF_ECC_INT_EN0 = (1<<0),	
};
#define NF_dwDMADATA		0x48		/* NF DMA data */
#define NF_dwDMAADDR		0x4C		/* NF DMA address */
#define NF_dwDMACONFIG		0x50		/* NF DMA config */
enum
{
	INIT_DMA_IMB	=(1<<18),	
	INIT_DMA_SRAM	=(1<<17),	
	NF_HI_PRIORITY	=(1<<16),	
};
#define NF_dwDETECTBLANK		0x58		/* NF DMA config */

#define HW_DMA_READ 0
#define HW_DMA_WRITE 1

// ALi SOC Registers define
#define SOC_STRAP_CTRL_REG	0x74
enum
{
	BOOT_SEL_TRIG_NF	= (1<<23),
	STRAP_PIN_EN_NF		= (1<<18),
};
#define SOC_PINMUX_REG1     0x88
#define NF_PIN_SEL			(1<<3)
#define SOC_PINMUX_REG2     0x8C
#define	STRAPIN_SEL_ENABLE	(1 << 31)

#endif  

