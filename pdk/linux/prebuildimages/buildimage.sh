#!/bin/sh

BUILD_FLASHIMG_RET=0
HOSTTOOLS_DIR=$LINUX_SRC_DIR/../tools/hosttools
IMAGES_DIR=$LINUX_SRC_DIR/install/bin
MKIMAGE_FILE=mkimage
LZMA_FILE=lzma
FIDMERGER_FILE=fidmerger
MKYAFFS2IMAGE_FILE=mkyaffs2image
LZOX99_FILE=lzox99
MKIMAGE_YAFFS2_SH_FILE=mkimage_yaffs2.sh
MKIMAGE_CRAMFS_YAFFS2_SH_FILE=mkimage_cramfs_yaffs2.sh
MKIMAGE_UBIFS_SH_FILE=mkimage_ubifs.sh

echo "Input ROOTFSTYPE: $ROOTFSTYPE"
if [ "$ROOTFSTYPE" = "ubifs" ]; then
  DEV_ROOTFS_UBIFS=y
elif [ "$ROOTFSTYPE" = "yaffs2" ]; then
  DEV_ROOTFS_YAFFS2=y
elif [ "$ROOTFSTYPE" = "cramfs_yaffs2" ]; then
  DEV_ROOTFS_CRAMFS_YAFFS2=y  
elif [ "$ROOTFSTYPE" = "ext4" ]; then
  DEV_ROOTFS_EXT4=y
else
  DEV_ROOTFS_UBIFS=y
fi

# use shell funcs
. $HOSTTOOLS_DIR/funcs.sh

# check image
echo 
echo " --- start to build image --- "

  # build main_bin image
  echo
  echo " --- start to generate .ubo image (main/see...)"
  # check "$CP $LINUX_DIR/vmlinux $IMAGES_DIR/main_bin.out"
  cd $IMAGES_DIR  
  if [ "$BOARD" = "M3515B" ]; then
  	check "$HOSTTOOLS_DIR/mkimage -A mips -O linux -C none  -T kernel -a 0x800FFFC0 -e 0x80100000 -n 'linux-3.12.20' -d main_bin.abs main_bin.ubo"
  else
  	check "$HOSTTOOLS_DIR/mkimage -A arm -O linux -C none  -T kernel -a 0x80007fc0 -e 0x80008000 -n 'Linux-3.4.0-rc3S3921' -d main_bin.abs main_bin.ubo"
  fi
  check "$HOSTTOOLS_DIR/mkimage -A mips -O linux -T kernel -C none -a 0x840001c0 -e 0x84000200 -n see -d see_bin.abs see_bin.ubo"
  if [ -f ae_bin.abs ]; then 
    check "$HOSTTOOLS_DIR/mkimage -A mips -O linux -T kernel -C none -a 0x85EFD1c0 -e 0x85EFD200 -n AudCode -d ae_bin.abs ae_bin.ubo"
  fi
  
  echo " --- genereate .ubo done!!"
  if [ "$FULL" = "" ]; then
  	echo "Donn't need to build rootfs image"
	exit 0
  fi
  
  # build other flash image
  echo

  echo " --- start to post adjust rootfs files"
  cp -rfp $LINUX_SRC_DIR/kernel/alidrivers/modules/alimmc/*.ko $LINUX_SRC_DIR/install/fs/
  cp -rfp $LINUX_SRC_DIR/kernel/alidrivers/modules/alimmc/*.sh $LINUX_SRC_DIR/install/fs/
  cp -rfp $LINUX_SRC_DIR/install/lib/mali/* $LINUX_SRC_DIR/install/fs/
	mkdir -p  $LINUX_SRC_DIR/install/fs/demo/shaders
	cp -rfp $LINUX_SRC_DIR/fsmodules/install/bin/linux_stb $LINUX_SRC_DIR/install/fs/demo/
	cp -rfp $LINUX_SRC_DIR/kernel/thirddrivers/mali/Mali_Linux/ali_app/deve_filter_test/shaders/*.glsl $LINUX_SRC_DIR/install/fs/demo/shaders/

rm -rf $LINUX_SRC_DIR/install/lib/*
  rm -rf $LINUX_SRC_DIR/install/include/*
  mkdir -p $LINUX_SRC_DIR/install/lib/hld
  mkdir -p $LINUX_SRC_DIR/install/lib/thirddrivers/mali
  mkdir -p $LINUX_SRC_DIR/install/include/hld
  mkdir -p $LINUX_SRC_DIR/install/include/hld_ibu
  cp -rfp $LINUX_SRC_DIR/pdkapi/install/lib/* $LINUX_SRC_DIR/install/lib/hld/
  cp -rfp $LINUX_SRC_DIR/pdkapi/install/include/* $LINUX_SRC_DIR/install/include/hld/
  cp -rfp $LINUX_SRC_DIR/pdkapi/install/include/* $LINUX_SRC_DIR/install/include/hld_ibu/
  cp -rfp $LINUX_SRC_DIR/kernel/install/lib/mali/lib/* $LINUX_SRC_DIR/install/lib/thirddrivers/mali/
  cp -a $LINUX_SRC_DIR/kernel/install/include/thirddrivers $LINUX_SRC_DIR/install/include/
  ln -s $LINUX_SRC_DIR/kernel/install/include/kernel $LINUX_SRC_DIR/install/include/kernel
  ln -s $LINUX_SRC_DIR/kernel/install/include/alidrivers $LINUX_SRC_DIR/install/include/alidrivers
  
  find $LINUX_SRC_DIR/install/include/hld_ibu/ -name "*.h" | xargs sed -i 's|adr_\([^\\/]*\)\.h|\1\.h|'
  find $LINUX_SRC_DIR/install/include/hld_ibu/ -name "adr_*.h"|sed 's/\(.*\)\(adr_\)\(.*\)/mv \1\2\3 \1\3/'|sh

  echo
  echo " --- start to build rootfs image"

  if [ "$DEV_ROOTFS_YAFFS2" = "y" ]; then

    if [ "$DEV_FLASHBOOT_NOR" = "y" ]; then
      echo "Do wrong menuconfig, please make menuconfig again!"
      BUILD_FLASHIMG_RET=-1
    else
      # build yaffs for rootfs
      echo "build yaffs2 image..."
      if [ -f $HOSTTOOLS_DIR/flashfiles/yaffs2/$MKIMAGE_YAFFS2_SH_FILE ]; then
        $HOSTTOOLS_DIR/flashfiles/yaffs2/$MKIMAGE_YAFFS2_SH_FILE
        BUILD_FLASHIMG_RET=$?
      else
        echo "No mkimage_yaffs2.sh file!"
        BUILD_FLASHIMG_RET=-1
      fi
    fi

  elif [ "$DEV_ROOTFS_CRAMFS_YAFFS2" = "y" ]; then

    if [ "$DEV_FLASHBOOT_NOR" = "y" ]; then
      echo "Do wrong menuconfig, please make menuconfig again!"
      BUILD_FLASHIMG_RET=-1
    else
      # build cramfs for rootfs, and yaffs2 for other fs
      echo "build cramfs_yaffs2 image..."
      if [ -f $HOSTTOOLS_DIR/flashfiles/cramfs_yaffs2/$MKIMAGE_CRAMFS_YAFFS2_SH_FILE ]; then
        $HOSTTOOLS_DIR/flashfiles/cramfs_yaffs2/$MKIMAGE_CRAMFS_YAFFS2_SH_FILE
        BUILD_FLASHIMG_RET=$?
      else
        echo "No mkimage_cramfs_yaffs2.sh file!"
        BUILD_FLASHIMG_RET=-1
      fi
    fi

  elif [ "$DEV_ROOTFS_UBIFS" = "y" ]; then

    if [ "$DEV_FLASHBOOT_NOR" = "y" ]; then
      echo "Do wrong menuconfig, please make menuconfig again!"
      BUILD_FLASHIMG_RET=-1
    else
      # build ubifs for rootfs and other fs
	cp -f $LINUX_SRC_DIR/install/etc/flashfiles/ubifs/* $LINUX_SRC_DIR/install/bin
	echo "build ubifs image..."
	if [ -f $HOSTTOOLS_DIR/$MKIMAGE_UBIFS_SH_FILE ]; then
	  if [ "$BOARD" = "C3921" ] ; then
	    $HOSTTOOLS_DIR/$MKIMAGE_UBIFS_SH_FILE.2kpage
	    $HOSTTOOLS_DIR/$MKIMAGE_UBIFS_SH_FILE.4kpage
	    $HOSTTOOLS_DIR/$MKIMAGE_UBIFS_SH_FILE.16kpage
	  fi
	  $HOSTTOOLS_DIR/$MKIMAGE_UBIFS_SH_FILE
	  BUILD_FLASHIMG_RET=$?
      else
        echo "No mkimage_ubifs.sh file!"
        BUILD_FLASHIMG_RET=-1
      fi
    fi

  elif [ "$DEV_FLASHBOOT_NOR" = "y" ]; then

    if [ "$DEV_BOOTLOADER_UBOOT" = "y" ]; then
      $HOSTTOOLS_DIR/$FIDMERGER_FILE blocks_script_flash.txt
      BUILD_FLASHIMG_RET=$?
    elif [ "$DEV_BOOTLOADER_ALIBOOT" = "y" ]; then
      check "$HOSTTOOLS_DIR/$LZMA_FILE e main_bin.abs main_bin.7z -lc0 -lp2"
      check "$HOSTTOOLS_DIR/$LZMA_FILE e see_bin.abs see_bin.7z -lc0 -lp2"
      check "$HOSTTOOLS_DIR/$FIDMERGER_FILE blocks_script_flash.7z"
      BUILD_FLASHIMG_RET=$?
    fi

  elif [ "$DEV_ROOTFS_NFS" = "y" ]; then

     echo "build nfs..."
     BUILD_FLASHIMG_RET=$?
  
  elif [ "$DEV_ROOTFS_EXT4" = "y" ]; then

    ##added by kinson, generate ext4 rootfs for emmc booting up
    echo "generate rootfs_ext4_emmc.img for eMMC..."
    cd $LINUX_SRC_DIR/install/fs/
    du -bcs $LINUX_SRC_DIR/install/fs|grep "total"|cut -d "	" -f 1 > ../ext4_fs_size.txt
    cd -
    cat $LINUX_SRC_DIR/install/ext4_fs_size.txt
    ext4_fs_size=$(cat $LINUX_SRC_DIR/install/ext4_fs_size.txt)
    ext4_fs_size="`expr "$ext4_fs_size" + 52428800`"  ###reserve 50MB
    ext4_fs_size="`expr "$ext4_fs_size" / 1024`" 
    echo ext4_fs_size is $ext4_fs_size
    $HOSTTOOLS_DIR/genext2fs-1.4.1/bin/genext2fs -b $ext4_fs_size -d $LINUX_SRC_DIR/install/fs/ $LINUX_SRC_DIR/install/bin/rootfs_emmc.img 
    e2fsck -fy $LINUX_SRC_DIR/install/bin/rootfs_emmc.img
    tune2fs -j -O extents,uninit_bg,dir_index $LINUX_SRC_DIR/install/bin/rootfs_emmc.img 
    fsck.ext4 -fy $LINUX_SRC_DIR/install/bin/rootfs_emmc.img
    file $LINUX_SRC_DIR/install/bin/rootfs_emmc.img
    BUILD_FLASHIMG_RET=$?
    echo "generate rootfs_ext4_emmc.img for eMMC booting up done!!---"

  else
    echo "No rootfs select, please input varialbe ROOTFSTYPE when make!"
    BUILD_FLASHIMG_RET=-1
  fi
  
echo "----------------------------------------------------------------------"
