ifeq ($(shell echo "$$(((($(VERSION)<<8)|$(PATCHLEVEL))==((3<<8)|4)))"), 1)
obj-$(CONFIG_ALI_MMC)  += core/
else
obj-$(CONFIG_ALI_MMC)  += core_3.12/
endif
obj-$(CONFIG_ALI_MMC)  += card/
obj-$(CONFIG_ALI_MMC)  += host/

.PHONY : clean
clean:
	@echo "mmc driver clean..."
	@find . -name '*.o' | xargs rm -rf
	@find . -name '*.cmd' | xargs rm -rf
	@find . -name 'modules.order' | xargs rm -rf
	@find . -name 'modules.builtin' | xargs rm -rf
	@find . -name '*.mod.c' | xargs rm -rf

.PHONY : cleanln
cleanln:
	@echo "mmc driver clean link file..."
	@find . -type l |xargs rm -rf
