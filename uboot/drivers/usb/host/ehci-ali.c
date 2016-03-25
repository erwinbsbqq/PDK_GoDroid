/*
 * URB EHCI HCD (Host Controller Driver) initialization for USB on ALi M39xx
 *
 * Copyright (C) 2011, ALitech.com
 *
 */
 
#include <common.h>
#include <usb.h>

#include "ehci.h"
#include "ehci-core.h"
#include "ali-usb-power.c"

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */

int ehci_hcd_init(void)
{
int chip_id;

	chip_id = ((*(volatile u32*)0xB80000000)) >> 16;

	hccr = (struct ehci_hccr *)(0xB803A100);
	*(volatile u32 *)(0xB803A000 + 0x10) = 0x1803A100;
	hcor = (struct ehci_hcor *)((u32) hccr	+ HC_LENGTH(ehci_readl(&hccr->cr_capbase)));	

	*(volatile u32 *)(0xB803A000 + 0x04) = 0x02900157;
	*(volatile u32 *)(0xB803A000 + 0x0c) = 0x00008008;
	//*(volatile u32 *)(0xB803A000 + 0x10) = 0x1803A100;
	*(volatile u32 *)(0xB803A000 + 0x44) = 0x1000041E;
	*(volatile u32 *)(0xB803A000 + 0x40) = 0xC4000070;				
	
	

	if (chip_id == 0x3701)
	{
		//for C3701 BGA 292 pin USB setting
		(*(volatile u8*)0xB803D813) = 0x87;  
		(*(volatile u32*)0xB803D813) = 0x54954;  
	}	
	
	if (chip_id == 0x3901)
		(*(volatile u32*)0xB803D810) = (*(volatile u32*)0xB803D810) | 0x49AB64;  //large driving slew rate
	
	/* ali patch setting */
	ehci_writel(&hccr->cr_hccparams, ehci_readl(&hccr->cr_hccparams) & 0xFFFE);				//32bits mode
	ehci_writel(&hccr->cr_hcsparams, (ehci_readl(&hccr->cr_hcsparams) & 0xFFF0) | 0x3);		//three ports in M3901
		
	// Power On USB
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

