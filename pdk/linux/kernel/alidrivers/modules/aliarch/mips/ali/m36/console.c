/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation. 2010 Copyright (C)
 *  (C)
 *  File: console.c
 *  (I)
 *  Description:
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.03			Sam			Create

*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version
* 2 of the License, or (at your option) any later version.
*

 ****************************************************************************/
 
#include <linux/console.h>
#include <linux/init.h>
#include <linux/serial_reg.h>
#include <asm/io.h>

int prom_putchar(char c)
{
	/* Wait Tx empty */
	volatile unsigned char status;
	while(1){
		status = *((volatile unsigned char *)(0xB8018305));
		if(status & 0x40)
			break;
	}			

	/* Send character. */
	(*((volatile unsigned char *)(0xB8018300)) = (c));

	return 1;
}
