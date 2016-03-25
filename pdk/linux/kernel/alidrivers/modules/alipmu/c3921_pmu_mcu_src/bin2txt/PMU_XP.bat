
cp ../../../../../prj/tools/HEX2BIN.EXE ./ -rf
cp ../../../../../prj/tools/bin2txt.exe ./ -rf
rm -rf  mcu.bin 
HEX2BIN.EXE mcu.hex 
bin2txt.exe mcu.bin mcu.txt 1 16 4
cp  mcu.bin ../mcu_debug/ -rf

