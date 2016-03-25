############################################################################
#
# Define and Export Environment Variables
# NOTICE: Don't DELETE or MODIFY it, but can only ADD new variable.
#
############################################################################


##
## Common variables
##


##
## Path Variables
## Rule: directory name + '_DIR'
##
export ALIBOOT_DIR = $(SRC_DIR)/aliboot
export ALISEE_DIR = $(SRC_DIR)/alisee
export BUILD_DIR = $(SRC_DIR)/build
export EXTERNAL_DIR = $(SRC_DIR)/open_source
export HOSTTOOLS_DIR = $(SRC_DIR)/hosttools
export IMAGES_DIR = $(SRC_DIR)/install
export KERNEL_DIR = $(SRC_DIR)/kernel
export PLATFORM_DIR = $(SRC_DIR)/platform
export RELEASE_DIR = $(SRC_DIR)/release
export TARGETS_DIR = $(SRC_DIR)/boards
export ALISDK_DIR = opt/alisdk
ifneq ($(strip $(KERNELVER)),)
 export LINUX_KERNEL_VER = $(KERNELVER)
else
 export LINUX_KERNEL_VER = linux-linaro-3.4-rc3
endif

##
## Sub Tree Variables
## Rule: directory name + '_[parent dir]' + '_SUBDIR'
##

# build subdir
export BASEFS_BUILD_SUBDIR = $(BUILD_DIR)/basefs
export MISC_BUILD_SUBDIR = $(BUILD_DIR)/misc
export OTTFS_BUILD_SUBDIR = $(BUILD_DIR)/ottfs

# hosttools subdir
export MKLIBS_HT_SUBDIR = $(HOSTTOOLS_DIR)/mklibs
export SCRIPTS_HT_SUBDIR = $(HOSTTOOLS_DIR)/scripts

# images subdir
export FS_IMAGES_SUBDIR = $(IMAGES_DIR)/fs
export MODULE_IMAGES_SUBDIR = $(IMAGES_DIR)/module
export INSTALL_MOD_PATH = $(IMAGES_DIR)/module
export INSTALLFS_IMAGES_SUBDIR = $(IMAGES_DIR)/fs.install
export APPFS_IMAGES_SUBDIR = $(IMAGES_DIR)/fs.app
export RSCFS_IMAGES_SUBDIR = $(IMAGES_DIR)/fs.rsc
export DBFS_IMAGES_SUBDIR = $(IMAGES_DIR)/fs.db
export UIFS_IMAGES_SUBDIR = $(IMAGES_DIR)/fs.ui
export ROOTFS_IMAGES_SUBDIR = $(IMAGES_DIR)/fs.root
export OUTPUT_IMAGES_SUBDIR = $(IMAGES_DIR)/output
 
# kernel subdir
export LINUX_KERNEL_SUBDIR = $(KERNEL_DIR)/build/$(LINUX_KERNEL_VER)
#export LINUX_KERNEL_SUBDIR = $(KERNEL_DIR)/linux
export ALIDRIVERS_KERNEL_SUBDIR = $(SRC_DIR)/alidrivers/modules

# platform subdir
export APP_PF_SUBDIR = $(PLATFORM_DIR)/tools
export BINLIB_PF_SUBDIR = $(PLATFORM_DIR)/bin_lib
export INC_PF_SUBDIR = $(PLATFORM_DIR)/inc
export OBJ_PF_SUBDIR = $(PLATFORM_DIR)/obj
export SRC_PF_SUBDIR = $(PLATFORM_DIR)/src

# targets subdir
export BOARD_TARGETS_SUBDIR = $(TARGETS_DIR)/$(BOARD)


 
##
## Friendly name
##
export BASEFS_DIR = $(BASEFS_BUILD_SUBDIR)
export IMAGE_FS_DIR = $(FS_IMAGES_SUBDIR)
export IMAGE_APPFS_DIR = $(APPFS_IMAGES_SUBDIR)
export IMAGE_RSCFS_DIR = $(RSCFS_IMAGES_SUBDIR)
export IMAGE_DBFS_DIR = $(DBFS_IMAGES_SUBDIR)
export IMAGE_UIFS_DIR = $(UIFS_IMAGES_SUBDIR)
export IMAGE_ROOTFS_DIR = $(ROOTFS_IMAGES_SUBDIR)
export IMAGE_MOD_DIR = $(MODULE_IMAGES_SUBDIR)
export INSTALL_DIR = $(INSTALLFS_IMAGES_SUBDIR)
export OUTPUT_DIR = $(OUTPUT_IMAGES_SUBDIR)
export INSTALL_USER_DIR = $(INSTALL_DIR)/usr
export INSTALL_OUTPUT_DIR = $(INSTALL_DIR)/$(ALISDK_DIR)

export LINUX_DIR = $(LINUX_KERNEL_SUBDIR)
export ALIDRIVERS_DIR = $(ALIDRIVERS_KERNEL_SUBDIR)
export BOARD_DIR = $(BOARD_TARGETS_SUBDIR)

export ALISDK_INC_DIR = $(OUTPUT_DIR)/include
export ALISDK_LIB_DIR = $(OUTPUT_DIR)/lib



##
## Important file name
## rule: [file name] + _FILE
##
export ALI_BOOT_BIN_FILE = loader3.bin

export ALI_SEE_BIN_OUT_FILE = see_bin.out
export ALI_SEE_BIN_ABS_FILE = see_bin.abs

export BUILDFS_SH_FILE = buildfs.sh
export SEPARATEFS_SH_FILE = separatefs.sh
export BUILDIMAGE_SH_FILE = buildimage.sh
export BUILDLOADERIMAGE_SH_FILE = buildloaderimage.sh
export CONFIG_IN_FILE = config.in
#export DEFKERNELCONFIG_FILE = defkernelconfig
export GENKERNELCONFIG_SH_FILE = genkernelconfig.sh
export MACROS_MAK_FILE = macros.mak
export VARIABLES_MAK_FILE = variables.mak
export VERSION_MAK_FILE = version.mak
export LAST_BOARD_FILE = .last_board

export MKIMAGE_FILE = mkimage
export LZMA_FILE = lzma
export FIDMERGER_FILE = fidmerger
export CONFIG_FILE = config
export MENUCONFIG_FILE = menuconfig
export MKYAFFS2IMAGE_FILE = mkyaffs2image 
export LZOX99_FILE = lzox99

export BOARDCONFIG_VAR_FILE = boardconfig.var
export BOARDCONFIG_DEF_FILE = boardconfig.def
export BOARDCONFIG_CON_FILE = boardconfig.con
export POSTBUILD_SH_FILE = postbuild.sh
export PREBUILD_SH_FILE = prebuild.sh
export MKIMAGE_YAFFS2_SH_FILE = mkimage_yaffs2.sh
export MKIMAGE_CRAMFS_YAFFS2_SH_FILE = mkimage_cramfs_yaffs2.sh
export MKIMAGE_UBIFS_SH_FILE = mkimage_ubifs.sh
export IMAGEPROCESS_SH_FILE = imageprocess.sh

export APP_MAKEFILE_FILE = Makefile.app


##
## Misc Variables
##
export HOST_KERNEL_VERSION := $(shell uname -r | sed -e 's/\([0-9]\.[0-9]\+\).*/\1/')
export LAST_BOARD_COOKIE := $(BUILD_DIR)/$(LAST_BOARD_FILE)
export LAST_BOARD        := $(strip $(shell cat $(LAST_BOARD_COOKIE) 2>/dev/null))
ifeq ($(strip $(BOARD)),)
export BOARD = $(LAST_BOARD)
endif



##
## Useful Tool Variables
##
export CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          else if [ -x /bin/bash ]; then echo /bin/bash; \
          else echo sh; fi ; fi)
export MKDIR = mkdir -p
export RM	 = rm -f
export RMDIR = rm -rf
export CP	 = cp -af
export CPDIR = cp -raf
export SLINK = ln -sf
export HLINK = ln -f
export RMALL = $(RMDIR)
export CPALL = $(CPDIR)
export MKNOD = mknod
export CHMOD = chmod
export CHOWN = chown
export USER = $(shell whoami)



##
## Toolchain Variables
##
export DEV_CROSS_COMPILE_PATH = /opt/arm-linux-gnueabi

ifneq ($(strip $(TOOLCHAIN_PREFIX)),)
  export CROSS_COMPILE = $(TOOLCHAIN_PREFIX)
else
  export CROSS_COMPILE   = $(DEV_CROSS_COMPILE_PATH)/bin/arm-linux-gnueabi-
endif

export AR              = $(CROSS_COMPILE)ar
export AS              = $(CROSS_COMPILE)as
export LD              = $(CROSS_COMPILE)ld
export CC              = $(CROSS_COMPILE)gcc
export CXX             = $(CROSS_COMPILE)g++
export CPP             = $(CROSS_COMPILE)cpp
export NM              = $(CROSS_COMPILE)nm
export STRIP           = $(CROSS_COMPILE)strip
export SSTRIP          = $(CROSS_COMPILE)sstrip
export OBJCOPY         = $(CROSS_COMPILE)objcopy
export OBJDUMP         = $(CROSS_COMPILE)objdump
export RANLIB          = $(CROSS_COMPILE)ranlib
export SIZE            = $(CROSS_COMPILE)size



##
## Shell functions
##
export SHELL_FUNCS=. $(BUILD_DIR)/funcs.sh
export SH_PANIC=$(SHELL_FUNCS) && panic
export SH_COPY=$(SHELL_FUNCS) && copy
export SH_CHECK=$(SHELL_FUNCS) && check
export SH_CHECKIT=$(SHELL_FUNCS) && checkit
