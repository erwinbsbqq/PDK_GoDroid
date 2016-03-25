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

extern u16 Get_AliChipGen();

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */

int ehci_hcd_init(void)
{
  u16 ChipGen = Get_AliChipGen();
  
	hccr = (struct ehci_hccr *)(0xB803A100);
	*(volatile u32 *)(0xB803A000 + 0x10) = 0x1803A100;
	hcor = (struct ehci_hcor *)((u32) hccr	+ HC_LENGTH(ehci_readl(&hccr->cr_capbase)));	

       if (0x3901 == ChipGen)
        {
        	*(volatile u32 *)(0xB803A000 + 0x04) = 0x02900157;
        	*(volatile u32 *)(0xB803A000 + 0x0c) = 0x00008008;
        	//*(volatile u32 *)(0xB803A000 + 0x10) = 0x1803A100;
        	*(volatile u32 *)(0xB803A000 + 0x44) = 0x1000041E;
        	*(volatile u32 *)(0xB803A000 + 0x40) = 0xC4000071;				
        	
        	(*(volatile u32*)0xB803D810) = (*(volatile u32*)0xB803D810) | 0x49AB64;  //large driving slew rate
        }
       else if (0x3701 == ChipGen)
        {
        	void *pci_regs = 0xB803A000;
        	
        	ehci_writel((pci_regs + 0x04),0x02900157);
        	ehci_writel((pci_regs + 0x0c),0x00008008); //0x00008010;
        	//ehci_writel((pci_regs + 0x10),0x1803A100);
        	ehci_writel((pci_regs + 0x44),0x1000041E);
        	ehci_writel((pci_regs + 0x40),0xC4000071);//for fpga  // 0x20000031
        	ehci_writel(0xb803d818,0x0);	//for C3701C
        	if((ehci_readl(0xB8000000) & 0xFFFF0000) == 0x37010000)
        	{		
        		(*(volatile u32*)0xB803D814) = 0x0005C9D4;;	//  20120420 change GPSR14 register (Address=0xB803_D814)  to 0005_4954h, for RX squelch level
        		(*(volatile u8*)0xB803D813) = 0x87;
        		udelay(200);		
        	}

        	if((ehci_readl(0xB8000000) & 0xFFFF0000) == 0x37010000)
        	{	
        		printf("[ali_start_ehc] PLL reg 10 = 0x%4x\n", ehci_readl(0xb803d810));			
        	}
        }

	/* ali patch setting */
	ehci_writel(&hccr->cr_hccparams, ehci_readl(&hccr->cr_hccparams) & 0xFFFE);				//32bits mode
	ehci_writel(&hccr->cr_hcsparams, (ehci_readl(&hccr->cr_hcsparams) & 0xFFF0) | 0x3);		//three ports in M3901
		

	// Power On USB
       if (0x3901 == ChipGen)
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

