/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_misc.h
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader management.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#ifndef _ALI_SMARTCARD_MISC_H_
#define _ALI_SMARTCARD_MISC_H_

#include "ali_smartcard.h"
#include "ali_smartcard_binding.h"
#include "ali_smartcard_irq.h"
#include "ali_smartcard_txrx.h"

/* activation&deactivation timming: refer to ST8024 */
#define BASIC_T			(26<<6)
#define ATV_VCC2IO		((BASIC_T*5)>>1)
#define ATV_IO2CLK		(5)
#define DATV_RST2CLK	(BASIC_T>>1)
#define DATV_CLK2IO		(BASIC_T>>1)
#define DATV_IO2VCC		(BASIC_T>>1)

extern void smc_misc_dev_deactive(struct smc_device *dev);
extern void smc_misc_init_hw(struct smc_device *dev);
extern int smc_misc_is_card_insert(struct smc_device *dev);
extern int smc_misc_register_irq_server(struct smc_device *dev);
extern void smc_misc_unregister_irq_server(struct smc_device *dev);
extern int smc_misc_warm_reset(struct smc_device *dev);
extern int smc_misc_card_exist(struct smc_device *dev);
extern void smc_misc_set_card_flags(struct smc_device *dev, unsigned long flags);
extern void smc_misc_unset_card_flags(struct smc_device *dev, unsigned long flags);
extern INT32 smc_misc_set_pps(struct smc_device *dev, void __iomem *p,
                                  UINT8 PPS0, UINT8 FI, UINT8 DI);
extern void smc_misc_set_wclk(struct smc_device *dev, UINT32 clk);
extern void smc_misc_set_etu(struct smc_device *dev, unsigned long etu);

/* Check config param address */
extern int smc_config_param_check(UINT32 config_param);
/* Configure PIN */
extern void smc_misc_set_pin(void);
extern void smc_misc_unset_pin(void);
/* Powen management */
extern void smc_misc_power_enable(void);
extern void smc_misc_power_disable(void);

#endif
 