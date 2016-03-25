/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: ethernet_mac.h
 *
 *  Description: head file for ethernet mac device management.
 *
 *  History:
 *      Date		Author		Version		Comment
 *      ====		======		=======	=======
 *  1.  2008.02.19  Mao Feng					Original MAC
 *  2.  2008.02.25	Mao Feng		1.0			Add Mii interface management
 *  3.  2009.04.23  						Cost Down MAC
 *
 ****************************************************************************/

#ifndef _ETHERNET_MAC_H_
#define _ETHERNET_MAC_H_

#include "basic_types.h"


#include "net.h"
#include "ethtool.h"
#include "if.h"
#include "net_dev.h"

/* #if !defined(RET_CODE)
typedef int RET_CODE;
#endif
#if !defined(BOOL)
typedef int BOOL;
#endif */
#if !defined(OSAL_ID)
typedef int OSAL_ID;
#endif

#define DEBUG_MAC	0
#if DEBUG_MAC
#define MAC_PRINTF   printf  //printf //libc_printf
#else
#define MAC_PRINTF(...)	do{}while(0)
#endif


//en HW MDIO
#define HW_MDIO


#define MAC_BaseAddr		0xB802C000 //0xb8016000
#define MAC_InterruptVector	(32+8+6) //27//(28+8)

#define MAC_AddrLength		6


//#define PHY_SMSC

//ethernet frame size.
#define		MAX_ETHERNET_PACKET_SIZE	1518	//Head(14)+VlanType(2)+VlanTag(2)+Data(1500), without crc(4)
#define 		TOE2_MSS						1514	//max eth packet size, without CRC
#define		MIN_ETHERNET_FRAME_SIZE		64
#define		MAC_HEADER_SIZE				14
#define		MAC_VLAN_SIZE					4
#define		MAC_CRC_SIZE					4



//Rx ring buffer size.
#define		RING_BUF_UNIT			0x2000		// 8K
#define		RING_BUF_SIZE			0x200000		// 2m
#define		RING_BUF_PAD			1024*2

//Max tx packets descriptor number. 
#define		NUM_OF_DESC	4 

#define TOE2_RX_DESC_NUM		64
#define TOE2_TX_DESC_NUM		64
#define TOE2_DESC_SZ			16

//Setup frame size.
#define		SETUP_FRAME_SIZE		192
#define		IMPERFECT_PA_OFFSET	156       /* Bytes */

//Max packets number in rx buffer. 
#define		NUM_OF_PACKETS		0x40	//64	

#define		RX_BUF_SIZE			1536
#define		TX_BUF_SIZE			RX_BUF_SIZE

#define virtual_to_dma(addr)	((UINT32) addr & 0x0fffffff)

// recycling use.
#define	NEXT_TX_DESC(i)		(i) == (NUM_OF_DESC - 1) ? 0 : (i + 1)


#define	HASH_TABLE_LEN		512		/* Bits */
#define	HASH_BITS				0x01ff	/* 9 LS bits */

#define	CRCPOLY_LE				0xedb88320		//crc poly


// multicast list size                          
#define	MAX_MC_LIST		16
#define	MAX_MC_ADDR	6

//local bus r/w access.
#define readb(addr) (*(volatile UINT8 *)(addr))
#define readw(addr) (*(volatile UINT16 *)(addr))
#define readl(addr) (*(volatile UINT32 *)(addr))

#define writeb(b,addr) (*(volatile UINT8 *)(addr)) = (b)
#define writew(b,addr) (*(volatile UINT16 *)(addr)) = (b)
#define writel(b,addr) (*(volatile UINT32 *)(addr)) = (b)


#define MAC_R8(reg)			readb(((UINT32)base_addr) + (reg))
#define MAC_R16(reg)			readw(((UINT32)base_addr) + (reg))
#define MAC_R32(reg)			readl(((UINT32)base_addr) + (reg))

#define MAC_W8(reg, val8)	writeb((val8), base_addr + (reg))
#define MAC_W16(reg, val16)	writew((val16), base_addr + (reg))
#define MAC_W32(reg, val32)	writel((val32), base_addr + (reg))

#define barrier()			asm volatile("": : :"memory")

//typedef unsigned int UINT32;
//typedef unsigned short u16;

typedef struct teo2_tx_desc{
	union {
		struct {
			UINT32 Reserved1_0: 2;			//bit 1~0.Reserved.
			UINT32 HF: 1;					//bit 2. Heartbeat Fail Error.
			UINT32 FifoUnderrun: 1; 			//bit 3. Fifo Underrun Error.
			UINT32 ColCnt: 4;					//bit 7~4. Collision Count.
			UINT32 ExCol: 1;					//bit 8. Excessive Collsion.
			UINT32 LateCol: 1;				//bit 9. Late Collsion.
			UINT32 NoCarrier: 1;				//bit 10. No Carrier.
			UINT32 LossOfCarrier: 1;			//bit 11. Loss Of Carrier..
			UINT32 ES: 1;						//bit 12. Error Sumary.
			UINT32 Reserved27_13: 15;		//bit 27~13. Reserved.
			UINT32 LS: 1;						//bit 28. indicate a Last Segment Descriptor.
			UINT32 FS: 1;						//bit 29. indicate a First Segment Descriptor.
			UINT32 Reserved30: 1;			//bit 30. indicate End Of descriptor Ring.
			UINT32 OWN: 1;					//bit 31. 0->Owned by SW, 1->Owned by HW.
		} sw;
		
		struct {
			UINT32 Mfl: 13; 					//bit 12~0.Maximum Frame Length(Without CRC)
			UINT32 VlanEn: 1;					//bit 13.
			UINT32 IpStartOff: 8;				//bit 21~14.IP header offset.
			UINT32 UdpPkt: 1;				//bit 22. indicate UDP packet type.
			UINT32 TcpPkt: 1;					//bit 23. indicate TCP packet type.
			UINT32 CoeEn: 1;					//bit 24. enable Tx COE.
			UINT32 SetupPkt: 1; 				//bit 25. indicate Setup packet type.
			UINT32 FilteringMode: 2;			//bit 27~26. filtering mode for a setup packet.
			UINT32 LS: 1;						//bit 28. indicate a Last Segment Descriptor.
			UINT32 FS: 1;						//bit 29. indicate a First Segment Descriptor.
			UINT32 EOR: 1;					//bit 30. indicate End Of descriptor Ring.
			UINT32 OWN: 1;					//bit 31. 0->Owned by SW, 1->Owned by HW.		
		} hw;
	} tx_sts;

	dma_addr_t pkt_buf_dma;
	u16 seg_len;
	u16 tot_len;
	u16 vlan_tag;
	u16 seg_num;			
}*ptx_desc;

//head structure
typedef struct tag_packet_head{
	UINT16  PacketLength: 12;
	
	UINT16	TCPChksum: 1;			//TCP Checksum is Okay.
	UINT16	IPChksum: 1;				//IP Checksum is Okay.	
	UINT16	PPPoE: 1;				//PPPoE packet.
	UINT16	VLAN: 1;					//VLAN Fram.
	
	UINT16	WatchdogTimeout: 1;		//Receive Watchdog Time-out.
	UINT16	FrameType : 1;			//Frame Type.
	UINT16	PhysicalLayerError: 1;		//Physical Layer Error.
	UINT16	FifoOverflow: 1;			//FIFO Overflow Error. 
	UINT16	FAE : 1;					//Alignment Error.
	UINT16	CRC: 1;					//CRC error.
	UINT16	LateCollision: 1;			//Late Collision Seen.
	UINT16	Long : 1;					//Excessive Frame Length.
	UINT16	Runt: 1;					//Runt Frame.
	UINT16	ES: 1;					//Error Summary.
	UINT16	BF : 1;					//Broadcast Frame.
	UINT16  PF : 1; 					//Physical Frame.
	UINT16	MF : 1;					//Multicast Frame.
	UINT16	Dribble: 1;				//Dribble Error.
	UINT16	IPFrag: 1;				//IP Fragment.
	UINT16	IPFrame: 1;				//IP Frame.
	
}packet_head, *ppacket_head;

typedef struct teo2_rx_desc{
	packet_head pkt_hdr;
	u16 l3_chs;
	u16 l4_chs;
	u16 vlan_tag;
	u16 reserved: 15;
	u16 EOR: 1;
	dma_addr_t pkt_buf_dma;
}*prx_desc;


typedef struct tag_mac_adapter{

	struct net_device *net_dev;

	UINT32 io_base;
	UINT8 mac_addr[MAC_AddrLength];
	
	UINT8 *setup_frame_buf;
	UINT32 setup_frame_dma;

	prx_desc rx_desc;
	dma_addr_t rx_desc_dma;

	ptx_desc tx_desc;
	dma_addr_t tx_desc_dma;

	u16 rx_wptr, rx_bptr;
	u16 tx_wptr;

	UINT32 interrupt_mask;
	UINT16 interrupt_vector;
	UINT32 cur_isr;

	//UINT32 rx_early_fifo_threshold;
	//UINT32 tx_early_fifo_threshold;

	net_phy_mode phy_mode;
	UINT32 link_speed;
	BOOL duplex_mode;
	
	BOOL pause_frame_rx;
	BOOL pause_frame_tx;

	//link partner ability.
	UINT16 link_partner;

	//link status change. 
	BOOL phy_reset;
	BOOL auto_n_completed;
	BOOL link_established;
	BOOL transmit_okay;

	struct net_device_stats net_stats;
	mac_device_stats mac_stats;

	struct mii_if_info mii;

	UINT32 multicast_flags;						/* Standard interface flags */
	UINT16 multicast_counter;
	UINT8 multicast_list[MAX_MC_LIST][MAX_MC_ADDR];

	//OSAL_ID task_id;
	//OSAL_ID int_sem;

	UINT8 iphdr_off;
	
	UINT8 accelerator;

	UINT8 isr_registered;
}mac_adapter, *pmac_adapter;


RET_CODE eth_mac_detach(struct net_device *dev);

RET_CODE eth_mac_open(struct net_device*dev, void (*callback) (UINT32, UINT32));

RET_CODE eth_mac_close(struct net_device *dev);

RET_CODE eth_mac_ioctl(struct net_device *dev, UINT32 _data, UINT32 cmd);

RET_CODE eth_mac_send_one_packet(struct net_device *dev, void *buf, UINT16 len);

RET_CODE 
eth_mac_get_info(struct net_device *dev, UINT32 info_type, void *info_buf);

RET_CODE 
eth_mac_set_info(struct net_device *dev, UINT32 info_type, void *info_buf);

struct net_device_stats * eth_mac_get_stats(struct net_device *dev);

RET_CODE eth_mac_set_rx_mode(struct net_device *dev);

#endif //_ETHERNET_MAC_H_
