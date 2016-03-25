#!/bin/sh

# parser Ali_nand_desc.xml -> ALI.ini
#python $HOSTTOOLS_DIR/python/ftoolini.py $BOARD_DIR/flashfiles/ubifs/Ali_nand_desc.xml.2kpage
#mv ALI.ini $LINUX_SRC_DIR/install/bin/ALI_ubi_2k.ini

#python $HOSTTOOLS_DIR/python/ftoolini.py $BOARD_DIR/flashfiles/ubifs/Ali_nand_desc.xml.4kpage
#mv ALI.ini $LINUX_SRC_DIR/install/bin/ALI_ubi_4k.ini

python $HOSTTOOLS_DIR/python/ftoolini.py $BOARD_DIR/flashfiles/ubifs/Ali_nand_desc.xml
mv ALI.ini $LINUX_SRC_DIR/install/bin
