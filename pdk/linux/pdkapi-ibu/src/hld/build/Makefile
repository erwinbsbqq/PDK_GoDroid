# Destination to the root of software tree
export SRC_DIR := $(realpath $(shell pwd)/../../..)
export BUILD_DIR := $(SRC_DIR)/install
ROOT := $(SRC_DIR)


export BOARD := HLD_DEMO
#
#include $(BUILD_DIR)/variables.mak
#include $(BUILD_DIR)/macros.mak
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def
#

LMOD = 	OSAL_PTHREAD \
	HLD_BUS_UART \
   	HLD_BUS_DOG \
	HLD_OTP \
	HLD_MISC_RFK \
	HLD_BASE \
	HLD_PAN \
	HLD_STO \
	HLD_VP \
	HLD_OSD \
	HLD_DECV \
	HLD_DMX \
	HLD_SND \
	HLD_DECA \
	HLD_NIM \
	HLD_DSC \
	HLD_SMC \
	HLD_HDMI \
	HLD_AVSYNC \
	HLD_BDMA \
	HLD_RTC \
	HLD_APE \
	HLD_SOC \
	HLD_DBG \
	HLD_CIC	\
	HLD_FAKE \
	HLD_GE \
	HLD_SCART \
	HLD_SDEC \
	HLD_VBI	\
	HLD_CE \
	HLD_ENCODER \
	HLD_TRNG \
#	HLD_WIFI

#version mod
VMOD =

LMOD_ = ${LMOD:=_}
VMOD_ = ${VMOD:=_}

LMOD__ = ${LMOD:=__}
VMOD__ = ${VMOD:=__}

DDKLIB = libadr_hld2



.PHONY: all build

all: build


build:
	rm -f ${LIB_DIR}/list.sdk
	rm -f ${LIB_DIR}/list.nmt
#cp -Rf ${COMMON_DIR}/alidefinition ${ROOT}/src/inc
	cd ${ROOT}/src; \
	  ${MAKE} ${LMOD} ${VMOD} 
	cd ${LIB_DIR}; \
	  echo DDK = \\ > ${DDKLIB}.mak; \
	  echo cat \\ > cat.sh; \
	  cat list.sdk >> cat.sh; \
	  echo \>\> ${DDKLIB}.mak >> cat.sh; \
	  sh < cat.sh;
# rm -f cat.sh
	echo "#" >> ${LIB_DIR}/${DDKLIB}.mak	
	 
	echo all: >> ${LIB_DIR}/${DDKLIB}.mak
	echo "	${AR} -r ${DDKLIB}.a \$${DDK}" >> ${LIB_DIR}/${DDKLIB}.mak
	echo "	${CC} ${CFLAGS} -fPIC -shared -o ${DDKLIB}.so \$${DDK} -Wl,-e,livcm_main" >> ${LIB_DIR}/${DDKLIB}.mak

	echo "	" >> ${LIB_DIR}/${DDKLIB}.mak
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${DDKLIB}.mak
	
	cd ${LIB_DIR}; \
		cp -f ${DDKLIB}.a ${DDKLIB}.so $(PDKAPI_INSTALL)/lib; \
		cp -af $(PDKAPI_DIR)/src/inc/* $(PDKAPI_INSTALL)/include

#	rm -f ${LIB_DIR}/${DDKLIB}.mak

.PHONY: clean
clean:
	cd ${LIB_DIR}; \
	rm -f ${DDKLIB}.*
	cd ${ROOT}/src; \
	  ${MAKE} ${LMOD_} ${VMOD_};

.PHONY : release prerelease
release : prerelease build	
	cd ${ROOT}/src; \
		${MAKE} ${LMOD__} ${VMOD_}; \
	rm -rf $(RELEASE_DIR)/rabbit/platform/src
	rm -f $(RELEASE_DIR)/rabbit/platform/bin_lib/*
	cp -f $(ROOT)/bin_lib/${DDKLIB}.* $(RELEASE_DIR)/rabbit/platform/bin_lib
	cp -f $(ROOT)/inc/hld_cfg.h $(RELEASE_DIR)/rabbit/platform/inc
	cp -rf $(ROOT)/inc/hld/pmu $(RELEASE_DIR)/rabbit/platform/inc/hld
	cp -rf $(ROOT)/inc/hld/power $(RELEASE_DIR)/rabbit/platform/inc/hld
	-rm -f $(RELEASE_DIR)/rabbit/platform/inc/adr_sys_config.h

prerelease:
	$(if $(strip $(RELEASE_DIR)), ,$(error miss RELEASE_DIR))
	-rm -rf $(RELEASE_DIR)
	mkdir -p $(RELEASE_DIR)/rabbit/platform/inc/bus
	mkdir -p $(RELEASE_DIR)/rabbit/platform/inc/hld
	mkdir -p $(RELEASE_DIR)/rabbit/platform/bin_lib
