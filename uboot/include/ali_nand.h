#ifndef _ALI_NAND_H
#define _ALI_NAND_H
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
 
#include <nand.h>

#ifdef HAVE_PART_TBL_PARTITION
#define MAX_STBPART_NUM		64
#else
#define MAX_STBPART_NUM		31
#endif

#define ALI_NAND_PART_0		    0x0
#define ALI_NAND_PART_1		    0x1
#define ALI_NAND_PART_2		    0x2
#define ALI_NAND_PART_3		    0x3
#define ALI_NAND_PART_4		    0x4
#define ALI_NAND_PART_5		    0x5
#define ALI_NAND_PART_6		    0x6
#define ALI_NAND_PART_7		    0x7
#define ALI_NAND_PART_8		    0x8
#define ALI_NAND_PART_9		    0x9
#define ALI_NAND_PART_10	    0xa
#define ALI_NAND_PART_11	    0xb
#define ALI_NAND_PART_12        0xc
#define ALI_NAND_PART_13        0xd
#define ALI_NAND_PART_ALL	    0xFF
#define ALI_NAND_PMI            0x20
#define ALI_NAND_DRAM           0x21
#define ALI_NAND_BBT            0x22
#define ALI_NAND_SEC_DATA       0x23
#define ALI_NAND_LOADER         0x24
#define ALI_NAND_KERNEL         0x25
#define ALI_NAND_MAC            0x26
#define ALI_NAND_HDCP_KEY       0x27
#define ALI_NAND_HDCP_KEY_BAK   0x28
#define ALI_NAND_LOGO           0x29
#define ALI_NAND_LOGO_BAK       0x2a
#define ALI_NAND_STMACH         0x2b
#define ALI_NAND_ENVVAR         0x2c
#define ALI_NAND_UBOOT          0x2d
#define ALI_NAND_UBOOT_BAK      0x2e
#define ALI_NAND_PART_MAX       0x2e

#define INVALID_BLK_NUM	0xFFFFFFFF

//when call ali_nand_write_page_hwecc/ali_nand_read_page_hwecc, the buffer size should be Datalen + 8 bytes oob_poi data. 
//otherwize the two function will destory the data struture of "mchunkptr old_top" (dlmalloc.c),then system will die.
#define NAND_BUFFER_ADD_SIZE           8

#define PMI_HASH_OFFSET (8192)
#define PMI_SIZE        (512)  
#define PMI_SIG_OFFSET (8)

struct state_machine_t{
    unsigned int b_boot_status;
    unsigned int b_lowlevel_status;
    unsigned int b_application_status;
    unsigned int b_bootloader_upgrade;
    unsigned int b_lowlevel_upgrade;
    unsigned int b_application_upgrade;
    unsigned int b_bootloader_run_cnt;
    unsigned int b_lowlevel_run_cnt;
    unsigned int b_application_run_cnt;
    unsigned int b_reserved1;
    unsigned int b_reserved2;

    //for expand
    unsigned int b_need_upgrade;
    unsigned int b_backup_exist;
    unsigned int b_lowlevel_backup_exist;
    unsigned int b_boot_backup_exist;
    unsigned int b_nor_upgrade;
    unsigned int b_nor_reserved;
    unsigned int b_nor_reserved_upgrade;
    unsigned int b_nand_reserved;
    unsigned int b_nand_reserved_upgrade;
    unsigned int b_nand_whole_upgrade;
    unsigned int b_cur_uboot;
    unsigned int b_reserved[4];
};


struct NAND_CONFIG
{
	unsigned long				wBlksPerChip;			//0x00  Blocks per chip
	unsigned long				wBytesPerPage;			//0x04  Eg. 2K, 4K, 8K/page
	unsigned long				wPagesPerBlock;			//0x08  Eg. 64, 128, 192, 256
	unsigned long				wBytesPerECCSec;		//0x0c  1024
	unsigned long				wBytesPerLBA;			//0x10  512
	unsigned long				bBytesRednt;			//0x14  32 (16 bit ECC) , 46 (24 bit ECC) 
	unsigned long				bBytesPerRow;			//0x18  2, 3, 4 (times of row addr.)
	unsigned long				bECCType;				//0x1c  16bits, 24 bits, 1 bit , no ECC
	unsigned long				wLogEndBlk;				//0x20  Logical end block
	unsigned long				bReadClock;				//0x24  Read clock setting, 0x11, 0x22, 0x33
	unsigned long				bWriteClock;				//0x28  Write clock setting, 0x11, 0x22, 0x33
	unsigned long				wPMPhyEndBlock;		//0x2c  PM physical block
	unsigned long				bBlockShift;				//0x30  Shift bit for block. Could be 5,6, 7 , 8
	unsigned char				bSmReadyMode;			//0x34
	unsigned char				bPollBitMask;			//0x35
	unsigned char				bExpBitMask;			//0x36
	unsigned char				bDelay; 					//0x37
};

typedef struct PMI
{
	unsigned long				Test_Area[3];			//0x00/0, 12Bytes (all 1: 24 bits ECC all 0: 16 bits ECC)
	unsigned long				Signature;				//0x0c/12,   0x3701-55AA
	struct NAND_CONFIG	Config;					//0x10/16, 56Bytes
	unsigned long				Resv[1];					//0x48/72,  Reserved
	unsigned long				DRAM_Init_Start;		//0x4c/76,
	unsigned long				DRAM_Init_Len;			//0x50/80,
	unsigned long				Nand_Bad_Block_Start;	//0x54/84
	unsigned long				Nand_Bad_Block_Len;	//0x58/88
	unsigned long				Security_Data_Start;		//0x5c/92
	unsigned long				Security_Data_Len;		//0x60/96
	unsigned long				Loader_Start;			//0x64/100
	unsigned long				Loader_Len;				//0x68/104
	unsigned long				Resource_Start;			//0x6c/108
	unsigned long				Resource_Len;			//0x70/112
	unsigned long				Linux_Kernel_Start;		//0x74/116
	unsigned long				Linux_Kernel_Len;		//0x78/120
	
#ifdef PMI_PART_IBU
	unsigned long Resv2[9] ;       
    unsigned long Mac_Addr_Start;     //160
    unsigned long HDCP_Key_Start;  
    unsigned long HDCP_Key_Backup_Start; 
    unsigned long Logo_Start;
    unsigned long Logo_Len;      
    unsigned long Logo_Backup_Start;   //180
	unsigned long stmach_addr;      /* state machine, 184 */
    unsigned long envvar_addr;      /* uboot environment variable */
	unsigned long uboot_start;
    unsigned long uboot_len;
	unsigned long uboot_backup_start;
    unsigned long uboot_backup_len;

    unsigned long Resv3[12] ;   
    unsigned long Partation_Num;      //256   actual partations numbers    
    /* Partations 0    "ali-private"
    *  Partations 1    "Kernel"
    *  Partations 2    "SEE"  
    */
    // 260 - 419 is partations offst and length, 420 - 499 actual image len by bytes
    unsigned long Partation_Buf[MAX_STBPART_NUM*3] ;
    unsigned long Resv4[3];  // Rederved for upgrade   
#endif

}PMI_t;

struct Partation
{  
  unsigned long offset ;   // 
  unsigned long part_len ;  // partation total length by bytes
  unsigned long img_len;   // actual imagel length  by bytes
};

/* 128 bytes */
typedef struct
{
#ifdef HAVE_PART_TBL_PARTITION
	char name[32];			/* identifier string */
	char fs_type[16];
	unsigned int flash_type;
	unsigned int start;		/* offset within the master MTD space */
	unsigned int size;			/* partition size */
	unsigned int logicstart;			/* partition logic start */
	unsigned int logicsize;			/* partition logic size */
	unsigned int mask_flags;		/* master MTD flags to mask out for this partition */
	char reserved[56];
#else
	unsigned char name[16];
	unsigned long start;
	unsigned long size;
#endif
}PART_INFO;

typedef struct
{
	unsigned int part_num;
	PART_INFO parts[MAX_STBPART_NUM];
#ifdef HAVE_PART_TBL_PARTITION
	char root[1024];
	char mtdparts[2048];
#endif
}PART_TBL;

struct ali_nand_host {
	struct mtd_info		mtd;
	struct nand_chip	nand;
	struct mtd_partition	*parts;
	struct device		*dev;	
	void __iomem		*regs;
	u8					*dmabuf1;
	dma_addr_t			dmaaddr1;
};



#endif  

