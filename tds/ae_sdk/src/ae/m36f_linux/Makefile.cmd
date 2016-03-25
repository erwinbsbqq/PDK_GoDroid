#
# -= Makefile for application compile =-
#
# Note:
# . This is the 1st part of two makefile components;
#   Please refer "Makefile.lin" for the other part.
#
# Usage:
# . Name this file as "Makefile";
#   Put it in the same directory as application's source code.
# . Change the ROOT definition, if necessary;
#   Make it point to the root of the software tree.
# . Take application as a module, define the name in MOD;
#   There should be NO duplicated module names in the whole project.
# . List all files want to be compiled in ASMS and/or SRCS;
#   Including files, such as header files, must NfOT be listed here.
# . List all library files this module may depends on in LIBS.
# . Give a new name in SMOD, if want to include sub-directories;
#   Write the linkage information at the end of file.
#

# Destination to the root of software tree
ROOT = ../../..
SDK_REL_DIR=${BASE_DIR}/../ae_sdk_release
AE_PRJ_DIR=src/ae/m36f_linux
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

# Module Name
MOD = AE_OUT
BL_RMOD = LIB_BOOTUPG BOOT_SLOT3602

# Link Script File
AE_LSCR = ae_ldscript.ld

AE_LINKER = ae_link

# List of source files
ASMS =
SRCS = ae_root.c
 

# List of library files
LIBS =

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.
SMOD = 

# List of dependent modules

DMOD = 

# Libplus pvr+fs modules

PMOD =

PPMOD = AE_PLUGIN_ALL

# Libcore library

LMOD = 	ARCH_M63 OS_TDS3 HLD_BASE LIB_C VERSION_INFO BUS_SCI_UART HLD_AE LLD_AE

## SEE modules
CPU_RMOD = VERSION_INFO HLD_AE LLD_AE

# Release library
RMOD = 

BOOT_MOD=boot_ae
SEE_RMOD = 
ALL_RMOD = HLD_BASE ${CPU_RMOD} 

RELPROJ = ${BASE_DIR}
RELLIB = libae

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.

OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o}
SMOD_ = ${SMOD:=_}
LMOD_ = ${LMOD:=_}
DMOD_ = ${DMOD:=_}
RMOD_ = ${RMOD:=_}
PMOD_ = ${PMOD:=_}
PPMOD_ = ${PPMOD:=_}
SMOD__ = ${SMOD:=__}
LMOD__ = ${LMOD:=__}
DMOD__ = ${DMOD:=__}
RMOD__ = ${RMOD:=__}
PMOD__ = ${PMOD:=__}
PPMOD__= ${PPMOD:=__}

compress_7z :
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk compress_7z

generate_7z :
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${MOD}.lk generate_7z

release :
	rm -f ${LIB_DIR}/list.sdk
	rm -f ${LIB_DIR}/list.mod
	cd ${ROOT}/src; \
	  ${MAKE} ${LMOD} ${PMOD}
	cd ${LIB_DIR}; \
	  echo NEED = \\ > ${RELLIB}.mak; \
	  echo cat \\ > cat.sh; \
	  cat list.mod >> cat.sh; \
	  echo \>\> ${RELLIB}.mak >> cat.sh; \
	  sh < cat.sh; \
	  rm -f cat.sh
	echo "#" >> ${LIB_DIR}/${RELLIB}.mak
	echo all: >> ${LIB_DIR}/${RELLIB}.mak
	echo "	${AR} -r ${RELLIB}.a \$${NEED}" >> ${LIB_DIR}/${RELLIB}.mak
	cd ${LIB_DIR}; \
	  ${MAKE} -f ${RELLIB}.mak

all : ${MOD}.mk
	${MAKE} -f ${MOD}.mk ${OBJS}
	cp -f ${OBJS} ${LIBS} ${LIB_DIR}
	echo ${OBJS} ${LIBS} \\ > ${LIB_DIR}/${MOD}.lst
	rm -f ${LIB_DIR}/list.mod
	cd ${LIB_DIR}; \
	  echo include ../src/path.def > ${MOD}.lk; \
	  echo include ../src/compiler.def >> ${MOD}.lk; \
	  echo NEED = \\ >> ${MOD}.lk; \
	    echo cat \\ > cat.sh; \
	    cat list.mod >> cat.sh; \
	    echo ${MOD}.lst \\ >> cat.sh; \
	    echo \>\> ${MOD}.lk >> cat.sh; \
	    cp cat.sh catt.sh; \
	    sh < cat.sh; \
	    rm -f cat.sh;
	echo "#" >> ${LIB_DIR}/${MOD}.lk
	echo ar: >> ${LIB_DIR}/${MOD}.lk
	echo "	${AR} -r ${RELLIB}.a \$${NEED} boot.o" >> ${LIB_DIR}/${MOD}.lk
	cd ${LIB_DIR}; \
  		${MAKE} -f ${MOD}.lk ar
	cp -f ${LIB_DIR}/boot.o ${DDK_DIR}/${BOOT_MOD}.o
	cp -f ${LIB_DIR}/libae.a ${DDK_DIR}/

generate_ae :
	cd ${LIB_DIR}; \
	  echo include ../src/path.def > ${MOD}.lk; \
	  echo include ../src/compiler.def >> ${MOD}.lk; \
	  echo NEED = \\ >> ${MOD}.lk; \
	    echo cat \\ > cat.sh; \
	    cat list.mod >> cat.sh; \
	    echo ${MOD}.lst \\ >> cat.sh; \
	    echo \>\> ${MOD}.lk >> cat.sh; \
	    cp cat.sh catt.sh; \
	    sh < cat.sh; \
	    rm -f cat.sh;
	echo "#" >> ${LIB_DIR}/${MOD}.lk
	cat Makefile >> ${LIB_DIR}/${MOD}.lk
	echo ar: >> ${LIB_DIR}/${MOD}.lk
	echo "	${AR} -r ${RELLIB}.a \$${NEED} boot.o" >> ${LIB_DIR}/${MOD}.lk	
	cp -f ${AE_LSCR} ${LIB_DIR}	
	cd ${LIB_DIR};\
	${MAKE} -f ${MOD}.lk ${AE_LINKER}
	
sdk_release:
	rm -rf ${SDK_REL_DIR}
	mkdir ${SDK_REL_DIR}
	mkdir -p ${SDK_REL_DIR}/obj
	mkdir -p ${SDK_REL_DIR}/ddk
	mkdir -p ${SDK_REL_DIR}/ddk/ae_plugin
	mkdir -p ${SDK_REL_DIR}/ddk/ae_plugin_ext
	mkdir -p ${SDK_REL_DIR}/ddk/blsdk
	mkdir -p ${SDK_REL_DIR}/src/app
	mkdir -p ${SDK_REL_DIR}/${AE_PRJ_DIR}
	
	cp ${DDK_DIR}/boot_ae.o ${SDK_REL_DIR}/ddk/
	cp ${DDK_DIR}/libae.a ${SDK_REL_DIR}/ddk/
	cp ${DDK_DIR}/mpgdec_t2.a ${SDK_REL_DIR}/ddk/
	cp ${DDK_DIR}/liblog.a ${SDK_REL_DIR}/ddk/
	cp -a ${PLUGINDDK_DIR}/ ${SDK_REL_DIR}/ddk/
	cp ${ROOT}/src/compiler.def ${SDK_REL_DIR}/${AE_PRJ_DIR}/compiler_ae_linux.def
	cp ${BASE_DIR}/${AE_PRJ_DIR}/getpath.sh ${SDK_REL_DIR}/${AE_PRJ_DIR}
	cp ${BASE_DIR}/${AE_PRJ_DIR}/Makefile ${SDK_REL_DIR}/${AE_PRJ_DIR}
	cp ${BASE_DIR}/${AE_PRJ_DIR}/Makefile.cmd ${SDK_REL_DIR}/${AE_PRJ_DIR}
	cp ${BASE_DIR}/${AE_PRJ_DIR}/ae_ldscript.ld ${SDK_REL_DIR}/${AE_PRJ_DIR}
	
	echo ================================================== >> readme.txt
	echo Link SEE executable file for ali linux system >> readme.txt
	echo ================================================== >> readme.txt
	echo step 1. >> readme.txt
	echo 	Make sure CC_DIR in /src/ae/m36f_linux/compiler_ae.def >>readme.txt
	echo 	define the correct mips-sde path of your compiling PC. >> readme.txt
	echo step 2. >> readme.txt
	echo 	If need not support AC3/AAC decoding, remove AC3/AAC decoding from /ddk/ae_plugin >>readme.txt
	echo 	plug-in file to /ddk/ae_plugin folder. >> readme.txt
	echo step 3. >>readme.txt
	echo 	Config the path, cd src/ae/m36f_linux:>>readme.txt
	echo		make path >>readme.txt
	echo step 4. >>readme.txt
	echo 	Config compiler.def , cd src/ae/m36f_linux:>>readme.txt
	echo		make config_3921_release >>readme.txt
	echo step 5. >>readme.txt
	echo 	Link ae_bin.abs, cd src/ae/m36f_linux:>>readme.txt
	echo		make generate_ae >>readme.txt
	cp readme.txt ${SDK_REL_DIR}
	rm readme.txt
	
new :
	rm -f ${OBJS} ${MOD}.mk
	cd ${ROOT}/src; \
	  ${MAKE} ${SMOD_} ${LMOD_} ${DMOD_} ${RMOD_} ${PMOD_}
	  
${MOD}.mk : ${ASMS} ${SRCS}
	echo include ${ROOT}/src/path.def > $@
	echo include ${ROOT}/src/compiler.def >> $@
	cat Makefile >> $@
	${CC} ${CFLAGS} -M ${ASMS} ${SRCS} >> $@

pplus:
	cd ${ROOT}/src; \
	  ${MAKE} ${PPMOD}

pplus_clean:
	cd ${ROOT}/src; \
	  ${MAKE} ${PPMOD_};

module_config:
#generate compiler_ae.def
	cat compiler_3602.def > compiler_tmp.def
ifdef ALIDROID_AE
	echo	"DEFS += -DAE_CPU -DSUPPORT_LINUX" >tmp; echo.>>tmp; echo "AE_CPU = 1">>tmp;echo.>>tmp; echo "SUPPORT_LINUX = 1">>tmp;echo.>>tmp; echo "LDFLAGS += -defsym __AE_START_ADDR=0x88000200">>tmp;cat compiler_tmp.def tmp > compiler_ae.def	
else
	echo	"DEFS += -DAE_CPU -DSUPPORT_LINUX" >tmp; echo.>>tmp; echo "AE_CPU = 1">>tmp;echo.>>tmp; echo "SUPPORT_LINUX = 1">>tmp;echo.>>tmp; echo "LDFLAGS += -defsym __AE_START_ADDR=0x85EFD200">>tmp;cat compiler_tmp.def tmp > compiler_ae.def
endif
	rm -f tmp;  rm -f compiler_tmp.def
	
module_config_linux:
#generate compiler_ae_linux.def
	cat compiler_3701c_linux.def > compiler_tmp.def
ifdef ALIDROID_AE
	echo	"DEFS += -DAE_CPU -DSUPPORT_LINUX" >tmp; echo.>>tmp; echo "LINUX_BUILD = 1">>tmp;echo.>>tmp; echo "AE_CPU = 1">>tmp;echo.>>tmp; echo "SUPPORT_LINUX = 1">>tmp;echo.>>tmp; echo "LDFLAGS += -defsym __AE_START_ADDR=0x88000200">>tmp;cat compiler_tmp.def tmp > compiler_ae_linux.def
else
	echo	"DEFS += -DAE_CPU -DSUPPORT_LINUX" >tmp; echo.>>tmp; echo "LINUX_BUILD = 1">>tmp;echo.>>tmp; echo "AE_CPU = 1">>tmp;echo.>>tmp; echo "SUPPORT_LINUX = 1">>tmp;echo.>>tmp; echo "LDFLAGS += -defsym __AE_START_ADDR=0x85EFD200">>tmp;cat compiler_tmp.def tmp > compiler_ae_linux.def
endif
	rm -f tmp;  rm -f compiler_tmp.def

# End of common description.

# Construct of sub-modules

# Deconstruct of sub-modules

