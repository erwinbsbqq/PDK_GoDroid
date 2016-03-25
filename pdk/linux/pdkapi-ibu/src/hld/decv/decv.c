/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: decv.c
 *
 *  Description: Hld vdec driver
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2003.04.11  David Wang     0.1.000   Initial
 *  2.  2010.03.11  Sam         4.0     Support Linux Driver
 ****************************************************************************/

#include <osal/osal.h>
//corei#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <sys_config.h>
#include <mediatypes.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
//corei#include <err/errno.h>

#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>

#include <misc/rfk.h>

#if 1
#define VDEC_PRF(...)   do{}while(0)
#else
#define VDEC_PRF printf
#endif

#define ENTRY       VDEC_PRF("%s : in\n", __FUNCTION__)
#define EXIT        VDEC_PRF("%s : out\n", __FUNCTION__)

#define MUTEX_LOCK osal_mutex_lock(priv->mutex_id, OSAL_WAIT_FOREVER_TIME)

#define MUTEX_UNLOCK osal_mutex_unlock(priv->mutex_id)

#define VDEC_MAX_DEV_NUM            2

#define VDEC_MPG_DEV                0
#define VDEC_AVC_DEV                1

struct vdec_private
{
    int handle;
    char *file_path;

    volatile int open; /* 0 : device is closed; 1 : device is opened */
    volatile int start; /* 0 : device is idle; 1 : device is working*/

    volatile void *req_buf;
    int req_size;
    int update_size;

    VDecCBFunc  pcb_first_showed;
    VDecCBFunc  pcb_mode_switch_ok;
    VDecCBFunc  pcb_backward_restart_gop;
    VDecCBFunc  pcb_first_head_parsed;
    VDecCBFunc  pcb_first_i_deocded;

    int task_id;
    int mutex_id;
    int rfk_port;

    UINT32 first_show:1;
    UINT32 res:31;

    struct ali_video_out_info_pars out_info;
};

static char decv_avc_name[] = {"DECV_AVC_0"};
static char decv_mpg_name[] = {"DECV_S3601_0"};
static char decv_mpeg4_name[] = {"DECV_MPEG4_0"};

static struct vdec_device *m_vdec_dev[VDEC_MAX_DEV_NUM] = {NULL, NULL};
static struct vdec_private *m_vdec_priv[VDEC_MAX_DEV_NUM] = {NULL, NULL};
static struct vdec_private m_vdec_priv_data;
static char video_path[] = "/dev/ali_video0";
static int m_is_attatch = 0;

/* 0 : mpeg2
    1 : avc
*/
static int cur_decv_type = 0xFFFFFFFF;
static BOOL cur_decv_mode = 0xFFFFFFFF;

static int vdec_attach(char *file_path, struct vdec_device **pdev, int mpg)
{
    struct vdec_device *vdec_dev;
    struct vdec_private *priv;
    char *name = (mpg) ? decv_mpg_name : decv_avc_name;

    *pdev = NULL;

    vdec_dev = dev_alloc(name,HLD_DEV_TYPE_DECV,sizeof(struct vdec_device));
    if(vdec_dev == NULL)
    {
        VDEC_PRF("malloc vdec dev fail\n");
        return 0;
    }

    priv = &m_vdec_priv_data;
    if(priv == NULL)
    {
        VDEC_PRF("malloc osd priv fail\n");
        return 0;
    }

    memset((void *)priv, 0, sizeof(*priv));
    priv->file_path = file_path;
    vdec_dev->priv = (void *)priv;
    vdec_dev->next = NULL;
    vdec_dev->flags = 0;

    if(dev_register(vdec_dev) != RET_SUCCESS)
    {
        VDEC_PRF("register vdec dev fail\n");
        return 0;
    }

    *pdev = vdec_dev;
    return 1;
}

static void vdec_moniter_task(UINT32 upara1,UINT32 upara2)
{
#define VDEC_MONITER_TIME_OUT               (10)

    struct vdec_device *dev = (struct vdec_device *)upara1;
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    uint8 *msg_data = NULL;
    uint32 msg_type = 0;
    uint32 msg_len = 0;
    uint32 flags = 0;

    int ret = 0;

    VDEC_PRF("vdec enter the moniter task\n");
    //_linux_task_id(__FUNCTION__);

    while(1)
    {
        VDEC_PRF("start to wait new message\n");

        msg_data = rfk_receive_msg(priv->rfk_port);
        if(msg_data == NULL)
        {
            osal_task_sleep(10);
            continue;
        }

        msg_type = msg_data[0];
        msg_len = msg_data[1];
        VDEC_PRF("receive msg type %d len %d\n", msg_type, msg_len);

        MUTEX_LOCK;

        switch(msg_type)
        {
            case MSG_FIRST_SHOWED:
                if(priv->first_show == 0)
                {
                    struct ali_video_out_info_pars pars;

                    VDEC_PRF("get the MSG_FIRST_SHOWED\n");
                    ioctl(priv->handle, ALIVIDEOIO_GET_OUT_INFO, &pars);
                    VDEC_PRF("first show pic w %d h %d\n"
                        , pars.width, pars.height);

                    memcpy((void *)&(priv->out_info), (void *)&pars, sizeof(pars));

                    if(priv->pcb_first_showed)
                        priv->pcb_first_showed(0, 0);

                    priv->first_show = 1;
                }
                break;
            case MSG_MODE_SWITCH_OK:
                if(priv->pcb_mode_switch_ok)
                    priv->pcb_mode_switch_ok(msg_data[2], msg_data[3]);
                break;
            case MSG_BACKWARD_RESTART_GOP:
                if(priv->pcb_backward_restart_gop)
                    priv->pcb_backward_restart_gop(msg_data[2], msg_data[3]);
                break;
            case MSG_FIRST_HEADRE_PARSED:
                if(priv->pcb_first_head_parsed)
                    priv->pcb_first_head_parsed(msg_data[2], msg_data[3]);
                break;
            case MSG_UPDATE_OUT_INFO:
                if(msg_len == sizeof(priv->out_info))
                    memcpy((void *)&priv->out_info, msg_data + 2, msg_len);
                break;
            case MSG_FIRST_I_DECODED:
                if(priv->pcb_first_i_deocded)
                    priv->pcb_first_i_deocded(msg_data[2], msg_data[3]);
                break;
            default:
                break;
        }

        MUTEX_UNLOCK;

        if(!priv->start)
            osal_task_sleep(VDEC_MONITER_TIME_OUT);
    }
}

RET_CODE vdec_open(struct vdec_device *dev)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    enum ali_decv_format format;
    enum ALIFB_OUTPUT_FRAME_PATH out_frm_path;
    void *fb_info = NULL;

    if(priv->open)
    {
        VDEC_PRF("vdec %s is already opened\n", dev->name);
        return RET_SUCCESS;
    }

    priv->handle = open(priv->file_path, O_RDWR);
    if(priv->handle <= 0)
    {
        VDEC_PRF("%s : open file fail %s\n", __FUNCTION__, priv->file_path);
        return RET_FAILURE;
    }

    VDEC_PRF("vdec handle %d %s\n", priv->handle, priv->file_path);

    /* get the free rfk port */
    priv->rfk_port = rfk_get_port();
    if(priv->rfk_port <= 0)
    {
        VDEC_PRF("%s : rfk get port fail\n", __FUNCTION__);
        goto FAIL;
    }

    /* create a task to moniter the status of ali video */
    {
        OSAL_T_CTSK t_ctsk;

        priv->mutex_id = osal_mutex_create();
        if(priv->mutex_id == OSAL_INVALID_ID)
        {
            VDEC_PRF("vdec create mutex fail\n");
            goto FAIL;
        }
        VDEC_PRF("vdec create the mutex %d done\n", priv->mutex_id);

        t_ctsk.stksz = 0x1000;
        t_ctsk.quantum = 15;
        t_ctsk.itskpri = OSAL_PRI_NORMAL;
        t_ctsk.para1 = (UINT32)(dev);
        t_ctsk.para2 = 0;
		t_ctsk.name[0] = 'D';
		t_ctsk.name[1] = 'C';
		t_ctsk.name[2] = 'V';
		t_ctsk.task = (FP)vdec_moniter_task;
		priv->task_id = osal_task_create(&t_ctsk);
        if(priv->task_id == OSAL_INVALID_ID)
        {
            VDEC_PRF("vdec create the moniter task fail\n");
            goto FAIL;
        }
        VDEC_PRF("vdec create the moniter task %d done\n", priv->task_id);

        osal_task_sleep(5);
        ioctl(priv->handle, VDECIO_SET_SOCK_PORT_ID, priv->rfk_port);
    }

    priv->open = 1;
    priv->start = 0;
    VDEC_PRF("open the vdec %s ok\n", dev->name);

    return RET_SUCCESS;

FAIL:
    if(priv->rfk_port > 0)
        rfk_free_port(priv->rfk_port);

    if(priv->task_id != OSAL_INVALID_ID)
        osal_task_delete(priv->task_id);

    if(priv->mutex_id != OSAL_INVALID_ID)
        osal_mutex_delete(priv->mutex_id);

    if(priv->handle != 0)
        close(priv->handle);

    return RET_FAILURE;
}

RET_CODE vdec_close(struct vdec_device *dev)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;

    if(priv->start)
        vdec_stop(dev, 0, 0);

    if(priv->open)
    {
        rfk_free_port(priv->rfk_port);
        osal_task_delete(priv->task_id);
        osal_mutex_delete(priv->mutex_id);
        close(priv->handle);

        priv->open = 0;
        priv->start = 0;
    }

    return RET_SUCCESS;
}

RET_CODE vdec_start(struct vdec_device *dev)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    RET_CODE ret = RET_SUCCESS;

    if(!priv->open)
    {
        VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
        return RET_FAILURE;
    }

    MUTEX_LOCK;

    if(priv->start)
    {
        MUTEX_UNLOCK;

        VDEC_PRF("vdec is already started\n");
        return RET_SUCCESS;
    }

    ENTRY;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_START;
    rpc_pars.arg_num = 0;
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    #else
    ret = ioctl(priv->handle, VDECIO_START, 0);
    #endif

    if(ret < 0)
    {
        ret = RET_FAILURE;
    }

    /* init some info */
    priv->req_buf = NULL;
    priv->req_size = 0;
    priv->update_size = 0;
    priv->first_show = 0;
    memset((void *)&priv->out_info, 0, sizeof(priv->out_info));

    #ifdef USE_OLD_RPC_IO
    ioctl(priv->handle, ALIVIDEOIO_VIDEO_PLAY, 0);
    #endif

    priv->start = 1;

    EXIT;

    MUTEX_UNLOCK;

    return ret;
}

RET_CODE vdec_stop(struct vdec_device *dev, BOOL bclosevp, BOOL bfillblack)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    struct vdec_stop_param stop_param;
    RET_CODE ret = RET_SUCCESS;

    if(!priv->open)
    {
        VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
        return RET_FAILURE;
    }

    MUTEX_LOCK;

    if(!priv->start)
    {
        MUTEX_UNLOCK;

        VDEC_PRF("vdec is already stopped\n");
        return RET_SUCCESS;
    }

    ENTRY;

    if(priv->req_buf)
    {
        free(priv->req_buf);
        priv->req_buf = NULL;
    }

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_STOP;
    rpc_pars.arg_num = 2;
    rpc_pars.arg[0].arg = (void *)&bclosevp;
    rpc_pars.arg[0].arg_size = sizeof(bclosevp);
    rpc_pars.arg[1].arg = (void *)&bfillblack;
    rpc_pars.arg[1].arg_size = sizeof(bfillblack);
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    #else
    stop_param.close_display = bclosevp;
    stop_param.fill_black = bfillblack;
    ret = ioctl(priv->handle, VDECIO_STOP, &stop_param);
    #endif

    if(ret < 0)
    {
        ret = RET_FAILURE;
    }

    #ifdef USE_OLD_RPC_IO
    ioctl(priv->handle, ALIVIDEOIO_VIDEO_STOP, 0);
    #endif

    priv->start = 0;

    EXIT;

    MUTEX_UNLOCK;

    return ret;
}

RET_CODE vdec_vbv_request(void *dev, UINT32 uSizeRequested, void ** ppVData, UINT32 * puSizeGot, struct control_block * ctrl_blk)
{
    struct vdec_device *decv_dev = (struct vdec_device *)dev;
    struct vdec_private *priv = (struct vdec_private *)decv_dev->priv;

    if(!priv->open)
    {
        // VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, decv_dev->name);
        return RET_STA_ERR;
    }

    MUTEX_LOCK;

    if(!priv->start)
    {
        MUTEX_UNLOCK;

        VDEC_PRF("vdec %s is stopped or closed\n", decv_dev->name);
        return RET_STA_ERR;
    }

    ENTRY;

    priv->req_buf = malloc(uSizeRequested);
    if(priv->req_buf == NULL)
    {
        MUTEX_UNLOCK;

        VDEC_PRF("malloc request buffer fail\n");
        return RET_STA_ERR;
    }

    priv->req_size = uSizeRequested;
    priv->update_size = 0;
    *ppVData = priv->req_buf;
    *puSizeGot = priv->req_size;

    EXIT;

    MUTEX_UNLOCK;

    return RET_SUCCESS;

}

void vdec_vbv_update(void *dev, UINT32 uDataSize)
{
    struct vdec_device *decv_dev = (struct vdec_device *)dev;
    struct vdec_private *priv = (struct vdec_private *)decv_dev->priv;
    int write_size = 0;

    if(!priv->open)
    {
        VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, decv_dev->name);
        return;
    }

    MUTEX_LOCK;

    if(!priv->start)
    {
        MUTEX_UNLOCK;

        VDEC_PRF("vdec %s is stopped or closed\n", decv_dev->name);
        return;
    }

    ENTRY;

    while(1)
    {
        write_size = write(priv->handle, priv->req_buf + priv->update_size, uDataSize);
        if(write_size > 0)
        {
            uDataSize -= write_size;
            priv->update_size += write_size;
            if(uDataSize == 0)
                break;
        }
        osal_task_sleep(10);
    }

    if(priv->update_size >= priv->req_size)
    {
        // printf("1 %x ", (int)priv->req_buf);
        free(priv->req_buf);
        priv->req_buf = NULL;
    }

    EXIT;

    MUTEX_UNLOCK;
}

RET_CODE vdec_set_output(struct vdec_device *dev, enum VDecOutputMode eMode, struct VDecPIPInfo *pInitInfo,
                           struct MPSource_CallBack *pMPCallBack, struct PIPSource_CallBack *pPIPCallBack)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    struct vdec_output_param output_param;
    RET_CODE ret = RET_SUCCESS;

    if(!priv->open)
    {
        VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
        return RET_FAILURE;
    }

    MUTEX_LOCK;

    ENTRY;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_SET_OUT;
    rpc_pars.arg_num = 4;//3;
    rpc_pars.arg[0].arg = (void *)&eMode;
    rpc_pars.arg[0].arg_size = sizeof(eMode);
    rpc_pars.arg[1].arg = (void *)pInitInfo;
    rpc_pars.arg[1].arg_size = sizeof(*pInitInfo);
    rpc_pars.arg[2].arg = (void *)pMPCallBack;
    rpc_pars.arg[2].arg_size = sizeof(*pMPCallBack);
    rpc_pars.arg[2].out = 1;
    rpc_pars.arg[3].arg = (void *)pPIPCallBack;
    rpc_pars.arg[3].arg_size = sizeof(*pPIPCallBack);
    rpc_pars.arg[3].out = 1;
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    #else
    if(eMode == MP_MODE)
    {
        output_param.output_mode = VDEC_FULL_VIEW;
    }
    else if(eMode == PREVIEW_MODE)
    {
        output_param.output_mode = VDEC_PREVIEW;
    }
    else if(eMode == SW_PASS_MODE)
    {
        output_param.output_mode = VDEC_SW_PASS;
    }
    else
    {
        output_param.output_mode = VDEC_FULL_VIEW;
    }
    output_param.progressive   = pInitInfo->adv_setting.bprogressive;
    output_param.tv_sys        = pInitInfo->adv_setting.out_sys;
    output_param.smooth_switch = pInitInfo->adv_setting.switch_mode;
    ret = ioctl(priv->handle, VDECIO_SET_OUTPUT_MODE, &output_param);
    memcpy(pMPCallBack, &output_param.mp_callback, sizeof(struct MPSource_CallBack));
    memcpy(pPIPCallBack, &output_param.pip_callback, sizeof(struct PIPSource_CallBack));
    #endif
    
    if(ret < 0)
    {
        ret = RET_FAILURE;
    }

    EXIT;

    MUTEX_UNLOCK;

    return ret;
}

RET_CODE vdec_switch_pip(struct vdec_device *dev, struct Position *pPIPPos)
{
    return RET_FAILURE;
}

RET_CODE vdec_switch_pip_ext(struct vdec_device *dev, struct Rect *pPIPWin)
{
    return RET_FAILURE;
}

RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 uSyncMode, UINT8 uSyncLevel)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    struct vdec_sync_param sync_param;
    RET_CODE ret = RET_SUCCESS;

    if(!priv->open)
    {
        VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
        return RET_FAILURE;
    }

    MUTEX_LOCK;

    ENTRY;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    UINT32 mode = uSyncMode, level = uSyncLevel;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_SYNC_MODE;
    rpc_pars.arg_num = 2;
    rpc_pars.arg[0].arg = (void *)&mode;
    rpc_pars.arg[0].arg_size = sizeof(mode);
    rpc_pars.arg[1].arg = (void *)&level;
    rpc_pars.arg[1].arg_size = sizeof(level);
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    #else
    sync_param.sync_mode = uSyncMode;
    ret = ioctl(priv->handle, VDECIO_SET_SYNC_MODE, &sync_param);
    #endif
    
    if(ret < 0)
    {
        ret = RET_FAILURE;
    }

    EXIT;

    MUTEX_UNLOCK;

    return ret;
}

RET_CODE vdec_extrawin_store_last_pic(struct vdec_device *dev, struct Rect *pPIPWin)
{
    return RET_FAILURE;
}

RET_CODE vdec_profile_level(struct vdec_device *dev, UINT8 uProfileLevel,VDEC_BEYOND_LEVEL cb_beyond_level)
{
    return RET_FAILURE;
}

RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param)
{
    if(!m_is_attatch)
    {
        return RET_FAILURE;
    }
    
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    RET_CODE ret = RET_SUCCESS;

    if(!priv->open)
    {
        VDEC_PRF("%s : don't open dev %s \n", __FUNCTION__, dev->name);
        return RET_FAILURE;
    }

    MUTEX_LOCK;

    switch(io_code)
    {
        case VDEC_IO_GET_STATUS:
        {
            #ifdef USE_OLD_RPC_IO
            struct ali_video_rpc_pars rpc_pars;

            memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
            rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
            rpc_pars.arg_num = 2;
            rpc_pars.arg[0].arg = (void *)&io_code;
            rpc_pars.arg[0].arg_size = sizeof(io_code);
            rpc_pars.arg[1].arg = (void *)param;
            rpc_pars.arg[1].arg_size = sizeof(struct VDec_StatusInfo);
            rpc_pars.arg[1].out = 1;
            ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
            #else
            struct vdec_status_info vdec_stat;
            struct VDec_StatusInfo *pstatus = (struct VDec_StatusInfo *)param;

            memset(&vdec_stat, 0, sizeof(vdec_stat));
            ret = ioctl(priv->handle, VDECIO_GET_STATUS, &vdec_stat);

            pstatus->uCurStatus      = vdec_stat.status;
            pstatus->bFirstHeaderGot = vdec_stat.first_header_parsed;
            pstatus->uFirstPicShowed = vdec_stat.first_pic_showed;
            pstatus->pic_width       = vdec_stat.pic_width;
            pstatus->pic_height      = vdec_stat.pic_height;
            pstatus->aspect_ratio    = vdec_stat.aspect_ratio;
            pstatus->display_idx     = vdec_stat.frames_displayed;
            pstatus->display_frm     = vdec_stat.show_frame;
            pstatus->top_cnt         = vdec_stat.queue_count;
            pstatus->vbv_size        = vdec_stat.buffer_size;
            pstatus->valid_size      = vdec_stat.buffer_used;
            pstatus->frame_rate      = vdec_stat.frame_rate;
            pstatus->hw_dec_error    = vdec_stat.hw_dec_error;
            pstatus->is_support      = vdec_stat.is_support;
            pstatus->play_direction  = vdec_stat.playback_param.direction;
            pstatus->play_speed      = vdec_stat.playback_param.rate;
            pstatus->api_play_direction = vdec_stat.api_playback_param.direction;
            pstatus->api_play_speed  = vdec_stat.api_playback_param.rate;

            if(vdec_stat.output_mode == VDEC_FULL_VIEW)
            {
                pstatus->output_mode = MP_MODE;
            }
            else if(vdec_stat.output_mode == VDEC_PREVIEW)
            {
                pstatus->output_mode = PREVIEW_MODE;
            }
            #endif
            
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }

            break;
        }

        case VDEC_IO_GET_MODE:
        {
            struct ali_video_out_info_pars *out_info = &priv->out_info;
            int frame_rate;
            int source_width;

            if(!(out_info->flag & ALI_VIDEO_OUT_INFO_FLAG_FIRST_SHOWED))
            {
                ret = RET_FAILURE;
                break;
            }

            frame_rate = priv->out_info.frame_rate;
            source_width = priv->out_info.width;

            switch((frame_rate + 999) / 1000)
            {
                case 25:
                case 50:
                    if(source_width <= 720)
                        *(enum TVSystem *)(param) =PAL;
                    else if(source_width <= 1280)
                        *(enum TVSystem *)(param) =LINE_720_25;
                    else
                            *(enum TVSystem *)(param) = LINE_1080_25;
                    break;
                case 24:
                case 30:
                case 60:
                    if(source_width<=720)
                        *(enum TVSystem *)(param) =NTSC;
                    else if(source_width<=1280)
                        *(enum TVSystem *)(param) =LINE_720_30;
                    else
                        *(enum TVSystem *)(param) = LINE_1080_30;
                    break;
                default:
                    ret = RET_FAILURE;
                    break;
            }

            break;
        }

        case VDEC_IO_GET_OUTPUT_FORMAT:
        {
            struct ali_video_out_info_pars *out_info = &priv->out_info;


            if(!(out_info->flag & ALI_VIDEO_OUT_INFO_FLAG_FIRST_SHOWED))
            {
                ret = RET_FAILURE;
                break;
            }

            *(BOOL *)param = out_info->progressive;

            break;
        }

        case VDEC_IO_FILL_FRM:
        {
            #ifdef USE_OLD_RPC_IO
            struct ali_video_rpc_pars rpc_pars;

            memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
            rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
            rpc_pars.arg_num = 2;
            rpc_pars.arg[0].arg = (void *)&io_code;
            rpc_pars.arg[0].arg_size = sizeof(io_code);
            rpc_pars.arg[1].arg = (void *)param;
            rpc_pars.arg[1].arg_size = sizeof(struct YCbCrColor);
            ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
            #else
            struct vdec_yuv_color yuv_color;
            struct YCbCrColor *pcolor = (struct YCbCrColor *)param;

            yuv_color.y = pcolor->uY;
            yuv_color.u = pcolor->uCb;
            yuv_color.v = pcolor->uCb;
            ret = ioctl(priv->handle, VDECIO_FILL_FRAME, &yuv_color);
            #endif
            
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }

            break;
        }

        case VDEC_IO_REG_CALLBACK:
        {
            struct vdec_io_reg_callback_para *ppara = (struct vdec_io_reg_callback_para *)(param);

            switch(ppara->eCBType)
            {
                case VDEC_CB_FIRST_SHOWED:
                    priv->pcb_first_showed = ppara->pCB;
                    break;
                case VDEC_CB_MODE_SWITCH_OK:
                    priv->pcb_mode_switch_ok = ppara->pCB;
                    break;
                case VDEC_CB_BACKWARD_RESTART_GOP: // used for advance play
                    priv->pcb_backward_restart_gop = ppara->pCB;
                    break;
                case VDEC_CB_FIRST_HEAD_PARSED:
                    priv->pcb_first_head_parsed = ppara->pCB;
                    break;
                case VDEC_CB_FIRST_I_DECODED:
                    priv->pcb_first_i_deocded = ppara->pCB;
                    break;
                default:
                    break;
            }

            break;
        }

        case VDEC_IO_COLORBAR:
            ret = ioctl(priv->handle, VDECIO_DRAW_COLOR_BAR, 0);
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }
            break;
            
        case VDEC_SET_DMA_CHANNEL:
            ret = ioctl(priv->handle, VDECIO_SET_DMA_CHANNEL, (UINT8)param);
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }
            break;
            
        case VDEC_DTVCC_PARSING_ENABLE:
            ret = ioctl(priv->handle, VDECIO_DTVCC_PARSING_ENABLE, (INT32)param);
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }
            break;

        case VDEC_IO_SET_SYNC_DELAY:
            ret = ioctl(priv->handle, VDECIO_SET_SYNC_DELAY, param);
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }
            break;
            
        case VDEC_IO_SAR_ENABLE:
            ret = ioctl(priv->handle, VDECIO_SAR_ENABLE, param);
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }
            break;
            
        case VDEC_IO_FIRST_I_FREERUN:
            ret = ioctl(priv->handle, VDECIO_FIRST_I_FREERUN, param);
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }
            break;

        case VDEC_VBV_BUFFER_OVERFLOW_RESET:
            ret = ioctl(priv->handle, VDECIO_VBV_BUFFER_OVERFLOW_RESET, param);
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }
            break;
            
        case VDEC_IO_CONTINUE_ON_ERROR:
        {
            #ifdef USE_OLD_RPC_IO
            struct ali_video_rpc_pars rpc_pars;

            memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
            rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
            rpc_pars.arg_num = 2;
            rpc_pars.arg[0].arg = (void *)&io_code;
            rpc_pars.arg[0].arg_size = sizeof(io_code);
            rpc_pars.arg[1].arg = (void *)&param;
            rpc_pars.arg[1].arg_size = 4;
            ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
            #else
            ret = ioctl(priv->handle, VDECIO_CONTINUE_ON_ERROR, param);
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }
            #endif
            
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }

            break;
        }

        case VDEC_IO_SET_OUTPUT_RECT:
        {
            #ifdef USE_OLD_RPC_IO
            struct ali_video_rpc_pars rpc_pars;

            memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
            rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
            rpc_pars.arg_num = 2;
            rpc_pars.arg[0].arg = (void *)&io_code;
            rpc_pars.arg[0].arg_size = sizeof(io_code);
            rpc_pars.arg[1].arg = (void *)param;
            rpc_pars.arg[1].arg_size = sizeof(struct VDecPIPInfo);
            ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
            #else
            struct vdec_display_rect display_rect;
            struct VDecPIPInfo *ppip_info = (struct VDecPIPInfo *)param;

            display_rect.src_x = ppip_info->src_rect.uStartX;
            display_rect.src_y = ppip_info->src_rect.uStartY;
            display_rect.src_w = ppip_info->src_rect.uWidth;
            display_rect.src_h = ppip_info->src_rect.uHeight;
            display_rect.dst_x = ppip_info->dst_rect.uStartX;
            display_rect.dst_y = ppip_info->dst_rect.uStartY;
            display_rect.dst_w = ppip_info->dst_rect.uWidth;
            display_rect.dst_h = ppip_info->dst_rect.uHeight;
            ret = ioctl(priv->handle, VDECIO_SET_OUTPUT_RECT, &display_rect);
            #endif
            
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }

            break;
        }

        case VDEC_IO_CAPTURE_DISPLAYING_FRAME:
        {
            #ifdef USE_OLD_RPC_IO
            struct ali_video_rpc_pars rpc_pars;

            memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
            rpc_pars.API_ID = RPC_VIDEO_IO_CONTROL;
            rpc_pars.arg_num = 2;
            rpc_pars.arg[0].arg = (void *)&io_code;
            rpc_pars.arg[0].arg_size = sizeof(io_code);
            rpc_pars.arg[1].arg = (void *)param;
            rpc_pars.arg[1].arg_size = sizeof(struct vdec_picture);
            rpc_pars.arg[1].out = 1;
            ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
            #else
            ret = ioctl(priv->handle, VDECIO_CAPTURE_DISPLAYING_FRAME, param);
            #endif
            
            if(ret < 0)
            {
                ret = RET_FAILURE;
            }

            break;
        }

        case VDEC_IO_SET_MODULE_INFO:
            ret = ioctl(priv->handle, ALIVIDEOIO_SET_MODULE_INFO, param);
            break;

        default:
            ret = RET_FAILURE;

            break;
    }

    MUTEX_UNLOCK;

    return ret;
}

RET_CODE vdec_playmode(struct vdec_device *dev, enum VDecDirection direction, enum VDecSpeed speed)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    struct vdec_playback_param playback_param;
    RET_CODE ret = RET_SUCCESS;

    MUTEX_LOCK;

    ENTRY;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_PLAY_MODE;
    rpc_pars.arg_num = 2;
    rpc_pars.arg[0].arg = (void *)&direction;
    rpc_pars.arg[0].arg_size = sizeof(direction);
    rpc_pars.arg[1].arg = (void *)&speed;
    rpc_pars.arg[1].arg_size = sizeof(speed);
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    #else
    playback_param.direction = direction;
    playback_param.rate = speed;
    ret = ioctl(priv->handle, VDECIO_SET_PLAY_MODE, &playback_param);
    #endif
    
    if(ret < 0)
    {
        ret = RET_FAILURE;
    }

    EXIT;

    MUTEX_UNLOCK;

    return ret;
}

RET_CODE vdec_dvr_set_param(struct vdec_device *dev, struct VDec_DvrConfigParam param)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    struct vdec_pvr_param pvr_param;
    int ret = RET_SUCCESS;

    MUTEX_LOCK;

    ENTRY;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_DVR_SET_PAR;
    rpc_pars.arg_num = 1;
    rpc_pars.arg[0].arg = (void *)&param;
    rpc_pars.arg[0].arg_size = sizeof(param);
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    #else
    pvr_param.is_scrambled = param.is_scrambled;
    ret = ioctl(priv->handle, VDECIO_SET_PVR_PARAM, &pvr_param);
    #endif
    
    if(ret < 0)
    {
        ret = RET_FAILURE;
    }

    EXIT;

    MUTEX_UNLOCK;

    return ret;
}

RET_CODE vdec_dvr_pause(struct vdec_device *dev)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    RET_CODE ret = RET_SUCCESS;

    MUTEX_LOCK;

    ENTRY;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_DVR_PAUSE;
    rpc_pars.arg_num = 0;
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    #else
    ret = ioctl(priv->handle, VDECIO_PAUSE, 0);
    #endif
    
    if(ret < 0)
    {
        ret = RET_FAILURE;
    }

    EXIT;

    MUTEX_UNLOCK;

    return ret;
}

RET_CODE vdec_dvr_resume(struct vdec_device *dev)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    RET_CODE ret = RET_SUCCESS;

    MUTEX_LOCK;

    ENTRY;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_DVR_RESUME;
    rpc_pars.arg_num = 0;
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    #else
    ret = ioctl(priv->handle, VDECIO_RESUME, 0);
    #endif
    
    if(ret < 0)
    {
        ret = RET_FAILURE;
    }

    EXIT;

    MUTEX_UNLOCK;

    return ret;
}

RET_CODE vdec_step(struct vdec_device *dev)
{
    struct vdec_private *priv = (struct vdec_private *)dev->priv;
    RET_CODE ret = RET_SUCCESS;

    MUTEX_LOCK;

    ENTRY;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_DVR_STEP;
    rpc_pars.arg_num = 0;
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    #else
    ret = ioctl(priv->handle, VDECIO_STEP, 0);
    #endif
    
    if(ret < 0)
    {
        ret = RET_FAILURE;
    }

    EXIT;

    MUTEX_UNLOCK;

    return ret;
}

BOOL is_cur_decoder_avc(void)
{
    struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;
    enum vdec_type type = VDEC_MPEG;
    RET_CODE ret = RET_SUCCESS;

    MUTEX_LOCK;

    ENTRY;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_IS_AVC;
    rpc_pars.arg_num = 0;
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);

    cur_decv_type = (ret == TRUE) ? 1 : 0;
    #else
    ret = ioctl(priv->handle, VDECIO_GET_CUR_DECODER, &type);
    if(ret < 0)
    {
        ret = RET_FAILURE;
    }
    
    cur_decv_type = (type == VDEC_AVC) ? 1 : 0;
    #endif

    EXIT;

    MUTEX_UNLOCK;

    return (cur_decv_type == 1 ? TRUE : FALSE);
}

/*
	select : 0 : mpg decoder; 1 : H264 decoder; 2 : avs decoder
	in_preview : 0 : main pic mode; 1 : preview mode
*/
void video_decoder_select(enum video_decoder_type select, BOOL in_preview)
{
	struct vdec_private *priv = &m_vdec_priv_data;
	struct ali_video_rpc_pars rpc_pars;			
	RET_CODE ret = RET_SUCCESS;

#ifdef ADR_IPC_ENABLE
	if(m_vdec_attached == 0)
		HLD_vdec_attach();
#endif

	if(!priv->open)
	{
		VDEC_PRF("%s : don't open\n", __FUNCTION__);
		return;
	}
	
	MUTEX_LOCK;
	
	ENTRY;
	
	VDEC_PRF("video_decoder_select in select %d in preview %d curret select %d current preview %d \n", select, in_preview
		, cur_decv_type, cur_decv_mode);

	// if((select != cur_decv_type) || (in_preview != cur_decv_mode))
	{
		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
		rpc_pars.API_ID = RPC_VIDEO_DECODER_SELECT_NEW;
		rpc_pars.arg_num = 2;	
		rpc_pars.arg[0].arg = (void *)&select;
		rpc_pars.arg[0].arg_size = sizeof(select);			
		rpc_pars.arg[1].arg = (void *)&in_preview;
		rpc_pars.arg[1].arg_size = sizeof(in_preview);	
		ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
		if(ret < 0)
			ret = RET_FAILURE;

		cur_decv_mode = in_preview;
		if(select == MPEG2_DECODER)
			cur_decv_type = 0;
		else if(select == H264_DECODER)
			cur_decv_type = 1;
		else if(select == AVS_DECODER)
			cur_decv_type = 2;

		ioctl(priv->handle, ALIVIDEOIO_VIDEO_STOP, 0);	

		priv->start = 0;
	}
	
	EXIT;
	MUTEX_UNLOCK;
}

struct vdec_device * get_selected_decoder(void)
{
    struct vdec_device *dev = NULL;
    struct vdec_private *priv = (struct vdec_private *)&m_vdec_priv_data;
    enum vdec_type type = VDEC_MPEG;
    RET_CODE ret = RET_SUCCESS;

    MUTEX_LOCK;

    #ifdef USE_OLD_RPC_IO
    struct ali_video_rpc_pars rpc_pars;
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_IS_AVC;
    rpc_pars.arg_num = 0;
    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    cur_decv_type = (ret == TRUE) ? 1 : 0;
    #else
    ret = ioctl(priv->handle, VDECIO_GET_CUR_DECODER, &type);
    if(ret < 0)
    {
        ret = RET_FAILURE;
    }
    
    cur_decv_type = (type == VDEC_AVC) ? 1 : 0;
    #endif

    MUTEX_UNLOCK;

    dev = (cur_decv_type == 0) ? m_vdec_dev[VDEC_MPG_DEV] : m_vdec_dev[VDEC_AVC_DEV];

    return dev;
}

/* it is used only for AVC decoder. keep it NULL now */
void set_avc_output_mode_check_cb(VDecCBFunc pCB)
{
    do{}while(0);
}

/*
    select : 0 : mpg decoder; 1 : H264 decoder
    in_preview : 0 : main pic mode; 1 : preview mode
*/
void h264_decoder_select(int select, BOOL in_preview)
{
    struct vdec_private *priv = &m_vdec_priv_data;
    struct vdec_codec_param codec_param;
    RET_CODE ret = RET_SUCCESS;

    MUTEX_LOCK;

    ENTRY;

    if((select != cur_decv_type) || (in_preview != cur_decv_mode))
    {
        #ifdef USE_OLD_RPC_IO
        struct ali_video_rpc_pars rpc_pars;
        memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
        rpc_pars.API_ID = RPC_VIDEO_SELECT_DEC;
        rpc_pars.arg_num = 2;
        rpc_pars.arg[0].arg = (void *)&select;
        rpc_pars.arg[0].arg_size = sizeof(select);
        rpc_pars.arg[1].arg = (void *)&in_preview;
        rpc_pars.arg[1].arg_size = sizeof(in_preview);
        ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
        #else
        codec_param.type = (select == 0) ? VDEC_MPEG : VDEC_AVC;
        codec_param.preview = in_preview;
        ret = ioctl(priv->handle, VDECIO_SELECT_DECODER, &codec_param);
        #endif
        
        if(ret < 0)
        {
            ret = RET_FAILURE;
        }

        cur_decv_type = (select == 0) ? 0 : 1;
        cur_decv_mode = in_preview;
    }

    EXIT;

    MUTEX_UNLOCK;

    return;
}

RET_CODE vdec_decore_ioctl(void *phandle, int cmd, void *param1, void *param2)
{
    struct vdec_private *priv = &m_vdec_priv_data;
    struct ali_video_rpc_pars rpc_pars;
    RET_CODE ret = RET_SUCCESS;

    MUTEX_LOCK;

    VDEC_PRF("In ======%s====== 0x%x\n", __FUNCTION__,(unsigned int)priv);

    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));

    rpc_pars.API_ID = RPC_VIDEO_DECORE_IOCTL;
    rpc_pars.arg_num = 4;

    switch(cmd)
    {
        case VDEC_CMD_INIT:
            rpc_pars.arg[0].arg = (void *)phandle;
            rpc_pars.arg[0].arg_size = sizeof(phandle);
            rpc_pars.arg[1].arg = (void *)&cmd;
            rpc_pars.arg[1].arg_size = sizeof(cmd);
            rpc_pars.arg[2].arg = (void *)param1;
            rpc_pars.arg[2].arg_size = sizeof(param1);
            rpc_pars.arg[3].arg = (void *)param2;
            rpc_pars.arg[3].arg_size = sizeof(VdecInit);
            break;
        case VDEC_CMD_EXTRA_DADA:
        case VDEC_CMD_RELEASE:
        case VDEC_CMD_SW_RESET:
        case VDEC_CMD_HW_RESET:
        case VDEC_CMD_PAUSE_DECODE:
        case VDEC_CFG_VIDEO_SBM_BUF:
        case VDEC_CFG_DISPLAY_SBM_BUF:
        case VDEC_CFG_SYNC_MODE:
        case VDEC_CFG_SYNC_THRESHOLD:
            rpc_pars.arg[0].arg = (void *)phandle;
            rpc_pars.arg[0].arg_size = sizeof(phandle);
            rpc_pars.arg[1].arg = (void *)&cmd;
            rpc_pars.arg[1].arg_size = sizeof(cmd);
            rpc_pars.arg[2].arg = (void *)param1;
            rpc_pars.arg[2].arg_size = sizeof(param1);
            rpc_pars.arg[3].arg = (void *)param2;
            rpc_pars.arg[3].arg_size = sizeof(param2);
            break;
        case VDEC_CFG_DISPLAY_RECT:
            rpc_pars.arg[0].arg = (void *)phandle;
            rpc_pars.arg[0].arg_size = sizeof(phandle);
            rpc_pars.arg[1].arg = (void *)&cmd;
            rpc_pars.arg[1].arg_size = sizeof(cmd);
            rpc_pars.arg[2].arg = (void *)param1;
            rpc_pars.arg[2].arg_size = sizeof(struct Video_Rect);
            rpc_pars.arg[3].arg = (void *)param2;
            rpc_pars.arg[3].arg_size = sizeof(struct Video_Rect);
            break;
        case VDEC_CMD_GET_STATUS:
            rpc_pars.arg[0].arg = (void *)phandle;
            rpc_pars.arg[0].arg_size = sizeof(phandle);
            rpc_pars.arg[1].arg = (void *)&cmd;
            rpc_pars.arg[1].arg_size = sizeof(cmd);
            rpc_pars.arg[2].arg = (void *)param1;
            rpc_pars.arg[2].arg_size = sizeof(struct vdec_decore_status);
            rpc_pars.arg[2].out = 1;
            rpc_pars.arg[3].arg = (void *)param2;
            rpc_pars.arg[3].arg_size = sizeof(struct vdec_decore_status);
            rpc_pars.arg[3].out = 1;
            break;
        default:
            return RET_FAILURE;
    }

    ret = ioctl(priv->handle, ALIVIDEOIO_RPC_OPERATION, &rpc_pars);
    if(ret < 0)
    {
        printf("decore ioctl fail: %d %d\n", priv->handle, ret);
        ret = RET_FAILURE;
    }

    VDEC_PRF("Out ======%s======\n\n", __FUNCTION__);

    MUTEX_UNLOCK;

    return ret;
}

static void HLD_vdec_attach_avc(void)
{
    if(m_vdec_dev[VDEC_AVC_DEV])
    {
        VDEC_PRF("avc decoder is already attached\n");
        return;
    }

    if(!vdec_attach(video_path, &m_vdec_dev[VDEC_AVC_DEV], 0))
    {
        VDEC_PRF("attach avc decoder fail\n");
        return;
    }

    m_vdec_priv[VDEC_AVC_DEV] = (struct vdec_private *)m_vdec_dev[VDEC_AVC_DEV]->priv;
    VDEC_PRF("attach hld avc decoder ok\n");
}

static void HLD_vdec_attach_mpg(void)
{
    if(m_vdec_dev[VDEC_MPG_DEV])
    {
        VDEC_PRF("mpg decoder is already attached\n");
        return;
    }

    if(!vdec_attach(video_path, &m_vdec_dev[VDEC_MPG_DEV], 1))
    {
        VDEC_PRF("attach mpg decoder fail\n");
        return;
    }

    m_vdec_priv[VDEC_MPG_DEV] = (struct vdec_private *)m_vdec_dev[VDEC_MPG_DEV]->priv;
    VDEC_PRF("attach hld mpg decoder ok\n");
}

void HLD_vdec_attach(void)
{
    HLD_vdec_attach_mpg();
    HLD_vdec_attach_avc();

    vdec_open(m_vdec_dev[0]);
    m_is_attatch = 1;
}

void HLD_vdec_detach(void)
{
    int i = 0;

    vdec_close(m_vdec_dev[0]);
    for(i = 0;i < VDEC_MAX_DEV_NUM;i++)
    {
        if(m_vdec_dev[i])
        {
            dev_free(m_vdec_dev[i]);
            m_vdec_dev[i] = NULL;
        }
    }
    m_is_attatch = 0;
}

