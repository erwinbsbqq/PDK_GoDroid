

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
#if 0
#include <asm/mach-ali/m39xx.h>
#include <asm/mach-ali/typedef.h>
#include <asm/mach-ali/m36_gpio.h>
#endif
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
#include <linux/timer.h>
//#include "../codecs/snd-codec-m3912.h"
#define LOG(fmt,arg...)		printk(fmt,##arg)
#define ERROR_LOG(fmt,arg...)	printk(fmt,##arg)

static struct snd_soc_dai_link asoc_dai_link_machine_m3912[] = {
	{
		.name = "asoc_dai_link_machine_m3912",
		.stream_name = "asoc dai link machine m3912",
		.cpu_dai_name ="m3701_i2s",
		.platform_name	= "asoc_platform_m39xx",
		.codec_name		= "codec_s3921",
		.codec_dai_name = "codec_s3921_HiFi",
	},
	
};

static struct snd_soc_card sound_card_m3912 = {
	.name = "sound_card_m3912",
	.dai_link = asoc_dai_link_machine_m3912,
	.num_links = ARRAY_SIZE(asoc_dai_link_machine_m3912),
};


static struct platform_device *snd_device_m3912;

static int __init asoc_machine_init_m3912(void)
{
	int ret;

	printk("!!!in function %s, line %d\n",__FUNCTION__,__LINE__);

	snd_device_m3912 = platform_device_alloc("soc-audio", -1);
	if (!snd_device_m3912)
		return -ENOMEM;

	platform_set_drvdata(snd_device_m3912, &sound_card_m3912);
	
	ret = platform_device_add(snd_device_m3912);

	if (ret)
		platform_device_put(snd_device_m3912);

	return ret;
}

static void __exit asoc_machine_exit_m3912(void)
{
       printk("!!!in function %s, line %d\n",__FUNCTION__,__LINE__);
	platform_device_unregister(snd_device_m3912);
}

module_init(asoc_machine_init_m3912);
module_exit(asoc_machine_exit_m3912);



MODULE_AUTHOR("shenzhen os team base on xavier chang ");
MODULE_DESCRIPTION("ALi PCM Asoc driver");
MODULE_LICENSE("GPL");

/* }}} */

/*
 * Local variables:
 * indent-tabs-mode: t
 * End:
 */
