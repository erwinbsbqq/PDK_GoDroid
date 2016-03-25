#!/bin/bash

DEST_DIR_NAME=$1

if [ "$DEST_DIR_NAME" != "" ]; then
    rm -rf prebuildimages/aliboot/*
    mkdir -p prebuildimages/aliboot/$DEST_DIR_NAME
    
    rm -rf prebuildimages/alisee/*
    mkdir -p prebuildimages/alisee/$DEST_DIR_NAME
else
  echo "-----------------------------------------------------"
  echo " error : please set proper board name ! "
  echo " usage : ./cp_bootabs_release.sh <board name>"
  echo "-----------------------------------------------------"
  exit
fi

# bootloader
echo "copy bootloader to prebuildimages/aliboot/$DEST_DIR_NAME ..." ;
if [ -f ../../uboot/uboot_merger/uboot_output/uboot_unify_1GB_training.abs ]; then
  cp -raf ../../uboot/uboot_merger/uboot_output/* prebuildimages/aliboot/$DEST_DIR_NAME/
  cp -raf ../../uboot/u-boot.bin prebuildimages/aliboot/$DEST_DIR_NAME/
else
  echo "warning: please make&merge bootloader first!"
  exit
fi

# ae/see
echo "copy ae/see to prebuildimages/alisee/$DEST_DIR_NAME ..." ;
if [ -f ../../tds/ae_sdk/obj/ae_bin.abs ]; then
  cp -raf ../../tds/ae_sdk/obj/ae_bin.abs prebuildimages/alisee/$DEST_DIR_NAME/
else
  echo "warning: please geneate ae_bin.abs first!"
  exit
fi

if [ -f ../../tds/see_sdk/obj/see_bin.abs ]; then
  cp -raf ../../tds/see_sdk/obj/see_bin.abs prebuildimages/alisee/$DEST_DIR_NAME/
else
  echo "warning: please geneate see_bin.abs first!"
  exit
fi

echo "done!"
echo "--------------------------------------------------------------------------"
