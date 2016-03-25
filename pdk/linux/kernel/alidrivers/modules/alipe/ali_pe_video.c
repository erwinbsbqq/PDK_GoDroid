/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_pe_video.c
 *  (I)
 *  Description: ali pe video engine for media player library
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.07.05			Sam			Create
 ****************************************************************************/
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vt.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/fb.h>
#include <ali_pe_common.h>
#include <rpc_hld/ali_rpc_pe.h>
#include <ali_cache.h>
#include "ali_pe.h"

#define LOCK_MUTEX	\
	if(down_interruptible(&m_video_sem)){ \
		PE_PRF("ali_pe down sem fail\n"); \
		return -1; \
	}

#define UNLOCK_MUTEX \
	up(&m_video_sem)
	
extern volatile unsigned long *g_ali_pe_rpc_arg[MAX_PE_RPC_ARG_NUM];
extern volatile int g_ali_pe_rpc_arg_size[MAX_PE_RPC_ARG_NUM];

static struct semaphore m_video_sem;

static int real_cb_enable = 0;
static int video_cb_pending = 0;
static unsigned long video_cb_par1 = 0;
static unsigned long video_cb_par2 = 0;

static int real_cb(unsigned long type, unsigned long param)
{
	LOCK_MUTEX;
	
	video_cb_pending = 1;
	video_cb_par1 = type;
	video_cb_par2 = param;
	
	UNLOCK_MUTEX;
}

static void vde_cb(unsigned long type, unsigned long param)
{
	if(real_cb_enable)
		real_cb(type, param);
}	

int ali_pe_video_routine(struct ali_pe_info *info)
{
	LOCK_MUTEX;
	
	if(video_cb_pending){		
		video_cb_pending = 0;
		info->video.par1 = video_cb_par1;
		info->video.par2 = video_cb_par2;
		
		info->flag |= ALI_PE_CMD_FLAG_VIDEO;
	}
	
	UNLOCK_MUTEX;
	return 0;
}

int ali_pe_video_operation(int API_idx)
{
#define RPC_VIDEO_ENGINE_INIT					1
#define RPC_VIDEO_ENGINE_DECODE				2
#define RPC_VIDEO_ENGINE_SET_OUT				3
#define RPC_VIDEO_ENGINE_PLAY					4
#define RPC_VIDEO_ENGINE_PAUSE				5
#define RPC_VIDEO_ENGINE_RESUME				6
#define RPC_VIDEO_ENGINE_FF					7
#define RPC_VIDEO_ENGINE_FB					8
#define RPC_VIDEO_ENGINE_SLOW					9
#define RPC_VIDEO_ENGINE_STOP					10
#define RPC_VIDEO_ENGINE_SEARCH				11
#define RPC_VIDEO_ENGINE_CHANGE_AUDIO_TRACK 12
#define RPC_VIDEO_ENGINE_CHANGE_PROG			13
#define RPC_VIDEO_ENGINE_GET_STREAM_INFO	14
#define RPC_VIDEO_ENGINE_GET_PLAY_TIME		15
#define RPC_VIDEO_ENGINE_GET_TOTAL_TIME		16
#define RPC_VIDEO_ENGINE_RELEASE_CACHE		17
#define RPC_VIDEO_ENGINE_CMD_SEARCH_MS     18
#define RPC_VIDEO_ENGINE_CMD_RESUME_STOP   19
#define RPC_VIDEO_ENGINE_CMD_CHANGE_SUBTITLE  20
#define RPC_VIDEO_ENGINE_GET_CHAPTER_INFO     21
#define RPC_VIDEO_ENGINE_DECODER_CONTROL      22
#define RPC_VIDEO_ENGINE_GET_NEXT_STATE       23
#define RPC_VIDEO_ENGINE_GET_TOTAL_PLAY_TIMEMS  24
#define RPC_VIDEO_ENGINE_GET_PLAY_TIMEMS      25
#define RPC_VIDEO_ENGINE_SET_AVSYNC_DELAY     26

	int ret = 0;

	if(API_idx != RPC_VIDEO_ENGINE_GET_PLAY_TIME)
		PE_PRF("%s : API idx %d\n", __FUNCTION__, API_idx);
	switch(API_idx)
	{
		case RPC_VIDEO_ENGINE_INIT:
		{
extern unsigned long g_ali_pe_buf_size;
extern unsigned long g_ali_pe_buf_start_addr;

			struct pe_video_cfg *cfg = (struct pe_video_cfg *)g_ali_pe_rpc_arg[0];
			
			cfg->decoder_buf = g_ali_pe_buf_start_addr;
			cfg->decoder_buf_len = g_ali_pe_buf_size;
			if(cfg->mp_cb != NULL)
				real_cb_enable = 1;
			else
				real_cb_enable = 0;
			
			if(cfg->reserved)
			{
				void *buf = kmalloc(sizeof(struct pe_video_cfg_extra), GFP_KERNEL);

				copy_from_user(buf, (void *)cfg->reserved, sizeof(struct pe_video_cfg_extra));

				cfg->reserved = (UINT32)buf;
				__CACHE_FLUSH_ALI((unsigned long)cfg->reserved, sizeof(struct pe_video_cfg_extra));				
				cfg->reserved |= 0xA0000000;
			}
			
			ret = video_engine_init((struct pe_video_cfg *)g_ali_pe_rpc_arg[0]);
			break;
		}
		case RPC_VIDEO_ENGINE_RELEASE_CACHE:
			video_engine_pe_cache_release();
			break;
		case RPC_VIDEO_ENGINE_DECODE:
			__CACHE_FLUSH_ALI((unsigned long)g_ali_pe_rpc_arg[0], g_ali_pe_rpc_arg_size[0]);
			ret = video_engine_decode((char *)((unsigned long)g_ali_pe_rpc_arg[0] | 0xA0000000)
				, *(UINT8 *)g_ali_pe_rpc_arg[1], *(enum SndDupChannel *)g_ali_pe_rpc_arg[2]
				, *(BOOL *)g_ali_pe_rpc_arg[3]);
			break;			
		case RPC_VIDEO_ENGINE_SET_OUT:
			ret = video_engine_set_output(*(enum VDecOutputMode *)g_ali_pe_rpc_arg[0]
				, (struct VDecPIPInfo *)g_ali_pe_rpc_arg[1]);
			break;
		case RPC_VIDEO_ENGINE_PLAY:
			ret = mpg_cmd_play_proc();
			break;			
		case RPC_VIDEO_ENGINE_PAUSE:
			ret = mpg_cmd_pause_proc();
			break;			
		case RPC_VIDEO_ENGINE_RESUME:
			ret = mpg_cmd_resume_proc();
			break;			
		case RPC_VIDEO_ENGINE_FF:
			ret = mpg_cmd_ff_proc();
			break;			
		case RPC_VIDEO_ENGINE_FB:
			ret = mpg_cmd_fb_proc();
			break;			
		case RPC_VIDEO_ENGINE_SLOW:
			ret = mpg_cmd_slow_proc();
			break;			
		case RPC_VIDEO_ENGINE_STOP:
			ret = mpg_cmd_stop_proc();
			break;			
		case RPC_VIDEO_ENGINE_SEARCH:
			ret = mpg_cmd_search_proc(*(DWORD *)g_ali_pe_rpc_arg[0]);
			break;		
		case RPC_VIDEO_ENGINE_CHANGE_AUDIO_TRACK:
			ret = mpg_cmd_change_audio_track((INT32 *)g_ali_pe_rpc_arg[0]);
			break;			
		case RPC_VIDEO_ENGINE_CHANGE_PROG:
			ret = mpg_cmd_change_prog(*(int *)g_ali_pe_rpc_arg[0]);
			break;			
		case RPC_VIDEO_ENGINE_GET_STREAM_INFO:
			ret = MpgFileGetStreamInfo((p_fileinfo_video)g_ali_pe_rpc_arg[0]);
			break;			
		case RPC_VIDEO_ENGINE_GET_PLAY_TIME:
			ret = MPGFileDecoderGetPlayTime();
			break;				
		case RPC_VIDEO_ENGINE_GET_TOTAL_TIME:
			ret = MPGGetTotalPlayTime();
			break;
	    case RPC_VIDEO_ENGINE_CMD_SEARCH_MS:
			ret = mpg_cmd_search_ms_proc(*(DWORD *)g_ali_pe_rpc_arg[0]);
			break;	
		case RPC_VIDEO_ENGINE_CMD_RESUME_STOP:
			ret = mpg_cmd_resume_stop_proc();
			break;	
		case RPC_VIDEO_ENGINE_CMD_CHANGE_SUBTITLE:
			ret = mpg_cmd_change_subtitle((INT32 *)g_ali_pe_rpc_arg[0]);
			break;	
		case RPC_VIDEO_ENGINE_GET_CHAPTER_INFO:
			ret = MpgFileGetChapterInfo((PDEC_CHAPTER_INFO)g_ali_pe_rpc_arg[0]);;
			break;	
		case RPC_VIDEO_ENGINE_DECODER_CONTROL:
			ret = video_decoder_ioctl(*(unsigned long*)g_ali_pe_rpc_arg[0], *(unsigned long *)g_ali_pe_rpc_arg[1],
				 *(unsigned long *)g_ali_pe_rpc_arg[2]);
			break;	
		case RPC_VIDEO_ENGINE_GET_NEXT_STATE:
			ret = get_next_play_state();
			break;	
		case RPC_VIDEO_ENGINE_GET_TOTAL_PLAY_TIMEMS:
			ret = MPGGetTotalPlayTimeMS();
			break;	
		case RPC_VIDEO_ENGINE_GET_PLAY_TIMEMS:
			ret = MPGFileDecoderGetPlayTimeMS();
			break;	
		case RPC_VIDEO_ENGINE_SET_AVSYNC_DELAY:
			ret = MpgSetAVSyncDelay(*(UINT32 *)g_ali_pe_rpc_arg[0], *(INT32 *)g_ali_pe_rpc_arg[1]);
			break;	
		default:
			ret = -1;
			break;		
	}

	return ret;
}

void ali_pe_video_init(struct ali_pe_info *info)
{
	sema_init(&m_video_sem, 1);

	ali_rpc_register_callback(ALI_RPC_CB_VDE, vde_cb);	
}

