#include <linux/version.h>

#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 35)
#include <linux/slab.h> 
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 

#include <linux/fs.h>
#include <linux/types.h>

#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/delay.h>

#include <asm/uaccess.h>
#include <linux/slab.h>
#include "../../include/ali_cache.h"
#include <linux/sched.h> 

#include <asm/io.h>
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld_deca.h>
#include <rpc_hld/ali_rpc_hld_decv.h>
#include <rpc_hld/ali_rpc_hld_snd.h>

//#include <linux/ali_basic.h>

#include <ali_dmx_common.h>
#include <ali_dsc_common.h>

#include "dmx_stack.h"

#include "dmx_see_interface.h"
#if defined(CONFIG_ALI_RPCNG)
#include <ali_rpcng.h>
#endif
MODULE_LICENSE("Dual BSD/GPL");


extern unsigned long __G_ALI_MM_DMX_MEM_SIZE;
extern unsigned long __G_ALI_MM_DMX_MEM_START_ADDR;

struct class *ali_dmx_see_interface_class;




#define dmx_offsetof(type, f) ((unsigned long) \
((char *)&((type *)0)->f - (char *)(type *)0))


#define AUDIO_TYPE_TEST(pid, type)      (((pid)&0xE000)==(type))


#define AC3_DES_EXIST                   (1<<13)//0x2000
#define AAC_DES_EXIST                   (2<<13)//0x4000//LATM_AAC
#define EAC3_DES_EXIST                  (3<<13)//0x0001//EAC3
#define ADTS_AAC_DES_EXIST              (4<<13)//0x8000//ADTS_AAC



/* Must keep compatible with TDS array: UINT32 lld_dmx_m36_t_entry[]. */
enum LLD_DMX_M36F_FUNC{
    FUNC_SED_PLAY_CHANNEL = 0,  
    FUNC_SED_REG_SERV,
    FUNC_SED_M36_IS_AV_SCRAMBLED,
    FUNC_SED_SET_DEV,
    FUNC_SED_ADJUST_STC,
    FUNC_SED_SUMMIT_SPEC,
    FUNC_SED_STC_MONITOR,
    FUNC_SED_SET_PLAYBACK_SPEED,
    FUNC_SED_SET_PARSE_INFO,
    FUNC_SED_PARSE_PES,
    FUNC_SED_UPDATE_REMAIN_DATA,
    FUNC_SED_TSG_ADJUST_PCR,
    FUNC_SED_SET_AV_MODE,
    FUNC_SED_RESET_TO_LIVE_MODE,
    FUNC_SED_SET_DECRYPT_STATUS,
    FUNC_SED_ENABLE_VIDEO_DMA,
    FUNC_SED_SET_PARSE_STATUS,
    FUNC_SED_SET_PCR_PID,
    FUNC_SED_GET_DISCNT,
    FUNC_SED_CLEAR_DISCNT,
    FUNC_SED_AUDIO_CHANGE,
    FUNC_SED_M36_IS_AV_SCRAMBLED_OLD,
    FUNC_SED_IS_PROGRAM_LEGAL,
    FUNC_SED_RESET_PES_PARAM,
    FUNC_SED_SET_HW_PCR_BASE,
    FUNC_SED_SET_DDP_CERTIFICATION,
    FUNC_SED_M36_IS_AV_SOURCE_SCRAMBLED_OLD,
    FUNC_SED_SET_DROP_ERROR_PES,
    FUNC_SED_GET_DEV,
    FUNC_SED_DMX_ATTACH,
 //   FUNC_SED_STANDBY, // -> move to hld_base

    /* Added for linux A/V API.
     * Date:2012.02.23 by Joy.
     */
    FUNC_SED_DMX_VIDEO_STREAM_START,
    FUNC_SED_DMX_VIDEO_STREAM_STOP,
    FUNC_SED_DMX_AUDIO_STREAM_START,
    FUNC_SED_DMX_AUDIO_STREAM_STOP,

    /* Added for linux get descrambled TS packet back to main CPU.
     * Date:2012.10.28 by Joy.
     */
    FUNC_SED_DMX_SEE2MAIN_BUF_INFO_SET,
    FUNC_SED_DMX_SEE2MAIN_PID_ADD,
    FUNC_SED_DMX_SEE2MAIN_PID_DEL,	
};



__s32 dmx_see_open(struct inode *inode, struct file *file);

__s32 dmx_see_release(struct inode * inode, struct file * file);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_see_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
#else
__s32 dmx_see_ioctl(struct inode * inode, struct file * filp, unsigned int cmd, unsigned long arg);
#endif



struct file_operations g_ali_m36_dmx_see_fops = {
    .owner =    THIS_MODULE,
    //.llseek =   scull_llseek,
    //.read =     dmx_channel_read,
    //.write =    dmx_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = dmx_see_ioctl,
#else
    .ioctl =    dmx_see_ioctl,
#endif    
    .open =     dmx_see_open,
    .release =  dmx_see_release,
    //.poll = dmx_channel_poll,
};


struct dmx_see_device ali_dmx_see_dev[1];
__s32 g_see_crypto_started = 0;

#if !defined(CONFIG_ALI_RPCNG)
__u32 desc_sed_play_channel[] = 
{ //desc of pointer para
  3, DESC_STATIC_STRU(0, sizeof(struct pes_pkt_param)), DESC_STATIC_STRU(1, sizeof(struct pes_pkt_param)),DESC_STATIC_STRU(2, sizeof(struct pes_pkt_param)),
  3, DESC_P_PARA(0, 3, 0), DESC_P_PARA(1, 4, 1), DESC_P_PARA(2, 5, 2),
  //desc of pointer ret
  0,                          
  0,
};


__u32 desc_sed_reg_serv[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pes_pkt_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};
__u32 desc_sed_m36_is_av_scrambled[] = 
{ //desc of pointer para
  3, DESC_STATIC_STRU(0, sizeof(struct io_param)), DESC_STATIC_STRU(1, 4), DESC_OUTPUT_STRU(2, 4),
  3, DESC_P_PARA(0, 0, 0), DESC_P_STRU(1, 0, 1, offsetof(struct io_param, io_buff_in)), DESC_P_STRU(2, 0, 2, offsetof(struct io_param, io_buff_out)),
  0,                          
  0,
};


__u32 desc_sed_audio_change[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct pes_pkt_param)), DESC_STATIC_STRU(1, sizeof(struct pes_pkt_param)), 
  2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(0, 3, 0),
  //desc of pointer ret
  0,                          
  0,
};


UINT32 desc_sed_m36_is_av_scrambled_old[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(UINT8*)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,                          
  0,
};

UINT32 desc_sed_m36_is_av_source_scrambled_old[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(UINT8*)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,                          
  0,
};
UINT32 desc_sed_is_program_legal[] = 
{ //desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(UINT8*)),
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,                          
  0,
};
#endif

void sed_play_channel(UINT16 vpid, UINT16 apid, UINT16 ad_pid, struct pes_pkt_param *v_pes_param, struct pes_pkt_param *a_pes_param, struct pes_pkt_param *ad_pes_param)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, vpid, (LLD_DMX_M36F_MODULE<<24)|(6<<16)|FUNC_SED_PLAY_CHANNEL, desc_sed_play_channel);
#else
    Pes_pkt_param_rpc pes_v;
    Pes_pkt_param_rpc pes_a;
    Pes_pkt_param_rpc pes_ad;

    pes_v.stream_id = v_pes_param->stream_id;
    pes_v.pes_pkt_len = v_pes_param->pes_pkt_len;
    pes_v.filled_byte_3 = v_pes_param->filled_byte_3;
    pes_v.pes_scramb_ctrl = v_pes_param->pes_scramb_ctrl;
    pes_v.marker_10 = v_pes_param->marker_10;
    pes_v.filled_byte_4 = v_pes_param->filled_byte_4;
    pes_v.pts_dts_flags = v_pes_param->pts_dts_flags;
    pes_v.pes_header_data_len = v_pes_param->pes_header_data_len;
    pes_v.pes_data_len = v_pes_param->pes_data_len;
    pes_v.dmx_state = v_pes_param->dmx_state;
    pes_v.stream_type = v_pes_param->stream_type;
    pes_v.head_buf_pos = v_pes_param->head_buf_pos;
    pes_v.av_buf = (__u32)v_pes_param->av_buf;
    pes_v.av_buf_pos = v_pes_param->av_buf_pos;
    pes_v.av_buf_len = v_pes_param->av_buf_len;
    pes_v.ctrl_blk.stc_id_valid = v_pes_param->ctrl_blk->stc_id_valid;
    pes_v.ctrl_blk.pts_valid = v_pes_param->ctrl_blk->pts_valid;
    pes_v.ctrl_blk.data_continue = v_pes_param->ctrl_blk->data_continue;
    pes_v.ctrl_blk.ctrlblk_valid = v_pes_param->ctrl_blk->ctrlblk_valid;
    pes_v.ctrl_blk.instant_update = v_pes_param->ctrl_blk->instant_update;
    pes_v.ctrl_blk.vob_start = v_pes_param->ctrl_blk->vob_start;
    pes_v.ctrl_blk.bstream_run_back = v_pes_param->ctrl_blk->bstream_run_back;
    pes_v.ctrl_blk.reserve = v_pes_param->ctrl_blk->reserve;
    pes_v.ctrl_blk.stc_id = v_pes_param->ctrl_blk->stc_id;
    pes_v.ctrl_blk.stc_offset_idx = v_pes_param->ctrl_blk->stc_offset_idx;
    pes_v.ctrl_blk.pts = v_pes_param->ctrl_blk->pts;
    pes_v.device = (__u32)v_pes_param->device;
    pes_v.request_write = (__u32)v_pes_param->request_write;
    pes_v.update_write = (__u32)v_pes_param->update_write;
    pes_v.get_pkt_len = v_pes_param->get_pkt_len;
    pes_v.get_header_data_len = v_pes_param->get_header_data_len;
    pes_v.get_pts = v_pes_param->get_pts;
    pes_v.str_confirm = v_pes_param->str_confirm;
    pes_v.reserved = v_pes_param->reserved;
    pes_v.conti_conter = v_pes_param->conti_conter;
    pes_v.head_buf = (__u32)v_pes_param->head_buf;
    pes_v.ch_num = v_pes_param->ch_num;
    pes_v.channel = (__u32)v_pes_param->channel;
    pes_v.cw_parity_num = v_pes_param->cw_parity_num;
    pes_v.xfer_es_by_dma = v_pes_param->xfer_es_by_dma;
    pes_v.dma_ch_num = v_pes_param->dma_ch_num;
    pes_v.last_dma_xfer_id = v_pes_param->last_dma_xfer_id;
    pes_v.ts_err_code = v_pes_param->ts_err_code;
    pes_v.ovlp_cnt = v_pes_param->ovlp_cnt;
    pes_v.discont_cnt = v_pes_param->discont_cnt;
    pes_v.LastTsAdaCtrl = v_pes_param->LastTsAdaCtrl;
    pes_v.unlock_cnt = v_pes_param->unlock_cnt;
    pes_v.new_vbv_method_enable = v_pes_param->new_vbv_method_enable;
    pes_v.new_vbv_request = (__u32)v_pes_param->new_vbv_request;
    pes_v.new_vbv_update = (__u32)v_pes_param->new_vbv_update;    

    pes_a.stream_id = a_pes_param->stream_id;
    pes_a.pes_pkt_len = a_pes_param->pes_pkt_len;
    pes_a.filled_byte_3 = a_pes_param->filled_byte_3;
    pes_a.pes_scramb_ctrl = a_pes_param->pes_scramb_ctrl;
    pes_a.marker_10 = a_pes_param->marker_10;
    pes_a.filled_byte_4 = a_pes_param->filled_byte_4;
    pes_a.pts_dts_flags = a_pes_param->pts_dts_flags;
    pes_a.pes_header_data_len = a_pes_param->pes_header_data_len;
    pes_a.pes_data_len = a_pes_param->pes_data_len;
    pes_a.dmx_state = a_pes_param->dmx_state;
    pes_a.stream_type = a_pes_param->stream_type;
    pes_a.head_buf_pos = a_pes_param->head_buf_pos;
    pes_a.av_buf = (__u32)a_pes_param->av_buf;
    pes_a.av_buf_pos = a_pes_param->av_buf_pos;
    pes_a.av_buf_len = a_pes_param->av_buf_len;
    pes_a.ctrl_blk.stc_id_valid = a_pes_param->ctrl_blk->stc_id_valid;
    pes_a.ctrl_blk.pts_valid = a_pes_param->ctrl_blk->pts_valid;
    pes_a.ctrl_blk.data_continue = a_pes_param->ctrl_blk->data_continue;
    pes_a.ctrl_blk.ctrlblk_valid = a_pes_param->ctrl_blk->ctrlblk_valid;
    pes_a.ctrl_blk.instant_update = a_pes_param->ctrl_blk->instant_update;
    pes_a.ctrl_blk.vob_start = a_pes_param->ctrl_blk->vob_start;
    pes_a.ctrl_blk.bstream_run_back = a_pes_param->ctrl_blk->bstream_run_back;
    pes_a.ctrl_blk.reserve = a_pes_param->ctrl_blk->reserve;
    pes_a.ctrl_blk.stc_id = a_pes_param->ctrl_blk->stc_id;
    pes_a.ctrl_blk.stc_offset_idx = a_pes_param->ctrl_blk->stc_offset_idx;
    pes_a.ctrl_blk.pts = a_pes_param->ctrl_blk->pts;
    pes_a.device = (__u32)a_pes_param->device;
    pes_a.request_write = (__u32)a_pes_param->request_write;
    pes_a.update_write = (__u32)a_pes_param->update_write;
    pes_a.get_pkt_len = a_pes_param->get_pkt_len;
    pes_a.get_header_data_len = a_pes_param->get_header_data_len;
    pes_a.get_pts = a_pes_param->get_pts;
    pes_a.str_confirm = a_pes_param->str_confirm;
    pes_a.reserved = a_pes_param->reserved;
    pes_a.conti_conter = a_pes_param->conti_conter;
    pes_a.head_buf = (__u32)a_pes_param->head_buf;
    pes_a.ch_num = a_pes_param->ch_num;
    pes_a.channel = (__u32)a_pes_param->channel;
    pes_a.cw_parity_num = a_pes_param->cw_parity_num;
    pes_a.xfer_es_by_dma = a_pes_param->xfer_es_by_dma;
    pes_a.dma_ch_num = a_pes_param->dma_ch_num;
    pes_a.last_dma_xfer_id = a_pes_param->last_dma_xfer_id;
    pes_a.ts_err_code = a_pes_param->ts_err_code;
    pes_a.ovlp_cnt = a_pes_param->ovlp_cnt;
    pes_a.discont_cnt = a_pes_param->discont_cnt;
    pes_a.LastTsAdaCtrl = a_pes_param->LastTsAdaCtrl;
    pes_a.unlock_cnt = a_pes_param->unlock_cnt;
    pes_a.new_vbv_method_enable = a_pes_param->new_vbv_method_enable;
    pes_a.new_vbv_request = (__u32)a_pes_param->new_vbv_request;
    pes_a.new_vbv_update = (__u32)a_pes_param->new_vbv_update;

    pes_ad.stream_id = ad_pes_param->stream_id;
    pes_ad.pes_pkt_len = ad_pes_param->pes_pkt_len;
    pes_ad.filled_byte_3 = ad_pes_param->filled_byte_3;
    pes_ad.pes_scramb_ctrl = ad_pes_param->pes_scramb_ctrl;
    pes_ad.marker_10 = ad_pes_param->marker_10;
    pes_ad.filled_byte_4 = ad_pes_param->filled_byte_4;
    pes_ad.pts_dts_flags = ad_pes_param->pts_dts_flags;
    pes_ad.pes_header_data_len = ad_pes_param->pes_header_data_len;
    pes_ad.pes_data_len = ad_pes_param->pes_data_len;
    pes_ad.dmx_state = ad_pes_param->dmx_state;
    pes_ad.stream_type = ad_pes_param->stream_type;
    pes_ad.head_buf_pos = ad_pes_param->head_buf_pos;
    pes_ad.av_buf = (__u32)ad_pes_param->av_buf;
    pes_ad.av_buf_pos = ad_pes_param->av_buf_pos;
    pes_ad.av_buf_len = ad_pes_param->av_buf_len;
    pes_ad.ctrl_blk.stc_id_valid = ad_pes_param->ctrl_blk->stc_id_valid;
    pes_ad.ctrl_blk.pts_valid = ad_pes_param->ctrl_blk->pts_valid;
    pes_ad.ctrl_blk.data_continue = ad_pes_param->ctrl_blk->data_continue;
    pes_ad.ctrl_blk.ctrlblk_valid = ad_pes_param->ctrl_blk->ctrlblk_valid;
    pes_ad.ctrl_blk.instant_update = ad_pes_param->ctrl_blk->instant_update;
    pes_ad.ctrl_blk.vob_start = ad_pes_param->ctrl_blk->vob_start;
    pes_ad.ctrl_blk.bstream_run_back = ad_pes_param->ctrl_blk->bstream_run_back;
    pes_ad.ctrl_blk.reserve = ad_pes_param->ctrl_blk->reserve;
    pes_ad.ctrl_blk.stc_id = ad_pes_param->ctrl_blk->stc_id;
    pes_ad.ctrl_blk.stc_offset_idx = ad_pes_param->ctrl_blk->stc_offset_idx;
    pes_ad.ctrl_blk.pts = ad_pes_param->ctrl_blk->pts;
    pes_ad.device = (__u32)ad_pes_param->device;
    pes_ad.request_write = (__u32)ad_pes_param->request_write;
    pes_ad.update_write = (__u32)ad_pes_param->update_write;
    pes_ad.get_pkt_len = ad_pes_param->get_pkt_len;
    pes_ad.get_header_data_len = ad_pes_param->get_header_data_len;
    pes_ad.get_pts = ad_pes_param->get_pts;
    pes_ad.str_confirm = ad_pes_param->str_confirm;
    pes_ad.reserved = ad_pes_param->reserved;
    pes_ad.conti_conter = ad_pes_param->conti_conter;
    pes_ad.head_buf = (__u32)ad_pes_param->head_buf;
    pes_ad.ch_num = ad_pes_param->ch_num;
    pes_ad.channel = (__u32)ad_pes_param->channel;
    pes_ad.cw_parity_num = ad_pes_param->cw_parity_num;
    pes_ad.xfer_es_by_dma = ad_pes_param->xfer_es_by_dma;
    pes_ad.dma_ch_num = ad_pes_param->dma_ch_num;
    pes_ad.last_dma_xfer_id = ad_pes_param->last_dma_xfer_id;
    pes_ad.ts_err_code = ad_pes_param->ts_err_code;
    pes_ad.ovlp_cnt = ad_pes_param->ovlp_cnt;
    pes_ad.discont_cnt = ad_pes_param->discont_cnt;
    pes_ad.LastTsAdaCtrl = ad_pes_param->LastTsAdaCtrl;
    pes_ad.unlock_cnt = ad_pes_param->unlock_cnt;
    pes_ad.new_vbv_method_enable = ad_pes_param->new_vbv_method_enable;
    pes_ad.new_vbv_request = (__u32)ad_pes_param->new_vbv_request;
    pes_ad.new_vbv_update = (__u32)ad_pes_param->new_vbv_update;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT16, sizeof(UINT16), &vpid);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT16, sizeof(UINT16), &apid);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT16, sizeof(UINT16), &ad_pid);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_Pes_pkt_param_rpc, sizeof(Pes_pkt_param_rpc), &pes_v);
    RPC_PARAM_CREATE(p5, PARAM_IN, PARAM_Pes_pkt_param_rpc, sizeof(Pes_pkt_param_rpc), &pes_a);
    RPC_PARAM_CREATE(p6, PARAM_IN, PARAM_Pes_pkt_param_rpc, sizeof(Pes_pkt_param_rpc), &pes_ad);

    RpcCallCompletion(RPC_sed_play_channel,&p1,&p2,&p3,&p4,&p5,&p6,NULL);
#endif
}


void sed_reg_serv(UINT16 pid, UINT8 filter_idx, struct pes_pkt_param *pes_param)
{
#if defined(CONFIG_ALI_RPCNG)

    Pes_pkt_param_rpc pes_p;

    #if 0
    pes_p.stream_id = pes_param->stream_id;
    pes_p.pes_pkt_len = pes_param->pes_pkt_len;
    pes_p.filled_byte_3 = pes_param->filled_byte_3;
    pes_p.pes_scramb_ctrl = pes_param->pes_scramb_ctrl;
    pes_p.marker_10 = pes_param->marker_10;
    pes_p.filled_byte_4 = pes_param->filled_byte_4;
    pes_p.pts_dts_flags = pes_param->pts_dts_flags;
    pes_p.pes_header_data_len = pes_param->pes_header_data_len;
    pes_p.pes_data_len = pes_param->pes_data_len;
    pes_p.dmx_state = pes_param->dmx_state;
    pes_p.stream_type = pes_param->stream_type;
    pes_p.head_buf_pos = pes_param->head_buf_pos;
    pes_p.av_buf = (__u32)pes_param->av_buf;
    pes_p.av_buf_pos = pes_param->av_buf_pos;
    pes_p.av_buf_len = pes_param->av_buf_len;
    #endif
	
	#if 0
    pes_p.ctrl_blk.stc_id_valid = pes_param->ctrl_blk->stc_id_valid;
    pes_p.ctrl_blk.pts_valid = pes_param->ctrl_blk->pts_valid;
    pes_p.ctrl_blk.data_continue = pes_param->ctrl_blk->data_continue;
    pes_p.ctrl_blk.ctrlblk_valid = pes_param->ctrl_blk->ctrlblk_valid;
    pes_p.ctrl_blk.instant_update = pes_param->ctrl_blk->instant_update;
    pes_p.ctrl_blk.vob_start = pes_param->ctrl_blk->vob_start;
    pes_p.ctrl_blk.bstream_run_back = pes_param->ctrl_blk->bstream_run_back;
    pes_p.ctrl_blk.reserve = pes_param->ctrl_blk->reserve;
    pes_p.ctrl_blk.stc_id = pes_param->ctrl_blk->stc_id;
    pes_p.ctrl_blk.stc_offset_idx = pes_param->ctrl_blk->stc_offset_idx;
    pes_p.ctrl_blk.pts = pes_param->ctrl_blk->pts;
	#endif

	#if 0
    pes_p.device = (__u32)pes_param->device;
    pes_p.request_write = (__u32)pes_param->request_write;
    pes_p.update_write = (__u32)pes_param->update_write;
    pes_p.get_pkt_len = pes_param->get_pkt_len;
    pes_p.get_header_data_len = pes_param->get_header_data_len;
    pes_p.get_pts = pes_param->get_pts;
    pes_p.str_confirm = pes_param->str_confirm;
    pes_p.reserved = pes_param->reserved;
    pes_p.conti_conter = pes_param->conti_conter;
    pes_p.head_buf = (__u32)pes_param->head_buf;
    pes_p.ch_num = pes_param->ch_num;
    pes_p.channel = (__u32)pes_param->channel;
    pes_p.cw_parity_num = pes_param->cw_parity_num;
    pes_p.xfer_es_by_dma = pes_param->xfer_es_by_dma;
    pes_p.dma_ch_num = pes_param->dma_ch_num;
    pes_p.last_dma_xfer_id = pes_param->last_dma_xfer_id;
    pes_p.ts_err_code = pes_param->ts_err_code;
    pes_p.ovlp_cnt = pes_param->ovlp_cnt;
    pes_p.discont_cnt = pes_param->discont_cnt;
    pes_p.LastTsAdaCtrl = pes_param->LastTsAdaCtrl;
    pes_p.unlock_cnt = pes_param->unlock_cnt;
    pes_p.new_vbv_method_enable = pes_param->new_vbv_method_enable;
    pes_p.new_vbv_request = (__u32)pes_param->new_vbv_request;
    pes_p.new_vbv_update = (__u32)pes_param->new_vbv_update;
	#endif

    memset(&pes_p, 0, sizeof(Pes_pkt_param_rpc));

    pes_p.device = (__u32)pes_param->device;	
    pes_p.ch_num = pes_param->ch_num;
	
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT16, sizeof(UINT16), &pid);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &filter_idx);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_Pes_pkt_param_rpc, sizeof(Pes_pkt_param_rpc), &pes_p);

    RpcCallCompletion(RPC_sed_reg_serv,&p1,&p2,&p3,NULL);

#else
    jump_to_func(NULL, ali_rpc_call, pid, (LLD_DMX_M36F_MODULE<<24)|(3<<16)|FUNC_SED_REG_SERV, desc_sed_reg_serv);
#endif
}
__s32 sed_m36_is_av_scrambled(struct io_param_ex   *param)
{
#if !defined(CONFIG_ALI_RPCNG)
    register __s32 ret asm("$2");   

    jump_to_func(NULL, ali_rpc_call, param, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_M36_IS_AV_SCRAMBLED, desc_sed_m36_is_av_scrambled);

    return(ret);
#else
    Int32 ret;
    Io_param_ex_rpc io_par;
    
    io_par.io_buff_in = (UINT32)param->io_buff_in;
    io_par.buff_in_len = param->buff_in_len;
    io_par.io_buff_out = (UINT32)param->io_buff_out;
    io_par.buff_out_len = param->buff_out_len;
    io_par.hnd = param->hnd;
    io_par.h264_flag = param->h264_flag;
    io_par.is_scrambled = param->is_scrambled;
    io_par.record_all = param->record_all;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Io_param_ex_rpc, sizeof(Io_param_ex_rpc), &io_par);

    ret = RpcCallCompletion(RPC_sed_m36_is_av_scrambled,&p1,NULL);
    return ret;
    
#endif
}
/*
UINT32 sed_standby(UINT32 status)  // -> move to hld_base
{
    register __s32 ret asm("$2");	

    jump_to_func(NULL, ali_rpc_call, status, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_STANDBY, NULL);

    return(ret);
}
*/

void sed_set_dev(void *dec_dev,UINT32 decrypt_type)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, dec_dev, (LLD_DMX_M36F_MODULE<<24)|(2<<16)|FUNC_SED_SET_DEV, NULL);

#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dec_dev);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &decrypt_type);

    RpcCallCompletion(RPC_sed_set_dev,&p1,&p2,NULL);
#endif
}

void sed_adjust_stc(__u32 pdev)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, pdev, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_ADJUST_STC, NULL);
#else
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pdev);

    RpcCallCompletion(RPC_sed_adjust_stc,&p1,NULL);        

#endif
    
}
void sed_dmx_playback_submit_special(UINT8 *buf, __u32 len)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, buf, (LLD_DMX_M36F_MODULE<<24)|(2<<16)|FUNC_SED_SUMMIT_SPEC, NULL);
#else
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &buf);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &len);
    if(buf == NULL)
    {
        RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UCHAR, 0, NULL);
    }       

    RpcCallCompletion(RPC_sed_dmx_playback_submit_special,&p1,&p2,NULL);        
#endif  
}
void sed_dmx_playback_stc_monitor(__u32 pcr)
{
#if !defined(CONFIG_ALI_RPCNG)    
    jump_to_func(NULL, ali_rpc_call, pcr, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_STC_MONITOR, NULL);
#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pcr);

    RpcCallCompletion(RPC_sed_dmx_playback_stc_monitor,&p1,NULL);        

#endif

    
}

void sed_set_playback_speed(__u32 speed)
{
#if !defined(CONFIG_ALI_RPCNG)
    
     jump_to_func(NULL, ali_rpc_call, speed, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_PLAYBACK_SPEED, NULL);
#else

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &speed);

    RpcCallCompletion(RPC_sed_set_playback_speed,&p1,NULL);        

#endif
    
}

__u32 sed_set_dmx_parse_info(void *sed_info)
{
#if !defined(CONFIG_ALI_RPCNG)
    register __s32 ret asm("$2");   

    jump_to_func(NULL, ali_rpc_call, sed_info, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_PARSE_INFO, NULL);
    return(ret);
#else
    
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &sed_info);
    if(sed_info == NULL)
    {
        RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_UINT32, 0, NULL);
    }        

    ret = RpcCallCompletion(RPC_sed_set_dmx_parse_info,&p1,NULL);
    return ret;    

#endif

}

__s32 see_parse_pes_header(__u32 stream_id, UINT8 *buf,__u32 buf_len)
{
#if !defined(CONFIG_ALI_RPCNG)
    register __s32 ret asm("$2");   

    jump_to_func(NULL, ali_rpc_call, stream_id, (LLD_DMX_M36F_MODULE<<24)|(3<<16)|FUNC_SED_PARSE_PES, NULL);
    return(ret);
#else
    
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &stream_id);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &buf);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &buf_len);

    if(buf == NULL)
    {
       RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UCHAR, 0, NULL);
    }
    ret = RpcCallCompletion(RPC_see_parse_pes_header,&p1,&p2,&p3,NULL);        
    return ret;    

#endif    
}
void see_dmx_update_remain_data(__u32 dummy)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_UPDATE_REMAIN_DATA, NULL);
#else
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dummy);

    RpcCallCompletion(RPC_see_dmx_update_remain_data,&p1,NULL);        

#endif        
}
void sed_dmx_tsg_pre_adjust(__u32 pcr,__u32 mode)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, pcr, (LLD_DMX_M36F_MODULE<<24)|(2<<16)|FUNC_SED_TSG_ADJUST_PCR, NULL);
#else
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pcr);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &mode);

    RpcCallCompletion(RPC_sed_dmx_tsg_pre_adjust,&p1,&p2,NULL);        

#endif
}

void sed_set_av_mode(__u32 mode)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, mode, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_AV_MODE, NULL);
#else
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &mode);

    RpcCallCompletion(RPC_sed_set_av_mode,&p1,NULL);        

#endif    
}

void sed_reset_to_live_mode(__u32 dummy)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_RESET_TO_LIVE_MODE, NULL);
#else
    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dummy);

    RpcCallCompletion(RPC_sed_reset_to_live_mode,&p1,NULL); 
#endif    
}
 
void sed_set_decrypt_status(__u32 OnOff)
{
#if !defined(CONFIG_ALI_RPCNG)
   jump_to_func(NULL, ali_rpc_call, OnOff, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_DECRYPT_STATUS, NULL);
#else
    
   RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &OnOff);

   RpcCallCompletion(RPC_sed_set_decrypt_status,&p1,NULL); 

#endif   
}

void sed_enable_video_dma_channel(__u32 dummy)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(0<<16)|FUNC_SED_ENABLE_VIDEO_DMA, NULL);
#else    
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dummy);

    RpcCallCompletion(RPC_sed_enable_video_dma_channel,&p1,NULL);        
#endif
}

void set_sed_parse_status(__u32 OnOff)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, OnOff, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_PARSE_STATUS, NULL);
#else    
    
   RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &OnOff);

   RpcCallCompletion(RPC_set_sed_parse_status,&p1,NULL);  

#endif   
}
void sed_set_pcr_pid(UINT16 pcr_pid)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, pcr_pid, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_PCR_PID, NULL);
#else    
    
   RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT16, sizeof(UINT16), &pcr_pid);

   RpcCallCompletion(RPC_sed_set_pcr_pid,&p1,NULL);   
#endif   
}


void sed_dmx_get_discont(__u32 *data)
{
#if !defined(CONFIG_ALI_RPCNG)
    __u32 desc[] = 
    { //desc of pointer para
      1, DESC_STATIC_STRU(0, 4),
      1, DESC_P_PARA(0, 0, 0),
      0,                          
      0,
    };
    DESC_OUTPUT_STRU_SET_SIZE(desc, 0, sizeof(data)); 
    jump_to_func(NULL, ali_rpc_call, data, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_GET_DISCNT, desc);

#else    
    
   RPC_PARAM_CREATE(p1, PARAM_BUFFER, PARAM_OPAQUE, sizeof(data), data);

   RpcCallCompletion(RPC_sed_dmx_get_discont,&p1,NULL); 
#endif   
   
}
void sed_dmx_clear_discont(__u32 dummy)
{
#if !defined(CONFIG_ALI_RPCNG)
    jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_CLEAR_DISCNT, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dummy);

    RpcCallCompletion(RPC_sed_dmx_clear_discont,&p1,NULL);
#endif
}

void sed_change_audio_channel(UINT16 apid, UINT16 adpid, struct pes_pkt_param *a_pes_param, struct pes_pkt_param *ad_pes_param)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, apid, (LLD_DMX_M36F_MODULE<<24)|(4<<16)|FUNC_SED_AUDIO_CHANGE, desc_sed_audio_change);
#else
    Pes_pkt_param_rpc pes_a;
    Pes_pkt_param_rpc pes_p;

    pes_a.stream_id = a_pes_param->stream_id;
    pes_a.pes_pkt_len = a_pes_param->pes_pkt_len;
    pes_a.filled_byte_3 = a_pes_param->filled_byte_3;
    pes_a.pes_scramb_ctrl = a_pes_param->pes_scramb_ctrl;
    pes_a.marker_10 = a_pes_param->marker_10;
    pes_a.filled_byte_4 = a_pes_param->filled_byte_4;
    pes_a.pts_dts_flags = a_pes_param->pts_dts_flags;
    pes_a.pes_header_data_len = a_pes_param->pes_header_data_len;
    pes_a.pes_data_len = a_pes_param->pes_data_len;
    pes_a.dmx_state = a_pes_param->dmx_state;
    pes_a.stream_type = a_pes_param->stream_type;
    pes_a.head_buf_pos = a_pes_param->head_buf_pos;
    pes_a.av_buf = (__u32)a_pes_param->av_buf;
    pes_a.av_buf_pos = a_pes_param->av_buf_pos;
    pes_a.av_buf_len = a_pes_param->av_buf_len;

	#if 0
    pes_a.ctrl_blk.stc_id_valid = a_pes_param->ctrl_blk->stc_id_valid;
    pes_a.ctrl_blk.pts_valid = a_pes_param->ctrl_blk->pts_valid;
    pes_a.ctrl_blk.data_continue = a_pes_param->ctrl_blk->data_continue;
    pes_a.ctrl_blk.ctrlblk_valid = a_pes_param->ctrl_blk->ctrlblk_valid;
    pes_a.ctrl_blk.instant_update = a_pes_param->ctrl_blk->instant_update;
    pes_a.ctrl_blk.vob_start = a_pes_param->ctrl_blk->vob_start;
    pes_a.ctrl_blk.bstream_run_back = a_pes_param->ctrl_blk->bstream_run_back;
    pes_a.ctrl_blk.reserve = a_pes_param->ctrl_blk->reserve;
    pes_a.ctrl_blk.stc_id = a_pes_param->ctrl_blk->stc_id;
    pes_a.ctrl_blk.stc_offset_idx = a_pes_param->ctrl_blk->stc_offset_idx;
    pes_a.ctrl_blk.pts = a_pes_param->ctrl_blk->pts;
	#endif
	
    pes_a.device = (__u32)a_pes_param->device;
    pes_a.request_write = (__u32)a_pes_param->request_write;
    pes_a.update_write = (__u32)a_pes_param->update_write;
    pes_a.get_pkt_len = a_pes_param->get_pkt_len;
    pes_a.get_header_data_len = a_pes_param->get_header_data_len;
    pes_a.get_pts = a_pes_param->get_pts;
    pes_a.str_confirm = a_pes_param->str_confirm;
    pes_a.reserved = a_pes_param->reserved;
    pes_a.conti_conter = a_pes_param->conti_conter;
    pes_a.head_buf = (__u32)a_pes_param->head_buf;
    pes_a.ch_num = a_pes_param->ch_num;
    pes_a.channel = (__u32)a_pes_param->channel;
    pes_a.cw_parity_num = a_pes_param->cw_parity_num;
    pes_a.xfer_es_by_dma = a_pes_param->xfer_es_by_dma;
    pes_a.dma_ch_num = a_pes_param->dma_ch_num;
    pes_a.last_dma_xfer_id = a_pes_param->last_dma_xfer_id;
    pes_a.ts_err_code = a_pes_param->ts_err_code;
    pes_a.ovlp_cnt = a_pes_param->ovlp_cnt;
    pes_a.discont_cnt = a_pes_param->discont_cnt;
    pes_a.LastTsAdaCtrl = a_pes_param->LastTsAdaCtrl;
    pes_a.unlock_cnt = a_pes_param->unlock_cnt;
    pes_a.new_vbv_method_enable = a_pes_param->new_vbv_method_enable;
    pes_a.new_vbv_request = (__u32)a_pes_param->new_vbv_request;
    pes_a.new_vbv_update = (__u32)a_pes_param->new_vbv_update;

    pes_p.stream_id = ad_pes_param->stream_id;
    pes_p.pes_pkt_len = ad_pes_param->pes_pkt_len;
    pes_p.filled_byte_3 = ad_pes_param->filled_byte_3;
    pes_p.pes_scramb_ctrl = ad_pes_param->pes_scramb_ctrl;
    pes_p.marker_10 = ad_pes_param->marker_10;
    pes_p.filled_byte_4 = ad_pes_param->filled_byte_4;
    pes_p.pts_dts_flags = ad_pes_param->pts_dts_flags;
    pes_p.pes_header_data_len = ad_pes_param->pes_header_data_len;
    pes_p.pes_data_len = ad_pes_param->pes_data_len;
    pes_p.dmx_state = ad_pes_param->dmx_state;
    pes_p.stream_type = ad_pes_param->stream_type;
    pes_p.head_buf_pos = ad_pes_param->head_buf_pos;
    pes_p.av_buf = (__u32)ad_pes_param->av_buf;
    pes_p.av_buf_pos = ad_pes_param->av_buf_pos;
    pes_p.av_buf_len = ad_pes_param->av_buf_len;

	#if 0
    pes_p.ctrl_blk.stc_id_valid = ad_pes_param->ctrl_blk->stc_id_valid;
    pes_p.ctrl_blk.pts_valid = ad_pes_param->ctrl_blk->pts_valid;
    pes_p.ctrl_blk.data_continue = ad_pes_param->ctrl_blk->data_continue;
    pes_p.ctrl_blk.ctrlblk_valid = ad_pes_param->ctrl_blk->ctrlblk_valid;
    pes_p.ctrl_blk.instant_update = ad_pes_param->ctrl_blk->instant_update;
    pes_p.ctrl_blk.vob_start = ad_pes_param->ctrl_blk->vob_start;
    pes_p.ctrl_blk.bstream_run_back = ad_pes_param->ctrl_blk->bstream_run_back;
    pes_p.ctrl_blk.reserve = ad_pes_param->ctrl_blk->reserve;
    pes_p.ctrl_blk.stc_id = ad_pes_param->ctrl_blk->stc_id;
    pes_p.ctrl_blk.stc_offset_idx = ad_pes_param->ctrl_blk->stc_offset_idx;
    pes_p.ctrl_blk.pts = ad_pes_param->ctrl_blk->pts;
	#endif
	
    pes_p.device = (__u32)ad_pes_param->device;
    pes_p.request_write = (__u32)ad_pes_param->request_write;
    pes_p.update_write = (__u32)ad_pes_param->update_write;
    pes_p.get_pkt_len = ad_pes_param->get_pkt_len;
    pes_p.get_header_data_len = ad_pes_param->get_header_data_len;
    pes_p.get_pts = ad_pes_param->get_pts;
    pes_p.str_confirm = ad_pes_param->str_confirm;
    pes_p.reserved = ad_pes_param->reserved;
    pes_p.conti_conter = ad_pes_param->conti_conter;
    pes_p.head_buf = (__u32)ad_pes_param->head_buf;
    pes_p.ch_num = ad_pes_param->ch_num;
    pes_p.channel = (__u32)ad_pes_param->channel;
    pes_p.cw_parity_num = ad_pes_param->cw_parity_num;
    pes_p.xfer_es_by_dma = ad_pes_param->xfer_es_by_dma;
    pes_p.dma_ch_num = ad_pes_param->dma_ch_num;
    pes_p.last_dma_xfer_id = ad_pes_param->last_dma_xfer_id;
    pes_p.ts_err_code = ad_pes_param->ts_err_code;
    pes_p.ovlp_cnt = ad_pes_param->ovlp_cnt;
    pes_p.discont_cnt = ad_pes_param->discont_cnt;
    pes_p.LastTsAdaCtrl = ad_pes_param->LastTsAdaCtrl;
    pes_p.unlock_cnt = ad_pes_param->unlock_cnt;
    pes_p.new_vbv_method_enable = ad_pes_param->new_vbv_method_enable;
    pes_p.new_vbv_request = (__u32)ad_pes_param->new_vbv_request;
    pes_p.new_vbv_update = (__u32)ad_pes_param->new_vbv_update;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT16, sizeof(UINT16), &apid);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT16, sizeof(UINT16), &adpid);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_Pes_pkt_param_rpc, sizeof(Pes_pkt_param_rpc), &pes_a);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_Pes_pkt_param_rpc, sizeof(Pes_pkt_param_rpc), &pes_p);

    RpcCallCompletion(RPC_sed_change_audio_channel,&p1,&p2,&p3,&p4,NULL);
#endif
}

RET_CODE sed_dmx_is_av_scrambled_old(UINT8 *scramble)
{
#if !defined(CONFIG_ALI_RPCNG)
    register __s32 ret asm("$2");   

    jump_to_func(NULL, ali_rpc_call, scramble, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_M36_IS_AV_SCRAMBLED_OLD, desc_sed_m36_is_av_scrambled_old);

    return(ret);
#else
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), scramble);

    ret = RpcCallCompletion(RPC_sed_dmx_is_av_scrambled_old,&p1,NULL);
    return ret;
#endif

}

RET_CODE sed_dmx_is_av_source_scrambled_old(UINT8 *scramble)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, scramble, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_M36_IS_AV_SOURCE_SCRAMBLED_OLD, desc_sed_m36_is_av_source_scrambled_old);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), scramble);

    RpcCallCompletion(RPC_sed_dmx_is_av_source_scrambled_old,&p1,NULL);        
#endif
    return(0);
}
void sed_program_is_legal(UINT8 *scramble)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, scramble, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_IS_PROGRAM_LEGAL, desc_sed_is_program_legal);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), scramble);

    RpcCallCompletion(RPC_sed_dmx_is_av_source_scrambled_old,&p1,NULL);        
#endif
}
void sed_reset_pes_param(UINT8 type, UINT8 in_tsk_context)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, type, (LLD_DMX_M36F_MODULE<<24)|(2<<16)|FUNC_SED_RESET_PES_PARAM, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &type);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UCHAR, sizeof(UINT8), &in_tsk_context);

    RpcCallCompletion(RPC_sed_reset_pes_param,&p1,&p2,NULL);        
#endif
}
void sed_set_hw_pcr_base(UINT32 base)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, base, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_HW_PCR_BASE, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &base);

    RpcCallCompletion(RPC_sed_set_hw_pcr_base,&p1,NULL);        
#endif
}

void sed_set_ddp_certification(UINT32 enable)
{
#if !defined(CONFIG_ALI_RPCNG)

   jump_to_func(NULL, ali_rpc_call, enable, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_DDP_CERTIFICATION, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &enable);

    RpcCallCompletion(RPC_sed_set_ddp_certification,&p1,NULL);        
#endif
}

void sed_set_drop_error_pes(UINT32 enable)
{
#if !defined(CONFIG_ALI_RPCNG)

   jump_to_func(NULL, ali_rpc_call, enable, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_DROP_ERROR_PES, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &enable);

    RpcCallCompletion(RPC_sed_set_drop_error_pes,&p1,NULL);        
#endif
}  
void sed_dmx_attach(UINT32 dummy)
{
#if !defined(CONFIG_ALI_RPCNG)

    jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE << 24) | (1 << 16) | FUNC_SED_DMX_ATTACH, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dummy);

    RpcCallCompletion(RPC_sed_dmx_attach,&p1,NULL);        
#endif
}
  

/* Added for linux A/V API.
 * Date:2012.02.23 by Joy.
 */
void sed_dmx_video_stream_start(__u32 pid)
{
#if !defined(CONFIG_ALI_RPCNG)

   jump_to_func(NULL, ali_rpc_call, pid, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_DMX_VIDEO_STREAM_START, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pid);

    RpcCallCompletion(RPC_sed_dmx_video_stream_start,&p1,NULL);        
#endif
}

void sed_dmx_video_stream_stop(__u32 dummy)
{
#if !defined(CONFIG_ALI_RPCNG)

   jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_DMX_VIDEO_STREAM_STOP, NULL);
#else
   RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dummy);

    RpcCallCompletion(RPC_sed_dmx_video_stream_stop,&p1,NULL);        
#endif
}


void sed_dmx_audio_stream_start(__u32 pid)
{
#if !defined(CONFIG_ALI_RPCNG)

   jump_to_func(NULL, ali_rpc_call, pid, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_DMX_AUDIO_STREAM_START, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pid);

    RpcCallCompletion(RPC_sed_dmx_audio_stream_start,&p1,NULL);        
#endif
}

void sed_dmx_audio_stream_stop(__u32 dummy)
{
#if !defined(CONFIG_ALI_RPCNG)

   jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_DMX_AUDIO_STREAM_STOP, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dummy);

    RpcCallCompletion(RPC_sed_dmx_audio_stream_stop,&p1,NULL);        
#endif
}




void sed_dmx_ttx_stream_start(__u32 pid)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pid);

    RpcCallCompletion(RPC_sed_dmx_ttx_stream_start,&p1,NULL);        
}


void sed_dmx_ttx_stream_stop(__u32 dummy)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dummy);

    RpcCallCompletion(RPC_sed_dmx_ttx_stream_stop,&p1,NULL);        
}



void sed_dmx_subt_stream_start(__u32 pid)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &pid);

    RpcCallCompletion(RPC_sed_dmx_subt_stream_start,&p1,NULL);        
}


void sed_dmx_subt_stream_stop(__u32 dummy)
{
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &dummy);

    RpcCallCompletion(RPC_sed_dmx_subt_stream_stop,&p1,NULL);        
}



__u32 Sed_DmxSee2mainBufInfoSet(void *See2mainBufInfo)
{
#if !defined(CONFIG_ALI_RPCNG)

    register __s32 ret asm("$2");   

    jump_to_func(NULL, ali_rpc_call, See2mainBufInfo, (LLD_DMX_M36F_MODULE<<24)|(1<<16) | FUNC_SED_DMX_SEE2MAIN_BUF_INFO_SET, NULL);
	
    return(ret);
#else
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &See2mainBufInfo);

    ret = RpcCallCompletion(RPC_Sed_DmxSee2mainBufInfoSet,&p1,NULL);
    return ret;
#endif
}



__s32 Sed_DmxSee2mainPidAdd(__u32 Pid)
{
#if !defined(CONFIG_ALI_RPCNG)

	jump_to_func(NULL, ali_rpc_call, pid, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_DMX_SEE2MAIN_PID_ADD, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &Pid);

    RpcCallCompletion(RPC_Sed_DmxSee2mainPidAdd,&p1,NULL);        
#endif
    return(0);
}



__s32 Sed_DmxSee2mainPidDel(__u32 Pid)
{
#if !defined(CONFIG_ALI_RPCNG)
	jump_to_func(NULL, ali_rpc_call, pid, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_DMX_SEE2MAIN_PID_DEL, NULL);
#else
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &Pid);

    RpcCallCompletion(RPC_Sed_DmxSee2mainPidDel,&p1,NULL);        
#endif

    return(0);
}



__s32 Sed_DmxSeeBufSet
(
    __u32 Main2SeeBufInfo,
    __u32 See2MianBufInfo,
    __u32 SeeDecryptBufAddr,
    __u32 SeeDecryptBufLen,
    __u32 SeeCoreStatisticsAddr,     
    __u32 SeeGlobalStatisticsAddr, 
    __u32 SeePlayChStatisticsAddr
)
{
    __s32 ret;
	
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &Main2SeeBufInfo);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &See2MianBufInfo);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &SeeDecryptBufAddr);
    RPC_PARAM_CREATE(p4, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &SeeDecryptBufLen);
    RPC_PARAM_CREATE(p5, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &SeeCoreStatisticsAddr);
    RPC_PARAM_CREATE(p6, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &SeeGlobalStatisticsAddr);
    RPC_PARAM_CREATE(p7, PARAM_IN, PARAM_UINT32, sizeof(UINT32), &SeePlayChStatisticsAddr);

    printk("Main2SeeBufAddr:%x\n", Main2SeeBufInfo);
    printk("See2MianBufAddr:%x\n", See2MianBufInfo);
    printk("SeeDecryptBufAddr:%x\n", SeeDecryptBufAddr);
    printk("SeeCoreStatisticsAddr:%x\n", SeeCoreStatisticsAddr);
    printk("SeeGlobalStatisticsAddr:%x\n", SeeGlobalStatisticsAddr);
    printk("SeePlayChStatisticsAddr:%x\n", SeePlayChStatisticsAddr);	

    ret = RpcCallCompletion(RPC_Sed_DmxSeeBufSet, &p1, &p2, &p3, &p4, &p5, &p6, &p7, NULL);  

	return(ret);
}


__s32 dmx_see_glb_info_get
(
    struct Ali_DmxSeeGlobalStatInfo *glb_inf
)
{

	__s32 ret;

	//printk("%s,%d\n", __FUNCTION__, __LINE__);
	
	RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_OPAQUE, sizeof(struct Ali_DmxSeeGlobalStatInfo), glb_inf);
	
    ret = RpcCallCompletion(RPC_sed_dmx_see_glb_info_get, &p1, NULL);  	

	//printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);		
	
    return(ret);
}


__s32 dmx_see_ch_info_get
(
    struct Ali_DmxSeePlyChStatInfo *ch_inf,
    __s32                           ch_cnt
)
{

	__s32 ret;
	__s32 data_len;

	data_len = sizeof(struct Ali_DmxSeePlyChStatInfo) * ch_cnt;

	//printk("%s,%d,data_len:%d\n", __FUNCTION__, __LINE__, data_len);
	
	RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_OPAQUE, data_len, ch_inf);

	RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_INT32, sizeof(INT32), &ch_cnt);
	
    ret = RpcCallCompletion(RPC_sed_dmx_see_ch_info_get, &p1, &p2, NULL);  	

	//printk("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, ret);		
	
    return(ret);
}


__s32 dmx_see_pid_is_descramling
(
    __u16 pid
)
{
    struct dmx_see_device *dmx_see;

    dmx_see = &(ali_dmx_see_dev[0]);

    if (0 == g_see_crypto_started)
    {
        return(-1);
    }

    if (dmx_see->status != DMX_SEE_STATUS_RUN)
    {
        return(-1);
    }

    if (dmx_see->usr_param.v_pid == pid)
    {
        return(0);
    }

    if (dmx_see->usr_param.a_pid == pid)
    {
        return(1);
    }

    if (dmx_see->usr_param.v_pid == pid)
    {
        return(2);
    }

    if (dmx_see->subt_pid == pid)
    {
        return(3);
    }

    if (dmx_see->ttx_pid == pid)
    {
        return(4);
    }

    return(-1);
}





__s32 dmx_see_pid_to_audio_type
(
    UINT16                pid, 
    enum AudioStreamType *audio_format, 
    enum STREAM_TYPE     *pes_type
)
{
    if(AUDIO_TYPE_TEST(pid, AC3_DES_EXIST))
    {
        *audio_format  = AUDIO_AC3;
        *pes_type = PRIV_STR_1;
    }
    else if(AUDIO_TYPE_TEST(pid, EAC3_DES_EXIST))
    {
        *audio_format  = AUDIO_EC3;
        *pes_type = PRIV_STR_1;
    }
    else if(AUDIO_TYPE_TEST(pid, AAC_DES_EXIST))
    {
        *audio_format  = AUDIO_MPEG_AAC;
        *pes_type = PRIV_STR_1;
    }   
    else if(AUDIO_TYPE_TEST(pid, ADTS_AAC_DES_EXIST))
    {
        *audio_format  = AUDIO_MPEG_ADTS_AAC;
        *pes_type = PRIV_STR_1;
    }
    else
    {
        *audio_format = AUDIO_MPEG2;
        *pes_type = AUDIO_STR;
    }   

    return(0);
}




__s32 dmx_see_av_start_param
(
    struct pes_pkt_param *pes_param,
    struct control_block *ctrl_blk
)
{
    memset(pes_param, 0, sizeof(struct pes_pkt_param));

    pes_param->av_buf = (void *)0xFFFFFFFF;

    pes_param->conti_conter = 0xFF;

    pes_param->ctrl_blk = ctrl_blk;

    pes_param->last_dma_xfer_id = 0XFFFFFFFF;
    
    pes_param->ch_num = 0xFF;

    memset(ctrl_blk, 0, sizeof(struct control_block));

    return(0);
}








__s32 dmx_see_av_start
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    __s32                   ret;
    void                   *decv;
    struct deca_device     *deca;
    struct snd_device      *snd_device;
    struct dmx_see_av_pid_info *usr_param;
    struct pes_pkt_param   *a_pes_para;
    struct pes_pkt_param   *v_pes_para;
    struct pes_pkt_param   *ad_pes_para;
    enum AudioStreamType    a_format;
    enum STREAM_TYPE        a_pes_type;

    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (dmx_see->status != DMX_SEE_STATUS_READY)
    {
        if(dmx_see->status == DMX_SEE_STATUS_PAUSE)
        {
            set_sed_parse_status(1);
            dmx_see->status = DMX_SEE_STATUS_RUN;
            return 0;
        }
        return(-EPERM);        
    }

    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    usr_param = &dmx_see->usr_param;

    ret = copy_from_user(usr_param, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    /* Make sure SEE dmx buf is empty. */
    dmx_see->see_buf_init.main2see_buf->wr = dmx_see->see_buf_init.main2see_buf->rd;


    /* Set param for sed_play_channel(). */

    v_pes_para = &(dmx_see->pes_para[0]);

#if 0
    if (v_pes_para->device != NULL)
    {
        vdec_io_control(v_pes_para->device, VDEC_SET_DMA_CHANNEL, 0);
    }
#endif

    dmx_see_av_start_param(v_pes_para, &(dmx_see->ctrl_blk[0]));

    v_pes_para->xfer_es_by_dma = 1;

    if(usr_param->v_pid & 0x2000)
    {
        decv = hld_dev_get_by_name("DECV_AVC_0");
    }
	else if(usr_param->v_pid & 0x4000)
    {
        decv = hld_dev_get_by_name("DECV_AVS_0");
    }
    else
    {
        decv = hld_dev_get_by_type(NULL, HLD_DEV_TYPE_DECV);
    }

    v_pes_para->device = decv;
    
    DMX_SEE_DEBUG("%s,%d,decv:%x\n", __FUNCTION__, __LINE__, (DMX_UINT32)decv);

    v_pes_para->stream_type = VIDEO_STR; 

    v_pes_para->ch_num = 0;

 
    /* Audio PES stream param. */
    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);
    deca = (struct deca_device *)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_DECA); 
    snd_device = (struct snd_device *)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_SND);

    a_pes_para = &(dmx_see->pes_para[1]);

    dmx_see_av_start_param(a_pes_para, &(dmx_see->ctrl_blk[1])); 

    a_pes_para->xfer_es_by_dma = 0;


    a_pes_para->device = deca;   

    DMX_SEE_DEBUG("%s,%d,decv:%x\n", __FUNCTION__, __LINE__, (DMX_UINT32)deca);

    if((usr_param->a_pid & 0x1FFF) != 0x1FFF)
    {
        dmx_see_pid_to_audio_type(usr_param->a_pid, &a_format, &a_pes_type);
        
        a_pes_para->stream_type = a_pes_type;

        deca_io_control(deca, DECA_SET_STR_TYPE, a_format);
    }

    a_pes_para->ch_num = 1;

    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    ad_pes_para = &(dmx_see->pes_para[5]);

    dmx_see_av_start_param(ad_pes_para, &(dmx_see->ctrl_blk[5])); 

    ad_pes_para->xfer_es_by_dma = 0;

    ad_pes_para->device = deca;   

    deca_io_control(deca, DECA_INDEPENDENT_DESC_ENABLE, ADEC_DESC_CHANNEL_DISABLE);
    snd_io_control(snd_device, SND_SECOND_DECA_ENABLE, 0);

    if(usr_param->ad_pid != 0x1FFF)
    {
        deca_io_control(deca, DECA_INDEPENDENT_DESC_ENABLE, ADEC_DESC_CHANNEL_ENABLE);
    }

    ad_pes_para->ch_num = 2;
    
#if 1
    //stc_invalid();

    //deca_set_sync_mode(a_pes_para->device, ADEC_SYNC_PTS);

    sed_set_pcr_pid(usr_param->p_pid & 0x1FFF);
#if 0
    unsigned int dmx_base_addr[4]={
        DMX0_BASE_ADDR,
        DMX1_BASE_ADDR,
        0,
        DMX3_BASE_ADDR,
    };
#endif
#endif

    DMX_SEE_DEBUG("%s, %d,v:%d,a:%d\n", __FUNCTION__, __LINE__, 
                  usr_param->v_pid, usr_param->a_pid);

    /* Control SEE dmx. */
#if 0
    sed_play_channel(usr_param->v_pid, usr_param->a_pid, v_pes_para,
                     a_pes_para);
#else

    sed_play_channel(usr_param->v_pid, usr_param->a_pid, usr_param->ad_pid, v_pes_para, 
                     a_pes_para, &(dmx_see->pes_para[5]));
#endif

    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    //vdec_start(dmx_see->pes_para[0].device);

    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    //sed_set_pcr_pid(usr_param->p_pid);

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    set_sed_parse_status(1);

    dmx_see->status = DMX_SEE_STATUS_RUN;


    //sgdma_test_start();

    return(0);
}

__s32 dmx_see_av_stop
(
    struct dmx_see_device *dmx_see,
    UINT32 param
)
{
    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    if (dmx_see->status != DMX_SEE_STATUS_RUN)
    {
        DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(0);        
    }



    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);


#if 0
    //sed_reset_pes_param(dmx_see->pes_para[0].stream_type, FALSE);

    //sed_reset_pes_param(dmx_see->pes_para[1].stream_type, FALSE);
#endif

    set_sed_parse_status(0);

#if 0
    memset(vobu_start_pakcet, 0, sizeof(vobu_start_pakcet));

    vobu_start_pakcet[0] = 0xAA;

    vobu_start_pakcet[1] = 0x0;

    pkt_inf.pkt_addr = vobu_start_pakcet;

    dmx_see_buf_wr_ts(NULL, &pkt_inf, (DMX_UINT32)dmx_see);
#endif

#if 1
    /* dmx_see->usr_param.v_pid == 0x1FFF means see is playing radio only
     * program now.
     */
    if (dmx_see->usr_param.v_pid != 0x1FFF)
    {
        //vdec_stop(dmx_see->pes_para[0].device, 0, 0);
        if(param != 0xFF /*DMA_INVALID_CHA*/)
        vdec_io_control(dmx_see->pes_para[0].device, VDEC_SET_DMA_CHANNEL,
                        0xFF/*DMA_INVALID_CHA*/);
    }
#endif

#if 0
    stc_invalid();
#endif

    dmx_see->status = DMX_SEE_STATUS_READY;

    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);
}



__s32 dmx_see_a_change_pid
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    __s32                       ret;
    struct dmx_see_av_pid_info *usr_param;
    struct dmx_see_av_pid_info  see_av_pid_info;
    struct pes_pkt_param       *a_pes_para;
    struct pes_pkt_param       *ad_pes_para;
    struct deca_device         *deca;
    struct snd_device          *snd_device;
    enum AudioStreamType        a_format;
    enum STREAM_TYPE            a_pes_type;

    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

#if 1
#if 0
    if (dmx_see->status != DMX_SEE_STATUS_READY)
    {
        return(-EPERM);        
    }
#endif

    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    usr_param = &dmx_see->usr_param;

    ret = copy_from_user(&see_av_pid_info, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    usr_param->a_pid = see_av_pid_info.a_pid;
    usr_param->ad_pid = see_av_pid_info.ad_pid;
    /* Audio PES stream param. */
    DMX_SEE_DEBUG("%s,%d,pid:%d\n", __FUNCTION__, __LINE__, pid);

    a_pes_para = &(dmx_see->pes_para[1]);

    dmx_see_av_start_param(a_pes_para, &(dmx_see->ctrl_blk[1])); 

    a_pes_para->xfer_es_by_dma = 0;

    deca = (struct deca_device *)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_DECA); 

    a_pes_para->device = deca;   

    DMX_SEE_DEBUG("%s,%d,decv:%x\n", __FUNCTION__, __LINE__, (DMX_UINT32)deca);

    if(usr_param->a_pid != 0x1FFF)
    {
        dmx_see_pid_to_audio_type(usr_param->a_pid, &a_format, &a_pes_type);
        
        a_pes_para->stream_type = a_pes_type;

        deca_io_control(deca, DECA_SET_STR_TYPE, a_format);
    }

    a_pes_para->ch_num = 1;

    ad_pes_para = &(dmx_see->pes_para[5]);

    dmx_see_av_start_param(ad_pes_para, &(dmx_see->ctrl_blk[5])); 

    ad_pes_para->xfer_es_by_dma = 0;

    deca = (struct deca_device *)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_DECA); 

    ad_pes_para->device = deca;   

    DMX_SEE_DEBUG("%s,%d,decv:%x\n", __FUNCTION__, __LINE__, (DMX_UINT32)deca);

    if(usr_param->ad_pid != 0x1FFF)
    {
        deca_io_control(deca, DECA_INDEPENDENT_DESC_ENABLE, ADEC_DESC_CHANNEL_ENABLE);
        ad_pes_para->ch_num = 2;

    }
    else
    {
        snd_device = (struct snd_device *)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_SND);
        deca_io_control(deca, DECA_INDEPENDENT_DESC_ENABLE, ADEC_DESC_CHANNEL_DISABLE);
        snd_io_control(snd_device, SND_SECOND_DECA_ENABLE, 0);
    }
#if 0
    sed_change_audio_channel(pid, a_pes_para);
#else
    sed_change_audio_channel(see_av_pid_info.a_pid, see_av_pid_info.ad_pid, a_pes_para, ad_pes_para);    
#endif

    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    //set_sed_parse_status(1);
#endif

    return(0);
}



__s32 dmx_see_teletext_start
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    __s32               ret;
    struct pes_pkt_param   *pes_para;
    UINT16                 *pid;

    pid = &dmx_see->ttx_pid;

    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(pid, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    pes_para = &(dmx_see->pes_para[3]);

    pes_para->device = hld_dev_get_by_type(NULL, HLD_DEV_TYPE_VBI);
    //pes_para->request_write = reg_serv->request_write;
    //pes_para->update_write = reg_serv->update_write;

    pes_para->ch_num = 3;

    pes_para->xfer_es_by_dma = FALSE;

    //pes_para->dma_ch_num = 0xFF;/*DMA_INVALID_CHA;*/

#if 0
    pes_para->stream_type = reg_serv->str_type;

    if(reg_serv->service_pid & 0x2000) //ac3
        pes_param->stream_type=PRIV_STR_1;//infact is private audio
#endif

  sed_reg_serv((*pid) & 0x1FFF, 3, pes_para);   

  return(0);
}




__s32 dmx_see_subtitle_start
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    __s32               ret;
    struct pes_pkt_param   *pes_para;
    UINT16                 *pid;

    pid = &dmx_see->subt_pid;

    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(pid, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    pes_para = &(dmx_see->pes_para[4]);

    pes_para->device = hld_dev_get_by_type(NULL, HLD_DEV_TYPE_SDEC);
    //pes_para->request_write = reg_serv->request_write;
    //pes_para->update_write = reg_serv->update_write;

    pes_para->ch_num = 4;

    pes_para->xfer_es_by_dma = FALSE;

    //pes_para->dma_ch_num = 0xFF;/*DMA_INVALID_CHA;*/

#if 0
    pes_para->stream_type = reg_serv->str_type;

    if(reg_serv->service_pid & 0x2000) //ac3
        pes_param->stream_type=PRIV_STR_1;//infact is private audio
#endif

    sed_reg_serv((*pid) & 0x1FFF, 4, pes_para);   

    return(0);
}


__s32 dmx_see_dcii_subtitle_start
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    __s32                 ret;
    struct pes_pkt_param *pes_para;
    UINT16               *pid;

    pid = &dmx_see->dcii_subt_pid;

    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(pid, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    pes_para = &(dmx_see->pes_para[6]);

    pes_para->device = hld_dev_get_by_type(NULL, HLD_DEV_TYPE_SDEC);
    //pes_para->request_write = reg_serv->request_write;
    //pes_para->update_write = reg_serv->update_write;

    pes_para->ch_num = 6;

    pes_para->xfer_es_by_dma = FALSE;

    //pes_para->dma_ch_num = 0xFF;/*DMA_INVALID_CHA;*/

#if 0
    pes_para->stream_type = reg_serv->str_type;

    if(reg_serv->service_pid & 0x2000) //ac3
        pes_param->stream_type=PRIV_STR_1;//infact is private audio
#endif
    pes_para->stream_type = DC2SUB_STR;
    sed_reg_serv((*pid) & 0x1FFF, 6, pes_para);   

    return(0);
}



__s32 dmx_see_crypto_type_set
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    __s32            ret;
    struct dec_parse_param p_param;
    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&p_param, (void __user *)arg, sizeof(struct dec_parse_param));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }
    
    sed_set_dev(p_param.dec_dev , (UINT32)(p_param.type));  //1: AES, 0: DES
    
    return(0);
}


__s32 dmx_see_crypto_start
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    sed_set_decrypt_status(1);

    g_see_crypto_started = 1;

    return(0);
}


__s32 dmx_see_crypto_stop
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    sed_set_decrypt_status(0);

    g_see_crypto_started = 0;

    return(0);
}


__s32 dmx_see_av_sync_mode_set
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
#if 1
    __s32 ret;

    ret = copy_from_user(&dmx_see->av_sync_mode, (void __user *)arg,
                         _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);
    }

    DMX_SEE_DEBUG("%s,%d,mode:%d\n", __FUNCTION__, __LINE__, 
                  dmx_see->av_sync_mode);

    if (DMX_SEE_AV_SYNC_MODE_LIVE == dmx_see->av_sync_mode)
    {
        sed_reset_to_live_mode(0);
    }
    else
    {
        sed_set_av_mode(dmx_see->av_sync_mode);
    }
#endif

    return(0);
}






__s32 dmx_see_buf_wr_ts
(
    struct dmx_ts_pkt_inf *pkt_inf,
    __u32                  param
)
{
    struct dmx_see_device                *dmx_see;
    __u32                                 wr;
    __u32                                 next_wr;    
    volatile struct dmx_see_raw_buf_info *main2see_buf;
    UINT8                                *main2see_buf_addr;
    //__u32                                 dmx_id;
    //__u32                                 rd;

    //dmx_id = param;

    //DMX_SEE_DEBUG("%s, %d, param:%x\n", __FUNCTION__, __LINE__, param);

    dmx_see = &ali_dmx_see_dev[0];

    /* Do not send same TS pakcet to SEE more than once, this could happen to
     * PCR service.
     * Do not compare first byte because it is always 0x47.
     */
    if ((dmx_see->last_ts_hdr[1] == pkt_inf->pkt_addr[1]) &&
        (dmx_see->last_ts_hdr[2] == pkt_inf->pkt_addr[2]) &&
        (dmx_see->last_ts_hdr[3] == pkt_inf->pkt_addr[3]))
    {
        static __u32 g_v_drop_cnt = 0;
        
        g_v_drop_cnt++;
 
        if ((g_v_drop_cnt & 0xFFF) == 500)
        {
            //DMX_SEE_DEBUG("g_v_drop_cnt:%d\n", g_v_drop_cnt);
        }   

        /* PVR.
        */
        if (1 == dmx_see->is_last_ts_resend)
        {
            dmx_see->is_last_ts_resend = 0;
        }
        else
        {
            return(0);
        }
    }
    
    dmx_see->last_ts_hdr[1] = pkt_inf->pkt_addr[1];
    dmx_see->last_ts_hdr[2] = pkt_inf->pkt_addr[2];
    dmx_see->last_ts_hdr[3] = pkt_inf->pkt_addr[3];

    main2see_buf = dmx_see->see_buf_init.main2see_buf;

    wr = main2see_buf->wr;

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    /* Buf full detection.
     * next_wr is used for buf full detection.
     */
    next_wr = wr + 188;

    if (next_wr >= DMX_SEE_BUF_SIZE)
    {
        next_wr = 0;

        //DMX_SEE_DEBUG("%s, %d,r:%d,w:%d\n", __FUNCTION__, __LINE__, rd, wr);
    }


#if 0
    for (;;)
    {
        rd = main2see_buf->rd;

        /* Main to SEE buffer not full.
        */
        if (next_wr != rd)
        {
            //DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);
        
            //return(DMX_ERR_BUF_BUSY);
            break;
        }

        //DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

#if 0
        if (dmx_mutex_output_unlock(dmx_id))
        {
            //DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);
        
            return(0);
        }           

        msleep(10);

        dmx_mutex_output_lock(dmx_id);
#else
        msleep(10);
#endif
    }
#else    
    /* Main to SEE buffer full.
    */
    if (next_wr == main2see_buf->rd)
    {
        //DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);
        dmx_see->is_last_ts_resend = 1;
        
        return(DMX_ERR_BUF_BUSY);
    }
#endif


    /* Got room to put TS packet, copy it.
    */
    main2see_buf_addr = (UINT8 *)main2see_buf->buf;

	/* Change from SEE MIPS address to main ARM address.(0xA0000000 -> 0xCXXXXXXX)
	 * Since this address is shared by MAIN and SEE, SEE is running on MIPS, but MIAN may run on 
	 * ARM or MIPS, so we should change ARM address to SEE adress on the main side in the case of
	 * main is running on ARM. 
	*/
	#ifdef CONFIG_ARM
    main2see_buf_addr += 0x20000000;
	#endif

    //printk("%s,%d,buf:%x,main2see_buf_addr:%x,pkt_inf->pkt_addr:%x\n", __FUNCTION__, __LINE__, main2see_buf->buf, main2see_buf_addr, pkt_inf->pkt_addr);

    memcpy(main2see_buf_addr + wr, pkt_inf->pkt_addr, 188);

    //printk("%s,%d,buf:%x,main2see_buf_addr:%x,pkt_inf->pkt_addr:%x\n", __FUNCTION__, __LINE__, main2see_buf->buf, main2see_buf_addr, pkt_inf->pkt_addr);

    main2see_buf->wr = next_wr;

    //printk("%s,%d,buf:%x,main2see_buf_addr:%x,pkt_inf->pkt_addr:%x,main2see_buf->wr:%d\n", __FUNCTION__, __LINE__, main2see_buf->buf, main2see_buf_addr, pkt_inf->pkt_addr, main2see_buf->wr);

#if 0
    static __u32 g_dmx_see_ts_wr_cnt = 0;

    g_dmx_see_ts_wr_cnt++;

    if ((g_dmx_see_ts_wr_cnt & 0xFFF) == 500)
    {
        DMX_SEE_DEBUG("%d\n", g_dmx_see_ts_wr_cnt);
    }
#endif  

#if 0
    if (2231 == pkt_inf->pid)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    }
#endif

    return(0);
}




__s32 dmx_see_av_scram_status
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    __u8  *scram_status;
    __s32  ret;

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    scram_status = &dmx_see->av_scram_status;

    sed_dmx_is_av_scrambled_old(scram_status);

    ret = copy_to_user((void __user *)arg, scram_status, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_to_user() failed, ret:%d\n", ret);

        ret = -ENOTTY;
    }

    //DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    return(ret);
}




__s32 dmx_see_av_scram_status_ext
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    struct dmx_see_av_scram_info *scram_param_linux;
    struct io_param_ex           *scram_param_tds;
    __s32                        ret;

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    scram_param_linux = &dmx_see->scram_param_linux;

    scram_param_tds = &dmx_see->scram_param_tds;

    ret = copy_from_user(scram_param_linux, (void __user *)arg,
                         _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    memset(scram_param_tds, 0, sizeof(struct dmx_see_av_scram_info));

    scram_param_tds->io_buff_in = (__u8 *)(&(scram_param_linux->pid[0]));

    scram_param_tds->io_buff_out = &(scram_param_linux->scram_status);
        scram_param_linux->scram_status = 0;

    sed_m36_is_av_scrambled(scram_param_tds);

    ret = copy_to_user((void __user *)arg, scram_param_linux, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_to_user() failed, ret:%d\n", ret);

        ret = -ENOTTY;
    }

    //DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    return(ret);
}



__s32 dmx_see_get_statistics
(
    struct Ali_DmxSeeStatistics *statistics
)
{
    memcpy(statistics, (void *)ali_dmx_see_dev[0].see_buf_init.statistics,
           sizeof(struct Ali_DmxSeeStatistics));
    
    return(0);
}


__s32 dmx_see_legacy_discon_cnt_get
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    struct Ali_DmxSeeStatistics see_stat;
	__u32                       discont;
	__s32                       ret;

	dmx_see_get_statistics(&see_stat);

	discont = (see_stat.AudioTsLostCnt + see_stat.VideoTsLostCnt) - ali_dmx_see_dev[0].last_chk_see_discon_cnt;
	
    ret = copy_to_user((void __user *)arg, &discont, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        printk("copy_to_user() failed, ret:%d\n", ret);

        ret = -ENOTTY;
    }

    return(0);
}


__s32 dmx_see_legacy_discon_cnt_clean
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    struct Ali_DmxSeeStatistics see_stat;

	dmx_see_get_statistics(&see_stat);

	ali_dmx_see_dev[0].last_chk_see_discon_cnt = (see_stat.AudioTsLostCnt + see_stat.VideoTsLostCnt);
	
    return(0);
}


__s32 dmx_see_legacy_get_dmx_buffer_remain_data_len
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    
    volatile struct dmx_see_raw_buf_info *see_buf_info;
    __u32                                 rd;
    __u32                                 wr;
    __u32                                 nDataInBuffer = 0;
    __u32                                 ret = 0;
	
    see_buf_info = dmx_see->see_buf_init.main2see_buf;
    
    wr = see_buf_info->wr;
    rd = see_buf_info->rd;

    if(wr>=rd)
    {
        nDataInBuffer = wr - rd;
    }
    else
    {
        nDataInBuffer = DMX_SEE_BUF_SIZE - rd + wr;
        
        
    }
    DMX_SEE_DEBUG("nDataInBuffer=%d\r\n",nDataInBuffer);

    ret = copy_to_user((void __user *)arg, &nDataInBuffer, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_to_user() failed, ret:%d\n", ret);

        ret = -ENOTTY;
    }
    return 0;
}


__s32 dmx_see_set_pcr
(
    __u32 pcr
)
{
    ali_dmx_see_dev[0].see_buf_init.statistics->Pcr = pcr;

    return(0);
}
__s32 dmx_see_release
(
    struct inode *inode,
    struct file  *file
)
{
    __s32              ret;
    struct dmx_see_device *dmx_see;

	DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    dmx_see = file->private_data;

    //if (dmx_see->status == DMX_SEE_STATUS_IDLE)
    //{
    //    return(-EPERM);        
    //}

    ret = 0;

    if (mutex_lock_interruptible(&dmx_see->mutex))
    {
        return(-ERESTARTSYS);
    }

    dmx_see_av_stop(dmx_see, 0);

    dmx_see->status = DMX_SEE_STATUS_IDLE;

    file->private_data = NULL;

    mutex_unlock(&dmx_see->mutex);

	DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    return(ret);
}







__s32 dmx_see_open
(
    struct inode *inode,
    struct file  *file
)
{
    __s32                  ret;
    struct dmx_see_device *dmx_see;

	DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    dmx_see = container_of(inode->i_cdev, struct dmx_see_device, cdev);

    ret = 0;

    if (dmx_see->status != DMX_SEE_STATUS_IDLE)
    {
        ret = (-EMFILE);
    }

    if (mutex_lock_interruptible(&dmx_see->mutex))
    {
        return(-ERESTARTSYS);
    }

    file->private_data = dmx_see;

    dmx_see->status = DMX_SEE_STATUS_READY;

    mutex_unlock(&dmx_see->mutex);

	DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    return(ret);
}


__s32 dmx_see_video_stream_start
(
    __u32 pid
)
{
    ali_dmx_see_dev[0].video_pid = pid;

    sed_dmx_video_stream_start(pid);

    return(0);
}



__s32 dmx_see_video_stream_stop
(
    __u32 dummy
)
{
    ali_dmx_see_dev[0].video_pid = 0xFFFFFFFF;

    sed_dmx_video_stream_stop(0);

    return(0);
}



__s32 dmx_see_audio_stream_start
(
    __u32 pid
)
{
    ali_dmx_see_dev[0].audio_pid = pid;

    sed_dmx_audio_stream_start(pid);

    return(0);
}



__s32 dmx_see_audio_stream_stop
(
    __u32 dummy
)
{
    ali_dmx_see_dev[0].audio_pid = 0xFFFFFFFF;

    sed_dmx_audio_stream_stop(0);

    return(0);
}



__s32 dmx_see_ttx_stream_start
(
    __u32 pid
)
{
    sed_dmx_ttx_stream_start(pid);

    return(0);
}


__s32 dmx_see_ttx_stream_stop
(
    void
)
{
    sed_dmx_ttx_stream_stop(0);

    return(0);
}


__s32 dmx_see_subt_stream_start
(
    __u32 pid
)
{
    sed_dmx_subt_stream_start(pid);

    return(0);
}


__s32 dmx_see_subt_stream_stop
(
    void
)
{
    sed_dmx_subt_stream_stop(0);

    return(0);
}




__s32 dmx_see_get_dmx_buffer_remain_data_len
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    volatile struct dmx_see_raw_buf_info *main2see_buf;
    __u32                                 rd;
    __u32                                 wr;
    __u32                                 nDataInBuffer = 0;
    __s32                                 ret = 0;

    main2see_buf = dmx_see->see_buf_init.main2see_buf;
    
    wr = main2see_buf->wr;
    rd = main2see_buf->rd;

    if(wr > rd)
    {
        nDataInBuffer = wr - rd;
    }
    else
    {
        nDataInBuffer = DMX_SEE_BUF_SIZE - rd + wr;
    }
	
    DMX_SEE_DEBUG("nDataInBuffer=%d\r\n",nDataInBuffer);

    ret = copy_to_user((void __user *)arg, &nDataInBuffer, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_to_user() failed, ret:%d\n", ret);

        ret = -ENOTTY;
    }
	
    return 0;
}



#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long dmx_see_ioctl
(
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#else
__s32 dmx_see_ioctl
(
    struct inode *inode,
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#endif
{
    __s32                  ret;
    struct dmx_see_device *dmx_see;
    struct dmx_ts_kern_recv_info ts_kern_recv_info;
	unsigned long speed;

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = 0;

    dmx_see = filp->private_data;

    switch(cmd)
    {
        case ALI_DMX_SEE_AV_START:
        {
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

            ret = dmx_see_av_start(dmx_see, cmd, arg);

            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

            mutex_unlock(&dmx_see->mutex);
        }
        break;

        case ALI_DMX_SEE_AV_STOP:
        {
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
			
            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_see_av_stop(dmx_see, arg);

            mutex_unlock(&dmx_see->mutex);
        }
        break;

#if 0
        case ALI_DMX_SEE_AV_SCRAMBLE_STATUS:
        {
            ret = dmx_see_av_scram_status(dmx_see, cmd, arg);
        }
        break;
#endif

        case ALI_DMX_SEE_AV_SCRAMBLE_STATUS_EXT:
        {

            //printk("%s, %d\n", __FUNCTION__, __LINE__);
			
            //ret = dmx_see_av_scram_status_ext(dmx_see, cmd, arg);
            ret = -1;
        }
        break;

        case ALI_DMX_SEE_GET_TS_INPUT_ROUTINE:
        {
            ts_kern_recv_info.kern_recv_routine = (__u32)dmx_see_buf_wr_ts;

            ts_kern_recv_info.kern_recv_routine_para = 
                (__u32)(&(ali_dmx_see_dev[0]));

            ret = copy_to_user((void __user *)arg, &ts_kern_recv_info,
                               _IOC_SIZE(cmd));


            DMX_SEE_DEBUG("%s, %d, routine:%x, para:%x\n",
                          __FUNCTION__, __LINE__, 
                          ts_kern_recv_info.kern_recv_routine, 
                          ts_kern_recv_info.kern_recv_routine_para);


            if (0 != ret)
            {
                DMX_SEE_DEBUG("copy_to_user() failed, ret:%d\n", ret);

                ret = -ENOTTY;
            }
        }
        break;

        case ALI_DMX_SEE_A_CHANGE_PID:
        {
            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_see_a_change_pid(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
        }
        break;

        case ALI_DMX_SEE_TELETXT_START:
        {
            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_see_teletext_start(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
        }
        break;

        case ALI_DMX_SEE_SUBTITLE_START:
        {
            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_see_subtitle_start(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
        }
        break;

        case ALI_DMX_SEE_AV_SYNC_MODE_SET:
        {
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            
            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_see_av_sync_mode_set(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
        }
        break;


        case ALI_DMX_SEE_CRYPTO_TYPE_SET:
        {
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_see_crypto_type_set(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
			
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        }
        break;

        case ALI_DMX_SEE_CRYPTO_START:
        {
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_see_crypto_start(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
			
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        }
        break;

        case ALI_DMX_SEE_CRYPTO_STOP:
        {
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_see_crypto_stop(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);

            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        }
        break;

        case ALI_DMX_SEE_SET_PLAYBACK_SPEED:
        {
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            
            ret = copy_from_user(&speed, (void __user *)arg, _IOC_SIZE(cmd));
            if (0 != ret)
                return(-EFAULT);
            if (mutex_lock_interruptible(&dmx_see->mutex))
                  return(-ERESTARTSYS);
            sed_set_playback_speed(speed);
            mutex_unlock(&dmx_see->mutex);
			
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            
            return RET_SUCCESS;        
        }
        break;

		/* Waring: Not in as_dmx, why comes this command?
		*/
        case ALI_DMX_SEE_SET_UPDATE_REMAIN:
        {
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            if (mutex_lock_interruptible(&dmx_see->mutex))
                  return(-ERESTARTSYS);
            see_dmx_update_remain_data(arg);
            mutex_unlock(&dmx_see->mutex);
			
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            return RET_SUCCESS;        
        }
        
        break;

        /* TODO: maybe do not need implement, just ignoring it?
		*/
        case ALI_DMX_SEE_PLAYBACK_PAUSE:
        {
#if 0			
            if (mutex_lock_interruptible(&dmx_see->mutex))
    	          return(-ERESTARTSYS);
            //printk("");
            set_sed_parse_status(2);
            //see_dmx_update_remain_data(1);
            //dmx_see->status = DMX_SEE_STATUS_PAUSE;
            mutex_unlock(&dmx_see->mutex);
#endif
            return RET_SUCCESS;        
        }
        break;

        /* TODO: maybe do not need implement, just ignoring it?
		*/
		case ALI_DMX_SEE_PLAYBACK_PRE_PAUSE:
        {
            return RET_SUCCESS;        
        }		

        case ALI_DMX_SEE_GET_DISCONTINUE_COUNT:
        {	
            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
              return(-ERESTARTSYS);
			}
			
            ret = dmx_see_legacy_discon_cnt_get(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);			
        }
        break;

        case ALI_DMX_SEE_CLEAR_DISCONTINUE_COUNT:
        {   
            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
              return(-ERESTARTSYS);
			}
			
            ret = dmx_see_legacy_discon_cnt_clean(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
		}
        break;		

        case ALI_DMX_SEE_GET_MAIN2SEEBUF_REAMIN_DATA_LEN:
        {   
            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
              return(-ERESTARTSYS);
			}

            dmx_see_legacy_get_dmx_buffer_remain_data_len(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
			
            return RET_SUCCESS;  
        }
        break;

        /* TODO: control audio, need merge see code first.
		*/
        case ALI_DMX_SEE_DO_DDP_CERTIFICATION:
        {
            return RET_SUCCESS;  
        }
        break;	

        /* TODO: need merge see code first.
		*/
        case ALI_DMX_SEE_SET_SAT2IP:
        {
            return RET_SUCCESS;  
        }
        break;

        #if 0
        /* TODO: need merge see code first.
		*/
        case ALI_DMX_SEE_SET_SEE2MAIN:
        {
            return RET_SUCCESS;  
        }
        break;		
		#endif

        case ALI_DMX_SEE_DCII_SUBT_START:
        {
            if (mutex_lock_interruptible(&dmx_see->mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = dmx_see_dcii_subtitle_start(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
        }
        break;
		
        default:
        {
            //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            ret = -EINVAL;
        }
        break;
    }

	DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    return(ret);

}


__u32 dmx_see_see2main_buf_rd_get
(
    __u32 dev_idx
)
{
    return(ali_dmx_see_dev[dev_idx].see2main_buf_info->rd);
}



__u32 dmx_see_see2main_buf_rd_set
(
    __u32 dev_idx,
    __u32 rd
)
{
    ali_dmx_see_dev[dev_idx].see2main_buf_info->rd = rd;

	return(0);
}



__u32 dmx_see_see2main_buf_wr_get
(
    __u32 dev_idx
)
{
    return(ali_dmx_see_dev[dev_idx].see2main_buf_info->wr);
}


__s32 dmx_see_see2main_buf_wr_set
(
    __u32 dev_idx,
    __u32 wr
)
{
    return(DMX_ERR_HW_NOT_PERMIT);
}


__u32 dmx_see_see2main_buf_end_idx_get
(
    __u32 dev_idx
)
{
    return((DMX_SEE_BUF_SIZE / 188) - 1);
}

__u32 dmx_see_see2main_buf_start_addr_get
(
    __u32 dev_idx
)
{
#ifdef CONFIG_ARM
	return((__u32)(ali_dmx_see_dev[dev_idx].see2main_buf_info->buf + 0x20000000));
#else
    return((__u32)(ali_dmx_see_dev[dev_idx].see2main_buf_info->buf));
#endif
}



extern __u32 __G_SEE_DMX_SRC_BUF_START;

extern __u32 __G_SEE_DMX_DECRYPTO_BUF_START;



#if 0
__s32 dmx_see_init
(
    void
)
{
    __s32                               result;
    struct device                      *clsdev;
    struct dmx_see_device              *dmx_see;
	__u32                               size_order;
    __u32                               cfg_buf_len;
    volatile struct dmx_see_init_param *see_buf_init;
	__u32                               main2see_buf_addr;
	__u32                               see_decrypt_buf_addr;
	__u32                               see2main_buf_addr;

    result = 0;

   return 0;
   
    dmx_see = &(ali_dmx_see_dev[0]);

	//sed_dmx_attach(0);

    /* Get memory for SEE init param.
     */  
    see_buf_init = &dmx_see->see_buf_init;

    memset((void *)see_buf_init, 0, sizeof(struct dmx_see_init_param));

    /* Get memory for MAIN to SEE buffer.
     */    
    see_buf_init->main2see_buf = ali_rpc_malloc_shared_mm(sizeof(struct dmx_see_raw_buf_info));

    if (NULL == see_buf_init->main2see_buf)
    {
        DMX_SEE_DEBUG("%s, %d, ali_rpc_malloc_shared_mm() failed.\n",
                __FUNCTION__, __LINE__);

        return(-1);
    }

    see_buf_init->main2see_buf->rd = 0;

    see_buf_init->main2see_buf->wr = 0;


    cfg_buf_len = __G_ALI_MM_DMX_MEM_TOP_ADDR - __G_ALI_MM_DMX_MEM_START_ADDR;
    
    /* DMX buffer configration validation.
     * 2 ^ 16 = 65536 unit of TS pakcet. 
    */
    if (cfg_buf_len >= ((65536 * 188) + (3 * DMX_SEE_BUF_SIZE)))
    {
    	panic("%s,%d,DMX Buffer configration error,len:%u\n",
    		  __FUNCTION__, __LINE__, cfg_buf_len);
    }
    /* Old way to configure DMX buffer.
     * DMX HW buffers are configured at system memory map,
     * main2see buffer, see2main buffer, see decrypt buffer are dynamicly
     * allocated at driver initilization.
     */
    else if (cfg_buf_len < ((2 * 1024 * 1024) + (3 * DMX_SEE_BUF_SIZE)))
    {
        size_order = get_order(DMX_SEE_BUF_SIZE);
		
		main2see_buf_addr = (__u32)__get_free_pages(GFP_KERNEL, size_order);
    
        if (!main2see_buf_addr)
        {
        	DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);
        
        	return(-ENOMEM);
        }

    	see_decrypt_buf_addr = (__u32)__get_free_pages(GFP_KERNEL, size_order);
    
        if (!see_decrypt_buf_addr)
        {
            kfree((void *)main2see_buf_addr);
			
        	DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);
        
        	return(-ENOMEM);
        }

        see2main_buf_addr = (__u32)__get_free_pages(GFP_KERNEL, size_order);
        
        if (!see2main_buf_addr)
        {
    		kfree((void *)main2see_buf_addr);
			
            kfree((void *)see_decrypt_buf_addr);
			
        	DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);
        
        	return(-ENOMEM);
        }	

		/* S3701C may have HW bug that osal_cache_invalidate() can be no affection,
		 * so we must give SEE an uncached address.
		 * This code could be write back as(main2see_buf->buf = (DMX_UINT32)buf_addr;),
		 *	if SEE bug is fixed.
		 */
		main2see_buf_addr &= 0x8FFFFFFF;

        main2see_buf_addr |= 0xA0000000;	
		

		see_decrypt_buf_addr &= 0x8FFFFFFF;

        see_decrypt_buf_addr |= 0xA0000000;	
		

		see2main_buf_addr &= 0x8FFFFFFF;

        see2main_buf_addr |= 0xA0000000;			
    }
    /* New way to configure DMX buffer to keep all buffer below 256M 
     * ram address.
     * All buffers, DMX HW buffers , main2see buffer, see2main buffer,
     * see decrypt buffer are configured at system memory map.
     */ 
    else
    {
		/* S3701C may have HW bug that osal_cache_invalidate() can be no affection,
		 * so we must give SEE an uncached address.
		 * This code could be write back as(main2see_buf->buf = (DMX_UINT32)buf_addr;),
		 *	if SEE bug is fixed.
		 */    
		main2see_buf_addr = (__G_ALI_MM_DMX_MEM_TOP_ADDR - DMX_SEE_BUF_SIZE) | 0xA0000000;

		see_decrypt_buf_addr = (main2see_buf_addr - DMX_SEE_BUF_SIZE) | 0xA0000000;

		see2main_buf_addr = (see_decrypt_buf_addr - DMX_SEE_BUF_SIZE) | 0xA0000000;
    }	

	see_buf_init->main2see_buf->buf = main2see_buf_addr;

    DMX_SEE_DEBUG("%s,%d,main2see_buf->buf:%x\n", __FUNCTION__, __LINE__, see_buf_init->main2see_buf->buf);

    see_buf_init->decrypt_buf = see_decrypt_buf_addr;
	
    see_buf_init->decrypt_buf_size = DMX_SEE_BUF_SIZE;

    /* statistics must live in share memory to allow main CPU know SEE DMX
     * statistics. 
     * Note share memory's size is less than 512 bytes, so 
     * struct dmx_see_statistics size should be keeped as small as possible.
     */
    see_buf_init->statistics = (struct Ali_DmxSeeStatistics *)ali_rpc_malloc_shared_mm(sizeof(struct Ali_DmxSeeStatistics));

    if (NULL == see_buf_init->statistics)
    {
        DMX_SEE_DEBUG("%s, %d, ali_rpc_malloc_shared_mm() failed.\n",
                __FUNCTION__, __LINE__);

        return(-1);
    }

    memset((void *)see_buf_init->statistics, 0, sizeof(struct Ali_DmxSeeStatistics));

    see_buf_init->GlobalStatInfo = (volatile struct Ali_DmxSeeGlobalStatInfo *)kmalloc(sizeof(struct Ali_DmxSeeGlobalStatInfo), GFP_KERNEL);

    if (NULL == see_buf_init->GlobalStatInfo)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        kfree((void *)see_buf_init->GlobalStatInfo);

        return(-1);
    }

    memset((void *)see_buf_init->GlobalStatInfo, 0, sizeof(struct Ali_DmxSeeGlobalStatInfo));

    see_buf_init->PlyChStatInfo = (volatile struct Ali_DmxSeePlyChStatInfo *)kmalloc(6 * sizeof(struct Ali_DmxSeePlyChStatInfo), GFP_KERNEL);

    if (NULL == see_buf_init->PlyChStatInfo)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        kfree((void *)see_buf_init->PlyChStatInfo);

        return(-1);
    }

    memset((void *)see_buf_init->PlyChStatInfo, 0, 6 * sizeof(struct Ali_DmxSeePlyChStatInfo));
	
    __CACHE_FLUSH_ALI((__u32)see_buf_init, sizeof(struct dmx_see_init_param));

    sed_set_dmx_parse_info((void *)see_buf_init);
	

    /* Set See2Main buffer.
	*/	
	dmx_see->see2main_buf_info = ali_rpc_malloc_shared_mm(sizeof(struct dmx_see_raw_buf_info));

    if (NULL == dmx_see->see2main_buf_info)
    {
        DMX_SEE_DEBUG("%s, %d, ali_rpc_malloc_shared_mm() failed.\n",
                __FUNCTION__, __LINE__);

        return(-1);
    }
	
	memset((void *)dmx_see->see2main_buf_info, 0, sizeof(struct dmx_see_raw_buf_info));

	dmx_see->see2main_buf_info->buf = see2main_buf_addr;

    __CACHE_FLUSH_ALI((__u32)(dmx_see->see2main_buf_info), sizeof(struct dmx_see_raw_buf_info));
	
	Sed_DmxSee2mainBufInfoSet((void *)(dmx_see->see2main_buf_info));

    dmx_see->usr_param.a_pid = 0x1FFF;

    dmx_see->usr_param.v_pid = 0x1FFF;

    dmx_see->usr_param.p_pid = 0x1FFF;
    dmx_see->usr_param.ad_pid = 0x1FFF;

    dmx_see->audio_pid = 0x1FFF;
    dmx_see->video_pid = 0x1FFF;
    dmx_see->pcr_pid = 0x1FFF;

    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    mutex_init(&(dmx_see->mutex));

    ali_dmx_see_interface_class = class_create(THIS_MODULE, 
                                               "ali_dmx_see_interface_class");
    
    if (IS_ERR(ali_dmx_see_interface_class))
    {            
        return(PTR_ERR(ali_dmx_see_interface_class));
    }

    result = alloc_chrdev_region(&dmx_see->dev_id, 0, 1, "ali_m36_dmx_see_0");

    if (result < 0) 
    {
        DMX_SEE_DEBUG("%s, alloc_chrdev_region() failed.\n", __FUNCTION__);

        goto fail;
    }

    DMX_SEE_DEBUG("%s, dev_id:%d.\n", __FUNCTION__, dmx_see->dev_id);

    cdev_init(&(dmx_see->cdev), &g_ali_m36_dmx_see_fops);

    dmx_see->cdev.owner = THIS_MODULE;

    result = cdev_add(&dmx_see->cdev, dmx_see->dev_id, 1);

    /* TODO: Fail gracefully if need be. 
    */
    if (result)
    {
        DMX_SEE_DEBUG("cdev_add() failed, result:%d\n", result);

        goto fail;
    }

    clsdev = device_create(ali_dmx_see_interface_class, NULL, dmx_see->dev_id, 
                           dmx_see, "ali_m36_dmx_see_0");

    if (IS_ERR(clsdev))
    {
        DMX_SEE_DEBUG(KERN_ERR "device_create() failed!\n");

        result = PTR_ERR(clsdev);

        goto fail;
    }

    /* Init SEE2Main engine here just to make sure it is inited AFTER SEE DMX
     * has been inited.
     * Need to do this more elegantly.
	*/
#if 1
    dmx_hw_interface_init(6);

    dmx_data_engine_module_init_kern(6, "ali_dmx_6_output_task", DMX_DATA_ENGINE_SRC_VIRTUAL_HW);
#endif		

    return(0);

fail:
    return(-1);
}
#else
__s32 dmx_see_init
(
    void
)
{
    __s32                               result;
    struct device                      *clsdev;
    struct dmx_see_device              *dmx_see;
	__u32                               size_order;
    __u32                               cfg_buf_len;
    volatile struct dmx_see_init_param *see_buf_init;
	__u32                               main2see_buf_addr;
	__u32                               see_decrypt_buf_addr;
	__u32                               see2main_buf_addr;

    result = 0;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    //dump_stack();
	
    dmx_see = &(ali_dmx_see_dev[0]);

    /* For standby test. Root cause still unkonwn.
     * Date:2014.11.13
     */
	if (1 == dmx_see->inited)
	{
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
        return(-__LINE__);
	}
	else
	{
        dmx_see->inited = 1;
	}

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

	//sed_dmx_attach(0);

    /* Get memory for SEE init param.
     */  
    see_buf_init = &dmx_see->see_buf_init;

    memset((void *)see_buf_init, 0, sizeof(struct dmx_see_init_param));

    /* Get memory for MAIN to SEE buffer.
     */    
    see_buf_init->main2see_buf = ali_rpc_malloc_shared_mm(sizeof(struct dmx_see_raw_buf_info));

    if (NULL == see_buf_init->main2see_buf)
    {
        printk("%s, %d, ali_rpc_malloc_shared_mm() failed.\n",
                __FUNCTION__, __LINE__);

        return(-1);
    }

    //printk("%s,%d\n", __FUNCTION__, __LINE__);

    see_buf_init->main2see_buf->rd = 0;

    see_buf_init->main2see_buf->wr = 0;

    cfg_buf_len = __G_ALI_MM_DMX_MEM_SIZE;

    //printk("%s,%d\n", __FUNCTION__, __LINE__);
	
    
    /* DMX buffer configration validation.
     * 2 ^ 16 = 65536 unit of TS pakcet. 
    */
    if (cfg_buf_len >= ((65536 * 188) + (3 * DMX_SEE_BUF_SIZE)))
    {
    	panic("%s,%d,DMX Buffer configration error,len:%u\n",
    		  __FUNCTION__, __LINE__, cfg_buf_len);

    	return(-__LINE__);
    }
    /* Old way to configure DMX buffer.
     * DMX HW buffers are configured at system memory map,
     * main2see buffer, see2main buffer, see decrypt buffer are dynamicly
     * allocated at driver initilization.
     * Only valid for M3516 linux project.
     */
    else if (cfg_buf_len < ((2 * 1024 * 1024) + (3 * DMX_SEE_BUF_SIZE)))
    {
        //printk("%s,%d\n", __FUNCTION__, __LINE__);
    
        size_order = get_order(DMX_SEE_BUF_SIZE);
		
		main2see_buf_addr = (__u32)__get_free_pages(GFP_KERNEL, size_order);
    
        if (!main2see_buf_addr)
        {
        	printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        	return(-ENOMEM);
        }

    	see_decrypt_buf_addr = (__u32)__get_free_pages(GFP_KERNEL, size_order);
    
        if (!see_decrypt_buf_addr)
        {
            kfree((void *)main2see_buf_addr);
			
        	printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        	return(-ENOMEM);
        }

        see2main_buf_addr = (__u32)__get_free_pages(GFP_KERNEL, size_order);
        
        if (!see2main_buf_addr)
        {
    		kfree((void *)main2see_buf_addr);
			
            kfree((void *)see_decrypt_buf_addr);
			
        	printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        	return(-ENOMEM);
        }	

		/* S3701C may have HW bug that osal_cache_invalidate() can be no affection,
		 * so we must give SEE an uncached address.
		 * This code could be write back as(main2see_buf->buf = (DMX_UINT32)buf_addr;),
		 * if SEE bug is fixed.
		 */
		main2see_buf_addr &= 0x8FFFFFFF;
        main2see_buf_addr |= 0xA0000000;	
		
		see_decrypt_buf_addr &= 0x8FFFFFFF;
        see_decrypt_buf_addr |= 0xA0000000;	

		see2main_buf_addr &= 0x8FFFFFFF;
        see2main_buf_addr |= 0xA0000000;	

        //printk("%s,%d\n", __FUNCTION__, __LINE__);		
    }
    /* New way to configure DMX buffer to keep all buffer below 256M 
     * ram address.
     * All buffers, DMX HW buffers , main2see buffer, see2main buffer,
     * see decrypt buffer are configured at system memory map.
     */ 
    else
    {
#if 0    
		/* S3701C may have HW bug that osal_cache_invalidate() can be no affection,
		 * so we must give SEE an uncached address.
		 * This code could be write back as(main2see_buf->buf = (DMX_UINT32)buf_addr;),
		 *	if SEE bug is fixed.
		 */    
		main2see_buf_addr = (__G_ALI_MM_DMX_MEM_TOP_ADDR - DMX_SEE_BUF_SIZE) | 0xA0000000;

		see_decrypt_buf_addr = (main2see_buf_addr - DMX_SEE_BUF_SIZE) | 0xA0000000;

		see2main_buf_addr = (see_decrypt_buf_addr - DMX_SEE_BUF_SIZE) | 0xA0000000;
#else
		/* S3701C may have HW bug that osal_cache_invalidate() can be no affection,
		 * so we must give SEE an uncached address.
		 * This code could be write back as(main2see_buf->buf = (DMX_UINT32)buf_addr;),
		 *	if SEE bug is fixed.
		 */    
		//main2see_buf_addr = (virt_to_phys(__G_ALI_MM_DMX_MEM_TOP_ADDR) - DMX_SEE_BUF_SIZE) | 0xA0000000;
		main2see_buf_addr = (__G_ALI_MM_DMX_MEM_START_ADDR + __G_ALI_MM_DMX_MEM_SIZE - DMX_SEE_BUF_SIZE);

        //printk("%s,%d,__G_ALI_MM_DMX_MEM_TOP_ADDR:%x,main2see_buf_addr:%x\n",
			   //__FUNCTION__, __LINE__, __G_ALI_MM_DMX_MEM_START_ADDR + __G_ALI_MM_DMX_MEM_SIZE, main2see_buf_addr);		

		see_decrypt_buf_addr = (main2see_buf_addr - DMX_SEE_BUF_SIZE);

		see2main_buf_addr = (see_decrypt_buf_addr - DMX_SEE_BUF_SIZE);

        //printk("%s,%d,__G_ALI_MM_DMX_MEM_TOP_ADDR:%x,main2see_buf_addr:%x\n", __FUNCTION__, __LINE__, __G_ALI_MM_DMX_MEM_TOP_ADDR, main2see_buf_addr);		
		
#endif
    }	

	see_buf_init->main2see_buf->buf = main2see_buf_addr;

    //printk("%s,%d,main2see_buf->buf:%x\n", __FUNCTION__, __LINE__, see_buf_init->main2see_buf->buf);

    see_buf_init->decrypt_buf = see_decrypt_buf_addr;
	
    see_buf_init->decrypt_buf_size = DMX_SEE_BUF_SIZE;

    /* statistics must live in share memory to allow main CPU know SEE DMX
     * statistics. 
     * Note share memory's size is less than 512 bytes, so 
     * struct dmx_see_statistics size should be keeped as small as possible.
     */
    see_buf_init->statistics = (struct Ali_DmxSeeStatistics *)ali_rpc_malloc_shared_mm(sizeof(struct Ali_DmxSeeStatistics));

    if (NULL == see_buf_init->statistics)
    {
        DMX_SEE_DEBUG("%s, %d, ali_rpc_malloc_shared_mm() failed.\n",
                __FUNCTION__, __LINE__);

        return(-1);
    }

    memset((void *)see_buf_init->statistics, 0, sizeof(struct Ali_DmxSeeStatistics));

    /* Modified for AS solution with limited share memory size.
     * TODO: replaced by RPC.
     * Date:2014.07.10, by Jingang.Chu.
	 */	
    #if !defined(CONFIG_ALI_AS)
    see_buf_init->GlobalStatInfo = (volatile struct Ali_DmxSeeGlobalStatInfo *)ali_rpc_malloc_shared_mm(sizeof(struct Ali_DmxSeeGlobalStatInfo));
	#else
    see_buf_init->GlobalStatInfo = (volatile struct Ali_DmxSeeGlobalStatInfo *)kmalloc(sizeof(struct Ali_DmxSeeGlobalStatInfo), GFP_KERNEL);
	#endif
    
    if (NULL == see_buf_init->GlobalStatInfo)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        ali_rpc_free_shared_mm((void *)see_buf_init->statistics, sizeof(struct Ali_DmxSeeStatistics));
        return(-1);
    }

    memset((void *)see_buf_init->GlobalStatInfo, 0, sizeof(struct Ali_DmxSeeGlobalStatInfo));

    /* Modified for AS solution with limited share memory size.
     * TODO: replaced by RPC.
     * Date:2014.07.10, by Jingang.Chu.
	 */	
    #if !defined(CONFIG_ALI_AS)
    see_buf_init->PlyChStatInfo = (volatile struct Ali_DmxSeePlyChStatInfo *)ali_rpc_malloc_shared_mm(6 * sizeof(struct Ali_DmxSeePlyChStatInfo));
    #else
    see_buf_init->PlyChStatInfo = (volatile struct Ali_DmxSeePlyChStatInfo *)kmalloc(6 * sizeof(struct Ali_DmxSeePlyChStatInfo), GFP_KERNEL);
	#endif
	
    if (NULL == see_buf_init->PlyChStatInfo)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        ali_rpc_free_shared_mm((void *)see_buf_init->statistics, sizeof(struct Ali_DmxSeeStatistics));
        kfree((void *)see_buf_init->GlobalStatInfo);
        return(-1);
    }

    memset((void *)see_buf_init->PlyChStatInfo, 0, 6 * sizeof(struct Ali_DmxSeePlyChStatInfo));

    __CACHE_FLUSH_ALI((__u32)see_buf_init, sizeof(struct dmx_see_init_param));

    //printk("%s,%d,see_buf_init:%x,decrypt_buf:%x,decrypt_buf_size:%d,GlobalStatInfo:%x,main2see_buf:%x,main2see_buf->buf:%x,PlyChStatInfo:%x,statistics:%x\n", __FUNCTION__, __LINE__, see_buf_init, see_buf_init->decrypt_buf, 
		   //see_buf_init->decrypt_buf_size, see_buf_init->GlobalStatInfo, see_buf_init->main2see_buf, see_buf_init->main2see_buf->buf, see_buf_init->PlyChStatInfo, see_buf_init->statistics);

#if 0
	/* Change from MAIN ARM address to SEE MIPS address.(0xCXXXXXXX -> 0x80000000)
	 * Since this address is shared by MAIN and SEE, SEE is running on MIPS, but MIAN is running on 
	 * ARM or MIPS, so we should change ARM address to SEE adress on the main side. 
	*/
    see_buf_init->main2see_buf->buf -= 0x40000000;
#endif

#if 0
    sed_set_dmx_parse_info((void *)see_buf_init);
#endif
    //printk("%s,%d,decrypt_buf:%x,decrypt_buf_size:%d,GlobalStatInfo:%x,main2see_buf:%x,main2see_buf->buf:%x,PlyChStatInfo:%x,statistics:%x\n", __FUNCTION__, __LINE__, see_buf_init->decrypt_buf, 
		   //see_buf_init->decrypt_buf_size, see_buf_init->GlobalStatInfo, see_buf_init->main2see_buf, see_buf_init->main2see_buf->buf, see_buf_init->PlyChStatInfo, see_buf_init->statistics);

    /* Set See2Main buffer.
	*/	
	dmx_see->see2main_buf_info = ali_rpc_malloc_shared_mm(sizeof(struct dmx_see_raw_buf_info));

    if (NULL == dmx_see->see2main_buf_info)
    {
        DMX_SEE_DEBUG("%s, %d, ali_rpc_malloc_shared_mm() failed.\n",
                __FUNCTION__, __LINE__);

        return(-1);
    }
	
	memset((void *)dmx_see->see2main_buf_info, 0, sizeof(struct dmx_see_raw_buf_info));

	dmx_see->see2main_buf_info->buf = see2main_buf_addr;

	/* Replaced by Sed_DmxSeeBufSet()
	*/
	//Sed_DmxSee2mainBufInfoSet((void *)(dmx_see->see2main_buf_info));
	
    __CACHE_FLUSH_ALI((__u32)(dmx_see->see2main_buf_info), sizeof(struct dmx_see_raw_buf_info));

#ifdef CONFIG_ALI_RPCNG
	/* Change from MAIN ARM address to SEE MIPS address.(0xCXXXXXXX -> 0x80000000)
	 * Since this address is shared by MAIN and SEE, SEE is running on MIPS, but MIAN is running on 
	 * ARM or MIPS, so we should change ARM address to SEE adress on the main side. 
	*/
	#ifdef CONFIG_ARM
    see_buf_init->main2see_buf->buf -= 0x20000000;
	dmx_see->see2main_buf_info->buf -= 0x20000000;
	
    Sed_DmxSeeBufSet((__u32)see_buf_init->main2see_buf - 0x20000000, (__u32)dmx_see->see2main_buf_info - 0x20000000, (__u32)see_buf_init->decrypt_buf - 0x20000000, 
                     (__u32)see_buf_init->decrypt_buf_size, (__u32)see_buf_init->statistics - 0x20000000, (__u32)see_buf_init->GlobalStatInfo - 0x20000000,
                     (__u32)see_buf_init->PlyChStatInfo - 0x20000000);
	#elif defined CONFIG_MIPS
    Sed_DmxSeeBufSet((__u32)see_buf_init->main2see_buf, (__u32)dmx_see->see2main_buf_info, (__u32)see_buf_init->decrypt_buf, 
                     (__u32)see_buf_init->decrypt_buf_size, (__u32)see_buf_init->statistics, (__u32)see_buf_init->GlobalStatInfo,
                     (__u32)see_buf_init->PlyChStatInfo);	
	#else
	#error "Unsupported CPU ARCH, suppoted main CPU archs are ARM and MIPS."
	#endif
#else
    /* Only valid for:
     * 1, main and see are both MIPS;
     * 2, main is using old RPC.
     * For example, M3516 project is in such case.
    */
    sed_set_dmx_parse_info((void *)see_buf_init->main2see_buf);
#endif

    dmx_see->usr_param.a_pid = 0x1FFF;
    dmx_see->usr_param.v_pid = 0x1FFF;
    dmx_see->usr_param.p_pid = 0x1FFF;
    dmx_see->usr_param.ad_pid = 0x1FFF;
    dmx_see->audio_pid = 0x1FFF;
    dmx_see->video_pid = 0x1FFF;
    dmx_see->pcr_pid = 0x1FFF;

    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    mutex_init(&(dmx_see->mutex));

    ali_dmx_see_interface_class = class_create(THIS_MODULE, 
                                               "ali_dmx_see_interface_class");
    
    if (IS_ERR(ali_dmx_see_interface_class))
    {            
        return(PTR_ERR(ali_dmx_see_interface_class));
    }

    result = alloc_chrdev_region(&dmx_see->dev_id, 0, 1, "ali_m36_dmx_see_0");

    if (result < 0) 
    {
        DMX_SEE_DEBUG("%s, alloc_chrdev_region() failed.\n", __FUNCTION__);

        goto fail;
    }

    DMX_SEE_DEBUG("%s, dev_id:%d.\n", __FUNCTION__, dmx_see->dev_id);

    cdev_init(&(dmx_see->cdev), &g_ali_m36_dmx_see_fops);

    dmx_see->cdev.owner = THIS_MODULE;

    result = cdev_add(&dmx_see->cdev, dmx_see->dev_id, 1);

    /* TODO: Fail gracefully if need be. 
    */
    if (result)
    {
        DMX_SEE_DEBUG("cdev_add() failed, result:%d\n", result);

        goto fail;
    }

    clsdev = device_create(ali_dmx_see_interface_class, NULL, dmx_see->dev_id, 
                           dmx_see, "ali_m36_dmx_see_0");

    if (IS_ERR(clsdev))
    {
        DMX_SEE_DEBUG(KERN_ERR "device_create() failed!\n");

        result = PTR_ERR(clsdev);

        goto fail;
    }

    /* Init SEE2Main engine here just to make sure it is inited AFTER SEE DMX
     * has been inited.
     * Need to do this more elegantly.
	*/
#if 0
    dmx_hw_interface_init(6);

    dmx_data_engine_module_init_kern(6, "ali_dmx_6_output_task", DMX_DATA_ENGINE_SRC_VIRTUAL_HW);
#endif		

    return(0);

fail:
    return(-1);
}

#endif

static void __exit dmx_see_exit(void)
{
    DMX_SEE_DEBUG("%s\n", __FUNCTION__);
}

//device_initcall_sync(dmx_see_init);
//module_init(dmx_see_init);

//module_exit(dmx_see_exit);




