/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_gpio.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard gpio.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#include "ali_smartcard_irq.h"
#include "ali_smartcard_gpio.h"

/* SMC GPIO direction definition */
#define SMC_GPIO_I_DIRECTION 0
#define SMC_GPIO_O_DIRECTION 1

/* Set SMC GPIO value and direction */
static void smc_gpio_set_gpio(const struct smc_gpio gpio)
{
    if (SMC_GPIO_I_DIRECTION == gpio.direction)
        ali_gpio_direction_input(gpio.pos);
    else if (SMC_GPIO_O_DIRECTION == gpio.direction)
        ali_gpio_direction_output(gpio.pos, gpio.val);
    else
        smc_debug(KERN_INFO "SMC GPIO: In %s Not set direction\n", __func__);

    
   	ali_gpio_set_value(gpio.pos, gpio.val);    
}

void smc_gpio_set_vpp(struct smartcard_private *tp)
{
    struct smc_gpio gpio = {0, 0, 0};

    if (tp->use_gpio_vpp)
    {
        gpio.pos = tp->gpio_vpp_pos;
        gpio.val = tp->gpio_vpp_pol;
        gpio.direction = tp->gpio_vpp_io;
        
        smc_gpio_set_gpio(gpio);
    }
}

/* GPIO card detect config */
int smc_gpio_set_card_detect(struct smc_device *dev)
{    
    struct smc_gpio gpio = {0, 0, 0};
    struct smartcard_private *tp = NULL;
    int ret = 0;


    BUG_ON(NULL == dev || NULL == dev->priv);
    
	tp = (struct smartcard_private *)dev->priv;
    
    if (tp->use_gpio_cd)
	{	
		enable_irq(tp->gpio_cd_pos);
		/* disable tmp */
        //set_irq_type(tp->gpio_cd_pos, IRQ_TYPE_EDGE_RISING);
        //set_irq_type(tp->gpio_cd_pos, IRQ_TYPE_EDGE_FALLING);
		gpio_irq_clear(tp->gpio_cd_pos); // Different from disable_irq
		                                 // Linux kernel don't implement clear function
		                                 // This is an ALi GPIO function
		
		if (SYS_IC_BONDING_TYPE_5(tp))
			dev->gpio_irq = 8;
		else
		{
			if (tp->gpio_cd_pos < 32)
				dev->gpio_irq = 9;
			else
			{
				if (SYS_IC_BONDING_TYPE_6(tp))
				{
					if (tp->gpio_cd_pos < 64) dev->gpio_irq = 24;
					else dev->gpio_irq = 31;
				}
				else 
                    dev->gpio_irq = 24;
			}
		}
        gpio.val = -1;
        gpio.pos = tp->gpio_cd_pos;
        gpio.direction = tp->gpio_cd_io;
		smc_gpio_set_gpio(gpio);

        smc_debug(KERN_INFO "SMC GPIO: In %s Request irq %d\n", 
                      __func__, dev->gpio_irq);
        ret = request_irq(dev->gpio_irq, smc_irq_gpio_detect, 0, dev->dev_name, dev);
        if (ret)
        {
            smc_debug(KERN_ERR "SMC GPIO: In %s Can't request irq\n", __func__);
            return ret;
        }
	}

    return 0;
}

void smc_gpio_unset_card_detect(struct smc_device *dev)
{    
    struct smartcard_private *tp = NULL;


    BUG_ON(NULL == dev || NULL == dev->priv);

    tp = (struct smartcard_private *)dev->priv;
    
    if (tp->use_gpio_cd)
    {
        smc_debug(KERN_INFO "SMC MISC: In %s Release irq %d\n", 
                      __func__, dev->gpio_irq);
        free_irq(dev->gpio_irq, dev);
        dev->gpio_irq = 0;
    }
}


 