/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    adr_avsync_dbg.c
*
*    Description:    This file contains all diagnostic functions definition of avsync module.
*
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Oct.24.2012         leimei       Ver 0.1        Create file.
*****************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <linux/input.h>

#include <hld_cfg.h>
#include <adr_basic_types.h>
#include <adr_retcode.h>
#include <hld/adr_hld_dev.h>

#include <hld/avsync/adr_avsync.h>
#include <ali_avsync_common.h>

#include <hld/dbg/adr_dbg_parser.h>

#define AVSYNC_DBG_DEBUG
#ifdef AVSYNC_DBG_DEBUG
#define DBG_PRF(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(AYSYNC, fmt, ##args); \
			} while(0)
#else
#define DBG_PRF(...)
#endif

#define AVSYNC_DBG_CMD_NR 12

#define FUNCTION_ENTER()  DBG_PRF("API into: <%s>\n", __FUNCTION__)

#define AVSYNC_DBG_INFO			DBG_PRF

#define AVSYNC_DBG_ERR(fmt,args...) \
	do { \
		DBG_PRF("[%s:%d]ERR:  ", __FUNCTION__, __LINE__); \
		DBG_PRF(fmt, ##args); \
	} while(0);	

#define AVSYNC_PARA_CHECK_ERR(fmt,option, args...) \
{	\
	DBG_PRF("Parse option \"%s\" fail: ", option); \
	DBG_PRF(fmt, ##args); \
}		

typedef enum avsync_dbg_level
{
	AVSYNC_DBG_LEVEL_HLD = 0x1,
    AVSYNC_DBG_LEVEL_KERNEL = 0x2,
    AVSYNC_DBG_LEVEL_SEE = 0x4,
} AVSYNC_DBG_LEVEL;

struct option_map_t
{
	char *arg;	
	UINT32 opt;
};

static UINT32 g_avsync_dbg_level = AVSYNC_DBG_LEVEL_SEE;
static UINT32 g_avsync_dbg_rt_option= 0;
static UINT32 g_avsync_dbg_poll_option= 0;
static struct avsync_device *g_avsync_dbg_dev=NULL;


/*
//实时打印选项 
#define AVSYNC_DBG_PRINT_DEFAULT 1 //!<打印STC调整及音视频不同步信息
#define AVSYNC_DBG_PRINT_PCR	2  //!<打印PCR及PCR间隔
#define AVSYNC_DBG_PRINT_APTS	4  //!<打印音频PTS及PTS间隔
#define AVSYNC_DBG_PRINT_VPTS 8  //!<打印视频PTS及PTS间隔
#define AVSYNC_DBG_PRINT_A_SYNC 0x10 //!<打印音频帧同步时PTS/STC值
#define AVSYNC_DBG_PRINT_A_UNSYNC_PTS_STC 0x20 //!<打印音频帧不同步时PTS/STC值
#define AVSYNC_DBG_PRINT_A_UNSYNC_STCID_OFFSET 0x40 //!<打印音频帧不同步时STC偏移
#define AVSYNC_DBG_PRINT_A_UNSYNC_ESBUF 0x80 //!<打印音频帧不同步时可用ES buffer长度
#define AVSYNC_DBG_PRINT_V_SYNC 0x100 //!<打印视频帧同步时PTS/STC值
#define AVSYNC_DBG_PRINT_V_UNSYNC_PTS_STC 0x200 //!<打印视频帧不同步时PTS/STC值
#define AVSYNC_DBG_PRINT_V_UNSYNC_STCID_OFFSET 0x400 //!<打印视频帧不同步时STC偏移
#define AVSYNC_DBG_PRINT_V_UNSYNC_VBVBUF 0x800 //!<打印视频帧不同步时可用VBV buffer长度
#define AVSYNC_DBG_PRINT_PTS_OFFSET  0x1000    //!<打印PTS偏移调整信息
#define AVSYNC_DBG_PRINT_STREAM_LOOP 0x2000    //!<检测到流回头时打印

#define AVSYNC_DBG_PRINT_VESBUF_OF_UR 0x4000   //!<打印视频ES buffer空或满
#define AVSYNC_DBG_PRINT_AESBUF_OF_UR 0x8000   //!<打印音频ES buffer空或满

#define AVSYNC_DBG_PRINT_API	0x10000000     //!<打印API调用
#define AVSYNC_DBG_PRINT_LOG	0x20000000
#define AVSYNC_DBG_PRINT_ERR	0x40000000     //!<打印模块出错信息

//轮询调试选项
#define AVSYNC_DBG_POLL_V_SYNC_STATISTICS  0x01
#define AVSYNC_DBG_POLL_A_SYNC_STATISTICS  0x02
#define AVSYNC_DBG_POLL_PCR_STATISTICS  0x04
#define AVSYNC_DBG_POLL_VBV_FULL_CNT  0x08
#define AVSYNC_DBG_POLL_PTS_OFFSET  0x10
#define AVSYNC_DBG_POLL_SYNC_MODE 0x20
*/
struct option_map_t rt_option_map[]=
{
	{"all",	0xFFFFFFFF},
	{"def", AVSYNC_DBG_PRINT_DEFAULT},
	{"pcr", AVSYNC_DBG_PRINT_PCR},
	{"apts", AVSYNC_DBG_PRINT_APTS},
	{"vpts", AVSYNC_DBG_PRINT_VPTS},
	{"asyn", AVSYNC_DBG_PRINT_A_SYNC},		
	{"aups", AVSYNC_DBG_PRINT_A_UNSYNC_PTS_STC},
	{"auso", AVSYNC_DBG_PRINT_A_UNSYNC_STCID_OFFSET},
	{"aues", AVSYNC_DBG_PRINT_A_UNSYNC_ESBUF},		
	{"vsyn", AVSYNC_DBG_PRINT_V_SYNC},
	{"vups", AVSYNC_DBG_PRINT_V_UNSYNC_PTS_STC},
	{"vuso", AVSYNC_DBG_PRINT_V_UNSYNC_STCID_OFFSET},
	{"vues", AVSYNC_DBG_PRINT_V_UNSYNC_VBVBUF},		
	{"ptso", AVSYNC_DBG_PRINT_PTS_OFFSET},
	{"strlp", AVSYNC_DBG_PRINT_STREAM_LOOP},
	{"vesou", AVSYNC_DBG_PRINT_VESBUF_OF_UR},
	{"aesou", AVSYNC_DBG_PRINT_AESBUF_OF_UR},			
	{NULL, 0},			
};

struct option_map_t poll_option_map[]=
{
	{"all",	0xFFFFFFFF},
	{"vs", AVSYNC_DBG_POLL_V_SYNC_STATISTICS},
	{"as", AVSYNC_DBG_POLL_A_SYNC_STATISTICS},
	{"ps", AVSYNC_DBG_POLL_PCR_STATISTICS},
	{"md", AVSYNC_DBG_POLL_SYNC_MODE},
	{"ptso", AVSYNC_DBG_POLL_PTS_OFFSET},
	{NULL, 0},			
};

struct option_map_t sync_mode_map[]=
{
	{"pcr", AVSYNC_DBG_FORCE_SYNCMODE_PCR},
	{"audio", AVSYNC_DBG_FORCE_SYNCMODE_AUDIO},
	{"avfree", AVSYNC_DBG_FORCE_SYNCMODE_FREERUN},
	{"invalid", AVSYNC_DBG_FORCE_SYNCMODE_INVALID},
	{NULL, 0},			
};

struct option_map_t get_info_map[]=
{
	{"cfg", 1},
	{"advcfg", 2},
	{NULL, 0},			
};

//get debug option
static RET_CODE avsync_dbg_sub_get_option(char *cmd, int argc, char **argv, struct option_map_t *poption_map, UINT32 *opt)
{
	UINT8 arg_num=0;
	int len;
	char poll_opt[16] = "";
	int ret;
	struct option_map_t *popt_map;
		
	*opt = 0;
	while (argc-- > 0)
	{
		//convert char between space to string
		ret = sscanf(*argv, "%s", &poll_opt[0]);
		if(ret == 0)
		{
			AVSYNC_DBG_ERR("sscanf string fail\n", NULL);
			return RET_FAILURE;
		}
		popt_map = poption_map;
		arg_num++;
		AVSYNC_DBG_INFO("arg[%d]: %s\n", arg_num, poll_opt);
		
		len = strlen(poll_opt);		
		while (NULL != popt_map->arg)
		{ // find mapped print option
			if (0 == strcmp(poll_opt, popt_map->arg))
			{
				*opt |= popt_map->opt;
				break;
			}
			popt_map++;
		}

		if(NULL == popt_map->arg)
		{ // not find print option
			AVSYNC_PARA_CHECK_ERR("invalid argument \"%s\"\n", cmd, poll_opt);
			return RET_FAILURE;
		}

		argv++;
	}	

	if(0 == arg_num)
	{
		AVSYNC_PARA_CHECK_ERR("no argument found, argument number should be >= 1\n", cmd, NULL);		
		return RET_FAILURE;
	}
	
	return RET_SUCCESS;
}

//get argument number
static UINT8 avsync_dbg_sub_get_arg_num(int argc, char **argv)
{
	UINT8 arg_num=0;
	int len,ret;
	char option[16] = "";
	
	while (argc-- > 0)
	{
		arg_num++;
		//convert char between space to string
		ret = sscanf(*argv, "%s", &option[0]);
		if(ret == 0)
			return RET_FAILURE;
		
		len = strlen(option);		
		argv++;
	}	

	return arg_num;
}

static ARG_CHK avsync_dbgcmd_help_paracheck(int argc, char **argv, char *option)
{
	int arg_num;
	
	FUNCTION_ENTER();	

	arg_num = avsync_dbg_sub_get_arg_num(argc, argv);
	
	if(0 != arg_num)
	{
		AVSYNC_PARA_CHECK_ERR("should be without argument\n", option, NULL);		
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static ARG_CHK avsync_dbgcmd_set_level_paracheck(int argc, char **argv, char *option)
{
	int level, ret, arg_num;

	FUNCTION_ENTER();	
	
	arg_num = avsync_dbg_sub_get_arg_num(argc, argv);
	
	if(arg_num != 1)
	{
		AVSYNC_PARA_CHECK_ERR("argument number should be 1, but %d argument found\n", option, arg_num);
		return DBG_ARG_INVALID;
	}
		
	ret = sscanf(*argv, "%d", &level);
	if (0==ret || level < 0 ||
		level > (AVSYNC_DBG_LEVEL_KERNEL | AVSYNC_DBG_LEVEL_HLD | AVSYNC_DBG_LEVEL_SEE))
	{
		AVSYNC_PARA_CHECK_ERR("argument  \"%s\" is not a number or out of range!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	
	return DBG_ARG_VALID;	
}

static ARG_CHK avsync_dbgcmd_polling_option_paracheck(int argc, char **argv, char *option)
{
	UINT8 arg_num = 0;
	UINT32 poll_opt;
	
	FUNCTION_ENTER();	

	if(RET_SUCCESS != avsync_dbg_sub_get_option(option, argc, argv, poll_option_map, &poll_opt))
		return DBG_ARG_INVALID;

	return DBG_ARG_VALID;
}


static ARG_CHK avsync_dbgcmd_polling_onoff_paracheck(int argc, char **argv, char *option)
{
	int on_off, ret, arg_num;
	
	FUNCTION_ENTER();	

	arg_num = avsync_dbg_sub_get_arg_num(argc, argv);
		
	if(arg_num == 0)
	{
		AVSYNC_PARA_CHECK_ERR("no argument found, argument number should be >=1\n", option, NULL);
		return DBG_ARG_INVALID;
	}
		
	ret = sscanf(*argv, "%d", &on_off);
	if (0 == ret || on_off != 0 && on_off != 1) 
	{
		AVSYNC_PARA_CHECK_ERR("argument \"%s\" is out of range!\n", option, *argv);
		return DBG_ARG_INVALID;
	}

	if(arg_num > 1)
	{
		if (0 == on_off)
		{
			AVSYNC_PARA_CHECK_ERR("polling disable should be without argument\n", option, NULL);
			return DBG_ARG_INVALID;
		}

		avsync_dbgcmd_polling_option_paracheck(argc, ++argv, option);
	}
	
	return DBG_ARG_VALID;
}

static ARG_CHK avsync_dbgcmd_set_polling_interval_paracheck(int argc, char **argv, char *option)
{
	int interval, ret, arg_num;
	
	FUNCTION_ENTER();	

	arg_num = avsync_dbg_sub_get_arg_num(argc, argv);
	
	if(arg_num != 1)
	{
		AVSYNC_PARA_CHECK_ERR("argument number should be 1, but %d argument found\n", option, arg_num);
		return DBG_ARG_INVALID;
	}
		
	ret = sscanf(*argv, "%d", &interval);
	if (interval < 0 || interval > 4*3600*1000) /*4 hours*/ 
	{
		AVSYNC_PARA_CHECK_ERR("argument \"%s\" is out of range, should be less than 4 hours!\n", option, *argv);
		return DBG_ARG_INVALID;	
	}
	
	return DBG_ARG_VALID;
}

static ARG_CHK avsync_dbgcmd_realtime_option_paracheck(int argc, char **argv, char *option)
{
	UINT32 opt;
	
	FUNCTION_ENTER();	

	if (RET_SUCCESS != avsync_dbg_sub_get_option(option, argc, argv, rt_option_map, &opt))
		return DBG_ARG_INVALID;	
	
	return DBG_ARG_VALID;
}

static ARG_CHK avsync_dbgcmd_get_info_paracheck(int argc, char **argv, char *option)
{
	UINT32 opt;
	
	FUNCTION_ENTER();	

	if(RET_SUCCESS != avsync_dbg_sub_get_option(option, argc, argv, get_info_map, &opt))
		return DBG_ARG_INVALID;	

	return DBG_ARG_VALID;	
}

static ARG_CHK avsync_dbgcmd_set_syncmode_paracheck(int argc, char **argv, char *option)
{
	UINT32 opt, arg_num;
	
	FUNCTION_ENTER();	
	
	arg_num = avsync_dbg_sub_get_arg_num(argc, argv);
	
	if(arg_num != 1)
	{
		AVSYNC_PARA_CHECK_ERR("argument number should be 1, but %d argument found\n", option, arg_num);
		return DBG_ARG_INVALID;	
	}
		
	if (RET_SUCCESS != avsync_dbg_sub_get_option(option, argc, argv, sync_mode_map, &opt))
		return DBG_ARG_INVALID;	

	return DBG_ARG_VALID;	
}

static ARG_CHK avsync_dbgcmd_smooth_play_onoff_paracheck(int argc, char **argv, char *option)
{
	int onoff, level, interval;
	int ret;
	
	if (argc != 3)
	{
		AVSYNC_PARA_CHECK_ERR("argument number shold be 3\n", option, NULL);			
		return DBG_ARG_INVALID;	
	}

	//get argument
	ret = sscanf(*argv++, "%d", &onoff);
	if(ret != 1)
	{
		AVSYNC_PARA_CHECK_ERR("argument number shold be 3\n", option, ret);			
		return DBG_ARG_INVALID;
	}
	ret = sscanf(*argv++, "%d", &level);
	if(ret != 1)
	{
		AVSYNC_PARA_CHECK_ERR("argument number shold be 3\n", option, ret);			
		return DBG_ARG_INVALID;
	}
	ret = sscanf(*argv++, "%d", &interval);
	if(ret != 1)
	{
		AVSYNC_PARA_CHECK_ERR("argument number shold be 3\n", option, ret);			
		return DBG_ARG_INVALID;
	}

	if (onoff != 0 && onoff != 1)
	{
		AVSYNC_PARA_CHECK_ERR("invalid first argument \"%d\"\n", option, onoff);			
		return DBG_ARG_INVALID;
	}

	if (level != 1 && level != 2)
	{
		AVSYNC_PARA_CHECK_ERR("invalid second argument \"%d\"\n", option, level);			
		return DBG_ARG_INVALID;
	}

	if(interval < 1 || interval > 9)
	{
		AVSYNC_PARA_CHECK_ERR("invalid third argument \"%d\", should be 1~9\n", option, interval);			
		return DBG_ARG_INVALID;
	}
	
	return DBG_ARG_VALID;
}

static ARG_CHK avsync_dbgcmd_set_sync_thres_paracheck(int argc, char **argv, char *option)
{
	char av;
	int drop_thres, hold_thres;
	int ret;
	
	if (argc != 3)
	{
		AVSYNC_PARA_CHECK_ERR("argument number shold be 3\n", option, NULL);			
		return DBG_ARG_INVALID;	
	}

	//get argument
	ret = sscanf(*argv++, "%c", &av);
	if(ret != 1)
	{
		AVSYNC_PARA_CHECK_ERR("argument number shold be 3\n", option, ret);			
		return DBG_ARG_INVALID;
	}
	ret = sscanf(*argv++, "%d", &drop_thres);
	if(ret != 1)
	{
		AVSYNC_PARA_CHECK_ERR("argument number shold be 3\n", option, ret);			
		return DBG_ARG_INVALID;
	}
	ret = sscanf(*argv++, "%d", &hold_thres);
	if(ret != 1)
	{
		AVSYNC_PARA_CHECK_ERR("argument number shold be 3\n", option, ret);			
		return DBG_ARG_INVALID;
	}
	
	if (av != 'a' && av !='v')
	{
		AVSYNC_PARA_CHECK_ERR("invalid first argument \"%c\"\n", option, av);			
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static RET_CODE avsync_dbgcmd_help(int argc, char **argv)
{	
	FUNCTION_ENTER();	
	
	DBG_PRF("\nModule name:\n");
	DBG_PRF("    avsync\n");
	
	DBG_PRF("\nModule Description:\n");
	DBG_PRF("    avsync diagnostic debugging module.\n");	
	
	DBG_PRF("\nSyntax:\n");
	DBG_PRF("    avsync -option argument... [-option argument...]...\n");
	
	DBG_PRF("\nOption description:\n");
	
	DBG_PRF("\n    -h, --help\n");
	DBG_PRF("        Self description of this debugging module.\n");
	
	DBG_PRF("\n    -level number\n");
	DBG_PRF("        Set debugging printout level(The default value is \"6\").\n");
	DBG_PRF("        number:\n");	
	DBG_PRF("            0\t--No printout.\n");
	DBG_PRF("            1\t--Show HLD printout infomation.\n");
	DBG_PRF("            2\t--Show Kernel printout infomation.\n");
	DBG_PRF("            3\t--Show HLD & Kernel printout infomation.\n");
	DBG_PRF("            4\t--Show SEE printout infomation.\n");
	DBG_PRF("            5\t--Show HLD & SEE printout infomation.\n");
	DBG_PRF("            6\t--Show Kernel & SEE printout infomation.\n");
	DBG_PRF("            7\t--Show HLD & Kernel & SEE printout infomation.\n");
	
	DBG_PRF("\n    -ro arg...\n");
	DBG_PRF("        Set realtime print option\n");
	DBG_PRF("    -rc arg...\n");
	DBG_PRF("        Clear realtime print option.\n");
	DBG_PRF("        arg:\n");	
	DBG_PRF("            all\t\t--print all realtime debugging info.\n");
	DBG_PRF("            def\t\t--print default debugging info\n");
	DBG_PRF("            pcr\t\t--print PCR value in TS stream\n");
	DBG_PRF("            apts\t--print audio PTS in TS stream\n");
	DBG_PRF("            vpts\t--print video PTS in TS stream\n");
	DBG_PRF("            asyn\t--print PTS and STC and free audio es buffer size  if frame is in sync\n");
	DBG_PRF("            aups\t--print PTS and STC if audio frame is out of sync\n");
	DBG_PRF("            auso\t--print stcid and offset if audio frame is out of sync\n");
	DBG_PRF("            aues\t--print free vbv size if audio frame is out of sync\n");
	DBG_PRF("            vsyn\t--print PTS and STC and free vbv size  if frame is in sync\n");
	DBG_PRF("            vups\t--print PTS and STC if video frame is out of sync\n");
	DBG_PRF("            vuso\t--print stcid and offset if video frame is out of sync\n");
	DBG_PRF("            vues\t--print free vbv size if video frame is out of sync\n");
	DBG_PRF("            ptso\t--print pts offset\n");
	DBG_PRF("            strlp\t--print info when stream loop detected\n");
	DBG_PRF("            vesou\t--print info when vbv buffer full or empty\n");
	DBG_PRF("            aesou\t--print info when audio es buffer full or empty\n");	
	
	DBG_PRF("\n    -pt interval\n");
	DBG_PRF("        Set polling interval, unit is ms, default value is 1000ms.\n");
	DBG_PRF("        interval:\n");	
	DBG_PRF("            --decimal number, unit is ms, default value is 1000ms.\n");
		
	DBG_PRF("\n    -po arg...\n");
	DBG_PRF("        Set polling option.\n");
	DBG_PRF("    -pc arg...\n");
	DBG_PRF("        Clear polling option.\n");
	DBG_PRF("        arg:\n");	
	DBG_PRF("            all\t\t--print all polling debugging info.\n");
	DBG_PRF("            vs\t--polling video frame statistics\n");
	DBG_PRF("            as\t--polling audio frame statistics\n");
	DBG_PRF("            ps\t--polling PCR statistics\n");
	DBG_PRF("            md\t--polling sync mode\n");
		
	DBG_PRF("\n    -pe enable [option]...\n");
	DBG_PRF("        Enable/disable polling information.\n");
	DBG_PRF("        enable:\n");	
	DBG_PRF("            --binary  number, 1: enable 0: disable\n");
	DBG_PRF("        option:\n");	
	DBG_PRF("            --refer to -po arg..., this argument used only when enable is 1\n");
	
	DBG_PRF("\n    -sm mode\n");
	DBG_PRF("        Set sync mode\n");
	DBG_PRF("        mode:\n");	
	DBG_PRF("            pcr\t\t-- audio and video sync to pcr\n");
	DBG_PRF("            audio\t--freerun audio, video sync to audio\n");
	DBG_PRF("            avfree\t--freerun audio and video\n");
	DBG_PRF("            invalid\t--invalidate debug sync mode\n");

#if 0
	DBG_PRF("\n    -smooth enable level interval\n")
	DBG_PRF("        Enable/disable smoothly play video\n");
	DBG_PRF("        enable:\n");	
	DBG_PRF("            --binary  number, 1: enable 0: disable, default value is 1\n");
	DBG_PRF("        level:\n");	
	DBG_PRF("            --decimal  number, 1: level1 2: level2, default value is level1\n");
	DBG_PRF("        interval:\n");	
	DBG_PRF("            --decimal  number, repeat frame interval, default value is 4\n");
#endif

	DBG_PRF("\n    -syncthres av drop_threshold hold_threshold\n");
	DBG_PRF("        set audio/video sync threshold\n");
	DBG_PRF("        av:\n");	
	DBG_PRF("            a\t\t--audio\n");
	DBG_PRF("            v\t\t--video\n");
	DBG_PRF("        drop_threshold:\n");	
	DBG_PRF("            --decimal  number, drop threshold\n");
	DBG_PRF("        hold_threshold:\n");	
	DBG_PRF("            --decimal  number, hold threshold\n");

	DBG_PRF("\n    -get option\n");
	DBG_PRF("        Get configure inforamtion\n");
	DBG_PRF("        option:\n");	
	DBG_PRF("           cfg\t\t--print basic configuration\n");
	DBG_PRF("           advcfg\t\t--print advanced configuration\n");

	DBG_PRF("\nUsage example:\n");
	DBG_PRF("    avsync -h\n");
	DBG_PRF("    avsync -ro def pcr apts vpts\n");
	DBG_PRF("    avsync -rc vpts asyn aups\n");
	DBG_PRF("    avsync -pe 1(0)\n");
	DBG_PRF("    avsync -po vs as\n");
	DBG_PRF("    avsync -pc vs as\n");
	DBG_PRF("    avsync -pt 5000 \n");
	DBG_PRF("    avsync -sm audio\n");
	DBG_PRF("    avsync -l 4\n");		

	return RET_SUCCESS;
}

static RET_CODE avsync_dbgcmd_set_level(int argc, char **argv)
{
	int level, ret;
	UINT8 valid;

	FUNCTION_ENTER();

	ret = sscanf(*argv, "%d", &level);
	if (0 == ret)
	{
		AVSYNC_DBG_ERR("argument \"%s\" is not a number!\n", *argv);
		return RET_FAILURE;
	}

	g_avsync_dbg_level	= level;
	return RET_SUCCESS;
	
}



static RET_CODE avsync_dbgcmd_set_polling_option(int argc, char **argv)
{
	UINT32 opt;

	FUNCTION_ENTER();
	
	if (RET_FAILURE == avsync_dbg_sub_get_option("-po", argc, argv, poll_option_map, &opt))
	{
		AVSYNC_DBG_ERR("Argument is error!\n", NULL);
		return RET_FAILURE;
	}
	
	g_avsync_dbg_poll_option |= opt;
	
	return avsync_dbg_set_polling_option(g_avsync_dbg_dev, g_avsync_dbg_poll_option);	
}



static RET_CODE avsync_dbgcmd_polling_onoff(int argc, char **argv)
{
	UINT8 on_off;
	UINT8 valid;
	int ret;

	FUNCTION_ENTER();
	
	ret = sscanf(*argv, "%d", &on_off);
	if (0 == ret)
	{
		AVSYNC_DBG_ERR("argument \"%s\" is not a number!\n", *argv);
		return RET_FAILURE;
	}
	
	if(0 == on_off)
		g_avsync_dbg_poll_option = 0;
	
	avsync_dbg_polling_onoff(g_avsync_dbg_dev, on_off);

	if (avsync_dbg_sub_get_arg_num(argc, argv) == 1)
	{
		return RET_SUCCESS;
	}

	return avsync_dbgcmd_set_polling_option(argc, ++argv);	
}



static RET_CODE avsync_dbgcmd_clr_polling_option(int argc, char **argv)
{
	UINT32 opt;
	
	FUNCTION_ENTER();
	
	if (RET_FAILURE == avsync_dbg_sub_get_option("-pc", argc, argv, poll_option_map, &opt))
	{
		return RET_FAILURE;
	}
	
	g_avsync_dbg_poll_option &= (~opt);

	return avsync_dbg_set_polling_option(g_avsync_dbg_dev, g_avsync_dbg_poll_option);		
}

static RET_CODE avsync_dbgcmd_set_polling_interval(int argc, char **argv)
{
	UINT32 interval;
	UINT8 valid;
	int ret;

	FUNCTION_ENTER();

	ret = sscanf(*argv, "%d", &interval);
	if (0 == ret)
	{
		AVSYNC_DBG_ERR("Argument \"%s\" is not a number!\n", *argv);
		return RET_FAILURE;
	}
	
	return avsync_dbg_set_polling_interval(g_avsync_dbg_dev, interval);
}

static RET_CODE avsync_dbgcmd_set_realtime_option(int argc, char **argv)
{
	UINT32 opt;

	FUNCTION_ENTER();
	
	if (RET_FAILURE == avsync_dbg_sub_get_option("-ro", argc, argv, rt_option_map, &opt))
	{
		return RET_FAILURE;
	}
	
	g_avsync_dbg_rt_option |= opt;

	return avsync_dbg_set_print_option(g_avsync_dbg_dev, g_avsync_dbg_rt_option);	
	
}

static RET_CODE avsync_dbgcmd_clr_realtime_option(int argc, char **argv)
{
	UINT32 opt;

	FUNCTION_ENTER();

	if (RET_FAILURE == avsync_dbg_sub_get_option("-rc", argc, argv, rt_option_map, &opt))
	{
		return RET_FAILURE;
	}
	
	g_avsync_dbg_rt_option &= (~opt);

	return avsync_dbg_set_print_option(g_avsync_dbg_dev, g_avsync_dbg_rt_option);			
}

static RET_CODE avsync_dbgcmd_set_syncmode(int argc, char **argv)
{
	
	AVSYNC_MODE_E opt;

	FUNCTION_ENTER();

	if(RET_FAILURE == avsync_dbg_sub_get_option("-sm", argc, argv, sync_mode_map, (UINT32 *)&opt))
	{
		return RET_FAILURE;
	}
	
//	return avsync_set_syncmode(g_avsync_dbg_dev, opt);			
	g_avsync_dbg_rt_option &= ~(AVSYNC_DBG_FORCE_SYNCMODE_PCR|AVSYNC_DBG_FORCE_SYNCMODE_AUDIO|AVSYNC_DBG_FORCE_SYNCMODE_FREERUN|AVSYNC_DBG_FORCE_SYNCMODE_INVALID);
	g_avsync_dbg_rt_option |= opt;
	return avsync_dbg_set_print_option(g_avsync_dbg_dev, g_avsync_dbg_rt_option);
}

static RET_CODE avsync_dbgcmd_get_info(int argc, char **argv)
{	
	UINT32 opt;
	avsync_cfg_param_t cfg_params;
	avsync_adv_param_t adv_cfg_params;
	
	memset(&cfg_params,0,sizeof(avsync_cfg_param_t));
	memset(&adv_cfg_params,0,sizeof(avsync_adv_param_t));
	FUNCTION_ENTER();

	if (RET_FAILURE == avsync_dbg_sub_get_option("-get", argc, argv, get_info_map, (UINT32 *)&opt))
	{
		return RET_FAILURE;
	}

	if(opt & 1)
	{
		avsync_get_params(g_avsync_dbg_dev, &cfg_params);

		DBG_PRF("AVSYNC basic configuraiton:\n");
		DBG_PRF("    sync mode:    %d\n", cfg_params.sync_mode);
		DBG_PRF("    source type:   %d\n", cfg_params.src_type);
		DBG_PRF("    video hold threshold: %dms\n", cfg_params.vhold_thres);
		DBG_PRF("    video drop threshold: %dms\n", cfg_params.vdrop_thres);
		DBG_PRF("    audio hold threshold: %dms\n", cfg_params.ahold_thres);
		DBG_PRF("    audio drop threshold: %dms\n", cfg_params.adrop_thres);
	}

	if(opt & 2)
	{
		avsync_get_advance_params(g_avsync_dbg_dev, &adv_cfg_params);
		DBG_PRF("AVSYNC advanced configuraiton:\n");
		DBG_PRF("    video freerun threshold: %dms\n", adv_cfg_params.vfreerun_thres);
		DBG_PRF("    audio freerun threshold: %dms\n", adv_cfg_params.afreerun_thres);
		DBG_PRF("    sd output delay:t %d\n", adv_cfg_params.dual_output_sd_delay);
		DBG_PRF("    first frame freerun enable: %d\n", adv_cfg_params.disable_first_video_freerun);
	}

	return RET_SUCCESS;
}

static RET_CODE avsync_dbgcmd_smooth_play_onoff(int argc, char **argv)
{
	int onoff, level, interval;
	int ret;
	
	//get argument
	ret = sscanf(*argv++, "%d", &onoff);
	if(ret != 1)
	{
		AVSYNC_DBG_ERR("sscanf string fail\n", NULL);
		return RET_FAILURE;
	}
	ret = sscanf(*argv++, "%d", &level);
	if(ret != 1)
	{
		AVSYNC_DBG_ERR("sscanf string fail\n", NULL);
		return RET_FAILURE;
	}	
	ret = sscanf(*argv++, "%d", &interval);
	if(ret != 1)
	{
		AVSYNC_DBG_ERR("sscanf string fail\n", NULL);
		return RET_FAILURE;
	}

	avsync_video_smoothly_play_onoff(g_avsync_dbg_dev, onoff, level, interval);
	
	return RET_SUCCESS;	
}

static RET_CODE avsync_dbgcmd_set_sync_thres(int argc, char **argv)
{
	int ret;
	char opt;
	int baudio = 0, drop_thres, hold_thres;
	avsync_cfg_param_t cfg_params;

	//get argument
	ret = sscanf(*argv++, "%c", &opt);
	if(ret != 1)
	{
		AVSYNC_DBG_ERR("sscanf string fail\n", NULL);
		return RET_FAILURE;
	}
	ret = sscanf(*argv++, "%d", &drop_thres);
	if(ret != 1)
	{
		AVSYNC_DBG_ERR("sscanf string fail\n", NULL);
		return RET_FAILURE;
	}
	ret = sscanf(*argv++, "%d", &hold_thres);
	if(ret != 1)
	{
		AVSYNC_DBG_ERR("sscanf string fail\n", NULL);
		return RET_FAILURE;
	}

	if(opt == 'a')
	{
		baudio = 1;
	}

	avsync_get_params(g_avsync_dbg_dev, &cfg_params);

	if(baudio)
	{
		cfg_params.adrop_thres = drop_thres;
		cfg_params.ahold_thres = hold_thres;		
	}
	else
	{
		cfg_params.vdrop_thres = drop_thres;
		cfg_params.vhold_thres = hold_thres;		
	}

	return avsync_config_params(g_avsync_dbg_dev, &cfg_params);		
}


static PARSE_COMMAND_LIST avsync_dbg_cmd[] = {
	{ { NULL, NULL }, (void *)avsync_dbgcmd_help, (void *)avsync_dbgcmd_help_paracheck, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_set_level, (void *)avsync_dbgcmd_set_level_paracheck, NULL, "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_polling_onoff, (void *)avsync_dbgcmd_polling_onoff_paracheck, NULL, "pe", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_set_polling_option, (void *)avsync_dbgcmd_polling_option_paracheck, NULL, "po", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_clr_polling_option, (void *)avsync_dbgcmd_polling_option_paracheck, NULL, "pc", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_set_polling_interval, (void *)avsync_dbgcmd_set_polling_interval_paracheck, NULL, "pt", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_set_realtime_option, (void *)avsync_dbgcmd_realtime_option_paracheck, NULL, "ro", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_clr_realtime_option, (void *)avsync_dbgcmd_realtime_option_paracheck, NULL, "rc", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_set_syncmode, (void *)avsync_dbgcmd_set_syncmode_paracheck, NULL, "sm", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_get_info, (void *)avsync_dbgcmd_get_info_paracheck, NULL, "get", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_smooth_play_onoff, (void *)avsync_dbgcmd_smooth_play_onoff_paracheck, NULL, "smooth", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, (void *)avsync_dbgcmd_set_sync_thres, (void *)avsync_dbgcmd_set_sync_thres_paracheck, NULL, "syncthres", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

RET_CODE avsync_dbg_init()
{
	debug_module_add("avsync", &avsync_dbg_cmd[0], ARRAY_SIZE(avsync_dbg_cmd));
	
	g_avsync_dbg_dev = (struct avsync_device *)dev_get_by_id(HLD_DEV_TYPE_AVSYNC, 0);
	if(NULL == g_avsync_dbg_dev)
	{
		AVSYNC_DBG_ERR("get avsync device fail, device is NULL\n", NULL);
		return RET_FAILURE;
	}
	return RET_SUCCESS;  
}

INT32 avsync_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	avsync_attach();
	*cmd_list = &avsync_dbg_cmd[0];
	*list_cnt = ARRAY_SIZE(avsync_dbg_cmd);

	g_avsync_dbg_dev = (struct avsync_device *)dev_get_by_id(HLD_DEV_TYPE_AVSYNC, 0);
	if(NULL == g_avsync_dbg_dev)
	{
		//AVSYNC_DBG_ERR("get avsync device fail, device is NULL\n", NULL);
		return RET_FAILURE;
	}

	return RET_SUCCESS;
}

INT32 avsync_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return avsync_dbg_cmd_get(cmd_list, list_cnt);
}
