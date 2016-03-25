/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: adr_decv_dbg.c
 *
 *  Description: Hld decv driver dbg function
 *
 *  History:
 *      Date        Author         	Version   Comment
 *      ====        ======         =======   =======
 *  1.  2012.10.22  Sam			0.9		 inited
 ****************************************************************************/

#include <errno.h>
#include <stdio.h>

#include <hld_cfg.h>
#include <adr_basic_types.h>
#include <adr_retcode.h>
#include <hld/adr_hld_dev.h>
#include <hld/decv/adr_decv.h>
#include <hld/dbg/adr_dbg_parser.h>

#include <ali_video_common.h>

#include "adr_decv_dbg.h"

#define __ADR_DECV_DBG
#ifdef __ADR_DECV_DBG
#define DECV_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DECV, fmt, ##args); \
			} while(0)
#else
#define DECV_DBG_PRINT(...)	do{}while(0)
#endif

struct option_map_t
{
	char *arg;	
	UINT32 opt;
};

static struct option_map_t test_option_map[]=
{
	{"clear_all",	0},
	{"clear", 1},
	{"err_conti", 2},
	{NULL, 0},			
};

#define test_opition_num ((sizeof(test_option_map) / sizeof(test_option_map[0])) - 1)
static int m_decv_last_dbg_test_opt = -1;

static int m_decv_dbg_level = (DECV_DBG_LEVEL_HLD | DECV_DBG_LEVEL_SEE);

static RET_CODE dbg_sub_get_option(char *cmd, int argc, char **argv, struct option_map_t *poption_map, UINT32 *opt)
{
	UINT8 arg_num=0;
	int len;
	char poll_opt[16] = {0};
	int ret;
	struct option_map_t *popt_map = NULL;
	memset(&poll_opt[0], 0, sizeof(poll_opt));	
	*opt = 0;
	while (argc-- > 0)
	{
		//convert char between space to string
		ret = sscanf(*argv, "%s", &poll_opt[0]);
		if(ret == 0)
		{
			DECV_DBG_PRINT("Option \"%s\": Sscanf string fail\n", cmd, NULL);
			return RET_FAILURE;
		}
		popt_map = poption_map;
		arg_num++;
		DECV_DBG_PRINT("arg[%d]: %s\n", arg_num, poll_opt);
		
		len = strlen(poll_opt);		
		while(NULL != popt_map->arg)
		{ // find mapped print option
		    popt_map->arg[sizeof(poll_opt)] = 0;
			if (0 == strcmp(poll_opt, popt_map->arg))
			{
				*opt |= popt_map->opt;
				break;
			}
			popt_map++;
		}

		if(NULL == popt_map->arg)
		{ // not find print option
			DECV_DBG_PRINT("Option \"%s\": Invalid argument \"%s\"\n", cmd, poll_opt);
			return RET_FAILURE;
		}

		argv++;
	}	

	if(0 == arg_num)
	{
		DECV_DBG_PRINT("Option \"%s\": No argument found, argument number should be >= 1\n", cmd, NULL);		
		return RET_FAILURE;
	}
	
	return RET_SUCCESS;
}

static void set_test_mode(int mode, int active_flag)
{
	if(mode == 2)
	{
		if(active_flag == TRUE)
			vdec_io_control(get_selected_decoder(), VDEC_IO_CONTINUE_ON_ERROR, TRUE);
		else
			vdec_io_control(get_selected_decoder(), VDEC_IO_CONTINUE_ON_ERROR, FALSE);
	}
}

static ARG_CHK decv_dbg_test_check(int argc, char **argv, char *option)
{
	UINT8 arg_num=0;
	UINT32 poll_opt;
	
	if (0 == argc)
	{
		DECV_DBG_PRINT("Option \"%s\": No argument found, argument number should be >= 1\n", option);		
		return DBG_ARG_INVALID;
	}

	if(RET_SUCCESS != dbg_sub_get_option(option, argc, argv, test_option_map, &poll_opt))
		return DBG_ARG_INVALID;

	return DBG_ARG_VALID;
}

static void decv_dbg_test(int argc, char **argv)
{
	UINT32 opt;
	int i = 0;
 
	if(RET_FAILURE ==  dbg_sub_get_option("-t", argc, argv, test_option_map, &opt))
	{
		DECV_DBG_PRINT("argument is error!\n");
		return;
	}

	if(opt == 0)
	{
		for(i = 2;i < (int)test_opition_num;i++)		
			set_test_mode(i, FALSE);
	}
	else if((opt == 1) && (m_decv_last_dbg_test_opt > 0))
	{
		set_test_mode(m_decv_last_dbg_test_opt, FALSE);
		m_decv_last_dbg_test_opt = -1;
	}
	else
	{
		set_test_mode(opt, TRUE);
		m_decv_last_dbg_test_opt = opt;
	}

	DECV_DBG_PRINT("%s : cmd %s\n", __FUNCTION__, test_option_map[opt].arg);		
}

static void decv_dbg_level(int argc, char **argv)
{
	int level = 0;
	int ret = 0;
	
	if(argc != 1)
	{
		DECV_DBG_PRINT("%s : param number should be one!\n", __FUNCTION__);
		return;
	}
	
	ret = sscanf(*argv, "%d", &level);
	if((ret == 0) || (level < 0) 
		|| (level > (DECV_DBG_LEVEL_HLD | DECV_DBG_LEVEL_KERNEL | DECV_DBG_LEVEL_SEE)))
	{
		DECV_DBG_PRINT("%s : param fail %s\n", __FUNCTION__, *argv);
		return;
	}
	
	m_decv_dbg_level |= level;
	
	DECV_DBG_PRINT("%s : level %d\n", __FUNCTION__, level);		
	return;
}

static void decv_dbg_info(int argc, char **argv)
{
	struct vdec_device *dev = get_selected_decoder();
	RET_CODE ret = RET_SUCCESS;
	struct VDec_StatusInfo info;
	struct vdec_decore_status dec_info;
				
	if (argc != 0)
	{
		DECV_DBG_PRINT("Option \"-i\" should not with any argument(s)!\n");
		return;
	}

	memset((void *)&info, 0, sizeof(info));
	ret = vdec_io_control(dev, VDEC_IO_GET_STATUS, (UINT32)&info);
	if(ret != RET_SUCCESS)
	{
		DECV_DBG_PRINT("%s : %d fail\n", __FUNCTION__, __LINE__);
		goto EXIT;
	}
	
	DECV_DBG_PRINT("%s : as below**********************:\n\n", __FUNCTION__);
	
	DECV_DBG_PRINT("\t\tdecv type ");
	switch(get_current_decoder())
	{
	    case MPEG2_DECODER:
		DECV_DBG_PRINT("Mpeg2\n");
		break;
	    case H264_DECODER:
		DECV_DBG_PRINT("H.264\n");
		break;
	    case AVS_DECODER:
		DECV_DBG_PRINT("AVS\n");
		break;
	    dafault:
		DECV_DBG_PRINT("Unknown\n");
		break;
	}
	
	DECV_DBG_PRINT("\t\tdecv status ");
	if(info.uCurStatus == VDEC27_STARTED)
	{
		DECV_DBG_PRINT("STARTED\n");

		if(info.uFirstPicShowed)
			DECV_DBG_PRINT("\t\tfirst pic TRUE\n");
		else
			DECV_DBG_PRINT("\t\tfirst pic FALSE");

		DECV_DBG_PRINT("\t\tvideo width %d\n", info.pic_width);
		DECV_DBG_PRINT("\t\tvideo height %d\n", info.pic_height);
		DECV_DBG_PRINT("\t\tvideo frame rate %d\n", info.frame_rate);
		DECV_DBG_PRINT("\t\tvideo aspect ");
		switch(info.aspect_ratio)
		{
			case SAR:
				DECV_DBG_PRINT("SAR\n");
				break;
			case DAR_4_3:
				DECV_DBG_PRINT("4:3\n");
				break;
			case DAR_16_9:
				DECV_DBG_PRINT("16:9\n");
				break;
			default:
				DECV_DBG_PRINT(" Error\n");
				break;	
			
		}
		DECV_DBG_PRINT("\t\tvbv buf size 0x%08x\n", info.vbv_size);	
		DECV_DBG_PRINT("\t\tvbv valid size 0x%08x\n", info.valid_size);	
		DECV_DBG_PRINT("\t\tvbv read ptr 0x%08x\n", info.read_p_offset);	
		DECV_DBG_PRINT("\t\tvbv write ptr 0x%08x\n", info.write_p_offset);			
		
		DECV_DBG_PRINT("\t\tvideo output mode is ");
		if(info.output_mode == MP_MODE)
			DECV_DBG_PRINT("full screen\n");
		else if(info.output_mode == PREVIEW_MODE)
			DECV_DBG_PRINT("preview output\n");
		else
			DECV_DBG_PRINT("unvalid\n");
		
		DECV_DBG_PRINT("\t\tdmx channel is ");
		if(info.cur_dma_ch == 0xFF)
			DECV_DBG_PRINT("unvalid\n");
		else
			DECV_DBG_PRINT("%d\n", info.cur_dma_ch);

		{
			/* some internal information */
			memset((void *)&dec_info, 0, sizeof(dec_info));
			ret = vdec_io_control(dev, VDEC_IO_GET_DECORE_STATUS, (UINT32)&dec_info);
			if(ret != RET_SUCCESS)
			{
				DECV_DBG_PRINT("%s : %d fail\n", __FUNCTION__, __LINE__);
				goto EXIT;
			}

			DECV_DBG_PRINT("\t\tintput frame count %d\n", dec_info.frames_decoded);
			DECV_DBG_PRINT("\t\toutput frame count %d\n", dec_info.frames_displayed);
		}
	}
	else
	{
		DECV_DBG_PRINT("STOPPED. No more other information is valid\n\n");
	}

	DECV_DBG_PRINT("ali decv dbg info end**************************\n");
	
EXIT:	

	return;
}

static void decv_dbg_pause(int argc, char **argv)
{
	if (argc != 0)
	{
		DECV_DBG_PRINT("Option \"-p\" should not with any argument(s)!\n");
		return;
	}

	vdec_set_dbg_level(0, 0, 0);	

	m_decv_dbg_level = 0;
	
	DECV_DBG_PRINT("%s : done\n", __FUNCTION__);
	return;
}

static void decv_dbg_start(int argc, char **argv)
{	
	if (argc != 0)
	{
		DECV_DBG_PRINT("Option \"-s\" should not with any argument(s)!\n");
		return;
	}

	vdec_set_dbg_level(m_decv_dbg_level & DECV_DBG_LEVEL_HLD
		, m_decv_dbg_level & DECV_DBG_LEVEL_KERNEL
		, m_decv_dbg_level & DECV_DBG_LEVEL_SEE);		
	
	DECV_DBG_PRINT("%s : done\n", __FUNCTION__);
	return;
}

static void decv_dbg_help(int argc, char **argv)
{
	if (argc != 0)
	{
		DECV_DBG_PRINT("Option \"-h\" should not with any argument(s)!\n");
		return;
	}

	DECV_DBG_PRINT("Module Name:\n");	
	DECV_DBG_PRINT("\tvideo decoder diagnostic module.\n\n");	

	DECV_DBG_PRINT("Synopsis:\n");	
	DECV_DBG_PRINT("\tadrdbg decv -cmd [arguments] ...\n\n");
	
	DECV_DBG_PRINT("Description:\n");
	
	DECV_DBG_PRINT("\t-h, --help\n");
	DECV_DBG_PRINT("\t\tself description of DECV.\n\n");	

	DECV_DBG_PRINT("\t-i, --info\n");
	DECV_DBG_PRINT("\t\tshow the selected video decoder info.\n\n");
	
	DECV_DBG_PRINT("\t-s, --start\n");
	DECV_DBG_PRINT("\t\tstart DECV tracing.\n\n");

	DECV_DBG_PRINT("\t-p, --pause\n");
	DECV_DBG_PRINT("\t\tpause DECV tracing.\n\n");

	DECV_DBG_PRINT("\t-l, --level num\n");
	DECV_DBG_PRINT("\t\tSet the tracing level of DECV (default is 1).\n");
	DECV_DBG_PRINT("\t\t0\t\t--no printout.\n");
	DECV_DBG_PRINT("\t\t1\t\t--show Hld printout infomation.\n");	
	DECV_DBG_PRINT("\t\t2\t\t--show Kernel printout infomation.\n");
	DECV_DBG_PRINT("\t\t3\t\t--show Kernel & Hld printout infomation.\n");	
	DECV_DBG_PRINT("\t\t4\t\t--show See printout infomation.\n");
	DECV_DBG_PRINT("\t\t5\t\t--show Hld & See printout infomation.\n");
	DECV_DBG_PRINT("\t\t6\t\t--show Kernel & See printout infomation.\n");	
	DECV_DBG_PRINT("\t\t7\t\t--show All printout infomation.\n");	

	DECV_DBG_PRINT("\t-t, --test mode\n");
	DECV_DBG_PRINT("\t\tSet the DECV test mode.\n");	
	DECV_DBG_PRINT("\t\tclear\t\t--clear all test mode.\n\n");
	DECV_DBG_PRINT("\t\terr_conti\t\t--cotinue to parse the stream even some error happen.\n\n");

	DECV_DBG_PRINT("Usage example:\n");
	DECV_DBG_PRINT("\tdecv -h\n");
	DECV_DBG_PRINT("\tdecv -s\n");
	DECV_DBG_PRINT("\tdecv -p\n");
	DECV_DBG_PRINT("\tdecv -i\n");
	DECV_DBG_PRINT("\tdecv -l 6\n");
	DECV_DBG_PRINT("\tdecv -t err_conti\n\n");	

	return;
}

static PARSE_COMMAND_LIST decvdbg_command[] =
{
	{ { NULL, NULL }, decv_dbg_test, decv_dbg_test_check, "test", "t", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, decv_dbg_level, NULL, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, decv_dbg_info, NULL, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
	{ { NULL, NULL }, decv_dbg_pause, NULL, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, decv_dbg_start, NULL, "start", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, decv_dbg_help, NULL, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

void decvdbg_register(void)
{
	debug_module_add("decv", &decvdbg_command[0], sizeof(decvdbg_command) / sizeof(decvdbg_command[0]));
}

INT32 decv_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &decvdbg_command[0];
	*list_cnt = ARRAY_SIZE(decvdbg_command);

	return RET_SUCCESS;
}

INT32 decv_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return decv_dbg_cmd_get(cmd_list, list_cnt);
}
