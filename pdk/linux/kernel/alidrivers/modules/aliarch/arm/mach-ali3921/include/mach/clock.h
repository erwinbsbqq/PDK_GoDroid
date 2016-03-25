/* linux/arch/arm/mach-ali3921/include/mach/clock.h
 *
 * Copyright 2013 ALI Tech, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_PLAT_CLOCK_H
#define __ASM_PLAT_CLOCK_H __FILE__

#include <linux/spinlock.h>
#include <linux/clkdev.h>

struct clk;

#define MAX_DIV_FREQUENCY_NUM 16



/**
 * struct clk_ratio_reg - register definition for clock ratio control bits
 * @reg: pointer to the register in virtual memory.
 * @shift: the shift in bits to where the bitfield is.
 * @size: the size in bits of the bitfield.
 *
 * This specifies the size and position of the bits we are interested
 * in within the register specified by @reg.
 */
struct clk_ratio_reg 
{
	unsigned long 		reg;
	unsigned short		shift;
	unsigned short		size;
};

/**
 * struct clk_ops - standard clock operations
 * @set_rate: set the clock rate, see clk_set_rate().
 * @get_rate: get the clock rate, see clk_get_rate().
 * @round_rate: round a given clock rate, see clk_round_rate().
 * @set_parent: set the clock's parent, see clk_set_parent().
 *
 * Group the common clock implementations together so that we
 * don't have to keep setting the same fields again. We leave
 * enable in struct clk.
 *
 * Adding an extra layer of indirection into the process should
 * not be a problem as it is unlikely these operations are going
 * to need to be called quickly.
 */
struct clk_ops 
{
    int		      (*set_rate)(struct clk *c, unsigned long rate);
    unsigned long    (*get_rate)(struct clk *c);
    unsigned long    (*round_rate)(struct clk *c, unsigned long rate);
    int		      (*set_parent)(struct clk *c, struct clk *parent);
};

struct clk 
{
    struct list_head      list;
    struct module        *owner;
    struct clk           *parent;
    const char           *name;
    const char		*devname;

    unsigned char    uc_div_num;
    unsigned char   auc_ctrl_to_div_map[MAX_DIV_FREQUENCY_NUM];
    unsigned char   auc_div_to_ctrl_map[MAX_DIV_FREQUENCY_NUM];
    
    int		      id;
    int		      usage;
    unsigned long         rate;
    unsigned long         ctrlbit;
    struct clk_ratio_reg reg_div;
	
    struct clk_ops		*ops;
    int (*enable)(struct clk *, int enable);
    struct clk_lookup	lookup;

	
#if defined(CONFIG_PM_DEBUG) && defined(CONFIG_DEBUG_FS)
	struct dentry		*dent;	/* For visible tree hierarchy */
#endif
};

extern int clk_default_setrate(struct clk *clk, unsigned long rate);


extern int ali_s39XX_register_clock(struct clk *clk);
extern int ali_s39XX_register_clocks(struct clk **clk, int nr_clks);


#endif /* __ASM_PLAT_CLOCK_H */
