/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2012 Copyright (C)
 *
 *  File: adr_ape_playback.c
 *
 *  Description: Hld ape driver dbg function
 *
 *  History:
 *      Date        Author         	Version   Comment
 *      ====        ======         =======   =======
 *  1.  2012.10.22  Sam			0.9		 inited
 ****************************************************************************/

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

#include <adr_basic_types.h>
#include <adr_retcode.h>

#include <osal/osal.h>
#include <hld/adr_hld_dev.h>
#include <hld/decv/adr_decv.h>
#include <hld/ape/adr_ape.h>
#include <hld/dbg/adr_dbg_parser.h>

#include <ali_video_common.h>
#include <ali_sbm_common.h>

#include "ape_priv.h"

#define MAX_READ_SIZE		(2048000)
#define MAX_VIDEO_WR_NUM	(50)
#define MAX_AUDIO_WR_NUM	(50)

extern int g_ape_dbg_on;

static int ape_playing = 0;
static int ape_rgb_enable = 0;
static int ape_stream_mode = 0;
static int ape_preview_play = 0;
static int ape_frame_angle = VDEC_ANGLE_0;
static void wait_finish(void)
{
	ape_priv_t* ape_priv = ape_get_priv();
	unsigned int valid_size = 0, last_size = 0, audio_valid_size = 0;
	int cnt = 0;

	if(ape_playing == 0)
		return;
	
	while(cnt < 100)
	{
		valid_size = ape_show_valid_buf_size(ape_priv->vpkt_data_fd);
                audio_valid_size = ape_show_valid_buf_size(ape_priv->apkt_data_fd);
		if(last_size == valid_size)
			cnt++;
		else
			cnt = 0;
        if(valid_size == 0 && audio_valid_size == 0)
            break;
		osal_task_sleep(50);	

		APE_DEBUG("Waiting ape finish... valid size %d,audio_valid_size %d\n", valid_size, audio_valid_size);

		last_size = valid_size;
	}

	APE_LOG("ape play es is finished now\n");

	ape_playing = 0;
}

static int send_video_packet(void *buffer, int len)
{
	ape_priv_t* ape_priv = ape_get_priv();
	volatile unsigned long free_size = 0, total_size = 0, valid_size = 0, head_free_size = 0;
	int ret = 0;
	while(1)
	{
		free_size = ape_show_free_buf_size(ape_priv->vpkt_data_fd);
		valid_size = ape_show_valid_buf_size(ape_priv->vpkt_data_fd);
		total_size = ape_show_total_buf_size(ape_priv->vpkt_data_fd);
		head_free_size = ape_show_free_buf_size(ape_priv->vpkt_hdr_fd);
	
		if ((free_size > (unsigned long)len) && (valid_size < (total_size*3)/4)
			&& (head_free_size > sizeof(struct av_packet)))
		{
			ret = ape_write_data(ape_priv->vpkt_data_fd, buffer, len);
			if(ret <= 0)
			{
				APE_DEBUG("write vpkt data fail len %d\n", len);
				
				continue;
			}
                        break;
		}

		osal_task_sleep(50);

		APE_DEBUG("wait for SBM free\n");
	}

        return ret;
}

static int send_audio_packet(void *buffer, int len)
{
	ape_priv_t* ape_priv = ape_get_priv();
	volatile unsigned long free_size = 0, total_size = 0, valid_size = 0, head_free_size = 0;
	int ret = 0;

	while(1)
	{
		free_size = ape_show_free_buf_size(ape_priv->apkt_data_fd);
		valid_size = ape_show_valid_buf_size(ape_priv->apkt_data_fd);
		total_size = ape_show_total_buf_size(ape_priv->apkt_data_fd);
		head_free_size = ape_show_free_buf_size(ape_priv->apkt_hdr_fd);
	
		if ((free_size > (unsigned long)len) && (valid_size < (total_size*4)/5)
			&& (head_free_size > (sizeof(struct av_packet) * 10)))
		{
			ret = ape_write_data(ape_priv->apkt_data_fd, buffer, len);
			if(ret < 0)
			{
				APE_DEBUG("write apkt data fail len %d\n", len);
				continue;
			}
                        break;			
		}

		osal_task_sleep(50);

		APE_DEBUG("wait for SBM free\n");		
	}
	
	return ret;
}

void ape_stream_playback(FILE *test_file, UINT32 codec_tag)
{
    video_config_info_t video_cfg;
	unsigned char *read_buf = NULL;
	int audio_extra_size = 0;
	ape_priv_t* ape_priv = NULL;		
	int ret = 0;
    struct av_packet pkt;
    int size = 10 * 1024;
    int read_size = 0;

    codec_tag = h264;

	fseek(test_file, 0, SEEK_SET);
	
	read_buf = malloc(MAX_READ_SIZE);
	if(read_buf == NULL)
	{
		APE_LOG("malloc read buffer fail\n");
		goto EXIT;
	}

	memset(read_buf, 0, MAX_READ_SIZE);
    memset(&pkt, 0, sizeof(pkt));
	
	HLD_vdec_attach();
	HLD_vpo_attach();
	deca_m36_attach(NULL);
	snd_m36_attach(NULL);

	ape_close();
	
	ape_open();
	ape_malloc_av_buf();
	if(ape_rgb_enable)
	{
		ape_enable_rgb_output();	
		ape_av_output_mode(0);
		ape_video_rotation_enable(1, 90);

		ape_rgb_enable = 0;
	}
	else
		ape_av_output_mode(1);

	APE_LOG("start the play ape %x %x es file \n", codec_tag, h264);
	
	ape_priv = ape_get_priv();
	if(ape_priv == NULL)
	{
		APE_LOG("init ape fail\n");

		goto EXIT;
	}	

    memset(&video_cfg, 0, sizeof(video_cfg));
    video_cfg.decoder_flag |= (1 << 31);
    video_cfg.width  = 1280;
    video_cfg.height = 720;
    video_cfg.fourcc = codec_tag;
    video_cfg.sample_aspect_ratio_den = 1;
    video_cfg.sample_aspect_ratio_num = 1;
    video_cfg.time_base_den = 25000;
    video_cfg.time_base_num = 1000;
    ape_vdec_init(codec_tag, &video_cfg);
    
	ape_playing = 1;
	
	while(1)
	{
		if(ape_playing == 0)
		{
			APE_LOG("force ape es playing to stop\n");
			goto EXIT;
		}

        read_size = fread(read_buf, 1, size, test_file);
        
        if(read_size > 0)
        {
            pkt.pts  = AV_NOPTS_VALUE;
            pkt.dts  = AV_NOPTS_VALUE;
            pkt.size = read_size;
            ret = ape_write_data(ape_priv->vpkt_hdr_fd, &pkt, sizeof(pkt));
            ret = send_video_packet(read_buf, read_size);
        }
        else
        {
            APE_LOG("read the es data fail\n");
            break;
        }

	}

EXIT:
	if(ret < 0)
		ape_playing = 0;
	
	wait_finish();

	ape_vdec_release(0);
	ape_audio_release();
	
	if(test_file)
		fclose(test_file);

	if(read_buf)
		free(read_buf);
}

void ape_playback(FILE *test_file)
{
	unsigned char *read_buf = NULL;
	unsigned char *audio_extra_buf = NULL;
	int audio_extra_size = 0;
	unsigned int codec_tag = 0;
	ape_block_header_t block_header;
	ape_priv_t* ape_priv = NULL;		
	int ret = 0;

	fseek(test_file, 0, SEEK_SET);
	
	read_buf = malloc(MAX_READ_SIZE);
	if(read_buf == NULL)
	{
		APE_LOG("malloc read buffer fail\n");
		goto EXIT;
	}

	memset(read_buf, 0, MAX_READ_SIZE);

	fread(read_buf, 1, 8, test_file);
	if(memcmp(read_buf, APE_MAGIC, 8))
	{
		APE_LOG("ape magic at the header of the file is error\n");
		goto EXIT;
	}

	APE_LOG("ape magic recognized\n");
	
	HLD_vdec_attach();
	HLD_vpo_attach();
	deca_m36_attach(NULL);
	snd_m36_attach(NULL);

	ape_close();
	
	ape_open();
	ape_malloc_av_buf();
	if(ape_rgb_enable)
	{
		ape_enable_rgb_output();	
		ape_av_output_mode(0);
		ape_video_rotation_enable(1, 90);

		ape_rgb_enable = 0;
	}
	else
		ape_av_output_mode(1);

	APE_LOG("start the play ape es file\n");
	
	ape_priv = ape_get_priv();
	if(ape_priv == NULL)
	{
		APE_LOG("init ape fail\n");

		goto EXIT;
	}	

	ape_playing = 1;
	
	while(1)
	{
		if(ape_playing == 0)
		{
			APE_LOG("force ape es playing to stop\n");
			goto EXIT;
		}
		
		memset((void *)&block_header, 0, sizeof(block_header));
		ret = fread(read_buf, 1, 8, test_file);
		if(ret <= 0)
		{
			APE_LOG("read block magic fail\n");
			break;
		}
		
		if(memcmp(read_buf, APE_BLOCK_MAGIC, 8))
		{
			APE_LOG("ape block magic fail\n");
			ret = -1;
			goto EXIT;
		}
		
		ret = fread((void *)&block_header, 1, sizeof(block_header), test_file);
		if(ret <= 0)
		{
			APE_LOG("read block header fail\n");
			break;
		}
		
		if(block_header.len > MAX_READ_SIZE)
		{
			APE_LOG("too big block, just skip it len %d\n", block_header.len);
			
			fseek(test_file, block_header.len, SEEK_CUR);
			continue;
		}
		
		ret = fread(read_buf, 1, block_header.len, test_file);
		if(ret < (int)block_header.len)
		{
			APE_LOG("read the block data fail\n");
			break;
		}
		
		switch(block_header.type)
		{
			case BLOCK_VIDEO_DATA:
				ret = send_video_packet(read_buf, block_header.len);
				break;
			case BLOCK_AUDIO_DATA:
				ret = send_audio_packet(read_buf, block_header.len);
				break;
			case BLOCK_VIDEO_HEADER:
				ret = ape_write_data(ape_priv->vpkt_hdr_fd, read_buf, block_header.len);
				break;
			case BLOCK_AUDIO_HEADER:
				ret = ape_write_data(ape_priv->apkt_hdr_fd, read_buf, block_header.len);
				break;
			case BLOCK_VIDEO_INIT_INFO:				
				if(codec_tag == 0)
				{
					APE_LOG("video codec is not inited\n");
					ret = -1;
					goto EXIT;
				}
				
				ape_vdec_init(codec_tag, (video_config_info_t *)read_buf);
				break;
			case BLOCK_AUDIO_INIT_INFO:
			{
				audio_config_info_t *info = (audio_config_info_t *)read_buf;

				if(audio_extra_size)
				{
					info->extradata_size = audio_extra_size;
					info->extradata = audio_extra_buf;
				}
				
				ape_adec_init(info, 0);

				if(audio_extra_buf)
				{
					free(audio_extra_buf);
					
					audio_extra_buf = 0;
				}
				break;
			}
			case BLOCK_VIDEO_EXTRA_DATA:
				ape_vdec_extra_data(read_buf, &block_header.len);
				break;
			case BLOCK_AUDIO_EXTRA_DATA:
				audio_extra_size = block_header.len;
				audio_extra_buf = malloc(audio_extra_size);
				if(audio_extra_buf == NULL)
				{
					APE_LOG("malloc audio extra buf fail\n");
					ret = -1;
					goto EXIT;
				}
				
				memcpy((void *)audio_extra_buf, read_buf, audio_extra_size);
				break;
			case BLOCK_VIDEO_CODEC_TYPE:
				codec_tag = (read_buf[3]<<24) | (read_buf[2]<<16) | (read_buf[1]<<8) | (read_buf[0]);
				APE_DEBUG("video codec tag %x\n", codec_tag);
				break;
			default:				
				APE_DEBUG("block is error or end type %d\n", block_header.type);
				goto EXIT;
				break;
		}

		if(ret < 0)
		{
			APE_DEBUG("write block error block type %d len %d\n", block_header.type, block_header.len);
			
			goto EXIT;
		}
	}

EXIT:
	if(ret < 0)
		ape_playing = 0;
	
	wait_finish();

	ape_vdec_release(0);
	ape_audio_release();
	
	if(test_file)
		fclose(test_file);

	if(read_buf)
		free(read_buf);

	if(audio_extra_buf)
		free(audio_extra_buf);
}

static void ape_play_task(UINT32 upara1, UINT32 upara2)
{
	ape_playing = 0;
    ape_preview_play = 0;
    ape_frame_angle = VDEC_ANGLE_0;

    APE_LOG("\t----------------------------\n");
    APE_LOG("\tEnter P : Preview           \n");
    APE_LOG("\tEnter R : Rotate            \n");
    APE_LOG("\tEnter Q : Quit              \n");
    APE_LOG("\t----------------------------\n\n");
    
    if(ape_stream_mode != 0)
    {
        ape_stream_playback((void *)upara1, ape_stream_mode);
    }
    else
    {
	    ape_playback((void *)upara1);
    }
}

static void ape_key_task(UINT32 upara1, UINT32 upara2)
{
    unsigned char c = 0;
    int ret = 0;
    
    while(1)
    {
        c = getchar();
        if((c == 'Q') || (c == 'q'))
        {
            ape_playing = 0;
            APE_DEBUG("receive exit key %c\n", c);
            break;
        }
        else if((c == 'P') || (c == 'p'))
        {
            APE_DEBUG("receive preview key %c\n", c);
            
            if(ape_preview_play)
            {
                ret = ape_video_zoom(0, 0, 720, 2880); 
                if(ret == 0)
                {
                    ape_preview_play = 0;
                }
            }
            else
            {
                ret = ape_video_zoom(430, 475, 165, 710); 
                if(ret == 0)
                {
                    ape_preview_play = 1;
                }
            }
        }
        else if((c == 'R') || (c == 'r'))
        {
            APE_DEBUG("receive rotate key %c\n", c);
            
            ape_frame_angle++;
            if(ape_frame_angle > VDEC_ANGLE_270)
            {
                ape_frame_angle = VDEC_ANGLE_0;
            }
            ret = ape_video_rotate(ape_frame_angle); 
        }
        else
        {
            APE_DEBUG("press key %c\n", c);
        }
    }
}

void ape_playback_start(void *file_path)
{
	OSAL_T_CTSK t_ctsk;
	FILE *test_file = NULL;
	
	test_file = fopen(file_path, "r");
	if(test_file == NULL)
	{
		APE_LOG("open file fail %s\n", file_path);
		return;
	}

    if(strstr(file_path, ".h264"))
    {
        ape_stream_mode = h264;
    }
    else
    {
        ape_stream_mode = 0;
    }

	t_ctsk.stksz = 0x4000;
	t_ctsk.quantum = 15;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.para1 = (UINT32)(test_file);
	t_ctsk.para2 = 0;
	t_ctsk.name[0] = 'A';
	t_ctsk.name[1] = 'P';
	t_ctsk.name[2] = 'E';
	t_ctsk.task = (FP)ape_play_task;
	if(osal_task_create(&t_ctsk) == OSAL_INVALID_ID)
	{
        fclose(test_file);
		APE_LOG("ape create the play task fail\n");
	}

#if 0
    t_ctsk.stksz = 0x4000;
	t_ctsk.quantum = 15;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.para1 = 0;
	t_ctsk.para2 = 0;
	t_ctsk.name[0] = 'K';
	t_ctsk.name[1] = 'E';
	t_ctsk.name[2] = 'Y';
	t_ctsk.task = (FP)ape_key_task;
	if(osal_task_create(&t_ctsk) == OSAL_INVALID_ID)
	{
		APE_LOG("ape create the key task fail\n");
	}
#endif
}

void ape_playback_stop(void)
{
	ape_playing = 0;
}

int ape_playback_status(void)
{
	int ret = 0;
	
	if(ape_playing)
		ret = 1;

	return ret;
}

int ape_playback_enable_rgb_output(void)
{
	ape_rgb_enable = 1;
}

