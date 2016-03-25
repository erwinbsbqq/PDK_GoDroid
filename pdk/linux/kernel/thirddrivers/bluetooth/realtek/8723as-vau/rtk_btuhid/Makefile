
# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then includes this
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.

EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN
ARCH:=arm
CROSS_COMPILE:= /opt/arm-linux-gnueabi/bin/arm-linux-gnueabi-
KVER:= 3.4.0
#KSRC:= /home/zmiao/work3/s3921_0812/SCT/PDK_S3921/snake/kernel/linux-linaro-3.4-rc3
#KSRC:= /home/zmiao/work5/PDK_15_0303/PDK_Release/PDK1.5/linux/kernel/kernel/build/linux-linaro-3.4-rc3
#KSRC:= /home/zmiao/work5/C3921_0318/PDK/linux/kernel/kernel/build/linux-linaro-3.4-rc3
KSRC:= /home/joy_chu/work_2/restore/workspace/20140319_c3921_verify/PDK_1.6_2014_04_04_B_IBU_SDK/PDK/linux/kernel/kernel/build/linux-linaro-3.4-rc3

ifeq ($(KERNELRELEASE),)

    # Assume the source tree is where the running kernel was built
    # You should set KERNELDIR in the environment if it's elsewhere
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    # The current directory is passed to sub-makes as argument
    PWD := $(shell pwd)

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd)  modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

.PHONY: modules modules_install clean

else
    # called from kernel build system: just declare what our modules are
    obj-m := uhid.o
endif


