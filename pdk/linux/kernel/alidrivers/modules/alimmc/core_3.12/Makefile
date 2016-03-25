#
# Makefile for the kernel mmc core.
#

obj-$(CONFIG_ALI_MMC)		+= alimmc_core.o
alimmc_core-y			:= core.o bus.o host.o \
				   mmc.o mmc_ops.o sd.o sd_ops.o \
				   sdio.o sdio_ops.o sdio_bus.o \
				   sdio_cis.o sdio_io.o sdio_irq.o \
				   quirks.o slot-gpio.o

alimmc_core-$(CONFIG_DEBUG_FS)	+= debugfs.o
