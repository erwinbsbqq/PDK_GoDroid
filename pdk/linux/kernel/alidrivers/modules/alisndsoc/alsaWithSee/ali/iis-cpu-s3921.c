

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
#include <linux/version.h>
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


#include <linux/dma-mapping.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>


#include <linux/sched.h>
#include <linux/time.h>

#define ALSA_WITH_SEE          1

#define ALSAPRINT KERN_DEBUG

/*******************************************************************
*                                                                                                                    *
*             Following code are for s3921 cpu common function                             *
*                                                                                                                    *
********************************************************************/

/*******************************************************************
*                                                                                                                    *
*             Following code are for m3912 cpu DAI operation function                     *
*                                                                                                                    *
********************************************************************/	
static int iis_cpu_s3921_set_dai_fmt(struct snd_soc_dai *cpu_dai,
		unsigned int fmt)
{
	int ret = 0;


	return ret;
}


#if 0
static void iis_cpu_s3921_audio_init(int mode)
{
	//unsigned short vol = 0;
	//static u32 flag = 0;
	//s8 i = 7;
	//u8 value = 0;
	printk(ALSAPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
	printk(ALSAPRINT "ALi m39_soc_audio_init\n");

}
#endif

static int iis_cpu_s3921_startup(struct snd_pcm_substream *substream,
			     struct snd_soc_dai *dai)
{

	//int stream_id = substream->pstr->stream;
	printk(ALSAPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
	return 0;
}

static int iis_cpu_s3921_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	int ret = 0;

	return ret;
}

static void iis_cpu_s3921_shutdown(struct snd_pcm_substream *substream,
			       struct snd_soc_dai *dai)
{
	printk(ALSAPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
}


static int iis_cpu_s3921_prepare(struct snd_pcm_substream *substream,struct snd_soc_dai *dai)
{
	//struct snd_pcm_runtime *runtime = substream->runtime;
	//snd_pcm_format_t format = runtime->format;
	//unsigned int channels = runtime->channels;
	//unsigned int sample_bits = runtime->sample_bits;
	//int sample_rate =runtime->rate;

	//int stream_id = substream->pstr->stream;
	//u32 *pcm_buff_base;    

	printk(ALSAPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);    
	return 0;
}

static int iis_cpu_s3921_hw_free(struct snd_pcm_substream *substream,struct snd_soc_dai *dai)
{
	printk(ALSAPRINT "!!!infunction %s, line %d !!!\n",__FUNCTION__,__LINE__);
	return 0;	
}

static struct snd_soc_dai_ops iis_cpu_s3921_dai_ops = {
	.startup	= iis_cpu_s3921_startup,
	.shutdown	= iis_cpu_s3921_shutdown,
	.hw_params	= iis_cpu_s3921_hw_params,
	.set_fmt	= iis_cpu_s3921_set_dai_fmt,
	.prepare =iis_cpu_s3921_prepare,
	.hw_free = iis_cpu_s3921_hw_free,
};


/*******************************************************************
*                                                                                                                    *
*             Following code are for m3912 cpu DAI register function                        *
*                                                                                                                    *
********************************************************************/
#if 0
static int iis_cpu_s3921_probe(struct platform_device *pdev,
			   struct snd_soc_dai *dai)
{
	//	pr_debug("%s enter\n", __func__);

	return 0;
}
#else
static int iis_cpu_s3921_probe(struct snd_soc_dai *dai)
{
	//	pr_debug("%s enter\n", __func__);

	return 0;
}
#endif

#if 0
static void iis_cpu_s3921_remove(struct platform_device *pdev,
			struct snd_soc_dai *dai)
{
	//	pr_debug("%s enter\n", __func__);

}
#else
static int iis_cpu_s3921_remove(struct snd_soc_dai *dai)
{
	//	pr_debug("%s enter\n", __func__);

	return 0;
}
#endif

#ifdef CONFIG_PM
static int iis_cpu_s3921_suspend(struct snd_soc_dai *dai)
{

	//	pr_debug("%s : sport %d\n", __func__, dai->id);

	return 0;
}

static int iis_cpu_s3921_resume(struct snd_soc_dai *dai)
{
	int ret=0;

	//	pr_debug("%s : sport %d\n", __func__, dai->id);

	return ret;
}

#else
#define iis_cpu_s3921_suspend	NULL
#define iis_cpu_s3921_resume	NULL
#endif




//static const struct snd_soc_dai_driver iis_cpu_s3921_dai = {
static struct snd_soc_dai_driver iis_cpu_s3921_dai = {
	.name		= "iis_cpu_s3921_dai",
	.id		= 0,
	.probe = iis_cpu_s3921_probe,
	.remove = iis_cpu_s3921_remove,
	.suspend = iis_cpu_s3921_suspend,
	.resume = iis_cpu_s3921_resume,
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
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_32000 |\
				   SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 |\
				   SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_U8 |SNDRV_PCM_FMTBIT_S8|SNDRV_PCM_FMTBIT_S24_LE|SNDRV_PCM_FMTBIT_S16_LE|SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops = &iis_cpu_s3921_dai_ops,
};
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static const struct snd_soc_component_driver s3921_iis_component = {
	.name		= "s3921_iis",
};
#endif
static __devinit int iis_cpu_s3921_dev_probe(struct platform_device *pdev)
{
	printk(ALSAPRINT "in function %s line %d\n", __FUNCTION__, __LINE__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	return snd_soc_register_component(&pdev->dev, &s3921_iis_component, &iis_cpu_s3921_dai, 1);
#else
	return snd_soc_register_dai(&pdev->dev, &iis_cpu_s3921_dai);
#endif	
}

static __devexit int iis_cpu_s3921_dev_remove(struct platform_device *pdev)
{
	printk(ALSAPRINT "in function %s line %d\n", __FUNCTION__, __LINE__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))	
	snd_soc_unregister_component(&pdev->dev);
#else
	snd_soc_unregister_dai(&pdev->dev);
#endif
	return 0;
}

static struct platform_driver iis_cpu_s3921_dev_driver = {
	.probe  = iis_cpu_s3921_dev_probe,
	.remove = __devexit_p(iis_cpu_s3921_dev_remove),
	.driver = {
		.name = "iis_cpu_s3921_dev",
		.owner = THIS_MODULE,
	},
};

module_platform_driver(iis_cpu_s3921_dev_driver);

MODULE_AUTHOR("shenzhen os team base on xavier chang ");
MODULE_DESCRIPTION("ALi PCM Asoc driver");
MODULE_LICENSE("GPL");

