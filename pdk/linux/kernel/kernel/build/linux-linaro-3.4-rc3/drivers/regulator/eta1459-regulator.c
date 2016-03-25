/*
 * eta1459.c  --  Voltage and current regulation for the Wolfson ETA1459 PMIC
 *
 * Copyright 2007, 2008 Wolfson Microelectronics PLC.
 *
 * Author: Liam Girdwood
 *         linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/mfd/eta1459/core.h>
#include <linux/mfd/eta1459/pmic.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

/* Maximum value possible for VSEL */
#define ETA1459_DCDC_MAX_VSEL 0x66

#define ALI_DVFS_VOLT_IDX       (1.3529 * 1000)

#define ALI_DVFS_TABLE_SIZE		64

static const int ali_dvfs_volt_map[ALI_DVFS_TABLE_SIZE] = 
{
    0.696 * ALI_DVFS_VOLT_IDX , 0.704 * ALI_DVFS_VOLT_IDX, 0.712 * ALI_DVFS_VOLT_IDX, 0.72 * ALI_DVFS_VOLT_IDX, 0.728 * ALI_DVFS_VOLT_IDX, 0.736 * ALI_DVFS_VOLT_IDX, 0.744 * ALI_DVFS_VOLT_IDX,  0.752 * ALI_DVFS_VOLT_IDX, 
    0.76 * ALI_DVFS_VOLT_IDX, 0.768 * ALI_DVFS_VOLT_IDX , 0.776 * ALI_DVFS_VOLT_IDX, 0.784 * ALI_DVFS_VOLT_IDX, 0.792 * ALI_DVFS_VOLT_IDX, 0.8 * ALI_DVFS_VOLT_IDX, 0.808 * ALI_DVFS_VOLT_IDX,  0.816 * ALI_DVFS_VOLT_IDX, 
    0.824 * ALI_DVFS_VOLT_IDX, 0.832 * ALI_DVFS_VOLT_IDX ,  0.84 * ALI_DVFS_VOLT_IDX, 0.848 * ALI_DVFS_VOLT_IDX, 0.856 * ALI_DVFS_VOLT_IDX, 0.864 * ALI_DVFS_VOLT_IDX, 0.872 * ALI_DVFS_VOLT_IDX,  0.88 * ALI_DVFS_VOLT_IDX, 
    0.888 * ALI_DVFS_VOLT_IDX, 0.896 * ALI_DVFS_VOLT_IDX ,  0.904 * ALI_DVFS_VOLT_IDX, 0.912 * ALI_DVFS_VOLT_IDX, 0.92 * ALI_DVFS_VOLT_IDX, 0.928 * ALI_DVFS_VOLT_IDX, 0.936 * ALI_DVFS_VOLT_IDX,  0.944 * ALI_DVFS_VOLT_IDX, 

    0.952 * ALI_DVFS_VOLT_IDX, 0.96 * ALI_DVFS_VOLT_IDX ,  0.968 * ALI_DVFS_VOLT_IDX, 0.976 * ALI_DVFS_VOLT_IDX, 0.984 * ALI_DVFS_VOLT_IDX, 0.992 * ALI_DVFS_VOLT_IDX, 1 * ALI_DVFS_VOLT_IDX,  1.008 * ALI_DVFS_VOLT_IDX, 
    1.016 * ALI_DVFS_VOLT_IDX, 1.024 * ALI_DVFS_VOLT_IDX ,  1.032 * ALI_DVFS_VOLT_IDX, 1.04 * ALI_DVFS_VOLT_IDX, 1.048 * ALI_DVFS_VOLT_IDX, 1.056 * ALI_DVFS_VOLT_IDX, 1.064 * ALI_DVFS_VOLT_IDX,  1.072 * ALI_DVFS_VOLT_IDX, 
    1.08 * ALI_DVFS_VOLT_IDX, 1.088 * ALI_DVFS_VOLT_IDX ,  1.096 * ALI_DVFS_VOLT_IDX, 1.104 * ALI_DVFS_VOLT_IDX, 1.112 * ALI_DVFS_VOLT_IDX, 1.12 * ALI_DVFS_VOLT_IDX, 1.128 * ALI_DVFS_VOLT_IDX, 1.136 * ALI_DVFS_VOLT_IDX,  
    1.144 * ALI_DVFS_VOLT_IDX, 1.152 * ALI_DVFS_VOLT_IDX, 1.16 * ALI_DVFS_VOLT_IDX ,  1.168 * ALI_DVFS_VOLT_IDX, 1.176 * ALI_DVFS_VOLT_IDX, 1.184* ALI_DVFS_VOLT_IDX, 1.192 * ALI_DVFS_VOLT_IDX, 1.2 * ALI_DVFS_VOLT_IDX
};

static inline int eta1459_dcdc_val_to_mvolts(unsigned int val)
{
	return (val * 25) + 850;
}

extern unsigned int g_ali_cpufreq_debug_ctrl;

static int eta1459_dcdc_set_voltage(struct regulator_dev *rdev, int min_uV,
				   int max_uV, unsigned *selector)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	//int volt_reg;
	//int dcdc = rdev_get_id(rdev);
	//int mV;
	int min_mV = min_uV / 1000;
	int max_mV = max_uV / 1000;
    
	u16 val;
	u8 reg_data=0;
	//u8 step=0;

	int i = 0;
    int j = 0;

	if (min_mV < 900 || min_mV > 1600)
		return -EINVAL;
	if (max_mV < 900 || max_mV > 1600)
		return -EINVAL;

	//printk("eta1459_dcdc_set_voltage:min_uv:%d max_uv:%d\n\n",min_uV,max_uV);
                
	for (i = 0; i< ALI_DVFS_TABLE_SIZE; i++)    
    {
        if (min_mV <=  ali_dvfs_volt_map[i])
        {
            break;        
        }        
    }

    j = i;
    if ( i > 0)
    {
        i--;
    }

    //printk("eta1459_dcdc_set_voltage:volt:%d,   reg_data:0x%x\n\n",  ali_dvfs_volt_map[i], reg_data);

    reg_data = (u8)(i << 2);
        
#if 0        
    step = (min_mV - 900) / 13;
        
    reg_data = step << 2;
#endif

	/* all DCDCs have same mV bits */
	
	eta1459_reg_write(eta1459, 0x01, reg_data & 0xff);
	
	//val = eta1459_reg_read(eta1459, 0x01)&0xff;

    if (g_ali_cpufreq_debug_ctrl != 0)
    {
        val = eta1459_reg_read(eta1459, 0x01) & 0xff;
        printk(KERN_ALERT"\n*ARM CA9 Core Demand Voltage = %d mv, ETA1459 Actual Voltage = %d mv,  ETA1459 value = 0x%x*\n",  min_mV, ali_dvfs_volt_map[j], (val >> 2));
    }
	
	//printk("eta1459_dcdc_set_voltage:reg:0x01 read_data:0x%x\n\n",val);

	return 0;
}

static int eta1459_dcdc_get_voltage_sel(struct regulator_dev *rdev)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int volt_reg, dcdc = rdev_get_id(rdev);

	switch (dcdc) {
	case ETA1459_DCDC_1:
		volt_reg = ETA1459_DCDC1_CONTROL;
		break;
        
	default:
		return -EINVAL;
	}

	/* all DCDCs have same mV bits */
	return eta1459_reg_read(eta1459, volt_reg) & ETA1459_DC1_VSEL_MASK;
}

static int eta1459_dcdc_list_voltage(struct regulator_dev *rdev,
				    unsigned selector)
{
	if (selector > ETA1459_DCDC_MAX_VSEL)
		return -EINVAL;
	return eta1459_dcdc_val_to_mvolts(selector) * 1000;
}

static int eta1459_dcdc_set_suspend_voltage(struct regulator_dev *rdev, int uV)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int volt_reg, mV = uV / 1000, dcdc = rdev_get_id(rdev);
	u16 val;

	dev_dbg(eta1459->dev, "%s %d mV %d\n", __func__, dcdc, mV);

	if (mV && (mV < 850 || mV > 4025)) {
		dev_err(eta1459->dev,
			"DCDC%d suspend voltage %d mV out of range\n",
			dcdc, mV);
		return -EINVAL;
	}
	if (mV == 0)
		mV = 850;

	switch (dcdc) {
	case ETA1459_DCDC_1:
		volt_reg = ETA1459_DCDC1_LOW_POWER;
		break;
        
	default:
		return -EINVAL;
	}

	/* all DCDCs have same mV bits */
	val = eta1459_reg_read(eta1459, volt_reg) & ~ETA1459_DC1_VSEL_MASK;
	eta1459_reg_write(eta1459, volt_reg, val | mV);
	return 0;
}

static int eta1459_dcdc_set_suspend_enable(struct regulator_dev *rdev)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 val;

	switch (dcdc) {
	case ETA1459_DCDC_1:
		val = eta1459_reg_read(eta1459, ETA1459_DCDC1_LOW_POWER)
			& ~ETA1459_DCDC_HIB_MODE_MASK;
		eta1459_reg_write(eta1459, ETA1459_DCDC1_LOW_POWER,
			val | eta1459->pmic.dcdc1_hib_mode);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int eta1459_dcdc_set_suspend_disable(struct regulator_dev *rdev)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 val;

	switch (dcdc) {
	case ETA1459_DCDC_1:
		val = eta1459_reg_read(eta1459, ETA1459_DCDC1_LOW_POWER);
		eta1459->pmic.dcdc1_hib_mode = val & ETA1459_DCDC_HIB_MODE_MASK;
		eta1459_reg_write(eta1459, ETA1459_DCDC1_LOW_POWER,
				 val | ETA1459_DCDC_HIB_MODE_DIS);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int eta1459_dcdc_set_suspend_mode(struct regulator_dev *rdev,
	unsigned int mode)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 *hib_mode;

	switch (dcdc) {
	case ETA1459_DCDC_1:
		hib_mode = &eta1459->pmic.dcdc1_hib_mode;
		break;
        
	default:
		return -EINVAL;
	}

	switch (mode) {
	case REGULATOR_MODE_NORMAL:
		*hib_mode = ETA1459_DCDC_HIB_MODE_IMAGE;
		break;
	case REGULATOR_MODE_IDLE:
		*hib_mode = ETA1459_DCDC_HIB_MODE_STANDBY;
		break;
	case REGULATOR_MODE_STANDBY:
		*hib_mode = ETA1459_DCDC_HIB_MODE_LDO_IM;
		break;
	default:
		return -EINVAL;
	}
    
	return 0;
}






static int eta1459_dcdc_enable(struct regulator_dev *rdev)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 shift;

	if (dcdc < ETA1459_DCDC_1 || dcdc > ETA1459_DCDC_6)
		return -EINVAL;

	shift = dcdc - ETA1459_DCDC_1;
	eta1459_set_bits(eta1459, ETA1459_DCDC_LDO_REQUESTED, 1 << shift);
	return 0;
}

static int eta1459_dcdc_disable(struct regulator_dev *rdev)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 shift;

	if (dcdc < ETA1459_DCDC_1 || dcdc > ETA1459_DCDC_6)
		return -EINVAL;

	shift = dcdc - ETA1459_DCDC_1;
	eta1459_clear_bits(eta1459, ETA1459_DCDC_LDO_REQUESTED, 1 << shift);

	return 0;
}



static int force_continuous_enable(struct eta1459 *eta1459, int dcdc, int enable)
{
	int reg = 0, ret;

	switch (dcdc) {
	case ETA1459_DCDC_1:
		reg = ETA1459_DCDC1_FORCE_PWM;
		break;
	case ETA1459_DCDC_3:
		reg = ETA1459_DCDC3_FORCE_PWM;
		break;
	case ETA1459_DCDC_4:
		reg = ETA1459_DCDC4_FORCE_PWM;
		break;
	case ETA1459_DCDC_6:
		reg = ETA1459_DCDC6_FORCE_PWM;
		break;
	default:
		return -EINVAL;
	}

	if (enable)
		ret = eta1459_set_bits(eta1459, reg,
			ETA1459_DCDC1_FORCE_PWM_ENA);
	else
		ret = eta1459_clear_bits(eta1459, reg,
			ETA1459_DCDC1_FORCE_PWM_ENA);
	return ret;
}

static int eta1459_dcdc_set_mode(struct regulator_dev *rdev, unsigned int mode)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 val;

	if (dcdc < ETA1459_DCDC_1 || dcdc > ETA1459_DCDC_6)
		return -EINVAL;

	if (dcdc == ETA1459_DCDC_2 || dcdc == ETA1459_DCDC_5)
		return -EINVAL;

	val = 1 << (dcdc - ETA1459_DCDC_1);

	switch (mode) {
	case REGULATOR_MODE_FAST:
		/* force continuous mode */
		eta1459_set_bits(eta1459, ETA1459_DCDC_ACTIVE_OPTIONS, val);
		eta1459_clear_bits(eta1459, ETA1459_DCDC_SLEEP_OPTIONS, val);
		force_continuous_enable(eta1459, dcdc, 1);
		break;
	case REGULATOR_MODE_NORMAL:
		/* active / pulse skipping */
		eta1459_set_bits(eta1459, ETA1459_DCDC_ACTIVE_OPTIONS, val);
		eta1459_clear_bits(eta1459, ETA1459_DCDC_SLEEP_OPTIONS, val);
		force_continuous_enable(eta1459, dcdc, 0);
		break;
	case REGULATOR_MODE_IDLE:
		/* standby mode */
		force_continuous_enable(eta1459, dcdc, 0);
		eta1459_clear_bits(eta1459, ETA1459_DCDC_SLEEP_OPTIONS, val);
		eta1459_clear_bits(eta1459, ETA1459_DCDC_ACTIVE_OPTIONS, val);
		break;
	case REGULATOR_MODE_STANDBY:
		/* LDO mode */
		force_continuous_enable(eta1459, dcdc, 0);
		eta1459_set_bits(eta1459, ETA1459_DCDC_SLEEP_OPTIONS, val);
		break;
	}

	return 0;
}

static unsigned int eta1459_dcdc_get_mode(struct regulator_dev *rdev)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev);
	u16 mask, sleep, active, force;
	int mode = REGULATOR_MODE_NORMAL;
	int reg;

	switch (dcdc) {
	case ETA1459_DCDC_1:
		reg = ETA1459_DCDC1_FORCE_PWM;
		break;
	case ETA1459_DCDC_3:
		reg = ETA1459_DCDC3_FORCE_PWM;
		break;
	case ETA1459_DCDC_4:
		reg = ETA1459_DCDC4_FORCE_PWM;
		break;
	case ETA1459_DCDC_6:
		reg = ETA1459_DCDC6_FORCE_PWM;
		break;
	default:
		return -EINVAL;
	}

	mask = 1 << (dcdc - ETA1459_DCDC_1);
	active = eta1459_reg_read(eta1459, ETA1459_DCDC_ACTIVE_OPTIONS) & mask;
	force = eta1459_reg_read(eta1459, reg) & ETA1459_DCDC1_FORCE_PWM_ENA;
	sleep = eta1459_reg_read(eta1459, ETA1459_DCDC_SLEEP_OPTIONS) & mask;

	dev_dbg(eta1459->dev, "mask %x active %x sleep %x force %x",
		mask, active, sleep, force);

	if (active && !sleep) {
		if (force)
			mode = REGULATOR_MODE_FAST;
		else
			mode = REGULATOR_MODE_NORMAL;
	} else if (!active && !sleep)
		mode = REGULATOR_MODE_IDLE;
	else if (sleep)
		mode = REGULATOR_MODE_STANDBY;

	return mode;
}



struct eta1459_dcdc_efficiency {
	int uA_load_min;
	int uA_load_max;
	unsigned int mode;
};

static const struct eta1459_dcdc_efficiency dcdc1_6_efficiency[] = {
	{0, 10000, REGULATOR_MODE_STANDBY},       /* 0 - 10mA - LDO */
	{10000, 100000, REGULATOR_MODE_IDLE},     /* 10mA - 100mA - Standby */
	{100000, 1000000, REGULATOR_MODE_NORMAL}, /* > 100mA - Active */
	{-1, -1, REGULATOR_MODE_NORMAL},
};

static const struct eta1459_dcdc_efficiency dcdc3_4_efficiency[] = {
	{0, 10000, REGULATOR_MODE_STANDBY},      /* 0 - 10mA - LDO */
	{10000, 100000, REGULATOR_MODE_IDLE},    /* 10mA - 100mA - Standby */
	{100000, 800000, REGULATOR_MODE_NORMAL}, /* > 100mA - Active */
	{-1, -1, REGULATOR_MODE_NORMAL},
};

static unsigned int get_mode(int uA, const struct eta1459_dcdc_efficiency *eff)
{
	int i = 0;

	while (eff[i].uA_load_min != -1) {
		if (uA >= eff[i].uA_load_min && uA <= eff[i].uA_load_max)
			return eff[i].mode;
	}
	return REGULATOR_MODE_NORMAL;
}

/* Query the regulator for it's most efficient mode @ uV,uA
 * ETA1459 regulator efficiency is pretty similar over
 * different input and output uV.
 */
static unsigned int eta1459_dcdc_get_optimum_mode(struct regulator_dev *rdev,
						 int input_uV, int output_uV,
						 int output_uA)
{
	int dcdc = rdev_get_id(rdev), mode;

	switch (dcdc) {
	case ETA1459_DCDC_1:
	case ETA1459_DCDC_6:
		mode = get_mode(output_uA, dcdc1_6_efficiency);
		break;
	case ETA1459_DCDC_3:
	case ETA1459_DCDC_4:
		mode = get_mode(output_uA, dcdc3_4_efficiency);
		break;
	default:
		mode = REGULATOR_MODE_NORMAL;
		break;
	}
	return mode;
}

static int eta1459_dcdc_is_enabled(struct regulator_dev *rdev)
{
	struct eta1459 *eta1459 = rdev_get_drvdata(rdev);
	int dcdc = rdev_get_id(rdev), shift;

	if (dcdc < ETA1459_DCDC_1 || dcdc > ETA1459_DCDC_6)
		return -EINVAL;

	shift = dcdc - ETA1459_DCDC_1;
	return eta1459_reg_read(eta1459, ETA1459_DCDC_LDO_REQUESTED)
	    & (1 << shift);
}



static struct regulator_ops eta1459_dcdc_ops = {
	.set_voltage = eta1459_dcdc_set_voltage,
	.get_voltage_sel = eta1459_dcdc_get_voltage_sel,
	.list_voltage = eta1459_dcdc_list_voltage,
	.enable = eta1459_dcdc_enable,
	.disable = eta1459_dcdc_disable,
	.get_mode = eta1459_dcdc_get_mode,
	.set_mode = eta1459_dcdc_set_mode,
	.get_optimum_mode = eta1459_dcdc_get_optimum_mode,
	.is_enabled = eta1459_dcdc_is_enabled,
	.set_suspend_voltage = eta1459_dcdc_set_suspend_voltage,
	.set_suspend_enable = eta1459_dcdc_set_suspend_enable,
	.set_suspend_disable = eta1459_dcdc_set_suspend_disable,
	.set_suspend_mode = eta1459_dcdc_set_suspend_mode,
};


static struct regulator_desc eta1459_reg[NUM_ETA1459_REGULATORS] = {
	{
		.name = "DCDC1",
		.id = ETA1459_DCDC_1,
		.ops = &eta1459_dcdc_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = ETA1459_DCDC_MAX_VSEL + 1,
		.owner = THIS_MODULE,
	},
};

static int eta1459_regulator_probe(struct platform_device *pdev)
{
    //struct eta1459 *eta1459 = dev_get_drvdata(&pdev->dev);
    struct regulator_dev *rdev;
	//int ret;
	//u16 val;

	if (pdev->id < ETA1459_DCDC_1 || pdev->id > ETA1459_ISINK_B)
		return -ENODEV;
#if 0
	/* do any regulatior specific init */
	switch (pdev->id) {
	case ETA1459_DCDC_1:
		val = eta1459_reg_read(eta1459, ETA1459_DCDC1_LOW_POWER);
		eta1459->pmic.dcdc1_hib_mode = val & ETA1459_DCDC_HIB_MODE_MASK;
		break;
	case ETA1459_DCDC_3:
		val = eta1459_reg_read(eta1459, ETA1459_DCDC3_LOW_POWER);
		eta1459->pmic.dcdc3_hib_mode = val & ETA1459_DCDC_HIB_MODE_MASK;
		break;
	case ETA1459_DCDC_4:
		val = eta1459_reg_read(eta1459, ETA1459_DCDC4_LOW_POWER);
		eta1459->pmic.dcdc4_hib_mode = val & ETA1459_DCDC_HIB_MODE_MASK;
		break;
	case ETA1459_DCDC_6:
		val = eta1459_reg_read(eta1459, ETA1459_DCDC6_LOW_POWER);
		eta1459->pmic.dcdc6_hib_mode = val & ETA1459_DCDC_HIB_MODE_MASK;
		break;
	}
#endif
	/* register regulator */
	rdev = regulator_register(&eta1459_reg[pdev->id], &pdev->dev,
				  pdev->dev.platform_data,
				  dev_get_drvdata(&pdev->dev), NULL);
	if (IS_ERR(rdev)) {
		dev_err(&pdev->dev, "failed to register %s\n",
			eta1459_reg[pdev->id].name);
		return PTR_ERR(rdev);
	}

	return 0;
}

static int eta1459_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);

    //struct eta1459 *eta1459 = rdev_get_drvdata(rdev);

	//eta1459_free_irq(eta1459, eta1459_reg[pdev->id].irq, rdev);

	regulator_unregister(rdev);

	return 0;
}

int eta1459_register_regulator(struct eta1459 *eta1459, int reg,
			      struct regulator_init_data *initdata)
{
	struct platform_device *pdev;
	int ret;
	if (reg < 0 || reg >= NUM_ETA1459_REGULATORS)
		return -EINVAL;

	if (eta1459->pmic.pdev[reg])
		return -EBUSY;

	if (reg >= ETA1459_DCDC_1 && reg <= ETA1459_DCDC_6 &&
	    reg > eta1459->pmic.max_dcdc)
		return -ENODEV;
	if (reg >= ETA1459_ISINK_A && reg <= ETA1459_ISINK_B &&
	    reg > eta1459->pmic.max_isink)
		return -ENODEV;

	pdev = platform_device_alloc("eta1459-regulator", reg);
	if (!pdev)
		return -ENOMEM;

	eta1459->pmic.pdev[reg] = pdev;

	initdata->driver_data = eta1459;

	pdev->dev.platform_data = initdata;
	pdev->dev.parent = eta1459->dev;
	platform_set_drvdata(pdev, eta1459);

	ret = platform_device_add(pdev);

	if (ret != 0) {
		dev_err(eta1459->dev, "Failed to register regulator %d: %d\n",
			reg, ret);
		platform_device_put(pdev);
		eta1459->pmic.pdev[reg] = NULL;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(eta1459_register_regulator);



static struct platform_driver eta1459_regulator_driver = {
	.probe = eta1459_regulator_probe,
	.remove = eta1459_regulator_remove,
	.driver		= {
		.name	= "eta1459-regulator",
	},
};

static int __init eta1459_regulator_init(void)
{
	return platform_driver_register(&eta1459_regulator_driver);
}
subsys_initcall(eta1459_regulator_init);

static void __exit eta1459_regulator_exit(void)
{
	platform_driver_unregister(&eta1459_regulator_driver);
}
module_exit(eta1459_regulator_exit);

/* Module information */
MODULE_AUTHOR("Kinson Zhou");
MODULE_DESCRIPTION("ETA1459 voltage and current regulator driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:eta1459-regulator");
