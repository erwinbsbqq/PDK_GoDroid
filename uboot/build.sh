#!/bin/bash

if [[ $1 = clean ]];then
 make distclean
 exit 
fi

if [[ $1 = backup ]];then
 make distclean
 DateStr=`date +%Y%m%d_%k%M`
 TarFile="u-boot201204_bk$DateStr.tar.bz2"
 tar jcvf ../$TarFile .
 exit 
fi

if [[ $1 = rom ]];then
 # build flash boot binary file
 TARGET=ali-stb
 export TARGET

 # For release version, we need to copy libali-stb.ooo to libali-stb.o fistly.
 if [ -f board/ali-stb/libali-stb.ooo ]; then
   cp board/ali-stb/libali-stb.ooo board/ali-stb/libali-stb.o
 fi

 make distclean
 make ${TARGET}_config
 make all
 if [ "$?" == "0" ]; then 
  cp u-boot.bin u-boot_${TARGET}.bin
 fi

else

 # default: build WinGDB Download out file
 TARGET=ali-stb_ram
 export TARGET

 # For release version, we need to copy libali-stb.ooo to libali-stb.o fistly.
 if [ -f board/ali-stb/libali-stb.ooo ]; then
  cp board/ali-stb/libali-stb.ooo board/ali-stb/libali-stb.o
 fi

 # make distclean
 make ${TARGET}_config
 make all

 # Only if the above build step is successful, I do the following thing.
 if [ "$?" == "0" ]; then 
  cp u-boot u-boot_${TARGET}.out
  # following line is to get 'CROSS_COMPILE' setting
  . arch/mips/config.mk > /dev/null 2>&1
  # echo ${CROSS_COMPILE}
  ${CROSS_COMPILE}objdump -D u-boot_${TARGET}.out > u-boot_${TARGET}.dis
  tools/mkimage -A mips -O linux -T kernel -C none -a A30001C0 -e a3000200 -n u-boot -d u-boot.bin u-boot.ubo
 fi

fi