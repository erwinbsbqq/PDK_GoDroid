/*
 * ALSA Soc codec_s3921 codec support
 *
 *
 * Based on AC97 Soc codec, original copyright follow:
 * Copyright 2005 Wolfson Microelectronics PLC.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 * Generic codec_s3921 support.
 */
#include <linux/version.h> 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/info.h>
#include <sound/soc.h>

#define IIS_CODEC_S3921_VERSION "0.2"
#define TESTPRINT KERN_DEBUG
#define R_TARGET_VOLUME_0X6E                 0x6E // 08 bits
/*******************************************************************
*                                                                                                                    *
*             Following code are for sound controls                                                  *
*                                                                                                                    *
********************************************************************/

static int snd_s39_vol_get_single(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	//struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	//unsigned int reg = mc->reg;
	//unsigned int reg2 = mc->rreg;
	//unsigned int shift = mc->shift;
	//unsigned int rshift = mc->rshift;
	int max = mc->max;
	//unsigned int mask = (1 << fls(max)) - 1;
	unsigned int invert = mc->invert;

//	ucontrol->value.integer.value[0] =AUDREG_GetVolume();
	if (invert)
		ucontrol->value.integer.value[0] =
			max - ucontrol->value.integer.value[0];

	return 0;
}

static int snd_s39_vol_put_single(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	//struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	//unsigned int reg = mc->reg;
	//unsigned int reg2 = mc->rreg;
	//unsigned int shift = mc->shift;
	//unsigned int rshift = mc->rshift;
	int max = mc->max;
	unsigned int mask = (1 << fls(max)) - 1;
	unsigned int invert = mc->invert;
	//int err;
	//bool type_2r = 0;
	//unsigned int val2 = 0;
	unsigned int val;
	//unsigned int valval_mask;

	val = (ucontrol->value.integer.value[0] & mask);
	if (invert)
		val = max - val;

     printk(TESTPRINT "in function %s we set volume to %d \n", __FUNCTION__, val);

//     AUDREG_SetVolume(val);
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
#define S39_VOL_SINGLE(xname, reg, shift, max, invert) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = snd_soc_info_volsw, .get = snd_s39_vol_get_single,\
	.put = snd_s39_vol_put_single, \
	.private_value =  SOC_SINGLE_VALUE(reg, shift, max, invert, 0) }
#else
#define S39_VOL_SINGLE(xname, reg, shift, max, invert) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = snd_soc_info_volsw, .get = snd_s39_vol_get_single,\
	.put = snd_s39_vol_put_single, \
	.private_value =  SOC_SINGLE_VALUE(reg, shift, max, invert) }
#endif

static struct snd_kcontrol_new iis_codec_s3921_controls[] = {
       
       S39_VOL_SINGLE("Master Playback Volume", (R_TARGET_VOLUME_0X6E),  0, 255, 0),

};

/* codec private data */
struct iis_codec_s3921_priv {
	unsigned int sysclk;
	int id;
};


/*******************************************************************
*                                                                                                                    *
*             Following code are for codec DAI opration function and codec DAI         *
*                                                                                                                    *
********************************************************************/


static int iis_codec_s3921_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	//struct snd_soc_pcm_runtime *rtd = substream->private_data;
	//struct snd_soc_codec *codec = dai->codec;

	printk(TESTPRINT "in function %s \n", __FUNCTION__);


	return 0;
}

static void iis_codec_s3921_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	//struct snd_soc_pcm_runtime *rtd = substream->private_data;
	//struct snd_soc_codec *codec = dai->codec;

	printk(TESTPRINT "in function %s \n", __FUNCTION__);

}

static int iis_codec_s3921_mute(struct snd_soc_dai *dai, int mute)
{
	//struct snd_soc_codec *codec = dai->codec;
	printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}
static int iis_codec_s3921_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	//struct snd_soc_pcm_runtime *rtd = substream->private_data;
	//struct snd_soc_codec *codec = dai->codec;

	printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}

static int iis_codec_s3921_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				  int clk_id, unsigned int freq, int dir)
{
	//struct snd_soc_codec *codec = codec_dai->codec;
	printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}

static int iis_codec_s3921_set_dai_fmt(struct snd_soc_dai *codec_dai,
			       unsigned int fmt)
{
	//struct snd_soc_codec *codec = codec_dai->codec;
	printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}

static struct snd_soc_dai_ops iis_codec_s3921_dai_ops = {
	.startup	= iis_codec_s3921_startup,
	.shutdown	= iis_codec_s3921_shutdown,
	.hw_params	= iis_codec_s3921_hw_params,
	.digital_mute	= iis_codec_s3921_mute,
	.set_sysclk	= iis_codec_s3921_set_dai_sysclk,
	.set_fmt	= iis_codec_s3921_set_dai_fmt,
};

#define IIS_CODEC_S3921_RATES (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |	\
		       SNDRV_PCM_RATE_48000)
		       
static struct snd_soc_dai_driver  iis_codec_s3921_dai[] = {
{
	.name = "iis_codec_s3921_HiFi",
     	/* playback capabilities */
	.playback = {
		.stream_name = "iis_codec_s3921_Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = IIS_CODEC_S3921_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
	/* capture capabilities */
	.capture = {
		.stream_name = "iis_codec_s3921_Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = IIS_CODEC_S3921_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
        /* pcm operations */
	.ops = &iis_codec_s3921_dai_ops,
},
};


/*******************************************************************
*                                                                                                                    *
*             Following code are for codec device instantialize                                  *
*                                                                                                                    *
********************************************************************/
static unsigned int iis_codec_s3921_register_read(struct snd_soc_codec *codec, u32 regAddress)
{
	unsigned int value = 0;

	//value=(*(volatile u32 *)(AUDIO_BASE_ADDRESS+regAddress));  
	//printk(TESTPRINT "in function %s, regAddress is %x, get valueis %d \n", __FUNCTION__,regAddress, value);
	printk(TESTPRINT "in function %s, regAddress is %x\n", __FUNCTION__,regAddress);
	
	return value;
}

static int iis_codec_s3921_register_write(struct snd_soc_codec *codec, u32 regAddress, u32 value)
{

	//(*(volatile u32 *)(AUDIO_BASE_ADDRESS+regAddress)) =value;
	printk(TESTPRINT "in function %s, regAddress is %x, write valueis %d \n", __FUNCTION__,regAddress, value);
	return value;
}

static int iis_codec_s3921_probe(struct snd_soc_codec *codec)
{
	//struct iis_codec_s3921_priv *s3921Priv =  codec->dev->platform_data;
	int ret = 0;

	printk(KERN_INFO "codec_s3921 SoC Audio Codec %s\n", IIS_CODEC_S3921_VERSION);

	/* Register Sound controls. */
	ret=snd_soc_add_codec_controls(codec, iis_codec_s3921_controls,
		 ARRAY_SIZE(iis_codec_s3921_controls));
	if (ret != 0)
		dev_err(codec->dev,
		"Failed to add 3921 PCM controls: %d\n", ret);
	return ret;
}

static int iis_codec_s3921_remove(struct snd_soc_codec *codec)
{
	//struct iis_codec_s3921_priv *s3921Priv =  codec->dev->platform_data;


	return 0;
}

#ifdef CONFIG_PM
static int iis_codec_s3921_suspend(struct snd_soc_codec *codec)
{
	//struct iis_codec_s3921_priv *s3921Priv =  codec->dev->platform_data;

	return 0;
}

static int iis_codec_s3921_resume(struct snd_soc_codec *codec)
{
	//struct iis_codec_s3921_priv *s3921Priv =  codec->dev->platform_data;


	return 0;
}
#else
#define iis_codec_s3921_suspend NULL
#define iis_codec_s3921_resume NULL
#endif

/*******************************************************************
*                                                                                                                    *
*    in fact the snd_soc_codec_device is the codec register driver                         *
*                                                                                                                    *
********************************************************************/
static struct snd_soc_codec_driver iis_codec_s3921_drv= {
	.probe = 	iis_codec_s3921_probe,
	.remove = 	iis_codec_s3921_remove,
	.suspend =	iis_codec_s3921_suspend,
	.resume =	iis_codec_s3921_resume,
	.write     = 	iis_codec_s3921_register_write,
	.read	=	iis_codec_s3921_register_read,
	.controls		= iis_codec_s3921_controls,
	.num_controls		= ARRAY_SIZE(iis_codec_s3921_controls),
};

/*******************************************************************
*                                                                                                                    *
*    Following code are will register the codec interface, .codec_dai will use it        *
*                                                                                                                    *
********************************************************************/
static int __devinit iis_codec_s3921_dev_probe(struct platform_device *pdev)
{
   	printk(KERN_ERR "in function %s line %d\n", __FUNCTION__, __LINE__);

	return snd_soc_register_codec(&pdev->dev,
			&iis_codec_s3921_drv, iis_codec_s3921_dai,  ARRAY_SIZE(iis_codec_s3921_dai));
}

static int __devexit iis_codec_s3921_dev_remove(struct platform_device *pdev)
{
	printk(KERN_ERR "in function %s line %d\n", __FUNCTION__, __LINE__);
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

MODULE_ALIAS("platform:s3921-codec-dev");

static struct platform_driver iis_codec_s3921_dev_driver = {
	.probe		= iis_codec_s3921_dev_probe,
	.remove		= __devexit_p(iis_codec_s3921_dev_remove),
	.driver		= {
		.name	= "iis_codec_s3921_dev",
		.owner	= THIS_MODULE,
	},
};

module_platform_driver(iis_codec_s3921_dev_driver);

MODULE_DESCRIPTION("Soc codec_s3921 driver");
MODULE_AUTHOR("ALi shenzhen OS team");
MODULE_LICENSE("GPL");
