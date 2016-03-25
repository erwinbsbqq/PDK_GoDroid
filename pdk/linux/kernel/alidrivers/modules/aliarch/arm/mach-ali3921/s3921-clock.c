/*linux/arch/arm/mach-ali3921/s3921-clock.c
 *
 * Copyright 2013 ALI Tech, Inc.
 *
 * s3921 Base clock support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/err.h>

//#include <linux/io.h>

#include <asm/io.h>

#include <mach/clock.h>

#include <mach/s3921-clock.h>
#include <mach/timex.h>


#include <linux/delay.h>

/* Get the CPLL clock output frequence,  CPLL is clock source in the CPU system */
static unsigned long s3921_cpll_fout_get_rate(struct clk *clk);

/* Get the cpll fout round frequence value according to the user setting  rate */
static unsigned long s3921_cpll_fout_round_rate(struct clk *clk, unsigned long rate);

/* Set the cpll fout frequency based on the user setting rate */
static int s3921_cpll_fout_set_rate(struct clk *clk, unsigned long rate);

/* Get the ARM CA9  core working clock frequence */
static unsigned long s3921_clk_ca9_get_rate(struct clk *clk);

/* Get the ARM CA9  core  frequence value according to the user setting  rate */
static unsigned long s3921_clk_ca9_round_rate(struct clk *clk, unsigned long rate);

/* Set the ARM ca9 working frequency based on the user setting rate */
static int s3921_clk_ca9_set_rate(struct clk *clk, unsigned long rate);

static unsigned long s3921_clk_common_get_rate(struct clk *clk);

static unsigned long s3921_clk_common_round_rate(struct clk *clk, unsigned long rate);

static int s3921_clk_common_set_rate(struct clk *clk, unsigned long rate);


static inline unsigned long bit_mask(unsigned long shift, unsigned long nr_bits)
{
	unsigned long mask = 0xffffffff >> (32 - nr_bits);

	return mask << shift;
}


/* fin_apll, fin_mpll and fin_epll are all the same clock, which we call
 * ext_xtal_mux for want of an actual name from the manual.
*/

static struct clk clk_ext_xtal_mux = {
    .name   = "ext_xtal",
};

#if 0
static struct clk clk_bypass_clk_mux = {
	.name		= "bypass_clkl",
};
#endif

#define clk_fin_cpll clk_ext_xtal_mux

#define DELAY_COUNT 20


/* Define the cpll fout clock object  */
static struct clk clk_fout_cpll = 
{
    .name         = "cpll_fout",
    /*.devname   = "s3921-cpll-fout",*/
    .parent	      = &clk_fin_cpll,
    .enable      = NULL,

    .reg_div     = (struct clk_ratio_reg) 
    {
        .reg = S3921_STRAP_CTRL_CLOCK, 
        .shift = 8,
        .size = 3  
    },	

    .ops    =   &(struct clk_ops) 
    {
        .get_rate	    = s3921_cpll_fout_get_rate,
        .set_rate	    = s3921_cpll_fout_set_rate,
        .round_rate = s3921_cpll_fout_round_rate,
    },
        
};

/* Define the ARM CA9 clock object  */
static struct clk clk_arm_ca9 = 
{
    .name       = "ca9_arm_clk",
    /*.devname  = "s3921-ca9-clock",*/
    .parent	    = &clk_fout_cpll,
    .enable      = NULL,

    .uc_div_num = 5,
    .auc_ctrl_to_div_map = {1, 2, 4, 3, 0xff},
    .auc_div_to_ctrl_map = {0, 0, 1, 3, 2},
     
    .reg_div     = (struct clk_ratio_reg) 
    {
        .reg = S3921_CPU_CTRL_CLOCK, 
        .shift = 11,
        .size = 2  
    },	

    .ops    = &(struct clk_ops) 
    {
        .get_rate	= s3921_clk_ca9_get_rate,
        .set_rate	= s3921_clk_ca9_set_rate,
        .round_rate = s3921_clk_ca9_round_rate,
    },
};



/* Define the CA9 Peripheral clock object  */
static struct clk clk_ca9_peripheral = 
{
    .name       = "ca9_peripheral_clk",
    /*.devname  = "s3921-peripheral-clock",*/
    .parent	    = &clk_arm_ca9,
    .enable      = NULL,

    .uc_div_num = 5,
    .auc_ctrl_to_div_map = {2, 4, 0xff, 0xff, 0xff},
    .auc_div_to_ctrl_map = {0, 0, 0, 0, 1},
     
    .reg_div     =  (struct clk_ratio_reg) 
    {
        .reg = S3921_CPU_CTRL_CLOCK, 
        .shift = 31,
        .size = 1  
    },	

    .ops    = &(struct clk_ops) 
    {
        .get_rate	= s3921_clk_common_get_rate,
        .set_rate	= s3921_clk_common_set_rate,
        .round_rate = s3921_clk_common_round_rate,
    },
};



/* Define the PL310 (L2 cache controller) clock object  */
static struct clk clk_pl310_controller = 
{
    .name       = "pl310_ctrl_clk",
    /*.devname  = "s3921-pl310-clock",*/
    .parent	    = &clk_arm_ca9,
    .enable      = NULL,

    .uc_div_num = 5,
    .auc_ctrl_to_div_map = {1, 0x0, 2, 3, 0xff},
    .auc_div_to_ctrl_map = {0, 0, 2, 3, 0},
     
    .reg_div     = (struct clk_ratio_reg) 
    {
        .reg = S3921_CPU_CTRL_CLOCK, 
        .shift = 28,
        .size = 2  
    },	

    .ops    = &(struct clk_ops) 
    {
        .get_rate	= s3921_clk_common_get_rate,
        .set_rate	= s3921_clk_common_set_rate,
        .round_rate = s3921_clk_common_round_rate,
    },
};


/* Define the L2 TAG RAM clock object  */
static struct clk clk_l2_tag_ram = 
{
    .name       = "l2_tag_ram_clk",
    /*.devname  = "s3921-l2-tag-ram-clock",*/
    .parent	    = &clk_pl310_controller,
    .enable      = NULL,

    .uc_div_num = 5,
    .auc_ctrl_to_div_map = {1, 2, 3, 4, 0xff},
    .auc_div_to_ctrl_map = {0, 0, 1, 2, 3},
     
    .reg_div     = (struct clk_ratio_reg) 
    {
        .reg = S3921_CPU_CTRL_CLOCK, 
        .shift = 26,
        .size = 2  
    },	

    .ops    = &(struct clk_ops) 
    {
        .get_rate	= s3921_clk_common_get_rate,
        .set_rate	= s3921_clk_common_set_rate,
        .round_rate = s3921_clk_common_round_rate,
    },
};


/* Define the L2 DATA RAM clock object  */
static struct clk clk_l2_data_ram = 
{
    .name       = "l2_data_ram_clk",
    /*.devname  = "s3921-l2-data-ram-clock",*/
    .parent	    = &clk_pl310_controller,
    .enable      = NULL,

    .uc_div_num = 5,
    .auc_ctrl_to_div_map = {1, 2, 3, 4, 0xff},
    .auc_div_to_ctrl_map = {0, 0, 1, 2, 3},
     
    .reg_div     =  (struct clk_ratio_reg) 
    {
        .reg = S3921_CPU_CTRL_CLOCK, 
        .shift = 24,
        .size = 2  
    },	

    .ops    = &(struct clk_ops) 
    {
        .get_rate	= s3921_clk_common_get_rate,
        .set_rate	= s3921_clk_common_set_rate,
        .round_rate = s3921_clk_common_round_rate,
    },
};


/* Define the L2 AXI Master 0/1 clock object  */
static struct clk clk_l2_axi_master = 
{
    .name       = "l2_axi_master_clk",
    /*.devname  = "s3921-l2-axi-master-clock",*/
    .parent	    = &clk_pl310_controller,
    .enable      = NULL,

    .uc_div_num = 5,
    .auc_ctrl_to_div_map = {1, 2, 3, 4, 0xff},
    .auc_div_to_ctrl_map = {0, 0, 1, 2, 3},
     
    .reg_div     = (struct clk_ratio_reg) 
    {
        .reg = S3921_CPU_CTRL_CLOCK, 
        .shift = 22,
        .size = 2  
    },	

    .ops    = &(struct clk_ops) 
    {
        .get_rate	= s3921_clk_common_get_rate,
        .set_rate	= s3921_clk_common_set_rate,
        .round_rate = s3921_clk_common_round_rate,
    },
};


/* Define the cpll output frequency mapping table, unit : kHz */
static const unsigned long g_cpll_clock_select[CPLL_FOUT_SELECT_END] = {800000, 900000, 1000000, 1100000, 1200000, 1300000, 1400000, 1500000};

/* Define the CA9 Clock real work Frequency mapping table, unit : kHz */
static const unsigned long g_ca9_clock_select[CA9_CLOCK_SELECT_END] = 
{
    200000, 225000, 250000, 275000, 300000, 325000, 350000, 375000,
    400000, 450000, 500000, 550000, 600000, 650000, 700000, 750000,
    800000, 900000, 1000000, 1100000, 1200000, 1300000, 1400000, 1500000
};


#if 0
static const unsigned long g_cpll_clock_select[CPLL_FOUT_SELECT_END] = {800000, 900000, 1000000};

static const unsigned long g_ca9_clock_select[CA9_CLOCK_SELECT_END] = 
{
    200000, 225000, 250000,
    400000, 450000, 500000, 
    800000, 900000, 1000000
};

#endif


/*************************************************************************
*
*   FUNCTION
*
*       s3921_cpll_fout_get_rate
*
*   DESCRIPTION
*
*       Get the CPLL clock output frequence,  CPLL is clock source in the CPU system .
*
*   INPUTS
*
*       @*clk   Pointer to the cpll fout clock object
*
*   OUTPUTS
*
*   RETURN
*       Current clock frequence value of the cpll fout
*
*
*************************************************************************/
static unsigned long s3921_cpll_fout_get_rate(struct clk *clk)
{
    unsigned long ul_reg_val = 0;
    
    unsigned long ul_clk_select = 0;
    unsigned long ul_clk_select_status = 0;
        
    unsigned long ul_bit_mask = 0;
    unsigned long ul_cpll_fout = 0;

    /* Caculate the clock selection bits mask */
    ul_bit_mask = bit_mask(clk->reg_div.shift, clk->reg_div.size);

    /* Read the Strap Pin Control Register value */
    ul_reg_val = readl(clk->reg_div.reg);

    /* Get the cpll clock selection control configuration */
    ul_clk_select = (ul_reg_val & ul_bit_mask) >> (clk->reg_div.shift);


    /* Read back the Strap Pin Control Register value */
    ul_reg_val = readl(S3921_STRAP_INFO_CLOCK); 

    /* Get the cpll clock selection information */
    ul_clk_select_status = (ul_reg_val & ul_bit_mask) >> (clk->reg_div.shift);

    if (ul_clk_select != ul_clk_select_status)
    {
    	//printk(KERN_ALERT"\n The S3921 STRAP CTRL CLOCK: %d do not match the STRAP INFO CLOCK: %d \n ", ul_clk_select, ul_clk_select_status);
        ul_clk_select = ul_clk_select_status;
        
    }

    if (unlikely(ul_clk_select > CPLL_FOUTSEL_SELECT_1500M))
    {
        ul_clk_select = CPLL_FOUTSEL_SELECT_1500M;
    }

    /* Get the cpll output frequency value */
    ul_cpll_fout = g_cpll_clock_select[ul_clk_select];
                 
    return ul_cpll_fout;
}


/*************************************************************************
*
*   FUNCTION
*
*       s3921_cpll_fout_round_rate
*
*   DESCRIPTION
*
*       Get the cpll fout round frequence value according to the user setting  rate.
*
*   INPUTS
*
*       @*clk   Pointer to the cpll fout clock object
*
*       @rate   User setting  rate for cpll fout clock
*
*   OUTPUTS
*
*   RETURN
*       Round frequence value support by  CPLL fout
*
*
*************************************************************************/
static unsigned long s3921_cpll_fout_round_rate(struct clk *clk, unsigned long rate)
{
    unsigned long ul_clk_round_rate = g_cpll_clock_select[CPLL_FOUTSEL_SELECT_1000M];
    int i = 0;
    
    (void)clk;

    /*Polling the cpll clock working frequency table, find out the most approximate working frequency according the user setting  rate*/
   /* The ul_clk_round_rate should be less or equal to the  user setting  rate*/
    for (i = (CPLL_FOUT_SELECT_END - 1); i >= 0; i--)
    {
        if (rate >=  g_cpll_clock_select[i])
        {
            ul_clk_round_rate =  g_cpll_clock_select[i];
            break;
        }
    }
    
    /* If the user setting  rate is less than the minimum working frequency, then the ul_clk_round_rate is  the minimum working frequency */
    if (i  < 0)
    {
        ul_clk_round_rate = g_cpll_clock_select[CPLL_FOUTSEL_SELECT_800M];

    }

    return ul_clk_round_rate;  
}


/*************************************************************************
*
*   FUNCTION
*
*       s3921_cpll_fout_set_rate
*
*   DESCRIPTION
*
*       Set the cpll fout frequency based on the user setting rate
*       Due to the user setting rate is random, it need translate the user setting rate to the clock frequency support by cpll fout
*   INPUTS
*
*       @*clk   Pointer to the cpll fout clock object
*
*       @rate   User setting  rate for cpll fout clock
*
*   OUTPUTS
*
*   RETURN
*       Implementation result:
*       0: Success,    other value: Fail
*
*
*************************************************************************/
static int s3921_cpll_fout_set_rate(struct clk *clk, unsigned long rate)
{
    unsigned long ul_clk_setting_rate = g_cpll_clock_select[CPLL_FOUTSEL_SELECT_1000M];
    unsigned long ul_clk_select_ctrl = CPLL_FOUTSEL_SELECT_1000M;
        
    unsigned long ul_reg_val = 0;
    unsigned long ul_bit_mask = 0;
   volatile unsigned long ul_delay_cnt = 0;
    
    int i = 0;
    
    /*Polling the cpll clock working frequency table, find out the most approximate working frequency according the user setting  rate*/
    for (i = (CPLL_FOUT_SELECT_END - 1); i >= 0; i--)
    {
        if (rate >=  g_cpll_clock_select[i])
        {
            ul_clk_setting_rate =  g_cpll_clock_select[i];
            ul_clk_select_ctrl = i;
            break;
        }
    }

    if (i  < 0)
    {
        ul_clk_setting_rate = g_cpll_clock_select[CPLL_FOUTSEL_SELECT_800M];
        ul_clk_select_ctrl = CPLL_FOUTSEL_SELECT_800M;
    }

    /* Caculate the cpll clock selection bits mask */
    ul_bit_mask = bit_mask(clk->reg_div.shift, clk->reg_div.size);
    
    /* Read back the Strap Pin Control Register value */
    ul_reg_val = readl(clk->reg_div.reg);
  
    /* Caculate the value written to the Strap Pin Control Register  */
    ul_reg_val &= ~ul_bit_mask;

    ul_reg_val |= (ul_clk_select_ctrl << clk->reg_div.shift);
    ul_reg_val |= CPLL_FOUTSEL_TRIG;

    /* Set the the Strap Pin Control Register with the user setting rate  */
    writel(ul_reg_val, clk->reg_div.reg);

    //dmb();
    for (ul_delay_cnt = DELAY_COUNT; ul_delay_cnt > 0; ul_delay_cnt--); 

    return S3921_CLK_SUCCESS;

}


/*************************************************************************
*
*   FUNCTION
*
*       s3921_clk_ca9_get_rate
*
*   DESCRIPTION
*
*       Get the ARM CA9  core working clock frequence.
*
*   INPUTS
*
*       @*clk   Pointer to ARM CA9  core clock object
*
*   OUTPUTS
*
*   RETURN
*       Current clock frequence value of ARM CA9  core
*
*
*************************************************************************/
static unsigned long s3921_clk_ca9_get_rate(struct clk *clk)
{
    unsigned long ul_rate = 0;
    unsigned long ul_reg_val = 0;    
    unsigned long ul_pll_div_ratio = 0;
    unsigned long ul_pll_div_value = 0;
    unsigned long ul_bit_mask = 0;
   
    ul_rate = clk_get_rate(clk->parent);

    //printk(KERN_ALERT"\n  **** s3921_clk_ca9_get_rate,  parent rate :%d,   \n ", ul_rate);

    /* Read the S3921 CPU CLOCK Control Register value */
    ul_reg_val = readl(clk->reg_div.reg);

    //printk(KERN_ALERT"\n  **** s3921_clk_ca9_get_rate,  reg_addr:0x%x,   reg_val: 0x%x  \n ", clk->reg_div.reg,  ul_reg_val);

    /* Caculate the  CA9  PLL_DIV_RATIO  bits mask */
    ul_bit_mask = bit_mask(clk->reg_div.shift, clk->reg_div.size);

    /* Get the CA9  PLL_DIV_RATIO  */
    ul_pll_div_ratio = (ul_reg_val & ul_bit_mask) >> (clk->reg_div.shift);

    if (ul_pll_div_ratio > clk->uc_div_num)
    {
        ul_pll_div_ratio = clk->uc_div_num;
    }

    /* Get the CA9  actual PLL DIV value */
    ul_pll_div_value = clk->auc_ctrl_to_div_map[ul_pll_div_ratio];

    /* Caculate the  CA9 Clock real work Frequency */
    ul_rate = ul_rate / ul_pll_div_value;
                 
    return ul_rate;
}



/*************************************************************************
*
*   FUNCTION
*
*       s3921_cpll_fout_round_rate
*
*   DESCRIPTION
*
*       Get the ARM CA9  core  frequence value according to the user setting  rate.
*
*   INPUTS
*
*       @*clk  Pointer to ARM CA9  core clock object
*
*       @rate   User setting  rate for ARM CA9  core 
*
*   OUTPUTS
*
*   RETURN
*       Round frequence value support by  ARM CA9  core 
*
*
*************************************************************************/
static unsigned long s3921_clk_ca9_round_rate(struct clk *clk, unsigned long rate)
{
    unsigned long ul_clk_round_rate = g_ca9_clock_select[CA9_CLOCK_SELECT_1000M];
    int i = 0;

    (void)clk;

    /*Polling the ca9 clock working frequency table, find out the most approximate working frequency according the user setting  rate*/
    /* The ul_clk_round_rate should be less or equal to the  user setting  rate*/
    for (i = (CA9_CLOCK_SELECT_END - 1); i >= 0; i--)
    {
        if (rate >=  g_ca9_clock_select[i])
        {
            ul_clk_round_rate =  g_ca9_clock_select[i];
            break;
        }
    }
    
    /* If the user setting  rate is less than the minimum working frequency, then the ul_clk_round_rate is  the minimum working frequency */
    if (i  < 0)
    {
        ul_clk_round_rate = g_ca9_clock_select[CA9_CLOCK_SELECT_200M];

    }
    
    return ul_clk_round_rate;  
}

/*************************************************************************
*
*   FUNCTION
*
*       s3921_clk_ca9_set_rate
*
*   DESCRIPTION
*
*       Set the ARM ca9 working frequency based on the user setting rate
*       Due to the user setting rate is random, it need translate the user setting rate to the clock frequency support by  ARM ca9 
*   INPUTS
*
*       @*clk   Pointer to ARM CA9  core clock object
*
*       @rate   User setting  rate for Pointer to ARM CA9  core clock
*
*   OUTPUTS
*
*   RETURN
*       Implementation result:
*       0: Success,    other value: Fail
*
*
*************************************************************************/
static int s3921_clk_ca9_set_rate(struct clk *clk, unsigned long rate)
{
    unsigned long ul_clk_setting_rate = g_ca9_clock_select[CA9_CLOCK_SELECT_1000M];
    unsigned long ul_ca9_clk_select_ctrl = CA9_CLOCK_SELECT_1000M;
    unsigned long ul_cpll_clk_select_ctrl = CPLL_FOUTSEL_SELECT_1000M;

    unsigned long ul_ca9_div = 0;
    unsigned long ul_rate = 0;

    unsigned long ul_reg_val = 0;
    unsigned long ul_clk_div_ctrl = 0;
    
    unsigned long ul_bit_mask = 0;

    volatile unsigned long ul_delay_cnt = 0;

    int i_ret_value = S3921_CLK_SUCCESS;
    int i = 0;

    /* Get the cpll fout clock frequency */
    ul_rate = clk_get_rate(clk->parent);

    /* Get the round CA9 clock frequency based on the user setting clock rate  */
    for (i = (CA9_CLOCK_SELECT_END - 1); i >= 0; i--)
    {
        if (rate >=  g_ca9_clock_select[i])
        {
            ul_clk_setting_rate =  g_ca9_clock_select[i];
            ul_ca9_clk_select_ctrl = i;
            break;
        }
    }

    if (i < 0)
    {      
        /* The user setting clock rate is less than  CA9 minimun support clock frequency   */
        ul_clk_setting_rate =  g_ca9_clock_select[CA9_CLOCK_SELECT_200M];
        ul_ca9_clk_select_ctrl = CA9_CLOCK_SELECT_200M;
    }

    /* Calculate the parent clock selection( cpll fout clock)  of  ca9 ,   and the  ca9 div value */
    if (ul_ca9_clk_select_ctrl < CA9_CLOCK_SELECT_400M)
    {
        ul_cpll_clk_select_ctrl = ul_ca9_clk_select_ctrl;
        ul_ca9_div = CA9_Clock_DIV_4;
    }
    else if (ul_ca9_clk_select_ctrl < CA9_CLOCK_SELECT_800M)
    {
        ul_cpll_clk_select_ctrl = ul_ca9_clk_select_ctrl - CA9_CLOCK_SELECT_400M;
        ul_ca9_div = CA9_Clock_DIV_2;      
    }
    else
    {
        ul_cpll_clk_select_ctrl = ul_ca9_clk_select_ctrl - CA9_CLOCK_SELECT_800M;
        ul_ca9_div = CA9_Clock_DIV_1;        
    }

    /* Caculate the ca9 clock div bits mask */
    ul_bit_mask = bit_mask(clk->reg_div.shift, clk->reg_div.size);

    
    /* Read back the CPU_REG_CTRL Register value */
    ul_reg_val = readl(clk->reg_div.reg);

    /* Get ca9 div control value */
    ul_clk_div_ctrl = clk->auc_div_to_ctrl_map[ul_ca9_div];
     
    ul_reg_val &= ~ul_bit_mask;
    ul_reg_val |= (ul_clk_div_ctrl << clk->reg_div.shift);

    if (ul_rate > g_cpll_clock_select[ul_cpll_clk_select_ctrl])
    {
        /*
        ARM CA9 工作频率由CPLL_FOUT(800MHz~1500MHz) 和PLL_DIV_RATIO(CA9_Clock pre-divider) 两个部分决定
        修改AMR CA9 CPU Core 主频可能需要同时修改CPLL_FOUT 和PLL_DIV_RATIO

        比如: AMR CA9 CPU Core 的工作频率从750M (CPLL_FOUT = 1500M, PLL_DIV_RATIO = 2)  ---> 800M (CPLL_FOUT = 800M, PLL_DIV_RATIO = 1) ，
                               
                     必须保证先设置CPLL_FOUT 从1500M --->800M，然后设置PLL_DIV_RATIO 从CPLL_FOUT/2 (2分频) ---> CPLL_FOUT/1 (1分频)
                      
                      否则会引起AMR CA9 CPU Core 的工作频率短暂瞬间为1500M 的情况，瞬间超越最高频率极限而引起CPU Crash        


                      当前需要降低CPLL_FOUT 频率，所以必须保证先设置CPLL_FOUT，再设置PLL_DIV_RATIO 的顺序
        
        */
        
        /* The  parent clock ( cpll fout clock) frequency should be reduced */
       

        /*  设置CPLL_FOUT 频率*/
        /* Set the cpll fout clock frequency  first*/        
        i_ret_value = clk_set_rate(clk->parent,  g_cpll_clock_select[ul_cpll_clk_select_ctrl]);

        if (i_ret_value != S3921_CLK_SUCCESS)
        {
            pr_err("Failed to set  rate: %d\n", i_ret_value);
        }

        /* 
        延时目的是考虑CPLL_FOUT 的设置可能不会立即生效，需要等待一个可靠时间
        当CPLL_FOUT 的设置生效，再设置PLL_DIV_RATIO
        */
        //udelay(10);

        //dmb();
        for (ul_delay_cnt = DELAY_COUNT; ul_delay_cnt > 0; ul_delay_cnt--); 

        /*  设置PLL_DIV_RATIO  分频比率*/        
        /* Set the ca9  div control register */
        writel(ul_reg_val, clk->reg_div.reg);

        //dmb();
        for (ul_delay_cnt = DELAY_COUNT; ul_delay_cnt > 0; ul_delay_cnt--); 
        
    }
    else if (ul_rate < g_cpll_clock_select[ul_cpll_clk_select_ctrl])
    {         

        /*
        当前需要提高CPLL_FOUT 频率，必须保证先设置PLL_DIV_RATIO，再设置CPLL_FOUT 的顺序

        比如: AMR CA9 CPU Core 的工作频率从800M (CPLL_FOUT = 800M, PLL_DIV_RATIO = 1)  ---> 750M (CPLL_FOUT = 1500M, PLL_DIV_RATIO = 2) ，
        
                      必须保证先设置PLL_DIV_RATIO 从CPLL_FOUT/1 (1分频) ---> CPLL_FOUT/2 (2分频)，然后设置CPLL_FOUT 从800M ---> 1500M
                      
                      否则会引起AMR CA9 CPU Core 的工作频率短暂瞬间为1500M 的情况，瞬间超越最高频率极限而引起CPU Crash                
        
        */
        
        /* The  parent clock ( cpll fout clock) frequency should be raised */

        /*  设置PLL_DIV_RATIO  分频比率*/  
        /* Set the ca9  div control register first*/
        writel(ul_reg_val, clk->reg_div.reg);    

        //dmb();
        for (ul_delay_cnt = DELAY_COUNT; ul_delay_cnt > 0; ul_delay_cnt--); 

        /*  设置CPLL_FOUT 频率*/
        /* Set the cpll fout clock frequency */
        i_ret_value = clk_set_rate(clk->parent,  g_cpll_clock_select[ul_cpll_clk_select_ctrl]);
        if (i_ret_value != S3921_CLK_SUCCESS)
        {
            pr_err("Failed to set  rate: %d\n", i_ret_value);
        }
    }
    else
    {       

       /* 当前不需要改变CPLL_FOUT 频率，只需要设置PLL_DIV_RATIO */
        
        /* The  parent clock ( cpll fout clock) not need to change,   only change  the  ca9 div value */

        /*  设置PLL_DIV_RATIO  分频比率*/  
        /* Set the ca9  div control register */
        writel(ul_reg_val, clk->reg_div.reg);

        //dmb();
        for (ul_delay_cnt = DELAY_COUNT; ul_delay_cnt > 0; ul_delay_cnt--); 
    }

    /* Updating the frequency */
    clkevt_freq_update(rate * 1000);

    return i_ret_value;

}


static unsigned long s3921_clk_common_get_rate(struct clk *clk)
{
    unsigned long ul_parent_rate = 0;
    unsigned long ul_rate = 0;    
    unsigned long ul_reg_val = 0;    
    unsigned long ul_div_ctrl = 0;
    unsigned long ul_div_value = 0;
    unsigned long ul_bit_mask = 0;
   
    ul_parent_rate = clk_get_rate(clk->parent) & 0x00FFFFFF;

    /* Read the clock div control register value */
    ul_reg_val = readl(clk->reg_div.reg);

    /* Caculate the clock div bits mask */
    ul_bit_mask = bit_mask(clk->reg_div.shift, clk->reg_div.size);

    /* Get the clock div ctrl  */
    ul_div_ctrl = (ul_reg_val & ul_bit_mask) >> (clk->reg_div.shift);

    if (ul_div_ctrl > clk->uc_div_num)
    {
        ul_div_ctrl = clk->uc_div_num;
    }
    
    /* Get the actual clock div value */
    ul_div_value = clk->auc_ctrl_to_div_map[ul_div_ctrl];


    /* Caculate the clock work frequency */
    ul_rate = ul_parent_rate / ul_div_value;

    /* Save the clock div value in the return value */
    ul_div_value = (ul_div_value << 24) & 0xFF000000;

    ul_rate = (ul_rate & 0x00FFFFFF) | ul_div_value;
                 
    return ul_rate;
}

static unsigned long s3921_clk_common_round_rate(struct clk *clk, unsigned long rate)
{
    unsigned long ul_parent_rate = 0;
    unsigned long ul_clk_round_rate = 0;
    unsigned long clock_frequence_table[MAX_DIV_FREQUENCY_NUM] = {0}; 
    int i_cnt = 0;
    int i = 0;

    /* Get the parent clock frequence*/
    ul_parent_rate = clk_get_rate(clk->parent)  & 0x00FFFFFF;

     if (ul_parent_rate <= rate)
    {
        return ul_parent_rate;
    }


    /* Calculate all the possible clock frequence based on the clock div value*/
    for (i = 0; i < clk->uc_div_num; i++)
    {
        if (0 == clk->auc_ctrl_to_div_map[i])
        {
            continue;
        }
        
        if (0xfff == clk->auc_ctrl_to_div_map[i])
        {
            break;
        }
        
        clock_frequence_table[i_cnt] = ul_parent_rate / clk->auc_ctrl_to_div_map[i];
        i_cnt++;        
    }
    
    /* Get the clock round frequence based on the user setting rate*/
    for (i = (i_cnt - 1); i >= 0 ; i--)
    {
        if (rate <= clock_frequence_table[i])
        {
            ul_clk_round_rate = clock_frequence_table[i];
            break;        
        }
    }

    if (i < 0)
    {
        ul_clk_round_rate = clock_frequence_table[0];
    }
       
    return ul_clk_round_rate;  
}

static int s3921_clk_common_set_rate(struct clk *clk, unsigned long rate)
{
    unsigned long ul_parent_rate = 0;
    unsigned long ul_clk_round_rate = 0;
    unsigned long clock_frequence_table[MAX_DIV_FREQUENCY_NUM] = {0}; 

    unsigned long ul_rate = 0;
    unsigned long ul_reg_val = 0;
    unsigned long ul_clk_div_ctrl = 0;
    
    unsigned long ul_bit_mask = 0;
    int i_ret_value = S3921_CLK_SUCCESS;

    volatile unsigned long ul_delay_cnt = 0;

    int i_cnt = 0;
    int i = 0;

    if ( (rate & 0x80000000) != 0)
    {
        /* If the most significant bit is set, the  rate indicates the  frequency div value*/
        
        /* Get the user setting div value */
        ul_rate = rate & 0x0000000f; 
        
        /* Get div control value */
        ul_clk_div_ctrl = clk->auc_div_to_ctrl_map[ul_rate];
    }
    else
    {
    
        /* Get the parent clock frequence*/
        ul_parent_rate = clk_get_rate(clk->parent)  & 0x00FFFFFF;

        if (ul_parent_rate <= rate)
        {
            return ul_parent_rate;
        }


        /* Calculate all the possible clock frequence based on the clock div value*/
        for (i = 0; i < clk->uc_div_num; i++)
        {
            if (0 == clk->auc_ctrl_to_div_map[i])
            {
                continue;
            }
        
            if (0xfff == clk->auc_ctrl_to_div_map[i])
            {
                break;
            }
        
            clock_frequence_table[i_cnt] = ul_parent_rate / clk->auc_ctrl_to_div_map[i];
            i_cnt++;        
        }

        /* Get the clock round frequence based on the user setting rate*/
        for (i = (i_cnt - 1); i >= 0 ; i--)
        {
            if (rate <= clock_frequence_table[i])
            {
                ul_clk_round_rate = clock_frequence_table[i];
                ul_clk_div_ctrl = i;
                break;        
            }
        }

        if (i < 0)
        {
            ul_clk_round_rate = clock_frequence_table[0];
            ul_clk_div_ctrl = 0;
        }
    }
    
    /* Caculate the clock div bits mask */
    ul_bit_mask = bit_mask(clk->reg_div.shift, clk->reg_div.size);

    /* Read back the div control register value */
    ul_reg_val = readl(clk->reg_div.reg);
     
    ul_reg_val &= ~ul_bit_mask;
    ul_reg_val |= (ul_clk_div_ctrl << clk->reg_div.shift);

    /* Set the ca9  div control register first*/
    writel(ul_reg_val, clk->reg_div.reg);                       
   
    //dmb();
    for (ul_delay_cnt = DELAY_COUNT; ul_delay_cnt > 0; ul_delay_cnt--); 

    return i_ret_value;

}


/*
static struct clk clk_arm = {
	.name		= "armclk",
	.parent		= &clk_mout_apll.clk,
	.ops		= &(struct clk_ops) {
		.get_rate	= s3c64xx_clk_arm_get_rate,
		.set_rate	= s3c64xx_clk_arm_set_rate,
		.round_rate	= s3c64xx_clk_arm_round_rate,
	},
};

*/

/* Define the s3921 clock initial list */

static struct clk *clks[] __initdata = {
	&clk_fout_cpll,
	&clk_arm_ca9,
	&clk_ca9_peripheral,
	&clk_pl310_controller,
	&clk_l2_tag_ram,
	&clk_l2_data_ram,
	&clk_l2_axi_master
};


void s3921_setup_clocks(void)
{
    (void)clk_set_rate(&clk_arm_ca9,  g_ca9_clock_select[CA9_CLOCK_SELECT_1000M]);
    return;
}



/**
 * s3921_register_clocks - register clocks for s3921
 * @xtal: The rate for the clock crystal feeding the PLLs.
 * @armclk_divlimit: Divisor mask for ARMCLK.
 *
 * Register the clocks for the S3C6400 and S3C6410 SoC range, such
 * as ARMCLK as well as the necessary parent clocks.
 *
 * This call does not setup the clocks, which is left to the
 * s3921_setup_clocks() call which may be needed by the cpufreq
 * or resume code to re-set the clocks if the bootloader has changed
 * them.
 */
void __init s3921_register_clocks(void)
{
        
	ali_s39XX_register_clocks(clks, ARRAY_SIZE(clks));	
}

