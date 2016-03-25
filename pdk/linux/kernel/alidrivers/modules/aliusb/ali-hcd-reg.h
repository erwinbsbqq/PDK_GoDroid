/****************************************************************************
*  ALi Corporation, All Rights Reserved. 2006 Copyright (C)
*
*  File: hc_s3602_reg.h
*
*  Description: Register addr & Bit Field Definition of S3602 USB Host IP
*  History:
*      Date 			  Author		 	 	Version    	Comment
*      ========    ========		======		=======
*  1.  2006.11.30	  Jimmy Chen		0.1.000 		Initial
*  2.
****************************************************************************/
#ifndef __HC_3602_REG_H__
#define __HC_3602_REG_H__

// Base I/O address with USB Host 
#define HC_S3101E_BASE_ADDR 		0xb8022000  
#define HC_S3101D_BASE_ADDR 		0xb800e000
#define HC_M3202_BASE_ADDR		0xb800e000
#define HC_S3327G4_BASE_ADDR 		0xb800a000
#define HC_S3329E_BASE_ADDR		0xb800f000
#define HC_S3329E5_BASE_ADDR 		0xb8012000
#define HC_S3602_BASE_ADDR 		0xb8022000
#define HC_S3603_BASE_ADDR 		0xb8032000
#define HC_S3603_BASE_ADDR1 		0xb8032400



#define USB_HC_S3101E_INTERRUPT_ID	(29+8) 
#define USB_HC_S3101D_INTERRUPT_ID	(22+8) 
#define USB_HC_M3202_INTERRUPT_ID	 	(19+8)
#define USB_HC_S3327G4_INTERRUPT_ID	(18+8) 
#define USB_HC_M3329E_INTERRUPT_ID	(20+8)
#define USB_HC_M3329E5_INTERRUPT_ID	(28+8)
#define USB_HC_S3602_INTERRUPT_ID		(29+8)
#define USB_HC_S3602F_INTERRUPT_ID	(29+8) 
#define USB_HC_S3602F_INTERRUPT_ID1	(4+32+8) 

#define ALI_CHIPID_M3202	0x3202
#define ALI_CHIPID_S3602	0x3602
#define ALI_CHIPID_S3603	0x3603
#define ALI_CHIPID_M3329E	0x3329

#define CHIP_IC_REV_5       5

#ifndef UINT32
typedef char INT8;
typedef unsigned char UINT8;
typedef short INT16;
typedef unsigned short UINT16;
typedef int INT32;
typedef unsigned int UINT32;
#endif

#ifndef BOOL
#define BOOL    int
#endif

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#undef SUCCESS
#define SUCCESS 0

static inline void hc_WriteByte(UINT32 addr,UINT32 offset,UINT8 value)
{
	*(volatile UINT8 *)(addr+offset) = (value);
}

static inline void hc_WriteDword(UINT32 addr,UINT32 offset,UINT32 value)
{
	*(volatile UINT32 *)(addr+offset) = (value);
}

static inline UINT8 hc_ReadByte(UINT32 addr,UINT32 offset)	
{
	return (*(volatile UINT8 *)(addr+offset));
}

static inline UINT32 hc_ReadDword(UINT32 addr,UINT32 offset)
{
	return (*(volatile UINT32 *)(addr+offset));
}

#define	hc_s3602_delay		udelay



//***************************************************************************
// 			USB Host Register address
//***************************************************************************
enum USB_HOST_REG
{
	HPORTCTLR = 0x10,			// Port Control Register
// endpoint 0 	
	HEP0FCTR = 0x11,			// Endpoint 0 FIFO Control Register
	HEP0FIFO = 0x12,			// Endpoint 0 FIFO Data Port
// endpoint A 	
	HEPASETR = 0x13,			// Endpoint A Setting Register
	HEPAFCTR = 0x14,			// Endpoint A FIFO Control Register
	HEPAFIFO = 0x15,			// Endpoint A FIFO Data Port
// endpoint B 	
	HEPBSETR = 0x16,			// Endpoint B Setting Register
	HEPBFCTR = 0x17,			// Endpoint B FIFO Control Register
	HEPBFIFO = 0x18,			// Endpoint B FIFO Data Port
// endpoint C 	
	HEPCSETR = 0x19,			// Endpoint C Setting Register
	HEPCFCTR = 0x1A,			// Endpoint C FIFO Control Register
	HEPCFIFO = 0x1B,			// Endpoint C FIFO Data Port

	HFNCADD = 0x1C,				// Function Address Register
	HTKNCC1 = 0x1D,				// Token Control Register
// Interrupt	
	HINTENR1 = 0x1E,			// Interrupt Enable Register1
	HINTENR2 = 0x1F,			// Interrupt Enable Register2
	HINTFLR1 = 0x20,			// Interrupt Flag Register1
	HINTFLR2 = 0x21,			// Interrupt Flag Register2

//3603
// endpoint D	
	HEPDSETR = 0x22,			// Endpoint D Setting Register
	HEPDFCTR = 0x23,			// Endpoint D FIFO Control Register
	HEPDFIFO = 0x24,			// Endpoint D FIFO Data Port
// endpoint E	
	HEPESETR = 0x25,			// Endpoint E Setting Register
	HEPEFCTR = 0x26,			// Endpoint E FIFO Control Register
	HEPEFIFO = 0x27,			// Endpoint E FIFO Data Port
// endpoint F	
	HEPFSETR = 0x28,			// Endpoint F Setting Register
	HEPFFCTR = 0x29,			// Endpoint F FIFO Control Register
	HEPFFIFO = 0x2A,			// Endpoint F FIFO Data Port
	
// Endpoint Interval
	HBINTERVALCTRL = 0x2B,		// Interrupt Pipe bInterval Control Register
	EPB_DMA_CNTL_3603 = 0x2C,		// EPB Transaction Counter Low Byte Register
	EPB_DMA_CNTH_3603 = 0x2D,		// EPB Transaction Counter High Byte Register
	EPB_DMA_CNTL = 0x2A,		// EPB Transaction Counter Low Byte Register 
	EPB_DMA_CNTH = 0x2B,		// EPB Transaction Counter High Byte Register 
	HINTENR3 = 0x2E,			// Interrupt Enable Register3
	HINTFLR3 = 0x2F,			// Interrupt Flag Register3

/*
//29D/F
	// Endpoint Interval 	
	HBINTERVALA = 0x22,			// EpA Interrupt Pipe Poll Interval Register 
	HBINTERVALB = 0x23,			// EpB Interrupt Pipe Poll Interval Register 
	HBINTERVALC = 0x24,			// EpC Interrupt Pipe Poll Interval Register 
	HBINTERVALCTRL = 0x25,		// Interrupt Pipe bInterval Control Register 
	
	NAKPERIOD = 0x26,			// NAK Retry Period Register 
	HPKTSIZE = 0x27,			// Largest Packet Size Addend Register 
	HBW = 0x28,					// EP High Bandwidth Setting Register 
	EPCDSIZE = 0x29,
	EPB_DMA_CNTL = 0x2A,		// EPB Transaction Counter Low Byte Register 
	EPB_DMA_CNTH = 0x2B,		// EPB Transaction Counter High Byte Register 

	HEPDSETR = 0x2C,			//Endpoint D setting Register
	HEPDFCTR = 0x2D,			//Endpoint D FIFO Cotrol Register
	HEPDFIFO = 0x2E,			//Endpiont D FIFO data Port
	HINTFLR3 = 0x2F				//Interrupt Flag Register3
*/
};

//****************************************************************************
//* 			USB IP Level Register address
//****************************************************************************
enum USB_IP_LEVEL_REG
{
	EPSIZE = 0x30,				// Endpoint Size Register
	EPCDSIZE = 0x31,			// Endpoint C D Size Register	// 3603
 	TEST1 = 0x31,				// IP Test Register 1 // 29D F
	USBSPD = 0x32,				// USB Speed Register
	INTFLG2 = 0x33,				// PHY Test Register
	OCTLR = 0x34,				// OTG Control Register
	USBTLR = 0x34,				// 3202 USB Control Register
	OSTSR = 0x35,				// OTG Status Register
	
	OINTENR = 0x36,				// OTG Interrupt Enable Register
	OINTFLR = 0x37,				// OTG Interrupt Flag Register
	
	DMACLR = 0x38,				// DMA Counter Low Byte Register
	DMACMR = 0x39,				// DMA Counter Middle Byte Register
	DMACHR = 0x3A,				// DMA Counter High Byte Register
	
	PWRCTL = 0x3B,				// Power Control Register
	PHY_CTRL1 = 0x3C,			// PHY Control Register 1
	PHY_CTRL2 = 0x3D,			// PHY Control Register 2
	TEST1_3603 = 0x3E,			    // IP Test Register	// 3603
	TEST_REG = 0x3F,			// Test Register
};

//****************************************************************************
//* 			USB_INTF Register address
//****************************************************************************
enum USB_INTF_REG
{
	DMA_RX_BASE_ADDR = 0x40,			// DMA_RX base address to access SDRAM
	DMA_RX_INDEX = 0x44,				// (W) Set DMA_RX_INDEX, (R) Current DMA_RX_INDEX
	DMA_RXED_COUNT = 0x48,			    // the byte-count transferred from USB_HOST to USB_INTF
	
	DMA_TX_BASE_ADDR = 0x50,			// DMA_TX base address to access SDRAM
	DMA_TX_INDEX = 0x54,				// (W) Set DMA_TX_INDEX, (R) Current DMA_TX_INDEX
	DMA_TX_COUNT = 0x58,				// the byte-count need to be transferred from USB_HOST to USB_INTF
	DMA_NONTX_COUNT = 0x5C,			    // (R) the byte-count not be transferred from USB_HOST to USB_INTF
	
	USBINTF_CTRL = 0x60,				// DMA_TX/DMA_RX Control Register
	USBINTF_LATENCY = 0x61,			    // USB_INTF DRAM Latency Register
	
	USBINTF_ICR = 0x62,					// USB_INTF Interrupt Control Register
	USBINTF_ISR = 0x63,					// USB_INTF Interrupt Clear/Status Register
	
	USBINTF_DMATXCS = 0x64,			    // DMA_TX_FSM Status Register
	USBINTF_DMARXCS = 0x65,			    // DMA_RX_FSM Status Register
	USBINTF_IMBCS = 0x66,				// IMB_FSM Status Register
	
	DEBUG_REG = 0x67,					// DEBUG Function Register
	DMA_AUTO_OPTION = 0x68,			    // DMA Automation Option Register
	AUTO_RX_DELAY_CNT = 0x69,			// DMA RX Automation Token IN Delay Register
	IMB_OPTION_REQ = 0x6A,				// IMB Option Setting Register
	BULK_CMD_BASE_ADDR = 0x70,		    // BULK Command Queue base address to access SDRAM
	BULK_CMD_INDEX = 0x74,				// (W)Set BULK_CMD_INDEX, (R)Current BULK_CMD_INDEX
	BULK_STS_BASE_ADDR = 0x78,			// BULK Status Queue base address to access SDRAM
	BULK_STS_INDEX = 0x7C,				// (W)Set BULK_STS_INDEX, (R)Current BULK_STS_INDEX

	EP0_HUB_SET_REG	= 0x80,             //EP0 hub setting register
	EP0_SPT_CSPT_CTL_REG = 0x81,        //EP0 SSPLIT/CSPLIT Control Register
	EPA_HUB_SET_REG	=0x82,              //EPA hub setting register
	EPA_SPT_CSPT_CTL_REG =0x83,         //EPA SSPLIT/CSPLIT Control Register
	EPB_HUB_SET_REG =0x84,              //EPB hub setting register
	EPB_SPT_CSPT_CTL_REG =0x85,         //EPB SSPLIT/CSPLIT Control Register
	EPC_HUB_SET_REG =0x86,              //EPC hub setting register
	EPC_SPT_CSPT_CTL_REG =0x87,         //EPC SSPLIT/CSPLIT Control Register
	EPD_HUB_SET_REG =0x88,              //EPD hub setting register
	EPD_SPT_CSPT_CTL_REG =0x89,         //EPD SSPLIT/CSPLIT Control Register
	EPE_HUB_SET_REG =0x8A,              //EPE hub setting register
	EPE_SPT_CSPT_CTL_REG =0x8B,         //EPE SSPLIT/CSPLIT Control Register
	EPF_HUB_SET_REG =0x8C,              //EPF hub setting register
	EPF_SPT_CSPT_CTL_REG =0x8D,         //EPF SSPLIT/CSPLIT Control Register

	HID_ADD_SET_REG = 0x8E,			    // HID Address Seting Register 
	HUB_FUNC_CTRL_REG = 0x8F,			// HUB Function Control Register
	HUB_HOTPLUG_CTRL_REG =0x90,         //Bit 2 :  Reset the Hub controller FSM  Bit 1 :  Reset the SS_CS Bit 0 :  Reset the NYET register

	HBINTERVALA = 0xA0,			// EpA Interrupt Pipe Poll Interval Register
	HBINTERVALB = 0xA1,			// EpB Interrupt Pipe Poll Interval Register
	HBINTERVALC = 0xA2,			// EpC Interrupt Pipe Poll Interval Register
	HBINTERVALD = 0xA3,			// EpD Interrupt Pipe Poll Interval Register
	HBINTERVALE = 0xA4,			// EpE Interrupt Pipe Poll Interval Register
	HBINTERVALF = 0xA5,			// EpF Interrupt Pipe Poll Interval Register

	HBW = 0xA7,					// EP High Bandwidth Setting Register
	NAKPERIOD = 0xA8,			// NAK Retry Period Register
	HPKTSIZE = 0xA9,			// Largest Packet Size Addend Register


	HUB_CTRL_REG =0x8c,                 // HUB Function Control Register 
	// Bit 7: Host_Dev_Ctr: Control the device numbers of the connected devices.
	// 0:		Device numbers  are decided by Ep(X)_Dev_Num,
	// 1:		Device numbers are the same as the Function address register.

	CS_TIMEOUT_TIME_REG =0x94,          //Hub function cotrol register

	USBDMACTR = 0x64,					// 3202 USB Dma Control Register 
	DMA_RX_BASE_ADDR_3202 = 0x6c,	// DMA_RX base address to access SDRAM 	
	DMA_TX_BASE_ADDR_3202 = 0x68,	// DMA_TX base address to access SDRAM 
	INT_CTRL_3202 = 0x70,				// INT CTRL REGISTER 

};

//**************************************************************************
// 			USB Host Registers
//**************************************************************************
// Port Control - Bit Field Definition *******
#define B_H_RESUME			0x80	// Issue host resume 
#define B_H_SUSPEND		0x40	// host Suspend 
#define B_USBRESET			0x20	// Issue usb reset 
#define B_RMT_WKP_SUPP		0x10	// Support remote wakeup 
#define B_PORT_CONN		0x02	// Device is connected 
#define B_PORT_OVER_CUR	0x01	// Port Over Current 

// Endpoint 0, A, B, C fifo control  - Bit Field Definition *******
#define B_FIFO_EMP			0x80	// Endpoint FIFO Empty 
#define B_FIFO_FUL			0x40	// Endpoint FIFO Full 
#define B_FRC_SEND			0x20	// Force sending the packet 
#define B_FIFO_INIT			0x10	// Reset the Endpoint's FIFO 
#define B_FIFO_OVER_RW		0x08	// FIFO is over-written for output and FIFO is over-read for input 
#define B_FIFO_ALL_EMP		0x04	// Fifo All empty, only for ep A, B, C 
#define B_TOKEN_SETUP		0x00	// Setup Token, only for ep 0 
#define B_TOKEN_OUT		0x01	// Out Token, only for ep 0 
#define B_TOKEN_IN			0x02	// In Token, only for ep 0 
#define B_EP_DATA0			0x00	// Dataset 0 FIFO, only for ep A, B, C 
#define B_EP_DATA1			0x02	// Dataset 1 FIFO, only for ep A, B, C 
#define B_EP_STALL			0x01	// Set the STALL Condition, only for ep A, B 

// Endpoint A, B, C Setting  - Bit Field Definition *******
#define B_EPTYPE_ISOC		0x40	// Isochronous Tansfer 
#define B_EPTYPE_BULK		0x80	// Bulk Tansfer 
#define B_EPTYPE_INTR		0xC0	// Interrupt Tansfer 
#define B_EP_NUM(n)			(n<<2)	// Endpoint Logic Number 
#define B_FIFO_PINGPONG	0x00	// Fifo Type: Ping-pong 
#define B_FIFO_SINGLE		0x01	// Fifo Type: Single 
#define B_FIFO_EXED_PP		0x10	// Fifo Type: Extend ping-pong, only for ep A, B 

// Token Control, only read  - Bit Field Definition *******
#define B_STALL_ERR			0x80	// Stall error 
#define B_PID_CHK_ERR		0x40	// PID check failure 
#define B_DEV_NOT_RSP		0x20	// Device is not responding 
#define B_UNEXPCT_PID		0x10	// Unexpected PID 
#define B_DATATOG_MIS		0x08	// Data toggle mismatch 
#define B_UTMI_ERR			0x04	// UTMI error 
#define B_CRC_ERR			0x02	// CRC Error 

// Host Interrupt Enable1  -  Bit Field Definition *******
#define B_OVERCUR_IE		0x20	// Overcurrent Interrupt Enable 
#define B_SOF_IE				0x10	// SOF Interrupt Enable 
#define B_BABBLE_IE			0x08	// Babble Interrupt Enable 
#define B_H_SUSREM_IE		0x04	// Host Suspend/Resume Finish Interrupt Enable 
#define B_USB_RST_IE		0x02	// Host USB Reset Finish Interrupt Enable 
#define B_CONN_DSCN_IE		0x01	// Device Connection Interrupt Enable 

// Host Interrupt Enable2  -  Bit Field Definition *******
#define B_EP0_FSHOK_IE		0x80	// EP0 finished OK Interrupt enable 
#define B_EPA_FSHOK_IE		0x40	// EPA finished OK Interrupt enable 
#define B_EPB_FSHOK_IE		0x20	// EPB finished OK Interrupt enable 
#define B_EPC_FSHOK_IE		0x10	// EPC finished OK Interrupt enable 
#define B_EP0_FSHERR_IE		0x08	// EP0 finished with error Interrupt enable 
#define B_EPA_FSHERR_IE	0x04	// EPA finished with error Interrupt enable 
#define B_EPB_FSHERR_IE		0x02	// EPB finished with error Interrupt enable 
#define B_EPC_FSHERR_IE		0x01	// EPC finished with error Interrupt enable 
#define B_H_INT2_ALL_IE		0xFF	// enable all host int2  

// Host Interrupt Flag1, Read-Clear  -  Bit Field Definition *******
//#define B_OVERCUR_INT		0x20	// Overcurrent Interrupt 
#define B_DEV_NO_RESP_INT	0x20	// Device No Response 
#define B_SOF_INT			0x10	// SOF Interrupt 
#define B_BABBLE_INT		0x08	// Babble Interrupt 
#define B_H_SUSREM_INT		0x04	// Host Suspend/Resume Finish Interrupt 
#define B_USB_RST_INT		0x02	// Host USB Reset Finish Interrupt 
#define B_CONN_DSCN_INT	0x01	// Device Connection/Disconnect Interrupt 

// Host Interrupt Flag2  -  Bit Field Definition *******
#define B_EP0_FSHOK_INT	0x80	// EP0 finished OK Interrupt 
#define B_EPA_FSHOK_INT	0x40	// EPA finished OK Interrupt 
#define B_EPB_FSHOK_INT	0x20	// EPB finished OK Interrupt 
#define B_EPC_FSHOK_INT	0x10	// EPC finished OK Interrupt 
#define B_EP0_FSHERR_INT	0x08	// EP0 finished with error Interrupt 
#define B_EPA_FSHERR_INT	0x04	// EPA finished with error Interrupt 
#define B_EPB_FSHERR_INT	0x02	// EPB finished with error Interrupt 
#define B_EPC_FSHERR_INT	0x01	// EPC finished with error Interrupt 

//3603 reg 0x2E: Reserved / EPFFINISHOKIE / EPEFINISHOKIE / EPDFINISHOKIE    Reserved / EPFFINISHERRIE / EPEFINISHERRIE / EPDFINISHERRIE
//3603 reg 0x2F: Reserved / EPFFINISHOKINT / EPEFINISHOKINT / EPDFINISHOKINT     Reserved / EPFFINISHERRINT / EPEFINISHERRINT / EPDFINISHERRINT
#define B_EPD_FSHOK_INT	0x10	// EPD finished OK Interrupt 
#define B_EPE_FSHOK_INT	0x20	// EPD finished OK Interrupt 
#define B_EPF_FSHOK_INT	0x40	// EPD finished OK Interrupt 
#define B_EPD_FSHERR_INT	0x01	// EPDfinished with error Interrupt 
#define B_EPE_FSHERR_INT	0x02	// EPDfinished with error Interrupt 
#define B_EPF_FSHERR_INT	0x04	// EPDfinished with error Interrupt 

#define B_EPD_FSHOK_IE		0x10	// EPD finished OK Interrupt enable 
#define B_EPE_FSHOK_IE		0x20	// EPE finished OK Interrupt enable 
#define B_EPF_FSHOK_IE		0x40	// EPF finished OK Interrupt enable 
#define B_EPD_FSHERR_IE	0x01	// EPD finished with error Interrupt enable 
#define B_EPE_FSHERR_IE		0x02	// EPE finished with error Interrupt enable 
#define B_EPF_FSHERR_IE		0x04	// EPF finished with error Interrupt enable 


/*
//29D/F reg 0x2F:Reserved / Reserved / EPDFINISHOKIE / EPDFINISHERRIE   Reserved / Reserved / EPDFINISHOKINT / EPDFINISHERRINT
#define B_EPD_FSHOK_INT	0x02	// EPD finished OK Interrupt 
#define B_EPD_FSHERR_INT	0x01	// EPDfinished with error Interrupt 
#define B_EPD_FSHOK_IE		0x20	// EPD finished OK Interrupt enable 
#define B_EPD_FSHERR_IE	0x10	// EPD finished with error Interrupt enable 
*/

// Interrupt EP Intervel Control  -  Bit Field Definition *******
#define B_EPA_INTVL_EN		0x80	// EPA Binterval Register Enable 
#define B_EPB_INTVL_EN		0x40	// EPB Binterval Register Enable 
#define B_EPC_INTVL_EN		0x20	// EPC Binterval Register Enable 

// NAK Retry Period  -  Bit Field Definition *******
#define B_NAK_RTY_NOMAL	0x00	// Normal, retry at once 
#define B_NAK1_PER_FRM		0x01	// Retry for every one (micro) frame 
#define B_NAK2_PER_FRM		0x02	// Retry for every 1/2 (micro) frame 
#define B_NAK4_PER_FRM		0x03	// Retry for every 1/4 (micro) frame 
#define B_NAK8_PER_FRM		0x04	// Retry for every 1/8 (micro) frame 
#define B_NAK16_PER_FRM	0x05	// Retry for every 1/16 (micro) frame 
#define B_NAK32_PER_FRM	0x06	// Retry for every 1/32 (micro) frame 
#define B_NAK64_PER_FRM	0x07	// Retry for every 1/64 (micro) frame 
#define B_NAK128_PER_FRM	0x08	// Retry for every 1/128 (micro) frame 
#define B_NAK256_PER_FRM	0x09	// Retry for every 1/256 (micro) frame 

// EP High Bandwidth Setting Register  -  Bit Field Definition *******
#define B_EPA_2TKN_PERFRM		0x80	// Two tokens in every micro-frame 
#define B_EPA_3TKN_PERFRM		0xC0	// Three tokens in every micro-frame 
#define B_EPB_2TKN_PERFRM		0x20	// Two tokens in every micro-frame 
#define B_EPB_3TKN_PERFRM		0x30	// Three tokens in every micro-frame 
#define B_EPC_2TKN_PERFRM		0x08	// Two tokens in every micro-frame 
#define B_EPC_3TKN_PERFRM		0x0C	// Three tokens in every micro-frame 


//**************************************************************************
// 			USB IP Level Registers
//**************************************************************************
// Endpoint Size  -  Bit Field Definition *******
#define B_EPB_TEST		0x80	// FIFO Test 
#define B_EPBSIZE_8		0x00	// EPB size used of Bulk and Interrupt transfer in FS 
#define B_EPBSIZE_16	0x10
#define B_EPBSIZE_32	0x20	
#define B_EPBSIZE_64	0x30
#define B_EPBSIZE_128	0x40
#define B_EPBSIZE_256	0x50
#define B_EPBSIZE_512	0x60
#define B_EPBSIZE_1024	0x70
#define B_EPA_TEST		0x08	// USB IP Timer Test Mode 
#define B_EPASIZE_8		0x00	// EPA size used of Bulk and Interrupt transfer in FS 
#define B_EPASIZE_16	0x01
#define B_EPASIZE_32	0x02
#define B_EPASIZE_64	0x03
#define B_EPASIZE_128	0x04
#define B_EPASIZE_256	0x05
#define B_EPASIZE_512	0x06
#define B_EPASIZE_1024	0x07

// IP Test1  -  Bit Field Definition *******
#define B_IPTEST_FIFO		0x80	// FIFO Test 
#define B_IPTEST_TMR		0x40	// USB IP Timer Test Mode 
#define B_SETUPDATA_LOCK	0x20	// Setup Data Lock Enable 

//**********************************
// When the IP is in the device mode, 
// auto-clear USBON register when USBVBUS signal is high and POWERDOWM=0
//*********************************
#define B_CLRUSBON_EN		0x10 								
#define B_EPA_AUTOFORCE	0x08	// Endpoint A auto force enable when DMA counter down counts form 1 to 0, or EOT event 
#define B_EPA_ISODMY_EN	0x04	// EPA ISO pipe insert dummy bytes enable 
#define B_EPA_ISOHBW		0x02	// 2 Transfer per uSOF 

// USB Speed Register  -  Bit Field Definition *******
#define B_OPER_HS			0x00	// Operational speed: HS 
#define B_OPER_FS			0x20	// Operational speed: FS 
#define B_OPER_LS			0x40	// Operational speed: LS 
#define B_DEV_HS			0x00	// for device mode, High Speed 
#define B_DEV_FS			0x10	// for device mode, Full Speed 
#define B_HOST_U20			0x00	// for host mode, Support USB2.0 
#define B_HOST_U11			0x10	// for host mode, Support USB1.1 
#define B_DMATRANS_EN 		0x08	// Enable DMA transfer 
#define B_EPB_HOST_IN		0x04	// Host IN token (only for EPB) 

// PHY Test Register  -  Bit Field Definition *******
#define B_VBUSRST_EN		0x40	// USB link layer is reset when USB VBUS is inavailable 
#define B_VBUSRST_DN		0x00	// Disable USB link layer reset 
#define B_HOST_UDCRST		0x08	// Software Reset USBIP 	
#define B_EXT_CLK_EN		0x04	// External Clock Enable (read only) 
#define B_DMY_MODE			0x01	// For ISOC Endpoint Only 

// OTG Control Register  -  Bit Field Definition *******
#define B_HOST_MODE		0x90	// Host mode 
#define B_DEV_MODE			0x80	// Device mode 
#define B_CNT_DOWN_EN		0x08	// Enable Counter to Count Down 

// OTG Status Register  -  Bit Field Definition *******
#define B_B_CNCT_OK			0x04	// B Device Connect OK. (Long debounce), Check after timeout interrupt

// OTG Interrupt Enable Register  -  Bit Field Definition *******
#define B_ID_CHANGE_IE		0x80	// ID changes (cable is moved, etc.) interrupt enable 
#define B_TIMEOUT_IE		0x40	// Timer count down to 0 interrupt enable 
#define B_VBUS_INVLD_IE		0x20	// VBUS VLD from 1 to 0 interrupt enable 
#define B_CNTINT_IE			0x10	// Data count down to zero enable 
#define B_FINISHINT_IE		0x08	// Transfer finish interrupt enable 
#define B_OTG_ALL_IE		0xF8	// OTG all interrupt enable 

// OTG Interrupt Flag Register  -  Bit Field Definition *******
#define B_ID_CHANGE			0x80	// ID changes (cable is moved, etc.) interrupt 
#define B_TIMEOUT			0x40	// Timer count down to 0 interrupt 
#define B_VBUS_INVLD		0x20	// VBUS VLD from 1 to 0 interrupt 
#define B_CNTINT			0x10	// Data count down to zero 
#define B_FINISHINT			0x08	// Transfer finish interrupt 
#define B_OTGINTMASK		0xF8	// all OTG interrupt 


// Power Control Register  -  Bit Field Definition *******
#define B_EPATXSTS_ACK		0x00	// EPA TX Status of Last Transaction: ACK 
#define B_EPATXSTS_NAK		0x80	// EPA TX Status of Last Transaction: NAK 
#define B_EPCTXSTS_ACK		0x00	// EPC TX Status of Last Transaction: ACK 
#define B_EPCTXSTS_NAK		0x40	// EPC TX Status of Last Transaction: NAK 
#define B_UCLKGATEEN1J		0x08	// Clock gating function for FM/TQ in UDC disable 
#define B_UCLKGATEENJ		0x04	// Clock gating function for PIE/UBL/EPI in UDC disable 
#define B_FCLKON			0x02	// FIFO clock on (only used when FCLKGATEENJ = 1) 
#define B_FCLKGATEENJ		0x01	// EPFIFO gate clock function disable 


// PHY Control Register 2  -  Bit Field Definition *******
#define B_TST_LOOPBACKEN		0x80	// Loop Back Test Enable 
#define B_DYNA_HSTX_DEV_EN	0x40	// Dynamic HS TX function enable for device mode 
#define B_DYNA_HSTX_HST_EN	0x20	// Dynamic HS TX function enable for host mode 
#define B_HSTX_EN				0x10	// HS TX Function Enable 
#define B_HSRX_EN				0x08	// HS RX Function Enable 
#define B_PD_HS					0x04	// Power Down HS PHY 
#define B_PD_FS					0x02	// Power Down FS PHY 
#define B_PD_DLL				0x01	// Power Down PLL 

// Certification Test Register  -  Bit Field Definition *****
#define B_DISCONNECT_EN		0x80	// Disconnect Enable 
#define B_TEST_EN				0x40	// Test Enable 
#define B_RST_50MS_EN			0x20	// Reset 50ms Enable 
#define B_TEST_FORCE_EN		0x10	// Test Force Enable 
#define B_HST_TEST_PKT_EN		0x08	// Host Test Packet 
#define B_HST_TEST_SE0_EN		0x04	// Host Test SE0 
#define B_HST_TEST_K_EN		0x02	// Host Test K 
#define B_HST_TEST_J_EN		0x01	// Host Test J 


//**************************************************************************
// 			USB_INTF Registers
//**************************************************************************
// DMA_TX/DMA_RX Control  -  Bit Field Definition *******
#define B_BULK_OPT_CLR		0x80	// Clear BULK_OPT_CTRL_FSM status, one cycle and auto-clear to 0 by itself 
#define B_BULK_OPT_START	0x40	// Enable BULK_OPT_CTRL_FSM to process BULK command queue 
#define B_DMA_RX_CLR		0x20	// Clear DMA_RX_FSM status, one cycle and auto-clear to '0' by itself 
#define B_DMA_RX_START 	0x10	// Enable DMA_RX_FSM to receive data from USB_HOST, one cycle and auto-clear to '0' by itself 
#define B_DMA_TX_CLR		0x02	// Clear DMA_RX_FSM status, one cycle and auto-clear to '0' by itself 
#define B_DMA_TX_START		0x01	// Enable DMA_TX_FSM to transfer data from USB_HOST, one cycle and auto-clear to '0' by itself 

// USB_INTF DRAM Latency Register  -  Bit Field Definition *******
#define B_UIF_LATENCY_16	0x00	// 16 mem_clk cycles 
#define B_UIF_LATENCY_32	0x01	// 32 mem_clk cycles 
#define B_UIF_LATENCY_240	0x0E	// 240 mem_clk cycles 

// USB_INTF Interrupt Control  -  Bit Field Definition *******
#define B_BULK_FIN_INT_DN		0x10	// Mask USB BULK_FINISH_INT to CPU interrupt 
#define B_UIP_IRQ_DN			0x08	// Defult: Mask USB HOST IP interrupt to CPU interrupt 
#define B_UIP_IRQ_EN			0x00	// Enable USB HOST IP interrupt to CPU interrupt 
#define B_RXED_INT_DN			0x04	// Mask USB DMA RXED_INT to CPU interrupt 
#define B_RXED_INT_EN			0x00	// Enable USB DMA RXED_INT to CPU interrupt 
#define B_TXED_INT_DN			0x02	// Mask USB DMA TXED_INT to CPU interrupt 
#define B_TXED_INT_EN			0x00	// Enable USB DMA TXED_INT to CPU interrupt 
#define B_USB2CPU_INT_DN		0x01	// Mask USB to CPU interrupt 
#define B_USB2CPU_INT_EN		0x00	// Enable USB to CPU interrupt 
#define B_U2C_ALL_INT_MASK 	0x1F	// Mask all USB to CPU interrupt 

// USB_INTF Interrupt Clear/Status Register  -  Bit Field Definition *******
#define B_BULK_FIN_INT		0x10	// (RC) USB_INTF BULK command queue finish interrupt 
#define B_USB_IP_IRQ		0x08	// USB IP IRQ occur 
#define B_RXED_INT			0x04	// USB_INTF DMA_RX_FSM received all data to USB_HOST interrupt 
#define B_TXED_INT			0x02	// USB_INTF DMA_TX_FSM transferred all data to USB_HOST interrupt 
#define B_USB2CPU_INT		0x01	// USB_INTF to North-Bridge interrupt 

//DMA Automation Option  -  Bit Field Definition *******
#define B_AUTO_RX_EN		0x80	// Auto force Token IN after receiving EPB_FINISH_OK_INT 
#define B_RX_PSIZE_8		0x00	// RX Packet Size is 8 
#define B_RX_PSIZE_16		0x10	// RX Packet Size is 16 
#define B_RX_PSIZE_32		0x20	// RX Packet Size is 32 
#define B_RX_PSIZE_64		0x30	// RX Packet Size is 64 
#define B_RX_PSIZE_128		0x40	// RX Packet Size is 128 
#define B_RX_PSIZE_256		0x50	// RX Packet Size is 256 
#define B_RX_PSIZE_512		0x60	// RX Packet Size is 512 
#define B_RX_PSIZE_1024		0x70	// RX Packet Size is 1024 
#define B_TX_ALIGN_CHK_EN	0x08	// Count DMA_TX_COUNT to see if the size is packet size aligned 
#define B_TX_PSIZE_8		0x00	// TX Packet Size is 8 
#define B_TX_PSIZE_16		0x01	// TX Packet Size is 16 
#define B_TX_PSIZE_32		0x02	// TX Packet Size is 32 
#define B_TX_PSIZE_64		0x03	// TX Packet Size is 64 
#define B_TX_PSIZE_128		0x04	// TX Packet Size is 128 
#define B_TX_PSIZE_256		0x05	// TX Packet Size is 256 
#define B_TX_PSIZE_512		0x06	// TX Packet Size is 512 
#define B_TX_PSIZE_1024		0x07	// TX Packet Size is 1024 

/*
// 29D/F reg 0x25
#define B_INTERVAL_A_ENABLE     0x80   //B interval A register Enale
#define B_INTERVAL_B_ENABLE     0x40   //B interval B register Enale
#define B_INTERVAL_C_ENABLE     0x20   //B interval C register Enale
#define B_INTERVAL_D_ENABLE     0x10   //B interval D register Enale
*/

//3603 reg 0x2B
#define B_INTERVAL_A_ENABLE     0x01   //B interval A register Enale
#define B_INTERVAL_B_ENABLE     0x02   //B interval B register Enale
#define B_INTERVAL_C_ENABLE     0x04   //B interval C register Enale
#define B_INTERVAL_D_ENABLE     0x08   //B interval D register Enale
#define B_INTERVAL_E_ENABLE     0x10   //B interval D register Enale
#define B_INTERVAL_F_ENABLE     0x20   //B interval D register Enale

#define HUB_SS_CS_ENABLE     0x01

#define HUB_EP0_SS_CS_IE   0x40  //Enable SS_CS function
#define HUB_EPA_SS_CS_IE   0x20  //Enable SS_CS function
#define HUB_EPB_SS_CS_IE   0x10  //Enable SS_CS function
#define HUB_EPC_SS_CS_IE   0x08  //Enable SS_CS function
#define HUB_EPD_SS_CS_IE   0x04  //Enable SS_CS function


//offset 0x10 bit 7
static inline void issue_host_resume(UINT32 addr)
{
	if(0)// ((sys_ic_get_chip_id() == ALI_M3329E)&&(sys_ic_get_rev_id() <IC_REV_5))
	{
		*((volatile unsigned char *)0xb800007C)  &=  ~(0x0C);  // Clear Squelch Level setting,USB_PLL_SEL: DISC_SEL/PD_HS/PD_FS/PD_PLL . SQUELCH_LEVEL -/TX_DRIVING -
	}
	hc_WriteByte(addr, HPORTCTLR, (hc_ReadByte(addr, HPORTCTLR) | B_H_RESUME));
}

//offset 0x10 bit 6
static inline void issue_host_suspend(UINT32 addr)
{
	if (0)//((sys_ic_get_chip_id() == ALI_M3329E)&&(sys_ic_get_rev_id()<IC_REV_5))
	{
		*((volatile unsigned char *)0xb800007C) |= 0x0C;	// set  Squelch Level setting,USB_PLL_SEL: DISC_SEL/PD_HS/PD_FS/PD_PLL . SQUELCH_LEVEL -/TX_DRIVING -
	}
	hc_WriteByte(addr, HPORTCTLR, (hc_ReadByte(addr, HPORTCTLR) | B_H_SUSPEND));
}

//offset 0x10 bit 6
static inline void clear_host_suspend(UINT32 addr)
{
	if (0)//((sys_ic_get_chip_id() == ALI_M3329E)&&(sys_ic_get_rev_id()<IC_REV_5))
	{
		*((volatile unsigned char *)0xb800007C) &=  ~(0x0C);  // Clear Squelch Level setting,USB_PLL_SEL: DISC_SEL/PD_HS/PD_FS/PD_PLL . SQUELCH_LEVEL -/TX_DRIVING -
	}
	hc_WriteByte(addr, HPORTCTLR, (hc_ReadByte(addr, HPORTCTLR) & (~B_H_SUSPEND)));
}

//offset 0x10 bit 5
static inline void issue_reset_to_device(UINT32 addr)
{
	//hc_WriteByte(addr, HPORTCTLR, (hc_ReadByte(addr, HPORTCTLR) | B_USBRESET));
	hc_WriteByte(addr, HPORTCTLR,  B_USBRESET);
}

//offset 0x10 bit 4
static inline void remote_wakeup_support(UINT32 addr, BOOL bSupport)
{
	UINT8 uTemp = hc_ReadByte(addr, HPORTCTLR);
	hc_WriteByte(addr, HPORTCTLR, ((bSupport == TRUE) ? (uTemp | B_H_RESUME) : (uTemp & (~B_H_RESUME))));
}

//offset 0x10 bit 1
static inline BOOL get_port_connect_status(UINT32 addr)
{
	return ((hc_ReadByte(addr, HPORTCTLR) & B_PORT_CONN) ? TRUE : FALSE);
}

//offset 0x10 bit 0
static inline BOOL get_over_current_status(UINT32 addr)
{
	return ((hc_ReadByte(addr, HPORTCTLR) & B_PORT_OVER_CUR) ? TRUE : FALSE);
}

//offset 0x11, 0x14, 0x17 or 0x1A bit 7
static inline BOOL get_ep_fifo_empty_status(UINT32 addr, UINT8 ep)
{
	UINT8 uTemp = hc_ReadByte(addr, ep);
	return (uTemp & B_FIFO_EMP) ? TRUE : FALSE;
}

static inline BOOL get_ep_fifo_all_empty_status(UINT32 addr, UINT8 ep)
{
	UINT8 uTemp = hc_ReadByte(addr, ep);
	return (uTemp & B_FIFO_ALL_EMP) ? TRUE : FALSE;
}

//offset 0x11, 0x14, 0x17 or 0x1A bit 6
static inline BOOL get_ep_fifo_full_status(UINT32 addr, UINT8 ep)
{
	UINT8 uTemp = hc_ReadByte(addr, ep);
	return (uTemp & B_FIFO_FUL) ? TRUE : FALSE;
}

//offset 0x11, 0x14, 0x17 or 0x1A bit 5
static inline void ep_force_token_sending(UINT32 addr, UINT8 ep)
{
	hc_WriteByte(addr, ep, (hc_ReadByte(addr, ep) | B_FRC_SEND));
}

//offset 0x11, 0x14, 0x17 or 0x1A bit 4
static inline void init_ep_fifo(UINT32 addr, UINT8 ep)
{
#if 1
	hc_WriteByte(addr, ep, B_FIFO_INIT);
	hc_s3602_delay(1);
	hc_WriteByte(addr, ep, 0x00);

#else
	hc_WriteByte(ep, (hc_ReadByte(ep) | B_FIFO_INIT));
	hc_s3602_delay(200);
	hc_WriteByte(ep, (hc_ReadByte(ep) & (~B_FIFO_INIT)));
#endif	
}


static inline void init_ep0_fifo(UINT32 addr)
{
	hc_WriteByte(addr, HEP0FCTR, B_FIFO_INIT);
	hc_s3602_delay(1);
	hc_WriteByte(addr, HEP0FCTR, (hc_ReadByte(addr,HEP0FCTR)|0x04));
}

//offset 0x11, 0x14, 0x17 or 0x1A bit 3
static inline BOOL get_ep_ovfunf_status(UINT32 addr, UINT8 ep)
{
	return ((hc_ReadByte(addr, ep) & B_FIFO_OVER_RW) ? TRUE : FALSE);
}


//offset  0x14, 0x17  bit 1
static inline void set_ep_data_toggle(UINT32 addr, UINT8 ep )
{
	UINT8 uTemp = hc_ReadByte(addr, ep);
	hc_WriteByte(addr, ep, (uTemp | 0x01) );
	hc_s3602_delay(1);
	hc_WriteByte(addr, ep, (uTemp & 0xFE) );
	hc_s3602_delay(1);
//	uTemp = hc_ReadByte(addr, ep);
//	USB_DEBUG_PRINTF("\n\n ------set_ep_data_toggle ep:%02x, %02x \n\n",ep,uTemp);
}
//offset 0x11 bit 1-0
static inline void set_ep0_token_pid(UINT32 addr, UINT8 pid)
{
	UINT8 uTemp = hc_ReadByte(addr, HEP0FCTR);
	hc_WriteByte(addr, HEP0FCTR, (uTemp & 0xFC) | pid);
}

//offset 0x12, 0x15, 0x18 and 0x1B
static inline void rw_ep_fifo_port(UINT32 addr, UINT8 ep, BOOL bIsWrite, UINT8 *data)
{
	if (bIsWrite == TRUE)
		hc_WriteByte(addr, ep, *data);
	else
		*data = hc_ReadByte(addr, ep);
}

//offset 0x13, 0x16, and 0x19 bit7-6
static inline void set_ep_transfer_type(UINT32 addr, UINT8 ep, UINT8 type)
{
	UINT8 uTemp = hc_ReadByte(addr, ep);
	hc_WriteByte(addr, ep, (uTemp & 0x3F) | type);
}

//offset 0x13, 0x16, and 0x19 bit5-2
static inline void set_ep_logical_num(UINT32 addr, UINT8 ep, UINT8 logical_num)
{
	UINT8 uTemp = hc_ReadByte(addr, ep);
	hc_WriteByte(addr, ep, (uTemp & 0xC3) | B_EP_NUM(logical_num));
}

//offset 0x13, 0x16, and 0x19 bit1-0
static inline void set_fifo_type(UINT32 addr, UINT8 ep, UINT8 fifo_type)
{
	UINT8 uTemp = hc_ReadByte(addr, ep);
	if (ep == HEPCSETR && fifo_type > B_FIFO_SINGLE)		//No extend ping-pong for EPC
		fifo_type = B_FIFO_PINGPONG;		
	hc_WriteByte(addr, ep, (uTemp & 0xFC) | fifo_type);
}

//offset 0x1C
static inline void set_device_address(UINT32 addr, UINT8 fun_addr)
{
	hc_WriteByte(addr, HFNCADD, fun_addr);
}

//offset 0x1D
static inline UINT8 get_token_err_infor(UINT32 addr)
{
	return hc_ReadByte(addr, HTKNCC1);
}

//offset 0x1E
static inline void en_host_int_flag1(UINT32 addr, UINT8 type)
{
	hc_WriteByte(addr, HINTENR1, type);
}

//offset 0x1E except for SOF and overcurrent and BABBLE
static inline void en_host_int_flag1_all(UINT32 addr)
{
	hc_WriteByte(addr, HINTENR1, 0x07);
}

//offset 0x1E
static inline UINT8 get_host_int_en_flag1(UINT32 addr)
{
	return hc_ReadByte(addr, HINTENR1);
}

//offset 0x1F
static inline void en_host_int_flag2(UINT32 addr, UINT8 type)
{
	hc_WriteByte(addr, HINTENR2, type);
}


//offset 0x1F
static inline void en_host_int_flag2_all(UINT32 addr)
{
	hc_WriteByte(addr, HINTENR2, 0xFF);
}

//offset 0x2E
static inline void en_host_int_flag3_all(UINT32 addr)
{
//	hc_WriteByte(addr, HINTFLR3, 0x30);	// 29D/F
	hc_WriteByte(addr, HINTENR3, 0xFF);
}

//offset 0x1E
static inline void mask_host_int_flag1(UINT32 addr, UINT8 type)
{
	hc_WriteByte(addr, HINTENR1, (hc_ReadByte(addr, HINTENR1) & ~type));
}

//offset 0x1F
static inline void mask_host_int_flag2(UINT32 addr, UINT8 type)
{
	hc_WriteByte(addr, HINTENR2, (hc_ReadByte(addr, HINTENR2) & ~type));
}

//offset 0x1F
static inline UINT8 get_host_int_en_flag2(UINT32 addr)
{
	return hc_ReadByte(addr, HINTENR2);
}

//offset 0x2E
static inline UINT8 get_host_int_en_flag3(UINT32 addr)
{
	return hc_ReadByte(addr, HINTENR3);	// 3603
}

//offset 0x20
static inline UINT8 get_host_int_flag1(UINT32 addr)
{
	return hc_ReadByte(addr, HINTFLR1);
}
/*
//offset 0x2f
static inline UINT8 get_host_int_en_flag3(UINT32 addr)
{
	return hc_ReadByte(addr, HINTFLR3);	// 29D F
}
*/
//offset 0x21
static inline UINT8 get_host_int_flag2(UINT32 addr)
{
	return hc_ReadByte(addr, HINTFLR2);
}

//offset 0x22, 0x23 and 0x24
static inline void set_int_interval_value(UINT32 addr, UINT8 ep, UINT8 interval_value)
{
	hc_WriteByte(addr, ep, interval_value);
}

//offset 0x25
static inline void en_int_interval_ctrl(UINT32 addr, UINT8 en_ep_interval)
{
	hc_WriteByte(addr, HBINTERVALCTRL, (hc_ReadByte(addr,HBINTERVALCTRL))|en_ep_interval);
}

//offset 0x25
static inline void mask_int_interval_ctrl(UINT32 addr, UINT8 en_ep_interval)
{
	hc_WriteByte(addr, HBINTERVALCTRL, (hc_ReadByte(addr,HBINTERVALCTRL))&(~en_ep_interval));
}

//offset 0x26
static inline void set_nak_retry_period(UINT32 addr, UINT8 period)
{
	hc_WriteByte(addr, NAKPERIOD, period);
}

//offset 0x28
static inline void hb_ep_token_period(UINT32 addr, UINT8 data)
{
	hc_WriteByte(addr, HBW, data);
}

//offset 0x2A and 0x2B
static inline UINT16 get_epb_last_data_size(UINT32 addr)
{
	return ((hc_ReadByte(addr, EPB_DMA_CNTH)<<8) | hc_ReadByte(addr, EPB_DMA_CNTL));
}

//offset 0x2A and 0x2B
static inline UINT16 get_epb_last_data_size_3603(UINT32 addr)
{
	return ((hc_ReadByte(addr, EPB_DMA_CNTH_3603)<<8) | hc_ReadByte(addr, EPB_DMA_CNTL_3603));
}

//offset 0x30 bit2-0
static inline void set_epa_packet_size(UINT32 addr, UINT8 pkt_size)
{
	//hc_WriteByte(addr, EPSIZE, (hc_ReadByte(addr, EPSIZE) & 0xF8) | pkt_size);
	hc_WriteByte(addr, EPSIZE, (hc_ReadByte(addr, EPSIZE) & 0xF8) | pkt_size|0x8);
}

//offset 0x30 bit6-4
static inline void set_epb_packet_size(UINT32 addr, UINT8 pkt_size)
{
//	hc_WriteByte(addr, EPSIZE,
//		(hc_ReadByte(addr, EPSIZE) & 0x8F) | (pkt_size << 4));
		hc_WriteByte(addr, EPSIZE,
		(hc_ReadByte(addr, EPSIZE) & 0x8F) | (pkt_size << 4)|0x80);
}

static inline void set_epc_packet_size(UINT32 addr, UINT8 pkt_size)
{
	hc_WriteByte(addr, EPCDSIZE,
		(hc_ReadByte(addr, EPCDSIZE))| (pkt_size ));
}

static inline void set_epd_packet_size(UINT32 addr, UINT8 pkt_size)
{
	hc_WriteByte(addr, EPCDSIZE,
		(hc_ReadByte(addr, EPCDSIZE)) | (pkt_size << 4));
}

//offset 0x31 bit3
static inline void en_epa_auto_force(UINT32 addr)
{
	hc_WriteByte(addr, TEST1, (hc_ReadByte(addr, TEST1) | B_EPA_AUTOFORCE));
}
//offset 0x3E bit3
static inline void en_epa_auto_force_3603(UINT32 addr)
{
	hc_WriteByte(addr, TEST1_3603, (hc_ReadByte(addr, TEST1_3603) | B_EPA_AUTOFORCE));
}

//offset 0x32 bit6-5
static inline UINT8 get_device_speed(UINT32 addr)
{
	return (hc_ReadByte(addr, USBSPD) & 0x60);
}

//offset 0x32 bit3
static inline void en_dma_transfer(UINT32 addr, BOOL bEn)
{
	volatile UINT8 bTemp = hc_ReadByte(addr, USBSPD);
	hc_WriteByte(addr, USBSPD, ((bEn == TRUE) ? (bTemp | B_DMATRANS_EN) : (bTemp & (~B_DMATRANS_EN))));
}

//offset 0x32 bit2
static inline void dma_host_in_token(UINT32 addr, BOOL bEn)
{
	UINT8 bTemp = hc_ReadByte(addr, USBSPD);
	hc_WriteByte(addr, USBSPD, ((bEn == TRUE) ? (bTemp | B_EPB_HOST_IN) : (bTemp & (~B_EPB_HOST_IN))));
}

//offset 0x33 bit3
static inline void software_rst_usbip(UINT32 addr)
{
	hc_WriteByte(addr, INTFLG2, (hc_ReadByte(addr, INTFLG2) | B_HOST_UDCRST));
	hc_s3602_delay(20);
	hc_WriteByte(addr, INTFLG2, (hc_ReadByte(addr, INTFLG2) & (~B_HOST_UDCRST)));
}

//offset 0x34 bit4
static inline void set_usbip_host_mode(UINT32 addr)
{
	hc_WriteByte(addr, OCTLR, (hc_ReadByte(addr, OCTLR) | B_HOST_MODE));
}

//M3202 offset 0x34 bit2 
static inline void usb_dma_init(UINT32 addr)
{
	hc_WriteByte(addr, USBTLR, 0x04);
	hc_s3602_delay(1);
	hc_WriteByte(addr, USBTLR, 0x00);
}

//M3202 offset 0x34 bit4
static inline void en_3202_dma_transfer(UINT32 addr, UINT8 in_out)
{
	UINT32 data = hc_ReadByte(addr, USBTLR);

	data &= (~(1<<6));
	data |=  (in_out<<6);
	//soc_printf("34 bit6 data = %x\n", data);
	hc_WriteDword(addr, USBTLR, data);
	
	data |= 0x80;

	//soc_printf("34 data = %x\n", data);
	//hc_WriteByte(addr, USBTLR, data);
	//hc_WriteByte(addr, USBTLR, 0x88);
	hc_WriteDword(addr, USBTLR, data);
}

//M3202 offset 0x34 bit4
static inline void disable_dma_transfer(UINT32 addr)
{
	hc_WriteDword(addr, USBTLR, 0);
}

//M3202 offset 0x64
static inline void clear_dma_interrupt(UINT32 addr, UINT8 status)
{
	hc_WriteByte(addr, USBDMACTR, status); // 3202 usb1.1
}

//M3202 offset 0x64 bit0, bit1, bit2
static inline UINT8 get_dma_interrupt_status(UINT32 addr)
{
	return hc_ReadByte(addr, USBDMACTR); // 3202 usb1.1
}
//offset 0x38, 0x39 and 0x3A
static inline void set_dma_rx_cnt(UINT32 addr, UINT32 data_len)
{
	hc_WriteByte(addr, DMACLR, (UINT8)(data_len & 0x000000FF));
	hc_WriteByte(addr, DMACMR, (UINT8)((data_len & 0x0000FF00)>>8));
	hc_WriteByte(addr, DMACHR, (UINT8)((data_len & 0x00FF0000)>>16));
}

//M3202 offset 0x04~0x07  -->  The address need to be 32 bytes align
static inline void set_3202_dma_rx_base_addr(UINT32 addr, UINT32 base_addr)
{
	hc_WriteDword(addr, DMA_RX_BASE_ADDR_3202, base_addr & 0x0FFFFFE0);
}


//M3202 offset 0x00~0x03  -->  The address need to be 32 bytes align
static inline void set_3202_dma_tx_base_addr(UINT32 addr, UINT32 base_addr)
{
	hc_WriteDword(addr, DMA_TX_BASE_ADDR_3202, base_addr & 0x0FFFFFE0);
}

//offset 0x3F
static inline void en_certification_test(UINT32 addr, UINT8 test_type)
{
	hc_WriteByte(addr, TEST_REG, test_type);
}

//offset 0x40~0x43  -->  The address need to be 32 bytes align
static inline void set_dma_rx_base_addr(UINT32 addr, UINT32 base_addr)
{
	//libc_printf("RX base_addr= %x\n",base_addr);
	hc_WriteDword(addr, DMA_RX_BASE_ADDR, base_addr & 0x0FFFFFE0);
}

//offset 0x44~0x47  -->  It need to be 32 bytes align
static inline UINT32 read_dma_rx_index(UINT32 addr)
{
	return (hc_ReadDword(addr, DMA_RX_INDEX));
	//return (hc_ReadDword(DMA_RX_INDEX)>>5);
}

//offset 0x44~0x47  -->  It need to be 32 bytes align
static inline void clear_dma_rx_index(UINT32 addr)
{
	hc_WriteDword(addr, DMA_RX_INDEX,0x00000000);
}

//offset 0x44~0x47  -->  It need to be 32 bytes align
static inline void set_dma_rx_index(UINT32 addr, UINT32 index_offset)
{
	hc_WriteDword(addr, DMA_RX_INDEX, ((index_offset<<5) & 0x0FFFFFE0));
}

//offset 0x48~0x4B
static inline void set_dma_rx_transfer_byte(UINT32 addr, UINT32 byte_cnt)
{
	hc_WriteDword(addr, DMA_RXED_COUNT, byte_cnt & 0x00FFFFFF);
}

//offset 0x48~0x4B
static inline UINT32 read_dma_rx_count(UINT32 addr)
{
	return (hc_ReadDword(addr, DMA_RXED_COUNT) & 0x00FFFFFF);
}

//offset 0x50~0x43  -->  The address need to be 32 bytes align
static inline void set_dma_tx_base_addr(UINT32 addr, UINT32 base_addr)
{
	//libc_printf("TX base_addr= %x\n",base_addr);
	hc_WriteDword(addr, DMA_TX_BASE_ADDR, base_addr & 0x0FFFFFE0);
}

//offset 0x54~0x57  -->  It need to be 32 bytes align
static inline UINT32 read_dma_tx_index(UINT32 addr)
{
	return (hc_ReadDword(addr, DMA_TX_INDEX)>>5);
}

//offset 0x54~0x57  -->  It need to be 32 bytes align
static inline void clear_dma_tx_index(UINT32 addr)
{
	hc_WriteDword(addr, DMA_TX_INDEX, 0x00000000);
}

//offset 0x54~0x57  -->  It need to be 32 bytes align
static inline void set_dma_tx_index(UINT32 addr, UINT32 index_offset)
{
	hc_WriteDword(addr, DMA_TX_INDEX, ((index_offset<<5) & 0x0FFFFFE0));
}

//offset 0x58~0x5B
static inline void set_dma_tx_transfer_byte(UINT32 addr, UINT32 byte_cnt)
{
	hc_WriteDword(addr, DMA_TX_COUNT, byte_cnt & 0x00FFFFFF);
}

//offset 0x5C~0x5F
static inline UINT32 read_dma_tx_remain_byte(UINT32 addr)
{
	return hc_ReadDword(addr, DMA_NONTX_COUNT);
}

//offset 0x60 bit7
static inline void init_bulk_opt_state(UINT32 addr)
{
	hc_WriteByte(addr, USBINTF_CTRL, (hc_ReadByte(addr, USBINTF_CTRL) | B_BULK_OPT_CLR));
}

//offset 0x60 bit6
static inline void start_bulk_opt_transfer(UINT32 addr)
{
	hc_WriteByte(addr, USBINTF_CTRL, (hc_ReadByte(addr, USBINTF_CTRL) | B_BULK_OPT_START));
}

//offset 0x60 bit5
static inline void init_dma_rx_state(UINT32 addr)
{
	hc_WriteByte(addr, USBINTF_CTRL, (hc_ReadByte(addr, USBINTF_CTRL) | B_DMA_RX_CLR));
}

//offset 0x60 bit4
static inline void start_dma_rx_transfer(UINT32 addr)
{
	hc_WriteByte(addr, USBINTF_CTRL, (hc_ReadByte(addr, USBINTF_CTRL) | B_DMA_RX_START));
}

//offset 0x60 bit1
static inline void init_dma_tx_state(UINT32 addr)
{
	hc_WriteByte(addr, USBINTF_CTRL, (hc_ReadByte(addr, USBINTF_CTRL) | B_DMA_TX_CLR));
}

//offset 0x60 bit0
static inline void start_dma_tx_transfer(UINT32 addr)
{
	hc_WriteByte(addr, USBINTF_CTRL, (hc_ReadByte(addr, USBINTF_CTRL) | B_DMA_TX_START));
}

//offset 0x61
static inline void set_dram_latency(UINT32 addr, UINT8 latency)
{
	hc_WriteByte(addr, USBINTF_LATENCY, latency);
}

//offset 0x62
static inline void enable_all_usb_intf_int(UINT32 addr)
{
	hc_WriteByte(addr, USBINTF_ICR, 0x00);
}

//offset 0x62
static inline void enable_usb_intf_int(UINT32 addr,UINT8 Int)
{
	hc_WriteByte(addr, USBINTF_ICR, Int);
}

//offset 0x62
static inline void disable_all_usb_intf_int(UINT32 addr)
{
	hc_WriteByte(addr, USBINTF_ICR, B_U2C_ALL_INT_MASK);
}

//offset 0x62
static inline void mask_usb_intf_int(UINT32 addr, UINT8 mask_type)
{
	hc_WriteByte(addr, USBINTF_ICR, (hc_ReadByte(addr, USBINTF_ICR) | mask_type));
}

//offset 0x63
static inline UINT8 get_usb_intf_int_status(UINT32 addr)
{
	return hc_ReadByte(addr, USBINTF_ISR);
}

//offste 0x67 bit 5
/*
static inline void set_token_in_by_hw(UINT32 addr)
{
	if(sys_ic_get_chip_id() == ALI_M3329E)
	{
		hc_WriteByte(addr, DEBUG_REG, 0);
	}
	else
		hc_WriteByte(addr, DEBUG_REG, (hc_ReadByte(addr, DEBUG_REG) |0x20));
}
*/
static inline void set_token_in_by_hw_29(UINT32 addr)
{
	hc_WriteByte(addr, DEBUG_REG, 0);
}
static inline void set_token_in_by_hw_36(UINT32 addr)
{
	hc_WriteByte(addr, DEBUG_REG, (hc_ReadByte(addr, DEBUG_REG) |0x20));
}

//offste 0x67 bit 5
static inline void set_token_in_by_pio(UINT32 addr)
{
	hc_WriteByte(addr, DEBUG_REG, (hc_ReadByte(addr, DEBUG_REG) |0x00));
}

//offset 0x68
static inline void disable_dma_auto_option(UINT32 addr)
{
	hc_WriteByte(addr, DMA_AUTO_OPTION, 0x00);
}

//offset 0x68 bit 7~4
static inline void enable_dma_auto_rx(UINT32 addr, UINT8 pkt_size)
{
	hc_WriteByte(addr, DMA_AUTO_OPTION, B_AUTO_RX_EN | (pkt_size<<4));
}

//offset 0x68 bit 4~0
static inline void enable_dma_auto_tx(UINT32 addr, UINT8 pkt_size)
{
	hc_WriteByte(addr, DMA_AUTO_OPTION, B_TX_ALIGN_CHK_EN | pkt_size);
}

//offset 0x68
static inline void enable_dma_auto_rxtx(UINT32 addr, UINT8 tx_pkt_size, UINT8 rx_pkt_size)
{
	hc_WriteByte(addr, DMA_AUTO_OPTION, B_AUTO_RX_EN | (rx_pkt_size<<4) | B_TX_ALIGN_CHK_EN | tx_pkt_size);
}

//offset 0x70
static inline void enable_3202_int_ctrl(UINT32 addr)
{
	hc_WriteByte(addr, INT_CTRL_3202, 0x01);
}

//offset 0x70~0x73  -->  The address need to be 32 bytes align
static inline void set_bulk_cmd_base_addr(UINT32 addr, UINT32 base_addr)
{
	hc_WriteDword(addr, BULK_CMD_BASE_ADDR, base_addr & 0x0FFFFFE0);
}

//offset 0x74~0x77  -->  It need to be 32 bytes align
static inline UINT32 read_bulk_cmd_index(UINT32 addr)
{
	return (hc_ReadDword(addr, BULK_CMD_INDEX)>>5);
}

//offset 0x74~0x77  -->  It need to be 32 bytes align
static inline void clear_bulk_cmd_index(UINT32 addr)
{
	hc_WriteDword(addr, BULK_CMD_INDEX, 0x00000000);
}

//offset 0x74~0x77  -->  It need to be 32 bytes align
static inline void set_bulk_cmd_index(UINT32 addr, UINT32 index_offset)
{
	hc_WriteDword(addr, BULK_CMD_INDEX, ((index_offset<<5) & 0x0FFFFFE0));
}

//offset 0x78~0x7B  -->  The address need to be 32 bytes align
static inline void set_bulk_status_base_addr(UINT32 addr, UINT32 base_addr)
{
	hc_WriteDword(addr, BULK_STS_BASE_ADDR, base_addr & 0x0FFFFFE0);
}

//offset 0x7C~0x7F  -->  It need to be 32 bytes align
static inline UINT32 read_bulk_status_index(UINT32 addr)
{
	return (hc_ReadDword(addr, BULK_STS_INDEX)>>5);
}

//offset 0x7C~0x7F  -->  It need to be 32 bytes align
static inline void clear_bulk_status_index(UINT32 addr)
{
	hc_WriteDword(addr, BULK_STS_INDEX, 0x00000000);
}

//offset 0x7C~0x7F  -->  It need to be 32 bytes align
static inline void set_bulk_status_index(UINT32 addr, UINT32 index_offset)
{
	hc_WriteDword(addr, BULK_STS_INDEX, ((index_offset<<5) & 0x0FFFFFE0));
}


//offset 0x2F
static inline UINT8 get_host_int_flag3(UINT32 addr)
{
	return hc_ReadByte(addr, HINTFLR3);
}

//offset 0x2F
static inline void en_host_int_flag3(UINT32 addr, UINT8 type)
{
	hc_WriteByte(addr, HINTFLR3, type);
}

//offset 0x80,0x82,0x84,0x86,0x88
static inline void set_ep_device_address(UINT32 addr,UINT8 ep,UINT8 ep_dev_num,UINT8 sscs_DisableOrEnable)
{
	UINT8 temp;
	
	temp =(ep_dev_num&0xF)<<1;
	temp |= (sscs_DisableOrEnable&0x1) ;
	hc_WriteByte(addr, ep, (temp&0x1F));
	
}
//offset 0x81,0x83,0x85,0x87,0x89
static inline void get_ssplit_ctrl_info(UINT32 addr,UINT8 ep, UINT8 *tt_speed, UINT8 *hub_port, UINT8 *hub_addr)
{
   UINT8 temp;
  	temp = hc_ReadByte(addr, ep);

	*tt_speed= (temp & 0x40)>>6 ;//0: full speed; 1: low speed
	*hub_port=(temp & 0x38)>>3;
	*hub_addr=temp & 0x07;
}

//offset 0x81,0x83,0x85,0x87,0x89
static inline void set_ssplit_ctrl_info(UINT32 hc_addr,UINT8 ep, UINT8 speed, UINT8 hub_port, UINT8 hub_addr)
{
   	UINT8 temp;;

	temp=((speed&0x1)<<6) |((hub_port&0x7)<<3)|(hub_addr&0x7);
	hc_WriteByte(hc_addr,ep,temp);	
}

//offset 0x8C
static inline void host_new_mode_set(UINT32 addr, UINT32 EnableOrDisable)
{
	  UINT8 Value=0;

	  if (EnableOrDisable == TRUE)
	  	Value = 0x80;
	  
//	   hc_WriteByte(addr, HUB_CTRL_REG,Value );
	   hc_WriteByte(addr, HUB_FUNC_CTRL_REG,Value );
}

//offset 0x90
static inline void set_plug_status(UINT32 addr,UINT32 ctrl)
{
	hc_WriteByte(addr, HUB_HOTPLUG_CTRL_REG, ctrl|0x01);	//Bit 0: Reset the NYET register, never ping dev after NYET, OUT packet may lost bandwidth
}

/*
//offset 0x8c
static inline void mask_ep0_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) &(~ HUB_EP0_SS_CS_IE)));
}

//offset 0x8c
static inline void mask_epA_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) &(~ HUB_EPA_SS_CS_IE)));
}

//offset 0x8c
static inline void mask_epB_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) &(~ HUB_EPB_SS_CS_IE)));
}
//offset 0x8c
static inline void mask_epC_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) &(~ HUB_EPC_SS_CS_IE)));
}

//offset 0x8c
static inline void mask_epD_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) &(~ HUB_EPD_SS_CS_IE)));
}

//offset 0x8c
static inline void en_ep0_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) | HUB_EP0_SS_CS_IE));
}
//offset 0x8c
static inline void en_epA_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) | HUB_EPA_SS_CS_IE));
}
//offset 0x8c
static inline void en_epB_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) | HUB_EPB_SS_CS_IE));
}
//offset 0x8c
static inline void en_epC_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) | HUB_EPC_SS_CS_IE));
}
//offset 0x8c
static inline void en_epD_ss_function(UINT32 addr)
{
	hc_WriteByte(addr, HUB_CTRL_REG, (hc_ReadByte(addr, HUB_CTRL_REG) | HUB_EPD_SS_CS_IE));
}
*/

#endif // __HC_3602_REG_H__

// end of file
