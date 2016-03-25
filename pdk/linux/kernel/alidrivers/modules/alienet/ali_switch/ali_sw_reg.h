/*
 *	Linux ALi switch
 *
 *	Authors:
 *	peter.li <gilbertjuly@gmail.com>/<peter.li@alitech.com>
 *
 */
 
#ifndef _ALI_SW_REG_
#define _ALI_SW_REG_


#ifdef CONFIG_ARM
#include <ali_interrupt.h>
#include <mach/ali-s3921.h>
//#define ALI_SW_STATUS
//#define SW_CRCCHK

//ali_sw Base address.
//#define _ALI_SW_BASE		0xB802C000
//#define _ALI_SW_IRQ			(6+32+8)
#define _ALI_SW_BASE	    ALI_SW_BASE_ADDR	
#define _ALI_SW_IRQ		    INT_ALI_SWITCH_INT
#else
//ali_sw Base address.
#define _ALI_SW_BASE		0xB802C000
#define _ALI_SW_IRQ			(6+32+8)
#endif
/*
//#define PHY_SMSC
#define ALI_SW_PHY0_ADDR			32//just make it impossible.
#define ALI_SW_PHY1_ADDR		5 //5 //1 //17 
#define ALI_SW_PHY2_ADDR		5
#define ALI_SW_PHY3_ADDR		5
*/

////////////////////////////////////////////////////////////////////////////////
//Port Access Ctl Register
enum ALI_SW_PORT_ACCESS_REGISTER
{
	SysPortAccess = 0x1f8,			//system port access ctl register
};

////////////////////////////////////////////////////////////////////////////////
//ALi Switch Port0 Register
enum ALI_PORT0_REGISTERS {
	SysCtlP0 = 0,					//System Control Register.
	MacPhyAddrP0 = 0x08,			//Phydical Address Register (MAC address).
	TxDesStartAddr = 0x0C,		//Tx Start Address of Descriptors.
	RxDesStartAddr = 0x10,		//Rx Start Address of Descriptors.
	
	RxRingDesWPtr = 0x14,		//by SW.
	RxRingDesRPtr = 0x16,			//by HW.
	TxRingDesWPtr = 0x18,			//by SW.
	TxRingDesRPtr = 0x1A,			//by HW.	

	TxDesTotNum = 0x1C,			//Tx Descriptor Total Number.
	RxDesTotNum = 0x1E,			//Rx Descriptor Total Number.

	RxCoeWBDelay = 0x20, 		//Receive Buffer pointer, and Coe Write Back delay Control Reg. TBD(from here)
	LateColAjustP0 = 0x22, 		//beyond which a collision will be considered as a late collision. 
	
	TxDMARdPt = 0x24, 			//TX DMA read point. 
	RxTxArbiter = 0x28, 			//some Arbiter.

	IPCksFailCnt = 0x2C, 			//IP cks failure counter
	IPPayloadCksFailCnt = 0x2E,	//IP payloader cks failure counter
	
	SetupFrame_IO = 0x30,
	P0FilterMode = 0x34,
	

	P0ISR = 0x38,				//interrupt status register of p0
	P0IMR = 0x3c,				//interrupt mask register of p0
};

enum SysCtlP0Bits {
	SCRRxVlanEn = (1<<5),
	SCRRxCoeEn = (1<<4),
	SCRTxCoeEn = (1<<3),
	SCRRxEnP0 = (1<<2),
	SCRTxEnP0 = (1<<1),
	//SCRBufEmptyP0 = 1,
};

enum RxCoeWBDelayBits{
	CksCntDly = (0x3f<<10),
	CbrDwDly = (0x1f<<5),
	CbrCntDly = (0x1f),
};

enum LateColAjustBits{
	LateBnd = (0x7f),
};


enum RxTxArbiterBits{
	WlastPatchEn = (1<<9),
	RRdyFifoOvEn = (1<<8),
	TxNum = (0xf<<4),
	RxNum = (0xf),
};

enum MonitorRBits{
	MacTestSel = (0x1f),
};

enum P0ISRBits {
	EXP_DES_INT = (1<<3),
	EXP_DES_UPT_INT = (1<<2),
	P0_TX_COMPLETE = (1<<1),
	P0_RX_COMPLETE = (1<<0),
};

#define P0_INTERRUPT_MASK_SIMPLE (P0_TX_COMPLETE|P0_RX_COMPLETE)
#define P0_INTERRUPT_MASK (P0_TX_COMPLETE|P0_RX_COMPLETE|EXP_DES_INT|EXP_DES_UPT_INT)

/*
#define ALI_SW_INTERRUPT_MASK (ISRTxCoeErr|ISRLinkStatus|ISRWatchdog \
	|ISRRxComplete|ISRTxUnderrun|ISRTimer|ISRRxFifoOverflow|ISRRxBufOverflow|ISRRxBufDiscard)
#define ALI_SW_INTERRUPT_MASK_NOTIME (ISRTxCoeErr|ISRLinkStatus|ISRWatchdog \
	|ISRRxComplete|ISRTxUnderrun|ISRRxFifoOverflow|ISRRxBufOverflow|ISRRxBufDiscard)
*/

enum P0FilterModeBits{
	FILTER_MODE_MSK = (0xf),
	PASS_ALL_MULTICAST = (1<<3),
	PASS_PROMISCUOUS = (1<<2),
	PASS_FILTER_HASH_ONLY = 3,
	PASS_FILTER_INVERSE = 2,
	PASS_FILTER_HASH = 1,
	PASS_FILTER_PERFECT = 0,
};

////////////////////////////////////////////////////////////////////////////////

enum ALI_PORT123_REGISTERS {
	SysCtlP123 = 0,				//System Control Register.
	MacPhyAddrP123 = 0x08,		//Phydical Address Register (MAC address).
	PortTxCRCErrCnt = 0x14,
	
	NetStatus = 0x1c,				//Tx, Rx, Fifo status,etc
	NetOpMode = 0x20,				//Network operation mode
    TxRxCfg = 0x24,	
	LongFrameCnt = 0x30,
	RuntPktCnt = 0x32,
	
	AlignErrCnt = 0x34,
	CRCErrPktsCnt = 0x36,

	RmiiCR = 0x40,				//RMII/RGMII Control Register.

	PortStatistics = 0x48,

	PortSentCnt = 0x58,
	PortMonitor = 0x5c,

	
};


enum SysCtlP123Bits {
/*	
	SCRSoftRstP123 = (1<<3),
	SCRRxEnP123 = (1<<2),
	SCRTxEnP123 = (1<<1),
	SCRBufEmptyP123 = 1,
*/
	SCRSoftRstP123 = 1,
};

enum NetStatusBits{
	TxFSM = (7<<4),
	RxFSM = (7<<1),
	RxFifoEmpty = 1,
};

#define SW_SPEED_10MBPS		0
#define SW_SPEED_100MBPS		1
#define SW_SPEED_1000MBPS		2
enum NetOpModeBits{
	ForceLateColMode = (1<<12),
	PassExPadding = (1<<11),
	HeartBeatEn = (1<<10),
	ForceColMode = (1<<9),

	WorkModeNormal = (0<<7),
	WorkModeWakeUp = (1<<7),
	WorkModeLoopBack = (2<<7),
	WorkModeMask = (0x3<<7),
	
	FullDuplexMode = (1<<6),
	
	TxFlowCtlEn = (1<<5),
	RxFlowCtlEn = (1<<4),
	
//	PassErr = (1<<3),
//	PassMask = (0x7<<3),
	
	DribbleHandling = (1<<2),
/*	
	PerfectFiltering = (0<<0),
	HashFiltering = (1<<0),
	InverseFiltering = (2<<0),
	HashOnlyFiltering = (3<<0),
	FilteringMask = (0x3<<0),	
*/
	SWSpeedMask = 0x3,
//question, where is pass error, multicast, filter? in 0x34
};
#define NetworkOMConfig	(PassExPadding|WorkModeNormal|DribbleHandling)

enum TxRxCR1Bits{
	ForceReqDis = (1<<23),
	ForceSendPause = (1<<22),
	CrtlFrameTranEn = (1<<21),
	IpgTime1 = (0x1f<<5),
	IpgTime = (0x1f),
};

#define RxFifoThOff	20
#define TxFifoThOff	9
#define TxRxConfig2	(0x3f<<TxFifoThOff)
enum TxRxCR2Bits {
	PadDis = (1<<29),
		
	BackoffSelPseudo1 = (0<26),
	BackoffSelPseudo2 = (1<26),
	BackoffSelPseudo3 = (2<26),	
	BackoffSellMask = (0x3<<26),
	
	ChkLinkMode = (1<<25),
	ChkMagicMode = (1<<24),
	RXMaxLenEn = (1<<23),
	
	TxFifoThMask = (0x3f<<9),	//8 bytes in unit. 

	CrcDisable = (1<<8),
	//RxRemoveVlanTagEn = (1<<7),
	
//	TxFlowControlEn = (1<<6),
//	RxFlowControlEn = (1<<5),
	TxPauseFlag = (1<<4),
	RxPauseFlag = (1<<3),
	Port0_VlanEn = (1<<2),
	RxWatchdogRelease = (1<<1),
	RxWatchdogDisable = 1,
}; 

//#define RmiiSpeedOffset  4
enum RmiiCRBits {
	RgmiiEn = (1<<5),
	RmiiEn = (1<<4),
};

enum PortStatusFlgBits{
	PortStaClr = (1<<4),
	PortLateCol = (1<<3),
	PortHBFail = (1<<2),
	PortLosCarrier = (1<<1),
	PortNoCarrier = 1,
};

enum PortStatisticsBits{
	PortStatisticsAllClr = (1<<4),	
};




////////////////////////////////////////////////////////////////////////////////
enum ALI_SW_FABRIC_REGISTERS{
	SysCtlSwFabric = 0,
		
	PktDropLevel = 0x04,
	PktDropBM = 0x08,
	PktDropP0CntEgress = 0x0c,
	PktDropP1CntEgress = 0x10,	
	PktDropP2CntEgress = 0x14,	
	PktDropP3CntEgress = 0x18,		
	PktDropP1CntIngressBL = 0x1c,	
	PktDropP2CntIngressBL = 0x20,	
	PktDropP3CntIngressBL = 0x24,	

//	FlowCtlPauseLevel = 0x28,
//	FlowCtlResumeLevel = 0x2c,

	EgressRateP0Q0 = 0x28,
	EgressRateP0Q1 = 0x2a,
	EgressRateP0Q2 = 0x2c,
	EgressRateP0Q3 = 0x2e,
	
	EgressRateP1Q0 = 0x30,
	EgressRateP1Q1 = 0x32,
	EgressRateP1Q2 = 0x34,
	EgressRateP1Q3 = 0x36,

	FabricISR = 0x38,					//interrupt status register of fabric
	FabricIMR = 0x3c,				//interrupt status register of fabric

	EgressRateP2Q0 = 0x40,
	EgressRateP2Q1 = 0x42,
	EgressRateP2Q2 = 0x44,
	EgressRateP2Q3 = 0x46,

	EgressRateP3Q0 = 0x48,
	EgressRateP3Q1 = 0x4a,
	EgressRateP3Q2 = 0x4c,
	EgressRateP3Q3 = 0x4e,
		
	BroadcastStorm1 = 0x50,

	VlanPortType = 0x54,
	VlanTableCmdWd = 0x58,
	VlanTableRd = 0x5c,
	VlanPort3Tag = 0x62,
	VlanPort2Tag = 0x60,
	VlanPort1Tag = 0x66,
	VlanPort0Tag = 0x64,

	P123IngressRateLimitCtl = 0x70,
	P23CBS = 0x74,
	P23EBS = 0x78,
	
	Port1TC0 = 0x7c,
	Port1TC1 = 0x7e,
	Port1TC2 = 0x80,
	Port1TC3 = 0x82,
	Port1TC4 = 0x84,
	Port1TC5 = 0x86,
	Port1TC6 = 0x88,
	Port1TC7 = 0x8a,
	Port1TC8 = 0x8c,	
	Port1TC9 = 0x8e,
	Port1TC10 = 0x90,
	Port1TC11 = 0x92,
	Port1TC12 = 0x94,
	Port1TC13 = 0x96,
	Port1TC14 = 0x98,
	Port1TC15 = 0x9a,

	Port2TC0 = 0x9c,
	Port2TC1 = 0x9e,
	Port2TC2 = 0xa0,
	Port2TC3 = 0xa2,
	Port2TC4 = 0xa4,
	Port2TC5 = 0xa6,
	Port2TC6 = 0xa8,
	Port2TC7 = 0xaa,
	Port2TC8 = 0xac,	
	Port2TC9 = 0xae,
	Port2TC10 = 0xb0,
	Port2TC11 = 0xb2,
	Port2TC12 = 0xb4,
	Port2TC13 = 0xb6,
	Port2TC14 = 0xb8,
	Port2TC15 = 0xba,

	Port3TC0 = 0xbc,
	Port3TC1 = 0xbe,
	Port3TC2 = 0xc0,
	Port3TC3 = 0xc2,
	Port3TC4 = 0xc4,
	Port3TC5 = 0xc6,
	Port3TC6 = 0xc8,
	Port3TC7 = 0xca,
	Port3TC8 = 0xcc,	
	Port3TC9 = 0xce,
	Port3TC10 = 0xd0,
	Port3TC11 = 0xd2,
	Port3TC12 = 0xd4,
	Port3TC13 = 0xd6,
	Port3TC14 = 0xd8,
	Port3TC15 = 0xda,

	BroadcastStorm2 = 0xdc,
	BroadcastStorm3 = 0xe0,
	BroadcastStorm4 = 0xe4,

	ACLCtl = 0xe8,
	ACLWdata1 = 0xec,
	ACLWdata2 = 0xf0,
	ACLWdata3 = 0xf4,
	ACLWdata4 = 0xf8,
	ACLWdata5 = 0x100,

	QosCtl = 0x104,
	
	ACLRdata1 = 0x108,
	ACLRdata2 = 0x10c,
	ACLRdata3 = 0x110,
	ACLRdata4 = 0x114,
	ACLRdata5 = 0x118,

/*
	QosCtl = 0x104,
	IPv4Msk = 0x108,
	IPv4DAddr = 0x10c,
	IPv4SAddr = 0x110,	

	IPv6SAddr4 = 0x114,
	IPv6SAddr3 = 0x118,
	IPv6SAddr2 = 0x11c,
	IPv6SAddr1 = 0x120,
	IPv6DAddr4 = 0x124,
	IPv6DAddr3 = 0x128,
	IPv6DAddr2 = 0x12c,
	IPv6DAddr1 = 0x130,
*/
	PTD_DRP_CNT = 0x11c,
	P1TC = 0x11e,
	P2TC = 0X120,
	P3TC = 0X122,
	STP_DRP_CNT = 0x124,
	ALR_DRP_CNT = 0x126,
	BDC_DRP_CNT = 0x128,
	IGL_DRP_CNT = 0x12a,
	P0_DRP_CNT = 0x12c,
	P1_DRP_CNT = 0x12e,
	P2_DRP_CNT = 0x130,
	P3_DRP_CNT = 0x132,

	
	AlrCtl = 0x134,
	AlrRdata1 = 0x138,
	AlrRdata2 = 0x13c,
	AlrWdata1 = 0x140,
	AlrWdata2 = 0x144,
	
	PktDecCtl = 0x148,
	PriMap = 0x14c, 
	MdioCtl = 0x150,
	MdioWData = 0x154,
	MdioRData = 0x156,
	GETimer = 0x158,
	LinkCheckCtl = 0x15c,
	ScanTime = 0x160,
//	LinkCheckStat = 0x160,

	EgressRateCtl = 0x19c,
	P0IngressRateCtl = 0x1a0,
	P0IngressRateScale = 0x1a2,

	P1CBS = 0x1a4,
	P1EBS = 0x1fc,
	

	FlowCtl = 0x1a8,
	BR_TIME_INTERVAL = 0x1c4,
	PORT_STA_SEL = 0x1c7,
	P0_RX_BANDRATE = 0x1c8,
	P1_RX_BANDRATE = 0x1cc,
	P2_RX_BANDRATE = 0x1d0,
	P3_RX_BANDRATE = 0x1d4,
	P0_TX_BANDRATE = 0x1d8,
	P1_TX_BANDRATE = 0x1dc,
	P2_TX_BANDRATE = 0x1e0,
	P3_TX_BANDRATE = 0x1e4,
		
};

#define WAN_PORT_MSK 		0x7
#define WAN_PORT_OFFSET	12
enum SysCtlSwFabricBits {
	SCRPort0RxEn = (1<<9),
	SCRPort0TxEn = (1<<8),
	SCRPort3En = (1<<6),
	SCRPort2En = (1<<5),
	SCRPort1En = (1<<4),
	SCRBMEn = (1<<3),
	SCRAutoFreshTable = (1<<2),
	SCRSWFEn = (1<<1), 
	SCRSWFRst = 1, //question, no TX en? rst is different than others???
};
#define SCRPort0En	(SCRPort0TxEn|SCRPort0RxEn)

enum FabricISRBits{
	EXCEPT_BM = (1<<9),
	PORT0_EVENT = (1<<8),
	PORT3_RX_LOOPBACK = (1<<7),
	PORT2_RX_LOOPBACK = (1<<6),
	PORT1_RX_LOOPBACK = (1<<5),
	GP_TIMER_EXPIRED = (1<<4),
	MDIO_COMPLETE = (1<<3),
	PORT3_RX_FIFO_OVERFLOW = (1<<2),
	PORT2_RX_FIFO_OVERFLOW = (1<<1),
	PORT1_RX_FIFO_OVERFLOW = 1,//question, we need to reset port???
};
#define FABRIC_INTERRUPT_MASK (PORT0_EVENT|PORT3_RX_LOOPBACK|PORT2_RX_LOOPBACK|PORT1_RX_LOOPBACK |GP_TIMER_EXPIRED|MDIO_COMPLETE|PORT3_RX_FIFO_OVERFLOW|PORT2_RX_FIFO_OVERFLOW|PORT1_RX_FIFO_OVERFLOW|EXCEPT_BM)

#define FABRIC_INTERRUPT_MASK_CAREFUL (PORT3_RX_LOOPBACK|PORT2_RX_LOOPBACK|PORT1_RX_LOOPBACK\
		|PORT3_RX_FIFO_OVERFLOW|PORT2_RX_FIFO_OVERFLOW|PORT1_RX_FIFO_OVERFLOW)


#define MdioOperOffset 	26
#define MdioPhyAddrOffset 	21
#define MdioRegAddrOffset 	16

enum MdioCtlBits{
	MdioHWTansStart = (1<<31),
	MdioReadOper = (2<<26),
	MdioWriteOper = (1<<26),
	MdioOperMsk = (0x3<<26),
	MdioPhyAddrMsk = (0x1f<<21),
	MdioRegAddrMsk = (0x1f<<16),
	MdioOutputDelayMsk = (0x3f<<10),
	MdioOutputClkDivMsk = (0x3ff),
};


#define P3PhyAddrOffset 	16
#define P2PhyAddrOffset 	8
#define P1PhyAddrOffset  	0
enum LinkCheckCtlBits{
	AutoMdioEn = (1<<23), //enable hardware polling, driver check regs to know status; disable, driver needs to implement read/write funcs to access PHY status

	P3PhyActive = (1<<22),
	P3PhyType = (1<<21),

	P2PhyActive = (1<<14),
	P2PhyType = (1<<13),

	P1PhyActive = (1<<6),
	P1PhyType = (1<<5),

	P123PhyAddrMsk = 0x1f,
};

#define P1SpeedModOffset	9
#define P2SpeedModOffset	5
#define P3SpeedModOffset	1
#define P123LinkStatusMsk 	1
#define P123SpeedModMsk	3
enum LinkCheckStatBits{
	P1LinkDown = (1<<11),
//	P1SpeedModOffset = 9,
	P1DuplexMod = (1<<8),
	P2LinkDown = (1<<7),
//	P2SpeedModOffset = 5,
	P2DuplexMod = (1<<4),
	P3LinkDown = (1<<3),
//	P3SpeedModOffset = 1,
	P3DuplexMod = 1,
};

#define AlrEntryNumOffset	19
#define AlrIndexModOffset	16
#define AlrIdxOffset		8
enum AlrCtlBits{
	AlrEntryNumMsk = (0x7f<<19),
//	AlrEntryNumOffset = 19,
	AlrFullFlag = (1<<18),
	AlrIndexModHash = (0<<16),
	AlrIndexModXor = (1<<16),
	AlrIndexModLow6 = (2<<16),
	AlrIndexModMsk = (0x3<<16),
//	AlrIndexModOffset = 16,
	
	AlrErrFlag = (1<<15),
	AlrIdxMsk = (0x7f<<8),
//	AlrAddrOffset = 8,
	AlrWRCmd = (1<<3),
	AlrCmdAck = (1<<2),	
	AlrCmdReq = (1<<1),
	AlrAgeFastAging = 1,	
};

#define AlrRdataPortOffset 16
enum AlrRdataHiBits{
	AlrRdataValid = (1<<23),
//	AlrRdataEOT = (1<<22),	//EOT -- End of Table, question, there is a GET NEXT cmd?
	AlrRdataStatic = (1<<21), 	//static entry, won't be removed because of aging reason or learning process.
	AlrRdataFilter = (1<<20),	//What for? question, filtered in or out?
	AlrRdataPortMsk = (0xf<<16),
//	AlrRdataPortOffset = 16,
	AlrRdataMacAddrHiMsk = (0xffff),
};

enum AlrRdataLowBits{
	AlrRdataMacAddrLowMsk = (0xffffffff),
};

#define AlrWdataPortOffset  16 
enum AlrWdataHiBits{
	AlrWdataValid = (1<<23),
	AlrWdataAge = (1<<22),	//what for? question.
	AlrWdataStatic = (1<<21), 	//static entry, won't be removed because of aging reason or learning process.
	AlrWdataFilter = (1<<20),	//What for? question, filtered in or out?
	AlrWdataPortMsk = (0xf<<16),
//	AlrWdataPortOffset = 16,
	AlrWdataMacAddrHiMsk = (0xffff),
};

enum AlrWdataLowBits{
	AlrWdataMacAddrLowMsk = (0xffffffff),
};

#define P3StpStatusOffset	6
#define P2StpStatusOffset	4
#define P1StpStatusOffset	2
#define P0StpStatusOffset	0
#define PortStpStatusMsk 0x3

enum PktDecCtlBits{
	BrdgeCtlEn = (1<<12),
	IgmpMldEn = (1<<11),
	GmrpMmrpEn = (1<<10),
	GvrpMvrpEn = (1<<9),
	StpEn = (1<<8),
	P3StpStatusMsk = (0x3<<6),
	P2StpStatusMsk = (0x3<<4),
	P1StpStatusMsk = (0x3<<2),
	P0StpStatusMsk = (0x3<<0),
//	P3StpStatusOffset = 6,
//	P2StpStatusOffset = 4,
//	P1StpStatusOffset = 2,
//	P0StpStatusOffset = 0,
};



#define VLAN_PORT0_FFF_CHG	(1<<25)
#define VLAN_PORT1_FFF_CHG	(1<<26)
#define VLAN_PORT2_FFF_CHG	(1<<27)
#define VLAN_PORT3_FFF_CHG	(1<<28)
#define VLAN_PORT0_FFF_CHG_OFFSET		25
#define VLAN_PORT1_FFF_CHG_OFFSET		26
#define VLAN_PORT2_FFF_CHG_OFFSET		27
#define VLAN_PORT3_FFF_CHG_OFFSET		28

#define VLAN_PORT0_IGRS_FILTER	(1<<20)
#define VLAN_PORT1_IGRS_FILTER	(1<<21)
#define VLAN_PORT2_IGRS_FILTER	(1<<22)
#define VLAN_PORT3_IGRS_FILTER	(1<<23)
#define VLAN_PORT0_IGRS_FILTER_OFFSET	20
#define VLAN_PORT1_IGRS_FILTER_OFFSET	21
#define VLAN_PORT2_IGRS_FILTER_OFFSET	22
#define VLAN_PORT3_IGRS_FILTER_OFFSET	23

#define VLAN_PORT0_TAG_CHG	(1<<16)
#define VLAN_PORT1_TAG_CHG	(1<<17)
#define VLAN_PORT2_TAG_CHG	(1<<18)
#define VLAN_PORT3_TAG_CHG	(1<<19)
#define VLAN_PORT0_TAG_CHG_OFFSET	16
#define VLAN_PORT1_TAG_CHG_OFFSET	17
#define VLAN_PORT2_TAG_CHG_OFFSET	18
#define VLAN_PORT3_TAG_CHG_OFFSET	19

#define VLAN_PORT0_TAG_ONLY	(1<<12)
#define VLAN_PORT1_TAG_ONLY	(1<<13)
#define VLAN_PORT2_TAG_ONLY	(1<<14)
#define VLAN_PORT3_TAG_ONLY	(1<<15)
#define VLAN_PORT0_TAG_ONLY_OFFSET	12
#define VLAN_PORT1_TAG_ONLY_OFFSET	13
#define VLAN_PORT2_TAG_ONLY_OFFSET	14
#define VLAN_PORT3_TAG_ONLY_OFFSET	15

#define VLAN_PORT0_UNTAG_ONLY	(1<<8)
#define VLAN_PORT1_UNTAG_ONLY	(1<<9)
#define VLAN_PORT2_UNTAG_ONLY	(1<<10)
#define VLAN_PORT3_UNTAG_ONLY	(1<<11)
#define VLAN_PORT0_UNTAG_ONLY_OFFSET	8
#define VLAN_PORT1_UNTAG_ONLY_OFFSET	9
#define VLAN_PORT2_UNTAG_ONLY_OFFSET	10
#define VLAN_PORT3_UNTAG_ONLY_OFFSET	11

#define VlanPort0TypeOffset	0
#define VlanPort1TypeOffset	2
#define VlanPort2TypeOffset	4
#define VlanPort3TypeOffset	6
#define VlanPortTypeMsk 		0x3
enum VlanPortTypeBits{
	SW_VlanEn = (1<<24),
	VlanPortDumb = 0,
	VlanPortAccess = 1,
	VlanPortTrunk = 2,
	VlanPortHybrid = 3,
};

#define VlutAddrOffset 20
enum VlanTableCmdBits{
	VLutCmdAck = (1<<26),
	VLutCmdRequest = (1<<25),
	VlutCmdWR = (1<<24),
	VlutAddrMsk = (0xf<<20),
	VlutData = (0xfffff),
};

#define VlanPpriortyOffset  (12)
#define VlanPvidOffset  (0x0)
enum VlanPortTag{
	PvidMsk = (0xfff),
	PpriortyMsk = (0x7<<12),
};

enum EgressRateCtlBits{
	P0EgressRateLimitEn = 1,
	P3EgressRateLimitEn = (1<<1),
	P2EgressRateLimitEn = (1<<2),
	P1EgressRateLimitEn = (1<<3),
	P0EgressRateLimitQueueMode = (1<<4),
	P3EgressRateLimitQueueMode = (1<<5),
	P2EgressRateLimitQueueMode = (1<<6),
	P1EgressRateLimitQueueMode = (1<<7),
};

enum P0IngressRateCtlBits{
	P0IgressRateLimittEn = (1<<15),
};

#define IGRS_MODE_MSK 		0x3
#define IGRS_MODE_OFFSET	3
enum P123IngressRateLimitCtlBits{
	IGRS_DSCP_EN = (1<<9),
	IGRS_ACL_EN = (1<<8),
	IGRS_VLAN_EN = (1<<7),
	DROP_YELLOW = (1<<6),
	DROP_RED = (1<<5),
//	IGRS_MODE_OFFSET = (1<<3),
	IGRS_PORT3_EN = (1<<2),
	IGRS_PORT2_EN = (1<<1),
	IGRS_PORT1_EN = (1<<0),
};


#define 	P0_BLOCKS_NUM_MSK 0xff 
enum FlowCtlBits{
	FC_P1_IGRS_PORT_EN = (1<<11),
	FC_P2_IGRS_PORT_EN = (1<<12),
	FC_P3_IGRS_PORT_EN = (1<<13),
	FC_P1_IGRS_PRIOR_EN = (1<<14),
	FC_P2_IGRS_PRIOR_EN = (1<<15),
	FC_P3_IGRS_PRIOR_EN = (1<<16),
};



#define ACL_MODE_MSK 0xf
#define ACL_ADDR_MSK 0xf
#define ACL_MODE_OFFSET	7
#define ACL_ADDR_OFFSET	2
enum ACLCtlBits{
//	ACL_MODE_OFFSET = (1<<7),
	ACL_ACK = (1<<6),
//	ACL_ADDR_OFFSET = (1<<2),
	ACL_RW = (1<<1),
	ACL_REQ = (1<<0),
};

#define ACL_PRIORITY_MSK	0xf
#define ACL_IP_PORT_MSK	0xffff
#define ACL_PORTS_MSK		0xf
#define ACL_TCP_UDP_MSK	0x3
#define ACL_DEST_PORT_END_16B_OFFSET		4
#define ACL_DEST_PORT_START_F12B_OFFSET	20
#define ACL_SRC_PORT_END_16B_OFFSET 		4
#define ACL_SRC_PORT_START_F12B_OFFSET	20
#define ACL_TCP_UDP_OFFSET					4
#define ACL_PORTS_OFFSET					6
#define ACL_IP2_ADDR_F22B_OFFSET			10
#define ACL_IP1_ADDR_F22B_OFFSET			10

#if 0
enum ACLWdata1Bits{
//	ACL_PRIORITY_OFFSET = (1<<0),
//	ACL_DEST_PORT_END_16B_OFFSET = (1<<4),
//	ACL_DEST_PORT_START_F12B_OFFSET = (1<<20),
};
enum ACLWdata2Bits{
//	ACL_DEST_PORT_START_L4B_OFFSET = (1<<0),
//	ACL_SRC_PORT_END_16B_OFFSET = (1<<4),
//	ACL_SRC_PORT_START_F12B_OFFSET = (1<<20),
};
enum ACLWdata3Bits{
//	ACL_SRC_PORT_START_L4B_OFFSET = (1<<0),
//	ACL_TCP_UDP_OFFSET = (1<<4),
//	ACL_PORTS_OFFSET = (1<<6),
//	ACL_IP2_ADDR_F22B_OFFSET = (1<<10),
};
enum ACLWdata4Bits{
//	ACL_IP2_ADDR_L10B_OFFSET = (1<<0),
//	ACL_IP1_ADDR_F22B_OFFSET = (1<<10),
};
enum ACLWdata5Bits{
//	ACL_IP1_ADDR_L10B_OFFSET = (1<<0),
};
#endif


enum QosCtlBits{
	ACL_QOS_EN = (1<<4),
	VLAN_QOS_EN = (1<<3),
	DSCP_QOS_EN = (1<<2),
};


////////////////////////////////////////////////////////////////////////////////

enum PHY_REGISTERS {
	PhyBasicModeCtrl = 0,
	PhyBasicModeStatus = 1,
	PhyNWayAdvert = 4,
	PhyNWayLPAR = 5,
	PhyNWayExpansion = 6,
	Phy1000BaseTControl = 9,
	Phy1000BaseTStatus = 0xa,
};

enum Phy1000TCtlBits {
	T_1000_FD = (1<<9),
	T_1000_HD = (1<<8),
};
enum Phy1000TStatusBits {
	LP_1000_FD = (1<<11),
	LP_1000_HD = (1<<10),
};
enum BMCRBits {
	BMCRReset = (1<<15),
	BMCRLoopback = (1<<14),
	BMCRSpeedSet = (1<<13),
	BMCRSpeedSet13 = (1<<13),
	BMCRANEnable = (1<<12),
	BMCRPowerDown = (1<<11),
	BMCRRestartAN = (1<<9),
	BMCRDuplexMode = (1<<8),
	BMCRSpeedSet6 = (1<<6),
};


enum BMSRBits {
	BMSR100BT4 = (1<<15),
	BMSR100BTXFULL = (1<<14),
	BMSR100BTXHALF = (1<<13),
	BMSR10BTFULL = (1<<12),
	BMSR10BTHALF = (1<<11),	
	BMSRANComplete = (1<<5),
	BMSRRemoteFault = (1<<4),
	BMSRAN = (1<<3),
	BMSRLinkStatus = (1<<2),
	BMSRJabberDectect = (1<<1),
	BMSREC = 1,
};


enum ANARBits {
	ANARNP = (1<<15),
	ANARACK = (1<<14),
	ANARRF = (1<<13),
	ANARASMDIR = (1<<11),	
	ANARPause = (1<<10),	
	ANART4 = (1<<9),
	ANARTXFD = (1<<8),
	ANARTX = (1<<7),
	ANAR10FD = (1<<6),
	ANAR10 = (1<<5),
};

//#define ANAR_MEDIA	(ANAR10|ANAR10FD|ANARTX|ANARTXFD|ANART4)
#define ANAR_MEDIA	(ANAR10|ANAR10FD|ANARTX|ANARTXFD)

////////////////////////////////////////////////////////////////////////////////


#define FPGA_PORT2_RMII_PHY_OFFSET 	2
#define FPGA_PORT3_RMII_PHY_OFFSET 	10
#define FPGA_RMII_PHY_SET 		2
enum FPGA_PHY_MAC_SET_REGBits{
	FPGA_PORT2_PHY_FULL_DUPLEX = (1<<0),
	FPGA_PORT2_PHY_SPEED = (1<<1),
	FPGA_PORT2_PHY_RMII_EN = (1<<6),
	FPGA_PORT3_PHY_FULL_DUPLEX = (1<<8),
	FPGA_PORT3_PHY_SPEED = (1<<9),
	FPGA_PORT3_PHY_RMII_EN = (1<<14),	
};

#endif // _ALI_SW_REG_



