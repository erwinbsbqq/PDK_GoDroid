#
#Makefile for ali soc dirver
#
obj-$(CONFIG_ALI_SOC) += alisoc.o

ifeq ($(CONFIG_ARM), y)
alisoc-y := ali_soc.o chip_feature.o 
else
alisoc-y := ali_soc.o chip.o chip_feature.o reboot.o
endif

