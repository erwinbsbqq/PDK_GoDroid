/*****************************************************************************
*	Copyrights(C) 2012 Acer Laboratries inc. All Rights Reserved.
*
*	FILE NAME:		adr_dbg_osd.c
*
*	DESCRIPTION:	OSD debug module.
*
*	HISTORY:
*						Date 	 Author      Version 	  Notes
*					=========	=========	=========	===========
*					2012-12-6	 Leo.Ma      Ver 1.0	Create file.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <osal/osal_task.h>
#include <hld/adr_hld_dev.h>
#include <hld/dbg/adr_dbg_parser.h>

#include <hld/osd/adr_osddrv.h>
#include <hld_cfg.h>

#define __ADR_OSD2_DBG 

#ifdef __ADR_OSD2_DBG 
#define OSD_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(OSD, fmt, ##args); \
			} while(0)

#else
#define OSD_DBG_PRINT(...)
#endif

#ifndef BIT_PER_PIXEL
#define	BIT_PER_PIXEL	(32)
#endif

#ifndef OSD_TRANSPARENT_COLOR
#define OSD_TRANSPARENT_COLOR	0//0xFF
#endif

#ifndef OSD_TRANSPARENT_COLOR_BYTE
#define OSD_TRANSPARENT_COLOR_BYTE 		0//0xFF
#endif

#ifndef FACTOR
#define	FACTOR	0
#endif

typedef enum osd2_dbg_color_mode {
	OSD2_DBG_ARGB1555 = 1,
	OSD2_DBG_ARGB8888,
} OSD2_DBG_COLOR_MODE_E;

typedef struct osd2_dbg_info {
	OSD2_DBG_COLOR_MODE_E color_mode;
	INT16 left;
	INT16 top;
	INT16 width;
	INT16 height;
	UINT32 alpha_value;
} OSD2_DBG_INFO_S;

static struct osd_device * g_osd_dev_layer2;
static OSD2_DBG_INFO_S Osd2DbgInfo;

static void osd2_dbg_init(void)
{
	memset(&Osd2DbgInfo, 0, sizeof(Osd2DbgInfo));
	Osd2DbgInfo.color_mode = OSD2_DBG_ARGB1555;
	Osd2DbgInfo.alpha_value = 255;
	Osd2DbgInfo.width = 1440;
	Osd2DbgInfo.height = 720;

	return;
}

static void osd2_dbg_show_help(int argc, char **argv)
{
	OSD_DBG_PRINT("\nModule name:\n");
	OSD_DBG_PRINT("\tosd2\n");
	OSD_DBG_PRINT("\nModule Description:\n");
	OSD_DBG_PRINT("\tOSD layer 2 testing debug module.\n");	
	OSD_DBG_PRINT("\nSyntax:\n");
	OSD_DBG_PRINT("\tosd2 -option [arguments]\n");
	OSD_DBG_PRINT("\nDescription:\n");
	OSD_DBG_PRINT("\t-h, --help\n");
	OSD_DBG_PRINT("\t\tSelf description of this debugging moduld.\n");
	OSD_DBG_PRINT("\t-i, --info\n");
	OSD_DBG_PRINT("\t\tShow OSD layer 2 configure data.\n");
	OSD_DBG_PRINT("\t-op\n");
	OSD_DBG_PRINT("\t\tOpen OSD layer 2.\n");
	OSD_DBG_PRINT("\t-cl\n");
	OSD_DBG_PRINT("\t\tClose OSD layer 2.\n");
	OSD_DBG_PRINT("\t-rect num1 num2 num3 num4\n");
	OSD_DBG_PRINT("\t\tConfigure OSD layer 2 region size(The default value is \"0\", \"0\", \"1280\", \"720\").\n");
	OSD_DBG_PRINT("\t-alp, --alpha number\n");
	OSD_DBG_PRINT("\t\tConfigure OSD layer 2 alpha value(The default value is \"255\").\n");
	OSD_DBG_PRINT("\t-clrm number\n");
	OSD_DBG_PRINT("\t\tConfigure OSD layer 2 color mode(The default value is \"2\").\n");
	OSD_DBG_PRINT("\nUsage example:\n");
	OSD_DBG_PRINT("\tosd2 -h[--help]\n");
	OSD_DBG_PRINT("\tosd2 -i[--info]\n");
	OSD_DBG_PRINT("\tosd2 -op\n");
	OSD_DBG_PRINT("\tosd2 -cl\n");	
	OSD_DBG_PRINT("\tosd2 -rect[--rect] 0 0 1280 720\n");
	OSD_DBG_PRINT("\tosd2 -alp[--alpha] 255\n");
	OSD_DBG_PRINT("\tosd2 -clrm 2\n");

	return;
}

static void osd2_dbg_show_info(int argc, char **argv)
{
	OSD_DBG_PRINT("OSD2 left: %d\n", Osd2DbgInfo.left);
	OSD_DBG_PRINT("OSD2 top: %d\n", Osd2DbgInfo.top);
	OSD_DBG_PRINT("OSD2 width: %d\n", Osd2DbgInfo.width);
	OSD_DBG_PRINT("OSD2 height: %d\n", Osd2DbgInfo.height);
	OSD_DBG_PRINT("OSD2 alpha value: %d\n", Osd2DbgInfo.alpha_value);
	if (OSD2_DBG_ARGB1555 == Osd2DbgInfo.color_mode)
		OSD_DBG_PRINT("OSD2 color mode: OSD2_DBG_ARGB1555\n");
	else if (OSD2_DBG_ARGB8888 == Osd2DbgInfo.color_mode)
		OSD_DBG_PRINT("OSD2 color mode: OSD2_DBG_ARGB8888\n");

	return;
}

static void osd2_dbg_layer_open(int argc, char **argv)
{
	RET_CODE ret;
	struct OSDPara tOpenPara;
	struct OSDRect region1, region2, rc1, rc2;

	OSDDrv_Close((HANDLE)g_osd_dev_layer2);

	if (OSD2_DBG_ARGB1555 == Osd2DbgInfo.color_mode)
		tOpenPara.eMode = OSD_HD_ARGB1555;
	else if (OSD2_DBG_ARGB8888 == Osd2DbgInfo.color_mode)
		tOpenPara.eMode = OSD_HD_ARGB8888;
	else
		tOpenPara.eMode = OSD_HD_ARGB8888;

	tOpenPara.uGAlphaEnable = 0;
	tOpenPara.uGAlpha = 0x0F;
	tOpenPara.uPalletteSel = 0;
	ret = OSDDrv_Open((HANDLE)g_osd_dev_layer2, &tOpenPara);
	if (ret != RET_SUCCESS)
	{
		OSD_DBG_PRINT("OSD layer 2 opened fail!\n");
		return;
	}
	osal_task_sleep(20);

	//create test region
	region1.uLeft = Osd2DbgInfo.left;
	region1.uTop = Osd2DbgInfo.top;
	region1.uWidth = Osd2DbgInfo.width;
	region1.uHeight = Osd2DbgInfo.height;
	rc1.uLeft = 0;
	rc1.uTop= 0;
	rc1.uWidth = region1.uWidth;
	rc1.uHeight = region1.uHeight;

	ret = OSDDrv_DeleteRegion((HANDLE)g_osd_dev_layer2, 0);
	ret = OSDDrv_CreateRegion((HANDLE)g_osd_dev_layer2, 0, &region1, NULL);
	if (ret != RET_SUCCESS)
	{
		OSD_DBG_PRINT("OSD layer 2 created region fail!\n");
		return;
	}

	/* Fill full screen transparent */
	if (OSD2_DBG_ARGB1555 == Osd2DbgInfo.color_mode)
		OSDDrv_RegionFill((HANDLE)g_osd_dev_layer2, 0, &rc1, (UINT32)0xFc00);
	else if (OSD2_DBG_ARGB8888 == Osd2DbgInfo.color_mode)
		OSDDrv_RegionFill((HANDLE)g_osd_dev_layer2, 0, &rc1, (UINT32)0xFFFF0000);
	else
		OSDDrv_RegionFill((HANDLE)g_osd_dev_layer2, 0, &rc1, (UINT32)OSD_TRANSPARENT_COLOR);

	ret = OSDDrv_IoCtl((HANDLE)g_osd_dev_layer2, OSD_IO_SET_GLOBAL_ALPHA, Osd2DbgInfo.alpha_value);
	if (ret != RET_SUCCESS)
	{
		OSD_DBG_PRINT("OSD layer 2 created region fail!\n");
		return;
	}	
	OSDDrv_ShowOnOff((HANDLE)g_osd_dev_layer2, OSDDRV_ON);

	return;
}

static void osd2_dbg_layer_close(int argc, char **argv)
{
	OSDDrv_Close((HANDLE)g_osd_dev_layer2);
	OSDDrv_DeleteRegion((HANDLE)g_osd_dev_layer2, 0);
}

static ARG_CHK osd2_dbg_no_para_preview(int argc, char **argv, char *option)
{
	if (argc != 0)
	{
		OSD_DBG_PRINT("Option \"%s\": Should not be with any argument(s)!\n", option);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static void osd2_dbg_region_set(int argc, char **argv)
{
	if (argc != 4)
	{
		OSD_DBG_PRINT("Needs 4 arguments!\n");
		return;
	}
	
	sscanf(*argv++, "%d", &Osd2DbgInfo.left);
	sscanf(*argv++, "%d", &Osd2DbgInfo.top);
	sscanf(*argv++, "%d", &Osd2DbgInfo.width);
	sscanf(*argv++, "%d", &Osd2DbgInfo.height);

	return;
}

static ARG_CHK osd2_dbg_region_preview(int argc, char **argv, char *option)
{
	INT32 number;

	if (argc != 4)
	{
		OSD_DBG_PRINT("Option \"%s\": Needs 4 arguments!\n", option);
		return DBG_ARG_INVALID;
	}

	while (argc-- > 0)
	{
		if (sscanf(*argv, "%d", &number) != 1)
		{
			OSD_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number!\n", option, *argv++);
			return DBG_ARG_INVALID;
		}
	}

	return DBG_ARG_VALID;
}

static void osd2_dbg_alpha_set(int argc, char **argv)
{	
	if (argc != 1)
	{
		OSD_DBG_PRINT("Just needs 1 argument!\n");
		return;
	}
	
	sscanf(*argv++, "%d", &Osd2DbgInfo.alpha_value);

	return;
}

static ARG_CHK osd2_dbg_alpha_preview(int argc, char **argv, char *option)
{
	INT32 number, ret;
	
	if (argc != 1)
	{
		OSD_DBG_PRINT("Option \"%s\": Just needs 1 argument!\n", option);
		return DBG_ARG_INVALID;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret)
	{
		OSD_DBG_PRINT("Option \"%s\": Argument \"%s\" not a hexadecimal number!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	else if (number < 0 || number > 255)
	{
		OSD_DBG_PRINT("Option \"%s\": Alpha value \"%s\" should within 0 - 255!\n", option, *argv);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}

static void osd2_dbg_color_mode_set(int argc, char **argv)
{
	INT32 number, ret;

	if (argc != 1)
	{
		OSD_DBG_PRINT("Just needs 1 argument!\n");
		return;
	}
	
	sscanf(*argv++, "%d", &Osd2DbgInfo.color_mode);

	return;
}

static ARG_CHK osd2_dbg_color_mode_preview(int argc, char **argv, char *option)
{
	INT32 number, ret;

	if (argc != 1)
	{
		OSD_DBG_PRINT("Option \"%s\": Just needs 1 argument!\n", option);
		return DBG_ARG_INVALID;
	}
	
	ret = sscanf(*argv, "%d", &number);
	if (0 == ret)
	{
		OSD_DBG_PRINT("Option \"%s\": Argument \"%s\" not a number!\n", option, *argv);
		return DBG_ARG_INVALID;
	}
	else if (number < OSD2_DBG_ARGB1555 || number > OSD2_DBG_ARGB8888)
	{
		OSD_DBG_PRINT("Option \"%s\": Color mode value \"%s\" should within 1 - 2!\n", option, *argv);
		return DBG_ARG_INVALID;
	}

	return DBG_ARG_VALID;
}


PARSE_COMMAND_LIST osd2dbg_command[] = {
	{ { NULL, NULL }, osd2_dbg_show_help, osd2_dbg_no_para_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, osd2_dbg_show_info, osd2_dbg_no_para_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, osd2_dbg_layer_open, osd2_dbg_no_para_preview, NULL, "op", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, osd2_dbg_layer_close, osd2_dbg_no_para_preview, NULL, "cl", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, osd2_dbg_region_set, osd2_dbg_region_preview, "rect", "rect", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, osd2_dbg_alpha_set, osd2_dbg_alpha_preview, "alpha", "alp", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, osd2_dbg_color_mode_set, osd2_dbg_color_mode_preview, NULL, "clrm", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

INT32 osd2_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &osd2dbg_command[0];
	*list_cnt = ARRAY_SIZE(osd2dbg_command);

	g_osd_dev_layer2 = (struct osd_device*)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
	if (NULL == g_osd_dev_layer2)
		return RET_FAILURE;

	osd2_dbg_init();

	return RET_SUCCESS;
}

INT32 osd2_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return osd2_dbg_cmd_get(cmd_list, list_cnt);
}
