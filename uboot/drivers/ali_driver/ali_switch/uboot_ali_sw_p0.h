/*
 *	Linux ALi switch
 *
 *	Authors:
 */

#ifndef __ETHERNET_ALI_SW__H
#define __ETHERNET_ALI_SW__H
#include "uboot_ali_sw_types.h"
//VLAN tagging feature enable/disable
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
//#define ALI_SW_VLAN_TAG_USED 1
#define ALI_SW_VLAN_TAG_USED 0
#else
#define ALI_SW_VLAN_TAG_USED 0
#endif

#define ALI_SW_DEF_MSG_ENABLE \
			0x7fff//(NETIF_MSG_DRV |NETIF_MSG_PROBE |NETIF_MSG_LINK)
			
//#define TX_ASYNC_DMA
#define TX_CONTEXT_SELECTION

#define ALI_SW_TX_TIMEOUT	(5*HZ)

//Link mode bits.
#define ALI_SW_10HD		(0x01UL)
#define ALI_SW_10FD		(0x02UL)
#define ALI_SW_100HD		(0x04UL)
#define ALI_SW_100FD		(0x08UL)
#define ALI_SW_RX_PAUSE	(0x10UL)
#define ALI_SW_TX_PAUSE	(0x20UL)
#define ALI_SW_AUTONEG	(0x40UL)

//Rx Mode
#define ALI_SW_RX_MODE_ISR		(0UL)
#define ALI_SW_RX_MODE_TASKLET	(1UL)
#define ALI_SW_RX_MODE_NAPI		(2UL)

//Rx Filtering mode
#define ALI_SW_RX_FILTER_PERFECT	(0UL)
#define ALI_SW_RX_FILTER_HASH		(1UL)
#define ALI_SW_RX_FILTER_INVERSE		(2UL)
#define ALI_SW_RX_FILTER_HASHONLY	(3UL)

#define RING_CRC_SZ				4
#define MAX_ALI_SW_PKT_LEN		(64*1024)
#define ALI_SW_RX_DESC_NUM		(64)
#define ALI_SW_TX_DESC_NUM		(64)
#define ALI_SW_BUF_SZ				1536
#define ALI_SW_DESC_SZ			16

#define MAX_CONTEXT_DESC_ID	8

#define ALI_SW_MSS				1514

//Setup frame size.
#define SETUP_FRAME_SZ			192
#define IMPERFECT_PA_OFF		156
//Hash
#define ALI_SW_HASH_TABLE_LEN	512		//64 bytes
//#define ALI_SW_HASH_BITS			0x01ff	//=512-1


#define ALI_SW_REGS_RANGE			0x94
#define ALI_SW_REGS_VER			1

#define HIBYTE(word)  ((u8)(((u16)(word))>>8))
#define LOBYTE(word)  ((u8)(((u16)(word))&0x00FFU))
#define HIWORD(dword) ((u16)(((u32)(dword))>>16))
#define LOWORD(dword) ((u16)(((u32)(dword))&0x0000FFFFUL))

typedef volatile struct p0_tx_context_desc{
	u32 VlanTag: 16; 					//bit 0~15.Vlan tag
	u32 L3Type: 1; 						//bit 16.  indicate L3 header type.
	u32 L4Type: 2;						//bit 17~18. indicate L4 header type. //question, what's the l3type and l4type address in memory???
	u32 L4HeaderLen: 8;				//bit 19~26. indicate L4 header length.
	u32 ContextIndex: 3;				//bit 27~29. indicate Index of Context Descriptor.
	u32 ContextData: 1;					//bit 30. indicate Descriptor type.
	u32 OWN: 1;						//bit 31. 0->Owned by SW, 1->Owned by HW.		

	u32 L3HeaderLen: 9;				//bit 0~8. indicate L4 header length.		
	u32 L2HeaderLen: 7;				//bit 9~15. indicate L2 header length.		
	u32 MSS: 16;						//bit 16~31. Maximum Segment Size

	u32 Reserved1;
	u32 Reserved2;
}__attribute__((packed)) *p_tx_ctx_desc, tx_ctx_desc;


typedef volatile struct p0_tx_data_desc{
	u32 SegmentNum: 16; 			//bit 15~0.Maximum Frame Length(Without CRC)
	u32 ForwardPorts:3;			//bit 16~18, indicate which ports a packet should be sent to.
	u32 TX_PRIOR:2;				//bit 19~20. indicates packet priority
	u32 TOE_UFO_En: 1;			//bit 21.IP header offset.
	u32 VlanEn: 1;					//bit 22.
	u32 CoeEn: 1;					//bit 23. enable Tx COE.
	u32 TX_PRIOR_EN: 1;			//bit 24. indicate whether it should use CPU specified priority.
	u32 FS: 1;						//bit 25. indicate a First Segment Descriptor.
	u32 EOR: 1;					//bit 26. indicate End Of descriptor Ring.
	u32 ContextIndex: 3;			//bit 27~29. indicate Index of Context Descriptor.
	u32 ContextData: 1;			//bit 30. indicate Descriptor type.
	u32 OWN: 1;					//bit 31. 0->Owned by SW, 1->Owned by HW.		

	dma_addr_t pkt_buf_dma;
	
	u16 seg_len;
	u16 tot_len;
	
	u32 FragmentID;
}__attribute__((packed)) *p_tx_data_desc, tx_data_desc;

typedef volatile struct p0_tx_status_desc{
	u32 Reserved30_0: 31;		//bit 1~0.Reserved.
	u32 OWN: 1;				//bit 31. 0->Owned by SW, 1->Owned by HW.

	u32 Reserved1;
	u32 Reserved2;
	u32 Reserved3;
}__attribute__((packed)) *p_tx_stat_desc, tx_stat_desc;

typedef union p0_tx_desc{

	tx_ctx_desc					ContextDescriptor;
	tx_data_desc					DataDescriptor;
	tx_stat_desc					SatusDescriptor;
	
} *p_tx_desc;

//head structure//-->question: a lot of whole difference.
typedef struct packet_head{
	u32 GARP:1;
	u32 GVRP_MVRP:1;
	u32 GMRP_MMRP:1;
	u32 IGMP_MLD:1;
	u32 BPDU:1;
	u32 SNAP:1;
	u32 PPPoE: 1;				//PPPoE packet.
	u32 L2FrameType : 1;			//Frame Type.
	u32 IPDChksum: 1;			//TCP Checksum is Okay.
	u32 IPChksum: 1;				//IP Checksum is Okay.	
	u32 UDPpkt:1;					//UDP packet
	u32 TCPpkt:1;					//TCP packet
	u32 IPFrag: 1;					//IP Fragment.
	u32 IP6Frame: 1;				//IP Frame.
	u32 IPFrame: 1;				//IPv6 Frame.
	u32 MulFrame:1;				//it's a multicase frame.
	u32 BroFrame:1;				//it's a broadcast frame.
	u32 UniFrame:1;				//it's a unicast frame.
	u32 PortFrame:2;
	u32 Reserved20_31:12;

}__attribute__((packed)) *ppacket_head;

typedef struct p0_rx_desc{
	struct packet_head pkt_hdr;
	u16 l4_chs;
	u16 l3_chs;	//question, should it be big endian?
	u16 vlan_tag;	
	u16 PacketLength;
	dma_addr_t pkt_buf_dma;
}__attribute__((packed)) *p_rx_desc;

typedef struct p0_txrx_stats {
	u32   rx_from_port[4];//MAX_PORT_NUM
	u32	rx_uc;				
	u32	rx_bc;				
	u32	rx_mc;				
	u32	rx_ip;										
	u32   rx_ipv6;
	u32	rx_frag;	
	u32	rx_tcp;	
	u32	rx_udp;	
	u32	rx_8023;
	u32	rx_pppoe;
	u32 	rx_snap;
	u32 	rx_bpdu;
	u32 	rx_igmp_mld;
	u32 	rx_gmrp_mmrp;
	u32 	rx_gvrp_mvrp;
	u32 	rx_garp;
	u32	rx_ip_chksum_err_from_desc;	
	u32	rx_ip_data_chksum_err_from_desc;	//count in descriptor or register? 
	u32	rx_ip_chksum_err_from_reg;	
	u32	rx_ip_data_chksum_err_from_reg;		//count in descriptor or register? 
}*p_p0_txrx_stats;

#define ALI_SW_NUM_STATS 36
//#define ALI_SW_WORKQUEUE_TEST
#define ALI_SW_SINGLE_THREAD_WQ

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/****************************************************************************
	p0_private -- used only for P0's TX/RX, the private struct of net_device.
	p0_private contains net_ali_sw.
	net_ali_sw -- implement switch's abilities, things like STP, MC snooping.
*****************************************************************************/
#define IPV4_VERSION_VAL		0x40
#define IPV6_VERSION_VAL		0x60
#define IPV4_PROTOCAL_OFFSET	9
#define IPV6_NEXTHEAD_OFFSET	6
#define IPV6_HOPBYHOP_NEXTHEAD	40
#define ICMP_PROTOCAL_VAL		0x01

#define PACKET_STP			0
#define PACKET_MCS   		1
#define PACKET_OTHERS		99

#define FILTER_PERFECT_MODE        1
#define FILTER_ALLMULTI_MODE       2

typedef struct _sw_skb_info_t {
	u16 start;  /* first descript */
	u16 first;  /* first data descript */
	u16 cnt;
	struct sk_buff *skb;
} sw_skb_info_t;


typedef struct p0_private{
	struct eth_device 			*dev;
	u32                    io_base;
	bool 					acquired_isr;	
	u32 						irq_num;
	u32 						p0_isr;
	u32 						fabric_isr;
	
	u32                         msg_enable;

	struct p0_txrx_stats 		mac_stats;

	u16 						rx_wptr, rx_bptr;
	u16 						tx_wptr;
	
	u8 						*setup_buf;
	u32                     setup_buf_dma;
	
	p_rx_desc 				rx_desc;				
	u32                     rx_desc_dma;		//Rx Start Address of Descriptors.
    u8 * rx_buf[ALI_SW_RX_DESC_NUM];
    

	p_tx_desc 				tx_desc;
	u32                     tx_desc_dma;
    u8 * tx_buf; 

	struct net_ali_sw 			*p_switch;
	volatile u16 avail_desc_num;
    int filter_mode;
} * p_p0_private;

extern u32 ali_sw_base;

typedef enum
{
	ETH_PHY_MODE_RMII,
	ETH_PHY_MODE_MII,
	ETH_PHY_MODE_RGMII,	
	ETH_PHY_MODE_GMII,
}eth_phy_mode;

struct sw_reg_io
{
    unsigned char   mux;
    unsigned short  addr;
    unsigned long   val;
};

#define SW_PORT0_REGS_RANGE 		0x3c
#define SW_PORT123_REGS_RANGE 	0x5c
#define SW_FABRIC_REGS_RANGE 		0x1ff
#define SW_ALR_TOTAL_ENTRY_NUM	68
#define SW_ALR_SHOW_ALL_ENTRY	255
#define SW_VLUT_TOTAL_ENTRY_NUM	16

#define SW_R8(reg)			(*(volatile u8 *)(ali_sw_base + (reg)))
#define SW_R16(reg)		(*(volatile u16 *)(ali_sw_base + (reg)))
#define SW_R32(reg)		(*(volatile u32 *)(ali_sw_base + (reg)))

#define SW_W8(reg, val)		(*(volatile u8 *)(ali_sw_base + (reg))) = (u8)(val)	
#define SW_W16(reg, val)	(*(volatile u16 *)(ali_sw_base + (reg))) = (u16)(val)	
#define SW_W32(reg, val)	(*(volatile u32 *)(ali_sw_base + (reg))) = (u32)(val)

#endif //__ETHERNET_ALI_SW__H

