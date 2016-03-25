/*
 * Copyright 2009 Wolfson Microelectronics plc
 *
 * S3C64xx CPUfreq Support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "cpufreq: " fmt

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <linux/module.h>

#define DEBUG

static struct clk *ca9clk = NULL;
static struct regulator *vddca9 = NULL;

static unsigned long regulator_latency;

struct s3921_dvfs {
	unsigned int vddarm_min;
	unsigned int vddarm_max;
};


static struct s3921_dvfs s3921_dvfs_table[] = 
{
	[0] = { 1000000, 1150000 },
	[1] = { 1050000, 1150000 },
	[2] = { 1100000, 1150000 },
	[3] = { 1200000, 1350000 },
	[4] = { 1300000, 1350000 },
	[5] = { 1000000, 1150000 },
	[6] = { 1050000, 1150000 },
	[7] = { 1100000, 1150000 },

	[8] = { 1200000, 1350000 },
	[9] = { 1300000, 1350000 },
	[10] = { 1000000, 1150000 },
	[11] = { 1050000, 1150000 },
	[12] = { 1100000, 1150000 },	
	[13] = { 1200000, 1350000 },
	[14] = { 1300000, 1350000 },
	[15] = { 1300000, 1350000 },

	[16] = { 1200000, 1350000 },
	[17] = { 1300000, 1350000 },
	[18] = { 1000000, 1150000 },
	[19] = { 1050000, 1150000 },
	[20] = { 1100000, 1150000 },	
	[21] = { 1200000, 1350000 },
	[22] = { 1300000, 1350000 },
	[23] = { 1300000, 1350000 },
};

static struct cpufreq_frequency_table s3921_freq_table[] = 
{
	{ 0,  200000 },
	{ 1,  225000 },
	{ 2,  250000 },
	{ 3,  275000 },
	{ 4,  300000 },
	{ 5,  325000 },
	{ 6,  350000 },
	{ 7,  375000 },

	{ 8,   400000 },
	{ 9,   450000 },
	{ 10, 500000 },
	{ 11, 550000 },
	{ 12, 600000 },
	{ 13, 650000 },
	{ 14, 700000 },
	{ 15, 750000 },

	{ 16,  800000 },
	{ 17,  900000 },
	{ 18,  1000000},
	{ 19,  1100000},
	{ 20,  1200000},
	{ 21,  1300000},
	{ 22,  1400000},
	{ 23,  1500000},    
	{ 24, CPUFREQ_TABLE_END },
};


#if 0
static struct s3921_dvfs s3921_dvfs_table[] = {
	[0] = { 1000000, 1150000 },
	[1] = { 1050000, 1150000 },
	[2] = { 1100000, 1150000 },
	[3] = { 1200000, 1350000 },
	[4] = { 1300000, 1350000 },
	[5] = { 1000000, 1150000 },
	[6] = { 1050000, 1150000 },
	[7] = { 1100000, 1150000 },
	[8] = { 1100000, 1150000 },

};



static struct cpufreq_frequency_table s3921_freq_table[] = {
	{ 0,  200000 },
	{ 1,  225000 },
	{ 2,  250000 },


	{ 3,   400000 },
	{ 4,   450000 },
	{ 5,   500000 },

	{ 6,  800000 },
	{ 7,  900000 },
	{ 8,  1000000},
	{ 9, CPUFREQ_TABLE_END },
};

#endif


static int s3921_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	if (policy->cpu != 0)
		return -EINVAL;

	return cpufreq_frequency_table_verify(policy, s3921_freq_table);
}

static unsigned int s3921_cpufreq_get_speed(unsigned int cpu)
{
	if (cpu != 0)
		return 0;

	return clk_get_rate(ca9clk);
}

static int s3921_cpufreq_set_target(struct cpufreq_policy *policy,
				      unsigned int target_freq,
				      unsigned int relation)
{
	int ret;
	unsigned int i;
	struct cpufreq_freqs freqs;
	struct s3921_dvfs *dvfs;

	ret = cpufreq_frequency_table_target(policy, s3921_freq_table,
					     target_freq, relation, &i);
	if (ret != 0)
		return ret;

	freqs.cpu = 0;
	freqs.old = clk_get_rate(ca9clk);
	freqs.new = s3921_freq_table[i].frequency;
	freqs.flags = 0;
	dvfs = &s3921_dvfs_table[s3921_freq_table[i].index];

#if 0
	if (freqs.old == freqs.new)
		return 0;
#endif

	pr_debug("Transition %d-%dkHz\n", freqs.old, freqs.new);

	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

#ifdef CONFIG_REGULATOR
	if (vddca9 && freqs.new > freqs.old) {
		ret = regulator_set_voltage(vddca9,
					    dvfs->vddarm_min,
					    dvfs->vddarm_max);
		if (ret != 0) {
			pr_err("Failed to set VDDARM for %dkHz: %d\n",
			       freqs.new, ret);
			goto err;
		}
	}
#endif

	printk(KERN_ALERT"\n\n   clk_set_rate : %dkHz: %d \n\n  ", freqs.new);        
	ret = clk_set_rate(ca9clk, freqs.new);
	if (ret < 0) {
		pr_err("Failed to set rate %dkHz: %d\n",
		       freqs.new, ret);
		goto err;
	}

	//printk(KERN_ALERT"\n\n   clk_get_rate : %dkHz: %d \n\n  ", clk_get_rate(ca9clk));     

	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

#ifdef CONFIG_REGULATOR
	if (vddca9 && freqs.new < freqs.old) {
		ret = regulator_set_voltage(vddca9,
					    dvfs->vddarm_min,
					    dvfs->vddarm_max);
		if (ret != 0) {
			pr_err("Failed to set VDDARM for %dkHz: %d\n",
			       freqs.new, ret);
			goto err_clk;
		}
	}
#endif

	pr_debug("Set actual frequency %lukHz\n",
		 clk_get_rate(ca9clk));

	return 0;

err_clk:
    
	if (clk_set_rate(ca9clk, freqs.old) < 0)
		pr_err("Failed to restore original clock rate\n");
		
err:
	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

	return ret;
}

#ifdef CONFIG_REGULATOR
static void __init s3921_cpufreq_config_regulator(void)
{
	int count, v, i, found;
	struct cpufreq_frequency_table *freq;
	struct s3c64xx_dvfs *dvfs;

	count = regulator_count_voltages(vddca9);
	if (count < 0) {
		pr_err("Unable to check supported voltages\n");
	}

	freq = s3921_freq_table;
	while (count > 0 && freq->frequency != CPUFREQ_TABLE_END) {
		if (freq->frequency == CPUFREQ_ENTRY_INVALID)
			continue;

		dvfs = &s3921_dvfs_table[freq->index];
		found = 0;

		for (i = 0; i < count; i++) {
			v = regulator_list_voltage(vddca9, i);
			if (v >= dvfs->vddarm_min && v <= dvfs->vddarm_max)
				found = 1;
		}

		if (!found) {
			pr_debug("%dkHz unsupported by regulator\n",
				 freq->frequency);
			freq->frequency = CPUFREQ_ENTRY_INVALID;
		}

		freq++;
	}

	/* Guess based on having to do an I2C/SPI write; in future we
	 * will be able to query the regulator performance here. */
	regulator_latency = 1 * 1000 * 1000;
}
#endif

static int s3921_cpufreq_driver_init(struct cpufreq_policy *policy)
{
	int ret;
	struct cpufreq_frequency_table *freq;

	printk(KERN_EMERG"\n\n\n\n\n   s3921_cpufreq_driver_init :\n \n\n\n\n\n  ");

	if (policy->cpu != 0)
		return -EINVAL;

	if (s3921_freq_table == NULL) {
		pr_err("No frequency information for this CPU\n");
		return -ENODEV;
	}
#if 0
       struct device ca9_clk_device =
       {
            .init_name = "s3921-ca9-clock"
       } ;
#endif

	ca9clk = clk_get(NULL, "ca9_arm_clk");
	//printk(KERN_ALERT"\n\n\n\n\n   ca9clk = 0x%x :\n \n\n\n\n\n  ", ca9clk);
        
	if (IS_ERR(ca9clk)) {
		pr_err("Unable to obtain ARMCLK: %ld\n",
		       PTR_ERR(ca9clk));
		return PTR_ERR(ca9clk);
	}

#ifdef CONFIG_REGULATOR
	vddca9 = regulator_get(NULL, "vddarm");
	if (IS_ERR(vddca9)) {
		ret = PTR_ERR(vddca9);
		pr_err("Failed to obtain VDDARM: %d\n", ret);
		pr_err("Only frequency scaling available\n");
		vddca9 = NULL;
	} else {
		s3921_cpufreq_config_regulator();
	}
#endif

	freq = s3921_freq_table;
	while (freq->frequency != CPUFREQ_TABLE_END) {
		unsigned long r;

		/* Check for frequencies we can generate */
		r = clk_round_rate(ca9clk, freq->frequency);
        
		if (r != freq->frequency) {
			pr_debug("%dkHz unsupported by clock\n",
				 freq->frequency);
			freq->frequency = CPUFREQ_ENTRY_INVALID;
		}

		/* If we have no regulator then assume startup
		 * frequency is the maximum we can support. */
#if 0		 
		if (!vddca9 && freq->frequency > s3921_cpufreq_get_speed(0))
			freq->frequency = CPUFREQ_ENTRY_INVALID;
#endif
		freq++;
	}

	policy->cur = clk_get_rate(ca9clk);

	/* Datasheet says PLL stabalisation time (if we were to use
	 * the PLLs, which we don't currently) is ~300us worst case,
	 * but add some fudge.
	 */
	policy->cpuinfo.transition_latency = (500 * 1000) + regulator_latency;

	ret = cpufreq_frequency_table_cpuinfo(policy, s3921_freq_table);
	if (ret != 0) {
		pr_err("Failed to configure frequency table: %d\n",
		       ret);
		regulator_put(vddca9);
		clk_put(ca9clk);
	}

	return ret;
}

static struct cpufreq_driver s3921_cpufreq_driver = {
	.owner        = THIS_MODULE,
	.flags          = 0,
	.verify         = s3921_cpufreq_verify_speed,
	.target        = s3921_cpufreq_set_target,
	.get		   = s3921_cpufreq_get_speed,
	.init	           = s3921_cpufreq_driver_init,
	.name          = "s3921",
};

static int __init s3921_cpufreq_init(void)
{
        pr_debug("\n\n\n\n\ns3921_cpufreq_init\n");
        
        printk(KERN_ALERT"\n\n\n\n\n   s3921_cpufreq_init :\n \n\n\n\n\n  ");
        
	return cpufreq_register_driver(&s3921_cpufreq_driver);
}
//module_init(s3921_cpufreq_init);


subsys_initcall(s3921_cpufreq_init);
