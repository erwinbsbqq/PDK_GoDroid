/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *
 *  File: ali_rpc_service.c
 *
 *  Description: XDR new type implementation functions.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <ali_rpc_xdr.h>
#include <ali_rpc_service.h>


Bool XDR_TestStruct(XDR *xdrs, TestStruct *cp, Uint32 cnt);
Bool XDR_Rpcdbginfo(XDR *xdrs, void *cp, Uint32 cnt);
/********Audio playback example ***********/
Bool XDR_Deca_feature_config_rpc(XDR *xdrs, Deca_feature_config_rpc *cp, Uint32 cnt);
Bool XDR_Snd_output_cfg_rpc(XDR *xdrs, Snd_output_cfg_rpc *cp, Uint32 cnt);
Bool XDR_Snd_feature_config_rpc(XDR *xdrs, Snd_feature_config_rpc *cp, Uint32 cnt);
Bool XDR_Hld_device_rpc(XDR *xdrs, Hld_device_rpc *cp, Uint32 cnt);

Bool XDR_Pcm_output_rpc(XDR *xdrs, Pcm_output_rpc *cp, Uint32 cnt);
Bool XDR_Control_block_rpc(XDR *xdrs, Control_block_rpc *cp, Uint32 cnt);
Bool XDR_Pe_music_cfg_rpc(XDR *xdrs, Pe_music_cfg_rpc *cp, Uint32 cnt);
Bool XDR_MusicInfo_rpc(XDR *xdrs, MusicInfo_rpc *cp, Uint32 cnt);
Bool XDR_DecoderInfo_rpc(XDR *xdrs, DecoderInfo_rpc *cp, Uint32 cnt);
Bool XDR_Image_info_rpc(XDR *xdrs, Image_info_rpc *cp, Uint32 cnt);
Bool XDR_Image_init_config_rpc(XDR *xdrs, Image_init_config_rpc *cp, Uint32 cnt);
Bool XDR_Image_hw_info_rpc(XDR *xdrs, Image_hw_info_rpc *cp, Uint32 cnt);
Bool XDR_Pe_image_cfg_rpc(XDR *xdrs, Pe_image_cfg_rpc *cp, Uint32 cnt);
Bool XDR_Image_info_pe_rpc(XDR *xdrs, Image_info_pe_rpc *cp, Uint32 cnt);
Bool XDR_Imagedec_frm_rpc(XDR *xdrs, Imagedec_frm_rpc *cp, Uint32 cnt);
Bool XDR_Pe_video_cfg_rpc(XDR *xdrs, Pe_video_cfg_rpc *cp, Uint32 cnt);
Bool XDR_DEC_CHAPTER_INFO_RPC(XDR *xdrs, DEC_CHAPTER_INFO_RPC *cp, Uint32 cnt);
Bool XDR_Fileinfo_video_rpc(XDR *xdrs, Fileinfo_video_rpc *cp, Uint32 cnt);
Bool XDR_Pe_cache_ex_rpc(XDR *xdrs, Pe_cache_ex_rpc *cp, Uint32 cnt);
Bool XDR_Pe_cache_cmd_rpc(XDR *xdrs, Pe_cache_cmd_rpc *cp, Uint32 cnt);
Bool XDR_Rect_rpc(XDR *xdrs, Rect_rpc *cp, Uint32 cnt);
Bool XDR_Audio_config_rpc(XDR *xdrs, Audio_config_rpc *cp, Uint32 cnt);
Bool XDR_Snd_dev_status_rpc(XDR *xdrs, Snd_dev_status_rpc *cp, Uint32 cnt);
Bool XDR_Spec_param_rpc(XDR *xdrs, Spec_param_rpc *cp, Uint32 cnt);
Bool XDR_Spec_step_table_rpc(XDR *xdrs, Spec_step_table_rpc *cp, Uint32 cnt);
Bool XDR_Snd_spdif_scms_rpc(XDR *xdrs, Snd_spdif_scms_rpc *cp, Uint32 cnt);
Bool XDR_Snd_sync_param_rpc(XDR *xdrs, Snd_sync_param_rpc *cp, Uint32 cnt);
Bool XDR_Isdbtcc_config_par_rpc(XDR *xdrs, Isdbtcc_config_par_rpc *cp, Uint32 cnt);
Bool XDR_Atsc_subt_config_par_rpc(XDR *xdrs, Atsc_subt_config_par_rpc *cp, Uint32 cnt);
Bool XDR_Sdec_feature_config_rpc(XDR *xdrs, Sdec_feature_config_rpc *cp, Uint32 cnt);
Bool XDR_Subt_config_par_rpc(XDR *xdrs, Subt_config_par_rpc *cp, Uint32 cnt);
Bool XDR_AUDIO_INFO_rpc(XDR *xdrs, AUDIO_INFO_rpc *cp, Uint32 cnt);
Bool XDR_Reverb_param_rpc(XDR *xdrs, Reverb_param_rpc *cp, Uint32 cnt);
Bool XDR_Pl_ii_param_rpc(XDR *xdrs, Pl_ii_param_rpc *cp, Uint32 cnt);
Bool XDR_Io_param_rpc(XDR *xdrs, Io_param_rpc *cp, Uint32 cnt);
Bool XDR_Cur_stream_info_rpc(XDR *xdrs, Cur_stream_info_rpc *cp, Uint32 cnt);
Bool XDR_Deca_buf_info_rpc(XDR *xdrs, Deca_buf_info_rpc *cp, Uint32 cnt);
Bool XDR_Ase_str_play_param_rpc(XDR *xdrs, Ase_str_play_param_rpc *cp, Uint32 cnt);
Bool XDR_Position_rpc(XDR *xdrs, Position_rpc *cp, Uint32 cnt);
Bool XDR_RectSize_rpc(XDR *xdrs, RectSize_rpc *cp, Uint32 cnt);
Bool XDR_AdvSetting_rpc(XDR *xdrs, AdvSetting_rpc *cp, Uint32 cnt);
Bool XDR_VDecPIPInfo_rpc(XDR *xdrs, Vdecpipinfo_rpc *cp, Uint32 cnt);
Bool XDR_VDec_StatusInfo_rpc(XDR *xdrs, Vdec_statusinfo_rpc *cp, Uint32 cnt);
Bool XDR_Audio_decore_status_rpc(XDR *xdrs, Audio_decore_status_rpc *cp, Uint32 cnt);


Bool XDR_Imagedec_show_brush_rpc(XDR *xdrs, Imagedec_show_brush_rpc *cp, Uint32 cnt);
Bool XDR_Imagedec_show_brush_rpc(XDR *xdrs, Imagedec_show_brush_rpc *cp, Uint32 cnt);
Bool XDR_Imagedec_show_slide_rpc(XDR *xdrs, Imagedec_show_slide_rpc *cp, Uint32 cnt);
Bool XDR_Imagedec_show_show_random_rpc(XDR *xdrs, Imagedec_show_show_random_rpc *cp, Uint32 cnt);
Bool XDR_Imagedec_show_fade_rpc(XDR *xdrs, Imagedec_show_fade_rpc *cp, Uint32 cnt);
Bool XDR_Image_slideshow_effect_rpc(XDR *xdrs, Image_slideshow_effect_rpc *cp, Uint32 cnt);
Bool XDR_Image_config_rpc(XDR *xdrs, Image_config_rpc *cp, Uint32 cnt);
Bool XDR_Image_engine_config_rpc(XDR *xdrs, Image_engine_config_rpc *cp, Uint32 cnt);
Bool XDR_Image_display_rpc(XDR *xdrs, Image_display_rpc *cp, Uint32 cnt);



/******** Devc **********/
Bool XDR_Vdec_adpcm_rpc(XDR *xdrs, Vdec_adpcm_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_sml_frm_rpc(XDR *xdrs, Vdec_sml_frm_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_mem_map_rpc(XDR *xdrs, Vdec_mem_map_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_config_par_rpc(XDR *xdrs, Vdec_config_par_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_avs_memmap_rpc(XDR *xdrs, Vdec_avs_memmap_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_avc_memmap_rpc(XDR *xdrs, Vdec_avc_memmap_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_avc_config_par_rpc(XDR *xdrs, Vdec_avc_config_par_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_dvrconfigparam_rpc(XDR *xdrs, Vdec_dvrconfigparam_rpc *cp, Uint32 cnt);
Bool XDR_Vdecpipinfo_rpc(XDR *xdrs, Vdecpipinfo_rpc *cp, Uint32 cnt);
Bool XDR_Mpsource_callback_rpc(XDR *xdrs, Mpsource_callback_rpc *cp, Uint32 cnt);
Bool XDR_Pipsource_callback_rpc(XDR *xdrs, Pipsource_callback_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_statusinfo_rpc(XDR *xdrs, Vdec_statusinfo_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_decore_status_rpc(XDR *xdrs, Vdec_decore_status_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_io_get_frm_para_rpc(XDR *xdrs, Vdec_io_get_frm_para_rpc *cp, Uint32 cnt);
Bool XDR_Ycbcrcolor_rpc(XDR *xdrs, Ycbcrcolor_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_io_reg_callback_para_rpc(XDR *xdrs, Vdec_io_reg_callback_para_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_picture_rpc(XDR *xdrs, Vdec_picture_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_io_dbg_flag_info_rpc(XDR *xdrs, Vdec_io_dbg_flag_info_rpc *cp, Uint32 cnt);
Bool XDR_Video_rect_rpc(XDR *xdrs, Video_rect_rpc *cp, Uint32 cnt);
Bool XDR_OutputFrmManager_rpc(XDR *xdrs, OutputFrmManager_rpc *cp, Uint32 cnt);
Bool XDR_Vdecinit_rpc(XDR *xdrs, Vdecinit_rpc *cp, Uint32 cnt);
Bool XDR_Vdec_playback_param_rpc(XDR *xdrs, Vdec_playback_param_rpc *cp, Uint32 cnt);

/******** avs *************/
Bool XDR_Avsync_cfg_param_t_rpc(XDR *xdrs, Avsync_cfg_param_t_rpc *cp, Uint32 cnt);
Bool XDR_Avsync_adv_param_t_rpc(XDR *xdrs, Avsync_adv_param_t_rpc *cp, Uint32 cnt);
Bool XDR_Avsync_status_t_rpc(XDR *xdrs, Avsync_status_t_rpc *cp, Uint32 cnt);
Bool XDR_Avsync_statistics_t_rpc(XDR *xdrs, Avsync_statistics_t_rpc *cp, Uint32 cnt);
Bool XDR_Avsync_smoothly_play_cfg_t_rpc(XDR *xdrs, Avsync_smoothly_play_cfg_t_rpc *cp, Uint32 cnt);
Bool XDR_Avsync_get_stc_en_t_rpc(XDR *xdrs, Avsync_get_stc_en_t_rpc *cp, Uint32 cnt);

/******** dmx *************/
Bool XDR_Io_param_ex_rpc(XDR *xdrs, Io_param_ex_rpc *cp, Uint32 cnt);
Bool XDR_Pes_pkt_param_rpc(XDR *xdrs, Pes_pkt_param_rpc *cp, Uint32 cnt);

/******** VBI *************/
Bool XDR_Vbi_config_par_rpc(XDR *xdrs, Vbi_config_par_rpc *cp, Uint32 cnt);
Bool XDR_Ttx_config_par_rpc(XDR *xdrs, Ttx_config_par_rpc *cp, Uint32 cnt);
Bool XDR_Ttx_page_info_rpc(XDR *xdrs, Ttx_page_info_rpc *cp, Uint32 cnt);

/********* CE *************/
Bool XDR_Ce_data_info_rpc(XDR *xdrs, Ce_data_info_rpc *cp, Uint32 cnt);
Bool XDR_Otp_param_rpc(XDR *xdrs, Otp_param_rpc *cp, Uint32 cnt);
Bool XDR_Data_param_rpc(XDR *xdrs, Data_param_rpc *cp, Uint32 cnt);
Bool XDR_Des_param_rpc(XDR *xdrs, Des_param_rpc *cp, Uint32 cnt);
Bool XDR_Ce_key_param_rpc(XDR *xdrs, Ce_key_param_rpc *cp, Uint32 cnt);
Bool XDR_Ce_debug_key_info_rpc(XDR *xdrs, Ce_debug_key_info_rpc *cp, Uint32 cnt);
Bool XDR_Ce_pos_status_param_rpc(XDR *xdrs, Ce_pos_status_param_rpc *cp, Uint32 cnt);
Bool XDR_Ce_found_free_pos_param_rpc(XDR *xdrs, Ce_found_free_pos_param_rpc *cp, Uint32 cnt);
Bool XDR_Ce_pvr_key_param_rpc(XDR *xdrs, Ce_pvr_key_param_rpc *cp, Uint32 cnt);

/********* dsc ************/
Bool XDR_DeEncrypt_config_rpc(XDR *xdrs, DeEncrypt_config_rpc *cp, Uint32 cnt);
Bool XDR_Sha_init_param_rpc(XDR *xdrs, Sha_init_param_rpc *cp, Uint32 cnt);
Bool XDR_Aes_init_param_rpc(XDR *xdrs, Aes_init_param_rpc *cp, Uint32 cnt);
Bool XDR_Des_init_param_rpc(XDR *xdrs, Des_init_param_rpc *cp, Uint32 cnt);
Bool XDR_Pid_param_rpc(XDR *xdrs, Pid_param_rpc *cp, Uint32 cnt);
Bool XDR_Csa_init_param_rpc(XDR *xdrs, Csa_init_param_rpc *cp, Uint32 cnt);
Bool XDR_Dsc_pvr_key_param_rpc(XDR *xdrs, Dsc_pvr_key_param_rpc *cp, Uint32 cnt);
Bool XDR_Dsc_deen_parity_rpc(XDR *xdrs, Dsc_deen_parity_rpc *cp, Uint32 cnt);
Bool XDR_Key_param_rpc(XDR *xdrs, Key_param_rpc *cp, Uint32 cnt);
Bool XDR_Trng_data_rpc(XDR *xdrs, Trng_data_rpc *cp, Uint32 cnt);
Bool XDR_Sha_hash_rpc(XDR *xdrs, Sha_hash_rpc *cp, Uint32 cnt);
Bool XDR_Dsc_drv_ver_rpc(XDR *xdrs, Dsc_drv_ver_rpc *cp, Uint32 cnt);
Bool XDR_Dsc_ver_chk_rpc(XDR *xdrs, Dsc_ver_chk_param_rpc *cp, Uint32 cnt);

/********* dis ************/
Bool XDR_Vp_feature_config_rpc(XDR *xdrs, Vp_feature_config_rpc *cp, Uint32 cnt);
Bool XDR_Tve_feature_config_rpc(XDR *xdrs, Tve_feature_config_rpc *cp, Uint32 cnt);
Bool XDR_Osd_driver_config_rpc(XDR *xdrs, Osd_driver_config_rpc *cp, Uint32 cnt);
Bool XDR_Vp_win_config_para_rpc(XDR *xdrs, Vp_win_config_para_rpc *cp, Uint32 cnt);
Bool XDR_Vpo_io_ttx_rpc(XDR *xdrs, Vpo_io_ttx_rpc *cp, Uint32 cnt);
Bool XDR_Vpo_io_cc_rpc(XDR *xdrs, Vpo_io_cc_rpc *cp, Uint32 cnt);
Bool XDR_Dacindex_rpc(XDR *xdrs, Dacindex_rpc *cp, Uint32 cnt);
Bool XDR_Vp_dacinfo_rpc(XDR *xdrs, Vp_dacinfo_rpc *cp, Uint32 cnt);
Bool XDR_Vp_io_reg_dac_para_rpc(XDR *xdrs, Vp_io_reg_dac_para_rpc *cp, Uint32 cnt);
Bool XDR_Vp_initinfo_rpc(XDR *xdrs, Vp_initinfo_rpc *cp, Uint32 cnt);
Bool XDR_Vpo_io_set_parameter_rpc(XDR *xdrs, Vpo_io_set_parameter_rpc *cp, Uint32 cnt);
Bool XDR_Vpo_io_get_info_rpc(XDR *xdrs, Vpo_io_get_info_rpc *cp, Uint32 cnt);
Bool XDR_De2Hdmi_video_infor_rpc(XDR *xdrs, De2Hdmi_video_infor_rpc *cp, Uint32 cnt);
Bool XDR_Vpo_io_cgms_info_rpc(XDR *xdrs, Vpo_io_cgms_info_rpc *cp, Uint32 cnt);
Bool XDR_Vp_io_afd_para_rpc(XDR *xdrs, Vp_io_afd_para_rpc *cp, Uint32 cnt);
Bool XDR_Vpo_io_get_picture_info_rpc(XDR *xdrs, Vpo_io_get_picture_info_rpc *cp, Uint32 cnt);
Bool XDR_Vpo_osd_show_time_rpc(XDR *xdrs, Vpo_osd_show_time_rpc *cp, Uint32 cnt);
Bool XDR_Vpo_io_cutline_pars_rpc(XDR *xdrs, Vpo_io_cutline_pars_rpc *cp, Uint32 cnt);
Bool XDR_Alifbio_3d_pars_rpc(XDR *xdrs, Alifbio_3d_pars_rpc *cp, Uint32 cnt);







/*A sample new struct XDR process function*/
Bool XDR_TestStruct(XDR *xdrs, TestStruct *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->ii) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->uii) == False)
    {
        return False;
    }
    if (XDR_Char(xdrs, &cp->cc) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ucc) == False)
    {
        return False;
    }
    if (XDR_Long(xdrs, &cp->ll) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->ull) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bb) == False)
    {
        return False;
    }
    return True;
}

/***************Audio play example ***********************/
Bool XDR_Deca_feature_config_rpc(XDR *xdrs, Deca_feature_config_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->detect_sprt_change) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bs_buff_size) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->support_desc) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->reserved) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->reserved16) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Snd_output_cfg_rpc(XDR *xdrs, Snd_output_cfg_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->mute_num) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->mute_polar) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->dac_precision) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->dac_format) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->is_ext_dac) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->reserved8) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->gpio_mute_circuit) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->ext_mute_mode) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->enable_hw_accelerator) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->chip_type_config) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->reserved) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Snd_feature_config_rpc(XDR *xdrs, Snd_feature_config_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->support_spdif_mute) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->swap_lr_channel) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->conti_clk_while_ch_chg) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->support_desc) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Hld_device_rpc(XDR *xdrs, Hld_device_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    if (XDR_Uint32(xdrs, &cp->HLD_DEV) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->type) == False)
    {
        return False;
    }
    for (i = 0; i < 16; i++)
    {
        if (XDR_Uchar(xdrs, &cp->name[i]) == False)
        {
            return False;
        }
    }
    return True;
}


Bool XDR_Pcm_output_rpc(XDR *xdrs, Pcm_output_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->ch_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_mod) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->samp_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sample_rata_id) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->inmode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_left) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_right) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_sl) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_sr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_c) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_lfe) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_dl) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_dr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->raw_data_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->iec_pc) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->raw_data_ddp_start) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->raw_data_ddp_len) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->iec_pc_ddp) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Control_block_rpc(XDR *xdrs, Control_block_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->stc_id_valid) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pts_valid) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->data_continue) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ctrlblk_valid) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->instant_update) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->vob_start) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bstream_run_back) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->reserve) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->stc_id) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->stc_offset_idx) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pts) == False)
    {
        return False;
    }

    return True;

}


Bool XDR_Pe_music_cfg_rpc(XDR *xdrs, Pe_music_cfg_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->pcm_out_buff) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pcm_out_buff_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->processed_pcm_buff) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->processed_pcm_buff_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mp_cb) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_MusicInfo_rpc(XDR *xdrs, MusicInfo_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;

    for (i = 0; i < 30; i++)
    {
        if (XDR_Char(xdrs, &cp->title[i]) == False)
        {
            return False;
        }
    }

    for (i = 0; i < 30; i++)
    {
        if (XDR_Char(xdrs, &cp->artist[i]) == False)
        {
            return False;
        }
    }

    for (i = 0; i < 30; i++)
    {
        if (XDR_Char(xdrs, &cp->album[i]) == False)
        {
            return False;
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (XDR_Char(xdrs, &cp->year[i]) == False)
        {
            return False;
        }
    }

    for (i = 0; i < 30; i++)
    {
        if (XDR_Char(xdrs, &cp->comment[i]) == False)
        {
            return False;
        }
    }

    if (XDR_Char(xdrs, &cp->genre) == False)
    {
        return False;
    }
    if (XDR_Char(xdrs, &cp->track) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->time) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->file_length) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_DecoderInfo_rpc(XDR *xdrs, DecoderInfo_rpc *cp, Uint32 cnt)
{
    if (XDR_Ulong(xdrs, &cp->bit_rate) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->sample_rate) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->channel_mode) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Image_info_rpc(XDR *xdrs, Image_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Ulong(xdrs, &cp->fsize) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->width) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->height) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->bbp) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->type) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Image_init_config_rpc(XDR *xdrs, Image_init_config_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->frm_y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_c_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_c_size) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm2_y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_c_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_c_size) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm_mb_type) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm3_y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm3_y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm3_c_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm3_c_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm4_y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm4_y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm4_c_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm4_c_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoder_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoder_buf_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->vbv_buf) == False)
    {
        return False;
    }
	
    if (XDR_Uint32(xdrs, &cp->vbv_buf_len) == False)
    {
        return False;
    }
	
    if (XDR_Int32(xdrs, &cp->img_fmt) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->pkt_sbm_idx) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->info_sbm_idx) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->info_end_idx) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Image_hw_info_rpc(XDR *xdrs, Image_hw_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->c_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->c_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->width) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->height) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->w_stride) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->sample_format) == False)
    {
        return False;
    }

    return True;
}



Bool XDR_Pe_image_cfg_rpc(XDR *xdrs, Pe_image_cfg_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->frm_y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_c_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_c_size) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm2_y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_c_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_c_size) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm_mb_type) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm3_y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm3_y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm3_c_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm3_c_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm4_y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm4_y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm4_c_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm4_c_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoder_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoder_buf_len) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->mp_cb) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Image_info_pe_rpc(XDR *xdrs, Image_info_pe_rpc *cp, Uint32 cnt)
{
    if (XDR_Ulong(xdrs, &cp->fsize) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->width) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->height) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->bbp) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Imagedec_frm_rpc(XDR *xdrs, Imagedec_frm_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->frm_y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_y_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_c_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_c_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->busy) == False)
    {
        return False;
    }

    return True;
}



Bool XDR_Pe_video_cfg_rpc(XDR *xdrs, Pe_video_cfg_rpc *cp, Uint32 cnt)
{
    if (XDR_Ulong(xdrs, &cp->mp_cb) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoder_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoder_buf_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mm_vbv_len) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->disable_seek) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->reserved) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_DEC_CHAPTER_INFO_RPC(XDR *xdrs, DEC_CHAPTER_INFO_RPC *cp, Uint32 cnt)
{
    Uchar i = 0;

    if (XDR_Ulong(xdrs, &cp->nb_chapter) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->cur_chapter) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->cur_start_time) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->cur_end_time) == False)
    {
        return False;
    }
    for (i = 0; i < 10; i++)
    {
        if (XDR_Uchar(xdrs, &cp->cur_title[i]) == False)
        {
            return False;
        }
    }

    if (XDR_Int32(xdrs, &cp->dst_chapter) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->dst_start_time) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->dst_end_time) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->dst_end_time) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Fileinfo_video_rpc(XDR *xdrs, Fileinfo_video_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;

    for (i = 0; i < 10; i++)
    {
        if (XDR_Char(xdrs, &cp->VideoDec[i]) == False)
        {
            return False;
        }
    }

    for (i = 0; i < 10; i++)
    {
        if (XDR_Char(xdrs, &cp->AudioDec[i]) == False)
        {
            return False;
        }
    }

    if (XDR_Ulong(xdrs, &cp->AudioStreamNum) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->SubStreamNum) == False)
    {
        return False;
    }

    if (XDR_Ulong(xdrs, &cp->TotalFrameNum) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->FramePeriod) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->TotalTime) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->width) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->height) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->VideoBitrate) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->AudioBitrate) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->AudioChannelNum) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->fsize) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->AudioSampleRate) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->video_codec_tag) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->cur_audio_stream_id) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->cur_sub_stream_id) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->cur_prog_id) == False)
    {
        return False;
    }
    if (XDR_Ulong(xdrs, &cp->ProgNum) == False)
    {
        return False;
    }

    return True;
}



Bool XDR_Pe_cache_ex_rpc(XDR *xdrs, Pe_cache_ex_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->status) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->sub_status) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->mutex) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cache_buff) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->cache_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->data_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->rd_pos) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->wr_pos) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->file_offset) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Pe_cache_cmd_rpc(XDR *xdrs, Pe_cache_cmd_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;

    if (XDR_Uint32(xdrs, &cp->status) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->type) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->reserved) == False)
    {
        return False;
    }

    for (i = 0; i < 10; i++)
    {
        if (XDR_Uint32(xdrs, &cp->param[i]) == False)
        {
            return False;
        }
    }

    return True;
}






Bool XDR_Audio_config_rpc(XDR *xdrs, Audio_config_rpc *cp, Uint32 cnt)
{
    Uint32 i = 0;

    if (XDR_Int32(xdrs, &cp->decode_mode) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->sync_mode) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->sync_unit) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->deca_input_sbm) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->deca_output_sbm) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->snd_input_sbm) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->pcm_sbm) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->codec_id) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->bits_per_coded_sample) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->sample_rate) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->channels) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->bit_rate) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->pcm_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pcm_buf_size) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->block_align) == False)
    {
        return False;
    }
    #if 0
    for (i = 0; i < 512; i++)
    {
        if (XDR_Uchar(xdrs, &cp->extra_data[i]) == False)
        {
            return False;
        }
    }
    #endif
    if (XDR_Int32(xdrs, &cp->codec_frame_size) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->extradata_size) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->extradata_mode) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->cloud_game_prj) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Audio_decore_status_rpc(XDR *xdrs, Audio_decore_status_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->sample_rate) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->channels) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->bits_per_sample) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->first_header_got) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->first_header_parsed) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frames_decoded) == False)
    {
        return False;
    }
    return True;
}


Bool XDR_Snd_dev_status_rpc(XDR *xdrs, Snd_dev_status_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->flags) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->volume) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->in_mute) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->spdif_out) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->trackmode) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->samp_rate) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->samp_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ch_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->drop_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->play_cnt) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->pcm_out_frames) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pcm_dma_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pcm_dma_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pcm_rd) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pcm_wt) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->underrun_cnts) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pcm_dump_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pcm_dump_len) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Spec_param_rpc(XDR *xdrs, Spec_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->spec_call_back) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->collumn_num) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Spec_step_table_rpc(XDR *xdrs, Spec_step_table_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->column_num) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ptr_table) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Snd_spdif_scms_rpc(XDR *xdrs, Snd_spdif_scms_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->copyright) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->reserved) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->l_bit) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->category_code) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->reserved16) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Snd_sync_param_rpc(XDR *xdrs, Snd_sync_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->drop_threshold) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->wait_threshold) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->delay_video_sthreshold) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Isdbtcc_config_par_rpc(XDR *xdrs, Isdbtcc_config_par_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->g_buf_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->g_buf_len) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->max_isdbtcc_height) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->isdbtcc_osd_layer_id) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->isdbtcc_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->isdbtcc_height) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->isdbtcc_hor_offset) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->isdbtcc_ver_offset) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->osd_isdbtcc_enter) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->osd_isdbtcc_leave) == False)
    {
        return False;
    }

    return True;
}



Bool XDR_Atsc_subt_config_par_rpc(XDR *xdrs, Atsc_subt_config_par_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->bs_buf_addr) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->bs_buf_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sec_buf_addr) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->sec_buf_len) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->Outline_thickness_from_stream) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->DropShadow_right_from_stream) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->DropShadow_bottom_from_stream) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->Outline_thickness) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->DropShadow_right) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->DropShadow_bottom) == False)
    {
        return False;
    }

    return True;
}



Bool XDR_Sdec_feature_config_rpc(XDR *xdrs, Sdec_feature_config_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->temp_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->temp_buf_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->bs_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->bs_buf_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->bs_hdr_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->bs_hdr_buf_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pixel_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pixel_buf_len) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->tsk_qtm) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->tsk_pri) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->transparent_color) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->support_hw_decode) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->sdec_hw_buf) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->sdec_hw_buf_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->subt_draw_pixelmap) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->subt_draw_pixel) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->region_is_created) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->subt_create_region) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->subt_region_show) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->subt_delete_region) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->subt_get_region_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->draw_obj_hardware) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->subt_display_define) == False)
    {
        return False;
    }

    return True;
}



Bool XDR_Subt_config_par_rpc(XDR *xdrs, Subt_config_par_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->cc_by_vbi) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->cc_by_osd) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->osd_blocklink_enable) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->g_ps_buf_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->g_ps_buf_len) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->sys_sdram_size) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->max_subt_height) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->speed_up_subt_enable) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->osd_layer_id) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->hd_subtitle_support) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->subt_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->subt_height) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->subt_hor_offset) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->subt_ver_offset) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->osd_subt_enter) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->osd_subt_leave) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->osd_get_scale_para) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_AUDIO_INFO_rpc(XDR *xdrs, AUDIO_INFO_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->bit_rate) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sample_freq) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->layer) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->id) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->mpeg25) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->frm_size) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Reverb_param_rpc(XDR *xdrs, Reverb_param_rpc *cp, Uint32 cnt)
{
    if (XDR_UInt16(xdrs, &cp->enable) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->reverb_mode) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Pl_ii_param_rpc(XDR *xdrs, Pl_ii_param_rpc *cp, Uint32 cnt)
{
    if (XDR_UInt16(xdrs, &cp->enable) == False)
    {
        return False;
    }

    if (XDR_Int16(xdrs, &cp->abaldisable) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->chanconfig) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->dimset) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->surfiltenable) == False)
    {
        return False;
    }

    if (XDR_Int16(xdrs, &cp->modeselect) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->panoramaenable) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->pcmscalefac) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->rsinvenable) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->cwidthset) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Io_param_rpc(XDR *xdrs, Io_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->io_buff_in) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->buff_in_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->io_buff_out) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->buff_out_len) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Cur_stream_info_rpc(XDR *xdrs, Cur_stream_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->str_type) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bit_depth) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sample_rate) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->samp_num) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->chan_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->reserved1) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->reserved2) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->input_ts_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sync_error_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sync_success_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sync_frm_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->decode_error_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decode_success_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cur_frm_pts) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Deca_buf_info_rpc(XDR *xdrs, Deca_buf_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->buf_base_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->buf_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->used_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->remain_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->cb_rd) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cb_wt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->es_rd) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->es_wt) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Ase_str_play_param_rpc(XDR *xdrs, Ase_str_play_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->src) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->loop_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->loop_interval) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->async_play) == False)
    {
        return False;
    }
	if (XDR_Uint32(xdrs, &cp->reserved) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->need_notify) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->registered_cb) == False)
    {
        return False;
    }
    
    return True;
}


Bool XDR_Position_rpc(XDR *xdrs, Position_rpc *cp, Uint32 cnt)
{
    if (XDR_UInt16(xdrs, &cp->uX) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->uY) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_RectSize_rpc(XDR *xdrs, RectSize_rpc *cp, Uint32 cnt)
{
    if (XDR_UInt16(xdrs, &cp->uWidth) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->uHeight) == False)
    {
        return False;
    }

    return True;
}


#if 0
Bool XDR_YCbCrColor_rpc(XDR *xdrs, YCbCrColor_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->uY) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->uCb) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->uCr) == False)
    {
        return False;
    }

    return True;
}
#endif

Bool XDR_AdvSetting_rpc(XDR *xdrs, AdvSetting_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->init_mode) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->out_sys) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->bprogressive) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->switch_mode) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_VDecPIPInfo_rpc(XDR *xdrs, Vdecpipinfo_rpc *cp, Uint32 cnt)
{
    if (XDR_Position_rpc(xdrs, &cp->pipStaPos, sizeof(Position_rpc)) == False)
    {
        return False;
    }
    if (XDR_RectSize_rpc(xdrs, &cp->pipSize, sizeof(RectSize_rpc)) == False)
    {
        return False;
    }
    if (XDR_RectSize_rpc(xdrs, &cp->mpSize, sizeof(RectSize_rpc)) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->bUseBgColor) == False)
    {
        return False;
    }
    if (XDR_Ycbcrcolor_rpc(xdrs, &cp->bgColor, sizeof(Ycbcrcolor_rpc)) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->buse_sml_buf) == False)
    {
        return False;
    }


    if (XDR_Rect_rpc(xdrs, &cp->src_rect, sizeof(Rect_rpc)) == False)
    {
        return False;
    }
    if (XDR_Rect_rpc(xdrs, &cp->dst_rect, sizeof(Rect_rpc)) == False)
    {
        return False;
    }
    if (XDR_AdvSetting_rpc(xdrs, &cp->adv_setting, sizeof(AdvSetting_rpc)) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_VDec_StatusInfo_rpc(XDR *xdrs, Vdec_statusinfo_rpc *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->uCurStatus) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->uFirstPicShowed) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->bFirstHeaderGot) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pic_width) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->pic_height) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->status_flag) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->read_p_offset) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->write_p_offset) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->display_idx) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->use_sml_buf) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->output_mode) == False)
    {
        return False;
    }

#ifdef VIDEO_SECOND_B_MONITOR
    if (XDR_Uchar(xdrs, &cp->mb_x) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->mb_y) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoding_head_idx) == False)
    {
        return False;
    }
#endif

    if (XDR_Uint32(xdrs, &cp->valid_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->MPEG_format) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->aspect_ratio) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->frame_rate) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->bit_rate) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->hw_dec_error) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->display_frm) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->top_cnt) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->play_direction) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->play_speed) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->api_play_direction) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->api_play_speed) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->is_support) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vbv_size) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->cur_dma_ch) == False)
    {
        return False;
    }

    return True;
}



Bool XDR_Imagedec_show_shutters_rpc(XDR *xdrs, Imagedec_show_shutters_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->direction) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->type) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->time) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Imagedec_show_brush_rpc(XDR *xdrs, Imagedec_show_brush_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->direction) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->type) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->time) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Imagedec_show_slide_rpc(XDR *xdrs, Imagedec_show_slide_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->direction) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->type) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->time) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Imagedec_show_show_random_rpc(XDR *xdrs, Imagedec_show_show_random_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->type) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->res) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->time) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Imagedec_show_fade_rpc(XDR *xdrs, Imagedec_show_fade_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->type) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->res) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->time) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Image_slideshow_effect_rpc(XDR *xdrs, Image_slideshow_effect_rpc *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->mode) == False)
    {
        return False;
    }

    if (XDR_Imagedec_show_shutters_rpc(xdrs, &cp->shuttles_param, sizeof(Imagedec_show_shutters_rpc)) == False)
    {
        return False;
    }
    if (XDR_Imagedec_show_brush_rpc(xdrs, &cp->brush_param, sizeof(Imagedec_show_brush_rpc)) == False)
    {
        return False;
    }
    if (XDR_Imagedec_show_slide_rpc(xdrs, &cp->slide_param, sizeof(Imagedec_show_slide_rpc)) == False)
    {
        return False;
    }

    if (XDR_Imagedec_show_show_random_rpc(xdrs, &cp->random_param, sizeof(Imagedec_show_show_random_rpc)) == False)
    {
        return False;
    }
    if (XDR_Imagedec_show_fade_rpc(xdrs, &cp->fade_param, sizeof(Imagedec_show_fade_rpc)) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Image_config_rpc(XDR *xdrs, Image_config_rpc *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->file_name) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->decode_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->show_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->vpo_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->rotate) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->src_left) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_top) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_height) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->dest_left) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dest_top) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dest_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dest_height) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->mp_cb) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Image_engine_config_rpc(XDR *xdrs, Image_engine_config_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->decode_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->show_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->vpo_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->rotate) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->src_left) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_top) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_height) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->dest_left) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dest_top) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dest_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dest_height) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->img_fmt) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Image_display_rpc(XDR *xdrs, Image_display_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->decode_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->show_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->vpo_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->rotate) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->src_left) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_top) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_height) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->dest_left) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dest_top) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dest_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dest_height) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->img_fmt) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->y_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->y_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->c_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->c_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->width) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->height) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->sample_format) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->mp_cb) == False)
    {
        return False;
    }
    return True;
}


/* Video Encoder added by Vic_Zhang_on_20131005. */
Bool XDR_VidencSeeConfig_t(XDR *xdrs, VidencSeeConfig_t *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->buffer_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->buffer_size) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->y_sbm_idx) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->c_sbm_idx) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->status_idx) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->yuv_width) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->yuv_height) == False)
    {
        return False;
    }

    return True;
}
Bool XDR_VidencTriggerPara_t(XDR *xdrs, VidencTriggerPara_t *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->frm_width) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm_height) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->Y_length) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->C_length) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->encoder_ID) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->job_status) == False)
    {
        return False;
    }

    return True;
}


/********************* decv ********************************/

Bool XDR_Vdec_adpcm_rpc(XDR *xdrs, Vdec_adpcm_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->adpcm_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->adpcm_ratio) == False)
    {
        return False;
    }

    return True;
}
Bool XDR_Vdec_sml_frm_rpc(XDR *xdrs, Vdec_sml_frm_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->sml_frm_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sml_frm_size) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_mem_map_rpc(XDR *xdrs, Vdec_mem_map_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->frm0_Y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm0_C_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm1_Y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm1_C_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_Y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_C_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm0_Y_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm0_C_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm1_Y_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm1_C_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_Y_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm2_C_start_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->dvw_frm_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->dvw_frm_start_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->maf_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->maf_start_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->vbv_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vbv_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vbv_end_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm3_Y_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm3_C_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm3_Y_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frm3_C_start_addr) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frm_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->res_bits) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->res_pointer) == False)
    {
        return False;
    }

    return True;
}
Bool XDR_Vdec_config_par_rpc(XDR *xdrs, Vdec_config_par_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->user_data_parsing) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->dtg_afd_parsing) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->drop_freerun_pic_before_firstpic_show) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->reset_hw_directly_when_stop) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->show_hd_service) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->still_frm_in_cc) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->extra_dview_window) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->not_show_mosaic) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->return_multi_freebuf) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->advance_play) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->lm_shiftthreshold) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->vp_init_phase) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->preview_solution) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->res_bits) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->res_pointer) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_avs_memmap_rpc(XDR *xdrs, Vdec_avs_memmap_rpc *cp, Uint32 cnt)
{
    if (XDR_Bool(xdrs, &cp->support_multi_bank) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frame_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frame_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->dv_frame_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->dv_frame_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->mv_buffer_base) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->mv_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->mb_col_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mb_col_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->mb_neighbour_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mb_neighbour_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->cmd_queue_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cmd_queue_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->vbv_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vbv_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->laf_rw_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->laf_rw_buffer_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->laf_flag_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->laf_flag_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->support_conti_memory) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->avs_mem_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->avs_mem_len) == False)
    {
        return False;
    }
    return True;
}

Bool XDR_Vdec_avc_memmap_rpc(XDR *xdrs, Vdec_avc_memmap_rpc *cp, Uint32 cnt)
{
    if (XDR_Bool(xdrs, &cp->support_multi_bank) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->frame_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frame_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->dv_frame_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->dv_frame_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->mv_buffer_base) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->mv_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->mb_col_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mb_col_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->mb_neighbour_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mb_neighbour_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->cmd_queue_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cmd_queue_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->vbv_buffer_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vbv_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->laf_rw_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->laf_rw_buffer_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->laf_flag_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->laf_flag_buffer_len) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->support_conti_memory) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->avc_mem_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->avc_mem_len) == False)
    {
        return False;
    }
    return True;
}

Bool XDR_Vdec_avc_config_par_rpc(XDR *xdrs, Vdec_avc_config_par_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->max_additional_fb_num) == False)
    {
        return False;
    }
#ifdef VIDEO_SEAMLESS_SWITCHING
    if (XDR_Uchar(xdrs, &cp->seamless_enable) == False)
    {
        return False;
    }
#endif
    if (XDR_Uchar(xdrs, &cp->dtg_afd_parsing) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Rect_rpc(XDR *xdrs, Rect_rpc *cp, Uint32 cnt)
{
#if (defined(_MHEG5_ENABLE_) || defined(_MHEG5_V20_ENABLE_) )
    if (XDR_Int16(xdrs, &cp->uStartX) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->uStartY) == False)
    {
        return False;
    }
#else
    if (XDR_UInt16(xdrs, &cp->uStartX) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->uStartY) == False)
    {
        return False;
    }

#endif
    if (XDR_UInt16(xdrs, &cp->uWidth) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->uHeight) == False)
    {
        return False;
    }
    return True;
}

Bool XDR_Vdec_dvrconfigparam_rpc(XDR *xdrs, Vdec_dvrconfigparam_rpc *cp, Uint32 cnt)
{
    if (XDR_Bool(xdrs, &cp->is_scrambled) == False)
    {
        return False;
    }
    return True;
}
Bool XDR_Vdecpipinfo_rpc(XDR *xdrs, Vdecpipinfo_rpc *cp, Uint32 cnt)
{
    if (XDR_UInt16(xdrs, &cp->pipStaPos.uX) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pipStaPos.uY) == False)
    {
        return False;
    }
#if (defined(_MHEG5_ENABLE_) || defined(_MHEG5_V20_ENABLE_) )
    if (XDR_Int16(xdrs, &cp->pipSize.uStartX) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->pipSize.uStartY) == False)
    {
        return False;
    }
#else
    if (XDR_UInt16(xdrs, &cp->pipSize.uStartX) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pipSize.uStartY) == False)
    {
        return False;
    }
#endif
    if (XDR_UInt16(xdrs, &cp->pipSize.uWidth) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pipSize.uHeight) == False)
    {
        return False;
    }

#if (defined(_MHEG5_ENABLE_) || defined(_MHEG5_V20_ENABLE_) )
    if (XDR_Int16(xdrs, &cp->mpSize.uStartX) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->mpSize.uStartY) == False)
    {
        return False;
    }
#else
    if (XDR_UInt16(xdrs, &cp->mpSize.uStartX) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->mpSize.uStartY) == False)
    {
        return False;
    }
#endif
    if (XDR_UInt16(xdrs, &cp->mpSize.uWidth) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->mpSize.uHeight) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->bUseBgColor) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->bgColor.uY) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bgColor.uCb) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bgColor.uCr) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->buse_sml_buf) == False)
    {
        return False;
    }

#if (defined(_MHEG5_ENABLE_) || defined(_MHEG5_V20_ENABLE_) )
    if (XDR_Int16(xdrs, &cp->src_rect.uStartX) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->src_rect.uStartY) == False)
    {
        return False;
    }
#else
    if (XDR_UInt16(xdrs, &cp->src_rect.uStartX) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_rect.uStartY) == False)
    {
        return False;
    }
#endif
    if (XDR_UInt16(xdrs, &cp->src_rect.uWidth) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->src_rect.uHeight) == False)
    {
        return False;
    }

#if (defined(_MHEG5_ENABLE_) || defined(_MHEG5_V20_ENABLE_) )
    if (XDR_Int16(xdrs, &cp->dst_rect.uStartX) == False)
    {
        return False;
    }
    if (XDR_Int16(xdrs, &cp->dst_rect.uStartY) == False)
    {
        return False;
    }
#else
    if (XDR_UInt16(xdrs, &cp->dst_rect.uStartX) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dst_rect.uStartY) == False)
    {
        return False;
    }
#endif
    if (XDR_UInt16(xdrs, &cp->dst_rect.uWidth) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dst_rect.uHeight) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->adv_setting.init_mode) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->adv_setting.out_sys) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->adv_setting.bprogressive) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->adv_setting.switch_mode) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Mpsource_callback_rpc(XDR *xdrs, Mpsource_callback_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->handler) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->RequestCallback) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ReleaseCallback) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vblanking_callback) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Pipsource_callback_rpc(XDR *xdrs, Pipsource_callback_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->RequestCallback) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ReleaseCallback) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vblanking_callback) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->handler) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_statusinfo_rpc(XDR *xdrs, Vdec_statusinfo_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->uCurStatus) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->uFirstPicShowed) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bFirstHeaderGot) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pic_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pic_height) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->status_flag) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->read_p_offset) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->write_p_offset) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->display_idx) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->use_sml_buf) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->output_mode) == False)
    {
        return False;
    }
#ifdef VIDEO_SECOND_B_MONITOR
    if (XDR_Uchar(xdrs, &cp->mb_x) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->mb_y) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoding_head_idx) == False)
    {
        return False;
    }
#endif
    if (XDR_Uint32(xdrs, &cp->valid_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->MPEG_format) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->aspect_ratio) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->frame_rate) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->bit_rate) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->hw_dec_error) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->display_frm) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->top_cnt) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->play_direction) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->play_speed) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->api_play_direction) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->api_play_speed) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->is_support) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vbv_size) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->cur_dma_ch) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_decore_status_rpc(XDR *xdrs, Vdec_decore_status_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->decode_status) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pic_width) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pic_height) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sar_width) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sar_heigth) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frame_rate) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->interlaced_frame) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->top_field_first) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->first_header_got) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->first_pic_showed) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frames_decoded) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frames_displayed) == False)
    {
        return False;
    }
    if (XDR_Long(xdrs, &cp->frame_last_pts) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->buffer_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->buffer_used) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decode_error) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoder_feature) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->under_run_cnt) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->first_pic_decoded) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->output_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->frame_angle) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_io_get_frm_para_rpc(XDR *xdrs, Vdec_io_get_frm_para_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->ufrm_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->tFrmInfo.uY_Addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->tFrmInfo.uC_Addr) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->tFrmInfo.uWidth) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->tFrmInfo.uHeight) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->tFrmInfo.uY_Size) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->tFrmInfo.uC_Size) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ycbcrcolor_rpc(XDR *xdrs, Ycbcrcolor_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->uY) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->uCb) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->uCr) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_io_reg_callback_para_rpc(XDR *xdrs, Vdec_io_reg_callback_para_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->eCBType) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pCB) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_picture_rpc(XDR *xdrs, Vdec_picture_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->type) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->out_data_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->out_data_buf_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->out_data_buf_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->out_data_valid_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pic_width) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pic_height) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_io_dbg_flag_info_rpc(XDR *xdrs, Vdec_io_dbg_flag_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->dbg_flag) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->active_flag) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->unique_flag) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Video_rect_rpc(XDR *xdrs, Video_rect_rpc *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->x) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->y) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->w) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->h) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_OutputFrmManager_rpc(XDR *xdrs, OutputFrmManager_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->display_index[0]) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->display_index[1]) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->de_last_request_frm_array[0]) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->de_last_request_frm_array[1]) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->de_last_release_frm_array[0]) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->de_last_release_frm_array[1]) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->de_last_request_idx[0]) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->de_last_request_idx[1]) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->de_last_release_idx[0]) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->de_last_release_idx[1]) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->last_output_pic_released[0]) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->last_output_pic_released[1]) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->de_last_release_poc[0]) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->de_last_release_poc[1]) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->de_last_request_poc[0]) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->de_last_request_poc[1]) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->frm_number) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pip_frm_number) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdecinit_rpc(XDR *xdrs, Vdecinit_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    if (XDR_Ulong(xdrs, &cp->fmt_out.fourcc) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_out.bits_per_pixel) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_out.pic_width) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_out.pic_height) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_out.pic_inverted) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_out.pixel_aspect_x) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_out.pixel_aspect_y) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_out.frame_rate) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_out.frame_period) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->fmt_out.frame_period_const) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->fmt_in.fourcc) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_in.bits_per_pixel) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_in.pic_width) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_in.pic_height) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_in.pic_inverted) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_in.pixel_aspect_x) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_in.pixel_aspect_y) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_in.frame_rate) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fmt_in.frame_period) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->fmt_in.frame_period_const) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->pfrm_buf.FBStride) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pfrm_buf.FBMaxHeight) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pfrm_buf.FrmBuf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pfrm_buf.DispBuf) == False)
    {
        return False;
    }

    for (i = 0; i < 6; i++)
    {
        if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.fb_y[i]) == False)
        {
            return False;
        }
    }
    for (i = 0; i < 6; i++)
    {
        if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.fb_c[i]) == False)
        {
            return False;
        }
    }
    for (i = 0; i < 6; i++)
    {
        if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.dv_y[i]) == False)
        {
            return False;
        }
    }
    for (i = 0; i < 6; i++)
    {
        if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.dv_c[i]) == False)
        {
            return False;
        }
    }

    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.fb_max_stride) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.fb_max_height) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.dv_max_stride) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.dv_max_height) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.fb_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.neighbor_mem) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.colocate_mem) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.cmdq_base) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.cmdq_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.vbv_start) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->phw_mem_cfg.vbv_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->on_decode_event) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pfn_decore_de_request) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pfn_decore_de_release) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decoder_flag) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->decode_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->preview) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_playback_param_rpc(XDR *xdrs, Vdec_playback_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->direction) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->rate) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vdec_capture_frm_info_rpc(XDR *xdrs, Vdec_capture_frm_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->pic_height) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pic_width) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pic_stride) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->y_buf_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->y_buf_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->c_buf_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->c_buf_size) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->de_map_mode) == False)
    {
        return False;
    }

    return True;
}

/************************** avs *******************************/
Bool XDR_Avsync_cfg_param_t_rpc(XDR *xdrs, Avsync_cfg_param_t_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->vhold_thres) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vdrop_thres) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ahold_thres) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->adrop_thres) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->sync_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->src_type) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Avsync_adv_param_t_rpc(XDR *xdrs, Avsync_adv_param_t_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->afreerun_thres) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vfreerun_thres) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->disable_monitor) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->disable_first_video_freerun) == False)
    {
        return False;
    }
	
    if (XDR_UInt16(xdrs, &cp->dual_output_sd_delay) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pts_adjust_threshold) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->rsvd2) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->rsvd3) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Avsync_status_t_rpc(XDR *xdrs, Avsync_status_t_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->device_status) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->vpts_offset) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->apts_offset) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->v_sync_flg) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->a_sync_flg) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->rsvd0) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->rsvd1) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cur_vpts) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->cur_apts) == False)
    {
        return False;
    }


    return True;
}

Bool XDR_Avsync_statistics_t_rpc(XDR *xdrs, Avsync_statistics_t_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->total_v_play_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->total_v_drop_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->total_v_hold_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->total_v_freerun_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->total_a_play_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->total_a_drop_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->total_a_hold_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->total_a_freerun_cnt) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Avsync_smoothly_play_cfg_t_rpc(XDR *xdrs, Avsync_smoothly_play_cfg_t_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->onoff) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->interval) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->level) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Avsync_get_stc_en_t_rpc(XDR *xdrs, Avsync_get_stc_en_t_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->sharem_addr) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->enable) == False)
    {
        return False;
    }
    return True;
}

/************************** dmx *******************************/
Bool XDR_Io_param_ex_rpc(XDR *xdrs, Io_param_ex_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->io_buff_in) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->buff_in_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->io_buff_out) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->buff_out_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->hnd) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->h264_flag) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->is_scrambled) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->record_all) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Pes_pkt_param_rpc(XDR *xdrs, Pes_pkt_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->stream_id) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pes_pkt_len) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->filled_byte_3) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pes_scramb_ctrl) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->marker_10) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->filled_byte_4) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pts_dts_flags) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pes_header_data_len) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pes_data_len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->dmx_state) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->stream_type) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->head_buf_pos) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->av_buf) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->av_buf_pos) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->av_buf_len) == False)
    {
        return False;
    }
    if (XDR_Control_block_rpc(xdrs, &cp->ctrl_blk, sizeof(Control_block_rpc)) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->device) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->request_write) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->update_write) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->get_pkt_len) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->get_header_data_len) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->get_pts) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->str_confirm) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->reserved) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->conti_conter) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->head_buf) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ch_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->channel) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->cw_parity_num) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->xfer_es_by_dma) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->dma_ch_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->last_dma_xfer_id) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ts_err_code) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ovlp_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->discont_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->LastTsAdaCtrl) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->unlock_cnt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->new_vbv_method_enable) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->new_vbv_request) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->new_vbv_update) == False)
    {
        return False;
    }

    return True;
}

/************************** vbi *******************************/
Bool XDR_Vbi_config_par_rpc(XDR *xdrs, Vbi_config_par_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->ttx_by_vbi) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->cc_by_vbi) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->vps_by_vbi) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->wss_by_vbi) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->hamming_8_4_enable) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->hamming_24_16_enable) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->erase_unknown_packet) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->parse_packet26_enable) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_sub_page) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->user_fast_text) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.sbf_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.sbf_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.data_hdr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.pbf_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.pbf_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.sub_page_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.sub_page_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.sbf_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.p26_nation_buf_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.p26_nation_buf_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.p26_data_buf_start_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->mem_map.p26_data_buf_size) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ttx_config_par_rpc(XDR *xdrs, Ttx_config_par_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->erase_unknown_packet) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_sub_page) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->parse_packet26_enable) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->user_fast_text) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->no_ttx_descriptor) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->sys_sdram_size_2m) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->hdtv_support_enable) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_vscrbuf) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_pallette) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_cyrillic_1_support) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_cyrillic_2_support) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_cyrillic_3_support) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_greek_support) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_arabic_support) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_hebrew_support) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_cyrillic_g2_support) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_greek_g2_support) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_arabic_g2_support) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->ttx_g3_support) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->ttx_color_number) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ttx_subpage_addr) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->osd_layer_id) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_cyrillic_1) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_cyrillic_2) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_cyrillic_3) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_greek) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_arabic) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_hebrew) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_g2) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_cyrillic_g2) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_greek_g2) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_arabic_g2) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->get_ttxchar_from_g3) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ttx_drawchar) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->osd_get_scale_para) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ttx_page_info_rpc(XDR *xdrs, Ttx_page_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->page_addr) == False)
    {
        return False;
    }

    return True;
}

/************************** ce *******************************/
Bool XDR_Ce_data_info_rpc(XDR *xdrs, Ce_data_info_rpc *cp, Uint32 cnt)
{
    Uint32 i = 0;
    if (XDR_Char(xdrs, &cp->otp_info.otp_addr) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->otp_info.otp_key_pos) == False)
    {
        return False;
    }

    for (i = 0; i < 4; i++)
    {
        if (XDR_Uint32(xdrs, &cp->data_info.crypt_data[i]) == False)
        {
            return False;
        }
    }
    if (XDR_Uint32(xdrs, &cp->data_info.data_len) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->des_aes_info.crypt_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->des_aes_info.aes_or_des) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->des_aes_info.des_low_or_high) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->key_info.first_key_pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_info.second_key_pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_info.hdcp_mode) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Otp_param_rpc(XDR *xdrs, Otp_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Char(xdrs, &cp->otp_addr) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->otp_key_pos) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Data_param_rpc(XDR *xdrs, Data_param_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    for (i = 0; i < 4; i++)
    {
        if (XDR_Uint32(xdrs, &cp->crypt_data[i]) == False)
        {
            return False;
        }
    }
    if (XDR_Uint32(xdrs, &cp->data_len) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Des_param_rpc(XDR *xdrs, Des_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->crypt_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->aes_or_des) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->des_low_or_high) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_key_param_rpc(XDR *xdrs, Ce_key_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->first_key_pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->second_key_pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->hdcp_mode) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_debug_key_info_rpc(XDR *xdrs, Ce_debug_key_info_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;

    if (XDR_Enum(xdrs, &cp->sel) == False)
    {
        return False;
    }

    for (i = 0; i < 4; i++)
    {
        if (XDR_Uint32(xdrs, &cp->buffer[i]) == False)
        {
            return False;
        }
    }
    if (XDR_Uint32(xdrs, &cp->len) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_pos_status_param_rpc(XDR *xdrs, Ce_pos_status_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->pos) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->status) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_found_free_pos_param_rpc(XDR *xdrs, Ce_found_free_pos_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->pos) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->ce_key_level_r) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->number) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->root) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Ce_pvr_key_param_rpc(XDR *xdrs, Ce_pvr_key_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->input_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->second_pos) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->first_pos) == False)
    {
        return False;
    }

    return True;
}

/**************** dsc *************************/
Bool XDR_DeEncrypt_config_rpc(XDR *xdrs, DeEncrypt_config_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->do_encrypt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->dec_dev) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->Decrypt_Mode) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->dec_dmx_id) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->do_decrypt) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->enc_dev) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->Encrypt_Mode) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->enc_dmx_id) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Sha_init_param_rpc(XDR *xdrs, Sha_init_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->sha_work_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->sha_data_source) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->sha_buf) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Aes_init_param_rpc(XDR *xdrs, Aes_init_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->parity_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_from) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->scramble_control) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->stream_id) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->dma_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->residue_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->work_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cbc_cts_enable) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Des_init_param_rpc(XDR *xdrs, Des_init_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->parity_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_from) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->scramble_control) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->stream_id) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->dma_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->residue_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->work_mode) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->cbc_cts_enable) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Pid_param_rpc(XDR *xdrs, Pid_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->dmx_id) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pid) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->pos) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->key_addr) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Csa_init_param_rpc(XDR *xdrs, Csa_init_param_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    if (XDR_Enum(xdrs, &cp->version) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->dma_mode) == False)
    {
        return False;
    }
    for (i = 0; i < 4; i++)
    {
        if (XDR_Uint32(xdrs, &cp->Dcw[i]) == False)
        {
            return False;
        }
    }
    if (XDR_Uint32(xdrs, &cp->pes_en) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->parity_mode) == False)
    {
        return False;
    }
    if (XDR_Enum(xdrs, &cp->key_from) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->scramble_control) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->stream_id) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Dsc_pvr_key_param_rpc(XDR *xdrs, Dsc_pvr_key_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->input_addr) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->valid_key_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->current_key_num) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pvr_key_length) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pvr_user_key_pos) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->total_quantum_number) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->current_quantum_number) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ts_packet_number) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pvr_key_change_enable) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->stream_id) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pvr_first_key_pos) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Dsc_deen_parity_rpc(XDR *xdrs, Dsc_deen_parity_rpc *cp, Uint32 cnt)
{
	if (XDR_Uint32(xdrs, &cp->sub_module) == False)
	{
		return False;
	}
	if (XDR_Uint32(xdrs, &cp->priority) == False)
	{
		return False;
	}
	if (XDR_Uint32(xdrs, &cp->dev_ptr) == False)
	{
		return False;
	}

	return True;
}


Bool XDR_Key_param_rpc(XDR *xdrs, Key_param_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    Uchar j = 0;
    if (XDR_Uint32(xdrs, &cp->handle) == False)
    {
        return False;
    }
    
    if (XDR_Uint32(xdrs, &cp->pid_list) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->pid_len) == False)
    {
        return False;
    }
    
    if (XDR_Uint32(xdrs, &cp->p_aes_key_info) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->p_csa_key_info) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->p_des_key_info) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->key_length) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->p_aes_iv_info) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->p_des_iv_info) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->stream_id) == False)
    {
        return False;
    }
    
    if (XDR_Uint32(xdrs, &cp->init_vector) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->ctr_counter) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->force_mode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->pos) == False)
    {
        return False;
    }
    
    if (XDR_Uchar(xdrs, &cp->no_even) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->no_odd) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->not_refresh_iv) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Trng_data_rpc(XDR *xdrs, Trng_data_rpc *cp, Uint32 cnt)
{
    Uint32 i=0;
    
    for (i = 0; i < sizeof(Trng_data_rpc); i++)
    {
        if (XDR_Uchar(xdrs, &cp->data[i]) == False)
        {
            return False;
        }
    }
    
    return True;
}

Bool XDR_Sha_hash_rpc(XDR *xdrs, Sha_hash_rpc *cp, Uint32 cnt)
{
    Uint32 i=0;
    
    for (i = 0; i < sizeof(Sha_hash_rpc); i++)
    {
        if (XDR_Uchar(xdrs, &cp->hash[i]) == False)
        {
            return False;
        }
    }
    
    return True;
}

Bool XDR_Dsc_drv_ver_rpc(XDR *xdrs, Dsc_drv_ver_rpc *cp, Uint32 cnt)
{
    Uint32 i=0;
    
    for (i = 0; i < sizeof(Dsc_drv_ver_rpc); i++)
    {
        if (XDR_Uchar(xdrs, &cp->data[i]) == False)
        {
            return False;
        }
    }
    
    return True;
}

Bool XDR_Dsc_ver_chk_param_rpc(XDR *xdrs, Dsc_ver_chk_param_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->input_mem) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->len) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->chk_mode) == False)
    {
        return False;
    }	
    if (XDR_Uint32(xdrs, &cp->chk_id) == False)
    {
        return False;
    }		
    return True;
}

/*********************** dis ***************************************/
Bool XDR_Vp_feature_config_rpc(XDR *xdrs, Vp_feature_config_rpc *cp, Uint32 cnt)
{
    if (XDR_Bool(xdrs, &cp->bOsdMulitLayer) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bMHEG5Enable) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bAvoidMosaicByFreezScr) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bSupportExtraWin) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bOvershootSolution) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bP2NDisableMAF) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bADPCMEnable) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pMPGetMemInfo) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->pSrcAspRatioChange_CallBack) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Tve_feature_config_rpc(XDR *xdrs, Tve_feature_config_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    if (XDR_Uint32(xdrs, &cp->config) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->type_tve_adj) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->sys_tve_adj) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->value_tve_adj) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->type_tve_tbl) == False)
    {
        return False;
    }
    for (i = 0; i < 8; i++)
    {
        if (XDR_Bool(xdrs, &cp->tve_data_tbl[i].valid) == False)
        {
            return False;
        }
        if (XDR_Bool(xdrs, &cp->tve_data_tbl[i].value) == False)
        {
            return False;
        }
    }
    if (XDR_Uint32(xdrs, &cp->index_tve_tbl_all) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->index_tve_tbl_all_1) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->val_tve_tbl_all) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->type_tve_adj_adv) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->sys_tve_adj_adv) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->value_tve_adj_adv) == False)
    {
        return False;
    }

    return True;
}

/*
Bool XDR_Tve_feature_config_rpc(XDR *xdrs, Tve_feature_config_rpc *cp, Uint32 cnt)
{
         if (XDR_Uint32(xdrs, &cp->config) == False)
                return False;
         if (XDR_Uint32(xdrs, &cp->tve_adjust) == False)
                return False;

         if (XDR_Uint32(xdrs, &cp->tve_tbl) == False)
                return False;
         if (XDR_Uint32(xdrs, &cp->tve_tbl_all) == False)
                return False;
         if (XDR_Uint32(xdrs, &cp->tve_adjust_adv) == False)
                return False;

        return True;
}*/



Bool XDR_Osd_driver_config_rpc(XDR *xdrs, Osd_driver_config_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    Uchar j = 0;
    if (XDR_Uint32(xdrs, &cp->uMemBase) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->uMemSize) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bStaticBlock) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bDirectDraw) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bCacheable) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bVFilterEnable) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bP2NScaleInNormalPlay) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->bP2NScaleInSubtitlePlay) == False)
    {
        return False;
    }
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (XDR_UInt16(xdrs, &cp->uDViewScaleRatio[i][j]) == False)
            {
                return False;
            }
        }
    }
    for (i = 0; i < 4; i++)
    {
        if (XDR_Uchar(xdrs, &cp->af_par[i].id) == False)
        {
            return False;
        }
        if (XDR_Uchar(xdrs, &cp->af_par[i].vd) == False)
        {
            return False;
        }
        if (XDR_Uchar(xdrs, &cp->af_par[i].af) == False)
        {
            return False;
        }
        if (XDR_Uchar(xdrs, &cp->af_par[i].res) == False)
        {
            return False;
        }
    }

    if (XDR_Uint32(xdrs, &cp->uSDMemBase) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->uSDMemSize) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vp_win_config_para_rpc(XDR *xdrs, Vp_win_config_para_rpc *cp, Uint32 cnt)
{
    Uchar i = 0;
    if (XDR_Uchar(xdrs, &cp->source_number) == False)
    {
        return False;
    }
    for (i = 0; i < 4; i++)
    {
        if (XDR_Uint32(xdrs, &cp->source_info[i].src_module_devide_handle) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->source_info[i].handler) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->source_info[i].request_callback) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->source_info[i].release_callback) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->source_info[i].vblanking_callback) == False)
        {
            return False;
        }
        if (XDR_Uchar(xdrs, &cp->source_info[i].src_path_index) == False)
        {
            return False;
        }
        if (XDR_Uchar(xdrs, &cp->source_info[i].attach_source_index) == False)
        {
            return False;
        }
    }
    if (XDR_Uchar(xdrs, &cp->control_source_index) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->mainwin_source_index) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->window_number) == False)
    {
        return False;
    }
    for (i = 0; i < 4; i++)
    {
        if (XDR_Uchar(xdrs, &cp->window_parameter[i].source_index) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->window_parameter[i].display_layer) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->window_parameter[i].src_module_devide_handle) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->window_parameter[i].handler) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->window_parameter[i].request_callback) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->window_parameter[i].release_callback) == False)
        {
            return False;
        }
        if (XDR_Uint32(xdrs, &cp->window_parameter[i].vblanking_callback) == False)
        {
            return False;
        }

        if (XDR_Rect_rpc(xdrs, &cp->window_parameter[i].rect.dst_rect, sizeof(Rect_rpc)) == False)
        {
            return False;
        }
        if (XDR_Rect_rpc(xdrs, &cp->window_parameter[i].rect.dst_rect, sizeof(Rect_rpc)) == False)
        {
            return False;
        }
    }

    return True;
}

Bool XDR_Vpo_io_ttx_rpc(XDR *xdrs, Vpo_io_ttx_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->LineAddr) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->Addr) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->Data) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vpo_io_cc_rpc(XDR *xdrs, Vpo_io_cc_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->FieldParity) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->Data) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Dacindex_rpc(XDR *xdrs, Dacindex_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->uDacFirst) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->uDacSecond) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->uDacThird) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vp_dacinfo_rpc(XDR *xdrs, Vp_dacinfo_rpc *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->bEnable) == False)
    {
        return False;
    }
    if (XDR_Dacindex_rpc(xdrs, &cp->tDacIndex, sizeof(Dacindex_rpc)) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->eVGAMode) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->bProgressive) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Vp_io_reg_dac_para_rpc(XDR *xdrs, Vp_io_reg_dac_para_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->eDacType) == False)
    {
        return False;
    }
    if (XDR_Vp_dacinfo_rpc(xdrs, &cp->DacInfo, sizeof(Vp_dacinfo_rpc)) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vp_initinfo_rpc(XDR *xdrs, Vp_initinfo_rpc *cp, Uint32 cnt)
{
    if (XDR_Ycbcrcolor_rpc(xdrs, &cp->tInitColor, sizeof(Ycbcrcolor_rpc)) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->bBrightnessValue) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->fBrightnessValueSign) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->wContrastValue) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->fContrastSign) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->wSaturationValue) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->fSaturationValueSign) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->wSharpness) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->fSharpnessSign) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->wHueSin) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->fHueSinSign) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->wHueCos) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->fHueCosSign) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->eTVAspect) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->eDisplayMode) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->uNonlinearChangePoint) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->uPanScanOffset) == False)
    {
        return False;
    }

    Int32 i = 0;

    for (i = 0; i < 23; i++)
        if (XDR_Vp_dacinfo_rpc(xdrs, &cp->pDacConfig[i], sizeof(Vp_dacinfo_rpc)) == False)
        {
            return False;
        }

    if (XDR_Int32(xdrs, &cp->eTVSys) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->bWinOnOff) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->uWinMode) == False)
    {
        return False;
    }
    if (XDR_Rect_rpc(xdrs, &cp->tSrcRect, sizeof(Rect_rpc)) == False)
    {
        return False;
    }
    if (XDR_Rect_rpc(xdrs, &cp->DstRect, sizeof(Rect_rpc)) == False)
    {
        return False;
    }

    if (XDR_Mpsource_callback_rpc(xdrs, &cp->tMPCallBack, sizeof(Mpsource_callback_rpc)) == False)
    {
        return False;
    }
    if (XDR_Pipsource_callback_rpc(xdrs, &cp->tPIPCallBack, sizeof(Pipsource_callback_rpc)) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->pSrcChange_CallBack) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->device_priority) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bCCIR656Enable) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Vpo_io_set_parameter_rpc(XDR *xdrs, Vpo_io_set_parameter_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->changed_flag) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->fetch_mode_api_en) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->fetch_mode) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->dit_enable) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->vt_enable) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->vertical_2tap) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->edge_preserve_enable) == False)
    {
        return False;
    }

    return True;
}



Bool XDR_Vpo_io_video_enhance_rpc(XDR *xdrs, Vpo_io_video_enhance_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->changed_flag) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->grade) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Vpo_io_get_info_rpc(XDR *xdrs, Vpo_io_get_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->display_index) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->api_act) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->bprogressive) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->tvsys) == False)
    {
        return False;
    }


    if (XDR_Bool(xdrs, &cp->fetch_mode_api_en) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->fetch_mode) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->dit_enable) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->vt_enable) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->vertical_2tap) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->edge_preserve_enable) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->source_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->source_height) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->des_width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->des_height) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->preframe_enable) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->gma1_onoff) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->reg90) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->scart_16_9) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->mp_onoff) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_De2Hdmi_video_infor_rpc(XDR *xdrs, De2Hdmi_video_infor_rpc *cp, Uint32 cnt)
{
    if (XDR_Uint32(xdrs, &cp->tv_mode) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->width) == False)
    {
        return False;
    }
    if (XDR_UInt16(xdrs, &cp->height) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->format) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->scan_mode) == False)
    {
        return False;
    }
    if (XDR_Bool(xdrs, &cp->afd_present) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->output_aspect_ratio) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->active_format_aspect_ratio) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->av_chg_ste) == False)
    {
        return False;
    }
	if (XDR_Uint32(xdrs, &cp->ext_video_format) == False)
	{
		return False;
	}

	if (XDR_Uint32(xdrs, &cp->_4K_VIC_3D_structure) == False)
	{
		return False;
	}

	if (XDR_Uint32(xdrs, &cp->ext_data) == False)
	{
		return False;
	}

    return True;
}


Bool XDR_Vpo_io_cgms_info_rpc(XDR *xdrs, Vpo_io_cgms_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->cgms) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->aps) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Vp_io_afd_para_rpc(XDR *xdrs, Vp_io_afd_para_rpc *cp, Uint32 cnt)
{
    if (XDR_Bool(xdrs, &cp->bSwscaleAfd) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->afd_solution) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->protect_mode_enable) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vpo_io_get_picture_info_rpc(XDR *xdrs, Vpo_io_get_picture_info_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->de_index) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->sw_hw) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->status) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->top_y) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->top_c) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->maf_buffer) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->y_buf_size) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->c_buf_size) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->maf_buf_size) == False)
    {
        return False;
    }

    Int32 i = 0;
    for (i = 0; i < 10; i++)
    {
        if (XDR_Uint32(xdrs, &cp->reserved[i]) == False)
        {
            return False;
        }
    }

    return True;
}


Bool XDR_Vpo_osd_show_time_rpc(XDR *xdrs, Vpo_osd_show_time_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->show_on_off) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->layer_id) == False)
    {
        return False;
    }
    if (XDR_Uchar(xdrs, &cp->reserved0) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->reserved1) == False)
    {
        return False;
    }
    if (XDR_Uint32(xdrs, &cp->time_in_ms) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Vpo_io_cutline_pars_rpc(XDR *xdrs, Vpo_io_cutline_pars_rpc *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->top_bottom) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->cut_line_number) == False)
    {
        return False;
    }

    return True;
}


Bool XDR_Alifbio_3d_pars_rpc(XDR *xdrs, Alifbio_3d_pars_rpc *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->display_3d_enable) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->side_by_side) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->top_and_bottom) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->display_2d_to_3d) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->depth_2d_to_3d) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->mode_2d_to_3d) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->eInputFormat) == False)
    {
        return False;
    }
    if (XDR_Int32(xdrs, &cp->red_blue) == False)
	{
        return False;
	}
 //   if (XDR_Int32(xdrs, &cp->mvc_flag) == False)
	//{
  //      return False;
	//}

    return True;
}

Bool XDR_Alifbio_set_3d_enh_rpc(XDR *xdrs, Alifbio_3d_enhance_rpc *cp, Uint32 cnt)
{
        if (XDR_Int32(xdrs, &cp->enable_3d_enhance) == False)
            return False;

        if (XDR_Int32(xdrs, &cp->set_3d_enhance) == False)
            return False;
            
        if (XDR_Int32(xdrs, &cp->use_enhance_default) == False)
            return False;
            
        if (XDR_Char(xdrs, &cp->eye_protect_shift) == False)
            return False;
            
        if (XDR_Int32(xdrs, &cp->user_true3d_enhance) == False)
            return False;
            
        if (XDR_Char(xdrs, &cp->user_true3d_constant_shift) == False)
            return False;

        if (XDR_Int32(xdrs, &cp->ver_gradient_ratio) == False)
            return False;
            
        if (XDR_Char(xdrs, &cp->hor_gradient_ratio) == False)
            return False;

        if (XDR_Int32(xdrs, &cp->pop_for_reduction_ratio) == False)
            return False;
            
        if (XDR_Char(xdrs, &cp->parallax_sign_bias) == False)
            return False;
            
        return True;
}



Bool XDR_GmaSize(XDR *xdrs, GMA_SIZE *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->w) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->h) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_GmaLayerPars(XDR *xdrs, GMA_LAYER_PARS *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->format) == False)
    {
        return False;
    }

    if (XDR_GmaSize(xdrs, &cp->max_size, sizeof(GMA_SIZE)) == False)
    {
        return False;
    }

    if (XDR_Char(xdrs, &cp->global_alpha_enable) == False)
    {
        return False;
    }

    if (XDR_Char(xdrs, &cp->global_alpha_value) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_GmaEnhPars(XDR *xdrs, GMA_ENH_PARS *cp, Uint32 cnt)
{
    if (XDR_Uchar(xdrs, &cp->enhance_flag) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->enhance_grade) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_GmaDmemPars(XDR *xdrs, GMA_DMEM_PARS *cp, Uint32 cnt)
{
    int i = 0;

    for(i = 0;i < MAX_GMA_NUM;i++)
    {
      if(XDR_Uint32(xdrs, &(cp->dmem_start[i])) == False)
      {
         return False;
      }
    }

    for(i = 0;i < MAX_GMA_NUM;i++)
    {
      if(XDR_Uint32(xdrs, &(cp->dmem_size[i])) == False)
      {
         return False;
      }	
    }

    return True;
}

Bool XDR_GmaScalePars(XDR *xdrs, GMA_SCALE_PARS *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->scale_type) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->h_dst) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->h_src) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->v_dst) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->v_src) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_GmaPalPars(XDR *xdrs, GMA_PAL_PARS *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->type) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->alpha_level) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->pallette_buf) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->color_num) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_GmaRect(XDR *xdrs, GMA_RECT *cp, Uint32 cnt)
{
    if (XDR_Int32(xdrs, &cp->x) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->y) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->w) == False)
    {
        return False;
    }

    if (XDR_Int32(xdrs, &cp->h) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_GmaRegionPars(XDR *xdrs, GMA_REGION_PARS *cp, Uint32 cnt)
{
    if (XDR_GmaRect(xdrs, &cp->rect, sizeof(GMA_RECT)) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->dmem_start) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->dmem_len) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->line_length) == False)
    {
        return False;
    }

    if (XDR_Char(xdrs, &cp->global_alpha_enable) == False)
    {
        return False;
    }

    if (XDR_Char(xdrs, &cp->global_alpha_value) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_De2Hdmi_VidInf(XDR *xdrs, DE2HDMI_VIDINF *cp, Uint32 cnt)
{
    if (XDR_Enum(xdrs, &cp->tv_mode) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->width) == False)
    {
        return False;
    }

    if (XDR_UInt16(xdrs, &cp->height) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->format) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->scan_mode) == False)
    {
        return False;
    }

    if (XDR_Bool(xdrs, &cp->afd_present) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->output_aspect_ratio) == False)
    {
        return False;
    }

    if (XDR_Uchar(xdrs, &cp->active_format_aspect_ratio) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->av_chg_ste) == False)
    {
        return False;
    }

    return True;
}

Bool XDR_Snd2Hdmi_AudInf(XDR *xdrs, SND2HDMI_AUDINF *cp, Uint32 cnt)
{
    int i = 0;

    if (XDR_Uint32(xdrs, &cp->user_def_ch_num) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->pcm_out) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->coding_type) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->max_bit_rate) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->ch_count) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->sample_rate) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->level_shift) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->spdif_edge_clk) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->ch_status) == False)
    {
        return False;
    }

    for (i = 0; i < 8; i++)
    {
        if (XDR_Uchar(xdrs, &cp->ch_position[i]) == False)
        {
            return False;
        }
    }

    if (XDR_Uint32(xdrs, &cp->bclk_lrck) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->word_length) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->i2s_edge_clk) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->i2s_format) == False)
    {
        return False;
    }

    if (XDR_Uint32(xdrs, &cp->lrck_hi_left) == False)
    {
        return False;
    }

    if (XDR_Enum(xdrs, &cp->av_chg_ste) == False)
    {
        return False;
    }

    return True;
}


XdrOpTable gXdrOpTable[] =
{
    {PARAM_VOID, XDR_Void},
    {PARAM_INT32, XDR_Int32},
    {PARAM_UINT32, XDR_Uint32},
    {PARAM_LONG, XDR_Long},
    {PARAM_ULONG, XDR_Ulong},
    {PARAM_INT16, XDR_Int16},
    {PARAM_UINT16, XDR_UInt16},
    {PARAM_BOOL, XDR_Bool},
    {PARAM_ENUM, XDR_Enum},
    {PARAM_ARRAY, XDR_Array},
    {PARAM_BYTES, XDR_Bytes},
    {PARAM_OPAQUE, XDR_Opaque},
    {PARAM_STRING, XDR_String},
    {PARAM_UNION, XDR_Union},
    {PARAM_CHAR, XDR_Char},
    {PARAM_UCHAR, XDR_Uchar},
    {PARAM_VECTOR, XDR_Vector},
    {PARAM_FLOAT, XDR_Float},
    {PARAM_DOUBLE, XDR_Double},
    {PARAM_REFERENCE, XDR_Reference},
    {PARAM_POINTER, XDR_Pointer},
    {PARAM_WRAPSTRING, XDR_Wrapstring},
    {PARAM_STRARRAY, XDR_Strarray},
    {PARAM_RPCDBGINFO, XDR_Rpcdbginfo},
    /*User specific data type proc table: START*/
    {PARAM_TESTSTRUCT, XDR_TestStruct}, /*Sample for new struct*/

    /******** Audio playback example ***********/
    {PARAM_Deca_feature_config_rpc, XDR_Deca_feature_config_rpc},
    {PARAM_Snd_output_cfg_rpc, XDR_Snd_output_cfg_rpc},
    {PARAM_Snd_feature_config_rpc, XDR_Snd_feature_config_rpc},
    {PARAM_Hld_device_rpc, XDR_Hld_device_rpc},
    {PARAM_Pcm_output_rpc, XDR_Pcm_output_rpc},
    {PARAM_Control_block_rpc, XDR_Control_block_rpc},
    {PARAM_Pe_music_cfg_rpc, XDR_Pe_music_cfg_rpc},
    {PARAM_MusicInfo_rpc, XDR_MusicInfo_rpc},
    {PARAM_DecoderInfo_rpc, XDR_DecoderInfo_rpc},
    {PARAM_Image_info_rpc, XDR_Image_info_rpc},
    {PARAM_Image_init_config_rpc, XDR_Image_init_config_rpc},
    {PARAM_Image_hw_info_rpc, XDR_Image_hw_info_rpc},
    {PARAM_Pe_image_cfg_rpc, XDR_Pe_image_cfg_rpc},
    {PARAM_Image_info_pe_rpc, XDR_Image_info_pe_rpc},
    {PARAM_Imagedec_frm_rpc, XDR_Imagedec_frm_rpc},
    {PARAM_Pe_video_cfg_rpc, XDR_Pe_video_cfg_rpc},
    {PARAM_DEC_CHAPTER_INFO_RPC, XDR_DEC_CHAPTER_INFO_RPC},
    {PARAM_Fileinfo_video_rpc, XDR_Fileinfo_video_rpc},
    {PARAM_Pe_cache_ex_rpc, XDR_Pe_cache_ex_rpc},
    {PARAM_Pe_cache_cmd_rpc, XDR_Pe_cache_cmd_rpc},
    {PARAM_Audio_config_rpc, XDR_Audio_config_rpc},
    {PARAM_Snd_dev_status_rpc, XDR_Snd_dev_status_rpc},
    {PARAM_Spec_param_rpc, XDR_Spec_param_rpc},
    {PARAM_Spec_step_table_rpc, XDR_Spec_step_table_rpc},
    {PARAM_Snd_spdif_scms_rpc, XDR_Snd_spdif_scms_rpc},
    {PARAM_Snd_sync_param_rpc, XDR_Snd_sync_param_rpc},
    {PARAM_Isdbtcc_config_par_rpc, XDR_Isdbtcc_config_par_rpc},
    {PARAM_Atsc_subt_config_par_rpc, XDR_Atsc_subt_config_par_rpc},
    {PARAM_Sdec_feature_config_rpc, XDR_Sdec_feature_config_rpc},
    {PARAM_Subt_config_par_rpc, XDR_Subt_config_par_rpc},
    {PARAM_AUDIO_INFO_rpc, XDR_AUDIO_INFO_rpc},
    {PARAM_Reverb_param_rpc, XDR_Reverb_param_rpc},
    {PARAM_Pl_ii_param_rpc, XDR_Pl_ii_param_rpc},
    {PARAM_Io_param_rpc, XDR_Io_param_rpc},
    {PARAM_Cur_stream_info_rpc, XDR_Cur_stream_info_rpc},
    {PARAM_Deca_buf_info_rpc, XDR_Deca_buf_info_rpc},
    {PARAM_Ase_str_play_param_rpc, XDR_Ase_str_play_param_rpc},
    {PARAM_RectSize_rpc, XDR_RectSize_rpc},
    {PARAM_AdvSetting_rpc, XDR_AdvSetting_rpc},
    {PARAM_VDecPIPInfo_rpc, XDR_VDecPIPInfo_rpc},
    {PARAM_VDec_StatusInfo_rpc, XDR_VDec_StatusInfo_rpc},
    {PARAM_Audio_decore_status_rpc, XDR_Audio_decore_status_rpc},

    {PARAM_Imagedec_show_brush_rpc, XDR_Imagedec_show_brush_rpc},
    {PARAM_Imagedec_show_brush_rpc, XDR_Imagedec_show_brush_rpc},
    {PARAM_Imagedec_show_slide_rpc, XDR_Imagedec_show_slide_rpc},
    {PARAM_Imagedec_show_show_random_rpc, XDR_Imagedec_show_show_random_rpc},
    {PARAM_Imagedec_show_fade_rpc, XDR_Imagedec_show_fade_rpc},
    {PARAM_Image_slideshow_effect_rpc, XDR_Image_slideshow_effect_rpc},
    {PARAM_Image_config_rpc, XDR_Image_config_rpc},
    {PARAM_Image_engine_config_rpc, XDR_Image_engine_config_rpc},
    {PARAM_Image_display_rpc, XDR_Image_display_rpc},

    /* Video Encoder added by Vic_Zhang_on_20131005. */
    {PARAM_VidencSeeConfig, XDR_VidencSeeConfig_t},
    {PARAM_VidencTriggerPara, XDR_VidencTriggerPara_t},

    /******** decv *****************************/
    {PARAM_Vdec_adpcm_rpc, XDR_Vdec_adpcm_rpc},
    {PARAM_Vdec_sml_frm_rpc, XDR_Vdec_sml_frm_rpc},
    {PARAM_Vdec_mem_map_rpc, XDR_Vdec_mem_map_rpc},
    {PARAM_Vdec_config_par_rpc, XDR_Vdec_config_par_rpc},
    {PARAM_Vdec_avs_memmap_rpc, XDR_Vdec_avs_memmap_rpc},
    {PARAM_Vdec_avc_memmap_rpc, XDR_Vdec_avc_memmap_rpc},
    {PARAM_Vdec_avc_config_par_rpc, XDR_Vdec_avc_config_par_rpc},
    {PARAM_Position_rpc, XDR_Position_rpc},
    {PARAM_Rect_rpc, XDR_Rect_rpc},
    {PARAM_Vdec_dvrconfigparam_rpc, XDR_Vdec_dvrconfigparam_rpc},
    {PARAM_Vdecpipinfo_rpc, XDR_Vdecpipinfo_rpc},
    {PARAM_Mpsource_callback_rpc, XDR_Mpsource_callback_rpc},
    {PARAM_Pipsource_callback_rpc, XDR_Pipsource_callback_rpc},
    {PARAM_Vdec_statusinfo_rpc, XDR_Vdec_statusinfo_rpc},
    {PARAM_Vdec_decore_status_rpc, XDR_Vdec_decore_status_rpc},
    {PARAM_Vdec_io_get_frm_para_rpc, XDR_Vdec_io_get_frm_para_rpc},
    {PARAM_Ycbcrcolor_rpc, XDR_Ycbcrcolor_rpc},
    {PARAM_Vdec_io_reg_callback_para_rpc, XDR_Vdec_io_reg_callback_para_rpc},
    {PARAM_Vdec_picture_rpc, XDR_Vdec_picture_rpc},
    {PARAM_Vdec_io_dbg_flag_info_rpc, XDR_Vdec_io_dbg_flag_info_rpc},
    {PARAM_Video_rect_rpc, XDR_Video_rect_rpc},
    {PARAM_OutputFrmManager_rpc, XDR_OutputFrmManager_rpc},
    {PARAM_Vdecinit_rpc, XDR_Vdecinit_rpc},
    {PARAM_Vdec_playback_param_rpc, XDR_Vdec_playback_param_rpc},
    {PARAM_Vdec_capture_frm_info_rpc, XDR_Vdec_capture_frm_info_rpc},

    /******** avs *****************************/
    {PARAM_Avsync_cfg_param_t_rpc, XDR_Avsync_cfg_param_t_rpc},
    {PARAM_Avsync_adv_param_t_rpc, XDR_Avsync_adv_param_t_rpc},
    {PARAM_Avsync_status_t_rpc, XDR_Avsync_status_t_rpc},
    {PARAM_Avsync_statistics_t_rpc, XDR_Avsync_statistics_t_rpc},
    {PARAM_Avsync_smoothly_play_cfg_t_rpc, XDR_Avsync_smoothly_play_cfg_t_rpc},
    {PARAM_Avsync_get_stc_en_t_rpc, XDR_Avsync_get_stc_en_t_rpc},

    /******** DMX *****************************/
    {PARAM_Io_param_ex_rpc, XDR_Io_param_ex_rpc},
    {PARAM_Pes_pkt_param_rpc, XDR_Pes_pkt_param_rpc},

    /******** VBI *****************************/
    {PARAM_Vbi_config_par_rpc, XDR_Vbi_config_par_rpc},
    {PARAM_Ttx_config_par_rpc, XDR_Ttx_config_par_rpc},
    {PARAM_Ttx_page_info_rpc, XDR_Ttx_page_info_rpc},

    /********* CE *****************************/
    {PARAM_Ce_data_info_rpc, XDR_Ce_data_info_rpc},
    {PARAM_Otp_param_rpc, XDR_Otp_param_rpc},
    {PARAM_Data_param_rpc, XDR_Data_param_rpc},
    {PARAM_Des_param_rpc, XDR_Des_param_rpc},
    {PARAM_Ce_key_param_rpc, XDR_Ce_key_param_rpc},
    {PARAM_Ce_debug_key_info_rpc, XDR_Ce_debug_key_info_rpc},
    {PARAM_Ce_pos_status_param_rpc, XDR_Ce_pos_status_param_rpc},
    {PARAM_Ce_found_free_pos_param_rpc, XDR_Ce_found_free_pos_param_rpc},
    {PARAM_Ce_pvr_key_param_rpc, XDR_Ce_pvr_key_param_rpc},

    /********* dsc ****************************/
    {PARAM_DeEncrypt_config_rpc, XDR_DeEncrypt_config_rpc},
    {PARAM_Sha_init_param_rpc, XDR_Sha_init_param_rpc},
    {PARAM_Aes_init_param_rpc, XDR_Aes_init_param_rpc},
    {PARAM_Des_init_param_rpc, XDR_Des_init_param_rpc},
    {PARAM_Pid_param_rpc, XDR_Pid_param_rpc},
    {PARAM_Csa_init_param_rpc, XDR_Csa_init_param_rpc},
    {PARAM_Dsc_pvr_key_param_rpc, XDR_Dsc_pvr_key_param_rpc},
    {PARAM_Key_param_rpc, XDR_Key_param_rpc},
    {PARAM_Sha_hash_rpc, XDR_Sha_hash_rpc},    
    {PARAM_Trng_data_rpc, XDR_Trng_data_rpc},
    {PARAM_Dsc_deen_parity_rpc, XDR_Dsc_deen_parity_rpc},
    {PARAM_Dsc_drv_ver_rpc, XDR_Dsc_drv_ver_rpc},
    {PARAM_Dsc_ver_chk_param_rpc, XDR_Dsc_ver_chk_param_rpc},

    /********* dis ****************************/
    {PARAM_Vp_feature_config_rpc, XDR_Vp_feature_config_rpc},
    {PARAM_Tve_feature_config_rpc, XDR_Tve_feature_config_rpc},
    {PARAM_Osd_driver_config_rpc, XDR_Osd_driver_config_rpc},
    {PARAM_Vp_win_config_para_rpc, XDR_Vp_win_config_para_rpc},
    {PARAM_Vpo_io_ttx_rpc, XDR_Vpo_io_ttx_rpc},
    {PARAM_Vpo_io_cc_rpc, XDR_Vpo_io_cc_rpc},
    {PARAM_Dacindex_rpc, XDR_Dacindex_rpc},
    {PARAM_Vp_dacinfo_rpc, XDR_Vp_dacinfo_rpc},
    {PARAM_Vp_io_reg_dac_para_rpc, XDR_Vp_io_reg_dac_para_rpc},
    {PARAM_Vp_initinfo_rpc, XDR_Vp_initinfo_rpc},
    {PARAM_Vpo_io_set_parameter_rpc, XDR_Vpo_io_set_parameter_rpc},
    {PARAM_Vpo_io_video_enhance_rpc, XDR_Vpo_io_video_enhance_rpc},
    {PARAM_Vpo_io_get_info_rpc, XDR_Vpo_io_get_info_rpc},
    {PARAM_De2Hdmi_video_infor_rpc, XDR_De2Hdmi_video_infor_rpc},
    {PARAM_Vpo_io_cgms_info_rpc, XDR_Vpo_io_cgms_info_rpc},
    {PARAM_Vp_io_afd_para_rpc, XDR_Vp_io_afd_para_rpc},
    {PARAM_Vpo_io_get_picture_info_rpc, XDR_Vpo_io_get_picture_info_rpc},
    {PARAM_Vpo_osd_show_time_rpc, XDR_Vpo_osd_show_time_rpc},
    {PARAM_Vpo_io_cutline_pars_rpc, XDR_Vpo_io_cutline_pars_rpc},
    {PARAM_Alifbio_3d_pars_rpc, XDR_Alifbio_3d_pars_rpc},
	{PARAM_Alifbio_set_3d_enh_rpc, XDR_Alifbio_set_3d_enh_rpc},


    {PARAM_GMA_SIZE, XDR_GmaSize},
    {PARAM_GMA_LAYER_PARS, XDR_GmaLayerPars},
    {PARAM_GMA_ENH_PARS, XDR_GmaEnhPars},
    {PARAM_GMA_DMEM_PARS, XDR_GmaDmemPars},
    {PARAM_GMA_SCALE_PARS, XDR_GmaScalePars},
    {PARAM_GMA_PAL_PARS, XDR_GmaPalPars},
    {PARAM_GMA_RECT, XDR_GmaRect},
    {PARAM_GMA_REGION_PARS, XDR_GmaRegionPars},
    {PARAM_DE2HDMI_VIDINF, XDR_De2Hdmi_VidInf},
    {PARAM_SND2HDMI_AUDINF, XDR_Snd2Hdmi_AudInf},


    /*User specific data type proc table: END*/
    {PARAM_ID_MAX, NULL}
};
