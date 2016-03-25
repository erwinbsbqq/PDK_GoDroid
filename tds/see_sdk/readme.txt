==================================================
Link SEE executable file for ali linux system
==================================================
step 1.
Make sure CC_DIR in /src/see/m36f_linux/compiler_see.def
define the correct mips-sde path of your compiling PC.
step 2.
If need not support AC3/AAC decoding, remove AC3/AAC decoding from /ddk/plugin
plug-in file to /ddk/plugin folder.
step 3.
Config the path, cd src/see/m36f_linux:
make path
step 4.
Config compiler.def , cd src/see/m36f_linux:
make config_3921_release
step 5.
Link see.abs, cd src/see/m36f_linux:
make generate_see
