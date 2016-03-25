#!/bin/sh

HOSTTOOLS_DIR=$LINUX_SRC_DIR/../tools/hosttools

$BOARD_DIR/flashfiles/genftoolini.sh
$BOARD_DIR/flashfiles/genbootargs.sh
$BOARD_DIR/flashfiles/gendeviceinfo.sh
$BOARD_DIR/flashfiles/genbaseparams.sh
$BOARD_DIR/flashfiles/genbootmedia.sh
