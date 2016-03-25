#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/io.h>
#include <malloc.h>
#include "uboot_eth_gmac.h"
#include "uboot_eth_reg.h"


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

#define __CTDADDRALI(x) (((u32)(x)) & 0x7fffffff)
#define READ_MAC_REG   0x1
#define READ_SOC_REG   0x2
#define WRITE_MAC_REG  0x3
#define WRITE_SOC_REG  0x4
#define DEBUG_DRIVER_LEVEL 0x5
#define TX_PKTS        0x6
/* end */
u32 LIMIT_PKTS;

int lock_debug = 0;

UINT32 gmac_base = _GMAC_BASE;
u8 stb_gmac_mac[] = {0xde, 0xad, 0xbe, 0xef, 0x01, 0x01};
static UINT32 gmac_mac_hi16 = 0;
static UINT32 gmac_mac_lo32 = 0;
UINT32 use_default_mac = 1;

static UINT32 gmac_phy_addr = GMAC_PHY_ADDR;

#ifdef USE_GPHY
static int gmac_phy_mode = ETH_PHY_MODE_RGMII;
#else
static int gmac_phy_mode = ETH_PHY_MODE_RMII;
#endif

/*ToDo: uboot doesn't support dma_alloc_coherent
 *      need reference to 3701c uboot ethernet
 */
/* why add 0x20?  need 32byte align? */
#define GEN_32BYTES_ALIGN(x) ((((u32)(x))+0x1f) & 0xFFFFFFE0)

static u8 g_rx_desc[GMAC_DESC_SZ * GMAC_RX_DESC_NUM + 0x20];
static u8 g_rx_buf[GMAC_BUF_SZ * GMAC_RX_DESC_NUM + 0x20];
static u8 g_tx_desc[GMAC_DESC_SZ * GMAC_TX_DESC_NUM + 0x20];
static u8 g_tx_buf[GMAC_BUF_SZ + 0x20];
static u8 g_setup_buf[SETUP_FRAME_SZ + 0x20];

unsigned long dbg_runtime_val = 0x1;

/* hw capability */
static int gmac_rx_csum = false;
static int gmac_tx_csum = false;

#define ETH_ALEN 6
static int gmac_debug = -1;
u8 stb_mac_addr[ETH_ALEN] = {0xDE, 0xAD, 0xBE, 0xEF,  0x01, 0x01};

pgmac_private g_pgmac;

void show_rx_desc(prx_desc rx_desc, int no) ;
void enable_gmac_isr (pgmac_private pgmac, int lock);
void phy_reg_set(pgmac_private pgmac) ;
void gmac_link_established(pgmac_private pgmac);
void do_dll_pd_reset(pgmac_private pgmac) ;

static void gmac_print(unsigned char *p, unsigned short len) {
	int i;
	for(i=0; i<len; i++) {
		if (i%16 == 0) {
			printf("\n0x%08x:  ", (UINT32)(p+i));
		}

		printf("%02x ", *(p+i));
	}
	printf("\n");
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

int mac_mdio_read(struct eth_device *dev, int phy_addr, int reg_addr) {
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
mac_mdio_write (struct eth_device *dev, int phy_addr, int reg_addr, int value)
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

	GMAC_W8(SCR, SCRReset|GMAC_R8(SCR));	
	do{
		ali_info("->");	
		tmp_u8 = GMAC_R8(SCR);
	}while(tmp_u8 & SCRReset);
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

void mac_alloc_rings(pgmac_private priv) {
	void *rx_desc = NULL;
	void *tx_desc = NULL;	
	void *setup_buf = NULL;
    u8 *rx_buf = NULL;
    int i = 0;

    rx_desc = GEN_32BYTES_ALIGN(g_rx_desc);
    tx_desc = GEN_32BYTES_ALIGN(g_tx_desc);
    setup_buf = GEN_32BYTES_ALIGN(g_setup_buf);

	priv->rx_desc_dma = __CTDADDRALI(virt_to_phys(rx_desc));
	priv->tx_desc_dma = __CTDADDRALI(virt_to_phys(tx_desc));
	priv->setup_buf = __CTDADDRALI(virt_to_phys(setup_buf));

	memset(rx_desc, 0, (GMAC_DESC_SZ * GMAC_RX_DESC_NUM));
	memset(tx_desc, 0, (GMAC_DESC_SZ * GMAC_TX_DESC_NUM));
	memset(setup_buf, 0, SETUP_FRAME_SZ);

	priv->rx_desc = (prx_desc)rx_desc;
	priv->tx_desc = (ptx_desc)tx_desc;
	priv->setup_buf = (u8 *)setup_buf;

    priv->tx_buf = GEN_32BYTES_ALIGN(g_tx_buf);
    rx_buf = GEN_32BYTES_ALIGN(g_rx_buf);
    for(i=0; i<GMAC_RX_DESC_NUM; i++) {
        priv->rx_buf[i] = rx_buf + i * GMAC_BUF_SZ; 
        priv->rx_desc[i].pkt_buf_dma = __CTDADDRALI(virt_to_phys(priv->rx_buf[i]));
    }

#ifdef TEST_DEBUG
	ali_info("%s()=> rx_desc = 0x%x, pgmac->rx_desc_dma = 0x%x size %d\n", __FUNCTION__, (UINT32)rx_desc, (UINT32)pgmac->rx_desc_dma,
                GMAC_DESC_SZ * GMAC_RX_DESC_NUM);
	ali_info("%s()=> tx_desc = 0x%x, pgmac->tx_desc_dma = 0x%x size %d\n", __FUNCTION__, (UINT32)tx_desc, (UINT32)pgmac->tx_desc_dma,
                GMAC_DESC_SZ * GMAC_RX_DESC_NUM);
#endif
    return;
}

void mac_desc_clean(pgmac_private priv) {
    return;
}

void mac_free_rings(pgmac_private pgmac) {
	mac_desc_clean(pgmac);
    return;
}

void mac_disconnect(pgmac_private pgmac) {

	GMAC_W16(IMR, (ISRTimer|ISRLinkStatus));
	
    pgmac->link_partner = 0;
    pgmac->phy_reset = false;
    pgmac->link_established = false;
    pgmac->transmit_okay = false;
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
	UINT32 tmp_u32_2;
    UINT32 rmii_speed = 0;
    UINT32 rgmii_speed = 0;
	
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

    //printf("Rx Desc num %d   Tx Desc num %d\n", GMAC_RX_DESC_NUM, GMAC_TX_DESC_NUM);
	GMAC_W16(RxRingDesWPtr, GMAC_RX_DESC_NUM -1);
	GMAC_W16(TxRingDesWPtr, 0);

	tmp_8 = (SCRRxEn|SCRTxEn);
#if 0
	if(gmac_tso) {
		tmp_8 |= (SCRTxCoeEn|SCRTsoEn);
    }
	if(gmac_ufo) {
		tmp_8 |= (SCRTxCoeEn|SCRUfoEn);
    }
#endif
	if(gmac_tx_csum) {
		tmp_8 |= SCRTxCoeEn;
    }
	if(gmac_rx_csum) {
		tmp_8 |= SCRRxCoeEn;
    }
	GMAC_W8(SCR, tmp_8);

    //printf("%s SCR 0x%x \n", __FUNCTION__, GMAC_R8(SCR));
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
    mac_hw_start(pgmac);
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
    pgmac->unlink_error_state = 0;
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

int mac_weird_int(pgmac_private pgmac) {
	u16 tmp_u16;
    u16 ctrl, status;
    int link_up = 0;
    u16 tmp1 = 0;
    int j = 0;
    u32 reg32 = 0;
retry:
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
                gmac_link_established(pgmac);
                mac_mdio_write(pgmac->dev, gmac_phy_addr, 0x1d, 7);
                tmp1 = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, 0x1f);
                ali_info("%s: *******PHY 1f value 0x%x.", __FUNCTION__, tmp1);
                printf("gmac link established.\n");
                return 0;
            }
        } else {
#ifndef USE_GPHY
    //        handle_unlink_error(pgmac);
#endif
        }
	} else {
        /* last state is link established */
        status = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, PhyBasicModeStatus);
        ali_trace(1, "%s: cur_isr is ISRLinkStatus|ISRtimer\n", __FUNCTION__);
        if (status & BMSRLinkStatus) {
            tmp_u16 = (u16)mac_mdio_read(pgmac->dev, gmac_phy_addr, PhyNWayLPAR);
            ali_trace(1, "%s: PhyNWayLPAR 0x%x link_partner 0x%x\n", __FUNCTION__, tmp_u16, pgmac->link_partner);
            if (tmp_u16 != pgmac->link_partner) {
                if (tmp_u16) {
                    ali_info("%s tmp_u16 %x link_partner %x link connected.",\
                            __FUNCTION__, tmp_u16, pgmac->link_partner);
                    gmac_link_changed(pgmac);
                }
            }
        } else {
            mac_disconnect(pgmac);
            ali_info("%s: link disconnected.", __FUNCTION__);
        }
	}
    /* 1 mean failed */
    if (++j < 2000) {
        udelay(1000);
        goto retry;
    }
    printf("gmac link doesn't established.\n");
    return -5;
}

u16 mac_rx_update_wptr(pgmac_private pgmac) {
	u16 rx_bptr, rx_rptr, rx_wptr;
	u16 updata = 0;
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
        i = (i + updata) % GMAC_RX_DESC_NUM;
		
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


bool mac_rx_hdr_chk(struct eth_device *dev, prx_desc rx_desc) {
	pgmac_private pgmac = dev->priv;
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
#else
    return true;
#endif
}

int mac_rx_pkts(pgmac_private pgmac, int budget)
{
	prx_desc rx_desc;
	u16 rx_bptr, rx_rptr;
	ppacket_head	pHead;
	u16 pkt_sz, pkts, rx, i;
	rx_rptr = mac_rx_update_wptr(pgmac);
	rx_bptr = pgmac->rx_bptr;
	
	if(rx_rptr >= rx_bptr)
		pkts = rx_rptr - rx_bptr;
	else
		pkts = GMAC_RX_DESC_NUM + rx_rptr - rx_bptr;
	
	if((pkts > 0)&&(rx_rptr == pgmac->rx_wptr))
		pkts -= 1;
	
	i = rx_bptr;
	rx = 0;
	while(pkts > 0) {
		rx_desc = &pgmac->rx_desc[i];
		pHead = &(rx_desc->pkt_hdr);

		if(mac_rx_hdr_chk(pgmac->dev, rx_desc)) {
            pkt_sz = rx_desc->PacketLength - RING_CRC_SZ;
            ali_trace(2, "RECV: pkt_sz=%d, rx_rptr=%d, rx_bptr=%d time %lu", pkt_sz, rx_rptr, rx_bptr, get_timer(0));
            NetReceive(pgmac->rx_buf[i], pkt_sz);
        } else {
			ali_error("packet_head: Head(%08x). rx_desc->pkt_buf_dma=0x%x", *(u32 *)pHead, (u32)rx_desc->pkt_buf_dma);
        }

        if(i == GMAC_RX_DESC_NUM - 1) {
            i = 0;
        } else {
            i ++;
        }
        pkts --;
		rx ++;
	}
	pgmac->rx_bptr = i; 
	return rx;
}

int phy_detect(pgmac_private pgmac) {
	u8 phy_id;
	u16 mode_control_reg;
	u16 status_reg;
	struct eth_device *dev;
	
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
			return 1;
		}
	}

	ali_info("%s: phy address not detected.\n", __FUNCTION__);
	return -1;
}

void phy_reset(pgmac_private pgmac) {
	struct eth_device *dev = pgmac->dev;
	u16 tmp_u16;
	//reset phy registers in a default state.
    tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyBasicModeCtrl);
	mac_mdio_write(dev, gmac_phy_addr, PhyBasicModeCtrl, (tmp_u16 | BMCRReset));

	do {	
		tmp_u16 = (u16)mac_mdio_read(dev, gmac_phy_addr, PhyBasicModeCtrl);
	} while(tmp_u16 & BMCRReset);
	ali_info("phy_set()=>phy reset complete.");
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
	struct eth_device *dev = pgmac->dev; 
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

void disable_gmac_isr (pgmac_private pgmac, int lock) {
    ali_trace(4, "disable IMR (%d) = 0x%x\n", IMR, 0); 
	GMAC_W32(IMR, 0);
    GMAC_R32(IMR);
    return;
}

void enable_gmac_isr (pgmac_private pgmac, int lock) {
    ali_trace(4, "enable IMR (%d) = 0x%x\n", IMR, pgmac->isr_mask); 
	GMAC_W32(IMR, pgmac->isr_mask);
    GMAC_R32(IMR);
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

static int gmac_recv(struct eth_device * dev) {
	pgmac_private pgmac;
	UINT32 cur_isr;

	pgmac = dev->priv;
	cur_isr = GMAC_R32(ISR);
	GMAC_W32(ISR, 0);

	if (cur_isr & pgmac->isr_mask) {
		pgmac->cur_isr = (cur_isr & pgmac->isr_mask);	
	    mac_rx_pkts(pgmac, 0);
	}
    return 0;
}

static int gmac_poweron_sequence(void)
{
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

static int gmac_open (struct eth_device *dev) {
	pgmac_private pgmac = dev->priv;
	UINT32 tmp_u32;

    //printf("%s: enabling interface.\n", dev->name);
    disable_gmac_isr(pgmac, 0);
	
    gmac_poweron_sequence();

	mac_chip_rst();
	
	mac_alloc_rings(pgmac);

	mac_cnt_init(pgmac);

#if 1
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
    phy_reg_set(pgmac);	
	phy_set(pgmac);
    //printf("%s -->\n", __FUNCTION__);
#endif
	return 0;
open_err_out:
	mac_free_rings(pgmac);
	return 1;
}

static void gmac_halt(struct eth_device * dev) {
	pgmac_private priv = dev->priv;
    disable_gmac_isr(priv, 0);
	mac_free_rings(priv);
    return;
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

    return 0;
}

static int gmac_send(struct eth_device *dev, volatile void * packet,\
                   int length) {
	ptx_desc desc;
	volatile u16 tx_rptr;
	u32 off; 
	static tx_data_desc tmp_tx_desc, tmp2_tx_desc; 
	pgmac_private pgmac = dev->priv;
    u16 tmp;
    u32 j = 0;

    if (!pgmac->link_established) {
        printf("%s: link doesn't established\n", __FUNCTION__);
        return -5;
    }
	off = pgmac->tx_wptr;
   // printf("%s --->\n off %d\n", __FUNCTION__, off);
	/***************************************************************************************/
	//2. configure the data descriptor...
    desc = &pgmac->tx_desc[off];
    memset(desc, 0, GMAC_DESC_SZ);
    memcpy(pgmac->tx_buf, packet, length);

    if(off == GMAC_TX_DESC_NUM -1) {
        desc->DataDescriptor.EOR = 1;
    }
    desc->DataDescriptor.seg_len = length;
    desc->DataDescriptor.tot_len = length;
    desc->DataDescriptor.OWN = 1;
    desc->DataDescriptor.ContextData = 0;
    desc->DataDescriptor.FS = 1;
    desc->DataDescriptor.LS = 1;
    desc->DataDescriptor.SegmentNum = 1;
    desc->DataDescriptor.FragmentID = 0; 
    desc->DataDescriptor.CoeEn = 0;
    desc->DataDescriptor.TOE_UFO_En = 0;
    //printf("length %d \n", length);
	desc->DataDescriptor.pkt_buf_dma = __CTDADDRALI(virt_to_phys(pgmac->tx_buf));
	//desc->DataDescriptor.pkt_buf_dma = __CTDADDRALI(pgmac->tx_buf);
    if((++off) >= GMAC_TX_DESC_NUM) {
        off = 0;
    }

	//3. trigger to start DMA...
	memcpy (&tmp_tx_desc, desc, sizeof(tx_data_desc));
	memcpy (&tmp2_tx_desc, desc, sizeof(tx_data_desc));
    
	pgmac->tx_wptr = off;

    j = 0;
    do {
        GMAC_W16(TxRingDesWPtr, off); 
        if(++j > 100000) {
            printf("%s can't update tx_wptr \n", __FUNCTION__);
            return -5;
        }
    } while (GMAC_R16(TxRingDesWPtr) != pgmac->tx_wptr);

    j = 0;
    do {
        tx_rptr = GMAC_R16(TxRingDesRPtr); 
        tmp = GMAC_R16(0x6c); 
        if(++j > 100000) {
            printf("%s mac can't update tx_rptr %d \n", __FUNCTION__, tx_rptr);
            return -5;
        }
    } while (tx_rptr != pgmac->tx_wptr);
    //printf("tx_wptr %d tx_rptr %d\n", tx_rptr, pgmac->tx_wptr);
    //gmac_print(pgmac->tx_buf, length);
	return 0;
}

void init_stb_mac(void) {
#if 0
    u8 tmp_mac_addr[ETH_ALEN];
    u32 board_info_addr = (u32)(phys_to_virt)PRE_DEFINED_ADF_BOOT_START;
    ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO*)board_info_addr;
    memset(tmp_mac_addr, 0, (sizeof(u8)*ETH_ALEN));	
    memcpy(stb_gmac_mac, info->macinfo.phyaddr1, sizeof(u8)*ETH_ALEN);
#endif
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

void phy_reg_set(pgmac_private pgmac) {
#ifndef USE_GPHY
    int phy_reg = 0;
    u16 tmp_u16;
	struct eth_device *dev = pgmac->dev;
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

static void gmac_read_mac(struct eth_device *dev) {
    memcpy(dev->enetaddr, stb_mac_addr, ETH_ALEN);
    return;
}

static int gmac_init(struct eth_device *dev, bd_t *bd) {
	struct gmac_private *priv = dev->priv;
	priv->link_established = false;
    return mac_weird_int(priv);
}

int gmac_write_hwaddr(struct eth_device *dev) {
    return 0;
}

int gmac_register(bd_t * bis) {
    struct eth_device *dev = NULL;
	struct gmac_private *priv = NULL;
    dev = malloc(sizeof(struct eth_device));
    if (!dev) {
        goto malloc_err;
    }

    priv = malloc(sizeof(struct gmac_private));
    if (!priv) {
        goto malloc_err;
    }
    memset(priv, 0, sizeof(*priv));
    memset(dev, 0, sizeof(*dev));
    priv->dev = dev;
    priv->io_base = gmac_base;
	g_pgmac =  priv;

    sprintf(dev->name, "gmac");
    init_stb_mac();
    gmac_read_mac(dev);
    dev->iobase = gmac_base;
    dev->priv = priv;
    dev->init = gmac_init;
    dev->halt = gmac_halt;
    dev->send = gmac_send;
    dev->recv = gmac_recv;
    dev->write_hwaddr = gmac_write_hwaddr;
    eth_register(dev);

    //g_chip_id = (u16) (SOC_R32(0) >> 16);
    //printf("%s chip id %x\n", g_chip_id);
    set_soc_reg();
    gmac_open(dev);
    //mac_weird_int(priv);
    //printf("%s -->\n", __FUNCTION__);
    return 0;
malloc_err:
    if(priv) {
        free(priv);
    }
    if(dev) {
        free(dev);
    }
    return 1;
}
