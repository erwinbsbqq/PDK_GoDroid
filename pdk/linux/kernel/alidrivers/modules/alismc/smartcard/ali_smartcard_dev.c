/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: ali_smartcard_conax.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/
#include <linux/slab.h>
#if defined(CONFIG_ALI_CHIP_M3921)
#include <ali_interrupt.h>
#else
#include <asm/mach-ali/m36_irq.h>
#endif
#include <ali_cache.h>
#include <ali_reg.h>
#include "ali_smartcard_dev.h"
#include "ali_smartcard_config.h"


/* Smart card controller base address */
#define SMC_DEV_BASE_DEF    0x18018800
#define SMC_DEV_BASE_0      0x18001800
#define SMC_DEV_PWM_ADDR_0  0x18001430
#define SMC_DEV_PWM_ADDR_1  0x18001a00
#define SMC_DEV_PWM_ADDR_2  0x18000000

/* ALi SMC device */
static struct smc_device ali_smc_dev[SMC_DEV_NUM] = 
{
#if SMC_DEV_NUM >= 1
    {
        .dev_name = "ali_smc_0", 
        .reg_base = SMC_DEV_BASE_DEF,
        .io_base = 0,
		#if defined(CONFIG_ALI_CHIP_M3921)
        .irq = INT_ALI_SCR1, 
        #else
        .irq = M36_SYS_IRQ_BASE + 20, 
        #endif
        .pwm_addr = SMC_DEV_PWM_ADDR_2, 
        .hsr = /* smc_dev_hsr_0 */ NULL, 
        .lsr = /* smc_dev_hsr_0 */ NULL, 
        .in_use = 0, 
        .priv = NULL, 
        .smc_device_node = NULL,
        .dev_id = 0,
        .rst_addr = SMC_DEV_RST_ADDR_0,
        .wq_flags = 0,
    }
#endif
#if SMC_DEV_NUM >= 2
    ,
    {
        .dev_name = "ali_smc_1", 
        .reg_base = SMC_DEV_BASE_DEF + 0x100,
        .io_base = 0,
        #if defined(CONFIG_ALI_CHIP_M3921)
        .irq = INT_ALI_SCR2, 
        #else
        .irq = M36_SYS_IRQ_BASE + 21, 
        #endif
        .pwm_addr = SMC_DEV_PWM_ADDR_2,
        .hsr = /* smc_dev_hsr_1 */ NULL, 
        .lsr = /* smc_dev_hsr_1 */ NULL, 
        .in_use = 0, 
        .priv = NULL, 
        .smc_device_node = NULL, 
        .dev_id = 1,
        .rst_addr = SMC_DEV_RST_ADDR_0,
        .wq_flags = 0,
    }
#endif
};

/* Definition for board class selection */
#define SMC_BOARD_CLASS_NUM 3

/* SMC device class name */
#define SMC_CLASS_NAME "ali_smc"
struct smc_class
{
    dev_t   dev_no;
    char    *class_name;
    int     dev_count;
    struct class *smc_class;
};

static struct smc_class ali_smc_class =
{
    .class_name = SMC_CLASS_NAME,
    .dev_count = SMC_DEV_NUM,
    .smc_class = NULL,
};

 /* Device private data config functions */
static void smc_dev_priv_init(struct smartcard_private *tp)
{
    BUG_ON(tp == NULL);
    tp->scr_sys_clk = 0;
    tp->pwm_sys_clk = 0;
    tp->smc_chip_id = 0xffffffff;
    tp->smc_chip_version = 0xffffffff;
    tp->smc_tx_fifo_size = 64;
    tp->smc_rx_fifo_size = 64;

	tp->inserted = 0;
	tp->reseted = 0;
	tp->inverse_convention = 0;
	tp->the_last_send = 0;
}

static void smc_dev_binding_set(struct smartcard_private *tp)
{
    BUG_ON(tp == NULL);
    tp->smc_chip_id = ali_sys_ic_get_chip_id();
	tp->smc_chip_version = sys_ic_get_rev_id();


	if (SYS_IC_BONDING_TYPE_4(tp))
	{
		tp->smc_chip_id = ALI_M3101;
	}
    
    if (SYS_IC_BONDING_TYPE_3(tp))
    {
        tp->smc_chip_id = ALI_S3602F;
    }	

	if (SYS_IC_BONDING_TYPE_5(tp))
	{
		tp->smc_tx_fifo_size = 8;
		tp->smc_rx_fifo_size = 32;
	}
    
	if (SYS_IC_BONDING_TYPE_2(tp))
	{
        tp->smc_tx_fifo_size = 256;
        tp->smc_rx_fifo_size = 256;
	}

}

static inline void smc_dev_priv_clk_config(struct smartcard_private *tp, 
                                              struct smc_dev_cfg *config_param)
{
    if (config_param->init_clk_trigger &&
		config_param->init_clk_number &&
		config_param->init_clk_array)
	{
		tp->init_clk_number = config_param->init_clk_number;
        if (NULL == tp->init_clk_array)
		    tp->init_clk_array = (UINT32 *)kmalloc(tp->init_clk_number * sizeof(UINT32), \
		                                           GFP_KERNEL);
		BUG_ON(NULL == tp->init_clk_array);
		memcpy(tp->init_clk_array, config_param->init_clk_array, 
               (tp->init_clk_number * sizeof(UINT32)));
	}
	else
	{
		tp->init_clk_number = 1;
		tp->init_clk_array = &(tp->smc_clock);
		tp->smc_clock = DFT_WORK_CLK;
	}

    tp->sys_clk_trigger = config_param->sys_clk_trigger;
    tp->smc_sys_clk = config_param->smc_sys_clk;
}

/* This won't be work when initialization, we'll have an IO command */
static inline void smc_dev_priv_boardsel_config(struct smartcard_private *tp, 
                                                     struct smc_dev_cfg *config_param)
{
	if (config_param->class_selection_supported &&
		(config_param->board_supported_class & 0x7) &&
		(NULL != config_param->class_select))
	{
		tp->class_selection_supported = 1;
		tp->board_supported_class = (config_param->board_supported_class & 0x7);
		tp->class_select = config_param->class_select;
	}
}

static inline void smc_dev_priv_trigger_config(struct smartcard_private *tp, 
                                                  struct smc_dev_cfg *config_param)
{
    if (config_param->gpio_cd_trigger)
	{
		tp->use_gpio_cd = 1;
		tp->gpio_cd_io = config_param->gpio_cd_io;
		tp->gpio_cd_pol = config_param->gpio_cd_pol;
		tp->gpio_cd_pos = config_param->gpio_cd_pos;
	}
    
	if (config_param->gpio_vpp_trigger)
	{
		tp->use_gpio_vpp = 1;
		if (SYS_IC_BONDING_TYPE_1(tp))
		{
			tp->internal_ctrl_vpp = 1;
		}
		tp->gpio_vpp_io = config_param->gpio_vpp_io;
		tp->gpio_vpp_pol = config_param->gpio_vpp_pol;
		tp->gpio_vpp_pos = config_param->gpio_vpp_pos;
	}
    
	if (config_param->def_etu_trigger)
	{
		tp->use_default_etu = 1;
		tp->default_etu = config_param->default_etu;
	}
       else
       {
            tp->use_default_etu = 0;
       }

    tp->parity_disable = config_param->parity_disable_trigger;
	tp->parity_odd = config_param->parity_odd_trigger;
	tp->apd_disable = config_param->apd_disable_trigger;
	tp->warm_reset_enable = config_param->warm_reset_trigger;
	tp->disable_pps = config_param->disable_pps;
}

static inline void smc_dev_priv_hwtxrx_config(struct smartcard_private *tp, 
                                                  struct smc_dev_cfg *config_param)
{
	if (SYS_IC_BONDING_TYPE_6(tp))
	{
		if (SYS_IC_BONDING_TYPE_9(tp))
			tp->auto_tx_rx_triger = 1;
		else
			tp->force_tx_rx_triger = 1;
		
		if (SYS_IC_BONDING_TYPE_10(tp))
			tp->ts_auto_detect = 1;
		
		if (SYS_IC_BONDING_TYPE_11(tp))
		{
			tp->invert_power = config_param->invert_power;
			tp->invert_detect = config_param->invert_detect;
		}
	}
    else if (SYS_IC_BONDING_TYPE_7(tp))
    {
        tp->invert_power = config_param->invert_power;
		tp->invert_detect = config_param->invert_detect;
		tp->auto_tx_rx_triger = 1;
		tp->ts_auto_detect = 1;
		
		tp->open_drain_supported = config_param->en_power_open_drain;
		tp->open_drain_supported |= config_param->en_clk_open_drain;
		tp->open_drain_supported |= config_param->en_data_open_drain;
		tp->open_drain_supported |= config_param->en_rst_open_drain;	
		if(tp->open_drain_supported)
		{
			tp->en_power_open_drain = config_param->en_power_open_drain;
			tp->en_clk_open_drain = config_param->en_clk_open_drain;
			tp->en_data_open_drain = config_param->en_data_open_drain;
			tp->en_rst_open_drain = config_param->en_rst_open_drain;
		}
    }
    
	if (SYS_IC_BONDING_TYPE_8(tp))
	{
		if (SYS_IC_BONDING_TYPE_9(tp))
		{
			tp->auto_tx_rx_triger = 1;
			tp->invert_power = config_param->invert_power;
			tp->invert_detect = config_param->invert_detect;
			if (SYS_IC_BONDING_TYPE_12(tp))
				 tp->ts_auto_detect = 1;
		}
		else
			tp->force_tx_rx_triger = 1;
	}
	if (SYS_IC_BONDING_TYPE_5(tp))
	{
		if (SYS_IC_BONDING_TYPE_9(tp))
		{
			tp->auto_tx_rx_triger = 1;
		}
	}
	
}

static int smc_dev_priv_config(struct smartcard_private *tp, 
                      struct smc_dev_cfg *config_param)
{    
    BUG_ON(NULL == tp);
    BUG_ON(NULL == config_param);

    smc_dev_priv_clk_config(tp, config_param);
    smc_dev_priv_boardsel_config(tp, config_param);    
    smc_dev_priv_trigger_config(tp, config_param);
    smc_dev_priv_hwtxrx_config(tp, config_param);
    
    return 0;
}

static int __smc_dev_priv_request(struct smc_device *dev)
{
    int ret = -ENOMEM;
    struct smartcard_private *tp = NULL;

    /* The clock buffer will be malloc when config */
    tp = (struct smartcard_private *)kmalloc(sizeof(struct smartcard_private), GFP_KERNEL);
    if (NULL == tp)
        goto out1;
    memset(tp, 0x00, sizeof(struct smartcard_private));

    tp->atr = smc_atr_alloc(tp);
    //if (NULL == tp->atr)
        //goto out2;

    if ((ret = smc_txrx_buf_alloc(tp)) < 0)
        goto out3;

    dev->priv = tp;
    return 0;

out3:
    kfree(tp->atr);
//out2:
    //kfree(tp);
out1: 
    return ret;
}
static void __smc_dev_priv_free(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    if (NULL == tp)
        return;
    
    if ((1 != tp->init_clk_number) && \
        (NULL != tp->init_clk_array))
    {
        kfree(tp->init_clk_array);
        tp->init_clk_array = NULL;
    }

    if (NULL != tp->atr)
    {
        smc_atr_free(tp);
        tp->atr = NULL;
    }

    smc_txrx_buf_free(tp);
    tp->T1= NULL;

    kfree(tp);
    dev->priv = NULL;
}

/* Device parameters config functions */
static inline void smc_dev_clkcfg_group_one(struct smartcard_private *tp)
{
    if (!tp->scr_sys_clk)
	{
		UINT32 tmp;
		tmp = (__REG32ALI(0x18000070) >> 2) & 0x3;
		if (tmp==0x0)
 			tp->pwm_sys_clk = 135000000;
 		else if (tmp==0x1)
 			tp->pwm_sys_clk = 120000000;
 		else if (tmp==0x2)
 			tp->pwm_sys_clk = 166000000;
 		else
 			tp->pwm_sys_clk = 154000000;		
		tp->scr_sys_clk = 108000000;
	}
}

static inline void smc_dev_clkcfg_group_two(struct smartcard_private *tp)
{
    if (!tp->scr_sys_clk)
	{
		UINT32 tmp;
		if (tp->sys_clk_trigger)
		{
			tp->scr_sys_clk = tp->smc_sys_clk;
			if (tp->scr_sys_clk == 166000000)
 				tmp = 0x3;
 			else if (tp->scr_sys_clk == 135000000)
 				tmp = 0x1;
 			else if (tp->scr_sys_clk == 154000000)
 				tmp = 0x2;
 			else
 			{
 				tmp = 0x0;	
 				tp->scr_sys_clk = 108000000;
 			}
			*((volatile UINT8 *)0x1800007a) &= ~(0x3<<1);
			__REG8ALI(0x1800007a) |= tmp<<1;
		}
		else
		{
			tmp = (__REG32ALI(0x18000078) >> 17) & 0x3;
 			if (tmp==0x0)
 				tp->scr_sys_clk = 108000000;
 			else if (tmp==0x1)
 				tp->scr_sys_clk = 135000000;
 			else if (tmp==0x2)
 				tp->scr_sys_clk = 154000000;
 			else
 				tp->scr_sys_clk = 166000000;
		}
		tp->pwm_sys_clk = tp->scr_sys_clk;
	}
}

static inline void smc_dev_clkcfg_group_three(struct smartcard_private *tp)
{
    if (!tp->scr_sys_clk)
	{
		tp->scr_sys_clk = 108000000;
		tp->pwm_sys_clk = 108000000;
	}
}

static inline void smc_dev_hwcfg_group_one(struct smc_device *dev, int dev_id)
{
    dev->reg_base = SMC_DEV_BASE_0;
	dev->irq = 20;
	dev->priv->scr_sys_clk = 108000000;
    dev->priv->pwm_sys_clk = 108000000;    
}

static inline void smc_dev_hwcfg_group_two(struct smc_device *dev, int dev_id)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    dev->reg_base = SMC_DEV_BASE_0 + 0x100 * dev_id;
	dev->irq = 14 + dev_id;
	dev->pwm_addr = SMC_DEV_PWM_ADDR_0 + 0x30 * dev_id;
	dev->pwm_sel_ofst = 0x0;
	dev->pwm_seh_ofst = 0x2;
	dev->pwm_gpio_ofst = 0x4;
	dev->pwm_cfg_ofst = 0x5;
	dev->pwm_frac_ofst = 0x6;	
	
	smc_dev_clkcfg_group_one(tp);
}

static inline void smc_dev_hwcfg_group_three(struct smc_device *dev, int dev_id)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    dev->reg_base = SMC_DEV_BASE_0 + 0x100 * dev_id;
	dev->irq = 20 + dev_id;
	dev->pwm_addr = SMC_DEV_PWM_ADDR_1 + 0x100 * dev_id;
	dev->pwm_sel_ofst = 0x0;
	dev->pwm_seh_ofst = 0x2;
	dev->pwm_gpio_ofst = 0x6;
	dev->pwm_cfg_ofst = 0x4;
	dev->pwm_frac_ofst = 0x5;
	
	smc_dev_clkcfg_group_two(tp);
}

static inline void smc_dev_hwcfg_iospace(struct smc_device *dev, int dev_id)
{
    /* The I/O memory should be remapped  */       	
    
    dev->io_base = (UINT32)ioremap(ali_smc_dev[dev_id].reg_base, REG_MAX_LIMIT);
    dev->pwm_addr &= 0x0fffffff;
    dev->pwm_addr |= 0x10000000;

    smc_debug("[ %s %d ], dev->io_base = 0x%lx, ali_smc_dev[%d].reg_base = 0x%lx\n", 
    	__FUNCTION__,__LINE__, dev->io_base, dev_id, ali_smc_dev[dev_id].reg_base);    
}

void smc_dev_hwcfg(struct smartcard_private *tp, int dev_id)
{
    BUG_ON(NULL == tp);
    BUG_ON(dev_id >= SMC_DEV_NUM);

    ali_smc_dev[dev_id].priv = tp;

    if (SYS_IC_BONDING_TYPE_6(tp))
	{
		if (SYS_IC_BONDING_TYPE_11(tp))
		{
			smc_dev_hwcfg_group_one(&ali_smc_dev[dev_id], dev_id);
		}
		else
		{
			smc_dev_hwcfg_group_two(&ali_smc_dev[dev_id], dev_id);
		}
	}
    
	if (1 == SYS_IC_BONDING_TYPE_8(tp))
	{
		smc_dev_hwcfg_group_three(&ali_smc_dev[dev_id], dev_id);
	}
    
	if (SYS_IC_BONDING_TYPE_13(tp))
	{
		smc_dev_clkcfg_group_three(tp);
	}
    
	if (SYS_IC_BONDING_TYPE_14(tp))
	{
		smc_dev_hwcfg_group_one(&ali_smc_dev[dev_id], dev_id);
	}
	
    smc_dev_hwcfg_iospace(&ali_smc_dev[dev_id], dev_id);    
}

int smc_dev_config(struct smc_device *dev, struct smc_dev_cfg *cfg)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;


    smc_dev_priv_init(tp);
    smc_dev_binding_set(tp);

    smc_dev_priv_config(tp, cfg);
    smc_dev_hwcfg(tp, dev->dev_id);    

    /* Set VPP GPIO attribute */
    smc_gpio_set_vpp(tp);    
	
    /* Set T1 protocol */
    smc_t1_get(tp);    
    
    /* Hardware configuration */
    smc_misc_dev_deactive(dev);   
    
    smc_misc_init_hw(dev);
    if ((ret = smc_gpio_set_card_detect(dev)) < 0)
    {
    	smc_debug("[ %s %d ], error!\n", __FUNCTION__,__LINE__);
        return ret;
    }

    smc_misc_is_card_insert(dev);      
	
    memcpy(&config_param_def, cfg, sizeof(struct smc_dev_cfg));  
	
    smc_gpio_unset_card_detect(dev);
	
    return ret;
}

void smc_dev_unconfig(struct smc_device *dev)
{    
    smc_gpio_unset_card_detect(dev);
}

int smc_dev_create(int dev_id, dev_t dev_no)
{
    long result;

	ali_smc_dev[dev_id].smc_device_node = device_create(ali_smc_class.smc_class, 
                                                        NULL, dev_no, 
                                                        &ali_smc_dev[dev_id], 
                                                        ali_smc_dev[dev_id].dev_name);
    if (IS_ERR(ali_smc_dev[dev_id].smc_device_node))
    {
		smc_debug(KERN_ERR "SMC DEV: In %s Device create failed!\n", __func__);
		result = PTR_ERR(ali_smc_dev[dev_id].smc_device_node);
		return -EINVAL;  
	}

    init_waitqueue_head(&ali_smc_dev[dev_id].smc_wq_head);
    spin_lock_init(&ali_smc_dev[dev_id].smc_spinlock);
    spin_lock_init(&ali_smc_dev[dev_id].smc_status_spinlock);   
    spin_lock_init(&ali_smc_dev[dev_id].smc_irq_spinlock);
    sema_init(&ali_smc_dev[dev_id].smc_semaphore, 1);
    
	return 0;  
}

struct smc_device *smc_dev_get(int dev_id)
{
    BUG_ON(dev_id >= SMC_DEV_NUM);
    return &ali_smc_dev[dev_id];
}

int smc_dev_register(int dev_id, dev_t dev_no, 
                        struct file_operations *f_ops)
{
    int ret = 0;

    cdev_init(&ali_smc_dev[dev_id].smc_cdev, f_ops);
	ali_smc_dev[dev_id].smc_cdev.owner=THIS_MODULE;
	ali_smc_dev[dev_id].smc_cdev.ops=f_ops;
	ret=cdev_add(&ali_smc_dev[dev_id].smc_cdev, dev_no, 1);
	if (ret < 0)
	{
		smc_debug(KERN_ERR "SMC DEV: In %s Add device %d error %d\n", 
                      __func__, dev_id, ret);
		return ret;
	}
	
	smc_debug(KERN_INFO "SMC DEV: In %s Register device OK\n", __func__);

    return 0;
}

void smc_dev_unregister(int dev_id)
{
    if (NULL != ali_smc_dev[dev_id].smc_device_node)
    {
        device_del(ali_smc_dev[dev_id].smc_device_node);
        ali_smc_dev[dev_id].smc_device_node = NULL;
    }

    cdev_del(&ali_smc_dev[dev_id].smc_cdev);
}

int smc_dev_priv_request(int dev_id)
{
    if (NULL == ali_smc_dev[dev_id].priv)
        return __smc_dev_priv_request(&ali_smc_dev[dev_id]);
    
    return 0;
}

void smc_dev_priv_free(int dev_id)
{
    if (NULL != ali_smc_dev[dev_id].priv)
        __smc_dev_priv_free(&ali_smc_dev[dev_id]);
}


void smc_dev_workqueue_request(int dev_id)
{
    ali_smc_dev[dev_id].smc_notification_workqueue = create_workqueue(ali_smc_dev[dev_id].dev_name);
    if (NULL == ali_smc_dev[dev_id].smc_notification_workqueue)
    {
        smc_debug(KERN_ERR "SMC DEV: In %s Warning - Work queue create fail!\n", __func__);
    }
}

void smc_dev_workqueue_release(int dev_id)
{
    if (NULL != ali_smc_dev[dev_id].smc_notification_workqueue)
    {
        destroy_workqueue(ali_smc_dev[dev_id].smc_notification_workqueue);
        ali_smc_dev[dev_id].smc_notification_workqueue = NULL;
    }
}

void smc_dev_mutex_request(int dev_id)
{
    mutex_init(&ali_smc_dev[dev_id].smc_mutex);
    mutex_init(&ali_smc_dev[dev_id].smc_iso_mutex);
    mutex_init(&ali_smc_dev[dev_id].smc_irq_mutex);
}

void smc_dev_mutex_release(int dev_id)
{
    mutex_destroy(&ali_smc_dev[dev_id].smc_irq_mutex);
    mutex_destroy(&ali_smc_dev[dev_id].smc_iso_mutex);
    mutex_destroy(&ali_smc_dev[dev_id].smc_mutex);    
}

int smc_dev_class_create(dev_t *dev_no)
{
    int ret= 0;
    long result;


    ret = alloc_chrdev_region(&ali_smc_class.dev_no, 
                              0, 
                              ali_smc_class.dev_count, 
                              ali_smc_class.class_name);
    if (ret < 0)
    {
        smc_debug(KERN_ERR "SMC DEV: In %s Alloc device region error %d\n", 
                      __func__, ret);
        return ret;
    }

    ali_smc_class.smc_class = class_create(THIS_MODULE, "ali_smc_class");

	if (IS_ERR(ali_smc_class.smc_class))
	{
        smc_debug(KERN_ERR "SMC DEV: In %s Class create failed!\n", __func__);
		result = PTR_ERR(ali_smc_class.smc_class);
        /* Release the major number */
        unregister_chrdev_region(ali_smc_class.dev_no, ali_smc_class.dev_count);
		return -EINVAL;
	}

    /* Do we need the class->pm_ops ? */
    ali_smc_class.smc_class->suspend = smc_pm_suspend;
    ali_smc_class.smc_class->resume = smc_pm_resume;
    
    *dev_no = ali_smc_class.dev_no;


    return 0;
}

void smc_dev_class_delete(void)
{
    if (NULL != ali_smc_class.smc_class)
    {
        class_destroy(ali_smc_class.smc_class);
        ali_smc_class.smc_class = NULL;
    }

    unregister_chrdev_region(ali_smc_class.dev_no, ali_smc_class.dev_count);
}

static inline void smc_dev_set_time_unit(struct smartcard_private *tp, void __iomem *p)
{
    if (tp->use_default_etu)
    {
		tp->smc_etu = tp->default_etu;
    }
	else
	{
		tp->smc_etu = DFT_WORK_ETU;
	}

    writew(tp->smc_etu, p + REG_ETU0);
    writew(12, p + REG_GTR0);
    writel(12800, p + REG_CBWTR0);
}

static inline void smc_dev_config_reset_param(struct smartcard_private *tp, void __iomem *p)
{
    if (1 == tp->reseted)
	{
		tp->reseted = 0;
		if (1 == tp->warm_reset_enable)
			tp->warm_reset = 1;
	}
	else
		tp->warm_reset = 0;
    
	smc_debug(KERN_INFO "SMC DEV: In %s Warm reset %ld\n", __func__, tp->warm_reset);
    
	tp->isr0_interrupt_status = 0;
	tp->isr1_interrupt_status = 0;
    
    writeb(readb(p + REG_IER0) & (~SMC_IER0_TRANS_FIFO_EMPY), p + REG_IER0);
}

static inline void smc_dev_reset_default(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;    
    
    if (!tp->warm_reset)
	{
		tp->smc_clock = tp->init_clk_array[tp->init_clk_idx];
		smc_debug(KERN_INFO "SMC DEV: In %s Init clock %ld, No. %ld\n", 
                      __func__, tp->smc_clock, tp->init_clk_idx);
		smc_misc_set_wclk(dev, tp->smc_clock);
		msleep(1);
		smc_misc_dev_deactive(dev);
		msleep(1);
	}
	else
	{
		msleep(1);
		smc_misc_warm_reset(dev);
		msleep(1);
	}
}

#define SMC_JUST_DO_RESET(dev, tp) \
    if (!tp->warm_reset) \
	{ \
		smc_misc_dev_deactive(dev); \
		msleep(1); \
	} \
	else \
	{ \
		smc_misc_warm_reset(dev); \
		msleep(1); \
	}

static inline int smc_dev_reset_with_default_etu(struct smc_device *dev)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;    
    
    if ((ret = smc_atr_get(dev)) < 0)
	{
		if (0 == tp->inverse_convention && 0 == tp->ts_auto_detect)
		{
			SMC_JUST_DO_RESET(dev, tp)
            
			tp->inverse_convention = 1;
			if ((ret = smc_atr_get(dev)) < 0)
			{
				tp->inverse_convention = 0;
			}
		}
	}

    return ret;
}

static inline int smc_dev_card_etu_prepare(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv; 
    void __iomem *p = (void __iomem *)dev->io_base;
    int ret = 0;
    
    writeb(SMC_SCR_CTRL_OP|SMC_SCR_CTRL_RECV|(tp->parity_odd<<4), p + REG_SCR_CTRL);
	udelay(200);
	
	writeb(0xFF, p + REG_ISR0);
	writeb(0xFF, p + REG_ISR1);
	
	writeb(SMC_FIFO_CTRL_EN | SMC_FIFO_CTRL_TX_OP | SMC_FIFO_CTRL_RX_OP, p + REG_FIFO_CTRL);                                         
	smc_write_rx(tp, p, 32);
    
	local_irq_disable();
	if (!tp->inserted)
	{
		ret = -EIO;
        goto out;
	}
    
	if (tp->warm_reset)
	{
		if (tp->apd_disable)
			writeb(readb(p + REG_ICCR) | SMC_RB_ICCR_PRT_EN, p + REG_ICCR);
		else
			writeb(readb(p + REG_ICCR) | SMC_RB_ICCR_PRT_EN | 0x40, p + REG_ICCR);
	}
	else	
	{
		if (tp->apd_disable)
			writeb(SMC_RB_ICCR_PRT_EN, p + REG_ICCR);
		else	
			writeb(SMC_RB_ICCR_PRT_EN | 0x40, p + REG_ICCR);
        
		mdelay((ATV_VCC2IO>>1)/1000);
        
		if (tp->use_gpio_vpp)
		{
			if (tp->internal_ctrl_vpp)
				writeb(readb(p + REG_CLK_VPP) | SMC_RB_CTRL_VPP, p + REG_CLK_VPP);
			else
				ali_gpio_direction_output(tp->gpio_vpp_pos, tp->gpio_vpp_pol);
		}
        
		mdelay((ATV_VCC2IO>>1)/1000);
		writeb(readb(p + REG_ICCR) | SMC_RB_ICCR_DIO, p + REG_ICCR);
	}
    
out:
    local_irq_enable();
    return ret;
}

static inline int __smc_dev_get_card_etu(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv; 
    void __iomem *p = (void __iomem *)dev->io_base;
    UINT8 u8_etu_trigger1 = 0;
	UINT8 u8_etu_trigger2 = 0;
    UINT32 u32_old_etu_tick1 = 0, u32_old_etu_tick2 = 0;
    UINT8 cc = 0;

    for ( ; ; )
	{
		if (tp->isr1_interrupt_status & SMC_ISR1_RST_LOW)
		{
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_LOW);
			if (SYS_IC_BONDING_TYPE_5(tp))
			{
				if (0 != smc_read_rx(tp, p))
					break;
			}
			else
				break;
		}
        
		if (tp->isr1_interrupt_status & SMC_ISR1_COUNT_ST)
		{
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
			tp->isr1_interrupt_status &= (~SMC_ISR1_COUNT_ST);
			
			writeb(readb(p + REG_ISR1) | SMC_ISR1_RST_NATR, p + REG_ISR1);
			writeb(readb(p + REG_ICCR) | SMC_RB_ICCR_RST, p + REG_ICCR);
            
			while (0 == smc_read_rx(tp, p))
			{			
				if ((tp->isr1_interrupt_status & SMC_ISR1_RST_NATR) && \
                    (0 == u8_etu_trigger1))
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
					u8_etu_trigger1 = 1;
					u32_old_etu_tick1 = jiffies;
				}
                
				if (u8_etu_trigger1)
				{
					if((jiffies - u32_old_etu_tick1) > 100)
					{
						return -ETIMEDOUT;
					}
				}
                
				if (0 == smc_misc_card_exist(dev))
				{
					return -EIO;
				}
			}
            
			cc = readb(p + REG_RBR);
			if (0x3b == cc)
			{
				tp->inverse_convention = 0;
			}
			else
			{
				tp->inverse_convention = 1;
			}
            
			for ( ; ; )
			{
				if (tp->isr1_interrupt_status & SMC_ISR1_RST_HIGH)
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_HIGH);
					break;
				}
				
				if ((tp->isr1_interrupt_status & SMC_ISR1_RST_NATR) && \
                    (0 == u8_etu_trigger2))
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
					u8_etu_trigger2 = 1;
					u32_old_etu_tick2 = jiffies;
				}
                
				if (u8_etu_trigger2)
				{
					if ((jiffies - u32_old_etu_tick2) > 100)
						return -ETIMEDOUT;
				}
				
				if (0 == smc_misc_card_exist(dev))
					return -EIO;
			}
			break;
		}
        
		if (0 == smc_misc_card_exist(dev)) 
            return -EIO;
	}
    
    return 0;
}

static inline int smc_dev_get_card_etu_internal(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv; 
    void __iomem *p = (void __iomem *)dev->io_base;
    UINT32 t_wait_atr_tmo = 0, t_wait_atr_time = 0;
    UINT16 i_rx_cnt =0;

    t_wait_atr_tmo =  (9600 * 372 * 2) / (tp->smc_clock / 1000);
	t_wait_atr_time = jiffies;
	i_rx_cnt = smc_read_rx(tp, p); 
    
	while (SMC_ISR0_BYTE_RECV != (tp->isr0_interrupt_status & SMC_ISR0_BYTE_RECV))
	{
		if (0 == smc_misc_card_exist(dev)) 
            return -EIO;
        
		if (i_rx_cnt == smc_read_rx(tp, p))
		{
			msleep(1);
			t_wait_atr_time = jiffies - t_wait_atr_time;
			if (t_wait_atr_tmo >= t_wait_atr_time)
				t_wait_atr_tmo -= t_wait_atr_time;
			else
			{
				return -ETIMEDOUT;
			}	
		}
		else
		{
			i_rx_cnt = smc_read_rx(tp, p);
			t_wait_atr_tmo =  (9600 * 372 * 2) / (tp->smc_clock / 1000);
		}
		t_wait_atr_time = jiffies;
	}

    return 0;
}

static inline int smc_dev_card_etu_post(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv; 
    void __iomem *p = (void __iomem *)dev->io_base;
    UINT32 u32_etu = 0, u32_etu3 = 0;

	writeb(readb(p + REG_SCR_CTRL) & (~SMC_SCR_CTRL_RECV), p + REG_SCR_CTRL);
	tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
	
	u32_etu = readw(p + REG_RCNT_ETU);

	u32_etu3 = (readw(p + REG_RCNT_3ETU) / 3 - 15);
	u32_etu = (u32_etu3 < DFT_WORK_ETU) ? u32_etu : u32_etu3;

	tp->smc_etu = u32_etu;
	writew(u32_etu, p + REG_ETU0);

    return 0;
}

static inline int smc_dev_get_card_etu(struct smc_device *dev)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv; 
    void __iomem *p = (void __iomem *)dev->io_base;

	if ((ret = smc_dev_card_etu_prepare(dev)) < 0)
        return ret;
	
	if (!tp->warm_reset)
	{
		udelay(ATV_IO2CLK);
		writeb(readb(p + REG_ICCR) | SMC_RB_ICCR_CLK, p + REG_ICCR);
		udelay(200);
	}
	
    if ((ret = __smc_dev_get_card_etu(dev)) < 0)
        return ret;
    if ((ret = smc_dev_get_card_etu_internal(dev)) < 0)
        return ret;

    ret = smc_dev_card_etu_post(dev);
	
    return ret;
}

static inline int smc_dev_reset_with_card_etu(struct smc_device *dev)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;    

    if ((ret = smc_dev_get_card_etu(dev)) < 0)
        smc_debug(KERN_ERR "SMC DEV: In %s Reset failure\n", __func__);
    else        
	{
		SMC_JUST_DO_RESET(dev, tp)          
		ret = smc_atr_get(dev);
	}

    return ret;
}

static inline int smc_dev_reset_conditional(struct smc_device *dev)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;    

    if (tp->use_default_etu)
	{
		ret = smc_dev_reset_with_default_etu(dev);
	}
	else
	{
		ret = smc_dev_reset_with_card_etu(dev);
	}

    return ret;
}

#define SMC_CARD_INSERTED_STATUS(tp, ret, err_tag)  if (!tp->inserted) \
    { \
        smc_debug(KERN_INFO "SMC DEV: In %s No smart card inserted\n", __func__); \
        ret = -EINVAL; \
        goto err_tag; \
    }



static int __smc_dev_reset(struct smc_device *dev)
{   
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    int ret = 0;
    
    dev->smc_reset_result = 0;
    
    while (tp->init_clk_idx < tp->init_clk_number)
    {
        SMC_CARD_INSERTED_STATUS(tp, ret, out);

        smc_dev_reset_default(dev);        
        ret = smc_dev_reset_conditional(dev);

        if (ret < 0)
		{
			if (!tp->warm_reset)
				tp->init_clk_idx++;
			else
				break;
		}
		else
			break;
    }

out:
    dev->smc_reset_result = ret;
    dev->b_reset_over = 1;   
    return ret;
}




static int __smc_dev_atr_config(struct smc_device *dev)
{    
    int ret = 0;

    dev->smc_atr_cfg_result = 0;
    ret = smc_atr_info_config(dev);

    dev->smc_atr_cfg_result = ret;
    dev->b_atr_cfg_over = 1;   
    
    return 0;
}


#define SMC_DEV_SCHEDULE_WORK_TMO(dev, work, func, tmo, ret_condition, ret, err_tag) \
    do { \
        unsigned long ret_wq = 0; \
        INIT_WORK(&work, (void *)func); \
        if (NULL != dev->smc_notification_workqueue)  \
            queue_work(dev->smc_notification_workqueue, &work); \
        ret_condition = 0; \
        ret_wq = wait_event_timeout(dev->smc_wq_head, ret_condition, tmo); \
    	if (0 == ret_wq)  \
    	{  \
            ret = -ETIMEDOUT;  \
            goto err_tag; \
    	} \
    } while (0)


int smc_dev_reset(struct smc_device *dev)
{    	    
    struct smartcard_private *tp = NULL;
    struct smartcard_atr *atr = NULL;
    atr_t *p_atr_info = NULL;
    void __iomem *p = NULL;
    int ret = 0;    
    u_long u32_time = 0;        


    BUG_ON(NULL == dev);
    BUG_ON(NULL == dev->priv);

    tp = (struct smartcard_private *)dev->priv;
    atr = (struct smartcard_atr *)tp->atr;
    p_atr_info = (atr_t *)atr->atr_info;
    p = (void __iomem *)dev->io_base;


    down(&dev->smc_semaphore);
    smc_t1_init(tp->T1);    
    smc_txrx_buf_reset(tp);   
    smc_dev_set_time_unit(tp, p);
    SMC_CARD_INSERTED_STATUS(tp, ret, out);
    smc_dev_config_reset_param(tp, p);    

    tp->init_clk_idx = 0;
	atr->atr_rlt = SMC_ATR_NONE;
    
    u32_time = jiffies;


    __smc_dev_reset(dev);
    if ((ret = dev->smc_reset_result) < 0)
        goto out;

    smc_debug("SMC: In %s Device reset time %ld\n", __func__, jiffies - u32_time);

    if (ret < 0)
	{
        smc_debug(KERN_ERR "SMC DEV: In %s Smart card reset error.\n", __func__);
		smc_misc_dev_deactive(dev);
	    goto out;
	}
    
	memset(p_atr_info, 0x00, sizeof(atr_t));
    
    u32_time = jiffies;


    __smc_dev_atr_config(dev);
    if ((ret = dev->smc_atr_cfg_result) < 0)
        goto out;
    
    smc_debug("SMC: In %s Device reset time %ld\n", __func__, jiffies - u32_time);

out:
    up(&dev->smc_semaphore);
    return ret;
}

/* We have one more type reset function */
int smc_dev_multi_class_reset(struct smc_device *dev)
{
	int i;
	int ret = 0;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    
	if (SMC_CLASS_NONE_SELECT == tp->smc_current_select)
	{
		for (i = 0; i < SMC_BOARD_CLASS_NUM; i++)
		{
			if (tp->board_supported_class & (1<<i))
			{
				tp->smc_current_select = (enum class_selection)(SMC_CLASS_NONE_SELECT + i + 1);
				break;
			}
		}
	}
    
	i = ((INT32)tp->smc_current_select - SMC_CLASS_NONE_SELECT - 1);
	for ( ; i < SMC_BOARD_CLASS_NUM; i++)
	{
		if (tp->board_supported_class & (1<<i))
		{
			tp->smc_current_select = (enum class_selection)(SMC_CLASS_NONE_SELECT + i + 1);
            if (NULL != tp->class_select)
			    tp->class_select(tp->smc_current_select);

			if (!((ret = smc_dev_reset(dev)) < 0))
			{
				if (0 == (tp->smc_supported_class & 0x7))
				{
					break;
				}
				else
				{
					if (tp->smc_supported_class & (1<<i))
					{
						break;
					}
				}
			}
			else
			{
				smc_misc_dev_deactive(dev);
			}
		}
	}
    
	if (i > SMC_BOARD_CLASS_NUM - 1)
	{
        smc_debug(KERN_ERR "SMC DEV: In %s Smart card reset fail.\n", __func__);
		tp->smc_current_select = SMC_CLASS_NONE_SELECT;
		return -EINVAL;
	}
	return ret;
}

/* Read function implementation */
ssize_t smc_dev_read(struct smc_device *dev, char *buf, size_t count)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    void __iomem *p = (void __iomem *)dev->io_base;
    int u32_smc_rd_tmo;
	//UINT32 u32_rd_tick = jiffies;
    ssize_t ret_size = 0;

	mutex_lock(&dev->smc_mutex);	
	if (0 != smc_read_rx(tp, p))
	{
        spin_lock_irq(&dev->smc_spinlock);
        ret_size = smc_txrx_read(tp, p);	
        spin_unlock_irq(&dev->smc_spinlock);
	}

    /* < 0: Error; 0: Timeout; > 0: Right */
    if ((u32_smc_rd_tmo = smc_txrx_data_timeout(tp, count)) < 0)
    {
        ret_size = 0;
        goto out;
    }
	
	spin_lock_irq(&dev->smc_spinlock);
	if (tp->smc_rx_head < ((UINT32)count))
		count = tp->smc_rx_head;		
    memcpy(buf, (const void *)tp->smc_rx_tmp_buf, count);
	ret_size = count;
	tp->smc_rx_head -= (UINT32)count;
    smc_dump("SMC DEV: ", (char *)tp->smc_rx_tmp_buf, ret_size);
    
	if (tp->smc_rx_head)
	{
		memcpy((void *)tp->smc_rx_tmp_buf, (const void *)(tp->smc_rx_tmp_buf + count), tp->smc_rx_head);
		__CACHE_FLUSH_ALI((unsigned long)(tp->smc_rx_tmp_buf), count+tp->smc_rx_head);
	}
	else
	{
		__CACHE_FLUSH_ALI((unsigned long)(tp->smc_rx_tmp_buf), count);
	}
	
	spin_unlock_irq(&dev->smc_spinlock);
    
	if (0 == u32_smc_rd_tmo && 0 == ret_size)
	{
		smc_debug(KERN_ERR "SMC DEV: In %s Read %d bytes\n", 
                      __func__, count);
		goto out;
	}

	smc_debug(KERN_INFO "SMC DEV: In %s Read %d bytes\n", 
                  __func__, ret_size);
out:
    mutex_unlock(&dev->smc_mutex);	
    return ret_size;
}

/* Write function implementation */
ssize_t smc_dev_write(struct smc_device *dev, char *buf, size_t count)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    void __iomem *p = (void __iomem *)dev->io_base;
    //UINT32 u32_wr_tick = jiffies;
	ssize_t ret_size = 0; 
    int ret= 0;

    /* Patch for S3602F */
	mdelay(3);
    /* The lock need to be disable due to the wait queue in smc_txrx_transfer_data */
    mutex_lock(&dev->smc_mutex);
    ret = smc_txrx_transfer_data(dev, p, buf, count, 
                                (UINT8 *)tp->smc_rx_tmp_buf, 0, (UINT16 *)&ret_size);
    if (ret < 0) ret_size = 0;
    mutex_unlock(&dev->smc_mutex);
	smc_debug(KERN_INFO "SMC DEV: In %s Write %d\n", 
                  __func__, ret_size);
    
	return ret_size;
}

/* Set and unset the PIN MUX */
#define FOREACH_SMC_DEV_IN_USE_RETURN \
	do { \
    		int dev_id = 0; \
    		for (dev_id = 0; dev_id < SMC_DEV_NUM; dev_id++) \
    		{ \
    			if (ali_smc_dev[dev_id].in_use) \
				return; \
    		} \
	} while (0);

void smc_dev_set_pin(void)
{
    FOREACH_SMC_DEV_IN_USE_RETURN
    /* We may cause strange error, so we let */
    /* The system to enable the power when card inserted */
    /* smc_misc_power_enable(); */
    smc_misc_set_pin();
}

void smc_dev_unset_pin(void)
{
    FOREACH_SMC_DEV_IN_USE_RETURN
    smc_misc_unset_pin();
    /* We always disable the power after close all smc to low the power */
    /* But we never enable it, because it'll be done by the reset */
    /* smc_misc_power_disable(); */
}



