#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/io.h>
#include <malloc.h>
#include "uboot_ali_sw_reg.h"
#include "uboot_ali_sw_p0.h"
#include "uboot_ali_sw.h"

#define ETH_ALEN 6
u32 ali_sw_base = ALI_SW_BASE;
u32 use_default_mac = 1;
u8 stb_mac_addr[ETH_ALEN] = {0x00, 0x90, 0xe6, 0x00,  0x00, 0x0d};
u8 p0_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x0a};
u8 p1_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x0b};
u8 p2_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x0c};
u8 p3_mac_addr[ETH_ALEN] = { 0x00, 0x90, 0xe6, 0x00,  0x00, 0x0d};

u8	test_forwarding_ports = 0;
#define MAX_PKT_LEN_DWORD (1536/4-1)                                                                                                   
#define MIN_PKT_LEN_DWORD (64/4-2)     /* -1 is crc, -2 is crc+vlan tag */

#define __CTDADDRALI(x) (((u32)(x)) & 0x7fffffff)
#define P0_PHY_ADDRESS 32
#define P1_PHY_ADDRESS 0
#define P2_PHY_ADDRESS 2
#define P3_PHY_ADDRESS 3
static u8 p123_phy_mode[MAX_PORT_NUM] = \
	{0xff, ETH_PHY_MODE_RGMII, ETH_PHY_MODE_RMII, ETH_PHY_MODE_RMII};

static u8 port_phy_addr[MAX_PORT_NUM] = \
	{P0_PHY_ADDRESS, P1_PHY_ADDRESS, P2_PHY_ADDRESS, P3_PHY_ADDRESS};

struct p0_private *gp_p0 = NULL;
struct net_ali_sw *gp_sw = NULL;
int mac_mdio_read(p_p0_private priv, int phy_addr, int reg_addr);
void mac_mdio_write (p_p0_private priv, int phy_addr, int reg_addr, int value);

u32 dbg_runtime_val = 0x0;
/*ToDo: uboot doesn't support dma_alloc_coherent
 *      need reference to 3701c uboot ethernet
 */
/* why add 0x20?  need 32byte align? */
#define GEN_32BYTES_ALIGN(x) ((((u32)(x))+0x1f) & 0xFFFFFFE0)

static u8 g_rx_desc[ALI_SW_DESC_SZ * ALI_SW_RX_DESC_NUM + 0x20];
static u8 g_rx_buf[ALI_SW_BUF_SZ * ALI_SW_RX_DESC_NUM + 0x20];
static u8 g_tx_desc[ALI_SW_DESC_SZ * ALI_SW_TX_DESC_NUM + 0x20];
static u8 g_tx_buf[ALI_SW_BUF_SZ + 0x20];
static u8 g_setup_buf[SETUP_FRAME_SZ + 0x20];

/*--------------------------------------------------------------------*/
static void sw_read_mac(struct eth_device *dev) {
#if 0
    if (eth_getenv_enetaddr("ethaddr", dev->enetaddr)) {
        /* get correct hw addr */
        return;
    } 
#endif
    memcpy(dev->enetaddr, stb_mac_addr, ETH_ALEN);
    return;
}

void set_soc_reg(void) {
    u32 reg32;
	ali_info ("set SOC registers\n");
	reg32 = (*(volatile u32 *)(ali_sw_base - 0x2c000 + 0x6c)); 
	reg32 &=  0x7fffffff;
	*(volatile u32 *)(ali_sw_base - 0x2c000 + 0x6c) = reg32; 
	reg32 = (*(volatile u32 *)(ali_sw_base - 0x2c000 + 0x6c)); 
	ali_info("SOC registers 6c %x\n", reg32);

	reg32 = (*(volatile u32 *)(ali_sw_base - 0x2c000 + 0xa8)); 
	reg32 &= 0xfffffe00;
	reg32 |= 0x109;
	*(volatile u32 *)(ali_sw_base - 0x2c000 + 0xa8) = reg32; 
	ali_info("SOC registers a8 %x\n", reg32);

	reg32 = (*(volatile u32 *)(ali_sw_base - 0x2c000 + 0x8c)); 
	reg32 |= 0x00100000;
	*(volatile u32 *)(ali_sw_base - 0x2c000 + 0x8c) = reg32; 
	ali_info("SOC registers 8c %x\n", reg32);
    return;
}

void sw_irq_enable(void) {
	SW_W8(SysPortAccess, 0);
	SW_W32(P0IMR, P0_INTERRUPT_MASK_SIMPLE);
	SW_R32(P0IMR);
	SW_W8(SysPortAccess, 4);
	SW_W32(FabricIMR, FABRIC_INTERRUPT_MASK);
	SW_R32(FabricIMR);
    return;
}

void sw_irq_disable(void) {
	SW_W8(SysPortAccess, 0);
	SW_W32(P0IMR, 0);
	SW_R32(P0IMR);
	SW_W8(SysPortAccess, 4);
	SW_W32(FabricIMR, 0);
	SW_R32(FabricIMR);
    return;
}

static void ali_sw_chip_rst (void) {
	u8 tmp_u8;
	//u8 i = 4;
	SW_W8(SysPortAccess, 4);//assuming i = 4 right now
	SW_W8(SysCtlSwFabric, SCRSWFRst|SW_R8(SysCtlSwFabric));	
	do{
		SW_W8(SysPortAccess, 4);
        SW_W8(SysCtlSwFabric, SCRSWFRst|SW_R8(SysCtlSwFabric));	
		tmp_u8 = SW_R8(SysCtlSwFabric);
		ali_info("switch fabric is reseting...");
	}while(tmp_u8 & SCRSWFRst);

	SW_W8(SysPortAccess, 4);
	SW_W8(SysCtlSwFabric, SCRSWFEn|SW_R8(SysCtlSwFabric));	
	SW_W8(SysCtlSwFabric, SCRAutoFreshTable|SW_R8(SysCtlSwFabric));	
	//SW_W16(SysCtlSwFabric, (SCRPort0En)|SW_R16(SysCtlSwFabric));	//shouldn't here.
	ali_info("done!reg[SysCtlSwFabric]=0x%x\n", SW_R16(SysCtlSwFabric));
    return;
}

static void set_sw_reg(void) {
	u32 tmp_reg32, tmp1, tmp2, mask;

	/*1.1 disable stp. default stp enabled */
	SW_W8(SysPortAccess, 4);
	SW_W32(PktDecCtl, SW_R32(PktDecCtl)&(~StpEn));

	/* 1.2 set registers */
	SW_W8(SysPortAccess, 4);
	SW_W32(0x188, 0xffffffff);
	ali_info("0x188 reg %x\n", SW_R32(0x188));
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
	ali_info("0x184 reg %x\n", SW_R32(0x184));

	SW_W8(SysPortAccess, 1);
	mask = 0x00000020;
	tmp_reg32 = SW_R32(0x40);
	tmp_reg32 |= mask;
	SW_W32(0x40, tmp_reg32);
	ali_info("0x40 reg %x\n", SW_R32(0x40));

	SW_W8(SysPortAccess, 4);
	SW_W32(0x1c4, 0x10400);
	ali_info("Fabric 0x1c4 %x\n", SW_R32(0x1c4));

#if 0
	SW_W8(SysPortAccess, 4);
	SW_W32(0x154, 0x0);
	SW_W32(0x150, 0x841d4100);
	ali_info("Fabric 0x154 %x\n", SW_R32(0x154));
	SW_W8(SysPortAccess, 4);
	SW_W32(0x154, 0x2ee);
	SW_W32(0x150, 0x841e4100);
	ali_info("Fabric 0x154 %x\n", SW_R32(0x154));
#endif

	SW_W8(SysPortAccess, 4);
	mask = 0x7ffffff;
	mask = ~mask;
	tmp_reg32 = SW_R32(0x190);
	tmp_reg32 &= mask;	
	tmp_reg32 |= 0x24ccccd;
	SW_W32(0x190, tmp_reg32);
	ali_info("Fabric 0x190 %x\n", SW_R32(0x190));

	tmp_reg32 = SW_R32(0x18c);
	mask = 0xC7FFFFFF;
	mask = ~mask;
	tmp_reg32 &= mask;	
	tmp_reg32 |= 0x81000000;
	SW_W32(0x18c, tmp_reg32);
	ali_info("Fabric 0x18c %x\n", SW_R32(0x18c));

	SW_W8(SysPortAccess, 4);
	SW_W32(VlanPortType, (SW_R32(VlanPortType)&(~SW_VlanEn)));
	ali_info("reg[VlanPortType]=0x%x", SW_R32(VlanPortType));
    return;
}

static void p0_set_filter(p_p0_private priv) {
    struct eth_device *dev;
	u8 i;
	u32 tmp_u32;
    dev = priv->dev;
	ali_info("own MAC address: %02x-%02x-%02x-%02x-%02x-%02x", \
				dev->enetaddr[0], dev->enetaddr[1], dev->enetaddr[2], \
				dev->enetaddr[3], dev->enetaddr[4], dev->enetaddr[5]);

	SW_W8(SysPortAccess, 0);
	
	/* configure own MAC address */
	tmp_u32 = 0;
	tmp_u32 = dev->enetaddr[1];
	tmp_u32 <<= 8;
	tmp_u32 |= dev->enetaddr[0];
	SW_W32(SetupFrame_IO, tmp_u32);

	tmp_u32 = 0;
	tmp_u32 = dev->enetaddr[3];
	tmp_u32 <<= 8;
	tmp_u32 |= dev->enetaddr[2];
	SW_W32(SetupFrame_IO, tmp_u32);
	
	tmp_u32 = 0;
	tmp_u32 = dev->enetaddr[5];
	tmp_u32 <<=8;
	tmp_u32 |= dev->enetaddr[4];
	SW_W32(SetupFrame_IO, tmp_u32);

    for(i=0; i<15; i++) {
        tmp_u32 = 0xffffffff;
        SW_W32(SetupFrame_IO, tmp_u32);
        tmp_u32 = 0xffffffff;
        SW_W32(SetupFrame_IO, tmp_u32);
        tmp_u32 = 0xffffffff;
        SW_W32(SetupFrame_IO, tmp_u32);
    }
    return;
}

static int sw_set_mac_addr(p_p0_private priv) {
	struct net_ali_sw *sw=priv->p_switch;
	struct net_ali_sw_port *p;
	u32 mac_addr = 0;

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

    /* set p0 own mac and filter mac, doesn't support multicast */
	p0_set_filter(priv);
	return 0;
}

static void p0123_phy_link_state_init(p_p0_private priv) {
	struct net_ali_sw *sw=priv->p_switch;
	u8	i;
	
	for(i=1; i<MAX_PORT_NUM; i++) {
		sw->port_list[i].got_phy = 0;
		sw->port_list[i].phy_rst = 0;
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
    return;
}

static void p123_set_linkcheck_mode(p_p0_private priv) {
	u32 tmp=0;
	u8 i;
	for(i=1; i<MAX_PORT_NUM; i++) {
		if(i==1) {
			tmp|=P1PhyActive;
			/* tmp|=P1PhyType; */
			tmp|=(port_phy_addr[1]<<P1PhyAddrOffset);
		} else if(i==2) {
			tmp|=P2PhyActive;
			/* tmp|=P2PhyType; */
			tmp|=(port_phy_addr[2]<<P2PhyAddrOffset);
		} else if(i==3) {
			tmp|=P3PhyActive;
			/* tmp|=P3PhyType; */
			tmp|=(port_phy_addr[3]<<P3PhyAddrOffset);
		}
	}

	SW_W8(SysPortAccess, 4);
	SW_W32(LinkCheckCtl, tmp);
    return;
}

static bool p123_phy_detect(p_p0_private priv) {
	struct net_ali_sw *sw=priv->p_switch;
	unsigned short		mode_control_reg;
	unsigned short		status_reg;
	u8	i, cnt=0;
	u32 reg_val;

	SW_W8(SysPortAccess, 4);
	reg_val = SW_R32(LinkCheckCtl);
	reg_val &= (~AutoMdioEn);
	SW_W8(SysPortAccess, 4);
	SW_W32(LinkCheckCtl, reg_val);

	for(i=1; i<MAX_PORT_NUM; i++) {
		mode_control_reg = 0;
		status_reg = 0;
		sw->port_list[i].got_phy = 1;

		mode_control_reg = (unsigned short)mac_mdio_read(priv, port_phy_addr[i], PhyBasicModeCtrl);
		status_reg = (unsigned short)mac_mdio_read(priv, port_phy_addr[i], PhyBasicModeStatus);
		
		if ((status_reg != 0xffff) && (status_reg != 0)) {
			ali_info("%s: port[%d] phy address[%d] is right. PhyBasicModeCtrl=0x%x, PhyBasicModeStatus=0x%x", \
						__FUNCTION__, i, port_phy_addr[i], mode_control_reg, status_reg);
			continue;
		}

		ali_error("%s: port[%d] phy address is wrong. :(", __FUNCTION__, i);
		sw->port_list[i].got_phy = 0;
		cnt++;
	}

	if(cnt == MAX_PORT_NUM-1) {	
		ali_error("%s: no phy detected on any port!Freaking wired!", __FUNCTION__);
		return false;
	}
	return true;
}

static bool p0123_port_is_in_use(p_p0_private priv, u8 port_no) {
	struct net_ali_sw *sw=priv->p_switch;
	return (sw->port_list[port_no].got_phy);
}

int mac_mdio_read(p_p0_private priv, int phy_addr, int reg_addr) {
	u16 tmp_16;
	u32 tmp_32;
	u32 addr;

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

	do {
		SW_W8(SysPortAccess, 4);
		tmp_32 = SW_R32(MdioCtl);		
		mdelay(1);
		ali_trace(7, ".");
	} while(tmp_32&MdioHWTansStart);
	ali_trace(7, "\n");

	SW_W8(SysPortAccess, 4);
	tmp_16 = SW_R16(MdioRData);
	return (u32)tmp_16;
}

void mac_mdio_write (p_p0_private priv, int phy_addr, int reg_addr, int value) {
	u32 tmp_32;
	u32 addr;

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
			
	do {
		SW_W8(SysPortAccess, 4);
		tmp_32 = SW_R32(MdioCtl);
	} while(tmp_32&MdioHWTansStart);
    return;
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
		do {	
			mdelay(5);
			ali_info("read phy port %d status \n", port_no);
			tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], PhyBasicModeCtrl);
		} while(tmp_u16 & BMCRReset);
	}

	if (port_no == 2) {
		ali_info("disable all FSM\n");
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x10);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x10, tmp_u16|0x1);

		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x08);
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1e);
		tmp_u16 &= 0xfc7f;
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1e, tmp_u16);

		ali_info("set phy sock\n");
		tmp_u32 = (*(volatile u32 *)(ali_sw_base - 0x2c000 + 0x6c)); 
		tmp_u32 |= 0x2;
		*(volatile u32 *)(ali_sw_base - 0x2c000 + 0x6c) = tmp_u32; 
		mdelay(1);

		tmp_u32 = (*(volatile u32 *)(ali_sw_base - 0x2c000 + 0x6c)); 
		tmp_u32 &= 0xfffffffd;
		*(volatile u32 *)(ali_sw_base - 0x2c000 + 0x6c) = tmp_u32; 
		mdelay(4);

		ali_info("set port 2 ephy AGC\n");
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x07);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1f, 0x4151);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x00);

		/* to fix can't link with asus sw */
		ali_info("Fix asus \n");
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x8);
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1f);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1f, (tmp_u16 | 0x1));
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1f);
		ali_info("PHY %d 0x1f %x\n", port_no, tmp_u16);

		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x09);
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1e);
		tmp_u16 &= 0xFFFC;
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1e, (tmp_u16 | 0x2));
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1e);
		ali_info("PHY %d 0x1e %x\n", port_no, tmp_u16);
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1c);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1c, (tmp_u16 | 0x7));
		tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1c);
		ali_info("PHY %d 0x1c %x\n", port_no, tmp_u16);

		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x6);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x1e, 0x0104);

		mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0);
		mac_mdio_write(priv, port_phy_addr[port_no], 0x10, 0);
	}
	sw->port_list[port_no].phy_rst= true;//phy_reset is set after phy_detect and phy_set!
	return true;
}


static void p0_alloc_descriptors(p_p0_private priv) {
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

	memset(rx_desc, 0, (ALI_SW_DESC_SZ * ALI_SW_RX_DESC_NUM));
	memset(tx_desc, 0, (ALI_SW_DESC_SZ * ALI_SW_TX_DESC_NUM));
	memset(setup_buf, 0, SETUP_FRAME_SZ);

	priv->rx_desc = (p_rx_desc)rx_desc;
	priv->tx_desc = (p_tx_desc)tx_desc;
	priv->setup_buf = (u8 *)setup_buf;//question, we actually don't use it...

    priv->tx_buf = GEN_32BYTES_ALIGN(g_tx_buf);
    rx_buf = GEN_32BYTES_ALIGN(g_rx_buf);
    for(i=0; i<ALI_SW_RX_DESC_NUM; i++) {
        priv->rx_buf[i] = rx_buf + i * ALI_SW_BUF_SZ; 
        priv->rx_desc[i].pkt_buf_dma = __CTDADDRALI(virt_to_phys(priv->rx_buf[i]));
    }
	ali_info("%s()=> rx_desc = 0x%x, rx_desc_dma = 0x%x", __FUNCTION__, (u32)priv->rx_desc, (u32)priv->rx_desc_dma);
	ali_info("%s()=> tx_desc = 0x%x, tx_desc_dma = 0x%x", __FUNCTION__, (u32)priv->tx_desc, (u32)priv->tx_desc_dma);
    return;
}

static void p0_cnt_init(p_p0_private priv) {
	priv->fabric_isr= 0;
	priv->p0_isr= 0;
	priv->avail_desc_num = ALI_SW_TX_DESC_NUM - 1;
	priv->rx_wptr = ALI_SW_RX_DESC_NUM -1;
	priv->rx_bptr= 0;
	priv->tx_wptr= 0;	
    return;
}

/* ToDo: modify dma_unmap_single */
static void p0_desc_clean(p_p0_private priv) {
    return;
}

/* ToDo: need modify dma_free_coherent */
static void p0_free_rings(p_p0_private priv) {
	p0_desc_clean(priv);
    return;
}

static bool p0123_port_enable(u8 port_no) {
	u16 tmp_16;

	SW_W8(SysPortAccess, 4);
	SW_W8(SysPortAccess, port_no);

	SW_W8(SysPortAccess, 4);
	tmp_16 = SW_R16(SysCtlSwFabric);
	
	if(port_no==0) {	
		tmp_16|=SCRPort0En;
	} else if(port_no==1) {
		tmp_16|=SCRPort1En;
    } else if(port_no==2) {
		tmp_16|=SCRPort2En;
    } else if(port_no==3) {
		tmp_16|=SCRPort3En;
    } else {	
		ali_error("%s: wrong port_no input=%d!\n", __FUNCTION__, port_no);
		return false;
	}

	SW_W8(SysPortAccess, 4);
	SW_W16(SysCtlSwFabric, tmp_16);
	SW_W8(SysPortAccess, 4);
	SW_W8(SysPortAccess, port_no);
	return true;
}

static void p0_get_start(p_p0_private priv) {
	u32 tmp_u32;
	
	SW_W8(SysPortAccess, 0);

	SW_W32(TxDesStartAddr, priv->tx_desc_dma);
	SW_W32(RxDesStartAddr, priv->rx_desc_dma);

	SW_W16(RxDesTotNum, ALI_SW_RX_DESC_NUM);
	SW_W16(TxDesTotNum, ALI_SW_RX_DESC_NUM);

	SW_W16(RxRingDesWPtr, ALI_SW_RX_DESC_NUM -1);
	SW_W16(TxRingDesWPtr, 0);
	
	SW_W8(SysPortAccess, 0);
#if 0
	if(p0_tx_csum) {
		tmp_8 |= SCRTxCoeEn;
    }
	if(p0_rx_csum) {
		tmp_8 |= SCRRxCoeEn;
    }
	SW_W8(SysCtlP0, tmp_8);
#endif

    /* set filter table */
	SW_W8(SysPortAccess, 0);
	tmp_u32 = SW_R32(P0FilterMode);
	ali_info("default reg[P0FilterMode]=0x%x", tmp_u32);
	tmp_u32 &= ~(FILTER_MODE_MSK);

    if (priv->filter_mode == FILTER_ALLMULTI_MODE) {
        SW_W32(P0FilterMode, (tmp_u32|PASS_ALL_MULTICAST));
        p0_set_filter(priv); /* maybe doesn't need caill it , because call it in init func */
        ali_info("P0_MC_FILTER_MODE_IS_ALLMULTI");
    } else {
        ali_info("uboot doesn't support others filter mode except ALLMULTI_MODE\n");
    }
	
	p0123_port_enable(0);
	ali_info("get p0 started, reg[SysCtlSwFabric] = 0x%x", SW_R16(SysCtlSwFabric));
    return;	
}

static int sw_open(struct eth_device * dev) {
	u32 i;
	p_p0_private priv = dev->priv;

    sw_irq_disable();
    /* step 1: reset&enable sw */
    ali_sw_chip_rst();

    /* step 2: set some switch regs */
    set_sw_reg();

    /* step 3: set mac address */
	sw_set_mac_addr(priv);
    p0123_phy_link_state_init(priv);

    /* step 4: set link ctrl reg */
    p123_set_linkcheck_mode(priv);
    
    /* step 5: detect phy */
	if (!p123_phy_detect(priv)) {
		ali_error("no phy detected, close switch!\n");
		goto open_err_out;
	}
    
    /* step 6: phy rst, set some phy reg */
	for(i=1; i<MAX_PORT_NUM; i++) {
		p123_phy_rst (priv, i);
    }
    
    /* step 7: memory alloc for p0 */
	p0_alloc_descriptors(priv);

    /* step 8: init for sw cnt */
    p0_cnt_init(priv);

    /* step 9: p0 start */
    p0_get_start(priv);
    sw_irq_enable();
	//ali_warn("ALi switch opened!");
	return 0;
open_err_out:
	p0_free_rings(priv);
	return 1;	
}

static int sw_init(struct eth_device *dev, bd_t *bd) {
	struct p0_private *priv = dev->priv;
    ali_info("%s -->\n", __FUNCTION__);
    set_soc_reg();
    sw_open(dev);
    return _new_ali_sw_super_housekeeper(dev->priv);
}

static int sw_send(struct eth_device *dev, volatile void * packet,\
                   int length) {
	volatile p_tx_desc desc;
	volatile u16 tx_rptr;
	volatile u16 off;
	p_p0_private priv = NULL;
    u8 forward_ports = 0;
    priv = dev->priv;
	off = priv->tx_wptr;
    forward_ports = test_forwarding_ports;

    desc = &priv->tx_desc[off];
    memset(desc, 0, ALI_SW_DESC_SZ);

    if(off == ALI_SW_TX_DESC_NUM -1) {
        desc->DataDescriptor.EOR = 1;
    }
    desc->DataDescriptor.seg_len = length;
    desc->DataDescriptor.tot_len = length;
    desc->DataDescriptor.OWN = 1;
    desc->DataDescriptor.ContextData = 0;
    desc->DataDescriptor.FS = 1;
    desc->DataDescriptor.SegmentNum = 1;
    desc->DataDescriptor.TX_PRIOR_EN = 0;
    desc->DataDescriptor.ForwardPorts = forward_ports;
    desc->DataDescriptor.ContextIndex = 0;
    desc->DataDescriptor.FragmentID = 0;
    desc->DataDescriptor.CoeEn = 0;
    desc->DataDescriptor.TOE_UFO_En = 0;
		
    memcpy(priv->tx_buf, packet, length);
	desc->DataDescriptor.pkt_buf_dma = __CTDADDRALI(priv->tx_buf);
    if((++off) >= ALI_SW_TX_DESC_NUM) {
        off = 0;
    }
	priv->tx_wptr = off;
	do {
		SW_W8(SysPortAccess, 0);
		SW_W16(TxRingDesWPtr, off); 
	} while (SW_R16(TxRingDesWPtr) != priv->tx_wptr);

	do {
		SW_W8(SysPortAccess, 0);
		tx_rptr = SW_R16(TxRingDesRPtr);
	} while(priv->tx_wptr != tx_rptr);
    //ali_warn("send on pkts priv->tx_wptr %d tx_rptr %d\n", priv->tx_wptr, tx_rptr);
    return 0;
}

static void sw_halt(struct eth_device * dev) {
	p_p0_private priv = dev->priv; 
    sw_irq_disable();
	p0_free_rings(priv);
    return;
}

bool new_p123_phy_link_chk(struct net_ali_sw_port *p) {
	p_p0_private priv = p->p_P0;//TBD, seems redundant
		
	u16 advertisement, link;
	unsigned long	pause;
	unsigned long asm_dir;
	unsigned long	partner_pause;
	unsigned long partner_asm_dir;
	unsigned short expansion = 0;
	u16 status;
	u16 giga_status;
	u16 giga_ctrl;
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
    ali_trace(7, "port %d status %x advertisement %x expansion %x link %x giga_ctrl %x giga_status %x\n",p->port_no, \
                status, advertisement, expansion, link, giga_ctrl, giga_status) 
	/* auto nego and complete nego */
	if ((status & BMCRANEnable) && (status & BMSRANComplete)) {
		//used for LinkStatusChg check.	
		p->link_partner = link; 
		if (expansion & 0x1) { /* partner support auto nego */
			if ((giga_ctrl & T_1000_FD) && (giga_status & LP_1000_FD)) {
				ali_info("1000 Base-TX full duplex.");
				p->link_dup = true;
				p->link_spd = LINK_SPEED_1000M;
			} else if ((giga_ctrl & T_1000_HD) && (giga_status & LP_1000_HD)) {
				ali_info("1000 Base-TX half duplex.");
				p->link_dup = false;
				p->link_spd = LINK_SPEED_1000M;
			} else if ((advertisement & ANARTXFD) && (link & ANARTXFD)) {
				ali_info("100 Base-TX full duplex.");
				p->link_dup = true;
				p->link_spd = LINK_SPEED_100M;
			} else if ((advertisement & ANARTX) && (link & ANARTX)) {
				ali_info("100 Base-TX half duplex.");
				p->link_dup = false;
				p->link_spd = LINK_SPEED_100M;
			} else if ((advertisement & ANAR10FD) && (link & ANAR10FD)) {
				ali_info("10 Base-TX full duplex.");
				p->link_dup = true;
				p->link_spd = LINK_SPEED_10M;
			} else {
				ali_info("10 Base-TX half duplex.");
				p->link_dup = false;
				p->link_spd = LINK_SPEED_10M;
			}
		} else { /* partner doesn't support auto nego */
			if (link & ANARTXFD) {
				p->link_dup = true;
				p->link_spd = LINK_SPEED_100M;
				ali_info("100 Base-TX full duplex.");
			} else if (link & ANARTX) {
				p->link_dup = false;
				p->link_spd = LINK_SPEED_100M;
				ali_info("100 Base-TX half duplex.");
			} 
			else if (link & ANAR10FD) {
				p->link_dup = true;
				p->link_spd = LINK_SPEED_10M;
				ali_info("10 Base-TX full duplex.");
			} else if (link & ANAR10) {
				p->link_dup = false;
				p->link_spd = LINK_SPEED_10M;
				ali_info("10 Base-TX half duplex.");
			} else {
				ali_info("link check doesn't get corrent speed set 10M half\n");
				p->link_dup = false;
				p->link_spd = LINK_SPEED_10M;
			}
		}
	} else if (!(status & BMCRANEnable)) {
		if (status & BMCRDuplexMode) {
			p->link_dup = true;
		} else {
			p->link_dup = false;
		}
		if (!(status & BMCRSpeedSet6) && (status & BMCRSpeedSet13)) {
			p->link_spd = LINK_SPEED_100M;
		} else if (!(status & BMCRSpeedSet13) && !(status & BMCRSpeedSet6)) {
			p->link_spd = LINK_SPEED_10M;
		} else {
			p->link_spd = LINK_SPEED_10M;
			ali_info("%s can't get valid speed, set to 10M\n", __FUNCTION__);
		}
	} else {
		ali_info("error, shouldn't call %s\n", __FUNCTION__);
		return -1;
	}

	pause = (advertisement & ANARPause) > 0 ? 1 : 0;
	asm_dir = (advertisement & ANARASMDIR) > 0 ? 1 : 0;
	partner_pause = (link & ANARPause) > 0 ? 1 : 0;
	partner_asm_dir = (link & ANARASMDIR) > 0 ? 1 : 0;

	ali_info(" Pause&ASM=%ld%ld%ld%ld. ", pause, asm_dir, partner_pause, partner_asm_dir);

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
		ali_info("%s: impossiable!\n", __FUNCTION__);
	}

	ali_info("Tx flowctl=%ld, Rx flowctl=%ld.\n", (long)p->rx_pause, (long)p->tx_pause);
	return 0;
}

static bool p123_port_rst(u8 port_no) {
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
		ali_error("%s: wrong port_no input!\n", __FUNCTION__);
		return false;
	}
	
	return true;
}

static void p123_mac_set(p_p0_private priv, u8 port_no)
{
	struct net_ali_sw *sw=priv->p_switch;
	u32 speed, tmp, duplex_mode, pause_frame=0;
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
		ali_error("%s: unsupported port_no input!", __FUNCTION__);
		BUG();
	}

	//step2. set MII reg
	SW_W8(SysPortAccess, port_no);
	tmp = SW_R32(RmiiCR);
	tmp &= ~(RmiiEn |RgmiiEn);
		
	if (p123_phy_mode[port_no] == ETH_PHY_MODE_RGMII)
	{
		tmp|=RgmiiEn;
		SW_W8(SysPortAccess, port_no);
		SW_W32(RmiiCR, (tmp));
		ali_info("RGMII port_no=%d, tmp=0x%x", port_no, tmp);
		ali_info("RGMII port_no=%d, Reg(RmiiCR)=0x%x", port_no, SW_R32(RmiiCR));
	}
	else if(p123_phy_mode[port_no] == ETH_PHY_MODE_RMII)
	{
		SW_W8(SysPortAccess, port_no);
		SW_W32(RmiiCR, (tmp |RmiiEn));//actually it doesn't look at this, always mii.  tbd.
		ali_info("RMII port_no=%d, reg[RmiiCR]=0x%x\n", port_no, SW_R32(RmiiCR));
	}
	else
	{
		ali_error("unsupported MII mode!\n");
		BUG();
	}
	
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
			//if(ali_info_ratelimit())
			//	ali_info(".");
		}
	ali_info("port_no=%d, reg[RmiiCR]=0x%x", port_no, SW_R32(RmiiCR));
	
	//step2. set NET operation mode.
	if (sw->port_list[port_no].link_spd == LINK_SPEED_1000M)
	{
		if(p123_phy_mode[port_no] != ETH_PHY_MODE_RGMII)
		{
			ali_error("uncorrect MII mode!\n");
			BUG();
		}
		speed = (u32)(SW_SPEED_1000MBPS);	//1000Mbps
	}
	else if (sw->port_list[port_no].link_spd == LINK_SPEED_100M)
	{
		speed = (u32)(SW_SPEED_100MBPS); //100Mbps
	}
	else if (sw->port_list[port_no].link_spd == LINK_SPEED_10M)
	{
		speed = SW_SPEED_10MBPS; //10mbps
	}
	else
	{
		ali_error("link_speed undetected yet, shouldn't be here!\n");
		BUG();
	}
		
	if (sw->port_list[port_no].link_dup)
		duplex_mode = (u32)FullDuplexMode;
	else
		duplex_mode = (u32)0;

	if (sw->port_list[port_no].rx_pause)
		pause_frame |= (u32)RxFlowCtlEn;
	if (sw->port_list[port_no].tx_pause)
		pause_frame |= (u32)TxFlowCtlEn;
	
	tmp = NetworkOMConfig|duplex_mode|speed|pause_frame;
	SW_W8(SysPortAccess, port_no);
	SW_W32(NetOpMode, tmp);
    return;
}

static void p123_get_start(p_p0_private priv, u8 port_no) {
	//ali_sw_get_stats(priv->dev);
		
	//step1. port reset..	
	p123_port_rst(port_no);

	//step2. setting p123's mac.	
	p123_mac_set(priv, port_no);
	
	//step3. enable RX/TX.	
	p0123_port_enable(port_no);

	SW_W8(SysPortAccess, port_no);
	SW_W8(SysPortAccess, 4);

	ali_info("port[%d] get started, reg[SysCtlP123]=0x%x, reg[SysCtlSwFabric]=0x%x", port_no, SW_R32(SysCtlP123), SW_R32(SysCtlSwFabric));
}

void p123_link_established(struct net_ali_sw_port *p) {
	new_p123_phy_link_chk(p);
	p123_get_start(p->p_P0, p->port_no);
	p->transmit_okay = true;
}

void new_p123_link_changing(struct net_ali_sw_port *p) {
	if (p->link_established == true) {
		p123_link_established(p);
	} else {
		ali_error("%s: shoudn't be here", __FUNCTION__);
	}
}

static void _ali_sw_get_stats(struct eth_device *dev) {
	p_p0_private priv = dev->priv;
	struct net_ali_sw *sw = priv->p_switch;
	u8 port_no = 1;
	u32 tmp;

    SW_W8(SysPortAccess, 0);
    
    tmp = SW_R16(IPCksFailCnt);
    SW_W16(IPCksFailCnt, 0);
    priv->mac_stats.rx_ip_chksum_err_from_reg += tmp;

    tmp = SW_R16(IPPayloadCksFailCnt);
    SW_W16(IPPayloadCksFailCnt, 0);
    priv->mac_stats.rx_ip_data_chksum_err_from_reg+= tmp;

    for(port_no = 1; port_no<MAX_PORT_NUM; port_no++) {
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
	return;
}

static bool p0123_port_disable(u8 port_no) {
	u16 tmp_16;
	
	SW_W32(SysPortAccess, 4);
	tmp_16 = SW_R16(SysCtlSwFabric);

	if(port_no==0) {	
		tmp_16&=~SCRPort0En;
	} else if(port_no==1) {
		tmp_16&=~SCRPort1En;
    } else if(port_no==2) {
		tmp_16&=~SCRPort2En;
    } else if(port_no==3) {
		tmp_16&=~SCRPort3En;
    } else {	
		ali_error("%s: wrong port_no input!\n", __FUNCTION__);
		return false;
	}
	SW_W32(SysPortAccess, 4);
	SW_W16(SysCtlSwFabric, tmp_16);
	return true;
}

void p123_link_disconnected(struct net_ali_sw_port *p) {
    p->link_partner = 0;		
    p->phy_rst = false;		//set after phy_set
    p->link_established = false;	//set after auto-neg
    p->transmit_okay = false;	//
    _ali_sw_get_stats(p->p_P0->dev);
    p0123_port_disable(p->port_no);//locking? tbd?
    return;
}

int _new_ali_sw_super_housekeeper(p_p0_private priv) {
	struct net_ali_sw *sw=priv->p_switch;
	struct net_ali_sw_port *p;
	u8 port_no=0;
	u16 status;
    u16 mode_control_reg;
	u16 ctrl;
	int link_changed = 0;
	u16 tmp_u16;
    u32 tmp32;
    u32 link_established = 0;
    u32 j = 0;

    ali_info("%s -->\n", __FUNCTION__);
	if(unlikely(sw==NULL)) {	
		ali_error("%s: sw is null\n", __FUNCTION__);
		return 1;
	}
	
retry:
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
					ali_info("ali sw: port[%d]=>auto nego link established.", p->port_no);
					link_changed = 1;
				} else if (!(ctrl & BMCRANEnable)) {
					ali_info("ali sw:port[%d]=>non auto nego link established.", p->port_no);
					link_changed = 1;
				}
				if (link_changed) {
					p->link_established= true;
					if (port_no == 1) {
						ali_info("giga phy rx \n");
						mac_mdio_write(priv, port_phy_addr[port_no], 0x1d, 0x00);
						mac_mdio_write(priv, port_phy_addr[port_no], 0x1e, 0x2ee);
						tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], 0x1e);
						ali_info("port %d: 1e %x\n", port_no, tmp_u16);

                        /* to improve 10 100 half speed performance */ 
                        SW_W8(SysPortAccess, 1);
                        tmp32 = SW_R32(TxRxCfg);
                        ali_info("TxRxCfg is %x\n", tmp32);
                        tmp32 &= 0xFFFFFC00;
                        tmp32 |= (0x9 << 5);
                        tmp32 |= 0x10;
                        SW_W32(TxRxCfg, tmp32);
                        tmp32 = SW_R32(TxRxCfg);
                        ali_info("TxRxCfg changed to  %x\n", tmp32);
					}
					p123_link_established(p);
				}
			}
		} else { /*last state is up*/
			if (status & BMSRLinkStatus) {
				tmp_u16 = (u16)mac_mdio_read(priv, port_phy_addr[port_no], PhyNWayLPAR);
				if (tmp_u16 != p->link_partner && tmp_u16) {
					ali_info("ali sw port[%d]=>link changes while up.", p->port_no);
					new_p123_link_changing(p);
				}
			} else {
				ali_info("ali sw port[%d]=>link disconnected.", p->port_no);
				p123_link_disconnected(p);
			}
        }
	} /* for (port_no = 1; */	

	for(port_no=1; port_no<MAX_PORT_NUM; port_no++) {
		p = &(sw->port_list[port_no]);
        if(p->transmit_okay) {
            /* 0 mean success */
            ali_info("%s port_no %d is ok\n", __FUNCTION__, port_no);
            return 0;
        }
    }
    /* 1 mean failed */
    if (++j < 1500) {
        udelay(1000);
        goto retry;
    }
    ali_warn("link doesn't established!!!\n");
	return -5;
}

static u16 p0_rx_update_wptr(p_p0_private priv) {
	u16 rx_bptr, rx_rptr, rx_wptr;
	u16 updata = 0;
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
	if(updata > 0) {
		i = rx_wptr;
        i = (i + updata) % ALI_SW_RX_DESC_NUM;
#if 0
		while(updata > 0)
		{
			if(i == ALI_SW_RX_DESC_NUM - 1){
				i = 0;
			} else {
				i ++;
			}		
			updata --;
		}
		
#endif
		priv->rx_wptr = i;
		SW_W8(SysPortAccess, 0);
		SW_W16(RxRingDesWPtr, priv->rx_wptr);
	}

	return rx_rptr;

rx_lost:
	ali_warn("%s()=>rx_bptr got lost, rx_wptr:%d, rx_bptr:%d, rx_rptr:%d.", __FUNCTION__, rx_wptr, rx_bptr, rx_rptr);
	return rx_rptr;
}

static bool p0_rx_desc_check(p_p0_private priv, p_rx_desc rx_desc, u8 *from_port_no) {
	ppacket_head pHead;

	pHead = &(rx_desc->pkt_hdr);

	if((rx_desc->PacketLength > MAX_PKT_LEN_DWORD) || \
       (rx_desc->PacketLength < MIN_PKT_LEN_DWORD)) {
		ali_error("RX=>PacketLength err %d", rx_desc->PacketLength);
		return false;
	}
	
	if (pHead->UniFrame) {
		priv->mac_stats.rx_uc++;
    }
	if (pHead->BroFrame) {
		priv->mac_stats.rx_bc++;
    }
	if (pHead->MulFrame) {
		priv->mac_stats.rx_mc++;
    }
	if (pHead->IPFrame) {
		priv->mac_stats.rx_ip++;
    }
	if (pHead->IP6Frame) {
		priv->mac_stats.rx_ipv6++;
    }
	if (pHead->IPFrag) {
		priv->mac_stats.rx_frag++;
    }
	if (pHead->TCPpkt) {
		priv->mac_stats.rx_tcp++;
    }
	if (pHead->UDPpkt) {
		priv->mac_stats.rx_udp++;
    }
	if (!pHead->L2FrameType) {
		priv->mac_stats.rx_8023++;
    }
	if (pHead->PPPoE) {
		priv->mac_stats.rx_pppoe++;
    }
	if (pHead->SNAP) {
		priv->mac_stats.rx_snap++;
    }
	if (pHead->BPDU) {
		priv->mac_stats.rx_bpdu++;
    }
	if (pHead->IGMP_MLD) {
		priv->mac_stats.rx_igmp_mld++;
    }
	if (pHead->GMRP_MMRP) {
		priv->mac_stats.rx_gmrp_mmrp++;
    }
	if (pHead->GVRP_MVRP) {
		priv->mac_stats.rx_gvrp_mvrp++;
    }
	if (pHead->GARP) {
		priv->mac_stats.rx_garp++;
    }

	*from_port_no = pHead->PortFrame;
	(priv->mac_stats.rx_from_port[pHead->PortFrame])++;
	return true;	
}

static bool p0_rx_hw_cks_ok(p_p0_private priv, ppacket_head pHead) {	
    bool p0_rx_csum = false; 
	if((p0_rx_csum == true) && (pHead->IPFrame || pHead->IP6Frame)) {
		if(pHead->IPFrag) {
			goto Soft_Cks;
        }

		if(!pHead->IP6Frame && !pHead->IPChksum) {
            priv->mac_stats.rx_ip_chksum_err_from_desc++;
            goto Soft_Cks;
        }

		if ((pHead->TCPpkt || pHead->UDPpkt) && !pHead->IPDChksum) {
            priv->mac_stats.rx_ip_data_chksum_err_from_desc++;
            ali_warn("=>ip data checksum err\n");
            goto Soft_Cks;
        }
		return true;
	}
Soft_Cks:
	return false;
}

static int p0_rx_pkts (p_p0_private priv, int budget) {
	p_rx_desc rx_desc;
	u16 rx_bptr, rx_rptr;
	ppacket_head	pHead;
	u16 pkt_sz, pkts, rx, i;
	u8 from_port_no;

	rx_rptr = p0_rx_update_wptr(priv);
	
	rx_bptr = priv->rx_bptr;
	if(rx_rptr >= rx_bptr) {
		pkts = rx_rptr - rx_bptr;
    } else {
		pkts = ALI_SW_RX_DESC_NUM + rx_rptr - rx_bptr;
    }
	
	if((pkts > 0)&&(rx_rptr == priv->rx_wptr)) {
		pkts -= 1;
    }

	if((budget != 0)&&(pkts > budget)) {
		pkts = budget;
	}

	i = rx_bptr;
	rx = 0;

    ali_trace(2, "RECV: pkts=%d, rx_rptr=%d, rx_bptr=%d time %lu", pkts, rx_rptr, rx_bptr, get_timer(0));
	while(pkts > 0) {
		rx_desc = &priv->rx_desc[i];
		pHead = &(rx_desc->pkt_hdr);
		
		if (p0_rx_desc_check(priv, rx_desc, &from_port_no)) {
			pkt_sz = (rx_desc->PacketLength) << 2;
            ali_trace(2, "RECV: pkt_sz=%d, rx_rptr=%d, rx_bptr=%d time %lu", pkt_sz, rx_rptr, rx_bptr, get_timer(0));
            NetReceive(priv->rx_buf[i], pkt_sz);
		} else {
			ali_error("packet_head: Head(%08x). rx_desc->pkt_buf_dma=0x%x", *(u32 *)pHead, (u32)rx_desc->pkt_buf_dma);
		}

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

static int sw_recv(struct eth_device * dev) {
	p_p0_private priv;
	u32 p0_isr, fabric_isr; 
    priv = dev->priv;

	SW_W8(SysPortAccess, 4);
	fabric_isr = SW_R32(FabricISR);

	SW_W8(SysPortAccess, 0);
	p0_isr = SW_R32(P0ISR);

	if (fabric_isr & FABRIC_INTERRUPT_MASK) {
        SW_W32(FabricISR, ~FABRIC_INTERRUPT_MASK);
        SW_W32(P0ISR, 0);
        /* handle rx pkts */
        p0_rx_pkts(priv, 0);

        /* add watch dog for link change */
        if (fabric_isr & GP_TIMER_EXPIRED) {
           // _new_ali_sw_super_housekeeper(priv);
            fabric_isr &= ~GP_TIMER_EXPIRED;
        }
	}
    return 0;
}

int sw_write_hwaddr(struct eth_device *dev) {
    return 0;
}

int sw_register(bd_t * bis) {
    struct eth_device *dev = NULL;
    struct p0_private *priv = NULL;
    struct net_ali_sw *sw = NULL;
	struct net_ali_sw_port *sw_port = NULL;
    int i = 0;

    priv = malloc(sizeof(struct p0_private));
    if (!priv) {
        goto malloc_err;
    }
    dev = malloc(sizeof(struct eth_device));
    if (!dev) {
        goto malloc_err;
    }
    sw = (struct net_ali_sw*)malloc(sizeof(struct net_ali_sw));
    if (!sw) {
        goto malloc_err;
    }

    memset(priv, 0, sizeof(*priv));
    memset(dev, 0, sizeof(*dev));
    memset(sw, 0, sizeof(struct net_ali_sw));

    gp_p0 = priv;
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
    priv->filter_mode = FILTER_ALLMULTI_MODE; 
	priv->io_base = ali_sw_base;
/* init eth_device */
    sw_read_mac(dev);
    sprintf(dev->name, "sw");
    dev->iobase = ali_sw_base;
    dev->priv = priv;
    dev->init = sw_init;
    dev->halt = sw_halt;
    dev->send = sw_send;
    dev->recv = sw_recv;
    dev->write_hwaddr = sw_write_hwaddr;
    priv->dev = dev;
    eth_register(dev);
    ali_info("%s -->\n", __FUNCTION__);
#ifdef CONFIG_CMD_MII
   // miiphy_register(dev->name, sw_mii_read, sw_mii_write);
#endif
    return 0;
malloc_err:
    if(priv) {
        free(priv);
    }
    if(dev) {
        free(dev);
    }
    if(sw) {
        free(sw);
    }
    return 1;
}
