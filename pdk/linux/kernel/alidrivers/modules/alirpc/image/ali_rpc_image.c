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
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
//#include <linux/ali_image.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/div64.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
//#include <linux/smp_lock.h>
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
#include "../ali_rpc.h"

#define CACHE_ADDR(addr)    ((unsigned int)addr&0x8fffffff)

#define NONCACHE_ADDR(addr) ((unsigned int)addr|0xa0000000)


static UINT32 desc_ie_init[] = 
{ //desc of pointer para
  1, DESC_STATIC_STRU(0, sizeof(struct image_init_config)), 
  1, DESC_P_PARA(0, 0, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ie_decode[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct image_engine_config)), DESC_STATIC_STRU(1, sizeof(struct image_slideshow_effect)), 
  2, DESC_P_PARA(0, 1, 0), DESC_P_STRU(1, 0, 1, offsetof(struct image_engine_config, effect)),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ie_display[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct image_display_t)), DESC_STATIC_STRU(1, sizeof(struct image_slideshow_effect)), 
  2, DESC_P_PARA(0, 1, 0), DESC_P_STRU(1, 0, 1, offsetof(struct image_display_t, effect)),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ie_zoom[] = 
{ //desc of pointer para
  2, DESC_STATIC_STRU(0, sizeof(struct Rect)), DESC_STATIC_STRU(1, sizeof(struct Rect)), 
  2, DESC_P_PARA(0, 0, 0), DESC_P_PARA(1, 1, 1),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ie_hw_info[] =
{//desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct image_hw_info)), 
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,                          
  0,
};

static UINT32 desc_ie_info[] =
{//desc of pointer para
  1, DESC_OUTPUT_STRU(0, sizeof(struct image_info)), 
  1, DESC_P_PARA(0, 1, 0),
  //desc of pointer ret
  0,                          
  0,
};

int image_init(struct image_init_config *cfg)
{
	
	
	jump_to_func(NULL, ali_rpc_call, cfg, ((LIB_IMAGE_ENGINE_MODULE<<24)|(1<<16)|FUNC_IMAGE_INIT), desc_ie_init);

	
}

int image_release()
{
	
	
	jump_to_func(NULL, ali_rpc_call, null, ((LIB_IMAGE_ENGINE_MODULE<<24)|(0<<16)|FUNC_IMAGE_RELEASE),NULL);

	
}

int image_stop()
{
	
	
	jump_to_func(NULL, ali_rpc_call, null, ((LIB_IMAGE_ENGINE_MODULE<<24)|(0<<16)|FUNC_IMAGE_STOP),NULL);

	
}

int image_decode(int sw_hw, struct image_engine_config *cfg)
{
	
	
	jump_to_func(NULL, ali_rpc_call, sw_hw, ((LIB_IMAGE_ENGINE_MODULE<<24)|(2<<16)|FUNC_IMAGE_DECODE), desc_ie_decode);

	
}

int image_rotate(unsigned char rotate_angle)
{
    

    jump_to_func(NULL, ali_rpc_call, rotate_angle, ((LIB_IMAGE_ENGINE_MODULE<<24)|(1<<16)|FUNC_IMAGE_ROTATE), NULL);

    
}

int image_zoom(struct Rect *dstRect, struct Rect *srcRect)
{
    

    jump_to_func(NULL, ali_rpc_call, dstRect, ((LIB_IMAGE_ENGINE_MODULE<<24)|(2<<16)|FUNC_IMAGE_ZOOM), desc_ie_zoom);
    
    
}

int image_display(int sw_hw, struct image_display_t *cfg)
{
    
	
	jump_to_func(NULL, ali_rpc_call, sw_hw, ((LIB_IMAGE_ENGINE_MODULE<<24)|(2<<16)|FUNC_IMAGE_DISPLAY), desc_ie_display);

	
}

int image_get_hw_addr(int flag_nused, struct image_hw_info *hw_info)
{
    

    jump_to_func(NULL, ali_rpc_call, flag_nused, ((LIB_IMAGE_ENGINE_MODULE<<24)|(2<<16)|FUNC_IMAGE_GET_HWADDR), desc_ie_hw_info);
    
    
}

int image_get_info(int format, struct image_info *info)
{
    

    jump_to_func(NULL, ali_rpc_call, format, ((LIB_IMAGE_ENGINE_MODULE<<24)|(2<<16)|FUNC_IMAGE_GET_INFO), desc_ie_info);

    
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

