#!/bin/sh

cp -f ./cmd/*.cmd ./
cp -f ./sdram/* ./
mkdir ./uboot_output

./bl_tool Pure_Retailer_unify_1GB_training.cmd
./bl_tool Pure_Retailer_unify_512MB_training.cmd
./bl_tool Pure_Retailer_unify_256MB_training.cmd
./bl_tool Pure_Retailer_hdmi_dongle.cmd
rm -f ./uboot_output/uboot*.abs
mv -f uboot*.abs ./uboot_output/
rm -f *.abs
rm -f *.cmd
rm -f Params_Area_demo.txt