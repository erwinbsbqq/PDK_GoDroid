#include <common.h>
#include <asm/arch/sys_proto.h>

/* Define the return  SUCCESS/FAIL code */
#define S3921_CLK_SUCCESS              0
#define S3921_CLK_FAIL                      1

#define PHYS_SYSTEM		(0x18000000)	// ALi Soc System IO 		0x1800_0000 - 0x1800_0FFF (4K)

/*ALI S3921 clock registers address macro definition*/
#define S3921_CLKREG(x)                     (PHYS_SYSTEM + (x))


#define S3921_STRAP_INFO_CLOCK      S3921_CLKREG(0x70)
#define S3921_STRAP_CTRL_CLOCK      S3921_CLKREG(0x74)
#define S3921_CPU_CTRL_CLOCK          S3921_CLKREG(0x94)
#define S3921_PLL_CTRL_CLOCK           S3921_CLKREG(0x94)
#define S3921_CA9_CTRL_CLOCK           S3921_CLKREG(0x0C)




#define NULL 0


#define CA9_Clock_DIV_1                 1
#define CA9_Clock_DIV_2                 2
#define CA9_Clock_DIV_4                 4
#define CA9_Clock_BYPASS_CLK     3

/* Strap Pin control register  shift bits definition for setting function  */
#define BYPASS_EN_TRIG              (0x1 << 26)
#define CPLL_FOUTSEL_TRIG        (0x1 << 22)
#define BYPASS_EN                        (0x1 << 14)
#define CPLL_FOUTSEL_800M       (0x0 << 8)
#define CPLL_FOUTSEL_900M       (0x1 << 8)
#define CPLL_FOUTSEL_1000M     (0x2 << 8)
#define CPLL_FOUTSEL_1100M     (0x3 << 8)
#define CPLL_FOUTSEL_1200M     (0x4 << 8)
#define CPLL_FOUTSEL_1300M     (0x5 << 8)
#define CPLL_FOUTSEL_1400M     (0x6 << 8)
#define CPLL_FOUTSEL_1500M     (0x7 << 8)

#define MAX_DIV_FREQUENCY_NUM 16

/* Define the cpll  output frequency selection enum */
enum CPLL_FOUT_SELECT
{
     CPLL_FOUT_SELECT_BEGIN =0,
    CPLL_FOUTSEL_SELECT_800M  = CPLL_FOUT_SELECT_BEGIN,   
    CPLL_FOUTSEL_SELECT_900M,                                                    
    CPLL_FOUTSEL_SELECT_1000M,      
    CPLL_FOUTSEL_SELECT_1100M,         
    CPLL_FOUTSEL_SELECT_1200M,                                                 
    CPLL_FOUTSEL_SELECT_1300M,
    CPLL_FOUTSEL_SELECT_1400M,
    CPLL_FOUTSEL_SELECT_1500M,
    CPLL_FOUT_SELECT_END
};


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

};




/* Get the CPLL clock output frequence,  CPLL is clock source in the CPU system */
static unsigned long s3921_cpll_fout_get_rate(struct clk *clk);

/* Get the cpll fout round frequence value according to the user setting  rate */
static unsigned long s3921_cpll_fout_round_rate(struct clk *clk, unsigned long rate);

/* Set the cpll fout frequency based on the user setting rate */
static int s3921_cpll_fout_set_rate(struct clk *clk, unsigned long rate);

/* Get the ARM CA9  core working clock frequence */
extern unsigned long s3921_clk_ca9_get_rate(struct clk *clk);

/* Get the ARM CA9  core  frequence value according to the user setting  rate */
extern unsigned long s3921_clk_ca9_round_rate(struct clk *clk, unsigned long rate);

/* Set the ARM ca9 working frequency based on the user setting rate */
extern int s3921_clk_ca9_set_rate(struct clk *clk, unsigned long rate);


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




#define clk_fin_cpll clk_ext_xtal_mux




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



/* Define the cpll output frequency mapping table, unit : kHz */
static const unsigned long g_cpll_clock_select[CPLL_FOUT_SELECT_END] = {800000, 900000, 1000000, 1100000, 1200000, 1300000, 1400000, 1500000};

/* Define the CA9 Clock real work Frequency mapping table, unit : kHz */
static const unsigned long g_ca9_clock_select[CA9_CLOCK_SELECT_END] = 
{
    200000, 225000, 250000, 275000, 300000, 325000, 350000, 375000,
    400000, 450000, 500000, 550000, 600000, 650000, 700000, 750000,
    800000, 900000, 1000000, 1100000, 1200000, 1300000, 1400000, 1500000
};



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

    if (ul_clk_select > CPLL_FOUTSEL_SELECT_1500M)
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

    //printk(KERN_EMERG"\n2 s3921_cpll_fout_set_rate : reg addr = 0x%x,  reg val = 0x%x\n", clk->reg_div.reg, ul_reg_val);

    /* Set the the Strap Pin Control Register with the user setting rate  */
    writel(ul_reg_val, clk->reg_div.reg);

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
unsigned long s3921_clk_ca9_get_rate(struct clk *clk)
{
    unsigned long ul_rate = 0;
    unsigned long ul_reg_val = 0;    
    unsigned long ul_pll_div_ratio = 0;
    unsigned long ul_pll_div_value = 0;
    unsigned long ul_bit_mask = 0;
   
    //ul_rate = clk_get_rate(clk->parent);

    ul_rate = s3921_cpll_fout_get_rate(clk->parent);

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
*       @*clk   Pointer to the cpll fout clock object
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
unsigned long s3921_clk_ca9_round_rate(struct clk *clk, unsigned long rate)
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
int s3921_clk_ca9_set_rate(struct clk *clk, unsigned long rate)
{
    unsigned long ul_clk_setting_rate = g_ca9_clock_select[CA9_CLOCK_SELECT_1000M];
    unsigned long ul_ca9_clk_select_ctrl = CA9_CLOCK_SELECT_1000M;
    unsigned long ul_cpll_clk_select_ctrl = CPLL_FOUTSEL_SELECT_1000M;

    unsigned long ul_ca9_div = 0;
    unsigned long ul_rate = 0;

    unsigned long ul_reg_val = 0;
    unsigned long ul_clk_div_ctrl = 0;
    
    unsigned long ul_bit_mask = 0;

    int i_ret_value = S3921_CLK_SUCCESS;
    int i = 0;

    /* Get the cpll fout clock frequency */
    //ul_rate = clk_get_rate(clk->parent);

    ul_rate = s3921_cpll_fout_get_rate(clk->parent);

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
        /* The  parent clock ( cpll fout clock) frequency should be reduced */
       
        //printk(KERN_EMERG"\n1 cpll clock : %dkHz,  clk div:  %d\n", g_cpll_clock_select[ul_cpll_clk_select_ctrl], ul_clk_div_ctrl);

         /* Set the cpll fout clock frequency  first*/        
        //i_ret_value = clk_set_rate(clk->parent,  g_cpll_clock_select[ul_cpll_clk_select_ctrl]);

        i_ret_value = s3921_cpll_fout_set_rate(clk->parent,  g_cpll_clock_select[ul_cpll_clk_select_ctrl]);

        //printk(KERN_EMERG"\n2 cpll clock : %dkHz,  clk div:  %d\n", g_cpll_clock_select[ul_cpll_clk_select_ctrl], ul_clk_div_ctrl);

        if (i_ret_value != S3921_CLK_SUCCESS)
        {
            printf("Failed to set  rate: %d\n", i_ret_value);
        }

        udelay(10);
        
        /* Set the ca9  div control register */
        writel(ul_reg_val, clk->reg_div.reg);        
    }
    else if (ul_rate < g_cpll_clock_select[ul_cpll_clk_select_ctrl])
    {         
        /* The  parent clock ( cpll fout clock) frequency should be raised */

        /* Set the ca9  div control register first*/
        writel(ul_reg_val, clk->reg_div.reg);        
        
        /* Set the cpll fout clock frequency */
        //i_ret_value = clk_set_rate(clk->parent,  g_cpll_clock_select[ul_cpll_clk_select_ctrl]);
        i_ret_value = s3921_cpll_fout_set_rate(clk->parent,  g_cpll_clock_select[ul_cpll_clk_select_ctrl]);
        
        
        if (i_ret_value != S3921_CLK_SUCCESS)
        {
            printf("Failed to set  rate: %d\n", i_ret_value);
        }
    }
    else
    {       
        /* The  parent clock ( cpll fout clock) not need to change,   only change  the  ca9 div value */

        /* Set the ca9  div control register */
        writel(ul_reg_val, clk->reg_div.reg);                       
    }

    /* Updating the frequency */
    //clkevt_freq_update(rate * 1000);

    return i_ret_value;

}


int SetCPUFreq(enum CA9_CLOCK_SELECT val)
{
    unsigned long rate = g_ca9_clock_select[val];
    s3921_clk_ca9_set_rate(&clk_arm_ca9, rate);
}
     
