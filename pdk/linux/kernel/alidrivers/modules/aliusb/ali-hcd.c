/****************************************************************************
 *  ALi Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: hcd_s3602_proc.c
 *
 *  Description: The core of HCD to process all UPT from USBD and report event/result to USBD
 *
 *  History:
 *      Date 			  Author		 	 	Version    	Comment
 *      ========    ========		======		=======
 *  1.  2006.12.04	  Jimmy Chen		0.1.000 		Initial
 *  2.
 ****************************************************************************/
#include <linux/version.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/usb.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/irq.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
#include <linux/usb/hcd.h>
#else
#include "../core/hcd.h"
#endif
#include "ali-hcd-reg.h"
#include "ali-hcd.h"


#define DRIVER_VERSION	"01 Mar 2010"

static const char hcd_name[] = "ali_hcd";


static void finish_request(struct ali_host *ali_host,struct urb *urb,	int status);
static void ali_hcd_init_data_toggle(UINT32 base_addr);
static void ali_hcd_init_epa_data_toggle(UINT32 base_addr);
static void ali_hcd_init_epb_data_toggle(UINT32 base_addr);
static void epa_error_handler(UINT32 base_addr);
static void epb_error_handler(UINT32 base_addr);
static void ali_hcd_all_urb_err(struct ali_host *ali_host);

static inline struct ali_host *hcd_to_ali_host(struct usb_hcd *hcd);

static inline struct urb *ali_host_get_ctrl_urb(struct ali_host *ali_host);
static inline struct urb *ali_host_get_bulk_in_urb(struct ali_host *ali_host);
static inline struct urb *ali_host_get_bulk_out_urb(struct ali_host *ali_host);
static inline struct urb *ali_host_get_opt_bulk_urb(struct ali_host *ali_host);
static inline struct urb *ali_host_get_int_c_urb(struct ali_host *ali_host);
static inline struct urb *ali_host_get_int_d_urb(struct ali_host *ali_host);
static inline struct urb *ali_host_get_int_e_urb(struct ali_host *ali_host);
static inline struct urb *ali_host_get_int_f_urb(struct ali_host *ali_host);

static int ali_host_start(struct usb_hcd *hcd);
static void ali_host_stop(struct usb_hcd *hcd);
static int ali_host_bus_reset(struct usb_hcd *hcd);
static int ali_host_bus_suspend(struct usb_hcd *hcd);
static int ali_host_bus_resume(struct usb_hcd *hcd);

/*
   asm (        
   "assembly code" 
   : output_operand 		// 输出参数列表
   : input_operand		// 输入参数列表 
   : clobbered_operand	// 被改变的操作对象列表 
   );
 */

#define Hit_Writeback_D                 0x19
#define Hit_Invalidate_D                0x11	

#ifndef cache_op
#define cache_op(op,addr)						\
	__asm__ __volatile__(						\
			"	.set	push					\n"	\
			"	.set	noreorder				\n"	\
			"	.set	mips3\n\t				\n"	\
			"	cache	%0, %1					\n"	\
			"	.set	pop					\n"	\
			:								\
			: "i" (op), "R" (*(unsigned char *)(addr)))
#endif

static inline void cache_flush(UINT8 *addr,UINT32  len)
{
	UINT32 end_addr;

	end_addr = (UINT32)addr + len;
	addr = (UINT8 *)((UINT32)addr&0xFFFFFFF0);
	while((UINT32)addr < end_addr)
	{
		cache_op(Hit_Writeback_D,addr);
		addr += 16;
	}
	return;

}

static inline void cache_invalid(UINT8 *addr,UINT32  len)
{
	UINT32 end_addr;

	end_addr = (UINT32)addr + len;
	addr = (UINT8 *)((UINT32)addr&0xFFFFFFF0);
	while((UINT32)addr < end_addr)
	{
		cache_op(Hit_Invalidate_D,addr);
		addr += 16;
	}
	return;
}


/**********************************************************
 * 	Name		:   	ali_hcd_sw_init
 *	Description	:   	initialize the hc s3602 software
 *	Parameter	:	ali_host: struct ali_host 
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_sw_init(struct ali_host *ali_host)
{
	ali_host->uif_int_flag = 0;
	ali_host->otg_int_flag = 0;
	ali_host->host_int_flag1 = 0;
	ali_host->host_int_flag2 = 0;

	ali_host->curr_urb_ctrl= NULL;
	ali_host->curr_urb_bulk_out= NULL;
	ali_host->curr_urb_bulk_in = NULL;
	ali_host->curr_urb_opt_bulk= NULL;
	ali_host->curr_urb_int_c= NULL;
	ali_host->curr_urb_int_d= NULL;
	ali_host->curr_urb_int_e= NULL;
	ali_host->curr_urb_int_f= NULL;

	INIT_LIST_HEAD(&ali_host->pipe_ctrl);
	INIT_LIST_HEAD(&ali_host->pipe_bulk_in);
	INIT_LIST_HEAD(&ali_host->pipe_bulk_out);
	INIT_LIST_HEAD(&ali_host->pipe_opt_bulk);
	INIT_LIST_HEAD(&ali_host->pipe_int_c);
	INIT_LIST_HEAD(&ali_host->pipe_int_d);
	INIT_LIST_HEAD(&ali_host->pipe_int_e);
	INIT_LIST_HEAD(&ali_host->pipe_int_f);

}

/**********************************************************
 * 	Name		:   	ali_hcd_hw_init
 *	Description	:   	initialize the hc s3602 hardware
 *	Parameter	:	struct ali_host* ali_host
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_hw_init(struct ali_host *ali_host)
{
	unsigned long	flags;

	UINT32 base_addr = ali_host->base_addr;

	//issue resume to clear suspend state 
	//issue_host_resume();

	//usb system reset
	spin_lock_irqsave(&ali_host->lock, flags);
	//	osal_task_dispatch_off();		// cpu will hang up. if other task access usb reg, while reset usb or power down usb PLL
	if (ali_host->soc_chip == ALI_CHIPID_M3329E)
	{
		*((volatile UINT32 *) 0xb8000060) = (*((volatile UINT32 *) 0xb8000060) | (1 << 10));	
		udelay(1);
		*((volatile UINT32 *) 0xb8000060) = (*((volatile UINT32 *) 0xb8000060) & (~(1 << 10)));
		udelay(1);	// wait clk  steady
	}
	if ((ali_host->soc_chip == ALI_CHIPID_S3602)||(ali_host->soc_chip == ALI_CHIPID_S3603))
	{
		if(base_addr == HC_S3603_BASE_ADDR)
		{
			*((UINT32 *) 0xb8000080) |= (0x1<<28);	
			udelay(1);
			*((UINT32 *) 0xb8000080) &= ~(0x1<<28);
		}
		else if(base_addr == HC_S3603_BASE_ADDR1)
		{
			*((UINT32 *) 0xb8000080) |= (0x1<<29);
			udelay(1);
			*((UINT32 *) 0xb8000080) &= ~(0x1<<29);
		}
		else if(base_addr == HC_S3602_BASE_ADDR)
		{
			*((volatile UINT32 *) 0xb8000080) = (*((volatile UINT32 *) 0xb8000080) | 0x10000000);	
			udelay(1);
			*((volatile UINT32 *) 0xb8000080) = (*((volatile UINT32 *) 0xb8000080) & 0xEFFFFFFF);
		}
		udelay(1);
	}
	if ((ali_host->soc_chip == ALI_CHIPID_M3329E)&&(ali_host->ic_version>= CHIP_IC_REV_5))
	{
		*((volatile UINT8 *)0xb800007E) = 0x07;	//USB_PLL_SEL: Reserved	/ USB_INTF Reset/PING Token Control /Retry Control		DISC_SEL/PD_HS / PD_FS /PD_PLL
		udelay(1);
		*((volatile UINT8 *)0xb800007E) = 0x00;	//USB_PLL_SEL: Reserved	Reserved	Reserved	Reserved	 DISC_SEL/PD_HS /PD_FS	PD_PLL
		udelay(1);
		*((volatile UINT8 *)0xb800007C) = 0x0b;	//bit4-0 PLL LPF control
		*((volatile UINT8 *)0xb800007D) = 0x7C;	//Bit 7-4: For squelch level Control  Bit 3-2: TX Driving Current Control
		*((volatile UINT8 *)0xb800007E) = 0x30;	//USB_PLL_SEL: Reserved	/ USB_INTF Reset/PING Token Control /Retry Control	  DISC_SEL/PD_HS/ PD_FS/ PD_PLL
	}
	if (ali_host->soc_chip == ALI_CHIPID_S3602)
	{
		ali_host->ic_version = 2;
		if(ali_host->ic_version < 2)
		{
			*((UINT8 *)0xb800000C) = 0x07;	//USB_PLL_SEL: DISC_SEL/PD_HS/PD_FS/PD_PLL . SQUELCH_LEVEL -/TX_DRIVING -
			udelay(2);
			*((UINT8 *)0xb800000C) = 0x00;	//USB_PLL_SEL: DISC_SEL/PD_HS/PD_FS/PD_PLL . SQUELCH_LEVEL -/TX_DRIVING -
			udelay(1);
		}else{
			*((volatile UINT8 *) 0xb8000064) = 0x07;	//USB_PLL_SEL: DISC_SEL/PD_HS/PD_FS/PD_PLL . SQUELCH_LEVEL -/TX_DRIVING -
			udelay(2);
			*((volatile UINT8 *) 0xb8000064) = 0x00;	//USB_PLL_SEL: DISC_SEL/PD_HS/PD_FS/PD_PLL . SQUELCH_LEVEL -/TX_DRIVING -
			udelay(1);
		}
	}
	if(ali_host->soc_chip == ALI_CHIPID_S3603)
	{
		*((UINT8 *) (base_addr + 0xf2)) = 0x07;	//USB_PLL_SEL: DISC_SEL/PD_HS/PD_FS/PD_PLL . SQUELCH_LEVEL -/TX_DRIVING -
		udelay(2);
		*((UINT8 *) (base_addr + 0xf2)) = 0x00;	//USB_PLL_SEL: DISC_SEL/PD_HS/PD_FS/PD_PLL . SQUELCH_LEVEL -/TX_DRIVING -
	}
	//	osal_task_dispatch_on();

	// software reset USB host IP 
	software_rst_usbip(base_addr);

	// clear suspend state 
	//clear_host_suspend(base_addr);

	// EPA auto force enable when DMA counter down counts form 1 to 0, or EOT event
	if(ali_host->soc_chip == ALI_CHIPID_S3603)
		en_epa_auto_force_3603(base_addr);
	else
		en_epa_auto_force(base_addr);


	// set OTG to host mode 
	set_usbip_host_mode(base_addr);

	// EPA type is Bulk OUT, fifo type is ping-pong 
	set_ep_transfer_type(base_addr, HEPASETR, B_EPTYPE_BULK);
	set_fifo_type(base_addr, HEPASETR, B_FIFO_PINGPONG);

	// EPB type is Bulk IN, fifo type is ping-pong 
	set_ep_transfer_type(base_addr, HEPBSETR, B_EPTYPE_BULK);
	set_fifo_type(base_addr, HEPBSETR, B_FIFO_PINGPONG);

	if(ali_host->soc_chip == ALI_CHIPID_S3603)
	{
		// EPC type is Intr, fifo type is ping-pong 
		set_ep_transfer_type(base_addr, HEPCSETR, B_EPTYPE_INTR);
		set_fifo_type(base_addr, HEPCSETR, B_FIFO_PINGPONG);

		// EPD type is Intr, fifo type is ping-pong
		set_ep_transfer_type(base_addr, HEPDSETR, B_EPTYPE_INTR);
		set_fifo_type(base_addr, HEPDSETR, B_FIFO_PINGPONG);

		// EPE type is Intr, fifo type is ping-pong
		set_ep_transfer_type(base_addr, HEPESETR, B_EPTYPE_INTR);
		set_fifo_type(base_addr, HEPESETR, B_FIFO_PINGPONG);

		// EPF type is Intr, fifo type is ping-pong
		set_ep_transfer_type(base_addr, HEPFSETR, B_EPTYPE_INTR);
		set_fifo_type(base_addr, HEPFSETR, B_FIFO_PINGPONG);
	}

	// init EP0's fifo 
	init_ep_fifo(base_addr, HEP0FCTR);
	// init EPA's fifo 
	init_ep_fifo(base_addr, HEPAFCTR);
	// init EPB's fifo 
	init_ep_fifo(base_addr, HEPBFCTR);

	if(ali_host->soc_chip == ALI_CHIPID_S3603)
	{
		// init EPC's fifo 
		init_ep_fifo(base_addr, HEPCFCTR);
		// init EPD's fifo 
		init_ep_fifo(base_addr, HEPDFCTR);
		// init EPE's fifo 
		init_ep_fifo(base_addr, HEPEFCTR);
		// init EPF's fifo 
		init_ep_fifo(base_addr, HEPFFCTR);
	}

	// read all interrupt flag first to make sure no previous state occur 
	get_host_int_flag1(base_addr);
	get_host_int_flag2(base_addr);
	get_host_int_flag3(base_addr);
	get_usb_intf_int_status(base_addr);

	// enable usb host interrupt except SOF int
	//	en_host_int_flag1(base_addr, B_OVERCUR_IE|B_BABBLE_IE|B_H_SUSREM_IE|B_USB_RST_IE|B_CONN_DSCN_IE);
	en_host_int_flag1(base_addr,  B_CONN_DSCN_IE);	// enable connect disconnect interrup only
	en_host_int_flag2(base_addr, B_H_INT2_ALL_IE);

	set_nak_retry_period(base_addr, 1);

	// enable usb interface interrupt 
	enable_all_usb_intf_int(base_addr);

	if(ali_host->support_hub == 1)
		host_new_mode_set(base_addr,TRUE);

	spin_unlock_irqrestore(&ali_host->lock, flags);

}

/**********************************************************
 * 	Name		:   	ali_hcd_get_ctrlin_data
 *	Description	:   	get contorl IN data from ep0's fifo to urb' buffer
 *	Parameter	:	urb: pointer of current working urb
 *	Return		:	void
 ***********************************************************/
static UINT32 ali_hcd_get_ctrlin_data(struct ali_host *ali_host,struct urb *urb, UINT32 offset)
{
	UINT32 base_addr;
	UINT32 i = 0;

	base_addr = ali_host->base_addr;
	// get contorl IN data from ep0's fifo to upt' buffer 
	while (get_ep_fifo_empty_status(base_addr, HEP0FCTR) ==	FALSE)
	{
		// data length in fifo more than request length 
		if ((i + offset) == urb->transfer_buffer_length)
		{
			break;
		}
		rw_ep_fifo_port(base_addr, HEP0FIFO, FALSE,
				((UINT8 *) ((urb->transfer_buffer) + offset) + i));
		ALI_HOST_PRINTF(" %02x", *((UINT8 *) ((urb->transfer_buffer) + offset) + i));
		i++;
		//udelay(50);
	}

	urb->actual_length += i;
	return i;
}

/**********************************************************
 * 	Name		:   	ali_hcd_pkindex2value
 *	Description	:   	covert the packet size index to real value
 *	Parameter	:	pkt_size: packet size index
 *	Return		:	UINT16: real packet size value
 ***********************************************************/
#if 0
static UINT16 ali_hcd_pkindex2value(UINT8 pkt_size)
{
	UINT16 pkt_value = 8;
	switch (pkt_size)
	{
		case 0:
			pkt_value = 8; break;
		case 1:
			pkt_value = 16; break;
		case 2:
			pkt_value = 32; break;
		case 3:
			pkt_value = 64; break;
		case 4:
			pkt_value = 128; break;
		case 5:
			pkt_value = 256; break;
		case 6:
			pkt_value = 512; break;
		case 7:
			pkt_value = 1024; break;
	}
	return pkt_value;
}
#endif

/**********************************************************
 * 	Name		:   	ali_hcd_pkt_size2index
 *	Description	:   	covert the packet size  to index
 *	Parameter	:	pkt_size: packet size
 *	Return		:	UINT16: packet size index
 ***********************************************************/
static UINT16 ali_hcd_pkt_size2index(UINT32 pkt_size)
{
	UINT16 pkt_index = 8;
	switch (pkt_size)
	{
		case 8:
			pkt_index = 0; break;
		case 16:
			pkt_index = 1; break;
		case 32:
			pkt_index = 2; break;
		case 64:
			pkt_index = 3; break;
		case 128:
			pkt_index = 4; break;
		case 256:
			pkt_index = 5; break;
		case 512:
			pkt_index = 6; break;
		case 1024:
			pkt_index = 7; break;
	}
	return pkt_index;
}

/**********************************************************
 * 	Name		:   	ali_hcd_issue_setup
 *	Description	:   	when control transfer, Set SETUP Packet data into EP0 fifo and force issue it 
 *	Parameter	:	
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_issue_setup(struct ali_host *ali_host,struct urb *urb)
{
	UINT8* fifo_data;
	UINT32 i;
	UINT32 base_addr = ali_host->base_addr;
	UINT8 dev_addr;
	struct ali_host_ep *ep = NULL;	
	struct usb_device *udev = urb->dev;
	struct usb_device *parent_udev = udev->parent;
	BOOL splitFlag= FALSE;

	ep = urb->hcpriv;
	if(ep == NULL)
		return;

	if (NULL == urb->setup_packet)
	{
		ALI_HOST_PRINTF("setup packet has not built up!\n");
		return;
	}

	fifo_data = (UINT8 *)urb->setup_packet;

	//set function address
	dev_addr = (UINT8) usb_pipedevice(urb->pipe);
	if(ali_host->support_hub == 0)
	{
		set_device_address(base_addr, dev_addr);
	}else{
		if((udev->speed !=  USB_SPEED_HIGH)&&(parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if((parent_udev->speed == USB_SPEED_HIGH)&&(parent_udev->descriptor.bDeviceClass == USB_CLASS_HUB))  // USB_NODETYPE_HUB udev->parent->descriptor.bDeviceClass == USB_CLASS_HUB
			{	
				splitFlag=TRUE;
				set_ssplit_ctrl_info(base_addr,EP0_SPT_CSPT_CTL_REG, 0, udev->ttport, parent_udev->devnum);
			}
		}
		if((parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if(udev->speed ==USB_SPEED_HIGH)
				set_plug_status(base_addr,0x2);
			if(udev->speed ==USB_SPEED_FULL)
				set_plug_status(base_addr,0x1);
		}
		set_ep_device_address(base_addr,EP0_HUB_SET_REG,dev_addr,splitFlag);
	}

	//set SETUP token
	set_ep0_token_pid(base_addr, B_TOKEN_SETUP);

	// init the EP0 FIFO 
	init_ep_fifo(base_addr, HEP0FCTR);

	// write data bytes one by one to ep0 fifo 
	ALI_HOST_PRINTF("The data of ctrl setup are: ");
	for (i = 0 ; i < 8 ; i++)
	{
		rw_ep_fifo_port(base_addr, HEP0FIFO, TRUE, (UINT8 *)(fifo_data + i));
		ALI_HOST_PRINTF("%02x ", *(UINT8 *) (fifo_data + i));
	}
	ALI_HOST_PRINTF("\n");

	// force issue setup packet 
	ep_force_token_sending(base_addr, HEP0FCTR);
	ep->ctrl_transfer_stage = HCD_CTRL_STAGE_SETUP;
}

/**********************************************************
 * 	Name		:   	proc_ctrl_setup_done
 *	Description	:   	process for ep0 OK event on sub status: SETUP 
 *	Parameter	:	
 *	Return		:	void
 ***********************************************************/
static void proc_ctrl_setup_done(struct ali_host *ali_host)
{
	UINT32 i;
	struct ali_host_ep *ep = NULL;	
	struct urb *urb = ali_host->curr_urb_ctrl;
	UINT32 base_addr = ali_host->base_addr;
	struct usb_ctrlrequest  * setup_packet;

	if(urb == NULL)
		return;
	ep = urb->hcpriv;
	if(ep == NULL)
		return;

	ALI_HOST_PRINTF("control SETUP phase OK!\n");

	setup_packet =(struct usb_ctrlrequest  *)(urb->setup_packet);
	//	ALI_HOST_PRINTF("setup_packet->bRequest: %02x\n",setup_packet->bRequest);
	//	if((setup_packet->bRequest == USB_REQ_SET_CONFIGURATION)||
	//(setup_packet->bRequest == USB_REQ_CLEAR_FEATURE))

	if(setup_packet->bRequest == USB_REQ_SET_CONFIGURATION)
		ali_hcd_init_data_toggle(base_addr);

	// turn sub status to DATA phase 
	ep->ctrl_transfer_stage = HCD_CTRL_STAGE_DATA;

	// init EP0 and force IN token 
	init_ep_fifo(base_addr, HEP0FCTR);
	if (usb_pipeout(urb->pipe)&&(urb->transfer_buffer_length > 0))
	{
		i = 0;
		while (get_ep_fifo_full_status(base_addr, HEP0FCTR) ==	FALSE)
		{
			// data length in fifo more than request length 
			if (i  >= urb->transfer_buffer_length)
			{
				break;
			}
			rw_ep_fifo_port(base_addr, HEP0FIFO, TRUE,	((UINT8 *) (urb->transfer_buffer  + i)));
			ALI_HOST_PRINTF(" %02x", *((UINT8 *) (urb->transfer_buffer + i)));
			i++;
		}
		set_ep0_token_pid(base_addr, B_TOKEN_OUT);
	}else{
		set_ep0_token_pid(base_addr, B_TOKEN_IN);
	}
	ep_force_token_sending(base_addr, HEP0FCTR);
}

/**********************************************************
 * 	Name		:   	proc_ctrl_data_done
 *	Description	:   	process for ep0 OK event on sub status: DATA 
 *	Parameter	:	p_priv: s3602 usb hcd hal private
 *	Return		:	void
 ***********************************************************/
static void proc_ctrl_data_done(struct ali_host *ali_host)
{
	UINT32 pkt_size;
	struct ali_host_ep *ep = NULL;	
	struct urb *urb = ali_host->curr_urb_ctrl;
	UINT32 base_addr = ali_host->base_addr;
	int is_out;
	int urb_status;
	UINT32 single_in_len;

	if(urb == NULL)
		return;
	ep = urb->hcpriv;
	if(ep == NULL)
		return;

	// control IN transfer 
	if (usb_pipein(urb->pipe))
	{
		ALI_HOST_PRINTF("ep0 recv data \n" );
		is_out = !usb_pipein(urb->pipe);
		pkt_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
		if (pkt_size <urb->transfer_buffer_length)
		{
			single_in_len = ali_hcd_get_ctrlin_data(ali_host, urb, urb->actual_length);
			ALI_HOST_PRINTF("pkt_size:%d single_in_len:%d \n" ,pkt_size,single_in_len);
			if ((urb->actual_length < urb->transfer_buffer_length)&&(single_in_len == pkt_size)) // fow string lang ID wlen:255 but len = 4
			{
				init_ep_fifo(base_addr, HEP0FCTR);
				set_ep0_token_pid(base_addr, B_TOKEN_IN);
				ep_force_token_sending(base_addr, HEP0FCTR);
				return;
			}
		}
		else
		{
			ali_hcd_get_ctrlin_data(ali_host, urb, 0);
		}
		ALI_HOST_PRINTF("\n");
		ALI_HOST_PRINTF("control DATA phase OK!\n");
		// turn sub status to STATUS phase 
		ep->ctrl_transfer_stage = HCD_CTRL_STAGE_STATUS;

		// init EP0 and force OUT token 
		init_ep_fifo(base_addr, HEP0FCTR);
		set_ep0_token_pid(base_addr, B_TOKEN_OUT);
		ep_force_token_sending(base_addr, HEP0FCTR);
	}
	// control OUT transfer 
	else
	{
		ALI_HOST_PRINTF("control STATUS phase OK!\n");	
		// control out upt ok 
		urb->actual_length = 0;

		//init ep0 fifo	 and force IN token	
		init_ep_fifo(base_addr, HEP0FCTR);
		if(urb->transfer_buffer_length > 0)
		{
			set_ep0_token_pid(base_addr, B_TOKEN_IN);
			ep_force_token_sending(base_addr, HEP0FCTR);
			ep->ctrl_transfer_stage = HCD_CTRL_STAGE_STATUS;
		}else{
			// init status & clear all int flag after usb transfer complete 
			urb_status = 0;
			finish_request(ali_host, urb,urb_status);
			urb = ali_host_get_ctrl_urb(ali_host);
			ali_host ->curr_urb_ctrl = urb;
			if(urb != NULL)
			{
				ali_hcd_issue_setup(ali_host,urb);
			}	
		}
	}
}

/**********************************************************
 * 	Name		:   	proc_ctrl_status_done
 *	Description	:   	process for ep0 OK event on sub status: STATUS 
 *	Parameter	:	p_priv: s3602 usb hcd hal private
 *	Return		:	void
 ***********************************************************/
static void proc_ctrl_status_done(struct ali_host *ali_host)
{
	UINT8 temp;
	struct ali_host_ep *ep = NULL;	
	struct urb *urb = ali_host->curr_urb_ctrl;
	UINT32 base_addr = ali_host->base_addr;
	int urb_status;

	if(urb == NULL)
		return;
	ep = urb->hcpriv;
	if(ep == NULL)
		return;

	ALI_HOST_PRINTF("control STATUS phase OK!\n");

	if (usb_pipeout(urb->pipe))
	{
		rw_ep_fifo_port(base_addr, HEP0FIFO, FALSE,&temp);
	}
	//init ep0 fifo		
	init_ep_fifo(base_addr, HEP0FCTR);

	// control in urb ok 

	// init status & clear all int flag after usb transfer complete 
	urb_status = 0;
	finish_request(ali_host, urb,urb_status);
	ep->ctrl_transfer_stage = HCD_CTRL_STAGE_NULL;
	urb = ali_host_get_ctrl_urb(ali_host);
	ali_host ->curr_urb_ctrl = urb;
	if(urb != NULL)
	{
		ali_hcd_issue_setup(ali_host,urb);
	}	
}

/**********************************************************
 * 	Name		:   	ali_hcd_bulkin_start
 *	Description	:   	s3602 usb bulk in transfer start 
 *	Parameter	:	
 *	Return		:	void
 ***********************************************************/ 
static void ali_hcd_bulkin_start(struct ali_host *ali_host, struct urb *urb)
{
	UINT8 dev_addr;
	UINT32 pkt_size;
	UINT8 pkt_index;
	UINT8 ep_num;
	UINT32 base_addr = ali_host->base_addr;
	struct usb_device *udev = urb->dev;
	struct usb_device *parent_udev = udev->parent;
	struct ali_host_ep *ep = urb->hcpriv;
	BOOL splitFlag= FALSE;
	int is_out;

	// set function address
	dev_addr = (UINT8) usb_pipedevice(urb->pipe);
	if(ali_host->support_hub == 0)
	{
		set_device_address(base_addr, dev_addr);
	}else{
		if ((udev->speed !=  USB_SPEED_HIGH) &&(parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if((parent_udev->speed == USB_SPEED_HIGH)&&(parent_udev->descriptor.bDeviceClass == USB_CLASS_HUB)) 
			{	
				splitFlag=TRUE;
				set_ssplit_ctrl_info(base_addr,EPB_SPT_CSPT_CTL_REG, 0, udev->ttport,parent_udev->devnum);

			}
		}
		if((parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if(udev->speed ==USB_SPEED_HIGH)
				set_plug_status(base_addr,0x2);
			if(udev->speed ==USB_SPEED_FULL)
				set_plug_status(base_addr,0x1);
		}
		set_ep_device_address(base_addr,EPB_HUB_SET_REG,dev_addr,splitFlag);
	}

	// set EPB's max packet size
	is_out = !usb_pipein(urb->pipe);
	pkt_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
	pkt_index = ali_hcd_pkt_size2index(pkt_size);
	set_epb_packet_size(base_addr, pkt_index);

	// set logical ep num. 
	ep_num = usb_pipeendpoint(urb->pipe);
	set_ep_logical_num(base_addr, HEPBSETR, ep_num);

	//HCD_PROC_PRINTF("dev %d; epIn %d; size%d\n", dev_addr, ep_num, pk_size);

	// read all interrupt flag first to make sure no previous state occur
	get_host_int_flag1(base_addr);
	get_host_int_flag2(base_addr);
	get_usb_intf_int_status(base_addr);

	// Enable all interrupt of usb interface except for BULK finish int
	enable_all_usb_intf_int(base_addr);
	mask_usb_intf_int(base_addr, B_BULK_FIN_INT_DN);

	// set token in from PIO
	set_token_in_by_pio(base_addr);

	// Disable DMA Auto
	disable_dma_auto_option(base_addr);	

	// clear DMA RX index
	clear_dma_rx_index(base_addr); 

	ALI_HOST_PRINTF("bulk in dma addr: %08x\n", ((UINT32) urb->transfer_buffer));
	// set DMA RX base address match the upt
	set_dma_rx_base_addr(base_addr, ((UINT32) urb->transfer_buffer)|0xA0000000); 	// un cache

	// init EPB's fifo
	init_ep_fifo(base_addr, HEPBFCTR);

	//pkt_value = hcd_pkindex2value(usb_pipepktsize_get(p_upt->pipe));
	// single bulk in 
	//if(p_upt->length <= pkt_value)

	// turn bulk sub type 
	ep->bulk_type = BULK_IN_MULTIPLE;

	// Auto DMA RX enable and set packet size
	//	enable_dma_auto_rx(base_addr, usb_pipepktsize_get(urb->pipe));
	enable_dma_auto_rx(base_addr, pkt_index);

	// enable all host interrupt flag2 except EPB/EPA FINISH_OK
	//	en_host_int_flag2_all(base_addr);
	//	mask_host_int_flag2(base_addr, B_EPB_FSHOK_IE|B_EPA_FSHOK_IE);
	en_host_int_flag2(base_addr, 0x9F);

	// Set HOST IP RX DMA Count 
	set_dma_rx_cnt(base_addr, urb->transfer_buffer_length);
	ALI_HOST_PRINTF(" bulk in dma len: %d\n",urb->transfer_buffer_length);

	// Init DMA RX FSM status 
	init_dma_rx_state(base_addr);

	// Set USB_INTF DMA_RX_START 
	start_dma_rx_transfer(base_addr);

	// Set HOST IP Token IN and DMA Enable to transfer 
	if(ali_host->soc_chip == ALI_CHIPID_S3603)
		dma_host_in_token(base_addr,TRUE);      
	//duanwu    03/29/2009

	en_dma_transfer(base_addr, TRUE);
}

/*
   single token EPB IN programming flow.
   1. EPB Force IN		
   USB_IO_BASE + 0x17 = 0x20;  
   2. wait EPB finish OK interrutp
   check USB_IO_BASE + 0x21 == 0x20;
   3. read EPB transaction counter
   EPB_TRANS_CNT = USB_IO_BASE + 0x28,0x29
   4. set dma enable
   USB_IO_BASE + 0x40 =  RX_BASE_ADDRESS;  
   USB_IO_BASE + 0x44 =  RX_INDEX;  
   USB_IO_BASE + 0x38,39,3a = EPB_TRANS_CNT;
   USB_IO_BASE + 0x60 =  0x10;		//DMA_RX_START
   USB_IO_BASE + 0x32 =  0x08;		//DMA transfer EN
   5. wait DMA finish interrupt
   check USB_IO_BASE + 0x63 == 0x04;
 */
static void ali_hcd_single_bulkin_start(struct ali_host *ali_host, struct urb *urb)
{
	UINT8 dev_addr;
	UINT32 pkt_size;
	UINT8 pkt_index;
	UINT8 ep_num;
	UINT32 base_addr = ali_host->base_addr;
	struct usb_device *udev = urb->dev;
	struct usb_device *parent_udev = udev->parent;
	struct ali_host_ep*ep = urb->hcpriv;
	BOOL splitFlag= FALSE;
	//	UINT16 epb_trans_cnt = 0;
	int is_out;

	// set function address 
	dev_addr = (UINT8) usb_pipedevice(urb->pipe);
	if(ali_host->support_hub == 0)
	{
		set_device_address(base_addr, dev_addr);
	}else{
		if ((udev->speed !=  USB_SPEED_HIGH) &&(parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if((parent_udev->speed == USB_SPEED_HIGH)&&(parent_udev->descriptor.bDeviceClass == USB_CLASS_HUB)) 
			{	
				splitFlag=TRUE;
				set_ssplit_ctrl_info(base_addr,EPB_SPT_CSPT_CTL_REG, 0, udev->ttport,parent_udev->devnum);

			}
		}
		if((parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if(udev->speed ==USB_SPEED_HIGH)
				set_plug_status(base_addr,0x2);
			if(udev->speed ==USB_SPEED_FULL)
				set_plug_status(base_addr,0x1);
		}
		set_ep_device_address(base_addr,EPB_HUB_SET_REG,dev_addr,splitFlag);
	}

	// set EPB's max packet size
	//	pkt_size = usb_pipepktsize_get(urb->pipe);
	is_out = !usb_pipein(urb->pipe);
	pkt_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
	pkt_index = ali_hcd_pkt_size2index(pkt_size);
	set_epb_packet_size(base_addr, pkt_index);

	// set logical ep num. 
	ep_num = usb_pipeendpoint(urb->pipe);
	set_ep_logical_num(base_addr, HEPBSETR, ep_num);

	//HCD_PROC_PRINTF("dev %d; epIn %d; size%d\n", dev_addr, ep_num, pk_size);

	// read all interrupt flag first to make sure no previous state occur 
	get_host_int_flag1(base_addr);
	get_host_int_flag2(base_addr);
	get_usb_intf_int_status(base_addr);

	// Enable all interrupt of usb interface except for BULK finish int
	enable_all_usb_intf_int(base_addr);
	mask_usb_intf_int(base_addr, B_BULK_FIN_INT_DN);

	// set token in from PIO 
	set_token_in_by_pio(base_addr);

	// Disable DMA Auto 
	disable_dma_auto_option(base_addr);	

	// clear DMA RX index 
	clear_dma_rx_index(base_addr); 

	ALI_HOST_PRINTF("single token bulk in dma addr: %08x\n", ((UINT32) urb->transfer_buffer));
	ALI_HOST_PRINTF("single token bulk in dma len: %d\n",urb->transfer_buffer_length);
	// set DMA RX base address match the upt
	set_dma_rx_base_addr(base_addr, (UINT32) urb->transfer_buffer); 

	// init EPB's fifo 
	init_ep_fifo(base_addr, HEPBFCTR);

	//pkt_value = hcd_pkindex2value(usb_pipepktsize_get(p_upt->pipe));
	// single bulk in 
	//if(p_upt->length <= pkt_value)

	// turn bulk sub type 
	ep->bulk_type = BULK_IN_SINGLE;

	// Auto DMA RX enable and set packet size
	//	enable_dma_auto_rx(base_addr, usb_pipepktsize_get(p_upt->pipe));

	// enable all host interrupt flag2 except EPB/EPA FINISH_OK
	en_host_int_flag2_all(base_addr);
	mask_host_int_flag2(base_addr, B_EPA_FSHOK_IE);
	//en_host_int_flag2(base_addr, 0x9F);

	// force issue bulk in token 
	ep_force_token_sending(base_addr, HEPBFCTR);

}

/**********************************************************
 * 	Name		:   	ali_hcd_bulkout_start
 *	Description	:   	s3602 usb bulk out transfer start 
 *	Parameter	:	
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_bulkout_start(struct ali_host *ali_host, struct urb *urb)
{
	UINT8 dev_addr;
	UINT32 pkt_size;
	UINT8 pkt_index;
	UINT8 ep_num;
	UINT32 base_addr = ali_host->base_addr;
	struct usb_device *udev = urb->dev;
	struct usb_device *parent_udev = udev->parent;
	struct ali_host_ep *ep = urb->hcpriv;
	BOOL splitFlag= FALSE;
	int is_out;

	// set function address 
	dev_addr = (UINT8) usb_pipedevice(urb->pipe);

	if(ali_host->support_hub == 0)
	{
		set_device_address(base_addr, dev_addr);
	}else{
		if ((udev->speed !=  USB_SPEED_HIGH) &&(parent_udev!= NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if((parent_udev->speed == USB_SPEED_HIGH)&&(parent_udev->descriptor.bDeviceClass == USB_CLASS_HUB)) // USB_NODETYPE_HUB
			{	
				splitFlag=TRUE;
				set_ssplit_ctrl_info(base_addr,EPA_SPT_CSPT_CTL_REG, 0, udev->ttport, parent_udev->devnum);
			}
		}
		if((parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if(udev->speed ==USB_SPEED_HIGH)
				set_plug_status(base_addr,0x2);
			if(udev->speed ==USB_SPEED_FULL)
				set_plug_status(base_addr,0x1);
		}
		set_ep_device_address(base_addr,EPA_HUB_SET_REG,dev_addr,splitFlag);
	}

	// set EPA's max packet size 
	//pkt_size = usb_pipepktsize_get(urb->pipe);
	is_out = !usb_pipein(urb->pipe);
	pkt_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
	//	ALI_HOST_PRINTF("bulk out pkt_size: %x\n", pkt_size);
	pkt_index = ali_hcd_pkt_size2index(pkt_size);
	set_epa_packet_size(base_addr, pkt_index);
	//	ALI_HOST_PRINTF("bulk out pkt_index: %x\n", pkt_index);

	// set logical ep no. 
	ep_num = usb_pipeendpoint(urb->pipe);
	set_ep_logical_num(base_addr, HEPASETR, ep_num);

	//HCD_PROC_PRINTF("dev %d; epOut %d; size%d\n", dev_addr, ep_num, pk_size);

	// read all interrupt flag first to make sure no previous state occur 
	get_host_int_flag1(base_addr);
	get_host_int_flag2(base_addr);
	get_usb_intf_int_status(base_addr);

	// Enable all usb interface interrupt except for BULK finish int
	enable_all_usb_intf_int(base_addr);
	mask_usb_intf_int(base_addr, B_BULK_FIN_INT_DN);

	// set token in from PIO 
	set_token_in_by_pio(base_addr);

	// turn bulk sub type
	ep->bulk_type = BULK_OUT_MULTIPLE;
	set_dma_rx_cnt(base_addr, 0);
	// enable all host interrupt1 except for SOF flag
	en_host_int_flag1_all(base_addr);

	// enable all host interrupt flag2 except EPB/EPA FINISH_OK 
	//en_host_int_flag2_all(base_addr);
	//mask_host_int_flag2(base_addr, B_EPB_FSHOK_IE|B_EPA_FSHOK_IE);
	en_host_int_flag2(base_addr, 0x9F);

	// Auto DMA TX enable and set packet size 
	//	enable_dma_auto_tx(base_addr, usb_pipepktsize_get(urb->pipe));
	enable_dma_auto_tx(base_addr, pkt_size);


	// init EPA's fifo 
	init_ep_fifo(base_addr, HEPAFCTR);

	ALI_HOST_PRINTF("bulk out dma addr: %08x\n", ((UINT32) urb->transfer_buffer));

	cache_flush( urb->transfer_buffer,urb->transfer_buffer_length);
	//	msleep(200);
	// set DMA TX base address 
	set_dma_tx_base_addr(base_addr, ((UINT32) urb->transfer_buffer)|0xA0000000); 


	// clear DMA TX index 
	clear_dma_tx_index(base_addr); 	

	// Set HOST IP TX DMA Count 
	set_dma_tx_transfer_byte(base_addr, urb->transfer_buffer_length);
	ALI_HOST_PRINTF(" bulk out dma len: %d \n",urb->transfer_buffer_length);

	// Set USB_INTF DMA_TX_START 
	start_dma_tx_transfer(base_addr);

	// Set HOST IP Token IN and DMA Enable to transfer 
	//dma_host_in_token(FALSE);
	en_dma_transfer(base_addr, TRUE);

}

/**********************************************************
 * 	Name		:   	ali_hcd_opt_bulk_start
 *	Description	:   	s3602 usb optimize bulk transfer start 
 *	Parameter	:	
 *	Return		:	void
 ***********************************************************/
#if 0
static void ali_hcd_opt_bulk_start(struct ali_host *ali_host, struct urb *urb)
{

#if 0	// linux urb have no cmd queue mode
	UINT8 dev_addr;
	UINT32 pka_size;
	UINT32 pkb_size;
	UINT8 pka_index;
	UINT8 pkb_index;
	UINT8 ep_num;
	UINT32 base_addr = ali_host->base_addr;
	int is_out;

	// set function address
	dev_addr = (UINT8) usb_pipedevice(urb->pipe);
	set_device_address(base_addr, dev_addr);

	// set EPA's max packet size
	pka_size = usb_pipepktsize_get(urb->pipe);
	is_out = !usb_pipein(urb->pipe);
	pka_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
	pka_index = ali_hcd_pkt_size2index(pka_size)
		set_epa_packet_size(base_addr, pka_index);

	// set EPB's max packet size
	pkb_size = usb_pipepktsize_get(urb->pipe1);
	is_out = !usb_pipein(urb->pipe);
	pkb_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
	pkb_index = ali_hcd_pkt_size2index(pkb_size)
		set_epb_packet_size(base_addr, pkb_index);

	// set epA logical ep no. 
	ep_num = usb_pipeendpoint(urb->pipe);
	set_ep_logical_num(base_addr, HEPASETR, ep_num);
	set_ep_device_address(base_addr,EPA_HUB_SET_REG,dev_addr,FALSE);

	// set epB logical ep no.
	ep_num = usb_pipeendpoint(urb->pipe1);
	set_ep_logical_num(base_addr, HEPBSETR, ep_num);
	set_ep_device_address(base_addr,EPB_HUB_SET_REG,dev_addr,FALSE);

	// read all interrupt flag first to make sure no previous state occur
	get_host_int_flag1(base_addr);
	get_host_int_flag2(base_addr);
	get_usb_intf_int_status(base_addr);

	// enable all host interrupt1 except for SOF flag
	en_host_int_flag1_all(base_addr);

	// enable all host interrupt flag2 except EPB/EPA FINISH_OK
	//en_host_int_flag2_all(base_addr);
	//mask_host_int_flag2(base_addr, B_EPA_FSHOK_IE|B_EPB_FSHOK_IE);
	en_host_int_flag2(base_addr, 0x9F);


	// mask dma rx and tx finish interrupt
	//enable_all_usb_intf_int(base_addr);
	//mask_usb_intf_int(base_addr, B_RXED_INT_DN | B_TXED_INT_DN);
	enable_usb_intf_int(base_addr, 0x06);
	// set token in by HW
	set_token_in_by_hw(base_addr);

	// enable TX and RX automation option
	enable_dma_auto_rxtx(base_addr, pka_index, pkb_index);

	// set base address with tx, rx, cmd and status
	set_dma_tx_base_addr(base_addr, (UINT32) p_upt->opt_bulk.tx_buf);
	set_dma_rx_base_addr(base_addr, (UINT32) p_upt->opt_bulk.rx_buf);
	set_bulk_cmd_base_addr(base_addr, (UINT32) p_upt->opt_bulk.cmd_buf);
	set_bulk_status_base_addr(base_addr, (UINT32) p_upt->opt_bulk.sts_buf);

	// set index offset with tx, rx, cmd and status
	set_dma_tx_index(base_addr, p_upt->opt_bulk.index_offset);
	set_dma_rx_index(base_addr, p_upt->opt_bulk.index_offset);
	set_bulk_cmd_index(base_addr, p_upt->opt_bulk.index_offset);
	set_bulk_status_index(base_addr, p_upt->opt_bulk.index_offset);

	//if(MEMCMP(0xa0400000, 0xa0500000, 512) != 0) asm("sdbbp");

	// init bulk opt FSM state
	init_bulk_opt_state(base_addr);
	start_bulk_opt_transfer(base_addr);
#endif
}
#endif

/**********************************************************
 * 	Name		:   	ali_hcd_bulk_opt_done
 *	Description	:   	process BULK opt.  finish interrupt 
 *	Parameter	:	
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_bulk_opt_done(struct ali_host *ali_host)
{

#if 0	// linux urb have no cmd queue mode
	struct urb *urb = ali_host->curr_urb_opt_bulk;
	UINT32 actual_length, trans_len;
	UINT8 size;
	int urb_status;

	if(urb == NULL)
		return;

	if (sys_ic_get_chip_id() == ALI_M3329E)
		size = 16;
	else
		size = 32;

	actual_length = read_dma_rx_index(ali_host->base_addr);

	if ((urb->transfer_buffer_length % size))
	{
		trans_len = (urb->transfer_buffer_length + (size - 1)) / size;
		trans_len *= size;
	}
	else
		trans_len = urb->transfer_buffer_length;

	if (actual_length != trans_len)
	{
		urb->actual_length = actual_length;
		urb->result = HC_EVENT_SUBTYPE_UPT_LENGTH_MIS;
		urb_status = 0;
	}
	else
	{
		urb->actual_length = p_upt->length;
		urb_status = 0;
	}

	urb_status = 0;
	finish_request(ali_host, urb,urb_status);
	urb = ali_host_get_opt_bulk_urb(ali_host);
	ali_host ->curr_urb_opt_bulk= urb;
	if(urb != NULL)
	{
		ali_hcd_opt_bulk_start(ali_host,urb)
	}
#endif
}

/**********************************************************
 * 	Name		:   	ali_hcd_dma_rx_done
 *	Description	:   	process DMA RX finish interrupt 
 *	Parameter	:	
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_dma_rx_done(struct ali_host *ali_host)
{
	UINT32 base_addr = ali_host->base_addr;
	struct urb *urb = ali_host->curr_urb_bulk_in;
	volatile UINT32 actual_len;
	int urb_status = 0;

	if(urb == NULL)
		return;

	actual_len = read_dma_rx_count(base_addr);

	if (actual_len != urb->transfer_buffer_length)
	{
		ALI_HOST_PRINTF("rlen: %d actlen: %d transfer_flags: %x \n",urb->transfer_buffer_length,actual_len,urb->transfer_flags);
		urb->actual_length = actual_len;
		if(urb->transfer_flags & URB_SHORT_NOT_OK)
		{
			urb_status =  -EREMOTEIO ;			
		}else{
			urb_status = 0;
		}
	}
	else
	{
		urb->actual_length = urb->transfer_buffer_length;
		urb_status = 0;
	}

	// clear DMA RX bit 
	init_dma_rx_state(base_addr);

	ALI_HOST_PRINTF("bulk in actual  len: %d \n",urb->actual_length);
	cache_invalid(urb->transfer_buffer,urb->transfer_buffer_length);
	// Bulk In transfer complete 
	finish_request(ali_host, urb,urb_status);
	urb = ali_host_get_bulk_in_urb(ali_host);
	ali_host ->curr_urb_bulk_in= urb;
	if(urb != NULL)
	{	if(urb->transfer_buffer_length <=512)
		{
			ali_hcd_single_bulkin_start(ali_host, urb);
		}else{
			ali_hcd_bulkin_start(ali_host,urb);
		}
	}	
	return;
}

/**********************************************************
 * 	Name		:   	hcd_s3602_dma_tx_done
 *	Description	:   	procss DMA TX finish interrupt 
 *	Parameter	:	priv: s3602 usb hcd hal private
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_dma_tx_done(struct ali_host *ali_host)
{
	struct urb* urb =ali_host->curr_urb_bulk_out;
	UINT32 base_addr = ali_host->base_addr;
	int urb_status = 0;

	if(urb == NULL)
		return;

	// delay to ensure the data have been transmitted out from host IP to bus 
	//			-> the speed of bus aoubt 4kbyte/128us
	//			-> so for 1kbyte(host's SRAM) need to spend about 32us 
	//udelay(32);

	// clear DMA TX bit
	init_dma_tx_state(base_addr);

	//trun off the DMA operation
	en_dma_transfer(base_addr, FALSE);

	urb->actual_length = urb->transfer_buffer_length;
	// Bulk Out transfer complete 
	urb_status = 0;
	finish_request(ali_host, urb,urb_status);
	urb = ali_host_get_bulk_out_urb(ali_host);
	ali_host ->curr_urb_bulk_out = urb;
	if(urb != NULL)
	{
		ali_hcd_bulkout_start(ali_host,urb);
	}	

	return;
}

/**********************************************************
 * 	Name		:   	ali_hcd_urb_parse_error
 *	Description	:   	parse upt result when a token is finished with an error 
 *	Parameter	:	
 *	Return		:	void
 *	int status 
 *	When the urb is finished, or being processed by the USB core, this variable is set to the current status of the urb. The only time a USB driver can safely access this variable is in the urb completion handler function (described in Section 13.3.4). This restriction is to prevent race conditions that occur while the urb is being processed by the USB core. For isochronous urbs, a successful value (0) in this variable merely indicates whether the urb has been unlinked. To obtain a detailed status on isochronous urbs, the iso_frame_desc variables should be checked.
 *	Valid values for this variable include:
 *	
 *	0
 *	The urb transfer was successful.
 *	
 *	-ENOENT 
 *	The urb was stopped by a call to usb_kill_urb.
 *	
 *	-ECONNRESET 
 *	The urb was unlinked by a call to usb_unlink_urb, and the TRansfer_flags variable of the urb was set to URB_ASYNC_UNLINK.
 *	
 *	-EINPROGRESS 
 *	The urb is still being processed by the USB host controllers. If your driver ever sees this value, it is a bug in your driver.
 *	
 *	-EPROTO 
 *	One of the following errors occurred with this urb:
 *	A bitstuff error happened during the transfer.
 *	No response packet was received in time by the hardware.
 *	
 *	-EILSEQ 
 *	There was a CRC mismatch in the urb transfer.
 *	
 *	-EPIPE 
 *	The endpoint is now stalled. If the endpoint involved is not a control endpoint, this error can be cleared through a call to the function usb_clear_halt.
 *	
 *	-ECOMM 
 *	Data was received faster during the transfer than it could be written to system memory. This error value happens only for an IN urb.
 *	
 *	-ENOSR 
 *	Data could not be retrieved from the system memory during the transfer fast enough to keep up with the requested USB data rate. This error value happens only for an OUT urb.
 *	
 *	-EOVERFLOW 
 *	A "babble" error happened to the urb. A "babble" error occurs when the endpoint receives more data than the endpoint's specified maximum packet size.
 *	
 *	-EREMOTEIO 
 *	Occurs only if the URB_SHORT_NOT_OK flag is set in the urb's transfer_flags variable and means that the full amount of data requested by the urb was not received.
 *	
 *	-ENODEV 
 *	The USB device is now gone from the system.
 *	
 *	-EXDEV 
 *	Occurs only for a isochronous urb and means that the transfer was only partially completed. In order to determine what was transferred, the driver must look at the individual frame status.
 *	
 *	-EINVAL 
 *	Something very bad happened with the urb. The USB kernel documentation describes what this value means:
 *	ISO madness, if this happens: Log off and go home
 *	It also can happen if a parameter is incorrectly set in the urb stucture or if an incorrect function parameter in the usb_submit_urb call submitted the urb to the USB core.
 *	
 *	-ESHUTDOWN 
 *	There was a severe error with the USB host controller driver; it has now been disabled, or the device was disconnected from the system, and the urb was submitted after the device was removed. It can also occur if the configuration was changed for the device, while the urb was submitted to the device.
 *	Generally, the error values -EPROTO, -EILSEQ, and -EOVERFLOW indicate hardware problems with the device, the device firmware, or the cable connecting the device to the computer.
 *	
 *	 int status 
 *	当这个 urb 被结束, 或者开始由 USB 核心处理, 这个变量被设置为 urb 的当前状态. 一个 USB 驱动可安全存取这个变量的唯一时间是在 urb 完成处理者函数中(在"CompletingUrbs: 完成回调处理者"一节中描述). 这个限制是阻止竞争情况, 发生在这个 urb 被 USB 核心处理当中. 对于同步 urb, 在这个变量中的一个成功的值(0)只指示是否这个 urb 已被去链. 为获得在同步 urb 上的详细状态, 应当检查 iso_frame_desc 变量.
 *	这个变量的有效值包括:
 *	0 
 *	这个 urb 传送是成功的.
 *	-ENOENT 
 *	这个 urb 被对 usb_kill_urb 的调用停止.
 *	-ECONNRESET 
 *	urb 被对 usb_unlink_urb 的调用去链, 并且 transfer_flags 变量被设置为 URB_ASYNC_UNLINK.
 *	-EINPROGRESS 
 *	这个 urb 仍然在被 USB 主机控制器处理中. 如果你的驱动曾见到这个值, 它是一个你的驱动中的 bug.
 *	-EPROTO 
*	这个 urb 发生下面一个错误:
*	一个 bitstuff 错误在传送中发生.
*	硬件没有及时收到响应帧.
*	-EILSEQ 
*	在这个 urb 传送中有一个 CRC 不匹配.
*	-EPIPE 
*	这个端点现在被停止. 如果这个包含的端点不是一个控制端点, 这个错误可被清除通过一个对函数 usb_clear_halt 的调用.
*	-ECOMM 
*	在传送中数据接收快于能被写入系统内存. 这个错误值只对 IN urb.
*	-ENOSR 
*	在传送中数据不能从系统内存中获取得足够快, 以便可跟上请求的 USB 数据速率. 这个错误只对 OUT urb.
*	-EOVERFLOW 
*	这个 urb 发生一个"babble"错误. 一个"babble"错误发生当端点接受数据多于端点的特定最大报文大小.
*	-EREMOTEIO 
*	只发生在当 URB_SHORT_NOT_OK 标志被设置在 urb 的 transfer_flags 变量, 并且意味着 urb 请求的完整数量的数据没有收到.
*	-ENODEV 
*	这个 USB 设备现在从系统中消失.
*	-EXDEV 
*	只对同步 urb 发生, 并且意味着传送只部分完成. 为了决定传送什么, 驱动必须看单独的帧状态.
*	-EINVAL 
*	这个 urb 发生了非常坏的事情. USB 内核文档描述了这个值意味着什么:
*	ISO 疯了, 如果发生这个: 退出并回家.
*	它也可发生, 如果一个参数在 urb 结构中被不正确地设置了, 或者如果在提交这个 urb 给 USB 核心的 usb_submit_urb 调用中, 有一个不正确的函数参数.
*	-ESHUTDOWN 
*	这个 USB 主机控制器驱动有严重的错误; 它现在已被禁止, 或者设备和系统去掉连接, 并且这个urb 在设备被去除后被提交. 它也可发生当这个设备的配置改变, 而这个 urb 被提交给设备.
*	通常, 错误值 -EPROTO, -EILSEQ, 和 -EOVERFLOW 指示设备的硬件问题, 设备固件, 或者连接设备到计算机的线缆. 
	***********************************************************/
static void ali_hcd_urb_parse_error(struct ali_host *ali_host,struct urb *urb)
{
	UINT8 reg;
	int urb_status = 0;

	reg = get_token_err_infor(ali_host->base_addr);
	switch (reg)
	{
		case B_STALL_ERR:
			ALI_HOST_PRINTF("B_STALL_ERR\n");
			if(ali_host ->curr_urb_bulk_out == urb)
			{
				ali_hcd_init_epa_data_toggle(ali_host->base_addr);
			}else if(ali_host ->curr_urb_bulk_in == urb)
			{
				ali_hcd_init_epb_data_toggle(ali_host->base_addr);			
			}
			urb_status= -EPIPE ; // HC_EVENT_SUBTYPE_UPT_STALL;
			break;
		case B_DEV_NOT_RSP:
			ALI_HOST_PRINTF("B_DEV_NOT_RSP\n");	
			urb_status = -EPROTO; // HC_EVENT_SUBTYPE_UPT_NAK;
			break;
		case B_DATATOG_MIS:
			ALI_HOST_PRINTF("B_DATATOG_MIS\n");
			urb_status= -EPROTO ; //HC_EVENT_SUBTYPE_UPT_DATATOG_MIS;
			break;
		case B_PID_CHK_ERR:
			ALI_HOST_PRINTF("B_PID_CHK_ERR\n");
			urb_status = -EPROTO; //HC_EVENT_SUBTYPE_UPT_FATAL_ERR;
			break;
		case B_UTMI_ERR:
			ALI_HOST_PRINTF("B_UTMI_ERR\n");	
			urb_status = -EILSEQ ; //HC_EVENT_SUBTYPE_UPT_FATAL_ERR;
			break;
		case B_CRC_ERR:
			ALI_HOST_PRINTF("B_CRC_ERR\n");	
			urb_status = -EILSEQ ; //HC_EVENT_SUBTYPE_UPT_FATAL_ERR;
			break;
		case B_UNEXPCT_PID:
			ALI_HOST_PRINTF("B_UNEXPCT_PID\n");
			urb_status = -EPROTO ; //HC_EVENT_SUBTYPE_UPT_FATAL_ERR;
			break;
		default:
			break;
	}
	finish_request(ali_host,urb,urb_status);
}

static void ali_hcd_IntInC_start(struct ali_host *ali_host, struct urb *urb)
{
	UINT8 dev_addr;
	UINT32 pkt_size;
	UINT8 pkt_index;
	UINT8 ep_num;
	UINT32 base_addr = ali_host->base_addr;
	struct usb_device*udev = urb->dev;
	struct usb_device *parent_udev = udev->parent;
	BOOL splitFlag= FALSE;
	int is_out;
	UINT32 interval,i;

	ALI_HOST_PRINTF("ali_hcd_IntInC_start, len:%d \n",urb->transfer_buffer_length);

	// set function address 

	dev_addr = (UINT8) usb_pipedevice(urb->pipe);

	if(ali_host->support_hub == 0)
	{
		set_device_address(base_addr, dev_addr);
	}else{
		if ((udev->speed !=  USB_SPEED_HIGH) &&(parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if((parent_udev->speed == USB_SPEED_HIGH)&&(parent_udev->descriptor.bDeviceClass == USB_CLASS_HUB)) 
			{	
				splitFlag=TRUE;
				set_ssplit_ctrl_info(base_addr,EPC_SPT_CSPT_CTL_REG, 0, udev->ttport, parent_udev->devnum);
			}
		}
		set_ep_device_address(base_addr,EPC_HUB_SET_REG,dev_addr,splitFlag);
	}
	// set EPB's max packet size 

	//pkt_size = usb_pipepktsize_get(urb->pipe);
	is_out = !usb_pipein(urb->pipe);
	pkt_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
	pkt_index = ali_hcd_pkt_size2index(pkt_size);
	//set_epb_packet_size(base_addr, pk_size);
	set_epc_packet_size(base_addr,  pkt_index);

	set_ep_transfer_type(base_addr, HEPCSETR, B_EPTYPE_INTR);

	// set logical ep num. 
	ep_num = usb_pipeendpoint(urb->pipe);

	set_ep_logical_num(base_addr, HEPCSETR, ep_num);

	//HCD_PROC_PRINTF("dev %d; epIn %d; size%d\n", dev_addr, ep_num, pk_size);

	// read all interrupt flag first to make sure no previous state occur
	get_host_int_flag1(base_addr);
	get_host_int_flag2(base_addr);

	get_usb_intf_int_status(base_addr);

	// Enable all interrupt of usb interface except for BULK finish int
	enable_all_usb_intf_int(base_addr);
	//mask_usb_intf_int(base_addr, B_BULK_FIN_INT_DN);

	// init EPC's fifo 
	init_ep_fifo(base_addr, HEPCFCTR);

	// enable all host interrupt flag2 except EPB/EPA FINISH_OK
	en_host_int_flag2_all(base_addr);
	//en_host_int_flag2(base_addr, (B_EPC_FSHOK_IE|B_EPC_FSHERR_IE));

	if(udev->speed ==  USB_SPEED_HIGH)
	{	//urb->interval = 1 << (ep->desc->bInterval - 1);
		i = 0;
		interval = urb->interval;
		while(interval != 0) 
		{
			interval = interval>>1;
			i++;
		}
		interval = i ;
	}else{
		interval = urb->interval;
	}
	set_int_interval_value(base_addr, HBINTERVALC, interval);
	en_int_interval_ctrl(base_addr, B_INTERVAL_C_ENABLE);

	//ep_force_token_sending(base_addr, HEPCFCTR);
}

static void ali_hcd_IntInD_start(struct ali_host *ali_host, struct urb *urb)
{
	UINT8 dev_addr;
	UINT32 pkt_size;
	UINT8 pkt_index;
	UINT8 ep_num;
	UINT32 base_addr = ali_host->base_addr;
	struct usb_device *udev = urb->dev;
	BOOL splitFlag= FALSE;
	struct usb_device *parent_udev = udev->parent;
	int is_out;
	UINT32 interval,i;

	ALI_HOST_PRINTF("ali_hcd_IntInD_start, len:%d \n",urb->transfer_buffer_length);
	// set function address 

	dev_addr = (UINT8) usb_pipedevice(urb->pipe);

	if(ali_host->support_hub == 0)
	{
		set_device_address(base_addr, dev_addr);
	}else{
		if ((udev->speed !=  USB_SPEED_HIGH) &&(parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if((parent_udev->speed == USB_SPEED_HIGH)&&(parent_udev->descriptor.bDeviceClass == USB_CLASS_HUB)) //USB_NODETYPE_HUB
			{	
				splitFlag=TRUE;
				set_ssplit_ctrl_info(base_addr,EPD_SPT_CSPT_CTL_REG, 0, udev->ttport, parent_udev->devnum);
			}
		}
		set_ep_device_address(base_addr,EPD_HUB_SET_REG,dev_addr,splitFlag);
	}
	// pkt_size = usb_pipepktsize_get(urb->pipe);
	is_out = !usb_pipein(urb->pipe);
	pkt_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
	pkt_index = ali_hcd_pkt_size2index(pkt_size);
	set_epd_packet_size(base_addr,  pkt_index);
	set_ep_transfer_type(base_addr, HEPDSETR, B_EPTYPE_INTR);

	// set logical ep num. 
	ep_num = usb_pipeendpoint(urb->pipe);

	set_ep_logical_num(base_addr, HEPDSETR, ep_num);

	//HCD_PROC_PRINTF("dev %d; epIn %d; size%d\n", dev_addr, ep_num, pk_size);

	// read all interrupt flag first to make sure no previous state occur 
	get_host_int_flag1(base_addr);
	get_host_int_flag2(base_addr);
	get_host_int_flag3(base_addr);

	get_usb_intf_int_status(base_addr);

	// Enable all interrupt of usb interface except for BULK finish int
	enable_all_usb_intf_int(base_addr);
	//mask_usb_intf_int(base_addr, B_BULK_FIN_INT_DN);

	// init EPD's fifo 
	init_ep_fifo(base_addr, HEPDFCTR);

	// enable all host interrupt flag2 except EPB/EPA FINISH_OK 
	en_host_int_flag3_all(base_addr);

	if(udev->speed ==  USB_SPEED_HIGH)
	{	//urb->interval = 1 << (ep->desc->bInterval - 1);
		i = 0;
		interval = urb->interval;
		while(interval != 0) 
		{
			interval = interval>>1;
			i++;
		}
		interval = i ;
	}else{
		interval = urb->interval;
	}
	set_int_interval_value(base_addr, HBINTERVALD, interval);
	en_int_interval_ctrl(base_addr, B_INTERVAL_D_ENABLE);


	//ep_force_token_sending(base_addr, HEPCFCTR);
}

static void ali_hcd_IntInE_start(struct ali_host *ali_host, struct urb *urb)
{
	UINT8 dev_addr;
	UINT32 pkt_size;
	UINT8 pkt_index;
	UINT8 ep_num;
	UINT32 base_addr = ali_host->base_addr;
	struct usb_device *udev = urb->dev;
	BOOL splitFlag= FALSE;
	struct usb_device *parent_udev = udev->parent;
	int is_out;
	UINT32 interval,i;

	ALI_HOST_PRINTF("ali_hcd_IntInE_start, len:%d \n",urb->transfer_buffer_length);
	// set function address 

	dev_addr = (UINT8) usb_pipedevice(urb->pipe);

	if(ali_host->support_hub == 0)
	{
		set_device_address(base_addr, dev_addr);
	}else{
		if ((udev->speed !=  USB_SPEED_HIGH) &&(parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if((parent_udev->speed == USB_SPEED_HIGH)&&(parent_udev->descriptor.bDeviceClass == USB_CLASS_HUB)) //USB_NODETYPE_HUB
			{	
				splitFlag=TRUE;
				set_ssplit_ctrl_info(base_addr,EPE_SPT_CSPT_CTL_REG, 0, udev->ttport, parent_udev->devnum);
			}
		}
		set_ep_device_address(base_addr,EPE_HUB_SET_REG,dev_addr,splitFlag);
	}
	// pkt_size = usb_pipepktsize_get(urb->pipe);
	is_out = !usb_pipein(urb->pipe);
	pkt_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
	pkt_index = ali_hcd_pkt_size2index(pkt_size);
	set_epd_packet_size(base_addr,  pkt_index);
	set_ep_transfer_type(base_addr, HEPESETR, B_EPTYPE_INTR);

	// set logical ep num. 
	ep_num = usb_pipeendpoint(urb->pipe);

	set_ep_logical_num(base_addr, HEPESETR, ep_num);

	//HCD_PROC_PRINTF("dev %d; epIn %d; size%d\n", dev_addr, ep_num, pk_size);

	// read all interrupt flag first to make sure no previous state occur 
	get_host_int_flag1(base_addr);
	get_host_int_flag2(base_addr);
	get_host_int_flag3(base_addr);

	get_usb_intf_int_status(base_addr);

	// Enable all interrupt of usb interface except for BULK finish int
	enable_all_usb_intf_int(base_addr);
	//mask_usb_intf_int(base_addr, B_BULK_FIN_INT_DN);

	// init EPE's fifo 
	init_ep_fifo(base_addr, HEPEFCTR);

	// enable all host interrupt flag2 except EPB/EPA FINISH_OK 
	en_host_int_flag3_all(base_addr);

	if(udev->speed ==  USB_SPEED_HIGH)
	{	//urb->interval = 1 << (ep->desc->bInterval - 1);
		i = 0;
		interval = urb->interval;
		while(interval != 0) 
		{
			interval = interval>>1;
			i++;
		}
		interval = i ;
	}else{
		interval = urb->interval;
	}
	set_int_interval_value(base_addr, HBINTERVALE, interval);
	en_int_interval_ctrl(base_addr, B_INTERVAL_E_ENABLE);


	//ep_force_token_sending(base_addr, HEPCFCTR);
}

static void ali_hcd_IntInF_start(struct ali_host *ali_host, struct urb *urb)
{
	UINT8 dev_addr;
	UINT32 pkt_size;
	UINT8 pkt_index;
	UINT8 ep_num;
	UINT32 base_addr = ali_host->base_addr;
	struct usb_device *udev = urb->dev;
	BOOL splitFlag= FALSE;
	struct usb_device *parent_udev = udev->parent;
	int is_out;
	UINT32 interval,i;

	ALI_HOST_PRINTF("ali_hcd_IntInF_start, len:%d \n",urb->transfer_buffer_length);
	// set function address 

	dev_addr = (UINT8) usb_pipedevice(urb->pipe);

	if(ali_host->support_hub == 0)
	{
		set_device_address(base_addr, dev_addr);
	}else{
		if ((udev->speed !=  USB_SPEED_HIGH) &&(parent_udev != NULL)&&(parent_udev->devnum != 1))		// addr:1 virtual root hub
		{
			if((parent_udev->speed == USB_SPEED_HIGH)&&(parent_udev->descriptor.bDeviceClass == USB_CLASS_HUB)) //USB_NODETYPE_HUB
			{	
				splitFlag=TRUE;
				set_ssplit_ctrl_info(base_addr,EPF_SPT_CSPT_CTL_REG, 0, udev->ttport, parent_udev->devnum);
			}
		}
		set_ep_device_address(base_addr,EPF_HUB_SET_REG,dev_addr,splitFlag);
	}
	// pkt_size = usb_pipepktsize_get(urb->pipe);
	is_out = !usb_pipein(urb->pipe);
	pkt_size = usb_maxpacket(urb->dev,urb->pipe,is_out);
	pkt_index = ali_hcd_pkt_size2index(pkt_size);
	set_epd_packet_size(base_addr,  pkt_index);
	set_ep_transfer_type(base_addr, HEPFSETR, B_EPTYPE_INTR);

	// set logical ep num. 
	ep_num = usb_pipeendpoint(urb->pipe);

	set_ep_logical_num(base_addr, HEPFSETR, ep_num);

	//HCD_PROC_PRINTF("dev %d; epIn %d; size%d\n", dev_addr, ep_num, pk_size);

	// read all interrupt flag first to make sure no previous state occur 
	get_host_int_flag1(base_addr);
	get_host_int_flag2(base_addr);
	get_host_int_flag3(base_addr);

	get_usb_intf_int_status(base_addr);

	// Enable all interrupt of usb interface except for BULK finish int
	enable_all_usb_intf_int(base_addr);
	//mask_usb_intf_int(base_addr, B_BULK_FIN_INT_DN);

	// init EPF's fifo 
	init_ep_fifo(base_addr, HEPFFCTR);

	// enable all host interrupt flag2 except EPB/EPA FINISH_OK 
	en_host_int_flag3_all(base_addr);

	if(udev->speed ==  USB_SPEED_HIGH)
	{	//urb->interval = 1 << (ep->desc->bInterval - 1);
		i = 0;
		interval = urb->interval;
		while(interval != 0) 
		{
			interval = interval>>1;
			i++;
		}
		interval = i;
	}else{
		interval = urb->interval;
	}
	set_int_interval_value(base_addr, HBINTERVALF, interval);
	en_int_interval_ctrl(base_addr, B_INTERVAL_F_ENABLE);


	//ep_force_token_sending(base_addr, HEPCFCTR);
}

/**********************************************************
 * 	Name		:   	hcd_s3602_epA_finish_ok
 *	Description	:   	procss EPA transaction finish ok 
 *	Parameter	:	priv: s3602 usb hcd hal private
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_epA_finish_ok(struct ali_host *ali_host)
{
	int urb_status;
	struct urb* urb = NULL;
	struct ali_host_ep *ep = NULL;

	urb = ali_host->curr_urb_bulk_out;
	if(urb == NULL)
		return;
	ep = urb->hcpriv;
	if(ep == NULL)
		return;

	if (ep->bulk_type == BULK_OUT_SINGLE)
	{
		ALI_HOST_PRINTF("epA finish OK occur unexpectedly 1!\n");
		// single bulk out success
		urb->actual_length = urb->transfer_buffer_length;
		urb_status = 0;
		finish_request(ali_host, urb,urb_status);
		urb = ali_host_get_bulk_out_urb(ali_host);
		ali_host ->curr_urb_bulk_out = urb;
		if(urb != NULL)
		{
			ali_hcd_bulkout_start(ali_host,urb);
		}
	}
	return;
}

/**********************************************************
 * 	Name		:   	hcd_s3602_epB_finish_ok
 *	Description	:   	procss EPB transaction finish ok 
 *	Parameter	:	priv: s3602 usb hcd hal private
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_epB_finish_ok(struct ali_host *ali_host)
{
	UINT16 epb_trans_cnt = 0;
	UINT32 base_addr = ali_host->base_addr;
	struct urb *urb = NULL;
	struct ali_host_ep *ep = NULL;

	urb = ali_host->curr_urb_bulk_in;
	if(urb == NULL)
		return;
	ep = urb->hcpriv;
	if(ep == NULL)
		return;

	if (ep->bulk_type == BULK_IN_SINGLE)	
	{
		if(ali_host->soc_chip == ALI_CHIPID_S3603)
			epb_trans_cnt = get_epb_last_data_size_3603(base_addr);
		else
			epb_trans_cnt = get_epb_last_data_size(base_addr);

		set_dma_rx_cnt(base_addr, epb_trans_cnt);		// set dma rx count 
		// start a DMA bulk In transfer 
		init_dma_rx_state(base_addr);			// Init DMA RX FSM status
		start_dma_rx_transfer(base_addr);		// Set USB_INTF DMA_RX_START
		//dma_host_in_token(TRUE);	// Set HOST IP Token IN 
		en_dma_transfer(base_addr, TRUE);		// and DMA Enable to transfer 
		return;
	}
}

static void ali_hcd_epC_finish_ok(struct ali_host *ali_host)
{
	UINT32 i = 0;
	int urb_status = 0;
	struct usb_device *udev = NULL;
	UINT32 base_addr = ali_host->base_addr;
	struct urb *urb = ali_host->curr_urb_int_c;

	if(urb == NULL)
		return;

	udev = urb->dev;

	set_int_interval_value(base_addr, HBINTERVALC, 0x0);
	mask_int_interval_ctrl(base_addr, B_INTERVAL_C_ENABLE);

	ALI_HOST_PRINTF("\n EPC get data:" );
	// get contorl IN data from ep0's fifo to upt' buffer
	while (get_ep_fifo_empty_status(base_addr, HEPCFCTR) == FALSE)
	{
		// data length in fifo more than request length 
		if (i == urb->transfer_buffer_length)
		{
			break;
		}
		rw_ep_fifo_port(base_addr, HEPCFIFO, FALSE,
				((UINT8 *) (urb->transfer_buffer) + i));
		ALI_HOST_PRINTF(" %02x", *((UINT8*)(urb->transfer_buffer) + i));
		i++;
	}
	ALI_HOST_PRINTF("\n" );

	//disable interrupt endpoint poll

	urb->actual_length = i;	
	// Reset  Parameter to  default

	ali_host->curr_urb_int_c = NULL;

	urb_status = 0;
	finish_request(ali_host, urb,urb_status);
	urb = ali_host_get_int_c_urb(ali_host);
	ali_host ->curr_urb_int_c = urb;
	if(urb != NULL)
	{
		ali_hcd_IntInC_start(ali_host,urb);
	}	
}

static void ali_hcd_epD_finish_ok(struct ali_host *ali_host)
{
	UINT32 base_addr = ali_host->base_addr;
	UINT32 i = 0;
	struct urb *urb = NULL;
	int urb_status = 0;

	urb = ali_host->curr_urb_int_d;
	if(NULL == urb)
		return;

	set_int_interval_value(base_addr, HBINTERVALD, 0x0);
	mask_int_interval_ctrl(base_addr, B_INTERVAL_D_ENABLE);

	ALI_HOST_PRINTF("\n EPD get data:" );
	// get contorl IN data from ep0's fifo to upt' buffer 
	while (get_ep_fifo_empty_status(base_addr, HEPDFCTR) == FALSE)
	{
		// data length in fifo more than request length 
		if (i == urb->transfer_buffer_length)
		{
			break;
		}
		rw_ep_fifo_port(base_addr, HEPDFIFO, FALSE,((UINT8 *) (urb->transfer_buffer) + i));
		ALI_HOST_PRINTF(" %02x", *((UINT8*)(urb->transfer_buffer) + i));
		i++;
	}
	ALI_HOST_PRINTF("\n" );

	//disable interrupt endpoint poll

	urb->actual_length = i;		
	urb_status = 0;
	finish_request(ali_host, urb,urb_status);
	urb = ali_host_get_int_d_urb(ali_host);
	ali_host ->curr_urb_int_d= urb;
	if(urb != NULL)
	{
		ali_hcd_IntInD_start(ali_host,urb);
	}	
}

static void ali_hcd_epE_finish_ok(struct ali_host *ali_host)
{
	UINT32 base_addr = ali_host->base_addr;
	UINT32 i = 0;
	struct urb *urb = NULL;
	int urb_status = 0;

	urb = ali_host->curr_urb_int_d;
	if(NULL == urb)
		return;

	set_int_interval_value(base_addr, HBINTERVALE, 0x0);
	mask_int_interval_ctrl(base_addr, B_INTERVAL_E_ENABLE);

	ALI_HOST_PRINTF("\n EPE get data:" );
	// get contorl IN data from ep0's fifo to upt' buffer 
	while (get_ep_fifo_empty_status(base_addr, HEPEFCTR) == FALSE)
	{
		// data length in fifo more than request length 
		if (i == urb->transfer_buffer_length)
		{
			break;
		}
		rw_ep_fifo_port(base_addr, HEPEFIFO, FALSE,((UINT8 *) (urb->transfer_buffer) + i));
		ALI_HOST_PRINTF(" %02x", *((UINT8*)(urb->transfer_buffer) + i));
		i++;
	}
	ALI_HOST_PRINTF("\n" );

	//disable interrupt endpoint poll

	urb->actual_length = i;		
	urb_status = 0;
	finish_request(ali_host, urb,urb_status);
	urb = ali_host_get_int_e_urb(ali_host);
	ali_host ->curr_urb_int_d= urb;
	if(urb != NULL)
	{
		ali_hcd_IntInE_start(ali_host,urb);
	}	
}

static void ali_hcd_epF_finish_ok(struct ali_host *ali_host)
{
	UINT32 base_addr = ali_host->base_addr;
	UINT32 i = 0;
	struct urb *urb = NULL;
	int urb_status = 0;

	urb = ali_host->curr_urb_int_f;
	if(NULL == urb)
		return;

	set_int_interval_value(base_addr, HBINTERVALF, 0x0);
	mask_int_interval_ctrl(base_addr, B_INTERVAL_F_ENABLE);

	ALI_HOST_PRINTF("\n EPF get data:" );
	// get contorl IN data from ep0's fifo to upt' buffer 
	while (get_ep_fifo_empty_status(base_addr, HEPFFCTR) == FALSE)
	{
		// data length in fifo more than request length 
		if (i == urb->transfer_buffer_length)
		{
			break;
		}
		rw_ep_fifo_port(base_addr, HEPFFIFO, FALSE,((UINT8 *) (urb->transfer_buffer) + i));
		ALI_HOST_PRINTF(" %02x", *((UINT8*)(urb->transfer_buffer) + i));
		i++;
	}
	ALI_HOST_PRINTF("\n" );

	//disable interrupt endpoint poll

	urb->actual_length = i;		
	urb_status = 0;
	finish_request(ali_host, urb,urb_status);
	urb = ali_host_get_int_f_urb(ali_host);
	ali_host ->curr_urb_int_d= urb;
	if(urb != NULL)
	{
		ali_hcd_IntInF_start(ali_host,urb);
	}	
}

/**********************************************************
 * 	Name		:   	hcd_s3602_h2c_int
 *	Description	:   	procss host to cpu interrupt 
 *	Parameter	:	hc_dev: hc s3602 device
 *					flag1: host int flag 1
 *					flag2: host int flag 2
 *	Return		:	void
 ***********************************************************/
static void ali_hcd_h2c_int(struct ali_host *ali_host)
{	
	UINT32 speed;
	struct urb *urb = NULL;
	struct ali_host_ep* ep = NULL;	
	UINT32 base_addr = ali_host->base_addr;
	UINT8 flag1 = ali_host->host_int_flag1;
	UINT8 flag2 = ali_host->host_int_flag2;
	UINT8 flag3 = ali_host->host_int_flag3;

	// check interrupt flag1 
	if (flag1 & B_BABBLE_INT)
	{
		// Babble 
		ali_host->host_int_flag1 &= ~B_BABBLE_INT;

		ali_host->usb_bus_err = HCD_S3602_BUS_ERR;
		ALI_HOST_PRINTF("usb bus babble error!\n");

		ali_hcd_all_urb_err(ali_host);
		//hcd_s3602_sw_init(priv);
		//hcd_s3602_hw_init();
		return;
	}
	// Device Connection/Disconnection
	else if (flag1 & B_CONN_DSCN_INT)
	{
		ali_host->host_int_flag1 &= ~B_CONN_DSCN_INT;
		ali_host->port_changed = 1;
		/*		if (get_port_connect_status(ali_host->base_addr) == TRUE)
				{
		// notify USBD that ATTACH event 
		ALI_HOST_PRINTF("HC_EVENT_DYNAMIC_DEV_ATTACHED!\n");
		//notify.event_sub_type = HC_EVENT_DYNAMIC_DEV_ATTACHED;
		usbd_notify_dynamic_attached(ali_host);
		}
		else
		{
		// notify USBD that DETTACH event 
		ali_hcd_all_urb_err(ali_host);
		ali_hcd_sw_init(ali_host);
		ali_hcd_hw_init(ali_host);
		ALI_HOST_PRINTF("HC_EVENT_DYNAMIC_UPT_DEV_REMOVE!\n")
		usbd_notify_dynamic_removerd(ali_host);
		}
		 */		return;
	}
	// Device No Response
	else if (flag1 & B_DEV_NO_RESP_INT)
	{
		ali_host->host_int_flag1 &= ~B_DEV_NO_RESP_INT;
		ALI_HOST_PRINTF("Device No Response!\n");
	}
	else if (flag1 & B_USB_RST_INT)
	{
		ali_host->host_int_flag1 &= ~B_USB_RST_INT;

		speed = get_device_speed(ali_host->base_addr);
		if (speed & B_OPER_FS)
		{
			speed = USB_SPEED_FULL;
			ALI_HOST_PRINTF("ISR-Device is in FS mode!\n");
		}
		else if (speed & B_OPER_LS)
		{
			speed = USB_SPEED_LOW;
			ALI_HOST_PRINTF("ISR-Device is in LS mode!\n");
		}
		else
		{
			speed = USB_SPEED_HIGH;
			ALI_HOST_PRINTF("ISR-Device is in HS mode!\n");
		}
		//ali_host->hc_notify_uplayer(notify, 0,(UINT32)ali_host);
		return;
	}

	//Host suspend/resume finish
	if (flag1 & B_H_SUSREM_INT)
	{
		ali_host->host_int_flag1 &= ~B_H_SUSREM_INT;
		//ali_host->hc_notify_uplayer(notify, 0,(UINT32)ali_host);
		return;
	}

	// check interrupt flag2 
	// EP0 finish ERROR
	if (flag2 & B_EP0_FSHERR_INT)
	{
		ali_host->host_int_flag2 &= ~B_EP0_FSHERR_INT;
		ALI_HOST_PRINTF("EP0_FSHERR\n");
		urb = ali_host->curr_urb_ctrl;
		if(urb != NULL)
			ali_hcd_urb_parse_error(ali_host, urb);
	}	
	// EP0 finish OK
	if (flag2 & B_EP0_FSHOK_INT)
	{
		ali_host->host_int_flag2 &= ~B_EP0_FSHOK_INT;
		urb = ali_host->curr_urb_ctrl;
		if(urb != NULL)
		{
			ep = urb->hcpriv;
			if(ep != NULL)
			{
				if(ep->ctrl_transfer_stage == HCD_CTRL_STAGE_SETUP)
				{
					proc_ctrl_setup_done(ali_host);
				}
				else if(ep->ctrl_transfer_stage == HCD_CTRL_STAGE_DATA)
				{
					proc_ctrl_data_done(ali_host);
				}
				else if(ep->ctrl_transfer_stage == HCD_CTRL_STAGE_STATUS)
				{
					proc_ctrl_status_done(ali_host);
				}
				else 
				{
					ALI_HOST_PRINTF("control transfer stage err, stage:%d\n", ep->ctrl_transfer_stage);
				}
			}
		}
	}
	// EPA finish OK 
	if (flag2 & B_EPA_FSHOK_INT)
	{
		ali_host->host_int_flag2 &= ~B_EPA_FSHOK_INT;
		ali_hcd_epA_finish_ok(ali_host);
	}
	// EPB finish OK
	if (flag2 & B_EPB_FSHOK_INT)
	{
		ali_host->host_int_flag2 &= ~B_EPB_FSHOK_INT;
		ali_hcd_epB_finish_ok(ali_host);
	}
	// EPC finish OK
	if (flag2 & B_EPC_FSHOK_INT)
	{
		ali_host->host_int_flag2 &= ~B_EPC_FSHOK_INT;
		mask_host_int_flag2(base_addr, B_EPC_FSHOK_IE);
		ali_hcd_epC_finish_ok(ali_host);
	}
	// EPD finish OK
	if (flag3 & B_EPD_FSHOK_INT)
	{
		ali_host->host_int_flag3 &= ~B_EPD_FSHOK_INT;
		ali_hcd_epD_finish_ok(ali_host);
	}
	// EPE finish OK
	if (flag3 & B_EPE_FSHOK_INT)
	{
		ali_host->host_int_flag3 &= ~B_EPE_FSHOK_INT;
		ali_hcd_epE_finish_ok(ali_host);
	}
	// EPF finish OK
	if (flag3 & B_EPF_FSHOK_INT)
	{
		ali_host->host_int_flag3 &= ~B_EPF_FSHOK_INT;
		ali_hcd_epF_finish_ok(ali_host);
	}

	// EPA finish ERROR
	if (flag2 & B_EPA_FSHERR_INT)
	{
		ali_host->host_int_flag2 &= ~B_EPA_FSHERR_INT;
		ALI_HOST_PRINTF("B_EPA_FSHERR_INT\n");
		urb= ali_host->curr_urb_bulk_out;
		if(urb != NULL)
		{
			ali_hcd_urb_parse_error(ali_host, urb);
			if(ali_host->support_hub == 1)
				epa_error_handler(base_addr);
			urb = ali_host_get_bulk_out_urb(ali_host);
			if(urb != NULL)
			{
				ali_hcd_bulkout_start(ali_host, urb);
			}
		}
	}
	// EPB finish ERROR
	if (flag2 & B_EPB_FSHERR_INT)
	{
		ALI_HOST_PRINTF("B_EPB_FSHERR_INT\n");
		ali_host->host_int_flag2 &= ~B_EPB_FSHERR_INT;
		urb= ali_host->curr_urb_bulk_in;
		if(urb != NULL)
		{
			ali_hcd_urb_parse_error(ali_host, urb);
			if(ali_host->support_hub == 1)
				epb_error_handler(base_addr);
			urb = ali_host_get_bulk_in_urb(ali_host);
			if(urb != NULL)
			{
				ali_hcd_bulkin_start(ali_host, urb);
			}
		}
	}
	// EPC finish ERROR
	if (flag2 & B_EPC_FSHERR_INT)
	{
		ALI_HOST_PRINTF("B_EPC_FSHERR_INT\n");
		ali_host->host_int_flag2 &= ~B_EPC_FSHERR_INT;
		urb= ali_host->curr_urb_int_c;
		if(urb != NULL)
		{
			ali_hcd_urb_parse_error(ali_host, urb);
		}
	}
	// EPD finish ERROR
	if (flag3 & B_EPD_FSHERR_INT)
	{
		ALI_HOST_PRINTF("B_EPD_FSHERR_INT\n");
		ali_host->host_int_flag3 &= ~B_EPD_FSHERR_INT;
		urb= ali_host->curr_urb_int_d;
		if(urb != NULL)
		{
			ali_hcd_urb_parse_error(ali_host, urb);
		}
	}
	// EPE finish ERROR
	if (flag3 & B_EPE_FSHERR_INT)
	{
		ALI_HOST_PRINTF("B_EPE_FSHERR_INT\n");
		ali_host->host_int_flag3 &= ~B_EPE_FSHERR_INT;
		urb= ali_host->curr_urb_int_e;
		if(urb != NULL)
		{
			ali_hcd_urb_parse_error(ali_host, urb);
		}
	}
	// EPF finish ERROR
	if (flag3 & B_EPF_FSHERR_INT)
	{
		ALI_HOST_PRINTF("B_EPE_FSHERR_INT\n");
		ali_host->host_int_flag3 &= ~B_EPF_FSHERR_INT;
		urb= ali_host->curr_urb_int_f;
		if(urb != NULL)
		{
			ali_hcd_urb_parse_error(ali_host, urb);
		}
	}
}

/**********************************************************
 * 	Name		:   	hcd_ioctrl_get_speed
 *	Description	:   	s3602 get usb device speed
 *	Parameter	:	void
 *	Return		:	UINT32: speed type
 ***********************************************************/
#if 0
static UINT32 ali_hcd_ioctrl_get_speed(struct ali_host* ali_host)
{
	UINT8 speed;
	speed = get_device_speed(ali_host->base_addr);
	if (speed == B_OPER_HS)
	{
		ALI_HOST_PRINTF("USB_SPEED_HIGH\n");
		return USB_SPEED_HIGH;
	}
	else if (speed == B_OPER_FS)
	{
		ALI_HOST_PRINTF("USB_SPEED_FULL\n");
		return USB_SPEED_FULL;
	}
	else if (speed == B_OPER_LS)
	{
		ALI_HOST_PRINTF("USB_SPEED_LOW\n");
		return USB_SPEED_LOW;
	}
	else
	{
		ALI_HOST_PRINTF("USB_SPEED_UNKNOWN\n");
		return USB_SPEED_UNKNOWN;
	}
}

/**********************************************************
 * 	Name		:   	hcd_ioctrl_get_port_connect
 *	Description	:   	s3602 get port connection
 *	Parameter	:	void
 *	Return		:	BOOL : 
 *					TRUE->port connect; FALSE->port disconnect
 ***********************************************************/
static BOOL ali_hcd_ioctrl_get_port_connect(struct ali_host* ali_host)
{
	return get_port_connect_status(ali_host->base_addr);
}

/**********************************************************
 * 	Name		:   	ali_hcd_ioctrl_get_bulkopt_index
 *	Description	:   	get index of the bulk opt
 *	Parameter	:	void
 *	Return		:	UINT32: return the index number
 ***********************************************************/
static UINT32 ali_hcd_ioctrl_get_bulkopt_index(struct ali_host* ali_host)
{
	return read_bulk_status_index(ali_host->base_addr);
}

//offset 0x10 bit 1
static BOOL ali_hcd_get_port_connect_status(UINT32 addr)
{
	return get_port_connect_status(addr);
}

static void ali_hcd_init_ep0_fifo(UINT32 addr)
{
	init_ep0_fifo(addr);
}
#endif

static void ali_hcd_init_data_toggle(UINT32 base_addr)
{
	UINT8 intervalctrl_save;
	ALI_HOST_PRINTF("ali_hcd_init_data_toggle!\n");
	intervalctrl_save = hc_ReadByte(base_addr,HBINTERVALCTRL);
	hc_WriteByte(base_addr, HBINTERVALCTRL, 0x00);
	set_ep_data_toggle(base_addr, HEPAFCTR);
	set_ep_data_toggle(base_addr, HEPBFCTR);
	hc_WriteByte(base_addr, HBINTERVALCTRL, intervalctrl_save);
}

static void ali_hcd_init_epa_data_toggle(UINT32 base_addr)
{
	UINT8 intervalctrl_save;
	ALI_HOST_PRINTF("ali_hcd_init_EPA_data_toggle!\n");
	intervalctrl_save = hc_ReadByte(base_addr,HBINTERVALCTRL);
	hc_WriteByte(base_addr, HBINTERVALCTRL, 0x00);
	set_ep_data_toggle(base_addr, HEPBFCTR);
	hc_WriteByte(base_addr, HBINTERVALCTRL, intervalctrl_save);
}

static void ali_hcd_init_epb_data_toggle(UINT32 base_addr)
{
	UINT8 intervalctrl_save;
	ALI_HOST_PRINTF("ali_hcd_init_EPB_data_toggle!\n");
	intervalctrl_save = hc_ReadByte(base_addr,HBINTERVALCTRL);
	hc_WriteByte(base_addr, HBINTERVALCTRL, 0x00);
	set_ep_data_toggle(base_addr, HEPBFCTR);
	hc_WriteByte(base_addr, HBINTERVALCTRL, intervalctrl_save);
}

#if 0
static void init_epa_fifo(UINT32 addr)
{
	init_ep_fifo(addr, HEPAFCTR);;
}

static void init_epb_fifo(UINT32 addr)
{
	init_ep_fifo(addr, HEPBFCTR);
}

static INT32 poll_epc(struct ali_host* ali_host, UINT8* buff)
{
	UINT32 base_addr;
	INT32  ret = !SUCCESS;
	UINT32 i = 0;

	base_addr = ali_host->base_addr;
	//ALI_HOST_PRINTF("\n EPC get data:" );
	// get contorl IN data from ep0's fifo to upt' buffer 
	while (get_ep_fifo_empty_status(base_addr, HEPCFCTR) ==FALSE)
	{
		// data length in fifo more than request length 
		if (i >= 4)
		{
			break;
		}
		rw_ep_fifo_port(base_addr, HEPCFIFO, FALSE,&buff[i]);
		//ALI_HOST_PRINTF(" %02x", *((UINT8*)(p_upt->buffer) + i));
		i++;
	}
	if( i != 0 )
		ret = SUCCESS;
	return ret;
}
#endif

#define TIMEOUT_VALUE 0x1000
UINT8 ali_hcd_wait_txfifo_is_empty(UINT32 base_addr)
{
	UINT8 epa_fifo_empty;
	UINT32 TimerOut = 0;

	do
	{
		epa_fifo_empty = get_ep_fifo_all_empty_status(base_addr, HEPAFCTR) ;	
		if (epa_fifo_empty == TRUE)
			break;
		if(!get_port_connect_status(base_addr))
		{
			ALI_HOST_PRINTF("Device removed while wait tx fifo empty\n");
			return FALSE ;
		}
		udelay(1);
		TimerOut++;
	}while (TimerOut < TIMEOUT_VALUE);

	if (TimerOut >= TIMEOUT_VALUE)
	{
		ALI_HOST_PRINTF("wait tx fifo empty time out\n");
		return FALSE ;
	}
	else
		return TRUE ;
}


//offset 0x33 bit3
void ali_hcd_software_rst_usbip(UINT32 addr)
{
	software_rst_usbip(addr);
}

void ali_hcd_clear_dma_rx_cnt(UINT32 addr)
{
	set_dma_rx_cnt(addr,0);
}

static int ali_hcd_get_connect_stably(struct ali_host *ali_host, UINT32 *port_status)
{	
	UINT32 count,stably_count;
	UINT32 connect_status,connect_status_last;

	count = 0;
	stably_count = 0;
	while(1)
	{
		connect_status = get_port_connect_status(ali_host->base_addr);
		if(connect_status == connect_status_last)
		{
			stably_count++;
		}else{
			connect_status_last = connect_status;
			stably_count = 0;
		}
		msleep(1);
		if(stably_count > 10)
			break;
		if(count ++ > 200)
		{
			ALI_HOST_PRINTF("usb device connect on the port not stably !\n");
			return 1;
			break;
		}
	}
	*port_status = connect_status;
	return 0;
}

static void usb_dma_intf_reset( UINT32 base_addr )
{
	UINT32 USB_DMA_REG;

	if ( HC_S3329E5_BASE_ADDR == base_addr )
	{
		USB_DMA_REG = 0xb800007E;
		hc_WriteByte(USB_DMA_REG, 0, (hc_ReadByte(USB_DMA_REG, 0) | 0x40));
		udelay(10);
		hc_WriteByte(USB_DMA_REG, 0, (hc_ReadByte(USB_DMA_REG, 0) & (~0x40)));
	}
	else if (HC_S3101E_BASE_ADDR == base_addr )
	{
		USB_DMA_REG = 0xb80000e2;
		hc_WriteByte(USB_DMA_REG, 0, (hc_ReadByte(USB_DMA_REG, 0) | 0x40));
		udelay(10);
		hc_WriteByte(USB_DMA_REG, 0, (hc_ReadByte(USB_DMA_REG, 0) & (~0x40)));
	}
	else if ( (HC_S3603_BASE_ADDR == base_addr) || (HC_S3603_BASE_ADDR1 == base_addr) )
	{
		hc_WriteByte(base_addr, 0x60, (hc_ReadByte(base_addr, 0x60) | 0x22));
	}
}

static void epa_error_handler(UINT32 base_addr)
{
	mask_usb_intf_int(base_addr, 0x2);
	usb_dma_intf_reset(base_addr);
	init_ep_fifo(base_addr, HEPAFCTR);
	en_dma_transfer(base_addr, FALSE);
	enable_all_usb_intf_int(base_addr);
}

static void epb_error_handler(UINT32 base_addr)
{
	mask_usb_intf_int(base_addr, 0x2);
	usb_dma_intf_reset(base_addr);
	// EPB type is Bulk IN, fifo type is ping-pong 
	init_ep_fifo(base_addr, HEPBFCTR);
	en_dma_transfer(base_addr, FALSE);
	set_dma_rx_cnt(base_addr, 0);
	enable_all_usb_intf_int(base_addr);
}

static void ali_hcd_all_urb_err(struct ali_host * ali_host)
{
	struct urb * urb = NULL;

	urb = ali_host->curr_urb_ctrl;
	if (urb != NULL)
		ali_hcd_urb_parse_error(ali_host,urb);
	urb = ali_host->curr_urb_bulk_in;
	if (urb != NULL)
		ali_hcd_urb_parse_error(ali_host,urb);
	urb = ali_host->curr_urb_bulk_out;
	if (urb != NULL)
		ali_hcd_urb_parse_error(ali_host,urb);
	urb = ali_host->curr_urb_int_c;
	if (urb != NULL)
		ali_hcd_urb_parse_error(ali_host,urb);
	urb = ali_host->curr_urb_int_d;
	if (urb != NULL)
		ali_hcd_urb_parse_error(ali_host,urb);
	urb = ali_host->curr_urb_int_e;
	if (urb != NULL)
		ali_hcd_urb_parse_error(ali_host,urb);
	urb = ali_host->curr_urb_int_f;
	if (urb != NULL)
		ali_hcd_urb_parse_error(ali_host,urb);
	return;
}

static inline struct urb *ali_host_get_ctrl_urb(struct ali_host *ali_host)
{
	struct ali_host_ep *ep = NULL;
	struct urb *urb = NULL;

	if (unlikely(list_empty(&ali_host->pipe_ctrl)))
		return NULL;
	ep = list_entry(ali_host->pipe_ctrl.next , struct ali_host_ep, queue);
	urb = ep->urb;
	return urb;
}

static inline struct urb *ali_host_get_bulk_in_urb(struct ali_host *ali_host)
{
	struct ali_host_ep *ep = NULL;
	struct urb *urb = NULL;

	if (unlikely(list_empty(&ali_host->pipe_bulk_in)))
		return NULL;
	ep = list_entry(ali_host->pipe_bulk_in.next , struct ali_host_ep, queue);
	urb= list_entry(ep->hep->urb_list.next, struct urb, urb_list);
	return urb;
}

static inline struct urb *ali_host_get_bulk_out_urb(struct ali_host *ali_host)
{
	struct ali_host_ep *ep = NULL;
	struct urb *urb = NULL;

	if (unlikely(list_empty(&ali_host->pipe_bulk_out)))
		return NULL;
	ep = list_entry(ali_host->pipe_bulk_out.next , struct ali_host_ep, queue);
	urb = ep->urb;
	return urb;
}

static inline struct urb *ali_host_get_opt_bulk_urb(struct ali_host *ali_host)
{
	struct ali_host_ep *ep = NULL;
	struct urb *urb = NULL;

	if (unlikely(list_empty(&ali_host->pipe_opt_bulk)))
		return NULL;
	ep = list_entry(ali_host->pipe_opt_bulk.next , struct ali_host_ep, queue);
	urb = ep->urb;
	return urb;
}

static inline struct urb *ali_host_get_int_c_urb(struct ali_host *ali_host)
{
	struct ali_host_ep *ep = NULL;
	struct urb *urb = NULL;

	if (unlikely(list_empty(&ali_host->pipe_int_c)))
		return NULL;
	ep = list_entry(ali_host->pipe_int_c.next , struct ali_host_ep, queue);
	urb = ep->urb;
	return urb;
}

static inline struct urb *ali_host_get_int_d_urb(struct ali_host *ali_host)
{
	struct ali_host_ep *ep = NULL;
	struct urb *urb = NULL;

	if (unlikely(list_empty(&ali_host->pipe_int_d)))
		return NULL;
	ep = list_entry(ali_host->pipe_int_d.next , struct ali_host_ep, queue);
	urb = ep->urb;
	return urb;
}

static inline struct urb *ali_host_get_int_e_urb(struct ali_host *ali_host)
{
	struct ali_host_ep *ep = NULL;
	struct urb *urb = NULL;

	if (unlikely(list_empty(&ali_host->pipe_int_e)))
		return NULL;
	ep = list_entry(ali_host->pipe_int_e.next , struct ali_host_ep, queue);
	urb = ep->urb;
	return urb;
}

static inline struct urb *ali_host_get_int_f_urb(struct ali_host *ali_host)
{
	struct ali_host_ep *ep = NULL;
	struct urb *urb = NULL;

	if (unlikely(list_empty(&ali_host->pipe_int_f)))
		return NULL;
	ep = list_entry(ali_host->pipe_int_f.next , struct ali_host_ep, queue);
	urb = ep->urb;
	return urb;
}

static inline struct ali_host *hcd_to_ali_host(struct usb_hcd *hcd)
{
	return (struct ali_host *) (hcd->hcd_priv);
}

static inline struct usb_hcd *ali_host_to_hcd(struct ali_host *ali_host)
{
	return container_of((void *) ali_host, struct usb_hcd, hcd_priv);
}

static irqreturn_t ali_host_irq(struct usb_hcd *hcd) 
{
	struct ali_host *ali_host = NULL;
	UINT32 base_addr;
	UINT8 usbif_int_flag;
	UINT8 otg_int_flag;
	UINT8 h_int_flag1;
	UINT8 h_int_flag2;
	UINT8 flag3_sts;//Read and clear
	UINT8 flag3_int;

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;

	base_addr = ali_host->base_addr;

	// read-clear to save interrupt flag to private variable 
	ali_host->uif_int_flag |= get_usb_intf_int_status(base_addr);
	if ((ali_host->soc_chip == ALI_CHIPID_M3329E)&&(ali_host->ic_version >= CHIP_IC_REV_5))
	{
		flag3_sts=get_host_int_flag3(base_addr);//Read and clear
		flag3_int=get_host_int_flag3(base_addr)>>4;
	}
	else if(ali_host->soc_chip  == ALI_CHIPID_S3603)
	{
		flag3_sts=get_host_int_en_flag3(base_addr);//Read and clear
		flag3_int=get_host_int_flag3(base_addr);		
	}
	else
	{
		flag3_sts= 0;
		flag3_int= 0;	
	}


	if ((ali_host->uif_int_flag & B_USB2CPU_INT) &&(ali_host->uif_int_flag & B_USB_IP_IRQ))
	{
		ali_host->host_int_flag1 |= (get_host_int_flag1(base_addr) & get_host_int_en_flag1(base_addr));
		ali_host->host_int_flag2 |= (get_host_int_flag2(base_addr) & get_host_int_en_flag2(base_addr));
		ali_host->host_int_flag3 |= flag3_sts&flag3_int;
	}
	else
	{
		ali_host->host_int_flag1 = 0x00;
		ali_host->host_int_flag2 = 0x00;
		ali_host->host_int_flag3 = 0x00;
	}
	ALI_HOST_PRINTF(" fg:%02x %02x %02x %02x", ali_host->uif_int_flag,ali_host->host_int_flag1,ali_host->host_int_flag2,ali_host->host_int_flag3);
	if (0)//(sys_ic_get_chip_id() == ALI_M3329E)
	{
		if (((ali_host->host_int_flag1 & 0x01)== 0x01) && (get_port_connect_status(base_addr)==0x00)) 
		{	
			en_host_int_flag1(base_addr, B_H_SUSREM_IE|B_USB_RST_IE);	//patch for lots of connect interrupt,close it
			en_host_int_flag2(base_addr, B_H_INT2_ALL_IE&(~B_EP0_FSHERR_IE));
		}
	}

	usbif_int_flag = ali_host->uif_int_flag;
	otg_int_flag = ali_host->otg_int_flag;
	h_int_flag1 = ali_host->host_int_flag1;
	h_int_flag2 = ali_host->host_int_flag2;

	// Bulk opt. finished 
	if ((usbif_int_flag & B_USB2CPU_INT) && (usbif_int_flag & B_BULK_FIN_INT))
	{
		ali_host->uif_int_flag &= ~B_BULK_FIN_INT;
		if(ali_host->curr_urb_opt_bulk != NULL)
		{
			ali_hcd_bulk_opt_done(ali_host);
		}
	}
	// DMA RX finished 
	if ((usbif_int_flag & B_USB2CPU_INT) && (usbif_int_flag & B_RXED_INT))
	{

		ali_host->uif_int_flag &= ~B_RXED_INT;
		if(ali_host->curr_urb_bulk_in != NULL)
		{
			ali_hcd_dma_rx_done(ali_host);
		}
	}
	// DMA TX finished 
	if ((usbif_int_flag & B_USB2CPU_INT) && (usbif_int_flag & B_TXED_INT))
	{
		ali_host->uif_int_flag &= ~B_TXED_INT;
		if(ali_host->curr_urb_bulk_out != NULL)
		{
			ali_hcd_wait_txfifo_is_empty(base_addr);
			ali_hcd_dma_tx_done(ali_host);
		}
	}
	// Host_to_CPU Interrupt 
	if ((usbif_int_flag & B_USB2CPU_INT) &&(usbif_int_flag & B_USB_IP_IRQ))
	{
		ali_host->uif_int_flag &= ~B_USB_IP_IRQ;
		ali_hcd_h2c_int(ali_host);
	}

	return IRQ_HANDLED;
}

static int ali_host_urb_enqueue(struct usb_hcd *hcd,struct urb *urb,gfp_t mem_flags) 
{
	int ret = 0;
	struct ali_host *ali_host = NULL;
	UINT32 base_addr;
	struct usb_device	*udev = NULL;
	int epnum;
	struct usb_host_endpoint *hep = NULL;
	struct ali_host_ep *ep = NULL;
	unsigned long flags;

	unsigned int pipe = urb->pipe;
	int is_out = !usb_pipein(pipe);
	int transfer_type = usb_pipetype(pipe);
	UINT32 pkt_size = usb_maxpacket(urb->dev,urb->pipe,is_out);

	udev = urb->dev;
	pipe = urb->pipe;
	is_out = !usb_pipein(pipe);
	epnum = usb_pipeendpoint(pipe);
	hep = urb->ep;

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;

	base_addr = ali_host->base_addr;

	spin_lock_irqsave(&ali_host->lock, flags);

	ret = usb_hcd_link_urb_to_ep(hcd, urb);
	if (ret)
		goto error_not_linked;

	if(1)// (!hep->hcpriv)  20100511 hcpriv not NULL
	{
		ep = kzalloc(sizeof(struct ali_host_ep),GFP_ATOMIC);
		hep->hcpriv = ep;
		if (!hep->hcpriv) 
		{
			ret = -ENOMEM;
			goto error;
		}
		ep->hep = hep;
		ep->urb = urb;
		ep->transfer_type = transfer_type;
		urb->hcpriv = ep;
	}

	switch (transfer_type)
	{
		case PIPE_CONTROL:			
			if (list_empty(&ali_host->pipe_ctrl))
			{
				list_add_tail(&ep->queue, &ali_host->pipe_ctrl);
				ep->pipe_num = ALI_HOST_EP0;
				urb = ali_host_get_ctrl_urb(ali_host);
				ali_host->curr_urb_ctrl= urb;
				ali_hcd_issue_setup(ali_host, urb);
			}else{
				list_add_tail(&ep->queue, &ali_host->pipe_ctrl);
			}
			break;
		case PIPE_BULK:
			if (is_out)
			{
				ep->pipe_num = ALI_HOST_EPA;
				if (list_empty(&ali_host->pipe_bulk_out))
				{
					list_add_tail(&ep->queue, &ali_host->pipe_bulk_out);
					urb = ali_host_get_bulk_out_urb(ali_host);
					ali_host->curr_urb_bulk_out = urb;
					ali_hcd_bulkout_start(ali_host, urb);
				}else{
					list_add_tail(&ep->queue, &ali_host->pipe_bulk_out);
				}
			}else{
				ep->pipe_num = ALI_HOST_EPB;
				if (list_empty(&ali_host->pipe_bulk_in))
				{
					list_add_tail(&ep->queue, &ali_host->pipe_bulk_in);
					urb = ali_host_get_bulk_in_urb(ali_host);
					ali_host->curr_urb_bulk_in = urb;
					//					if((udev->descriptor.idVendor == 0x0424)&&(udev->descriptor.idProduct == 0xEC00)&&(urb->transfer_buffer_length <=512))
					if((urb->transfer_buffer_length < 512)&&(urb->transfer_buffer_length !=13))
					{	// mode sense bulk in len:192 byte, actual len: 4byte ... 
						ali_hcd_single_bulkin_start(ali_host, urb);
					}else{
						ali_hcd_bulkin_start(ali_host, urb);
					}
				}else{
					list_add_tail(&ep->queue, &ali_host->pipe_bulk_in);
				}			
			}
			break;
		case PIPE_INTERRUPT:
			if (!is_out)
			{
				if(ali_host->soc_chip != ALI_CHIPID_S3603)	
				{
					ret = -ENOSPC;
					break;
				}

				if(pkt_size > 64)
				{
					ret = -ENOSPC;
					break;
				}
				else if(pkt_size > 16) //(udev->descriptor.bDeviceClass == USB_CLASS_HUB) // (pkt_size > 16) 
				{
					ep->pipe_num = ALI_HOST_EPC;
					if (list_empty(&ali_host->pipe_int_c))
					{
						list_add_tail(&ep->queue, &ali_host->pipe_int_c);
						urb = ali_host_get_int_c_urb(ali_host);
						ali_host->curr_urb_int_c = urb;
						ali_hcd_IntInC_start(ali_host, urb);
					}else{
						list_add_tail(&ep->queue, &ali_host->pipe_int_c);
					}
				}
				else 
				{
					if (list_empty(&ali_host->pipe_int_d))
					{
						ep->pipe_num = ALI_HOST_EPD;
						list_add_tail(&ep->queue, &ali_host->pipe_int_d);
						urb = ali_host_get_int_d_urb(ali_host);
						ali_host->curr_urb_int_d = urb;
						ali_hcd_IntInD_start(ali_host, urb);
					}
					else if (list_empty(&ali_host->pipe_int_e))
					{
						ep->pipe_num = ALI_HOST_EPE;
						list_add_tail(&ep->queue, &ali_host->pipe_int_e);
						urb = ali_host_get_int_e_urb(ali_host);
						ali_host->curr_urb_int_d = urb;
						ali_hcd_IntInE_start(ali_host, urb);
					}
					else if (list_empty(&ali_host->pipe_int_f))
					{
						ep->pipe_num = ALI_HOST_EPF;
						list_add_tail(&ep->queue, &ali_host->pipe_int_f);
						urb = ali_host_get_int_f_urb(ali_host);
						ali_host->curr_urb_int_f = urb;
						ali_hcd_IntInF_start(ali_host, urb);
					}else{
						ep->pipe_num = ALI_HOST_EPD;
						list_add_tail(&ep->queue, &ali_host->pipe_int_d);
					}
				}
			}else{
				ret = -ENOSPC;
				break;
			}

			break;
		case PIPE_ISOCHRONOUS:
			ret = -ENOSPC;
			break;
		default:
			//  keep the old value of status and time-out
			ret = -ENOSPC;
			break;
	}
error:
	if (ret)
		usb_hcd_unlink_urb_from_ep(hcd, urb);
error_not_linked:
	spin_unlock_irqrestore(&ali_host->lock, flags);
	return ret;
}

	static void finish_request(struct ali_host *ali_host,struct urb * urb,	int status) 
__releases(ali_host->lock) __acquires(ali_host->lock)
{
	struct usb_hcd *hcd = NULL; 
	struct ali_host_ep *ep = NULL;
	struct usb_host_endpoint *hep = NULL;

	hep = urb->ep;
	ep = urb->hcpriv;

	hcd = ali_host_to_hcd(ali_host);

	if(ep != NULL)
	{
		list_del(&ep->queue);	
		kfree(ep);
		hep->hcpriv = NULL;
	}

	usb_hcd_unlink_urb_from_ep(hcd, urb);
	spin_unlock(&ali_host->lock);
	usb_hcd_giveback_urb(hcd, urb, status);
	spin_lock(&ali_host->lock);

}

static int ali_host_urb_dequeue(struct usb_hcd *hcd, struct urb *urb,int status) 
{
	struct ali_host *ali_host = NULL;
	struct ali_host_ep *ep = NULL;
	struct usb_host_endpoint *hep = NULL;
	INT32 ret;
	unsigned long 	flags;
	int urb_status;

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;	

	spin_lock_irqsave(&ali_host->lock, flags);
	ret = usb_hcd_check_unlink_urb(hcd, urb, status);
	if (ret)
		goto fail;

	hep = urb->ep;
	ep = urb->hcpriv;
	if (ep) 
	{
		if (ali_host->curr_urb_bulk_in == urb)
		{
			epb_error_handler(ali_host->base_addr);
			ali_host->curr_urb_bulk_in = NULL;
		}
		else if (ali_host->curr_urb_bulk_out == urb) 
		{
			epa_error_handler(ali_host->base_addr);
			ali_host->curr_urb_bulk_out = NULL;
		}
		else if(ali_host->curr_urb_ctrl == urb) 
		{
			ali_host->curr_urb_ctrl = NULL;
		}
		else if(ali_host->curr_urb_opt_bulk == urb) 
		{
			ali_host->curr_urb_opt_bulk = NULL;
		}
		else if(ali_host->curr_urb_int_c == urb) 
		{
			ali_host->curr_urb_int_c = NULL;
		}
		else if(ali_host->curr_urb_int_d == urb) 
		{
			ali_host->curr_urb_int_d = NULL;
		}
		else if(ali_host->curr_urb_int_e == urb) 
		{
			ali_host->curr_urb_int_e = NULL;
		}
		else if(ali_host->curr_urb_int_f == urb)
		{
			ali_host->curr_urb_int_f = NULL;
		} 

		if (urb)
		{
			urb_status =status ;
			finish_request(ali_host,  urb, urb_status);
			ALI_HOST_PRINTF("dequeue, urb %p  \n", urb);
		}else{
			ALI_HOST_PRINTF("dequeue, urb %p active; wait4irq\n", urb);
		}
	} else
		ret = -EINVAL;
fail:
	spin_unlock_irqrestore(&ali_host->lock, flags);
	return ret;

}

static void ali_host_endpoint_disable(struct usb_hcd *hcd, struct usb_host_endpoint *hep) 
{
	struct ali_host *ali_host = NULL;
	struct ali_host_ep *ep;
	struct urb *urb;
	unsigned long flags;

	ALI_HOST_PRINTF("ali_host_endpoint_disable!\n");

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return ;
	ep = (struct ali_host_ep *)(hep->hcpriv);
	if(ep == NULL)
		return ;
	urb = ep->urb;
	if(urb == NULL)
		return ;

	if (ali_host->curr_urb_bulk_in == urb)
	{
		ALI_HOST_PRINTF("ali host epb disable!\n");
		epb_error_handler(ali_host->base_addr);
		ali_host->curr_urb_bulk_in = NULL;
	}
	else if (ali_host->curr_urb_bulk_out == urb) 
	{
		ALI_HOST_PRINTF("ali host epa disable!\n");
		epa_error_handler(ali_host->base_addr);
		ali_host->curr_urb_bulk_out = NULL;
	}
	else if(ali_host->curr_urb_ctrl == urb) 
	{
		ALI_HOST_PRINTF("ali host ep0 disable!\n");
		ali_host->curr_urb_ctrl = NULL;
	}
	else if(ali_host->curr_urb_opt_bulk == urb) 
	{
		ALI_HOST_PRINTF("ali host epa disable!\n");
		ali_host->curr_urb_opt_bulk = NULL;
	}
	else if(ali_host->curr_urb_int_c == urb) 
	{
		ALI_HOST_PRINTF("ali host epc disable!\n");
		ali_host->curr_urb_int_c = NULL;
	}
	else if(ali_host->curr_urb_int_d == urb) 
	{
		ALI_HOST_PRINTF("ali host epd disable!\n");
		ali_host->curr_urb_int_d = NULL;
	}
	else if(ali_host->curr_urb_int_e == urb) 
	{
		ALI_HOST_PRINTF("ali host epe disable!\n");
		ali_host->curr_urb_int_e = NULL;
	}
	else if(ali_host->curr_urb_int_f == urb)
	{
		ALI_HOST_PRINTF("ali host epf disable!\n");
		ali_host->curr_urb_int_f = NULL;
	} 

	spin_lock_irqsave(&ali_host->lock, flags);
	//	irq_disable(ep->pipe_num);
	finish_request(ali_host, urb, -ESHUTDOWN);
	spin_unlock_irqrestore(&ali_host->lock, flags);
	return ;
}

static int ali_host_get_frame(struct usb_hcd *hcd) 
{
	struct ali_host *ali_host = NULL;
	ALI_HOST_PRINTF("ali_host_get_frame!\n");

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;

	return 0;
}

static void ali_hub_descriptor (struct usb_hub_descriptor *desc)
{
	desc->bDescriptorType = 0x29; 		//Descriptor Type, value:  29H for hub descriptor
	desc->bHubContrCurrent = 0;		//Maximum current requirements of the Hub Controller electronics in mA.

	desc->bNbrPorts = 1;			//Number of downstream facing ports that this hub supports
	desc->bDescLength = 9;		//Number of bytes in this descriptor, including this byte

	desc->bPwrOn2PwrGood = 0;		// Time (in 2 ms intervals) from the time the power-on sequence begins on a port until power is good on that port.
	//D1...D0:  Logical Power Switching Mode
	//D4...D3:  Over-current Protection Mode no overcurrent errors detection
	//D6...D5:  TT Think Time
	//D7:  Port Indicators Supported
	desc->wHubCharacteristics = cpu_to_le16(0x0011);	//

	// two bitmaps:  ports removable, and legacy PortPwrCtrlMask 
//	desc->bitmap[0] = 0 << 1;		//Indicates if a port has a removable device attached
//	desc->bitmap[1] = ~0;		// for 1.0 compliant devices.  All bits in this field should be set to 1B.
}

/*
   ali_host->port_status:
   hub control GetStatus data
sample:

wPortStatus.Connection  1: A device is present 
wPortStatus.Enable  0: Port is disabled
wPortStatus.Suspend 0: Not suspended 
wPortStatus.OverCurrent :0 No over-current condition exists
wPortStatus.Reset: 0: Reset signaling not asserted 
wPortStatus.Sleep: Reset signaling not asserted 
wPortStatus.Reserved: bit5-bit7 000 Zero
wPortStatus.Power 1: Powered-on state 
wPortStatus.LowSpeed: 0: Full or High-speed device attached
wPortStatus.HighSpeed 0:  Low or Full-speed device attached
wPortStatus.Test 0: Not in the Port Test Mode 
wPortStatus.Indicator : 0 Displays default colors
wPortStatus.Reserved bit13-bit15 000 Zero

wPortChange.Connection 1: Changed 
wPortChange.Enable 1: No change 
wPortChange.Suspend 0:  No change 
wPortChange.OverCurrent 0: No change 
wPortChange.Reset 0: No change
wPortChange.Reset 0: No change
wPortChange.Reserved bit5-bit15 000 Zero
 */
static int ali_hub_status_data(struct usb_hcd *hcd, char *buf) 
{
	struct ali_host *ali_host = NULL;
	UINT32 base_addr;
	UINT32 connect_status;

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;

	base_addr = ali_host->base_addr;

	//	if (!(ali_host->port_status & (0xffff << 16)))
	//		return 0;

	if(ali_host->port_changed == 1)
	{
		ali_host->port_changed = 0;
		// hub port 1 changed
		*buf = (1 << 1);
		return 1;
	}
	connect_status = ali_host->port_status & (1 << USB_PORT_FEAT_CONNECTION);	
	if(connect_status != get_port_connect_status(base_addr))
	{
		ALI_HOST_PRINTF("usb port changed \n");
		// hub port 1 changed
		*buf = (1 << 1);
		return 1;
	}
	if(ali_host->port_status & (1 << USB_PORT_FEAT_RESET))
	{
		ali_host->port_status &= ~(1 <<USB_PORT_FEAT_RESET);
		ALI_HOST_PRINTF("usb port reset \n");
		ali_host->port_status |= (1 <<USB_PORT_FEAT_C_RESET);
		// hub port 1 changed
		*buf = (1 << 1);
		return 1;
	}
	return 0;	
}

static int ali_hub_control(struct usb_hcd *hcd,
		u16  typeReq,
		u16  wValue,
		u16  wIndex,
		char *buf,
		u16  wLength)
{
	struct ali_host *ali_host = NULL ;
	int		retval = 0;
	int ret = 0;
	UINT32 port_status;		


	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;

	ALI_HOST_PRINTF(" ali_hub_control typeReq:%x wValue:%d wIndex:%d \n",typeReq,wValue,wIndex);


	switch (typeReq) {
		case ClearHubFeature:
		case SetHubFeature:
			switch (wValue) {
				case C_HUB_OVER_CURRENT:
					ALI_HOST_PRINTF("Set / clear HubFeature: C_HUB_OVER_CURRENT \n");
					break;
				case C_HUB_LOCAL_POWER:
					ALI_HOST_PRINTF("Set /clear HubFeature: C_HUB_LOCAL_POWER \n");
					break;
					break;
				default:
					goto error;
			}
			break;
		case ClearPortFeature:
			if (wIndex != 1 || wLength != 0)	// wIndex: port1
				goto error;
			switch (wValue) {
				case USB_PORT_FEAT_ENABLE:
					ALI_HOST_PRINTF("ClearPortFeature: USB_PORT_FEAT_ENABLE \n");
					ali_host->port_status &= ~(1 << USB_PORT_FEAT_ENABLE);
					ali_hcd_hw_init(ali_host);
					ali_hcd_sw_init(ali_host);
					break;
				case USB_PORT_FEAT_SUSPEND:
					ALI_HOST_PRINTF("ClearPortFeature: USB_PORT_FEAT_SUSPEND \n");
					if (!(ali_host->port_status & (1 << USB_PORT_FEAT_SUSPEND)))
						break;
					ali_host_bus_resume(hcd);
					ali_host->port_status &= ~(1 << USB_PORT_FEAT_SUSPEND);
					break;
				case USB_PORT_FEAT_POWER:
					ali_host->port_status &= ~(1 << USB_PORT_FEAT_POWER);
					ALI_HOST_PRINTF("ClearPortFeature: USB_PORT_FEAT_POWER \n");
					break;
				case USB_PORT_FEAT_C_ENABLE:
					ALI_HOST_PRINTF("ClearPortFeature: USB_PORT_FEAT_C_ENABLE \n");
					break;
				case USB_PORT_FEAT_C_SUSPEND:
					ALI_HOST_PRINTF("ClearPortFeature: USB_PORT_FEAT_C_SUSPEND \n");
					break;
				case USB_PORT_FEAT_C_CONNECTION:
					ALI_HOST_PRINTF("ClearPortFeature: USB_PORT_FEAT_C_CONNECTION \n");
					break;
				case USB_PORT_FEAT_C_OVER_CURRENT:
					ALI_HOST_PRINTF("ClearPortFeature: USB_PORT_FEAT_C_OVER_CURRENT \n");
					break;
				case USB_PORT_FEAT_C_RESET:
					ALI_HOST_PRINTF("ClearPortFeature: USB_PORT_FEAT_C_RESET \n");
					ali_host->port_status &= ~(1 <<USB_PORT_FEAT_C_RESET);
					break;
				default:
					goto error;
			}
			ali_host->port_status &= ~(1 << wValue);
			break;
		case GetHubDescriptor:
			ALI_HOST_PRINTF("GetHubDescriptor \n");
			ali_hub_descriptor((struct usb_hub_descriptor *) buf);
			break;
		case GetHubStatus:
			ALI_HOST_PRINTF("GetHubStatus \n");
			*(__le32 *) buf = cpu_to_le32(0);
			break;
		case GetPortStatus:
			ALI_HOST_PRINTF("GetPortStatus \n");
			//		if(ali_hcd_get_connect_stably(ali_host,&port_status));
			ret = ali_hcd_get_connect_stably(ali_host,&port_status);
			if(ret == 0)
			{
				if(port_status != (ali_host->port_status_last & (1 << USB_PORT_FEAT_CONNECTION) ))
					ali_host->port_status |= (1 << USB_PORT_FEAT_C_CONNECTION);	
				//			ali_host->port_status &= ~(1 << USB_PORT_FEAT_C_CONNECTION);				
				if (port_status == TRUE)
				{
					ALI_HOST_PRINTF("DEV_ATTACH!\n");
					ali_host->port_status |= (1 << USB_PORT_FEAT_CONNECTION);
				}else{
					ALI_HOST_PRINTF(" DEV_DETACH!\n");
					ali_host->port_status &= ~(1 << USB_PORT_FEAT_CONNECTION);
				}
			}
			if (wIndex != 1)
				goto error;
			*(__le32 *) buf = cpu_to_le32(ali_host->port_status);
			ali_host->port_status_last = ali_host->port_status;

			ALI_HOST_PRINTF("GetPortStatus %08x\n", ali_host->port_status);
			break;
		case SetPortFeature:
			if (wIndex != 1 || wLength != 0)
				goto error;
			ali_host->port_status |= 1 << wValue;
			switch (wValue) {
				case USB_PORT_FEAT_SUSPEND:
					ALI_HOST_PRINTF("SetPortFeature: USB_PORT_FEAT_SUSPEND \n");
					if (ali_host->port_status & (1 << USB_PORT_FEAT_RESET))
						goto error;
					if (!(ali_host->port_status & (1 << USB_PORT_FEAT_ENABLE)))
						goto error;
					ali_host_bus_suspend(hcd);
					ali_host->port_status |= (1 << USB_PORT_FEAT_SUSPEND);
					break;
				case USB_PORT_FEAT_POWER:	
					ALI_HOST_PRINTF("SetPortFeature: USB_PORT_FEAT_POWER \n");
					ali_hcd_hw_init(ali_host);
					ali_hcd_sw_init(ali_host);
					ali_host->port_status = (1 << USB_PORT_FEAT_POWER);
					break;
				case USB_PORT_FEAT_RESET:
					ALI_HOST_PRINTF("SetPortFeature: USB_PORT_FEAT_RESET \n");
					if (ali_host->port_status & (1 << USB_PORT_FEAT_SUSPEND))
						goto error;
					if (!(ali_host->port_status & (1 << USB_PORT_FEAT_POWER)))
						break;
					ali_host->port_status |= (1 << USB_PORT_FEAT_RESET);
					//			ali_hcd_hw_init(ali_host);
					//			msleep(10);
					ali_host_bus_reset(hcd);
					ali_hcd_sw_init(ali_host);
					ali_host->port_status |= (1 << USB_PORT_FEAT_ENABLE);
					ali_host->port_status &= ~(1 <<USB_PORT_FEAT_RESET);
					ali_host->port_status |= (1 <<USB_PORT_FEAT_C_RESET);
					break;
				default:
					goto error;
			}
			break;

		default:
error:
			// "protocol stall" on error 
			retval = -EPIPE;
	}

	return retval;
}

static int ali_host_start(struct usb_hcd *hcd) 
{
	struct ali_host *ali_host = NULL;
	ALI_HOST_PRINTF("ali_host_start!\n");

	hcd->state = HC_STATE_RUNNING;

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;
	ali_hcd_hw_init(ali_host);
	return 0;

}
static void ali_host_stop(struct usb_hcd *hcd) 
{
	struct ali_host *ali_host = NULL;
	unsigned long	flags;

	ALI_HOST_PRINTF("ali_host_stop!\n");

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return ;
	spin_lock_irqsave(&ali_host->lock, flags);
	//ali_hcd_power_off(ali_host);
	spin_unlock_irqrestore(&ali_host->lock, flags);

}

static int ali_host_bus_reset(struct usb_hcd *hcd)
{
	volatile unsigned char host_int_flag1;
	UINT32 speed;
	UINT32 i;
	struct ali_host *ali_host = NULL;
	unsigned long	flags;

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;
	spin_lock_irqsave(&ali_host->lock, flags);
	mask_host_int_flag1(ali_host->base_addr,  B_USB_RST_IE);
	// force usb bus reset 
	issue_reset_to_device(ali_host->base_addr);
	spin_unlock_irqrestore(&ali_host->lock, flags);
	i = 0;
	while(1)
	{
		msleep(5);
		host_int_flag1 = get_host_int_flag1(ali_host->base_addr);
		if(host_int_flag1&B_USB_RST_INT)
			break;
		if(i++ > 20)
		{
			ALI_HOST_PRINTF("reset bus timeout 0x20: 0x%02x",host_int_flag1);
			break;
		}
	}
	if(i<100)
	{
		ali_host->port_status |= (1 <<USB_PORT_FEAT_C_RESET);
		msleep(2);
		speed = get_device_speed(ali_host->base_addr);
		if (speed & B_OPER_FS)
		{
			//bit9:
			// 0 = Full-speed or High-speed device attached to this port (determined by bit 10).
			// 1 = Low-speed device attached to this port.
			// bit10:
			// 0 = Full-speed device attached to this port.
			// 1 = High-speed device attached to this port.
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
			ali_host->port_status &= ~USB_PORT_STAT_LOW_SPEED;
			ali_host->port_status &= ~USB_PORT_STAT_HIGH_SPEED;
#else		
			ali_host->port_status &= ~(1 << USB_PORT_FEAT_LOWSPEED);
			ali_host->port_status &= ~(1 << USB_PORT_FEAT_HIGHSPEED);
#endif
			ALI_HOST_PRINTF("Device is in FS mode!\n");
		}
		else if (speed & B_OPER_LS)
		{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
			ali_host->port_status |= USB_PORT_STAT_LOW_SPEED;
			ALI_HOST_PRINTF("Device is in LS mode!\n");
#else
			ali_host->port_status |= (1 << USB_PORT_FEAT_LOWSPEED);
			ALI_HOST_PRINTF("Device is in LS mode!\n");
#endif			
		}
		else
		{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
			ali_host->port_status |= USB_PORT_STAT_HIGH_SPEED;
			ALI_HOST_PRINTF("Device is in HS mode!\n");
#else
			ali_host->port_status |= (1 << USB_PORT_FEAT_HIGHSPEED);
			ALI_HOST_PRINTF("Device is in HS mode!\n");
#endif
		}
	}
	ali_host->port_changed = 1;
	return 0;
}

static int ali_host_bus_suspend(struct usb_hcd *hcd)
{
	volatile unsigned char host_int_flag1;
	UINT32 i;
	struct ali_host *ali_host = NULL;
	unsigned long	flags;

	ALI_HOST_PRINTF("ali_host_bus_suspend!\n");

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;
	spin_lock_irqsave(&ali_host->lock, flags);
	mask_host_int_flag1(ali_host->base_addr,  B_H_SUSREM_IE);
	issue_host_suspend(ali_host->base_addr);
	spin_unlock_irqrestore(&ali_host->lock, flags);
	i=0;
	while(1)
	{
		msleep(5);
		host_int_flag1 = get_host_int_flag1(ali_host->base_addr);
		if(host_int_flag1&B_H_SUSREM_INT)
			break;
		if(i++ > 20)
		{
			ALI_HOST_PRINTF("suspend timeout 0x20: 0x%02x",host_int_flag1);
			break;
		}
	}
	return 0;
}

static int ali_host_bus_resume(struct usb_hcd *hcd)
{
	volatile unsigned char host_int_flag1;
	UINT32 i;
	struct ali_host *ali_host = NULL;
	unsigned long	flags;

	ALI_HOST_PRINTF("ali_host_bus_resume!\n");

	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;

	spin_lock_irqsave(&ali_host->lock, flags);
	ali_host->port_status = 0;
	mask_host_int_flag1(ali_host->base_addr,  B_H_SUSREM_IE);
	issue_host_resume(ali_host->base_addr);
	spin_unlock_irqrestore(&ali_host->lock, flags);
	i=0;
	while(1)
	{
		msleep(5);
		host_int_flag1 = get_host_int_flag1(ali_host->base_addr);
		if(host_int_flag1&B_H_SUSREM_INT)
			break;
		if(i++ > 20)
		{
			ALI_HOST_PRINTF("resume timeout 0x20: 0x%02x",host_int_flag1);
			break;
		}
	}
	return 0;
}

static struct hc_driver ali_host_hc_driver = {
	.description =		hcd_name,
	.hcd_priv_size =	sizeof(struct ali_host),
	.irq =			ali_host_irq,

	// generic hardware linkage
	.flags =		HCD_USB2,

	.start =		ali_host_start,
	.stop =		ali_host_stop,

	//managing i/o requests and associated device resources
	.urb_enqueue =		ali_host_urb_enqueue,
	.urb_dequeue =		ali_host_urb_dequeue,
	.endpoint_disable =	ali_host_endpoint_disable,

	//periodic schedule support
	.get_frame_number =	ali_host_get_frame,

	// root hub support
	.hub_status_data = ali_hub_status_data,
	.hub_control =	ali_hub_control,
	.bus_suspend =	ali_host_bus_suspend,
	.bus_resume =	ali_host_bus_resume,
};

static int ali_host_probe(struct platform_device *pdev) 
{
	struct usb_hcd *hcd = NULL;
	struct ali_host *ali_host =NULL;
	UINT32 irq_num;
	UINT32 base_addr;
	INT32 retval;
	UINT32 chip_id,chip_version;

	// allocate and initialize hcd */
	hcd = usb_create_hcd(&ali_host_hc_driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		retval = -ENOMEM;
		goto err1;
	}
	//	hcd->rsrc_start = base_addr;
	//	hcd->rsrc_len = addr->start, addr->end - addr->start +1;
	//	base_addr = ioremap(hcd->rsrc_start, hcd->rsrc_len);

	ali_host = (struct ali_host *)(hcd->hcd_priv);

	chip_id = (*(volatile UINT32*)0xB8000000)>>16;
	ALI_HOST_PRINTF("chip_id: %x\n", chip_id);
	ali_host->soc_chip = chip_id; //sys_ic_get_chip_id() 
	chip_version = (*(volatile UINT32*)0xB8000000)&0xFF;
	ALI_HOST_PRINTF("chip_version: %x\n", chip_version);
	ali_host->ic_version = chip_version;	//sys_ic_get_rev_id()

	if (ali_host->soc_chip == ALI_CHIPID_M3202)//sys_ic_is_M3202())//(sys_ic_get_chip_id() == ALI_CHIPID_M3202)
		base_addr = HC_M3202_BASE_ADDR;
	else if ((ali_host->soc_chip == ALI_CHIPID_M3329E)&&(ali_host->ic_version >= CHIP_IC_REV_5))
		base_addr = HC_S3329E5_BASE_ADDR;
	else if ((ali_host->soc_chip == ALI_CHIPID_M3329E)&&(ali_host->ic_version < CHIP_IC_REV_5))
		base_addr = HC_S3329E_BASE_ADDR;
	else if (ali_host->soc_chip == ALI_CHIPID_S3602)
		base_addr = HC_S3602_BASE_ADDR;
	else if (ali_host->soc_chip == ALI_CHIPID_S3603)
		base_addr = HC_S3603_BASE_ADDR;
	else
	{
		retval = -ENODEV; // usb invalid chip id
		goto err;
	}

	if (ali_host->soc_chip == ALI_CHIPID_M3202 ) //(sys_ic_is_M3202())//(sys_ic_get_chip_id() == ALI_CHIPID_M3202)
		irq_num = USB_HC_M3202_INTERRUPT_ID;
	else if ((ali_host->soc_chip == ALI_CHIPID_M3329E)&&(ali_host->ic_version >= CHIP_IC_REV_5))
		irq_num = USB_HC_M3329E5_INTERRUPT_ID;
	else if ((ali_host->soc_chip == ALI_CHIPID_M3329E)&&(ali_host->ic_version < CHIP_IC_REV_5))
		irq_num = USB_HC_M3329E_INTERRUPT_ID;
	else if (ali_host->soc_chip == ALI_CHIPID_S3602)
		irq_num = USB_HC_S3602_INTERRUPT_ID;
	else if (ali_host->soc_chip == ALI_CHIPID_S3603)
		irq_num = USB_HC_S3602F_INTERRUPT_ID;
	else
	{
		retval = -ENODEV; // usb invalid chip id
		goto err;
	}

	if (((ali_host->soc_chip == ALI_CHIPID_M3329E)&&(ali_host->ic_version >= CHIP_IC_REV_5))
			|| (ali_host->soc_chip == ALI_CHIPID_S3603))
	{
		ali_host->support_hub = 1;
	}
	else
	{
		ali_host->support_hub = 0;
	}

	ali_host->base_addr = base_addr;
	ali_host->irq_num = irq_num;

	spin_lock_init(&ali_host->lock);
	ali_hcd_sw_init(ali_host);
	ali_hcd_hw_init(ali_host);

	retval = usb_add_hcd(hcd, irq_num, IRQF_DISABLED);
	if (retval != 0)
		goto err1;

	if ((ali_host->soc_chip  == ALI_CHIPID_S3603)&&(ali_host->ic_version >= 1))	// s3603 only one usb host
	{
		hcd = usb_create_hcd(&ali_host_hc_driver, &pdev->dev, dev_name(&pdev->dev));
		if (!hcd) {
			retval = -ENOMEM;
			goto err1;
		}		
		ali_host = (struct ali_host *)(hcd->hcd_priv);

		chip_id = (*(volatile UINT32*)0xB8000000)>>16;
		ALI_HOST_PRINTF("chip_id: %x\n", chip_id);
		ali_host->soc_chip = chip_id; //sys_ic_get_chip_id() 
		ali_host->ic_version = 0;	//sys_ic_get_rev_id()

		base_addr = HC_S3603_BASE_ADDR1;
		irq_num = USB_HC_S3602F_INTERRUPT_ID1;
		ali_host->support_hub = 1;			
		ali_host->base_addr = base_addr;
		ali_host->irq_num = irq_num;
		spin_lock_init(&ali_host->lock);
		ali_hcd_sw_init(ali_host);
		ali_hcd_hw_init(ali_host);
		retval = usb_add_hcd(hcd, irq_num, IRQF_DISABLED);
	}
	if (retval != 0)
		goto err1;

	return retval;

err1:
	usb_put_hcd(hcd);
err:
	ALI_HOST_PRINTF("init error, %d\n", retval);
	return retval;

}

static int ali_host_remove(struct platform_device *pdev) 
{
	struct usb_hcd *hcd = NULL;
	struct ali_host *ali_host = NULL;

	ALI_HOST_PRINTF("ali_host_remove!\n");

	hcd = platform_get_drvdata(pdev);
	if(hcd == NULL)
		return -ENODEV;
	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;	

	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);

	return 0;
}

static int ali_host_suspend(struct platform_device *pdev, pm_message_t state) 
{
	struct usb_hcd *hcd = NULL;
	struct ali_host *ali_host = NULL;

	ALI_HOST_PRINTF("ali_host_suspend!\n");

	hcd = platform_get_drvdata(pdev);
	if(hcd == NULL)
		return -ENODEV;
	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;

	switch (state.event) 
	{
		case PM_EVENT_FREEZE:
		case PM_EVENT_SUSPEND:
		case PM_EVENT_HIBERNATE:
		case PM_EVENT_PRETHAW:		/* explicitly discard hw state */
			issue_host_suspend(ali_host->base_addr);
			break;
	}
	return 0;
	//	pdev->dev.power.power_state = state;
}

static int ali_host_resume(struct platform_device *pdev) 
{
	struct usb_hcd *hcd= NULL;
	struct ali_host *ali_host = NULL;

	ALI_HOST_PRINTF("ali_host_resume!\n");

	hcd = platform_get_drvdata(pdev);
	if(hcd == NULL)
		return -ENODEV;
	ali_host = (struct ali_host *)(hcd->hcd_priv);
	if(ali_host == NULL)
		return -ENODEV;
	issue_host_resume(ali_host->base_addr);
	return 0;
	//pdev->dev.power.power_state = PMSG_ON;
}

static struct platform_driver ali_host_driver = {
	.probe =	ali_host_probe,
	.remove =	ali_host_remove,
	.suspend =	ali_host_suspend,
	.resume =	ali_host_resume,
	.driver		= {
		.name = (char *) hcd_name,
		.owner	= THIS_MODULE,
	},
};

static int __init ali_host_init(void)
{
	int ret;

	if (usb_disabled())
		return -ENODEV;

	ALI_HOST_PRINTF(" platform_driver_register %s, %s\n", hcd_name,DRIVER_VERSION);
	ret = platform_driver_register(&ali_host_driver);
	if (ret) {
		ALI_HOST_PRINTF(" register platform driver fail \n");
	}
	return ret;
}

module_init(ali_host_init);

static void __exit ali_host_cleanup(void)
{
	ALI_HOST_PRINTF("ali_host_cleanup!\n");

	platform_driver_unregister(&ali_host_driver);
}

module_exit(ali_host_cleanup);

MODULE_AUTHOR("ALi Corp");
MODULE_DESCRIPTION("ALi STB usb host controller driver");
MODULE_ALIAS("platform:ali-hcd");
MODULE_LICENSE("GPL");


