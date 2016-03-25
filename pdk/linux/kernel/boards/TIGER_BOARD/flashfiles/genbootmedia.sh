#!/bin/sh

# parser bootlogo.xml -> bootlogo.bin
python $HOSTTOOLS_DIR/python/bootmedia.py $BOARD_DIR/flashfiles/bootlogo.xml bootlogo.bin
# bootlogo.bin -> bootlogo.ubo
$HOSTTOOLS_DIR/mkimage -A mips -O linux -T kernel -C none -a 800fffc0 -e 80100000 -n bootlogo -d bootlogo.bin bootlogo.ubo

rm bootlogo.bin
mv bootlogo.ubo $LINUX_SRC_DIR/install/bin


# parser bootmedia.xml -> bootmedia.bin
python $HOSTTOOLS_DIR/python/bootmedia.py $BOARD_DIR/flashfiles/bootmedia.xml bootmedia.bin
# bootmedia.bin -> bootmedia.ubo
$HOSTTOOLS_DIR/mkimage -A mips -O linux -T kernel -C none -a 800fffc0 -e 80100000 -n bootmedia -d bootmedia.bin bootmedia.ubo

rm bootmedia.bin
mv bootmedia.ubo $LINUX_SRC_DIR/install/bin