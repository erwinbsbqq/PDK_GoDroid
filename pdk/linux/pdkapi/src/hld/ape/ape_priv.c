#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <sys/time.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <dvb_audio.h>

#include <linux/ali_stc.h>

#include <hld_cfg.h>
#include <hld/decv/adr_decv.h>
#include <hld/deca/adr_deca_dev.h>
#include <hld/snd/adr_snd_dev.h>
#include <hld/ape/adr_ape.h>

#include <ali_video_common.h>
#include <ali_sbm_common.h>
#include <ali_pe_common.h>

#include "ape_priv.h"

static int g_ape_video_frame_rateX10 = 0;
static int g_ape_audio_byte_rateX10 = 0;

static int g_ape_video_timescale_num = 1000;
static int g_ape_video_timescale_den = 1;

static ape_priv_t* g_ape_priv = NULL;
ape_priv_t* ape_get_priv()
{
	return g_ape_priv;
}

ape_priv_t* ape_malloc_priv()
{
	ape_priv_t* ape_priv = NULL;


	g_ape_priv = ape_priv = (ape_priv_t*)malloc(sizeof(ape_priv_t));
	if (NULL == ape_priv)
	{
		APE_DEBUG( "malloc ape priv mem error!!\n");
	}

	return ape_priv;
}

void ape_free_priv()
{
	free(g_ape_priv);
	g_ape_priv = NULL;
}

int ape_check_fd(int fd)
{
	ape_priv_t* ape_priv = ape_get_priv();
	return (fd == ape_priv->apkt_data_fd || fd == ape_priv->apkt_hdr_fd \
			|| fd == ape_priv->vpkt_data_fd || fd == ape_priv->vpkt_hdr_fd \
			|| fd == ape_priv->decv_out_fd || fd == ape_priv->deca_out_fd \
			|| fd == ape_priv->disp_in_fd || fd == ape_priv->snd_in_fd);
}

void ape_store_pts(int fd, unsigned int pts)
{
	ape_priv_t* ape_priv = ape_get_priv();
	if (fd == ape_priv->apkt_hdr_fd)
	{
		ape_priv->audio_pts[ape_priv->audio_hdr_num++] = pts;
		ape_priv->audio_hdr_num %= 10;
	}
	else
	{
		ape_priv->video_pts[ape_priv->video_hdr_num++] = pts;
		ape_priv->video_hdr_num %= 10;
	}
}

unsigned int ape_get_pts(int fd)
{
	ape_priv_t* ape_priv = ape_get_priv();
	unsigned int pts = 0;
	if (fd == ape_priv->apkt_data_fd)
	{
		pts = ape_priv->audio_pts[ape_priv->audio_pkt_num++];
		ape_priv->audio_pkt_num %= 10;
	}
	else
	{
		pts = ape_priv->video_pts[ape_priv->video_pkt_num++];
		ape_priv->video_pkt_num %= 10;
	}
	return pts;
}

int ape_adec_sbm_reset()
{
	int ret = 0;
	ape_priv_t* ape_priv = ape_get_priv();
	ret = ioctl(ape_priv->apkt_hdr_fd, SBMIO_RESET_SBM, 0);
	if(ret < 0)
	{
        return -1;
	}

    ret = ioctl(ape_priv->apkt_data_fd, SBMIO_RESET_SBM, 0);
	if(ret < 0)
	{
        return -1;
	}
    ret = ioctl(ape_priv->deca_out_fd, SBMIO_RESET_SBM, 0);
	if(ret < 0)
	{
        return -1;
	}
    ret = ioctl(ape_priv->snd_in_fd, SBMIO_RESET_SBM, 0);
	if(ret < 0)
	{
        return -1;
	}
	return 0;
}

int ape_vdec_sbm_reset()
{
	int ret = 0;
	ape_priv_t* ape_priv = ape_get_priv();
	ret = ioctl(ape_priv->vpkt_hdr_fd, SBMIO_RESET_SBM, 0);
	if(ret < 0)
	{
        return -1;
	}

    ret = ioctl(ape_priv->vpkt_data_fd, SBMIO_RESET_SBM, 0);
	if(ret < 0)
	{
        return -1;
	}
    ret = ioctl(ape_priv->decv_out_fd, SBMIO_RESET_SBM, 0);
	if(ret < 0)
	{
        return -1;
	}
    ret = ioctl(ape_priv->disp_in_fd, SBMIO_RESET_SBM, 0);
	if(ret < 0)
	{
        return -1;
	}
	return 0;
}

static int ape_video_fd_open()
{
	ape_priv_t* ape_priv = ape_get_priv();
	
	ape_priv->video_fd = open("/dev/ali_video0", O_RDWR | O_CLOEXEC);
    if(ape_priv->video_fd <= 0) 
	{
		APE_DEBUG( "open video fail\n");
	    return -1;
    }	
	
	ape_priv->vpkt_data_fd = open(SBMDEV_VPKT_DATA, O_WRONLY | O_CLOEXEC);
	if (ape_priv->vpkt_data_fd < 0) 
	{
	   APE_DEBUG( "Unable to open share memory buffer %s!\n", SBMDEV_VPKT_DATA);
	   return -1;
	} 	
	
	ape_priv->vpkt_hdr_fd = open(SBMDEV_VPKT_HDR, O_WRONLY | O_CLOEXEC);
	if (ape_priv->vpkt_hdr_fd < 0) 
	{
	    APE_DEBUG( "Unable to open share memory buffer %s!\n", SBMDEV_VPKT_HDR);
	    return -1;
	} 	
	
	ape_priv->decv_out_fd = open(SBMDEV_DECV_OUT, O_RDONLY | O_CLOEXEC);  
	if (ape_priv->decv_out_fd < 0) 
	{
	    APE_DEBUG( "Unable to open share memory buffer %s!\n", SBMDEV_DECV_OUT);
	    return -1;
	} 
	
	ape_priv->disp_in_fd = open(SBMDEV_DISP_IN, O_WRONLY | O_CLOEXEC); 
	if (ape_priv->disp_in_fd < 0) 
	{
	    APE_DEBUG( "Unable to open share memory buffer %s!\n", SBMDEV_DISP_IN);
	    return -1;
	}	
	
	return 0;
}

static int ape_audio_fd_open()
{
	ape_priv_t* ape_priv = ape_get_priv();
	
	ape_priv->apkt_data_fd = open(SBMDEV_APKT_DATA, O_WRONLY | O_CLOEXEC);
	if (ape_priv->apkt_data_fd < 0) 
	{
	    APE_DEBUG( "Unable to open share memory buffer %s!\n", SBMDEV_APKT_DATA);
	    return -1;
	}	
	
	ape_priv->apkt_hdr_fd = open(SBMDEV_APKT_HDR, O_WRONLY | O_CLOEXEC);
	if (ape_priv->apkt_hdr_fd < 0) 
	{
	    APE_DEBUG( "Unable to open share memory buffer %s!\n", SBMDEV_APKT_HDR);
	    return -1;
	} 	
	
	ape_priv->deca_out_fd = open(SBMDEV_DECA_OUT, O_RDONLY | O_CLOEXEC);
	if (ape_priv->deca_out_fd < 0) 
	{
	    APE_DEBUG( "Unable to open share memory buffer %s!\n", SBMDEV_DECA_OUT);
	    return -1;
	}	
	
	ape_priv->snd_in_fd = open(SBMDEV_SND_IN, O_WRONLY | O_CLOEXEC);
	if (ape_priv->snd_in_fd < 0) 
	{
	    APE_DEBUG( "Unable to open share memory buffer %s\n", SBMDEV_SND_IN);
	    return -1;
	}	
	
	return 0;
}

static int ape_clock_fd_open()
{
	ape_priv_t* ape_priv = ape_get_priv();
	
	ape_priv->clock0_fd = open(STCDEV0_FILENAME, O_RDWR | O_CLOEXEC);
	if (ape_priv->clock0_fd < 0) 
	{
	    APE_DEBUG( "Unable to open clock fd %s!\n", STCDEV0_FILENAME);
	    return -1;
	}
	
	ape_priv->clock1_fd = open(STCDEV1_FILENAME, O_RDWR | O_CLOEXEC);
	if (ape_priv->clock1_fd < 0) 
	{
	    APE_DEBUG( "Unable to open clock fd %s!\n", STCDEV1_FILENAME);
	    return -1;
	}	
	
	return 0;
}

static int ape_clock_fd_close()
{
	ape_priv_t* ape_priv = ape_get_priv();
	if (NULL == ape_priv)
	{
		APE_DEBUG( "the buf have already freed!!\n");
		return -1;
	}
	if (ape_priv->clock0_fd > 0)
	{
		close(ape_priv->clock0_fd);
	    ape_priv->clock0_fd = -1;
	}
	if (ape_priv->clock1_fd > 0)
	{
		close(ape_priv->clock1_fd);
	    ape_priv->clock1_fd = -1;
	}
	return 0;
}

static int ape_video_fd_close()
{
	ape_priv_t* ape_priv = ape_get_priv();
	if (NULL == ape_priv)
	{
		APE_DEBUG( "the buf have already freed!!\n");
		return 0;
	}
	
	if (ape_priv->video_fd > 0)
	{
		close(ape_priv->video_fd);
	    ape_priv->video_fd = -1;
	}
	
	if(ape_priv->vpkt_data_fd > 0) 
	{
	    close(ape_priv->vpkt_data_fd);
	    ape_priv->vpkt_data_fd = -1;
  	}

  	if(ape_priv->vpkt_hdr_fd > 0) 
	{
	    close(ape_priv->vpkt_hdr_fd);
	    ape_priv->vpkt_hdr_fd = -1;
  	}

  	if(ape_priv->decv_out_fd > 0) 
	{
	    close(ape_priv->decv_out_fd);
	    ape_priv->decv_out_fd = -1;
	}

  	if(ape_priv->disp_in_fd > 0) 
  	{
	    close(ape_priv->disp_in_fd);
	    ape_priv->disp_in_fd = -1;
  	}
	return 0;
}

static int ape_audio_fd_close()
{
	ape_priv_t* ape_priv = ape_get_priv();
	if (NULL == ape_priv)
	{
		APE_DEBUG( "the buf have already freed!!\n");
		return -1;
	}
	if(ape_priv->apkt_data_fd > 0) 
  	{
	    close(ape_priv->apkt_data_fd);
	    ape_priv->apkt_data_fd = -1;
  	}	

  	if(ape_priv->apkt_hdr_fd > 0) 
	 {
	    close(ape_priv->apkt_hdr_fd);
	    ape_priv->apkt_hdr_fd = -1;
	 }

  	if(ape_priv->deca_out_fd > 0) 
	{
	    close(ape_priv->deca_out_fd);
	    ape_priv->deca_out_fd = -1;
	}

  	if(ape_priv->snd_in_fd > 0) 
  	{
	    close(ape_priv->snd_in_fd);
	    ape_priv->snd_in_fd = -1;
  	}
	return 0;
}

int ape_fd_open()
{
	ape_video_fd_open();
	ape_audio_fd_open();
	ape_clock_fd_open();

	return 0;
}

void ape_fd_close()
{
	ape_video_fd_close();
	ape_audio_fd_close();
	ape_clock_fd_close();
}

int ape_check_codec_support(unsigned int codec_tag)
{
	return (codec_tag != rmvb && codec_tag != xvid && codec_tag != vp8 && codec_tag != h264 \
		&& codec_tag != vc1 && codec_tag != mpg2);

}

int ape_video_select_decoder(ape_priv_t *ape_priv, int preview)
{
	int decoder;

	if(ape_priv->codec_tag == h264) 
		decoder = 1;
	else 
		decoder = 0;
	
	video_decoder_select(decoder, preview);

	return 0;
}

int ape_video_start(ape_priv_t *ape_priv)
{
	struct ali_video_rpc_pars rpc_pars;	
	int ret = 1;

	if(ape_priv->codec_tag != h264 && ape_priv->codec_tag != mpg2)  
		return 0;

	vdec_io_control(get_selected_decoder(), VDEC_IO_CONTINUE_ON_ERROR, TRUE);
	
	ret = vdec_start(get_selected_decoder());

	return ret;
}

int ape_video_enable_rgb_output(void)
{
	int ret = 1;

	ret = vdec_io_control(get_selected_decoder(), VDEC_IO_SET_RGB_OUTPUT_FLAG, 1);

	return ret;
}

int ape_video_stop(ape_priv_t *ape_priv, int bclosevp, int bfillblack)
{
	struct ali_video_rpc_pars rpc_pars;	
	int ret = 1;

	if(ape_priv->codec_tag != h264 && ape_priv->codec_tag != mpg2)  
		return 0;

	ret = vdec_stop(get_selected_decoder(), bclosevp, bfillblack);

	//vdec_io_control(get_selected_decoder(), VDEC_IO_CONTINUE_ON_ERROR, FALSE);

	return ret;
}

int ape_video_decore_ioctl(ape_priv_t* ape_priv, int cmd, void *param1, void *param2)
{
	struct ali_video_rpc_pars rpc_pars;	
	int video_fd, ret;
	int io_code;

	if(ape_priv->codec_tag != h264 && ape_priv->codec_tag != mpg2 \
		&& ape_priv->codec_tag != xvid && ape_priv->codec_tag != vp8 \
		&& ape_priv->codec_tag != rmvb && ape_priv->codec_tag != vc1) 
	{
		APE_DEBUG( "video decore ioctl not support %x\n",ape_priv->codec_tag);
		return -1;
	}

	if(ape_priv->video_fd <= 0) {
		APE_DEBUG( "open video fail\n");
		return -1;
	}

	memset((void *)&rpc_pars, 0, sizeof(struct ali_video_rpc_pars));

	rpc_pars.API_ID = RPC_VIDEO_DECORE_IOCTL;
	rpc_pars.arg_num = 4;

	switch(cmd) {
		case VDEC_CMD_INIT:
			rpc_pars.arg[0].arg = (void *)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void *)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);			
			rpc_pars.arg[2].arg = (void *)param1;
			rpc_pars.arg[2].arg_size = sizeof(param1);	
			rpc_pars.arg[3].arg = (void *)param2;
			rpc_pars.arg[3].arg_size = sizeof(VdecInit);
			break;
		case VDEC_CMD_EXTRA_DADA:
			rpc_pars.arg[0].arg = (void *)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void *)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);			
			rpc_pars.arg[2].arg = (void *)param1;
			rpc_pars.arg[2].arg_size = *(unsigned int *)param2;	
			rpc_pars.arg[3].arg = (void *)param2;
			rpc_pars.arg[3].arg_size = sizeof(param2);
			break;
		case VDEC_CMD_RELEASE:
		case VDEC_CMD_SW_RESET:
		case VDEC_CMD_HW_RESET:
		case VDEC_CMD_PAUSE_DECODE:    
		case VDEC_CFG_VIDEO_SBM_BUF:
		case VDEC_CFG_DISPLAY_SBM_BUF:
		case VDEC_CFG_SYNC_MODE:
		case VDEC_CFG_SYNC_THRESHOLD:
			rpc_pars.arg[0].arg = (void *)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void *)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);			
			rpc_pars.arg[2].arg = (void *)param1;
			rpc_pars.arg[2].arg_size = sizeof(param1);	
			rpc_pars.arg[3].arg = (void *)param2;
			rpc_pars.arg[3].arg_size = sizeof(param2);
			break;
        case VDEC_ROTATE_FRAME:
		case VDEC_CFG_DECODE_MODE:
			rpc_pars.arg[0].arg = (void *)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void *)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);			
			rpc_pars.arg[2].arg = (void *)param1;
			rpc_pars.arg[2].arg_size = sizeof(param1);	
			rpc_pars.arg[3].arg = (void *)param2;
			rpc_pars.arg[3].arg_size = sizeof(param2);
			break;
		case VDEC_CFG_DISPLAY_RECT:
			rpc_pars.arg[0].arg = (void *)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void *)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);			
			rpc_pars.arg[2].arg = (void *)param1;
			rpc_pars.arg[2].arg_size = sizeof(struct Video_Rect);	
			rpc_pars.arg[3].arg = (void *)param2;
			rpc_pars.arg[3].arg_size = sizeof(struct Video_Rect);
			break;
		case VDEC_CMD_GET_STATUS:
			rpc_pars.arg[0].arg = (void *)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void *)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);			
			rpc_pars.arg[2].arg = (void *)param1;
			rpc_pars.arg[2].arg_size = sizeof(struct vdec_decore_status);	
			rpc_pars.arg[2].out = 1;
			rpc_pars.arg[3].arg = (void *)param2;
			rpc_pars.arg[3].arg_size = sizeof(param2);
			break;
		case VDEC_CMD_FF_FB:
		case VDEC_CMD_NORMAL_PLAY:
			rpc_pars.arg[0].arg = (void*)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void*)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);
			rpc_pars.arg[2].arg = (void *)param1;
			rpc_pars.arg[2].arg_size = sizeof(param1);
			rpc_pars.arg[3].arg = (void*)param2;
			rpc_pars.arg[3].arg_size = sizeof(param2);
			break;
		case VDEC_CMD_REG_CB:
			rpc_pars.arg[0].arg = (void*)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void*)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);
			rpc_pars.arg[2].arg = (void*)param1;
			rpc_pars.arg[2].arg_size = sizeof(param1);
			rpc_pars.arg[3].arg = (void*)param2;
			rpc_pars.arg[3].arg_size = sizeof(param2);
			break;
		case VDEC_CMD_GET_TRICK_FRAME:
			rpc_pars.arg[0].arg = (void*)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void*)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);
			rpc_pars.arg[2].arg = (void*)param1;
			rpc_pars.arg[2].arg_size = sizeof(param1);
			rpc_pars.arg[3].arg = (void*)param2;
			rpc_pars.arg[3].arg_size = sizeof(param2);	
			rpc_pars.arg[3].out = 1;
			break;
		case VDEC_CMD_ROTATION:
			ret = ioctl(ape_priv->video_fd, ALIVIDEO_ROTATION, (struct rotation_rect*)param1);
			return ret; 
		case VDEC_CMD_ROTATION_UPDATA_FRMBUF:
			rpc_pars.arg[0].arg = (void*)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void*)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);
			rpc_pars.arg[2].arg = (void*)param1;
			rpc_pars.arg[2].arg_size = sizeof(struct rotation_rect);
			rpc_pars.arg[2].out = 1;
			rpc_pars.arg[3].arg = (void*)param2;
			rpc_pars.arg[3].arg_size = sizeof(param2);	
			break;
		case VDEC_CMD_ROTATION_SET_FRMBUF:
			rpc_pars.arg[0].arg = (void*)&ape_priv->codec_tag;
			rpc_pars.arg[0].arg_size = sizeof(int);
			rpc_pars.arg[1].arg = (void*)&cmd;
			rpc_pars.arg[1].arg_size = sizeof(cmd);
			rpc_pars.arg[2].arg = (void*)param1;
			rpc_pars.arg[2].arg_size = sizeof(struct rotation_rect);
			rpc_pars.arg[3].arg = (void*)param2;
			rpc_pars.arg[3].arg_size = sizeof(param2);	
			break;	
		default:
			ret = -1;
			break;
	}

	ret = ioctl(ape_priv->video_fd, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
	if(ret < 0) {
		APE_DEBUG( "video decore ioctl fail,video_fd = %d\n", ape_priv->video_fd);
		ret = -1;
	}

	return ret;
}

int ape_config_video_init_par(video_config_info_t* info)
{
	int video_fd;
	int ret;
	VdecInit vdec_init;
	struct ali_video_mem_info decoder_info;
	ape_priv_t* ape_priv = ape_get_priv();
	
#if 0	
	// for still frame solution is not ready, close vpo at 2013-02-07 by Sam
	vpo_win_onoff((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), 0);
#endif

	memset((void *)&vdec_init, 0, sizeof(vdec_init));
	
	if (ape_priv->codec_tag != h264 && ape_priv->codec_tag != mpg2)
	{
		if(ape_priv->video_fd <= 0){
			APE_DEBUG( "open ali_video0 fail\n");
			return -1;
		}
		ioctl(ape_priv->video_fd, ALIVIDEOIO_GET_MEM_INFO, (int)&decoder_info);
		vdec_init.pfrm_buf = (VdecFBConfig *) decoder_info.mem_start;
		vdec_init.phw_mem_cfg = (VdecHWMemConfig *)decoder_info.mem_size;
		vdec_init.preview = info->preview;
	}
	
	vdec_init.decode_mode = (info->decoder_flag & (1<<31)) ? VDEC_MODE_SBM_STREAM : VDEC_MODE_SBM;
	vdec_init.fmt_in.fourcc = info->fourcc;
	vdec_init.fmt_in.pic_width = info->width;
	vdec_init.fmt_in.pic_height = info->height;
	vdec_init.fmt_in.pixel_aspect_x = info->sample_aspect_ratio_num;
	vdec_init.fmt_in.pixel_aspect_y = info->sample_aspect_ratio_den;
	if(info->time_base_den == -1)
	{
		APE_DEBUG("can't get framerate from user,set 25000 ad default!\n");
		vdec_init.fmt_in.frame_rate = 25000;
		vdec_init.fmt_out.frame_rate = 25000;
	}
	else
	{
		vdec_init.fmt_in.frame_rate = info->time_base_den * 10;
		vdec_init.fmt_out.frame_rate = info->time_base_den * 10;
	}
	vdec_init.fmt_out.pic_width = info->width;
	vdec_init.fmt_out.pic_height = info->height;
	vdec_init.fmt_out.pixel_aspect_x = info->sample_aspect_ratio_num;
	vdec_init.fmt_out.pixel_aspect_y = info->sample_aspect_ratio_den;
	vdec_init.decoder_flag = info->decoder_flag;

	if(ape_get_dbg_on())
	{
		if(info->time_base_den == -1)
			g_ape_video_frame_rateX10 = 250;
		else
			g_ape_video_frame_rateX10 = info->time_base_den / 100;
		
		APE_DEBUG("dec info as below:");
		APE_DEBUG("type %s\n", ape_priv->codec_tag == h264 ? "h.264" : 
			(ape_priv->codec_tag == mpg2 ? "mpeg" : 
			(ape_priv->codec_tag == rmvb ? "rmvb" : 
			(ape_priv->codec_tag == vp8 ? "vp8" : 
			(ape_priv->codec_tag == vc1 ? "vc1" : 
			(ape_priv->codec_tag == xvid ? "xvid" : "unknow"))))));
		APE_DEBUG("fourcc %x\n", info->fourcc);
		APE_DEBUG("width %d\n", info->width);
		APE_DEBUG("height %d\n", info->height);
		APE_DEBUG("ratio x %d\n", info->sample_aspect_ratio_num);
		APE_DEBUG("ratio y %d\n", info->sample_aspect_ratio_den);
		APE_DEBUG("dec flag %x\n", info->decoder_flag);
		APE_DEBUG("time base num %d\n", info->time_base_num);
		APE_DEBUG("time base den %d\n", info->time_base_den);		
		APE_DEBUG("time scale num %d\n", info->timescale_num);
		APE_DEBUG("time scale den %d\n", info->timescale_den);			
		APE_DEBUG("video frame rate X 10 = %d\n", g_ape_video_frame_rateX10);		
	}		
	
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_INIT, (void*)&(ape_priv->codec_tag), &vdec_init);
	if(ret != 0){
		APE_DEBUG( "decore init fail\n");
		return -1;
	}

	if((info->timescale_num != 0) && (info->timescale_den != 0))
	{
		g_ape_video_timescale_num = info->timescale_num;
		g_ape_video_timescale_den = info->timescale_den;
	}

	memcpy((void *)&ape_priv->video_info, (void *)info, sizeof(*info));
	
	return 0;
}

void ape_video_read_avframe(struct av_frame *decv_frm_info)
{
	ape_priv_t* ape_priv = ape_get_priv();

	read(ape_priv->decv_out_fd, decv_frm_info, sizeof(struct av_frame));
}

void ape_video_write_avframe(struct av_frame *disp_frm_info)
{
	ape_priv_t* ape_priv = ape_get_priv();
	
	write(ape_priv->disp_in_fd, disp_frm_info, sizeof(struct av_frame));
	
}

int ape_video_get_disp_decv_info_size(unsigned int *disp_valid_size, unsigned int* decv_valid_size)
{
	ape_priv_t* ape_priv = ape_get_priv();

	ioctl(ape_priv->disp_in_fd, SBMIO_SHOW_VALID_SIZE, disp_valid_size);
	ioctl(ape_priv->decv_out_fd, SBMIO_SHOW_VALID_SIZE, decv_valid_size);
}

int ape_get_io_status(struct VDec_StatusInfo *status)
{
	return vdec_io_control(get_selected_decoder(),VDEC_IO_GET_STATUS ,(UINT32)status);
}

int ape_audio_decore_ioctl(ape_priv_t* ape_priv, int cmd, void *param1, void *param2)
{
	return deca_decore_ioctl(dev_get_by_type(NULL, HLD_DEV_TYPE_DECA), cmd, param1, param2);
}

int ape_config_audio_para(audio_config_info_t* config_info, int* reset)
{
	struct audio_config audio_init;
	int ret;
	ape_priv_t* ape_priv = ape_get_priv();

	memset((void *)&audio_init, 0, sizeof(audio_init));

    if ((config_info->codec_id >= 0x10000) && (config_info->codec_id <= 0x10018))
    {
        ape_priv->audio_merge_mode = 1;
    }
    else
    {
        ape_priv->audio_merge_mode = 0;
    }
	
	audio_init.decode_mode = 1;
	audio_init.sync_mode = config_info->av_sync_mode;
	audio_init.sync_unit = config_info->av_sync_unit;
	audio_init.deca_input_sbm = (APKT_HDR_SBM_IDX<<16)|APKT_DATA_SBM_IDX;
	audio_init.deca_output_sbm = DECA_OUT_SBM_IDX;
	audio_init.snd_input_sbm = ape_priv->av_output_mode ? DECA_OUT_SBM_IDX : SND_IN_SBM_IDX;
	audio_init.codec_id = config_info->codec_id;
	audio_init.bits_per_coded_sample = config_info->bits_per_coded_sample;
	audio_init.sample_rate = config_info->sample_rate;
	audio_init.channels = config_info->channels;
	audio_init.bit_rate = config_info->bit_rate;
	audio_init.block_align = config_info->block_align;
	audio_init.codec_frame_size = config_info->codec_frame_size;
	audio_init.pcm_buf_size = ape_priv->pcm_buf_size;
	audio_init.pcm_buf = ape_priv->pcm_buf_addr;
	if(config_info->extradata != NULL )
	{	
		APE_DEBUG("ape send extradata to audio dec!! data %x size %d\n", (int)config_info->extradata, config_info->extradata_size);
		audio_init.extradata_mode = 1;

		struct av_packet pkt_info;
		pkt_info.data = config_info->extradata;
		pkt_info.size = config_info->extradata_size;

		write(ape_priv->apkt_data_fd,(void*)config_info->extradata,config_info->extradata_size);
		write(ape_priv->apkt_hdr_fd, (void*)&pkt_info, sizeof(struct av_packet));
	}
	else
		audio_init.extradata_mode = 0;
	
	if(ape_get_dbg_on())
	{
		g_ape_audio_byte_rateX10 = (config_info->bit_rate * 10) / 8;
	
		APE_DEBUG("dec info as below:\n");
		APE_DEBUG("id %x\n", config_info->codec_id);
		APE_DEBUG("bits per sample %d\n", config_info->bits_per_coded_sample);
		APE_DEBUG("sample rate %d\n", config_info->sample_rate);
		APE_DEBUG("channel %d\n", config_info->channels);
		APE_DEBUG("bit rate %d\n", config_info->bit_rate);
		APE_DEBUG("frame size %d\n", config_info->codec_frame_size);
		APE_DEBUG("pcm buf addr %x size %x\n", ape_priv->pcm_buf_addr, ape_priv->pcm_buf_size);
		APE_DEBUG("extra buf addr %x size %x\n", config_info->extradata, config_info->extradata_size);	
		APE_DEBUG("time scale num %d\n", config_info->timescale_num);
		APE_DEBUG("time scale den %d\n", config_info->timescale_den);			
		APE_DEBUG("audio byte rate X 10 = %d\n", g_ape_audio_byte_rateX10);
	}
	
	ret = ape_audio_decore_ioctl(ape_priv, DECA_DECORE_INIT, &audio_init, reset);
	if(ret < 0) 
	{
		APE_DEBUG( "deca decore init fail\n");
	}
	
	memcpy((void *)&ape_priv->audio_info, (void *)config_info, sizeof(*config_info));


	return ret;
}

void ape_set_dbg_mode(int mode)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int dbg_on = 0;
	
	if(mode)
		dbg_on = 1;
	
	ioctl(ape_priv->video_fd, ALIVIDEOIO_SET_APE_DBG_MODE, (int)dbg_on);
}

int ape_get_dbg_mode(void)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int dbg_on = 0;

	ioctl(ape_priv->video_fd, ALIVIDEOIO_GET_APE_DBG_MODE, (int)&dbg_on);

	return dbg_on;
}

int ape_get_video_frame_rateX10()
{
	return g_ape_video_frame_rateX10;
}

int ape_get_audio_byte_rateX10()
{
	return g_ape_audio_byte_rateX10;
}

int ape_get_video_timescale_num()
{
	return g_ape_video_timescale_num;
}

int ape_get_video_timescale_den()
{
	return g_ape_video_timescale_den;
}
