/*****************************************************************************
*	Copyright (C)2010 Ali Corporation. All Rights Reserved.
*
*	File: eth_gmac.c
*
*	Description: ALi Linux GMAC Driver for kernel v2.6:

*	History:
*	Date				Athor		Version		Reason
* ===========================================================
*	2012.1			Peter.Li 		0.1
*
*
******************************************************************************/
#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ctype.h>
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
#include <linux/mii.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/crc32.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <asm/irq.h>
#include <asm/dma.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <linux/skbuff.h>
#include <asm/mach-ali/m6303.h>

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0))
#include <asm-generic/dma-coherent.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#else
#include <dma-coherent.h>
#endif
#include <ali_reg.h>
#include <ali_dma.h>
#include <ali_board_config.h>
#include <alidefinition/adf_boot.h>
#include <mach-ali/m36_gpio.h>
#include "eth_gmac.h"
#include "eth_reg.h"
#include "gmac_util.h"


#define GMAC_DRV_NAME "ALi Ethernet GMAC"
#define GMAC_DRV_VER "Ver 0.1"

//#define USE_GPHY 

#define TSO_DEBUG
#define HWCKS_DEBUG

#define C3821       0x3821      
#define C3921       0x3921

u16 g_chip_id = 0;

/* add those define to separate head file */
typedef struct _cmd_param {
    u32 cmd; 
    u32 reg;
    u32 value;
    void * data;
} cmd_param_t;

typedef struct _ali_reg_t {
    u32 cmd;
    u32 phy_addr;
    u32 val;
} ali_reg_t;

#define READ_MAC_REG   0x1
#define READ_SOC_REG   0x2
#define WRITE_MAC_REG  0x3
#define WRITE_SOC_REG  0x4
#define DEBUG_DRIVER_LEVEL 0x5
#define TX_PKTS        0x6
/* end */
u32 LIMIT_PKTS;

int lock_debug = 0;
//#define LOCK_DEBUG
#ifdef LOCK_DEBUG
#define ALI_LOCK(lock)  {if(lock_debug) {printk("%s try to get lock %x\n", __FUNCTION__, (UINT32)(lock));}\
                          spin_lock(lock);\
                         if(lock_debug) {printk("%s get lock %x\n", __FUNCTION__, (UINT32)(lock));}}
                        
#define ALI_UNLOCK(lock) {spin_unlock(lock); if(lock_debug) {printk("%s unlock lock %x\n", __FUNCTION__, (UINT32)(lock));}}
#define ALI_LOCK_IRQ(lock, flag)  \
                 {if(lock_debug) {printk("%s try to get irq lock %x\n", __FUNCTION__, (UINT32)(lock));}\
                   spin_lock_irqsave(lock,flag);\
                  if(lock_debug) {printk("%s get irq lock %x\n", __FUNCTION__, (UINT32)(lock));}}
#define ALI_UNLOCK_IRQ(lock, flag) \
				 {spin_unlock_irqrestore(lock, flag);\
                  if(lock_debug) {printk("%s unlock irq lock %x\n",__FUNCTION__, (u32)(lock));}}
#else
#define ALI_LOCK(lock)  {spin_lock(lock);}
#define ALI_UNLOCK(lock)  {spin_unlock(lock);}
#define ALI_LOCK_IRQ(lock, flag)  \
                 {spin_lock_irqsave(lock,flag);}
#define ALI_UNLOCK_IRQ(lock, flag) \
				 {spin_unlock_irqrestore(lock, flag);}
#endif 

UINT32 gmac_base = _GMAC_BASE;
static UINT32 gmac_irq = _GMAC_IRQ;
u8 stb_gmac_mac[] = {0x00, 0x90, 0xe6, 0x00, 0x00, 0x03};
static UINT32 gmac_mac_hi16 = 0;
static UINT32 gmac_mac_lo32 = 0;
UINT32 use_default_mac = 1;

static UINT32 gmac_phy_addr = GMAC_PHY_ADDR;
static UINT32 gmac_rx_mode = GMAC_RX_MODE_NAPI; //GMAC_RX_MODE_TASKLET;
static UINT32 gmac_rx_filter = GMAC_RX_FILTER_PERFECT;

static u32  mac_clk = 198 * 1024 * 1024;  // 198M
static UINT32 gmac_timer_frq = 0x840000; // gmac_timer_frq / 198M = seconds 
#define MAX_LINK_CHANGE_IN_5SEC 3
static u32  gmac_5sec = 0;
static u32  gmac_pll_time = 0;

#ifdef USE_GPHY
static int gmac_phy_mode = ETH_PHY_MODE_RGMII;
#else
static int gmac_phy_mode = ETH_PHY_MODE_RMII;
#endif

unsigned long dbg_runtime_val = 1;

/* hw capability */
static int gmac_rx_csum = true;
static int gmac_tx_csum = true;
static int gmac_sg = true;
static int gmac_tso = true;
static int gmac_ufo = true;

static int gmac_debug = -1;

pgmac_private g_pgmac;

static int crc_det_err = 0;
void housekeeper_tasklet(unsigned long para) ;
DECLARE_TASKLET(gmac_tasklet, housekeeper_tasklet, 0);

static void gmac_free_tx_skb(pgmac_private priv, int lock);
static struct net_device_stats *gmac_get_stats(struct net_device *dev);
void show_rx_desc(prx_desc rx_desc, int no) ;
void enable_gmac_isr (pgmac_private pgmac, int lock);
void phy_reg_set(pgmac_private pgmac) ;
void gmac_link_established(pgmac_private pgmac);
void do_dll_pd_reset(pgmac_private pgmac) ;
void mac_tx_sts(pgmac_private pgmac, ptx_desc desc) ;
static void gmac_print(unsigned char *p, unsigned short len) {
	int i;
	for(i=0; i<len; i++) {
		if (i%16 == 0) {
			printk("\n0x%08x:  ", (UINT32)(p+i));
		}

		printk("%02x ", *(p+i));
	}
	printk("\n");
    return;
}

void show_rx_desc(prx_desc rx_desc, int n) {
    struct packet_head * pHdr;
    if(!rx_desc) {
        return;
    }
    pHdr = &rx_desc->pkt_hdr; 
    if (pHdr->FirstSegment && pHdr->LastSegment) {
    //    return;
    }
    ali_info("--> Begin rx_desc \n");
    ali_info("rx_desc %x no %d", (u32)rx_desc, n);
    ali_info("pkt_hdr:\n");
    ali_info("SegLength %d  TCPChksum %d IPChksum %d\n",\
        pHdr->SegLength, pHdr->TCPChksum, pHdr->IPChksum);
    ali_info("PPPoE %d  VLAN %d WatchdogTimeout %d\n",\
        pHdr->PPPoE, pHdr->VLAN, pHdr->WatchdogTimeout);
    ali_info("FrameType %d  PhysicalLayerError %d InvalidLength %d\n",\
        pHdr->FrameType, pHdr->PhysicalLayerError, pHdr->InvalidLength);
    ali_info("FAE %d  CRC %d LateCollision %d\n",\
        pHdr->FAE, pHdr->CRC, pHdr->LateCollision);
    ali_info("Long %d  Runt %d  ES %d\n",\
        pHdr->Long, pHdr->Runt, pHdr->ES);
    ali_info("BF %d  PF %d  MF %d\n",\
        pHdr->BF, pHdr->PF, pHdr->MF);
    ali_info("Dribble %d  FirstSegment %d  LastSegment %d\n",\
        pHdr->Dribble, pHdr->FirstSegment, pHdr->LastSegment);
    ali_info("IPFrag %d  IP6Frame %d  IPFrame %d\n",\
        pHdr->IPFrag, pHdr->IP6Frame , pHdr->IPFrame );
    ali_info("l4_chs %d l3_chs %d vlan_tag %d PacketLength %d pkt_buf_dma %x\n",\
    rx_desc->l4_chs, rx_desc->l3_chs, rx_desc->vlan_tag, rx_desc->PacketLength, rx_desc->pkt_buf_dma);
    ali_info("<-- end rx_desc \n");
    ali_info("rx desc ----------> \n");
    gmac_print((char*)rx_desc, 64);
    ali_info("end rx desc ------->\n");
    return;
}

void separate_mac_addr(u8 * mac_addr, u32 *mac_hi16, u32 *mac_lo32) {
    u32 tmp1=0, tmp2=0, tmp3=0, tmp4=0;
	tmp1 = mac_addr[0];
	tmp2 = mac_addr[1];
	tmp3 = mac_addr[2];
	tmp4 = mac_addr[3];
    tmp2 = tmp2 << 8;
    tmp3 = tmp3 << 16;
    tmp4 = tmp4 << 24;
	/* set mac address. */
    *mac_lo32 = tmp1 | tmp2 | tmp3 | tmp4; 
	tmp1 = mac_addr[4];
	tmp2 = mac_addr[5];
    tmp2 = tmp2 << 8;
    *mac_hi16 = tmp1 | tmp2;
    return;
} 

//#define TEST_JUMBO_FRAME
#ifdef TEST_JUMBO_FRAME
static inline struct sk_buff * tmp_alloc_skb_ip_align (struct net_device *dev, unsigned int length) {
    struct sk_buff * skb = netdev_alloc_skb(dev, length + NET_IP_ALIGN);
    if (NET_IP_ALIGN && skb)
        skb_reserve(skb, NET_IP_ALIGN);
    return skb;
}
#endif

#ifndef HW_MDIO
#define	mac_mdio_delay(mdio_addr)	GMAC_R32(mdio_addr)
static void mac_mdio_sync(pgmac_private pgmac) {
	int i;
	for (i = 32; i >= 0; i--) {
		GMAC_W32(MiiMR1, MiiMdioWrite1|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
		GMAC_W32(MiiMR1, MiiMdioWrite1|MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
    }
}
#endif //HW_MDIO

int mac_mdio_read(struct net_device *dev, int phy_addr, int reg_addr) {
#ifdef HW_MDIO
	u16 tmp_16;
	UINT32 tmp_32;
	UINT32 addr;
	
	addr = ((phy_addr << MiiPhyAddrOff) & MiiPhyAddrMask)|((reg_addr << MiiRegAddrOff) & MiiRegAddrMask);
	
	tmp_32 = GMAC_R32(MiiMR2);
	tmp_32 &= ~(MiiPhyAddrMask|MiiRegAddrMask|MiiOpMask);
	
	GMAC_W32(MiiMR2, (tmp_32|addr|MiiOpRead|MiiTransStart));
	
	do{
		tmp_32 = GMAC_R32(MiiMR2);
	}while(tmp_32&MiiTransStart);
	
	tmp_16 = GMAC_R16(MdioR);
	
	return (UINT32)tmp_16;
		
#else
	UINT32 retval = 0;
	int i,	data_val = 0;
	UINT32 mii_cmd = (0xf6 << 10) | (phy_addr << 5) | reg_addr;
	UINT32 tmp_u32, cmd_u32;

	mac_mdio_sync(pgmac);

	/* Shift the read command bits out. */
	for (i = 15; i >= 0; i--) {
		data_val = (mii_cmd & (1 << i)) ? MiiMdioOut : 0;

		GMAC_W32(MiiMR1, MiiMdioDir|data_val|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
		
		GMAC_W32(MiiMR1, MiiMdioDir|data_val|MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}

	/* Read the two transition, 16 data, and wire-idle bits. */
	for (i = 19; i > 0; i--) {
		GMAC_W32(MiiMR1, 0|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		cmd_u32 = GMAC_R32 (MiiMR1);
		retval = (retval << 1) | ((cmd_u32 & MiiMdioIn) ? 1 : 0);

		GMAC_W32(MiiMR1, MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}
	return (retval >> 1) & 0xffff;
#endif //HW_MDIO
}


static void
mac_mdio_write (struct net_device *dev, int phy_addr, int reg_addr, int value)
{
#ifdef HW_MDIO
	UINT32 tmp_32;
	UINT32 addr;
			
	GMAC_W16(MdioW, (u16)value);
		
	addr = ((phy_addr << MiiPhyAddrOff) & MiiPhyAddrMask)|((reg_addr << MiiRegAddrOff) & MiiRegAddrMask);
	
	tmp_32 = GMAC_R32(MiiMR2);
	tmp_32 &= ~(MiiPhyAddrMask|MiiRegAddrMask|MiiOpMask);
	
	GMAC_W32(MiiMR2, (tmp_32|addr|MiiOpWrite|MiiTransStart));
			
	do{
		tmp_32 = GMAC_R32(MiiMR2);
	}while(tmp_32&MiiTransStart);
				
#else

	UINT32 mii_cmd = (0x5002 << 16) | (phy_addr << 23) | (reg_addr << 18) | value;
	int i;
	
	mac_mdio_sync(pgmac);

	/* Shift the command bits out. */
	for (i = 31; i >= 0; i--)
	{
		int data_val = (mii_cmd & (1 << i)) ? MiiMdioWrite1 : MiiMdioWrite0;

		GMAC_W32(MiiMR1, data_val|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		GMAC_W32(MiiMR1, data_val|MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}

	/* Clear out extra bits. */
	for (i = 2; i > 0; i--) 
	{
		GMAC_W32(MiiMR1, 0|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		GMAC_W32(MiiMR1, MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}
	
#endif //HW_MDIO
}

void mac_chip_rst (void) {
	u8 tmp_u8;
    UINT32 txrxcr2;

	GMAC_W8(SCR, SCRReset|GMAC_R8(SCR));	
	do{
		ali_info("->");	
		tmp_u8 = GMAC_R8(SCR);
	}while(tmp_u8 & SCRReset);
#ifdef TEST_JUMBO_FRAME
    /* for jumbo frame, watchdog must be 1*/
    txrxcr2 = GMAC_R32(TxRxCR2);
    GMAC_W32(TxRxCR2, (txrxcr2|RxWatchdogDis)); 
#endif
}

void mac_cnt_init(pgmac_private pgmac) {
	pgmac->cur_isr = 0;
	pgmac->isr_mask = GMAC_INTERRUPT_MASK; 
	//pgmac->acquired_isr = false;
	
	pgmac->rx_wptr = GMAC_RX_DESC_NUM -1;
	pgmac->rx_bptr= 0;
	pgmac->tx_wptr= 0;	
	
	pgmac->phy_reset = false;
	pgmac->auto_n_completed = false;
	pgmac->link_established = false;
	pgmac->transmit_okay = false;
#ifndef USE_GPHY
    pgmac->blink_light = 0;
    pgmac->in_blink = 0;
#endif

	pgmac->pause_frame_rx = false;
	pgmac->pause_frame_tx = false;

#if GMAC_VLAN_TAG_USED
	pgmac->vlan_tag_remove = false;
#endif //GMAC_VLAN_TAG_USED
}

int mac_rx_refill(pgmac_private pgmac) {
	struct net_device *dev = pgmac->dev;
    struct sk_buff *skb;
    prx_desc rx_desc;
	int i;
	
	for(i = 0; i < GMAC_RX_DESC_NUM; i ++) {
		rx_desc = &pgmac->rx_desc[i];

		if(!pgmac->rx_skb[i]) {
			skb = netdev_alloc_skb(dev, GMAC_BUF_SZ);
            //ali_info("netdev_alloc_skb %d  0x%x\n", i, skb);
			if (!skb) {
				goto refill_err_out;
            }

			//skb_reserve(skb, NET_IP_ALIGN);
			rx_desc->pkt_buf_dma = dma_map_single((struct device *)NULL, skb->data,
						 GMAC_BUF_SZ, DMA_FROM_DEVICE);
			rx_desc->pkt_buf_dma = __CTDADDRALI(rx_desc->pkt_buf_dma);

			ali_trace(9, "i=%d, pkt_buf_dma=0x%x, skb->data=0x%x\n", i, rx_desc->pkt_buf_dma, (UINT32)skb->data);
			pgmac->rx_skb[i] = skb;
		}
	}
	return 0;
refill_err_out:
	ali_info("%s: fatal error, alloc %d skb failed.",__FUNCTION__, i);
	return -ENOMEM;
}

#ifdef TEST_JUMBO_FRAME 
int mac_rx_jumbo_refill(pgmac_private pgmac) {
	struct net_device *dev = pgmac->dev;
    struct sk_buff *skb;
    prx_desc rx_desc = NULL;
    int bufsz = GMAC_JUMBO_SKB_LINEAR_SIZE;
	int i;
    struct page * page;
	
	for(i = 0; i < GMAC_RX_DESC_NUM-1; i ++) {
		rx_desc = &pgmac->rx_desc[i];
        skb = pgmac->rx_skb[i];
		if(skb) {
            skb_trim(skb, 0);
            goto check_page;
        }

        skb = tmp_alloc_skb_ip_align(dev, bufsz);
        //skb = netdev_alloc_skb_ip_align(dev, bufsz);
        if (unlikely(!skb)) {
            /* Better luck next round */
            continue;
        }
        pgmac->rx_skb[i] = skb;
check_page:
        page = pgmac->pages[i];
        if(!page) { 
            page = alloc_page(GFP_ATOMIC);
            if (unlikely(!page)) {
                break;
            }
            pgmac->pages[i] = page;
        }
#if 0
        if (rx_desc->pkt_buf_dma) {
            printk("%s bug on rx_desc->pkt_buf_dma %x isn't NULL\n", \
                    __FUNCTION__, rx_desc->pkt_buf_dma);
        }
#endif
        rx_desc->pkt_buf_dma = dma_map_page((struct device *)NULL, page,\
                    0, GMAC_JUMBO_FRAG_SZ, DMA_FROM_DEVICE); 
        rx_desc->pkt_buf_dma = __CTDADDRALI(rx_desc->pkt_buf_dma);

        ali_trace(9, "idx %d rx_desc %x skb %x page %x pkt_buf_dma %x\n", i, \
                  (u32)rx_desc, (u32)skb,  (u32)page, rx_desc->pkt_buf_dma);
        if(dma_mapping_error((struct device*)NULL, rx_desc->pkt_buf_dma)) {
            ali_info("%s dma mapping 0x%xerror \n", __FUNCTION__, rx_desc->pkt_buf_dma);
        }
	}
	return 0;
}
#endif

int verify_buf(pgmac_private pgmac, void * buf, dma_addr_t dma, int size) {
    dma_addr_t tmp_dma;
    u8 *tmp_buf=NULL;
    tmp_dma = dma + size;
    if (((u32)pgmac->rx_desc_dma < (u32)dma) && ((u32)dma < ((u32)pgmac->rx_desc_dma + (GMAC_DESC_SZ*GMAC_TX_DESC_NUM)))) {
        printk("WARNING!!! dma overlap rx_desc_dma 0x%x dma 0x%x\n", (u32)pgmac->rx_desc_dma, (u32)dma); 
        return 0;
    }
    if (((u32)pgmac->rx_desc_dma < (u32)tmp_dma) && ((u32)tmp_dma < ((u32)pgmac->rx_desc_dma + (GMAC_DESC_SZ*GMAC_TX_DESC_NUM)))) {
        printk("WARNING!!! dma overlap rx_desc_dma 0x%x tmp_dma 0x%x\n", (u32)pgmac->rx_desc_dma, (u32)tmp_dma); 
        return 0;
    }
    tmp_buf = buf + size;
    if (((u32)pgmac->rx_desc < (u32)dma) && ((u32)buf < ((u32)pgmac->rx_desc + (GMAC_DESC_SZ*GMAC_TX_DESC_NUM)))) {
        printk("WARNING!!! dma overlap rx_desc 0x%x buf 0x%x\n", (u32)pgmac->rx_desc, (u32)buf); 
        return 0;
    }
    if (((u32)pgmac->rx_desc < (u32)tmp_dma) && ((u32)tmp_buf < ((u32)pgmac->rx_desc + (GMAC_DESC_SZ*GMAC_TX_DESC_NUM)))) {
        printk("WARNING!!! dma overlap rx_desc 0x%x tmp_buf 0x%x\n", (u32)pgmac->rx_desc, (u32)tmp_buf); 
        return 0;
    }
    return 1;
}  
void mac_alloc_rings(pgmac_private pgmac) {
	void *rx_desc = NULL;
	void *tx_desc = NULL;	
	void *setup_buf = NULL;

#ifndef TEST_RX_DESC_BUG 
	rx_desc = dma_alloc_coherent((struct device *)NULL, (GMAC_DESC_SZ * GMAC_RX_DESC_NUM),
		&pgmac->rx_desc_dma, GFP_KERNEL);
	tx_desc = dma_alloc_coherent((struct device *)NULL, (GMAC_DESC_SZ * GMAC_TX_DESC_NUM),
		&pgmac->tx_desc_dma, GFP_KERNEL);
	setup_buf = dma_alloc_coherent((struct device *)NULL, SETUP_FRAME_SZ,
		&pgmac->setup_buf_dma, GFP_KERNEL);

	pgmac->rx_desc_dma = __CTDADDRALI(pgmac->rx_desc_dma);
	pgmac->tx_desc_dma = __CTDADDRALI(pgmac->tx_desc_dma);
	pgmac->setup_buf_dma = __CTDADDRALI(pgmac->setup_buf_dma);

	memset(rx_desc, 0, (GMAC_DESC_SZ * GMAC_RX_DESC_NUM));
	memset(tx_desc, 0, (GMAC_DESC_SZ * GMAC_TX_DESC_NUM));
	memset(setup_buf, 0, SETUP_FRAME_SZ);

	pgmac->rx_desc = (prx_desc)rx_desc;
	pgmac->tx_desc = (ptx_desc)tx_desc;
	pgmac->setup_buf = (u8 *)setup_buf;
#else
	pgmac->rx_desc_dma = 0x06000000; 
	pgmac->tx_desc_dma = 0x06001000; 
	pgmac->setup_buf_dma = 0x06002000; 
	pgmac->rx_desc = (prx_desc)0xC6000000;
	pgmac->tx_desc = (ptx_desc)0xC6001000;;
	pgmac->setup_buf = (u8 *)0xC6002000;
#endif

#ifdef TEST_DEBUG
	ali_info("%s()=> rx_desc = 0x%x, pgmac->rx_desc_dma = 0x%x size %d\n", __FUNCTION__, (UINT32)rx_desc, (UINT32)pgmac->rx_desc_dma,
                GMAC_DESC_SZ * GMAC_RX_DESC_NUM);
	ali_info("%s()=> tx_desc = 0x%x, pgmac->tx_desc_dma = 0x%x size %d\n", __FUNCTION__, (UINT32)tx_desc, (UINT32)pgmac->tx_desc_dma,
                GMAC_DESC_SZ * GMAC_RX_DESC_NUM);
#endif
    return;
}

void mac_desc_clean(pgmac_private priv) {
	unsigned i;
    struct sk_buff *skb;
	prx_desc rx_desc;
	ptx_desc desc;

	for (i = 0; i < GMAC_RX_DESC_NUM; i++) {
		if (priv->rx_skb[i]) {
			rx_desc = &priv->rx_desc[i];
#ifdef TEST_JUMBO_FRAME
            dma_unmap_page((struct device *)NULL, __DTCADDRALI(rx_desc->pkt_buf_dma), \
                           GMAC_JUMBO_FRAG_SZ, DMA_FROM_DEVICE);
#else
			dma_unmap_single((struct device *)NULL, __DTCADDRALI(rx_desc->pkt_buf_dma),
				GMAC_BUF_SZ, DMA_FROM_DEVICE);
#endif
//            ali_warn("%s dev_kfree_skb %d skb 0x%x\n",__FUNCTION__, i, priv->rx_skb[i]);
			dev_kfree_skb_any(priv->rx_skb[i]);
			priv->rx_skb[i] = NULL;
		}
        if (priv->pages[i]) {
            /* if a page added to skb, priv->pages has been set to NULL */
            //free_pages(priv->pages[i], 0);
            priv->pages[i] = NULL;
        }
	}

	while (priv->tx_skb_wr != priv->tx_skb_rd) {
		if (!priv->gmac_tx_skb[priv->tx_skb_rd].skb) {
			priv->gmac_tx_skb[priv->tx_skb_rd].skb = NULL;
			priv->gmac_tx_skb[priv->tx_skb_rd].first = 0;
			priv->gmac_tx_skb[priv->tx_skb_rd].cnt = 0;
			priv->gmac_tx_skb[priv->tx_skb_rd].start = 0;
			priv->tx_skb_rd++;
			if (priv->tx_skb_rd == GMAC_TX_DESC_NUM) {
				priv->tx_skb_rd = 0;
			}
			continue;
		}

		skb = priv->gmac_tx_skb[priv->tx_skb_rd].skb;

		dev_kfree_skb_any(priv->gmac_tx_skb[priv->tx_skb_rd].skb);
		priv->gmac_tx_skb[priv->tx_skb_rd].skb = NULL;
		priv->gmac_tx_skb[priv->tx_skb_rd].first = 0;
		priv->gmac_tx_skb[priv->tx_skb_rd].cnt = 0;
		priv->gmac_tx_skb[priv->tx_skb_rd].start = 0;

		priv->tx_skb_rd++;
		if (priv->tx_skb_rd == GMAC_TX_DESC_NUM) {
			priv->tx_skb_rd = 0;
		}
        ali_info("tx_skb_rd %d tx_skb_wr %d\n", priv->tx_skb_rd, priv->tx_skb_wr);
    }

    for (i = 0; i < GMAC_TX_DESC_NUM; i++) {
        desc = &priv->tx_desc[i];
        if(desc->DataDescriptor.seg_len> 0) {
            dma_unmap_single((struct device *)NULL, __DTCADDRALI(desc->DataDescriptor.pkt_buf_dma), \
                            desc->DataDescriptor.seg_len, DMA_TO_DEVICE);
            desc->DataDescriptor.seg_len = 0;
        }
        mac_tx_sts(priv, desc);
    }
	priv->avail_desc_num = GMAC_TX_DESC_NUM - 1;
    ali_info("priv->tx_skb_wr %d priv->tx_skb_rd %d priv->avail_desc_num %d\n", \
              priv->tx_skb_wr, priv->tx_skb_rd, priv->avail_desc_num);
    return;
}

void mac_free_rings(pgmac_private pgmac) {
	mac_desc_clean(pgmac);

	if(pgmac->rx_desc != NULL) {
		dma_free_coherent((struct device *)NULL, (GMAC_DESC_SZ * GMAC_RX_DESC_NUM), 
			pgmac->rx_desc, __DTCADDRALI(pgmac->rx_desc_dma));
		pgmac->rx_desc = NULL;
	}
	
	if(pgmac->tx_desc != NULL) {
		dma_free_coherent((struct device *)NULL, (GMAC_DESC_SZ * GMAC_TX_DESC_NUM), 
			pgmac->tx_desc, __DTCADDRALI(pgmac->tx_desc_dma));
		pgmac->tx_desc = NULL;
	}	
	
	if(pgmac->setup_buf != NULL) {
		dma_free_coherent((struct device *)NULL, SETUP_FRAME_SZ, 
			pgmac->setup_buf, __DTCADDRALI(pgmac->setup_buf_dma));
		pgmac->setup_buf = NULL;
	}
    return;
}

#ifndef USE_GPHY
void link_light_blink(pgmac_private pgmac) {
    if(!pgmac->transmit_okay) {
        return;
    }
    if(pgmac->blink_light) {
        pgmac->blink_light = 0;
        pgmac->in_blink = 1;
        ali_gpio_set_value(g_enet_link_gpio, 1);
        return;
    } 
    if (pgmac->in_blink) {
        pgmac->in_blink = 0;
        ali_gpio_set_value(g_enet_link_gpio, 0);
    } 
    return;
}
#endif
void mac_disconnect(pgmac_private pgmac) {

	GMAC_W16(IMR, (ISRTimer|ISRLinkStatus));
	
	netif_stop_queue(pgmac->dev);
	netif_carrier_off(pgmac->dev);	

    pgmac->link_partner = 0;
    pgmac->phy_reset = false;
    pgmac->link_established = false;
    pgmac->transmit_okay = false;
#ifndef USE_GPHY
    pgmac->blink_light = 0;
    pgmac->in_blink = 0;
    //ali_info("disable gpio light\n");
    ali_gpio_set_value(g_enet_link_gpio, 1);
    ali_gpio_set_value(g_enet_speed_gpio, 1);
#endif
    return;
}

void set_gmac_reg(void) {
    GMAC_W32(RGMII_PadDriven, 0xFFFFFFFF);
	// 1. Increase IPG time to ensure the gap between 2 packets > mini IPG time
	// 2. decrease the first portion of the interframe gap time
	GMAC_W8(TxRxCR1, 0x30);
	GMAC_W8(TxRxCR1+1, 0x1);
	if (gmac_phy_mode == ETH_PHY_MODE_RGMII) {
        GMAC_W32(RGMII_Rx_DelayT, 0x7000000);
        GMAC_W32(RGMII_Tx_DelayT, 0x2000000);
        GMAC_W32(RGMII_Rx_DelayT, 0x87000000);
    }
}
void mac_hw_start(pgmac_private pgmac) {
	u8 tmp_8;
	UINT32 tmp_u32;
	UINT32 duplex_mode = 0;
	UINT32 pause_frame = 0;
	int re = 0;
	UINT32 tmp_u32_2;
    UINT32 rmii_speed = 0;
    UINT32 rgmii_speed = 0;
	
	if (netif_msg_ifup(pgmac)) {
		ali_info("GMAC hardware start...\n");
    }

	
	//set mac address.
	GMAC_W32(PAR, gmac_mac_lo32);
	GMAC_W32(PAR - 4, gmac_mac_hi16);

	tmp_u32_2 = GMAC_R32(PAR);
	ali_info("mac reg 0x08 (gmac_mac_lo32) = 0x%x\n", tmp_u32_2);
	tmp_u32_2 = GMAC_R32(PAR-4);
	ali_info("mac reg 0x04 (gmac_mac_hi16) = 0x%x\n", tmp_u32_2);
	
	if(gmac_phy_mode == ETH_PHY_MODE_RMII) {
		if (pgmac->link_speed == LINK_SPEED_100M) {
			rmii_speed = (UINT32)(RmiiCrSpeedModeBitLow); //100Mbps
		} else {
			rmii_speed = (UINT32)0; //10Mbps
		}
		
		//Set RMII.
		tmp_u32 = GMAC_R32(RmiiCR);
		tmp_u32 &= ~(RmiiEn |RgmiiEn);
		tmp_u32 &= ~(RmiiCrSpeedModeBitLow |RmiiCrSpeedModeBitHigh);
		GMAC_W32(RmiiCR, (tmp_u32 |rmii_speed|RmiiEn));
	} else if (gmac_phy_mode == ETH_PHY_MODE_RGMII) {
		if (pgmac->link_speed == LINK_SPEED_1000M) {
			rgmii_speed = (UINT32)(RmiiCrSpeedModeBitHigh);	//1000Mbps
		} else if (pgmac->link_speed == LINK_SPEED_100M) {
			rgmii_speed = (UINT32)(RmiiCrSpeedModeBitLow); //100Mbps
		} else {
			rgmii_speed = 0; //10mbps
		}
		
		//Set RMII.
		tmp_u32 = GMAC_R32(RmiiCR);
		tmp_u32 &= ~(RmiiEn |RgmiiEn);		
		tmp_u32 &= ~(RmiiCrSpeedModeBitLow |RmiiCrSpeedModeBitHigh);

#if 1//for 100m bitfile only...100M_BITFILE
		tmp_u32 |= (rgmii_speed|RgmiiEn);
		tmp_u32 &= ~(3<<12);
		tmp_u32 |= (2<<12);
		GMAC_W32(RmiiCR, (tmp_u32 ));
#else
		GMAC_W32(RmiiCR, (tmp_u32 |rgmii_speed|RgmiiEn));
#endif
	}

	if (pgmac->duplex_mode) {
		duplex_mode = (UINT32)FullDuplexMode;
    } else {
		duplex_mode = (UINT32)0;
    }

	if(gmac_rx_csum == true) {
		duplex_mode |= RxTOEWorkMode;
    }

	//config network operation mode.
	GMAC_W32(NetworkOM, (duplex_mode|NetworkOMConfig));

	if (pgmac->pause_frame_rx) {
		pause_frame |= (UINT32)RxFlowControlEn;
    }
	if (pgmac->pause_frame_tx) {
		pause_frame |= (UINT32)TxFlowControlEn;
    }

	// 1. Increase IPG time to ensure the gap between 2 packets > mini IPG time
	// 2. decrease the first portion of the interframe gap time
//	GMAC_W8(TxRxCR1, 0xdf);
//	GMAC_W8(TxRxCR1+1, 0x00);

	tmp_u32 = GMAC_R32(TxRxCR2);
    tmp_u32 &= 0xf7ffffff;
    tmp_u32 |= 0x04000000;
	tmp_u32 &= ~TxFifoThMask;
	tmp_u32 |= (0x2<<TxFifoThOff);
	tmp_u32 &= ~RxFlowControlEn;
	tmp_u32 &= ~TxFlowControlEn;
#if(GMAC_VLAN_TAG_USED)
	if(pgmac->vlan_tag_remove)
		GMAC_W32(TxRxCR2, (pause_frame |tmp_u32|TxRxConfig2|RxRemoveVlanTagEn|VlanEn));
	else
		GMAC_W32(TxRxCR2, (pause_frame |tmp_u32|TxRxConfig2|VlanEn));
#else
	GMAC_W32(TxRxCR2, (pause_frame|tmp_u32 |TxRxConfig2));
#endif //GMAC_VLAN_TAG_USED

	GMAC_W32(TSAD, pgmac->tx_desc_dma);
	GMAC_W32(RSAD, pgmac->rx_desc_dma);

	GMAC_W16(RxDesTotNum, GMAC_RX_DESC_NUM);
	GMAC_W16(TxDesTotNum, GMAC_TX_DESC_NUM);

    printk("Rx Desc num %d   Tx Desc num %d\n", GMAC_RX_DESC_NUM, GMAC_TX_DESC_NUM);
	GMAC_W16(RxRingDesWPtr, GMAC_RX_DESC_NUM -1);
	GMAC_W16(TxRingDesWPtr, 0);

	GMAC_W32(TimerR, gmac_timer_frq);
	
	//gmac_set_rx_mode(pgmac->dev);
#ifdef TEST_JUMBO_FRAME
    re = mac_rx_jumbo_refill(pgmac);
#else 
	re = mac_rx_refill(pgmac);
#endif
	if(re) {
        //asm("sdbbp");
	}

	tmp_8 = (SCRRxEn|SCRTxEn);
	if(gmac_tso) {
		tmp_8 |= (SCRTxCoeEn|SCRTsoEn);
    }
	if(gmac_ufo) {
		tmp_8 |= (SCRTxCoeEn|SCRUfoEn);
    }
	if(gmac_tx_csum) {
		tmp_8 |= SCRTxCoeEn;
    }
	if(gmac_rx_csum) {
		tmp_8 |= SCRRxCoeEn;
    }
	GMAC_W8(SCR, tmp_8);

	/* Enable all surported interrupts. */
	GMAC_W32(IMR, pgmac->isr_mask);
	ali_info("<----%s mac_hw_start is done!\n", __FUNCTION__);
    return;
}

void gmac_link_changed(pgmac_private pgmac) {
    if (pgmac->link_established == false) {
       ali_info("%s link_established shouln't be false!\n", __FUNCTION__); 
        return;
    }
    phy_link_chk(pgmac);
#ifndef USE_GPHY
    if(pgmac->link_speed == LINK_SPEED_100M) {
        //ali_info("100M, light speed led\n");
        ali_gpio_set_value(g_enet_speed_gpio, 0);
    } else {
        //ali_info("non 100M, disable speed led\n");
        ali_gpio_set_value(g_enet_speed_gpio, 1);
    }
#endif
    mac_hw_start(pgmac);
    netif_carrier_on(pgmac->dev);
    netif_start_queue(pgmac->dev);
    pgmac->transmit_okay = true;
}

void mac_init_for_link_established(pgmac_private pgmac) {
    mac_desc_clean(pgmac);
    mac_chip_rst();
	pgmac->cur_isr = 0;
	pgmac->isr_mask = GMAC_INTERRUPT_MASK; 
	
	pgmac->rx_wptr = GMAC_RX_DESC_NUM -1;
	pgmac->rx_bptr= 0;
	pgmac->tx_wptr= 0;	
	
	pgmac->phy_reset = false;

	pgmac->pause_frame_rx = false;
	pgmac->pause_frame_tx = false;

#if GMAC_VLAN_TAG_USED
	pgmac->vlan_tag_remove = false;
#endif //GMAC_VLAN_TAG_USED
}
void gmac_link_established(pgmac_private pgmac) {
    GMAC_W16(IMR, 0);
    //GMAC_W8(SCR, 0);
    mac_init_for_link_established(pgmac);
    phy_set(pgmac);
    pgmac->phy_reset = true;
    GMAC_W16(IMR, (ISRTimer|ISRLinkStatus));
    gmac_link_changed(pgmac);
#ifndef USE_GPHY
    //ali_info("light link gpio\n");    
    ali_gpio_set_value(g_enet_link_gpio, 0);
    pgmac->unlink_error_state = 0;
#endif
}

void mac_set(pgmac_private pgmac) {
	if(pgmac->phy_reset == false) {
		
		GMAC_W16(IMR, 0);
		//GMAC_W8(SCR, 0);
		
		mac_desc_clean(pgmac);
		mac_chip_rst();
		mac_cnt_init(pgmac);
		
		phy_set(pgmac);
		
		pgmac->phy_reset = true;
		
		GMAC_W16(IMR, (ISRTimer|ISRLinkStatus));
	} else {
		GMAC_W16(IMR, 0);
		if (pgmac->link_established == false) {
			GMAC_W16(IMR, (ISRTimer|ISRLinkStatus));
		} else {
            phy_link_chk(pgmac);
			
			mac_hw_start(pgmac);
			
			netif_carrier_on(pgmac->dev);
			netif_start_queue(pgmac->dev);
			pgmac->transmit_okay = true;
		}
	}
    return;
}

void handle_unlink_error(pgmac_private pgmac) {
    u16 ctrl = 0; 
    u16 status = 0;
    u16 reg10 = 0;
    u16 tmp = 0;
    
    switch(pgmac->unlink_error_state) {
        case 0: /* state A */ 
            reg10 = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, 0x10);
            ctrl = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, PhyBasicModeCtrl);
            tmp = (1 << 7);
            /* check whether there is signal on line */
            if (!(reg10 & tmp)) { 
                /* no signal on line */
                ali_trace(11, "no signal on line\n");
                return;
            }
            /* check support auto nego and unlink */
            tmp = reg10;
            tmp = (tmp >> 12);
            if (!(ctrl & BMCRANEnable) || (tmp == 6)) {
                /* doesn't support auto nego or link */
                ali_trace(11, "non auto nego or linked\n");
                return;
            }
            pgmac->unlink_error_state = 1; /* change state to parallel detection */
            pgmac->para_detect_times = 0;  
            /* through */
            ali_trace(11, "From A to Para_Detect\n");
        case 1:
            /*only when para_detect_times == 0 do parallel detect 41ms * 12 = 500ms*/
            if(pgmac->para_detect_times >= 12) {
                pgmac->unlink_error_state = 0;
                ali_trace(11, "Para_Detect over 500ms, change to A\n");
                return;
            }
            reg10 = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, 0x10);
            /* check link partner support auto nego */
            tmp = reg10 & 0x0F00;
            tmp = (tmp >> 8);
            if(tmp == 3 || tmp == 5) {
                pgmac->unlink_error_state = 0; /* change state to A */ 
                ali_trace(11, "link partner support auto nego, change to A\n");
                return;
            }
            pgmac->para_detect_times++;
            if(tmp != 4) {
                ali_trace(11, "do another Para Detect again\n");
                return;
            }
            pgmac->unlink_error_state = 2; /* change state to unlink and signal state */ 
            pgmac->unlink_with_signal_times = 0;
            ali_trace(11, "Para Detect ok, change to unlink with no signale state\n");
            /* through */
        case 2:
            if (pgmac->unlink_with_signal_times >= 48) {
                /* > 2s */
                do_dll_pd_reset(pgmac); 
                pgmac->unlink_error_state = 0;
                return;
            }
            /* check whether there is signal on line */
            reg10 = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, 0x10);
            status = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, PhyBasicModeStatus);
            tmp = (1 << 7);
            if (!(reg10 & tmp) || (status & BMSRLinkStatus)) { 
                /* no signal on line */
                ali_trace(11, "no signal on line, change to A\n");
                pgmac->unlink_error_state = 0;
                return;
            }
            pgmac->unlink_with_signal_times++; 
            break;
        default:
            ali_info("something wrong, pgmac->unlink_error_state is %d\n", pgmac->unlink_error_state);
            pgmac->unlink_error_state = 0;
            break;
    } 
    return;
}

void mac_weird_int(pgmac_private pgmac) {
	u16 tmp_u16;
    u16 ctrl, status;
    int link_up = 0;
    u16 tmp1 = 0;
	if (pgmac->link_established == false) {
        ali_trace(1, "%s: link established false\n",__FUNCTION__);
        ctrl = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, PhyBasicModeCtrl);
        status = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, PhyBasicModeStatus);
        if (status & BMSRLinkStatus) {
            if((ctrl & BMCRANEnable) && (status & BMSRANComplete)) {
                ali_info("%s: auto-negotiation completed.", __FUNCTION__);
                link_up = 1;
            } else if (!(ctrl & BMCRANEnable)) {
                ali_info("%s: non auto-negotiation link established.", __FUNCTION__);
                link_up = 1;
            }
        if (link_up) {
            pgmac->link_established = true;
            //mac_set(pgmac);
            gmac_link_established(pgmac);
            mac_mdio_write(pgmac->dev, gmac_phy_addr, 0x1d, 7);
            tmp1 = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, 0x1f);
            ali_info("%s: *******PHY 1f value 0x%x.", __FUNCTION__, tmp1);
            ali_info("%s: link established.", __FUNCTION__);
            pgmac->times_of_link_change++;
            }
        } else {
#ifndef USE_GPHY
            handle_unlink_error(pgmac);
#endif
        }
		return;
	}
    /* last state is link established */
    ali_trace(1, "%s: link established true\n",__FUNCTION__);
	if (pgmac->cur_isr & (ISRLinkStatus|ISRTimer)) {
        status = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, PhyBasicModeStatus);
        ali_trace(1, "%s: cur_isr is ISRLinkStatus|ISRtimer\n", __FUNCTION__);
        if (status & BMSRLinkStatus) {
            tmp_u16 = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, PhyNWayLPAR);
            ali_trace(1, "%s: PhyNWayLPAR 0x%x link_partner 0x%x\n", __FUNCTION__, tmp_u16, pgmac->link_partner);
            if (tmp_u16 != pgmac->link_partner) {
                if (tmp_u16) {
                    ali_info("%s tmp_u16 %x link_partner %x link connected.",\
                            __FUNCTION__, tmp_u16, pgmac->link_partner);
                    //mac_set(pgmac);
                    gmac_link_changed(pgmac);
                }
            }
        } else {
            mac_disconnect(pgmac);
            pgmac->times_of_link_change++;
            ali_info("%s: link disconnected.", __FUNCTION__);
        }
	}
    return;
}

int mac_build_setup(struct net_device *dev) {
	pgmac_private pgmac = netdev_priv(dev);
	UINT8 *pa, *buf_base, *addr;
	UINT32 crc;
	UINT16  bit, byte, hashcode;
	int i, j, rv = 0;
	struct netdev_hw_addr_list * mc;
	struct netdev_hw_addr * ha;

	memset(pgmac->setup_buf, 0x00, SETUP_FRAME_SZ);

	mc = &dev->mc;
	if(gmac_rx_filter == GMAC_RX_FILTER_HASH) {
		pa = pgmac->setup_buf + IMPERFECT_PA_OFF;
		//fill our own mac addr in setup frame buffer(offset from 156 to 167) 
		for (i = 0; i < ETH_ALEN; i ++) {
			pa[i&0x01] = dev->dev_addr[i]; //host mac addr.
			if(i&0x01) 
				pa += 4;
		}
		//offset is 168 now.
	
		pa[(GMAC_HASH_TABLE_LEN >> 3) - 3] = 0x80; //bc.FM-20091123.

		buf_base = pgmac->setup_buf;
		i = 0;
		list_for_each_entry(ha, &mc->list, list) {
 			addr = &(ha->addr[0]);
			crc = ether_crc(ETH_ALEN, addr);
			
			hashcode = (UINT16)crc & GMAC_HASH_BITS;
		
			byte = hashcode >> 3;				//bit[3-8] -> byte offset
			bit = 1 << (hashcode & 0x07);		//bit[0-2] -> bit offset
		
			byte <<= 1;						//maybe hard to get it!^_^
			if (byte & 0x02) 
				byte -= 1;
			
			buf_base[byte] |= bit;
		}	
	} else if(gmac_rx_filter == GMAC_RX_FILTER_PERFECT) {
		pa = pgmac->setup_buf + 0;
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
		
		list_for_each_entry(ha, &mc->list, list) {
			for (j = 0; j < ETH_ALEN; j ++) 
			{ 
				pa[j&0x01] = ha->addr[j];				
				if (j&0x01) 
					pa += 4;
			}
		}
	}
	else
	{
		GMAC_WARNING("%s()=>gmac_rx_filter(%d) not supported yet.", __FUNCTION__, gmac_rx_filter);
		rv = -1;
	}
	return rv;
}

int mac_set_mc_filter(struct net_device *dev) {
	u8 cmd_u8;
	UINT32 cmd_u32;

	cmd_u8 = GMAC_R8(SCR);
	GMAC_W8(SCR, cmd_u8 & ~(SCRRxEn|SCRTxEn));

	cmd_u32 = GMAC_R32(NetworkOM);
	cmd_u32 &= ~PassMask;
	GMAC_W32(NetworkOM, cmd_u32);
	return mac_build_setup(dev);
}

#ifdef TEST_JUMBO_FRAME
u16 mac_rx_jumbo_update_wptr(pgmac_private pgmac) {
	struct net_device *dev = pgmac->dev;
	u16 rx_bptr, rx_rptr, rx_wptr;
	u16 updata = 0;
	volatile prx_desc rx_desc;
	struct sk_buff *new_skb;
	int i, j;
    struct page * page;
    int bufsz = GMAC_JUMBO_SKB_LINEAR_SIZE;

	rx_wptr = pgmac->rx_wptr;
	rx_bptr = pgmac->rx_bptr;
	rx_rptr = GMAC_R16(RxRingDesRPtr);

	//ali_info("%s begin rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.",__FUNCTION__, rx_wptr, rx_bptr, rx_rptr);

	if(rx_wptr > rx_rptr) {
		if((rx_bptr > rx_rptr) && (rx_bptr <= rx_wptr)) {
			goto rx_lost;
        } else {
			if(rx_bptr > rx_wptr) {
				updata = rx_bptr - rx_wptr -1;
            } else {
				updata = GMAC_RX_DESC_NUM + rx_bptr - rx_wptr -1;
            }
		}
	} else if(rx_wptr < rx_rptr) {
		if((rx_bptr > rx_rptr) ||(rx_bptr <= rx_wptr)) {
			goto rx_lost;	
        } else {
			updata = rx_bptr - rx_wptr -1;
        }
	} else {
		if(rx_bptr > rx_wptr) {
			updata = rx_bptr - rx_wptr -1;
        } else if(rx_bptr < rx_wptr) {
			updata = GMAC_RX_DESC_NUM + rx_bptr - rx_wptr -1;
        } else {
			goto rx_lost;	
        }
	}
   
	if(updata > 0) {
		i = rx_wptr;
		while(updata > 0) {
            if (pgmac->rx_skb[i] == NULL) {
                //new_skb = netdev_alloc_skb_ip_align(dev, bufsz);
                new_skb = tmp_alloc_skb_ip_align(dev, bufsz);
                if (unlikely(!new_skb)) {
                    dev->stats.rx_dropped++;
                    break;
                }
                pgmac->rx_skb[i] = new_skb;
            }
            page = pgmac->pages[i];
            if (page) {
                ali_error("bug on %s page should be NULL,but idx %d  page %x", __FUNCTION__, i, (u32)page);
                break;
            }
            page = alloc_page(GFP_ATOMIC);
            if (unlikely(!page)) {
                ali_error("%s failed alloc_page\n", __FUNCTION__);
                break;
            }
            pgmac->pages[i] = page;
			rx_desc = &pgmac->rx_desc[i];
#if 0
            if (rx_desc->pkt_buf_dma) {
                printk("%s bug on rx_desc->pkt_buf_dma %x isn't NULL\n", \
                        __FUNCTION__, rx_desc->pkt_buf_dma);
            }
#endif
            memset(rx_desc, 0, sizeof(struct gmac_rx_desc));
            rx_desc->pkt_buf_dma = dma_map_page((struct device *)NULL, page,\
                        0, GMAC_JUMBO_FRAG_SZ, DMA_FROM_DEVICE); 
            rx_desc->pkt_buf_dma = __CTDADDRALI(rx_desc->pkt_buf_dma);
            //ali_info("%s i %d pgmac->rx_skb[%d] %x, pgmac->pages[%d] %x\n", __FUNCTION__,i,i, pgmac->rx_skb[i], i, pgmac->pages[i]);
            //ali_info("%s i %d rx_desc->pkt_buf_dma %x\n", __FUNCTION__,i,rx_desc->pkt_buf_dma); 
            i = (i + 1) % GMAC_RX_DESC_NUM;
			updata --;
		}
		pgmac->rx_wptr = i;
		GMAC_W16(RxRingDesWPtr, pgmac->rx_wptr);
	}
	//ali_info("%s end rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.",__FUNCTION__, pgmac->rx_wptr, rx_bptr, rx_rptr);
	return rx_rptr;
rx_lost:
	GMAC_WARNING("%s()=>rx_bptr got lost.", __FUNCTION__);
	GMAC_WARNING("rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.\n", rx_wptr, rx_bptr, rx_rptr);
    for (j=0; j<64; j++) {
		rx_desc = &pgmac->rx_desc[j];
        ali_error("RX_DESC %d:\n", j);
        show_rx_desc(rx_desc, j);
    }
	//asm("sdbbp");
	return rx_rptr;
}
#endif

u16 mac_rx_update_wptr(pgmac_private pgmac) {
	struct net_device *dev = pgmac->dev;
	u16 rx_bptr, rx_rptr, rx_wptr;
	u16 updata = 0;
	prx_desc rx_desc;
	struct sk_buff *new_skb;
	int i;

	rx_wptr = pgmac->rx_wptr;
	rx_bptr = pgmac->rx_bptr;
	rx_rptr = GMAC_R16(RxRingDesRPtr);

	ali_trace(2, "--> %s: rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.", __FUNCTION__, rx_wptr, rx_bptr, rx_rptr);

	if(rx_wptr > rx_rptr)
	{
		if((rx_bptr > rx_rptr) && (rx_bptr <= rx_wptr))
			goto rx_lost;
		else
		{
			if(rx_bptr > rx_wptr)
				updata = rx_bptr - rx_wptr -1;
			else
				updata = GMAC_RX_DESC_NUM + rx_bptr - rx_wptr -1;
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
			updata = GMAC_RX_DESC_NUM + rx_bptr - rx_wptr -1;
		else
			goto rx_lost;	
	}

	if(updata > 0)
	{
		i = rx_wptr;
		while(updata > 0)
		{
			new_skb = netdev_alloc_skb(dev, GMAC_BUF_SZ);
			if (!new_skb) {
				dev->stats.rx_dropped++;
				break;
			}
			//skb_reserve(new_skb, NET_IP_ALIGN);
            if(test_bit(9, &dbg_runtime_val)) {
                ali_info("memset new_skb data size %d\n", new_skb->end - new_skb->data);
                gmac_print(new_skb->data, new_skb->end - new_skb->data);
            }

			pgmac->rx_skb[i] = new_skb;
            
			rx_desc = &pgmac->rx_desc[i];
			rx_desc->pkt_buf_dma = dma_map_single((struct device *)NULL, new_skb->data, 
				GMAC_BUF_SZ, DMA_FROM_DEVICE);
			rx_desc->pkt_buf_dma = __CTDADDRALI(rx_desc->pkt_buf_dma);

            if(!verify_buf(pgmac, new_skb->data, rx_desc->pkt_buf_dma, GMAC_BUF_SZ)) {
                printk("WARNING..............................\n");
            }
            ali_trace(2, "add skb %x to no %d new_skb->data %x rx_desc pkt_buf_dma %x\n", (u32)new_skb, i, \
                (u32)new_skb->data, (u32)rx_desc->pkt_buf_dma); 
			if(i == GMAC_RX_DESC_NUM - 1){
//				rx_desc->EOR = 1;
				i = 0;
			}
			else{
//				rx_desc->EOR = 0;
				i++;
			}		
			updata --;
		}
		
		pgmac->rx_wptr = i;
		GMAC_W16(RxRingDesWPtr, pgmac->rx_wptr);

	}

	ali_trace(2, "<-- %s: rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.", __FUNCTION__, GMAC_R16(RxRingDesWPtr), rx_bptr, rx_rptr);
	return rx_rptr;

rx_lost:
	ali_warn("%s()=>rx_bptr got lost.", __FUNCTION__);
	ali_warn("rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.\n", rx_wptr, rx_bptr, rx_rptr);
	//asm("sdbbp");
	return rx_rptr;
}


bool mac_rx_hdr_chk(struct net_device *dev, prx_desc rx_desc) {
	pgmac_private pgmac = netdev_priv(dev);
	int fatal_err = 0;
    u32 tmp_seg_len = 0;
	ppacket_head pHead;

	pHead = &(rx_desc->pkt_hdr);

#ifndef TEST_JUMBO_FRAME
    if(rx_desc->PacketLength < 60 || rx_desc->PacketLength > 1522) {
        ali_info("rx_desc->PacketLength %d\n", rx_desc->PacketLength);
        return false;  
    } 
#endif

    tmp_seg_len = (rx_desc->PacketLength + 3) / 4;
    //ali_info("tmp_seg_len %d pHead->SegLength %d\n", tmp_seg_len, pHead->SegLength+1);
    if(tmp_seg_len != (pHead->SegLength + 1)) {
        return false;
    } 

	if(pHead->ES) {
		if(pHead->WatchdogTimeout || pHead->LateCollision || 
           pHead->Long || pHead->Runt || pHead->CRC || pHead->Dribble) {
            return false;
		} else {
            return true;
        }
	} else {
		if(pHead->WatchdogTimeout || pHead->LateCollision || 
           pHead->Long || pHead->Runt || pHead->CRC) {
            return false;
		}

		dev->stats.rx_packets++;
		dev->stats.rx_bytes += rx_desc->PacketLength;
	
		if (pHead->BF)
			pgmac->mac_stats.rx_bc++;
		if (pHead->PF)
			pgmac->mac_stats.rx_uc++;
		if (pHead->MF)
			pgmac->mac_stats.rx_mc++;
		if (pHead->PPPoE)
			pgmac->mac_stats.rx_pppoe++;
		if (pHead->VLAN)
			pgmac->mac_stats.rx_vlan++;
		if (pHead->IPFrame)
			pgmac->mac_stats.rx_ip++;
		if (pHead->IP6Frame)
			pgmac->mac_stats.rx_ipv6++;
		if (pHead->IPFrag)
			pgmac->mac_stats.rx_frag++;
		return true;
	}	
}

//analyze & recode rx status while head is okay.
bool mac_rx_chs_ok(pgmac_private pgmac, ppacket_head pHead, struct sk_buff *skb) {
#if 1
	if((gmac_rx_csum == true) &&( (pHead->IPFrame)||(pHead->IP6Frame))) {
		if(pHead->IPFrag)
			goto Done;

		if(!pHead->IP6Frame && !pHead->IPChksum) {
            pgmac->mac_stats.rx_ip_chksum_errors++;
//            ali_warn("%s ip_chksum failed!!!", __FUNCTION__);
/*            gmac_print(skb->data, skb->len + RING_CRC_SZ); */
            goto Done;
        }
        /* doesn't mean TCP only, including UDP */
		if(!pHead->TCPChksum) {
			pgmac->mac_stats.rx_tcp_chksum_errors++;
//            ali_warn("%s rx_tcp_chksum failed!!!", __FUNCTION__);
//            gmac_print(skb->data, skb->len + RING_CRC_SZ);
			goto Done;
		}
		return true;
	}
Done:
	return false;
#endif
    return true;
}

#ifdef TEST_JUMBO_FRAME
static void gmac_consume_page (struct sk_buff *skb, u16 length) {
    skb->len += length;
    skb->data_len += length;
    skb->truesize += PAGE_SIZE;
   // ali_info("consume_page skb %x len %d data_len %d truesize %d\n", skb, skb->len, skb->data_len, skb->truesize);
}

#define rxtop pgmac->rx_top_skb
void show_jumbo_pkts(struct sk_buff * skb) {
    skb_frag_t *frag = NULL;
    if(!skb) {
        return;
    }
    if (skb_shinfo(skb)->nr_frags) {
        return;
    }
    ali_info("skb %x len %d data_len %d data %x tail %x\n", \
             (u32)skb, skb->len, skb->data_len, (u32)skb->data, (u32)skb->tail);
    ali_info("linear section:\n");
    gmac_print(skb->data, (skb->len - skb->data_len));
    ali_info("pages section:\n");
    if (skb->data_len > 0) {
        frag = &skb_shinfo(skb)->frags[0];
        gmac_print(page_address(skb_frag_page(frag)), 40);
    } else {
        ali_info("no pages\n");
    }
    return;
}


int mac_rx_jumbo_pkts(pgmac_private pgmac, int budget) {
	volatile prx_desc rx_desc;
	u16 rx_bptr, rx_rptr;
	ppacket_head pHead;
	u16 pkts, rx, i;
    u16 next_i;
	struct sk_buff *skb;
    int length = 0;
    struct page * rx_page;
    struct gmac_rx_desc tmp_rx_desc;
    int tmp_ret = 0;
	
	rx_rptr = mac_rx_jumbo_update_wptr(pgmac);
	rx_bptr = pgmac->rx_bptr;
	
	if(rx_rptr >= rx_bptr) {
		pkts = rx_rptr - rx_bptr;
    } else {
		pkts = GMAC_RX_DESC_NUM + rx_rptr - rx_bptr;
    }
	
	if((pkts > 0) && (rx_rptr == pgmac->rx_wptr)) {
		pkts -= 1;
    }
	
	if(gmac_rx_mode == GMAC_RX_MODE_NAPI) {
		if((budget != 0)&&(pkts > budget)) {
			pkts = budget;
        }
	}
	
	i = rx_bptr;
    next_i = rx_bptr;
	rx = 0;
    //ali_info("pkts %d\n", pkts);
	while(pkts > 0) {
        i = next_i;
		if(next_i == GMAC_RX_DESC_NUM - 1)
			next_i = 0;
		else
			next_i++;
		pkts--;
		rx++;

        rx_page = pgmac->pages[i];
        pgmac->pages[i] = NULL;
        skb = pgmac->rx_skb[i];
        pgmac->rx_skb[i] = NULL;
retry:
        memcpy(&tmp_rx_desc, &pgmac->rx_desc[i], sizeof(struct gmac_rx_desc));  
		rx_desc = &pgmac->rx_desc[i];
        rmb();
        
/* need unmap page ?? */
        dma_unmap_page((struct device *)NULL, __DTCADDRALI(rx_desc->pkt_buf_dma), GMAC_JUMBO_FRAG_SZ, DMA_FROM_DEVICE);
        rx_desc->pkt_buf_dma = NULL;
        printk("%s set idx %d page NULL\n", __FUNCTION__, i);

        //ali_info ("i %d rxtop %x, rx_page %x\n", i, rxtop, rx_page);
		pHead = &(rx_desc->pkt_hdr);
        if (pHead->FirstSegment && pHead->LastSegment) {
            length = rx_desc->PacketLength - RING_CRC_SZ;
        } else {
            length = (pHead->SegLength + 1) * 4; 
        }

        //ali_info("rx_desc NO %d\n", i);
        //show_rx_desc(rx_desc);

        if (pHead->FirstSegment) { /*first seg */
            //ali_info("first seg len %d\n", length);
            if (rxtop) {
                ali_error("rx_top_skb should be NULL");
                ali_error("someting wrong, rx_top_skb %x", (u32)rxtop);
            }
            rxtop = skb;
            //ali_info ("i %d rxtop %x, rx_page %x, length %d\n", i, (u32)rxtop, (u32)rx_page, length);
            skb_fill_page_desc(rxtop, 0, rx_page, 0, length);
			if(mac_rx_chs_ok(pgmac, pHead, rxtop)) {
				skb->ip_summed = CHECKSUM_UNNECESSARY;
			} else {
				skb->ip_summed = CHECKSUM_NONE;
			}
			
            if (pHead->LastSegment) {
                rxtop = NULL;
                ali_trace(2, "set rxtop to NULL, skb  %x\n", (u32)rxtop);
                gmac_consume_page(skb, length); 
                goto rx_pkt;
            } else {
                gmac_consume_page(rxtop, length); 
                continue;
            }
        } 

        /* middle seg and last seg*/
        if (!rxtop) {
            show_rx_desc(rx_desc, j);
            ali_info("rx_page %x\n",(u32)rx_page);
            gmac_print(page_address(rx_page), 64);
            ali_error("middle pkt, rx_top_skb shouldn't be NULL\n");
            //asm("sdbbp");
            goto retry;
        }
       
        /* to set actually length */
        if (pHead->LastSegment) {
            length = rx_desc->PacketLength - RING_CRC_SZ - rxtop->len;
            ali_trace (2, "last Packetlength %d, rxtop->len %d seg length %d\n",\
                       rx_desc->PacketLength, rxtop->len, length);
        }

        skb_fill_page_desc(rxtop, skb_shinfo(rxtop)->nr_frags,
                            rx_page, 0, length);
        /* reuse for future */
        pgmac->rx_skb[i] = skb;

        if (pHead->LastSegment) {
            skb = rxtop;
            gmac_consume_page(skb, length); 
            rxtop = NULL;
            ali_trace(2, "set rxtop to NULL , before rxtop is %x\n", (u32)skb);
        } else {
            gmac_consume_page(rxtop, length); 
            continue;
        }
rx_pkt:
        //show_jumbo_pkts(skb);
		if (mac_rx_hdr_chk(pgmac->dev, rx_desc)) {
            if (!pskb_may_pull(skb, ETH_HLEN)) {
                ali_info("pskb_may_pull failed.\n");
                dev_kfree_skb_any(skb);
                continue;
            } 
            //show_jumbo_pkts(skb);
			skb->protocol = eth_type_trans(skb, pgmac->dev);
			skb->dev = pgmac->dev;
            ali_trace(2, "send skb %x protocol %x to skb->len %d kernel protocol\n", \
                        (u32)skb, skb->protocol, skb->len);	
			if(gmac_rx_mode == GMAC_RX_MODE_NAPI) {
#if GMAC_VLAN_TAG_USED
				if (pgmac->vlgrp && pHead->VLAN) 
					vlan_hwaccel_receive_skb(skb, pgmac->vlgrp, swab16(rx_desc->vlan_tag));
				else
#endif
					netif_receive_skb(skb);
			} else {
#if GMAC_VLAN_TAG_USED
				if (pgmac->vlgrp && pHead->VLAN) 
					vlan_hwaccel_rx(skb, pgmac->vlgrp, swab16(rx_desc->vlan_tag));
				else
#endif
					netif_rx(skb);
			}

		} else {
			ali_warn("WARNING mac_rx_pkts(head error): Head(%08x).", *(UINT32 *)pHead);
            dev_kfree_skb_any(skb);
		}
	} /* while (pkts > 0) */
	
	pgmac->rx_bptr = next_i; 
	return rx;
}
#endif

int mac_rx_pkts(pgmac_private pgmac, int budget)
{
    u32 flags = 0;
	prx_desc rx_desc;
	u16 rx_bptr, rx_rptr;
	ppacket_head	pHead;
	u16 pkt_sz, pkts, rx, i;
	struct sk_buff *skb;
    u32 crc_sw, crc_hw;
    int tmp_ret = 0;
	rx_rptr = mac_rx_update_wptr(pgmac);
	rx_bptr = pgmac->rx_bptr;
	
	if(rx_rptr >= rx_bptr)
		pkts = rx_rptr - rx_bptr;
	else
		pkts = GMAC_RX_DESC_NUM + rx_rptr - rx_bptr;
	
	if((pkts > 0)&&(rx_rptr == pgmac->rx_wptr))
		pkts -= 1;
	
	if(gmac_rx_mode == GMAC_RX_MODE_NAPI) {
		if((budget != 0)&&(pkts > budget)) 
			pkts = budget;
	}
	
	i = rx_bptr;
	rx = 0;
	while(pkts > 0) {
		rx_desc = &pgmac->rx_desc[i];
		pHead = &(rx_desc->pkt_hdr);
        dma_unmap_single((struct device *)NULL, __DTCADDRALI(rx_desc->pkt_buf_dma),
				GMAC_BUF_SZ, DMA_FROM_DEVICE);
        ali_trace(2, "rx_desc %x no %d\n", (u32)rx_desc, i);

        pkt_sz = rx_desc ->PacketLength - RING_CRC_SZ;
        skb = pgmac->rx_skb[i];
		tmp_ret = mac_rx_hdr_chk(pgmac->dev, rx_desc);
        if(!tmp_ret || test_bit(8, &dbg_runtime_val)) {
            clear_bit(8, &dbg_runtime_val);
            ali_warn("mac_rx_pkts(head error): Head(%08x).", *(UINT32 *)pHead);
            //GMAC_W32(0x58, (GMAC_R32(0x58) |0x80000000)); 
            dev_kfree_skb_any(skb);
            ali_warn("%s free_%d skb 0x%x\n",__FUNCTION__, i, skb);
            pgmac->rx_skb[i] = NULL;
            show_rx_desc(rx_desc, i);
            ALI_LOCK_IRQ(&pgmac->lock, flags);
            mac_init_for_link_established(pgmac);
            mac_hw_start(pgmac);
            ALI_UNLOCK_IRQ(&pgmac->lock, flags);
            goto handle_exception;
        }

#ifndef USE_GPHY
        if(!pgmac->blink_light && !pgmac->in_blink) {
            pgmac->blink_light = 1;
        }
#endif
        pkt_sz = rx_desc ->PacketLength - RING_CRC_SZ;
        skb = pgmac->rx_skb[i];
        if (pkt_sz > 1522) {
            show_rx_desc(rx_desc, i);
        }
        skb_put(skb, pkt_sz);
        if(test_bit(7, &dbg_runtime_val)) {
            printk("---------------------------------\n");
            gmac_print(skb->data, skb->len + RING_CRC_SZ);
            printk("--------------------------------\n");
        }

        if(crc_det_err == 1) {
            crc_sw = *(u32 *)(skb->data + skb->len);
            crc_sw = ~crc_sw;
            crc_hw = ether_crc_le(skb->len, skb->data);
            if(crc_sw != crc_hw) {
                ali_error("---------------------------------\n");
                show_rx_desc(rx_desc, i);
                ali_error("--------------------------------\n");
                gmac_print(skb->data, skb->len + RING_CRC_SZ);
                ali_error("CRC ERR:crc_sw = 0x%08x, crc_hw = 0x%08x\n", crc_sw, crc_hw);
                if(test_bit(5, &dbg_runtime_val)) {
                    GMAC_W32(0x58, (GMAC_R32(0x58) |0x80000000)); 
                }
                dbg_runtime_val = 0;
            }	
        }

        if(mac_rx_chs_ok(pgmac, pHead, skb)) {
            skb->ip_summed = CHECKSUM_UNNECESSARY;
        } else {
            skb->ip_summed = CHECKSUM_NONE;
        }
        //skb->ip_summed = CHECKSUM_NONE;

        skb->protocol = eth_type_trans(skb, pgmac->dev);
        skb->dev = pgmac->dev;

        if(gmac_rx_mode == GMAC_RX_MODE_NAPI) {
#if GMAC_VLAN_TAG_USED
            if (pgmac->vlgrp && pHead->VLAN) 
                vlan_hwaccel_receive_skb(skb, pgmac->vlgrp, swab16(rx_desc->vlan_tag));
            else
#endif
            {
                
                ali_trace(2, "rx_desc->pkt_buf_dma %x skb %x  skb->data %x no %d to kernel\n", (u32)rx_desc->pkt_buf_dma, (u32)skb, (u32)skb->data, i);
                ali_trace(2, "send skb %x  skb->data %x no %d to kernel\n", (u32)skb, (u32)skb->data, i);
                netif_receive_skb(skb);
            }
        } else {
#if GMAC_VLAN_TAG_USED
            if (pgmac->vlgrp && pHead->VLAN) 
                vlan_hwaccel_rx(skb, pgmac->vlgrp, swab16(rx_desc->vlan_tag));
            else
#endif
                netif_rx(skb);
        }
        pgmac->rx_skb[i] = NULL;
    
        if(i == GMAC_RX_DESC_NUM - 1) {
            i = 0;
        } else {
            i ++;
        }
        pkts --;
		rx ++;
	}
	pgmac->rx_bptr = i; 

handle_exception:
	return rx;
}

static int mac_rx_poll(struct napi_struct *napi, int budget) {
	pgmac_private pgmac = container_of(napi, struct gmac_private, napi);
	int work_done;

#ifdef TEST_JUMBO_FRAME
    work_done = mac_rx_jumbo_pkts(pgmac, budget);
#else
	work_done = mac_rx_pkts(pgmac, budget);
#endif
    gmac_free_tx_skb(pgmac, true);
    ali_trace(4, "%s : work_done %d budget %d\n", __FUNCTION__, work_done, budget);
	if (work_done < budget) {
		napi_complete(napi);
        enable_gmac_isr(pgmac, true);
    }
	return work_done;
}

void housekeeper_tasklet(unsigned long para) {
	u8 tmp_u8;
    u32 mask;
	para = para;

    ali_trace(1, "--->housekeeper_tasklet\n");
	if (g_pgmac->cur_isr & ISRTxUnderrun) {
		if (netif_msg_ifup(g_pgmac)) { 
			ali_warn("%s---> Tx Fifo underrun.!!!", __FUNCTION__);
        }
	}
    mask = (ISRRxComplete|ISRRxFifoOverflow |ISRRxBufOverflow|ISRRxBufDiscard|ISRTimer);
    tmp_u8 = GMAC_R8(SCR);
    if ((tmp_u8 & (u8)SCRBufEmpty)) {
        g_pgmac->mac_stats.rx_buf_empty ++;
    }

    mac_weird_int(g_pgmac);
    if (g_pgmac->cur_isr & (ISRTxComplete|ISRTimer)) {
        gmac_free_tx_skb(g_pgmac, true);
    }
    ali_trace(1, "<---housekeeper_tasklet\n");
    return;
}

int phy_detect(pgmac_private pgmac) {
	u8 phy_id;
	u16 mode_control_reg;
	u16 status_reg;
	struct net_device *dev;
	
	mode_control_reg = 0;
	status_reg = 0;
	phy_id = 0;
	dev = pgmac->dev;

	mode_control_reg = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyBasicModeCtrl);
	status_reg = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyBasicModeStatus);

	if (((mode_control_reg != 0xffff) && (status_reg != 0xffff)) && ((mode_control_reg != 0) && (status_reg != 0))) {
		return 1;
	}

	ali_info("%s: phy address is wrong, start auto detecting, mode_control_reg = 0x%x, status_reg = 0x%x\n", \
             __FUNCTION__, mode_control_reg, status_reg);

	for (phy_id = 0; phy_id < 32; ++phy_id) {
		mode_control_reg = (u16)mac_mdio_read(dev, phy_id, PhyBasicModeCtrl);
		status_reg = (u16)mac_mdio_read(dev, phy_id, PhyBasicModeStatus);

		if ((mode_control_reg != 0xffff) && (status_reg != 0xffff)) {
			ali_info("%s: phy address detected: %d. \n", __FUNCTION__, phy_id);
			gmac_phy_addr = phy_id;
			pgmac->mii_if.phy_id = phy_id;
			return 1;
		}
	}

	ali_info("%s: phy address not detected.\n", __FUNCTION__);
	return -1;
}

void phy_reset(pgmac_private pgmac) {
	struct net_device *dev = pgmac->dev;
	u16 tmp_u16;
	//reset phy registers in a default state.
    tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyBasicModeCtrl);
	mac_mdio_write(dev, gmac_phy_addr, PhyBasicModeCtrl, (tmp_u16 | BMCRReset));

	do {	
		tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyBasicModeCtrl);
	} while(tmp_u16 & BMCRReset);
	GMAC_TRACE("phy_set()=>phy reset complete.");
    return;
}

void phy_set(pgmac_private pgmac) {
#if 0
	struct net_device *dev = pgmac->dev;
	u16 tmp_u16;
	//enable Rx/Tx pause frame.
	tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyNWayAdvert);
	//tmp_u16 |= ANARPause | ANARASMDIR |ANAR_MEDIA;
	tmp_u16 |= ANAR_MEDIA;
	//tmp_u16 = ANAR10FD; //force to 10-full.
	mac_mdio_write(dev, gmac_phy_addr, PhyNWayAdvert, (int)tmp_u16);

	//auto-negotiation enable & restart auto-negotiation.
    /* for FPGA, must set BMCRRestartAN, otherwise, pkts will lost, or ping unreachable */
	tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyBasicModeCtrl);
	tmp_u16 |= BMCRANEnable | BMCRRestartAN; 	
	mac_mdio_write(dev, gmac_phy_addr, PhyBasicModeCtrl, (int)tmp_u16);

#endif
	pgmac->phy_reset = true;
}

void phy_link_chk(pgmac_private pgmac) {
	struct net_device *dev = pgmac->dev; 
	u16 advertisement, link;
	u32	pause;
	u32	asm_dir;
	u32	partner_pause;
	u32	partner_asm_dir;
	u16	expansion = 0;
    u16 status = 0;
	u16	giga_status = 0;
    u16 giga_ctrl = 0;
    u16 ctrl = 0;

 	pgmac->duplex_mode = false;
	pgmac->link_speed = (u32)0; /* 10Mbps */
	
	ctrl = (u16) mac_mdio_read(dev, gmac_phy_addr, PhyBasicModeCtrl);
    /* reg 1 basic mode status */
	status = (u16) mac_mdio_read(dev, gmac_phy_addr, PhyBasicModeStatus);
	/* reg 4 advertisement */
	advertisement = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyNWayAdvert);
	/* reg 6 */
	expansion = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyNWayExpansion);
	/* reg 5 link partner ability*/
	link = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyNWayLPAR);

	/* reg 9 1000baseT control */
	giga_ctrl = (u16)mac_mdio_read(dev, gmac_phy_addr, Phy1000BaseTControl);
	/* reg 0x1 1000 Base T Status */
	giga_status = (u16)mac_mdio_read(dev, gmac_phy_addr, Phy1000BaseTStatus);
    ali_trace(1, "gmac_phy_addr 0x%x status 0x%x advertisement 0x%x expansion 0x%x \nlink 0x%x giga_ctrl 0x%x giga_status 0x%x\n",\
                gmac_phy_addr, status, advertisement, expansion, link, giga_ctrl, giga_status) 
	/* auto nego and complete nego */
	if ((ctrl & BMCRANEnable) && (status & BMSRANComplete)) {
		//used for LinkStatusChg check.	
        pgmac->link_partner = link;
		if (expansion & 0x1) { /* partner support auto nego */
			if ((giga_ctrl & T_1000_FD) && (giga_status & LP_1000_FD)) {
				ali_info("1000 Base-TX full duplex.");
				pgmac->duplex_mode = true;
				pgmac->link_speed = LINK_SPEED_1000M;
			} else if ((giga_ctrl & T_1000_HD) && (giga_status & LP_1000_HD)) {
				ali_info("1000 Base-TX half duplex.");
				pgmac->duplex_mode = false;
				pgmac->link_speed = LINK_SPEED_1000M;
			} else if ((advertisement & ANARTXFD) && (link & ANARTXFD)) {
				ali_info("100 Base-TX full duplex.");
				pgmac->duplex_mode = true;
				pgmac->link_speed = LINK_SPEED_100M;
			} else if ((advertisement & ANARTX) && (link & ANARTX)) {
				ali_info("100 Base-TX half duplex.");
				pgmac->duplex_mode = false;
				pgmac->link_speed = LINK_SPEED_100M;
			} else if ((advertisement & ANAR10FD) && (link & ANAR10FD)) {
				ali_info("10 Base-TX full duplex.");
				pgmac->duplex_mode = true;
				pgmac->link_speed = LINK_SPEED_10M;
			} else {
				ali_info("10 Base-TX half duplex.");
				pgmac->duplex_mode = false;
				pgmac->link_speed = LINK_SPEED_10M;
			}
		} else { /* partner doesn't support auto nego */
			if (link & ANARTXFD) {
				ali_info("100 Base-TX full duplex.");
				pgmac->duplex_mode = true;
				pgmac->link_speed = LINK_SPEED_100M;
			} else if (link & ANARTX) {
				ali_info("100 Base-TX half duplex.");
				pgmac->duplex_mode = false;
				pgmac->link_speed = LINK_SPEED_100M;
			} else if (link & ANAR10FD) {
				ali_info("10 Base-TX full duplex.");
				pgmac->duplex_mode = true;
				pgmac->link_speed = LINK_SPEED_10M;
			} else if (link & ANAR10) {
				ali_info("10 Base-TX half duplex.");
				pgmac->duplex_mode = false;
				pgmac->link_speed = LINK_SPEED_10M;
			} else {
				ali_info("link check doesn't get corrent speed set 10M half\n");
				pgmac->duplex_mode = false;
				pgmac->link_speed = LINK_SPEED_10M;
			}
		}
	} else if (!(ctrl & BMCRANEnable)) {
		if (ctrl & BMCRDuplexMode) {
            pgmac->duplex_mode = true;
		} else {
            pgmac->duplex_mode = false;
		}
		if (!(ctrl & BMCRSpeedSet6) && (ctrl & BMCRSpeedSet13)) {
            pgmac->link_speed = LINK_SPEED_100M;
		} else if (!(ctrl & BMCRSpeedSet13) && !(ctrl & BMCRSpeedSet6)) {
            pgmac->link_speed = LINK_SPEED_10M;
		} else {
            pgmac->link_speed = LINK_SPEED_10M;
			ali_info("%s can't get valid speed, set to 10M\n", __FUNCTION__);
		}
        ali_info("%dM Base-Tx %s duplex\n", \
            (pgmac->link_speed == LINK_SPEED_10M)? 10:100,\
            pgmac->duplex_mode ? "full" : "half");
	} else {
		ali_info("error, shouldn't call %s\n", __FUNCTION__);
		return;
	}

	pause = (advertisement & ANARPause) > 0 ? 1 : 0;
	asm_dir = (advertisement & ANARASMDIR) > 0 ? 1 : 0;
	partner_pause = (link & ANARPause) > 0 ? 1 : 0;
	partner_asm_dir = (link & ANARASMDIR) > 0 ? 1 : 0;

	ali_info("phy_link_chk(): Pause = %d, ASM_DIR = %d, PartnerPause = %d, PartnerASM_DIR = %d. ", pause, asm_dir, partner_pause, partner_asm_dir);
	
	if ((pause == 0) && (asm_dir == 0)) {
		pgmac->pause_frame_rx = false;
		pgmac->pause_frame_tx = false;		
	} else if ((pause == 0) && (partner_pause == 0)) {
		pgmac->pause_frame_rx = false;
		pgmac->pause_frame_tx = false;		
	} else if ((pause == 0) && (asm_dir == 1) && (partner_pause == 1) && (partner_asm_dir == 0)) {
		pgmac->pause_frame_rx = false;
		pgmac->pause_frame_tx = false;		
	} else if ((pause == 0) && (asm_dir == 1) && (partner_pause == 1) && (partner_asm_dir == 1)) {
		pgmac->pause_frame_rx = false;
		pgmac->pause_frame_tx = true;		
	} else if ((pause == 1) && (asm_dir == 0) && (partner_pause == 0)) {
		pgmac->pause_frame_rx = false;
		pgmac->pause_frame_tx = false;		
	} else if ((pause == 1) && (partner_pause == 1)) {
		pgmac->pause_frame_rx = true;
		pgmac->pause_frame_tx = true;		
	} else if ((pause == 1) && (asm_dir == 1) && (partner_pause == 0) && (partner_asm_dir == 0)) {
		pgmac->pause_frame_rx = false;
		pgmac->pause_frame_tx = false;		
	} else if ((pause == 1) && (asm_dir == 1) && (partner_pause == 0) && (partner_asm_dir == 1)) {
		pgmac->pause_frame_rx = true;
		pgmac->pause_frame_tx = false;		
	} else {
		ali_info("Impossiable!\n");
		//asm("sdbbp");
	}

	ali_info("phy_link_chk(): Tx flow control = %d. ", pgmac->pause_frame_tx);
	ali_info("phy_link_chk(): Rx flow control = %d. ", pgmac->pause_frame_rx);
    return;
}

static void gmac_get_drvinfo (struct net_device *dev, \
                              struct ethtool_drvinfo *info) {
	strcpy (info->driver, GMAC_DRV_NAME);
	strcpy (info->version, GMAC_DRV_VER);
	strcpy (info->bus_info, "Local Bus");
    return;
}

static int gmac_get_sset_count (struct net_device *dev, int sset) {
	switch (sset) {
        case ETH_SS_STATS:
            return GMAC_NUM_STATS;
        default:
            return -EOPNOTSUPP;
	}
}

int gmac_mii_ethtool_sset(struct mii_if_info *mii, struct ethtool_cmd *ecmd) {
	struct net_device *dev = mii->dev;
	u32 bmcr;

	mii->mdio_write(dev, mii->phy_id, MII_ADVERTISE, ecmd->advertising);

	/* turn on autonegotiation, and force a renegotiate */
	bmcr = mii->mdio_read(dev, mii->phy_id, MII_BMCR);
	bmcr |= (BMCR_ANENABLE | BMCR_ANRESTART);
	mii->mdio_write(dev, mii->phy_id, MII_BMCR, bmcr);
	return 0;
}

static int gmac_get_settings(struct net_device *dev, struct ethtool_cmd *cmd) {
	pgmac_private pgmac = netdev_priv(dev);
	int rc;
	unsigned long flags;

	ALI_LOCK_IRQ(&pgmac->lock, flags);
	rc = mii_ethtool_gset(&pgmac->mii_if, cmd);
	ALI_UNLOCK_IRQ(&pgmac->lock, flags);

	return rc;
}

static int gmac_set_settings(struct net_device *dev, struct ethtool_cmd *cmd) {
	pgmac_private pgmac = netdev_priv(dev);
	int rc;
	unsigned long flags;

	ALI_LOCK_IRQ(&pgmac->lock, flags);
	if (cmd->advertising == 0) {
		rc = mii_ethtool_sset(&pgmac->mii_if, cmd);
	} else {
		//For test, enable pasue write
		rc = gmac_mii_ethtool_sset(&pgmac->mii_if, cmd);
	}
	ALI_UNLOCK_IRQ(&pgmac->lock, flags);

	return rc;
}

static int gmac_nway_reset(struct net_device *dev) {
	pgmac_private pgmac = netdev_priv(dev);
	return mii_nway_restart(&pgmac->mii_if);
}

static u32 gmac_get_msglevel(struct net_device *dev) {
	pgmac_private pgmac = netdev_priv(dev);
	return pgmac->msg_enable;
}

static void gmac_set_msglevel(struct net_device *dev, u32 value) {
	pgmac_private pgmac = netdev_priv(dev);
	pgmac->msg_enable = value;
    return;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)) /*zmiao*/
#else
static u32 gmac_get_rx_csum(struct net_device *dev) {
	return (u32)gmac_rx_csum;
}

static int gmac_set_rx_csum(struct net_device *dev, u32 data) {
	pgmac_private pgmac = netdev_priv(dev);
	bool chg = false;
	u8 tmp_8;
    u32	flags;

	if(gmac_rx_csum) {
		if(!data) {
			chg = true;
			gmac_rx_csum = false;
		}
	} else {
		if(data) {
			chg = true;
			gmac_rx_csum = true;
		}		
	}
		
	if(chg) {
		ALI_LOCK_IRQ(&pgmac->lock, flags);
		tmp_8 = GMAC_R8(SCR);
		if(gmac_rx_csum == true) {
			GMAC_W8(SCR, (tmp_8 | SCRRxCoeEn));
        } else {
			GMAC_W8(SCR, (tmp_8 & (~SCRRxCoeEn)));
        }
		ALI_UNLOCK_IRQ(&pgmac->lock, flags);
	}

	return 0;
}
#endif

static void gmac_get_regs(struct net_device *dev, struct ethtool_regs *regs, void *p) {
	pgmac_private pgmac = netdev_priv(dev);
	u32 flags;

	if (regs->len < GMAC_REGS_RANGE) {
		return;
    }
	regs->version = GMAC_REGS_VER;

	ALI_LOCK_IRQ(&pgmac->lock, flags);
	memcpy_fromio(p, (void *)pgmac->io_base, GMAC_REGS_RANGE);
	ALI_UNLOCK_IRQ(&pgmac->lock, flags);
    return;
}

static const char gmac_ethtool_gstrings_stats[][ETH_GSTRING_LEN] = {
	"rx_mc",				//muticast packets received.	//ok
	"rx_bc",				//broadcast packets received.	//ok
	"rx_uc",				//unicast packets received.		//ok
	"rx_vlan",										//ok
	"rx_pppoe",			//pppoe packets received.		//ok
	"rx_ip",										//ok
	"rx_ipv6",										//ok
	"rx_frag",										//ok
	"rx_runt_errors",							//ok
	"rx_long_errors",							//ok
	"rx_dribble_errors",						//ok
	"rx_phy_layer_errors",						//ok
	"rx_wd_timeout_errors",					//ok
	"rx_ip_chksum_errors",						//ok
	"rx_tcp_chksum_errors",					//ok
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
	"tx_no_carr_errors",
	"tx_loss_carr_errors",
	"rx_packets",		/* total packets received	*/
	"tx_packets",		/* total packets transmitted	*/
	"rx_bytes",		/* total bytes received 	*/
	"tx_bytes",		/* total bytes transmitted	*/
	"rx_errors",		/* bad packets received		*/
	"tx_errors",		/* packet transmit problems	*/
	"rx_dropped",		/* no space in linux buffers	*/
	"tx_dropped",		/* no space available in linux	*/
	"multicast",		/* multicast packets received	*/
	"collisions",

	/* detailed rx_errors: */
	"rx_length_errors",
	"rx_over_errors",		/* receiver ring buff overflow	*/
	"rx_crc_errors",		/* recved pkt with crc error	*/
	"rx_frame_errors",	/* recv'd frame alignment error */
	"rx_fifo_errors",		/* recv'r fifo overrun		*/
	"rx_missed_errors",	/* receiver missed packet	*/

	/* detailed tx_errors */
	"tx_aborted_errors",
	"tx_carrier_errors",
	"tx_fifo_errors",
	"tx_heartbeat_errors",
	"tx_window_errors",
	
	/* for cslip etc */
	"rx_compressed",
	"tx_compressed",
};

#define GMAC_STATS_LEN	ARRAY_SIZE(gmac_ethtool_gstrings_stats)

static int gmac_ethtool_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
        case ETH_SS_STATS:
            return GMAC_STATS_LEN;
        default:
            return -EOPNOTSUPP;
	}
}

static void gmac_ethtool_get_strings(struct net_device *dev, u32 stringset, u8 *data) {
	switch(stringset) {
        case ETH_SS_STATS:
            memcpy(data, *gmac_ethtool_gstrings_stats, sizeof(gmac_ethtool_gstrings_stats));
            break;
	}	
    return;
}

static void gmac_get_ethtool_stats (struct net_device *dev, \
                      struct ethtool_stats *estats, u64 *data) {
	pgmac_private pgmac = netdev_priv(dev);
	struct gmac_device_stats mac_stats;
	struct net_device_stats net_stats;
	long i;
	long count1;
	long count2;

	gmac_get_stats(dev);
	net_stats = dev->stats;
	mac_stats = pgmac->mac_stats;
	count1 = sizeof(mac_stats) /4;
	count2 = sizeof(struct net_device_stats)/4;

	if ((count1 + count2) != GMAC_STATS_LEN) {
		ali_error("gmac_get_ethtool_stats: error!\n");
		return;
	}
	
	for(i = 0; i < GMAC_STATS_LEN; i++) {
		if (i < count1) {
			data[i] = ((unsigned long *)&mac_stats)[i];
		} else {
			data[i] = ((unsigned long *)&net_stats)[i-count1];
		}
	}
    return;
}

static const struct ethtool_ops gmac_ethtool_ops = {
	.get_drvinfo		= gmac_get_drvinfo,
	.get_sset_count	= gmac_get_sset_count,
	.get_settings		= gmac_get_settings,
	.set_settings		= gmac_set_settings,
	.nway_reset		= gmac_nway_reset,
	.get_link			= ethtool_op_get_link,
	.get_msglevel		= gmac_get_msglevel,
	.set_msglevel		= gmac_set_msglevel,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)) /*zmiao*/
#else
	.get_rx_csum		= gmac_get_rx_csum,
	.set_rx_csum		= gmac_set_rx_csum,
	.set_tx_csum		= ethtool_op_set_tx_csum,
	.set_sg			= ethtool_op_set_sg,
	.set_tso			= ethtool_op_set_tso,
#endif
	.get_regs			= gmac_get_regs,
	.get_strings		= gmac_ethtool_get_strings,
	.get_sset_count	= gmac_ethtool_get_sset_count,
	.get_ethtool_stats	= gmac_get_ethtool_stats,
};

static void gmac_free_tx_skb(pgmac_private priv, int lock) {
	struct sk_buff *skb;
	unsigned long flags;
	u16 tx_rptr, tx_wptr;
	u16 start, first;
	u16 desc_num;
    ptx_desc desc;
    u32 i = 0;
    ali_trace(1, "---> %s\n", __FUNCTION__);
    if (lock) {
        ALI_LOCK_IRQ(&priv->lock, flags);
    }
    tx_rptr = GMAC_R16(TxRingDesRPtr);  
    tx_wptr = GMAC_R16(TxRingDesWPtr);
	while (priv->tx_skb_wr != priv->tx_skb_rd) {
		start = priv->gmac_tx_skb[priv->tx_skb_rd].start;
		first = priv->gmac_tx_skb[priv->tx_skb_rd].first;
		desc_num = priv->gmac_tx_skb[priv->tx_skb_rd].cnt;
		
		if ((start < tx_rptr) && ((tx_rptr - start) >= desc_num)) {
			goto free_skb_dma;
		}
		if ((start > tx_rptr) && (tx_wptr >= tx_rptr) && (start > tx_wptr) &&
			((GMAC_TX_DESC_NUM - (start - tx_rptr)) >= desc_num)) {
			goto free_skb_dma;
		}
        break;
free_skb_dma:
        ali_trace(1, "%s tx_skb_rd %d tx_skb_wr %d\n",\
            __FUNCTION__, priv->tx_skb_rd, priv->tx_skb_wr);
        ali_trace(1, "%s start %d first %d tx_rptr %d desc_num %d\n",\
            __FUNCTION__, start, first, tx_rptr, desc_num);

		if (!priv->gmac_tx_skb[priv->tx_skb_rd].skb) {
			priv->gmac_tx_skb[priv->tx_skb_rd].skb = NULL;
			priv->gmac_tx_skb[priv->tx_skb_rd].first = 0;
			priv->gmac_tx_skb[priv->tx_skb_rd].cnt = 0;
			priv->gmac_tx_skb[priv->tx_skb_rd].start = 0;
			priv->tx_skb_rd++;
			if (priv->tx_skb_rd == GMAC_TX_DESC_NUM) {
				priv->tx_skb_rd = 0;
			}
			continue;
		}

		skb = priv->gmac_tx_skb[priv->tx_skb_rd].skb;
		priv->dev->stats.tx_bytes += (skb->len);
		desc = &priv->tx_desc[start];
        mac_tx_sts(priv, desc);
		while(desc_num > 0) {
			i++;
			if(desc->DataDescriptor.seg_len> 0) {
				dma_unmap_single((struct device *)NULL, __DTCADDRALI(desc->DataDescriptor.pkt_buf_dma), \
                                desc->DataDescriptor.seg_len, DMA_TO_DEVICE);
				desc->DataDescriptor.seg_len = 0;
			}
			
			if(++start >= GMAC_TX_DESC_NUM) {
				start = 0;
			}
			desc = &priv->tx_desc[start];
			desc_num--;
		}

		dev_kfree_skb_any(priv->gmac_tx_skb[priv->tx_skb_rd].skb);
		priv->gmac_tx_skb[priv->tx_skb_rd].skb = NULL;
		priv->gmac_tx_skb[priv->tx_skb_rd].first = 0;
		priv->gmac_tx_skb[priv->tx_skb_rd].cnt = 0;
		priv->gmac_tx_skb[priv->tx_skb_rd].start = 0;
		priv->tx_skb_rd++;
		if (priv->tx_skb_rd == GMAC_TX_DESC_NUM) {
			priv->tx_skb_rd = 0;
		}
    }

	priv->avail_desc_num += i;
	if (priv->avail_desc_num > (GMAC_TX_DESC_NUM-1)) {
		ali_error("error avail_desc_num > %d\n", priv->avail_desc_num);
	}

    if (lock) {
        ALI_UNLOCK_IRQ(&priv->lock, flags);
    }
    ali_trace(1, "<--- %s\n", __FUNCTION__);
    return;
}

void disable_gmac_isr (pgmac_private pgmac, int lock) {
	u32 flags;
	if (lock) {
		ALI_LOCK_IRQ(&pgmac->lock, flags);
	}
    ali_trace(4, "disable IMR (%d) = 0x%x\n", IMR, 0); 
	GMAC_W32(IMR, 0);
    GMAC_R32(IMR);
	if (lock) {
		ALI_UNLOCK_IRQ(&pgmac->lock, flags);
	}
    return;
}

void enable_gmac_isr (pgmac_private pgmac, int lock) {
	u32 flags;
	if (lock) {
		ALI_LOCK_IRQ(&pgmac->lock, flags);
	}
    ali_trace(4, "enable IMR (%d) = 0x%x\n", IMR, pgmac->isr_mask); 
	GMAC_W32(IMR, pgmac->isr_mask);
    GMAC_R32(IMR);
	if (lock) {
		ALI_UNLOCK_IRQ(&pgmac->lock, flags);
	}
    return;
}

void do_dll_pd_reset(pgmac_private pgmac) {
    u32 tmp_val = 0;
    ali_info("WARNING!!! DLL PD need reset WARNING!!!\n");    
    tmp_val = SOC_R32(0x6c);
    tmp_val |= (1<<7);
    SOC_W32(0x6c, tmp_val);
    udelay(250);
    tmp_val = SOC_R32(0x6c);
    tmp_val &= ~(1<<7);
    SOC_W32(0x6c, tmp_val);
    ali_info("WARNING!!! DLL PD reset completed WARNING!!!\n");    
}
irqreturn_t gmac_isr(int Irq, void *dev_id) {
	struct net_device *dev = dev_id;
	pgmac_private pgmac;
	UINT32 cur_isr;

	if (unlikely(dev == NULL)) {
		return IRQ_NONE;
    }
	pgmac = netdev_priv(dev);
	ALI_LOCK(&pgmac->lock);
	cur_isr = GMAC_R32(ISR);
	GMAC_W32(ISR, 0);
	if (unlikely(!netif_running(dev))) {
        ALI_UNLOCK(&pgmac->lock);
		return IRQ_HANDLED;
	}

	if (cur_isr & pgmac->isr_mask) {
		if(cur_isr & ISRRxBufDiscard) {
			ali_info("%s isr(0x%08x).", __FUNCTION__, cur_isr);
        }
		pgmac->cur_isr = (cur_isr & pgmac->isr_mask);	
        if (pgmac->cur_isr & ISRTimer) {
#ifndef USE_GPHY
            link_light_blink(pgmac);
#endif
        }
/*  every 5 sec, it will check times of link change 
    if times of link change is over MAX_LINK_CHANGE_IN_5SEC 
    it mean that PLL clk failed, need do DLL PD reset, otherwise,
    just zero gmac_pll_time and time_of_link_change 
*/
        if (pgmac->cur_isr & ISRTimer) {
            gmac_pll_time++;
            if(gmac_pll_time > gmac_5sec) {
                
                if(test_bit(10, &dbg_runtime_val)) {
                    ali_info("times_of_link_change %d, gmac_pll_time %d gmac_5sec %d\n", pgmac->times_of_link_change, 
                            gmac_pll_time, gmac_5sec);
                }
                if(pgmac->times_of_link_change >= MAX_LINK_CHANGE_IN_5SEC) {
                    do_dll_pd_reset(pgmac);
                }
                pgmac->times_of_link_change = 0;
                gmac_pll_time = 0;
            }
        }
        
        if (pgmac->cur_isr & (ISRLinkStatus|ISRTimer|ISRWatchdog)) {
			tasklet_schedule(&gmac_tasklet);
        }
            disable_gmac_isr(pgmac, 0);
            napi_reschedule(&pgmac->napi);
	} else {
		if((cur_isr & 0x1FFF) && test_bit(14, &dbg_runtime_val)) {
			if (netif_msg_ifup(pgmac)) {
				ali_warn("gmac_isr()=>isr(0x%08x) not supported.", cur_isr);
            }
		}
	}
	ALI_UNLOCK(&pgmac->lock);
	return IRQ_RETVAL(true);
}

static void phy_gain_fine_tune(struct net_device *dev) {
#if 0
    u32 result = 0; 
    u16 tmp_u16;
    int phy_reg;
    ali_info("[XX] ready to gain fine tune from otp:");
    ali_otp_read(0xDF*4, (unsigned char *)(&result), 4);
    ali_info("     fine tune value: 0x%08x\n", result);
    phy_reg = 0x1C;
    tmp_u16 = (UINT16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 &= 0x00FF;
    tmp_u16 |= (((result>>22)&0x0F)<<12);
    tmp_u16 |= (((result>>26)&0x0F)<<8);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);
#endif
}

static int gmac_poweron_sequence(void)
{
    int i=0;
    //"Async reset ETH_TOP"
    u32 tmp_val;
    tmp_val = SOC_R32(0x84);
    tmp_val |= 1<<23;
    SOC_W32(0x84, tmp_val);
    //ali_info("gmac seq1    = 0x%08x", (u32)SOC_R32(0x84));

    //"Enable ENET_DLL_PD & ENET_PLL_PD ENET_SLEEP"
    tmp_val = SOC_R32(0x6c);
    tmp_val |= 1<<7;
    tmp_val |= 1<<1;
    tmp_val |= 1<<0;
    SOC_W32(0x6c, tmp_val);
    //ali_info("gmac seq2    = 0x%08x", (u32)SOC_R32(0x6c));
    udelay(10);

    //"Disable ENET_SLEEP"
    tmp_val = SOC_R32(0x6c);
    tmp_val &= ~(1<<0);
    SOC_W32(0x6c, tmp_val);
    //ali_info("gmac seq3    = 0x%08x", (u32)SOC_R32(0x6c));
    udelay(250);

    //"Disable ENET_PLL_PD"
    tmp_val = SOC_R32(0x6c);
    tmp_val &= ~(1<<1);
    SOC_W32(0x6c, tmp_val);
    //ali_info("gmac seq4    = 0x%08x", (u32)SOC_R32(0x6c));
    udelay(250);

    //"Disable ENET_DLL_PD"
    tmp_val = SOC_R32(0x6c);
    tmp_val &= ~(1<<7);
    SOC_W32(0x6c, tmp_val);
    //ali_info("gmac seq5    = 0x%08x", (u32)SOC_R32(0x6c));
    udelay(250);

    //"Async reset ETH_TOP"
    tmp_val = SOC_R32(0x84);
    tmp_val &= ~(1<<23);
    SOC_W32(0x84, tmp_val);
    //ali_info("gmac seq6    = 0x%08x", (u32)SOC_R32(0x84));
    udelay(250);
#if 0 
    ali_info("Add Power On Patch (1)----> \n");
    tmp_val = SOC_R32(0x6c);
    tmp_val |= 1<<7;
    SOC_W32(0x6c, tmp_val);
    udelay(10);
    ali_info("DLL_PD set 1: 0x%08x", (u32)SOC_R32(0x6c));
    tmp_val = SOC_R32(0x6c);
    tmp_val &= ~(1<<7);
    SOC_W32(0x6c, tmp_val);
    udelay(250);
    ali_info("DLL_PD set 0: 0x%08x", (u32)SOC_R32(0x6c));
    tmp_val = SOC_R32(0x84);
    tmp_val &= ~(1<<23);
    SOC_W32(0x84, tmp_val);
    udelay(10);
    ali_info("IP_RST 0 0x%08x", (u32)SOC_R32(0x84));
#endif
#if 1
    ali_info("Add Power On Patch (2)----> \n");
    tmp_val = SOC_R32(0x6c);
    tmp_val |= 1<<1;
    SOC_W32(0x6c, tmp_val);
    udelay(250);
    ali_info("PLL_PD set 1: 0x%08x", (u32)SOC_R32(0x6c));
    tmp_val = SOC_R32(0x6c);
    tmp_val &= ~(1<<1);
    SOC_W32(0x6c, tmp_val);
    udelay(250);
    //ali_info("PLL_PD set 0: 0x%08x", (u32)SOC_R32(0x6c));
    tmp_val = SOC_R32(0x84);
    tmp_val |= 1<<23;
    SOC_W32(0x84, tmp_val);
    udelay(10);
    //ali_info("IP_RST 1 0x%08x", (u32)SOC_R32(0x84));
    tmp_val = SOC_R32(0x84);
    tmp_val &= ~(1<<23);
    SOC_W32(0x84, tmp_val);
    udelay(10);
    //ali_info("IP_RST 0 0x%08x", (u32)SOC_R32(0x84));
#endif

#if 0
    ali_info("Add Power On Patch (3)----> \n");
    tmp_val = SOC_R32(0x6c);
    tmp_val |= 1<<1;
    SOC_W32(0x6c, tmp_val);
    udelay(10);
    ali_info("PLL_PD set 1: 0x%08x", (u32)SOC_R32(0x6c));
    tmp_val = SOC_R32(0x6c);
    tmp_val &= ~(1<<1);
    SOC_W32(0x6c, tmp_val);
    udelay(250);
    ali_info("PLL_PD set 0: 0x%08x", (u32)SOC_R32(0x6c));
    tmp_val = SOC_R32(0x6c);
    tmp_val |= 1<<7;
    SOC_W32(0x6c, tmp_val);
    udelay(10);
    ali_info("DLL_PD set 1: 0x%08x", (u32)SOC_R32(0x6c));
    tmp_val = SOC_R32(0x6c);
    tmp_val &= ~(1<<7);
    SOC_W32(0x6c, tmp_val);
    udelay(250);
    ali_info("DLL_PD set 0: 0x%08x", (u32)SOC_R32(0x6c));
    tmp_val = SOC_R32(0x84);
    tmp_val &= ~(1<<23);
    SOC_W32(0x84, tmp_val);
    udelay(10);
    ali_info("IP_RST 0 0x%08x", (u32)SOC_R32(0x84));
#endif

#if 0
    ali_info("Add Power On Patch (4)----> \n");
    for(i=0; i< 10; i++) {
        tmp_val = SOC_R32(0x6c);
        tmp_val |= 1<<7;
        SOC_W32(0x6c, tmp_val);
        udelay(10);
        ali_info("%d DLL_PD set 1: 0x%08x",i+1, (u32)SOC_R32(0x6c));
        tmp_val = SOC_R32(0x6c);
        tmp_val &= ~(1<<7);
        SOC_W32(0x6c, tmp_val);
        udelay(250);
        ali_info("%d DLL_PD set 0: 0x%08x", i+1, (u32)SOC_R32(0x6c));
    }
    ali_info("IP_RST 0 0x%08x", (u32)SOC_R32(0x84));
    tmp_val = SOC_R32(0x84);
    tmp_val &= ~(1<<23);
    SOC_W32(0x84, tmp_val);
    udelay(10);
#endif
#if 0
    ali_info("Add Power On Patch (5)----> \n");
    for(i=0; i<10; i++) {
        tmp_val = SOC_R32(0x6c);
        tmp_val |= 1<<1;
        SOC_W32(0x6c, tmp_val);
        ali_info("%d PLL_PD set 1: 0x%08x", i+1, (u32)SOC_R32(0x6c));
        udelay(10);
        tmp_val = SOC_R32(0x6c);
        tmp_val &= ~(1<<1);
        SOC_W32(0x6c, tmp_val);
        udelay(250);
        ali_info("%d PLL_PD set 0: 0x%08x", i+1, (u32)SOC_R32(0x6c));
    }
    tmp_val = SOC_R32(0x84);
    tmp_val &= ~(1<<23);
    SOC_W32(0x84, tmp_val);
    udelay(10);
    ali_info("IP_RST 0 0x%08x", (u32)SOC_R32(0x84));
#endif
    return 0;
}

static int gmac_open (struct net_device *dev) {
	pgmac_private pgmac = netdev_priv(dev);
	UINT32 tmp_u32;
	int i;

    //ali_info("%s: enabling interface.\n", dev->name);
	netif_carrier_off(dev);

    disable_gmac_isr(pgmac, 0);
	
    gmac_poweron_sequence();

	mac_chip_rst();
	
	mac_alloc_rings(pgmac);

	mac_cnt_init(pgmac);

    /* for PLL reset */
    pgmac->times_of_link_change = 0;/* when link change, it shouldn't set to 0 */
    pgmac->unlink_error_state = 0;
    pgmac->para_detect_times = 0;
    pgmac->unlink_with_signal_times = 0;

	tmp_u32 = GMAC_R32(MiiMR1);
#ifdef HW_MDIO
	tmp_u32 &= ~MiiMdioEn;
#else
	tmp_u32 |= MiiMdioEn;
#endif //HW_MDIO
	GMAC_W32(MiiMR1, tmp_u32);

	//detect phy address
	if (phy_detect(pgmac) < 0) {
		ali_error("GMAC: Unable to use phy!\n");
		goto open_err_out;
	}
    set_gmac_reg();
    phy_gain_fine_tune(pgmac->dev);
    phy_reg_set(pgmac);	
	phy_set(pgmac);
	
	i = request_irq(pgmac->irq_num, gmac_isr, 0, dev->name, (void *)dev);
	if(i != 0) {
		ali_error("GMAC: Unable to use IRQ = %d, Error number = %d\n",pgmac->irq_num, i);
        pgmac->acquired_isr = false;
		goto open_err_out;
	}
	pgmac->acquired_isr = true;

	netif_start_queue(dev);
    napi_enable(&pgmac->napi);
	//ali_info("	gmac_rx_mode = %s\n", ((gmac_rx_mode == GMAC_RX_MODE_NAPI)?"NAPI":"Tasklet"));
	//ali_info("	gmac_sg = %d\n", gmac_sg);
	//ali_info("	gmac_tso = %d\n", gmac_tso);
	//ali_info("	gmac_ufo = %d\n", gmac_ufo);
	//ali_info("	crc_det_err = %d\n", crc_det_err);
	//ali_info("<---- gmac_open is done!\n");
	GMAC_W32(TimerR, gmac_timer_frq);
	GMAC_W16(IMR, (ISRTimer|ISRLinkStatus));
    //ali_info("enable gpio\n");
#ifndef USE_GPHY
    gpio_enable_pin(g_enet_speed_gpio);
    gpio_enable_pin(g_enet_link_gpio);
#endif
	return 0;
open_err_out:
	mac_free_rings(pgmac);
	return -EFAULT;
}

static int gmac_close (struct net_device *dev) {
	struct gmac_private *pgmac = netdev_priv(dev);
	UINT32 flags;
	
    ali_info("%s: disabling interface\n", dev->name);
	
	ALI_LOCK_IRQ(&pgmac->lock, flags);
	GMAC_W16(IMR, 0);
	GMAC_W8(SCR, 0);
	ALI_UNLOCK_IRQ(&pgmac->lock, flags);
	
	if(pgmac->acquired_isr) {
		free_irq(pgmac->irq_num, dev);
		pgmac->acquired_isr = false;
	}
    napi_disable(&pgmac->napi);
	netif_stop_queue(dev);
	netif_carrier_off(dev);	
	mac_free_rings(pgmac);
	return 0;
}

void gmac_set_rx_mode(struct net_device *dev) {
	pgmac_private pgmac = netdev_priv(dev);
    struct netdev_hw_addr_list * mc;
    struct netdev_hw_addr * ha;
    
	u8 cmd_u8;
	UINT32 cmd_u32, flags;
	int i;
#if 0
    mc = &dev->mc;
	ALI_LOCK_IRQ(&pgmac->lock, flags);

	cmd_u32 = GMAC_R32(NetworkOM);
	cmd_u32 &= ~(PassMask);

	if(dev->flags & IFF_PROMISC) {
		GMAC_W32(NetworkOM, (cmd_u32|PassPromiscuous));
		ali_info("%s: cmd_u32 %x.", __FUNCTION__, (cmd_u32|PassPromiscuous));
		ali_info("%s: (dev->flags & IFF_PROMISC).", __FUNCTION__);
		ali_info("%s: (dev->mtu = %d).", __FUNCTION__, dev->mtu);
        
#ifdef TEST_JUMBO_FRAME /* ic doesn't support to send over 1500 bytes pkt, so can't change mtu to enable jumbo pkt */
        UINT32 mtu_cmd;
        UINT32 RxChkSum_cmd;
        mtu_cmd = GMAC_R32(TxRxCR2);
        if(dev->mtu > (16*1024-1)) {
            ali_error("gmac doesn't support packet size over 16k\n");
        } else if (dev->mtu > 1500) {
            printk("dev->mtu set to %d\n", dev->mtu);
            RxChkSum_cmd = GMAC_R32(RxChkSumStartOff);
            RxChkSum_cmd &= 0xFFFFC000;
            RxChkSum_cmd |= dev->mtu;
            printk("mtu_cmd %x RxChkSum_cmd %x\n", (mtu_cmd|RxMaxLenEn), RxChkSum_cmd);
            GMAC_W32(TxRxCR2, (mtu_cmd|RxMaxLenEn)); 
            GMAC_W32(RxChkSumStartOff, RxChkSum_cmd); 
        } else {
            printk("dev->mtu is %d and set mac maxlen %d\n", dev->mtu, 6000);
            RxChkSum_cmd = GMAC_R32(RxChkSumStartOff);
            RxChkSum_cmd &= 0xFFFFC000;
            RxChkSum_cmd |= 6000;
            ali_info("mtu_cmd %x RxChkSum_cmd %x\n", (mtu_cmd|RxMaxLenEn), RxChkSum_cmd);
            GMAC_W32(TxRxCR2, (mtu_cmd|RxMaxLenEn)); 
            GMAC_W32(RxChkSumStartOff, RxChkSum_cmd); 
        } 
        ali_info("%s Max_Len_Enable %x Packet_len %x\n", \
            __FUNCTION__, GMAC_R32(TxRxCR2), GMAC_R32(RxChkSumStartOff));
#endif
	} else if ((dev->flags & IFF_ALLMULTI) ||(dev->mc.count >14)) {
		GMAC_W32(NetworkOM, (cmd_u32|PassAllMulticast));
		ali_info("%s()=>((dev->flags & IFF_ALLMULTI) ||( dev->mc.count >14)).", __FUNCTION__);
	} else {
		if (dev->mc.count == 0) {
			ali_warn("%s()=>((dev->mc.count == 0)\n", __FUNCTION__);
			goto multicast_done;
		}

		if (netif_msg_probe(pgmac)) {
            mc = &dev->mc;
            i = 0;
            list_for_each_entry(ha, &mc->list, list) {
                ali_info("	%d. mc ether addr = %02x-%02x-%02x-%02x-%02x-%02x", i++,
                                            ha->addr[0], ha->addr[1], ha->addr[2], 
                                            ha->addr[3], ha->addr[4], ha->addr[5]);
            }
		}

		if (0 != mac_set_mc_filter(dev)) {
			//build setup frame failed, set mc all.
			GMAC_W32(NetworkOM, (cmd_u32|PassAllMulticast));

			//make sure rx & tx enabled.
			cmd_u8 = GMAC_R8(SCR);
			GMAC_W8(SCR, cmd_u8 | (SCRRxEn| SCRTxEn));
			goto multicast_done;
		}
    }
multicast_done:
	ALI_UNLOCK_IRQ(&pgmac->lock, flags);
#endif
}

static struct net_device_stats * gmac_get_stats(struct net_device *dev) {
	pgmac_private pgmac = netdev_priv(dev);
	unsigned long flags;
	u16 tmp_u16;

	ALI_LOCK_IRQ(&pgmac->lock, flags);

 	if (netif_running(dev) && netif_device_present(dev)) {
		tmp_u16 = GMAC_R16(MFC);
		//printk("Missed error = %d\n", tmp_u16);
		GMAC_W16(MFC, 0);
		dev->stats.rx_missed_errors += tmp_u16;
		dev->stats.rx_over_errors += tmp_u16;
		//pgmac->rx_stats.rx_missed_errors += tmp_u16;

		tmp_u16 = GMAC_R16(PPC);
		//printk("FIFO error = %d\n", tmp_u16);
		GMAC_W16(PPC, 0);
		dev->stats.rx_fifo_errors+= tmp_u16;
		//pgmac->rx_stats.rx_fifo_errors += tmp_u16;
		

		tmp_u16 = GMAC_R16(LFC);
		//printk("Long error = %d\n", tmp_u16);
		GMAC_W16(LFC, 0);
		dev->stats.rx_length_errors += tmp_u16;
		//pgmac->rx_stats.rx_long_errors += tmp_u16;
		pgmac->mac_stats.rx_long_errors += tmp_u16;
		

		tmp_u16 = GMAC_R16(RPC);
		//printk("Runt error = %d\n", tmp_u16);
		GMAC_W16(RPC, 0);
		dev->stats.rx_length_errors += tmp_u16;
		//pgmac->rx_stats.rx_runt_errors += tmp_u16;
		pgmac->mac_stats.rx_runt_errors += tmp_u16;
		

		tmp_u16 = GMAC_R16(CrcErrCnt);
		//printk("CRC error = %d\n", tmp_u16);
		GMAC_W16(CrcErrCnt, 0);
		dev->stats.rx_crc_errors += tmp_u16;
		//pgmac->rx_stats.rx_crc_errors += tmp_u16;
		

		tmp_u16 = GMAC_R16(AlignErrCnt);
		//printk("Align error = %d\n", tmp_u16);
		GMAC_W16(AlignErrCnt, 0);
		dev->stats.rx_frame_errors += tmp_u16;
		//pgmac->rx_stats.rx_align_errors += tmp_u16;
 	}
	ALI_UNLOCK_IRQ(&pgmac->lock, flags);
	return &dev->stats;
}

int process_mac_tool_ioctl(cmd_param_t * cmd_param) {
    if (cmd_param->cmd == READ_MAC_REG) {
        cmd_param->value = GMAC_R32(cmd_param->reg);
        ali_info("Mac Reg 0x%02x is 0x%08x\n", cmd_param->reg, cmd_param->value);
    }
    if (cmd_param->cmd == READ_SOC_REG) {
        cmd_param->value = SOC_REG32(cmd_param->reg);
        ali_info("Soc Reg 0x%02x is 0x%08x\n", cmd_param->reg, cmd_param->value);
    }

    if (cmd_param->cmd == WRITE_MAC_REG) {
        GMAC_W32(cmd_param->reg, cmd_param->value);
        ali_info("Mac Reg 0x%02x is 0x%08x\n", cmd_param->reg, GMAC_R32(cmd_param->reg));
    }

    if (cmd_param->cmd == WRITE_SOC_REG) {
        SOC_REG32(cmd_param->reg) = cmd_param->value;
        ali_info("Soc Reg 0x%02x is 0x%08x\n", cmd_param->reg, (u32)SOC_REG32(cmd_param->reg));
    }

    if (cmd_param->cmd == DEBUG_DRIVER_LEVEL) {
        if (cmd_param->value) {
            set_bit(cmd_param->reg, &dbg_runtime_val);
        } else {
            clear_bit(cmd_param->reg, &dbg_runtime_val);
        }
        ali_info("dbg=0x%x", (u32)dbg_runtime_val);
        ali_info("dbg0=0x%x", test_bit(0, &dbg_runtime_val));//
        ali_info("dbg1=0x%x", test_bit(1, &dbg_runtime_val));//link status
        ali_info("dbg2=0x%x", test_bit(2, &dbg_runtime_val));//rx pkts
        ali_info("dbg3=0x%x", test_bit(3, &dbg_runtime_val));//tx pkts
        ali_info("dbg4=0x%x", test_bit(4, &dbg_runtime_val));//
        ali_info("dbg5=0x%x", test_bit(5, &dbg_runtime_val));//stp/mc pkt 
        ali_info("dbg6=0x%x", test_bit(6, &dbg_runtime_val));//cks, nonport, mc
        ali_info("dbg7=0x%x", test_bit(7, &dbg_runtime_val));//xmit print
        ali_info("dbg8=0x%x", test_bit(8, &dbg_runtime_val));//udp recv print
        ali_info("dbg9=0x%x", test_bit(9, &dbg_runtime_val));//udp recv print2
        ali_info("dbg10=0x%x", test_bit(10, &dbg_runtime_val));//alloc
        ali_info("dbg11=0x%x", test_bit(11, &dbg_runtime_val));//async dma 2
        ali_info("dbg12=0x%x", test_bit(12, &dbg_runtime_val));
        ali_info("dbg13=0x%x", test_bit(13, &dbg_runtime_val));//forward ports
    }

    return 0;
}

/* dbg 1 for link state
       2 for rx
       3 for tx */
static int gmac_ioctl(struct net_device *dev, struct ifreq *rq, int cmd) {
	int rc;
	unsigned long flags;
    ali_mac_data_t * mac_data;
    cmd_param_t * cmd_param;
	struct ali_mac_xmit_io util_xmit_cmd;
	pgmac_private pgmac = netdev_priv(dev);
#if 0
    u8 tmp8;
#endif

	if (!netif_running(dev)) {
		return -EINVAL;
    }
    rc = 0;
	if ((cmd >= SIOCDEVPRIVATE) && (cmd <= SIOCDEVPRIVATE + 15)) {
        ALI_LOCK_IRQ(&pgmac->lock, flags);
		switch (cmd) {
			case SIOCDEVPRIVATE+1: { /* read regs */
				mac_data = (ali_mac_data_t *) &(rq->ifr_ifru);
				if (mac_data->reg > GMAC_REGS_RANGE) {
					ali_info ("The requested register is out of range!\n");
					break;
				}
				mac_data->value = GMAC_R32(mac_data->reg);
				break;
			}

			case SIOCDEVPRIVATE+2: { /* write regs */
				mac_data = (struct ali_mac_data_t *) &(rq->ifr_ifru);
				if (mac_data->reg  > GMAC_REGS_RANGE) {
					ali_info ("The requested register is out of range!\n");
					break;
				}
				GMAC_W32(mac_data->reg, mac_data->value);
				break;
			}

			case SIOCDEVPRIVATE+3: {
                ali_info("clean ethtool counts\n");
                memset(&(dev->stats), 0, sizeof(struct net_device_stats));
                memset(&(pgmac->mac_stats), 0, sizeof(struct gmac_device_stats));
                pgmac->num_rx_complete = 0;
                pgmac->num_timer = 0;
				break;
			}

			case SIOCDEVPRIVATE+4: {
				ali_reg_t ali_reg;
				u32 val;
				ali_info("Read/Write alireg\n");
				if (copy_from_user(&ali_reg, rq->ifr_data, sizeof(ali_reg)))
				{
					ali_info("copy_from_user error\n");
                    ALI_UNLOCK_IRQ(&pgmac->lock, flags);
					return -EINVAL;
				}
				if (ali_reg.cmd == 0)
				{
					ali_info("read phy_addr is %x ", ali_reg.phy_addr);	
					val = SOC_REG32(ali_reg.phy_addr); //__REG32ALI(ali_reg.phy_addr);	
					ali_info("val is %08x\n", val);	
				}
				if (ali_reg.cmd == 1) {
					ali_info("write phy_addr is %x ", ali_reg.phy_addr);	
					ali_info("      val is %x\n", ali_reg.val);	
					SOC_REG32(ali_reg.phy_addr) = ali_reg.val; //__REG32ALI(ali_reg.phy_addr) = ali_reg.val;		
				}			
				break;
			}

			//*********** test patch: LIMIT_PKTS *************//
			case SIOCDEVPRIVATE+5:
			{
				break;
			}
			//*********** test patch: LIMIT_PKTS *************//

            case SIOCDEVPRIVATE+11: {/* MAC_TOOL_IOCTL_CMD */
                cmd_param = (cmd_param_t*) &(rq->ifr_data); 
                ali_info("get cmd from mac_tool\n");
                if (cmd_param->reg > GMAC_REGS_RANGE) {
                    ali_info ("The requested register is out of range!\n");
                    rc = -EOPNOTSUPP;
                }
                if (cmd_param->cmd == TX_PKTS) {
                    ali_info("Begin gmac utiliy!\n");
#if 0
                    ali_info("disable Rx..\n");
                    tmp8 = GMAC_R8(SCR);
                    tmp8 &= (~SCRRxEn & ~SCRRxCoeEn); 
                    GMAC_W8(SCR, tmp8);
                    ali_info("mask all isr except timer isr\n");
                    pgmac->isr_mask = ISRLinkStatus|ISRTimer|ISRWatchdog; 
                    GMAC_W32(IMR, pgmac->isr_mask);
#endif                
                    ali_info("%s %d: tx_pkt test start...",__FUNCTION__,__LINE__);
                    if (copy_from_user(&util_xmit_cmd, cmd_param->data, sizeof(struct ali_mac_xmit_io))) {
                        ali_info("copy_from_user error\n");
                        rc = -EINVAL;
                    }
                    start_xmit_test(pgmac, &util_xmit_cmd);
                    ali_info("%s %d: tx_pkt test done!",__FUNCTION__,__LINE__);
                } else {
                    printk("recv rx compelete isr %d timer isr %d\n", pgmac->num_rx_complete,
                        pgmac->num_timer);
                    rc = process_mac_tool_ioctl(cmd_param);
                }
                break; 
            }
            case SIOCDEVPRIVATE+12: { /* DLL PD reset patch for APP */
                do_dll_pd_reset(pgmac);
                break;
            }
			default :
			{
				rc = -EOPNOTSUPP;
				break;
			}
		}
        ALI_UNLOCK_IRQ(&pgmac->lock, flags);
        return rc;
    }
    
    ALI_LOCK_IRQ(&pgmac->lock, flags);
	rc = generic_mii_ioctl(&pgmac->mii_if, if_mii(rq), cmd, NULL);
    ALI_UNLOCK_IRQ(&pgmac->lock, flags);
	return rc;
}

void mac_tx_sts(pgmac_private pgmac, ptx_desc desc) {
	if((desc->SatusDescriptor.FS) && !(desc->SatusDescriptor.OWN)) {
		if (!(desc->SatusDescriptor.ES)) {
			pgmac->dev->stats.tx_packets++;
        } else { 
			pgmac->dev->stats.tx_errors++;
			if(desc->SatusDescriptor.LossOfCarrier) {
				pgmac->dev->stats.tx_carrier_errors++;
				pgmac->mac_stats.tx_loss_carr_errors++;
			}

			if(desc->SatusDescriptor.NoCarrier) {
				pgmac->dev->stats.tx_carrier_errors++;
				pgmac->mac_stats.tx_no_carr_errors++;
			}
			
			if (desc->SatusDescriptor.LateCol) {
				pgmac->dev->stats.tx_window_errors++;
            }

			if (desc->SatusDescriptor.FifoUnderrun) {
				pgmac->dev->stats.tx_fifo_errors++;
            }

			if (desc->SatusDescriptor.HF) {
				pgmac->dev->stats.tx_heartbeat_errors++;
            }
		}
		
		if (desc->SatusDescriptor.ExCol) {
			pgmac->mac_stats.tx_col_errors++;
        } else {
			pgmac->mac_stats.tx_col_cnts[desc->SatusDescriptor.ColCnt]++;
        }
	}
}

void mac_tx_data_desc_cfg(pgmac_private pgmac, u32 off, ptx_desc desc, struct sk_buff *skb, \
                          void *seg_addr, u16 seg_len) {

	desc->DataDescriptor.pkt_buf_dma= dma_map_single((struct device *)NULL, seg_addr, \
                                                        seg_len, DMA_TO_DEVICE);
	desc->DataDescriptor.pkt_buf_dma = __CTDADDRALI(desc->DataDescriptor.pkt_buf_dma);
    if(!verify_buf(pgmac, skb->data, desc->DataDescriptor.pkt_buf_dma, seg_len)) {
        ali_error("WARNING..............................\n");
    }

    pgmac->tx_skb[off] = skb; 

    if (skb->ip_summed == CHECKSUM_PARTIAL) {
        if(gmac_tx_csum) { 
				desc->DataDescriptor.CoeEn = 1;
				desc->DataDescriptor.TOE_UFO_En = 1; //peter, or should be 0 here?
        } else { 
				ali_trace(1, "%s()=>Tx cks not enabled.", __FUNCTION__); 
				desc->DataDescriptor.CoeEn = 0;
				//asm("sdbbp"); 
        } 
    } else {
			desc->DataDescriptor.CoeEn = 0;
			desc->DataDescriptor.TOE_UFO_En = 0;
    }

#if GMAC_VLAN_TAG_USED 
    if (pgmac->vlgrp && vlan_tx_tag_present(skb)) { 
        desc->DataDescriptor.VlanEn = 1; 
    } else 
#endif 
        desc->DataDescriptor.VlanEn= 0; 	
    return;
}

static int cfg_tx_context_desc(pgmac_private priv, \
                               ptx_desc desc, struct sk_buff *skb) {
    struct iphdr *ipHeader = NULL;
    struct ipv6hdr *ipv6Header = NULL;
	struct net_device *dev = priv->dev;
	struct skb_shared_info *skb_info = skb_shinfo(skb);
    u8 ipv6nxthdr_type = 0;
    u8 loop = 0;
    u8* pNxthdr = NULL;
	u16 gso_type = 0;
    u32 hdr_len;

    memset(desc, 0, GMAC_DESC_SZ);
    desc->ContextDescriptor.OWN = 1;
    desc->ContextDescriptor.ContextData = 1;
    
	gso_type = skb_info->gso_type;

    if(gso_type & SKB_GSO_TCPV4) {
        hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
        desc->ContextDescriptor.MSS = skb_info->gso_size; 
    } else if (gso_type & SKB_GSO_UDP) {
        desc->ContextDescriptor.MSS  = dev->mtu + 14;
    } else if (gso_type & SKB_GSO_TCPV6) {
        hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
        desc->ContextDescriptor.MSS = skb_info->gso_size; 
    } else {	
        desc->ContextDescriptor.MSS = dev->mtu + 14;
    }

    if (skb->protocol == htons(ETH_P_IP)) {
        ipHeader = ip_hdr(skb); 
        desc->ContextDescriptor.L3HeaderLen = (ipHeader->ihl*4);
        desc->ContextDescriptor.L2HeaderLen = 14;
        desc->ContextDescriptor.L3Type = 0;
        if(ipHeader->protocol == IPPROTO_TCP) {
            desc->ContextDescriptor.L4Type = 0;
            desc->ContextDescriptor.L4HeaderLen= tcp_hdrlen(skb);
        } else if(ipHeader->protocol == IPPROTO_UDP) {
            desc->ContextDescriptor.L4Type = 1; 
            desc->ContextDescriptor.L4HeaderLen= 8; 	
        } else { 
            ali_warn ("%s()=>Pkt type unrecognized while configing context.", __FUNCTION__); 
        } 
    } else if(skb->protocol == htons(ETH_P_IPV6)) {
        ipv6Header = (struct ipv6hdr *)skb_network_header(skb);
        desc->ContextDescriptor.L3HeaderLen= 40; 
        desc->ContextDescriptor.L2HeaderLen = 14;
        desc->ContextDescriptor.L3Type = 1;

        ipv6Header = (struct ipv6hdr *)skb_network_header(skb);
        ipv6nxthdr_type = (u8)(ipv6Header->nexthdr);
        pNxthdr = (u8 *)skb->network_header + 40;
        
        while((ipv6nxthdr_type != IPPROTO_TCP) && (ipv6nxthdr_type != IPPROTO_UDP)) {
            loop++;
            ipv6nxthdr_type = *(u8 *)pNxthdr;//to find TCP or UDP header...
            pNxthdr+= ((*(u8 *)(pNxthdr+1)+1)*8);
            if(loop>=7) {
                break;
            }
        }

        desc->ContextDescriptor.L3HeaderLen = pNxthdr - (u8 *)(skb->network_header);

        if(ipv6nxthdr_type == IPPROTO_TCP) {
            desc->ContextDescriptor.L4Type = 0;
            desc->ContextDescriptor.L4HeaderLen = tcp_hdrlen(skb);//peter, right?
        } else if(ipv6nxthdr_type == IPPROTO_UDP) {
            desc->ContextDescriptor.L4Type = 1; 
            desc->ContextDescriptor.L4HeaderLen= 8; 	
        } else { 
            ali_warn("%s()=>Pkt type unrecognized while configing context\n", __FUNCTION__);
        }
    } else { 
        ali_warn("%s protocol %d Neithor IP nor IPv6 pkts", __FUNCTION__, skb->protocol);;
    }
        
#if GMAC_VLAN_TAG_USED 
    if (pgmac->vlgrp && vlan_tx_tag_present(skb)) { 
        desc->ContextDescriptor.VlanTag= swab16(vlan_tx_tag_get(skb)); 
    } 
#endif 
    return 0;
}
void show_tx_data_desc(ptx_desc desc, int no, int idx) {
   printk("desc No %d\n", no);
   printk("SegmentNum:%d\n", desc->DataDescriptor.SegmentNum);
   printk("Reserved:%d\n", desc->DataDescriptor.Reserved20_16);
   printk("TOE_UFO_En:%d\n", desc->DataDescriptor.TOE_UFO_En);
   printk("VlanEn:%d\n", desc->DataDescriptor.VlanEn);
   printk("CoeEn:%d\n", desc->DataDescriptor.CoeEn);
   printk("LS:%d\n", desc->DataDescriptor.LS);
   printk("FS:%d\n", desc->DataDescriptor.FS);
   printk("EOR:%d\n", desc->DataDescriptor.EOR);
   printk("ContextIndex:%d\n", desc->DataDescriptor.ContextIndex);
   printk("ContextData:%d\n", desc->DataDescriptor.ContextData);
   printk("OWN:%d\n", desc->DataDescriptor.OWN);
   printk("pkt_buf_dma:0x%x\n", desc->DataDescriptor.pkt_buf_dma);
   printk("seg_len:%d\n", desc->DataDescriptor.seg_len);
   printk("tot_len:%d\n", desc->DataDescriptor.tot_len);
   printk("FragmentID:%d\n", desc->DataDescriptor.FragmentID);
    return;
}

static int gmac_start_xmit(struct sk_buff *skb, struct net_device *dev) {
	UINT32 flags;
	ptx_desc desc;
	u32 desc_num;
	u32 off; 
	u32 gso_type = 0;
	int err;
	int fragIdx;
	void *seg_addr;
	struct skb_shared_info *skb_info = skb_shinfo(skb);
	static tx_data_desc tmp_tx_desc, tmp2_tx_desc; 
	pgmac_private pgmac = netdev_priv(dev);
    u32 need_desc_num = 0;
    volatile u32 first = 0; 
    volatile u32 start = 0;
    skb_frag_t *this_frag = NULL;
    u32 have_context_desc = 0;

	ALI_LOCK_IRQ(&pgmac->lock, flags);

	ali_trace(3, "--> gmac_start_xmit! skb %x\n", (u32)skb);

	if(!pgmac->link_established) {
		dev_kfree_skb_any(skb);	
        ali_trace(3, "%s: !link_established", __FUNCTION__);
        ALI_UNLOCK_IRQ(&pgmac->lock, flags);
		goto xmit_done;
	}

    gmac_free_tx_skb(pgmac, 0);

	desc_num = skb_info->nr_frags + 1;

#if 0
	if((skb->ip_summed != CHECKSUM_PARTIAL) &&\
       (skb->ip_summed != CHECKSUM_NONE)) {
        ali_error("error skb->ip_summed is %d\n", skb->ip_summed);
        gmac_print(skb->data, skb->len + RING_CRC_SZ);
        ALI_UNLOCK_IRQ(&pgmac->lock, flags);
		return -1;
	}
#endif

	if(skb_is_gso(skb)||skb_is_gso_v6(skb)) {
		ali_trace(3, ">>skb_info->gso_size=%d, shinfo->gso_type=0x%x!\n", \
                  skb_shinfo(skb)->gso_size, skb_shinfo(skb)->gso_type);
		if(skb_header_cloned(skb)) {
			err = pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
			if(err) {
                ali_error("%s: pskb_expand_head error.\n", __FUNCTION__);
                ALI_UNLOCK_IRQ(&pgmac->lock, flags);
				return err;
            }
		}
	}

	if(gmac_tx_csum && (skb->ip_summed == CHECKSUM_PARTIAL)) {
        need_desc_num = desc_num + 1;
    } else {
        need_desc_num = desc_num ;
    }

    ali_trace (3, "gmac tx_skb_rd %d tx_skb_wr %d desc_num %d\n", \
                  pgmac->tx_skb_rd, pgmac->tx_skb_wr, desc_num);

	if (pgmac->avail_desc_num < need_desc_num) {
		ali_trace(5, "###dma busy, avail:%u desc_num+1 %u\n", pgmac->avail_desc_num, desc_num+1);
		ali_trace(5, "###dma busy, tx_skb_rd %d tx_skb_wr %d\n", pgmac->tx_skb_rd, pgmac->tx_skb_wr);
		//netif_stop_queue(dev); 
        ALI_UNLOCK_IRQ(&pgmac->lock, flags);
		return NETDEV_TX_BUSY;
	}
	gso_type = skb_info->gso_type;
	off = pgmac->tx_wptr;

	//1. configure the context descriptor...
    start = off;
	have_context_desc = 0;
	if(gmac_tx_csum && (skb->ip_summed == CHECKSUM_PARTIAL)) {
		ali_trace(3, "configure the context descriptor...\n");
		desc = &pgmac->tx_desc[off];
        cfg_tx_context_desc(pgmac, desc, skb);
		if((++off) >= GMAC_TX_DESC_NUM) {
			off = 0;
        }
		have_context_desc = 1;
    } else {
		ali_trace(3, "no need to configure the context descriptor...\n");
	}

	/***************************************************************************************/
	//2. configure the data descriptor...
	first = off;
	if(skb_shinfo(skb)->nr_frags == 0) {
		//ali_info("skb->data = 0x%x, skb->len = %d, skb_shinfo(skb)->nr_frags = %d\n", skb->data, skb->len, skb_shinfo(skb)->nr_frags);

		desc = &pgmac->tx_desc[off];
		memset(desc, 0, GMAC_DESC_SZ);

		if(off == GMAC_TX_DESC_NUM -1) {
			desc->DataDescriptor.EOR = 1;
        }
		desc->DataDescriptor.seg_len = skb->len;
		desc->DataDescriptor.tot_len = skb->len;
		desc->DataDescriptor.OWN = 1;
		desc->DataDescriptor.ContextData = 0;
		desc->DataDescriptor.FS = 1;
		desc->DataDescriptor.LS = 1;
		desc->DataDescriptor.SegmentNum = 1;
		desc->DataDescriptor.FragmentID = skb_shinfo(skb)->ip6_frag_id;

		mac_tx_data_desc_cfg(pgmac, off, desc, skb, skb->data, skb->len);
		if((++off) >= GMAC_TX_DESC_NUM) {
			off = 0;
        }
	} else {
		//ali_trace(1, "configure the data descriptor...skb_shinfo(skb)->nr_frags != 0\n");
		//ali_trace(1, "	skb->data = 0x%x, skb->len = %d, skb_shinfo(skb)->nr_frags = %d\n", skb->data, skb->len, skb_shinfo(skb)->nr_frags);
        
		//ali_info("skb->data = 0x%x, skb->len = %d, skb_shinfo(skb)->nr_frags = %d\n", skb->data, skb->len, skb_shinfo(skb)->nr_frags);

		//1. configure the first data descriptor...
		desc = &pgmac->tx_desc[off];
		memset(desc, 0, GMAC_DESC_SZ);
		
		if(off == GMAC_TX_DESC_NUM -1) {
			desc->DataDescriptor.EOR = 1;
        }
		desc->DataDescriptor.seg_len = skb_headlen(skb);
		desc->DataDescriptor.tot_len = skb->len;
		desc->DataDescriptor.OWN = 1;
		desc->DataDescriptor.ContextData = 0;
		desc->DataDescriptor.FS = 1;
		desc->DataDescriptor.SegmentNum = desc_num;
		desc->DataDescriptor.FragmentID = skb_shinfo(skb)->ip6_frag_id;

		mac_tx_data_desc_cfg(pgmac, off, desc, skb, skb->data, skb_headlen(skb));//note, it's skb_headlen(skb)!
		if((++off) >= GMAC_TX_DESC_NUM) {
			off = 0;
        }
		
		//2. configure the other data descriptor...
		for (fragIdx = 0; fragIdx < skb_shinfo(skb)->nr_frags; fragIdx++) 
		{
			this_frag = &skb_shinfo(skb)->frags[fragIdx];
			seg_addr = (void *)page_address(skb_frag_page(this_frag)) + this_frag->page_offset;

			desc = &pgmac->tx_desc[off];
			memset(desc, 0, GMAC_DESC_SZ);

			if(off == GMAC_TX_DESC_NUM -1) {
				desc->DataDescriptor.EOR = 1;
            }
			desc->DataDescriptor.seg_len = this_frag->size;
			desc->DataDescriptor.tot_len = skb->len;
			desc->DataDescriptor.OWN = 1;
			desc->DataDescriptor.ContextData = 0;
			if(fragIdx == (skb_shinfo(skb)->nr_frags - 1)) {
				desc->DataDescriptor.LS = 1;
            }
			desc->DataDescriptor.SegmentNum = desc_num;
			mac_tx_data_desc_cfg(pgmac, off, desc, skb, seg_addr, this_frag->size);//note, it's this_frag->size!
			if((++off) >= GMAC_TX_DESC_NUM) {
				off = 0;
            }
		}
	}

	//3. trigger to start DMA...
	ali_trace(3, "trigger to start xmit\n");
	wmb();
	memcpy (&tmp_tx_desc, desc, sizeof(tx_data_desc));
	memcpy (&tmp2_tx_desc, desc, sizeof(tx_data_desc));
    
    if(test_bit(4, &dbg_runtime_val)) {
        show_tx_data_desc(desc, off-1, 0);
    }
	pgmac->tx_wptr = off;
	GMAC_W16(TxRingDesWPtr, off); 

	pgmac->gmac_tx_skb[pgmac->tx_skb_wr].first = first;
	pgmac->gmac_tx_skb[pgmac->tx_skb_wr].cnt = desc_num + have_context_desc;
	pgmac->gmac_tx_skb[pgmac->tx_skb_wr].skb = skb;
	pgmac->gmac_tx_skb[pgmac->tx_skb_wr].start = start;
	pgmac->avail_desc_num -= pgmac->gmac_tx_skb[pgmac->tx_skb_wr].cnt;
	if (pgmac->avail_desc_num < 0) {
		ali_error("something error, priv->avail_desc_num < 0");
	}

	pgmac->tx_skb_wr++;
	if (pgmac->tx_skb_wr == GMAC_TX_DESC_NUM) {
		pgmac->tx_skb_wr = 0;
	}
#ifndef USE_GPHY
    if(!pgmac->blink_light && !pgmac->in_blink) {
        pgmac->blink_light = 1;
    }
#endif
    ali_trace(3, "tx_skb_wr %d first %d cnt %d skb %x start %d\n",\
			pgmac->tx_skb_wr, first, desc_num+have_context_desc, (u32)skb, start);
xmit_done:	
	ALI_UNLOCK_IRQ(&pgmac->lock, flags);
	ali_trace(3, "<--gmac_start_xmit \n");
	return 0;
}

static void gmac_tx_timeout(struct net_device *dev) {
	pgmac_private pgmac = netdev_priv(dev);
	unsigned long flags; 

	ali_warn("%s: gmac_tx_timeout()...", __FUNCTION__);

	ALI_LOCK_IRQ(&pgmac->lock, flags);

	mac_desc_clean(pgmac);
	mac_chip_rst();
	mac_cnt_init(pgmac);
	mac_hw_start(pgmac);

	netif_wake_queue(dev);
	
	ALI_UNLOCK_IRQ(&pgmac->lock, flags);
	return;
}


#if GMAC_VLAN_TAG_USED
static void gmac_vlan_rx_register(struct net_device *dev, struct vlan_group *grp)
{
	pgmac_private pgmac = netdev_priv(dev);
	unsigned long flags, tmp_u32;

	ALI_LOCK_IRQ(&pgmac->lock, flags);
	
	pgmac->vlgrp = grp;
	pgmac->vlan_tag_remove = true;
	
	tmp_u32 = GMAC_R32(TxRxCR2);
	GMAC_W32(TxRxCR2, (tmp_u32|RxRemoveVlanTagEn|VlanEn));

	ALI_UNLOCK_IRQ(&pgmac->lock, flags);
}
#endif //GMAC_VLAN_TAG_USED

#define VLAN_HLEN 4
#if 0
static int gmac_change_mtu(struct net_device *netdev, int new_mtu) {
	struct gmac_private * pgmac =  netdev_priv(netdev);
    int max_frame_size = new_mtu + ETH_HLEN + ETH_FCS_LEN;
    int jumbo_flag = 0;
	u8 cmd_u8;
	UINT32 cmd_u32, flags;
	int i;
    UINT32 mtu_cmd;
    UINT32 RxChkSum_cmd;

	ALI_LOCK_IRQ(&pgmac->lock, flags);
    mtu_cmd = GMAC_R32(TxRxCR2);
    if (max_frame_size > ETH_FRAME_LEN + ETH_FCS_LEN) {
        jumbo_flag = 1;
        if(max_frame_size > (16*1024-1)) {
            printk("gmac doesn't support packet size over 16k\n");
            ALI_UNLOCK_IRQ(&pgmac->lock, flags);
            return -EINVAL;
        }
        printk("set dev->mtu from %d to %d\n", netdev->mtu, new_mtu);
        RxChkSum_cmd = GMAC_R32(RxChkSumStartOff);
        RxChkSum_cmd &= 0xFFFFC000;
        RxChkSum_cmd |= netdev->mtu;
        printk("mtu_cmd %x RxChkSum_cmd %x\n", (mtu_cmd|RxMaxLenEn), RxChkSum_cmd);
        GMAC_W32(TxRxCR2, (mtu_cmd|RxMaxLenEn)); 
        GMAC_W32(RxChkSumStartOff, RxChkSum_cmd); 
    } else {
        mtu_cmd &= ~RxMaxLenEn;
        GMAC_W32(TxRxCR2, mtu_cmd); 
    }
	ALI_UNLOCK_IRQ(&pgmac->lock, flags);
    ali_info("%s Max_Len_Enable %x Packet_len %x\n", \
        __FUNCTION__, GMAC_R32(TxRxCR2), GMAC_R32(RxChkSumStartOff));
    if (new_mtu < (ETH_ZLEN + ETH_FCS_LEN + VLAN_HLEN)) {
        printk("Unsupported %d MTU setting \n", new_mtu);
        return -EINVAL;
    } 
    pgmac->max_frame_size = max_frame_size;  
    netdev->mtu = new_mtu;
    if (netif_running(netdev)) {
        // interface down 
        gmac_close(netdev);
    }
    if (netif_running(netdev)) {
        // interface up
        gmac_open(netdev);
    } else {
        // e1000e _reset
    }
    return 0;
}
#endif

int gmac_eth_validate_addr(struct net_device *dev) {
    if (!is_valid_ether_addr(dev->dev_addr))
        return -EADDRNOTAVAIL;
    return 0;
}

static int gmac_set_mac_address(struct net_device *netdev, void *p) {
    struct sockaddr *addr = p;
	struct gmac_private * priv = netdev_priv(netdev);
	u32 flags;
    u32 tmp;

    if (!is_valid_ether_addr(addr->sa_data)) {
        return -EADDRNOTAVAIL;
    }
	ALI_LOCK_IRQ(&priv->lock, flags);
    memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
    ali_info("Ali Sw Mac len %d\n", netdev->addr_len);
    ali_info("set Ali Sw Mac: ");

    separate_mac_addr(netdev->dev_addr, &gmac_mac_hi16, &gmac_mac_lo32);
	GMAC_W32(PAR - 4, gmac_mac_hi16);
	GMAC_W32(PAR, gmac_mac_lo32);

	tmp = GMAC_R32(PAR);
	ali_info("mac reg 0x08 (gmac_mac_lo32) = 0x%x\n", tmp);
	tmp = GMAC_R32(PAR-4);
	ali_info("mac reg 0x04 (gmac_mac_hi16) = 0x%x\n", tmp);
	ALI_UNLOCK_IRQ(&priv->lock, flags);
    return 0;
}

static const struct net_device_ops gmac_netdev_ops = {
	.ndo_open		= gmac_open,
	.ndo_stop		= gmac_close,
	.ndo_validate_addr	= eth_validate_addr,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)) /*zmiao*/
    .ndo_set_rx_mode =  gmac_set_rx_mode,
#else 
	.ndo_set_multicast_list	= gmac_set_rx_mode,
#endif
	.ndo_get_stats		= gmac_get_stats,
	.ndo_do_ioctl		= gmac_ioctl,
	.ndo_start_xmit		= gmac_start_xmit,
	.ndo_tx_timeout		= gmac_tx_timeout,
    .ndo_validate_addr  = gmac_eth_validate_addr,
    .ndo_set_mac_address    = gmac_set_mac_address,
#if GMAC_VLAN_TAG_USED
	.ndo_vlan_rx_register	= gmac_vlan_rx_register,
#endif
};

static int gmac_attach(void) {
	struct net_device *dev;
	struct gmac_private *pgmac;
	int re;

	dev = alloc_etherdev(sizeof(struct gmac_private));//device name, it's eh%d, %d will be changed by register_netdev.
	if (!dev) {
        ali_error("error allo_etherdev \n");
		return -ENOMEM;
    }

	pgmac = netdev_priv(dev);
	g_pgmac = pgmac;
	
	pgmac->dev = dev;
	pgmac->io_base = gmac_base;
	pgmac->irq_num = gmac_irq;
	
	pgmac->tx_skb_wr = 0;
	pgmac->tx_skb_rd = 0;
	pgmac->avail_desc_num = GMAC_TX_DESC_NUM- 1;

	pgmac->msg_enable = (gmac_debug < 0 ? GMAC_DEF_MSG_ENABLE : gmac_debug);

    ali_info("---->%s \n", __FUNCTION__);
	spin_lock_init (&pgmac->lock);

	pgmac->mii_if.dev = dev;
	pgmac->mii_if.mdio_read = mac_mdio_read;
	pgmac->mii_if.mdio_write = mac_mdio_write;
	pgmac->mii_if.phy_id = gmac_phy_addr;
	pgmac->mii_if.phy_id_mask = 0x1f;
	pgmac->mii_if.reg_num_mask = 0x1f;
    
    memcpy(dev->dev_addr, stb_gmac_mac, ETH_ALEN);
    separate_mac_addr(stb_gmac_mac, &gmac_mac_hi16, &gmac_mac_lo32);
    
	dev->irq = pgmac->irq_num;

	dev->netdev_ops = &gmac_netdev_ops;

#ifndef TEST_RX_DESC_BUG
    netif_napi_add(dev, &pgmac->napi, mac_rx_poll, 64);
#else
    netif_napi_add(dev, &pgmac->napi, mac_rx_poll, 64);
#endif
	
	dev->ethtool_ops = &gmac_ethtool_ops;
	dev->watchdog_timeo = GMAC_TX_TIMEOUT;

#if GMAC_VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX |NETIF_F_HW_VLAN_RX;
#endif

	if(gmac_sg) {
		dev->features |= NETIF_F_SG |NETIF_F_FRAGLIST;
    }

	if(gmac_tx_csum) {	
		dev->features |= NETIF_F_IP_CSUM|NETIF_F_IPV6_CSUM; 
	}

	if(gmac_tso) {	
		dev->features |= NETIF_F_TSO;
		dev->features |= NETIF_F_TSO6;
		netif_set_gso_max_size(dev, 65536);
	}
	
	if(gmac_ufo) {
		dev->features |= NETIF_F_UFO |NETIF_F_HW_CSUM;
	}
	
	dev->flags |= IFF_PROMISC|IFF_MULTICAST;

	re = register_netdev(dev);
	if (re) {
		ali_error("error register %d\n", re);
		goto err_out_free;
    }
    ali_info("<----%s \n", __FUNCTION__);
	return 0;

err_out_free:
	free_netdev(dev);
	return re;
}

void init_stb_mac() {
    u8 tmp_mac_addr[ETH_ALEN];
    u32 board_info_addr = (u32)(phys_to_virt)PRE_DEFINED_ADF_BOOT_START;
    ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO*)board_info_addr;
    memset(tmp_mac_addr, 0, (sizeof(u8)*ETH_ALEN));	
    if (is_valid_ether_addr(info->macinfo.phyaddr1)) {
        memcpy(stb_gmac_mac, info->macinfo.phyaddr1, sizeof(u8)*ETH_ALEN);
    } else {
        printk("boot mac isn't valid, use default stb mac\n");
    }
    return;
}

void set_soc_reg(void) {
    u32 reg32 = 0;
    ali_info("set soc registers\n");
    reg32 = SOC_R32(0x6C);
#ifndef USE_GPHY
    SOC_W32(0x6c, 0x7fffffff & reg32);
#else
    ali_info("set to gphy\n");
    SOC_W32(0x6c, 0x80000000 | reg32);
#endif
    ali_info("0x6c reg is %x\n", (u32)SOC_R32(0x6C));
}
static int __init gmac_module_init(void) {
    int i = 0;

    init_stb_mac();
#if 0
	ali_info("Compiled: %s, %s",__DATE__,__TIME__);
	ali_info("gmac_base			= 0x%08x", gmac_base);
	ali_info("gmac_irq			= %d", gmac_irq);
    for(i=0; i<ETH_ALEN; i++) {
       printk("%2x ", stb_gmac_mac[i]); 
    }
	ali_info("gmac_phy_addr		= %d", gmac_phy_addr);

	if(gmac_rx_mode == GMAC_RX_MODE_TASKLET) {
		ali_info("gmac_rx_mode		= 0x%08x, Tasklets enabled", gmac_rx_mode);
	} else if(gmac_rx_mode == GMAC_RX_MODE_NAPI) {
		ali_info("gmac_rx_mode		= 0x%08x, NAPI enabled", gmac_rx_mode);
	} else {
		ali_info("gmac_rx_mode		= 0, use ISR");
    }
	
	ali_info("gmac_phy_mode			= %d", gmac_phy_mode);
	ali_info("gmac_sg			= %s", (gmac_sg == true)? "true":"false");	
	ali_info("gmac_tx_csum		= %s", (gmac_tx_csum == true)? "true":"false");
	ali_info("gmac_rx_csum		= %s", (gmac_rx_csum == true)? "true":"false");
	ali_info("gmac_tso			= %s", (gmac_tso== true)? "true":"false");
	ali_info("gmac_ufo			= %s", (gmac_ufo== true)? "true":"false");
#endif

#ifdef TEST_JUMBO_FRAME
    ali_info("test jumbo frame!!!\n");
#endif
    g_chip_id = (u16) (SOC_R32(0) >> 16);
    //ali_info("Chip ID: %x\n", g_chip_id);
    set_soc_reg();
    gmac_5sec = 6 * mac_clk / gmac_timer_frq;  
	return gmac_attach();
}

void gmac_module_exit(void) {
	struct net_device *dev = NULL;
	ali_info("--->gmac_module_exit");
	if(g_pgmac != NULL) {
		dev = g_pgmac->dev;
		mac_free_rings(g_pgmac);
		g_pgmac = NULL;
	}
	unregister_netdev(dev);
	free_netdev(dev); 
	ali_info("<---gmac_module_exit().");
}


void phy_reg_set(pgmac_private pgmac) {
#ifndef USE_GPHY
    int phy_reg = 0;
    u16 tmp_u16;
	struct net_device *dev = pgmac->dev;
    //Step1: set FSM to disable state, this process will shut down all related PHY's DSP part.
    phy_reg = 0x10;
    tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 |= (1<<0);  //Reset main FSM to IDLE State
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);

    //Step2: configure the autoneg ability, that we can't support next page exchange.
#ifndef PHY_IOL_TEST
    //For Mass product, do not modify this register
    phy_reg = 0x04;
    tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 &= ~(1<<15);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);
#else
    //For IOL test, the following register should be added
    phy_reg = 0x04;
    tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 |= (1<<15);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);

    phy_reg = 0x11;
    tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 &= ~(1<<10);
    tmp_u16 &= ~(1<<11);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);
#endif

    //Step3: configure the AGC threshold.
    mac_mdio_write(dev, gmac_phy_addr, 0x1D, 0x07);  //select the AGC debug register
    phy_reg = 0x1F;
    tmp_u16 = (UINT16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    //tmp_u16 &= ~(1<<1);
    //tmp_u16 |= (1<<2);
    tmp_u16 &= ~(1<<7);
    tmp_u16 &= ~(1<<8);
    tmp_u16 |= (1<<9);
    tmp_u16 &= ~(1<<10);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);
    phy_reg = 0x1E;
    tmp_u16 = (UINT16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 &= ~(1<<8);
    tmp_u16 &= ~(1<<9);
    tmp_u16 |= (1<<10);
    tmp_u16 |= (1<<11);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);

    //Step4: configure the DAC clock phase.
    phy_reg = 0x1C;
    tmp_u16 = (UINT16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 &= ~(1<<4);
    tmp_u16 &= ~(1<<5);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);

    //Step5: enlarge the AGC kill det windows length to 8us.
    mac_mdio_write(dev, gmac_phy_addr, 0x1D, 0x0A);
    phy_reg = 0x1F;
    tmp_u16 = (UINT16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 |= (1<<0);
    tmp_u16 |= (1<<1);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);

    //Step6: enlarge the lower threshold of slicer.
#if 0
    mac_mdio_write(dev, gmac_phy_addr, 0x1D, 0x03);
    phy_reg = 0x1F;
    tmp_u16 = (UINT16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 |= (1<<8);
    tmp_u16 |= (1<<9);
    tmp_u16 &= ~(1<<10);
    tmp_u16 |= (1<<11);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);
#endif

    //Step7: decrease the BLW gain when in tracking killer pattern.
    mac_mdio_write(dev, gmac_phy_addr, 0x1D, 0x0B);
    phy_reg = 0x1F;
    tmp_u16 = (UINT16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 |= (1<<5);
    tmp_u16 &= ~(1<<6);
    tmp_u16 |= (1<<7);
    tmp_u16 &= ~(1<<8);
    tmp_u16 &= ~(1<<9);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);

    //Step8: enlarge the TX template swing.
    mac_mdio_write(dev, gmac_phy_addr, 0x1C, 0xC400);
    mac_mdio_write(dev, gmac_phy_addr, 0x1d, 0x8);
    mac_mdio_write(dev, gmac_phy_addr, 0x1e, 0x0020);

    //Step9: set FSM to start.
    mac_mdio_write(dev, gmac_phy_addr, 0x1D, 0);
    phy_reg = 0x10;
    tmp_u16 = (UINT16)mac_mdio_read(dev, gmac_phy_addr, phy_reg);
    tmp_u16 &= ~(1<<0);
    mac_mdio_write(dev, gmac_phy_addr, phy_reg, tmp_u16);
#else
    ali_info("Use GPHY\n");
#endif
}

#if 0
static void __init parse_mac_addr(char *macstr) {
	int i, j;
	unsigned char result, value;
    u8 tmp_mac_addr[ETH_ALEN];
    u32 tmp1=0, tmp2=0, tmp3=0, tmp4=0;

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
	tmp1 = tmp_mac_addr[0];
	tmp2 = tmp_mac_addr[1];
	tmp3 = tmp_mac_addr[2];
	tmp4 = tmp_mac_addr[3];
    tmp2 = tmp2 << 8;
    tmp3 = tmp3 << 16;
    tmp4 = tmp4 << 24;
	/* set mac address. */
    gmac_mac_lo32 = tmp1 | tmp2 | tmp3 | tmp4; 
	tmp1 = tmp_mac_addr[4];
	tmp2 = tmp_mac_addr[5];
    tmp2 = tmp2 << 8;
    gmac_mac_hi16 = tmp1 | tmp2;
    use_default_mac = 0;
    return;
}

static int __init program_setup_kmac(char *s) {
	if(s) {
		ali_info("cmdline: mac = %s\n", s);
		parse_mac_addr(s);
	}
	return 0;
}
__setup("mac=", program_setup_kmac);
#endif

module_init(gmac_module_init);
module_exit(gmac_module_exit);
