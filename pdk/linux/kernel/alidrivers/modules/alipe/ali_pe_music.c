/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_pe_music.c
 *  (I)
 *  Description: ali pe music engine for media player library
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
	if(down_interruptible(&m_music_sem)){ \
		PE_PRF("ali_pe down sem fail\n"); \
		return -1; \
	}

#define UNLOCK_MUTEX \
	up(&m_music_sem)

extern volatile unsigned long *g_ali_pe_rpc_arg[MAX_PE_RPC_ARG_NUM];
extern volatile int g_ali_pe_rpc_arg_size[MAX_PE_RPC_ARG_NUM];

static struct semaphore m_music_sem;

static int real_cb_enable = 0;
static int music_cb_pending = 0;
static unsigned long music_cb_par1 = 0;
static unsigned long music_cb_par2 = 0;
static int real_cb(unsigned long type, unsigned long param)
{
	LOCK_MUTEX;
	
	music_cb_pending = 1;
	music_cb_par1 = type;
	music_cb_par2 = param;
	
	UNLOCK_MUTEX;
}

static void mus_cb(unsigned long type, unsigned long param)
{
	if(real_cb_enable)
		real_cb(type, param);
}

int ali_pe_music_routine(struct ali_pe_info *info)
{
	LOCK_MUTEX;
	
	if(music_cb_pending){		
		music_cb_pending = 0;
		info->music.par1 = music_cb_par1;
		info->music.par2 = music_cb_par2;
		
		info->flag |= ALI_PE_CMD_FLAG_MUSIC;
	}
	
	UNLOCK_MUTEX;
	
	return 0;
}

int ali_pe_music_operation(int API_idx)
{
#define RPC_MUSIC_ENGINE_INIT					1
#define RPC_MUSIC_ENGINE_RELEASE				2
#define RPC_MUSIC_ENGINE_PLAY					3
#define RPC_MUSIC_ENGINE_SEEK					4
#define RPC_MUSIC_ENGINE_STOP					5
#define RPC_MUSIC_ENGINE_PAUSE				6
#define RPC_MUSIC_ENGINE_GET_TIME				7
#define RPC_MUSIC_ENGINE_SET_EQ				8
#define RPC_MUSIC_ENGINE_GET_SONG_INFO		9
#define RPC_MUSIC_ENGINE_GET_DECODER_INFO	10

	int ret = 0;

	if(API_idx != 7)
		PE_PRF("%s : API idx %d\n", __FUNCTION__, API_idx);
	switch(API_idx)
	{
		case RPC_MUSIC_ENGINE_INIT:
		{
#if 0			
#define __MM_TTX_BS_LEN				(0x5000)
#define __MM_SUB_BS_LEN				(0xA000)

extern unsigned long __G_MM_TTX_BS_START_ADDR;
extern unsigned long __G_MM_SUB_BS_START_ADDR;

			struct pe_music_cfg *cfg = (struct pe_music_cfg *)g_ali_pe_rpc_arg[0];

			cfg->pcm_out_buff = __G_MM_TTX_BS_START_ADDR & 0x8FFFFFFF;
			cfg->pcm_out_buff_size = __MM_TTX_BS_LEN;
			cfg->processed_pcm_buff = __G_MM_SUB_BS_START_ADDR & 0x8FFFFFFF;
			cfg->processed_pcm_buff_size = __MM_SUB_BS_LEN;			
			if(cfg->mp_cb != NULL)
				real_cb_enable = 1;
			else
				real_cb_enable = 0;				
			ret = music_engine_init((struct pe_music_cfg *)g_ali_pe_rpc_arg[0]);
#endif			
			break;
		}
		case RPC_MUSIC_ENGINE_RELEASE:
			music_engine_release();
			break;
		case RPC_MUSIC_ENGINE_PLAY:
			__CACHE_FLUSH_ALI((unsigned long)g_ali_pe_rpc_arg[0], g_ali_pe_rpc_arg_size[0]);
			ret = music_engine_play((char *)((unsigned long)g_ali_pe_rpc_arg[0] | 0xA0000000));
			break;
		case RPC_MUSIC_ENGINE_SEEK:
			music_engine_seek(*(int *)g_ali_pe_rpc_arg[0]);
			break;
		case RPC_MUSIC_ENGINE_STOP:
			music_engine_stop();
			break;
		case RPC_MUSIC_ENGINE_PAUSE:
			music_engine_pause();
			break;
		case RPC_MUSIC_ENGINE_GET_TIME:
			ret = music_engine_get_time();
			break;
		case RPC_MUSIC_ENGINE_SET_EQ:
#if 0			
			ret = music_engine_set_eq(*(int *)g_ali_pe_rpc_arg[0]
				, *(float *)g_ali_pe_rpc_arg[1], (float *)g_ali_pe_rpc_arg[2]);
#else
			ret = -1;
#endif
			break;			
		case RPC_MUSIC_ENGINE_GET_SONG_INFO:
			__CACHE_FLUSH_ALI((unsigned long)g_ali_pe_rpc_arg[0], g_ali_pe_rpc_arg_size[0]);			
			ret = music_engine_get_song_info((char *)((unsigned long)g_ali_pe_rpc_arg[0] | 0xA0000000)
				, (MusicInfo *)g_ali_pe_rpc_arg[1]);
			break;
		case RPC_MUSIC_ENGINE_GET_DECODER_INFO:
			ret = music_engine_get_decoder_info(*(char **)g_ali_pe_rpc_arg[0]
				, (DecoderInfo *)g_ali_pe_rpc_arg[1]);
			break;				
		default:
			ret = -1;
			break;
	}
	
	return ret;
}

void ali_pe_music_init(struct ali_pe_info *info)
{
	sema_init(&m_music_sem, 1);
	ali_rpc_register_callback(ALI_RPC_CB_MUS, mus_cb);		
}

