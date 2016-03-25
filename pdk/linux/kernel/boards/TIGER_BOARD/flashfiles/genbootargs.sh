#!/bin/sh

# parser Ali_nand_desc.xml -> cmdline & cmdline_rcv
python $HOSTTOOLS_DIR/python/cmdline.py $BOARD_DIR/flashfiles/ubifs/Ali_nand_desc.xml -r rootfs -v recovery

# parser memmapinfo.xml -> memmapinfo.bin
python $HOSTTOOLS_DIR/python/memmapinfo.py $BOARD_DIR/flashfiles/memmapinfo.xml memmapinfo.bin

# parser memmapinfo_rcv.xml -> memmapinfo_rcv.bin
python $HOSTTOOLS_DIR/python/memmapinfo.py $BOARD_DIR/flashfiles/memmapinfo_rcv.xml memmapinfo_rcv.bin

# parser registerinfo.xml -> registerinfo.bin
python $HOSTTOOLS_DIR/python/registerinfo.py $BOARD_DIR/flashfiles/registerinfo.xml registerinfo.bin

# parser tveconfig.xml -> tveconfig.bin
python $HOSTTOOLS_DIR/python/tveconfig.py $BOARD_DIR/flashfiles/tveconfig.xml tveconfig.bin

# parser bootargs.xml -> bootargs.bin
python $HOSTTOOLS_DIR/python/bootargs.py $BOARD_DIR/flashfiles/bootargs.xml bootargs.bin
#gen sector struct, 131072 = 128k
$HOSTTOOLS_DIR/sectorpack bootargs.bin 131072 1

rm bootargs.bin
rm cmdline
rm cmdline_rcv
rm memmapinfo.bin
rm memmapinfo_rcv.bin
rm registerinfo.bin
rm tveconfig.bin

mv bootargs.abs $LINUX_SRC_DIR/install/bin

#mv bootargs.bin $LINUX_SRC_DIR/install/bin
#mv cmdline ../../../image/
#mv cmdline_rcv ../../../image/
