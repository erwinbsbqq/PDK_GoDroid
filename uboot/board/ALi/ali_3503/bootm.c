/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/byteorder.h>
#include <asm/addrspace.h>

DECLARE_GLOBAL_DATA_PTR;

#define	LINUX_MAX_ENVS		256
#define	LINUX_MAX_ARGS		256
#define ALI_DEBUG

static int	linux_argc;
static char **	linux_argv;

static char **	linux_env;
static char *	linux_env_p;
static int	linux_env_idx;

#define pause()		asm volatile(".word	0x1000FFFF; nop")

static void linux_params_init (ulong start, char * commandline);
static void linux_env_set (char * env_name, char * env_val);

#ifdef HAVE_PART_TBL_PARTITION
#define BOOTPARAMS_MAX_NUM 128
static long boot_argc=0;
static unsigned char *boot_argv[BOOTPARAMS_MAX_NUM];
extern long get_root(unsigned char **root);
extern long get_mtdparts(unsigned char **mtdparts);


static void boot_params_init (void)
{
       char MacAddr[30];

	/* Always ignore argv[0] */
	boot_argv[0] = 0;
	boot_argc = 1;

	get_root(&boot_argv[boot_argc]);
	boot_argc++;

	get_mtdparts(&boot_argv[boot_argc]);
	boot_argc++;

	sprintf(MacAddr,"ethaddr=%s",getenv("ethaddr"));
       boot_argv[boot_argc] = MacAddr;
	boot_argc++;

#if 0 //def ALI_DEBUG
	int i;
	for (i=0; i<boot_argc; i++)
	{
		printf("boot arg%d: %s\n",i,boot_argv[i]);
	}
#endif
}
#endif

int do_bootm_linux(int flag, int argc, char * const argv[], bootm_headers_t *images)
{
	void	(*theKernel) (int, char **, char **, int *);
#ifndef CONFIG_NAND_ALI
	char	*commandline = getenv ("bootargs");
#else
#define COMMAND_LINE_SIZE	1024
       char commandline[COMMAND_LINE_SIZE];
#endif
	char	env_buf[12];
	char	*cp;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* find kernel entry point */
	theKernel = (void (*)(int, char **, char **, int *))images->ep;

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

#ifdef DEBUG
	printf ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong) theKernel);
#endif

#ifdef CONFIG_NAND_ALI
//	sprintf(commandline, "%s mac=%s mem=%dM",getenv("bootargs"),getenv("ethaddr"),get_board_mem_size());
	sprintf(commandline, "%s mac=%s mem=256M",getenv("bootargs"),getenv("ethaddr"));
#endif
	linux_params_init (UNCACHED_SDRAM (gd->bd->bi_boot_params), commandline);	// CKSEG0 0x80000000 fw_arg0 0x9 fw_arg1 0xa966bf98
			
#ifdef CONFIG_MEMSIZE_IN_BYTES
	sprintf (env_buf, "%lu", (ulong)gd->ram_size);
	debug ("## Giving linux memsize in bytes, %lu\n", (ulong)gd->ram_size);
#else
	//sprintf (env_buf, "%lu", (ulong)(gd->ram_size >> 20));
	//debug ("## Giving linux memsize in MB, %lu\n", (ulong)(gd->ram_size >> 20));
	sprintf (env_buf, "256");
#endif /* CONFIG_MEMSIZE_IN_BYTES */

	linux_env_set ("memsize", env_buf);

	sprintf (env_buf, "0x%08X", (uint) UNCACHED_SDRAM (images->rd_start));
	linux_env_set ("initrd_start", env_buf);

	sprintf (env_buf, "0x%X", (uint) (images->rd_end - images->rd_start));
	linux_env_set ("initrd_size", env_buf);

	sprintf (env_buf, "0x%08X", (uint) (gd->bd->bi_flashstart));
	linux_env_set ("flash_start", env_buf);

	sprintf (env_buf, "0x%X", (uint) (gd->bd->bi_flashsize));
	linux_env_set ("flash_size", env_buf);

	cp = getenv("ethaddr");
	if (cp != NULL) {
		linux_env_set("ethaddr", cp);
	}

	cp = getenv("eth1addr");
	if (cp != NULL) {
		linux_env_set("eth1addr", cp);
	}

	/* we assume that the kernel is in place */
	printf ("\nStarting kernel ...\n\n");
#ifdef ALI_DEBUG
//	printf ("=====Start of Kernel params==commandline=%s===\n",commandline);
	int i = 0;
#ifndef HAVE_PART_TBL_PARTITION
	for (i = 0; i < linux_argc; i++)
	{
		printf("%p 0x%08x ",&linux_argv[i],linux_argv[i]);
		printf ("linux_argv[%d]=%s\n", i, linux_argv[i]);
	}
#endif	
	for (i = 0; i < linux_env_idx; i++)
	{
		printf("%p 0x%08x ",&linux_env[i],linux_env[i]);
		printf ("linux_env[%d]=%s\n", i,linux_env[i]);
	}
	printf ("=====End of Kernel params=====\n");
#endif

#if defined(ALI_SOC_M37XX)
	*((volatile u_char *)0xb8000220) &= 0xfd;//reset see before booting kernel
#endif

#ifdef HAVE_PART_TBL_PARTITION
       boot_params_init();
#ifdef ALI_DEBUG
	for (i = 0; i < boot_argc; i++)
	{
		printf ("boot_argv[%d]=%s\n", i, boot_argv[i]);
	}
#endif
	theKernel (boot_argc, boot_argv, linux_env, 0);
#else
	//printf("jump to theKernel:%p \n",theKernel);
	theKernel (linux_argc, linux_argv, linux_env, 0);
#endif
	/* does not return */
	return 1;
}

static void linux_params_init (ulong start, char *line)
{
	char *next, *quote, *argp;

	linux_argc = 1;
	linux_argv = (char **) start;
	linux_argv[0] = 0;
	argp = (char *) (linux_argv + LINUX_MAX_ARGS);

	next = line;

	while (line && *line && linux_argc < LINUX_MAX_ARGS) {
		quote = strchr (line, '"');
		next = strchr (line, ' ');

		while (next != NULL && quote != NULL && quote < next) {
			/* we found a left quote before the next blank
			 * now we have to find the matching right quote
			 */
			next = strchr (quote + 1, '"');
			if (next != NULL) {
				quote = strchr (next + 1, '"');
				next = strchr (next + 1, ' ');
			}
		}

		if (next == NULL) {
			next = line + strlen (line);
		}

		linux_argv[linux_argc] = argp;
		memcpy (argp, line, next - line);
		argp[next - line] = 0;

		argp += next - line + 1;
		linux_argc++;

		if (*next)
			next++;

		line = next;
	}

	linux_env = (char **) (((ulong) argp + 15) & ~15);
	linux_env[0] = 0;
	linux_env_p = (char *) (linux_env + LINUX_MAX_ENVS);
	linux_env_idx = 0;
}

static void linux_env_set (char *env_name, char *env_val)
{
	if (linux_env_idx < LINUX_MAX_ENVS - 1) {
		linux_env[linux_env_idx] = linux_env_p;

		strcpy (linux_env_p, env_name);
		linux_env_p += strlen (env_name);

		strcpy (linux_env_p, "=");
		linux_env_p += 1;

		strcpy (linux_env_p, env_val);
		linux_env_p += strlen (env_val);

		linux_env_p++;
		linux_env[++linux_env_idx] = 0;
	}
}
