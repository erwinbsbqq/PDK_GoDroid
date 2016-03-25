#include <linux/netdevice.h>
#include <linux/random.h>
#include <linux/if_vlan.h>
#include <linux/mii.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/crc32.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include <asm/dma.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <linux/workqueue.h>
#include <asm/mach-ali/m6303.h>
#include <asm/mach-ali/m36_gpio.h>
#include <asm/gpio.h>

#include "eth_reg.h"
#include "eth_toe2.h"
#include "eth_toe2_util.h"

#ifdef UTILITY_SUPPORT

#define SWAP(value) (((value<<8)&0xFF00)|((value>>8)&0x00FF))

/****************************************************
#define TOE2U_DRV_NAME "ALi Ethernet TOE2 Utility"
#define TOE2U_DRV_VER "Ver 0.1"
 ****************************************************/
#define  MAC_FRAME_SETUP    0
#define  MAC_FRAME_PAUSE    1
#define  MAC_FRAME_8023     2
#define  MAC_FRAME_ETHERII  3

#define  MAC_FRAME_ARP      4
#define  MAC_FRAME_IP_ICMP  5
#define  MAC_FRAME_IP_IGMP  6
#define  MAC_FRAME_IP_UDP   7
#define  MAC_FRAME_IP_TCP   8

#define  MAC_FRAME_PPPoE_IP_ICMP  9
#define  MAC_FRAME_PPPoE_IP_IGMP  10
#define  MAC_FRAME_PPPoE_IP_UDP   11
#define  MAC_FRAME_PPPoE_IP_TCP   12

#define  MAC_FILTER_HASH      0
#define  MAC_FILTER_HASHONLY  1
#define  MAC_FILTER_INVERSE   2
#define  MAC_FILTER_PERFECT   3

/******temp*******/
#define MAX_ETH_PKT_LEN	    1536
#define MIN_ETH_PKT_LEN	    64
//#define ETH_HLEN		    14
//#define MAC_VLAN_SIZE		4
//#define MAC_CRC_SIZE		4
//#define ARP_HDR_LEN       28
//#define ICMP_HDR_LEN		12
//#define IGMP_HDR_LEN		8
//#define TCP_HDR_LEN       20
//#define UDP_HDR_LEN       8
#define PPPOE_HDR_LEN		8
//#define MIN_IP_HDR_LEN	20
//#define MAX_IP_HDR_LEN	64
//#define MAX_MFL_LEN	    1536
//Eth Type definitions.

//#define ETH_P_ARP		    0x0806
//#define ETH_P_IP			0x0800
//#define ETH_P_PPP_DISC	0x8863  /* PPP Over Ethernet Discovery Stage */
//#define ETH_P_PPP_SES		0x8864  /* PPP Over Ethernet Session Stage */
//#define ETH_P_8021Q		0x8100

#define ETH_TYPE_OFF        (6+6)
#define ETH_PPPOE_PRO_OFF   (6+6+2+6)
#define ETH_PPPOE_PRO_LEN   2

#define ETH_IP_OFF          (6+6+2)
#define PPPOE_IP            0x21    /* Internet Protocol */

#define IP_LEN_OFF		0
#define IP_TOTLEN_OFF	2
#define IP_PRO_OFF		8
#define IP_IPSRC_OFF	12
#define IP_IPDES_OFF	16

//#define IPPROTO_ICMP    1
//#define IPPROTO_IGMP    2
//#define IPPROTO_UDP     17
//#define IPPROTO_UDPLITE 136
//#define IPPROTO_TCP     6
/*************/

extern ptoe2_private g_ptoe2;
MAC_ADAPTER util_mac_adapter;
static int m_bRxThreadStart;
static int m_bRxStopThread;
static int m_bTxThreadStart;
static int m_bTxStopThread;

//utility driver init source
#define MAC_RX_DESC_NUM 64
#define MAC_TX_DESC_NUM 64
static unsigned char util_rxbuf_addr[TOE2_BUF_SZ*MAC_RX_DESC_NUM+0x20];
static unsigned char util_txbuf_addr[(MAX_TOE2_PKT_LEN + 1024)*MAC_TX_DESC_NUM+0x20];

//board config
static unsigned int ali_phy_addr = 1;
static unsigned int ali_physet_gpio;

//rx/tx params
static unsigned char MacSrcAddr[6];
static unsigned char MacDstAddr[6];

static MAC_TX_PARA ali_tx_para;
static struct teo2_tx_desc ali_desc_hw;
static unsigned char ali_vlan_chk[TOE2_BUF_SZ];

static unsigned short ali_desc_size[MAC_TX_DESC_NUM];
static unsigned char ali_rx_buf[MAC_RX_DESC_NUM][TOE2_BUF_SZ];

static unsigned char MacTxBuf[TOE2_BUF_SZ];      //1536
static unsigned char ToeTxBuf[MAX_TOE2_PKT_LEN]; //64*1024

//crc
static unsigned short ChsAcc, ChsOff;
static unsigned int crc32_table[256];
#define CRC32_POLY 0x04c11db7
#define ReverseBit(data) (((data<<7)&0x80)|((data<<5)&0x40)|((data<<3)&0x20)|((data<<1)&0x10)|\
		((data>>1)&0x08)|((data>>3)&0x04)|((data>>5)&0x02)|((data>>7)&0x01))
#define ReverseByte(data) ((data&0xff)<<8)|((data&0xff00)>>8)

static void init_crc32(void)
{
	int i, j;
	unsigned long c;
	unsigned char *p=(unsigned char *)&c, *p1;
	unsigned char k;

	for (i = 0; i < 256; ++i) 
	{
		k = ReverseBit((unsigned char)i);
		for (c = ((unsigned long)k) << 24, j = 8; j > 0; --j)
			c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
		p1 = (unsigned char *)(&crc32_table[i]);

		p1[0] = ReverseBit(p[3]);
		p1[1] = ReverseBit(p[2]);
		p1[2] = ReverseBit(p[1]);
		p1[3] = ReverseBit(p[0]);
	}
}

static unsigned int ALiCrc32(unsigned char *buf, int len)
{
	unsigned char *p;
	unsigned long  crc;

	crc = 0xffffffff;       /* preload shift register, per CRC-32 spec */

	for (p = buf; len > 0; ++p, --len)
	{
		crc = crc32_table[ (crc ^ *p) & 0xff] ^ (crc >> 8);
	}
	return ~crc;    /* transmit complement, per CRC-32 spec */
}

static void ALiCrcChk(PMAC_ADAPTER pAdapter, unsigned char *buf, int len)
{
	UINT32 Crc, CrcChk, Tmp32;
	UINT32 k;

	UINT16 DescWritePtr, DescReadPtr;
	UINT32 ba = pAdapter->BaseAddr;

	//wait untill no sending pkts. 
	do
	{
		DescReadPtr = TOE2_R16(TxRingDesRPtr);
		DescWritePtr = pAdapter->TxDescWPtr;

	} while(DescReadPtr != DescWritePtr);

	if ((pAdapter->ToeTxEn) && (ChsOff != 0))
		*(UINT16 *)(buf + ChsOff) =  ChsAcc; 

	Crc = ALiCrc32(buf, len);
	CrcChk = 0;

	for (k = 0; k < 32; k++)
	{
		Tmp32 = Crc >> k;
		Tmp32 &= 0x1;
		CrcChk |= (Tmp32<<(31 - k));
	}

	CrcChk = ~CrcChk;

	TOE2_W32(0x2C/*CRCCHK*/, CrcChk);

	if ((pAdapter->ToeTxEn) && (ChsOff != 0))
		*(UINT16 *)(buf + ChsOff) =  0; 
	//TOE2_UTILITY_TRACE("chs:%08x,offset:%d, crc:%08x", ChsAcc, ChsOff, CrcChk);
}

static UINT16 standard_chksum(void *dataptr, UINT16 len)
{
	UINT32 acc;
	UINT16 src;
	UINT8 *octetptr;

	acc = 0;
	/* dataptr may be at odd or even addresses */
	octetptr = (UINT8*)dataptr;
	while (len > 1)
	{
		/* declare first octet as most significant
		   thus assume network order, ignoring host order */
		src = (*octetptr) << 8;
		octetptr++;
		/* declare second octet as least significant */
		src |= (*octetptr);
		octetptr++;
		acc += src;
		len -= 2;
	}
	if (len > 0)
	{
		/* accumulate remaining octet */
		src = (*octetptr) << 8;
		acc += src;
	}
	/* add deferred carry bits */
	acc = (acc >> 16) + (acc & 0x0000ffffUL);
	if ((acc & 0xffff0000) != 0)
	{
		acc = (acc >> 16) + (acc & 0x0000ffffUL);
	}
	/* This maybe a little confusing: reorder sum using toe_htons()
	   instead of ntohs() since it has a little less call overhead.
	   The caller must invert bits for Internet sum ! */
	return htons((UINT16)acc);
}

static UINT32 standard_chksum_2(void *dataptr, UINT16 len)
{
	UINT32 acc;
	UINT16 src;
	UINT8 *octetptr;

	acc = 0;
	/* dataptr may be at odd or even addresses */
	octetptr = (UINT8*)dataptr;
	while (len > 1)
	{
		/* declare first octet as most significant
		   thus assume network order, ignoring host order */
		src = (*octetptr) << 8;
		octetptr++;
		/* declare second octet as least significant */
		src |= (*octetptr);
		octetptr++;
		acc += src;
		len -= 2;
	}

	if (len > 0)
	{
		/* accumulate remaining octet */
		src = (*octetptr) << 8;
		acc += src;
	}

	while ((acc >> 16) != 0)
	{
		acc = (acc & 0xffff) + (acc >> 16);
	}

	return (acc);
}

static unsigned short inet_chksum(void *dataptr, UINT16 len)
{
	UINT32 acc;

	acc = standard_chksum(dataptr, len);
	while ((acc >> 16) != 0)
	{
		acc = (acc & 0xffff) + (acc >> 16);
	}
	return (UINT16)(acc & 0xffff);
}

//copy built-in memory from host memory. 
static int ALICopyFromMem(unsigned char *to, UINT32 from, UINT32 len)
{
	int Status = TRUE;

	//dump_out_mem(from, to, len);
	memcpy(to, from, len);
	return Status;
}

//copy built-in memory to host memory. 
static int ALICopyToMem(UINT32 to, unsigned char *from, UINT32 len)
{
	int Status = TRUE;

	//load_in_mem(to, from, len);
	memcpy(to, from, len);
	return Status;
}

static int MacAllocateResource(PMAC_ADAPTER pAdapter)
{
	int i;

	UINT32 RxDescBuffer;
	UINT32 TxDescBuffer;

	TOE2_UTILITY_TRACE("UTILITY Memory Allocation:\n");			

	RxDescBuffer = (UINT32)(&util_rxbuf_addr[0]);
	TxDescBuffer = (UINT32)(&util_txbuf_addr[0]);
	TOE2_UTILITY_TRACE(" 1: rx: %x", RxDescBuffer);
	TOE2_UTILITY_TRACE(" 1: tx: %x\n", TxDescBuffer);

	RxDescBuffer = (RxDescBuffer+0x1f)&0xFFFFFFE0;
	TxDescBuffer = (TxDescBuffer+0x1f)&0xFFFFFFE0;
	TOE2_UTILITY_TRACE(" 2: rx: %x", RxDescBuffer);
	TOE2_UTILITY_TRACE(" 2: tx: %x\n", TxDescBuffer);

	for (i = 0; i < MAC_RX_DESC_NUM; i++)
	{
		pAdapter->RxDescBuf[i] = RxDescBuffer + TOE2_BUF_SZ*i;
		pAdapter->TxDescBuf[i] = TxDescBuffer + MAX_TOE2_PKT_LEN*i;// + i;
		//TOE2_UTILITY_TRACE(" sub: rx: %x", pAdapter->RxDescBuf[i]);
	    //TOE2_UTILITY_TRACE(" sub: tx: %x", pAdapter->TxDescBuf[i]);

		pAdapter->RxDescBuf_DMA[i] = dma_map_single((struct device *)NULL, pAdapter->RxDescBuf[i], TOE2_BUF_SZ, DMA_FROM_DEVICE);
		pAdapter->TxDescBuf_DMA[i] = dma_map_single((struct device *)NULL, pAdapter->TxDescBuf[i], MAX_TOE2_PKT_LEN, DMA_FROM_DEVICE);
		//TOE2_UTILITY_TRACE(" dma: rx: %x", pAdapter->RxDescBuf_DMA[i]);
	    //TOE2_UTILITY_TRACE(" dma: tx: %x\n", pAdapter->TxDescBuf_DMA[i]);
	}

	pAdapter->RxBuffer = (unsigned int)&ali_rx_buf[0][0];
	TOE2_UTILITY_TRACE(" RxBuffer: tx: %x\n", pAdapter->RxBuffer);
	return 0;
}

static void MacInitRing(PMAC_ADAPTER pAdapter)
{
	TOE2_UTILITY_TRACE("MacInitRing()");

	pAdapter->pSetupBuf = g_ptoe2->setup_buf;
	pAdapter->pSetupBuf_DMA= g_ptoe2->setup_buf_dma;
	pAdapter->pRxDesc = g_ptoe2->rx_desc;
	pAdapter->pRxDesc_DMA = g_ptoe2->rx_desc_dma;
	pAdapter->pTxDesc = g_ptoe2->tx_desc;
	pAdapter->pTxDesc_DMA = g_ptoe2->tx_desc_dma;

	TOE2_UTILITY_TRACE("MacInitRing pTxDesc addr:     0x%x", pAdapter->pTxDesc);
	TOE2_UTILITY_TRACE("MacInitRing pTxDesc_DMA addr: 0x%x\n", pAdapter->pTxDesc_DMA);

	pAdapter->RxDescWPtr = MAC_RX_DESC_NUM -1;
	pAdapter->RxBufRPtr= 0;
	pAdapter->TxDescWPtr= 0;	

	pAdapter->InterruptMask = TOE2_INTERRUPT_MASK; 
	pAdapter->CurrentISR = 0;
}

#define	MdioDelay(MdioAddr) TOE2_R32(MdioAddr)

static void MacChipReset(UINT32 ba)
{
	UINT8 tmp_u8;
	TOE2_UTILITY_TRACE("TOE2: %s()=>O", __FUNCTION__);
	TOE2_W8(SCR, SCRReset|TOE2_R8(SCR));	
	do
	{
		TOE2_UTILITY_TRACE("->");	
		tmp_u8 = TOE2_R8(SCR);
	} while(tmp_u8 & SCRReset);
	TOE2_UTILITY_TRACE("K!\n");
}

static void MdioSync(PMAC_ADAPTER pAdapter)
{
	int i;
	UINT32 ba = pAdapter->BaseAddr;
	for (i = 32; i >= 0; i--)
	{
		TOE2_W32(MiiMR1, MiiMdioWrite1|MiiMdioEn);
		MdioDelay(MiiMR1);
		TOE2_W32(MiiMR1, MiiMdioWrite1|MiiMdioClk|MiiMdioEn);
		MdioDelay(MiiMR1);
	}
	//TOE2_UTILITY_TRACE("MdioSync: end\n");
}

static UINT32 MdioRead(PMAC_ADAPTER pAdapter, UINT32 Phyaddr, UINT32 RegAddr)
{
	UINT16 tmp_16;
	UINT32 tmp_32;
	UINT32 addr;
	UINT32 cnt = 0;
	UINT32 retval = 0;

	int i,	DataVal = 0;
	UINT32 mii_cmd = (0xf6 << 10) | (Phyaddr << 5) | RegAddr;
	UINT32 Tmplong, Cmdlong;
	UINT32 ba = pAdapter->BaseAddr;

	if (pAdapter->HwMdioEn == TRUE)
	{
		addr = ((Phyaddr << MiiPhyAddrOff) & MiiPhyAddrMask)|((RegAddr << MiiRegAddrOff) & MiiRegAddrMask);

		tmp_32 = TOE2_R32(MiiMR2);
		tmp_32 &= ~(MiiPhyAddrMask|MiiRegAddrMask|MiiOpMask);

		TOE2_W32(MiiMR2, (tmp_32|addr|MiiOpRead|MiiTransStart));

		do
		{
			cnt ++;
			tmp_32 = TOE2_R32(MiiMR2);
			mdelay(1);
		} while(tmp_32&MiiTransStart);

		//TOE2_UTILITY_TRACE("MdioRead: cnt(%d)\n", cnt);
		tmp_16 = TOE2_R16(MdioR);

		return (UINT32)tmp_16;
	}
	else
	{
		//TOE2_UTILITY_TRACE("MdioRead: start.\n");
		MdioSync(pAdapter);

		/* Shift the read command bits out. */
		for (i = 15; i >= 0; i--)
		{
			DataVal = (mii_cmd & (1 << i)) ? MiiMdioOut : 0;

			TOE2_W32(MiiMR1, MiiMdioDir|DataVal|MiiMdioEn);
			Tmplong = MdioDelay(MiiMR1);
			TOE2_W32(MiiMR1, MiiMdioDir|DataVal|MiiMdioClk|MiiMdioEn);
			Tmplong = MdioDelay(MiiMR1);
		}

		/* Read the two transition, 16 data, and wire-idle bits. */
		for (i = 19; i > 0; i--)
		{
			TOE2_W32(MiiMR1, 0|MiiMdioEn);
			Tmplong = MdioDelay(MiiMR1);

			Cmdlong = TOE2_R32(MiiMR1);
			retval = (retval << 1) | ((Cmdlong & MiiMdioIn) ? 1 : 0);

			TOE2_W32(MiiMR1, MiiMdioClk|MiiMdioEn);
			Tmplong = MdioDelay(MiiMR1);
		}
		return (retval >> 1) & 0xffff;
	}
}

static void MdioWrite(PMAC_ADAPTER pAdapter, UINT32 Phyaddr, UINT32 RegAddr, UINT32 Value)
{
	UINT32 tmp_32;
	UINT32 addr;
	UINT32 cnt = 0;
	UINT32 mii_cmd = (0x5002 << 16) | (Phyaddr << 23) | (RegAddr << 18) | Value;
	UINT32 i;
	UINT32 Tmplong;
	UINT32 ba = pAdapter->BaseAddr;

	if (pAdapter->HwMdioEn == TRUE)
	{		
		TOE2_W16(MdioW, (UINT16)Value);

		addr = ((Phyaddr << MiiPhyAddrOff) & MiiPhyAddrMask)|((RegAddr << MiiRegAddrOff) & MiiRegAddrMask);

		tmp_32 = TOE2_R32(MiiMR2);
		tmp_32 &= ~(MiiPhyAddrMask|MiiRegAddrMask|MiiOpMask);

		TOE2_W32(MiiMR2, (tmp_32|addr|MiiOpWrite|MiiTransStart));

		do
		{
			cnt ++;
			tmp_32 = TOE2_R32(MiiMR2);
			mdelay(1);
		} while(tmp_32&MiiTransStart);

		//TOE2_UTILITY_TRACE("MdioWrite: cnt(%d)\n", cnt);
	}
	else
	{
		//TOE2_UTILITY_TRACE("MdioWrite: start.\n");
		MdioSync(pAdapter);

		/* Shift the command bits out. */
		for (i = 31; i >= 0; i--)
		{
			int dataval = (mii_cmd & (1 << i)) ? MiiMdioWrite1 : MiiMdioWrite0;
			TOE2_W32(MiiMR1, dataval|MiiMdioEn);
			Tmplong = MdioDelay(MiiMR1);
			TOE2_W32(MiiMR1, dataval|MiiMdioClk|MiiMdioEn);
			Tmplong = MdioDelay(MiiMR1);
		}

		/* Clear out extra bits. */
		for (i = 2; i > 0; i--)
		{
			TOE2_W32(MiiMR1, 0|MiiMdioEn);
			Tmplong = MdioDelay(MiiMR1);
			TOE2_W32(MiiMR1, MiiMdioClk|MiiMdioEn);
			Tmplong = MdioDelay(MiiMR1);
		}
	}
}

static void PhySet(PMAC_ADAPTER pAdapter)
{
	TOE2_UTILITY_TRACE("PhySet()");

	//reset phy registers in a default state.
	//MdioWrite(pAdapter, ALi_PhyAddr, PhyBasicModeCtrl, BMCRReset);

    ali_phy_addr = g_ptoe2->mii_if.phy_id;

	pAdapter->Duplex = g_ptoe2->duplex_mode;
	pAdapter->Speed = (g_ptoe2->link_speed == 100) ? TRUE:FALSE;

	pAdapter->PauseFrameRx = g_ptoe2->pause_frame_rx;
	pAdapter->PauseFrameTx = g_ptoe2->pause_frame_tx;

	TOE2_UTILITY_TRACE("[NOTE] phy addr is: %d.", ali_phy_addr);
	TOE2_UTILITY_TRACE("[NOTE] phy duplex_mode is: %d.", pAdapter->Duplex);
	TOE2_UTILITY_TRACE("[NOTE] phy link_speed is: %d.", pAdapter->Speed);
	TOE2_UTILITY_TRACE("[NOTE] phy PauseFrame is: [%d,%d].", pAdapter->PauseFrameRx, pAdapter->PauseFrameTx);
}

static void PhySet_Force(PMAC_ADAPTER pAdapter)
{
	UINT16 BMCR = 0, TmpShort;

	do
	{	
		TmpShort = (UINT16)MdioRead(pAdapter, ali_phy_addr, PhyBasicModeCtrl);
	} while(TmpShort & BMCRReset);
	TOE2_UTILITY_TRACE("SetMII(): MII reset complete.");

	if (pAdapter->Speed == TRUE)
	{
		TOE2_UTILITY_TRACE("PhySet_Force(): 100M-");
		BMCR |= BMCRSpeedSet;
		if (pAdapter->Duplex == TRUE)
		{
			TOE2_UTILITY_TRACE("FD");
			BMCR |= BMCRDuplexMode;
		}
		else
		{
			TOE2_UTILITY_TRACE("HD");
			BMCR &= ~BMCRDuplexMode;
		}
	}		
	else
	{
		TOE2_UTILITY_TRACE("PhySet_Force(): 10M-");
		BMCR &= ~BMCRSpeedSet;
		if (pAdapter->Duplex == TRUE)
		{
			TOE2_UTILITY_TRACE("FD");
			BMCR |= BMCRDuplexMode;
		}
		else
		{
			TOE2_UTILITY_TRACE("HD");
			BMCR &= ~BMCRDuplexMode;
		}
	}

	//enable Rx/Tx pause frame.
	TmpShort = (UINT16)MdioRead(pAdapter, ali_phy_addr, PhyBasicModeCtrl);
	TmpShort &= ~(BMCRDuplexMode|BMCRSpeedSet);
	MdioWrite(pAdapter, ali_phy_addr, PhyBasicModeCtrl, BMCR|TmpShort);
}

static int MacHardwareStart(PMAC_ADAPTER pAdapter)
{
	UINT32 DuplexMode = 0;
	UINT32 PauseFrame = 0;	
	UINT32 tmp_u32;
	UINT32 ba = pAdapter->BaseAddr;

	TOE2_UTILITY_TRACE("MacHardwareStart()");

	tmp_u32 = TOE2_R32(DelayControlR);
	tmp_u32 &= ~CBR_DW_DLY;
	TOE2_W32(DelayControlR, tmp_u32 | (1<<CBR_DW_DLY_OFF));
	tmp_u32 = TOE2_R32(MiiMR1);

	if (pAdapter->HwMdioEn == TRUE)
		tmp_u32 &= ~MiiMdioEn;
	else
		tmp_u32 |= MiiMdioEn;
	TOE2_W32(MiiMR1, tmp_u32);

	//check phy registers first.
	if (pAdapter->AutoNeg == TRUE)
	{
		PhySet(pAdapter);	//Auto-Neg
	}
	else
	{
		PhySet_Force(pAdapter);
	}

	//set mac address.
	TOE2_W32(PAR, *((UINT32*)(pAdapter->MacAddr + 0)));
	TOE2_W32(PAR - 4, *((UINT32*)(pAdapter->MacAddr + 4)));

	//TOE2_W32(TimerR, 0x1bfffff); //28M Clk
	TOE2_W32(TimerR, 0x31fffff); //50M Clk

	//Set RMII.
	tmp_u32 = TOE2_R32(RmiiCR);
	tmp_u32 &= ~(RmiiCrSpeed|RmiiEn);	
	if (pAdapter->RmiiEn == TRUE)
	{
		if (pAdapter->Speed == TRUE)
		{
			tmp_u32 = (UINT32)RmiiCrSpeed;	//100Mbps
		}	
		TOE2_W32(RmiiCR, (tmp_u32|RmiiEn));
	}
	else
	{
		TOE2_W32(RmiiCR, tmp_u32);
	}

	if (pAdapter->Duplex)
		DuplexMode = (UINT32)FullDuplexMode;
	else 
		DuplexMode = (UINT32)0;

	if (pAdapter->ToeRxEn)
		DuplexMode |= RxTOEWorkMode;	

	TOE2_W32(NetworkOM, (DuplexMode|NetworkOMConfig)); 

	//Test Mux
	//TOE2_W8(0x58, 0x0E);  

	if (pAdapter->PauseFrameRx)	
		PauseFrame |= (UINT32)RxFlowControlEn;
	if (pAdapter->PauseFrameTx)	
		PauseFrame |= (UINT32)TxFlowControlEn;

	//Tx & Rx Config 2
	if (pAdapter->VlanSupport)
	{
		if (pAdapter->VlanTagRemove)
			TOE2_W32(TxRxCR2, (PauseFrame|TxRxConfig2|RxRemoveVlanTagEn|VlanEn));
		else
			TOE2_W32(TxRxCR2, (PauseFrame|TxRxConfig2|VlanEn));
	}
	else
		TOE2_W32(TxRxCR2, (PauseFrame|TxRxConfig2));

	TOE2_W32(TSAD, pAdapter->pTxDesc_DMA);
	TOE2_W32(RSAD, pAdapter->pRxDesc_DMA);

	TOE2_W16(RxDesTotNum, MAC_RX_DESC_NUM);
	TOE2_W16(TxDesTotNum, MAC_TX_DESC_NUM);

	TOE2_W16(RxRingDesWPtr, MAC_RX_DESC_NUM - 1);
	TOE2_W16(TxRingDesWPtr, 0);

	//Clr Cnts.
	TOE2_W16(MFC, 0);
	TOE2_W16(PPC, 0);
	TOE2_W16(LFC, 0);
	TOE2_W16(RPC, 0);
	TOE2_W16(AlignErrCnt, 0);
	TOE2_W16(CrcErrCnt, 0);
	TOE2_W16(IPHdrChsFailPC, 0);
	TOE2_W16(IPPayChsFailPC, 0);

	//Dis & Clr Int.
	TOE2_W32(IMR, 0);
	TOE2_W32(ISR, 0);

	return 0;
}

static void Mac_Init(PMAC_ADAPTER pAdapter)
{
	TOE2_UTILITY_TRACE("toe2 start phy hw reset...");
	ali_gpio_set_value(ali_physet_gpio, 0);
	mdelay(10);
	ali_gpio_set_value(ali_physet_gpio, 1);

#ifdef UTIL_DEBUG
    pAdapter->AutoNeg = 1;
    pAdapter->HwMdioEn = 1;
#endif

    TOE2_UTILITY_TRACE("  BasicAddr =      %x", pAdapter->BaseAddr);
	TOE2_UTILITY_TRACE("  AutoNeg =        %d", pAdapter->AutoNeg);
	TOE2_UTILITY_TRACE("  Speed =          %d", pAdapter->Speed);
	TOE2_UTILITY_TRACE("  Duplex =         %d", pAdapter->Duplex);
	TOE2_UTILITY_TRACE("  RmiiEn =         %d", pAdapter->RmiiEn);
	TOE2_UTILITY_TRACE("  HwMdioEn =       %d", pAdapter->HwMdioEn);
	TOE2_UTILITY_TRACE("  VlanSupport =    %d", pAdapter->VlanSupport);
	TOE2_UTILITY_TRACE("  VlanTagRemove =  %d", pAdapter->VlanTagRemove);
	TOE2_UTILITY_TRACE("  ToeRxEn =        %d", pAdapter->ToeRxEn);

	MacChipReset(pAdapter->BaseAddr);

	((UINT16*)pAdapter->MacAddr)[0] = 0x0012;
	((UINT16*)pAdapter->MacAddr)[1] = 0x3456;
	((UINT16*)pAdapter->MacAddr)[2] = 0x789a;
	TOE2_UTILITY_TRACE("Mac addr: 00-12-34-56-78-9a.");

	MacAllocateResource(pAdapter);
	MacInitRing(pAdapter);
	init_crc32();
	MacHardwareStart(pAdapter);
}

int util_mac_init(PMAC_Init_Context init_cxt)
{
	memset(&util_mac_adapter, 0, sizeof(MAC_ADAPTER));
	util_mac_adapter.BaseAddr = _TOE2_BASE - ALI_REGS_PHYS_BASE + ALI_REGS_VIRT_BASE;//__REG32ALI(_TOE2_BASE);

	util_mac_adapter.Duplex = init_cxt->Duplex;
	util_mac_adapter.Speed = init_cxt->Speed;
	util_mac_adapter.AutoNeg = init_cxt->AutoNeg;
	util_mac_adapter.RmiiEn = init_cxt->RmiiEn;
	util_mac_adapter.HwMdioEn = init_cxt->HwMdioEn;
	util_mac_adapter.VlanSupport = init_cxt->VlanSupport;
	util_mac_adapter.VlanTagRemove = init_cxt->VlanTagRemove;
	util_mac_adapter.ToeRxEn = init_cxt->ToeRxEn;

	Mac_Init(&util_mac_adapter);

	return 0;
}

static void MacWeirdInterrupt(PMAC_ADAPTER pAdapter)
{
	UINT16 TmpShort;
	UINT32 ba = pAdapter->BaseAddr;

	TmpShort = TOE2_R16(MFC);
	if (TmpShort)
	{
		TOE2_W16(MFC, 0);
		pAdapter->net_stats.rx_missed_errors += TmpShort;
		pAdapter->net_stats.rx_over_errors += TmpShort;
	}

	TmpShort = TOE2_R16(PPC);
	if (TmpShort)
	{
		TOE2_W16(PPC, 0);
		pAdapter->net_stats.rx_fifo_errors+= TmpShort;
	}

	TmpShort = TOE2_R16(LFC);
	if (TmpShort)
	{
		TOE2_W16(LFC, 0);
		pAdapter->net_stats.rx_length_errors += TmpShort;	
	}

	TmpShort = TOE2_R16(RPC);
	if (TmpShort)
	{
		TOE2_W16(RPC, 0);
		pAdapter->net_stats.rx_length_errors += TmpShort;	
	}

	TmpShort = TOE2_R16(CrcErrCnt);
	if (TmpShort)
	{
		TOE2_W16(CrcErrCnt, 0);
		pAdapter->net_stats.rx_crc_errors += TmpShort;	
	}

	TmpShort = TOE2_R16(AlignErrCnt);
	if (TmpShort)
	{
		TOE2_W16(AlignErrCnt, 0);
		pAdapter->net_stats.rx_frame_errors += TmpShort;	
	}

	TmpShort = TOE2_R16(IPHdrChsFailPC);
	if (TmpShort)
	{
		TOE2_W16(IPHdrChsFailPC, 0);
		pAdapter->mac_stats.rx_hdr_chs_errs+= TmpShort;
	}

	TmpShort = TOE2_R16(IPPayChsFailPC);
	if (TmpShort)
	{
		TOE2_W16(IPPayChsFailPC, 0);
		pAdapter->mac_stats.rx_pay_chs_errs+= TmpShort;
	}

	if ((pAdapter->CurrentISR & (ISRLinkStatus|ISRTimer)) && (pAdapter->AutoNeg))
	{
		TmpShort = (UINT16)MdioRead(pAdapter, ali_phy_addr, PhyBasicModeStatus);
		if (TmpShort & BMSRLinkStatus)
		{
			TmpShort = (UINT16)MdioRead(pAdapter, ali_phy_addr, PhyNWayLPAR);
			if (TmpShort != pAdapter->LinkPartner)
			{
				if (TmpShort)
				{
					TOE2_UTILITY_TRACE("MacWeirdInterrupt: Link Connected. \n");
					Mac_Init(pAdapter);
				}
			}
		}
		else
			TOE2_UTILITY_TRACE("MacWeirdInterrupt: Link Disconnected. \n");
	}
}

//analyze & recode rx status while head is okay.
static int ReceiveChksumOk(PMAC_ADAPTER pAdapter, ppacket_head pHead)
{	
	if (pAdapter->ToeRxEn)
	{
		if (pHead->IPFrame)
		{
			if (pHead->IPFrag)
				goto Done;

			if (!pHead->IPChksum)
			{
				pAdapter->mac_stats.rx_ip_chksum_errors++;
				TOE2_UTILITY_TRACE("ReceiveChksumOk: ip checksum err");
				goto Done;
			}

			if (!pHead->TCPChksum)
			{
				pAdapter->mac_stats.rx_tcp_chksum_errors++;
				TOE2_UTILITY_TRACE("ReceiveChksumOk: tcp checksum err");
				goto Done;
			}
			return TRUE;
		}
	}

Done:
	return FALSE;
}

static int ReceiveHeadOK(PMAC_ADAPTER pAdapter, ppacket_head pHead)
{
	int fatal_err = 0;

	if (pHead->ES)
	{
		if (pHead->WatchdogTimeout)
		{
			fatal_err ++;
			pAdapter->mac_stats.rx_wd_timeout_errors++;
		}
		if (pHead->PhysicalLayerError)
		{
			fatal_err ++;
			pAdapter->mac_stats.rx_phy_layer_errors++;
		}
		if (pHead->LateCollision)
		{
			fatal_err ++;
			pAdapter->mac_stats.rx_late_col_seen++;
		}
		if (pHead->Long)
		{
			fatal_err ++;
			pAdapter->mac_stats.rx_long_errors++;
		}
		if (pHead->Runt) 
		{
			fatal_err ++;
			pAdapter->mac_stats.rx_runt_errors++;	
		}
		if (pHead->Dribble)
		{
			fatal_err ++;
			pAdapter->mac_stats.rx_dribble_errors++;	
		}

		if ((pHead->FifoOverflow) && (0 == fatal_err))
		{
			return TRUE;
		}
		else
		{
			pAdapter->net_stats.rx_errors++;
			return FALSE;
		}
	}	
	else
	{
		if ((pHead->PacketLength > MAX_ETH_PKT_LEN) || (pHead->PacketLength < MIN_ETH_PKT_LEN))
			return FALSE;

		pAdapter->net_stats.rx_packets++;
		pAdapter->net_stats.rx_bytes += pHead->PacketLength;

		if (pHead->BF)
			pAdapter->mac_stats.rx_bc++;
		if (pHead->PF)
			pAdapter->mac_stats.rx_uc++;
		if (pHead->MF)
			pAdapter->mac_stats.rx_mc++;
		if (pHead->PPPoE)
			pAdapter->mac_stats.rx_pppoe++;
		if (pHead->VLAN)
			pAdapter->mac_stats.rx_vlan++;
		if (pHead->IPFrame)
			pAdapter->mac_stats.rx_ip++;
		if (pHead->IPFrag)
			pAdapter->mac_stats.rx_frag++;

		return TRUE;
	}	
}

static int ToeTCPCheck(PMAC_ADAPTER pAdapter, ppacket_head pHead, UINT8 *pChkStart, UINT16 ChkLen,
		UINT32 ip_src, UINT32 ip_des, UINT16 ip_protocol, UINT16 TCPChksum)
{
	int ret = TRUE;
	UINT32 acc;

	acc = standard_chksum(pChkStart, ChkLen);

	while ((acc >> 16) != 0) {
		acc = (acc & 0xffff) + (acc >> 16);
	}

	acc += (ip_src & 0xFFFFUL);
	acc += ((ip_src>>16) & 0xFFFFUL);

	acc += (ip_des & 0xFFFFUL);
	acc += ((ip_des>>16) & 0xFFFFUL);

	acc += (UINT32)htons(ip_protocol);
	acc += (UINT32)htons(ChkLen);

	while ((acc >> 16) != 0) {
		acc = (acc & 0xffffUL) + (acc >> 16);
	}

	if ((UINT16)acc != TCPChksum)	
	{
		TOE2_UTILITY_TRACE("ToeTCPCheck: err.\n");
		ret = FALSE;
	}

	return ret;	
}

static int ToeICMPCheck(PMAC_ADAPTER pAdapter, ppacket_head pHead, UINT8 *pChkStart, UINT16 ChkLen, UINT16 TCPChksum)
{
	int ret = TRUE;
	UINT16 tmp_u16;

	tmp_u16 = inet_chksum(pChkStart, ChkLen);

	if (tmp_u16 != TCPChksum)	
	{
		TOE2_UTILITY_TRACE("ToeICMPCheck: err.\n");
		ret = FALSE;
	}

	return ret;	
}

static int ToeOthCheck(PMAC_ADAPTER pAdapter, ppacket_head pHead, UINT8 *pChkStart, UINT16 ChkLen, UINT16 TCPChksum)
{
	return TRUE;
}

static int ToeIPCheck(PMAC_ADAPTER pAdapter, ppacket_head pHead, UINT8 *pPkt, UINT16 IPChksum, UINT16 TCPChksum)
{
	int ret = TRUE;
	UINT32 ip_src, ip_des;
	UINT16 ip_hdrlen, ip_totlen, ip_chksum;
	UINT16 eth_type;
	UINT16 tmp_u16;
	UINT16 chk_len;
	UINT8 ip_protocol;
	UINT8 *chk_start = pPkt;

	tmp_u16 = *(UINT16 *)(pPkt + ETH_TYPE_OFF);
	eth_type = htons(tmp_u16);

	if (pHead->IPFrame)
	{
		if (pHead->PPPoE)
		{
			if (eth_type != ETH_P_PPP_SES)
			{
				TOE2_UTILITY_TRACE("ToeCheck: eth_type != ETHTYPE_PPPOE, Eth type err.\n");
				ret = FALSE;
				goto ToeCheckEnd;
			}

			chk_start += ETH_PPPOE_PRO_OFF;

			tmp_u16 = *((UINT16 *)chk_start);
			tmp_u16 = htons(tmp_u16);

			if (tmp_u16 != PPPOE_IP)
			{
				TOE2_UTILITY_TRACE("ToeCheck: tmp_u16 != PPPOE_IP, No need to do chksum.\n");
				ret = TRUE;
				goto ToeCheckEnd;
			}			

			chk_start += ETH_PPPOE_PRO_LEN;
		}
		else
		{
			chk_start += ETH_IP_OFF;
		}

		tmp_u16 = *(UINT16 *)(chk_start + IP_LEN_OFF);
		tmp_u16 = htons(tmp_u16);
		ip_hdrlen = ((tmp_u16>>8) & 0xF) * 4;

		tmp_u16 = *(UINT16 *)(chk_start + IP_TOTLEN_OFF);
		ip_totlen = htons(tmp_u16);		

		tmp_u16 = *(UINT16 *)(chk_start + IP_PRO_OFF);
		tmp_u16 = htons(tmp_u16);
		ip_protocol = (UINT8)(tmp_u16 & 0xFF);

		//ip_src = *(UINT32 *)(chk_start + IP_IPSRC_OFF);
		//ip_des = *(UINT32*)(chk_start + IP_IPDES_OFF);

		ip_src = (chk_start + IP_IPSRC_OFF)[0]|((chk_start + IP_IPSRC_OFF)[1]<<8)|((chk_start + IP_IPSRC_OFF)[2]<<16)|((chk_start + IP_IPSRC_OFF)[3]<<24);
		ip_des = (chk_start + IP_IPDES_OFF)[0]|((chk_start + IP_IPDES_OFF)[1]<<8)|((chk_start + IP_IPDES_OFF)[2]<<16)|((chk_start + IP_IPDES_OFF)[3]<<24);

		ip_chksum = inet_chksum(chk_start, ip_hdrlen);
		if (ip_chksum != IPChksum)
		{
			TOE2_UTILITY_TRACE("ToeCheck: (ip_chksum != IPChksum).\n");
			ret = FALSE;
			goto ToeCheckEnd;
		}			

		if ((pHead->IPChksum) && (IPChksum != 0xFFFF)) //IP chksum is ok.
		{
			TOE2_UTILITY_TRACE("ToeCheck: ((pHead->IPChksum) && (IPChksum != 0xFFFF)).\n");
			ret = FALSE;
			goto ToeCheckEnd;
		}

		//ip payloader
		chk_start += ip_hdrlen;
		chk_len = ip_totlen - ip_hdrlen;

		switch(ip_protocol)
		{
			case IPPROTO_TCP:
			case IPPROTO_UDP:	
			case IPPROTO_UDPLITE:
				ret = ToeTCPCheck(pAdapter, pHead, chk_start, chk_len, 
						ip_src, ip_des, (UINT16)ip_protocol, TCPChksum);
				break;

			case IPPROTO_ICMP:
			case IPPROTO_IGMP:
				ret = ToeICMPCheck(pAdapter, pHead, chk_start, chk_len, TCPChksum);
				break;

			default:
				TOE2_UTILITY_TRACE("ToeCheck: Unknow ip_protocol.\n");
				ret = ToeICMPCheck(pAdapter, pHead, chk_start, chk_len, TCPChksum);
				break;				
		}

	}
	else //(pHead->IPFrame)
	{
		if (pHead->PPPoE)
		{
			if ((eth_type != ETH_P_PPP_SES) && (eth_type != ETH_P_PPP_DISC))
			{
				TOE2_UTILITY_TRACE("ToeCheck: (eth_type != ETHTYPE_PPPOE) && (eth_type != ETHTYPE_PPPOEDISC).\n");
				ret = FALSE;
				goto ToeCheckEnd;
			}
		}
		else
		{
			;//TOE2_UTILITY_TRACE("ToeCheck: Unknow IP type.\n");
		}
	}

ToeCheckEnd:

	return ret;
}

static void MacRxStop(PMAC_ADAPTER pAdapter)
{
	UINT8 TmpChar;
	UINT32 ba = pAdapter->BaseAddr;

	TmpChar = TOE2_R8(SCR);

	if (pAdapter->ToeRxEn)
		TOE2_W8(SCR, (TmpChar&(~(UINT8)SCRRxEn)&(~(UINT8)SCRRxCoeEn)));
	else	
		TOE2_W8(SCR, (TmpChar&(~(UINT8)SCRRxEn)));
}

static unsigned short ReceiveUpdateDescPtr(PMAC_ADAPTER pAdapter)
{
	UINT16 BufReadPtr, DescReadPtr, DescWritePtr;
	UINT16 Updata = 0;
	UINT32 RxDescD0, RxDescD1, RxDescD2, RxDescD3;
	UINT32 i;

	UINT32 ba = pAdapter->BaseAddr;

	BufReadPtr = pAdapter->RxBufRPtr;
	DescReadPtr = TOE2_R16(RxRingDesRPtr);
	DescWritePtr = pAdapter->RxDescWPtr;

	//TOE2_UTILITY_TRACE("DescReadPtr:%d, BufReadPtr:%d, DescWritePtr:%d.\n", DescReadPtr, BufReadPtr, DescWritePtr);

	//Complicated here...
	if (DescWritePtr > DescReadPtr)
	{
		if ((BufReadPtr > DescReadPtr) && (BufReadPtr <= DescWritePtr))
			goto RX_LOST;
		else
		{
			if (BufReadPtr > DescWritePtr)
				Updata = BufReadPtr - DescWritePtr - 1;
			else
				Updata = MAC_RX_DESC_NUM + BufReadPtr - DescWritePtr -1;
		}			
	}	
	else if (DescWritePtr < DescReadPtr)
	{
		if ((BufReadPtr > DescReadPtr) ||(BufReadPtr <=DescWritePtr))
			goto RX_LOST;	
		else
			Updata = BufReadPtr - DescWritePtr -1;
	}
	else
	{
		if (DescWritePtr < BufReadPtr)
			Updata = BufReadPtr - DescWritePtr -1;
		else if (DescWritePtr > BufReadPtr)
			Updata = MAC_RX_DESC_NUM + BufReadPtr - DescWritePtr -1;
		else
			goto RX_LOST;	
	}

	if (Updata > 0)
	{
		i = DescWritePtr;
		while (Updata > 0)
		{
			RxDescD0 = pAdapter->RxDescBuf[i];
			RxDescD1 = pAdapter->RxDescBuf[i] + 4;
			RxDescD2 = pAdapter->RxDescBuf[i] + 8;
			RxDescD3 = pAdapter->RxDescBuf[i] + 12;
			/*
			   WriteMemD(RxDescD0,0);
			   WriteMemD(RxDescD1,0);
			   WriteMemD(RxDescD3, pAdapter->RxDescBuf[i]);
			   */
			*((UINT32*)RxDescD0) = 0;
			*((UINT32*)RxDescD1) = 0;
			*((UINT32*)RxDescD3) = pAdapter->RxDescBuf[i];

			if (i == MAC_RX_DESC_NUM - 1)
			{
				//WriteMemD(RxDescD2,RX_DESC_EOR);
				//#define RX_DESC_EOR	0x80000000
				*((UINT32*)RxDescD2) = 0x80000000;//RX_DESC_EOR;
				i = 0;
			}
			else
			{
				//WriteMemD(RxDescD2,0);
				*((UINT32*)RxDescD2) = 0;
				i ++;
			}		
			Updata --;
		}

		pAdapter->RxDescWPtr = i;
		TOE2_W16(RxRingDesWPtr, pAdapter->RxDescWPtr);
	}

	//TOE2_UTILITY_TRACE("DescReadPtr:%d, BufReadPtr:%d, DescWritePtr:%d.\n", DescReadPtr, BufReadPtr, DescWritePtr);
	return DescReadPtr;

RX_LOST:
	//AfxMessageBox("BufReadPtr Got Lost");
	//while(1);//asm("sdbbp");
	TOE2_UTILITY_TRACE("DescReadPtr:%d, BufReadPtr:%d, DescWritePtr:%d.\n", 
			DescReadPtr, BufReadPtr, DescWritePtr);
	return DescReadPtr;
}

static int ReceivePackets(PMAC_ADAPTER pAdapter)
{
	UINT16 BufReadPtr, DescReadPtr;
	UINT16 HdrChs, PayChs, VlanTag;
	UINT16 Pkts;
	UINT32 RxDescD0, RxDescD1, RxDescD2, RxDescD3;
	UINT32 Sts, BufAddr; 
	UINT32 Crc,Crc_hw;
	ppacket_head	pHead;
	UINT16 PacketSize;
	UINT32 i;
	UINT16 *Temp;
	UINT8 *pPkt = NULL;
	int Result = TRUE;

	DescReadPtr = ReceiveUpdateDescPtr(pAdapter);
	//DescReadPtr = MAC_R16(RxRingDesRPtr);
	BufReadPtr = pAdapter->RxBufRPtr;

	if (DescReadPtr >= BufReadPtr)
		Pkts = DescReadPtr - BufReadPtr;
	else
		Pkts = MAC_RX_DESC_NUM + DescReadPtr - BufReadPtr;

	if (Pkts == 0)
		return FALSE;
	if (DescReadPtr == pAdapter->RxDescWPtr)
		Pkts --;

	i = BufReadPtr;
	while(Pkts > 0)
	{
		RxDescD0 = pAdapter->RxDescBuf[i];
		RxDescD1 = pAdapter->RxDescBuf[i] + 4;
		RxDescD2 = pAdapter->RxDescBuf[i] + 8;
		RxDescD3 = pAdapter->RxDescBuf[i] + 12;
		/*
		   Sts = ReadMemD(RxDescD0);
		   PayChs = ReadMemW(RxDescD1);
		   HdrChs = ReadMemW(RxDescD1+2);
		   VlanTag = ReadMemW(RxDescD2);
		   BufAddr = ReadMemD(RxDescD3);
		   */
		Sts = *((UINT32*)RxDescD0);
		Temp = (UINT16*)RxDescD1;
		PayChs = *(Temp);
		Temp = (UINT16*)(RxDescD1+2);
		HdrChs = *(Temp);
		Temp = (UINT16*)RxDescD2;
		VlanTag = *(Temp);
		BufAddr = *((UINT32*)RxDescD3);
		BufAddr |= 0xa0000000;

		pHead = (ppacket_head)(&Sts);
		if (ReceiveHeadOK(pAdapter, pHead))
		{
			//TOE2_UTILITY_TRACE("head ok 0x:%08x \n",Sts);
			PacketSize = pHead ->PacketLength;
			ReceiveChksumOk(pAdapter, pHead);

			if (pAdapter->CrcVerify || pAdapter->ChksumVerify)
			{
				//copy one packet with head & crc to host memory.
				ALICopyFromMem(
						pAdapter->RxBuffer + (i * TOE2_BUF_SZ),
						BufAddr,
						PacketSize//pkt & crc.
						);

				pPkt = pAdapter->RxBuffer + (i * TOE2_BUF_SZ);
			}

			/*************************************************
			 *	Checksum Verify
			 *************************************************/
			if (pAdapter->ChksumVerify)
			{
				if (!pAdapter->ToeRxEn)
					return FALSE;

				//ALiPrintBuf(pPkt, PacketSize);	

				Result = ToeIPCheck(pAdapter, pHead, pPkt, HdrChs, PayChs);

				if (Result == FALSE)
				{
					MacRxStop(pAdapter);
					//test use.
					//TmpCMD = MAC_R8(0x5b);  
					//MAC_W8(0x5b, TmpCMD|0x80);  
					//AfxMessageBox("ReceivePackets: Chksum error!!\n");
					while(1);// asm("sdbbp");
				}
			}

			/*************************************************
			 *	CRC Verify
			 *************************************************/
			if (pAdapter->CrcVerify)
			{
				if (pHead->VLAN && pAdapter->VlanTagRemove)
				{
					int v_off = 0;
					memcpy(ali_vlan_chk, pPkt, 12); v_off += 12;
					*(UINT16 *)(ali_vlan_chk + v_off) = SWAP(ETH_P_8021Q); v_off += 2;
					*(UINT16 *)(ali_vlan_chk + v_off) = SWAP(VlanTag); v_off += 2;

					memcpy((ali_vlan_chk + v_off), (pPkt + 12), PacketSize -12); 

					PacketSize += 4;
					pPkt = ali_vlan_chk;
				}

				Crc = ALiCrc32(pPkt, (PacketSize - 4));
				//TOE2_UTILITY_TRACE("Len(%x)->CRC(0x%08x)\n", PacketSize - i, CRC);
				//if (Crc != *(UINT32 *)(pPkt +  PacketSize - 4))
				Crc_hw = (pPkt+PacketSize-4)[0]|((pPkt+PacketSize-4)[1]<<8)|((pPkt+PacketSize-4)[2]<<16)|((pPkt+PacketSize-4)[3]<<24);
				if (Crc != Crc_hw)
				{
					//MacRxStop(pAdapter);
					TOE2_UTILITY_TRACE("CRC error: Crc(0x%08x)!=(0x%08x)\n", Crc, Crc_hw);
					//test use.
					//TmpCMD = MAC_R8(0x5b);  
					//MAC_W8(0x5b, TmpCMD|0x80);  
					//AfxMessageBox("ReceivePackets: CRC compare error!!\n");
					while(1);//asm("sdbbp");
				}
			}
		}
		else
		{
			//TOE2_UTILITY_TRACE("head err 0x%08x\n",Sts);
			PacketSize = pHead ->PacketLength;
			//copy one packet with head & crc to host memory.
			ALICopyFromMem(
					pAdapter->RxBuffer + (i * TOE2_BUF_SZ),
					BufAddr,
					PacketSize//pkt & crc.
					);
			pPkt = pAdapter->RxBuffer + (i * TOE2_BUF_SZ);

			//test use.
			//TmpCMD = MAC_R8(0x54);  
			//MAC_W8(0x54, TmpCMD|0x01);  
			//AfxMessageBox("ReceivePackets: Header error!!\n");

			if (i == MAC_RX_DESC_NUM - 1)
				i = 0;
			else
				i ++;
			break;
		}

		if (i == MAC_RX_DESC_NUM - 1)
			i = 0;
		else
			i ++;

		Pkts --;
	}

	pAdapter->RxBufRPtr = i;
	return TRUE;
}

static void MacRxInterrupt(PMAC_ADAPTER pAdapter)
{
	UINT8 TmpCMD;
	UINT32 ba = pAdapter->BaseAddr;
	//TOE2_UTILITY_TRACE("MacRxInterrupt. \n");
	do
	{
		// read Command Register.
		TmpCMD = TOE2_R8(SCR);
		// if Rx ring buffer empty, break out.
		if ((TmpCMD & (UINT8)SCRBufEmpty))
		{
			pAdapter->mac_stats.rx_buf_empty ++;
			break;
		}

		if (ReceivePackets(pAdapter) != 0)
			break;
	} while(1);
}

void MacIsr(PMAC_ADAPTER pAdapter)
{
	UINT32 ba = pAdapter->BaseAddr;
	UINT32 CurrentISR = TOE2_R32(ISR);

	TOE2_W32(ISR, 0);

	if (0)
		TOE2_UTILITY_TRACE("ISR=%x.", CurrentISR);

	if (CurrentISR & pAdapter->InterruptMask)
	{
		pAdapter->CurrentISR = (CurrentISR & pAdapter->InterruptMask);

		//Rx interrupt.
		if (pAdapter->CurrentISR & (ISRRxComplete|ISRRxFifoOverflow |ISRRxBufOverflow))
		{
			//TOE2_UTILITY_TRACE("ISR=%x. \n", CurrentISR);
			MacRxInterrupt(pAdapter);
		}

		if (pAdapter->CurrentISR & (ISRLinkStatus |ISRTimer))
			MacWeirdInterrupt(pAdapter);

		if (pAdapter->CurrentISR & (ISRRxHdrErr|ISRTxCoeErr|ISRWatchdog))
			TOE2_UTILITY_TRACE("(ISRRxHdrErr|ISRTxCoeErr|ISRWatchdog)=>%x.", CurrentISR);
	}	
}

static int MacRxTest(struct work_struct *work)
{
	PMAC_ADAPTER pAdapter;

	while (0 == m_bRxThreadStart)
	{	
		//TOE2_UTILITY_TRACE("rx task hold: \n");
		msleep(100);
	}

	pAdapter = &util_mac_adapter;
	TOE2_UTILITY_TRACE("rx task start:\n");
	while (!m_bRxStopThread)
	{
		MacIsr(pAdapter);	
	}

	return 0;
}

struct workqueue_struct *util_rx_wq;
struct work_struct util_rx_work;
void util_mac_rx_thread_create(void)
{
	TOE2_UTILITY_TRACE("allocate work queue for rx thread");
	util_rx_wq = create_workqueue("MacRxTest");
	if (!util_rx_wq)
	{    
		printk("[ERR] Failed to allocate rx work queue\n");
	}    
	else 
	{    
		INIT_WORK(&util_rx_work, MacRxTest);
		queue_work(util_rx_wq, &util_rx_work);
	}    
}

void util_mac_rx_thread_destroy(void)
{
	destroy_workqueue(util_rx_wq);
}

static void MacRxStart(PMAC_ADAPTER pAdapter)
{
	UINT8 TmpChar;
	UINT32 Tmp32;
	UINT32 ba = pAdapter->BaseAddr;

	pAdapter->net_stats.rx_packets = 0;
	pAdapter->net_stats.rx_bytes = 0;
	pAdapter->net_stats.rx_errors = 0;
	pAdapter->net_stats.rx_dropped = 0;
	pAdapter->net_stats.rx_length_errors = 0;
	pAdapter->net_stats.rx_over_errors = 0;
	pAdapter->net_stats.rx_crc_errors = 0;
	pAdapter->net_stats.rx_frame_errors = 0;
	pAdapter->net_stats.rx_fifo_errors = 0;
	pAdapter->net_stats.rx_missed_errors = 0;

	pAdapter->mac_stats.rx_mc = 0;
	pAdapter->mac_stats.rx_bc = 0;
	pAdapter->mac_stats.rx_uc = 0;
	pAdapter->mac_stats.rx_vlan = 0;
	pAdapter->mac_stats.rx_pppoe = 0;	
	pAdapter->mac_stats.rx_ip = 0;
	pAdapter->mac_stats.rx_runt_errors = 0;
	pAdapter->mac_stats.rx_long_errors = 0;
	pAdapter->mac_stats.rx_dribble_errors = 0;
	pAdapter->mac_stats.rx_phy_layer_errors = 0;
	pAdapter->mac_stats.rx_wd_timeout_errors = 0;
	pAdapter->mac_stats.rx_ip_chksum_errors = 0;
	pAdapter->mac_stats.rx_tcp_chksum_errors = 0;
	pAdapter->mac_stats.rx_buf_empty = 0;
	pAdapter->mac_stats.rx_late_col_seen = 0;
	pAdapter->mac_stats.rx_lost_in_ring = 0;
	pAdapter->mac_stats.rx_hdr_chs_errs = 0;
	pAdapter->mac_stats.rx_pay_chs_errs = 0;

	memset(pAdapter->RxBuffer, 0x0, (MAC_RX_DESC_NUM * TOE2_BUF_SZ));

	Tmp32 = TOE2_R32(NetworkOM);
	Tmp32 &= ~(FilteringMask|PassMask|WorkModeMask);

	if (pAdapter->LoopBackEn)
		Tmp32 |= WorkModeLoopBack;
	else
		Tmp32 |= WorkModeNormal;

	if (pAdapter->PassMulticast)
		Tmp32 |= PassAllMulticast;
	if (pAdapter->Promiscuous)
		Tmp32 |= PassPromiscuous;
	if (pAdapter->PassBad)
		Tmp32 |= PassErr;

	Tmp32 |= PassAllMulticast;
	Tmp32 |= PassPromiscuous;
	Tmp32 |= PassErr;

	switch (pAdapter->FilteringMode)
	{
		case 0:
			Tmp32 |= HashFiltering;
			break;
		case 1:
			Tmp32 |= HashOnlyFiltering;
			break;
		case 2:
			Tmp32 |= InverseFiltering;
			break;
		case 3:
			Tmp32 |= PerfectFiltering;
			break;
		default:
			Tmp32 |= PerfectFiltering;
			break;
	}

	TOE2_W32(NetworkOM, Tmp32);

	TmpChar = TOE2_R8(SCR);

	if (pAdapter->ToeRxEn)
		TOE2_W8(SCR, (TmpChar|(UINT8)SCRRxEn|(UINT8)SCRRxCoeEn));
	else
		TOE2_W8(SCR, (TmpChar|(UINT8)SCRRxEn));
}

int util_mac_rx_start(PMAC_Rx_Context pRxContext)
{
	util_mac_adapter.LoopBackEn = pRxContext->LoopBackEn;
	util_mac_adapter.PassMulticast = pRxContext->PassMulticast;
	util_mac_adapter.Promiscuous = pRxContext->Promiscuous;
	util_mac_adapter.PassBad = pRxContext->PassBad;
	util_mac_adapter.CrcVerify= pRxContext->CrcVerify;
	util_mac_adapter.ChksumVerify = pRxContext->ChksumVerify;	
	util_mac_adapter.FilteringMode = pRxContext->FilteringMode;

#ifdef UTIL_DEBUG
	util_mac_adapter.FilteringMode = MAC_FILTER_PERFECT;
#endif

	TOE2_UTILITY_TRACE("[T] rx LoopBackEn:%d", util_mac_adapter.LoopBackEn);
	TOE2_UTILITY_TRACE("[T] rx PassMulticast:%d", util_mac_adapter.PassMulticast);
	TOE2_UTILITY_TRACE("[T] rx Promiscuous:%d", util_mac_adapter.Promiscuous);
	TOE2_UTILITY_TRACE("[T] rx PassBad:%d", util_mac_adapter.PassBad);
	TOE2_UTILITY_TRACE("[T] rx CrcVerify:%d", util_mac_adapter.CrcVerify);
	TOE2_UTILITY_TRACE("[T] rx ChksumVerify:%d", util_mac_adapter.ChksumVerify);
	TOE2_UTILITY_TRACE("[T] rx FilteringMode:%d", util_mac_adapter.FilteringMode);

	m_bRxStopThread = 0;

	TOE2_UTILITY_TRACE("util_mac_rx_start start...");
	MacRxStart(&util_mac_adapter);

	m_bRxThreadStart = 1;

	//struct task_struct *rx_thread = kthread_create(MacRxTest, (void *)(&util_mac_adapter), "MacRxTest");
	//wake_up_process(rx_thread);
	//kernel_thread(MacRxTest, (void *)(&util_mac_adapter), 0);
	return 0;
}

void util_mac_rx_stop(PMAC_ADAPTER pAdapter)
{
	MacRxStop(pAdapter);
	m_bRxStopThread = 1;
}

static void MacHeaderBuild(PMAC_ADAPTER pAdapter, UINT8 *pBuf, UINT16 len)
{
	unsigned char *pHdr = pBuf;
	UINT16 tmp_u16;
	UINT16 IpHdrLen = 5;
	UINT32 acc = 0;
	UINT32 ip_src, ip_des; 
	UINT16 ip_protocol;
	UINT16 ChkLen;
	UINT32 L3HeaderLen;

	UINT8 IpStartOff = 12; 

	ChsOff = 0;

	memcpy(pHdr, &MacDstAddr[0], 6);
	pHdr += 6; // 6 now.
	memcpy(pHdr, &MacSrcAddr[0], 6);
	pHdr += 6; // 12 now.
	IpStartOff = 12; 

	memset(&ali_desc_hw, 0, sizeof(ali_desc_hw));

	if (ali_tx_para.AddVlanTag)
	{
#if (1)		
		ali_desc_hw.tx_sts.hw.VlanEn = 1;
		ali_desc_hw.vlan_tag = ali_tx_para.TxVlanTag;
#else		
		*(UINT16 *)pHdr = SWAP(ETH_P_8021Q); pHdr += 2; //type: VLAN tag
		*(UINT16 *)pHdr = SWAP(ali_tx_para.TxVlanTag); pHdr += 2; //tag 
		IpStartOff += 4; 
#endif
	}

	switch (ali_tx_para.FrameType)
	{
		case MAC_FRAME_8023:
			IpStartOff = 0;
			break;

		case MAC_FRAME_ETHERII:
			*(UINT16 *)pHdr = SWAP(ETH_P_IP); // IP
			//*(UINT16 *)pHdr = 0x0608; // ARP
			//*(UINT16 *)pHdr = 0x3580; // RARP
			IpStartOff = 0;
			break;

		case MAC_FRAME_IP_ICMP:
			*(UINT16 *)pHdr = SWAP(ETH_P_IP); 
			pHdr += 2; // 14 now.

			*(UINT16 *)(pHdr+0) = (0x0040 |IpHdrLen); //Version HeaderLength.
			*(UINT16 *)(pHdr+2) = SWAP((len - ETH_HLEN)); //Tot Len.
			*(UINT16 *)(pHdr+6) = 0x0000; //Offset
			*(UINT16 *)(pHdr+8) = 0x0180; //Ttl(128) & Protocol(ICMP:0x01)
			*(UINT16 *)(pHdr+10) = 0x0000; //IP Checksum
			IpStartOff += 2; // 14 now.

			pHdr += (IpHdrLen*4); 
			*(UINT16 *)(pHdr+2) = 0x0000; //Clear ICMP checksum.

			if (pAdapter->ToeTxEn)
				ali_desc_hw.tx_sts.hw.CoeEn = 1;
			break;

		case MAC_FRAME_IP_IGMP:
			*(UINT16 *)pHdr = SWAP(ETH_P_IP); 
			pHdr += 2; // 14 now.

			*(UINT16 *)(pHdr+0) = (0x0040 |IpHdrLen); //Version HeaderLength.
			*(UINT16 *)(pHdr+2) = SWAP((len - ETH_HLEN)); //Tot Len.	
			*(UINT16 *)(pHdr+6) = 0x0000; //Offset
			*(UINT16 *)(pHdr+8) = 0x0280; //Ttl(128) & Protocol(IGMP:0x02)
			*(UINT16 *)(pHdr+10) = 0x0000; //IP Checksum
			IpStartOff += 2;  // 14 now.

			pHdr += (IpHdrLen*4); 
			*(UINT16 *)(pHdr+2) = 0x0000; //Clear IGMP checksum.
			if (pAdapter->ToeTxEn)
				ali_desc_hw.tx_sts.hw.CoeEn = 1;

			break;

		case MAC_FRAME_IP_UDP:
			*(UINT16 *)pHdr = SWAP(ETH_P_IP); 
			pHdr += 2; // 14 now.

			*(UINT16 *)(pHdr+0) = (0x0040 |IpHdrLen); //Version HeaderLength.
			*(UINT16 *)(pHdr+2) = SWAP((len - ETH_HLEN)); //Tot Len.
			*(UINT16 *)(pHdr+6) = 0x0000; //Offset.
			ip_protocol = 0x1180;
			*(UINT16 *)(pHdr+8) = ip_protocol; //Ttl(128) & Protocol(UDP:0x11).
			*(UINT16 *)(pHdr+10) = 0x0000; //IP Checksum.
			IpStartOff += 2;  // 14 now.	

			acc = standard_chksum_2(pHdr, (IpHdrLen*4));
			acc = htons((UINT16)~acc);
			*(UINT16 *)(pHdr+10) = acc;
			acc = 0;

			L3HeaderLen = 20;
			*(UINT16 *)(pHdr+ (IpHdrLen*4) +4)=SWAP((len - ETH_HLEN - L3HeaderLen)); 
			*(UINT16 *)(pHdr + (IpHdrLen*4) +6) = 0x0000; //Clear UDP checksum
			if (pAdapter->ToeTxEn)
			{
				ali_desc_hw.tx_sts.hw.CoeEn = 1;
				ali_desc_hw.tx_sts.hw.UdpPkt = 1;

				//ip_src = *(UINT32 *)(pHdr + IP_IPSRC_OFF);
				//ip_des = *(UINT32*)(pHdr + IP_IPDES_OFF);			

				ip_src = (pHdr + IP_IPSRC_OFF)[0]|((pHdr + IP_IPSRC_OFF)[1]<<8)|((pHdr + IP_IPSRC_OFF)[2]<<16)|((pHdr + IP_IPSRC_OFF)[3]<<24);
				ip_des = (pHdr + IP_IPDES_OFF)[0]|((pHdr + IP_IPDES_OFF)[1]<<8)|((pHdr + IP_IPDES_OFF)[2]<<16)|((pHdr + IP_IPDES_OFF)[3]<<24);

				pHdr += (IpHdrLen*4);
				ChkLen = len - ETH_HLEN - (IpHdrLen*4);

				acc = standard_chksum(pHdr, ChkLen);

				while ((acc >> 16) != 0)
				{
					acc = (acc & 0xffff) + (acc >> 16);
				}

				acc += (ip_src & 0xFFFFUL);
				acc += ((ip_src>>16) & 0xFFFFUL);

				acc += (ip_des & 0xFFFFUL);
				acc += ((ip_des>>16) & 0xFFFFUL);

				acc += (ip_protocol & 0xFF00);
				acc += (UINT32)SWAP(ChkLen);

				while ((acc >> 16) != 0)
				{
					acc = (acc & 0xffffUL) + (acc >> 16);
				}

				ChsAcc =(UINT16)~acc;
				ChsOff = ETH_HLEN + (IpHdrLen*4) + 6;
			}
			break;

		case MAC_FRAME_IP_TCP:
			*(UINT16 *)pHdr = SWAP(ETH_P_IP); 
			pHdr += 2; // 14 now.	

			*(UINT16 *)(pHdr+0) = (0x0040 |IpHdrLen); //Version HeaderLength.
			*(UINT16 *)(pHdr+2) = SWAP((len - ETH_HLEN)); //Tot Len.
			*(UINT16 *)(pHdr+6) = 0x0000; //Offset.
			ip_protocol = 0x0680;
			*(UINT16 *)(pHdr+8) = ip_protocol; //Ttl(128) & Protocol(TCP:0x06).
			*(UINT16 *)(pHdr+10) = 0x0000; //IP Checksum.
			IpStartOff += 2;  // 14 now.

			acc = standard_chksum_2(pHdr, (IpHdrLen*4));
			acc = htons((UINT16)~acc);
			*(UINT16 *)(pHdr+10) = acc;
			acc = 0;

			*(UINT16 *)(pHdr+(IpHdrLen*4)+12) = 0x50;//make tcp header len is 5(5*4=20)

			*(UINT16 *)(pHdr+(IpHdrLen*4)+16) = 0x0000; //Clear TCP checksum
			if (pAdapter->ToeTxEn)
			{
				ali_desc_hw.tx_sts.hw.CoeEn = 1;
				ali_desc_hw.tx_sts.hw.TcpPkt = 1;

				//ip_src = *(UINT32 *)(pHdr + IP_IPSRC_OFF);
				//ip_des = *(UINT32*)(pHdr + IP_IPDES_OFF);	

				ip_src = (pHdr + IP_IPSRC_OFF)[0]|((pHdr + IP_IPSRC_OFF)[1]<<8)|((pHdr + IP_IPSRC_OFF)[2]<<16)|((pHdr + IP_IPSRC_OFF)[3]<<24);
				ip_des = (pHdr + IP_IPDES_OFF)[0]|((pHdr + IP_IPDES_OFF)[1]<<8)|((pHdr + IP_IPDES_OFF)[2]<<16)|((pHdr + IP_IPDES_OFF)[3]<<24);

				pHdr += (IpHdrLen*4);

				ChkLen = len - ETH_HLEN - (IpHdrLen*4);
				acc = standard_chksum(pHdr, ChkLen);

				while ((acc >> 16) != 0)
				{
					acc = (acc & 0xffff) + (acc >> 16);
				}

				acc += (ip_src & 0xFFFFUL);
				acc += ((ip_src>>16) & 0xFFFFUL);

				acc += (ip_des & 0xFFFFUL);
				acc += ((ip_des>>16) & 0xFFFFUL);

				acc += (ip_protocol & 0xFF00);
				acc += SWAP(ChkLen);;

				while ((acc >> 16) != 0)
				{
					acc = (acc & 0xffffUL) + (acc >> 16);
				}

				ChsAcc =(UINT16)~acc;
				ChsOff = ETH_HLEN + (IpHdrLen*4) + 16;
			}
			break;

		case MAC_FRAME_PPPoE_IP_ICMP:
			*(UINT16 *)pHdr = SWAP(ETH_P_PPP_SES); 
			pHdr += 2; // 14 now.

			*(UINT16 *)(pHdr+0) = 0x0011; //PPPoE version & type.
			tmp_u16  = len - ETH_HLEN - PPPOE_HDR_LEN + 2;
			*(UINT16 *)(pHdr+4) = SWAP(tmp_u16); 
			*(UINT16 *)(pHdr+6) = SWAP(PPPOE_IP);
			pHdr += 8; // 22 now.
			IpStartOff += 10; // 22 now.

			*(UINT16 *)(pHdr+0) = (0x0040 |IpHdrLen); //Version HeaderLength
			tmp_u16  = len - ETH_HLEN - PPPOE_HDR_LEN;
			*(UINT16 *)(pHdr+2) = SWAP(tmp_u16); //Tot Len.	
			*(UINT16 *)(pHdr+6) = 0x0000; //Offset.
			*(UINT16 *)(pHdr+8) = 0x0180; //Ttl(128) & Protocol(ICMP:0x01).
			*(UINT16 *)(pHdr+10) = 0x0000; //IP Checksum.

			pHdr += (IpHdrLen*4);
			*(UINT16 *)(pHdr+2) = 0x0000; //Clear ICMP/IGMP checksum
			if (pAdapter->ToeTxEn)
				ali_desc_hw.tx_sts.hw.CoeEn = 1;
			break;	

		case MAC_FRAME_PPPoE_IP_IGMP:
			*(UINT16 *)pHdr = SWAP(ETH_P_PPP_SES); 
			pHdr += 2; // 14 now.	

			*(UINT16 *)(pHdr+0) = 0x0011; //PPPoE version & type.
			tmp_u16  = len - ETH_HLEN - PPPOE_HDR_LEN + 2;
			*(UINT16 *)(pHdr+4) = SWAP(tmp_u16); 		
			*(UINT16 *)(pHdr+6) = SWAP(PPPOE_IP);
			pHdr += 8; // 22 now.	
			IpStartOff += 10;

			*(UINT16 *)(pHdr+0) = (0x0040 |IpHdrLen); //Version HeaderLength
			tmp_u16  = len - ETH_HLEN - PPPOE_HDR_LEN;
			*(UINT16 *)(pHdr+2) = SWAP(tmp_u16); //Tot Len.	
			*(UINT16 *)(pHdr+6) = 0x0000; //Offset.
			*(UINT16 *)(pHdr+8) = 0x0280; //Ttl(128) & Protocol(IGMP:0x02).
			*(UINT16 *)(pHdr+10) = 0x0000; //IP Checksum.

			pHdr += (IpHdrLen*4);
			*(UINT16 *)(pHdr+2) = 0x0000; //Clear ICMP/IGMP checksum		
			if (pAdapter->ToeTxEn)
				ali_desc_hw.tx_sts.hw.CoeEn = 1;
			break;

		case MAC_FRAME_PPPoE_IP_UDP:
			*(UINT16 *)pHdr = SWAP(ETH_P_PPP_SES); 
			pHdr += 2; // 14 now.

			*(UINT16 *)(pHdr+0) = 0x0011; //PPPoE version & type.
			tmp_u16  = len - ETH_HLEN - PPPOE_HDR_LEN + 2;
			*(UINT16 *)(pHdr+4) = SWAP(tmp_u16); 
			*(UINT16 *)(pHdr+6) = SWAP(PPPOE_IP);
			pHdr += 8; // 22 now.	
			IpStartOff += 10;

			*(UINT16 *)(pHdr+0) = (0x0040 |IpHdrLen); //Version HeaderLength
			tmp_u16  = len - ETH_HLEN - PPPOE_HDR_LEN;
			*(UINT16 *)(pHdr+2) = SWAP(tmp_u16); //Tot Len.	
			*(UINT16 *)(pHdr+6) = 0x0000; //Offset.
			ip_protocol = 0x1180;
			*(UINT16 *)(pHdr+8) = ip_protocol; //Ttl(128) & Protocol(UDP:0x11).
			*(UINT16 *)(pHdr+10) = 0x0000; //IP Checksum.

			*(UINT16 *)(pHdr+(IpHdrLen*4)+6) = 0x0000; //Clear UDP checksum		 
			if (pAdapter->ToeTxEn)
			{
				ali_desc_hw.tx_sts.hw.CoeEn = 1;
				ali_desc_hw.tx_sts.hw.UdpPkt = 1;

				//ip_src = *(UINT32 *)(pHdr + IP_IPSRC_OFF);
				//ip_des = *(UINT32*)(pHdr + IP_IPDES_OFF);	

				ip_src = (pHdr + IP_IPSRC_OFF)[0]|((pHdr + IP_IPSRC_OFF)[1]<<8)|((pHdr + IP_IPSRC_OFF)[2]<<16)|((pHdr + IP_IPSRC_OFF)[3]<<24);
				ip_des = (pHdr + IP_IPDES_OFF)[0]|((pHdr + IP_IPDES_OFF)[1]<<8)|((pHdr + IP_IPDES_OFF)[2]<<16)|((pHdr + IP_IPDES_OFF)[3]<<24);

				pHdr += (IpHdrLen*4);
				ChkLen = len - ETH_HLEN - PPPOE_HDR_LEN - (IpHdrLen*4);
				acc = standard_chksum(pHdr, ChkLen);

				while ((acc >> 16) != 0)
				{
					acc = (acc & 0xffff) + (acc >> 16);
				}

				acc += (ip_src & 0xFFFFUL);
				acc += ((ip_src>>16) & 0xFFFFUL);

				acc += (ip_des & 0xFFFFUL);
				acc += ((ip_des>>16) & 0xFFFFUL);

				acc += (ip_protocol & 0xFF00);
				acc += SWAP(ChkLen);

				while ((acc >> 16) != 0)
				{
					acc = (acc & 0xffffUL) + (acc >> 16);
				}

				ChsAcc =(UINT16)~acc;
				ChsOff = ETH_HLEN + PPPOE_HDR_LEN + (IpHdrLen*4) + 6;
			}
			break;

		case MAC_FRAME_PPPoE_IP_TCP:
			*(UINT16 *)pHdr = SWAP(ETH_P_PPP_SES); 
			pHdr += 2; // 14 now.

			*(UINT16 *)(pHdr+0) = 0x0011; //PPPoE version & type.
			tmp_u16  = len - ETH_HLEN - PPPOE_HDR_LEN + 2;
			*(UINT16 *)(pHdr+4) = SWAP(tmp_u16); 
			*(UINT16 *)(pHdr+6) = SWAP(PPPOE_IP);
			pHdr += 8; // 22 now.	
			IpStartOff += 10;

			*(UINT16 *)(pHdr+0) = (0x0040 |IpHdrLen); //Version HeaderLength
			tmp_u16  = len - ETH_HLEN - PPPOE_HDR_LEN;
			*(UINT16 *)(pHdr+2) = SWAP(tmp_u16); //Tot Len.	
			*(UINT16 *)(pHdr+6) = 0x0000; //Offset.
			ip_protocol = 0x0680; 
			*(UINT16 *)(pHdr+8) = ip_protocol; //Ttl(128) & Protocol(TCP:0x06).
			*(UINT16 *)(pHdr+10) = 0x0000; //IP Checksum.

			*(UINT16 *)(pHdr+(IpHdrLen*4)+16) = 0x0000; //Clear TCP checksum
			if (pAdapter->ToeTxEn)
			{
				ali_desc_hw.tx_sts.hw.CoeEn = 1;
				ali_desc_hw.tx_sts.hw.TcpPkt = 1;

				//ip_src = *(UINT32 *)(pHdr + IP_IPSRC_OFF);
				//ip_des = *(UINT32*)(pHdr + IP_IPDES_OFF);	

				ip_src = (pHdr + IP_IPSRC_OFF)[0]|((pHdr + IP_IPSRC_OFF)[1]<<8)|((pHdr + IP_IPSRC_OFF)[2]<<16)|((pHdr + IP_IPSRC_OFF)[3]<<24);
				ip_des = (pHdr + IP_IPDES_OFF)[0]|((pHdr + IP_IPDES_OFF)[1]<<8)|((pHdr + IP_IPDES_OFF)[2]<<16)|((pHdr + IP_IPDES_OFF)[3]<<24);

				pHdr += (IpHdrLen*4);
				ChkLen = len - ETH_HLEN - PPPOE_HDR_LEN - (IpHdrLen*4);
				acc = standard_chksum(pHdr, ChkLen);

				while ((acc >> 16) != 0)
				{
					acc = (acc & 0xffff) + (acc >> 16);
				}

				acc += (ip_src & 0xFFFFUL);
				acc += ((ip_src>>16) & 0xFFFFUL);

				acc += (ip_des & 0xFFFFUL);
				acc += ((ip_des>>16) & 0xFFFFUL);

				acc += (ip_protocol & 0xFF00);
				acc += SWAP(ChkLen);

				while ((acc >> 16) != 0)
				{
					acc = (acc & 0xffffUL) + (acc >> 16);
				}

				ChsAcc =(UINT16)~acc;
				ChsOff = ETH_HLEN + PPPOE_HDR_LEN + (IpHdrLen*4) + 16;	
			}
			break;

		default:
			TOE2_UTILITY_TRACE("FrameType(%d) Unknow.\n", ali_tx_para.FrameType);
			break;	
	}	

	if (pAdapter->ToeTxEn)
		ali_desc_hw.tx_sts.hw.IpStartOff = IpStartOff;
}

static void MacTxStatus(PMAC_ADAPTER pAdapter, ptx_desc desc_sw)
{
	if ((desc_sw->tx_sts.sw.FS) && !(desc_sw->tx_sts.sw.OWN))
	{
		if (!(desc_sw->tx_sts.sw.ES))
		{
			pAdapter->net_stats.tx_packets++;
		}
		else
		{ 
			pAdapter->net_stats.tx_errors++;

			if ((desc_sw->tx_sts.sw.LossOfCarrier) ||(desc_sw->tx_sts.sw.NoCarrier))
				pAdapter->net_stats.tx_carrier_errors++;
			if (desc_sw->tx_sts.sw.LateCol) 
				pAdapter->net_stats.tx_window_errors++;
			if (desc_sw->tx_sts.sw.FifoUnderrun) 
				pAdapter->net_stats.tx_fifo_errors++;
			if (desc_sw->tx_sts.sw.HF) 
				pAdapter->net_stats.tx_heartbeat_errors++;
		}

		if (desc_sw->tx_sts.sw.ExCol) 
		{
			pAdapter->mac_stats.tx_col_errors++;
		}
		else
		{
			pAdapter->mac_stats.tx_col_cnts[desc_sw->tx_sts.sw.ColCnt]++;
		}
	}
}

static void TransmitConfig(PMAC_ADAPTER pAdapter, UINT16 WPtr, UINT16 Off, UINT32 Size)
{
    volatile char wait_buffer[TOE2_DESC_SZ] = {0};
	ptx_desc DescSw = &pAdapter->pTxDesc[WPtr];

	MacTxStatus(pAdapter, DescSw);

	ali_desc_hw.seg_len = Size;

	TOE2_UTILITY_TRACE("TransmitConfig: Off/Size = [%d,%d]", Off, Size);

	if (Off == 0)
	{
		ali_desc_hw.tx_sts.hw.OWN = 1;
		
		if (pAdapter->TsoEn || pAdapter->UfoEn)
		{
		    TOE2_UTILITY_TRACE("TransmitConfig: Tso/Ufo enabled");
			ali_desc_hw.tx_sts.hw.CoeEn = 1;
			ali_desc_hw.tx_sts.hw.Mfl = pAdapter->MflAuto;
			if ((ali_tx_para.FrameType == MAC_FRAME_IP_UDP) ||
				(ali_tx_para.FrameType == MAC_FRAME_PPPoE_IP_UDP))
			{
				ali_desc_hw.tx_sts.hw.UdpPkt = 1;
			}
			else if ((ali_tx_para.FrameType == MAC_FRAME_IP_TCP) ||
				(ali_tx_para.FrameType == MAC_FRAME_PPPoE_IP_TCP))
			{
				ali_desc_hw.tx_sts.hw.TcpPkt = 1;	
			}
			else
			{
				TOE2_UTILITY_TRACE("TransmitConfig: Tso/Ufo Pkt Type err");
				while(1);//asm("sdbbp");
			}
		}
		else if (pAdapter->ToeTxEn)
		{
		    TOE2_UTILITY_TRACE("TransmitConfig: ToeTxEn enabled");
			ali_desc_hw.tx_sts.hw.CoeEn = 1;
			ali_desc_hw.tx_sts.hw.Mfl = TOE2_MSS;//TOE2_BUF_SZ;
			if ((ali_tx_para.FrameType == MAC_FRAME_IP_UDP) ||
				(ali_tx_para.FrameType == MAC_FRAME_PPPoE_IP_UDP))
				ali_desc_hw.tx_sts.hw.UdpPkt = 1;
			else if ((ali_tx_para.FrameType == MAC_FRAME_IP_TCP) ||
				(ali_tx_para.FrameType == MAC_FRAME_PPPoE_IP_TCP))
				ali_desc_hw.tx_sts.hw.TcpPkt = 1;	
			else
			{
				TOE2_UTILITY_TRACE("TransmitConfig: Tx Coe Pkt Type err");
				while(1);//asm("sdbbp");
			}
		}
		else
		{
		    TOE2_UTILITY_TRACE("TransmitConfig: NO ToeTxEn");
			//ali_desc_hw.tx_sts.hw.Mfl = TOE2_BUF_SZ - 18;
		}
	}

	if (WPtr == MAC_TX_DESC_NUM - 1)
		ali_desc_hw.tx_sts.hw.EOR = 1;
	else
		ali_desc_hw.tx_sts.hw.EOR = 0;

	ali_desc_hw.pkt_buf_dma = pAdapter->TxDescBuf_DMA[WPtr] + Off;
	memcpy(&pAdapter->pTxDesc[WPtr], (unsigned char *)(&ali_desc_hw), sizeof(ali_desc_hw));

	memcpy((void *)wait_buffer, (void *)&pAdapter->pTxDesc[WPtr], TOE2_DESC_SZ);//HW dma wait
}

static int TransmitPackets(PMAC_ADAPTER pAdapter, UINT16 DescNum)
{
	UINT16 DescWritePtr, DescReadPtr;
	UINT16 Available;
	UINT16 Off, Desc;
	UINT32 ba = pAdapter->BaseAddr;

	do
	{
		DescReadPtr = TOE2_R16(TxRingDesRPtr);
		DescWritePtr = pAdapter->TxDescWPtr;

		if (DescReadPtr > DescWritePtr)
			Available = DescReadPtr - DescWritePtr - 1;
		else
			Available = MAC_TX_DESC_NUM + DescReadPtr - DescWritePtr - 1;

	} while(Available<DescNum);

	for (Off = 0, Desc = 0; Desc < DescNum; Desc++)
	{
		//?? Need to check further more		
		//memset(&ali_desc_hw, 0, sizeof(tx_desc_hw));

		ali_desc_hw.tx_sts.hw.OWN = 0;
		ali_desc_hw.tx_sts.hw.FS = 0;
		ali_desc_hw.tx_sts.hw.LS = 0;
		ali_desc_hw.tx_sts.hw.Mfl = 0;
		ali_desc_hw.tx_sts.hw.CoeEn = 0;
		ali_desc_hw.tx_sts.hw.UdpPkt = 0;
		ali_desc_hw.tx_sts.hw.TcpPkt = 0;

		if (Desc == 0)
		{
			ali_desc_hw.tx_sts.hw.FS = 1;
		}

		if (Desc == (DescNum -1))
		{
			ali_desc_hw.tx_sts.hw.LS = 1;
		}

		TransmitConfig(pAdapter, pAdapter->TxDescWPtr, Off, ali_desc_size[Desc]);

		Off += ali_desc_size[Desc];

		if ((++pAdapter->TxDescWPtr) >= MAC_TX_DESC_NUM)
			pAdapter->TxDescWPtr = 0;
	}

	TOE2_W16(TxRingDesWPtr, pAdapter->TxDescWPtr);
	TOE2_UTILITY_TRACE("TxDescWPtr: %d", pAdapter->TxDescWPtr);

	pAdapter->TxCrcErrs = TOE2_R16(0x30/*CRCERR*/);

	return TRUE;
}

static void TransmitFrames(PMAC_ADAPTER pAdapter, UINT32 DescNum, UINT32 DescLen, UINT32 DescTimes)
{
	unsigned int bytes, rand, i;
	int times;

	ali_desc_hw.tot_len = DescLen;
	ali_desc_hw.seg_num= DescNum;

	times = DescTimes;

	while (times--)
	{
		if (m_bTxStopThread)
		{
			break;
		}

		//TOE2_UTILITY_TRACE("DescSize(%d): ", DescNum);
		//sprintf(ali_mem_print_buffer+ali_mem_print_buffer_offset, "DescSize(%4d): ", DescNum);
		//ali_mem_print_buffer_offset += 16;
		bytes = DescLen;
		for (i = 0; i < (DescNum - 1); i++)
		{
			//rand = ali_uart_mac_get_rand(bytes-(DescNum - i -1));
			rand = 1 + get_random_int()%(bytes-(DescNum-i));
			ali_desc_size[i] = rand;
			//TOE2_UTILITY_TRACE("(%d)->", rand);
			//sprintf(ali_mem_print_buffer+ali_mem_print_buffer_offset, "(%4d)->", rand);
			//ali_mem_print_buffer_offset += 8;
			bytes -= rand;
			//TOE2_UTILITY_TRACE("TransmitFrames segment[%d] size = %d.", i, ali_desc_size[i]);
		}

		ali_desc_size[DescNum - 1] = bytes;
		//TOE2_UTILITY_TRACE("TransmitFrames segment[%d] size = %d.", DescNum - 1, ali_desc_size[DescNum - 1]);
		//sprintf(ali_mem_print_buffer+ali_mem_print_buffer_offset, "(%4d)\n", bytes);
		//ali_mem_print_buffer_offset += 7;

		TransmitPackets(pAdapter, DescNum);
		//msleep(1);
	}
}

static void MacTxTest(struct work_struct *work)
{
	unsigned long DescTimes, DescLen, DescFrom, DescTo, Desc;
	long j;
	PMAC_ADAPTER pAdapter;

	//daemonize("MacTxTest");
	while (1)
	{
		while (0 == m_bTxThreadStart)
		{	
			//TOE2_UTILITY_TRACE("tx task hold: \n");
			msleep(100);
		}

		//pAdapter = (PMAC_ADAPTER)params;
		pAdapter = &util_mac_adapter;

		TOE2_UTILITY_TRACE("tx task start:\n");
		DescTimes = ali_tx_para.DescTimes;
		DescLen = ali_tx_para.DescLen;
		DescFrom = ali_tx_para.DescFrom;
		DescTo = ali_tx_para.DescTo;

        TOE2_UTILITY_TRACE("MacTxTest DescTimes/DescLen = [%d,%d]", DescTimes, DescLen);
        TOE2_UTILITY_TRACE("MacTxTest DescFrom/DescTo = [%d,%d]", DescFrom, DescTo);
		TOE2_UTILITY_TRACE("MacTxTest UfoEn/TsoEn = [%d,%d]", pAdapter->UfoEn, pAdapter->TsoEn);

		if (pAdapter->UfoEn || pAdapter->TsoEn)
		{
		    TOE2_UTILITY_TRACE("MacTxTest in UfoEn/TsoEn mode");
			//Fill in Frame Buffer.
			for (j=0; j<MAX_TOE2_PKT_LEN; j++)
			{
				ToeTxBuf[j] = (unsigned char)j;
			}

			if (DescLen > MAX_TOE2_PKT_LEN)
			{
				DescLen = MAX_TOE2_PKT_LEN;
			}

			MacHeaderBuild(pAdapter, &ToeTxBuf[0], DescLen);

			for (pAdapter->MflAuto = pAdapter->MinMfl; pAdapter->MflAuto <= pAdapter->MaxMfl; pAdapter->MflAuto++)
			{
			    TOE2_UTILITY_TRACE("MacTxTest -> Max Frame Length = %d:", pAdapter->MflAuto);
				for (j=0; j<MAC_TX_DESC_NUM; j++)
				{
					//load_in_mem(pAdapter->TxDescBuf[j], &ToeTxBuf[0], MAX_TOE_PKT_LEN);
					memcpy((unsigned char*)(pAdapter->TxDescBuf[j]), &ToeTxBuf[0], MAX_TOE2_PKT_LEN);
				}

				//ALiCrcChk(&MacTxBuf[0], Adapter.MflAuto);

				for (Desc = DescFrom; Desc <= DescTo; Desc++)
				{
				    TOE2_UTILITY_TRACE("MacTxTest -> TransmitFrames segment occupied descriptor num = %d", Desc);
					TransmitFrames(pAdapter, Desc, DescLen, DescTimes);
					TOE2_UTILITY_TRACE("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}
				TOE2_UTILITY_TRACE("================================one MFL Set end================================\n");
			}
		}
		else
		{
		    TOE2_UTILITY_TRACE("MacTxTest NOT in UfoEn/TsoEn mode");
			//Fill in Frame Buffer.
			for (j=0; j<TOE2_BUF_SZ; j++)
			{
				MacTxBuf[j] = (unsigned char)j;
			}

			if (DescLen > (TOE2_BUF_SZ-18))    //TOE2_MSS or 1518:HW LIMIT
			{
				DescLen = TOE2_BUF_SZ - 18;
			}

			if (pAdapter->TxLenAutoInc)
			{
			    TOE2_UTILITY_TRACE("MacTxTest in TxLenAutoInc");
				for (pAdapter->TxLenAuto = DescLen; pAdapter->TxLenAuto <= TOE2_BUF_SZ; pAdapter->TxLenAuto++)
				{
				    TOE2_UTILITY_TRACE("MacTxTest -> TxLenAuto = %d:", pAdapter->TxLenAuto);
					MacHeaderBuild(pAdapter, &MacTxBuf[0], pAdapter->TxLenAuto);

					for (j=0; j<MAC_TX_DESC_NUM; j++)
					{
						//load_in_mem(pAdapter->TxDescBuf[j], &MacTxBuf[0], MAX_ETH_PKT_LEN);
						memcpy((unsigned char*)(pAdapter->TxDescBuf[j]), &MacTxBuf[0], TOE2_BUF_SZ);
					}

					ALiCrcChk(pAdapter, &MacTxBuf[0], pAdapter->TxLenAuto);

					for (Desc = DescFrom; Desc <= DescTo; Desc++)
					{
					    TOE2_UTILITY_TRACE("MacTxTest -> TransmitFrames segment occupied descriptor num = %d", Desc);
						TransmitFrames(pAdapter, Desc, pAdapter->TxLenAuto, DescTimes);
						TOE2_UTILITY_TRACE("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
					}
					TOE2_UTILITY_TRACE("================================one FRAME end================================\n");
				}
			}
			else
			{
			    TOE2_UTILITY_TRACE("MacTxTest NOT in TxLenAutoInc");
				MacHeaderBuild(pAdapter, &MacTxBuf[0], DescLen);

				for (j=0; j<MAC_TX_DESC_NUM; j++)
				{
					//load_in_mem(pAdapter->TxDescBuf[j], &MacTxBuf[0], MAX_ETH_PKT_LEN);
					memcpy((unsigned char*)(pAdapter->TxDescBuf[j]), &MacTxBuf[0], TOE2_BUF_SZ);
				}

				ALiCrcChk(pAdapter, &MacTxBuf[0], DescLen);

				for (Desc = DescFrom; Desc <= DescTo; Desc++)
				{
				    TOE2_UTILITY_TRACE("MacTxTest -> TransmitFrames segment occupied descriptor num = %d", Desc);
					TransmitFrames(pAdapter, Desc, DescLen, DescTimes);
					TOE2_UTILITY_TRACE("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}
			}
		}

		m_bTxStopThread = 1;
		//complete_and_exit(0, 0);
		TOE2_UTILITY_TRACE("tx test %d done\n", DescTimes);
		m_bTxThreadStart = 0;
	}
	//while(1);
	//return 0;
}

struct workqueue_struct *util_tx_wq;
struct work_struct util_tx_work;
void util_mac_tx_thread_create(void)
{
	//struct workqueue_struct *tx_wq;
	//struct work_struct tx_work;
	TOE2_UTILITY_TRACE("allocate work queue for tx thread");
	util_tx_wq = create_workqueue("MacTxTest");
	if (!util_tx_wq)
	{    
		printk("[ERR] Failed to allocate tx work queue\n");
	}    
	else 
	{    
		INIT_WORK(&util_tx_work, MacTxTest);
		queue_work(util_tx_wq, &util_tx_work);
	}    
}

void util_mac_tx_thread_destroy(void)
{
	destroy_workqueue(util_tx_wq);
}

static void MacTxStart(PMAC_ADAPTER pAdapter)
{
	UINT8 TmpChar;
	UINT32 i;
	UINT32 ba = pAdapter->BaseAddr;

	pAdapter->net_stats.tx_packets = 0;
	pAdapter->net_stats.tx_bytes = 0;
	pAdapter->net_stats.tx_errors = 0;
	pAdapter->net_stats.tx_carrier_errors = 0;
	pAdapter->net_stats.tx_fifo_errors = 0;
	pAdapter->net_stats.tx_heartbeat_errors = 0;
	pAdapter->net_stats.tx_window_errors = 0;

	pAdapter->mac_stats.tx_col_errors = 0;

	for (i = 0; i < 16; i ++)	
		pAdapter->mac_stats.tx_col_cnts[i] = 0;

	TmpChar = TOE2_R8(SCR);
	TmpChar &= ~(SCRTxCoeEn|SCRTsoEn|SCRUfoEn);

	if (pAdapter->ToeTxEn)
		TmpChar |= SCRTxCoeEn;
	if (pAdapter->TsoEn)
		TmpChar |= (SCRTsoEn|SCRTxCoeEn);
	if (pAdapter->UfoEn)
		TmpChar |= (SCRUfoEn|SCRTxCoeEn);	

	TOE2_W8(SCR, (TmpChar|(UINT8)SCRTxEn));	
}

int util_mac_tx_start(PMAC_Tx_Context pTxContext)
{
	//ali_tx_para = pTxContext->TxPara;
	memcpy(&ali_tx_para, &(pTxContext->TxPara), sizeof(ali_tx_para));

	//Tx Settings.
	util_mac_adapter.ToeTxEn = pTxContext->ToeTxEn;
	util_mac_adapter.TsoEn = pTxContext->TsoEn;
	util_mac_adapter.UfoEn = pTxContext->UfoEn;
	util_mac_adapter.TxLenAutoInc = pTxContext->TxLenAutoInc;
	util_mac_adapter.MinMfl = pTxContext->MinMfl;
	util_mac_adapter.MaxMfl = pTxContext->MaxMfl;

	memcpy(&MacDstAddr[0], &(pTxContext->MacDstAddr[0]), 6);
	memcpy(&MacSrcAddr[0], &(pTxContext->MacSrcAddr[0]), 6);

#ifdef UTIL_DEBUG
    ali_tx_para.FrameType = MAC_FRAME_IP_TCP;
    ali_tx_para.DescFrom = 1;
    ali_tx_para.DescTo = 16;
	ali_tx_para.DescLen = 8*1024;//1536;
	ali_tx_para.DescTimes = 2;
	util_mac_adapter.ToeTxEn = 1;//0;
	util_mac_adapter.TsoEn = 1;//0;
	util_mac_adapter.MinMfl = 100;
	util_mac_adapter.MaxMfl = 108;
	
	MacDstAddr[0] = 0xFF; MacDstAddr[1] = 0xFF; MacDstAddr[2] = 0xFF;
	MacDstAddr[3] = 0xFF; MacDstAddr[4] = 0xFF; MacDstAddr[5] = 0xFF;
	
	MacSrcAddr[0] = 0x00; MacSrcAddr[1] = 0x12; MacSrcAddr[2] = 0x34;
	MacSrcAddr[3] = 0x56; MacSrcAddr[4] = 0x78; MacSrcAddr[5] = 0x9A;
#endif
	
	TOE2_UTILITY_TRACE("[T] tx ali_tx_para FrameType: %d", ali_tx_para.FrameType);
	TOE2_UTILITY_TRACE("[T] tx ali_tx_para DescFrom: %d", ali_tx_para.DescFrom);
	TOE2_UTILITY_TRACE("[T] tx ali_tx_para DescTo: %d", ali_tx_para.DescTo);
	TOE2_UTILITY_TRACE("[T] tx ali_tx_para DescLen: %d", ali_tx_para.DescLen);
	TOE2_UTILITY_TRACE("[T] tx ali_tx_para DescTimes: %d", ali_tx_para.DescTimes);
	TOE2_UTILITY_TRACE("[T] tx ali_tx_para AddVlanTag: %d", ali_tx_para.AddVlanTag);
	TOE2_UTILITY_TRACE("[T] tx ali_tx_para TxVlanTag: %d", ali_tx_para.TxVlanTag);
	TOE2_UTILITY_TRACE("[T] TxContext ToeTxEn: %d", util_mac_adapter.ToeTxEn);
	TOE2_UTILITY_TRACE("[T] TxContext TsoEn: %d", util_mac_adapter.TsoEn);
	TOE2_UTILITY_TRACE("[T] TxContext UfoEn: %d", util_mac_adapter.UfoEn);
	TOE2_UTILITY_TRACE("[T] TxContext TxLenAutoInc: %d", util_mac_adapter.TxLenAutoInc);
	TOE2_UTILITY_TRACE("[T] TxContext MinMfl: %d", util_mac_adapter.MinMfl);
	TOE2_UTILITY_TRACE("[T] TxContext MaxMfl: %d", util_mac_adapter.MaxMfl);
	
	m_bTxStopThread = 0;

	TOE2_UTILITY_TRACE("util_mac_tx_start start...");
	MacTxStart(&util_mac_adapter);

	m_bTxThreadStart = 1;

	//kernel_thread(MacTxTest, (void *)(&util_mac_adapter), 0);
	/*if (mac_tx_thread_status < 0)
	  {
	  asm("sdbbp");
	  }*/
	return 0;
}

void util_mac_tx_stop(void)
{
	m_bTxStopThread = 1;
}

void util_mac_status(PMAC_Status_Context cxt, PMAC_ADAPTER pAdapter)
{
	//cxt->net_stats = pAdapter->net_stats;
	memcpy(&(cxt->net_stats), &(pAdapter->net_stats), sizeof(struct net_device_stats));
	//cxt->mac_stats = pAdapter->mac_stats;
	memcpy(&(cxt->mac_stats), &(pAdapter->mac_stats), sizeof(struct toe2_device_stats));
	cxt->TxLenAuto = pAdapter->TxLenAuto;
	cxt->TxCrcErrs = pAdapter->TxCrcErrs;

#ifdef UTIL_DEBUG
	int i;
	TOE2_UTILITY_TRACE("[T] Status TxLenAuto: %d", cxt->TxLenAuto);
	TOE2_UTILITY_TRACE("[T] Status TxCrcErrs: %d\n", cxt->TxCrcErrs);

	TOE2_UTILITY_TRACE("[T] Status net_stats rx_packets: %d", cxt->net_stats.rx_packets);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_packets: %d", cxt->net_stats.tx_packets);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_bytes: %d", cxt->net_stats.rx_bytes);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_bytes: %d", cxt->net_stats.tx_bytes);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_errors: %d", cxt->net_stats.rx_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_errors: %d", cxt->net_stats.tx_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_dropped: %d", cxt->net_stats.rx_dropped);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_dropped: %d", cxt->net_stats.tx_dropped);
	TOE2_UTILITY_TRACE("[T] Status net_stats multicast: %d", cxt->net_stats.multicast);
	TOE2_UTILITY_TRACE("[T] Status net_stats collisions: %d", cxt->net_stats.collisions);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_length_errors: %d", cxt->net_stats.rx_length_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_over_errors: %d", cxt->net_stats.rx_over_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_crc_errors: %d", cxt->net_stats.rx_crc_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_frame_errors: %d", cxt->net_stats.rx_frame_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_fifo_errors: %d", cxt->net_stats.rx_fifo_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_missed_errors: %d", cxt->net_stats.rx_missed_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_aborted_errors: %d", cxt->net_stats.tx_aborted_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_carrier_errors: %d", cxt->net_stats.tx_carrier_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_fifo_errors: %d", cxt->net_stats.tx_fifo_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_heartbeat_errors: %d", cxt->net_stats.tx_heartbeat_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_window_errors: %d", cxt->net_stats.tx_window_errors);
	TOE2_UTILITY_TRACE("[T] Status net_stats rx_compressed: %d", cxt->net_stats.rx_compressed);
	TOE2_UTILITY_TRACE("[T] Status net_stats tx_compressed: %d\n", cxt->net_stats.tx_compressed);

	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_bc: %d", cxt->mac_stats.rx_bc);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_buf_empty: %d", cxt->mac_stats.rx_buf_empty);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_dribble_errors: %d", cxt->mac_stats.rx_dribble_errors);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_frag: %d", cxt->mac_stats.rx_frag);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_hdr_chs_errs: %d", cxt->mac_stats.rx_hdr_chs_errs);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_ip: %d", cxt->mac_stats.rx_ip);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_ip_chksum_errors: %d", cxt->mac_stats.rx_ip_chksum_errors);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_late_col_seen: %d", cxt->mac_stats.rx_late_col_seen);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_long_errors: %d", cxt->mac_stats.rx_long_errors);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_lost_in_ring: %d", cxt->mac_stats.rx_lost_in_ring);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_mc: %d", cxt->mac_stats.rx_mc);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_pay_chs_errs: %d", cxt->mac_stats.rx_pay_chs_errs);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_phy_layer_errors: %d", cxt->mac_stats.rx_phy_layer_errors);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_pppoe: %d", cxt->mac_stats.rx_pppoe);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_runt_errors: %d", cxt->mac_stats.rx_runt_errors);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_tcp_chksum_errors: %d", cxt->mac_stats.rx_tcp_chksum_errors);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_uc: %d", cxt->mac_stats.rx_uc);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_vlan: %d", cxt->mac_stats.rx_vlan);
	TOE2_UTILITY_TRACE("[T] Status mac_stats rx_wd_timeout_errors: %d", cxt->mac_stats.rx_wd_timeout_errors);
	for (i=0; i<16; i++)
	{
		TOE2_UTILITY_TRACE("[T] Status   mac_stats tx_col_cnts: %d", cxt->mac_stats.tx_col_cnts[i]);
	}
	TOE2_UTILITY_TRACE("[T] Status mac_stats tx_col_errors: %d\n", cxt->mac_stats.tx_col_errors);
#endif
}

#endif

