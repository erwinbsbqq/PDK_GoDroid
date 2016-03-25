/*
 * (C) Copyright 2011
 * ALi Corporation 
 *
 * David Shih, Software Engineering, david.shih@alitech.com
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

#if defined(CONFIG_CMD_NAND)

#include <nand.h>
#include "ali_nand.h"


extern UINT32 ali_nf_reg;
extern UINT32 ali_soc_reg;
extern void enable_cs(int n);
	

static u8 *chip_hw_dma_address;
#define nfreg_read(reg)  (u32) (*(volatile u32 *) ((u32) reg))
#define nfreg_write(val, reg)  *(volatile u32 *) ((u32) reg) = (u32) val


/*
 * 
 */
static void ali_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{    
	if ((ctrl & NAND_NCE) != NAND_NCE)    
	{     
		nfreg_write(NF_CEJ, ali_nf_reg + bCTRL);        
		return;    
	}

	if (ctrl & NAND_CTRL_CHANGE)    
	{        
		if (ctrl & NAND_CLE)            
			nfreg_write(NF_CLE, ali_nf_reg + bCTRL);        
		else if (ctrl & NAND_ALE)            
			nfreg_write(NF_ALE, ali_nf_reg + bCTRL);        
		else            
			nfreg_write(0, ali_nf_reg + bCTRL);    
	}    
	
	if (NAND_CMD_NONE != cmd)    	
		nfreg_write((uint8_t) cmd,  ali_nf_reg + bPIODATA);	
}


/* 
 *ECC will be calculated automatically, and errors will be detected in
 * waitfunc.
 */
#define debug_read ALI_NAND_DEBUG
int ali_nand_read_page_hwecc(struct mtd_info *mtd, 
				struct nand_chip *chip,
				u8 * buf, int page)
{    
u32 tmp0, tmp1;    
unsigned long  timeo = (CONFIG_SYS_HZ * 10) / 1000;	
u32 time_start;

	debug_read("--------Read page %x-------\n", page);
	chip->waitfunc(mtd, chip);           
	chip->cmd_ctrl(mtd, NAND_CMD_READ0, 
				NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);            
	chip->cmd_ctrl(mtd, NAND_CMD_NONE, 
				NAND_NCE | NAND_CTRL_CHANGE);

	/*
	* init DMA
	*/
	nfreg_write(0, (ali_nf_reg + bDMACTRL));    	

	tmp0= nfreg_read(ali_nf_reg + dwDMACONFIG);    
	tmp0 |= (INIT_DMA_IMB | INIT_DMA_SRAM);    
	nfreg_write(tmp0, (ali_nf_reg + dwDMACONFIG));  
	tmp0 &= ~(INIT_DMA_IMB | INIT_DMA_SRAM);    
	nfreg_write(tmp0, (ali_nf_reg + dwDMACONFIG));  		

	nfreg_write(0, (ali_nf_reg + bECCSTS));    
	nfreg_write(0, (ali_nf_reg + dwINTFLAG)); 
	nfreg_write(0xffff, (ali_nf_reg + wECCCURSEC));

	/*
	* set_dma_length, address
	*/
	nfreg_write(chip_hw_dma_address, (ali_nf_reg + dwDMAADDR));		
	tmp0= nfreg_read(ali_nf_reg + dwDMAADDR) & (~0xF0000000);	
	nfreg_write(tmp0, (ali_nf_reg + dwDMAADDR));	

	tmp0= nfreg_read(ali_nf_reg + bDMALEN);   
	tmp0 &= 0xE0;    
	tmp0 |= (mtd->writesize >> 10); 
	nfreg_write(tmp0, (ali_nf_reg + bDMALEN));	

	
	/*
	* set DMA start (Nand to Ali chip)
	*/	
	nfreg_write(NF_DMA_IMB_EN | NF_DMA_EN,  ali_nf_reg + bDMACTRL); 	

	/*
	* wait DMA finish
	*/
	time_start = get_timer(0);
	while (get_timer(time_start) < timeo) 
	{
		tmp0 = nfreg_read(ali_nf_reg + dwINTFLAG);       
		if ( (NF_DMA_DONE | NF_ECC_DONE |IMB_WR_FSH_FLAG) 
			==  (tmp0 & (NF_DMA_DONE | NF_ECC_DONE |IMB_WR_FSH_FLAG)) )
		{            
			nfreg_write(0, ali_nf_reg + dwINTFLAG); 
			break;        
		}        
	}	

	if (buf != NULL)   
		memcpy(buf, chip_hw_dma_address, mtd->writesize);    

	tmp0 = nfreg_read(ali_nf_reg + dwREADREDU0);    
	tmp1 = nfreg_read(ali_nf_reg + dwREADREDU1);    

	chip->oob_poi =  buf + mtd->writesize;
#ifdef ALI_NAND_DEBUG_ON
	debug_read(" data");
	int i;
	for (i=0;i<16;i++)
		debug_read(" %x", buf[i]);
	debug_read("\nredundant %x %x\n", tmp0, tmp1);		
#endif
	chip->oob_poi[0] = (uint8_t) tmp0;    
	chip->oob_poi[1] = (uint8_t) (tmp0 >> 8);    
	chip->oob_poi[2] = (uint8_t) (tmp0 >> 16);    
	chip->oob_poi[3] = (uint8_t) (tmp0 >> 24);    
	chip->oob_poi[4] = (uint8_t) tmp1;    
	chip->oob_poi[5] = (uint8_t) (tmp1 >> 8);    
	chip->oob_poi[6] = (uint8_t) (tmp1 >> 16);    
	chip->oob_poi[7] = (uint8_t) (tmp1 >> 24);    	 

	/*
	* get ECC status
	*/	
	tmp0 = nfreg_read(ali_nf_reg + bECCCTRL);    
	if (NF_ALL_FF_FLAG != (tmp0 & NF_ALL_FF_FLAG))  
	{
		tmp0 = nfreg_read(ali_nf_reg + wECCCURSEC);
		chip->ecc.steps = mtd->writesize >> 10;
		while(chip->ecc.steps)
		{
			if (!(tmp0 & 0x01))
			{
				mtd->ecc_stats.failed++;
				debug_read("uncorrect ECC error %x\n",chip->ecc.steps);
			}	
			tmp0 = tmp0>>1;
			chip->ecc.steps--;
			
		}

		if (nfreg_read(ali_nf_reg + wECCERROCR) & 0xFF)
		{
			mtd->ecc_stats.corrected += nfreg_read(ali_nf_reg + bECCSTS) & 0x1f;		
			debug_read("correctable ECC  %x\n",nfreg_read(ali_nf_reg + bECCSTS) & 0x1f);
		}	
	}

	
	
	return 0;
}

#define debug_write ALI_NAND_DEBUG
static void ali_nand_write_page_hwecc(struct mtd_info *mtd,
				struct nand_chip *chip,
				const uint8_t *buf)
{
u32 tmp0, tmp1;
unsigned long  timeo = (CONFIG_SYS_HZ * 50) / 1000;	
u32 time_start;


	debug_write("---------------------------\n");
	debug_write("------ali Write page ---------\n" );
	debug_write("---------------------------\n");

	tmp0= nfreg_read(ali_nf_reg + dwDMACONFIG);    
	tmp0 |= (INIT_DMA_IMB | INIT_DMA_SRAM);    
	nfreg_write(tmp0, (ali_nf_reg + dwDMACONFIG));  
	tmp0 &= ~(INIT_DMA_IMB | INIT_DMA_SRAM);    
	nfreg_write(tmp0, (ali_nf_reg + dwDMACONFIG));  

	nfreg_write(chip_hw_dma_address, (ali_nf_reg + dwDMAADDR));		
	tmp0= nfreg_read(ali_nf_reg + dwDMAADDR) & (~0xF0000000);	
	nfreg_write(tmp0, (ali_nf_reg + dwDMAADDR));	

	debug_write("DMA address %x\n", chip_hw_dma_address);
	if (buf != NULL)        
		memcpy(chip_hw_dma_address, buf, mtd->writesize);    

#if 0 //Fix a nand write bug: 64 bytes is writen as 0xFF after 0x20000 bytes is writen.This result in a bug: the .UBO header verify failure.
	chip->oob_poi = buf + mtd->writesize;	
	
	tmp0 = chip->oob_poi[0] | (chip->oob_poi[1] << 8) | 
		    (chip->oob_poi[2] << 16) | (chip->oob_poi[3] << 24);   
	tmp1 = chip->oob_poi[4] | (chip->oob_poi[5] << 8) | 
		    (chip->oob_poi[6] << 16) | (chip->oob_poi[7] << 24);    
#endif
	//debug
	tmp0 = 0x12345678;
	tmp1 = 0x87654321;
	
	nfreg_write(tmp0, ali_nf_reg + dwWRITEREDU0);    
	nfreg_write(tmp1, ali_nf_reg + dwWRITEREDU1);   

#ifdef ALI_NAND_DEBUG_ON
	debug_write(" data");
	int i;
	for (i=0;i<16;i++)
		debug_write(" %x", buf[i]);
	debug_write("\nredundant %x %x\n", tmp0, tmp1);	
#endif
	/*
	* set_dma_length
	*/
	tmp0 = nfreg_read(ali_nf_reg + bDMALEN);   
	tmp0 &= 0xE0;    
	tmp0 |= mtd->writesize >> 10;        
	nfreg_write(tmp0, ali_nf_reg + bDMALEN);
	
	
	/*
	* init DMA
	*/	
	nfreg_write(0, (ali_nf_reg + bDMACTRL));    
	nfreg_write(0, (ali_nf_reg + bECCSTS));    
	nfreg_write(0, (ali_nf_reg + dwINTFLAG));    

	/*
	* set DMA start (Ali chip to Nand)
	*/	
	nfreg_write(NF_DMA_IMB_EN | NF_DMA_OUT | NF_DMA_EN, 
				(ali_nf_reg + bDMACTRL));   

	/*
	* wait DMA finish
	*/
	time_start = get_timer(0);
	while (get_timer(time_start) < timeo) 
	{
		tmp0= nfreg_read(ali_nf_reg + dwINTFLAG);       
		if ( NF_DMA_DONE == (tmp0 & NF_DMA_DONE) )
		{            
			nfreg_write(0, (ali_nf_reg + dwINTFLAG)); 
			debug_write("DMA done\n");
			break;        
		}        
	}	
}


static void ali_nand_select_chip(struct mtd_info *mtd, int chips)
{
	switch (chips)
	{		
		case 0:			
			nfreg_write(~NF_CEJ, ali_nf_reg + bCTRL);				
			enable_cs(0);
			break;		
		case 1:			
			nfreg_write(NF_CEJ, ali_nf_reg + bCTRL);					
			enable_cs(1);
			break;		
		case -1:			
		case 2:		
		case 3:
		default:							
			nfreg_write(NF_CEJ, ali_nf_reg + bCTRL);						
			break;						
	}			
}


static inline void *malloc_aligned(u32 size, u32 align)
{
	return (void *)(((u32)malloc(size + align) + align - 1) &
			~(align - 1));
}

int ali_nand_init(struct nand_chip *nand)
{
        u32 tmp;

	ALI_NAND_PRINTF("ali_nand_init\n");

	nand->IO_ADDR_R = (void __iomem *) (ali_nf_reg +  bPIODATA);
	nand->IO_ADDR_W = (void __iomem *) (ali_nf_reg+ bPIODATA);	
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.size = 1024;
	nand->ecc.bytes = 28;
	/* Reference hardware control function */
	nand->cmd_ctrl  = ali_nand_hwcontrol;
	nand->ecc.read_page = ali_nand_read_page_hwecc;
	nand->ecc.write_page = ali_nand_write_page_hwecc;
	nand->select_chip = ali_nand_select_chip;	
	nand->dev_ready = NULL;
	nand->chip_delay = 1;

#if 1 //TO-Check 001:
	/* 
	*chip init
	*/
	nfreg_write(NF_EN, (ali_nf_reg + bMODE));

	/*
	* data layout, 4 bytes OOB per 1024 bytes
	*/
	tmp= nfreg_read(ali_nf_reg + bDMALEN);   
	tmp &= 0xE0;    
	tmp |= NF_FW_RED_4; 					
	nfreg_write(tmp, ali_nf_reg + bDMALEN);

	/*
	* DMA HW address
	*/
	chip_hw_dma_address = malloc_aligned(0x2000, 0x10);	
	//nfreg_write(chip_hw_dma_address, (ali_nf_reg + dwDMAADDR));	
	ALI_NAND_PRINTF("Nand DMA address is: 0x%x, length = 0x2000...\n", chip_hw_dma_address);
	
	/*
	* ECC typem, 16 or 24
	*/
	nfreg_write(NF_ECC_EN | NF_ECC_NON_STOP, ali_nf_reg + bECCCTRL);   

	/*
	* Clock seeting
	*/
	nfreg_write(0x33, ali_nf_reg + bREADCYC);
	nfreg_write(0x33, ali_nf_reg + bWRITECYC);	

	/*
	* options
	*/
	nand->options = NAND_NO_READRDY | NAND_NO_SUBPAGE_WRITE
					| NAND_USE_FLASH_BBT | NAND_NO_AUTOINCR;	
#else
        struct mtd_info *mtd = &nand_info[nand_curr_device];
        ali_nand_chip_init(mtd);
#endif
	/* test only
	for (tmp =0; tmp <=0x4C; tmp+=4){
		if (tmp!=0x48)
			printf("%x -> %08x\n", tmp, nfreg_read(ali_nf_reg+tmp));
	}	
	*/
	return 0;
}
#endif

