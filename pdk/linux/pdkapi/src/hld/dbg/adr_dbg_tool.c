/*****************************************************************************
*	Copyrights(C) 2012 Acer Laboratries inc. All Rights Reserved.
*
*	FILE NAME:		adr_dbg_soc.c
*
*	DESCRIPTION:	Debug SOC module.
*
*	HISTORY:
*						Date 	 Author      Version 	  Notes
*					=========	=========	=========	===========
*					2012-09-30	 Leo.Ma      Ver 1.0	Create File.
*					2012-10-31	 John.CHen   Ver 1.1	Add soc dbg module.
*					2013-05-17	 Leo.Ma      Ver 1.2	Modify the arch.
*****************************************************************************/

#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <ali_soc_common.h>

#include <hld_cfg.h>
#include <hld/dbg/adr_dbg_parser.h>
#include <hld/soc/adr_soc.h>
#include <adr_version.h>
#include<sys/mman.h>  
#include <hld/otp/otp.h>


//#define __ADR_DBG_SOC

#define MAX_MEM_SIZE	(0x3FFFFFFF)



#define SOC_INFO(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(SOC, fmt, ##args); \
			} while(0)

#define SOC_ERR(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(SOC,fmt, ##args); \
			} while(0)

#ifdef __ADR_DBG_SOC
	#define ADR_DBG_SOC_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(SOC, fmt, ##args); \
			} while(0)
#else
	#define ADR_DBG_SOC_PRINT(...)
#endif

typedef enum socdbg_level
{
	DBG_LEVEL_DEFAULT = 0x00,
	DBG_LEVEL_HLD 	  = 0x01,
    DBG_LEVEL_KERNEL = 0x02,
} socdbg_level_e;

INT32 soc_dbg_cmd_get(PARSE_COMMAND_LIST **cmd_list, INT32 *list_cnt);
INT32 ir_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt);
INT32 pan_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 *list_cnt);
INT32 bw_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt);
INT32 osd2_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt);
INT32 adc_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt);
INT32 smc_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt);


static DEBUG_MODULE_LIST debug_module[] = {
	{ { NULL, NULL }, "soc", 0, (GET_CMD_FP)soc_dbg_cmd_get, { NULL, NULL } },	
	{ { NULL, NULL }, "ir", 0, (GET_CMD_FP)ir_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "pan", 0, (GET_CMD_FP)pan_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "fb", 0, (GET_CMD_FP)fb_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "tun", 0, (GET_CMD_FP)tun_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "dmx", 0, (GET_CMD_FP)dmx_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "dmx0", 0, (GET_CMD_FP)dmx0_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "deca", 0, (GET_CMD_FP)deca_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "snd", 0, (GET_CMD_FP)snd_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "decv", 0, (GET_CMD_FP)decv_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "dis", 0, (GET_CMD_FP)dis_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "avsync", 0, (GET_CMD_FP)avsync_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "bw", 0, (GET_CMD_FP)bw_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "osd", 0, (GET_CMD_FP)osd2_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "adc", 0, (GET_CMD_FP)adc_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "smc", 0, (GET_CMD_FP)smc_dbg_cmd_get, { NULL, NULL } },
	{ { NULL, NULL }, "ape", 0, (GET_CMD_FP)ape_dbg_cmd_get, { NULL, NULL } },	
	
};


int soc_dbg_get_num(int argc, char **argv, int *data, char *option)
{
	char *v;
	int base, number, i;

	if (0 == argc)
	{
		SOC_ERR("option \"%s\", Lack of valid argument!\n", option);
		SOC_ERR("Try \"-h\" for more information.\n");
		return ERR_FAILURE;
	}

	for (i = 0; i < argc; i++)
	{
		v = *argv++;
		base = 0;
		if ('0' == *v)
		{
			if ('x' == (0x20 | v[1]) && isxdigit(v[2]))
			{				
				base = 16;
			}
			else
			{				
				base = 8;
			}
		}
		else
		{			
			base = 10;
		}

		if (16 == base && '0' == v[0] && 'x' == (0x20 | v[1]))
		{
			v += 2;
		}

		while (isxdigit(*v))
		{
			number = isdigit(*v) ? *v - '0' : (0x20 | *v) - 'a' + 10;
			if (number >= base)
			{	
				break;
			}
			data[i] = data[i] * base + number;
			v++;
		}

		if (*v && base)
		{
			SOC_ERR("option \"%s\": Argument not a valid number!\n", option);
			return ERR_FAILURE;
		}
	}

	return SUCCESS;
}


ARG_CHK soc_dbg_no_param_preview(int argc, char **argv, char *option)
{
	if (argc != 0)
	{
		SOC_ERR("option \"%s\" should not be with any argument(s)!\n", option);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}


ARG_CHK soc_dbg_one_param_preview(int argc, char **argv, char *option)
{
	int data = 0;

	if (argc != 1)
	{
		SOC_ERR("Option \"-s\": Argument count must be 1!\n");
		return DBG_ARG_INVALID;
	}

	if (SUCCESS != soc_dbg_get_num(argc, argv, &data, option))
	{
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}


ARG_CHK soc_dbg_two_param_preview(int argc, char **argv, char *option)
{
	int data[2];

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return DBG_ARG_INVALID;
	}

	memset(data, 0x00, sizeof(data));
	if (SUCCESS != soc_dbg_get_num(argc, argv, data, option))
	{
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}


static void soc_dbg_show_help(int argc, char **argv)
{
	SOC_INFO("Module Name:\n");
	SOC_INFO("\tsoc - soc debug diagnostic module.\n\n");

	SOC_INFO("Synopsis:\n");
	SOC_INFO("\tadrdbg -cmd [arguments] ...\n\n");

	SOC_INFO("Description:\n");
	SOC_INFO("\t-h, --help\n");
	SOC_INFO("\t\tself description of this diagnostic module.\n\n");

	SOC_INFO("\t-ls, --list\n");
	SOC_INFO("\t\tlist all modules.\n\n");

	SOC_INFO("\t-s, --system\n");
	SOC_INFO("\t\tshow system info.\n\n");

	SOC_INFO("\t-rd8, --read8 addr\n");
	SOC_INFO("\t\tread 1 bytes data from address.\n");
	SOC_INFO("\t\taddr    \t\t--address to read.\n\n");

	SOC_INFO("\t-rd16, --read16 addr\n");
	SOC_INFO("\t\tread 2 bytes data from address.\n");
	SOC_INFO("\t\taddr    \t\t--address to read.\n\n");

	SOC_INFO("\t-rd32, --read32 addr\n");
	SOC_INFO("\t\tread 4 bytes data from address.\n");
	SOC_INFO("\t\taddr    \t\t--address to read.\n\n");

	SOC_INFO("\t-wr8, --write8 addr data\n");
	SOC_INFO("\t\twrite 1 bytes data to address.\n");
	SOC_INFO("\t\taddr    \t\t--address to write.\n");
	SOC_INFO("\t\tdata    \t\t--data to write to address.\n\n");

	SOC_INFO("\t-wr16, --write16 addr data\n");
	SOC_INFO("\t\twrite 2 bytes data to address.\n");
	SOC_INFO("\t\taddr    \t\t--address to write.\n");
	SOC_INFO("\t\tdata    \t\t--data to write to address.\n\n");

	SOC_INFO("\t-wr32, --write32 addr data\n");
	SOC_INFO("\t\twrite 4 bytes data to address.\n");
	SOC_INFO("\t\taddr    \t\t--address to write.\n");
	SOC_INFO("\t\tdata    \t\t--data to write to address.\n\n");

	SOC_INFO("\t-d, --dump addr size\n");
	SOC_INFO("\t\tdump size bytes data from address.\n");
	SOC_INFO("\t\taddr    \t\t--address to dump.\n");
	SOC_INFO("\t\tsize    \t\t--data size(bytes) to dump from address.\n\n");

	SOC_INFO("\t-mem_rd32, --mem_read32 addr\n");
	SOC_INFO("\t\tread 4 bytes data from addr of memory.\n");
	SOC_INFO("\t\taddr    \t\t--address to read.\n\n");

	SOC_INFO("\t-mem_wr32, --mem_write32 addr data\n");
	SOC_INFO("\t\twrite 4 bytes data to addr of memory.\n");
	SOC_INFO("\t\taddr    \t\t--address to write.\n");
	SOC_INFO("\t\tdata    \t\t--data to write to address.\n\n");

	SOC_INFO("\t-mem_d, --mem_dump addr size\n");
	SOC_INFO("\t\tdump size bytes data from addr of memory.\n");
	SOC_INFO("\t\taddr    \t\t--address to dump.\n");
	SOC_INFO("\t\tsize    \t\t--data size(bytes) to dump from addr of memory.\n\n");

	SOC_INFO("\t-l, --level level\n");
	SOC_INFO("\t\tset soc_dbg debugging printout level (default is 0).\n");
	SOC_INFO("\t\tlevel 0 \t\t--no printout.\n");
	SOC_INFO("\t\tlevel 1 \t\t--show HLD printout infomation.\n");
	SOC_INFO("\t\tlevel 2 \t\t--show Kernel printout infomation.\n");
	SOC_INFO("\t\tlevel 3 \t\t--show HLD & Kernel printout infomation.\n\n");

	SOC_INFO("\t-see_prf --see_printf disable or enable flag\n");
	SOC_INFO("\t\tdisable or enable the see printf function (default is enable).\n");
	SOC_INFO("\t\tflag 1 \t\t -- enable the function. \n");
	SOC_INFO("\t\tflag 0 \t\t -- disable the function. \n\n");

	SOC_INFO("\t-see_hit\n");
	SOC_INFO("\t\tcheck the status of see heart beat.\n\n");

	SOC_INFO("\t-see_exp\n");
	SOC_INFO("\t\tEnable SEE exception mornitor.\n\n");	

	SOC_INFO("\nUsage example:\n");
	SOC_INFO("\tadrdbg -ap -h\n");
	SOC_INFO("\tadrdbg -ap -ls\n");
	SOC_INFO("\tadrdbg -ap -s\n");
	SOC_INFO("\tadrdbg -ap -rd8 0x18000434\n");
	SOC_INFO("\tadrdbg -ap -rd16 0x18000434\n");
	SOC_INFO("\tadrdbg -ap -rd32 0x18000434\n");
	SOC_INFO("\tadrdbg -ap -wr8 0x18000434 0xde\n");
	SOC_INFO("\tadrdbg -ap -wr16 0x18000434 0x9abc\n");
	SOC_INFO("\tadrdbg -ap -wr32 0x18000434 0x12345678\n");
	SOC_INFO("\tadrdbg -ap -d 0x18000434 8\n");
	SOC_INFO("\tadrdbg -ap -mem_rd32 0x03fff154\n");
	SOC_INFO("\tadrdbg -ap -mem_wr32 0x03fff154 0x12345678\n");
	SOC_INFO("\tadrdbg -ap -mem_d 0x03fff154 64\n");
	SOC_INFO("\tadrdbg -ap -l 3\n");
	SOC_INFO("\tadrdbg -ap -see_prf 0\n");
	SOC_INFO("\tadrdbg -ap -see_hit\n");
	SOC_INFO("\tadrdbg -ap -see_exp\n");	
}

static void soc_dbg_show_list(int argc, char **argv)
{
	unsigned char found;
	struct list_head* cur_mod_list;
	struct list_head* next_mod_list;
	DEBUG_MODULE_LIST *mod_list;

	mod_list = NULL;
	found = 0;

	list_for_each_safe(cur_mod_list, next_mod_list, &dbg_mod_list)
	{
		mod_list = list_entry(cur_mod_list, DEBUG_MODULE_LIST, list);
		if (NULL == mod_list)
		{
			ADR_DBG_SOC_PRINT("[DBGTOOL] Invalid module list!\n");
			break;
		}

		if (mod_list->registered)
		{
			if (!found)
				SOC_INFO("Debug module name list:\n");

			SOC_INFO("\"%s\"\n", mod_list->name);
			found = 1;
		}
	}

	if (!found)
	{
		SOC_ERR("No debug module registered!\n");
	}

	return;
}

static void soc_app_show_list(int argc, char **argv)
{
	unsigned char found;
	struct list_head* cur_mod_list;
	struct list_head* next_mod_list;
	DEBUG_MODULE_LIST *mod_list;

	mod_list = NULL;
	found = 0;

	list_for_each_safe(cur_mod_list, next_mod_list, &app_mod_list)
	{
		mod_list = list_entry(cur_mod_list, DEBUG_MODULE_LIST, list);
		if (NULL == mod_list)
		{
			ADR_DBG_SOC_PRINT("[DBGTOOL] Invalid module list!\n");
			break;
		}

		if (mod_list->registered)
		{
			if (!found)
				SOC_INFO("Debug module name list:\n");

			SOC_INFO("\"%s\"\n", mod_list->name);
			found = 1;
		}
	}

	if (!found)
	{
		SOC_ERR("No debug module registered!\n");
	}

	return;
}

static void soc_dbg_show_system(int argc, char **argv)
{
	int fd = -1;
	int ret = 0;
	struct utsname testbuff;
	struct soc_memory_map smm;
	unsigned char see_ver[SEE_VER_MAX_LEN];


	SOC_INFO("Debugtool system\n");

	fd=uname(&testbuff);
	if(fd<0)
	{
		perror("uname");
	}
	else
	{
		SOC_INFO("kernel version  \t\t\t : %s %s\n", testbuff.sysname, testbuff.release);
	}
	SOC_INFO("main version \t\t\t\t : %s\n", MAIN_VER);

	memset(see_ver, 0, sizeof(see_ver));
	ret = soc_get_see_ver(see_ver);
	if (SUCCESS != ret)
	{
		SOC_ERR("ret = %d\n", ret);
		perror("soc_get_see_ver");
	}
	else
	{
		SOC_INFO("see version \t\t\t\t : %s\n", see_ver);
	}


	SOC_INFO("chip id   \t\t\t\t : 0x%08x\n", soc_get_chip_id());
	SOC_INFO("cpu clock   \t\t\t\t : %d MHz\n", soc_get_cpu_clock());
	SOC_INFO("dram clock   \t\t\t\t : %d MHz\n", soc_get_dram_clock());
	SOC_INFO("dram size   \t\t\t\t : %d M\n", soc_get_dram_size()); 


	ret = soc_get_memory_map(&smm);
	if (SUCCESS != ret)
	{
		SOC_ERR("ret = %d\n", ret);
		perror("soc_get_memory_map");
	}
	else
	{		
		SOC_INFO("main_start   \t\t\t\t : 0x%08x\n", (unsigned int)smm.main_start);
		SOC_INFO("main_end   \t\t\t\t : 0x%08x\n", (unsigned int)smm.main_end);

		SOC_INFO("fb_start   \t\t\t\t : 0x%08x\n", (unsigned int)smm.fb_start);
		SOC_INFO("osd_bk   \t\t\t\t : 0x%08x\n", (unsigned int)smm.osd_bk);

		SOC_INFO("see_dmx_src_buf_start  \t\t\t : 0x%08x\n", (unsigned int)smm.see_dmx_src_buf_start);
		SOC_INFO("see_dmx_src_buf_end  \t\t\t : 0x%08x\n", (unsigned int)smm.see_dmx_src_buf_end);

		SOC_INFO("see_dmx_decrypto_buf_start  \t\t : 0x%08x\n", (unsigned int)smm.see_dmx_decrypto_buf_start);
		SOC_INFO("see_dmx_decrypto_buf_end  \t\t : 0x%08x\n", (unsigned int)smm.see_dmx_decrypto_buf_end);

		SOC_INFO("dmx_start  \t\t\t\t : 0x%08x\n", (unsigned int)smm.dmx_start);
		SOC_INFO("dmx_top  \t\t\t\t : 0x%08x\n", (unsigned int)smm.dmx_top);

		SOC_INFO("see_start  \t\t\t\t : 0x%08x\n", 	(unsigned int)smm.see_start);
		SOC_INFO("see_top  \t\t\t\t : 0x%08x\n", (unsigned int)smm.see_top);

		SOC_INFO("video_start  \t\t\t\t : 0x%08x\n", (unsigned int)smm.video_start);
		SOC_INFO("video_top  \t\t\t\t : 0x%08x\n", (unsigned int)smm.video_top);

		SOC_INFO("frame  \t\t\t\t\t : 0x%08x\n", (unsigned int)smm.frame);
		SOC_INFO("frame_size  \t\t\t\t : 0x%08x\n", (unsigned int)smm.frame_size);

		SOC_INFO("vcap_fb  \t\t\t\t : 0x%08x\n", (unsigned int)smm.vcap_fb);
		SOC_INFO("vcap_fb_size  \t\t\t\t : 0x%08x\n", (unsigned int)smm.vcap_fb_size);

		SOC_INFO("vdec_vbv_start  \t\t\t : 0x%08x\n", (unsigned int)smm.vdec_vbv_start);
		SOC_INFO("vdec_vbv_len  \t\t\t\t : 0x%08x\n", (unsigned int)smm.vdec_vbv_len);

		SOC_INFO("shared_start  \t\t\t\t : 0x%08x\n", (unsigned int)smm.shared_start);
		SOC_INFO("shared_top  \t\t\t\t : 0x%08x\n", (unsigned int)smm.shared_top);

		SOC_INFO("reserved_mem_addr  \t\t\t : 0x%08x\n", (unsigned int)smm.reserved_mem_addr);
		SOC_INFO("reserved_mem_size  \t\t\t : 0x%08x\n", (unsigned int)smm.reserved_mem_size);

		SOC_INFO("media_mem_addr  \t\t\t : 0x%08x\n", (unsigned int)smm.media_buf_addr);
		SOC_INFO("media_mem_size  \t\t\t : 0x%08x\n", (unsigned int)smm.media_buf_size);

		SOC_INFO("mcapi_mem_addr  \t\t\t : 0x%08x\n", (unsigned int)smm.mcapi_buf_addr);
		SOC_INFO("mcapi_mem_size  \t\t\t : 0x%08x\n", (unsigned int)smm.mcapi_buf_size);				
	}


	if (fd > 0)
	{
		close(fd);
	}
}


static void soc_dbg_read8(int argc, char **argv)
{
	unsigned addr = 0;
	unsigned char data = 0;

	if (argc != 1)
	{
		SOC_ERR("Option \"-s\": Argument count must be 1!\n");
		return;
	}

	if (SUCCESS != soc_dbg_get_num(argc, argv, &addr, "rd"))
	{
		return;
	}

	if (ERR_FAILURE != soc_read8(addr, (unsigned char *)&data, 1))
	{
		SOC_INFO("read *0x%08x = 0x%02x\n", addr, data);
	}

	return;
}


static void soc_dbg_read16(int argc, char **argv)
{
	unsigned addr = 0;
	unsigned short data = 0;

	if (argc != 1)
	{
		SOC_ERR("Option \"-s\": Argument count must be 1!\n");
		return;
	}

	if (SUCCESS != soc_dbg_get_num(argc, argv, &addr, "rd"))
	{
		return;
	}

	if (ERR_FAILURE != soc_read16(addr, (unsigned char *)&data, 1))
	{
		SOC_INFO("read *0x%08x = 0x%04x\n", addr, data);
	}

	return;
}


static void soc_dbg_read32(int argc, char **argv)
{
	unsigned addr= 0;
	unsigned int data = 0;

	if (argc != 1)
	{
		SOC_ERR("Option \"-s\": Argument count must be 1!\n");
		return;
	}

	if (SUCCESS != soc_dbg_get_num(argc, argv, &addr, "rd"))
	{
		return;
	}

	if (ERR_FAILURE != soc_read32(addr, (unsigned char *)&data, 1))
	{
		SOC_INFO("read *0x%08x = 0x%08x\n", addr, data);
	}

	return;
}


static void soc_dbg_write8(int argc, char **argv)
{
	unsigned data[2];

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return;
	}

	memset(data, 0x00, sizeof(data));
	if (SUCCESS != soc_dbg_get_num(argc, argv, data, "wr"))
	{
		return;
	}

	if (ERR_FAILURE != soc_write8(data[0], (unsigned char *)(data + 1), 1))
	{
		SOC_INFO("write *0x%08x = 0x%02x\n", data[0], (unsigned char)data[1]);
	}

	return;
}


static void soc_dbg_write16(int argc, char **argv)
{
	unsigned data[2];

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return;
	}
	
	memset(data, 0x00, sizeof(data));
	if (SUCCESS != soc_dbg_get_num(argc, argv, data, "wr"))
	{
		return;
	}

	if (ERR_FAILURE != soc_write16(data[0], (unsigned char *)(data + 1), 1))
	{
		SOC_INFO("write *0x%08x = 0x%04x\n", data[0], (unsigned short)data[1]);
	}

	return;
}


static void soc_dbg_write32(int argc, char **argv)
{
	unsigned data[2];

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return;
	}

	memset(data, 0x00, sizeof(data));
	if (SUCCESS != soc_dbg_get_num(argc, argv, data, "wr"))
	{
		return;
	}	

	if (ERR_FAILURE != soc_write32(data[0], (unsigned char *)(data + 1), 1))
	{
		SOC_INFO("write *0x%08x = 0x%08x\n", data[0], data[1]);
	}

	return;
}


static void soc_dbg_per_read32(int argc, char **argv)
{
	unsigned addr= 0;
	unsigned int data = 0;

	if (argc != 1)
	{
		SOC_ERR("Option \"-s\": Argument count must be 1!\n");
		return;
	}

	if (SUCCESS != soc_dbg_get_num(argc, argv, &addr, "per_rd32"))
	{
		return;
	}

	if (ERR_FAILURE != soc_per_read32(addr, (unsigned char *)&data, 1))
	{
		SOC_INFO("read *0x%08x = 0x%08x\n", addr, data);
	}

	return;
}


static void soc_dbg_per_write32(int argc, char **argv)
{
	unsigned data[2];

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return;
	}

	memset(data, 0x00, sizeof(data));
	if (SUCCESS != soc_dbg_get_num(argc, argv, data, "per_wr32"))
	{
		return;
	}	

	if (ERR_FAILURE != soc_per_write32(data[0], (unsigned char *)(data + 1), 1))
	{
		SOC_INFO("write *0x%08x = 0x%08x\n", data[0], data[1]);
	}

	return;
}




static void soc_dbg_set_level(int argc, char **argv)
{
	int level = 0;

	if (argc != 1)
	{
		SOC_ERR("Option \"-s\": Argument count must be 1!\n");
		return;
	}

	if (SUCCESS != soc_dbg_get_num(argc, argv, &level, "l"))
	{
		return;
	}

	level &= DBG_LEVEL_KERNEL;
	SOC_INFO("level = %d\n", level);
	soc_set_level(level);

	return;
}


static void soc_dbg_dump_data(int argc, char **argv)
{
	int file_handle;
	unsigned dump_paras[2], wr_size, size, addr, i;
	unsigned char *buffer;

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return;
	}

	memset(dump_paras, 0x00, sizeof(dump_paras));
	if (SUCCESS != soc_dbg_get_num(argc, argv, dump_paras, "d"))
	{
		return;
	}

	addr = dump_paras[0];
	size = dump_paras[1];

	buffer = (unsigned char *)malloc(size);
	if (!buffer)
	{
		SOC_ERR("[%s %d ], Fail, malloc %ld size buffer\n", __FUNCTION__, __LINE__, size);
		perror("malLoc");
		return;
	}
	memset(buffer, 0xff, size);

	if (ERR_FAILURE == soc_read8(addr, buffer, size))
	{
		goto END;
	}

	SOC_INFO("dump %d bytes form 0x%08x : ", size, addr);
	for (i = 0; i < size; i++)
	{
		if (0 == (i % 16))
		{
			SOC_INFO("\n%08xh: ", addr);
		}
		SOC_INFO("%02x ", buffer[i]);
		addr += 1;
	}
	SOC_INFO("\n");

	remove("data");
	file_handle = open("data", O_RDWR | O_CREAT, 666);
	if (file_handle < 0)
	{
		perror("open");
		goto END;
	}

	wr_size = write(file_handle, buffer, size);
	if (wr_size != size)
	{
		SOC_ERR("[%s %d ], Fail, write %ld(want %ld) size to file!\n",
			__FUNCTION__, __LINE__, wr_size, size);
		perror("write");
	}

	if (file_handle > 0)
	{
		close(file_handle);
	}

END:

	if (buffer)
	{
		free(buffer);
		buffer = NULL;
	}

	return;
}


static void soc_dbg_mem_read32(int argc, char **argv)
{
	unsigned int addr= 0;
	unsigned int data = 0;	
	unsigned char * map_base = NULL;  	
	int fd = -1;  	   	
  	unsigned int pa_offset = 0;
  	unsigned int offset = 0;
  	unsigned int length = 4;  	  	
  	  	

	if (argc != 1)
	{
		SOC_ERR("Option \"-s\": Argument count must be 1!\n");
		return;
	}

	if (SUCCESS != soc_dbg_get_num(argc, argv, &addr, "mem_rd32"))
	{
		return;
	}	
	  
	fd = open("/dev/mem", O_RDONLY);
  	if (fd == -1)
  	{
	  	perror("open");
	  	return;  
  	}	
	

	if ((addr + length -1) > MAX_MEM_SIZE)	/* max file size */
	{
		SOC_INFO("offset is past end of file, offset = 0x%08x, MAX_MEM_SIZE = 0x%08x\n", 
		  	(addr + length - 1), MAX_MEM_SIZE);	
		return;
	}

	/* offset for mmap() must be page aligned */	
	pa_offset = addr & ~(getpagesize() - 1);
	offset = pa_offset;	
	if (ALI_C3921 == soc_get_chip_id())
	{		
		offset |= 0x80000000;
	}
	/*
	SOC_INFO("[%s %d], addr = 0x%x, pa_offset = 0x%x, offset = 0x%x, len = 0x%x\n", 
		__FUNCTION__, __LINE__, addr, pa_offset, offset, (length + addr - pa_offset));	
	*/
		
	map_base = mmap(NULL, (length + addr - pa_offset), PROT_READ,
				MAP_SHARED, fd, offset);	
	if (map_base == NULL)
	{
		perror("mmap");
		close(fd);
		return;
	}	
	
	data = *(unsigned int *)(map_base + addr - pa_offset);
	SOC_INFO("read *0x%x = 0x%08x\n", addr, data);		

	close(fd);		
	munmap(map_base, (length + addr - pa_offset));  
	

	return;
}


static void soc_dbg_mem_write32(int argc, char **argv)
{
	unsigned data[2];
	unsigned char * map_base = NULL;  	
	int fd = -1;  	  	
  	unsigned int pa_offset = 0;
  	unsigned int offset = 0;
  	unsigned int length = 4;  	  	
	

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return;
	}

	memset(data, 0x00, sizeof(data));
	if (SUCCESS != soc_dbg_get_num(argc, argv, data, "mem_wr32"))
	{
		return;
	}	

	fd = open("/dev/mem", O_RDWR);
  	if (fd == -1)
  	{
	  	perror("open");
	  	return;  
  	}	

	if ((data[0] + length - 1) > MAX_MEM_SIZE)
	{
		SOC_INFO("offset is past end of file, offset = 0x%08x, MAX_MEM_SIZE = 0x%08x\n", 
		  	(data[0] + length -1), MAX_MEM_SIZE);	
		close(fd);
		return;
	}

	/* offset for mmap() must be page aligned */	
	pa_offset = data[0] & ~(sysconf(_SC_PAGE_SIZE) - 1);
	offset = pa_offset;
	if (ALI_C3921 == soc_get_chip_id())
	{		
		offset |= 0x80000000;
	}
	/*
	SOC_INFO("[%s %d], addr = 0x%x, pa_offset = 0x%x, offset = 0x%x, len = 0x%x\n", 
		__FUNCTION__, __LINE__, data[0], pa_offset, offset, (length + data[0] - pa_offset));		
	*/
	
	map_base = mmap(NULL, (length + data[0] - pa_offset), PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, offset);	
	if (map_base == NULL)
	{
		perror("mmap");
		close(fd);
		return;
	}
	
	*(unsigned int *)(map_base + data[0] - pa_offset) = data[1];
	SOC_INFO("write *0x%x = 0x%08x\n", data[0], data[1]);		

	close(fd);		
	munmap(map_base, (length + data[0] - pa_offset));  
	

	return;
}


static void soc_dbg_mem_dump_data(int argc, char **argv)
{
	int file_handle;
	unsigned int dump_paras[2], wr_size, size = 4, addr, i;
	unsigned char *buffer;
	unsigned char * map_base = NULL;  	
	int fd = -1;  	  	
  	unsigned int pa_offset = 0;  	
  	unsigned int offset = 0;
  	

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return;
	}

	memset(dump_paras, 0x00, sizeof(dump_paras));
	if (SUCCESS != soc_dbg_get_num(argc, argv, dump_paras, "d"))
	{
		return;
	}

	addr = dump_paras[0];
	size = dump_paras[1];

	buffer = (unsigned char *)malloc(size);
	if (!buffer)
	{
		SOC_ERR("[%s %d ], Fail, malloc %ld size buffer\n", __FUNCTION__, __LINE__, size);
		perror("malLoc");
		return;
	}
	memset(buffer, 0xff, size);

	fd = open("/dev/mem", O_RDONLY);
  	if (fd == -1)
  	{
	  	perror("open");
	  	
		free(buffer);
		buffer = NULL;
		
	  	return;  
  	}		
	
	if ((addr + size -1) > MAX_MEM_SIZE)	/* max file size */
	{
		SOC_INFO("offset is past end of file, offset = 0x%08x, MAX_MEM_SIZE = 0x%08x\n", 
		  	(addr + size - 1), MAX_MEM_SIZE);	
		goto END;
	}

	/* offset for mmap() must be page aligned */	
	pa_offset = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
	offset = pa_offset;	
	if (ALI_C3921 == soc_get_chip_id())
	{		
		offset |= 0x80000000;
	}
	/*
	SOC_INFO("[%s %d], addr = 0x%x, pa_offset = 0x%x, offset = 0x%x, len = 0x%x\n", 
		__FUNCTION__, __LINE__, addr, pa_offset, offset, (size + addr - pa_offset));	
	*/
	
	map_base = mmap(NULL, (size + addr - pa_offset), PROT_READ,
				MAP_SHARED, fd, offset);	
	if (map_base == NULL)
	{
		perror("mmap");
		goto END;
	}	  
	
	memcpy(buffer, (map_base + addr - pa_offset), size);
	close(fd);		
	munmap(map_base, (size + addr - pa_offset));  

	SOC_INFO("dump %d bytes form 0x%08x : ", size, addr);
	for (i = 0; i < size; i++)
	{
		if (0 == (i % 16))
		{
			SOC_INFO("\n%08xh: ", addr);
		}
		SOC_INFO("%02x ", buffer[i]);
		addr += 1;
	}
	SOC_INFO("\n");

	remove("data");
	file_handle = open("data", O_RDWR | O_CREAT, 666);
	if (file_handle < 0)
	{
		perror("open");
		goto END;
	}

	wr_size = write(file_handle, buffer, size);
	if (wr_size != size)
	{
		SOC_ERR("[%s %d ], Fail, write %ld(want %ld) size to file!\n",
			__FUNCTION__, __LINE__, wr_size, size);
		perror("write");
	}

	if (file_handle > 0)
	{
		close(file_handle);
	}

END:

	if (buffer)
	{
		free(buffer);
		buffer = NULL;
	}

	close(fd);

	return;
}


static void soc_dbg_otp_read32(int argc, char **argv)
{
	unsigned addr = 0, data = 0;
	int ret = 0;

	if (argc != 1)
	{
		SOC_ERR("Option \"-s\": Argument count must be 1!\n");
		return;
	}

	if (SUCCESS != soc_dbg_get_num(argc, argv, &addr, "otp_rd32"))
	{
		return;
	}

	ret = otp_read(addr * 4, (unsigned char *)&data, 4);
	if (4 == ret)
	{
		SOC_INFO("read *0x%02x = 0x%08x\n", addr, data);
	}
	else
	{
		SOC_ERR("[ %s %d ] error, read %d bytes(wanted %d)!\n", __FUNCTION__, __LINE__, ret, 4);
	}
}


static void soc_dbg_otp_write32(int argc, char **argv)
{
	unsigned input_data[2], addr = 0, data = 0;
	int ret = 0, i = 0;

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return;
	}

	memset(input_data, 0x00, sizeof(input_data));
	if (SUCCESS != soc_dbg_get_num(argc, argv, input_data, "otp_wr32"))
	{
		return;
	}

	addr = input_data[0];
	data = input_data[1];

	ret = otp_write((unsigned char *)&data, addr * 4, 4);
	if (4 == ret)
	{
		SOC_INFO("write *0x%02x = 0x%08x\n", addr, data);
	}
	else
	{
		SOC_ERR("[ %s %d ] error, write %d bytes(wanted %d)!\n", __FUNCTION__, __LINE__, ret, 4);
	}
}


static void soc_dbg_otp_dump_data(int argc, char **argv)
{
	int file_handle;
	unsigned int dump_paras[2], addr;
	int size, i, wr_size;
	unsigned char *buffer;

	if (argc != 2)
	{
		SOC_ERR("Option \"-s\": Argument count must be 2!\n");
		return;
	}

	memset(dump_paras, 0x00, sizeof(dump_paras));
	if (SUCCESS != soc_dbg_get_num(argc, argv, dump_paras, "otp_d"))
	{
		return;
	}

	addr = dump_paras[0];
	size = dump_paras[1];

	buffer = (unsigned char *)malloc(size);
	if (!buffer)
	{
		SOC_ERR("[%s %d ], Fail, malloc %ld size buffer\n", __FUNCTION__, __LINE__, size);
		perror("malLoc");
		return;
	}
	memset(buffer, 0xff, size);

	if (size != otp_read(addr * 4, buffer, size))
	{
		goto END;
	}

	SOC_INFO("dump %d bytes form 0x%08x : ", size, addr);
	for (i = 0; i < size; i++)
	{
		if (0 == (i % 4))
		{
			SOC_INFO("\n%02xh: ", addr);
		addr += 1;
		}
		SOC_INFO("%02x ", buffer[i]);
	}
	SOC_INFO("\n");

	remove("data");
	file_handle = open("data", O_RDWR | O_CREAT, 666);
	if (file_handle < 0)
	{
		perror("open");
		goto END;
	}

	wr_size = write(file_handle, buffer, size);
	if (wr_size != size)
	{
		SOC_ERR("[%s %d ], Fail, write %ld(want %ld) size to file!\n",
			__FUNCTION__, __LINE__, wr_size, size);
		perror("write");
	}

	if (file_handle > 0)
	{
		close(file_handle);
	}

END:

	if (buffer)
	{
		free(buffer);
		buffer = NULL;
	}
}

static void soc_dbg_set_see_printf(int argc, char **argv)
{
	int enable = 0;

	if (argc != 1)
	{
		SOC_ERR("Option \"-s\": Argument count must be 1!\n");
		return;
	}

	if (SUCCESS != soc_dbg_get_num(argc, argv, &enable, "see_prf"))
	{
		return;
	}

	soc_set_see_printf(enable);
}

static void soc_dbg_see_hit_heart(int argc, char **argv)
{
	soc_hit_see_heart();
}

static void soc_dbg_see_enable_exception(int argc, char **argv)
{
	//soc_enable_see_exception();
}

void soc_dbg_show_see_plugin(int argc, char **argv)
{
	soc_show_see_plugin_info();
}

static void dbgtool_module_register(int argc, char **argv)
{
	PARSE_COMMAND_LIST * cmd_list;
	INT32 list_cnt;
	UINT32 index;

	if (0 == argc)
		goto REG_ALL;

	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
REG_ALL:
			for (index = 0; index < ARRAY_SIZE(debug_module); index++)
			{
				debug_module[index].get_cmd_list(&cmd_list, &list_cnt);
				debug_module_add(debug_module[index].name, cmd_list, list_cnt);
			}
			break;
		}
		else
		{
			for (index = 0; index < ARRAY_SIZE(debug_module); index++)
				if (!strcmp(*argv, debug_module[index].name))
					break;

			if (index >= ARRAY_SIZE(debug_module))
			{
				SOC_ERR("Debug module \"%s\" doesn't exist in the system!\n", *argv);
			}
			else if (debug_module[index].get_cmd_list != NULL)
			{
				debug_module[index].get_cmd_list(&cmd_list, &list_cnt);
				debug_module_add(*argv, cmd_list, list_cnt);
			}
		}

		argv++;
	}

	return;
}

static void dbgtool_module_unregister(int argc, char **argv)
{
	PARSE_COMMAND_LIST *cmd_list;
	UINT32 list_cnt, index;

	if (0 == argc)
		goto UNREG_ALL;
	
	while (argc-- > 0)
	{
		if (!strcmp(*argv, "all"))
		{
UNREG_ALL:
			for (index = 0; index < ARRAY_SIZE(debug_module); index++)
			{
				debug_module_delete(debug_module[index].name);
			}
			break;
		}
		else
		{
			for (index = 0; index < ARRAY_SIZE(debug_module); index++)
				if (!strcmp(*argv, debug_module[index].name))
					break;

			if (index >= ARRAY_SIZE(debug_module))
			{
				SOC_ERR("Debug module \"%s\" doesn't exist in system!\n", *argv);
			}
			else
			{
				debug_module_delete(*argv);
			}
		}

		argv++;
	}
	return;
}

void dbg_mod_list_init(void)
{
	unsigned i;
	PARSE_COMMAND_LIST *cmd_list;
	INT32 list_cnt;

	// Init debug module list.
	INIT_LIST_HEAD(&dbg_mod_list);
	for (i = 0; i < ARRAY_SIZE(debug_module); i++)
	{
		INIT_LIST_HEAD(&debug_module[i].list);
		INIT_LIST_HEAD(&debug_module[i].command_list);
		list_add(&debug_module[i].list, &dbg_mod_list);
	}
	
	// Init sub-module command list.
	//for (i = 0; i < ARRAY_SIZE(debug_module); i++)
	{
		debug_module[0].get_cmd_list(&cmd_list, &list_cnt);
		debug_module_add(debug_module[0].name, cmd_list, list_cnt);
	}

	return;
}

static PARSE_COMMAND_LIST socdbg_command[] =
{
	{ { NULL, NULL }, soc_dbg_show_help, soc_dbg_no_param_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_show_list, soc_dbg_no_param_preview, "list", "ls", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_show_system, soc_dbg_no_param_preview, "system", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_read8, soc_dbg_one_param_preview, "read8", "rd8", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_read16, soc_dbg_one_param_preview, "read16", "rd16", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_read32, soc_dbg_one_param_preview, "read32", "rd32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_write8, soc_dbg_two_param_preview, "write8", "wr8", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_write16, soc_dbg_two_param_preview, "write16", "wr16", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_write32, soc_dbg_two_param_preview, "write32", "wr32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_set_level, soc_dbg_one_param_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_dump_data, soc_dbg_two_param_preview, "dump", "d", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_mem_read32, soc_dbg_one_param_preview, "mem_read32", "mem_rd32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_mem_write32, soc_dbg_two_param_preview, "mem_write32", "mem_wr32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_mem_dump_data, soc_dbg_two_param_preview, "mem_dump", "mem_d", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	
	{ { NULL, NULL }, soc_dbg_otp_read32, soc_dbg_one_param_preview, "otp_read32", "otp_rd32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_otp_write32, soc_dbg_two_param_preview, "otp_write32", "otp_wr32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_otp_dump_data, soc_dbg_two_param_preview, "otp_dump", "otp_d", 0, 0, NULL, 0, 0, NULL, 0, 0 },

	{ { NULL, NULL }, soc_dbg_set_see_printf, soc_dbg_one_param_preview, "see_printf", "see_prf", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_see_hit_heart, soc_dbg_no_param_preview, "see_hit", "see_hit", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_see_enable_exception, soc_dbg_no_param_preview, "see_exp", "see_exp", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	
	{ { NULL, NULL }, soc_dbg_show_see_plugin, soc_dbg_no_param_preview, "see_plg", "see_plg", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	
	{ { NULL, NULL }, dbgtool_module_register, NULL, NULL, "reg", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dbgtool_module_unregister, NULL, NULL, "ureg", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

static PARSE_COMMAND_LIST socdbg_app_command[] =
{
	{ { NULL, NULL }, soc_dbg_show_help, soc_dbg_no_param_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_app_show_list, soc_dbg_no_param_preview, "list", "ls", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_show_system, soc_dbg_no_param_preview, "system", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_read8, soc_dbg_one_param_preview, "read8", "rd8", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_read16, soc_dbg_one_param_preview, "read16", "rd16", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_read32, soc_dbg_one_param_preview, "read32", "rd32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_write8, soc_dbg_two_param_preview, "write8", "wr8", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_write16, soc_dbg_two_param_preview, "write16", "wr16", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_write32, soc_dbg_two_param_preview, "write32", "wr32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_set_level, soc_dbg_one_param_preview, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_dump_data, soc_dbg_two_param_preview, "dump", "d", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_mem_read32, soc_dbg_one_param_preview, "mem_read32", "mem_rd32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_mem_write32, soc_dbg_two_param_preview, "mem_write32", "mem_wr32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_mem_dump_data, soc_dbg_two_param_preview, "mem_dump", "mem_d", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_per_read32, soc_dbg_one_param_preview, "per_read32", "per_rd32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_per_write32, soc_dbg_two_param_preview, "per_write32", "per_wr32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	
	{ { NULL, NULL }, soc_dbg_otp_read32, soc_dbg_one_param_preview, "otp_read32", "otp_rd32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_otp_write32, soc_dbg_two_param_preview, "otp_write32", "otp_wr32", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_otp_dump_data, soc_dbg_two_param_preview, "otp_dump", "otp_d", 0, 0, NULL, 0, 0, NULL, 0, 0 },

	{ { NULL, NULL }, soc_dbg_set_see_printf, soc_dbg_one_param_preview, "see_printf", "see_prf", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_see_hit_heart, soc_dbg_no_param_preview, "see_hit", "see_hit", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_see_enable_exception, soc_dbg_no_param_preview, "see_exp", "see_exp", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, soc_dbg_show_see_plugin, soc_dbg_no_param_preview, "see_plg", "see_plg", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};


INT32 soc_dbg_cmd_get(PARSE_COMMAND_LIST **cmd_list, INT32 *list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return -1;

	*cmd_list = &socdbg_command[0];
	*list_cnt = ARRAY_SIZE(socdbg_command);

	return 0;
}

INT32 soc_app_cmd_get(PARSE_COMMAND_LIST **cmd_list, INT32 *list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return -1;

	*cmd_list = &socdbg_app_command[0];
	*list_cnt = ARRAY_SIZE(socdbg_app_command);

	return 0;
}

