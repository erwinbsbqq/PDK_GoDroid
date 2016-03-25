VERSION = 2
PATCHLEVEL = 0
SUBLEVEL = 0
EXTRAVERSION =

EXTRA_CFLAGS += -O3 -Wall

DEVICE = ali_mmc_host

ifneq ($(KERNELRELEASE),)
	obj-m := ${DEVICE}.o 
	$(DEVICE)-objs := ali_mci.o ali_gpio.o
	KBUILD_EXTRA_SYMBOLS=$(obj)/../core/Module.symvers
	KBUILD_EXTRA_SYMBOLS=$(obj)/../card/Module.symvers
else
	KERNELDIR = ../../../../kernel/build/linux-linaro-3.4-rc3
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
.PHONY:clean
clean:
	rm -rf *.o ${DEVICE}.mod* Module.symvers modules.order .*.cmd .tmp_versions

