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
#include "AUD_s3921_reg.h"
#include "../../AUD_common.h"
#include "../../AUD_reg.h"
#define codec_s3921_VERSION "0.2"
#define TESTPRINT KERN_ERR

/*******************************************************************
*                                                                                                                    *
*             Following code are for sound controls                                                  *
*                                                                                                                    *
********************************************************************/
#if 0
static int sys_io_info_single(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_info *uinfo)
{
	int mask = (kcontrol->private_value >> 15) & 0xff;

	uinfo->type = mask == 1 ? SNDRV_CTL_ELEM_TYPE_BOOLEAN : SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = mask;
	return 0;
}

static int sys_io_get_single(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	unsigned short val;

	int reg = kcontrol->private_value & 0xfff;
	int shift = (kcontrol->private_value >> 12) & 0x7;
	int mask = (kcontrol->private_value >> 15) & 0xff;
	int invert = (kcontrol->private_value >> 23) & 1;

	val = SYS_IOR_BYTE(reg);
	if (invert)
		val = ~val;
	//printk("Reg = %d, Shift = %d, Mask = %d,  Val = %d\n", reg, shift, mask, val);
	ucontrol->value.integer.value[0] = (val >> shift) & mask;
	return 0;
}

static int sys_io_put_single(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	unsigned short val;

	int reg = kcontrol->private_value & 0xfff;
	int shift = (kcontrol->private_value >> 12) & 0x7;
	int mask = (kcontrol->private_value >> 15) & 0xff;
	int invert = (kcontrol->private_value >> 23) & 1;

	val = (ucontrol->value.integer.value[0] & mask) << shift;
	if (invert)
		val = (mask << shift) - val;
	//printk("Reg = %d, Shift = %d, Mask = %d,  Val = %d\n", reg, shift, mask, val);
	SYS_IOW_BYTE(reg, (SYS_IOR_BYTE(reg)&(~(mask<<shift))) | val);
	return 0;
}
#endif


static int snd_m39_vol_get_single(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int reg2 = mc->rreg;
	unsigned int shift = mc->shift;
	unsigned int rshift = mc->rshift;
	int max = mc->max;
	unsigned int mask = (1 << fls(max)) - 1;
	unsigned int invert = mc->invert;

//	ucontrol->value.integer.value[0] =AUDREG_GetVolume();
	if (invert)
		ucontrol->value.integer.value[0] =
			max - ucontrol->value.integer.value[0];

	return 0;
}

static int snd_m39_vol_put_single(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int reg2 = mc->rreg;
	unsigned int shift = mc->shift;
	unsigned int rshift = mc->rshift;
	int max = mc->max;
	unsigned int mask = (1 << fls(max)) - 1;
	unsigned int invert = mc->invert;
	int err;
	bool type_2r = 0;
	unsigned int val2 = 0;
	unsigned int val, val_mask;

	val = (ucontrol->value.integer.value[0] & mask);
	if (invert)
		val = max - val;

     printk(TESTPRINT "in function %s we set volume to %d \n", __FUNCTION__, val);

     AUDREG_SetVolume(val);
	return 0;
}
#if 0
#define SYS_IO_SINGLE(xname, reg, shift, mask, invert) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, .info = sys_io_info_single, \
  .get = sys_io_get_single, .put = sys_io_put_single, \
  .private_value = reg | (shift << 12) | (mask << 15) | (invert << 23)\
}
#endif
#define M39_VOL_SINGLE(xname, reg, shift, max, invert) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = snd_soc_info_volsw, .get = snd_m39_vol_get_single,\
	.put = snd_m39_vol_put_single, \
	.private_value =  SOC_SINGLE_VALUE(reg, shift, max, invert) }


/*
 * hw preparation for spdif, add by alex
 */
#if 0
static int snd_m39_spdif_default_info(struct snd_kcontrol *kcontrol,
					 struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_IEC958;
	uinfo->count = 1;
	return 0;
}

static int snd_m39_spdif_default_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	struct m39_soc *chip = snd_kcontrol_chip(kcontrol);
	int i;
    
	spin_lock_irq(&chip->reg_lock);
	for (i = 0; i < 4; i++) {
		ucontrol->value.iec958.status[i] = (chip->dig_status >> (i * 8)) & 0xff;
	}
	spin_unlock_irq(&chip->reg_lock);
	return 0;
}

static int snd_m39_spdif_default_put(struct snd_kcontrol *kcontrol,
					 struct snd_ctl_elem_value *ucontrol)
{
	struct m39_soc *chip = snd_kcontrol_chip(kcontrol);
	int i, change;
	unsigned int val;

    ucontrol->value.iec958.status[1] = 0x82;
    ucontrol->value.iec958.status[2] = 0x00;
    ucontrol->value.iec958.status[3] = 0x02;

	val = 0;
	spin_lock_irq(&chip->reg_lock);
	for (i = 0; i < 4; i++) {
		val |= (unsigned int)ucontrol->value.iec958.status[i] << (i * 8);
	}
	change = val != chip->dig_status;
	chip->dig_status = val;
	spin_unlock_irq(&chip->reg_lock);
   
	return change;
}

static struct snd_kcontrol_new snd_m39_spdif_default __devinitdata =
{
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,DEFAULT),
	.info =		snd_m39_spdif_default_info,
	.get =		snd_m39_spdif_default_get,
	.put =		snd_m39_spdif_default_put
};

static int snd_m39_spdif_mask_info(struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_IEC958;
	uinfo->count = 1;
	return 0;
}

static int snd_m39_spdif_mask_get(struct snd_kcontrol *kcontrol,
				     struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.iec958.status[0] = 0xff;
	ucontrol->value.iec958.status[1] = 0xff;
	ucontrol->value.iec958.status[2] = 0xff;
	ucontrol->value.iec958.status[3] = 0xff;
	return 0;
}

static struct snd_kcontrol_new snd_m39_spdif_mask __devinitdata =
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READ,
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,CON_MASK),
	.info =		snd_m39_spdif_mask_info,
	.get =		snd_m39_spdif_mask_get,
};

static int snd_m39_spdif_stream_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_IEC958;
	uinfo->count = 1;
	return 0;
}

static int snd_m39_spdif_stream_get(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct m39_soc *chip = snd_kcontrol_chip(kcontrol);
	int i;

	spin_lock_irq(&chip->reg_lock);
	for (i = 0; i < 4; i++) {
		ucontrol->value.iec958.status[i] = (chip->dig_pcm_status >> (i * 8)) & 0xff;
	}
	spin_unlock_irq(&chip->reg_lock);
	return 0;
}

static int snd_m39_spdif_stream_put(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct m39_soc *chip = snd_kcontrol_chip(kcontrol);
	int i, change;
	unsigned int val;

	val = 0;
	spin_lock_irq(&chip->reg_lock);
	for (i = 0; i < 4; i++) {
		val |= (unsigned int)ucontrol->value.iec958.status[i] << (i * 8);
	}
	change = val != chip->dig_pcm_status;
	chip->dig_pcm_status = val;
	spin_unlock_irq(&chip->reg_lock);
	return change;
}

static struct snd_kcontrol_new snd_m39_spdif_stream __devinitdata =
{
	.access =	SNDRV_CTL_ELEM_ACCESS_READWRITE | SNDRV_CTL_ELEM_ACCESS_INACTIVE,
	.iface =	SNDRV_CTL_ELEM_IFACE_PCM,
	.name =		SNDRV_CTL_NAME_IEC958("",PLAYBACK,PCM_STREAM),
	.info =		snd_m39_spdif_stream_info,
	.get =		snd_m39_spdif_stream_get,
	.put =		snd_m39_spdif_stream_put
};


#endif

static struct snd_kcontrol_new codec_s3921_controls[] = {

	/* M39: 3911 Sorting board Mute GPIO4[1] high:mute */
//       SOC_SINGLE("Master Playback Switch", (HAL_GPIO3_DO_REG),  1, 1, 0),
       
       M39_VOL_SINGLE("Master Playback Volume", (R_TARGET_VOLUME_0X6E),  0, 255, 0),

};

/* codec private data */
struct codec_s3921_priv {
	unsigned int sysclk;
	int id;
};


/*******************************************************************
*                                                                                                                    *
*             Following code are for codec DAI opration function and codec DAI         *
*                                                                                                                    *
********************************************************************/


static int codec_s3921_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = dai->codec;

       printk(TESTPRINT "in function %s \n", __FUNCTION__);


	return 0;
}

static void codec_s3921_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = dai->codec;

       printk(TESTPRINT "in function %s \n", __FUNCTION__);

}

static int codec_s3921_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
       printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}
static int codec_s3921_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = dai->codec;

       printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}

static int codec_s3921_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				  int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
       printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}

static int codec_s3921_set_dai_fmt(struct snd_soc_dai *codec_dai,
			       unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
       printk(TESTPRINT "in function %s \n", __FUNCTION__);

	return 0;
}

static struct snd_soc_dai_ops codec_s3921_dai_ops = {
	.startup	= codec_s3921_startup,
	.shutdown	= codec_s3921_shutdown,
	.hw_params	= codec_s3921_hw_params,
	.digital_mute	= codec_s3921_mute,
	.set_sysclk	= codec_s3921_set_dai_sysclk,
	.set_fmt	= codec_s3921_set_dai_fmt,
};

#define codec_s3921_RATES (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |	\
		       SNDRV_PCM_RATE_48000)
		       
static struct snd_soc_dai_driver  codec_dai_s3921[] = {
{
	.name = "codec_s3921_HiFi",
     	/* playback capabilities */
	.playback = {
		.stream_name = "codec_s3921_Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = codec_s3921_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
	/* capture capabilities */
	.capture = {
		.stream_name = "codec_s3921_Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = codec_s3921_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
        /* pcm operations */
	.ops = &codec_s3921_dai_ops,
},
};


/*******************************************************************
*                                                                                                                    *
*             Following code are for codec device instantialize                                  *
*                                                                                                                    *
********************************************************************/
static unsigned int codec_s3921_register_read(struct snd_soc_codec *codec, u32 regAddress)
{
    unsigned int value;

    value=(*(volatile u32 *)(AUDIO_BASE_ADDRESS+regAddress));  
    printk(TESTPRINT "in function %s, regAddress is %x, get valueis %d \n", __FUNCTION__,regAddress, value);
    return value;
}

static int codec_s3921_register_write(struct snd_soc_codec *codec, u32 regAddress, u32 value)
{

    (*(volatile u32 *)(AUDIO_BASE_ADDRESS+regAddress)) =value;
    printk(TESTPRINT "in function %s, regAddress is %x, write valueis %d \n", __FUNCTION__,regAddress, value);
    return value;
}

static int codec_s3921_soc_probe(struct snd_soc_codec *codec)
{
	struct codec_s3921_priv *s3921Priv =  codec->dev->platform_data;
	int ret = 0;

	printk(KERN_INFO "codec_s3921 SoC Audio Codec %s\n", codec_s3921_VERSION);
#if 0
	socdev->card->codec = kzalloc(sizeof(struct snd_soc_codec), GFP_KERNEL);
	if (!socdev->card->codec)
		return -ENOMEM;

	codec = socdev->card->codec;
	mutex_init(&codec->mutex);

	codec->name = "codec_s3921";
	codec->owner = THIS_MODULE;
	codec->dai = &codec_dai_s3921;
	codec->num_dai = 1;

	codec->write = codec_s3921_register_write;
	codec->read = codec_s3921_register_read;

	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	/* Register PCMs. */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		printk(KERN_ERR "codec_s3921: failed to create pcms\n");
		goto pcm_err;
	}
    
    	/* Register Sound controls. */
	snd_soc_add_controls(codec, codec_s3921_controls,
			     ARRAY_SIZE(codec_s3921_controls));
    
	/* Register Card. */
	ret = snd_soc_init_card(socdev);
	if (ret < 0) {
		printk(KERN_ERR "codec_s3921: failed to register card\n");
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

static int codec_s3921_soc_remove(struct snd_soc_codec *codec)
{
	struct codec_s3921_priv *s3921Priv =  codec->dev->platform_data;


	return 0;
}

#ifdef CONFIG_PM
static int codec_s3921_soc_suspend(struct snd_soc_codec *codec)
{
	struct codec_s3921_priv *s3921Priv =  codec->dev->platform_data;

	return 0;
}

static int codec_s3921_soc_resume(struct snd_soc_codec *codec)
{
	struct codec_s3921_priv *s3921Priv =  codec->dev->platform_data;


	return 0;
}
#else
#define codec_s3921_soc_suspend NULL
#define codec_s3921_soc_resume NULL
#endif

/*******************************************************************
*                                                                                                                    *
*    in fact the snd_soc_codec_device is the codec register driver                         *
*                                                                                                                    *
********************************************************************/
static struct snd_soc_codec_driver soc_codec_dev_s3921 = {
	.probe = 	codec_s3921_soc_probe,
	.remove = 	codec_s3921_soc_remove,
	.suspend =	codec_s3921_soc_suspend,
	.resume =	codec_s3921_soc_resume,
	.write     = 	codec_s3921_register_write,
	.read	=	codec_s3921_register_read,
	.controls		= codec_s3921_controls,
	.num_controls		= ARRAY_SIZE(codec_s3921_controls),
};

/*******************************************************************
*                                                                                                                    *
*    Following code are will register the codec interface, .codec_dai will use it        *
*                                                                                                                    *
********************************************************************/
static int __devinit s3921_codec_probe(struct platform_device *pdev)
{
   	printk(KERN_ERR "in function %s line %d\n", __FUNCTION__, __LINE__);

	return snd_soc_register_codec(&pdev->dev,
			&soc_codec_dev_s3921, codec_dai_s3921,  ARRAY_SIZE(codec_dai_s3921));
}

static int __devexit s3921_codec_remove(struct platform_device *pdev)
{
     printk(KERN_ERR "in function %s line %d\n", __FUNCTION__, __LINE__);
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

MODULE_ALIAS("platform:s3921-codec");

static struct platform_driver s3921_codec_driver = {
	.probe		= s3921_codec_probe,
	.remove		= __devexit_p(s3921_codec_remove),
	.driver		= {
		.name	= "codec_s3921",
		.owner	= THIS_MODULE,
	},
};

module_platform_driver(s3921_codec_driver);

MODULE_DESCRIPTION("Soc codec_s3921 driver");
MODULE_AUTHOR("ALi shenzhen OS team");
MODULE_LICENSE("GPL");
