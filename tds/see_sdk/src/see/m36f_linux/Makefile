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
ROOT_PRJ = ../src/see/m36f_linux
RLS_DATE = $(shell date +%y%m%d)
RLS_NAME = SAB36F_S2_$(RLS_DATE)_HDDual

BSP_16M = 16M_blocks_script_uboot_flash.txt

# output file name
OUT = sabbat_s3602
SEE_OUT = see_bin

all :
	${MAKE} release
	${MAKE} -f Makefile.cmd all
#	cd ${ROOT}/obj ;\
#	cp -f libsee.a ../../Elephant/ddk;\
#	cp -f boot.o ../../Elephant/ddk/boot_see.o;\
#	cp -f ../ddk/plugin/*.o ../../Elephant/ddk/plugin

path:	
	./getpath.sh

config_3921_release : 
	cp -f compiler_see_linux.def ${ROOT}/src/compiler.def
		
ui :
	${MAKE} -f Makefile.cmd all

config_linux :  compiler_3602.def sys_config_3602.h
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3701C_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3701C_IN_USE = 1">>${ROOT}/src/compiler.def
	
config_linux_3603 :  compiler_3602.def sys_config_3602.h
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	
config_linux_3921 :  compiler_3602.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def	
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DALSA_SUPPORT">>${ROOT}/src/compiler.def
	echo "ALSA_SUPPORT = 1">>${ROOT}/src/compiler.def
	
config_linux_3921_bootmedia :  compiler_3602.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DALSA_SUPPORT">>${ROOT}/src/compiler.def
	echo "ALSA_SUPPORT = 1">>${ROOT}/src/compiler.def
	
config_linux_3921_linux :  compiler_3701c_linux.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DALSA_SUPPORT">>${ROOT}/src/compiler.def
	echo "ALSA_SUPPORT = 1">>${ROOT}/src/compiler.def

config_linux_3921_linux_bootmedia :  compiler_3701c_linux.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__ -DCONFIG_ALI_MBX_AS">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__ -DCONFIG_ALI_MBX_AS">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def		
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DALSA_SUPPORT">>${ROOT}/src/compiler.def
	echo "ALSA_SUPPORT = 1">>${ROOT}/src/compiler.def

config_linux_3921_linux_bootmedia_mbxas :  compiler_3701c_linux.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__ -DCONFIG_ALI_MBX_AS">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def   
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def 
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod 
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__ -DCONFIG_ALI_MBX_AS">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def   
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def            
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_MBX_AS = 1">>${ROOT}/src/compiler.def

config_linux_3921_linux_ubootsee :  compiler_3701c_linux.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def		
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -UBOOT_SEE">>${ROOT}/src/compiler.def
	echo "UBOOT_SEE = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def		
	echo "DEFS  += -DCONFIG_DE_M3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_M3921 = 1">>${ROOT}/src/compiler.def		
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	

config_linux_s3921 :  compiler_3602.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def	
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DALSA_SUPPORT">>${ROOT}/src/compiler.def
	echo "ALSA_SUPPORT = 1">>${ROOT}/src/compiler.def

config_linux_s3921_bootmedia :  compiler_3602.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DALSA_SUPPORT">>${ROOT}/src/compiler.def
	echo "ALSA_SUPPORT = 1">>${ROOT}/src/compiler.def
	
config_linux_s3921_linux :  compiler_3701c_linux.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DALSA_SUPPORT">>${ROOT}/src/compiler.def
	echo "ALSA_SUPPORT = 1">>${ROOT}/src/compiler.def

config_linux_s3921_linux_bootmedia :  compiler_3701c_linux.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def		
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DALSA_SUPPORT">>${ROOT}/src/compiler.def
	echo "ALSA_SUPPORT = 1">>${ROOT}/src/compiler.def

config_alsa :  
	echo "DEFS  += -DALSA_SUPPORT">>${ROOT}/src/compiler.def
	echo "ALSA_SUPPORT = 1">>${ROOT}/src/compiler.def

config_as :  
	echo "DEFS  += -D_DISABLE_PRINTF">>${ROOT}/src/compiler.def

config_linux_s3921_linux_ubootsee :  compiler_3701c_linux.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def		
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3921_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3921_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -UBOOT_SEE">>${ROOT}/src/compiler.def
	echo "UBOOT_SEE = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def		
	echo "DEFS  += -DCONFIG_DE_S3921 -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_DE_S3921 = 1">>${ROOT}/src/compiler.def		
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def	
	
config_linux_3701c:compiler_3602.def sys_config_3602.h
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3701C_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3701C_IN_USE = 1">>${ROOT}/src/compiler.def

config_linux_3701c_RPCng:compiler_3602.def sys_config_3602.h RPCng_cygwin
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def		
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3701C_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3701C_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def		
	
config_linux_3503 : compiler_3503_linux.def sys_config_3602.h RPCng_cygwin
	cp -Rf ${ROOT}/../../alicommon/alidefinition ${ROOT}/inc
	sed '/#Add chip definition:/a\ _M3503_ = 1' compiler_3503_linux.def > tmp_compiler_default.def
	cp -f tmp_compiler_default.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3503_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3503_IN_USE = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod
#	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	sed -e 's/#define SYS_CPU_MODULE				CPU_ALI_V1/#define SYS_CPU_MODULE				CPU_MIPS24KE/' sys_config_3602.h > ${ROOT}/inc/sys_config.h
	cp -f compiler_3503_linux.def ${ROOT}/src/compiler.def
	rm -f tmp_compiler_default.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3503_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3503_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__ -DCONFIG_ALI_MBX_AS">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	

config_linux_3503_bootmedia : compiler_3503_linux.def sys_config_3602.h RPCng_cygwin
	cp -Rf ${ROOT}/../../alicommon/alidefinition ${ROOT}/inc
	sed '/#Add chip definition:/a\ _M3503_ = 1' compiler_3503_linux.def > tmp_compiler_default.def
	cp -f tmp_compiler_default.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3503_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3503_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod
#	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	sed -e 's/#define SYS_CPU_MODULE				CPU_ALI_V1/#define SYS_CPU_MODULE				CPU_MIPS24KE/' sys_config_3602.h > ${ROOT}/inc/sys_config.h
	cp -f compiler_3503_linux.def ${ROOT}/src/compiler.def
	rm -f tmp_compiler_default.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3503_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3503_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def

config_linux_3503_bootmedia_as : compiler_3503_linux.def sys_config_3602.h RPCng_cygwin
	cp -Rf ${ROOT}/../../alicommon/alidefinition ${ROOT}/inc
	sed '/#Add chip definition:/a\ _M3503_ = 1' compiler_3503_linux.def > tmp_compiler_default.def
	cp -f tmp_compiler_default.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3503_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3503_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod
#	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	sed -e 's/#define SYS_CPU_MODULE				CPU_ALI_V1/#define SYS_CPU_MODULE				CPU_MIPS24KE/' sys_config_3602.h > ${ROOT}/inc/sys_config.h
	cp -f compiler_3503_linux.def ${ROOT}/src/compiler.def
	rm -f tmp_compiler_default.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3503_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3503_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DCONFIG_ALI_RPCNG -D__ALI_TDS__ -DCONFIG_ALI_MBX_AS">>${ROOT}/src/compiler.def
	echo "CONFIG_ALI_RPCNG = 1">>${ROOT}/src/compiler.def	
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def
	
config_linux_3503_upg : compiler_3503_linux.def sys_config_3602.h
	sed '/#Add chip definition:/a\ _M3503_ = 1\n_UPG_SEE_ = 1' compiler_3503_linux.def > tmp_compiler_default.def
	cp -f tmp_compiler_default.def ${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod
#	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	sed -e 's/#define SYS_CPU_MODULE				CPU_ALI_V1/#define SYS_CPU_MODULE				CPU_MIPS24KE/' sys_config_3602.h > ${ROOT}/inc/sys_config.h
	cp -f compiler_3503_linux.def ${ROOT}/src/compiler.def
	rm -f tmp_compiler_default.def
	echo "DEFS  += -DCHIP_3503_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3503_IN_USE = 1">>${ROOT}/src/compiler.def

	
config_3821 :  compiler_3821.def sys_config_3602.h
	cp -f compiler_3821.def ${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3821_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3821_IN_USE = 1">>${ROOT}/src/compiler.def

	
config_3821_ota :  compiler_3821.def sys_config_3602.h
	cp -f compiler_3821.def ${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo -e "\n_BUILD_OTA_E_ = 1" >> ${ROOT}/src/compiler.def
	echo "DEFS += -D_BUILD_OTA_E_" >> ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3821_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3821_IN_USE = 1">>${ROOT}/src/compiler.def	

	
config_see:compiler_see.def
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3701C_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3701C_IN_USE = 1">>${ROOT}/src/compiler.def
	
config_linux_3701c_linux:compiler_3701c_linux.def sys_config_3602.h
	cp -f compiler_3701c_linux.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod	
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see_linux.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3701C_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3701C_IN_USE = 1">>${ROOT}/src/compiler.def
	
config_linux_bootmedia:compiler_3602.def sys_config_3602.h
	cp -f compiler_3602.def ${ROOT}/src/compiler.def
	echo "SUPPORT_LINUX = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def
	${MAKE} -f Makefile.cmd module_config_linux
	cp -f modules_cpu.c ${ROOT}/src/hld/mod
	cp -f modules_see.c ${ROOT}/src/hld/mod		
	cp -f sys_config_3602.h ${ROOT}/inc/sys_config.h
	cp -f compiler_see.def ${ROOT}/src/compiler.def
	echo "DEFS  += -DCHIP_3701C_IN_USE">>${ROOT}/src/compiler.def
	echo "CHIP_3701C_IN_USE = 1">>${ROOT}/src/compiler.def
	echo "DEFS  += -DBOOT_MEDIA">>${ROOT}/src/compiler.def
	echo "BOOT_MEDIA = 1">>${ROOT}/src/compiler.def
		
config_nmp_cost_down:
	${MAKE} -f Makefile.cmd config_nmp_cost_down
	echo "DEFS  += -DDISABLE_OLD_MP">>${ROOT}/src/compiler.def
	echo "DEFS  += -DDISABLE_DVB">>${ROOT}/src/compiler.def
	echo "DISABLE_DVB = 1">>${ROOT}/src/compiler.def
	
plugin_cost_down:
	${MAKE} -f Makefile.cmd plugin_cost_down	

plugin_down_4_linux_build:
	${MAKE} -f Makefile.cmd plugindown_4_linux_build

config_power_down :  
	echo "DEFS  += -DPOWER_DOWN">>${ROOT}/src/compiler.def
	echo "POWER_DOWN = 1">>${ROOT}/src/compiler.def

clean :
	${MAKE} -f Makefile.cmd clean

new :
	${MAKE} -f Makefile.cmd new

release :
	${MAKE} -f Makefile.cmd release

sdk_release :
  
	${MAKE} -f Makefile.cmd sdk_release	

generate_see  :
	${MAKE} -f Makefile.cmd generate_see	

generate_see_linux  :
	${MAKE} -f Makefile.cmd generate_see
	
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
	${OBJCOPY} -R .flash -O binary ${SEE_OUT}.out ${SEE_OUT}; \
	${OBJCOPY} -j .flash -O binary ${SEE_OUT}.out see_flash.sec; \
	rm -f ${OUT}.7z; \
	../prj/tools/lzma e ${OUT} ${OUT}.7z -lc0 -lp2
	rm -f ${SEE_OUT}.7z; \
	../prj/tools/lzma e ${SEE_OUT} ${SEE_OUT}.7z -lc0 -lp2

generate_7z :
	rm -f app_${OUT}.bin
	cp -f ${OUT}.7z ${ROOT_PRJ}/app.bin
	cp -f ${SEE_OUT}.7z ${ROOT_PRJ}/app_see.bin
	cd ${ROOT_PRJ}; \
		../../../../tools/fidmerger.exe ${BSP}

# Link file
link :
	${LD} ${LDFLAGS} $(shell ls $(PLUGINDDK_DIR)/*.o ) -o ${SEE_OUT}.out -Map ${SEE_OUT}.map \
	  -T see_ldscript.ld ${LIB} $(DDK_DIR)/libHDMI.a $(DDK_DIR)/libsee.a \
	    $(DDK_DIR)/libmpg2c.a liblog.a\
	    -lm -lgcc -lc -le
	${OBJCOPY} -O binary ${SEE_OUT}.out ${SEE_OUT}.abs

# */

see_link :
	rm -f ${SEE_OUT}.out
	rm -f ${SEE_OUT}.abs
ifeq ($(LINUX_BUILD),1)
	${LD} ${LDFLAGS} $(shell ls $(PLUGINDDK_DIR)/*.o ) -o ${SEE_OUT}.out -Map ${SEE_OUT}.map \
	  -T see_ldscript.ld ${LIB} -L$(DDK_DIR) $(DDK_DIR)/libsee.a \
	    $(DDK_DIR)/libmpg2c.a $(DDK_DIR)/liblog.a\
	    -lHDMI -lm -lgcc -lc
else	
	${LD} ${LDFLAGS} $(shell ls $(PLUGINDDK_DIR)/*.o ) -o ${SEE_OUT}.out -Map ${SEE_OUT}.map \
	  -T see_ldscript.ld ${LIB} -L$(DDK_DIR) $(DDK_DIR)/libsee.a \
	    $(DDK_DIR)/libmpg2c.a \
	    -lHDMI -lm -lgcc -lc -le
endif	    
	${OBJCOPY} -O binary ${SEE_OUT}.out ${SEE_OUT}.abs
	#cp -f ${SEE_OUT}.abs /home/sam/PDK1.5_20131122/PDK/linux/prebuildimages/alisee/C3921
	#cp -f ${SEE_OUT}.out  /home/sam/PDK1.5_20131122/PDK/linux/prebuildimages/alisee/C3921
	${OBJDUMP} -D ${SEE_OUT}.out > ${SEE_OUT}.dis
	#cp -f ${SEE_OUT}.dis /cygdrive/x/s3921_20130803/sct/pdk_s3921/snake/alisee/m3921	
	cp -f ${SEE_OUT}.abs ${ROOT_PRJ}
#	cp -f ${SEE_OUT}.out /cygdrive/x/demo/kernel
#	cp -f ${SEE_OUT}.out ../../main
	
cleanld :
	rm -f ${OUT}.out ${OUT}.map ${OUT}.dis ${OUT}.abs libstb.a \
	  ${OUT} flash.sec ${OUT}.gz ${OUT}.7z app_${OUT}.bin
pplus:
	${MAKE} -f Makefile.cmd pplus

pplus_clean:
	${MAKE} -f Makefile.cmd pplus_clean
	
label :
	./updatelabel.bat $(RLS_NAME) > label_log.txt

RPCng_cygwin :
	cp -f ${ROOT}/../../alicommon/alirpcng/inc/* ${ROOT}/inc/alirpcng
	cp -f ${ROOT}/../../alicommon/alirpcng/libmcapi/* ${ROOT}/src/lib/libmcapi
	cp -f ${ROOT}/../../alicommon/alirpcng/rpc/* ${ROOT}/src/lld/rpc
	cp -Rf ${ROOT}/../../alicommon/alidefinition ${ROOT}/inc
	
RPCng_dos :
	dos2unix -f ${ROOT}/inc/alirpcng/*
	dos2unix -f ${ROOT}/src/lib/libmcapi/*
	dos2unix -f ${ROOT}/src/lld/rpc/*

