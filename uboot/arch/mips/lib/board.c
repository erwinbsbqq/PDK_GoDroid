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

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <version.h>
#include <net.h>
#include <environment.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <spi.h>

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

//#define ALI_DEBUG_ON
#ifdef ALI_DEBUG_ON
    #define ALI_PRINTF   printf
#else
    #define ALI_PRINTF(...)	do{}while(0)
#endif

extern int timer_init(void);

extern int incaip_set_cpuclk(void);

extern ulong uboot_end_data;
extern ulong uboot_end;

ulong monitor_flash_len;

static char *failed = "*** failed ***\n";

#if 0 //defined(DEBUG_NAND_DRV)
#define probe_info printf
#else
#define probe_info(args...) do{}while(0)
#endif

/*
 * mips_io_port_base is the begin of the address space to which x86 style
 * I/O ports are mapped.
 */
unsigned long mips_io_port_base = -1;

int __board_early_init_f(void)
{
	/*
	 * Nothing to do in this dummy implementation
	 */
	return 0;
}
int board_early_init_f(void)
	__attribute__((weak, alias("__board_early_init_f")));

static int init_func_ram(void)
{
#ifdef	CONFIG_BOARD_TYPES
	int board_type = gd->board_type;
#else
	int board_type = 0;	/* use dummy arg */
#endif
	puts("DRAM:  ");

	gd->ram_size = initdram(board_type);
	if (gd->ram_size > 0) {
		print_size(gd->ram_size, "\n");
		return 0;
	}
	puts(failed);
	return 1;
}

static int display_banner(void)
{

	printf("\n\n%s\n\n", version_string);
	return 0;
}

#ifdef CONFIG_NORFLASH_ALI
static void display_flash_config(ulong size)
{
	puts("Flash: ");
	print_size(size, "\n");
}
#endif

static int init_baudrate(void)
{
	gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);
	return 0;
}


/*
 * Breath some life into the board...
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t)(void);

init_fnc_t *init_sequence[] = {
	board_early_init_f,
	timer_init,
	env_init,		/* initialize environment */
#ifdef CONFIG_INCA_IP
	incaip_set_cpuclk,	/* set cpu clock according to env. variable */
#endif
	init_baudrate,		/* initialize baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,
	display_banner,		/* say that we are here */
	checkboard,
	init_func_ram,
	NULL,
};

#ifdef CONFIG_RAMBOOT    
static void clear_bbs(ulong beg, ulong end)
{
    ulong i=0;
    for(i=beg ; i!= end ;i++)
        *(unsigned char*)i = 0 ; //byte align
}
#endif

void board_init_f(ulong bootflag)
{
	gd_t gd_data, *id;
	bd_t *bd;
	init_fnc_t **init_fnc_ptr;
	ulong addr, addr_sp, len = (ulong)&uboot_end - CONFIG_SYS_MONITOR_BASE;
	ulong *s;

	/* Pointer is writable since we allocated a register for it.
	 */
	gd = &gd_data;
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("" : : : "memory");

	memset((void *)gd, 0, sizeof(gd_t));

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0)
			hang();
	}

	printf("U-boot startup address is: 0x%x\n", CONFIG_SYS_MONITOR_BASE);
	ALI_PRINTF("uboot_end addr = 0x%x\n",&uboot_end);
	ALI_PRINTF("uboot_end_data addr= 0x%x\n",&uboot_end_data);
	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 */
	addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size;
#if defined(ALI_SOC_M37XX)
	addr |= 0xa0000000; //force address to uncached area,otherwize mac driver will not work.
#endif

	/* We can reserve some RAM "on top" here.
	 */

	/* round down to next 4 kB limit.
	 */
	addr &= ~(4096 - 1);
	printf("Top of RAM usable for U-Boot at: %08lx\n", addr);

	/* Reserve memory for U-Boot code, data & bss
	 * round down to next 16 kB limit
	 */
	addr -= len;
	addr &= ~(16 * 1024 - 1);

	printf("Reserving %ldk for U-Boot at: %08lx\n", len >> 10, addr);

	 /* Reserve memory for malloc() arena.
	 */
	addr_sp = addr - TOTAL_MALLOC_LEN;
	printf("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, addr_sp);

	/*
	 * (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr_sp -= sizeof(bd_t);
	bd = (bd_t *)addr_sp;
	gd->bd = bd;
	debug("Reserving %zu Bytes for Board Info at: %08lx\n",
			sizeof(bd_t), addr_sp);

	addr_sp -= sizeof(gd_t);
	id = (gd_t *)addr_sp;
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
			sizeof(gd_t), addr_sp);

	/* Reserve memory for boot params.
	 */
	addr_sp -= CONFIG_SYS_BOOTPARAMS_LEN;
	bd->bi_boot_params = addr_sp;
	debug("Reserving %dk for boot params() at: %08lx\n",
			CONFIG_SYS_BOOTPARAMS_LEN >> 10, addr_sp);

	/*
	 * Finally, we set up a new (bigger) stack.
	 *
	 * Leave some safety gap for SP, force alignment on 16 byte boundary
	 * Clear initial stack frame
	 */
	addr_sp -= 16;
	addr_sp &= ~0xF;
	s = (ulong *)addr_sp;
	*s-- = 0;
	*s-- = 0;
	addr_sp = (ulong)s;
	printf("Stack Pointer at: %08lx\n", addr_sp);

	/*
	 * Save local variables to board info struct
	 */
	bd->bi_memstart	= CONFIG_SYS_SDRAM_BASE;	/* start of DRAM */
	bd->bi_memsize	= gd->ram_size;		/* size of DRAM in bytes */
	bd->bi_baudrate	= gd->baudrate;		/* Console Baudrate */

	memcpy(id, (void *)gd, sizeof(gd_t));

#if 0 //def CONFIG_RAMBOOT    
	ALI_PRINTF("board_init_f---before clear_bbs---\n");
	clear_bbs((ulong)(&uboot_end_data),(ulong)(&uboot_end));
	board_init_r ( id, CONFIG_SYS_MONITOR_BASE);  	//DEBUGF in ram
#else
	ALI_PRINTF("board_init_f---before relocate_code---\n");
	relocate_code(addr_sp, id, addr);
#endif
	/* NOTREACHED - relocate_code() does not return */
}

/*
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, BSS has been cleared, the stack size in not
 * that critical any more, etc.
 */

void board_init_r(gd_t *id, ulong dest_addr)
{
#ifdef CONFIG_NORFLASH_ALI
	ulong size;
#endif
	extern void malloc_bin_reloc(void);
#ifndef CONFIG_ENV_IS_NOWHERE
	extern char *env_name_spec;
#endif
	bd_t *bd;

	gd = id;
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

	printf("Now running in RAM - U-Boot at: %08lx\n", dest_addr);

	gd->reloc_off = dest_addr - CONFIG_SYS_MONITOR_BASE;

	monitor_flash_len = (ulong)&uboot_end_data - dest_addr;

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	/*
	 * We have to relocate the command table manually
	 */
	fixup_cmdtable(&__u_boot_cmd_start,
		(ulong)(&__u_boot_cmd_end - &__u_boot_cmd_start));
#endif /* defined(CONFIG_NEEDS_MANUAL_RELOC) */

	/* there are some other pointer constants we must deal with */
#ifndef CONFIG_ENV_IS_NOWHERE
	env_name_spec += gd->reloc_off;
#endif

	bd = gd->bd;

	/* The Malloc area is immediately below the monitor copy in DRAM */
	mem_malloc_init(CONFIG_SYS_MONITOR_BASE + gd->reloc_off -
			TOTAL_MALLOC_LEN, TOTAL_MALLOC_LEN);
	malloc_bin_reloc();

#ifdef CONFIG_NORFLASH_ALI
	/* configure available FLASH banks */
	size = flash_init();
	display_flash_config(size);
	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
	bd->bi_flashsize = size;

#if CONFIG_SYS_MONITOR_BASE == CONFIG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for U-Boot */
#else
	bd->bi_flashoffset = 0;
#endif
#else
	bd->bi_flashstart = 0;
	bd->bi_flashsize = 0;
	bd->bi_flashoffset = 0;
#endif

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#ifdef CONFIG_CMD_NAND
	puts("NAND:  ");
	nand_init();		/* go init the NAND */
#endif

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#if defined(CONFIG_CMD_ONENAND)
	onenand_init();
#endif

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#ifdef CONFIG_GENERIC_MMC
       puts("MMC:   ");
       mmc_initialize(gd->bd);
#endif

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
	/* relocate environment function pointers etc. */
	env_relocate();

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr("ipaddr");

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#if defined(CONFIG_PCI)
	/*
	 * Do pci configuration
	 */
	pci_init();
#endif

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
/** leave this here (after malloc(), environment and PCI are working) **/
	/* Initialize stdio devices */
	stdio_init();

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
	jumptable_init();

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
	/* Initialize the console (after the relocation and devices init) */
	console_init_r();
/** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **/

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
	/* Initialize from environment */
	load_addr = getenv_ulong("loadaddr", 16, load_addr);
#if defined(CONFIG_CMD_NET)
	{
		char *s = getenv("bootfile");

		if (s != NULL)
			copy_filename(BootFile, s, sizeof(BootFile));
	}
#endif

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#ifdef CONFIG_CMD_SPI
	puts("SPI:   ");
	spi_init();		/* go init the SPI */
	puts("ready\n");
#endif

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#ifdef CONFIG_NAND_ALI
  //      Ali_PMI_Init();
#endif

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#if defined(CONFIG_CMD_NET) && !defined(ETH_MANUAL_START)
	puts("Net:   ");
	eth_initialize(gd->bd);
#endif
	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);

	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
#if defined(CONFIG_NAND_ALI) 
	set_mtdparts();
#endif
#if defined(CONFIG_ALI_MMC)
    set_gptparts_emmc();
#endif
	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
	set_mac();
	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
	set_boardinfo(0);
	
	probe_info("%s  line %d  \n", __FUNCTION__,__LINE__);
	profile_dbg("--------Profile dbg on <%s> Line:%d-----before Load_Kernel_See()----\n", __FUNCTION__, __LINE__);
#if defined(LOAD_KERNEL_SEE) 
  #if defined(CONFIG_ALI_MMC)      //emmc
	Load_Kernel_See();
  #elif  defined(CONFIG_NAND_ALI)   //nand
//	load_bootmedia();

	load_kernel();
  #endif
#endif
	profile_dbg("--------Profile dbg on <%s> Line:%d-----after Load_Kernel_See()----\n", __FUNCTION__, __LINE__);


#if defined(UBOOT_BOOT_MEDIA) && defined(ENABLE_AUTO_PLAYBACK)
       Load_BootMedia();
	StartupBMediaSee();
#else
	ali_nand_refresh();
	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;)
		main_loop();
#endif

	/* NOTREACHED - no way out of command loop except booting */
}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;)
		;
}
