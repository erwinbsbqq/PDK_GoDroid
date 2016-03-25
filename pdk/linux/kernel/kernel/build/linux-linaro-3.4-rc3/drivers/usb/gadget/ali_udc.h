/*
 * ALi USB Device Controller Driver ALi UDC
 *
 * linux/drivers/usb/gadget/alidev_udc.h 
 *
 * Copyright (C) 2007-2014 Alitech Corp.
 *
 * Author : David Shih <david.shih@alitech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */
 
#ifndef _ALIDEV_UDC_H
#define _ALIDEV_UDC_H
 
typedef volatile unsigned char      VUINT8;
typedef volatile unsigned short     VUINT16;
typedef volatile unsigned long      VUINT32;
typedef volatile unsigned long long VUINT64;

/* C3921 provide 5 bulk in/out ep, others only 1 */
#define SUPPORT_C3921
#define SUPPORT_ANDROID_UDC
/* debug message */
//#define CONFIG_DEBUG_USB_ALI_DEV

#define DRIVER_DESC	    "ALI USB Device Controller Gadget"
#define DRIVER_VERSION	"17 JUL 2014"
#define DRIVER_AUTHOR	  "David Shih"

#define H2D 	1 		//use EPB, bulk out
#define D2H		0 		//use EPA, bulk in

#define C39_DEV_RESET 0x1803d810
#define USB_DEV_RESET	(1<<2)

#define EP0_SETUP                   0x04
#define IN_DIR                      0x02

//=========== DEV====================
/* USB Device Mode Operation Registers (Offset)*/
#define USBD_DEVSETR              (VUINT8) (0x00)   // Device Setting Register (00h)
#define USBD_ON          (1<<7)   // Bit[7] USB Turn On, 0:off, 1:On
#define DET_FULL_SPD     (1<<6)   // Bit[6] Detected USB Speed, 0:High Speed, 1:Full Speed 
#define EXP_FULL_SPD     (1<<5)   // Bit[5] Expected USB Speed, 0:High Speed, 1:Full Speed 
#define RMT_WKUP_FEAT    (1<<4)   // Bit[4] Remote Wakeup Fature, 
                                  // 0: device disabled to request remote wakeup, 
                                  // 1: device enabled to request remote wakeup,  
#define SUSPEND_STS      (1<<3)   // Bit[3] Suspend Status, 0:Normal operation, 1:In the suspend state.
#define VBUS_STS         (1<<2)   // Bit[2] VBUS Status, 0:Not plug into the host. 1:Plug into the host.
#define RMT_WKUP_SUPP    (1<<1)   // Bit[1] Remote Wakeup Support, 
                                  // 0:Do not support the remote wakeup,
                                  // 1:Support the remote wakeup.
#define SELF_PWDED       (1<<0)   // Bit[0] Detected USB Speed, 0:High Speed, 1:Full Speed 

/* Endpoint Control/Setting/Data Port Registers */
#define USBD_EP0_FCTR             (VUINT8) (0x01)   // Endpoint 0 FIFO Control Register   (01h)
#define USBD_EP0_FIFO             (VUINT8) (0x02)   // Endpoint 0 FIFO Data Port Register (02h)

#define USBD_EPA_SETR             (VUINT8) (0x03)   // Endpoint A FIFO Setting Register   (03h)
#define USBD_EPA_FCTR             (VUINT8) (0x04)   // Endpoint A FIFO Control Register   (04h)
#define USBD_EPA_FIFO             (VUINT8) (0x05)   // Endpoint A FIFO Data Port Register (05h)
#define USBD_EPB_SETR             (VUINT8) (0x06)   // Endpoint B FIFO Setting Register   (06h)
#define USBD_EPB_FCTR             (VUINT8) (0x07)   // Endpoint B FIFO Control Register   (07h)
#define USBD_EPB_FIFO             (VUINT8) (0x08)   // Endpoint B FIFO Data Port Register (08h)
#define USBD_EPC_SETR             (VUINT8) (0x09)   // Endpoint C FIFO Setting Register   (09h)
#define USBD_EPC_FCTR             (VUINT8) (0x0A)   // Endpoint C FIFO Control Register   (0Ah)
#define USBD_EPC_FIFO             (VUINT8) (0x0B)   // Endpoint C FIFO Data Port Register (0Bh)

#define USBD_EPD_SETR             (VUINT8) (0x23)   // Endpoint D FIFO Setting Register   (23h)
#define USBD_EPD_FCTR             (VUINT8) (0x24)   // Endpoint D FIFO Control Register   (24h)
#define USBD_EPD_FIFO             (VUINT8) (0x25)   // Endpoint D FIFO Data Port Register (25h)
#define USBD_EPE_SETR             (VUINT8) (0x26)   // Endpoint E FIFO Setting Register   (26h)
#define USBD_EPE_FCTR             (VUINT8) (0x27)   // Endpoint E FIFO Control Register   (27h)
#define USBD_EPE_FIFO             (VUINT8) (0x28)   // Endpoint E FIFO Data Port Register (28h)
#define USBD_EPF_SETR             (VUINT8) (0x29)   // Endpoint F FIFO Setting Register   (29h)
#define USBD_EPF_FCTR             (VUINT8) (0x2A)   // Endpoint F FIFO Control Register   (2Ah)
#define USBD_EPF_FIFO             (VUINT8) (0x2B)   // Endpoint F FIFO Data Port Register (2Bh)

#define USBD_EPG_SETR             (VUINT8) (0xC3)   // Endpoint G FIFO Setting Register   (C3h)
#define USBD_EPG_FCTR             (VUINT8) (0xC4)   // Endpoint G FIFO Control Register   (C4h)
#define USBD_EPG_FIFO             (VUINT8) (0xC5)   // Endpoint G FIFO Data Port Register (C5h)
#define USBD_EPH_SETR             (VUINT8) (0xC6)   // Endpoint H FIFO Setting Register   (C6h)
#define USBD_EPH_FCTR             (VUINT8) (0xC7)   // Endpoint H FIFO Control Register   (C7h)
#define USBD_EPH_FIFO             (VUINT8) (0xC8)   // Endpoint H FIFO Data Port Register (C8h)

#define USBD_EPI_SETR             (VUINT8) (0xD3)   // Endpoint I FIFO Setting Register   (D3h)
#define USBD_EPI_FCTR             (VUINT8) (0xD4)   // Endpoint I FIFO Control Register   (D4h)
#define USBD_EPI_FIFO             (VUINT8) (0xD5)   // Endpoint I FIFO Data Port Register (D5h)
#define USBD_EPJ_SETR             (VUINT8) (0xD6)   // Endpoint J FIFO Setting Register   (D6h)
#define USBD_EPJ_FCTR             (VUINT8) (0xD7)   // Endpoint J FIFO Control Register   (D7h)
#define USBD_EPJ_FIFO             (VUINT8) (0xD8)   // Endpoint J FIFO Data Port Register (D8h)

#define USBD_EPK_SETR             (VUINT8) (0xE3)   // Endpoint K FIFO Setting Register   (E3h)
#define USBD_EPK_FCTR             (VUINT8) (0xE4)   // Endpoint K FIFO Control Register   (E4h)
#define USBD_EPK_FIFO             (VUINT8) (0xE5)   // Endpoint K FIFO Data Port Register (E5h)
#define USBD_EPL_SETR             (VUINT8) (0xE6)   // Endpoint L FIFO Setting Register   (E6h)
#define USBD_EPL_FCTR             (VUINT8) (0xE7)   // Endpoint L FIFO Control Register   (E7h)
#define USBD_EPL_FIFO             (VUINT8) (0xE8)   // Endpoint L FIFO Data Port Register (E8h)

/* Endpoint FIFO Control Register Bit field */
#define EP_STS_EMPTY    (1<<7)    // Bit[7] EP FIFO Empty 0:Not empty 1:At least one bank of FIFO is empty
#define EP_STS_FULL     (1<<6)    // Bit[6] EP FIFO Full 0:Non-Full 1:Full
#define EP_FORCE        (1<<5)    // Bit[5] EP FIFO Force, Write 1 to force Endpoint FIFO to send the current bank data.
                                  // This bit will be auto-cleared when the transmission is finished.
#define EP_INIT         (1<<4)    // Bit[4] EP FIFO Initialization 0:Normal operation 1:Initialize
#define EP_ALL_EMPTY    (1<<2)    // Bit[2] EP FIFO All Banks are Empty. 0:Non-empty 1:Empty
#define EP_STALL        (1<<0)    // Bit[0] EP Stall 0:Normal operation 1: Stall

/* Endpoint Setting Register Bit field */
#define EP_ISO          (1<<6)    // Bit[6:7] Endpoint Type, 1:ISO, 2:Bulk 3:INT
#define EP_BULK         (2<<6)
#define EP_INT          (3<<6)
#define EP_NUM(n)       ((n&0x07)<<3|(n&0x08)>>3) //Bit[5:3] EP Num bit[2:0], Bit[0] EP Num bit[3]

#define EP0              0        // Control EP
#define EPA              1        // Bulk In EP (D2H)
#define EPB              2        // Bulk Out EP (H2D)
#define EPC              3        // Interrupt In EP 
#define EPD              4        // Bulk In EP
#define EPE              5        // Bulk Out EP
#define EPF              6        // Interrupt EP
#define EPG              7        // Bulk In EP
#define EPH              8        // Bulk Out EP
#define EPI              9        // Bulk In EP
#define EPJ              10       // Bulk Out EP
#define EPK              11       // Bulk In EP
#define EPL              12       // Bulk Out EP

#define USBD_RESCTL               (VUINT8) (0x0D)   // PHY Reset Control Register (0Dh)
#define USBD_PWRCTL               (VUINT8) (0x0E)   // Power Control Register     (0Eh)
#define USBD_PHYTEST              (VUINT8) (0x0F)   // PHY Test Register          (0Fh)
#define USBD_PHYTEST1             (VUINT8) (0x10)   // PHY Test Register 1        (10h)
#define USBD_PHYANATEST           (VUINT8) (0x11)   // PHY Alanlog Test In/Out    (11h)
#define USBD_PHYVER               (VUINT8) (0x12)   // PHY Version                (12h)
#define USBD_PHYCLKCTL            (VUINT8) (0x13)   // PHY Gated Clock Control    (13h)
#define USBD_PHYCLKVAR            (VUINT8) (0x14)   // PHY Gated Clock Value      (14h)

/* Interrupt Enable & Status Register */
#define USBD_INTENR               (VUINT8) (0x15)   // USB 2.0 Interrupt Enable Register (15h)
#define USBD_INTFLAG              (VUINT8) (0x17)   // USB 2.0 Interrupt Flag Register   (17h)
#define RESET_INT        (1<<7)   // Bit[7] USB Reset Interrupt
#define SUS_RES_INT      (1<<6)   // Bit[6] Suspend/Resume Interrupt
#define VBUS_INT         (1<<5)   // Bit[5] VBUS Interrupt
#define EPC_TX_INT       (1<<4)   // Bit[4] EPC Transmit Done Interrupt
#define EPB_RX_INT       (1<<3)   // Bit[3] EPB Receive Done Interrupt
#define EPA_TX_INT       (1<<2)   // Bit[2] EPA Transmit Done Interrupt
#define EP0_RX_INT       (1<<1)   // Bit[1] EP0 Receive Done Interrupt
#define EP0_TX_INT       (1<<0)   // Bit[0] EP0 Transmit Done Interrupt


#define USBD_INTENR1              (VUINT8) (0x16)   // USB 2.0 Interrupt Enable Register 1 (16h)
#define USBD_INTFLAG1             (VUINT8) (0x18)   // USB 2.0 Interrupt Flag Register 1  (18h)
#define EPF_TX_INT       (1<<4)   // Bit[4] EPF Transmit Done Interrupt
#define EPE_RX_INT       (1<<3)   // Bit[3] EPE Receive Done Interrupt
#define EPD_TX_INT       (1<<2)   // Bit[2] EPD Transmit Done Interrupt
#define DMA_IE2_EN       (1<<1)   // Bit[1] DMA finish EPD and EPE
#define DMA_IE1_EN       (1<<0)   // Bit[0] DMA finish EPA and EPB

#define USBD_INTENR2              (VUINT8) (0x30)   // USB 2.0 Interrupt Enable Register 2 (30h)
#define USBD_INTFLAG2             (VUINT8) (0x32)   // USB 2.0 Interrupt Flag Register 2  (32h)
#define EPJ_RX_INT       (1<<6)   // Bit[6] EPJ Receive Done Interrupt
#define EPI_TX_INT       (1<<5)   // Bit[5] EPI Transmit Done Interrupt
#define EPH_RX_INT       (1<<3)   // Bit[3] EPH Receive Done Interrupt
#define EPG_TX_INT       (1<<2)   // Bit[2] EPG Transmit Done Interrupt
#define DMA_IE4_EN       (1<<1)   // Bit[1] DMA finish EPI and EPJ
#define DMA_IE3_EN       (1<<0)   // Bit[0] DMA finish EPG and EPH

#define USBD_INTENR3              (VUINT8) (0x31)   // USB 2.0 Interrupt Enable Register 3 (31h)
#define USBD_INTFLAG3             (VUINT8) (0x33)   // USB 2.0 Interrupt Flag Register 3  (33h)
#define EPL_RX_INT       (1<<3)   // Bit[3] EPL Receive Done Interrupt
#define EPK_TX_INT       (1<<2)   // Bit[2] EPK Transmit Done Interrupt
#define DMA_IE5_EN       (1<<0)   // Bit[0] DMA finish EPK and EPL

/* DMA Control Register */
#define USBD_DMACTL               (VUINT8) (0x19)   // DMA Control Register   (19h)
#define USBD_DMA1CTL              (VUINT8) (0x2C)   // DMA Control Register 1 (2Ch)
#define USBD_DMA2CTL              (VUINT8) (0xCC)   // DMA Control Register 2 (CCh)
#define USBD_DMA3CTL              (VUINT8) (0xDC)   // DMA Control Register 3 (DCh)
#define USBD_DMA4CTL              (VUINT8) (0xEC)   // DMA Control Register 4 (ECh)
#define DMA_INIT         (1<<7)   // Bit[7] DMA Initialize 0:Normal Operation 1:Initialize
#define DMA_EN           (1<<0)   // Bit[0] DMA Transfer Enable 0:Disable 1:Enable

#define USBD_DMACNTL              (VUINT8) (0x1A)   // DMA Counter Low Byte Register      (1Ah)
#define USBD_DMACNTM              (VUINT8) (0x1B)   // DMA Counter Middle Byte Register   (1Bh)
#define USBD_DMACNTH              (VUINT8) (0x1C)   // DMA Counter High Byte Register     (1Ch)
                                                    
#define USBD_DMA1CNTL             (VUINT8) (0x2D)   // DMA Counter Low Byte Register 1    (2Dh)
#define USBD_DMA1CNTM             (VUINT8) (0x2E)   // DMA Counter Middle Byte Register 1 (2Eh)
#define USBD_DMA1CNTH             (VUINT8) (0x2F)   // DMA Counter High Byte Register 1   (2Fh) 
                                                    
#define USBD_DMA2CNTL             (VUINT8) (0xCD)   // DMA Counter Low Byte Register 2    (CDh)
#define USBD_DMA2CNTM             (VUINT8) (0xCE)   // DMA Counter Middle Byte Register 2 (CEh)
#define USBD_DMA2CNTH             (VUINT8) (0xCF)   // DMA Counter High Byte Register 2   (CFh)
                                                    
#define USBD_DMA3CNTL             (VUINT8) (0xDD)   // DMA Counter Low Byte Register 3    (DDh)
#define USBD_DMA3CNTM             (VUINT8) (0xDE)   // DMA Counter Middle Byte Register 3 (DEh)
#define USBD_DMA3CNTH             (VUINT8) (0xDF)   // DMA Counter High Byte Register 3   (DFh)
                                                    
#define USBD_DMA4CNTL             (VUINT8) (0xED)   // DMA Counter Low Byte Register 4    (EDh)
#define USBD_DMA4CNTM             (VUINT8) (0xEE)   // DMA Counter Middle Byte Register 4 (EEh)
#define USBD_DMA4CNTH             (VUINT8) (0xEF)   // DMA Counter High Byte Register 4   (EFh)
              
#define USB_CHIPVER               (VUINT8) (0x1F)   // USB IP Version Register            (1Fh)
                                      
#define USBD_EPSIZE               (VUINT8) (0x1D)   // Endpoint A, B Size Control Register (1Dh)
#define USBD_MISCCTL              (VUINT8) (0x1E)   // Misc. Control Register              (1Eh)
#define USB_CFG          (1<<4)   // Bit[4] USB Device Configuration Status 0:Un-configured, 1:Configured
#define EP_DMA_AUTO_FRC  (1<<3)   // Bit[3] Endpoint A DMA Auto Force Enable 0:Disable, 1:Enable

#define USBD_EPSIZE1              (VUINT8) (0x21)   // Endpoint D, E Size Control Register (21h)
#define USBD_MISCCTL1             (VUINT8) (0x22)   // Misc. Control Register 1            (22h)
#define USBD_EPSIZE2              (VUINT8) (0xC1)   // Endpoint G, H Size Control Register (C1h)
#define USBD_MISCCTL2             (VUINT8) (0xC2)   // Misc. Control Register 2            (C2h)
#define USBD_EPSIZE3              (VUINT8) (0xD1)   // Endpoint I, J Size Control Register (D1h)
#define USBD_MISCCTL3             (VUINT8) (0xD2)   // Misc. Control Register 3            (D2h)
#define USBD_EPSIZE4              (VUINT8) (0xE1)   // Endpoint K, L Size Control Register (E1h)
#define USBD_MISCCTL4             (VUINT8) (0xE2)   // Misc. Control Register 4            (E2h)

//========== USB_INFT ==================
#define USBD_INTF_DMA_BASE_ADDR   (VUINT32) (0x40)  //40-43 DMA Base Address (DWORD Align)
#define USBD_INTF_DMA_BASE_LEN    (VUINT32) (0x44)  //44-47

#define USBD_INTF_DMA_1_BASE_ADDR (VUINT32) (0x50)  //50-53
#define USBD_INTF_DMA_1_BASE_LEN  (VUINT32) (0x54)  //54-57

#define USBD_INTF_DMA_2_BASE_ADDR (VUINT32) (0x60)  //60-63
#define USBD_INTF_DMA_2_BASE_LEN  (VUINT32) (0x64)  //64-67

#define USBD_INTF_DMA_3_BASE_ADDR (VUINT32) (0x70)  //70-73
#define USBD_INTF_DMA_3_BASE_LEN  (VUINT32) (0x74)  //74-77

#define USBD_INTF_DMA_4_BASE_ADDR (VUINT32) (0x80)  //80-83
#define USBD_INTF_DMA_4_BASE_LEN  (VUINT32) (0x84)  //84-87

#define USBD_INTF_DMA_CTL         (VUINT8)  (0x48)  // DMA Control Register    
#define USBD_INTF_DMA_1_CTL       (VUINT8)  (0x58)  
#define USBD_INTF_DMA_2_CTL       (VUINT8)  (0x68)  
#define USBD_INTF_DMA_3_CTL       (VUINT8)  (0x78)  
#define USBD_INTF_DMA_4_CTL       (VUINT8)  (0x88)  
#define INTF_DMA_INIT    (1<<7)   // Bit[7] DMA INIT 1:Clear DMA Controller status 0:Normal operation
#define INTF_DMA_GOING   (1<<6)   // Bit[6] DMA PROC 1:DMA controller is processing 0:DMA is idle
#define INTF_DMA_STOP    (1<<1)   // Bit[1] DMA STOP 1:Stop current DMA process
#define INTF_DMA_EN      (1<<0)   // Bit[0] DMA START Enable DMA to transfer data from/to USB_IP (Auto clear to 0)

#define USBD_INTF_DMA_CTL2        (VUINT8)  (0x49)
#define USBD_INTF_DMA_1_CTL2      (VUINT8)  (0x59)
#define USBD_INTF_DMA_2_CTL2      (VUINT8)  (0x69)
#define USBD_INTF_DMA_3_CTL2      (VUINT8)  (0x79)
#define USBD_INTF_DMA_4_CTL2      (VUINT8)  (0x89)

#define DMA_CMD_TX       0x01     // D2H, dram to USBIP
#define DMA_CMD_RX       0x00     // H2D, USBIP to DRAM

struct alidev_ep {  
    char                    name[14];
	struct list_head		queue;
	unsigned long			last_io;	        /* jiffies timestamp */
	struct usb_gadget		*gadget;
	struct alidev_udc		*dev;
	const struct usb_endpoint_descriptor *desc;
	struct usb_ep			ep;
	u8				        num;

	unsigned short			fifo_size;    
	u8				        bEndpointAddress;
	u8				        bmAttributes;

	unsigned			    halted : 1;
	unsigned			    already_seen : 1;
	unsigned			    setup_stage : 1;
};

#ifdef SUPPORT_C3921
	#define ALIDEV_ENDPOINTS        13
	#define DMA_BUF_NUM             5
#else
	#define ALIDEV_ENDPOINTS        4
	#define DMA_BUF_NUM             1
#endif

bool inline ep_is_ep0(struct usb_ep *ep)
{
    return (strcmp(ep->name,"ep0") == 0) ? true : false;            
}

/* Warning : ep0 has a fifo of 16 bytes */
/* Don't try to set 32 or 64            */
/* also testusb 14 fails  with 16 but is */
/* fine with 8                          */
#define EP0_FIFO_SIZE		    64
#define EP_FIFO_SIZE		    512
#define DEFAULT_POWER_STATE	    0x00

#define ALI_EP_FIFO_SIZE	    512
#define ALI_EPC_FIFO_SIZE		8

struct alidev_request {
	struct list_head		queue;		/* ep's requests */
	struct usb_request		req;
};

enum ep0_state {
        EP0_IDLE,
        EP0_IN_DATA_PHASE,
        EP0_OUT_DATA_PHASE,
        EP0_END_XFER,
        EP0_STALL,
};

#define DMA_MEM_SIZE    (64*1024)


struct dma_config
{
	u8			*buf;
	dma_addr_t	addr;	    
	//u8          control_reg;
	//u8          intf_control_reg;
	//u8          counter_low_reg;
	//u8          counter_midde_reg;
	//u8          counter_high_reg;		
};


struct alidev_udc {
	spinlock_t			  lock;
	void __iomem	    *base_addr;
	int								usb_5v_control_gpio;
	int               usb_5v_detect_gpio;	
	int               irq;
	
	struct alidev_ep	ep[ALIDEV_ENDPOINTS];
	
	struct usb_gadget	gadget;
	struct usb_gadget_driver	*driver;

	u16				    devstatus;
	int				    ep0state;

	unsigned			req_std : 1;
	unsigned			req_config : 1;
	unsigned			req_pending : 1;
	u8				    vbus;

	u8 usb_intf_isr;
	//u8 usb_dma_int;
	u8 usb_dev_int;
	u8 usb_auto_setconfig;
	//u8 usb_rst_action;
	//u8 usb_thread_run;
	//int thread_wakeup_needed;
	u8 dev_ep0_rx;
	//u8 dev_ep0_tx;	
	//u8 dev_dma_tx;
	//u8 dev_dma_rx;	
	//struct task_struct	*thread_task;	
	//struct completion	thread_notifier;

	struct mutex		    mutex;	
	spinlock_t		        lock1;	
	struct timer_list	    timer;	/* P: lock */	
  struct dma_config     dma[DMA_BUF_NUM];
  
	struct completion dma_epab_completion;
#ifdef 	SUPPORT_C3921
	struct completion dma_epde_completion;
	struct completion dma_epgh_completion;
	struct completion dma_epij_completion;
	struct completion dma_epkl_completion;	
#endif	
};

static inline u32 udc_read(struct alidev_udc *udc, u32 reg)  
{ 
    return readb(udc->base_addr + reg); 
}
static inline u32 udc_read32(struct alidev_udc *udc, u32 reg)
{
    return readl(udc->base_addr + reg);
}
static inline void udc_write(struct alidev_udc *udc, u32 value, u32 reg)
{
	writeb(value, udc->base_addr + reg);
}
static inline void udc_write32(struct alidev_udc *udc, u32 value, u32 reg)
{
	writel(value, udc->base_addr + reg);
}

static inline struct alidev_ep *to_alidev_ep(struct usb_ep *ep)
{
	return container_of(ep, struct alidev_ep, ep);
}

static inline struct alidev_udc *to_alidev_udc(struct usb_gadget *gadget)
{
	return container_of(gadget, struct alidev_udc, gadget);
}

static inline struct alidev_request *to_alidev_req(struct usb_request *req)
{
	return container_of(req, struct alidev_request, req);
}

/*
#define gadget_to_alidev_udc(_gadget)	\
	container_of(_gadget, struct alidev_udc, gadget)
*/	
		
#define gadget_to_alidev_udc(_gadget)	\
		container_of(_gadget, struct alidev_udc, gadget)
#endif
