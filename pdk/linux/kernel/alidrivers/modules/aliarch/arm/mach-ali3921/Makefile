#
# Makefile for the linux kernel.
#
obj-y += aliarch.o
aliarch-y				+= ali-s3921.o board_config.o m36_gpio.o ali-cache.o unified_bsp_desc.o
aliarch-$(CONFIG_SMP)		+= headsmp.o platsmp.o timer_smp.o
aliarch-$(CONFIG_HOTPLUG_CPU)	+= hotplug.o
aliarch-y				+= clock.o s3921-clock.o
aliarch-$(CONFIG_ALI_PM) 		+= sleepdep.o m3921_sleep.o  ali_suspend.o
