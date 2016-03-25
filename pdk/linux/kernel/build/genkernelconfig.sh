#!/bin/bash


############################################################
# 	Functions
############################################################


general_setup ()
{
	SEDCMD="$SEDCMD -e 's/# "$1" is not set/"$1"="$2"/'"
}


#. $1
if [ "$ARCH" = "mips" ]; then
export INITRAMFS=$LINUX_SRC_DIR/fsmodules/build_mips/fs.init
$LINUX_SRC_DIR/fsmodules/build_mips/buildinitramfs.sh
else
export INITRAMFS=$LINUX_SRC_DIR/fsmodules/build/fs.init
$LINUX_SRC_DIR/fsmodules/build/buildinitramfs.sh
fi
SEDCMD="$SEDCMD -e 's:# CONFIG_INITRAMFS_SOURCE is not set:CONFIG_INITRAMFS_SOURCE=\"$INITRAMFS\":'"
gen="sed $SEDCMD $TARGET_DIR/$BOARD/defkernelconfig_\$LINUX_KERNEL_VER\-template"
eval $gen > $TARGET_DIR/$BOARD/defkernelconfig_$LINUX_KERNEL_VER


