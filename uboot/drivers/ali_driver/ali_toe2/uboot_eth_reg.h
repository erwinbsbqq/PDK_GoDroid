/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    ethernet_register.h
*
*    Description:    S3202C Ethernetnet Mac & Phy register definition.
*
*    History:
*		Date			Athor		Version		Reason
*	    =======================================================
*     1.    20.02.2008	Mao Feng		Ver 1.0		Original MAC
*	2.	23.04.2009				Ver 2.0		Cost Down MAC
*
******************************************************************************/
#ifndef _ETHERNET_REGISTER_
#define _ETHERNET_REGISTER_

enum ALI_MAC_REGISTERS {
	SCR = 0,					//Command Register.
	PAR = 0x8,				//Phydical Address Register.
	TSAD = 0x0C,				//Tx Start Address of Descriptors.
	RSAD = 0x10,				//Rx Start Address of Descriptors.
	RxRingDesWPtr = 0x14,	//by SW.
	RxRingDesRPtr = 0x16,	//by HW.
	TxRingDesWPtr = 0x18,	//by SW.
	TxRingDesRPtr = 0x1a,	//by HW.	
	RmiiMode = 0x1c,
	VerID = 0x20,			//Version ID.
	TxDesTotNum = 0x24,		//Tx Descriptor Total Number.
	RxDesTotNum = 0x26,		//Rx Descriptor Total Number.
	BackPressure =  0x28,
	RSR = 0x34,				//Rx Status Register.
	ISR = 0x38,				//Interrupt Status Register.
	IMR = 0x3C,				//Interrupt Mask Register.
	NetworkOM = 0x40,		//Network Operation Mode.
	TxRxCR1 = 0x44,			//Transmit and Receive Configuration Register 1.
	MiiMR1 = 0x48,			//Mii Management Register 1.
	DelayControlR = 0x4C,		//Delay Control Register.			//TOE
	LateColAjustR = 0x4E,		//Late Collision Ajustment Register.	//TOE
	
	TxRxCR2 = 0x54,			//Transmit and Receive Configuration Register 2.
	MonitorR = 0x58,			//Monitor Register.
	TimerR = 0x5C,			//General Purpose Timer Register.
	MFC = 0x60,				//Missed Frame Counter. due to rx ring buffer overflow. 15 bits valid, 1 bit overflow. write clears.	
	PPC = 0x62,				// Purged Packet Counter. due to rx fifo overflow. 15 bits valid, 1 bit overflow. write clears.
	LFC = 0x64,				//Long Frame Counter. due to long packet received.
	RPC = 0x66,				//Runt Packet Counter. due to fragment packet received.
	AlignErrCnt = 0x68,			//Alignment Error Counter.
	RxStFrameCfg = 0x70,	//RX Setup Frame config
	CrcErrCnt = 0x6A,			//CRC error Packet Counter. due to CRC error packet received.
	ClkDelayChainSR = 0x74,	//Clock Delay Chain Setting Register.
	RmiiCR = 0x78,			//RMII Control Register.
	MiiMR2 = 0x7C,			//Mii Management Register 2.
	MdioW =	0x80,			//Mdio Write Data.
	MdioR = 0x82,			//Mdio Read Data.

	IMBLatency = 0x84,		//MAC IMB Latency.
	RxChkSumStartOff = 0x8C,	//Rx CheckSum Start Offset.
	IPHdrChsFailPC = 0x90,	//IP Header Checksum Fail Packet Counter. TOE II.
	IPPayChsFailPC = 0x92,	//IP Payloader Checksum Fail Packet Counter. TOE II.
};

enum SCRBits {
	SCRUfoEn = (1<<7),
	SCRTsoEn = (1<<6),
	SCRRxCoeEn = (1<<5),
	SCRTxCoeEn = (1<<4),
	SCRReset = (1<<3),
	SCRRxEn = (1<<2),
	SCRTxEn = (1<<1),
	SCRBufEmpty = 1,
};

enum RmiiModeBits{
	RmiiDuplex = (1<<4),
	RmiiMdSpeed = (1<<3),
	RmiiLinkStatus = (1<<2),
	RmiiTxCrsEn = (1<<1),
	RmiiAutoNeg = 1,
};

enum BackPressureBits{
	BP_En = (1<<15),
	BP_SendCntMask =  (0x7ff<<4),
	BP_SendPatMask = (0xf),
};
#define BP_SendCntOff 	4
#define BP_SendPatOff 	0

enum ISRBits {
	ISRRxHdrErr = (1<<14),		//Rx Header Error.
	ISRTxCoeErr = (1<<13),		//Tx COE interpolator Error.
	ISRNormal = (1<<12),			//Normal Interrupt Summary.
	ISRAbnormal = (1<<11),		//Abnormal Interrupt Summary.
	ISRRxBufDiscard = (1<<10),	//Buffer Discard..
	ISRLinkStatus = (1<<9),		//Link Status Change.
	ISRMdioComplete = (1<<8),	//HW Mdio Transaction Completion.
	ISRTxEarly = (1<<7),			//Early Transmit.
	ISRWatchdog = (1<<6),		//Receive Watchdog Time-Out.
	ISRRxComplete = (1<<5),		//Receive Complete.
	ISRTxUnderrun= (1<<4),		//Transmit Fifo Underrun.
	ISRTimer = (1<<3),			//General Purpose Timer Expired.
	ISRTxComplete = (1<<2),		//Transmit Complete Mask.
	ISRRxFifoOverflow = (1<<1),	//Receive Fifo Overflow Mask.
	ISRRxBufOverflow = 1,		//Receive Buffer Overflow Mask.
};
#define TOE2_INTERRUPT_MASK (ISRTxCoeErr|ISRLinkStatus|ISRWatchdog \
	|ISRRxComplete|ISRTxUnderrun|ISRTimer|ISRRxFifoOverflow|ISRRxBufOverflow|ISRRxBufDiscard)

enum NetworkOMBits {
	RxTOEWorkMode = (1<<14),
	ForceTxFifoUnderrun = (1<<13),
	ForceLateColMode = (1<<12),
	PassExPadding = (1<<11),
	HeartBeatEn = (1<<10),
	ForceColMode = (1<<9),

	WorkModeNormal = 0,
	WorkModeWakeUp = (1<<7),
	WorkModeLoopBack = (2<<7),
	WorkModeMask = (0x3<<7),
	
	FullDuplexMode = (1<<6),
	
	PassAllMulticast = (1<<5),
	PassPromiscuous = (1<<4),
	PassErr = (1<<3),
	PassMask = (0x7<<3),
	
	DribbleHandling = (1<<2),
	
	PerfectFiltering = (0<<0),
	HashFiltering = (1<<0),
	InverseFiltering = (2<<0),
	HashOnlyFiltering = (3<<0),
	FilteringMask = (0x3<<0),
};
#define NetworkOMConfig	(PassExPadding|WorkModeNormal|PassPromiscuous|PerfectFiltering|DribbleHandling)

enum TxRxCR1Bits{
	ForceReqDis = (1<<23),
	ForceSendPause = (1<<22),
	CrtlFrameTranEn = (1<<21),
};

enum MiiMR1Bits {
	MiiMdioEn = (1<<10),
	MiiMdioIn = (1<<9),			//Used to read data from external PHY.
	MiiMdioDir = (1<<8),			//
	MiiMdioOut = (1<<7),			//Used to write data to external PHY.
	MiiMdioClk = (1<<6),

	MiiMdioWrite0 = (MiiMdioDir),
	MiiMdioWrite1 = (MiiMdioDir|MiiMdioOut),
};

enum DelayControlRBits {
	LATE_BND = (0x7F<<16),
	CHKSUM_CNT_DLY = (0x3F<<10),
	CBR_DW_DLY = (0x1F<<5),
	CBR_CNT_DLY = (0x1F),
};
#define LATE_BND_OFF			16
#define CHKSUM_CNT_DLY_OFF	10
#define CBR_DW_DLY_OFF		5
#define CBR_CNT_DLY_OFF		0

enum TxRxCR2Bits {
	SetupFramType = (1<<30),
	PadDis = (1<<29),
	ChkLinkMode = (1<<25),
	ChkMagicMode = (1<<24),
	RxMaxLenEn = (1<<23),
	
	RxFifoTh16 = (0x0<<20),
	RxFifoTh32 = (0x1<<20),
	RxFifoTh64 = (0x2<<20),
	RxFifoTh128 = (0x3<<20),
	RxFifoTh256 = (0x4<<20),
	RxFifoTh512 = (0x5<<20),
	RxFifoTh1024 = (0x6<<20),
	RxFifoThPkt = (0x7<<20),
	RxFifoThMask = (0x7<<20),

	TxFifoThMask = (0x3f<<9),	//8 bytes in unit. 

	CrcDis = (1<<8),
	RxRemoveVlanTagEn = (1<<7),
	
	TxFlowControlEn = (1<<6),
	RxFlowControlEn = (1<<5),
	TxPauseFlag = (1<<4),
	RxPauseFlag = (1<<3),
	VlanEn = (1<<2),
	RxWatchdogRelease = (1<<1),
	RxWatchdogDis = 1,
};
#define RxFifoThOff	20
#define TxFifoThOff	9
#define TxRxConfig2	(0x3f<<TxFifoThOff)

enum RmiiCRBits {
	RmiiEn = (1<<5),
	RmiiCrSpeed = (1<<4),
	RmiiPhyDlyMask = (0xf),
};

enum MiiMR2Bits {
	MiiTransStart = (1<<31),
	MiiOpRead = (2<<26),
	MiiOpWrite = (1<<26),
	MiiOpMask = (0x3<<26),
	MiiPhyAddrMask = (0x1F<<21),
	MiiRegAddrMask = (0x1F<<16),
};
#define MiiPhyAddrOff	21
#define MiiRegAddrOff	16

enum IMBLatencyBits {
	MAC_IMB_LAT = (0xFF<<8),
	MAC_HI_PRTY_CNT = (0xFF),
};
#define MAC_IMB_LAT_OFF		8
#define MAC_HI_PRTY_CNT_OFF	0

enum TxDesIPStartOBits {
	RxMaxPktLenMask = (0xfff),
};



enum ALI_PHY_REGISTERS {
	PhyBasicModeCtrl = 0,
	PhyBasicModeStatus = 1,
	PhyNWayAdvert = 4,
	PhyNWayLPAR = 5,
	PhyNWayExpansion = 6,
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

#define ANAR_MEDIA	(ANAR10|ANAR10FD|ANARTX|ANARTXFD|ANART4)

#endif // _ETHERNET_REGISTER_

