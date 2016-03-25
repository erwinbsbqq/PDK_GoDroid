

/***************************************************************************************************
*
* To understand what Alsa Drivers should be doing look at "Writing an Alsa Driver" by Takashi Iwai
* available in the Alsa doc section on the website		
* 
* A few notes to make things clearer. The UDA1341 is hooked up to Serial port 4 on the SA1100.
* We are using  SSP mode to talk to the UDA1341. The UDA1341 bit & wordselect clocks are generated
* by this UART. Unfortunately, the clock only runs if the transmit buffer has something in it.
* So, if we are just recording, we feed the transmit DMA stream a bunch of 0x0000 so that the
* transmit buffer is full and the clock keeps going. The zeroes come from FLUSH_BASE_PHYS which
* is a mem loc that always decodes to 0's w/ no off chip access.
*
* Some alsa terminology:
*	frame => num_channels * sample_size  e.g stereo 16 bit is 2 * 16 = 32 bytes
*	period => the least number of bytes that will generate an interrupt e.g. we have a 1024 byte
*             buffer and 4 periods in the runtime structure this means we'll get an int every 256
*             bytes or 4 times per buffer.
*             A number of the sizes are in frames rather than bytes, use frames_to_bytes and
*             bytes_to_frames to convert.  The easiest way to tell the units is to look at the
*             type i.e. runtime-> buffer_size is in frames and its type is snd_pcm_uframes_t
*             
*	Notes about the pointer fxn:
*	The pointer fxn needs to return the offset into the dma buffer in frames.
*	Interrupts must be blocked before calling the dma_get_pos fxn to avoid race with interrupts.
*
*	Notes about pause/resume
*	Implementing this would be complicated so it's skipped.  The problem case is:
*	A full duplex connection is going, then play is paused. At this point you need to start xmitting
*	0's to keep the record active which means you cant just freeze the dma and resume it later you'd
*	need to	save off the dma info, and restore it properly on a resume.  Yeach!
*
*	Notes about transfer methods:
*	The async write calls fail.  I probably need to implement something else to support them?
* 
***************************************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/slab.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include <linux/types.h>
#include <sound/control.h>
#include <sound/info.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>

#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/errno.h>

#include "../m39-pcm.h"
#include "ali-pcm.h"
#include <linux/dma-mapping.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

//#define CONFIG_HDMI_ALI
#undef CONFIG_HDMI_ALI
#ifdef CONFIG_HDMI_ALI
#include <linux/ali_hdmi.h>
#endif

#include <linux/sched.h>
#include <linux/time.h>

#include "../AUD_common.h"
#include "../AUD_reg.h"

#define S3901  0
#define C3901  1
static u8 g_s_ic_version;


#define LOG(fmt,arg...)		printk(fmt,##arg)
#define ERROR_LOG(fmt,arg...)	printk(fmt,##arg)

#undef pr_debug
#undef dev_dbg
#define pr_debug(fmt, ...) \
	printk(KERN_ERR, pr_fmt(fmt), ##__VA_ARGS__)
#define dev_dbg(dev, format, arg...)		\
	dev_printk(KERN_ERR,  dev , format , ## arg)
#define TXJPRINT KERN_ERR

/*******************************************************************
*                                                                                                                    *
*             Following code are for m3912 cpu common function                             *
*                                                                                                                    *
********************************************************************/
#if 0
void cpu_m3912_en_clk(u8 enable)
{
	// bit 1//0x00
	u8 temp;
	if(enable == 0)
		temp = 0x70;
	else
		temp =0x00;
	SND_SET_BYTE(R_AUD_CTRL, (SND_GET_BYTE(R_AUD_CTRL)&(~0x70))|temp);
}

void cpu_m3912_set_dma_bit_num(u8 bit_num)
{
	// bit 5
	u8 temp = 0;
	if(bit_num==16)
	{
		temp=0;
	}
	else if (bit_num==24)
	{
		temp=0x20;
	}
	else
	{
		while(1); //ASSERT(0);
	}					//0x15
	SND_SET_BYTE(R_DMA_CTRL, (SND_GET_BYTE(R_DMA_CTRL)&(~0x20))|temp);
}



static void cpu_m3912_i2s_channel_base(u32 base)
{
	SND_SET_DWORD(R_I2SO_DMA_BASE_ADDR, base & 0x0FFFFFFF);
}


static void cpu_m3912_i2s_channel_len(u16 ch_len)
{
	SND_SET_WORD(R_I2SO_DMA_LENG, ch_len);
}


void cpu_m3912_i2s_sc_thld(u16 threshold)
{ 	//0x10
	SND_SET_WORD(R_I2SO_SAM_COUNT_THD, threshold);
}

void cpu_m3912_en_i2s_int(u8 enable)
{
	// bit 0//0x01
	SND_SET_BYTE(R_AUD_INT, (SND_GET_BYTE(R_AUD_INT) & (~0x01)) | enable);
}

void cpu_m3912_en_resume_int(u8 enable)
{
	// bit 3//0x01
	SND_SET_BYTE(R_AUD_INT, (SND_GET_BYTE(R_AUD_INT) & (~0x08)) | (enable<<3));
}

void cpu_m3912_spo_channel_base(u32 base)
{	//0x60
	SND_SET_DWORD(R_SPO_DMA_BASE_ADDR, base & 0x0FFFFFFF);
}

void cpu_m3912_spo_channel_len(u16 len)
{	//0x66
	SND_SET_WORD(R_SPO_DMA_LENG, len);
}

void cpu_m3912_spo_sc_thld(u16 threshold)
{	//0x22
	SND_SET_WORD(R_SPO_SAM_COUNT_THD, threshold);
}

void cpu_m3912_spo_mem_div(u8 div)
{
	// bit 7:6  //0x20
	SND_SET_BYTE(R_SPO_CTRL, (SND_GET_BYTE(R_SPO_CTRL) & (~0xc0)) | (div << 6));
}

void cpu_m3912_spo_clk_sel(u8 clk)
{
	// bit 1:0
	SND_SET_BYTE(R_SPO_CTRL, (SND_GET_BYTE(R_SPO_CTRL) & (~0x03)) | clk);
}

void cpu_m3912_spo_pause_play(u8 enable)
{
	// bit 5				//0x21
	SND_SET_BYTE(R_IEC_CTRL, (SND_GET_BYTE(R_IEC_CTRL) & (~0x20)) | (enable << 5));
}

void cpu_m3912_set_spo_iec_ctrl(u8 val)
{
    SND_SET_BYTE(R_IEC_CTRL, val);
}

void cpu_m3912_set_spo_data_mode(u8 val)
{
    // bit 4	//0x21
	SND_SET_BYTE(R_IEC_CTRL, (SND_GET_BYTE(R_IEC_CTRL) & (~0x10)) | (val << 4));
}

void cpu_m3912_set_spo_source_sel(u8 val)
{
    // bit 2	//0x21
	SND_SET_BYTE(R_IEC_CTRL, (SND_GET_BYTE(R_IEC_CTRL) & (~0x04)) | (val << 2));
}

void cpu_m3912_en_spo_int(u8 enable)
{
    // bit 2    //0x01
	SND_SET_BYTE(R_AUD_INT, (SND_GET_BYTE(R_AUD_INT) & (~0x04)) | (enable << 2));
}

void cpu_m3912_en_av_sync_int(u8 enable)
{
    // bit 1    //0x01
	SND_SET_BYTE(R_AUD_INT, (SND_GET_BYTE(R_AUD_INT) & (~0x02)) | (enable << 1));
}

void cpu_m3912_set_pc(u16 val)
{
    //0x58
	SND_SET_WORD(R_IEC_PC, val);
}

void cpu_m3912_spo_long_mode(u8 enable)
{
	// bit 5			//0x20
	SND_SET_BYTE(R_SPO_CTRL, (SND_GET_BYTE(R_SPO_CTRL) & (~0x20)) | (enable << 5));
}

void cpu_m3912_spo_valid_data(u8 valid)
{
	// bit 4			//0x20
	SND_SET_BYTE(R_SPO_CTRL, (SND_GET_BYTE(R_SPO_CTRL) & (~0x10)) | (valid << 4));
}

void cpu_m3912_spo_data_reorder(u8 reorder)
{
	// bit 3
	SND_SET_BYTE(R_SPO_CTRL, (SND_GET_BYTE(R_SPO_CTRL) & (~0x08)) | (reorder << 3));
}

void cpu_m3912_spo_sample_rate(u8 sample_rate)
{
	// bit 27:24		//0x24
	SND_SET_BYTE(R_SPO_CS + 3, (SND_GET_BYTE(R_SPO_CS+3) & (~0x0f)) | sample_rate);
}

void cpu_m3912_spo_copyright(u8 copyright)
{
	// bit 2		//0x24
	SND_SET_BYTE(R_SPO_CS, (SND_GET_BYTE(R_SPO_CS) & (~0x04)) | (copyright << 2));
}

void cpu_m3912_spo_audio_content(u8 aud_content)
{
	// bit 1
	SND_SET_BYTE(R_SPO_CS, (SND_GET_BYTE(R_SPO_CS) & (~0x02)) | (aud_content << 1));
}

void cpu_m3912_spo_st_sel(u8 st_sel)
{
	// bit 2			//0x20
	SND_SET_BYTE(R_SPO_CTRL, (SND_GET_BYTE(R_SPO_CTRL)) ^ (st_sel << 2));
//	SET_BYTE(base_addr+R_SPO_CTRL, (GET_BYTE(base_addr+R_SPO_CTRL)&(~0x04))|(st_sel<<2));
}

void cpu_m3912_i2si_tx_dma_start(u8 enable)
{
    //0x10e bit 3
    SND_SET_WORD(R_I2SI_START_CTRL, (SND_GET_BYTE(R_I2SI_START_CTRL) & (~0x8)) | (enable << 3));
}

void cpu_m3912_i2si_rx_dma_start(u8 enable)
{
    //0x10e bit 2
    SND_SET_WORD(R_I2SI_START_CTRL, (SND_GET_BYTE(R_I2SI_START_CTRL) & (~0x4)) | (enable << 2));
}

void cpu_m3912_i2si_tx_en(u8 enable)
{
    //0x10e bit 1
    SND_SET_WORD(R_I2SI_START_CTRL, (SND_GET_BYTE(R_I2SI_START_CTRL) & (~0x2)) | (enable << 1));
}

void cpu_m3912_i2si_rx_en(u8 enable)
{
    //0x10e bit 0
    SND_SET_WORD(R_I2SI_START_CTRL, (SND_GET_BYTE(R_I2SI_START_CTRL) & (~0x1)) | (enable));
}

void cpu_m3912_i2si_tx_underrun_auto_resume(u8 enable)
{
    //0x10f bit1
    SND_SET_WORD(R_I2SI_INT_CTRL, (SND_GET_BYTE(R_I2SI_INT_CTRL) & (~0x2)) | (enable << 1));
}

void cpu_m3912_i2si_tx_underrun_fade_en(u8 enable)
{
    //0x10f bit0
    SND_SET_WORD(R_I2SI_INT_CTRL, (SND_GET_BYTE(R_I2SI_INT_CTRL) & (~0x1)) | (enable));
}

//#if C3901
/**
 * bit 0~1, bit 6~7
 * bit 2, 3, 4, 5
*/
void spo_iec_ctrl(u8 bit, u8 value)
{ 
    if ((bit >= 2 || bit <= 5)) {
        SND_SET_BYTE(R_IEC_CTRL, (SND_GET_BYTE(R_IEC_CTRL) & (~(1 << bit))) | value);
    } else if ((0 == bit) || (6 == bit)) {
        SND_SET_BYTE(R_IEC_CTRL, (SND_GET_BYTE(R_IEC_CTRL) & (~(3 << bit))) | value);
    } 
}
//#endif
static void cpu_m3912_set_audio_clock(long val)
{
	switch (val) {
	case 48000: 
		/* for 48k PLL clk select MCLK 18.432 */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000064));
		/* SPDIF_CLK_SEL the same as I2SO */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000003));
		break;
	case 44100: 
		/* for 44.1k PLL clkselect MCLK 16.9344 */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000064));
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) | (0x00000020));
		/* SPDIF_CLK_SEL the same as I2SO */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000003));
		break;
	/* M39 internal DAC not support 8k, 16k */
	//case 8000:
	//case 16000:
	case 32000:	
		/* for 8k, 16k, 32k PLL clkselect MCLK 24.576 */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000064));
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) | (0x00000044));
		/* SPDIF_CLK_SEL the same as I2SO */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000003));
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) | (0x00000002));
		break;
	}

}

static void cpu_m3912_set_samplerate( long rate)
{
	//printk(KERN_ERR "ALi m39_soc_set_samplerate\n") ; 	 

	/* wait for any frame to complete */
	//udelay(125);
    //rate = 48000;
  	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);  
	if (rate >= 48000) {
		rate = 48000;
	} else if (rate >= 44100) {
		rate = 44100;
	} else {	/* if (rate >= 32000) */
		rate = 32000;
	}
	/* M39 internal DAC not support 8k, 16k */ 

	/* BCLK_LRCLK_SEL[1:0] 
	00: 32 (L:16 / R: 16) 
	01: 48 (L:24 / R: 24) 
	10: 64 (L:32 / R: 32) */
	SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, \
		SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x18));
	SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, \
		SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x08));	// 24-bit:ON, 16-bit-bit:OFF
	

	/* I2S_MCLK_SEL output กา1 / กา2 / กา4 / กา8 select
	00: MCLK = I2S_SOURCE_CLK
	01: MCLK = I2S_SOURCE_CLK กา 2
	10: MCLK = I2S_SOURCE_CLK กา 4
	11: MCLK = I2S_SOURCE_CLK กา 8 */
	SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) & (~0x0C)); //MCLK/1

	/* I2SO_MCLK_DIV_SEL[2:0]
	000: 1x
	001: 2x
	010: 4x
	011: 8x -> Default
	100: 3x
	101: 6x
	110: 12x
	111: 24x */
	SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, \
		SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x07));
	SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, \
		SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x03));	//8x // 24-bit
	/*SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, \
		SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x06));	// 16-bit: 44.1 / 48 */

	/* Select the clock divisor */
	switch (rate) {
	/* M39 internal DAC not support 8k, 16k */ 
	case 32000:
//add by alex     
#if 1
        //48x: 24.576/2
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, \
		    SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x18));
	    SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, \
		    SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x08));	// 48(L:24/R:24)

        SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) & (~0x0C));
        SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) | (0x04));	//MCLK/2
            
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, \
		    SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x07));
	    SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, \
		    SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x03));	//8x 
#endif
		break;
	case 44100:
	case 48000:
		break;
	}

	cpu_m3912_set_audio_clock(rate);
	printk("ALi in function %s: samplerate = %ld\n", __FUNCTION__,rate);

}
static void cpu_m3912_capture_set_audio_clock(long val)
{
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	switch (val) {
	case 48000: 
		/* for 48k PLL clk select MCLK 24.576 */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000064));
        SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) | (0x00000004));
		/* SPDIF_CLK_SEL the same as I2SO */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000003));
        SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) | (0x00000002));
		break;
	case 44100: 
		/* for 44.1k PLL clkselect MCLK 22.5792 */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000064));
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) | (0x00000024));
		/* SPDIF_CLK_SEL the same as I2SO */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000003));
        SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) | (0x00000002));
		break;
	case 32000:	
		/* for 32k PLL clkselect MCLK 24.576 */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000064));
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) | (0x00000044));
		/* SPDIF_CLK_SEL the same as I2SO */
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) & (~0x00000003));
		SYS_IOW_DW(0xb0, SYS_IOR_DW(0xb0) | (0x00000002));
		break;
	}

}

static void cpu_m3912_capture_set_samplerate( long rate)
{
	//printk(KERN_ERR "ALi m39_soc_set_samplerate\n") ; 	 
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	/* wait for any frame to complete */
	//udelay(125);
    //rate = 48000;
    
	if (rate >= 48000) {
		rate = 48000;
	} else if (rate >= 44100) {
		rate = 44100;
	} else {	/* if (rate >= 32000) */
		rate = 32000;
	}
        
	/* Select the clock divisor */
	switch (rate) {
	/* M39 internal DAC not support 8k, 16k */ 
	case 32000: // 24.576/3
        /* BCLK_LRCLK_SEL[4:3], we select 00 here, 00: 32 (L:16 / R: 16) */
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x18));
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x00));

        /* I2SO_MCLK_DIV_SEL[2:0], we select 011 here, 011: 8x */ 
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x07));
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x03));
    
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x20));
        /* I2S_MCLK_SEL output[3:2], we select 01 here, 01: MCLK = I2S_SOURCE_CLK */
        SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) & (~0x0C));
        SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) | (0x00));

        cpu_m3912_spo_sample_rate(3);
		cpu_m3912_spo_clk_sel(0x3);
        cpu_m3912_spo_mem_div(0);

        //SND_SET_BYTE(R_IEC_CTRL, SND_GET_BYTE(R_IEC_CTRL) & (~0xc3));
        //SND_SET_BYTE(R_IEC_CTRL, SND_GET_BYTE(R_IEC_CTRL) | (0x42));        
        break;
	case 44100: // 22.5792/2
		/* BCLK_LRCLK_SEL[4:3], we select 00 here, 00: 32 (L:16 / R: 16) */
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x18));
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x00));

        /* I2SO_MCLK_DIV_SEL[2:0], we select 011 here, 011: 8x */ 
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x07));
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x03));
        
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x20));
        /* I2S_MCLK_SEL output[3:2], we select 01 here, 01: MCLK = I2S_SOURCE_CLK กา2 */
        SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) & (~0x0C));
        SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) | (0x04));

        cpu_m3912_spo_sample_rate(0);
		cpu_m3912_spo_clk_sel(0);
        cpu_m3912_spo_mem_div(2);

        //SND_SET_BYTE(R_IEC_CTRL, SND_GET_BYTE(R_IEC_CTRL) & (~0xc3));
        //SND_SET_BYTE(R_IEC_CTRL, SND_GET_BYTE(R_IEC_CTRL) | (0x41));   
        break;
	case 48000: // 24.576/2
	    /* BCLK_LRCLK_SEL[4:3], we select 00 here, 00: 32 (L:16 / R: 16) */
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x18));
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x00));

        /* I2SO_MCLK_DIV_SEL[2:0], we select 011 here, 011: 8x */ 
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x07));
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | (0x03));
        
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0x20));
        /* I2S_MCLK_SEL output[3:2], we select 01 here, 01: MCLK = I2S_SOURCE_CLK กา2 */
        SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) & (~0x0C));
        SND_SET_BYTE(R_AUD_CTRL, SND_GET_BYTE(R_AUD_CTRL) | (0x04));

        cpu_m3912_spo_sample_rate(2);
		cpu_m3912_spo_clk_sel(0);
        cpu_m3912_spo_mem_div(2);
        
        //SND_SET_BYTE(R_IEC_CTRL, SND_GET_BYTE(R_IEC_CTRL) & (~0xc3));
        //SND_SET_BYTE(R_IEC_CTRL, SND_GET_BYTE(R_IEC_CTRL) | (0x41)); 
        break;
	}

	cpu_m3912_capture_set_audio_clock(rate);

	printk("ALi in function %s: samplerate = %ld\n", __FUNCTION__,rate);

}
#endif

/*******************************************************************
*                                                                                                                    *
*             Following code are for m3912 cpu DAI operation function                     *
*                                                                                                                    *
********************************************************************/	
static int m3701_iis_set_dai_fmt(struct snd_soc_dai *cpu_dai,
		unsigned int fmt)
{
	int ret = 0;


	return ret;
}


static void cpu_m3912_audio_init(int mode)
{
    unsigned short vol = 0;
    static u32 flag = 0;
    s8 i = 7;
    u8 value = 0;
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
	printk(KERN_ERR "ALi m39_soc_audio_init\n");

/******************* For DB-M3911-01V02  ********************/
/******************* For DB-M3911-01V02  ********************/
#if 0

    /* Keep Volume Setting for restore after reset audio core */	
    vol = SND_GET_BYTE(R_TARGET_VOLUME);
    
    SYS_IOW_BYTE(0x80,  SYS_IOR_BYTE(0x80) | 0x20);//reset audio core
	udelay(3);
	SYS_IOW_BYTE(0x80,  SYS_IOR_BYTE(0x80) & (~0x20));//clear reset audio core
	
	/* SDATA_IN enable */
	SND_SET_BYTE(R_DISIR_16_23, SND_GET_BYTE(R_DISIR_16_23) | 0x02);

	/* enable ADAC_BIT_PDR & PDL */
	SND_SET_BYTE(R_DACR_0_7, 0x0);

    if (0 == flag) {
    	SYS_IOW_BYTE(0x81,  SYS_IOR_BYTE(0x81) | 0x08);//reset audio dec
	    udelay(3);
	    SYS_IOW_BYTE(0x81,  SYS_IOR_BYTE(0x81) & (~0x08));//clear reset audio dec 
	
        /* Disable DAC */
	    SND_SET_BYTE(R_DDCR_32_39, 0x0);
	    mdelay(20);
	    /* Enable DAC */
	    SND_SET_BYTE(R_DDCR_32_39, 0x80);
    
    	SND_SET_BYTE(R_DACR_8_15, 0x0); /* enable PD_DAC */
        flag = 1;
    }
    
	//QFP MUTE[2:0] single mode
	SND_SET_BYTE(R_DACR_8_15, (SND_GET_BYTE(R_DACR_8_15) & (~0x80)) | (1<<7));

    i = 7;
    value = 0;
    for (; i >= 0; i--) {     
        value = SND_GET_BYTE(R_DDCR_24_31) & 0xf8;
	    SND_SET_BYTE(R_DDCR_24_31, (value | i));
        mdelay(15);
    }

    if (0 == mode) {    
	    /* 24-bit WCLK,I2S mode,MSB,LRCLK form low */
        SND_SET_BYTE(R_DISIR_16_23, SND_GET_BYTE(R_DISIR_16_23) | 0xa0); 
	    /* 24-bit data length */
	    SND_SET_BYTE(R_DISIR_8_15, SND_GET_BYTE(R_DISIR_8_15) | 0x80);
    } else {
        /* 16-bit WCLK,I2S mode,MSB,LRCLK form low */
        SND_SET_BYTE(R_DISIR_16_23, SND_GET_BYTE(R_DISIR_16_23) | 0x20); 
	    /* 16-bit data length */
	    SND_SET_BYTE(R_DISIR_8_15, SND_GET_BYTE(R_DISIR_8_15) | 0x00); 
    }

	/* 0xf7[3] = 1: DMA enable */
	SND_SET_BYTE(R_DISIR_24_31, SND_GET_BYTE(R_DISIR_24_31) | 0x08);

	SND_SET_BYTE(R_DMA_CTRL, SND_GET_BYTE(R_DMA_CTRL) & (~0x11)); //I2SO SPO dma disable

	/* TO DO: Set Volume should depend on system config */
	//SND_SET_BYTE(R_TARGET_VOLUME, SND_GET_BYTE(R_TARGET_VOLUME) | 0x80);
	SND_SET_BYTE(R_TARGET_VOLUME, SND_GET_BYTE(R_TARGET_VOLUME) | vol);

	/* M39 Disable I2S Header */
	SND_SET_BYTE(R_DMA_CTRL, SND_GET_BYTE(R_DMA_CTRL) | 0x80);
#else
    /* Keep Volume Setting for restore after reset audio core */	
    vol =AUDREG_GetVolume();

    //reset audio core
    ResetAudCore();

    // disable I2SO SPO DMA,disable I2SO DMA
    StartSPODMA(FALSE);
    StartI2SODMA(FALSE);

    AUDREG_SetVolume( vol);

	/* M39 Disable I2S Header */
    SetI2SODMADataWithHeader(FALSE);
       // enable I2SO, SPO Interface
    EnableI2SOInterface(TRUE);
    EnableSPOInterface(TRUE);
#endif
}

static int m3701_iis_startup(struct snd_pcm_substream *substream,
			     struct snd_soc_dai *dai)
{

   	int stream_id = substream->pstr->stream;
       printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
#if 1
	if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) {
        /* reset snd for dma occurs error,set 24-bit for playback */
        cpu_m3912_audio_init(0);
                /* setup DMA controller */
                
	} else {
                /* reset snd for dma occurs error,set 16-bit for playback */
                cpu_m3912_audio_init(1);
	}

#endif

	return 0;
}

static int m3701_iis_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	int ret = 0;

	return ret;
}

static void m3701_iis_shutdown(struct snd_pcm_substream *substream,
			       struct snd_soc_dai *dai)
{
    printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
    // disable I2SO, SPO Interface
   EnableI2SOInterface(FALSE);
   EnableSPOInterface(FALSE);

}


static int m3701_iis_prepare(struct snd_pcm_substream *substream,struct snd_soc_dai *dai)
{
//	struct snd_soc_pcm_runtime *rtd = substream->private_data;
//	struct snd_soc_device *socdev = rtd->socdev;
//	struct snd_soc_codec *codec = socdev->card->codec;
    AUD_OutputParams_t Params_p;
    struct snd_pcm_runtime *runtime = substream->runtime;
    
    snd_pcm_format_t format = runtime->format;
    unsigned int channels = runtime->channels;
    unsigned int sample_bits = runtime->sample_bits;
    int sample_rate =runtime->rate;
    
    int stream_id = substream->pstr->stream;
    u32 *pcm_buff_base;    

    printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
    AudioPinmuxConfigurate();
#if 0    
    if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) {     
        /* set requested samplerate */
        cpu_m3912_set_samplerate(runtime->rate);
        printk("ALi in function %s: runtime->rate = %d, runtime->period_size = %ld\n", __FUNCTION__, 
        runtime->rate, runtime->period_size);
	
        SND_SET_BYTE(R_FADE_SPEED, 0x20);
        SND_SET_WORD(R_TIME_CHK_THRESHOLD, 100);        
        
        /* FORMAT_I2SO[1:0]
        00: for I2S format (16bit or 24bit )
        01: for Left-Justified format (16bit or 24bit) -> DAC default
        10: for Right-Justified 24bit format (Standard format)
        11: for Right-Justified 16bit format */
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0xC0));
        /* M39 Left-Justified test 
	    SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | 0x40);
        */
    
        cpu_m3912_en_i2s_int(1);	// enable I2SO INT
        cpu_m3912_en_resume_int(1);	// enable under resume int

        SND_SET_BYTE(R_BUF_UNDRUN_CTRL, (SND_GET_BYTE(R_BUF_UNDRUN_CTRL)&(~0x02))|(0x01<<1));
        SND_SET_BYTE(R_BUF_UNDRUN_CTRL, (SND_GET_BYTE(R_BUF_UNDRUN_CTRL)&(~0x04))|(0x01<<2));

        cpu_m3912_i2s_sc_thld(runtime->period_size);
        /* Trigger HW Read Idx running */

        /* M39 128 bits base */
        //SND_SET_WORD(R_LAST_VALID_INDEX, (runtime->period_size) / 2);
        //SND_SET_WORD(R_LAST_VALID_INDEX, (runtime->period_size) / 2);

        cpu_m3912_set_dma_bit_num(24);
        //cpu_m3912_set_dma_bit_num(16);

        /* write S/PDIF Audio interface registers */
        cpu_m3912_spo_pause_play(0);
        cpu_m3912_set_spo_iec_ctrl(0);
        //cpu_m3912_set_spo_iec_ctrl(spo_iec_ctrl);
	
        cpu_m3912_spo_sc_thld(runtime->period_size);
        cpu_m3912_spo_audio_content(0);	//((bPcm==0)?1:0) //pcm
        cpu_m3912_spo_valid_data(0);	//((bPcm==0)?1:0) //pcm
        //cpu_m3912_spo_valid_data(dev->base_addr,0);
        cpu_m3912_spo_data_reorder(0);
        cpu_m3912_spo_long_mode(1);      // 24 bits
        cpu_m3912_spo_copyright(1);
        cpu_m3912_spo_st_sel(1);     // right channel
    } else {  
        // set the pinmux for I2S in
        SYS_IOW_DW(0x8c, SYS_IOR_DW(0x8c) & (~(1 << 2)) | 0x4);
        
        /* set requested samplerate */
        cpu_m3912_capture_set_samplerate(runtime->rate);
        printk("ALi in function %s: runtime->rate = %d, runtime->period_size = %ld\n", __FUNCTION__, 
        runtime->rate, runtime->period_size);
	
        SND_SET_BYTE(R_FADE_SPEED, 0x20);
        SND_SET_WORD(R_TIME_CHK_THRESHOLD, 100);   
        SND_SET_WORD(R_LAST_VALID_INDEX, (runtime->period_size) / 2);

        /* FORMAT_I2SO[1:0]
        00: for I2S format (16bit or 24bit )
        01: for Left-Justified format (16bit or 24bit) -> DAC default
        10: for Right-Justified 24bit format (Standard format)
        11: for Right-Justified 16bit format */
        SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) & (~0xC0));
        /* M39 Left-Justified test 
	    SND_SET_BYTE(R_I2SO_CLK_INTF_CTRL, SND_GET_BYTE(R_I2SO_CLK_INTF_CTRL) | 0x40);
        */

        // set channel valume & I2S in valume
        SND_SET_WORD(R_I2SI_VOLUME_CTRL, 0x00ff);
        
        // MSB, LRCLK from low, I2S in serial data input enable, set channnel
        SND_SET_BYTE(R_I2SI_INTF_CTRL_0, 0x08 | ((runtime->channels - 1) << 1));
        // PIO data control Mode, Audio data Mode, Master Mode
        SND_SET_BYTE(R_I2SI_INTF_CTRL_1, 0x41);
        // 16-bit WCLK, I2S mode, 16-bit data length, I2S in DMA disable
        SND_SET_BYTE(R_I2SI_INTF_CTRL_2, 0x20);
        // I2S in:external device,enable volume,disable mix,disable I2S in interface 
        SND_SET_BYTE(R_I2SI_INTF_CTRL_3, 0x24);
    }

//#if C3901
    if (C3901 == g_s_ic_version) {
        #define STEREO_32          (1 << 7)
        #define MONO_32            (1 << 6)
        #define STEREO_24          (1 << 5)
        #define MONO_24            (1 << 4)
        #define STEREO_16          (1 << 3)
        #define MONO_16            (1 << 2)
        
        #define STEREO_8           (1 << 1)
        #define MONO_8             (1 << 0)
        #define ENABLE_UNSIGN_8     1
        #define DISABLE_UNSIGN_8    0
        
        //I2SO data in DRAM no header
    	SND_SET_BYTE(R_DMA_CTRL, (SND_GET_BYTE(R_DMA_CTRL) | (1 << 7)));

        SND_SET_BYTE(R_CONFIG_DATA_FMT, 0); //disable all format
        
        printk("ALi cpu_m3912_pcm_prepare: runtime->sample_bits = %d, runtime->channels = %d, runtime->format = %d \n", 
            runtime->sample_bits, runtime->channels, runtime->format);

        switch (sample_bits) {
            case 8:
                if (1 == channels) {
                    SND_SET_BYTE(R_CONFIG_DATA_FMT, STEREO_8); 
                } else if (2 == channels) {
                    SND_SET_BYTE(R_CONFIG_DATA_FMT, MONO_8);  
                }
                if (SNDRV_PCM_FMTBIT_S8 == format) {
                    SND_SET_BYTE(R_CONFIG_DATA_FMT_CTRL, DISABLE_UNSIGN_8);   
                } else if (SNDRV_PCM_FMTBIT_U8 == format){
                    SND_SET_BYTE(R_CONFIG_DATA_FMT_CTRL, ENABLE_UNSIGN_8);    
                }
                break;
            case 16:
                if (1 == channels) {
                    SND_SET_BYTE(R_CONFIG_DATA_FMT, MONO_16); 
                } else if (2 == channels) {
                    SND_SET_BYTE(R_CONFIG_DATA_FMT, STEREO_16); 
                }
                break;
            case 24:
                if (1 == channels) {
                    SND_SET_BYTE(R_CONFIG_DATA_FMT, MONO_24); 
                } else if (2 == channels) {
                    SND_SET_BYTE(R_CONFIG_DATA_FMT, STEREO_24);  
                }
                break;
            case 32:
                if (1 == channels) {
                    SND_SET_BYTE(R_CONFIG_DATA_FMT, MONO_32);
                } else if (2 == channels) {
                    SND_SET_BYTE(R_CONFIG_DATA_FMT, STEREO_32);
                }
                break;
            default:
                SND_SET_BYTE(R_CONFIG_DATA_FMT, STEREO_16); 
                break;
        }
    }
#else
    if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) {     
     sample_rate =runtime->rate;
    if(24000 == sample_rate || 16000 == sample_rate || 22050 == sample_rate)
   {
      sample_rate = sample_rate << 1;
   }
    if(12000 == sample_rate || 11025 == sample_rate || 8000 == sample_rate)
   {
      sample_rate = sample_rate<<2;
   }   
    
    Params_p.SampleRate = sample_rate;
    Params_p.SampleNum = runtime->period_size;
    Params_p.Format = I2S_FORMAT;
    Params_p.Precision = 16;
    Params_p.ChannelNum=channels;
    Params_p.Endian = 0;
    Params_p.SPOSrcMode = SPDIF_OUT_PCM;
    Params_p.SPOChlPCMType = SRC_DMLR;
    Params_p.DDPSPOSrcMode = SPDIF_OUT_PCM;
    Params_p.DDPSPOChlPCMType = SRC_DMLR;
    Params_p.BitStreamSetByUser = FALSE;
    Params_p.MPEGM8dbEnableFlag = FALSE;
    Params_p.DMADataHeaderFlag = FALSE;
    Params_p.SoftwareCfgClkFlag = FALSE;
    Params_p.SPODMAForPCMOutFlag = FALSE;
    Params_p.DDPSPODMAForPCMOutFlag = FALSE;
   
    AUDREG_ConfigInterface(AUD_SUB_OUT, &Params_p);

        printk("ALi in function %s: runtime->rate = %d, runtime->period_size = %ld\n", __FUNCTION__, 
        runtime->rate, runtime->period_size);
    
    } else {  

    sample_rate =runtime->rate;
    Params_p.SampleRate = sample_rate;
    Params_p.SampleNum = runtime->period_size;
    Params_p.Format = I2S_FORMAT;
    Params_p.Precision = 16;
    Params_p.ChannelNum=channels;
    Params_p.Endian = 0;
    Params_p.SPOSrcMode = SPDIF_OUT_PCM;
    Params_p.SPOChlPCMType = SRC_DMLR;
    Params_p.DDPSPOSrcMode = SPDIF_OUT_PCM;
    Params_p.DDPSPOChlPCMType = SRC_DMLR;
    Params_p.BitStreamSetByUser = FALSE;
    Params_p.MPEGM8dbEnableFlag = FALSE;
    Params_p.DMADataHeaderFlag = FALSE;
    Params_p.SoftwareCfgClkFlag = FALSE;
    Params_p.SPODMAForPCMOutFlag = FALSE;
    Params_p.DDPSPODMAForPCMOutFlag = FALSE;
   
   AUDREG_ConfigInterface(AUD_SUB_IN, &Params_p);

        printk("ALi in function %s: runtime->rate = %d, runtime->period_size = %ld\n", __FUNCTION__, 
        runtime->rate, runtime->period_size);

    }


#endif

//       u32 *pcm_buff_base;
       /******************************************************/
       /****      following code will trigger dma                          *********/
       printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
//txj add, tony suggested that we should use full address
    pcm_buff_base = (u32 *)(((u32)runtime->dma_addr + 15) & 0xfffffff0);
    if (stream_id == SNDRV_PCM_STREAM_PLAYBACK) { 

      AUDREG_ConfigDMA(AUD_SUB_OUT,pcm_buff_base, runtime->dma_bytes/16);
	printk(TXJPRINT "!!!infunction %s, pcm_buff_base %x !!!\n",__FUNCTION__,(u32)pcm_buff_base);
	printk(TXJPRINT "!!!infunction %s, dma_bytes %d !!!\n",__FUNCTION__, runtime->dma_bytes);

     AUDREG_ConfigDMA(AUD_SUB_SPDIFOUT,pcm_buff_base, runtime->dma_bytes/16);

    } 
     

	return 0;
}

static int m3701_iis_hw_free(struct snd_pcm_substream *substream,struct snd_soc_dai *dai)
{
	printk(TXJPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);

    // disable I2SO, SPO Interface
   EnableI2SOInterface(FALSE);
   EnableSPOInterface(FALSE);
   return 0;	
}

static struct snd_soc_dai_ops m3701_iis_dai_ops = {
	.startup	= m3701_iis_startup,
	.shutdown	= m3701_iis_shutdown,
	.hw_params	= m3701_iis_hw_params,
	.set_fmt	= m3701_iis_set_dai_fmt,
	.prepare =m3701_iis_prepare,
	.hw_free = m3701_iis_hw_free,
};


/*******************************************************************
*                                                                                                                    *
*             Following code are for m3912 cpu DAI register function                        *
*                                                                                                                    *
********************************************************************/	
static int m3701_iis_probe(struct platform_device *pdev,
			   struct snd_soc_dai *dai)
{
//	pr_debug("%s enter\n", __func__);

	return 0;
}

static void m3701_iis_remove(struct platform_device *pdev,
			struct snd_soc_dai *dai)
{
//	pr_debug("%s enter\n", __func__);

}

#ifdef CONFIG_PM
static int m3701_iis_suspend(struct snd_soc_dai *dai)
{

//	pr_debug("%s : sport %d\n", __func__, dai->id);

	return 0;
}

static int m3701_iis_resume(struct snd_soc_dai *dai)
{
	int ret=0;

//	pr_debug("%s : sport %d\n", __func__, dai->id);

	return ret;
}

#else
#define m3701_iis_suspend	NULL
#define m3701_iis_resume	NULL
#endif




static const struct snd_soc_dai_driver m3701_iis = {
	.name		= "m3701_iis",
	.id		= 0,
	.probe = m3701_iis_probe,
	.remove = m3701_iis_remove,
	.suspend = m3701_iis_suspend,
	.resume = m3701_iis_resume,
	.playback = {
		.channels_min	= 2,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_32000 |\
				   SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 |\
				   SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S8|\
                                SNDRV_PCM_FMTBIT_S24_LE| \
				   SNDRV_PCM_FMTBIT_S16_LE| \
				   SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture = {
		.channels_min	= 2,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_32000 |\
				   SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 |\
				   SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_U8 |SNDRV_PCM_FMTBIT_S8|SNDRV_PCM_FMTBIT_S24_LE|SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops = &m3701_iis_dai_ops,
};

static __devinit int m3912_iis_dev_probe(struct platform_device *pdev)
{
   	printk(KERN_ERR "in function %s line %d\n", __FUNCTION__, __LINE__);

	return snd_soc_register_dai(&pdev->dev, &m3701_iis);
}

static __devexit int m3912_iis_dev_remove(struct platform_device *pdev)
{
   	printk(KERN_ERR "in function %s line %d\n", __FUNCTION__, __LINE__);

	snd_soc_unregister_dai(&pdev->dev);
	return 0;
}

static struct platform_driver m3912_iis_driver = {
	.probe  = m3912_iis_dev_probe,
	.remove = __devexit_p(m3912_iis_dev_remove),
	.driver = {
		.name = "m3701_i2s",
		.owner = THIS_MODULE,
	},
};

module_platform_driver(m3912_iis_driver);

MODULE_AUTHOR("shenzhen os team base on xavier chang ");
MODULE_DESCRIPTION("ALi PCM Asoc driver");
MODULE_LICENSE("GPL");

/* }}} */

/*
 * Local variables:
 * indent-tabs-mode: t
 * End:
 */
