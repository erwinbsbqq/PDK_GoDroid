/*******************************************************************************

File name   : ali_hld_venc.c

Description : Video driver stack LINUX platform OS hld-encoder driver source file

Author      : Vic Zhang <Vic.Zhang@Alitech.com>

Create date : Oct 22, 2013

COPYRIGHT (C) ALi Corporation 2013

Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 22, 2013                   Created               V0.1             Vic.Zhang

*******************************************************************************/

#include <osal/osal.h>
#include <adr_mediatypes.h>
#include <adr_retcode.h>
#include <hld/adr_hld_dev.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

//#include <linux/ali_sbm.h>
#include <ali_sbm_common.h>
#include <linux/ali_venc.h>

#include <hld/encoder/ali_hld_venc.h>
#include "venc_HwMapping.h"


#define WAIT_DISPLAY					100			/* 100 ms */
#define WAIT_DISPLAY_END				(200)		/* 20s, */

#define ENCODER_FLG_DATA    0x01
#define ENCODER_FLG_IDLE    0x02

static int venc_hw_Fd;
static int venc_sbm_inFd_Y;
static int venc_sbm_inFd_C;
static int venc_sbm_outFd;
static int venc_sbm_statusFd;

static BOOL venc_task_active;
static const VENC_PARAMS_t *encode_params_content_p = NULL;
static OSAL_ID videnc_flagID;
static OSAL_ID taskID;


static int ComputeFrameNum(FILE *fp, int img_width, int img_height)
{
	int flen = 0;
	int frames = 0;
	fseek(fp, 0, SEEK_END);
	flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	frames = flen / (img_width*img_height*3/2);
	return frames;
}

static int align(int val, int align)
{
	int v = (val + align-1) / align * align;
	return v;
}

/*******************************************************************************
Name        : ExplodeYUVVectors
Description : Explode the Y and C vectors data from YUV420 data in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 22, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 22, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
static RET_CODE ExplodeYUVVectors(const char *FileName,
                                  const int imgWidth, const int imgHeight,
                                  const char *OutFileName)
{
    VENC_ERROR("Encoding. In %s, Out %s\n", FileName, OutFileName);
    const char *yuv = FileName; //only support YUV420 Planar Sequences
    int img_width = imgWidth;
    int img_height = imgHeight;
    int luma_size = img_width * img_height;
    int chroma_size = luma_size >> 1;
    int img_size = luma_size + chroma_size;
    int i;
    int frames;
    enum PicMode encode_target;
    unsigned char* buffer;
    int img_width16;
    int img_height16;
    int img_width_tiled;
    int img_height_tiled;
    int fb_stride;
    int tiled_luma_size;
    unsigned char *tile_luma_buf;
    int img_width_uv_tiled;
    int img_height_uv_tiled;
    int tiled_chroma_size;
    unsigned char* tile_chroma_buf;
    unsigned char* frm_luma_buf;
    unsigned char* frm_u_buf;
    unsigned char* frm_v_buf;

    struct ImgDram dram_out;
    struct ImgPlane image_in;

    int ret = 0;
    int len = 0;
    UINT32 PhyAddr = 0;
    UINT32 venc_time = 0;
    struct videnc_trigger_para venc_encoder_cfg;
    struct videnc_config bitstream_info;
	memset(&venc_encoder_cfg, 0, sizeof(struct videnc_trigger_para));
	memset(&bitstream_info, 0, sizeof(struct videnc_config));

	FILE *fp = fopen(yuv, "rb");

	if (NULL == fp)
    {
		VENC_ERROR("open YUV file failed !\n");
		return ERR_FAILURE;
	}
	if((img_width%2 == 0) && (img_height%2 == 0))
	{
    	frames = ComputeFrameNum(fp, img_width, img_height);
    	encode_target = PIC_FRAME;
    	buffer = (unsigned char*)malloc(img_size);
        if (NULL == buffer)
        {
            fclose(fp);
            return ERR_FAILURE;
        }
        
    	img_width16 = align(img_width, 16); //for mb
    	img_height16 = align(img_height, 16); //for mb
    	img_width_tiled = align(img_width16, TILE_WIDTH); //luma tiled
    	img_height_tiled = align(img_height16, TILE_HEIGHT); //luma tiled
    	fb_stride = img_width_tiled/TILE_WIDTH;
    	tiled_luma_size = img_width_tiled*img_height_tiled;
    	tile_luma_buf = (unsigned char*)malloc(tiled_luma_size);
        if (NULL == tile_luma_buf)
        {
            free(buffer);
            fclose(fp);
            return ERR_FAILURE;
        }

    	img_width_uv_tiled = align(img_width16, TILE_WIDTH);
    	img_height_uv_tiled = align(img_height16/2, TILE_HEIGHT);
    	tiled_chroma_size = img_width_uv_tiled*img_height_uv_tiled;
    	tile_chroma_buf = (unsigned char*)malloc(tiled_chroma_size);
        if (NULL == tile_chroma_buf)
        {
            free(buffer);
            free(tile_luma_buf);
            fclose(fp);
            return ERR_FAILURE;
        }

        /*        **  Output file.  **        */
        const char* img_name = OutFileName;
        FILE *fp_bin = fopen(img_name, "awb");;
        if (NULL == fp_bin)
        {
            VENC_ERROR("open bin_chroma failed !\n");
        }

        VENC_ERROR("In yuv file %d FRMS.\n", frames);
    	for(i = 0; i < frames; i++)
        {
    		frm_luma_buf = buffer;
    		frm_u_buf = buffer + luma_size;
    		frm_v_buf = buffer + luma_size + chroma_size/2;

    		fread(frm_luma_buf, 1, luma_size, fp);
    		fread(frm_u_buf, 1, chroma_size, fp);

            dram_out.chroma_addr = (unsigned int)tile_chroma_buf;
            dram_out.luma_addr = (unsigned int)tile_luma_buf;
            dram_out.stride = fb_stride;

            image_in.buf_y = frm_luma_buf;
            image_in.pitch_y = img_width;
            image_in.buf_width = img_width;
            image_in.buf_height = img_height;
            image_in.buf_cb = frm_u_buf;
            image_in.pitch_uv = img_width/2;
            image_in.buf_cr = frm_v_buf;
            image_in.buf_width_c = img_width/2;
            image_in.buf_height_c = img_height/2;

    		LoadEncFrmToDram(&dram_out, encode_target, &image_in);


            /*/ First verify the divided data in S3921 board.
            {
                char* img_name11 = malloc(strlen(yuv)+6+4);
                sprintf(img_name11, "%s_%d_y_M.bin", "a1", i);
                VENC_ERROR("Out bin file:%s\n", img_name11);
                FILE *fp_outbin = fopen(img_name11, "awb");
                fwrite(tile_luma_buf, 1, tiled_luma_size, fp_outbin);
                fclose(fp_outbin);
                sprintf(img_name11, "%s_%d_c_M.bin", "a1", i);
                VENC_ERROR("Out bin file:%s\n", img_name11);
                fp_outbin = fopen(img_name11, "awb");
                fwrite(tile_chroma_buf, 1, tiled_chroma_size, fp_outbin);
                fclose(fp_outbin);
                free(img_name11);
                continue;
            }
            // End to verify.*/



            len = write(venc_sbm_inFd_Y, tile_luma_buf, tiled_luma_size);
            if( len != tiled_luma_size )
            {
                VENC_ERROR("Write Y data into sbm buffer failed!\n");

            }

            len = write(venc_sbm_inFd_C, tile_chroma_buf, tiled_chroma_size);
            if( len != tiled_chroma_size )
            {
                VENC_ERROR("Write C data into sbm buffer failed!\n");
            }

            venc_encoder_cfg.Y_length = tiled_luma_size;
            venc_encoder_cfg.C_length = tiled_chroma_size;
            venc_encoder_cfg.frm_width = img_width;
            venc_encoder_cfg.frm_height = img_height;
            if (0 == i)
            {
                venc_encoder_cfg.job_status = 0;
            }
            else if (frames-1 == i)
            {
                venc_encoder_cfg.job_status = 2;
            }
            else
            {
                venc_encoder_cfg.job_status = 1;
            }
            ret = ioctl(venc_hw_Fd, VENC_IOWR_ENCODE, (int)&venc_encoder_cfg);
            if (ret < 0)
            {
        		VENC_ERROR("Fail to VENC_CMD_ENCODE! Return: %d \n", ret);
        		return ERR_DEV_ERROR;
            }



            /*// === BEGIN Verify Fetch Y/C data back.
                venc_time = 0;
                do
                {
                    // Fetch the bitstream start address and length.
                    len = read(venc_sbm_statusFd, &bitstream_info, sizeof(struct videnc_config));
                    if(len > 0)
                    {
                        break;
                    }
                    else
                    {
                        if( venc_time ++ == WAIT_DISPLAY_END)
                        {
                            //ret1 = ERR_TIME_OUT;
                            VENC_ERROR("\nWait decoding, len:%d, timeout: %dms\n ", len, venc_time*(WAIT_DISPLAY));
                            break;
                        }
                        VENC_DEBUG("\nWait: %dms\n", venc_time*(WAIT_DISPLAY));
                        osal_task_sleep(WAIT_DISPLAY);
                    }
                }while (1);

                if(len > 0)
                {
                    ret = ioctl(venc_hw_Fd, VENC_IOWR_BS_MAPADDR, &bitstream_info.buffer_addr);
                    PhyAddr = (UINT32)mmap(NULL, bitstream_info.buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, venc_hw_Fd, 0);
                    bitstream_info.buffer_addr = PhyAddr;
                    //fwrite((unsigned char*)bitstream_info.buffer_addr, 1, bitstream_info.buffer_size, fp_bin);
                    {
                        char* img_name11 = malloc(strlen(yuv)+6+4);
                        sprintf(img_name11, "%s_%d_y.bin", "a1", i);
                        VENC_ERROR("Out file:%s, addr %p, size %p\n", img_name11,
                                    bitstream_info.buffer_addr, bitstream_info.buffer_size);
                        FILE *fp_outbin = fopen(img_name11, "awb");
                        fwrite((unsigned char*)bitstream_info.buffer_addr, 1, bitstream_info.buffer_size, fp_outbin);
                        fclose(fp_outbin);
                        if(img_name11) free(img_name11);
                    }
                }

                venc_time = 0;
                do
                {
                    // Fetch the bitstream start address and length.
                    len = read(venc_sbm_statusFd, &bitstream_info, sizeof(struct videnc_config));
                    if(len > 0)
                    {
                        break;
                    }
                    else
                    {
                        if( venc_time ++ == WAIT_DISPLAY_END)
                        {
                            //ret1 = ERR_TIME_OUT;
                            VENC_ERROR("\nWait decoding, len:%d, timeout: %dms\n ", len, venc_time*(WAIT_DISPLAY));
                            break;
                        }
                        VENC_DEBUG("\nWait: %dms\n", venc_time*(WAIT_DISPLAY));
                        osal_task_sleep(WAIT_DISPLAY);
                    }
                }while (1);

                if(len > 0)
                {
                    ret = ioctl(venc_hw_Fd, VENC_IOWR_BS_MAPADDR, &bitstream_info.buffer_addr);
                    PhyAddr = (UINT32)mmap(NULL, bitstream_info.buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, venc_hw_Fd, 0);
                    bitstream_info.buffer_addr = PhyAddr;
                    {
                        char* img_name11 = malloc(strlen(yuv)+6+4);
                        sprintf(img_name11, "%s_%d_c.bin", "a1", i);
                        VENC_ERROR("Out file:%s, addr %p, size %p\n", img_name11,
                                    bitstream_info.buffer_addr, bitstream_info.buffer_size);
                        FILE *fp_outbin = fopen(img_name11, "awb");
                        fwrite((unsigned char*)bitstream_info.buffer_addr, 1, bitstream_info.buffer_size, fp_outbin);
                        fclose(fp_outbin);
                        if(img_name11) free(img_name11);
                    }
                }
                continue;
            // === END to Verify. */




            venc_time = 0;
            do
            {
                /*** Fetch the bitstream start address and length. */
                len = read(venc_sbm_statusFd, &bitstream_info, sizeof(struct videnc_config));
                if(len > 0)
                {
                    break;
                }
                else
                {
                    if( venc_time ++ == WAIT_DISPLAY_END)
                    {
                        //ret1 = ERR_TIME_OUT;
                        VENC_ERROR("\nWait decoding, len:%d, timeout: %dms\n ", len, venc_time*(WAIT_DISPLAY));

                        //return ERR_DEV_ERROR;
                        break;
                    }
                    //VENC_DEBUG("\nWait: %dms\n", venc_time*(WAIT_DISPLAY));
                    osal_task_sleep(WAIT_DISPLAY);
                }
            }while (1);

            if(len > 0)
            {
                //VENC_DEBUG("#");
            }
            else
            {
                VENC_DEBUG("\nEncoder Exception.");
            }

            if(len > 0 && (frames-1 == i))
            {
                ret = ioctl(venc_hw_Fd, VENC_IOWR_BS_MAPADDR, &bitstream_info.buffer_addr);
                PhyAddr = (UINT32)mmap(NULL, bitstream_info.buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, venc_hw_Fd, 0);
                bitstream_info.buffer_addr = PhyAddr;
                fwrite((unsigned char*)bitstream_info.buffer_addr, 1, bitstream_info.buffer_size, fp_bin);
                VENC_DEBUG("\nOut file:%s, addr %p, size %p\n", OutFileName,
                                    bitstream_info.buffer_addr, bitstream_info.buffer_size);

                munmap((void *)PhyAddr, bitstream_info.buffer_size);
            }

        }

    
        if (fp_bin != NULL) 
        {
            fclose(fp_bin);
        }
     
        if (buffer != NULL) 
       {
            free(buffer);
        }
    
        if ( tile_luma_buf != NULL) 
        {
            free(tile_luma_buf);
        }

        if ( tile_chroma_buf != NULL) 
        {
            free(tile_chroma_buf);
        }

/*
    if (NULL tile_luma_buf) 
            free(tile_luma_buf);
        if (tile_chroma_buf) free(tile_chroma_buf);
        if (buffer) free(buffer);
*/        
    }

    if (fp != NULL) 
    {
            fclose(fp);
    }


    return SUCCESS;
}

/*******************************************************************************
Name        : videnc_task
Description : The task to monitor the encoder in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 22, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 22, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
static void videnc_task(void)
{
    /* Statement Local Variables. --------------------------------------------*/
    UINT32 ret_flag = 0;

    /* Check for input parameters. -------------------------------------------*/


    /* Initialize and Variable Assignments. ----------------------------------*/


    /* Funciton Entity Contents. ---------------------------------------------*/
    //VENC_ERROR("videnc task running.\n");

    while (TRUE == venc_task_active)
    {
        if (OSAL_E_OK != osal_flag_wait(&ret_flag, videnc_flagID, ENCODER_FLG_DATA,
                                        OSAL_TWF_ORW | OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME))
    	{
#if 0    	
            if (FALSE == venc_task_active)
            {
                osal_flag_set(videnc_flagID, ENCODER_FLG_IDLE);
                break;
            }
#endif             
    	    VENC_ERROR( "\nFunc: %s, Line: %d: " "osal_flag_wait(%d) failed!\n", __FUNCTION__, __LINE__, ENCODER_FLG_IDLE);
    	    osal_flag_set(videnc_flagID, ENCODER_FLG_IDLE);
            continue;
    	}

#if 0
        if (FALSE == venc_task_active)
        {
            osal_flag_set(videnc_flagID, ENCODER_FLG_IDLE);
            break;
        }
#endif 

        VENC_ERROR("%s get the ENCODER_FLG_DATA.\n", __FUNCTION__);

        if (NULL != encode_params_content_p)
        {
            ExplodeYUVVectors(encode_params_content_p->inFile,
                              encode_params_content_p->nWidth,
                              encode_params_content_p->nHeight,
                              encode_params_content_p->bitstreams);

        }

        VENC_ERROR("%s finish ExplodeYUVVectors().\n", __FUNCTION__);

        osal_flag_set(videnc_flagID, ENCODER_FLG_IDLE);
        encode_params_content_p = NULL;
	}

}

/*******************************************************************************
Name        : videnc_task_init
Description : Be invoked to initialize the encoder task in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 22, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 22, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
static RET_CODE videnc_task_init(void)
{
	OSAL_T_CTSK venc_tsk;
	memset(&venc_tsk, 0, sizeof(OSAL_T_CTSK));
	venc_tsk.task = (OSAL_T_TASK_FUNC_PTR)videnc_task;
	venc_tsk.stksz = 0x4000;
	venc_tsk.quantum = 10;
	venc_tsk.itskpri = OSAL_PRI_NORMAL;
	venc_tsk.name[0] = 'E';
	venc_tsk.name[1] = 'N';
	venc_tsk.name[2] = 'C';

	taskID = (OSAL_ID)osal_task_create(&venc_tsk);
	if (taskID == OSAL_INVALID_ID)
	{
	    VENC_ERROR("\nFail to create task!\n");
	    return ERR_FAILURE;
	}

    venc_task_active = TRUE;
    osal_flag_set(videnc_flagID, ENCODER_FLG_IDLE);

	return SUCCESS;
}

static RET_CODE videnc_hw_init(void)
{
	const char *sbm_device_Y = "/dev/ali_sbm8";
	const char *sbm_device_C = "/dev/ali_sbm9";
    const char *sbm_device_status = "/dev/ali_sbm10";
	const char *venc_device0 = "/dev/ali_venc0";

	struct sbm_config sbm_info;
	struct videnc_config map_info;
    struct videnc_config encoder_info;
	struct videnc_see_config encoder_init_cfg;

	RET_CODE ret = SUCCESS;
	UINT32 run_addr, run_size;

	memset(&sbm_info, 0, sizeof(struct sbm_config));
	memset(&encoder_init_cfg, 0, sizeof(struct videnc_see_config));

	venc_hw_Fd = open(venc_device0, O_RDWR | O_CLOEXEC);
	if(venc_hw_Fd < 0)
	{
		VENC_ERROR("\nFail to open device: %s ! Return: %d \n", venc_device0, venc_hw_Fd);
		return ERR_NO_DEV;
	}

    /*
    **  Fetch necessary buffer parameters to create SBM.
    **
    */
	ret = ioctl(venc_hw_Fd, VENC_IOWR_SBMMEM, (int)&map_info);
	if(ret < 0)
	{
		VENC_ERROR("\nFail to VENC_IOWR_SBMMEM ! Return: %d \n", ret);
		return ERR_DEV_ERROR;
	}
	run_addr = (UINT32)(map_info.buffer_addr) & 0x1FFFFFFF;
	run_size = map_info.buffer_size;
    /*
    **  Create In buffer for SBM.
    **
    */
	venc_sbm_inFd_Y = open(sbm_device_Y, O_WRONLY | O_CLOEXEC);
	if(venc_sbm_inFd_Y < 0)
	{
		VENC_ERROR("\nFail to open device: %s ! Return: %d \n", sbm_device_Y, venc_sbm_inFd_Y);
		return ERR_NO_DEV;
	}

	sbm_info.buffer_addr = run_addr;
	sbm_info.buffer_size = 0x200000;
	sbm_info.block_size = 0x20000;
    sbm_info.reserve_size = 0;
	sbm_info.wrap_mode = SBM_MODE_NORMAL;
	sbm_info.lock_mode = SBM_SPIN_LOCK;//SBM_MUTEX_LOCK;
	run_addr += sbm_info.buffer_size;
	run_size -= sbm_info.buffer_size;
	ret = ioctl(venc_sbm_inFd_Y, SBMIO_CREATE_SBM, (int)&sbm_info);
	if(ret < 0)
	{
		VENC_ERROR("\nFail to create data sbm ! Return: %d \n", ret);
		return ERR_DEV_ERROR;
	}

	venc_sbm_inFd_C = open(sbm_device_C, O_WRONLY | O_CLOEXEC);
	if(venc_sbm_inFd_C < 0)
	{
		VENC_ERROR("\nFail to open device: %s ! Return: %d \n", sbm_device_C, venc_sbm_inFd_C);
		return ERR_NO_DEV;
	}

	sbm_info.buffer_addr = run_addr;
	sbm_info.buffer_size = 0x100000;
	sbm_info.block_size = 0x20000;
    sbm_info.reserve_size = 0;
	sbm_info.wrap_mode = SBM_MODE_NORMAL;
	sbm_info.lock_mode = SBM_SPIN_LOCK;//SBM_MUTEX_LOCK;
	run_addr += sbm_info.buffer_size;
	run_size -= sbm_info.buffer_size;
	ret = ioctl(venc_sbm_inFd_C, SBMIO_CREATE_SBM, (int)&sbm_info);
	if(ret < 0)
	{
		VENC_ERROR("\nFail to create data sbm ! Return: %d \n", ret);
		return ERR_DEV_ERROR;
	}

    /*
    **  Create Status buffer for SBM.
    **
    */
	venc_sbm_statusFd = open(sbm_device_status, O_RDONLY | O_CLOEXEC);
	if(venc_sbm_statusFd < 0)
	{
		VENC_ERROR("\nFail to open device: %s ! Return: %d \n", sbm_device_status, venc_sbm_statusFd);
		return ERR_NO_DEV;
	}

	sbm_info.buffer_addr = run_addr;
	sbm_info.buffer_size = 1024;//1024*sizeof(struct info_end);
	sbm_info.reserve_size = 0;//10*sizeof(struct info_end);
	sbm_info.wrap_mode = SBM_MODE_NORMAL;
	sbm_info.lock_mode = SBM_SPIN_LOCK;//SBM_MUTEX_LOCK;
	run_addr += sbm_info.buffer_size;
	run_size -= sbm_info.buffer_size;
	ret = ioctl(venc_sbm_statusFd, SBMIO_CREATE_SBM, (int)&sbm_info);
	if(ret < 0)
	{
		VENC_ERROR("\nFail to create info_end_fd sbm ! Return: %d \n", ret);
		return ERR_DEV_ERROR;
	}

    /*
    **  It's not necessary to fetch initial parameters to configure SEE Encoder buffer.
    **  Trigger SEE Encoder to initialize itself.
    **
    */
    encoder_init_cfg.y_sbm_idx = 8;
    encoder_init_cfg.c_sbm_idx = 9;
    encoder_init_cfg.status_idx = 10;
    encoder_init_cfg.yuv_width = encode_params_content_p->nWidth;
    encoder_init_cfg.yuv_height = encode_params_content_p->nHeight;
	ret = ioctl(venc_hw_Fd, VENC_IOWR_START, (int)&encoder_init_cfg);
	if(ret < 0)
    {
		VENC_ERROR("\nFail to VENC_IOWR_START! Return: %d \n", ret);
		return ERR_DEV_ERROR;
	}

	return ret;
}


/*******************************************************************************
Name        : videnc_hw_unload
Description : Be invoked to release the encoder in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 22, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 22, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
int videnc_hw_unload (void)
{
    /* Statement Local Variables. --------------------------------------------*/
	RET_CODE ret = 0;
    unsigned int stopInfo = 1;

    /* Check for input parameters. -------------------------------------------*/


    /* Initialize and Variable Assignments. ----------------------------------*/


    /* Funciton Entity Contents. ---------------------------------------------*/
    ret = ioctl(venc_hw_Fd, VENC_IOWR_STOP, (int)&stopInfo);
	if(ret < 0)
	{
		VENC_ERROR("\nFail to VENC_IOWR_STOP! Return: %d \n", ret);
		return ERR_DEV_ERROR;
	}
    osal_task_sleep(200);
    ret = ioctl(venc_hw_Fd, VENC_IOWR_RELEASE);
    if(ret < 0)
    {
        VENC_ERROR("\nFail to VENC_IOWR_RELEASE! Return: %d \n", ret);
        return ERR_DEV_ERROR;
    }
    osal_task_sleep(100);

    close(venc_sbm_statusFd);
    close(venc_sbm_inFd_C);
    close(venc_sbm_inFd_Y);
    close(venc_hw_Fd);

    return 0;
}

/*******************************************************************************
Name        : videnc_init
Description : Be invoked to initialize the encoder in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 22, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 22, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
int videnc_init(void)
{
    /* Statement Local Variables. --------------------------------------------*/
	RET_CODE ret = 0;


    /* Check for input parameters. -------------------------------------------*/


    /* Initialize and Variable Assignments. ----------------------------------*/
    //memset(encode_params_content_p, 0, sizeof(VENC_PARAMS_t));
    videnc_flagID = osal_flag_create(0);
	if (OSAL_INVALID_ID == videnc_flagID)
	{
		VENC_ERROR( "Fail to call osal_flag_create!" );
		return (-1);

	}

    /* Funciton Entity Contents. ---------------------------------------------*/
    ret = videnc_hw_init();
    if(ret)
    {
        VENC_ERROR( "Fail to initialize encoder driver!\n" );

    }

    ret = videnc_task_init();
    if(ret)
    {
        VENC_ERROR( "Fail to initialize encoder task!\n" );

    }

    //VENC_ERROR( "End to videnc_init().\n" );

    return ret;
}

/*******************************************************************************
Name        : videnc_uninit
Description : Be invoked to release the encoder in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 22, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 22, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
int videnc_uninit(void)
{
    /* Statement Local Variables. --------------------------------------------*/


    /* Check for input parameters. -------------------------------------------*/


    /* Initialize and Variable Assignments. ----------------------------------*/


    /* Funciton Entity Contents. ---------------------------------------------*/
    venc_task_active = FALSE;
    osal_flag_set(videnc_flagID, ENCODER_FLG_DATA);

    return 0;
}

/*******************************************************************************
Name        : TransferSH
Description : Be invoked to verify the encoder in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 29, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 29, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
static RET_CODE TransferSH(const char *FileName,
                          const int imgWidth, const int imgHeight,
                          const char *OutFileName)
{
    FILE *fp;
    int tiled_luma_size, len, ret, venc_time;
    unsigned char* tile_luma_buf;
    struct videnc_trigger_para venc_encoder_cfg;
    struct videnc_config bitstream_info;

    VENC_DEBUG("%s running. In %s, Out %s\n", __FUNCTION__, FileName, OutFileName);

#if 0
    fp = fopen(FileName, "r");
    if (NULL == fp)
    {
        VENC_DBG_PRINT("open failed!\n");
        
        return ERR_FAILURE;  
    }
#endif

    if((fp = fopen(FileName, "r")) == 0)
    {
        VENC_DBG_PRINT("open failed!\n");
        exit(1);
    }
    
    fseek(fp, 0L, SEEK_END);
    tiled_luma_size = ftell(fp);
    rewind(fp);

    VENC_DEBUG("In file size is %d\n", tiled_luma_size);

    tile_luma_buf = (unsigned char*)malloc(tiled_luma_size);
    if (NULL == tile_luma_buf)
    {
        fclose(fp);
        return ERR_FAILURE;
    }
    
    fread(tile_luma_buf, 1, tiled_luma_size , fp);

    len = write(venc_sbm_inFd_Y, tile_luma_buf, tiled_luma_size);
    if( len != tiled_luma_size )
    {
        VENC_ERROR("Write Y data into sbm buffer failed!\n");
    }
    len = write(venc_sbm_inFd_C, tile_luma_buf, tiled_luma_size);
    if( len != tiled_luma_size )
    {
        VENC_ERROR("Write Y data into sbm buffer failed!\n");
    }
    venc_encoder_cfg.Y_length = tiled_luma_size;
    venc_encoder_cfg.C_length = tiled_luma_size;
    venc_encoder_cfg.frm_width = imgWidth;
    venc_encoder_cfg.frm_height = imgHeight;
    venc_encoder_cfg.job_status = 0;


    ret = ioctl(venc_hw_Fd, VENC_IOWR_ENCODE, (int)&venc_encoder_cfg);
    if (ret < 0)
    {
        VENC_ERROR("Fail to VENC_CMD_ENCODE! Return: %d \n", ret);

        fclose(fp);
        free(tile_luma_buf);
        return ERR_DEV_ERROR;
    }

    venc_time = 0;
    do
    {
     // Fetch the bitstream start address and length.
     len = read(venc_sbm_statusFd, &bitstream_info, sizeof(struct videnc_config));
     if(len > 0)
     {
         break;
     }
     else
     {
         if( venc_time ++ == WAIT_DISPLAY_END)
         {
             //ret1 = ERR_TIME_OUT;
             VENC_ERROR("\nWait decoding, len:%d, timeout: %dms\n ", len, venc_time*(WAIT_DISPLAY));
             break;
         }
         VENC_DEBUG("\nWait: %dms\n", venc_time*(WAIT_DISPLAY));
         osal_task_sleep(WAIT_DISPLAY);
     }
    }while (1);

    if(len > 0)
    {
     ret = ioctl(venc_hw_Fd, VENC_IOWR_BS_MAPADDR, &bitstream_info.buffer_addr);
     UINT32 PhyAddr = (UINT32)mmap(NULL, bitstream_info.buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, venc_hw_Fd, 0);
     bitstream_info.buffer_addr = PhyAddr;

         {
             char* img_name11 = malloc(16);
             if (NULL == img_name11)
             {
                 fclose(fp);
                 free(tile_luma_buf);                 
                 return ERR_FAILURE;
             }
             sprintf(img_name11, "%s.bin", "sh");
#if 0                
             VENC_ERROR("Out file:%s, addr %p, size %p\n", img_name11,
                                      bitstream_info.buffer_addr, bitstream_info.buffer_size);
#endif
             FILE *fp_outbin = fopen(img_name11, "awb");             

             if(NULL == fp_outbin)
             {
                 VENC_DBG_PRINT("open failed!\n");
                 fclose(fp);
                 free(tile_luma_buf); 
                 free(img_name11); 
                 exit(1);
             }
             
             fwrite((unsigned char*)bitstream_info.buffer_addr, 1, bitstream_info.buffer_size, fp_outbin);
             fclose(fp_outbin);
             free(img_name11);
             //if(img_name11) free(img_name11);
         }
     }


    fclose(fp);
    free(tile_luma_buf);

}


static RET_CODE FastTransferSH(const char *FileName,
                          const int imgWidth, const int imgHeight,
                          const char *OutFileName)
{
    FILE *fp;
    int tiled_luma_size, len, ret, venc_time;
    unsigned char* tile_luma_buf;
    struct videnc_trigger_para venc_encoder_cfg;
    struct videnc_config bitstream_info;
    int devMEM;

    VENC_DEBUG("%s running. In %s, Out %s\n", __FUNCTION__, FileName, OutFileName);

    if((fp = fopen(FileName, "r")) == 0)
    {
        VENC_ERROR("open failed!\n");
        exit(1);
    }

    fseek(fp, 0L, SEEK_END);
    tiled_luma_size = ftell(fp);
    rewind(fp);

    VENC_DEBUG("In file size is %d\n", tiled_luma_size);

    tile_luma_buf = (unsigned char*)malloc(tiled_luma_size);
    if (NULL == tile_luma_buf)
    {
        fclose(fp);
        return ERR_FAILURE;
    }
        
    fread(tile_luma_buf, 1, tiled_luma_size , fp);

    len = write(venc_sbm_inFd_Y, tile_luma_buf, tiled_luma_size);
    if( len != tiled_luma_size )
    {
        VENC_ERROR("Write Y data into sbm buffer failed!\n");
    }
    len = write(venc_sbm_inFd_C, tile_luma_buf, tiled_luma_size);
    if( len != tiled_luma_size )
    {
        VENC_ERROR("Write Y data into sbm buffer failed!\n");
    }
    venc_encoder_cfg.Y_length = tiled_luma_size;
    venc_encoder_cfg.C_length = tiled_luma_size;
    venc_encoder_cfg.frm_width = imgWidth;
    venc_encoder_cfg.frm_height = imgHeight;
    venc_encoder_cfg.job_status = 0;


    ret = ioctl(venc_hw_Fd, VENC_IOWR_ENCODE, (int)&venc_encoder_cfg);

    if(ret < 0)
    {
        VENC_ERROR("Fail to VENC_CMD_ENCODE! Return: %d \n", ret);

        fclose(fp);
        free(tile_luma_buf);
        return ERR_DEV_ERROR;
    }
    venc_time = 0;
    do
    {
        // Fetch the bitstream start address and length.
        len = read(venc_sbm_statusFd, &bitstream_info, sizeof(struct videnc_config));
        if(len > 0)
        {
         break;
        }
        else
        {
         if( venc_time ++ == WAIT_DISPLAY_END)
         {
             //ret1 = ERR_TIME_OUT;
             VENC_ERROR("\nWait decoding, len:%d, timeout: %dms\n ", len, venc_time*(WAIT_DISPLAY));
             break;
         }
         VENC_DEBUG("\nWait: %dms\n", venc_time*(WAIT_DISPLAY));
         osal_task_sleep(WAIT_DISPLAY);
        }
    }while (1);

    if(len > 0)
    {
        devMEM = open("/dev/mem", O_RDWR|O_SYNC);
        if (devMEM == -1)
        {
            fclose(fp);
            free(tile_luma_buf);
            return (-1);
        }

        //UINT32 PhyAddr = mmap(NULL, bitstream_info.buffer_size, PROT_READ|PROT_WRITE, MAP_SHARED, devMEM, bitstream_info.buffer_addr & 0x1fffffff);
        UINT32 PhyAddr = (UINT32)mmap(NULL, bitstream_info.buffer_size, PROT_READ|PROT_WRITE, MAP_SHARED, devMEM, __pa(bitstream_info.buffer_addr));
        if (PhyAddr == 0)
        {
            VENC_ERROR("%s NULL pointer!\n", __FUNCTION__);

            fclose(fp);
            free(tile_luma_buf);
            return (-1);
        }
        bitstream_info.buffer_addr = PhyAddr;
            {
                char* img_name11 = malloc(16);

                if (NULL == img_name11)
                {
                    fclose(fp);
                    free(tile_luma_buf);                 
                    return ERR_FAILURE;
                }
                sprintf(img_name11, "%s.bin", "sh");
#if 0                
                VENC_ERROR("Out file:%s, addr %p, size %p\n", img_name11,
                                             bitstream_info.buffer_addr, bitstream_info.buffer_size);
#endif
                FILE *fp_outbin = fopen(img_name11, "awb");
                
                if(NULL == fp_outbin)
                {
                    VENC_DBG_PRINT("open failed!\n");

                    fclose(fp);
                    free(tile_luma_buf); 
                    free(img_name11); 
                    exit(1);
                }

                fwrite((unsigned char*)bitstream_info.buffer_addr, 1, bitstream_info.buffer_size, fp_outbin);
                fclose(fp_outbin);
                free(img_name11);
                //if(img_name11) free(img_name11);
            }
        close(devMEM);
        munmap((void *)PhyAddr, bitstream_info.buffer_size);
    }


    fclose(fp);
    free(tile_luma_buf);

}


/*******************************************************************************
Name        : videnc_encode
Description : Be invoked to trigger the encoder in the low level.
Parameters  :
Assumptions :
Limitations :
Returns     :
Author      : Vic Zhang <Vic.Zhang@Alitech.com>
Create date : Oct 22, 2013
Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Oct 22, 2013                   Created               V0.1             Vic.Zhang
*******************************************************************************/
int videnc_encode(const VENC_PARAMS_t *params)
{
    /* Statement Local Variables. --------------------------------------------*/
	UINT32 ret_flag = 0;
	RET_CODE ret = SUCCESS;

    /* Check for input parameters. -------------------------------------------*/
    if(NULL == params)
    {
        return -1;
    }

    /* Initialize and Variable Assignments. ----------------------------------*/


    /* Funciton Entity Contents. ---------------------------------------------*/
    encode_params_content_p = params;


    /*---------------- Method 1: Create Task to do encoder. ------------------*/
#if 0
    videnc_init();
    osal_task_sleep(100);
    encode_params_content_p = params;
    osal_flag_set(videnc_flagID, ENCODER_FLG_DATA);

    osal_task_sleep(10000);

    if (OSAL_E_OK != osal_flag_wait(&ret_flag, videnc_flagID, ENCODER_FLG_IDLE,
                                    OSAL_TWF_ANDW | OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME))
	{
		VENC_ERROR( "\nFunc: %s, Line: %d: " "osal_flag_wait(%d) failed!\n", __FUNCTION__, __LINE__, ENCODER_FLG_IDLE);
		return ERR_FAILURE;
	}

    VENC_ERROR( "In %s get the ENCODER_FLG_IDLE.\n", __FUNCTION__);

    videnc_uninit();
#endif

    /*---------------- Method 2: directly block to do encoder. ------------------*/
#if 1
    ret = videnc_hw_init();
    //osal_task_sleep(400);

    if(ret)
    {
        VENC_ERROR( "Fail to initialize encoder driver!\n" );

    }

    if (NULL != encode_params_content_p)
    {
        ret = ExplodeYUVVectors(encode_params_content_p->inFile,
                          encode_params_content_p->nWidth,
                          encode_params_content_p->nHeight,
                          encode_params_content_p->bitstreams);

    }
    if (SUCCESS == ret)
    {
        videnc_hw_unload();
    }
#endif


    /*---------------- Method 3: directly block to do encoder. ------------------*/
#if 0
    videnc_init();

    if (NULL != encode_params_content_p)
    {
        TransferSH(encode_params_content_p->inFile,
                  encode_params_content_p->nWidth,
                  encode_params_content_p->nHeight,
                  encode_params_content_p->bitstreams);

    }
#endif


    /*---------------- Method 4: directly fast block to do encoder. ------------------*/
#if 0
    videnc_init();

    if (NULL != encode_params_content_p)
    {
        FastTransferSH(encode_params_content_p->inFile,
                  encode_params_content_p->nWidth,
                  encode_params_content_p->nHeight,
                  encode_params_content_p->bitstreams);
    }
#endif

    return 0;
}
