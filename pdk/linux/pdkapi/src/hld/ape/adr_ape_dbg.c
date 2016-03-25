/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: adr_ape_dbg.c
 *
 *  Description: Hld ape driver dbg function
 *
 *  History:
 *      Date        Author         	Version   Comment
 *      ====        ======         =======   =======
 *  1.  2012.10.22  Sam			0.9		 inited
 ****************************************************************************/

#include <errno.h>
#include <stdio.h>

#include <adr_basic_types.h>
#include <adr_retcode.h>
#include <hld/adr_hld_dev.h>
#include <hld/decv/adr_decv.h>
#include <hld/ape/adr_ape.h>
#include <hld/dbg/adr_dbg_parser.h>

#include <ali_video_common.h>

#include "ape_priv.h"
#include "adr_ape_dbg.h"


#define __ADR_APE_DBG 

#ifdef __ADR_APE_DBG
#define APE_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(APE, fmt, ##args); \
			} while(0)
#else
#define APE_DBG_PRINT(...)
#endif

static int m_ape_dbg_level = APE_DBG_LEVEL_HLD;

static void ape_dbg_dump(int argc, char **argv)
{
	if((argc != 1) || (argv == NULL) || (argv[0] == NULL))
	{
		APE_DBG_PRINT("%s : param number should be one!\n", __FUNCTION__);
		return;
	}

	ape_dump_enable(*argv);

	APE_DBG_PRINT("%s: start to dump es %s\n", __FUNCTION__, *argv);
	return;
}

static void ape_dbg_play(int argc, char **argv)
{
	int stop = 0;
	int ret = 0;
	
	if((argc != 1) || (argv == NULL) || (argv[0] == NULL))
	{
		APE_DBG_PRINT("%s : param number should be one!\n", __FUNCTION__);
		return;
	}

	ret = sscanf(*argv, "%c", &stop);
	if((ret > 0) && (stop == 'p'))
	{
		ape_playback_stop();

		APE_DBG_PRINT("%s: stop play es\n", __FUNCTION__);
	}
	else
	{
		ape_playback_start(*argv);

		APE_DBG_PRINT("%s: start to play es %s\n", __FUNCTION__, *argv);
	}
	return;
}

static void ape_dbg_level(int argc, char **argv)
{
	int level = 0;
	int ret = 0;
	
	if(argc != 1)
	{
		APE_DBG_PRINT("%s : param number should be one!\n", __FUNCTION__);
		return;
	}
	
	ret = sscanf(*argv, "%d", &level);
	if((ret == 0) || (level < 0) 
		|| (level > (APE_DBG_LEVEL_HLD | APE_DBG_LEVEL_KERNEL | APE_DBG_LEVEL_SEE)))
	{
		APE_DBG_PRINT("%s : param fail %s\n", __FUNCTION__, *argv);
		return;
	}
	
	m_ape_dbg_level = level;
	
	APE_DBG_PRINT("%s : level %d\n", __FUNCTION__, level);		
	return;
}

static void ape_dbg_info(int argc, char **argv)
{
	ape_priv_t* ape_priv = ape_get_priv();
	audio_config_info_t audio_config_info;
	video_config_info_t video_config_info;

	memset((void *)&audio_config_info, 0, sizeof(audio_config_info));
	memset((void *)&video_config_info, 0, sizeof(video_config_info));

	APE_DBG_PRINT("%s : as below**********************:\n\n", __FUNCTION__);
	APE_DBG_PRINT("\tvdec info as below\n");
	if(ape_priv->v_init_flag)
	{
		memcpy((void *)&video_config_info, (void *)&ape_priv->video_info, sizeof(ape_priv->video_info));
		
		APE_DBG_PRINT("\t\ttype %s\n", ape_priv->codec_tag == h264 ? "h.264" : 
			(ape_priv->codec_tag == mpg2 ? "mpeg" : 
			(ape_priv->codec_tag == rmvb ? "rmvb" : 
			(ape_priv->codec_tag == vp8 ? "vp8" : 
			(ape_priv->codec_tag == vc1 ? "vc1" : 
			(ape_priv->codec_tag == xvid ? "xvid" : "unknow"))))));
		APE_DBG_PRINT("\t\tfourcc %x\n", video_config_info.fourcc);
		APE_DBG_PRINT("\t\twidth %d\n", video_config_info.width);
		APE_DBG_PRINT("\t\theight %d\n", video_config_info.height);
		APE_DBG_PRINT("\t\tratio x %d\n", video_config_info.sample_aspect_ratio_num);
		APE_DBG_PRINT("\t\tratio y %d\n", video_config_info.sample_aspect_ratio_den);
		APE_DBG_PRINT("\t\tdec flag %x\n", video_config_info.decoder_flag);
		APE_DBG_PRINT("\t\ttime base num %d\n", video_config_info.time_base_num);
		APE_DBG_PRINT("\t\ttime base den %d\n", video_config_info.time_base_den);		
		APE_DBG_PRINT("\t\ttime scale num %d\n", video_config_info.timescale_num);
		APE_DBG_PRINT("\t\ttime scale den %d\n", video_config_info.timescale_den);	
	}
	else
	{
		APE_DBG_PRINT("\t\tvdec is idle\n");
	}


	APE_DBG_PRINT("\tadec info as below\n");
	if(ape_priv->a_init_flag)
	{
		memcpy((void *)&audio_config_info, (void *)&ape_priv->audio_info, sizeof(ape_priv->audio_info));
		
		APE_DBG_PRINT("\t\tid %x\n", audio_config_info.codec_id);
		APE_DBG_PRINT("\t\tbits per sample %d\n", audio_config_info.bits_per_coded_sample);
		APE_DBG_PRINT("\t\tsample rate %d\n", audio_config_info.sample_rate);
		APE_DBG_PRINT("\t\tchannel %d\n", audio_config_info.channels);
		APE_DBG_PRINT("\t\tbit rate %d\n", audio_config_info.bit_rate);
		APE_DBG_PRINT("\t\tframe size %d\n", audio_config_info.codec_frame_size);
		APE_DBG_PRINT("\t\tpcm buf addr %x size %x\n", ape_priv->pcm_buf_addr, ape_priv->pcm_buf_size);
		APE_DBG_PRINT("\t\textra buf addr %x size %x\n", audio_config_info.extradata, audio_config_info.extradata_size);	
		APE_DBG_PRINT("\t\ttime scale num %d\n", audio_config_info.timescale_num);
		APE_DBG_PRINT("\t\ttime scale den %d\n", audio_config_info.timescale_den);
	}
	else
	{
		APE_DBG_PRINT("\t\tadec is idle\n");
	}

	APE_DBG_PRINT("ali ape dbg info end**************************\n\n");
	
EXIT:	

	return;
}

static void ape_dbg_pause(int argc, char **argv)
{
	ape_dbg_enable(0);
	
	APE_DBG_PRINT("%s : done\n", __FUNCTION__);
	return;
}

static void ape_dbg_start(int argc, char **argv)
{	
	ape_dbg_enable(m_ape_dbg_level);
	
	APE_DBG_PRINT("%s : done\n", __FUNCTION__);
	return;
}

static void ape_dbg_help(int argc, char **argv)
{
	APE_DBG_PRINT("Module Name:\n");	
	APE_DBG_PRINT("\tali play engine diagnostic module.\n\n");	

	APE_DBG_PRINT("Synopsis:\n");	
	APE_DBG_PRINT("\tadrdbg ape -cmd [arguments] ...\n\n");
	
	APE_DBG_PRINT("Description:\n");
	
	APE_DBG_PRINT("\t-h, --help\n");
	APE_DBG_PRINT("\t\tself description of APE.\n\n");	

	APE_DBG_PRINT("\t-i, --info\n");
	APE_DBG_PRINT("\t\tshow the play engine info.\n\n");
	
	APE_DBG_PRINT("\t-s, --start\n");
	APE_DBG_PRINT("\t\tstart APE tracing.\n\n");

	APE_DBG_PRINT("\t-p, --pause\n");
	APE_DBG_PRINT("\t\tpause APE tracing.\n\n");

	APE_DBG_PRINT("\t-l, --level num\n");
	APE_DBG_PRINT("\t\tSet the tracing level of APE (default is 1).\n");
	APE_DBG_PRINT("\t\t0\t\t--no printout.\n");
	APE_DBG_PRINT("\t\t1\t\t--show Hld printout infomation.\n");	
	APE_DBG_PRINT("\t\t2\t\t--show Kernel printout infomation.\n");
	APE_DBG_PRINT("\t\t3\t\t--show Kernel & Hld printout infomation.\n");	
	APE_DBG_PRINT("\t\t4\t\t--show See printout infomation.\n");
	APE_DBG_PRINT("\t\t5\t\t--show Hld & See printout infomation.\n");
	APE_DBG_PRINT("\t\t6\t\t--show Kernel & See printout infomation.\n");	
	APE_DBG_PRINT("\t\t7\t\t--show All printout infomation.\n\n");	

	APE_DBG_PRINT("\t-d, --dump the path of the local file\n");
	APE_DBG_PRINT("\t\tdump the es data into the local file\n");

	APE_DBG_PRINT("\t-py, --play the path of the local file\n");
	APE_DBG_PRINT("\t\tplay back the local file or stop the playback(-py p)\n\n");
	
	APE_DBG_PRINT("Usage example:\n");
	APE_DBG_PRINT("\tape -h\n");
	APE_DBG_PRINT("\tape -s\n");
	APE_DBG_PRINT("\tape -p\n");
	APE_DBG_PRINT("\tape -i\n");
	APE_DBG_PRINT("\tape -l 6\n");	
	APE_DBG_PRINT("\tape -d /mnt/sda1/ali_ape_es_001.ts\n");	
	APE_DBG_PRINT("\tape -py /mnt/sda1/ali_ape_es_001.ts\n");		
	APE_DBG_PRINT("\tape -py p\n");	
	
	return;
}

static PARSE_COMMAND_LIST apedbg_command[] =
{
	{ { NULL, NULL }, ape_dbg_dump, NULL, "dump", "d", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ape_dbg_play, NULL, "play", "py", 0, 0, NULL, 0, 0, NULL, 0, 0 },		
	{ { NULL, NULL }, ape_dbg_level, NULL, "level", "l", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ape_dbg_info, soc_dbg_no_param_preview, "info", "i", 0, 0, NULL, 0, 0, NULL, 0, 0 },	
	{ { NULL, NULL }, ape_dbg_pause, soc_dbg_no_param_preview, "pause", "p", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ape_dbg_start, soc_dbg_no_param_preview, "start", "s", 0, 0, NULL, 0, 0, NULL, 0, 0 },
	{ { NULL, NULL }, ape_dbg_help, soc_dbg_no_param_preview, "help", "h", 0, 0, NULL, 0, 0, NULL, 0, 0 },
};

void apedbg_register(void)
{
	debug_module_add("ape", &apedbg_command[0], sizeof(apedbg_command) / sizeof(apedbg_command[0]));
}

INT32 ape_dbg_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	if (NULL == cmd_list || NULL == list_cnt)
		return RET_FAILURE;

	*cmd_list = &apedbg_command[0];
	*list_cnt = ARRAY_SIZE(apedbg_command);

	ape_open();
	
	return RET_SUCCESS;
}

INT32 ape_app_cmd_get(PARSE_COMMAND_LIST ** cmd_list, INT32 * list_cnt)
{
	return ape_dbg_cmd_get(cmd_list, list_cnt);
}

