############################################################################
#
# Main Makefile
# ALi Corp.
# NOTICE: Don't REMOVE or MODIFY it, but can only ADD.
#
############################################################################



############################################################################
#
# Include other make files
# NOTICE: Don't change this position!
#
############################################################################
#export SRC_DIR = $(shell pwd)
#export BUILD_DIR = $(SRC_DIR)/build
#export TARGET_DIR = $(SRC_DIR)/boards

#include $(TARGET_DIR)/$(BOARD)/variables.mak
#-include $(BOARD_DIR)/boardconfig.var
#include $(BUILD_DIR)/macros.mak
#include $(BUILD_DIR)/version.mak



############################################################################
#
# PHONY targets
#
############################################################################
.PHONY: $(HOSTTOOLS_DIR)



############################################################################
#
# Make all
#
############################################################################
all: envircheck compilecheck doprebuild \
	 kernelbuild modulesbuild dopostbuild
envircheck: makecheck kernelversioncheck toolchaincheck
compilecheck: targetdefinedcheck targetchangedcheck 



############################################################################
#
# Others build
#
############################################################################
makecheck:
	@echo "linux kernel make debug:$(BOARD)"
	@echo "==========================================================="
	@echo "           Welcome to ALi Linux SDK v$(ALI_SDK_VERSION)"
	@echo "==========================================================="
	@echo "make version is $(MAKE_VERSION)"
	@if [ `echo "$(MAKE_VERSION) < 3.81" | bc` -eq 1 ]; then \
	  echo "***********************************************************"; \
	  echo "ERROR: You must use GNU Make(>=3.81) to build the image!!"; \
	  echo "***********************************************************"; \
	  exit 1; \
	fi

kernelversioncheck:
	@echo "kernel version is $(HOST_KERNEL_VERSION)"
	@if [ `echo "$(HOST_KERNEL_VERSION) < 2.6" | bc` -eq 1 ]; then \
	  echo "***********************************************************"; \
	  echo "ERROR: You must use a relatively recent Linux distribution"; \
	  echo "ERROR: with a 2.6 kernel or above to build the image!!"; \
	  echo "***********************************************************"; \
	  exit 1; \
	fi

toolchaincheck:

	@if [ ! -f $(CC) ]; then \
	  echo "***********************************************************"; \
	  echo "ERROR: You must install toolchain!! current CC:$(CC)"; \
	  echo "***********************************************************"; \
	  exit 1; \
	else \
	  echo "Toolchain is $(CROSS_COMPILE)"; \
	fi

targetdefinedcheck:
	@if [ "$(BOARD)" = "" ]; then \
	  echo "***********************************************************"; \
	  echo "ERROR: You must use correct board name to build the image!!"; \
	  echo "***********************************************************"; \
	  echo "Usage: 'make BOARD=<board name>...', or 'make help'"; \
	  echo "The specific 'board name' is from $(TARGETS_DIR)"; \
	  exit 1; \
	else \
	  if [ ! -f $(BOARD_DIR)/$(BOARD) ]; then \
  	    echo "***********************************************************"; \
	    echo "ERROR: Your board name does not exist!!"; \
	    echo "***********************************************************"; \
	    echo "The specific 'board name' is from $(TARGETS_DIR)"; \
	    exit 1; \
	  fi \
	fi

targetchangedcheck:
	@if [ "$(LAST_BOARD)" = "" ]; then \
	  echo "$(BOARD)" > $(LAST_BOARD_COOKIE); \
	else \
	  if [ "$(LAST_BOARD)" != "$(BOARD)" ]; then \
	    echo "***********************************************************"; \
	    echo "ERROR: Your board name is different from the last build!!"; \
	    echo "***********************************************************"; \
		echo "The specified board, $(BOARD), differs from the last one, $(LAST_BOARD)."; \
		echo "The entire image must be rebuilt as follows:"; \
		echo "make BOARD=$(LAST_BOARD) clean"; \
		echo "make BOARD=$(BOARD)"; \
		exit 1; \
	  fi \
	fi


doprebuild: compilecheck
	@echo "ALI_KERNEL_CFLAGS:$(ALI_KERNEL_CFLAGS)"
	@echo "cross toolchain CC:$(CC)"
	@echo "Kernel Version:$(LINUX_KERNEL_VER)"
	rm -rf $(IMAGES_DIR)/*
	$(MKDIR) $(IMAGES_DIR)
	$(MKDIR) $(IMAGE_FS_DIR)
	$(MKDIR) $(IMAGE_MOD_DIR)
	#$(SRC_DIR)/alidrivers/patch.sh
	chmod -R 777 alidrivers/modules/aliarch/*
	@if [ -d ../../alicommon/alirpcng ]; then \
	echo "will copy the alicommon files to alidriver" ;\
	cp -rfp ../../alicommon/alirpcng/inc/* alidrivers/include/alirpcng/ ;\
	cp -rfp ../../alicommon/alirpcng/libmcapi/* alidrivers/modules/alirpcng/libmcapi/ ;\
	cp -rfp ../../alicommon/alirpcng/rpc/* alidrivers/modules/alirpcng/rpc/ ;\
	cp -rfp ../../alicommon/alirpcng/mbx/* alidrivers/modules/alirpcng/mbx/ ; \
	cp -rfp ../../alicommon/alidefinition alidrivers/include ; \
	fi
	
	$(SRC_DIR)/alidrivers/patch.sh

	@if [ "${MEM}" = "1G" ]; then \
	  echo "Use 1G Memory configuration...."; \
	  cp $(TARGET_DIR)/$(BOARD)/boardfiles/board_config_1G.h $(TARGET_DIR)/$(BOARD)/boardfiles/board_config.h -f ;\
	  cp $(TARGET_DIR)/$(BOARD)/flashfiles/memmapinfo_1G.xml $(TARGET_DIR)/$(BOARD)/flashfiles/memmapinfo.xml -f ;\
	  cp $(TARGET_DIR)/$(BOARD)/flashfiles/ubifs/Ali_nand_desc_1G.xml $(TARGET_DIR)/$(BOARD)/flashfiles/ubifs/Ali_nand_desc.xml -f ;\
	elif  [ "${MEM}" = "512M" ]; then \
	  echo "Use default Memory configuration...."; \
	  cp $(TARGET_DIR)/$(BOARD)/boardfiles/board_config_512M.h $(TARGET_DIR)/$(BOARD)/boardfiles/board_config.h -f ;\
	  cp $(TARGET_DIR)/$(BOARD)/flashfiles/memmapinfo_512M.xml $(TARGET_DIR)/$(BOARD)/flashfiles/memmapinfo.xml -f ;\
	  cp $(TARGET_DIR)/$(BOARD)/flashfiles/ubifs/Ali_nand_desc_512M.xml $(TARGET_DIR)/$(BOARD)/flashfiles/ubifs/Ali_nand_desc.xml -f ;\
	fi
	
	@if [ "$(ARCH)" != "mips" ]; then \
	  cp $(TARGET_DIR)/$(BOARD)/boardfiles/board_config.c $(LINUX_DIR)/arch/arm/mach-ali3921/board_config.c -f ;\
          cp $(TARGET_DIR)/$(BOARD)/boardfiles/board_config.h $(LINUX_DIR)/arch/arm/mach-ali3921/board_config.h -f ;\
	  cp $(TARGET_DIR)/$(BOARD)/boardfiles/tve_config.h $(LINUX_DIR)/arch/arm/mach-ali3921/tve_config.h -f ;\
	else \
	  cp $(TARGET_DIR)/$(BOARD)/boardfiles/board_config.c $(LINUX_DIR)/arch/mips/ali/m36/board_config.c -f ;\
	  cp $(TARGET_DIR)/$(BOARD)/boardfiles/board_config.h $(LINUX_DIR)/arch/mips/ali/m36/board_config.h -f ;\
	  cp $(TARGET_DIR)/$(BOARD)/boardfiles/tve_config.h $(LINUX_DIR)/arch/mips/ali/m36/tve_config.h -f ;\
	fi

	@if [ "$(BOOTMEDIUM)" = "EMMC" ]; then \
	  sed -e 's/CONFIG_MTD=y/# CONFIG_MTD is not set/' $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER) >\
	    $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER).bak ;\
	  mv $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER).bak $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER) ;\
	elif [ "$(BOOTMEDIUM)" = "NAND" ]; then \
	  sed -e 's/# CONFIG_MTD is not set/CONFIG_MTD=y/' $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER) >\
	    $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER).bak ;\
	  mv $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER).bak $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER) ;\
	fi

	@if [ "$(DEBUG)" = "" ]; then \
	  cp -f $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER) $(LINUX_DIR)/.config ;\
	else \
	  cp -f $(TARGET_DIR)/$(BOARD)/debugconfig_$(LINUX_KERNEL_VER) $(LINUX_DIR)/.config ;\
	fi

	@if [ -f $(BOARD_DIR)/$(PREBUILD_SH_FILE) ]; then \
	  echo "Do some prebuild actions before compile..."; \
	  $(BOARD_DIR)/$(PREBUILD_SH_FILE); \
	fi

kernelpatch:
	@if [ -d $(KERNEL_PATCH_SUBDIR) ]; then \
	  	echo "Do some prepatch actions for kernel before compile..."; \
		$(MAKE) -C $(KERNEL_PATCH_SUBDIR) patch; \
	fi
kernelrepatch:
	@if [ -d $(KERNEL_PATCH_SUBDIR) ]; then \
	  	echo "Do some prepatch actions for kernel before compile..."; \
		$(MAKE) -C $(KERNEL_PATCH_SUBDIR) repatch; \
	fi
	
#Need to exported kernel head files for hld building. Added by jingang, date:2014.03.08	
kernelbuild: doprebuild
	@if [ "$(RELEASEPKT)" = "Y" ]; then \
	  echo "relese packet mode, donn't need to build kernel";\
	  echo "export ALI_KERNEL_CFLAGS=\"$(ALI_KERNEL_CFLAGS)\"" >$(LINUX_DIR)/ali_cflash ; \
	  exit 1;\
	else \
	  cd $(LINUX_DIR); $(MAKE) || exit "$$?"; $(MAKE) headers_install || exit "$$?"; \
	fi
	
modulesbuild: doprebuild
	mkdir -p $(IMAGES_DIR)/bin
	mkdir -p $(IMAGES_DIR)/lib
	mkdir -p $(IMAGES_DIR)/include
	mkdir -p $(IMAGES_DIR)/etc
	$(MKDIR) $(IMAGE_MOD_DIR) 
	@if [ ! -f $(LINUX_DIR)/.config ]; then \
      echo "***********************************************************"; \
	  echo "ERROR: Your kernel config does not exist!!"; \
	  echo "***********************************************************"; \
	  echo "Please do 'make BOARD=$(BOARD)' or 'make BOARD=$(BOARD) kernel' firstly"; \
	  exit 1; \
	fi
	@if [ "$(SEE_IN_KERNEL)" = "y" ]; then \
	  cd $(LINUX_DIR); $(MAKE) modules SEEINKERNEL=1; $(MAKE) modules_install; \
	else \
	  cd $(LINUX_DIR); $(MAKE) modules; $(MAKE) modules_install; \
	fi
	echo "building third drivers";\
	cd $(SRC_DIR)/thirddrivers; $(MAKE)
	

dopostbuild: compilecheck
	@if [ "$(RELEASEPKT)" = "Y" ]; then \
		echo "donn't need to do postbuild"; \
		exit 0;\
	fi
	@if [ -f $(BOARD_DIR)/$(POSTBUILD_SH_FILE) ]; then \
	  echo "Do some postbuild actions after compile..."; \
	  $(BOARD_DIR)/$(POSTBUILD_SH_FILE); \
	  cp $(LINUX_DIR)/vmlinux $(IMAGES_DIR)/bin/main_bin.out; \
	  cd $(IMAGES_DIR)/bin/; \
	  $(OBJCOPY) -O binary main_bin.out main_bin.abs; \
	  touch $(IMAGES_DIR)/include/Readme; \
	  touch $(IMAGES_DIR)/etc/Readme; \
	  cd $(IMAGES_DIR)/include; \
#	  ln -sf $(LINUX_DIR)/include kernel; \
	  ln -sf $(LINUX_DIR)/usr/include kernel; \
	  ln -sf $(SRC_DIR)/alidrivers/include alidrivers; \
	  cp -af $(BOARD_DIR)/flashfiles $(IMAGES_DIR)/etc/; \
	fi



############################################################################
#
# Lists for user to make 
#
############################################################################

linux-config: targetdefinedcheck
	$(SRC_DIR)/alidrivers/patch.sh
	@if [ "$(DEBUG)" = "" ]; then \
	 cp -f $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER) $(LINUX_DIR)/.config ;\
	else \
	 cp -f $(TARGET_DIR)/$(BOARD)/debugconfig_$(LINUX_KERNEL_VER) $(LINUX_DIR)/.config ;\
	fi
	cd $(LINUX_DIR);make menuconfig
	@if [ "$(DEBUG)" = "" ]; then \
	 cp -f $(LINUX_DIR)/.config $(TARGET_DIR)/$(BOARD)/defkernelconfig_$(LINUX_KERNEL_VER) ;\
	else \
	 cp -f $(LINUX_DIR)/.config $(TARGET_DIR)/$(BOARD)/debugconfig_$(LINUX_KERNEL_VER) ;\
	fi


help:
	@echo "***********************************************************"
	@echo "************************ PDK linux make help ************************"
	@echo "***********************************************************"
	@echo "make BOARD=[XX] FULL=Y -- compile kernel and minimal rootfs image"
	@echo "make BOARD=[XX] linux-config -- configure linux kernel and alidrivers"
	@echo "make BOARD=[XX] -- just compile and create linux kernel image"
	@echo ""


############################################################################
#
# Clean up
#
############################################################################
clean: targetclean kernelclean


targetclean: targetdefinedcheck
	$(RM) $(LAST_BOARD_COOKIE)

kernelclean: targetdefinedcheck
	$(MAKE) -C $(LINUX_DIR) mrproper
	$(RMDIR) $(KERNEL_DIR)/.tmp_versions
	rm -rf $(IMAGES_DIR)/*	



