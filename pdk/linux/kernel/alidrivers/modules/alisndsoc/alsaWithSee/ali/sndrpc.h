#include "ali_rpc_type.h"
#include "ali_rpc_errno.h"
#include "ali_rpc_cfg.h"
#include "ali_rpc_osdep.h"
#include "ali_rpc_debug.h"
#include "ali_rpc_util.h"

typedef char			INT8;
typedef unsigned char	UINT8;

typedef short			INT16;
typedef unsigned short	UINT16;

typedef long			INT32;
typedef unsigned long	UINT32;

typedef unsigned long long UINT64;
typedef long long INT64;


typedef enum _ParamId {
	PARAM_VOID,
	PARAM_INT32,
	PARAM_UINT32,
	PARAM_LONG,
	PARAM_ULONG,
	PARAM_INT16,
	PARAM_UINT16,
	PARAM_BOOL,
	PARAM_ENUM,
	PARAM_ARRAY,
	PARAM_BYTES,
	PARAM_OPAQUE,
	PARAM_STRING,
	PARAM_UNION,
	PARAM_CHAR,
	PARAM_UCHAR,
	PARAM_VECTOR,
	PARAM_FLOAT,
	PARAM_DOUBLE,
	PARAM_REFERENCE,
	PARAM_POINTER,
	PARAM_WRAPSTRING,
	PARAM_STRARRAY,
	PARAM_RPCDBGINFO,
/*User specific data type id: START*/
	PARAM_TESTSTRUCT, /*sample struct*/
    
/************ Audio playback example ******/
    PARAM_Deca_feature_config_rpc,
    PARAM_Snd_feature_config_rpc,
    PARAM_Snd_output_cfg_rpc,
    PARAM_Hld_device_rpc,
    PARAM_Pcm_output_rpc,
    PARAM_Control_block_rpc,
    PARAM_Pe_music_cfg_rpc,
    PARAM_MusicInfo_rpc,
    PARAM_DecoderInfo_rpc,
    PARAM_Image_info_rpc,
    PARAM_Image_init_config_rpc,
    PARAM_Image_hw_info_rpc,
    PARAM_Pe_image_cfg_rpc,
    PARAM_Image_info_pe_rpc,
    PARAM_Imagedec_frm_rpc,
    PARAM_Pe_video_cfg_rpc,
    PARAM_DEC_CHAPTER_INFO_RPC,
    PARAM_Fileinfo_video_rpc,
    PARAM_Pe_cache_ex_rpc,
    PARAM_Pe_cache_cmd_rpc,
    PARAM_Rect_rpc,
    PARAM_Audio_config_rpc,
    PARAM_Snd_dev_status_rpc,
    PARAM_Spec_param_rpc,
    PARAM_Spec_step_table_rpc,
    PARAM_Snd_spdif_scms_rpc,
    PARAM_Snd_sync_param_rpc,
    PARAM_Isdbtcc_config_par_rpc,
    PARAM_Atsc_subt_config_par_rpc,
    PARAM_Sdec_feature_config_rpc,
    PARAM_Subt_config_par_rpc,
    PARAM_AUDIO_INFO_rpc,
    PARAM_Reverb_param_rpc,
    PARAM_Pl_ii_param_rpc,
    PARAM_Io_param_rpc,
    PARAM_Cur_stream_info_rpc,
    PARAM_Deca_buf_info_rpc,
    PARAM_Ase_str_play_param_rpc, //add by jacket
    PARAM_Position_rpc,
    PARAM_RectSize_rpc,
    PARAM_YCbCrColor_rpc,
    PARAM_AdvSetting_rpc,
    PARAM_VDecPIPInfo_rpc,
    PARAM_VDec_StatusInfo_rpc,
	
	PARAM_Imagedec_show_shutters_rpc,
    PARAM_Imagedec_show_brush_rpc,
    PARAM_Imagedec_show_slide_rpc,
    PARAM_Imagedec_show_show_random_rpc,
    PARAM_Imagedec_show_fade_rpc,
    PARAM_Image_slideshow_effect_rpc,
    PARAM_Image_config_rpc,
    PARAM_Image_engine_config_rpc,
    PARAM_Image_display_rpc,

	
	
/**************** decv *******************/
    PARAM_Vdec_adpcm_rpc,
    PARAM_Vdec_sml_frm_rpc,
    PARAM_Vdec_mem_map_rpc,
    PARAM_Vdec_config_par_rpc,
    PARAM_Vdec_avs_memmap_rpc,
    PARAM_Vdec_avc_memmap_rpc,
    PARAM_Vdec_avc_config_par_rpc,
    PARAM_Vdec_dvrconfigparam_rpc,
    PARAM_Vdecpipinfo_rpc,
    PARAM_Mpsource_callback_rpc,
    PARAM_Pipsource_callback_rpc,
    PARAM_Vdec_statusinfo_rpc, 
    PARAM_Vdec_decore_status_rpc, 
    PARAM_Vdec_io_get_frm_para_rpc,
    PARAM_Ycbcrcolor_rpc,
    PARAM_Vdec_io_reg_callback_para_rpc,
    PARAM_Vdec_picture_rpc,
    PARAM_Vdec_io_dbg_flag_info_rpc,
    PARAM_Video_rect_rpc,
    PARAM_OutputFrmManager_rpc,
    PARAM_Vdecinit_rpc,
    
/***************** Avs ********************/
    PARAM_Avsync_cfg_param_t_rpc,
    PARAM_Avsync_adv_param_t_rpc,
    PARAM_Avsync_status_t_rpc,
    PARAM_Avsync_statistics_t_rpc,
    PARAM_Avsync_smoothly_play_cfg_t_rpc,
    PARAM_Avsync_get_stc_en_t_rpc,
    
/**************** dmx *******************/
    PARAM_Io_param_ex_rpc,
    PARAM_Pes_pkt_param_rpc,
    
/**************** vbi *******************/    
    PARAM_Vbi_config_par_rpc,
    PARAM_Ttx_config_par_rpc,
    PARAM_Ttx_page_info_rpc,
    
/**************** ce *******************/
    PARAM_Ce_data_info_rpc,
    PARAM_Otp_param_rpc,
    PARAM_Data_param_rpc,
    PARAM_Des_param_rpc,
    PARAM_Ce_key_param_rpc,
    PARAM_Ce_debug_key_info_rpc,
    PARAM_Ce_pos_status_param_rpc,
    PARAM_Ce_found_free_pos_param_rpc,
    PARAM_Ce_pvr_key_param_rpc,
    
/**************** dsc ******************/    
    PARAM_DeEncrypt_config_rpc,
    PARAM_Sha_init_param_rpc,
    PARAM_Aes_init_param_rpc,
    PARAM_Des_init_param_rpc,
    PARAM_Pid_param_rpc,
    PARAM_Csa_init_param_rpc,
    PARAM_Dsc_pvr_key_param_rpc,
    PARAM_Key_param_rpc,
    
/*************** dis *******************/
    PARAM_Vp_feature_config_rpc,
    PARAM_Tve_feature_config_rpc,
    PARAM_Osd_driver_config_rpc,
    PARAM_Vp_win_config_para_rpc,
    PARAM_Vpo_io_ttx_rpc,
    PARAM_Vpo_io_cc_rpc,
    PARAM_Dacindex_rpc,
    PARAM_Vp_dacinfo_rpc,
    PARAM_Vp_io_reg_dac_para_rpc,
    PARAM_Vp_initinfo_rpc,
    PARAM_Vpo_io_set_parameter_rpc,
    PARAM_Vpo_io_video_enhance_rpc,    
    PARAM_Vpo_io_get_info_rpc,
    PARAM_De2Hdmi_video_infor_rpc,
    PARAM_Vpo_io_cgms_info_rpc,
    PARAM_Vp_io_afd_para_rpc,
    PARAM_Vpo_io_get_picture_info_rpc,
    PARAM_Vpo_osd_show_time_rpc,
    PARAM_Vpo_io_cutline_pars_rpc,
    PARAM_Alifbio_3d_pars_rpc,


    PARAM_GMA_SIZE,
    PARAM_GMA_LAYER_PARS,
    PARAM_GMA_ENH_PARS,
    PARAM_GMA_SCALE_PARS,
    PARAM_GMA_PAL_PARS,
    PARAM_GMA_RECT,
    PARAM_GMA_REGION_PARS,
    PARAM_DE2HDMI_VIDINF,
    PARAM_SND2HDMI_AUDINF,
/*User specific data type id: END*/
	PARAM_ID_MAX
}ParamId;

typedef enum _ConnType ConnType;

enum _ConnType {
	MCAPI_MSG,
	MCAPI_PKTCHAN,
	MCAPI_SCLCHAN
};


typedef struct _RpcCd {
	ConnType connType;		/*connect type*/
	Int32 connHandle;		/*RPC internal usage handle*/
}RpcCd;


/*Create Param usage*/
#define RPC_PARAM_CREATE(param_name, param_type, param_id, param_len, data_ptr) \
			Param param_name; \
			param_name.type = param_type; \
			param_name.paramId = param_id;\
			param_name.len = param_len; \
			param_name.pData = data_ptr;

#define HASH_STR(x) hash(#x)

#define RpcCallCompletion( FUNC, ARG...) \
		__rpc_call_completion(NULL,  HASH_STR(FUNC), ##ARG);

#define MAIN_ADDR_TO_SEE_ADDR(x) (((__u32)x & 0x0FFFFFFF) | 0xA0000000)


extern Int32 __rpc_call_completion(RpcCd *rcd, Uint32 funcid, ...);

