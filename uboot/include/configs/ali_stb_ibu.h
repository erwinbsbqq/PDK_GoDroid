/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * This file contains the configuration parameters for the ALi STB Demo Board board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SKIP_LOWLEVEL_INIT 1

#define CONFIG_MIPS32			1				/* MIPS32 CPU core	*/
#define ALI_STB_BOARD							//should always define.
#define CONFIG_CMDLINE_EDITING    //support command history.
#define HAVE_PART_TBL_PARTITION

//#define ALI_SOC_M39XX
//#define ALI_SOC_M36XX
#define ALI_SOC_M37XX

#define CONFIG_ALI_TOE2		//added by DavidChen
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_SYS_NO_FLASH		/* no *NOR* flash , Ali Nor flash seems use a special interface,not CFI*/
//#define ETH_MANUAL_START                //add eth on/off cmd to apply user-defined Mac adress and IP address.
#define CONFIG_SYS_CONSOLE_INFO_QUIET
//#define CONFIG_NORFLASH_ALI

#define CONFIG_ALI_I2C
#define CONFIG_ALI_GPIO

/* 
 * add Nand Flash support
 */ 
#define CONFIG_CMD_NAND     //Nand Flash Only board

#ifdef CONFIG_CMD_NAND
   #ifdef CONFIG_NORFLASH_ALI
   #error "You can't define Nand and Nor at the same time."
   #endif
//#define GET_ETHADDR_STBID  //Now STBID info only is saved in Nand flash board.  but you can uncomment this line define if you want to use default MAC address in the following MACRO  "CONFIG_ETHADDR".
#define PMI_PART_SCT
#endif


#define CFG_KEY_LEN      (0x2000)  // reversed for sig key 
#define CFG_KEY_ADDR     (0x88000000) 
#define CFG_HW_CIPHER_KEY_LEN      (0x400)      // reversed for hw cipher key 
#define CFG_HW_CIPHER_KEY_ADDR     (0x88000400) 
#define CFG_SYS_UK_LEN      (0x400)      // reversed for sys universal key
#define CFG_SYS_UK_ADDR     (CFG_HW_CIPHER_KEY_ADDR + CFG_HW_CIPHER_KEY_LEN)

//#define UBOOT_LOGO
//#define _CAS9_CA_ENABLE_ 
/*-----------------------------------------------------------------------
 * set some initial configurations depending on configure target
 *
 * ali-stb_config			ROM Version
 * ali-stb_ram_config		RAM Version
 */
 
#ifdef CONFIG_RAMBOOT
//#define CONFIG_SYS_TEXT_BASE 0x80100000		//ALI_SOC_M36XX RAM Version
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_TEXT_BASE 0x85000000//0xa3000200		// RAM Version  Nand AliBoot to U-boot
#else
#define CONFIG_SYS_TEXT_BASE 0x85000000		// RAM Version Nor AliBoot to U-boot  0xa3800000
#endif
#else
#define CONFIG_SYS_TEXT_BASE 0xafc00000		// ROM Version
//#define CONFIG_SYS_TEXT_BASE 0xbfc00000		//ALI_SOC_M36XX ROM Version
#endif

/* IP address is default used by ALi-STB */
#define CONFIG_ETHADDR		DE:AD:BE:EF:01:01    /* Ethernet address */
#define CONFIG_IPADDR		192.168.9.198			/* Our IP address */
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_GATEWAYIP	192.168.9.254
#define CONFIG_SERVERIP		192.168.20.80	/* Server IP address */
#define CONFIG_SYS_DIRECT_FLASH_TFTP    1

#define CONFIG_BOOTDELAY	0	/* autoboot after 3 seconds */
#define CONFIG_BAUDRATE	115200

/*-----------------------------------------------------------------------
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

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */
#undef	CONFIG_BOOTARGS

//#define CONFIG_BOOTARGS "root=/dev/nfs rootfstype=nfs rw init=/bin/init nfsroot=192.168.9.226:/home/owen/work/ALi_SDK/C3603/linux_stb_nfs ip=192.168.21.11:192.168.9.226:192.168.21.254:255.255.255.0::eth0"
#define CFG_KEY_LEN      (0x2000)  // reversed for sig key 
#define CFG_KEY_ADDR     (0x88000000) 
#define CFG_HW_CIPHER_KEY_LEN      (0x400)      // reversed for hw cipher key 
#define CFG_HW_CIPHER_KEY_ADDR     (0x88000400) 
#define CFG_SYS_UK_LEN      (0x400)      // reversed for sys universal key
#define CFG_SYS_UK_ADDR     (CFG_HW_CIPHER_KEY_ADDR + CFG_HW_CIPHER_KEY_LEN)
#define CFG_SYS_VER_LEN      (0x100)      // reversed for system version
#define CFG_SYS_VER_ADDR     (CFG_SYS_UK_ADDR + CFG_SYS_UK_LEN)
//#define CONFIG_BOOTARGS "noinitrd init=/bin/init root=/dev/mtdblock5 rootfstype=yaffs2 rootflags=inband-tags,verify ro signature=0x88000000,0x1010 sys_uk=0x88000800,0x400 sys_ver=0x88000c00,0x100"
#define CONFIG_BOOTARGS "noinitrd init=/bin/init root=/dev/mtdblock5 rootfstype=ext4 rootflags=verify ro signature=0x88000000,0x1010 sys_uk=0x88000800,0x400 sys_ver=0x88000c00,0x100"
//#define CONFIG_BOOTARGS "noinitrd init=/sbin/bootchartd bootchart_init=/bin/init root=/dev/mtdblock5 rootfstype=yaffs2 rootflags=inband-tags,verify ro signature=0x88000000,0x1010 sys_uk=0x88000800,0x400 sys_ver=0x88000c00,0x100"
//#define CONFIG_BOOTARGS "noinitrd init=/sbin/bootchartd bootchart_init=/bin/init root=/dev/mtdblock5 rootfstype=ext4 rootflags=verify ro signature=0x88000000,0x1010 sys_uk=0x88000800,0x400 sys_ver=0x88000c00,0x100"
//#define CONFIG_BOOTARGS "signature=0x88000000,0x1010 sys_uk=0x88000800,0x400 sys_ver=0x88000c00,0x100"

//#define ET_DEBUG
//#define NFS_DEBUG
//#define M3515_SECURE_BOOT_ENABLE
#define CONFIG_CONSOLE
#define NAND_BOOT
//#define CONFIG_RODATA_ENCRYPT

#define	CONFIG_EXTRA_ENV_SETTINGS					\
    "addmisc=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate} "				\
		"panic=1\0"						\
	"bootfile=/var/lib/tftpboot/main_bin.ubo\0"				\
	"loadaddr=0x800fffc0\0"\
	"see_loadaddr=0x86000200\0"                \
	"load=tftp 80500000 ${u-boot}\0"				\
	""



//Main code load address is 0x80100000, See code load address is 0x84000200
//but .UBO file has a header whose length is 64(0x40)
#define ADDR_LOAD_MAIN            0x800FFFC0   
#define ADDR_LOAD_SEE              0x840001C0
#define UBO_HEAD_LEN                0x40

#ifdef CONFIG_NORFLASH_ALI
#define CHUNK_LENGTH    4
#define CHUNK_HEADER_SIZE       128

#define CHUNKID_MAINCODE        0x01FE0101
#define CHUNKID_MAINCODE_MASK   0xFFFF0000
#define CHUNKID_SEECODE         0x06F90101
#define CHUNKID_SEECODE_MASK    0xFFFF0000
#define CHUNKID_SECSEECODE       0x07F80000
#define CHUNKID_SECSEECODE_MASK  0xFFFF0000
#endif

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

/* Boot from flash by default, revert to bootp */	
#define CONFIG_BOOTCOMMAND	"bootm " MK_STR(ADDR_LOAD_MAIN)

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
//#undef CONFIG_CMD_BDI
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

#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING

#undef CONFIG_CMD_FLASH

#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS 

#define	CONFIG_CMD_USB    //USB boot function support.
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_ALIUPG
/*
 * Miscellaneous configurable options
 */
#if defined(ALI_SOC_M37XX)
#define CONFIG_SYS_MHZ			600
#else
#define CONFIG_SYS_MHZ			396
#endif

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
#define CONFIG_SYS_MALLOC_LEN		       48*1024*1024 //2//1024*1024 //16*1024*1024
#define CONFIG_SYS_BOOTPARAMS_LEN		256*1024 //256*1024
#define CONFIG_SYS_BOOTPARAMS_SATRT		0x85800000 /* boot argment buffer share with kernel*/
#endif

#define CONFIG_SYS_HZ			1000


#define CONFIG_SYS_SDRAM_BASE		0x80000000     /* Cached addr */
#define CONFIG_SYS_LOAD_ADDR		0x81000000     /* default load address	*/

#define CONFIG_SYS_MEMTEST_START	0x80700000
#define CONFIG_SYS_MEMTEST_END	0x80800000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1				/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT (64)				/* max number of sectors on one chip */

#define PHYS_FLASH_1				0xafc00000 		/* Flash Bank #1 */
//#define PHYS_FLASH_2				0xbc000000 		/* Flash Bank #2 */
#define CONFIG_SYS_FLASH_BANKS_LIST {PHYS_FLASH_1}
//#define CONFIG_SYS_FLASH_BANKS_LIST {PHYS_FLASH_1, PHYS_FLASH_2}

//#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_CFI		0
#define CONFIG_FLASH_CFI_DRIVER		0
#if 0
#define CONFIG_SPI_FLASH			1
#define CONFIG_CMD_SF				1
//#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_SST
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_WINBOND
#endif
/* The following #defines are needed to get flash environment right */
#define	CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define	CONFIG_SYS_MONITOR_LEN		(192 << 10)


#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_MONITOR_BASE - CONFIG_SYS_SDRAM_BASE - CONFIG_SYS_STACK_LEN 
#define CONFIG_SYS_STACK_LEN			0x200000	 // resered for Stack & key param


/* We boot from this flash, selected with dip switch */
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2 * CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2 * CONFIG_SYS_HZ) /* Timeout for Flash Write */

#define	CONFIG_ENV_IS_NOWHERE		1
//#define CONFIG_ENV_IS_IN_FLASH	1

/* Address and size of Primary Environment Sector	*/
#define CONFIG_ENV_ADDR				0xbfc00000
//#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SIZE				0x10000

#define CONFIG_FLASH_16BIT

#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_NET_MULTI

#define MEM_SIZE 					64 /* M bytes According to the ALi specific memory mapping(Private memory),
										U-boot can't use the whole DRAM space but the lower space.*/

#define CONFIG_MEMSIZE_IN_BYTES
 
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE		8*1024
#define CONFIG_SYS_ICACHE_SIZE		       16*1024
#define CONFIG_SYS_CACHELINE_SIZE	       32

/*----------------------------------------------------------------------- 
  * NAND-FLASH stuff
  *-----------------------------------------------------------------------
  */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define ALI_NAND_BASE                               0xB8032000  //NAND FLASH Base Address: C3701/C3901: 0xB8032000    C3603/C3811: 0xB803C000
#define CONFIG_SYS_NAND_BASE_LIST		{ ALI_NAND_BASE,}	
#define CONFIG_MTD_DEVICE
///#define CONFIG_CMD_MTDPARTS
#define CONFIG_NAND_ALI
//? #define CONFIG_MTD_PARTITIONS
#endif

#ifndef _CAS9_CA_ENABLE_

#define __G_MM_PRIVATE_AREA_START_ADDR	0x6000000		// 0xa6000000
#define __G_MM_PRIVATE_AREA_TOP_ADDR	0x7FFFE00
#define __G_MM_SHARED_MEM_START_ADDR	__G_MM_PRIVATE_AREA_TOP_ADDR
#define __G_RPC_MM_LEN					0x200

#else

#define __G_MM_PRIVATE_AREA_START_ADDR	0x6000000		// 0xa6000000
#define __G_MM_PRIVATE_AREA_TOP_ADDR	0x7FFFF00
#define __G_MM_SHARED_MEM_START_ADDR	__G_MM_PRIVATE_AREA_TOP_ADDR
#define __G_RPC_MM_LEN					0x100

#endif


#ifdef CONFIG_CMD_USB
/* USB */

/* OHCI */
//#define CONFIG_USB_OHCI_NEW						1
//#define CONFIG_USB_OHCI_ALI						1
//#define   CONFIG_SYS_USB_OHCI_CPU_INIT			1
//#define	CONFIG_SYS_USB_OHCI_BOARD_INIT			1
//#define CONFIG_SYS_USB_OHCI_REGS_BASE			0xB803B000
//#define CONFIG_SYS_USB_OHCI_SLOT_NAME			"ali_ohci"
//#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS		3

/* EHCI */
#define CONFIG_USB_EHCI								1 	/* Enable EHCI USB support	*/                          
#define CONFIG_USB_EHCI_ALI							1	/* on ALi M39xx platform		*/                                
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS			3
#if defined(ALI_SOC_M37XX)
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET 			1 	/* re-init HCD after CMD_RESET */
#endif
//#define CONFIG_USB_KEYBOARD						1
#define CONFIG_USB_STORAGE						    1
#define CONFIG_EHCI_DCACHE				            1
#define CONFIG_DIRECT_FLASH_USB				        1
#endif

#endif	/* __CONFIG_H */
