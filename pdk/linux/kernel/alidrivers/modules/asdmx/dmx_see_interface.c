
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/delay.h>
#include "../../include/ali_cache.h"
#include <linux/freezer.h>



#include <asm/io.h>
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld_deca.h>
#include <rpc_hld/ali_rpc_hld_decv.h>
#include <rpc_hld/ali_rpc_hld_snd.h>

#include <ali_basic_common.h>

#include <ali_dmx_common.h>
#include <linux/dvb/ali_dsc.h>

#include "dmx_internal.h"


MODULE_LICENSE("GPL");

//#define DMX_INT32 DMX_INT32

extern void stc_invalid(void);

extern void stc_valid(void);

extern struct class *g_ali_m36_dmx_class;

//EXPORT_SYMBOL(dmx_see_pid_is_descramling);



#define dmx_offsetof(type, f) ((unsigned long) \
((char *)&((type *)0)->f - (char *)(type *)0))


#define AUDIO_TYPE_TEST(pid, type)		(((pid)&0xE000)==(type))


#define AC3_DES_EXIST					(1<<13)//0x2000
#define AAC_DES_EXIST					(2<<13)//0x4000//LATM_AAC
#define EAC3_DES_EXIST					(3<<13)//0x0001//EAC3
#define ADTS_AAC_DES_EXIST				(4<<13)//0x8000//ADTS_AAC



#if 0
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
    FUNC_SED_SET_PARSE_STATUS,
    FUNC_SED_SET_PCR_PID,
    FUNC_SED_GET_DISCNT,
    FUNC_SED_CLEAR_DISCNT
};
#else

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
    FUNC_SED_STANDBY,
    FUNC_SED_SET_SAT2IP,
};
#endif










#if 0


enum DMX_SEE_STATUS
{
    DMX_SEE_STATUS_IDLE,
    DMX_SEE_STATUS_READY,
    DMX_SEE_STATUS_RUN
};


/* Must keep compatible with TDS macro: #define TS_BLOCK_SIZE 0xbc000. */
#define DMX_SEE_BUF_SIZE             0xBC000       



/* Must keep compatible with TDS enum: enum DEMUX_STATE. */
enum DEMUX_STATE{
    DEMUX_FIND_START = 0,
    DEMUX_HEADER,
    DEMUX_DATA,
    DEMUX_SKIP 
};



/* Must keep compatible with TDS enum: enum STREAM_TYPE. */
enum STREAM_TYPE
{
	UNKNOW_STR = 0,
	PRG_STR_MAP,
	PRIV_STR_1,
	PAD_STR,
	PRIV_STR_2,
	AUDIO_STR,
	VIDEO_STR,
	ECM_STR,
	EMM_STR,
	DSM_CC_STR,
	ISO_13522_STR,
	H2221_A_STR,
	H2221_B_STR,
	H2221_C_STR,
	H2221_D_STR,
	H2221_E_STR,
	ANCILLARY_STR,
	REV_DATA_STR,
	PRG_STR_DIR
};



/* Must keep compatible with TDS struct: struct pes_pkt_param. */
struct pes_pkt_param{
	UINT8 stream_id;
	UINT16 pes_pkt_len;
        /*
       UINT8 orig_or_copy:1;
       UINT8 copyright:1;
       UINT8 data_align_indi:1;
       UINT8 pes_priority:1;
       */
	UINT8 filled_byte_3:4;
	UINT8 pes_scramb_ctrl:2;
	UINT8 marker_10:2;
        /*
       UINT8 pes_ext_flag:1;
       UINT8 pes_crc_flag:1;
       UINT8 additional_copy_info_flag:1;
       UINT8 dsm_trick_md_flag:1;
       UINT8 es_rate_flag:1;
       UINT8 escr_flag:1;
        */
	UINT8 filled_byte_4:6;
	UINT8 pts_dts_flags:2;

	UINT8   pes_header_data_len;
	UINT16 pes_data_len;
        /*
       UINT8 marker_bit_0:1;
       UINT8 pts_32_30:3;
       UINT8 prefix_4_bits:4;

       UINT16 marker_bit_1:1;
       UINT16 pts_29_15:15;

       UINT16 marker_bit_2:1;
       UINT16 pts_14_0:15;
        */
        // UINT32  total_byte_len;
	enum DEMUX_STATE dmx_state ;
    enum STREAM_TYPE stream_type;
	UINT32 head_buf_pos;
	void * av_buf ;
	UINT32  av_buf_pos ;
	UINT32  av_buf_len ;
	struct control_block * ctrl_blk;
	void * device;
	DMX_INT32 (* request_write)(void *, UINT32, void **, UINT32 *, struct control_block *);
	void (* update_write)(void *, UINT32 );


	UINT8   get_pkt_len:1;
	UINT8   get_header_data_len:1;
	UINT8   get_pts:1;
	UINT8   str_confirm:1;
	UINT8   reserved:4;

	UINT8   conti_conter;
	UINT8 	*head_buf;

	UINT8 ch_num;
	struct dmx_channel *channel;

//cw 
  UINT8		cw_parity_num; /*for 3601, (actual index+1)
							1~8:  a ecw/ocw parity number,
							0: no cw parity set for this channel*/
							
//sgdma	
	UINT8		xfer_es_by_dma;//transfer it by dma or not
	UINT8		dma_ch_num;//dma channel of this stream
	UINT32	last_dma_xfer_id;//latest dma transfer id
	
//statistic
	UINT8		ts_err_code; //last err code of this channel
	UINT32	ovlp_cnt;	//ovlp INT cnt
	UINT32	discont_cnt;	
};


/* Must keep compartible with TDS:
 * struct SEE_PARSE_INFO
 * {
 *     UINT32 rd ;
 *     UINT32 wt ;
 *     UINT32 dmx_ts_blk ;
 *     ID mutex;
 * };
 */
struct dmx_see_buf_info
{
    DMX_UINT32 rd;
    DMX_UINT32 wr;
    DMX_UINT32 buf;

    /* Not used. */
    DMX_UINT16 mutex;
};




struct dmx_see_device
{
    UINT8  name[16];

    enum DMX_SEE_STATUS status;

    dev_t dev_id;

    /* Linux char device for dmx. */
    struct cdev cdev;

    struct mutex mutex; 

    struct dmx_see_av_info usr_param;

    /* For SEE.*/
	struct dmx_see_init_param see_buf_init;

    /* For see pes parsing. */
    struct pes_pkt_param pes_para[4];

    struct control_block ctrl_blk[4];
};
#endif


DMX_INT32 dmx_see_open(struct inode *inode, struct file *file);

DMX_INT32 dmx_see_release(struct inode * inode, struct file * file);

DMX_INT32 dmx_see_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);




struct file_operations g_ali_m36_dmx_see_fops = {
	.owner =    THIS_MODULE,
	//.llseek =   scull_llseek,
	//.read =     dmx_channel_read,
	//.write =    dmx_write,
	.unlocked_ioctl =    dmx_see_ioctl,
	.open =     dmx_see_open,
	.release =  dmx_see_release,
    //.poll = dmx_channel_poll,
};


struct dmx_see_device g_dmx_see_devices[1];
volatile struct Ali_DmxSeePlyChStatInfo g_PlyChStatInfo[8];

__s32 g_see_crypto_started = 0;

#if 0
DMX_UINT32 desc_sed_play_channel[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct pes_pkt_param)), DESC_STATIC_STRU(1, sizeof(struct pes_pkt_param)),
  2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(1, 3, 1),
  //desc of pointer ret
  0,                          
  0,
};
#else
DMX_UINT32 desc_sed_play_channel[] = 
{ //desc of pointer para
  3, DESC_STATIC_STRU(0, sizeof(struct pes_pkt_param)), DESC_STATIC_STRU(1, sizeof(struct pes_pkt_param)),DESC_STATIC_STRU(2, sizeof(struct pes_pkt_param)),
  3, DESC_P_PARA(0, 3, 0), DESC_P_PARA(1, 4, 1), DESC_P_PARA(2, 5, 2),
  //desc of pointer ret
  0,                          
  0,
};
#endif


DMX_UINT32 desc_sed_reg_serv[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pes_pkt_param)),
  1, DESC_P_PARA(0, 2, 0),
  //desc of pointer ret
  0,                          
  0,
};
/*
DMX_UINT32 desc_sed_m36_is_av_scrambled[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct io_param_ex)),DESC_OUTPUT_STRU(1, 4),
  2, DESC_P_PARA(0, 0, 0), DESC_P_STRU(1, 0, 1, offsetof(struct io_param_ex, io_buff_out)),
  //desc of pointer ret
  0,                          
  0,
};
*/
DMX_UINT32 desc_sed_m36_is_av_scrambled[] = 
{ //desc of pointer para
  3, DESC_STATIC_STRU(0, sizeof(struct io_param)), DESC_STATIC_STRU(1, 4), DESC_OUTPUT_STRU(2, 4),
  3, DESC_P_PARA(0, 0, 0), DESC_P_STRU(1, 0, 1, offsetof(struct io_param, io_buff_in)), DESC_P_STRU(2, 0, 2, offsetof(struct io_param, io_buff_out)),
  0,                          
  0,
};


#if 0
UINT32 desc_sed_audio_change[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct pes_pkt_param)), 
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,                          
  0,
};
#else
DMX_UINT32 desc_sed_audio_change[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct pes_pkt_param)), DESC_STATIC_STRU(1, sizeof(struct pes_pkt_param)), 
  2, DESC_P_PARA(0, 2, 0), DESC_P_PARA(0, 3, 0),
  //desc of pointer ret
  0,                          
  0,
};
#endif


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


#if 0
void sed_play_channel(UINT16 vpid, UINT16 apid, struct pes_pkt_param *v_pes_param, struct pes_pkt_param *a_pes_param)
{
    jump_to_func(NULL, ali_rpc_call, vpid, (LLD_DMX_M36F_MODULE<<24)|(4<<16)|FUNC_SED_PLAY_CHANNEL, desc_sed_play_channel);
}
#else
void sed_play_channel(UINT16 vpid, UINT16 apid, UINT16 ad_pid, struct pes_pkt_param *v_pes_param, struct pes_pkt_param *a_pes_param, struct pes_pkt_param *ad_pes_param)
{
    jump_to_func(NULL, ali_rpc_call, vpid, (LLD_DMX_M36F_MODULE<<24)|(6<<16)|FUNC_SED_PLAY_CHANNEL, desc_sed_play_channel);
}

#endif

void sed_reg_serv(UINT16 pid, UINT8 filter_idx, struct pes_pkt_param *pes_param)
{
    jump_to_func(NULL, ali_rpc_call, pid, (LLD_DMX_M36F_MODULE<<24)|(3<<16)|FUNC_SED_REG_SERV, desc_sed_reg_serv);
}
DMX_INT32 sed_m36_is_av_scrambled(struct io_param_ex   *param)
{
    register DMX_INT32 ret asm("$2");	

    jump_to_func(NULL, ali_rpc_call, param, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_M36_IS_AV_SCRAMBLED, desc_sed_m36_is_av_scrambled);

    return(ret);
}

UINT32 sed_standby(UINT32 status)
{
    register DMX_INT32 ret asm("$2");	

    jump_to_func(NULL, ali_rpc_call, status, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_STANDBY, NULL);

    return(ret);
}

#if 0
void sed_set_dev(void *dec_dev,DMX_UINT32 callback)
{
    jump_to_func(NULL, ali_rpc_call, dec_dev, (LLD_DMX_M36F_MODULE<<24)|(2<<16)|FUNC_SED_SET_DEV, NULL);
}
#else
void sed_set_dev(void *dec_dev,UINT32 decrypt_type)
{
    jump_to_func(NULL, ali_rpc_call, dec_dev, (LLD_DMX_M36F_MODULE<<24)|(2<<16)|FUNC_SED_SET_DEV, NULL);
}
#endif

void sed_adjust_stc(DMX_UINT32 pdev)
{
    jump_to_func(NULL, ali_rpc_call, pdev, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_ADJUST_STC, NULL);
}
void sed_dmx_playback_submit_special(UINT8 *buf, DMX_UINT32 len)
{
   
    jump_to_func(NULL, ali_rpc_call, buf, (LLD_DMX_M36F_MODULE<<24)|(2<<16)|FUNC_SED_SUMMIT_SPEC, NULL);
}
void sed_dmx_playback_stc_monitor(DMX_UINT32 pcr)
{
    jump_to_func(NULL, ali_rpc_call, pcr, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_STC_MONITOR, NULL);
}

void sed_set_playback_speed(DMX_UINT32 speed)
{
    
     jump_to_func(NULL, ali_rpc_call, speed, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_PLAYBACK_SPEED, NULL);
}
DMX_UINT32 sed_set_dmx_parse_info(void *sed_info)
{
    register DMX_INT32 ret asm("$2");	

    jump_to_func(NULL, ali_rpc_call, sed_info, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_PARSE_INFO, NULL);
    return(ret);
}
DMX_INT32 see_parse_pes_header(DMX_UINT32 stream_id, UINT8 *buf,DMX_UINT32 buf_len)
{
    register DMX_INT32 ret asm("$2");	

    jump_to_func(NULL, ali_rpc_call, stream_id, (LLD_DMX_M36F_MODULE<<24)|(3<<16)|FUNC_SED_PARSE_PES, NULL);
    return(ret);
}
void see_dmx_update_remain_data(DMX_UINT32 dummy)
{
    jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_UPDATE_REMAIN_DATA, NULL);
}
void sed_dmx_tsg_pre_adjust(DMX_UINT32 pcr,DMX_UINT32 mode)
{
    jump_to_func(NULL, ali_rpc_call, pcr, (LLD_DMX_M36F_MODULE<<24)|(2<<16)|FUNC_SED_TSG_ADJUST_PCR, NULL);
}

void sed_set_av_mode(DMX_UINT32 mode)
{
    jump_to_func(NULL, ali_rpc_call, mode, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_AV_MODE, NULL);
}

void sed_reset_to_live_mode(DMX_UINT32 dummy)
{
     jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_RESET_TO_LIVE_MODE, NULL);
}
 
void sed_set_decrypt_status(DMX_UINT32 OnOff)
{
   jump_to_func(NULL, ali_rpc_call, OnOff, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_DECRYPT_STATUS, NULL);
}

void sed_enable_video_dma_channel(DMX_UINT32 dummy)
{
    jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(0<<16)|FUNC_SED_ENABLE_VIDEO_DMA, NULL);
}

void set_sed_parse_status(DMX_UINT32 OnOff)
{
    jump_to_func(NULL, ali_rpc_call, OnOff, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_PARSE_STATUS, NULL);
}
void sed_set_pcr_pid(UINT16 pcr_pid)
{
    jump_to_func(NULL, ali_rpc_call, pcr_pid, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_PCR_PID, NULL);
}


void sed_dmx_get_discont(DMX_UINT32 *data)
{
    DMX_UINT32 desc[] = 
    { //desc of pointer para
      1, DESC_STATIC_STRU(0, 4),
      1, DESC_P_PARA(0, 0, 0),
      0,                          
      0,
    };
    DESC_OUTPUT_STRU_SET_SIZE(desc, 0, sizeof(data)); 
    jump_to_func(NULL, ali_rpc_call, data, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_GET_DISCNT, desc);
}
void sed_dmx_clear_discont(DMX_UINT32 dummy)
{
    jump_to_func(NULL, ali_rpc_call, dummy, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_CLEAR_DISCNT, NULL);
}

void sed_change_audio_channel(UINT16 apid, UINT16 adpid, struct pes_pkt_param *a_pes_param, struct pes_pkt_param *ad_pes_param)
{
    jump_to_func(NULL, ali_rpc_call, apid, (LLD_DMX_M36F_MODULE<<24)|(4<<16)|FUNC_SED_AUDIO_CHANGE, desc_sed_audio_change);
}

RET_CODE sed_dmx_is_av_scrambled_old(UINT8 *scramble)
{
    register DMX_INT32 ret asm("$2");	

    jump_to_func(NULL, ali_rpc_call, scramble, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_M36_IS_AV_SCRAMBLED_OLD, desc_sed_m36_is_av_scrambled_old);

    return(ret);
}

RET_CODE sed_dmx_is_av_source_scrambled_old(UINT8 *scramble)
{
    jump_to_func(NULL, ali_rpc_call, scramble, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_M36_IS_AV_SOURCE_SCRAMBLED_OLD, desc_sed_m36_is_av_source_scrambled_old);
}

void sed_program_is_legal(UINT8 *scramble)
{
    jump_to_func(NULL, ali_rpc_call, scramble, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_IS_PROGRAM_LEGAL, desc_sed_is_program_legal);
}
void sed_reset_pes_param(UINT8 type, UINT8 in_tsk_context)
{
    jump_to_func(NULL, ali_rpc_call, type, (LLD_DMX_M36F_MODULE<<24)|(2<<16)|FUNC_SED_RESET_PES_PARAM, NULL);
}
  
void sed_set_hw_pcr_base(UINT32 base)
{
    jump_to_func(NULL, ali_rpc_call, base, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_HW_PCR_BASE, NULL);
}

void sed_set_ddp_certification(UINT32 enable)
{
   jump_to_func(NULL, ali_rpc_call, enable, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_DDP_CERTIFICATION, NULL);
}

void sed_set_drop_error_pes(UINT32 enable)
{
   jump_to_func(NULL, ali_rpc_call, enable, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_DROP_ERROR_PES, NULL);
}  


void sed_set_sat2ip(UINT32 enable)
{
    jump_to_func(NULL, ali_rpc_call, enable, (LLD_DMX_M36F_MODULE<<24)|(1<<16)|FUNC_SED_SET_SAT2IP, NULL);
}

__s32 dmx_see_pid_is_descramling
(
    __u16 pid
)
{
    struct dmx_see_device *dmx_see;

    dmx_see = &(g_dmx_see_devices[0]);

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





DMX_INT32 dmx_see_pid_to_audio_type
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




DMX_INT32 dmx_see_av_start_param
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

#define CNT 200

__u8 sgdma_test_src[CNT * 188];


__u8 sgdma_test_dest[CNT * 188];


__s32 g_A_See_Sgdma_ch = -1;

DMX_UINT32 g_dmx_dbg_last_pid;
DMX_UINT32 g_dmx_see_ts_wr_cnt = 0;




DMX_INT32 dmx_see_av_start
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    DMX_INT32               ret;
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
#ifdef CONFIG_RPC_HLD_DECA
		deca_io_control(deca, DECA_SET_STR_TYPE, a_format);
#endif
	}

    a_pes_para->ch_num = 1;


    /* AD PES stream param. */
    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    ad_pes_para = &(dmx_see->pes_para[5]);

    dmx_see_av_start_param(ad_pes_para, &(dmx_see->ctrl_blk[5])); 

    ad_pes_para->xfer_es_by_dma = 0;

	ad_pes_para->device = deca;   
#ifdef CONFIG_RPC_HLD_DECA
    deca_io_control(deca, DECA_INDEPENDENT_DESC_ENABLE, ADEC_DESC_CHANNEL_DISABLE);
#endif
#ifdef CONFIG_RPC_HLD_SND
    
    snd_io_control(snd_device, SND_SECOND_DECA_ENABLE, 0);
#endif
    if(usr_param->ad_pid != 0x1FFF)
    {
 #ifdef CONFIG_RPC_HLD_DECA   
        deca_io_control(deca, DECA_INDEPENDENT_DESC_ENABLE, ADEC_DESC_CHANNEL_ENABLE);
 #endif
    }

    ad_pes_para->ch_num = 2;

    
#if 1
#ifdef CONFIG_RPC_HLD_SND

    stc_invalid();
#endif
#ifdef CONFIG_RPC_HLD_DECA
    deca_set_sync_mode(a_pes_para->device, ADEC_SYNC_PTS);
#endif
    sed_set_pcr_pid(usr_param->p_pid & 0x1FFF);
    unsigned short dmx_base_addr[4]={
    	DMX0_BASE_ADDR,
    	DMX1_BASE_ADDR,
    	0,
    	DMX3_BASE_ADDR,
  	};
  	//if(usr_param->dmx_id < 3)
  	
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

DMX_INT32 dmx_see_av_stop
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
#ifdef CONFIG_RPC_HLD_DECV  
        //vdec_stop(dmx_see->pes_para[0].device, 0, 0);
        if(param != 0xFF /*DMA_INVALID_CHA*/)
            vdec_io_control(dmx_see->pes_para[0].device, VDEC_SET_DMA_CHANNEL,
                            0xFF/*DMA_INVALID_CHA*/);
#endif
    }
#endif

#if 0
    stc_invalid();
#endif
    dmx_see->status = DMX_SEE_STATUS_READY;

    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    return(0);
}



DMX_INT32 dmx_see_a_change_pid
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    DMX_INT32               ret;
    __u16                   pid;
    struct dmx_see_av_pid_info *usr_param;
    struct dmx_see_av_pid_info see_av_pid_info;
    struct pes_pkt_param   *a_pes_para;
    struct pes_pkt_param   *ad_pes_para;
    struct deca_device     *deca;
    struct snd_device      *snd_device;
    enum AudioStreamType    a_format;
	enum STREAM_TYPE        a_pes_type;

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
#ifdef CONFIG_RPC_HLD_DECA
		deca_io_control(deca, DECA_SET_STR_TYPE, a_format);
#endif
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
#ifdef CONFIG_RPC_HLD_DECA    
        deca_io_control(deca, DECA_INDEPENDENT_DESC_ENABLE, ADEC_DESC_CHANNEL_ENABLE);
#endif
        ad_pes_para->ch_num = 2;

    }
    else
    {
        snd_device = (struct snd_device *)hld_dev_get_by_type(NULL, HLD_DEV_TYPE_SND);
#ifdef CONFIG_RPC_HLD_DECA        
        deca_io_control(deca, DECA_INDEPENDENT_DESC_ENABLE, ADEC_DESC_CHANNEL_DISABLE);
#endif
#ifdef CONFIG_RPC_HLD_SND
        
        snd_io_control(snd_device, SND_SECOND_DECA_ENABLE, 0);
#endif
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



DMX_INT32 dmx_see_teletext_start
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    DMX_INT32               ret;
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




DMX_INT32 dmx_see_subtitle_start
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    DMX_INT32               ret;
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

DMX_INT32 dmx_see_crypto_type_set
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    int            ret;
    struct dec_parse_param p_param;
    DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&p_param, (void __user *)arg, sizeof(struct dec_parse_param));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }
    
    sed_set_dev(p_param.dec_dev , (UINT32)(p_param.type));	//1: AES, 0: DES
    
    return(0);
}


DMX_INT32 dmx_see_crypto_start
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


DMX_INT32 dmx_see_crypto_stop
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


DMX_INT32 dmx_see_av_sync_mode_set
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
#if 1
    DMX_INT32 ret;

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


__u32 g_ts_cnt = 0;

__u8 g_test_ts_buf[DMX_SEE_BUF_SIZE];


DMX_INT32 dmx_see_buf_wr_ts
(
    struct dmx_device     *dmx,
    struct dmx_ts_pkt_inf *pkt_inf,
    DMX_UINT32             param
)
{
    struct dmx_see_device   *dmx_see;
    DMX_UINT32                   rd;
    DMX_UINT32                   wr;
    DMX_UINT32                   next_wr;    
    volatile struct dmx_see_raw_buf_info *see_buf_info;
    UINT8                   *see_buf_addr;
    __u32                    i;

    //DMX_SEE_DEBUG("%s, %d, param:%x\n", __FUNCTION__, __LINE__, param);

    if (DMX_TS_PKT_CONTINU_DUPLICATE == pkt_inf->continuity)
    {
        //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        //return(0);
    }

    dmx_see = (struct dmx_see_device *)param;

    see_buf_info = dmx_see->see_buf_init.main2see_buf;

    wr = see_buf_info->wr;

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    //dmx_check_ts_to_kernel(pkt_inf->pkt_addr, 1);
    
   // dmx_playback_stream_control(pkt_inf->pkt_addr, dmx_see->usr_param.p_pid);
#if 1
    for (;;)
    {      
        rd = see_buf_info->rd;

        /* Buf full detection.
         * next_wr is used for buf full detection.
         */
        next_wr = wr + 188;
    
        if (next_wr >= DMX_SEE_BUF_SIZE)
        {
            next_wr = 0;

            //DMX_SEE_DEBUG("%s, %d,r:%d,w:%d\n", __FUNCTION__, __LINE__, rd, wr);

            //return(0);

        }

       
        /* Buf not full. */
        
        if(dmx->isRadio_playback == 0)
        {
            if (next_wr != rd)
            {
                //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            
                //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            
                break;
            }
        
        }
        else
        {
            if(wr>=rd)
            {
                if(wr  - rd < 20*188 )
                {
                    break;
                }
            }
            else
            {
                if(DMX_SEE_BUF_SIZE + wr  - rd < 20*188 )
                {
                    break;
                }
            }
        
        }
        

           
        
        //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        /* Buf full, wait for 5 ms. */
		mutex_unlock(&dmx->io_mutex);
        msleep(5);
        mutex_lock_interruptible(&dmx->io_mutex);
    }
#else

    rd = see_buf_info->rd;

    /* Buf full detection.
     * next_wr is used for buf full detection.
     */
    next_wr = wr + 188;

    if (next_wr >= DMX_SEE_BUF_SIZE)
    {
        //next_wr = 0;

        DMX_SEE_DEBUG("%s, %d,r:%x,w:%x,total:%x\n", __FUNCTION__, __LINE__, rd, wr, DMX_SEE_BUF_SIZE);

        return(0);

    }

#endif

    /* Got room to put TS packet, copy it. */
    see_buf_addr = (UINT8 *)see_buf_info->buf;

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    //memcpy(&see_buf_addr[wr], pkt_inf->pkt_addr, 188);

#if 1
    memcpy(see_buf_addr + wr, pkt_inf->pkt_addr, 188);

#if 0
    if (1 == pkt_inf->unit_start)
    {
        if (pkt_inf->payload_len != 0)
        {
            if (!((0 == pkt_inf->payload_addr[0]) && (0 == pkt_inf->payload_addr[1]) && (1 == pkt_inf->payload_addr[2])))
            {
                printk("%s,%d,%02x,%02x,%02x\n", __FUNCTION__, __LINE__, pkt_inf->payload_addr[0], pkt_inf->payload_addr[1], pkt_inf->payload_addr[2]);
            }
        }

    }
#endif
    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

#if 0
    dma_cache_wback(see_buf_info->buf + wr, 188);
#endif
    //dma_cache_wback(see_buf_info->buf, DMX_SEE_BUF_SIZE);

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
#else

    g_ts_cnt++;

    if (0 > g_A_See_Sgdma_ch)
    {
        memset(g_test_ts_buf, 0, sizeof(g_test_ts_buf));

        g_A_See_Sgdma_ch = A_SgdmaChOpen(A_SGDMA_CH_TYPE_NO_SC);
        //g_A_See_Sgdma_ch = A_SgdmaChOpen(A_SGDMA_CH_TYPE_SC);


        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    }

#if 1
    //if ((0 > g_A_See_Sgdma_ch) && (g_ts_cnt > 500))
    if (0 > g_A_See_Sgdma_ch)
    {
        memcpy(see_buf_addr + wr, pkt_inf->pkt_addr, 188);
   
        dma_cache_wback(see_buf_info->buf + wr, 188);

        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    }
    else
    {
#if 0
        for (i = 0; i < 188; i++)
        {
            (see_buf_addr + wr)[i] = i;
        }     

        dma_cache_wback(see_buf_info->buf + wr, 188);   
#endif

#if 1
        A_SgdmaChCpy(g_A_See_Sgdma_ch, see_buf_addr + wr, pkt_inf->pkt_addr, 188);

        A_SgdmaChFlush(g_A_See_Sgdma_ch, ALI_SGDMA_FLUSH_MODE_SPINWAIT);

#if 0
        //dma_cache_wback(see_buf_info->buf + wr, 188);

        dma_cache_inv(see_buf_addr + wr, 188);

#if 0
        for (i = 0; i < 188; i++)
        {
            printk("%2x,%2x \n", pkt_inf->pkt_addr[i], (see_buf_addr + wr)[i]);
        }
#endif
        if (0 != memcmp(see_buf_addr + wr, pkt_inf->pkt_addr, 188))
        {
            printk("\n\n%d\n", g_ts_cnt);

#if 1
            for (i = 0; i < 188; i++)
            {
                printk("%2x,%2x \n", pkt_inf->pkt_addr[i], (see_buf_addr + wr)[i]);
            }
#endif
            return;
        }
#endif
#else

#if 1
        for (i = 0; i < 188; i++)
        {
            (see_buf_addr + wr)[i] = i;
        }
#endif  

        A_SgdmaChCpy(g_A_See_Sgdma_ch, g_test_ts_buf, pkt_inf->pkt_addr, 188);
    
        A_SgdmaChFlush(g_A_See_Sgdma_ch, ALI_SGDMA_FLUSH_MODE_SPINWAIT);
    
        dma_cache_inv((__u32)g_test_ts_buf, 188);
    
        if (0 != memcmp(pkt_inf->pkt_addr, g_test_ts_buf, 188))
        {
            printk("1:%d\n", g_ts_cnt);

            for (i = 0; i < 188; i++)
            {
                printk("%2x,%2x \n", pkt_inf->pkt_addr[i], (see_buf_addr + wr)[i]);
            }
        }

        memcpy(see_buf_addr + wr, g_test_ts_buf, 188);
    
        dma_cache_wback(see_buf_info->buf + wr, 188);

        if (0 != memcmp(see_buf_addr + wr, g_test_ts_buf, 188))
        {
            printk("2:%d\n", g_ts_cnt);
    
        }
#endif
    }
#endif




#if 0
    //dma_cache_wback(see_buf_info->buf + wr, 188);

    A_SgdmaChCpy(g_A_See_Sgdma_ch, g_test_ts_buf, pkt_inf->pkt_addr, 188);

    A_SgdmaChFlush(g_A_See_Sgdma_ch, ALI_SGDMA_FLUSH_MODE_SPINWAIT);

    //dma_cache_wback(see_buf_info->buf + wr, 188);
#endif

#if 0
    g_ts_cnt++;

    dma_cache_inv(g_test_ts_buf, 188);
#endif

#if 0
    if (0 != memcmp(see_buf_addr + wr, pkt_inf->pkt_addr, 188))
    {
        printk("%d\n", g_ts_cnt);

    }
#endif

#if 0
    for (i = 0; i < 188; i++)
    {
        printk("%2x,%2x \n", pkt_inf->pkt_addr[i], g_test_ts_buf[i]);
    }
#endif

#if 0
    if (0 != memcmp(pkt_inf->pkt_addr, g_test_ts_buf, 188))
    {
        printk("1:%d\n", g_ts_cnt);

    }
#endif

#if 0
    memcpy(see_buf_addr + wr, g_test_ts_buf, 188);

    dma_cache_wback(see_buf_info->buf + wr, 188);

#if 1
    if (0 != memcmp(see_buf_addr + wr, g_test_ts_buf, 188))
    {
        printk("2:%d\n", g_ts_cnt);

    }
#endif

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
#endif


#endif

#if 0
    if (pkt_inf->pkt_addr[0] != 0x47)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    }
#endif

#if 0
    if (512 == pkt_inf->pid)
    {
        if ((g_dmx_dbg_last_pid != pkt_inf->pid) &&
           ((g_dmx_dbg_last_pid + 1) != pkt_inf->pid))
        {
            DMX_SEE_DEBUG("%s,%d,discon,exp:%d,cur:%d\n",
                          __FUNCTION__, __LINE__, g_dmx_dbg_last_pid + 1,
                          pkt_inf->pid);
         
        }

        g_dmx_dbg_last_pid = pkt_inf->pid;
    }
#endif


    see_buf_info->wr = next_wr;

    //DMX_SEE_DEBUG("rd:%d,wr:%d\n", see_buf_info->rd, see_buf_info->wr);

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    g_dmx_see_ts_wr_cnt++;

#if 0
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
    __u32  ret;

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
    __u32                        ret;

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
		//initialized here!
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


__s32 dmx_see_get_discont
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    __u32  discont=0;
    __u32  ret = 0;

    DMX_SEE_DEBUG("%s %d\r\n",__FUNCTION__,__LINE__);

    sed_dmx_get_discont(&discont);

    DMX_SEE_DEBUG("discont=%d\r\n",discont);

    ret = copy_to_user((void __user *)arg, &discont, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_to_user() failed, ret:%d\n", ret);

        ret = -ENOTTY;
    }

    return ret;
}


__s32 dmx_see_clear_discont
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    DMX_SEE_DEBUG("%s %d\r\n",__FUNCTION__,__LINE__);
    
    sed_dmx_clear_discont(0);

    return 0;
}

__s32 dmx_see_get_dmx_buffer_remain_data_len
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    
    volatile struct dmx_see_raw_buf_info *see_buf_info;
    DMX_UINT32   rd;
    DMX_UINT32   wr;
    DMX_UINT32   nDataInBuffer = 0;
    __u32       ret = 0;
    
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

__s32 dmx_see_do_ddp_certification
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
    unsigned long enable;
    __u32    ret;

    
    ret = copy_from_user(&enable, (void __user *)arg,
                         _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    
    sed_set_ddp_certification(enable);
    
    return 0;
}

__s32 dmx_see_set_sat2ip
(
    struct dmx_see_device *dmx_see,
    unsigned int           cmd,
    unsigned long          arg
)
{
        unsigned long enable;
    __u32    ret;

    
    ret = copy_from_user(&enable, (void __user *)arg,
                         _IOC_SIZE(cmd));

    if (0 != ret)
    {
        DMX_SEE_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    
    sed_set_sat2ip(enable);
    
    return 0;
}

DMX_INT32 dmx_see_release
(
    struct inode *inode,
    struct file  *file
)
{
    DMX_INT32              ret;
    struct dmx_see_device *dmx_see;

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

    return(ret);
}







DMX_INT32 dmx_see_open
(
    struct inode *inode,
    struct file  *file
)
{
    DMX_INT32                  ret;
    struct dmx_see_device *dmx_see;

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

    return(ret);
}



DMX_INT32 dmx_see_paused(void)
{
    struct dmx_see_device *dmx_see;
    dmx_see = &(g_dmx_see_devices[0]);
	if(dmx_see->status == DMX_SEE_STATUS_PAUSE)
		return 1;
	return 0;
}

DMX_INT32 dmx_see_ioctl
(

    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
{
    DMX_INT32                  ret;
    struct dmx_see_device *dmx_see;
    struct dmx_ts_kern_recv_info ts_kern_recv_info;

    //DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = 0;

    dmx_see = filp->private_data;

	switch(cmd)
    {
        case ALI_DMX_SEE_AV_START:
        {
            DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    		if (mutex_lock_interruptible(&dmx_see->mutex))
            {
    	        return(-ERESTARTSYS);
    		}

            DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

            ret = dmx_see_av_start(dmx_see, cmd, arg);

            DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

		    mutex_unlock(&dmx_see->mutex);
        }
        break;

        case ALI_DMX_SEE_AV_STOP:
        {
    		if (mutex_lock_interruptible(&dmx_see->mutex))
            {
    	        return(-ERESTARTSYS);
    		}

            ret = dmx_see_av_stop(dmx_see, arg);

		    mutex_unlock(&dmx_see->mutex);
        }
        break;

    	case ALI_DMX_SEE_AV_SCRAMBLE_STATUS:
        {
            ret = dmx_see_av_scram_status(dmx_see, cmd, arg);
        }
        break;

    	case ALI_DMX_SEE_AV_SCRAMBLE_STATUS_EXT:
        {
            ret = dmx_see_av_scram_status_ext(dmx_see, cmd, arg);
        }
        break;

        case ALI_DMX_SEE_GET_TS_INPUT_ROUTINE:
        {
            ts_kern_recv_info.kern_recv_routine = (DMX_UINT32)dmx_see_buf_wr_ts;

            ts_kern_recv_info.kern_recv_routine_para = 
                (DMX_UINT32)(&(g_dmx_see_devices[0]));

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
            DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    		if (mutex_lock_interruptible(&dmx_see->mutex))
            {
    	        return(-ERESTARTSYS);
    		}

            ret = dmx_see_crypto_type_set(dmx_see, cmd, arg);

		    mutex_unlock(&dmx_see->mutex);
        }
        break;

        case ALI_DMX_SEE_CRYPTO_START:
        {
            DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    		if (mutex_lock_interruptible(&dmx_see->mutex))
            {
    	        return(-ERESTARTSYS);
    		}

            ret = dmx_see_crypto_start(dmx_see, cmd, arg);

		    mutex_unlock(&dmx_see->mutex);
        }
        break;

        case ALI_DMX_SEE_CRYPTO_STOP:
        {
            DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    		if (mutex_lock_interruptible(&dmx_see->mutex))
            {
    	        return(-ERESTARTSYS);
    		}

            ret = dmx_see_crypto_stop(dmx_see, cmd, arg);

		    mutex_unlock(&dmx_see->mutex);
        }
        break;

        case ALI_DMX_SEE_SET_PLAYBACK_SPEED:
        {
            unsigned long speed;
            
            ret = copy_from_user(&speed, (void __user *)arg, _IOC_SIZE(cmd));
            if (0 != ret)
                return(-EFAULT);
            if (mutex_lock_interruptible(&dmx_see->mutex))
    	          return(-ERESTARTSYS);
            sed_set_playback_speed(speed);
            mutex_unlock(&dmx_see->mutex);
            return RET_SUCCESS;        
        }
        break;
#if 0//by wen
        case ALI_DMX_SEE_PLAYBACK_PAUSE:
        {
            if (mutex_lock_interruptible(&dmx_see->mutex))
    	          return(-ERESTARTSYS);
            //printk("");
            set_sed_parse_status(2);
            //see_dmx_update_remain_data(1);
            //dmx_see->status = DMX_SEE_STATUS_PAUSE;
            mutex_unlock(&dmx_see->mutex);
            return RET_SUCCESS;        
        }
#endif        
        break;
#if 0
		case ALI_DMX_SEE_PLAYBACK_PRE_PAUSE:
        {
            if (mutex_lock_interruptible(&dmx_see->mutex))
    	          return(-ERESTARTSYS);
            
            dmx_see->status = DMX_SEE_STATUS_PAUSE;
            mutex_unlock(&dmx_see->mutex);
            return RET_SUCCESS;        
        }
        
        break;
#endif
        case ALI_DMX_SEE_GET_DISCONTINUE_COUNT:
        {
            if (mutex_lock_interruptible(&dmx_see->mutex))
              return(-ERESTARTSYS);

            dmx_see_get_discont(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
            return RET_SUCCESS;  
        }
        break;
#if 0        
        case ALI_DMX_SEE_CLEAR_DISCONTINUE_COUNT:
        {   
            
            if (mutex_lock_interruptible(&dmx_see->mutex))
              return(-ERESTARTSYS);

            dmx_see_clear_discont(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
            return RET_SUCCESS;  
        }
        break;
#endif        
        case ALI_DMX_SEE_GET_MAIN2SEEBUF_REAMIN_DATA_LEN:
        {   
            
            if (mutex_lock_interruptible(&dmx_see->mutex))
              return(-ERESTARTSYS);

            dmx_see_get_dmx_buffer_remain_data_len(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
            return RET_SUCCESS;  
        }
        break;
#if 0 //by wen
        case ALI_DMX_SEE_DO_DDP_CERTIFICATION:
        {
            if (mutex_lock_interruptible(&dmx_see->mutex))
              return(-ERESTARTSYS);

            dmx_see_do_ddp_certification(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
            return RET_SUCCESS;  
            
        }
        break;
        case ALI_DMX_SEE_SET_SAT2IP:
        {
            if (mutex_lock_interruptible(&dmx_see->mutex))
              return(-ERESTARTSYS);

            dmx_see_set_sat2ip(dmx_see, cmd, arg);

            mutex_unlock(&dmx_see->mutex);
            return RET_SUCCESS;  
            
        }
        break;
#endif        
        default:
        {
		    ret = -EINVAL;
        }
        break;
    }

	return(ret);

}





extern __u32 __G_SEE_DMX_SRC_BUF_START;

extern __u32 __G_SEE_DMX_DECRYPTO_BUF_START;


static DMX_INT32 __init ali_dmx_see_init
(
    void//struct dmx_device *dmx
)
{
    DMX_INT32              result;
	struct device         *clsdev;
    struct dmx_see_device *dmx_see;
    UINT8                 *buf_addr;

    volatile struct dmx_see_raw_buf_info *raw_buf;
    volatile struct dmx_see_init_param *see_buf_init;

    result = 0;

    dmx_see = &(g_dmx_see_devices[0]);
	
#if 1
    buf_addr = kmalloc(DMX_SEE_BUF_SIZE, GFP_KERNEL);
#else
#if 1
    buf_addr = (__u8 *)(__G_SEE_DMX_SRC_BUF_START & 0x9FFFFFFF);
#else
    buf_addr = g_test_ts_buf;
#endif
#endif

    if(buf_addr == NULL)
    {
        DMX_SEE_DEBUG("%s, %d, ali_rpc_malloc_shared_mm() failed.\n",
                __FUNCTION__, __LINE__);

        return(-1);
    }

    printk("buf_addr:%x,g_test_ts_buf:%x\n", buf_addr, g_test_ts_buf);

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


	see_buf_init->main2see_buf->buf = (__u8 *)(((__u32)buf_addr) & 0x9FFFFFFF | 0xA0000000);

    printk("%s,%d,main2see_buf->buf:%x\n", __FUNCTION__, __LINE__, see_buf_init->main2see_buf->buf);

#if 1
    see_buf_init->decrypt_buf = (UINT32)kmalloc(DMX_SEE_BUF_SIZE, GFP_KERNEL);
    if(see_buf_init->decrypt_buf == NULL)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        kfree(see_buf_init->decrypt_buf);

        return(-1);
    }
#else
    see_buf_init->decrypt_buf = (__u32)(__G_SEE_DMX_DECRYPTO_BUF_START & 0x9FFFFFFF);
#endif


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

    //see_buf_init->GlobalStatInfo = (volatile struct Ali_DmxSeeGlobalStatInfo *)kmalloc(sizeof(struct Ali_DmxSeeGlobalStatInfo), GFP_KERNEL);
    see_buf_init->GlobalStatInfo = (volatile struct Ali_DmxSeeGlobalStatInfo *)ali_rpc_malloc_shared_mm(sizeof(struct Ali_DmxSeeGlobalStatInfo));

    if (NULL == see_buf_init->GlobalStatInfo)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        kfree((void *)see_buf_init->GlobalStatInfo);

        return(-1);
    }

    memset((void *)see_buf_init->GlobalStatInfo, 0, sizeof(struct Ali_DmxSeeGlobalStatInfo));

    see_buf_init->PlyChStatInfo = &g_PlyChStatInfo[1];
    //see_buf_init->PlyChStatInfo = (volatile struct Ali_DmxSeePlyChStatInfo *)ali_rpc_malloc_shared_mm(6 * sizeof(struct Ali_DmxSeePlyChStatInfo));
    see_buf_init->PlyChStatInfo = (volatile struct Ali_DmxSeePlyChStatInfo *)(((__u32)(see_buf_init->PlyChStatInfo)) & 0x9FFFFFFF | 0xA0000000);

	
    if (NULL == see_buf_init->PlyChStatInfo)
    {
        DMX_SEE_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        kfree((void *)see_buf_init->PlyChStatInfo);

        return(-1);
    }

    memset((void *)see_buf_init->PlyChStatInfo, 0, 6 * sizeof(struct Ali_DmxSeePlyChStatInfo));

    __CACHE_FLUSH_ALI((__u32)see_buf_init, sizeof(struct dmx_see_init_param));


#if 1
    printk("%s,%d,see_buf_init:%x,decrypt_buf:%x,decrypt_buf_size:%d,GlobalStatInfo:%x,main2see_buf:%x,main2see_buf->buf:%x,PlyChStatInfo:%x,statistics:%x\n", __FUNCTION__, __LINE__, see_buf_init, see_buf_init->decrypt_buf, 
		   see_buf_init->decrypt_buf_size, see_buf_init->GlobalStatInfo, see_buf_init->main2see_buf, see_buf_init->main2see_buf->buf, see_buf_init->PlyChStatInfo, see_buf_init->statistics);
#endif


    sed_set_dmx_parse_info((void *)see_buf_init);

    dmx_see->usr_param.a_pid = 0x1FFF;

    dmx_see->usr_param.v_pid = 0x1FFF;

    dmx_see->usr_param.p_pid = 0x1FFF;

    dmx_see->usr_param.ad_pid = 0x1FFF;


    DMX_SEE_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

    mutex_init(&(dmx_see->mutex));


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

	/* Fail gracefully if need be. */
	if (result)
    {
		DMX_SEE_DEBUG("cdev_add() failed, result:%d\n", result);

		goto fail;
    }

	clsdev = device_create(g_ali_m36_dmx_class, NULL, dmx_see->dev_id, 
                           dmx_see, "ali_m36_dmx_see_0");

	if (IS_ERR(clsdev))
    {
		DMX_SEE_DEBUG(KERN_ERR "device_create() failed!\n");

		result = PTR_ERR(clsdev);

		goto fail;
	}

    return(0);

fail:
    return(-1);
}

static void __exit ali_dmx_see_exit(void)
{
    DMX_SEE_DEBUG("%s\n", __FUNCTION__);
}

//module_init(ali_dmx_see_init);
device_initcall_sync(ali_dmx_see_init);
module_exit(ali_dmx_see_exit);
