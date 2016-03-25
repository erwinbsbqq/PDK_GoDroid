/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_gpio.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard gpio.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#ifndef _ALI_SMARTCARD_GPIO_H_
#define _ALI_SMARTCARD_GPIO_H_

#include "ali_smartcard.h"

struct smc_gpio 
{
    UINT8 pos: 1;
    UINT8 val: 1;
    UINT8 direction: 1;
    UINT8 reserved: 5;
};

extern void smc_gpio_set_vpp(struct smartcard_private *tp);
extern int smc_gpio_set_card_detect(struct smc_device *dev);
extern void smc_gpio_unset_card_detect(struct smc_device *dev);

#endif