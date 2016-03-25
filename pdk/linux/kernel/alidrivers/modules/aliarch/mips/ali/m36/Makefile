
ifdef CONFIG_KFT
EXTRA_CFLAGS	+= -finstrument-functions
KBUILD_CFLAGS	+= -finstrument-functions
endif

obj-y := init.o console.o time.o cmdline.o memory.o m36_platform.o m36_irq.o m36_setup.o m36_printf.o \
		board_config.o m36_gpio.o m36_setup_board.o ali_mem_map.o


obj-$(CONFIG_ALI_PM) += unified_sleepdep.o unified_sleep.o ali_suspend.o

ifdef CONFIG_ALI_PM

ifdef CONFIG_INPUT_ALI_PAN_CH455
obj-y +=  slot_ch455.o i2c_scb.o
else
obj-y += slot_null.o
endif

endif


EXTRA_CFLAGS += -Werror
