[PARAMETER]
chip_name=3921
#RSASSA_PKCS or RSASSA_PSS
sig_format=RSASSA_PKCS
#public key level is 1 or 2
pub_key_level=2
#CSTM need modify it
market_id=12345678
#Default address is 0xa0000200
loader_entry=0x80100000
# external public key, X509 with DER format
ext_pub_key=public.key
#aux_code need set before bl_name
aux_code=mem_init_file.abs
aux_code_len=0x3700
# Root Key pair, ALI or DER format
root_key_format=ALI
rsa_key=root_key_pair.key
# External Pubic Key, ALI or DER format 
ext_key_format=DER
ext_rsa_key=ext_key_pair.der
##input bl name
bl_name= input_bl.abs   
##output bl name
output= output_bl.abs 
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
ext_key_sign
aux_sign
bl_sign
merge