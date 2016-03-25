/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_irq.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader irq.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/

#include "ali_smartcard_irq.h"

/* SMC GPIO ---- card detect */
irqreturn_t smc_irq_gpio_detect(int irq, void *param)
{
    struct smc_device *dev = (struct smc_device *)param;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	
	if (0 == gpio_irq_get_status(tp->gpio_cd_pos))
		return IRQ_NONE;	
	gpio_irq_clear(tp->gpio_cd_pos);
	if ((ali_gpio_get_value(tp->gpio_cd_pos)) == tp->gpio_cd_pol)
	{
		if (tp->inserted)
		{
			smc_misc_dev_deactive((struct smc_device *)dev);
		}
	}

    return IRQ_HANDLED;
}

/* Workqueue to notify application */
static void smc_irq_card_status_notification(struct work_struct *work)
{
    struct smc_device *p_smc_dev = container_of((void *)work, struct smc_device, smc_notification_work);
    struct smc_notification_param_t smc_notification;
    struct smartcard_private *tp = (struct smartcard_private *)p_smc_dev->priv;

    if (SMC_INVALID_TRANSPORT_ID == p_smc_dev->port)
        return;
    
    smc_notification.smc_msg_tag = SMC_MSG_CARD_STATUS;
    smc_notification.smc_msg_len = 1;
    smc_notification.smc_msg_buf[0] = tp->inserted;

    mutex_lock(&p_smc_dev->smc_irq_mutex);
    ali_transport_send_msg(p_smc_dev->port, 
                           &smc_notification, 
                           sizeof(smc_notification_param_t));
    mutex_unlock(&p_smc_dev->smc_irq_mutex);
}

/* SMC interrupt */
static inline void smc_irq_isr0_bytes_receive(struct smc_device *dev,
                                          void __iomem *p_io_base)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 i = 0;
    
    if (tp->isr0_interrupt_status & SMC_ISR0_PE_RECV)
	{
        smc_debug(KERN_ERR "SMC IRQ: In %s RX PE ERR\n", __func__);
        if (smc_read_rx(tp, p_io_base))
        {
            tp->isr0_interrupt_status &= (~SMC_ISR0_PE_RECV);
        }
	}
    
	if ((0 != tp->smc_rx_tail) && (0 == tp->got_first_byte))
	{     
		tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
		tp->got_first_byte = 1; 
		/* TO DO: We need to notify the byte receiving to who want to know */
        smc_misc_set_card_flags(dev, SMC_RX_BYTE_RCV);
        smc_debug(KERN_INFO "SMC IRQ: In %s receive first byte:%ld\n", __func__, tp->smc_rx_head);
	}
    
	if (tp->smc_rx_buf == tp->smc_rx_tmp_buf)
	{
		UINT16 c;
		UINT32 rem_space = tp->smc_rx_tail - tp->smc_rx_head;
	
		c =smc_read_rx(tp, p_io_base);
		c = (c <= rem_space ? c : rem_space);			
		
		for (i = 0; i < c; i++)
			tp->smc_rx_buf[tp->smc_rx_head+i] = readb(p_io_base + REG_RBR);
			
		tp->smc_rx_head += c; 
		tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
        smc_debug(KERN_INFO "SMC IRQ: In %s receive data:%ld\n", __func__, tp->smc_rx_head);
	}
}

static inline int smc_irq_isr0_fifo_transfer_force_loop(struct smartcard_private *tp,
                                                  void __iomem *p_io_base)
{
    UINT32 force_loop_tmo;
	UINT8 force_tx_rx;
    

	force_tx_rx = 1;
	force_loop_tmo = (smc_read_tx(tp, p_io_base) + 1) * tp->char_frm_dura;
	
	while (0 != smc_read_tx(tp, p_io_base))
	{
		udelay(1);
		force_loop_tmo--;
		if (0 != force_loop_tmo)
		{
			force_tx_rx = 0;
			break;
		}
		
		if (0 == (readb(p_io_base + REG_ICCSR) & 0x80))
		{
            smc_debug(KERN_ERR "SMC IRQ: In %s Smart card out\n", __func__);
			return -EIO;
		}		
	}
    
	if (force_tx_rx)
	{
		writeb((readb(p_io_base + REG_SCR_CTRL) & \
               (~SMC_SCR_CTRL_TRANS)) | SMC_SCR_CTRL_RECV,
               p_io_base + REG_SCR_CTRL);
	}

    return 0;
}

static inline void smc_irq_isr0_fifo_transfer_auto_loop(struct smc_device *dev,
                                                  void __iomem *p_io_base)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    if (!(tp->auto_tx_rx_triger))
	{	
		writeb((readb(p_io_base + REG_SCR_CTRL) & \
               (~SMC_SCR_CTRL_TRANS)) | SMC_SCR_CTRL_RECV,
               (p_io_base + REG_SCR_CTRL));		
	}

    /*****************************************/
    /* TO DO: Notify SMC_TX_FINISHED message */
    /*****************************************/
    smc_misc_set_card_flags(dev, SMC_TX_FINISHED);
    
	if (SYS_IC_BONDING_TYPE_5(tp))
	{
		tp->the_last_send = 0;  
		tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);						
	}
}

static inline int smc_irq_isr0_fifo_transfer_end(struct smc_device *dev,
                                            void __iomem *p_io_base)
{
    int ret = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    /*Once TX finished, set interface device to RX mode immediately*/
	if (tp->force_tx_rx_triger && 1 == tp->force_tx_rx_state)
	{
		smc_write_tx(tp, p_io_base, tp->force_tx_rx_thld);
		tp->force_tx_rx_state = 2;
		if (0 == smc_read_tx(tp, p_io_base))
		{	
			writeb((readb(p_io_base + REG_SCR_CTRL) & \
                    (~SMC_SCR_CTRL_TRANS)) | SMC_SCR_CTRL_RECV, 
                    p_io_base + REG_SCR_CTRL);
			smc_debug(KERN_WARNING "SMC IRQ: In %s Too late!\n", __func__);
		}
		else
		{
			ret = smc_irq_isr0_fifo_transfer_force_loop(tp, p_io_base);
		}
	}
	else
	{
        smc_irq_isr0_fifo_transfer_auto_loop(dev, p_io_base);
	}

    return ret;
}

static inline void smc_irq_isr0_fifo_transfer_force_size(struct smartcard_private *tp,
                                                   void __iomem *p_io_base,
                                                   UINT32 prepare_size)
{
    if (0==smc_read_tx(tp, p_io_base))
	{
		if (prepare_size > tp->force_tx_rx_thld)
		{	
			smc_write_tx(tp, p_io_base, prepare_size - tp->force_tx_rx_thld);
			tp->force_tx_rx_state = 1;
		}
		else
		{	
			smc_write_tx(tp, p_io_base, prepare_size);
			tp->force_tx_rx_state = 2;
		}
	}
	else
	{	
		smc_write_tx(tp, p_io_base, 
                     (prepare_size + (tp->smc_tx_fifo_size>>1)) - tp->force_tx_rx_thld);
		tp->force_tx_rx_state = 1;
	}
}

static inline void smc_irq_isr0_fifo_transfer_auto_size(struct smartcard_private *tp,
                                                  void __iomem *p_io_base,
                                                  UINT32 prepare_size)
{
    if (SYS_IC_BONDING_TYPE_5(tp))
	{
 		writeb((readb(p_io_base + REG_FIFO_CTRL) & 0xf0) | \
                (prepare_size + (tp->smc_tx_fifo_size>>1)), 
                p_io_base + REG_FIFO_CTRL);
		tp->the_last_send = 1;
		tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);
	}
	else
	{
		if (0 == smc_read_tx(tp, p_io_base))
			smc_write_tx(tp, p_io_base, prepare_size);
		else	
			smc_write_tx(tp, p_io_base, prepare_size + (tp->smc_tx_fifo_size>>1));
	}
}

static inline UINT32 smc_irq_isr0_fifo_transfer_size_config(struct smartcard_private *tp,
                                                      void __iomem *p_io_base,
                                                      UINT32 prepare_size)
{
    UINT32 size = prepare_size;
    
    if (size > (tp->smc_tx_fifo_size>>1))
	{	
		size = tp->smc_tx_fifo_size>>1;
		if (SYS_IC_BONDING_TYPE_5(tp))
			writeb((readb(p_io_base + REG_FIFO_CTRL) & 0xf0) | (size), 
			        p_io_base + REG_FIFO_CTRL);
		else
			smc_write_tx(tp, p_io_base, size);
	}
	else
	{
		if(tp->force_tx_rx_triger)
		{
			smc_irq_isr0_fifo_transfer_force_size(tp, p_io_base, size);
		}
		else
		{	
			smc_irq_isr0_fifo_transfer_auto_size(tp, p_io_base, size);
		}
	}

    return size;
}

static inline void smc_irq_isr0_fifo_transfer_action(struct smartcard_private *tp,
                                              void __iomem *p_io_base, UINT32 size)
{
    UINT32 i = 0;
    
    for (i = 0; i < size; i++)
	{
		if ((tp->smc_tx_rd + i + 1) == tp->smc_tx_wr)
		{
			if (tp->auto_tx_rx_triger)
			{
				writeb(1<<5, p_io_base + REG_ICCSR);  //tx->rx auto switch
				smc_debug(KERN_INFO "SMC IRQ: In %s tx->rx auto rd %ld, cnt %ld, wr %ld\n", 
                              __func__, tp->smc_tx_rd, i, tp->smc_tx_wr);
			}
		}
		writeb(tp->smc_tx_buf[tp->smc_tx_rd+i], p_io_base + REG_THR);
	}
    
	tp->smc_tx_rd += size;
	smc_debug(KERN_INFO "SMC IRQ: In %s Continue feed data %ld\n", __func__, size);
}

static inline int smc_irq_isr0_fifo_transfer_start(struct smartcard_private *tp,
                                            void __iomem *p_io_base)
{
    UINT32 size = tp->smc_tx_wr - tp->smc_tx_rd;
    int ret = 0;
	
	size = smc_irq_isr0_fifo_transfer_size_config(tp, p_io_base, size);
	smc_irq_isr0_fifo_transfer_action(tp, p_io_base, size);		
    
	if (tp->smc_tx_rd == tp->smc_tx_wr && \
		1 == tp->force_tx_rx_triger && \
		tp->force_tx_rx_state == 2)
	{
        ret = smc_irq_isr0_fifo_transfer_force_loop(tp, p_io_base);
	}

	return ret;
}

static inline int smc_irq_isr0_fifo_transfer(struct smc_device *dev,
                                      void __iomem *p_io_base)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    int ret = 0;
    
    tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_TRANS);
    
	if (tp->smc_tx_wr)
	{
		if(tp->smc_tx_rd == tp->smc_tx_wr)
		{
			ret = smc_irq_isr0_fifo_transfer_end(dev, p_io_base);		
		}
		else
		{
			ret = smc_irq_isr0_fifo_transfer_start(tp, p_io_base);
		}
    }

    return ret;
}

static inline void smc_irq_isr0_fifo_receive(struct smc_device *dev,
                                       void __iomem *p_io_base)
{
    UINT16 c, i = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv; 
	UINT32 rem_space = tp->smc_rx_tail - tp->smc_rx_head;       
    
	tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_RECV);    
	if (tp->smc_rx_tail && tp->smc_rx_buf != tp->smc_rx_tmp_buf)
	{
		c =smc_read_rx(tp, p_io_base);
		c = (c <= rem_space ? c : rem_space);			
	
		for (i = 0; i < c; i++)
			tp->smc_rx_buf[tp->smc_rx_head + i] = readb(p_io_base + REG_RBR);
		tp->smc_rx_head += c; 

		if (tp->smc_rx_head == tp->smc_rx_tail)
		{
            /* TO DO: Notify SMC_RX_FINISHED message */            
            smc_misc_set_card_flags(dev, SMC_RX_FINISHED);
            smc_debug(KERN_INFO "SMC IRQ: In %s receive all data:%ld\n", __func__, tp->smc_rx_head);
		}
		else
		{
			rem_space = tp->smc_rx_tail - tp->smc_rx_head;    
			if (rem_space / tp->smc_rx_fifo_size)
				smc_write_rx(tp, p_io_base, 32);
			else
				smc_write_rx(tp, p_io_base, rem_space);	
			smc_debug(KERN_INFO "SMC IRQ: In %s Continue rx %ld data\n", __func__, rem_space);		
		}
	}
}

static inline void smc_irq_isr0_fifo_empty(struct smc_device *dev,
                                      void __iomem *p_io_base)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;  
    
    if (!(tp->auto_tx_rx_triger))
	{	
		writeb((readb(p_io_base + REG_SCR_CTRL) & \
               (~SMC_SCR_CTRL_TRANS)) | SMC_SCR_CTRL_RECV,
               p_io_base + REG_SCR_CTRL);		
	}

    /*****************************************/
    /* TO DO: Notify SMC_TX_FINISHED message */
    /*****************************************/    
    smc_misc_set_card_flags(dev, SMC_TX_FINISHED);

	tp->the_last_send = 0;
	tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);	
}

static inline void smc_irq_isr1_card_inserted(struct smartcard_private *tp,
                                          void __iomem *p_io_base)
{
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    
    tp->inserted = 1;
	atr->atr_rlt = SMC_ATR_NONE;
	tp->reseted = 0;
}

static inline void smc_irq_isr1_card_removed(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    struct smartcard_atr *atr = (struct smartcard_atr *)tp->atr;
    
    tp->inserted = 0;
	tp->reseted = 0;
	atr->atr_rlt = SMC_ATR_NONE;
	tp->smc_supported_class = 0;
	tp->smc_current_select = SMC_CLASS_NONE_SELECT;
	tp->reseted = 0;
    
	if(tp->class_selection_supported)
	{
		INT32 class_id;
		for(class_id = 0; class_id < 3; class_id++)
		{
			if(tp->board_supported_class & (1<<class_id))
			{
				smc_debug(KERN_INFO "SMC IRQ: In %s Set class to %ld\n", 
                              __func__, ('A' + class_id));
                if (NULL != tp->class_select)
				    tp->class_select((enum class_selection)(SMC_CLASS_NONE_SELECT + class_id + 1));
				break;
			}
		}
	}
    
	smc_misc_init_hw(dev);
}

#define SMC_ISR_STATUS(idx, isr, p, tp) \
    do { \
        isr = readb(p + REG_ISR0 + idx); \
        if (0 == idx) \
    	    tp->isr0_interrupt_status |= isr; \
    	else \
            tp->isr1_interrupt_status |= isr; \
    	writeb(isr, p + REG_ISR0 + idx); \
    } while (0)

static inline int smc_irq_isr0_process(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    void __iomem *p = (void __iomem *)dev->io_base;
    UINT8 isr0_status;
    int ret = 0;

    SMC_ISR_STATUS(0, isr0_status, p, tp);
    
    if (isr0_status & SMC_ISR0_BYTE_RECV)
	{
		smc_irq_isr0_bytes_receive(dev, p);
	}
	if ((isr0_status & SMC_ISR0_FIFO_TRANS) && 
        (tp->force_tx_rx_triger || tp->auto_tx_rx_triger))
	{
    	ret = smc_irq_isr0_fifo_transfer(dev, p);
	}
	if (isr0_status & SMC_ISR0_FIFO_RECV)
	{
		smc_irq_isr0_fifo_receive(dev, p);
	}
    
	if((SYS_IC_BONDING_TYPE_5(tp)) && \
       (isr0_status & SMC_ISR0_FIFO_EMPTY) && \
       (1 == tp->the_last_send))
	{
		smc_irq_isr0_fifo_empty(dev, p);		
	}

    return ret;
}

static inline int smc_irq_isr1_process(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    void __iomem *p = (void __iomem *)dev->io_base;
    UINT8 isr1_status;

	SMC_ISR_STATUS(1, isr1_status, p, tp);

    if (0 != (isr1_status & SMC_ISR1_CARD_INSERT))
	{          
        smc_irq_isr1_card_inserted(tp, p);
        /* Fixed bug:re-insert card cause PPS error,"card problem" */
        smc_misc_unset_card_flags(dev,SMC_RX_FINISHED | SMC_TX_FINISHED | SMC_RX_BYTE_RCV);
        smc_misc_set_card_flags(dev, SMC_INSERTED);
        smc_debug(KERN_INFO "SMC IRQ: In %s : Wait queue flag 0x%lx\n", __func__, dev->wq_flags);
	}
	else if(0 != (isr1_status & SMC_ISR1_CARD_REMOVE))
	{        
	    smc_irq_isr1_card_removed(dev);  
        smc_misc_set_card_flags(dev, SMC_REMOVED);
	}

    return 0;
}

/**
 * The only way we are here is the different status
 * So, we just need to check if we have enough gap
 * for two plugs
 */
static inline int smc_irq_valid_plug_check(struct smc_device *dev)
{
    static unsigned long u32_smc_tm = 0;
    int ret = 0;

#define SMC_VALID_PLUG_GAP  100

    if (jiffies - u32_smc_tm < SMC_VALID_PLUG_GAP)
        ret = -EINVAL;

    u32_smc_tm = jiffies;
    return ret;
}

static int b_last_status = -1;

/**
 * We need to let other to set the status, especially for resume / suspend
 */
void smc_irq_set_monitor_status(int status)
{
	b_last_status = status;
}

int smc_irq_get_monitor_status(void)
{
	return b_last_status;
}

irqreturn_t smc_irq_dev_interrupt(int irq, void *param)
{
    struct smc_device *dev = (struct smc_device *)param;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    int ret = IRQ_NONE;

    /* ISR0 & ISR1 doesn't means the two smart card reader */
    /* It just means the different registers of the reader */
    /* Interrupt will be cleared in one of the callee */    
	if ((ret = smc_irq_isr0_process(dev)) < 0)
	{
        smc_debug(KERN_ERR "SMC IRQ: In %s ISR0 errno %d\n", __func__, ret);        
		return IRQ_HANDLED;		       
	}

    smc_irq_isr1_process(dev);
     
	/*********************************************/
    /* TO DO: Notidy application card status     */
    /*********************************************/
    if (0 != (tp->isr1_interrupt_status & SMC_ISR1_CARD_INSERT) || \
        0 != (tp->isr1_interrupt_status & SMC_ISR1_CARD_REMOVE) || \
		0 != (b_last_status & SMC_IRQ_FORCE_STATUS))
        smc_debug(KERN_INFO "SMC IRQ: In %s card inserted status %d %d\n", \
                      __func__, b_last_status, tp->inserted);
    if (b_last_status == tp->inserted) 
    {
    	return IRQ_HANDLED;		
    }
    

    if (b_last_status < 0)
        INIT_WORK(&dev->smc_notification_work, \
                  smc_irq_card_status_notification);
    else
        PREPARE_WORK(&dev->smc_notification_work, \
                     smc_irq_card_status_notification);
    
    if (NULL != dev->smc_notification_workqueue)
        queue_work(dev->smc_notification_workqueue, \
                   &dev->smc_notification_work);
    b_last_status = tp->inserted;
        
	return IRQ_HANDLED;
}



