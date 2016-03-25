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
ROOT = ../../..
ALI_ROOT_DIR = ../${ROOT}

# Module Name
MOD = HLD_WIFI

# List of source files
ASMS =
SRCS = RPCProxy_WifiConfig.c

# List of library files
#LIBS =-L${ALI_ROOT_DIR}/images/app.install/usr/lib  -lalirpcdl

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.
SMOD =

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}
SMOD_ = ${SMOD:=_}


#DEFINES +=-I${ALI_ROOT_DIR}/images/app.include/
DEFINES +=-I${ALI_ROOT_DIR}/images/app.include/ -I${ALI_ROOT_DIR}/kernel/linux/include/ali_common/ -I${ALI_ROOT_DIR}/platform/app/distro/ipc_binder/
DEFINES +=-L${ALI_ROOT_DIR}/images/app.install/usr/lib


#include "alirpc_binder.h"
DIRS = nmpnet

.PHONY : $(DIRS)


all :$(DIRS) ${MOD}.mk
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.sdk	
	${CC} ${CFLAGS} ${DEFINES} -g -Wall  RPCProxy_WifiConfig.c  test.c -lalirpcdl -lpthread -o  wifi_test  
	cp -f wifi_test ${ALI_ROOT_DIR}/images/app.install/usr/bin
	
$(DIRS):
	if [ -d $@ ]; then \
		$(MAKE) -C $@; \
	fi


${MOD}.mk : ${ASMS} ${SRCS}
	cp -f Makefile $@
	chmod +w $@
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} >> $@
	
ddk_rel :
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} > dep.txt
	${DDK_DIR}/genlist dep.txt "${BASE_DIR}" "${BASE_DIR}/../ddk_release" "${BASE_DIR}/../ddk_release" "${BASE_DIR}/../ddk_release" 1>file.txt 2>dir.txt
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

# Deconstruct of sub-modules
