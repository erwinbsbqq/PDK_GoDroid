/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: adr_vpo_dbg.c
 *
 *  Description: Hld vpo driver dbg function
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
#include <hld_cfg.h>
#include <hld/adr_hld_dev.h>
#include <hld/dis/adr_vpo.h>
#include <hld/dbg/adr_dbg_parser.h>

#include <ali_video_common.h>

#include "adr_vpo_dbg.h"

#define __ADR_VPO_DBG
#ifdef __ADR_VPO_DBG
#define VPO_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DIS, fmt, ##args); \
			} while(0)
#else
#define VPO_DBG_PRINT(...)
#endif

#ifdef ADR_IPC_ENABLE
extern int *p_vpo_dbg_on;
#else
extern int g_vpo_dbg_on;
#endif
static int m_dis_dbg_level = DIS_DBG_LEVEL_HLD;

static void dis_dbg_on(int argc, char **argv)
{
	struct vpo_device *dev = dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	RET_CODE ret = RET_SUCCESS;
	BOOL on = 1;

	if (argc != 0)
	{
		VPO_DBG_PRINT("Option \"-on\" should not with any argument(s)!\n");
		return;
	}

	ret = vpo_win_onoff(dev, on);
	if(ret != RET_SUCCESS)
		VPO_DBG_PRINT("%s : fail\n", __FUNCTION__);
	else
		VPO_DBG_PRINT("%s : on dis done\n", __FUNCTION__);
	
EXIT:
	return;
}

static void dis_dbg_off(int argc, char **argv)
{
	struct vpo_device *dev = dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	RET_CODE ret = RET_SUCCESS;
	BOOL on = 0;

	if (argc != 0)
	{
		VPO_DBG_PRINT("Option \"-off\" should not with any argument(s)!\n");
		return;
	}

	ret = vpo_win_onoff(dev, on);
	if(ret != RET_SUCCESS)
		VPO_DBG_PRINT("%s : fail\n", __FUNCTION__);
	else
		VPO_DBG_PRINT("%s : off dis done\n", __FUNCTION__);
	
EXIT:
	return;
}

static void dis_dbg_level(int argc, char **argv)
{
	int level = 0;
	int ret = 0;

	if(argc != 1)
	{
		VPO_DBG_PRINT("%s : The param number should be one!\n", __FUNCTION__);
		return;
	}

	ret = sscanf(*argv, "%d", &level);
	if((ret == 0) || (level < 0) 
		|| (level > (DIS_DBG_LEVEL_HLD | DIS_DBG_LEVEL_KERNEL | DIS_DBG_LEVEL_SEE)))
	{
		VPO_DBG_PRINT("%s : param fail %s\n", __FUNCTION__, *argv);
		return;
	}
	
	m_dis_dbg_level = level;
	
	VPO_DBG_PRINT("%s : level %d\n", __FUNCTION__, level);		
	return;
}

static void dis_dbg_info(int argc, char **argv)
{
	struct vpo_device *dev = dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	RET_CODE ret = RET_SUCCESS;
	struct vpo_io_get_info info;

	if (argc != 0)
	{
		VPO_DBG_PRINT("Option \"-i\" should not with any argument(s)!\n");
		return;
	}

	memset((void *)&info, 0, sizeof(info));
	ret = vpo_ioctl(dev, VPO_IO_GET_INFO, (UINT32)&info);
	if(ret != RET_SUCCESS)
	{
		VPO_DBG_PRINT("%s : fail\n", __FUNCTION__);
		goto EXIT;
	}
	
	VPO_DBG_PRINT("%s : as below**********************:\n\n", __FUNCTION__);
	
	VPO_DBG_PRINT("\t\ttv system ");
	switch(info.tvsys)
	{
		case PAL:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("PAL\n");
			else
				VPO_DBG_PRINT("576p25\n");
			break;
		case PAL_NC:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("PAL_NC\n");
			else
				VPO_DBG_PRINT("576p25\n");			
			break;			
		case NTSC:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("NTSC\n");
			else
				VPO_DBG_PRINT("480p30\n");			
			break;
		case PAL_M:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("PAL_M\n");
			else
				VPO_DBG_PRINT("480p30\n");				
			break;
		case PAL_N:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("PAL_N\n");
			else
				VPO_DBG_PRINT("576p25\n");			
			break;
		case PAL_60:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("PAL_60\n");
			else
				VPO_DBG_PRINT("576p25\n");			
			break;
		case NTSC_443:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("NTSC_443\n");
			else
				VPO_DBG_PRINT("480p30\n");				
			break;
		case LINE_720_25:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("fail %d %d\n", info.tvsys, info.bprogressive);
			else
				VPO_DBG_PRINT("720p50\n");	
			break;
		case LINE_720_30:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("fail %d %d\n", info.tvsys, info.bprogressive);
			else
				VPO_DBG_PRINT("720p60\n");				
			break;
		case LINE_1080_25:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("1080i25\n");
			else
				VPO_DBG_PRINT("1080p25\n");				
			break;
		case LINE_1080_30:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("1080i30\n");
			else
				VPO_DBG_PRINT("1080p30\n");				
			break;
		case LINE_1080_50:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("fail %d %d\n", info.tvsys, info.bprogressive);
			else
				VPO_DBG_PRINT("1080p50\n");		
			break;
		case LINE_1080_60:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("fail %d %d\n", info.tvsys, info.bprogressive);
			else
				VPO_DBG_PRINT("1080p60\n");					
			break;
		case LINE_1080_24:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("fail %d %d\n", info.tvsys, info.bprogressive);
			else
				VPO_DBG_PRINT("1080p24\n");					
			break;
		case LINE_1152_ASS:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("1152_ASS\n");
			else
				VPO_DBG_PRINT("fail %d %d\n", info.tvsys, info.bprogressive);				
			break;
		case LINE_1080_ASS:
			if(info.bprogressive == FALSE)
				VPO_DBG_PRINT("1080_ASS\n");
			else
				VPO_DBG_PRINT("fail %d %d\n", info.tvsys, info.bprogressive);					
			break;
		default:
			VPO_DBG_PRINT("\t\ttv system fail %d %d\n", info.tvsys, info.bprogressive);
			break;
	}

	VPO_DBG_PRINT("\t\tdisplay frame count %d\n", info.display_index);
	VPO_DBG_PRINT("\t\tframe width %d\n", info.source_width);
	VPO_DBG_PRINT("\t\tframe height %d\n", info.source_height);
	VPO_DBG_PRINT("\t\tscreen width %d\n", info.des_width);
	VPO_DBG_PRINT("\t\tscreen height %d\n\n", info.des_height);
	
	VPO_DBG_PRINT("ali dis dbg info end**************************\n");
	
EXIT:	
	return;
}

static void dis_dbg_pause(int argc, char **argv)
{
	if (argc != 0)
	{
		VPO_DBG_PRINT("Option \"-p\" should not with any argument(s)!\n");
		return;
	}

#ifdef ADR_IPC_ENABLE
	*p_vpo_dbg_on = 0;
#else
	g_vpo_dbg_on = 0;
#endif
			
	VPO_DBG_PRINT("%s : done\n", __FUNCTION__);
	return;
}

static void dis_dbg_start(int argc, char **argv)
{	
	if (argc != 0)
	{
		VPO_DBG_PRINT("Option \"-s\" should not with any argument(s)!\n");
		return;
	}

	if(m_dis_dbg_level & DIS_DBG_LEVEL_HLD)
	{
#ifdef ADR_IPC_ENABLE
		*p_vpo_dbg_on = 1;
#else
		g_vpo_dbg_on = 1;
#endif
	}
	
	VPO_DBG_PRINT("%s : done\n", __FUNCTION__);
	return;
}

static void dis_dbg_help(int argc, char **argv)
{
	if (argc != 0)
	{
		VPO_DBG_PRINT("Option \"-h\" should not with any argument(s)!\n");
		return;
	}

	VPO_DBG_PRINT("Module Name:\n");	
	VPO_DBG_PRINT("\tvideo display diagnostic module.\n\n");	

	VPO_DBG_PRINT("Synopsis:\n");	
	VPO_DBG_PRINT("\tadrdbg dis -cmd [arguments] ...\n\n");
	
	VPO_DBG_PRINT("Description:\n");
	
	VPO_DBG_PRINT("\t-h, --help\n");
	VPO_DBG_PRINT("\t\tSelf description of DIS.\n\n");	

	VPO_DBG_PRINT("\t-i, --info\n");
	VPO_DBG_PRINT("\t\tShow DIS info.\n\n");

	VPO_DBG_PRINT("\t-o, -on\n");
	VPO_DBG_PRINT("\t\tEnable the display function of DIS.\n\n");		

	VPO_DBG_PRINT("\t-f, -off\n");
	VPO_DBG_PRINT("\t\tDisable the display function of DIS.\n\n");	
	
	VPO_DBG_PRINT("\t-s, --start\n");
	VPO_DBG_PRINT("\t\tStart DIS tracing.\n\n");

	VPO_DBG_PRINT("\t-p, --pause\n");
	VPO_DBG_PRINT("\t\tPause DIS tracing.\n\n");

	VPO_DBG_PRINT("\t-l, --level num\n");
	VPO_DBG_PRINT("\t\tSet the tracing level of DIS (default is 1).\n");
	VPO_DBG_PRINT("\t\t0\t\t--no printout.\n");
	VPO_DBG_PRINT("\t\t1\t\t--show Hld printout infomation.\n");	
	VPO_DBG_PRINT("\t\t2\t\t--show Kernel printout infomation.\n");
	VPO_DBG_PRINT("\t\t3\t\t--show Kernel & Hld printout infomation.\n");
	VPO_DBG_PRINT("\t\t4\t\t--show See printout infomation.\n");
	VPO_DBG_PRINT("\t\t5\t\t--show Hld & See printout infomation.\n");
	VPO_DBG_PRINT("\t\t6\t\t--show Kernel & See printout infomation.\n");
	VPO_DBG_PRINT("\t\t7\t\t--show All printout infomation.\n\n");	

	VPO_DBG_PRINT("\nUsage example:\n");
	VPO_DBG_PRINT("\tdis -h\n");
	VPO_DBG_PRINT("\tdis -s\n");
	VPO_DBG_PRINT("\tdis -p\n");
	VPO_DBG_PRINT("\tdis -i\n");
	VPO_DBG_PRINT("\tdis -on\n");
	VPO_DBG_PRINT("\tdis -off\n");
	VPO_DBG_PRINT("\tdis -l 6\n");
	
	return;
}

static PARSE_COMMAND_LIST disdbg_command[] =
{
	{ { NULL, NULL }, dis_dbg_on, NULL, "on", "on", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dis_dbg_off, NULL, "off", "off", 0, 0, NULL, 0, 0, NULL, 0, 0 },		
	{ { NULL, NULL }, dis_dbg_level, NULL, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dis_dbg_info, NULL, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
	{ { NULL, NULL }, dis_dbg_pause, NULL, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dis_dbg_start, NULL, "start", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, dis_dbg_help, NULL, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

void disdbg_register(void)
{
	debug_module_add("dis", &disdbg_command[0], sizeof(disdbg_command)/sizeof(disdbg_command[0]));
}

INT32 dis_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &disdbg_command[0];
	*list_cnt = ARRAY_SIZE(disdbg_command);

	return RET_SUCCESS;
}

INT32 dis_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return dis_dbg_cmd_get(cmd_list, list_cnt);
}
