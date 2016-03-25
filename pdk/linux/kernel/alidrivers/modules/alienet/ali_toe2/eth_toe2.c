/*****************************************************************************
 *	Copyright (C)2010 Ali Corporation. All Rights Reserved.
 *
 *	File: eth_toe2.c
 *
 *	Description: ALi Linux Ethernet TOE II Driver for kernel v2.6:
 *			Ver 0.1 is 4 Old MAC,
 *			Ver 0.2 is 4 CostDown MAC
 *			Ver 0.3 is 4 TOE I
 *			Ver 0.4 is 4 TOE II
 *			Ver 0.5 is 4 TOE II simplify phy operate flow
 *	History:
 *	Date				Athor		Version		Reason
 * ===========================================================
 *	1. 03.25.2010		Mao Feng		Ver 0.4		TOE II
 *
 *
 ******************************************************************************/
#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/ctype.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/if_vlan.h>
#include <linux/mii.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/crc32.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <asm/dma.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/mach-ali/m6303.h>
#include <asm/mach-ali/m36_gpio.h>
#include <asm/gpio.h>
#include <linux/platform_device.h>
//#include <linux/wrapper.h>
#include <linux/proc_fs.h>
#include <alidefinition/adf_boot.h>

#ifdef CONFIG_ALI_AS
//#include <mach/unified_bsp_board_attr.h>
//#include <asm/mach-ali/chip.h>
#include <linux/mtd/mtd.h>
//#include <linux/dvb/ali_ce.h>
#endif

#ifdef TOE2_NAPI_DEBUG
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif

#include "eth_reg.h"
#include "eth_toe2.h"
#ifdef UTILITY_SUPPORT
#include "eth_toe2_util.h"
#endif

#define TOE2_DRV_NAME "ALi Ethernet TOE2"
#define TOE2_DRV_VER  "Ver 1.6"

/*chip id*/
#define C3701       0x3701
#define C3821       0x3821      
#define C3921       0x3921

//patch for 38/39
#define MAX_LINK_CHANGE_IN_5SEC 3
static UINT32 mac_clk = 50 * 1024 * 1024;  // 50M
static UINT32 mac_5sec = 0;
static UINT32 mac_pll_time = 0;

//DB-M3912-01v02: C3701
static unsigned char g_RMII_RESET_PIN = 29;

//#define PHY_BLINK_USING_GPIO
#ifdef PHY_BLINK_USING_GPIO
static INT32 g_enet_link_gpio = -1;
static INT32 g_enet_speed_gpio = -1;
static bool  g_enet_gpio_light;
#endif

extern unsigned long __G_ALI_MM_PRIVATE_AREA_START_ADDR;
static u16 g_chip_id = 0;

static bool toe2_use_R8032_patch = false;
static bool toe2_physet_gpio_vld = false;
static bool toe2_device_open     = false;
static UINT8 toe2_physet_gpio    = 0;

typedef struct toe2_dev_info
{
	UINT32 flag;		//must be 0x7938b5ed, flag标志位,"0x7938b5ed"表示这个结构体信息有效
	UINT8 gpio_valid;	//whether gpio_num valid, 1表示gpio_num有效, 九联项目应该设为"1"
	UINT8 gpio_num;		//gpio number for phy hw_reset_pin, 九联项目应该取值"29"
	UINT8 reserve[10];	//reserve
} *ptoe2_dev_info;

#ifdef TX_ASYNC_XMIT
typedef struct toe2_skb_info
{
	u16 first;
	u16 cnt;
	struct sk_buff *skb;
} tx_skb_info;
u16 tx_skb_wr = 0;
u16 tx_skb_rd = 0;
tx_skb_info g_tx_skb[TOE2_TX_DESC_NUM];
#endif

static int timer_period = 0;
module_param(timer_period, int, 0644);
MODULE_PARM_DESC(timer_period, "timer interrupt period");

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0))
static struct proc_dir_entry *network_dir, *timer_file;
#endif

static UINT32 toe2_base = _TOE2_BASE;
static UINT32 toe2_irq = _TOE2_IRQ;
//static UINT32 toe2_mac_hi16 = 0x00009988UL;
//static UINT32 toe2_mac_lo32 = 0x77665500UL;
static UINT8  toe2_mac_addr[6] = {0x0,0x0,0x0,0x0,0x0,0x0};
static UINT32 toe2_mac_hi16    = 0x00000A00UL;
static UINT32 toe2_mac_lo32    = 0x00E69000UL;
static UINT32 toe2_phy_addr    = TOE2_PHY_ADDR;
static UINT32 toe2_link_mode   = 0x7FUL; //All: 10HD,10FD,100HD,100FD,ASYMP,SYMP,ANEG

#ifdef NAPI_MODE_SUPPORT
static UINT32 toe2_rx_mode     = TOE2_RX_MODE_NAPI;
#else
static UINT32 toe2_rx_mode     = TOE2_RX_MODE_TASKLET;
#endif

//static UINT32 toe2_rx_filter = TOE2_RX_FILTER_PERFECT;
//static UINT32 toe2_timer_frq = 0x31fffff; //50M
//static UINT32 toe2_timer_frq = 0x18fffff; //50M
//static UINT32 toe2_timer_frq = 0xc7ffff; //12.5M

/********************************************************************
VALUE_ADJUST:
toe2_timer_frq --
small value means more timer interrupt
since packet receiving relies on timer interrupt,
it should not be too big, otherwise it will cause bigger latency, 
also it will cause packet dropping, because the ring buffer has limits.
however, it should not be too small, 
because it will cause too many interrupts, which is no need.
 ********************************************************************/
//static UINT32 toe2_timer_frq = 0x63FFFE;//500 times per minute
//    static UINT32 toe2_timer_frq = 0x31FFFF;//1000 times per minute   using this one
//static UINT32 toe2_timer_frq = 0x18FFFE;
//static UINT32 toe2_timer_frq = 0xC7FFF;
//static UINT32 toe2_timer_frq;
/*
   20,000,000/1514 = 13000.
   ingress rate = 1/14000, 
   64 pkts ingress = 0.0045s
   so timer should be arised every 0.0045s
   = 222 timer interrupt per second
   */

#ifdef DYNAMIC_CHANGING_IMK	  //IMK -- interrupt mask
#define TIMER_200_FREQ	      0xc7ffff //almost 200 timer interrupt per minute
#define TIMER_2000_FREQ	      0x18FFFE //almost 2000 timer interrupt per minute 

#define TOO_MANY_RX_INTERRUPT            500 //numbers of pkts in per HZ/2
#define TOO_MANY_USELESS_TIMER_INTERRUPT 500 //times of zero pkt ingress in timer interrupt

#define HIGH_INGRESS_IMK 1
#define LOW_INGRESS_IMK  0

#define JUDGE_TOO_MANY_RX_INTERRUPT_PERIOD (HZ/2)
/********************************************************************
  1.startup:
  the default is TOE2_INTERRUPT_MASK, which enable all kinda interrupts
  the default is TIMER_200_FREQ
  2.more pkt coming: 
judge: in certain period of timer, there are too many RX_COMPLETER interrupts
action: TIMER_200_FREQ -> TIMER_2000_FREQ, TOE2_INTERRUPT_MASK -> TOE2_INTERRUPT_MASK2
3.no pkt coming: 
judge: too many timer interrupts, but with no packets ingress
action: TIMER_2000_FREQ -> TIMER_200_FREQ, TOE2_INTERRUPT_MASK2 -> TOE2_INTERRUPT_MASK

in short, there are two types of interrupt, HIGH_INGRESS_IMK & LOW_INGRESS_IMK,
LOW_INGRESS_IMK -- for startups and low ingress rate,
HIGH_INGRESS_IMK -- for high ingress rate.
 ********************************************************************/
#endif

#ifdef NAPI_MODE_SUPPORT
static int toe2_rmii       = true;
static int toe2_rx_csum    = true;
static int toe2_tx_csum    = true;
static int toe2_sg         = true;
static int toe2_tso        = true;
static int toe2_ufo        = true;
static int toe2_debug      = -1;
static int toe2_reversemii = false;
#else
static int toe2_rmii       = true;
static int toe2_rx_csum    = false;
static int toe2_tx_csum    = false;
static int toe2_sg         = false;
static int toe2_tso        = false;
static int toe2_ufo        = false;
static int toe2_debug      = -1;
static int toe2_reversemii = false;
#endif

ptoe2_private g_ptoe2;

static struct net_device_stats *toe2_get_stats(struct net_device *dev);

static void mac_rx_tasklet(unsigned long para);
static void mac_weird_tasklet(unsigned long para);
static void mac_tx_sts(ptoe2_private ptoe2, ptx_desc desc_sw);
//static void phy_set(ptoe2_private ptoe2);
static void phy_link_chk(ptoe2_private ptoe2);
static void toe2_set_rx_mode(struct net_device *dev);

DECLARE_TASKLET(toe2_tasklet, mac_rx_tasklet, 0);
DECLARE_TASKLET(toe2_tasklet2, mac_weird_tasklet, 0);

//#define WORK_QUEUE_TEST
#ifdef WORK_QUEUE_TEST
struct toe2_wq
{
	struct sk_buff_head	special_pkt_queue;  //this is the contain of the work.
	struct work_struct  special_pkt_work;   //this is the work.
};

struct toe2_wq my_wq;
#endif

/********************************************************************************************/
//things needed to put information in PROC fs.

#ifdef TOE2_NAPI_DEBUG
static void *toe2_seq_start(struct seq_file *seq, loff_t *pos)
{
	if (!*pos)
		return SEQ_START_TOKEN;

	return NULL;
}

static void *toe2_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	return NULL;
}

static void toe2_seq_stop(struct seq_file *seq, void *v)
{
	//return NULL;
}

static int toe2_seq_show(struct seq_file *seq, void *v)
{
	ptoe2_private tmp_ptoe2 = g_ptoe2;

	if (v == SEQ_START_TOKEN)
	{	
		seq_printf(seq, "haha, TOE2: been_irq = %d, rx_int = %d, time_int = %d!\n", \
				tmp_ptoe2->has_been_interrupted, tmp_ptoe2->irq_due_to_rx_complete, tmp_ptoe2->irq_due_to_timer);
		seq_printf(seq, "haha, TOE2: rx_small_times = %d, rx_big_times = %d!\n", \
				tmp_ptoe2->rx_small_times, tmp_ptoe2->rx_big_times);
		seq_printf(seq, "haha, TOE2: num_rx_int = %d, rx_time_int = %d, cur_imr = %s!\n", \
				tmp_ptoe2->num_rx_complete, tmp_ptoe2->num_timer, (tmp_ptoe2->cur_dny_imk)?"HIGH_IMK":"LOW_IMK");
	}
	return 0;
}

static const struct seq_operations toe2_seq_ops =
{
	.start = toe2_seq_start,
	.next  = toe2_seq_next,
	.stop  = toe2_seq_stop,
	.show  = toe2_seq_show,
};

static int toe2_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &toe2_seq_ops);
}

static const struct file_operations toe2_seq_fops =
{
	.owner	 = THIS_MODULE,
	.open    = toe2_seq_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};
#endif

/********************************************************************************************/

//Debug Use.
/*
   static void 
   toe2_print(unsigned char *p, unsigned short len)
   {
   int i;
   for(i=0; i<len; i++)
   {
   if (i%16 == 0)
   {
   printk("\n0x%08lx:  ", (UINT32)(p+i));
   }

   printk("%02x ", *(p+i));
   }
   printk("\n");
   }
   */

static void toe2_enable_irq(ptoe2_private ptoe2)
{
	UINT32 ba = ptoe2->io_base;
#ifdef NAPI_MODE_SUPPORT	
#ifdef DYNAMIC_CHANGING_IMK
	TOE2_W32(IMR, ptoe2->isr_mask);//it is dynmic
	TOE2_W32(TimerR, ptoe2->timer_freq);//DYNAMIC_CHANGING_IMK, the actually changing happens here!!
#else
	TOE2_W32(IMR, TOE2_INTERRUPT_MASK2);
#endif
#else
	TOE2_W32(IMR, TOE2_INTERRUPT_MASK);
#endif
}

static void toe2_disable_irq(ptoe2_private ptoe2)
{
	UINT32 ba = ptoe2->io_base;
	TOE2_W32(IMR, 0);
}

/*************************************************************************************************/

#ifdef WORK_QUEUE_TEST
static void my_wq_print(unsigned char *p, unsigned short len)
{
	int i;
	for (i=0; i<len; i++)
	{
		if (i%16 == 0)
		{
			TOE2_TRACE(KERN_INFO "\n0x%08lx:  ", (UINT32)(p+i));
		}
		TOE2_TRACE(KERN_INFO "%02x ", *(p+i));
	}
	TOE2_TRACE(KERN_INFO "\n");
}

static void my_wq_handler(struct work_struct *work)
{
	struct toe2_wq *p_my_wq =
		container_of(work, struct toe2_wq, special_pkt_work);

	struct sk_buff *skb;

	TOE2_TRACE("enter my_wq_handler");

	skb = __skb_dequeue(&p_my_wq->special_pkt_work);

	my_wq_print(skb->data, skb->len + RING_CRC_SZ);
}
#endif

/*************************************************************************************************/

#ifdef DYNAMIC_CHANGING_IMK
static void dny_imk2low(ptoe2_private ptoe2)
{
	ptoe2->isr_mask = TOE2_INTERRUPT_MASK; 
	ptoe2->cur_dny_imk = LOW_INGRESS_IMK;
	ptoe2->timer_freq = TIMER_200_FREQ;
}

static void dny_imk2high(ptoe2_private ptoe2)
{
	ptoe2->isr_mask = TOE2_INTERRUPT_MASK2; //ignores RX_COMPLETE interrupt
	ptoe2->cur_dny_imk = HIGH_INGRESS_IMK;
	ptoe2->timer_freq = timer_period*50000/*TIMER_2000_FREQ*/;
}
#endif

/**************************************************************************************************
  Mdio.
 **************************************************************************************************/
#ifndef HW_MDIO
#define	mac_mdio_delay(mdio_addr)	TOE2_R32(mdio_addr)

//mdio preamble: 32 "1"s.
//provides synchronization for the PHY.
static void mac_mdio_sync(ptoe2_private ptoe2)
{
	UINT32 ba = ptoe2->io_base;
	int i;

	for (i = 32; i >= 0; i--)
	{
		TOE2_W32(MiiMR1, MiiMdioWrite1|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
		TOE2_W32(MiiMR1, MiiMdioWrite1|MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}
}
#endif //HW_MDIO

static int mac_mdio_read(struct net_device *dev, int phy_addr, int reg_addr)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;

#ifdef HW_MDIO
	u16 tmp_16;
	UINT32 tmp_32;
	UINT32 addr;

	addr = ((phy_addr << MiiPhyAddrOff) & MiiPhyAddrMask)|((reg_addr << MiiRegAddrOff) & MiiRegAddrMask);

	tmp_32 = TOE2_R32(MiiMR2);
	tmp_32 &= ~(MiiPhyAddrMask|MiiRegAddrMask|MiiOpMask);

	TOE2_W32(MiiMR2, (tmp_32|addr|MiiOpRead|MiiTransStart));

	do
	{
		tmp_32 = TOE2_R32(MiiMR2);
	} while(tmp_32&MiiTransStart);

	tmp_16 = TOE2_R16(MdioR);

	return (UINT32)tmp_16;
#else
	UINT32 retval = 0;
	int i,	data_val = 0;
	UINT32 mii_cmd = (0xf6 << 10) | (phy_addr << 5) | reg_addr;
	UINT32 tmp_u32, cmd_u32;

	mac_mdio_sync(ptoe2);

	/* Shift the read command bits out. */
	for (i = 15; i >= 0; i--) 
	{
		data_val = (mii_cmd & (1 << i)) ? MiiMdioOut : 0;

		TOE2_W32(MiiMR1, MiiMdioDir|data_val|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		TOE2_W32(MiiMR1, MiiMdioDir|data_val|MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}

	/* Read the two transition, 16 data, and wire-idle bits. */
	for (i = 19; i > 0; i--)
	{
		TOE2_W32(MiiMR1, 0|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		cmd_u32 = TOE2_R32 (MiiMR1);
		retval = (retval << 1) | ((cmd_u32 & MiiMdioIn) ? 1 : 0);

		TOE2_W32(MiiMR1, MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}

	return (retval >> 1) & 0xffff;
#endif //HW_MDIO
}

static void mac_mdio_write(struct net_device *dev, int phy_addr, int reg_addr, int value)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;

#ifdef HW_MDIO
	UINT32 tmp_32;
	UINT32 addr;

	TOE2_W16(MdioW, (u16)value);

	addr = ((phy_addr << MiiPhyAddrOff) & MiiPhyAddrMask)|((reg_addr << MiiRegAddrOff) & MiiRegAddrMask);

	tmp_32 = TOE2_R32(MiiMR2);
	tmp_32 &= ~(MiiPhyAddrMask|MiiRegAddrMask|MiiOpMask);

	TOE2_W32(MiiMR2, (tmp_32|addr|MiiOpWrite|MiiTransStart));

	do
	{
		tmp_32 = TOE2_R32(MiiMR2);
	} while(tmp_32&MiiTransStart);
#else
	UINT32 mii_cmd = (0x5002 << 16) | (phy_addr << 23) | (reg_addr << 18) | value;
	int i;

	mac_mdio_sync(ptoe2);

	/* Shift the command bits out. */
	for (i = 31; i >= 0; i--)
	{
		int data_val = (mii_cmd & (1 << i)) ? MiiMdioWrite1 : MiiMdioWrite0;

		TOE2_W32(MiiMR1, data_val|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		TOE2_W32(MiiMR1, data_val|MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}

	/* Clear out extra bits. */
	for (i = 2; i > 0; i--) 
	{
		TOE2_W32(MiiMR1, 0|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		TOE2_W32(MiiMR1, MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}
#endif //HW_MDIO
}

#ifdef PHY_BLINK_USING_GPIO
static void link_light_blink(ptoe2_private ptoe2)
{
    if (!ptoe2->transmit_okay)
	{
	    return;
    }
    if (ptoe2->blink_light)
	{
	    ptoe2->blink_light = 0;
        ptoe2->in_blink = 1;
        ali_gpio_set_value(g_enet_link_gpio, !g_enet_gpio_light);
        return;
    }
    if (ptoe2->in_blink)
	{
        ptoe2->in_blink = 0;
        ali_gpio_set_value(g_enet_link_gpio, g_enet_gpio_light);
    }

	if (((UINT32)100 == ptoe2->link_speed) && (g_enet_speed_gpio >= 0))
	{
        ali_gpio_set_value(g_enet_speed_gpio, g_enet_gpio_light);
	}
	
    return;
}
#endif

static void mac_chip_rst(UINT32 ba)
{
	u8 tmp_u8;
	TOE2_INFO("TOE2: %s()=>O", __FUNCTION__);
	TOE2_W8(SCR, SCRReset|TOE2_R8(SCR));	
	do
	{
		TOE2_INFO("->");	
		tmp_u8 = TOE2_R8(SCR);
	} while(tmp_u8 & SCRReset);
	TOE2_INFO("K!\n");
}

static void mac_cnt_init(ptoe2_private ptoe2)
{
	ptoe2->cur_isr = 0;

#ifdef NAPI_MODE_SUPPORT	
#ifdef DYNAMIC_CHANGING_IMK
	dny_imk2low(ptoe2);
	ptoe2->spot_rx_complete_int_HZ = 0;
	ptoe2->num_rx_complete_int_HZ = 0;
	ptoe2->num_zero_timer_int_HZ = 0;
#else
	ptoe2->isr_mask = TOE2_INTERRUPT_MASK2; 
#endif
#else
	ptoe2->isr_mask = TOE2_INTERRUPT_MASK; 
#endif

	//ptoe2->isr_mask = (ISRTimer|ISRLinkStatus);
	//ptoe2->acquired_isr = false;

	spin_lock_init(&ptoe2->lock);	

	ptoe2->rx_wptr = TOE2_RX_DESC_NUM -1;
	ptoe2->rx_bptr= 0;
	ptoe2->tx_wptr= 0;	

	ptoe2->phy_reset = false;
	ptoe2->auto_n_completed = false;
	ptoe2->link_established = false;
	ptoe2->transmit_okay = false;

	ptoe2->pause_frame_rx = false;
	ptoe2->pause_frame_tx = false;

#if (TOE2_VLAN_TAG_USED)
	ptoe2->vlan_tag_remove = false;
#endif //TOE2_VLAN_TAG_USED

#ifdef TOE2_NAPI_DEBUG
	ptoe2->during_pkt_rx = false;
	ptoe2->has_been_interrupted = 0;
	ptoe2->total_interrupt = 0;
	ptoe2->irq_due_to_rx_complete = 0;				
	ptoe2->irq_due_to_timer = 0;
	ptoe2->polling_times = 0;
	ptoe2->rx_small_times = 0;
	ptoe2->rx_big_times = 0;
#endif
	ptoe2->cur_dny_imk = LOW_INGRESS_IMK;

#ifdef DYNAMIC_CHANGING_IMK_DBG
	ptoe2->num_rx_complete = 0;
	ptoe2->num_timer = 0;
#endif	
}

static int mac_rx_refill(ptoe2_private ptoe2)
{
	struct net_device *dev = ptoe2->dev;
	int i;

	for (i = 0; i < TOE2_RX_DESC_NUM; i++)
	{
		struct sk_buff *skb;
		prx_desc rx_desc = &ptoe2->rx_desc[i];

		if (NULL == ptoe2->rx_skb[i])
		{
			skb = netdev_alloc_skb(dev, TOE2_BUF_SZ);
			if (!skb)
			{
				goto refill_err_out;
			}

			//skb_reserve(skb, NET_IP_ALIGN);
			rx_desc->pkt_buf_dma = dma_map_single((struct device *)NULL, skb->data,	TOE2_BUF_SZ, DMA_FROM_DEVICE);
			ptoe2->rx_skb[i] = skb;
		}
	}
	return 0;

refill_err_out:
	printk("[TOE2] %s()=>fatal error, alloc skb failed.\n", __FUNCTION__);
	return -ENOMEM;
}

static void mac_alloc_rings(ptoe2_private ptoe2)
{
	memset(ptoe2->rx_desc, 0, (TOE2_DESC_SZ * TOE2_RX_DESC_NUM));
	memset(ptoe2->tx_desc, 0, (TOE2_DESC_SZ * TOE2_TX_DESC_NUM));
	memset(ptoe2->setup_buf, 0, SETUP_FRAME_SZ);
}

static void mac_desc_clean(ptoe2_private ptoe2)
{
	unsigned i;
	prx_desc rx_desc;
	ptx_desc tx_desc;

	for (i = 0; i < TOE2_RX_DESC_NUM; i++)
	{
		if (ptoe2->rx_skb[i])
		{
			rx_desc = &ptoe2->rx_desc[i];
			dma_unmap_single((struct device *)NULL, rx_desc->pkt_buf_dma, TOE2_BUF_SZ, DMA_FROM_DEVICE);
			dev_kfree_skb_any(ptoe2->rx_skb[i]);
			ptoe2->rx_skb[i] = NULL;
		}
	}

#ifndef TX_ASYNC_XMIT
	for (i = 0; i < TOE2_TX_DESC_NUM; i++)
	{
		if (ptoe2->tx_skb[i])
		{
			tx_desc = &ptoe2->tx_desc[i];
			dma_unmap_single((struct device *)NULL, tx_desc->pkt_buf_dma, TOE2_BUF_SZ, DMA_TO_DEVICE);			
			dev_kfree_skb_any(ptoe2->tx_skb[i]);
			ptoe2->tx_skb[i] = NULL;
		}
	}
#else
	while (tx_skb_rd != tx_skb_wr)
	{
		if (g_tx_skb[tx_skb_rd].skb)
		{
			dev_kfree_skb_any(g_tx_skb[tx_skb_rd].skb);
			g_tx_skb[tx_skb_rd].first = 0;
			g_tx_skb[tx_skb_rd].cnt = 0;
			g_tx_skb[tx_skb_rd].skb = NULL;
		}
		if (tx_skb_rd+1 == TOE2_TX_DESC_NUM)
			tx_skb_rd = 0;
		else
			tx_skb_rd++;
	}
	tx_skb_rd = 0;
	tx_skb_wr = 0;

	for (i=0; i < TOE2_TX_DESC_NUM; i++)
	{
		tx_desc = &ptoe2->tx_desc[i];
		if (tx_desc->seg_len)
		{
			dma_unmap_single((struct device *)NULL, tx_desc->pkt_buf_dma, tx_desc->seg_len, DMA_TO_DEVICE);
			tx_desc->seg_len = 0;
		}
	}
#endif
}

static void mac_hw_start(ptoe2_private ptoe2)
{
	UINT32 ba = ptoe2->io_base;
	u8 tmp_8;
	UINT32 tmp_u32;
	UINT32 duplex_mode = 0;
	UINT32 pause_frame = 0;
	int re = 0;

	if (netif_msg_ifup(ptoe2))
		TOE2_TRACE("mac_hardware_start()...");

	//tmp_u32 = MAC_R32(DelayControlR);
	//tmp_u32 &= ~CBR_DW_DLY;
	//MAC_W32(DelayControlR, tmp_u32 | (1<<CBR_DW_DLY_OFF));

	TOE2_W8(ClkDelayChainSR, 0XE);
	//TOE2_W32(BackPressure, 0X784c5);

	//set mac address.
	TOE2_W32(PAR, toe2_mac_lo32);
	TOE2_W32(PAR - 4, toe2_mac_hi16);

	if (toe2_rmii == true)
	{
		UINT32 rmii_speed = 0;
		if (ptoe2->link_speed == (UINT32)100)
			rmii_speed = (UINT32)RmiiCrSpeed;	//100Mbps
		else
			rmii_speed = (UINT32)0; //10Mbps

		//Set RMII.
		tmp_u32 = TOE2_R32(RmiiCR);
		tmp_u32 &= ~(RmiiCrSpeed);
		TOE2_W32(RmiiCR, (tmp_u32 |rmii_speed|RmiiEn));
	}

	if (ptoe2->duplex_mode)
		duplex_mode = (UINT32)FullDuplexMode;
	else
		duplex_mode = (UINT32)0;

	if (toe2_rx_csum == true)
		duplex_mode |= RxTOEWorkMode;

	//config network operation mode.
	TOE2_W32(NetworkOM, (duplex_mode |NetworkOMConfig));// |WorkMode_LoopBack));

	//test mux
	TOE2_W8(0x58, 0x0F); 
	//TOE2_W32(0x84, 0x5FF); 

	if (ptoe2->pause_frame_rx)
		pause_frame |= (UINT32)RxFlowControlEn;
	if (ptoe2->pause_frame_tx)
		pause_frame |= (UINT32)TxFlowControlEn;

	// 1. Increase IPG time to ensure the gap between 2 packets > mini IPG time
	// 2. decrease the first portion of the interframe gap time
	TOE2_W8(TxRxCR1, 0xdf);
	TOE2_W8(TxRxCR1+1, 0x00);

	//VLAN Patch 1/3 For receiving packet length 1517 & 1518, we need to add a patch here.
	//tmp_u32 = TOE2_R32(RxChkSumStartOff);
	//tmp_u32 &= ~(0xfff);
	//tmp_u32 |= 0x5f7;
	//TOE2_W32(RxChkSumStartOff, tmp_u32);

	tmp_u32 = TOE2_R32(TxRxCR2);

	//VLAN Patch 2/3 For receiving packet length 1517 & 1518, we need to add a patch here.
	//tmp_u32 |= RxMaxLenEn;

	tmp_u32 &= 0xf7ffffff;
	tmp_u32 |= 0x04000000;
	tmp_u32 &= ~TxFifoThMask;
	tmp_u32 |= (0x2<<TxFifoThOff);
	//tmp_u32 |= (0x6<<TxFifoThOff);
	//tmp_u32 |= (RxFifoSz|TxFifoSz)
	tmp_u32 &= ~RxFlowControlEn;
	tmp_u32 &= ~TxFlowControlEn;

#if (TOE2_VLAN_TAG_USED)
	if (ptoe2->vlan_tag_remove)
		TOE2_W32(TxRxCR2, (pause_frame |tmp_u32|TxRxConfig2|RxRemoveVlanTagEn|VlanEn));
	else
		TOE2_W32(TxRxCR2, (pause_frame |tmp_u32|TxRxConfig2|VlanEn));
#else
	TOE2_W32(TxRxCR2, (pause_frame|tmp_u32 |TxRxConfig2));
#endif //TOE2_VLAN_TAG_USED

	TOE2_W32(TSAD, ptoe2->tx_desc_dma);
	TOE2_W32(RSAD, ptoe2->rx_desc_dma);

	TOE2_W16(RxDesTotNum, TOE2_RX_DESC_NUM);
	TOE2_W16(TxDesTotNum, TOE2_TX_DESC_NUM);

	TOE2_W16(RxRingDesWPtr, TOE2_RX_DESC_NUM - 1);
	TOE2_W16(TxRingDesWPtr, 0);

#ifdef DYNAMIC_CHANGING_IMK
	if (ptoe2->timer_freq != TIMER_200_FREQ)
		TOE2_TRACE("TOE2: weird timer!");
	TOE2_W32(TimerR, ptoe2->timer_freq);
#else
	TOE2_W32(TimerR, toe2_timer_frq);
#endif

	toe2_set_rx_mode(ptoe2->dev);
	re = mac_rx_refill(ptoe2);
	if (re)
	{
		//asm("sdbbp");
	}

	tmp_8 = (SCRRxEn|SCRTxEn);
	if (toe2_tso)
		tmp_8 |= (SCRTxCoeEn|SCRTsoEn);
	if (toe2_ufo)
		tmp_8 |= (SCRTxCoeEn|SCRUfoEn);
	if (toe2_tx_csum)
		tmp_8 |= SCRTxCoeEn;

	if (toe2_rx_csum)
		tmp_8 |= SCRRxCoeEn;
	TOE2_W8(SCR, tmp_8);

	// Enable all surported interrupts.
	TOE2_W32(IMR, ptoe2->isr_mask);
}

static void mac_free_rings(ptoe2_private ptoe2)
{
	mac_desc_clean(ptoe2);	

	if (ptoe2->acquired_isr)
	{
		free_irq(ptoe2->irq_num, (void *)ptoe2->dev);
		ptoe2->acquired_isr = false;
	}
}

static void do_dll_pd_reset(void)
{
    UINT32 tmp_val = 0;
    TOE2_TRACE("WARNING!!! DLL PD need reset WARNING!!!"); 
    tmp_val = SOC_R32(0x640);
    tmp_val |= (1<<1);
    SOC_W32(0x640, tmp_val);
	
    mdelay(250);
	
    tmp_val = SOC_R32(0x640);
    tmp_val &= ~(1<<1);
    SOC_W32(0x640, tmp_val);
    TOE2_TRACE("WARNING!!! DLL PD reset completed WARNING!!!");  
}

static void mac_link_changed(ptoe2_private ptoe2)
{
    UINT32 ba = ptoe2->io_base;
	
    if (false == ptoe2->link_established)
	{
	    TOE2_TRACE("%s link_established shouln't be false!", __FUNCTION__);
        return;
    }
	
    if (toe2_rx_mode == TOE2_RX_MODE_NAPI)
	{
		napi_enable(&ptoe2->napi);
	}

	if (toe2_reversemii == false)
	{
		phy_link_chk(ptoe2);
	}
	else
	{
		if (TOE2_R32(RmiiMode) & 0x00000010)
		{
			ptoe2->duplex_mode = 1;
		}
		else
		{
			ptoe2->duplex_mode = 0;
		}
	}
			
#ifdef PHY_BLINK_USING_GPIO
    if ((UINT32)100 == ptoe2->link_speed)
	{
        //TOE2_TRACE("100M, light speed led\n");
        if (g_enet_speed_gpio >= 0)
	    {
            ali_gpio_set_value(g_enet_speed_gpio, g_enet_gpio_light);
	    }
    }
	else
	{
        //TOE2_TRACE("non 100M, disable speed led\n");
        if (g_enet_speed_gpio >= 0)
	    {
            ali_gpio_set_value(g_enet_speed_gpio, !g_enet_gpio_light);
	    }
    }
#endif

    mac_hw_start(ptoe2);
    netif_carrier_on(ptoe2->dev);
    netif_start_queue(ptoe2->dev);
    ptoe2->transmit_okay = true;
}

static void mac_init_for_link_established(ptoe2_private ptoe2)
{
    UINT32 base_addr = ptoe2->io_base;
    mac_desc_clean(ptoe2);
    mac_chip_rst(base_addr);

#ifdef NAPI_MODE_SUPPORT	
#ifdef DYNAMIC_CHANGING_IMK
	dny_imk2low(ptoe2);
	ptoe2->spot_rx_complete_int_HZ = 0;
	ptoe2->num_rx_complete_int_HZ = 0;
	ptoe2->num_zero_timer_int_HZ = 0;
#else
	ptoe2->isr_mask = TOE2_INTERRUPT_MASK2; 
#endif
#else
	ptoe2->isr_mask = TOE2_INTERRUPT_MASK; 
#endif

    ptoe2->cur_isr = 0;

    ptoe2->rx_wptr = TOE2_RX_DESC_NUM -1;
    ptoe2->rx_bptr= 0;
    ptoe2->tx_wptr= 0;

    ptoe2->phy_reset = false;

    ptoe2->pause_frame_rx = false;
    ptoe2->pause_frame_tx = false;

#if (TOE2_VLAN_TAG_USED)
    ptoe2->vlan_tag_remove = false;
#endif
}

static void mac_link_established(ptoe2_private ptoe2)
{
    UINT32 ba = ptoe2->io_base;
    TOE2_W16(IMR, 0);
    //TOE2_W8(SCR, 0);
    mac_init_for_link_established(ptoe2);
    ptoe2->phy_reset = true;
    TOE2_W16(IMR, (ISRTimer|ISRLinkStatus));
    mac_link_changed(ptoe2);

#ifdef PHY_BLINK_USING_GPIO
	//TOE2_TRACE("light link gpio\n");
	if (g_enet_link_gpio >= 0)
	{
        ali_gpio_set_value(g_enet_link_gpio, g_enet_gpio_light);
	}
#endif

    ptoe2->unlink_error_state = 0;
}

static void mac_disconnect(ptoe2_private ptoe2)
{
	UINT32 ba = ptoe2->io_base;

	TOE2_W16(IMR, (ISRTimer|ISRLinkStatus));

	if (toe2_rx_mode == TOE2_RX_MODE_NAPI)
		napi_disable(&ptoe2->napi);

	netif_stop_queue(ptoe2->dev);
	netif_carrier_off(ptoe2->dev);	

	//if (ptoe2->link_partner != 0)
	{
		ptoe2->link_partner = 0;
		ptoe2->phy_reset = false;
		ptoe2->link_established = false;
		ptoe2->transmit_okay = false;
	}

	//ali_gpio_set_value(g_RMII_RESET_PIN, 0);
	//mdelay(200);
	//ali_gpio_set_value(g_RMII_RESET_PIN, 1);

#ifdef PHY_BLINK_USING_GPIO
	ptoe2->blink_light = 0;
    ptoe2->in_blink = 0;
    if (g_enet_link_gpio >= 0)
    {
	    ali_gpio_set_value(g_enet_link_gpio, !g_enet_gpio_light);
    }
	if (g_enet_speed_gpio >= 0)
	{
        ali_gpio_set_value(g_enet_speed_gpio, !g_enet_gpio_light);
	}
#endif
}

static void mac_set(ptoe2_private ptoe2)
{
	UINT32 ba = ptoe2->io_base;
	UINT32 temp = 0;

	if (ptoe2->phy_reset == false)
	{
		TOE2_TRACE("mac_set()=>PhyReset == false.");
		TOE2_W16(IMR, 0);
		//TOE2_W8(SCR, 0);

		mac_desc_clean(ptoe2);
		mac_chip_rst(ba);
		mac_cnt_init(ptoe2);

		//remove phy_set(ptoe2);
		ptoe2->phy_reset = true;

		TOE2_W16(IMR, (ISRTimer|ISRLinkStatus));
	}	
	else
	{
		TOE2_W16(IMR, 0);
		if (ptoe2->link_established == false)
		{
			TOE2_TRACE("mac_set()=>link not established yet...");
			TOE2_W16(IMR, (ISRTimer|ISRLinkStatus));
		}
		else
		{
			if (toe2_rx_mode == TOE2_RX_MODE_NAPI)
			{
				napi_enable(&ptoe2->napi);
			}

			if (toe2_reversemii == false)
			{
				phy_link_chk(ptoe2);
			}
			else
			{
				temp = TOE2_R32(RmiiMode);
				if (temp & 0x00000010)
				{
					ptoe2->duplex_mode = 1;
				}
				else
				{
					ptoe2->duplex_mode = 0;
				}
			}

			mac_hw_start(ptoe2);

			netif_carrier_on(ptoe2->dev);
			netif_start_queue(ptoe2->dev);

			ptoe2->transmit_okay = true;
		}
	}
}

static void handle_unlink_error(ptoe2_private ptoe2)
{
    UINT16 ctrl = 0; 
    UINT16 status = 0;
    UINT16 reg10 = 0;
    UINT16 tmp = 0;
    
    switch (ptoe2->unlink_error_state)
	{
        case 0: /* state A */ 
            reg10 = (UINT16)mac_mdio_read(ptoe2->dev, toe2_phy_addr, 0x10);
            ctrl = (UINT16)mac_mdio_read(ptoe2->dev, toe2_phy_addr, PhyBasicModeCtrl);
            tmp = (1 << 7);
            /* check whether there is signal on line */
            if (!(reg10 & tmp))
			{ 
                /* no signal on line */
                TOE2_TRACE("[UNLINK] no signal on line");
                return;
            }
            /* check support auto nego and unlink */
            tmp = reg10;
            tmp = (tmp >> 12);
            if (!(ctrl & BMCRANEnable) || (tmp == 6))
			{
                /* doesn't support auto nego or link */
                TOE2_TRACE("[UNLINK] non auto nego or linked");
                return;
            }
            ptoe2->unlink_error_state = 1; /* change state to parallel detection */
            ptoe2->para_detect_times = 0;  
            /* through */
            TOE2_TRACE("[UNLINK] From A to Para_Detect\n");
        case 1:
            /*only when para_detect_times == 0 do parallel detect 41ms * 12 = 500ms*/
            if (ptoe2->para_detect_times >= 12)
			{
                ptoe2->unlink_error_state = 0;
                TOE2_TRACE("[UNLINK] Para_Detect over 500ms, change to A");
                return;
            }
            reg10 = (UINT16)mac_mdio_read(ptoe2->dev, toe2_phy_addr, 0x10);
            /* check link partner support auto nego */
            tmp = reg10 & 0x0F00;
            tmp = (tmp >> 8);
            if (tmp == 3 || tmp == 5)
			{
                ptoe2->unlink_error_state = 0; /* change state to A */ 
                TOE2_TRACE("[UNLINK] link partner support auto nego, change to A");
                return;
            }
            ptoe2->para_detect_times++;
            if (tmp != 4)
			{
                TOE2_TRACE("[UNLINK] do another Para Detect again");
                return;
            }
            ptoe2->unlink_error_state = 2; /* change state to unlink and signal state */ 
            ptoe2->unlink_with_signal_times = 0;
            TOE2_TRACE("[UNLINK] Para Detect ok, change to unlink with no signale state");
            /* through */
        case 2:
            if (ptoe2->unlink_with_signal_times >= 48)
			{
                /* > 2s */
                do_dll_pd_reset(); 
                ptoe2->unlink_error_state = 0;
                return;
            }
            /* check whether there is signal on line */
            reg10 = (UINT16)mac_mdio_read(ptoe2->dev, toe2_phy_addr, 0x10);
            status = (UINT16)mac_mdio_read(ptoe2->dev, toe2_phy_addr, PhyBasicModeStatus);
            tmp = (1 << 7);
            if (!(reg10 & tmp) || (status & BMSRLinkStatus))
			{ 
                /* no signal on line */
                TOE2_TRACE("[UNLINK] no signal on line, change to A");
                ptoe2->unlink_error_state = 0;
                return;
            }
            ptoe2->unlink_with_signal_times++; 
            break;
        default:
            TOE2_TRACE("[UNLINK] something wrong, padapter->unlink_error_state is %d", ptoe2->unlink_error_state);
            ptoe2->unlink_error_state = 0;
            break;
    } 
    return;
}

//int rst_cnt = 0;
static void mac_weird_int(ptoe2_private ptoe2)
{
	u16 ctrl, status;
	bool link_up = false;
	/*
	   if(rst_cnt ++> 10)
	   {
	   printk("\n\nmac rst test...\n\n");
	   ptoe2->phy_reset = false;
	   ptoe2->link_established = false;
	   rst_cnt = 0;
	   }
	   */

	if (ptoe2->link_established == false)
	{
		TOE2_TRACE("%s: link cur status is false.", __FUNCTION__);
		if (toe2_reversemii == false)	
		{
			ctrl = (u16)mac_mdio_read(ptoe2->dev, toe2_phy_addr, PhyBasicModeCtrl);
			status = (u16)mac_mdio_read(ptoe2->dev, toe2_phy_addr, PhyBasicModeStatus);
			if (status & BMSRLinkStatus)
			{
				if ((ctrl & BMCRANEnable) && (status & BMSRANComplete))
				{
					ptoe2->auto_n_completed = TRUE;
					TOE2_TRACE("%s: auto-negotiation completed.", __FUNCTION__);
					link_up = true;
				}
				else if (!(ctrl & BMCRANEnable))
				{
					TOE2_TRACE("%s: non auto-negotiation link established.", __FUNCTION__);
					link_up = true;
				}
				if (link_up)
				{
					ptoe2->link_established = true;
					//mac_set(ptoe2);
					mac_link_established(ptoe2);
					TOE2_TRACE("%s: link established.", __FUNCTION__);
					ptoe2->times_of_link_change++;
				}
			}
			else
			{
				if (C3821 == g_chip_id)
				{
					handle_unlink_error(ptoe2);
				}
				else
				{
				    mac_set(ptoe2);
				}
			}
		}
		else
		{
			//Reversed MII mode, the phy is fake
			ptoe2->auto_n_completed = true;
			ptoe2->link_established= true;
			mac_set(ptoe2);
		}
		
		return;
	}

	TOE2_TRACE("%s: link cur status is true.", __FUNCTION__);
	if (ptoe2->cur_isr & (ISRLinkStatus|ISRTimer))
	{
		TOE2_TRACE("%s: cur_isr is ISRLinkStatus|ISRtimer.", __FUNCTION__);
		if (toe2_reversemii == false)
		{
			status = (u16)mac_mdio_read(ptoe2->dev, toe2_phy_addr, PhyBasicModeStatus);
			if (status & BMSRLinkStatus) //up
			{
				status = (u16)mac_mdio_read(ptoe2->dev, toe2_phy_addr, PhyNWayLPAR);
				if (status != ptoe2->link_partner)
				{
					if (status)
					{
						TOE2_LINK("mac_weird_int()=>link reconnected.");
						//mac_set(ptoe2);
						mac_link_changed(ptoe2);
					}
				}
			}
			else //down
			{
				mac_disconnect(ptoe2);
				ptoe2->times_of_link_change++;
				TOE2_LINK("mac_weird_int()=>link disconnected.");
			}
		}
		else
		{
			//Reversed MII mode, the phy is fake
		}
	}
}

#if 0
int mac_build_setup(struct net_device *dev)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	struct dev_mc_list *mc_list = dev->mc_list;
	UINT8 *pa, *buf_base, *addr;
	UINT32 crc;
	UINT16  bit, byte, hashcode;
	int i, j, rv = 0;

	memset(ptoe2->setup_buf, 0x00, SETUP_FRAME_SZ);

	if (toe2_rx_filter == TOE2_RX_FILTER_HASH)
	{
		pa = ptoe2->setup_buf + IMPERFECT_PA_OFF;
		//fill our own mac addr in setup frame buffer(offset from 156 to 167) 
		for (i = 0; i < ETH_ALEN; i ++) 
		{
			pa[i&0x01] = dev->dev_addr[i]; //host mac addr.
			if(i&0x01) 
				pa += 4;
		}
		//offset is 168 now.

		pa[(TOE2_HASH_TABLE_LEN >> 3) - 3] = 0x80; //bc.FM-20091123.

		buf_base = ptoe2->setup_buf;
		for (i = 0, mc_list = dev->mc_list; mc_list && i < dev->mc_count; i++) 
		{
			addr = &mc_list->da_addr[0];
			crc = ether_crc(ETH_ALEN, addr);

			hashcode = (UINT16)crc & TOE2_HASH_BITS;

			byte = hashcode >> 3;				//bit[3-8] -> byte offset
			bit = 1 << (hashcode & 0x07);		//bit[0-2] -> bit offset

			byte <<= 1;						//maybe hard to get it!^_^
			if (byte & 0x02) 
				byte -= 1;

			buf_base[byte] |= bit;
			mc_list = mc_list->next;			
		}	
	} 
	else if (toe2_rx_filter == TOE2_RX_FILTER_PERFECT)
	{
		pa = ptoe2->setup_buf + 0;
		for (i=0; i<ETH_ALEN; i++) 
		{ 
			pa[i&0x01] = dev->dev_addr[i]; //host mac addr.
			if (i&0x01) 
				pa += 4;
		}

		for (i=0; i<ETH_ALEN; i++) 
		{
			pa[i&0x01] = 0xFF; 			//bc addr.
			if (i & 0x01) pa += 4;
		}
		//offset is 24 now. know why 14 now?

		for (i = 0, mc_list = dev->mc_list; mc_list && i < dev->mc_count; i++) 
		{
			for (j = 0; j < ETH_ALEN; j ++) 
			{ 
				pa[j&0x01] = mc_list->da_addr[j];				
				if (j&0x01) 
					pa += 4;
			}
			mc_list = mc_list->next;	
		}
	}
	else
	{
		TOE2_WARNING("%s()=>toe2_rx_filter(%ld) not supported yet.", __FUNCTION__, toe2_rx_filter);
		rv = -1;
	}
	return rv;
}

int mac_set_mc_filter(struct net_device *dev)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;
	u8 cmd_u8;
	UINT32 cmd_u32;

	cmd_u8 = TOE2_R8(SCR);
	TOE2_W8(SCR, cmd_u8 & ~(SCRRxEn|SCRTxEn));

	cmd_u32 = TOE2_R32(NetworkOM);
	cmd_u32 &= ~PassMask;
	TOE2_W32(NetworkOM, cmd_u32);

	return mac_build_setup(dev);
}

void mac_load_setup(ptoe2_private ptoe2)
{
	/*
	   UINT32 ba = ptoe2->io_base;
	   */
}
#endif

static u16 mac_rx_update_wptr(ptoe2_private ptoe2)
{
	struct net_device *dev = ptoe2->dev;
	UINT32 ba = ptoe2->io_base;
	u16 rx_bptr, rx_rptr, rx_wptr;
	u16 updata = 0;
	prx_desc rx_desc;
	struct sk_buff *new_skb;
	int i;

	rx_wptr = ptoe2->rx_wptr;
	rx_bptr = ptoe2->rx_bptr;
	rx_rptr = TOE2_R16(RxRingDesRPtr);

	//TOE2_TRACE("rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.", rx_wptr, rx_bptr, rx_rptr);

	if (rx_wptr > rx_rptr)
	{
		if ((rx_bptr > rx_rptr) && (rx_bptr <= rx_wptr))
			goto rx_lost;
		else
		{
			if (rx_bptr > rx_wptr)
				updata = rx_bptr - rx_wptr -1;
			else
				updata = TOE2_RX_DESC_NUM + rx_bptr - rx_wptr -1;
		}
	}	
	else if (rx_wptr < rx_rptr)
	{
		if ((rx_bptr > rx_rptr) ||(rx_bptr <= rx_wptr))
			goto rx_lost;	
		else
			updata = rx_bptr - rx_wptr -1;
	}
	else
	{
		if (rx_bptr > rx_wptr)
			updata = rx_bptr - rx_wptr -1;
		else if (rx_bptr < rx_wptr)
			updata = TOE2_RX_DESC_NUM + rx_bptr - rx_wptr -1;
		else
			goto rx_lost;	
	}

	if (updata > 0)
	{
		i = rx_wptr;
		while (updata > 0)
		{
			if (ptoe2->rx_skb[i])
			{
				new_skb = ptoe2->rx_skb[i];
			}
			else
			{
				new_skb = netdev_alloc_skb(dev, TOE2_BUF_SZ);
			}
			if (!new_skb)
			{
				dev->stats.rx_dropped++;
				break;
			}
			//skb_reserve(new_skb, NET_IP_ALIGN);

			ptoe2->rx_skb[i] = new_skb;

			rx_desc = &ptoe2->rx_desc[i];
			dma_unmap_single((struct device *)NULL, rx_desc->pkt_buf_dma, TOE2_BUF_SZ, DMA_FROM_DEVICE);
			rx_desc->pkt_buf_dma = dma_map_single((struct device *)NULL, new_skb->data, TOE2_BUF_SZ, DMA_FROM_DEVICE);

			if (i == TOE2_RX_DESC_NUM - 1)
			{
				rx_desc->EOR = 1;
				i = 0;
			}
			else
			{
				rx_desc->EOR = 0;
				i ++;
			}		
			updata --;
		}

		ptoe2->rx_wptr = i;
		TOE2_W16(RxRingDesWPtr, ptoe2->rx_wptr);
	}

	//TOE2_TRACE("rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.", rx_wptr, rx_bptr, rx_rptr);
	return rx_rptr;

rx_lost:
	TOE2_WARNING("%()=>rx_bptr got lost.", __FUNCTION__);
	TOE2_WARNING("rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.\n", rx_wptr, rx_bptr, rx_rptr);
	//asm("sdbbp");
	return rx_rptr;
}

static bool mac_rx_hdr_chk(struct net_device *dev, ppacket_head pHead)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	int fatal_err = 0;
	UINT32 temp_u32 = 0;

	if (pHead->ES)
	{
		if (pHead->WatchdogTimeout)
		{
			fatal_err ++;
			ptoe2->mac_stats.rx_wd_timeout_errors++;
		}
		if (pHead->PhysicalLayerError)
		{
			fatal_err ++;
			ptoe2->mac_stats.rx_phy_layer_errors++;
		}
		if (pHead->LateCollision)
		{
			fatal_err ++;
			ptoe2->mac_stats.rx_late_col_seen++;
		}
		if (pHead->Long)
		{
			fatal_err ++;
			ptoe2->mac_stats.rx_long_errors++;
		}
		if (pHead->Runt) 
		{
			fatal_err ++;
			ptoe2->mac_stats.rx_runt_errors++;	
		}
		if (pHead->Dribble)
		{
			fatal_err ++;
			ptoe2->mac_stats.rx_dribble_errors++;	
		}

		if ((pHead->FifoOverflow) && (0 == fatal_err))
		{
			return true;
		}
		else
		{
			dev->stats.rx_errors++;
			return false;
		}
	}	
	else
	{
		if (pHead->PacketLength > 1536)
		{
			return false;
		}

#if (TOE2_VLAN_TAG_USED)
		if ((pHead->VLAN) && (ptoe2->vlan_tag_remove))
		{
			temp_u32 = 60;
		}
		else
#else			
		{
			temp_u32 = 64;
		}
#endif
		if (pHead->PacketLength < temp_u32)
		{
			return false;
		}

#if (TOE2_VLAN_TAG_USED)
		//VLAN Patch 3/3 For receiving packet length 1517 & 1518, we need to add a patch here.
		if ((pHead->VLAN) && (!ptoe2->vlan_tag_remove))
		{
			temp_u32 = 1522;
		}
		else
#else			
		{
			temp_u32 = 1518;
		}
#endif
		if (pHead->PacketLength > temp_u32)
		{
			dev->stats.rx_length_errors++;
			ptoe2->mac_stats.rx_long_errors++;
			return false;
		}

		dev->stats.rx_packets++;
		dev->stats.rx_bytes += pHead->PacketLength;

		if (pHead->BF)
			ptoe2->mac_stats.rx_bc++;
		if (pHead->PF)
			ptoe2->mac_stats.rx_uc++;
		if (pHead->MF)
			ptoe2->mac_stats.rx_mc++;
		if (pHead->PPPoE)
			ptoe2->mac_stats.rx_pppoe++;
		if (pHead->VLAN)
			ptoe2->mac_stats.rx_vlan++;
		if (pHead->IPFrame)
			ptoe2->mac_stats.rx_ip++;
		if (pHead->IPFrag)
			ptoe2->mac_stats.rx_frag++;

		return true;
	}	
}

//analyze & recode rx status while head is okay.i
static bool mac_rx_chs_ok(ptoe2_private ptoe2, ppacket_head pHead)
{	
	if ((toe2_rx_csum == true) && (pHead->IPFrame))
	{
		if (pHead->IPFrag)
			goto Done;

		if (!pHead->IPChksum)
		{
			ptoe2->mac_stats.rx_ip_chksum_errors++;
			TOE2_WARNING("mac_rx_chksum_ok()=>ip checksum err");
			goto Done;
		}

		if (!pHead->TCPChksum)
		{
			ptoe2->mac_stats.rx_tcp_chksum_errors++;
			TOE2_WARNING("mac_rx_chksum_ok()=>tcp checksum err");
			goto Done;
		}

		return true;
	}

Done:
	return false;
}

static int mac_rx_pkts(ptoe2_private ptoe2, int budget)
{
	//UINT32 ba = ptoe2->io_base;
	prx_desc rx_desc;
	u16 rx_bptr, rx_rptr;
	ppacket_head	pHead;
	u16 pkt_sz, pkts, rx, i;
	struct sk_buff *skb;

	if (!toe2_device_open)
		return 0;

	rx_rptr = mac_rx_update_wptr(ptoe2);
	//rx_rptr = TOE2_R16(RxRingDesRPtr);
	rx_bptr = ptoe2->rx_bptr;

	if (rx_rptr >= rx_bptr)
		pkts = rx_rptr - rx_bptr;
	else
		pkts = TOE2_RX_DESC_NUM + rx_rptr - rx_bptr;

	if ((pkts > 0) && (rx_rptr == ptoe2->rx_wptr))
		pkts -= 1;

	if (toe2_rx_mode == TOE2_RX_MODE_NAPI)
	{
		if ((budget != 0) && (pkts > budget)) 
			pkts = budget;
	}

	i = rx_bptr;
	rx = 0;
	while (pkts > 0)
	{
		rx_desc = &ptoe2->rx_desc[i];
		pHead = &(rx_desc->pkt_hdr);

		if (mac_rx_hdr_chk(ptoe2->dev, pHead))
		{			
			pkt_sz = pHead ->PacketLength - RING_CRC_SZ;
			skb = ptoe2->rx_skb[i];
			skb_put(skb, pkt_sz);

#ifdef SW_CRCCHK
			u32 crc_sw, crc_hw;

			crc_sw = *(u32 *)(skb->data + skb->len);
			crc_sw = ~crc_sw;
			crc_hw = ether_crc_le(skb->len, skb->data);
			if (crc_sw != crc_hw)
			{
				//toe2_print(skb->data, skb->len + RING_CRC_SZ);
				TOE2_TRACE("CRC ERR:crc_sw = 0x%08x, crc_hw = 0x%08x.", crc_sw, crc_hw);
				//TOE2_W8(0x5b, (TOE2_R8(0x5b) |0x80)); 
				//asm("sdbbp");
			}	
			//printk("crc_sw = 0x%08x, crc_hw = 0x%08x\n", crc_sw, crc_hw);
#endif

			//toe2_print(skb->data, skb->len);
			if (mac_rx_chs_ok(ptoe2, pHead))
			{
				skb->ip_summed = CHECKSUM_UNNECESSARY;
			}
			else
			{
				skb->ip_summed = CHECKSUM_NONE;
				if (pHead->IPFrame && !pHead->IPFrag && !pHead->TCPChksum)
				{
					//toe2_print(skb->data, skb->len + RING_CRC_SZ);
				}
			}

			skb->protocol = eth_type_trans(skb, ptoe2->dev);
			skb->dev = ptoe2->dev;

#ifdef WORK_QUEUE_TEST
			TOE2_TRACE("before");
			struct iphdr *ipHeader = ip_hdr(skb); 
			TOE2_TRACE("after");
			if (ipHeader->protocol == IPPROTO_ICMP)
			{
				TOE2_TRACE("this is a ICMP!");
				__skb_queue_tail(&my_wq.special_pkt_queue, skb);
				schedule_work(&my_wq.special_pkt_work);
			}
#endif

			if (toe2_rx_mode == TOE2_RX_MODE_NAPI) 
			{
#if (TOE2_VLAN_TAG_USED)
				if (ptoe2->vlgrp && pHead->VLAN) 
					vlan_hwaccel_receive_skb(skb, ptoe2->vlgrp, swab16(rx_desc->vlan_tag));
				else
#endif
					netif_receive_skb(skb);
			}
			else 
			{
#if (TOE2_VLAN_TAG_USED)
				if (ptoe2->vlgrp && pHead->VLAN) 
					vlan_hwaccel_rx(skb, ptoe2->vlgrp, swab16(rx_desc->vlan_tag));
				else
#endif
					netif_rx(skb);
			}

		}
		else
		{
			TOE2_WARNING("mac_rx_pkts(head error): Head(%08lX).", *(UINT32 *)pHead);
			//asm("sdbbp");
			dev_kfree_skb_any(ptoe2->rx_skb[i]);
			TOE2_INFO("[NOTE] rx head err to free rx skb[%d]\n", i);
		}

		//free rx skb.
		ptoe2->rx_skb[i] = NULL;

		if (i == TOE2_RX_DESC_NUM - 1)
			i = 0;
		else
			i ++;

		pkts --;
		rx ++;
	}

	ptoe2->rx_bptr = i; 
#ifdef TOE2_NAPI_DEBUG	
	ptoe2->polling_times++;
	if (rx<10)
	{
		ptoe2->rx_small_times++;
	}
	else
	{
		ptoe2->rx_big_times++;
		//TOE2_TRACE("No.%d, rx=%d, budget=%d", ptoe2->polling_times, rx, budget);
	}
	//TOE2_TRACE("No.%d, rx=%d", ptoe2->polling_times, rx);
#endif

#ifdef DYNAMIC_CHANGING_IMK
	if (ptoe2->cur_isr&ISRTimer)
	{
		if (ptoe2->cur_dny_imk == HIGH_INGRESS_IMK)
		{
			if ((ptoe2->num_zero_timer_int_HZ>TOO_MANY_USELESS_TIMER_INTERRUPT) && (timer_period > 0))
			{		
				dny_imk2low(ptoe2);
				DNY_INT_TRACE("TOE2: dny_imk2low!");
				//printk("TOE2: dny_imk2low!\n");
				//register of IMR will be renewed after toe2_enable_irq.				
			}
			if (rx==0)
				ptoe2->num_zero_timer_int_HZ++;
			else
				ptoe2->num_zero_timer_int_HZ = 0;			
		}
	}
#endif

	return rx;
}

static int mac_rx_poll(struct napi_struct *napi, int budget)
{
	ptoe2_private ptoe2 = container_of(napi, struct toe2_private, napi);
	int re;
	spin_lock(&ptoe2->lock);
	re = mac_rx_pkts(ptoe2, budget);
	spin_unlock(&ptoe2->lock);

	if (re < budget)
	{
#ifdef KERNEL_VERSION_2_6_28 // 2.6.28
		netif_rx_complete(napi);
		toe2_enable_irq(ptoe2);
#else				// 2.6.32
		napi_complete(napi);
		toe2_enable_irq(ptoe2);
#endif
		//printk("mac_rx_poll, re < budget, re = %d, budget = %d\n", re, budget);

#ifdef TOE2_NAPI_DEBUG
		g_ptoe2->during_pkt_rx = false;
#endif
	}
	return re;
}

#ifdef TX_ASYNC_XMIT
static void free_tx_skb(ptoe2_private ptoe2, bool ulock)
{
	u16 first, desc_num;
	ptx_desc desc;
	UINT32 ba = ptoe2->io_base;
	u16 tx_rptr;
	unsigned long flags=0;

	if (ulock)
		spin_lock_irqsave(&ptoe2->lock, flags);

	tx_rptr = TOE2_R16(TxRingDesRPtr);
	while (tx_skb_wr != tx_skb_rd)
	{
		first = g_tx_skb[tx_skb_rd].first;
		desc_num = g_tx_skb[tx_skb_rd].cnt;

		if ((first<tx_rptr && tx_rptr-first>=desc_num) ||
			(first>tx_rptr && ptoe2->tx_wptr>=tx_rptr && first>ptoe2->tx_wptr &&
			 TOE2_TX_DESC_NUM-(first-tx_rptr)>=desc_num))
		{
			; //g_tx_skb[tx_skb_rd] output had done!
		}
		else
		{
			if (ulock)
				spin_unlock_irqrestore(&ptoe2->lock, flags);
			return;
		}

		desc = &ptoe2->tx_desc[first];
		mac_tx_sts(ptoe2, desc);

		if (g_tx_skb[tx_skb_rd].skb)
		{
			dev_kfree_skb_any(g_tx_skb[tx_skb_rd].skb);
			g_tx_skb[tx_skb_rd].skb = NULL;
			g_tx_skb[tx_skb_rd].first = 0;
			g_tx_skb[tx_skb_rd].cnt = 0;

			do
			{
				if (desc->seg_len)
				{
					dma_unmap_single((struct device *)NULL, desc->pkt_buf_dma, desc->seg_len, DMA_TO_DEVICE);
					desc->seg_len = 0;
				}
				else
				{
					//asm("sdbbp");
				}

				if ((++first) >= TOE2_TX_DESC_NUM)
				{
					first = 0;
				}
				desc = &ptoe2->tx_desc[first];
				desc_num --;

			} while(desc_num > 0);		
		}
		else
		{
			//asm("sdbbp");
		}

		if (tx_skb_rd+1 == TOE2_TX_DESC_NUM)
			tx_skb_rd = 0;
		else
			tx_skb_rd++;
	}	
	if (ulock)
		spin_unlock_irqrestore(&ptoe2->lock, flags);
}
#endif

static void mac_rx_tasklet(unsigned long para)
{
	UINT32 ba = g_ptoe2->io_base;
	u8 tmp_u8;

	para = para;

	if (unlikely(!netif_running(g_ptoe2->dev)))
	{
		TOE2_TRACE("mac_rx_tasklet?");    
		TOE2_W16(IMR, 0);
		return;
	}

	if (g_ptoe2->cur_isr & ISRTxUnderrun)
	{
		if (netif_msg_ifup(g_ptoe2))
			TOE2_TRACE("%s()=>Tx Fifo underrun.", __FUNCTION__);
	}

	if (g_ptoe2->cur_isr & (ISRRxComplete|ISRRxFifoOverflow | ISRRxBufOverflow | ISRRxBufDiscard | ISRTimer))
	{
		spin_lock(&g_ptoe2->lock);
		do
		{
			tmp_u8 = TOE2_R8(SCR);
			if ((tmp_u8 & (u8)SCRBufEmpty))
			{
				g_ptoe2->mac_stats.rx_buf_empty ++;
				break;
			}

			if (0 == mac_rx_pkts(g_ptoe2, 0))
				break;

		} while(true);
		spin_unlock(&g_ptoe2->lock);
	}

	if (g_ptoe2->cur_isr & (ISRLinkStatus | ISRTimer | ISRWatchdog))
	{
		mac_weird_int(g_ptoe2);
	}

#ifdef TX_ASYNC_XMIT
	if (g_ptoe2->cur_isr & (ISRTxComplete|ISRTimer))
	{
		free_tx_skb(g_ptoe2, true);
	}
#endif
}

static void mac_weird_tasklet(unsigned long para)
{
	//UINT32 ba = g_ptoe2->io_base;
	//u8 tmp_u8;

	//toe2_disable_irq(g_ptoe2);
	/*	
		para = para;

		if (unlikely(!netif_running(g_ptoe2->dev)))
		{
		printk("mac_weird_tasklet?\n");    
		TOE2_W16(IMR, 0);
		return ;
		}
		*/
	if (g_ptoe2->cur_isr & ISRTxUnderrun)
	{
		if (netif_msg_ifup(g_ptoe2))
			TOE2_TRACE("%s()=>Tx Fifo underrun.", __FUNCTION__);
	}

	if (g_ptoe2->cur_isr & (ISRLinkStatus | ISRTimer | ISRWatchdog))
	{
		mac_weird_int(g_ptoe2);
	}

	//toe2_enable_irq(g_ptoe2);
}

static void mac_tx_sts(ptoe2_private ptoe2, ptx_desc desc_sw)
{
	if ((desc_sw->tx_sts.sw.FS) && !(desc_sw->tx_sts.sw.OWN))
	{
		if (!(desc_sw->tx_sts.sw.ES))
			ptoe2->dev->stats.tx_packets++;
		else
		{ 
			ptoe2->dev->stats.tx_errors++;
			if ((desc_sw->tx_sts.sw.LossOfCarrier) || (desc_sw->tx_sts.sw.NoCarrier))
			{
				ptoe2->dev->stats.tx_carrier_errors++;
				//printk("Carrier Err\n");
			}
			if (desc_sw->tx_sts.sw.LateCol) 
				ptoe2->dev->stats.tx_window_errors++;
			if (desc_sw->tx_sts.sw.FifoUnderrun) 
				ptoe2->dev->stats.tx_fifo_errors++;
			if (desc_sw->tx_sts.sw.HF) 
				ptoe2->dev->stats.tx_heartbeat_errors++;
		}

		if (desc_sw->tx_sts.sw.ExCol) 
			ptoe2->mac_stats.tx_col_errors++;
		else
			ptoe2->mac_stats.tx_col_cnts[desc_sw->tx_sts.sw.ColCnt]++;
	}
}

static void mac_tx_cfg(ptoe2_private ptoe2, u16 off, struct sk_buff *skb, void *seg_addr, u16 seg_len)
{
	ptx_desc desc = &ptoe2->tx_desc[off];
	/*
	   mac_tx_sts(ptoe2, desc);

	   if(desc->seg_len > 0)
	   dma_unmap_single((struct device *)NULL, desc->pkt_buf_dma, desc->seg_len, DMA_TO_DEVICE);

	   if(ptoe2->tx_skb[off])
	   {
	   dev_kfree_skb_any(ptoe2->tx_skb[off]);	
	   ptoe2->tx_skb[off] = NULL;
	   }
	   */
	memset(desc, 0, TOE2_DESC_SZ);
	desc->pkt_buf_dma = dma_map_single((struct device *)NULL, seg_addr, seg_len, DMA_TO_DEVICE);

	if (off == TOE2_TX_DESC_NUM-1)
		desc->tx_sts.hw.EOR = 1;

	desc->seg_len = seg_len;

	if (skb)
	{
#ifndef TX_ASYNC_XMIT
		ptoe2->tx_skb[off] = skb;
#endif

		if (skb->ip_summed == CHECKSUM_PARTIAL) 
		{
			if (toe2_tx_csum)
			{
				const struct iphdr *ip = ip_hdr(skb);
				desc->tx_sts.hw.IpStartOff = (u8)((u8 *)ip - skb->data);

				desc->tx_sts.hw.CoeEn = 1;
				if (desc->tx_sts.hw.IpStartOff > 0)
				{
					if (ip->protocol == IPPROTO_TCP)
						desc->tx_sts.hw.TcpPkt = 1;
					else if (ip->protocol == IPPROTO_UDP)
						desc->tx_sts.hw.UdpPkt = 1;
					else
					{
						TOE2_WARNING("%s()=>Pkt type err while doing chs.", __FUNCTION__);
						//asm("sdbbp");
					}
				}
				else
				{
					TOE2_WARNING("%s()=>(IpStartOff <= 0)?!.", __FUNCTION__);
					//asm("sdbbp");
				}
			}
			else
			{
				TOE2_WARNING("%s()=>Tx cks not enabled.", __FUNCTION__);
				//asm("sdbbp");
			}
		}

#if (TOE2_VLAN_TAG_USED)
		if (ptoe2->vlgrp && vlan_tx_tag_present(skb))
		{
			desc->vlan_tag = swab16(vlan_tx_tag_get(skb));
			desc->tx_sts.hw.VlanEn = 1;
		}
		else
#endif
			desc->tx_sts.hw.VlanEn = 0;
	}
}

static long phy_detect(ptoe2_private ptoe2)
{
	unsigned char	   phy_id;
	unsigned short	   mode_control_reg;
	unsigned short	   status_reg;
	unsigned short 	   phy_mod;
	unsigned short 	   phy_oui;
	struct net_device *dev;

	//TOE2_TRACE("phy_detect: start. \n");

	if (!ptoe2)
	{
		TOE2_INFO("%s: Invalid Parameter!", __FUNCTION__);
		return -1;
	}

	mode_control_reg = 0;
	status_reg = 0;
	phy_id = 0;
	dev = ptoe2->dev;

	mode_control_reg = (unsigned short)mac_mdio_read(dev, toe2_phy_addr, PhyBasicModeCtrl);

	status_reg = (unsigned short)mac_mdio_read(dev, toe2_phy_addr, PhyBasicModeStatus);
	if ((mode_control_reg != 0xffff) && (status_reg != 0xffff))
	{
		//TOE2_TRACE("%s: phy address is right. \n", __FUNCTION__);
		phy_mod = (unsigned short)mac_mdio_read(dev, toe2_phy_addr, 0x2);
		phy_oui = (unsigned short)mac_mdio_read(dev, toe2_phy_addr, 0x3);
		TOE2_INFO("phy_mod=0x%08X, phy_oui=0x%08X.",phy_mod, phy_oui);
		if (phy_mod==0x004D && phy_oui==0xD023 && toe2_physet_gpio_vld)
		{
			TOE2_INFO("phy detected, use R8032 phy.");
			toe2_use_R8032_patch = true;
		}
		return 1;
	}

	//TOE2_TRACE("%s: phy address is wrong, start auto detecting.\n", __FUNCTION__);

	for (phy_id = 0; phy_id < 32; ++phy_id)
	{
		mode_control_reg = (unsigned short)mac_mdio_read(dev, phy_id, PhyBasicModeCtrl);
		status_reg = (unsigned short)mac_mdio_read(dev, phy_id, PhyBasicModeStatus);

		if ((mode_control_reg != 0xffff) && (status_reg != 0xffff))
		{
			TOE2_INFO("%s: phy address detected: %d.", __FUNCTION__, phy_id);
			toe2_phy_addr = phy_id;
			ptoe2->mii_if.phy_id = phy_id;
			phy_mod = (unsigned short)mac_mdio_read(dev, toe2_phy_addr, 0x2);
			phy_oui = (unsigned short)mac_mdio_read(dev, toe2_phy_addr, 0x3);
			TOE2_INFO("phy_mod=0x%08X, phy_oui=0x%08X.",phy_mod, phy_oui);
			if (phy_mod==0x004D && phy_oui==0xD023 && toe2_physet_gpio_vld)
			{
				TOE2_INFO("phy detected, use R8032 phy.");
				toe2_use_R8032_patch = true;
			}
			return 1;
		}
	}

	TOE2_INFO("%s: phy address not detected. :(", __FUNCTION__);
	return -1;
}

extern int ali_otp_read(unsigned long offset, unsigned char *buf, int len);
static void phy_gain_fine_tune(struct net_device *dev)
{
    u32 result = 0;
	u16 tmp_u16;
	int phy_reg;
	
	TOE2_INFO("[XX] ready to gain fine tune from otp:");
    ali_otp_read(0xDF*4, (unsigned char *)(&result), 4);
	TOE2_INFO("     fine tune value: 0x%08x.", result);

    //10BASE  [25:22]->[15:12]
	//100BASE [29:26]->[11:8]
	phy_reg = 0x1C;
	tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
	tmp_u16 &= 0x00FF;
	tmp_u16 |= (((result>>22)&0x0F)<<12);
	tmp_u16 |= (((result>>26)&0x0F)<<8);
	mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);
}

static void phy_set(ptoe2_private ptoe2)
{
	struct net_device *dev = ptoe2->dev;
	u16 tmp_u16;
	int phy_reg;

    if (C3821 == g_chip_id)
	{
	    phy_gain_fine_tune(dev);
		
		//Step1: set FSM to disable state, this process will shut down all related PHY's DSP part.
		phy_reg = 0x10;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 |= (1<<0);  //Reset main FSM to IDLE State
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);

		//Step2: configure the autoneg ability, that we can't support next page exchange.
#ifndef PHY_IOL_TEST
		//For Mass product, do not modify this register
		phy_reg = 0x04;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 &= ~(1<<15);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);
#else
		//For IOL test, the following register should be added
		phy_reg = 0x04;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 |= (1<<15);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);

		phy_reg = 0x11;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 &= ~(1<<10);
		tmp_u16 &= ~(1<<11);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);

		//enlarge the TX template swing
		mac_mdio_write(dev, toe2_phy_addr, 0x1D, 0x06);
		phy_reg = 0x1E;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 |= (1<<15);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);

		mac_mdio_write(dev, toe2_phy_addr, 0x1D, 0x0A);
		phy_reg = 0x1E;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 |= (1<<12);
		tmp_u16 |= (1<<13);
		tmp_u16 |= (1<<14);
		tmp_u16 |= (1<<15);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);

	    mac_mdio_write(dev, toe2_phy_addr, 0x1D, 0x02);
		phy_reg = 0x1F;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 |= (1<<4);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);
#endif

		//Step3: configure the AGC threshold.
		mac_mdio_write(dev, toe2_phy_addr, 0x1D, 0x07);  //select the AGC debug register
		phy_reg = 0x1F;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		//tmp_u16 &= ~(1<<1);
		//tmp_u16 |= (1<<2);
		tmp_u16 &= ~(1<<7);
		tmp_u16 &= ~(1<<8);
		tmp_u16 |= (1<<9);
		tmp_u16 &= ~(1<<10);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);
        phy_reg = 0x1E;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 &= ~(1<<8);
		tmp_u16 &= ~(1<<9);
		tmp_u16 |= (1<<10);
		tmp_u16 |= (1<<11);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);
		
		//Step4: configure the DAC clock phase.
		phy_reg = 0x1C;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 &= ~(1<<4);
		tmp_u16 &= ~(1<<5);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);

        //Step5: enlarge the AGC kill det windows length to 8us.
        mac_mdio_write(dev, toe2_phy_addr, 0x1D, 0x0A);
		phy_reg = 0x1F;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 |= (1<<0);
		tmp_u16 |= (1<<1);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);

        /*remove
		//Step6: enlarge the lower threshold of slicer.
        mac_mdio_write(dev, toe2_phy_addr, 0x1D, 0x03);
		phy_reg = 0x1F;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 |= (1<<8);
		tmp_u16 |= (1<<9);
		tmp_u16 &= ~(1<<10);
		tmp_u16 |= (1<<11);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);
		*/

        //Step7: decrease the BLW gain when in tracking killer pattern.
		mac_mdio_write(dev, toe2_phy_addr, 0x1D, 0x0B);
		phy_reg = 0x1F;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 |= (1<<5);
		tmp_u16 &= ~(1<<6);
		tmp_u16 |= (1<<7);
		tmp_u16 &= ~(1<<8);
		tmp_u16 &= ~(1<<9);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);

        //Step8: enlarge the TX template swing.
		mac_mdio_write(dev, toe2_phy_addr, 0x1C, 0xC500);

		//Step9: set FSM to start.
		mac_mdio_write(dev, toe2_phy_addr, 0x1D, 0);
		phy_reg = 0x10;
		tmp_u16 = (UINT16)mac_mdio_read(dev, toe2_phy_addr, phy_reg);
		tmp_u16 &= ~(1<<0);
		mac_mdio_write(dev, toe2_phy_addr, phy_reg, tmp_u16);
	}
	else
	{
		//reset phy registers in a default state.
		mac_mdio_write(dev, toe2_phy_addr, PhyBasicModeCtrl, BMCRReset);

		do
		{	
			tmp_u16 = (u16)mac_mdio_read(dev, toe2_phy_addr, PhyBasicModeCtrl);
		} while(tmp_u16 & BMCRReset);
		TOE2_INFO("phy_set()=>phy reset complete.");

		//enable Rx/Tx pause frame.
		tmp_u16 = (u16)mac_mdio_read(dev, toe2_phy_addr, PhyNWayAdvert);
		//tmp_u16 |= ANARPause | ANARASMDIR |ANAR_MEDIA;
		tmp_u16 |= ANAR_MEDIA;
		//tmp_u16 = ANAR10FD; //force to 10-full.
		mac_mdio_write(dev, toe2_phy_addr, PhyNWayAdvert, (int)tmp_u16);

		//auto-negotiation enable & restart auto-negotiation.
		tmp_u16 = (u16)mac_mdio_read(dev, toe2_phy_addr, PhyBasicModeCtrl);
		tmp_u16 |= BMCRANEnable | BMCRRestartAN;	
		mac_mdio_write(dev, toe2_phy_addr, PhyBasicModeCtrl, (int)tmp_u16);

		ptoe2->phy_reset = true;
	}
}

static void phy_link_chk(ptoe2_private ptoe2)
{
	struct net_device *dev = ptoe2->dev; 
	u16 media, advertisement, link;

    u16 expansion = 0;
	
	unsigned long pause;
	unsigned long asm_dir;
	unsigned long partner_pause;
	unsigned long partner_asm_dir;

	ptoe2->duplex_mode = false;
	ptoe2->link_speed = (UINT32)10; //M bps in unit. that is 10M bps.
	ptoe2->pause_frame_rx = false;
	ptoe2->pause_frame_tx = false;	

    media = (u16)mac_mdio_read(dev, toe2_phy_addr, PhyBasicModeCtrl);
    advertisement = (u16)mac_mdio_read(dev, toe2_phy_addr, PhyNWayAdvert);
    /* reg 5 link partner ability */
    link = (u16)mac_mdio_read(dev, toe2_phy_addr, PhyNWayLPAR);
		
#ifdef OLDER_LINK_CHK
	if (media&BMCRANEnable)
	{
		media = (u16)mac_mdio_read(dev, toe2_phy_addr, 0x10);
		media = (media>>8)&0xf;
		if (media==0xE)
		{
			ptoe2->link_speed = (UINT32)100;
			ptoe2->duplex_mode = true;
			printk("%s()=>100 Base speed, Full Duplex Mode.\n", __FUNCTION__);
		}
		else if (media==0xD)
		{
			ptoe2->link_speed = (UINT32)100;
			ptoe2->duplex_mode = false;
			printk("%s()=>100 Base speed, Half Duplex Mode.\n", __FUNCTION__);
		}
		else if (media==0xC)
		{
			ptoe2->link_speed = (UINT32)10;
			ptoe2->duplex_mode = true;
			printk("%s()=>10 Base speed, Full Duplex Mode.\n", __FUNCTION__);
		}
		else if (media==0xB)
		{
			ptoe2->link_speed = (UINT32)10;
			ptoe2->duplex_mode = false;
			printk("%s()=>10 Base speed, Half Duplex Mode.\n", __FUNCTION__);
		}
		else
		{
			printk("%s() use default 10 Base speed, Half Duplex Mode.\n", __FUNCTION__);
		}
	}
	else
	{
		if (media&BMCRSpeedSet)
		{
			ptoe2->link_speed = (UINT32)100;
			printk("%s()=>100 Base speed.\n", __FUNCTION__);
		}
		else
		{
			printk("%s()=>10 Base speed.\n", __FUNCTION__);
		}

		if (media&BMCRDuplexMode)
		{
			ptoe2->duplex_mode = true;
			printk("%s()=>full duplex mode.\n", __FUNCTION__);
		}
		else
		{
			printk("%s()=>half duplex mode.\n", __FUNCTION__);
		}
	}
#else
	if (media & BMCRANEnable)
	{
	    //used for LinkStatusChg check. 
        ptoe2->link_partner = link;

        expansion = (u16)mac_mdio_read(dev, toe2_phy_addr, PhyNWayExpansion);
	
        if (expansion & 0x1)
		{
		    /* partner support auto nego */
            if ((advertisement & ANARTXFD) && (link & ANARTXFD))
			{
				ptoe2->link_speed = (UINT32)100;
			    ptoe2->duplex_mode = true;
				TOE2_LINK("NEGO:100 Base-TX full duplex.");
            }
			else if ((advertisement & ANARTX) && (link & ANARTX))
			{
                ptoe2->link_speed = (UINT32)100;
			    ptoe2->duplex_mode = false;
				TOE2_LINK("NEGO:100 Base-TX half duplex.");
            }
			else if ((advertisement & ANAR10FD) && (link & ANAR10FD))
			{
                ptoe2->link_speed = (UINT32)10;
			    ptoe2->duplex_mode = true;
                TOE2_LINK("NEGO:10 Base-TX full duplex.");
            }
			else
			{
			    ptoe2->link_speed = (UINT32)10;
			    ptoe2->duplex_mode = false;
                TOE2_LINK("NEGO:10 Base-TX half duplex.");
            }
        }
		else
		{
			/* partner doesn't support auto nego */
            if (link & ANARTXFD)
			{
                ptoe2->link_speed = (UINT32)100;
			    ptoe2->duplex_mode = true;
                TOE2_LINK("SELF:100 Base-TX full duplex.");
            }
			else if (link & ANARTX)
			{
                ptoe2->link_speed = (UINT32)100;
			    ptoe2->duplex_mode = false;
                TOE2_LINK("SELF:100 Base-TX half duplex.");
            }
            else if (link & ANAR10FD)
			{
                ptoe2->link_speed = (UINT32)10;
			    ptoe2->duplex_mode = true;
                TOE2_LINK("SELF:10 Base-TX full duplex.");
            }
			else if (link & ANAR10)
			{
                ptoe2->link_speed = (UINT32)10;
			    ptoe2->duplex_mode = false;
                TOE2_LINK("SELF:10 Base-TX half duplex.");
            }
			else
			{
			    ptoe2->link_speed = (UINT32)10;
			    ptoe2->duplex_mode = false;
                TOE2_LINK("link check doesn't get corrent speed set 10M half.");
            }
		}
	}
	else
	{
	    if (media & BMCRDuplexMode)
		{
            ptoe2->duplex_mode = true;
        }
		else
		{
            ptoe2->duplex_mode = false;
        }
        if (!(media & BMCRSpeedSet6) && (media & BMCRSpeedSet13))
		{
            ptoe2->link_speed = (UINT32)100;
            TOE2_LINK("DEF:100 Base-TX %s duplex.", ((ptoe2->duplex_mode) ? "full" : "half"));
        }
		else if (!(media & BMCRSpeedSet13) && !(media & BMCRSpeedSet6))
		{
            ptoe2->link_speed = (UINT32)10;
            TOE2_LINK("DEF:10 Base-TX %s duplex.", ((ptoe2->duplex_mode) ? "full" : "half"));
        }
		else
		{
            ptoe2->link_speed = (UINT32)10;
            TOE2_LINK("%s can't get valid speed, set to 10M.", __FUNCTION__);
        }
	}
	/*else
	{
        printk("error, shouldn't call %s.\n", __FUNCTION__);
        return;
    }*/
#endif

	if (!(ptoe2->duplex_mode))
	{
		TOE2_LINK("%s()=>half duplex mode, no flow control.", __FUNCTION__);
		ptoe2->pause_frame_rx = false;
        ptoe2->pause_frame_tx = false;
		return;
	}

#if 0
	media = (u16)mac_mdio_read(dev, toe2_phy_addr, PhyBasicModeCtrl);
	media &= (~BMCRLoopback);

	//the order is ANAR_TXFD->ANAR_T4->ANAR_TX->ANAR_10FD->ANAR_10. chg on 2008.03.13.
	if ((advertisement & ANARTXFD) && (link & ANARTXFD))
	{
		media |= BMCRSpeedSet;				//100Mbps.
		media |= BMCRDuplexMode;				//full-duplex.
		ptoe2->duplex_mode = true;
		ptoe2->link_speed = (UINT32)100;
		TOE2_TRACE("%s()=>100 Base-TX full duplex.", __FUNCTION__);
	} 
	else if ((advertisement & ANARTX) && (link & ANARTX))
	{
		media |= BMCRSpeedSet;				//100Mbps.
		media &= ~BMCRDuplexMode;			//normal op.
		ptoe2->link_speed = (UINT32)100;
		TOE2_TRACE("%s()=>100 Base-TX.", __FUNCTION__);
	} 
	else if ((advertisement & ANAR10FD) && (link & ANAR10FD))
	{
		media &= ~BMCRSpeedSet;				//10Mbps.
		media |= BMCRDuplexMode;				//full-duplex.
		ptoe2->duplex_mode = true;
		TOE2_TRACE("%s()=>10 Base-TX full duplex.", __FUNCTION__);
	} 
	else 
	{
		media &= ~BMCRSpeedSet;				//10Mbps.
		media &= ~BMCRDuplexMode;			//normal op.
		TOE2_TRACE("%s()=>10 Base-TX.", __FUNCTION__);
	}
#endif

#ifdef PHY_SMSC
	//Dis SQE Test for SMSC8700
	u16 tmp_u16 =  (u16)mac_mdio_read(dev, toe2_phy_addr, 27);
	mac_mdio_write(dev, toe2_phy_addr, 27, (int)tmp_u16 |(1<<11));
#endif
	/*
	   if ((advertisement & ANARPauseRx) && (link & ANARPauseTx))
	   {
	   ptoe2->pause_frame_rx = true;
	   TOE2_TRACE("%s()=>Rx flow control is suported by local node.", __FUNCTION__);
	   }
	   else 
	   {
	   ptoe2->pause_frame_rx = false;
	   TOE2_TRACE("%s()=>Rx flow control not suported by local node.", __FUNCTION__);
	   }

	   if ((advertisement & ANARPauseTx) && (link & ANARPauseRx))
	   {
	   ptoe2->pause_frame_tx = true;
	   TOE2_TRACE("%s()=>Tx flow control is suported by local node.", __FUNCTION__);
	   }
	   else 
	   {
	   ptoe2->pause_frame_tx = false;
	   TOE2_TRACE("%s()=>Tx flow control not suported by local node.", __FUNCTION__);
	   }
	   */

	pause = (advertisement & ANARPause) > 0 ? 1 : 0;
	asm_dir = (advertisement & ANARASMDIR) > 0 ? 1 : 0;
	partner_pause = (link & ANARPause) > 0 ? 1 : 0;
	partner_asm_dir = (link & ANARASMDIR) > 0 ? 1 : 0;

	TOE2_INFO("phy_link_chk(): Pause = %ld, ASM_DIR = %ld, PartnerPause = %ld, PartnerASM_DIR = %ld.", pause, asm_dir, partner_pause, partner_asm_dir);

	if ((pause == 0) && (asm_dir == 0))
	{
		ptoe2->pause_frame_rx = false;
		ptoe2->pause_frame_tx = false;		
	}
	else if ((pause == 0) && (partner_pause == 0))
	{
		ptoe2->pause_frame_rx = false;
		ptoe2->pause_frame_tx = false;		
	}
	else if ((pause == 0) && (asm_dir == 1) && (partner_pause == 1) && (partner_asm_dir == 0))
	{
		ptoe2->pause_frame_rx = false;
		ptoe2->pause_frame_tx = false;		
	}
	else if ((pause == 0) && (asm_dir == 1) && (partner_pause == 1) && (partner_asm_dir == 1))
	{
		ptoe2->pause_frame_rx = false;
		ptoe2->pause_frame_tx = true;		
	}
	else if ((pause == 1) && (asm_dir == 0) && (partner_pause == 0))
	{
		ptoe2->pause_frame_rx = false;
		ptoe2->pause_frame_tx = false;		
	}
	else if ((pause == 1) && (partner_pause == 1))
	{
		ptoe2->pause_frame_rx = true;
		ptoe2->pause_frame_tx = true;		
	}
	else if ((pause == 1) && (asm_dir == 1) && (partner_pause == 0) && (partner_asm_dir == 0))
	{
		ptoe2->pause_frame_rx = false;
		ptoe2->pause_frame_tx = false;		
	}
	else if ((pause == 1) && (asm_dir == 1) && (partner_pause == 0) && (partner_asm_dir == 1))
	{
		ptoe2->pause_frame_rx = true;
		ptoe2->pause_frame_tx = false;		
	}
	else
	{
		TOE2_WARNING("Impossiable!\n");
		//asm("sdbbp");
	}

	TOE2_INFO("phy_link_chk(): Tx flow control = %ld.", (long)ptoe2->pause_frame_tx);
	TOE2_INFO("phy_link_chk(): Rx flow control = %ld.", (long)ptoe2->pause_frame_rx);

	/*
	if (!toe2_use_R8032_patch)
		mac_mdio_write(dev, toe2_phy_addr, PhyBasicModeCtrl, (int)media);
	*/
}

static void toe2_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	strcpy(info->driver, TOE2_DRV_NAME);
	strcpy(info->version, TOE2_DRV_VER);
	strcpy(info->bus_info, "Local Bus");
}


static int toe2_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset)
	{
		case ETH_SS_STATS:
			return TOE2_NUM_STATS;
		default:
			return -EOPNOTSUPP;
	}
}

static int toe2_mii_ethtool_sset(struct mii_if_info *mii, struct ethtool_cmd *ecmd)
{
	struct net_device *dev = mii->dev;
	u32 bmcr;

	mii->mdio_write(dev, mii->phy_id, MII_ADVERTISE, ecmd->advertising);

	/* turn on autonegotiation, and force a renegotiate */
	bmcr = mii->mdio_read(dev, mii->phy_id, MII_BMCR);
	bmcr |= (BMCR_ANENABLE | BMCR_ANRESTART);
	mii->mdio_write(dev, mii->phy_id, MII_BMCR, bmcr);

	return 0;
}

static int toe2_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	int rc;
	unsigned long flags = 0;

	spin_lock_irqsave(&ptoe2->lock, flags);
	rc = mii_ethtool_gset(&ptoe2->mii_if, cmd);
	spin_unlock_irqrestore(&ptoe2->lock, flags);

	return rc;
}

static int toe2_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	int rc;
	unsigned long flags=0;

	spin_lock_irqsave(&ptoe2->lock, flags);
	if (cmd->advertising == 0)
	{
		rc = mii_ethtool_sset(&ptoe2->mii_if, cmd);
	}
	else
	{
		//For test, enable pasue write
		rc = toe2_mii_ethtool_sset(&ptoe2->mii_if, cmd);
	}
	spin_unlock_irqrestore(&ptoe2->lock, flags);

	return rc;
}

static int toe2_nway_reset(struct net_device *dev)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	return mii_nway_restart(&ptoe2->mii_if);
}

static u32 toe2_get_msglevel(struct net_device *dev)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	return ptoe2->msg_enable;
}

static void toe2_set_msglevel(struct net_device *dev, u32 value)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	ptoe2->msg_enable = value;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0))

#else
static u32 toe2_get_rx_csum(struct net_device *dev)
{
	return (u32)toe2_rx_csum;
}

static int toe2_set_rx_csum(struct net_device *dev, u32 data)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;
	bool chg = false;
	u8 tmp_8;

	if (toe2_rx_csum) 
	{
		if (!data)
		{
			chg = true;
			toe2_rx_csum = false;
		}
	}
	else //!toe2_rx_csum
	{
		if (data)
		{
			chg = true;
			toe2_rx_csum = true;
		}		
	}

	if (chg)
	{
		unsigned long flags=0;
		spin_lock_irqsave(&ptoe2->lock, flags);

		tmp_8 = TOE2_R8(SCR);
		if (toe2_rx_csum == true)
			TOE2_W8(SCR, (tmp_8 | SCRRxCoeEn));
		else
			TOE2_W8(SCR, (tmp_8 & (~SCRRxCoeEn)));

		spin_unlock_irqrestore(&ptoe2->lock, flags);
	}

	return 0;
}
#endif

static void toe2_get_regs(struct net_device *dev, struct ethtool_regs *regs, void *p)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	unsigned long flags=0;

	if (regs->len < TOE2_REGS_RANGE)
		return;

	regs->version = TOE2_REGS_VER;

	spin_lock_irqsave(&ptoe2->lock, flags);
	memcpy_fromio(p, (void *)ptoe2->io_base, TOE2_REGS_RANGE);
	spin_unlock_irqrestore(&ptoe2->lock, flags);
}

static const char toe2_ethtool_gstrings_stats[][ETH_GSTRING_LEN] = 
{
	"rx_mc",				//muticast packets received.	//ok
	"rx_bc",				//broadcast packets received.	//ok
	"rx_uc",				//unicast packets received.		//ok
	"rx_vlan",									//ok
	"rx_pppoe",			//pppoe packets received.	    	//ok
	"rx_ip",									//ok
	"rx_frag",									//ok
	"rx_runt_errors",							//ok
	"rx_long_errors",							//ok
	"rx_dribble_errors",						//ok
	"rx_phy_layer_errors",						//ok
	"rx_wd_timeout_errors",					    //ok
	"rx_ip_chksum_errors",						//ok
	"rx_tcp_chksum_errors",					    //ok
	"rx_buf_empty",		//there is no packet stored in rx ring buffer.
	"rx_late_col_seen",	
	"rx_lost_in_ring",
	"rx_hdr_chs_errs",							//ok
	"rx_pay_chs_errs",							//ok
	"tx_col_cnts[0]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[1]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[2]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[3]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[4]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[5]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[6]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[7]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[8]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[9]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[10]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[11]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[12]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[13]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[14]",		//0 to 15 collisions.		//ok
	"tx_col_cnts[15]",		//0 to 15 collisions.		//ok	
	"tx_col_errors",		//excessive collision.		//ok
	"rx_packets",		/* total packets received	*/
	"tx_packets",		/* total packets transmitted	*/
	"rx_bytes",		    /* total bytes received 	*/
	"tx_bytes",		    /* total bytes transmitted	*/
	"rx_errors",		/* bad packets received		*/
	"tx_errors",		/* packet transmit problems	*/
	"rx_dropped",		/* no space in linux buffers	*/
	"tx_dropped",		/* no space available in linux	*/
	"multicast",		/* multicast packets received	*/
	"collisions",
	"rx_length_errors",		/* detailed rx_errors: */
	"rx_over_errors",		/* receiver ring buff overflow	*/
	"rx_crc_errors",		/* recved pkt with crc error	*/
	"rx_frame_errors",		/* recv'd frame alignment error */
	"rx_fifo_errors",		/* recv'r fifo overrun		*/
	"rx_missed_errors",	    /* receiver missed packet	*/
	"tx_aborted_errors",	/* detailed tx_errors */
	"tx_carrier_errors",
	"tx_fifo_errors",
	"tx_heartbeat_errors",
	"tx_window_errors",
	"rx_compressed",		/* for cslip etc */
	"tx_compressed",
	/* repeated
		"rx_missed_errors",
		"rx_fifo_errors",
		"rx_long_errors",
		"rx_runt_errors",
		"rx_crc_errors",
		"rx_align_errors",
		*/	
};

#define TOE2_STATS_LEN	ARRAY_SIZE(toe2_ethtool_gstrings_stats)

static int toe2_ethtool_get_sset_count(struct net_device *dev, int sset)
{
	switch(sset)
	{
		case ETH_SS_STATS:
			return TOE2_STATS_LEN;
		default:
			return -EOPNOTSUPP;
	}
}

static void toe2_ethtool_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	switch(stringset)
	{
		case ETH_SS_STATS:
			memcpy(data, *toe2_ethtool_gstrings_stats, sizeof(toe2_ethtool_gstrings_stats));
			break;
	}	
}

static void toe2_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *estats, u64 *data)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	struct toe2_device_stats mac_stats;
	struct net_device_stats net_stats;
	long i;
	long count1;
	long count2;

	toe2_get_stats(dev);
	net_stats = dev->stats;
	mac_stats = ptoe2->mac_stats;

	count1 = sizeof(mac_stats) /4;
	count2 = sizeof(struct net_device_stats)/4;

	if ((count1 + count2) != TOE2_STATS_LEN)
	{
		TOE2_WARNING("toe2_get_ethtool_stats: error!\n");
		return;
	}

	for (i = 0; i < TOE2_STATS_LEN; i++)
	{
		if (i < count1)
		{
			data[i] = ((unsigned long *)&mac_stats)[i];
		}
		else
		{
			data[i] = ((unsigned long *)&net_stats)[i-count1];
		}
	}

	//printk("CRC error = %ld\n", dev->stats.rx_crc_errors);
	//printk("Align error = %ld\n", dev->stats.rx_frame_errors);
	//printk("CRC error = %ld\n"dev->stats.rx_crc_errors);
}

static const struct ethtool_ops toe2_ethtool_ops =
{
	.get_drvinfo		= toe2_get_drvinfo,
	.get_sset_count     = toe2_get_sset_count,
	.get_settings		= toe2_get_settings,
	.set_settings		= toe2_set_settings,
	.nway_reset         = toe2_nway_reset,
	.get_link			= ethtool_op_get_link,
	.get_msglevel		= toe2_get_msglevel,
	.set_msglevel		= toe2_set_msglevel,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0))

#else
	.get_rx_csum		= toe2_get_rx_csum,
	.set_rx_csum		= toe2_set_rx_csum,
	.set_tx_csum		= ethtool_op_set_tx_csum,
	.set_sg             = ethtool_op_set_sg,
	.set_tso			= ethtool_op_set_tso,
#endif
	.get_regs			= toe2_get_regs,
	.get_strings		= toe2_ethtool_get_strings,
	.get_sset_count	    = toe2_ethtool_get_sset_count,
	.get_ethtool_stats	= toe2_get_ethtool_stats,
};

irqreturn_t toe2_isr(int Irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	ptoe2_private ptoe2;
	UINT32 ba, cur_isr;

	if (unlikely(dev == NULL))
		return IRQ_NONE;
	ptoe2 = netdev_priv(dev);
	ba = ptoe2->io_base;

	//spin_lock(&ptoe2->lock);

	if (unlikely(!netif_running(dev)))
	{
		TOE2_W16(IMR, 0);
		//spin_unlock(&ptoe2->lock);
		return IRQ_HANDLED;
	}

	cur_isr = TOE2_R32(ISR);
	TOE2_W32(ISR, 0);

#ifdef TOE2_NAPI_DEBUG
	ptoe2->total_interrupt++;
#ifdef NAPI_MODE_SUPPORT
	if (ptoe2->during_pkt_rx == true)
#endif			
	{
		ptoe2->has_been_interrupted++;//very weird, why do we still have this even we use NAPI...
#ifdef NAPI_MODE_SUPPORT			
		TOE2_TRACE("weird NAPI int.");
#endif
		if (cur_isr & ISRRxComplete)
		{
			ptoe2->irq_due_to_rx_complete++;//weird because of ISRRxComplete
			TOE2_TRACE("ISRRxComplete.");	
		}
		if (cur_isr & ISRTimer)
		{	
			ptoe2->irq_due_to_timer++;//weird because of ISRTimer
			TOE2_TRACE("ISRTimer");
		}
		TOE2_TRACE("IMR=0x%x, ISR=0x%x.", TOE2_R32(IMR), TOE2_R32(cur_isr));
	}
	//TOE2_TRACE("ISR, dev_id = 0x%x", (u8 *)dev_id);
	/*		
			if(ptoe2->cur_isr & ISRRxComplete)
			ptoe2->irq_due_to_rx_complete++;
			if(ptoe2->cur_isr & ISRTimer)
			ptoe2->irq_due_to_timer++;
			*/
#endif

#ifdef DYNAMIC_CHANGING_IMK_DBG
	if (cur_isr & ISRRxComplete)
		ptoe2->num_rx_complete++;
	if (cur_isr & ISRTimer)
		ptoe2->num_timer++;
#endif			

	if (cur_isr & ptoe2->isr_mask)
	{
		if (cur_isr & ISRRxBufDiscard)
			TOE2_TRACE("%s()=>isr(0x%08lX).", __FUNCTION__, cur_isr);
		ptoe2->cur_isr = (cur_isr & ptoe2->isr_mask);

		if (C3821 == g_chip_id)
        {
#ifdef PHY_BLINK_USING_GPIO
            if (g_enet_link_gpio >= 0)
		    {
		        link_light_blink(ptoe2);
			}
#endif
			
            if (ptoe2->cur_isr & ISRTimer)
		    {
                mac_pll_time++;
                if (mac_pll_time > mac_5sec)
			    {
                    /*if (test_bit(10, &dbg_runtime_val))
				    {
                        MAC_PRINTF("times_of_link_change %d, mac_pll_time %d mac_5sec %d\n", padapter->times_of_link_change, 
                            mac_pll_time, mac_5sec);
                    }*/
                    if (ptoe2->times_of_link_change >= MAX_LINK_CHANGE_IN_5SEC)
				    {
                        do_dll_pd_reset();
                    }
                    ptoe2->times_of_link_change = 0;
                    mac_pll_time = 0;
                }
            }
        }

		if (toe2_rx_mode == TOE2_RX_MODE_NAPI)
		{
			/****************************************************************************************/
#if 0
			if (ptoe2->cur_isr & (ISRLinkStatus|ISRTimer |ISRWatchdog|ISRTxUnderrun))
				//if (ptoe2->cur_isr & (ISRLinkStatus|ISRWatchdog|ISRTxUnderrun))//do not put timer here.
			{
				tasklet_schedule(&toe2_tasklet2);
				//goto early_return;
			}

			if (ptoe2->cur_isr &
					(ISRRxComplete | ISRRxFifoOverflow | ISRRxBufOverflow |
					 ISRRxBufDiscard | ISRTimer))
			{

#ifdef KERNEL_VERSION_2_6_28 // 2.6.28
				if(likely(netif_rx_schedule_prep(&ptoe2->napi))) {
					toe2_disable_irq(ptoe2);
					__netif_rx_schedule(&ptoe2->napi);
				}
#else				// 2.6.32
				if (likely(napi_schedule_prep(&ptoe2->napi))) {
					toe2_disable_irq(ptoe2);
					__napi_schedule(&ptoe2->napi);
				}
#endif
			}

#else

#if 1//NAPI_fast, related to TOE2_INTERRUPT_MASK!!!
			//if (ptoe2->cur_isr & (ISRLinkStatus |ISRTimer |ISRWatchdog|ISRTxUnderrun))
			{
				tasklet_schedule(&toe2_tasklet2);
				//mac_weird_tasklet(0);
			}

			if (ptoe2->cur_isr &
				(ISRRxComplete | ISRRxFifoOverflow | ISRRxBufOverflow | ISRRxBufDiscard | ISRTimer))
			{
#ifdef DYNAMIC_CHANGING_IMK
				if ((ptoe2->cur_isr&ISRRxComplete) && (timer_period > 0))
				{
					if ((jiffies-ptoe2->spot_rx_complete_int_HZ)>JUDGE_TOO_MANY_RX_INTERRUPT_PERIOD)
					{
						ptoe2->spot_rx_complete_int_HZ = jiffies;
						ptoe2->num_rx_complete_int_HZ = 0;
					}
					else
					{
						ptoe2->num_rx_complete_int_HZ++;
						if (ptoe2->num_rx_complete_int_HZ>TOO_MANY_RX_INTERRUPT)
						{	
							dny_imk2high(ptoe2);
							DNY_INT_TRACE("TOE2: dny_imk2high!");
							//printk("TOE2:dny_imk2high!\n");
							//register of IMR will be renewed after toe2_enable_irq.
						}
					}
				}
#endif

#ifdef KERNEL_VERSION_2_6_28 // 2.6.28
				if (likely(netif_rx_schedule_prep(&ptoe2->napi)))
				{
					toe2_disable_irq(ptoe2);
					__netif_rx_schedule(&ptoe2->napi);
				}
#else				// 2.6.32
				if (likely(napi_schedule_prep(&ptoe2->napi)))
				{
					toe2_disable_irq(ptoe2);
#ifdef TOE2_NAPI_DEBUG					
					g_ptoe2->during_pkt_rx = true;
#endif
					__napi_schedule(&ptoe2->napi);
				}
				//else
				//{
				//    printk("no!no!no!\n");
				//}
#endif
			}
#else//NAPI_slow
			if (ptoe2->cur_isr & (ISRLinkStatus | ISRTimer | ISRWatchdog | ISRTxUnderrun))
			{
				mac_weird_tasklet(0);
			}

			if (ptoe2->cur_isr &
				(ISRRxComplete | ISRRxFifoOverflow | ISRRxBufOverflow | ISRRxBufDiscard))
			{

#ifdef KERNEL_VERSION_2_6_28 // 2.6.28
				if (likely(netif_rx_schedule_prep(&ptoe2->napi)))
				{
					toe2_disable_irq(ptoe2);
					__netif_rx_schedule(&ptoe2->napi);
				}
#else			// 2.6.32
				if (likely(napi_schedule_prep(&ptoe2->napi)))
				{
					toe2_disable_irq(ptoe2);
					__napi_schedule(&ptoe2->napi);
				}
#endif
			}
#endif
#endif		
			/****************************************************************************************/
		}
		else if (toe2_rx_mode == TOE2_RX_MODE_TASKLET)
		{
			tasklet_schedule(&toe2_tasklet);
		}
		else 
		{//TOE2_RX_MODE_ISR
			mac_rx_pkts(ptoe2, 0);
		}
	}
	else
	{
		if(cur_isr & 0x1FFF)
		{
			if (netif_msg_ifup(ptoe2))
				TOE2_WARNING("toe2_isr()=>isr(0x%08lX) not supported.", cur_isr);
		}
	}

	//spin_unlock(&ptoe2->lock);
	return IRQ_HANDLED;//IRQ_RETVAL(true);
}

static int toe2_open(struct net_device *dev)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba, tmp_u32;
	int i;

	if (netif_msg_ifup(ptoe2))
		TOE2_INFO("%s: enabling interface.", dev->name);

	ba = ptoe2->io_base;
	mac_chip_rst(ba);

    tmp_u32 = TOE2_R32(MiiMR1);
#ifdef HW_MDIO
	tmp_u32 &= ~MiiMdioEn;
#else
	tmp_u32 |= MiiMdioEn;
#endif //HW_MDIO
	TOE2_W32(MiiMR1, tmp_u32);
	
	mac_alloc_rings(ptoe2);
	mac_cnt_init(ptoe2);
	
    /* for PLL reset patch for ALi Phy (eg:38/39) */
    ptoe2->times_of_link_change = 0; //when link change, it shouldn't set to 0
    ptoe2->unlink_error_state = 0;
    ptoe2->para_detect_times = 0;
    ptoe2->unlink_with_signal_times = 0;

	netif_carrier_off(dev);

	//detect phy address
	if (phy_detect(ptoe2) < 0)
	{
		printk("TOE2: Unable to use phy! try to reset it!\n");
		//ali_gpio_set_value(g_RMII_RESET_PIN, 0);
		//mdelay(200);
		//ali_gpio_set_value(g_RMII_RESET_PIN, 1);

		goto open_err_out;
	}

	if (!toe2_use_R8032_patch)
		phy_set(ptoe2);

#ifdef DYNAMIC_CHANGING_IMK
	ptoe2->timer_freq/*toe2_timer_frq*/ = TIMER_200_FREQ;
#endif

	TOE2_W32(TimerR, ptoe2->timer_freq/*toe2_timer_frq*/);

	//napi_enable(&ptoe2->napi);
	//netif_start_queue(dev);
	//toe2_enable_irq(ptoe2);//shouldn't enable them here!

	TOE2_W16(IMR, (ISRTimer | ISRLinkStatus));//right now only passes TIMER/LINK interrupt!!!

	i = request_irq(ptoe2->irq_num, toe2_isr, IRQF_DISABLED, dev->name, (void *)dev);
	if (i != 0)
	{
		printk("[Err] TOE2: Unable to use IRQ = %d, errno = %d.\n", ptoe2->irq_num, i);
		goto open_err_out;
	}

	ptoe2->acquired_isr = true;

#ifdef WORK_QUEUE_TEST
	skb_queue_head_init(&my_wq.special_pkt_queue);
	INIT_WORK(&my_wq.special_pkt_work, my_wq_handler);
#endif
	toe2_device_open = true;
	return 0;

open_err_out:
	if (toe2_rx_mode == TOE2_RX_MODE_NAPI)
	{
		//modified by davy.lee@alitech.com ,for supporting 15880
		//when there is no PHY,it can't to do napi_enable in function of mac_set(),and here not to disable
		//---->>
		if (test_bit(NAPI_STATE_SCHED, &((ptoe2->napi).state)) == 0) //if it's 1, means already disabled.
			//<<---
			napi_disable(&ptoe2->napi);
	}	
	mac_free_rings(ptoe2);
	return -EFAULT;
}

static int toe2_close(struct net_device *dev)
{
	struct toe2_private *ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base; 
	UINT32 flags=0;

	toe2_device_open = false;

	if (toe2_rx_mode == TOE2_RX_MODE_NAPI)
	{	
		if (test_bit(NAPI_STATE_SCHED, &((ptoe2->napi).state)) == 0) //if it's 1, means already disabled.
			napi_disable(&ptoe2->napi);
		//netif_stop_queue(dev);
	}	

	TOE2_W16(IMR, 0);
	TOE2_W8(SCR, 0);

	if (netif_msg_ifdown(ptoe2))
		TOE2_INFO("%s: disabling interface.", dev->name);

	spin_lock_irqsave(&ptoe2->lock, flags);
	netif_stop_queue(dev);
	netif_carrier_off(dev);	
	mac_free_rings(ptoe2);

#ifdef TOE2_NAPI_DEBUG
	proc_net_remove(&init_net, "toe2");
#endif	
	spin_unlock_irqrestore(&ptoe2->lock, flags);

	return 0;
}

static void toe2_set_rx_mode(struct net_device *dev)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;
	//struct dev_addr_list *mc_list;
	//u8 cmd_u8;
	UINT32 cmd_u32=0;
	UINT32 flags=0;
	//int i;

	spin_lock_irqsave(&ptoe2->lock, flags);

	cmd_u32 = TOE2_R32(NetworkOM);
	cmd_u32 &= ~(PassMask);

	if (dev->flags & IFF_PROMISC)
	{
		TOE2_W32(NetworkOM, (cmd_u32|PassPromiscuous));
		TOE2_INFO("%s()=>(dev->flags & IFF_PROMISC).", __FUNCTION__);
	}

#if 0
	else if ((dev->flags & IFF_ALLMULTI) ||(dev->mc_count >14))
	{
		TOE2_W32(NetworkOM, (cmd_u32|PassAllMulticast));
		TOE2_TRACE("%s()=>((dev->flags & IFF_ALLMULTI) ||(dev->mc_count >14)).", __FUNCTION__);
	}
	else
	{
		if ((dev->mc_count == 0) ||(dev->mc_list == NULL))
		{
			TOE2_WARNING("%s()=>((dev->mc_count == 0) ||(dev->mc_list == NULL)).", __FUNCTION__);
			goto multicast_done;
		}

		if (netif_msg_probe(ptoe2)) 
		{
			for (i = 0, mc_list = dev->mc_list; mc_list && i < dev->mc_count; i++)
			{
				TOE2_TRACE("%s()=>MC(%d) = %02x-%02x-%02x-%02x-%02x-%02x", 
						__FUNCTION__, i, 
						mc_list->da_addr[0], mc_list->da_addr[1], mc_list->da_addr[2],
						mc_list->da_addr[3], mc_list->da_addr[4], mc_list->da_addr[5]);
				mc_list = mc_list->next;
			}
		}

		if (0 != mac_set_mc_filter(dev))
		{
			//build setup frame failed, set mc all.
			TOE2_W32(NetworkOM, (cmd_u32|PassAllMulticast));

			//make sure rx & tx enabled.
			cmd_u8 = TOE2_R8(SCR);
			TOE2_W8(SCR, cmd_u8 | (SCRRxEn| SCRTxEn));
			goto multicast_done;
		}

		mac_load_setup(ptoe2);
	}
multicast_done:
#endif
	spin_unlock_irqrestore(&ptoe2->lock, flags);
}

static struct net_device_stats *toe2_get_stats(struct net_device *dev)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;

	unsigned long flags = 0;
	u16 tmp_u16;

	spin_lock_irqsave(&ptoe2->lock, flags);

	if (netif_running(dev) && netif_device_present(dev))
	{
		tmp_u16 = TOE2_R16(MFC);
		//printk("Missed error = %d\n", tmp_u16);
		TOE2_W16(MFC, 0);
		dev->stats.rx_missed_errors += tmp_u16;
		dev->stats.rx_over_errors += tmp_u16;
		//ptoe2->rx_stats.rx_missed_errors += tmp_u16;

		tmp_u16 = TOE2_R16(PPC);
		//printk("FIFO error = %d\n", tmp_u16);
		TOE2_W16(PPC, 0);
		dev->stats.rx_fifo_errors+= tmp_u16;
		//ptoe2->rx_stats.rx_fifo_errors += tmp_u16;

		tmp_u16 = TOE2_R16(LFC);
		//printk("Long error = %d\n", tmp_u16);
		TOE2_W16(LFC, 0);
		dev->stats.rx_length_errors += tmp_u16;
		//ptoe2->rx_stats.rx_long_errors += tmp_u16;
		ptoe2->mac_stats.rx_long_errors += tmp_u16;

		tmp_u16 = TOE2_R16(RPC);
		//printk("Runt error = %d\n", tmp_u16);
		TOE2_W16(RPC, 0);
		dev->stats.rx_length_errors += tmp_u16;
		//ptoe2->rx_stats.rx_runt_errors += tmp_u16;
		ptoe2->mac_stats.rx_runt_errors += tmp_u16;

		tmp_u16 = TOE2_R16(CrcErrCnt);
		//printk("CRC error = %d\n", tmp_u16);
		TOE2_W16(CrcErrCnt, 0);
		dev->stats.rx_crc_errors += tmp_u16;
		//ptoe2->rx_stats.rx_crc_errors += tmp_u16;

		tmp_u16 = TOE2_R16(AlignErrCnt);
		//printk("Align error = %d\n", tmp_u16);
		TOE2_W16(AlignErrCnt, 0);
		dev->stats.rx_frame_errors += tmp_u16;
		//ptoe2->rx_stats.rx_align_errors += tmp_u16;
	}

	spin_unlock_irqrestore(&ptoe2->lock, flags);

	return &dev->stats;
}

static int toe2_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	int rc;
	unsigned long flags = 0;
	struct ali_mac_priv_io *mac_data;

	typedef struct _ali_reg_t
	{
		u32 cmd;
		u32 addr;
		u32 val;
	} ali_reg_t;

	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;	

#ifdef UTILITY_SUPPORT
	extern MAC_ADAPTER util_mac_adapter;
	struct tagMAC_transfer_ifr *mac_config;
	MAC_Init_Context u_init_cxt;
	MAC_Rx_Context u_rx_cxt;
	MAC_Tx_Context u_tx_cxt;
	MAC_Status_Context u_st_cxt;
#endif

	if ((cmd >= SIOCDEVPRIVATE) && (cmd <= SIOCDEVPRIVATE + 15))
	{
		rc = 0;
		spin_lock_irqsave(&ptoe2->lock, flags);
		switch (cmd)
		{
			case SIOCDEVPRIVATE+1:
				{
					mac_data = (struct ali_mac_priv_io *)&(rq->ifr_ifru);
					if (mac_data->reg > TOE2_REGS_RANGE)
					{
					TOE2_INFO("The requested register is out of range!");
					break;
				}
				mac_data->value = TOE2_R32(mac_data->reg);
				break;
			}

			case SIOCDEVPRIVATE+2:
			{
				mac_data = (struct ali_mac_priv_io *)&(rq->ifr_ifru);
				if (mac_data->reg > TOE2_REGS_RANGE)
				{
					TOE2_INFO("The requested register is out of range!");
					break;
				}
				TOE2_W32(mac_data->reg, mac_data->value);
				break;
			}

#ifdef UTILITY_SUPPORT
			case SIOCDEVPRIVATE+3:
			{
				mac_config = (struct tagMAC_transfer_ifr *)&(rq->ifr_ifru);
				TOE2_UTILITY_TRACE("enter utility mode: %d, %x", mac_config->cmd_type, mac_config->pointer);
				switch (mac_config->cmd_type)
				{
					case 0:
					case '0':
				        TOE2_UTILITY_TRACE("MAC_Init:");
						copy_from_user(&u_init_cxt, mac_config->pointer, sizeof(MAC_Init_Context));
						util_mac_init(&u_init_cxt);
						break;
					case 1:
					case '1':
						TOE2_UTILITY_TRACE("MAC_Rx_Start:");
						copy_from_user(&u_rx_cxt, mac_config->pointer, sizeof(MAC_Rx_Context));
						util_mac_rx_start(&u_rx_cxt);
						break;
					case 2:
					case '2':
						TOE2_UTILITY_TRACE("MAC_Rx_Stop:");
						util_mac_rx_stop(&util_mac_adapter);
						break;
					case 3:
					case '3':
						TOE2_UTILITY_TRACE("MAC_Tx_Start:");
						copy_from_user(&u_tx_cxt, mac_config->pointer, sizeof(MAC_Tx_Context));
						util_mac_tx_start(&u_tx_cxt);
						break;
					case 4:
					case '4':
						TOE2_UTILITY_TRACE("MAC_Tx_Stop:");
						util_mac_tx_stop();
						break;
					case 5:
					case '5':
						TOE2_UTILITY_TRACE("MAC_Tx_Setup:");
						break;
					case 7:
					case '7':
						TOE2_UTILITY_TRACE("MAC_Status:");
						memset(&u_st_cxt, 0, sizeof(MAC_Status_Context));
						util_mac_status(&u_st_cxt, &util_mac_adapter);
						copy_to_user(mac_config->pointer, &u_st_cxt, sizeof(MAC_Status_Context));
						break;
				}
				break;
			}
#endif

			case SIOCDEVPRIVATE+4:
			{    
				ali_reg_t ali_reg;
				u32 val; 
				if (copy_from_user(&ali_reg, rq->ifr_data, sizeof(ali_reg)))
				{    
					printk("copy_from_user error\n");
					spin_unlock_irqrestore(&ptoe2->lock, flags);
					return -EINVAL;
				}    
				if (ali_reg.cmd == 0)
				{    
					val = SOC_R32(ali_reg.addr);   
					printk("read reg addr is 0x%x\n", ali_reg.addr);
					printk("          val is 0x%08x\n", val);
				}    
				if (ali_reg.cmd == 1) { 
					printk("write reg addr is 0x%x\n", ali_reg.addr);
					printk("           val is 0x%08x\n", ali_reg.val);
					SOC_W32(ali_reg.addr, ali_reg.val);
				}     
				break;
			}

			case SIOCDEVPRIVATE+5:
			{
				/* toe2_clean_stats */
				//toe2_get_stats(dev);
				memset(&(ptoe2->mac_stats), 0, sizeof(struct toe2_device_stats));
				memset(&(dev->stats), 0, sizeof(struct net_device_stats));
#ifdef DYNAMIC_CHANGING_IMK_DBG
				ptoe2->num_rx_complete = 0;
				ptoe2->num_timer = 0;
#endif          
				break;
			}

			case SIOCDEVPRIVATE+6:
			{
#ifdef DYNAMIC_CHANGING_IMK_DBG
				printk("[*] num_rx_complete = %d\n", ptoe2->num_rx_complete);
				printk("[*] ptoe2->num_timer = %d\n", ptoe2->num_timer++);
#endif
				break;
			}

			default:
			{
				rc = -EOPNOTSUPP;
				break;
			}

		}
		spin_unlock_irqrestore(&ptoe2->lock, flags);
		return rc;
	}

	if (!netif_running(dev))
	{
		return -EINVAL;
	}

	spin_lock_irqsave(&ptoe2->lock, flags);
	rc = generic_mii_ioctl(&ptoe2->mii_if, if_mii(rq), cmd, NULL);
	spin_unlock_irqrestore(&ptoe2->lock, flags);
	return rc;
}

static int toe2_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;
	UINT32 flags = 0;
	ptx_desc desc;
	u16 tx_wptr, tx_rptr;
	u16 desc_num;
	u16 off, first, last, gso_type = 0;
	u8 hdr_len = 0;
	int err, ret=NETDEV_TX_OK;
	struct skb_shared_info *skb_info = skb_shinfo(skb);
	volatile static u8 temp_buff[TOE2_DESC_SZ];

	spin_lock_irqsave(&ptoe2->lock, flags);

	if (!ptoe2->link_established)
	{
		dev_kfree_skb_any(skb);	
		goto xmit_done;
	}

#ifdef TX_ASYNC_XMIT
	free_tx_skb(g_ptoe2, false);
#endif		

	desc_num = skb_info->nr_frags + 1;
	tx_wptr = ptoe2->tx_wptr;

#ifdef TX_ASYNC_XMIT
	//tx_rptr = TOE2_R16(TxRingDesRPtr);
	if (tx_skb_rd != tx_skb_wr)
	{
		tx_rptr = g_tx_skb[tx_skb_rd].first;
		if ((tx_wptr>tx_rptr && TOE2_TX_DESC_NUM-(tx_wptr-tx_rptr) < desc_num+1) ||
			(tx_wptr<tx_rptr && tx_rptr-tx_wptr<desc_num+1)) //full
		{		
			//dev_kfree_skb_any(skb);	
			TOE2_TRACE("%s %d: tx ring desc full.\n", __FUNCTION__, __LINE__);
			ret=NETDEV_TX_BUSY;
			goto xmit_done;
		}
	}
#endif

	if (skb_is_gso(skb))
	{
		if (skb_header_cloned(skb))
		{
			err = pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
			if (err)
			{
				spin_unlock_irqrestore(&ptoe2->lock, flags);
				return err;
			}
		}
		gso_type = skb_info->gso_type;
	}

	first = tx_wptr;
	last = first + skb_info->nr_frags;

	off = tx_wptr;
	if (skb_shinfo(skb)->nr_frags == 0)
	{
		mac_tx_cfg(ptoe2, off, skb, skb->data, skb->len);
		desc = &ptoe2->tx_desc[off];
		desc->tx_sts.hw.OWN = 1;
		desc->tx_sts.hw.FS = 1;
		desc->tx_sts.hw.LS = 1;
		desc->tot_len = skb->len;
		desc->seg_num = 1;

		if (gso_type & SKB_GSO_TCPV4)
		{
			hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
			desc->tx_sts.hw.Mfl = skb_info->gso_size + hdr_len;
			//printk("TSO: ");
		}
		else if (gso_type & SKB_GSO_UDP)
		{
			desc->tx_sts.hw.Mfl  = dev->mtu + 14;
			//desc->tx_sts.hw.Mfl = TOE2_MSS;
			//printk("UFO: ");
		}
		else
			desc->tx_sts.hw.Mfl = TOE2_MSS;

		//printk("len(%d), nr_frags(%d), Mfl(%d)\n", skb->len, desc_num, desc->tx_sts.hw.Mfl);

		if ((++off) >= TOE2_TX_DESC_NUM)
			off = 0;
	}
	else //skb_shinfo(skb)->nr_frags >= 1
	{
		//ptx_desc desc;
		u32 first_len, frag_len;
		int frag;
		void *seg_addr;

		first_len = skb_headlen(skb);
		mac_tx_cfg(ptoe2, off, skb, skb->data, first_len);
		desc = &ptoe2->tx_desc[off];
		desc->tx_sts.hw.OWN = 1;
		desc->tx_sts.hw.FS = 1;
		desc->tot_len = skb->len;
		desc->seg_num = desc_num;

		if (gso_type & SKB_GSO_TCPV4)
		{
			hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
			desc->tx_sts.hw.Mfl = skb_info->gso_size + hdr_len;
			//printk("TSO: ");
		}
		else if (gso_type & SKB_GSO_UDP)
		{
			desc->tx_sts.hw.Mfl  = dev->mtu + 14;
			//desc->tx_sts.hw.Mfl = TOE2_MSS;
			//printk("UFO: ");
		}
		else
			desc->tx_sts.hw.Mfl = TOE2_MSS;

		//printk("len(%d), nr_frags(%d), Mfl(%d)\n", skb->len, desc_num, desc->tx_sts.hw.Mfl);

		if ((++off) >= TOE2_TX_DESC_NUM)
			off = 0;

		for (frag = 0; frag < skb_shinfo(skb)->nr_frags; frag++) 
		{
			skb_frag_t *this_frag = &skb_shinfo(skb)->frags[frag];

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0))
			seg_addr = (void *)page_address(this_frag->page) + this_frag->page_offset;
#else
			seg_addr =  (void *)skb_frag_address(this_frag);
#endif
			frag_len = this_frag->size;
			mac_tx_cfg(ptoe2, off, NULL, seg_addr, frag_len);

			if (frag == skb_shinfo(skb)->nr_frags - 1)
			{
				desc = &ptoe2->tx_desc[off];
				desc->tx_sts.hw.LS = 1;
			}

			if ((++off) >= TOE2_TX_DESC_NUM)
				off = 0;
		}
	}
	memcpy((void*)temp_buff, (void*)desc, TOE2_DESC_SZ);

	TOE2_W16(TxRingDesWPtr, off); //printk("off = %d\n", off);	
	ptoe2->tx_wptr = off;

#ifndef TX_ASYNC_XMIT
	do
	{
		tx_rptr = TOE2_R16(TxRingDesRPtr);
		/*
		   tx_wptr = ptoe2->tx_wptr;

		   if(tx_rptr > tx_wptr)
		   available = tx_rptr - tx_wptr -1;
		   else
		   available = TOE2_TX_DESC_NUM + tx_rptr - tx_wptr -1;
		   } while(available < desc_num);
		   */
    } while(ptoe2->tx_wptr != tx_rptr);

    desc = &ptoe2->tx_desc[first];
    mac_tx_sts(ptoe2, desc);

    if (ptoe2->tx_skb[first])
    {
	    dev_kfree_skb_any(ptoe2->tx_skb[first]);	
	    ptoe2->tx_skb[first] = NULL;

	    do
	    {
		    if (desc->seg_len > 0)
		    {
			    dma_unmap_single((struct device *)NULL, desc->pkt_buf_dma, desc->seg_len, DMA_TO_DEVICE);
		    }
		    else
		    {
			    //asm("sdbbp");
		    }

		    if ((++first) >= TOE2_TX_DESC_NUM)
		    {
			    first = 0;
		    }
		    desc = &ptoe2->tx_desc[first];
		    desc_num --;

	    } while(desc_num > 0);		
    }
    else
    {
	    //asm("sdbbp");
    }
#else
    g_tx_skb[tx_skb_wr].first = first;
    g_tx_skb[tx_skb_wr].cnt = desc_num;
    g_tx_skb[tx_skb_wr].skb = skb;
    if (tx_skb_wr+1 == TOE2_TX_DESC_NUM)
	    tx_skb_wr=0;
	else
	    tx_skb_wr++;
#endif	

xmit_done:	
    spin_unlock_irqrestore(&ptoe2->lock, flags);

    return ret;
}

static void toe2_tx_timeout(struct net_device *dev)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;
	unsigned long flags=0; 

	TOE2_WARNING("%s: toe2_tx_timeout()...");

	spin_lock_irqsave(&ptoe2->lock, flags);

	ba = ptoe2->io_base;
	mac_desc_clean(ptoe2);
	mac_chip_rst(ba);
	mac_cnt_init(ptoe2);
	mac_hw_start(ptoe2);

	netif_wake_queue(dev);

	spin_unlock_irqrestore(&ptoe2->lock, flags);

	return;
}

#if (TOE2_VLAN_TAG_USED)
static void toe2_vlan_rx_register(struct net_device *dev, struct vlan_group *grp)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba = ptoe2->io_base;
	unsigned long flags=0, tmp_u32=0;

	spin_lock_irqsave(&ptoe2->lock, flags);

	ptoe2->vlgrp = grp;
	ptoe2->vlan_tag_remove = true;

	tmp_u32 = TOE2_R32(TxRxCR2);
	TOE2_W32(TxRxCR2, (tmp_u32|RxRemoveVlanTagEn|VlanEn));

	spin_unlock_irqrestore(&ptoe2->lock, flags);
}
#endif //TOE2_VLAN_TAG_USED

static int toe2_set_mac_address(struct net_device *dev, void *p)
{
	ptoe2_private ptoe2 = netdev_priv(dev);
	UINT32 ba;	
	u8 *da = NULL;
	struct sockaddr *addr=p;

	if (netif_running(dev))
		return -EBUSY;

	ba = ptoe2->io_base;

	memcpy(dev->dev_addr, addr->sa_data,dev->addr_len);

	da = (u8 *)dev->dev_addr;

	TOE2_W32(PAR - 4, da[0] << 8 | da[1]);
	TOE2_W32(PAR, ((da[2] << 24) | (da[3] << 16) | (da[4] << 8) | da[5]));

	return 0;		
}

static const struct net_device_ops 
toe2_netdev_ops =
{
	.ndo_open		    = toe2_open,
	.ndo_stop		    = toe2_close,
	.ndo_validate_addr	= eth_validate_addr,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0))
    .ndo_set_rx_mode    =  toe2_set_rx_mode,
#else
    .ndo_set_multicast_list = toe2_set_rx_mode,
#endif
	.ndo_get_stats		= toe2_get_stats,
	.ndo_do_ioctl		= toe2_ioctl,
	.ndo_start_xmit		= toe2_start_xmit,
	.ndo_tx_timeout		= toe2_tx_timeout,
#if (TOE2_VLAN_TAG_USED)
	.ndo_vlan_rx_register	= toe2_vlan_rx_register,
#endif
	.ndo_set_mac_address    = toe2_set_mac_address,
};

static int toe2_attach(void)
{
	struct net_device *dev;
	struct toe2_private *ptoe2;
	int re;
	void *rx_desc = NULL;
	void *tx_desc = NULL;	
	void *setup_buf = NULL;

	//Externel clock
	//unsigned long tmp_u32 = (*(volatile unsigned long*) 0xb80000ac);
	//*((volatile unsigned long *)0xb80000ac) = (tmp_u32 | (1<<24));

	dev = alloc_etherdev(sizeof(struct toe2_private));
	if (!dev)
	{
		printk("[Err] TOE2: alloc_etherdev fail!\n");
		return -ENOMEM;
	}

	ptoe2 = netdev_priv(dev);
	g_ptoe2 = ptoe2;

	ptoe2->dev = dev;
	ptoe2->io_base = toe2_base - ALI_REGS_PHYS_BASE + ALI_REGS_VIRT_BASE;
	ptoe2->irq_num = toe2_irq;

	ptoe2->msg_enable = (toe2_debug < 0 ? TOE2_DEF_MSG_ENABLE : toe2_debug);
	spin_lock_init (&ptoe2->lock);
	ptoe2->mii_if.dev = dev;
	ptoe2->mii_if.mdio_read = mac_mdio_read;
	ptoe2->mii_if.mdio_write = mac_mdio_write;
	ptoe2->mii_if.phy_id = toe2_phy_addr;
	ptoe2->mii_if.phy_id_mask = 0x1f;
	ptoe2->mii_if.reg_num_mask = 0x1f;

	dev->dev_addr[0]=LOBYTE(LOWORD(toe2_mac_lo32));
	dev->dev_addr[1]=HIBYTE(LOWORD(toe2_mac_lo32));
	dev->dev_addr[2]=LOBYTE(HIWORD(toe2_mac_lo32));
	dev->dev_addr[3]=HIBYTE(HIWORD(toe2_mac_lo32));
	dev->dev_addr[4]=LOBYTE(LOWORD(toe2_mac_hi16));
	dev->dev_addr[5]=HIBYTE(LOWORD(toe2_mac_hi16));
	dev->irq = ptoe2->irq_num;

	dev->netdev_ops = &toe2_netdev_ops;

	/********************************************************************
VALUE_ADJUST:
netif_napi_add(dev, &ptoe2->napi, mac_rx_poll, 256);
256 -- how many packets linux network stack process at a time.
it should not be too big, because it will cause longer latency,
also because mac_rx_poll is in softirq or ksoftirq, it might affect other threads.
	 ********************************************************************/
	if (toe2_rx_mode == TOE2_RX_MODE_NAPI)
		netif_napi_add(dev, &ptoe2->napi, mac_rx_poll, 256);//peter, I think this could be larger, like 256 instead of 16. too big???

	dev->ethtool_ops = &toe2_ethtool_ops;
	dev->watchdog_timeo = TOE2_TX_TIMEOUT;

#if (TOE2_VLAN_TAG_USED)
	dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
#endif

	if (toe2_sg)
		dev->features |= NETIF_F_SG | NETIF_F_FRAGLIST;
	if (toe2_tso)
		dev->features |= NETIF_F_TSO;
	if (toe2_ufo)
		dev->features |= NETIF_F_UFO | NETIF_F_HW_CSUM;
	if (toe2_tx_csum)
		dev->features |= NETIF_F_IP_CSUM; //support TCP/UDP checksum over IPv4 

	dev->flags |= IFF_PROMISC | IFF_MULTICAST;

	re = register_netdev(dev);
	if (re)
		goto err_out_free;

#ifdef TOE2_NAPI_DEBUG
	if (!proc_net_fops_create(&init_net, "toe2", S_IRUGO, &toe2_seq_fops))
		goto err_out_free;
#endif

	rx_desc = dma_alloc_coherent((struct device *)NULL, (TOE2_DESC_SZ*TOE2_RX_DESC_NUM),
			&ptoe2->rx_desc_dma, GFP_KERNEL);

	tx_desc = dma_alloc_coherent((struct device *)NULL, (TOE2_DESC_SZ*TOE2_TX_DESC_NUM),
			&ptoe2->tx_desc_dma, GFP_KERNEL);

	setup_buf = dma_alloc_coherent((struct device *)NULL, SETUP_FRAME_SZ,
			&ptoe2->setup_buf_dma, GFP_KERNEL);

	memset(rx_desc, 0, (TOE2_DESC_SZ * TOE2_RX_DESC_NUM));
	memset(tx_desc, 0, (TOE2_DESC_SZ * TOE2_TX_DESC_NUM));
	memset(setup_buf, 0, SETUP_FRAME_SZ);

	ptoe2->rx_desc = (prx_desc)rx_desc;
	ptoe2->tx_desc = (ptx_desc)tx_desc;
	ptoe2->setup_buf = (u8 *)setup_buf;

#ifdef UTILITY_SUPPORT
    util_mac_rx_thread_create();
    util_mac_tx_thread_create();
#endif

	return 0;

err_out_free:
	free_netdev(dev);
	return re;
}

static int ali_mac_suspend(struct platform_device *pdev , pm_message_t state)
{
	if (netif_running(g_ptoe2->dev))
	{
		TOE2_INFO("nmp_mac_suspend.\n");    
		g_ptoe2->dev->flags &= ~IFF_UP;
		toe2_close(g_ptoe2->dev);
	}
	////SET_BITE(HAL_PINMUX_CTRL_REG, 0, 1);
	////WRITE_DATA_OR(GPIOB_CTL_REG, 0x0ffe0000);
	return 0;
}

static int ali_mac_resume(struct platform_device *pdev)
{
	////WRITE_DATA_AND(GPIOB_CTL_REG, ~0x0ffe0000);
	////SET_BITE(HAL_PINMUX_CTRL_REG, 0, 0);
	//mzhu........
	//ali_gpio_set_value(g_RMII_RESET_PIN, 0);
	//mdelay(200);
	//ali_gpio_set_value(g_RMII_RESET_PIN, 1);
	return 0;
}

static struct platform_driver ali_mac_driver =
{ 
	.driver     = { 
		.name   = "ali_mac",
		.owner  = THIS_MODULE,
	},  
	.suspend    = ali_mac_suspend,
	.resume     = ali_mac_resume,
};

static int read_timer(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char buf[10];

	sprintf(buf, "%d\n", timer_period);
	strcpy(page, buf);
	return strlen(buf);
}

static int write_timer(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	int period;

	sscanf(buffer, "%d", &period);
	if ((period <= 100) && (period>=0))
	{
		timer_period = period;
		TOE2_INFO(KERN_INFO "timer period = 0x%x\n", timer_period*50000);
	}
	return count; 
}

static void init_stb_mac(void)
{
	u32 board_info_addr = (u32)(phys_to_virt)PRE_DEFINED_ADF_BOOT_START;
	ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO*)board_info_addr;
	memcpy(toe2_mac_addr, info->macinfo.phyaddr1, sizeof(u8)*ETH_ALEN);
	return;
}

#ifdef CONFIG_ALI_AS
#define NAND_MAC_OFFSET             0
#define NAND_MAC_LEN                0x100 //encrypt mac address
#define NAND_WHOLE_PARTITION_COUNT  2
#define MAC_EN_MAGIC                (0xdeadbeef)

static int toe2_load_mac(u8 *buf)
{
	struct mtd_info *mtd_handle = NULL;
	u8 *tmp_buff;
	memset(buf, 0x00, NAND_MAC_LEN);
	mtd_handle = get_mtd_device(NULL, NAND_WHOLE_PARTITION_COUNT);
	mtd_handle->get_sys_info(mtd_handle, buf, NAND_MAC_OFFSET, NAND_MAC_LEN);

	if (MAC_EN_MAGIC == *(u32*)(buf+16))
	{
#ifdef CONFIG_ALI_CE
extern int ali_verify_mac_addr(__u8 *mac, __u32 len, __u8 *clear_mac);
		tmp_buff = (u8 *)kmalloc(NAND_MAC_LEN, GFP_KERNEL);
		if (ali_verify_mac_addr(buf, NAND_MAC_LEN, tmp_buff) != 0)
		{
			TOE2_INFO("ali_decrypt_mac_addr error!");
			//hw_watchdog_reboot();
			//asm volatile(".word 0x7000003f; nop; nop;");
		    kfree(tmp_buff);
			return -2;
		}
		else
		{
		    kfree(tmp_buff);
			return 0;
		}
#else
		return 0;
#endif
	}
	else
	{
		return -1;
	}
}
#endif

static int toe2_poweron_sequence(void)
{
    //"Async reset ETH_TOP"
    u32 tmp_val;
    tmp_val = SOC_R32(0x84);
	tmp_val |= 1<<17;
    SOC_W32(0x84, tmp_val);
    
    //"Enable ENET_PLL_PD & ENET_DLL_PD"
    tmp_val = SOC_R32(0x640);
	tmp_val |= 1<<0;
	tmp_val |= 1<<1;
	tmp_val |= 1<<8;
    SOC_W32(0x640, tmp_val);
	udelay(10);
	
    //"Disable ENET_SLEEP"
    tmp_val = SOC_R32(0x640);
	tmp_val &= ~(1<<8);
    SOC_W32(0x640, tmp_val);
	udelay(250);

    //"Disable ENET_PLL_PD"
    tmp_val = SOC_R32(0x640);
	tmp_val &= ~(1<<0);
    SOC_W32(0x640, tmp_val);
	udelay(250);

    //"Disable ENET_DLL_PD"
    tmp_val = SOC_R32(0x640);
	tmp_val &= ~(1<<1);
    SOC_W32(0x640, tmp_val);
	udelay(250);
	
    //"Async reset ETH_TOP"
    tmp_val = SOC_R32(0x84);
	tmp_val &= ~(1<<17);
	SOC_W32(0x84, tmp_val);
	udelay(250);

	//Power On Patch
	//"PLL_PD=1"
	tmp_val = SOC_R32(0x640);
	tmp_val |= 1<<0;
	SOC_W32(0x640, tmp_val);
	udelay(10);

	//"PLL_PD=0"
	tmp_val = SOC_R32(0x640);
	tmp_val &= ~(1<<0);
	SOC_W32(0x640, tmp_val);
	udelay(250);

	//"IP_RST=1"
	tmp_val = SOC_R32(0x84);
	tmp_val |= 1<<17;
	SOC_W32(0x84, tmp_val);
	udelay(10);

	//"IP_RST=0"
	tmp_val = SOC_R32(0x84);
	tmp_val &= ~(1<<17);
	SOC_W32(0x84, tmp_val);
	udelay(10);

	return 0;
}

static int __init toe2_module_init(void)
{
	//toe2_mac_hi16 = random32();
	//toe2_mac_lo32 = random32();
	//4K user data, 288 byte hdcp, 20 byte av info, 8 byte Mac addr
	u8 *read_buff = NULL;
	ptoe2_dev_info mac_dev_info;

#if 0
	3503
	u32 tmp_val = SOC_R32(0x88);
	tmp_val &= ~(1<<9);
	//tmp_val = 0x008010F0;
	SOC_W32(0x88, tmp_val);
	
	tmp_val = SOC_R32(0x8C);
	tmp_val &= ~(1<<24);
	//tmp_val = 0x00800102;
	SOC_W32(0x8C, tmp_val);

    printk("[TOE2] reg88 = 0x%08X, reg8C = 0x%08X, reg430 = 0x%08X, reg434 = 0x%08X\n",
			SOC_R32(0x88), SOC_R32(0x8C), SOC_R32(0x430), SOC_R32(0x434));
#endif

	g_chip_id = (u16) (SOC_R32(0) >> 16);
	if (C3821 == g_chip_id)
	{
    	TOE2_INFO("toe2 toe2_poweron_sequence set!");
		mac_5sec = 6 * mac_clk / TIMER_200_FREQ/*toe2_timer_frq*/;
		toe2_poweron_sequence();
	}
	
	/*mac_dev_info = (ptoe2_dev_info)(__G_ALI_MM_PRIVATE_AREA_START_ADDR - 0x1000 + 288 + 20 + 8);
	TOE2_TRACE("TOE2 userdata: %x, %x, %x\n", mac_dev_info->flag, mac_dev_info->gpio_valid, mac_dev_info->gpio_num);
	if (mac_dev_info->flag == 0x7938b5ed)
	{
		TOE2_TRACE("TOE2: user data mac info valid.\n");
		toe2_physet_gpio_vld = (mac_dev_info->gpio_valid == 0x1)?true:false;
		if (toe2_physet_gpio_vld)
		{
			TOE2_TRACE("TOE2: phy reset gpio valid, gpio = %d.\n", mac_dev_info->gpio_num);
			toe2_physet_gpio = mac_dev_info->gpio_num;
			g_RMII_RESET_PIN = toe2_physet_gpio;
		}
	}*/

	platform_driver_register(&ali_mac_driver);
	
	if ((C3821 == g_chip_id) || (C3921 == g_chip_id))
	{
		init_stb_mac();
	}

	if (toe2_mac_addr[0])
	{
		TOE2_INFO("use boot mac addr.");
		toe2_mac_lo32 = (toe2_mac_addr[3]<<24)|(toe2_mac_addr[2]<<16)|(toe2_mac_addr[1]<<8)|(toe2_mac_addr[0]);
		toe2_mac_lo32 &= 0xFFFFFFFE;
		toe2_mac_hi16 = (toe2_mac_addr[5]<<8)|(toe2_mac_addr[4]);
	}
	else
	{
		TOE2_INFO("invalid mac addr in boot, use default MAC addr!");
#ifdef CONFIG_ALI_AS
#if 0
		not OK............
		read_buff = (u8*)kmalloc(NAND_MAC_LEN, GFP_KERNEL);
		if (0 == toe2_load_mac(read_buff))
		{
			toe2_mac_lo32 = (read_buff[3]<<24)|(read_buff[2]<<16)|(read_buff[1]<<8)|(read_buff[0]<<0); //B31-0
			toe2_mac_hi16 = (read_buff[5]<<8)|(read_buff[4]<<0); //B47-B32
		}
#endif
#endif
	}

	TOE2_INFO("Compiled: %s, %s", __DATE__, __TIME__);
	TOE2_INFO("toe2_base           = 0x%08lX", toe2_base);
	TOE2_INFO("toe2_RMII_RESET     = 0x%uX", g_RMII_RESET_PIN);
	TOE2_INFO("toe2_irq            = %ld", toe2_irq);
	TOE2_INFO("toe2_mac_hi16       = 0x%08lX", toe2_mac_hi16);
	TOE2_INFO("toe2_mac_lo32       = 0x%08lX", toe2_mac_lo32);
	TOE2_INFO("toe2_phy_addr       = %ld", toe2_phy_addr);

	if (toe2_link_mode > 0x7FUL)
	{
		TOE2_INFO("toe2_link_mode      = %ld, Unknown", toe2_link_mode);
		toe2_link_mode = 0x7FUL;
		TOE2_INFO("resetting toe_link_mode to %ld, 10HD,10FD,100HD,100FD,ASYMP,SYMP,ANEG", toe2_link_mode);
	}
	else if (toe2_link_mode == 0UL)
	{
		TOE2_INFO("toe2_link_mode      = %ld, Link Off", toe2_link_mode);
	}
	else 
	{
		TOE2_INFO("toe2_link_mode      = 0x%lX, %s,%s,%s,%s,%s,%s,%s", toe2_link_mode,
				(toe2_link_mode & TOE2_10HD)?"10HD":"",
				(toe2_link_mode & TOE2_10FD)?"10FD":"",
				(toe2_link_mode & TOE2_100HD)?"100HD":"",
				(toe2_link_mode & TOE2_100FD)?"100FD":"",
				(toe2_link_mode & TOE2_RX_PAUSE)?"PAUSE RX":"",
				(toe2_link_mode & TOE2_TX_PAUSE)?"PAUSE_TX":"",
				(toe2_link_mode & TOE2_AUTONEG)?"AUTONEG":"");
	}

	if (toe2_rx_mode == TOE2_RX_MODE_TASKLET)
	{
		TOE2_INFO("toe2_rx_mode        = 0x%08lX, Tasklets enabled", toe2_rx_mode);
	} 
	else if (toe2_rx_mode == TOE2_RX_MODE_NAPI) //only 2.6 supports
	{
		TOE2_INFO("toe2_rx_mode        = 0x%08lX, NAPI enabled", toe2_rx_mode);
	}
	else
		TOE2_INFO("toe2_rx_mode        = 0, use ISR");

	TOE2_INFO("toe2_rmii       = %s", (toe2_rmii == true)? "true":"false");
	TOE2_INFO("toe2_sg         = %s", (toe2_sg == true)? "true":"false");	
	TOE2_INFO("toe2_tx_csum    = %s", (toe2_tx_csum == true)? "true":"false");
	TOE2_INFO("toe2_rx_csum    = %s", (toe2_rx_csum == true)? "true":"false");
	TOE2_INFO("toe2_tso        = %s", (toe2_tso== true)? "true":"false");
	TOE2_INFO("toe2_ufo        = %s", (toe2_ufo== true)? "true":"false");

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0))
	/* create a directory */
	network_dir = proc_mkdir("network", NULL);
	if (network_dir == NULL)
	{
		printk("create /proc/network fail.\n");
		goto ERR;
	}

	/* create a file */
	timer_file = create_proc_entry("timer", 0644, network_dir);
	if (timer_file == NULL)
	{
		remove_proc_entry("network", NULL);
		printk("create /proc/network/timer fail.\n");
		goto ERR;
	}

	timer_file->read_proc = read_timer;
	timer_file->write_proc = write_timer;

ERR:
#endif

	return toe2_attach();
}

static void toe2_module_exit(void)
{
	struct net_device *dev = NULL;
	platform_driver_unregister(&ali_mac_driver);

	TOE2_INFO("toe2_module_exit()=>in.");
	if (g_ptoe2 != NULL)
	{
		dev = g_ptoe2->dev;
		mac_free_rings(g_ptoe2);

		if (g_ptoe2->rx_desc != NULL)
		{
			dma_free_coherent((struct device *)NULL, (TOE2_DESC_SZ*TOE2_RX_DESC_NUM), 
					g_ptoe2->rx_desc, g_ptoe2->rx_desc_dma);
			g_ptoe2->rx_desc = NULL;
		}

		if (g_ptoe2->tx_desc != NULL)
		{
			dma_free_coherent((struct device *)NULL, (TOE2_DESC_SZ*TOE2_TX_DESC_NUM), 
					g_ptoe2->tx_desc, g_ptoe2->tx_desc_dma);
			g_ptoe2->tx_desc = NULL;
		}

		if (g_ptoe2->setup_buf != NULL)
		{
			dma_free_coherent((struct device *)NULL, SETUP_FRAME_SZ, 
					g_ptoe2->setup_buf, g_ptoe2->setup_buf_dma);
			g_ptoe2->setup_buf = NULL;
		}

		//kfree(g_ptoe2);
		g_ptoe2 = NULL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0))
	remove_proc_entry("timer", network_dir);
	remove_proc_entry("network", NULL);
#endif

	unregister_netdev(dev);
	free_netdev(dev); //need to chk ag.

	TOE2_INFO("toe2_module_exit()=>out.");

#ifdef UTILITY_SUPPORT
    util_mac_rx_thread_destroy();
    util_mac_tx_thread_destroy();
#endif
}

#if 0
static void __init parse_mac_addr(char *macstr)
{
	int i, j;
	unsigned char result, value;

	if (strlen(macstr)<12+5)//12 digit + 5 ':'
		return;

	for (i = 0; i < 6; i++) 
	{
		if (i != 5 && *(macstr + 2) != ':')
			return;

		result = 0;	

		for (j = 0; j < 2; j++) 
		{
			if (isxdigit(*macstr) && (value = isdigit(*macstr) ? *macstr - '0' : toupper(*macstr) - 'A' + 10) < 16) 
			{
				result = result * 16 + value;
				macstr++;
			}
			else
				return;
		}

		macstr++;
		toe2_mac_addr[i] = result;
	}
	toe2_mac_addr[i] = 1;
}

static int __init program_setup_kmac(char *s)
{
	if(s)
	{
		TOE2_TRACE("cmdline: mac = %s\n", s);
		parse_mac_addr(s);
	}
	return 0;
}

__setup("mac=", program_setup_kmac);
#endif

module_init(toe2_module_init);
//late_initcall(toe2_module_init);
module_exit(toe2_module_exit);

#ifdef TOE2_NAPI_DEBUG
MODULE_LICENSE("GPL");
#else
MODULE_LICENSE("proprietary");
#endif

