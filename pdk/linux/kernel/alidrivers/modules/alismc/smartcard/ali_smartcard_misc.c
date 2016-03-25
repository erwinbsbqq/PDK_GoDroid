/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_misc.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader management.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#include "ali_smartcard_misc.h"

/* 
 *  Normally, we have the address like b8******
 *  When do the I/O operation, we need to remap the address
 *  The remapped address should be like 18******
 */

/* Macros definitions */
#define SMC_MISC_INVERT_CTRL 0x1800001d

/* Different IC rst pin */
static UINT32 rst_msk_group_1[2] = {1 << 23, 1 << 24};
static UINT32 rst_msk_group_2[2] = {1 << 15, 1 << 28};
static UINT32 rst_msk_group_3[2] = {1 << 20, 1 << 21};
static UINT32 rst_msk_group_4[2] = {1 << 15, 1 << 15};

/* Strip pin & PIN MUX for 3603 demo board*/
#define SMC_PIN_MUX_ADDR    0x18000088  
#define SMC_STRIP_PIN_BIT   	 73
#define SMC_PIN_MUX_BIT     	 14
#define SMC_CA1_POWER_ENABLE     54
#define SMC_CA2_POWER_ENABLE     51

/* Power management */
void smc_misc_power_enable(void)
{
	#if 0
	ali_gpio_direction_output(SMC_CA1_POWER_ENABLE, 0);	
	ali_gpio_direction_output(SMC_CA2_POWER_ENABLE, 0);	
	#endif
}

void smc_misc_power_disable(void)
{
	#if 0
	ali_gpio_direction_output(SMC_CA1_POWER_ENABLE, 1);	
	ali_gpio_direction_output(SMC_CA2_POWER_ENABLE, 1);	 
	#endif
}

/* Set Strip Pin & Pin MUX */
void smc_misc_set_pin(void)
{
	#if 0
    void __iomem *p = NULL;

    ali_gpio_direction_output(SMC_STRIP_PIN_BIT, 1);
    p = ioremap(SMC_PIN_MUX_ADDR, sizeof(UINT32));
    writel(readl(p) | (1 << SMC_PIN_MUX_BIT), p);
    iounmap(p);
    #endif
}

/* UnSet Strip Pin & Pin MUX */
void smc_misc_unset_pin(void)
{
	#if 0
    void __iomem *p = NULL;

    ali_gpio_direction_output(SMC_STRIP_PIN_BIT, 0);
    /* This will cause the CA light on */
    /* But we have to unset the PIN, or the flash won't work */ 
    p = ioremap(SMC_PIN_MUX_ADDR, sizeof(UINT32));
    writel(readl(p) & ~(1 << SMC_PIN_MUX_BIT), p);
    iounmap(p);
    #endif
}

/* Check config param address */
int smc_config_param_check(UINT32 config_param)
{	
    if(0x80000000 != (((UINT32)config_param) & 0xf0000000) &&
		0xa0000000 != (((UINT32)config_param) & 0xf0000000) &&
		0xb0000000 != (((UINT32)config_param) & 0xf0000000))
		return -EFAULT;

    return 0;
}

int smc_misc_warm_reset(struct smc_device *dev)
{	
    void __iomem *p = (void __iomem *)dev->io_base;
    UINT32 i;

    writeb(readb(p + REG_ICCR) & (~SMC_RB_ICCR_RST), p + REG_ICCR);
    for (i = 0; i < 11; i++) msleep(1);    

    smc_debug(KERN_INFO "SMC MISC: In %s: over\n", __func__);
	return 0;
}

/* Deactive */
void smc_misc_dev_deactive(struct smc_device *dev)
{	
    struct smartcard_private *tp = NULL;     
    void __iomem *p = NULL;    
    int i;


    BUG_ON(NULL == dev);    
    BUG_ON(NULL == dev->priv);   
    BUG_ON(NULL == (void *)dev->io_base);  

    tp = (struct smartcard_private *)dev->priv;   
    p = (void __iomem *)dev->io_base;        

    /* ?? */
    smc_debug("SMC MISC: In %s REG_ICCR %d %p\n", 
                  __FUNCTION__, readb(p + REG_ICCR), p);    
                  
    writeb(readb(p + REG_ICCR) & (~SMC_RB_ICCR_RST), p + REG_ICCR);    
    udelay(DATV_RST2CLK);
    writeb(readb(p + REG_ICCR) & (~SMC_RB_ICCR_CLK), p + REG_ICCR);        
    udelay(DATV_CLK2IO);
    writeb(readb(p + REG_ICCR) & (~SMC_RB_ICCR_DIO), p + REG_ICCR);    
    udelay(DATV_IO2VCC >> 1);
    
    smc_debug("SMC MISC: In %s REG_ICCR %d %p\n", 
                  __FUNCTION__, readb(p + REG_ICCR), p);    

    /* ?? */
    if (tp->use_gpio_vpp)
	{	
		if (tp->internal_ctrl_vpp)
            writeb(readb(p + REG_CLK_VPP) & (~SMC_RB_CTRL_VPP), p + REG_CLK_VPP);
		else
            ali_gpio_set_value(tp->gpio_vpp_pos, !(tp->gpio_vpp_pol));
		smc_debug(KERN_INFO "SMC MISC: In %s SMC VPP Low! ", __func__);
	}
    udelay(DATV_IO2VCC>>1);

    writeb(readb(p + REG_ICCR) | SMC_RB_ICCR_VCC, p + REG_ICCR);
    for (i = 0; i < 11; i++) udelay(1000);    
	tp->reseted = 0;    

    smc_debug(KERN_INFO "SMC MISC: In %s over\n", __func__);
}

/* Initialize HW */
static inline void smc_misc_reset_config(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    void __iomem *p = NULL;
    

    dev->rst_msk = 0;
    dev->rst_addr = SMC_DEV_RST_ADDR_0;
    
    if (SYS_IC_BONDING_TYPE_6(tp))
	{
		dev->rst_msk = rst_msk_group_1[dev->dev_id];
		dev->rst_addr = SMC_DEV_RST_ADDR_1;
	}
    
	if (sys_ic_is_M3202()==1)
	{
		dev->rst_msk = rst_msk_group_2[dev->dev_id];
		dev->rst_addr = SMC_DEV_RST_ADDR_1;
	}
    
    if (SYS_IC_BONDING_TYPE_13(tp))
	{
		dev->rst_msk = rst_msk_group_3[dev->dev_id];
		dev->rst_addr = SMC_DEV_RST_ADDR_2;
	}
    
	if (SYS_IC_BONDING_TYPE_14(tp))
	{
		dev->rst_msk = rst_msk_group_4[dev->dev_id];
		dev->rst_addr = SMC_DEV_RST_ADDR_1;
	}

    /* This address must be 0x18****** likely */
    dev->rst_addr &= 0x0fffffff;
    dev->rst_addr |= 0x10000000;
    
    p = ioremap(dev->rst_addr, sizeof(UINT32));   	
    writel(readl(p) | dev->rst_msk, p);           	
	udelay(3);    	
	writel(readl(p) & ~(dev->rst_msk), p);	
	
    iounmap(p);
}

static inline void smc_misc_init_ctrl(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    void __iomem *p = (void __iomem *)dev->io_base;
    
    if (tp->ts_auto_detect)
	{
        /* Enable TS auto detect */
        writeb(0x10 | (tp->invert_detect<<5) | (tp->invert_power<<6), p + REG_DEV_CTRL);
	}

    /* Enable SCR interface */
    writeb(0x00, p + REG_SCR_CTRL);
    writeb(0x80 | (tp->parity_odd<<4), p + REG_SCR_CTRL);

	if (tp->apd_disable)
	{
        writeb(0x1 | (tp->parity_disable<<5) | (tp->apd_disable<<4), p + REG_ICCR);
    }
	else // Power off
	{
	    writeb(0x41 | (tp->parity_disable<<5) | (tp->apd_disable<<4), p + REG_ICCR);
	}

	
	if(tp->open_drain_supported)
	{		
		UINT8 temp_val =  readb(p + REG_CLK_VPP);
		temp_val &= 0x9f;			
		temp_val |= (tp->en_power_open_drain<<6);				
		temp_val |= ((tp->en_clk_open_drain|tp->en_data_open_drain|tp->en_rst_open_drain)<<5);
		writeb(temp_val, p + REG_CLK_VPP);			
	}
    
 	if (tp->use_gpio_vpp && tp->internal_ctrl_vpp)
 	{
 		UINT8 temp_val = (readb(p + REG_CLK_VPP) & 0xf3) | (tp->gpio_vpp_pol<<3);
 		if(SYS_IC_BONDING_TYPE_7(tp))
		{
    		/*
    		 * S3602f SmartCard interface auto disable clock function has problem 
    		 * while meet the smart card without parity bit such as Irdeto.
    		 * need disable this function for s3602f
    		 */
			temp_val |= 0x10;
		}
        writeb(temp_val, p + REG_CLK_VPP);
        writeb((tp->gpio_vpp_pos&0x3f) | 0x80, p + REG_VPP_GPIO);
 	}


    p = ioremap(SMC_MISC_INVERT_CTRL, 1);   
    #if 0
	if (SYS_IC_BONDING_TYPE_15(tp))
	{
        writeb(readb(p) | tp->invert_power<<7, p);
        writeb(readb(p) | tp->invert_detect <<6, p);
	}
	#endif
    iounmap(p);
}

/*  Emulation of the float operation
 *  @ It won't return a float value, but a closed value
      which is the scale multiple of the real result
 *  \param dividend:
 *  \param divisor:
 *  \param scale: use to close the real result 
 *  \return value: the closed value
 */
static inline unsigned int smc_misc_fdiv_emu(unsigned int dividend,
                                          unsigned int divisor,
                                          unsigned int scale)
{
    unsigned int quotient = 0;
    unsigned int remainder = 0;

    quotient = dividend / divisor;
    remainder = dividend % divisor;

    return (quotient * scale + remainder * scale / divisor);
}

void smc_misc_set_wclk(struct smc_device *dev, UINT32 clk)
{    
	UINT32 scr_div_inte, pwm_div_inte;
	UINT32 scr_div_fract, pwm_div_fract;
    UINT32 scale = 100;
    
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    void __iomem *p = (void __iomem *)dev->io_base;
    void __iomem *p_pwm_base = ioremap(dev->pwm_addr, 0x20);

    /* Note: due to the restrict of the kernel stack, we
     *       can't do float operation in kernel modules
     *       So we use a special way to replace the float operation
     */
	scr_div_inte = smc_misc_fdiv_emu(tp->scr_sys_clk, clk, scale);
	scr_div_fract = scr_div_inte % scale;
	scr_div_inte = scr_div_inte / scale;

    if (SYS_IC_BONDING_TYPE_6(tp))
	{
        writeb(0, p_pwm_base + dev->pwm_gpio_ofst);	
	}
    
	if (scr_div_fract)
	{
		UINT8 fract;
        
		pwm_div_inte = smc_misc_fdiv_emu(tp->pwm_sys_clk, clk, scale);
		pwm_div_fract = pwm_div_inte % scale;
		pwm_div_inte = pwm_div_inte / scale;
		fract = scale / pwm_div_fract;		

        writeb(scr_div_inte>>1, p + REG_CLKH_SEH);
        writeb((scr_div_inte>>1) + (scr_div_inte & 0x1), p + REG_CLKL_SEL);

        writew(pwm_div_inte>>1, p_pwm_base + dev->pwm_seh_ofst);
        writew((pwm_div_inte>>1) + (pwm_div_inte & 0x1), p_pwm_base + dev->pwm_sel_ofst);

        writeb(fract, p_pwm_base + dev->pwm_frac_ofst);
		writeb(0x81, p_pwm_base + dev->pwm_cfg_ofst);
        
		if (SYS_IC_BONDING_TYPE_16(tp)) writeb(fract, p + REG_CLK_FRAC); 	
	}
	else
	{		
		writeb(0, p_pwm_base + dev->pwm_cfg_ofst);	
		writeb(scr_div_inte>>1, p + REG_CLKH_SEH); 	
		writeb((scr_div_inte>>1) + (scr_div_inte&0x1), p + REG_CLKL_SEL);		
	}	

    iounmap(p_pwm_base);
}

static inline void smc_misc_set_clock(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    tp->smc_clock = DFT_WORK_CLK;     /* Set default clk, it's necessary. */
	tp->smc_clock = tp->init_clk_array[0];

    /* We should unmap the mapped I/O space before use the dev */
    smc_misc_set_wclk(dev, tp->smc_clock);
}

static inline void smc_misc_set_etu_default(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    void __iomem *p = (void __iomem *)dev->io_base;
    
    if(tp->use_default_etu)
		tp->smc_etu = tp->default_etu;
	else
		tp->smc_etu = DFT_WORK_ETU;

    writew(tp->smc_etu, p + REG_ETU0);
}

static inline void smc_misc_enable_interrupt(struct smc_device *dev)
{    
    void __iomem *p = (void __iomem *)dev->io_base;
    

    writeb(0x7f, p + REG_IER0);  //enable receive interrupt, enable wait timeout interrupt
    writeb(0xff, p + REG_IER1);  //detect Card inserting or removal interrupt enable

    /* 
     * Delete to fixed bug 49283: "incorrect card" 
     * can't disappear when standby/resume
     */
    /*
	while (tp->invert_detect)
	{
		if (readb(p + REG_ISR1) & (SMC_ISR1_CARD_REMOVE | SMC_ISR1_CARD_INSERT))
		{	
			smc_debug(KERN_INFO "SMC MISC: In %s Status i %d: %02x, isr0: %02x, isr1: %02x\n", 
                          __func__, i, 
                          readb(p + REG_DEV_CTRL),
                          readb(p + REG_ISR0), 
                          readb(p + REG_ISR1));
			writeb(SMC_ISR1_CARD_REMOVE | SMC_ISR1_CARD_INSERT, p + REG_ISR1);
			break;
		}
		udelay(1);
		i++;
		if(i > 2000)
			break;
	}
	*/
	writeb(15, p + REG_PDBR);		    //set de-bounce to 15
	writew(12, p + REG_GTR0);           //set gtr to 12
	writel(12800, p + REG_CBWTR0);      //set wt to 9600
	writeb(0x49, p + REG_RCVPR);		//set value1_r to 4 and glitch_v to 3
	writeb(0x71, p + REG_RXTX_PP);	    //set rxpp to 3 and txpp to 3
}

void smc_misc_init_hw(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    /* This should be done every time ? Or just when booting ? */
    smc_misc_reset_config(dev);
    smc_misc_init_ctrl(dev);
    smc_misc_enable_interrupt(dev);
    smc_misc_set_etu_default(dev);
    smc_misc_set_clock(dev);

    tp->inverse_convention = 0;
}

/*  Check if smart card inserted
 *  \param dev
 *  \return value: 1, inserted; 0 no smart card
 */
int smc_misc_is_card_insert(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    short status = 0;

    void __iomem *p = (void __iomem *)dev->io_base;

    status = readb(p + REG_ICCSR);
	
	smc_debug(KERN_INFO "SMC MISC: In %s Check card status 0x80 : 0x%x\n", __func__, status);
	if (0 != (status & 0x80))
	{
		smc_debug(KERN_INFO "SMC MISC: In %s Card inserted\n", __func__);
		tp->inserted = 1;
		writeb(SMC_ISR1_CARD_INSERT, p + REG_ISR1);
		/* TODO: Notify the application of card status to show UI */
	}

    return tp->inserted;
}

int smc_misc_register_irq_server(struct smc_device *dev)
{
    int ret = 0;

    smc_debug(KERN_INFO "SMC MISC: In %s Request irq %d\n", 
                  __func__, dev->irq);
    ret = request_irq(dev->irq, smc_irq_dev_interrupt, 
                      0, dev->dev_name, dev);
    if (ret)
    {
        smc_debug(KERN_INFO "SMC MISC: In %s Can't request irq %d\n", 
                      __func__, dev->irq);
        return ret;
    }

    return 0;
}

void smc_misc_unregister_irq_server(struct smc_device *dev)
{    
    smc_debug("SMC MISC: In %s Release irq %d\n", 
                  __func__, dev->irq);
    free_irq(dev->irq, dev);
}

/* Smart card status: 2 inserted; 0 not inserted */
int smc_misc_card_exist(struct smc_device *dev)
{    
	struct smartcard_private *tp = NULL;

	BUG_ON(NULL == dev);
    BUG_ON(NULL == dev->priv);

    tp = (struct smartcard_private *)dev->priv;

	if (tp->inserted)
		return 0x02;
	else
		return 0x00;
}

/* Check card status then set cooresponding flag */
void smc_misc_set_card_flags(struct smc_device *dev, unsigned long flags)
{
	if (NULL == dev)
		return;
    
	if (flags & SMC_INSERTED)
		dev->wq_flags &= ~SMC_REMOVED;    
	else if (flags & SMC_REMOVED)
		dev->wq_flags &= ~SMC_INSERTED; 
	dev->wq_flags |= flags;

    if (waitqueue_active(&dev->smc_wq_head))
        wake_up(&dev->smc_wq_head);
} 

/* Check card status then unset cooresponding flag */
void smc_misc_unset_card_flags(struct smc_device *dev, unsigned long flags)
{
	if (NULL == dev)
		return;
    
	dev->wq_flags &= ~flags;

    if (waitqueue_active(&dev->smc_wq_head))
    {
        smc_debug(KERN_INFO "SMC MISC: In %s wake up queue\n",__func__);
        wake_up(&dev->smc_wq_head);
    }
} 

/* Config PPS for Smart card */
INT32 smc_misc_set_pps(struct smc_device *dev, void __iomem *p,
                           UINT8 PPS0, UINT8 FI, UINT8 DI)
{
	UINT8 a_pps_buf[4], a_pps_echo[4];
	INT32 ret = -EINVAL;
	UINT16 i_act_size = 0, i_rw_len = 0;
    
    
	a_pps_buf[0] = 0xff;
	a_pps_buf[1] = PPS0;
	if (PPS0 & 0x10)
	{
		a_pps_buf[2] = ((FI & 0xf)<<4) | (DI & 0xf);
		a_pps_buf[3] = a_pps_buf[0]^a_pps_buf[1]^a_pps_buf[2];
		smc_debug(KERN_INFO "SMC MISC: In %s ppss %02x pps0 %02x pps1 %02x pck %02x\n", 
                      __func__, a_pps_buf[0], a_pps_buf[1], a_pps_buf[2], a_pps_buf[3]);
        i_rw_len = 4;
	}
	else
	{
		a_pps_buf[2] = a_pps_buf[0]^a_pps_buf[1];
		smc_debug(KERN_INFO "SMC MISC: In %s ppss %02x pps0 %02x pck %02x\n", 
                      __func__, a_pps_buf[0], a_pps_buf[1], a_pps_buf[2]);
        i_rw_len = 3;
	}
		
	memset(a_pps_echo, 0x5a, 4);
    smc_debug(KERN_INFO "SMC MISC: In %s PPS buffer: %02x %02x %02x %02x %d\n", 
                  __func__, a_pps_buf[0], a_pps_buf[1], a_pps_buf[2], a_pps_buf[3], i_rw_len);
    
	smc_txrx_transfer_data(dev, p, a_pps_buf, i_rw_len, a_pps_echo, i_rw_len, &i_act_size);

	smc_debug(KERN_INFO "SMC MISC: In %s PPS echo: %02x %02x %02x %02x\n", 
                  __func__, a_pps_echo[0], a_pps_echo[1], a_pps_echo[2], a_pps_echo[3]);
	if (i_act_size && a_pps_buf[0] == a_pps_echo[0] && 
        (a_pps_buf[1] & 0xf) == (a_pps_echo[1] & 0xf))
	{
		if (PPS0 & 0x10)
		{
			if ((a_pps_buf[1] & 0x10) == (a_pps_echo[1] & 0x10))
			{
				ret = 0;
				smc_debug(KERN_INFO "SMC MISC: In %s PPS SUCCESS!\n", __func__);
			}
		}
		else
		{
			ret = 0;
			smc_debug(KERN_INFO "SMC MISC: In %s PPS SUCCESS!\n", __func__);
		}
	}
	
	return ret;
}

/* Set new ETU */
void smc_misc_set_etu(struct smc_device *dev, unsigned long etu)
{
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	void __iomem *p = (void __iomem *)dev->io_base;
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_smc_atr = (atr_t *)atr->atr_info;
	UINT8 FI = 1;
	UINT8 DI = 1;

	if (p_smc_atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		FI = (p_smc_atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0)>>4;
		DI = (p_smc_atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F); 
	}
	if (tp->reseted)
	{
		if (0 == tp->T)
		{
			UINT8 WI = ATR_DEFAULT_WI;
				
			if (p_smc_atr->ib[2][ATR_INTERFACE_BYTE_TC].present)
			    WI = p_smc_atr->ib[2][ATR_INTERFACE_BYTE_TC].value;
			
			writel(960 * WI * DI, p + REG_CBWTR0);
		}
		else if (1 == tp->T)
		{
			UINT8 BWI = tp->BWI; 
			writel(11 + ((960 * 372 * (1<<BWI) * DI) / FI), p + REG_CBWTR0);
		}
		tp->smc_etu = etu;
		writew(tp->smc_etu, p + REG_ETU0);	
	}	
}



