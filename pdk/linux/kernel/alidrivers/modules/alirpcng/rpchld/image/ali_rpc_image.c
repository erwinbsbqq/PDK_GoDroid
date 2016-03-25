/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: ali_image_rpc.c
 *  (I)
 *  Description: rpc opeartion
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2012.04.09			Blady		Create
 ****************************************************************************/
#include <linux/version.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/div64.h>
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
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_image.h>

#include <ali_image_common.h>

//#include "../ali_rpc.h"
#include <ali_rpcng.h>
#define CACHE_ADDR(addr)    ((unsigned int)addr&0x8fffffff)

#define NONCACHE_ADDR(addr) ((unsigned int)addr|0xa0000000)

int image_init(struct image_init_config *cfg)
{
    Int32 ret;

    struct image_init_config_rpc image_init_config_rpc_struct;
    struct image_init_config_rpc *temp = &image_init_config_rpc_struct;

    temp->frm_y_addr = cfg->frm_y_addr;
    temp->frm_y_size = cfg->frm_y_size;
    temp->frm_c_addr = cfg->frm_c_addr;
    temp->frm_c_size = cfg->frm_c_size;

    temp->frm2_y_addr = cfg->frm2_y_addr;
    temp->frm2_y_size = cfg->frm2_y_size;
    temp->frm2_c_addr = cfg->frm2_c_addr;
    temp->frm2_c_size = cfg->frm2_c_size;

    temp->frm_mb_type = cfg->frm_mb_type;
    temp->frm3_y_addr = cfg->frm3_y_addr;
    temp->frm3_y_size = cfg->frm3_y_size;
    temp->frm3_c_addr = cfg->frm3_c_addr;

    temp->frm3_c_size = cfg->frm3_c_size;
    temp->frm4_y_addr = cfg->frm4_y_addr;
    temp->frm4_y_size = cfg->frm4_y_size;
    temp->frm4_c_addr = cfg->frm4_c_addr;

    temp->frm4_c_size = cfg->frm4_c_size;
    temp->decoder_buf = cfg->decoder_buf;
    temp->decoder_buf_len = cfg->decoder_buf_len;
    temp->img_fmt = cfg->img_fmt;

    temp->pkt_sbm_idx = cfg->pkt_sbm_idx;
    temp->info_sbm_idx = cfg->info_sbm_idx;
    temp->info_end_idx = cfg->info_end_idx;


    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Image_init_config_rpc, sizeof(Image_init_config_rpc), temp);

    ret = RpcCallCompletion(RPC_image_init, &p1, NULL);
    return ret;
}

int image_release()
{
    Int32 ret;

    ret = RpcCallCompletion(RPC_image_release, NULL);
    return ret;
}

int image_stop()
{

    Int32 ret;

    ret = RpcCallCompletion(RPC_image_stop, NULL);
    return ret;
}

int image_decode(int sw_hw, struct image_engine_config *cfg)
{
    Int32 ret;

    struct image_engine_config_rpc image_engine_config_rpc_struct;
    struct image_engine_config_rpc *temp1 = &image_engine_config_rpc_struct;

    struct image_slideshow_effect_rpc temp2_struct;
    struct image_slideshow_effect_rpc *temp2 = &temp2_struct;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, 4, &sw_hw);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Image_engine_config_rpc, sizeof(Image_engine_config_rpc), temp1);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_Image_slideshow_effect_rpc, sizeof(Image_slideshow_effect_rpc), temp2);

    if(cfg == NULL)
    {
        RPC_PARAM_UPDATE(p2, PARAM_IN, PARAM_UINT32, 0, NULL);
    }
    else
    {       
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
        
        temp1->img_fmt= cfg->img_fmt;

        if(cfg->effect == NULL)
	    {
	        RPC_PARAM_UPDATE(p3, PARAM_IN, PARAM_UINT32, 0, NULL);
	    }
        else
        {
            temp2->mode = cfg->effect->mode;

            memcpy(&temp2->shuttles_param, &cfg->effect->mode_param.shuttles_param, sizeof(Imagedec_show_shutters_rpc));
            memcpy(&temp2->brush_param, &cfg->effect->mode_param.brush_param, sizeof(Imagedec_show_brush_rpc));
            memcpy(&temp2->slide_param, &cfg->effect->mode_param.slide_param, sizeof(Imagedec_show_slide_rpc));

            memcpy(&temp2->random_param, &cfg->effect->mode_param.random_param, sizeof(Imagedec_show_show_random_rpc));
            memcpy(&temp2->fade_param, &cfg->effect->mode_param.fade_param, sizeof(Imagedec_show_fade_rpc));
        }
    }

    ret = RpcCallCompletion(RPC_image_decode, &p1, &p2, &p3, NULL);
    
    return ret;
}

int image_rotate(unsigned char rotate_angle)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_UCHAR, sizeof(Uchar), &rotate_angle);

    ret = RpcCallCompletion(RPC_image_rotate, &p1, NULL);
    return ret;
}

int image_zoom(struct Rect *dstRect, struct Rect *srcRect)
{
    Int32 ret;

    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_Rect_rpc, sizeof(Rect_rpc), dstRect);
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Rect_rpc, sizeof(Rect_rpc), srcRect);

    ret = RpcCallCompletion(RPC_image_zoom, &p1, &p2, NULL);
    return ret;
}

int image_display(int sw_hw, struct image_display_t *cfg)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, 4, &sw_hw);

    struct image_display_rpc image_display_rpc_struct;
    struct image_display_rpc *temp1 = &image_display_rpc_struct;
    
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
    
    temp1->img_fmt= cfg->img_fmt;

    temp1->y_addr = cfg->y_addr;
    temp1->y_len = cfg->y_len;
    temp1->c_addr = cfg->c_addr;
    temp1->c_len = cfg->c_len;
    
    temp1->width = cfg->width;
    temp1->height = cfg->height;
    
    temp1->sample_format = cfg->sample_format;



    struct image_slideshow_effect_rpc temp2_struct;
    struct image_slideshow_effect_rpc *temp2 = &temp2_struct;



    temp2->mode = cfg->effect->mode;

    memcpy(&temp2->shuttles_param, &cfg->effect->mode_param.shuttles_param, sizeof(Imagedec_show_shutters_rpc));
    memcpy(&temp2->brush_param, &cfg->effect->mode_param.brush_param, sizeof(Imagedec_show_brush_rpc));
    memcpy(&temp2->slide_param, &cfg->effect->mode_param.slide_param, sizeof(Imagedec_show_slide_rpc));

    memcpy(&temp2->random_param, &cfg->effect->mode_param.random_param, sizeof(Imagedec_show_show_random_rpc));
    memcpy(&temp2->fade_param, &cfg->effect->mode_param.fade_param, sizeof(Imagedec_show_fade_rpc));

    
    RPC_PARAM_CREATE(p2, PARAM_IN, PARAM_Image_engine_config_rpc, sizeof(Image_engine_config_rpc), temp1);
    RPC_PARAM_CREATE(p3, PARAM_IN, PARAM_Image_slideshow_effect_rpc, sizeof(Image_slideshow_effect_rpc), temp2);

    ret = RpcCallCompletion(RPC_image_display, &p1, &p2, &p3, NULL);
    
    return ret;
}

int image_get_hw_addr(int flag_nused, struct image_hw_info *hw_info)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, 4, &flag_nused);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Image_hw_info_rpc, sizeof(Image_hw_info_rpc), hw_info);

    ret = RpcCallCompletion(RPC_image_get_hw_addr, &p1, &p2, NULL);
    return ret;
}

int image_get_info(int format, struct image_info *info)
{
    Int32 ret;
    RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, sizeof(Int32), &format);
    RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_Image_info_rpc, sizeof(Image_info_rpc), info);

    ret = RpcCallCompletion(RPC_image_get_info, &p1, &p2, NULL);
    return ret;
}

/*
int ali_rpc_image_decode(struct image_engine_config cfg)
{
	struct image_engine_config *img_cfg = NULL;
	void *ptr = NULL;
	int ret;

	ptr = kmalloc(sizeof(struct image_engine_config), GFP_KERNEL);
	dma_cache_inv(ptr, sizeof(struct image_engine_config));
	img_cfg = (struct image_engine_config *)(NONCACHE_ADDR(ptr));
	
	ptr = kmalloc(sizeof(struct image_slideshow_effect), GFP_KERNEL);
	dma_cache_inv(ptr, sizeof(struct image_slideshow_effect));
	img_cfg->effect = (struct image_slideshow_effect *)(NONCACHE_ADDR(ptr));

	if(img_cfg == NULL || img_cfg->effect == NULL) {
		printk("Kmalloc image config fail\n");
		return -1;
	} 
	else {
		img_cfg->effect->mode = cfg.effect->mode;
		img_cfg->effect->mode_param = cfg.effect->mode_param;
		
		img_cfg->decode_mode = cfg.decode_mode;
		img_cfg->show_mode = cfg.show_mode;
		img_cfg->dest_height = cfg.dest_height;
		img_cfg->dest_left = cfg.dest_left;
		img_cfg->dest_top = cfg.dest_top;
		img_cfg->dest_width = cfg.dest_width;
		img_cfg->src_height = cfg.src_height;
		img_cfg->src_left = cfg.src_left;
		img_cfg->src_top = cfg.src_top;
		img_cfg->src_width = cfg.src_width;
		img_cfg->img_fmt = cfg.img_fmt;
		img_cfg->rotate = cfg.rotate;
		img_cfg->vpo_mode = cfg.vpo_mode;
		
		ret = image_decode((void *)img_cfg);
	}
	
	return ret;
}
*/

