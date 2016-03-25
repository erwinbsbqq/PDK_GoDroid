/* linux/arch/arm/mach-ali3921/clock.c
 *
 * Copyright 2013 ALI Tech, Inc.
 *
 * ALI S39xx Core clock control support, S39xx clock interface implementation
 *
 * Based on the code  linux/arch/arm/mach-ali3921/s3921-clock.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#if defined(CONFIG_DEBUG_FS)
#include <linux/debugfs.h>
#endif


#include <mach/clock.h>




/* clock information */

static LIST_HEAD(clocks);

/* We originally used an mutex here, but some contexts (see resume)
 * are calling functions such as clk_set_parent() with IRQs disabled
 * causing an BUG to be triggered.
 */
DEFINE_SPINLOCK(clocks_lock);


/* base clocks */

int clk_default_setrate(struct clk *clk, unsigned long rate)
{
	clk->rate = rate;
	return 0;
}

/* enable and disable calls for use with the clk struct */

static int clk_null_enable(struct clk *clk, int enable)
{
	return 0;
}

/* enable and disable calls for use with the clk struct */

int clk_enable(struct clk *clk)
{
	unsigned long flags;

	if (IS_ERR(clk) || clk == NULL)
		return -EINVAL;

	clk_enable(clk->parent);

	spin_lock_irqsave(&clocks_lock, flags);

	if ((clk->usage++) == 0)
		(clk->enable)(clk, 1);

	spin_unlock_irqrestore(&clocks_lock, flags);
	return 0;
}

void clk_disable(struct clk *clk)
{
	unsigned long flags;

	if (IS_ERR(clk) || clk == NULL)
		return;

	spin_lock_irqsave(&clocks_lock, flags);

	if ((--clk->usage) == 0)
		(clk->enable)(clk, 0);

	spin_unlock_irqrestore(&clocks_lock, flags);
	clk_disable(clk->parent);
}


unsigned long clk_get_rate(struct clk *clk)
{         
	if (IS_ERR(clk))
		return 0;

	if (clk->rate != 0)
		return clk->rate;

	if (clk->ops != NULL && clk->ops->get_rate != NULL)
		return (clk->ops->get_rate)(clk);

	if (clk->parent != NULL)
		return clk_get_rate(clk->parent);

	return clk->rate;
}

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	if (!IS_ERR(clk) && clk->ops && clk->ops->round_rate)
		return (clk->ops->round_rate)(clk, rate);

	return rate;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	int ret;

	if (IS_ERR(clk))
		return -EINVAL;

	/* We do not default just do a clk->rate = rate as
	 * the clock may have been made this way by choice.
	 */

	WARN_ON(clk->ops == NULL);
	WARN_ON(clk->ops && clk->ops->set_rate == NULL);

	if (clk->ops == NULL || clk->ops->set_rate == NULL)
		return -EINVAL;

	//spin_lock(&clocks_lock);
	ret = (clk->ops->set_rate)(clk, rate);
	//spin_unlock(&clocks_lock);

	return ret;
}

struct clk *clk_get_parent(struct clk *clk)
{
	return clk->parent;
}

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	int ret = 0;

	if (IS_ERR(clk))
		return -EINVAL;

	spin_lock(&clocks_lock);

	if (clk->ops && clk->ops->set_parent)
		ret = (clk->ops->set_parent)(clk, parent);

	spin_unlock(&clocks_lock);

	return ret;
}

EXPORT_SYMBOL(clk_enable);
EXPORT_SYMBOL(clk_disable);
EXPORT_SYMBOL(clk_get_rate);
EXPORT_SYMBOL(clk_round_rate);
EXPORT_SYMBOL(clk_set_rate);
EXPORT_SYMBOL(clk_get_parent);
EXPORT_SYMBOL(clk_set_parent);



/* initialise the clock system */

/**
 * ali_s39XX_register_clock() - register a clock
 * @clk: The clock to register
 *
 * Add the specified clock to the list of clocks known by the system.
 */
int ali_s39XX_register_clock(struct clk *clk)
{
	if(!clk)
		return -1;
	if (clk->enable == NULL)
		clk->enable = clk_null_enable;

	/* fill up the clk_lookup structure and register it*/
	clk->lookup.dev_id = clk->devname;
	clk->lookup.con_id = clk->name;
	clk->lookup.clk = clk;

	//printk(KERN_ALERT"\n\n\n\n\n   ali_s39XX_register_clock, name: %s,  devname: %s :\n \n\n\n\n\n", clk->name, clk->devname);
	
#ifdef CONFIG_ARM_ALIS3921_CPUFREQ
	clkdev_add(&clk->lookup);
#endif

	return 0;
}

/**
 * ali_s39XX_register_clocks() - register an array of clock pointers
 * @clks: Pointer to an array of struct clk pointers
 * @nr_clks: The number of clocks in the @clks array.
 *
 * Call ali_s39XX_register_clock() for all the clock pointers contained
 * in the @clks list. Returns the number of failures.
 */
int ali_s39XX_register_clocks(struct clk **clks, int nr_clks)
{
	int fails = 0;

	for (; nr_clks > 0; nr_clks--, clks++) {
		if (ali_s39XX_register_clock(*clks) < 0) {
			struct clk *clk = *clks;
			printk(KERN_ERR "%s: failed to register %p: %s\n",
			       __func__, clk, clk->name);
			fails++;
		}
	}

	return fails;
}


