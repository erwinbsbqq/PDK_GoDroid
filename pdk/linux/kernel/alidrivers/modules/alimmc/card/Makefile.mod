EXTRA_CFLAGS += -O3 -Wall -DCONFIG_MMC_BLOCK_MINORS=16

DEVICE = ali_mmc_card

ifneq ($(KERNELRELEASE),)
	obj-m := ${DEVICE}.o 
	$(DEVICE)-objs := block.o queue.o 	
	KBUILD_EXTRA_SYMBOLS=$(obj)/../core/Module.symvers
else
	KERNELDIR = ../../../../kernel/build/linux-linaro-3.4-rc3
	PWD := $(shell pwd)
default:
ifneq (block.c, $(wildcard block.c))
	ln -s $(KERNELDIR)/drivers/mmc/card/*.c .
	ln -s $(KERNELDIR)/drivers/mmc/card/*.h .
endif
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
.PHONY:clean
clean:
	rm -rf *.o ${DEVICE}.mod* modules.order .*.cmd .tmp_versions

