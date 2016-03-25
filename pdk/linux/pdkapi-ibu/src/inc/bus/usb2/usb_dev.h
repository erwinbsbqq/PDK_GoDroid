#ifndef __USB_DEV_NEW_H__
#define __USB_DEV_NEW_H__
#include <basic_types.h>
#include <hld/hld_dev.h>
#include <bus/usb2/usb.h>

//for debug
#define DBG_FUN     do { \
                        printf("%s\n", __FUNCTION__);  \
                    } while (0)

#define DBG_LINE    do { \
                        printf("%s, %d\n", __FUNCTION__, __LINE__); \
                    } while (0)


#define DBG_NUM(a)  do { \
                        printf("%s, line %d: %d\n", __FUNCTION__, __LINE__, a); \
                    } while (0)

#define DBG_ADDR(a)  do { \
                        printf("%s, line %d: 0x%x\n", __FUNCTION__, __LINE__, a); \
                    } while (0)

#define DBG_STR(a)  do { \
                        printf("%s, line %d: %s\n", __FUNCTION__, __LINE__, a); \
                    } while (0)

enum usb_device_class
{
	USB_CLASS_UNKNOWN				= -1,
	USB_CLASS_PER_INTERFACE			= 0,
	/* for DeviceClass */
	USB_CLASS_AUDIO					= 1,
	USB_CLASS_COMM					= 2,
	USB_CLASS_HID					= 3,
	USB_CLASS_PHYSICAL				= 5,
	USB_CLASS_STILL_IMAGE			= 6,
	USB_CLASS_PRINTER				= 7,
	USB_CLASS_MASS_STORAGE			= 8,
	USB_CLASS_HUB					= 9,
	USB_CLASS_CDC_DATA				= 0x0a,
	USB_CLASS_CSCID					= 0x0b,
	/* chip+ smart card */
	USB_CLASS_CONTENT_SEC			= 0x0d,
	/* content security */
	USB_CLASS_VIDEO					= 0x0e,
	USB_CLASS_WIRELESS_CONTROLLER	= 0xe0,
	USB_CLASS_APP_SPEC				= 0xfe,
	USB_CLASS_VENDOR_SPEC			= 0xff
};

/*
 * USB HID interface subclass and protocol codes
 */
#define USB_INTERFACE_SUBCLASS_BOOT		1
#define USB_INTERFACE_PROTOCOL_KEYBOARD	1
#define USB_INTERFACE_PROTOCOL_MOUSE		2
//end

/*USB Communication interface class subclass codes*/
#define USB_INTERFACE_SUBCLASS_DLCM	1  /*Direct line control Model*/
#define USB_INTERFACE_SUBCLASS_ACM	2  /*Abstract Control Model*/
#define USB_INTERFACE_SUBCLASS_TCM	3 /*Telephone Control Model*/
#define USB_INTERFACE_SUBCLASS_MCCM	4 /*Multi Channel Control Model*/
#define USB_INTERFACE_SUBCLASS_CAPI	5 /*CAPI Control Model*/
#define USB_INTERFACE_SUBCLASS_ETHERNET	6 /*ethernet networkingControl Model*/
#define USB_INTERFACE_SUBCLASS_ATM	6 /*ATM networkingControl Model*/

#define USB_HUB_TRANSFER_MODE 1
#define USB_INTERRUPT_TRANSFER_MODE 2
#define USB_BULK_IN_TRANSFER_MODE 3
#define USB_BULK_OUT_TRANSFER_MODE 5
#define USB_CONTROL_TRANSFER_MODE 4
#define USB_OPT_BULK_TRANSFER_MODE 6

//#define USB_INTERNAL_DISK_PARSE

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK			0x1f
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03

#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

#ifndef __MM_USB_START_ADDR
#define __MM_USB_START_ADDR     0xa2100000
#endif
#define HC_TIMEOUT_FORVER			0xFFFFFFFF

#define USB_CTRL_SET_TIMEOUT	2000
#define USB_CTRL_GET_TIMEOUT	3000

//#define USING_SEMAPHORE
//#define USING_MUTEX

#define GET_C0_COUNT(val)       asm volatile("mfc0 %0, $9" :"=r"(val))
#define MS_INTERVAL_COUNT		(SYS_CPU_CLOCK/2000)

#define USBD_MBF_SIZE		100

#define USB_TX_MEM_SIZE		512*1024
#define USB_RX_MEM_SIZE		512*1024
#define USB_CMD_MEM_SIZE		32*1000
#define USB_STS_MEM_SIZE		13*1000

#define USB_TX_MEM_ADDR		((__MM_USB_START_ADDR-USB_TX_MEM_SIZE)&0XFFFFFFE0)
#define USB_RX_MEM_ADDR		((USB_TX_MEM_ADDR-USB_RX_MEM_SIZE)&0XFFFFFFE0)
#define USB_CMD_MEM_ADDR		((USB_RX_MEM_ADDR-USB_CMD_MEM_SIZE)&0XFFFFFFE0)
#define USB_STS_MEM_ADDR		((USB_CMD_MEM_ADDR-USB_STS_MEM_SIZE)&0XFFFFFFE0)

#define USB_MAX_DEVICES 	127

#define USBD_NODE_SIG		((UINT32) 0x12345678)

#define USBD_DYNA_ATTACH	0
#define USBD_DYNA_REMOVE	1

#define DEVICE_CONNECT  1
#define DEVICE_DISCONNECT  0

#define MAX_HUB_STATUS_LEN	((USB_MAX_DEVICES + 1 + 7) / 8)


#define USB_NODETYPE_NONE	0
#define USB_NODETYPE_HUB	1
#define USB_NODETYPE_DEVICE	2
#define USB_NODETYPE_INT	3


#define MASS_DEVICE_ATTACHED  0x80000000
#define MASS_DEVICE_DETTACHED 0

#define USB_DEVICE_TIMER_PERIOD  800

/* The structure is used to reduce the interrupt times of DMA mode For BULK-only transfer protocol */
struct dma_queue_buf
{
	UINT32 *cmd_buf;		//Base address with CBW block
	UINT32 *tx_buf;			//Base address with Tx data buffer
	UINT32 *rx_buf;			//Base address with Rx data buffer
	UINT32 *sts_buf;		//Base address with CSW block
	UINT32 index_offset;		//index offset to indicate the start tx/rx position. it need to be 4 bytes alignment
	BOOL opt_bulk_en;		//TURE: enable Opt. BULK Mode; FALSE: disable it
};


typedef struct usb_setup
{
	UINT8		bmRequestType;
	UINT8 		bRequest;
	UINT16		wValue;
	UINT16		wIndex;
	UINT16		wLength;
}USB_SETUP;

typedef enum
{
	DESC_TYPE_DEVICE = 1,
	DESC_TYPE_CONFIG,
	DESC_TYPE_STRING,
	DESC_TYPE_INTERFACE,
	DESC_TYPE_ENDPOINT,
	DESC_TYPE_DEV_QUALIFIER,
	DESC_TYPE_SPEED_CONFIG,
	DESC_TYPE_INTF_PWR,
	DESC_TYPE_OTG,
}DescriptorType_t;

typedef enum
{
	GET_STATUS = 0,
	CLEAR_FEATURE = 1,
	SET_FEATURE = 3,
	SET_ADDRESS = 5,
	GET_DESCRIPTOR = 6,
	SET_DESCRIPTOR = 7,
	GET_CONFIGURATION = 8,
	SET_CONFIGURATION = 9,
	GET_INTERFACE = 10,
	SET_INTERFACE = 11,
	SYNCH_FRAME = 12,
	GET_MAX_LUN = 0xFE,
}StandReqCode_t;

typedef enum
{
	PKTSIZE_8 = 0x00,
	PKTSIZE_16 = 0x01,
	PKTSIZE_32 = 0x02,
	PKTSIZE_64 = 0x03,
	PKTSIZE_128 = 0x04,
	PKTSIZE_256 = 0x05,
	PKTSIZE_512 = 0x06,
	PKTSIZE_1024 = 0x07,
}EP_MaxPacketSize;


/* The interface is used to communicate between USBD and HCD */
struct upt
{
	struct dma_queue_buf		opt_bulk;

	void		*buffer;					//base address of transfer buffer
	UINT32	length;					//required transfer length
	UINT32	actual_length;			//actual transfer length
	UINT32	result;					/*transfer result includes the cases of 
									HC_EVENT_SUBTYPE_UPT_SUCCESS, HC_EVENT_SUBTYPE_UPT_DEV_REMOVE,
									HC_EVENT_SUBTYPE_UPT_LENGTH_MIS, HC_EVENT_SUBTYPE_UPT_TIMEOUT, 
									HC_EVENT_SUBTYPE_UPT_STALL, HC_EVENT_SUBTYPE_UPT_NAK,
									HC_EVENT_SUBTYPE_UPT_DATATOGGLE_MIS and HC_EVENT_SUBTYPE_UPT_FATAL_ERR
									(PID check error, UTMI error, CRC error and un-expect error) */
	UINT32	timeout;					//Transaction Timeout
	struct usb_setup		*setup;		//Transaction for control pipe
	struct usb_device		*dev;			
	UINT32 pipe;						/* pipe information:
 									bit31~30(ep type): USB_CTRL, USB_ISO, USB_BULK, USB_INT
 									bit15~18(ep addr): 0~15
 									bit8~14(dev addr): 0~127
									bit7(dir): USB_DIR_IN, USB_DIR_OUT
									bit0~2(max pkt size):000b(8), 001b(16). . ., 111b(1024)*/
	UINT32 pipe1;					/* As the same as the pipe. It was only used when opt_bulk_en == TRUE
									to assign the other pipe such that IN pipe, otherwise don't care it*/
	void *hc_dev;					/* point to specific hc */
	UINT8  interval;					/* (modify) transfer interval	 * (INT/ISO) */
	UINT8 type;
};

enum hc_s3602_io_cmd
{
	HC_GET_PORT_SPEED = 1,
	HC_GET_PORT_CONNECT,
	HC_USB_IP_RESET,
	HC_GET_BULK_OPT_INDEX,
	HC_SET_NEW_OPERATION_MODE,
	HC_BUS_RESET,
};


typedef enum
{
	HC_NO_EVENT_ASSIGN = 0,		/* No Event to be Notified */
	HC_BUS_EVENT_TYPE,			/* Bus Event Type to be Notified */
	HC_UPT_EVENT_TYPE,			/* UPT Event Type to be Notified */
} HcEventType_t;

typedef enum
{
	/* NOTATTACHED isn't in the USB spec, and this state acts
	 * the same as ATTACHED ... but it's clearer this way.
	 */
	USB_STATE_NOTATTACHED = 0,

	/* chapter 9 and authentication (wireless) device states */
	USB_STATE_ATTACHED,
	USB_STATE_POWERED,			/* wired */
	USB_STATE_DEFAULT,			/* limited function */
	USB_STATE_ADDRESS,
	USB_STATE_CONFIGURED,			/* most functions */

	USB_STATE_SUSPENDED,
	USB_STATE_READY,
	/* NOTE:  there are actually four different SUSPENDED
	 * states, returning to POWERED, DEFAULT, ADDRESS, or
	 * CONFIGURED respectively when SOF tokens flow again.
	 */
}UsbDevState_t;
typedef enum
{
     USBD_UNLOCKED=0,
     USBD_LOCKED,
     USBD_BAD,
}UsbdLockedState_t;
typedef enum
{
	HC_EVENT_SUBTYPE_UNKNOWN = 0,
	HC_EVENT_SUBTYPE_DEV_ATTACHED,			/*Device is attached*/
	HC_EVENT_SUBTYPE_DEV_DETACHED,			/*Device is detached*/
	//HC_EVENT_SUBTYPE_BUS_OVERCURRENT,		/*bus over current*/
	HC_EVENT_SUBTYPE_BUS_DEV_NO_RESP,		/*bus no device response*/
	HC_EVENT_SUBTYPE_BUS_FATAL_ERR,			/*bus babble error*/
	HC_EVENT_SUBTYPE_BUS_RST_FINISH,		/*bus reset finish*/
	HC_EVENT_SUBTYPE_BUS_SUSREM_FINISH,	/*bus suspend/resume finish*/
	HC_EVENT_SUBTYPE_BUS_TIMEOUT,			/*bus time out*/
	HC_EVENT_SUBTYPE_UPT_SUCCESS,			/*UPT transfer success*/
	HC_EVENT_SUBTYPE_UPT_DEV_REMOVE,		/*device be removed*/
	HC_EVENT_SUBTYPE_UPT_LENGTH_MIS,		/*UPT transfer data length mismatch*/
	HC_EVENT_SUBTYPE_UPT_TIMEOUT,			/*UPT transfer timeout*/
	HC_EVENT_SUBTYPE_UPT_STALL,				/*UPT occur STALL*/
	HC_EVENT_SUBTYPE_UPT_NAK,				/*Device is not responding*/
	HC_EVENT_SUBTYPE_UPT_DATATOG_MIS,		/*Data toggle mismatch*/
	HC_EVENT_SUBTYPE_UPT_FATAL_ERR,			/* UPT fatal error such as PID check failure, un expected PID, UTMI error and CRC error */
	HC_EVENT_SUBTYPE_DEV_DETACHED_AUTO,
	HC_EVENT_DYNAMIC_DEV_ATTACHED,			/*Device is attached*/
	HC_EVENT_DYNAMIC_DEV_DETACHED,			/*Device is detached*/
	HC_EVENT_DYNAMIC_UPT_DEV_REMOVE,		/*device be removed*/
} HcEventSubType_t;

typedef struct
{
	HcEventType_t event_type;
	HcEventSubType_t event_sub_type;
	UINT32 notify_param;
	void *nodeHandle;
	
}HCEvtMsg_t, *pHCEvtMsg_t;

#define USB_STATUS_UPDATE	1	/* update a hub */
#define USB_CLIENT_ATTACH	2	/* update a hub */
#define USB_CLIENT_DEATTACH	4	/* update a hub */

/* the callback function to notify the USBD what event is occurred from HCD */
typedef void (* HC_NOTIFY_CB)(HCEvtMsg_t, UINT32,UINT32);
typedef void (* BLK_NOTIFY_CB)(UINT32);

/* USB_DT_DEVICE: Device descriptor */
struct usb_device_descriptor
{
	UINT8  bLength;
	UINT8  bDescriptorType;

	UINT16 bcdUSB;
	UINT8  bDeviceClass;
	UINT8  bDeviceSubClass;
	UINT8  bDeviceProtocol;
	UINT8  bMaxPacketSize0;
	UINT16 idVendor;
	UINT16 idProduct;
	UINT16 bcdDevice;
	UINT8  iManufacturer;
	UINT8  iProduct;
	UINT8  iSerialNumber;
	UINT8  bNumConfigurations;

}__attribute__((aligned(32)));
#define USB_DT_DEVICE_SIZE		18
/*-------------------------------------------------------------------------*/

/* USB_DT_CONFIG: Configuration descriptor information.
 *
 * USB_DT_OTHER_SPEED_CONFIG is the same descriptor, except that the
 * descriptor type is different.  Highspeed-capable devices can look
 * different depending on what speed they're currently running.  Only
 * devices with a USB_DT_DEVICE_QUALIFIER have any OTHER_SPEED_CONFIG
 * descriptors.
 */
struct usb_config_descriptor
{
	UINT8  bLength;	
	UINT8  bDescriptorType;

	UINT16 wTotalLength;
	UINT8  bNumInterfaces;
	UINT8  bConfigurationValue;
	UINT8  iConfiguration;
	UINT8  bmAttributes;
	UINT8  bMaxPower;

	struct usb_interface_descriptor *interface; //add by victor
}__attribute__((aligned(32)));
#define USB_DT_CONFIG_SIZE		9
/* USB_DT_INTERFACE: Interface descriptor */
struct usb_interface_descriptor
{
	UINT8  bLength;
	UINT8  bDescriptorType;

	UINT8  bInterfaceNumber;
	UINT8  bAlternateSetting;
	UINT8  bNumEndpoints;
	UINT8  bInterfaceClass;
	UINT8  bInterfaceSubClass;
	UINT8  bInterfaceProtocol;
	UINT8  iInterface;
	
	struct usb_endpoint_descriptor *endpoint; //add by victor
} __attribute__((aligned(32)));

#define USB_DT_INTERFACE_SIZE		9


/* USB_DT_ENDPOINT: Endpoint descriptor */
struct usb_endpoint_descriptor
{
	UINT8  bLength;
	UINT8  bDescriptorType;

	UINT8  bEndpointAddress;
	UINT8  bmAttributes;
	UINT16 wMaxPacketSize;
	UINT8  bInterval;
	UINT8  temp;
} __attribute__((aligned(32)));

#define USB_DT_ENDPOINT_SIZE		7

struct usb_hc_device
{
	INT32 type; 						//HLD_DEV_TYPE_USB_HOST
	INT8 name[HLD_MAX_NAME_SIZE];	//Device Name
	UINT32 base_addr;				//USB IP I/O Base Address
	UINT32 rh_port_num;				//Root Hub Port Number, Default is one
	//void *priv;	// Used to be private for USB Host IP
	void *priv_bulk;
	void *priv_bulk_out;
	void *priv_opt_bulk;
	void *priv_int_c;
	void *priv_int_d;
	void *priv_ctrl;
	
	//struct upt *curr_upt;				// current working upt 
	struct upt *curr_upt_ctl;				// current working upt 
	struct upt *curr_upt_bulk;				// current working upt 
	struct upt *curr_upt_bulk_out;	
	struct upt *curr_upt_opt_bulk;	
	struct upt *curr_upt_int_c;				// current working upt 
	struct upt *curr_upt_int_d;				// current working upt 

	RET_CODE (*init)(HC_NOTIFY_CB);
	RET_CODE (*open)(struct usb_hc_device *);
	RET_CODE (*close)(struct usb_hc_device *);
	
	RET_CODE (*submit_upt)(struct upt *);
	RET_CODE (*cancel_upt)(struct upt *);
	RET_CODE (*suspend)(struct usb_hc_device *);
	RET_CODE (*resume)(struct usb_hc_device *);
	RET_CODE (*reset)(struct usb_hc_device *);
	RET_CODE (*io_control)(struct usb_hc_device *, enum hc_s3602_io_cmd, UINT32);
	
	HC_NOTIFY_CB hc_notify_uplayer;
	BLK_NOTIFY_CB bdev_callback;
	void(*dev_mount)(UINT32 id);
	void(*dev_umount)(UINT32 id);
	
	UINT8 adrsVec[USB_MAX_DEVICES];	/* USB addresses in use */
	struct usb_device  *pRoot;
	UINT32 nanSeconds;

	UINT8 uif_int_flag;			/* usb interface int flag bits */
	UINT8 otg_int_flag;			/* OTG int flag bits */
	UINT8 host_int_flag1;			/* host mode int flag1 bits */
	UINT8 host_int_flag2;			/* host mode int flag2 bits */
	UINT8 host_int_flag3;			/* host mode int flag3 bits */

	ID usbd_mbf_id;
	ID usbd_flg_id;

	ID hub_mbf_id;
//	ID bus_sema_id;
	ID int_sema_id;
	ID hub_sema_id;

	ID bulk_sema_id;
//	ID bulk_out_sema_id;
	ID ctrl_sema_id;
	UINT8 dev_existed;
	UINT8 rw_flag; //for ic bug used
};

typedef enum usb_device_speed
{
	USB_SPEED_UNKNOWN = 0,				/* enumerating */
	USB_SPEED_LOW,						/* usb 1.1 */
	USB_SPEED_FULL,						/* usb 1.1 */
	USB_SPEED_HIGH,						/* usb 2.0 */
} DEVICE_SPEED;

#define MAX_LUN_NUMBER	16
#define USB_MAXCHILDREN		(31)


//typedef void* USBD_NODE_ID, * pUSBD_NODE_ID;

typedef struct mass_info
{
	struct usb_hc_device *hc_dev;
	pUSBD_NODE_ID  NodeID;
	UINT16 Lun;
	UINT16 DevState;
	 
} MASS_INFO, *pMASS_INFO;


typedef struct usbd_node_info
{
	UINT16 nodeType;		/* Type of node */
	DEVICE_SPEED nodeSpeed;		/* Speed of node, e.g., 12MBit, 1.5MBit */
	USBD_NODE_ID parentHubId;	/* Node Id of hub to which node is connected */
	UINT16 parentHubPort;	/* Port on parent hub to which connected */
	USBD_NODE_ID rootId;	/* Node Id of root for USB to which connected */
} USBD_NODE_INFO, *pUSBD_NODE_INFO;


struct usb_hub_status 
{
	UINT16 wHubStatus;
	UINT16 wHubChange;
} __attribute__ ((packed));

/*  * Hub Status and Hub Change results * See USB 2.0 spec Table 11-19 and Table 11-20 */
struct usb_port_status 
{	
	UINT16 wPortStatus;	
	UINT16 wPortChange;	
} __attribute__ ((packed));

struct usb_hub_descriptor 
{
	UINT8  bDescLength;
	UINT8  bDescriptorType;
	UINT8  bNbrPorts;
	UINT16 wHubCharacteristics;
	UINT8  bPwrOn2PwrGood;
	UINT8  bHubContrCurrent;
	    	/* add 1 bit for hub status change; round to bytes */
	UINT8  DeviceRemovable[(USB_MAXCHILDREN + 1 + 7) / 8];
	UINT8  PortPwrCtrlMask[(USB_MAXCHILDREN + 1 + 7) / 8];
} __attribute__ ((packed));

typedef struct usbd_port
{
  struct usb_device* pNode;		/* node attached to port */
} USBD_PORT, * pUSBD_PORT;

struct usb_device
{
	
	USBD_NODE_ID nodeHandle;		/* handle assigned to node */
	USBD_NODE_INFO nodeInfo;		/* Current node information */
	UINT32 base_addr;		/* USB bus address for dev/hub */
	UINT16 topologyDepth;		/* number of cable hops from root  0: for root */
	UINT16 maxPower;			/* power draw for selected config */
	UINT32 CtrlRcvPipe;
	UINT32 CtrlTxPipe;

	BOOL nodeDeletePending;		/* TRUE when node being deleted */
	ID controlSem;		
	UINT8 				cmd_queue_flag;
	UsbDevState_t		dev_state;	/* configured, not attached, etc */
	struct usb_hc_device *hc_dev;
	struct usb_endpoint_descriptor *pEndpointDescriptor ;
	 struct usb_interface_descriptor *pInterfaceDescriptor ;
	void *priv;
	void (*release)(USBD_NODE_ID NodeId);
	UINT32 connected; /*Connect?*/
	UINT8 which_port;

	UINT16 bcdUSB;
	UINT8  bDeviceProtocol;
	UINT16 idVendor;
	UINT16 idProduct;
	UINT8  iSerialNumber;
	UsbdLockedState_t  IsLocked;

	/* NOTE: The following fields are used only for hub nodes. */
	UINT16 pwrGoodDelay;		/* power-ON to power good delay */
	UINT16 hubContrCurrent;		/* hub controller power requirements */
	BOOL selfPowered;			/* TRUE if hub/port is self powered */
	UINT16 maxPowerPerPort;		/* max power port can provide */
	struct upt *hubIrp;			/* IRP used to monitor hub status */
	UINT16 numPorts;			/* number of ports, used for hubs */
	pUSBD_PORT pPorts;			/* Array of ports, used for hubs */
	UINT8* pHubStatus; 		/* receives hub status */

	UINT32		HubStatusPipe;	/* status pipe handle */
	UINT32 		HubEndponitPacketSize;
	UINT8		HubEndponitInterval;	 /* interrupt interval */ 

	void (*notifyClient)(struct usb_hc_device *,USBD_NODE_ID  ,UINT16 );
	BOOL wr_flag;
	ID EventFlag;
	ID TimerID;

	UINT32 safe_removed;

	
}__attribute__((aligned(32)));

typedef struct hdl_descr
{
	void* handle;		/* Currently assigned handle */
	UINT32 handleSig;			/* signature used to validate handle */
	void* handleParam;			/* arbitrary parameter used by caller */
} HDL_DESCR, * pHDL_DESCR;

typedef struct usb_message
{
	UINT16 msg; 			/* Message code - application specific */
	UINT16 wParam;			/* WORD parameter - application specific */
	UINT32 lParam;			/* DWORD parameter - application specific */
} USB_MESSAGE, * pUSB_MESSAGE;

//typedef void* GENERIC_HANDLE;		/* type of a generic handle */

typedef struct usb_device USBD_NODE,*pUSBD_NODE;
typedef struct usb_hc_device USBD_BUS, *pUSBD_BUS ;
typedef struct usb_hub_descriptor  HUB_DESCRIPTOR,*pHUB_DESCRIPTOR ;
typedef struct usb_hub_status USB_HUB_STATUS, *pUSB_HUB_STATUS;
typedef struct usb_port_status HUB_PORT_STATUS, *pHUB_PORT_STATUS;

typedef struct usb_device_descriptor DEVICE_DESCRIPTOR, *pDEVICE_DESCRIPTOR;/* Descriptor */
typedef	struct usb_config_descriptor CONFIGURATION_DESCRIPTOR,*pCONFIGURATION_DESCRIPTOR; /* All of the configs */
typedef struct usb_interface_descriptor INTERFACE_DESCRIPTOR,*pINTERFACE_DESCRIPTOR; 
typedef struct usb_endpoint_descriptor ENDPOIN_DESCRIPTOR,*pENDPOIN_DESCRIPTOR; //add by victor



#endif /*__UEV_H__*/
