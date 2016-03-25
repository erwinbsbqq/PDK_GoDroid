/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2011 Copyright (C)
 *
 *  File: ali_smartcard_txrx.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard transferation.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *     
 ****************************************************************************/
#include <linux/slab.h>

#include "ali_smartcard_txrx.h"

/* TX / RX buffer parameters */
#define SMC_TX_BUF_SIZE 	256
#define SMC_RX_BUF_SIZE 	512
#define SMC_TX_RX_THLD	    4000 /* MS */

/* ISO7816 command parameters */
 #define MAX_LENGTH	        256
 #define CMD_LENGTH	        5
 #define CLA_OFFSET	        0
 #define INS_OFFSET	        1
 #define P1_OFFSET	        2
 #define P2_OFFSET	        3
 #define P3_OFFSET	        4

/* Alloc and Free buffer */
int smc_txrx_buf_alloc(struct smartcard_private *tp)
{
    BUG_ON(NULL == tp);
	if (NULL == tp->smc_rx_tmp_buf)
		tp->smc_rx_tmp_buf = (UINT8 *)kmalloc(SMC_RX_BUF_SIZE, GFP_KERNEL);
	if (NULL == tp->smc_rx_tmp_buf)
	{
	    smc_debug(KERN_ERR "SMC TXRX: In %s Buffer: out of memory\n", __func__);
        return -ENOMEM;
	}

    tp->smc_rx_tmp_buf = (UINT8 *)((UINT32)(tp->smc_rx_tmp_buf)); 

    return 0;
}

void smc_txrx_buf_free(struct smartcard_private *tp)
{
    BUG_ON(NULL == tp);

    if (NULL != tp->smc_rx_tmp_buf)
        kfree((UINT8 *)tp->smc_rx_tmp_buf);

    tp->smc_rx_tmp_buf = NULL;
}

/* Reset all buffer */
void smc_txrx_buf_reset(struct smartcard_private *tp)
{
    BUG_ON(NULL == tp);

	tp->smc_tx_buf = NULL;    
    tp->smc_rx_buf = NULL;
	tp->smc_tx_rd = 0;
	tp->smc_tx_wr = 0;
	tp->smc_rx_head = 0;
	tp->got_first_byte = 0;
	tp->smc_rx_tail = 0;
}

/* Transfer Data */
static int smc_txrx_preset_check(struct smartcard_private *tp, UINT8 *buffer, 
                                     UINT16 size, UINT8 *recv_buffer, UINT16 reply_num)
{
    /* This should be assigned for the upper error check */
    tp->smc_tx_buf = buffer;
    
    if (!tp->inserted)
	{
		smc_debug("SMC TXRX: In %s: Smart card not inserted!\n", __func__);
		return -EIO;
	}

    if(tp->reseted != 1)
	{
		smc_debug("SMC TXRX: In %s: Smart card not reseted!\n", __func__);
		return -EIO;
	}	
	
	tp->smc_rx_buf = recv_buffer;
	tp->smc_tx_rd = 0;
	tp->smc_tx_wr = size;
	tp->smc_rx_head = 0;
	tp->got_first_byte = 0;
    
	if(reply_num)
		tp->smc_rx_tail = reply_num;
	else
		tp->smc_rx_tail = SMC_RX_BUF_SIZE;

    return 0;
}

static int smc_txrx_config_rx_receiver(struct smartcard_private *tp, 
                                          void __iomem *p, int reply_num)
{
    /* enable transmit mode disable receiver mode */
    writeb(readb(p + REG_SCR_CTRL) & (~(SMC_SCR_CTRL_RECV | \
           SMC_SCR_CTRL_TRANS)), 
           p + REG_SCR_CTRL);
    writeb(readb(p + REG_IER0) & (~(SMC_IER0_BYTE_RECV_TRIG | \
           SMC_IER0_BYTE_TRANS_TRIG)), 
           p + REG_IER0);
           
	if (reply_num)
	{
		if (reply_num / tp->smc_rx_fifo_size)
			smc_write_rx(tp, p, (tp->smc_rx_fifo_size>>1));
		else
			smc_write_rx(tp, p, reply_num);	
	}
	else
		smc_write_rx(tp, p, 0);
    
	/*Always enable byte receive int*/
    writeb(readb(p + REG_IER0) | SMC_IER0_BYTE_RECV_TRIG, p + REG_IER0);
    writeb(SMC_FIFO_CTRL_EN |SMC_FIFO_CTRL_TX_OP | SMC_FIFO_CTRL_RX_OP, p + REG_FIFO_CTRL);

    return 0;
}

#define SMC_SET_TX_TRANSFER_PARAM(tp, p, j, tmo, scale) \
    for(j = 0; j < scale; j++) \
        writeb(tp->smc_tx_buf[tp->smc_tx_rd + j], p + REG_THR); \
	tp->smc_tx_rd += scale; \
	tmo = (scale + 2) * 2; \
	while (0 == (tp->isr0_interrupt_status & SMC_ISR0_FIFO_EMPTY)) \
	{ \
		mdelay(1); \
		tmo--; \
		if (0 == tmo) return -ETIMEDOUT; \
		if (0 == (readb(p + REG_ICCSR) & 0x80)) \
		{ \
		smc_debug("SMC TXRX: In %s Smart card not inserted!\n", __func__); \
		return -EIO; \
		} \
	} \
	tp->isr0_interrupt_status &= ~SMC_ISR0_FIFO_EMPTY;

#define SMC_SET_TX_TRANSFER_PARAM_LOCK_IRQ(dev, tp, p, j, tmo, scale) \
    spin_lock_irq(&dev->smc_spinlock); \
    for(j = 0; j < scale; j++) \
        writeb(tp->smc_tx_buf[tp->smc_tx_rd + j], p + REG_THR); \
	tp->smc_tx_rd += scale; \
	tmo = scale * 2000; \
	while (0 == (tp->isr0_interrupt_status & SMC_ISR0_FIFO_EMPTY)) \
	{ \
		udelay(1); \
		tmo--; \
		if (0 == tmo) \
        { \
            spin_unlock_irq(&dev->smc_spinlock); \
            return -ETIMEDOUT; \
		} \
		if (0 == (readb(p + REG_ICCSR) & 0x80)) \
		{ \
            spin_unlock_irq(&dev->smc_spinlock); \
            smc_debug("SMC TXRX: In %s Smart card not inserted!\n", __func__); \
            return -EIO; \
		} \
	} \
	tp->isr0_interrupt_status &= ~SMC_ISR0_FIFO_EMPTY; \
	spin_unlock_irq(&dev->smc_spinlock);

static int __smc_txrx_config_tx_rx_trigger_group_one(struct smc_device *dev, 
                                                          void __iomem *p, int size, int loop)
{
    UINT16 loop_cnt = 0;
	UINT16 loop_remain = 0;
	UINT32 tmo;
    int i = 0, j = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
	/* Enable Fifo Empty Interrupt */
    writeb(readb(p + REG_IER0) | SMC_IER0_TRANS_FIFO_EMPY, p + REG_IER0);	
    /*smc_misc_set_card_flags(dev, dev->wq_flags & ~(SMC_RX_BYTE_RCV | \
                            SMC_RX_FINISHED | SMC_TX_FINISHED));*/
    smc_misc_unset_card_flags(dev, SMC_RX_BYTE_RCV | \
                            SMC_RX_FINISHED | SMC_TX_FINISHED);
    writeb(readb(p + REG_SCR_CTRL) | SMC_SCR_CTRL_TRANS, p + REG_SCR_CTRL);

    if (size > 5)
	{
		loop_remain = size - 5;
		loop_cnt = loop_remain / tp->smc_tx_fifo_size;
		loop_remain = loop_remain % tp->smc_tx_fifo_size;
		size = 5;
	}
    
	for (i = 0; i < loop_cnt; i++)
	{
		SMC_SET_TX_TRANSFER_PARAM(tp, p, j, tmo, tp->smc_tx_fifo_size)
	}
    
	if (loop_remain)
	{
		SMC_SET_TX_TRANSFER_PARAM(tp, p, j, tmo, loop_remain)
	}
    SMC_SET_TX_TRANSFER_PARAM_LOCK_IRQ(dev, tp, p, j, tmo, size)

    spin_lock_irq(&dev->smc_spinlock);
	writeb(SMC_ISR0_FIFO_EMPTY, p + REG_ISR0);
	writeb((readb(p + REG_SCR_CTRL) & (~SMC_SCR_CTRL_TRANS)) | \
           SMC_SCR_CTRL_RECV, 
           p + REG_SCR_CTRL);
	spin_unlock_irq(&dev->smc_spinlock);
	smc_misc_set_card_flags(dev, SMC_TX_FINISHED);

	return 0;
}

static void __smc_txrx_config_force_tx_rx_trigger_group_one(struct smartcard_private *tp,
                                                                  void __iomem *p)
{
    UINT16 real_work_etu = readw(p + REG_ETU0);
	UINT16 char_frame = readw(p + REG_GTR0);

    tp->char_frm_dura = char_frame * ((real_work_etu * 1000) / (tp->smc_clock / 1000));
    if (tp->force_tx_rx_triger)
    {
        tp->force_tx_rx_state = 0;
    	tp->force_tx_rx_thld = ((tp->smc_clock / 1000) * (SMC_TX_RX_THLD / 1000)) / \
                                (real_work_etu * char_frame);
    	if (tp->force_tx_rx_thld < 5)
    		tp->force_tx_rx_thld = 5;
    }
    smc_debug("SMC TXRX: In %s Force tx rx thld %d, char frm dura %ld\n", 
                  __func__, tp->force_tx_rx_thld, tp->char_frm_dura);
}

static int __smc_txrx_config_force_tx_rx_trigger_group_two(struct smartcard_private *tp,
                                                                 void __iomem *p, int loop)
{
    if (tp->force_tx_rx_triger && 0 == loop && 0 == tp->force_tx_rx_state)
	{
		if (0 == smc_read_tx(tp, p))
		{	
			writeb((readb(p + REG_SCR_CTRL) & (~SMC_SCR_CTRL_TRANS)) | \
                   SMC_SCR_CTRL_RECV, 
                   p + REG_SCR_CTRL);
			smc_debug("SMC TXRX: In %s TMO: too late!\n", __func__);
		}
		else
		{
			UINT32 force_loop_tmo = (smc_read_tx(tp, p) + 1) * tp->char_frm_dura;
			UINT8 force_tx_rx = 1;		
			
			while (0 != smc_read_tx(tp, p))
			{
				udelay(1);
				force_loop_tmo--;
                
				if (0 == force_loop_tmo)
				{
					force_tx_rx = 0;
					smc_debug("SMC TXRX: In %s TMO %d\n", 
                                  __func__, smc_read_tx(tp, p));
					break;
				}
                
				if (0 == (readb(p + REG_ICCSR) & 0x80))
				{
					smc_debug("SMC TXRX: In %s Smart card not inserted!\n", __func__);
					return -EIO;
				}
			}
            
			if(force_tx_rx)
			{
				writeb((readb(p + REG_SCR_CTRL) & (~SMC_SCR_CTRL_TRANS)) | \
                       SMC_SCR_CTRL_RECV, p + REG_SCR_CTRL);
			}
		}
	}

    return 0;
}

static void __smc_txrx_config_tx_transmitor(struct smartcard_private *tp, 
                                              void __iomem *p, int size, int loop)
{
    if (loop)
	{
		size = tp->smc_tx_fifo_size;
		if (SYS_IC_BONDING_TYPE_5(tp))
            writeb((readb(p + REG_FIFO_CTRL) & 0xf0) | (size>>1), p + REG_FIFO_CTRL);
		else
			smc_write_tx(tp, p, tp->smc_tx_fifo_size>>1);
	}
	else
	{
		if (tp->force_tx_rx_triger && size > tp->force_tx_rx_thld)
		{
			smc_write_tx(tp, p, size - tp->force_tx_rx_thld);
			tp->force_tx_rx_state = 1;
		}
		else
		{
			if (SYS_IC_BONDING_TYPE_5(tp))
                writeb((readb(p + REG_FIFO_CTRL) & 0xf0) | size, p + REG_FIFO_CTRL);
			else
				smc_write_tx(tp, p, size);
		}
	}
}

static int __smc_txrx_config_tx_rx_trigger_group_two(struct smc_device *dev, 
                                                          void __iomem *p, int size, int loop)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    int j = 0;    
    
    
    __smc_txrx_config_force_tx_rx_trigger_group_one(tp, p);
    __smc_txrx_config_tx_transmitor(tp, p, size, loop);
	
	for (j = 0; j < size; j++)
	{
		if ((tp->smc_tx_rd + j + 1) == tp->smc_tx_wr && tp->auto_tx_rx_triger)
		{
			writeb(1<<5, p + REG_ICCSR); /* tx->rx auto switch */
			/*smc_debug("SMC TXRX: In %s tx->rx auto ---- rd %ld, cnt %ld, wr %ld\n", 
                          __func__, tp->smc_tx_rd, j, tp->smc_tx_wr);
                    */
		}
		writeb(tp->smc_tx_buf[j], p + REG_THR);
	}
	tp->smc_tx_rd += size;
    /*smc_misc_set_card_flags(dev, dev->wq_flags & ~(SMC_RX_BYTE_RCV | \
                            SMC_RX_FINISHED | SMC_TX_FINISHED));*/
    smc_misc_unset_card_flags(dev, SMC_RX_BYTE_RCV | \
                            SMC_RX_FINISHED | SMC_TX_FINISHED);
    writeb(readb(p + REG_IER0) | SMC_ISR0_FIFO_TRANS, p + REG_IER0);	
	writeb(readb(p + REG_SCR_CTRL) | SMC_SCR_CTRL_TRANS, p + REG_SCR_CTRL);

    return __smc_txrx_config_force_tx_rx_trigger_group_two(tp, p, loop);
	
}

static int smc_txrx_config_tx_rx_trigger(struct smc_device *dev, void __iomem *p,
                                           int size, int loop)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
       

    smc_debug("SMC TXRX: In %s Trigger method %d %d\n",
                  __func__, tp->force_tx_rx_triger, tp->auto_tx_rx_triger);
    if ((!tp->force_tx_rx_triger) && (!tp->auto_tx_rx_triger))
	{
		return __smc_txrx_config_tx_rx_trigger_group_one(dev, p, size, loop);
	}
	else
	{
		return __smc_txrx_config_tx_rx_trigger_group_two(dev, p, size, loop);
	}
}

static int __smc_txrx_wait_finish_timeout_type_one(struct smc_device *dev, 
                                                        void __iomem *p)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    if (tp->smc_tx_wr)
	{
		spin_lock_irq(&dev->smc_spinlock);
		if (tp->smc_tx_rd == tp->smc_tx_wr)
		{
			if (tp->force_tx_rx_triger && 1 == tp->force_tx_rx_state)
			{
				smc_write_tx(tp, p, tp->force_tx_rx_thld);
				tp->force_tx_rx_state = 2;					
			}
		}
		else
		{
			smc_debug(KERN_ERR "SMC TXRX: In %s Tx not finished: w %ld: r %ld!\n", 
                          __func__, tp->smc_tx_wr, tp->smc_tx_rd);
			tp->smc_tx_rd=tp->smc_tx_wr;
			if (tp->force_tx_rx_triger)
				tp->force_tx_rx_state = 2;	
			spin_unlock_irq(&dev->smc_spinlock);
			return -ETIMEDOUT;
		}
		spin_unlock_irq(&dev->smc_spinlock);
	}	

    return 0;
}

static int __smc_txrx_wait_finish_timeout_type_two(struct smc_device *dev, 
                                                        void __iomem *p, 
                                                        int reply_num, UINT16 *actsize)
{
    UINT32 u32_rem_space;
	UINT32 i;
	UINT16 c, u16_total_cnt;
	/* <== patch for miss tx finish flag */
	UINT32  u32_cur_rx_head, u32_smc_rd_tmo, u32_tmp_rd_tick;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    if (reply_num)
	{
		if (tp->smc_rx_tail)
		{	
			u32_cur_rx_head = tp->smc_rx_head;
			if (u32_cur_rx_head)
				u32_smc_rd_tmo = tp->cwt;
			else
				u32_smc_rd_tmo = tp->first_cwt;
            
			u32_tmp_rd_tick = jiffies;
			while (tp->smc_rx_head < tp->smc_rx_tail)
			{
				if (0 == tp->inserted)
				{
					smc_debug(KERN_ERR "SMC TXRX: In %s Smart card not inserted!\n", __func__);
					return -EIO;
				}
				mdelay(1);
                
				if (u32_cur_rx_head != tp->smc_rx_head)
				{
					u32_cur_rx_head = tp->smc_rx_head;
					u32_smc_rd_tmo = tp->cwt;
					u32_tmp_rd_tick = jiffies;
				}
                
				if (u32_smc_rd_tmo < (jiffies - u32_tmp_rd_tick))
				{	
					u32_smc_rd_tmo = 0;
					break;
				}
			}

			spin_lock_irq(&dev->smc_spinlock);
			u32_rem_space = tp->smc_rx_tail - tp->smc_rx_head;
			u16_total_cnt = c =smc_read_rx(tp, p);
			c = (c <= u32_rem_space ? c : u32_rem_space);			
			u16_total_cnt -= c;
			for (i = 0; i < c; i++)
				tp->smc_rx_buf[tp->smc_rx_head + i] = readb(p + REG_RBR);
			tp->smc_rx_head += c; 
			spin_unlock_irq(&dev->smc_spinlock);
			for (i = 0; i < u16_total_cnt; i++)		
				c = readb(p + REG_RBR);
			smc_debug("SMC TXRX: In %s TMO with %ld, rem %d \n", 
                          __func__, tp->smc_rx_head, u16_total_cnt);
		}
        
		*actsize = tp->smc_rx_head;
	
		if(tp->smc_rx_head)
		{
			smc_txrx_buf_reset(tp);
    		return SMC_FAKE_RETURN_VALUE; /* Fake return for caller */
		}
		smc_debug(KERN_ERR "SMC TXRX: In %s Failed \n", __func__); 	
		smc_txrx_buf_reset(tp);	
		return -ENODATA;
	}
	else
	{
		if (tp->smc_tx_rd != tp->smc_tx_wr)
		{
			*actsize = tp->smc_tx_rd;
			return -ENODATA;
		}
	}

    return 0;
}

static int smc_txrx_wait_for_byte(struct smc_device *dev, void __iomem *p, 
                                    int *wait_ret, UINT16 *actsize)
{
    
    UINT32 t_wait_tmo = 6000;
    int ret_wq;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    
    t_wait_tmo = tp->first_cwt;    
    ret_wq = wait_event_timeout(dev->smc_wq_head, dev->wq_flags & (SMC_REMOVED | \
                                SMC_RX_FINISHED | SMC_RX_BYTE_RCV), t_wait_tmo);
     
	if (0 == ret_wq)
	{
		*actsize = tp->smc_rx_head;
		smc_debug("SMC TXRX: In %s Wait 1st byte TMO with %ld\n", 
                      __func__, tp->smc_rx_head);  
        /*smc_misc_set_card_flags(dev, dev->wq_flags & ~(SMC_RX_FINISHED | \
                                SMC_TX_FINISHED | SMC_RX_BYTE_RCV));*/
        smc_misc_unset_card_flags(dev, SMC_RX_FINISHED | \
                                SMC_TX_FINISHED | SMC_RX_BYTE_RCV);
		smc_txrx_buf_reset(tp);
        *wait_ret = ret_wq;
		return -ETIMEDOUT;
	}
    
	if (dev->wq_flags & SMC_RX_BYTE_RCV)
	{
		UINT16 current_cnt = smc_read_rx(tp, p);
		UINT32 current_head = tp->smc_rx_head;
		
        //smc_misc_set_card_flags(dev, dev->wq_flags & ~SMC_RX_BYTE_RCV);
        smc_misc_unset_card_flags(dev, SMC_RX_BYTE_RCV);
		t_wait_tmo = tp->cwt;
		do
		{
			UINT16 tmp_cnt;
            ret_wq = wait_event_timeout(dev->smc_wq_head, dev->wq_flags & (SMC_REMOVED | \
                                        SMC_RX_FINISHED), t_wait_tmo);
           
			if (0 != ret_wq)		
				break;			
			tmp_cnt = smc_read_rx(tp, p);
			if (tmp_cnt != current_cnt || current_head != tp->smc_rx_head)
			{
				current_cnt = tmp_cnt;
				current_head=tp->smc_rx_head;
                ret_wq = 0xff; /* This is just to maintain the loop */
			}
		}while (ret_wq != 0);

        *wait_ret = ret_wq;
	}
    return 0;
}

/*
 *  \param: buffer this is a kernel buffer, not from user
 */
int smc_txrx_transfer_data(struct smc_device *dev, void __iomem *p,
                             UINT8 *buffer, UINT16 size, UINT8 *recv_buffer, 
                             UINT16 reply_num, UINT16 *actsize)
{
	//UINT16 i, j;
	UINT32 loop = 0;
	UINT32 t_wait_tmo = 6000;
    int ret = 0;
    unsigned long ret_wq = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    smc_debug("SMC TXRX: In %s: Input %d, Output %d size %ld\n", \
                  __func__, size, reply_num, \
                  reply_num ? tp->smc_rx_head : tp->smc_tx_rd);
    smc_dump("SMC TXRX: ", buffer, size);
    
	if ((ret = smc_txrx_preset_check(tp, buffer, size, recv_buffer, reply_num)) < 0)
        goto out;
    	
	if (size > tp->smc_tx_fifo_size)
		loop = 1;
    smc_txrx_config_rx_receiver(tp, p, reply_num);

    if ((ret = smc_txrx_config_tx_rx_trigger(dev, p, size, loop)) < 0)
        goto out;

	/* Wait TX ready */
	t_wait_tmo = ((tp->smc_tx_wr + 1) * tp->char_frm_dura * 3) / 1000 + 1;	

    /* For some S3701C board, the IRQ will be delayed, then */
    /* We don't have the right data to be replied, so we wait */
    /* sometimes for the IRQ. It may be better to return here */
    /* until we finish the pointer updating, but sleep is a  */
    /* easier way to solve that. RX_FINISHED will be considered */
    /* If new issues happen in the future    ---- Owen ----  */
    /*
    if (SYS_IC_BONDING_TYPE_17(tp)) 
    {
        smc_debug("SMC TXRX: In %s Delay for"
					  " special S3701C board!\n", __func__);
        msleep(20);
    }
    */
    ret_wq = wait_event_timeout(dev->smc_wq_head, dev->wq_flags & (SMC_REMOVED | \
                                SMC_TX_FINISHED), t_wait_tmo);

    /* This means timeout of wait queue */
	if (0 == ret_wq)
	{
        if ((ret = __smc_txrx_wait_finish_timeout_type_one(dev, p)) < 0) 
            goto out;
	}
	
	if (tp->isr0_interrupt_status & SMC_ISR0_PE_TRANS)
	{
		smc_debug("SMC TXRX: In %s TX Parity Error!\n", __func__);
		tp->isr0_interrupt_status &= (~SMC_ISR0_PE_TRANS);
	}
    
	if (tp->smc_rx_tail && tp->smc_rx_buf != tp->smc_rx_tmp_buf && 
       (dev->wq_flags & SMC_TX_FINISHED) && (!(dev->wq_flags & SMC_REMOVED)))
	{
		if ((ret = smc_txrx_wait_for_byte(dev, p, (int *)&ret_wq, actsize)) < 0)
            goto out;
	}
	
	if (0 == ret_wq)
	{		
		if ((ret = __smc_txrx_wait_finish_timeout_type_two(dev, p, reply_num, actsize)) < 0)
		{
            if (SMC_FAKE_RETURN_VALUE == ret) ret = 0; /* This is the fake return of success */
            goto out;
		}
	}
    
	if (dev->wq_flags & SMC_REMOVED)
	{
        //smc_misc_set_card_flags(dev, dev->wq_flags & ~SMC_REMOVED);
        smc_misc_unset_card_flags(dev, SMC_REMOVED);
		*actsize = 0;
		smc_debug("SMC TXRX: In %s CARD removed!\n\n", __func__);
		smc_txrx_buf_reset(tp);
        ret = -EIO;
		goto out;
	}
    
	if(reply_num)
		*actsize = tp->smc_rx_head;
	else
		*actsize = tp->smc_tx_rd;
    
	smc_debug("SMC TXRX: In %s SUCCESS with %d\n", __func__, *actsize);
    tp->smc_tx_buf = NULL;
	tp->smc_tx_rd = 0;
	tp->smc_tx_wr = 0;

out:
	return ret;
}

/* Get data in Smart card buffer */
ssize_t smc_txrx_read(struct smartcard_private *tp, void __iomem *p)
{
	UINT32 u32_rem_space = tp->smc_rx_tail - tp->smc_rx_head;
	UINT32 i;
	UINT16 c, u16_total_cnt;

	u16_total_cnt = c = smc_read_rx(tp, p);
	c = (c <= u32_rem_space ? c : u32_rem_space);			
	u16_total_cnt -= c;
    
	for (i = 0; i < c; i++)
	    tp->smc_rx_buf[tp->smc_rx_head + i] = readb(p + REG_RBR);
	tp->smc_rx_head += c; 

	return c;
}

/* 
 *  Normally, we'll read enough data, then unblocked 
 *  Sometimes, there is not enough data, then we need wait for timeout
 */
int smc_txrx_data_timeout(struct smartcard_private *tp, size_t size)
{
	UINT32 u32_cur_rx_head;
	UINT32 u32_tmp_rd_tick = jiffies;
    UINT32 u32_smc_rd_tmo = tp->first_cwt;
    
    u32_cur_rx_head = tp->smc_rx_head;
	if (u32_cur_rx_head)
		u32_smc_rd_tmo = tp->cwt;

    /* We read the data in block mode with timeout */
	while (tp->smc_rx_head < ((UINT32)size))
	{
		if (0 == tp->inserted)
		{
			smc_debug(KERN_ERR "SMC TXRX: In %s Smart card not inserted!\n", __func__);
			return -EIO;
		}
		else if (1 != tp->reseted)
		{
			smc_debug(KERN_ERR "SMC TXRX: In %s Smart card not reseted!\n", __func__);
			return -EIO;
		}	
        
		mdelay(1);

		if (u32_cur_rx_head != tp->smc_rx_head)
		{
			u32_cur_rx_head = tp->smc_rx_head;
			u32_smc_rd_tmo = tp->cwt;
			u32_tmp_rd_tick = jiffies;
		}
        
		if (u32_smc_rd_tmo < (jiffies - u32_tmp_rd_tick))
		{	
			return 0;
		}
	}

    /* Something like wait event: 0, timeout; >0, left time; <0, error */
    return u32_smc_rd_tmo;
}

/* ISO7816 transferation command implementation */
static int smc_txrx_card_error_check(struct smartcard_private *tp)
{
    if (0 == tp->inserted)
	{
		smc_debug(KERN_ERR "SMC TXRX: In %s Card not inserted!\n", __func__);
		return -EIO;
	}
	else if (1 != tp->reseted)
	{
		smc_debug(KERN_ERR "SMC TXRX: In %s Card not reseted!\n", __func__);
		return -EIO;
	}

    return 0;
}

int smc_txrx_iso_transfer_len(smc_iso_transfer_t *p_iso_transfer, int *b_write)
{
    int ret_len = 0;
    
    if (p_iso_transfer->num_to_write > CMD_LENGTH)
	{
		*b_write = 1;
		ret_len = p_iso_transfer->command[P3_OFFSET];
	}
	else if (CMD_LENGTH == p_iso_transfer->num_to_write)
	{
		*b_write = 0;
        
		if (0 == p_iso_transfer->num_to_read)
		{
			ret_len = 0;
		}
		else		/*read data from smart card*/
		{
			ret_len = (0 == p_iso_transfer->command[P3_OFFSET]) ? \
			          MAX_LENGTH : p_iso_transfer->command[P3_OFFSET];		
		}
	}
	else
	{
		smc_debug("SMC TXRX: In %s Error command length!\n", __func__);
		return -EINVAL;
	}

	smc_debug("SMC TXRX: In %s Write flag = %d, ret_len = %d\n", 
		__func__, *b_write, ret_len);
	
    return ret_len;
}

static INT32 smc_txrx_process_bytes(struct smc_device *dev, UINT8 INS, 
                                        INT16 num_to_transfer, UINT8 status[2])
{
	INT16 r = 0;
	UINT8 buff = 0;
    
	smc_debug("SMC TXRX: In %s ISO <- PROC\n", __func__);
    
	do
	{
		do
		{
			if (0 == smc_dev_read(dev, &buff, 1)) 
			{
				smc_debug(KERN_ERR "SMC TXRX: In %s [1]PCSS Read error!\n", __func__);
				return -EIO;
			}
            switch(buff)
            {
                case 0x90:
                    smc_debug("SMC TXRX: In %s SW1:0x%x, command normally completed.\n",__func__,buff);
                    break;
                case 0x6e:
                    smc_debug("SMC TXRX: In %s SW1:0x%x, CLA not supported.\n",__func__,buff);
                    break;
                case 0x6d:
                    smc_debug("SMC TXRX: In %s SW1:0x%x, CLA supported, but INS not programmed or invalid.\n",__func__,buff);
                    break;
                case 0x6b:
                    smc_debug("SMC TXRX: In %s SW1:0x%x, CLA INS supported, P1 P2incorrect.\n",__func__,buff);
                    break;
                case 0x67:
                    smc_debug("SMC TXRX: In %s SW1:0x%x, CLA INS P1 P2 supported, but P3 incorrect.\n",__func__,buff); 
                    break;
                case 0x6f:
                    smc_debug("SMC TXRX: In %s SW1:0x%x, command not supported and no precise diagnosis given.\n",__func__,buff);
                    break;
            }
		} while (0x60 == buff);	/* NULL, send by the card to reset WWT */

		if (0x60 == (buff & 0xF0) || 0x90 == (buff & 0xF0))	 /* SW1/SW2 */
		{
			status[0] = buff;
			if (0 == smc_dev_read(dev, &buff, 1)) 
			{
				smc_debug("SMC TXRX: In %s [2]PCSS Read error!\n", __func__);
				return -EIO;
			}
			status[1] = buff;
            smc_debug("SMC TXRX: In %s SW2:0x%x.\n",__func__,status[1]);
			return 0;
		}
		else
		{
            smc_debug("SMC TXRX: In %s ACK = 0x%x, INS = 0x%x \n", __func__,buff,INS );
			if (0 == (buff ^ INS))	/* ACK == INS*/
			{
				/*Vpp is idle. All remaining bytes are transfered subsequently.*/
				r = num_to_transfer;
			}
			else if (0xff == (buff ^ INS))		/* ACK == ~INS*/
			{
				/*Vpp is idle. Next byte is transfered subsequently.*/
				r = 1;
			}
			else if (0x01 == (buff ^ INS))	/* ACK == INS+1*/
			{
				/*Vpp is active. All remaining bytes are transfered subsequently.*/
				r = num_to_transfer;
			}
			else if (0xFE == (buff ^ INS))	/* ACK == ~INS+1*/
			{
				/*Vpp is active. Next bytes is transfered subsequently.*/
				r = 1;
			}
            /*###########################################################*/			
            /* seca exceptions */
			else if ((0x3c == (buff ^ INS)) || (0x40 == (buff ^ INS)))	
			{
				r = num_to_transfer;
			}
            /*###########################################################*/	
			else
			{
				smc_debug("SMC TXRX: In %s Cannot handle procedure %02x (INS = %02x)\n", 
                              __func__, buff, INS);
				return -EINVAL;
			}
            
			if (r > num_to_transfer)
			{
				smc_debug("SMC TXRX: In %s Data overrun r = %d num_to_transfer = %d\n", 
                              __func__, r, num_to_transfer);
				return -EINVAL;

			}
		}
	} while (0 == r);
    
	return r;
}

static inline int __smc_txrx_iso_transfer_command_process(struct smc_device *dev, 
                                                           smc_iso_transfer_t *p_iso_transfer,
                                                           int num_to_transfer, int b_write)
{
	INT16 u16_length = 0;
	INT16 u16_temp_length = 0;
	UINT8 status[2] = {0,0};


    if (0 == smc_dev_write(dev, p_iso_transfer->command, CMD_LENGTH))
	{
		smc_debug("SMC TXRX: In %s Write cmd error!\n", __func__);
		return -EIO;
	}
	
	for ( ; ; )
	{
		u16_temp_length = smc_txrx_process_bytes(dev, p_iso_transfer->command[INS_OFFSET], 
                                                 num_to_transfer - u16_length, status);
        smc_debug("SMC TXRX: In %s u16_temp_length = %d\n", __func__,u16_temp_length);
		if (0 == u16_temp_length)
		{
			if ((NULL != p_iso_transfer->response) && (p_iso_transfer->num_to_read >= 2))
			{	            	
				p_iso_transfer->response[p_iso_transfer->actual_size] = status[0];
				p_iso_transfer->response[p_iso_transfer->actual_size+1] = status[1];
				p_iso_transfer->actual_size += 2;
			}
			return 0; 
		}
		else if (u16_temp_length < 0)
		{
			smc_debug("SMC TXRX: In %s Procedure error! CMD DATA\n", __func__);
			smc_dump("SMC TXRX: ", p_iso_transfer->command, (unsigned int)p_iso_transfer->num_to_write);
			return -EIO;
		}

		if (b_write)
		{
			if (0 == smc_dev_write(dev, 
                                   p_iso_transfer->command + CMD_LENGTH + u16_length, 
                                   u16_temp_length))
			{
				smc_debug(KERN_ERR "SMC TXRX: In %s Write data error!\n", __func__);
				return -EIO;
			}
			smc_debug("SMC TXRX: In %s DATA: IFD -> SMC\n", __func__);
            smc_dump("SMC TXRX: ", p_iso_transfer->command + CMD_LENGTH + u16_length, u16_temp_length);
		}
		else
		{
			if (0 == smc_dev_read(dev, 
                                  p_iso_transfer->response + u16_length, 
                                  u16_temp_length))
			{
				smc_debug(KERN_ERR "SMC TXRX: In %s Read data error!\n", __func__);
				return -EIO;
			}
			p_iso_transfer->actual_size += u16_temp_length;
			smc_debug("SMC TXRX: In %s DATA: IFD <- SMC\n", __func__);
			smc_dump("SMC TXRX: ", p_iso_transfer->response + u16_length, u16_temp_length);
		}
		u16_length += u16_temp_length;
	}

    return 0;
}

int smc_txrx_iso_transfer(struct smc_device *dev, smc_iso_transfer_t *p_iso_transfer)
{
	
	int ret = 0;
	UINT8 b_write = 0;
    INT16 u16_num_to_transfer = 0;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

	if ((ret = smc_txrx_card_error_check(tp)) < 0)
        return ret;

    p_iso_transfer->transfer_err = SMART_NO_ERROR;
	if (tp->T > 1)
	{   
        p_iso_transfer->transfer_err = SMART_INVALID_PROTOCOL;
		return -EINVAL;
	}

	smc_debug("SMC TXRX: In %s CMD: IFD -> SMC\n", __func__);
	smc_dump("SMC TXRX: ", p_iso_transfer->command, /*CMD_LENGTH*/(unsigned int)p_iso_transfer->num_to_write);
	p_iso_transfer->actual_size = 0;
	
	u16_num_to_transfer = smc_txrx_iso_transfer_len(p_iso_transfer, (int *)&b_write);
    if (u16_num_to_transfer < 0) 
    {
    	smc_debug("[ %s %d ], error!\n", __FUNCTION__, __LINE__);
    	return -EINVAL;     
    }

	/* Check the CLA and INS bytes are valid */
	if (0xff != p_iso_transfer->command[CLA_OFFSET])	
	{
		if ((0x60 != ((p_iso_transfer->command[INS_OFFSET]) & 0xF0)) && (0x90 != ((p_iso_transfer->command[INS_OFFSET]) & 0xF0)))				
		{
			ret = __smc_txrx_iso_transfer_command_process(dev, p_iso_transfer, u16_num_to_transfer, b_write);
            if (ret < 0) 
            {
            	smc_debug("[ %s %d ], error!\n", __FUNCTION__, __LINE__);
            	return ret;
            }
		}
		else
		{
			/* INS is invalid */
			p_iso_transfer->transfer_err = SMART_INVALID_CODE;
		}
	}
	else
	{
		/* CLA is invalid */
		p_iso_transfer->transfer_err = SMART_INVALID_CLASS;
	}

	return 0;
}

int smc_txrx_iso_transfer_t1(struct smc_device *dev, smc_iso_transfer_t *p_iso_transfer)
{
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    void __iomem *p = (void __iomem *)dev->io_base;
    int ret = 0;

    p_iso_transfer->transfer_err = SMART_NO_ERROR;

	if ((ret = smc_txrx_card_error_check(tp)) < 0)
        return ret;

	if ((ret = smc_txrx_transfer_data(dev, p, p_iso_transfer->command, 
                               p_iso_transfer->num_to_write, 
                               p_iso_transfer->response, 
                               p_iso_transfer->num_to_read, 
                               (UINT16 *)(&p_iso_transfer->actual_size))) < 0)
	{
        p_iso_transfer->actual_size = 0;
	}

	return ret;
}



