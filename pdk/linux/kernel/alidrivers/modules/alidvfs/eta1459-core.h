/*
 * core.h  --  Core Driver for Wolfson ETA1459 PMIC
 *
 * Copyright 2007 Wolfson Microelectronics PLC
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#ifndef __LINUX_MFD_ETA1459_CORE_H_
#define __LINUX_MFD_ETA1459_CORE_H_

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/completion.h>

#include "eta1459-pmic.h"

//#include <linux/mfd/eta1459/audio.h>
//#include <linux/mfd/eta1459/gpio.h>
//#include <linux/mfd/eta1459/rtc.h>
//#include <linux/mfd/eta1459/supply.h>
//#include <linux/mfd/eta1459/wdt.h>

/*
 * Register values.
 */
#define ETA1459_RESET_ID                         0x00
#define ETA1459_ID                               0x01
#define ETA1459_REVISION				0x02
#define ETA1459_SYSTEM_CONTROL_1                 0x03
#define ETA1459_SYSTEM_CONTROL_2                 0x04
#define ETA1459_SYSTEM_HIBERNATE                 0x05
#define ETA1459_INTERFACE_CONTROL                0x06
#define ETA1459_POWER_MGMT_1                     0x08
#define ETA1459_POWER_MGMT_2                     0x09
#define ETA1459_POWER_MGMT_3                     0x0A
#define ETA1459_POWER_MGMT_4                     0x0B
#define ETA1459_POWER_MGMT_5                     0x0C
#define ETA1459_POWER_MGMT_6                     0x0D
#define ETA1459_POWER_MGMT_7                     0x0E


#define ETA1459_MAX_REGISTER                     0xFF

/*
 * Field Definitions.
 */

/*
 * R0 (0x00) - Reset/ID
 */
#define ETA1459_SW_RESET_CHIP_ID_MASK            0xFFFF

/*
 * R1 (0x01) - ID
 */
#define ETA1459_CHIP_REV_MASK                    0x7000
#define ETA1459_CONF_STS_MASK                    0x0C00
#define ETA1459_CUST_ID_MASK                     0x00FF

/*
 * R2 (0x02) - Revision
 */
#define ETA1459_MASK_REV_MASK			0x00FF




/*
 * R5 (0x05) - System Hibernate
 */
#define ETA1459_HIBERNATE                        0x8000
#define ETA1459_WDOG_HIB_MODE                    0x0080
#define ETA1459_REG_HIB_STARTUP_SEQ              0x0040
#define ETA1459_REG_RESET_HIB_MODE               0x0020
#define ETA1459_RST_HIB_MODE                     0x0010
#define ETA1459_IRQ_HIB_MODE                     0x0008
#define ETA1459_MEMRST_HIB_MODE                  0x0004
#define ETA1459_PCCOMP_HIB_MODE                  0x0002
#define ETA1459_TEMPMON_HIB_MODE                 0x0001


/*
 * R8 (0x08) - Power mgmt (1)
 */
#define ETA1459_CODEC_ISEL_MASK                  0xC000
#define ETA1459_VBUFEN                           0x2000
#define ETA1459_OUTPUT_DRAIN_EN                  0x0400
#define ETA1459_MIC_DET_ENA                      0x0100
#define ETA1459_BIASEN                           0x0020
#define ETA1459_MICBEN                           0x0010
#define ETA1459_VMIDEN                           0x0004
#define ETA1459_VMID_MASK                        0x0003
#define ETA1459_VMID_SHIFT                            0


/*
 * R13 (0x0D) - Power mgmt (6)
 */
#define ETA1459_LS_ENA                           0x8000
#define ETA1459_LDO4_ENA                         0x0800
#define ETA1459_LDO3_ENA                         0x0400
#define ETA1459_LDO2_ENA                         0x0200
#define ETA1459_LDO1_ENA                         0x0100
#define ETA1459_DC6_ENA                          0x0020
#define ETA1459_DC5_ENA                          0x0010
#define ETA1459_DC4_ENA                          0x0008
#define ETA1459_DC3_ENA                          0x0004
#define ETA1459_DC2_ENA                          0x0002
#define ETA1459_DC1_ENA                          0x0001

/*
 * R14 (0x0E) - Power mgmt (7)
 */
#define ETA1459_CS2_ENA                          0x0002
#define ETA1459_CS1_ENA                          0x0001


/*
 * R225 (0xE1) - DCDC/LDO status
 */
#define ETA1459_LS_STS                           0x8000
#define ETA1459_LDO4_STS                         0x0800
#define ETA1459_LDO3_STS                         0x0400
#define ETA1459_LDO2_STS                         0x0200
#define ETA1459_LDO1_STS                         0x0100
#define ETA1459_DC6_STS                          0x0020
#define ETA1459_DC5_STS                          0x0010
#define ETA1459_DC4_STS                          0x0008
#define ETA1459_DC3_STS                          0x0004
#define ETA1459_DC2_STS                          0x0002
#define ETA1459_DC1_STS                          0x0001

/*
 * R226 (0xE2) - Charger status
 */
#define ETA1459_CHG_BATT_HOT_OVRDE		0x8000
#define ETA1459_CHG_BATT_COLD_OVRDE		0x4000

/*
 * R227 (0xE3) - Misc Overrides
 */
#define ETA1459_USB_LIMIT_OVRDE			0x0400

/*
 * R227 (0xE7) - Comparator Overrides
 */
#define ETA1459_USB_FB_OVRDE			0x8000
#define ETA1459_WALL_FB_OVRDE			0x4000
#define ETA1459_BATT_FB_OVRDE			0x2000


/* eta1459 chip revisions */
#define ETA1459_REV_E				0x4
#define ETA1459_REV_F				0x5
#define ETA1459_REV_G				0x6
#define ETA1459_REV_H				0x7

#define ETA1459_NUM_IRQ				63

#define ETA1459_NUM_IRQ_REGS 7



struct eta1459;

struct eta1459_hwmon {
	struct platform_device *pdev;
	struct device *classdev;
};

struct eta1459 {
	struct device *dev;

	/* device IO */
	union {
		struct i2c_client *i2c_client;
		struct spi_device *spi_device;
	};
	int (*read_dev)(struct eta1459 *eta1459, char reg, int size, void *dest);
	int (*write_dev)(struct eta1459 *eta1459, char reg, int size, void *src);
	//u16 *reg_cache;

	/* Client devices */
	//struct eta1459_codec codec;
	//struct eta1459_gpio gpio;
	//struct eta1459_hwmon hwmon;
	struct eta1459_pmic pmic;
    
	//struct eta1459_power power;
	//struct eta1459_rtc rtc;
	//struct eta1459_wdt wdt;
};

/**
 * Data to be supplied by the platform to initialise the ETA1459.
 *
 * @init: Function called during driver initialisation.  Should be
 *        used by the platform to configure GPIO functions and similar.
 * @irq_high: Set if ETA1459 IRQ is active high.
 * @irq_base: Base IRQ for genirq (not currently used).
 * @gpio_base: Base for gpiolib.
 */
struct eta1459_platform_data {
	int (*init)(struct eta1459 *eta1459);
	int irq_high;
	int irq_base;
	int gpio_base;
};


/*
 * ETA1459 device initialisation and exit.
 */
extern int eta1459_device_init(struct eta1459 *eta1459, int irq, struct eta1459_platform_data *pdata);

extern void eta1459_device_exit(struct eta1459 *eta1459);

/*
 * ETA1459 device IO
 */
int eta1459_clear_bits(struct eta1459 *eta1459, u16 reg, u16 mask);
int eta1459_set_bits(struct eta1459 *eta1459, u16 reg, u16 mask);
u16 eta1459_reg_read(struct eta1459 *eta1459, int reg);
int eta1459_reg_write(struct eta1459 *eta1459, int reg, u16 val);
int eta1459_reg_lock(struct eta1459 *eta1459);
int eta1459_reg_unlock(struct eta1459 *eta1459);
int eta1459_block_read(struct eta1459 *eta1459, int reg, int size, u16 *dest);
int eta1459_block_write(struct eta1459 *eta1459, int reg, int size, u16 *src);



#endif
