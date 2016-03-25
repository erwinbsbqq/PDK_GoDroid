/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_pm.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard power management.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/
#include "ali_smartcard_config.h"
#include "ali_smartcard_pm.h"

/*
 * Suspend and resume interface
 */
int smc_pm_suspend(struct device *dev, pm_message_t state)
{
    struct smc_device *p_smc_dev = dev_get_drvdata(dev);

	if (NULL == p_smc_dev || NULL == p_smc_dev->priv) return 0;
    smc_debug("SMC MNGT: suspend %s\n", p_smc_dev->dev_name);
    smc_misc_dev_deactive(p_smc_dev);
    smc_dev_unset_pin();
	smc_dev_unconfig(p_smc_dev);
    smc_irq_set_monitor_status(SMC_IRQ_FORCE_STATUS);
    return 0;
}

int smc_pm_resume(struct device *dev)
{
    struct smc_device *p_smc_dev = dev_get_drvdata(dev);

	if (NULL == p_smc_dev || NULL == p_smc_dev->priv) return 0;
    smc_debug("SMC MNGT: resume %s\n", p_smc_dev->dev_name);
    smc_dev_set_pin();
    smc_dev_config(p_smc_dev, &config_param_def);
    return 0;
}