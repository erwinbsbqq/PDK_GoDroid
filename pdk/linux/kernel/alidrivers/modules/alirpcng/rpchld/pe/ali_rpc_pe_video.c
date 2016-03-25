/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_pe_video.c
 *  (I)
 *  Description: pe video Remote Call Process API
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
int video_engine_init(struct pe_video_cfg *pe_video_cfg)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Pe_video_cfg_rpc, sizeof(Pe_video_cfg_rpc), pe_video_cfg);

    ret = RpcCallCompletion(RPC_video_engine_init, &p1, NULL);
    return ret;
}

int video_engine_pe_cache_init(pe_cache_ex *pe_cache_info, pe_cache_cmd *pe_cache_cmd_buf, int pe_cache_mutex)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Pe_cache_ex_rpc, sizeof(Pe_cache_ex_rpc), pe_cache_info);

    Pe_cache_cmd_rpc pe_cache_cmd_rpc_struct;
    Pe_cache_cmd_rpc *temp = &pe_cache_cmd_rpc_struct;

    temp->status = pe_cache_cmd_buf->status;
    temp->type = pe_cache_cmd_buf->type;
    temp->reserved = pe_cache_cmd_buf->reserved;
    
    Int32 i = 0;
    for(i = 0; i<4; i++)
    {
        temp->param[i] = pe_cache_cmd_buf->param[i];
    }
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Pe_cache_cmd_rpc, sizeof(Pe_cache_cmd_rpc), temp);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_INT32, sizeof(Int32), &pe_cache_mutex);

    ret = RpcCallCompletion(RPC_video_engine_pe_cache_init, &p1, &p2, &p3, NULL);
    return ret;
}

int video_engine_pe_cache_release(void)
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_video_engine_pe_cache_release, NULL);
    return ret;
}

int video_engine_decode(char *video_file, UINT8 video_stream_type, enum SndDupChannel audio_channel, BOOL preview)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_OPAQUE, strlen(video_file), video_file);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &video_stream_type);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_ENUM, sizeof(enum SndDupChannel), &audio_channel);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_INT32, sizeof(Uint32), &preview);

    ret = RpcCallCompletion(RPC_video_engine_decode, &p1, &p2, &p3, &p4, NULL);
    return ret;
}

int video_engine_set_output(enum VDecOutputMode eOutMode, struct VDecPIPInfo *pInitInfo)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_ENUM, sizeof(enum VDecOutputMode), &eOutMode);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_VDecPIPInfo_rpc, sizeof(Vdecpipinfo_rpc), pInitInfo);

    ret = RpcCallCompletion(RPC_video_engine_set_output, &p1, &p2, NULL);
    return ret;
}

DWORD mpg_cmd_play_proc(void)
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_mpg_cmd_play_proc, NULL);
    return ret;
}

DWORD mpg_cmd_pause_proc(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_mpg_cmd_pause_proc, NULL);
    return ret;
}

DWORD mpg_cmd_resume_proc(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_mpg_cmd_resume_proc, NULL);
    return ret;
}

DWORD mpg_cmd_ff_proc(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_mpg_cmd_ff_proc, NULL);
    return ret;
}

DWORD mpg_cmd_fb_proc(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_mpg_cmd_fb_proc, NULL);
    return ret;
}

DWORD mpg_cmd_slow_proc(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_mpg_cmd_slow_proc, NULL);
    return ret;
}

DWORD mpg_cmd_stop_proc(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_mpg_cmd_stop_proc, NULL);
    return ret;

}

DWORD mpg_cmd_resume_stop_proc(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_mpg_cmd_resume_stop_proc, NULL);
    return ret;


}
DWORD mpg_cmd_search_proc(DWORD search_time)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &search_time);
    
    ret = RpcCallCompletion(RPC_mpg_cmd_search_proc, &p1, NULL);
    return ret;

}

DWORD mpg_cmd_search_ms_proc(DWORD search_ms_time)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &search_ms_time);
    
    ret = RpcCallCompletion(RPC_mpg_cmd_search_ms_proc, &p1, NULL);
    return ret;
}

DWORD mpg_cmd_change_audio_track(INT32 *aud_pid)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_INT32, 4, aud_pid);
    
    ret = RpcCallCompletion(RPC_mpg_cmd_change_audio_track, &p1, NULL);
    return ret;
}

DWORD mpg_cmd_change_subtitle(INT32 sub_pid)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, 4, &sub_pid);
    
    ret = RpcCallCompletion(RPC_mpg_cmd_change_subtitle, &p1, NULL);
    return ret;
}
DWORD mpg_cmd_change_prog(int prog_id)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, 4, &prog_id);
    
    ret = RpcCallCompletion(RPC_mpg_cmd_change_prog, &p1, NULL);
    return ret;
}

DWORD MpgFileGetChapterInfo(PDEC_CHAPTER_INFO pDecChapterInfo)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_DEC_CHAPTER_INFO_RPC, sizeof(DEC_CHAPTER_INFO_RPC), pDecChapterInfo);
    
    ret = RpcCallCompletion(RPC_MpgFileGetChapterInfo, &p1, NULL);
    return ret;
}

DWORD MpgFileGetStreamInfo(p_fileinfo_video pDecStreamInfo)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_Fileinfo_video_rpc, sizeof(Fileinfo_video_rpc), pDecStreamInfo);
    
    ret = RpcCallCompletion(RPC_MpgFileGetStreamInfo, &p1, NULL);
    return ret;
}

DWORD MPGFileDecoderGetPlayTime(void)
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_MPGFileDecoderGetPlayTime, NULL);
    return ret;
}

DWORD MPGFileDecoderGetPlayTimeMS(void)
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_MPGFileDecoderGetPlayTimeMS, NULL);
    return ret;
}

int MPGGetTotalPlayTime(void)
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_MPGGetTotalPlayTime, NULL);
    return ret;
}

DWORD MPGGetTotalPlayTimeMS(void)
{
    Int32 ret;
    ret = RpcCallCompletion(RPC_MPGGetTotalPlayTimeMS, NULL);
    return ret;
}

DWORD MpgSetAVSyncDelay(UINT32 stream_type, INT32 time_ms)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, 4, &stream_type);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(Int32), &time_ms);

    ret = RpcCallCompletion(RPC_MpgSetAVSyncDelay, &p1, &p2, NULL);
    return ret;
}

RET_CODE video_decoder_ioctl(unsigned long io_cmd, unsigned long param1, unsigned long param2)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_ULONG, sizeof(Ulong), &io_cmd);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_ULONG, sizeof(Ulong), &param1);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_ULONG, sizeof(Ulong), &param2);

    
    switch(io_cmd)
    {
        case VDEC_IO_SET_SCALE_MODE:
            RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_ULONG, sizeof(Ulong), &param1);
            RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_VDecPIPInfo_rpc, sizeof(Vdecpipinfo_rpc), param2);
            break;
        case CONTAINER_IO_CONTROL_GET_TIME_MS:
            RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_ULONG, sizeof(Ulong), param1);
            break;
        case VDEC_IO_GET_STATUS:
            RPC_PARAM_UPDATE(p2, PARAM_OUT, PARAM_VDec_StatusInfo_rpc, sizeof(Vdec_statusinfo_rpc), param1);
            break;
        case VDEC_IO_GET_MODE:
            RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_ULONG, sizeof(Ulong), param1);
            break;
        case CONTAINER_IO_CONTROL_GET_CHAPTER_INFO:
            RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_DEC_CHAPTER_INFO_RPC, sizeof(DEC_CHAPTER_INFO_RPC), param1);
            break;
        case CONTAINER_IO_CONTROL_SET_AVSYNC_DELAY:
            break;

        case CONTAINER_IO_CONTROL_EN_AC3_BS_MODE:
        default:
            break;
    }

    ret = RpcCallCompletion(RPC_video_decoder_ioctl, &p1, &p2, &p3, NULL);

    return ret;
}

AF_PE_PLAY_STATE get_next_play_state(void)
{
    AF_PE_PLAY_STATE ret;

    ret = RpcCallCompletion(RPC_get_next_play_state, NULL);
    return ret;
}

