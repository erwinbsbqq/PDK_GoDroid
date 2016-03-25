/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_irq.h
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader irq.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#ifndef _ALI_SMARTCARD_IRQ_H_
#define _ALI_SMARTCARD_IRQ_H_

#include "ali_smartcard.h"
#include "ali_smartcard_misc.h"
#include "ali_smartcard_txrx.h"

#define SMC_IRQ_NATURAL_STATUS	0X01
#define SMC_IRQ_FORCE_STATUS	0X02

extern irqreturn_t smc_irq_gpio_detect(int irq, void *param);
extern irqreturn_t smc_irq_dev_interrupt(int irq, void *param);
extern void smc_irq_set_monitor_status(int status);
extern int smc_irq_get_monitor_status(void);

#endif