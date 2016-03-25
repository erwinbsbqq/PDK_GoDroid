# Makefile for ALSA
#
# ALi sound

# ALi Codec Support
#ali-snd-codec-objs := codecs/c3701/snd-codec-c3701.o codecs/c3701/AUD_c3701_reg.o
ali-snd-codec-objs := codecs/s3921/snd-codec-s3921.o codecs/s3921/AUD_s3921_reg.o 
#obj-$(CONFIG_SND_SOC_ALI_PCM) += ali-snd-codec.o
obj-m += ali-snd-codec.o


# ALi Platform Support
ali-snd-platform-objs := ali/ali-snd-platform-m39xx.o
#obj-$(CONFIG_SND_SOC_ALI_PCM) += ali-snd-platform.o
obj-m += ali-snd-platform.o

# ALi cpu Support
ali-snd-cpu-iis-objs := ali/ali-snd-m3912-iis.o
#obj-$(CONFIG_SND_SOC_ALI_PCM) += ali-snd-cpu-iis.o
obj-m += ali-snd-cpu-iis.o

# ALi Machine Support
ali-snd-machine-objs := ali/ali-snd-machine-M3912NMP.o
#obj-$(CONFIG_SND_SOC_ALI_PCM) += ali-snd-machine.o
obj-m += ali-snd-machine.o
