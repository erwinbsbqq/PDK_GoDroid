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
LINUX_ROOT = ../../../..
DSC_HLD_DIR = ../dsc
CE_HLD_DIR = ../ce
TRNG_HLD_DIR = ../TRNG

# Module Name
MOD = HLD_DSC
MOD_LIB = libdsc

# List of source files
ASMS =
SRCS = dsc.c dsc_ca.c dsc_tds.c
SRCS_LIB = dsc.c dsc_ca.c dsc_common.c
CE_SRCS_LIB = crypto_engine.c crypto_eng_common.c crypto_engine_utils.c
TRNG_SRCS_LIB = trng.c

# List of library files
LIBS =

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.
SMOD =

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def


OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}
OBJS_LIB = $(SRCS_LIB:.c=.o)
CE_OBJS_LIB = $(CE_SRCS_LIB:.c=.o)
TRNG_OBJS_LIB = $(TRNG_SRCS_LIB:.c=.o)

SMOD_ = ${SMOD:=_}

all : ${MOD}.mk
	@rm -f ${OBJS}
	${MAKE} -f ${MOD}.mk ${OBJS} ${SMOD}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.mod
	echo ${MOD}.lst \\ >> ${LIB_DIR}/list.sdk
#	@rm -f ${OBJS}
#	cd ${CE_HLD_DIR}; \
#		$(CC) -O1 -fPIC ${CFLAGS} -c ${CE_SRCS_LIB}; \
#		cp -f $(CE_OBJS_LIB) ${DSC_HLD_DIR}
#	cd ${TRNG_HLD_DIR}; \
#		$(CC) -O1 -fPIC ${CFLAGS} -c ${TRNG_SRCS_LIB}; \
#		cp -f $(TRNG_OBJS_LIB) ${DSC_HLD_DIR}	
#	@$(CC) -O1 -fPIC ${CFLAGS} -c ${SRCS_LIB}

#	@$(CC) ${CFLAGS}  -shared -fPIC -o $(MOD_LIB).so $(OBJS_LIB) ${CE_OBJS_LIB} ${TRNG_OBJS_LIB}
#	@$(STRIP) $(MOD_LIB).so
#	@chmod 755 $(INSTALL_DIR)/fs.install/usr/lib
#	@cp -f $(MOD_LIB).so $(INSTALL_DIR)/fs.install/usr/lib

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
	@rm -f ${OBJS} $(OBJS_LIB) ${CE_OBJS_LIB} ${TRNG_OBJS_LIB} ${MOD}.mk ${MOD_LIB}.so
	cd ${LIB_DIR}; \
	  echo rm -f \\ > rm.sh; \
	  cat ${MOD}.lst >> rm.sh; \
	  sh < rm.sh; \
	  rm -f rm.sh ${MOD}.lst

rebuild: clean all
# End of common description.

# Construct of sub-modules

# Deconstruct of sub-modules
