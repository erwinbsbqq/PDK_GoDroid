#
# -= Makefile for application =-
#
# Note:
# . "make config" must be done for initial routines.
#
# Usage:
# . Change the ROOT definition, if necessary;
#   Make it point to the root of the software tree.
#

# Destination to the root of software tree
RUN_DIR = d:/winice
ROOT = ../../..
ROOT_PRJ = ../src/ae/m36f_linux
RLS_DATE = $(shell date +%y%m%d)
RLS_NAME = SAB36F_S2_$(RLS_DATE)_HDDual
#LINUX_DIR = /zhsa022/usrhome/jacket.wu/Workspace/PDK_1_6_2_20140726/PDK

BSP_16M = 16M_blocks_script_uboot_flash.txt

# output file name
OUT = sabbat_s3602
AE_OUT = ae_bin

all :
	${MAKE} release
	${MAKE} -f Makefile.cmd all

path:	
	./getpath.sh

config_3921_release : 
	cp -f compiler_ae_linux.def ${ROOT}/src/compiler.def
		
ui :
	${MAKE} -f Makefile.cmd all

config_linux_3921 :  compiler_3602.def sys_config_3602.h
	cp -Rf ${ROOT}/../../alicommon/alidefinition ${ROOT}/inc
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_ae.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	
config_linux_3921_linux :  compiler_3701c_linux.def sys_config_3602.h
	cp -Rf ${ROOT}/../../alicommon/alidefinition ${ROOT}/inc
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_ae_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def

config_linux_3921_linux_alidroid :  compiler_3701c_linux.def sys_config_3602.h
	cp -Rf ${ROOT}/../../alicommon/alidefinition ${ROOT}/inc
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DALIDROID_AE">>${ROOT}/src/compiler.def
	echo "ALIDROID_AE = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_ae_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DALIDROID_AE">>${ROOT}/src/compiler.def
	echo "ALIDROID_AE = 1">>${ROOT}/src/compiler.def
	
clean :
	${MAKE} -f Makefile.cmd clean

new :
	${MAKE} -f Makefile.cmd new

release :
	${MAKE} -f Makefile.cmd release

sdk_release :
	${MAKE} -f Makefile.cmd sdk_release	

generate_ae  :
	${MAKE} -f Makefile.cmd generate_ae	

generate_ae_linux  :
	${MAKE} -f Makefile.cmd generate_ae
	
product_16M:	
	${MAKE} -f Makefile.cmd generate_see
	echo generate 16M flash file ...
	${MAKE} -f Makefile.cmd product_16M
	echo generate flash file done!

fidmerge_16M:
	cd ${ROOT_PRJ}; \
	../../../prj/tools/fidmerger.exe ${BSP_16M}	

product_7z :
	echo Step 1: Compiler application ...
	${MAKE} all

	echo Step 2: Compress application bin file ...
	${MAKE} -f Makefile.cmd compress_7z
	echo Step 3: Generate Flash file ...
	${MAKE} -f Makefile.cmd generate_7z
	cp -f product_sabbat_dual.abs $(ROOT)/obj/product_sabbat_dual.abs
	echo All finished!

compress_7z :
	${OBJCOPY} -R .flash -O binary ${OUT}.out ${OUT}; \
	${OBJCOPY} -j .flash -O binary ${OUT}.out flash.sec; \
	${OBJCOPY} -R .flash -O binary ${AE_OUT}.out ${AE_OUT}; \
	${OBJCOPY} -j .flash -O binary ${AE_OUT}.out see_flash.sec; \
	rm -f ${OUT}.7z; \
	../prj/tools/lzma e ${OUT} ${OUT}.7z -lc0 -lp2
	rm -f ${AE_OUT}.7z; \
	../prj/tools/lzma e ${AE_OUT} ${AE_OUT}.7z -lc0 -lp2

generate_7z :
	rm -f app_${OUT}.bin
	cp -f ${OUT}.7z ${ROOT_PRJ}/app.bin
	cp -f ${AE_OUT}.7z ${ROOT_PRJ}/app_see.bin
	cd ${ROOT_PRJ}; \
		../../../../tools/fidmerger.exe ${BSP}

# Link file

ae_link :
	rm -f ${AE_OUT}.out
	rm -f ${AE_OUT}.abs
ifeq ($(LINUX_BUILD),1)
	${LD} ${LDFLAGS} $(shell ls $(PLUGINDDK_DIR)/*.o ) -o ${AE_OUT}.out -Map ${AE_OUT}.map \
	  -T ae_ldscript.ld ${LIB} $(DDK_DIR)/libae.a $(DDK_DIR)/liblog.a\
	    $(DDK_DIR)/mpgdec_t2.a\
	    -lm -lgcc -lc
else	
	${LD} ${LDFLAGS} $(shell ls $(PLUGINDDK_DIR)/*.o ) -o ${AE_OUT}.out -Map ${AE_OUT}.map \
	  -T ae_ldscript.ld ${LIB} $(DDK_DIR)/libae.a $(DDK_DIR)/liblog.a\
	    $(DDK_DIR)/mpgdec_t2.a\
	    -lm -lgcc -lc -le
endif	    
	${OBJCOPY} -O binary ${AE_OUT}.out ${AE_OUT}.abs
	#cp -f ${AE_OUT}.abs ${LINUX_DIR}/linux/prebuildimages/alisee/C3921A
	#cp -f ${AE_OUT}.out ${LINUX_DIR}/linux/prebuildimages/alisee/C3921A
	${OBJDUMP} -D ${AE_OUT}.out > ${AE_OUT}.dis	
	#cp -f ${AE_OUT}.dis ${LINUX_DIR}/linux/prebuildimages/alisee/C3921A
	#${LINUX_DIR}/tools/hosttools/mkimage -A mips -O linux -T kernel -C none -a 0x857741c0 -e 0x85774200 -n AudCode -d ${AE_OUT}.abs ${AE_OUT}.ubo
	#cp -f ${AE_OUT}.ubo ${LINUX_DIR}/linux/install/bin
	#cp -f ${AE_OUT}.ubo ${LINUX_DIR}/images
	cp -f ${AE_OUT}.abs ${ROOT_PRJ}
	
cleanld :
	rm -f ${OUT}.out ${OUT}.map ${OUT}.dis ${OUT}.abs libstb.a \
	  ${OUT} flash.sec ${OUT}.gz ${OUT}.7z app_${OUT}.bin

pplus:
	${MAKE} -f Makefile.cmd pplus

pplus_clean:
	${MAKE} -f Makefile.cmd pplus_clean
	
label :
	./updatelabel.bat $(RLS_NAME) > label_log.txt
