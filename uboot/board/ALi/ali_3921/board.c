/*
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
 * To match the U-Boot user interface on ARM platforms to the U-Boot
 * standard (as on PPC platforms), some messages with debug character
 * are removed from the default U-Boot build.
 *
 * Define DEBUG here if you want additional info as shown below
 * printed upon startup:
 *
 * U-Boot code: 00F00000 -> 00F3C774  BSS: -> 00FC3274
 * IRQ Stack: 00ebff7c
 * FIQ Stack: 00ebef7c
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <version.h>
#include <net.h>
#include <serial.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <mmc.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <post.h>
#include <logbuff.h>
#ifdef S3921_FT_NOR_TEST
#include "../../../board/ALi/ali_3921/see_bloader3.h"//add for FT Test
#endif

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

#ifdef CONFIG_DRIVER_SMC91111
#include "../drivers/net/smc91111.h"
#endif
#ifdef CONFIG_DRIVER_LAN91C96
#include "../drivers/net/lan91c96.h"
#endif

#include "../../../board/ALi/ali_3921/bcb.h"

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

#ifdef CONFIG_HAS_DATAFLASH
extern int  AT91F_DataflashInit(void);
extern void dataflash_print_info(void);
#endif

#if defined(CONFIG_HARD_I2C) || \
    defined(CONFIG_SOFT_I2C)
#include <i2c.h>
#endif

/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
inline void __coloured_LED_init(void) {}
void coloured_LED_init(void)
	__attribute__((weak, alias("__coloured_LED_init")));
inline void __red_led_on(void) {}
void red_led_on(void) __attribute__((weak, alias("__red_led_on")));
inline void __red_led_off(void) {}
void red_led_off(void) __attribute__((weak, alias("__red_led_off")));
inline void __green_led_on(void) {}
void green_led_on(void) __attribute__((weak, alias("__green_led_on")));
inline void __green_led_off(void) {}
void green_led_off(void) __attribute__((weak, alias("__green_led_off")));
inline void __yellow_led_on(void) {}
void yellow_led_on(void) __attribute__((weak, alias("__yellow_led_on")));
inline void __yellow_led_off(void) {}
void yellow_led_off(void) __attribute__((weak, alias("__yellow_led_off")));
inline void __blue_led_on(void) {}
void blue_led_on(void) __attribute__((weak, alias("__blue_led_on")));
inline void __blue_led_off(void) {}
void blue_led_off(void) __attribute__((weak, alias("__blue_led_off")));


/*
 ************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */

#if defined(CONFIG_ARM_DCC) && !defined(CONFIG_BAUDRATE)
#define CONFIG_BAUDRATE 115200
#endif

static int init_baudrate(void)
{
	gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);
	return 0;
}

static int display_banner(void)
{
	printf("\n\n%s\n\n", version_string);
	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	       _TEXT_BASE,
	       _bss_start_ofs + _TEXT_BASE, _bss_end_ofs + _TEXT_BASE);
#ifdef CONFIG_MODEM_SUPPORT
	debug("Modem Support enabled\n");
#endif
#ifdef CONFIG_USE_IRQ
	debug("IRQ Stack: %08lx\n", IRQ_STACK_START);
	debug("FIQ Stack: %08lx\n", FIQ_STACK_START);
#endif

	return (0);
}

/*
 * WARNING: this code looks "cleaner" than the PowerPC version, but
 * has the disadvantage that you either get nothing, or everything.
 * On PowerPC, you might see "DRAM: " before the system hangs - which
 * gives a simple yet clear indication which part of the
 * initialization if failing.
 */
static int display_dram_config(void)
{
	int i;

#ifdef DEBUG
	puts("RAM Configuration:\n");

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		printf("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size(gd->bd->bi_dram[i].size, "\n");
	}
#else
	ulong size = 0;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		size += gd->bd->bi_dram[i].size;

	puts("DRAM:  ");
	print_size(size, "\n");
#endif

	return (0);
}

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
static int init_func_i2c(void)
{
	puts("I2C:   ");
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	puts("ready\n");
	return (0);
}
#endif

#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
#include <pci.h>
static int arm_pci_init(void)
{
	pci_init();
	return 0;
}
#endif /* CONFIG_CMD_PCI || CONFIG_PCI */

/*
 * Breathe some life into the board...
 *
 * Initialize a serial port as console, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependent #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

int print_cpuinfo(void);

void __dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  gd->ram_size;
}
void dram_init_banksize(void)
	__attribute__((weak, alias("__dram_init_banksize")));

init_fnc_t *init_sequence[] = {
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
#ifdef CONFIG_OF_CONTROL
	fdtdec_check_fdt,
#endif
	timer_init,		/* initialize timer */
#ifdef CONFIG_FSL_ESDHC
	get_clocks,
#endif
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	display_banner,		/* say that we are here */
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	init_func_i2c,
#endif
	dram_init,		/* configure available RAM banks */
	NULL,
};

void board_init_f(ulong bootflag)
{
	bd_t *bd;
	init_fnc_t **init_fnc_ptr;
	gd_t *id;
	ulong addr, addr_sp;
#ifdef CONFIG_PRAM
	ulong reg;
#endif

	//profile_dbg("--------Profile dbg on <%s> Line:%d-----enter board_init_f()----\n", __FUNCTION__,__LINE__);
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) ((CONFIG_SYS_INIT_SP_ADDR) & ~0x07);
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	memset((void *)gd, 0, sizeof(gd_t));

	gd->mon_len = _bss_end_ofs;
#ifdef CONFIG_OF_EMBED
	/* Get a pointer to the FDT */
	gd->fdt_blob = _binary_dt_dtb_start;
#elif defined CONFIG_OF_SEPARATE
	/* FDT is at end of image */
	gd->fdt_blob = (void *)(_end_ofs + _TEXT_BASE);
#endif
	/* Allow the early environment to override the fdt address */
	gd->fdt_blob = (void *)getenv_ulong("fdtcontroladdr", 16,
						(uintptr_t)gd->fdt_blob);

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}
	profile_dbg("--------Profile dbg on <%s> Line:%d-----after init_fnc_ptr()----\n", __FUNCTION__,__LINE__);

#ifdef CONFIG_OF_CONTROL
	/* For now, put this check after the console is ready */
	if (fdtdec_prepare_fdt()) {
		panic("** CONFIG_OF_CONTROL defined but no FDT - please see "
			"doc/README.fdt-control");
	}
#endif

	debug("monitor len: %08lX\n", gd->mon_len);
	/*
	 * Ram is setup, size stored in gd !!
	 */
	debug("ramsize: %08lX\n", gd->ram_size);
#if defined(CONFIG_SYS_MEM_TOP_HIDE)
	/*
	 * Subtract specified amount of memory to hide so that it won't
	 * get "touched" at all by U-Boot. By fixing up gd->ram_size
	 * the Linux kernel should now get passed the now "corrected"
	 * memory size and won't touch it either. This should work
	 * for arch/ppc and arch/powerpc. Only Linux board ports in
	 * arch/powerpc with bootwrapper support, that recalculate the
	 * memory size from the SDRAM controller setup will have to
	 * get fixed.
	 */
	gd->ram_size -= CONFIG_SYS_MEM_TOP_HIDE;
#endif

	addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size;

#ifdef CONFIG_LOGBUFFER
#ifndef CONFIG_ALT_LB_ADDR
	/* reserve kernel log buffer */
	addr -= (LOGBUFF_RESERVE);
	debug("Reserving %dk for kernel logbuffer at %08lx\n", LOGBUFF_LEN,
		addr);
#endif
#endif

#ifdef CONFIG_PRAM
	/*
	 * reserve protected RAM
	 */
	reg = getenv_ulong("pram", 10, CONFIG_PRAM);
	addr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg, addr);
#endif /* CONFIG_PRAM */

#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
	/* reserve TLB table */
	addr -= (4096 * 4);

	/* round down to next 64 kB limit */
	addr &= ~(0x10000 - 1);

	gd->tlb_addr = addr;
	debug("TLB table at: %08lx\n", addr);
#endif

	/* round down to next 4 kB limit */
	addr &= ~(4096 - 1);
	printf("Top of RAM usable for U-Boot at: %08lx\n", addr);

#ifdef CONFIG_LCD
#ifdef CONFIG_FB_ADDR
	gd->fb_base = CONFIG_FB_ADDR;
#else
	/* reserve memory for LCD display (always full pages) */
	addr = lcd_setmem(addr);
	gd->fb_base = addr;
#endif /* CONFIG_FB_ADDR */
#endif /* CONFIG_LCD */

	/*
	 * reserve memory for U-Boot code, data & bss
	 * round down to next 4 kB limit
	 */
	addr -= gd->mon_len;
	addr &= ~(4096 - 1);

	printf("Reserving %ldk for U-Boot at: %08lx\n", gd->mon_len >> 10, addr);

#ifndef CONFIG_SPL_BUILD
	/*
	 * reserve memory for malloc() arena
	 */
	addr_sp = addr - TOTAL_MALLOC_LEN;
	printf("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, addr_sp);
	/*
	 * (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr_sp -= sizeof (bd_t);
	bd = (bd_t *) addr_sp;
	gd->bd = bd;
	printf("Reserving %zu Bytes for Board Info at: %08lx\n",
			sizeof (bd_t), addr_sp);

#ifdef CONFIG_MACH_TYPE
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE; /* board id for Linux */
#endif

	addr_sp -= sizeof (gd_t);
	id = (gd_t *) addr_sp;
	printf("Reserving %zu Bytes for Global Data at: %08lx\n",
			sizeof (gd_t), addr_sp);

	/* setup stackpointer for exeptions */
	gd->irq_sp = addr_sp;
#ifdef CONFIG_USE_IRQ
	addr_sp -= (CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ);
	debug("Reserving %zu Bytes for IRQ stack at: %08lx\n",
		CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ, addr_sp);
#endif
	/* leave 3 words for abort-stack    */
	addr_sp -= 12;

	/* 8-byte alignment for ABI compliance */
	addr_sp &= ~0x07;
#else
	addr_sp += 128;	/* leave 32 words for abort-stack   */
	gd->irq_sp = addr_sp;
#endif

	printf("New Stack Pointer is: %08lx\n", addr_sp);

#ifdef CONFIG_POST
	post_bootmode_init();
	post_run(NULL, POST_ROM | post_bootmode_get(0));
#endif

	gd->bd->bi_baudrate = gd->baudrate;
	/* Ram ist board specific, so move it to board code ... */
	dram_init_banksize();
	display_dram_config();	/* and display it */

	gd->relocaddr = addr;
	gd->start_addr_sp = addr_sp;
	gd->reloc_off = addr - _TEXT_BASE;
	debug("relocation Offset is: %08lx\n", gd->reloc_off);
	memcpy(id, (void *)gd, sizeof(gd_t));

	profile_dbg("----------Profile dbg on <%s> Line:%d-----before relocate_code()----\n", __FUNCTION__,__LINE__);
	relocate_code(addr_sp, id, addr);
	///board_init_r ( id, addr);

	/* NOTREACHED - relocate_code() does not return */
}

#if !defined(CONFIG_SYS_NO_FLASH)
static char *failed = "*** failed ***\n";
#endif

/*
 ************************************************************************
 *
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, BSS has been cleared, the stack size in not
 * that critical any more, etc.
 *
 ************************************************************************
 */
#ifdef S3921_FT_NOR_TEST
 
//#define ADDR_LOAD_BMEDIA_SEE 0x85c00200
//#define ADDR_LOAD_BMEDIA_SEE_UBO_START 0x85c001c0
#define ADDR_LOAD_BMEDIA_SEE 0x86000200
#define ADDR_LOAD_BMEDIA_SEE_UBO_START 0x86000200

 void StartupBMediaSee(void)
{
	u32 addr = ADDR_LOAD_BMEDIA_SEE;

	addr = (addr & 0x0FFFFFFF) | 0x80000000;
	/* start see */
	printf("\n/********Start see....********/\n");

	*(volatile u32 *)(0x18000200) = addr;
	*(volatile u32 *)(0x18000220) = 0x00; //init see
	*(volatile u32 *)(0x18000220) |= 0x2; 
}

void S3921_FT_NorFlash_Boot_Test()
{
	printf("\nStart to copy SeeCode from NorFlash to memory....\n");
	memcpy((void *)(ADDR_LOAD_BMEDIA_SEE_UBO_START), (void *)see_bloader3_data, sizeof(see_bloader3_data));

#if 0//you can open here if you want to verify SeeCode integration
	printf("\nStart to verify SeeCode integration....\n");
	PartitionDataChk(ADDR_LOAD_BMEDIA_SEE_UBO_START);
#endif

	StartupBMediaSee();
	while(1);
}
#endif


/* Globle  variable for CPU Process Monitor Counter Value  */
unsigned int ali_g_pm_count = 0;

unsigned int ali_g_per_cell_delay = 0;

void board_init_r(gd_t *id, ulong dest_addr)
{
	ulong malloc_start;

    int i, j , retry_cnt;  //PM value
    unsigned int k, RingOSCDelay, PerCellDelay;  //PM value

#if !defined(CONFIG_SYS_NO_FLASH)
	ulong flash_size;
#endif

	profile_dbg("--------Profile dbg on <%s> Line:%d-----enter board_init_r()----\n", __FUNCTION__,__LINE__);
	gd = id;

	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");


	monitor_flash_len = _end_ofs;

	/* Enable caches */
#if !defined(ALI_ARM_STB)
	enable_caches(); //enable cache will result hangup, so I comment it out now. David@2013/06/30
#endif


#ifdef CONFIG_SET_CPU_FREQ
        SetCPUFreq(CA9_CLOCK_SELECT_1000M);
        printf("----Set CPU Freqency to 1.0G Hz---\n");
#endif
	debug("monitor flash len: %08lX\n", monitor_flash_len);
	board_init();	/* Setup chipselects */
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#ifdef CONFIG_CLOCKS
	set_cpu_clk_info(); /* Setup clock information */
#endif
#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif

	printf("Now running in RAM - U-Boot at: %08lx\n", dest_addr);

#ifdef BOOTLOADER_VERSION
	printf("%s\n", BOOTLOADER_VERSION);
#endif

#ifdef CONFIG_LOGBUFFER
	logbuff_init_ptrs();
#endif
#ifdef CONFIG_POST
	post_output_backlog();
#endif

	/* The Malloc area is immediately below the monitor copy in DRAM */
	malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_init (malloc_start, TOTAL_MALLOC_LEN);

#if !defined(CONFIG_SYS_NO_FLASH)
	puts("Flash: ");

	flash_size = flash_init();
	if (flash_size > 0) {
# ifdef CONFIG_SYS_FLASH_CHECKSUM
		char *s = getenv("flashchecksum");

		print_size(flash_size, "");
		/*
		 * Compute and print flash CRC if flashchecksum is set to 'y'
		 *
		 * NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
		 */
		if (s && (*s == 'y')) {
			printf("  CRC: %08X", crc32(0,
				(const unsigned char *) CONFIG_SYS_FLASH_BASE,
				flash_size));
		}
		putc('\n');
# else	/* !CONFIG_SYS_FLASH_CHECKSUM */
		print_size(flash_size, "\n");
# endif /* CONFIG_SYS_FLASH_CHECKSUM */
	} else {
		puts(failed);
		hang();
	}
#endif

#ifdef S3921_FT_NOR_TEST
	S3921_FT_NorFlash_Boot_Test();
#endif

	profile_dbg("--------Profile dbg on <%s> Line:%d-----before nand_init()----\n", __FUNCTION__,__LINE__);
#if defined(CONFIG_CMD_NAND)
	puts("NAND:  ");
	nand_init();		/* go init the NAND */
#else
	//ali_nand_init();
#endif
	profile_dbg("--------Profile dbg on <%s> Line:%d-----after nand_init()----\n", __FUNCTION__,__LINE__);

#if defined(CONFIG_CMD_ONENAND)
	onenand_init();
#endif

#ifdef CONFIG_GENERIC_MMC
       puts("MMC:   ");
       mmc_initialize(gd->bd);
#endif

#ifdef CONFIG_HAS_DATAFLASH
	AT91F_DataflashInit();
	dataflash_print_info();
#endif

	/* initialize environment */
	env_relocate();

#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI)
	arm_pci_init();
#endif

	/* IP Address */
	gd->bd->bi_ip_addr = getenv_IPaddr("ipaddr");

	stdio_init();	/* get the devices list going. */

	jumptable_init();

#if defined(CONFIG_API)
	/* Initialize API */
	api_init();
#endif

#ifdef CONFIG_CMD_NAND
	ali_nand_bak_blk_recovery();
#endif

	console_init_r();	/* fully init console as a device */

#if defined(CONFIG_ARCH_MISC_INIT)
	/* miscellaneous arch dependent initialisations */
	arch_misc_init();
#endif
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

#ifdef CONFIG_USE_IRQ
	relocate_irq_vector(dest_addr,0x80000000); /*Begin:added by kinson, copy int vector to the low addr*/
	interrupt_init(); 	/*interrupt system init*/	
	enable_interrupts();/* enable interrupt */
#endif

	/* Perform network card initialisation if necessary */
#if defined(CONFIG_DRIVER_SMC91111) || defined (CONFIG_DRIVER_LAN91C96)
	/* XXX: this needs to be moved to board init */
	if (getenv("ethaddr")) {
		uchar enetaddr[6];
		eth_getenv_enetaddr("ethaddr", enetaddr);
		smc_set_mac_addr(enetaddr);
	}
#endif /* CONFIG_DRIVER_SMC91111 || CONFIG_DRIVER_LAN91C96 */

	/* Initialize from environment */
	load_addr = getenv_ulong("loadaddr", 16, load_addr);
#if defined(CONFIG_CMD_NET)
	{
		char *s = getenv("bootfile");

		if (s != NULL)
			copy_filename(BootFile, s, sizeof(BootFile));
	}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
	board_late_init();
#endif

#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
#if defined(CONFIG_CMD_NET)
	puts("Net:   ");
	eth_initialize(gd->bd);
#if defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
#endif

#ifdef CONFIG_POST
	post_run(NULL, POST_RAM | post_bootmode_get(0));
#endif

#if defined(CONFIG_PRAM) || defined(CONFIG_LOGBUFFER)
	/*
	 * Export available size of memory for Linux,
	 * taking into account the protected RAM at top of memory
	 */
	{
		ulong pram = 0;
		uchar memsz[32];

#ifdef CONFIG_PRAM
		pram = getenv_ulong("pram", 10, CONFIG_PRAM);
#endif
#ifdef CONFIG_LOGBUFFER
#ifndef CONFIG_ALT_LB_ADDR
		/* Also take the logbuffer into account (pram is in kB) */
		pram += (LOGBUFF_LEN + LOGBUFF_OVERHEAD) / 1024;
#endif
#endif
		sprintf((char *)memsz, "%ldk", (gd->ram_size / 1024) - pram);
		setenv("mem", (char *)memsz);
	}
#endif

	set_mtdparts();
	set_mac();
	set_boardinfo(0);

	//profile_dbg("--------Profile dbg on <%s> Line:%d-----before Load_Kernel_See()----\n", __FUNCTION__, __LINE__);
	load_bootmedia();

#ifdef LOAD_RECOVERY
	int bootkey = bootkey_check();
	set_recovery_bootkey(bootkey);

	struct bootloader_message boot;
	memset(&boot, 0, sizeof(boot));
    
	if (!bootkey) {
		get_bootloader_message(&boot);  // this may fail, leaving a zeroed structure
	}    

	if (bootkey ||(0 == strcmp(boot.command ,"boot-recovery")))
	{
		load_recovery();
	}
	else
#endif
	{
		if (0 != load_kernel())
		{
#ifdef LOAD_RECOVERY
			printf("Load kernel fail, start loading recovery!\n");
			load_recovery();
#else
			printf("Load kernel fail, but \'LOAD_RECOVERY\' was disabled!\n");
#endif
		}
	}
	//profile_dbg("--------Profile dbg on <%s> Line:%d-----after Load_Kernel_See()----\n", __FUNCTION__, __LINE__);

#if defined(UBOOT_BOOT_MEDIA) && defined(ENABLE_AUTO_PLAYBACK)
	Load_BootMedia();
	StartupBMediaSee();
#else

#ifdef CONFIG_CMD_NAND
	ali_nand_refresh();
#endif

/**************************Begin:PM Value***************************/

/* 
Process Monitor 1 Control & Status Register (300h) 
Address: IOBASE + 0300h 			        Attribute: R/W
Default Value: 00000000h 				Size: 32 bits
Bit	        Description
31:20	Reserved 
19:16	PM_SEL (Default:4?��h0)
15:4	        Reserved
3	        PM SE (Default:0)
2	        PM CLK_GATE
	        0: Clock enable(Default)
	        1: Clock disbaled
1	        Process Monitor RESETJ
	        0:Reset  (Default)
	        1:active
	        Note: Software control
0	        Process Monitor function enable
	        0:function Disable(Default)
	        1: function Enable	        

Process Monitor 1 Status Register (304h) 
Address: IOBASE + 0304h 	                      Attribute: RO	
Default Value: 00000000h 		              Size: 32 bits
Bit	              Description
31:16	      Process Monitor Counter (RO)
15:1	              reserved
0	              Process Monitor Finish (RO)
	              0:not finished 
	              1:Finished	        
*/

    printf("\n\nRingOSCDelay(ns) = 1024 * 37.037 /count\n");
    printf("PerCellDelay(ps) = RingOSCDelay*1000/89/2\n");
    for (i = 0; i < 1; i++)
    { 
        for ( j = 0; j < 5; j++)
        {
            *(volatile unsigned int *)0x18000300 = 0x0;
            *(volatile unsigned int *)0x18000300 |= i << 16;
            *(volatile unsigned int *)0x18000300 |= 0x2;
            *(volatile unsigned int *)0x18000300 |= 0x1;
            retry_cnt = 0;
            
            do
            {
                retry_cnt++;
                k = *(volatile unsigned int *)0x18000304;
            }while (!(k&0x1) || (retry_cnt > 5000));

            if (retry_cnt >= 5000)
            {
                printf("\n\n\n^^^^Read PM Count Value Timeout!\n\n\n");
            }		

            k >>= 16;

           RingOSCDelay = (100 * 1024 * 37037) / k;
           PerCellDelay = (1000 * RingOSCDelay) / 89 / 2;
           
           printf("PM_SEL=%2x, loop=%2d, count=%8d, RingOSCDelay = %d.%d(ns), PerCellDelay = %d.%d(ps)\n\n", i, j, k, RingOSCDelay / 100000, RingOSCDelay % 100000, PerCellDelay / 100000, PerCellDelay % 100000);
        }
    }
    
    ali_g_pm_count = k;
    
    ali_g_per_cell_delay = PerCellDelay;
    
/**************************End:PM Value****************************/


#ifdef CONFIG_ALI_MINIRPC
    /*Begin:added by kinson for minirpc module init*/
	extern int mcomm_ali_modinit(void);
	extern int MiniRpcConnect(void);
    extern int Demo_Service_Test0(void);
	extern int MiniRpcDisConnect(void);
	extern int wait_see_minirpc_prepared(void);
    int m=0;

    //wait for see to been prepared
	wait_see_minirpc_prepared();

	printf("\n\n--------Now mcomm_ali_modinit--------\n");
	mcomm_ali_modinit();

	printf("\n\n--------Now MiniRpcConnect---------\n");
	MiniRpcConnect();

	for(m=0;m<3;m++)
	{
		Demo_Service_Test0();
	}

	printf("\n\n--------Now MiniRpcDisConnect---------\n");
	MiniRpcDisConnect();
	/*End:added by kinson for minirpc module init*/
#endif

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop();
	}
#endif

	/* NOTREACHED - no way out of command loop except booting */
}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
