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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
#include <linux/smp_lock.h>
#endif
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

//#include "../ali_rpc.h"

#include <ali_rpcng.h>

int music_engine_init(struct pe_music_cfg *pe_music_cfg)
{
    Int32 ret;
    struct pe_music_cfg_rpc pe_music_cfg_rpc_struct;
    struct pe_music_cfg_rpc *temp1 = &pe_music_cfg_rpc_struct;

    temp1->pcm_out_buff = pe_music_cfg->pcm_out_buff;
    temp1->pcm_out_buff_size = pe_music_cfg->pcm_out_buff_size;
    temp1->processed_pcm_buff = pe_music_cfg->processed_pcm_buff;
    temp1->processed_pcm_buff_size = pe_music_cfg->processed_pcm_buff_size;
    temp1->mp_cb = pe_music_cfg->mp_cb;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Pe_music_cfg_rpc, sizeof(struct pe_music_cfg_rpc), temp1);

    ret = RpcCallCompletion(RPC_music_engine_init, &p1, NULL);
    return ret;
}

void music_engine_release(void)
{
    RpcCallCompletion(RPC_music_engine_release, NULL);
}

int music_engine_play(char *filename)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_OPAQUE, strlen(filename), filename);

    ret = RpcCallCompletion(RPC_music_engine_play, &p1, NULL);
    return ret;
}

void music_engine_seek(int time)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, sizeof(Int32), &time);    

    RpcCallCompletion(RPC_music_engine_seek, &p1, NULL);
}

void music_engine_stop(void)
{
     RpcCallCompletion(RPC_music_engine_stop, NULL);
}

void music_engine_pause(void)
{
     RpcCallCompletion(RPC_music_engine_pause, NULL);
}

int music_engine_get_time(void)
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_music_engine_get_time, NULL);
    return ret;
}

void music_engine_set_eq(int on, float preamp, float *bands)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, sizeof(Int32), &on);    
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_FLOAT, sizeof(Float), &preamp);    
    RPC_PARAM_CREATE(p3, PARAM_OUT, PARAM_FLOAT, sizeof(Float), bands);    

    RpcCallCompletion(RPC_music_engine_set_eq, &p1, &p2, &p3, NULL);
}

int music_engine_get_song_info(char * filename, MusicInfo *music_info)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_OPAQUE, strlen(filename), filename);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_MusicInfo_rpc, sizeof(MusicInfo_rpc), music_info);

    ret =  RpcCallCompletion(RPC_music_engine_get_song_info, &p1, &p2, NULL);
    return ret;
}

int music_engine_get_decoder_info(char * filename, DecoderInfo *decoder_info)
{
    Int32 ret;
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_OPAQUE, strlen(filename), filename);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_DecoderInfo_rpc, sizeof(DecoderInfo_rpc), decoder_info);
    ret =  RpcCallCompletion(RPC_music_engine_get_decoder_info, &p1, &p2, NULL);
    return ret;
}

