#!/bin/sh

# parser deviceinfo.xml -> deviceinfo.bin
python $HOSTTOOLS_DIR/python/deviceinfo.py $BOARD_DIR/flashfiles/deviceinfo.xml deviceinfo.bin
#gen sector struct, 131072 = 128k
$HOSTTOOLS_DIR/sectorpack deviceinfo.bin 131072 1

rm deviceinfo.bin
mv deviceinfo.abs $LINUX_SRC_DIR/install/bin