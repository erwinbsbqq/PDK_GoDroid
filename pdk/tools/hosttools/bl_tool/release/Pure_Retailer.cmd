[PARAMETER]
chip_name=3921
#aux_code need set before bl_name
aux_code_len=0x3700
loader_entry=0x80100000
bl_name= uboot.bin
aux_code= memory_init.abs
output= output_uboot.abs 
[NAND CONFIG]
block_perchip=1000
pages_perblock=0x40
bytes_perpage=0x800
eccsec_size=0x400
eccredu_size=0x20
rowaddr_cycle=0x3
ecctype=0x0
read_timing=0x22
write_timing=0x22
eccsec_perpage=0x2
[CMD]
merge