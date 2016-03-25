/*
 * Copyright 2013 ALI Tech, Inc.
 *
 * s3921 CPUfreq Support, CPUfreq low level driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "cpufreq: " fmt

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <linux/module.h>

#include <mach/ali-s3921.h>
#include <asm/io.h>
#include <linux/cpu.h>
#include <linux/delay.h>

//#include <bus/i2c/i2c.h>

#define DEBUG

static struct clk *ca9clk = NULL;

static struct clk *pl310clk = NULL;

static struct clk *ca9_peripheral_clk = NULL;

static struct clk *l2_tag_ram_clk = NULL;

static struct clk *l2_data_ram_clk = NULL;

static struct clk *l2_axi_master_clk = NULL;

static struct regulator *vddca9 = NULL;

static unsigned long regulator_latency =  0;

struct s3921_dvfs {
	unsigned int vddarm_min;
	unsigned int vddarm_max;
};

/* Define the S3921 cpu frequency index enum */
enum S3921_FREQUENCY_INDEX
{
    S3921_FREQUENCY_INDEX_BEGIN = 0,
    S3921_FREQUENCY_200M_INDEX = S3921_FREQUENCY_INDEX_BEGIN,
    S3921_FREQUENCY_225M_INDEX,
    S3921_FREQUENCY_250M_INDEX,
    S3921_FREQUENCY_275M_INDEX,
    S3921_FREQUENCY_300M_INDEX,
    S3921_FREQUENCY_325M_INDEX,
    S3921_FREQUENCY_350M_INDEX,
    S3921_FREQUENCY_375M_INDEX,
    
    S3921_FREQUENCY_400M_INDEX,
    S3921_FREQUENCY_450M_INDEX,
    S3921_FREQUENCY_500M_INDEX,
    S3921_FREQUENCY_550M_INDEX,
    S3921_FREQUENCY_600M_INDEX,
    S3921_FREQUENCY_650M_INDEX,
    S3921_FREQUENCY_700M_INDEX,
    S3921_FREQUENCY_750M_INDEX,
    
    S3921_FREQUENCY_800M_INDEX,
    S3921_FREQUENCY_900M_INDEX,
    S3921_FREQUENCY_1000M_INDEX,
    S3921_FREQUENCY_1100M_INDEX,
    S3921_FREQUENCY_1200M_INDEX,
    S3921_FREQUENCY_1300M_INDEX,
    S3921_FREQUENCY_1400M_INDEX,
    S3921_FREQUENCY_1500M_INDEX,    
    S3921_FREQUENCY_INDEX_END
};

/* Define the S3921 cpu frequency value enum */
enum S3921_FREQUENCY_VALUE
{
    S3921_FREQUENCY_VALUE_BEGIN = 200000,
    S3921_FREQUENCY_200M = S3921_FREQUENCY_VALUE_BEGIN,
    S3921_FREQUENCY_225M = 225000,
    S3921_FREQUENCY_250M = 250000,
    S3921_FREQUENCY_275M = 275000,
    S3921_FREQUENCY_300M = 300000,
    S3921_FREQUENCY_325M = 325000,
    S3921_FREQUENCY_350M = 350000,
    S3921_FREQUENCY_375M = 375000,
    
    S3921_FREQUENCY_400M = 400000,
    S3921_FREQUENCY_450M = 450000,
    S3921_FREQUENCY_500M = 500000,
    S3921_FREQUENCY_550M = 550000,
    S3921_FREQUENCY_600M = 600000,
    S3921_FREQUENCY_650M = 650000,
    S3921_FREQUENCY_700M = 700000,
    S3921_FREQUENCY_750M = 750000,
    
    S3921_FREQUENCY_800M = 800000,
    S3921_FREQUENCY_900M = 900000,
    S3921_FREQUENCY_1000M = 1000000,
    S3921_FREQUENCY_1100M = 1100000,
    S3921_FREQUENCY_1200M = 1200000,
    S3921_FREQUENCY_1300M = 1300000,
    S3921_FREQUENCY_1400M = 1400000,
    S3921_FREQUENCY_1500M = 1500000,    
    S3921_FREQUENCY_VALUE_END = S3921_FREQUENCY_1500M
};


/* Define the cpu packaging type enum */
enum S3921_CPU_PACKAGING_TYPE
{
    S3921_CPU_PACKAGING_TYPE_BEGIN,
    S3921_CPU_PACKAGING_TYPE_QFP = S3921_CPU_PACKAGING_TYPE_BEGIN,
    S3921_CPU_PACKAGING_TYPE_BGA,
    S3921_CPU_PACKAGING_TYPE_END
};


/* Define the cpu regulator online/offline status enum */
enum S3921_CPU_REGULATOR_STATUS
{
    S3921_CPU_REGULATOR_STATUS_BEGIN,
    S3921_CPU_REGULATOR_ONLINE = S3921_CPU_REGULATOR_STATUS_BEGIN,
    S3921_CPU_REGULATOR_OFFLINE,
    S3921_CPU_REGULATOR_STATUS_END
};

/* Define the cpu regulator dvfs table type enum */
enum S3921_CPU_REGULATOR_TABLE
{
    S3921_CPU_REGULATOR_TABLE_BEGIN,
    S3921_CPU_REGULATOR_TABLE_W = S3921_CPU_REGULATOR_TABLE_BEGIN,
    S3921_CPU_REGULATOR_TABLE_T,
    S3921_CPU_REGULATOR_TABLE_B,
    S3921_CPU_REGULATOR_TABLE_END
};

static struct s3921_dvfs s3921_dvfs_bga_table_w[] = 
{
	[0] = { 900000,  910000 },            /*200M */
	[1] = { 900000,  910000 },            /*225M */
	[2] = { 900000,  910000 },            /*250M */
	[3] = { 900000,  910000 },            /*275M */
	[4] = { 920000 , 930000 },            /*300M */
	[5] = { 920000 , 930000 },            /*325M */
	[6] = { 920000 , 930000 },            /*350M */
	[7] = { 920000 , 930000 },            /*375M */

	[8] =  { 902000, 902000 },            /*400M */
	[9] =  { 943000, 943000 },            /*450M */
	[10] = { 943000, 943000 },            /*500M */
	[11] = { 994000, 994000 },            /*550M */
	[12] = { 994000, 994000 },	      /*600M */
	[13] = { 1046000, 1046000 },          /*650M */
	[14] = { 1046000, 1046000 },          /*700M */
	[15] = { 1128000, 1128000 },          /*750M */

	[16] = { 1128000, 1128000 },          /*800M */
	[17] = { 1220000, 1220000 },          /*900M */
	[18] = { 1292000, 1292000 },          /*1000M */
	[19] = { 1353000, 1353000 },          /*1100M */
	[20] = { 1425000, 1425000 },          /*1200M */
#if 1	
	[21] = { 1530000, 1600000 },          /*1300M */
	[22] = { 1550000, 1600000 },	      /*1400M */
	[23] = { 1580000, 1600000 },          /*1500M */
#endif
};

static struct s3921_dvfs s3921_dvfs_bga_table_t[] = 
{
	[0] = { 900000,  910000 },            /*200M */
	[1] = { 900000,  910000 },            /*225M */
	[2] = { 900000,  910000 },            /*250M */
	[3] = { 900000,  910000 },            /*275M */
	[4] = { 920000 , 930000 },            /*300M */
	[5] = { 920000 , 930000 },            /*325M */
	[6] = { 920000 , 930000 },            /*350M */
	[7] = { 920000 , 930000 },            /*375M */

	[8] =  { 974000, 974000 },            /*400M */
	[9] =  { 974000, 974000 },            /*450M */
	[10] = { 994000, 994000 },            /*500M */
	[11] = { 994000, 994000 },            /*550M */
	[12] = { 994000, 994000 },	          /*600M */
	[13] = { 994000, 994000 },            /*650M */
	[14] = { 994000, 994000 },            /*700M */
	[15] = { 994000, 994000 },            /*750M */

	[16] = { 1056000, 1056000 },          /*800M */
	[17] = { 1117000, 1117000 },          /*900M */
	[18] = { 1199000, 1199000 },          /*1000M */
	[19] = { 1271000, 1271000 },          /*1100M */
	[20] = { 1343000, 1343000 },          /*1200M */
#if 1	
	[21] = { 1460000, 1600000 },          /*1300M */
	[22] = { 1500000, 1600000 },	      /*1400M */
	[23] = { 1550000, 1600000 },          /*1500M */
#endif
};

static struct s3921_dvfs s3921_dvfs_bga_table_b[] = 
{
	[0] = { 900000,  910000 },            /*200M */
	[1] = { 900000,  910000 },            /*225M */
	[2] = { 900000,  910000 },            /*250M */
	[3] = { 900000,  910000 },            /*275M */
	[4] = { 920000 , 930000 },            /*300M */
	[5] = { 920000 , 930000 },            /*325M */
	[6] = { 920000 , 930000 },            /*350M */
	[7] = { 920000 , 930000 },            /*375M */

	[8] =  { 871000, 871000 },            /*400M */
	[9] =  { 912000, 912000 },            /*450M */
	[10] = { 912000, 912000 },            /*500M */
	[11] = { 953000, 953000 },            /*550M */
	[12] = { 953000, 953000 },	          /*600M */
	[13] = { 1015000, 1015000 },            /*650M */
	[14] = { 1015000, 1015000 },            /*700M */
	[15] = { 1066000, 1066000 },            /*750M */

	[16] = { 1066000, 1066000 },            /*800M */
	[17] = { 1128000, 1128000 },          /*900M */
	[18] = { 1199000, 1199000 },          /*1000M */
	[19] = { 1261000, 1261000 },          /*1100M */
	[20] = { 1322000, 1322000 },          /*1200M */
#if 1	
	[21] = { 1400000, 1600000 },          /*1300M */
	[22] = { 1500000, 1600000 },	      /*1400M */
	[23] = { 1550000, 1600000 },          /*1500M */
#endif
};



static struct s3921_dvfs s3921_dvfs_qfp_table_w[] = 
{
	[0] = { 900000,  910000 },            /*200M */
	[1] = { 900000,  910000 },            /*225M */
	[2] = { 900000,  910000 },            /*250M */
	[3] = { 900000,  910000 },            /*275M */
	[4] = { 920000 , 930000 },            /*300M */
	[5] = { 920000 , 930000 },            /*325M */
	[6] = { 920000 , 930000 },            /*350M */
	[7] = { 920000 , 930000 },            /*375M */

	[8] =  { 974000, 974000 },            /*400M */
	[9] =  { 974000, 974000 },            /*450M */
	[10] = { 994000, 994000 },            /*500M */
	[11] = { 994000, 994000 },            /*550M */
	[12] = { 1025000, 1025000 },	      /*600M */
	[13] = { 1025000, 1025000 },          /*650M */
	[14] = { 1087000, 1087000 },          /*700M */
	[15] = { 1087000, 1087000 },          /*750M */

	[16] = { 1138000, 1138000 },          /*800M */
	[17] = { 1220000, 1220000 },          /*900M */
	[18] = { 1292000, 1292000 },          /*1000M */
	[19] = { 1363000, 1363000 },          /*1100M */
	[20] = { 1456000, 1456000 },          /*1200M */
#if 1	
	[21] = { 1530000, 1600000 },          /*1300M */
	[22] = { 1550000, 1600000 },	      /*1400M */
	[23] = { 1580000, 1600000 },          /*1500M */
#endif
};

static struct s3921_dvfs s3921_dvfs_qfp_table_t[] = 
{
	[0] = { 900000,  910000 },            /*200M */
	[1] = { 900000,  910000 },            /*225M */
	[2] = { 900000,  910000 },            /*250M */
	[3] = { 900000,  910000 },            /*275M */
	[4] = { 920000 , 930000 },            /*300M */
	[5] = { 920000 , 930000 },            /*325M */
	[6] = { 920000 , 930000 },            /*350M */
	[7] = { 920000 , 930000 },            /*375M */

	[8] =  { 974000, 974000 },            /*400M */
	[9] =  { 974000, 974000 },            /*450M */
	[10] = { 994000, 994000 },            /*500M */
	[11] = { 994000, 994000 },            /*550M */
	[12] = { 994000, 994000 },	          /*600M */
	[13] = { 994000, 994000 },            /*650M */
	[14] = { 994000, 994000 },            /*700M */
	[15] = { 994000, 994000 },            /*750M */

	[16] = { 1056000, 1056000 },          /*800M */
	[17] = { 1117000, 1117000 },          /*900M */
	[18] = { 1199000, 1199000 },          /*1000M */
	[19] = { 1271000, 1271000 },          /*1100M */
	[20] = { 1343000, 1343000 },          /*1200M */
#if 1	
	[21] = { 1460000, 1600000 },          /*1300M */
	[22] = { 1500000, 1600000 },	      /*1400M */
	[23] = { 1550000, 1600000 },          /*1500M */
#endif
};

static struct s3921_dvfs s3921_dvfs_qfp_table_b[] = 
{
	[0] = { 900000,  910000 },            /*200M */
	[1] = { 900000,  910000 },            /*225M */
	[2] = { 900000,  910000 },            /*250M */
	[3] = { 900000,  910000 },            /*275M */
	[4] = { 920000 , 930000 },            /*300M */
	[5] = { 920000 , 930000 },            /*325M */
	[6] = { 920000 , 930000 },            /*350M */
	[7] = { 920000 , 930000 },            /*375M */

	[8] =  { 974000, 974000 },            /*400M */
	[9] =  { 974000, 974000 },            /*450M */
	[10] = { 994000, 994000 },            /*500M */
	[11] = { 994000, 994000 },            /*550M */
	[12] = { 994000, 994000 },	          /*600M */
	[13] = { 994000, 994000 },            /*650M */
	[14] = { 994000, 994000 },            /*700M */
	[15] = { 994000, 994000 },            /*750M */

	[16] = { 994000, 994000 },            /*800M */
	[17] = { 1025000, 1025000 },          /*900M */
	[18] = { 1056000, 1056000 },          /*1000M */
	[19] = { 1138000, 1138000 },          /*1100M */
	[20] = { 1199000, 1199000 },          /*1200M */
#if 1	
	[21] = { 1400000, 1600000 },          /*1300M */
	[22] = { 1500000, 1600000 },	      /*1400M */
	[23] = { 1550000, 1600000 },          /*1500M */
#endif
};


static struct s3921_dvfs *s3921_dvfs_table =  s3921_dvfs_qfp_table_w;


#if 0
static struct s3921_dvfs s3921_dvfs_table[] = 
{
	[0] = { 900000,  910000 },            /*200M */
	[1] = { 900000,  910000 },            /*225M */
	[2] = { 900000,  910000 },            /*250M */
	[3] = { 900000,  910000 },            /*275M */
	[4] = { 920000 , 930000 },            /*300M */
	[5] = { 920000 , 930000 },            /*325M */
	[6] = { 920000 , 930000 },            /*350M */
	[7] = { 920000 , 930000 },            /*375M */

	[8] =  { 974000, 974000 },             /*400M */
	[9] =  { 974000, 974000 },             /*450M */
	[10] = { 994000, 994000 },            /*500M */
	[11] = { 994000, 994000 },            /*550M */
	[12] = { 1025000, 1025000 },	      /*600M */
	[13] = { 1025000, 1025000 },          /*650M */
	[14] = { 1087000, 1087000 },          /*700M */
	[15] = { 1087000, 1087000 },          /*750M */

	[16] = { 1138000, 1138000 },          /*800M */
	[17] = { 1220000, 1220000 },          /*900M */
	[18] = { 1292000, 1292000 },          /*1000M */
	[19] = { 1363000, 1363000 },          /*1100M */
	[20] = { 1456000, 1456000 },          /*1200M */
#if 1	
	[21] = { 1530000, 1600000 },          /*1300M */
	[22] = { 1550000, 1600000 },	      /*1400M */
	[23] = { 1580000, 1600000 },          /*1500M */
#endif
};
#endif


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
#if 0		
	{ 21, CPUFREQ_TABLE_END },
#else	
	{ 21,  1300000},	
	{ 22,  1400000},	
	{ 23,  1500000},    
	{ 24, CPUFREQ_TABLE_END },
#endif
};

/* Define the PL310 (L2 controller) clock ratio */
static struct cpufreq_frequency_table s3921_pl310_ratio_table[] = 
{
	{ 0,  1 },              /*200M */
	{ 1,  1 },              /*225M */
	{ 2,  1 },              /*250M */
	{ 3,  1 },              /*275M */
	{ 4,  1 },              /*300M */
	{ 5,  1 },              /*325M */
	{ 6,  1 },              /*350M */
	{ 7,  1 },              /*375M */

	{ 8,   1 },             /*400M */
	{ 9,   1 },             /*450M */
	{ 10, 1 },              /*500M */
	{ 11, 1 },              /*550M */
	{ 12, 1 },              /*600M */
	{ 13, 1 },              /*650M */
	{ 14, 1 },              /*700M */
	{ 15, 1 },              /*750M */
	{ 16, 1 },             /*800M */
	{ 17, 1 },             /*900M */
	{ 18, 1 },              /*1000M */
	{ 19, 1 },              /*1100M */
	{ 20, 2 },              /*1200M */
#if 0	
	{ 21, CPUFREQ_TABLE_END },
#else	
	{ 21,  2},           /*1300M */
	{ 22,  2},           /*1400M */
	{ 23,  2},           /*1500M */
	{ 24, CPUFREQ_TABLE_END },
#endif
};

/* Define the L2 TAG RAM clock ratio */
static struct cpufreq_frequency_table s3921_l2_tag_ratio_table[] = 
{
	{ 0,  2 },              /*200M */
	{ 1,  2 },              /*225M */
	{ 2,  2 },              /*250M */
	{ 3,  2 },              /*275M */
	{ 4,  2 },              /*300M */
	{ 5,  2 },              /*325M */
	{ 6,  2 },              /*350M */
	{ 7,  2 },              /*375M */

	{ 8,   2 },             /*400M */
	{ 9,   2 },             /*450M */
	{ 10, 2 },              /*500M */
	{ 11, 2 },              /*550M */
	{ 12, 2 },              /*600M */
	{ 13, 2 },              /*650M */
	{ 14, 3 },              /*700M */
	{ 15, 3 },              /*750M */
	{ 16, 3 },             /*800M */
	{ 17, 3 },             /*900M */
	{ 18, 3 },              /*1000M */
	{ 19, 3 },              /*1100M */
	{ 20, 2 },              /*1200M */
#if 0		
	{ 21, CPUFREQ_TABLE_END },
#else	
	{ 21,  2},           /*1300M */
	{ 22,  3},           /*1400M */
	{ 23,  3},           /*1500M */
	{ 24, CPUFREQ_TABLE_END },
#endif
};

/* Define the L2 DATA RAM clock ratio */
static struct cpufreq_frequency_table s3921_l2_data_ratio_table[] = 
{
	{ 0,  2 },              /*200M */
	{ 1,  2 },              /*225M */
	{ 2,  2 },              /*250M */
	{ 3,  2 },              /*275M */
	{ 4,  2 },              /*300M */
	{ 5,  2 },              /*325M */
	{ 6,  2 },              /*350M */
	{ 7,  2 },              /*375M */

	{ 8,   2 },             /*400M */
	{ 9,   2 },             /*450M */
	{ 10, 2 },              /*500M */
	{ 11, 2 },              /*550M */
	{ 12, 2 },              /*600M */
	{ 13, 2 },              /*650M */
	{ 14, 3 },              /*700M */
	{ 15, 3 },              /*750M */
	{ 16, 3 },             /*800M */
	{ 17, 3 },             /*900M */
	{ 18, 3 },              /*1000M */
	{ 19, 3 },              /*1100M */
	{ 20, 2 },              /*1200M */
#if 0		
	{ 21, CPUFREQ_TABLE_END },
#else	
	{ 21,  2},           /*1300M */
	{ 22,  3},           /*1400M */
	{ 23,  3},           /*1500M */
	{ 24, CPUFREQ_TABLE_END },
#endif
};

/* Define the L2 AXI Master 0/1 clock ratio */
static struct cpufreq_frequency_table s3921_l2_axi_ratio_table[] = 
{
	{ 0,  2 },              /*200M */
	{ 1,  2 },              /*225M */
	{ 2,  2 },              /*250M */
	{ 3,  2 },              /*275M */
	{ 4,  2 },              /*300M */
	{ 5,  2 },              /*325M */
	{ 6,  2 },              /*350M */
	{ 7,  2 },              /*375M */

	{ 8,   2 },             /*400M */
	{ 9,   2 },             /*450M */
	{ 10, 2 },              /*500M */
	{ 11, 2 },              /*550M */
	{ 12, 3 },              /*600M */
	{ 13, 3 },              /*650M */
	{ 14, 3 },              /*700M */
	{ 15, 3 },              /*750M */
	{ 16, 3 },             /*800M */
	{ 17, 4 },             /*900M */
	{ 18, 4 },              /*1000M */
	{ 19, 4 },              /*1100M */
	{ 20, 3 },              /*1200M */
#if 0		
	{ 21, CPUFREQ_TABLE_END },
#else	
	{ 21,  3},           /*1300M */
	{ 22,  3},           /*1400M */
	{ 23,  3},           /*1500M */
	{ 24, CPUFREQ_TABLE_END },
#endif
};

static struct kobject *cpuinfo_local_kobject = NULL;

static char bga_packaging_name[10] = "BGA";
static char qfp_packaging_name[10] = "QFP";

static char regulator_online_name[10] = "ONLINE";
static char regulator_offline_name[10] = "OFFLINE";

static char regulator_table_w_name[10]   =  "W";
static char regulator_table_t_name[10]   =  "T";
static char regulator_table_b_name[10]   =  "B";

static unsigned int	ui_cpu_regulator_latency = 350;

static unsigned int	ui_cpu_regulator_table_type = S3921_CPU_REGULATOR_TABLE_W;

static unsigned int	ui_cpu_regulator_status = S3921_CPU_REGULATOR_ONLINE;

static unsigned int ui_cpu_packaging_type = S3921_CPU_PACKAGING_TYPE_BGA;

unsigned int g_ali_cpufreq_debug_ctrl = 1;

static unsigned int	ui_cpu_pm_threshold = 24;


static unsigned int	ui_cpu_per_cell_delay_value = 0;


static ssize_t show_cpu_packaging(struct kobject *kobj, struct attribute *attr, char *buf)
{
    if (S3921_CPU_PACKAGING_TYPE_QFP == ui_cpu_packaging_type)
    {
        /* The Chip Package type is QFP */
        //printk(KERN_ALERT"\n ***************QFP  package********************* \n");
        return sprintf(buf, "%s\n", qfp_packaging_name);
    }
    else
    {
         /* The Chip Package type is BGA */    
        //printk(KERN_ALERT"\n ***************BGA  package********************* \n");
        return sprintf(buf, "%s\n", bga_packaging_name);
    }
}


static ssize_t show_cpu_regulator_status(struct kobject *kobj, struct attribute *attr, char *buf)
{
    if (S3921_CPU_REGULATOR_ONLINE == ui_cpu_regulator_status)
    {
        /* The  regulator status is online */
        //printk(KERN_ALERT"\n ***************REGULATOR  ONLINE********************* \n");
       return sprintf(buf, "%s\n", regulator_online_name);
     }
     else
    {
        /* The  regulator status is offline */
        //printk(KERN_ALERT"\n ***************REGULATOR  OFFLINE********************* \n");
        return sprintf(buf, "%s\n", regulator_offline_name);
    }
}

static ssize_t show_cpu_regulator_latency(struct kobject *kobj, struct attribute *attr, char *buf)
{
     return sprintf(buf, "%u\n", ui_cpu_regulator_latency);
}


static ssize_t store_cpu_regulator_latency(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	ui_cpu_regulator_latency = input;
	return count;
}


static ssize_t show_cpu_regulator_table(struct kobject *kobj, struct attribute *attr, char *buf)
{
     if (S3921_CPU_REGULATOR_TABLE_W == ui_cpu_regulator_table_type)
    {
        /* The  regulator table type is "W" */
        //printk(KERN_ALERT"\n ***************REGULATOR  TABLE W********************* \n");
        return sprintf(buf, "%s\n", regulator_table_w_name);
     }
     else if (S3921_CPU_REGULATOR_TABLE_T == ui_cpu_regulator_table_type)
    {
        /* The  regulator table type is "T" */
        //printk(KERN_ALERT"\n ***************REGULATOR  TABLE T********************* \n");
        return sprintf(buf, "%s\n", regulator_table_t_name);
    }
    else
    {
        /* The  regulator table type is "B" */
         //printk(KERN_ALERT"\n ***************REGULATOR  TABLE B********************* \n");
        return sprintf(buf, "%s\n", regulator_table_b_name);
    }
     
}

static ssize_t store_cpu_regulator_table(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
    unsigned int ret = -EINVAL;
    char	str_regulator_table[16];
    ret = sscanf(buf, "%15s", str_regulator_table);
    if (ret != 1)
    {
        return -EINVAL;
    }

    if (0 == strnicmp(str_regulator_table, "W", 16))
    {
        ui_cpu_regulator_table_type = S3921_CPU_REGULATOR_TABLE_W;
    }
    else if (0 == strnicmp(str_regulator_table, "T", 16))
    {
        ui_cpu_regulator_table_type = S3921_CPU_REGULATOR_TABLE_T;
    }
    else if (0 == strnicmp(str_regulator_table, "B", 16))
    {
        ui_cpu_regulator_table_type = S3921_CPU_REGULATOR_TABLE_B;                
    }    
    else 
    {
        ret = -EINVAL;
    }

    if (ret)
        return ret;
    else
        return count;

}


static ssize_t show_cpu_cpufreq_debug_ctrl(struct kobject *kobj, struct attribute *attr, char *buf)
{
     return sprintf(buf, "%u\n", g_ali_cpufreq_debug_ctrl);
}


static ssize_t store_cpu_cpufreq_debug_ctrl(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
	{
        return -EINVAL;
	}
	g_ali_cpufreq_debug_ctrl = input;
	return count;
}

static ssize_t show_cpu_pm_threshold(struct kobject *kobj, struct attribute *attr, char *buf)
{
     return sprintf(buf, "%u\n", ui_cpu_pm_threshold);
}


static ssize_t store_cpu_pm_threshold(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
	{
	    return -EINVAL;
	}
	ui_cpu_pm_threshold = input;
	return count;
}


define_one_global_ro(cpu_packaging);

define_one_global_ro(cpu_regulator_status);

define_one_global_rw(cpu_regulator_table);

define_one_global_rw(cpu_regulator_latency);

define_one_global_rw(cpu_cpufreq_debug_ctrl);

define_one_global_rw(cpu_pm_threshold);

static int s3921_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
    if (policy->cpu != 0)
    {
        return -EINVAL;
    }

    //printk(KERN_ALERT"request for verification of policy (%u - %u kHz) for cpu %u\n", policy->min, policy->max, policy->cpu);

    if (policy->max <  s3921_freq_table[S3921_FREQUENCY_1300M_INDEX].frequency )
    {
        s3921_freq_table[S3921_FREQUENCY_325M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
        s3921_freq_table[S3921_FREQUENCY_350M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
        s3921_freq_table[S3921_FREQUENCY_375M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
            
        s3921_freq_table[S3921_FREQUENCY_650M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
        s3921_freq_table[S3921_FREQUENCY_700M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
        s3921_freq_table[S3921_FREQUENCY_750M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
    }
    else if (policy->max <  s3921_freq_table[S3921_FREQUENCY_1400M_INDEX].frequency )
    {
        s3921_freq_table[S3921_FREQUENCY_325M_INDEX].frequency = S3921_FREQUENCY_325M;
        s3921_freq_table[S3921_FREQUENCY_350M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
        s3921_freq_table[S3921_FREQUENCY_375M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
            
        s3921_freq_table[S3921_FREQUENCY_650M_INDEX].frequency = S3921_FREQUENCY_650M;
        s3921_freq_table[S3921_FREQUENCY_700M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
        s3921_freq_table[S3921_FREQUENCY_750M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
    } 
    else if (policy->max <  s3921_freq_table[S3921_FREQUENCY_1500M_INDEX].frequency )
    {
        s3921_freq_table[S3921_FREQUENCY_325M_INDEX].frequency = S3921_FREQUENCY_325M;
        s3921_freq_table[S3921_FREQUENCY_350M_INDEX].frequency = S3921_FREQUENCY_350M;
        s3921_freq_table[S3921_FREQUENCY_375M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;
            
        s3921_freq_table[S3921_FREQUENCY_650M_INDEX].frequency = S3921_FREQUENCY_650M;
        s3921_freq_table[S3921_FREQUENCY_700M_INDEX].frequency = S3921_FREQUENCY_700M;
        s3921_freq_table[S3921_FREQUENCY_750M_INDEX].frequency = CPUFREQ_ENTRY_INVALID;        
    }
    else
    {
        s3921_freq_table[S3921_FREQUENCY_325M_INDEX].frequency = S3921_FREQUENCY_325M;
        s3921_freq_table[S3921_FREQUENCY_350M_INDEX].frequency = S3921_FREQUENCY_350M;
        s3921_freq_table[S3921_FREQUENCY_375M_INDEX].frequency = S3921_FREQUENCY_375M;
            
        s3921_freq_table[S3921_FREQUENCY_650M_INDEX].frequency = S3921_FREQUENCY_650M;
        s3921_freq_table[S3921_FREQUENCY_700M_INDEX].frequency = S3921_FREQUENCY_700M;
        s3921_freq_table[S3921_FREQUENCY_750M_INDEX].frequency = S3921_FREQUENCY_750M;
    }

                
	return cpufreq_frequency_table_verify(policy, s3921_freq_table);
}

static unsigned int s3921_cpufreq_get_speed(unsigned int cpu)
{
    if (cpu != 0)
    {
        return 0;
    }

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

    struct cpufreq_freqs pl310_ratio;
    struct cpufreq_freqs l2_tag_ratio;
    struct cpufreq_freqs l2_data_ratio;
    struct cpufreq_freqs l2_axi_ratio;

    //unsigned long ul_clk_rate = 0;
    
    //unsigned int i32 = 0;
    //volatile unsigned int cnt = 0;

    /* Prevent CPU1 form seting the CPU frequency, only CPU0 can seting */
    /* Only one PLL Clock system in the 3921 SOC */
    if (1 == policy->cpu)
    {
        return 0;
    }

    /* Get the matching frequency index in the  frequency table based on the  target frequency */
    ret = cpufreq_frequency_table_target(policy, s3921_freq_table,
					     target_freq, relation, &i);
    if (ret != 0)
    {
        return ret;
    }

    freqs.cpu = policy->cpu;
    freqs.old = clk_get_rate(ca9clk);
    freqs.new = s3921_freq_table[i].frequency;
    freqs.flags = 0;

    
    /* Select the cpu regulator table type table based on the  pm threshold  */
    if (ui_cpu_pm_threshold != 0)
    {
        if (ui_cpu_per_cell_delay_value < ui_cpu_pm_threshold )
        {
             /* Select the best  regulator table type table */
            ui_cpu_regulator_table_type = S3921_CPU_REGULATOR_TABLE_B;
        }	
        else
        {
            ui_cpu_regulator_table_type = S3921_CPU_REGULATOR_TABLE_W;
        }
    }

    if (S3921_CPU_PACKAGING_TYPE_BGA == ui_cpu_packaging_type)
    {                   
        /* The Chip Package type is BGA */    
        if (S3921_CPU_REGULATOR_TABLE_W == ui_cpu_regulator_table_type)
        {
            /* The  regulator table type is "W" */
            s3921_dvfs_table = s3921_dvfs_bga_table_w;
        }
        else if (S3921_CPU_REGULATOR_TABLE_T == ui_cpu_regulator_table_type)
        {
            /* The  regulator table type is "T" */
            s3921_dvfs_table = s3921_dvfs_bga_table_t;
        }
        else
        {
            /* The  regulator table type is "B" */
            s3921_dvfs_table = s3921_dvfs_bga_table_b;        
        }  
    }
    else
    {
        /* The Chip Package type is QFP */ 

        if (S3921_CPU_REGULATOR_TABLE_W == ui_cpu_regulator_table_type)
        {
            /* The  regulator table type is "W" */
            s3921_dvfs_table = s3921_dvfs_qfp_table_w;
        }
        else if (S3921_CPU_REGULATOR_TABLE_T == ui_cpu_regulator_table_type)
        {
            /* The  regulator table type is "T" */
            s3921_dvfs_table = s3921_dvfs_qfp_table_t;
        }
        else
        {
            /* The  regulator table type is "B" */
            s3921_dvfs_table = s3921_dvfs_qfp_table_b;        
        }
       
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
    dvfs = &s3921_dvfs_table[s3921_freq_table[i].driver_data];
#else
    dvfs = &s3921_dvfs_table[s3921_freq_table[i].index];
#endif        
    pl310_ratio.cpu = 0;
    pl310_ratio.old = (clk_get_rate(pl310clk) >> 24) & 0x000000FF;
    pl310_ratio.new = s3921_pl310_ratio_table[i].frequency;
    pl310_ratio.flags = 0;

    l2_tag_ratio.cpu = 0;
    l2_tag_ratio.old = (clk_get_rate(l2_tag_ram_clk) >> 24) & 0x000000FF;
    l2_tag_ratio.new = s3921_l2_tag_ratio_table[i].frequency;
    l2_tag_ratio.flags = 0;

    l2_data_ratio.cpu = 0;
    l2_data_ratio.old = (clk_get_rate(l2_data_ram_clk) >> 24) & 0x000000FF;
    l2_data_ratio.new = s3921_l2_data_ratio_table[i].frequency;
    l2_data_ratio.flags = 0;

    l2_axi_ratio.cpu = 0;
    l2_axi_ratio.old = (clk_get_rate(l2_axi_master_clk) >> 24) & 0x000000FF;
    l2_axi_ratio.new = s3921_l2_axi_ratio_table[i].frequency;
    l2_axi_ratio.flags = 0;

#if 0
    printk(KERN_ALERT"\n\n**freqs.new = %lu, freqs.old = %lu\n\n", freqs.new, freqs.old);

    printk(KERN_ALERT"\n\n**PL310 clock ratio new= %lu,  L2 TAG RAM clock ratio new= %lu, L2 DATA RAM clock ratio new= %lu, L2 AXI Master clock ratio new= %lu \n\n" 
                                               , pl310_ratio.new, l2_tag_ratio.new, l2_data_ratio.new,  l2_axi_ratio.new );
    printk(KERN_ALERT"\n\n**PL310 clock ratio old= %lu,  L2 TAG RAM clock ratio old= %lu, L2 DATA RAM clock ratio old= %lu, L2 AXI Master clock ratio old= %lu \n\n" 
                                               ,pl310_ratio.old, l2_tag_ratio.old, l2_data_ratio.old,  l2_axi_ratio.old);  
#endif

#if 1
    if ((freqs.old == freqs.new) && (pl310_ratio.old == pl310_ratio.new)  && (l2_tag_ratio.old == l2_tag_ratio.new) 
          && (l2_data_ratio.old == l2_data_ratio.new) && (l2_axi_ratio.old == l2_axi_ratio.new)) 
    {
#if 0    
        printk(KERN_ALERT"\n\n *(freqs.old == freqs.new)* \n\n");
#endif

        /* The new  frequency & ratio are the same as the  current frequency & ratio, no need to do any setting */
        return 0;
    }
#endif

    pr_debug("Transition %d-%dkHz\n", freqs.old, freqs.new);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
    cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);

     /* Sync the new frequency to both CPU0 & CPU1 */
#if 1
    if (0 == freqs.cpu)
    {
        freqs.cpu = 1;
        policy->cpu = 1;        
    }
    else
    {
        freqs.cpu = 0;
        policy->cpu = 0;                
    }
    cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);
#endif
    
#else
    cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

     /* Sync the new frequency to both CPU0 & CPU1 */
#if 1
     if (0 == freqs.cpu)
    {
        freqs.cpu = 1;    
    }
    else
    {
        freqs.cpu = 0;
    }
    
    cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
#endif    
    
#endif

#ifdef CONFIG_REGULATOR
    if (vddca9 && freqs.new > freqs.old) 
    {            
        //printk(KERN_ALERT"\n\n    freqs.new > freqs.old : new=%dkHz, old=%dkHz \n\n  ", freqs.new, freqs.old);

#if 1           
        ret = regulator_set_voltage(vddca9, dvfs->vddarm_min, dvfs->vddarm_max);
        if (ret != 0)
        {
            pr_err("Failed to set VDDARM for %dkHz: %d\n", freqs.new, ret);
            goto err;
        }
            
        udelay(ui_cpu_regulator_latency); 
        
#endif
            
    }
#endif
	//printk(KERN_ALERT"\n Seting ARM CA9 core frequency : %dkHz, voltage: %d \n\n  ", freqs.new, dvfs->vddarm_min);

/*
	ul_clk_rate = clk_get_rate(ca9clk) & 0x00FFFFFF;
    printk(KERN_ALERT"\n   ca9clk = %d :\n ", ul_clk_rate);

    ul_clk_rate = clk_get_rate(pl310clk);
    printk(KERN_ALERT"\n   pl310clk = %d :\n ", ul_clk_rate);

    ul_clk_rate = clk_get_rate(ca9_peripheral_clk);
    printk(KERN_ALERT"\n   ca9_peripheral_clk = %d :\n ", ul_clk_rate);

    ul_clk_rate = clk_get_rate(l2_tag_ram_clk);
    printk(KERN_ALERT"\n   l2_tag_ram_clk = %d :\n ", ul_clk_rate);

    ul_clk_rate = clk_get_rate(l2_data_ram_clk);
    printk(KERN_ALERT"\n   l2_data_ram_clk = %d :\n ", ul_clk_rate);

    ul_clk_rate = clk_get_rate(l2_axi_master_clk);
    printk(KERN_ALERT"\n   l2_axi_master_clk = %d :\n ", ul_clk_rate);
*/

#if 1 

    if (pl310_ratio.old < pl310_ratio.new)
    {
        /*         
        当前需要提高PL310 (L2 controller) 的分频比率，比如: 分频比从PL310_CLK = CA9_Clock/1  提升为PL310_CLK = CA9_Clock /2，
        提高分频比率相当于降低PL310 (L2 controller)的时钟频率；

        为防止调整AMR CA9 CPU Core 主频时，L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock  瞬间超越最高频率极限而引起CPU Crash        
        所以需要在提升AMR CA9 CPU Core 主频之前，先设置降低PL310 (L2 controller) 的工作频率，
        保证L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock  任何时刻都运行在安全的频率范围
        */
        
    	/* Increase the pl310 div ratio , decrease the pl310 working frequency */

        /* 设置PL310 (L2 controller) 的分频比率，降低PL310 (L2 controller)的时钟频率*/
        ret = clk_set_rate(pl310clk, pl310_ratio.new | 0x80000000);
        if (ret < 0) 
        {
            pr_err("Failed to set rate %dkHz: %d\n", pl310_ratio.new, ret);
	        goto err;
	    }
 
        
        /* 设置L2 TAG RAM  的分频比率*/
        ret = clk_set_rate(l2_tag_ram_clk, l2_tag_ratio.new | 0x80000000);
        if (ret < 0) 
        {
        	pr_err("Failed to set rate %dkHz: %d\n", l2_tag_ratio.new, ret);
			goto err;
	    }

        /* 设置L2 DATA RAM  的分频比率*/
        ret = clk_set_rate(l2_data_ram_clk, l2_data_ratio.new | 0x80000000);
        if (ret < 0) 
        {
        	pr_err("Failed to set rate %dkHz: %d\n", l2_data_ratio.new, ret);
			goto err;
	    }

         /* 设置L2 AXI Master 0/1   的分频比率*/
		ret = clk_set_rate(l2_axi_master_clk, l2_axi_ratio.new | 0x80000000);
        if (ret < 0) 
        {
        	pr_err("Failed to set rate %dkHz: %d\n", l2_axi_ratio.new, ret);
			goto err;
	    }
    }
    else if (pl310_ratio.old == pl310_ratio.new)
    {
        /*
        如果PL310 (L2 controller) 的分频比率保持不变，
        判断是否需要提高L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock 的分频比率
        提高分频比率相当于降低时钟频率；

        为防止调整AMR CA9 CPU Core 主频时，L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock 瞬间超越最高频率极限而引起CPU Crash        
        所以需要在提升AMR CA9 CPU Core 主频之前，先设置降低L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock  的工作频率，
        保证L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock  任何时刻都运行在安全的频率范围
        */
        
        if (l2_tag_ratio.old < l2_tag_ratio.new )
        {
            /* 设置L2 TAG RAM clock 的分频比率，降低L2 TAG RAM clock 的时钟频率*/
            ret = clk_set_rate(l2_tag_ram_clk, l2_tag_ratio.new | 0x80000000);
            if (ret < 0) 
            {
        		pr_err("Failed to set rate %dkHz: %d\n", l2_tag_ratio.new, ret);
		    	goto err;
	        }                
        }

        if (l2_data_ratio.old < l2_data_ratio.new )
        {
            /* 设置L2 DATA RAM clock 的分频比率，降低L2 DATA RAM clock 的时钟频率*/       
            ret = clk_set_rate(l2_data_ram_clk, l2_data_ratio.new | 0x80000000);
            if (ret < 0) 
            {
            	pr_err("Failed to set rate %dkHz: %d\n", l2_data_ratio.new, ret);
		    	goto err;
	        }                
        }

        if (l2_axi_ratio.old < l2_axi_ratio.new )
        {
            /* 设置L2 AXI Master 0/1 的分频比率，降低L2 AXI Master 0/1 的时钟频率*/       
            ret = clk_set_rate(l2_axi_master_clk, l2_axi_ratio.new | 0x80000000);
            if (ret < 0) 
            {
            	pr_err("Failed to set rate %dkHz: %d\n", l2_axi_ratio.new, ret);
		    	goto err;
                
	        }             
        }            
    }

    /* 根据CPU Freq 算法模块计算的最新频率，设置当前ARM CA9  CPU Core 工作频率*/       
    ret = clk_set_rate(ca9clk, freqs.new);
    if (ret < 0)
    {
        pr_err("Failed to set rate %dkHz: %d\n", freqs.new, ret);
        goto err;
    }

    if (pl310_ratio.old > pl310_ratio.new)
    {
        /*         
        当前需要降低PL310 (L2 controller) 的分频比率，比如: 分频比从PL310_CLK = CA9_Clock/2  降低为PL310_CLK = CA9_Clock /1，
        降低分频比率相当于提升PL310 (L2 controller) 的时钟频率；

        为防止调整AMR CA9 CPU Core 主频时，L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock  瞬间超越最高频率极限而引起CPU Crash        
        所以需要在降低AMR CA9 CPU Core 主频之后，再设置提升PL310 (L2 controller) 的工作频率，
        保证L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock  任何时刻都运行在安全的频率范围
        */

    	/* Decrease the pl310 div ratio , increase the pl310 working frequency */

        /* 设置PL310 (L2 controller) 的分频比率，提升PL310 (L2 controller)的时钟频率*/
        ret = clk_set_rate(pl310clk, pl310_ratio.new | 0x80000000);
        if (ret < 0) 
        {
        	pr_err("Failed to set rate %dkHz: %d\n", pl310_ratio.new, ret);
			goto err;
	    }

        /* 设置L2 TAG RAM clock 的分频比率*/
        ret = clk_set_rate(l2_tag_ram_clk, l2_tag_ratio.new | 0x80000000);
        if (ret < 0) 
        {
        	pr_err("Failed to set rate %dkHz: %d\n", l2_tag_ratio.new, ret);
			goto err;
        }

        /* 设置L2 DATA RAM  的分频比率*/
        ret = clk_set_rate(l2_data_ram_clk, l2_data_ratio.new | 0x80000000);
        if (ret < 0) 
        {
        	pr_err("Failed to set rate %dkHz: %d\n", l2_data_ratio.new, ret);
			goto err;
        }

        /* 设置L2 AXI Master 0/1   的分频比率*/
        ret = clk_set_rate(l2_axi_master_clk, l2_axi_ratio.new | 0x80000000);
        if (ret < 0) 
        {
        	pr_err("Failed to set rate %dkHz: %d\n", l2_axi_ratio.new, ret);
			goto err;
	    }
            
    }
    else if (pl310_ratio.old == pl310_ratio.new)
    {
        /*
        如果PL310 (L2 controller) 的分频比率保持不变，
        判断是否需要降低L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock 的分频比率
        降低分频比率相当于提升时钟频率；

        为防止调整AMR CA9 CPU Core 主频时，L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock 瞬间超越最高频率极限而引起CPU Crash        
        所以需要在降低AMR CA9 CPU Core 主频之后，再设置提升L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock  的工作频率，
        保证L2 TAG RAM clock /L2 DATA RAM clock /L2 AXI Master 0/1 clock  任何时刻都运行在安全的频率范围
        */

        if (l2_tag_ratio.old > l2_tag_ratio.new )
        {
            /* 设置L2 TAG RAM clock 的分频比率，提升L2 TAG RAM clock 的时钟频率*/
            ret = clk_set_rate(l2_tag_ram_clk, l2_tag_ratio.new | 0x80000000);
            if (ret < 0) 
            {
            	pr_err("Failed to set rate %dkHz: %d\n", l2_tag_ratio.new, ret);
		    	goto err;
	        }                
        }

        if (l2_data_ratio.old > l2_data_ratio.new )
        {
             /* 设置L2 DATA RAM clock 的分频比率，提升L2 DATA RAM clock 的时钟频率*/     
            ret = clk_set_rate(l2_data_ram_clk, l2_data_ratio.new | 0x80000000);
            if (ret < 0) 
            {
            	pr_err("Failed to set rate %dkHz: %d\n", l2_data_ratio.new, ret);
		    	goto err;
	        }                
        }

        if (l2_axi_ratio.old > l2_axi_ratio.new )
        {
            /* 设置L2 AXI Master 0/1 的分频比率，提升L2 AXI Master 0/1 的时钟频率*/       
            ret = clk_set_rate(l2_axi_master_clk, l2_axi_ratio.new | 0x80000000);
            if (ret < 0) 
            {
        		pr_err("Failed to set rate %dkHz: %d\n", l2_axi_ratio.new, ret);
		    	goto err;
	        }                
        }            
    }


	//printk(KERN_ALERT"\n\n   clk_get_rate : %dkHz: %d \n\n  ", clk_get_rate(ca9clk));     
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);

    /* Sync the new frequency to both CPU0 & CPU1 */
#if 1
    if (freqs.cpu == 0)
    {
        freqs.cpu = 1;
        policy->cpu = 1;        
    }
    else
    {
        freqs.cpu = 0;
        policy->cpu = 0;                
    }
    cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);
#endif        
        
#else
    cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

#if 1
     if (freqs.cpu == 0)
     {
         freqs.cpu = 1;    
     }
     else
     {
         freqs.cpu = 0;
     }
     cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
#endif
        
#endif	
#endif 	

#ifdef CONFIG_REGULATOR
    if (vddca9 && freqs.new < freqs.old) 
    {
        //printk(KERN_ALERT"\n\n    freqs.new < freqs.old : new=%dkHz, old=%dkHz \n\n  ", freqs.new, freqs.old);
#if 1            
        ret = regulator_set_voltage(vddca9, dvfs->vddarm_min, dvfs->vddarm_max);
        if (ret != 0) 
       {
            pr_err("Failed to set VDDARM for %dkHz: %d\n", freqs.new, ret);
	    goto err_clk;
       }
#endif 

    }
#endif

    if (g_ali_cpufreq_debug_ctrl != 0)
    {
        printk(KERN_EMERG"\nARM CA9 core actual frequency = %lukHz\n", clk_get_rate(ca9clk));
        printk(KERN_EMERG"PL310 CLK = %lukHz,  L2 TAG RAM CLK = %lukHz, L2 DATA RAM CLK = %lukHz, L2 AXI Master CLK = %lukHz \n" 
					       ,clk_get_rate(pl310clk) & 0x00FFFFFF, clk_get_rate(l2_tag_ram_clk) & 0x00FFFFFF, clk_get_rate(l2_data_ram_clk) & 0x00FFFFFF, clk_get_rate(l2_axi_master_clk) & 0x00FFFFFF);
        
        printk(KERN_EMERG"PL310 clock ratio = %lu,  L2 TAG RAM clock ratio = %lu, L2 DATA RAM clock ratio = %lu, L2 AXI Master clock ratio = %lu \n\n" 
                                               ,clk_get_rate(pl310clk) >> 24, clk_get_rate(l2_tag_ram_clk) >> 24, clk_get_rate(l2_data_ram_clk) >> 24, clk_get_rate(l2_axi_master_clk) >> 24);
    }
        
    return 0;

err_clk:
    
    if (clk_set_rate(ca9clk, freqs.old) < 0)
    {
		pr_err("Failed to restore original clock rate\n");
    }
		
err:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))	
    cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);

    /* Sync the new frequency to both CPU0 & CPU1 */
#if 1
    if (0 == freqs.cpu)
    {
        freqs.cpu = 1;
        policy->cpu = 1;        
    }
    else
    {
        freqs.cpu = 0;
        policy->cpu = 0;                
    }
    cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);
#endif

#else
    cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

    /* Sync the new frequency to both CPU0 & CPU1 */
#if 1
     if (0 == freqs.cpu)
    {
        freqs.cpu = 1;    
    }
    else
    {
        freqs.cpu = 0;
    }    
    cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
#endif    
        
#endif
	return ret;
}

#if 0
static void __init s3921_cpufreq_config_regulator(void)
{
	int count, v, i, found;
	struct cpufreq_frequency_table *freq;
	struct s3921_dvfs *dvfs;

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
            printk(KERN_EMERG"%dkHz unsupported by regulator\n", freq->frequency);
                        
			freq->frequency = CPUFREQ_ENTRY_INVALID;
		}

		freq++;
	}

	/* Guess based on having to do an I2C/SPI write; in future we
	 * will be able to query the regulator performance here. */
	regulator_latency = 1 * 1000 * 1000;
}
#endif

/*
extern int ali_i2c_read(u32 id, u8 slv_addr, u8 *data, int len);


extern int ali_i2c_write(u32 id, u8 slv_addr, u8 *data, int len);
    
extern int ali_i2c_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int rlen);
*/

#define I2C_TYPE_SCB			0x00000000
#define I2C_TYPE_GPIO			0x00010000
#define I2C_TYPE_SCB_RM		0x00020000

// For TI2C_ISR
#define	ISR_TDI			0x01	// Transaction Done Interrupt
// For TI2C_ISR
#define ISR1_TRIG	    0x01
#define I2C_TYPE_SCB0			(I2C_TYPE_SCB|0)
#define I2C_TYPE_SCB1			(I2C_TYPE_SCB|1)
#define I2C_TYPE_SCB2			(I2C_TYPE_SCB|2)
#define I2C_TYPE_SCB3			(I2C_TYPE_SCB|3)

#define I2C_TYPE_GPIO0			(I2C_TYPE_GPIO|0)
#define I2C_TYPE_GPIO1			(I2C_TYPE_GPIO|1)
#define I2C_TYPE_GPIO2			(I2C_TYPE_GPIO|2)
#define I2C_TYPE_GPIO3			(I2C_TYPE_GPIO|3)

int ali_i2c_scb_write(u32 id, u8 slv_addr, u8 *data, int len);
int ali_i2c_scb_read(u32 id, u8 slv_addr, u8 *data, int len);
int ali_i2c_scb_write_read(u32 id, u8 slv_addr, u8 *data, int wlen, int len);


static int s3921_cpufreq_driver_init(struct cpufreq_policy *policy)
{
    int ret;
    struct cpufreq_frequency_table *freq;

    unsigned long *ul_chipid_version_addr = NULL;
    unsigned long ul_reg_val = 0;
        
    ul_chipid_version_addr = (unsigned long*)ioremap((PHYS_SYSTEM + 0), sizeof(unsigned long));
    ul_reg_val = *ul_chipid_version_addr;
    ul_reg_val = (ul_reg_val >> 8) & 0x01;

    if (0 == ul_reg_val)
    {
        /* The Chip Package type is QFP */
        ui_cpu_packaging_type = S3921_CPU_PACKAGING_TYPE_QFP;
      
    }
    else
    {
         /* The Chip Package type is BGA */    
        ui_cpu_packaging_type = S3921_CPU_PACKAGING_TYPE_BGA;
    }

    ui_cpu_regulator_table_type = S3921_CPU_REGULATOR_TABLE_W;

    printk(KERN_ALERT"\ns3921 cpufreq driver init\n");

    if (policy->cpu != 0)
    {
        return -EINVAL;
    }

#if 0
    if (s3921_freq_table == NULL) 
    {
        pr_err("No frequency information for this CPU\n");
        return -ENODEV;
    }
#endif

    ui_cpu_regulator_latency = 350;
    
#if 0
       struct device ca9_clk_device =
       {
            .init_name = "s3921-ca9-clock"
       } ;
#endif

    ca9clk = clk_get(NULL, "ca9_arm_clk");
    //printk(KERN_ALERT"\n\n\n\n\n   ca9clk = 0x%x :\n \n\n\n\n\n  ", ca9clk);
        
    if (IS_ERR(ca9clk)) 
    {
        pr_err("Unable to obtain ARMCLK: %ld\n",
        PTR_ERR(ca9clk));
        return PTR_ERR(ca9clk);
    }

    pl310clk = clk_get(NULL, "pl310_ctrl_clk");

    //printk(KERN_ALERT"\n\n\n\n\n   pl310clk = 0x%x :\n \n\n\n\n\n  ", pl310clk);
        
    if (IS_ERR(pl310clk)) 
    {
        pr_err("Unable to obtain pl310clk: %ld\n",
	    PTR_ERR(pl310clk));
	    return PTR_ERR(pl310clk);
    }
        

    ca9_peripheral_clk = clk_get(NULL, "ca9_peripheral_clk");

    if (IS_ERR(ca9_peripheral_clk)) 
    {
        pr_err("Unable to obtain ca9_peripheral_clk: %ld\n",
		PTR_ERR(ca9_peripheral_clk));
		return PTR_ERR(ca9_peripheral_clk);
    }

    //printk(KERN_ALERT"\n\n\n\n\n   ca9_peripheral_clk = 0x%x :\n \n\n\n\n\n  ", ca9_peripheral_clk);

    l2_tag_ram_clk = clk_get(NULL, "l2_tag_ram_clk");
    
    if (IS_ERR(l2_tag_ram_clk)) 
    {
        pr_err("Unable to obtain l2_tag_ram_clk: %ld\n",
	    PTR_ERR(l2_tag_ram_clk));
        return PTR_ERR(l2_tag_ram_clk);
    }

    //printk(KERN_ALERT"\n\n\n\n\n   l2_tag_ram_clk = 0x%x :\n \n\n\n\n\n  ", l2_tag_ram_clk);

    l2_data_ram_clk = clk_get(NULL, "l2_data_ram_clk");

    if (IS_ERR(l2_data_ram_clk)) 
    {
        pr_err("Unable to obtain l2_data_ram_clk: %ld\n",
	    PTR_ERR(l2_data_ram_clk));
	    return PTR_ERR(l2_data_ram_clk);
    }

    //printk(KERN_ALERT"\n\n\n\n\n   l2_data_ram_clk = 0x%x :\n \n\n\n\n\n  ", l2_data_ram_clk);

    l2_axi_master_clk = clk_get(NULL, "l2_axi_master_clk");

    if (IS_ERR(l2_axi_master_clk)) 
    {
        pr_err("Unable to obtain l2_axi_master_clk: %ld\n",
	    PTR_ERR(l2_axi_master_clk));
	    return PTR_ERR(l2_axi_master_clk);
    }

#if 0 

    printk(KERN_ALERT"\n\n\n\n\n   Prepare  to test the I2C :\n \n\n\n\n\n  ");

    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);

   a_uc_i2c_buffer[0] = 0x01;


   ret = ali_i2c_scb_write_read(I2C_TYPE_SCB1, 0x90, a_uc_i2c_buffer, 1, 1);

   printk(KERN_ALERT"\n\n\n\n\n   ali_i2c_write_read, ret = 0x%x; data1 = 0x%x, data2 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[0], a_uc_i2c_buffer[1]);

/*
   ret = ali_i2c_write_read(I2C_TYPE_SCB1, 0x48, a_uc_i2c_buffer, 1, 1);

   printk(KERN_ALERT"\n\n\n\n\n   s3921_cpufreq_driver_init, ali_i2c_write_read, ret = 0x%x; data1 = 0x%x, data2 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[0], a_uc_i2c_buffer[1]);

   ret = ali_i2c_write_read(2, 0x48, a_uc_i2c_buffer, 1, 1);

   printk(KERN_ALERT"\n\n\n\n\n   s3921_cpufreq_driver_init, ali_i2c_write_read, ret = 0x%x; data1 = 0x%x, data2 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[0], a_uc_i2c_buffer[1]);

   ret = ali_i2c_write_read(3, 0x48, a_uc_i2c_buffer, 1, 1);

   printk(KERN_ALERT"\n\n\n\n\n   s3921_cpufreq_driver_init, ali_i2c_write_read, ret = 0x%x; data1 = 0x%x, data2 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[0], a_uc_i2c_buffer[1]);
*/   

    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);
    msleep(1000);

   a_uc_i2c_buffer[0] = 0x01;
    
   ret = ali_i2c_scb_write(I2C_TYPE_SCB1, 0x48, a_uc_i2c_buffer, 1);

   printk(KERN_ALERT"\n\n\n\n\n  ali_i2c_write I2C_TYPE_SCB1,  ret = 0x%x; data1 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[0]);

   ret = ali_i2c_scb_read(I2C_TYPE_SCB1, 0x48, &a_uc_i2c_buffer[1], 1);
   
   printk(KERN_ALERT"\n\n\n\n\n   ali_i2c_read I2C_TYPE_SCB1, ret = 0x%x; data2 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[1]);

/*
  a_uc_i2c_buffer[0] = 0x01;
  a_uc_i2c_buffer[1] = 0x00;
  
  ret = ali_i2c_write(I2C_TYPE_SCB1, 0x48, a_uc_i2c_buffer, 2);

   printk(KERN_ALERT"\n\n\n\n\n   ali_i2c_read I2C_TYPE_SCB1, ret = 0x%x; data2 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[1]);
*/   

/*   
   ret = ali_i2c_write(I2C_TYPE_SCB1, 0x48, a_uc_i2c_buffer, 1);

   printk(KERN_ALERT"\n\n\n\n\n  ali_i2c_write I2C_TYPE_SCB0,  ret = 0x%x; data1 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[0]);

   ret = ali_i2c_read(I2C_TYPE_SCB1, 0x48, &a_uc_i2c_buffer[1], 1);
   
   printk(KERN_ALERT"\n\n\n\n\n   ali_i2c_read I2C_TYPE_SCB0, ret = 0x%x; data2 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[1]);

   ret = ali_i2c_write(I2C_TYPE_SCB2, 0x48, a_uc_i2c_buffer, 1);

   printk(KERN_ALERT"\n\n\n\n\n  ali_i2c_write I2C_TYPE_SCB0,  ret = 0x%x; data1 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[0]);

   ret = ali_i2c_read(I2C_TYPE_SCB2, 0x48, &a_uc_i2c_buffer[1], 1);
   
   printk(KERN_ALERT"\n\n\n\n\n   ali_i2c_read I2C_TYPE_SCB0, ret = 0x%x; data2 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[1]);

   ret = ali_i2c_write(I2C_TYPE_SCB3, 0x48, a_uc_i2c_buffer, 1);

   printk(KERN_ALERT"\n\n\n\n\n  ali_i2c_write I2C_TYPE_SCB0,  ret = 0x%x; data1 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[0]);

   ret = ali_i2c_read(I2C_TYPE_SCB3, 0x48, &a_uc_i2c_buffer[1], 1);
   
   printk(KERN_ALERT"\n\n\n\n\n   ali_i2c_read I2C_TYPE_SCB0, ret = 0x%x; data2 = 0x%x \n \n\n\n\n\n  ", ret, a_uc_i2c_buffer[1]);
*/

#endif

#ifdef CONFIG_REGULATOR
    vddca9 = regulator_get(NULL, "vddarm_s3921");
    if (IS_ERR(vddca9)) {
        ret = PTR_ERR(vddca9);
	    pr_err("Failed to obtain vddarm_s3921: %d\n", ret);
	    pr_err("Only frequency scaling available\n");

        ui_cpu_regulator_status = S3921_CPU_REGULATOR_OFFLINE;
	    vddca9 = NULL;
    } 
    else 
    {
        ui_cpu_regulator_status = S3921_CPU_REGULATOR_ONLINE;
        
        /* Guess based on having to do an I2C/SPI write; in future we
	 * will be able to query the regulator performance here. */
        regulator_latency = 1 * 1000 * 1000;

        //s3921_cpufreq_config_regulator();
    }
#endif

    freq = s3921_freq_table;
    while (freq->frequency != CPUFREQ_TABLE_END) 
    {
        unsigned long r;

        /* Check for frequencies we can generate */
        r = clk_round_rate(ca9clk, freq->frequency);
        
        if (r != freq->frequency) 
        {
	    pr_debug("%dkHz unsupported by clock\n", freq->frequency);
	    freq->frequency = CPUFREQ_ENTRY_INVALID;
        }

		/* If we have no regulator then assume startup frequency is the maximum we can support. */
#if 0		 
		if (!vddca9 && freq->frequency > s3921_cpufreq_get_speed(0))
		{
			freq->frequency = CPUFREQ_ENTRY_INVALID;
		}
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
    if (ret != 0)
    {
        pr_err("Failed to configure frequency table: %d\n", ret);
        regulator_put(vddca9);
        clk_put(ca9clk);
    }


    if  (S3921_CPU_PACKAGING_TYPE_BGA == ui_cpu_packaging_type)
    {
        //policy->min = policy->cpuinfo.min_freq = S3921_FREQUENCY_500M;
        policy->max = S3921_FREQUENCY_1200M;
    }
    else
    {
        //policy->min = policy->cpuinfo.min_freq = S3921_FREQUENCY_300M;
        policy->max = S3921_FREQUENCY_800M;
    }
    

    return ret;
}

static struct cpufreq_driver s3921_cpufreq_driver = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
#else	
	.owner        = THIS_MODULE,
#endif
	.flags          = 0,
	.verify         = s3921_cpufreq_verify_speed,
	.target        = s3921_cpufreq_set_target,
	.get		   = s3921_cpufreq_get_speed,
	.init	           = s3921_cpufreq_driver_init,
	.name          = "s3921",
};


extern int register_eta1459_i2c(void);


static int __init s3921_cpufreq_init(void)
{
    int rc;
    
    pr_debug("\ns3921 cpufreq init\n");
        
    //printk(KERN_ALERT"\n s3921_cpufreq_init \n  ");
    cpuinfo_local_kobject = kobject_create_and_add("cpuinfo", &cpu_subsys.dev_root->kobj);
    BUG_ON(!cpuinfo_local_kobject);

    rc = sysfs_create_file(cpuinfo_local_kobject, &cpu_packaging.attr);

    rc |= sysfs_create_file(cpuinfo_local_kobject, &cpu_regulator_status.attr);

    rc |= sysfs_create_file(cpuinfo_local_kobject, &cpu_regulator_table.attr);

    rc |= sysfs_create_file(cpuinfo_local_kobject, &cpu_regulator_latency.attr);

    rc |= sysfs_create_file(cpuinfo_local_kobject, &cpu_cpufreq_debug_ctrl.attr);

    rc |= sysfs_create_file(cpuinfo_local_kobject, &cpu_pm_threshold.attr);

    rc |= register_eta1459_i2c();
    
    rc |= cpufreq_register_driver(&s3921_cpufreq_driver);

    //rc |= register_eta1459_i2c();

    //printk(KERN_ALERT"\n\n\n\n\n ******************** s3921_cpufreq_init: rc = %d ********************\n\n\n\n\n\n\n\n\n\n", rc);
        
    return rc;
}
//module_init(s3921_cpufreq_init);

static int __init get_pm_counter(char *s)
{
    unsigned int ui_pm_counter = 0;
    int ret = 0;
    if (s != NULL) 
    {        
        ret = sscanf(s, "%u", &ui_pm_counter);
        printk("\n^^cmdline: pm_count string = %s,  pm_count value = %u^\n", s,  ui_pm_counter);         
    }
    return 0;
}

static int __init get_per_cell_delay(char *s)
{
    unsigned int ui_per_cell_delay = 0;
    int ret = 0;
    if (s != NULL) 
    {        
        ret = sscanf(s, "%u", &ui_per_cell_delay);
        printk("\n^^cmdline: per_cell_delay string = %s,  per_cell_delay value = %u^\n", s,  ui_per_cell_delay);
    }
    ui_cpu_per_cell_delay_value = ui_per_cell_delay / 100000;
    return 0;
}

__setup("pm_counter=", get_pm_counter);

__setup("per_cell_delay=", get_per_cell_delay);


module_init(s3921_cpufreq_init);

//subsys_initcall(s3921_cpufreq_init);
