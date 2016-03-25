/************************************************************************************************************
* Copyright (C) 2008 ALi Corp. All Rights Reserved.All rights reserved.
*
* File: hdmi_cec.c
*
* Description¡GThis is a simple sample file to illustrate format of file
*             prologue.
*
* History:
*     Date         By          Reason           Ver.
*   ==========  =========    ================= ======
************************************************************************************************************/
/*******************
* INCLUDE FILES    *
********************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/wait.h>

#include "hdmi_cec.h"
#include "hdmi_proc.h"
#include "hdmi_register.h"

extern HDMI_PRIVATE_DATA* hdmi_drv;

/************************************************************************************************************
* NAME: hdmi_cec_polling_message
* Returns : bool
* Additional information:
* For Address Allocation
************************************************************************************************************/
bool hdmi_cec_polling_message(unsigned char dest_addr)
{
	unsigned char buf;

	buf = (hdmi_drv->cec.logical_address <<4) | dest_addr;
	CEC_DEBUG("<Polling> 0x%.2x \n", buf);
	return hdmi_cec_transmit(&buf, 1);
}

/************************************************************************************************************
* NAME: hdmi_cec_polling_message
* Returns : void
* Additional information:
* For Address Allocation
************************************************************************************************************/
void hdmi_cec_report_physicalAddr(void)
{
	unsigned char buf[5];

	buf[0] = (hdmi_drv->cec.logical_address<<4) | LOGADDR_UNREG_BRDCST;
	buf[1] = 0x84;
	buf[2] = (unsigned char) (hdmi_drv->edid.physical_address >>8);
	buf[3] = (unsigned char) (hdmi_drv->edid.physical_address & 0xFF);
	buf[4] = 3; // "TV" =0, "Recording Device"=1, "STB"=3, "DVD"=4, "Audio"System"=5.

	CEC_DEBUG("<Report Physical Address> 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
	hdmi_cec_transmit(buf, 5);
}

/************************************************************************************************************
* NAME: hdmi_cec_init
* Returns : void
* Additional information:
* Add this function to hdmi_porc.c: hdmi_init_hw_config funciton to initial CEC module.
*
************************************************************************************************************/
void hdmi_cec_init()
{

	// Set CEC RESET to TRUE
	CEC_DEBUG("%s: Set CEC RESET to TRUE\n", __FUNCTION__);
	HDMI_REG_OPT2 = HDMI_REG_OPT2 | B_CEC_RST;

	// debug mode
	//HDMI_REG_OPT1 = (HDMI_REG_OPT1 & 0x7F) | 0x40;

	// LOGIC Address, Need to perform Logic address allocation after read EDID physical address.
	CEC_DEBUG("%s: Set LOGIC Address to LOGADDR_UNREG_BRDCST\n", __FUNCTION__);
	hdmi_set_cec_logic_address(LOGADDR_UNREG_BRDCST);

	hdmi_set_biu_frequency();

	// DIVISER Number: 0x31 = 49. suggested value from Jia Lin Sheu.
	CEC_DEBUG("%s: Set DIVISER Number: 0x31 = 49\n", __FUNCTION__);
	HDMI_REG_CEC_DIV_NUM = (HDMI_REG_CEC_DIV_NUM & (~B_DIV_NUM)) | (0x31 & B_DIV_NUM);

    // Low_CNT, Sampling timing count value tried by Victor.
	CEC_DEBUG("%s: Set Low_CNT to 0x06\n", __FUNCTION__);
	HDMI_REG_CEC_LOW_CNT = (HDMI_REG_CEC_LOW_CNT & (~B_LOW_CNT)) | (0x06 & B_LOW_CNT);

	// Clear Interrupt (CEC_STA) and unmask CEC interrupt.
	CEC_DEBUG("%s: Clear Interrupt (CEC_STA) and unmask CEC interrupt\n", __FUNCTION__);
	hdmi_clear_cec_status();
	HDMI_REG_OPT1 = HDMI_REG_OPT1 & (~B_CEC_MSK);	//unmask CEC interrupt

	// Set CEC RESET to FALSE
	CEC_DEBUG("%s: Set CEC RESET to FALSE\n", __FUNCTION__);
	HDMI_REG_OPT2 = HDMI_REG_OPT2 & (~B_CEC_RST);

	return;
}

/************************************************************************************************************
* NAME: hdmi_cec_transmit(unsigned char* Data, unsigned char Data_length)
*
* Returns : SUCCESS (ACK) /ERR_FAILUE (NACK)
*
* Transmit cec message data, with max length 16 blocks.
************************************************************************************************************/
bool hdmi_cec_transmit(unsigned char* Data, unsigned char Data_length)
{
	unsigned char i=0;
	
	CEC_DEBUG("%s len 0x%x\n", __FUNCTION__, Data_length);

	// Set Broadcast bit
	HDMI_REG_CEC_DIV_NUM = ((Data[0] & LOGADDR_UNREG_BRDCST) == LOGADDR_UNREG_BRDCST) ?
							(HDMI_REG_CEC_DIV_NUM|B_BRDCST) : (HDMI_REG_CEC_DIV_NUM & (~B_BRDCST));

	// Write Transmit Data to Buffer
	for( i=0; i< Data_length; i++)
	{
		HDMI_REG_CEC_TX_ADDR = (i & B_CEC_TX_REG_ADDR) | B_CEC_TX_REG_RW;		// Fill Address
		HDMI_REG_CEC_TX_DATA = Data[i];											// Fill Data
		// Fill EOM Bit
		HDMI_REG_CEC_CTRL = (i!=Data_length-1) ?
							(HDMI_REG_CEC_CTRL&(~B_CEC_TX_EOM)):(HDMI_REG_CEC_CTRL|B_CEC_TX_EOM|B_CEC_TX_EN);
	}

	// Wait transmit done interrupt. (4.5 (start bit) + 2.4 (ms/bit) * 10(10bits/block) * 16(max blocks)) * 5 (retry) = 1942.5 ms
    wait_event_interruptible_timeout(hdmi_drv->control.wait_queue, hdmi_drv->control.cec_int_status & B_TX_DONE, 2000);
    if((hdmi_drv->control.cec_int_status & B_TX_DONE) == B_TX_DONE )
    {
		CEC_DEBUG("hdmi_cec_transmit: Got HDMI_FLG_CEC_TX_DONE, CEC Transmit Success\n");
		hdmi_drv->control.cec_int_status &= ~B_TX_DONE;

		if((Data[0] & LOGADDR_UNREG_BRDCST) == LOGADDR_UNREG_BRDCST)
			HDMI_REG_CEC_DIV_NUM &= ~(B_BRDCST);			// Clear Broadcast bit

		// Check ACK
		if( hdmi_drv->control.cec_int_status & B_RCV_UNKNOW)
		{
			hdmi_drv->control.cec_int_status &= ~B_RCV_UNKNOW;
			return false;	
		}
		else
		{
			return true;
		}
    }
	else
	{
    	wait_event_interruptible_timeout(hdmi_drv->control.wait_queue, hdmi_drv->control.cec_int_status & B_RCV_UNKNOW, 1);
    	if((hdmi_drv->control.cec_int_status & B_RCV_UNKNOW) == B_RCV_UNKNOW )
    	{
 			CEC_DEBUG("hdmi_cec_transmit: Got HDMI_FLG_CEC_RCV_UNKNOWN, CEC Transmit Fail\n");
 			hdmi_drv->control.cec_int_status &= ~B_RCV_UNKNOW;
			if((Data[0] & LOGADDR_UNREG_BRDCST) == LOGADDR_UNREG_BRDCST)
				HDMI_REG_CEC_DIV_NUM &= ~(B_BRDCST);			// Clear Broadcast bit
    	}
		else
		{
			if(HDMI_REG_CEC_CTRL & B_CEC_TX_TRY_DONE)
			{
				CEC_DEBUG("hdmi_cec_transmit: CEC_TX_TRY_DONE\n");
				HDMI_REG_CEC_CTRL |= B_CEC_TX_TRY_DONE;
			}

		}
		return false;
	}
}

/************************************************************************************************************
* NAME: hdmi_cec_receive(unsigned char* buf)
*
* Returns : unsigned char recieve message block length
*
* Receive cec message data, with max length 16 blocks.
************************************************************************************************************/
unsigned char hdmi_cec_receive(unsigned char* buf)
{
	unsigned char message_length, i;

	// Read Number of message size
	message_length = HDMI_REG_CEC_RX_ADDR+1;

	// Read Block from i = 0 to message_length-1
	for(i = 0; i<message_length; i++)
	{
		HDMI_REG_CEC_RX_ADDR = i & B_CEC_RX_BLOCK_CNT;
		buf[i] = HDMI_REG_CEC_RX_DATA;
	}
	return message_length;
}

/************************************************************************************************************
* NAME: hdmi_set_biu_frequency( void)
*
* Returns : void
*
* Set BIU Frequency according HDMI PHY clock. this register should be set per each video resolution change.
************************************************************************************************************/
inline void hdmi_set_biu_frequency( void)
{

	// Set BIU Frequency according HDMI PHY clock. this register should be set per each video resolution change.
	switch((M36_SYS_REG_SYS_CTRL & 0x00000070) >> 4) // read phy clock from system register
	{
		if(hdmi_drv->chip_info.chip_id == 0x3503)
		{
			case 0x00: //27 MHz
				  CEC_DEBUG("%s:Set BIU Frequency to 26\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 27-1;
			      break;
			case 0x01: //54M
				  CEC_DEBUG("%s:Set BIU Frequency to 53\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 54-1;
			      break;
			case 0x02: //74M
				  CEC_DEBUG("%s:Set BIU Frequency to 74\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 74;
			      break;
			case 0x03: //72M
				  CEC_DEBUG("%s:Set BIU Frequency to 71\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 72-1;
			      break;
			case 0x04: //48M
				  CEC_DEBUG("%s:Set BIU Frequency to 47\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 48-1;
			      break;
			case 0x05: //148M
				  CEC_DEBUG("%s:Set BIU Frequency to 148\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 148;
			      break;
		}
		else
		{
			case 0x00: //27 MHz
				  CEC_DEBUG("%s:Set BIU Frequency to 26\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 27-1;
			      break;
			case 0x01: //54M
				  CEC_DEBUG("%s:Set BIU Frequency to 53\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 54-1;
			      break;
			case 0x02: //108M
				  CEC_DEBUG("%s:Set BIU Frequency to 108\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 108;
			      break;
			case 0x03: //74M
				  CEC_DEBUG("%s:Set BIU Frequency to 74\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 74;
			      break;
			case 0x04: //148M
				  CEC_DEBUG("%s:Set BIU Frequency to 148\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 148;
			      break;
			case 0x05: //297M
				  CEC_DEBUG("%s:Set BIU Frequency to 297\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 297;
			      break;
			default :
				CEC_DEBUG("%s:Set BIU Frequency to 297\n", __FUNCTION__);
				  HDMI_REG_CEC_BIU_FREQ = 297;
				break;
		}
	}

}

/************************************************************************************************************
*    CEC Module Register set access functions.
************************************************************************************************************/
// offset 0x6E[3:0]
inline void hdmi_set_cec_logic_address( unsigned char cec_address)
{
	HDMI_REG_OPT1 = (HDMI_REG_OPT1 & (~B_CEC_ADDR)) | (cec_address & B_CEC_ADDR);
}

inline unsigned char hdmi_get_cec_logic_address(void)
{
	return (HDMI_REG_OPT1 & B_CEC_ADDR);
}

// offset 0x6E[5]
void hdmi_set_cec_intr_mask(bool bMask)
{
    if(bMask == true)
    	HDMI_REG_OPT1 = (HDMI_REG_OPT1 | B_CEC_MSK);	//mask CEC interrupt
    else
   	    HDMI_REG_OPT1 = (HDMI_REG_OPT1 & (~B_CEC_MSK));	//unmask CEC interrupt
}

// offset 0x6F[2]
inline void hdmi_set_cec_rst( bool cec_reset)
{
	HDMI_REG_OPT2 = (cec_reset) ? (HDMI_REG_OPT2 | B_CEC_RST) : (HDMI_REG_OPT2 & (~B_CEC_RST));
}

//offset 0x71
inline unsigned char hdmi_get_cec_status(void)
{
	return HDMI_REG_CEC_STATUS;
}

// offse 0x 71
inline void hdmi_clear_cec_status(void)
{
	HDMI_REG_CEC_STATUS = HDMI_REG_CEC_STATUS;
}

//offset 0x76
inline unsigned char hdmi_get_cec_datastatus(void)
{
	return HDMI_REG_CEC_CTRL;
}

//offset 0x76[5]
inline void hdmi_clear_cec_datastatus(void)
{
	HDMI_REG_CEC_CTRL |= B_CEC_RX_DATA_CLR;
}

