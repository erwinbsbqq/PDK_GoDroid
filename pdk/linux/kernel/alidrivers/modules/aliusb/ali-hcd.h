/****************************************************************************
*  ALi Corporation, All Rights Reserved. 2006 Copyright (C)
*
*  File: hcd_s3602_proc.h
*
*  Description:  Declare of S3602 USB_INF & USB_IP private feature and state
*
*  History:
*      Date 			  Author		 	 	Version    	Comment
*      ========    ========		======		=======
*  1.  2006.12.04	  Jimmy Chen		0.1.000 		Initial
*  2.
****************************************************************************/
#ifndef  __HC_S3602_PROC_H__
#define  __HC_S3602_PROC_H__


//#define ALI_HOST_PRINTF(msg,args...)  do{printk("ali usb: " msg "\n", ## args);}while(0)	
#define ALI_HOST_PRINTF(msg,args...)	do{(void)0;}while(0)


// usb bus error 
#define HCD_S3602_BUS_OK				0
#define HCD_S3602_BUS_ERR				1

// Bulk transfer sub type 
enum ali_host_bulk_type
{
	BULK_TYPE_NULL = 0,
	BULK_IN_SINGLE,
	BULK_IN_MULTIPLE,
	BULK_OUT_SINGLE,
	BULK_OUT_MULTIPLE,
	BULK_CMD_QUEUE,
};

// hcd working sub status ,for contrl transfer only
enum ali_hcd_ctrl_transfer_stage
{
	HCD_CTRL_STAGE_NULL	= 0,
	HCD_CTRL_STAGE_SETUP,
	// indicate SETUP transaction
	HCD_CTRL_STAGE_DATA,
	// indicate DATA transaction
	HCD_CTRL_STAGE_STATUS,
	// indicate STATUS transaction
};

enum ali_host_pipe
{
	ALI_PIPE_NULL = 0,
	ALI_HOST_EP0,
	ALI_HOST_EPA,
	ALI_HOST_EPB,
	ALI_HOST_EPC,
	ALI_HOST_EPD,
	ALI_HOST_EPE,
	ALI_HOST_EPF,
};

struct ali_host
{
	spinlock_t lock;
	
	INT32 type; 						//HLD_DEV_TYPE_USB_HOST
//	INT8 name[HLD_MAX_NAME_SIZE];	//Device Name
	UINT32 soc_chip;					// ALI_S3602 ALI_S3603 ...
	UINT32 ic_version;				//
	UINT32 support_hub;				//3329E5 3101D/E/F 3602F
	UINT32 base_addr;				//USB IP I/O Base Address
	UINT32 irq_num;
	UINT32 rh_port_num;				//Root Hub Port Number, Default is one
	UINT32 usb_bus_err;
	UINT8 port_changed;
	UINT32 port_status;				// USB_PORT_FEAT_SUSPEND ...
	UINT32 port_status_last;				// USB_PORT_FEAT_SUSPEND ...
		
//ep = list_entry(ali_host->pipe_ctrl , struct ali_host_ep, queue);urb = ep->urb;ep = urb->hcpriv;ep= hep->hcpriv;
	struct list_head pipe_ctrl;			// endpoint0, 64 bytes FIFO
	struct list_head pipe_bulk_out;		//  endpoint A, bulk out 1024 bytes FIFO
	struct list_head pipe_bulk_in;		//  endpoint B, bulk in 1024 bytes FIFO
	struct list_head pipe_opt_bulk;		// endpoint A+B, not used
	struct list_head pipe_int_c;		// endpoint C, interrupt in 64 bytes FIFO
	struct list_head pipe_int_d;		// endpoint D, interrupt in 16 bytes FIFO
	struct list_head pipe_int_e;		// endpoint E, interrupt in 16 bytes FIFO
	struct list_head pipe_int_f;		// endpoint F,interrupt in 16 bytes FIFO
	
//	urb = container_of(ep->hep->urb_list.next, struct urb, urb_list);
	struct urb *curr_urb_ctrl;			// endpoint 0, 64 bytes FIFO
	struct urb *curr_urb_bulk_out;	// endpoint A, bulk out 1024 bytes FIFO
	struct urb *curr_urb_bulk_in;		//  endpoint B, bulk in 1024 bytes FIFO
	struct urb *curr_urb_opt_bulk;	// endpoint A+B, not used
	struct urb *curr_urb_int_c;		// endpoint C
	struct urb *curr_urb_int_d;		// endpint D, interrupt in 16 bytes FIFO
	struct urb *curr_urb_int_e;		// endpint E, interrupt in 16 bytes FIFO
	struct urb *curr_urb_int_f;		// endpint F, interrupt in 16 bytes FIFO

	struct usb_device  *pRoot;

	UINT8 uif_int_flag;			// usb interface int flag bits
	UINT8 otg_int_flag;			// OTG int flag bits
	UINT8 host_int_flag1;			// host mode int flag1 bits 
	UINT8 host_int_flag2;			// host mode int flag2 bits 
	UINT8 host_int_flag3;			// host mode int flag3 bits

};


struct ali_host_ep
{
	struct ali_host* ali_host;	// point to HC device 
	UINT8 pipe_num;			// EP0 EPA EPB EPC ...
	struct usb_host_endpoint *hep;
	struct usb_device	*udev;
	struct urb *urb;
	struct list_head queue;
	
	UINT32 ctrl_transfer_stage;	// status for contrl transfer only

	UINT16 int_id;				// interrupt ID 
	UINT8 bulk_type;				// current upt bulk transfer type: single or multi
	INT8 usb_bus_err;			// indicate usb bus error flag 

	UINT32 event_type;			/* event type to dispatch procedure */
	UINT32 transfer_type;
};


#endif /* __HC_S3602_PROC_H__ */

