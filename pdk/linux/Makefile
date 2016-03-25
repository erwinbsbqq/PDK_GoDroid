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
export LINUX_SRC_DIR = $(shell pwd)
export BUILD_APP_BUSYBOX = y

export SRC_DIR = $(shell pwd)/kernel
export BUILD_DIR = $(SRC_DIR)/build
export TARGET_DIR = $(SRC_DIR)/boards

include $(TARGET_DIR)/$(BOARD)/variables.mak
-include $(BOARD_DIR)/boardconfig.var
include $(BUILD_DIR)/macros.mak
include $(BUILD_DIR)/version.mak

############################################################################
#
# PHONY targets
#
############################################################################
.PHONY: all 



############################################################################
#
# Make all
#
############################################################################
all: prebuild alibootbuild kernelbuild pdkapibuild fsmodulesbuild imagesbuild 



############################################################################
#
# build
#
############################################################################
prebuild:
	rm -rf $(LINUX_SRC_DIR)/install/*
	mkdir -p $(LINUX_SRC_DIR)/install/bin
	mkdir -p $(LINUX_SRC_DIR)/install/lib
	mkdir -p $(LINUX_SRC_DIR)/install/include
	mkdir -p $(LINUX_SRC_DIR)/install/etc

alibootbuild:
	cp -rf $(LINUX_SRC_DIR)/prebuildimages/alisee/$(BOARD)/* $(LINUX_SRC_DIR)/install/bin
	cp -rf $(LINUX_SRC_DIR)/prebuildimages/aliboot/$(BOARD)/* $(LINUX_SRC_DIR)/install/bin	
	@if [ "$(ALIAS)" = "Y" ]; then \
   	 rm -rf $(LINUX_SRC_DIR)/install/bin/see_bin.abs; \
   	 rm -rf $(LINUX_SRC_DIR)/install/bin/see_bin_as.abs; \
	 cp $(LINUX_SRC_DIR)/prebuildimages/alisee/$(BOARD)/see_bin_as.abs $(LINUX_SRC_DIR)/install/bin/see_bin.abs; \
	else \
	 echo "use default see_bin.abs"; \
	fi	

kernelbuild: 
	cd $(LINUX_SRC_DIR)/kernel; $(MAKE) || exit "$$?"; \
	cp -rfp $(LINUX_SRC_DIR)/kernel/install/bin/* $(LINUX_SRC_DIR)/install/bin
	cp -rfp $(LINUX_SRC_DIR)/kernel/install/lib/* $(LINUX_SRC_DIR)/install/lib
	cp -rfp $(LINUX_SRC_DIR)/kernel/install/include/* $(LINUX_SRC_DIR)/install/include
	cp -rfp $(LINUX_SRC_DIR)/kernel/install/etc/* $(LINUX_SRC_DIR)/install/etc

pdkapibuild:
	@if [ "$(FULL)" = "" ]; then \
	 echo "FULL:$(FULL), pdkapi, Donn't need to build the minimal rootfs"; \
	else \
	 if [ "$(IBUSDK)" = "Y" ]; then \
	  $(MAKE) -C $(LINUX_SRC_DIR)/pdkapi-ibu/ || exit "$$?"; \
	  if [ "$(ALIDBG)" = "Y" ]; then \
	   $(MAKE) -C $(LINUX_SRC_DIR)/pdkapi/ || exit "$$?"; \
	  fi \
	 else \
	  $(MAKE) -C $(LINUX_SRC_DIR)/pdkapi/ || exit "$$?"; \
	 fi \
	fi

ibupdkapibuild:
	$(MAKE) -C $(LINUX_SRC_DIR)/pdkapi-ibu/ || exit "$$?"; 
	rm -rf $(LINUX_SRC_DIR)/release_for_ibu; 
	mkdir $(LINUX_SRC_DIR)/release_for_ibu; 
	mkdir $(LINUX_SRC_DIR)/release_for_ibu/ali_pdk_headfiles; 
	mkdir $(LINUX_SRC_DIR)/release_for_ibu/ali_pdk_headfiles/kern; 	 
	mkdir $(LINUX_SRC_DIR)/release_for_ibu/ali_pdk_headfiles/hld; 	
	cp -rf $(LINUX_SRC_DIR)/kernel/alidrivers/include $(LINUX_SRC_DIR)/release_for_ibu/ali_pdk_headfiles/kern;  
	cp -rf $(LINUX_SRC_DIR)/pdkapi-ibu/src/inc $(LINUX_SRC_DIR)/release_for_ibu/ali_pdk_headfiles/hld;
	rm -rf $(LINUX_SRC_DIR)/release_for_ibu/ali_lib;
	mkdir $(LINUX_SRC_DIR)/release_for_ibu/ali_lib;
	cp -rf $(LINUX_SRC_DIR)/pdkapi-ibu/install/lib/libadr_hld.a $(LINUX_SRC_DIR)/release_for_ibu/ali_lib; 
	cp -rf $(LINUX_SRC_DIR)/pdkapi-ibu/install/lib/libadr_hld.so $(LINUX_SRC_DIR)/release_for_ibu/ali_lib; 
	 
fsmodulesbuild:
	@if [ "$(FULL)" = "" ]; then \
	 echo "FULL:$(FULL), fsmodules, Donn't need to build the minimal rootfs"; \
	else \
	 if [ "$(IBUSDK)" = "Y" ]; then \
	  export IBUSDK= Y; \
	 fi; \
	 $(MAKE) -C $(LINUX_SRC_DIR)/fsmodules/ || exit "$$?"; \
	 cp -ap $(LINUX_SRC_DIR)/fsmodules/install/fs $(LINUX_SRC_DIR)/install/; \
	fi

imagesbuild:
	@if [ "$(IBUSDK)" = "" ]; then \
	 $(LINUX_SRC_DIR)/prebuildimages/buildimage.sh; \
	 echo "Linux SDK Build Finish!"; \
	else \
	 cd $(LINUX_SRC_DIR)/kernel/boards/$(BOARD); ./buildimage.sh;\
	fi	

linux-config:
	cd $(LINUX_SRC_DIR)/kernel; $(MAKE) linux-config;


############################################################################
#
# Clean up
#
############################################################################
clean:  kernelclean pdkapiclean fsmodulesclean imagesclean


kernelclean:
	@if [ -d $(LINUX_SRC_DIR)/kernel ]; then \
	cd $(LINUX_SRC_DIR)/kernel; $(MAKE) clean; \
	else \
	echo "the kernel doesn't exist, donn't need to clean!"; \
	fi

pdkapiclean:
	@if [ -d $(LINUX_SRC_DIR)/pdkapi ]; then \
	cd $(LINUX_SRC_DIR)/pdkapi; $(MAKE) clean; \
	else \
	echo "the pdkapi doesn't exist, donn't need to clean!"; \
	fi

fsmodulesclean:
	@if [ -d $(LINUX_SRC_DIR)/fsmodules ]; then \
	cd $(LINUX_SRC_DIR)/fsmodules; $(MAKE) clean; \
	else \
	echo "the fsmodules doesn't exist, donn't need to clean!"; \
	fi

imagesclean: 
	rm -rf $(LINUX_SRC_DIR)/install/*


