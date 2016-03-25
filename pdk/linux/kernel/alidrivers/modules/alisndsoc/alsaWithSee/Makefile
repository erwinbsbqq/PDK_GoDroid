# Makefile for ALSA
#
# ALi sound

# ALi Codec Support
#iis-codec-objs := codecs/c3701/snd-codec-c3701.o codecs/c3701/AUD_c3701_reg.o
obj-y += alialsa.o
alialsa-y += codecs/s3921/iis_codec_s3921.o
#obj-$(CONFIG_SND_SOC_ALI_PCM) += iis-codec.o
#obj-y += iis-codec.o


# ALi Platform Support
alialsa-y += ali/iis-platform-s39.o
#obj-$(CONFIG_SND_SOC_ALI_PCM) += iis-platform.o
#obj-y += iis-platform.o

# ALi cpu Support
alialsa-y += ali/iis-cpu-s3921.o
#obj-$(CONFIG_SND_SOC_ALI_PCM) += iis-cpu.o
#obj-y+= iis-cpu.o

# ALi spdif Codec Support
#spdif-codec-objs := ali/spdif_codec_s3921.o 
#obj-$(CONFIG_SND_SOC_ALI_PCM) += spdif-codec.o
#obj-m += spdif-codec.o


# ALi spdif Platform Support
#spdif-platform-objs := ali/spdif_platform_s39.o
#obj-$(CONFIG_SND_SOC_ALI_PCM) += spdif-platform.o
#obj-m += spdif-platform.o

# ALi spdif cpu Support
#spdif-cpu-objs := ali/spdif_cpu_s3921.o
#obj-$(CONFIG_SND_SOC_ALI_PCM) += spdif-cpu.o
#obj-m += spdif-cpu.o

alialsa-y += ali/ali_alsa_i2sirx_i2so_mix.o

alialsa-y += ali/i2sirx_i2so_mix_ring.o

alialsa-y += ali/i2so_see2main_ring.o

alialsa-y += ali/i2sirx_see2main_ring.o

alialsa-y += ali/ali_alsa_dbg.o

# ALi Machine Support
alialsa-y += ali/ali-snd-machine-S3921.o
#obj-$(CONFIG_SND_SOC_ALI_PCM) += ali-snd-machine.o
#obj-y += ali-snd-machine.o
