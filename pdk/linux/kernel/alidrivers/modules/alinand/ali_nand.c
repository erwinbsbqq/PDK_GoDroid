/*
 * Copyright 2014 Ali Corporation Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/leds.h>
#include <asm/io.h>
#include <linux/version.h>
#if defined(CONFIG_ALI_CHIP_M3921)
	#include <mach/ali-s3921.h>
#endif
#include <ali_reg.h>
#include <ali_board_config.h>
#include "ali_nand.h"

#if defined(CONFIG_ALI_CHIP_M3921)
extern struct mutex ali_sto_mutex;
#endif

/* wait HW DMA completion */
struct completion dma_completion;
u8 micron_l73 = 0;
u8 micron_l74 = 0;
u8 micron_l83 = 0;
u8 micron_l84 = 0;

struct boot_area_nand_parameter *boot_area_parm = NULL;
static const char nandname[] = "alidev_nand_reg";
static u8 bbt_pattern[] = {'B', 'b', 't', '0' };
static u8 mirror_pattern[] = {'1', 't', 'b', 'B' };
static u8 scan_ff_pattern[] = { 0xff, 0xff };
static u8 pattern_3701[4] = {0x37, 0x01, 0x55, 0xAA,};
static u8 pattern_3503[4] = {0x35, 0x03, 0x55, 0xAA,};
static u8 pattern_3821[4] = {0x38, 0x21, 0x55, 0xAA,};
static u8 pattern_3921[4] = {0x39, 0x21, 0x55, 0xAA,};

u8 nf_3921_gpio_num[]={93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 0};

// PMI ecc pattern base on MPTool
static u8 ECC_pattern_16b[6]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static u8 ECC_pattern_24b[6]={0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

static struct nand_ecclayout ali_nand_oob_32 = {
	.eccbytes = 28,
	.eccpos = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			   19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31},
	.oobfree = {
		{.offset = 0,
		 .length = 4}}
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

struct PMI_descr{
	u8 maxblocks;
	u32 pos[4];
	u8 *pattern;
};

enum{
	ECC_16=1,
	ECC_24=2,
	ECC_40=3,
	ECC_48=4,
	ECC_60=5,
};

enum{
	PAGE_16KB=0,
	PAGE_8KB=1,
	PAGE_4KB=2,
	PAGE_2KB=3,
};

enum{
	BLOCK_PAGES_64=0,
	BLOCK_PAGES_128=1,
	BLOCK_PAGES_192=2,
	BLOCK_PAGES_256=3,
};

static void nf_cmd(struct mtd_info *mtd, unsigned int command, int column, int page_addr);

#ifdef CONFIG_MTD_CMDLINE_PARTS
	extern int parse_mtd_partitions(struct mtd_info *master, const char **types, struct mtd_partition **pparts, struct mtd_part_parser_data *data);
#endif

struct mtd_partition ali_nand_partitions[] = {
	{ .name = "ALI-Private    ",    .offset = 0,    .size = 0,    .mask_flags = MTD_WRITEABLE,},/* force read-only */     
	{ .name = "Partition1     ",    .offset = 0,    .size = 0,    .mask_flags = 0,  }, /*partition name max length: 15bytes*/
	{ .name = "Partition2     ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition3     ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition4     ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition5     ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition6     ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition7     ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition8     ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition9     ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition10    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition11    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition12    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition13    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition14    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition15    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition16    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition17    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition18    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition19    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition20    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition21    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition22    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition23    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition24    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition25    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition26    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition27    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition28    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition29    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
	{ .name = "Partition30    ",    .offset = 0,    .size = 0,    .mask_flags = 0,  },
};

static inline struct ali_nand_host *to_ali_nand_host(struct mtd_info *mtd)
{
	return container_of((void *) mtd, struct ali_nand_host, mtd);
}

static u8 nfreg_read8(struct mtd_info *mtd, u32 reg)
{													
struct ali_nand_host *host = to_ali_nand_host(mtd);	
	return	readb(host->regs + reg);
}	

static void nfreg_write8(struct mtd_info *mtd, u8 val, u32 reg)	
{													
struct ali_nand_host *host = to_ali_nand_host(mtd);	
	writeb(val, host->regs + reg);
}	
		
static u32 nfreg_read32(struct mtd_info *mtd, u32 reg)
{													
struct ali_nand_host *host = to_ali_nand_host(mtd);	
	return readl(host->regs + reg);
}	

static void nfreg_write32(struct mtd_info *mtd, u32 val, u32 reg)
{													
struct ali_nand_host *host = to_ali_nand_host(mtd);	
	writel(val, host->regs + reg);
}			

void ali_nand_reg_dump(struct mtd_info *mtd)
{
int i;

	printk( "SOC reg:\n");
	printk( " reg060 0x%08x\n", (int) ali_soc_read(0x60));
	printk( " reg070 0x%08x\n", (int) ali_soc_read(0x70));
	printk( " reg074 0x%08x\n", (int) ali_soc_read(0x74));
	printk( " reg07c 0x%08x\n", (int) ali_soc_read(0x7c));
	printk( " reg084 0x%08x\n", (int) ali_soc_read(0x84));
	printk( " reg088 0x%08x\n", (int) ali_soc_read(0x88));
	printk( " reg08c 0x%08x\n", (int) ali_soc_read(0x8c));
	printk( " reg090 0x%08x\n", (int) ali_soc_read(0x90));
	printk( " reg0d4 0x%08x\n", (int) ali_soc_read(0xa4));
	printk( " reg0a4 0x%08x\n", (int) ali_soc_read(0xd4));
	printk( " reg0d8 0x%08x\n", (int) ali_soc_read(0xd8));
	printk( " reg434 0x%08x\n",(int) ali_soc_read(0x434));
	printk( " reg434 0x%08x\n",(int) ali_soc_read(0x434));
	printk( " reg490 0x%08x\n",(int) ali_soc_read(0x490));

	printk( "NAND reg:");
	for (i=0; i<ALI_NAND_REG_LEN; i++)
	{
		if ((i % 16) == 0)
			printk("\n");
		if((i>=0x48) && (i<0x4c))
			printk(" 0xXX");
		else if (i == 0x18)
			printk(" 0xXX");
		else
			printk(" 0x%02x", nfreg_read8(mtd, i));
	}
	printk("\n 0x18 = 0x%02x\n", nfreg_read8(mtd, 0x18));
}

static void nf_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
    if (0 == (ctrl & NAND_NCE))
    {
        nfreg_write8(mtd, NF_CEJ, NF_bCTRL);
        return;
    }
	
    if (ctrl & NAND_CTRL_CHANGE)
    {
		if (ctrl & NAND_CLE)
		{
			nfreg_write8(mtd, NF_CLE, NF_bCTRL);
		}
		else if (ctrl & NAND_ALE)
		{
		    nfreg_write8(mtd, NF_ALE, NF_bCTRL);
		}
		else
		{
      		nfreg_write8(mtd, 0, NF_bCTRL);
		}
	}
		
    if (NAND_CMD_NONE != cmd)
	{
		nfreg_write8(mtd,  (u8) cmd, NF_bPIODATA);//write command to register 0x1c
	}
}

static void ali_nand_pinmux_release(struct mtd_info *mtd, unsigned int callline)
{
	struct ali_nand_host *host = to_ali_nand_host(mtd);
	long nand_wp_gpio = -1;
	struct ali_nand_platform_data {
		int *nand_wp_gpio;
	} *platform_data = host->dev->platform_data;
	
	if(platform_data != NULL)
	{
		nand_wp_gpio = *(platform_data->nand_wp_gpio);
	}
	
	if (host->pin_mux_lock_flag < 1)
	{
		return;
	}

 	switch(host->chip_id)
	{
		case C3921:
			ali_soc_write((ali_soc_read(C3921_SOC_PINMUX_REG1) & (~C3921_NF_PIN_SEL)), C3921_SOC_PINMUX_REG1);			
			break;
	    	
		case C3503:
			/* bit 9,8
			 00 : 292 BGA
			 01 : 256 QFP
			 10 : 128 QFP
			 11 : 144 QFP
			*/			
			switch(host->chip_pkg & 0x03)
			{
				case 0x00: //BGA 292 Pin package
					if(host->chip_ver == 2)
					{
						ali_soc_write((ali_soc_read(C3503_SOC_PINMUX_REG3) & ~(PK292_NAGRA_SEL)), C3503_SOC_PINMUX_REG3);
					}
					else
					{
						ali_soc_write((ali_soc_read(C3503_SOC_PINMUX_REG3) & ~(NF_FUNC_EN)), C3503_SOC_PINMUX_REG3);
					}
					if(nand_wp_gpio > 0)
					{						
						gpio_direction_output(nand_wp_gpio, 0);
					}
					break;
					
				case 0x01: // 256 LQFP package
				case 0x02: // 128 LQFP package
	      case 0x03: // 144 LQFP package
					ali_soc_write((ali_soc_read(C3503_SOC_PINMUX_REG3) & (~NF_FUNC_EN)), C3503_SOC_PINMUX_REG3);
					break;
					
				default:
					printk("[ERR] chip package N/A");
					break;
			}
			break;
					
		case C3821: 
			/*
				bit 12,11
				00 : QFP 256/ BGA380
				01 : QFP 128
				10 : QFP 156
				11 : QPF 128 (DDR SIP)
			*/
			switch((host->chip_pkg >> 3) & 0x03)
			{
				case 0x00: //QFP256/BGA380 package
					// pinmux setting.
					ali_soc_write((ali_soc_read(C3821_SOC_PINMUX_REG3) & (~BGA_SEL_NF)), C3821_SOC_PINMUX_REG3);					
					break;
				case 0x02: //QFP156PIN package									
					ali_soc_write((ali_soc_read(C3821_SOC_PINMUX_REG3) & (~QFP_SEL_NF)), C3821_SOC_PINMUX_REG3);											
					break;
				default: 
					break;
			}
			break;
			
		case C3701:		
			switch(host->chip_pkg & 0x3)
			{
				case 00:/* 376 pin */
				case 01:/* 256 pin */
				case M3701H://02:/* 256 pin */					/* WP use XGPIO41 */
					gpio_direction_output(C3701_BGA_WP_XGPIO, 0);
					break;								

				case M3701C://03:/* QAM 128 pin */				/* enable 128 pin NF select */
					ali_soc_write((ali_soc_read(C3701_SOC_PINMUX_REG1) & (~NF128_SEL)), C3701_SOC_PINMUX_REG1);					
					break;

				case M3701NMP://04:/* NMP 128 pin */				/* enable 128 pin NF select */
					ali_soc_write((ali_soc_read(C3701_SOC_PINMUX_REG1) & (~NF128_SEL)), C3701_SOC_PINMUX_REG1);					
					/* WP use XGPIO14, not HW */
					gpio_direction_output(C3701_QFP_WP_XGPIO, 0);
					break;

				default:
					break;
			}
			break;	
			
		default:
			break;
	}
	host->pin_mux_lock_flag--;
#if defined(CONFIG_ALI_CHIP_M3921)
	mutex_unlock(&ali_sto_mutex);
#endif
}

void ali_nand_pinmux_set(struct mtd_info *mtd, unsigned int callline)
{
	struct ali_nand_host *host = to_ali_nand_host(mtd);
	long nand_wp_gpio = -1;
	int i = 0;
	
	struct ali_nand_platform_data {
		int *nand_wp_gpio;
	} *platform_data = host->dev->platform_data;
	
	if(platform_data != NULL)
	{
		nand_wp_gpio = *(platform_data->nand_wp_gpio);
	}
	
#if defined(CONFIG_ALI_CHIP_M3921)
	mutex_lock(&ali_sto_mutex);
#endif
	host->pin_mux_lock_flag++;
	 
 	switch(host->chip_id)
	{
		case C3921:						
			i = 0;
			while(1)
			{
				if (!nf_3921_gpio_num[i])
					break;
				gpio_disable_pin(nf_3921_gpio_num[i]);
				i++;	
			}					
			ali_soc_write((ali_soc_read(C3921_SOC_PINMUX_REG2) & 
				(~(C3921_STRAPIN_SEL_ENABLE | C3921_EMMC_SEL | C3921_EMMC_SEL2))), 
				C3921_SOC_PINMUX_REG2);
			ali_soc_write((ali_soc_read(C3921_SOC_PINMUX_REG1) & 
				(~ (C3921_SF_PIN_SEL | C3921_SDIO_SEL))), 
				C3921_SOC_PINMUX_REG1);	
			ali_soc_write((ali_soc_read(C3921_SOC_PINMUX_REG1) | C3921_NF_PIN_SEL), 
				C3921_SOC_PINMUX_REG1);			
			break;
		
		case C3503:			
			ali_soc_write((ali_soc_read(SOC_STRAP_PIN_CTRL_REG) | NF_BOOT_EN | NF_BOOT_VAL), SOC_STRAP_PIN_CTRL_REG);
			ali_soc_write((ali_soc_read(C3503_SOC_PINMUX_REG1) & ~(C3503_STRAP_PIN | C3503_SEL_SFALSH | C3503_SEL_256_SQI)), 
					C3503_SOC_PINMUX_REG1);
			ali_soc_write((ali_soc_read(C3503_SOC_PINMUX_REG2) & ~(NF_CE0_CE1 | C3503_SEL_EMMC)), C3503_SOC_PINMUX_REG2);
			/* bit 9,8
			 00 : 292 BGA
			 01 : 256 QFP
			 10 : 128 QFP
			 11 : 144 QFP
			*/			
			switch(host->chip_pkg & 0x3)
			{
				case 0x00: //BGA 292 Pin package
					if(host->chip_ver == 2)
					{
						ali_soc_write((ali_soc_read(C3503_SOC_PINMUX_REG3) | PK292_NAGRA_SEL), C3503_SOC_PINMUX_REG3);
					}
					else
					{
						ali_soc_write((ali_soc_read(C3503_SOC_PINMUX_REG3) | NF_FUNC_EN), C3503_SOC_PINMUX_REG3);
					}
					//write protect disable XGPIO[56]
					if(nand_wp_gpio > 0)
					{
						gpio_enable_pin(nand_wp_gpio);
						gpio_direction_output(nand_wp_gpio, 1);
					}
					break;					
				case 0x01: // 256 LQFP package
				case 0x02: // 128 LQFP package
	      case 0x03: // 144 LQFP package
					ali_soc_write((ali_soc_read(C3503_SOC_PINMUX_REG3) | NF_FUNC_EN), C3503_SOC_PINMUX_REG3);
					break;					
				default:
					printk("[ERR] chip package N/A");
					break;
			}
			break;
		
		case C3821:
			ali_soc_write((ali_soc_read(SOC_STRAP_PIN_CTRL_REG) | NF_BOOT_EN | NF_BOOT_VAL), SOC_STRAP_PIN_CTRL_REG);
			ali_soc_write((ali_soc_read(C3821_SOC_SYSTEM_CLOCK) & ~SEL_74M_54M), C3821_SOC_SYSTEM_CLOCK);			
			/*
				bit 12,11
				00 : QFP 256/ BGA380
				01 : QFP 128
				10 : QFP 156
				11 : QPF 128 (DDR SIP)
			*/
			switch((host->chip_pkg >> 3) & 0x03)
			{
				case 0x00: //QFP256/BGA380 package
					// pinmux setting.
					ali_soc_write((ali_soc_read(C3821_SOC_PINMUX_REG3) | BGA_NF_SDIO_STRAP_PIN | BGA_SEL_NF),
							 C3821_SOC_PINMUX_REG3);					
					ali_soc_write((ali_soc_read(C3821_SOC_PINMUX_REG3) & ~(QFP_156_SEL_CI | NF_256_PKT_CS)), C3821_SOC_PINMUX_REG3);					
					break;
				case 0x02: //QFP156PIN package									
					ali_soc_write((ali_soc_read(C3821_SOC_PINMUX_REG3) | QFP_SF_NF_STRAP_PIN | QFP_SEL_NF), C3821_SOC_PINMUX_REG3);						
					ali_soc_write((ali_soc_read(C3821_SOC_PINMUX_REG3) & ~(QFP_156_SEL_CI)), C3821_SOC_PINMUX_REG3);									
					break;
				default: 
					break;
			}
			break;
			
		case C3701:
			ali_soc_write((ali_soc_read(SOC_STRAP_PIN_CTRL_REG) | NF_BOOT_EN | NF_BOOT_VAL), SOC_STRAP_PIN_CTRL_REG);
			switch(host->chip_pkg & 0x03)
			{
				case 00:/* 376 pin */
				case 01:/* 256 pin */
				case M3701H://02:/* 256 pin */					/* WP use XGPIO41 */
					gpio_enable_pin(C3701_BGA_WP_XGPIO);
					gpio_direction_output(C3701_BGA_WP_XGPIO, 1);
					break;

				case M3701C://03:/* QAM 128 pin */				/* enable 128 pin NF select */
					ali_soc_write((ali_soc_read(C3701_SOC_PINMUX_REG1) | NF128_SEL), C3701_SOC_PINMUX_REG1);					
					break;

				case M3701NMP://04:/* NMP 128 pin */				/* enable 128 pin NF select */
					ali_soc_write((ali_soc_read(C3701_SOC_PINMUX_REG1) | NF128_SEL), C3701_SOC_PINMUX_REG1);

					/* WP use XGPIO14, not HW */
					gpio_enable_pin(C3701_QFP_WP_XGPIO);
					gpio_direction_output(C3701_QFP_WP_XGPIO, 1);
					break;

				default:
					break;
			}
			break;	
			
		default:
			printk("[ERR] chip id N/A");
			break;
	}
}

static void ali_nand_select_chip(struct mtd_info *mtd, int chips)
{
	switch (chips)
	{
		case 0:
			ali_nand_pinmux_set(mtd, __LINE__);
			break;
		case 1:
			break;
		case -1:
		case 2:
		case 3:
		default:
			ali_nand_pinmux_release(mtd, __LINE__);
			break;
	}		
}
static void nf_wait_ready(struct mtd_info *mtd)
{
    nf_cmd(mtd, NAND_CMD_STATUS, -1, -1);
    while(1)
    {
        udelay(1);
        if(nfreg_read8(mtd, NF_bPIODATA) & NAND_STATUS_READY)
            break;
    };
}

static void nf_cmd(struct mtd_info *mtd, unsigned int command, int column, int page_addr)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);       
unsigned long timen;

    timen = 5000;//timeo + DELAY_1MS;

    /* Command latch cycle */
    nf_ctrl(mtd, command, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
    if ((column != -1) || (page_addr != -1))
    {
        int ctrl = NAND_CTRL_CHANGE | NAND_NCE | NAND_ALE;
        /* not support 512 page nand */
        if (column != -1) 
        {
            nf_ctrl(mtd, column, ctrl);            
            ctrl &= ~NAND_CTRL_CHANGE;
            nf_ctrl(mtd, column >> 8, ctrl);
        }
        if (page_addr != -1) 
        {
            if (192 == host->nf_parm.pages_perblock)
			{
				page_addr = (page_addr / 192) * 256 + (page_addr % 192);
			}
            nf_ctrl(mtd, page_addr, ctrl);
            nf_ctrl(mtd, page_addr>>8, ctrl);
            if (host->nf_parm.rowaddr_cycle >= 3)
			{
                nf_ctrl(mtd, page_addr>>16, ctrl);
			}
            if (host->nf_parm.rowaddr_cycle == 4)
            {
				nf_ctrl(mtd, page_addr>>24, ctrl);
			}
        }
    }
    nf_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
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
            nf_ctrl(mtd, NAND_CMD_STATUS,
                           NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
            nf_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
            while(timen--)//(timen < timeo)// (time_before(jiffies, timeo))
            {
                if (nfreg_read8(mtd, NF_bPIODATA) & NAND_STATUS_READY)
                    break;
                udelay(1000);//timeo = boot_gettime();        
            }
            return;
        case NAND_CMD_READ0:
            nf_ctrl(mtd, NAND_CMD_READSTART,
                           NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
            nf_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
            //nf_ctrl(mtd, mtd, chip);
			nf_wait_ready(mtd);
            nf_ctrl(mtd, NAND_CMD_READ0, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
            nf_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
			return;
    }
}

static void nf_set_seed(struct mtd_info *mtd, u32 sec)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);    
u8 tmp;

    if(host->nf_parm.data_scramble)
    {
        tmp = nfreg_read8(mtd, NF_bEDORBCYC);
        tmp &= ~0x70;
        nfreg_write8(mtd, tmp, NF_bEDORBCYC);
        if(sec % host->nf_parm.eccsec_perpage)
        {
            tmp |= ((sec % host->nf_parm.eccsec_perpage) << 4);
            nfreg_write8(mtd, tmp, NF_bEDORBCYC);
        } 
    }
}

static void nf_host_init(struct mtd_info *mtd)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);   	
u8 tmp;

	nfreg_write8(mtd, NF_EN, NF_bMODE);
	nfreg_write8(mtd, 0x00, NF_bDMACTRL);
	nfreg_write32(mtd, 0x00, NF_dwINTFLAG);
	
	if(host->nf_parm.data_scramble)
	{        
		tmp = nfreg_read8(mtd, NF_bEDORBCYC);
		tmp |= NF_CRYPTO_EN;
		nfreg_write8(mtd, tmp, NF_bEDORBCYC);
	}
	else
	{
		tmp = nfreg_read8(mtd, NF_bEDORBCYC);
		tmp &= ~NF_CRYPTO_EN;
		nfreg_write8(mtd, tmp, NF_bEDORBCYC);
	}	

	switch (host->nf_parm.ecctype)
	{
		case NF_BCH_16B_MODE: //ECC_16:
		    nfreg_write8(mtd, NF_ECC_EN | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write8(mtd, NF_FW_RED_4, NF_bDMALEN);
		    break;
		case NF_BCH_24B_MODE://ECC_24:
		    nfreg_write8(mtd, NF_ECC_EN | NF_BCH_24B_MODE | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write8(mtd, NF_FW_RED_4, NF_bDMALEN);
		    break;
		case NF_BCH_40B_MODE://ECC_40:
		    nfreg_write8(mtd, NF_ECC_EN | NF_BCH_40B_MODE | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write8(mtd, NF_FW_RED_4, NF_bDMALEN);
		    break;
		case NF_BCH_48B_MODE://ECC_48:
		    nfreg_write8(mtd, NF_ECC_EN | NF_BCH_48B_MODE | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write8(mtd, NF_FW_RED_4, NF_bDMALEN);
		    break;
		case NF_BCH_60B_MODE://ECC_60:
		    nfreg_write8(mtd, NF_ECC_EN | NF_BCH_60B_MODE | NF_ECC_NON_STOP, NF_bECCCTRL);
		    nfreg_write8(mtd, NF_FW_RED_4, NF_bDMALEN);
		    break;
	}
}

static void set_dma_length(struct mtd_info *mtd, u8 sectors)
{
u8 tmp;

    tmp = nfreg_read8(mtd, NF_bDMALEN);
    tmp &= 0xE0;
    tmp |= sectors;    
    nfreg_write8(mtd, tmp, NF_bDMALEN);
}

/* set DMA sectors and dma buffer address */
static void set_dma_addr(struct mtd_info *mtd, u32 addr)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);  
u32 tmp;

	tmp = nfreg_read32(mtd, NF_dwDMACONFIG);
	tmp |= (INIT_DMA_IMB | INIT_DMA_SRAM);
	nfreg_write32(mtd, tmp, NF_dwDMACONFIG);
	tmp &= ~(INIT_DMA_IMB | INIT_DMA_SRAM);
	nfreg_write32(mtd, tmp, NF_dwDMACONFIG);	
	if(host->chip_id == C3921)
	{	
		nfreg_write32(mtd, addr & ~0x80000000, NF_dwDMAADDR);
	}	
	else
	{	
		nfreg_write32(mtd, addr & ~0xE0000000, NF_dwDMAADDR);	
	}		
}

/* set DMA read start*/
static void set_dma_start(struct mtd_info *mtd, u32 to_sram, u8 is_read)
{
    nfreg_write8(mtd, 0, NF_bINTFLAG);         
    if (is_read)	
    {	
    	nfreg_write8(mtd, NF_DMA_EN|(to_sram?0:NF_DMA_IMB_EN), NF_bDMACTRL);   
    }		
    else
    {	
    	nfreg_write8(mtd, NF_DMA_IMB_EN | NF_DMA_OUT | NF_DMA_EN, NF_bDMACTRL);	 		   
    }	
}

/*
 * get ECC status
 */	
static int check_ecc_status(struct mtd_info *mtd, int sectors)
{ 
	struct ali_nand_host *host = to_ali_nand_host(mtd);
	u32 tmp, sector_mask=0xFF, i, warning_corrected_bits;
	u8 ecc_fail = 0;

	/* cehck page is blank */ 	
	switch(sectors)
	{
		case 1:
			sector_mask = 0x01;
			break;
		case 2:
			sector_mask = 0x03;
			break;
		case 4:
			sector_mask = 0x0F;
			break;
		case 8:
			sector_mask = 0xFF;
			break;
	}
	
	host->data_all_ff = 0; 
	if (((C3701 == host->chip_id) && (host->chip_ver >= 0x01)) ||
		(C3821 == host->chip_id) ||
		(C3921 == host->chip_id))
	{
		/*check all page is uncorrect and blank detect*/
		tmp = nfreg_read32(mtd, NF_wECCCURSEC) & sector_mask;
		if (!tmp)
		{
			if (sector_mask == (nfreg_read32(mtd, NF_dwDETECTBLANK) & sector_mask))
			{	
				host->data_all_ff = 1;
				return 0;
			}
		}	
	}
	else
	{			
		if (NF_ALL_FF_FLAG == (nfreg_read8(mtd, NF_bECCCTRL) & NF_ALL_FF_FLAG)) 
		{	
			host->data_all_ff = 1;
			return 0;
		}	
	}		
	
	/*check ecc status*/
	tmp = nfreg_read32(mtd, NF_wECCCURSEC) & sector_mask;

	for (i=0; i<sectors; i++)
	{
		if (!(tmp & 0x01))
		{	
			//printk("<check_ecc_status> ecc error %x\n", i);	
			mtd->ecc_stats.failed++;
			ecc_fail = 1;
		}	
		tmp = tmp >> 1; 	
	}
	if(ecc_fail)
	{
		//printk(KERN_ERR "Nand read ECC error\n");
		ali_nand_reg_dump(mtd);
		return -1;
	}


	/*ecc is ok, check ecc corrected count*/
	if (!(nfreg_read32(mtd, NF_wECCCURSEC) & sector_mask))
	{
		return 0;
	}	
		
	switch(host->nf_parm.ecctype)
	{		
		case NF_BCH_24B_MODE:
			warning_corrected_bits = 20;
			break;
		case NF_BCH_40B_MODE:
			warning_corrected_bits = 32;
			break;
		case NF_BCH_48B_MODE:
			warning_corrected_bits = 40;
			break;
		case NF_BCH_60B_MODE:
			warning_corrected_bits = 48;
			break;	
		case NF_BCH_16B_MODE:
		default:	
			warning_corrected_bits = 13;
			break;	
	}
				
	if ((nfreg_read8(mtd, NF_bECCSTS) & 0x3f) > warning_corrected_bits)
	{	
		//mtd->ecc_stats.corrected += nfreg_read8(mtd, NF_bECCSTS) & 0x3f;	
		printk(KERN_WARNING "ECC corrected bits=%d\n", nfreg_read8(mtd, NF_bECCSTS) & 0x3f);		
	}	
	return 0;
}

static int check_read_dma_done(struct mtd_info *mtd, u32 is_sram, u32 secs)
{
	int status;
	u32 timeo = 1000;
	u32 mask = NF_DMA_FLAG;
				
	while (timeo--)
	{
		udelay(1);
		if (mask == (nfreg_read32(mtd, NF_dwINTFLAG) & mask))
			break;			
	}			
	if (!timeo)
	{
		printk(KERN_ERR "Nand DMA read time-out\n");
		return -1;
	}		
	status = check_ecc_status(mtd, secs);		
	return status;			
}

static void hw_ecc_init(struct mtd_info *mtd)
{
    /* clear HW Ecc status */
    nfreg_write8(mtd, 0, NF_bECCSTS);
    nfreg_write32(mtd, 0, NF_wECCERROCR);
    nfreg_write32(mtd, 0xffff, NF_wECCCURSEC);
}

static void hw_ecc_enable(struct mtd_info *mtd)
{
    u8 tmp;

    tmp = nfreg_read8(mtd, NF_bECCCTRL);
    tmp &= ~NF_ECC_BYPASS;
    tmp |= NF_ECC_NON_STOP | NF_ECC_EN;
    nfreg_write8(mtd, tmp, NF_bECCCTRL);
    hw_ecc_init(mtd);
}

static void hw_ecc_disable(struct mtd_info *mtd)
{
}

char word_to_bit(u16 word)     
{                              
	u8 i, bit0_cnt=0, bit1_cnt=0;
                               
	for(i=0; i<16; i++)          
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

#include "ali_mlc.c"      

/*
Read from nand to sram or dram by ecc sectors
*/
#define DATA_TO_SRAM 	1
#define DATA_TO_DRAM 	0
#define CHECK_ECC		1
#define DONT_CHECK_ECC	0
#define ECC_SECTOR	1
/*
	sector : read position, unit is 1024
*/
static u32 nf_dma_read(struct mtd_info *mtd, u32 sector, int len, u8 *buf, u8 to_sram, u8 check_ecc)
{
	struct ali_nand_host *host = to_ali_nand_host(mtd);
	u32 col; 
	u32 page;
	u32 max_sectors;
	u32 status;
	u32 unit;
	u8 is_read = 1;
    
	unit = host->nf_parm.eccsec_perpage;
	while(len > 0)
	{
		col  = sector % unit;
		if (col + len > unit)
			max_sectors = unit - col;
		else
			max_sectors = len;
    
		page = sector / host->nf_parm.eccsec_perpage;
		col *= (host->nf_parm.eccsec_size + host->nf_parm.eccredu_size);
		nf_cmd(mtd, NAND_CMD_READ0, col, page);	 
		hw_ecc_enable(mtd);
		nf_set_seed(mtd, sector);
		set_dma_length(mtd, max_sectors);
		set_dma_addr(mtd, host->hw_dma_addr);
		set_dma_start(mtd, to_sram, is_read);
		status = check_read_dma_done(mtd, to_sram, max_sectors);
		if ((check_ecc && status) != 0)
		{
			printk("[ERR] %s read error\n", __FUNCTION__);
			return -1;
		}
		hw_ecc_disable(mtd);
		if(to_sram && (buf != NULL))
		{
			memcpy((u8 *)buf, (u8 *)(ALI_NANDREG_SRAM), max_sectors * host->nf_parm.eccsec_size);
		}
		else if (buf != NULL)
		{
			memcpy((u8 *)buf, (u8 *)host->dma_buf, max_sectors * host->nf_parm.eccsec_size);
		}
		sector += max_sectors;
		len -= max_sectors;
		buf += max_sectors * host->nf_parm.eccsec_size;
	}
	return 0;
}
 
u16 ba_chip_id_pattern[4]={0x39, 0x21, 0x55, 0xAA};
int check_data_is_correct(u8 *buf)
{
volatile unsigned char i=0, j=0;
volatile unsigned short *p = (u16 *)buf;
volatile unsigned char data[4];

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
	return 0;
}

void get_nf_parm_from_boot_area(struct mtd_info *mtd, u8 *buf)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);	
u32 i = 0;
u8 data = 0;
u16 *p = (u16 *)buf;
u8 ECC_Type = 0;
u8 Bytes_Perpage = 0;
u8 Pages_PerBlock = 0;
	
	data = 0x00;
	for(i=0; i<8; i++)
	{
		data |= word_to_bit(*(p+i)) << i;
	}
	ECC_Type = data & 0x07;
	if(ECC_16 == ECC_Type)
	{
		host->nf_parm.ecctype = NF_BCH_16B_MODE;
		host->nf_parm.eccredu_size  = 32;
		probe_info("NF_BCH_16B_MODE \n");
	}
	else if(ECC_24 == ECC_Type)
	{
		host->nf_parm.ecctype = NF_BCH_24B_MODE;
		host->nf_parm.eccredu_size  = 46;
		probe_info("NF_BCH_24B_MODE \n");
	}
	else if(ECC_40 == ECC_Type)
	{
		host->nf_parm.ecctype = NF_BCH_40B_MODE;
		host->nf_parm.eccredu_size  = 74;
		probe_info("NF_BCH_40B_MODE \n");
	}
	else if(ECC_48 == ECC_Type)
	{
		host->nf_parm.ecctype = NF_BCH_48B_MODE;
		host->nf_parm.eccredu_size  = 88;
		probe_info("NF_BCH_48B_MODE \n");
	}
	else if(ECC_60 == ECC_Type)
	{
		host->nf_parm.ecctype  = NF_BCH_60B_MODE;
		host->nf_parm.eccredu_size  = 110;
		probe_info("NF_BCH_60B_MODE \n");
	}
	else
		printk("[ERR] ECC MODE INVALID %d\n", ECC_Type);

	Bytes_Perpage = (data & 0x18) >> 3;
	if(PAGE_16KB == Bytes_Perpage)
	{
		host->nf_parm.bytes_perpage = 0x4000;		
	}
	else if(PAGE_8KB == Bytes_Perpage)
	{
		host->nf_parm.bytes_perpage = 0x2000;		
	}
	else if(PAGE_4KB == Bytes_Perpage)
	{
		host->nf_parm.bytes_perpage = 0x1000;		
	}
	else if(PAGE_2KB == Bytes_Perpage)
	{
		host->nf_parm.bytes_perpage = 0x800;		
	}
	host->nf_parm.eccsec_perpage = host->nf_parm.bytes_perpage / host->nf_parm.eccsec_size;

	Pages_PerBlock = (data & 0x60) >> 5;
	if(BLOCK_PAGES_64 == Pages_PerBlock)
	{
		host->nf_parm.pages_perblock = 64;
	}
	else if(BLOCK_PAGES_128 == Pages_PerBlock)
	{
		host->nf_parm.pages_perblock = 128;
	}
	else if(BLOCK_PAGES_192 == Pages_PerBlock)
	{
		host->nf_parm.pages_perblock = 192;
	}
	else if(BLOCK_PAGES_256 == Pages_PerBlock)
	{
		host->nf_parm.pages_perblock = 256;
	}	
}

static int check_pmi_pattern(u8 *buf, int pos, int pattern_len, u8 *pattern)
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

static int set_partition_info_c3921(struct mtd_info *mtd)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);
u8 i = 0;

#ifdef CONFIG_MTD_CMDLINE_PARTS
	struct mtd_partition *partitions = NULL;
	static const char *part_probe_types[] = { "cmdlinepart", NULL };
	host->nr_parts = parse_mtd_partitions(mtd, part_probe_types, &partitions, (struct mtd_part_parser_data *)DRIVER_NAME);
	memcpy(ali_nand_partitions, partitions, host->nr_parts*sizeof(struct mtd_partition));
#else
	printk("[ERR] no partitions\n");
	return -1;
#endif

	probe_info("total partition number %d\n", host->nr_parts);
	for (i=0; i<host->nr_parts; i++)
	{
		probe_info("partition[%02d] ofs=0x%012llx len=0x%012llx\n",
				i, ali_nand_partitions[i].offset, ali_nand_partitions[i].size);
	} 
	
	host->parts = ali_nand_partitions;
	/* Register the partitions */
	if (host->nr_parts > 0)
	{
		add_mtd_partitions(mtd, host->parts, host->nr_parts);
	}
	else
	{
		probe_info("Registering %s as whole device\n", mtd->name);
		add_mtd_device(mtd);
	}
	return 0;	
} 
  
static int set_partition_info_s3921(struct mtd_info *mtd, u8 *pmi_area_data)
{ 
	struct ali_nand_host *host = to_ali_nand_host(mtd);
	u8 i = 0;
	
#ifdef CONFIG_MTD_CMDLINE_PARTS
	struct mtd_partition *partitions = NULL;
	static const char *part_probe_types[]	= { "cmdlinepart", NULL };
	host->nr_parts = parse_mtd_partitions(mtd, part_probe_types, &partitions, (struct mtd_part_parser_data *)DRIVER_NAME);
	memcpy(ali_nand_partitions, partitions, host->nr_parts*sizeof(struct mtd_partition));
#else	
	host->nr_parts = *(u32 *) &pmi_area_data[PARTITIONS_NUM];
	for (i=0; i<host->nr_parts; i++){
		ali_nand_partitions[i].offset = *(u32 *) &pmi_area_data[PARTITIONS_OFS + i * 8];						
		ali_nand_partitions[i].size = *(u32 *) &pmi_area_data[PARTITIONS_OFS + i * 8 + 4];						
	}

	for (i=host->nr_parts; i<MAX_PARTITION_NUM; i++)
	{
		ali_nand_partitions[i].name = NULL;
	}
#endif

	probe_info("total partition number %d\n", host->nr_parts);
	for (i=0; i<host->nr_parts; i++)
	{
		probe_info("partition[%02d] ofs=0x%012llx len=0x%012llx\n",
				i, ali_nand_partitions[i].offset, ali_nand_partitions[i].size);
	} 
		host->parts = ali_nand_partitions;
	/* Register the partitions */
	if (host->nr_parts > 0)
	{
		add_mtd_partitions(mtd, host->parts, host->nr_parts);
	}
	else
	{
		probe_info("Registering %s as whole device\n", mtd->name);
		add_mtd_device(mtd);
	}
	return 0;
}

#define START_PMI_SCAN_PAGE 0
#define NEXT_PMI_SCAN_PAGE	256
#define LAST_PMI_SCAN_PAGE	768
static int get_nandflash_parm_from_pmi(struct mtd_info *mtd)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);	
u8  is_scramble = 0;
int page, len;
u8 *pattern;	
u8 *pmi_area_data;
	
	pmi_area_data = kmalloc(2048, GFP_KERNEL); 
	if (NULL == pmi_area_data)
	{	
		printk("[FIX ME] %s (%d)\n", __FUNCTION__, __LINE__);
	}	
	host->nf_parm.rowaddr_cycle = 3;	
	host->nf_parm.eccsec_size   = 1024;			
	if (host->chip_id == C3921)
	{		
		pattern = (u8 *) &pattern_3921;
	}	
	else if (host->chip_id == C3821)
	{		
		pattern = (u8 *) &pattern_3821;
	}	
	else if (host->chip_id == C3503)
	{		
		pattern = (u8 *) &pattern_3503;
	}	
	else if (host->chip_id == C3701)
	{		
		pattern = (u8 *) &pattern_3701;
	}	
	else 
	{
		printk(KERN_ERR "not support, chip id error\n");
		kfree(pmi_area_data);
		return -1;
	}	  
  
	ali_nand_select_chip(mtd, 0); 
	nf_host_init(mtd);
	nf_cmd(mtd, NAND_CMD_RESET, -1, -1);

	//get nand pmi info and id, search boot area per 256 pages
	for(page=START_PMI_SCAN_PAGE; page<=LAST_PMI_SCAN_PAGE; page+=NEXT_PMI_SCAN_PAGE)
	{
		//try 2k, 4k, 8k page
		for (host->nf_parm.eccsec_perpage=2; 
				host->nf_parm.eccsec_perpage <=8; 
					host->nf_parm.eccsec_perpage=host->nf_parm.eccsec_perpage*2)
		{			
			//try scramble on/off
			for(is_scramble=0; is_scramble<2; is_scramble++)
			{			
				host->nf_parm.data_scramble  = is_scramble;
				len = 2 * ECC_SECTOR;
				nf_host_init(mtd);
				if (0 == nf_dma_read(mtd, page*host->nf_parm.eccsec_perpage, len, pmi_area_data, DATA_TO_SRAM, CHECK_ECC))
				{	
					if (-1 == check_pmi_pattern(pmi_area_data, PMI_PATTERN_POS, PMI_PATTERN_LEN, pattern))
					{
						printk(KERN_ERR "PMI chip id error, expect %x\n", *(u32 *)pattern);
						break;
					}				
					host->nf_parm.blocks_perchip = *(u32 *) &pmi_area_data[CONFIG_OFS + 0];
					host->nf_parm.bytes_perpage = *(u32 *) &pmi_area_data[CONFIG_OFS + 4];
					host->nf_parm.pages_perblock = *(u32*) &pmi_area_data[CONFIG_OFS + 8];				
					host->nf_parm.read_timing = *(u32*) &pmi_area_data[CONFIG_OFS + 36];
					host->nf_parm.write_timing = *(u32*) &pmi_area_data[CONFIG_OFS + 40];
					host->nf_parm.eccsec_perpage = host->nf_parm.bytes_perpage / host->nf_parm.eccsec_size;
					set_partition_info_s3921(mtd, pmi_area_data);
					kfree(pmi_area_data);
					probe_info("wBlksPerChip 0x%x wBytesPerPage 0x%x wPagesPerBlock 0x%x\n",
					host->nf_parm.blocks_perchip, host->nf_parm.bytes_perpage, host->nf_parm.pages_perblock);
					ali_nand_select_chip(mtd, -1); 	
					return 0;
				}
			}
		}
	}
	kfree(pmi_area_data);
	ali_nand_select_chip(mtd, -1); 
	printk("[ERR] %s fail\n", __FUNCTION__);
	return -1;
}

/*
Scan boot area from page_start to page_end page and skip page_skip page.
 page_start:  start page number.
*/
int get_nandflash_parm(struct mtd_info *mtd, int page_start)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);	
u8  is_scramble = 0;
int page = 0;
int len = 0;
u8 *boot_area_data;
	
	boot_area_data = kmalloc(2048, GFP_KERNEL); 
	if (NULL == boot_area_data)
	{	
		printk("[FIX ME] %s (%d)\n", __FUNCTION__, __LINE__);
	}	
	//read first 2K data from Nandflash into internal memory
	host->nf_parm.ecctype = NF_BCH_40B_MODE;//ECC_40; 
	host->nf_parm.rowaddr_cycle = 3;
	host->nf_parm.eccsec_size   = 1024;
	host->nf_parm.eccredu_size  = 32;
	host->nf_parm.eccsec_perpage = 2;
	host->nf_parm.data_scramble = 0;  

	ali_nand_select_chip(mtd, 0); 
	nf_host_init(mtd);	
	nf_cmd(mtd, NAND_CMD_RESET, -1, -1);
	//get nand info and id, search boot area per 64 pages
	for(page=page_start; page<=SCAN_RANGE; page+=NEXT_SCAN_PAGE)
	{
		//try scramble on/off
		for(is_scramble=0; is_scramble<2; is_scramble++)
		{			
			host->nf_parm.data_scramble  = is_scramble;
			len = ECC_SECTOR;
			nf_host_init(mtd);
			nf_dma_read(mtd, page*host->nf_parm.eccsec_perpage, len, boot_area_data, DATA_TO_SRAM, DONT_CHECK_ECC);		
			if(check_data_is_correct(&(boot_area_data[NAND_CHIP_ID_OFFSET])) == 0)
			{				
				get_nf_parm_from_boot_area(mtd, &(boot_area_data[NAND_PARM_OFFSET]));				
				//update nand info read again using real ecc type
				len = 2 * ECC_SECTOR;
				nf_host_init(mtd);
				if(nf_dma_read(mtd, page*(host->nf_parm.bytes_perpage/host->nf_parm.eccsec_size), len, boot_area_data, DATA_TO_SRAM, CHECK_ECC) == 0)
				{
					boot_area_parm = (struct boot_area_nand_parameter *)(boot_area_data + NAND_PARM_START);
					printk("%s boot_area_parm: wReadClock 0x%x wWriteClock 0x%x wBlksPerChip 0x%x wBytesPerPage 0x%x wPagesPerBlock 0x%x\n", __FUNCTION__,
						boot_area_parm->wReadClock,
						boot_area_parm->wWriteClock,
						boot_area_parm->wBlksPerChip,
						boot_area_parm->wBytesPerPage,
						boot_area_parm->wPagesPerBlock);
					
					host->nf_parm.read_timing = boot_area_parm->wReadClock;
					host->nf_parm.write_timing = boot_area_parm->wWriteClock;
					host->nf_parm.bytes_perpage = boot_area_parm->wBytesPerPage;
					host->nf_parm.pages_perblock = boot_area_parm->wPagesPerBlock;
					host->nf_parm.blocks_perchip = boot_area_parm->wBlksPerChip;
					host->nf_parm.eccsec_perpage = host->nf_parm.bytes_perpage / host->nf_parm.eccsec_size;
					//ba_get_nand_rr_mode(mtd, &(boot_area_data[NAND_RR_MODE_PARM_OFFSET]));										
					kfree(boot_area_data);
					ali_nand_select_chip(mtd, -1);
					return page;
				}
			}
		}
	}
	kfree(boot_area_data);
	ali_nand_select_chip(mtd, -1);
	host->nf_parm.data_scramble  = 0;
	nf_host_init(mtd);
	printk("[ERR] %s fail\n", __FUNCTION__);
	return -1;
}

/*
Scan boot area at page 0 for AS(advance secuirty).
*/
u32 get_as_nandflash_parm(struct mtd_info *mtd)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);
u32 page = 0;
u8 *boot_area_data;
	
	boot_area_data = kmalloc(2048, GFP_KERNEL); 
	if (NULL == boot_area_data)
	{	
		printk("[FIX ME] %s (%d)\n", __FUNCTION__, __LINE__);
	}
	//read first 2K data from Nandflash into internal memory
	memset(&host->nf_parm, 0x0, sizeof(struct nandflash_parameter));
	host->nf_parm.ecctype = NF_BCH_16B_MODE;//ECC_16;
	host->nf_parm.eccredu_size  = 32;
	host->nf_parm.rowaddr_cycle = 3;
	host->nf_parm.eccsec_size   = 1024;
	host->nf_parm.eccsec_perpage = 2;
	host->nf_parm.data_scramble = 0;

	ali_nand_select_chip(mtd, 0); 
	nf_host_init(mtd);
	//Reset Nand Chip
	nf_cmd(mtd, NAND_CMD_RESET, -1, -1);
	//get nand info and id	
	if(nf_dma_read(mtd, page, 2 * ECC_SECTOR, boot_area_data, DATA_TO_SRAM, CHECK_ECC) == 0)
	{
		if ((host->chip_id == 0x3921) && (host->chip_ver >= 2))
			boot_area_parm = (struct boot_area_nand_parameter *)(boot_area_data + NAND_PARM_START_3921B);
		else
			boot_area_parm = (struct boot_area_nand_parameter *)(boot_area_data + NAND_PARM_START);

		printk("%s boot_area_parm: wReadClock 0x%x wWriteClock 0x%x wBlksPerChip 0x%x \
			wBytesPerPage 0x%x wPagesPerBlock 0x%x\n", __FUNCTION__,
			boot_area_parm->wReadClock,	boot_area_parm->wWriteClock, boot_area_parm->wBlksPerChip,
			boot_area_parm->wBytesPerPage, boot_area_parm->wPagesPerBlock);
		
		host->nf_parm.read_timing = boot_area_parm->wReadClock;
		host->nf_parm.write_timing = boot_area_parm->wWriteClock;
		host->nf_parm.bytes_perpage = boot_area_parm->wBytesPerPage;
		host->nf_parm.pages_perblock = boot_area_parm->wPagesPerBlock;
    	host->nf_parm.blocks_perchip = boot_area_parm->wBlksPerChip;
		host->nf_parm.eccsec_perpage = host->nf_parm.bytes_perpage / host->nf_parm.eccsec_size;				
		kfree(boot_area_data);
		printk("%s ok\n", __FUNCTION__);
		ali_nand_select_chip(mtd, -1);  //added by kinson
		return 0;
	}	
	kfree(boot_area_data);
	ali_nand_select_chip(mtd, -1);
	host->nf_parm.data_scramble  = 0;
	nf_host_init(mtd);
	printk("[ERR] %s fail\n", __FUNCTION__);
	return -1;
}

static void ali_nand_clk_disable(struct mtd_info *mtd)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);
	
	switch(host->chip_id)
	{
		case C3701:
 		case C3503:
		case C3821:
			ali_soc_write((ali_soc_read(0x60) | (1<<2)), 0x60);
			break;

		case C3921:
			ali_soc_write((ali_soc_read(0x60) | (1<<5)), 0x60);
			break;
		default:
			break;
	}
}

static void ali_nand_clk_enable(struct mtd_info *mtd)
{
	struct ali_nand_host *host = to_ali_nand_host(mtd);
	
	switch(host->chip_id)
	{
		case C3701:
		case C3503:
		case C3821:
			ali_soc_write((ali_soc_read(0x60) & ~(1<<2)), 0x60);
			break;
			
		case C3921:
			ali_soc_write((ali_soc_read(0x60) & ~(1<<5)), 0x60);
			break;

		default:
			break;
	}
}

/*
 * init DMA
 */
static void init_hw_dma(struct mtd_info *mtd, u8 is_read)
{
	struct ali_nand_host *host = to_ali_nand_host(mtd);
	u32 tmp;

	nfreg_write8(mtd, 0, NF_bDMACTRL); 
	tmp = nfreg_read32(mtd, NF_dwDMACONFIG);    
	tmp |= (INIT_DMA_IMB | INIT_DMA_SRAM);    
	nfreg_write32(mtd, tmp, NF_dwDMACONFIG);  
	tmp &= ~(INIT_DMA_IMB | INIT_DMA_SRAM);    
	nfreg_write32(mtd, tmp, NF_dwDMACONFIG);  		

	nfreg_write8(mtd, 0, NF_bECCSTS);    
	nfreg_write32(mtd, 0, NF_dwINTFLAG);	
	nfreg_write32(mtd, 0xffff, NF_wECCCURSEC);
	nfreg_write8(mtd, NF_ECC_EN | NF_ECC_NON_STOP | (u8) host->nf_parm.ecctype, NF_bECCCTRL);
	if (is_read)
		nfreg_write32(mtd, IMB_WR_FSH_INT_EN, NF_dwINTFLAG);
	else
		nfreg_write32(mtd, NF_DMA_INT_EN, NF_dwINTFLAG);	
}

/*
 * wait DMA finish
 */	
static int wait_dma_finish(struct mtd_info *mtd, int mode)
{
	int ret = 0;
	unsigned long timeo = jiffies;
	
	ret = wait_for_completion_timeout(&dma_completion, msecs_to_jiffies(1000));
	if (!ret)
	{
		if (mode == HW_DMA_READ)
		{
			printk("[NAND Warning] : Fix me, timeout R %x SOC IRQ 2C=%x 3C=%x\n",
				(int) nfreg_read32(mtd, NF_dwINTFLAG), (int) ali_soc_read(SOC_INT_POLARITY), (int) ali_soc_read(SOC_INT_ENABLE));
		}
		else
		{
			printk("[NAND Warning] : Fix me, timeout W %x SOC IRQ 2C=%x 3C=%x\n",
				(int) nfreg_read32(mtd, NF_dwINTFLAG), (int) ali_soc_read(SOC_INT_POLARITY), (int) ali_soc_read(SOC_INT_ENABLE));
		}
		ali_nand_reg_dump(mtd);
		
		/* polling finish */
		timeo = jiffies + HZ;
		while (time_before(jiffies, timeo))
		{
			ret = nfreg_read32(mtd, NF_dwINTFLAG);
			if (ret & IMB_WR_FSH_FLAG)
			{
				printk("[NAND Warning] : Fix me, polloing read end\n");
				nfreg_write32(mtd, 0, NF_dwINTFLAG);
				return 0;
			}
			else if (ret & NF_DMA_FLAG)
			{
				printk("[NAND Warning] : Fix me, polloing write end\n");
				nfreg_write32(mtd, 0, NF_dwINTFLAG);
				return 0;
			}
			cond_resched();
		}
		printk("[NAND Warning] : Fix me, Nand HW wait dma time out %x\n", (int) nfreg_read32(mtd, NF_dwINTFLAG));
		return -1;
	}	
	return 0;					
}

/*
 * 
 */
static void ali_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	if ((ctrl & NAND_NCE) != NAND_NCE)    
	{     		
		nfreg_write8(mtd, NF_CEJ, NF_bCTRL);        		
		return;    
	}		

	if (ctrl & NAND_CTRL_CHANGE)    
	{        
		if ((ctrl & NAND_CTRL_CLE) == NAND_CTRL_CLE)		
		{           					
			nfreg_write8(mtd, NF_CLE, NF_bCTRL);
		}	
		else if ((ctrl & NAND_CTRL_ALE) == NAND_CTRL_ALE)
		{			
			nfreg_write8(mtd, NF_ALE, NF_bCTRL);		
		}	
		else            
		{						
			nfreg_write8(mtd, 0, NF_bCTRL);   
		}	
	}  

	if (NAND_CMD_NONE != cmd)    	
	{
		nfreg_write8(mtd,  (u8) cmd, NF_bPIODATA);
	}  
}

//for 16k page and read retry
/* 
 *ECC will be calculated automatically, and errors will be detected in
 * waitfunc.
 */ 
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
	int ali_nand_read_page_hwecc(struct mtd_info *mtd, 
			struct nand_chip *chip,	u8 * buf, int oob_required, int page)
#else
	int ali_nand_read_page_hwecc(struct mtd_info *mtd, 
			struct nand_chip *chip,	u8 * buf, int page)
#endif
{
	struct ali_nand_host *host = to_ali_nand_host(mtd);
	int i;
	u32 dma_transfer_len, max_dma_transfer_len = 8192;
	u8 read_retry_option = 0;
	u32 ecc_err_cnt = 0;
	u8 is_read = 1;
	
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
		chip->cmd_ctrl(mtd, NAND_CMD_READ0, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);	

		for(i=0; i<mtd->writesize; i+= dma_transfer_len) // for 16kpage
		{
			/*
			 * init DMA
			 */
			init_completion(&dma_completion); 
			init_hw_dma(mtd, is_read);
			/*
			 * set_dma address, length, start
			 */
			set_dma_addr(mtd, host->hw_dma_addr + i);
			set_dma_length(mtd, dma_transfer_len >> 10);
			set_dma_start(mtd, 0, is_read);			 			
			/*
			 * wait DMA finish
			 */	
			if (wait_dma_finish(mtd, HW_DMA_READ))
			{
				printk("[ERR] %s read page %d error\n", __FUNCTION__, page);				
				return -1;
			}		
			/*
			 * check ecc status
			 */	
			if (check_ecc_status(mtd, dma_transfer_len >> 10))
			{
				printk("ecc error page #0x%08x\n", page);
			}
			/* 
			 * copy data to buf
			 */
			if (buf != NULL)
			{
				if (host->data_all_ff && host->nf_parm.data_scramble)
					memset(buf+i, 0xff, dma_transfer_len);
				else
					memcpy(buf+i, host->dma_buf, dma_transfer_len);
			}
		}
		
		/*
		 * chip->oob always after dma buf + page lebgth
		 */
		*(u32 *) &chip->oob_poi[0] = nfreg_read32(mtd, NF_dwREADREDU0);
		*(u32 *) &chip->oob_poi[4] = nfreg_read32(mtd, NF_dwREADREDU1);
		
		if(mtd->ecc_stats.failed > ecc_err_cnt)//arthur read retry if ecc error happened.
		{
			read_retry_option++;
			if (read_retry_option < g_retry_options_nb)
			{
				mtd->ecc_stats.failed = ecc_err_cnt;
				if (nand_set_feature_rr(mtd, read_retry_option))
					return -1;
			}
		}
		else
		{
			break;
		}
	}while (read_retry_option < g_retry_options_nb);
	
	if(read_retry_option && (host->nf_parm.rr_mode == RR_MODE_MICRON))
	{
		if (read_retry_option < g_retry_options_nb)
			printk("Recover OK\n");
		else
			printk("Recover NG\n");	
		nand_set_feature_rr(mtd, 0);
	}
	
	if(mtd->ecc_stats.failed > ecc_err_cnt)
	{
		printk("\n[ERR] %s page = 0x%x \n", __FUNCTION__, page);
		ali_nand_reg_dump(mtd);
	}
	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
static int ali_nand_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	return 0;
}
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
static int ali_nand_read_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
		int page)
#else
static int ali_nand_read_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
		int page, int sndcmd)

#endif
{
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
	chip->ecc.read_page(mtd, chip, NULL, 1, page);
	return 0;
#else
	sndcmd = chip->ecc.read_page(mtd, chip, NULL, page);
	return sndcmd;
#endif
	
}

// for 16k page
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
	static int ali_nand_write_page_hwecc(struct mtd_info *mtd,
		struct nand_chip *chip,	const uint8_t *buf, int oob_required)
#else
	static void ali_nand_write_page_hwecc(struct mtd_info *mtd,
		struct nand_chip *chip,	const uint8_t *buf)
#endif
{
	struct ali_nand_host *host = to_ali_nand_host(mtd);
	int i;
	u32 dma_transfer_len, max_dma_transfer_len = 8192;
	u8 is_read = 0;

	//arthur 16k page
	if (mtd->writesize > max_dma_transfer_len)
	{
		dma_transfer_len = max_dma_transfer_len;
	}
	else
	{
		dma_transfer_len = mtd->writesize;
	}
		
	for (i=0; i<mtd->writesize; i+= dma_transfer_len)
	{
		/* 
		 * init hw dma 
		 */
		init_completion(&dma_completion); 		
		/* fill OOB*/		
		nfreg_write32(mtd, *(u32 *) &chip->oob_poi[0], NF_dwWRITEREDU0);    
		nfreg_write32(mtd, *(u32 *) &chip->oob_poi[4], NF_dwWRITEREDU1); 
		if (buf != NULL) 
		{	
			memcpy(host->dma_buf, buf+i, dma_transfer_len);   
		}	
		else
		{
			memset(host->dma_buf, 0xFF, dma_transfer_len);		
		}		
		/*
		 * set_dma addr, length, start
		 */	
		init_hw_dma(mtd, is_read);	
		set_dma_addr(mtd, host->hw_dma_addr + i);
		set_dma_length(mtd, dma_transfer_len >> 10);
		set_dma_start(mtd, 0, is_read);			 	
		/*
		 * wait DMA finish
		 */
		wait_dma_finish(mtd, HW_DMA_WRITE);
	}
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
	return 0;
#endif
}

static int ali_nand_write_oob_std(struct mtd_info *mtd, struct nand_chip *chip, 
		int page)
{
	int status = 0;
	
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0, page);
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
		chip->ecc.write_page(mtd, chip, NULL, 1);
	#else
		chip->ecc.write_page(mtd, chip, NULL);
	#endif
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
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
static int ali_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			uint32_t offset, int data_len, const uint8_t *buf, int oob_required, int page,
			int cached, int raw)
#else
	static int ali_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			const uint8_t *buf, int page, int cached, int raw)
#endif
{
	int status;	
	int i;	
	
	#if 0 // ubifs add space_fixup: fixup free space on first mount
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
				printk("All FF, skip write #%d\n", page);
				//DEBUG(MTD_DEBUG_LEVEL3, "All FF, skip write\n");
				//return 0;
			}
		}
	#endif	

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);

	if (unlikely(raw))
	{
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
				chip->ecc.write_page_raw(mtd, chip, buf, 1);
		#else
				chip->ecc.write_page_raw(mtd, chip, buf);
		#endif
	}
	else
	{
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
				chip->ecc.write_page(mtd, chip, buf, 1);
		#else
				chip->ecc.write_page(mtd, chip, buf);
		#endif
	}
	/*
	 * Cached programming disabled for now, Not sure if its worth the
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
		status = chip->errstat(mtd, chip, FL_WRITING, status, page);

	if (status & NAND_STATUS_FAIL)
	{
		printk("[ERR] %s fail. status 0x%x !!!\n", __FUNCTION__, status);
		ali_nand_reg_dump(mtd);
		return -EIO;
	}
	
	#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/* Send command to read back the data */
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
		#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
			if (chip->verify_buf(mtd, buf, mtd->writesize))
			{
				return -EIO;
			}
		#endif		
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


static int nf_pio_read(struct mtd_info *mtd, int page, u8 *buf, int len)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);
struct nand_chip *chip = &host->nand;
int i, status, device_ready = 0;
unsigned long  timeo;

	ali_nand_select_chip(mtd, 0);
	chip->cmd_ctrl(mtd, NAND_CMD_RESET, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);            
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);	
	chip->cmd_ctrl(mtd, NAND_CMD_STATUS, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);            
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);

	device_ready = 0;
	timeo = jiffies;
	timeo += (HZ * 50) / 1000;	
	while (time_before(jiffies, timeo))
	{
		status = nfreg_read8(mtd, NF_bPIODATA);
		if (status & NAND_STATUS_READY)
		{	
			device_ready = 1;
			break;
		}	
	}
	
	if (!device_ready)
	{
		ali_nand_select_chip(mtd, -1);
		return -1;
	}	

	/* read page 0,256,512,768 */	
	chip->cmd_ctrl(mtd, NAND_CMD_READ0,	NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, 0x0,			NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, 0x0, 			NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, (u8) page, 		NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, (u8)(page>>8),	NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, (u8)(page>>16),	NAND_NCE | NAND_ALE | NAND_CTRL_CHANGE);  				
	chip->cmd_ctrl(mtd, NAND_CMD_NONE,	NAND_NCE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, NAND_CMD_READSTART, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);            
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);
	/* wait status ready */
	chip->cmd_ctrl(mtd, NAND_CMD_STATUS, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);	
	device_ready = 0;
	timeo = jiffies;
	timeo += (HZ * 50) / 1000;	
	while (time_before(jiffies, timeo))
	{
		status = nfreg_read8(mtd, NF_bPIODATA);
		if (status & NAND_STATUS_READY)
		{	
			device_ready = 1;
			break;
		}	
	}
	
	if (!device_ready)
	{
		ali_nand_select_chip(mtd, -1);
		return -1;
	}	

	/* start read */
	chip->cmd_ctrl(mtd, NAND_CMD_READ0, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 	NAND_NCE | NAND_CTRL_CHANGE);
	/* 16 bytes */
	for (i=0; i<len; i++)
	{
		buf[i] = nfreg_read8(mtd, NF_bPIODATA);
	}	
	ali_nand_select_chip(mtd, -1);
	return 0;
}
/**
 * unknown ecc type, use PIO read byte 0~11 for judgement 
 *
 **/ 
static int get_ecc_type(struct mtd_info *mtd)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);	
int i, status, page = 0;
u8 buf[16];
char bit[6];
char not_match = 0;

	for (page=START_PMI_SCAN_PAGE; page<=LAST_PMI_SCAN_PAGE; page+=NEXT_PMI_SCAN_PAGE)
	{
		status = nf_pio_read(mtd, page, &buf[0], 16);
		if (status == -1)
			continue;
		
		bit[0] = (char) word_to_bit(*(u16 *) &buf[0]);
		bit[1] = (char) word_to_bit(*(u16 *) &buf[2]);
		bit[2] = (char) word_to_bit(*(u16 *) &buf[4]);
		bit[3] = (char) word_to_bit(*(u16 *) &buf[6]);
		bit[4] = (char) word_to_bit(*(u16 *) &buf[8]);
		bit[5] = (char) word_to_bit(*(u16 *) &buf[10]);
		host->nf_parm.ecctype = 0;
		if ((bit[0] == -1) || (bit[1] == -1) || (bit[2] == -1) || (bit[3] == -1)
				|| (bit[4] == -1) || (bit[5] == -1))
		{
			printk("get ecc type error (1)\n");
			continue;
		}

		for (i=0; i<6; i++)
		{
			if (bit[i] != ECC_pattern_16b[i])
				not_match = 1;
		}
		if (!not_match)
		{
			host->nf_parm.ecctype = NF_BCH_16B_MODE;
			host->nf_parm.eccredu_size  = 32;
		}
		else
		{
			not_match = 0;
			for (i=0; i<6; i++)
			{
				if (bit[i] != ECC_pattern_24b[i])
					not_match = 1;
			}
			
			if (!not_match)
			{
				host->nf_parm.ecctype = NF_BCH_24B_MODE;
				host->nf_parm.eccredu_size  = 46;
			}	
		}				
		if (host->nf_parm.ecctype == 0)
		{
			printk("get ecc type error (2)\n");			
			continue;
		}	
		else
			return 0;
	}	
	printk("[fix me], get ecc type error\n");					
	return -1;
}

static void update_mtd_info(struct mtd_info *mtd)
{
struct ali_nand_host *host = to_ali_nand_host(mtd);
	
	mtd->oobsize = 8;
	mtd->oobavail = 8;
	mtd->subpage_sft = 0;
	mtd->erasesize = host->nf_parm.bytes_perpage * host->nf_parm.pages_perblock;
	mtd->writesize = host->nf_parm.bytes_perpage;	
	mtd->size = (u64) mtd->erasesize * host->nf_parm.blocks_perchip;
	probe_info("mtd->size 0x%012llx\n", mtd->size);		
	mtd->erasesize_shift = ffs(mtd->erasesize) - 1;
	mtd->writesize_shift = ffs(mtd->writesize) - 1;	
	probe_info("mtd->erasesize 0x%x\n", mtd->erasesize);
	probe_info("mtd->writesize 0x%x\n", mtd->writesize);
}  

static void update_chip_info(struct mtd_info *mtd)  
{
struct ali_nand_host *host = to_ali_nand_host(mtd); 
struct nand_chip *chip = &host->nand; 
	
	chip->page_shift = ffs(mtd->writesize) - 1;
	chip->bbt_erase_shift = chip->phys_erase_shift = ffs(mtd->erasesize) - 1;				
	chip->subpagesize = mtd->writesize;
	chip->chipsize = (u64) mtd->erasesize * host->nf_parm.blocks_perchip;
	if (chip->chipsize & 0xffffffff)
	{
		chip->chip_shift = ffs((unsigned)chip->chipsize) - 1;
	}
	else
	{
		chip->chip_shift = ffs((unsigned)(chip->chipsize >> 32)) + 32 - 1;
	}
	chip->pagemask = (chip->chipsize >> chip->page_shift) - 1;	
}      

static void ali_nand_set_chip_clk(struct mtd_info *mtd)
{
struct ali_nand_host *host = to_ali_nand_host(mtd); 
		
	switch(host->chip_id)
	{
		case C3921:
			ali_soc_write((ali_soc_read(C3921_SOC_SYSTEM_CLOCK) | SEL_74M), C3921_SOC_SYSTEM_CLOCK);	
			break;
		
		case C3503:			
			ali_soc_write((ali_soc_read(C3503_SOC_SYSTEM_CLOCK) & ~SEL_74M_54M), C3503_SOC_SYSTEM_CLOCK);	
			break;
		
		case C3821:
			ali_soc_write((ali_soc_read(C3821_SOC_SYSTEM_CLOCK) & ~SEL_74M_54M), C3821_SOC_SYSTEM_CLOCK);			
			break;		
			
		default:
			break;
	}
	
	if (host->nf_parm.read_timing)
	{
		nfreg_write8(mtd, host->nf_parm.read_timing, NF_bREADCYC);
	}
	if (host->nf_parm.write_timing)
	{
		nfreg_write8(mtd, host->nf_parm.write_timing, NF_bWRITECYC);
	}
}


/*
 *	alidev_udc_irq - interrupt handler
 */
static irqreturn_t ali_nand_irq(int dummy, void *_host)
{	
struct ali_nand_host *host = _host;
struct mtd_info	*mtd = &host->mtd;
	
	nfreg_write32(mtd, 0, NF_dwINTFLAG);
	complete(&dma_completion);
	return IRQ_HANDLED;
}

/*****************************************************************
    return Enable/Disable advance security bootrom.
    0x03[1] BOOT_SECURITY_EN 
    return 1, enable
    return 0, disable
*****************************************************************/
u32 AS_bootrom_is_enable(struct mtd_info *mtd)
{
	struct ali_nand_host *host = to_ali_nand_host(mtd);
    u32 OTP_value = 0;
    u32 ret = 0;

	if (C3921== host->chip_id)
    	OTP_value = __REG32ALI(0x18042044);
    else
    	OTP_value = __REG32ALI(0xB8042044);

    ret = (OTP_value & 0x2) >> 1;
    return ret;
}

u32 is_pmi_bootrom(struct mtd_info *mtd)
{
	struct ali_nand_host *host = to_ali_nand_host(mtd);
    u32 ret = 0;
	
	switch(host->chip_id)
	{
		case C3701:
			ret = 1;
			break;

 		case C3503:
 			if (host->chip_ver < 2)	//C3503A
				ret = 1;
			break;
			
		case C3821:
			if(AS_bootrom_is_enable(mtd) == 0) //C3821 non-AS
				ret = 1;
			break;

		default:
			break;
	}

    return ret;
}

/*	C3701 C version IC support new blank page detecttion
	0x58 [23:16] : threshold cont
	0x58 [31:24] : detect value (default is 0)
	0x58 [7:0]	: page 0~7 status, 1: blank / 0: non blank 
	
	initial set:
	1. 0x58 [23:16] = x , threshold value , for 16 bit ECC 15, 24 bit ECC 23					
*/	
static int set_blank_page_detect(struct mtd_info *mtd)
{	
	struct ali_nand_host *host = to_ali_nand_host(mtd);	
		
	if (((C3701 == host->chip_id) && (host->chip_ver >= 0x01)) ||
		 (C3821 == host->chip_id) ||
		 (C3503 == host->chip_id) ||
		 (C3921 == host->chip_id))
	{	
		if (host->nf_parm.ecctype  == NF_BCH_16B_MODE)
		{
			nfreg_write32(mtd, BLANK_DETECT_16ECC, NF_dwDETECTBLANK);
		}
		else if(host->nf_parm.ecctype  == NF_BCH_24B_MODE)
		{
			nfreg_write32(mtd, BLANK_DETECT_24ECC, NF_dwDETECTBLANK);
		}
		else if(host->nf_parm.ecctype  == NF_BCH_40B_MODE)
		{
			nfreg_write32(mtd, BLANK_DETECT_40ECC, NF_dwDETECTBLANK);
		}
		else if(host->nf_parm.ecctype  == NF_BCH_48B_MODE)
		{
			nfreg_write32(mtd, BLANK_DETECT_48ECC, NF_dwDETECTBLANK);
		}
		else if(host->nf_parm.ecctype  == NF_BCH_60B_MODE)
		{
			nfreg_write32(mtd, BLANK_DETECT_60ECC, NF_dwDETECTBLANK);
		}
		else
		{
			printk(KERN_ERR "host->nf_parm.ecctype  error \n");
			return -1;
		}
	}
	return 0;
}

static int __init ali_nand_probe(struct platform_device *pdev)
{
struct ali_nand_host *host = NULL;
struct nand_chip *nand = NULL;
struct mtd_info *mtd = NULL;
struct resource *res = NULL;
unsigned int irq = 0;
int retval = 0;
int err = 0;
long nand_wp_gpio = 0;
struct ali_nand_platform_data {
	int *nand_wp_gpio;
} *platform_data = pdev->dev.platform_data;

	probe_info("%s, %s\n", __FUNCTION__,ALI_NAND_DRIVER_VERSION);
	/* Allocate memory for MTD device structure and private data */
	host = kzalloc(sizeof(struct ali_nand_host), GFP_KERNEL);
	if (!host)
		return -ENOMEM;
	
	host->chip_id = (u16) (ali_soc_read(0) >> 16);
	host->chip_pkg = (u8) (ali_soc_read(0) >> 8) & 0xFF;
	host->chip_ver = (u8) (ali_soc_read(0)) & 0xFF;
	
	probe_info("%s ALI_SOC_BASE 0x%x\n", __FUNCTION__, (u32)ALI_SOC_BASE);
	printk("%s chip id 0x%x, chip_pkg 0x%x, ver 0x%x\n", __FUNCTION__, 
		host->chip_id, host->chip_pkg, host->chip_ver);
	
	host->dev = &pdev->dev;
	res  = pdev->resource;	
	/* structures must be linked */
	nand = &host->nand;
	mtd = &host->mtd;
	mtd->priv = nand;	
	mtd->owner = THIS_MODULE;
	mtd->dev.parent = &pdev->dev;
	mtd->name = "ali_nand";		
	/*
	 * Nand flash reset / clk not gated 
	 */
	ali_nand_clk_enable(mtd);	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
	{
		err = -ENODEV;
		goto eres;
	}

	switch(host->chip_id)
	{
		case C3701:
		case C3503:
		case C3821:
	    host->regs = ioremap(ALI_NANDREG_BASE, ALI_NANDREG_LEN);		
			probe_info("mips ali_nand_reg, viture=0x%08x\n", (int) host->regs);
			break;
		case C3921:
	    host->regs = ioremap(ALI_NANDREG_BASE, ALI_NANDREG_LEN);		
			probe_info("arm ali_nand_reg, viture=0x%08x\n", (int) host->regs);
			break;
		default:
			break;
	}

	nand->IO_ADDR_R = (void __iomem *) (host->regs + NF_bPIODATA);
	nand->IO_ADDR_W = (void __iomem *) (host->regs + NF_bPIODATA);
	if (!request_mem_region(res->start, res->end - res->start + 1, 
				nandname))
		return -EBUSY;

	/* 
	 * buffer1 for DMA access 
	 */
	res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (!res)
	{
		err = -ENODEV;
		goto eres;
	}

	host->dma_buf = dma_alloc_coherent(&pdev->dev, 
			res->end - res->start + 1, &host->hw_dma_addr, GFP_KERNEL);
	if (!host->dma_buf)
	{
		err = -ENOMEM;
		goto eres;
	}
	probe_info("vitual dma buffer addr 0x%x\n", (u32) host->dma_buf);
	probe_info("physical dma buffer addr 0x%x\n", (u32)host->hw_dma_addr);
	
	platform_data = pdev->dev.platform_data;	// Get Platform Data: nand_wp_gpio
	if (platform_data != NULL) 
	{
		printk("ali nand write protect gpio %d\n", *platform_data->nand_wp_gpio);
		nand_wp_gpio = *platform_data->nand_wp_gpio;
		if(nand_wp_gpio > 0)
		{
			if(gpio_request(nand_wp_gpio, "nand_wp_gpio"))
			{
				printk("[ERR] ali nand write protect gpio!\n");
				gpio_free(nand_wp_gpio);
			}
			else
			{
				gpio_enable_pin(nand_wp_gpio);
				gpio_direction_output(nand_wp_gpio, 0);
			}
		}
	}

	/* 
	 * Reference hardware control function 
	 */
	nand->cmd_ctrl  = ali_nand_cmd_ctrl;
	nand->write_page = ali_nand_write_page;
	nand->ecc.read_page = ali_nand_read_page_hwecc;
	nand->ecc.write_page = ali_nand_write_page_hwecc;
	nand->select_chip = ali_nand_select_chip;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
	nand->verify_buf = ali_nand_verify_buf;	
#endif
	nand->ecc.read_oob = ali_nand_read_oob_std;
	nand->ecc.write_oob = ali_nand_write_oob_std;
	nand->scan_bbt = ali_nand_default_bbt;
	nand->dev_ready = NULL;
	nand->chip_delay = 1;
	
	/*
	* reauest irq
	*/	
	irq = platform_get_irq(pdev, 0);	
	if (irq < 0)
	{
		printk("failed %s (%d)  \n", __FUNCTION__, __LINE__);
		goto eres;
	}
	retval = request_irq(irq, ali_nand_irq, IRQF_SHARED, mtd->name, host);	
	if (retval != 0) 
	{
		printk(KERN_ALERT "failed %s (%d)  \n", __FUNCTION__, __LINE__);
		goto eres;
	}
	probe_info("nand irq = %d\n", irq);
		
	/* use complete */
	init_completion(&dma_completion);
	
	/* set nand rr*/
	get_mlc_nand_id(mtd, nand); //get mlc parameter	

	if (is_pmi_bootrom(mtd))
	{
		/*
		 * Get ECC type, 16/24 bits BCH mode
		 */
		if (get_ecc_type(mtd))
		{
			printk(KERN_ERR "Can't get ECC type\n");
				goto escan;
		}
	}
	else if (AS_bootrom_is_enable(mtd))
	{
		printk("as bootrom enable\n");
		if (get_as_nandflash_parm(mtd) == -1)
		{
			printk(KERN_ERR "get_as_nandflash_parm fail\n");
			goto escan;
		}
	}
	else
	{
		printk("as bootrom disable\n");
		if (get_nandflash_parm(mtd, SCAN_START_PAGE) == -1)
		{
			printk(KERN_ERR "get_nandflash_parm fail\n");
			goto escan;
		}
	}
	
	/*
	 * options
	 */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 0))
	nand->options = NAND_NO_SUBPAGE_WRITE;
#else
	nand->options = NAND_NO_SUBPAGE_WRITE | NAND_USE_FLASH_BBT;	
#endif
	nand->ecc.layout = &ali_nand_oob_32;
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.size = 1024;
	nand->ecc.bytes = 28;
	nand->ecc.layout->oobavail = 8;
	nand->bbt_td = &ali_bbt_main_descr;
	nand->bbt_md = &ali_bbt_mirror_descr;
	
	if (nand_scan(mtd, 1)) 
	{		
		err = -ENXIO;
		goto escan;
	}
	
	if (is_pmi_bootrom(mtd))
	{		
		if (get_nandflash_parm_from_pmi(mtd))
		{
			printk(KERN_ERR "Can't get pmi\n");		
			goto escan;
		}					
		update_mtd_info(mtd);	
		update_chip_info(mtd);
		ali_nand_set_chip_clk(mtd);
	}
	else
	{
		update_mtd_info(mtd);	
		update_chip_info(mtd);
		ali_nand_set_chip_clk(mtd);
		set_partition_info_c3921(mtd);
	}
 	
 	if (-1 == set_blank_page_detect(mtd))
 		goto escan;   

	platform_set_drvdata(pdev, host);
	return 0;
escan:	
	iounmap(host->regs);
	dma_free_coherent(&pdev->dev, res->end - res->start + 1, host->dma_buf, host->hw_dma_addr);
eres:
	kfree(host);
	return err;
}

#include "ali_ibu.c"

/* module_text_address() isn't exported, and it's mostly a pointless
   test if this is a module _anyway_ -- they'd have to try _really_ hard
   to call us from in-kernel code if the core NAND support is modular. */
#ifdef MODULE
#define caller_is_module() (1)
#else
#define caller_is_module() \
	module_text_address((unsigned long)__builtin_return_address(0))
#endif

#ifdef CONFIG_PM
	#ifdef CONFIG_SUSPEND_TO_DRAM
	static int ali_nand_reg_store(struct mtd_info *mtd)
	{
		struct ali_nand_host *host = to_ali_nand_host(mtd);
		int i = 0;

		for (i=0; i<ALI_NAND_REG_LEN; i+=4)
		{
			if ((i != NF_dwDMADATA) && (i != NF_bPIODATA))
			{
				host->ali_nand_regs[i/4] = nfreg_read32(mtd, i);
			}
		}
	}

	static int ali_nand_reg_load(struct mtd_info *mtd)
	{
		struct ali_nand_host *host = to_ali_nand_host(mtd);
		int i = 0;

		for (i=0; i<ALI_NAND_REG_LEN; i+=4)
		{
			if ((i != NF_dwDMADATA) && (i != NF_bPIODATA))
			{
				nfreg_write32(mtd, host->ali_nand_regs[i/4], i);
			}
		}
	}
	#endif
	static int ali_nand_suspend(struct platform_device *pdev, pm_message_t state)
	{
		struct ali_nand_host *host = platform_get_drvdata(pdev);
		struct mtd_info	*mtd = &host->mtd;
		int ret = 0;
	
		DEBUG(MTD_DEBUG_LEVEL0, "ALI_NAND : NAND suspend\n");
		if(!host)
			return -1;
	#ifdef CONFIG_SUSPEND_TO_DRAM
		ali_nand_reg_store(mtd);
	#endif
		
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0))
			ret = mtd->_suspend(mtd);
	#else
			ret = mtd->suspend(mtd);
	#endif
		ali_nand_clk_disable(mtd);
		return ret;
	}
	
	static int ali_nand_resume(struct platform_device *pdev)
	{
		struct ali_nand_host *host = platform_get_drvdata(pdev);
		struct mtd_info	*mtd = &host->mtd;
		struct nand_chip *chip = &host->nand;
		int ret = 0;

		if (!host)
			return -1;

		/*
		 * Enable the NFC clock
		 */
		ali_nand_clk_enable(mtd);
	
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0))
			mtd->_resume(mtd);
	#else
			mtd->resume(mtd);
	#endif
	
		ali_nand_select_chip(mtd, 0);
	#ifdef CONFIG_SUSPEND_TO_DRAM
		ali_nand_reg_load(mtd);
	#endif
		chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
		ali_nand_select_chip(mtd, -1);
		udelay(500);

		return ret;
	}
#else
	#define ali_nand_suspend   NULL
	#define ali_nand_resume    NULL
#endif				/* CONFIG_PM */

static int __devexit ali_nand_remove(struct platform_device *pdev)
{
struct ali_nand_host *host = platform_get_drvdata(pdev);
struct mtd_info	*mtd = &host->mtd;
  
	ali_nand_clk_disable(mtd);	
	platform_set_drvdata(pdev, NULL);
	nand_release(mtd);	
	iounmap(host->regs);
	kfree(host);
	return 0;
}

static struct platform_driver ali_nand_driver = {
	.driver = {
		.name = DRIVER_NAME,
	},
	.remove = ali_nand_remove,
	.suspend = ali_nand_suspend,
	.resume = ali_nand_resume,
};

static int __init ali_nand_driver_init(void)
{
	return platform_driver_probe(&ali_nand_driver, ali_nand_probe);
}

static void __exit ali_nand_driver_cleanup(void)
{
	/* Unregister the device structure */
	platform_driver_unregister(&ali_nand_driver);
}

EXPORT_SYMBOL(ali_nand_partitions);
module_init(ali_nand_driver_init);
module_exit(ali_nand_driver_cleanup);
MODULE_AUTHOR("ALi Corporation, Inc.");
MODULE_DESCRIPTION("ALI NAND MTD driver");
MODULE_LICENSE("GPL");

