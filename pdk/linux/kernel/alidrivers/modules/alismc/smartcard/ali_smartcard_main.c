/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_main.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#include <linux/version.h>
#include <linux/slab.h>
#include <asm/uaccess.h> 
#include <ali_pdk_version.h>

#include "ali_smartcard.h"
#include "ali_smartcard_dev.h"
#include "ali_smartcard_gpio.h"
#include "ali_smartcard_txrx.h"
#include "ali_smartcard_t1.h"
#include "ali_smartcard_misc.h"

/* This is the configuration definition for different board */
#include "ali_smartcard_config.h"
#include <linux/debugfs.h>

                         
static struct dentry *g_ali_smc_debugfs_root;
static __u32 g_ali_smc_debug_level = 0;

/******************************************************************************
 * Smart card driver operation function
 ******************************************************************************/
static int ali_smc_open(struct inode *inode, struct file *file);
static int ali_smc_release(struct inode *inode, struct file *file);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_smc_ioctl(struct file *file,
			             unsigned int cmd, unsigned long parg);
#else
static int ali_smc_ioctl(struct inode *inode, struct file *file,
			             unsigned int cmd, unsigned long parg);
#endif
static ssize_t ali_smc_read(struct file *file, char *buf, 
                              size_t count, loff_t *ppos);
static ssize_t ali_smc_write(struct file *file, const char *buf, 
                              size_t count, loff_t *ppos);

static struct file_operations ali_smc_fops = {
	.owner		= THIS_MODULE,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_smc_ioctl,
#else
	.ioctl		= ali_smc_ioctl,
#endif	
	.open		= ali_smc_open,
	.release	= ali_smc_release,
	.read       = ali_smc_read,
	.write      = ali_smc_write,
};


#define SMC_GPIO_BOARD_CLASS_SELECTION 73
#define SMC_GPIO_BOARD_CLASS_VOLTAGE   94


int smc_debug(char *fmt, ...)
{
    va_list args;
    int r = 0;
    
    
    if (0 == g_ali_smc_debug_level)
    {
        return  (-__LINE__);
    }
 
    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);

    return(r);
}

int smc_error(char *fmt, ...)
{
    va_list args;
    int r = 0;    
 
    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);

    return(r);
}


void smc_dump(char *s, char *buf, int size)
{
    int i = 0; 
    
    smc_debug(s);
    smc_debug("In %s Transfer DATA %d bytes\n", __func__, size); 
    for (i = 0; i < size; i++) 
    {
        smc_debug("%02x ", (unsigned char)(buf)[i]); 
    }
    smc_debug("\n");   
}

static void smc_config_board_class_select(enum class_selection smc_class)
{
    int val = 1;

	if (SMC_CLASS_B_SELECT == smc_class)
		val = 0;
	ali_gpio_direction_output(SMC_GPIO_BOARD_CLASS_SELECTION, val);
}

/* Smartcard device attach parameters */
static UINT32 init_clk = 3600000;
struct smc_dev_cfg config_param_def = {
    .init_clk_trigger = 1,
	.init_clk_number = 1,
	.force_tx_rx_trigger = 1,
	.def_etu_trigger = 1,
	.default_etu = 372,
    .warm_reset_trigger = 1,
	.force_tx_rx_cmd = 0xdd,
	.force_tx_rx_cmd_len = 5,
	.invert_detect=0,
	.init_clk_array = &init_clk,
	.class_selection_supported = 1,
	.board_supported_class = BOARD_SUPPORT_CLASS_A | BOARD_SUPPORT_CLASS_B,
	.class_select = smc_config_board_class_select,
};


/* Smart card reader device operation */
static int ali_smc_open(struct inode *inode, struct file *file)
{
    struct smc_device *p_smc_dev = NULL;   
    int ret = 0;

    /* Get the per-device structure that contains this cdev */
    p_smc_dev = container_of(inode->i_cdev, struct smc_device, smc_cdev);
    if (NULL == p_smc_dev || NULL != p_smc_dev->priv)
    {
        smc_debug(KERN_ERR "SMC: In %s, device error %p %p\n", 
                      __func__, p_smc_dev, p_smc_dev->priv);
        return -EINVAL;
    }

    BUG_ON(p_smc_dev->dev_id >= SMC_DEV_NUM || SMC_INVALID_DEV == p_smc_dev->dev_id);

    if (p_smc_dev->in_use)
        return -EBUSY;

	
    preempt_disable();
    smc_dev_set_pin();
    mutex_lock(&p_smc_dev->smc_mutex);
    if ((ret = smc_dev_priv_request(p_smc_dev->dev_id)) < 0)
        goto out1;

   
    if (!request_mem_region(p_smc_dev->reg_base, REG_MAX_LIMIT, p_smc_dev->dev_name))
    {
        ret = -ENOMEM;
        goto out2;
    }      

  
    /* Easy access to smc_dev from rest of the entry points */
    p_smc_dev->in_use = 1;    
    file->private_data = p_smc_dev;

   smc_dev_config(p_smc_dev, &config_param_def);    
   smc_misc_register_irq_server(p_smc_dev);    

    smc_debug(KERN_INFO "SMC: In %s Open successful %d\n", 
                  __func__, p_smc_dev->dev_id);
    goto out;

out2:
    smc_dev_priv_free(p_smc_dev->dev_id);
out1:
    smc_dev_unset_pin();
out:
    mutex_unlock(&p_smc_dev->smc_mutex);
    preempt_enable();

    return ret;
}

static int ali_smc_release(struct inode *inode, struct file *file)
{
    struct smc_device *p_smc_dev = file->private_data;

    BUG_ON(NULL == p_smc_dev);

    smc_debug("SMC: In %s Release device %d\n", \
                  __func__, p_smc_dev->dev_id);

    mutex_lock(&p_smc_dev->smc_mutex);

    	smc_dev_unconfig(p_smc_dev);	
       smc_misc_unregister_irq_server(p_smc_dev);
	smc_misc_dev_deactive(p_smc_dev);  
	
	if (NULL != (void *)p_smc_dev->io_base)
	{
		iounmap((void *)p_smc_dev->io_base);
	}
    release_mem_region(p_smc_dev->reg_base, REG_MAX_LIMIT);    
    
    smc_dev_priv_free(p_smc_dev->dev_id);
    p_smc_dev->in_use = 0;
    p_smc_dev->priv = NULL;
    mutex_unlock(&p_smc_dev->smc_mutex);     
    smc_dev_unset_pin();

    return 0;
}

static ssize_t ali_smc_read(struct file *file, char *buf, 
                              size_t count, loff_t *ppos)
{
    struct smc_device *p_smc_dev = NULL;
    
    unsigned char *p_buf = NULL;
    int ret_size = 0;

	p_smc_dev = file->private_data;
	
    BUG_ON(NULL == p_smc_dev);
    BUG_ON(NULL == p_smc_dev->priv);

	p_buf = (unsigned char *)kmalloc(count, GFP_KERNEL);
	if (NULL != p_buf)
	{	
	    ret_size = smc_dev_read(p_smc_dev, p_buf, count);
	    if (0 != ret_size)
	    {
	        if (0 != copy_to_user(buf, p_buf, ret_size))
	        {
	        	return -EFAULT;
	        }
	    }
	    if (ret_size != count)
	        smc_debug(KERN_WARNING "SMC: In %s Warning - Not enough data read\n", __func__);
	    kfree(p_buf);
	}
    
    return ret_size;
}

static ssize_t ali_smc_write(struct file *file, const char *buf, 
                              size_t count, loff_t *ppos)
{
    struct smc_device *p_smc_dev = NULL;    
    unsigned char *p_buf = NULL;
    int ret = 0;

	p_smc_dev = file->private_data;
	BUG_ON(NULL == p_smc_dev);
    BUG_ON(NULL == p_smc_dev->priv);

    p_buf = (unsigned char *)kmalloc(count, GFP_KERNEL);
    if (NULL != p_buf)
    {
	    if (0 != copy_from_user(p_buf, buf, count))
	    {
	    	return -EFAULT;
	    }
	    ret = smc_dev_write(p_smc_dev, p_buf, count);
	    kfree(p_buf);
    }
    
    return ret;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_smc_ioctl(struct file *file,
			             unsigned int cmd, unsigned long parg)
#else
static int ali_smc_ioctl(struct inode *inode, struct file *file,
			             unsigned int cmd, unsigned long parg)
#endif			             
{
    struct smc_device *p_smc_dev = file->private_data;    
    struct smartcard_private *tp = (struct smartcard_private *)p_smc_dev->priv;
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;    
    void __iomem *p = (void __iomem *)p_smc_dev->io_base;
    int ret = 0;
    unsigned long cmd_param= 0;
    
    
    BUG_ON(NULL == p_smc_dev);
    BUG_ON(NULL == p_smc_dev->priv);
    
    switch (cmd)
    {
        case SMC_CMD_RESET:
        {
            smc_atr_t __user *atr_ret = (smc_atr_t __user *)parg;
            int (*func)(struct smc_device *) = NULL;            
            u_long u32_time = 0;           
           
            	
            func = tp->class_selection_supported ? \
                   smc_dev_multi_class_reset : \
                   smc_dev_reset;
            /* We can't lock the mutex here, 
             * because it may used in smc_dev_reset 
             */
            u32_time = jiffies;

            if (mutex_lock_interruptible(&p_smc_dev->smc_mutex))
				return -EBUSY;

            if ((ret = func(p_smc_dev)) < 0)
            {
                smc_debug(KERN_ERR "SMC: In %s Device reset failure.\n", __func__);
                atr->atr_size = 0;
            }
            
            if (0 != atr->atr_size)
            {
                /* This might sleep */
                if (0 != copy_to_user(atr_ret->atr_buf, atr->atr_buf, atr->atr_size))
                {
                	return -EFAULT;
                }
                smc_dump("SMC ATR: ", atr->atr_buf, atr->atr_size);
            }
            put_user(atr->atr_size, (unsigned short __user *)&(atr_ret->atr_size));
            mutex_unlock(&p_smc_dev->smc_mutex);
            smc_debug("SMC: In %s Device reset time %ld\n", __func__, jiffies - u32_time);
            break;
        }
        case SMC_CMD_SET_TRANSPORT_ID:
            mutex_lock(&p_smc_dev->smc_mutex);
            get_user(p_smc_dev->port, (unsigned int __user *)parg);
			smc_debug(KERN_INFO "SMC: In %s Transport ID %ld\n", __func__, p_smc_dev->port);
            mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        case SMC_CMD_GET_CARD_STATUS:
        {
            int __user *p_u8_card_status = (int __user *)parg;
            int smc_status = 0x00;
            if (mutex_lock_interruptible(&p_smc_dev->smc_mutex))
				return -EBUSY;
            /* HW is already inserted */
            spin_lock_irq(&p_smc_dev->smc_status_spinlock);
            smc_status |= ((readb(p + REG_ICCSR) & 0x80) ? 0x01 : 0x00);
            /* SW is also initialized */
            smc_status |= smc_misc_card_exist(p_smc_dev);
            spin_unlock_irq(&p_smc_dev->smc_status_spinlock);
            put_user(smc_status, p_u8_card_status);
            mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        }
        case SMC_CMD_DEACTIVE:
            mutex_lock(&p_smc_dev->smc_mutex);
            smc_misc_dev_deactive(p_smc_dev);
            mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        case SMC_CMD_ISO_TRANS:
        case SMC_CMD_ISO_TRANS_T1:
        {
            smc_iso_transfer_t *p_iso_transfer = (smc_iso_transfer_t __user *)parg;
            smc_iso_transfer_t iso_transfer;
            /* We need to convert buffer to kernel buffer */
            /* Due to the function want to process a kernel buffer content */
            
            iso_transfer.actual_size = 0;
            iso_transfer.response = NULL;
            get_user(iso_transfer.num_to_read, (int __user *)&p_iso_transfer->num_to_read);
            get_user(iso_transfer.num_to_write, (int __user *)&p_iso_transfer->num_to_write);

			smc_debug("[ %s %d ], iso_transfer.num_to_write = %ld\n", 
            	__FUNCTION__,__LINE__, iso_transfer.num_to_write);
            	
            smc_debug("[ %s %d ], iso_transfer.num_to_read = %ld\n", 
            	__FUNCTION__,__LINE__, iso_transfer.num_to_read);
            	
            iso_transfer.command = (unsigned char *)kmalloc(iso_transfer.num_to_write, GFP_KERNEL);
            if (iso_transfer.num_to_read > 0)
            {
            	/* response is not NULL when num_to_read == 0 */
            	iso_transfer.response = (unsigned char *)kmalloc(iso_transfer.num_to_read, GFP_KERNEL);
            }
            if (0 != copy_from_user(iso_transfer.command, p_iso_transfer->command, p_iso_transfer->num_to_write))
            {
            	return -EFAULT;
            }
            
            mutex_lock(&p_smc_dev->smc_iso_mutex);
            if (SMC_CMD_ISO_TRANS_T1 == cmd)
                ret = smc_txrx_iso_transfer_t1(p_smc_dev, &iso_transfer);
            else
            {
                ret = smc_txrx_iso_transfer(p_smc_dev, &iso_transfer);                
            }
            
            mutex_unlock(&p_smc_dev->smc_iso_mutex);
            
            put_user(iso_transfer.actual_size, (int __user *)&p_iso_transfer->actual_size); 
            if (0 != copy_to_user(p_iso_transfer->response, iso_transfer.response, iso_transfer.num_to_read))
            {
            	return -EFAULT;
            }
           
            
            kfree(iso_transfer.command); 
            
            kfree(iso_transfer.response);
           
            break;
        }
        case SMC_CMD_SET_IO_ONOFF:
            break;
        case SMC_CMD_CHECK_STATUS:
			if (!tp->inserted)
                put_user(SMC_STATUS_NOT_EXIST, (int __user *)parg);
			else if(!tp->reseted)
                put_user(SMC_STATUS_NOT_RESET, (int __user *)parg);
			else
                put_user(SMC_STATUS_OK, (int __user *)parg);
            break;
        case SMC_CMD_SET_WWT:
			mutex_lock(&p_smc_dev->smc_mutex);
			get_user(tp->first_cwt, (unsigned long __user *)parg);
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        case SMC_CMD_SET_CWT:
            mutex_lock(&p_smc_dev->smc_mutex);
			get_user(tp->cwt, (unsigned long __user *)parg);
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        case SMC_CMD_SET_ETU:
        {
            unsigned long smc_etu = 0;
            get_user(smc_etu, (unsigned long __user *)parg);
            mutex_lock(&p_smc_dev->smc_mutex);
			smc_misc_set_etu(p_smc_dev, smc_etu);
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        }
        case SMC_CMD_GET_F:
            mutex_lock(&p_smc_dev->smc_mutex);
			if (!tp->reseted)
				ret = -EIO;
			else
                put_user(tp->F, (unsigned long __user *)parg);
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;
         case SMC_CMD_GET_D:
            mutex_lock(&p_smc_dev->smc_mutex);
			if (!tp->reseted)
				ret = -EIO;
			else
				put_user(tp->D, (unsigned long *__user )parg);
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        case SMC_CMD_GET_ATR_RESULT:
            mutex_lock(&p_smc_dev->smc_mutex);
            put_user(atr->atr_rlt, (enum smc_atr_result __user *)parg);
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;
         case SMC_CMD_GET_PROTOCOL:
            mutex_lock(&p_smc_dev->smc_mutex);
			if (tp->inserted && tp->reseted)
                put_user(tp->T, (unsigned long __user *)parg);
			else 
			{
                ret = -EIO;
                put_user(0xffffffff, (unsigned long __user *)parg);
			}
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        case SMC_CMD_GET_HB:
		{
            atr_t *p_smc_atr = (atr_t *)atr->atr_info;
            UINT8 len = p_smc_atr->hbn;
			smc_hb_t __user *p = (smc_hb_t __user *)parg;
            
            mutex_lock(&p_smc_dev->smc_mutex);				
			if (len > ATR_MAX_HISTORICAL)
				len = ATR_MAX_HISTORICAL;
			if (0 != copy_to_user(p->hb, p_smc_atr->hb, len))
			{
				return -EFAULT;
			}
			put_user(len, (unsigned char __user *)&p->hbn);
            mutex_unlock(&p_smc_dev->smc_mutex);
            break;
		}
        case SMC_CMD_SET_WCLK:
            mutex_lock(&p_smc_dev->smc_mutex);
            tp->init_clk_idx = 0;
			get_user(tp->init_clk_array[tp->init_clk_idx], (unsigned long __user *)parg);
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;        
		
        case SMC_CMD_GET_CLASS:
            mutex_lock(&p_smc_dev->smc_mutex);
			if ((tp->class_selection_supported) && (!tp->reseted))
                put_user(tp->smc_current_select, (enum class_selection __user *)parg);
			else
				ret = -EIO;
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        case SMC_CMD_SET_CLASS:
            mutex_lock(&p_smc_dev->smc_mutex);
			if ((tp->class_selection_supported) && (!tp->reseted))
				get_user(tp->smc_current_select, (enum class_selection __user *)parg);
			else
				ret = -EIO;
			mutex_unlock(&p_smc_dev->smc_mutex);
            break;
        case SMC_CMD_T1_TRANS:
        {
            smc_t1_trans_t __user *p_t1_trans = (smc_t1_trans_t __user *)parg;
            smc_t1_trans_t t1_trans;

            get_user(t1_trans.dad, (unsigned char __user *)&p_t1_trans->dad);
            get_user(t1_trans.rcv_len, (unsigned long __user *)&p_t1_trans->rcv_len);
            get_user(t1_trans.send_len, (unsigned long __user *)&p_t1_trans->send_len);
            t1_trans.send_buf = (void *)kmalloc(t1_trans.send_len, GFP_KERNEL);
            t1_trans.rcv_buf = (void *)kmalloc(t1_trans.rcv_len, GFP_KERNEL);

            if (0 != copy_from_user(t1_trans.send_buf, p_t1_trans->send_buf, t1_trans.send_len))
            {
            	return -EFAULT;
            }
            ret = smc_t1_bi_comm(p_smc_dev, &t1_trans);
            if (0 != copy_to_user(p_t1_trans->rcv_buf, t1_trans.rcv_buf, t1_trans.rcv_len))
            {
            	return -EFAULT;
            }
            kfree(t1_trans.send_buf);
            kfree(t1_trans.rcv_buf);
            
            break;
        }
        case SMC_CMD_T1_XCV:
        {
            smc_t1_xcv_t *p_t1_xcv = (smc_t1_xcv_t *)parg;
            smc_t1_xcv_t t1_xcv;

            t1_xcv.actual_size = 0;
            get_user(t1_xcv.rmax, (unsigned long __user *)&p_t1_xcv->rmax);
            get_user(t1_xcv.slen, (unsigned long __user *)&p_t1_xcv->slen);
            t1_xcv.rblock = (unsigned char *)kmalloc(p_t1_xcv->rmax, GFP_KERNEL);
            t1_xcv.sblock = (unsigned char *)kmalloc(p_t1_xcv->slen, GFP_KERNEL);

            if (0 != copy_from_user(t1_xcv.sblock, p_t1_xcv->sblock, t1_xcv.slen))
            {
            	return -EFAULT;
            }
            ret = smc_t1_xcv(p_smc_dev, &t1_xcv);
            if (0 != copy_to_user(p_t1_xcv->rblock, t1_xcv.rblock, t1_xcv.rmax))
            {
            	return -EFAULT;
            }
            put_user(t1_xcv.actual_size, (unsigned long __user *)&p_t1_xcv->actual_size);
            kfree(t1_xcv.rblock);
            kfree(t1_xcv.sblock);
            
            break;
        }
        case SMC_CMD_T1_NEGO_IFSD:
        {
            smc_t1_nego_ifsd_t t1_nego_ifsd;
            if (0 != copy_from_user(&t1_nego_ifsd, (smc_t1_nego_ifsd_t __user *)parg, 
                           sizeof(smc_t1_nego_ifsd_t)))
            {
            	return -EFAULT;
            }
            ret = smc_t1_negociate_ifsd(p_smc_dev, &t1_nego_ifsd);
            break;
        }
		case SMC_CMD_CONFIG:
		{
			struct smc_dev_cfg cfg;
			//smc_attr_t *p_smc_attr = (smc_attr_t *)request_attr(SMC);
			struct smc_dev_cfg *p_cfg = (struct smc_dev_cfg __user *)parg;
                    int i = 0;

                    
			/* Here we never touch the default param of the driver */
			memset(&cfg, 0x00, sizeof(struct smc_dev_cfg));
			if (0 != copy_from_user(&cfg, p_cfg, sizeof(struct smc_dev_cfg)))
			{
				return -EFAULT;
			}
			/* We always use the same class_select function */
			/* Which could not be supplied by application */
			
            smc_debug(KERN_INFO "SMC: In %s cfg default %ld\n", \
                          __func__, cfg.use_default_cfg);	
                          
			if (0 != cfg.use_default_cfg)
			{		
				smc_dev_unconfig(p_smc_dev);
				ret = smc_dev_config(p_smc_dev, &config_param_def);
			}
			else						
			{	
				if (0 != p_cfg->init_clk_number && \
					NULL != p_cfg->init_clk_array)
				{
					cfg.init_clk_array = (unsigned long *)kmalloc(cfg.init_clk_number * \
																  sizeof(unsigned long), 
																  GFP_KERNEL);
					if (0 != copy_from_user(cfg.init_clk_array, p_cfg->init_clk_array, \
								   cfg.init_clk_number * sizeof(unsigned long)))
					{
						return -EFAULT;
					}


                                for (i=0; i<cfg.init_clk_number; i++)
                                {
                                    if (0 == cfg.init_clk_array[i])
                                    {
                                        cfg.init_clk_array[i] = config_param_def.init_clk_array[0];
                                    }
                                }                               
    				}

				cfg.class_select = config_param_def.class_select;
				smc_dev_unconfig(p_smc_dev);
				ret = smc_dev_config(p_smc_dev, &cfg);
			}
			break;
		}
		case SMC_CMD_DECONFIG:
			smc_dev_unconfig(p_smc_dev);
			break;
		
		case SMC_CMD_SEND_PPS:			
			if (0 != (copy_from_user(&cmd_param, (void*)parg, sizeof(cmd_param))))
			{
				smc_debug("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}	
			mutex_lock(&p_smc_dev->smc_mutex);	
			//smc_set_pps(tp, cmd_param);
			mutex_unlock(&p_smc_dev->smc_mutex);
			break;
			
		case SMC_CMD_SET_OPEN_DRAIN:
			if (0 != (copy_from_user(&cmd_param, (void*)parg, sizeof(cmd_param))))
			{
				smc_debug("[ %s %d ], error\n", __FUNCTION__, __LINE__);
				return -EFAULT;
			}				
			
			mutex_lock(&p_smc_dev->smc_mutex);		
			tp->open_drain_supported = cmd_param & 0x1;			
			tp->open_drain_supported |= (cmd_param & 0x2);			
			tp->open_drain_supported |= (cmd_param & 0x4);			
			tp->open_drain_supported |= (cmd_param & 0x8);	

			tp->en_power_open_drain = cmd_param & 0x1;			
			tp->en_clk_open_drain = (cmd_param & 0x2) >> 1;			
			tp->en_data_open_drain = (cmd_param & 0x4) >> 2;			
			tp->en_rst_open_drain = (cmd_param & 0x8) >> 3;			
			
			if(tp->open_drain_supported)
			{		
				UINT8 temp_val =  readb(p + REG_CLK_VPP);				
				temp_val &= 0x9f;			
				temp_val |= (tp->en_power_open_drain<<6);				
				temp_val |= ((tp->en_clk_open_drain|tp->en_data_open_drain|tp->en_rst_open_drain)<<5);
				writeb(temp_val, p + REG_CLK_VPP);	 				
			}
			mutex_unlock(&p_smc_dev->smc_mutex);
			break;
		
		case SMC_CMD_SET_DEBUG_LEVEL:		
		    break;
			
        default:
            break;
    }
    return ret;
}

/******************************************************************************************************
 * 	Name		:	ali_smc_dev_init()
 *	Description	:	Smart card reader init funciton.
 *	Parameter	:	void
 *	Return		:	
 *
 ******************************************************************************************************/
static int __devinit ali_smc_dev_init(void)
{
	UINT8 dev_id;
    int ret = 0;
    int smc_dev_cnt = 0;
    dev_t dev_no = 0;
    struct dentry *fs_entry;  
	
	
    smc_dev_class_create(&dev_no);  
    
    for (dev_id = 0; dev_id < SMC_DEV_NUM; dev_id++)
    {
        smc_dev_mutex_request(dev_id);
        smc_dev_workqueue_request(dev_id);
        
        ret = smc_dev_register(dev_id, dev_no + dev_id, &ali_smc_fops);
        if (ret < 0)
        {
            smc_debug(KERN_ERR "SMC: In %s Device %d register error: %d\n", 
                          __func__, dev_id, ret);
            continue;       
        }

        ret = smc_dev_create(dev_id, dev_no + dev_id);
        if (ret < 0)
        {
            smc_debug(KERN_ERR "SMC: In %s Device %d create error: %d\n", 
                          __func__, dev_id, ret);
            goto err1;
        }

        smc_debug(KERN_INFO "SMC: In %s Device %p Private %p\n", 
                      __func__, smc_dev_get(dev_id), smc_dev_get(dev_id)->priv);

        smc_dev_cnt++;
        continue;
err1:
        smc_dev_workqueue_release(dev_id);
        smc_dev_mutex_release(dev_id);
        smc_dev_unregister(dev_id);
    }

    
    
    g_ali_smc_debugfs_root = debugfs_create_dir("ali_smc", NULL);    
    if (NULL == g_ali_smc_debugfs_root)
    {
        smc_error("[ %s %d ], error!\n", __FUNCTION__, __LINE__);
        return(-ENOENT);
    }
  
    fs_entry = debugfs_create_u32("debug_level", 0644, g_ali_smc_debugfs_root,
                                  &g_ali_smc_debug_level);
    if (!fs_entry)
    {
        smc_error("%s,%d\n", __FUNCTION__, __LINE__);        
    }

    if (!smc_dev_cnt)
    {
        smc_debug(KERN_ERR "SMC: In %s Device created\n", __func__);
        smc_dev_class_delete();
        return -EINVAL;
    }

	
	return 0;
}


static void __exit ali_smc_dev_exit(void)
{
    UINT8 dev_id = 0;

    
    for (dev_id = 0; dev_id < SMC_DEV_NUM; dev_id++)
    {
        smc_dev_workqueue_release(dev_id);
        smc_dev_mutex_release(dev_id);
        smc_dev_unregister(dev_id);
    }

    smc_dev_class_delete();

    debugfs_remove_recursive(g_ali_smc_debugfs_root);
    g_ali_smc_debugfs_root = NULL;
    g_ali_smc_debug_level = 0;

    
    return;
}

module_init(ali_smc_dev_init);
module_exit(ali_smc_dev_exit);

MODULE_DESCRIPTION("driver for the Ali M36xx Smartcard device");
MODULE_AUTHOR("ALi Corp Zhuhai SDK Team, Owen Zhao");
MODULE_LICENSE("GPL");



