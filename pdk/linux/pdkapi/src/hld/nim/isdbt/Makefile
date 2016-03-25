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

# Destination of definition files
ROOT = ../../../..

# Module Name
MOD = HLD_NIM_ISDBT

# List of source files
ASMS =
# for M3327 & M3327C
SRCS = nim_dib8000.c
# List of library files
LIBS =

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.
SMOD = LLD_NIM_DIB_MONITOR LLD_NIM_DIB_INTERFACE LLD_NIM_DIB_BUS LLD_NIM_DIB_PLATFORM LLD_NIM_DIB_DRIVER_SIP LLD_NIM_DIB_DRIVER_ADAPTER LLD_NIM_DIB_DRIVER_TUNER LLD_NIM_DIB_DRIVER_DEMOD   LLD_NIM_DIB_DRIVER_DEMOD_COMMON

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}
SMOD_ = ${SMOD:=_}
SMOD__ = ${SMOD:=__}


all : ${MOD}.mk
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.sdk	

${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
	chmod +w $@
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} >> $@

ddk_rel : ${SMOD__}
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} > dep.txt
	$(HOSTTOOLS_DIR)/ddk_rel.py -P=$(PLATFORM_DIR) -M=$(PWD) -R=$(RELEASE_DIR) -F=dep.txt 1>file.txt 2>dir.txt
	sh dir.txt
	sh file.txt
	rm -f dep.txt
	rm -f dir.txt
	rm -f file.txt

clean : ${SMOD_}
	rm -f ${OBJS} ${MOD}.mk
	cd ${LIB_DIR}; \
	  echo rm -f \\ > rm.sh; \
	  cat ${MOD}.lst >> rm.sh; \
	  sh < rm.sh; \
	  rm -f rm.sh ${MOD}.lst

rebuild: clean all
# End of common description.

# Construct of sub-modules
LLD_NIM_DIB_PLATFORM :
	cd  dibcom/platform; \
	  ${MAKE}

LLD_NIM_DIB_PLATFORM_ :
	cd  dibcom/platform; \
	  ${MAKE} clean

LLD_NIM_DIB_PLATFORM__ :
	cd  dibcom/platform; \
	  ${MAKE} ddk_rel

LLD_NIM_DIB_BUS :
	cd  dibcom/bus/spp_i2c; \
	  ${MAKE}

LLD_NIM_DIB_BUS_ :
	cd  dibcom/bus/spp_i2c; \
	  ${MAKE} clean

LLD_NIM_DIB_BUS__ :
	cd  dibcom/bus/spp_i2c; \
	  ${MAKE} ddk_rel

LLD_NIM_DIB_MONITOR :
	cd  dibcom/monitor; \
	  ${MAKE}

LLD_NIM_DIB_MONITOR_ :
	cd  dibcom/monitor; \
	  ${MAKE} clean

LLD_NIM_DIB_MONITOR__ :
	cd  dibcom/monitor; \
	  ${MAKE} ddk_rel
	  
LLD_NIM_DIB_INTERFACE :
	cd  dibcom/interface; \
	  ${MAKE}

LLD_NIM_DIB_INTERFACE_ :
	cd  dibcom/interface; \
	  ${MAKE} clean

LLD_NIM_DIB_INTERFACE__ :
	cd  dibcom/interface; \
	  ${MAKE} ddk_rel
	    
LLD_NIM_DIB_DRIVER_DEMOD_COMMON :
	cd  dibcom/driver/demod/common; \
	  ${MAKE}

LLD_NIM_DIB_DRIVER_DEMOD_COMMON_ :
	cd  dibcom/driver/demod/common; \
	  ${MAKE} clean

LLD_NIM_DIB_DRIVER_DEMOD_COMMON__ :
	cd  dibcom/driver/demod/common; \
	  ${MAKE} ddk_rel

LLD_NIM_DIB_DRIVER_SIP :
	cd  dibcom/driver/sip/dibx09x; \
	  ${MAKE}

LLD_NIM_DIB_DRIVER_SIP_ :
	cd  dibcom/driver/sip/dibx09x; \
	  ${MAKE} clean

LLD_NIM_DIB_DRIVER_SIP__ :
	cd  dibcom/driver/sip/dibx09x; \
	  ${MAKE} ddk_rel
	  
LLD_NIM_DIB_DRIVER_TUNER :
	cd  dibcom/driver/tuner/dib0090; \
	  ${MAKE}

LLD_NIM_DIB_DRIVER_TUNER_ :
	cd  dibcom/driver/tuner/dib0090; \
	  ${MAKE} clean

LLD_NIM_DIB_DRIVER_TUNER__ :
	cd  dibcom/driver/tuner/dib0090; \
	  ${MAKE} ddk_rel
	  	  
LLD_NIM_DIB_DRIVER_DEMOD :
	cd  dibcom/driver/demod/dib8000; \
	  ${MAKE}

LLD_NIM_DIB_DRIVER_DEMOD_ :
	cd  dibcom/driver/demod/dib8000; \
	  ${MAKE} clean

LLD_NIM_DIB_DRIVER_DEMOD__ :
	cd  dibcom/driver/demod/dib8000; \
	  ${MAKE} ddk_rel
	  
LLD_NIM_DIB_DRIVER_ADAPTER :
	cd  dibcom/driver/adapter; \
	  ${MAKE}

LLD_NIM_DIB_DRIVER_ADAPTER_ : 
	cd  dibcom/driver/adapter; \
	  ${MAKE} clean

LLD_NIM_DIB_DRIVER_ADAPTER__ :
	cd  dibcom/driver/adapter; \
	  ${MAKE} ddk_rel
# Deconstruct of sub-modules
