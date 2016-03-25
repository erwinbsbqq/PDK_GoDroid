/*
 *	Linux ALi switch
 *
 *	Authors:
 *	peter.li <gilbertjuly@gmail.com>/<peter.li@alitech.com>
 *	
 *	Goddamnit, i can't believe this file alone has more than 5000 lines!
 *
 */

/***************************************************************************
Design considerion
	besides the sw_p0 structure, the protocal impliment should have a ali_sw struture,
		which is to control the whole switch, not only port0's tx and rx, which is sw_p0 does.
	therefore, I think the file name rule should be like:
		ali_sw.c -------------- swith management (p0~p3)
					|------- p0's tx/rx.
					|------- handle skb to appropriate protocal handler.
					|------- implement of sw_tool (ioctl).
		ali_sw_stp.c----------- stp protocal handler
		ali_sw_mc_snooping.c--- multicast snooping handler (igmp, mld)

		ali_sw_p0.h--------- descriptors definition used for TX/RX
					|------- after all, it's a ethernet MAC device, although it doesn' have a MAC in reality.
					|------- giving it this name also because it operates like a MAC.
		ali_sw_reg.h----------- switch register definition
		ali_sw.h -------------- data structure
	the function name rule is like:
		p0_XXX -------------- p0 (port0/MAC)'s TX/RX operation
		ali_sw_XXX ----------- switch related functions.

	Add CONFIG_XXX
		CONFIG_ALI_SWITCH
			CONFIG_ALI_STP
			CONFIG_ALI_MC_SNOOPING
			CONFIG_ALI_PROC -- undone.


Bear in your mind, the data structure is like this:
	data struct connection:
		net_device <--> p0_private <--> net_ali_sw
										|-----net_ali_sw_port[0] --> p0_private
										|-----net_ali_sw_port[1] --> p0_private
										|-----net_ali_sw_port[2] --> p0_private
										|-----net_ali_sw_port[3] --> p0_private							
	data struct in memory:
		+----------+
		| net_device   |
		+----------+
		| p0_private   |-------
		+----------+           |           +--------------+
						     ------>| net_ali_sw          |
						                  +--------------+
						                  |net_ali_sw_port*4|
						                  +--------------+
	Yes, I know it's a large structure.				                  
	as diagram shows, to access each net_ali_sw_port's data, must get 'net_ali_sw' first.
****************************************************************************/

#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/mii.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/crc32.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <asm/irq.h>
#include <asm/dma.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/mach-ali/m6303.h>
#include <linux/if_vlan.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)) /*zmiao*/
#include <asm-generic/dma-coherent.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#else 
#include <dma-coherent.h>
#endif

#ifdef ALI_SW_PROC
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif

#include "ali_sw.h"		
#include "ali_sw_reg.h"	
#include "ali_sw_p0.h"	
#include <ali_reg.h>
#include <ali_dma.h>
#include <alidefinition/adf_boot.h>


UINT32 ali_sw_base = _ALI_SW_BASE;
static UINT32 ali_sw_irq = _ALI_SW_IRQ;
UINT32 use_default_mac = 1;
//------------------------------------------------------------------------------------
/* PHY and MAC setting*/

//#define DISABLE_SW_MDIO_PERIOD

#if 1
u8 stb_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x03};
//const u8 stb_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x03};
u8 p0_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x0a};
u8 p1_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x0b};
u8 p2_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x0c};
u8 p3_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x0d};
#else
//0x9c
u8 stb_mac_addr[ETH_ALEN] = { 0x9c, 0x90, 0xe6, 0x00,  0x00, 0x01};
u8 p0_mac_addr[ETH_ALEN] = { 0x9c, 0x90, 0xe6, 0x00,  0x00, 0x0a};
u8 p1_mac_addr[ETH_ALEN] = { 0x9c, 0x90, 0xe6, 0x00,  0x00, 0x0b};
u8 p2_mac_addr[ETH_ALEN] = { 0x9c, 0x90, 0xe6, 0x00,  0x00, 0x0c};
u8 p3_mac_addr[ETH_ALEN] = { 0x9c, 0x90, 0xe6, 0x00,  0x00, 0x0d};
/*
const u8 stb_mac_addr[ETH_ALEN] = { 0x0a, 0x00, 0x00, 0xe6,  0x90, 0x0f};
const u8 p0_mac_addr[ETH_ALEN] = { 0x0a, 0x00, 0x00, 0xe6,  0x90, 0x00};
const u8 p1_mac_addr[ETH_ALEN] = { 0x0a, 0x00, 0x00, 0xe6,  0x90, 0x01};
const u8 p2_mac_addr[ETH_ALEN] = { 0x0a, 0x00, 0x00, 0xe6,  0x90, 0x02};
const u8 p3_mac_addr[ETH_ALEN] = { 0x0a, 0x00, 0x00, 0xe6,  0x90, 0x03};
*/
#endif


static u8* port_mac_addr[MAX_PORT_NUM] = 
	{p0_mac_addr, p1_mac_addr, p2_mac_addr, p3_mac_addr};

const u8 brdcast_mac_addr[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

void print_icmp_stats(void);
//	to make less global varible, 
//	p123 stands for p0, p1, p2, p3,
//	'p' means port
//static UINT32 p123_phy_addr = ALI_SW_PHY_ADDR;
//#define HW_AUTO_MDIO_POLLING

#define P0_PHY_ADDRESS 32

#define NEW_PHY_TEST

#ifdef NEW_PHY_TEST
#define P1_PHY_ADDRESS 0
#define P2_PHY_ADDRESS 2
#define P3_PHY_ADDRESS 3
static u8 p123_phy_mode[MAX_PORT_NUM] = 
	{0xff, ETH_PHY_MODE_RGMII, ETH_PHY_MODE_RMII, ETH_PHY_MODE_RMII};

#else
#define P1_PHY_ADDRESS 0
#define P2_PHY_ADDRESS 2
#define P3_PHY_ADDRESS 3
static u8 p123_phy_mode[MAX_PORT_NUM] = 
	{0xff, ETH_PHY_MODE_RMII, ETH_PHY_MODE_RMII, ETH_PHY_MODE_RMII};

#endif

static u8 port_phy_addr[MAX_PORT_NUM] = 
	{P0_PHY_ADDRESS, P1_PHY_ADDRESS, P2_PHY_ADDRESS, P3_PHY_ADDRESS};

//static UINT32 p123_link_mode = 0x7FUL;
//static UINT32 p0123_rx_filter = ALI_SW_RX_FILTER_PERFECT;
static UINT32 p0_rx_mode = ALI_SW_RX_MODE_TASKLET;

#ifdef LOCK_DEBUG
#define CMD_LOCK(lock)  {printk("%s try to get cmd_lock %x\n", __FUNCTION__, (UINT32)(lock)); spin_lock(lock);printk("%s get cmd lock %x\n", __FUNCTION__, (UINT32)(lock));}
#define CMD_UNLOCK(lock)  {spin_unlock(lock); printk("%s unlock cmd_lock %x\n", __FUNCTION__, (UINT32)(lock));}
#define CMD_LOCK_IRQ(lock, flag)  \
                 {printk("%s try to get irq lock %x\n", __FUNCTION__, (UINT32)(lock)); spin_lock_irqsave(lock,flag);printk("%s get irq lock %x\n", __FUNCTION__, (UINT32)(lock));}
#define CMD_UNLOCK_IRQ(lock, flag) \
				 {spin_unlock_irqrestore(lock, flag);printk("%s unlock irq lock %x\n",__FUNCTION__, (UINT32)(lock));}
#else
#define CMD_LOCK(lock)  {spin_lock(lock);}
#define CMD_UNLOCK(lock)  {spin_unlock(lock);}
#define CMD_LOCK_IRQ(lock, flag)  \
                 {spin_lock_irqsave(lock,flag);}
#define CMD_UNLOCK_IRQ(lock, flag) \
				 {spin_unlock_irqrestore(lock, flag);}
#endif 
//------------------------------------------------------------------------------------
static UINT32 p0_timer_frq = 0x06666666;
//------------------------------------------------------------------------------------
/* hardware accelerate */
#if 1
static int p0_rx_csum = true;
static int p0_tx_csum = true;
static int p0_sg = true;
static int p0_tso = true;
static int p0_ufo = true;
#else
//	false -- software is taking care of it.
//	true -- hardware is taking care of it.
static int p0_rx_csum = false;
static int p0_tx_csum = false;
static int p0_sg = false;
static int p0_tso = false;
static int p0_ufo = false;
#endif


static int ali_sw_debug_level = -1;
u8	test_forwarding_ports = 0;
	
#define SKB_IP_ALIGN	2
//#define P0_MC_FILTER_MODE_IS_PERFECT
//#define P0_MC_FILTER_MODE_IS_ALLMULTI

#define MAX_PKT_LEN_DWORD (1536/4-1)
#define MIN_PKT_LEN_DWORD (64/4-2)//-1 is crc, -2 is crc+vlan tag

u8 tmp_flag = 0;
struct timer_list	sw_test_timer;	
struct p0_private *gp_p0 = NULL;
struct net_ali_sw *gp_sw = NULL;

unsigned long dbg_runtime_val;

static void p0_tasklet_fn(unsigned long para);

DECLARE_TASKLET(p0_tasklet, p0_tasklet_fn, 0);

void ali_sw_irq_enable(int lock);
void ali_sw_irq_disable(int lock);

/* _xxxx mean no spin lock */
static struct net_device_stats *_ali_sw_get_stats(struct net_device *dev);

/* use new phy check flow by Sean , after full test, 
 * old phy check flow will be deleted */
void _new_ali_sw_super_housekeeper(p_p0_private priv);
/* xxx mean have spin irq lock */
static struct net_device_stats * ali_sw_get_stats(struct net_device *dev);

static bool p0123_port_enable(u8 port_no);
static int p123_phy_test(p_p0_private priv);
//static void dump_all_tx_desc_verbose(void);
static void dump_tx_desc_verbose(u8 idx) ;
void dump_sw_tx_skb(p_p0_private priv, int index) ;
void dump_all_sw_tx_skb(p_p0_private priv) ;
void verify_sw_tx_skb(p_p0_private priv);
#ifdef ALI_SW_DEBUG1
void p0_print(void * buf, unsigned short len)
{
	int i = 0;
    u8 * p = NULL;
    p = (u8*) buf;
	for(i=0; i<len; i++)
	{
		if (i%16 == 0)
		{
			printk("0x%08x:  ", (UINT32)(p+i));
		}

		printk("%02x ", *(p+i));
	}
	printk("\n");
}
#else
static void p0_print(void *buf, unsigned short len)
{
	int i = 0;
    u8 * p = NULL;
    p = (u8*)buf;
	for(i=0; i<len; i++)
	{
		if (i%16 == 0)
		{
			printk("\n0x%08x:  ", (UINT32)(p+i));
		}

		printk("%02x ", *(p+i));
	}
	printk("\n");
}
#endif

void p0_print_line(unsigned char *p, unsigned short len)
{
	int i;
	for(i=0; i<len; i++)
	{
		if (i%16 == 0)
		{
			printk("\n0x%08x:  ", (UINT32)(p+i));
		}

		printk("%02x ", *(p+i));
	}
	printk("\n");
}
void verify_sw_tx_skb(p_p0_private priv) {
	u32 cnt;
	u16 start, next_start;
	u16 tx_skb_wr, tx_skb_rd, next_tx_skb_rd;
	tx_skb_rd = priv->tx_skb_rd;
	tx_skb_wr = priv->tx_skb_wr;
	cnt = 0;
	while (tx_skb_rd != tx_skb_wr) {
		start = priv->sw_tx_skb[tx_skb_rd].start;	
		if (((tx_skb_rd + 1) % ALI_SW_TX_DESC_NUM) != tx_skb_wr) {
			next_start = (start + priv->sw_tx_skb[tx_skb_rd].cnt) % ALI_SW_TX_DESC_NUM;
			next_tx_skb_rd = (tx_skb_rd + 1) % ALI_SW_TX_DESC_NUM;
			if(priv->sw_tx_skb[next_tx_skb_rd].start != next_start) {
				sw_error("ERROR: sw_tx_skb mis order rd %d wr %d next_rd %d start %d next_start %d\n",\
						tx_skb_rd, tx_skb_wr, next_tx_skb_rd, start, next_start);
				dump_all_sw_tx_skb(priv);
			}
		}
		cnt += priv->sw_tx_skb[tx_skb_rd].cnt;
		if (cnt > (ALI_SW_TX_DESC_NUM - 1)) {
			sw_error("ERROR: sw_tx_skb oversize \n");
			dump_all_sw_tx_skb(priv);
		}
        ++tx_skb_rd;
		tx_skb_rd =  tx_skb_rd % ALI_SW_TX_DESC_NUM; 
	}
	return;
}
static void p0_free_tx_skb(p_p0_private priv, int lock) {
	struct sk_buff *skb;
	p_tx_desc desc;
	unsigned long flags;
	u16 tx_rptr, tx_wptr;
	u16 start, first;
	u32 i = 0;
	u16 desc_num;

	if (test_bit(3, &dbg_runtime_val)) {
		trace_tx_l1("async free skb and unmap dma tx_skb_wr %d tx_skb_rd %d\n", priv->tx_skb_wr, priv->tx_skb_rd);
	}
	if (lock) {	
		CMD_LOCK_IRQ(&priv->p_switch->lock, flags);
	}

	CMD_LOCK(&priv->cmd_lock); /* called in napi poll, it disable intr, so doesn't need to lock cmd_lock */
	SW_W8(SysPortAccess, 0);
	tx_wptr = SW_R16(TxRingDesWPtr);
	tx_rptr = SW_R16(TxRingDesRPtr);
	CMD_UNLOCK(&priv->cmd_lock); /* called in napi poll, it disable intr, so doesn't need to lock cmd_lock */

	if (test_bit(3, &dbg_runtime_val)) {
		trace_tx_l1 ("p0_free_tx_skb get switch lock tx_rptr %d tx_wptr %d\n",\
			tx_rptr, tx_wptr);
	}
#if 0
	if(netif_queue_stopped(priv->dev)) {
		dump_all_sw_tx_skb(priv);
	}
#endif
	while (priv->tx_skb_wr != priv->tx_skb_rd) {
#if 0
        if(priv->tx_skb_wr >= ALI_SW_TX_DESC_NUM || \
           priv->tx_skb_rd >= ALI_SW_TX_DESC_NUM) {
            sw_error("%s oversize priv->tx_skb_wr %d priv->tx_skb_rd %d\n", priv->tx_skb_wr, priv->tx_skb_rd);
        }
#endif
		start = priv->sw_tx_skb[priv->tx_skb_rd].start;
		first = priv->sw_tx_skb[priv->tx_skb_rd].first;
		desc_num = priv->sw_tx_skb[priv->tx_skb_rd].cnt;
		
		if ((start < tx_rptr) && ((tx_rptr - start) >= desc_num)) {
			goto free_skb_dma;
		}
		if ((start > tx_rptr) && (priv->tx_wptr >= tx_rptr) && (start > tx_wptr) &&
			((ALI_SW_TX_DESC_NUM - (start - tx_rptr)) >= desc_num)) {
			goto free_skb_dma;
		}
		/* no tx desc will be free */
		if (test_bit(3, &dbg_runtime_val)) {
			trace_tx_l1("p0_free_tx_skb no tx desc will be free!!\n"); 
			trace_tx_l1("tx_skb_rd %d tx_skb_wr %d\n",priv->tx_skb_rd, priv->tx_skb_wr); 
			trace_tx_l1("start %d first %d desc_num %d\n",start, first, desc_num); 
			trace_tx_l1("priv->tx_wptr %d \n",priv->tx_wptr); 
		}
		if (test_bit(4, &dbg_runtime_val)) {
			dump_all_sw_tx_skb(priv);
		}
		break;

free_skb_dma:
		if (test_bit(3, &dbg_runtime_val)) {
			trace_tx_l1 ("free_dma_skb start %d tx_rptr %d priv->tx_wptr %d\n",
				start, tx_rptr, priv->tx_wptr);
		}

		if (!priv->sw_tx_skb[priv->tx_skb_rd].skb) {
			sw_error("%s: skb is null\n", __FUNCTION__);
			priv->sw_tx_skb[priv->tx_skb_rd].skb = NULL;
			priv->sw_tx_skb[priv->tx_skb_rd].first = 0;
			priv->sw_tx_skb[priv->tx_skb_rd].cnt = 0;
			priv->sw_tx_skb[priv->tx_skb_rd].start = 0;
			priv->tx_skb_rd++;
			if (priv->tx_skb_rd == ALI_SW_TX_DESC_NUM) {
				priv->tx_skb_rd = 0;
			}
			continue;
		}
		
		skb = priv->sw_tx_skb[priv->tx_skb_rd].skb;
		priv->dev->stats.tx_packets++;
		priv->dev->stats.tx_bytes += (skb->len);
		
		desc = &priv->tx_desc[start];
		while(desc_num > 0) {
			i++;
			if(desc->DataDescriptor.seg_len> 0) {
				dma_unmap_single((struct device *)NULL, __DTCADDRALI(desc->DataDescriptor.pkt_buf_dma), \
													desc->DataDescriptor.seg_len, DMA_TO_DEVICE);
				desc->DataDescriptor.seg_len = 0;
			}
			
			if(++start >= ALI_SW_TX_DESC_NUM) {
				start = 0;
			}
			desc = &priv->tx_desc[start];
			desc_num--;
		}

		dev_kfree_skb(priv->sw_tx_skb[priv->tx_skb_rd].skb);
		priv->sw_tx_skb[priv->tx_skb_rd].skb = NULL;
		priv->sw_tx_skb[priv->tx_skb_rd].first = 0;
		priv->sw_tx_skb[priv->tx_skb_rd].cnt = 0;
		priv->sw_tx_skb[priv->tx_skb_rd].start = 0;
		priv->tx_skb_rd++;
		if (priv->tx_skb_rd == ALI_SW_TX_DESC_NUM) {
			priv->tx_skb_rd = 0;
		}
	}
	priv->avail_desc_num += i;
	if (priv->avail_desc_num > (ALI_SW_TX_DESC_NUM-1)) {
		sw_error("error avail_desc_num > %d\n", priv->avail_desc_num);
		dump_all_sw_tx_skb(priv);
	}
	if(test_bit(5, &dbg_runtime_val)) {
		verify_sw_tx_skb(priv);
	}
	if (lock) {	
		CMD_UNLOCK_IRQ(&priv->p_switch->lock,flags);
	}
	if((i > 0) && netif_queue_stopped(priv->dev)) {
		sw_trace("netif wake up again i %d tx_skb_rd %d tx_skb_wr %d avail_desc_num %d\n", \
				i , priv->tx_skb_rd, priv->tx_skb_wr, priv->avail_desc_num);
		//dump_all_sw_tx_skb(priv);
		//netif_wake_queue(priv->dev);
	}
	ali_trace(3, "p0_free_tx_skb unlock\n");
	return ;
}

#if 0
static void ali_sw_stp_rcv_handler(struct work_struct *work)
{
	struct p0_private *priv =
		container_of(work, struct p0_private, stp_pkt_work);

	struct sk_buff *skb;
	//printk(KERN_INFO "enter ali_sw_stp_rcv_handler\n");
	
	if(!skb_queue_empty(&priv->stp_pkt_queue))
		skb = skb_dequeue(&priv->stp_pkt_queue);//question here.
	else
		return;
	
	ali_sw_stp_rcv(skb);
}

static void ali_sw_mc_rcv_handler(struct work_struct *work)
{
	struct p0_private *priv =
		container_of(work, struct p0_private, mcs_pkt_work);

	struct sk_buff *skb;
	//printk(KERN_INFO "enter ali_sw_stp_rcv_handler\n");

	if(!skb_queue_empty(&priv->mcs_pkt_queue))
		skb = skb_dequeue(&priv->mcs_pkt_queue);//question here.
	else{
		return;
	}
	
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb->mac_len = skb->network_header - skb->mac_header;
	
	//p0_print_line(eth_hdr(skb)->h_dest, skb->len );
	
	ali_sw_multicast_rcv(skb);

	return;
}

#endif

void ali_sw_stp_enable(void)
{
	SW_W8(SysPortAccess, 4);
	SW_W32(PktDecCtl, SW_R32(PktDecCtl)|StpEn);
	gp_sw->stp_enabled = ALI_SW_KERNEL_STP;
	gp_sw->stp_is_working = ALI_SW_STP_WORKING;
	
	ali_sw_stp_init();
	
	sw_info("ALi sw: STP enabled!");
}

void ali_sw_stp_disable(void)
{
	SW_W8(SysPortAccess, 4);
	SW_W32(PktDecCtl, SW_R32(PktDecCtl)&(~StpEn));
	
	ali_sw_stop_sw_stp(gp_sw);
	gp_sw->stp_enabled = ALI_SW_NO_STP;	
	gp_sw->stp_is_working = ALI_SW_STP_NOT_WORKING;
	
	sw_info("ALi sw: STP disabled!");
}

void ali_sw_mc_snooping_enable(void)
{
	SW_W8(SysPortAccess, 4);
	SW_W32(PktDecCtl, SW_R32(PktDecCtl)|IgmpMldEn);
	ali_sw_mc_snooping_init();
	//printk("ALI_SW: MCS enabled\n");	
}

void ali_sw_mc_snooping_disable(void)
{
	SW_W8(SysPortAccess, 4);
	SW_W32(PktDecCtl, SW_R32(PktDecCtl)&(~IgmpMldEn));
	//TBD, needs to call protocal ending.
	printk("ALI_SW: MCS disabled\n");		
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ali_sw_acl_read_entry(pacl_info p_acl_val)
{
	u32 reg_val=0;
	
	if(p_acl_val->acl_idx>0xf)
		sw_error("unsupported: %d, %s", __LINE__, __FUNCTION__);

	SW_W8(SysPortAccess, 4);
	reg_val=SW_R32(ACLCtl);
	reg_val&=(ACL_MODE_MSK<<ACL_MODE_OFFSET);
	reg_val|=(p_acl_val->acl_idx)<<ACL_ADDR_OFFSET;
	reg_val&=(~ACL_RW);
	reg_val|=ACL_REQ;
	SW_W8(SysPortAccess, 4);
	SW_W32(ACLCtl, reg_val);	
	//printk("reg[ACLCtl]=0x%x	", SW_R32(ACLCtl));
	while(SW_R32(ACLCtl)&ACL_REQ)
	{;}
	mdelay(1);
	
	reg_val=SW_R32(ACLCtl);
	//printk("reg[ACLCtl]=0x%x	", SW_R32(ACLCtl));
	p_acl_val->acl_mode = reg_val&(ACL_MODE_MSK<<ACL_MODE_OFFSET);

	reg_val=SW_R32(ACLRdata1);
	//printk("reg[ACLRdata1]=0x%x	", SW_R32(ACLRdata1));
	p_acl_val->priority_sel = reg_val&ACL_PRIORITY_MSK;
	p_acl_val->port2_end = (reg_val>>ACL_DEST_PORT_END_16B_OFFSET)&ACL_IP_PORT_MSK;
	p_acl_val->port2_start = (reg_val>>ACL_DEST_PORT_START_F12B_OFFSET)&0xfff;

	reg_val=SW_R32(ACLRdata2);
	p_acl_val->port2_start |= (reg_val&0xf)<<12;
	p_acl_val->port1_end = (reg_val>>ACL_SRC_PORT_END_16B_OFFSET)&ACL_IP_PORT_MSK;
	p_acl_val->port1_start = (reg_val>>ACL_SRC_PORT_START_F12B_OFFSET)&0xfff;

	reg_val=SW_R32(ACLRdata3);
	p_acl_val->port1_start |= (reg_val&0xf)<<12;
	p_acl_val->acl_proto = (reg_val>>ACL_TCP_UDP_OFFSET)&ACL_TCP_UDP_MSK;
	p_acl_val->ports_enable =  (reg_val>>ACL_PORTS_OFFSET)&ACL_PORTS_MSK;
	p_acl_val->ip2 =  (reg_val>>ACL_IP2_ADDR_F22B_OFFSET)&0x3fffff;

	reg_val=SW_R32(ACLRdata4);
	p_acl_val->ip2 |=  (reg_val&0x3ff)<<22;
	p_acl_val->ip1 =  (reg_val>>ACL_IP1_ADDR_F22B_OFFSET)&0x3fffff;

	reg_val=SW_R32(ACLRdata5);
	//printk("reg[ACLRdata5]=0x%x	\n", SW_R32(ACLRdata5));
	p_acl_val->ip1|=(reg_val&0x3ff)<<22;

}

void ali_sw_acl_show_single(pacl_info p_acl_val)
{
	u8 tmp = 0;

	printk("\nACL table display, mode:0x%x\n", p_acl_val->acl_mode);
	printk("No		ip1		ip2	   	  ports  proto     ip1_port       ip2_port   prior\n");
	
	ali_sw_acl_read_entry(p_acl_val);
	printk("%2d    ", p_acl_val->acl_idx);
	
	tmp = (p_acl_val->ip1>>24)&0xff;
	printk("%3d.",tmp);
	tmp = (p_acl_val->ip1>>16)&0xff;
	printk("%3d.",tmp);
	tmp = (p_acl_val->ip1>>8)&0xff;
	printk("%3d.",tmp);
	tmp = (p_acl_val->ip1)&0xff;
	printk("%3d	   ",tmp);

	tmp = (p_acl_val->ip2>>24)&0xff;
	printk("%3d.",tmp);
	tmp = (p_acl_val->ip2>>16)&0xff;
	printk("%3d.",tmp);
	tmp = (p_acl_val->ip2>>8)&0xff;
	printk("%3d.",tmp);
	tmp = (p_acl_val->ip2)&0xff;
	printk("%3d	   ",tmp);

	printk("%c", p_acl_val->ports_enable&PORT3_EN?'1':'0');
	printk("%c", p_acl_val->ports_enable&PORT2_EN?'1':'0');
	printk("%c", p_acl_val->ports_enable&PORT1_EN?'1':'0');
	printk("%c  ", p_acl_val->ports_enable&PORT0_EN?'1':'0');
	
	//printk("%d	 ", p_acl_val->acl_proto);
	if(p_acl_val->acl_proto==1)
		printk("tcp    ");
	else if(p_acl_val->acl_proto==2)
		printk("udp    ");
	else if(p_acl_val->acl_proto==3)
		printk("both   ");
	else
		printk("none   ");
	
	printk("%5d~%5d   ",p_acl_val->port1_start, p_acl_val->port1_end);
	printk("%5d~%5d	",p_acl_val->port2_start, p_acl_val->port2_end);

	
	printk("%d\n", p_acl_val->priority_sel);
	mdelay(1);
}

void ali_sw_acl_show_all(pacl_info p_acl_val)
{
	u8 idx =0;
	u8 tmp = 0;

	printk("\nACL table display, mode:0x%x\n", p_acl_val->acl_mode);
	printk("No		ip1		ip2	   	  ports  proto     ip1_port       ip2_port   prior\n");
	for(idx=0; idx<16; idx++){
		p_acl_val->acl_idx=idx;
		ali_sw_acl_read_entry(p_acl_val);
		printk("%2d    ", p_acl_val->acl_idx);
		
		tmp = (p_acl_val->ip1>>24)&0xff;
		printk("%3d.",tmp);
		tmp = (p_acl_val->ip1>>16)&0xff;
		printk("%3d.",tmp);
		tmp = (p_acl_val->ip1>>8)&0xff;
		printk("%3d.",tmp);
		tmp = (p_acl_val->ip1)&0xff;
		printk("%3d	   ",tmp);

		tmp = (p_acl_val->ip2>>24)&0xff;
		printk("%3d.",tmp);
		tmp = (p_acl_val->ip2>>16)&0xff;
		printk("%3d.",tmp);
		tmp = (p_acl_val->ip2>>8)&0xff;
		printk("%3d.",tmp);
		tmp = (p_acl_val->ip2)&0xff;
		printk("%3d	   ",tmp);

		printk("%c", p_acl_val->ports_enable&PORT3_EN?'1':'0');
		printk("%c", p_acl_val->ports_enable&PORT2_EN?'1':'0');
		printk("%c", p_acl_val->ports_enable&PORT1_EN?'1':'0');
		printk("%c  ", p_acl_val->ports_enable&PORT0_EN?'1':'0');
		
		//printk("%d	 ", p_acl_val->acl_proto);
		if(p_acl_val->acl_proto==1)
			printk("tcp    ");
		else if(p_acl_val->acl_proto==2)
			printk("udp    ");
		else if(p_acl_val->acl_proto==3)
			printk("both   ");
		else
			printk("none   ");
		
		printk("%5d~%5d   ",p_acl_val->port1_start, p_acl_val->port1_end);
		printk("%5d~%5d	",p_acl_val->port2_start, p_acl_val->port2_end);

		
		printk("%d\n", p_acl_val->priority_sel);
		mdelay(1);
		
	}
		
}

void ali_sw_acl_write_entry(pacl_info p_acl_val)
{
	u32 reg_val=0;
	SW_W8(SysPortAccess, 4);
	reg_val = p_acl_val->priority_sel;
	reg_val|=(p_acl_val->port2_end)<<ACL_DEST_PORT_END_16B_OFFSET;
	reg_val|=(p_acl_val->port2_start)<<ACL_DEST_PORT_START_F12B_OFFSET;
	SW_W32(ACLWdata1, reg_val);

	reg_val=(p_acl_val->port2_start)>>12;
	reg_val|=(p_acl_val->port1_end)<<ACL_SRC_PORT_END_16B_OFFSET;
	reg_val|=(p_acl_val->port1_start)<<ACL_SRC_PORT_START_F12B_OFFSET;
	SW_W32(ACLWdata2, reg_val);

	reg_val=(p_acl_val->port1_start)>>12;
	reg_val|=(p_acl_val->acl_proto)<<ACL_TCP_UDP_OFFSET;
	reg_val|=(p_acl_val->ports_enable)<<ACL_PORTS_OFFSET;
	reg_val|=(p_acl_val->ip2)<<ACL_IP2_ADDR_F22B_OFFSET;
	SW_W32(ACLWdata3, reg_val);

	reg_val=(p_acl_val->ip2)>>22;
	reg_val|=(p_acl_val->ip1)<<ACL_IP1_ADDR_F22B_OFFSET;
	SW_W32(ACLWdata4, reg_val);

	reg_val=(p_acl_val->ip1)>>22;
	SW_W32(ACLWdata5, reg_val);

	reg_val=(p_acl_val->acl_mode)<<ACL_MODE_OFFSET;
	//sw_trace("0x%x", reg_val);
	reg_val|=(p_acl_val->acl_idx)<<ACL_ADDR_OFFSET;
	//sw_trace("0x%x", reg_val);
	reg_val|=ACL_RW;
	//sw_trace("0x%x", reg_val);
	reg_val|=ACL_REQ;
	//sw_trace("0x%x", reg_val);
	SW_W32(ACLCtl, reg_val);
	//sw_trace("0x%x", reg_val);

	sw_trace("0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", SW_R32(ACLCtl), SW_R32(ACLWdata1), SW_R32(ACLWdata2), \
												SW_R32(ACLWdata3), SW_R32(ACLWdata4), SW_R32(ACLWdata5));
	while(SW_R32(ACLCtl)&ACL_REQ)
	{;}
	
}


void ali_sw_band_rate_show(pbandrate_info p_br_val)
{
	if(p_br_val->port_no>MAX_PORT_NUM)
		sw_error("p_br_val wrong: port_no=%d, interval=%d", p_br_val->port_no, p_br_val->interval);

	SW_W8(SysPortAccess, 4);
	SW_W16(BR_TIME_INTERVAL, p_br_val->interval);

	printk("\nbandrate statistic show	 (bytes)\n");
	printk("port			TX		RX:\n");
	printk("0.		%8d		%8d\n", SW_R32(P0_TX_BANDRATE), SW_R32(P0_RX_BANDRATE));
	printk("1.		%8d		%8d\n", SW_R32(P1_TX_BANDRATE), SW_R32(P1_RX_BANDRATE));
	printk("2.		%8d		%8d\n", SW_R32(P2_TX_BANDRATE), SW_R32(P2_RX_BANDRATE));
	printk("3.		%8d		%8d\n", SW_R32(P3_TX_BANDRATE), SW_R32(P3_RX_BANDRATE));
	printk("				statistic time interval: %d (ms)\n\n", SW_R16(BR_TIME_INTERVAL));
	
}

void ali_sw_show_port_link(void)
{
	struct net_ali_sw_port *p;
	u8 port_no;

	for(port_no=1; port_no<MAX_PORT_NUM; port_no++){
		p = &(gp_sw->port_list[port_no]);
		if(p==NULL)
			return;
		
		sw_info("\nport[%d] link status:", port_no);

		sw_info("	phy address: %d", port_phy_addr[port_no]);

		if(p123_phy_mode[port_no]==ETH_PHY_MODE_RGMII)
			sw_info("	link type: rgmii");
		else if(p123_phy_mode[port_no]==ETH_PHY_MODE_RMII)
			sw_info("	link type: rmii");
		else
			sw_info("	link type: unknown");

		if(p->transmit_okay){
			if(p->link_spd==LINK_SPEED_10M)
				sw_info("	speed: 10M");
			else if(p->link_spd==LINK_SPEED_100M)
				sw_info("	speed: 100M");
			else if(p->link_spd==LINK_SPEED_1000M)
				sw_info("	speed: 1000M");
			else
				sw_info("	speed: unknown");		

			if(p->link_dup==LINK_HALF_DUPLEX)
				sw_info("	duplex: half");
			else if(p->link_dup==LINK_FULL_DUPLEX)
				sw_info("	duplex: full");
			else
				sw_info("	duplex: unknown");
		}else{
			sw_info("	speed: unknown");
			sw_info("	duplex: unknown");
		}
	
	}

	sw_info("\n");
        
}

#define CLEAN_DROPS
void ali_sw_show_drops(void)
{
	u32 i;
	SW_W8(SysPortAccess, 4);

	sw_info("\n=== disable packet dropping statistics ===");
	sw_info("egress packet drop count:");
	//packets remain too long time in switch fabric or the port is disabled, so drops the packets.
	sw_info("    port[0]: %d", SW_R32(PktDropP0CntEgress));
	sw_info("    port[1]: %d", SW_R32(PktDropP1CntEgress));
	sw_info("    port[2]: %d", SW_R32(PktDropP2CntEgress));
	sw_info("    port[3]: %d", SW_R32(PktDropP3CntEgress));
#ifdef CLEAN_DROPS	
	SW_W32(PktDropP0CntEgress, 0);
	SW_W32(PktDropP1CntEgress, 0);
	SW_W32(PktDropP2CntEgress, 0);
	SW_W32(PktDropP3CntEgress, 0);
#endif	
	sw_info("ingress packet drop count -- block limit:");
	sw_info("    port[1]: %d", SW_R32(PktDropP1CntIngressBL));
	sw_info("    port[2]: %d", SW_R32(PktDropP2CntIngressBL));
	sw_info("    port[3]: %d", SW_R32(PktDropP3CntIngressBL));
#ifdef CLEAN_DROPS		
	SW_W32(PktDropP1CntIngressBL, 0);
	SW_W32(PktDropP2CntIngressBL, 0);
	SW_W32(PktDropP3CntIngressBL, 0);
#endif
	sw_info("ingress packet drop count -- stp filter:");
	for(i=0;i<MAX_PORT_NUM;i++){
		SW_W8(PORT_STA_SEL, i<<6);
		sw_info("    port[%d]: %d", i, SW_R16(STP_DRP_CNT));
#ifdef CLEAN_DROPS			
		//SW_W16(STP_DRP_CNT, 0);
#endif
	}

	sw_info("ingress packet drop count -- alr filter:");
	for(i=0;i<MAX_PORT_NUM;i++){
		SW_W8(PORT_STA_SEL, i<<6);
		sw_info("    port[%d]: %d", i, SW_R16(ALR_DRP_CNT));
#ifdef CLEAN_DROPS			
		//SW_W16(ALR_DRP_CNT, 0);
#endif
	}
	
	sw_info("ingress packet drop count -- broadcast filter:");
	for(i=0;i<MAX_PORT_NUM;i++){
		SW_W8(PORT_STA_SEL, i<<6);
		sw_info("    port[%d]: %d", i, SW_R16(BDC_DRP_CNT));
#ifdef CLEAN_DROPS			
		//SW_W16(BDC_DRP_CNT, 0);
#endif
	}
	
	sw_info("ingress packet drop count -- ingress limit:");
	for(i=0;i<MAX_PORT_NUM;i++){
		SW_W8(PORT_STA_SEL, i<<6);
		sw_info("    port[%d]: %d", i, SW_R16(IGL_DRP_CNT));
	}
	for(i=1;i<MAX_PORT_NUM;i++){
#ifdef CLEAN_DROPS		
		SW_W16(STP_DRP_CNT, 0);
		SW_W16(ALR_DRP_CNT, 0);
		SW_W16(BDC_DRP_CNT, 0);
		SW_W16(IGL_DRP_CNT, 0);
#endif
	}
	
	sw_info("ingress packet drop count -- port disable:");
	for(i=0;i<MAX_PORT_NUM;i++){
		SW_W8(PORT_STA_SEL, i<<6);
		sw_info("    port[%d]: %d", i, SW_R16(PTD_DRP_CNT));
#ifdef CLEAN_DROPS			
		SW_W16(PTD_DRP_CNT, 0);
#endif
	}

	sw_info("port total drop count:");
	sw_info("    port[0]: %d", SW_R16(P0_DRP_CNT));
	sw_info("    port[1]: %d", SW_R16(P1_DRP_CNT));
	sw_info("    port[2]: %d", SW_R16(P2_DRP_CNT));
	sw_info("    port[3]: %d", SW_R16(P3_DRP_CNT));
#ifdef CLEAN_DROPS		
	SW_W16(P0_DRP_CNT, 0);
	SW_W16(P1_DRP_CNT, 0);
	SW_W16(P2_DRP_CNT, 0);
	SW_W16(P3_DRP_CNT, 0);
#endif	
	sw_info("");
	
}

void ali_sw_flow_control(pflowctl_info p_fc_val)
{
	u16 reg_val;
	if(p_fc_val->port_no>=MAX_PORT_NUM)
		sw_error("p_fc_val wrong: port_no=%d", p_fc_val->port_no);
	
	SW_W8(SysPortAccess, 4);
	if(p_fc_val->port_no == 0){
		SW_W8(FlowCtl, p_fc_val->p0_blks);
	}else{
		reg_val = SW_R16(FlowCtl);
		if(p_fc_val->port_no == 1){
			reg_val &= ~(FC_P1_IGRS_PORT_EN|FC_P1_IGRS_PRIOR_EN);
			if(p_fc_val->p123_mode==P123_FLOWCTL_NONE)
				;
			else if(p_fc_val->p123_mode==P123_FLOWCTL_PORT_ONLY)
				reg_val|=FC_P1_IGRS_PORT_EN;
			else if(p_fc_val->p123_mode==P123_FLOWCTL_PRIOR_ONLY)
				reg_val|=FC_P1_IGRS_PRIOR_EN;
			else if(p_fc_val->p123_mode==P123_FLOWCTL_PORT_PRIOR)
				reg_val|=(FC_P1_IGRS_PORT_EN|FC_P1_IGRS_PRIOR_EN);
			else
				sw_error("unsupported: %d, %s", __LINE__, __FUNCTION__);

			SW_W16(FlowCtl, reg_val);
		}else if(p_fc_val->port_no == 2){
			reg_val &= ~(FC_P2_IGRS_PORT_EN|FC_P2_IGRS_PRIOR_EN);
			if(p_fc_val->p123_mode==P123_FLOWCTL_NONE)
				;
			else if(p_fc_val->p123_mode==P123_FLOWCTL_PORT_ONLY)
				reg_val|=FC_P2_IGRS_PORT_EN;
			else if(p_fc_val->p123_mode==P123_FLOWCTL_PRIOR_ONLY)
				reg_val|=FC_P2_IGRS_PRIOR_EN;
			else if(p_fc_val->p123_mode==P123_FLOWCTL_PORT_PRIOR)
				reg_val|=(FC_P2_IGRS_PORT_EN|FC_P2_IGRS_PRIOR_EN);
			else
				sw_error("unsupported: %d, %s", __LINE__, __FUNCTION__);

			SW_W16(FlowCtl, reg_val);
		}else if(p_fc_val->port_no == 3){
			reg_val &= ~(FC_P3_IGRS_PORT_EN|FC_P3_IGRS_PRIOR_EN);
			if(p_fc_val->p123_mode==P123_FLOWCTL_NONE)
				;
			else if(p_fc_val->p123_mode==P123_FLOWCTL_PORT_ONLY)
				reg_val|=FC_P3_IGRS_PORT_EN;
			else if(p_fc_val->p123_mode==P123_FLOWCTL_PRIOR_ONLY)
				reg_val|=FC_P3_IGRS_PRIOR_EN;
			else if(p_fc_val->p123_mode==P123_FLOWCTL_PORT_PRIOR)
				reg_val|=(FC_P3_IGRS_PORT_EN|FC_P3_IGRS_PRIOR_EN);
			else
				sw_error("unsupported: %d, %s", __LINE__, __FUNCTION__);

			SW_W16(FlowCtl, reg_val);
		}
	}

	sw_trace("reg[FlowCtl]=0x%x", SW_R32(FlowCtl));
}

void ali_sw_ingress_rate_adjust_ext(pigrs_info p_igr_val)
{
	u16 reg_val;
	u8 i;
	SW_W8(SysPortAccess, 4);
	SW_W16(P0IngressRateScale, p_igr_val->p0_scale);
	SW_W32(P23CBS, p_igr_val->p23_cbs);
	SW_W32(P23EBS, p_igr_val->p23_ebs);
	SW_W32(P1CBS, p_igr_val->p1_cbs);
	SW_W32(P1EBS, p_igr_val->p1_ebs);

	for(i=0; i<16; i++){
		SW_W16((Port1TC0+i*2), p_igr_val->p1_tc[i]);
		SW_W16((Port2TC0+i*2), p_igr_val->p2_tc[i]);
		SW_W16((Port3TC0+i*2), p_igr_val->p3_tc[i]);
	}

	if(p_igr_val->ports_enable&PORT0_EN){
		reg_val = SW_R16(P0IngressRateCtl);
		reg_val|=P0IgressRateLimittEn;
		SW_W16(P0IngressRateCtl, reg_val);	
	}else{
		reg_val = SW_R16(P0IngressRateCtl);
		reg_val&=~P0IgressRateLimittEn;
		SW_W16(P0IngressRateCtl, reg_val);	
	}
	
	reg_val = 0;// SW_R16(P123IngressRateLimitCtl);
	if(p_igr_val->ports_enable&PORT1_EN)
		reg_val|=IGRS_PORT1_EN;	
	else
		reg_val&=~IGRS_PORT1_EN;	
	if(p_igr_val->ports_enable&PORT2_EN)
		reg_val|=IGRS_PORT2_EN;
	else
		reg_val&=~IGRS_PORT2_EN;
	if(p_igr_val->ports_enable&PORT3_EN)
		reg_val|=IGRS_PORT3_EN;
	else
		reg_val&=~IGRS_PORT3_EN;

	reg_val|=(DROP_YELLOW|DROP_RED);
	reg_val|=(p_igr_val->igrs_mode)<<IGRS_MODE_OFFSET;
	if(p_igr_val->priority_mode&IGRS_INFO_DSCP_EN)
		reg_val|=IGRS_DSCP_EN;
	if(p_igr_val->priority_mode&IGRS_INFO_ACL_EN)
		reg_val|=IGRS_ACL_EN;
	if(p_igr_val->priority_mode&IGRS_INFO_VLAN_EN)
		reg_val|=IGRS_VLAN_EN;

	SW_W16(P123IngressRateLimitCtl, reg_val);
	sw_trace("reg[P0IngressRateCtl]=0x%x, reg[P123IngressRateLimitCtl]=0x%x", SW_R32(P0IngressRateCtl), SW_R16(P123IngressRateLimitCtl));

}


void ali_sw_egress_rate_adjust(pegrs_info p_egr_val)
{
	u8 reg_val8;

	if((p_egr_val->port_no>=MAX_PORT_NUM)||(p_egr_val->mode>EGRS_NONE))
		sw_error("pegrs_info wrong: port_no=%d, mode=%d, scale=%d", p_egr_val->port_no, p_egr_val->mode, p_egr_val->scale);
	
	SW_W8(SysPortAccess, 4);
	if(p_egr_val->port_no == 0)
	{
		if(p_egr_val->mode == EGRS_PORT){
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8&=~(P0EgressRateLimitQueueMode);
			reg_val8|=P0EgressRateLimitEn;
			SW_W8(EgressRateCtl, reg_val8);
			
			SW_W16(EgressRateP0Q0, p_egr_val->scale);
		}
		else if(p_egr_val->mode == EGRS_NONE){
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8&=~(P0EgressRateLimitQueueMode);
			reg_val8&=~(P0EgressRateLimitEn);
			SW_W8(EgressRateCtl, reg_val8);

			SW_W16(EgressRateP0Q0, 0);
			SW_W16(EgressRateP0Q1, 0);
			SW_W16(EgressRateP0Q2, 0);
			SW_W16(EgressRateP0Q3, 0);
		}
		else{
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8|=P0EgressRateLimitQueueMode;
			reg_val8|=P0EgressRateLimitEn;
			SW_W8(EgressRateCtl, reg_val8);
			
			if(p_egr_val->mode==EGRS_QUEUE0)
				SW_W16(EgressRateP0Q0, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE1)
				SW_W16(EgressRateP0Q1, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE2)
				SW_W16(EgressRateP0Q2, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE3)
				SW_W16(EgressRateP0Q3, p_egr_val->scale);			
		}
		sw_trace("reg[EgressRateCtl]=0x%x, reg[EgressRateP0Q0~3]=0x%x, 0x%x, 0x%x, 0x%x", SW_R32(EgressRateCtl), SW_R16(EgressRateP0Q0), \
														SW_R16(EgressRateP0Q1), SW_R16(EgressRateP0Q2), SW_R16(EgressRateP0Q3));
	}

	if(p_egr_val->port_no == 1)
	{
		if(p_egr_val->mode == EGRS_PORT){
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8&=~(P1EgressRateLimitQueueMode);
			reg_val8|=P1EgressRateLimitEn;
			SW_W8(EgressRateCtl, reg_val8);
			
			SW_W16(EgressRateP1Q0, p_egr_val->scale);
		}
		else if(p_egr_val->mode == EGRS_NONE){
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8&=~(P1EgressRateLimitQueueMode);
			reg_val8&=~(P1EgressRateLimitEn);
			SW_W8(EgressRateCtl, reg_val8);

			SW_W16(EgressRateP1Q0, 0);
			SW_W16(EgressRateP1Q1, 0);
			SW_W16(EgressRateP1Q2, 0);
			SW_W16(EgressRateP1Q3, 0);
		}
		else{
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8|=P1EgressRateLimitQueueMode;
			reg_val8|=P1EgressRateLimitEn;
			SW_W8(EgressRateCtl, reg_val8);
			
			if(p_egr_val->mode==EGRS_QUEUE0)
				SW_W16(EgressRateP1Q0, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE1)
				SW_W16(EgressRateP1Q1, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE2)
				SW_W16(EgressRateP1Q2, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE3)
				SW_W16(EgressRateP1Q3, p_egr_val->scale);			
		}
		sw_trace("reg[EgressRateCtl]=0x%x, reg[EgressRateP1Q0~3]=0x%x, 0x%x, 0x%x, 0x%x", SW_R32(EgressRateCtl), SW_R16(EgressRateP1Q0), \
														SW_R16(EgressRateP1Q1), SW_R16(EgressRateP1Q2), SW_R16(EgressRateP1Q3));
	}


	if(p_egr_val->port_no == 2)
	{
		if(p_egr_val->mode == EGRS_PORT){
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8&=~(P2EgressRateLimitQueueMode);
			reg_val8|=P2EgressRateLimitEn;
			SW_W8(EgressRateCtl, reg_val8);
			
			SW_W16(EgressRateP2Q0, p_egr_val->scale);
		}
		else if(p_egr_val->mode == EGRS_NONE){
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8&=~(P2EgressRateLimitQueueMode);
			reg_val8&=~(P2EgressRateLimitEn);
			SW_W8(EgressRateCtl, reg_val8);

			SW_W16(EgressRateP2Q0, 0);
			SW_W16(EgressRateP2Q1, 0);
			SW_W16(EgressRateP2Q2, 0);
			SW_W16(EgressRateP2Q3, 0);
		}
		else{
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8|=P2EgressRateLimitQueueMode;
			reg_val8|=P2EgressRateLimitEn;
			SW_W8(EgressRateCtl, reg_val8);
			
			if(p_egr_val->mode==EGRS_QUEUE0)
				SW_W16(EgressRateP2Q0, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE1)
				SW_W16(EgressRateP2Q1, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE2)
				SW_W16(EgressRateP2Q2, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE3)
				SW_W16(EgressRateP2Q3, p_egr_val->scale);			
		}
		sw_trace("reg[EgressRateCtl]=0x%x, reg[EgressRateP2Q0~3]=0x%x, 0x%x, 0x%x, 0x%x", SW_R32(EgressRateCtl), SW_R16(EgressRateP2Q0), \
														SW_R16(EgressRateP2Q1), SW_R16(EgressRateP2Q2), SW_R16(EgressRateP2Q3));
	}


	if(p_egr_val->port_no == 3)
	{
		if(p_egr_val->mode == EGRS_PORT){
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8&=~(P3EgressRateLimitQueueMode);
			reg_val8|=P3EgressRateLimitEn;
			SW_W8(EgressRateCtl, reg_val8);
			
			SW_W16(EgressRateP3Q0, p_egr_val->scale);
		}
		else if(p_egr_val->mode == EGRS_NONE){
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8&=~(P3EgressRateLimitQueueMode);
			reg_val8&=~(P3EgressRateLimitEn);
			SW_W8(EgressRateCtl, reg_val8);

			SW_W16(EgressRateP3Q0, 0);
			SW_W16(EgressRateP3Q1, 0);
			SW_W16(EgressRateP3Q2, 0);
			SW_W16(EgressRateP3Q3, 0);
		}
		else{
			reg_val8 = SW_R8(EgressRateCtl);
			reg_val8|=P3EgressRateLimitQueueMode;
			reg_val8|=P3EgressRateLimitEn;
			SW_W8(EgressRateCtl, reg_val8);
			
			if(p_egr_val->mode==EGRS_QUEUE0)
				SW_W16(EgressRateP3Q0, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE1)
				SW_W16(EgressRateP3Q1, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE2)
				SW_W16(EgressRateP3Q2, p_egr_val->scale);
			if(p_egr_val->mode==EGRS_QUEUE3)
				SW_W16(EgressRateP3Q3, p_egr_val->scale);			
		}
		sw_trace("reg[EgressRateCtl]=0x%x, reg[EgressRateP3Q0~3]=0x%x, 0x%x, 0x%x, 0x%x", SW_R32(EgressRateCtl), SW_R16(EgressRateP3Q0), \
														SW_R16(EgressRateP3Q1), SW_R16(EgressRateP3Q2), SW_R16(EgressRateP3Q3));
	}


}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/****************************************************************************
-->	switch specific operations, such as ALR, PORT, etc.
	the functions here should not be static, becuase STP, MC snooping willl call them.

ALR is address logic Resolution (ALR) , the ALR format is:
	Bit 55	54			  53		  52		51		50		49		48		   47.........0
        Vaild   Age/Override	Static	Filter		Port3	Port2	Port1	Port0	MAC address
ALR related opertions:
	read MAC address, primarily for information printing.
	write MAC address  MC snooping will use it a lot. 
	clear all ALR records
	showing and displaying
****************************************************************************/
//#define DEBUG_ALR_OPERATION

#if 0
static int ali_sw_alr_set_index_mode(UINT32 idx)
{
//	UINT32 ba = priv->io_base;
	UINT32 tmp;
	tmp = SW_R32(AlrCtl);
	tmp &= (~AlrIndexModMsk);
	tmp |= (idx<<AlrIndexModOffset);
#ifdef DEBUG_ALR_OPERATION
	printk("AlrCtl = 0x%x\n", SW_R32(AlrCtl));
#endif
	SW_W32(AlrCtl, tmp);
	return (SW_R32(AlrCtl)&AlrErrFlag);
}

int ali_sw_alr_check_full(void)
{
//	UINT32 ba = priv->io_base;
	SW_W8(SysPortAccess, 4);
	return (SW_R32(AlrCtl)&AlrFullFlag);
}

#endif
int ali_sw_alr_current_entry_number(void)
{
	UINT32 tmp;
	SW_W8(SysPortAccess, 4);
	tmp = SW_R32(AlrCtl);
	tmp &= AlrEntryNumMsk;
	tmp >>= AlrEntryNumOffset;
	return (tmp);
}
int ali_sw_alr_delete_entry(p_alr_wdata p_entry)
{

}
int ali_sw_alr_write_entry(p_alr_wdata p_entry)
{
	UINT32 tmp;
#ifdef DEBUG_ALR_OPERATION
	printk("p_entry->mac_addr_low32=0x%x\n", p_entry->mac_addr_low32);
	printk("p_entry->mac_addr_high16=0x%x\n", p_entry->mac_addr_high16);
	printk("p_entry->port=0x%x\n", p_entry->ports);
	printk("p_entry->filter=0x%x\n", p_entry->filter);
	printk("p_entry->static=0x%x\n", p_entry->static_entry);
	printk("p_entry->age=0x%x\n", p_entry->age);
	printk("p_entry->valid=0x%x\n", p_entry->valid);
#endif

	tmp = *(UINT32 *)p_entry;
	
//step 1. write content
	SW_W8(SysPortAccess, 4);
	SW_W32(AlrWdata1, tmp);
	//SW_W32(AlrCtl, *(UINT32 *)p_entry);//this is prefered.
	SW_W32(AlrWdata2, p_entry->mac_addr_low32);
	
//step 2. write cmd
	tmp = SW_R32(AlrCtl);
//	tmp &= (~AlrAddrMsk);
//	tmp |= ((p_entry->entery_addr)<<AlrAddrOffset);//entry address
	tmp |= (AlrWRCmd|AlrCmdReq);
	SW_W32(AlrCtl, tmp);
	
#ifdef DEBUG_ALR_OPERATION
	sw_info("the result is:");
	printk("reg[AlrCtl] = 0x%x\n", SW_R32(AlrCtl));
	printk("reg[AlrWdata1] = 0x%x\n", SW_R32(AlrWdata1));
	printk("reg[AlrWdata2] = 0x%x\n", SW_R32(AlrWdata2));
#endif

//step 3. check status
	do{
		tmp = SW_R32(AlrCtl);
	}while(tmp&AlrCmdReq);

	if((tmp&AlrErrFlag)!=0)
	{
		sw_warn("%s, error, table full or same entry!", __FUNCTION__);
		SW_W32(AlrCtl, SW_R32(AlrCtl));//clear error
		//SW_W32(AlrCtl, SW_R32(AlrCtl)&(~AlrErrFlag));
		return -1;
	}
	return 0;
}

int ali_sw_alr_read_entry(p_alr_rdata p_entry)
{
	UINT32 tmp;

//step 1. write cmd
	SW_W8(SysPortAccess, 4);
	tmp = SW_R32(AlrCtl);
	tmp &= (~AlrIdxMsk);
	tmp |= ((p_entry->entery_idx)<<AlrIdxOffset);//entry address
	tmp &= (~AlrWRCmd);
	tmp |= (AlrCmdReq);
	SW_W32(AlrCtl, tmp);
	
#ifdef DEBUG_ALR_OPERATION
	printk("reg[AlrCtl] = 0x%x\n", SW_R32(AlrCtl));
#endif

//step 2. check status
	do{
		tmp = SW_R32(AlrCtl);
	}while(tmp&AlrCmdReq);

//step 1. read content
	tmp = SW_R32(AlrRdata1);
	*(UINT32 *)p_entry = tmp;
	//*(UINT32 *)p_entry = SW_W32(AlrCtl, );
	p_entry->mac_addr_low32 = SW_R32(AlrRdata2);
	
#ifdef DEBUG_ALR_OPERATION
	sw_info("the result is:");
	printk("p_entry->mac_addr_high16=0x%x\n", p_entry->mac_addr_high16);
	printk("p_entry->mac_addr_low32=0x%x\n", p_entry->mac_addr_low32);
	printk("p_entry->port=0x%x\n", p_entry->ports);
	printk("p_entry->filter=0x%x\n", p_entry->filter);
	printk("p_entry->static=0x%x\n", p_entry->static_entry);
	//printk("p_entry->EOT=0x%x\n", p_entry->EOT);
	printk("p_entry->valid=0x%x\n", p_entry->valid);
#endif
	p_entry->valid = (SW_R32(AlrWdata1)>>29)&(1);
	
	if((tmp&AlrErrFlag)!=0)
	{
		//sw_warn("%s, error? weird.", __FUNCTION__);
		SW_W32(AlrCtl, SW_R32(AlrCtl));//clear error
		//SW_W32(AlrCtl, SW_R32(AlrCtl)&(~AlrErrFlag));
		return -1;
	}
	return 0;
}

static void ali_sw_alr_format_display(p_alr_rdata p_entry, bool show_title)
{
	u8 tmp;
	
	if(show_title == 0)
	{	
		printk("\nShowing ALi Switch Address Logic Resolution (ALR) Entries...\n");
		printk("No. Vaild   Static	Filter	P3 P2 P1 P0	MAC address\n");
	}

	printk("%d	", p_entry->entery_idx);
	printk("%s	", p_entry->valid?"Y":"N");
	printk("%s	", p_entry->static_entry?"Y":"N");
	printk("%s	", p_entry->filter?"out":"in");

	tmp = p_entry->ports;
	printk("%c  ", (tmp&0x8)?'Y':'N');
	printk("%c  ", (tmp&0x4)?'Y':'N');
	printk("%c  ", (tmp&0x2)?'Y':'N');
	printk("%c	", (tmp&0x1)?'Y':'N');
	
	printk("%02x-%02x-%02x-%02x-%02x-%02x\n", \
									HIBYTE(LOWORD(p_entry->mac_addr_high16)),\
									LOBYTE(LOWORD(p_entry->mac_addr_high16)),\
									HIBYTE(HIWORD(p_entry->mac_addr_low32)),\
									LOBYTE(HIWORD(p_entry->mac_addr_low32)),\
									HIBYTE(LOWORD(p_entry->mac_addr_low32)),\
									LOBYTE(LOWORD(p_entry->mac_addr_low32)));
	
}


static UINT32 ali_sw_alr_show_all_entries(p_alr_rdata p_entry)
{
//	UINT32 ba = priv->io_base;
	UINT32 idx, total;
	//struct alr_rdata entry;

	total = ali_sw_alr_current_entry_number();

	for(idx=0; idx<SW_ALR_TOTAL_ENTRY_NUM; idx++)
	{
		p_entry->entery_idx = idx;
		//if(ali_sw_alr_read_entry(p_entry));
		//	printk(KERN_ERR "%s: return error.\n",__FUNCTION__);
		ali_sw_alr_read_entry(p_entry);	
		ali_sw_alr_format_display(p_entry, idx);
	}
	printk("total %d entries in ALR table.\n", total);
	
	return total;
}

void ali_sw_alr_fast_aging(u8 flag)
{
	u32 tmp;
	SW_W8(SysPortAccess, 4);//0x160
	tmp = SW_R32(AlrCtl);
	if(flag==1)
		tmp |= (AlrAgeFastAging);
	else
		tmp &= ~(AlrAgeFastAging);
	SW_W32(AlrCtl, tmp);
	sw_trace("fast_aging: reg[AlrCtl]=0x%x", SW_R32(AlrCtl));
}

void ali_sw_alr_fast_aging_ext(u32 aging_time)
{
	SW_W8(SysPortAccess, 4);
	SW_W32(ScanTime, aging_time);
	sw_trace("fast_aging: reg[ScanTime]=0x%x", SW_R32(ScanTime));
}


static void ali_sw_test_timer_hdler(unsigned long arg)
{
	//pr_info("sw: alr fast aging timer expired\n");
	sw_trace("aging time changed to 300 (s)");
	del_timer(&sw_test_timer);
	ali_sw_alr_fast_aging_ext(300);
}

static void dump_all_tx_descriptors(void)
{
	p_tx_desc desc;
	u8 idx = 0;
	sw_trace("");
	for(idx=0; idx<ALI_SW_TX_DESC_NUM; idx++){
		desc = &gp_p0->tx_desc[idx];
		sw_trace("tx desc no.%d at 0x%x", idx, (u32)desc);
		p0_print(desc, ALI_SW_DESC_SZ);			
	}
	sw_trace("");
}

static void dump_tx_desc_verbose(u8 idx) {
	p_tx_desc desc;
	desc = &gp_p0->tx_desc[idx];
	sw_trace("tx desc no.%d at 0x%x", idx, (u32)desc);
	if (desc->ContextDescriptor.ContextData == 1) {
		printk("	ContextDes.VlanTag = %d\n", desc->ContextDescriptor.VlanTag);
		printk("	ContextDes.L3Type = %d\n", desc->ContextDescriptor.L3Type);
		printk(" 	ContextDes.L4Type = %d\n", desc->ContextDescriptor.L4Type);
		printk(" 	ContextDes.L4HeaderLen = %d\n", desc->ContextDescriptor.L4HeaderLen);
		printk(" 	ContextDes.ContextIndex = %d\n", desc->ContextDescriptor.ContextIndex);
		printk(" 	ContextDes.ContextData = %d\n", desc->ContextDescriptor.ContextData);
		printk(" 	ContextDes.OWN = %d\n", desc->ContextDescriptor.OWN);
		printk(" 	ContextDes.L3HeaderLen = %d\n", desc->ContextDescriptor.L3HeaderLen);
		printk(" 	ContextDes.L2HeaderLen = %d\n", desc->ContextDescriptor.L2HeaderLen);
		printk(" 	ContextDes.MSS = %d\n", desc->ContextDescriptor.MSS);
		printk(" 	ContextDes.Reserved1  = %d\n", desc->ContextDescriptor.Reserved1);
		printk(" 	ContextDes.Reserved2  = %d\n", desc->ContextDescriptor.Reserved2);
	} else {
		printk("	desc->DataDescriptor.SegmentNum = %d\n", desc->DataDescriptor.SegmentNum);
		printk("	desc->DataDescriptor.ForwardPorts = %d\n", desc->DataDescriptor.ForwardPorts);
		printk("	desc->DataDescriptor.TX_PRIOR = %d\n", desc->DataDescriptor.TX_PRIOR);
		printk("	desc->DataDescriptor.TOE_UFO_En = %d\n", desc->DataDescriptor.TOE_UFO_En);
		printk("	desc->DataDescriptor.VlanEn = %d\n", desc->DataDescriptor.VlanEn);
		printk("	desc->DataDescriptor.CoeEn = %d\n", desc->DataDescriptor.CoeEn);
		printk("	desc->DataDescriptor.TX_PRIOR_EN = %d\n", desc->DataDescriptor.TX_PRIOR_EN);
		printk("	desc->DataDescriptor.FS = %d\n", desc->DataDescriptor.FS);
		printk("	desc->DataDescriptor.EOR = %d\n", desc->DataDescriptor.EOR);
		printk("	desc->DataDescriptor.ContextIndex = %d\n", desc->DataDescriptor.ContextIndex);
		printk("	desc->DataDescriptor.ContextData = %d\n", desc->DataDescriptor.ContextData);
		printk("	desc->DataDescriptor.OWN = %d\n", desc->DataDescriptor.OWN);
		printk("	desc->DataDescriptor.pkt_buf_dma = 0x%x\n", desc->DataDescriptor.pkt_buf_dma);
		printk("	desc->DataDescriptor.seg_len = %d\n", desc->DataDescriptor.seg_len);
		printk("	desc->DataDescriptor.tot_len = %d\n", desc->DataDescriptor.tot_len);
		printk("	desc->DataDescriptor.FragmentID = %d\n", desc->DataDescriptor.FragmentID);	
	}
}
#if 0
static void dump_all_tx_desc_verbose(void)
{
	u8 idx = 0;
	sw_trace("");
	for(idx=0; idx<ALI_SW_TX_DESC_NUM; idx++){
		dump_tx_desc_verbose(idx);
		printk("\n");
	}
	sw_trace("");
}
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define DEBUG_VLAN_OPERATION
static int ali_sw_vlut_read_entry(u8 idx, u32* p_entry)
{
	u32 reg_val=0;
	
	if(idx>0xf)
	{
		sw_error("%s: idx range error (%d)!", __FUNCTION__, idx);
		return -1;
	}

	reg_val&=(~VlutCmdWR);//read
	reg_val&=(~VlutAddrMsk);
	reg_val|=(idx<<VlutAddrOffset);
	reg_val|=VLutCmdRequest;

#ifdef DEBUG_VLAN_OPERATION
	//sw_trace("idx=%d, reg_val=0x%x", idx, reg_val);
#endif

	SW_W8(SysPortAccess, 4);
	SW_W32(VlanTableCmdWd, reg_val);
	do{
		reg_val = SW_R32(VlanTableCmdWd);
	}while(reg_val&VLutCmdAck);

	reg_val = SW_R32(VlanTableRd);
	*p_entry = (reg_val&VlutData);
	
#ifdef DEBUG_VLAN_OPERATION
	//sw_trace("idx=%d, reg_val=0x%x", idx, reg_val);
#endif
	return 0;
}

static int ali_sw_vlut_write_entry(u8 idx, u32* p_entry)
{
	u32 reg_val=0;
	
	if(idx>0xf)
	{
		sw_error("%s: idx range error (%d)!", __FUNCTION__, idx);
		return -1;
	}

	reg_val = *p_entry;
	reg_val|=VlutCmdWR;//write
	reg_val&=(~VlutAddrMsk);
	reg_val|=(idx<<VlutAddrOffset);
	reg_val|=VLutCmdRequest;

#ifdef DEBUG_VLAN_OPERATION
	sw_trace("write vlut: idx=%d, reg_val=0x%x", idx, reg_val);
#endif

	SW_W8(SysPortAccess, 4);
	SW_W32(VlanTableCmdWd, reg_val);
	do{
		reg_val = SW_R32(VlanTableCmdWd);
	}while(reg_val&VLutCmdAck);
	
	return 0;
}

static void ali_sw_vlut_format_display(u8 idx, p_vlut_data p_entry, bool show_title)
{
	if(show_title == 0)
	{	
		printk("\nShowing VLAN Lookup Table...\n");
		printk("No.	P3   P2   P1   P0	VID\n");
	}
    if(!p_entry) {
        return;
    }
	printk("%d	", idx);
	printk("%c(%c) ", p_entry->port3_member?'Y':'N', p_entry->port3_tag?'u':'t');
	printk("%c(%c) ", p_entry->port2_member?'Y':'N', p_entry->port2_tag?'u':'t');
	printk("%c(%c) ", p_entry->port1_member?'Y':'N', p_entry->port1_tag?'u':'t');
	printk("%c(%c) 	", p_entry->port0_member?'Y':'N', p_entry->port0_tag?'u':'t');
	printk("%d\n", p_entry->vid);
}

static void ali_sw_vlut_show_all_entries(void)
{
	u8 idx;
	struct vlut_data entry;
	
	for(idx=0; idx<SW_VLUT_TOTAL_ENTRY_NUM; idx++)
	{
		memset(&entry, 0, sizeof(entry));
		ali_sw_vlut_read_entry(idx, (u32*)&entry);	
		ali_sw_vlut_format_display(idx, &entry, idx);
	}
	printk("\n");
}

static int ali_sw_vlan_read_port_type_ext(u8 port_no, pvlan_port_info p)
{
	u32 reg_val=0;
	
	if(port_no>4)
	{
		sw_error("%s: port range error (%d)!", __FUNCTION__, port_no);
		return -1;
	}
	
	SW_W8(SysPortAccess, 4);
	reg_val = SW_R32(VlanPortType);

	if(port_no==0){
		p->port_type = ((reg_val>>VlanPort0TypeOffset)&VlanPortTypeMsk);
		
		p->fff_chg = (reg_val&VLAN_PORT0_FFF_CHG)>>VLAN_PORT0_FFF_CHG_OFFSET;
		p->igrs_filter = (reg_val&VLAN_PORT0_IGRS_FILTER)>>VLAN_PORT0_IGRS_FILTER_OFFSET;
		p->tag_chg = (reg_val&VLAN_PORT0_TAG_CHG)>>VLAN_PORT0_TAG_CHG_OFFSET;
		p->tag_only = (reg_val&VLAN_PORT0_TAG_ONLY)>>VLAN_PORT0_TAG_ONLY_OFFSET;
		p->untag_only = (reg_val&VLAN_PORT0_UNTAG_ONLY)>>VLAN_PORT0_UNTAG_ONLY_OFFSET;
	}
	else if(port_no==1){
		p->port_type = ((reg_val>>VlanPort1TypeOffset)&VlanPortTypeMsk);
		
		p->fff_chg = (reg_val&VLAN_PORT1_FFF_CHG)>>VLAN_PORT1_FFF_CHG_OFFSET;
		p->igrs_filter = (reg_val&VLAN_PORT1_IGRS_FILTER)>>VLAN_PORT1_IGRS_FILTER_OFFSET;
		p->tag_chg = (reg_val&VLAN_PORT1_TAG_CHG)>>VLAN_PORT1_TAG_CHG_OFFSET;
		p->tag_only = (reg_val&VLAN_PORT1_TAG_ONLY)>>VLAN_PORT1_TAG_ONLY_OFFSET;
		p->untag_only = (reg_val&VLAN_PORT1_UNTAG_ONLY)>>VLAN_PORT1_UNTAG_ONLY_OFFSET;
	}
	else if(port_no==2){
		p->port_type = ((reg_val>>VlanPort2TypeOffset)&VlanPortTypeMsk);

		p->fff_chg = (reg_val&VLAN_PORT2_FFF_CHG)>>VLAN_PORT2_FFF_CHG_OFFSET;
		p->igrs_filter = (reg_val&VLAN_PORT2_IGRS_FILTER)>>VLAN_PORT2_IGRS_FILTER_OFFSET;
		p->tag_chg = (reg_val&VLAN_PORT2_TAG_CHG)>>VLAN_PORT2_TAG_CHG_OFFSET;
		p->tag_only = (reg_val&VLAN_PORT2_TAG_ONLY)>>VLAN_PORT2_TAG_ONLY_OFFSET;
		p->untag_only = (reg_val&VLAN_PORT2_UNTAG_ONLY)>>VLAN_PORT2_UNTAG_ONLY_OFFSET;
	}
	else{
		p->port_type = ((reg_val>>VlanPort3TypeOffset)&VlanPortTypeMsk);

		p->fff_chg = (reg_val&VLAN_PORT3_FFF_CHG)>>VLAN_PORT3_FFF_CHG_OFFSET;
		p->igrs_filter = (reg_val&VLAN_PORT3_IGRS_FILTER)>>VLAN_PORT3_IGRS_FILTER_OFFSET;
		p->tag_chg = (reg_val&VLAN_PORT3_TAG_CHG)>>VLAN_PORT3_TAG_CHG_OFFSET;
		p->tag_only = (reg_val&VLAN_PORT3_TAG_ONLY)>>VLAN_PORT3_TAG_ONLY_OFFSET;
		p->untag_only = (reg_val&VLAN_PORT3_UNTAG_ONLY)>>VLAN_PORT3_UNTAG_ONLY_OFFSET;		
	}
	
	return 0;
}

#if 0
static int ali_sw_vlan_read_port_type(u8 port_no, u8 *port_type)
{
	u32 reg_val=0;
	
	if(port_no>4)
	{
		sw_error("%s: port range error (%d)!", __FUNCTION__, port_no);
		return -1;
	}
	
	SW_W8(SysPortAccess, 4);
	reg_val = SW_R32(VlanPortType);

	if(port_no==0)
		*port_type = ((reg_val>>VlanPort0TypeOffset)&VlanPortTypeMsk);
	else if(port_no==1)
		*port_type = ((reg_val>>VlanPort1TypeOffset)&VlanPortTypeMsk);
	else if(port_no==2)
		*port_type = ((reg_val>>VlanPort2TypeOffset)&VlanPortTypeMsk);
	else 
		*port_type = ((reg_val>>VlanPort3TypeOffset)&VlanPortTypeMsk);

#ifdef DEBUG_VLAN_OPERATION
	//sw_trace("port_no=%d, port_type=%d, reg_val=0x%x", port_no, *port_type, reg_val);
#endif
	
	return 0;
}
#endif

static int ali_sw_vlan_write_port_type_ext(pvlan_port_info p)
{
	u8 port_no;
	u8 port_type;
	u32 reg_val=0;

	port_no = p->port_no;
	port_type = p->port_type;
	
	if(port_no>4)
	{
		sw_error("%s: port range error (%d)!", __FUNCTION__, port_no);
		return -1;
	}
	if(port_type>VlanPortTypeMsk)
	{
		sw_error("%s: port type error (%d)!", __FUNCTION__, port_type);
		return -1;
	}
	
	SW_W8(SysPortAccess, 4);
	reg_val = SW_R32(VlanPortType);
	
	if(port_no==0)
	{	
		reg_val&=(~(VlanPortTypeMsk<<VlanPort0TypeOffset));
		reg_val|=(port_type<<VlanPort0TypeOffset);

		reg_val&=~VLAN_PORT0_FFF_CHG;
		reg_val&=~VLAN_PORT0_IGRS_FILTER;
		reg_val&=~VLAN_PORT0_TAG_CHG;
		reg_val&=~VLAN_PORT0_TAG_ONLY;
		reg_val&=~VLAN_PORT0_UNTAG_ONLY;
		
		reg_val|=(p->fff_chg<<VLAN_PORT0_FFF_CHG_OFFSET);
		reg_val|=(p->igrs_filter<<VLAN_PORT0_IGRS_FILTER_OFFSET);
		reg_val|=(p->tag_chg<<VLAN_PORT0_TAG_CHG_OFFSET);
		reg_val|=(p->tag_only<<VLAN_PORT0_TAG_ONLY_OFFSET);
		reg_val|=(p->untag_only<<VLAN_PORT0_UNTAG_ONLY_OFFSET);
	}
	else if(port_no==1)
	{
		reg_val&=(~(VlanPortTypeMsk<<VlanPort1TypeOffset));
		reg_val|=(port_type<<VlanPort1TypeOffset);

		reg_val&=~VLAN_PORT1_FFF_CHG;
		reg_val&=~VLAN_PORT1_IGRS_FILTER;
		reg_val&=~VLAN_PORT1_TAG_CHG;
		reg_val&=~VLAN_PORT1_TAG_ONLY;
		reg_val&=~VLAN_PORT1_UNTAG_ONLY;
		
		reg_val|=(p->fff_chg<<VLAN_PORT1_FFF_CHG_OFFSET);
		reg_val|=(p->igrs_filter<<VLAN_PORT1_IGRS_FILTER_OFFSET);
		reg_val|=(p->tag_chg<<VLAN_PORT1_TAG_CHG_OFFSET);
		reg_val|=(p->tag_only<<VLAN_PORT1_TAG_ONLY_OFFSET);
		reg_val|=(p->untag_only<<VLAN_PORT1_UNTAG_ONLY_OFFSET);
	}
	else if(port_no==2)
	{
		reg_val&=(~(VlanPortTypeMsk<<VlanPort2TypeOffset));
		reg_val|=(port_type<<VlanPort2TypeOffset);

		reg_val&=~VLAN_PORT2_FFF_CHG;
		reg_val&=~VLAN_PORT2_IGRS_FILTER;
		reg_val&=~VLAN_PORT2_TAG_CHG;
		reg_val&=~VLAN_PORT2_TAG_ONLY;
		reg_val&=~VLAN_PORT2_UNTAG_ONLY;
		
		reg_val|=(p->fff_chg<<VLAN_PORT2_FFF_CHG_OFFSET);
		reg_val|=(p->igrs_filter<<VLAN_PORT2_IGRS_FILTER_OFFSET);
		reg_val|=(p->tag_chg<<VLAN_PORT2_TAG_CHG_OFFSET);
		reg_val|=(p->tag_only<<VLAN_PORT2_TAG_ONLY_OFFSET);
		reg_val|=(p->untag_only<<VLAN_PORT2_UNTAG_ONLY_OFFSET);
	}
	else 
	{
		reg_val&=(~(VlanPortTypeMsk<<VlanPort3TypeOffset));
		reg_val|=(port_type<<VlanPort3TypeOffset);

		reg_val&=~VLAN_PORT3_FFF_CHG;
		reg_val&=~VLAN_PORT3_IGRS_FILTER;
		reg_val&=~VLAN_PORT3_TAG_CHG;
		reg_val&=~VLAN_PORT3_TAG_ONLY;
		reg_val&=~VLAN_PORT3_UNTAG_ONLY;
		
		reg_val|=(p->fff_chg<<VLAN_PORT3_FFF_CHG_OFFSET);
		reg_val|=(p->igrs_filter<<VLAN_PORT3_IGRS_FILTER_OFFSET);
		reg_val|=(p->tag_chg<<VLAN_PORT3_TAG_CHG_OFFSET);
		reg_val|=(p->tag_only<<VLAN_PORT3_TAG_ONLY_OFFSET);
		reg_val|=(p->untag_only<<VLAN_PORT3_UNTAG_ONLY_OFFSET);
	}

	
#ifdef DEBUG_VLAN_OPERATION
	sw_trace("write vport: port_no=%d, port_type=%d, reg_val=0x%x", port_no, port_type, reg_val);
#endif

	SW_W32(VlanPortType, reg_val);
	
	return 0;
}

#if 0
static int ali_sw_vlan_write_port_type(u8 port_no, u8 port_type)
{
	u32 reg_val=0;
	
	if(port_no>4)
	{
		sw_error("%s: port range error (%d)!", __FUNCTION__, port_no);
		return -1;
	}
	if(port_type>VlanPortTypeMsk)
	{
		sw_error("%s: port type error (%d)!", __FUNCTION__, port_type);
		return -1;
	}
	
	SW_W8(SysPortAccess, 4);
	reg_val = SW_R32(VlanPortType);
	
	if(port_no==0)
	{	
		reg_val&=(~(VlanPortTypeMsk<<VlanPort0TypeOffset));
		reg_val|=(port_type<<VlanPort0TypeOffset);
	}
	else if(port_no==1)
	{
		reg_val&=(~(VlanPortTypeMsk<<VlanPort1TypeOffset));
		reg_val|=(port_type<<VlanPort1TypeOffset);
	}
	else if(port_no==2)
	{
		reg_val&=(~(VlanPortTypeMsk<<VlanPort2TypeOffset));
		reg_val|=(port_type<<VlanPort2TypeOffset);
	}
	else 
	{
		reg_val&=(~(VlanPortTypeMsk<<VlanPort3TypeOffset));
		reg_val|=(port_type<<VlanPort3TypeOffset);
	}

	
#ifdef DEBUG_VLAN_OPERATION
	sw_trace("write vport: port_no=%d, port_type=%d, reg_val=0x%x", port_no, port_type, reg_val);
#endif

	SW_W32(VlanPortType, reg_val);
	
	return 0;
}
#endif

static int ali_sw_vlan_read_port_tag(u8 port_no, u8 *ppriority, u16 *pvid)
{
	u16 reg_val=0;
	
	if(port_no>4)
	{
		sw_error("%s: port range error (%d)!", __FUNCTION__, port_no);
		return -1;
	}	
	
	SW_W8(SysPortAccess, 4);

	if(port_no==0)
		reg_val=SW_R16(VlanPort0Tag);
	else if(port_no==1)
		reg_val=SW_R16(VlanPort1Tag);
	else if(port_no==2)
		reg_val=SW_R16(VlanPort2Tag);
	else 
		reg_val=SW_R16(VlanPort3Tag);

	*pvid = (reg_val&PvidMsk)>>VlanPvidOffset;
	*ppriority = (reg_val&PpriortyMsk)>>VlanPpriortyOffset;

#ifdef DEBUG_VLAN_OPERATION
	//sw_trace("port_no=%d, priority=%d, pvid=%d, reg_val=0x%x", port_no, *ppriority, *pvid, reg_val);
#endif

	return 0;
}

static int ali_sw_vlan_write_port_tag(u8 port_no, u8 ppriority, u16 pvid)
{
	u16 reg_val=0;
	
	if(port_no>4)
	{
		sw_error("%s: port range error (%d)!", __FUNCTION__, port_no);
		return -1;
	}
	if(ppriority>0x7)
	{
		sw_error("%s: port priority range error (%d)!", __FUNCTION__, ppriority);
		return -1;
	}
	if(pvid>0xfff)
	{
		sw_error("%s: port pvid range error (%d)!", __FUNCTION__, pvid);
		return -1;
	}		
	
	SW_W8(SysPortAccess, 4);

	if(port_no==0)
		reg_val=SW_R16(VlanPort0Tag);
	else if(port_no==1)
		reg_val=SW_R16(VlanPort1Tag);
	else if(port_no==2)
		reg_val=SW_R16(VlanPort2Tag);
	else 
		reg_val=SW_R16(VlanPort3Tag);

	reg_val&=(~PvidMsk);
	reg_val&=(~PpriortyMsk);

	reg_val|=(ppriority<<VlanPpriortyOffset);
	reg_val|=(pvid<<VlanPvidOffset);

#ifdef DEBUG_VLAN_OPERATION
	sw_trace("port_no=%d, priority=%d, pvid=%d, reg_val=0x%x", port_no, ppriority, pvid, reg_val);
#endif

	if(port_no==0)
		SW_W16(VlanPort0Tag, reg_val);
	else if(port_no==1)
		SW_W16(VlanPort1Tag, reg_val);
	else if(port_no==2)
		SW_W16(VlanPort2Tag, reg_val);
	else 
		SW_W16(VlanPort3Tag, reg_val);

	return 0;
}

static void ali_sw_vlan_port_info_format_display(pvlan_port_info p_entry, bool show_title)
{
	if(show_title == 0)
	{	
		printk("\nShowing VLAN port info...\n");
		printk("No.	Type	      Priority  PVID  4095_chg	  igrs_filter  tag_chg  tag_only  untag_only\n");
	}

	printk("%d	", p_entry->port_no);

	if(p_entry->port_type==VlanPortDumb)
		printk("dump	");
	else if(p_entry->port_type==VlanPortAccess)
		printk("access	");
	else if(p_entry->port_type==VlanPortTrunk)
		printk("trunk	");
	else 
		printk("hybrid	");
	
	printk("	%d	", p_entry->ppriority);
	printk("%d	    ", p_entry->pvid);

	printk("%d	  	", p_entry->fff_chg);
	printk("%d	  ", p_entry->igrs_filter);
	printk("%d	     ", p_entry->tag_chg);
	printk("%d	     ", p_entry->tag_only);
	printk("%d", p_entry->untag_only);



	printk("\n");
}

static void ali_sw_vlan_show_all_port_info(void)
{
	u8 port_no;
	//u8 ppriority, port_type;
	//u16 pvid;
	struct vlan_port_info one_info;
	
	for(port_no=0; port_no<MAX_PORT_NUM; port_no++)
	{
		one_info.port_no=port_no;
		ali_sw_vlan_read_port_tag(port_no, &(one_info.ppriority), &(one_info.pvid));
		//ali_sw_vlan_read_port_type(port_no, &(one_info.port_type));
		ali_sw_vlan_read_port_type_ext(port_no, &one_info);
		ali_sw_vlan_port_info_format_display(&one_info, one_info.port_no);
	}
	printk("\n");
}

/****************************************************************************/
/*STP*/
void ali_sw_set_port_stp_hwstatus(u8 port_no, u8 stp_status)
{
//	UINT32 ba = priv->io_base;
	UINT32 tmp;
	u32 flags = 0;
	
	if((port_no>4)||(stp_status>ALI_SW_STATE_BLOCKING))
	{
		sw_error("out of range, %d, %d, %s, %d", port_no, stp_status, __FUNCTION__, __LINE__);
		return; 
	}

	spin_lock_irqsave(&gp_sw->lock, flags);
	
	SW_W8(SysPortAccess, 4);
	tmp = SW_R32(PktDecCtl);
	tmp&=(~(PortStpStatusMsk<<(port_no<<1)));

	switch(stp_status)
	{
		case ALI_SW_STATE_DISABLED:
			tmp |= (STP_STATE_HW_DISABLED<<(port_no<<1));
			break;
		case ALI_SW_STATE_BLOCKING:
		case ALI_SW_STATE_LISTENING:
			tmp |= (STP_STATE_HW_BLOCKING_LISTENING<<(port_no<<1));
			break;
		case ALI_SW_STATE_LEARNING:
			tmp |= (STP_STATE_HW_LEARNING<<(port_no<<1));
			break;
		case ALI_SW_STATE_FORWARDING:
			tmp |= (STP_STATE_HW_FORWARDING<<(port_no<<1));
			break;	
		default:
			sw_error("%s, wrong stp_status!\n", __FUNCTION__);
			
	}

	SW_W32(PktDecCtl, tmp);
	spin_unlock_irqrestore(&gp_sw->lock, flags);
	
#ifdef ALI_SW_DEBUG_STP
	if(test_bit(5, &dbg_runtime_val)){
		sw_trace("set_port_stp_hwstatus: port_no:%d, stp_status:%d, reg[PktDecCtl]:0x%x", port_no, stp_status, SW_R32(PktDecCtl));;
	}
#endif	
	return;
}

u8 ali_sw_get_port_cost(struct net_ali_sw_port *p)
{
/*
	UINT32 tmp;
	tmp = SW_R32(LinkCheckStat);

	if(port_no == 0)
		tmp = STP_SPEED_1000M;
	else
	{
		tmp>>=(P3SpeedModOffset+(MAX_PORT_NUM - 1 - port_no)*4);// 1--9, 2--5, 3--1
		tmp&=(~P123SpeedModMsk);
		printk("%s, tmp=0x%x\n", __FUNCTION__, tmp);	
	}
*/
	if((p->transmit_okay!=LINK_OKAY_TO_TRANSMIT)||(p->link_spd==LINK_SPEED_RESERVED))
	{
		return 0xff;
	}

	switch(p->link_spd) {
	case LINK_SPEED_1000M:
		return 4;
	case LINK_SPEED_100M:
		return 19;
	case LINK_SPEED_10M:
		return 100;
	default:
		sw_error("%s, path cost err", __FUNCTION__);
		return 0xff;
	}

}

bool ali_sw_check_port_link_status(struct net_ali_sw_port *p)
{
	return p->transmit_okay;
}

static void p0_alloc_descriptors(p_p0_private priv) {
	void *rx_desc = NULL;
	void *tx_desc = NULL;	
	void *setup_buf = NULL;

	rx_desc = dma_alloc_coherent((struct device *)NULL, (ALI_SW_DESC_SZ * ALI_SW_RX_DESC_NUM), &priv->rx_desc_dma, GFP_KERNEL);
	tx_desc = dma_alloc_coherent((struct device *)NULL, (ALI_SW_DESC_SZ * ALI_SW_TX_DESC_NUM), &priv->tx_desc_dma, GFP_KERNEL);
	setup_buf = dma_alloc_coherent((struct device *)NULL, SETUP_FRAME_SZ,
		&priv->setup_buf_dma, GFP_KERNEL);

	priv->rx_desc_dma = __CTDADDRALI(priv->rx_desc_dma);
	priv->tx_desc_dma = __CTDADDRALI(priv->tx_desc_dma);
	priv->setup_buf_dma = __CTDADDRALI(priv->setup_buf_dma);
	memset(rx_desc, 0, (ALI_SW_DESC_SZ * ALI_SW_RX_DESC_NUM));
	memset(tx_desc, 0, (ALI_SW_DESC_SZ * ALI_SW_TX_DESC_NUM));
	memset(setup_buf, 0, SETUP_FRAME_SZ);

	priv->rx_desc = (p_rx_desc)rx_desc;
	priv->tx_desc = (p_tx_desc)tx_desc;
	priv->setup_buf = (u8 *)setup_buf;//question, we actually don't use it...

	sw_info("%s()=> rx_desc = 0x%x, rx_desc_dma = 0x%x", __FUNCTION__, (u32)priv->rx_desc, (u32)priv->rx_desc_dma);
	sw_info("%s()=> tx_desc = 0x%x, tx_desc_dma = 0x%x", __FUNCTION__, (u32)priv->tx_desc, (u32)priv->tx_desc_dma);
}


static void p0_desc_clean(p_p0_private priv)//tbd, funcation name inpropriate
{
	unsigned i;
	p_rx_desc rx_desc;
	p_tx_desc tx_desc;

	for (i = 0; i < ALI_SW_RX_DESC_NUM; i++) {
		if (priv->rx_skb[i]) {
			rx_desc = &priv->rx_desc[i];
			dma_unmap_single((struct device *)NULL, __DTCADDRALI(rx_desc->pkt_buf_dma),
				ALI_SW_BUF_SZ, DMA_FROM_DEVICE);
			//dma_unmap_single((struct device *)NULL, rx_desc->pkt_buf_dma,
			dev_kfree_skb(priv->rx_skb[i]);
			priv->rx_skb[i] = NULL;
		}
	}

	for (i = 0; i < ALI_SW_TX_DESC_NUM; i++) {
		if (priv->tx_skb[i]) {
			tx_desc = &priv->tx_desc[i];
			//dma_unmap_single((struct device *)NULL, tx_desc->DataDescriptor.pkt_buf_dma,
			dma_unmap_single((struct device *)NULL, __DTCADDRALI(tx_desc->DataDescriptor.pkt_buf_dma),
				ALI_SW_BUF_SZ, DMA_TO_DEVICE);			
			dev_kfree_skb(priv->tx_skb[i]);
			priv->tx_skb[i] = NULL;
		}

	}
}

static void p0_set_perfect_filter(p_p0_private priv, bool is_mc_flag)
{
	struct net_device *dev = priv->dev;
	struct netdev_hw_addr_list * mc;
	struct netdev_hw_addr * ha;
	u8 i;
	u32 tmp_u32;

	sw_trace("own MAC address: %02x-%02x-%02x-%02x-%02x-%02x", \
				dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2], \
				dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

	SW_W8(SysPortAccess, 0);
	
	/////////////////////////////////////////////////////////////
	//step1. configure own MAC address
	tmp_u32 = 0;
	tmp_u32 = dev->dev_addr[1];
	tmp_u32 <<= 8;
	tmp_u32 |= dev->dev_addr[0];
	SW_W32(SetupFrame_IO, tmp_u32);

	tmp_u32 = 0;
	tmp_u32 = dev->dev_addr[3];
	tmp_u32 <<= 8;
	tmp_u32 |= dev->dev_addr[2];
	SW_W32(SetupFrame_IO, tmp_u32);
	
	tmp_u32 = 0;
	tmp_u32 = dev->dev_addr[5];
	tmp_u32 <<=8;
	tmp_u32 |= dev->dev_addr[4];
	SW_W32(SetupFrame_IO, tmp_u32);

	/////////////////////////////////////////////////////////////
	//step2. configure Multicast MAC address	
	mc = &dev->mc;
	if(is_mc_flag==1)
	{	//there are MC MAC address to configure
		i = 0;
		list_for_each_entry(ha, &mc->list, list) {
			sw_trace("MC MAC address(%d) = %02x-%02x-%02x-%02x-%02x-%02x", 
										i++,
										ha->addr[0], ha->addr[1],
										ha->addr[2], ha->addr[3],
										ha->addr[4], ha->addr[5]);

			tmp_u32 = 0;
			tmp_u32 = ha->addr[1];
			tmp_u32 <<= 8;
			tmp_u32 |= ha->addr[0];
			SW_W32(SetupFrame_IO, tmp_u32);

			tmp_u32 = 0;
			tmp_u32 = ha->addr[3];
			tmp_u32 <<= 8;
			tmp_u32 |= ha->addr[2];
			SW_W32(SetupFrame_IO, tmp_u32);
			
			tmp_u32 = 0;
			tmp_u32 = ha->addr[5];
			tmp_u32 <<=8;
			tmp_u32 |= ha->addr[4];
			SW_W32(SetupFrame_IO, tmp_u32);
		}
		
		for(; i<15; i++)
		{
			tmp_u32 = 0xffffffff;
			SW_W32(SetupFrame_IO, tmp_u32);
			tmp_u32 = 0xffffffff;
			SW_W32(SetupFrame_IO, tmp_u32);
			tmp_u32 = 0xffffffff;
			SW_W32(SetupFrame_IO, tmp_u32);
		}
	}
	else
	{	//there is no MC MAC address to configure, so besides own MAC address, there are all "0xff"! 
		//that is 1 own mac address, 15 "0xff" mac address!
		for(i=0; i<15; i++)
		{
			tmp_u32 = 0xffffffff;
			SW_W32(SetupFrame_IO, tmp_u32);
			tmp_u32 = 0xffffffff;
			SW_W32(SetupFrame_IO, tmp_u32);
			tmp_u32 = 0xffffffff;
			SW_W32(SetupFrame_IO, tmp_u32);
		}
	}
	
	return;
}

static void p0_set_rx_mode(struct net_device *dev)//TBD, vlan also here, reference tg3.c
{
	p_p0_private priv = netdev_priv(dev);
	u32 reg_val, flags;

	//spin_lock_irqsave (&priv->cmd_lock, flags);
	CMD_LOCK_IRQ(&priv->cmd_lock, flags);
	
	SW_W8(SysPortAccess, 0);
	reg_val = SW_R32(P0FilterMode);
	reg_val &= ~(FILTER_MODE_MSK);

	//sw_trace("%s", __FUNCTION__);
	sw_info("%s, line %d, promisc? %c", __FUNCTION__, __LINE__, dev->flags&IFF_PROMISC?'Y':'N');//0907
	if(dev->flags & IFF_PROMISC) 
	{
		SW_W32(P0FilterMode, (reg_val|PASS_PROMISCUOUS|PASS_ALL_MULTICAST));
		sw_trace("%s () => PASS_PROMISCUOUS", __FUNCTION__);

	} else if ((dev->flags & IFF_ALLMULTI) || (dev->mc.count > 14)) {
		SW_W32(P0FilterMode, (reg_val|PASS_ALL_MULTICAST));
		sw_trace("%s() => PASS_ALL_MULTICAST, mc.count=%d", __FUNCTION__, dev->mc.count);

	} else if(dev->flags & IFF_MULTICAST) {
        if (priv->filter_mode == FILTER_PERFECT_MODE) {
		SW_W32(P0FilterMode, (reg_val|PASS_FILTER_PERFECT));
            if ((dev->mc.count == 0)) {
			sw_warn("%s()=>(dev->mc.count == 0\n", __FUNCTION__);
			goto multicast_done;
		}
		p0_set_perfect_filter(priv, 1);
        } else {
		sw_warn("%s(): unsupported MC filter mode!", __FUNCTION__);
        }
	}

multicast_done:
	//spin_unlock_irqrestore(&priv->cmd_lock, flags);
	CMD_UNLOCK_IRQ(&priv->cmd_lock, flags);
	sw_trace("end %s\n", __FUNCTION__);
    return;
}

static int p0_rx_buff_refill(p_p0_private priv)
{
	struct net_device *dev = priv->dev;
    struct sk_buff *skb;
	int i;
	
/****************************************************************************
	p0_rx_buff_refill is like "mac_rx_refill' in older driver,
	when bootup or whenever link changes or timeout, 
	mac_rx_refill will be called.

	the main purpose of "mac_rx_refill" is to allocated DMA buffer when bootup,
	after that, the DMA buffer is taken care by "mac_rx_update_wptr" in old driver,
	when link changes or timeout, mac_rx_refill will enter, but it only alloc rx_skb[i] which are NULL.
	
	in switch, since port0 is always linked up, 	so "p0_rx_buff_refill" 
	only allocs buffers for NULL rx_skb[i]s in the circumstances of bootup and timeout.
****************************************************************************/	

	for(i = 0; i < ALI_SW_RX_DESC_NUM; i ++)
	{
		p_rx_desc rx_desc = &priv->rx_desc[i];

		if(NULL == priv->rx_skb[i])
		{
			skb = netdev_alloc_skb(dev, ALI_SW_BUF_SZ);
			if (!skb)
				goto refill_err_out;

			//skb_reserve(skb, NET_IP_ALIGN);
			
#ifdef NEW_DMA_CACHE_API//look into dma-default.c
			rx_desc->pkt_buf_dma = virt_to_phys(skb->data);
#else			
			rx_desc->pkt_buf_dma = dma_map_single((struct device *)NULL, skb->data,
						 ALI_SW_BUF_SZ, DMA_FROM_DEVICE);
			rx_desc->pkt_buf_dma = __CTDADDRALI(rx_desc->pkt_buf_dma);
#endif
	
			priv->rx_skb[i] = skb;
#if 0
			sw_trace("desp[%d]: &priv->rx_desc[i]=0x%x, priv->rx_skb[i]=0x%x, skb->data=0x%x, pkt_buf_dma=0x%x", \
				i, &priv->rx_desc[i], priv->rx_skb[i], skb->data, rx_desc->pkt_buf_dma);
#endif	
			//note: "&priv->rx_desc[i]->pkt_buf_dma" is incorrect!
		}
	}

#if 0
	for(i = 0; i < ALI_SW_RX_DESC_NUM; i ++)
	{
		sw_trace("desp[%d]: &priv->tx_desc[i]=0x%x", i, &priv->tx_desc[i]);
	}
#endif

	return 0;

refill_err_out:
	sw_error("%s()=>fatal error, alloc skb failed.",__FUNCTION__);
	return -ENOMEM;
}

static void p0_get_start(p_p0_private priv, bool called_from_open)
{
//	UINT32 ba = priv->io_base;
	u8 tmp_8=0;
	u32 tmp_u32;
//	UINT32 pause_frame = 0;
//	int ret = 0;
	
	//if (netif_msg_ifup(priv))
	//	sw_trace("p0_get_start()...");

	SW_W8(SysPortAccess, 0);

	SW_W32(TxDesStartAddr, priv->tx_desc_dma);
	SW_W32(RxDesStartAddr, priv->rx_desc_dma);

	SW_W16(RxDesTotNum, ALI_SW_RX_DESC_NUM);
	SW_W16(TxDesTotNum, ALI_SW_RX_DESC_NUM);

	SW_W16(RxRingDesWPtr, ALI_SW_RX_DESC_NUM -1);
	SW_W16(TxRingDesWPtr, 0);
	
	if(p0_rx_buff_refill(priv)) {
		BUG();
		asm("sdbbp");
	}
	
	SW_W8(SysPortAccess, 0);
	if(p0_tx_csum)
		tmp_8 |= SCRTxCoeEn;
	if(p0_rx_csum)
		tmp_8 |= SCRRxCoeEn;
	SW_W8(SysCtlP0, tmp_8);

    /* set filter table */
	SW_W8(SysPortAccess, 0);
	tmp_u32 = SW_R32(P0FilterMode);
	sw_info("default reg[P0FilterMode]=0x%x", tmp_u32);
	tmp_u32 &= ~(FILTER_MODE_MSK);

    if (priv->filter_mode == FILTER_PERFECT_MODE) {
	SW_W32(P0FilterMode, (tmp_u32|PASS_FILTER_PERFECT));
	p0_set_perfect_filter(priv, 0);
        sw_trace("P0_MC_FILTER_MODE_IS_PERFECT");
    }

    if (priv->filter_mode == FILTER_ALLMULTI_MODE) {
	SW_W32(P0FilterMode, (tmp_u32|PASS_ALL_MULTICAST));
	p0_set_perfect_filter(priv, 0);
        sw_trace("P0_MC_FILTER_MODE_IS_ALLMULTI");
    }
	
	p0123_port_enable(0);
	sw_info("get p0 started, reg[SysCtlSwFabric] = 0x%x", SW_R16(SysCtlSwFabric));
    return;	
}


static void p0_free_rings(p_p0_private priv)//tbd, change name
{
	p0_desc_clean(priv);

	if(priv->rx_desc != NULL) {
		dma_free_coherent((struct device *)NULL, (ALI_SW_DESC_SZ * ALI_SW_RX_DESC_NUM), 
			priv->rx_desc, __DTCADDRALI(priv->rx_desc_dma));
			//priv->rx_desc, priv->rx_desc_dma);
		priv->rx_desc = NULL;
	}
	
	if(priv->tx_desc != NULL) {
		dma_free_coherent((struct device *)NULL, (ALI_SW_DESC_SZ * ALI_SW_TX_DESC_NUM), 
			priv->tx_desc, __DTCADDRALI(priv->tx_desc_dma));
			//priv->tx_desc, priv->tx_desc_dma);
		priv->tx_desc = NULL;
	}	
	
	if(priv->setup_buf != NULL) {
		dma_free_coherent((struct device *)NULL, SETUP_FRAME_SZ, 
			priv->setup_buf, __DTCADDRALI(priv->setup_buf_dma));
			//priv->setup_buf, priv->setup_buf_dma);
		priv->setup_buf = NULL;
	}


}

static void p0_cnt_init(p_p0_private priv)
{
	priv->fabric_isr= 0;
	priv->p0_isr= 0;
	
	//priv->isr_mask = 0xffffffff; 
	//priv->isr_mask = (ISRTimer|ISRLinkStatus);
	priv->acquired_isr = false;
	
	priv->rx_wptr = ALI_SW_RX_DESC_NUM -1;
	priv->rx_bptr= 0;
	priv->tx_wptr= 0;	

#if ALI_SW_VLAN_TAG_USED//question
	priv->vlan_tag_remove = false;
#endif 
}

/***********************************************************************
	priv->rx_wptr	-- rx ring buffer's wptr, software controlled
	priv->rx_rptr	-- rx ring buffer's rptr, hardware controlled
	priv->rx_bptr		-- between the wptr and rptr

	rx_wptr can't be the same as rx_rptr
	at first, rx_wptr is TOE2_RX_DESC_NUM-1, and rx_rptr is 0,
		which means 'TOE2_RX_DESC_NUM-1' number of pkts that ring buffer can hold

	you can't say rx_wptr is always bigger than rx_rptr, because it's a ring buffer
		you can only say rx_wptr is, and should always run ahead of rx_rptr,
		or you can say rx_rptr is always chasing rx_wptr.
		the distance between rx_wptr and rx_rptr indicates how many pkts can hold.
		if (rx_wptr-1) == rx_rptr, indicates the ring buffer is full, drop the new incoming pkts.

	ptoe2->rx_bptr -- 
		keeps tracking of how many pkts have been handled(handed up).
		purly a software thing, unrelated to hardware, have no idea why it's named 'rx_bptr', hehe.
		since we know the ring buffer holds the pkts,
		when the associated interrupts arosed, we receving pkts,
		which actually means we read pkts from ring buffer, and handle these pkts to Network stack.
		but there is a limit that how many pkts we can handle to Network stack,
		so we might not be able to hand up all the pkts in ring buffer,
		because there is a limit (search budget/quota/weight in dev.c),
		also because the RX routine might be interrupted while it is working.
		in any cases, rx_bptr always records when we should restarted the RX job next time
		and we can imagine that rx_bptr is always trying to catch rx_rptr.

	the relationship is like this:
		rx_bptr -->-- rx_rptr -->-- rx_wptr 
			|						|
			|---------<-------------|
		'-->--' means 'trying to catch/chasing'
	
	how to update rx_wptr?
		since it's a ring buffer, rx_wptr should be updatad like a cycle.
		everytime ISR read starting position from rx_bptr, 
			because rx_bptr means the previous position we read in the ring buffer.
		and everything ahead of rx_bptr, we don't care in driver
			although it doesn't mean these memory could be free
			it just means driver doesn't need them,
			RX memory freeing are token care by the Network stack. (TX is quite opposite)
			and Network stack would free them eventually.
		rx_wptr can be updated to the position(value) ahead of rx_bptr (can't surpass rx_bptr).
		by the word of 'updata' we mean actually two things:
			1.update the value of rx_wptr
			2.allocate memory, one MTU size for each.
				the memories we allocate don't look like ring buffer
				we just arrange these memories' pointers in a 'ring' way

	again, in driver, we only allocate memory, we don't free them,
		the action of freeing is taken by Network stack.
		when the rx_rptr updates, it means we have pkts to handle.
***********************************************************************/
static u16 p0_rx_update_wptr(p_p0_private priv)
{
	struct net_device *dev = priv->dev;
	//UINT32 ba = pgmac->io_base;
	u16 rx_bptr, rx_rptr, rx_wptr;
	u16 updata = 0;
	p_rx_desc rx_desc;
	struct sk_buff *new_skb;
	int i;

	rx_wptr = priv->rx_wptr;
	rx_bptr = priv->rx_bptr;
	
	SW_W8(SysPortAccess, 0);
	rx_rptr = SW_R16(RxRingDesRPtr);

	ali_trace(2, "RX: rx_wptr=%d, rx_bptr=%d, rx_rptr=%d", rx_wptr, rx_bptr, rx_rptr);
	
	if(rx_wptr > rx_rptr)
	{
		if((rx_bptr > rx_rptr) && (rx_bptr <= rx_wptr))
			goto rx_lost;
		else
		{
			if(rx_bptr > rx_wptr)
				updata = rx_bptr - rx_wptr -1;
			else
				updata = ALI_SW_RX_DESC_NUM + rx_bptr - rx_wptr -1;
		}
	}	
	else if(rx_wptr < rx_rptr)
	{
		if((rx_bptr > rx_rptr) ||(rx_bptr <= rx_wptr))
			goto rx_lost;	
		else
			updata = rx_bptr - rx_wptr -1;
	}
	else
	{
		if(rx_bptr > rx_wptr)
			updata = rx_bptr - rx_wptr -1;
		else if(rx_bptr < rx_wptr)
			updata = ALI_SW_RX_DESC_NUM + rx_bptr - rx_wptr -1;
		else
			goto rx_lost;	
	}
	/* the above 'if' routine seems unnecessary... */

    ali_trace(2, "RX: %s updata is %d\n", __FUNCTION__, updata);
	if(updata > 0)
	{
		i = rx_wptr;
		while(updata > 0)
		{
			new_skb = netdev_alloc_skb(dev, ALI_SW_BUF_SZ);//1518+16+2 = 1536 (ntohs(eth->h_proto) >= 1536)
		
			if (!new_skb) {
				//dev->stats.rx_dropped++;
				sw_error("new_skb==NULL, rx_dropped, i:%d, updata%d", i, updata);
				break;
			}
			
#if 0		//can't skb_reserve. question.
			//tbd, Align IP on 16 byte, skb->data += 2, skb->tail += 2;
			skb_reserve(new_skb, SKB_IP_ALIGN);
#endif				
			priv->rx_skb[i] = new_skb;
			rx_desc = &priv->rx_desc[i];

#ifdef NEW_DMA_CACHE_API
			rx_desc->pkt_buf_dma = virt_to_phys(new_skb->data);
#else
			rx_desc->pkt_buf_dma = dma_map_single((struct device *)NULL, new_skb->data, ALI_SW_BUF_SZ, DMA_FROM_DEVICE);
			rx_desc->pkt_buf_dma = __CTDADDRALI(rx_desc->pkt_buf_dma);
#endif			

			if(i == ALI_SW_RX_DESC_NUM - 1){
				i = 0;
			} else {
				i ++;
			}		
			updata --;
		}
		
		priv->rx_wptr = i;
		
		SW_W8(SysPortAccess, 0);
		SW_W16(RxRingDesWPtr, priv->rx_wptr);

	}

	return rx_rptr;

rx_lost:
	sw_warn("%s()=>rx_bptr got lost, rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.", __FUNCTION__, rx_wptr, rx_bptr, rx_rptr);
	return rx_rptr;
}

static bool p0_rx_desc_check(p_p0_private priv, p_rx_desc rx_desc, u8 *from_port_no)
{
	ppacket_head pHead;

	pHead = &(rx_desc->pkt_hdr);

	if((rx_desc->PacketLength > MAX_PKT_LEN_DWORD) || (rx_desc->PacketLength < MIN_PKT_LEN_DWORD))
	{
		sw_error("RX=>PacketLength err %d", rx_desc->PacketLength);
		return false;
	}
	
	tmp_flag = 0;
	
	if (pHead->UniFrame)
		priv->mac_stats.rx_uc++;
	if (pHead->BroFrame)
		priv->mac_stats.rx_bc++;
	if (pHead->MulFrame)
		priv->mac_stats.rx_mc++;
	if (pHead->IPFrame)
		priv->mac_stats.rx_ip++;
	if (pHead->IP6Frame)
		priv->mac_stats.rx_ipv6++;
	if (pHead->IPFrag)
		priv->mac_stats.rx_frag++;
	if (pHead->TCPpkt)
		priv->mac_stats.rx_tcp++;
	if (pHead->UDPpkt)
		priv->mac_stats.rx_udp++;
	if (!pHead->L2FrameType)
		priv->mac_stats.rx_8023++;
	if (pHead->PPPoE)
		priv->mac_stats.rx_pppoe++;
	if (pHead->SNAP)
		priv->mac_stats.rx_snap++;
	if (pHead->BPDU)
		priv->mac_stats.rx_bpdu++;
	if (pHead->IGMP_MLD)
		priv->mac_stats.rx_igmp_mld++;
	if (pHead->GMRP_MMRP)
		priv->mac_stats.rx_gmrp_mmrp++;
	if (pHead->GVRP_MVRP)
		priv->mac_stats.rx_gvrp_mvrp++;
	if (pHead->GARP)
		priv->mac_stats.rx_garp++;

	*from_port_no = pHead->PortFrame;
	(priv->mac_stats.rx_from_port[pHead->PortFrame])++;

/*
	if(pHead->PortFrame!=1){
		sw_trace("coming from nonport1, port=%d", pHead->PortFrame);
		tmp_flag = 1;
	}
*/	
	return true;	
}

/*
*	hw_cks is hardware checksum.
*/
static bool p0_rx_hw_cks_ok(p_p0_private priv, ppacket_head pHead)
{	

	if((p0_rx_csum == true) &&( (pHead->IPFrame)||(pHead->IP6Frame)))
	{
		if(pHead->IPFrag)
			goto Soft_Cks;

		if(!pHead->IP6Frame)		
			if(!pHead->IPChksum)
			{
				priv->mac_stats.rx_ip_chksum_err_from_desc++;
				sw_warn("=>ip head checksum err");
				goto Soft_Cks;
			}

		if (pHead->TCPpkt||pHead->UDPpkt)
			if(!pHead->IPDChksum)//doesn't mean TCP only, including UDP....
			{
				priv->mac_stats.rx_ip_data_chksum_err_from_desc++;
				sw_warn("=>ip %s data checksum err\n", (pHead->TCPpkt ? "tcp":"udp"));
				goto Soft_Cks;
			}

		return true;
	}

Soft_Cks:
	return false;
}

/****************************************************************************
non-NAPI
	update reg[RxRingDesWPtr] based on priv->rx_wptr, priv->rx_rptr and priv->rx_bptr.
		along with memory allocation, because 'update' acctually means make descriptors available to use,
		each RX descriptor surely indicates one piece of free memory that is reserved for the RX of newly arriving packets.
	receive packets from priv->rx_bptr to reg[RxRingDesRPtr]
	add to workqueue if it's BPDU or IGMP
	netif_rx
		put skb into input_pkt_queue (might be dropped)
		put backlog into poll_list
		__raise_softirq_irqoff(NET_RX_SOFTIRQ);
	net_rx_action
		process_backlog
			netif_receive_skb

NAPI (unsupported yet)
	disable irq
	napi_schedule
		put dev into poll_isr
		__raise_softirq_irqoff(NET_RX_SOFTIRQ);
	net_rx_action
		dev's poll fn.
			add to workqueue if it's BPDU or IGMP
			netif_receive_skb
****************************************************************************/
static int driver_icmp = 0;
static int p0_rx_pkts (p_p0_private priv, int budget) {
	p_rx_desc rx_desc;
	u16 rx_bptr, rx_rptr;
	ppacket_head	pHead;
	u16 pkt_sz, pkts, rx, i;
	struct sk_buff *skb;
	u8 from_port_no;

	rx_rptr = p0_rx_update_wptr(priv);
	
	rx_bptr = priv->rx_bptr;
	if(rx_rptr >= rx_bptr)
		pkts = rx_rptr - rx_bptr;
	else
		pkts = ALI_SW_RX_DESC_NUM + rx_rptr - rx_bptr;
	
	if((pkts > 0)&&(rx_rptr == priv->rx_wptr))
		pkts -= 1;

	if((budget != 0)&&(pkts > budget)) {
		pkts = budget;
	}

	i = rx_bptr;
	rx = 0;

    ali_trace(2, "RECV: pkts=%d, rx_rptr=%d, rx_bptr=%d", pkts, rx_rptr, rx_bptr);
		
	while(pkts > 0) {
		rx_desc = &priv->rx_desc[i];
		pHead = &(rx_desc->pkt_hdr);
		
        dma_unmap_single((struct device *)NULL, __DTCADDRALI(rx_desc->pkt_buf_dma), ALI_SW_BUF_SZ, DMA_FROM_DEVICE);

		if (p0_rx_desc_check(priv, rx_desc, &from_port_no)) {
			pkt_sz = (rx_desc->PacketLength) << 2;
			
			skb = priv->rx_skb[i];
			skb_put(skb, pkt_sz);
			priv->dev->stats.rx_packets++;
			priv->dev->stats.rx_bytes += pkt_sz;
			
			if(p0_rx_hw_cks_ok(priv, pHead)) {
				skb->ip_summed = CHECKSUM_UNNECESSARY;
			} else {
				skb->ip_summed = CHECKSUM_NONE;
			}

			if(test_bit(6, &dbg_runtime_val)){		
				if(skb->ip_summed==CHECKSUM_NONE){	
					if( (pHead->TCPpkt||pHead->UDPpkt)&&(!pHead->IPFrag)){
						sw_trace("CHECKSUM_NONE");
						p0_print_line(skb->data, skb->len);	
						p0_print(rx_desc, ALI_SW_DESC_SZ);	
					}	
				}
				if(tmp_flag==1){
					p0_print_line(skb->data, skb->len);	
					p0_print(rx_desc, ALI_SW_DESC_SZ);	
				}
			}
			

			skb->protocol = eth_type_trans(skb, priv->dev);
			skb->dev = priv->dev;

			if((((*((u8 *)(skb->data)))&0xF0) == IPV4_VERSION_VAL)&&(*((u8 *)(skb->data+IPV4_PROTOCAL_OFFSET)) == ICMP_PROTOCAL_VAL)) {
				driver_icmp++;
			}

#if ALI_SW_VLAN_TAG_USED
			if (priv->vlgrp) 
				vlan_hwaccel_receive_skb(skb, priv->vlgrp, swab16(rx_desc->vlan_tag));
			else 
#endif
			{

				ali_trace(2, "send pkts to kernel %d skb 0x%x skb->data 0x%x len %d\n",i, (u32)skb, (u32)skb->data, skb->len); 
				netif_receive_skb(skb);
			}

		} else{
			sw_error("packet_head: Head(%08x). rx_desc->pkt_buf_dma=0x%x", *(UINT32 *)pHead, rx_desc->pkt_buf_dma);
			p0_print(rx_desc, ALI_SW_DESC_SZ);
			p0_print_line(((struct sk_buff *)(priv->rx_skb[i]))->data, 64);
		}

		priv->rx_skb[i] = NULL;
		
		if(i == ALI_SW_RX_DESC_NUM - 1)
			i = 0;
		else
			i ++;

		pkts --;
		rx ++;
	}
	priv->rx_bptr = i;
	return rx;
}

static void p0_tasklet_fn(unsigned long para)
{
	if (gp_p0->fabric_isr & GP_TIMER_EXPIRED) {
		_new_ali_sw_super_housekeeper(gp_p0);
	}
}

static bool p0123_port_is_in_use(p_p0_private priv, u8 port_no);
static void p123_get_start(p_p0_private priv, u8 port_no);
void ali_sw_chip_rst (void);
void handle_exception_isr (p_p0_private priv) {
	u8 port_no = 0;
	ali_sw_chip_rst();	
	for(port_no=1; port_no<MAX_PORT_NUM; port_no++){
		if(!p0123_port_is_in_use(priv, port_no)) {
			continue;	
		}
		if (port_no == 2) {
			p123_get_start(priv, port_no);
		}
	}
}

static int sw_napi_poll (struct napi_struct *napi, int budget) {
	p_p0_private priv = container_of(napi, struct p0_private, napi);
	int work_done; 
	int to_lock = 1;
	work_done = p0_rx_pkts(priv, budget);
	p0_free_tx_skb(priv, to_lock);
	if (work_done < budget) {
		napi_complete(napi);
		ali_sw_irq_enable(1);
	}
	return work_done;
}

static int p0_tx_context_des_cfg(p_p0_private priv, p_tx_desc desc, struct sk_buff *skb)
{
	struct net_device *dev = priv->dev;
	struct skb_shared_info *skb_info = skb_shinfo(skb);
	u16 gso_type = skb_info->gso_type;
	tx_ctx_desc tmp_desc;
	
	memset(desc, 0, ALI_SW_DESC_SZ);
	
	desc->ContextDescriptor.OWN = 1;
	desc->ContextDescriptor.ContextData = 1;

	/*set desc->ContextDescriptor.MSS*/
	if(gso_type & SKB_GSO_TCPV4)
	{
		desc->ContextDescriptor.MSS = skb_info->gso_size; 
	}
	else if (gso_type & SKB_GSO_UDP)
	{
		desc->ContextDescriptor.MSS  = dev->mtu + 14;//actually, it is MFL, when the packet is UDP.
	}
	else if(gso_type & SKB_GSO_TCPV6)//peter, SKB_GSO_TCPV6? same with SKB_GSO_TCPV4?
	{
		desc->ContextDescriptor.MSS = skb_info->gso_size; 
	}
	else
	{	
		desc->ContextDescriptor.MSS = dev->mtu + 14;
	}

	if (skb->protocol == htons(ETH_P_IP))
	{
		struct iphdr *ipHeader = NULL;

		ipHeader = ip_hdr(skb); 
		//desc->ContextDescriptor.L3HeaderLen= (u8)((u8 *)ip - skb->data); 
		desc->ContextDescriptor.L3HeaderLen = (ipHeader->ihl*4);
		desc->ContextDescriptor.L2HeaderLen = 14;
		desc->ContextDescriptor.L3Type = 0;

		if(ipHeader->protocol == IPPROTO_TCP) 
		{
			desc->ContextDescriptor.L4Type = 0;
			desc->ContextDescriptor.L4HeaderLen= tcp_hdrlen(skb);//20;	//peter, fixed?
		}
		else if(ipHeader->protocol == IPPROTO_UDP) 
		{
			desc->ContextDescriptor.L4Type = 1; 
			desc->ContextDescriptor.L4HeaderLen= 8; 	
		}
		else 
		{ 
			sw_warn("%s()=>Pkt type unrecognized while configing context.", __FUNCTION__); 
		}				
	}
	else if(skb->protocol == htons(ETH_P_IPV6))
	{
		struct ipv6hdr *ipv6Header = NULL;

		u8 ipv6nxthdr_type = 0;
		u8 loop = 0;
		//u32 ipv6exthdr_len_sin = 0;
		u8* pNxthdr = NULL;

		//ipv6 = ipv6_hdr(skb); 
		ipv6Header = (struct ipv6hdr *)skb_network_header(skb);
		//ipv6 =  (struct ipv6hdr *)skb->network_header;
		//desc->ContextDescriptor.L3HeaderLen= (u8)(skb->transport_header - skb->network_header); 
		desc->ContextDescriptor.L3HeaderLen= 40; //question, fixed? not include ext hdr?
		desc->ContextDescriptor.L2HeaderLen = 14;
		desc->ContextDescriptor.L3Type = 1;

		//ipv6Header = skb->network_header;
		ipv6Header = (struct ipv6hdr *)skb_network_header(skb);
		ipv6nxthdr_type = (u8)(ipv6Header->nexthdr);
		//ipv6nxthdr_type = *(u8 *)((u8 *)skb_network_header(skb)+6);
		pNxthdr = (u8 *)skb->network_header + 40;
		
		//peter, should be changed in the future to support AH, ESP...
		while((ipv6nxthdr_type!=IPPROTO_TCP)&&(ipv6nxthdr_type!=IPPROTO_UDP))
		{
			loop++;
			ipv6nxthdr_type = *(u8 *)pNxthdr;//to find TCP or UDP header...
			pNxthdr+= ((*(u8 *)(pNxthdr+1)+1)*8);
			if(loop>=7){
				//printk("TSO/UFO error: couldn't find TCP/UDP header\n");
				break;
			}
		}

		desc->ContextDescriptor.L3HeaderLen = pNxthdr - (u8 *)(skb->network_header);

		if(ipv6nxthdr_type == IPPROTO_TCP) 
		{
			desc->ContextDescriptor.L4Type = 0;
			desc->ContextDescriptor.L4HeaderLen = tcp_hdrlen(skb);//peter, right?
			
		}
		else if(ipv6nxthdr_type == IPPROTO_UDP) 
		{
			desc->ContextDescriptor.L4Type = 1; 
			desc->ContextDescriptor.L4HeaderLen= 8; 
			//udpipv6fragid++;
		}
		else 
		{ 
			sw_warn("%s()=>Pkt type unrecognized while configing context\n", __FUNCTION__);
		}
		
		//skb_shared_info->gso_size = 1428, which is 1514 - 14 - 40 - 32(tcp_hdr) = 1428				
	}
	else if(skb->protocol == htons(ETH_P_PPP_DISC))
	{
		sw_error("%s, skb->protocol=0x%x not support yet!", __FUNCTION__, skb->protocol);
		return -1;
	}
	else if(skb->protocol == htons(ETH_P_PPP_SES))
	{
		sw_error("%s, skb->protocol=0x%x not support yet!", __FUNCTION__, skb->protocol);
		return -1;
	}
	else
	{
		sw_error("%s, skb->protocol=0x%x not support yet!", __FUNCTION__, skb->protocol);
		return -1;
	}

	
#if ALI_SW_VLAN_TAG_USED //peter, tbd
	if (priv->vlgrp && vlan_tx_tag_present(skb)) 
	{ 
		
		desc->ContextDescriptor.VlanTag= swab16(vlan_tx_tag_get(skb)); 
	} 
#endif 


		tmp_desc.L3Type = desc->ContextDescriptor.L3Type;
		tmp_desc.L4Type = desc->ContextDescriptor.L4Type;
		tmp_desc.L2HeaderLen = desc->ContextDescriptor.L2HeaderLen;
		tmp_desc.L3HeaderLen = desc->ContextDescriptor.L3HeaderLen;
		tmp_desc.L4HeaderLen = desc->ContextDescriptor.L4HeaderLen;
		tmp_desc.MSS = desc->ContextDescriptor.MSS;
	memcpy(&tmp_desc, desc, sizeof(tmp_desc));	
	if (desc->ContextDescriptor.MSS == 0xffff) {
		printk("hello\n");
	}
	if (desc->ContextDescriptor.L4Type == 1) {
		;
	}
//#ifdef ALI_SW_DEBUG1
	if(test_bit(3, &dbg_runtime_val))
	{
		//trace_tx_l1(" tx_context_desc[%d]'s info, off(%d), desc_num(%d), desc@(0x%x):\n", off, off, desc_num, (UINT32)desc);
		trace_tx_l1("	ContextDes.L3Type = %d", desc->ContextDescriptor.L3Type);
		trace_tx_l1(" 	ContextDes.L4Type = %d", desc->ContextDescriptor.L4Type);
		trace_tx_l1(" 	ContextDes.L2HeaderLen = %d", desc->ContextDescriptor.L2HeaderLen);
		trace_tx_l1(" 	ContextDes.L3HeaderLen = %d", desc->ContextDescriptor.L3HeaderLen);
		trace_tx_l1(" 	ContextDes.L4HeaderLen = %d", desc->ContextDescriptor.L4HeaderLen);
//		trace_tx_l1(" 	ContextDes.MSS = %d", tmp_desc.MSS);
	}
//#endif

	return 0;
}


static void p0_tx_data_des_part_cfg(p_p0_private priv, u16 off, p_tx_desc desc, struct sk_buff *skb, void *seg_addr, u16 seg_len)
{
	tx_data_desc tmp_desc;
#ifdef NEW_DMA_CACHE_API
	desc->DataDescriptor.pkt_buf_dma = virt_to_phys(seg_addr);
	dma_cache_wback(seg_addr, seg_len);
#else
	desc->DataDescriptor.pkt_buf_dma = dma_map_single((struct device *)NULL, seg_addr, seg_len, DMA_TO_DEVICE);
	desc->DataDescriptor.pkt_buf_dma = __CTDADDRALI(desc->DataDescriptor.pkt_buf_dma);
#endif

	if(skb)//peter, could be used as an indepentend function.
	{
		//priv->tx_skb[off] = skb; 

		if (skb->ip_summed == CHECKSUM_PARTIAL) //
		{
			if(p0_tx_csum)
			{ 
				desc->DataDescriptor.CoeEn = 1;
				desc->DataDescriptor.TOE_UFO_En = 1; //peter, or should be 0 here?
			} 
			else 
			{ 
				sw_error("%s()=>Tx cks not enabled.", __FUNCTION__); 
				desc->DataDescriptor.CoeEn = 0;
				desc->DataDescriptor.TOE_UFO_En = 0;
				BUG(); 
			} 
			//sw_trace("really?");
		} 		
		else
		{
			desc->DataDescriptor.CoeEn = 0;
			desc->DataDescriptor.TOE_UFO_En = 0;
		}

		
#if ALI_SW_VLAN_TAG_USED 
		if (priv->vlgrp && vlan_tx_tag_present(skb)) 
		{ 
			desc->DataDescriptor.VlanEn = 1; 
		} 
		else 
#endif 
			desc->DataDescriptor.VlanEn= 0; 	
	}
	
	tmp_desc.SegmentNum = desc->DataDescriptor.SegmentNum;
	tmp_desc.ForwardPorts = desc->DataDescriptor.ForwardPorts;
    tmp_desc.TOE_UFO_En = desc->DataDescriptor.TOE_UFO_En;
	tmp_desc.VlanEn = desc->DataDescriptor.VlanEn;
	tmp_desc.CoeEn = desc->DataDescriptor.CoeEn;
	tmp_desc.FS = desc->DataDescriptor.FS;
	tmp_desc.EOR = desc->DataDescriptor.EOR;
	tmp_desc.ContextIndex = desc->DataDescriptor.ContextIndex;
	tmp_desc.ContextData = desc->DataDescriptor.ContextData;
	tmp_desc.OWN = desc->DataDescriptor.OWN;
	tmp_desc.pkt_buf_dma = desc->DataDescriptor.pkt_buf_dma;
	tmp_desc.seg_len = desc->DataDescriptor.seg_len;
	tmp_desc.tot_len = desc->DataDescriptor.tot_len;
	tmp_desc.FragmentID = desc->DataDescriptor.FragmentID;	
	if (desc->DataDescriptor.pkt_buf_dma == 0xfffffff) {
		printk("hello\n");
	}
	memcpy(&tmp_desc, desc, sizeof(tmp_desc));	
// #ifdef ALI_SW_DEBUG1
#if 1
	if(test_bit(3, &dbg_runtime_val))
	{
	trace_tx_l1("	desc->DataDescriptor.SegmentNum = %d", desc->DataDescriptor.SegmentNum);
		trace_tx_l1("	desc->DataDescriptor.TOE_UFO_En = %d", desc->DataDescriptor.TOE_UFO_En);
		//trace_tx_l1("		desc->DataDescriptor.VlanEn = %d", desc->DataDescriptor.VlanEn);
		trace_tx_l1("	desc->DataDescriptor.CoeEn = %d", desc->DataDescriptor.CoeEn);
		trace_tx_l1("	desc->DataDescriptor.FS = %d", desc->DataDescriptor.FS);
		//trace_tx_l1("		desc->DataDescriptor.EOR = %d", desc->DataDescriptor.EOR);
		trace_tx_l1("	desc->DataDescriptor.ContextIndex = %d", desc->DataDescriptor.ContextIndex);
		//trace_tx_l1("		desc->DataDescriptor.ContextData = %d", desc->DataDescriptor.ContextData);
		trace_tx_l1("	desc->DataDescriptor.OWN = %d", desc->DataDescriptor.OWN);
		trace_tx_l1("	desc->DataDescriptor.pkt_buf_dma = 0x%x", desc->DataDescriptor.pkt_buf_dma);
		trace_tx_l1("	desc->DataDescriptor.seg_len = %d", desc->DataDescriptor.seg_len);
		trace_tx_l1("	desc->DataDescriptor.tot_len = %d", desc->DataDescriptor.tot_len);
		trace_tx_l1("	desc->DataDescriptor.FragmentID = %d", desc->DataDescriptor.FragmentID);	
		trace_tx_l1("	desc->DataDescriptor.ForwardPorts = %d", desc->DataDescriptor.ForwardPorts);
		p0_print(desc, ALI_SW_DESC_SZ);	
	}
	if(test_bit(13, &dbg_runtime_val))
		p0_print(desc, ALI_SW_DESC_SZ);	
#endif
//#endif	

}
void dump_all_sw_tx_skb(p_p0_private priv) {
	int i ;
	for (i=0; i < ALI_SW_TX_DESC_NUM; i++) {
		dump_sw_tx_skb(priv, i);
	}
	return;
}

void dump_sw_tx_skb(p_p0_private priv, int index) {
	printk("sw_tx_skb[%d].first %d\n",index, priv->sw_tx_skb[index].first);
	printk("sw_tx_skb[%d].cnt %d\n", index, priv->sw_tx_skb[index].cnt);
	printk("sw_tx_skb[%d].start %d\n", index, priv->sw_tx_skb[index].start);
	printk("sw_tx_skb[%d].skb %x\n", index, (u32)priv->sw_tx_skb[index].skb);
}

static int p0_start_xmit(struct sk_buff *skb, struct net_device *dev) {
	UINT32 flags;
	volatile p_tx_desc desc;
	volatile u16 tx_wptr;
	u16 desc_num;
	volatile u16 off, first, start;
	/* fragIdx, fragment index:  means fragments in memory, not IP fragments */
	int fragIdx; 
	u8 context_id=0;
	void *seg_addr;
	u8 forward_ports=0;
	tx_data_desc tmpbuf;
	struct skb_shared_info *skb_info = skb_shinfo(skb);
	volatile p_p0_private priv = netdev_priv(dev);
	u16 need_desc_num;
	u32 have_context_desc;
	int i=0, j = 0;
	int zm_r, zm_w;
#ifdef TX_SYNC_DMA
	volatile u16 tx_rptr;
#endif

	if(test_bit(3, &dbg_runtime_val)) {
		trace_tx_l1("\nXMIT: skb->len=%d, skb_info->gso_size=%d, shinfo->gso_type=0x%x, \
					 skb->ip_summed=%x, skb->protocol=0x%x", \
					 skb->len, skb_shinfo(skb)->gso_size, skb_shinfo(skb)->gso_type,\
					 skb->ip_summed, skb->protocol);
	}

	if(test_bit(7, &dbg_runtime_val)) {
		p0_print_line(skb->data, skb->len);
	}

	if(skb_is_gso(skb) || skb_is_gso_v6(skb)) {
		if(skb_header_cloned(skb)) {
			if(pskb_expand_head(skb, 0, 0, GFP_ATOMIC)) {	
				BUG();
				return -ENOMEM;
			}
		}
	}
	
	CMD_LOCK_IRQ(&priv->p_switch->lock, flags);
	/* free tx desc before xmit */
	p0_free_tx_skb(priv, 0);
	off = priv->tx_wptr;
	tx_wptr = priv->tx_wptr;

	desc_num = skb_shinfo(skb)->nr_frags + 1; 
	if(p0_tx_csum && (skb->ip_summed == CHECKSUM_PARTIAL)) {
		need_desc_num = desc_num + 1; /* add context desc */ 
	} else {
		need_desc_num = desc_num;
	}
	if(test_bit(3, &dbg_runtime_val)) {
		trace_tx_l1("get switch lock tx_skb_rd %d tx_skb_wr %d tx_wptr %d desc_num %d\n",priv->tx_skb_rd, priv->tx_skb_wr, tx_wptr, desc_num);
	}

	if (priv->avail_desc_num < need_desc_num) {
		ali_trace(3, "###dma busy, wptr:%d, avail:%u desc_num+1 %u\n", tx_wptr, priv->avail_desc_num, desc_num+1);
		ali_trace(3, "###dma busy, tx_skb_rd %d tx_skb_wr %d\n", priv->tx_skb_rd, priv->tx_skb_wr);
		//netif_stop_queue(dev); 
		//dump_all_sw_tx_skb(priv);
		CMD_UNLOCK_IRQ(&priv->p_switch->lock, flags);
		return NETDEV_TX_BUSY;
	}

	start = off;
	have_context_desc = 0;

	//1. configure the context descriptor...
	if(p0_tx_csum && (skb->ip_summed == CHECKSUM_PARTIAL)) {
		if(test_bit(3, &dbg_runtime_val)) {
			trace_tx_l1("==>1. configure the context descriptor...");
		}

		desc = &priv->tx_desc[off];
		if(p0_tx_context_des_cfg(priv, desc, skb) < 0) {
			BUG();
			CMD_UNLOCK_IRQ(&priv->p_switch->lock, flags);
			return -1;
		}
		if((++off) >= ALI_SW_TX_DESC_NUM) {
			off = 0;
		}
		have_context_desc = 1;
	} else {
		if(test_bit(3, &dbg_runtime_val)) {
			trace_tx_l1("==>1. no need to configure the context descriptor...");
		}
	}

	//2. configure the data descriptor...
	//fist means first data descriptor, not context descriptor.
	first = off;
	forward_ports = test_forwarding_ports;

	if(skb_shinfo(skb)->nr_frags == 0) {
		if(test_bit(3, &dbg_runtime_val)) {
			trace_tx_l1("==>2. configure the data descriptor... skb_shinfo(skb)->nr_frags=0, skb->len=%d", skb->len);

		}
		desc = &priv->tx_desc[off];
		memset(desc, 0, ALI_SW_DESC_SZ);

		if(off == ALI_SW_TX_DESC_NUM -1) {
			desc->DataDescriptor.EOR = 1;
		}
		desc->DataDescriptor.seg_len = skb->len;
		desc->DataDescriptor.tot_len = skb->len;
		desc->DataDescriptor.OWN = 1;
		desc->DataDescriptor.ContextData = 0;
		desc->DataDescriptor.FS = 1;
		desc->DataDescriptor.SegmentNum = 1;
		desc->DataDescriptor.TX_PRIOR_EN = 0;
		desc->DataDescriptor.ForwardPorts = forward_ports;
		desc->DataDescriptor.ContextIndex = context_id;
		desc->DataDescriptor.FragmentID = skb_shinfo(skb)->ip6_frag_id;
		
		p0_tx_data_des_part_cfg(priv, off, desc, skb, skb->data, skb->len);

		if((++off) >= ALI_SW_TX_DESC_NUM) {
			off = 0;
		}
	} else { //skb_shinfo(skb)->nr_frags >= 1
		if(test_bit(3, &dbg_runtime_val)) {
			trace_tx_l1("==>2. configure the data descriptor......, nr_frags=%d", skb_shinfo(skb)->nr_frags);
		}
		//1. configure the first data descriptor...
		desc = &priv->tx_desc[off];
		memset(desc, 0, ALI_SW_DESC_SZ);
		
		if(off == ALI_SW_TX_DESC_NUM -1) {
			desc->DataDescriptor.EOR = 1;
		}
		desc->DataDescriptor.seg_len = skb_headlen(skb);
		desc->DataDescriptor.tot_len = skb->len;
		desc->DataDescriptor.OWN = 1;
		desc->DataDescriptor.ContextData = 0;
		desc->DataDescriptor.FS = 1;
		desc->DataDescriptor.SegmentNum = desc_num;//skb_shinfo(skb)->nr_frags + 1
		desc->DataDescriptor.TX_PRIOR_EN = 0;
		desc->DataDescriptor.ForwardPorts = forward_ports;
		desc->DataDescriptor.ContextIndex = context_id;
		desc->DataDescriptor.FragmentID = skb_shinfo(skb)->ip6_frag_id;

		p0_tx_data_des_part_cfg(priv, off, desc, skb, skb->data, skb_headlen(skb));//note, it's skb_headlen(skb)!

		if((++off) >= ALI_SW_TX_DESC_NUM) {
			off = 0;
		}
		
		//2. configure the other data descriptor...
		for (fragIdx = 0; fragIdx < skb_shinfo(skb)->nr_frags; fragIdx++) 
		{
			skb_frag_t *this_frag = &skb_shinfo(skb)->frags[fragIdx];
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0))
			seg_addr = (void *)page_address(skb_frag_page(this_frag)) + this_frag->page_offset;
#else
			seg_addr = (void *)page_address(this_frag->page) + this_frag->page_offset;
#endif

			desc = &priv->tx_desc[off];
			memset(desc, 0, ALI_SW_DESC_SZ);

			if(off == ALI_SW_TX_DESC_NUM -1) {
				desc->DataDescriptor.EOR = 1;
			}
			desc->DataDescriptor.seg_len = this_frag->size;
			desc->DataDescriptor.tot_len = skb->len;
			desc->DataDescriptor.OWN = 1;
			desc->DataDescriptor.ContextData = 0;
			//desc->DataDescriptor.FS = 0;
			desc->DataDescriptor.TX_PRIOR_EN = 0;
			desc->DataDescriptor.ForwardPorts = forward_ports;
			//if(fragIdx == (skb_shinfo(skb)->nr_frags - 1))
			//	desc->DataDescriptor.LS = 0;//question, should it be 1?
			desc->DataDescriptor.SegmentNum = desc_num;
			p0_tx_data_des_part_cfg(priv, off, desc, skb, seg_addr, this_frag->size);
			if((++off) >= ALI_SW_TX_DESC_NUM) {
				off = 0;
			}
		}
		
	}	
	wmb();

	memcpy(&tmpbuf, desc, sizeof(tx_data_desc));;
	memcpy(&tmpbuf, desc, sizeof(tx_ctx_desc));;
	CMD_LOCK(&priv->cmd_lock); 
	SW_W8(SysPortAccess, 0);
	zm_r = SW_R16(TxRingDesRPtr);
	zm_w = SW_R16(TxRingDesWPtr);

	SW_W16(TxRingDesWPtr, off); 
	priv->tx_wptr = off;
	do {
		SW_W8(SysPortAccess, 0);
		SW_W16(TxRingDesWPtr, off); 
		if (j == 0) {
			i++;
			if (i > 50) {
				j = 1;
				SW_W8(SysPortAccess, 0);
				printk("Rptr %d, Wptr %d off %d\n", SW_R16(TxRingDesRPtr),\
						SW_R16(TxRingDesWPtr), off);
			}
		}
	} while (SW_R16(TxRingDesWPtr) != priv->tx_wptr);
#ifndef TX_SYNC_DMA
	CMD_UNLOCK(&priv->cmd_lock);
#else /* SYNC DMA, just for debug */
	i = 0, j = 0;
	do {
		SW_W8(SysPortAccess, 0);
		tx_rptr = SW_R16(TxRingDesRPtr);
		udelay(10);
		if (j == 0) {
			++i;
			if (i > 10000) {
				j = 1;
				printk("xmit tx_wptr %d tx_rptr %d\n", priv->tx_wptr, tx_rptr);
				SW_W8(SysPortAccess, 0);
				printk("Rptr %d, Wptr %d\n", SW_R16(TxRingDesRPtr), \
						SW_R16(TxRingDesWPtr));
				printk("WARNING, loop, break it\n");
			}
		}
	} while(priv->tx_wptr != tx_rptr);
	if (j == 1) {
		printk("skip ...");
		SW_W8(SysPortAccess, 0);
		printk("Rptr %d, Wptr %d\n", SW_R16(TxRingDesRPtr), \
						SW_R16(TxRingDesWPtr));
	}
	CMD_UNLOCK(&priv->cmd_lock,flags);
	priv->dev->stats.tx_packets++;
	if(test_bit(3, &dbg_runtime_val))
		trace_tx_l1("==>4. clean stuff");
	if(skb) {
		desc = &priv->tx_desc[first];
		dev_kfree_skb(skb);
		do
		{
			if(desc->DataDescriptor.seg_len> 0)
			{
				dma_unmap_single((struct device *)NULL, __DTCADDRALI(desc->DataDescriptor.pkt_buf_dma), desc->DataDescriptor.seg_len, DMA_TO_DEVICE);
			}
			else
			{
				BUG();
			}
			
			if((++first) >= ALI_SW_TX_DESC_NUM)
			{
				first = 0;
			}
			desc = &priv->tx_desc[first];
			desc_num --;

		}while(desc_num > 0);
	}
	else
	{
		BUG();
	}
#endif	
	priv->sw_tx_skb[priv->tx_skb_wr].first = first;
	priv->sw_tx_skb[priv->tx_skb_wr].cnt = desc_num + have_context_desc;
	priv->sw_tx_skb[priv->tx_skb_wr].skb = skb;
	priv->sw_tx_skb[priv->tx_skb_wr].start = start;
	priv->avail_desc_num -= priv->sw_tx_skb[priv->tx_skb_wr].cnt;
	if (priv->avail_desc_num < 0) {
		sw_error("something error, priv->avail_desc_num < 0");
		dump_all_sw_tx_skb(priv);
	}

	if(test_bit(3, &dbg_runtime_val)) {
		trace_tx_l1("tx_skb_wr %d first %d cnt %d skb %x start %d\n",\
			priv->tx_skb_wr, first, desc_num+have_context_desc, (u32)skb, start);
	}
//	dump_sw_tx_skb(priv, priv->tx_skb_wr);
	priv->tx_skb_wr++;
	if (priv->tx_skb_wr == ALI_SW_TX_DESC_NUM) {
		priv->tx_skb_wr = 0;
	}
	if(test_bit(5, &dbg_runtime_val)) {
		verify_sw_tx_skb(priv);
	}
	CMD_UNLOCK_IRQ(&priv->p_switch->lock, flags);
	if(test_bit(3, &dbg_runtime_val)) {
		trace_tx_l1("unlock switch lock\n");
	}
	return NETDEV_TX_OK;
}


/****************************************************************************
	p123 related implements, primarily focusing on controlling...
****************************************************************************/
//#define ALI_SW_MDIO_DEBUG
static int mac_mdio_read(p_p0_private priv, int phy_addr, int reg_addr)
{

	u16 tmp_16;
	UINT32 tmp_32;
	UINT32 addr;

	SW_W8(SysPortAccess, 4);
	
	addr = ((phy_addr << MdioPhyAddrOffset) & MdioPhyAddrMsk)|((reg_addr << MdioRegAddrOffset) & MdioRegAddrMsk);
	
	tmp_32 = SW_R32(MdioCtl);
	tmp_32 &= ~(MdioPhyAddrMsk|MdioRegAddrMsk|MdioOperMsk);
	tmp_32|=(addr|MdioReadOper);
	
	SW_W8(SysPortAccess, 4);
	SW_W32(MdioCtl, (tmp_32|MdioHWTansStart));

	ali_trace(7, "%s, phy_addr=%d, reg_addr=%d, addr=0x%x, tmp_32=0x%x, reg[MdioCtl]=0x%x, reg[LinkCheckCtl]=0x%x", \
				__FUNCTION__, phy_addr, reg_addr, addr, tmp_32, SW_R32(MdioCtl), SW_R32(LinkCheckCtl));
	ali_trace(7, "reading.");

	do{
		SW_W8(SysPortAccess, 4);
		tmp_32 = SW_R32(MdioCtl);		
		mdelay(1);
		ali_trace(7, ".");
	}while(tmp_32&MdioHWTansStart);

	ali_trace(7, "\n");

	SW_W8(SysPortAccess, 4);
	tmp_16 = SW_R16(MdioRData);
	
	return (UINT32)tmp_16;

}

static void mac_mdio_write (p_p0_private priv, int phy_addr, int reg_addr, int value)
{

	UINT32 tmp_32;
	UINT32 addr;

	SW_W8(SysPortAccess, 4);
	
	SW_W16(MdioWData, (u16)value);
		
	addr = ((phy_addr << MdioPhyAddrOffset) & MdioPhyAddrMsk)|((reg_addr << MdioRegAddrOffset) & MdioRegAddrMsk);
	
	SW_W8(SysPortAccess, 4);
	tmp_32 = SW_R32(MdioCtl);
	tmp_32 &= ~(MdioPhyAddrMsk|MdioRegAddrMsk|MdioOperMsk);
	tmp_32|=(addr|MdioWriteOper);
	
	ali_trace(7, "%s, phy_addr=%d, reg_addr=%d, addr=0x%x, tmp_32=0x%x, reg[MdioCtl]=0x%x, reg[LinkCheckCtl]=0x%x", \
				__FUNCTION__, phy_addr, reg_addr, addr, tmp_32, SW_R32(MdioCtl), SW_R32(LinkCheckCtl));
	ali_trace(7, "write.");

	SW_W8(SysPortAccess, 4);
	SW_W32(MdioCtl, (tmp_32|MdioHWTansStart));
			
	do{
		SW_W8(SysPortAccess, 4);
		tmp_32 = SW_R32(MdioCtl);
	}while(tmp_32&MdioHWTansStart);
		
}

bool new_p123_phy_link_chk(struct net_ali_sw_port *p) {
	p_p0_private priv = p->p_P0;//TBD, seems redundant
		
	u16 advertisement, link;
	unsigned long	pause;
	unsigned long asm_dir;
	unsigned long	partner_pause;
	unsigned long partner_asm_dir;
	unsigned short expansion = 0;
	u16 status, ctrl;
	u16 giga_status;
	u16 giga_ctrl;
    /* 0x0 */
	ctrl = (u16) mac_mdio_read(priv, port_phy_addr[p->port_no], PhyBasicModeCtrl);
    /* 0x1 */
	status = (u16) mac_mdio_read(priv, port_phy_addr[p->port_no], PhyBasicModeStatus);
	/* reg 4 advertisement */
	advertisement = (u16)mac_mdio_read(priv, port_phy_addr[p->port_no], PhyNWayAdvert);
	/* reg 6 */
	expansion = (u16)mac_mdio_read(priv, port_phy_addr[p->port_no], PhyNWayExpansion);
	/* reg 5 link partner ability*/
	link = (u16)mac_mdio_read(priv, port_phy_addr[p->port_no], PhyNWayLPAR);

	/* reg 9 1000baseT control */
	giga_ctrl = (u16)mac_mdio_read(priv, port_phy_addr[p->port_no], Phy1000BaseTControl);
	/* reg 0x1 1000 Base T Status */
	giga_status = (u16)mac_mdio_read(priv, port_phy_addr[p->port_no], Phy1000BaseTStatus);
    ali_trace(7, "port %d status %x advertisement %x expansion %x link %x giga_ctrl %x giga_status %x\n",\
            p->port_no, status, advertisement, expansion, link, giga_ctrl, giga_status) 
	/* auto nego and complete nego */
	if ((ctrl & BMCRANEnable) && (status & BMSRANComplete)) {
		//used for LinkStatusChg check.	
		p->link_partner = link; 
		if (expansion & 0x1) { /* partner support auto nego */
			if ((giga_ctrl & T_1000_FD) && (giga_status & LP_1000_FD)) {
				printk("1000 Base-TX full duplex.");
				p->link_dup = true;
				p->link_spd = LINK_SPEED_1000M;
			} else if ((giga_ctrl & T_1000_HD) && (giga_status & LP_1000_HD)) {
				printk("1000 Base-TX half duplex.");
				p->link_dup = false;
				p->link_spd = LINK_SPEED_1000M;
			} else if ((advertisement & ANARTXFD) && (link & ANARTXFD)) {
				printk("100 Base-TX full duplex.");
				p->link_dup = true;
				p->link_spd = LINK_SPEED_100M;
			} else if ((advertisement & ANARTX) && (link & ANARTX)) {
				printk("100 Base-TX half duplex.");
				p->link_dup = false;
				p->link_spd = LINK_SPEED_100M;
			} else if ((advertisement & ANAR10FD) && (link & ANAR10FD)) {
				printk("10 Base-TX full duplex.");
				p->link_dup = true;
				p->link_spd = LINK_SPEED_10M;
			} else {
				printk("10 Base-TX half duplex.");
				p->link_dup = false;
				p->link_spd = LINK_SPEED_10M;
			}
		} else { /* partner doesn't support auto nego */
			if (link & ANARTXFD) {
				p->link_dup = true;
				p->link_spd = LINK_SPEED_100M;
				printk("100 Base-TX full duplex.");
			} else if (link & ANARTX) {
				p->link_dup = false;
				p->link_spd = LINK_SPEED_100M;
				printk("100 Base-TX half duplex.");
			} 
			else if (link & ANAR10FD) {
				p->link_dup = true;
				p->link_spd = LINK_SPEED_10M;
				printk("10 Base-TX full duplex.");
			} else if (link & ANAR10) {
				p->link_dup = false;
				p->link_spd = LINK_SPEED_10M;
				printk("10 Base-TX half duplex.");
			} else {
				printk("link check doesn't get corrent speed set 10M half\n");
				p->link_dup = false;
				p->link_spd = LINK_SPEED_10M;
			}
		}
	} else if (!(ctrl & BMCRANEnable)) {
		if (ctrl & BMCRDuplexMode) {
			p->link_dup = true;
		} else {
			p->link_dup = false;
		}
		if (!(ctrl & BMCRSpeedSet6) && (ctrl & BMCRSpeedSet13)) {
			p->link_spd = LINK_SPEED_100M;
            printk("100 Base-TX %s duplex.", ((p->link_dup) ? "full" : "half"));
		} else if (!(ctrl & BMCRSpeedSet13) && !(ctrl & BMCRSpeedSet6)) {
			p->link_spd = LINK_SPEED_10M;
            printk("10 Base-TX %s duplex.", ((p->link_dup) ? "full" : "half"));
		} else {
			p->link_spd = LINK_SPEED_10M;
			printk("%s can't get valid speed, set to 10M\n", __FUNCTION__);
		}
	} else {
		printk("error, shouldn't call %s\n", __FUNCTION__);
		return -1;
	}

	pause = (advertisement & ANARPause) > 0 ? 1 : 0;
	asm_dir = (advertisement & ANARASMDIR) > 0 ? 1 : 0;
	partner_pause = (link & ANARPause) > 0 ? 1 : 0;
	partner_asm_dir = (link & ANARASMDIR) > 0 ? 1 : 0;

	printk(" Pause&ASM=%ld%ld%ld%ld. ", pause, asm_dir, partner_pause, partner_asm_dir);

	if ((pause == 0) && (asm_dir == 0))
	{
		p->rx_pause= false;
		p->tx_pause = false;		
	}
	else if ((pause == 0) && (partner_pause == 0))
	{
		p->rx_pause = false;
		p->tx_pause = false;		
	}
	else if ((pause == 0) && (asm_dir == 1) && (partner_pause == 1) && (partner_asm_dir == 0))
	{
		p->rx_pause = false;
		p->tx_pause = false;		
	}
	else if ((pause == 0) && (asm_dir == 1) && (partner_pause == 1) && (partner_asm_dir == 1))
	{
		p->rx_pause = false;
		p->tx_pause = true;		
	}
	else if ((pause == 1) && (asm_dir == 0) && (partner_pause == 0))
	{
		p->rx_pause = false;
		p->tx_pause = false;		
	}
	else if ((pause == 1) && (partner_pause == 1))
	{
		p->rx_pause = true;
		p->tx_pause = true;		
	}
	else if ((pause == 1) && (asm_dir == 1) && (partner_pause == 0) && (partner_asm_dir == 0))
	{
		p->rx_pause = false;
		p->tx_pause = false;		
	}
	else if ((pause == 1) && (asm_dir == 1) && (partner_pause == 0) && (partner_asm_dir == 1))
	{
		p->rx_pause = true;
		p->tx_pause = false;		
	}
	else
	{
		printk("%s: impossiable!\n", __FUNCTION__);
	}

	printk("Tx flowctl=%ld, Rx flowctl=%ld.\n", (long)p->rx_pause, (long)p->tx_pause);
	return 0;
}

static int p0123_set_mac_addr(p_p0_private priv)
{
	struct net_ali_sw *sw=priv->p_switch;
	struct net_ali_sw_port *p;
	u32 mac_addr = 0;
	u8 i, j;

	p = &(sw->port_list[0]);
	p->mac_addr = p0_mac_addr;

	SW_W8(SysPortAccess, 1);
	mac_addr =  ((p1_mac_addr[3]<<24)|(p1_mac_addr[2]<<16)|(p1_mac_addr[1]<<8)|p1_mac_addr[0]);
	SW_W32(MacPhyAddrP123, mac_addr);
	mac_addr =  ((p1_mac_addr[5]<<8)|p1_mac_addr[4]);
	SW_W32(MacPhyAddrP123 - 4, mac_addr);
	p = &(sw->port_list[1]);
	p->mac_addr = p1_mac_addr;	
	
	SW_W8(SysPortAccess, 2);
	mac_addr =  ((p2_mac_addr[3]<<24)|(p2_mac_addr[2]<<16)|(p2_mac_addr[1]<<8)|p2_mac_addr[0]);
	SW_W32(MacPhyAddrP123, mac_addr);
	mac_addr =  ((p2_mac_addr[5]<<8)|p2_mac_addr[4]);
	SW_W32(MacPhyAddrP123 - 4, mac_addr);
	p = &(sw->port_list[2]);
	p->mac_addr = p2_mac_addr;	
	
	SW_W8(SysPortAccess, 3);
	mac_addr =  ((p3_mac_addr[3]<<24)|(p3_mac_addr[2]<<16)|(p3_mac_addr[1]<<8)|p3_mac_addr[0]);
	SW_W32(MacPhyAddrP123, mac_addr);
	mac_addr =  ((p3_mac_addr[5]<<8)|p3_mac_addr[4]);
	SW_W32(MacPhyAddrP123 - 4, mac_addr);
	p = &(sw->port_list[3]);
	p->mac_addr = p3_mac_addr;	
	
	sw_info("ali switch ports mac address:");
	for(i=0; i<MAX_PORT_NUM; i++)	{
		printk("  port[%d]: ", i);
		for(j=0; j<ETH_ALEN; j++)	{
			if(j==ETH_ALEN-1)
				printk("%02x\n", *(u8 *)((u8 *)port_mac_addr[i]+j));
			else
				printk("%02x-", *(u8 *)((u8 *)port_mac_addr[i]+j));
		}
	}
	printk("ali STB mac address: ");
	for(j=0; j<ETH_ALEN; j++)	{
		if(j==ETH_ALEN-1)
			printk("%02x\n", stb_mac_addr[j]);
		else
			printk("%02x-", stb_mac_addr[j]);
	}

/*
	SW_W8(SysPortAccess, 0);
	sw_info("MacPhyAddrP0Low=0x%x, MacPhyAddrP0High=0x%x", SW_R32(MacPhyAddrP0), SW_R32(MacPhyAddrP0 - 4));	
*/

	SW_W8(SysPortAccess, 1);
	//sw_info("MacPhyAddrP1High=0x%x, MacPhyAddrP1Low=0x%x", SW_R32(MacPhyAddrP123 - 4), SW_R32(MacPhyAddrP123));	
	SW_W8(SysPortAccess, 2);
	//sw_info("MacPhyAddrP2High=0x%x, MacPhyAddrP2Low=0x%x", SW_R32(MacPhyAddrP123 - 4), SW_R32(MacPhyAddrP123));	
	SW_W8(SysPortAccess, 3);
	//sw_info("MacPhyAddrP3High=0x%x, MacPhyAddrP3Low=0x%x", SW_R32(MacPhyAddrP123 - 4), SW_R32(MacPhyAddrP123));	
	
	return 0;
}

static bool p0123_port_disable(u8 port_no)
{
	u16 tmp_16;
	
	SW_W32(SysPortAccess, 4);
	tmp_16 = SW_R16(SysCtlSwFabric);

	if(port_no==0)
	{	
		tmp_16&=~SCRPort0En;
	}
	else if(port_no==1)
		tmp_16&=~SCRPort1En;
	else if(port_no==2)
		tmp_16&=~SCRPort2En;
	else if(port_no==3)
		tmp_16&=~SCRPort3En;
	else
	{	
		sw_error("%s: wrong port_no input!\n", __FUNCTION__);
		return false;
	}
	SW_W32(SysPortAccess, 4);
	SW_W16(SysCtlSwFabric, tmp_16);

/*
	//question, do we need to reset it? 
	if(port_no!=0)
	{
		SW_W8(SysPortAccess, port_no);
		SW_W8(SysCtlP123, SCRSoftRstP123|SW_R8(SysCtlP123));	
		do{
			tmp_16 = SW_R16(SysCtlP123);
		}while(tmp_16 & SysCtlP123);					
	}
*/	

#ifdef FPGA_PHY_MAC_SET
	u32 tmp;
	if(port_no==2||port_no==3){
		SW_W8(SysPortAccess, 4);
		tmp = SW_R32(0x188);
		
		if(port_no==2)
			tmp&=~FPGA_PORT2_PHY_RMII_EN;
		else
			tmp&=~FPGA_PORT3_PHY_RMII_EN;

		SW_W32(0x188, tmp);
	}	
#endif
	
	return true;
}

static bool p0123_port_enable(u8 port_no)
{
	u16 tmp_16;

	SW_W8(SysPortAccess, 4);
	//sw_trace("%s, before, reg[SysCtlSwFabric]=0x%x", __FUNCTION__, SW_R32(SysCtlSwFabric));
	SW_W8(SysPortAccess, port_no);
	//sw_trace("%s, before, port[%d]: reg[SysCtlP123]=0x%x", __FUNCTION__, port_no, SW_R32(SysCtlP123));

	SW_W8(SysPortAccess, 4);
	tmp_16 = SW_R16(SysCtlSwFabric);
	
	if(port_no==0)
	{	
		tmp_16|=SCRPort0En;
		//tmp_16|=SCRPort0TxEn;
	}
	else if(port_no==1)
		tmp_16|=SCRPort1En;
	else if(port_no==2)
		tmp_16|=SCRPort2En;
	else if(port_no==3)
		tmp_16|=SCRPort3En;
	else
	{	
		sw_error("%s: wrong port_no input=%d!\n", __FUNCTION__, port_no);
		return false;
	}

	SW_W8(SysPortAccess, 4);
	SW_W16(SysCtlSwFabric, tmp_16);
	//sw_trace("%s, after, reg[SysCtlSwFabric]=0x%x", __FUNCTION__, SW_R32(SysCtlSwFabric));
	SW_W8(SysPortAccess, 4);
	SW_W8(SysPortAccess, port_no);
	//sw_trace("%s, after, port[%d]: reg[SysCtlP123]=0x%x", __FUNCTION__, port_no, SW_R32(SysCtlP123));
	return true;//tbd, the return value is ugly...
}

static bool p0123_port_is_in_use(p_p0_private priv, u8 port_no)
{
	struct net_ali_sw *sw=priv->p_switch;
	return (sw->port_list[port_no].got_phy);
}

static void p0123_phy_link_state_init(p_p0_private priv)
{
	//struct net_device *dev = priv->dev;
	struct net_ali_sw *sw=priv->p_switch;
	u8	i;
	
	for(i=1; i<MAX_PORT_NUM; i++)//i is the port_no
	{
		sw->port_list[i].got_phy = 0;
		sw->port_list[i].phy_rst = 0;
//		sw->port_list[i].link_state = LINK_DISCONNECT;
		sw->port_list[i].link_spd = LINK_SPEED_RESERVED;
		sw->port_list[i].link_dup = LINK_FULL_DUPLEX;
		sw->port_list[i].link_established = 0;
		sw->port_list[i].transmit_okay = LINK_DISCONNECT;
		sw->port_list[i].link_partner = 0;
	}
	
	sw->port_list[0].got_phy = 1;
	sw->port_list[0].phy_rst = 1;
	sw->port_list[0].link_spd = LINK_SPEED_1000M;
	sw->port_list[0].link_dup = LINK_FULL_DUPLEX;
	sw->port_list[0].link_established = 1;
	sw->port_list[0].transmit_okay = LINK_OKAY_TO_TRANSMIT;
	sw->port_list[0].link_partner = 0;

	sw->ports_link_status = 0;

	//tbd, should use "true/false" instead of "1/0".
}

static bool p123_phy_detect(p_p0_private priv)
{
	//struct net_device *dev = priv->dev;
	struct net_ali_sw *sw=priv->p_switch;
	
	unsigned char			phy_id;
	unsigned short		mode_control_reg;
	unsigned short		status_reg;
	u8	i, j, cnt=0;
	u32 reg_val;

#if 0//self-diagnosis: enable HW auto polling
	sw_info("---------- %s:  enable HW auto polling ----------", __FUNCTION__);
	SW_W8(SysPortAccess, 4);
	reg_val=SW_R32(LinkCheckCtl);
	reg_val|=(AutoMdioEn);
	SW_W32(LinkCheckCtl, reg_val);
	//sw_info("reg[LinkCheckCtl]=0x%x, reg[SysCtlSwFabric]=0x%x", SW_R32(LinkCheckCtl), SW_R16(SysCtlSwFabric));

	SW_W32(GETimer, p0_timer_frq);
	printk("waiting for HW auto polling..., reg(GETimer)=0x%x\n", SW_R32(GETimer));

	for(j=0; j<0; j++)
	{
		mdelay(1000);
		if(j%5==0)
			for(i=1; i<4; i++)
			{
				SW_W8(SysPortAccess, i);
				sw_info("#%d# Port[%d]'s reg[PortMonitor]=0x%x", j, i, SW_R32(PortMonitor));
			}
	}
	
	mdelay(1000);
	SW_W8(SysPortAccess, 4);
	//sw_info("reg[LinkCheckCtl]=0x%x", SW_R32(LinkCheckCtl));
	//sw_info("reg[LinkCheckStat]=0x%x", SW_R32(LinkCheckStat));

	for(i=1; i<4; i++)
	{
		SW_W8(SysPortAccess, i);
		sw_info("Port[%d]'s reg[PortMonitor]=0x%x", i, SW_R32(PortMonitor));
	}
#endif

	//sw_info("---------- %s:  disable HW auto polling ----------", __FUNCTION__);
	SW_W8(SysPortAccess, 4);
	reg_val=SW_R32(LinkCheckCtl);
	reg_val&=(~AutoMdioEn);
	SW_W8(SysPortAccess, 4);
	SW_W32(LinkCheckCtl, reg_val);
	//sw_info("reg[LinkCheckCtl]=0x%x", SW_R32(LinkCheckCtl));
	//sw_info("reg[LinkCheckStat]=0x%x", SW_R32(LinkCheckStat));

	for(i=1; i<MAX_PORT_NUM; i++)//i is the port_no
	{
		mode_control_reg = 0;
		status_reg = 0;
		phy_id = 0;

		sw->port_list[i].got_phy = 1;//assuming we got phy.

		mode_control_reg = (unsigned short)mac_mdio_read(priv, port_phy_addr[i], PhyBasicModeCtrl);
		status_reg = (unsigned short)mac_mdio_read(priv, port_phy_addr[i], PhyBasicModeStatus);
		
		if ((status_reg != 0xffff) && (status_reg != 0))
		{
			sw_info("%s: port[%d] phy address[%d] is right. PhyBasicModeCtrl=0x%x, PhyBasicModeStatus=0x%x", \
						__FUNCTION__, i, port_phy_addr[i], mode_control_reg, status_reg);
			continue;
		}

		sw_error("%s: port[%d] phy address is wrong. :(", __FUNCTION__, i);
		sw->port_list[i].got_phy = 0;//we didn't get phy.
		cnt++;
		
	}


#ifdef NEW_PHY_TEST
	//int j;
	for(i=1; i<MAX_PORT_NUM; i++)//i is the port_no
	{
		mode_control_reg = 0;
		status_reg = 0;
		phy_id = 0;
		
		for(j=0;j<32;j++){
			mode_control_reg = (unsigned short)mac_mdio_read(priv, j, PhyBasicModeCtrl);
			status_reg = (unsigned short)mac_mdio_read(priv, j, PhyBasicModeStatus);
			
			if (((mode_control_reg != 0xffff)&&(status_reg != 0xffff))&&\
				((mode_control_reg != 0)&&(status_reg != 0)))
			{
				sw_info("%s: port[%d] phy address[%d] found. PhyBasicModeCtrl=0x%x, PhyBasicModeStatus=0x%x", \
							__FUNCTION__, i, j, mode_control_reg, status_reg);
				continue;
			}
		}

	
	}
#endif

	if(cnt==MAX_PORT_NUM-1)
	{	
		sw_error("%s: no phy detected on any port!Freaking wired!", __FUNCTION__);
		return false;
	}

	return true;
}

static bool p123_phy_rst(p_p0_private priv, u8 port_no) {
	struct net_ali_sw *sw=priv->p_switch;
	u16 	tmp_u16;
	u32     tmp_u32;

	if(!p0123_port_is_in_use(priv, port_no)) {
		return false;
	}
	tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], PhyBasicModeCtrl);
	if (port_no != 1) {
		mac_mdio_write(priv, port_phy_addr[port_no], PhyBasicModeCtrl, BMCRReset|tmp_u16);
		do
		{	
			mdelay(5);
			printk("read phy port %d status \n", port_no);
			tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], PhyBasicModeCtrl);
		} while(tmp_u16 & BMCRReset);
	}

#if 0
	if (port_no == 1) {
		printk("giga phy rx \n");
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x00);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1e, 0x2ee);
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1e);
		printk("port %d: 1e %x\n",port_no, tmp_u16);
	}
#endif 
	if (port_no == 2) {

		printk("disable all FSM\n");
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x10);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x10, tmp_u16|0x1);

		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x08);
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1e);
		tmp_u16 &= 0xfc7f;
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1e, tmp_u16);

		printk("set phy sock\n");
		tmp_u32 = (*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0x6c)); 
		tmp_u32 |= 0x2;
		*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0x6c) = tmp_u32; 
		mdelay(1);

		tmp_u32 = (*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0x6c)); 
		tmp_u32 &= 0xfffffffd;
		*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0x6c) = tmp_u32; 
		mdelay(4);

		printk("set port 2 ephy AGC\n");
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x07);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1f, 0x4151);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x00);

		/* to fix can't link with asus sw */
		printk("Fix asus \n");
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x8);
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1f);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1f, (tmp_u16 | 0x1));
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1f);
		printk("PHY %d 0x1f %x\n", port_no, tmp_u16);

		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x09);
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1e);
		tmp_u16 &= 0xFFFC;
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1e, (tmp_u16 | 0x2));
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1e);
		printk("PHY %d 0x1e %x\n", port_no, tmp_u16);
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1c);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1c, (tmp_u16 | 0x7));
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1c);
		printk("PHY %d 0x1c %x\n", port_no, tmp_u16);

		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x6);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1e, 0x0104);

		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x10, 0);
	}
    if (port_no == 3) {
        printk("power down port 3 !!!\n");
        tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], PhyBasicModeCtrl);
		mac_mdio_write(priv, port_phy_addr[port_no], PhyBasicModeCtrl, BMCRPowerDown|tmp_u16);
    }
	sw->port_list[port_no].phy_rst= true;//phy_reset is set after phy_detect and phy_set!
	return true;
}

static void p123_set_linkcheck_mode(p_p0_private priv)
{
	UINT32 tmp=0;
	u8 i;

	SW_W8(SysPortAccess, 4);

	for(i=1; i<MAX_PORT_NUM; i++)
	{
		if(i==1)
		{
			tmp|=P1PhyActive;
			//tmp|=P1PhyType;
			tmp|=(port_phy_addr[1]<<P1PhyAddrOffset);
		}
		else if(i==2)
		{
			tmp|=P2PhyActive;
			//tmp|=P2PhyType;
			tmp|=(port_phy_addr[2]<<P2PhyAddrOffset);
		}
		else if(i==3)
		{
			tmp|=P3PhyActive;
			//tmp|=P3PhyType;
			tmp|=(port_phy_addr[3]<<P3PhyAddrOffset);
		}
		else
			BUG();
	}

#ifdef HW_AUTO_MDIO_POLLING
	tmp|=AutoMdioEn;
	sw_info(DRV_NAME "HW_AUTO_MDIO_POLLING is enabled!\n");
#endif

	SW_W8(SysPortAccess, 4);
	SW_W32(LinkCheckCtl, tmp);
}

/*
*	p123_phy_test
*	-- it just test the phy/link status under HW AUTO MDIO mode
*/
static int p123_phy_test(p_p0_private priv)
{

//	struct net_device *dev = priv->dev;
	struct net_ali_sw *sw=priv->p_switch;
	//int ret;
	UINT32 tmp;
//	u8 i;

	if(sw == NULL)
	{
		sw_error("gp_sw should not be NULL!\n");
	}

	SW_W8(SysPortAccess, 4);
	tmp = SW_R32(LinkCheckCtl);
	sw_trace("%s: reg[LinkCheckCtl] = 0x%x", __FUNCTION__, SW_R32(LinkCheckCtl));
	sw_trace("Enable AUTO MDIO? %s", ((tmp&AutoMdioEn) )? "true":"false");
	sw_trace("Port1 PHY active?	%s", ((tmp&P1PhyActive) )? "true":"false");
	sw_trace("Port1 PHY type?	%s", ((tmp&P1PhyType))? "10/100M/1000M":"10/100");
	sw_trace("Port1 PHY address?	%d", (tmp>>P1PhyAddrOffset)&P123PhyAddrMsk);	
	sw_trace("Port2 PHY active?	%s", ((tmp&P2PhyActive) )? "true":"false");
	sw_trace("Port2 PHY type?	%s", ((tmp&P2PhyType))? "10/100M/1000M":"10/100");
	sw_trace("Port2 PHY address?	%d", (tmp>>P2PhyAddrOffset)&P123PhyAddrMsk);
	sw_trace("Port3 PHY active?	%s", ((tmp&P3PhyActive) )? "true":"false");
	sw_trace("Port3 PHY type?	%s", ((tmp&P3PhyType))? "10/100M/1000M":"10/100");
	sw_trace("Port3 PHY address?	%d", (tmp>>P3PhyAddrOffset)&P123PhyAddrMsk);	

	//tmp = SW_R32(LinkCheckStat);
    return 0;
}

static bool p123_port_rst(u8 port_no)
{
	u16 tmp_16;
	
	if((port_no!=0)&&(port_no<MAX_PORT_NUM))
	{
		SW_W8(SysPortAccess, port_no);
		SW_W8(SysCtlP123, SCRSoftRstP123|SW_R8(SysCtlP123));	
		do{
			SW_W8(SysPortAccess, port_no);
			tmp_16 = SW_R16(SysCtlP123);
		}while(tmp_16 & SysCtlP123);					
	}
	else if(port_no!=0)
	{	
		sw_error("%s: wrong port_no input!\n", __FUNCTION__);
		return false;
	}
	
	return true;
}

static void p123_mac_set(p_p0_private priv, u8 port_no)
{
	struct net_ali_sw *sw=priv->p_switch;
	UINT32 speed, tmp, duplex_mode, pause_frame=0;
	u32 mac_addr;

//ClkDelayChainSR? TxRxCR1?
	SW_W8(SysPortAccess, port_no);

	//-----------------------------------------------------------------
	//step1. set MAC address, 
	/*	yes, we have 'p0123_set_mac_addr' in the beginning, 
		but here, just in case port's register are reseted.
	*/
	if(port_no>0 && port_no<MAX_PORT_NUM)//honestly, this code does look ugly....but whatsoever.
	{
		SW_W8(SysPortAccess, port_no);
		mac_addr =  ((sw->port_list[port_no].mac_addr[3]<<24)|(sw->port_list[port_no].mac_addr[2]<<16)\
					|(sw->port_list[port_no].mac_addr[1]<<8)|sw->port_list[port_no].mac_addr[0]);

		SW_W8(SysPortAccess, port_no);
		SW_W32(MacPhyAddrP123, mac_addr);
		mac_addr =  ((sw->port_list[port_no].mac_addr[5]<<8)|sw->port_list[port_no].mac_addr[4]);

		SW_W8(SysPortAccess, port_no);
		SW_W32(MacPhyAddrP123 - 4, mac_addr);
		
		//sw_info("port_no(%d), MacPhyAddrHigh=0x%x, MacPhyAddrLow=0x%x", port_no, SW_R32(MacPhyAddrP123 - 4), SW_R32(MacPhyAddrP123));	
	}
	else
	{
		sw_error("%s: unsupported port_no input!", __FUNCTION__);
		BUG();
	}
	//sw_trace("port[%d]'s default reg[RmiiCR]=0x%x", port_no, SW_R32(RmiiCR));
	//sw_trace("port[%d]'s default reg[NetOpMode]=0x%x", port_no, SW_R32(NetOpMode));
	//sw_trace("port[%d]'s default reg[TxRxCR1]=0x%x, reg[TxRxCR2]=0x%x", port_no, SW_R32(TxRxCR1), SW_R32(TxRxCR2));
	//-----------------------------------------------------------------
	//step2. set MII reg
	SW_W8(SysPortAccess, port_no);
	tmp = SW_R32(RmiiCR);
	tmp &= ~(RmiiEn |RgmiiEn);
//	SW_W32(RmiiCR, tmp);
//	mdelay(1);
		
	if (p123_phy_mode[port_no] == ETH_PHY_MODE_RGMII)
	{
		tmp|=RgmiiEn;
		SW_W8(SysPortAccess, port_no);
		SW_W32(RmiiCR, (tmp));
		sw_trace("RGMII port_no=%d, tmp=0x%x", port_no, tmp);
		sw_trace("RGMII port_no=%d, Reg(RmiiCR)=0x%x", (u32)SW_R32(RmiiCR));
	}
	else if(p123_phy_mode[port_no] == ETH_PHY_MODE_RMII)
	{
		SW_W8(SysPortAccess, port_no);
		SW_W32(RmiiCR, (tmp |RmiiEn));//actually it doesn't look at this, always mii.  tbd.
		sw_trace("RMII port_no=%d, reg[RmiiCR]=0x%x\n", port_no, SW_R32(RmiiCR));
	}
	else
	{
		sw_error("unsupported MII mode!\n");
		BUG();
	}
	
	//sw_trace("%s, port[%d]: reg[RmiiCR]=0x%x", __FUNCTION__, port_no, SW_R32(RmiiCR));
	if(p123_phy_mode[port_no] == ETH_PHY_MODE_RMII)
		while(((SW_R32(RmiiCR)&RmiiEn) == 0))
		{
			SW_W8(SysPortAccess, port_no);
			SW_W32(RmiiCR, (tmp |RmiiEn));
			mdelay(1000);
		}
	if(p123_phy_mode[port_no] == ETH_PHY_MODE_RGMII)
		while(((SW_R32(RmiiCR)&RgmiiEn) == 0))
		{
			SW_W8(SysPortAccess, port_no);
			SW_W32(RmiiCR, (tmp |RgmiiEn));
			mdelay(1000);
			//if(printk_ratelimit())
			//	printk(".");
		}
	sw_trace("port_no=%d, reg[RmiiCR]=0x%x", port_no, SW_R32(RmiiCR));
	
	//printk("\n");//question here, takes two time?
	//sw_trace("%s, port[%d]: reg[RmiiCR]=0x%x", __FUNCTION__, port_no, SW_R32(RmiiCR));

	//-----------------------------------------------------------------
	//step2. set NET operation mode.
	if (sw->port_list[port_no].link_spd == LINK_SPEED_1000M)
	{
		if(p123_phy_mode[port_no] != ETH_PHY_MODE_RGMII)
		{
			sw_error("uncorrect MII mode!\n");
			BUG();
		}
		speed = (UINT32)(SW_SPEED_1000MBPS);	//1000Mbps
	}
	else if (sw->port_list[port_no].link_spd == LINK_SPEED_100M)
	{
		speed = (UINT32)(SW_SPEED_100MBPS); //100Mbps
	}
	else if (sw->port_list[port_no].link_spd == LINK_SPEED_10M)
	{
		speed = SW_SPEED_10MBPS; //10mbps
	}
	else
	{
		sw_error("link_speed undetected yet, shouldn't be here!\n");
		BUG();
	}
		
	if (sw->port_list[port_no].link_dup)
		duplex_mode = (UINT32)FullDuplexMode;
	else
		duplex_mode = (UINT32)0;

	if (sw->port_list[port_no].rx_pause)
		pause_frame |= (UINT32)RxFlowCtlEn;
	if (sw->port_list[port_no].tx_pause)
		pause_frame |= (UINT32)TxFlowCtlEn;
	
	tmp = NetworkOMConfig|duplex_mode|speed|pause_frame;
	SW_W8(SysPortAccess, port_no);
	SW_W32(NetOpMode, tmp);
	//SW_W32(NetOpMode, (duplex_mode |NetworkOMConfig));// |WorkMode_LoopBack));
	//sw_trace("%s, port[%d]: reg[NetOpMode]=0x%x", __FUNCTION__, port_no, SW_R32(NetOpMode));

#ifdef FPGA_PHY_MAC_SET
	if(port_no==2||port_no==3){
		SW_W8(SysPortAccess, 4);
		tmp = SW_R32(0x188);
		
		if (sw->port_list[port_no].link_spd == LINK_SPEED_100M){
			if(port_no==2)
				tmp|=FPGA_PORT2_PHY_SPEED;
			else
				tmp|=FPGA_PORT3_PHY_SPEED;
		}else{
			if(port_no==2)
				tmp&=~FPGA_PORT2_PHY_SPEED;
			else
				tmp&=~FPGA_PORT3_PHY_SPEED;
		}
		
		if (sw->port_list[port_no].link_dup){
			if(port_no==2)
				tmp|=FPGA_PORT2_PHY_FULL_DUPLEX;
			else
				tmp|=FPGA_PORT3_PHY_FULL_DUPLEX;
		}else{
			if(port_no==2)
				tmp&=~FPGA_PORT2_PHY_FULL_DUPLEX;
			else
				tmp&=~FPGA_PORT3_PHY_FULL_DUPLEX;
		}

		if(port_no==2)
			tmp|=FPGA_PORT2_PHY_RMII_EN;
		else
			tmp|=FPGA_PORT3_PHY_RMII_EN;

		tmp|=(FPGA_RMII_PHY_SET<<FPGA_PORT2_RMII_PHY_OFFSET);
		tmp|=(FPGA_RMII_PHY_SET<<FPGA_PORT3_RMII_PHY_OFFSET);

		SW_W32(0x188, tmp);
		
	}
#endif
	
}

static void p123_get_start(p_p0_private priv, u8 port_no)
{
	//ali_sw_get_stats(priv->dev);
		
	//step1. port reset..	
	p123_port_rst(port_no);

	//step2. setting p123's mac.	
	p123_mac_set(priv, port_no);
	
	//step3. enable RX/TX.	
	p0123_port_enable(port_no);

	SW_W8(SysPortAccess, port_no);
	SW_W8(SysPortAccess, 4);

	sw_trace("port[%d] get started, reg[SysCtlP123]=0x%x, reg[SysCtlSwFabric]=0x%x", port_no, SW_R32(SysCtlP123), SW_R32(SysCtlSwFabric));
}

/****************************************************************************
-->	net_device_ops "ali_sw_dev_ops" implements.
****************************************************************************/
static int ali_sw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	p_p0_private priv = netdev_priv(dev);
	int rc=0;
	unsigned long flags;
	struct sw_reg_io *p_reg_io;
	struct sw_alr_io *p_alr_io;
    struct alr_rdata alr_rentry;
    char forward_ports_string[5]={0};
    u8 idx;
    u32 mac_addr;
    struct alr_wdata alr_wentry;

#if 1
	if (!netif_running(dev)){	
		sw_warn("please try to use 'ifconifg' to open the NIC first!");
		sw_warn("common setting is like: ifconfig eth0 192.168.1.11 netmask 255.255.255.0\n");
		return -EINVAL;
	}
#endif	
	
/*
	sw_trace("------->>%s, pid %i(%s): cmd=0x%x, rq=0x%x; in_softirq()? %c, in_irq()? %c.", \
		__FUNCTION__, current->pid, current->comm, cmd, rq, in_softirq()?'Y':'N', in_irq()?'Y':'N');
*/		
	if ((cmd >= SIOCDEVPRIVATE) && (cmd <= SIOCDEVPRIVATE + 15))
	{
	//	spin_lock_irqsave(&priv->cmd_lock, flags);
		CMD_LOCK_IRQ(&priv->cmd_lock, flags);
		switch (cmd)
		{
			case SIOCDEVPRIVATE+1://readreg
			{
				p_reg_io = (struct sw_reg_io *)&(rq->ifr_ifru);
				if(p_reg_io->mux == 0)
				{
					if (p_reg_io->addr > SW_PORT0_REGS_RANGE)
					{
						rc = -EOPNOTSUPP;
						printk("sw_ioctl: requested register(0x%x) is out of range!\n", p_reg_io->addr);
						break;
					}
				}
				else if(p_reg_io->mux == 4)
				{
					if (p_reg_io->addr > SW_FABRIC_REGS_RANGE)
					{
						rc = -EOPNOTSUPP;
						printk("sw_ioctl: requested register(0x%x) is out of range!\n", p_reg_io->addr);
						break;
					}
				}
				else if((p_reg_io->mux > 0) && (p_reg_io->mux < 4))
				{
					if (p_reg_io->addr > SW_PORT123_REGS_RANGE)
					{
						rc = -EOPNOTSUPP;
						printk("sw_ioctl: requested register(0x%x) is out of range!\n", p_reg_io->addr);
						break;
					}
				}
				else
				{
					rc = -EOPNOTSUPP;
					printk("sw_ioctl: requested acess(0x%x) is out of range!\n", p_reg_io->mux);
					break;
				}

				if(p_reg_io->addr!=0x1f8)
					SW_W8(SysPortAccess, p_reg_io->mux);
					
				p_reg_io->val = SW_R32(p_reg_io->addr);

				//sw_trace("mux=%d, addr=0x%x, val=0x%x", p_reg_io->mux, p_reg_io->addr, p_reg_io->val)
				
				break;
			}

			case SIOCDEVPRIVATE+2://writereg
			{
				p_reg_io = (struct sw_reg_io *)&(rq->ifr_ifru);
				if(p_reg_io->mux == 0)
				{
					if (p_reg_io->addr > SW_PORT0_REGS_RANGE)
					{
						rc = -EOPNOTSUPP;
						printk("sw_ioctl: requested register(0x%x) is out of range!\n", p_reg_io->addr);
						break;
					}
				}
				else if(p_reg_io->mux == 4)
				{
					if (p_reg_io->addr > SW_FABRIC_REGS_RANGE)
					{
						rc = -EOPNOTSUPP;
						printk("sw_ioctl: requested register(0x%x) is out of range!\n", p_reg_io->addr);
						break;
					}
				}
				else if((p_reg_io->mux > 0) && (p_reg_io->mux < 4))
				{
					if (p_reg_io->addr > SW_PORT123_REGS_RANGE)
					{
						rc = -EOPNOTSUPP;
						printk("sw_ioctl: requested register(0x%x) is out of range!\n", p_reg_io->addr);
						break;
					}
				}
				else
				{
					rc = -EOPNOTSUPP;
					printk("sw_ioctl: requested acess(0x%x) is out of range!\n", p_reg_io->mux);
					break;
				}

				SW_W8(SysPortAccess, p_reg_io->mux);
				SW_W32(p_reg_io->addr, p_reg_io->val);
				p_reg_io->val = SW_R32(p_reg_io->addr);
				break;
			}
			
			case SIOCDEVPRIVATE+3://readalr
			{
				p_alr_io = (struct sw_alr_io *)&(rq->ifr_ifru);
				
				if(p_alr_io->idx==SW_ALR_SHOW_ALL_ENTRY)
				{
					ali_sw_alr_show_all_entries(&alr_rentry);
				}
				else
				{
					if(p_alr_io->idx>68)
					{	
						sw_error("sw_ioctl: entery_idx(%d) error!", p_alr_io->idx);
						rc = -EOPNOTSUPP;
						break;
					}		
					
					alr_rentry.entery_idx = p_alr_io->idx;
					rc = ali_sw_alr_read_entry(&alr_rentry);
					if(rc==0)
					{
						printk("index(%d): %02x-%02x-%02x-%02x-%02x-%02x to ", alr_rentry.entery_idx, \
											HIBYTE(LOWORD(alr_rentry.mac_addr_high16)),\
											LOBYTE(LOWORD(alr_rentry.mac_addr_high16)),\
											HIBYTE(HIWORD(alr_rentry.mac_addr_low32)),\
											LOBYTE(HIWORD(alr_rentry.mac_addr_low32)),\
											HIBYTE(LOWORD(alr_rentry.mac_addr_low32)),\
											LOBYTE(LOWORD(alr_rentry.mac_addr_low32)));				

						forward_ports_string[3] = (alr_rentry.ports&1)+'0';
						forward_ports_string[2] = ((alr_rentry.ports>>1)&1)+'0';
						forward_ports_string[1] = ((alr_rentry.ports>>2)&1)+'0';
						forward_ports_string[0] = ((alr_rentry.ports>>3)&1)+'0';
						
						printk("%s, ", forward_ports_string);
						
						printk("%s, %s, %s\n", \
							alr_rentry.filter?"filter_out":"filter_in", alr_rentry.static_entry?"static":"non-static", alr_rentry.valid?"valid":"invalid");
						
						ali_sw_alr_format_display(&alr_rentry, 0);
						
					}
				}
				
				break;
			}
			
			case SIOCDEVPRIVATE+4://writealr, question, what happens if i write, the hw is also writing?
			{
				p_alr_io = (struct sw_alr_io *)&(rq->ifr_ifru);
/*				
				alr_entry.mac_addr_high32 = ((p_alr_io->mac_addr[0]<<24)|(p_alr_io->mac_addr[1]<<16)|\
										(p_alr_io->mac_addr[2]<<8)|(p_alr_io->mac_addr[3]));
				alr_entry.mac_addr_low16 = ((p_alr_io->mac_addr[4]<<8)|(p_alr_io->mac_addr[5]));
*/
				memset(&alr_wentry, 0, sizeof(alr_wentry));
				mac_addr = p_alr_io->mac_addr_high32;
				alr_wentry.mac_addr_high16 = mac_addr>>16;
				mac_addr<<=16;
				mac_addr|= p_alr_io->mac_addr_low16;
				alr_wentry.mac_addr_low32 = mac_addr;
				
				if(p_alr_io->ports>0xf)
				{	
					sw_error("sw_ioctl: ports(0x%x) error!", p_alr_io->ports);
					rc = -EOPNOTSUPP;
					break;
				}
				if((p_alr_io->filter_flag!=0)&&(p_alr_io->filter_flag!=1))
				{
					sw_error("sw_ioctl: filter_flag(0x%x) error!", p_alr_io->filter_flag);
					rc = -EOPNOTSUPP;		
					break;
				}
				if((p_alr_io->static_flag!=0)&&(p_alr_io->static_flag!=1))
				{
					sw_error("sw_ioctl: static_flag(0x%x) error!", p_alr_io->static_flag);
					rc = -EOPNOTSUPP;	
					break;
				}
				if((p_alr_io->valid!=0)&&(p_alr_io->valid!=1))
				{
					sw_error("sw_ioctl: valid(0x%x) error!", p_alr_io->valid);
					rc = -EOPNOTSUPP;	
					break;
				}
				
				alr_wentry.ports = p_alr_io->ports;
				alr_wentry.filter = p_alr_io->filter_flag;
				alr_wentry.static_entry = p_alr_io->static_flag;
				alr_wentry.valid = p_alr_io->valid;
				alr_wentry.age = 1;
				
				rc = ali_sw_alr_write_entry(&alr_wentry);

				if(rc==0)
				{
					idx = ali_sw_alr_current_entry_number();
		
					printk("ALR %s : %02x-%02x-%02x-%02x-%02x-%02x to ", 
										alr_wentry.valid?"written":"deleted",\
										HIBYTE(LOWORD(alr_wentry.mac_addr_high16)),\
										LOBYTE(LOWORD(alr_wentry.mac_addr_high16)),\
										HIBYTE(HIWORD(alr_wentry.mac_addr_low32)),\
										LOBYTE(HIWORD(alr_wentry.mac_addr_low32)),\
										HIBYTE(LOWORD(alr_wentry.mac_addr_low32)),\
										LOBYTE(LOWORD(alr_wentry.mac_addr_low32)));

					forward_ports_string[3] = (alr_wentry.ports&1)+'0';
					forward_ports_string[2] = ((alr_wentry.ports>>1)&1)+'0';
					forward_ports_string[1] = ((alr_wentry.ports>>2)&1)+'0';
					forward_ports_string[0] = ((alr_wentry.ports>>3)&1)+'0';
					
					printk("%s, ", forward_ports_string);
						
					printk("%s, %s\n", \
						alr_wentry.filter?"filter_out":"filter_in", alr_wentry.static_entry?"static":"non-static");

					printk("total %d entries in ALR table.\n", idx);
				}
				break;
			}

			case SIOCDEVPRIVATE+5://readvlut
			{
				ali_reg_t ali_reg;
				u32 val;
				printk("ALI_REGS_PHYS_BASE %x \n", (u32)ALI_REGS_PHYS_BASE);
				printk("ALI_REGS_VIRT_BASE %x \n", (u32)ALI_REGS_VIRT_BASE);
				printk("Read/Write alireg\n");	
				printk("Driver ICMP recv %d\n", driver_icmp);
#ifndef	CONFIG_MIPS
				print_icmp_stats();
#endif
				if(copy_from_user(&ali_reg, rq->ifr_data, sizeof(ali_reg))) {
					printk("copy_from_user error\n");
					spin_unlock_irqrestore(&priv->p_switch->lock, flags);
					return -EINVAL;
				}
				if (ali_reg.cmd == 0) {
					printk("read phy_addr is %x ", ali_reg.phy_addr);
					val = __REG32ALI(ali_reg.phy_addr);
					printk("val is %x\n", val);
				}
				if (ali_reg.cmd == 1) {
					printk("write phy_addr is %x ", ali_reg.phy_addr);
					printk("virt_addr is %x ", (u32)__REG32ALI(ali_reg.phy_addr));
					printk("val is %x\n", ali_reg.val);
					__REG32ALI(ali_reg.phy_addr) = ali_reg.val;
				}
				break;
			}
			
			case SIOCDEVPRIVATE+6://writevlut
			{
				u8 idx;
				struct vlut_data vlut_entry;
				p_vlut_data p = (p_vlut_data)&(rq->ifr_ifru);
				
				vlut_entry.port0_member = p->port0_member;
				vlut_entry.port1_member = p->port1_member;
				vlut_entry.port2_member = p->port2_member;
				vlut_entry.port3_member = p->port3_member;
				
				vlut_entry.port0_tag = p->port0_tag;
				vlut_entry.port1_tag = p->port1_tag;
				vlut_entry.port2_tag = p->port2_tag;
				vlut_entry.port3_tag = p->port3_tag;

				vlut_entry.vid = p->vid;
				idx = p->reserved20_31;
				
				ali_sw_vlut_write_entry(idx, (u32 *)&vlut_entry);
				break;
			}

			case SIOCDEVPRIVATE+7://readvport
			{
				if (rq->ifr_ifindex == 2) {
					ali_sw_vlan_show_all_port_info();
				} 
				if (rq->ifr_ifindex == 1) {
					ali_sw_vlut_show_all_entries();
				}
				break;
			}
			
			case SIOCDEVPRIVATE+8://writevport
			{
				struct vlan_port_info vport_info;
				pvlan_port_info p = (pvlan_port_info)&(rq->ifr_ifru);

				vport_info.port_no = p->port_no;
				vport_info.ppriority = p->ppriority;
				vport_info.port_type = p->port_type;
				vport_info.pvid = p->pvid;
				vport_info.fff_chg = p->fff_chg;
				vport_info.igrs_filter = p->igrs_filter;
				vport_info.tag_chg = p->tag_chg;
				vport_info.tag_only = p->tag_only;
				vport_info.untag_only = p->untag_only;

				ali_sw_vlan_write_port_tag(vport_info.port_no, vport_info.ppriority, vport_info.pvid);
				//ali_sw_vlan_write_port_type(vport_info.port_no, vport_info.port_type);
				ali_sw_vlan_write_port_type_ext(&vport_info);
				
				break;
			}

			case SIOCDEVPRIVATE+9://settingdbg
			{
				//struct dbg_setting dbg_info;
				pdbg_setting p = (pdbg_setting)&(rq->ifr_ifru);
				u8 i=0;
				struct net_device *dev = priv->dev;
				struct netdev_hw_addr_list * mc;
				struct netdev_hw_addr * ha;

				if(p->dbg_cmd==DEBUG_DRIVER_LEVEL){
					if(p->dbg_val)
						set_bit(p->dbg_no, &dbg_runtime_val);
					else
						clear_bit(p->dbg_no, &dbg_runtime_val);

					//sw_info("param: no=%d, val=%d", p->dbg_no, p->dbg_val);
					sw_info("dbg=0x%x", (u32)dbg_runtime_val);
					sw_info("dbg0=0x%x", test_bit(0, &dbg_runtime_val));//dms busy
					sw_info("dbg1=0x%x", test_bit(1, &dbg_runtime_val));//
					sw_info("dbg2=0x%x", test_bit(2, &dbg_runtime_val));//recv print packet
					sw_info("dbg3=0x%x", test_bit(3, &dbg_runtime_val));//xmit
					sw_info("dbg4=0x%x", test_bit(4, &dbg_runtime_val));//
					sw_info("dbg5=0x%x", test_bit(5, &dbg_runtime_val));//stp/mc pkt 
					sw_info("dbg6=0x%x", test_bit(6, &dbg_runtime_val));//cks, nonport, mc
					sw_info("dbg7=0x%x", test_bit(7, &dbg_runtime_val));//xmit print
					sw_info("dbg8=0x%x", test_bit(8, &dbg_runtime_val));//udp recv print
					sw_info("dbg9=0x%x", test_bit(9, &dbg_runtime_val));//udp recv print2
					sw_info("dbg10=0x%x", test_bit(10, &dbg_runtime_val));//alloc
					sw_info("dbg11=0x%x", test_bit(11, &dbg_runtime_val));//async dma 2
					sw_info("dbg12=0x%x", test_bit(12, &dbg_runtime_val));
					sw_info("dbg13=0x%x", test_bit(13, &dbg_runtime_val));//forward ports

					if(test_bit(12, &dbg_runtime_val)){
						dump_all_tx_descriptors();
						clear_bit(12, &dbg_runtime_val);
					}
					
				}
				else if(p->dbg_cmd==DISPLAY_MC_SNOOPING_LIST){
					struct mc_alr_entry *p_entry;
					sw_trace("display MC alr table kept in memory:");
					list_for_each_entry(p_entry, &priv->p_switch->mc_alr_entry_list, list) 
					sw_trace("display P0's MC list in memory:");
					mc = &dev->mc;
					i = 0;
					list_for_each_entry(ha, &mc->list, list) {
						sw_trace("	%d. mc ether addr = %02x-%02x-%02x-%02x-%02x-%02x", i++,
													ha->addr[0], ha->addr[1], ha->addr[2], 
													ha->addr[3], ha->addr[4], ha->addr[5]);
					}
							
				}
				else if(p->dbg_cmd==DISPLAY_STP_INFO){
					sw_trace("display switch STP info:");
					ali_sw_show_stp_info();
				}
/*
				else if(p->dbg_cmd== START_FAST_ALR_AGING){
					sw_trace("start alr table fast aging...");
					ali_sw_alr_fast_aging(1);
				}
				else if(p->dbg_cmd== STOP_FAST_ALR_AGING){
					sw_trace("stop alr table fast aging...");
					ali_sw_alr_fast_aging(0);
				}
*/
				else if(p->dbg_cmd== 5){
					sw_trace("scaning mdio...");
					p123_phy_detect(priv);
				}
				
				//could p0_print here.
				break;
			}

			case SIOCDEVPRIVATE+10://egsr
			{
				struct egrs_info egrs_val;
				pegrs_info p = (pegrs_info)&(rq->ifr_ifru);

				egrs_val.mode = p->mode;
				egrs_val.port_no = p->port_no;
				egrs_val.scale = p->scale;

				//sw_trace("SIOCDEVPRIVATE+10");

				ali_sw_egress_rate_adjust(&egrs_val);
				
				break;
			}

			case SIOCDEVPRIVATE+11://igsr
			{
				struct igrs_info igrs_val;

				if (copy_from_user(&igrs_val, rq->ifr_data, sizeof(igrs_val)))
					return -EFAULT;				
/*
				pigrs_info p = (pigrs_info)&(rq->ifr_ifru);

				igrs_val.priority_mode=p->priority_mode;
				igrs_val.igrs_mode=p->igrs_mode;
				igrs_val.ports_enable=p->ports_enable;
				igrs_val.p0_scale = p->p0_scale;
				igrs_val.p1_cbs = p->p1_cbs;
				igrs_val.p1_ebs = p->p1_ebs;
				igrs_val.p23_cbs = p->p23_cbs;
				igrs_val.p23_ebs = p->p23_ebs;

				for(i=0; i<16; i++){
					igrs_val.p1_tc[i]=p->p1_tc[i];
					igrs_val.p2_tc[i]=p->p2_tc[i];
					igrs_val.p3_tc[i]=p->p3_tc[i];
				}
*/
#if 0
				sw_trace("sizeof(struct igrs_info)=%d", sizeof(struct igrs_info));
				sw_trace("igrs_val.priority_mode=%d", igrs_val.priority_mode);
				sw_trace("igrs_val.igrs_mode=%d", igrs_val.igrs_mode);
				sw_trace("igrs_val.ports_enable=%d", igrs_val.ports_enable);
				sw_trace("igrs_val.p0_scale=%d", igrs_val.p0_scale);
				sw_trace("igrs_val.p1_cbs=%d", igrs_val.p1_cbs);
				sw_trace("igrs_val.p1_ebs=%d", igrs_val.p1_ebs);
				sw_trace("igrs_val.p1_tc[0]=%d", igrs_val.p1_tc[0]);
				sw_trace("igrs_val.p1_tc[15]=%d", igrs_val.p1_tc[15]);
				sw_trace("igrs_val.p2_tc[0]=%d", igrs_val.p2_tc[0]);
				sw_trace("igrs_val.p2_tc[15]=%d", igrs_val.p2_tc[15]);
				sw_trace("igrs_val.p3_tc[0]=%d", igrs_val.p3_tc[0]);
				sw_trace("igrs_val.p3_tc[15]=%d", igrs_val.p3_tc[15]);
#endif

				ali_sw_ingress_rate_adjust_ext(&igrs_val);
				break;
			}			

			case SIOCDEVPRIVATE+12://flowctl
			{
				struct flowctl_info fc_val;
				pflowctl_info p = (pflowctl_info)&(rq->ifr_ifru);

				fc_val.port_no = p->port_no;
				fc_val.p0_blks = p->p0_blks;
				fc_val.p123_mode = p->p123_mode;

				//sw_trace("SIOCDEVPRIVATE+12");
				
				ali_sw_flow_control(&fc_val);
				
				break;
			}

			case SIOCDEVPRIVATE+13://bandrate
			{
				struct bandrate_info br_val;
				pbandrate_info p = (pbandrate_info)&(rq->ifr_ifru);

				br_val.port_no = p->port_no;//don't care
				br_val.interval= p->interval;

				//sw_trace("SIOCDEVPRIVATE+13");
				
				ali_sw_band_rate_show(&br_val);
				
				break;
			}

			case SIOCDEVPRIVATE+14://
			{
				struct acl_info acl_val;
				if (copy_from_user(&acl_val, rq->ifr_data, sizeof(acl_val)))
					return -EFAULT;					
				
#if 0
				sw_trace("sizeof(struct acl_info)=%d", sizeof(struct acl_info));
				sw_trace("acl_val.acl_proto=%d", acl_val.acl_proto);
				sw_trace("acl_val.acl_command=%d", acl_val.acl_command);
				sw_trace("acl_val.acl_mode=%d", acl_val.acl_mode);
				sw_trace("acl_val.acl_idx=%d", acl_val.acl_idx);
				sw_trace("acl_val.ports_enable=%d", acl_val.ports_enable);
				sw_trace("acl_val.priority_sel=%d", acl_val.priority_sel);
				sw_trace("acl_val.ip1=0x%x", acl_val.ip1);
				sw_trace("acl_val.ip2=0x%x", acl_val.ip2);
				sw_trace("acl_val.port1_start=0x%x", acl_val.port1_start);
				sw_trace("acl_val.port1_end=0x%x", acl_val.port1_end);
				sw_trace("acl_val.port2_start=0x%x", acl_val.port2_start);
				sw_trace("acl_val.port2_end=0x%x", acl_val.port2_end);
#endif				

				if(acl_val.acl_command==ACL_WRITE_ENTRY){			
					ali_sw_acl_write_entry(&acl_val);
				}else if(acl_val.acl_command==ACL_READ_ENTRY){
					if(acl_val.acl_idx==0x10)
						ali_sw_acl_show_all(&acl_val);
					else 
						ali_sw_acl_show_single(&acl_val);
					//else
						//sw_error("unsupported: %d, %s", __LINE__, __FUNCTION__);
				}else
					sw_error("unsupported: acl_command=%d, acl_idx=%d",acl_val.acl_command, acl_val.acl_idx);
				
				break;				
			}

			case SIOCDEVPRIVATE+15://settingstp
			{
				//struct stp_info stp_val;
				pstp_info p = (pstp_info)&(rq->ifr_ifru);

				if(p->stp_op==STP_OPERATION_ENABLE){
					spin_unlock_irqrestore(&priv->p_switch->lock, flags);//a potential problem.
					ali_sw_stp_enable();
					if(priv->p_switch->ports_link_status != 0)
						ali_sw_start_sw_stp(priv->p_switch);
					spin_lock_irqsave(&priv->p_switch->lock, flags);
				}
				else if(p->stp_op==STP_OPERATION_DISABLE){
					spin_unlock_irqrestore(&priv->p_switch->lock, flags);//a potential problem.
					ali_sw_stp_disable();
					spin_lock_irqsave(&priv->p_switch->lock, flags);
				}
				else if(p->stp_op==STP_OPERATION_INFO)
					ali_sw_show_stp_info();
				else if(p->stp_op==STP_SET_PRIORITY){
					printk("original switch priority: 0x%x, ", ali_sw_stp_priority);
					ali_sw_stp_priority = p->stp_priority;
					printk("now changes to: 0x%x\n", ali_sw_stp_priority);
					if(gp_sw->stp_is_working == ALI_SW_STP_WORKING)
						sw_info("STP is running, please disable it and re-enable it!");
				}
				else
					sw_error("unsupported: %d, %s", __LINE__, __FUNCTION__);
				
				break;
			}
			
			case SIOCDEVPRIVATE://settingmisc
			{
				pmisc_info pmisc = (pmisc_info)&(rq->ifr_ifru);
				sw_info("%s setting misc, misc_cmd %d\n", __FUNCTION__, pmisc->misc_cmd); 
				if(pmisc->misc_cmd==SHOW_DROP_STATISTICS){
					ali_sw_show_drops();
				}
				else if(pmisc->misc_cmd==SHOW_LINK_STATUS){
					ali_sw_show_port_link();
				}
				else if(pmisc->misc_cmd==READ_PHY_MDIO){
					u16 mido_result;
					/*
					sw_info("READ_PHY_MDIO: port_no=%d, phy_reg_addr=%d", pmisc->misc_val.mdio_val.port_no, 
																		pmisc->misc_val.mdio_val.phy_reg_addr);
					*/
					if((pmisc->misc_val.mdio_val.port_no>0&&pmisc->misc_val.mdio_val.port_no<4)&&pmisc->misc_val.mdio_val.phy_reg_addr<32){
						mido_result = (u16)mac_mdio_read(priv, port_phy_addr[pmisc->misc_val.mdio_val.port_no], pmisc->misc_val.mdio_val.phy_reg_addr);
						sw_info("port[%d] phy_reg[%d] = 0x%x", pmisc->misc_val.mdio_val.port_no, pmisc->misc_val.mdio_val.phy_reg_addr, mido_result);
					}else{
						sw_info("READ_PHY_MDIO: wrong input, port_no=%d, phy_reg_addr=%d", pmisc->misc_val.mdio_val.port_no, 
																		pmisc->misc_val.mdio_val.phy_reg_addr);
					}
				}
				else if(pmisc->misc_cmd==VLAN_OPERATION){
					if(pmisc->misc_val.vlan_en){
						SW_W8(SysPortAccess, 4);
						SW_W32(VlanPortType, (SW_R32(VlanPortType)|SW_VlanEn));
						sw_trace("reg[VlanPortType]=0x%x", SW_R32(VlanPortType));
					}else{
						SW_W8(SysPortAccess, 4);
						SW_W32(VlanPortType, (SW_R32(VlanPortType)&(~SW_VlanEn)));
						sw_trace("reg[VlanPortType]=0x%x", SW_R32(VlanPortType));
					}
				}

				else if(pmisc->misc_cmd==ALR_AGING_TEST){
					if(pmisc->misc_val.age_val.aging_type == 0){
						ali_sw_alr_fast_aging_ext(pmisc->misc_val.age_val.aging_time);
					}else if(pmisc->misc_val.age_val.aging_type == 1){
						sw_trace("aging time changed to %d (s)", pmisc->misc_val.age_val.aging_time);
						ali_sw_alr_fast_aging_ext(pmisc->misc_val.age_val.aging_time);
						mod_timer(&sw_test_timer, jiffies + (pmisc->misc_val.age_val.aging_time*HZ));
					}
				}

				else if(pmisc->misc_cmd==QOS_ACL_SETTING){
					SW_W8(SysPortAccess, 4);
					if(pmisc->misc_val.qos_en==1){
						SW_W32(QosCtl, SW_R32(QosCtl)|ACL_QOS_EN);
						sw_info("qos acl enabled!");
					}
					else {
						SW_W32(QosCtl, SW_R32(QosCtl)&(~ACL_QOS_EN));
						sw_info("qos acl disabled!");
					}
				}
				else if(pmisc->misc_cmd==QOS_VLAN_SETTING){
					SW_W8(SysPortAccess, 4);
					if(pmisc->misc_val.qos_en==1){
						SW_W32(QosCtl, SW_R32(QosCtl)|VLAN_QOS_EN);
						sw_info("qos vlan enabled!");
					}
					else {
						SW_W32(QosCtl, SW_R32(QosCtl)&(~VLAN_QOS_EN));
						sw_info("qos vlan disabled!");
					}
				}
				else if(pmisc->misc_cmd==QOS_DSCP_SETTING){
					SW_W8(SysPortAccess, 4);
					if(pmisc->misc_val.qos_en==1){
						SW_W32(QosCtl, SW_R32(QosCtl)|DSCP_QOS_EN);
						sw_info("qos dscp enabled!");
					}
					else {
						SW_W32(QosCtl, SW_R32(QosCtl)&(~DSCP_QOS_EN));
						sw_info("qos dscp disalbed!");
					}
				}

				else if(pmisc->misc_cmd==IGRS2_DBG_SETING){
					SW_W8(SysPortAccess, 4);
					if(pmisc->misc_val.igrs2_val.port==1){
						SW_W32(P1CBS, pmisc->misc_val.igrs2_val.cbs);
						SW_W32(P1EBS, pmisc->misc_val.igrs2_val.ebs);
						SW_W16(P1TC, pmisc->misc_val.igrs2_val.tc);
						sw_info("reg[p1cbs]=0x%x, reg[p1ebs]=0x%x, reg[p1tc]=0x%x", SW_R32(P1CBS), SW_R32(P1EBS), SW_R16(P1TC));
					}else if(pmisc->misc_val.igrs2_val.port==2){
						//sw_info("port=%d, cbs=0x%x, ebs=0x%x, tc=0x%x", pmisc->misc_val.igrs2_val.port,
						//	pmisc->misc_val.igrs2_val.cbs, pmisc->misc_val.igrs2_val.ebs, pmisc->misc_val.igrs2_val.tc);
						
						SW_W32(P23CBS, pmisc->misc_val.igrs2_val.cbs);
						SW_W32(P23EBS, pmisc->misc_val.igrs2_val.ebs);
						SW_W16(P2TC, pmisc->misc_val.igrs2_val.tc);
						sw_info("reg[p23cbs]=0x%x, reg[p23ebs]=0x%x, reg[p2tc]=0x%x", SW_R32(P23CBS), SW_R32(P23EBS), SW_R16(P2TC));
					}else if(pmisc->misc_val.igrs2_val.port==3){
						//sw_info("port=%d, cbs=0x%x, ebs=0x%x, tc=0x%x", pmisc->misc_val.igrs2_val.port,
						//	pmisc->misc_val.igrs2_val.cbs, pmisc->misc_val.igrs2_val.ebs, pmisc->misc_val.igrs2_val.tc);
						
						SW_W32(P23CBS, pmisc->misc_val.igrs2_val.cbs);
						SW_W32(P23EBS, pmisc->misc_val.igrs2_val.ebs);
						SW_W16(P3TC, pmisc->misc_val.igrs2_val.tc);
						sw_info("reg[p23cbs]=0x%x, reg[p23ebs]=0x%x, reg[p3tc]=0x%x", SW_R32(P23CBS), SW_R32(P23EBS), SW_R16(P3TC));
					}else{
						sw_error("error in IGRS2_DBG_SETING of MISC setting!");
					}
					
				}
				
				else if(pmisc->misc_cmd==ROUTE_SETTING){
					if(pmisc->misc_val.route_val.route_cmd == WAN_PORT_SELECTION){
						u32 tmp;
						SW_W8(SysPortAccess, 4);
						tmp = SW_R32(SysCtlSwFabric);
						tmp &= ~(WAN_PORT_MSK<<WAN_PORT_OFFSET);
						if(pmisc->misc_val.route_val.un.wan_port==1)
							tmp |= 1<<WAN_PORT_OFFSET;
						else if(pmisc->misc_val.route_val.un.wan_port==2)
							tmp |= 2<<WAN_PORT_OFFSET;
						else if(pmisc->misc_val.route_val.un.wan_port==3)
							tmp |= 4<<WAN_PORT_OFFSET;
						else if(pmisc->misc_val.route_val.un.wan_port==0)
							;
						else{
							sw_error("error in WAN_PORT_SELECTION of ROUTE_SETTING!");
							break;
						}
						
						SW_W32(SysCtlSwFabric, tmp);
						sw_info("reg[SysCtlSwFabric]=0x%x", SW_R32(SysCtlSwFabric));
					}
					else  if(pmisc->misc_val.route_val.route_cmd == ROUTE_FORWARDING_TEST){
						test_forwarding_ports = 0;
						if(pmisc->misc_val.route_val.un.forward_ports&0x1)
							test_forwarding_ports |= 1;
						if(pmisc->misc_val.route_val.un.forward_ports&0x10)
							test_forwarding_ports |= 2;
						if(pmisc->misc_val.route_val.un.forward_ports&0x100)
							test_forwarding_ports |= 4;
						sw_info("test_forwarding_ports=0x%x", test_forwarding_ports);
					}else{
						sw_error("error in ROUTE_SETTING of MISC setting!");
					}
				} 
				break;
			}
			
			default :
			{
				sw_warn("%s: receive unsupported cmd!", __FUNCTION__);
				rc = -EOPNOTSUPP;
				break;
			}
			
		}
		//spin_unlock_irqrestore(&priv->cmd_lock, flags);
		CMD_UNLOCK_IRQ(&priv->cmd_lock, flags);
	}
	else
	{
		if (!netif_running(dev))
		{
			sw_warn("%s: NIC isn't running!", __FUNCTION__);
			return -EINVAL;
		}
		///spin_lock_irqsave(&priv->cmd_lock, flags);
		CMD_LOCK_IRQ(&priv->cmd_lock, flags);
		switch(cmd) {
			case SIOCGMIIREG: {
				struct mii_ioctl_data *data = if_mii(rq);
				printk("phy id %x reg_num %x\n", data->phy_id, data->reg_num);
				data->val_out = mac_mdio_read(priv, data->phy_id, data->reg_num);
				rc = 0; 
				break;
		  	}
			case SIOCSMIIREG: {
				struct mii_ioctl_data *data = if_mii(rq);
				printk("phy id %x reg_num %x\n", data->phy_id, data->reg_num);
				mac_mdio_write(priv, data->phy_id, data->reg_num, data->val_in);
				rc = 0; 
				break;
		 	}
			default:{
				sw_warn("%s doesn't support cmd %d\n", __FUNCTION__, cmd);
				rc = -EOPNOTSUPP;
			}
		}
		//spin_unlock_irqrestore(&priv->cmd_lock, flags);	
		CMD_UNLOCK_IRQ(&priv->cmd_lock, flags);
	}
	return rc;
}

/* interrupt control, interrupt driven... */
void ali_sw_irq_enable(int lock) {
	u32 flags;
	if (lock) {
		CMD_LOCK_IRQ(&gp_p0->cmd_lock, flags);
	}
	SW_W8(SysPortAccess, 0);
	SW_W32(P0IMR, P0_INTERRUPT_MASK_SIMPLE);
	SW_R32(P0IMR);
	SW_W8(SysPortAccess, 4);
	SW_W32(FabricIMR, FABRIC_INTERRUPT_MASK);
	SW_R32(FabricIMR);
	if (lock) {
		CMD_UNLOCK_IRQ(&gp_p0->cmd_lock, flags);
	}
}

void ali_sw_irq_disable(int lock) {
	u32 flags;
	if (lock) {
		CMD_LOCK_IRQ(&gp_p0->cmd_lock, flags);
	}
	SW_W8(SysPortAccess, 0);
	SW_W32(P0IMR, 0);
	SW_R32(P0IMR);
	SW_W8(SysPortAccess, 4);
	SW_W32(FabricIMR, 0);
	SW_R32(FabricIMR);
	if (lock) {
		CMD_UNLOCK_IRQ(&gp_p0->cmd_lock, flags);
	}
}

void p123_link_status_tracking(struct net_ali_sw_port *p)
{
	//question, should we change IMR(RX complete) because of this??
	struct net_ali_sw *sw = p->sw;
	
	if(p->transmit_okay)
		set_bit((p->port_no-1), &sw->ports_link_status);
	else
		clear_bit((p->port_no-1), &sw->ports_link_status);

	if(netif_carrier_ok(sw->p_P0->dev))
	{
		if(sw->ports_link_status == 0)
		{
			netif_carrier_off(sw->p_P0->dev);
			netif_stop_queue(sw->p_P0->dev);//kinda redundent...
			sw_trace("netif_stop_queue due to port's link changing!");
			if(sw->stp_enabled==ALI_SW_KERNEL_STP)
				ali_sw_stop_sw_stp(sw);
		}	
	}
	else
	{
		if(sw->ports_link_status != 0)
		{
			netif_carrier_on(sw->p_P0->dev);
			netif_wake_queue(sw->p_P0->dev);
			sw_trace("netif_wake_queue due to port's link changing!");
			if(sw->stp_enabled==ALI_SW_KERNEL_STP)
				ali_sw_start_sw_stp(sw);
		}
	}
}

void p123_link_established(struct net_ali_sw_port *p)
{
	new_p123_phy_link_chk(p);
	p123_get_start(p->p_P0, p->port_no);
	p->transmit_okay = true;
	p123_link_status_tracking(p);
}

void p123_link_disconnected(struct net_ali_sw_port *p)
{
	
//	if(p->link_partner != 0)
//	{
		p->link_partner = 0;		
		p->phy_rst = false;		//set after phy_set
		p->link_established = false;	//set after auto-neg
		p->transmit_okay = false;	//
		_ali_sw_get_stats(p->p_P0->dev);
		p0123_port_disable(p->port_no);//locking? tbd?
		p123_link_status_tracking(p);//locking? tbd?
//	}
}

void new_p123_link_changing(struct net_ali_sw_port *p) {
	if (p->link_established == true) {
		p123_link_established(p);
	} else {
		sw_error("%s: shoudn't be here", __FUNCTION__);
	}
}

void _new_ali_sw_super_housekeeper(p_p0_private priv) {
	struct net_ali_sw *sw=priv->p_switch;
	struct net_ali_sw_port *p;
	u8 port_no=0;
	unsigned long flags;
	u16 status;
    u16 mode_control_reg;
	u16 ctrl;
	int link_changed = 0;
	u16 tmp_u16;
    u32 tmp32;
	if(unlikely(sw==NULL)) {	
		sw_error("%s: sw is null\n", __FUNCTION__);
		return;
	}
	
	CMD_LOCK_IRQ(&priv->cmd_lock, flags);
	for(port_no=1; port_no<MAX_PORT_NUM; port_no++) {
		if(!p0123_port_is_in_use(priv, port_no)) {
			ali_trace(1, "port %d isn't used", port_no);
			continue;
		}
		p = &(sw->port_list[port_no]);
		mode_control_reg = (unsigned short)mac_mdio_read(priv, port_phy_addr[port_no], PhyBasicModeCtrl);
		status = (u16) mac_mdio_read(priv, port_phy_addr[port_no], PhyBasicModeStatus);
        ali_trace(1, "port %d status %x mode_control_reg %x\n", port_no, status, mode_control_reg);
		if (p->link_established == false) {
			ali_trace(1, "port %d status %x link_established false\n", port_no, status);
			if (status & BMSRLinkStatus) {
				ali_trace(1, "port %d BMSRLinkStatus true\n", port_no);
				ctrl = (u16)mac_mdio_read(priv, port_phy_addr[port_no], PhyBasicModeCtrl);
				if ((ctrl & BMCRANEnable) && (status & BMSRANComplete)) {
					sw_trace("ali sw: port[%d]=>auto nego link established.", p->port_no);
					link_changed = 1;
				} else if (!(ctrl & BMCRANEnable)) {
					sw_trace("ali sw:port[%d]=>non auto nego link established.", p->port_no);
					link_changed = 1;
				}
				if (link_changed) {
					p->link_established= true;
					if (port_no == 1) {
						printk("giga phy rx \n");
						mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x00);
						mac_mdio_write(priv, port_phy_addr[port_no], 0x1e, 0x2ee);
						tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1e);
						printk("port %d: 1e %x\n", port_no, tmp_u16);
                        SW_W8(SysPortAccess, 1);
                        tmp32 = SW_R32(TxRxCfg);
                        printk("TxRxCfg is %x\n", tmp32);
                        tmp32 &= 0xFFFFFC00;
                        tmp32 |= (0x9 << 5);
                        tmp32 |= 0x10;
                        SW_W32(TxRxCfg, tmp32);
                        tmp32 = SW_R32(TxRxCfg);
                        printk("TxRxCfg changed to  %x\n", tmp32);
					}
					p123_link_established(p);
				}
			}
		} else { /*last state is up*/
			if (status & BMSRLinkStatus) {
				tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], PhyNWayLPAR);
				if (tmp_u16 != p->link_partner && tmp_u16) {
					sw_trace("ali sw port[%d]=>link changes while up.", p->port_no);
					new_p123_link_changing(p);
				}
			} else {
				sw_trace("ali sw port[%d]=>link disconnected.", p->port_no);
				p123_link_disconnected(p);
			}
        }
	} /* for (port_no = 1; */	
	CMD_UNLOCK_IRQ(&priv->cmd_lock, flags);
	return;
}

irqreturn_t ali_sw_isr (int Irq, void *dev_id) {
	struct net_device *dev = dev_id;
	p_p0_private priv;
	UINT32 p0_isr, fabric_isr;

	if (unlikely(dev == NULL)) {	
		sw_warn("%s, %d", __FUNCTION__, __LINE__);
		return IRQ_NONE;
	}
	priv = netdev_priv(dev);
	CMD_LOCK(&priv->cmd_lock);
	SW_W8(SysPortAccess, 4);
	fabric_isr = SW_R32(FabricISR);
	SW_W32(FabricISR, ~FABRIC_INTERRUPT_MASK);

	SW_W8(SysPortAccess, 0);
	p0_isr = SW_R32(P0ISR);
	SW_W32(P0ISR, 0);
	if (unlikely(!netif_running(dev))) {
		sw_warn("%s, %d", __FUNCTION__, __LINE__);
        CMD_UNLOCK(&priv->cmd_lock);
		return IRQ_HANDLED;
	}

	if (fabric_isr & FABRIC_INTERRUPT_MASK) {
		priv->fabric_isr = fabric_isr;
		if (fabric_isr & EXCEPT_BM) {
			handle_exception_isr(priv);
			fabric_isr &= ~EXCEPT_BM;
		}
		/* add watch dog for link change */
		if (fabric_isr & GP_TIMER_EXPIRED) {
			tasklet_schedule(&p0_tasklet);
			fabric_isr &= ~GP_TIMER_EXPIRED;
		}
		if(p0_isr & P0_RX_COMPLETE) {
			priv->p0_isr = p0_isr;
			ali_trace(2, "rx_complete");
		}

		if(p0_isr & P0_TX_COMPLETE) {
			ali_trace(2, "tx_complete");
		}
		if (likely(napi_schedule_prep(&priv->napi))) {
			ali_sw_irq_disable(0);
			__napi_schedule(&priv->napi);
		}
	}
	//spin_unlock(&priv->cmd_lock);
	CMD_UNLOCK(&priv->cmd_lock);
	return IRQ_RETVAL(true);
}

void ali_sw_chip_rst () {
	u8 tmp_u8;
	//u8 i = 4;
	SW_W8(SysPortAccess, 4);//assuming i = 4 right now
	SW_W8(SysCtlSwFabric, SCRSWFRst|SW_R8(SysCtlSwFabric));	
	do{
		SW_W8(SysPortAccess, 4);
		tmp_u8 = SW_R8(SysCtlSwFabric);
		printk("switch fabric is reseting...");
	}while(tmp_u8 & SCRSWFRst);

	SW_W8(SysPortAccess, 4);
	SW_W8(SysCtlSwFabric, SCRSWFEn|SW_R8(SysCtlSwFabric));	
	SW_W8(SysCtlSwFabric, SCRAutoFreshTable|SW_R8(SysCtlSwFabric));	
	//SW_W16(SysCtlSwFabric, (SCRPort0En)|SW_R16(SysCtlSwFabric));	//shouldn't here.
	printk("done!reg[SysCtlSwFabric]=0x%x\n", SW_R16(SysCtlSwFabric));
}

/****************************************************************************
	in device_open:
		initial software variable
		initial haredare
			chip reset
			PHY detecting
			PHY setting
		allocate descriptors
		allocate rx memory
		request IRQ
		set flag of net_device, queue to make it start to work
****************************************************************************/
static int ali_sw_open (struct net_device *dev) {
	UINT32 i;
	UINT32 ret;
	UINT32 tmp_reg32, tmp1, tmp2, mask;
	p_p0_private priv = netdev_priv(dev);

	if (netif_msg_ifup(priv)) {
		sw_trace("%s: start opening flow...", dev->name);
	}

	/* off carrier */
	netif_carrier_off(dev);
	ali_sw_irq_disable(1);

	// 1 reset sw & enable sw (enabe sw = enable p0)
	ali_sw_chip_rst();	

	/*1.1 disable stp. default stp enabled */
	SW_W8(SysPortAccess, 4);
	SW_W32(PktDecCtl, SW_R32(PktDecCtl)&(~StpEn));
	/* 1.2 set registers */
	SW_W8(SysPortAccess, 4);
	SW_W32(0x188, 0xffffffff);
	printk("0x188 reg %x\n", SW_R32(0x188));
	tmp_reg32 = SW_R32(0x184);
	mask = 0x001f1f00; 
	mask = ~mask;
	tmp1 = 0x3;
	tmp1 = tmp1 << 16;
	tmp2 = 0x5;
	tmp2 = tmp2 << 8;

	tmp_reg32 &= mask;
	tmp_reg32 |= tmp1;
	tmp_reg32 |= tmp2;
	SW_W32(0x184, tmp_reg32);
	printk("0x184 reg %x\n", SW_R32(0x184));

	SW_W8(SysPortAccess, 1);
	mask = 0x00000020;
	tmp_reg32 = SW_R32(0x40);
	tmp_reg32 |= mask;
	SW_W32(0x40, tmp_reg32);
	printk("0x40 reg %x\n", SW_R32(0x40));

	SW_W8(SysPortAccess, 4);
	SW_W32(0x1c4, 0x10400);
	printk("Fabric 0x1c4 %x\n", SW_R32(0x1c4));

#if 0
	SW_W8(SysPortAccess, 4);
	SW_W32(0x154, 0x0);
	SW_W32(0x150, 0x841d4100);
	printk("Fabric 0x154 %x\n", SW_R32(0x154));
	SW_W8(SysPortAccess, 4);
	SW_W32(0x154, 0x2ee);
	SW_W32(0x150, 0x841e4100);
	printk("Fabric 0x154 %x\n", SW_R32(0x154));
#endif

	SW_W8(SysPortAccess, 4);
	mask = 0x7ffffff;
	mask = ~mask;
	tmp_reg32 = SW_R32(0x190);
	tmp_reg32 &= mask;	
	tmp_reg32 |= 0x24ccccd;
	SW_W32(0x190, tmp_reg32);
	printk("Fabric 0x190 %x\n", SW_R32(0x190));

	tmp_reg32 = SW_R32(0x18c);
	mask = 0xC7FFFFFF;
	mask = ~mask;
	tmp_reg32 &= mask;	
	tmp_reg32 |= 0x81000000;
	SW_W32(0x18c, tmp_reg32);
	printk("Fabric 0x18c %x\n", SW_R32(0x18c));

	SW_W8(SysPortAccess, 4);
	SW_W32(VlanPortType, (SW_R32(VlanPortType)&(~SW_VlanEn)));
	sw_trace("reg[VlanPortType]=0x%x", SW_R32(VlanPortType));

	// 2 mac address
	p0123_set_mac_addr(priv);
					
	// 3 initialize phy/link status and flag, pretty much sw level.		
	p0123_phy_link_state_init(priv);

	// 4 phy detecting		
	printk("ali_sw_base %x\n", ali_sw_base);
	p123_set_linkcheck_mode(priv);

	p123_phy_test(priv);

	if (p123_phy_detect(priv)==false) {
		sw_error("no phy detected, close switch!\n");
		goto open_err_out;
	}
	
	// 5 phy setting	
	for(i=1; i<MAX_PORT_NUM; i++)
		p123_phy_rst (priv, i);//things like PHY rst, autoneg stuff...

#if 0
	SW_W8(SysPortAccess, 4);
	SW_W32(0x154, 0x0);
	SW_W32(0x150, 0x841d4100);
	printk("Fabric 0x154 %x\n", SW_R32(0x154));
	SW_W8(SysPortAccess, 4);
	SW_W32(0x154, 0x2ee);
	SW_W32(0x150, 0x841e4100);
	printk("Fabric 0x154 %x\n", SW_R32(0x154));
#endif

	//memory allocation  for P0
	p0_alloc_descriptors(priv);

	//SW reset for P0
	p0_cnt_init(priv);
	
	p0_get_start(priv, true);//RX memory is allocated here, first allocate by SW, than HW will DMA to these memory.
	
	gp_sw->stp_enabled = ALI_SW_NO_STP;

#ifdef ALI_SW_PROC
	if (!proc_net_fops_create(&init_net, "ali_switch", S_IRUGO, &ali_sw_seq_fops))
		goto open_err_out_proc;
#endif
	
	SW_W32(GETimer, p0_timer_frq);
	ret =request_irq(priv->irq_num, ali_sw_isr, 0, dev->name, (void *)dev);
	if(ret != 0) {
		priv->acquired_isr = false;	
		sw_error("Unable to use IRQ = %d, Error number = %d\n", priv->irq_num, ret);
		goto open_err_out;
	}
	priv->acquired_isr = true;	


#if ALI_SW_VLAN_TAG_USED
	sw_trace("ALI_SW_VLAN_TAG_USED");
#endif

	setup_timer(&sw_test_timer, ali_sw_test_timer_hdler, NULL);
	/* whether to start queue here ? */
	netif_start_queue(priv->dev);

	napi_enable(&priv->napi);
	ali_sw_irq_enable(1);

	sw_trace("ALi switch opened!");
	return 0;

open_err_out:
	if(p0_rx_mode == ALI_SW_RX_MODE_NAPI)
		napi_disable(&priv->napi);

	if(priv->acquired_isr) {
		free_irq(priv->irq_num, (void *)priv->dev);
		priv->acquired_isr = false;
	}
	
	p0_free_rings(priv);
	return -EFAULT;	
}

static int ali_sw_close (struct net_device *dev) {
	p_p0_private priv = netdev_priv(dev);

	if (netif_msg_ifdown(priv)) {
		printk("%s: disabling interface\n", dev->name);
	}
	
	//spin_lock_irqsave(&priv->p_switch->lock, flags);

	CMD_LOCK(&priv->cmd_lock);
	SW_W8(SysPortAccess, 0);
	SW_W32(P0IMR, 0);
	SW_W32(P0ISR, 0);
	SW_W8(SysPortAccess, 4);
	SW_W32(FabricIMR, 0);
	SW_W32(FabricISR, 0);

	//spin_unlock_irqrestore(&priv->p_switch->lock, flags);
	CMD_UNLOCK(&priv->cmd_lock);
	napi_disable(&priv->napi);
	netif_stop_queue(dev);
	netif_carrier_off(dev);	
	free_irq(priv->irq_num, dev);

	p0_free_rings(priv);
	p0_desc_clean(priv);

#ifdef ALI_SW_PROC
	proc_net_remove(&init_net, "ali_switch");
#endif
    return 0;
}

static void ali_sw_tx_timeout(struct net_device *dev) {
	p_p0_private priv = netdev_priv(dev);
	u32 reg_val1, reg_val2;
	
	//spin_lock_irqsave(&priv->p_switch->lock, flags);

	CMD_LOCK(&priv->cmd_lock);
	SW_W8(SysPortAccess, 0);
	reg_val1=SW_R32(P0IMR);
	SW_W8(SysPortAccess, 4);
	reg_val2=SW_R32(FabricIMR);

	CMD_UNLOCK(&priv->cmd_lock);
	//spin_unlock_irqrestore(&priv->p_switch->lock, flags);
	sw_warn("%s: reg[P0IMR]=0x%x, reg[FabricIMR]=0x%x", __FUNCTION__, reg_val1, reg_val2);
	return;
}

#if ALI_SW_VLAN_TAG_USED
static void p0_vlan_rx_register(struct net_device *dev, struct vlan_group *grp)
{
	p_p0_private priv = netdev_priv(dev);
	unsigned long flags, tmp_u32;

	spin_lock_irqsave(&priv->p_switch->lock, flags);
	
	priv->vlgrp = grp;
	priv->vlan_tag_remove = true;

	sw_trace("enter p0_vlan_rx_register");
	sw_trace("grp->real_dev->name:%s", grp->real_dev->name);
	sw_trace("grp->vlan_devices_arrays[0]:%s", ((struct net_device *)(grp->vlan_devices_arrays[0]))->name);

	spin_unlock_irqrestore(&priv->p_switch->lock, flags);
}
#endif 

static void ali_sw_get_drvinfo (struct net_device *dev, struct ethtool_drvinfo *info) {
	strcpy (info->driver, DRV_NAME);
	strcpy (info->version, DRV_VER);
	strcpy (info->bus_info, "Local Bus");
}

#if 0
static int ali_sw_mii_ethtool_sset(struct mii_if_info *mii, struct ethtool_cmd *ecmd) {
	printk("doesn't support ethtool sset\n");
	return 0;
}
#endif

static int ali_sw_get_settings(struct net_device *dev, struct ethtool_cmd *cmd) {
	printk("doesn't support ethtool get\n");
	return 0;
}

static int ali_sw_set_settings(struct net_device *dev, struct ethtool_cmd *cmd) {
	printk("doesn't support ethtool set\n");
	return 0;
}


static int ali_sw_nway_reset(struct net_device *dev) {
	return 0;
}

static u32 ali_sw_get_msglevel(struct net_device *dev) {
	p_p0_private priv = netdev_priv(dev);
	return priv->msg_enable; //question, what for?
}

static void ali_sw_set_msglevel(struct net_device *dev, u32 value) {
	p_p0_private priv = netdev_priv(dev);
	priv->msg_enable = value;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)) /*zmiao*/
#else
static u32 ali_sw_get_rx_csum(struct net_device *dev) {
	return (u32)p0_rx_csum;
}

static int ali_sw_set_rx_csum(struct net_device *dev, u32 data) {
	p_p0_private priv = netdev_priv(dev);
	return 0;
}
#endif

static void ali_sw_get_regs(struct net_device *dev, struct ethtool_regs *regs, void *p) {
	return;
}

static const char ali_sw_ethtool_gstrings_stats[][ETH_GSTRING_LEN] = {
	"rx_packets",		/* total packets received	*/
	"tx_packets",		/* total packets transmitted	*/
	"rx_bytes", 	/* total bytes received 	*/
	"tx_bytes", 	/* total bytes transmitted	*/
	"rx_errors",		/* bad packets received 	*/
	"tx_errors",		/* packet transmit problems */
	"rx_dropped",		/* no space in linux buffers	*/
	"tx_dropped",		/* no space available in linux	*/
	"multicast",		/* multicast packets received	*/
	"collisions",
	"rx_length_errors", 	/* detailed rx_errors: */
	"rx_over_errors",		/* receiver ring buff overflow	*/
	"rx_crc_errors",		/* recved pkt with crc error	*/
	"rx_frame_errors",		/* recv'd frame alignment error */
	"rx_fifo_errors",		/* recv'r fifo overrun		*/
	"rx_missed_errors", /* receiver missed packet	*/
	"tx_aborted_errors",	/* detailed tx_errors */
	"tx_carrier_errors",
	"tx_fifo_errors",
	"tx_heartbeat_errors",
	"tx_window_errors",
	"rx_compressed",		/* for cslip etc */
	"tx_compressed",		
//23 
//----------------------------------------------------------------------
	"rx_from_port[0]",
	"rx_from_port[1]",
	"rx_from_port[2]",
	"rx_from_port[3]",
	"rx_uc",	
	"rx_bc",	
	"rx_mc",		
	"rx_ip",								
	"rx_ipv6",						
	"rx_frag",	
	"rx_tcp",	
	"rx_udp",	
	"rx_8023",
	"rx_pppoe",
	"rx_snap",
	"rx_bpdu",
	"rx_igmp_mld",
	"rx_gmrp_mmrp",
	"rx_gvrp_mvrp",
	"rx_garp",
	"rx_ip_chksum_err_desc",	
	"rx_ip_data_chksum_err_desc", 
	"rx_ip_chksum_err_reg",	
	"rx_ip_data_chksum_err_reg", 	
//24
//----------------------------------------------------------------------
	"rx_runt_pkts_port[1]",
	"rx_runt_pkts_port[2]",
	"rx_runt_pkts_port[3]",
	"rx_long_frame_port[1]",
	"rx_long_frame_port[2]",
	"rx_long_frame_port[3]",
	"rx_crc_err_pkts_port[1]",
	"rx_crc_err_pkts_port[2]",
	"rx_crc_err_pkts_port[3]",
	"rx_alig_err_frame_port[1]",
	"rx_alig_err_frame_port[2]",
	"rx_alig_err_frame_port[3]",
	"tx_pkts_port[1]",
	"tx_pkts_port[2]",
	"tx_pkts_port[3]",
//6
};
 
	

#define ALI_SW_STATS_LEN	ARRAY_SIZE(ali_sw_ethtool_gstrings_stats)

static int ali_sw_ethtool_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ALI_SW_STATS_LEN;
	default:
		return -EOPNOTSUPP;
	}
}

static void ali_sw_ethtool_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	switch(stringset) {
	case ETH_SS_STATS:
		memcpy(data, *ali_sw_ethtool_gstrings_stats, sizeof(ali_sw_ethtool_gstrings_stats));
		break;
	}	
}

static struct net_device_stats * ali_sw_get_stats(struct net_device *dev) {
	struct net_device_stats * stats;
	p_p0_private priv = netdev_priv(dev);
	u32 flags;

	//spin_lock_irqsave(&sw->lock, flags);
	CMD_LOCK_IRQ(&priv->cmd_lock, flags);
	stats = _ali_sw_get_stats(dev);
	//spin_unlock_irqrestore(&sw->lock, flags);
	CMD_UNLOCK_IRQ(&priv->cmd_lock, flags);
	return stats;
}

static struct net_device_stats * _ali_sw_get_stats(struct net_device *dev)
{
	p_p0_private priv = netdev_priv(dev);
	struct net_ali_sw *sw = priv->p_switch;
	u8 port_no = 1;
	u32 tmp;

 	if (netif_running(dev) && netif_device_present(dev))
 	{
 		SW_W8(SysPortAccess, 0);
 		
		tmp = SW_R16(IPCksFailCnt);
		SW_W16(IPCksFailCnt, 0);
		priv->mac_stats.rx_ip_chksum_err_from_reg += tmp;

		tmp = SW_R16(IPPayloadCksFailCnt);
		SW_W16(IPPayloadCksFailCnt, 0);
		priv->mac_stats.rx_ip_data_chksum_err_from_reg+= tmp;

		for(port_no = 1; port_no<MAX_PORT_NUM; port_no++)
		{
			SW_W8(SysPortAccess, port_no);
			
			tmp = SW_R16(RuntPktCnt);
			sw->sw_stats.rx_runt_pkts_port[port_no-1] += tmp;
			SW_W16(RuntPktCnt, 0);
			
			tmp = SW_R16(LongFrameCnt);
			sw->sw_stats.rx_long_frame_port[port_no-1] += tmp;
			SW_W16(LongFrameCnt, 0);

			tmp = SW_R16(CRCErrPktsCnt);
			sw->sw_stats.rx_crc_err_pkts_port[port_no-1] += tmp;
			SW_W16(CRCErrPktsCnt, 0);
			
			tmp = SW_R16(AlignErrCnt);
			sw->sw_stats.rx_alig_err_frame_port[port_no-1] += tmp;
			SW_W16(AlignErrCnt, 0);
			
			tmp = SW_R32(PortSentCnt);
			sw->sw_stats.tx_pkts_port[port_no-1] += tmp;
		}
		
 	}
	

	return &dev->stats;
}



static void ali_sw_get_ethtool_stats (struct net_device *dev, struct ethtool_stats *estats, u64 *data)
{
	p_p0_private priv = netdev_priv(dev);
	struct net_device_stats p0_net_stats;
	struct p0_txrx_stats p0_mac_stats;
	struct sw_txrx_stats sw_port_stats;
	long i;
	u32 count1;
	u32 count2;
	u32 count3;

	ali_sw_get_stats(dev);
	p0_net_stats = dev->stats;
	p0_mac_stats = priv->mac_stats;
	sw_port_stats = priv->p_switch->sw_stats;
	
	count1 = sizeof(struct net_device_stats)/4;
	count2 = sizeof(struct p0_txrx_stats)/4;
	count3 = sizeof(struct sw_txrx_stats)/4;

	if ((count1 + count2 + count3) != ALI_SW_STATS_LEN)
	{
		sw_error("ali_sw_get_ethtool_stats: error!");
		return;
	}
	
	for(i = 0; i < ALI_SW_STATS_LEN; i++)
	{
		if (i < count1)
		{
			data[i] = ((unsigned long *)&p0_net_stats)[i];
		}
		else if(i < (count1+count2))
		{
			data[i] = ((unsigned long *)&p0_mac_stats)[i-count1];
		}
		else
		{
			data[i] = ((unsigned long *)&sw_port_stats)[i-(count1+count2)];
		}
	}

}

int sw_eth_validate_addr(struct net_device *dev) {
    if (!is_valid_ether_addr(dev->dev_addr))
        return -EADDRNOTAVAIL;
    return 0;
}
static int sw_set_mac_address(struct net_device *netdev, void *p) {
    struct sockaddr *addr = p;
	p_p0_private priv = netdev_priv(netdev);
    int j;
	u32 flags;
    if (!is_valid_ether_addr(addr->sa_data))
        return -EADDRNOTAVAIL;
	CMD_LOCK_IRQ(&gp_p0->cmd_lock, flags);
    memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
    memcpy(stb_mac_addr, addr->sa_data, netdev->addr_len);
    printk("Ali Sw Mac len %d\n", netdev->addr_len);
    printk("set Ali Sw Mac: ");
	for(j=0; j<ETH_ALEN; j++) {
		if(j==ETH_ALEN-1)
			printk("%02x\n", stb_mac_addr[j]);
		else
			printk("%02x-", stb_mac_addr[j]);
	}
    if ((priv->filter_mode == FILTER_PERFECT_MODE) || \
        (priv->filter_mode == FILTER_ALLMULTI_MODE)) {
        p0_set_perfect_filter(priv, 0);
    }
	CMD_UNLOCK_IRQ(&gp_p0->cmd_lock, flags);
    return 0;
}
/****************************************************************************
-->	common net_device driver structure...
****************************************************************************/
static const struct net_device_ops ali_sw_dev_ops = {
	.ndo_validate_addr		= eth_validate_addr,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)) /*zmiao*/
	.ndo_set_rx_mode = p0_set_rx_mode,
#else 
	.ndo_set_multicast_list	= p0_set_rx_mode,
#endif
	.ndo_get_stats		= ali_sw_get_stats,
	.ndo_do_ioctl			= ali_sw_ioctl,
	
#if ALI_SW_VLAN_TAG_USED 
	.ndo_vlan_rx_register	= p0_vlan_rx_register,
#endif
	.ndo_tx_timeout		= ali_sw_tx_timeout,
	.ndo_start_xmit		= p0_start_xmit,
	.ndo_open			= ali_sw_open,
	.ndo_stop			= ali_sw_close,
	.ndo_do_ioctl			= ali_sw_ioctl,
    .ndo_validate_addr  = sw_eth_validate_addr,
    .ndo_set_mac_address    = sw_set_mac_address,
};

static const struct ethtool_ops ali_sw_ethtool_ops = {
	.get_drvinfo		= ali_sw_get_drvinfo,
	//.get_sset_count	= ali_sw_get_sset_count,
	.get_settings		= ali_sw_get_settings,
	.set_settings		= ali_sw_set_settings,
	.nway_reset		= ali_sw_nway_reset,
	.get_link			= ethtool_op_get_link,
	.get_msglevel		= ali_sw_get_msglevel,
	.set_msglevel		= ali_sw_set_msglevel,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)) /*zmiao*/
#else
	.get_rx_csum		= ali_sw_get_rx_csum,
	.set_rx_csum		= ali_sw_set_rx_csum,
	.set_tx_csum		= ethtool_op_set_tx_csum,
	.set_sg			= ethtool_op_set_sg,
	.set_tso			= ethtool_op_set_tso,
#endif
	.get_regs			= ali_sw_get_regs,
	.get_strings		= ali_sw_ethtool_get_strings,
	.get_sset_count	= ali_sw_ethtool_get_sset_count,	
	.get_ethtool_stats	= ali_sw_get_ethtool_stats,
};

/****************************************************************************
	in device_attach, basically, it does things like...
		1. allocate net_device, including the private structure(p0_private, net_ali_sw, etc.)
		2. set value to members of net_device
		3. register net_device

data struct connection:
	net_device <--> p0_private <--> net_ali_sw
									|-----net_ali_sw_port[0] --> p0_private
									|-----net_ali_sw_port[1] --> p0_private
									|-----net_ali_sw_port[2] --> p0_private
									|-----net_ali_sw_port[3] --> p0_private							
data struct in memory:
	+----------+
	| net_device   |
	+----------+
	| p0_private   |-------
	+----------+           |           +--------------+
					     ------>| net_ali_sw          |
					                  +--------------+
					                  |net_ali_sw_port*4|
					                  +--------------+
					                  
as diagram shows, to access each net_ali_sw_port's data, must get 'net_ali_sw' first.
****************************************************************************/
static int ali_sw_dev_attach(void) {
	struct net_device *dev;
	struct p0_private *priv;
	struct net_ali_sw *sw;
	struct net_ali_sw_port *sw_port;
	int re;
	u8 i;
	UINT32 reg32;

	dev = alloc_etherdev(sizeof(struct p0_private));
	if (!dev) {	
		sw_error("fail to alloc_etherdev!\n");
		return -ENOMEM;
	}

	priv = netdev_priv(dev);
	gp_p0 = priv;

	sw = kzalloc(sizeof(struct net_ali_sw), GFP_KERNEL);
	if (!sw) {
		sw_error("fail to allocate net_ali_sw!\n");
		return -ENOMEM;	
	}
	priv->p_switch = sw;
	gp_sw = sw;
	sw->p_P0 = priv;
	
	for(i=0; i < MAX_PORT_NUM; i++) {
		sw_port = &(sw->port_list[i]);
		sw_port->p_P0 = priv;
		sw_port->port_no = i;
		sw_port->sw = gp_sw;
	}

	priv->dev = dev;
    priv->filter_mode = FILTER_PROMISC; 
	priv->io_base = ali_sw_base;
	priv->irq_num = ali_sw_irq;
	priv->tx_skb_wr = 0;
	priv->tx_skb_rd = 0;
	priv->avail_desc_num = ALI_SW_TX_DESC_NUM - 1;
	priv->msg_enable = (ali_sw_debug_level < 0 ? ALI_SW_DEF_MSG_ENABLE : ali_sw_debug_level);

	spin_lock_init (&priv->p_switch->lock);
	spin_lock_init (&priv->cmd_lock);
	memcpy(dev->dev_addr, stb_mac_addr, ETH_ALEN);

	dev->irq = priv->irq_num;
	dev->watchdog_timeo = ALI_SW_TX_TIMEOUT;

	printk("SW use NAPI\n");
	netif_napi_add(dev, &priv->napi, sw_napi_poll, 128);

	dev->netdev_ops = &ali_sw_dev_ops;
	dev->ethtool_ops = &ali_sw_ethtool_ops;

#if ALI_SW_VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX |NETIF_F_HW_VLAN_RX;
#endif

	printk("dev features %x\n", (u32)dev->features);
	if (dev->features & NETIF_F_SG) {
		printk("dev support SG\n");
	}
	if (dev->features & NETIF_F_FRAGLIST) {
		printk("dev support FRAGLIST\n");
	}
	if (dev->features & NETIF_F_IP_CSUM) {
		printk("dev support IP_CSUM\n");
	}
	if (dev->features & NETIF_F_TSO) {
		printk("dev support TSO\n");
	}
	if(p0_sg) {
		dev->features |= NETIF_F_SG | NETIF_F_FRAGLIST;
	}

	if(p0_tx_csum) {	
		dev->features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM; 
	}

	if(p0_tso) {	
		dev->features |= NETIF_F_TSO;
		dev->features |= NETIF_F_TSO6;
		netif_set_gso_max_size(dev, 65536);
	}
	
	if(p0_ufo) {
		dev->features |= NETIF_F_UFO | NETIF_F_HW_CSUM;
	}

	sw_info("%s, %d, promisc?%c", __FUNCTION__, __LINE__, dev->flags&IFF_PROMISC?'Y':'N');//0907
    if(priv->filter_mode == FILTER_PERFECT_MODE) {
	dev->flags |= IFF_MULTICAST;
    } else if (priv->filter_mode == FILTER_ALLMULTI_MODE) {
	dev->flags |= IFF_ALLMULTI;	
    } else {
	dev->flags |= IFF_PROMISC|IFF_ALLMULTI;	
    }
	sw_info("%s, %d, promisc?%c", __FUNCTION__, __LINE__, dev->flags&IFF_PROMISC?'Y':'N');//0907

	re = register_netdev(dev);//always put register_xxx at the end of initializaiton, and put regiset_isr at the end of opening.
	if (re) {
		goto err_out_free;
	}


	sw_trace("ALi switch attached!gp_p0=0x%x, gp_sw=0x%x", (u32)gp_p0, (u32)gp_sw);

	printk("set SOC registers\n");
	reg32 = (*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0x6c)); 
	reg32 &=  0x7fffffff;
	*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0x6c) = reg32; 
	reg32 = (*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0x6c)); 
	printk("SOC registers 6c %x\n", reg32);

	reg32 = (*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0xa8)); 
	reg32 &= 0xfffffe00;
	reg32 |= 0x109;
	*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0xa8) = reg32; 
	printk("SOC registers a8 %x\n", reg32);

	reg32 = (*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0x8c)); 
	reg32 |= 0x00100000;
	*(volatile UINT32 *)(ali_sw_base - 0x2c000 + 0x8c) = reg32; 
	printk("SOC registers 8c %x\n", reg32);

	return 0;

err_out_free:
	free_netdev(dev);
	return re;
}

void init_stb_mac(void) {
    u32 board_info_addr = (u32)(phys_to_virt)PRE_DEFINED_ADF_BOOT_START;
    ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO*)board_info_addr;
    if (is_valid_ether_addr(info->macinfo.phyaddr1)) {
    memcpy(stb_mac_addr, info->macinfo.phyaddr1, sizeof(u8)*ETH_ALEN);
    } else {
        printk("boot mac isn't valid, use default stb mac\n");
    }
    return;
}
static int __init ali_sw_module_init(void)
{
    int j = 0;
#ifdef CONFIG_MIPS
    return 0;
#endif    
#ifdef ALI_SW_DEBUG1
	printk(KERN_WARNING "enter ALi Switch Driver initialization!\n");
	printk(KERN_WARNING "HZ in this system is %d, jiffies now is %lu\n", HZ, jiffies);
	printk("-->" "%s, Compiled: %s, %s\n", __FUNCTION__, __DATE__, __TIME__);
#endif
    init_stb_mac();
	for(j=0; j<ETH_ALEN; j++)	{
		if(j==ETH_ALEN-1) {
			printk("%02x\n", stb_mac_addr[j]);
        } else {
			printk("%02x-", stb_mac_addr[j]);
        }
	}
	return ali_sw_dev_attach();
}


void ali_sw_module_exit(void)
{
	printk("ALi Switch Driver exit!\n");
	return;	
}

void hectrl_bymac(u32 regpos, u16 value)
{
	u32 flags;
	CMD_LOCK_IRQ(&gp_p0->cmd_lock, flags);
	mac_mdio_write(gp_p0, P3_PHY_ADDRESS, regpos, value);
	CMD_UNLOCK_IRQ(&gp_p0->cmd_lock, flags);
}

#if 0
static void __init parse_mac_addr(char *macstr) {
	int i, j;
	unsigned char result, value;
    u8 tmp_mac_addr[ETH_ALEN];

	if(strlen(macstr) < (12+5)) {//12 digit + 5 ':'
		return;
    }
    printk("%s macstr %s\n", __FUNCTION__, macstr);
    memset(tmp_mac_addr, 0, (sizeof(u8)*ETH_ALEN));	
	for (i = 0; i < 6; i++) {
		if (i != 5 && *(macstr + 2) != ':') {
			return;
        }
		
		result = 0;	
		for (j = 0; j < 2; j++) {
			if (isxdigit(*macstr) && \
               (value = isdigit(*macstr) ? *macstr - '0' : toupper(*macstr) - 'A' + 10) < 16) {
				result = result * 16 + value;
				macstr++;
			} else {
				return;
            }
		}
		macstr++;
		tmp_mac_addr[i] = result;
	}
    memcpy(stb_mac_addr, tmp_mac_addr, sizeof(u8)*ETH_ALEN);
    use_default_mac = 0;
    return;
}

static int __init program_setup_kmac(char *s) {
	if(s) {
		sw_info("cmdline: mac = %s\n", s);
		parse_mac_addr(s);
	}
	return 0;
}
__setup("mac=", program_setup_kmac);
#endif
module_init(ali_sw_module_init);
module_exit(ali_sw_module_exit);

#ifdef SW_NAPI_DEBUG
MODULE_LICENSE("GPL");
#else
MODULE_LICENSE("proprietary");
#endif


