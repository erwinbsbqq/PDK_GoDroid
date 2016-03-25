////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2013 ALI, Inc.  All rights reserved.
//
// ALI, Inc.
// ALI IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
// COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
// ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
// STANDARD, ALI IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
// IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
// FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
// ALI EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
// THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
// ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
// FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Id: board_config.c,v 1.0 2013/07/13 17:02:53 $
//
//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/
/**
*
* @file board_config.c
*
* The XTemac driver. Functions in this file are the minimum required functions
* for this driver. See xtemac.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a joy  2013/07/13 First release
* </pre>
******************************************************************************/

#include <linux/memblock.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <asm/pgtable.h>

#include <mach/ali-s3921.h>
#include <ali_reg.h>

#include "tve_config.h"
#include "board_config.h"

#include <alidefinition/adf_boot.h>
#define DEFINE_BORAD_VARIABLES

#include <ali_board_config.h>

#ifdef CONFIG_ALI_KEYPAD
#include <linux/gpio_keys.h>

extern struct gpio_keys_platform_data keypad_data;
#endif

extern unsigned int arm_get_early_mem_size(void);

#ifdef SUPPORT_AFD_WSS_OUTPUT
unsigned long g_support_afd_wss = 1;
#else
unsigned long g_support_afd_wss = 0;
#endif	

#ifdef SUPPORT_AFD_SCALE
unsigned long   g_support_afd_scale = 1;
#else
unsigned long   g_support_afd_scale = 0;
#endif



/* HW buffer map descriptor.
*/
static struct ali_hwbuf_desc ali_hwbuf_desc_m3921_1G[] =
{
    {
	  .name      = "Ali priv mem1",
        .phy_start = __MM_ALI_PRIVATE_MEM_START1,
        .phy_end   = __MM_ALI_PRIVATE_MEM_END1,
    },
    {
	  .name      = "Ali priv mem2",
        .phy_start = __MM_ALI_PRIVATE_MEM_START2,
        .phy_end   = __MM_ALI_PRIVATE_MEM_END2,
    }, 
    {
	  .name      = "Ali priv mem3",
        .phy_start = __MM_ALI_PRIVATE_MEM_START3_1G,
        .phy_end   = __MM_ALI_PRIVATE_MEM_END3_1G,
    },     
};

static struct ali_hwbuf_desc ali_hwbuf_desc_m3921_512M[] =
{
    {
	  .name      = "Ali priv mem1",
        .phy_start = __MM_ALI_PRIVATE_MEM_START1,
        .phy_end   = __MM_ALI_PRIVATE_MEM_END1,
    },
    {
	  .name      = "Ali priv mem2",
        .phy_start = __MM_ALI_PRIVATE_MEM_START2,
        .phy_end   = __MM_ALI_PRIVATE_MEM_END2,
    }, 
    {
	  .name      = "Ali priv mem3",
        .phy_start = __MM_ALI_PRIVATE_MEM_START3_512M,
        .phy_end   = __MM_ALI_PRIVATE_MEM_END3_512M,
    },    
};


/* Init HW buffer addresses & feature configs.
*/
static void set_global_setting(void)
{
	/*<==================== BOARD IDENTIFICATION START ====================>*/
	/* hardware board definition start */
	/* please review the below board type when you
	want to porting new board. if it belongs to a exist
	type, just allocate a new version for this new board.

		board type description :
			type 				value
			C3701_DEMO			0x00003000
			C3921_DEMO			0x00004000

		board version description :
			version		value
			invalid            0
			ver1   		1
			...
			vern			n	
	*/

	/*
	define 3921 board type as flow:

	0x00004001 : BGA445
	0x00004002 : QFP256
	*/	
	if(__REG8ALI(0x18000001) & 0x01)	
		__G_ALI_BOARD_TYPE = 0x00004001;
	else
		__G_ALI_BOARD_TYPE = 0x00004002;

	/*<==================== BOARD IDENTIFICATION END ====================>*/


	/*<==================== DRVIER MEMORY MAP VARIABLES START ====================>*/
	/* Anchor postions.
	*/
	/* MCAPI
	*/  
	__G_ALI_MM_MCAPI_MEM_SIZE = (__MM_MCAPI_MEM_END -__MM_MCAPI_MEM_START);
	__G_ALI_MM_MCAPI_MEM_START_ADDR = (unsigned long)(phys_to_virt)(__MM_MCAPI_MEM_START);

    /*Alsa Capture
	*/
	__G_ALI_MM_ALSA_CAPTURE_MEM_SIZE = (__MM_ALSA_CAP_MEM_END -__MM_ALSA_CAP_MEM_START);
	__G_ALI_MM_ALSA_CAPTURE_START_ADDR = (unsigned long)(phys_to_virt)(__MM_ALSA_CAP_MEM_START);

    /*Alsa I2SIRX_I2SO_MIX
	*/
	__G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_MEM_SIZE = (__MM_ALSA_I2SIRX_I2SO_MIX_MEM_END -__MM_ALSA_I2SIRX_I2SO_MIX_MEM_START);
	__G_ALI_MM_ALSA_I2SIRX_I2SO_MIX_START_ADDR = (unsigned long)(phys_to_virt)(__MM_ALSA_I2SIRX_I2SO_MIX_MEM_START);	

	/* Boot LOGO
	*/
	__G_ALI_MM_BOOT_COMMAND_SIZE = (__MM_BOOT_COMMAND_DATA_END - __MM_BOOT_COMMAND_DATA_START);
	__G_ALI_MM_BOOT_COMMAND_START_ADDR = (unsigned long)(phys_to_virt)(__MM_BOOT_COMMAND_DATA_START);	

	/* SEE PRIVATE MEM
	*/
	__G_ALI_MM_PRIVATE_AREA_SIZE = (__MM_SEE_PRIVATE_END - __MM_SEE_PRIVATE_START);
	__G_ALI_MM_PRIVATE_AREA_START_ADDR = (unsigned long)(phys_to_virt)(__MM_SEE_PRIVATE_START);	

	/* SEE MAIN SHARED MEM
	*/
	__G_ALI_MM_SHARED_MEM_SIZE = (__MM_SEE_MAIN_SHARED_MEM_END - __MM_SEE_MAIN_SHARED_MEM_START);		
	__G_ALI_MM_SHARED_MEM_START_ADDR = (unsigned long)(phys_to_virt)(__MM_SEE_MAIN_SHARED_MEM_START);
	
	/* TSG
	*/
	__G_ALI_MM_TSG_BUF_SIZE = (__MM_TSG_MEM_END - __MM_TSG_MEM_START);
	__G_ALI_MM_TSG_BUF_START_ADDR = (unsigned long)(phys_to_virt)(__MM_TSG_MEM_START);	

	/* DMX
	*/
	__G_ALI_MM_DMX_MEM_SIZE = (__MM_DMX_MEM_END - __MM_DMX_MEM_START);
	__G_ALI_MM_DMX_MEM_START_ADDR = (unsigned long)(phys_to_virt)(__MM_DMX_MEM_START);

	/* Image decoder
	*/
	__G_ALI_MM_IMAGE_DECODER_MEM_SIZE = (__MM_IMAGE_DECODER_MEM_END - __MM_IMAGE_DECODER_MEM_START);
	__G_ALI_MM_IMAGE_DECODER_MEM_START_ADDR = (unsigned long)(phys_to_virt)(__MM_IMAGE_DECODER_MEM_START);	

	/* APE for Media Player
	*/
	__G_ALI_MM_APE_MEM_SIZE = (__MM_MP_MEM_END - __MM_MP_MEM_START);
	__G_ALI_MM_APE_MEM_START_ADDR = (unsigned long)(phys_to_virt)(__MM_MP_MEM_START);	

	/* Raw Data Buffer for Media Player
	*/
	__G_ALI_MM_VDEC_RAW_DATA_SIZE = (__MM_VDEC_RAW_DATA_BUF_END - __MM_VDEC_RAW_DATA_BUF_START);
	__G_ALI_MM_VDEC_RAW_DATA_START_ADDR = (unsigned long)(phys_to_virt)(__MM_VDEC_RAW_DATA_BUF_START);

	/* VBV buffer
	*/
	__G_ALI_MM_VDEC_VBV_SIZE = (__MM_VBV_BUF_END - __MM_VBV_BUF_START);
	__G_ALI_MM_VDEC_VBV_START_ADDR = (unsigned long)(phys_to_virt)(__MM_VBV_BUF_START);	

	/* STILL FRAME
	*/
	__G_ALI_MM_STILL_FRAME_SIZE = (__MM_STILL_FRAME_END - __MM_STILL_FRAME_START);
	__G_ALI_MM_STILL_FRAME_START_ADDR = (unsigned long)(phys_to_virt)(__MM_STILL_FRAME_START);	

	/* VCAP
	*/
	__G_ALI_MM_VCAP_FB_SIZE = (__MM_VCAP_FB_END - __MM_VCAP_FB_START);
	__G_ALI_MM_VCAP_FB_START_ADDR = (unsigned long)(phys_to_virt)(__MM_VCAP_FB_START);	

	/* VIDEO
	*/
	__G_ALI_MM_VIDEO_SIZE = (__MM_VIDEO_BUF_END - __MM_VIDEO_BUF_START);
	__G_ALI_MM_VIDEO_START_ADDR = (unsigned long)(phys_to_virt)(__MM_VIDEO_BUF_START);

	/* NIM J83B
	*/
	__G_ALI_MM_NIM_J83B_MEM_SIZE = (__MM_NIM_J83B_MEM_END - __MM_NIM_J83B_MEM_START);
	__G_ALI_MM_NIM_J83B_MEM_START_ADDR = (unsigned long)(phys_to_virt)(__MM_NIM_J83B_MEM_START);

	/* Audio Engine
	*/
	__G_ALI_MM_AUDIO_DEC_MEM_SIZE = __MM_AUDIO_ENGINE_END - __MM_AUDIO_ENGINE_START;
	__G_ALI_MM_AUDIO_DEC_MEM_START_ADDR = (unsigned long)(phys_to_virt)__MM_AUDIO_ENGINE_START;

	/* FRAME BUFFER
	*/ 	
	//Android modified to 1280x720
	g_fb0_max_width = 1280;// FB0_VIRTUAL_WIDTH);
	g_fb0_max_height = 1440;// FB0_VIRTUAL_HEIGHT);
	g_fb0_width = 1280;// FB0_WIDTH);
	g_fb0_height = 720;//(FB0_HEIGHT);
	g_fb0_pitch = 1280;//(FB0_PITCH);
	g_fb0_bpp = (FB0_BPP);	
	
	g_fb2_max_width = (FB2_VIRTUAL_WIDTH);
	g_fb2_max_height = (FB2_VIRTUAL_HEIGHT);
	g_fb2_width = (FB2_WIDTH);
	g_fb2_height = (FB2_HEIGHT);
	g_fb2_pitch = (FB2_PITCH);
	g_fb2_bpp = (FB2_BPP);

	__G_ALI_MM_FB0_SIZE = (FB0_MEM_SIZE);
	__G_ALI_MM_FB2_SIZE = (FB2_MEM_SIZE);
	if(arm_get_early_mem_size() == 0x20000000)
	{
		__G_ALI_MM_FB0_START_ADDR = (unsigned long)(phys_to_virt)(__MM_FB_MEM_START_512M);
		__G_ALI_MM_FB2_START_ADDR = (unsigned long)(phys_to_virt)__MM_FB2_MEM_START_512M;
	}
	else
	{
		__G_ALI_MM_FB0_START_ADDR = (unsigned long)(phys_to_virt)(__MM_FB_MEM_START_1G);
		__G_ALI_MM_FB2_START_ADDR = (unsigned long)(phys_to_virt)__MM_FB2_MEM_START_1G;
	}

	__G_ALI_MM_GE_CMD_SIZE = 0;
	__G_ALI_MM_GE_CMD_START_ADDR = 0;

	__G_ALI_MM_FB0_CMAP_SIZE = (FB0_CMAP_MEM_SIZE);
	__G_ALI_MM_FB0_CMAP_START_ADDR = (unsigned long)(phys_to_virt)__MM_FB0_CMAP_START;
	__G_ALI_MM_FB2_CMAP_SIZE = (FB2_CMAP_MEM_SIZE);
	__G_ALI_MM_FB2_CMAP_START_ADDR = (unsigned long)(phys_to_virt)__MM_FB2_CMAP_START;
	
	/* Mali
	*/
	if(arm_get_early_mem_size() == 0x20000000)
	{	
		__G_ALI_MM_MALI_DEDICATED_MEM_SIZE = __MM_MALI_MEM_END_512M - __MM_MALI_MEM_START_512M;
		__G_ALI_MM_MALI_DEDICATED_MEM_START_ADDR = (unsigned long)(phys_to_virt)__MM_MALI_MEM_START_512M;
	}
	else
	{
		__G_ALI_MM_MALI_DEDICATED_MEM_SIZE = __MM_MALI_MEM_END_1G - __MM_MALI_MEM_START_1G;
		__G_ALI_MM_MALI_DEDICATED_MEM_START_ADDR = (unsigned long)(phys_to_virt)__MM_MALI_MEM_START_1G;
	}
	

	/* Mali UMP
	*/
	if(arm_get_early_mem_size() == 0x20000000)
	{	
		__G_ALI_MM_MALI_UMP_MEM_SIZE = __MM_MALI_UMP_MEM_END_512M - __MM_MALI_UMP_MEM_START_512M;
		__G_ALI_MM_MALI_UMP_MEM_START_ADDR = (unsigned long)(phys_to_virt)__MM_MALI_UMP_MEM_START_512M;
	}
	else
	{
		__G_ALI_MM_MALI_UMP_MEM_SIZE = __MM_MALI_UMP_MEM_END_1G - __MM_MALI_UMP_MEM_START_1G;
		__G_ALI_MM_MALI_UMP_MEM_START_ADDR = (unsigned long)(phys_to_virt)__MM_MALI_UMP_MEM_START_1G;
	}


	/* DSC 
	*/ 	
	__G_ALI_MM_DSC_MEM_SIZE = __MM_DSC_MEM_END - __MM_DSC_MEM_START;
	__G_ALI_MM_DSC_MEM_START_ADDR = (unsigned long)(phys_to_virt)__MM_DSC_MEM_START;

    __G_ALI_MM_DECA_MEM_SIZE = __MM_DECA_DATA_END - __MM_DECA_DATA_START;
	__G_ALI_MM_DECA_MEM_START_ADDR = (unsigned long)(phys_to_virt)__MM_DECA_DATA_START;

    /* VENC added_by_VicZhang_on_20131111. */
    __G_ALI_MM_VENC_MEM_SIZE = __MM_VENC_DATA_END - __MM_VENC_DATA_START;
	__G_ALI_MM_VENC_MEM_START_ADDR = (unsigned long)(phys_to_virt)__MM_VENC_DATA_START;

	/* Subtitle & Teletext. 
	* Added by Joy Chu, Date:2014.04.15
	*/
	__G_ALI_MM_SUBT_SIZE = __MM_SUBT_BUF_END - __MM_SUBT_BUF_START;
	__G_ALI_MM_SUBT_START_ADDR = (unsigned long)(phys_to_virt)__MM_SUBT_BUF_START;

	__G_ALI_MM_TTX_SIZE = __MM_TTX_BUF_END - __MM_TTX_BUF_START;
	__G_ALI_MM_TTX_START_ADDR = (unsigned long)(phys_to_virt)__MM_TTX_BUF_START;

	__G_ALI_MM_TTX_PARAM_BUF_SIZE = __MM_TTX_PARAM_BUF_END - __MM_TTX_PARAM_BUF_START;
	__G_ALI_MM_TTX_PARAM_BUF_ADDR = (unsigned long)(phys_to_virt)__MM_TTX_PARAM_BUF_START;

#if 0   
	printk("BOOT_CMD start %x size %x\n", __G_ALI_MM_BOOT_COMMAND_START_ADDR, __G_ALI_MM_BOOT_COMMAND_SIZE);
	printk("PRIV_AREA start %x size %x\n", __G_ALI_MM_PRIVATE_AREA_START_ADDR, __G_ALI_MM_PRIVATE_AREA_SIZE);
	printk("RPC start %x size %x\n", __G_ALI_MM_MCAPI_MEM_START_ADDR, __G_ALI_MM_MCAPI_MEM_SIZE);
	printk("SHARED_MEM start %x size %x\n", __G_ALI_MM_SHARED_MEM_START_ADDR, __G_ALI_MM_SHARED_MEM_SIZE);
	printk("TSG start %x size %x\n", __G_ALI_MM_TSG_BUF_START_ADDR, __G_ALI_MM_TSG_BUF_SIZE);
	printk("DMX start %x size %x\n", __G_ALI_MM_DMX_MEM_START_ADDR, __G_ALI_MM_DMX_MEM_SIZE);
	printk("IMAGE start %x size %x\n", __G_ALI_MM_IMAGE_DECODER_MEM_START_ADDR, __G_ALI_MM_IMAGE_DECODER_MEM_SIZE);
	printk("APE start %x size %x\n", __G_ALI_MM_APE_MEM_START_ADDR, __G_ALI_MM_APE_MEM_SIZE);  
	printk("VDEC_RAW %x size %x\n", __G_ALI_MM_VDEC_RAW_DATA_START_ADDR, __G_ALI_MM_VDEC_RAW_DATA_SIZE);   
	printk("VDEC_VBV %x size %x\n", __G_ALI_MM_VDEC_VBV_START_ADDR, __G_ALI_MM_VDEC_VBV_SIZE);	
	printk("VDEC_STILL %x size %x\n", __G_ALI_MM_STILL_FRAME_START_ADDR, __G_ALI_MM_STILL_FRAME_SIZE);	
	printk("VCAP %x size %x\n", __G_ALI_MM_VCAP_FB_START_ADDR, __G_ALI_MM_VCAP_FB_SIZE);
	printk("VDEC_FB %x size %x\n", __G_ALI_MM_VIDEO_START_ADDR, __G_ALI_MM_VIDEO_SIZE);
	printk("NIM_J83B start %x size %x\n", __G_ALI_MM_NIM_J83B_MEM_START_ADDR, __G_ALI_MM_NIM_J83B_MEM_SIZE);	
	printk("AUDIO_CPU start %x size %x\n", __G_ALI_MM_AUDIO_DEC_MEM_START_ADDR, __G_ALI_MM_AUDIO_DEC_MEM_SIZE);
	printk("FB0 start %x size %x\n", __G_ALI_MM_FB0_START_ADDR, __G_ALI_MM_FB0_SIZE);
	printk("FB2 start %x size %x\n", __G_ALI_MM_FB2_START_ADDR, __G_ALI_MM_FB2_SIZE);
	printk("FB0_CMAP start %x size %x\n", __G_ALI_MM_FB0_CMAP_START_ADDR, __G_ALI_MM_FB0_CMAP_SIZE);
	printk("FB2_CMAP start %x size %x\n", __G_ALI_MM_FB2_CMAP_START_ADDR, __G_ALI_MM_FB2_CMAP_SIZE);
	printk("MALI_DEDICATED start %x size %x\n", __G_ALI_MM_MALI_DEDICATED_MEM_START_ADDR, __G_ALI_MM_MALI_DEDICATED_MEM_SIZE);
	printk("MALI_DEDICATED start %x size %x\n", __G_ALI_MM_MALI_DEDICATED_MEM_START_ADDR, __G_ALI_MM_MALI_DEDICATED_MEM_SIZE);
	printk("MALI_UMP start %x size %x\n", __G_ALI_MM_MALI_UMP_MEM_START_ADDR, __G_ALI_MM_MALI_UMP_MEM_SIZE);
	printk("DSC start %x size %x\n", __G_ALI_MM_DSC_MEM_START_ADDR, __G_ALI_MM_DSC_MEM_SIZE);
	printk("AUDIO_DEC start %x size %x\n", __G_ALI_MM_DECA_MEM_START_ADDR, __G_ALI_MM_DECA_MEM_SIZE);
	printk("VIDEO_ENC start %x size %x\n", __G_ALI_MM_VENC_MEM_START_ADDR, __G_ALI_MM_VENC_MEM_SIZE);
	printk("SUBT start %x size %x\n", __G_ALI_MM_SUBT_START_ADDR, __G_ALI_MM_SUBT_SIZE);
	printk("TTX start %x size %x\n", __G_ALI_MM_TTX_START_ADDR, __G_ALI_MM_TTX_SIZE);
	printk("TTX_PARAM start %x size %x\n", __G_ALI_MM_TTX_PARAM_BUF_ADDR, __G_ALI_MM_TTX_PARAM_BUF_SIZE);
#endif

	printk("ali platform private memory bottom1 0x%08x top1 0x%08x\n", (int)__MM_ALI_PRIVATE_MEM_START1, (int)__MM_ALI_PRIVATE_MEM_END1);
	printk("ali platform private memory bottom2 0x%08x top1 0x%08x\n", (int)__MM_ALI_PRIVATE_MEM_START2, (int)__MM_ALI_PRIVATE_MEM_END2);
	if(arm_get_early_mem_size() == 0x20000000)
	{	
		printk("ali platform private memory bottom3 0x%08x top2 0x%08x\n", (int)__MM_ALI_PRIVATE_MEM_START3_512M, (int)__MM_ALI_PRIVATE_MEM_END3_512M);
	}
	else
	{
		printk("ali platform private memory bottom3 0x%08x top2 0x%08x\n", (int)__MM_ALI_PRIVATE_MEM_START3_1G, (int)__MM_ALI_PRIVATE_MEM_END3_1G);
	}

	/*<==================== DRVIER MEMORY MAP VARIABLES END ====================>*/

	
	/*<==================== DRVIER FEATURE CONFIG START ====================>*/

	/* Standard FB or not
	*/
	g_support_standard_fb = 1;
	
	/* DAC.
	*/
	/* default scart output 
	0 : CVBS
	1 : RGB
	2 : SVIDEO
	3 : YUV
	*/
	g_tve_default_scart_output = 3;

	/* TV ENCODER.
	*/
	//Android modified
	g_tve_hd_default_tv_mode = LINE_720_30;
	g_tve_sd_default_tv_mode = NTSC;

	/* whether use the CVBS output. the invalid value is -1.
	*/
	g_tve_dac_use_cvbs_type = CVBS_1;

	/* whether use the SVIDEO output. the invalid value is -1. 
	*/
	g_tve_dac_use_svideo_type = -1;

	/* whether use the RGB output. the invalid value is -1.
	*/
	g_tve_dac_use_rgb_type = RGB_1;

	/* whether use the YUV output. the invalie value is -1.
	*/
	g_tve_dac_use_yuv_type = YUV_1;

	/* CVBS dac definition.
	*/
	g_tve_dac_cvbs = DAC3;

	/* SVIDEO dac definition.
	*/
	g_tve_dac_svideo_y = 0;
	g_tve_dac_svideo_c = 0;

	/* RGB dac definition.
	*/
	g_tve_dac_rgb_r = DAC0;
	g_tve_dac_rgb_g = DAC2;
	g_tve_dac_rgb_b = DAC1;

	/* YUV dac definition.
	*/
	g_tve_dac_yuv_y = DAC2;
	g_tve_dac_yuv_u = DAC1;
	g_tve_dac_yuv_v = DAC0;

	/* feature list.
	*/
	g_support_isdbt_cc = 0;

	/* HDMI.
	*/
	g_hdmi_hdcp_enable = 0;

	/* NAND Flash pinmux.
	*/
	g_nand_pm1_pin_sel = ALI_PINMUX_CTRL_BIT3;
	g_nand_pm2_strapin_sel_en = ALI_PINMUX_CTRL_BIT31;

	/*<==================== DRVIER FEATURE CONFIG END ====================>*/   

	return;
}



static void __init alissi_setting(void)
{
	unsigned int value;
	unsigned int mask;

	/***********Config the PinMUX for ASSI1_2, ASSI2, ASSI3_1, ASSI4**************/
	value = __REG32ALI(0x18000088);
	//mask = (1<<4)|(1<<21)|(1<<22);  //clear SPI,ALISSI2 and ALISSI3_1
	mask = (1<<4);    
	/*(alissi4) mutual exclusion*/
	#if 0 
	mask |= (1<<7)|(1<<23);
	#endif	
	value = value&(~mask);
	mask = (1<<21)|(1<<22);
	value = value|mask;
	__REG32ALI(0x18000088) = value;

	value = __REG32ALI(0x1800008C);
	mask = (1<<6)|(1<<9)|(1<<14)|(1<<16)|(1<<17)|(1<<19);  //clear QAM_SPI,ALISSI4,ALISSI2,QAM_SSI
	/*(alissi4) mutual exclusion*/
	#if 0
	mask |= (1<<15)|(1<<21)|(1<<25);
	#endif	
	value = value&(~mask);

	/* Bit 22(alissi4) mutual exclusion with SD pin.
	*/
	/*(alissi4) mutual exclusion*/
	#if 0
	mask = (1<<8)|(1<<22);   //ALISSI1_2
	#else
	mask = (1<<8);   //ALISSI1_2
	#endif

	value = value|mask;
	__REG32ALI(0x1800008C) = value;

	/****************Cfg the GPIO **********************/
	/*GPIO for ASSI1_1*/
	value = __REG32ALI(0x18000430);
	mask = (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9);  
	value = value&(~mask);

	/* DVB-T: tiger board */
	/* GPIO: 0 & 11 */
	mask = (1<<0) | (1<<11);
	value = value|mask;

	__REG32ALI(0x18000430) = value;

	/* DVB-T: tiger board */
	/* GPIO: 0 & 11 */
	value = __REG32ALI(0x18000058);
	mask = (1<<0) | (1<<11);
	value = value|mask;		//1: output
	__REG32ALI(0x18000058) = value;

	/* DVB-T: tiger board */
	/* GPIO: 0 & 11 */
	value = __REG32ALI(0x18000054);
	mask = (1<<0);
	value = value&(~mask);	//0: off
	mask = (1<<11);
	value = value|mask;		//1: work
	__REG32ALI(0x18000054) = value;

	/*GPIO for ASSI2*/
	value = __REG32ALI(0x18000430);
	mask = (1<<13)|(1<<14)|(1<<15);  
	value = value&(~mask);
	__REG32ALI(0x18000430) = value;

	value = __REG32ALI(0x18000438);
	mask = (1<<18)|(1<<19)|(1<<20);  
	value = value&(~mask);
	__REG32ALI(0x18000438) = value;

	/* GPIO for ASSI3_1 */
	value = __REG32ALI(0x1800043c);
	mask = (1<<19)|(1<<20)|(1<<21)|(1<<22)|(1<<23)|(1<<26);  
	value = value&(~mask);
	__REG32ALI(0x1800043c) = value;

	/* Bit 22(alissi4) mutual exclusion with SD pin.
	*/
	/*(alissi4) mutual exclusion*/
	#if 0
	/* GPIO for ASSI4 */
	value = __REG32ALI(0x18000430);
	mask = (1<<0)|(1<<3)|(1<<10)|(1<11)|(1<12);  
	value = value&(~mask);
	__REG32ALI(0x18000430) = value;

	value = __REG32ALI(0x18000438);
	mask = (1<<23);  
	value = value&(~mask);
	__REG32ALI(0x18000438) = value;	
	#endif
}



/*	
		��һ��CA��
    	ca1_1:reg 0x18000088[0]
	ca1_2:reg 0x1800008c[23]

		�ڶ���CA��
	ca2_1:reg 0x1800008c[4]
	ca2_2:reg 0x1800008c[1]
*/
void ali_smc_pinmux_setting(void)
{	
	unsigned int value = 0;
	unsigned int mask = 0;
	
	if(__G_ALI_BOARD_TYPE == 0x00004001)	/* bga445 */
	{		
			/* ca2_1 */
		__REG32ALI(0x1800008c) |= __REG32ALI(0x1800008c) | 0x10; 	
		value = __REG32ALI(0x18000438);
		mask = (1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7);  
		value = value & (~mask);
		__REG32ALI(0x18000438) = value;					
	}
	else if(__G_ALI_BOARD_TYPE == 0x00004002)	/* qfp256 */
	{				
		/* ca2 */
		__REG32ALI(0x1800008c) |= __REG32ALI(0x1800008c) | 0x2; 	
		/*	ca2_2
			XSC2_CLK		gpio[40]
			XSC2_POWENJ	gpio[41]
			XSC2_DATA		gpio[42]
			XSC2_RST		gpio[43]
			XSC2_PRESJ		gpio[44]
		*/
		value = __REG32ALI(0x18000434);
		mask = (1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12);  
		value = value & (~mask);
		__REG32ALI(0x18000434) = value;				
	}	
}

void nim_setting(void)
{
  	//AGC
	__REG32ALI(0x18000088) |= 1<<7;// XIF_AGC_PDM_SEL
   	
 	//__REG32ALI(0x1800043c) != (1<<25)|(1<<26);
	//__REG32ALI(0x1800043c) &= (0<<25)|(0<<26);
	 	
	if(__G_ALI_BOARD_TYPE == 0x00004001)	/* bga445 */
	{		
		__REG32ALI(0x18000088) |= 1<<8;
		__REG32ALI(0x18000088) |= 0<<24;
		__REG32ALI(0x18000088) |= 0<<26;
	}
	else if(__G_ALI_BOARD_TYPE == 0x00004002)	/* qfp256 */
	{
		__REG32ALI(0x18000430) |= 1<<0; //for fullnim i2c
    __REG32ALI(0x18000430) |= 1<<3;
   
	}	
}

#ifdef CONFIG_ALI_KEYPAD
void keypad_gpio_setting(void)
{
	int i, num, gpio;
	struct gpio_keys_button *p = NULL;
	struct gpio_keys_platform_data *data = &keypad_data;

	num = data->nbuttons;
	for (i = 0; i < num; i++) {
		p = &(data->buttons[i]);
		gpio = p->gpio;
		if (gpio < 32) {
			__REG32ALI(0x18000430) |= 1<<gpio;		/*enable*/
			__REG32ALI(0x18000058) &= ~(1<<gpio);	/*input or output */
			__REG32ALI(0x18000044) |= 1<<gpio;		/*interrupt enable */
			__REG32ALI(0x18000048) &= ~(1<<gpio);	/*interrupt rising */
			__REG32ALI(0x1800004C) |= 1<<gpio;		/*interrupt falling */
		} else if (gpio < 64) {
			__REG32ALI(0x18000434) |= 1<<(gpio - 32);		
			__REG32ALI(0x180000D8) &= ~(1<<(gpio - 32));	
			__REG32ALI(0x180000C4) |= 1<<(gpio - 32);
			__REG32ALI(0x180000C8) &= ~(1<<(gpio - 32));
			__REG32ALI(0x180000CC) |= 1<<(gpio - 32);
		} else if (gpio < 96) {
			__REG32ALI(0x18000438) |= 1<<(gpio - 64);		
			__REG32ALI(0x180000F8) &= ~(1<<(gpio - 64));	
			__REG32ALI(0x180000E4) |= 1<<(gpio - 64);
			__REG32ALI(0x180000E8) &= ~(1<<(gpio - 64));
			__REG32ALI(0x180000EC) |= 1<<(gpio - 64);
		} else if (gpio < 128) {
			__REG32ALI(0x1800043C) |= 1<<(gpio - 96);
			__REG32ALI(0x18000358) &= ~(1<<(gpio - 96));
			__REG32ALI(0x18000344) |= 1<<(gpio - 96);
			__REG32ALI(0x18000348) &= ~(1<<(gpio - 96));
			__REG32ALI(0x1800034C) |= 1<<(gpio - 96);
		} 
	}
}
#endif

/* board configutation.
*/
#define TIGER_BOARD
#ifndef TIGER_BOARD
static void __init board_setting_bga445(void)
{
	/* TODO: Set pinmux here.
	*/
	 // disable all GPIO
    __REG32ALI(0x18000430) = 0; 
    __REG32ALI(0x18000434) = 0; 
    __REG32ALI(0x18000438) = 0; 
    __REG32ALI(0x1800043c) = 0; 
	
   
	__REG32ALI(0x18000088) |= 1<<9;// i2c1	
	__REG32ALI(0x18000088) |= 1<<12;	/* 20150707: i2c0 for DVB-C */
	
	nim_setting();

    /* Setting DVFS I2C device id number : 4, using Linux kernel i2c interface*/
    g_dvfs_i2c_device_id = 4;
	/* TODO: Set GPIO here.
	*/
	
	alissi_setting();

	/* SND GPIO.
	*/
	g_snd_mute_gpio = 80;
    g_snd_chip_type = 1;
    
    /* SND pimmux.
	*/
	__REG32ALI(0x18000088) |= 1<<13; // XSPDIF_SEL1
	
	/* for C3921 BGA GPIO 73 control usb port 0 5V*/
	g_usb_p0_host_5v_gpio = 73;
	
	/* for C3921 BGA GPIO 84 control usb port device port 5V detect*/
    g_usb_device_5v_detect_gpio = 84;
    g_enet_link_gpio = 121;

    /* 20150707: DVB-C need disable gpio 91&92 */
//    g_enet_speed_gpio = 92;
    g_enet_speed_gpio = -1;

	return;
}
#else
static void __init board_setting_bga445(void)
{
	unsigned int value = 0;
	unsigned int mask = 0;

	/* TODO: Set pinmux here.
	*/
	 // disable all GPIO
    __REG32ALI(0x18000430) = 0; 
    __REG32ALI(0x18000434) = 0; 
    __REG32ALI(0x18000438) = 0; 
    __REG32ALI(0x1800043c) = 0; 
	
   
	__REG32ALI(0x18000088) |= 1<<9;// i2c1	

#if 0
	nim_setting();
    alissi_setting();
#else
  /*if uart1  is enabled  ssi will be disabled*/
	value = __REG32ALI(0x18000088);
	mask =( 1<<6);  
	value |= mask;
	__REG32ALI(0x18000088) = value;

      value = __REG32ALI(0x18000438);
	mask = (1<<23);  
	value = value & (~mask);
	__REG32ALI(0x18000438) = value;

	value = __REG32ALI(0x1800043c);
	mask = (1<<22);  
	value = value & (~mask);
	__REG32ALI(0x1800043c) = value;
#endif

    /* Setting DVFS I2C device id number : 4, using Linux kernel i2c interface*/
    g_dvfs_i2c_device_id = 4;
	/* TODO: Set GPIO here.
	*/
	value = __REG32ALI(0x18000430);
	mask = (1<<5)|(1<<6)|(1<<12)|(1<<13)|(1<<14)|(1<<15);  
	value |= mask;
	__REG32ALI(0x18000430) = value;

	value = __REG32ALI(0x18000438);
	mask = (1<<12)|(1<<14)|(1<<15)|(1<<16)|(1<<17)|(1<<18)|(1<<19)|(1<<20);  
	value |= mask;
	__REG32ALI(0x18000438) = value;

	value = __REG32ALI(0x1800043c);
	mask = (1<<17);  
	value |= mask;
	__REG32ALI(0x1800043c) = value;

    
    /* SND pimmux.
	*/
	__REG32ALI(0x18000088) |= 1<<13; // XSPDIF_SEL1

	/* for C3921 BGA GPIO 73 control usb port 0 5V*/
	g_usb_p0_host_5v_gpio = 73;


    g_enet_link_gpio = 5;
    g_enet_speed_gpio = 6;
    
    
    //mask = (1<<(79-64)) | (1<<(81-64)) | (1<<(80-64)) | (1<<(78-64)) | \
    //        (1<<(76-64)) | (1<<(82-64)) | (1<<(83-64)) | (1<< (84-64));
    //Turn Tiger Board LED3 on
    
    mask = 1<<(81-64);

    value = __REG32ALI(0x18000438);
    value |= mask;
    __REG32ALI(0x18000438) = value;

    //Set gpio output
    value = __REG32ALI(0x180000f8);
    value |= mask;
    __REG32ALI(0x180000f8) = value;

    //Set defalt value 0
    mask = ~mask;
    value = __REG32ALI(0x180000f4);
    value &= mask;
    __REG32ALI(0x180000f4) = value;    

#if 0
    /*GPIO HEADER AS Raspberry pi2*/
    //GPIO 44,47,55,58,60,62 AS Raspberry pin 15,36,32,18,22,16(GPIO22,GPIO16,GPIO12,GPIO24,GPIO25,GPIO23)
    value = __REG32ALI(0x18000434);
    mask= (1<<(44-32))|(1<<(47-32))|(1<<(55-32))|(1<<(58-32))|(1<<(60-32))|(1<<(62-32));
    value |= mask;
   __REG32ALI(0x18000434)=value;
//GPIO 64,66,69,70,71,72 AS Raspberry pin 13,11,40,12,35,38(GPIO27,GPIO17,GPIO21,GPIO18,GPIO19,GPIO20)
    value = __REG32ALI(0x18000438);
    mask= (1<<(64-64))|(1<<(66-64))|(1<<(69-64))|(1<<(70-64))|(1<<(71-64))|(1<<(72-64));
    value |= mask;
    __REG32ALI(0x18000438)=value;
   

 /*GPIO 7,8,87,118,119 is now for SSI, else  it will be Raspberry pin 31,29,8,10,27(GPIO6,GPIO5,ID_SD,GPIO14,GPIO15)*/
   /*GPIO 0,11 is now for i2c in nim,else it will be Raspberry pin 37,33(GPIO26,GPIO13)*/	
    value = __REG32ALI(0x18000430);
    mask= (1<<7)|(1<<8)|(1<<11);
    value |= mask;
   __REG32ALI(0x18000430)=value;

   value = __REG32ALI(0x18000438);
    mask= (1<<(87-64));
    value |= mask;
   __REG32ALI(0x18000438)=value;

    value = __REG32ALI(0x1800043c);
    mask= (1<<(118-96))|(1<<(119-96));
    value |= mask;
   __REG32ALI(0x1800043c)=value;
#endif

	/* gpio input */
#ifdef CONFIG_ALI_KEYPAD
 	/* gpio is set in ali-s3921.c and configure here */
	keypad_gpio_setting();
#endif
    /*FOR TIGER BOARD 1.1  START*/
	//wifi reset(OUTPUT) ; user key2 open(INPUT)
	value = __REG32ALI(0x18000430);
	mask= (1<<4)|(1<<28);
	value |= mask;
	__REG32ALI(0x18000430)=value;

	value = __REG32ALI(0x18000058);
	mask= (1<<4);
	value |= mask;
	__REG32ALI(0x18000058)=value;

	value = __REG32ALI(0x18000054);
	mask= (1<<4);
	value |= mask;
	__REG32ALI(0x18000054)=value;

	//USB5V_1A (OUTPUT) ; LPWR_DET(INPUT)
	value = __REG32ALI(0x18000434);
	mask= (1<<(36-32))|(1<<(39-32));
	value |= mask;
	__REG32ALI(0x18000434)=value;

	value = __REG32ALI(0x180000D8);
	mask= (1<<(36-32));
	value |= mask;
	__REG32ALI(0x180000D8)=value;

	value = __REG32ALI(0x180000D4);
	mask= (1<<(36-32));
	value |= mask;
	__REG32ALI(0x180000D4)=value;

	
	//USB5V_ON(OUTPUT)   ; PWR_LED(OUTPUT)  ; MUTE(OUTPUT) 
	value = __REG32ALI(0x1800043c);
	mask= (1<<(116-96))|(1<<(117-96))|(1<<(122-96));
	value |= mask;
	__REG32ALI(0x1800043c)=value;

	
	value = __REG32ALI(0x18000358);
	mask= (1<<(116-96))|(1<<(117-96))|(1<<(122-96));
	value |= mask;
	__REG32ALI(0x18000358)=value;

	value = __REG32ALI(0x18000354);
	mask= (1<<(116-96))|(1<<(122-96));
	value |= mask;
	mask=(1<<(117-96));
	value =value&(~mask);
	__REG32ALI(0x18000354)=value;
	/*FOR TIGER BOARD 1.1  END*/
	return;
}

#endif

static void __init board_setting_gfp256(void)
{
	unsigned int value = 0;
	unsigned int mask = 0;

	
	/* TODO: Set pinmux here.
	*/
	 // disable all GPIO
    __REG32ALI(0x18000430) = 0; 
    __REG32ALI(0x18000434) = 0; 
    __REG32ALI(0x18000438) = 0; 
    __REG32ALI(0x1800043c) = 0; 
	
	//__REG32ALI(0x18000088) |= 1<<7;// XIF_AGC_PDM_SEL
	__REG32ALI(0x18000088) |= 1<<9;// i2c1
	
	nim_setting();

    /* Setting DVFS I2C device id number : I2C_TYPE_SCB1,   useing the scb i2c */
    g_dvfs_i2c_device_id = 1;

	/* TODO: Set GPIO here.
	*/
	
	/* SND GPIO.
	*/
	g_snd_mute_gpio = 92;
    g_snd_chip_type = 0;

    /* SND pimmux.
	*/
	__REG32ALI(0x18000088) |= 1<<13; // XSPDIF_SEL1
	
#if 1
	value = __REG32ALI(0x1800008C);
	//mask = (1<<6)|(1<<9)|(1<<14)|(1<<16)|(1<<17)|(1<<19);  //clear QAM_SPI,ALISSI4,ALISSI2,QAM_SSI
	//value = value&(~mask);
	mask = (1<<8);   //ALISSI1_2
	value = value|mask;
	__REG32ALI(0x1800008C) = value;
    
	value = __REG32ALI(0x18000430);
	mask = (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9);  
	value = value&(~mask);
	__REG32ALI(0x18000430) = value;
#endif


    g_enet_link_gpio = 6;
    g_enet_speed_gpio = 5;
	return;
}

struct ali_hwbuf_desc* __init ali_get_privbuf_desc
(
    int *hwbuf_desc_cnt
)
{
	struct ali_hwbuf_desc *desc = NULL;
	
	if(arm_get_early_mem_size() == 0x20000000)
	{
		printk("enable 512M solution\n");
		desc = ali_hwbuf_desc_m3921_512M;
		*hwbuf_desc_cnt = ARRAY_SIZE(ali_hwbuf_desc_m3921_512M);		
	}
	else
	{
		printk("enable 1G solution\n");
		desc = ali_hwbuf_desc_m3921_1G;
		*hwbuf_desc_cnt = ARRAY_SIZE(ali_hwbuf_desc_m3921_1G);		
	}
	
	return(desc);
}

static void __init board_setting_lcd(void)
{
	unsigned int value = 0;
	unsigned int mask = 0;

    value = __REG32ALI(0x18000088);
    printk("board_setting_lcd, value: 0x%08x\n", value);
    
	__REG32ALI(0x18000088) |= 1<<18;

    value = __REG32ALI(0x18000088);
    printk("board_setting_lcd, value: 0x%08x\n", value);

    value = __REG32ALI(0x18000078);
    printk("board_setting_lcd, value: 0x%08x, zero\n", value);

    //__REG32ALI(0x18000078) |= 1<<23;

    __REG32ALI(0x18006600) = 0x00000000;
		
	return;
}

void customize_board_setting(void)
{
	unsigned long ul_reg_val = 0;
	
	set_global_setting();
	if(__G_ALI_BOARD_TYPE == 0x00004001)
	{
		board_setting_bga445();
	}
	else if(__G_ALI_BOARD_TYPE == 0x00004002)
	{
		board_setting_gfp256();
	}

	#if 0
		ali_smc_pinmux_setting();
	#endif
	
   
	/* enable gpio 74, 75 for hdmi gpio i2c */    
	ul_reg_val = __REG32ALI(0x18000438);
	ul_reg_val |= (1<<(74-64));
	ul_reg_val |= (1<<(75-64));
	__REG32ALI(0x18000438) = ul_reg_val;

	/*panel gpio i2c set*/
	#ifdef CONFIG_ALI_PAN_CH455	
	g_gpio_i2c_panel_sda = I2C_GPIO_SDA_PIN;
	g_gpio_i2c_panel_scl = I2C_GPIO_SCL_PIN;
	g_gpio_i2c_panel_id = I2C_DEVICE_ID;
	#endif


#ifdef CONFIG_TOUCHSCREEN_FT5X0X

       board_setting_lcd();
       g_lcd_backlight_gpio = 37;
       g_lcd_reset_gpio = 42;

       ul_reg_val = __REG32ALI(0x18000434);
       ul_reg_val |= (1<<(35-32));     //   pwm
       ul_reg_val |= (1<<(37-32));    //   disp
       ul_reg_val |= (1<<(42-32));     //  reset
       ul_reg_val |= (1<<(43-32));     //  interrupt
       __REG32ALI(0x18000434) = ul_reg_val;
#endif
}
