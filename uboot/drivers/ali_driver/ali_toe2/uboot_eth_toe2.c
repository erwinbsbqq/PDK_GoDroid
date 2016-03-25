/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    ethernet_mac.c
*
*    Description:    Ethernetnet Mac Soc Driver
*
*    History:
*		Date			Athor		Version		Reason
*	    =======================================================
*     1.	19.02.2008	Mao Feng				Original MAC
*     2.	25.02.2008	Mao Feng	Ver 1.0		Add mii & net interface.
*	  3.	23.04.2009				Ver 2.0		Cost Down MAC
******************************************************************************/

#include <common.h>
#include <linux/string.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>

#include "types_def.h"
#include "net.h"
#include "ethtool.h"
#include "if.h"
#include "net_dev.h"

#include "eth_mii.h"
#include "uboot_eth_reg.h"
#include "uboot_eth_toe2.h"

//#define ALI_DEBUG_ON
#ifdef ALI_DEBUG_ON
    #define ALI_PRINTF   printf
#else
    #define ALI_PRINTF(...)	do{}while(0)
#endif

static UINT8 g_rx_ring_desc[TOE2_RX_DESC_NUM*TOE2_DESC_SZ+0x20];
static UINT8 g_rx_ring_buff[TOE2_RX_DESC_NUM*RX_BUF_SIZE+0x20];
static UINT8 g_tx_ring_desc[TOE2_TX_DESC_NUM*TOE2_DESC_SZ+0x20];
static UINT8 g_tx_ring_buff[TX_BUF_SIZE+0x20];
////static UINT8 g_setup_frame_buf[SETUP_FRAME_SIZE+0x20];

static struct net_device *g_net_dev = 0;
static struct mac_adapter *g_mac_adapter = 0;
static UINT32 toe2_timer_frq = 0x18fffff; //50M

//#define GEN_32BYTES_ALIGN(x) ((((u32)(x))+0x1f)&0xFFFFFFE0)
//#define __CTDADDRALI(x) (((u32)(x))&0x7FFFFFFF)
//static inline int printk(const char *s, ...)
//	__attribute__ ((format (printf, 1, 2)));

#define SDBBP() asm volatile(".word 0x7000003f; nop")

#define	mac_mdio_delay(mdio_addr)	MAC_R32(mdio_addr)

static BOOL mac_receive_one_packet(pmac_adapter padapter);
//static void phy_chip_reset(void);
static int m3602_eth_init(struct eth_device *dev,bd_t *bis);

RET_CODE eth_mac_get_info(struct net_device *dev, UINT32 info_type, void *info_buf)
{
	pmac_adapter padapter = (pmac_adapter)dev->priv;
	UINT32 base_addr = padapter->io_base;
	UINT32 ret = SUCCESS;

	MAC_PRINTF("eth_mac_get_info: Net device get information, type = %d.\n", info_type);

	switch(info_type)
	{
		case NET_GET_CURRENT_MAC :
		{
			memcpy((UINT8 *)info_buf, (UINT8 *)padapter->mac_addr, ETH_ALEN);
			break;
		}
		
		case NET_GET_PERMANENT_MAC :
		{
			memcpy((UINT8 *)info_buf, (UINT8 *)padapter->mac_addr, ETH_ALEN);
			break;
		}
		
		case NET_GET_LINK_STATUS :
		{
			if (padapter->link_established)
			{
				*((unsigned char*)info_buf) = NET_LINK_CONNECTED;
			}
			else
			{
				*((unsigned char*)info_buf) = NET_LINK_DISCONNECTED;
			}
			break;
		}
		
		case NET_GET_LINK_SPEED :
		{
			if (padapter->link_established)
			{
				if (padapter->link_speed == (UINT32)10)
				{
					*((unsigned char*)info_buf) = NET_LINK_10MBPS;
				}
				else if (padapter->link_speed == (UINT32)100)
				{
					*((unsigned char*)info_buf) = NET_LINK_100MBPS;
				}
				else
				{
					*((unsigned char*)info_buf) = NET_LINK_100MBPS;
					MAC_PRINTF("eth_mac_get_info: unknown link speed.\n");
				}
			}
			else
			{
				*((unsigned char*)info_buf) = NET_LINK_100MBPS;
				MAC_PRINTF("eth_mac_get_info: Get link speed but link is not established yet.\n");
			}
			break;
		}
		
		case NET_GET_LINK_MODE :
		{
			if (padapter->link_established)
			{
				if (padapter->duplex_mode)
				{
					*((unsigned char*)info_buf) = NET_LINK_FULL;
				}
				else
				{
					*((unsigned char*)info_buf) = NET_LINK_HALF;
				}
			}
			else
			{
				*((unsigned char*)info_buf) = NET_LINK_FULL;
				MAC_PRINTF("eth_mac_get_info: Get link mode but link is not established yet.\n");
			}
			break;
		}
		
		case NET_GET_CAPABILITY:
		{
			UINT32 *capability;
			capability = (UINT32*)info_buf;
			*capability = 1;
			break;
		}
		
		default :
		{
			ret = FAIL;
			break;
		}
	}
	
	return ret;
}

RET_CODE eth_mac_set_info(struct net_device *dev, UINT32 info_type, void *info_buf)
{
	pmac_adapter padapter = (pmac_adapter)dev->priv;
	UINT32 base_addr = padapter->io_base;
	UINT32 index;
	UINT8 *mac_addr;
	UINT32 ret = SUCCESS;

	//MAC_PRINTF("%s()=>Net device set information, type = %d.\n", __FUNCTION__, info_type);

	switch(info_type)
	{
		case NET_SET_OFFSET :
			padapter->iphdr_off = *(UINT8 *)info_buf;
			//MAC_PRINTF("%s()=>Set TOE offset to %d\n", __FUNCTION__, *(UINT8 *)info_buf);
			break;

		case NET_SET_LINK_SPEED :
		case NET_SET_LINK_MODE :
		case NET_ADD_MULTICAST_ADDR :
		case NET_DEL_MULTICAST_ADDR :
			MAC_PRINTF("%s()=>Command not supported yet.\n", __FUNCTION__);
			break;
			
		case NET_SET_MAC :
		{
			////set mac address must after attach() and before open()  
			if ((!info_buf) || (padapter->isr_registered))
			{
				return FAIL;
			}
			
			//set mac address.
			mac_addr = (UINT8 *)info_buf;
			for(index = 0; index < 6; index++)
			{
				padapter->mac_addr[index] = mac_addr[index];
			}
			
			MAC_PRINTF("%s(): Mac addr: %x-%x-%x-%x-%x-%x.\n ", __FUNCTION__, \
						padapter->mac_addr[0],padapter->mac_addr[1], \
						padapter->mac_addr[2],padapter->mac_addr[3], \
						padapter->mac_addr[4],padapter->mac_addr[5]);
			
			break;
		}			
			
		case NET_SET_CAPABILITY:
		{
			padapter->accelerator = 1;
			break;
		}
		default :
			ret = FAIL;
	}

	return ret;
}

void mac_chip_reset(UINT32 base_addr)
{
	UINT8 tmp_u8;
	
	MAC_PRINTF("mac_chip_reset: O");

	//reset device
	MAC_W8(SCR, SCRReset|MAC_R8(SCR));	

	//test if rest success ?
	do {
		MAC_PRINTF("->");	
		tmp_u8 = MAC_R8(SCR);
	}while(tmp_u8 & SCRReset);
	MAC_PRINTF("K!\n");
}

static void mac_hardware_start(pmac_adapter padapter)
{
	UINT8 tmp_u8;
	UINT16 tmp_u16;
	UINT32 tmp_u32;

	UINT32 duplex_mode = 0;
	UINT32 pause_frame = 0;
	UINT32 rmii_speed = 0;

	UINT32 base_addr = padapter->io_base;
	//int re = 0;

	MAC_PRINTF("%s in...\n", __FUNCTION__);

	//Disable TX RX
	MAC_W8(SCR, MAC_R8(SCR) & ~(SCRRxEn|SCRTxEn));

	//Config ClockDelayChain to exprience value 0xE 
	MAC_W8(ClkDelayChainSR, 0XE);
	
	//set mac address.
	MAC_W32(PAR, *((UINT32 *)((UINT32)padapter->mac_addr + 0)));
	MAC_W32(PAR - 4, *((UINT32 *)((UINT32)padapter->mac_addr + 4)));

	if (padapter->phy_mode == NET_PHY_RMII)
	{
		if (padapter->link_speed == (UINT32)100)
		{
			rmii_speed = (UINT32)RmiiCrSpeed;	//100Mbps
		}
		else
		{
			rmii_speed = (UINT32)0; //10Mbps
		}

		//Set RMII.
		tmp_u32 = MAC_R32(RmiiCR);
		tmp_u32 &= ~(RmiiCrSpeed);
		MAC_W32(RmiiCR, (tmp_u32|rmii_speed|RmiiEn));
	}

	if (padapter->duplex_mode)
	{
		duplex_mode = (UINT32)FullDuplexMode;
	}
	else
	{
		duplex_mode = (UINT32)0;
	}

	//config network operation mode.
	MAC_W32(NetworkOM, (duplex_mode |NetworkOMConfig));// |WorkMode_LoopBack));
	
	//test mux
	MAC_W8(0x58, 0x0F); 
	//MAC_W32(0x84, 0x5FF); 
	
	if (padapter->pause_frame_rx)
	{
		pause_frame |= (UINT32)RxFlowControlEn;
	}
	if (padapter->pause_frame_tx)
	{
		pause_frame |= (UINT32)TxFlowControlEn;
	}

	// 1. Increase IPG time to ensure the gap between 2 packets > mini IPG time
	// 2. decrease the first portion of the interframe gap time
	MAC_W8(TxRxCR1, 0xdf);
	MAC_W8(TxRxCR1+1, 0x00);

	//VLAN Patch 1/3 For receiving packet length 1517 & 1518, we need to add a patch here.
	tmp_u32 = MAC_R32(RxChkSumStartOff);
	tmp_u32 &= ~(0xfff);
	tmp_u32 |= 0x5f7;
	MAC_W32(RxChkSumStartOff, tmp_u32);

	tmp_u32 = MAC_R32(TxRxCR2);

	//VLAN Patch 2/3 For receiving packet length 1517 & 1518, we need to add a patch here.
	tmp_u32 |= RxMaxLenEn;
	
	tmp_u32 &= 0xf7ffffff;
	tmp_u32 |= 0x04000000;
	tmp_u32 &= ~TxFifoThMask;
	tmp_u32 |= (0x2<<TxFifoThOff);
	//tmp_u32 |= (0x6<<TxFifoThOff);
	//tmp_u32 |= (RxFifoSz|TxFifoSz)
	tmp_u32 &= ~RxFlowControlEn;
	tmp_u32 &= ~TxFlowControlEn;
#if(TOE2_VLAN_TAG_USED)
	if (padapter->vlan_tag_remove)
		MAC_W32(TxRxCR2, (pause_frame|tmp_u32|TxRxConfig2|RxRemoveVlanTagEn|VlanEn));
	else
		MAC_W32(TxRxCR2, (pause_frame|tmp_u32|TxRxConfig2|VlanEn));
#else
	MAC_W32(TxRxCR2, (pause_frame|tmp_u32 |TxRxConfig2));
#endif //TOE2_VLAN_TAG_USED

	MAC_W32(TSAD, padapter->tx_desc_dma);
	MAC_W32(RSAD, padapter->rx_desc_dma);

	MAC_W16(RxDesTotNum, TOE2_RX_DESC_NUM);
	MAC_W16(TxDesTotNum, TOE2_TX_DESC_NUM);

	MAC_W16(RxRingDesWPtr, TOE2_RX_DESC_NUM -1);
	MAC_W16(TxRingDesWPtr, 0);

	MAC_W32(TimerR, toe2_timer_frq);
	
	eth_mac_set_rx_mode(padapter->net_dev);
	/*re = mac_rx_refill(padapter);
	if(re)
	{
		asm("sdbbp");
	}*/

	tmp_u8 = (SCRRxEn|SCRTxEn);
	/*
	if(toe2_tso)
		tmp_u8 |= (SCRTxCoeEn|SCRTsoEn);
	if(toe2_ufo)
		tmp_u8 |= (SCRTxCoeEn|SCRUfoEn);
	if(toe2_tx_csum)
		tmp_u8 |= SCRTxCoeEn;	
	if(toe2_rx_csum)
		tmp_u8 |= SCRRxCoeEn;
	*/
	MAC_W8(SCR, tmp_u8);

	// Enable all surported interrupts.
	MAC_W32(IMR, padapter->interrupt_mask);
}

void mac_counter_init(pmac_adapter padapter)
{
	int	i; 

	padapter->cur_isr = 0;
	padapter->interrupt_mask = TOE2_INTERRUPT_MASK; 

	padapter->rx_wptr = TOE2_RX_DESC_NUM -1;
	padapter->rx_bptr= 0;
	padapter->tx_wptr= 0;	
	
	padapter->phy_reset = FALSE;
	padapter->auto_n_completed = FALSE;
	padapter->link_established = FALSE;
	padapter->transmit_okay = FALSE;	

	padapter->pause_frame_rx = FALSE;
	padapter->pause_frame_tx = FALSE;
}

//mdio preamble: 32 "1"s.
//provides synchronization for the PHY.
void mac_mdio_sync(pmac_adapter padapter)
{
	int i;
	//UINT32 tmp_u32;
	UINT32 base_addr = padapter->io_base;

	for (i = 32; i >= 0; i--)
	{
		MAC_W32(MiiMR1, MiiMdioWrite1|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
		MAC_W32(MiiMR1, MiiMdioWrite1|MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
    }
}

UINT32 mac_mdio_read(pmac_adapter padapter, UINT32 phy_addr, UINT32 reg_addr)
{
#ifdef HW_MDIO
	UINT32 base_addr = padapter->io_base;
	UINT16 tmp_16;
	UINT32 tmp_32;
	UINT32 addr;
	UINT32 cnt = 0;
	UINT32 time_out;
	
	addr = ((phy_addr << MiiPhyAddrOff) & MiiPhyAddrMask)|((reg_addr << MiiRegAddrOff) & MiiRegAddrMask);
	
	tmp_32 = MAC_R32(MiiMR2);
	tmp_32 &= ~(MiiPhyAddrMask|MiiRegAddrMask|MiiOpMask);
	
	MAC_W32(MiiMR2, (tmp_32|addr|MiiOpRead|MiiTransStart));

	//time_out = osal_get_tick();
	
	do {
		cnt ++;
		tmp_32 = MAC_R32(MiiMR2);
		//osal_delay(1);
	    //if ((osal_get_tick() - time_out) > 1000)
	    //{
	    //// 1 second passed, the mdio should failed.
	    //    return 0xffff;
	    //}
	} while((tmp_32&MiiTransStart));
	
	//MAC_PRINTF("MdioRead: cnt(%d)\n", cnt);
	
	tmp_16 = MAC_R16(MdioR);
	
	return (UINT32)tmp_16;
#else
	
	UINT32 retval = 0;

	int i,	data_val = 0;
	UINT32 mii_cmd = (0xf6 << 10) | (phy_addr << 5) | reg_addr;
	UINT32 tmp_u32, cmd_u32;

	UINT32 base_addr = padapter->io_base;
	
	mac_mdio_sync(padapter);

	/* Shift the read command bits out. */
	for (i = 15; i >= 0; i--) 
	{
		data_val = (mii_cmd & (1 << i)) ? MiiMdioOut : 0;

		MAC_W32(MiiMR1, MiiMdioDir|data_val|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
		
		MAC_W32(MiiMR1, MiiMdioDir|data_val|MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}

	/* Read the two transition, 16 data, and wire-idle bits. */
	for (i = 19; i > 0; i--)
	{
		MAC_W32(MiiMR1, 0|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		cmd_u32 = MAC_R32 (MiiMR1);
		retval = (retval << 1) | ((cmd_u32 & MiiMdioIn) ? 1 : 0);

		MAC_W32(MiiMR1, MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}

	 return (retval >> 1) & 0xffff;
#endif //HW_MDIO
}

void mac_mdio_write(pmac_adapter padapter, UINT32 phy_addr, UINT32 reg_addr, UINT32 value)
{
#ifdef HW_MDIO
	UINT32 base_addr = padapter->io_base;
	UINT32 tmp_32;
	UINT32 addr;
	UINT32 cnt = 0;
	UINT32 time_out;
	
	MAC_W16(MdioW, (UINT16)value);
		
	addr = ((phy_addr << MiiPhyAddrOff) & MiiPhyAddrMask)|((reg_addr << MiiRegAddrOff) & MiiRegAddrMask);
	
	tmp_32 = MAC_R32(MiiMR2);
	tmp_32 &= ~(MiiPhyAddrMask|MiiRegAddrMask|MiiOpMask);
	
	MAC_W32(MiiMR2, (tmp_32|addr|MiiOpWrite|MiiTransStart));

	//time_out = osal_get_tick();

	do {
		cnt ++;
		tmp_32 = MAC_R32(MiiMR2);
		//osal_delay(1);
	    //if ((osal_get_tick() - time_out) > 1000)
	    //{
		      // 1 second passed, the mdio should failed.
	    //    return;
	    //}		
	} while(tmp_32&MiiTransStart);
			
	//MAC_PRINTF("MdioWrite: cnt(%d)\n", cnt);
#else

	UINT32 mii_cmd = (0x5002 << 16) | (phy_addr << 23) | (reg_addr << 18) | value;
	int i;
	//UINT32 tmp_u32;
	
	UINT32 base_addr = padapter->io_base;

	mac_mdio_sync(padapter);

	/* Shift the command bits out. */
	for (i = 31; i >= 0; i--)
	{
		int data_val = (mii_cmd & (1 << i)) ? MiiMdioWrite1 : MiiMdioWrite0;

		MAC_W32(MiiMR1, data_val|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		MAC_W32(MiiMR1, data_val|MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}

	/* Clear out extra bits. */
	for (i = 2; i > 0; i--) 
	{
		MAC_W32(MiiMR1, 0|MiiMdioEn);
		mac_mdio_delay(MiiMR1);

		MAC_W32(MiiMR1, MiiMdioClk|MiiMdioEn);
		mac_mdio_delay(MiiMR1);
	}
#endif //HW_MDIO
}

//phy address 0-31
long phy_detect(pmac_adapter padapter)
{
	unsigned char  phy_id;
	unsigned short mode_control_reg;
	unsigned short status_reg;

	if (!padapter)
	{
		MAC_PRINTF("%s: Invalid Parameter!\n", __FUNCTION__);
		return -1;
	}
	
	mode_control_reg = 0;
	status_reg = 0;
	phy_id = 0;

	mode_control_reg = (unsigned short)mac_mdio_read(padapter, padapter->mii.phy_id, PhyBasicModeCtrl);

	status_reg = (unsigned short)mac_mdio_read(padapter, padapter->mii.phy_id, PhyBasicModeStatus);
	if ((mode_control_reg != 0xffff) && (status_reg != 0xffff))
	{
		MAC_PRINTF("%s: phy address is right. \n", __FUNCTION__);
		return 1;
	}

	MAC_PRINTF("%s: phy address is wrong, start auto detecting.\n", __FUNCTION__);

	for (phy_id = 0; phy_id < 32; ++phy_id)
	{
		mode_control_reg = (unsigned short)mac_mdio_read(padapter, phy_id, PhyBasicModeCtrl);

		status_reg = (unsigned short)mac_mdio_read(padapter, phy_id, PhyBasicModeStatus);

		if ((mode_control_reg != 0xffff) && (status_reg != 0xffff))
		{
			MAC_PRINTF("%s: phy address detected: %d. \n", __FUNCTION__, phy_id);
			padapter->mii.phy_id = phy_id;
			return 1;
		}
	}

	MAC_PRINTF("%s: phy address not detected. :(  \n", __FUNCTION__);
	return -1;
}

void phy_set(pmac_adapter padapter)
{
	UINT16 tmp_u16;

	//reset phy registers in a default state.
	mac_mdio_write(padapter, padapter->mii.phy_id, PhyBasicModeCtrl, BMCRReset);

	do
	{	
		tmp_u16 = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyBasicModeCtrl);
	} while(tmp_u16 & BMCRReset);
	MAC_PRINTF("phy_set: phy reset complete.\n");

	//enable Rx/Tx pause frame.
	tmp_u16 = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyNWayAdvert);
	//tmp_u16 |= ANARPauseRx | ANARPauseTx;	
	//tmp_u16 |= (ANARPause | ANARASMDIR | ANAR_MEDIA);
	tmp_u16 |= ANAR_MEDIA;	
	mac_mdio_write(padapter, padapter->mii.phy_id, PhyNWayAdvert, (UINT32)tmp_u16);

	//auto-negotiation enable & restart auto-negotiation.
	tmp_u16 = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyBasicModeCtrl);
	tmp_u16 |= BMCRANEnable | BMCRRestartAN;	
	mac_mdio_write(padapter, padapter->mii.phy_id, PhyBasicModeCtrl, (UINT32)tmp_u16);

	padapter->phy_reset = TRUE;
}

static int phy_link_status_check(pmac_adapter padapter)
{
	UINT16 media = 0;
	UINT16 advertisement = 0;
	UINT16 link = 0;
	UINT16 expansion = 0;
	RET_CODE re = SUCCESS;
	
	unsigned long pause = 0;
	unsigned long asm_dir = 0;
	unsigned long partner_pause = 0;
	unsigned long partner_asm_dir = 0;

	MAC_PRINTF("%s in...\n", __FUNCTION__);

    padapter->duplex_mode = FALSE;
	padapter->link_speed = (UINT32)10;
	padapter->pause_frame_rx = FALSE;
	padapter->pause_frame_tx = FALSE;

	media = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyBasicModeCtrl);
    advertisement = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyNWayAdvert);
    /* reg 5 link partner ability */
    link = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyNWayLPAR);

    if (media & BMCRANEnable)
	{
	    //used for LinkStatusChg check. 
        padapter->link_partner = link;

        expansion = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyNWayExpansion);
	
        if (expansion & 0x1)
		{
		    /* partner support auto nego */
            if ((advertisement & ANARTXFD) && (link & ANARTXFD))
			{
				padapter->link_speed = (UINT32)100;
			    padapter->duplex_mode = TRUE;
				MAC_PRINTF("NEGO:100 Base-TX full duplex.\n");
            }
			else if ((advertisement & ANARTX) && (link & ANARTX))
			{
                padapter->link_speed = (UINT32)100;
			    padapter->duplex_mode = FALSE;
				MAC_PRINTF("NEGO:100 Base-TX half duplex.\n");
            }
			else if ((advertisement & ANAR10FD) && (link & ANAR10FD))
			{
                padapter->link_speed = (UINT32)10;
			    padapter->duplex_mode = TRUE;
                MAC_PRINTF("NEGO:10 Base-TX full duplex.\n");
            }
			else
			{
			    padapter->link_speed = (UINT32)10;
			    padapter->duplex_mode = FALSE;
                MAC_PRINTF("NEGO:10 Base-TX half duplex.\n");
            }
        }
		else
		{
			/* partner doesn't support auto nego */
            if (link & ANARTXFD)
			{
                padapter->link_speed = (UINT32)100;
			    padapter->duplex_mode = TRUE;
                MAC_PRINTF("SELF:100 Base-TX full duplex.\n");
            }
			else if (link & ANARTX)
			{
                padapter->link_speed = (UINT32)100;
			    padapter->duplex_mode = FALSE;
                MAC_PRINTF("SELF:100 Base-TX half duplex.\n");
            }
            else if (link & ANAR10FD)
			{
                padapter->link_speed = (UINT32)10;
			    padapter->duplex_mode = TRUE;
                MAC_PRINTF("SELF:10 Base-TX full duplex.\n");
            }
			else if (link & ANAR10)
			{
                padapter->link_speed = (UINT32)10;
			    padapter->duplex_mode = FALSE;
                MAC_PRINTF("SELF:10 Base-TX half duplex.\n");
            }
			else
			{
			    padapter->link_speed = (UINT32)10;
			    padapter->duplex_mode = FALSE;
                MAC_PRINTF("link check doesn't get corrent speed set 10M half.\n");
				re = FAIL;
            }
		}
	}
	else
	{
	    if (media & BMCRDuplexMode)
		{
            padapter->duplex_mode = TRUE;
        }
		else
		{
            padapter->duplex_mode = FALSE;
        }
        if (!(media & BMCRSpeedSet6) && (media & BMCRSpeedSet13))
		{
            padapter->link_speed = (UINT32)100;
            MAC_PRINTF("DEF:100 Base-TX %s duplex.\n", ((padapter->duplex_mode) ? "full" : "half"));
        }
		else if (!(media & BMCRSpeedSet13) && !(media & BMCRSpeedSet6))
		{
            padapter->link_speed = (UINT32)10;
            MAC_PRINTF("DEF:10 Base-TX %s duplex.\n", ((padapter->duplex_mode) ? "full" : "half"));
        }
		else
		{
            padapter->link_speed = (UINT32)10;
            MAC_PRINTF("%s can't get valid speed, set to 10M.\n", __FUNCTION__);
			re = FAIL;
        }
	}
	/*else
	{
        MAC_PRINTF("error, shouldn't call %s\n", __FUNCTION__);
        return;
    }*/

	pause = (advertisement & ANARPause) > 0 ? 1 : 0;
	asm_dir = (advertisement & ANARASMDIR) > 0 ? 1 : 0;
	partner_pause = (link & ANARPause) > 0 ? 1 : 0;
	partner_asm_dir = (link & ANARASMDIR) > 0 ? 1 : 0;

	MAC_PRINTF("phy_link_chk(): Pause = %ld, ASM_DIR = %ld, PartnerPause = %ld, PartnerASM_DIR = %ld.\n", pause, asm_dir, partner_pause, partner_asm_dir);

	if ((0 == pause) && (0 == asm_dir))
	{
		padapter->pause_frame_rx = FALSE;
		padapter->pause_frame_tx = FALSE;		
	}
	else if ((0 == pause) && (0 == partner_pause))
	{
		padapter->pause_frame_rx = FALSE;
		padapter->pause_frame_tx = FALSE;		
	}
	else if ((0 == pause) && (1 == asm_dir) && (1 == partner_pause) && (0 == partner_asm_dir))
	{
		padapter->pause_frame_rx = FALSE;
		padapter->pause_frame_tx = FALSE;		
	}
	else if ((0 == pause) && (1 == asm_dir) && (1 == partner_pause) && (1 == partner_asm_dir))
	{
		padapter->pause_frame_rx = FALSE;
		padapter->pause_frame_tx = TRUE;		
	}
	else if ((1 == pause) && (0 == asm_dir) && (0 == partner_pause))
	{
		padapter->pause_frame_rx = FALSE;
		padapter->pause_frame_tx = FALSE;		
	}
	else if ((1 == pause) && (1 == partner_pause))
	{
		padapter->pause_frame_rx = TRUE;
		padapter->pause_frame_tx = TRUE;		
	}
	else if ((1 == pause) && (1 == asm_dir) && (0 == partner_pause) && (0 == partner_asm_dir))
	{
		padapter->pause_frame_rx = FALSE;
		padapter->pause_frame_tx = FALSE;		
	}
	else if ((1 == pause) && (1 == asm_dir) && (0 == partner_pause) && (1 == partner_asm_dir))
	{
		padapter->pause_frame_rx = TRUE;
		padapter->pause_frame_tx = FALSE;		
	}
	else
	{
		MAC_PRINTF("Impossiable!\n");
		re = FAIL;
		//asm("sdbbp");
	}

	MAC_PRINTF("phy_link_chk(): Tx flow control = %ld.\n", (long)padapter->pause_frame_tx);
	MAC_PRINTF("phy_link_chk(): Rx flow control = %ld.\n", (long)padapter->pause_frame_rx);
	MAC_PRINTF("%s done\n", __FUNCTION__);
	return re;
}

static void mac_set(pmac_adapter padapter)
{
	UINT32 base_addr = padapter->io_base;

	MAC_PRINTF("%s in...\n", __FUNCTION__);

	if (padapter->phy_reset == FALSE)
	{
		MAC_PRINTF("%s: PhyReset == FALSE. \n", __FUNCTION__);		
		//disable interrupt.
		MAC_W16(IMR, 0);

		//mac_desc_clean(padapter);
		mac_chip_reset(base_addr);
		mac_counter_init(padapter);
		
		phy_set(padapter);		
		padapter->phy_reset = TRUE;

		//enable Timer interrupt & LinkStatus interrupt to complete the Phy initialization.
		MAC_W16(IMR, (ISRTimer|ISRLinkStatus));	
	}	
	else
	{
		MAC_W16(IMR, 0);
		if (padapter->link_established == FALSE)
		{
			MAC_PRINTF("%s link not established, got to wait...\n", __FUNCTION__);
			MAC_W16(IMR, (ISRTimer|ISRLinkStatus));
		}
		else
		{
			phy_link_status_check(padapter);
			mac_hardware_start(padapter);
			padapter->transmit_okay = TRUE;
		}
	}
}

static void mac_weird_interrupt(pmac_adapter padapter)
{
	UINT32 base_addr = padapter->io_base;
	UINT16 tmp_u16, i;
	
	MAC_PRINTF("%s in...\n", __FUNCTION__);

	for (i=0; i<1000; i++)
	{
		if (padapter->link_established == FALSE)
		{
			tmp_u16 = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyBasicModeStatus);

			if (tmp_u16 & BMSRANComplete)
			{
				padapter->auto_n_completed = TRUE;
				ALI_PRINTF("%s: auto-negotiation completed.\n", __FUNCTION__);
			}

			if (tmp_u16 & BMSRLinkStatus)
			{
				padapter->link_established= TRUE;
				ALI_PRINTF("%s: link established. get_timer(0)=0x%x\n", __FUNCTION__, get_timer(0));
			}
			mac_set(padapter);
			continue;
		}

		//if (padapter->cur_isr & (ISRLinkStatus|ISRTimer))
		else
		{
			tmp_u16 = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyBasicModeStatus);
			if (tmp_u16 & BMSRLinkStatus) //up
			{
				tmp_u16 = (UINT16)mac_mdio_read(padapter, padapter->mii.phy_id, PhyNWayLPAR);
				if (tmp_u16 != padapter->link_partner)
				{
					if (tmp_u16)
					{
						ALI_PRINTF("<%s>Line: link reconnected====.\n", __FUNCTION__);
						mac_set(padapter);
					}
				}
				return;
			}
			else //down
			{
				MAC_W16(IMR, (ISRTimer|ISRLinkStatus));
				if(padapter->link_partner != 0)
				{
					padapter->link_partner = 0;
					padapter->phy_reset = FALSE;
					padapter->link_established = FALSE;
					padapter->transmit_okay = FALSE;
				}
				ALI_PRINTF("<%s>Line: link disconnected====.\n", __FUNCTION__);
				return;
			}
		}
	}
}

/*
RxRingDesWPtr = 0x14,   //by SW.
RxRingDesRPtr = 0x16,   //by HW.
TxRingDesWPtr = 0x18,   //by SW.
TxRingDesRPtr = 0x1a,   //by HW.
*/
void mac_rx_interrupt(pmac_adapter padapter)
{
	UINT32 base_addr = padapter->io_base;	
	UINT8 tmp_u8, num_of_pkts = 0;

	MAC_PRINTF("%s in...\n", __FUNCTION__);

	do
	{
		// read Command Register.
		tmp_u8 = MAC_R8(SCR);
		
		MAC_PRINTF("----MAC_R16(SCR) = 0x%X\n", MAC_R16(SCR));
		MAC_PRINTF("----MAC_R16(RxRingDesWPtr) = 0x%X\n", MAC_R16(RxRingDesWPtr));
		MAC_PRINTF("----MAC_R16(RxRingDesRPtr) = 0x%X\n", MAC_R16(RxRingDesRPtr));
		MAC_PRINTF("----MAC_R16(TxRingDesWPtr) = 0x%X\n", MAC_R16(TxRingDesWPtr));
		MAC_PRINTF("----MAC_R16(TxRingDesRPtr) = 0x%X\n", MAC_R16(TxRingDesRPtr));
		
		// if Rx ring buffer empty, break out.
		if ((tmp_u8 & (UINT8)SCRBufEmpty))
		{
			MAC_PRINTF("mac_rx_interrupt: Buffer Empty.\n");
			padapter->mac_stats.rx_buffer_empty ++;
			break;
		}

		if (mac_receive_one_packet(padapter))
		{
			num_of_pkts ++;
		}
		else
			break;
		
	} while(TRUE);

	//MAC_PRINTF("mac_rx_interrupt: %d packets received. \n");
    //printf("<%s>Line:%d, %d packets received---padapter->cur_isr=0x%x ---(ISRRxComplete|ISRRxFifoOverflow |ISRRxBufOverflow | ISRRxBufDiscard |ISRTimer)=0x%x------\n",__FUNCTION__,__LINE__,num_of_pkts,padapter->cur_isr ,(ISRRxComplete|ISRRxFifoOverflow |ISRRxBufOverflow | ISRRxBufDiscard |ISRTimer));
}

void mac_hsr(UINT32 sema_id)
{
}

static void mac_isr(UINT32 pdev)
{
	struct net_device *dev = (struct net_device *)pdev;
	mac_adapter *padapter = (pmac_adapter)dev->priv;
	UINT32 base_addr = padapter->io_base;
	UINT32 cur_isr, i;	

	cur_isr = MAC_R32(ISR);
	MAC_W32(ISR, 0);

	if (cur_isr & padapter->interrupt_mask)
	{		
		MAC_PRINTF("%s %d->", __FUNCTION__, __LINE__);
#if DEBUG_MAC	
		for (i=0; i<32; i++)
		{
			if ((cur_isr>>i)&0x1)
				MAC_PRINTF("%d:%d, ", i, (cur_isr>>i)&0x1);
		}
		MAC_PRINTF("\n");
#endif

		//record supported isr. 
		padapter->cur_isr = (cur_isr & padapter->interrupt_mask);

		if (cur_isr & ISRRxBufDiscard)
			MAC_PRINTF("%s()=>isr(0x%08lX).", __FUNCTION__, cur_isr);
		
		if (padapter->cur_isr & ISRTxUnderrun)
		{
			MAC_PRINTF("mac_task: Tx Fifo underrun. \n");
		}

		//Rx interrupt.
		if (padapter->cur_isr & (ISRRxComplete|ISRRxFifoOverflow|ISRRxBufOverflow|ISRRxBufDiscard|ISRTimer))
		{
			//MAC_PRINTF("mac_interrupt_handle: mac_rx_interrupt. \n");
			mac_rx_interrupt(padapter);
		}
		
		if (padapter->cur_isr & (ISRLinkStatus|ISRTimer |ISRWatchdog))
		{
			mac_weird_interrupt(padapter);
		}
	}
	else
	{
        //printf("<%s>---Line:%d,---------cur_isr=0x%x,padapter->interrupt_mask=0x%x---\n",__FUNCTION__,__LINE__,cur_isr,padapter->interrupt_mask);
		if (cur_isr & 0x1FFF)
		    MAC_PRINTF("mac_isr: interrupt(0x%08x) not supported.\n", cur_isr);
		else
		    MAC_PRINTF("mac_isr: what's this --> 0x%x\n", cur_isr);
	}
}

BOOL mac_rx_hdr_chk(struct net_device *dev, ppacket_head pHead)
{
	pmac_adapter padapter = (pmac_adapter)dev->priv;
	int fatal_err = 0;
	UINT32 temp_u32 = 0;

	if (pHead->ES)
	{
		if (pHead->WatchdogTimeout)
		{
			fatal_err ++;
			padapter->mac_stats.rx_wd_timeout_errors++;
		}
		if (pHead->PhysicalLayerError)
		{
			fatal_err ++;
			padapter->mac_stats.rx_physical_layer_error++;
		}
		if (pHead->LateCollision)
		{
			fatal_err ++;
			padapter->mac_stats.rx_late_collision_seen++;
		}
		if (pHead->Long)
		{
			fatal_err ++;
			padapter->mac_stats.rx_long_errors++;
		}
		if (pHead->Runt) 
		{
			fatal_err ++;
			padapter->mac_stats.rx_runt_errors++;	
		}
		if (pHead->Dribble)
		{
			fatal_err ++;
			padapter->mac_stats.rx_dribble_errors++;	
		}

		if ((pHead->FifoOverflow) && (0 == fatal_err))
		{
			return TRUE;
		}
		else
		{
			padapter->net_stats.rx_errors++;
			printf("<%s> Line:%d, fatal_err=0x%x, pHead->FifoOverflow=%d\n", __FUNCTION__, __LINE__, fatal_err, pHead->FifoOverflow);
			return FALSE;
		}
	}	
	else
	{
		if (pHead->PacketLength > 1536)
		{
			printf("<%s> Line:%d, pHead->PacketLength=%d\n", __FUNCTION__, __LINE__, pHead->PacketLength);
			return FALSE;
		}

#if(TOE2_VLAN_TAG_USED)
		if ((pHead->VLAN) && (padapter->vlan_tag_remove))
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
			printf("<%s> Line:%d, pHead->PacketLength=%d, temp_u32=%d\n", __FUNCTION__, __LINE__, pHead->PacketLength, temp_u32);
			return FALSE;
		}

#if(TOE2_VLAN_TAG_USED)
		//VLAN Patch 3/3 For receiving packet length 1517 & 1518, we need to add a patch here.
		if ((pHead->VLAN) && (!padapter->vlan_tag_remove))
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
			padapter->net_stats.rx_length_errors++;
			padapter->mac_stats.rx_long_errors++;
			printf("<%s> Line:%d, pHead->PacketLength=%d, temp_u32=%d\n", __FUNCTION__, __LINE__, pHead->PacketLength, temp_u32);
			return FALSE;
		}
		
		padapter->net_stats.rx_packets++;
		padapter->net_stats.rx_bytes += pHead->PacketLength;
	
		if (pHead->BF)
			padapter->mac_stats.rx_bc++;
		if (pHead->PF)
			padapter->mac_stats.rx_uc++;
		if (pHead->MF)
			padapter->mac_stats.rx_mc++;
		if (pHead->PPPoE)
			padapter->mac_stats.rx_pppoe++;
		if (pHead->VLAN)
			padapter->mac_stats.rx_vlan++;
		if (pHead->IPFrame)
			padapter->mac_stats.rx_ip++;
		if (pHead->IPFrag)
			padapter->mac_stats.rx_frag++;

		return TRUE;
	}	
}

u16 mac_rx_update_wptr(pmac_adapter padapter)
{
	struct net_device *dev = padapter->net_dev;
	UINT32 base_addr = padapter->io_base;
	u16 rx_bptr, rx_rptr, rx_wptr;
	u16 updata = 0;
	prx_desc rx_desc;
	int i;

	rx_wptr = padapter->rx_wptr;
	rx_bptr = padapter->rx_bptr;
	rx_rptr = MAC_R16(RxRingDesRPtr);

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
		if ((rx_bptr > rx_rptr) || (rx_bptr <= rx_wptr))
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
			rx_desc = &padapter->rx_desc[i];
			//dma_unmap_single((struct device *)NULL, rx_desc->pkt_buf_dma,TOE2_BUF_SZ, DMA_FROM_DEVICE);
			//rx_desc->pkt_buf_dma = dma_map_single((struct device *)NULL, new_skb->data, TOE2_BUF_SZ, DMA_FROM_DEVICE);

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
		
		padapter->rx_wptr = i;
		MAC_W16(RxRingDesWPtr, padapter->rx_wptr);
	}

	//TOE2_TRACE("rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.", rx_wptr, rx_bptr, rx_rptr);
	return rx_rptr;

rx_lost:
	MAC_PRINTF("%()=>rx_bptr got lost.", __FUNCTION__);
	MAC_PRINTF("rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.\n", rx_wptr, rx_bptr, rx_rptr);
	//asm("sdbbp");
	return rx_rptr;
}

static BOOL mac_receive_one_packet(pmac_adapter padapter)
{
	struct net_device *dev = padapter->net_dev;
	prx_desc rx_desc;
	u16 rx_bptr, rx_rptr;
	ppacket_head	pHead;
	u16 pkt_sz, pkts, rx, i;

	MAC_PRINTF("%s in...", __FUNCTION__);

	rx_rptr = mac_rx_update_wptr(padapter);
	//rx_rptr = TOE2_R16(RxRingDesRPtr);
	rx_bptr = padapter->rx_bptr;
	
	if (rx_rptr >= rx_bptr)
		pkts = rx_rptr - rx_bptr;
	else
		pkts = TOE2_RX_DESC_NUM + rx_rptr - rx_bptr;
	
	if ((pkts > 0)&&(rx_rptr == padapter->rx_wptr))
		pkts -= 1;
	
	i = rx_bptr;
	rx = 0;
	while (pkts > 0)
	{
		MAC_PRINTF("mac_receive_one_packet()-> desc[%d]\n", i);
		rx_desc = &padapter->rx_desc[i];
		pHead = &(rx_desc->pkt_hdr);
		//printf("<%s>---Line:%d,---------rx_desc=0x%x,&padapter->rx_desc[%d]=0x%x---\n",__FUNCTION__,__LINE__,rx_desc,i,&padapter->rx_desc[i]);
		//mac_dump_data((rx_desc->pkt_buf_dma | 0xa0000000), pHead ->PacketLength, __FUNCTION__, __LINE__);
		
		if (mac_rx_hdr_chk(padapter->net_dev, pHead))
		{			
			pkt_sz = pHead ->PacketLength; // - MAC_CRC_SIZE;

			if (dev->nd_callback)
			{
				struct net_pkt_t rx_pkt;
				rx_pkt.buffer = rx_desc->pkt_buf_dma | 0xa0000000;
				rx_pkt.length = pkt_sz;
    				dev->nd_callback(NET_RECV_PKT, (UINT32)&rx_pkt);
			}
		}
		else
		{
			printf("mac_rx_pkts(head error): Head(%08lX).", *(UINT32 *)pHead);
			//asm("sdbbp");
		}

		//free rx skb.
		//padapter->rx_skb[i] = NULL;
		
		if (i == TOE2_RX_DESC_NUM - 1)
			i = 0;
		else
			i ++;

		pkts --;
		rx ++;
	}
	
	padapter->rx_bptr = i; 
	return rx;
}

void mac_alloc_rings(pmac_adapter padapter)
{
	UINT16 i;
	UINT32 rx_desc = 0;
	UINT32 rx_buff = 0;
	UINT32 tx_desc = 0;
	////UINT32 setup_buf = 0;

	rx_desc = (UINT32)(&(g_rx_ring_desc[0]));
	rx_buff = (UINT32)(&(g_rx_ring_buff[0]));
	tx_desc = (UINT32)(&(g_tx_ring_desc[0]));
	////setup_buf = (UINT32)(&(g_setup_frame_buf[0]));
	MAC_PRINTF("%s: g_tx_ring_desc %08X\n", __FUNCTION__, (UINT32)g_tx_ring_desc);

	rx_desc = (rx_desc+0x1f)&0xFFFFFFE0;
	rx_buff = (rx_buff+0x1f)&0xFFFFFFE0;
	tx_desc = (tx_desc+0x1f)&0xFFFFFFE0;
	////setup_buf = (setup_buf+0x1f)&0xFFFFFFE0;

	rx_desc = ((rx_desc & 0x0fffffff)|0xa0000000);
	tx_desc = ((tx_desc & 0x0fffffff)|0xa0000000);
	////setup_buf = ((setup_buf & 0x0fffffff)|0xa0000000); 
	
	memset((UINT8*)rx_desc, 0, (TOE2_DESC_SZ * TOE2_RX_DESC_NUM));
	memset((UINT8*)tx_desc, 0, (TOE2_DESC_SZ * TOE2_TX_DESC_NUM));
	////memset((UINT8*)setup_buf, 0, SETUP_FRAME_SIZE);	

	padapter->rx_desc = (prx_desc)rx_desc;
	padapter->rx_desc_dma = virtual_to_dma(rx_desc);
	//memset(rx_buff, 'A', RX_BUF_SIZE);
	//printf("[core] rx_buff addr = 0x%08X\n", rx_buff);
	//mac_dump_data((UINT8*)(rx_buff), 256, __FUNCTION__, __LINE__);
	for (i=0; i<TOE2_RX_DESC_NUM; i++)
	{
		padapter->rx_desc[i].pkt_buf_dma = virtual_to_dma(rx_buff+i*RX_BUF_SIZE);
	}
	
	padapter->tx_desc = (ptx_desc)tx_desc;
	padapter->tx_desc_dma = virtual_to_dma(tx_desc);
	MAC_PRINTF("%s: tx_desc %08X, tx_desc_dma %08X\n", __FUNCTION__, (UINT32)(padapter->tx_desc), (UINT32)(padapter->tx_desc_dma));
	////padapter->setup_frame_buf = (u8 *)setup_buf;
	////padapter->setup_frame_dma = virtual_to_dma(setup_buf);
}

void mac_free_rings(pmac_adapter padapter)
{
	if (padapter->rx_desc != NULL)
	{
		memset(padapter->rx_desc, 0x00, (TOE2_DESC_SZ * TOE2_RX_DESC_NUM));
		//free(padapter->rx_desc);
		padapter->rx_desc = NULL;
	}
	
	if (padapter->tx_desc != NULL)
	{
		memset(padapter->tx_desc, 0x00, (TOE2_DESC_SZ * TOE2_TX_DESC_NUM));
		//free(padapter->tx_desc);
		padapter->tx_desc = NULL;
	}	
	
	if (padapter->setup_frame_buf != NULL)
	{
		memset(padapter->setup_frame_buf, 0x00, SETUP_FRAME_SIZE);
		//free(padapter->setup_buf);
		padapter->setup_frame_buf = NULL;
	}
}

void m3602_eth_halt(struct eth_device *dev)
{
	ALI_PRINTF("<%s> dev=0x%x\n", __FUNCTION__, dev);
	/*if (dev)
		free(dev);

	if (g_net_dev)
	{
		eth_mac_detach(g_net_dev);
		g_net_dev = NULL;
		g_mac_adapter = NULL;
	}*/
}

int m3602_eth_send(struct eth_device* dev, volatile void *packet, int length)
{
	MAC_PRINTF("%s in...\n", __FUNCTION__);

	if (g_net_dev)
	{
		eth_mac_send_one_packet(g_net_dev, packet, length);
	}
	else
	{
		puts("No net device available.\n");
	}
	return 0;
}

void m3602_eth_recv(struct eth_device *dev)
{
    //MAC_PRINTF("<%s> dev=0x%x\n", __FUNCTION__, dev);
	if (g_net_dev && g_mac_adapter)
	{
		//mac_isr((UINT32)g_net_dev);
		mac_rx_interrupt(g_mac_adapter);
	}
	else
	{
		puts("No mac adapter available.\n");
	}
}

int ali_IsEthAddrSet()
{
	char   *cp;
	u_char MacAddr[6];

	//check 'ethaddr' is set or not?
	cp = getenv("ethaddr");
	if (cp != NULL)
	{
		eth_parse_enetaddr(cp, MacAddr);
		if (is_valid_ether_addr(MacAddr))
		{
			return 1;
		}
		else
		{
			printf("Error: (%s) is not a valid mac address\n",cp);
		}
	}
	else
	{
		printf("Env var 'ethaddr' is not set yet.");
	}
	return 0;
}

//int ali_enet_initialize(bd_t *bis)
int	toe2_register(bd_t *bis)
{
	struct eth_device *dev;
	
	ALI_PRINTF("<%s> in...\n", __FUNCTION__);
	if (g_net_dev != NULL)
	{
		puts("*Ethernet already start up.\n");
		return 0;
	}

	if (0 == ali_IsEthAddrSet())
	{
		printf("Please use 'stbid set_mac' command to set a valid mac address.");
		return -1;
	}

	if (NULL == (dev = (struct eth_device *)malloc(sizeof(struct eth_device))))
	{
		puts("Error: eth malloc failure.\n");
		return -2;
	}

	MAC_PRINTF("<%s> g_net_dev=0x%x, dev=0x%x\n", __FUNCTION__, g_net_dev, dev);
	memset(dev, 0, sizeof(struct eth_device));
	sprintf(dev->name, "TOE2");

	dev->iobase = 0;
	dev->priv = 0;

	dev->init = m3602_eth_init;
	dev->halt = m3602_eth_halt;
	dev->send = m3602_eth_send;
	dev->recv = m3602_eth_recv;

	eth_register(dev);
	ALI_PRINTF("eth register success.\n");

	m3602_eth_init(NULL, NULL);

	return 0;
}

void eth_mac_callback(UINT32 type, UINT32 pkt)
{
	struct net_pkt_t *r_pkt = (struct net_pkt_t *)pkt;
	NetReceive(r_pkt->buffer, r_pkt->length);
}

#if 0
static void phy_chip_reset(void)
{
	unsigned long tmp_u32;			
		
	// pinmux RMII
	tmp_u32 = *((volatile unsigned long *)0xb8000088);
	tmp_u32 |= (1 << 27);  
	tmp_u32 &= ~((1 << 28)|(1 << 29)|(1 << 30)|(1 << 22)|(1<<23)|(1 << 24)|(1 << 25));  
	*((volatile unsigned long *)0xb8000088) = tmp_u32;
	
	tmp_u32 = *((volatile unsigned long *)0xb800008c);
	tmp_u32 &= ~((1 << 12));  
	*((volatile unsigned long *)0xb800008c) = tmp_u32;

	udelay(10000); //boot_delay(10);	

/*	tmp_u32 = *((volatile unsigned long *)0xb80000AC);
	tmp_u32 |= (1 << 24);
	*((volatile unsigned long *)0xb80000AC) = tmp_u32;
	udelay(100000);  //boot_delay(10);
*/
	// reset phy
	tmp_u32 = *((volatile unsigned long *)0xb8000430);
	tmp_u32 |= (1 << 29);  // GPIO29 PHY reset enable
	*((volatile unsigned long *)0xb8000430) = tmp_u32;
		
	tmp_u32 = *((volatile unsigned long *)0xb8000058);
	tmp_u32 |= (1 << 29);  // GPIO29 PHY reset output
	*((volatile unsigned long *)0xb8000058) = tmp_u32;

	tmp_u32 = *((volatile unsigned long *)0xb8000054);
	tmp_u32 |= (1 << 29);  // GPIO29 PHY reset output 1
	*((volatile unsigned long *)0xb8000054) = tmp_u32;

	udelay(10000); //boot_delay(10);		  	
	tmp_u32 = *((volatile unsigned long *)0xb8000054);
	tmp_u32 &= ~(1 << 29);  // GPIO29 PHY reset output 0
	*((volatile unsigned long *)0xb8000054) = tmp_u32;

	udelay(10000); //boot_delay(10);		  	
	tmp_u32 = *((volatile unsigned long *)0xb8000054);
	tmp_u32 |= (1 << 29);  // GPIO29 PHY reset output 1
	*((volatile unsigned long *)0xb8000054) = tmp_u32;		
		
	udelay(10000); //boot_delay(10);		  	
}
#endif

static int m3602_eth_init(struct eth_device *dev, bd_t *bis)
{
	MAC_CONFIG mac_cfg;
	UINT32 link_mode;
	
	memset(&mac_cfg, 0, sizeof(MAC_CONFIG));	

	ALI_PRINTF("<%s> g_net_dev=0x%x, g_mac_adapter=0x%x\n", __FUNCTION__, g_net_dev, g_mac_adapter);
	if (!g_net_dev)
	{
		//asm("sdbbp");
		mac_cfg.mac_addr[0] = 0xde;
		mac_cfg.mac_addr[1] = 0xad;
		mac_cfg.mac_addr[2] = 0xbe;
		mac_cfg.mac_addr[3] = 0xef;
		mac_cfg.mac_addr[4] = 0x01;
		mac_cfg.mac_addr[5] = 0x01;
		//Set phy address
		mac_cfg.phy_addr = 0x05;

		//Set phy mode to RMII
		mac_cfg.phy_mode = NET_PHY_RMII;

		//phy_chip_reset();
		eth_mac_attach(&mac_cfg);

		link_mode = NET_LINK_AUTO;

		MAC_PRINTF("<%s> g_net_dev->priv=0x%x, g_mac_adapter=0x%x\n", __FUNCTION__, g_net_dev->priv, g_mac_adapter);

		if (g_net_dev)
		{
			//eth_mac_set_info(g_net_dev, NET_SET_LINK_MODE, &link_mode);
			eth_mac_open(g_net_dev, /*NULL*/eth_mac_callback);
		}
	}

	{
		MAC_PRINTF("*check link...\n");
		mac_weird_interrupt(g_mac_adapter);
	}

	return 0;
}

RET_CODE eth_mac_attach(MAC_CONFIG *cfg)
{
	struct net_device *dev;
	mac_adapter *padapter;
	UINT32 base_addr;
	int i;

	base_addr = MAC_BaseAddr;

	MAC_PRINTF(" eth_mac_attach: ipbase=0x%08X\n", base_addr);
	
	//alloc memory space for net device.
	if (g_net_dev) 
	{
		free(g_net_dev);
		g_net_dev = NULL;
	}
	
	dev = malloc(sizeof(struct net_device));
	if (dev == NULL)
	{
		printf("[Err] eth_mac_attach: alloc net device error!\n");
		return ERR_NO_MEM;
	}

	//alloc memory space for ethernet mac private structure.
	if (g_mac_adapter)
	{
		free(g_mac_adapter);
		g_mac_adapter = NULL;
	}
	padapter = (void *)malloc(sizeof(mac_adapter));
	if (padapter == NULL)
	{
		free(dev);
		printf("[Err] eth_mac_attach: alloc mac private memory error!\n");
		return ERR_NO_MEM;
	}
	memset(padapter, 0, sizeof(mac_adapter));
	
	MAC_PRINTF(" eth_mac_attach: net dev = 0x%x, padapter = 0x%x\n", dev, padapter);

	dev->priv = padapter;
	dev->next = NULL;
	
	//padapter->multicast_flags = 0;
	//debug use...
	MAC_PRINTF(" eth_mac_attach: for sure, padapter->multicast_flags = IFF_Promiscuous.\n");
	padapter->multicast_flags = IFF_Promiscuous;

#if 0
	padapter->multicast_flags = IFF_PerfectFilterting;
	padapter->multicast_counter = 2;
	padapter->multicast_list[0][0] = 0xFF;
	padapter->multicast_list[0][1] = 0xFF;
	padapter->multicast_list[0][2] = 0xFF;
	padapter->multicast_list[0][3] = 0xFF;
	padapter->multicast_list[0][4] = 0xFF;
	padapter->multicast_list[0][5] = 0xFF;
	padapter->multicast_list[1][0] = 0x51;
	padapter->multicast_list[1][1] = 0x52;
	padapter->multicast_list[1][2] = 0x53;
	padapter->multicast_list[1][3] = 0x54;
	padapter->multicast_list[1][4] = 0x55;
	padapter->multicast_list[1][5] = 0x56;
#endif	
	
	dev->base_addr = base_addr;

	padapter->net_dev = dev;
	padapter->io_base = base_addr;
	padapter->interrupt_vector = MAC_InterruptVector;

	//set mac address.
	for (i = 0; i < 6; i++)
	{
		padapter->mac_addr[i] = cfg->mac_addr[i];
	}
	
	MAC_PRINTF(" eth_mac_attach: Mac addr: %X-%X-%X-%X-%X-%X.\n", \
				padapter->mac_addr[0],padapter->mac_addr[1], \
				padapter->mac_addr[2],padapter->mac_addr[3], \
				padapter->mac_addr[4],padapter->mac_addr[5]);

	padapter->phy_mode = cfg->phy_mode;
	
	//init function point.
	dev->attach = eth_mac_attach;
	dev->detach = eth_mac_detach;
	dev->open = eth_mac_open;
	dev->close = eth_mac_close;
	dev->ioctl = eth_mac_ioctl;
	dev->send_packet = eth_mac_send_one_packet;
	dev->nd_callback = NULL;

	dev->get_info = eth_mac_get_info;
	dev->set_info = eth_mac_set_info;
	
	//added on 2008.02.25
	padapter->mii.phy_id = cfg->phy_addr;
	padapter->mii.dev = dev;
	padapter->mii.phy_id_mask = 0x3f;
	padapter->mii.reg_num_mask = 0x1f;
	
	g_net_dev = dev;
	g_mac_adapter = padapter;
	
	MAC_PRINTF(" eth_mac_attach: g_net_dev=0x%x, g_mac_adapter=0x%x\n", g_net_dev, g_mac_adapter);
	return SUCCESS;
}

RET_CODE eth_mac_detach(struct net_device *dev)
{
	pmac_adapter padapter;
	UINT32 base_addr = dev->base_addr;

	MAC_PRINTF(" eth_mac_detach enter\n");
	
	padapter = (pmac_adapter)(dev->priv);
	memset(padapter, 0x00, sizeof(mac_adapter));
	free(padapter);
	padapter = NULL;
	free(dev);
	dev = NULL ;
	
	return SUCCESS;
}

RET_CODE eth_mac_open(struct net_device*dev, void (*callback)(UINT32, UINT32))
{
	pmac_adapter padapter;
	UINT32 base_addr = dev->base_addr;
	UINT32 tmp_u32;
	struct dev_mc_list *mc_list;
	RET_CODE rv = SUCCESS;	
	int i;

	MAC_PRINTF("<%s> in...\n", __FUNCTION__);

	padapter = (pmac_adapter)dev->priv;

	//mac software reset.
	mac_chip_reset(base_addr);

	tmp_u32 = MAC_R32(MiiMR1);
#ifdef HW_MDIO
	tmp_u32 &= ~MiiMdioEn;
#else
	tmp_u32 |= MiiMdioEn;
#endif //HW_MDIO
	MAC_W32(MiiMR1, tmp_u32);

 	mac_alloc_rings(padapter);

	//init counter value.
	mac_counter_init(padapter);

	//detect phy address
	if (phy_detect(padapter) < 0)
	{
		rv = ERR_FAILURE; //ERR_FAILUE;
		mac_free_rings(padapter);
		return rv;
	}

	//access phy registers first.
	phy_set(padapter);
	
	//frequency is 28MHz, so set timer to 1 time per second.     
	MAC_W32(TimerR, toe2_timer_frq);

	//enable Timer interrupt & LinkStatus interrupt to complete the Phy initialization.
	MAC_W16(IMR, (ISRTimer|ISRLinkStatus));

	dev->nd_callback = callback;

	MAC_PRINTF("<%s> done.\n", __FUNCTION__);

	return rv;
}

RET_CODE eth_mac_close(struct net_device *dev)
{
	pmac_adapter padapter = (mac_adapter *)dev->priv;
	UINT32 base_addr = dev->base_addr;
	UINT8 *tx_buf, *rx_buf;

	MAC_PRINTF("%s enter\n", __FUNCTION__);

	//while(padapter->free_tx_desc != NUM_OF_DESC)
	//	MAC_PRINTF("eth_mac_close: free_tx_desc = %x \n", padapter->free_tx_desc);

	//disable interrupt.
	//disable tx & rx.
	MAC_W16(IMR, 0);
	MAC_W8(SCR, 0);

	//deallocate resources.
	mac_free_rings(padapter);

	return SUCCESS;
}

RET_CODE eth_mac_ioctl(struct net_device *dev, UINT32 _data, UINT32 cmd)
{
	struct ifreq *rq = (struct ifreq *)_data;
	pmac_adapter padapter = dev->priv;
	struct mii_ioctl_data *data = (struct mii_ioctl_data *) & rq->ifr_data;
	int rc;

	MAC_PRINTF("%s enter\n", __FUNCTION__);

	rc = mii_ioctl(&padapter->mii, data, cmd, NULL);

	return rc;
}

void mac_tx_sts(pmac_adapter padapter, ptx_desc desc_sw)
{
    MAC_PRINTF("mac_tx_sts...\n");
	if ((desc_sw->tx_sts.sw.FS) && !(desc_sw->tx_sts.sw.OWN))
	{
		if (!(desc_sw->tx_sts.sw.ES))
		{
			padapter->net_stats.tx_packets++;
			MAC_PRINTF("\ttx_packets=%d\n", padapter->net_stats.tx_packets);
		}
		else
		{ 
			padapter->net_stats.tx_errors++;
		    MAC_PRINTF("\ttx_errors=%d\n", padapter->net_stats.tx_errors);
			if ((desc_sw->tx_sts.sw.LossOfCarrier)||(desc_sw->tx_sts.sw.NoCarrier))
			{
				padapter->net_stats.tx_carrier_errors++;
				MAC_PRINTF("\ttx_carrier_errors=%d\n", padapter->net_stats.tx_carrier_errors);
			}
			if (desc_sw->tx_sts.sw.LateCol)
			{
				padapter->net_stats.tx_window_errors++;
				MAC_PRINTF("\ttx_window_errors=%d\n", padapter->net_stats.tx_window_errors);
			}
			if (desc_sw->tx_sts.sw.FifoUnderrun)
			{
				padapter->net_stats.tx_fifo_errors++;
				MAC_PRINTF("\ttx_fifo_errors=%d\n", padapter->net_stats.tx_fifo_errors);
			}
			if (desc_sw->tx_sts.sw.HF)
			{
				padapter->net_stats.tx_heartbeat_errors++;
				MAC_PRINTF("\ttx_heartbeat_errors=%d\n", padapter->net_stats.tx_heartbeat_errors);
			}
		}
		
		if (desc_sw->tx_sts.sw.ExCol)
		{
			padapter->mac_stats.tx_collision_errors++;
			MAC_PRINTF("\ttx_collision_errors=%d\n", padapter->mac_stats.tx_collision_errors);
		}
		else
		{
			padapter->mac_stats.tx_collision_counter[desc_sw->tx_sts.sw.ColCnt]++;
			MAC_PRINTF("\ttx_collision_counter[%d]=%d\n", desc_sw->tx_sts.sw.ColCnt, padapter->mac_stats.tx_collision_counter[desc_sw->tx_sts.sw.ColCnt]);
		}
	}
}

void mac_dump_data(UINT8 *buf, UINT16 len, char *callfunc, int callline)
{
#ifdef ALI_DEBUG_ON
	UINT16 i;
	printf("%s %d: buf %08X, len %d:", callfunc, callline, (UINT32)buf, len);

	for (i=0; i<len; i++)
	{
		if(i%16==0) printf("\n");
		printf("%02X ", *(buf+i));
	}
	printf("\n");
#endif
}

RET_CODE eth_mac_send_one_packet(struct net_device *dev, void *buf, UINT16 len)
{
	pmac_adapter padapter = dev->priv;
	UINT32 base_addr = padapter->io_base;
	ptx_desc desc;
	u16 tx_rptr;
	u16 off, first = 0;
	UINT32 send_buffer = 0;
	//volatile char read_buffer[TOE2_DESC_SZ] = {0};

	MAC_PRINTF("%s: dev %08X, buf %08X, len %d, base_addr %08X\n", __FUNCTION__, (UINT32)dev, (UINT32)buf, len, base_addr);

    MAC_PRINTF("%----MAC_R16(SCR) = 0x%X\n", MAC_R16(SCR));
	MAC_PRINTF("%----MAC_R16(RxRingDesWPtr) = 0x%X\n", MAC_R16(RxRingDesWPtr));
	MAC_PRINTF("%----MAC_R16(RxRingDesRPtr) = 0x%X\n", MAC_R16(RxRingDesRPtr));
	MAC_PRINTF("%----MAC_R16(TxRingDesWPtr) = 0x%X\n", MAC_R16(TxRingDesWPtr));
	MAC_PRINTF("%----MAC_R16(TxRingDesRPtr) = 0x%X\n", MAC_R16(TxRingDesRPtr));

	mac_dump_data(buf, len, __FUNCTION__, __LINE__);
	if (!padapter->link_established || !padapter->transmit_okay)
	{
		ALI_PRINTF("ERROR:packet ISN'T sent since Link not established yet @ <%s>, link_established=0x%x\n", __FUNCTION__, padapter->link_established);
		return 0;
	}

	MAC_PRINTF("%s begin\n", __FUNCTION__, sizeof(struct teo2_tx_desc));
	off = padapter->tx_wptr;
	first = off;

	MAC_PRINTF("%s offset=%d\n", __FUNCTION__, off);

    //memset(g_tx_ring_buff, 'A', TX_BUF_SIZE+0x20);
	//mac_dump_data(g_tx_ring_buff, 100, __FUNCTION__, __LINE__);
	
	send_buffer = (UINT32)(&(g_tx_ring_buff[0]));
	send_buffer = (send_buffer+0x1f)&0xFFFFFFE0;
	memcpy((UINT8*)send_buffer, buf, len);
	cache_flush(send_buffer, len);

	mac_dump_data((UINT8*)send_buffer, len, __FUNCTION__, __LINE__);
	
	desc = &(padapter->tx_desc[off]);
	memset(desc, 0, TOE2_DESC_SZ);

    //memset((UINT8*)((padapter->rx_desc[0].pkt_buf_dma) | 0xA0000000), 'A', RX_BUF_SIZE);
    //mac_dump_data((UINT8*)((padapter->rx_desc[0].pkt_buf_dma) | 0xA0000000), 256, __FUNCTION__, __LINE__);
	//memcpy((UINT8*)((padapter->rx_desc[0].pkt_buf_dma) | 0xA0000000), buf, len);
	//mac_dump_data((UINT8*)((padapter->rx_desc[0].pkt_buf_dma) | 0xA0000000), 256, __FUNCTION__, __LINE__);

	desc->pkt_buf_dma = virtual_to_dma(send_buffer);  //padapter->rx_desc[0].pkt_buf_dma; //__CTDADDRALI(virt_to_phys(send_buffer));
	desc->seg_num = 1;
	desc->seg_len = len;
	desc->tot_len = len;
	desc->tx_sts.hw.VlanEn = 0;
	desc->tx_sts.hw.OWN = 1;
	desc->tx_sts.hw.FS = 1;
	desc->tx_sts.hw.LS = 1;
	desc->tx_sts.hw.Mfl = TOE2_MSS;
	if (off == TOE2_TX_DESC_NUM-1)
		desc->tx_sts.hw.EOR = 1;	

	tx_rptr = MAC_R16(TxRingDesRPtr);
	MAC_PRINTF("%s: %d %d\n", __FUNCTION__, tx_rptr, desc->tx_sts.hw.OWN);	

	if ((++off) >= TOE2_TX_DESC_NUM)
		off = 0;

    //printf("[core] TSAD = 0x%X, tx_desc=0x%X, tx_desc_dma=0x%X, tx_rptr=%d, off=%d\n",
		//MAC_R32(TSAD), padapter->tx_desc, padapter->tx_desc_dma, tx_rptr, off);
	//SDBBP();

    memcpy((UINT8*)send_buffer, buf, len);
	mac_dump_data((UINT8*)send_buffer, len, __FUNCTION__, __LINE__);
	MAC_W16(TxRingDesWPtr, off);

	padapter->tx_wptr = off;
	
	MAC_PRINTF("desc->pkt_buf_dma = 0x%x, 0x%x\n", desc->pkt_buf_dma, __CTDADDRALI(virt_to_phys(send_buffer)));

    mac_dump_data((UINT8*)send_buffer, len, __FUNCTION__, __LINE__);
	
	do {
		tx_rptr = MAC_R16(TxRingDesRPtr);
	} while(padapter->tx_wptr != tx_rptr);

	desc = &padapter->tx_desc[first];
	mac_tx_sts(padapter, desc);

	MAC_PRINTF("%,----MAC_R16(SCR) = 0x%X\n", MAC_R16(SCR));
	MAC_PRINTF("%,----MAC_R16(RxRingDesWPtr) = 0x%X\n", MAC_R16(RxRingDesWPtr));
	MAC_PRINTF("%,----MAC_R16(RxRingDesRPtr) = 0x%X\n", MAC_R16(RxRingDesRPtr));
	MAC_PRINTF("%,----MAC_R16(TxRingDesWPtr) = 0x%X\n", MAC_R16(TxRingDesWPtr));
	MAC_PRINTF("%,----MAC_R16(TxRingDesRPtr) = 0x%X\n", MAC_R16(TxRingDesRPtr));
	
	ALI_PRINTF("%s: SEND Successfully.\n", __FUNCTION__);

	return 0;
}

struct net_device_stats *eth_mac_get_stats(struct net_device *dev)
{
	pmac_adapter padapter;

	MAC_PRINTF("%s enter\n", __FUNCTION__);

	padapter = (mac_adapter *)(dev->priv);
	return &padapter->net_stats;
}

UINT32 mac_crc(UINT8 *p, int len)
{
	UINT32 crc = ~0;
	int i;
	while (len--)
	{
		crc ^= *p++;
		for (i = 0; i < 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LE : 0);
	}
	return crc;
}

void mac_setup_frame(struct net_device *dev)
{
	pmac_adapter padapter = (mac_adapter *)dev->priv;
	UINT32 base_addr = padapter->io_base;

	UINT32 i, j, crc;
	UINT16 hashcode, byte, bit;
	UINT8 *pa, *addr;

	memset(padapter->setup_frame_buf, 0x00, SETUP_FRAME_SIZE);

	if ((padapter->multicast_flags&IFF_MODE_MASK) == IFF_PerfectFilterting
			|| (padapter->multicast_flags&IFF_MODE_MASK) == IFF_Inverse)
	{
		//host mac addr
		pa = padapter->setup_frame_buf + 0;
		for (i=0; i<MAX_MC_ADDR; i++) 
		{
			pa[i&0x01] = padapter->mac_addr[i];
			if (i&0x01)
				pa += 4;
		}

		for (i = 0; i < padapter->multicast_counter && i <MAX_MC_LIST-1; i ++)
		{
			for (j = 0; j < MAX_MC_ADDR; j ++) 
			{
				pa[j&0x01] = padapter->multicast_list[i][j];
				if(j&0x01) pa += 4;
			}
		}

		// Broadcast address
		/*for (i=0; i<MAX_MC_ADDR; i++)
		  { 
		  pa[i&0x01] = 0xFF;
		  if (i & 0x01)
		  pa += 4;
		  }*/
	}
	else if ((padapter->multicast_flags&IFF_MODE_MASK) == IFF_HashFilterting_OnePecfect
			|| (padapter->multicast_flags&IFF_MODE_MASK) == IFF_HashFilterting)
	{
		pa = padapter->setup_frame_buf + 0;
		if (padapter->multicast_counter>MAX_MC_LIST)
			padapter->multicast_counter=MAX_MC_LIST;
		for (i = 0; i < padapter->multicast_counter; i ++)
		{
			addr = &padapter->multicast_list[i][0];
			crc = mac_crc(addr, MAX_MC_ADDR);

			hashcode = (UINT16)crc & HASH_BITS;  /* hashcode is 9 LSb of CRC */

			byte = hashcode >> 3;				/* bit[3-8] -> byte in filter */
			bit = 1 << (hashcode & 0x07);		/* bit[0-2] -> bit in byte */

			byte <<= 1;						/* calc offset into setup frame */
			if (byte & 0x02)
				byte -= 1;

			pa[byte] |= bit;
		}

		if ((padapter->multicast_flags&IFF_MODE_MASK) == IFF_HashFilterting_OnePecfect)
		{
			//fill our own mac addr in setup frame buffer(offset from 156 to 167) 
			pa = padapter->setup_frame_buf + IMPERFECT_PA_OFFSET;		
			for (i = 0; i < MAX_MC_ADDR; i ++) 
			{
				pa[i&0x01] = padapter->mac_addr[i];
				if(i&0x01) pa += 4;
			}
			//offset is 168 now.
		}

		//pa[(HASH_TABLE_LEN >> 3) - 3] = 0x80; //??? what for. broadcast address.
	}
	else
	{		
		SDBBP();//bug
	}
}

RET_CODE mac_set_multicast_filter(struct net_device *dev)
{
	pmac_adapter padapter = (mac_adapter *)dev->priv;
	UINT32 base_addr = padapter->io_base;
	RET_CODE rv = SUCCESS;
	int i, j;
	UINT8 cmd_u8, *addr;
	UINT32 cmd_u32, crc, bit, byte;
	UINT16 hashcode;

	MAC_PRINTF("mac_set_multicast_filter: \n");

	if (padapter->multicast_counter == 0)
	{  
		MAC_PRINTF("mac_set_multicast_filter: multicast_counter = 0. \n");
		return rv;
	}

#if DEBUG_MAC	
	for (i = 0; i < padapter->multicast_counter; i++)
	{
		MAC_PRINTF("mac_set_multicast_filter: MC(%d) = %02x-%02x-%02x-%02x-%02x-%02x\n", 
		i,
		padapter->multicast_list[i][0],
		padapter->multicast_list[i][1],
		padapter->multicast_list[i][2],
		padapter->multicast_list[i][3],
		padapter->multicast_list[i][4],
		padapter->multicast_list[i][5]);
	}
#endif //DEBUG_MAC

	mac_setup_frame(dev);

	//disable tx & rx.
	cmd_u8 = MAC_R8(SCR);
	MAC_W8(SCR, cmd_u8 & ~(SCRRxEn|SCRTxEn));
	
	cmd_u32 = MAC_R32(NetworkOM);
	cmd_u32 &= ~PassMask;
	//cmd_u32 |= PassErr;

	if (padapter->multicast_flags & IFF_PassAllMulticast) 
		cmd_u32 |= PassAllMulticast; 

	cmd_u32 &= ~FilteringMask;
	if ((padapter->multicast_flags&IFF_MODE_MASK) == IFF_PerfectFilterting)
	{
		cmd_u32 |= PerfectFiltering;
	}	
	else if ((padapter->multicast_flags&IFF_MODE_MASK) == IFF_HashFilterting_OnePecfect)
	{
		cmd_u32 |= HashFiltering;
	}
	else if ((padapter->multicast_flags&IFF_MODE_MASK) == IFF_Inverse)
	{
		cmd_u32 |= InverseFiltering;
	}
	else if ((padapter->multicast_flags&IFF_MODE_MASK) == IFF_HashFilterting)
	{
		cmd_u32 |= HashOnlyFiltering;
	}
	
	MAC_W32(NetworkOM, cmd_u32);

	//MAC_W8(SCR, cmd_u8 | (SCRRxEn| SCRTxEn));

	return rv;
}

RET_CODE mac_load_packet(struct net_device *dev)
{
	pmac_adapter padapter = dev->priv;
	UINT32 base_addr = padapter->io_base;
	ptx_desc desc;
	u16 tx_rptr;
	u16 off = 0;
	UINT8 cmd_u8;

	MAC_PRINTF("%s enter\n", __FUNCTION__);

	if (!padapter->link_established)
	{
		MAC_PRINTF("%s: link not established\n", __FUNCTION__);
		return 0;
	}

	cmd_u8 = MAC_R8(SCR);
	MAC_W8(SCR, cmd_u8 | (SCRTxEn));

	off = padapter->tx_wptr;

	MAC_PRINTF("%s: offset=%d\n", __FUNCTION__, off);

	desc = &(padapter->tx_desc[off]);
	memset(desc, 0, TOE2_DESC_SZ);
	desc->pkt_buf_dma = padapter->setup_frame_dma;	
	desc->seg_num = 1;
	desc->seg_len = SETUP_FRAME_SIZE;
	desc->tot_len = SETUP_FRAME_SIZE;
	desc->tx_sts.hw.VlanEn = 0;
	desc->tx_sts.hw.OWN = 1;
	desc->tx_sts.hw.SetupPkt = 1;
	desc->tx_sts.hw.FilteringMode = (padapter->multicast_flags&IFF_MODE_MASK);
	desc->tx_sts.hw.FS = 1;
	desc->tx_sts.hw.LS = 1;	
	desc->tx_sts.hw.Mfl = TOE2_MSS;
	if (off == TOE2_TX_DESC_NUM -1)
		desc->tx_sts.hw.EOR = 1;	

	tx_rptr = MAC_R16(TxRingDesRPtr);
	MAC_PRINTF("%s: %d %d\n", __FUNCTION__, tx_rptr, desc->tx_sts.hw.OWN);	
	
	if ((++off) >= TOE2_TX_DESC_NUM)
		off = 0;
	MAC_W16(TxRingDesWPtr, off);
	padapter->tx_wptr = off;

	MAC_PRINTF("%s begin:\n", __FUNCTION__);
	do
	{
		tx_rptr = MAC_R16(TxRingDesRPtr);
		udelay(10000);
		MAC_PRINTF("%s: %d %d\n", __FUNCTION__, tx_rptr, desc->tx_sts.hw.OWN);	
	} while(padapter->tx_wptr != tx_rptr);

	MAC_PRINTF("%s end:\n", __FUNCTION__);

	return 0;
}

RET_CODE eth_mac_set_rx_mode(struct net_device *dev)
{
	pmac_adapter padapter = dev->priv;
	UINT32 base_addr = padapter->io_base;

	UINT8 cmd_u8;
	UINT32 cmd_u32;
	RET_CODE rv = SUCCESS;

	if (padapter->multicast_flags & IFF_Promiscuous) 
	{
		/* set promiscuous mode */
		cmd_u32 = MAC_R32(NetworkOM);
		cmd_u32 &= ~(PassMask);
		MAC_W32(NetworkOM, (cmd_u32|PassPromiscuous));
		MAC_PRINTF("eth_mac_set_multicast_list:  multicast_flags & IFF_Promiscuous. \n");
	}
	else
	{	
		mac_set_multicast_filter(dev);
		mac_load_packet(dev);
	}

	return rv;
}

