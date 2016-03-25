/* The following TEXT start address leaves space for the monitor
   workspace. i.e. the NEC VR4300 (IDT) first free address is actually
   0xa001af20. */

ENTRY(_start)
OUTPUT_FORMAT("elf32-tradlittlemips")
SEARCH_DIR(.)
__DYNAMIC  =  0;

/*
 * Allocate the stack to be at the top of memory, since the stack
 * grows down
 */
PROVIDE (__stack = 0);
/* PROVIDE (__global = 0); */

/*
 * Initalize some symbols to be zero so we can reference them in the
 * crt0 without core dumping. These functions are all optional, but
 * we do this so we can have our crt0 always use them if they exist.
 * This is so BSPs work better when using the crt0 installed with gcc.
 * We have to initalize them twice, so we multiple object file
 * formats, as some prepend an underscore.
 */
 

__RAM_BASE = __SEE_START_ADDR;
__ROM_BASE = __BOOT_LOADER ? 0xBFC00000 :
  __BOOT_FROM_FLASH ? 0xBFC00000 : __RAM_BASE;

MEMORY
{
  ram (rwx) : ORIGIN = 0x80000000, LENGTH = 256M
  rom (rx)  : ORIGIN = 0xBFC00000, LENGTH = 512K - 128K
}

SECTIONS
{
  __BOOT_ROM_ADDR = (__BOOT_LOADER ? 0xBFC00000 - 512K : __ROM_BASE);
  .boot __RAM_BASE : AT(__BOOT_ROM_ADDR) {
     ../ddk/boot_see.o(.text)
  } >ram

  __TEXT_RAM_ADDR = __RAM_BASE + SIZEOF(.boot);
  __TEXT_ROM_ADDR = __BOOT_ROM_ADDR + SIZEOF(.boot);
  .text __TEXT_RAM_ADDR : AT(__TEXT_ROM_ADDR) {
    *(.init)
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t*)
    *(.mips16.fn.*)
    *(.mips16.call.*)
    PROVIDE (__runtime_reloc_start = .);
    *(.rel.sdata)
    PROVIDE (__runtime_reloc_stop = .);
    *(.fini)
    *(.reuse)
    *(.ctors)
  	*(.dtors)
    . = ALIGN(16);
  } >ram

  __RODATA_RAM_ADDR = __TEXT_RAM_ADDR + SIZEOF(.text);
  __RODATA_ROM_ADDR = __TEXT_ROM_ADDR + SIZEOF(.text);
  .rodata __RODATA_RAM_ADDR : AT(__RODATA_ROM_ADDR) {
    *(.rdata)
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r*)
    *(.flash)
    . = ALIGN(4);
  } >ram

  __DATA_RAM_ADDR = __RODATA_RAM_ADDR + SIZEOF(.rodata);
  __DATA_ROM_ADDR = __RODATA_ROM_ADDR + SIZEOF(.rodata);
  .data __DATA_RAM_ADDR : AT(__DATA_ROM_ADDR) {
    *(.eh_frame)
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d*)
    *(.gcc_except_table)
    *(.rel.dyn)
    . = ALIGN(4);
  } >ram

  _gp = . + 0x8000;
  __global = _gp;
  .lit8 : {
    *(.lit8)
  }
  .lit4 : {
    *(.lit4)
  }

  __INIT_RAM_ADDR = __DATA_RAM_ADDR + SIZEOF(.data);
  __INIT_ROM_ADDR = __DATA_ROM_ADDR + SIZEOF(.data);
  .structure.init __INIT_RAM_ADDR : AT(__INIT_ROM_ADDR) {
    _video_init_struct_start = .;
     *(.video.plugin)
    _video_init_struct_end = .; 
    . = ALIGN(4);

    _vdec_init_struct_start = .;
    *(.vdec.plugin)
    _vdec_init_struct_end = .; 
    . = ALIGN(4);
    
    _mp4_vdec_init_struct_start = .;
    *(.mp4.vdec.plugin)
    _mp4_vdec_init_struct_end = .;
    . = ALIGN(4);
    
    _adec_init_struct_start = .;
    *(.adec.plugin)
    _adec_init_struct_end = .; 
    . = ALIGN(4);

      	_mp_ext_subtitle_start = .;
  	*(.extsubt.plugin)
  	_mp_ext_subtitle_end = .; 
	. = ALIGN(4);

	_embed_subtitle_start = .;
  	*(.subt.plugin)
  	_embed_subtitle_end = .;
	. = ALIGN(4);
	
  	_fs_init_struct_start = .;
  	*(.fs.init)
  	_fs_init_struct_end = .; 
  	_deca_audio_stream_struct_start = .;
  	*(.deca.init)
  	_deca_audio_stream_struct_end = .; 
  	_audio_init_struct_start = .;
  	*(.audio.init)
  	_audio_init_struct_end = .; 	
   . = ALIGN(4);

       _videodev_init_struct_start = .;
    *(.videodev.plugin)
    _videodev_init_struct_end = .; 
    . = ALIGN(4);

       _audiodev_init_struct_start = .;
    *(.audiodev.plugin)
    _audiodev_init_struct_end = .; 
    . = ALIGN(4);  
    
    __start___rpctab = .;
    *(.rpctab.init)
     __stop___rpctab = .;
    . = ALIGN(4);

	_es_player_start = .;
	*(.es_player.plugin)
	_es_player_end = .;
	. = ALIGN(4);
	
 	_dsc_csa3_struct_start = .;
	*(.dsc_csa3.plugin)
	_dsc_csa3_struct_end = .;
	. = ALIGN(4);
   } >ram
  
  __SDATA_RAM_ADDR = __INIT_RAM_ADDR + SIZEOF(.structure.init);
  __SDATA_ROM_ADDR = __INIT_ROM_ADDR + SIZEOF(.structure.init);

  .sdata __SDATA_RAM_ADDR : AT(__SDATA_ROM_ADDR) {
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s*)
    . = ALIGN(4);
  } >ram

  _edata = __SDATA_ROM_ADDR + SIZEOF(.sdata);

  __SBSS_RAM_ADDR = __SDATA_RAM_ADDR + SIZEOF(.sdata);
  .sbss __SBSS_RAM_ADDR : AT(__SBSS_RAM_ADDR) {
    *(.sbss)
    *(.scommon)
  } >ram

  __BSS_RAM_ADDR = __SBSS_RAM_ADDR + SIZEOF(.sbss);
  .bss __BSS_RAM_ADDR : AT(__BSS_RAM_ADDR) {
    *(.bss)
    *(COMMON)
  } >ram

  _end = .;

/*
  __FLASH_ROM_ADDR = __BOOT_LOADER ?
        (__ROM_BASE + 464K) : __BOOT_FROM_FLASH ?
        (__SDATA_ROM_ADDR + SIZEOF(.sdata)) :
        (__BSS_RAM_ADDR + SIZEOF(.bss));
  .flash __FLASH_ROM_ADDR : AT(__FLASH_ROM_ADDR) {
    *(.flash)
  }
*/  
  
  .reginfo 0 : { *(.reginfo) }
  .mdebug 0 : { *(.mdebug) }
}
