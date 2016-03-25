/*
 * URB OHCI HCD (Host Controller Driver) initialization for USB on ALi M39xx
 *
 * Copyright (C) 2011, ALitech.com
 *
 */
 
#include <common.h>
#include "ali-usb-power.c"

 /* functions for doing board or CPU specific setup/cleanup */
int usb_cpu_init(void)
{
	*(volatile u32 *)(0xB8000080) |= 0x10000000;	
	udelay(100);	 
	*(volatile u32 *)(0xB8000080) &= ~(0x10000000); 
		
	*(volatile u32 *)(0xB803A800 + 0x04) = 0x02900157;
	*(volatile u32 *)(0xB803A800 + 0x10) = 0x1803b000;	
	
	(*(volatile u32*)0xB803D810) = (*(volatile u32*)0xB803D810) | 0x49AB64;  //large driving slew rate
	return 0;		
}

int usb_cpu_stop(void)			{	return 0;	}
int usb_cpu_init_fail(void) 	{	return 0;	}
int usb_board_init(void)		{	usb_power_on();    	return 0;	}
int usb_board_stop(void)		{	return 0;	}
int usb_board_init_fail(void)	{	return 0;	}

