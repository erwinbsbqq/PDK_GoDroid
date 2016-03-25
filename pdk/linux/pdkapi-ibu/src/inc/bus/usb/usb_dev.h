#ifndef __USB_DEV_H__
#define __USB_DEV_H__

//#ifdef USB_SUPPORT_HUB
//#include <bus/usb2/usb_dev.h>
//#else
#include <basic_types.h>
#include <hld/hld_dev.h>



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

//end

//#define USB_INTERNAL_DISK_PARSE

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
	INT32 interval;					/* (modify) transfer interval	 * (INT/ISO) */
};

enum hc_s3602_io_cmd
{
	HC_GET_PORT_SPEED = 1,
	HC_GET_PORT_CONNECT,
	HC_USB_IP_RESET,
	HC_GET_BULK_OPT_INDEX,
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
} HcEventSubType_t;

typedef struct
{
	HcEventType_t event_type;
	HcEventSubType_t event_sub_type;
	UINT32 notify_param;
}HCEvtMsg_t, *pHCEvtMsg_t;

/* the callback function to notify the USBD what event is occurred from HCD */
typedef void (* HC_NOTIFY_CB)(HCEvtMsg_t, UINT32);
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
	struct usb_hc_device  *next;		//next device
	INT32 type; 						//HLD_DEV_TYPE_USB_HOST
	INT8 name[HLD_MAX_NAME_SIZE];	//Device Name
	INT32 flags;						//This field used to record current running status
	INT32 minor;						//This field used to record previous running status
	UINT32 base_addr;				//USB IP I/O Base Address
	UINT32 rh_port_num;				//Root Hub Port Number, Default is one
	void *priv;						// Used to be private for USB Host IP
	struct upt *curr_upt;				// current working upt 

	struct usb_device *usb_dev;
	volatile UINT8 usb_dev_removed;
	volatile UINT8 usb_dev_remove_done;
	volatile UINT8 usb_dev_locked;
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
};

enum usb_device_speed
{
	USB_SPEED_UNKNOWN = 0,				/* enumerating */
	USB_SPEED_LOW,						/* usb 1.1 */
	USB_SPEED_FULL,						/* usb 1.1 */
	USB_SPEED_HIGH,						/* usb 2.0 */
};

#define MAX_LUN_NUMBER	16

struct usb_device
{
	struct usb_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type; //HLD_DEV_TYPE_USB_MASS or HLD_DEV_TYPE_USB_HUB
	INT8 name[HLD_MAX_NAME_SIZE];
	INT32 flags; //This field used to record current running status

	INT32 hardware;
	INT32 busy;   //This field used to record sub state of DECA_STATE_PLAY, could be: busy, no data, no buffer.
	INT32 minor;//This field used to record previous running status
	
	UINT32 base_addr;
	
	UINT8 				cmd_queue_flag;
	UsbDevState_t		dev_state;	/* configured, not attached, etc */
	struct usb_hc_device *hc_dev;
	struct usb_device_descriptor *descriptor;/* Descriptor */
	struct usb_config_descriptor *config; /* All of the configs */
	enum usb_device_speed	dev_speed;
	void *priv;
	void (*release)(struct usb_device *dev);
#if 0
	enum usb_device_state		dev_state;	/* configured, not attached, etc */
	enum usb_device_speed	dev_speed;	/* high/full/low (or error) */
	enum usb_device_class 	dev_class;
	
	UINT32 toggle[2];		/* one bit for each endpoint
					 * ([0] = IN, [1] = OUT) */

	struct usb_host_endpoint ep0;

	struct usb_device_descriptor descriptor;/* Descriptor */
	struct usb_host_config *config;	/* All of the configs */

	struct usb_host_config *actconfig;/* the active configuration */
	struct usb_host_endpoint *ep_in[16];
	struct usb_host_endpoint *ep_out[16];

	char **rawdescriptors;		/* Raw descriptors for each config */

	UINT16 bus_mA;		/* Current available from the bus */
	UINT8 portnum;			/* Parent port number (origin 1) */

	INT32 have_langid;		/* whether string_langid is valid */
	INT32 string_langid;		/* language ID for strings */

	/* static strings from the device */
	char *product;			/* iProduct string, if present */
	char *manufacturer;		/* iManufacturer string, if present */
	char *serial;			/* iSerialNumber string, if present */

	struct usb_device  * parent;
	UINT32 max_child;
	struct usb_device  * children[USB_MAXCHILDREN];
	struct usb_bus * bus;
	
	void *priv;		/* Used to be 'private' but that upsets C++ */
#endif	
}__attribute__((aligned(32)));

//#endif

#endif /*__USB_DEV_H__*/
