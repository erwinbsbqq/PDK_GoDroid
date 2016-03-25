/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_pm.h
 *
 *  Description: Head file of smart card power management
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0. 
 ****************************************************************************/

#ifndef  __ALI_SMARTCARD_PM_H__
#define  __ALI_SMARTCARD_PM_H__

#include "ali_smartcard.h"
#include "ali_smartcard_dev.h"
#include "ali_smartcard_misc.h"
#include "ali_smartcard_gpio.h"
#include "ali_smartcard_irq.h"

#ifdef CONFIG_PM
extern int smc_pm_suspend(struct device *dev, pm_message_t state);
extern int smc_pm_resume(struct device *dev);
#else
static inline int smc_pm_suspend(struct device *dev, pm_message_t state)
{
    return 0;
}
static inline int smc_pm_resume(struct device *dev)
{
    return 0;
}
#endif

#endif


