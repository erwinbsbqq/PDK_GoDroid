#ifndef _ALI_NAND_H
#define _ALI_NAND_H
/*
 *  linux/include/linux/mtd/ali_nand.h
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
#include "basic_types.h"
 
#define MAX_PARTITION_NUM	31

//when call ali_nand_write_page_hwecc/ali_nand_read_page_hwecc, the buffer size should be Datalen + 8 bytes oob_poi data. 
//otherwize the two function will destory the data struture of "mchunkptr old_top" (dlmalloc.c),then system will die.
#define NAND_BUFFER_ADD_SIZE           8

#if 0
#define PARTITION_NUM 256
#define PARITION0_INDEX 260
#define PARITION1_INDEX 268	
#define PARITION2_INDEX 276	
#define PARTITIONS_NAME	512	//partition name 512-1023

/* Partition Flag on PMI */
#define PMI_PART_FLAG_SFT	30
#define PMI_PART_FLAG_MSK	0x3
#define PMI_PART_FLAG_FTL	0x1

/* MTD flags for mtd_partition */
#define MTD_WRITEABLE		0x400	/* Device is writeable */
#define MTD_BIT_WRITEABLE	0x800	/* Single bits can be flipped */
#define MTD_NO_ERASE		0x1000	/* No erase necessary */
#define MTD_POWERUP_LOCK	0x2000	/* Always locked after reset */
#define MTD_DO_FTL		0x08	/* Do flash management for this partition */   // LYH 101227#1
#endif

#define DRIVER_NAME "ali_nand"

/*chip id*/
#define C3901	0x3901
#define C3811	0x3811
#define C3701	0x3701
#define C3603 	0x3603		

/*package*/
#define M3601E	0x1
#define M3701F	0x5
#define M3701E	0x7
#define M3606B	0x9
#define M3606	0x0B
#define M3603	0x0C

/*package*/
#define PIN256_NMP	0x0
#define M3701G	0x02

#define M36xx_BGA_CLE (89%32)   //(67%32) // 3
#define M36xx_BGA_CEJ (90%32)   //(68%32) // 4
#define M36xx_BGA_ALE (88%32)   // 24
#define M36xx_BGA_WEJ (65%32)   // 1
#define M36xx_BGA_REJ (85%32)   // 21
#define M36xx_BGA_DATA0 (77%32) // 13
#define M36xx_BGA_DATA1 (78%32) // 14
#define M36xx_BGA_DATA2 (79%32) // 15
#define M36xx_BGA_DATA3 (80%32) // 16
#define M36xx_BGA_DATA4 (81%32) // 17
#define M36xx_BGA_DATA5 (82%32) // 18
#define M36xx_BGA_DATA6 (86%32) // 22
#define M36xx_BGA_DATA7 (87%32) // 23

#define M36xx_QFP_CLE (102%32)  //(67%32) // 3
#define M36xx_QFP_CEJ (79%32)   //(68%32) // 4
#define M36xx_QFP_WPJ (101%32) // 1
#define M36xx_QFP_DATA0 (91%32)        // 13
#define M36xx_QFP_DATA1 (92%32)        // 14
#define M36xx_QFP_DATA2 (93%32)        // 15
#define M36xx_QFP_DATA3 (69%32)        // 16
#define M36xx_QFP_DATA4 (70%32)        // 17
#define M36xx_QFP_DATA5 (71%32)        // 18
#define M36xx_QFP_DATA6 (72%32)        // 22
#define M36xx_QFP_DATA7 (73%32)        // 23

/*
	    GPIO  031~000  0x430
	    GPIOA 063~032  0x434
	    GPIOB 095~064  0x438
	    GPIOC 127~096  0x43C
	    GPIOD 136~128  0x440 */

//Nand Flash
//#define NF_bCTRL		0x1C	/* NF control reg */
enum
{
	NF_CEJ	= (1<<7),
	NF_ALE	= (1<<5),
	NF_CLE	= (1<<4),
};	
//#define NF_bMODE		0x20
enum
{
	NF_EN	= (1<<7),
	NF_EDO_EN = (1<<3),
	NF_EF_MODE_EN = (1<<2),
	NF_CRC_MODE_EN =(1<<0),
};	
//#define NF_bEDORBCYC		0x2C	
enum
{
	NF_CRYPTO_EN = (1<<7),	
};	
//#define NF_bDMACTRL		0x30	/* DMA ctrl */
enum
{
	NF_DMA_IMB_EN =(1<<2),
	NF_DMA_OUT = (1<<1),	
	NF_DMA_EN = (1<<0),
};		
//#define NF_bDMALEN		0x34	/* DMA Length Control */
enum
{
	NF_FW_RED_0	=(0<<5),	/* 0 byte per 512/1024 bytes, such as BA Nand */
	NF_FW_RED_1	=(1<<5),		
	NF_FW_RED_2	=(2<<5),		
	NF_FW_RED_3	=(3<<5),		
	NF_FW_RED_4	=(4<<5),	/* 4 byte per 512/1024 bytes, such as row Nand */
	NF_DMA_1_SEC	=(1<<0),	/* DMA transfer 1 sector 1024 for 16/24 bit ECC, 512 for others */
	NF_DMA_2_SEC	=(1<<1),	/* DMA transfer 2 sector 2048 for 16/24 bit ECC */
	NF_DMA_4_SEC	=(1<<2),	/* DMA transfer 4 sector 4096 for 16/24 bit ECC */
	NF_DMA_8_SEC	=(1<<3),	/* DMA transfer 8 sector 8192 for 16/24 bit ECC */
	NF_DMA_16_SEC	=(1<<4),		
};
//#define NF_bECCCTRL		0x38	/* ECC ctrl */
enum
{
	NF_ECC_EN = (1<<7),
	NF_BCH_24B_MODE	= (1<<6),
	NF_ECC_AUTO_STOP = (0<<4),	/* Do ecc, auto stop when uncorrect error occurs */
	NF_ECC_AUTO_STOP_FF = (1<<4),	/* Do ecc, auto stop when uncorrect error occurs,except ALL FF */
	NF_ECC_NON_STOP	= (2<<4),	/* Do ecc, no auto stop */
	NF_ECC_BYPASS = (3<<4),		/* Do not do ECC */
	NF_ALL_FF_FLAG = (1<<1),	/* 1: blank block */
	NF_ECC_INI = (1<<0),		/* ECC soft reset */	
	NF_ECC_1B = 0,
};		
//#define NF_dwINTFLAG		0x40	/* INT flag */
enum
{
	NF_DMA_DONE = (1<<7),		/* NF read or write data finish in DMA mode, 1: Finish */
	NF_ECC_DONE = (1<<5),		/* NF ECC finish during reading data from NF */
	IMB_RD_FSH_FLAG	= (1<<17),
	IMB_WR_FSH_FLAG	= (1<<16),	/* 1: Finish */
};						
//#define NF_bCLKCTRL		0x44
enum
{
	REDUR_GCLK_EN = (1<<7),	/*1: disable low power, 0: enable low power*/
	CPUW_GCLK_EN = (1<<6),
	ECC_GCLK_EN = (1<<5),
	COR_GCLK_EN = (1<<4),
	NF_ECC_INT_EN3 = (1<<3),
	NF_ECC_INT_EN2 = (1<<2),
	NF_ECC_INT_EN1 = (1<<1),
	NF_ECC_INT_EN0 = (1<<0),	
};
//#define NF_dwDMACONFIG		0x50
enum
{
	INIT_DMA_IMB	=(1<<18),	
	INIT_DMA_SRAM	=(1<<17),	
	NF_HI_PRIORITY	=(1<<16),	
};	

enum{
	ECC_16=0,
	ECC_24=1,	
	ECC_1=2,
	CRC_MODE=3,
	EF_MODE=4,
	BA_MODE=5,
	ECC_24_SCRB=6,
	LBA_PNP=0,
};

#if 0
struct ali_nf_regs
{
u32 NF_dwREADREDU0;		/*0x00	 read OOB */
u32 NF_dwREADREDU1;		/* 0x04 */
u32 NF_dwWRITEREDU0;		/* 0x08	 write OOB */
u32 NF_dwWRITEREDU1;		/* 0x0C */
u32 NF_wECCERROCR;			/* 0x10	 ECC error occur */
u32 NF_wECCCURSEC;			/* 0x14	 ECC correctable sector */
u32 NF_bPIODATA;			/* 0x18	 NF PIO reg */
u32 NF_bCTRL;				/* 0x1C	 NF control reg */
u32 NF_bMODE;				/* 0x20 */
u32 NF_bREADCYC;	    		/* 0x24	 read cycle clk */
u32 NF_bWRITECYC;			/* 0x28 	 write cycle clk */
u32 NF_bEDORBCYC;			/* 0x2C */	
u32 NF_bDMACTRL;			/* 0x30	 DMA ctrl */
u32 NF_bDMALEN;			/* 0x34	 DMA Length Control */
u32 NF_bECCCTRL;			/* 0x38	 ECC ctrl */
u32 NF_bECCSTS;				/* 0x3C	 NF ECC status */
u32 NF_dwINTFLAG;			/* 0x40	 INT flag */
u32 NF_bCLKCTRL;			/* 0x44 */
u32 NF_dwDMADATA;			/* 0x48 */
u32 NF_dwDMAADDR;			/* 0x4C */
u32 NF_dwDMACONFIG;		/* 0x50 */
};

#define NF_dwREADREDU0		0x00	 /*read OOB */
#define NF_dwREADREDU1		0x04 
#define NF_dwWRITEREDU0	0x08	 /*write OOB */
#define NF_dwWRITEREDU1	0x0C
#define NF_wECCERROCR		0x10	 /*ECC error occur */
#define NF_wECCCURSEC			0x14	 /*ECC correctable sector */
#define NF_bPIODATA			0x18	 /*NF PIO reg */
#define NF_bCTRL				0x1C	 /*NF control reg */
#define NF_bMODE				0x20 
#define NF_bREADCYC	    		0x24	 /*read cycle clk */
#define NF_bWRITECYC			0x28 	 /*write cycle clk */
#define NF_bEDORBCYC			0x2C
#define NF_bDMACTRL			0x30	 /*DMA ctrl */
#define NF_bDMALEN				0x34	 /*DMA Length Control */
#define NF_bECCCTRL				0x38	 /*ECC ctrl */
#define NF_bECCSTS				0x3C	 /*NF ECC status */
#define NF_dwINTFLAG			0x40	 /*INT flag */
#define NF_bCLKCTRL				0x44
#define NF_dwDMADATA			0x48
#define NF_dwDMAADDR			0x4C
#define NF_dwDMACONFIG		0x50
#endif

#define dwREADREDU0		0x00	 /*read OOB */
#define dwREADREDU1		0x04 
#define dwWRITEREDU0		0x08	 /*write OOB */
#define dwWRITEREDU1		0x0C
#define wECCERROCR			0x10	 /*ECC error occur */
#define wECCCURSEC			0x14	 /*ECC correctable sector */
#define bPIODATA			0x18	 /*NF PIO reg */
#define bCTRL				0x1C	 /*NF control reg */
#define bMODE				0x20 
#define bREADCYC	    		0x24	 /*read cycle clk */
#define bWRITECYC			0x28 	 /*write cycle clk */
#define bEDORBCYC			0x2C
#define bDMACTRL			0x30	 /*DMA ctrl */
#define bDMALEN				0x34	 /*DMA Length Control */
#define bECCCTRL				0x38	 /*ECC ctrl */
#define bECCSTS				0x3C	 /*NF ECC status */
#define dwINTFLAG			0x40	 /*INT flag */
#define bCLKCTRL				0x44
#define dwDMADATA			0x48
#define dwDMAADDR			0x4C
#define dwDMACONFIG		0x50

struct ali_nand_host {
#if 0
	struct mtd_info	mtd;
	struct nand_chip	nand;
	struct mtd_partition	*parts;   //not fully support partition
	struct mtd_info *part_mtd[MAX_PARTITION_NUM];
	UINT8	*dmabuf1;
	UINT8	*dmaaddr1;   //dma_addr_t
	
	UINT32	base_addr;
	int 	crypto_mode;
#else

#endif
	UINT32	chip_id;
	UINT8	chip_package;
	UINT8	chip_ver;
};

#endif  

