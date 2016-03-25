/*
   All related Ethernet TOE II Definitions
   On 2010.03.25
   */

#ifndef __ETHERNET_TOE2__H
#define __ETHERNET_TOE2__H

#include <asm/mach-ali/typedef.h>
#include <ali_reg.h>

#define NAPI_MODE_SUPPORT
//#define UTILITY_SUPPORT

#define HW_MDIO
//#define PHY_SMSC

#define TOE2_STATUS
//#define SW_CRCCHK

//#define PHY_IOL_TEST

/* --------only debug use-------- */
#define TOE2_SHOW_LINK_INFO
//#define TOE2_DEBUG_INFO
//#define TOE2_DEBUG_TRACE
//#define TOE2_DEBUG_WARNING

#ifdef NAPI_MODE_SUPPORT
#define DYNAMIC_CHANGING_IMK //IMK -- interrupt mask MUST ENABLE NAPI_MODE_SUPPORT for DYNAMIC_CHANGING_IMK!
//#define TOE2_NAPI_DEBUG
//#define DYNAMIC_CHANGING_IMK_DBG
#endif

#ifdef TOE2_SHOW_LINK_INFO
#	define TOE2_LINK(msg,args...) \
	printk("[TOE2] " msg "\n", ## args)
#else
#	define TOE2_LINK(msg,args...)
#endif

#ifdef TOE2_DEBUG_INFO
#	define TOE2_INFO(msg,args...) \
	printk("TOE2: " msg "\n", ## args)
#else
#	define TOE2_INFO(msg,args...)
#endif

#ifdef TOE2_DEBUG_TRACE
#	define TOE2_TRACE(msg,args...) \
	printk("TOE2: " msg "\n", ## args)
#else
#	define TOE2_TRACE(msg,args...)
#endif

#ifdef TOE2_DEBUG_WARNING
#	define TOE2_WARNING(msg,args...) \
	printk("TOE2: " msg "\n", ## args)
#else
#	define TOE2_WARNING(msg,args...)
#endif

#ifdef DYNAMIC_CHANGING_IMK_DBG
#	define DNY_INT_TRACE(msg,args...) \
	printk(KERN_INFO "TOE2: " msg "\n", ## args)
#else
#	define DNY_INT_TRACE(msg,args...)
#endif
/* ------------------------------ */

//TOE Base address.
#define _TOE2_BASE		0x1802C000//0xB802C000
#define _TOE2_IRQ		(6+32+8)

#define TOE2_PHY_ADDR	5 //5 //1 //17 

//VLAN tagging feature enable/disable
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define TOE2_VLAN_TAG_USED 1
#else
#define TOE2_VLAN_TAG_USED 0
#endif

#define TOE2_DEF_MSG_ENABLE 0x7fff //(NETIF_MSG_DRV |NETIF_MSG_PROBE |NETIF_MSG_LINK)

#define TOE2_TX_TIMEOUT	    (2*HZ)

//Link mode bits.
#define TOE2_10HD		(0x01UL)
#define TOE2_10FD		(0x02UL)
#define TOE2_100HD		(0x04UL)
#define TOE2_100FD		(0x08UL)
#define TOE2_RX_PAUSE	(0x10UL)
#define TOE2_TX_PAUSE	(0x20UL)
#define TOE2_AUTONEG	(0x40UL)

//Rx Mode
#define TOE2_RX_MODE_ISR		(0UL)
#define TOE2_RX_MODE_TASKLET	(1UL)
#define TOE2_RX_MODE_NAPI		(2UL)

//Rx Filtering mode
#define TOE2_RX_FILTER_PERFECT	(0UL)
#define TOE2_RX_FILTER_HASH		(1UL)
#define TOE2_RX_FILTER_INVERSE	(2UL)
#define TOE2_RX_FILTER_HASHONLY	(3UL)

#define RING_CRC_SZ				4
#define MAX_TOE2_PKT_LEN		(64*1024)

#ifdef CONFIG_ALI_TOE2_128M
#define TOE2_RX_DESC_NUM		64//512
#define TOE2_TX_DESC_NUM		64//512
#define TOE2_BUF_SZ				1536
#define TOE2_DESC_SZ			16
#else
#define TOE2_RX_DESC_NUM		512
#define TOE2_TX_DESC_NUM		128
#define TOE2_BUF_SZ				1536
#define TOE2_DESC_SZ			16
#endif

#define TOE2_MSS				1514

//Setup frame size.
#define SETUP_FRAME_SZ			192
#define IMPERFECT_PA_OFF		156
//Hash
#define TOE2_HASH_TABLE_LEN	    512		//64 bytes
#define TOE2_HASH_BITS			0x01ff	//=512-1

#define TOE2_REGS_RANGE			0x98
#define TOE2_REGS_VER			1

#define HIBYTE(word)  ((u8)(((u16)(word))>>8))
#define LOBYTE(word)  ((u8)(((u16)(word))&0x00FFU))
#define HIWORD(dword) ((u16)(((UINT32)(dword))>>16))
#define LOWORD(dword) ((u16)(((UINT32)(dword))&0x0000FFFFUL))

typedef struct teo2_tx_desc
{
	union {
		struct {
			UINT32 Reserved1_0: 2;			//bit 1~0.Reserved.
			UINT32 HF: 1;					//bit 2. Heartbeat Fail Error.
			UINT32 FifoUnderrun: 1; 		//bit 3. Fifo Underrun Error.
			UINT32 ColCnt: 4;				//bit 7~4. Collision Count.
			UINT32 ExCol: 1;				//bit 8. Excessive Collsion.
			UINT32 LateCol: 1;				//bit 9. Late Collsion.
			UINT32 NoCarrier: 1;			//bit 10. No Carrier.
			UINT32 LossOfCarrier: 1;		//bit 11. Loss Of Carrier..
			UINT32 ES: 1;					//bit 12. Error Sumary.
			UINT32 Reserved27_13: 15;		//bit 27~13. Reserved.
			UINT32 LS: 1;					//bit 28. indicate a Last Segment Descriptor.
			UINT32 FS: 1;					//bit 29. indicate a First Segment Descriptor.
			UINT32 Reserved30: 1;			//bit 30. indicate End Of descriptor Ring.
			UINT32 OWN: 1;					//bit 31. 0->Owned by SW, 1->Owned by HW.
		} sw;

		struct {
			UINT32 Mfl: 13; 				//bit 12~0.Maximum Frame Length(Without CRC)
			UINT32 VlanEn: 1;				//bit 13.
			UINT32 IpStartOff: 8;			//bit 21~14.IP header offset.
			UINT32 UdpPkt: 1;				//bit 22. indicate UDP packet type.
			UINT32 TcpPkt: 1;				//bit 23. indicate TCP packet type.
			UINT32 CoeEn: 1;				//bit 24. enable Tx COE.
			UINT32 SetupPkt: 1; 			//bit 25. indicate Setup packet type.
			UINT32 FilteringMode: 2;		//bit 27~26. filtering mode for a setup packet.
			UINT32 LS: 1;					//bit 28. indicate a Last Segment Descriptor.
			UINT32 FS: 1;					//bit 29. indicate a First Segment Descriptor.
			UINT32 EOR: 1;					//bit 30. indicate End Of descriptor Ring.
			UINT32 OWN: 1;					//bit 31. 0->Owned by SW, 1->Owned by HW.		
		} hw;
	} tx_sts;

	dma_addr_t pkt_buf_dma;
	u16 seg_len;
	u16 tot_len;
	u16 vlan_tag;
	u16 seg_num;			
}__attribute__((packed)) *ptx_desc;

//head structure
typedef struct packet_head
{
	u16	PacketLength: 12;

	u16	TCPChksum: 1;			//TCP Checksum is Okay.
	u16	IPChksum: 1;			//IP Checksum is Okay.	
	u16	PPPoE: 1;				//PPPoE packet.
	u16	VLAN: 1;				//VLAN Fram.

	u16	WatchdogTimeout: 1;		//Receive Watchdog Time-out.
	u16	FrameType : 1;			//Frame Type.
	u16	PhysicalLayerError: 1;	//Physical Layer Error.
	u16	FifoOverflow: 1;		//FIFO Overflow Error. 
	u16	FAE : 1;				//Alignment Error.
	u16	CRC: 1;					//CRC error.
	u16	LateCollision: 1;		//Late Collision Seen.
	u16	Long : 1;				//Excessive Frame Length.
	u16	Runt: 1;				//Runt Frame.
	u16	ES: 1;					//Error Summary.
	u16	BF : 1;					//Broadcast Frame.
	u16	PF : 1; 				//Physical Frame.
	u16	MF : 1;					//Multicast Frame.
	u16	Dribble: 1;				//Dribble Error.
	u16	IPFrag: 1;				//IP Fragment.
	u16	IPFrame: 1;				//IP Frame.
}__attribute__((packed)) *ppacket_head;

typedef struct teo2_rx_desc
{
	struct packet_head pkt_hdr;
	u16 l3_chs;
	u16 l4_chs;
	u16 vlan_tag;
	u16 reserved: 15;
	u16 EOR: 1;
	dma_addr_t pkt_buf_dma;
}__attribute__((packed)) *prx_desc;

typedef struct toe2_device_stats
{
	UINT32	rx_mc;				//muticast packets received.	//ok
	UINT32	rx_bc;				//broadcast packets received.	//ok
	UINT32	rx_uc;				//unicast packets received.		//ok
	UINT32	rx_vlan;			     							//ok
	UINT32	rx_pppoe;			//pppoe packets received.		//ok
	UINT32	rx_ip;					   				//ok
	UINT32	rx_frag;								//ok

	UINT32	rx_runt_errors;							//ok
	UINT32	rx_long_errors;							//ok
	UINT32	rx_dribble_errors;						//ok
	UINT32	rx_phy_layer_errors;					//ok
	UINT32	rx_wd_timeout_errors;					//ok
	UINT32	rx_ip_chksum_errors;					//ok
	UINT32	rx_tcp_chksum_errors;					//ok

	UINT32	rx_buf_empty;		//there is no packet stored in rx ring buffer.	
	UINT32	rx_late_col_seen;	
	UINT32 	rx_lost_in_ring;

	UINT32 	rx_hdr_chs_errs;						//ok
	UINT32 	rx_pay_chs_errs;						//ok

	UINT32	tx_col_cnts[16];	//0 to 15 collisions.		//ok
	UINT32	tx_col_errors;		//excessive collision.		//ok
}*ptoe2_device_stats;

/*
   typedef struct toe2_rx_stats {
   unsigned long rx_missed_errors;
   unsigned long rx_fifo_errors;
   unsigned long rx_long_errors;
   unsigned long rx_runt_errors;
   unsigned long rx_crc_errors;
   unsigned long rx_align_errors;

   }*ptoe2_rx_stats;
   */

#define TOE2_NUM_STATS 36

typedef struct toe2_private
{
	struct net_device	*dev;
	UINT32 io_base;

	bool acquired_isr;	
	unsigned int irq_num;
	UINT32 cur_isr;
	UINT32 isr_mask;

	spinlock_t lock;
	UINT32 msg_enable;

	struct napi_struct napi;

	struct mii_if_info mii_if;

	struct toe2_device_stats mac_stats;
	//struct toe2_rx_stats rx_stats;

	u8 *setup_buf;
	dma_addr_t setup_buf_dma;

	prx_desc rx_desc;
	dma_addr_t rx_desc_dma;
	struct sk_buff	 *rx_skb[TOE2_RX_DESC_NUM];

	ptx_desc tx_desc;
	dma_addr_t tx_desc_dma;
	struct sk_buff	 *tx_skb[TOE2_TX_DESC_NUM];

	u16 rx_wptr, rx_bptr;
	u16 tx_wptr;

	//link status change. 
	bool phy_reset;
	bool auto_n_completed;
	bool link_established;
	bool transmit_okay;

	bool pause_frame_rx;
	bool pause_frame_tx;
	bool duplex_mode;

	UINT32 link_speed;
	u16 link_partner;	

#if TOE2_VLAN_TAG_USED
	struct vlan_group	*vlgrp;
	bool vlan_tag_remove;
#endif

#ifdef TOE2_NAPI_DEBUG
	bool during_pkt_rx;
	UINT32 polling_times;
	UINT32 has_been_interrupted;
	UINT32 total_interrupt;
	UINT32 irq_due_to_rx_complete;
	UINT32 irq_due_to_timer;
	UINT32 rx_small_times;
	UINT32 rx_big_times;
#endif

#ifdef DYNAMIC_CHANGING_IMK
	UINT32 spot_rx_complete_int_HZ;//time spot
	UINT32 num_rx_complete_int_HZ;//number
	UINT32 num_zero_timer_int_HZ;//number
	UINT32 timer_freq;
	bool cur_dny_imk;

#ifdef DYNAMIC_CHANGING_IMK_DBG
	UINT32 num_rx_complete;
	UINT32 num_timer;
#endif
#endif

    /* for PLL bug */
    UINT32 times_of_link_change; 
    UINT32 unlink_error_state;
    UINT32 para_detect_times;
    UINT32 unlink_with_signal_times;

	bool blink_light;
	bool in_blink;
} *ptoe2_private;

struct ali_mac_priv_io
{
	unsigned short  reg;
	unsigned long  value;
};

#define SOC_BASE_ADDR       0x18000000
#define SOC_R32(i)          __REG32ALI(SOC_BASE_ADDR+(i))
#define SOC_W32(i, val)     __REG32ALI(SOC_BASE_ADDR+(i)) = (UINT32)(val)

#define TOE2_R8(reg)		(*(volatile u8 *)(ba + (reg)))
#define TOE2_R16(reg)		(*(volatile u16 *)(ba + (reg)))
#define TOE2_R32(reg)		(*(volatile u32 *)(ba + (reg)))

#define TOE2_W8(reg, val)	(*(volatile u8 *)(ba + (reg))) = (u8)(val)	
#define TOE2_W16(reg, val)	(*(volatile u16 *)(ba + (reg))) = (u16)(val)	
#define TOE2_W32(reg, val)	(*(volatile u32 *)(ba + (reg))) = (UINT32)(val)

#endif //__ETHERNET_TOE2__H

