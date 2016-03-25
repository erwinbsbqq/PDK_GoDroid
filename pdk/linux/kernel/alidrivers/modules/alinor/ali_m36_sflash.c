/*
 * MTD SPI driver for ST M25Pxx (and similar) serial flash chips
 *
 * Author: Mike Lavender, mike@steroidmicros.com
 *
 * Copyright (c) 2005, Intec Automation Inc.
 *
 * Some parts are based on lart.c by Abraham Van Der Merwe
 *
 * Cleaned up and generalized based on mtd_dataflash.c
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/version.h>  
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#ifdef CONFIG_ARM
#include <asm-generic/uaccess.h>  // cari ,for copy_to_user ,copy_from_user
#include <asm/io.h>   // cari chen
#endif
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <alidefinition/adf_sysdef.h>
#include <alidefinition/adf_boot.h>
#include <ali_reg.h>
#include <ali_soc.h>

#include "ali_flash_data.h"

#ifdef CONFIG_ALI_NAND
#include <linux/platform_device.h>
#include <linux/mtd/nand.h>
extern struct bus_type platform_bus_type;
#endif

#if 0
#define PRINTK_INFO(x...) printk(KERN_INFO x)
#else
#define PRINTK_INFO(x...) do{}while(0)
#endif

#if 0
#define PRINTK_SFLASH(x...) printk(KERN_INFO x)
#else
#define PRINTK_SFLASH(x...) do{}while(0)
#endif

extern int add_mtd_device(struct mtd_info *mtd);
extern int del_mtd_device(struct mtd_info *mtd);
extern int add_mtd_partitions(struct mtd_info *, const struct mtd_partition *,int);
extern int del_mtd_partitions(struct mtd_info *);
extern int parse_mtd_partitions(struct mtd_info *master, const char **types,\
				struct mtd_partition **pparts,struct mtd_part_parser_data *data);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))				
extern int mtd_device_parse_register(struct mtd_info *mtd, const char * const *types,\
			    struct mtd_part_parser_data *parser_data,const struct mtd_partition *parts,int nr_parts);
#else
extern int mtd_device_parse_register(struct mtd_info *mtd, const char **types,\
			    struct mtd_part_parser_data *parser_data,const struct mtd_partition *parts,int nr_parts);
#endif

#define ALI_SOC_REG_BASE_PHYS 0x18000000

#define FLASH_PAGESIZE		256

/* Flash opcodes. */
#define	OPCODE_WREN		    0x06	/* Write enable */
#define	OPCODE_RDSR		    0x05	/* Read status register */
#define	OPCODE_WRSR		    0x01	/* Write status register 1 byte */
#define	OPCODE_NORM_READ	0x03	/* Read data bytes (low frequency) */
#define	OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define	OPCODE_PP		    0x02	/* Page program (up to 256 bytes) */
#define	OPCODE_BE_4K		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
#define	OPCODE_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define	OPCODE_SE		    0xd8	/* Sector erase (usually 64KiB) */

#define	OPCODE_JEDEC_ID		0x9f	/* Read JEDEC ID */
#define	OPCODE_DEV_ID		0xAB	/* Read Device ID */
#define	OPCODE_M_D_ID 	    0x90	/* Read Manufacture and Device ID*/

#define OPCODE_GIGA_GET_CSTMID  0x4B    /* Read custom id  */
#define OPCODE_GIGA_ESR         0x44    /* Erase Security Registers */
#define OPCODE_GIGA_PSR         0x42    /* Program Security Registers */
#define OPCODE_GIGA_RSR         0x48    /* Read Security Registers */

#if 0
#define GIGA_PRINTK      printk
#else
#define GIGA_PRINTK(...) do{}while(0)
#endif


/* Status Register bits. */
#define	SR_WIP			1	/* Write in progress */
#define	SR_WEL			2	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define	SR_BP0			4	/* Block protect 0 */
#define	SR_BP1			8	/* Block protect 1 */
#define	SR_BP2			0x10	/* Block protect 2 */
#define	SR_SRWD			0x80	/* SR write protect */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_COUNT	10000000
#define	CMD_SIZE		4

#ifdef CONFIG_ALI_SFLASH_USE_FAST_READ
#define OPCODE_READ 	OPCODE_FAST_READ
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	OPCODE_NORM_READ
#define FAST_READ_DUMMY_BYTE 0
#endif

#define CONFIG_MTD_PARTITIONS
//#define CONFIG_MTD_CMDLINE_PARTS

#ifdef CONFIG_MTD_PARTITIONS
#define	mtd_has_partitions()	(1)
#else
#define	mtd_has_partitions()	(0)
#endif

#define SF_BASE_ADDR		    0xb802e000
#define SYS_FLASH_BASE_ADDR		0xbfc00000//0xafc00000
unsigned long sflash_reg_addr = SF_BASE_ADDR;
#define	SF_INS			(sflash_reg_addr + 0x98)
#define	SF_FMT			(sflash_reg_addr + 0x99)
#define	SF_DUM			(sflash_reg_addr + 0x9A)
#define	SF_CFG			(sflash_reg_addr + 0x9B)
#define SUCCESS         0

#define SF_HIT_DATA		0x01
#define SF_HIT_DUMM		0x02
#define SF_HIT_ADDR		0x04
#define SF_HIT_CODE		0x08
#define SF_CONT_RD		0x40
#define SF_CONT_WR		0x80

UINT32 dummy;
#define write_uint8(addr, val)	do{*((volatile UINT8 *)(addr)) = (val),dummy =*((volatile UINT8 *)(addr)); }while(0)
#define read_uint8(addr)			*((volatile UINT8 *)(addr))
#define write_uint16(addr, val)	do{*((volatile UINT16 *)(addr)) = (val),dummy =*((volatile UINT16 *)(addr)); }while(0)
#define read_uint16(addr)			*((volatile UINT16 *)(addr))
#define write_uint32(addr, val)	do{*((volatile UINT32*)(addr)) = (val),dummy =*((volatile UINT32 *)(addr)); }while(0)
#define read_uint32(addr)			*((volatile UINT32 *)(addr))

#define STD_READ	(0x03 | ((SF_HIT_CODE|SF_HIT_ADDR|SF_HIT_DATA)<<8))
#define FST_READ	(0x0b | ((SF_HIT_CODE|SF_HIT_ADDR|SF_HIT_DATA|SF_HIT_DUMM)<<8))
#define DIO_READ	(0xbb | ((SF_HIT_CODE|SF_HIT_ADDR|SF_HIT_DATA|SF_HIT_DUMM)<<8))
#define QIO_READ	(0xeb | ((SF_HIT_CODE|SF_HIT_ADDR|SF_HIT_DATA|SF_HIT_DUMM|0x20)<<8))

#define STD_SECURITY_READ (0x48 | ((SF_HIT_CODE|SF_HIT_ADDR|SF_HIT_DATA|SF_HIT_DUMM)<<8))
#define STD_MODE	0
#define FST_MODE	STD_MODE
#define DIO_MODE	0x2
#define QIO_MODE	0x3

static UINT16 sflash_default_read = STD_READ;
static UINT8 sflash_default_mode = STD_MODE;

//extern DEFINE_MUTEX(nor_nand_mutex);
//extern nor_nand_mutex;

/* Mutex to protect requesting and releasing pinmux 1 and 2. 
   This is shared by Nand Flash, SDIO and Nor Flash */
 
#if defined(CONFIG_ALI_CHIP_M3921)
static UINT32 reg_data_store_8c;  // cari chen
static UINT32 reg_data_store_88;

extern struct mutex ali_sto_mutex ; // For S3921, use mutex and register to select NF/SD/SF/EMMC 
#endif

/****************************************************************************/

u32 g_chipId;
u32 g_pinmux70;

struct ali_sflash_private {
	struct spi_device *spi;
	struct mutex	  lock;
	struct mtd_info	  mtd;

	const UINT8 *flash_deviceid;
	const UINT8 *flash_id_p;
	UINT16 flash_deviceid_num;
	UINT16 flash_sectors;
	UINT32 flash_size;
	
	UINT8  flash_id;

	unsigned partitioned;
	u8		 erase_opcode;
	u8		 command[CMD_SIZE + FAST_READ_DUMMY_BYTE];
} ali_sflash_priv;


#if defined(CONFIG_ALI_CHIP_M3921)

u8 sf_3921_gpio_num[] = {98, 100, 101, 102, 103, 104, 0};
extern int gpio_disable_pin(int number);
static void strapin_2_pinmux(void)
{
	UINT32 value1;	
	int i = 0;
	
	while(1)
	{
			if (!sf_3921_gpio_num[i])
				break;
			gpio_disable_pin(sf_3921_gpio_num[i]);
			i++;	
	}				

	reg_data_store_8c = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c);
	reg_data_store_88 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c);
	value1 = (value1 & (~((0x01<<31) | (0x01<<28) | (0x01<<5))));
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c) = value1;
	
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	value1 = (value1 & (~((0x01<<5) | (0x01<<3))));
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88) = value1;
	
}

static void nand_2_nor(void)
{
	UINT32 value2 ;
	
	value2 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	value2 = (value2 & (~(0x01<<3)));
	value2 = (value2 |(0x01<<19));
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88) = value2;
}

static void pinmux88_2_default(void)
{
	UINT32 value1;
	
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88);
	value1 = reg_data_store_88;
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x88) = value1;		
}

static void pinmux8c_2_default(void)
{
	UINT32 value1;
	value1 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c);
	value1 = reg_data_store_8c;
	__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x8c) = value1;
}

static void pinmux_2_default(void)
{
	pinmux88_2_default();
	pinmux8c_2_default();
}

void ali_nor_pinmux_release(void) // For S3921, use register to select NF/SD/SF/EMMC  
{
	pinmux_2_default();
	mutex_unlock(&ali_sto_mutex);	
}

void ali_nor_pinmux_set(void)  // For S3921, use register to select NF/SD/SF/EMMC 
{
	mutex_lock(&ali_sto_mutex);
	strapin_2_pinmux();
	nand_2_nor();	
}
#endif

static void pinmux_2_spi(struct ali_sflash_private *priv)
{
	u32 tmp;
#if defined(CONFIG_ALI_CHIP_M3921)
	ali_nor_pinmux_set();
#else

#ifdef CONFIG_ALI_NAND
	struct device *dev;
	struct platform_device *pdev;
	struct mtd_info *nfmtd;
	struct nand_chip *nfchip;
	spinlock_t *nflock;

	dev = bus_find_device_by_name(&platform_bus_type, NULL, "ali_nand.5");
	if (NULL == dev)
	{
	    //printk("[NF] dev: %p\n", dev);
		goto pinmux;
	}
	
	pdev = container_of((void *)dev, struct platform_device, dev);
	if (NULL == pdev)
	{
	    //printk("[NF] pdev: %p\n", pdev);
		goto pinmux;
	}

    nfmtd = platform_get_drvdata(pdev);
	if (NULL == nfmtd || NULL == nfmtd->priv)
	{   
	    //printk("[NF] mtd: %p\n", nfmtd);
		goto pinmux;
	}

	nfchip = nfmtd->priv;
	nflock = &nfchip->controller->lock;
	//spin_lock(nflock);    //lock spin to forbid NF
pinmux:
#endif
	
	if (ALI_C3701 == g_chipId)
	{
		__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x74) &= ~(0x1<<18);
		__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x74) |= 0x40000000;
	}
	else if (ALI_S3503 == g_chipId)
	{
		g_pinmux70 = __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x70);  //(*(volatile uint32_t *)(0xb8000070));
		//printk("    reg70 = %X, %X\n", g_pinmux70);
		tmp = g_pinmux70;
		tmp &= ~(1<<18);
		tmp |= (1<<30);
		__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x74) = tmp;  //*(volatile uint32_t *)(0xb8000074) = tmp;
		//printk("    reg74 = %X, %X\n", *(volatile uint32_t *)(0xb8000074), __REG32ALI(ALI_SOC_REG_BASE_PHYS+0x74));
	}
	
	if (NULL != priv->spi)
	{
	    mutex_lock(&priv->lock);
	}

#endif
}

static void pinmux_2_nand(struct ali_sflash_private *priv)
{
	u32 tmp;
#if defined(CONFIG_ALI_CHIP_M3921)
	ali_nor_pinmux_release();
#else

#ifdef CONFIG_ALI_NAND
	struct device *dev;
	struct platform_device *pdev;
	struct mtd_info *nfmtd;
	struct nand_chip *nfchip;
	spinlock_t *nflock;
	//int nflock_st = 0;

	dev = bus_find_device_by_name(&platform_bus_type, NULL, "ali_nand.5");
	if (NULL == dev)
	{
	    //printk("[NF] dev: %p\n", dev);
		goto pinmux;
	}
	
	pdev = container_of((void *)dev, struct platform_device, dev);
	if (NULL == pdev)
	{
	    //printk("[NF] pdev: %p\n", pdev);
		goto pinmux;
	}

    nfmtd = platform_get_drvdata(pdev);
	if (NULL == nfmtd || NULL == nfmtd->priv)
	{   
		//printk("[NF] mtd: %p\n", nfmtd);
		goto pinmux;
	}

	nfchip = nfmtd->priv;
	nflock = &nfchip->controller->lock;
	//nflock_st = 1;
pinmux:
#endif
	
	if (NULL != priv->spi)
	{
	    mutex_unlock(&priv->lock);
	}

	if (ALI_C3701 == g_chipId)
	{
		__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x74) = 0x40040000;  //writel(0x40040000, (volatile void *)0xB8000074);
	}
	else if (ALI_S3503 == g_chipId)
	{
		/*
		                          74[30]  74[29]  74[24]  74[18]  74[17]  74[15]
         External Nor flash boot     1       /       1       0       0       X
		 SIP flash boot              1       /       1       0       1       X
		 Nand flash boot             1       1       1       1       0       0
		*/
		tmp = g_pinmux70;
		tmp |= ((1<<30)|(1<<29)|(1<<24));
		__REG32ALI(ALI_SOC_REG_BASE_PHYS+0x74) = tmp;  //*(volatile uint32_t *)(0xb8000074) = tmp;
	}
	
#ifdef CONFIG_ALI_NAND
/*
	if (nflock_st)
	{
	    spin_unlock(nflock);    //release spin for NF
	}
*/
#endif

#endif
}

static loff_t g_ali_sflash_commands_ext_group;

static inline struct ali_sflash_private *mtd_to_ali_sflash_private(struct mtd_info *mtd)
{
	return container_of(mtd, struct ali_sflash_private, mtd);
}

static int ali_sflash_config_cmdext_group(struct ali_sflash_private *priv, unsigned int cfg)
{
	unsigned int offset = (cfg&0xFF);
	if (!(cfg>>8))
	{
	    g_ali_sflash_commands_ext_group |= (1<<offset);
	}
	else
	{
		g_ali_sflash_commands_ext_group &= (~(1<<offset));
	}
	return offset;
}

int check_sflash_cmdext_group_by_bitoffset(unsigned int offset)
{
    return (g_ali_sflash_commands_ext_group & (1<<offset));
}

/**************************************************************
 * Function:
 * 	sto_flash_sector_size()
 * Description:
 * 	Return the size of specific sector.
 * Inpute:
 *	dev --- Device control block
 *	sector --- Sector number
 * Return Values:
 *	Size of the specific sector.
 ***************************************************************/
static unsigned long sto_flash_sector_size(struct ali_sflash_private *priv, UINT32 sector)
{
	unsigned char *p;
	
	if (sector >= priv->flash_sectors)
		sector = priv->flash_sectors - 1;

	p = (unsigned char *)ali_flash_sector_map + ali_flash_sector_begin[priv->flash_id] * 2;

	while (sector >= p[0]) 	
	{
		sector -= p[0];
		p += 2;
	}

	return 1 << p[1];
}

/**************************************************************
 * Function:
 * 	sto_flash_sector_align()
 * Description:
 * 	Return the sector which is align with the address.
 * Inpute:
 *	dev --- Device control block
 *	addr --- Specific address
 * Return Values:
 *	Sector which is align with the address.
 ***************************************************************/
static unsigned int sto_flash_sector_align(struct ali_sflash_private *priv, UINT32 addr)
{
	unsigned int i;
	unsigned int sector_size;

	for (i = 0; i < priv->flash_sectors; i++) {
		sector_size = sto_flash_sector_size(priv,i);
		if (addr < sector_size)
		    break;
		addr -= sector_size;
	}

	return i;
}

/**************************************************************
 * Function:
 * 	sto_flash_sector_start()
 * Description:
 * 	Return Start address of of specific sector.
 * Inpute:
 *	dev --- Device control block
 *	sector_no --- Sector number
 * Return Values:
 *	Start address of the specific sector.
 ***************************************************************/
static unsigned long sto_flash_sector_start(struct ali_sflash_private *priv, UINT32 sector_no)
{
	unsigned long addr = 0;
	
	while (sector_no--) 
	{
		addr += sto_flash_sector_size(priv, sector_no);
		//PRINTK_SFLASH("addr = 0x%08x\n",addr);
	}

	return addr;
}

/****************************************************************************/

/*
 * Internal helper functions
 */

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */

static int read_sr(struct ali_sflash_private *flash)
{
	
	ssize_t retval;
	u8 code = OPCODE_RDSR;  //0x05
	u8 val;

	retval = ali_spi_write_then_read(flash->spi, &code, 1, &val, 1);  //

	if (retval < 0) {
		dev_err(&flash->spi->dev, "error %d reading SR\n",
				(int) retval);
		return retval;
	}
	return val;
}

/*
 * Write status register 1 byte
 * Returns negative if error occurred.
 */
/*
static int write_sr(struct ali_sflash_private *flash, u8 val)
{
	flash->command[0] = OPCODE_WRSR;
	flash->command[1] = val;

	return spi_write(flash->spi, flash->command, 2);
}
*/
/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int write_enable(struct ali_sflash_private *flash)
{
#if 0
	u8	code = OPCODE_WREN;

	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
#else
	PRINTK_SFLASH("CARI CHEN ENTER ,%s\n",__FUNCTION__);

	flash->command[0] = OPCODE_WREN;   // 0x06

	return spi_write(flash->spi, flash->command, 1);
#endif
}


/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready_timeout(struct ali_sflash_private *flash) //tmo : timeout occurs
{
	int count;
	int sr;
	
	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < MAX_READY_WAIT_COUNT; count++) 
	{
		if ((sr = read_sr(flash)) < 0)    // sr : status register
			break;
		else if (!(sr & SR_WIP))
			return 0;
		udelay(10);

		/* REVISIT sometimes sleeping would be best */
	}

	return 1;
}

static int wait_till_ready_forever(struct ali_sflash_private *flash)
{
	int sr;

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	while(1){
		if ((sr = read_sr(flash)) < 0)
			break;
		else if (!(sr & SR_WIP))
			return 0;
		udelay(10);

		/* REVISIT sometimes sleeping would be best */
	}

	return 1;
}


/*
 * Erase the whole flash memory
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_chip(struct ali_sflash_private *flash)
{
	PRINTK_SFLASH("%s: %s %dKiB\n",
			dev_name(&flash->spi->dev), __func__,
			flash->mtd.size / 1024);

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_CHIP_ERASE;  //0xc7

	spi_write(flash->spi, flash->command, 1);
	/* Wait until finished previous write command. */
	if (wait_till_ready_forever(flash))
		return 1;

	return 0;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector(struct ali_sflash_private *flash, u32 offset)
{
	PRINTK_SFLASH("%s: %s %dKiB at 0x%08x\n",
			dev_name(&flash->spi->dev), __func__,
			flash->mtd.erasesize / 1024, offset);

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	//flash->command[0] = flash->erase_opcode;  /* IBU */
	flash->command[0] = OPCODE_SE ;  // 0xd8
	flash->command[1] = offset >> 16;
	flash->command[2] = offset >> 8;
	flash->command[3] = offset;

	spi_write(flash->spi, flash->command, CMD_SIZE);
	/* Wait until finished previous write command. */
	if (wait_till_ready_forever(flash))
		return 1;
	
	return 0;
}

/***************************GigaDevice Advanced Security Feature************************/
/*
 * Write status register 2 byte
 * Returns negative if error occurred.
 */
static int ali_sflash_write_giga_srlb_as(struct ali_sflash_private *flash)
{
	int ret;
	/* Send write enable, then erase commands. */
	pinmux_2_spi(flash);

	write_enable(flash);

	flash->command[0] = OPCODE_WRSR;
	flash->command[1] = 0;
	flash->command[2] = (1<<3);

	ret = spi_write(flash->spi, flash->command, 3);
	
	pinmux_2_nand(flash);
	return ret;
}

/*
 * Erase Security Registers
 * Similar to Sector/Block Erase command
 */
static int erase_sector_giga_as(struct ali_sflash_private *flash)
{
	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_GIGA_ESR;
	flash->command[1] = 0;
	flash->command[2] = 0;
	flash->command[3] = 0;

	spi_write(flash->spi, flash->command, CMD_SIZE);
	/* Wait until finished previous write command. */
	if (wait_till_ready_forever(flash))
	{
	    GIGA_PRINTK("[giga] erase fail\n");
		return 1;
	}

	return 0;
}

/*
 * Erase Security Registers: 4*256-byte
 * Returns 0 if successful, non-zero otherwise
 */
static int ali_sflash_erase_giga_as(struct ali_sflash_private *priv)
{
	GIGA_PRINTK("[giga] %s: %s\n", priv->spi->dev.init_name, __func__);

	//mutex_lock(&priv->lock);
	pinmux_2_spi(priv);
	if (erase_sector_giga_as(priv))
	{
		//mutex_unlock(&priv->lock);
		pinmux_2_nand(priv);
		return -EIO;
	}
	//mutex_unlock(&priv->lock);
	pinmux_2_nand(priv);

	return 0;
}

/*
 * Read Custom ID
 * Similar to read JEDEC command
 * Returns zero for success, else a negative errno status code
 */
static int ali_sflash_read_giga_customid(struct ali_sflash_private *priv, unsigned char *id_buf)
{
	int ret;
	u_char tx_buf[4];
	u_char rx_buf[2];
	
	GIGA_PRINTK("[giga] %s: %s\n", priv->spi->dev.init_name, __func__);

    tx_buf[0] = OPCODE_GIGA_GET_CSTMID;
	tx_buf[1] = 0;
	tx_buf[2] = 0;
	memset(rx_buf, 0, 2);
	
	//mutex_lock(&priv->lock);
	pinmux_2_spi(priv);
	ret = ali_spi_write_then_read(priv->spi, tx_buf, 3, rx_buf, 2);
    //mutex_unlock(&priv->lock);
	pinmux_2_nand(priv);
    
	if (ret)
	{
		dev_err(&priv->spi->dev, "error %d reading giga customid\n", ret);
		return ret;
	}
	copy_to_user(id_buf, rx_buf, 2);
	return 0;
}

/*
 * Read Security Registers
 * Similar to Fast Read command
 * Returns nonnegative number transfered in fact, negative if fail
 */
 
static int ali_sflash_read_giga_as(struct ali_sflash_private *priv, struct giga_as_transfer_user *arg)
{
	struct spi_transfer t[2];
	struct spi_message m;
	size_t retlen;
	u_char *buf;

	unsigned int from = arg->op_reg_addr;
	unsigned int len  = arg->op_data_len;

	GIGA_PRINTK("[giga] sflash sectors = %d, size = %dM\n", priv->flash_sectors, (priv->flash_size)/1024/1024);
	GIGA_PRINTK("[giga] %s: %s\n", priv->spi->dev.init_name, __func__);
	GIGA_PRINTK("[giga] sr idx = %d, data len = %d\n", from, len);

	/* sanity checks */
	if (!len)
	{
		GIGA_PRINTK("[giga] no data to read\n");
		return 0;
	}

	if ((from > 0x3FF) || (((from%1024)+len) > 1024))  //4*256-byte(Per Security Registers size)
	{
		GIGA_PRINTK("[giga] invalid security register address\n");
		return -EINVAL;
	}

	buf = kmalloc(sizeof(u_char)*len, GFP_KERNEL);
	if (!buf)
	{
		GIGA_PRINTK("[giga] no memory alloc in kernel\n");
		return -ENOMEM;
	}
	memset(buf, 0, sizeof(u_char)*len);

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	/* NOTE:
	 * OPCODE_FAST_READ (if available) is faster.
	 * Should add 1 byte DUMMY_BYTE.
	 */
	t[0].tx_buf = priv->command;
	t[0].len = CMD_SIZE + 1;//FAST_READ_DUMMY_BYTE;
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = len;
	spi_message_add_tail(&t[1], &m);

	//mutex_lock(&priv->lock);
	pinmux_2_spi(priv);

	/* FIXME switch to OPCODE_FAST_READ.  It's required for higher
	 * clocks; and at this writing, every chip this driver handles
	 * supports that opcode.
	 */

	/* Set up the write data buffer. */
	priv->command[0] = OPCODE_GIGA_RSR;
	priv->command[1] = 0;
	priv->command[2] = from >> 8;
	priv->command[3] = from;

	spi_sync(priv->spi, &m);

	retlen = m.actual_length - CMD_SIZE - 1;//FAST_READ_DUMMY_BYTE;
	
	/* Wait till previous write/erase is done. */
	if (wait_till_ready_timeout(priv))
	{
		/* REVISIT status return?? */
		//mutex_unlock(&priv->lock);
		pinmux_2_nand(priv);
		return -1;
	}

	//mutex_unlock(&priv->lock);
	pinmux_2_nand(priv);

    copy_to_user((u_char *)(arg->op_data_buf), buf, retlen);
	kfree(buf);
	return retlen;
}

/*
 * Program Security Registers
 * Similar to the Page Program command
 * Returns nonnegative number transfered in fact, negative if fail
 */
static int ali_sflash_write_giga_as(struct ali_sflash_private *priv, struct giga_as_transfer_user *arg)
{
	u32 page_offset, page_size;
	struct spi_transfer t[2];
	struct spi_message m;
    size_t retlen;
	u_char *buf;

	unsigned int to  = arg->op_reg_addr;
	unsigned int len = arg->op_data_len;

	GIGA_PRINTK("[giga] %s: %s\n", priv->spi->dev.init_name, __func__);
	GIGA_PRINTK("[giga] sr idx = %d, data len = %d\n", to, len);

	/* sanity checks */
	if (!len)
	{
		GIGA_PRINTK("[giga] no data to write\n");
		return 0;
	}

	if ((to > 0x3FF) || (((to%1024)+len) > 1024))  //4*256-byte(Per Security Registers size)
	{
		GIGA_PRINTK("[giga] invalid security register address\n");
		return -EINVAL;
	}

	buf = kmalloc(sizeof(u_char)*len, GFP_KERNEL);
	if (!buf)
	{   
		GIGA_PRINTK("[giga] no memory alloc in kernel\n");
		return -ENOMEM;
	}
	copy_from_user(buf, (u_char *)(arg->op_data_buf), sizeof(u_char)*len);
	
	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = priv->command;
	t[0].len = CMD_SIZE;
	spi_message_add_tail(&t[0], &m);

	
	t[1].tx_buf = buf;
	spi_message_add_tail(&t[1], &m);

	//mutex_lock(&priv->lock);
	pinmux_2_spi(priv);

	write_enable(priv);

	/* Set up the opcode in the write buffer. */
	priv->command[0] = OPCODE_GIGA_PSR;
	priv->command[1] = 0;
	priv->command[2] = to >> 8;
	priv->command[3] = to;

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= FLASH_PAGESIZE)
	{
		t[1].len = len;

		spi_sync(priv->spi, &m);

		retlen = len;//m.actual_length - CMD_SIZE;
		GIGA_PRINTK("[giga] AllLen:%d, act_len:%d, cmd_len:%d, ret_len:%d.\n", len, m.actual_length, CMD_SIZE, retlen);
	}
	else
	{
		u32 i;
		/* the size of data remaining on the first page */
		page_size = FLASH_PAGESIZE - page_offset;

		t[1].len = page_size;
		spi_sync(priv->spi, &m);
		
		retlen = page_size;//m.actual_length - CMD_SIZE;
		GIGA_PRINTK("[giga] StLen:%d, act_len:%d, cmd_len:%d, ret_len:%d.\n", len, m.actual_length, CMD_SIZE, retlen);
		/* write everything in PAGESIZE chunks */
		for (i = page_size; i < len; i += page_size)
		{
			page_size = len - i;
			if (page_size > FLASH_PAGESIZE)
			{
				page_size = FLASH_PAGESIZE;
			}

			wait_till_ready_timeout(priv);

			write_enable(priv);

			/* write the next page to flash */
			priv->command[0] = OPCODE_GIGA_PSR;
			priv->command[1] = 0;
			priv->command[2] = (to + i) >> 8;
			priv->command[3] = (to + i);

			t[1].tx_buf = buf + i;
			t[1].len = page_size;
			
			spi_sync(priv->spi, &m);

			retlen += page_size;//m.actual_length - CMD_SIZE;
			GIGA_PRINTK("[giga] <in loop> act_len:%d, cmd_len:%d, ret_len:%d.\n", m.actual_length, CMD_SIZE, retlen);
		}
	}

	/* Wait until finished previous write command. */
	if (wait_till_ready_timeout(priv))
	{
		//mutex_unlock(&priv->lock);
		pinmux_2_nand(priv);
		GIGA_PRINTK("[giga] err! act_len:%d, cmd_len:%d, ret_len:%d.\n", m.actual_length, CMD_SIZE, retlen);
		kfree(buf);
		return -1;
	}

	//mutex_unlock(&priv->lock);
	pinmux_2_nand(priv);
	kfree(buf);
	return retlen;
}

/****************************************************************************/

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int ali_sflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{	
	struct ali_sflash_private *flash = mtd_to_ali_sflash_private(mtd);
	u32 addr,len;
	
	PRINTK_SFLASH("%s: %s %s 0x%08x, len %d\n",
			dev_name(&flash->spi->dev), __func__, "at",
			(u32)instr->addr, instr->len);

	/* sanity checks */
	if (instr->addr + instr->len > flash->mtd.size)
		return -EINVAL;
	
	/*
	if ((instr->addr % mtd->erasesize) != 0
			|| (instr->len % mtd->erasesize) != 0) {
		return -EINVAL;
	}
	*/
	
	addr = instr->addr;
	len = instr->len;
	if ((addr % mtd->erasesize) != 0 \
		|| (len % mtd->erasesize) != 0)
		 {		
		    return -EINVAL;	
		}

	//mutex_lock(&flash->lock);
	pinmux_2_spi(flash);

	/* whole-chip erase? */
	if (len == flash->mtd.size && erase_chip(flash)) 
	{
		instr->state = MTD_ERASE_FAILED;
		//mutex_unlock(&flash->lock);
		pinmux_2_nand(flash);
		return -EIO;

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K.  We may have set up
	 * to use "small sector erase", but that's not always optimal.
	 */

	/* "sector"-at-a-time erase */
	} 
	else 
	{
		while (len) 
		{
			if (erase_sector(flash, addr)) 
			{
				instr->state = MTD_ERASE_FAILED;
				//mutex_unlock(&flash->lock);
				pinmux_2_nand(flash);
				return -EIO;
			}

			addr += mtd->erasesize;
			len -= mtd->erasesize;
		}
	}

	//mutex_unlock(&flash->lock);
	pinmux_2_nand(flash);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static void sflash_wait_free(void)
{
   UINT32 count =0;
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* Set SFLASH_INS to RDSR instruction */
	write_uint8(SF_INS, 0x05);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
	/* Read from any address of serial flash. */
    for (count = 0; count < MAX_READY_WAIT_COUNT; count++) {
	    if((((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] & 0x01) == 0)
		break;
		udelay(10);
	   	}
	/* Reset sflash to common read mode */
	write_uint16(SF_INS, sflash_default_read);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
}

static void sflash_write_enable(int en)
{
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE);
	/* Set SFLASH_INS to WREN instruction. */
	if (en)
		write_uint8(SF_INS, 0x06);
	else
		write_uint8(SF_INS, 0x04);
	
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
	/* Write to any address of serial flash. */
	((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0] = 0;
    
}
void sflash_write_status(struct ali_sflash_private *flash,UINT16 status)
{
    //MUTEX_ENTER();
    //mutex_lock(&nor_nand_mutex);
	pinmux_2_spi(flash);
	sflash_write_enable(1);

	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* Set SFLASH_INS to RDSR instruction */
	write_uint8(SF_INS, 0x01);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);

	/* write to any address of serial flash. */
	((volatile UINT16 *)SYS_FLASH_BASE_ADDR)[0] = status;

	sflash_wait_free();

	/* Reset sflash to common read mode */
	write_uint16(SF_INS, sflash_default_read);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|sflash_default_mode);
    //mutex_unlock(&nor_nand_mutex);
	pinmux_2_nand(flash);
    //	MUTEX_LEAVE();
}
UINT8 sflash_read_status(struct ali_sflash_private *flash)
{
	UINT8 ucStatus = 0;

	pinmux_2_spi(flash);
    //	MUTEX_ENTER();
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* Set SFLASH_INS to RDSR instruction */
	write_uint8(SF_INS, 0x05);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
	/* Read from any address of serial flash. */

	ucStatus = (((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0]);
    //	MUTEX_LEAVE(); 
	pinmux_2_nand(flash);
    
	return ucStatus;
}
UINT8 sflash_read_status_hb_GD(struct ali_sflash_private *flash)
{
	UINT8 ucStatus = 0;
    //mutex_lock(&nor_nand_mutex);
	pinmux_2_spi(flash);
    //	MUTEX_ENTER();
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* Set SFLASH_INS to RDSR instruction */
	write_uint8(SF_INS, 0x35);
	write_uint8(SF_DUM, (read_uint8(SF_DUM)&0xc0)|STD_MODE);
	/* Read from any address of serial flash. */
    ucStatus = (((volatile UINT8 *)SYS_FLASH_BASE_ADDR)[0]);
    //	MUTEX_LEAVE();
    //mutex_unlock(&nor_nand_mutex);
	pinmux_2_nand(flash);
	return ucStatus;
}
int sflash_erase_otp_sector(struct ali_sflash_private *flash,UINT32 secur_addr)
{
	unsigned long flash_base_addr = SYS_FLASH_BASE_ADDR;
    //mutex_lock(&nor_nand_mutex);
	pinmux_2_spi(flash);
	/*Support M3329E and M3202 SFlash addr mapping in 8M or 16M*/
	if(1)
	{
		flash_base_addr = SYS_FLASH_BASE_ADDR - (secur_addr&0xc00000);
		secur_addr &= 0x3fffff;
	}
	//MUTEX_ENTER();
	sflash_write_enable(1);
	/* Set CODE_HIT and ADDR_HIT to 1, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR);
	/* Set SFLASH_INS to ESR instruction. */
	
	/* Write to typical sector start address of serial flash. */
	write_uint8(SF_INS, 0x44);
	((volatile UINT8 *)flash_base_addr)[secur_addr] = 0;
	sflash_wait_free();
    //mutex_unlock(&nor_nand_mutex);
	pinmux_2_nand(flash);
    // MUTEX_LEAVE();	
	return SUCCESS;
}
int sflash_write_otp_data(struct ali_sflash_private *flash,UINT32 secur_addr, UINT8 *data, UINT32 len)
{
	UINT32 i;
	UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
	UINT32 tge_seg = 0;
	UINT32 cur_seg = 0;
	
	if(!len)
		return SUCCESS;
	if(1)
	{
        //mutex_lock(&nor_nand_mutex);
	    pinmux_2_spi(flash);
        tge_seg = ((secur_addr+len-1)&0xc00000)>>22;
		cur_seg = (secur_addr&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - secur_addr;
			if(SUCCESS!=sflash_write_otp_data(flash,secur_addr, data, inter_seg_len))
					return !SUCCESS;
			secur_addr += inter_seg_len;
			data += inter_seg_len;
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				if(SUCCESS!=sflash_write_otp_data(flash,secur_addr, data, 0x400000))
					return !SUCCESS;
				secur_addr += 0x400000;
				data += 0x400000;
				len -= 0x400000;
				cur_seg++;
			}
			
			return sflash_write_otp_data(flash,secur_addr, data, len);
		}
		else
		{
			flash_base_addr = SYS_FLASH_BASE_ADDR - (secur_addr&0xc00000);
			secur_addr &= 0x3fffff;
		}
	}
    //	MUTEX_ENTER();
    if(1)
	{
		for(i = 0; i < len; i=i+1)
		{
			sflash_write_enable(1);
			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			/* Set SFLASH_INS to PP. */
    			/* Write to typical address of serial flash. */
			write_uint8(SF_INS, 0x42);
			((volatile UINT8 *)flash_base_addr)[secur_addr + i] =*data++;
				//osal_delay(50000);
    		sflash_wait_free();
		}
		//sflash_global_protect(1);
        //mutex_unlock(&nor_nand_mutex);
	    pinmux_2_nand(flash);
	  //	MUTEX_LEAVE();
		return SUCCESS;
	}

}
int sflash_read_otp_data(struct ali_sflash_private *flash,void *buffer, void *flash_addr, UINT32 len)
{
	UINT32 i;
	UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
	UINT32 flash_offset = (UINT32)flash_addr-SYS_FLASH_BASE_ADDR;
	UINT32 cur_seg =0;
	UINT32 tge_seg =0;
	
    if(!len)
		return SUCCESS;
    //mutex_lock(&nor_nand_mutex);
	pinmux_2_spi(flash);
	
	if(1)
	{
		cur_seg = (flash_offset&0xc00000)>>22;
		tge_seg = ((flash_offset+len-1)&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_offset;
			sflash_read_otp_data(flash,buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), inter_seg_len);
			flash_offset += inter_seg_len;
			buffer = (void *)((UINT32)buffer+inter_seg_len);
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				sflash_read_otp_data(flash,buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), 0x400000);
				flash_offset += 0x400000;
				buffer = (void *)((UINT32)buffer+0x400000);
				len -= 0x400000;
				cur_seg++;
			}
			
			return sflash_read_otp_data(flash,buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), len);
		}
		else
		{
			flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_offset&0xc00000);
			flash_offset &= 0x3fffff;
		}	
	}
	flash_addr = (void *)(flash_base_addr+flash_offset);
	//MUTEX_ENTER();
	/* Read data in body align with 1 bytes */
    sflash_default_read = STD_SECURITY_READ;
	write_uint16(SF_INS, sflash_default_read|(SF_CONT_RD<<8));
	/* Read data */
	for (i = 0; i < len; i+=1)
	{
		*(UINT8 *)((UINT32)buffer + i) = *(UINT8 *)((UINT32)flash_addr + i);
	}
	sflash_default_read = STD_READ;
	write_uint16(SF_INS, sflash_default_read);
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
    sflash_wait_free();
    //mutex_unlock(&nor_nand_mutex);
	pinmux_2_nand(flash);
    return 0;
}
UINT8 sflash_set_otp(struct ali_sflash_private *flash,UINT8 ucBlockIndex) //start from 0
{
	UINT16 usStatus = 0;
    usStatus = (sflash_read_status(flash) | (sflash_read_status_hb_GD(flash) << 8));
	usStatus |= (1 << (ucBlockIndex + 11)); //s11 S12 S13 : LB1, LB2, LB3
	sflash_write_status(flash,usStatus);
    return 0;
}

UINT8  sflash_get_otp_status(struct ali_sflash_private *flash,UINT8 ucBlockIndex)
{
   UINT8 ucStatus = 0;
    ucStatus = sflash_read_status_hb_GD(flash);
    ucBlockIndex += 3;
	//bRet = ((ucStatus & (1 << ucBlockIndex)) ? true : false);
    ucStatus=(ucStatus&0x38)>>3;
	return ucStatus;
}
#if 0
static UINT8 get_flash_status_size(void)
{
	UINT8 ucFlashStatusSize = 0;

#if 0
	switch(ucOtpFlashType)
	{
	   
		case OTP_FLASH_EON:
			ucFlashStatusSize = 0; // 1 Byte
			break;
		case OTP_FLASH_GD:
		case OTP_FLASH_WB:
			ucFlashStatusSize = 1; // 2 Byte
			break;
	}
#endif
    ucFlashStatusSize = 1; // 2 Byte
	return ucFlashStatusSize;
}
#endif

 UINT8  flash_otp_operate =0;
 static int ali_sflash_ioctl(struct mtd_info *mtd, u_int cmd, u_long arg)  
 {	
 	//u32 *pt = (u32*)(mtd+1) ;
	//struct mtd_info * master =(struct mtd_info *)(*pt);  // patch for ioctl bug 
 	//struct ali_sflash_private *flash = mtd_to_ali_sflash_private(master);
	
    int ret_code =0;
	UINT8 otp_status=0;
	UINT8 otp_set_value = 0;

 	struct ali_sflash_private *flash = &ali_sflash_priv;
 	PRINTK_SFLASH("cari chen ,arg = %d  %s\n",arg,__FUNCTION__);
	
	switch (cmd)
	{
		case ALI_FLASH_SECTOR_ALIGN:
			PRINTK_SFLASH("1.ALI_FLASH_SECTOR_ALIGN\n");
			ret_code = sto_flash_sector_align(flash, arg);
			return ret_code;
		case ALI_FLASH_SECTOR_START:
			PRINTK_SFLASH("2.ALI_FLASH_SECTOR_START\n");
			ret_code = sto_flash_sector_start(flash, arg);
			return ret_code;
		case ALI_FLASH_SECTOR_SIZE:
			PRINTK_SFLASH("3.ALI_FLASH_SECTOR_SIZE\n");
			ret_code = sto_flash_sector_size(flash, arg);
			return ret_code;
		case ALI_FLASH_SECTOR_OTP_RW:
			flash_otp_operate = 1;
	        ret_code=-ENOIOCTLCMD;
			break;
	    case ALI_FLASH_SECTOR_OTP_ERASE:
	        sflash_erase_otp_sector(flash,0x1000*(arg+1));
	        break;
	    case ALI_FALSH_SECTOR_OTP_GET_STATUS:
	        otp_status= sflash_get_otp_status(flash,arg);
	        copy_to_user( ( unsigned long *)arg  ,&otp_status,sizeof(unsigned char));
	        break;
	    case ALI_FALSH_SECTOR_OTP_SET_STATUS:
	        copy_from_user(&otp_set_value,(unsigned long*)arg,sizeof(UINT8));
	        sflash_set_otp(flash,otp_set_value);
	    	break;
		case ALI_FLASH_CONFIG_CMD_EXT_GROUP:
			PRINTK_SFLASH("4.ALI_FLASH_CONFIG_CMD_EXT_GROUP\n");
			ret_code = ali_sflash_config_cmdext_group(flash, arg);
			return ret_code;
		default:
			ret_code = -ENOIOCTLCMD;
			break;
	}
    //special commands of giga sflash
	if (g_ali_sflash_commands_ext_group & (1<<FLASHCMD_EXT_OFFSETBIT_FOR_GIGA))
	{
		GIGA_PRINTK("[GIGA Kernel ioctl]: %02x\n", cmd);
		switch (cmd)
		{
			case ALI_GIGA_FLASH_READ_CSTMID:
				ret_code = ali_sflash_read_giga_customid(flash, (unsigned char *)arg);
				break;
			case ALI_GIGA_FLASH_ESR:
				ret_code = ali_sflash_erase_giga_as(flash);
				break;
			case ALI_GIGA_FLASH_PSR:
				ret_code = ali_sflash_write_giga_as(flash, (struct giga_as_transfer_user *)arg);
				break;
			case ALI_GIGA_FLASH_RSR:
				ret_code = ali_sflash_read_giga_as(flash, (struct giga_as_transfer_user *)arg);
				break;
			case ALI_GIGA_FLASH_LOCKSR:
				ret_code = ali_sflash_write_giga_srlb_as(flash);
				break;
		}
	}
	return ret_code;
 }


/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int ali_sflash_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct ali_sflash_private *flash = mtd_to_ali_sflash_private(mtd);
	struct spi_transfer t[2];
	struct spi_message m;
	UINT32 buffer ;

	PRINTK_SFLASH("%s: %s %s 0x%08x, len %zd\n",\
			dev_name(&flash->spi->dev), __func__, "from",\
			(u32)from, len);
	pinmux_2_spi(flash);
    
    if((flash_otp_operate==1))
    {
        flash_otp_operate = 0;
        //t[1].rx_buf = buf;
        buffer = (SYS_FLASH_BASE_ADDR + from);
        sflash_read_otp_data(flash,(u_char *)buf, (UINT8 *)buffer, len);
        //memset(buf,0x77,0x63);
        PRINTK_SFLASH("[%s] buf[0x%x]\n",__FUNCTION__,(UINT32)buf);
	    *retlen = len;
        return 0;
    }
	pinmux_2_nand(flash);
	/* sanity checks */
	if (!len)
		return 0;

	if (from + len > flash->mtd.size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	/* NOTE:
	 * OPCODE_FAST_READ (if available) is faster.
	 * Should add 1 byte DUMMY_BYTE.
	 */
	t[0].tx_buf = flash->command;
	t[0].len = CMD_SIZE + FAST_READ_DUMMY_BYTE;
	spi_message_add_tail(&t[0], &m);
	
	t[1].rx_buf = buf;
	t[1].len = len;
	spi_message_add_tail(&t[1], &m);

	/* Byte count starts at zero. */
	if (retlen)
		*retlen = 0;

	//mutex_lock(&flash->lock);
	pinmux_2_spi(flash);

	/* FIXME switch to OPCODE_FAST_READ.  It's required for higher
	 * clocks; and at this writing, every chip this driver handles
	 * supports that opcode.
	 */

	/* Set up the write data buffer. */
	flash->command[0] = OPCODE_READ;   //0x03
	flash->command[1] = from >> 16;    //64
	flash->command[2] = from >> 8;
	flash->command[3] = from;

	spi_sync(flash->spi, &m);

	*retlen = m.actual_length - CMD_SIZE - FAST_READ_DUMMY_BYTE;
	
	/* Wait till previous write/erase is done. */
	if (wait_till_ready_timeout(flash)) {
		/* REVISIT status return?? */
		//mutex_unlock(&flash->lock);
		pinmux_2_nand(flash);
		return 1;
	}
		
	//mutex_unlock(&flash->lock);
	pinmux_2_nand(flash);

	return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int ali_sflash_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct ali_sflash_private *flash = mtd_to_ali_sflash_private(mtd);
	u32 page_offset, page_size;
	struct spi_transfer t[2];
	struct spi_message m;
	
	PRINTK_SFLASH("%s: %s %s 0x%08x, len %zd\n",
			dev_name(&flash->spi->dev), __func__, "to",	(u32)to, len);
	pinmux_2_spi(flash);
    
    if(flash_otp_operate==1)
    {
        flash_otp_operate = 0;
	    // sflash_erase_security_register(0x1000);
	    // sflash_erase_security_register(to);
		sflash_write_otp_data(flash,to,(UINT8*)buf,len);
		*retlen = len;
		return 0;
	}
	pinmux_2_nand(flash);	
/*
	pinmux_2_spi(flash);
	erase_chip(flash);   // cari chen 
	pinmux_2_nand(flash);
*/	
	
	if (retlen)
	{
		*retlen = 0;
	}
	else
	{
		return -EINVAL;
	}
	/* sanity checks */
	if (!len)
	{
		PRINTK_INFO("%s, err line %d, ret_len:%d.\n", __FUNCTION__, __LINE__, *retlen);

		return(0);
	}

	if (to + len > flash->mtd.size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = CMD_SIZE;
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	spi_message_add_tail(&t[1], &m);

	//mutex_lock(&flash->lock);
	pinmux_2_spi(flash);

	write_enable(flash);

	/* Set up the opcode in the write buffer. */
	flash->command[0] = OPCODE_PP;  //  0x02
	flash->command[1] = to >> 16;
	flash->command[2] = to >> 8;
	flash->command[3] = to;

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= FLASH_PAGESIZE) 
	{
		t[1].len = len;  //64
		
		PRINTK_SFLASH("page_offset + len <= FLASH_PAGESIZE");
		
		spi_sync(flash->spi, &m);

		*retlen = len;//m.actual_length - CMD_SIZE;
		PRINTK_INFO("%s, line %d, len:%d, act_len:%d, cmd_len:%d, ret_len:%d.\n",\
			__FUNCTION__, __LINE__, len, m.actual_length, CMD_SIZE, *retlen);
	} 
	else 
	{
		u32 i;

		/* the size of data remaining on the first page */
		page_size = FLASH_PAGESIZE - page_offset;

		t[1].len = page_size;
		spi_sync(flash->spi, &m);
		
		*retlen = page_size;//m.actual_length - CMD_SIZE;
		
		PRINTK_INFO("%s, line %d, len:%d, act_len:%d, cmd_len:%d, ret_len:%d.\n",\
			__FUNCTION__, __LINE__, len, m.actual_length, CMD_SIZE, *retlen);
		
		/* write everything in PAGESIZE chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > FLASH_PAGESIZE)
				page_size = FLASH_PAGESIZE;

			wait_till_ready_timeout(flash);

			write_enable(flash);

			/* write the next page to flash */
			flash->command[0] = OPCODE_PP;
			flash->command[1] = (to + i) >> 16;
			flash->command[2] = (to + i) >> 8;
			flash->command[3] = (to + i);

			t[1].tx_buf = buf + i;
			t[1].len = page_size;
			
			spi_sync(flash->spi, &m);

			if (retlen)
				*retlen += page_size;//m.actual_length - CMD_SIZE;
			PRINTK_INFO("%s, line %d, len:%d, act_len:%d, cmd_len:%d, ret_len:%d.\n",\
				__FUNCTION__, __LINE__, len, m.actual_length, CMD_SIZE, *retlen);
		}
	}

	/* Wait until finished previous write command. */
	if (wait_till_ready_timeout(flash)) 
	{
		//mutex_unlock(&flash->lock);
		pinmux_2_nand(flash);
		
		PRINTK_SFLASH("%s, err line %d, act_len:%d, cmd_len:%d, ret_len:%d.\n",\
			__FUNCTION__, __LINE__, m.actual_length, CMD_SIZE, *retlen);
		
		return 1;
	}
	
	//mutex_unlock(&flash->lock);
	pinmux_2_nand(flash);
	//PRINTK_INFO("%s, success. act_len:%d, cmd_len:%d, ret_len:%d.\n",__FUNCTION__,m.actual_length,CMD_SIZE, *retlen);

	return 0;
}

/* obtain flash_sectors , flash_size */
INT32 ali_sflash_identify(struct ali_sflash_private *priv, struct spi_device *spi)
{
	UINT16 s,i;
	UINT32 id, id_buf[3];
	UINT8 flash_did;
	UINT8 flash_identified = false;   // 0
	UINT8 code;
	PRINTK_INFO("%s.\n", __FUNCTION__);

	
	//for (j = 0; j < 4; j++)
	{
		id_buf[0]=id_buf[1]=id_buf[2]=0xffffffff;
		/*
		In the mode, one Master STB will upgrade many Slave STBs
		in one time. So we identify local flash and assume that
		all Slave STBs have the same type of flashes. Also, we
		should inform remote_flash driver of the correct flash
		command address.
		*/
			
		code = OPCODE_JEDEC_ID;  //0x9f
		//spi_write_then_read(spi, &code, 1, (&id_buf[0]), 4);  //(UINT8 *)
		ali_spi_write_then_read(spi, &code, 1, (&id_buf[0]), 4);
			
		code = OPCODE_DEV_ID;  //0xab
		//spi_write_then_read(spi, &code, 1, (&id_buf[1]), 4);
		ali_spi_write_then_read(spi, &code, 1, (&id_buf[1]), 4);
			
		code = OPCODE_M_D_ID;  //0x90
		//spi_write_then_read(spi, &code, 1, (&id_buf[2]), 4);
		ali_spi_write_then_read(spi, &code, 1, (&id_buf[2]), 4);
			
		PRINTK_SFLASH("\n id_buf[0] = 0x%08x\n id_buf[1] = 0x%08x\n id_buf[2] = 0x%08x.\n\n",id_buf[0],id_buf[1],id_buf[2]);

		// (id_buf[0]&0xffff) == 0x20c2
		// id_buf[0] = 0xc21620c2
 		// id_buf[1] = 0x15000000
 		// id_buf[2] = 0xc2000000
 		
		if ((!id_buf[0] && !id_buf[1] && !id_buf[2]) || ((id_buf[0] == 0xffffffff) && \
            (id_buf[1] == 0xffffffff) && (id_buf[2] == 0xffffffff))){
            flash_identified = true;
            goto exit_func;
        }
		
		/*if(0xffffffff==id_buf[0]&&0xffffffff==id_buf[1]&&0xffffffff==id_buf[2])
		{
			//PRINTF("flash ID invalid\n");
			continue;
		}*/

		//PRINTK_SFLASH("priv->flash_deviceid_num = %d\n\n",priv->flash_deviceid_num);
		for (i = 0; i < (priv->flash_deviceid_num)*2; i += 2)   // 37*2
		{
			s = (priv->flash_deviceid)[i + 1];
			id = id_buf[s >> 5];
			
			//PRINTK_SFLASH("s = 0x%08x",s);
			//PRINTK_SFLASH("s >> 5 = 0x%08x",(s >> 5));
			//PRINTK_SFLASH("id = 0x%08x",id);
			
			s &= 0x1F;
			flash_did = (priv->flash_deviceid)[i];
			
			//PRINTK_SFLASH("flash_did = 0x%08x",flash_did);
			//PRINTK_SFLASH("((id >> s) & 0xFF) = 0x%08x",((id >> s) & 0xFF));
			
			if (((id >> s) & 0xFF) == flash_did)
			{
				priv->flash_id = (priv->flash_id_p)[i >> 1];
				//PRINTK_SFLASH("i >> 1 = %d",i >> 1);
				PRINTK_SFLASH("priv->flash_id = %d",priv->flash_id);
				
				break;
			}
		}
		if (i < (priv->flash_deviceid_num)*2)
		{
			priv->flash_sectors = (unsigned int)(ali_flash_sectors[priv->flash_id]);
			
			PRINTK_SFLASH("flash_sectors = %d\n",priv->flash_sectors);
			
			priv->flash_size = sto_flash_sector_start(priv, priv->flash_sectors);
			
			PRINTK_SFLASH("flash_size = 0x%08x\n",priv->flash_size);
			
			//tp->flash_cmdaddr = tflash_cmdaddr[j];
			flash_identified = true;
		}
	}
		
exit_func:
	
	return flash_identified;
}

/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int __devinit ali_sflash_probe(struct spi_device *spi)
{
	//struct ali_flash_platform_data *data;
	struct flash_platform_data *data;
	struct ali_sflash_private  *flash;
	//struct mtd_part_parser_data	ppdata;  // cari chen
	
	INT32 result;
	UINT32 i;
		
	PRINTK_INFO("ENTER : %s.\n", __FUNCTION__);
		
	/* Platform data helps sort out which chip type we have, as
	 * well as how this board partitions it.  If we don't have
	 * a chip ID, try the JEDEC id commands; they'll work for most
	 * newer chips, even if we don't recognize the particular chip.
	 */
	flash = &ali_sflash_priv;
	flash->flash_deviceid     = ali_sflash_deviceid_table;
	flash->flash_id_p         = ali_sflash_sector_struct_idx;
	flash->flash_deviceid_num = ali_sflash_deviceid_num;
	//flash->spi = spi;    // cari chen
	data = spi->dev.platform_data;
	
	pinmux_2_spi(flash);
	
	result = ali_sflash_identify(flash, spi);
		
	pinmux_2_nand(flash);
	
	if (!result)
	{
		printk("\nUnknown flash type.\n\n");
		return -ENODEV;
	}

	flash->spi = spi;
	mutex_init(&flash->lock);
	dev_set_drvdata(&spi->dev, flash);
#if 0
	/*
	 * Atmel serial flash tend to power up
	 * with the software protection bits set
	 */

	if (info->jedec_id >> 16 == 0x1f) {
		write_enable(flash);
		write_sr(flash, 0);
	}
#endif
	if (data && data->name)
	{
		flash->mtd.name = data->name;
	}
	else
	{
		flash->mtd.name = dev_name(&spi->dev);
		//cari modified ,dev.bus_id --> dev.init_name
		//cari modified ,dev.bus_id --> dev_name(&spi->dev)
	}
	
	flash->mtd.type       = MTD_NORFLASH;
	flash->mtd.writesize  = 1;
	flash->mtd.flags      = MTD_CAP_NORFLASH;
	flash->mtd.size       = flash->flash_size;
	flash->mtd._erase      = ali_sflash_erase;
	flash->mtd._read       = ali_sflash_read;
	flash->mtd._write      = ali_sflash_write;
	flash->mtd.ioctl_priv = ali_sflash_ioctl;

	/* prefer "small sector" erase if possible */
	if (0) //info->flags & SECT_4K)
	{
		flash->erase_opcode = OPCODE_BE_4K;
		flash->mtd.erasesize = 4096;
	}
	else
	{
		flash->erase_opcode  = OPCODE_SE;
		flash->mtd.erasesize = 65536; //info->sector_size;
	}
		
	/* partitions should match sector boundaries; and it may be good to
	 * use readonly partitions for writeprotected sectors (BP2..BP0).
	 */
	 
	if (mtd_has_partitions())
	{
		//struct ali_mtd_partition *parts = NULL;
		struct mtd_partition *parts = NULL;
		int	nr_parts = 0;

#ifdef CONFIG_MTD_CMDLINE_PARTS
		static const char *part_probes[] = { "cmdlinepart", NULL, };
		static const char *name = "ali_sflash";
		nr_parts = parse_mtd_partitions(&flash->mtd,part_probes, &parts, (struct mtd_part_parser_data *)name);
#endif
		
		if (nr_parts <= 0 && data && data->parts)
		{
			parts = data->parts;
			nr_parts = data->nr_parts;
		}

		//set parts size = whole flash size if nr_parts = 1
		if (1 == nr_parts)
		{
			parts->size = flash->flash_size;
			parts->offset = 0;
			
		}
		// 添加新分区
		if (nr_parts > 0) 
		{
			for (i = 0; i < nr_parts; i++) 
			{
				PRINTK_SFLASH("partitions[%d] = ""{.name = %s, .offset = 0x%08x, "".size = 0x%08x }\n\n\n",
					i, parts[i].name,parts[i].offset,parts[i].size);
			}
			flash->partitioned = 1;
			return add_mtd_partitions(&flash->mtd, parts, nr_parts);
		}
	}
	else if (data->nr_parts)
	{
		dev_warn(&spi->dev, "ignoring %d default partitions on %s\n",
				data->nr_parts, data->name);
	}
	
	return add_mtd_device(&flash->mtd) == 1 ? -ENODEV : 0;  // cari chen
}

static int __devexit ali_sflash_remove(struct spi_device *spi)
{
	struct ali_sflash_private	*flash = dev_get_drvdata(&spi->dev);
	int		status;

	/* Clean up MTD stuff. */
	if (mtd_has_partitions() && flash->partitioned)
		status = del_mtd_partitions(&flash->mtd);
	else
		status = del_mtd_device(&flash->mtd);
	if (status == 0)
		kfree(flash);
	return 0;
}

/*****************************************************************************/

/* ali_sflash_driver */
static struct spi_driver ali_sflash_driver = {
	.driver = {
		.name	= "ali_sflash",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe	= ali_sflash_probe,
	.remove	= __devexit_p(ali_sflash_remove),

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};

/*
static struct mtd_partition ali_spi_flash_partitions[] = {
	{
		.name = "bootloader(spi)",
		.size = 0x00040000,
		.offset = 0,
	}, 
	
	{
		.name = "linux kernel(spi)",
		.size = 0xe0000,
		.offset = 0x20000
	},
    {
		.name = "file system(spi)",
		.size = 0x100000,
		.offset = 0x00100000,
	}
};
*/

/* cari chen add , 内存映射情况结构体 */
/*
static struct map_info ali_sflash_map ={
	.name = sflash,
	.size = 0x400000,
	.bankwidth = 1,
	.phys = SYS_FLASH_BASE_ADDR ,
};
*/

/* 定义 norflash 中的分区 */

static struct mtd_partition ali_spi_flash_partitions[] = {
	{
		.name = "sflash",       // name : sflash
		.size = 0x400000,       // size : 4M
		.offset = 0,
	}
};

//static struct ali_flash_platform_data ali_spi_flash_data = {
static struct flash_platform_data ali_spi_flash_data = {
	.name = "ali_sflash",
	.parts = ali_spi_flash_partitions,
	.nr_parts = ARRAY_SIZE(ali_spi_flash_partitions),
	.type = "ali_spi_flash",
};

static struct spi_board_info ali_spi_board_info __initdata = {
		/* the modalias must be the same as spi device driver name */
		.modalias = "ali_sflash", /* Name of spi_driver for this device */
		.max_speed_hz = 25000000,     /* max spi clock (SCK) speed in HZ */
		.bus_num = 1, /* Framework bus number */
		.chip_select = 1, /* Framework chip select. On STAMP537 it is SPISSEL1*/
		.platform_data = &ali_spi_flash_data,
		.controller_data = &ali_sflash_priv,
		.mode = SPI_MODE_0,
};

/*************************************************************************/

static int ali_sflash_init(void)
{	
    g_chipId = ali_sys_ic_get_chip_id();
	//printk("[core] ali_sflash_init chipid=%x\n", g_chipId);
	spi_register_board_info(&ali_spi_board_info, 1);

	return spi_register_driver(&ali_sflash_driver);
}

static void ali_sflash_exit(void)
{
	spi_unregister_driver(&ali_sflash_driver);
}


module_init(ali_sflash_init);
module_exit(ali_sflash_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Li");
MODULE_DESCRIPTION("Ali flash controller and serial flash chips driver");

