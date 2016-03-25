# Here defines all macro definition for development

ifneq ($(strip $(BOARD)),)


##
## All macro definitions
##
export DEV_MACRO_FLAGS := $(shell if [ -f $(BOARD_DIR)/$(BOARDCONFIG_DEF_FILE) ]; then \
			sed -e "s/^/-D/g" $(BOARD_DIR)/$(BOARDCONFIG_DEF_FILE) | \
			awk '{print $0}' | tr '\r\n' ' '; fi)


##
## All macro const
##
export DEV_CONST_FLAGS := $(shell if [ -f $(BOARD_DIR)/$(BOARDCONFIG_CON_FILE) ]; then \
			sed -e "s/^/-D/g" $(BOARD_DIR)/$(BOARDCONFIG_CON_FILE) | \
			awk '{print $0}' | tr '\r\n' ' '; fi)



##
## ALi and customer macro for application
##
export ALI_CFLAGS = $(DEV_MACRO_FLAGS) $(DEV_CONST_FLAGS) $(CUSTOM_FLAGS)



##
## CFLAGS for application
##
ifneq ($(strip $(DEV_MIPS_UCLIBC)),)
	ifneq ($(strip $(MIPS_LINUX_GNU_GCC_4_4_1)),)
	export COMMON_CFLAGS = -g -O1 -muclibc -EL -fPIC -mips32r2
	export COMMON_CXXFLAGS = -g -O1 -muclibc -EL -fPIC -mips32r2
	endif
	
	ifneq ($(strip $(ARM_LINUX_GNUEABI_GCC_4_6_3)),)
	export COMMON_CFLAGS = -g -O1 -muclibc -EL -fPIC
	export COMMON_CXXFLAGS = -g -O1 -muclibc -EL -fPIC
	endif
else
	ifneq ($(strip $(MIPS_LINUX_GNU_GCC_4_4_1)),)
	export COMMON_CFLAGS = -g -O1 -EL -fPIC -mips32r2
	export COMMON_CXXFLAGS = -g -O1 -EL -fPIC -mips32r2
	endif
	
	ifneq ($(strip $(ARM_LINUX_GNUEABI_GCC_4_6_3)),)
	export COMMON_CFLAGS = -g -O1 -EL -fPIC
	export COMMON_CXXFLAGS = -g -O1 -EL -fPIC
	endif
endif

ifneq ($(strip $(DEV_MIPS_UCLIBC)),)
export INC_BASELIBS_CFLAGS = -I$(DEV_CROSS_COMPILE_PATH)/mips-linux-gnu/libc/uclibc/usr/include/ -I$(ALISDK_INC_DIR)
else
export INC_BASELIBS_CFLAGS = -I$(DEV_CROSS_COMPILE_PATH)/mips-linux-gnu/libc/usr/include/ -I$(ALISDK_INC_DIR)
endif



##
## LDFLAGS for application
##
ifneq ($(strip $(SUPPORT_DYNAMIC_LIBS)),)
	ifneq ($(strip $(MIPS_LINUX_GNU_GCC_4_4_1)),)
	export COMMON_LDFLAGS = -g -O1 -EL -fPIC -mips32r2 -L$(ALISDK_LIB_DIR)
	endif

	ifneq ($(strip $(ARM_LINUX_GNUEABI_GCC_4_6_3)),)
	export COMMON_LDFLAGS = -g -O1 -EL -fPIC -L$(ALISDK_LIB_DIR)
	endif
else
	ifneq ($(strip $(MIPS_LINUX_GNU_GCC_4_4_1)),)
	export COMMON_LDFLAGS = -static -g -O1 -EL -mips32r2 -L$(ALISDK_LIB_DIR)
	endif

	ifneq ($(strip $(ARM_LINUX_GNUEABI_GCC_4_6_3)),)
	export COMMON_LDFLAGS = -static -g -O1 -EL -L$(ALISDK_LIB_DIR)
	endif
endif



##
## CFLAGS for kernel and driver
##
export ALI_KERNEL_CFLAGS = -DALI_CHIPPF_$(ALI_CHIP_PLATFORM)
ALI_KERNEL_CFLAGS += -DALI_CHIPID_$(ALI_CHIP_ID) -DALI_IC_$(ALI_IC_REV)
ALI_KERNEL_CFLAGS += -DALI_BOARDTYPE_$(ALI_BOARD_TYPE) -DALI_BOARDID_$(ALI_BOARD_ID)
ALI_KERNEL_CFLAGS += -DDEV_FLASHBOOT_$(DEV_FLASHBOOT)
ALI_KERNEL_CFLAGS += -D__ALI_LINUX_KERNEL__
ALI_KERNEL_CFLAGS += $(KERNEL_CFLAGS)
ALI_KERNEL_CFLAGS += -D__NIM_LINUX_PLATFORM__

endif

