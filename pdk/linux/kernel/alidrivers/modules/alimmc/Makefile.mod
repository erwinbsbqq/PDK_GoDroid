#
# Makefile for the kernel mmc device drivers.
#

# ifdef CONFIG_ALI_MMC
# CONFIG_MMC	+= y
# endif

subdir-ccflags-$(CONFIG_MMC_DEBUG) := -DDEBUG

obj-$(CONFIG_MMC)		+= core/
obj-$(CONFIG_MMC)		+= card/
obj-$(subst m,y,$(CONFIG_MMC))	+= host/

bld_core:
	@echo "building core and card modules..."
ifdef CONFIG_ALI_MMC
	@echo "CONFIG_ALI_MMC is defined."
else
	@echo "CONFIG_ALI_MMC is NOT defined."
endif
ifdef CONFIG_MMC
	@echo "CONFIG_MMC is defined."
else
	@echo "CONFIG_MMC is NOT defined."
endif
	make -C core
	make -C core clean
	make -C card
	make -C card clean
	mv core/ali_mmc_core.ko .
	mv card/ali_mmc_card.ko .

bld_host:
	@echo "building host modules..."
	make -C host
	make -C host clean
	mv host/ali_mmc_host.ko .
	date

all: bld_core bld_host

	