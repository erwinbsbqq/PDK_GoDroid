#!/bin/sh

# use shell funcs
. $BUILD_DIR/funcs.sh


BUILD_FLASHIMG_RET=0

## Check if S3921
#if [ "$ALI_CHIPID_S3921" = "y" ]; then
#	echo "S3921 buildimage being ported, TBD..."
#	exit 0
#fi

# remake kernel for initrd ramdisk fs changed
if [ "$DEV_ROOTFS_INITRD" = "y" ]; then
  if [ -f $LINUX_DIR/.config ]; then
	$RM $LINUX_DIR/usr/*.cpio.gz
    cd $LINUX_DIR; 
    if [ "$LOADER_BUILD" = "y" ]; then
      echo "make loader"
      make SEEINKERNEL=1
    else
      echo "make main app"
      make
    fi
  else
    echo "***********************************************************"
	echo "ERROR: Your kernel config does not exist!!"
	echo "***********************************************************"
	echo "Please do 'make BOARD=$BOARD' or 'make BOARD=$BOARD kernel' firstly"
	exit 1
  fi
fi

# check image
echo 
echo " --- start to build image --- "

if [ "$LOADER_BUILD" = "y" ]; then
  # build loader_bin image
  echo
  echo " --- start to generate loader_bin"
  check "$CP $LINUX_DIR/vmlinux $IMAGES_DIR/loader_bin.out"
  cd $IMAGES_DIR
  check "$OBJCOPY -O binary loader_bin.out loader_bin.abs"
  check "$HOSTTOOLS_DIR/mkimage -A mips -O linux -T kernel -C none -a 800fffc0 -e 80100000 -n loader -d loader_bin.abs loader_bin.ubo"
  $RM loader_bin.abs
  echo " --- genereate loader_bin done!!"
else
  # build main_bin image
  echo
  echo " --- start to generate main_bin(kernel)"
  check "$CP $LINUX_DIR/vmlinux $IMAGES_DIR/main_bin.out"
  cd $IMAGES_DIR  
  check "$OBJCOPY -O binary main_bin.out main_bin.abs"
  check "$HOSTTOOLS_DIR/mkimage -A arm -O linux -C none  -T kernel -a 0x80007fc0 -e 0x80008000 -n 'Linux-3.4.0-rc3S3921' -d main_bin.abs main_bin.ubo"
  check "$HOSTTOOLS_DIR/mkimage -A mips -O linux -T kernel -C none -a 0x840001c0 -e 0x84000200 -n see -d see_bin.abs see_bin.ubo"
  check "$HOSTTOOLS_DIR/mkimage -A mips -O linux -T kernel -C none -a 0x85EFD1c0 -e 0x85EFD200 -n AudCode -d ae_bin.abs ae_bin.ubo"
  echo " --- genereate main_bin done!!"

  # build other flash image
  echo

  echo " --- start to build flash image"

  if [ "$DEV_ROOTFS_YAFFS2" = "y" ]; then

    if [ "$DEV_FLASHBOOT_NOR" = "y" ]; then
      echo "Do wrong menuconfig, please make menuconfig again!"
      BUILD_FLASHIMG_RET=-1
    else
      # build yaffs for rootfs
      echo "build yaffs2 image..."
      if [ -f $BOARD_DIR/flashfiles/yaffs2/$MKIMAGE_YAFFS2_SH_FILE ]; then
        $BOARD_DIR/flashfiles/yaffs2/$MKIMAGE_YAFFS2_SH_FILE
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
      if [ -f $BOARD_DIR/flashfiles/cramfs_yaffs2/$MKIMAGE_CRAMFS_YAFFS2_SH_FILE ]; then
        $BOARD_DIR/flashfiles/cramfs_yaffs2/$MKIMAGE_CRAMFS_YAFFS2_SH_FILE
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
      echo "build ubifs image..."
      if [ -f $BOARD_DIR/flashfiles/ubifs/$MKIMAGE_UBIFS_SH_FILE ]; then
        $BOARD_DIR/flashfiles/ubifs/$MKIMAGE_UBIFS_SH_FILE.2kpage
        $BOARD_DIR/flashfiles/ubifs/$MKIMAGE_UBIFS_SH_FILE.4kpage
        $BOARD_DIR/flashfiles/ubifs/$MKIMAGE_UBIFS_SH_FILE
        BUILD_FLASHIMG_RET=$?
      else
        echo "No mkimage_cramfs_yaffs2.sh file!"
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

  else
    echo "No rootfs select, please make menuconfig again!"
    BUILD_FLASHIMG_RET=-1
  fi

  # backup linux_stb binary file with debug info
  #$CP $PLATFORM_DIR/obj/linux_stb ./linux_stb.dbg
fi

# regenerate ini file to avoid to restart WinGDB when burn image again.

if [ "$LOADER_BUILD" = "y" ]; then
  # useful variables
  TIME_FLAG=`date +%m%d_%H%M%S`
  INI_FILE="$ALI_CHIP_ID""_burn_ramdisk.ini"
  FILE0_NAME="loader_bin-$TIME_FLAG.out" 


  # save previous files for backup
  rm -f loader_bin-*_*.out
  cp -raf loader_bin.out $FILE0_NAME
  rm -f $INI_FILE

else
  # useful variables
  TIME_FLAG=`date +%m%d_%H%M%S`
  INI_FILE="$ALI_CHIP_ID""_burn_ramdisk.ini"
  FILE0_NAME="main_bin-$TIME_FLAG.out" 
  FILE1_NAME="see_bin.out"

  # save previous files for backup(Fix WinGDB bug)
  $RM main_bin-*_*.out
  $CP main_bin.out $FILE0_NAME

fi

# generate ini file
echo "[Project]" > $INI_FILE
echo "LinuxSupport=1">>$INI_FILE
echo "RunMode=0" >> $INI_FILE
echo "InitMode=0" >> $INI_FILE
echo "RunAddr=0xa0100000" >> $INI_FILE

if [ "$LOADER_BUILD" = "y" ]; then
  echo "FileNum=1" >> $INI_FILE
else
  echo "FileNum=2" >> $INI_FILE
fi

echo "[File0]" >> $INI_FILE
echo "File=$FILE0_NAME" >> $INI_FILE
echo "Proc=kernel">>$INI_FILE
echo "Type=5" >> $INI_FILE
echo "Addr=AUTO" >> $INI_FILE

if [ "$LOADER_BUILD" != "y" ]; then
  echo "[File1]" >> $INI_FILE
  echo "File=$FILE1_NAME" >> $INI_FILE
  echo "Type=6" >> $INI_FILE
  echo "Addr=0xa4000200" >> $INI_FILE
fi

#if [ "$BUILD_FLASHIMG_RET" != "0" ]; then
#  echo "***********************************************************"
#  echo "ERROR: Build image failed!!"
#  echo "***********************************************************"
#  echo "Please check it from error message."
  #exit 1
#else
#  echo " --- build flash image done!!"
#  echo
#  echo "     All image files are saved in the directory of images!!"
#fi

echo "do imageprocess sh..."
if [ -f $BOARD_DIR/$IMAGEPROCESS_SH_FILE ]; then
    $BOARD_DIR/$IMAGEPROCESS_SH_FILE
      BUILD_FLASHIMG_RET=$?
fi

if [ "$LOADER_BUILD" != "y" ]; then
  cd $IMAGES_DIR
  $CHMOD 775 *.*
  BURN_FLASH_INI_FILE=`ls *_flash_*.ini 2>/dev/null`
  BURN_RET=$?

  echo
  echo " --- Please burn image with the following ini file:"
  echo
  echo "     Burn image to ramdisk: download through ICE tool"

  if [ ! -f $IMAGES_DIR/$ALI_SEE_BIN_OUT_FILE ]; then
    echo "          WARNING: No see_bin.out file!!"
    echo "                   Please compile it in TDS"
  else
    echo "          images/$INI_FILE"
  fi

  if [ "$DEV_FLASHBOOT_NOR" = "y" ]; then
    if [ "$BURN_RET" = "0" ]; then
      echo 
      echo "     Burn image to Flash: burn image to Nor flash"
      echo "          images/$BURN_FLASH_INI_FILE"
    fi
  else
    echo "     Burn image to Flash: copy files to MPTool directory"
    echo "          images/ALI.ini"
  fi
fi

#echo "I'm dumpping the dis file now, you can continue the GDB download or flash burning ..."
#echo "If I'm disturbbing you, mark me at buildimage.sh...."
#mipsel-linux-objdump -D $IMAGES_DIR/main_bin.out > $IMAGES_DIR/main_bin.dis
#mipsel-linux-objdump -D $IMAGES_DIR/see_bin.out > $IMAGES_DIR/see_bin.dis
echo "----------------------------------------------------------------------"
