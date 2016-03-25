
/***************************************************************************************************
*    ALi Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_interrupt.c
*
*    Description:
*		ALi HDMI Interrupt Handler
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#include <linux/workqueue.h>
#include <linux/wait.h>
#include "hdmi_interrupt.h"
#include "hdmi_register.h"
#include "hdmi_proc.h"

#include <linux/kernel.h>
#include <linux/delay.h>

#ifdef CONFIG_HDCP_ENABLE_ALI
#include "hdmi_hdcp.h"
#endif
#ifdef CONFIG_CEC_ENABLE_ALI
#include "hdmi_cec.h"
#endif

extern HDMI_PRIVATE_DATA* hdmi_drv;

void hdmi_interrupt_hotplug_work(struct work_struct *);
DECLARE_WORK(hdmi_hotplug_work, hdmi_interrupt_hotplug_work);

#ifdef CONFIG_CEC_ENABLE_ALI
void hdmi_interrupt_cec_work(struct work_struct *);
DECLARE_WORK(hdmi_cec_work, hdmi_interrupt_cec_work);
#endif

void hdmi_interrupt_hotplug_work(struct work_struct *data)
{ 
	bool pre_hotplug_state = false;
	bool hotplug_state = false;
	int  debounce_cnt = 0; 

	pre_hotplug_state = HOT_PLUG_STATE;
	HDMI_DEBUG(KERN_ALERT "HPG 0x%0x \n", pre_hotplug_state); 
	do
	{
		/* Update Hot Plug In/Out State */
		msleep(10); // hdmi cable htplg Debounce Delay
		hotplug_state = HOT_PLUG_STATE;
		if((pre_hotplug_state != hotplug_state))
			debounce_cnt = 0;
		pre_hotplug_state = hotplug_state;
	}while(debounce_cnt++ < 3); 
	HDMI_DEBUG(KERN_ALERT "%s: %s\n", __FUNCTION__, (HOT_PLUG_STATE) ? "HotPlug-In":"HotPlug-Out");  
	hdmi_proc_state_update(); 
}    

#ifdef CONFIG_CEC_ENABLE_ALI
void hdmi_interrupt_cec_work(struct work_struct *data)
{
	int i;
	unsigned char buf[4];

	HDMI_DEBUG("%s: CEC Receive Data ", __FUNCTION__);
	for(i = 0; i<hdmi_drv->cec.buffer_length; i++)
	{
		HDMI_DEBUG("0x%.2x ", hdmi_drv->cec.receive_buffer[i]);
	}
	HDMI_DEBUG("\n");
    //if(( hdmi_drv->cec.receive_buffer[0] & 0x0F) == hdmi_drv->cec.logical_address)
    {
    	switch(hdmi_drv->cec.receive_buffer[1])
    	{
    		case 0xFF: // <Abort> Message
		    	if(hdmi_drv->cec.buffer_length ==2)
		    	{
					buf[0] = (hdmi_drv->cec.logical_address<<4) | ((hdmi_drv->cec.receive_buffer[0]&0xF0)>>4);
					buf[1] = 0x00;
					buf[2] = hdmi_drv->cec.receive_buffer[1];
					buf[3] = 0x00;
					hdmi_cec_transmit(buf, 4);
		    	}
		    	break;
		    case 0x83: // <Give Physical Address> Message
		    	if(hdmi_drv->cec.buffer_length ==2)
					hdmi_cec_report_physicalAddr();
				break;
#if 1//arthurcec
			default:
			//case ??: // send cec receive message to user
				hdmi_drv->hdmi2usr.flag |= HDMI2USR_CEC_MSG;
				priv->hdmi2usr.cec_notify.parg1 = hdmi_drv->cec.receive_buffer;
				priv->hdmi2usr.cec_notify.parg2 = hdmi_drv->cec.buffer_length;
				hdmi_send_msg(hdmi_drv, hdmi_drv->hdmi2usr.flag);
				break;
#endif
    	}
    }
}
#endif

irqreturn_t hdmi_interrupt_handler(int irq, void *dev_id)
{
	// Read Interrupt Register
	hdmi_drv->control.hdmi_int_status = HDMI_REG_INT;
	    
	// Clear Interrupt
	HDMI_REG_INT = HDMI_REG_INT;

	if((hdmi_drv->control.hdmi_int_status & B_INT_CTRL_PKT_DONE) == B_INT_CTRL_PKT_DONE)
	{
		HDMI_REG_INT_MASK  |= B_INT_CTRL_PKT_DONE;
		HDMI_REG_INT_MASK  &= ~B_INT_CTRL_PKT_DONE;
	}

	// MDI & CTRL PKT DONE Interrupt need clear by Mask
	if((hdmi_drv->control.hdmi_int_status & B_INT_MDI) == B_INT_MDI)
	{
		HDMI_REG_INT_MASK  |= B_INT_MDI;
		HDMI_REG_INT_MASK  &= ~B_INT_MDI;
			if(HOT_PLUG_STATE)
				HDMI_REG_CFG6  &= ~B_PORD_MASK;
			else
				HDMI_REG_CFG6  |= B_PORD_MASK;
			
		queue_work(hdmi_drv->control.interrupt_work_queue, &hdmi_hotplug_work);
	}

	hdmi_drv->control.hdmi_int2_status = hdmi_get_hdmi_interrupt2_source();
	// Clear Interrupt
	HDMI_REG_PP_REG2= HDMI_REG_PP_REG2;
	if((hdmi_drv->control.hdmi_int2_status & B_PP_VALUE_INT) == B_PP_VALUE_INT)
	{
		if(hdmi_drv->control.pp_value != hdmi_get_pp_value())
		{
			HDMI_REG_CFG6 = HDMI_REG_CFG6 & (~B_GCP_EN);
			hdmi_drv->control.pp_value = hdmi_get_pp_value();
			hdmi_proc_gcp_update();
			hdmi_proc_transmit_infoframe(&hdmi_drv->gcp.infoframe);
		}
		else
		{
			HDMI_REG_CFG6 = HDMI_REG_CFG6 | B_GCP_EN;
		}
		// Timing issue, don't use queue_work, waste too long time
		//queue_work(hdmi_drv->control.interrupt_work_queue, &hdmi_pp_work);
	}

	// 3821 phy clk interrupt
	if(hdmi_drv->chip_info.chip_id == 0x3821)
	{
		hdmi_drv->control.phy_int_status = hdmi_get_phy_interrupt_source();
		// set and clear PCG, CMU INT
		if(hdmi_get_pcg_rdy())
			hdmi_set_pcg_rdy(TRUE);
		if(hdmi_get_cmu_rdy())
			hdmi_set_cmu_rdy(TRUE);
		if(hdmi_drv->control.phy_int_status & 0x01)
		{
			HDMI_DEBUG("PCG Interrupt \n");
			wake_up_interruptible(&hdmi_drv->control.wait_queue);
		}

		if(hdmi_drv->control.phy_int_status & 0x02)
		{
			HDMI_DEBUG("CMU Interrupt  \n");
			wake_up_interruptible(&hdmi_drv->control.wait_queue);
		}
	}
	
    
#ifdef CONFIG_HDCP_ENABLE_ALI
	if((hdmi_drv->control.hdmi_int_status & B_INT_HDCP) == B_INT_HDCP)
	{
		hdmi_drv->control.hdcp_int_status = HDMI_REG_HDCP_STATUS;
	        
		// Clear HDCP Interrupt
		HDMI_REG_HDCP_STATUS = HDMI_REG_HDCP_STATUS | B_AKSV_RDY;

		//HDMI_DEBUG(KERN_ALERT "%s: HDCP INT 0x%.2x\n", __FUNCTION__, hdmi_drv->control.hdcp_int_status);
		if(hdmi_drv->control.hdmi_state == HDMI_STATE_PLAY)
			wake_up_interruptible(&hdmi_drv->control.wait_queue);
	}
#endif

#ifdef CONFIG_CEC_ENABLE_ALI
	if(hdmi_drv->control.cec_enable)
	{
		hdmi_drv->control.cec_int_status	= hdmi_get_cec_status();
		hdmi_drv->cec.data_status 			= hdmi_get_cec_datastatus();

		wake_up_interruptible(&hdmi_drv->control.wait_queue);
		if((hdmi_drv->control.cec_int_status & B_RX_EOM) == B_RX_EOM)
		{
			hdmi_drv->cec.buffer_length = hdmi_cec_receive(hdmi_drv->cec.receive_buffer);	// receive cec message
        	queue_work(hdmi_drv->control.interrupt_work_queue, &hdmi_cec_work);
		}

		hdmi_clear_cec_datastatus();
		hdmi_clear_cec_status();

	}
#endif
   
    return IRQ_HANDLED;

}
