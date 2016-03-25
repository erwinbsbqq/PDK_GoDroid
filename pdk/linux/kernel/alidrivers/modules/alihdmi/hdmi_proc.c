/***************************************************************************************************
*    ALi Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_proc.c
*
*    Description:
*		ALi HDMI Driver
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fb.h>
#include <linux/wait.h>
#include <linux/kthread.h>

#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <rpc_hld/ali_rpc_hld_dis.h>
#include <rpc_hld/ali_rpc_hld_snd.h>

#include "hdmi_proc.h"
#include "hdmi_register.h"
#include "hdmi_infoframe.h"
#include "hdmi_edid.h"
#include "ali_reg.h"
#ifdef CONFIG_HDCP_ENABLE_ALI
#include "hdmi_hdcp.h"
#endif
#ifdef CONFIG_CEC_ENABLE_ALI
#include "hdmi_cec.h"
#endif
#include <alidefinition/adf_boot.h>
#include <alidefinition/adf_snd.h>
#include <linux/ali_transport.h>
extern HDMI_PRIVATE_DATA *hdmi_drv;
extern int hdmi_module_param_hdcp_onoff;

static int hdmi_phy_driving = HDMI_DRV_CUR_105MA;

#ifdef CONFIG_CEC_ENABLE_ALI
bool hdmi_proc_cec_addr_allocation(unsigned short phy_addr)
{
	HDMI_DEBUG("%s: Physical Address is %.1x.%.1x.%.1x.%.1x\n", __FUNCTION__, (phy_addr&0xF000) >> 12, (phy_addr&0x0F00) >> 8, (phy_addr&0x00F0) >> 4, phy_addr&0x000F);
						
	if(phy_addr == 0xFFFF)
		return false;	// Device doesn't have avalid physical address (not F.F.F.F)

	// Allocate the logical address. Ref. CEC 10.2.1
	hdmi_drv->cec.logical_address = LOGADDR_TUNER1;
	hdmi_set_cec_logic_address(LOGADDR_TUNER1);								// Set logical address TUNER 1
	if( hdmi_cec_polling_message(LOGADDR_TUNER1))							// Polling Address TUNER 1
	{
		hdmi_drv->cec.logical_address = LOGADDR_TUNER2;
		hdmi_set_cec_logic_address(LOGADDR_TUNER2);							// Set logical address TUNER 2
		if( hdmi_cec_polling_message(LOGADDR_TUNER2))						// Got ACK, Somedevice use TUNER 1, Polling Address TUNER 2
		{
			hdmi_drv->cec.logical_address = LOGADDR_TUNER3;
			hdmi_set_cec_logic_address(LOGADDR_TUNER3);						// Set logical address TUNER 3
			if( hdmi_cec_polling_message(LOGADDR_TUNER3))					// Got ACK, Somedevice use TUNER 2, Polling Address TUNER 3
			{
				hdmi_drv->cec.logical_address = LOGADDR_TUNER4;
				hdmi_set_cec_logic_address(LOGADDR_TUNER4);					// Set logical address TUNER 4
				if( hdmi_cec_polling_message(LOGADDR_TUNER4))				// Got ACK, Somedevice use TUNER 3, Polling Address TUNER 4
				{
					hdmi_drv->cec.logical_address = LOGADDR_UNREG_BRDCST;	// Failed All addresses(TUNER 1~TUNER 4) had been allocated!
					hdmi_set_cec_logic_address(LOGADDR_UNREG_BRDCST);
					HDMI_DEBUG(" Allocate the CEC logical address Failed!!\n");	
					return false;
				}
			}
		}
	}

    // Report the association between its logical and physical addresses by broadcasting <Report Physical Address>
    if (hdmi_drv->cec.logical_address != LOGADDR_UNREG_BRDCST)
        hdmi_cec_report_physicalAddr();

    return true;
}
#endif

void hdmi_proc_set_avmute(bool set_avmute, unsigned char cnt)
{
	int i;
	
	if (hdmi_drv)
	{
		HDMI_DEBUG("%s %d -> %d !\n", __FUNCTION__, hdmi_drv->control.av_mute_state, set_avmute);
	}
	else
	{
		printk("%s error line%d\n", __FUNCTION__, __LINE__);
		return;
	}

#ifdef AVMUTE_AT_VBLK

    if(set_avmute)  // Set AVMUTE
    {
		vpo_ioctl((struct vpo_device*) hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_SET_DE_AVMUTE_HDMI, TRUE);
    }
    else            // Clear AVMUTE
    {
		vpo_ioctl((struct vpo_device*) hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_SET_DE_AVMUTE_HDMI, FALSE);
    }
	msleep(20);
#else

	//if(set_avmute != hdmi_drv->control.av_mute_state) // ROWA TV issue. mute cause no signal.
	//{
		HDMI_DEBUG("HDMI Proc(%s)\n", (set_avmute) ? "Set AVMUTE":"Clear AVMUTE");
		for(i=0; i<cnt; i++)
		{
			if(HDMI_REG_CFG5 & B_MUTETYPE_SEL)      // Asymmetric Type AVMute
			{
				if(set_avmute)						// Set AVMUTE
				{
					HDMI_REG_CTRL &= ~B_AV_MUTE;    // cleared manually
					udelay(5);
					HDMI_REG_CTRL |= B_AV_MUTE;     // Trigger B_AV_MUTE to send SET_AVMUTE Control packet
				}
				else        						// Clear AVMUTE
				{
					HDMI_REG_CFG0 &= ~B_AV_UNMUTE;	// cleared manually
					udelay(5);
					HDMI_REG_CFG0 |= B_AV_UNMUTE;	// Trigger B_AV_UNMUTE to send CLEAR_AVMUTE Control packet
				}              
			}
			else
			{
				if(set_avmute)  // Set AVMUTE
					HDMI_REG_CTRL |= B_AV_MUTE;                    
				else            // Clear AVMUTE
					HDMI_REG_CTRL &= ~B_AV_MUTE;            
			}
			msleep(20);	// wait at least one video field
		}
	//}
	//else
	//{
	    //HDMI_DEBUG("%s do nothing!\n", __FUNCTION__);
	//}
#endif
	hdmi_drv->control.av_mute_state = set_avmute;
}

bool hdmi_proc_get_avmute_state(void)
{
    return hdmi_drv->control.av_mute_state;
}

/***************************************************************************
	Register			9 mA	9.5 mA	10 mA	10.5 mA	11 mA	11.5 mA
	0xB802A00B [7:0]	0x92	0x92	0xDB	0xDB	0xDB	0xDB
	0xB802A00C [7:0]	0x24	0x24	0xB6	0xB6	0xB6	0xB6
	0xB802A00D [7:0]	0x49	0x49	0x6D	0x6D	0x6D	0x6D
	0xB802A00F [3:1]	0b10	0b11	0b00	0b01	0b10	0b11
***************************************************************************/
void hdmi_proc_set_phy_driving(enum HDMI_TMDS_DRV_CURRENT driving_current)
{
	hdmi_phy_driving = driving_current;

	HDMI_DEBUG("%s 0x%x, %d \n", __FUNCTION__, hdmi_drv->chip_info.chip_id, hdmi_phy_driving); 

//	if(	(hdmi_drv->chip_info.chip_id == 0x3701) ||
//		(hdmi_drv->chip_info.chip_id == 0x3811) )
	if (1)
	{	// Fro C ver 3811 and 3701c
		switch (driving_current)
    	{
	        case HDMI_DRV_CUR_9MA:
	            HDMI_REG_CFG2 = 0x08;
	            HDMI_REG_CFG3 = 0x82;
	            HDMI_REG_CFG4 = 0x20;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x3 << 1);
	            break;
	        case HDMI_DRV_CUR_95MA:// N/A for M3701C
	            HDMI_REG_CFG2 = 0x08;
	            HDMI_REG_CFG3 = 0x82;
	            HDMI_REG_CFG4 = 0x20;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x3 << 1);
	            break;
	        case HDMI_DRV_CUR_10MA:
	            HDMI_REG_CFG2 = 0x10;
	            HDMI_REG_CFG3 = 0x00;
	            HDMI_REG_CFG4 = 0x40;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x00 << 1);
	            break;
	        case HDMI_DRV_CUR_105MA:
	            HDMI_REG_CFG2 = 0x10;
	            HDMI_REG_CFG3 = 0x02;
	            HDMI_REG_CFG4 = 0x40;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x01 << 1);
	            break;
	        case HDMI_DRV_CUR_11MA:
	            HDMI_REG_CFG2 = 0x10;
	            HDMI_REG_CFG3 = 0x80;
	            HDMI_REG_CFG4 = 0x40;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x02 << 1);
	            break;
	        case HDMI_DRV_CUR_115MA:
	            HDMI_REG_CFG2 = 0x10;
	            HDMI_REG_CFG3 = 0x82;
	            HDMI_REG_CFG4 = 0x40;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x03 << 1);
	            break;
		}
	}
	else
	{	// for shuttle version.
		switch (driving_current)
    	{
	        case HDMI_DRV_CUR_9MA:
	            HDMI_REG_CFG2 = 0x49;
	            HDMI_REG_CFG3 = 0x92;
	            HDMI_REG_CFG4 = 0x24;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x03 << 1);
	            break;
	        case HDMI_DRV_CUR_95MA:
	            HDMI_REG_CFG2 = 0x49;
	            HDMI_REG_CFG3 = 0x92;
	            HDMI_REG_CFG4 = 0x24;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x03 << 1);
	            break;
	        case HDMI_DRV_CUR_10MA:
	            HDMI_REG_CFG2 = 0x92;
	            HDMI_REG_CFG3 = 0x24;
	            HDMI_REG_CFG4 = 0x49;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x00 << 1);
	            break;
	        case HDMI_DRV_CUR_105MA:
	            HDMI_REG_CFG2 = 0x92;
	            HDMI_REG_CFG3 = 0x24;
	            HDMI_REG_CFG4 = 0x49;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x01 << 1);
	            break;
	        case HDMI_DRV_CUR_11MA:
	            HDMI_REG_CFG2 = 0x92;
	            HDMI_REG_CFG3 = 0x24;
	            HDMI_REG_CFG4 = 0x49;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x02 << 1);
	            break;
	        case HDMI_DRV_CUR_115MA:
	            HDMI_REG_CFG2 = 0x92;
	            HDMI_REG_CFG3 = 0x24;
	            HDMI_REG_CFG4 = 0x49;
	            HDMI_REG_CFG6 = (HDMI_REG_CFG6 & (~B_EMP_EN)) | (0x03 << 1);
	            break;
		}
    }

	//HDMI_DEBUG("%s 0x%x 0x%x\n", __FUNCTION__, (*(volatile unsigned int *) (0xB802A008)), (*(volatile unsigned int *) (0xB802A00C))); 

}

void hdmi_proc_set_phy_onoff(bool turn_on)
{
#if defined(CONFIG_ALI_CHIP_M3921)
	// C3921 chip limitation, will cause HW INT disable
	//HDMI_REG_3DPHY_REG =  (turn_on) ? (HDMI_REG_3DPHY_REG | B_ENHPDRXSENSE) : (HDMI_REG_3DPHY_REG & (~B_ENHPDRXSENSE));
#else
	M36_SYS_REG_HDMI_PWR_MODE_CTRL  = (turn_on) ? (M36_SYS_REG_HDMI_PWR_MODE_CTRL&(~(0x01<<17))) : (M36_SYS_REG_HDMI_PWR_MODE_CTRL|(0x01<<17));
	//(*(volatile unsigned int *)0xB800006C) = (turn_on) ? ((*(volatile unsigned int *)0xB800006C)&(~(0x01<<17))) : ((*(volatile unsigned int *)0xB800006C)|(0x01<<17));
#endif             
    udelay(10);
}

/* ALi 3821 HDMI phy config flow */
void hdmi_proc_hdmi_phy_output(void)
{
	UINT8 calibration_count = 0;
	HDMI_DEBUG("Config HDMI phy\n");
	// set PCG_SEL 
	hdmi_set_phy_pcg_sel(0x0C);
	//set input video format and clock rate
	hdmi_proc_tmds_clk_update();
	// set DCOLOR_SEL to desired
	switch(hdmi_drv->tmds_clock)
	{
		case HDMI_TMDS_CLK_27000:
		case HDMI_TMDS_CLK_74250:
		case HDMI_TMDS_CLK_148500:
		case HDMI_TMDS_CLK_297000:
			hdmi_set_phy_deepcolor_sel(0x00);
			break;
		case HDMI_TMDS_CLK_27000_125:
		case HDMI_TMDS_CLK_74250_125:
		case HDMI_TMDS_CLK_148500_125:    
			hdmi_set_phy_deepcolor_sel(0x01);
			break;
		case HDMI_TMDS_CLK_27000_15:
		case HDMI_TMDS_CLK_108000:
		case HDMI_TMDS_CLK_74250_15:
		case HDMI_TMDS_CLK_148500_15:
			hdmi_set_phy_deepcolor_sel(0x02);
			break;
		default:
			hdmi_set_phy_deepcolor_sel(0x00);
			break;
	}
	//set CLK_27M according to clock rate
	switch(hdmi_drv->tmds_clock)
	{
		case HDMI_TMDS_CLK_27000:
		case HDMI_TMDS_CLK_27000_125:
		case HDMI_TMDS_CLK_27000_15:
		case HDMI_TMDS_CLK_108000:
			hdmi_set_phy_reference_clk(TRUE);
			break;
		case HDMI_TMDS_CLK_74250:
		case HDMI_TMDS_CLK_74250_125:
		case HDMI_TMDS_CLK_74250_15:
		case HDMI_TMDS_CLK_148500:
		case HDMI_TMDS_CLK_148500_125:
		case HDMI_TMDS_CLK_148500_15:
		case HDMI_TMDS_CLK_297000:
			hdmi_set_phy_reference_clk(FALSE);
			break;
		default:
			hdmi_set_phy_reference_clk(TRUE);
			break;
	}
	udelay(50);
	// reset HDMI PHY
	hdmi_set_phy_rst(TRUE);
	udelay(5);
	hdmi_set_phy_rst(FALSE);
	udelay(5);
	// reset PCG
	hdmi_set_phy_pcg_powerdown(TRUE);
	udelay(5);
	hdmi_set_phy_pcg_powerdown(FALSE);
	udelay(50);
	//wait PCG_RDY = 1
	wait_event_interruptible_timeout(hdmi_drv->control.wait_queue, hdmi_drv->control.phy_int_status & 0x01, 100);
	if((hdmi_drv->control.phy_int_status & 0x01) != 1 )
	{
		HDMI_DEBUG("%s: wait PCG Ready flag fail\n", __FUNCTION__);
	}
	udelay(50);
	//set SEL[0:3] according to clock rate
	switch(hdmi_drv->tmds_clock)
	{
		case HDMI_TMDS_CLK_27000:
		case HDMI_TMDS_CLK_27000_125:
		case HDMI_TMDS_CLK_27000_15:
		case HDMI_TMDS_CLK_108000:
		case HDMI_TMDS_CLK_74250:
		case HDMI_TMDS_CLK_74250_125:
		case HDMI_TMDS_CLK_74250_15:
		case HDMI_TMDS_CLK_148500:
			hdmi_set_phy_sel0(0x00);
			hdmi_set_phy_sel1_2(0x00);
			hdmi_set_phy_sel3(0x00);
			break;
		case HDMI_TMDS_CLK_148500_125:
		case HDMI_TMDS_CLK_148500_15:
		case HDMI_TMDS_CLK_297000:
			hdmi_set_phy_sel0(0x00);
			hdmi_set_phy_sel1_2(0x00);
			hdmi_set_phy_sel3(0x01);
			break;
		default:
			hdmi_set_phy_sel0(0x00);
			hdmi_set_phy_sel1_2(0x00);
			hdmi_set_phy_sel3(0x00);
			break;
	}
	//set CMU_VCOSEL according to clock rate
	switch(hdmi_drv->tmds_clock)
	{
		case HDMI_TMDS_CLK_27000:
		case HDMI_TMDS_CLK_27000_125:
			hdmi_set_cmu_vcosel(0x00);
			break;
		case HDMI_TMDS_CLK_27000_15:
		case HDMI_TMDS_CLK_108000:
		case HDMI_TMDS_CLK_74250:
		case HDMI_TMDS_CLK_74250_125:
			hdmi_set_cmu_vcosel(0x01);
			break;
		case HDMI_TMDS_CLK_74250_15:
			hdmi_set_cmu_vcosel(0x02);
			break;
		case HDMI_TMDS_CLK_148500:
		case HDMI_TMDS_CLK_148500_125:
		case HDMI_TMDS_CLK_148500_15:
		case HDMI_TMDS_CLK_297000:
			hdmi_set_cmu_vcosel(0x03);
			break;
		default:
			hdmi_set_cmu_vcosel(0x00);
			break;
	}
	udelay(200);
	// reset CMU
	hdmi_set_cmu_pd(TRUE);
	udelay(5);
	hdmi_set_cmu_pd(FALSE);
	udelay(50);
	//wait CAL_DN = 1
	while((!((*(volatile UINT8 *)(0xb802a06b)) & B_CAL_DN)) | (calibration_count > 5000))
	{        
		calibration_count ++;		//prevent deadlock
		udelay(10);
		HDMI_DEBUG("HDMI CMU calibration not done.\n");
	}
	udelay(50);
	//wait CMU_RDY
	wait_event_interruptible_timeout(hdmi_drv->control.wait_queue, hdmi_drv->control.phy_int_status & 0x02, 100);
	if((hdmi_drv->control.phy_int_status & 0x02) != 1 )
	{
		HDMI_DEBUG("%s: wait CMU Ready flag fail\n", __FUNCTION__);
	}
	udelay(50);
	//Set DP_MODE default(24 bits)
	hdmi_proc_config_deep_color(hdmi_drv->control.deep_color);
	// set  driving
	switch(hdmi_drv->tmds_clock)
	{
		case HDMI_TMDS_CLK_27000:
		case HDMI_TMDS_CLK_27000_125:
		case HDMI_TMDS_CLK_27000_15:
		case HDMI_TMDS_CLK_108000:
		case HDMI_TMDS_CLK_74250:
		case HDMI_TMDS_CLK_74250_125:
		case HDMI_TMDS_CLK_74250_15:
		case HDMI_TMDS_CLK_148500:
			hdmi_set_drvbst_cntl(0x04);
			hdmi_set_ida_ccntl(0x04);
			hdmi_set_ick_ccntl(0x04);
			hdmi_set_ida_fcntl(0x03);
			hdmi_set_ick_fcntl(0x03);
			hdmi_set_dterm(0x06);
			hdmi_set_cterm(0x06);
			break;
		case HDMI_TMDS_CLK_148500_125:
			hdmi_set_drvbst_cntl(0x07);
			hdmi_set_ida_ccntl(0x05);
			hdmi_set_ick_ccntl(0x04);
			hdmi_set_ida_fcntl(0x03);
			hdmi_set_ick_fcntl(0x03);
			hdmi_set_dterm(0x02);
			hdmi_set_cterm(0x06);
			break;
		case HDMI_TMDS_CLK_148500_15:
		case HDMI_TMDS_CLK_297000:
			hdmi_set_drvbst_cntl(0x07);
			hdmi_set_ida_ccntl(0x07);
			hdmi_set_ick_ccntl(0x04);
			hdmi_set_ida_fcntl(0x00);
			hdmi_set_ick_fcntl(0x03);
			hdmi_set_dterm(0x02);
			hdmi_set_cterm(0x06);
			break;
		default:
			hdmi_set_drvbst_cntl(0x07);
			hdmi_set_ida_ccntl(0x07);
			hdmi_set_ick_ccntl(0x04);
			hdmi_set_ida_fcntl(0x00);
			hdmi_set_ick_fcntl(0x03);
			hdmi_set_dterm(0x02);
			hdmi_set_cterm(0x06);
			break;
	}
}

#ifdef CONFIG_ALI_CHIP_M3921
/* S3921 3D TX PHY config */
void hdmi_proc_3d_phy_output(void)
{
	hdmi_proc_tmds_clk_update();

	switch(hdmi_drv->tmds_clock)
	{
		case HDMI_TMDS_CLK_27000:
		case HDMI_TMDS_CLK_27000_125:
		case HDMI_TMDS_CLK_27000_15:
		case HDMI_TMDS_CLK_27000_2:
			hdmi_set_3d_phy_power(FALSE);
			M39_SYS_REG_HDMI_3DPHY = M39_SYS_REG_HDMI_3DPHY | 0x02;
			M39_SYS_REG_HDMI_3DPHY = M39_SYS_REG_HDMI_3DPHY & (~0x02);
			hdmi_proc_south_bridge_config();
			hdmi_set_3d_phy_power(TRUE);
			HDMI_REG_3DPHY_REG |= (B_VREGLPN | B_ENHPDRXSENSE);
			while(!(HDMI_REG_3DPHY_REG &B_TX_READY))
			{
				udelay(5);
			}
			break;
		case HDMI_TMDS_CLK_74250:
		case HDMI_TMDS_CLK_74250_125:
		case HDMI_TMDS_CLK_74250_15:
		case HDMI_TMDS_CLK_74250_2:
			hdmi_set_3d_phy_power(FALSE);
			M39_SYS_REG_HDMI_3DPHY = M39_SYS_REG_HDMI_3DPHY | 0x02;
			M39_SYS_REG_HDMI_3DPHY = M39_SYS_REG_HDMI_3DPHY & (~0x02);
			hdmi_proc_south_bridge_config();
			hdmi_set_3d_phy_power(TRUE);
			HDMI_REG_3DPHY_REG |= (B_VREGLPN | B_ENHPDRXSENSE);
			while(!(HDMI_REG_3DPHY_REG &B_TX_READY))
			{
				udelay(5);
			}
			break;
		case HDMI_TMDS_CLK_148500:
		case HDMI_TMDS_CLK_148500_125:
		case HDMI_TMDS_CLK_148500_15:
		case HDMI_TMDS_CLK_148500_2:
			hdmi_set_3d_phy_power(FALSE);
			M39_SYS_REG_HDMI_3DPHY = M39_SYS_REG_HDMI_3DPHY | 0x02;
			M39_SYS_REG_HDMI_3DPHY = M39_SYS_REG_HDMI_3DPHY & (~0x02);
			hdmi_proc_south_bridge_config();
			hdmi_set_3d_phy_power(TRUE);
			HDMI_REG_3DPHY_REG |= (B_VREGLPN | B_ENHPDRXSENSE);
			while(!(HDMI_REG_3DPHY_REG &B_TX_READY))
			{
				HDMI_DEBUG("Wait 3D Tx PHY ready\n");
				udelay(5);
			}
			break;
		case HDMI_TMDS_CLK_297000:
			hdmi_set_3d_phy_power(FALSE);
			M39_SYS_REG_HDMI_3DPHY = M39_SYS_REG_HDMI_3DPHY | 0x02;
			M39_SYS_REG_HDMI_3DPHY = M39_SYS_REG_HDMI_3DPHY & (~0x02);
			hdmi_proc_south_bridge_config();
			hdmi_set_3d_phy_power(TRUE);
			HDMI_REG_3DPHY_REG |= (B_VREGLPN | B_ENHPDRXSENSE);
			while(!(HDMI_REG_3DPHY_REG &B_TX_READY))
			{
				HDMI_DEBUG("Wait 3D Tx PHY ready\n");
				udelay(5);
			}
			break;
		default:
			break;
	}
	hdmi_proc_config_digital_value();
}

void hdmi_proc_south_bridge_config(void)
{
	M39_SYS_REG_SB_SLAVE_SCB = 0x55;
	M39_SYS_REG_SB_SLAVE_SEL = 0x00;
	M39_SYS_REG_SB_SLAVE = 0x3C3C00AA;
	M39_SYS_REG_SB_TIME = 0x30303030;
	switch(hdmi_drv->tmds_clock)
	{
		case HDMI_TMDS_CLK_27000:
			if(hdmi_drv->chip_info.bonding_option == 0x00) {
				hdmi_proc_3d_phy_config(0x06, 0x01E0);
				hdmi_proc_3d_phy_config(0x19, 0x0002);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0210);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x0000);
			}
			else{
				hdmi_proc_3d_phy_config(0x06, 0x01E0);
				hdmi_proc_3d_phy_config(0x19, 0x0006);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0273);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x0000);
			}
			break;
		case HDMI_TMDS_CLK_27000_125:
			if(hdmi_drv->chip_info.bonding_option == 0x00) {
				hdmi_proc_3d_phy_config(0x06, 0x21E1);
				hdmi_proc_3d_phy_config(0x19, 0x0002);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0210);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x0000);
			}
			else{
				hdmi_proc_3d_phy_config(0x06, 0x21E1);
				hdmi_proc_3d_phy_config(0x19, 0x0006);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0273);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x0000);
			}
			break;
		case HDMI_TMDS_CLK_27000_15:
			if(hdmi_drv->chip_info.bonding_option == 0x00) {
				hdmi_proc_3d_phy_config(0x06, 0x41E2);
				hdmi_proc_3d_phy_config(0x19, 0x0002);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0210);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x0000);
			}
			else{
				hdmi_proc_3d_phy_config(0x06, 0x41E2);
				hdmi_proc_3d_phy_config(0x19, 0x0006);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0273);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x0000);
			}
			break;
		case HDMI_TMDS_CLK_27000_2:
			if(hdmi_drv->chip_info.bonding_option == 0x00) {
				hdmi_proc_3d_phy_config(0x06, 0x6143);
				hdmi_proc_3d_phy_config(0x19, 0x0002);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0210);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x0005);
			}
			else{
				hdmi_proc_3d_phy_config(0x06, 0x6143);
				hdmi_proc_3d_phy_config(0x19, 0x0006);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0273);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x0005);
			}
			break;
		case HDMI_TMDS_CLK_74250:
			if(hdmi_drv->chip_info.bonding_option == 0x00) {
				hdmi_proc_3d_phy_config(0x06, 0x0140);
				hdmi_proc_3d_phy_config(0x19, 0x0002);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0210);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x0005);
			}
			else{
				hdmi_proc_3d_phy_config(0x06, 0x0140);
				hdmi_proc_3d_phy_config(0x19, 0x0006);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0273);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0000);
				hdmi_proc_3d_phy_config(0x15, 0x0005);
			}
			break;
		case HDMI_TMDS_CLK_74250_125:
			if(hdmi_drv->chip_info.bonding_option == 0x00) {
				hdmi_proc_3d_phy_config(0x06, 0x20A1);
				hdmi_proc_3d_phy_config(0x19, 0x0002);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0210);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x000A);
			}
			else{
				hdmi_proc_3d_phy_config(0x06, 0x20A1);
				hdmi_proc_3d_phy_config(0x19, 0x0006);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0273);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0000);
				hdmi_proc_3d_phy_config(0x15, 0x000A);
			}
			break;
		case HDMI_TMDS_CLK_74250_15:
			if(hdmi_drv->chip_info.bonding_option == 0x00) {
				hdmi_proc_3d_phy_config(0x06, 0x40A2);
				hdmi_proc_3d_phy_config(0x19, 0x0002);
				hdmi_proc_3d_phy_config(0x09, 0x801B);
				hdmi_proc_3d_phy_config(0x0E, 0x0210);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x000A);
			}
			else{
				hdmi_proc_3d_phy_config(0x06, 0x40A2);
				hdmi_proc_3d_phy_config(0x19, 0x0006);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0273);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0000);
				hdmi_proc_3d_phy_config(0x15, 0x000A);
			}
			break;
		case HDMI_TMDS_CLK_74250_2:
			if(hdmi_drv->chip_info.bonding_option == 0x00) {
				hdmi_proc_3d_phy_config(0x06, 0x60A3);
				hdmi_proc_3d_phy_config(0x19, 0x0002);
				hdmi_proc_3d_phy_config(0x09, 0x801B);
				hdmi_proc_3d_phy_config(0x0E, 0x0210);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x000A);

			}
			else{
				hdmi_proc_3d_phy_config(0x06, 0x60A3);
				hdmi_proc_3d_phy_config(0x19, 0x0006);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0273);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0000);
				hdmi_proc_3d_phy_config(0x15, 0x000A);
			}
			break;
		case HDMI_TMDS_CLK_148500:
			if(hdmi_drv->chip_info.bonding_option == 0x00) {
				hdmi_proc_3d_phy_config(0x06, 0x00A0);
				hdmi_proc_3d_phy_config(0x19, 0x0002);
				hdmi_proc_3d_phy_config(0x09, 0x801B);
				hdmi_proc_3d_phy_config(0x0E, 0x0210);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0618);
				hdmi_proc_3d_phy_config(0x15, 0x000A);

			}
			else{
				hdmi_proc_3d_phy_config(0x06, 0x00A0);
				hdmi_proc_3d_phy_config(0x19, 0x0006);
				hdmi_proc_3d_phy_config(0x09, 0x8019);
				hdmi_proc_3d_phy_config(0x0E, 0x0273);
				hdmi_proc_3d_phy_config(0x13, 0x0000);
				hdmi_proc_3d_phy_config(0x17, 0x0006);
				hdmi_proc_3d_phy_config(0x10, 0x0000);
				hdmi_proc_3d_phy_config(0x15, 0x000A);
			}

			break;
		case HDMI_TMDS_CLK_148500_125:
			hdmi_proc_3d_phy_config(0x06, 0x2001);
			hdmi_proc_3d_phy_config(0x19, 0x0006);
			hdmi_proc_3d_phy_config(0x09, 0x8019);
			hdmi_proc_3d_phy_config(0x0E, 0x0273);
			hdmi_proc_3d_phy_config(0x13, 0x0000);
			hdmi_proc_3d_phy_config(0x17, 0x0006);
			hdmi_proc_3d_phy_config(0x10, 0x0000);
			hdmi_proc_3d_phy_config(0x15, 0x000F);
			break;
		case HDMI_TMDS_CLK_148500_15:
			hdmi_proc_3d_phy_config(0x06, 0x4002);
			hdmi_proc_3d_phy_config(0x19, 0x0000);
			hdmi_proc_3d_phy_config(0x09, 0x800B);
			hdmi_proc_3d_phy_config(0x0E, 0x0109);
			hdmi_proc_3d_phy_config(0x13, 0x0000);
			hdmi_proc_3d_phy_config(0x17, 0x0006);
			hdmi_proc_3d_phy_config(0x10, 0x0608);
			hdmi_proc_3d_phy_config(0x15, 0x000F);
			hdmi_proc_3d_phy_config(0x1C, 0x0004);
			break;
		case HDMI_TMDS_CLK_148500_2:
			hdmi_proc_3d_phy_config(0x06, 0x0000);
			hdmi_proc_3d_phy_config(0x19, 0x0000);
			hdmi_proc_3d_phy_config(0x09, 0x801B);
			hdmi_proc_3d_phy_config(0x0E, 0x0109);
			hdmi_proc_3d_phy_config(0x13, 0x0800);
			hdmi_proc_3d_phy_config(0x17, 0x0006);
			hdmi_proc_3d_phy_config(0x10, 0x0018);
			hdmi_proc_3d_phy_config(0x15, 0x000F);
			hdmi_proc_3d_phy_config(0x1C, 0x0004);
			break;
		case HDMI_TMDS_CLK_297000:
			hdmi_proc_3d_phy_config(0x06, 0x0000);
			hdmi_proc_3d_phy_config(0x19, 0x0000);
			hdmi_proc_3d_phy_config(0x09, 0x801B);
			hdmi_proc_3d_phy_config(0x0E, 0x0109);
			hdmi_proc_3d_phy_config(0x13, 0x0800);
			hdmi_proc_3d_phy_config(0x17, 0x0006);
			hdmi_proc_3d_phy_config(0x10, 0x0018);
			hdmi_proc_3d_phy_config(0x15, 0x000F);
			hdmi_proc_3d_phy_config(0x1C, 0x0004);
			break;
		default:
			break;
	}
}
void hdmi_proc_3d_phy_config(unsigned char address, unsigned short value)
{
	M39_SYS_REG_SB_HOST = 0xFF0100C0;
	M39_SYS_REG_SB_FIFO_CTL = 0x83;
	M39_SYS_REG_SB_FIFO_DATA = address;
	M39_SYS_REG_SB_FIFO_DATA = ((value & 0xFF00) >> 8);
	M39_SYS_REG_SB_FIFO_DATA = (value & 0x00FF);
	M39_SYS_REG_SB_HOST |= 0x01;
	while(!(M39_SYS_REG_SB_HOST &0x01000000))
	{
		udelay(10);
		HDMI_DEBUG("Wait SB I2C done : 0x%x\n", M39_SYS_REG_SB_HOST);
	}
}
#endif
void hdmi_proc_tmds_clk_update(void)
{
	/* Temp for 4K x 2K */
	if(TV_MODE_4096X2160_24== hdmi_drv->video.config.resolution || TV_MODE_3840X2160_24== hdmi_drv->video.config.resolution ||
		TV_MODE_3840X2160_25== hdmi_drv->video.config.resolution || TV_MODE_3840X2160_30== hdmi_drv->video.config.resolution)
	{
		hdmi_drv->tmds_clock = HDMI_TMDS_CLK_297000;
	}
	else if(TV_MODE_1080P_50 == hdmi_drv->video.config.resolution || TV_MODE_1080P_60 == hdmi_drv->video.config.resolution)
	{
		//148.5 up
		if(HDMI_DEEPCOLOR_24 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_148500;
		else if(HDMI_DEEPCOLOR_30 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_148500_125;
		else if(HDMI_DEEPCOLOR_36 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_148500_15;
		else if(HDMI_DEEPCOLOR_48 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_148500_2;
		else
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_148500;
		// 3D 
		if(_3D_FORMAT_INDICATION_PRESENTED == hdmi_drv->video.config.ext_video_format)
		{
			if((FRAME_PACKING_3D == hdmi_drv->video.config._4K_VIC_3D_structure) ||
				(SIDE_BY_SIDE_FULL_3D== hdmi_drv->video.config._4K_VIC_3D_structure))
			{
				hdmi_drv->tmds_clock = HDMI_TMDS_CLK_297000;
			}
		}
	}
	else if(TV_MODE_720P_50 == hdmi_drv->video.config.resolution || TV_MODE_720P_60 == hdmi_drv->video.config.resolution ||
		TV_MODE_1080I_25 == hdmi_drv->video.config.resolution  || TV_MODE_1080I_30 == hdmi_drv->video.config.resolution ||
		TV_MODE_1080P_25 == hdmi_drv->video.config.resolution  || TV_MODE_1080P_30 == hdmi_drv->video.config.resolution ||
		TV_MODE_1080P_24 == hdmi_drv->video.config.resolution)
	{
		//74.25 up
		if(HDMI_DEEPCOLOR_24 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_74250;
		else if(HDMI_DEEPCOLOR_30 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_74250_125;
		else if(HDMI_DEEPCOLOR_36 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_74250_15;
		else if(HDMI_DEEPCOLOR_48 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_74250_2;
		else
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_74250;
		// 3D 
		if(_3D_FORMAT_INDICATION_PRESENTED == hdmi_drv->video.config.ext_video_format)
		{
			if((FRAME_PACKING_3D == hdmi_drv->video.config._4K_VIC_3D_structure) ||
				(SIDE_BY_SIDE_FULL_3D== hdmi_drv->video.config._4K_VIC_3D_structure))
			{
				hdmi_drv->tmds_clock = HDMI_TMDS_CLK_148500;
			}
		}
	}
	else if(TV_MODE_PAL == hdmi_drv->video.config.resolution || TV_MODE_PAL_N == hdmi_drv->video.config.resolution ||
		TV_MODE_PAL_M == hdmi_drv->video.config.resolution  || TV_MODE_NTSC358 == hdmi_drv->video.config.resolution ||
		TV_MODE_NTSC443 == hdmi_drv->video.config.resolution  || TV_MODE_576P == hdmi_drv->video.config.resolution ||
		TV_MODE_480P == hdmi_drv->video.config.resolution)
	{
		//27 up
		if(HDMI_DEEPCOLOR_24 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_27000;
		else if(HDMI_DEEPCOLOR_30 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_27000_125;
		else if(HDMI_DEEPCOLOR_36 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_27000_15;
		else if(HDMI_DEEPCOLOR_48 == hdmi_drv->control.deep_color)
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_27000_2;
		else
			hdmi_drv->tmds_clock = HDMI_TMDS_CLK_27000;
		// 3D 
		if(_3D_FORMAT_INDICATION_PRESENTED == hdmi_drv->video.config.ext_video_format)
		{
			if((FRAME_PACKING_3D == hdmi_drv->video.config._4K_VIC_3D_structure) ||
				(SIDE_BY_SIDE_FULL_3D== hdmi_drv->video.config._4K_VIC_3D_structure))
			{
				hdmi_drv->tmds_clock = HDMI_TMDS_CLK_27000_2;
			}
		}
	}
	HDMI_DEBUG("%s: %d \n", __FUNCTION__, hdmi_drv->tmds_clock); 
}
/* config HDMI digital register */
void hdmi_proc_config_digital_value(void)
{
	if((HDMI_RGB == hdmi_drv->control.color_space) || (HDMI_YCBCR_422== hdmi_drv->control.color_space))
		hdmi_set_yuv_ch_sel(0x03);
	else
		hdmi_set_yuv_ch_sel(0x00);
	// 480i & 576i
	if((TV_MODE_NTSC443 == hdmi_drv->video.config.resolution) || (TV_MODE_PAL == hdmi_drv->video.config.resolution))
	{
		// For 480i & 576i VOU_BUF_ADDR_INI issue, need change to 0x03
		hdmi_set_vou_buf_addr(0x03);
		udelay(10);
		HDMI_REG_DP_REG1 &= (~B_RX2_RSTN);
		udelay(10);
		HDMI_REG_DP_REG1 |= B_RX2_RSTN;
		udelay(10);
		HDMI_REG_DP_REG1 |= B_13DOT5_MODE;
	}
	else
	{
		HDMI_REG_DP_REG1 &= (~B_13DOT5_MODE);
	}
	// set polarity for 480 & 576
#ifdef CONFIG_ALI_CHIP_M3921
	if((TV_MODE_480P == hdmi_drv->video.config.resolution) || (TV_MODE_576P == hdmi_drv->video.config.resolution) ||
		(TV_MODE_NTSC443 == hdmi_drv->video.config.resolution) || (TV_MODE_PAL == hdmi_drv->video.config.resolution))
		HDMI_REG_DP_REG2 |= B_DP_POLARITY;
	else
		HDMI_REG_DP_REG2 &= (~B_DP_POLARITY);
#endif

	//Set DP_MODE default(24 bits)
	hdmi_proc_config_deep_color(hdmi_drv->control.deep_color);
}
/* config HDMI deep color mode as 24, 30, 36, 48bit */
INT32 hdmi_proc_config_deep_color(enum HDMI_API_DEEPCOLOR deep_color)
{
	unsigned char i;
	switch(deep_color)	
	{
		case HDMI_DEEPCOLOR_24:
			hdmi_set_dp_mode(0x00);
			hdmi_set_data_buf_grp_sel(0x00);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 & (~B_DP_RSTN);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 | B_DP_RSTN;
			hdmi_set_de_data_width(0x00);
			break;		
		case HDMI_DEEPCOLOR_30:	
			hdmi_set_dp_mode(0x01);	
			hdmi_set_data_buf_grp_sel(0x01);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 & (~B_DP_RSTN);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 | B_DP_RSTN;
			hdmi_set_de_data_width(0x01);
			break;
		case HDMI_DEEPCOLOR_36:
			hdmi_set_dp_mode(0x02);
			hdmi_set_data_buf_grp_sel(0x03);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 & (~B_DP_RSTN);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 | B_DP_RSTN;
			hdmi_set_de_data_width(0x02);
			break;	
		case HDMI_DEEPCOLOR_48:
			hdmi_set_dp_mode(0x03);	
			hdmi_set_data_buf_grp_sel(0x03);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 & (~B_DP_RSTN);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 | B_DP_RSTN;
			hdmi_set_de_data_width(0x03);
			break;		
		default:		
			hdmi_set_dp_mode(0x00);
			hdmi_set_data_buf_grp_sel(0x03);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 & (~B_DP_RSTN);
			HDMI_REG_DP_REG2 = HDMI_REG_DP_REG2 | B_DP_RSTN;
			hdmi_set_de_data_width(0x00);	
			break;	
	} 
	// set packing phase line
	switch(hdmi_drv->video.config.resolution)
	{
		case TV_MODE_NTSC443:
			HDMI_REG_PP_REG0 = 0xF0;
			HDMI_REG_PP_REG1 = (HDMI_REG_PP_REG1 & (~B_PP_LINE_11_8)) | 0x00;
			break;
		case TV_MODE_480P:
			HDMI_REG_PP_REG0 = 0xE0;
			HDMI_REG_PP_REG1 = (HDMI_REG_PP_REG1 & (~B_PP_LINE_11_8)) | 0x01;
#ifdef CONFIG_ALI_CHIP_M3921
			// IC Limitation, can't sent IFM
			if(deep_color == HDMI_DEEPCOLOR_30)
				( __REG32ALI(0x18006210)) = 0x0012003C;
			else
				( __REG32ALI(0x18006210)) = 0x0010003E;
#endif
			break;
		case TV_MODE_PAL:
			HDMI_REG_PP_REG0 = 0x20;
			HDMI_REG_PP_REG1 = (HDMI_REG_PP_REG1 & (~B_PP_LINE_11_8)) | 0x01;
			break;
		case TV_MODE_576P:
			HDMI_REG_PP_REG0 = 0x40;
			HDMI_REG_PP_REG1 = (HDMI_REG_PP_REG1 & (~B_PP_LINE_11_8)) | 0x02;
			break;
		case TV_MODE_720P_50:
		case TV_MODE_720P_60:
			HDMI_REG_PP_REG0 = 0xD0;
			HDMI_REG_PP_REG1 = (HDMI_REG_PP_REG1 & (~B_PP_LINE_11_8)) | 0x02;
			break;
		case TV_MODE_1080I_25:
		case TV_MODE_1080I_30:
			HDMI_REG_PP_REG0 = 0x1C;
			HDMI_REG_PP_REG1 = (HDMI_REG_PP_REG1 & (~B_PP_LINE_11_8)) | 0x02;
			break;
		case TV_MODE_1080P_24:
		case TV_MODE_1080P_25:
		case TV_MODE_1080P_30:
		case TV_MODE_1080P_50:
		case TV_MODE_1080P_60:
			HDMI_REG_PP_REG0 = 0x38;
			HDMI_REG_PP_REG1 = (HDMI_REG_PP_REG1 & (~B_PP_LINE_11_8)) | 0x04;
			break;
		default:
			HDMI_REG_PP_REG0 = 0x1C;
			HDMI_REG_PP_REG1 = (HDMI_REG_PP_REG1 & (~B_PP_LINE_11_8)) | 0x02;
			break;
	}
	mdelay(200);
	for(i = 0; i < 20; i++)
	{
		HDMI_DEBUG("PP vlaue : %d\n", hdmi_get_pp_value());
	}
	hdmi_drv->control.pp_value = hdmi_get_pp_value();
	HDMI_REG_PP_REG1 = HDMI_REG_PP_REG1 | B_CLEAR_PP_VALUE;
	HDMI_REG_PP_REG1 = HDMI_REG_PP_REG1 & (~B_CLEAR_PP_VALUE);
	HDMI_DEBUG("%s: %d \n", __FUNCTION__, hdmi_drv->control.deep_color);	
	return TRUE;
}

/* config HDMI color space mode */
void hdmi_proc_config_color_space(enum HDMI_API_COLOR_SPACE color_space)
{
	switch(color_space)
	{
		case HDMI_RGB:
			HDMI_REG_DP_REG0 = HDMI_REG_DP_REG0 & (~B_RGB_YUV_SEL);
			HDMI_REG_DP_REG0 = HDMI_REG_DP_REG0 & (~B_444TO422_EN);
			hdmi_set_yuv_ch_sel(0x03);
			break;
		case HDMI_YCBCR_422:
			HDMI_REG_DP_REG0 = HDMI_REG_DP_REG0 | B_RGB_YUV_SEL;
			HDMI_REG_DP_REG0 = HDMI_REG_DP_REG0 | B_444TO422_EN;
			hdmi_set_yuv_ch_sel(0x03);
			break;
		case HDMI_YCBCR_444:
			HDMI_REG_DP_REG0 = HDMI_REG_DP_REG0 | B_RGB_YUV_SEL;
			HDMI_REG_DP_REG0 = HDMI_REG_DP_REG0 & (~B_444TO422_EN);
			hdmi_set_yuv_ch_sel(0x00);
			break;
		default:
			break;
	}
}

/* calculate CheckSum value of InfoFrame Packet */
char hdmi_proc_calculate_check_sum(INFOFRAME* infoframe)
{
	short check_sum         = 0;
	unsigned char length    = infoframe->Length + 1;
	unsigned char i;
    
	check_sum = infoframe->Type + infoframe->Version + infoframe->Length;
	for (i=1; i<length; i++)
	{
		check_sum += infoframe->databyte[i];
	}
	return -check_sum;
}

void hdmi_proc_hardware_init(void)
{
	HDMI_DEBUG("HDMI hardware init\n");             
	// Reset HDMI Module (System Register)
	M36_SYS_REG_RST_CTRL |= (0x01 << (13));
	udelay(10);
	M36_SYS_REG_RST_CTRL &= ~(0x01 << (13));
	udelay(10);

	// Reset HDMI buffer, state machine (SRST) & Power down HDMI PHY (Turn off TMDS signal)
	HDMI_REG_CTRL |= B_SRST;
	udelay(10);
		
	hdmi_proc_set_phy_onoff(false);

	// Set to HDMI Working Mode
	HDMI_REG_CFG1 = (hdmi_drv->control.hdmi_dvi_mode == HDMI_MODE) ? (HDMI_REG_CFG1 & (~B_DVI)):(HDMI_REG_CFG1 | B_DVI);     

	// Set Notice High to avoid audio jitter issue
	HDMI_REG_CFG0 |= B_NOTICE_HIGH;
	
	// Set AVMUTE Type, Enable AVMute asymtric design. (S3602 D0)
	HDMI_REG_CFG5 |= B_MUTETYPE_SEL;
            
	if(hdmi_drv->chip_info.chip_id == 0x3503)
	{
		*((volatile unsigned int *)0xB8000400) = (*((volatile unsigned int *)0xB8000400) & ~(0x00001F00));
	}

	// Set YCbCr/RGB value
	if(hdmi_drv->chip_info.chip_id == 0x3921)
	{
		HDMI_REG_Y_L =  0x00;
		HDMI_REG_Y_H = 0x10;
		HDMI_REG_CB_L = 0x00;
		HDMI_REG_CB_H = 0x80;
		HDMI_REG_CR_L = 0x00;
		HDMI_REG_CR_H = 0x80;
	}

	// Set phy setting
	if(hdmi_drv->chip_info.chip_id == 0x3503)
	{
		/*00:292 BGA Package, 01:256 LQFP Package*/
		if((hdmi_drv->chip_info.bonding_option = 0x00) ||(hdmi_drv->chip_info.bonding_option = 0x01))
		{
			hdmi_set_pll_sel(0x04);
			hdmi_set_txp_ctl0(0x04);
			hdmi_set_txn_ctl0(0x04);
			hdmi_set_txp_ctl1(0x06);
			hdmi_set_txn_ctl1(0x06);
			hdmi_set_txp_ctl2(0x04);
			hdmi_set_txn_ctl2(0x04);
			hdmi_set_txp_ctl3(0x02);
			hdmi_set_txn_ctl3(0x02);
			hdmi_set_emp_en(0x00);
			hdmi_set_ldo_sel(0x00);
		}
		/*11:144 LQFP Package*/
		else if(hdmi_drv->chip_info.bonding_option = 0x03)
		{
			hdmi_set_pll_sel(0x04);
			hdmi_set_txp_ctl0(0x02);
			hdmi_set_txn_ctl0(0x02);
			hdmi_set_txp_ctl1(0x06);
			hdmi_set_txn_ctl1(0x06);
			hdmi_set_txp_ctl2(0x04);
			hdmi_set_txn_ctl2(0x04);
			hdmi_set_txp_ctl3(0x02);
			hdmi_set_txn_ctl3(0x02);
			hdmi_set_emp_en(0x01);
			hdmi_set_ldo_sel(0x01);
		}
	}

	// Set Pre Driver Control
	HDMI_REG_PHY_DRV_CTRL = 0x0777;

	// Enable Info frame Packet
	if(hdmi_drv->chip_info.chip_id == 0x3921)
	{
		HDMI_REG_CTRL |= (B_GENERIC_EN | B_AUDIO_EN | B_AVI_EN | B_SPD_EN);
		HDMI_REG_GMP |= B_VSI_EN;
		HDMI_REG_CFG6 |= B_GCP_EN;
	}
	else
	{
		HDMI_REG_CTRL |= (B_GENERIC_EN | B_AUDIO_EN | B_AVI_EN | B_SPD_EN);
	}

	// CTS Generate by Hardware or Software ?
#if 0	// By Software
	HDMI_REG_CTS_CTRL = HDMI_REG_CTS_CTRL | B_CTS_1_INI  | B_SOFT; 				
#else	// By Hardware
	HDMI_REG_CTS_CTRL = (HDMI_REG_CTS_CTRL | B_CTS_1_INI) & (~B_SOFT); 
#endif

	HDMI_REG_CFG5  = HDMI_REG_CFG5 | B_CTS_UPDATE;

	// Clear All HDMI and HDCP Interrupt
	HDMI_REG_INT = HDMI_REG_INT;
	HDMI_REG_HDCP_STATUS = HDMI_REG_HDCP_STATUS;

	// Set Interrupt Mask
	HDMI_REG_INT_MASK = B_INT_NCTS_DONE | B_INT_IFM_ERR | B_INT_INF_DONE | B_INT_FIFO_U | B_INT_FIFO_O;

	HDMI_REG_CFG6     |= B_PORD_MASK;

    // Fix 480P/576P 重影issue // fixed by de.20120827
	// Set Async mode and FIFO R/W
	if(hdmi_drv->chip_info.chip_id == 0x3503)
	{
		HDMI_REG_CFG6 = (HDMI_REG_CFG6 & ~B_ASYNC_MODE);
		udelay(5);
		HDMI_REG_CFG6 = (HDMI_REG_CFG6 | 0xA0);
	}

#ifdef CONFIG_CEC_ENABLE_ALI
	hdmi_cec_init();
#endif

}

void hdmi_proc_transmit_infoframe(INFOFRAME *infoframe)
{
	int i;
	// Write InfoFrame Header to HDMI Hardware Module
	HDMI_REG_INFRM_TYPE     = infoframe->Type;
	HDMI_REG_INFRM_VER      = infoframe->Version;	
	if(infoframe->Type == GCP_TYPE)
		HDMI_REG_INFRM_LENGTH   = 0x00;
	else
		HDMI_REG_INFRM_LENGTH   = infoframe->Length;

	// Write InfoFrame Content to HDMI Hardware Module
	if(infoframe->Type == GCP_TYPE)
	{
		for(i=0; i< infoframe->Length; i++)
			HDMI_REG_INFRM_DATA = infoframe->databyte[i];
	}
	else
	{
		for(i=0; i<= infoframe->Length; i++)
		HDMI_REG_INFRM_DATA = infoframe->databyte[i];
	}
}

void hdmi_proc_vsi_infoframe_update(void)
{
    // VSI InfoFrame
	hdmi_drv->vsi.infoframe.Type = VSI_IFM_TYPE;
	hdmi_drv->vsi.infoframe.Version = VSI_IFM_VER;
	hdmi_drv->vsi.infoframe.Length = VSI_IFM_LEN;

	hdmi_drv->vsi.infoframe.vsi.IEEE_reg_id[0] = 0x03;
	hdmi_drv->vsi.infoframe.vsi.IEEE_reg_id[1] = 0x0C;
	hdmi_drv->vsi.infoframe.vsi.IEEE_reg_id[2] = 0x00;
	hdmi_drv->vsi.infoframe.vsi.ext_video_format = hdmi_drv->video.config.ext_video_format << 5;
	if((hdmi_drv->vsi.infoframe.vsi.ext_video_format & 0xE0) == 0x20)		// Extended resolution format(ex 4Kx2K) present.
	{
		switch (hdmi_drv->video.config.resolution)
		{
			case TV_MODE_4096X2160_24:
				hdmi_drv->vsi.infoframe.vsi._4K_VIC_3D_structure = 0x04;
				break;
			case TV_MODE_3840X2160_24:
				hdmi_drv->vsi.infoframe.vsi._4K_VIC_3D_structure = 0x03;
				break;
			case TV_MODE_3840X2160_25:
				hdmi_drv->vsi.infoframe.vsi._4K_VIC_3D_structure = 0x02;
				break;
			case TV_MODE_3840X2160_30:
				hdmi_drv->vsi.infoframe.vsi._4K_VIC_3D_structure = 0x01;
				break;
			default:
				hdmi_drv->vsi.infoframe.vsi._4K_VIC_3D_structure = 0x00;
				break;
		}
	}
	else if((hdmi_drv->vsi.infoframe.vsi.ext_video_format & 0xE0) == 0x40)	// 3D format indication present.
	{
		// _3D_structure
		hdmi_drv->vsi.infoframe.vsi._4K_VIC_3D_structure = hdmi_drv->video.config._4K_VIC_3D_structure << 4;
		if(hdmi_drv->vsi.infoframe.vsi._4K_VIC_3D_structure & 0x80)
		{	// 3D_Ext_data field indicates addition information.
			hdmi_drv->vsi.infoframe.vsi.ext_data = hdmi_drv->video.config.ext_data << 4;
		}
		else
		{	// 3D_Ext_Data field shall not be present if 3D_Structure is 0000 ~ 0111
			hdmi_drv->vsi.infoframe.vsi.ext_data = 0;
		}
	}
	else
	{
		hdmi_drv->vsi.infoframe.vsi._4K_VIC_3D_structure = 0x00;
		hdmi_drv->vsi.infoframe.vsi.ext_data = 0;
	}
	memset(hdmi_drv->vsi.infoframe.vsi.reserve, 0, 22);
	hdmi_drv->vsi.infoframe.vsi.check_sum = hdmi_proc_calculate_check_sum(&hdmi_drv->vsi.infoframe);
}

void hdmi_proc_avi_infoframe_update(void)
{

    hdmi_drv->video.infoframe.avi.scan_information              = SCAN_INFO_OVERSCANED;	  // SCAN_INFO_NO_DATA
    hdmi_drv->video.infoframe.avi.bar_information               = BAR_INFO_NOT_VALID;//VERT_HORIZ_BAR_INFO_VALID;//BAR_INFO_NOT_VALID;
    hdmi_drv->video.infoframe.avi.reserved_1                    = 0;
    hdmi_drv->video.infoframe.avi.non_uniform_picture_scaling	= NO_KNOWN_SCALING;
    hdmi_drv->video.infoframe.avi.rgb_quantization_range	    = RGB_QUANT_RANGE_DEFAULT;
    hdmi_drv->video.infoframe.avi.extended_colorimetry          = EXT_CLRMETRY_XVYCC601;
    hdmi_drv->video.infoframe.avi.it_content                    = IT_NO_DATA;
    hdmi_drv->video.infoframe.avi.pixel_repetition_factor       = PIX_REP_PIX_NO_REPETITION;
    hdmi_drv->video.infoframe.avi.reserved_2                    = 0;
    hdmi_drv->video.infoframe.avi.ln_etb_lower                  = 0x00;
    hdmi_drv->video.infoframe.avi.ln_etb_upper                  = 0x00;
    hdmi_drv->video.infoframe.avi.ln_sbb_lower                  = 0x00;
    hdmi_drv->video.infoframe.avi.ln_sbb_upper                  = 0x00;
    hdmi_drv->video.infoframe.avi.pn_elb_lower                  = 0x00;
    hdmi_drv->video.infoframe.avi.pn_elb_upper                  = 0x00;
    hdmi_drv->video.infoframe.avi.pn_srb_lower                  = 0x00;
    hdmi_drv->video.infoframe.avi.pn_srb_upper                  = 0x00;

    // Setup Video Code, Colorimetry & Aspect ratio Information
    hdmi_drv->video.infoframe.avi.picture_aspect_ratio          = AR_16_9;
    hdmi_drv->video.infoframe.avi.colorimetry                   = COLORIMETRY_ITU709;

    switch (hdmi_drv->video.config.resolution)
    {
        case TV_MODE_PAL:       // 576i
        case TV_MODE_PAL_N:               
            hdmi_drv->video.infoframe.avi.picture_aspect_ratio      = (hdmi_drv->video.config.aspect_ratio == TV_4_3) ? AR_4_3 : AR_16_9;           
            hdmi_drv->video.infoframe.avi.video_identfy_code        = (hdmi_drv->video.config.aspect_ratio == TV_4_3) ? VIC_1440x576I_50HZ_4_3 : VIC_1440x576I_50HZ_16_9;
            hdmi_drv->video.infoframe.avi.colorimetry               = COLORIMETRY_ITU601;   
            hdmi_drv->video.infoframe.avi.pixel_repetition_factor   = PIX_REP_PIX_SENT_TWO_TIMES;                        
            break;	
        case TV_MODE_PAL_M:     // 480i
        case TV_MODE_NTSC358:
        case TV_MODE_NTSC443:
            hdmi_drv->video.infoframe.avi.picture_aspect_ratio      = (hdmi_drv->video.config.aspect_ratio == TV_4_3) ? AR_4_3 : AR_16_9;           
            hdmi_drv->video.infoframe.avi.video_identfy_code        = (hdmi_drv->video.config.aspect_ratio == TV_4_3) ? VIC_1440x480I_60HZ_4_3 : VIC_1440x480I_60HZ_16_9;
            hdmi_drv->video.infoframe.avi.colorimetry               = COLORIMETRY_ITU601;   
            hdmi_drv->video.infoframe.avi.pixel_repetition_factor   = PIX_REP_PIX_SENT_TWO_TIMES;             
            break;
        case TV_MODE_576P:    
            hdmi_drv->video.infoframe.avi.picture_aspect_ratio      = (hdmi_drv->video.config.aspect_ratio == TV_4_3) ? AR_4_3 : AR_16_9;           
            hdmi_drv->video.infoframe.avi.video_identfy_code        = (hdmi_drv->video.config.aspect_ratio == TV_4_3) ? VIC_720x576P_50HZ_4_3 : VIC_720x576P_50HZ_16_9;
            hdmi_drv->video.infoframe.avi.colorimetry               = COLORIMETRY_ITU601;             
            break;
        case TV_MODE_480P:    
            hdmi_drv->video.infoframe.avi.picture_aspect_ratio      = (hdmi_drv->video.config.aspect_ratio == TV_4_3) ? AR_4_3 : AR_16_9;           
            hdmi_drv->video.infoframe.avi.video_identfy_code        = (hdmi_drv->video.config.aspect_ratio == TV_4_3) ? VIC_720x480P_60HZ_4_3 : VIC_720x480P_60HZ_16_9;
            hdmi_drv->video.infoframe.avi.colorimetry               = COLORIMETRY_ITU601;           
            break;              
        case TV_MODE_720P_50:   hdmi_drv->video.infoframe.avi.video_identfy_code    = VIC_1280x720P_50HZ;   break; 
        case TV_MODE_720P_60:   hdmi_drv->video.infoframe.avi.video_identfy_code    = VIC_1280x720P_60HZ;   break; 
        case TV_MODE_1080I_25:  hdmi_drv->video.infoframe.avi.video_identfy_code    = VIC_1920x1080I_50HZ;  break; 
        case TV_MODE_1080I_30:  hdmi_drv->video.infoframe.avi.video_identfy_code    = VIC_1920x1080I_60HZ;  break; 
        case TV_MODE_1080P_50:  hdmi_drv->video.infoframe.avi.video_identfy_code    = VIC_1920x1080P_50HZ;  break; 
        case TV_MODE_1080P_60:  hdmi_drv->video.infoframe.avi.video_identfy_code    = VIC_1920x1080P_60HZ;  break;
        case TV_MODE_1080P_25:  hdmi_drv->video.infoframe.avi.video_identfy_code    = VIC_1920x1080P_25HZ;  break;
        case TV_MODE_1080P_30:  hdmi_drv->video.infoframe.avi.video_identfy_code    = VIC_1920x1080P_30HZ;  break;
        case TV_MODE_1080P_24:  hdmi_drv->video.infoframe.avi.video_identfy_code    = VIC_1920x1080P_24HZ;  break;
	case TV_MODE_4096X2160_24: hdmi_drv->video.infoframe.avi.video_identfy_code = 0; break;
	case TV_MODE_3840X2160_24: hdmi_drv->video.infoframe.avi.video_identfy_code = 0; break;
	case TV_MODE_3840X2160_25: hdmi_drv->video.infoframe.avi.video_identfy_code = 0; break;
	case TV_MODE_3840X2160_30: hdmi_drv->video.infoframe.avi.video_identfy_code = 0; break;
        default:
            HDMI_DEBUG("%s: Unsupport vidoe resolution %d\n", __FUNCTION__, hdmi_drv->video.config.resolution);             
            break;
    }

#if 0
    // Fix 480P/576P 重影issue
	if(hdmi_drv->video.config.resolution == TV_MODE_480P || hdmi_drv->video.config.resolution == TV_MODE_576P)
    {
		HDMI_REG_CFG6 = HDMI_REG_CFG6 & 0x9F;            //  0xb802A00F[6:5] to 0x0
	}
    else
	{
		HDMI_REG_CFG6 = (HDMI_REG_CFG6 & 0x9F) | 0x20;   //  0xb802A00F[6:5] to 0x1
	}
#endif
#ifdef CONFIG_ALI_CHIP_M3921
	 switch (hdmi_drv->control.color_space)
	{
		case HDMI_YCBCR_422:         hdmi_drv->video.infoframe.avi.rgb_ycbcr     = AVI_FORMAT_YCBCR_422;         break;            
		case HDMI_YCBCR_444:         hdmi_drv->video.infoframe.avi.rgb_ycbcr     = AVI_FORMAT_YCBCR_444;         break;  
		case HDMI_RGB:         hdmi_drv->video.infoframe.avi.rgb_ycbcr     = AVI_FORMAT_RGB;               break;
		default:
			HDMI_DEBUG("%s: Unknown color format %d\n", __FUNCTION__, hdmi_drv->control.color_space); 
			break;  
	}
#else
    // Setup Color Information
    switch (hdmi_drv->video.config.color_format)
    {
        case YCBCR_422:         hdmi_drv->video.infoframe.avi.rgb_ycbcr     = AVI_FORMAT_YCBCR_422;         break;            
        case YCBCR_444:         hdmi_drv->video.infoframe.avi.rgb_ycbcr     = AVI_FORMAT_YCBCR_444;         break;  
        case RGB_MODE1:
        case RGB_MODE2:         hdmi_drv->video.infoframe.avi.rgb_ycbcr     = AVI_FORMAT_RGB;               break;
        default:
            HDMI_DEBUG("%s: Unknown color format %d\n", __FUNCTION__, hdmi_drv->video.config.color_format); 
            break;  
    }
#endif

    // Setup AFD Information
    hdmi_drv->video.infoframe.avi.acitve_format_info_present = AFI_PRESENT_VALID;
    hdmi_drv->video.infoframe.avi.active_format_aspect_ratio = (hdmi_drv->video.config.afd_present) ? hdmi_drv->video.config.afd : AFD_AS_CODE_FRAME;       

	HDMI_REG_CFG6 = (HDMI_REG_CFG6 | B_ASYNC_SET | (B_ASYNC_SET &(~0x40)));
    // Calculate Check Sum    
    hdmi_drv->video.infoframe.avi.check_sum = (unsigned char) hdmi_proc_calculate_check_sum(&hdmi_drv->video.infoframe);

	HDMI_DEBUG("%s: color_format %d\n", __FUNCTION__, hdmi_drv->video.config.color_format); 
}

static void hdmi_proc_audio_cfg_i2s_intf(void)
{

	HDMI_DEBUG("%s: Config Audio to LPCM Interface\n", __FUNCTION__);
    HDMI_REG_CFG1 &= ~(B_SPDIF);	// I2S Interface
    hdmi_drv->audio.config.channel_status.audio_content_flag = 0;	// content flag = 0: PCM Data

	// Clear CFG0 Register, I2S_Mode, LRCK_INV, W_LENGTH Bits first
    HDMI_REG_CFG0 &= ~(B_W_LENGTH | B_LRCK | B_I2S_MODE);  

	// Setup CFG0 Register: NOTICE_HIGH, W_LENGTH, LRCK_INV, I2S_MODE
    HDMI_REG_CFG0 |= B_NOTICE_HIGH | (hdmi_drv->audio.config.word_length << 4) | (hdmi_drv->audio.config.i2s_format);

	if (hdmi_drv->audio.config.i2s_format == I2S_FMT_I2S)
	    HDMI_REG_CFG0 = (hdmi_drv->audio.config.lrck_hi_left) ? (HDMI_REG_CFG0 | B_LRCK) : (HDMI_REG_CFG0 & (~B_LRCK));
	else
	    HDMI_REG_CFG0 = (hdmi_drv->audio.config.lrck_hi_left) ? (HDMI_REG_CFG0 & (~B_LRCK)) : (HDMI_REG_CFG0 | B_LRCK);

    if (hdmi_drv->chip_info.chip_version2 == 0x01)
    {
        //HDMI_REG_CTRL |= SEL_128FS;
    }

    HDMI_REG_I2S_CHANNEL_STATUS             = hdmi_drv->audio.config.channel_status.data_uint32;
    HDMI_REG_I2S_CHANNEL_STATUS_LAST_BYTE   = 0x00;

    // Set Channel Count to downmix output for draft version
//    hdmi_drv->audio.config.channel_count    = 2;

	// Reset Channel Position & Enable Channels
    HDMI_REG_I2S_UV &= ~(B_CH_EN | B_USER_BIT | B_VALIDITY_BIT);
    switch (hdmi_drv->audio.config.channel_count)
    {
#if defined(CONFIG_ALI_M3603)||defined(CONFIG_ALI_M3606)
        case 2:
            HDMI_REG_I2S_UV |= (0x08 << 2);
            break;          // Stereo output: Enable CH3(7/8) For M36
#else
        case 2:
            HDMI_REG_I2S_UV |= (0x01 << 2);
            break;          // Stereo output: Enable CH3(1/2) For M39
#endif
        case 6:
            HDMI_REG_I2S_UV |= (0x07 << 2);
            break;          // 5.1 Channel output: Enable CH0(1/2), CH1(3/4), CH2(5/6)
        case 8:
        default:
            HDMI_REG_I2S_UV |= (0x0F << 2);
            break;          // 7.1 channel output: Enable All Channel
    }
}

static void hdmi_proc_audio_cfg_pd_intf(void)
{
	HDMI_DEBUG("%s: Config Audio to SPDIF Interface\n", __FUNCTION__);
	hdmi_drv->audio.config.channel_status.audio_content_flag = 1;
	HDMI_REG_CFG1   |= B_SPDIF;       
	HDMI_REG_I2S_UV &= ~(B_CH_EN | B_USER_BIT | B_VALIDITY_BIT);
	HDMI_REG_I2S_UV |= (0x0F << 2);// Enable All Channel
	if (hdmi_drv->chip_info.chip_version2 == 0x01)
	{
		//HDMI_REG_CTRL &= ~(SEL_128FS);
	}
}

void hdmi_proc_audio_interface_config(void)
{
    if (hdmi_drv->audio.config.user_audio_out == AUD_USR_LPCM_OUT)
    {
		hdmi_proc_audio_cfg_i2s_intf();
    }
	else if(hdmi_drv->audio.config.user_audio_out == AUD_USR_BITSTREAM_OUT)
	{
    	hdmi_proc_audio_cfg_pd_intf();
	}
	else if(hdmi_drv->audio.config.user_audio_out == AUD_USR_AUTO)
	{
		unsigned short aud_fmt_code;
		edid_get_prefer_audio_out(&aud_fmt_code);
		HDMI_DEBUG("edid_get_prefer_audio_out %x \n", aud_fmt_code);
		if(aud_fmt_code == 0x00)
		{
			HDMI_DEBUG("config_audio_interface: Parsing EDID Audio Support Failed, configure to LPCM Out \n");
  			hdmi_proc_audio_cfg_i2s_intf();
		}
		else
		{
			HDMI_DEBUG("config_audio_interface: HDMI Sink support Audio Codec \n");	
			if(hdmi_drv->audio.config.coding_type == AUD_CODING_PCM)
			{
				hdmi_proc_audio_cfg_i2s_intf();
			}
			else
			{
				if(aud_fmt_code & (1 << (hdmi_drv->audio.config.coding_type - 1)))
				{
					if( (hdmi_drv->audio.config.coding_type == AUD_CODING_AC3) ||
						(hdmi_drv->audio.config.coding_type == AUD_CODING_DTS) ||
						(hdmi_drv->audio.config.coding_type == AUD_CODING_DD_PLUS))
					{
						if(aud_fmt_code == 0x01) // Sink Device Support PCM only.
							hdmi_proc_audio_cfg_i2s_intf();
						else
						{
						    if (hdmi_drv->audio.config.coding_type == AUD_CODING_DD_PLUS)
						    {
						        snd_io_control((struct snd_device*) hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_HDMI_CONFIG_SPO_CLK, 0);
						        snd_io_control((struct snd_device*) hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_SET_BS_OUTPUT_SRC, 0);
						    }
							hdmi_proc_audio_cfg_pd_intf();
					    }
					}
					else
						hdmi_proc_audio_cfg_i2s_intf();
				}
				else
				{
					if (hdmi_drv->audio.config.coding_type == AUD_CODING_DD_PLUS)
					{
						snd_io_control((struct snd_device*) hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_HDMI_CONFIG_SPO_CLK, 1);
						snd_io_control((struct snd_device*) hld_dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_SET_BS_OUTPUT_SRC, 1);
						if(aud_fmt_code & (1 << (AUD_CODING_AC3 - 1)))
						{
							hdmi_proc_audio_cfg_pd_intf();
						}
						else
						{
							hdmi_proc_audio_cfg_i2s_intf();
						}
					}
					else
					{
						hdmi_proc_audio_cfg_i2s_intf();
					}
				}
			}
		}
	}
}

void hdmi_proc_audio_infoframe_update(void)
{

    hdmi_drv->audio.infoframe.audio.reserved_1              = 0;
    hdmi_drv->audio.infoframe.audio.audio_coding_type       = AUD_CODE_CT_REF_STREAM;
    hdmi_drv->audio.infoframe.audio.sample_size             = SAMPLE_SIZE_REF_STREAM;
    hdmi_drv->audio.infoframe.audio.sampling_frequency      = SAMPLE_FREQ_REF_STREAM;
    hdmi_drv->audio.infoframe.audio.level_shift_value       = LSV_0dB;    
    hdmi_drv->audio.infoframe.audio.down_mix_inhibit_flag   = DOWN_MIX_PERMITTED;
    hdmi_drv->audio.infoframe.audio.reserved_2              = 0;
    hdmi_drv->audio.infoframe.audio.reserved_3              = 0;
    hdmi_drv->audio.infoframe.audio.reserved_4              = 0;
    hdmi_drv->audio.infoframe.audio.reserved_5              = 0x00;
    hdmi_drv->audio.infoframe.audio.reserved_6              = 0x00;
    hdmi_drv->audio.infoframe.audio.reserved_7              = 0x00;
    hdmi_drv->audio.infoframe.audio.reserved_8              = 0x00;
    hdmi_drv->audio.infoframe.audio.reserved_9              = 0x00;	


    // Setup Video Code, Colorimetry & Aspect ratio Information
    hdmi_drv->audio.infoframe.audio.audio_channel_count     = CH_CNT_REF_STREAM;
    switch (hdmi_drv->audio.config.channel_count)
    {
        case 2:     hdmi_drv->audio.infoframe.audio.speaker_channel_allocation = 0x00;  break;
        case 6:     hdmi_drv->audio.infoframe.audio.speaker_channel_allocation = 0x0B;  break;       
        case 8:     hdmi_drv->audio.infoframe.audio.speaker_channel_allocation = 0x13;  break;
    }

    // Calculate Check Sum
    hdmi_drv->audio.infoframe.audio.check_sum = (unsigned char) hdmi_proc_calculate_check_sum(&hdmi_drv->audio.infoframe);

}

void hdmi_proc_audio_n_cts_update(void)
{
                                            /* 32K      44.1K   48K    88.2K    96K     176.4K  192K    768K  Hz */
    static const unsigned int n_table[8]=   {  4096,    6272,   6144,  12544,   12288,  25088,  24576,  98304};
    static const unsigned int cts_table[5][8] = {
        { 25200,  28000,  28000,  25200,  28000,  25200,  25200, 25200},	// 25.2Mhz  -> 32k, 44.1k, 48k, 88.2k, 96k, 176.4k, 192k
        { 27000,  30000,  27000,  30000,  27000,  30000,  27000, 27000},	// 27Mhz    -> 32k, 44.1k, 48k, 88.2k, 96k, 176.4k, 192k
        { 54000,  60000,  54000,  60000,  54000,  60000,  54000, 54000},	// 54Mhz    -> 32k, 44.1k, 48k, 88.2k, 96k, 176.4k, 192k
        { 74250,  82500,  74250,  82500,  74250,  82500,  74250, 74250},	// 74.25Mhz -> 32k, 44.1k, 48k, 88.2k, 96k, 176.4k, 192k
        {148500, 165000, 148500, 165000, 148500, 165000, 148500, 148500}	// 148.5Mhz -> 32k, 44.1k, 48k, 88.2k, 96k, 176.4k, 192k
    };
    static const unsigned int resolution_to_pixelclock[18] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 4, 4, 3, 3, 3 };

    if (hdmi_drv->audio.config.sample_rate == AUD_SAMPLERATE_UNKNOWN)
    {
    	HDMI_DEBUG("%s: Sampling Rate is Unknown\n", __FUNCTION__);
        hdmi_drv->audio.n   = 0;
        hdmi_drv->audio.cts = 0;
    }
    else
    {
        hdmi_drv->audio.n   = n_table[hdmi_drv->audio.config.sample_rate];
        hdmi_drv->audio.cts = cts_table[resolution_to_pixelclock[hdmi_drv->video.config.resolution]][hdmi_drv->audio.config.sample_rate];
    }
    HDMI_DEBUG("%s: Sampling Rate index = %d\n", __FUNCTION__, hdmi_drv->audio.config.sample_rate);
    HDMI_DEBUG("%s: N = %d\n", __FUNCTION__, hdmi_drv->audio.n);
    HDMI_DEBUG("%s: CTS = %d\n", __FUNCTION__, hdmi_drv->audio.cts);

    // Write N, CTS to HDMI Module
    HDMI_REG_NCTS = 0x00;                                               // Byte0 0x00
    HDMI_REG_NCTS = ((hdmi_drv->audio.cts & 0x00FF0000) >> 16) & 0x0F;  // Byte1 4'b0000 | CTS[19:16]
    HDMI_REG_NCTS = ((hdmi_drv->audio.cts & 0x0000FF00) >> 8);          // Byte2 CTS[15:8]    
    HDMI_REG_NCTS =  (hdmi_drv->audio.cts & 0x000000FF);                // Byte3 CTS[7:0] 
    HDMI_REG_NCTS = ((hdmi_drv->audio.n   & 0x00FF0000) >> 16) & 0x0F;  // Byte4 4'b0000 | N[19:16]
    HDMI_REG_NCTS = ((hdmi_drv->audio.n   & 0x0000FF00) >> 8);          // Byte5 N[15:8]    
    HDMI_REG_NCTS =  (hdmi_drv->audio.n   & 0x000000FF);                // Byte6 N[7:0]     
       
}

void hdmi_proc_spd_update(void)
{

	memcpy(hdmi_drv->spd.infoframe.spd.vendor_name, hdmi_drv->spd.vendor_name, 8);
	memcpy(hdmi_drv->spd.infoframe.spd.product_desc, hdmi_drv->spd.product_desc, 16);
	hdmi_drv->spd.infoframe.spd.source_device_information = SDI_DIGITAL_STB;

	// Calculate Check Sum
	hdmi_drv->spd.infoframe.spd.check_sum =	(unsigned char) hdmi_proc_calculate_check_sum(&hdmi_drv->spd.infoframe);
}

void hdmi_proc_gcp_update(void)
{
	hdmi_drv->gcp.infoframe.gcp.set_avmute = 0x00;
	hdmi_drv->gcp.infoframe.gcp.reserved_0 = 0x00;
	hdmi_drv->gcp.infoframe.gcp.clean_avmute = 0x00;
	hdmi_drv->gcp.infoframe.gcp.reserved_1 = 0x00;
	switch(hdmi_drv->tmds_clock)
	{
		case HDMI_TMDS_CLK_27000:		
		case HDMI_TMDS_CLK_74250:		
		case HDMI_TMDS_CLK_108000:		
		case HDMI_TMDS_CLK_148500:		
		case HDMI_TMDS_CLK_297000:
			hdmi_drv->gcp.infoframe.gcp.color_depth = COLOR_DEPTH_NOT_INDICATED;
			break;
		case HDMI_TMDS_CLK_27000_125:		
		case HDMI_TMDS_CLK_74250_125:		
		case HDMI_TMDS_CLK_148500_125:
			hdmi_drv->gcp.infoframe.gcp.color_depth = COLOR_DEPTH_30;
			break;
		case HDMI_TMDS_CLK_27000_15:		
		case HDMI_TMDS_CLK_74250_15:	
		case HDMI_TMDS_CLK_148500_15:
			hdmi_drv->gcp.infoframe.gcp.color_depth = COLOR_DEPTH_36;
			break;
		case HDMI_TMDS_CLK_27000_2:	
		case HDMI_TMDS_CLK_74250_2:
		case HDMI_TMDS_CLK_148500_2:
			hdmi_drv->gcp.infoframe.gcp.color_depth = COLOR_DEPTH_48;
			break;
		default:
			break;
	}
	switch(hdmi_drv->control.pp_value)
	{
		case 0:
			hdmi_drv->gcp.infoframe.gcp.packing_phase = PACKING_PHASE4;
			break;
		case 1:
			hdmi_drv->gcp.infoframe.gcp.packing_phase = PACKING_PHASE1;
			break;
		case 2:
			hdmi_drv->gcp.infoframe.gcp.packing_phase = PACKING_PHASE2;
			break;
		case 3:
			hdmi_drv->gcp.infoframe.gcp.packing_phase = PACKING_PHASE3;
			break;
		case 4:
			hdmi_drv->gcp.infoframe.gcp.packing_phase = PACKING_PHASE4;
			break;
		default:
			hdmi_drv->gcp.infoframe.gcp.packing_phase = PACKING_PHASE4;
			break;
	}
	hdmi_drv->gcp.infoframe.gcp.default_phase = 0x00;
	hdmi_drv->gcp.infoframe.gcp.reserved_2 = 0x00;
	hdmi_drv->gcp.infoframe.gcp.sub_byte3 = 0x00;
	hdmi_drv->gcp.infoframe.gcp.sub_byte4 = 0x00;
	hdmi_drv->gcp.infoframe.gcp.sub_byte5 = 0x00;
	hdmi_drv->gcp.infoframe.gcp.sub_byte6 = 0x00;
}

void hdmi_proc_get_parse_edid(void)
{
    struct i2c_adapter *adapter;

    unsigned char segment = 0x00, word_offset = 0x00;
    int i, j;

    struct i2c_msg msgs[] = {
        { /* DDC Addr 0x60, Segment Pointer = 0 */
            .addr	= 0x30,
            .flags	= I2C_M_IGNORE_NAK,
            .len	= 1,
            .buf	= &segment,
        },
        { /* DDC Addr 0xA0, Word Offset = 0 */
            .addr	= 0x50,
            .flags	= 0,
            .len	= 1,
            .buf	= &word_offset,
        },	
        { /* DDC Addr 0xA1, WRead = 0 */
            .addr	= 0x50,
            .flags	= I2C_M_RD,
            .len	= 128,
            .buf	= hdmi_drv->edid.block[0],
        }
    };
	
	hdmi_edid_clear();
	
    adapter = i2c_get_adapter(hdmi_drv->control.eddc_adapter_id);
    if (adapter)
    {
        adapter->retries = 0;   /* be replaced by defines       */
        // Get EDID 1.3 Block
        if (i2c_transfer(adapter, msgs, 3) == 3)
        {
            if((hdmi_drv->edid.block[0][0x12] == 0x01) && (hdmi_drv->edid.block[0][0x13] == 0x00))
            {
                EDID_DEBUG("EDID 1.0 Content: \n");
            }
            else if((hdmi_drv->edid.block[0][0x12] == 0x01) && (hdmi_drv->edid.block[0][0x13] == 0x01))
            {
                EDID_DEBUG("EDID 1.1 Content: \n");
            }
            else if((hdmi_drv->edid.block[0][0x12] == 0x01) && (hdmi_drv->edid.block[0][0x13] == 0x02))
            {
                EDID_DEBUG("EDID 1.2 Content: \n");
            }
            else if((hdmi_drv->edid.block[0][0x12] == 0x01) && (hdmi_drv->edid.block[0][0x13] == 0x03))
            {
                EDID_DEBUG("EDID 1.3 Content: \n");
            }
            else if((hdmi_drv->edid.block[0][0x12] == 0x01) && (hdmi_drv->edid.block[0][0x13] == 0x04))
            {
                EDID_DEBUG("EDID 1.4 Content: \n");
                // Check EDID header. EDID 1.4 explicit requirment
                if( (hdmi_drv->edid.block[0][0] != 0x00) || (hdmi_drv->edid.block[0][1] != 0xFF) ||
				    (hdmi_drv->edid.block[0][2] != 0xFF) || (hdmi_drv->edid.block[0][3] != 0xFF) ||
				    (hdmi_drv->edid.block[0][4] != 0xFF) || (hdmi_drv->edid.block[0][5] != 0xFF) ||
				    (hdmi_drv->edid.block[0][6] != 0xFF) || (hdmi_drv->edid.block[0][7] != 0x00))
                {
                    // For I2C data singal issue.
				    HDMI_REG_CFG1 = HDMI_REG_CFG1 & (~B_DVI);
				    return;
                }
            }
            for(i=0; i<128; i++)
			{
                EDID_DEBUG("%.2x %s", hdmi_drv->edid.block[0][i], (i%16 == 15) ? "\n":"");
			}
            fb_edid_to_monspecs(hdmi_drv->edid.block[0], &hdmi_drv->edid.mon_specs);

            // Because the marco "CONFIG_FB_MODE_HELPERS" not define in autoconfig.h, fb_edid_to_monspecs is NULL
            hdmi_edid_parse_std(hdmi_drv->edid.block[0]);
            // Get EDID Extension Blocks
            if (hdmi_drv->edid.block[0][0x7E] > MAX_EDID_BLOCKS_SUPPORT - 1)
            {
                EDID_DEBUG("HDMI Driver: MAX_EDID_BLOCKS_SUPPORT(%d) not enouth!\n", MAX_EDID_BLOCKS_SUPPORT);
                hdmi_drv->edid.number_of_extension_block = MAX_EDID_BLOCKS_SUPPORT - 1;
            }
            else
                hdmi_drv->edid.number_of_extension_block = hdmi_drv->edid.block[0][0x7E];

            for(j=1; j<=hdmi_drv->edid.number_of_extension_block; j++)
            {
                segment = j / 2;
                word_offset = (j % 2 == 0) ? 0 : 128;
                msgs[2].buf = hdmi_drv->edid.block[j];
                if (i2c_transfer(adapter, msgs, 3) == 3)
                {
                    EDID_DEBUG("EDID Extension Block(%d): \n", j);
                    for(i=0; i<128; i++)
                    {
                        EDID_DEBUG("%.2x %s", hdmi_drv->edid.block[j][i], (i % 16 == 15) ? "\n" : "");
                    }
                }
                hdmi_edid_parse_extension(hdmi_drv->edid.block[j]);
            }
            HDMI_REG_CFG1 = (hdmi_drv->edid.found_IEEE_reg_code) ? (HDMI_REG_CFG1 & (~B_DVI)) : (HDMI_REG_CFG1 | B_DVI);
            hdmi_drv->control.edid_ready = true;
        }
        else
            HDMI_REG_CFG1 = HDMI_REG_CFG1 & (~B_DVI);
    }
    else
        HDMI_REG_CFG1 = HDMI_REG_CFG1 & (~B_DVI);
}

void hdmi_proc_clear_edid(void)
{
    memset(&hdmi_drv->edid, 0, sizeof(EDID_DATA));
    hdmi_drv->control.edid_ready = false;
}


#ifdef CONFIG_HDCP_ENABLE_ALI

unsigned char ALi_HDCP_Key[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

void hdmi_hdcp_config(void)
{
	ADF_BOOT_BOARD_INFO *board_info = (ADF_BOOT_BOARD_INFO *)(ALI_VIRT(PRE_DEFINED_ADF_BOOT_START));
    //hdmi_drv->control.hdcp_enable = (bool)hdmi_module_param_hdcp_onoff;
	hdmi_drv->control.hdcp_enable =  (!(board_info->avinfo.hdcp_disable));

#if defined(CONFIG_HDCP_SOURCE_MAX_KSV_ALI)
    hdmi_drv->hdcp.source_max_ksv = CONFIG_HDCP_SOURCE_MAX_KSV_ALI;
#else
	hdmi_drv->hdcp.source_max_ksv = 9;
#endif // CONFIG_HDCP_SOURCE_MAX_KSV_ALI

#if defined(CONFIG_HDCP_SOURCE_AUTHE_COUNT_ALI)
    hdmi_drv->hdcp.source_authe_count = CONFIG_HDCP_SOURCE_AUTHE_COUNT_ALI;
#else
	 hdmi_drv->hdcp.source_authe_count = 1;
#endif // CONFIG_HDCP_SOURCE_AUTHE_COUNT_ALI

#if defined(CONFIG_HDCP_SOURCE_OUT_ONLYREP_ALI)
    hdmi_drv->hdcp.source_out_onlyrep = CONFIG_HDCP_SOURCE_OUT_ONLYREP_ALI;
#endif // CONFIG_HDCP_SOURCE_OUT_ONLYREP_ALI

	//memcpy(hdmi_drv->hdcp.key, ALi_HDCP_Key1, 286);
	if(hdmi_drv->control.hdcp_enable == TRUE)
	{
		ADF_BOOT_BOARD_INFO *board_info = (ADF_BOOT_BOARD_INFO *)(ALI_VIRT(PRE_DEFINED_ADF_BOOT_START));

		memcpy(hdmi_drv->hdcp.key, board_info->hdcp.hdcp, 286);
	}

    HDMI_DEBUG("%s: hdcp %s\n", __FUNCTION__, (hdmi_drv->control.hdcp_enable) ? "enable" : "disable");
    HDMI_DEBUG("%s: source_max_ksv = %d\n", __FUNCTION__, hdmi_drv->hdcp.source_max_ksv);
    HDMI_DEBUG("%s: source_authe_count = %d\n", __FUNCTION__, hdmi_drv->hdcp.source_authe_count);
    HDMI_DEBUG("%s: source_out_onlyrep is %s\n", __FUNCTION__, (hdmi_drv->hdcp.source_out_onlyrep) ? "true" : "false");
	
    hdmi_drv->hdcp.authenticated = false;
}
//a void hdmi_proc_hdcp_auth(struct work_struct *);
//a DECLARE_WORK(hdmi_hdcp_auth_work, hdmi_proc_hdcp_auth);

void hdmi_hdcp_cipher_reset(void) //arthur unused
{
    hdmi_drv->hdcp.authenticated = false;
    HDMI_REG_HDCP_CTRL &= ~B_ENC_EN;
	udelay(5);
    HDMI_REG_HDCP_CTRL |= B_CP_RST;
    udelay(5);
    HDMI_REG_HDCP_CTRL &= ~B_CP_RST;
    udelay(5);
    // Reset SHA, Auth, Encrypt Engine, and (disable scramble)
    HDMI_REG_HDCP_CTRL &= ~( B_SHA_EN | B_AUTHEN_EN | B_ENC_EN | B_SCRAMBLE);
    udelay(5);
}

// arthur replaced by hdcp_thread
void hdmi_proc_hdcp_auth(struct work_struct *work)
{
	HDMI_DEBUG("entry hdmi_proc_hdcp_auth %d %d\n", hdmi_drv->control.hdmi_state, hdmi_drv->control.hdcp_enable);
    while ((hdmi_drv->control.hdmi_state == HDMI_STATE_PLAY) && (hdmi_drv->control.hdcp_enable))
    {

        if (hdmi_drv->hdcp.authenticated)       // Stage #3
        {
            wait_event_interruptible_timeout(hdmi_drv->control.wait_queue, hdmi_drv->control.hdcp_int_status & B_RI_RDY, 1);
            if(hdmi_drv->control.hdcp_int_status & B_RI_RDY)
            {
                hdmi_drv->control.hdcp_int_status &= ~B_RI_RDY;
                if (hdmi_drv->hdcp.authenticated)
                {
                    HDMI_DEBUG("hdmi_proc_hdcp_auth 3rd\n");
                    hdmi_drv->hdcp.authenticated = hdmi_hdcp_3rd_ri_link_integrity_check();
                }
            }
            if (hdmi_drv->control.need_reauth)
                hdmi_drv->hdcp.authenticated = false;
        }
        else                    // Stage #1
        {
            if (hdmi_drv->control.need_reauth)
            {
                hdmi_drv->control.need_reauth = false;
                //Reset HDCP Cipher
                HDMI_REG_HDCP_CTRL |= B_CP_RST;
                udelay(5);
                HDMI_REG_HDCP_CTRL &= ~B_CP_RST;
                udelay(5);
            }
            hdmi_drv->hdcp.authenticated = hdmi_hdcp_1st_establish_share_value();
			HDMI_DEBUG("%s: 1st auth %s\n", __FUNCTION__, (hdmi_drv->hdcp.authenticated) ? "Success" : "Failed");
			if(hdmi_drv->hdcp.authenticated)
			{
           		// A6: Test for Repeater
            	HDCP_DEBUG("HDCP A6: Test for Repeater\n");
            	if (hdmi_drv->hdcp.sink_is_repeater && hdmi_drv->hdcp.authenticated)
            	{
               		HDCP_DEBUG("\tRx is Repeater, should verify repeater KSV List\n");
                	hdmi_drv->hdcp.authenticated = hdmi_hdcp_2nd_verify_repeater_ksvlist();
                	HDMI_DEBUG("%s: 2nd auth %s\n", __FUNCTION__, (hdmi_drv->hdcp. authenticated) ? "Success" : "Failed");
            	}
            	else
            	{
                	HDCP_DEBUG("\tRx is not Repeater, Enter Authenticated State %d %d\n", hdmi_drv->hdcp.sink_is_repeater, hdmi_drv->hdcp.authenticated);
            	}
			}

        }

		msleep(20);
    }
	HDMI_DEBUG("eixt hdmi_proc_hdcp_auth %d %d\n", hdmi_drv->control.hdmi_state, hdmi_drv->control.hdcp_enable);
}

void hdmi_proc_hdcp_stop(void)
{
	HDMI_DEBUG("%s\n", __FUNCTION__);
    hdmi_drv->hdcp.authenticated = false;
    hdmi_drv->control.hdcp_int_status = 0;

    HDMI_REG_HDCP_CTRL |= B_CP_RST;     //Reset HDCP Cipher
    udelay(5);
    HDMI_REG_HDCP_CTRL &= ~B_CP_RST;
    udelay(5);
    // Reset SHA, Auth, Encrypt Engine, and (disable scramble)
    HDMI_REG_HDCP_CTRL &= ~( B_SHA_EN | B_AUTHEN_EN | B_ENC_EN | B_SCRAMBLE);
    udelay(5);
}

static int hdcp_thread(void *num)
{
	HDCP_DEBUG("%s called, hdcp %s.\n", __FUNCTION__, (hdmi_drv->control.hdcp_enable) ? "on" : "off");
	while(1)
	{
	    if ((hdmi_drv->control.hdmi_state == HDMI_STATE_PLAY) && (hdmi_drv->control.hdcp_enable))
	    {
	        if (hdmi_drv->hdcp.authenticated)       // Stage #3
	        {
	            wait_event_interruptible_timeout(hdmi_drv->control.wait_queue, hdmi_drv->control.hdcp_int_status & B_RI_RDY, 2600); //128/50=2.56(s)
	            if(hdmi_drv->control.hdcp_int_status & B_RI_RDY)
	            {
	                hdmi_drv->control.hdcp_int_status &= ~B_RI_RDY;
	                if (hdmi_drv->hdcp.authenticated)
	                {
						HDMI_DEBUG("hdmi_proc_hdcp_auth 3rd %lu\n", jiffies);
	                    hdmi_drv->hdcp.authenticated = hdmi_hdcp_3rd_ri_link_integrity_check();
						hdmi_drv->link_status = ((hdmi_drv->link_status | HDMI_STATUS_LINK_HDCP_SUCCESSED) &(~(HDMI_STATUS_LINK_HDCP_FAILED|HDMI_STATUS_LINK_HDCP_IGNORED)));
	                }
	            }
				else
				{
					HDMI_DEBUG("hdmi_proc_hdcp_auth 3rd wait Ri timeout\n");
					//hdmi_drv->hdcp.authenticated = false;
				}
	            if (hdmi_drv->control.need_reauth)
	                hdmi_drv->hdcp.authenticated = false;

	        }
	        else                    // Stage #1
	        {
				hdmi_drv->link_status = ((hdmi_drv->link_status | HDMI_STATUS_LINK_HDCP_FAILED) &(~(HDMI_STATUS_LINK_HDCP_SUCCESSED |HDMI_STATUS_LINK_HDCP_IGNORED)));
	            hdmi_drv->hdcp.authenticated = hdmi_hdcp_1st_establish_share_value();
				HDCP_DEBUG("%s: 1st auth %s\n", __FUNCTION__, (hdmi_drv->hdcp.authenticated) ? "Success" : "Failed");
				if(hdmi_drv->control.aksv_fail == TRUE)
					msleep(2000);
				if(hdmi_drv->hdcp.authenticated)
				{
	           		// A6: Test for Repeater
	            	HDCP_DEBUG("HDCP A6: Test for Repeater\n");
	            	if (hdmi_drv->hdcp.sink_is_repeater && hdmi_drv->hdcp.authenticated)
	            	{
	               		HDCP_DEBUG("\tRx is Repeater, should verify repeater KSV List\n");
	                	hdmi_drv->hdcp.authenticated = hdmi_hdcp_2nd_verify_repeater_ksvlist();
	                	HDMI_DEBUG("%s: 2nd auth %s\n", __FUNCTION__, (hdmi_drv->hdcp.authenticated) ? "Success" : "Failed");
	            	}
	            	else
	            	{
	                	HDCP_DEBUG("\tRx is not Repeater, Enter Authenticated State\n");
	            	}
				}
	        }
	    }
		else
		{
			hdmi_drv->hdcp.authenticated = false;
		}
		msleep(5);
		if(kthread_should_stop())
		{
			HDCP_DEBUG("eixt hdmi_proc_hdcp_auth %d %d\n", hdmi_drv->control.hdmi_state, hdmi_drv->control.hdcp_enable);
			break;
		}
		
	}
	HDCP_DEBUG("%s leaved\n", __FUNCTION__);
	return 0;
}

#endif

static bool hdmi_send_edid_ready_msg(HDMI_PRIVATE_DATA *priv)
{
	bool ret = true;
	//EDID_READY_MSG *pedid = &(priv->hdmi2usr.edid_notify);
	unsigned char msg[11];
	int msg_size = 11;
	int id = 0;

	HDMI_DEBUG("%s in:\n", __FUNCTION__);

	memset((void*) msg, 0x00, 11);
	msg[0] = ALI_HDMI_MSG_EDID;
	msg[1] = 0;
	msg[2] = 8;
	//memcpy((void*)(msg + 3), (void*)(pedid->parg1), 8); // N/A
#if 0
	if (ali_transport_send_msg(priv->hdmi2usr.port_id, msg, msg_size) == -1)
	{
		HDMI_DEBUG("%s error line%d\n", __FUNCTION__, __LINE__);
		ret = false;
	}
#endif
	//for(id = 0; id <= net_id; id++)
	while(priv->hdmi2usr.port_id[id] != 0)
	{
		if (ali_transport_send_msg(priv->hdmi2usr.port_id[id], msg, msg_size) == -1)
		{
			HDMI_DEBUG("%s error line%d\n", __FUNCTION__, __LINE__);
			ret = false;
			break;
		}
		id++;
	}
	return ret;
}

static bool hdmi_send_plgin_msg(HDMI_PRIVATE_DATA *priv)
{
	bool ret = true;
	//HOT_PLUGIN_MSG *pplugin = &(priv->hdmi2usr.plgin_notify);
	unsigned char msg[11];
	int msg_size = 11;
	int id = 0;

	HDMI_DEBUG("%s in:\n", __FUNCTION__);

	memset((void*)msg, 0x00, 11);
	msg[0] = ALI_HDMI_MSG_PLUGIN;
	msg[1] = 0;
	msg[2] = 8;
	//memcpy((void*)(msg + 3), (void*)(pplugin->parg1), 8); // N/A
#if 0
	if(ali_transport_send_msg(priv->hdmi2usr.port_id, msg, msg_size) == -1)
	{
		HDMI_DEBUG("%s error line%d\n", __FUNCTION__, __LINE__);
		ret = false;
	}
#endif
	//for(id = 0; id <= net_id; id++)
	while(priv->hdmi2usr.port_id[id] != 0)
	{
		if (ali_transport_send_msg(priv->hdmi2usr.port_id[id], msg, msg_size) == -1)
		{
			HDMI_DEBUG("%s error line%d\n", __FUNCTION__, __LINE__);
			ret = false;
			break;
		}
		id++;
	}
	return ret;
}

static bool hdmi_send_plugout_msg(HDMI_PRIVATE_DATA *priv)
{
	bool ret = true;
	//HOT_PLUGOUT_MSG *pplugout = &(priv->hdmi2usr.plgout_notify);
	unsigned char msg[11];
	int msg_size = 11;
	int id = 0;

	HDMI_DEBUG("%s in:\n", __FUNCTION__);

	memset((void*)msg, 0x00, 11);
	msg[0] = ALI_HDMI_MSG_PLUGOUT;
	msg[1] = 0;
	msg[2] = 8;
	//memcpy((void*)(msg + 3), (void*)(pplugout->parg1), 8); // N/A
#if 0
	if(ali_transport_send_msg(priv->hdmi2usr.port_id, msg, msg_size) == -1)
	{
		HDMI_DEBUG("%s error line%d\n", __FUNCTION__, __LINE__);
		ret = false;
	}
#endif
	//for(id = 0; id <= net_id; id++)
	while(priv->hdmi2usr.port_id[id] != 0)
	{
		if (ali_transport_send_msg(priv->hdmi2usr.port_id[id], msg, msg_size) == -1)
		{
			HDMI_DEBUG("%s error line%d\n", __FUNCTION__, __LINE__);
			ret = false;
			break;
		}
		id++;
	}
	return ret;
}


static bool hdmi_send_cec_msg(HDMI_PRIVATE_DATA *priv)
{
	bool ret = true;
	CEC_MSG *pcec_msg = &(priv->hdmi2usr.cec_notify);
	unsigned char msg[19];
	int msg_size = pcec_msg->len + 3;
	int id = 0;

	HDMI_DEBUG("%s in:\n", __FUNCTION__);

	memset((void*)msg, 0x00, 19);
	msg[0] = ALI_HDMI_MSG_CEC;
	msg[1] = 0;
	msg[2] = pcec_msg->len;
	memcpy((void*)(msg + 3), (void*)(pcec_msg->data), 16);
#if 0
	if(ali_transport_send_msg(priv->hdmi2usr.port_id, msg, msg_size) == -1)
	{
		HDMI_DEBUG("%s error line%d\n", __FUNCTION__, __LINE__);
		ret = false;
	}
#endif
	//for(id = 0; id <= net_id; id++)
	while(priv->hdmi2usr.port_id[id] != 0)
	{
		if (ali_transport_send_msg(priv->hdmi2usr.port_id[id], msg, msg_size) == -1)
		{
			HDMI_DEBUG("%s error line%d\n", __FUNCTION__, __LINE__);
			ret = false;
			break;
		}
		id++;
	}
	return ret;
}

static bool hdmi_send_msg(HDMI_PRIVATE_DATA *priv, UINT32 flag)
{
	bool ret = true;

	HDMI_DEBUG("%s in: 0x%x 0x%x\n", __FUNCTION__, priv, flag);
	//if((priv == NULL) || (priv->hdmi2usr.port_id == 0))
	if(priv == NULL)
	{	
		HDMI_DEBUG("%s error line%d\n", __FUNCTION__, __LINE__);
		return false;
	}
	
	if(flag & HDMI2USR_EDID_READY_MSG)
	{
		ret = hdmi_send_edid_ready_msg(priv);
	}
	if(flag & HDMI2USR_HOTPLUG_IN_MSG)
	{
		ret = hdmi_send_plgin_msg(priv);
	}
	if(flag & HDMI2USR_HOTPLUG_OUT_MSG)
	{
		ret = hdmi_send_plugout_msg(priv);
	}
	if(flag & HDMI2USR_CEC_MSG)
	{
		ret = hdmi_send_cec_msg(priv);
	}	
	priv->hdmi2usr.flag = 0;
	return ret;
}

void hdmi_proc_update_avinfo(void)
{
    hdmi_proc_avi_infoframe_update();
    hdmi_proc_transmit_infoframe(&hdmi_drv->video.infoframe);
}

void hdmi_proc_update_audioinfo(void)
{
	hdmi_proc_audio_interface_config();
	hdmi_proc_audio_infoframe_update();
	hdmi_proc_audio_n_cts_update();
	hdmi_proc_transmit_infoframe(&hdmi_drv->audio.infoframe);
}

void hdmi_proc_state_update(void)
{
    bool hotplug = false;
    /* Ike, SHA request : Remove PORD detect*/
//	bool pord_Status = false;

	/* Update Hot Plug In/Out State */
    hotplug = HOT_PLUG_STATE;
//	pord_Status = HOT_PORD_STATE;
	HDMI_DEBUG(KERN_ALERT "%s: 0x%.4x\n", __FUNCTION__, HDMI_REG_STATUS);
//    HDMI_DEBUG(KERN_ALERT "%s: %s\n", __FUNCTION__, (hotplug && pord_Status) ? "HotPlug-In":"HotPlug-Out");	
    HDMI_DEBUG(KERN_ALERT "%s: %s\n", __FUNCTION__, (hotplug) ? "HotPlug-In":"HotPlug-Out");
	
//    if ((hotplug) && (pord_Status) && (hdmi_drv->control.hdmi_state == HDMI_STATE_IDLE))      // HotPlug-In
    if ((hotplug) && (hdmi_drv->control.hdmi_state == HDMI_STATE_IDLE))      // HotPlug-In
    {
        HDMI_DEBUG("%s: IDLE State -> Ready State\n", __FUNCTION__);
        hdmi_drv->control.hdmi_state = HDMI_STATE_READY;

		hdmi_drv->link_status = ((hdmi_drv->link_status | HDMI_STATUS_LINK) &(~HDMI_STATUS_UNLINK));

        HDMI_DEBUG("HDMI Get & Parsing EDID\n");
        hdmi_proc_get_parse_edid();
		// process edid ready event
		hdmi_drv->hdmi2usr.flag |= HDMI2USR_EDID_READY_MSG;
		hdmi_send_msg(hdmi_drv, hdmi_drv->hdmi2usr.flag);
		// process plug in event
		hdmi_drv->hdmi2usr.flag |= HDMI2USR_HOTPLUG_IN_MSG;
		hdmi_send_msg(hdmi_drv, hdmi_drv->hdmi2usr.flag);
//		hdmi_drv->plug_status = 0x5a5a5a5a;
//		wake_up_interruptible(&hdmi_drv->poll_plug_wq);
        hdmi_proc_set_phy_onoff(true);
        msleep(10);

#ifdef CONFIG_CEC_ENABLE_ALI
        if (hdmi_drv->control.cec_enable)
       	{
			hdmi_set_cec_rst(false);
//			if(hdmi_drv->edid.physical_address != 0xFFFF)
//				hdmi_proc_cec_addr_allocation(hdmi_drv->edid.physical_address);
        }
#endif
    }
//    else if ((!(hotplug) || (!pord_Status)) && (hdmi_drv->control.hdmi_state != HDMI_STATE_IDLE) )        // HotPlug-Out
    else if ((!(hotplug)) && (hdmi_drv->control.hdmi_state != HDMI_STATE_IDLE) )        // HotPlug-Out
    {
        HDMI_DEBUG("%s: %s State -> IDLE State\n", __FUNCTION__, (hdmi_drv->control.hdmi_state == HDMI_STATE_READY) ? "Ready" : "Play");

	hdmi_drv->link_status = ((hdmi_drv->link_status | HDMI_STATUS_UNLINK) &(~HDMI_STATUS_LINK));
#ifdef CONFIG_CEC_ENABLE_ALI
        // CEC Feature
        if (hdmi_drv->control.cec_enable)
        {
            hdmi_set_cec_rst(true);
            udelay(5);
        }
#endif
#ifdef CONFIG_HDCP_ENABLE_ALI
        hdmi_proc_hdcp_stop();
#endif
        HDMI_REG_CTRL |= B_SRST;    // Set SRST to 1 (trun off Video data to state machine)
        udelay(5);
        HDMI_DEBUG("HDMI Clear EDID\n");
        hdmi_proc_clear_edid();

		// process plug out event
		hdmi_drv->hdmi2usr.flag |= HDMI2USR_HOTPLUG_OUT_MSG;
		hdmi_send_msg(hdmi_drv, hdmi_drv->hdmi2usr.flag);
//		hdmi_drv->plug_status = 0xa5a5a5a5;
//		wake_up_interruptible(&hdmi_drv->poll_plug_wq);
		
        hdmi_drv->control.hdmi_state = HDMI_STATE_IDLE;
//		hdmi_proc_set_avmute(true, 1);
        hdmi_proc_set_phy_onoff(false);
    }
	else
	{
		HDMI_DEBUG("%s: plug in/out undo State \n", __FUNCTION__);
		HDMI_DEBUG("pord_Status %d\n", HOT_PORD_STATE);
		HDMI_DEBUG("hotplug %d\n", HOT_PLUG_STATE);
		HDMI_DEBUG("hdmi_enable %d\n", hdmi_drv->control.hdmi_enable);
		HDMI_DEBUG("hdmi_state %d\n", hdmi_drv->control.hdmi_state);
	}

    /* Turn Off HDMI */
    if((hdmi_drv->control.hdmi_enable == false) && (hdmi_drv->control.hdmi_state == HDMI_STATE_PLAY))
    {
		HDMI_DEBUG("%s: Play State -> Ready State\n", __FUNCTION__);
			
		hdmi_drv->control.hdmi_state = HDMI_STATE_READY;
		
		// Turn off Audio
		HDMI_REG_CFG6 &= (~B_AUDIO_CAP_RST);
		udelay(5);
		HDMI_REG_CTRL &= (~B_AUDIO_EN);
		udelay(5);
		HDMI_REG_CFG5 |= B_I2S_SWITCH;
		HDMI_REG_CFG5 |= B_SPDIF_SWITCH;
		udelay(5);

		hdmi_proc_set_avmute(true, 1);
		
#ifdef CONFIG_HDCP_ENABLE_ALI
        hdmi_proc_hdcp_stop();
#endif

//		{// reduce driving current after mute
//			HDMI_REG_CFG2 = 0x00;
//			HDMI_REG_CFG3 = 0x00;
//			HDMI_REG_CFG4 = 0x00;
//			HDMI_REG_CFG6 &= (~B_EMP_EN);
//			udelay(5);
//		}
		
		HDMI_REG_CTRL |= B_SRST;
		udelay(5);

    }
	else
    /* Turn On HDMI */
/*    if( (hdmi_drv->control.hdmi_enable == true) &&
		(hdmi_drv->control.valid_avi_info == true) &&
		(hdmi_drv->control.hdmi_state == HDMI_STATE_READY) &&
		pord_Status)*/
    if( (hdmi_drv->control.hdmi_enable == true) &&
		(hdmi_drv->control.valid_avi_info == true) &&
		(hdmi_drv->control.hdmi_state == HDMI_STATE_READY))
    {
        HDMI_DEBUG("%s: Ready State -> Play State\n", __FUNCTION__);
        hdmi_drv->control.hdmi_state = HDMI_STATE_PLAY;
				
		hdmi_proc_avi_infoframe_update();
		udelay(5);
		hdmi_proc_transmit_infoframe(&hdmi_drv->video.infoframe);
		hdmi_proc_vsi_infoframe_update();
		udelay(5);
		hdmi_proc_transmit_infoframe(&hdmi_drv->vsi.infoframe);
		udelay(5);
		hdmi_proc_gcp_update();
		udelay(5);
		hdmi_proc_transmit_infoframe(&hdmi_drv->gcp.infoframe);
		udelay(5);
		HDMI_DEBUG("[hdmi_proc_state_update] hdmi_drv->control.valid_aud_info=%d\n", hdmi_drv->control.valid_aud_info);
        if (hdmi_drv->control.valid_aud_info)
        {
            hdmi_proc_audio_interface_config();
            hdmi_proc_audio_infoframe_update();
            hdmi_proc_audio_n_cts_update();
            hdmi_proc_transmit_infoframe(&hdmi_drv->audio.infoframe);
            udelay(5);
		}
		
        HDMI_REG_CTRL &= ~(B_SRST);
        udelay(5);
		
		// Turn on Audio
		(HDMI_REG_CFG1 & B_SPDIF) ?  (HDMI_REG_CFG5 &= ~(B_SPDIF_SWITCH)) : (HDMI_REG_CFG5 &= (~B_I2S_SWITCH));
        udelay(5);
        HDMI_REG_CTRL |= B_AUDIO_EN;
        udelay(5);
        if(hdmi_drv->control.hdmi_audio_enable)                /* coship UDI issue, independent HDMI audio onoff */
        HDMI_REG_CFG6 |= B_AUDIO_CAP_RST;
        udelay(5);
		
		// restore driving current before clear mute
//		hdmi_proc_set_phy_driving(hdmi_phy_driving);
//	   	udelay(5);
		
#ifdef CONFIG_HDCP_ENABLE_ALI
        if (hdmi_drv->control.hdcp_enable)
        {
            //hdmi_proc_set_avmute(true, 2);
            //msleep(20);
            hdmi_drv->hdcp.authenticated = false;       // Re-Authentication
//a            hdmi_proc_hdcp_auth_work();
        }
        else
            hdmi_proc_set_avmute(false, 4);
#else
        hdmi_proc_set_avmute(false, 4);
#endif

    }
	else
	{
		HDMI_DEBUG("%s: hdmi on/off undo State \n", __FUNCTION__);
		HDMI_DEBUG("pord_Status %d\n", HOT_PORD_STATE);
		HDMI_DEBUG("hotplug %d\n", HOT_PLUG_STATE);
		HDMI_DEBUG("hdmi_enable %d\n", hdmi_drv->control.hdmi_enable);
		HDMI_DEBUG("hdmi_state %d\n", hdmi_drv->control.hdmi_state);
		HDMI_DEBUG("valid_avi_info %d\n", hdmi_drv->control.valid_avi_info);
	}
}
#if 0
static BOOL hdmi_get_bootmedia_flag()
{
	#include <alidefinition/adf_boot.h>
	ADF_BOOT_BOARD_INFO *board_info = (ADF_BOOT_BOARD_INFO *)(ALI_VIRT(PRE_DEFINED_ADF_BOOT_START));		
	if(board_info->media_info.play_enable)
	{
		//printk("%s\n",__FUNCTION__);
		HDMI_DEBUG("BootMedia\n");
		return TRUE;
	}
	else
	{
		HDMI_DEBUG("No BootMedia\n");
		return FALSE;
	}
}
#endif
void hdmi_proc_driver_init(void)
{
	ADF_BOOT_BOARD_INFO *board_info = (ADF_BOOT_BOARD_INFO *)(ALI_VIRT(PRE_DEFINED_ADF_BOOT_START));
// Initial HDMI HW register
#ifdef CONFIG_ALI_CHIP_M3921  
	if(!(board_info->media_info.play_enable))
	{
		hdmi_proc_3d_phy_output();
	}
#endif
	// Initial HDMI Driver Data
	hdmi_drv->control.hdmi_enable 	= true;
	hdmi_drv->control.hdmi_audio_enable = true;
	hdmi_drv->control.hdmi_dvi_mode = HDMI_MODE;
	hdmi_drv->control.hot_plug_in	= HOT_PLUG_STATE;
	hdmi_drv->control.edid_ready 	= false;
	hdmi_drv->control.valid_avi_info = false;
	hdmi_drv->control.valid_aud_info = false;
	hdmi_drv->control.av_mute_state = true;
	hdmi_drv->control.hdmi_state 	= HDMI_STATE_IDLE;
	hdmi_drv->control.load_key_from_ce = false;

#ifdef CONFIG_HDMI_EDDC_ADAPTER_ID
	hdmi_drv->control.eddc_adapter_id = CONFIG_HDMI_EDDC_ADAPTER_ID;
#else
	 hdmi_drv->control.eddc_adapter_id = 5;
#endif // CONFIG_HDMI_EDDC_ADAPTER_ID

	// For BootMedia
	if(board_info->media_info.play_enable)
	{
		hdmi_drv->control.valid_avi_info = true;
		if(HOT_PLUG_STATE)
		{
			hdmi_drv->control.hdmi_state = HDMI_STATE_PLAY;
			hdmi_proc_get_parse_edid();
		}
	}

#ifdef CONFIG_HDCP_ENABLE_ALI
	hdmi_hdcp_config();
	hdmi_drv->hdmi_thread = kthread_create(hdcp_thread, NULL, "hdcp_thread");
	wake_up_process(hdmi_drv->hdmi_thread);
#else
    hdmi_drv->control.hdcp_enable = false;
    HDMI_DEBUG("%s: HDCP is Disable\n", __FUNCTION__);
#endif // CONFIG_HDCP_ENABLE_ALI

	if(hdmi_drv->control.hdcp_enable != true)
	{
		hdmi_drv->link_status = HDMI_STATUS_LINK_HDCP_IGNORED;
	}
	
#ifdef CONFIG_CEC_ENABLE_ALI
    hdmi_drv->control.cec_enable = true;
#else
    hdmi_drv->control.cec_enable = false;
#endif // CONFIG_CEC_ENABLE_ALI

    // Video InfoFrame
	hdmi_drv->video.infoframe.Type      = AVI_IFM_TYPE;
	hdmi_drv->video.infoframe.Version   = AVI_IFM_VER;
	hdmi_drv->video.infoframe.Length    = AVI_IFM_LEN;
	
    // Audio InfoFrame
	hdmi_drv->audio.infoframe.Type      = AUDIO_IFM_TYPE;
	hdmi_drv->audio.infoframe.Version   = AUDIO_IFM_VER;
	hdmi_drv->audio.infoframe.Length    = AUDIO_IFM_LEN;

	// SPD InfoFrame
	hdmi_drv->spd.infoframe.Type		= SPD_IFM_TYPE;
	hdmi_drv->spd.infoframe.Version		= SPD_IFM_VER;
	hdmi_drv->spd.infoframe.Length		= SPD_IFM_LEN;
	hdmi_drv->spd.vendor_name_len 		= 8;
	hdmi_drv->spd.product_desc_len		= 11;

	//GCP InfoFrame
	hdmi_drv->gcp.infoframe.Type	= GCP_TYPE;
	hdmi_drv->gcp.infoframe.Version	= GCP_VER;
	hdmi_drv->gcp.infoframe.Length	= GCP_LEN;

	memcpy(hdmi_drv->spd.vendor_name,  "ALi Corp", 8);
	memcpy(hdmi_drv->spd.product_desc, "ALi HD STB", 11);

	if(HOT_PLUG_STATE)
		hdmi_drv->link_status =  ((hdmi_drv->link_status | HDMI_STATUS_LINK) &(~HDMI_STATUS_UNLINK));
	else
		hdmi_drv->link_status = ((hdmi_drv->link_status | HDMI_STATUS_UNLINK) &(~HDMI_STATUS_LINK));

	init_waitqueue_head(&hdmi_drv->control.wait_queue);

	if(!(board_info->media_info.play_enable))
	{
		hdmi_proc_hardware_init();
	}
	if(0 == HDMI_REG_INT_MASK)
	{
		//If hardware init is not run, some interrupts should be always masked. wen liu for M3516
		HDMI_REG_INT_MASK = B_INT_NCTS_DONE | B_INT_IFM_ERR | B_INT_INF_DONE | B_INT_FIFO_U | B_INT_FIFO_O;
		HDMI_REG_CFG6     |= B_PORD_MASK;	
	}
}
