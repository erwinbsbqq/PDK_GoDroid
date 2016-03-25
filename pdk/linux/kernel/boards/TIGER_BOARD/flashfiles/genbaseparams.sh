#!/bin/sh

# parser baseparams.xml -> baseparams.bin

python $HOSTTOOLS_DIR/python/baseparams.py $BOARD_DIR/flashfiles/baseparams.xml baseparams.bin
#gen sector struct, 131072 = 128k
$HOSTTOOLS_DIR/sectorpack baseparams.bin 131072 1

mv baseparams.abs $LINUX_SRC_DIR/install/bin