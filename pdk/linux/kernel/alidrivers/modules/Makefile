-include $(LINUX_DIR)/.config

ifdef CONFIG_KFT
EXTRA_CFLAGS	+= -finstrument-functions
KBUILD_CFLAGS	+= -finstrument-functions
endif

## ethernet driver ##
obj-$(CONFIG_ALI_TOE2)			+= alienet/ali_toe2/
obj-$(CONFIG_ALI_SWITCH)        += alienet/ali_switch/
obj-$(CONFIG_ALI_GMAC)        += alienet/ali_gmac/

## usb hcd driver ##
obj-$(CONFIG_USB_ALI_HCD)		+= aliusb/

## rpc driver ##
obj-$(CONFIG_RPC_ALI)		+= alirpc/

## rpcng driver ##
obj-$(CONFIG_ALI_RPCNG)                += alirpcng/

## pe driver ##
obj-$(CONFIG_PE_ALI)			+= alipe/

## sbm driver ##
obj-$(CONFIG_ALI_SBM)           += alisbm/

## stc driver ##
obj-$(CONFIG_ALI_STC)           +=alistc/

## pok driver ##
obj-$(CONFIG_ALI_POK)		+=alipok/

## hdmi driver ##
obj-$(CONFIG_HDMI_ALI)			+= alihdmi/

## hec driver ##
obj-$(CONFIG_ALI_HEC)		+=alihec/

## mmc driver ##
obj-$(CONFIG_ALI_MMC)			+= alimmc/

## video driver ##
obj-$(CONFIG_VIDEO_ALI)			+= alivideo/

## image driver ##
obj-$(CONFIG_ALI_IMAGE)			+=aliimage/

## fram buffer driver ##
obj-$(CONFIG_FB_ALI)			+= alifb/

## avsync driver ##
obj-$(CONFIG_ALI_AVSYNC_RPC)			+= aliavsync/

## DVB driver ##
obj-$(CONFIG_ALI_M36_AUDIO_RPC)		+= aliaudio/
obj-$(CONFIG_ALI_M36_CIC)		+= alicic/
obj-$(CONFIG_ALI_SDEC)		+= alisdec/
obj-$(CONFIG_ALI_VBI)		+= alivbi/
obj-$(CONFIG_ALI_M36_SMARTCARD)		+= alismc/smartcard/
obj-$(CONFIG_DVB_ALI_M36_DMX)		+= alidmx/
obj-$(CONFIG_DVB_ALI_M3602_DMX) += asdmx/
obj-$(CONFIG_DVB_ALI_M36_TSG)		+= alitsg/
obj-$(CONFIG_DVB_ALI_M36_TSI)		+= alitsi/
obj-$(CONFIG_DVB_ALI_M36_TEST)		+= alitest/

## BDMA driver ##
obj-$(CONFIG_ALI_BDMA)		+= alibdma/

## gpio i2c ##
obj-$(CONFIG_ALI_GPIO_I2C)  += alii2c/

## DVFS driver ##
obj-$(CONFIG_ALI_DVFS)  += alidvfs/

## tuner driver ##
obj-y		+= alinim/

## sound soc ##
obj-$(CONFIG_SND_SOC_ALI_PCM)	        += alisndsoc/alsaWithSee/

## openvg driver ##
obj-$(CONFIG_ALI_OPENVG)		+= aliovg/

## scart driver ##
obj-$(CONFIG_ALI_SCART)			+= aliscart/

## soc driver ##
obj-$(CONFIG_ALI_SOC)           += alisoc/

##Descrambler driver ##
obj-$(CONFIG_ALI_DSC)			+= alidsc/

## crypto engine driver ##                  
obj-$(CONFIG_ALI_CE)			+= alice/

## trng driver ##                  
obj-$(CONFIG_ALI_TRNG)			+= alitrng/

## Smart Card driver ##                  
obj-$(CONFIG_ALI_SMARTCARD)		+= alismc/smartcard/

## Crypto engine driver
obj-$(CONFIG_ALI_CRYPTO)		+= alicrypto/

## ALi power management implementation ##                  
obj-$(CONFIG_ALI_PM)		+= alipm/

## Fake trace driver ##
#obj-y	+= alifake/

## ALi UART READ WRITE driver ##
obj-$(CONFIG_ALI_UART_IO)       += aliuartio/

## ir driver ##
obj-$(CONFIG_ALI_IR)           += aliir/

## panel driver ##
obj-$(CONFIG_ALI_PAN_CH455)           += alipan/
obj-$(CONFIG_ALI_PAN_CH454)           += alipan/

## norflash driver ##
#obj-$(CONFIG_ALI_SFLASH)           += alinor/

##ALi spi driver ##
obj-$(CONFIG_ALI_SPI)           += alispi/

## ALi PMU driver ##
obj-$(CONFIG_ALI_PMU)						+= alipmu/

## ALi OTP driver ##
obj-$(CONFIG_ALI_OTP)			+= aliotp/

## ALi Video Encoder driver ##
obj-$(CONFIG_ALI_ENCODER)       += aliencoder/

## ALi Video Encoder driver ##
obj-$(CONFIG_ALI_WATCHDOG)       += aliwatchdog/

## Volume Bar driver ##
obj-$(CONFIG_ALI_VOLBAR)	+=alivolbar/

## keypad driver ##
obj-$(CONFIG_ALI_KEYPAD)	+=alikeypad/

## ALi TouchScreen driver ##
obj-y       += alitouchscreen/

## ALi GPIO PWM driver ##
obj-$(CONFIG_ALI_PWM)	+=alipwm/

.PHONY : symlinks

symlinks:
	echo "make some link or other action"
