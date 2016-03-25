#
# Makefile for MMC/SD card drivers
#

obj-$(CONFIG_ALI_MMC)		+= alimmc.o
alimmc-y			:= block.o queue.o

EXTRA_CFLAGS += -O3 -Wall -DCONFIG_MMC_BLOCK_MINORS=16
