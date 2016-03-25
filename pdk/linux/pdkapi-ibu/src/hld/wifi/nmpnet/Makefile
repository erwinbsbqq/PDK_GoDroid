#
# -= Makefile for module compile =-
#
# Usage:
# . Name this file as "Makefile";
#   Put it in the same directory as module's source code.
# . Change the ROOT definition, if necessary;
#   Make it point to the root of the software tree.
# . Define the Module Name in MOD as you like;
#   There should be NO duplicated module names in the whole project.
# . List all files want to be compiled in ASMS and/or SRCS;
#   Including files, such as header files, must NOT be listed here.
# . List all library files this module may depends on in LIBS.
# . Give a new name in SMOD, if want to include sub-directories;
#   Write the linkage information at the end of file.
#
-include mipsel_evn
#INC_DIR = ${BASE_DIR}/inc
DEFINES += -pipe -O2 -Wall -W -D_REENTRANT 
#DEFINES += -pipe -O2 -Wall -W -D_REENTRANT -I${INC_DIR}

# Destination of definition files
ROOT = ../../../..
ALI_ROOT_DIR = ../${ROOT}

# Module Name
#MOD = HLD_SETTING
MOD = HLD_WIFI_NEW

# List of source files
ASMS = 
SRCS = \
	net_util.c  qlog.c  
#	net_util.c  config.c  qlog.c   client.c  client_hal.c  wifi_lib_api.c
#	hld_setting_videoformat.c
# List of library files
LIBS =


# Note: Can NOT be the same name as sub-directories.
SMOD = 

include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def
#CXX=/opt/mipsel-4.4/bin/mipsel-linux-gnu-gcc
#CXX=mipsel-linux-gnu-g++
TARGET=libnmpnet.so
LIB_DIR=${ALI_ROOT_DIR}/platform/obj/
#EXTERN_LIBDIR +=-L${ALI_ROOT_DIR}/extern/fs.install/usr/lib/

ifneq ($(strip $(INSTALLFS_IMAGES_LIBDIR)),)
else
INSTALLFS_IMAGES_LIBDIR = ${ALI_ROOT_DIR}/images/fs.install/usr/lib
endif

#DEFINES +=-I${ALI_ROOT_DIR}/extern/fs.include/ -I${ALI_ROOT_DIR}/kernel/linux/include/ -I${ALI_ROOT_DIR}/platform/inc/
DEFINES +=-I${ALI_ROOT_DIR}/extern/fs.include/ -I${ALI_ROOT_DIR}/kernel/linux/include/

OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}  ${SRCS:.cpp=.o}
SMOD_ = ${SMOD:=_}


#INSTALLFS_IMAGES_LIBDIR = ${ALI_ROOT_DIR}/test_lib
#HLDVP_LIBS = -L${INSTALLFS_IMAGES_LIBDIR}  -lvp
#HLDVP_LIBS = -L${INSTALLFS_IMAGES_LIBDIR}  -ldsc

all : ${MOD}.mk
	${CC} ${CXXFLAGS} ${DEFINES} -fPIC  -shared  -lpthread -o  ${TARGET} ${SRCS} 
	#${CXX} ${CXXFLAGS} ${DEFINES}  wifi_test test.c 
	#${CXX} -s -Wall -ldl  test.c   ${LIBS}  -L. -lnmpnet  -lpthread -o  wifi_nmpnet_test 
	$(STRIP) $(TARGET)
	chmod 777 libnmpnet.so
	#cp -f libnmpnet.so ${ROOT}/app/hybrid/wifi_deamon/
	#cp -f libnmpnet.so ${ROOT}/app/demo/combo/sabbat_dual
#cp -f libnmpnet.so   ${ROOT}/../images/app.install/usr/lib
	cp -f libnmpnet.so   $(PDKAPI_INSTALL)/lib/
#cp -f nmpnetapi.h ${ROOT}/../images/app.include/
	cp -f nmpnetapi.h $(PDKAPI_INSTALL)/lib/
#cp -f qlog.h  ${ROOT}/../images/app.include/
	cp -f qlog.h  $(PDKAPI_INSTALL)/lib/
	#cp -f wifi_nmpnet_test ${ROOT}/../images/app.install/usr/bin

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@

clean : ${SMOD_}
	@if [ -f "${MOD}.mk" ];then \
	rm -f ${MOD}.mk ; \
	fi
	@if [ -f "${TARGET}" ];then \
        rm -f ${TARGET};\
        fi
	@if [ -f "${ALI_ROOT_DIR}/images/fs/rootfs/usr/lib/${TARGET}" ]; then \
        chmod -R 755 ${ALI_ROOT_DIR}/images/fs/rootfs/usr/lib/; \
         rm -f ${ALI_ROOT_DIR}/images/fs/rootfs/usr/lib/${TARGET}; \
        chmod -R 555 ${ALI_ROOT_DIR}/images/fs/rootfs/usr/lib/; \
        fi
	@if [ -f "${ALI_ROOT_DIR}/images/fs/userfs/usr_lib/${TARGET}" ]; then \
        chmod -R 755 ${ALI_ROOT_DIR}/images/fs/userfs/usr_lib/; \
         rm -f ${ALI_ROOT_DIR}/images/fs/userfs/usr_lib/${TARGET}; \
        chmod -R 555 ${ALI_ROOT_DIR}/images/fs/userfs/usr_lib/; \
        fi
	@if [ -f "${ALI_ROOT_DIR}/images/fs.install/usr/lib/${TARGET}" ]; then \
        chmod -R 755 ${ALI_ROOT_DIR}/images/fs.install/usr/lib/; \
        rm -f ${ALI_ROOT_DIR}/images/fs.install/usr/lib/${TARGET}; \
        chmod -R 555 ${ALI_ROOT_DIR}/images/fs.install/usr/lib/;\
        fi
	@if [ -f "${ALI_ROOT_DIR}/platform/obj/${TARGET}" ]; then \
        rm -f ${ALI_ROOT_DIR}/platform/obj/${TARGET}; \
        fi
	exit 0
rebuild: clean all
# End of common description.

# Deconstruct of sub-modules




