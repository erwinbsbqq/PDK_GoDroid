#ifndef __CONFIG_ALI_3921_H
#define __CONFIG_ALI_3921_H

//#define DEBUG			1

#define CONFIG_SKIP_LOWLEVEL_INIT 1
#define CONFIG_MIPS32			1				/* MIPS32 CPU core	*/

#define ALI_STB_BOARD							//should always define.

#define CONFIG_SYS_LITTLE_ENDIAN

/*define for uboot boot see*/
#define CONFIG_UBOOT_BOOT_SEE
//#define CONFIG_UBOOT_3921_SWITCH
//#define CONFIG_UBOOT_3921_GMAC
#define CONFIG_UBOOT_TOE2

#define CONFIG_ENV_OVERWRITE



//#define CONFIG_SET_CPU_FREQ  //SE need some fixed frequency SW to do some test. David.Chen@20131017

//#define PROFILE_ON

//#define UBOOT_BOOT_MEDIA

//#define DEBUG_NAND_DRV

//#define CVD_DEBUG_UBOOT_LOAD_KERNEL

//#define CONFIG_OF_LIBFDT		1

#define LOAD_KERNEL_SEE

//#define LOAD_RAM_DISK   // for android only, test_bench/ibu  do not need  it






/*======================================================
*
*                   The definitions which don't enrich the binary size  
*
*----------------------------------------------------------------------- */
#define CONFIG_SYS_PROMPT		"ALI_3503 > "

#define CONFIG_SYS_HZ			1000

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_MALLOC_LEN - \
					 GENERATED_GBL_DATA_SIZE)

#define DEFAULT_TOP_PAD        (1024)

#define ALI_SOC_BASE		0xb8000000//VIRT_SYSTEM
#define M3921_QFP256    0x0	// LQFP256
#define M3921_BGA445    0x1	// BGA445
/*chip id*/
#define C3921	0x3921
#define C3503	0x3503


#define CONFIG_ALI_GMAC
#define CONFIG_ETHADDR		DE:AD:BE:EF:01:01    /* Ethernet address */
#define CONFIG_IPADDR		192.168.20.199			/* Our IP address */
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_GATEWAYIP	192.168.20.254
#define CONFIG_SERVERIP		192.168.20.58	/* Server IP address */

/*
 * memtest setup
 */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (32 << 20))

/* Default load address */
#define CONFIG_SYS_LOAD_ADDR		0x80000000

/*
 * Size of malloc() pool
 * Total Size Environment - 128k
 * Malloc - add 256k
 */
#define CONFIG_ENV_SIZE			(1024 << 10)//arthur 128->1024
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + (1024 << 10)) //arthur 256->1024

#define	CONFIG_SYS_CBSIZE		1024 //256		/* Console I/O Buffer Size   */
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)  /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args*/
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)


//.UBO file has a header whose length is 64(0x40)
#define UBO_HEAD_LEN                0x40

#define PART_NAME_AUD_CODE  "ae"
#define PART_NAME_KERNEL        "kernel"
#define PART_NAME_RAMDISK       "rootfs"
#define PART_NAME_SEE               "see"
#define PART_NAME_BM "bootmedia"

#if defined(LOAD_KERNEL_SEE) && defined(UBOOT_BOOT_MEDIA)
   #error "You can't define LOAD_KERNEL_SEE and UBOOT_BOOT_MEDIA at the same time."
#endif

/*
 * serial port - NS16550 compatible
 */
#define CONFIG_BAUDRATE	115200

/*
 * Serial Configuration
 */
#define CONFIG_SYS_NS16550				// For driver/serial/ns16550.c
#define CONFIG_SYS_NS16550_SERIAL		// For driver/serial/serial.c
#define CONFIG_SYS_NS16550_REG_SIZE		1
#define CONFIG_CONS_INDEX				1
#define CONFIG_SYS_NS16550_CLK			100000	// 100KHz
#define CONFIG_SYS_NS16550_COM1			(0xb8018300)
#define CONFIG_SYS_NS16550_COM2			(0xb8018600)

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }


#define CONFIG_ARCH_CPU_INIT

/* Display CPU and Board Info */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

#undef CONFIG_USE_IRQ				/* no support for IRQs */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80007fc0\0" \
	"console=ttyO2,115200n8\0" \
	"usbtty=cdc_acm\0" \
	"vram=16M\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"vram=${vram} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc${mmcdev} ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
		"verify=n\0" \
//		"silent=1\0" \

/*
 * SDRAM Memory Map
 * Even though we use two CS all the memory
 * is mapped to one contiguous block
 */
#define CONFIG_NR_DRAM_BANKS	1
#define CONFIG_SYS_CACHELINE_SIZE	32

/* Defines for SDRAM init */
#define CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS

#ifndef CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
#define CONFIG_SYS_AUTOMATIC_SDRAM_DETECTION
#define CONFIG_SYS_DEFAULT_LPDDR2_TIMINGS
#endif

/*
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 80E7FFC0--0x80E80000 should not be used for any
 * other needs.
 */
 

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1




/*======================================================
*
*                   The definitions which will enrich the binary size or time  
*
*----------------------------------------------------------------------- */
/* Flash */
#define CONFIG_SYS_NO_FLASH	1
#define CONFIG_ENV_IS_NOWHERE       1

#undef CONFIG_SYS_LONGHELP	/* undef to save memory */

#define ALI_NAND_SUPPORT
#define CONFIG_ALI_GPIO


/*Begin:Ali Emmc Enable*/

//#define CONFIG_ALI_MMC
//#define CONFIG_EFI_PARTITION

/*End:Ali Emmc Enable*/

#define CONFIG_ALI_IR


/*
 * Command line configuration.
 */
/* commands to include */
#include <config_cmd_default.h>

/*  * add Nand Flash support */ 
#define CONFIG_CMD_NAND     //Nand Flash Only board

/* USB UHH support options */
#undef CONFIG_CMD_USB

#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define ETH_MANUAL_START
/* Disabled commands */
#undef CONFIG_CMD_BEDBUG
#undef CONFIG_CMD_ELF
#undef CONFIG_CMD_SAVEENV

#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_MII
#undef CONFIG_CMD_RUN
#undef CONFIG_CMD_IDE
#undef CONFIG_CMD_I2C
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_PCMCIA
#undef CONFIG_CMD_FLASH

#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS 

#undef CONFIG_CMD_IMLS		/* List all found images        */
/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_MHZ			600		// 3503 cpu clk default 600MHz, strap pin info:000D007F

#if (CONFIG_SYS_MHZ % 12) != 0
#error "Invalid CPU frequency - must be multiple of 12!"
#endif

#define CONFIG_SYS_MIPS_TIMER_FREQ	(CONFIG_SYS_MHZ * 1000000)


#define	CONFIG_SYS_LONGHELP							/* undef to save memory      */
#define	CONFIG_SYS_PROMPT			"U-boot> "	/* Monitor Command Prompt    */
#define CONFIG_AUTO_COMPLETE

#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)  /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args*/

#if defined(ALI_SOC_M36XX)
#define CONFIG_SYS_MALLOC_LEN		       128*1024
#define CONFIG_SYS_BOOTPARAMS_LEN			128*1024
#else
#define CONFIG_SYS_MALLOC_LEN		       40*1024*1024 //2//1024*1024 //16*1024*1024
#define CONFIG_SYS_BOOTPARAMS_LEN		256*1024 //256*1024
#define CONFIG_SYS_BOOTPARAMS_SATRT		0x85800000 /* boot argment buffer share with kernel*/
#endif

#define CONFIG_SYS_HZ			1000






/*======================================================
*
*                   The sub-condition definition when main condition is defined  
*
*----------------------------------------------------------------------- */
#define CONFIG_BOOTDELAY	1
#undef CONFIG_CMDLINE_EDITING    //support command history.
#undef CONFIG_AUTO_COMPLETE		1

#define CONFIG_SILENT_CONSOLE	1

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_TEXT_BASE		0x85000000 // 0x80100000
//#define CONFIG_SYS_TEXT_BASE		0x80010000 //as bootrom OTP is enable
#define ALI3503_MEM_SIZE            192

/* The following #defines are needed to get flash environment right */
#define	CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define	CONFIG_SYS_MONITOR_LEN		(192 << 10)


#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_MONITOR_BASE - CONFIG_SYS_SDRAM_BASE - CONFIG_SYS_STACK_LEN 
#define CONFIG_SYS_STACK_LEN			0x200000	 // resered for Stack & key param
 
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE		8*1024
#define CONFIG_SYS_ICACHE_SIZE		       16*1024
#define CONFIG_SYS_CACHELINE_SIZE	       32

#ifdef LOAD_KERNEL_SEE
#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1

//Main code load address is 0x80100000, See code load address is 0x84000200
//but .UBO file has a header whose length is 64(0x40)
#define ADDR_LOAD_SEE                      0x840001C0
#define KERNEL_START_ADDR              0x80008000
//#define ADDR_LOAD_KERNEL                0x80007fc0   //(KERNEL_START_ADDR - UBO_HEAD_LEN)
#define ADDR_LOAD_KERNEL                0x800FFFC0   //(KERNEL_START_ADDR - UBO_HEAD_LEN)
#define ADDR_LOAD_AUD_CODE           0x880001C0     //0xa8000200 - 0x40 (UBO header)
#define ADDR_LOAD_MEDIA                 0x82E00000  //0x86000000


#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

/* Boot from flash by default, revert to bootp */	
#define CONFIG_BOOTCOMMAND	"bootm " MK_STR(ADDR_LOAD_KERNEL)

#undef	CONFIG_BOOTARGS
//#define CONFIG_BOOTARGS "console=ttyS0,115200 mem=512M root=/dev/ram0 rootfstype=ramfs"
// if you change the below format, you may have to change the code "sprintf(commandline,CONFIG_BOOTARGS_FMT..." in boot_prep_linux() of arch/arm/lib/bootm.c
#if defined(CONFIG_ALI_MMC)
   //#define CONFIG_BOOTARGS_FMT "init=/init root=/dev/mmcblk0p4 rw rootfstype=ext4 rootwait rw mac=%s mem=%dM" //+ N* "%dK(%s)"
   #define CONFIG_BOOTARGS_FMT "init=/init root=/dev/mmcblk0p%d rw rootfstype=ext4 rootwait rw mac=%s mem=%dM" //+ N* "%dK(%s)"
#else
   #define CONFIG_BOOTARGS_FMT "init=/init androidboot.console=ttyS0 loglevel=0 ubi.fm_autoconvert=1 ubi.mtd=%d root=ubi0:rootfs rootfstype=ubifs rw mac=%s mem=%dM mtdparts=ali_nand:" //+ N* "%dK(%s)"#endif
#endif
#endif

#ifdef CONFIG_CMD_USB
#define CONFIG_DOS_PARTITION		1

#define CONFIG_USB_HOST
#define CONFIG_USB_EHCI
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI_ALI             1
#define CONFIG_EHCI_DCACHE 	1
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET 			1 
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS 3
#endif

#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define ALI_NAND_BASE               0xb8032000  //NAND FLASH Base Address: C3701/C3901: 0xB8032000    C3603/C3811: 0xB803C000
#define CONFIG_SYS_NAND_BASE_LIST	{ ALI_NAND_BASE,}	
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS
#define CONFIG_NAND_ALI
//? #define CONFIG_MTD_PARTITIONS
#define CONFIG_SYS_NAND_MAX_CHIPS	1	//arthuradd
#define CONFIG_SYS_NAND_ONFI_DETECTION //arthuradd
#endif

/*
 * SD/MMC Configuration
 */
#ifdef	CONFIG_ALI_MMC
#define	CONFIG_MMC
#define	CONFIG_CMD_MMC
#define	CONFIG_GENERIC_MMC
#define	CONFIG_MMC_BOUNCE_BUFFER

#define	CONFIG_CMD_FAT
//#define	CONFIG_CMD_EXT2
#define	CONFIG_DOS_PARTITION
//#define CONFIG_EFI_PARTITION
//#define CONFIG_MMC_TRACE
#endif

#ifdef UBOOT_BOOT_MEDIA
#define ENABLE_AUDIO_CPU
#define ENABLE_AUTO_PLAYBACK

#define PART_NAME_SEE_BM		"bootMedia_see"
#define PART_NAME_BM			"media_file"

#define ADDR_LOAD_BMEDIA_SEE     (0x85c00200)
#define ADDR_LOAD_CODE_AUD        (0x81000000)
//#define ADDR_LOAD_MEDIA              (0x88000000)
#endif


#ifdef PROFILE_ON
#define profile_dbg printf
#else
#define profile_dbg
#endif


/*Begin:add for minirpc*/

//#define   CONFIG_ALI_MINIRPC       //Switch To Support MiniRPC or Not

#ifdef CONFIG_ALI_MINIRPC
#define CONFIG_USE_IRQ             
#ifdef CONFIG_USE_IRQ
#  define CONFIG_STACKSIZE_IRQ	(16 * 1024)	/* IRQ stack */
#  define CONFIG_STACKSIZE_FIQ	(16 * 1024)	/* FIQ stack */
#endif
#endif

/*End:add for minirpc*/




//#define CONFIG_VIDEO
//define CONFIG_CFB_CONSOLE
//define CONFIG_VIDEO_LOGO
//define CONFIG_VIDEO_BMP_LOGO
//define CONFIG_VIDEO_ALIFB
//define CONFIG_VGA_AS_SINGLE_DEVICE

#endif /* __CONFIG_ALI_3921_H */
