/*
 * (C) Copyright 2003
 * Thomas.Lange@corelatus.se
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/io.h>
#ifdef CONFIG_CMD_NAND
#include <nand.h>
#include "ali_nand.h"
#endif

//#define ALI_DEBUG_ON
#ifdef ALI_DEBUG_ON
    #define ALI_PRINTF   printf
#else
    #define ALI_PRINTF(...)	do{}while(0)
#endif

#define ALI_SOC_BASE			              0xB8000000

phys_size_t initdram(int board_type)
{
	/* Sdram is setup by assembler code */
	/* If memory could be changed, we should return the true value here */
	return MEM_SIZE*1024*1024;
}

/* In arch/mips/cpu/cpu.c */
void write_one_tlb( int index, u32 pagemask, u32 hi, u32 low0, u32 low1 );

u16 Get_AliChipGen()
{
	u32 chip_id;
	chip_id = *((volatile u32 *)(ALI_SOC_BASE));	

       return (u16) ((chip_id & 0xFFFF0000) >> 16);
}

int checkboard (void)
{

	u32 proc_id, chip_id;
        u16 ChipGen = Get_AliChipGen();

	proc_id = read_c0_prid();
	chip_id = *((volatile u32 *)(ALI_SOC_BASE));	
	ALI_PRINTF("Board: ALi chipset proc_id=0x%x,chip_id=0x%x\n",proc_id,chip_id);

       if (0x3901 == ChipGen)
       {
        	switch( chip_id & 0xFFFF0300 )
        	{
        		case 0x39010000:	printf("M3911 256Pin NMP 32bits DDR2\n");			break;
        		case 0x39010100:	printf("M3901 216Pin NMP 16bits DDR2\n");			break;			
        		case 0x39010200:	printf("M3701G 256Pin STB with QAM 32bits DDR2\n");	break;
        		case 0x39010300:	printf("M39xx 256Pin STB with CI 16bits DDR2\n");   break;								
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
       }
       else if (0x3701 == ChipGen){
        	switch( chip_id & 0xFFFF0300 )
        	{
        		case 0x37010300:	printf("M3701C 144Pin STB (%d MHz) with DDR3\n",CONFIG_SYS_MHZ);   break;								
        		case 0x37010200:	printf("M3701H 256Pin STB (%d MHz) with DDR3\n",CONFIG_SYS_MHZ);   break;								
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
       }
       else{
        	switch( chip_id & 0xFFFF0C0F )
        	{
        		case 0x36020002:	printf("M3602B\n");			break;
        		case 0x36030001:	printf("M3601E\n");			break;			
        		case 0x36030401:	printf("M3701E/M3701F\n");	break;
        		case 0x36030801:	printf("M3606/M3606C\n");		break;
        		case 0x36030C01:	printf("M3603\n");			break;									
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
       }

	//printf(" proc_id=0x%x\n", proc_id);

	return 0;
}

#ifdef CONFIG_CMD_NAND

#define ali_soc_read(reg)  (u32) (*(volatile u32 *) ((u32) reg))
#define ali_soc_write(val, reg)  *(volatile u32 *) ((u32) reg) = (u32) val

UINT32 ali_nf_reg;
UINT32 ali_soc_reg;
extern int ali_nand_init(struct nand_chip *nand);
static struct ali_nand_host ali_host;

static void ali_nand_platform_init (struct ali_nand_host *host)
{
	UINT32 tmp32;
	
	host->chip_id = readl(ALI_SOC_BASE) >> 16;	
	host->chip_package = (UINT8) (readl(ALI_SOC_BASE) >> 8) & 0x0F;	
	host->chip_ver = (UINT8) readl(ALI_SOC_BASE) & 0xFF;	
	
	ALI_PRINTF("Chip id %x Version %x\n", host->chip_id, host->chip_ver);	
	switch(host->chip_id)
	{
		case C3603:
			tmp32 = readl( ALI_SOC_BASE+0x64);	
			tmp32 &= ~(1<<14);	
			writel(tmp32, ALI_SOC_BASE+0x64);	

			if (host->chip_package  == M3603)
			{
				/* pin mux enable BGA setting*/
				ALI_PRINTF("ali_nand_probe:BGA package\n");
				tmp32 = readl(ALI_SOC_BASE + 0xAC);
				tmp32 |= (1<<21);	
				writel(tmp32, ALI_SOC_BASE + 0xAC);

				/* disable GPIO function*/
				/* disable GPIOB */
				tmp32 = readl(ALI_SOC_BASE + 0x438);
				tmp32 &= ~( 	    (1<<M36xx_BGA_CLE) | 
				    	    (1<<M36xx_BGA_CEJ) |
				  	    (1<<M36xx_BGA_ALE) |
			 		    (1<<M36xx_BGA_WEJ) |  
				 	    (1<<M36xx_BGA_REJ) |   
				 	    (1<<M36xx_BGA_DATA0) |   
					    (1<<M36xx_BGA_DATA1) |   
					    (1<<M36xx_BGA_DATA2) |   
					    (1<<M36xx_BGA_DATA3) |   
					    (1<<M36xx_BGA_DATA4) |   
					    (1<<M36xx_BGA_DATA5) |   
					    (1<<M36xx_BGA_DATA6) |   
					    (1<<M36xx_BGA_DATA7) );   
				writel(tmp32, ALI_SOC_BASE + 0x438);						
			}
			else if ((host->chip_package  == M3606)||(host->chip_package  == M3701E))
			{
				ALI_PRINTF("ali_nand_probe:QFP package\n");				
				tmp32 = readl(ALI_SOC_BASE + 0x88);
				tmp32 &= ~((1<<1)|(1<20));	//CI UART2
				writel(tmp32,ALI_SOC_BASE + 0x88);
			
				tmp32 = readl(ALI_SOC_BASE + 0xAC);
				tmp32 |= (1<<22) | (1<<24);	
				writel(tmp32,ALI_SOC_BASE + 0xAC);
				//flash reg CPU_CTRL_DMA
				//bit8: PIO_arbit_fuc_en 1 sflash/pflash/ci can share the bus with flash arbiter
				//bit9:cpu_set_arbit_en 
				//bit12:10 cpu_set_arbit_en 001 sflash is enable 010 pflash is enable 100 CI is enable
				tmp32 = readl(ALI_SOC_BASE + 0x2E000);
				tmp32 &= ~(0x00001F00);	
				tmp32 |= 0x00001200 ;	
				writel(tmp32, ALI_SOC_BASE + 0x2E000);

				/* disable GPIO function*/
				/* disable GPIOB */			
				tmp32 = readl(ALI_SOC_BASE + 0x438);
				tmp32 &= ~( 	    
				    	    (1<<M36xx_QFP_CEJ) |   
			 		    (1<<M36xx_QFP_DATA0) |   
					    (1<<M36xx_QFP_DATA1) |   
					    (1<<M36xx_QFP_DATA2) |   
					    (1<<M36xx_QFP_DATA3) |   
					    (1<<M36xx_QFP_DATA4) |   
					    (1<<M36xx_QFP_DATA5) |   
					    (1<<M36xx_QFP_DATA6) |   
					    (1<<M36xx_QFP_DATA7) );   
				writel(tmp32, ALI_SOC_BASE + 0x438);		
		
				tmp32 = readl(ALI_SOC_BASE + 0x43c);
				tmp32 &= ~((1<<M36xx_QFP_CLE) |(1<<M36xx_QFP_WPJ));
				writel(tmp32, ALI_SOC_BASE + 0x43c);	
			}
			else
			{
				ALI_PRINTF("Package not support Nand Flash\n");	

			}
			break;
		case C3901:
			tmp32 = readl(ALI_SOC_BASE + 0x64);		/*Nand flash clk not gated */
			tmp32 &= ~(1<<6);	
			writel(tmp32, ALI_SOC_BASE + 0x64);

			tmp32 = readl(ALI_SOC_BASE + 0x74);
			writel(tmp32 | 0x40040000, ALI_SOC_BASE + 0x74);		

			if (M3701G == host->chip_package)
			{
				tmp32 = readl(ALI_SOC_BASE+0x438);	/* GPIOC_2 enable*/
				writel(tmp32 | (2<<1), ALI_SOC_BASE + 0x438);
			
				tmp32 = readl( ALI_SOC_BASE + 0x358);	/* GPIOC_2 as output*/
				writel(tmp32 | (2<<1), ALI_SOC_BASE + 0x358);
			
				tmp32 = readl(ALI_SOC_BASE + 0x354);	/* GPIOC_2 output H*/
				writel(tmp32 | (2<<1), ALI_SOC_BASE + 0x354);
			}
			break;

		case C3701:
			tmp32 = readl(ALI_SOC_BASE + 0x64);		/*Nand flash clk not gated */
			tmp32 &= ~(1<<6);	
			writel(tmp32, ALI_SOC_BASE + 0x64);

			tmp32 = readl(ALI_SOC_BASE + 0x74);
			writel(tmp32 | 0x40040000, ALI_SOC_BASE + 0x74);		
			switch(host->chip_package)			
			{								
				case 00:/* 376 pin */								
				case 01:/* 256 pin */				
				case 02:/* 256 pin */					/* WP use XGPIO41 */
					writel((readl(ALI_SOC_BASE + 0x434) | 0x200), ALI_SOC_BASE + 0x434);
					writel((readl(ALI_SOC_BASE + 0xD8) | 0x200), ALI_SOC_BASE + 0xD8);
					writel((readl(ALI_SOC_BASE + 0xD4) | 0x200), ALI_SOC_BASE + 0xD4);
					break;								
				case 03:/* QAM 128 pin */				/* enable 128 pin NF select */
					writel((readl(ALI_SOC_BASE + 0x88) | (1<<7)), ALI_SOC_BASE + 0x88);
					/* WP use XGPIO5 */
					writel((readl(ALI_SOC_BASE + 0x430) | 0x20), ALI_SOC_BASE + 0x430);
					writel((readl(ALI_SOC_BASE + 0x58) | 0x200), ALI_SOC_BASE + 0x58);
					writel((readl(ALI_SOC_BASE + 0x54) | 0x200), ALI_SOC_BASE + 0x54);
					break;									
				case 04:/* NMP 128 pin */				/* enable 128 pin NF select */
					writel((readl(ALI_SOC_BASE + 0x88) | (1<<7)), ALI_SOC_BASE + 0x88);
					/* WP use HW */					
					break;				
				default:
					break;			
			}				
			break;	
		case C3811:
			writel(0x00800200, ALI_SOC_BASE + 0x74);	
			break;
		default:
			break;
	}

}

int board_nand_init(struct nand_chip *nand)
{
	u32 tmp;

	ALI_PRINTF("board_nand_init");
	memset(nand, 0 ,sizeof(*nand));
	ali_nf_reg =  ALI_NAND_BASE;
	ali_soc_reg = ALI_SOC_BASE;	

#if 1
	memset(&ali_host, 0 ,sizeof(struct ali_nand_host));
       ali_nand_platform_init(&ali_host);
#else
	/* Nand flash clk not gated */
	tmp = ali_soc_read(ali_soc_reg + 0x64);			
	tmp &= ~(1<<6);			
	ali_soc_write(tmp, ali_soc_reg + 0x64);		
	tmp = ali_soc_read(ali_soc_reg + 0x74);		
	ali_soc_write(tmp | 0x40040000, ali_soc_reg + 0x74);
#endif
	if (ali_nand_init(nand))
		return -1;
	else
		return 0;
	
}	

void enable_cs(int n)
{
       if (0x3901 != Get_AliChipGen())
           return;

	if (n==0)
	{
		ali_soc_write(ali_soc_read(ali_soc_reg + 0x8C) & ~(1<<21), ali_soc_reg + 0x8C);
	}
	else if (n==1)
	{
		ali_soc_write(ali_soc_read(ali_soc_reg + 0x8C) | (1<<21), ali_soc_reg + 0x8C);
	}
		
}

#endif

/*
 * Initializes on-chip MMC controllers.
 */
int cpu_mmc_init(bd_t *bis)
{
#ifdef CONFIG_ALI_MMC
	return ali_mmc_register(bis);
#else
	return 0;
#endif
}

