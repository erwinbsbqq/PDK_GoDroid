/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_rpc_pe_image.c
 *  (I)
 *  Description: pe image Remote Call Process API
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
typedef struct Imagedec_frm_t
{
	UINT8 *frm_y_addr;
	UINT32 frm_y_len;
	UINT8 *frm_c_addr;
	UINT32 frm_c_len;
	UINT32 busy;
}Imagedec_frm,*pImagedec_frm;

int image_engine_init(struct pe_image_cfg *pe_image_cfg)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Pe_image_cfg_rpc, sizeof(Pe_image_cfg_rpc), pe_image_cfg);

    ret = RpcCallCompletion(RPC_image_engine_init, &p1, NULL);
    return ret;
}

int image_engine_cleanup(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_image_engine_cleanup, NULL);
    return ret;

}

int image_engine_rotate(unsigned char rotate_angle)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &rotate_angle);

    ret = RpcCallCompletion(RPC_image_engine_rotate, &p1, NULL);
    return ret;
}

int image_engine_decode(char *filename, struct image_config *cfg)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_OPAQUE, strlen(filename), filename);

    struct image_config_rpc image_config_rpc_struct;
    struct image_config_rpc *temp1 = &image_config_rpc_struct;

    temp1->file_name = cfg->file_name;
    
    temp1->decode_mode = cfg->decode_mode;
    temp1->show_mode = cfg->show_mode;
    temp1->vpo_mode = cfg->vpo_mode;
    temp1->rotate = cfg->rotate;
    
    temp1->src_left = cfg->src_left;
    temp1->src_top = cfg->src_top;
    temp1->src_width = cfg->src_width;
    temp1->src_height = cfg->src_height;
    
    temp1->dest_left = cfg->dest_left;
    temp1->dest_top = cfg->dest_top;
    temp1->dest_width = cfg->dest_width;
    temp1->dest_height = cfg->dest_height;
    
    temp1->mp_cb = cfg->mp_cb;

    struct image_slideshow_effect_rpc temp2_struct;
    struct image_slideshow_effect_rpc *temp2 = &temp2_struct;

    temp2->mode = cfg->effect->mode;

    memcpy(&temp2->shuttles_param, &cfg->effect->mode_param.shuttles_param, sizeof(Imagedec_show_shutters_rpc));
    memcpy(&temp2->brush_param, &cfg->effect->mode_param.brush_param, sizeof(Imagedec_show_brush_rpc));
    memcpy(&temp2->slide_param, &cfg->effect->mode_param.slide_param, sizeof(Imagedec_show_slide_rpc));

    memcpy(&temp2->random_param, &cfg->effect->mode_param.random_param, sizeof(Imagedec_show_show_random_rpc));
    memcpy(&temp2->fade_param, &cfg->effect->mode_param.fade_param, sizeof(Imagedec_show_fade_rpc));

    
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Image_config_rpc, sizeof(Image_config_rpc), temp1);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_Image_slideshow_effect_rpc, sizeof(Image_slideshow_effect_rpc), temp2);

    ret = RpcCallCompletion(RPC_image_engine_decode, &p1, &p2, &p3, NULL);
    
    return ret;
}

int image_engine_show(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_image_engine_show, NULL);
    return ret;
}

int image_engine_abort(void)
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_image_engine_abort, NULL);
    return ret;
}

int image_engine_zoom(struct Rect *dstRect, struct Rect *srcRect)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Rect_rpc, sizeof(Rect_rpc), dstRect);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Rect_rpc, sizeof(Rect_rpc), srcRect);

    ret = RpcCallCompletion(RPC_image_engine_zoom, &p1, &p2, NULL);
    return ret;
}

int image_engine_get_info(char *filename, struct image_info *info)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_OPAQUE, strlen(filename), filename);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Image_info_pe_rpc, sizeof(Image_info_pe_rpc), info);

    ret = RpcCallCompletion(RPC_image_engine_get_info, &p1, &p2, NULL);
    return ret;
}

BOOL imagedec_ioctl(unsigned long id,UINT32 cmd, UINT32 para)
{
    BOOL ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_ULONG, sizeof(Ulong), &id);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &cmd);    
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_UINT32, sizeof(Uint32), &para);


    switch(cmd)
	{
		case IMAGEDEC_IO_CMD_CFG_DEO_FRM:
                     RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_Imagedec_frm_rpc, sizeof(Imagedec_frm_rpc), para);
			break;		
		case IMAGEDEC_IO_CMD_OPEN_HW_ACCE:
		case IMAGEDEC_IO_CMD_FILL_LOGO:
		case IMAGEDEC_IO_CMD_CLEAN_FRM:
		case IMAGEDEC_IO_CMD_NEED_LOGO:
		default:
			break;
	}

    ret = RpcCallCompletion(RPC_imagedec_ioctl, &p1, &p2, &p3, NULL);
    return ret;
}

