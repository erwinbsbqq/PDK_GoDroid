/***************************************************************************************************
*    ALi Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_hdcp.c
*
*    Description:
*		ALi HDMI HDCP Process
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/wait.h>

#include "hdmi_hdcp.h"
#include "hdmi_interrupt.h"
#include "hdmi_register.h"
#include "hdmi_proc.h"
		
extern HDMI_PRIVATE_DATA* hdmi_drv;

void hdmi_hdcp_load_keys_to_hardware(void)
{
    unsigned int addr, i=0;

    HDMI_REG_OPT2 &= (~B_WD_ADDRH);
    for(addr=0; addr<286; addr++)
    {
        if(addr == 0x100)
           HDMI_REG_OPT2 |= B_WD_ADDRH; 
		
        HDMI_REG_SRAM_ADDRESS = (unsigned char) (addr & 0x00FF);		// Set SRAM Address
        HDMI_REG_SRAM_DATA = hdmi_drv->hdcp.key[addr];					// Write Data to SRAM
        HDMI_REG_OPT2 = ( HDMI_REG_OPT2 | B_RAM_EN ) & (~B_RAM_WR);

		while ( ((HDMI_REG_OPT2 & B_RAM_EN) == B_RAM_EN) && (i++ < 1000))	// Check Write Done
		;
    }
    HDMI_REG_OPT2 &= (~B_WD_ADDRH);

}

void hdmi_hdcp_hardware_reset(void)
{

    // Reset HDCP Cipher
    HDMI_REG_HDCP_CTRL |= B_CP_RST;
    udelay(5);
    HDMI_REG_HDCP_CTRL &= ~B_CP_RST;
    udelay(5);

    // Load HDCP Key to HDCP Module
    HDMI_REG_HDCP_CTRL &= ~B_HOST_KEY;  //disable host key
    if(hdmi_drv->control.load_key_from_ce == false)
    	{
    	hdmi_hdcp_load_keys_to_hardware();
    	}
    udelay(5);

    // Enable LD_KSV to fetch AKsv and calculate KM value in HDCP module
    HDMI_REG_OPT2 |= B_LD_KSV;
    udelay(5);

    // Reset SHA, Auth, Encrypt Engine, and (disable scramble)
    HDMI_REG_HDCP_CTRL &= ~( B_SHA_EN | B_AUTHEN_EN | B_ENC_EN | B_SCRAMBLE);
    udelay(5);

    // Set VSYNC_Sel to 1 to fix HDCP Bug (16pixel ENC_DIS timing delay of win_of_opp)
    HDMI_REG_CFG0 |= B_VSYNC_SEL;

    udelay(5);
    return;
}

bool hdmi_hdcp_read_from_sink(unsigned char offset, unsigned char* data, unsigned int length)
{
    struct i2c_adapter *adapter;
    struct i2c_msg msgs[] = {   { .addr = 0x3A, .flags	= 0,         .len = 1,       .buf = &offset },
                                { .addr = 0x3A, .flags	= I2C_M_RD,  .len = length,  .buf = data    } };

    adapter = i2c_get_adapter(hdmi_drv->control.eddc_adapter_id);
    if(adapter)
        if(i2c_transfer(adapter, msgs, 2) != 2)
        {
            HDCP_DEBUG("%s: i2c_transfer fail\n", __FUNCTION__);
            return false;
        }         
    return true;
}

bool hdmi_hdcp_write_to_sink(unsigned char offset, unsigned char* data, unsigned int length)
{
    unsigned char buf[10];
    struct i2c_adapter *adapter;
    struct i2c_msg msgs = { .addr = 0x3A, .flags = 0, .len = length+1,  .buf = buf };

    buf[0] = offset;
    memcpy(buf+1, data, length);

    adapter = i2c_get_adapter(hdmi_drv->control.eddc_adapter_id);
    if(adapter)
        if(i2c_transfer(adapter, &msgs, 1) != 1)
        {
            HDCP_DEBUG("%s: i2c_transfer fail\n", __FUNCTION__);
            return false;
        }    
    return true;
}

bool hdmi_hdcp_validate_ksv(unsigned char *ksv)
{
    unsigned int i, j,  number_of_one = 0, number_of_zero = 0;

    // Check KSV has 20 ONE and 20 ZERO or not.
    for(i=0; i<5; i++)
    {
	    for (j=0; j<8; j++)
	    {
    	    if ((ksv[i] >> j) & 0x01)	number_of_one++;
	        else            		    number_of_zero++;
	    }
    }
    return (number_of_one == 20 && number_of_zero == 20) ? true : false;
}

bool hdmi_hdcp_3rd_ri_link_integrity_check(void)
{
    unsigned char Ri_tx[2], Ri_rx[2];
    
    HDCP_DEBUG("hdmi_hdcp_3rd_ri_link_integrity_check\n");
    
// A5: Link Integrity Check
HDCP_DEBUG("HDCP A5: Link Integrity Check (Ri Ready)\n");

    HDCP_DEBUG("\tGet Ri interrupt, per 128 frame or every 2 second.\n");
    Ri_tx[0] = HDMI_REG_HDCP_RI_1; 
	Ri_tx[1] = HDMI_REG_HDCP_RI_2;      
    HDCP_DEBUG("\tRi(Tx): %.2x %.2x\n", Ri_tx[0], Ri_tx[1]);

    HDCP_DEBUG("\tRead Ri(Rx) from HDCP Rx\n");
    if(hdmi_hdcp_read_from_sink(HDCP_RX_PRI_RI, Ri_rx, sizeof(Ri_rx)) == false)
        return false; 
    
    HDCP_DEBUG("\tRi(Rx): %.2x %.2x\n", Ri_rx[0], Ri_rx[1]);
    HDCP_DEBUG("\tCompare Ri(tx) & Ri(rx)\n");
    if((Ri_rx[0]!=Ri_tx[0]) || (Ri_rx[1]!=Ri_tx[1]))
    {
		HDCP_DEBUG("%s: fail (Tx Ri != Rx Ri)\n",__FUNCTION__);
        return false;
    }
    else
    {
		HDCP_DEBUG("%s: Success (Tx Ri == Rx Ri)\n",__FUNCTION__);        
    }
    
    return true;

}
bool hdmi_hdcp_3rd_pj_link_integrity_check(void)
{
    unsigned char Pj_tx, Pj_rx;
    static unsigned int pj_successive_mismatch = 0;

    HDCP_DEBUG("hdmi_hdcp_3rd_pj_link_integrity_check\n");
    
// A5: Link Integrity Check
HDCP_DEBUG("HDCP A5: Link Integrity Check (Pj Ready)\n");    

    HDCP_DEBUG("\tGet Pj interrupt, per 16 frames for Advanced Cipher mode enable\n");
    Pj_tx = HDMI_REG_HDCP_PJ;
    HDCP_DEBUG("\tRj(Tx): %.2x\n", Pj_tx);    
    
    HDCP_DEBUG("\tRead Pj(Rx) from HDCP Rx\n");
    hdmi_hdcp_read_from_sink(HDCP_RX_PRI_PJ, &Pj_rx, sizeof(Pj_rx));
    HDCP_DEBUG("\tPj(Rx): %.2x\n", Pj_rx);

    if( Pj_tx != Pj_rx )
    {
        if(++pj_successive_mismatch >= 3)
        {
            pj_successive_mismatch = 0;
            return false;
        }
		HDCP_DEBUG("%s: fail (Tx Pj != Rx Pj), pj mismatch count = %d\n", __FUNCTION__, pj_successive_mismatch);        
    }
    else    
    {
        pj_successive_mismatch = 0;
		HDCP_DEBUG("%s: Success (Tx Pj == Rx Pj)\n", __FUNCTION__);          
    }
    return true;

}
void hdmi_hdcp_repeater_timout_function(unsigned long repeater_ready_timer)//(struct timer_list* repeater_ready_timer)
{
    HDCP_DEBUG("%s: 5 sec timeout!!\n", __FUNCTION__);
    hdmi_drv->hdcp.repeater_ready_timeout = true;
    del_timer((struct timer_list *)repeater_ready_timer);
}

bool hdmi_hdcp_2nd_verify_repeater_ksvlist(void)
{
    HDCP_BCAPS      Bcaps;
    HDCP_BSTATUS    Bstatus;
    unsigned char*	ksv_list_bstatus;
    unsigned char	V_rx[20];
    int			    i;

HDCP_DEBUG("hdmi_hdcp_2nd_verify_repeater_ksvlist\n");

// A9: Wait for Ready. 
HDCP_DEBUG("HDCP A8: Wait for Ready. \n");
    HDCP_DEBUG("\tSets up a 5 seconds watchdog timer and polls the receiver's READY bit.\n");
    init_timer(&hdmi_drv->hdcp.repeater_ready_timer);
    hdmi_drv->hdcp.repeater_ready_timer.data = (unsigned long)(&hdmi_drv->hdcp.repeater_ready_timer);
    hdmi_drv->hdcp.repeater_ready_timer.function = hdmi_hdcp_repeater_timout_function;
    hdmi_drv->hdcp.repeater_ready_timer.expires = jiffies + 5 * HZ; // 5 sec timer

    hdmi_drv->hdcp.repeater_ready_timeout = false;
    add_timer(&hdmi_drv->hdcp.repeater_ready_timer);

    msleep(10); // Add delay for 1B-02 hotplug test.

    while(!hdmi_drv->hdcp.repeater_ready_timeout)
    {
    	if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }
    	hdmi_hdcp_read_from_sink(HDCP_RX_PRI_BCAPS, (unsigned char*)&Bcaps, sizeof(Bcaps));

    	if(Bcaps.ready)
        {
    	    HDCP_DEBUG("\tGot HDCP Repeater READY\n");
    	    del_timer(&hdmi_drv->hdcp.repeater_ready_timer);
    	    break;
        }

    	msleep(50);
    }

    if(hdmi_drv->hdcp.repeater_ready_timeout)
    {
    	HDCP_DEBUG("%s: fail, READY indication not received.\n", __FUNCTION__);		
    	return false; 	// Go back to A0
    }

// A9: ReadKSV List. 
HDCP_DEBUG("HDCP A9: HDCP Tx reads the KSVs, V(Rx) and verifies V(Tx) == V(Rx)\n");
    if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }

    hdmi_hdcp_read_from_sink(HDCP_RX_PRI_BSTATUS, (unsigned char*)&Bstatus, sizeof(Bstatus));  
    HDCP_DEBUG("\tDevice Count = %d\n", Bstatus.device_count);
    HDCP_DEBUG("\tMax Devs Exceeded = %d\n", Bstatus.max_devs_exceeded);
    HDCP_DEBUG("\tdepth = %d\n", Bstatus.depth);
    HDCP_DEBUG("\tMax Cascade Exceeded = %d\n", Bstatus.max_cascade_exceeded);

    if( (Bstatus.max_devs_exceeded) || (Bstatus.max_cascade_exceeded) || (Bstatus.device_count > hdmi_drv->hdcp.source_max_ksv) 
        || ((hdmi_drv->hdcp.source_out_onlyrep == false) && (Bstatus.device_count == 0)) )
    {
        if(Bstatus.max_devs_exceeded)
        {
            HDCP_DEBUG("\tMAX_DEVS_EXCEEDED\n");
        }
        if(Bstatus.max_cascade_exceeded)			
        {
        	HDCP_DEBUG("\tMAX_CASCADE_EXCEEDED\n");
        }
	    if(Bstatus.device_count>hdmi_drv->hdcp.source_max_ksv)  
        {
        	HDCP_DEBUG("\tdev_count(%d) > SOURCE_MAX_KSV(%d)\n", Bstatus.device_count, hdmi_drv->hdcp.source_max_ksv);
	    }
	    if((hdmi_drv->hdcp.source_out_onlyrep == false) && (Bstatus.device_count == 0))
        {
        	HDCP_DEBUG("\tdev_count(zero) & Source_Out_OnlyRep(N)\n");	
	    }
        return false;   // Go back to A0
    }

    HDCP_DEBUG("\tHDCP Tx Read KSV List from HDCP Repeater\n");
    ksv_list_bstatus = kmalloc( Bstatus.device_count*5+sizeof(Bstatus), GFP_KERNEL);
    if(ksv_list_bstatus == NULL)
    {
        HDCP_DEBUG("%s: kmalloc for ksvlist fail.\n",__FUNCTION__);
        return false;   // Go back to A0
    }
    if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n",__FUNCTION__);  kfree(ksv_list_bstatus);  return false;   }

    hdmi_hdcp_read_from_sink(HDCP_RX_PRI_KSV_FIFO , ksv_list_bstatus, Bstatus.device_count*5 );    
    
    HDCP_DEBUG("\tKSV List:");
    for(i=0; i< Bstatus.device_count*5; i++)
    {
	    HDCP_DEBUG(" %02x", ksv_list_bstatus[i]);
    }
    HDCP_DEBUG("\n");

    /* Append Bstatus to end of ksv_list, for calculate V = SHA-1( ksv_list||Bstatus||M0 ) */
    hdmi_hdcp_read_from_sink(HDCP_RX_PRI_BSTATUS, (unsigned char*)&Bstatus, sizeof(Bstatus));  
    memcpy( &ksv_list_bstatus[Bstatus.device_count*5], &Bstatus, sizeof(Bstatus));    

    HDCP_DEBUG("\tBstatus:");
    for(i=Bstatus.device_count*5; i< Bstatus.device_count*5+2; i++)
    {
	    HDCP_DEBUG(" %02x", ksv_list_bstatus[i]);
    }
    HDCP_DEBUG("\n");
    
    HDCP_DEBUG("\tHDCP Tx Read V(Rx) from HDCP Repeater\n");
    if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n",__FUNCTION__);  kfree(ksv_list_bstatus);  return false;   }
   	hdmi_hdcp_read_from_sink(HDCP_RX_PRI_VH0, V_rx, 20);  

    HDCP_DEBUG("\tV(Rx): ");
    for(i=0; i<sizeof(V_rx); i++)
    {
	    HDCP_DEBUG(" %02x", V_rx[i]);
    }
    HDCP_DEBUG("\n");
 
    HDCP_DEBUG("Write KSV List, Bstatus, V(Rx) to HDMI module\n");
    for(i=0; i< Bstatus.device_count*5+2; i++)	
        HDMI_REG_HDCP_KSV_LIST = ksv_list_bstatus[i];
    
    memcpy((unsigned char*)&HDMI_REG_HDCP_SHA_H0, V_rx, 20 );
    HDMI_REG_HDCP_CTRL |= B_SHA_EN;

    HDCP_DEBUG("\tWait HDMI Module Calculate V(Tx)\n");
    wait_event_interruptible_timeout(hdmi_drv->control.wait_queue, hdmi_drv->control.hdcp_int_status & B_V_RDY, 100);

    if((hdmi_drv->control.hdcp_int_status & B_V_RDY) != B_V_RDY )
    {
        HDCP_DEBUG("%s: Wait V(Tx) Ready fail\n",__FUNCTION__);
	    kfree(ksv_list_bstatus);
        return false;
    }
    hdmi_drv->control.hdcp_int_status &= ~B_V_RDY;
    
    if((hdmi_drv->control.hdcp_int_status & B_V_MATCH ) != B_V_MATCH  )
    {
        HDCP_DEBUG("%s: V(Rx) and V(Tx) are not match\n",__FUNCTION__);
    	kfree(ksv_list_bstatus);
        return false;
    }
    hdmi_drv->control.hdcp_int_status &= ~B_V_MATCH;

    HDCP_DEBUG("\t%s: V(Rx) and V(Tx) are match\n",__FUNCTION__);
    HDCP_DEBUG("\t%s: HDCP Stage2 Authentication Success\n",__FUNCTION__);

    kfree(ksv_list_bstatus);

    return true;
}

bool hdmi_hdcp_1st_establish_share_value(void)
{
    int                 retry_count = 0;
    unsigned char  	    An[8], AKsv[5], BKsv[5], R0_tx[2], R0_rx[2], Ainfo, M0[8];
    HDCP_BCAPS          Bcaps;
    HDCP_BSTATUS        Bstatus;
    
HDCP_DEBUG("hdmi_hdcp_1st_establish_share_value\n");
	HDMI_REG_I2S_UV |= B_NORMAL_MONO_SEL;   // Output Black Screen
	udelay(5);

	if(hdmi_drv->control.av_mute_state == false) 
    {
		hdmi_proc_set_avmute(true, 1);      // Set AVMUTE
	}
#if 0
	if(hdmi_hdcp_validate_ksv(&(hdmi_drv->hdcp.key[1])) == false)
	{
#ifdef CONFIG_ALI_CHIP_M3921
		HDMI_REG_3DPHY_REG = HDMI_REG_3DPHY_REG & (~B_ENHPDRXSENSE);
#endif
		hdmi_proc_set_phy_onoff(false);
		return false;
	}
	else
		hdmi_proc_set_phy_onoff(true);
#endif
 	hdmi_hdcp_hardware_reset();             // Reset HDCP Cipher first.


    wait_event_interruptible_timeout(hdmi_drv->control.wait_queue, hdmi_drv->control.hdcp_int_status & B_AKSV_RDY, 10);
    if((hdmi_drv->control.hdcp_int_status & B_AKSV_RDY) != B_AKSV_RDY )
    {
    		hdmi_drv->control.aksv_fail = TRUE;
		HDCP_DEBUG("%s: wait AKSV Ready flag fail\n", __FUNCTION__);
		return false;
    }
	hdmi_drv->control.aksv_fail = FALSE;
    hdmi_drv->control.hdcp_int_status &= ~B_AKSV_RDY;

// A0: Wait for Active Receiver
HDCP_DEBUG("HDCP A0: Wait for Active HDCP Rx\n");
    do
	{
		msleep(500);
		if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }

        hdmi_hdcp_read_from_sink(HDCP_RX_PRI_BKSV, BKsv, sizeof(BKsv));
	    retry_count++;

	}while( (true!=hdmi_hdcp_validate_ksv(BKsv)) && (retry_count<10) );

	if(retry_count == 10)           {   HDCP_DEBUG("\tRead BKsv fail, Reciver not support HDCP)\n");    return false;   }

    HDCP_DEBUG("\tGot BKsv, Reciever support HDCP\n");
    HDCP_DEBUG("\tBKsv: %.2x %.2x %.2x %.2x %.2x\n", BKsv[0], BKsv[1], BKsv[2], BKsv[3], BKsv[4]);


// A1: Exchange KSVs
 HDCP_DEBUG("HDCP A1: Exchange KSVs\n");
    if(false == HOT_PLUG_STATE) 	{   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }

    HDCP_DEBUG("\tHDCP Tx reads Bcaps and Bstatus to determines operating mode (DVI/HDMI)\n");
//	hdmi_hdcp_read_from_sink(HDCP_RX_PRI_BCAPS, (unsigned char*)&Bcaps, sizeof(Bcaps));
    hdmi_hdcp_read_from_sink(HDCP_RX_PRI_BSTATUS, (unsigned char*)&Bstatus, sizeof(Bstatus));

    HDCP_DEBUG("\tHDCP Rx is %s Mode\n", (Bstatus.hdmi_mode) ? "HDMI" : "DVI");
    HDCP_DEBUG("\tHDCP Rx %s support Advanced Cipher\n", (Bcaps.advanced_cipher_support) ? "" : "doesn't");

    // For C++ logic test, advanced cipher not verify yet.
    //Bcaps.advanced_cipher_support = 0;
    //if(Bcaps.advanced_cipher_support)
    //{
	    // Ref 1A-05: Set Ainfo in the HDCP Rx to determine the options that will be in effect prior to writing Aksv to HDCP Receiver
        //Ainfo = 0x02;
        //hdmi_hdcp_write_to_sink(HDCP_RX_PRI_AINFO, &Ainfo, sizeof(Ainfo));
        //HDMI_REG_OPT = HDMI_REG_OPT | B_ELINK | B_AC;
    //}
    //else
        HDMI_REG_OPT = ( HDMI_REG_OPT & (~(B_ELINK|B_AC)));
    
    HDCP_DEBUG("\tPrepare An and AKsv\n");
    HDMI_REG_HDCP_CTRL |= B_AUTHEN_EN;      // Enable Authentication
    mdelay(10);
    HDMI_REG_HDCP_CTRL |= B_AN_STOP;        // Stop An Free Run

	memcpy(An, (unsigned char*)&HDMI_REG_HDCP_WR_AN, sizeof(An));           // Read An from HDMI module
	memcpy(AKsv, (unsigned char*)&HDMI_REG_HDCP_RD_AKSV, sizeof(AKsv));     // Read Aksv from HDMI module

    HDCP_DEBUG("\tAn: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n", An[0], An[1], An[2], An[3], An[4], An[5], An[6], An[7]);
    HDCP_DEBUG("\tAKsv: %.2x %.2x %.2x %.2x %.2x\n", AKsv[0], AKsv[1], AKsv[2], AKsv[3], AKsv[4]);

    HDCP_DEBUG("\tWrite An and AKsv to HDCP Rx to initiate Authentication\n");
    // Ref 1A-04: An value must be written by the HDCP Transmitter before the KSV is written.
    hdmi_hdcp_write_to_sink(HDCP_RX_PRI_AN, An, sizeof(An));
	mdelay(1); // Add delay because send 2 cmd QD will miss .
    hdmi_hdcp_write_to_sink(HDCP_RX_PRI_AKSV, AKsv , sizeof(AKsv));

    // HDCP CTS 1A-02: Regular procedure HPD after writing Aksv
    mdelay(10);
    if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }

    HDCP_DEBUG("\tRead Bcaps[6] Repeater and Bksv from to HDCP Rx\n");
    hdmi_hdcp_read_from_sink(HDCP_RX_PRI_BKSV, BKsv, sizeof(BKsv));
    hdmi_hdcp_read_from_sink(HDCP_RX_PRI_BCAPS, (unsigned char*)&Bcaps, sizeof(Bcaps));

    HDCP_DEBUG("\tBKsv: %.2x %.2x %.2x %.2x %.2x\n", BKsv[0], BKsv[1], BKsv[2], BKsv[3], BKsv[4]);
    HDCP_DEBUG("\tBcaps[6] Repeater = %d\n", Bcaps.repeater);
    hdmi_drv->hdcp.sink_is_repeater = (Bcaps.repeater) ? true : false;
    
    HDCP_DEBUG("\tCheck BKsv validation (containing 20 zeros and 20 ones)\n");
    if( hdmi_hdcp_validate_ksv(BKsv) != true)
    {
		HDCP_DEBUG("\tBKsv Not Valid!!\n");
		return false;
    }
    else
    {
		HDCP_DEBUG("\tBKsv is valid\n");
    }

// A2: Computations
HDCP_DEBUG("HDCP A2: Computations\n");
    if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }

    // Set Repeater to HDMI Module
    HDMI_REG_HDCP_CTRL = (Bcaps.repeater) ? (HDMI_REG_HDCP_CTRL|B_RX_RPTR) : (HDMI_REG_HDCP_CTRL & (~B_RX_RPTR));

    HDCP_DEBUG("\tWrite BKsv to HDMI Module\n");
 	memcpy((unsigned char*)&HDMI_REG_HDCP_WR_BKSV0, (unsigned char *)BKsv, sizeof(BKsv));

	// Ref 1A-06:  DUT reads R0' must be attempted later than 100ms after writing Aksv
	msleep(150);	if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }
//	mdelay(50);    if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }
//	mdelay(50);    if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }
//	mdelay(50);    if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }
//	mdelay(50);    if(false == HOT_PLUG_STATE) {   HDCP_DEBUG("\t%s: Hot-Plug Out fail\n", __FUNCTION__);   return false;   }
    
    HDCP_DEBUG("\tWait HDMI Module Calculate R0\n");
    wait_event_interruptible_timeout(hdmi_drv->control.wait_queue, hdmi_drv->control.hdcp_int_status & B_RI_RDY, 10);
    if((hdmi_drv->control.hdcp_int_status & B_RI_RDY) != B_RI_RDY )
    {
		HDCP_DEBUG("%s: Wait R0(Tx) Ready fail\n", __FUNCTION__);
        return false;
    }
    hdmi_drv->control.hdcp_int_status &= ~B_RI_RDY;

    R0_tx[0] = HDMI_REG_HDCP_RI_1; R0_tx[1]= HDMI_REG_HDCP_RI_2;  
    HDCP_DEBUG("\tCalculate Result R0(Tx): %.2x %.2x\n", R0_tx[0], R0_tx[1]);

// A3: Validate Receiver
HDCP_DEBUG("HDCP A3: Validate HDCP Rx\n");
    HDCP_DEBUG("\tRead R0(Rx) from HDCP Rx\n");
    hdmi_hdcp_read_from_sink(HDCP_RX_PRI_RI, R0_rx, sizeof(R0_rx));
    HDCP_DEBUG("\tR0(Rx): %.2x %.2x\n", R0_rx[0], R0_rx[1]);
	if((R0_rx[0] == 0x00) && (R0_rx[1] == 0x00))
	{
 		hdmi_hdcp_read_from_sink(HDCP_RX_PRI_RI, R0_rx, sizeof(R0_rx));// retry
    	HDCP_DEBUG("\tR0(Rx): %.2x %.2x\n", R0_rx[0], R0_rx[1]);
	}

    HDCP_DEBUG("\tCompare R0(tx) & R0(rx)\n");
    if((R0_rx[0]!=R0_tx[0]) || (R0_rx[1]!=R0_tx[1]))
    {
		HDCP_DEBUG("%s: fail (Tx R0 != Rx R0)\n", __FUNCTION__);
        return false;
    }
    else
    {
        HDCP_DEBUG("\tHDCP Stage1 Authentication Success\n");
    }
    
	memcpy(M0, (unsigned char*)&HDMI_REG_HDCP_TOP_MI, sizeof(M0));
    HDCP_DEBUG("\tM0: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n", M0[0], M0[1], M0[2], M0[3], M0[4], M0[5], M0[6], M0[7]);

	hdmi_proc_set_avmute(false, 4);             	// Clear AVMUTE
	
	msleep(40);

	HDMI_REG_HDCP_CTRL	|= B_ENC_EN;          	// Enable Encryption
	HDMI_REG_I2S_UV 	&= ~B_NORMAL_MONO_SEL;  // Disable Output Black Screen
	
    return true;
    
}

