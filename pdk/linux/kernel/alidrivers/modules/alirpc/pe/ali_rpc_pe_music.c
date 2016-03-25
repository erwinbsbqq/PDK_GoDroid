/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_pe_music.c
 *  (I)
 *  Description: pe music Remote Call Process API
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.28			Sam			Create
 ****************************************************************************/
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
//#include <linux/smp_lock.h>
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
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_pe.h>

#include "../ali_rpc.h"

static UINT32 desc_me_init[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pe_music_cfg)), 
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_me_set_eq[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(float)),  
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_me_get_song_info[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(MusicInfo)), 
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_me_get_decoder_info[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(DecoderInfo)), 
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,                          
  0,
};

int music_engine_init(struct pe_music_cfg *pe_music_cfg)
{


	jump_to_func(NULL, ali_rpc_call, pe_music_cfg, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(1<<16)|FUNC_MUSIC_ENGINE_INIT, desc_me_init);


}

void music_engine_release(void)
{
	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(0<<16)|FUNC_MUSIC_ENGINE_RELEASE, NULL);
}

int music_engine_play(char *filename)
{


	jump_to_func(NULL, ali_rpc_call, filename, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(1<<16)|FUNC_MUSIC_ENGINE_PLAY, NULL);


}

void music_engine_seek(int time)
{
	jump_to_func(NULL, ali_rpc_call, time, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(1<<16)|FUNC_MUSIC_ENGINE_SEEK, NULL);
}

void music_engine_stop(void)
{
	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(0<<16)|FUNC_MUSIC_ENGINE_STOP, NULL);
}

void music_engine_pause(void)
{
	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(0<<16)|FUNC_MUSIC_ENGINE_PAUSE, NULL);
}

int music_engine_get_time(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(0<<16)|FUNC_MUSIC_ENGINE_GET_TIME, NULL);


}

void music_engine_set_eq(int on, float preamp, float *bands)
{
	jump_to_func(NULL, ali_rpc_call, on, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(3<<16)|FUNC_MUSIC_ENGINE_SET_EQ, desc_me_set_eq);
}

int music_engine_get_song_info(char * filename, MusicInfo *music_info)
{


	jump_to_func(NULL, ali_rpc_call, filename, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(2<<16)|FUNC_MUSIC_ENGINE_GET_SONG_INFO, desc_me_get_song_info);


}

int music_engine_get_decoder_info(char * filename, DecoderInfo *decoder_info)
{


	jump_to_func(NULL, ali_rpc_call, filename, (LIB_PE_MUSIC_ENGINE_MODULE<<24)|(2<<16)|FUNC_MUSIC_ENGINE_GET_DECODER_INFO, desc_me_get_decoder_info);


}

