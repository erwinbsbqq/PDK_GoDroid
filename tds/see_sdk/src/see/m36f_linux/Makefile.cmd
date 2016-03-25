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
#   Including files, such as header files, must NOT be listed here.
# . List all library files this module may depends on in LIBS.
# . Give a new name in SMOD, if want to include sub-directories;
#   Write the linkage information at the end of file.
#

# Destination to the root of software tree
ROOT = ../../..
SDK_REL_DIR=${BASE_DIR}/../sdk_release
SEE_PRJ_DIR=src/see/m36f_linux
include ${ROOT}/src/path.def
include ${ROOT}/src/compiler.def

# Module Name
MOD = SEE_OUT
BL_RMOD = LIB_BOOTUPG BOOT_SLOT3602
HDMILIB = libHDMI

# Link Script File
LSCR = ldscript.ld

SEE_LSCR = see_ldscript.ld

SEE_LINKER = see_link

# List of source files
ASMS =
SRCS = see_root.c 
 

# List of library files
LIBS =

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.
SMOD = 

# List of dependent modules

DMOD = 

# Libplus pvr+fs modules

PMOD = LIB_MP

PPMOD = PLUGIN_ALL

# Libcore library

LMOD = 	ARCH_M63 OS_TDS3 BUS_DMA_M36F BUS_DPLL\
	HLD_BASE LIB_C BUS_SCI_UART VERSION_INFO LIB_PE\
	HLD_OSD HLD_DECA HLD_SND HLD_DECV HLD_VP LIB_ISDBTCC LIB_SUBT LIB_TTX \
	HLD_SDEC HLD_VBI LLD_DECA_M36F LLD_DCEV_CFG LLD_ISDBTCC LLD_SDEC_SW LLD_VBI_M33 LLD_SDEC_HW_M33 LLD_DMX_M36F \
	HLD_DSC HLD_CRYPTO \
	LLD_DECV_M36 LLD_DECV_AVC LLD_DCEV_CFG \
	LIB_PE_ADV_VIDEO_ENGINE \
	LIB_PE_MUSIC_ENGINE \
	LIB_PE_IMAGE_ENGINE \
	DUAL_MODULES \
	HLD_GMA LLD_DECV_AVS LLD_SBM\
	HLD_AVSYNC LLD_AVSYNC LIB_IMAGE_ENGINE HLD_DMX \
	HLD_SHM_COMM LLD_SHM_COMM LIB_MCAPI LLD_RPC BUS_OTP_M33
        
ifdef CONFIG_DE_S3921
LMOD += LLD_TVE_M3921 LLD_SND_M36G LLD_GMA_M3921 LLD_VP_M3921
LMOD += LLD_VENC_AVC
else
ifdef CONFIG_DE_M3921
LMOD += LLD_TVE_M3921 LLD_SND_M36G LLD_GMA_M3921 LLD_VP_FXDE
LMOD += LLD_VENC_AVC
else
ifdef CHIP_3503_IN_USE
LMOD += LLD_VP_M36G LLD_SND_M36G LLD_GMA_M36F LLD_OSD_M36F
else
LMOD += LLD_VP_M36G LLD_SND_M36F LLD_GMA_M36F LLD_OSD_M36F
endif
endif
endif

###release Mini-see used in uboot for show jpeg logo only usage
ifdef UBOOT_SEE
ifdef CONFIG_DE_S3921
LMOD = 	ARCH_M63 OS_TDS3 BUS_DMA_M36F LIB_PE \
	HLD_BASE LIB_C BUS_SCI_UART VERSION_INFO LIB_PE_IMAGE_ENGINE\
	HLD_OSD  HLD_DECA HLD_SND HLD_DECV  HLD_VP    \
	 HLD_GMA LLD_DECA_M36F LLD_DCEV_CFG  LLD_DECV_M36 LLD_DECV_AVC LLD_SBM LLD_DECV_AVS\
	LLD_TVE_M3921 LLD_GMA_M3921_S LLD_VP_M3921\
	HLD_SHM_COMM LLD_SHM_COMM LIB_MCAPI LLD_RPC HLD_AVSYNC LLD_AVSYNC\
		LIB_PE_ADV_VIDEO_ENGINE \
	LIB_PE_MUSIC_ENGINE \
	LIB_PE_IMAGE_ENGINE 
endif

ifdef CONFIG_DE_M3921
LMOD = 	ARCH_M63 OS_TDS3 BUS_DMA_M36F LIB_PE \
	HLD_BASE LIB_C BUS_SCI_UART VERSION_INFO LIB_PE_IMAGE_ENGINE\
	HLD_OSD  HLD_DECA HLD_SND HLD_DECV  HLD_VP    \
	 HLD_GMA LLD_DECA_M36F LLD_DCEV_CFG  LLD_DECV_M36 LLD_DECV_AVC LLD_SBM LLD_DECV_AVS\
	LLD_TVE_M3921 LLD_GMA_M3921_S LLD_VP_FXDE\
	HLD_SHM_COMM LLD_SHM_COMM LIB_MCAPI LLD_RPC HLD_AVSYNC LLD_AVSYNC\
		LIB_PE_ADV_VIDEO_ENGINE \
	LIB_PE_MUSIC_ENGINE \
	LIB_PE_IMAGE_ENGINE 
endif
endif

## SEE modules
CPU_RMOD = HLD_DECA HLD_SND HLD_VP LIB_ISDBTCC LIB_SUBT LIB_TTX HLD_SDEC \
	HLD_VBI LLD_DECA_M36F LLD_SND_M36F LLD_VP_M36G LLD_ISDBTCC LLD_SDEC_SW \
	LLD_VBI_M33 LLD_DMX_M36F LLD_DSC_M36F  LLD_CRYPTO_M36F \
	HLD_DSC HLD_CRYPTO HLD_DECV LLD_DECV_M36 LLD_DECV_AVC \
	LIB_PE_ADV_VIDEO_ENGINE LIB_PE_MUSIC_ENGINE LIB_PE_IMAGE_ENGINE \
	LLD_TRNG_M36F HLD_GMA LLD_GMA_M36F LLD_DECV_AVS LLD_SBM LIB_MP4DEC\
    HLD_AVSYNC LLD_AVSYNC LIB_MJPGDEC LIB_RVDEC LIB_VP8DEC LIB_VC1DEC LIB_IMAGE_ENGINE VERSION_INFO HLD_DMX

ifdef CHIP_3921_IN_USE
CPU_RMOD += LLD_VENC_AVC
endif

ifdef CONFIG_ALI_RPCNG
CPU_RMOD += HLD_SHM_COMM LLD_SHM_COMM LIB_MCAPI LLD_RPC
endif

ifdef  BOOT_MEDIA
LMOD += BOOT_MEDIA BUS_HDMI_M36 BUS_I2C_GPIO
CPU_RMOD += BOOT_MEDIA
endif
# Release library
RMOD = 

BOOT_MOD=boot_see
SEE_RMOD = 
ALL_RMOD = HLD_BASE ${CPU_RMOD} 

RELPROJ = ${BASE_DIR}
RELLIB = libsee
#AS lib name
SECLIB_SEE=libssec_m32

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
	cp -f ${LIB_DIR}/${RELLIB}.a ${DDK_DIR}/sec/
	cp -f ${ROOT}/src/lld/sec/m36f/${SECLIB_SEE}.a ${DDK_DIR}/sec/${SECLIB_SEE}.a
	cd ${DDK_DIR}/sec/ ;\
		rm -f *.o; \
		${AR} -x ${SECLIB_SEE}.a; \
		${AR} -r ${RELLIB}.a *.o; \
		rm -f *.o
	cp -f ${DDK_DIR}/sec/${RELLIB}.a ${DDK_DIR}/

generate_see :
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
	cp -f ${SEE_LSCR} ${LIB_DIR}	
	cd ${LIB_DIR};\
	${MAKE} -f ${MOD}.lk ${SEE_LINKER}
	
sdk_release:
	rm -rf ${SDK_REL_DIR}
	mkdir ${SDK_REL_DIR}
	mkdir -p ${SDK_REL_DIR}/obj
	mkdir -p ${SDK_REL_DIR}/ddk
	mkdir -p ${SDK_REL_DIR}/ddk/plugin
	mkdir -p ${SDK_REL_DIR}/ddk/plugin_ext
	mkdir -p ${SDK_REL_DIR}/ddk/blsdk
	mkdir -p ${SDK_REL_DIR}/src/app
	mkdir -p ${SDK_REL_DIR}/${SEE_PRJ_DIR}
	
	cp ${DDK_DIR}/boot_see.o ${SDK_REL_DIR}/ddk/
	cp ${DDK_DIR}/libsee.a ${SDK_REL_DIR}/ddk/
	cp ${DDK_DIR}/libmpg2c.a ${SDK_REL_DIR}/ddk/
	cp ${DDK_DIR}/liblog.a ${SDK_REL_DIR}/ddk/
#ifdef BOOT_MEDIA
	cp ${DDK_DIR}/libHDMI.a ${SDK_REL_DIR}/ddk/
#endif
	cp -a ${PLUGINDDK_DIR}/ ${SDK_REL_DIR}/ddk/
	
	cp ${BASE_DIR}/${SEE_PRJ_DIR}/compiler_see_linux.def ${SDK_REL_DIR}/${SEE_PRJ_DIR}
	cp ${BASE_DIR}/${SEE_PRJ_DIR}/getpath.sh ${SDK_REL_DIR}/${SEE_PRJ_DIR}
	cp ${BASE_DIR}/${SEE_PRJ_DIR}/Makefile ${SDK_REL_DIR}/${SEE_PRJ_DIR}
	cp ${BASE_DIR}/${SEE_PRJ_DIR}/Makefile.cmd ${SDK_REL_DIR}/${SEE_PRJ_DIR}
	cp ${BASE_DIR}/${SEE_PRJ_DIR}/see_ldscript.ld ${SDK_REL_DIR}/${SEE_PRJ_DIR}
	
	echo ================================================== >> readme.txt
	echo Link SEE executable file for ali linux system >> readme.txt
	echo ================================================== >> readme.txt
	echo step 1. >> readme.txt
	echo 	Make sure CC_DIR in /src/see/m36f_linux/compiler_see.def >>readme.txt
	echo 	define the correct mips-sde path of your compiling PC. >> readme.txt
	echo step 2. >> readme.txt
	echo 	If need not support AC3/AAC decoding, remove AC3/AAC decoding from /ddk/plugin >>readme.txt
	echo 	plug-in file to /ddk/plugin folder. >> readme.txt
	echo step 3. >>readme.txt
	echo 	Config the path, cd src/see/m36f_linux:>>readme.txt
	echo		make path >>readme.txt
	echo step 4. >>readme.txt
	echo 	Config compiler.def , cd src/see/m36f_linux:>>readme.txt
	echo		make config_3921_release >>readme.txt
	echo step 5. >>readme.txt
	echo 	Link see.abs, cd src/see/m36f_linux:>>readme.txt
	echo		make generate_see >>readme.txt
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

plugin_cost_down:
	mv ${PLUGINDDK_DIR}/plugin_ac32c.o ${PLUGINDDK_DIR}/plugin_ac32c.o.bak
	mv ${PLUGINDDK_DIR}/plugin_eac3.o ${PLUGINDDK_DIR}/plugin_eac3.o.bak
	mv ${PLUGINDDK_DIR}/lib_mjpgdec.o ${PLUGINDDK_DIR}/lib_mjpgdec.o.bak
	mv ${PLUGINDDK_DIR}/lib_con_mjpg.o ${PLUGINDDK_DIR}/lib_con_mjpg.o.bak

plugindown_4_linux_build:
	mv ${PLUGINDDK_DIR}/lib_con_alac.o ${PLUGINDDK_DIR}/lib_con_alac.o.bak
	mv ${PLUGINDDK_DIR}/lib_con_wmapro.o ${PLUGINDDK_DIR}/lib_con_wmapro.o.bak
	mv ${PLUGINDDK_DIR}/libapedec.o ${PLUGINDDK_DIR}/libapedec.o.bak
	mv ${PLUGINDDK_DIR}/lib_con_ape.o ${PLUGINDDK_DIR}/lib_con_ape.o.bak
	
pplus_clean:
	cd ${ROOT}/src; \
	  ${MAKE} ${PPMOD_};
	  
module_config:
#add "enum REMOTE_MODULES" into inc/modules.h
	cp modules.h $(ROOT)/inc; \
	cd $(ROOT)/inc; \
  echo	${ALL_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_MODULE,\n/g" | sed -e "\$${s/\$$/_MODULE,/}" >tmp.mod; \
	sed -e ':begin; /REMOTE_MODULES/,/\}/ {/\}/! { {N; b begin};};s/MODULES.*\}/MODULES\{\n\}/;};' modules.h | sed -e '/REMOTE_MODULES/r tmp.mod' > modules_tmp.h; \
	cp modules_tmp.h modules.h; rm -f tmp.mod; rm -f modules_tmp.h 
	
#generate g_remote_callee in modules_see.c based on modules.c
	echo	${ALL_RMOD}| sed -e 's/.*/\L&/' | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_callee(UINT8 *);\n/g" | sed -e "\$${s/\$$/_callee(UINT8 *);/}" |sed -e "s/^/extern void /" > tmp1; 
	echo	${ALL_RMOD}| sed -e 's/.*/\L&/' | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_callee,\n/g" | sed -e "\$${s/\$$/_callee,/}" | sed -e "s/^/\(UINT32\)/" >tmp2;
#	echo	${SEE_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/\\\|/g" |sed -e "s/^/sed -e \"s\/\\\(/" | sed -e "s/\$$/\\\).*\/0,\/i\" tmp3/" > tmp3.sh; 
#	chmod +x ./tmp3.sh; ./tmp3.sh > tmp2; 
	sed -e ':begin; /g_remote_callee/,/\}/ {/\}/! { {N; b begin};};s/callee.*\}/callee[] = \{\n\}/;};' modules.c | sed -e '/<modules.h>/r tmp1' | sed -e '/g_remote_callee/r tmp2' > modules_see.c; \
	rm -f tmp3.sh; rm -f tmp3; rm -f tmp2; rm -f tmp1
	
#generate g_remote_callee in modules_cpu.c based on modules.c
	echo	${ALL_RMOD} | sed -e 's/.*/\L&/' | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_callee(UINT8 *);\n/g" | sed -e "\$${s/\$$/_callee(UINT8 *);/}" |sed -e "s/^/extern void /" > tmp1; \
	echo	${ALL_RMOD} | sed -e 's/.*/\L&/' | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_callee,\n/g" | sed -e "\$${s/\$$/_callee,/}" | sed -e "s/^/\(UINT32\)/" >tmp3; \
	echo	${CPU_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/\\\|/g" |sed -e "s/^/sed -e \"s\/\\\(/" | sed -e "s/\$$/\\\).*\/0,\/i\" tmp3/" > tmp3.sh; \
	chmod +x ./tmp3.sh; ./tmp3.sh > tmp2; \
	sed -e ':begin; /g_remote_callee/,/\}/ {/\}/! { {N; b begin};};s/callee.*\}/callee[] = \{\n\}/;};' modules.c | sed -e '/<modules.h>/r tmp1' | sed -e '/g_remote_callee/r tmp2' > modules_cpu.c; \
	rm -f tmp3.sh; rm -f tmp3; rm -f tmp2; rm -f tmp1

#generate compiler_see.def and compiler_cpu.def
#	echo	${ALL_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_MOD = 1\n/g" | sed -e "\$${s/\$$/_MOD = 1/}" >tmp ; cat compiler_3602.def tmp > compiler_tmp.def 
	cat compiler_3602.def > compiler_tmp.def
#	echo	${SEE_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_REMOTE = 1\n/g" | sed -e "\$${s/\$$/_REMOTE = 1\n DEFS += -DDUAL_ENABLE -DSEE_CPU/}" >tmp;  cat compiler_tmp.def tmp > compiler_see.def
	echo	"DEFS += -DDUAL_ENABLE -DSEE_CPU" >tmp; echo.>>tmp; echo "DUAL_ENABLE = 1">>tmp;  echo.>>tmp; echo "SEE_CPU = 1">>tmp;cat compiler_tmp.def tmp > compiler_see.def
#	echo	${ALL_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_MOD = 1\n/g" | sed -e "\$${s/\$$/_MOD = 1/}" >tmp; cat compiler_3602.def tmp > compiler_tmp.def
	echo	${CPU_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_REMOTE = 1\n/g" | sed -e "\$${s/\$$/_REMOTE = 1\n DEFS += -DDUAL_ENABLE -DMAIN_CPU\nDUAL_ENABLE = 1\nMAIN_CPU = 1/}" >tmp ; cat compiler_tmp.def tmp> compiler_cpu.def
	rm -f tmp;  rm -f compiler_tmp.def

module_config_linux:
#add "enum REMOTE_MODULES" into inc/modules.h
	cp modules.h $(ROOT)/inc; \
	cd $(ROOT)/inc; \
  echo	${ALL_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_MODULE,\n/g" | sed -e "\$${s/\$$/_MODULE,/}" >tmp.mod; \
	sed -e ':begin; /REMOTE_MODULES/,/\}/ {/\}/! { {N; b begin};};s/MODULES.*\}/MODULES\{\n\}/;};' modules.h | sed -e '/REMOTE_MODULES/r tmp.mod' > modules_tmp.h; \
	cp modules_tmp.h modules.h; rm -f tmp.mod; rm -f modules_tmp.h 
	
#generate g_remote_callee in modules_see.c based on modules.c
	echo	${ALL_RMOD}| sed -e 's/.*/\L&/' | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_callee(UINT8 *);\n/g" | sed -e "\$${s/\$$/_callee(UINT8 *);/}" |sed -e "s/^/extern void /" > tmp1; 
	echo	${ALL_RMOD}| sed -e 's/.*/\L&/' | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_callee,\n/g" | sed -e "\$${s/\$$/_callee,/}" | sed -e "s/^/\(UINT32\)/" >tmp2;
#	echo	${SEE_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/\\\|/g" |sed -e "s/^/sed -e \"s\/\\\(/" | sed -e "s/\$$/\\\).*\/0,\/i\" tmp3/" > tmp3.sh; 
#	chmod +x ./tmp3.sh; ./tmp3.sh > tmp2; 
	sed -e ':begin; /g_remote_callee/,/\}/ {/\}/! { {N; b begin};};s/callee.*\}/callee[] = \{\n\}/;};' modules.c | sed -e '/<modules.h>/r tmp1' | sed -e '/g_remote_callee/r tmp2' > modules_see.c; \
	rm -f tmp3.sh; rm -f tmp3; rm -f tmp2; rm -f tmp1
	
#generate g_remote_callee in modules_cpu.c based on modules.c
	echo	${ALL_RMOD} | sed -e 's/.*/\L&/' | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_callee(UINT8 *);\n/g" | sed -e "\$${s/\$$/_callee(UINT8 *);/}" |sed -e "s/^/extern void /" > tmp1; \
	echo	${ALL_RMOD} | sed -e 's/.*/\L&/' | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_callee,\n/g" | sed -e "\$${s/\$$/_callee,/}" | sed -e "s/^/\(UINT32\)/" >tmp3; \
	echo	${CPU_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/\\\|/g" |sed -e "s/^/sed -e \"s\/\\\(/" | sed -e "s/\$$/\\\).*\/0,\/i\" tmp3/" > tmp3.sh; \
	chmod +x ./tmp3.sh; ./tmp3.sh > tmp2; \
	sed -e ':begin; /g_remote_callee/,/\}/ {/\}/! { {N; b begin};};s/callee.*\}/callee[] = \{\n\}/;};' modules.c | sed -e '/<modules.h>/r tmp1' | sed -e '/g_remote_callee/r tmp2' > modules_cpu.c; \
	rm -f tmp3.sh; rm -f tmp3; rm -f tmp2; rm -f tmp1

#generate compiler_see.def and compiler_cpu.def
#	echo	${ALL_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_MOD = 1\n/g" | sed -e "\$${s/\$$/_MOD = 1/}" >tmp ; cat compiler_3602.def tmp > compiler_tmp.def 
	cat compiler_3602.def > compiler_tmp.def
#	echo	${SEE_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_REMOTE = 1\n/g" | sed -e "\$${s/\$$/_REMOTE = 1\n DEFS += -DDUAL_ENABLE -DSEE_CPU/}" >tmp;  cat compiler_tmp.def tmp > compiler_see.def
	echo	"DEFS += -DDUAL_ENABLE -DSEE_CPU -DSUPPORT_LINUX" >tmp; echo.>>tmp; echo "DUAL_ENABLE = 1">>tmp;echo.>>tmp; echo "SEE_CPU = 1">>tmp;echo.>>tmp; echo "SUPPORT_LINUX = 1">>tmp;echo.>>tmp; echo "LDFLAGS += -defsym __SEE_START_ADDR=0x84000200">>tmp;cat compiler_tmp.def tmp > compiler_see.def
#	echo	${ALL_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_MOD = 1\n/g" | sed -e "\$${s/\$$/_MOD = 1/}" >tmp; cat compiler_3602.def tmp > compiler_tmp.def
	echo	${CPU_RMOD} | sed -e "s/[! ] *\$$//" | sed -e "s/ *[! ]/_REMOTE = 1\n/g" | sed -e "\$${s/\$$/_REMOTE = 1\n DEFS += -DDUAL_ENABLE -DMAIN_CPU\nDUAL_ENABLE = 1\nMAIN_CPU = 1/}" >tmp ; cat compiler_tmp.def tmp> compiler_cpu.def
	rm -f tmp;  rm -f compiler_tmp.def
	
config_nmp_cost_down:
	sed -e '/<modules.h>/r empty' modules_see.c |\
	sed -e 's/(UINT32)lib_isdbtcc_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)lib_subt_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)lib_ttx_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)hld_sdec_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)hld_vbi_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)lld_isdbtcc_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)lld_sdec_sw_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)lld_vbi_m33_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)lld_dmx_m36f_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)lld_decv_avs_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)lib_pe_music_engine_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)lib_mjpgdec_callee/(UINT32)empty/' |\
	sed -e 's/(UINT32)hld_dmx_callee/(UINT32)empty/' > tmp1;
	cp -f tmp1 ${ROOT}/src/hld/mod/modules_see.c;rm -f tmp1

# End of common description.

# Construct of sub-modules

# Deconstruct of sub-modules

