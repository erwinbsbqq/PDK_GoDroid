/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_dev.h
 *
 *  Description: Head file of smart card device
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0. 
 ****************************************************************************/

#ifndef  __ALI_SMARTCARD_DEV_H__
#define  __ALI_SMARTCARD_DEV_H__

#include "ali_smartcard.h"
#include "ali_smartcard_binding.h"

#include "ali_smartcard_txrx.h"
#include "ali_smartcard_t1.h"
#include "ali_smartcard_atr.h"
#include "ali_smartcard_pm.h"

/* Operation to Smart Card device */
extern int smc_dev_priv_request(int dev_id);
extern void smc_dev_priv_free(int dev_id);
extern void smc_dev_unconfig(struct smc_device *dev);
extern int smc_dev_config(struct smc_device *dev, struct smc_dev_cfg *cfg);
extern void smc_dev_hwcfg(struct smartcard_private *tp, int dev_id);
extern struct smc_device *smc_dev_get(int dev_id);
extern int smc_dev_register(int dev_id, dev_t dev_no, struct file_operations *f_ops);
extern int smc_dev_create(int dev_id, dev_t dev_no);
extern void smc_dev_unregister(int dev_id);
extern int smc_dev_reset(struct smc_device *dev);
extern int smc_dev_multi_class_reset(struct smc_device *dev);
extern ssize_t smc_dev_read(struct smc_device *dev, char *buf, size_t count);
extern ssize_t smc_dev_write(struct smc_device *dev, char *buf, size_t count);
extern void smc_dev_mutex_request(int dev_id);
extern void smc_dev_mutex_release(int dev_id);
extern void smc_dev_workqueue_request(int dev_id);
extern void smc_dev_workqueue_release(int dev_id);
extern int smc_dev_class_create(dev_t *dev_no);
extern void smc_dev_class_delete(void);
extern void smc_dev_set_pin(void);
extern void smc_dev_unset_pin(void);

#endif
