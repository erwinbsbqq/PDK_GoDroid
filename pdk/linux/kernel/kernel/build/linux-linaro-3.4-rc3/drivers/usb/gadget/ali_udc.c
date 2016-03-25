/*
 * ALi USB Device Controller Driver ALi UDC
 *
 * linux/drivers/usb/gadget/alidev_udc.c 
 *
 * Copyright (C) 2007-2014 Alitech Corp.
 *
 * Author : David Shih <david.shih@alitech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#ifdef CONFIG_ARM
#include <mach/ali-s3921.h>
#endif
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/prefetch.h>

#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>
#include <asm/irq.h>
#include <asm/unaligned.h>
#ifdef CONFIG_ARM
#include <mach/irqs.h>
#include <mach/hardware.h>
#endif
#include "ali_reg.h"
#include "ali_udc.h"

//#define CONFIG_DEBUG_USB_ALI_DEV
#ifdef CONFIG_DEBUG_USB_ALI_DEV
	#define UDC_DBG  printk
#else            
	#define UDC_DBG(...) do{} while(0)
#endif

#define ALI_UDC_DRIVER_VERSION "ali_udc_ver_2015_0303"

static const char		gadget_name[] = "alidev_udc";
static const char		driver_desc[] = DRIVER_DESC;
static struct alidev_udc	  *the_controller;

static struct usb_request * alidev_udc_alloc_request(struct usb_ep *_ep, gfp_t mem_flags);
static void alidev_udc_enable(struct alidev_udc *dev);
static void alidev_udc_disable(struct alidev_udc *dev);

u8 wait_epb_rx_data = 0;
#ifdef SUPPORT_C3921
u8 wait_epe_rx_data = 0;
u8 wait_eph_rx_data = 0;
u8 wait_epj_rx_data = 0;
u8 wait_epl_rx_data = 0;
#endif

int monitor_cfg = 0;
int manual_set_cfg = 0;
int manual_set_cfg_done = 0;

static void alidev_epfifo_rst(struct alidev_udc* dev, u8 ep)
{
	u32 ctrl_reg;

	switch(ep)
	{
		case EP0:
			ctrl_reg = USBD_EP0_FCTR;
			break;
		case EPA:
			ctrl_reg = USBD_EPA_FCTR;
			break;
		case EPB:
			ctrl_reg = USBD_EPB_FCTR;
			break;
    case EPC:
			ctrl_reg = USBD_EPC_FCTR;
			break;
		case EPD:
			ctrl_reg = USBD_EPD_FCTR;
			break;

#ifdef SUPPORT_C3921			
		case EPE:
			ctrl_reg = USBD_EPE_FCTR;
			break;
		case EPF:
			ctrl_reg = USBD_EPF_FCTR;
			break;
		case EPG:
			ctrl_reg = USBD_EPG_FCTR;
			break;
		case EPH:
			ctrl_reg = USBD_EPH_FCTR;
			break;
		case EPI:
			ctrl_reg = USBD_EPI_FCTR;
			break;
		case EPJ:
			ctrl_reg = USBD_EPJ_FCTR;
			break;
		case EPK:
			ctrl_reg = USBD_EPK_FCTR;
			break;	
		case EPL:
			ctrl_reg = USBD_EPL_FCTR;
			break;	
#endif

		default:	
        		ctrl_reg = USBD_EP0_FCTR;
			break;
	}
	udc_write(dev, udc_read(dev, ctrl_reg) | EP_INIT, ctrl_reg);
	udc_write(dev, udc_read(dev, ctrl_reg) & ~EP_INIT, ctrl_reg);
}

/*------------------------- I/O ----------------------------------*/
/*
 *	alidev_udc_done
 */
static void alidev_udc_done(struct alidev_ep *ep,
		struct alidev_request *req, int status)
{
	unsigned halted = ep->halted;

	UDC_DBG("%s ep->bEndpointAddress=%d req->req.length=%d\n",
                __func__, ep->bEndpointAddress,  req->req.length);
	list_del_init(&req->queue);

	if (likely (req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	ep->halted = 1;
	req->req.complete(&ep->ep, &req->req);
	ep->halted = halted;
}

static void alidev_udc_nuke(struct alidev_udc *udc,
		struct alidev_ep *ep, int status)
{	
	/* Sanity check */
	if (&ep->queue == NULL)
	{		
		return;
	}
	while (!list_empty (&ep->queue)) {
		struct alidev_request *req;
		req = list_entry (ep->queue.next, struct alidev_request,	queue);

		alidev_udc_done(ep, req, status);
	}
}

static inline void 
	alidev_udc_clear_ep_state(struct alidev_udc *dev)
{
	unsigned i;

	/* hardware SET_{CONFIGURATION,INTERFACE} automagic resets endpoint
	 * fifos, and pending transactions mustn't be continued in any case.
	 */

	for (i = 1; i < ALIDEV_ENDPOINTS; i++)
		alidev_udc_nuke(dev, &dev->ep[i], -ECONNABORTED);
}

static inline int alidev_udc_fifo_count_out(void)
{
	int tmp;
	return tmp;
}

void usbd_dma_init(struct alidev_udc* dev, u8 ep)
{
u32 reg = 0;
u32 intf_reg = 0;

    switch(ep)
    {
        case EPA:
        case EPB:
            reg = USBD_DMACTL;
            intf_reg = USBD_INTF_DMA_CTL;
            break;

#ifdef SUPPORT_C3921			
        case EPD:
        case EPE:
            reg = USBD_DMA1CTL;
            intf_reg = USBD_INTF_DMA_1_CTL;
            break;

        case EPG:
        case EPH:
            reg = USBD_DMA2CTL;
            intf_reg = USBD_INTF_DMA_2_CTL;
            break;
            
        case EPI:
        case EPJ:
            reg = USBD_DMA3CTL;
            intf_reg = USBD_INTF_DMA_3_CTL;
            break;
            
        case EPK:
        case EPL:
            reg = USBD_DMA4CTL;
            intf_reg = USBD_INTF_DMA_4_CTL;
            break;            
#endif
        default:
            reg = USBD_DMACTL;
            intf_reg = USBD_INTF_DMA_CTL;
            break;
    }

    udc_write(dev, DMA_INIT, reg);
    udc_write(dev, 0, reg);
    udc_write(dev, INTF_DMA_INIT, intf_reg);
    udc_write(dev, 0, intf_reg);
}

void usbd_set_dma_len(struct alidev_udc *dev, u8 ep, u32 len)
{
u32 dma_cnt_l = 0;
u32 dma_cnt_m = 0;
u32 dma_cnt_h = 0;
u32 dma_base_addr = 0;
u32 intf_base_len = 0;
dma_addr_t	dmaaddr;	


    switch(ep)
    {
#ifdef SUPPORT_C3921			        
        case EPD:
        case EPE:
            dma_cnt_l = USBD_DMA1CNTL;
            dma_cnt_m = USBD_DMA1CNTM;
            dma_cnt_h = USBD_DMA1CNTH;
            dma_base_addr = USBD_INTF_DMA_1_BASE_ADDR;
            intf_base_len  = USBD_INTF_DMA_1_BASE_LEN;
            dmaaddr = dev->dma[1].addr;
            break;
            
        case EPG:
        case EPH:
            dma_cnt_l = USBD_DMA2CNTL;
            dma_cnt_m = USBD_DMA2CNTM;
            dma_cnt_h = USBD_DMA2CNTH;
            dma_base_addr = USBD_INTF_DMA_2_BASE_ADDR;
            intf_base_len  = USBD_INTF_DMA_2_BASE_LEN;
            dmaaddr = dev->dma[2].addr;            
            break;
            
        case EPI:
        case EPJ:
            dma_cnt_l = USBD_DMA3CNTL;
            dma_cnt_m = USBD_DMA3CNTM;
            dma_cnt_h = USBD_DMA3CNTH;
            dma_base_addr = USBD_INTF_DMA_3_BASE_ADDR;
            intf_base_len  = USBD_INTF_DMA_3_BASE_LEN;            
            dmaaddr = dev->dma[3].addr;            
            break;
            
        case EPK:
        case EPL:
            dma_cnt_l = USBD_DMA4CNTL;
            dma_cnt_m = USBD_DMA4CNTM;
            dma_cnt_h = USBD_DMA4CNTH;
            dma_base_addr = USBD_INTF_DMA_4_BASE_ADDR;
            intf_base_len  = USBD_INTF_DMA_4_BASE_LEN;            
            dmaaddr = dev->dma[4].addr;
            break;            
#endif

        case EPA:
        case EPB:
        default:    
            dma_cnt_l = USBD_DMACNTL;
            dma_cnt_m = USBD_DMACNTM;
            dma_cnt_h = USBD_DMACNTH;
            dma_base_addr = USBD_INTF_DMA_BASE_ADDR;
            intf_base_len  = USBD_INTF_DMA_BASE_LEN;
            dmaaddr = dev->dma[0].addr;
            break;
    }

    udc_write(dev, (u8)(len&0xFF), dma_cnt_l);
    udc_write(dev, (u8)((len>>8)&0xFF), dma_cnt_m);
    udc_write(dev, (u8)((len>>16)&0xFF), dma_cnt_h);

    udc_write32(dev, (u32) (dmaaddr) & 0x7FFFFFFF, dma_base_addr);	
    udc_write32(dev, len, intf_base_len);
}

int usbd_set_dma_start(struct alidev_udc *dev, u8 ep, u8 is_in)
{
u32 intf_ctrl = 0;
u32 intf_ctrl2 = 0;
u32 dma_ctrl = 0;
int ret = 0;

    UDC_DBG("%s\n",__func__);    
    switch(ep)
    {        
#ifdef SUPPORT_C3921			
        case EPD:
        case EPE:
            intf_ctrl = USBD_INTF_DMA_1_CTL;
            intf_ctrl2 = USBD_INTF_DMA_1_CTL2;
            dma_ctrl = USBD_DMA1CTL;
            break;
            
        case EPG:
        case EPH:
            intf_ctrl = USBD_INTF_DMA_2_CTL;
            intf_ctrl2 = USBD_INTF_DMA_2_CTL2;
            dma_ctrl = USBD_DMA2CTL;
            break;
            
        case EPI:
        case EPJ:
            intf_ctrl = USBD_INTF_DMA_3_CTL;
            intf_ctrl2 = USBD_INTF_DMA_3_CTL2;
            dma_ctrl = USBD_DMA3CTL;
            break;
            
        case EPK:
        case EPL:
            intf_ctrl = USBD_INTF_DMA_4_CTL;
            intf_ctrl2 = USBD_INTF_DMA_4_CTL2;
            dma_ctrl = USBD_DMA4CTL;
            break;            
#endif

        case EPA:
        case EPB:
        default:    
            intf_ctrl = USBD_INTF_DMA_CTL;
            intf_ctrl2 = USBD_INTF_DMA_CTL2;
            dma_ctrl = USBD_DMACTL;            
            break;
    }

    if (is_in)
    {
        udc_write(dev, DMA_CMD_RX, intf_ctrl2);		
	 			udc_write(dev, INTF_DMA_EN, intf_ctrl);
	 			udc_write(dev, DMA_EN, dma_ctrl);	
    }
    else
    {   
       udc_write(dev, DMA_CMD_TX, intf_ctrl2);
			 udc_write(dev, DMA_EN, dma_ctrl);				
			 udc_write(dev, INTF_DMA_EN, intf_ctrl);				
    }
    
    switch(ep)
    {        
        case EPA:
        case EPB:
          ret = wait_for_completion_timeout(&dev->dma_epab_completion, msecs_to_jiffies(5000));	          
          break;

#ifdef SUPPORT_C3921			            
        case EPD:
        case EPE:
          ret = wait_for_completion_timeout(&dev->dma_epde_completion, msecs_to_jiffies(5000));	
          break;
            
        case EPG:
        case EPH:
          ret = wait_for_completion_timeout(&dev->dma_epgh_completion, msecs_to_jiffies(5000));	
          break;
            
        case EPI:
        case EPJ:
        	ret = wait_for_completion_timeout(&dev->dma_epij_completion, msecs_to_jiffies(5000));	
          break;
            
        case EPK:
        case EPL:
        	ret = wait_for_completion_timeout(&dev->dma_epkl_completion, msecs_to_jiffies(5000));	
          break;
#endif            
        default:  
        	ret = 0;  
        	break;    
    }
    
    if (!ret)
    {	
    	if (!dev->vbus)
    	  printk("[UDC Warning] : Dis-connected, DMA timeout, is_in %d, EP %d\n", is_in, ep);
    	else	
    	{
    		printk("[UDC Warning] : Fix me, DMA timeout, is_in %d, EP %d\n", is_in, ep);
    		usbd_dma_init(dev, ep); 	
    		alidev_epfifo_rst(dev, ep);
    	}
    	return 0;    	
    }	

    if (is_in)
    	udelay(50); /* delay for USBIF to DRAM */ 
    	
    return 1;
}

int alidev_udc_set_dma(struct alidev_request *req, struct alidev_udc *dev, u8 is_in, u8* buf, u8 ep)
{	
	unsigned len = req->req.length - req->req.actual;
    	u8			*dmabuf;
	
     	UDC_DBG("%s, is_in %d, len %d ep %d\n", __func__, is_in, len, ep);	
      switch(ep)
      {	     
#ifdef SUPPORT_C3921			            
            case EPD:
            case EPE:
                dmabuf = dev->dma[1].buf;
                break;

            case EPG:
            case EPH:
                dmabuf = dev->dma[2].buf;
                break;
                
            case EPI:
            case EPJ:
                dmabuf = dev->dma[3].buf;
                break;
                
            case EPK:
            case EPL:
                dmabuf = dev->dma[4].buf;
                break;            
#endif	   
            case EPA:
            case EPB:
            default:    
                dmabuf = dev->dma[0].buf;
                break;
      }

      usbd_dma_init(dev, ep); 		
      usbd_set_dma_len(dev, ep, len);

      if (is_in)
      {
           if (usbd_set_dma_start(dev, ep, is_in))
           {                
                memcpy(buf, dmabuf, len);		
                return len;
            }
      }
      else
      {
           memcpy(dmabuf, buf, len); 
           if (usbd_set_dma_start(dev, ep, is_in))
              return len;
      }
      return 0;       
}

/*
 *	alidev_udc_write_ep0_fifo
 * 
 */

static void alidev_udc_write_ep0_fifo(struct alidev_udc *dev, struct alidev_ep *ep,
		struct alidev_request *req)
{	
	u8 *buf = req->req.buf + req->req.actual;
	unsigned len = req->req.length - req->req.actual;
	int i;
	
	UDC_DBG("%s, ep=0x%02x, request len=%d, actual=%d \n",__func__, ep->bEndpointAddress,
		req->req.length,  req->req.actual);			
	
	prefetch(buf);		
	for (i=0; i<len; i++)
		udc_write(dev, buf[i], USBD_EP0_FIFO);
		
	udc_write(dev, udc_read(dev, USBD_EP0_FCTR) | EP_FORCE, USBD_EP0_FCTR);			
	req->req.actual += len;	
	ep->dev->ep0state = EP0_IDLE;	
	alidev_udc_done(ep, req, 0);
	
	i = 50000;
  while(i--)
  {
    	if (udc_read(dev, USBD_EP0_FCTR) & EP_ALL_EMPTY)
  	     break;
      udelay(1);                          
  }
        
  if (!i)
      printk("[UDC Warning] : Fix me, EP0 fifo not empty !\n");
	
	/*monitor Host send SETUP (SET CONFIGURATION)*/	
}

/*
 *	alidev_udc_write_fifo
 *
 * return:  0 = still running, 1 = completed, negative = errno
 */

static int alidev_udc_write_fifo(struct alidev_udc *dev, struct alidev_ep *ep,
		struct alidev_request *req)
{	
	int	is_last = 0;
	int i = 0;
	u32	idx = 0;
	u32	fifo_reg = 0;
  u32 ctrl_reg = 0;
  u32 misc_reg = 0;
	u8 *buf = req->req.buf + req->req.actual;
	unsigned len = req->req.length - req->req.actual;	
	
  idx = ep->bEndpointAddress & 0x7F;
  UDC_DBG("%s, ep=0x%02x, request len=%d, actual=%d \n",__func__, ep->bEndpointAddress, req->req.length,  req->req.actual);
      //printk("i %d\n", req->req.length);
	switch (idx) {
	case EP0:
		fifo_reg = USBD_EP0_FIFO;
		ctrl_reg = USBD_EP0_FCTR;
		break;
	case EPA:
		fifo_reg = USBD_EPA_FIFO;
		ctrl_reg = USBD_EPA_FCTR;
              misc_reg = USBD_MISCCTL;
		break;
	case EPC:
		fifo_reg = USBD_EPC_FIFO;
		ctrl_reg = USBD_EPC_FCTR;
		break;        

#ifdef SUPPORT_C3921			
	case EPD:
		fifo_reg = USBD_EPD_FIFO;
		ctrl_reg = USBD_EPD_FCTR;
             misc_reg = USBD_MISCCTL1;
            break;
  case EPF:
		fifo_reg = USBD_EPF_FIFO;
		ctrl_reg = USBD_EPF_FCTR;
            break;
	case EPG:
		fifo_reg = USBD_EPG_FIFO;
		ctrl_reg = USBD_EPG_FCTR;
             misc_reg = USBD_MISCCTL2;        
		break;
	case EPI:
		fifo_reg = USBD_EPI_FIFO;
		ctrl_reg = USBD_EPI_FCTR;
            misc_reg = USBD_MISCCTL3;        
		break;
	case EPK:
		fifo_reg = USBD_EPK_FIFO;
		ctrl_reg = USBD_EPK_FCTR;
            misc_reg = USBD_MISCCTL4;        
		break;
#endif

	default:
		//fifo_reg = USBD_EP0_FIFO;
		//ctrl_reg = USBD_EP0_FCTR;
    //misc_reg = USBD_MISCCTL;        
		printk("%s: bEndpointAddress = %d, error ! only EP0(ep0) and EPA(ep2)can write\n",__func__, ep->bEndpointAddress);
   	//break;	
   	return 0;
	}
	
	i = 50000;
  while(i--)
  {
   	if (udc_read(dev, ctrl_reg) & EP_ALL_EMPTY)
      break;
      udelay(1);                          
  }
        
  if (!i)
   	printk("[UDC Warning] : Fix me, fifo not empty 2!\n");

	prefetch(buf);
	UDC_DBG("%s: Out %d \n", __func__, len);
	while(1)
	{
      /* let IN/OUT transaction isolation */
#ifdef SUPPORT_C3921      
      if (wait_epb_rx_data || wait_epe_rx_data || wait_eph_rx_data || 
      	wait_epj_rx_data || wait_epl_rx_data)
#else      	      
      if (wait_epb_rx_data )
#endif      	
      {
        	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10 + (len / 1024)));	
      }     
                 
			if (len < 512)
			{									
				for (i=0; i<len; i++)
				{                                  
        	udc_write(dev, buf[i], fifo_reg);
        }
				buf = buf + len;			
				req->req.actual += len;
        udc_write(dev, udc_read(dev, ctrl_reg) | EP_FORCE, ctrl_reg);	
				len  = 0;		
			}	
			else	
			{                     
      	if (!(len%512))
				{	
			  	udc_write(dev, udc_read(dev, misc_reg) & (~EP_DMA_AUTO_FRC), misc_reg); /*EP DMA auto force disable*/
				}	
				else
				{	
			 		udc_write(dev, udc_read(dev, misc_reg) | EP_DMA_AUTO_FRC, misc_reg); /*EP DMA auto force enable*/
				}	
				len = alidev_udc_set_dma(req, dev, D2H, buf, idx);                     
				req->req.actual += len;
				buf += len;
				len = 0;
			}			
			if (!len)
				break;
	}		
	alidev_udc_done(ep, req, 0);
	is_last = 1;	
	return is_last;
}
	
/*
 * 
 */
static void alidev_udc_read_ep0_fifo(struct alidev_udc *dev, struct alidev_ep *ep,
				 struct alidev_request *req)
{
	u8		*buf;
	unsigned	bufferspace;
	int		count  = 0;	
	int i = 0;

	UDC_DBG("%s, ep=0x%02x, request len=%d, actual=%d \n",__func__, ep->bEndpointAddress, req->req.length,  req->req.actual);
	
	if (!req->req.length)
		return;
		
	buf = req->req.buf + req->req.actual;
	bufferspace = req->req.length - req->req.actual;
	if (!bufferspace) 
	{
		UDC_DBG("%s:bufferspace=0\n",__func__);
		return;
	}
	
	if (udc_read(dev, USBD_EP0_FCTR) & EP_ALL_EMPTY)
	{	
			udc_write(dev, udc_read(dev, USBD_INTENR) & (~EP0_RX_INT), USBD_INTENR);		//disable EP0 RX int
			list_add_tail(&req->queue, &ep->queue);				  
		  udc_write(dev, udc_read(dev, USBD_INTENR) | EP0_RX_INT, USBD_INTENR);			//Enable EP0 RX int		
		  return;
	}
	
	for (i=0; i<req->req.length; i++)
	{
  	if ((udc_read(dev, USBD_EP0_FCTR) & EP_ALL_EMPTY))
  	{                        
    	udelay(125);
    }
		buf[i] = udc_read(dev, USBD_EP0_FIFO);
	}
	
	req->req.actual += count;
	if (count)
	{		
		alidev_udc_done(ep, req, 0);					
	}		
}

/*
 * return:  0 = still running, 1 = queue empty, negative = errno
 */

 
static int alidev_udc_read_fifo(struct alidev_udc *dev, struct alidev_ep *ep,
				 struct alidev_request *req)
{
	u8		*buf;
	unsigned	bufferspace;
	int		is_last = 1;	
	int		count  = 0;
	u8		idx;
	u32		fifo_reg,ctrl_reg;
	int i = 0;

	UDC_DBG("%s, ep=0x%02x, request len=%d, actual=%d \n",__func__, ep->bEndpointAddress, req->req.length,  req->req.actual);	
	if (!req->req.length)
	{				
		printk("%s, exit line %d\n", __func__, __LINE__);				
		return 0;
	}
		
	idx = ep->bEndpointAddress & 0x7F;
	switch (idx)
	{		
		case EP0:
			fifo_reg = USBD_EP0_FIFO;
			ctrl_reg = USBD_EP0_FCTR;
			break;			
		case EPB: 			
			fifo_reg = USBD_EPB_FIFO;
			ctrl_reg = USBD_EPB_FCTR;
			break; 

#ifdef SUPPORT_C3921				           
		case EPE:
			fifo_reg = USBD_EPE_FIFO;
			ctrl_reg = USBD_EPE_FCTR;
			break;
		case EPH:
			fifo_reg = USBD_EPH_FIFO;
			ctrl_reg = USBD_EPH_FCTR;
			break;
		case EPJ:
			fifo_reg = USBD_EPJ_FIFO;
			ctrl_reg = USBD_EPJ_FCTR;
			break;
		case EPL:
			fifo_reg = USBD_EPL_FIFO;
			ctrl_reg = USBD_EPL_FCTR;
			break;
#endif

		default:
			printk("%s: bEndpointAddress = %d, error ! only EP0(ep0) and EPB(ep2)can read\n",
				__func__, ep->bEndpointAddress);
			return 0;
			//break;	
	}
  
	UDC_DBG("%s request len = %d, actual = %d, ep->ep.maxpacket %d\n",
			__func__, req->req.length,  req->req.actual,ep->ep.maxpacket);
	buf = req->req.buf + req->req.actual;
	bufferspace = req->req.length - req->req.actual;
	if (!bufferspace) 
	{
		printk("%s, exit line %d\n",__func__, __LINE__);						
		return -1;
	}				
	
	if (req->req.length <= 512)
	{		
		i = 0;	
wait_5ms:		
		if (udc_read(dev, ctrl_reg) & EP_ALL_EMPTY)
		{	
    	if (i++ < 5000)
    	{
    		udelay(1);
    		goto wait_5ms;
    	}	
    	
    	if (USBD_EPB_FCTR == ctrl_reg)
      {       
      	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));	 
        list_add_tail(&req->queue, &ep->queue);				  
        wait_epb_rx_data++;     
        return 0;
      }
#ifdef SUPPORT_C3921	      
      else if (USBD_EPE_FCTR == ctrl_reg)
      {			    	
      	wait_epe_rx_data++;	
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));	 
        list_add_tail(&req->queue, &ep->queue);				  
        return 0;
      }
      else if (USBD_EPH_FCTR == ctrl_reg)
      {
      	wait_eph_rx_data++;
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));	 
        list_add_tail(&req->queue, &ep->queue);				  
        return 0;
      }
      else if (USBD_EPJ_FCTR == ctrl_reg)
      {
      	wait_epj_rx_data++;
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));	 
        list_add_tail(&req->queue, &ep->queue);				  
        return 0;
      }
      else if (USBD_EPL_FCTR == ctrl_reg)
      {
      	wait_epl_rx_data++;
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));	 
        list_add_tail(&req->queue, &ep->queue);				  
        return 0;
      }             
#endif      
		}   
              
		for (i=0; i<req->req.length; i++)
    {
    	if ((udc_read(dev, ctrl_reg) & EP_ALL_EMPTY))
      {                        
      	udelay(125);
      }
			buf[i] = udc_read(dev, fifo_reg);					
    }
    req->req.actual += req->req.length;  
		alidev_udc_done(ep, req, 0);						
		return is_last;
	}	
	else
	{	
    //make sure fifo data exist    
		//udelay(125);		
    count = alidev_udc_set_dma(req, dev, H2D, buf, idx);     	            
    req->req.actual += count;
    alidev_udc_done(ep, req, 0);
		return is_last;				
	}
}

static int alidev_udc_read_fifo_crq(struct alidev_udc *dev, struct usb_ctrlrequest *crq)
{
	unsigned char *outbuf = (unsigned char*)crq;
	int bytes_read = 0;
	int i=0;

	bytes_read = 8;

	if (bytes_read > sizeof(struct usb_ctrlrequest))
		bytes_read = sizeof(struct usb_ctrlrequest);

	for (i=0; i<bytes_read; i++)
	{
  	if ((udc_read(dev, USBD_EP0_FCTR) & EP_ALL_EMPTY))
  	{                        
    	udelay(125);
  	}	
		outbuf[i] = udc_read(dev, USBD_EP0_FIFO);
  }
		
	for (i=0;i<8;i++)
	{
		UDC_DBG("%02x ",outbuf[i]);
	}	
	UDC_DBG("-\n");	
	
	if ((outbuf[0] == 0x80) &&
		(outbuf[1] == 0x06) &&
		(outbuf[2] == 0x04) &&
		(outbuf[3] == 0x03)&&
		(outbuf[4] == 0x09)&&
		(outbuf[5] == 0x04))
	{
		if (!manual_set_cfg_done)
		{	
			monitor_cfg = 1;
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(1));	 		
		}	
	}	
	return bytes_read;
}

/*------------------------- usb state machine -------------------------------*/
static int alidev_udc_set_halt(struct usb_ep *_ep, int value);
static void alidev_udc_handle_get_maxlun(struct alidev_udc *dev ,struct alidev_ep *ep, struct usb_ctrlrequest *crq)
{
	int ret;
	alidev_udc_nuke(dev, ep, -EPROTO);
	crq->bRequestType = 0xA1;
	crq ->bRequest = 0xFE;
	crq->wValue=0x00;
	crq->wIndex=0x00;
	crq->wLength=0x01;

	dev->req_std = (crq->bRequestType & USB_TYPE_MASK)
		== USB_TYPE_STANDARD;
	dev->req_config = 0;
	dev->req_pending = 1;

	if (crq->bRequestType & USB_DIR_IN)
		dev->ep0state = EP0_IN_DATA_PHASE;
	else
		dev->ep0state = EP0_OUT_DATA_PHASE;

	ret = dev->driver->setup(&dev->gadget, crq);
	if (ret < 0) {
		if (dev->req_config) {
			dev->ep0state = EP0_IDLE;
			return;
		}
		
		udelay(5);
		dev->ep0state = EP0_IDLE;
	} else if (dev->req_pending) {
		dev->req_pending=0;
	}
	dev->ep0state = EP0_IDLE;	
}

int alidev_udc_handle_set_config(struct alidev_udc *dev ,struct alidev_ep *ep,
		 struct usb_ctrlrequest *crq)
{
	unsigned char *outbuf = (unsigned char*)crq;
	int ret =0;

	outbuf[0]=0x00;
	outbuf[1]=0x09;
	outbuf[2]=0x01;
	outbuf[3]=0x00;
	outbuf[4]=0x00;
	outbuf[5]=0x00;
	outbuf[6]=0x00;
	outbuf[7]=0x00;
	dev->req_config = 1;
	dev->usb_auto_setconfig=1;	
	return ret;
}

static void alidev_udc_handle_ep0_idle(struct alidev_udc *dev,
					struct alidev_ep *ep,
					struct usb_ctrlrequest *crq
					)
{
	int len, ret;
	
	if (manual_set_cfg)
	{
		printk("Set Config\n");
		manual_set_cfg = 0;
		manual_set_cfg_done = 1;
		crq->bRequestType = 0x0;
		crq->bRequest = 0x09;
		crq->wValue = 0x1;
		crq->wIndex = 0x0;
		crq->wLength = 0x0;			
	}	
	else
	{			
		/* read 8 byte setup packet */
		alidev_udc_nuke(dev, ep, -EPROTO);
		len = alidev_udc_read_fifo_crq(dev, crq);
		if (len != sizeof(*crq)) {			
			printk("%s(): len !sizeof(*crq), len=%d, sizeof(*crq)=%d\n",
					__func__, len, sizeof(*crq));
			return;
		}	
	}   
  
  /* copy with automagic for some standard requests. */
	dev->req_std = (crq->bRequestType & USB_TYPE_MASK)
		== USB_TYPE_STANDARD;
	dev->req_config = 0;
	dev->req_pending = 1;
	switch (crq->bRequest) 
	{
		case 0xFE:
			UDC_DBG("alidev_udc_handle_set_config ...\n");
			alidev_udc_handle_set_config(dev, ep, crq);
			break;	
		default:		
			break;
	}

	if (crq->bRequestType & USB_DIR_IN)
		dev->ep0state = EP0_IN_DATA_PHASE;
	else
		dev->ep0state = EP0_OUT_DATA_PHASE;
	
	ret = dev->driver->setup(&dev->gadget, crq);
	if (ret < 0)
	{
		if (dev->req_config) 
		{		
			printk("%s(): config change %02x fail %d?\n",
					__func__, crq->bRequest, ret);
			dev->ep0state = EP0_IDLE;
			return;
		}
		
		if (ret == -EOPNOTSUPP)
			printk("Operation not supported\n");
		
		dev->ep0state = EP0_IDLE;		
	}
	else if (dev->req_pending)
	{		
		dev->req_pending=0;
	}	
}

static void alidev_udc_handle_ep0(struct alidev_udc *dev)
{
	struct alidev_ep	*ep = &dev->ep[0];
	struct alidev_request	*req;
	struct usb_ctrlrequest	crq;

	if(udc_read(dev, USBD_EP0_FCTR) & EP0_SETUP)
	{
			UDC_DBG("%s():setup, dev->ep0state %d\n",__func__, dev->ep0state);		
			udc_write(dev, udc_read(dev, USBD_EP0_FCTR) | EP0_SETUP, USBD_EP0_FCTR);
                     
			if (list_empty(&ep->queue))
				req = NULL;
			else
				req = list_entry(ep->queue.next, struct alidev_request, queue);
	
			switch (dev->ep0state)
			{
				case EP0_IDLE:
					alidev_udc_handle_ep0_idle(dev, ep, &crq);
					break;		

				case EP0_IN_DATA_PHASE:			/* GET_DESCRIPTOR etc */
					if (req) 
					{
						alidev_udc_write_ep0_fifo(dev, ep, req);
						dev->ep0state = EP0_IDLE;
					}
					break;
	
				case EP0_OUT_DATA_PHASE:		/* SET_DESCRIPTOR etc */
					if (req) 
					{
						alidev_udc_read_ep0_fifo(dev, ep, req);
						dev->ep0state = EP0_IDLE;
					}
					break;
		
				case EP0_END_XFER:
					dev->ep0state = EP0_IDLE;
					break;

				case EP0_STALL:
					dev->ep0state = EP0_IDLE;
					break;
			}
	}
}

/*
 *	handle_ep - Manage I/O endpoints
 */

static void alidev_udc_handle_ep(struct alidev_udc *dev, struct alidev_ep *ep)
{
	struct alidev_request	*req;
	int	is_in = ep->bEndpointAddress & USB_DIR_IN;
	u32	idx;

	if (likely (!list_empty(&ep->queue)))
	{
		req = list_entry(ep->queue.next,
				struct alidev_request, queue);		
	}
	else
	{				
		printk("%s, exit line %d, ep->bEndpointAddress=%d\n", __func__, __LINE__, ep->bEndpointAddress);			
		return;	
	}

	UDC_DBG("alidev_udc_handle_ep: %x, req: %p\n",  ep->bEndpointAddress, req);	    
	idx = ep->bEndpointAddress & 0x7F;

	if (!is_in) 
	{		
		if (req) 
		{					
			alidev_udc_read_fifo(dev, ep, req);		
		} 
	} 
	else 
	{
		if (req) 
		{			
			alidev_udc_write_fifo(dev, ep, req);
		}
	}
}

static int alidev_udc_set_pullup(struct alidev_udc *dev, int is_on)
{
	UDC_DBG("%s()\n", __func__);
  if (is_on)
		alidev_udc_enable(dev);
	else  
		alidev_udc_disable(dev);
		
	return 0;    
}


static int alidev_udc_vbus_session(struct usb_gadget *gadget, int is_active)
{
	struct alidev_udc *udc = to_alidev_udc(gadget);

	UDC_DBG("%s()\n", __func__);
	udc->vbus = (is_active != 0);
	alidev_udc_set_pullup(udc, is_active);
	return 0;
}

/*
 *	alidev_udc_irq - interrupt handler
 */
static irqreturn_t alidev_udc_irq(int dummy, void *_dev)
{
	struct alidev_udc *dev = _dev;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);

	/* Driver connected ? */ 
	if (!dev->driver) 
	{

	}
	/* RESET */	
	dev->usb_dev_int |= udc_read(dev, USBD_INTFLAG);
	dev->usb_intf_isr = udc_read(dev, USBD_INTFLAG1);

	if (dev->usb_intf_isr & DMA_IE1_EN)
	{
		UDC_DBG("(UDC INT) EPA/B DMA xfer done\n");
		dev->usb_intf_isr &= ~DMA_IE1_EN;
		complete(&dev->dma_epab_completion);		
	}

#ifdef SUPPORT_C3921
  if (dev->usb_intf_isr & DMA_IE2_EN)
	{
		UDC_DBG("(UDC INT) EPD/E DMA xfer done\n");		
		dev->usb_intf_isr &= ~DMA_IE2_EN;
		complete(&dev->dma_epde_completion);
	}

  if (dev->usb_intf_isr & EPD_TX_INT)
	{
		UDC_DBG("(UDC INT) EPD TX xfer done\n");
		dev->usb_intf_isr &= ~EPD_TX_INT;		
	}

  if (dev->usb_intf_isr & EPE_RX_INT)
	{
		UDC_DBG("(UDC INT) EPE xfer(Host OUT)\n");				
		dev->usb_intf_isr &= ~EPE_RX_INT;
	}

  if (dev->usb_intf_isr & EPF_TX_INT)
	{
		UDC_DBG("(UDC INT) EPF TX xfer done\n");
		dev->usb_intf_isr &= ~EPF_TX_INT;
	}

	dev->usb_intf_isr = udc_read(dev, USBD_INTFLAG2);

  if (dev->usb_intf_isr & DMA_IE3_EN)
	{
		UDC_DBG("(UDC INT) EPG/H DMA xfer done\n");
		dev->usb_intf_isr &= ~DMA_IE3_EN;
		complete(&dev->dma_epgh_completion);
	}

  if (dev->usb_intf_isr & DMA_IE4_EN)
	{
		UDC_DBG("(UDC INT) EPI/J DMA xfer done\n");
		dev->usb_intf_isr &= ~DMA_IE4_EN;
		complete(&dev->dma_epij_completion);
	}

  if (dev->usb_intf_isr & EPG_TX_INT)
	{
		UDC_DBG("(UDC INT) EPG TX xfer done\n");
		dev->usb_intf_isr &= ~EPG_TX_INT;		
	}

  if (dev->usb_intf_isr & EPH_RX_INT)
	{		
		UDC_DBG("(UDC INT) EPH RX INT(Host OUT), EPH_FCTR %02x\n", udc_read(dev, USBD_EPH_FCTR));				
		dev->usb_intf_isr &= ~EPH_RX_INT;
	}

  if (dev->usb_intf_isr & EPI_TX_INT)
	{
		UDC_DBG("(UDC INT) EPI TX xfer done\n");
		dev->usb_intf_isr &= ~EPI_TX_INT;		
	}

  if (dev->usb_intf_isr & EPJ_RX_INT)
	{
		UDC_DBG("(UDC INT) EPJ RX INT(Host OUT), EPJ_FCTR %02x\n", udc_read(dev, USBD_EPJ_FCTR));				
		dev->usb_intf_isr &= ~EPJ_RX_INT;
	}

	dev->usb_intf_isr = udc_read(dev, USBD_INTFLAG3);
  if (dev->usb_intf_isr & DMA_IE5_EN)
	{
		UDC_DBG("(UDC INT) EPK/L DMA xfer done\n");
		dev->usb_intf_isr &= ~DMA_IE5_EN;
		complete(&dev->dma_epkl_completion);
	}

  if (dev->usb_intf_isr & EPK_TX_INT)
	{
		UDC_DBG("(UDC INT) EPK TX xfer done\n");
		dev->usb_intf_isr &= ~EPK_TX_INT;		
	}

  if (dev->usb_intf_isr & EPL_RX_INT)
	{
		UDC_DBG("(UDC INT) EPL xfer(Host OUT)\n");				
		dev->usb_intf_isr &= ~EPL_RX_INT;
	}    
#endif
       
	 if(dev->usb_dev_int & RESET_INT)
	{		
		/*turn off 5V*/
		UDC_DBG("(UDC INT) Host RST\n");	
		if (dev->usb_5v_control_gpio > 0)
			gpio_direction_output(dev->usb_5v_control_gpio, 0);		
		dev->usb_dev_int &= ~0xC0;
		dev->ep0state = EP0_IDLE;		
	}

	 if(dev->usb_dev_int & SUS_RES_INT)
	{
		UDC_DBG("(UDC INT) Suspend\n");
    printk("(UDC INT) Suspend\n");
		dev->usb_dev_int &= ~SUS_RES_INT;		
		dev->dev_ep0_rx = 0;		
		dev->vbus = 0;     
    wait_epb_rx_data = 0;	/*stop service EPB*/
    monitor_cfg = 0;
    manual_set_cfg = 0;
    manual_set_cfg_done = 0;
    
#ifdef SUPPORT_C3921    
    wait_epe_rx_data = 0;	/*stop service EPE*/
    wait_eph_rx_data = 0;	/*stop service EPH*/
    wait_epj_rx_data = 0;	/*stop service EPJ*/
    wait_epl_rx_data = 0;	/*stop service EPL*/
#endif    
              
    if (dev->gadget.speed != USB_SPEED_UNKNOWN) 
		{
			if (dev->driver && dev->driver->disconnect)
			{	
				dev->driver->disconnect(&dev->gadget);				
			}	
		}

		/* HW initial Device Mode */
		__REG32ALI(C39_DEV_RESET) |= USB_DEV_RESET;
		ndelay(5);
		__REG32ALI(C39_DEV_RESET) &= ~USB_DEV_RESET;
		ndelay(10);		
		printk("Reset device mode\n");	
		
		//__REG32ALI(0x1803d814) = 0x36e836e8;

		alidev_udc_disable(dev);
		alidev_udc_enable(dev);
		dev->ep0state = EP0_IDLE;		
		spin_unlock_irqrestore(&dev->lock, flags);
		return IRQ_HANDLED;
	}

	if(dev->usb_dev_int & VBUS_INT)
	{
		UDC_DBG("(UDC INT) VBUS\n");		
		dev->usb_dev_int &= ~VBUS_INT;
		alidev_udc_vbus_session(&dev->gadget, 1);			
	} 

	if(dev->usb_dev_int & EPC_TX_INT)
	{
		UDC_DBG("(UDC INT) EPC xfer\n");
		dev->usb_dev_int &= ~EPC_TX_INT;
	}
		 
	 if(dev->usb_dev_int & EPA_TX_INT)
	{
		UDC_DBG("(UDC INT) EPA xfer (Host IN)\n");			
		dev->usb_dev_int&= ~EPA_TX_INT;					
	}

	if(dev->usb_dev_int & EPB_RX_INT)
	{		
		UDC_DBG("(UDC INT) EPB xfer(Host OUT)\n");			
		dev->usb_dev_int &= ~EPB_RX_INT;				
	}		 

	 if(dev->usb_dev_int & EP0_TX_INT)
	{
		UDC_DBG("(UDC INT) EP0 (HOST IN)\n");			
		dev->usb_dev_int &= ~EP0_TX_INT;
	}		 

	 if(dev->usb_dev_int & EP0_RX_INT)
	{
		UDC_DBG("(UDC INT) EP0 (HOST OUT)\n");			
		dev->usb_dev_int &= ~EP0_RX_INT;		
		alidev_udc_handle_ep0(dev);
		if(!dev->dev_ep0_rx)
		{
			UDC_DBG("HOST Connected\n");
			dev->dev_ep0_rx=1;
			dev->vbus = 1;
			udc_write(dev, udc_read(dev, USBD_INTENR) & (~RESET_INT), USBD_INTENR); 	//disable reset int
			udc_write(dev, udc_read(dev, USBD_INTENR) | SUS_RES_INT, USBD_INTENR); 		//enable suspend int
		}			
	}		
	spin_unlock_irqrestore(&dev->lock, flags);
	return IRQ_HANDLED;
}
/*------------------------- alidev_ep_ops ----------------------------------*/
/*
 *	alidev_udc_ep_enable
 */
static int alidev_udc_ep_enable(struct usb_ep *_ep,
				 const struct usb_endpoint_descriptor *desc)
{
	struct alidev_udc	*dev;
	struct alidev_ep	*ep;
	u32			max, tmp;
	unsigned long		flags;

	ep = to_alidev_ep(_ep);

	if (!_ep || !desc || ep->desc
			|| ep_is_ep0(_ep)
			|| desc->bDescriptorType != USB_DT_ENDPOINT)
		return -EINVAL;

	dev = ep->dev;
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;

	max = le16_to_cpu(desc->wMaxPacketSize) & 0x1fff;

	local_irq_save (flags);
	_ep->maxpacket = max & 0x7ff;
	ep->desc = desc;
	ep->halted = 0;
	ep->bEndpointAddress = desc->bEndpointAddress;

	/* enable irqs */
	/* print some debug message */
	tmp = desc->bEndpointAddress;	
	local_irq_restore (flags);
	alidev_udc_set_halt(_ep, 0);
	UDC_DBG( "enable %s(%d) ep%x%s-blk max %02x\n", _ep->name,ep->num, tmp,		 
	     desc->bEndpointAddress & USB_DIR_IN ? "in" : "out", max);	
	return 0;
}

/*
 * alidev_udc_ep_disable
 */
static int alidev_udc_ep_disable(struct usb_ep *_ep)
{
	struct alidev_ep *ep = to_alidev_ep(_ep);
       struct alidev_udc	*dev = ep->dev;	
	unsigned long flags;
	
	if (!_ep || !ep->desc) {
		return -EINVAL;
	}
	
	local_irq_save(flags);
	ep->desc = NULL;
	ep->ep.desc = NULL;
	ep->halted = 0/*1*/;
	alidev_udc_nuke (ep->dev, ep, -ESHUTDOWN);
	local_irq_restore(flags);
	UDC_DBG("%s disabled\n", _ep->name);  

	if (strcmp(ep->name,"ep1in-bulk") == 0)
	{
  	alidev_epfifo_rst(dev, EPA);
  }
  else  if (strcmp(ep->name,"ep2out-bulk") == 0)
  {
  	alidev_epfifo_rst(dev, EPB);
    wait_epb_rx_data = 0;
  }

#ifdef SUPPORT_C3921  
  else  if (strcmp(ep->name,"ep4in-bulk") == 0)
  {
  	alidev_epfifo_rst(dev, EPD);            
  }
  else  if (strcmp(ep->name,"ep5out-bulk") == 0)
  {
  	alidev_epfifo_rst(dev, EPE);
    wait_epe_rx_data = 0;
  }
  else  if (strcmp(ep->name,"ep7in-bulk") == 0)
  {
  	alidev_epfifo_rst(dev, EPG);            
  }
  else  if (strcmp(ep->name,"ep8out-bulk") == 0)
  {
  	alidev_epfifo_rst(dev, EPH);
    wait_eph_rx_data = 0;
  }
  else  if (strcmp(ep->name,"ep9in-bulk") == 0)
  {
  	alidev_epfifo_rst(dev, EPI);            
  }
  else  if (strcmp(ep->name,"ep10out-bulk") == 0)
  {
  	alidev_epfifo_rst(dev, EPJ);
    wait_epj_rx_data = 0;
  }
  else  if (strcmp(ep->name,"ep11in-bulk") == 0)
  {
  	alidev_epfifo_rst(dev, EPK);            
  }
  else  if (strcmp(ep->name,"ep12out-bulk") == 0)
  {
  	alidev_epfifo_rst(dev, EPL);
    wait_epl_rx_data = 0;
  }
#endif    
	return 0;
}

/*
 * alidev_udc_alloc_request
 */
static struct usb_request *
alidev_udc_alloc_request(struct usb_ep *_ep, gfp_t mem_flags)
{
	struct alidev_request *req;

	UDC_DBG("%s(%p,%d)\n", __func__, _ep, mem_flags);
	if (!_ep)
		return NULL;

	req = kzalloc(sizeof(struct alidev_request), mem_flags);
	if (!req)
		return NULL;

	INIT_LIST_HEAD (&req->queue);
	return &req->req;
}

/*
 * alidev_udc_free_request
 */
static void
alidev_udc_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct alidev_ep	*ep = to_alidev_ep(_ep);
	struct alidev_request	*req = to_alidev_req(_req);

	UDC_DBG("%s(%p,%p)\n", __func__, _ep, _req);
	
	if (!ep || !_req || (!ep->desc && !ep_is_ep0(_ep)))
		return;

	WARN_ON (!list_empty (&req->queue));
	kfree(req);
}

/*
 *	alidev_udc_queue
 */
static int alidev_udc_queue(struct usb_ep *_ep,	struct usb_request *_req,	gfp_t gfp_flags)
{
	struct alidev_request	*req = to_alidev_req(_req);
	struct alidev_ep	*ep = to_alidev_ep(_ep);
	struct alidev_udc	*dev = ep->dev;	

	if (unlikely (!_ep || !_req || (!ep->desc && !ep_is_ep0(_ep)))) 
	{
		printk( "%s failed : %d \n", __func__,__LINE__);
		return -EINVAL;
	}

	dev = ep->dev;
#if 0
	if (unlikely (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN))
	{
		printk( "%s failed : %d \n", __func__,__LINE__);	
		return -ESHUTDOWN;
	}
#endif
	//local_irq_save (flags);
	if (unlikely(!_req || !_req->complete	|| !_req->buf || !list_empty(&req->queue))) 
	{		
		printk( "%s failed : %d \n", __func__,__LINE__);			
		//local_irq_save (flags);
		return -EINVAL;
	}
	
	//spin_lock_irqsave(&dev->lock, flags);
	_req->status = -EINPROGRESS;
	_req->actual = 0;		
	
	UDC_DBG("%s: ep->bEndpointAddress 0x%x, ep->halted=%d  req->req.length=%d\n", 
		__func__, ep->bEndpointAddress, ep->halted,  req->req.length);
		
	if (list_empty(&ep->queue) && !ep->halted) 	
	{		
		if ((ep->bEndpointAddress & (~USB_DIR_IN)) == 0)  /* ep0 */
		{
			switch (dev->ep0state) 
			{
				case EP0_IN_DATA_PHASE:			/*D2H*/
					udc_write(dev, udc_read(dev, USBD_EP0_FCTR) | IN_DIR, USBD_EP0_FCTR);
					alidev_udc_write_ep0_fifo(dev, ep,req);
					udelay(10);
					udc_write(dev, udc_read(dev, USBD_EP0_FCTR) & (~IN_DIR), USBD_EP0_FCTR);
					dev->ep0state = EP0_IDLE;	
					req = NULL;							
					break;

				case EP0_OUT_DATA_PHASE:		/*H2D*/
					alidev_udc_read_ep0_fifo(dev, ep,req);
					dev->ep0state = EP0_IDLE;
					req = NULL;					
					break;
	
				default:
					//local_irq_restore(flags);
					printk("%s: dev->ep0state=%d, ep->bEndpointAddress=%d, req_length=%d, ep->halted=%d\n", 
						__func__, dev->ep0state, ep->bEndpointAddress, req->req.length, ep->halted);
					printk("%s (%d)  \n", __FUNCTION__, __LINE__);
					//spin_unlock_irqrestore(&dev->lock, flags);
					return -EL2HLT;
			}
		}
		else if((ep->bEndpointAddress & USB_DIR_IN))
		{
			if (alidev_udc_write_fifo(dev, ep, req))
			{
				req = NULL;
			}
		}
		else 
		{			
			if (alidev_udc_read_fifo(dev, ep, req))
			{								
				req = NULL;
			}
		}		
	}
	else
	{
		printk("%s, list_empty(&ep->queue)=%x, ep->halted=%x\n",__func__, list_empty(&ep->queue), ep->halted);	
	}
	
	if(dev->usb_auto_setconfig)
	{
		struct usb_ctrlrequest	crq;
		dev->usb_auto_setconfig = 0;
		UDC_DBG( "%s alidev_udc_handle_get_maxlun\n", __func__);
		alidev_udc_handle_get_maxlun(dev,ep, &crq);
	}	
	return 0;
}

/*
 *	alidev_udc_dequeue
 */
static int alidev_udc_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct alidev_ep	*ep = to_alidev_ep(_ep);
	struct alidev_udc	*udc;
	int			retval = -EINVAL;
	unsigned long		flags;
	struct alidev_request	*req = NULL;

	if (!the_controller->driver)
		return -ESHUTDOWN;

	if (!_ep || !_req)
		return retval;

	udc = to_alidev_udc(ep->gadget);

	local_irq_save (flags);

	list_for_each_entry (req, &ep->queue, queue) {
		if (&req->req == _req) {
			list_del_init (&req->queue);
			_req->status = -ECONNRESET;
			retval = 0;
			break;
		}
	}

	if (retval == 0) {
		alidev_udc_done(ep, req, -ECONNRESET);
	}

	local_irq_restore (flags);
	return retval;
}

/*
 * alidev_udc_set_halt
 */
static int alidev_udc_set_halt(struct usb_ep *_ep, int value)
{
	struct alidev_ep	*ep = to_alidev_ep(_ep);
  struct alidev_udc *dev = ep->dev;
	unsigned long		flags;
	u32		ctrl_reg;
	u32		idx;

	if (unlikely (!_ep || (!ep->desc && !ep_is_ep0(_ep)))) {
		UDC_DBG( "%s: inval 2\n", __func__);
		return -EINVAL;
	}
	
	local_irq_save (flags);
	idx = ep->bEndpointAddress & 0x7F;  
	UDC_DBG("%s, idx=%x, value=%x\n",__func__,  idx, value);
	
        switch(idx)
	{
		case EP0:
			ctrl_reg = USBD_EP0_FCTR;
			break;
		case EPA:
			ctrl_reg = USBD_EPA_FCTR;
			break;
		case EPB:
			ctrl_reg = USBD_EPB_FCTR;
			break;
    case EPC:
			ctrl_reg = USBD_EPC_FCTR;
			break;
		case EPD:
			ctrl_reg = USBD_EPD_FCTR;
			break;

#ifdef SUPPORT_C3921			
		case EPE:
			ctrl_reg = USBD_EPE_FCTR;
			break;
		case EPF:
			ctrl_reg = USBD_EPF_FCTR;
			break;
		case EPG:
			ctrl_reg = USBD_EPG_FCTR;
			break;
		case EPH:
			ctrl_reg = USBD_EPH_FCTR;
			break;
		case EPI:
			ctrl_reg = USBD_EPI_FCTR;
			break;
		case EPJ:
			ctrl_reg = USBD_EPJ_FCTR;
			break;
		case EPK:
			ctrl_reg = USBD_EPK_FCTR;
			break;	
		case EPL:
			ctrl_reg = USBD_EPL_FCTR;
			break;	
#endif
                default:
			//ctrl_reg = USBD_EP0_FCTR;     
		    printk("[UDC Warning] : Fix me, wrong end point, %s: bEndpointAddress = %d, \n",__func__, ep->bEndpointAddress);	   	
	   	    return 0;
       }

	if (idx == 0) {
		if (value)
			udc_write(dev, udc_read(dev, USBD_EP0_FCTR) | EP_STALL, USBD_EP0_FCTR);
		else
			udc_write(dev, udc_read(dev, USBD_EP0_FCTR) & ~EP_STALL, USBD_EP0_FCTR);
	} 
	else 
	{		
		if ((ep->bEndpointAddress & USB_DIR_IN) != 0) 
		{
			if (value)
			{
				udc_write(dev, udc_read(dev, ctrl_reg)| EP_STALL, ctrl_reg);
				while (udc_read(dev, ctrl_reg) & EP_STALL);        	
				value = 0;
			}	
			else 
				udc_write(dev, udc_read(dev, ctrl_reg) & ~EP_STALL, ctrl_reg);
		}
		else
		{
			if (value)
				udc_write(dev, udc_read(dev, ctrl_reg) | EP_STALL, ctrl_reg);
			else 
				udc_write(dev, udc_read(dev, ctrl_reg) & ~EP_STALL, ctrl_reg);			
		}		
	}
	
	ep->halted = value ? 1 : 0;	
	local_irq_restore (flags);

	UDC_DBG("%s(): ep->bEndpointAddress=0x%x, value %d \n",
		__func__,ep->bEndpointAddress,value) ;
	return 0;
}


static const struct usb_ep_ops alidev_ep_ops = {
	.enable		= alidev_udc_ep_enable,
	.disable	= alidev_udc_ep_disable,

	.alloc_request	= alidev_udc_alloc_request,
	.free_request	= alidev_udc_free_request,

	.queue		= alidev_udc_queue,
	.dequeue	= alidev_udc_dequeue,

	.set_halt	= alidev_udc_set_halt,
};

/*------------------------- usb_gadget_ops ----------------------------------*/

/*
 *	alidev_udc_get_frame
 */
static int alidev_udc_get_frame(struct usb_gadget *_gadget)
{
	int tmp=0;
	UDC_DBG("%s()\n", __func__);
	return tmp;
}

/*
 *	alidev_udc_wakeup
 */
static int alidev_udc_wakeup(struct usb_gadget *_gadget)
{
	UDC_DBG("%s()\n", __func__);
	return 0;
}

/*
 *	alidev_udc_set_selfpowered
 */
static int alidev_udc_set_selfpowered(struct usb_gadget *gadget, int value)
{
	struct alidev_udc *udc = to_alidev_udc(gadget);

	UDC_DBG("%s()\n", __func__);
	if (value)
		udc->devstatus |= (1 << USB_DEVICE_SELF_POWERED);
	else
		udc->devstatus &= ~(1 << USB_DEVICE_SELF_POWERED);

	return 0;
}

static void alidev_udc_disable(struct alidev_udc *dev);
static void alidev_udc_enable(struct alidev_udc *dev);

static int alidev_udc_pullup(struct usb_gadget *gadget, int is_on)
{
	struct alidev_udc *udc = to_alidev_udc(gadget);

	UDC_DBG("%s()\n", __func__);
	alidev_udc_set_pullup(udc, is_on ? 1 : 0);
	return 0;
}

static int alidev_vbus_draw(struct usb_gadget *_gadget, unsigned ma)
{
	UDC_DBG("%s()\n", __func__);

	return -ENOTSUPP;
}

#ifdef SUPPORT_ANDROID_UDC    
static int ali_udc_start(struct usb_gadget_driver *driver,
		int (*bind)(struct usb_gadget *));
static int ali_udc_stop(struct usb_gadget_driver *driver);
#else
static int ali_udc_start(struct usb_gadget *, struct usb_gadget_driver *);
static int ali_udc_stop(struct usb_gadget *, struct usb_gadget_driver *);
#endif

static const struct usb_gadget_ops alidev_ops = {
	.get_frame		 = alidev_udc_get_frame,
	.wakeup			 = alidev_udc_wakeup,
	.set_selfpowered = alidev_udc_set_selfpowered,
	.pullup			 = alidev_udc_pullup,
	.vbus_session	 = alidev_udc_vbus_session,
	.vbus_draw		 = alidev_vbus_draw,

#ifdef SUPPORT_ANDROID_UDC    
	.start		= ali_udc_start,
	.stop			= ali_udc_stop,
#else		
	.udc_start	 = ali_udc_start,
	.udc_stop		 = ali_udc_stop,
#endif
};

/*------------------------- gadget driver handling---------------------------*/
/*
 * alidev_udc_disable
 */
static void alidev_udc_disable(struct alidev_udc *dev)
{
	u8 temp;
	
	UDC_DBG("%s\n",__func__);
	//udc_write(dev, 0x00, USBD_INTENR);		//disable all dev int
	//udc_write(dev, 0x00, USBD_INTENR1); 	//disable all usb intf int

	temp = udc_read(dev, USBD_INTFLAG);		//read clear
	temp = udc_read(dev, USBD_INTFLAG1);	//read clear

	//udc_write(dev, 0x00, USBG_DEVSETR);		//diable dev mode
	/* Good bye, cruel world */
	//if (udc_info && udc_info->udc_command)
	//udc_info->udc_command(ALIDEV_UDC_P_DISABLE);

	/* Set speed to unknown */
	//dev->gadget.speed = USB_SPEED_UNKNOWN;
	//turn on 5V
	if (dev->usb_5v_control_gpio > 0)
		gpio_direction_output(dev->usb_5v_control_gpio, 1);

   udc_write(dev, SELF_PWDED, USBD_DEVSETR);   //USB 
   monitor_cfg = 0;
   manual_set_cfg = 0;
   manual_set_cfg_done = 0;	
}

/*
 * alidev_udc_reinit
 */
static void alidev_udc_reinit(struct alidev_udc *dev)
{
	u32 i;

  //UDC_DBG("%s\n",__func__);
	/* device/ep0 records init */
	INIT_LIST_HEAD (&dev->gadget.ep_list);
	INIT_LIST_HEAD (&dev->gadget.ep0->ep_list);
	dev->ep0state = EP0_IDLE;

	for (i = 0; i < ALIDEV_ENDPOINTS; i++) {
		struct alidev_ep *ep = &dev->ep[i];

		if (i != 0)
		{
			UDC_DBG("%s: add list %d\n",__func__,i);
			list_add_tail (&ep->ep.ep_list, &dev->gadget.ep_list);
		}	
		ep->dev = dev;
		ep->desc = NULL;
		ep->ep.desc = NULL;
		ep->halted = 0;
		INIT_LIST_HEAD (&ep->queue);
	}
}

/*
 * alidev_udc_enable
 */
static void alidev_udc_enable(struct alidev_udc *dev)
{
	u8 temp;

	UDC_DBG("%s\n",__func__);

	dev->gadget.speed = USB_SPEED_HIGH;
	temp = udc_read(dev, USBD_INTFLAG);		//read clear
	temp = udc_read(dev, USBD_INTFLAG1);		//read clear

#ifdef SUPPORT_C3921
	temp = udc_read(dev, USBD_INTFLAG2);		//read clear
	temp = udc_read(dev, USBD_INTFLAG3);		//read clear
#endif		

	//dma init
	udc_write(dev, DMA_INIT, USBD_DMACTL);
	udc_write(dev, 0, USBD_DMACTL);
	udc_write(dev, 0, USBD_DMACNTL);
	udc_write(dev, 0, USBD_DMACNTM);
	udc_write(dev, 0, USBD_DMACNTH);
#ifdef SUPPORT_C3921
	udc_write(dev, DMA_INIT, USBD_DMA1CTL);
	udc_write(dev, 0, USBD_DMA1CTL);
	udc_write(dev, 0, USBD_DMA1CNTL);
	udc_write(dev, 0, USBD_DMA1CNTM);
	udc_write(dev, 0, USBD_DMA1CNTH);
	udc_write(dev, DMA_INIT, USBD_DMA2CTL);
	udc_write(dev, 0, USBD_DMA2CTL);
	udc_write(dev, 0, USBD_DMA2CNTL);
	udc_write(dev, 0, USBD_DMA2CNTM);
	udc_write(dev, 0, USBD_DMA2CNTH);
	udc_write(dev, DMA_INIT, USBD_DMA3CTL);
	udc_write(dev, 0, USBD_DMA3CTL);
	udc_write(dev, 0, USBD_DMA3CNTL);
	udc_write(dev, 0, USBD_DMA3CNTM);
	udc_write(dev, 0, USBD_DMA3CNTH);
	udc_write(dev, DMA_INIT, USBD_DMA4CTL);
	udc_write(dev, 0, USBD_DMA4CTL);
	udc_write(dev, 0, USBD_DMA4CNTL);
	udc_write(dev, 0, USBD_DMA4CNTM);
	udc_write(dev, 0, USBD_DMA4CNTH);
#endif	

	for (temp=0; temp <ALIDEV_ENDPOINTS; temp++)
		alidev_epfifo_rst(dev, temp);
	
	udc_write(dev, EP_BULK | EP_NUM(1), USBD_EPA_SETR);		//EPA BULK EP#1
	udc_write(dev, EP_BULK | EP_NUM(2), USBD_EPB_SETR);		//EPB BULK EP#2
	udc_write(dev, EP_INT | EP_NUM(3), USBD_EPC_SETR);		//EPC INT EP#3

#ifdef SUPPORT_C3921	
	udc_write(dev, EP_BULK | EP_NUM(4), USBD_EPD_SETR);		//EPD BULK EP#4
	udc_write(dev, EP_BULK | EP_NUM(5), USBD_EPE_SETR);		//EPE BULK EP#5
	udc_write(dev, EP_INT | EP_NUM(6), USBD_EPF_SETR);		//EPF INT EP#6
	udc_write(dev, EP_BULK | EP_NUM(7), USBD_EPG_SETR);		//EPG BULK EP#7
	udc_write(dev, EP_BULK | EP_NUM(8), USBD_EPH_SETR);		//EPH BULK EP#8
	udc_write(dev, EP_BULK | EP_NUM(9), USBD_EPI_SETR);		//EPI INT EP#9
	udc_write(dev, EP_BULK | EP_NUM(10), USBD_EPJ_SETR);		//EPJ BULK EP#10
	udc_write(dev, EP_BULK | EP_NUM(11), USBD_EPK_SETR);	//EPK BULK EP#11
	udc_write(dev, EP_BULK | EP_NUM(12), USBD_EPL_SETR);	//EPL INT EP#12
#endif	

	udc_write(dev,  (RESET_INT | VBUS_INT | 
                   EPC_TX_INT | EP0_RX_INT | EP0_TX_INT), 
	                 USBD_INTENR);		        //Enable all Dev int, no suspend INT
	udc_write(dev, DMA_IE1_EN, USBD_INTENR1);    //DMA finish INT for EPA and EPB

#ifdef SUPPORT_C3921	   
	 udc_write(dev, (udc_read(dev,USBD_INTENR1) | DMA_IE2_EN), 
                   USBD_INTENR1);  //DMA finish INT for EPD and EPE

   udc_write(dev, (DMA_IE3_EN | DMA_IE4_EN),
	                 USBD_INTENR2);		        //DMA finish INT for EPI/J and EPG/H

   udc_write(dev, DMA_IE5_EN, USBD_INTENR3);	//DMA finish INT for EPL/K
#endif 

	udc_write(dev, USBD_ON | SELF_PWDED, USBD_DEVSETR);   //USB ON
    
	dev->usb_intf_isr=0;
	dev->usb_dev_int=0;
	dev->usb_auto_setconfig=0;	
	dev->dev_ep0_rx=0;
}


#ifdef SUPPORT_ANDROID_UDC
static int ali_udc_start(struct usb_gadget_driver *driver,
		int (*bind)(struct usb_gadget *))
#else
static int ali_udc_start(struct usb_gadget *g, struct usb_gadget_driver *driver)
#endif		
		
{
	struct alidev_udc *udc = the_controller;
	int		retval;

	UDC_DBG("%s() '%s'\n", __func__, driver->driver.name);
	/* Sanity checks */
	if (!udc)
		return -ENODEV;
       
    if (udc->driver)
		return -EBUSY;

#ifdef SUPPORT_ANDROID_UDC      
	if (!bind || !driver->setup || driver->max_speed < USB_SPEED_FULL) {
		printk(KERN_ERR "Invalid driver: bind %p setup %p speed %d\n",
			bind, driver->setup, driver->max_speed);
		return -EINVAL;
	}            

	#if defined(MODULE)
		if (!driver->unbind) 
		{
			printk(KERN_ERR "Invalid driver: no unbind method\n");
			return -EINVAL;
		}
	#endif

	/* Hook the driver */
	udc->driver = driver;
	udc->gadget.dev.driver = &driver->driver;

	/* Bind the driver */
	if ((retval = device_add(&udc->gadget.dev)) != 0) 
	{
		printk(KERN_ERR "Error in device_add() : %d\n",retval);
		goto register_error;
	}
	UDC_DBG( "binding gadget driver '%s'\n", driver->driver.name);

	if ((retval = bind(&udc->gadget)) != 0) 
	{
		UDC_DBG("%s (%d)  ng\n", __FUNCTION__, __LINE__);	    
		device_del(&udc->gadget.dev);
		goto register_error;
	}
 
	UDC_DBG("%s (%d)  ok\n", __FUNCTION__, __LINE__);
#endif
	/* Enable udc */
	alidev_udc_enable(udc);
	UDC_DBG("%s (%d)  ok\n", __FUNCTION__, __LINE__);	
	return 0;

#ifdef SUPPORT_ANDROID_UDC   
register_error:
#endif
	udc->driver = NULL;
	udc->gadget.dev.driver = NULL;
	return retval;
}

#ifdef SUPPORT_ANDROID_UDC
static int ali_udc_stop(struct usb_gadget_driver *driver)
#else
static int ali_udc_stop(struct usb_gadget *g, struct usb_gadget_driver *driver)
#endif
{
	struct alidev_udc *udc = the_controller;

	if (!udc)
		return -ENODEV;

#ifdef SUPPORT_ANDROID_UDC
	if (!driver || driver != udc->driver || !driver->unbind)
		return -EINVAL;

	UDC_DBG("usb_gadget_unregister_driver() '%s'\n",	driver->driver.name);

	/* report disconnect */
	if (driver->disconnect)
		driver->disconnect(&udc->gadget);

	driver->unbind(&udc->gadget);
	device_del(&udc->gadget.dev);
	udc->driver = NULL;
#endif	
	/* Disable udc */
	alidev_udc_disable(udc);
	return 0;
}

/*
	use for respond clear stall, when host received stall, device need clear
	stall state automaticlly.
*/


static void ali_udc_timer(unsigned long handle)
{	
struct alidev_udc *dev = (void *)handle;	
unsigned long flag;

struct alidev_ep	*ep = &dev->ep[0];
struct usb_ctrlrequest	crq;

	spin_lock_irqsave(&dev->lock1, flag);  		
  if (monitor_cfg)
  {			
  	if ((udc_read(dev, USBD_MISCCTL) & USB_CFG) == USB_CFG)
    { 	
  		printk("Configed\n");
  		monitor_cfg = 0;
  		manual_set_cfg = 1;
  		alidev_udc_handle_ep0_idle(dev, ep, &crq);
    }
    else
    	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(1));	 
  }		
	
	if (wait_epb_rx_data)
	{			
		if ((udc_read(dev, USBD_EPB_FCTR) & EP_ALL_EMPTY))
		{	
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));			
		}	
		else
		{		
			alidev_udc_handle_ep(dev, &dev->ep[EPB]);			
      wait_epb_rx_data--;
      if (wait_epb_rx_data)
				mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
		}	
	}		

#ifdef SUPPORT_C3921
  if (wait_epe_rx_data)
	{	
		if ((udc_read(dev, USBD_EPE_FCTR) & EP_ALL_EMPTY))
		{	
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
		}	
		else
		{	
			alidev_udc_handle_ep(dev, &dev->ep[EPE]);
			wait_epe_rx_data--;
             if (wait_epe_rx_data)
      mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
		}	
  }

  if (wait_eph_rx_data)
	{	
		if ((udc_read(dev, USBD_EPH_FCTR) & EP_ALL_EMPTY))
		{	
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));								
		}	
		else
		{	
			alidev_udc_handle_ep(dev, &dev->ep[EPH]);						      
			wait_eph_rx_data--;    
        if (wait_eph_rx_data)  
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));            
		}	
  }

  if (wait_epj_rx_data)
	{	
		if ((udc_read(dev, USBD_EPJ_FCTR) & EP_ALL_EMPTY))
		{	
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));								
		}	
		else
		{	
			alidev_udc_handle_ep(dev, &dev->ep[EPJ]);						
			wait_epj_rx_data--;
         if (wait_epj_rx_data)
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
		}	
  }
  
    if (wait_epl_rx_data)
	{	
		if ((udc_read(dev, USBD_EPL_FCTR) & EP_ALL_EMPTY))
		{	
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));								
		}	
		else
		{	
			alidev_udc_handle_ep(dev, &dev->ep[EPL]);						
			wait_epl_rx_data--;
        if (wait_epl_rx_data)
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
		}	
  }
#endif    
	spin_unlock_irqrestore(&dev->lock1, flag);	
}

/*
 *	probe - binds to the platform device
 */
void alidev_ep_setup(struct alidev_udc *udc, u8 num, const char* name, unsigned short fifo_size, u8 bmAttributes)
{
    struct alidev_ep* ali_ep = &udc->ep[num];
    
    strcpy(ali_ep->name, name);    
    ali_ep->gadget           = &udc->gadget;
    ali_ep->dev              = udc;
    ali_ep->num              = num;
    ali_ep->fifo_size        = fifo_size;    
    ali_ep->bEndpointAddress = num;
    ali_ep->bmAttributes     = bmAttributes;   
    ali_ep->ep.name          = ali_ep->name;
    ali_ep->ep.ops		     = &alidev_ep_ops;
    ali_ep->ep.maxpacket	 = (bmAttributes == USB_ENDPOINT_XFER_INT) ? ALI_EPC_FIFO_SIZE : fifo_size;
}

static int alidev_udc_probe(struct platform_device *pdev)
{
	struct alidev_udc   *udc = NULL;    
  struct resource     *res = NULL; 
  struct ali_usb_gadget_platform_data 
  {
  	int *usb_5v_control_gpio;   /* for C3921 BGA GPIO 73 control usb port 0 5V */
    int	*usb_5v_detect_gpio;    /* for C3921 BGA GPIO 84 control usb port device port 5V detect */
  } *platform_data = pdev->dev.platform_data;          
	int                 irq;   
	int                 retval;	
	int i;
 	
  UDC_DBG("ALi UDC Debug: %s %s\n", __FILE__, __FUNCTION__);  
  printk("%s, version %s\n",__FUNCTION__, ALI_UDC_DRIVER_VERSION);
    
  /* Get Platform Data & Resource from Platform Device */      
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);   // Get ALi USB Device Hardware Base Address 	
	if (!res) 
	{  		
		retval = -ENODEV;
		dev_err(&pdev->dev, "%s platform_get_resource error.\n", __FUNCTION__);	    
	    goto err_res;   	
	}     
  UDC_DBG("ALi UDC Debug: %s %s platform_get_resource res->start = 0x%x, resource_size = 0x%x\n", 
              __FILE__, __FUNCTION__, res->start, resource_size(res));   
 
	platform_data = pdev->dev.platform_data;   // Get Platform Data: GPIO 5v Power Detect & Control			
	if(platform_data!= NULL) 
	{  
	  UDC_DBG("ALi UDC Debug: USB 5V control gpio = %d\n", *platform_data->usb_5v_control_gpio );
	  UDC_DBG("ALi UDC Debug: USB 5V detect gpio = %d\n", *platform_data->usb_5v_detect_gpio );	    	    	   	   
	} 
 
	irq = platform_get_irq(pdev, 0);                        // Get IRQ Number	
	if (irq < 0) 
	{
	  retval = -ENODEV;
		dev_err(&pdev->dev, "%s platform_get_irq error.\n", __FUNCTION__);
		goto err_res; 
	} 	
  UDC_DBG("ALi UDC Debug: %s %s platform_get_irq irq = %d\n", __FILE__, __FUNCTION__, irq);  
 
 	/* Allocate and initialize ali udc */
	udc = kzalloc(sizeof(struct alidev_udc), GFP_KERNEL);       // Allocate alidev_udc
	if(udc == NULL) 
	{
		retval = -ENOMEM;
		dev_err(&pdev->dev, "%s kzalloc error\n", __FUNCTION__);
		goto err_res;
	}

	spin_lock_init (&udc->lock); 
  udc->base_addr = ioremap(res->start, resource_size(res));   // Assign hardware base address, and ioremap
	if (udc->base_addr == NULL) 
	{
		retval = -ENOMEM;
		dev_err(&pdev->dev, "%s ioremap error.\n", __FUNCTION__);
		goto err_mem;
	}
	UDC_DBG("%s (%d) base_addr = 0x%p \n", __FUNCTION__, __LINE__, udc->base_addr);
	
	if(platform_data!= NULL) // Enable 5V GPIO Control and Detec.
	{                                  
  	udc->usb_5v_control_gpio = *platform_data->usb_5v_control_gpio;
    udc->usb_5v_detect_gpio  = *platform_data->usb_5v_detect_gpio;   
        
    if (udc->usb_5v_control_gpio > 0)
    {	        	
	  	retval = gpio_request(udc->usb_5v_control_gpio, "gadget-5v-control");
	    if (retval) 
	    {
	    	dev_err(&pdev->dev, "gpio_request(gadget-5v-control %d) error %d\n", udc->usb_5v_control_gpio, retval);
	      goto err_map;
	    } 
    }
        	
    if (udc->usb_5v_detect_gpio > 0)	
    {	
	  	retval = gpio_request(udc->usb_5v_detect_gpio, "gadget-5v-detect");
	    if (retval) 
	    {
	    	dev_err(&pdev->dev, "gpio_request(gadget-5v-detect %d) error %d\n", udc->usb_5v_detect_gpio, retval);
	      goto err_map;
	    }         	    	   	   
	  }  
	} 
	else 
	{
  	udc->usb_5v_control_gpio = -1;
    udc->usb_5v_detect_gpio  = -1;	    
	}
    	
	udc->gadget.ops = &alidev_ops;
  udc->gadget.ep0 = &udc->ep[0].ep;
  udc->gadget.name = "alidev_udc";
  udc->gadget.dev.init_name = "gadget"; 

  alidev_ep_setup(udc, 0, "ep0", EP0_FIFO_SIZE, USB_ENDPOINT_XFER_CONTROL);         /* control endpoint */

#ifdef SUPPORT_C3921  
  /* first group of endpoints */
  alidev_ep_setup(udc, 4, "ep1in-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);        /* bulk-in endpoint */
  alidev_ep_setup(udc, 2, "ep2out-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);        /* bulk-out endpoint */
  alidev_ep_setup(udc, 3, "ep3in-int", ALI_EPC_FIFO_SIZE, USB_ENDPOINT_XFER_INT);     /* interrupt-in endpoint */   

  /* 2nd group of endpoints */
  alidev_ep_setup(udc, 1, "ep4in-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);        /* bulk-in endpoint */
  alidev_ep_setup(udc, 5, "ep5out-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);        /* bulk-out endpoint */
  alidev_ep_setup(udc, 6, "ep6in-int", ALI_EPC_FIFO_SIZE, USB_ENDPOINT_XFER_INT);     /* interrupt-in endpoint */  
  /* 3rd group of endpoints */
  alidev_ep_setup(udc, 7, "ep7in-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);        /* bulk-in endpoint */
  alidev_ep_setup(udc, 8, "ep8out-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);        /* bulk-out endpoint */
  /* 4th group of endpoints */
  alidev_ep_setup(udc, 9, "ep9in-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);        /* bulk-in endpoint */
  alidev_ep_setup(udc, 10, "ep10out-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);      /* bulk-out endpoint */
    /* 5th group of endpoints */
  alidev_ep_setup(udc, 11, "ep11in-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);      /* bulk-in endpoint */
  alidev_ep_setup(udc, 12, "ep12out-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);      /* bulk-out endpoint */            
#else
  /* first group of endpoints */
  alidev_ep_setup(udc, 1, "ep1in-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);        /* bulk-in endpoint */
  alidev_ep_setup(udc, 2, "ep2out-bulk", EP_FIFO_SIZE, USB_ENDPOINT_XFER_BULK);        /* bulk-out endpoint */
  alidev_ep_setup(udc, 3, "ep3in-int", ALI_EPC_FIFO_SIZE, USB_ENDPOINT_XFER_INT);     /* interrupt-in endpoint */    
#endif

	device_initialize(&udc->gadget.dev);
	udc->gadget.dev.parent = &pdev->dev;
	udc->gadget.dev.dma_mask = pdev->dev.dma_mask;

	the_controller = udc;
	platform_set_drvdata(pdev, udc);

	alidev_udc_enable(udc);
	alidev_udc_reinit(udc);

  init_completion(&udc->dma_epab_completion);  
#ifdef SUPPORT_C3921  
  init_completion(&udc->dma_epde_completion);  
  init_completion(&udc->dma_epgh_completion);  
  init_completion(&udc->dma_epij_completion);  
  init_completion(&udc->dma_epkl_completion);
#endif  
  
  udc->irq = irq;
	retval = request_irq(irq, alidev_udc_irq, IRQF_SHARED, gadget_name, udc);
	if (retval != 0) {
		dev_err(&pdev->dev,"request_irq(%d) failed with retvalue = %d \n", irq, retval);
		retval = -EBUSY;
		goto err_gpio;
	}

	retval = usb_add_gadget_udc(&pdev->dev, &udc->gadget);
	if (retval)
	{
		dev_err(&pdev->dev,"usb_add_gadget_udc failed %s (%d)  \n", __FUNCTION__, __LINE__);
		goto err_irq;
	}

	mutex_init(&udc->mutex);	
	spin_lock_init(&udc->lock1);	
	init_timer(&udc->timer);
	udc->timer.function = ali_udc_timer;
	udc->timer.data = (unsigned long) udc;
	udc->timer.expires = jiffies + msecs_to_jiffies(10);	
	add_timer(&udc->timer);	
	UDC_DBG("%s (%d)  ok\n", __FUNCTION__, __LINE__);

  for(i=0; i<DMA_BUF_NUM; i++) 
  {
  	udc->dma[i].buf = dma_alloc_coherent(&pdev->dev, DMA_MEM_SIZE, &udc->dma[i].addr, GFP_KERNEL);	
    if (!udc->dma[i].buf)    	
    {
    		retval = -ENOMEM;
    		printk("%s (%d)  err\n", __FUNCTION__, __LINE__);		
    		goto err_map;
    }
    UDC_DBG("%s (%d)  ok\n", __FUNCTION__, __LINE__);
  }
	return 0;
	
err_irq:
	free_irq(udc->irq, udc);
err_gpio:
  if(udc->usb_5v_control_gpio > 0)    
		gpio_free(udc->usb_5v_control_gpio);
  if(udc->usb_5v_detect_gpio > 0)	    
	  gpio_free(udc->usb_5v_detect_gpio);    
err_map:
	iounmap(udc->base_addr);
	
err_mem:
  if(udc)
    kfree(udc);
err_res:	
	printk("%s (%d)  \n", __FUNCTION__, __LINE__);
	return retval;
}

static int alidev_udc_remove(struct platform_device *pdev)
{
	struct alidev_udc *udc = platform_get_drvdata(pdev);
  int i;
    
	dev_dbg(&pdev->dev, "%s()\n", __func__);
	if (udc->driver)
		return -EBUSY;

	free_irq(udc->irq, udc);
	iounmap(udc->base_addr);
	platform_set_drvdata(pdev, NULL);
	dev_dbg(&pdev->dev, "%s: remove ok\n", __func__);

  for(i=0; i<DMA_BUF_NUM; i++)    
  {
  	if (udc->dma[i].buf) 
  		dma_free_coherent(&pdev->dev, DMA_MEM_SIZE, udc->dma[i].buf, udc->dma[i].addr);    
  }		
	return 0;
}

static struct platform_driver udc_driver_ali = {
	.driver		= {
		.name	= "alidev_udc",
		.owner	= THIS_MODULE,
	},
	.probe		= alidev_udc_probe,
	.remove		= alidev_udc_remove,
	.suspend	= NULL,
	.resume		= NULL,	
};

static int __init ali_udc_init(void)
{
	return platform_driver_register(&udc_driver_ali);
}

static void __exit ali_udc_exit(void)
{
	platform_driver_unregister(&udc_driver_ali);
}

module_init(ali_udc_init);
module_exit(ali_udc_exit);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:alidev-usbgadget");
