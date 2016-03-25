==================================================
Link SEE executable file for ali linux system
==================================================
step 1.
Make sure CC_DIR in /src/ae/m36f_linux/compiler_ae.def
define the correct mips-sde path of your compiling PC.
step 2.
If need not support AC3/AAC decoding, remove AC3/AAC decoding from /ddk/ae_plugin
plug-in file to /ddk/ae_plugin folder.
step 3.
Config the path, cd src/ae/m36f_linux:
make path
step 4.
Config compiler.def , cd src/ae/m36f_linux:
make config_3921_release
step 5.
Link ae_bin.abs, cd src/ae/m36f_linux:
make generate_ae
