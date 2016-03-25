/****************************************************************************
	if(argc != 1)
	{
		OSD_DBG_PRINT("%s : The param number should be one!\n", __FUNCTION__);
		return;
	}

 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: osddrv_dbg.c
 *
 *  Description: Hld osd driver dbg function
 *
 *  History:
 *      Date        Author         	Version   Comment
 *      ====        ======         =======   =======
 *  1.  2012.10.22  Sam			0.9		 inited
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <linux/fb.h>

#include <adr_basic_types.h>
#include <hld_cfg.h>
#include <adr_retcode.h>
#include <hld/adr_hld_dev.h>
#include <hld/osd/adr_osddrv.h>
#include <hld/dbg/adr_dbg_parser.h>
#include <osal/osal.h>

#include <ali_video_common.h>

#include "adr_osddrv_dbg.h"

#define __ADR_OSD_DBG
#ifdef __ADR_OSD_DBG
#define OSD_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(OSD, fmt, ##args); \
			} while(0)
#else
#define OSD_DBG_PRINT(...)
#endif

#define MAX_FB_NUM	2
#ifdef ADR_ALIDROID
static char m_fb0_path[] = "/dev/graphics/fb0";
static char m_fb2_path[] = "/dev/graphics/fb2";

#else
static char m_fb0_path[] = "/dev/fb0";
static char m_fb2_path[] = "/dev/fb2";
#endif

static int m_fb_dbg_level = FB_DBG_LEVEL_HLD;

#ifdef ADR_IPC_ENABLE
extern int *p_osd_dbg_on;
#else
extern int g_osd_dbg_on;
#endif

static int get_fb_info(int fb_handle)
{
	struct alifbio_fbinfo_data_pars fbinfo;
	int ret = 0;
	
	ret = ioctl(fb_handle, FBIO_GET_FBINFO_DATA, &fbinfo);
	if(ret < 0)
	{
		OSD_DBG_PRINT("%s : FBIO_GET_FBINFO_DATA fail\n", __FUNCTION__);	
		return -1;
	}
	OSD_DBG_PRINT("\t\tphys mem start address 0x%x\n", (int)fbinfo.mem_start);
	OSD_DBG_PRINT("\t\tphsy mem size 0x%x\n", (int)fbinfo.mem_size);
	OSD_DBG_PRINT("\t\tvirtual xres %d\n", fbinfo.xres_virtual);
	OSD_DBG_PRINT("\t\tvirtual yres %d\n", fbinfo.yres_virtual);
	
	return 1;
}

/* scale and postion */
static void fb_test_1(void)
{
#if 0
	struct osd_device *dev = dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
	struct OSDPara para;
	struct OSDRect rect;
	osd_scale_param scale_para;
	int pos_x, pos_y, h_div, h_mul, v_div, v_mul;

	memset((void *)&para, 0, sizeof(para));
	para.eMode = OSD_256_COLOR;
	para.uGAlpha = 0x7f;
	para.uGAlphaEnable = 0;
	para.uPalletteSel = 0;
	OSDDrv_Open((HANDLE)dev, &para);

#if 1
	memset((void *)&rect, 0, sizeof(rect));
	rect.uLeft = 0;
	rect.uTop = 0;
#if 1
	rect.uWidth = 1280;
	rect.uHeight = 200;	
#else
	rect.uWidth = 1920;
	rect.uHeight = 200;	
#endif	
	OSDDrv_CreateRegion((HANDLE)dev, 0, &rect, NULL);

	UINT32 pallette[256];

	memset((void *)pallette, 0, 1024);

	pallette[0] = 0xFFFF0000;
	pallette[1] = 0xFF00FF00;
	OSDDrv_SetPallette((HANDLE)dev, (UINT8 *)pallette, 256, OSDDRV_RGB);
	
	rect.uLeft = 0;
	rect.uTop = 0;
#if 1
	rect.uWidth = 1280;
	rect.uHeight = 200;	
#else
	rect.uWidth = 1920;
	rect.uHeight = 200;	
#endif			
	OSDDrv_RegionFill((HANDLE)dev, 0, &rect, 0x00);

#if 0
	rect.uLeft = 0;
	rect.uTop = 0;
	rect.uWidth = 200;
	rect.uHeight = 200;		
	OSDDrv_RegionFill((HANDLE)dev, 0, &rect, 0xFF00FF00);	

	rect.uLeft = 1720;
	rect.uTop = 0;
	rect.uWidth = 200;
	rect.uHeight = 200;		
	OSDDrv_RegionFill((HANDLE)dev, 0, &rect, 0xFF0000FF);	
#endif

	OSDDrv_ShowOnOff((HANDLE)dev, TRUE);

	rect.uLeft = 0;
	rect.uTop = 200;
	OSDDrv_SetRegionPos((HANDLE)dev, 0, &rect);
		
	osal_task_sleep(5000);	

#if 0
	scale_para.h_div = 128;
	scale_para.h_mul = 192;
#else
	scale_para.h_div = 192;
	scale_para.h_mul = 128;
#endif	
	scale_para.v_div = 128;
	scale_para.v_mul = 720;	
	OSDDrv_Scale((HANDLE)dev, OSD_SCALE_WITH_PARAM, (UINT32)&scale_para);		
#else
	pos_x = 0;
	pos_y = 360;
	
	h_mul = 1;
	h_div = 1;

	while(1)
	{
		pos_y += 10;
		
		if(pos_y > 360)
			break;

		scale_para.h_div = h_div;
		scale_para.h_mul = h_mul;
		scale_para.v_div = 720;
		scale_para.v_mul = (720 - pos_y);

		rect.uLeft = pos_x;
		rect.uTop = pos_y  * 720 / scale_para.v_mul;
		rect.uWidth = 1280;
		rect.uHeight = 720;
		OSDDrv_SetRegionPos((HANDLE)dev, 0, &rect);
		
		OSDDrv_Scale((HANDLE)dev, OSD_SCALE_WITH_PARAM, (UINT32)&scale_para);

		osal_task_sleep(5000);
	}
#endif	
#endif
}

static void fb_dbg_test(int argc, char **argv)
{
	int test_cmd;
	int ret = 0;

	if(argc != 1)
	{
		OSD_DBG_PRINT("%s : The param number should be one!\n", __FUNCTION__);
		return;
	}

	ret = sscanf(*argv, "%d", &test_cmd);
	if(ret == 0)
	{
		OSD_DBG_PRINT("%s : param fail %s\n", __FUNCTION__, *argv);
		return;
	}

	switch(test_cmd)
	{
		case 1:
			fb_test_1();
			break;
		default:
			break;
	}
	
}

static void fb_dbg_on(int argc, char **argv)
{
	int fb_handle = 0;
	int fb_num = 0;
	char *fb_path = NULL;
	int ret = 0;

	if(argc != 1)
	{
		OSD_DBG_PRINT("%s : The param number should be one!\n", __FUNCTION__);
		return;
	}

	ret = sscanf(*argv, "%d", &fb_num);
	if((ret == 0) || (fb_num < 0) || (fb_num >= MAX_FB_NUM))
	{
		OSD_DBG_PRINT("%s : param fail %s\n", __FUNCTION__, *argv);
		return;
	}
	
	switch(fb_num)
	{
		case 0:
			fb_path = m_fb0_path;
			break;
		case 1:
			fb_path = m_fb2_path;
			break;
		default:
			OSD_DBG_PRINT("%s : don't have this fb num %d\n", __FUNCTION__, fb_num);
			goto EXIT;
	}
	
	fb_handle = open(fb_path, O_RDWR | O_SYNC | O_CLOEXEC);
	if(fb_handle < 0)
	{
		OSD_DBG_PRINT("%s : open %s dev fail\n", __FUNCTION__, fb_path);
		goto EXIT;
	}
	else
	{
		ret = ioctl(fb_handle, FBIO_WIN_ONOFF, 1);		
		if(ret < 0)
			OSD_DBG_PRINT("%s : on fb %d fail\n", __FUNCTION__, fb_num);
		else
			OSD_DBG_PRINT("%s : on fb %d done\n", __FUNCTION__, fb_num);
	}
	
	close(fb_handle);	
	
EXIT:
	return;
}

static void fb_dbg_off(int argc, char **argv)
{
	int fb_handle = 0;
	int fb_num = 0;
	char *fb_path = NULL;
	int ret = 0;

	if(argc != 1)
	{
		OSD_DBG_PRINT("%s : The param number should be one!\n", __FUNCTION__);
		return;
	}

	ret = sscanf(*argv, "%d", &fb_num);
	if((ret == 0) || (fb_num < 0) || (fb_num >= MAX_FB_NUM))
	{
		OSD_DBG_PRINT("%s : param fail %s\n", __FUNCTION__, *argv);
		return;
	}
	
	switch(fb_num)
	{
		case 0:
			fb_path = m_fb0_path;
			break;
		case 1:
			fb_path = m_fb2_path;
			break;
		default:
			OSD_DBG_PRINT("%s : don't have this fb num %d\n", __FUNCTION__, fb_num);
			goto EXIT;
	}
	
	fb_handle = open(fb_path, O_RDWR | O_SYNC | O_CLOEXEC);
	if(fb_handle < 0)
	{
		OSD_DBG_PRINT("%s : open %s dev fail\n", __FUNCTION__, fb_path);
		goto EXIT;
	}
	else
	{
		ret = ioctl(fb_handle, FBIO_WIN_ONOFF, 0);		
		if(ret < 0)
			OSD_DBG_PRINT("%s : off fb %d fail\n", __FUNCTION__, fb_num);
		else
			OSD_DBG_PRINT("%s : off fb %d done\n", __FUNCTION__, fb_num);
	}
	
	close(fb_handle);		
	
EXIT:
	return;
}

static void fb_dbg_level(int argc, char **argv)
{
	int level = 0;
	int ret = 0;
	
	if(argc != 1)
	{
		OSD_DBG_PRINT("%s : The param number should be one!\n", __FUNCTION__);
		return;
	}
	
	ret = sscanf(*argv, "%d", &level);
	if((ret == 0) || (level < 0) 
		|| (level > (FB_DBG_LEVEL_HLD | FB_DBG_LEVEL_KERNEL | FB_DBG_LEVEL_SEE)))
	{
		OSD_DBG_PRINT("%s : param fail %s\n", __FUNCTION__, *argv);
		return;
	}
	
	m_fb_dbg_level = level;
	
	OSD_DBG_PRINT("%s : level %d\n", __FUNCTION__, level);		
	return;
}

static void fb_dbg_info(int argc, char **argv)
{
	int fb_handle = 0;
	int ret = 0;
	
	if (argc != 0)
	{
		OSD_DBG_PRINT("Option \"-i\" should not with any argument(s)!\n");
		return;
	}

	OSD_DBG_PRINT("%s : as below**********************:\n", __FUNCTION__);
	#ifdef ADR_ALIDROID
	fb_handle = open("/dev/graphics/fb0", O_RDWR | O_SYNC | O_CLOEXEC);
	#else
	fb_handle = open("/dev/fb0", O_RDWR | O_SYNC | O_CLOEXEC);
	#endif
	if(fb_handle < 0)
	{
		#ifdef ADR_ALIDROID
		OSD_DBG_PRINT("open /dev/graphics/fb0 dev fail\n");
		#else
		OSD_DBG_PRINT("open /dev/fb0 dev fail\n");
		#endif
	}
	else
	{
		OSD_DBG_PRINT("\tthe first FB info:\n");
		ret = get_fb_info(fb_handle);
		if(ret < 0)
		{
			OSD_DBG_PRINT("\t%s:get the first FB info fail\n\n", __FUNCTION__);
		}
		else
		{
			OSD_DBG_PRINT("\t%s:get the first FB info done\n\n", __FUNCTION__);	
		}
	}

	close(fb_handle);	
	
	#ifdef ADR_ALIDROID
	fb_handle = open("/dev/graphics/fb2", O_RDWR | O_SYNC | O_CLOEXEC);
	#else
	fb_handle = open("/dev/fb2", O_RDWR | O_SYNC | O_CLOEXEC);
	#endif
	if(fb_handle < 0)
	{
		OSD_DBG_PRINT("open /dev/fb2 dev fail\n");
		goto EXIT;
	}
	else
	{
		OSD_DBG_PRINT("\tthe second FB info:\n");
		ret = get_fb_info(fb_handle);
		if(ret < 0)
		{
			OSD_DBG_PRINT("\t%s:get the second FB info fail\n\n", __FUNCTION__);
		}
		else
		{
			OSD_DBG_PRINT("\t%s:get the second FB info done\n\n", __FUNCTION__);	
		}
	}

	close(fb_handle);	
	
EXIT:	
	OSD_DBG_PRINT("ali fb dbg info end**************************\n");
	return;
}

static void fb_dbg_pause(int argc, char **argv)
{
    int fb_handle = 0;
    int flag = 0;
    int ret = 0;
    
	if (argc != 0)
	{
		OSD_DBG_PRINT("Option \"-p\" should not with any argument(s)!\n");
		return;
	}

#ifdef ADR_IPC_ENABLE
    if(p_osd_dbg_on)
    {
	    *p_osd_dbg_on = 0;
    }
#else
	g_osd_dbg_on = 0;
#endif

    fb_handle = open(m_fb0_path, O_RDWR | O_SYNC | O_CLOEXEC);
    if(fb_handle >= 0)
    {
		ret = ioctl(fb_handle, FBIO_SET_GMA_DBG_FLAG, flag);		
		if(ret < 0)
			OSD_DBG_PRINT("%s : set gma dbg flag %d fail\n", __FUNCTION__, flag);
		else
			OSD_DBG_PRINT("%s : set gma dbg flag %d done\n", __FUNCTION__, flag);

        close(fb_handle);
    }

	OSD_DBG_PRINT("%s : done\n", __FUNCTION__);
	return;
}

static void fb_dbg_start(int argc, char **argv)
{
    int fb_handle = 0;
    int flag = 0;
    int ret = 0;
    
	if (argc != 0)
	{
		OSD_DBG_PRINT("Option \"-s\" should not with any argument(s)!\n");
		return;
	}

	if(m_fb_dbg_level & FB_DBG_LEVEL_HLD)
	{
#ifdef ADR_IPC_ENABLE
        if(p_osd_dbg_on)
        {
		    *p_osd_dbg_on = 1;
        }
#else	
		g_osd_dbg_on = 1;
#endif
	}

    if((m_fb_dbg_level & FB_DBG_LEVEL_SEE) 
       || (m_fb_dbg_level & FB_DBG_LEVEL_KERNEL)) 
    {
        if(m_fb_dbg_level & FB_DBG_LEVEL_KERNEL)
            flag = 1;
        else
            flag = 2;
        
    	fb_handle = open(m_fb0_path, O_RDWR | O_SYNC | O_CLOEXEC);
    	if(fb_handle < 0)
    	{
    		OSD_DBG_PRINT("%s : open %s dev fail\n", __FUNCTION__, m_fb0_path);
    	}
    	else
    	{
    		ret = ioctl(fb_handle, FBIO_SET_GMA_DBG_FLAG, flag);		
    		if(ret < 0)
    			OSD_DBG_PRINT("%s : set gma dbg flag %d fail\n", __FUNCTION__, flag);
    		else
    			OSD_DBG_PRINT("%s : set gma dbg flag %d done\n", __FUNCTION__, flag);

            close(fb_handle);
    	}
    }
	
	OSD_DBG_PRINT("%s : done\n", __FUNCTION__);
	return;
}

static void fb_dbg_help(int argc, char **argv)
{
	if (argc != 0)
	{
		OSD_DBG_PRINT("Option \"-h\" should not with any argument(s)!\n");
		return;
	}

	OSD_DBG_PRINT("Module Name:\n");	
	OSD_DBG_PRINT("\tfb diagnostic module.\n\n");	

	OSD_DBG_PRINT("Synopsis:\n");	
	OSD_DBG_PRINT("\tadrdbg fb -cmd [arguments] ...\n\n");
	
	OSD_DBG_PRINT("Description:\n");
	
	OSD_DBG_PRINT("\t-h, --help\n");
	OSD_DBG_PRINT("\t\tself description of FB.\n\n");	

	OSD_DBG_PRINT("\t-i, --info\n");
	OSD_DBG_PRINT("\t\tshow FB info.\n\n");

	OSD_DBG_PRINT("\t-on num\n");
	OSD_DBG_PRINT("\t\tEnable the display function of FB.\n\n");
	OSD_DBG_PRINT("\t\t0\t\t--the first FB device.\n");
	OSD_DBG_PRINT("\t\t1\t\t--the second FB device.\n");		

	OSD_DBG_PRINT("\t-off num\n");
	OSD_DBG_PRINT("\t\tDisable the display function of FB.\n\n");
	OSD_DBG_PRINT("\t\t0\t\t--the first FB device.\n");
	OSD_DBG_PRINT("\t\t1\t\t--the second FB devie.\n");	
	
	OSD_DBG_PRINT("\t-s, --start\n");
	OSD_DBG_PRINT("\t\tstart FB tracing.\n\n");

	OSD_DBG_PRINT("\t-p, --pause\n");
	OSD_DBG_PRINT("\t\tpause FB tracing.\n\n");

	OSD_DBG_PRINT("\t-l, --level num\n");
	OSD_DBG_PRINT("\t\tSet the tracing level of FB (default is 1).\n");
	OSD_DBG_PRINT("\t\t0\t\t--no printout.\n");
	OSD_DBG_PRINT("\t\t1\t\t--show Hld printout infomation.\n");	
	OSD_DBG_PRINT("\t\t2\t\t--show Kernel printout infomation.\n");
	OSD_DBG_PRINT("\t\t3\t\t--show Kernel & Hld printout infomation.\n");
	OSD_DBG_PRINT("\t\t4\t\t--show See printout infomation.\n");
	OSD_DBG_PRINT("\t\t5\t\t--show Hld & See printout infomation.\n");
	OSD_DBG_PRINT("\t\t6\t\t--show Kernel & See printout infomation.\n");
	OSD_DBG_PRINT("\t\t7\t\t--show All printout infomation.\n\n");	

	OSD_DBG_PRINT("\nUsage example:\n");
	OSD_DBG_PRINT("\tfb -h\n");
	OSD_DBG_PRINT("\tfb -s\n");
	OSD_DBG_PRINT("\tfb -p\n");
	OSD_DBG_PRINT("\tfb -i\n");
	OSD_DBG_PRINT("\tfb -on 0\n");
	OSD_DBG_PRINT("\tfb -off 0\n");
	OSD_DBG_PRINT("\tfb -l 6\n");
	
	return;
}

static PARSE_COMMAND_LIST fbdbg_command[] =
{
	{ { NULL, NULL }, fb_dbg_test, NULL, "test", "t", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
	{ { NULL, NULL }, fb_dbg_on, NULL, "on", "on", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, fb_dbg_off, NULL, "off", "off", 0, 0, NULL, 0, 0, NULL, 0, 0 },		
	{ { NULL, NULL }, fb_dbg_level, NULL, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, fb_dbg_info, NULL, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
	{ { NULL, NULL }, fb_dbg_pause, NULL, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, fb_dbg_start, NULL, "start", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, fb_dbg_help, NULL, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

void fbdbg_register(void)
{
	debug_module_add("fb", &fbdbg_command[0], sizeof(fbdbg_command)/sizeof(fbdbg_command[0]));
}

INT32 fb_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &fbdbg_command[0];
	*list_cnt = ARRAY_SIZE(fbdbg_command);

	return RET_SUCCESS;
}

INT32 fb_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return fb_dbg_cmd_get(cmd_list, list_cnt);
}
