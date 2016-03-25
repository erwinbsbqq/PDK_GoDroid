EXTRA_CFLAGS += -O3 -Wall

DEVICE = ali_mmc_core

ifneq ($(KERNELRELEASE),)
	obj-m := ${DEVICE}.o 
	$(DEVICE)-objs := core.o bus.o host.o \
				   mmc.o mmc_ops.o sd.o sd_ops.o \
				   sdio.o sdio_ops.o sdio_bus.o \
				   sdio_cis.o sdio_io.o sdio_irq.o \
				   quirks.o cd-gpio.o debugfs.o
else
	KERNELDIR = ../../../../kernel/build/linux-linaro-3.4-rc3
	PWD := $(shell pwd)
default:
ifneq (sd_ops.h, $(wildcard sd_ops.h))
	ln -s $(KERNELDIR)/drivers/mmc/core/sdio*.* .
	ln -s $(KERNELDIR)/drivers/mmc/core/mmc.c .
	ln -s $(KERNELDIR)/drivers/mmc/core/mmc_ops.* .
	ln -s $(KERNELDIR)/drivers/mmc/core/host.* .
	ln -s $(KERNELDIR)/drivers/mmc/core/bus.* .
	ln -s $(KERNELDIR)/drivers/mmc/core/sd_ops.* .
	ln -s $(KERNELDIR)/drivers/mmc/core/quirks.c .
	ln -s $(KERNELDIR)/drivers/mmc/core/debugfs.c .
	ln -s $(KERNELDIR)/drivers/mmc/core/cd-gpio.c .
	ln -s $(KERNELDIR)/drivers/mmc/core/sd.* .
endif
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
.PHONY:clean
clean:
	rm -rf *.o ${DEVICE}.mod* modules.order .*.cmd .tmp_versions

