/*
 * ALSA Soc codec_c3701 codec support
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
 * Generic codec_c3701 support.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include "AUD_c3701_reg.h"

#define codec_c3701_VERSION "0.2"
#define TESTPRINT KERN_ERR

/*******************************************************************
*                                                                                                                    *
*             Following code are for sound controls                                                  *
*                                                                                                                    *
********************************************************************/

static struct snd_kcontrol_new codec_c3701_controls[] = {

	/* M39: 3911 Sorting board Mute GPIO4[1] high:mute */
//       SOC_SINGLE("Master Playback Switch", (HAL_GPIO3_DO_REG),  1, 1, 0),
       
       SOC_SINGLE("Master Playback Volume", (R_TARGET_VOLUME_0X6E),  0, 255, 0),

};

/* codec private data */
struct codec_c3701_priv {
	unsigned int sysclk;
	int id;
};


/*******************************************************************
*                                                                                                                    *
*             Following code are for codec DAI opration function and codec DAI         *
*                                                                                                                    *
********************************************************************/


static int codec_c3701_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = dai->codec;

       printk(TESTPRINT "in function %s \n", __FUNCTION__);


	return 0;
}

static void codec_c3701_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = dai->codec;

       printk(TESTPRINT "in function %s \n", __FUNCTION__);

}

static int codec_c3701_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
       printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}
static int codec_c3701_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = dai->codec;

       printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}

static int codec_c3701_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				  int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
       printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}

static int codec_c3701_set_dai_fmt(struct snd_soc_dai *codec_dai,
			       unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
       printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}

static struct snd_soc_dai_ops codec_c3701_dai_ops = {
	.startup	= codec_c3701_startup,
	.shutdown	= codec_c3701_shutdown,
	.hw_params	= codec_c3701_hw_params,
	.digital_mute	= codec_c3701_mute,
	.set_sysclk	= codec_c3701_set_dai_sysclk,
	.set_fmt	= codec_c3701_set_dai_fmt,
};

#define codec_c3701_RATES (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |	\
		       SNDRV_PCM_RATE_48000)
		       
static struct snd_soc_dai_driver  codec_dai_c3701[] = {
{
	.name = "codec_c3701_HiFi",
     	/* playback capabilities */
	.playback = {
		.stream_name = "codec_c3701_Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = codec_c3701_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
	/* capture capabilities */
	.capture = {
		.stream_name = "codec_c3701_Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = codec_c3701_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
        /* pcm operations */
	.ops = &codec_c3701_dai_ops,
},
};


/*******************************************************************
*                                                                                                                    *
*             Following code are for codec device instantialize                                  *
*                                                                                                                    *
********************************************************************/
static unsigned int codec_c3701_register_read(struct snd_soc_codec *codec, u32 regAddress)
{
    unsigned int value;

    value=(*(volatile u32 *)(AUDIO_BASE_ADDRESS+regAddress));  
    printk(TESTPRINT "in function %s, regAddress is %x, get valueis %d \n", __FUNCTION__,regAddress, value);
    return value;
}

static int codec_c3701_register_write(struct snd_soc_codec *codec, u32 regAddress, u32 value)
{

    (*(volatile u32 *)(AUDIO_BASE_ADDRESS+regAddress)) =value;
    printk(TESTPRINT "in function %s, regAddress is %x, write valueis %d \n", __FUNCTION__,regAddress, value);
    return value;
}

static int codec_c3701_soc_probe(struct snd_soc_codec *codec)
{
	struct codec_c3701_priv *c3701Priv =  codec->dev->platform_data;
	int ret = 0;

	printk(KERN_INFO "codec_c3701 SoC Audio Codec %s\n", codec_c3701_VERSION);
#if 0
	socdev->card->codec = kzalloc(sizeof(struct snd_soc_codec), GFP_KERNEL);
	if (!socdev->card->codec)
		return -ENOMEM;

	codec = socdev->card->codec;
	mutex_init(&codec->mutex);

	codec->name = "codec_c3701";
	codec->owner = THIS_MODULE;
	codec->dai = &codec_dai_c3701;
	codec->num_dai = 1;

	codec->write = codec_c3701_register_write;
	codec->read = codec_c3701_register_read;

	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	/* Register PCMs. */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		printk(KERN_ERR "codec_c3701: failed to create pcms\n");
		goto pcm_err;
	}
    
    	/* Register Sound controls. */
	snd_soc_add_controls(codec, codec_c3701_controls,
			     ARRAY_SIZE(codec_c3701_controls));
    
	/* Register Card. */
	ret = snd_soc_init_card(socdev);
	if (ret < 0) {
		printk(KERN_ERR "codec_c3701: failed to register card\n");
		goto card_err;
	}



	return ret;

card_err:
	snd_soc_free_pcms(socdev);
pcm_err:
	kfree(socdev->card->codec);
#endif
	return ret;
}

static int codec_c3701_soc_remove(struct snd_soc_codec *codec)
{
	struct codec_c3701_priv *c3701Priv =  codec->dev->platform_data;


	return 0;
}

#ifdef CONFIG_PM
static int codec_c3701_soc_suspend(struct snd_soc_codec *codec)
{
	struct codec_c3701_priv *c3701Priv =  codec->dev->platform_data;

	return 0;
}

static int codec_c3701_soc_resume(struct snd_soc_codec *codec)
{
	struct codec_c3701_priv *c3701Priv =  codec->dev->platform_data;


	return 0;
}
#else
#define codec_c3701_soc_suspend NULL
#define codec_c3701_soc_resume NULL
#endif

/*******************************************************************
*                                                                                                                    *
*    in fact the snd_soc_codec_device is the codec register driver                         *
*                                                                                                                    *
********************************************************************/
static struct snd_soc_codec_driver soc_codec_dev_c3701 = {
	.probe = 	codec_c3701_soc_probe,
	.remove = 	codec_c3701_soc_remove,
	.suspend =	codec_c3701_soc_suspend,
	.resume =	codec_c3701_soc_resume,
	.write     = 	codec_c3701_register_write,
	.read	=	codec_c3701_register_read,
	.controls		= codec_c3701_controls,
	.num_controls		= ARRAY_SIZE(codec_c3701_controls),
};

/*******************************************************************
*                                                                                                                    *
*    Following code are will register the codec interface, .codec_dai will use it        *
*                                                                                                                    *
********************************************************************/
static int __devinit c3701_codec_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev,
			&soc_codec_dev_c3701, codec_dai_c3701,  ARRAY_SIZE(codec_dai_c3701));
}

static int __devexit c3701_codec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

MODULE_ALIAS("platform:c3701-codec");

static struct platform_driver c3701_codec_driver = {
	.probe		= c3701_codec_probe,
	.remove		= __devexit_p(c3701_codec_remove),
	.driver		= {
		.name	= "codec_c3701",
		.owner	= THIS_MODULE,
	},
};

module_platform_driver(c3701_codec_driver);

MODULE_DESCRIPTION("Soc codec_c3701 driver");
MODULE_AUTHOR("ALi shenzhen OS team");
MODULE_LICENSE("GPL");
