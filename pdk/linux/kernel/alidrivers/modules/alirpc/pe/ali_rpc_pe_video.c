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

static UINT32 desc_ve_init[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pe_video_cfg)), 
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ve_set_output[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct VDecPIPInfo)), 
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ve_p_int32[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, 4),  
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ve_get_stream_info[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(fileinfo_video)),  
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ve_get_chapter_info[] =
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(DEC_CHAPTER_INFO)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,
  0,
};

UINT32 desc_vdec_ioctl[] =
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, 0), DESC_STATIC_STRU(1, 0),
  2, DESC_P_PARA(0, 1, 0), DESC_P_PARA(0, 2, 1),
  //desc of pointer ret
  0,
  0,
};
int video_engine_init(struct pe_video_cfg *pe_video_cfg)
{


	jump_to_func(NULL, ali_rpc_call, pe_video_cfg, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(1<<16)|FUNC_VIDEO_ENGINE_INIT, desc_ve_init);


}

int video_engine_pe_cache_init(pe_cache_ex *pe_cache_info, pe_cache_cmd *pe_cache_cmd_buf, int pe_cache_mutex)
{


	jump_to_func(NULL, ali_rpc_call, pe_cache_info, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(3<<16)|FUNC_VIDEO_ENGINE_PE_CACHE_INIT, NULL);


}

int video_engine_pe_cache_release(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_PE_CACHE_RELEASE, NULL);


}

int video_engine_decode(char *video_file, UINT8 video_stream_type, enum SndDupChannel audio_channel, BOOL preview)
{


	jump_to_func(NULL, ali_rpc_call, video_file, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(4<<16)|FUNC_VIDEO_ENGINE_DECODE, NULL);


}

int video_engine_set_output(enum VDecOutputMode eOutMode, struct VDecPIPInfo *pInitInfo)
{
	

	jump_to_func(NULL, ali_rpc_call, eOutMode, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(2<<16)|FUNC_VIDEO_ENGINE_SET_OUTPUT, desc_ve_set_output);


}

DWORD mpg_cmd_play_proc(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_CMD_PLAY, NULL);


}

DWORD mpg_cmd_pause_proc(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_CMD_PAUSE, NULL);


}

DWORD mpg_cmd_resume_proc(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_CMD_RESUME, NULL);


}

DWORD mpg_cmd_ff_proc(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_CMD_FF, NULL);


}

DWORD mpg_cmd_fb_proc(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_CMD_FB, NULL);


}

DWORD mpg_cmd_slow_proc(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_CMD_SLOW, NULL);


}

DWORD mpg_cmd_stop_proc(void)
{


	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_CMD_STOP, NULL);

}

DWORD mpg_cmd_resume_stop_proc(void)
{

    
    jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_CMD_RESUME_STOP, NULL);


}
DWORD mpg_cmd_search_proc(DWORD search_time)
{
	

	jump_to_func(NULL, ali_rpc_call, search_time, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(1<<16)|FUNC_VIDEO_ENGINE_CMD_SEARCH, NULL);
	
}

DWORD mpg_cmd_search_ms_proc(DWORD search_ms_time)
{
    
    
    jump_to_func(NULL, ali_rpc_call, search_ms_time, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(1<<16)|FUNC_VIDEO_ENGINE_CMD_SEARCH_MS, NULL);

	
}

DWORD mpg_cmd_change_audio_track(INT32 *aud_pid)
{
	

	jump_to_func(NULL, ali_rpc_call, aud_pid, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(1<<16)|FUNC_VIDEO_ENGINE_CMD_CHANGE_AUDIO_TRACK, desc_ve_p_int32);

	
}

DWORD mpg_cmd_change_subtitle(INT32 sub_pid)
{
    
    
    jump_to_func(NULL, ali_rpc_call, sub_pid, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(1<<16)|FUNC_VIDEO_ENGINE_CMD_CHANGE_SUBTITLE, NULL);

    
}
DWORD mpg_cmd_change_prog(int prog_id)
{
	

	jump_to_func(NULL, ali_rpc_call, prog_id, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(1<<16)|FUNC_VIDEO_ENGINE_CMD_CHANGE_PROG, NULL);
	
}

DWORD MpgFileGetChapterInfo(PDEC_CHAPTER_INFO pDecChapterInfo)
{
    
    
    jump_to_func(NULL, ali_rpc_call, pDecChapterInfo, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(1<<16)|FUNC_VIDEO_ENGINE_GET_CHAPTER_INFO, desc_ve_get_chapter_info);

	
}

DWORD MpgFileGetStreamInfo(p_fileinfo_video pDecStreamInfo)
{
	

	jump_to_func(NULL, ali_rpc_call, pDecStreamInfo, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(1<<16)|FUNC_VIDEO_ENGINE_GET_STREAM_INFO, desc_ve_get_stream_info);

	
}

DWORD MPGFileDecoderGetPlayTime(void)
{
	

	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_GET_PLAY_TIME, NULL);

	
}

DWORD MPGFileDecoderGetPlayTimeMS(void)
{
		
		
    jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_GET_PLAY_TIMEMS, NULL);

		
}

int MPGGetTotalPlayTime(void)
{
	

	jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_GET_TOTAL_PLAY_TIME, NULL);

	
}

DWORD MPGGetTotalPlayTimeMS(void)
{
    
    
    jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_GET_TOTAL_PLAY_TIMEMS, NULL);

    
}

DWORD MpgSetAVSyncDelay(UINT32 stream_type, INT32 time_ms)
{
    
    
    jump_to_func(NULL, ali_rpc_call, stream_type, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(2<<16)|FUNC_VIDEO_ENGINE_SET_AVSYNC_DELAY, NULL);

    
}

UINT32 desc_video_get_status[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct VDec_StatusInfo)),
  1, DESC_P_PARA(0, 1, 0), 
  //desc of pointer ret
  0,                          
  0,
};

RET_CODE video_decoder_ioctl(unsigned long io_cmd, unsigned long param1, unsigned long param2)
{
    UINT32 i;
    UINT32 common_desc[sizeof(desc_vdec_ioctl)];
    UINT32 *desc = (UINT32 *)common_desc;
    UINT32 *b = (UINT32 *)desc_vdec_ioctl;
	
    memcpy(common_desc, desc_vdec_ioctl, sizeof(desc_vdec_ioctl));
    for(i = 0; i < sizeof(desc_vdec_ioctl)/sizeof(UINT32); i++)
    {
        desc[i] = b[i];
    }

    switch(io_cmd)
    {
        case VDEC_IO_SET_SCALE_MODE:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, 0);
            DESC_STATIC_STRU_SET_SIZE(common_desc, 1, sizeof(struct VDecPIPInfo));
            break;
        case CONTAINER_IO_CONTROL_GET_TIME_MS:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(unsigned long));
            break;
        case VDEC_IO_GET_STATUS:
            DESC_OUTPUT_STRU_SET_SIZE(common_desc, 0, sizeof(struct  VDec_StatusInfo));
            break;
        case VDEC_IO_GET_MODE:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, 4);
            break;
        case CONTAINER_IO_CONTROL_GET_CHAPTER_INFO:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, sizeof(DEC_CHAPTER_INFO));
            break;
        case CONTAINER_IO_CONTROL_SET_AVSYNC_DELAY:
            DESC_STATIC_STRU_SET_SIZE(common_desc, 0, 0);
            DESC_STATIC_STRU_SET_SIZE(common_desc, 1, 0);
            break;

        case CONTAINER_IO_CONTROL_EN_AC3_BS_MODE:
        default:
            desc = NULL;
            break;
    }

    jump_to_func(NULL, ali_rpc_call, io_cmd, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(3<<16)|FUNC_VIDEO_ENGINE_DECODER_CONTROL, desc);
}

AF_PE_PLAY_STATE get_next_play_state(void)
{
    
    
    jump_to_func(NULL, ali_rpc_call, null, (LIB_PE_ADV_VIDEO_ENGINE_MODULE<<24)|(0<<16)|FUNC_VIDEO_ENGINE_GET_NEXT_STATE, NULL);

    
}

