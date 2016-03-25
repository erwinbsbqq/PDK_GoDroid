/*
 * URB EHCI HCD (Host Controller Driver) initialization for USB on ALi M39xx
 *
 * Copyright (C) 2011, ALitech.com
 *
 */
 
#include <common.h>
#include <usb.h>

#include "../../drivers/usb/host/ehci.h"
#include "../../drivers/usb/host/ehci-core.h"
#include "ali-usb-power.c"

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
 
#define C3921	0x3921	
#define C3503 0x3503
#define C3821 0x3821
#define C3701 0x3701

struct ali_usb_setting {
	int reg_addr;   	
	int reg_val;    	 	
};

/* Define Platform-dependent data for ali-usb host */
/* c39_pkg_ver_setting
*/	
static struct ali_usb_setting C3921_XXX_V01_setting [6] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x83000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x36e836e8,},	/* phy */    
    [3] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */
    [4] ={.reg_addr = 0x20,    .reg_val  = 0x01000000,},	/* New CDR */
    [5] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};    

static struct ali_usb_setting C3503_BGA_XXX_setting [5] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x07000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x001F4A74,},	/* phy */    
    [3] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */    
    [4] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};    

static struct ali_usb_setting C3503_QFP_XXX_setting [5] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x07000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x001D4954,},	/* phy */    
    [3] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */    
    [4] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};    

static struct ali_usb_setting C3821_QFP128_XXX_setting [8] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x03000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x32E432E4,},	/* phy */    
    [3] ={.reg_addr = 0x18,    .reg_val  = 0x00000000,},	/*  */	
    [4] ={.reg_addr = 0x20,    .reg_val  = 0x01000000,},	/* New CDR */	
    [5] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */        
    [6] ={.reg_addr = 0x2C,    .reg_val  = 0x00000000,},	/* TRIM_CTRL */	
    [7] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};    

static struct ali_usb_setting C3821_QFP156_XXX_setting [8] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x07000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x32083208,},	/* phy */    
    [3] ={.reg_addr = 0x18,    .reg_val  = 0x00000000,},	/*  */	
    [4] ={.reg_addr = 0x20,    .reg_val  = 0x01000000,},	/* New CDR */	
    [5] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */        
    [6] ={.reg_addr = 0x2C,    .reg_val  = 0x00000000,},	/* TRIM_CTRL */	
    [7] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
}; 

static struct ali_usb_setting C3821_BGA_XXX_setting [8] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x03000000,},	/* pll */
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x32E432E4,},	/* phy */    
    [3] ={.reg_addr = 0x18,    .reg_val  = 0x00000000,},	/*  */	
    [4] ={.reg_addr = 0x20,    .reg_val  = 0x01000000,},	/* New CDR */	
    [5] ={.reg_addr = 0x24,    .reg_val  = 0x00080000,},	/* disconnect count */        
    [6] ={.reg_addr = 0x2C,    .reg_val  = 0x00000000,},	/* TRIM_CTRL */	
    [7] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};   

static struct ali_usb_setting C3701_XXX_V01_setting [4] = 
{
    [0] ={.reg_addr = 0x04,    .reg_val  = 0x00000000,},	/* Host Mode */
    [1] ={.reg_addr = 0x10,    .reg_val  = 0x87000000,},	/* pll */    
    [2] ={.reg_addr = 0x14,    .reg_val  = 0x00054954,},	/* phy */    
    [3] ={.reg_addr = -1,    	 .reg_val  = -1,},	/* end */
};    

int ehci_hcd_init(void)
{
struct ali_usb_setting *hw_setting;
int chip_id = 0, chip_package = 0, chip_ver =0;
int idx = 0;

#ifdef ALI_ARM_STB
	void *phy_regs = 0x1803D800;
	void *soc_regs = 0x18000000;
	void *usb_regs = 0x1803A000;
	hccr = (struct ehci_hccr *)(0x1803A100);
#else
	void *phy_regs = 0xB803D800;
	void *soc_regs = 0xB8000000;
	void *usb_regs = 0xB803A000;
	hccr = (struct ehci_hccr *)(0xB803A100);
#endif

	*(volatile u32 *)((u32) usb_regs + 0x04) = 0x02900157;
	*(volatile u32 *)((u32) usb_regs + 0x0c) = 0x00008008;		
	*(volatile u32 *)((u32) usb_regs + 0x10) = 0x1803A100;		
	hcor = (struct ehci_hcor *)((u32) hccr	+ HC_LENGTH(ehci_readl(&hccr->cr_capbase)));		
	*(volatile u32 *)((u32) usb_regs + 0x44) = 0x1000041E;
	*(volatile u32 *)((u32) usb_regs + 0x40) = 0xC4000070;				
	
	chip_id = ((*(volatile u32*)soc_regs) >> 16) & 0xFFFF;	
	chip_package = (*(volatile u32*)soc_regs >> 8) & 0xFF;	
	chip_ver = *(volatile u32*)soc_regs & 0xFF;		
	printf("chip id %x, pkt %x, ver %x hccr %x hcor %x\n", \
		chip_id, chip_package, chip_ver, (u32) hccr, (u32) hcor);	
	switch(chip_id)
	{
	case C3921:
			hw_setting = (struct ali_usb_setting *) &C3921_XXX_V01_setting;
  		break;
		
		case C3503:					
	    if ((chip_package == 0x10) && (chip_ver == 0x02))	
	    	hw_setting = (struct ali_usb_setting *) &C3503_BGA_XXX_setting;
			else					
	    	hw_setting = (struct ali_usb_setting *) &C3503_QFP_XXX_setting;
			break;
		
		case C3821:
			if ((0x01 == chip_package) || (0x00 == ((chip_package >>3) & 0x03)))	/* BGA pin*/
				hw_setting = (struct ali_usb_setting *) &C3821_BGA_XXX_setting;
			else if (0x02 == ((chip_package >>3) & 0x03) )	/* QFP156 pin*/	
				hw_setting = (struct ali_usb_setting *) &C3821_QFP156_XXX_setting;
			else																						/*QFP 128 pin*/				
				hw_setting = (struct ali_usb_setting *) &C3821_QFP128_XXX_setting;
			break;	
		
		case C3701:
			hw_setting = (struct ali_usb_setting *) &C3701_XXX_V01_setting;
			break;
			
		default:
			printf("Not Support Chip !!\n");	
			break;	
	}
	
	if (hw_setting)
	{
		while(1)
		{
			if (-1 == hw_setting[idx].reg_addr)
				break;
			ehci_writel(((u32) phy_regs + hw_setting[idx].reg_addr), hw_setting[idx].reg_val);	
			printf("Wr USB setting (0x%02x ,0x%08x) (rd back 0x%08x)\n", hw_setting[idx].reg_addr, 
					hw_setting[idx].reg_val, ehci_readl((u32) phy_regs + hw_setting[idx].reg_addr));	
			idx++;		
		}
	}			
		
	/* ali patch setting */
	ehci_writel(&hccr->cr_hccparams, ehci_readl(&hccr->cr_hccparams) & 0xFFFE);				//32bits mode
	ehci_writel(&hccr->cr_hcsparams, (ehci_readl(&hccr->cr_hcsparams) & 0xFFF0) | 0x2);		//two ports 
		
	// Power On USB
	 if (C3921 == chip_id)
		usb_power_on();    		
	return 0;	
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
	return 0;
}

