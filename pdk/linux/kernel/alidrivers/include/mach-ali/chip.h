/****************************************************************************(I)(S)
 *  (C)
 *  ALi (zhuhai) Corporation. 2012 Copyright (C)
 *  (C)
 *  File: chip.h
 *  (I)
 *  Description:
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2012.11.14			    barry			Create

*
****************************************************************************/

#ifndef __M36_CHIP_H
#define __M36_CHIP_H
/*
 * We should use soft link to replace the directory
 * if we want to config different board easily
 */
extern asmlinkage void hw_watchdog_reboot(void);
extern asmlinkage void load_to_icache(void *, unsigned int);

#endif