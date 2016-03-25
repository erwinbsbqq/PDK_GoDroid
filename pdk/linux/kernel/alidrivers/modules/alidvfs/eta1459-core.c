/*
 * eta1459-core.c  --  Device access for ETA1459
 *
 * Copyright (C) 2013 ALI Tech, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#include "eta1459-core.h"
#include "eta1459-pmic.h"


#define ETA1459_UNLOCK_KEY       0x0013
#define ETA1459_LOCK_KEY         0x0000

#define ETA1459_CLOCK_CONTROL_1  0x28
#define ETA1459_AIF_TEST	     0x74

/* debug */
#define ETA1459_BUS_DEBUG 0
#if ETA1459_BUS_DEBUG
#define dump(regs, src) do { \
	int i_; \
	u16 *src_ = src; \
	printk(KERN_DEBUG); \
	for (i_ = 0; i_ < regs; i_++) \
		printk(" 0x%4.4x", *src_++); \
	printk("\n"); \
} while (0);
#else
#define dump(bytes, src)
#endif

#define ETA1459_LOCK_DEBUG 0
#if ETA1459_LOCK_DEBUG
#define ldbg(format, arg...) printk(format, ## arg)
#else
#define ldbg(format, arg...)
#endif

/*
 * ETA1459 Device IO
 */
static DEFINE_MUTEX(io_mutex);
static DEFINE_MUTEX(reg_lock_mutex);

/* Perform a physical read from the device.
 */
static int eta1459_phys_read(struct eta1459 *eta1459, u8 reg, int num_regs,
			    u16 *dest)
{
	int ret;
	int bytes = num_regs;

	dev_dbg(eta1459->dev, "volatile read\n");
	ret = eta1459->read_dev(eta1459, reg, bytes, (char *)dest);

#if 0
	for (i = reg; i < reg + num_regs; i++) {
		/* Cache is CPU endian */
		dest[i - reg] = be16_to_cpu(dest[i - reg]);

		/* Mask out non-readable bits */
		dest[i - reg] &= eta1459_reg_io_map[i].readable;
	}
#endif
	dump(num_regs, dest);

	return ret;
}

static int eta1459_read(struct eta1459 *eta1459, u8 reg, int num_regs, u16 *dest)
{
	int i;
	int end = reg + num_regs;
	int ret = 0;
	//int bytes = num_regs;

	if (eta1459->read_dev == NULL)
		return -ENODEV;

	if ((reg + num_regs - 1) > ETA1459_MAX_REGISTER) {
		dev_err(eta1459->dev, "invalid reg %x\n",
			reg + num_regs - 1);
		return -EINVAL;
	}

	dev_dbg(eta1459->dev,
		"%s R%d(0x%2.2x) %d regs\n", __func__, reg, reg, num_regs);

#if 0 //ETA1459_BUS_DEBUG
	/* we can _safely_ read any register, but warn if read not supported */
	for (i = reg; i < end; i++) {
		if (!eta1459_reg_io_map[i].readable)
			dev_warn(eta1459->dev,
				"reg R%d is not readable\n", i);
	}
#endif

	/* if any volatile registers are required, then read back all */
	for (i = reg; i < end; i++)
		//if (eta1459_reg_io_map[i].vol)
			return eta1459_phys_read(eta1459, reg, num_regs, dest);

	/* no volatiles, then cache is good */
	dev_dbg(eta1459->dev, "cache read\n");
	//memcpy(dest, &eta1459->reg_cache[reg], bytes);
	dump(num_regs, dest);
	return ret;
}


static int eta1459_write(struct eta1459 *eta1459, u8 reg, int num_regs, u16 *src)
{
	//int i;
	//int end = reg + num_regs;
	int bytes = num_regs;

	if (eta1459->write_dev == NULL)
		return -ENODEV;

	if ((reg + num_regs - 1) > ETA1459_MAX_REGISTER) {
		dev_err(eta1459->dev, "invalid reg %x\n",
			reg + num_regs - 1);
		return -EINVAL;
	}
#if 0
	/* it's generally not a good idea to write to RO or locked registers */
	for (i = reg; i < end; i++) {
		if (!eta1459_reg_io_map[i].writable) {
			dev_err(eta1459->dev,
				"attempted write to read only reg R%d\n", i);
			return -EINVAL;
		}

		if (is_reg_locked(eta1459, i)) {
			dev_err(eta1459->dev,
			       "attempted write to locked reg R%d\n", i);
			return -EINVAL;
		}

		src[i - reg] &= eta1459_reg_io_map[i].writable;

		eta1459->reg_cache[i] =
			(eta1459->reg_cache[i] & ~eta1459_reg_io_map[i].writable)
			| src[i - reg];

		src[i - reg] = cpu_to_be16(src[i - reg]);
	}
#endif
	/* Actually write it out */
	return eta1459->write_dev(eta1459, reg, bytes, (char *)src);
}

/*
 * Safe read, modify, write methods
 */
int eta1459_clear_bits(struct eta1459 *eta1459, u16 reg, u16 mask)
{
	u16 data = 0;
	int err;

	mutex_lock(&io_mutex);
	err = eta1459_read(eta1459, reg, 1, &data);
	if (err) {
		dev_err(eta1459->dev, "read from reg R%d failed\n", reg);
		goto out;
	}

	data &= ~mask;
	err = eta1459_write(eta1459, reg, 1, &data);
	if (err)
		dev_err(eta1459->dev, "write to reg R%d failed\n", reg);
out:
	mutex_unlock(&io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(eta1459_clear_bits);

int eta1459_set_bits(struct eta1459 *eta1459, u16 reg, u16 mask)
{
	u16 data = 0;
	int err;

	mutex_lock(&io_mutex);
	err = eta1459_read(eta1459, reg, 1, &data);
	if (err) {
		dev_err(eta1459->dev, "read from reg R%d failed\n", reg);
		goto out;
	}

	data |= mask;
	err = eta1459_write(eta1459, reg, 1, &data);
	if (err)
		dev_err(eta1459->dev, "write to reg R%d failed\n", reg);
out:
	mutex_unlock(&io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(eta1459_set_bits);

u16 eta1459_reg_read(struct eta1459 *eta1459, int reg)
{
	u16 data = 0;
	int err;

	mutex_lock(&io_mutex);
	err = eta1459_read(eta1459, reg, 1, &data);
	if (err)
		dev_err(eta1459->dev, "read from reg R%d failed\n", reg);

	mutex_unlock(&io_mutex);
	return data;
}
EXPORT_SYMBOL_GPL(eta1459_reg_read);

int eta1459_reg_write(struct eta1459 *eta1459, int reg, u16 val)
{
	int ret;
	u16 data = val;

	mutex_lock(&io_mutex);
	ret = eta1459_write(eta1459, reg, 1, &data);
	if (ret)
		dev_err(eta1459->dev, "write to reg R%d failed\n", reg);
	mutex_unlock(&io_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(eta1459_reg_write);

int eta1459_block_read(struct eta1459 *eta1459, int start_reg, int regs,
		      u16 *dest)
{
	int err = 0;

	mutex_lock(&io_mutex);
	err = eta1459_read(eta1459, start_reg, regs, dest);
	if (err)
		dev_err(eta1459->dev, "block read starting from R%d failed\n",
			start_reg);
	mutex_unlock(&io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(eta1459_block_read);

int eta1459_block_write(struct eta1459 *eta1459, int start_reg, int regs,
		       u16 *src)
{
	int ret = 0;

	mutex_lock(&io_mutex);
	ret = eta1459_write(eta1459, start_reg, regs, src);
	if (ret)
		dev_err(eta1459->dev, "block write starting at R%d failed\n",
			start_reg);
	mutex_unlock(&io_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(eta1459_block_write);

/**
 * eta1459_reg_lock()
 *
 * The ETA1459 has a hardware lock which can be used to prevent writes to
 * some registers (generally those which can cause particularly serious
 * problems if misused).  This function enables that lock.
 */
int eta1459_reg_lock(struct eta1459 *eta1459)
{
	u16 key = ETA1459_LOCK_KEY;
	int ret;

	ldbg(__func__);
	mutex_lock(&io_mutex);
	ret = eta1459_write(eta1459, ETA1459_SECURITY, 1, &key);
	if (ret)
		dev_err(eta1459->dev, "lock failed\n");
	mutex_unlock(&io_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(eta1459_reg_lock);

/**
 * eta1459_reg_unlock()
 *
 * The ETA1459 has a hardware lock which can be used to prevent writes to
 * some registers (generally those which can cause particularly serious
 * problems if misused).  This function disables that lock so updates
 * can be performed.  For maximum safety this should be done only when
 * required.
 */
int eta1459_reg_unlock(struct eta1459 *eta1459)
{
	u16 key = ETA1459_UNLOCK_KEY;
	int ret;

	ldbg(__func__);
	mutex_lock(&io_mutex);
	ret = eta1459_write(eta1459, ETA1459_SECURITY, 1, &key);
	if (ret)
		dev_err(eta1459->dev, "unlock failed\n");
	mutex_unlock(&io_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(eta1459_reg_unlock);



/*
 * Register a client device.  This is non-fatal since there is no need to
 * fail the entire device init due to a single platform device failing.
 */
 #if 0
static void eta1459_client_dev_register(struct eta1459 *eta1459,
				       const char *name,
				       struct platform_device **pdev)
{
	int ret;

	*pdev = platform_device_alloc(name, -1);
	if (*pdev == NULL) {
		dev_err(eta1459->dev, "Failed to allocate %s\n", name);
		return;
	}

	(*pdev)->dev.parent = eta1459->dev;
	platform_set_drvdata(*pdev, eta1459);
	ret = platform_device_add(*pdev);
	if (ret != 0) {
		dev_err(eta1459->dev, "Failed to register %s: %d\n", name, ret);
		platform_device_put(*pdev);
		*pdev = NULL;
	}
}
#endif

int eta1459_device_init(struct eta1459 *eta1459, int irq,
		       struct eta1459_platform_data *pdata)
{
    int ret = 0;
    //u16 id1, id2, mask_rev;
    //u16 cust_id, mode, chip_rev;

    //dev_set_drvdata(eta1459->dev, eta1459);

    //printk("\n\neta1459_device_init:FILE:%s,LINE:%d zqs\n\n",__FILE__,__LINE__);


    if (pdata && pdata->init) 
    {
        ret = pdata->init(eta1459);
        if (ret != 0)
        {
            dev_err(eta1459->dev, "Platform init() failed: %d, system do not support eta1459 voltage regulator \n", ret);
            goto err;
        }
    }

    return 0;
    
err:
    return ret;
}
EXPORT_SYMBOL_GPL(eta1459_device_init);

void eta1459_device_exit(struct eta1459 *eta1459)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(eta1459->pmic.pdev); i++)
		platform_device_unregister(eta1459->pmic.pdev[i]);
#if 0
	platform_device_unregister(eta1459->wdt.pdev);
	platform_device_unregister(eta1459->rtc.pdev);
	platform_device_unregister(eta1459->power.pdev);
	platform_device_unregister(eta1459->hwmon.pdev);
	platform_device_unregister(eta1459->gpio.pdev);
	platform_device_unregister(eta1459->codec.pdev);
#endif    
}
EXPORT_SYMBOL_GPL(eta1459_device_exit);

MODULE_DESCRIPTION("ETA1459 PMIC core driver");
MODULE_LICENSE("GPL");
