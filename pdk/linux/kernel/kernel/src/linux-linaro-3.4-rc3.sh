#!/bin/bash

# patch kernel arch related files
cd $LINUX_DIR/arch/arm
ln -sf ../../drivers/alidrivers/modules/aliarch/arm/mach-ali3921 mach-ali3921
cd $LINUX_DIR/arch/arm/include/asm
ln -sf ../../../../drivers/alidrivers/include/mach-ali mach-ali

# check if exist patch files and try to apply its

exit 0
