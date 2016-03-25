/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_atr.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader attribute.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/
#include <linux/slab.h>

#include "ali_smartcard_atr.h"

/* Smart card config */
#define FORCE_TX_RX_THLD		2
#define FIRST_CWT_VAL		    1000  //ms
#define CWT_VAL			        300 //ms

/* To test the ATR */
//#define SMC_ATR_TEST
#ifdef SMC_ATR_TEST  
#define SMC_ATR_PRESET_TEST_BUF(atr, c, p, i) \
UINT8 atr_test_str[] = { \
	0X3B, 0XDA, 0X18, 0XFF, 0X81, 0XB1, 0XFE, \
	0X75, 0X1F, 0X03, 0X00, 0X31, 0XC5, 0X73, \
	0XC0, 0X01, 0X40, 0X00, 0X90, 0X00, 0X0C}; \
	atr->atr_size = c = sizeof(atr_test_str); \
	for (i = 0; i < c; i++) \
	{ \
		atr->atr_buf[i] = atr_test_str[i]; \
	}
#else
#define SMC_ATR_PRESET_TEST_BUF(atr, c, p, i)
#endif

/* Standard Macros */
#define F_RFU	0
#define D_RFU	0
#define I_RFU	0

#define ATR_ID_NUM		16
#define ATR_FI_NUM		16
#define ATR_DI_NUM		16
#define ATR_I_NUM		4

static UINT32 atr_num_ib_table[ATR_ID_NUM] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
static UINT32 atr_f_table[ATR_FI_NUM] = {372, 372, 558, 744, 1116, 1488, 1860, F_RFU, F_RFU, \
                                         512, 768, 1024, 1536, 2048, F_RFU, F_RFU};
static UINT32 atr_d_table[ATR_DI_NUM] = {D_RFU, 1, 2, 4, 8, 16, 32, D_RFU, 12, 20, D_RFU, D_RFU, \
                                         D_RFU, D_RFU, D_RFU, D_RFU};
static UINT32 atr_i_table[ATR_I_NUM] = {25, 50, 100, 0};

/* Get card ATR */
static inline void smc_atr_set_register_group_one(struct smartcard_private *tp, void __iomem *p)
{
    writeb(SMC_FIFO_CTRL_EN | SMC_FIFO_CTRL_TX_OP | SMC_FIFO_CTRL_RX_OP, p + REG_FIFO_CTRL);
                                         
	smc_write_rx(tp, p, ATR_MAX_SIZE);
	
	/* clear interrupt status ---- how ?? */
    writeb(readb(p + REG_ISR1), p + REG_ISR1);
    writeb(readb(p + REG_ISR0), p + REG_ISR0);
}

static inline void smc_atr_set_register_group_two(struct smartcard_private *tp, void __iomem *p)
{
    if (tp->inverse_convention)
	{
		smc_debug("SMC ATR: In %s Iverse card, Dis apd\n", __func__);
        writeb((readb(p + REG_ICCR) & (~0x30)) | SMC_RB_ICCR_AUTO_PRT, p + REG_ICCR);
	}
	else
	{
		smc_debug("SMC ATR: In %s Directed card, Parity %d, Apd %d\n", 
                      __func__, !tp->parity_disable, !tp->apd_disable);
        writeb(readb(p + REG_ICCR) & 0xcf, p + REG_ICCR);
        writeb(readb(p + REG_ICCR) | (tp->parity_disable<<5) | \
               (tp->apd_disable<<4), p + REG_ICCR);
	}
}

static inline void smc_atr_set_register_group_three(struct smartcard_private *tp, void __iomem *p)
{
    if (tp->inverse_convention)
	{
		smc_debug("SMC ATR: In %s Iverse card, Dis apd\n", __func__);
		if (tp->apd_disable)
            writeb(SMC_RB_ICCR_AUTO_PRT, p + REG_ICCR);
		else	
            writeb(SMC_RB_ICCR_AUTO_PRT | 0x40, p + REG_ICCR);
	}
	else
	{
		smc_debug("SMC ATR: In %s Directed card, Parity %d, Apd %d\n", 
                      __func__, !tp->parity_disable, !tp->apd_disable);
		if (tp->apd_disable)
            writeb((tp->parity_disable<<5) | (tp->apd_disable<<4), p + REG_ICCR);
		else	
            writeb((tp->parity_disable<<5) | (tp->apd_disable<<4) | 0x40, p + REG_ICCR);
	}
	msleep((ATV_VCC2IO>>1) / 1000);
    
	if (tp->use_gpio_vpp)
	{
		if (tp->internal_ctrl_vpp)
            writeb(readb(p + REG_CLK_VPP) | SMC_RB_CTRL_VPP, p + REG_CLK_VPP);
		else
			ali_gpio_set_value(tp->gpio_vpp_pos, tp->gpio_vpp_pol);
	}
	msleep((ATV_VCC2IO>>1) / 1000);

    writeb(readb(p + REG_ICCR) | SMC_RB_ICCR_DIO, p + REG_ICCR);
}

static inline void smc_atr_set_register_group_four(struct smartcard_private *tp, void __iomem *p)
{
    if (!tp->warm_reset)
	{
		udelay(ATV_IO2CLK);
		writeb(readb(p + REG_ICCR) | SMC_RB_ICCR_CLK, p + REG_ICCR);
		msleep(1);
	}
}

static inline void smc_atr_set_register_special_card_check(struct smartcard_private *tp, void __iomem *p)
{
    /* Basically, we should not disable parity check while get ATR */ 
    /* but for irdeto cards, we have to do so ---- why ?? */
    /* What means of magic number 550 ?? */		
	if ((1 == tp->parity_disable) || (readw(p + REG_ETU0) > 550))
		writeb(readb(p + REG_ICCR) | 0x20, p + REG_ICCR);
}

static inline int smc_atr_isr1_low_trigger(struct smc_device *dev, void __iomem *p)
{    
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;


    smc_debug("SMC ATR: In %s Low trigger!\n", __func__);
    
	tp->isr1_interrupt_status &= (~SMC_ISR1_RST_LOW);
	if (SYS_IC_BONDING_TYPE_5(tp))
	{
		if (0 != smc_read_rx(tp, p))
			return 1;
	}
	else
		return 1;

    return 0;
}

static inline int smc_atr_isr1_count_trigger(struct smc_device *dev, void __iomem *p)
{
    unsigned char b_cnt_trigger = 0;
    UINT32 t_old_tick = 0;    
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
	smc_debug("SMC ATR: In %s Set reset high, tp is %p 0x%x\n", 
                  __func__, tp, tp->isr1_interrupt_status);
    
	tp->isr1_interrupt_status &= (~SMC_ISR1_COUNT_ST);
	tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);

    writeb(readb(p + REG_ISR1) | SMC_ISR1_RST_NATR, p + REG_ISR1);
    writeb(readb(p + REG_ICCR) | SMC_RB_ICCR_RST, p + REG_ICCR);
	
	for ( ; ; )
	{		
		if (tp->isr1_interrupt_status & SMC_ISR1_RST_HIGH)
		{
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_HIGH);
            smc_debug("SMC ATR: In %s reset high!\n", __func__);
			break;
		}
        
		if ((tp->isr1_interrupt_status & SMC_ISR1_RST_NATR) && (0 == b_cnt_trigger))
		{
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
			b_cnt_trigger = 1;
			t_old_tick = jiffies;			
		}
        
  		if (b_cnt_trigger && (jiffies - t_old_tick) > 100)
		{
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
			smc_debug(KERN_ERR "SMC ATR: In %s None ATR!\n", __func__);
			return -ETIMEDOUT;
		}

		if (0 == smc_misc_card_exist(dev))
		{
			return -EIO;
		}		
	}    
	return 0;
}

static inline int smc_atr_isr1_process(struct smc_device *dev, void __iomem *p)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    for ( ; ; )
	{
		if (tp->isr1_interrupt_status & SMC_ISR1_RST_LOW)
		{
			ret = smc_atr_isr1_low_trigger(dev, p);
            if (ret) return ret;
		}
        
		if (tp->isr1_interrupt_status & SMC_ISR1_COUNT_ST)
		{
			ret = smc_atr_isr1_count_trigger(dev, p);
            return ret;
		}
        
		if (0 == smc_misc_card_exist(dev))
		{
			return -EIO;
		}	
        msleep(5);
	}
    return ret;
}

static inline int smc_atr_isr0_process(struct smc_device *dev, void __iomem *p)
{
    UINT32 t_wait_atr_tmo = 0, t_wait_atr_time = 0;
    UINT16 i_rx_cnt=0;
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    t_wait_atr_tmo = (9600 * 2 * readw(p + REG_ETU0)) / (tp->smc_clock / 1000);
	t_wait_atr_time = jiffies;
	i_rx_cnt = smc_read_rx(tp, p); 
    
	for ( ; ; )
	{
		if (0 == smc_misc_card_exist(dev))
		{
			return -EIO;
		}		
        
		if ((tp->isr0_interrupt_status & SMC_ISR0_FIFO_RECV ) || \
            (tp->isr0_interrupt_status & SMC_ISR0_TIMEOUT))
		{
			tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_RECV);
			tp->isr0_interrupt_status &= (~SMC_ISR0_TIMEOUT);
			break;
		}
        
		if (0 == (readb(p + REG_ICCR) & 0x30) && \
           (tp->isr0_interrupt_status & SMC_ISR0_PE_RECV))
		{
			smc_debug(KERN_ERR "SMC ATR: In %s ATR Parity Error!\n", __func__);
			tp->isr0_interrupt_status &= (~SMC_ISR0_PE_RECV);
			return -EINVAL;
		}
        
		if (i_rx_cnt == smc_read_rx(tp, p))
		{
			mdelay(1);
			t_wait_atr_time = jiffies - t_wait_atr_time;
            
			if (t_wait_atr_tmo >= t_wait_atr_time)
			{
				t_wait_atr_tmo -= t_wait_atr_time;
			}
			else
			{
				if(0 != smc_read_rx(tp, p))
					break;
                
				smc_debug(KERN_ERR "SMC ATR: In %s ATR time out!\n", __func__);
				return -ETIMEDOUT;
			}	
		}
		else
		{
			i_rx_cnt = smc_read_rx(tp, p);
			t_wait_atr_tmo = (9600 * 2 * readw(p + REG_ETU0)) / (tp->smc_clock / 1000);
		}
		t_wait_atr_time = jiffies;
	}

    return ret;
}

static inline int smc_atr_card_type(struct smartcard_private *tp, void __iomem *p)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;

#define SMC_ATR_SET_TO_INVERSE_MODE \
    tp->inverse_convention = 1; \
	writeb(SMC_SCR_CTRL_OP | SMC_SCR_CTRL_INVESE | (tp->parity_odd<<4), p + REG_SCR_CTRL);

    if (0x03 == atr->atr_buf[0])
	{
		smc_debug("SMC ATR: In %s Inv card detected!\n", __func__);
		SMC_ATR_SET_TO_INVERSE_MODE
		invert(atr->atr_buf, atr->atr_size);
	}
 	else if ((0x3f == atr->atr_buf[0]) && (1 == tp->ts_auto_detect))
 	{
 		smc_debug("SMC ATR: In %s Inv card auto detected!\n", __func__);
 		SMC_ATR_SET_TO_INVERSE_MODE
 	}
	else if (0x3b == atr->atr_buf[0])
	{
		smc_debug("SMC ATR: In %s Normal card detected!\n", __func__);
		tp->inverse_convention = 0;
	}
	else
		return -EINVAL;

    return 0;
}

static inline int smc_atr_decode(struct smc_device *dev, void __iomem *p)
{
    int i=0, c=0;	
	unsigned char cc = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    
    c = smc_read_rx(tp, p); 		
	if (0 == c || c > ATR_MAX_SIZE)
	{
		smc_debug(KERN_ERR "SMC ATR: In %s Invalid length: %d\n", __func__, c);
		if (c > ATR_MAX_SIZE)
			c = ATR_MAX_SIZE;
		else
			return -EINVAL;
	}
	smc_debug("SMC ATR: In %s [%d] bytes received\n", __func__, c);
    
	atr->atr_size = c;
    smc_debug("SMC ATR: Size =  %d\n", c);
	for (i = 0; i < c; i++)
	{
		cc = readb(p + REG_RBR);
		smc_debug("%02x ",cc);
		atr->atr_buf[i] = cc;
	}	
	smc_debug("\n");

    SMC_ATR_PRESET_TEST_BUF(atr, c, p, i)

	return smc_atr_card_type(tp, p);
}

static inline void smc_atr_set_receiver_mode(struct smc_device *dev, 
                                           void __iomem *p, int enable)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    if (enable) /* enable receiver mode and set to the direct mode */        
        writeb(SMC_SCR_CTRL_OP | SMC_SCR_CTRL_RECV | (tp->parity_odd<<4), p + REG_SCR_CTRL);
    else /* disable receiver mode. */        
	    writeb(readb(p + REG_SCR_CTRL) & (~SMC_SCR_CTRL_RECV), p + REG_SCR_CTRL);
}

#define SMC_ATR_INIT(atr) \
    do { \
        atr->atr_size = 0; \
        atr->atr_rlt = SMC_ATR_NONE; \
        memset(atr->atr_buf, 0x00, ATR_MAX_SIZE); \
    } while (0)

int smc_atr_get(struct smc_device *dev)
{	
	struct smartcard_private *tp = NULL;
    struct smartcard_atr *atr = NULL;
    void __iomem *p = NULL;
    int ret = 0;


    BUG_ON(NULL == dev);
    BUG_ON(NULL == dev->priv);

    tp = (struct smartcard_private *)dev->priv;
    atr = (struct smartcard_atr *)tp->atr;
	p = (void __iomem *)dev->io_base;
	
	smc_debug(KERN_INFO "SMC ATR: In %s\n", __func__);

	tp->force_tx_rx_thld = FORCE_TX_RX_THLD;	
    SMC_ATR_INIT(atr);
    smc_atr_set_receiver_mode(dev, p, TRUE);
	smc_atr_set_register_group_one(tp, p);
	                                         
	if (!tp->inserted)  //card not insert
	{
        return -EINVAL;
	}

    if(tp->warm_reset)
	{
		smc_atr_set_register_group_two(tp, p);
	}
	else	
	{
		smc_atr_set_register_group_three(tp, p);
	}

    smc_atr_set_register_special_card_check(tp, p);
    
    smc_atr_set_register_group_four(tp, p);
	
	if ((ret = smc_atr_isr1_process(dev, p)) < 0)
        return ret;
	if ((ret = smc_atr_isr0_process(dev, p)) < 0)
        return ret;

    spin_lock_irq(&dev->smc_spinlock);
	if ((ret = smc_atr_decode(dev, p)) < 0)
    {
        spin_unlock_irq(&dev->smc_spinlock);
        return ret;
	}
    spin_unlock_irq(&dev->smc_spinlock);
	smc_atr_set_receiver_mode(dev, p, FALSE);
    
	return 0;
}

/* Get ATR buffer for SMC device */
void *smc_atr_alloc(struct smartcard_private *tp)
{
    struct smartcard_atr *atr;
    atr = (struct smartcard_atr *)kmalloc(sizeof(struct smartcard_atr), GFP_KERNEL);
	BUG_ON(NULL == atr);
	memset(atr, 0, sizeof(struct smartcard_atr));
    atr->atr_info = (atr_t *)kmalloc(sizeof(atr_t), GFP_KERNEL);
    BUG_ON(NULL == atr->atr_info);
    memset(atr->atr_info, 0, sizeof(atr_t));

    spin_lock_init(&atr->atr_spinlock);

    return (void *)atr;
}

void smc_atr_free(struct smartcard_private *tp)
{
    struct smartcard_atr *atr;
    
    if (NULL == tp->atr) return;
    atr = (struct smartcard_atr *)tp->atr;
    if (NULL != atr->atr_info)
        kfree(atr->atr_info);
    kfree(atr);
    atr->atr_info = NULL;
    tp->atr = NULL;
}

static inline int smc_atr_extract_interface_bytes(struct smartcard_atr *atr, int TDi, unsigned char *const pn)
{
    UINT8 pointer = 1;
    atr_t *atr_info = (atr_t *)atr->atr_info;
    int ret = SMART_NO_ERROR;
    unsigned char i_pn = *pn; /* What a shit pointor used */

    smc_debug(KERN_INFO "SMC ATR: In %s Value %p %p %d %d\n", 
                  __func__, atr, atr_info, atr->atr_size, i_pn);
    
	/* Extract interface bytes */
	while (pointer < atr->atr_size)
	{
		/* Check if buffer is long enought */
		if (pointer + atr_num_ib_table[(0xF0 & TDi)>>4] >= atr->atr_size)
		{
			return SMART_WRONG_ATR;
		}
        
		/* Check TAi is present */
		if (0xFF == (TDi | 0xEF))
		{
			pointer++;
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TA].value = atr->atr_buf[pointer];
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TA].present = TRUE;
		}
		else
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TA].present = FALSE;
        
		/* Check TBi is present */
		if (0xFF == (TDi | 0xDF))
		{
			pointer++;
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TB].value = atr->atr_buf[pointer];
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TB].present = TRUE;
		}
		else
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TB].present = FALSE;

		/* Check TCi is present */
		if (0xFF == (TDi | 0xBF))
		{
			pointer++;
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TC].value = atr->atr_buf[pointer];
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TC].present = TRUE;
		}
		else
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TC].present = FALSE;

		/* Read TDi if present */
		if (0xFF == (TDi | 0x7F))
		{
			pointer++;
			TDi = atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TD].value = atr->atr_buf[pointer];
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TD].present = TRUE;
			(atr_info->TCK).present = ((TDi & 0x0F) != ATR_PROTOCOL_TYPE_T0);
			if (i_pn >= ATR_MAX_PROTOCOLS)
			{
				return SMART_WRONG_ATR;
			}
            i_pn++;
		}
		else
		{
			atr_info->ib[i_pn][ATR_INTERFACE_BYTE_TD].present = FALSE;
			break;
		}
	}

    *pn = i_pn;
    return ret;
}

static inline int smc_atr_init(struct smartcard_atr *atr)
{
	UINT8 TDi;
	UINT8 pointer = 0, pn = 0;
	UINT8 i;
    atr_t *p_atr_info = atr->atr_info;
    int ret = SMART_NO_ERROR;
	
	/* Check size of buffer */
	if (atr->atr_size < 2)
		return SMART_WRONG_ATR;
	
	/* Store T0 and TS */
	p_atr_info->TS = atr->atr_buf[0];
	p_atr_info->T0 = TDi = atr->atr_buf[1];
	
	/* Store number of historical bytes */
	p_atr_info->hbn = TDi & 0x0F;

	/* TCK is not present by default */
	(p_atr_info->TCK).present = FALSE;

    if (SMART_NO_ERROR != (ret = smc_atr_extract_interface_bytes(atr, TDi, &pn)))
        return ret;

	/* Store number of protocols */
	p_atr_info->pn = pn + 1;

	/* Store historical bytes */
	if (pointer + p_atr_info->hbn >= atr->atr_size)
		return SMART_WRONG_ATR;

	for (i = 0; i < p_atr_info->hbn; i++)
		*(p_atr_info->hb + i) = *(atr->atr_buf + pointer + 1 + i);
	pointer += (p_atr_info->hbn);

	/* Store TCK  */
	if ((p_atr_info->TCK).present)
	{

		if (pointer + 1 >= atr->atr_size)
		    return SMART_WRONG_ATR;
		pointer++;
		(p_atr_info->TCK).value = atr->atr_buf[pointer];
	}

	p_atr_info->length = pointer + 1;
	return SMART_NO_ERROR;
}

static inline void smc_atr_handle_F(struct smartcard_private *tp)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    UINT8 FI;
	UINT32 F;

    if (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		FI = (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
		F = atr_f_table[FI];
		if(F == F_RFU)
			F = ATR_DEFAULT_F;
		smc_debug(KERN_INFO "SMC ATR: In %s Clock Rate Conversion F=%ld, FI=%d\n", 
                      __func__, F, FI);
	}
	else
	{
		F = ATR_DEFAULT_F;
		smc_debug(KERN_INFO "SMC ATR: In %s Clock Rate Conversion F=(Default)%ld\n", 
                      __func__, F);
	}
	tp->F = F;
}

static inline void smc_atr_handle_D(struct smartcard_private *tp)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    UINT8 DI;
	UINT32 D;

    if (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		DI = (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F);
		D = atr_d_table[DI];
		if(D == D_RFU)
			D = ATR_DEFAULT_D;
		smc_debug(KERN_INFO "SMC ATR: In %s Bit Rate Adjustment Factor D=%ld, DI=%d\n", 
                      __func__, D, DI);
	}
	else
	{
		D = ATR_DEFAULT_D;
		smc_debug(KERN_INFO "SMC ATR: In %s Bit Rate Adjustment Factor D=(Default)%ld\n", 
                      __func__, D);
	}
	tp->D = D;
}

static inline void smc_atr_handle_I(struct smartcard_private *tp)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    UINT8 II;
	UINT32 I;

    if (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TB].present)
	{
		II = (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x60) >> 5;
		I = atr_i_table[II];
		if(I == I_RFU)
			I = ATR_DEFAULT_I;
		smc_debug(KERN_INFO "SMC ATR: In %s Programming Current Factor I=%ld, II=%d\n", 
                      __func__, I, II);
	}
	else
	{
		I= ATR_DEFAULT_I;
		smc_debug(KERN_INFO "SMC ATR: In %s Programming Current Factor I=(Default)%ld\n", 
                      __func__, I);	
	}
	tp->I = I;
}

static inline void smc_atr_handle_P(struct smartcard_private *tp)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    UINT8 PI1, PI2;
	UINT32 P;

    /* We don't need PI1/PI2, right ? */
    if (p_atr_info->ib[1][ATR_INTERFACE_BYTE_TB].present)
	{
		PI2 = p_atr_info->ib[1][ATR_INTERFACE_BYTE_TB].value;
		P = PI2;
		smc_debug(KERN_INFO "SMC ATR: In %s Programming Voltage Factor P=%ld, PI2=%d\n", 
                      __func__, P, PI2);
	}
	else if (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TB].present)
	{
		PI1 = (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x1F);
		P = PI1;
		smc_debug(KERN_INFO "SMC ATR: In %s Programming Voltage Factor P=%ld, PI1=%d\n", 
                      __func__, P, PI1);
	}
	else
	{
		P = ATR_DEFAULT_P;
		smc_debug(KERN_INFO "SMC ATR: In %s Programming Voltage Factor P=(Default)%ld\n", 
                      __func__, P);
	}
	tp->P = P;
}

static inline void smc_atr_handle_N(struct smartcard_private *tp)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;    
	UINT32 N;

    if (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TC].present)
	{
		N = p_atr_info->ib[0][ATR_INTERFACE_BYTE_TC].value;
	}
	else
		N = ATR_DEFAULT_N;
    
	smc_debug(KERN_INFO "SMC ATR: In %s Extra Guardtime N=%ld\n", __func__, N);
	tp->N = N;
}

static inline int smc_atr_handle_special_T(struct smartcard_private *tp, UINT8 *T)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;

    /* if has TA2, it indicate the special protocol */
	if (p_atr_info->ib[1][ATR_INTERFACE_BYTE_TA].present)
	{
		*T = p_atr_info->ib[1][ATR_INTERFACE_BYTE_TA].value & 0x0F;
		smc_debug(KERN_INFO "SMC ATR: In %s Specific mode found: T=%d\n", __func__, *T);
		if (p_atr_info->ib[1][ATR_INTERFACE_BYTE_TA].value & 0x10)  /* check TA(2), bit 5 */
			tp->TA2_spec = 0;         /* use the default value of F/D */
		else
			tp->TA2_spec = 1;         /* Use the value specified in the ATR  */
	}

    return *T;
}

static inline void smc_atr_config_T_0(struct smartcard_private *tp)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    UINT8 WI;

    if (p_atr_info->ib[2][ATR_INTERFACE_BYTE_TC].present)
	{
		WI = p_atr_info->ib[2][ATR_INTERFACE_BYTE_TC].value;
	}
	else
		WI = ATR_DEFAULT_WI;
    
	smc_debug(KERN_INFO "SMC ATR: In %s Work Waiting Time WI=%d\n", __func__, WI);
	tp->WI = WI;
	
	tp->first_cwt = tp->cwt = (960 * WI * tp->F) / (tp->smc_clock / 1000) + 1;
}

static inline void smc_atr_config_T_1(struct smartcard_private *tp)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    t1_state_t *T1 = (t1_state_t *)tp->T1;
    UINT8 i, checksum=0;

    for (i = 1 ; i < p_atr_info->pn ; i++) 
	{
        /* check for the first occurance of T=1 in TDi */
	    if (p_atr_info->ib[i][ATR_INTERFACE_BYTE_TD].present && 
    		ATR_PROTOCOL_TYPE_T1 == (p_atr_info->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F)) 
		{
 			/* check if ifsc exist */
   			if (p_atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TA].present)
   			    T1->ifsc = p_atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TA].value;
		    else
			    T1->ifsc = ATR_DEFAULT_IFSC; /* default 32*/
        
   			/* Get CWI */
   			if (p_atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
    		    tp->CWI = p_atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0x0F;
   			else
        		tp->CWI = ATR_DEFAULT_CWI; /* default 13*/
        	tp->cwt =  (((1<<(tp->CWI)) + 11) * tp->smc_etu) / (tp->smc_clock / 1000) + 1; 

            /*Get BWI*/
   			if (p_atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
    			tp->BWI = (p_atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0xF0) >> 4;
    		else
      		    tp->BWI = ATR_DEFAULT_BWI; /* default 4*/    

       		tp->first_cwt = (11 * tp->smc_etu) / (tp->smc_clock/1000) + \
                            ((1<<(tp->BWI)) * 960 * ATR_DEFAULT_F) / (tp->smc_clock / 1000) + 2;	
   			if (p_atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TC].present)
       			checksum = p_atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TC].value & 0x01;
   			else
       			checksum = ATR_DEFAULT_CHK; /* default - LRC */
		    tp->error_check_type = ((checksum == ATR_DEFAULT_CHK) ? \
                                    IFD_PROTOCOL_T1_CHECKSUM_LRC : IFD_PROTOCOL_T1_CHECKSUM_CRC);
    
		}
	}
    
	smc_debug(KERN_INFO "SMC ATR: In %s T1 special -- ifsc = %ld,  CWI = %d,  BWI = %d, \
                  checksum = %d(3: LRC;2: CRC)\n",
				  __FUNCTION__, T1->ifsc, tp->CWI, tp->BWI, tp->error_check_type);
}

static inline void smc_atr_config_T(struct smartcard_private *tp)
{
	if (0 != tp->D)
		tp->smc_etu = tp->F / tp->D;
	tp->first_cwt = FIRST_CWT_VAL; 
	tp->cwt = CWT_VAL;
    
	if(0 == tp->T)
	{
		smc_atr_config_T_0(tp);
	}
	else if(1 == tp->T)
	{
		smc_atr_config_T_1(tp);
	}	
}

static inline void smc_atr_handle_T(struct smartcard_private *tp)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    UINT8 T = 0xFF;
    UINT8 i;
    UINT8 TA_AFTER_T15 = 0;
	UINT8 b_find_T15 = 0;

    for (i = 0; i < p_atr_info->pn; i++)
    {
        if (p_atr_info->ib[i][ATR_INTERFACE_BYTE_TD].present && (0xFF == T))
        {
        	/* set to the first protocol byte found */
        	T = p_atr_info->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F;
        	smc_debug(KERN_INFO "SMC ATR: In %s Default protocol: T=%d\n", __func__, T);
        }
    }
    
	/* Try to find 1st TA after T = 15 */
	for (i = 0; i < p_atr_info->pn; i++)
	{
		if (p_atr_info->ib[i][ATR_INTERFACE_BYTE_TD].present)
		{
			if ((!b_find_T15) && (0xF == (p_atr_info->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F)))
			{
				b_find_T15 = 1;
				smc_debug(KERN_INFO "SMC ATR: In %s Find T==15 at TD%d\n", __func__, i + 1);
				continue;
			}
		}
        
		if ((b_find_T15) && (p_atr_info->ib[i][ATR_INTERFACE_BYTE_TA].present))
		{
			TA_AFTER_T15 = p_atr_info->ib[i][ATR_INTERFACE_BYTE_TA].value;
			smc_debug(KERN_INFO "SMC ATR: In %s Find 1st TA after T==15 at TA%d, value %02x\n", \
                          __func__, (i+1), TA_AFTER_T15);
			break;
		}
	}
    
	if (tp->class_selection_supported)
	{
		if (0 == (TA_AFTER_T15 & 0x3f))
			smc_debug(KERN_INFO "SMC ATR: In %s No class indicator!\n", __func__);
		tp->smc_supported_class = TA_AFTER_T15 & 0x3f;	
	}
    
	smc_atr_handle_special_T(tp, &T);
	
	if (0xFF == T)
	{
		smc_debug(KERN_INFO "SMC ATR: In %s No default protocol found, Using T=0\n", __func__);
		T = ATR_PROTOCOL_TYPE_T0;
	}
	tp->T = T;
    smc_atr_config_T(tp);	
}

static inline int smc_atr_config_parameter(struct smartcard_private *tp)
{
	UINT8 i;
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    
	smc_atr_handle_F(tp);
    smc_atr_handle_D(tp);
	smc_atr_handle_I(tp);
	smc_atr_handle_P(tp);
    smc_atr_handle_N(tp);
    smc_atr_handle_T(tp);

    smc_debug(KERN_INFO "SMC ATR: In %s First CWT: %ld, CWT: %ld\n", 
                  __func__, tp->first_cwt, tp->cwt);	
	
	smc_debug(KERN_INFO "SMC ATR: In %s HC ---- %d\n", 
                  __func__, p_atr_info->hbn);
	for (i = 0; i < p_atr_info->hbn; i++)
	{
		smc_debug("%2x ", (p_atr_info->hb)[i]);
	}
	smc_debug("\n");
	
	return SMART_NO_ERROR;
}

static inline int smc_atr_device_config_T_1(struct smc_device *dev, void __iomem *p)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    if (0 != tp->D)
	{
		tp->smc_etu = tp->F / tp->D;
		smc_debug(KERN_INFO "SMC ATR: In %s Work etu : %ld\n", __func__, tp->smc_etu);
	}
    
	spin_lock_irq(&dev->smc_spinlock);
	if (1 != tp->inserted)  /* card not insert */
	{
		spin_unlock_irq(&dev->smc_spinlock);
		return -EIO;
	}
    
	if (tp->apd_disable)
        writeb(SMC_RB_ICCR_AUTO_PRT | SMC_RB_ICCR_OP, p + REG_ICCR);
	else	 /* disable auto parity error pull down control for T=1 card. */
        writeb(SMC_RB_ICCR_AUTO_PRT | SMC_RB_ICCR_OP | 0x40, p + REG_ICCR);
    
	spin_unlock_irq(&dev->smc_spinlock);

    return 0;
}

static inline int smc_atr_device_config_T_14(struct smc_device *dev, void __iomem *p)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    spin_lock_irq(&dev->smc_spinlock);
	if (1 != tp->inserted)  /* card not insert */
	{
		spin_unlock_irq(&dev->smc_spinlock);
		return -EIO;
	}        

    if (tp->apd_disable)
        writeb(SMC_RB_ICCR_PRT_EN | SMC_RB_ICCR_OP, p + REG_ICCR);
	else	/* disable parity control for T=14 card. */
        writeb(SMC_RB_ICCR_PRT_EN | SMC_RB_ICCR_OP | 0x40, p + REG_ICCR);
	
	spin_unlock_irq(&dev->smc_spinlock);

    return 0;
}

static inline int smc_atr_device_config_T_0(struct smc_device *dev, void __iomem *p)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    if (0 != tp->D)
	{
		tp->smc_etu = tp->F / tp->D;
		smc_debug(KERN_INFO "SMC ATR: In %s Work etu : %ld\n", __func__, tp->smc_etu);
	}
    
	spin_lock_irq(&dev->smc_spinlock);
	if (1 != tp->inserted)  /* card not insert */
	{
        smc_debug(KERN_INFO "SMC ATR: In %s Card remove\n", __func__);
		spin_unlock_irq(&dev->smc_spinlock);
		return -EIO;
	}
    
	if (tp->apd_disable)
        writeb((tp->apd_disable<<4) | SMC_RB_ICCR_OP, p + REG_ICCR);
	else	 /* enable parity and auto parity error pull down control.T=0 card. */
        writeb((tp->apd_disable<<4) | SMC_RB_ICCR_OP | 0x40, p + REG_ICCR);
    
	spin_unlock_irq(&dev->smc_spinlock);

    return 0;
}

static inline void smc_atr_set_guart_value(struct smc_device *dev, void __iomem *p)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    if (0xFF == tp->N)
	{
		if (1 == tp->T)
            writeb(11, p + REG_GTR0);
		else
			writeb(12, p + REG_GTR0);
	}
	else
        writeb(12 + tp->N, p + REG_GTR0);
}

static inline void smc_atr_caculate_T_1_param_group_one(struct smartcard_private *tp)
{ 
    t1_state_t *T1 = (t1_state_t *)tp->T1;
    /*BGT = 22 etu*/
	T1->BGT =  (22 * tp->smc_etu) / (tp->smc_clock / 1000);
	/*CWT = (2^CWI + 11) etu*/
	T1->CWT =  tp->cwt;  
	/* BWT = (2^BWI*960 + 11)etu, 
     * Attention: BWT in ms, If calc in us, it will overflow for UINT32
     */
	T1->BWT= tp->first_cwt;

	/*Add error check type*/
	smc_t1_set_checksum(T1, tp->error_check_type);
	/*reset the T1 state to undead state*/
	smc_t1_set_param(T1, IFD_PROTOCOL_T1_STATE, SENDING);
	smc_debug(KERN_INFO "SMC ATR: In %s T1 special ---- BGT = %ld, CWT = %ld, us BWT = %ld ms\n",
                  __func__, T1->BGT, T1->CWT, T1->BWT);
}

static inline void smc_atr_caculate_T_1_param_group_two(struct smartcard_private *tp)
{
    t1_state_t *T1 = (t1_state_t *)tp->T1;
    
    tp->cwt = (((1<<(tp->CWI)) + 11) * tp->smc_etu) / (tp->smc_clock / 1000) + 1; 
	tp->first_cwt = (11 * tp->smc_etu) / (tp->smc_clock / 1000) + \
                    ((1<<((UINT32)tp->BWI)) * 960 * ATR_DEFAULT_F) / (tp->smc_clock / 1000) + 2;	
	/*BGT = 22 etu*/
	T1->BGT = (22 * tp->smc_etu) / (tp->smc_clock / 1000);
	/*CWT = (2^CWI + 11) etu*/
	T1->CWT = tp->cwt;  
	/* BWT = (2^BWI*960 + 11)etu, 
	 * Attention: BWT in ms, If calc in us, it will overflow for UINT32
	 */
	T1->BWT= tp->first_cwt;
	smc_debug(KERN_INFO "SMC ATR: In %s T1 disable PPS: CWT %ld, BWT %ld\n", 
                  __func__, tp->cwt,  tp->first_cwt);
}

static inline int smc_atr_check_special_card(struct smartcard_private *tp)
{
	#ifdef SUPPORT_NDS_CARD
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;    
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    #endif
    
#ifdef SUPPORT_NDS_CARD
	if ((0 == tp->T) && (1 == tp->inverse_convention))
	{
  		if((0x7F == atr->atr_buf[1] || 0xFF == atr->atr_buf[1] || 0xFD == atr->atr_buf[1]) && 
		   (0x13 == atr->atr_buf[2] || 0x11 == atr->atr_buf[2]))
  		{
  			p_atr_info->ib[1][ATR_INTERFACE_BYTE_TA].present = TRUE;
			return 1;
  		}
	}
    return 0;
#else
    return 0;
#endif
}

static inline int smc_atr_is_multi_procotol(struct smartcard_private *tp, int *protocol_num)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    int i = 0, ret = 0;
    UINT8 T = 0xff;

    *protocol_num = 0;
    for (i = 0; i < ATR_MAX_PROTOCOLS; i++)
    {
		if (p_atr_info->ib[i][ATR_INTERFACE_BYTE_TD].present)
		{
			UINT8 T_type = p_atr_info->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F;
            
			if (0xf == T_type) break;
			if (0xff == T) 
                T = T_type;
			else if (T != T_type)
			{
				T = T_type;
				ret = 1;
			}
		}
    }
    *protocol_num = i;

    return ret;
}

static inline int __smc_atr_config_pps(struct smc_device *dev, void __iomem *p, 
                                   int *b_need_reset_etu, UINT8 pps0, UINT8 FI, UINT8 DI,
                                   UINT32 bk_first_cwt, UINT32 bk_cwt)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
        
    if ((ret = smc_misc_set_pps(dev, p, pps0 | (tp->T & 0xf), FI, DI)) < 0)
	{
		smc_debug(KERN_ERR "SMC ATR: In %s PPS NG, deactive smart card\n", __func__);
        smc_misc_dev_deactive(dev);
        return ret; /* Special return value for caller to release device */
	}
    else
    {
        tp->first_cwt = bk_first_cwt;
		tp->cwt = bk_cwt;
		*b_need_reset_etu = 1;
		smc_debug(KERN_ERR "SMC ATR: In %s PPS OK, set etu as %ld\n", __func__, tp->smc_etu);
        return 0;
    }
}

static inline int smc_atr_config_pps(struct smc_device *dev, void __iomem *p, 
                                 int *b_need_reset_etu, int *p_DI, int *p_FI, int *protocol_num)
{
    UINT8 pps0 = 0;
	UINT32 first_cwt ; 
	UINT32 cwt;
	UINT8 b_multi_protocol = 0;
	UINT8 b_diff_f_d = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    UINT8 FI = 1;
	UINT8 DI = 1;
    int ret = 0;
    
	if (tp->disable_pps)
	{
		if (1 == tp->T)
		{
			tp->smc_etu = readb(p + REG_ETU0);
			smc_atr_caculate_T_1_param_group_two(tp);
		}
        mdelay(10);	
		return SMC_FAKE_RETURN_VALUE; /* This is a fake return for the upper function judgement */
	}
    
	if (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		*p_FI = FI = (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
		*p_DI = DI =  (p_atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F); 
        b_diff_f_d = 1;
	}
	
    b_multi_protocol = smc_atr_is_multi_procotol(tp, protocol_num);

	if (b_multi_protocol || b_diff_f_d)
	{
		first_cwt = tp->first_cwt; 
		cwt = tp->cwt;
        
		if (b_diff_f_d)
		{
			pps0 |= 0x10;
		}
		smc_debug(KERN_INFO "SMC ATR: In %s PPS0 %d, T %d, FI %d, DI %d\n", 
                      __func__, pps0, tp->T, FI, DI);
		tp->first_cwt = tp->cwt = (960 * ATR_DEFAULT_WI * ATR_DEFAULT_F) / (tp->smc_clock / 1000);
	
		ret = __smc_atr_config_pps(dev, p, b_need_reset_etu, pps0, FI, DI, first_cwt, cwt);
	}

    return ret;
}

static inline void smc_atr_reset_etu(struct smartcard_private *tp, void __iomem *p, 
                                int DI, int FI, int protocol_num)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    atr_t *p_atr_info = (atr_t *)atr->atr_info;
    
    if (0 == tp->T)
	{
		UINT8 WI = ATR_DEFAULT_WI;
			
		if (p_atr_info->ib[2][ATR_INTERFACE_BYTE_TC].present)
			WI = p_atr_info->ib[2][ATR_INTERFACE_BYTE_TC].value;
		
		writel(960 * WI * DI, p + REG_CBWTR0);
	}
	else if (1 == tp->T)
	{
		UINT8 BWI = ATR_DEFAULT_BWI; 
        
		if (p_atr_info->ib[protocol_num + 1][ATR_INTERFACE_BYTE_TB].present)
            BWI = (p_atr_info->ib[protocol_num + 1][ATR_INTERFACE_BYTE_TB].value & 0xF0)>>4;
		writel(11 + ((960 * 372 * (1<<BWI) * DI) / FI), p + REG_CBWTR0);
	}
    
	writew(tp->smc_etu, p + REG_ETU0);	
}

/* Configure the ATR info for Smart card */
int smc_atr_info_config(struct smc_device *dev)
{  
    struct smartcard_private *tp = NULL;
    void __iomem *p = NULL;
    struct smartcard_atr *atr = NULL;
    atr_t *p_atr_info = NULL;
    int ret = 0;
    int b_need_reset_etu = 0;
    UINT8 DI = 1, FI = 1;
    int u8_protocol_num = 0;


    BUG_ON(NULL == dev);
    BUG_ON(NULL == dev->priv);
    BUG_ON(NULL == dev->priv->atr);

    tp = (struct smartcard_private *)dev->priv;
    p = (void __iomem *)dev->io_base;
    atr = (struct smartcard_atr *)tp->atr;
    p_atr_info = (atr_t *)atr->atr_info;

    if (SMART_WRONG_ATR == smc_atr_init(atr))
		atr->atr_rlt = SMC_ATR_WRONG;
	else
		atr->atr_rlt = SMC_ATR_OK;

	smc_atr_config_parameter(tp);
    
	if (1 == tp->T)
	{
		if ((ret = smc_atr_device_config_T_1(dev, p)) < 0)
            return ret;
	}
	else if (14 == tp->T)
	{
		if ((ret = smc_atr_device_config_T_14(dev, p)) < 0)
            return ret;
	}
	else
	{
		if ((ret = smc_atr_device_config_T_0(dev, p)) < 0)
            return ret;
	}
	smc_atr_set_guart_value(dev, p);

	if (1 == tp->T)
	{
        smc_atr_caculate_T_1_param_group_one(tp);
    }
	tp->reseted = 1;
    b_need_reset_etu = smc_atr_check_special_card(tp);

	if (FALSE == p_atr_info->ib[1][ATR_INTERFACE_BYTE_TA].present)
	{		
		if ((ret = smc_atr_config_pps(dev, p, &b_need_reset_etu, \
                                        (int *)&DI, (int *)&FI, &u8_protocol_num)) < 0)
		{
            if (SMC_FAKE_RETURN_VALUE == ret) return 0;           
            return ret;
		}
	}
	else
	{	
		smc_debug(KERN_INFO "SMC ATR: In %s Specific mode!\n", __func__);
		if (!((p_atr_info->ib[1][ATR_INTERFACE_BYTE_TA].value) & 0x10))
			b_need_reset_etu = 1;
	}
    
	if (b_need_reset_etu)
	{
		smc_atr_reset_etu(tp, p, DI, FI, u8_protocol_num);
	}

    return 0;
}



