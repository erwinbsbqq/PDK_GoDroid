/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: alifb_vpo.c
 *  (I)
 *  Description: vpo register settting
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2009.12.16				Sam		Create
 ****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/div64.h>

#ifdef CONFIG_RPC_HLD_GMA
#include <rpc_hld/ali_rpc_hld_gma.h>
#include <rpc_hld/ali_rpc_hld_dis.h>
#else
//#include <rpc_hld/ali_rpc_hld_gma.h>
#include "ali_gma_m36f_lld.h"
#endif
#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>

#include <ali_shm.h>

#if defined(CONFIG_ALI_CHIP_M3921)
#include <ali_reg.h>
#endif

#include "ali_fb.h"
#include "ali_vpo.h"

static int m_gma_opened[2] = {0, 0};

#ifdef CONFIG_RPC_HLD_GMA
extern struct gma_device *g_gma_dev0;
extern struct gma_device *g_gma_dev1;
extern struct vpo_device *g_vpo_dev;

static enum GMA_FORMAT get_gma_format(uint32 dis_format)
{
	enum GMA_FORMAT format;

	switch(dis_format)
	{
		case DIS_FORMAT_CLUT8:
			format = GMA_FORMAT_CLUT8;
			break;
		case DIS_FORMAT_ARGB1555:
			format = GMA_FORMAT_ARGB1555;
			break;
		case DIS_FORMAT_ARGB8888:
			format = GMA_FORMAT_ARGB8888;
			break;
		case DIS_FORMAT_RGB888:
			format = GMA_FORMAT_RGB888;
			break;
		case DIS_FORMAT_RGB565:
			format = GMA_FORMAT_RGB565;
			break;
		default:
			format = GMA_FORMAT_MAX_VALUE;
			break;
	}

	return format;
}
#endif

void alifb_set_osd2_position(struct alifb_info *info, unsigned long screen_width, 
                              unsigned long screen_height, unsigned long param)
{
    struct vpo_device *dis_dev = (struct vpo_device*)hld_dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    struct gma_rect *posd_rect = (struct gma_rect *)param;
    enum TVSystem tv_sys = PAL;
    struct vpo_io_get_info dis_info;
    RET_CODE ret = RET_SUCCESS;
    int fb_idx = (info->DE_layer == DE_LAYER2) ? 0 : 1;
    int pos_x = 0;
    int pos_y = 0;
    int pos_w = 0;
    int pos_h = 0;
    int max_width  = 0;
    int max_height = 0;

    if ((1920 != screen_width) || (screen_width < 0) || (screen_height < 0) || (!posd_rect))
        return;

    if(dis_dev != NULL)
    {
        memset(&dis_info, 0, sizeof(dis_info));
	    ret = vpo_ioctl(dis_dev, VPO_IO_GET_INFO, (UINT32)&dis_info);
        if(ret == RET_SUCCESS)
        {
            tv_sys     = dis_info.tvsys;
            max_width  = dis_info.des_width;
            max_height = dis_info.des_height;

            if((max_width == 3840) || (max_width == 4096))
            {
                max_width /= 2;
            }
        }
        else
        {
            return;
        }
    }
    else
    {
        return;
    }
    
    FB_PRF("tv system %d srceen width %d height %d\n", tv_sys, max_width, max_height);
    
    pos_x = posd_rect->x;
    pos_w = posd_rect->w;
    pos_y = posd_rect->y;
    pos_h = posd_rect->h;
    
    if (max_width != screen_width)
    {
        pos_x *= max_width;
        pos_x /= screen_width;
        pos_w *= max_width;
        pos_w /= screen_width;
    }
    else
    {
        pos_w *= max_width;
        pos_w /= max_width;
    }

    if (max_height != screen_height)
    {
        pos_y *= max_height;
        pos_y /= screen_height;

        pos_h *= max_height;
        pos_h /= screen_height;
    }
    else
    {
        pos_h *= max_height;
        pos_h /= max_height;
    }

    FB_PRF("set osd2 rect <%d %d %d %d> => <%d %d %d %d>\n",
        posd_rect->x, posd_rect->y, posd_rect->w, posd_rect->h,
        pos_x, pos_y, pos_w, pos_h);

    posd_rect->x = pos_x;
    posd_rect->y = pos_y;
    posd_rect->w = pos_w;
    posd_rect->h = pos_h;
}

#ifndef CONFIG_RPC_HLD_GMA
int alifb_gma_open(struct alifb_info *info)
{
    int ret = 0;
    int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
    struct alifb_gma_info *gma_info = &info->gma_info[0];

    /* gma_attach */
    gma_layer_config_t osd_config[2];

    memset((void *)&osd_config, 0, sizeof(osd_config));
    osd_config[0].mem_base = info->fbinfo->fix.smem_start; 
    osd_config[0].mem_size = info->fbinfo->fix.smem_len/2;
    osd_config[0].bDirectDraw = FALSE;
    osd_config[0].bCacheable = FALSE;
    osd_config[0].bScaleFilterEnable = TRUE;
    osd_config[0].bP2NScaleInNormalPlay = FALSE;
    osd_config[0].bP2NScaleInSubtitlePlay = TRUE;

    osd_config[0].hw_layer_id = gma_info->layer_id;
    osd_config[0].color_format = GE_GMA_PF_ARGB8888;
    osd_config[0].pixel_pitch = info->fbinfo->var.xres_virtual; 
    osd_config[0].width = info->fbinfo->var.xres_virtual;
    osd_config[0].height = info->fbinfo->var.yres_virtual/2;

    osd_config[1].mem_base = info->fbinfo->fix.smem_start;
    osd_config[1].mem_size =info->fbinfo->fix.smem_len;
    osd_config[1].bDirectDraw = FALSE;
    osd_config[1].bCacheable = FALSE;
    osd_config[1].bScaleFilterEnable = TRUE;
    osd_config[1].bP2NScaleInNormalPlay = TRUE;
    osd_config[1].bP2NScaleInSubtitlePlay = TRUE;

    osd_config[1].hw_layer_id = gma_info->layer_id;
    osd_config[1].color_format = GE_GMA_PF_CLUT8;
    osd_config[1].pixel_pitch = info->fbinfo->var.xres_virtual;
    osd_config[1].width = info->fbinfo->var.xres_virtual;
    osd_config[1].height = info->fbinfo->var.yres_virtual;

    FB_PRF("Format = %d, pixel_pitch = %d, width = %d,height = %d\n",\
          gma_info->dis_format,gma_info->pitch,info->fbinfo->var.xres_virtual,\
          info->fbinfo->var.yres_virtual);
          
    for(fb_idx = 0; fb_idx < 2; fb_idx++) {
        gma_m36f_attach (fb_idx, &osd_config[fb_idx]); 

    //if(m_gma_opened[fb_idx] == 0)
    {
		if(gma_m36f_open(fb_idx) == RET_SUCCESS){
			FB_PRF("open gma dev ok\n");
			m_gma_opened[fb_idx] = 1;
		}
		else{
			FB_PRF("open gma dev fail\n");
			return 0;
		}
	}
}
    return ret;
}

int alifb_gma_create_region(struct alifb_info *info, int region_id)
{
    int fb_idx,ret = 0;
    struct alifb_gma_info *gma_info;
    alifb_gma_region_t pars;

    fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
    gma_info = &info->gma_info[region_id];
    
	if(region_id >= VIDEO_MAX_GMA_REGION_NUM || region_id < 0)
		return 0;

    if(info->gma_info[region_id].valid) {
        ali_vpo_delete_GMA_region(info, region_id);
	} 
	
    	
	memset((void *)&pars, 0, sizeof(pars));
    
    pars.color_format  =  gma_info->dis_format;
    pars.galpha_enable =  gma_info->glob_alpha_en;
    pars.global_alpha  =  gma_info->glob_alpha;
    pars.pallette_sel  =  gma_info->pallette_sel;
    
    pars.pixel_pitch   =  gma_info->w;//gma_info->pitch;
    pars.bitmap_addr   =  gma_info->bitmap_addr;

    pars.bitmap_x = gma_info->bitmap_x;
    pars.bitmap_y = gma_info->bitmap_y;
    pars.bitmap_w = gma_info->bitmap_w;
    pars.bitmap_h = gma_info->bitmap_h;
    
    pars.region_x = gma_info->x;
    pars.region_y = gma_info->y;
    pars.region_w = gma_info->w;
    pars.region_h = gma_info->h;
    FB_PRF("region_x = %d, y = %d, w = %d, h = %d\n",pars.region_x, pars.region_y, pars.region_w, pars.region_h);
    
	if(gma_m36f_create_region(fb_idx, region_id, &pars) == RET_SUCCESS){
		FB_PRF("gma create region ok\n");
		ret = 1;
	}
	else{
		FB_PRF("gma create region fail\n");
		return 0;
	}
    
    if(ret == 1)
		info->gma_info[region_id].valid = 1;
    
    return ret;
}
#endif

#ifdef ALI_FB_CREATE_NEW
/*
 * Attach and open GMA(the TDS way), called by and only called
 * by Linux Frambuffer framwork to probe for FB device at Kernel boot
 * time or dirver init time.
 *
 * This function will take care of OSD or SUBT GMA layer
 * initialization according to [alifb_info->DE_layer] id.
 *
 * @return :
 *     0 - error
 *     other - OK
 */
int ali_vpo_init_GMA(struct alifb_info *info)
{
    int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;

    /* gma_attach */
    gma_layer_config_t osd_config[2];

    /* GMA layer 0 - OSD layer */
    memset((void *) &osd_config, 0, sizeof(osd_config));
    osd_config[0].mem_base = info->fbinfo->fix.smem_start;
    /* FIXME: Here, we assume OSD layer use only half of FB-MEM.  */
    osd_config[0].mem_size = info->fbinfo->fix.smem_len;
	/* FB_INFO("OSD layer mem_size = %uld\n", osd_config[0].mem_size); */
    osd_config[0].width = info->fbinfo->var.xres_virtual;
    osd_config[0].height = info->fbinfo->var.yres_virtual;

    osd_config[0].bDirectDraw = FALSE;
    osd_config[0].bCacheable = FALSE;
    osd_config[0].bScaleFilterEnable = TRUE;
    osd_config[0].bP2NScaleInNormalPlay = FALSE;
    osd_config[0].bP2NScaleInSubtitlePlay = TRUE;

    osd_config[0].hw_layer_id = 0;
    /* osd_config[0].hw_layer_id = gma_info->layer_id; */
    osd_config[0].color_format = GE_GMA_PF_ARGB8888;
    osd_config[0].pixel_pitch = info->fbinfo->var.xres_virtual;

    /* GMA layer 0 - SUBT layer */
    osd_config[1].mem_base = info->fbinfo->fix.smem_start;
    /* FIXEME: Here , we assume SUBT layer is always single-buffered */
    osd_config[1].mem_size = info->fbinfo->fix.smem_len;
    osd_config[1].width = info->fbinfo->var.xres_virtual;
    osd_config[1].height = info->fbinfo->var.yres_virtual;

    osd_config[1].bDirectDraw = FALSE;
    osd_config[1].bCacheable = FALSE;
    osd_config[1].bScaleFilterEnable = TRUE;
    osd_config[1].bP2NScaleInNormalPlay = TRUE;
    osd_config[1].bP2NScaleInSubtitlePlay = TRUE;
    osd_config[1].hw_layer_id = 1;
    /* osd_config[1].hw_layer_id = gma_info->layer_id; */
    osd_config[1].color_format = GE_GMA_PF_CLUT8;
    osd_config[1].pixel_pitch = info->fbinfo->var.xres_virtual;

    gma_m36f_attach(fb_idx, &osd_config[fb_idx]);
    
    if (gma_m36f_open(fb_idx) == RET_SUCCESS) {
        m_gma_opened[fb_idx] = 1;
        //FB_INFO("layer %s opened!\n", fb_idx == 0 ? "OSD" : "SUBT");
        return 1;
    } else {
        //FB_ERR("layer %s open error!\n", fb_idx == 0 ? "OSD" : "SUBT");
        return 0;
    }
}

/*
 * Close and deattach GMA, a wapper of TDS GMA Driver. Since ALI
 * driver is compiled to be part of Linux kernel rather than a
 * .ko. Driver will never be unloaded, this funciton is for SW
 * completeness, and never called by kernel.
 *  */
int ali_vpo_deinit_GMA(struct alifb_info *info)
{
    int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
    
    gma_m36f_close(fb_idx);
    gma_m36f_detach(fb_idx);
    m_gma_opened[fb_idx] = 0;
    
    return 0;
}

static int __region_equal(alifb_gma_region_t* region1, alifb_gma_region_t* region2)
{
    if ((region1->color_format  == region2->color_format) &&
        (region1->galpha_enable == region2->galpha_enable) &&
        (region1->global_alpha  == region2->global_alpha) &&
        (region1->pallette_sel  == region2->pallette_sel) &&
        (region1->pixel_pitch   == region2->pixel_pitch) &&
        (region1->bitmap_addr   == region2->bitmap_addr) &&
        (region1->bitmap_x      == region2->bitmap_x) &&
        (region1->bitmap_y      == region2->bitmap_y) &&
        (region1->bitmap_w      == region2->bitmap_w) &&
        (region1->bitmap_h      == region2->bitmap_h) &&
        (region1->region_x      == region2->region_x) &&
        (region1->region_y      == region2->region_y) &&
        (region1->region_w      == region2->region_w) &&
        (region1->region_h      == region2->region_h)) {
        //FB_INFO("\n");
        return 1;
    } else {
        return 0;
    }

}

static void __copy_region(alifb_gma_region_t* to, alifb_gma_region_t* from)
{
    to->color_format  = from->color_format;;
    to->galpha_enable = from->galpha_enable;
    to->global_alpha  = from->global_alpha;
    to->pallette_sel  = from->pallette_sel;
    to->pixel_pitch   = from->pixel_pitch;
    to->bitmap_addr   = from->bitmap_addr;
    to->bitmap_x      = from->bitmap_x;
    to->bitmap_y      = from->bitmap_y;
    to->bitmap_w      = from->bitmap_w;
    to->bitmap_h      = from->bitmap_h;
    to->region_x      = from->region_x;
    to->region_y      = from->region_y;
    to->region_w      = from->region_w;
    to->region_h      = from->region_h;
}
#endif


int ali_vpo_create_GMA_region(struct alifb_info *info, int region_id)
{
	int ret = 0;

	#ifdef ALI_FB_CREATE_NEW
    /* store last SUBT region, when `last arg region' == `this one',
     * use last region without create a new one. */
    static int last_region_id = -1;
    static alifb_gma_region_t last_SUBT_region =
            {
                .color_format = -1,
                .galpha_enable = -1,
                .global_alpha = -1,
                .pallette_sel = -1,
                .pixel_pitch = -1,
                .bitmap_addr = -1,
                .bitmap_x = -1,
                .bitmap_y = -1,
                .bitmap_w = -1,
                .bitmap_h = -1,
                .region_x = -1,
                .region_y = -1,
                .region_w = -1,
                .region_h = -1,
    		};
	#endif

	if(region_id >= VIDEO_MAX_GMA_REGION_NUM || region_id < 0)
		return 0;
	
#ifdef CONFIG_RPC_HLD_GMA
	if(info->gma_info[region_id].valid)
		return 0;
#else
	#ifndef ALI_FB_CREATE_NEW
	if(info->gma_info[region_id].valid) {
        ali_vpo_delete_GMA_region(info, region_id);
	}    
    #endif
#endif

    #ifndef CONFIG_RPC_HLD_GMA
	{
		int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
        struct alifb_gma_info *gma_info = &info->gma_info[region_id];
        
		#ifndef ALI_FB_CREATE_NEW
        /* gma_attach */
        gma_layer_config_t osd_config[2];
        
        memset((void *)&osd_config, 0, sizeof(osd_config));
        osd_config[0].mem_base = info->fbinfo->fix.smem_start; 
        osd_config[0].mem_size = info->fbinfo->fix.smem_len/2;
        osd_config[0].bDirectDraw = FALSE;
        osd_config[0].bCacheable = FALSE;
        osd_config[0].bScaleFilterEnable = TRUE;
        osd_config[0].bP2NScaleInNormalPlay = FALSE;
        osd_config[0].bP2NScaleInSubtitlePlay = TRUE;

        osd_config[0].hw_layer_id = gma_info->layer_id;
        osd_config[0].color_format = GE_GMA_PF_ARGB8888;
        osd_config[0].pixel_pitch = info->fbinfo->var.xres_virtual; 
        osd_config[0].width = info->fbinfo->var.xres_virtual;
        osd_config[0].height = info->fbinfo->var.yres_virtual/2;

        osd_config[1].mem_base = info->fbinfo->fix.smem_start;
        osd_config[1].mem_size =info->fbinfo->fix.smem_len;
        osd_config[1].bDirectDraw = FALSE;
        osd_config[1].bCacheable = FALSE;
        osd_config[1].bScaleFilterEnable = TRUE;
        osd_config[1].bP2NScaleInNormalPlay = TRUE;
        osd_config[1].bP2NScaleInSubtitlePlay = TRUE;

        osd_config[1].hw_layer_id = gma_info->layer_id;
        osd_config[1].color_format = GE_GMA_PF_CLUT8;
        osd_config[1].pixel_pitch = info->fbinfo->var.xres_virtual;
        osd_config[1].width = info->fbinfo->var.xres_virtual;
        osd_config[1].height = info->fbinfo->var.yres_virtual;

        FB_PRF("--dis_format = %d, pixel_pitch = %d, width = %d,height = %d\n",\
              gma_info->dis_format,gma_info->pitch,info->fbinfo->var.xres_virtual,info->fbinfo->var.yres_virtual/2);
        gma_m36f_attach (fb_idx, &osd_config[fb_idx]); 

        if(m_gma_opened[fb_idx] == 0){
			if(gma_m36f_open(fb_idx) == RET_SUCCESS){
				FB_PRF("open gma dev ok\n");
				m_gma_opened[fb_idx] = 1;
			}
			else{
				FB_PRF("open gma dev fail\n");
				return 0;
			}
		}
		#endif
		
		{			
			alifb_gma_region_t pars;
			memset((void *)&pars, 0, sizeof(pars));
            
            pars.color_format  =  gma_info->dis_format;
            pars.galpha_enable =  gma_info->glob_alpha_en;
            pars.global_alpha  =  gma_info->glob_alpha;
            pars.pallette_sel  =  gma_info->pallette_sel;
            
            pars.bitmap_addr = gma_info->bitmap_addr;
            pars.bitmap_x = gma_info->bitmap_x;
            pars.bitmap_y = gma_info->bitmap_y;
            pars.bitmap_w = gma_info->bitmap_w;
            pars.bitmap_h = gma_info->bitmap_h;

            pars.pixel_pitch = gma_info->w;//gma_info->pitch;
            pars.region_x = gma_info->x;
            pars.region_y = gma_info->y;
            pars.region_w = gma_info->w;
            pars.region_h = gma_info->h;
            FB_PRF("region x = %d, y = %d, w = %d, h = %d\n",pars.region_x,pars.region_y,pars.region_w,pars.region_h);

    		#ifdef ALI_FB_CREATE_NEW
            if (info->gma_info[region_id].valid)
			{
                if (info->DE_layer == DE_LAYER3)
				{
    				/* if new region is same as last region, create it */
                    if (__region_equal(&last_SUBT_region, &pars) && (last_region_id == region_id))
					{
                        return 1;
                    } 
					else
					{
                        //FB_INFO("new region, now store\n");
                        last_region_id = region_id;
                        __copy_region(&last_SUBT_region, &pars);
                    }
                }
                ali_vpo_delete_GMA_region(info, region_id);
            }
        	#endif
		
			if(gma_m36f_create_region(fb_idx, region_id, &pars) == RET_SUCCESS){
				FB_PRF("gma create region ok\n");
				ret = 1;
			}
			else{
				FB_PRF("gma create region fail\n");
				return 0;
			}
		}
	}

    #else
	{
		struct gma_device *dev = NULL;
		int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;

		dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;
		if(m_gma_opened[fb_idx] == 0){
			struct layer_pars pars;
			
			if(dev == NULL){
				FB_PRF("remote gma device is NULL \n");
				return 0;
			}
			memset((void *)&pars, 0, sizeof(pars));
			pars.format = get_gma_format(info->gma_info[region_id].dis_format);
			pars.global_alpha_enable = 0;
			pars.global_alpha_value = 0xFF;
			pars.max_size.w = info->fbinfo->var.xres_virtual;
			pars.max_size.h = info->fbinfo->var.yres_virtual;
			if(gma_open(dev, &pars) == RET_SUCCESS){
				FB_PRF("gma open max size <%d %d>, format %d\n", pars.max_size.w, pars.max_size.h, pars.format);
				m_gma_opened[fb_idx] = 1;
			} else {
				FB_PRF("open gma dev fail\n");
				return 0;
			}
		}

		{
			struct alifb_gma_info *gma_info = &info->gma_info[region_id];		
			struct region_pars pars;

			memset((void *)&pars, 0, sizeof(pars));
			pars.rect.x = gma_info->x;
			pars.rect.y = gma_info->y;
			pars.rect.w = gma_info->w;
			pars.rect.h = gma_info->h;
			pars.dmem_start = (gma_info->data_buffer & 0x3fffffff);
			pars.dmem_len = gma_info->pitch * gma_info->h * gma_info->bpp;
			pars.line_length = gma_info->pitch;
			pars.global_alpha_enable = 0;
			pars.global_alpha_value = 0xFF;
			if(gma_create_region(dev, region_id, &pars) == RET_SUCCESS){
				FB_PRF("gma create region <%d %d %d %d> pitch %d buffer 0x%x\n",
                    gma_info->x, gma_info->y, gma_info->w, gma_info->h,
                    gma_info->pitch, gma_info->data_buffer);
				ret = 1;
			}
			else{
				FB_PRF("gma create region fail\n");
				return 0;
			}
		}
	}
    #endif

	//if(ret == 1)  poe modify for c++ test: always ret == 1 if can run here.
		info->gma_info[region_id].valid = 1;
	
	return ret;
}

int ali_vpo_delete_GMA_region(struct alifb_info *info, int region_id)
{
	int ret = 1;
    
	#ifndef CONFIG_RPC_HLD_GMA
    int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
    #endif

	if(region_id >= VIDEO_MAX_GMA_REGION_NUM || region_id < 0)
		return 0;
	
	if(!info->gma_info[region_id].valid)
		return 0;
    
#ifndef CONFIG_RPC_HLD_GMA
	if(gma_m36f_delete_region (fb_idx, region_id)== 0)
    {
        FB_PRF("gma delete region(layer:%d,region:%d) ok\n",fb_idx, region_id); 
        ret = 1;
    } else { 
        FB_PRF("gma delete region(layer:%d,region:%d) failed\n",fb_idx, region_id);
        ret = 0;
    }
	info->gma_info[region_id].valid = 0;
    
	#ifndef ALI_FB_CREATE_NEW
    {
		int i = 0;		
		for(i = 0;i < VIDEO_MAX_GMA_REGION_NUM;i++)
			if(info->gma_info[i].valid)
				break;

		if(i >= VIDEO_MAX_GMA_REGION_NUM){
			gma_m36f_close (fb_idx);
            gma_m36f_detach(fb_idx);
			m_gma_opened[fb_idx] = 0;
		}
	}

	info->gma_info[region_id].valid = 0;
	#endif
#else
	struct gma_device *dev = NULL;
	int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
    int i = 0;

	dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;	
	gma_delete_region(dev, region_id);

	info->gma_info[region_id].valid = 0;
		
	for(i = 0;i < VIDEO_MAX_GMA_REGION_NUM;i++)
		if(info->gma_info[i].valid)
			break;
		
	if(i >= VIDEO_MAX_GMA_REGION_NUM){
		gma_close(dev);
		m_gma_opened[fb_idx] = 0;
	}
#endif
    FB_PRF("gma delete region %d valid %d ok\n", region_id, info->gma_info[region_id].valid);
	return ret;
}

RET_CODE gma_m36f_store_scale_info(UINT32 layer_id, gma_scale_param_t *scale_param);

int ali_vpo_update_GMA_region(struct alifb_info *info, int region_id)
{
	int ret = 0;
    struct gma_rect rect_parm;
	#ifndef CONFIG_RPC_HLD_GMA
    int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
    unsigned long uScaleCmd,uScaleParam;
    gma_scale_param_t pars;
	#endif
	
	if(region_id >= VIDEO_MAX_GMA_REGION_NUM || region_id < 0)
		return 0;
	
	//if(!info->gma_info[region_id].valid)
	//	return 0;
	
#ifndef CONFIG_RPC_HLD_GMA
	struct alifb_gma_info *gma_info = &info->gma_info[region_id];
	if(gma_info->pending_flag & ALIFB_GMA_FLAG_SCALE_PENDING)
    {	    			
		memset((void *)&pars, 0, sizeof(pars));
        if(info->DE_layer == DE_LAYER3 && 
          (info->fbinfo->var.vmode & FB_VMODE_YWRAP)) 
        {        
            rect_parm.x = info->fbinfo->var.xoffset;
            rect_parm.y = info->fbinfo->var.yoffset;
            rect_parm.w = gma_info->h_dst;
            rect_parm.h = gma_info->v_dst;
            alifb_set_osd2_position(info, info->fbinfo->var.xres_virtual, \
                info->fbinfo->var.yres_virtual, (uint32)&rect_parm);                  
        }
        
        pars.tv_sys = gma_info->tv_sys;
        pars.h_div = gma_info->h_src;
        pars.v_div = gma_info->v_src;

        if(info->DE_layer == DE_LAYER3 
           && (info->fbinfo->var.vmode & FB_VMODE_YWRAP)) 
        {
            pars.h_mul = rect_parm.w;
            pars.v_mul = rect_parm.h;
        } else {
            pars.h_mul = gma_info->h_dst;
            pars.v_mul = gma_info->v_dst;
        }
        
        uScaleCmd =  gma_info->u_scale_cmd;
        uScaleParam = gma_info->uScaleParam;
   
	   	#ifdef ALI_FB_CREATE_NEW
		// store h_mul/v_mul + h_div/v_div to private info of gma
		if (info->DE_layer == DE_LAYER3 &&
			(info->fbinfo->var.vmode & FB_VMODE_YWRAP)) {
			gma_m36f_store_scale_info(fb_idx, &pars);
		}
		#endif
		
        if(uScaleCmd == GE_SET_SCALE_PARAM) 
        {
            if(gma_m36f_scale (fb_idx, GE_SET_SCALE_PARAM, (uint32)&pars) == RET_SUCCESS) 
            {
			    FB_PRF("gma scale done\n");
			    ret = 1;
		    }
		    else
            {
			    FB_PRF("gma scale fail\n");
			    ret = 0;
		    }
            
            if(info->DE_layer == DE_LAYER3 && (info->fbinfo->var.vmode & FB_VMODE_YWRAP)) 
            {
                gma_m36f_ioctl (fb_idx, GE_IO_SET_DISPLAY_RECT, (uint32)&rect_parm);
            }
        }
        else
        {
            if(gma_m36f_scale (fb_idx, uScaleCmd, uScaleParam) == RET_SUCCESS) 
            {
                FB_PRF("gma scale done\n");
			    ret = 1;
		    }
		    else
            {
			    FB_PRF("gma scale fail\n");
			    ret = 0;
		    }
        }  
        
	}

	if(gma_info->pending_flag & ALIFB_GMA_FLAG_RECT_PENDING)
    {
        alifb_gma_region_t pars;
        gma_m36f_get_region_info (fb_idx,region_id, &pars);
        pars.region_x = gma_info->x;
		pars.region_y= gma_info->y;
		pars.region_w= gma_info->width;
		pars.region_h= gma_info->height;
        gma_m36f_move_region (fb_idx,region_id, &pars);
		FB_PRF("new gma pos x %d y %d\n", gma_info->x, gma_info->y);
	}

	if(gma_info->pending_flag & ALIFB_GMA_FLAG_BUF_PENDING)
    {
        alifb_gma_region_t pars;
        gma_m36f_get_region_info (fb_idx,region_id, &pars);
        pars.bitmap_addr = gma_info->data_buffer | 0xA0000000;
        gma_m36f_set_region_info (fb_idx,region_id, &pars);
        FB_PRF("called by alifb_pan_display\n");
	}

	gma_info->pending_flag = 0;
#else
	{
		struct alifb_gma_info *gma_info = &info->gma_info[region_id];
		struct gma_device *dev = NULL;
		int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;

		dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;			

		if(gma_info->pending_flag & ALIFB_GMA_FLAG_SCALE_PENDING) {
			struct scale_pars pars;	

            if((info->DE_layer == DE_LAYER3) 
               && (info->fbinfo->var.vmode & FB_VMODE_YWRAP)) 
            {        
                rect_parm.x = info->fbinfo->var.xoffset;
                rect_parm.y = info->fbinfo->var.yoffset;
                rect_parm.w = gma_info->h_dst;
                rect_parm.h = gma_info->v_dst;
                alifb_set_osd2_position(info, info->fbinfo->var.xres_virtual, \
                    info->fbinfo->var.yres_virtual, (unsigned long)&rect_parm);
            }

			memset((void *)&pars, 0, sizeof(pars));
			pars.scale_type = (gma_info->scale_mode == GMA_RSZ_DIRECT_RESIZE) \
				              ? GMA_SCALE_DUPLICATE : GMA_SCALE_FILTER;
			pars.h_src = gma_info->h_src;
            pars.v_src = gma_info->v_src;

            if((info->DE_layer == DE_LAYER3) 
               && (info->fbinfo->var.vmode & FB_VMODE_YWRAP)) 
            {
                pars.h_dst = rect_parm.w;
                pars.v_dst = rect_parm.h;
            }
            else
            {
                pars.h_dst = gma_info->h_dst;
    			pars.v_dst = gma_info->v_dst;
            }
			
			if(gma_scale(dev, &pars) == RET_SUCCESS) {
				FB_PRF("gma scale done\n");
				ret = 1;
			} else {
				FB_PRF("gma scale fail\n");
				ret = 0;
			}

            #if defined(CONFIG_ALI_CHIP_M3921)			
			{
				struct region_pars pars;
				gma_get_region_info(dev, region_id, &pars);
                FB_PRF("region info <%d %d %d %d>\n", pars.rect.x, pars.rect.y, pars.rect.w, pars.rect.h);

				if((pars.dmem_start & 0x3FFFFFFF) != (gma_info->data_buffer & 0x3FFFFFFF))
				{
					pars.dmem_start = gma_info->data_buffer;

					gma_set_region_info(dev, region_id, &pars);					
				}				

			}
            #endif
		}

		if(gma_info->pending_flag & ALIFB_GMA_FLAG_RECT_PENDING) {
			struct region_pars pars;

            memset((void *)&pars, 0, sizeof(pars));
			gma_get_region_info(dev, region_id, &pars);
            #if defined(CONFIG_ALI_CHIP_M3921)		
			if((pars.dmem_start & 0x3FFFFFFF) != (gma_info->data_buffer & 0x3FFFFFFF))
			{
                FB_PRF("gma new data buffer 0x%x=>0x%x\n", pars.dmem_start, gma_info->data_buffer);
				pars.dmem_start = gma_info->data_buffer;
			}
            #endif
            FB_PRF("gma new pos <%d %d %d %d> => <%d %d %d %d>\n", 
                pars.rect.x, pars.rect.y, pars.rect.w, pars.rect.h,
                gma_info->x, gma_info->y, gma_info->w, gma_info->h);
			pars.rect.x = gma_info->x;
			pars.rect.y = gma_info->y;
			pars.rect.w = gma_info->w;
			pars.rect.h = gma_info->h;
			gma_set_region_info(dev, region_id, &pars);
		}

		if(gma_info->pending_flag & ALIFB_GMA_FLAG_BUF_PENDING) {
            #if defined(CONFIG_ALI_CHIP_M3921)
			unsigned long cq_phy_addr = 0;
			unsigned long cq_vir_addr = 0;
			unsigned long *block_addr = NULL;
            unsigned long *next_block_addr = NULL;
			int cnt = 100;
            int update = 0;
				
			if(fb_idx == 0)
			{
                cq_phy_addr = __REG32ALI(0x18006080);
                cq_vir_addr = __VSTMALI(cq_phy_addr);
                block_addr  = (unsigned long *)cq_vir_addr;
                /* interlace tv system has top and bottom block */
                if((*block_addr) & 0x80000000)
                {
                    cq_phy_addr = *(block_addr + 1);
                    cq_vir_addr = __VSTMALI(cq_phy_addr) + 0x20;
                    next_block_addr = (unsigned long *)cq_vir_addr; 
                }
              
				cq_phy_addr = __REG32ALI(0x18006080);				
				cq_vir_addr = __VSTMALI(cq_phy_addr) + 0x20;
				block_addr = (unsigned long *)cq_vir_addr;
				
				while(cnt > 0)
				{
					if(*(block_addr) == 0xB80069E0)
					{
						cq_phy_addr = *(block_addr + 1);
						cq_vir_addr = __VSTMALI(cq_phy_addr);
						block_addr = (unsigned long *)cq_vir_addr;

						*(block_addr + 1) = (gma_info->data_buffer & 0x3FFFFFFF);
                        update = 1;

                        if(next_block_addr == NULL)
                        {
						    break;
                        }
                        else
                        {
                            block_addr = next_block_addr;
                            next_block_addr = NULL;
                            cnt = 100;
                            continue;
                        }
					}

					block_addr += 2;
					cnt--;
				}

                if(update)
				{
					__REG32ALI(0x18006084) = 1;
				}
			}
			else
			{
				struct region_pars pars;
				gma_get_region_info(dev, region_id, &pars);
				pars.dmem_start = gma_info->data_buffer;
                FB_PRF("gma new data buffer 0x%x=>0x%x\n", pars.dmem_start, gma_info->data_buffer);
				gma_set_region_info(dev, region_id, &pars);	
			}
            #else
			struct region_pars pars;

			gma_get_region_info(dev, region_id, &pars);
			
			pars.dmem_start = gma_info->data_buffer;
			gma_set_region_info(dev, region_id, &pars);	
            #endif			
		}

		gma_info->pending_flag = 0;
	}
#endif
	
	return ret;	
}

void ali_vpo_show_GMA_layer(struct alifb_info *info, int on)
{
	//if(info->gma_info[info->cur_region_id].valid)	
	{
        #ifndef CONFIG_RPC_HLD_GMA
		int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
		gma_m36f_show_onoff(fb_idx, on);		
        #else		
		struct gma_device *dev = NULL;
		int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;

		dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;	
		gma_show(dev, on);		
        FB_PRF("gma onoff %d\n", on);
        #endif
	}
}

void ali_vpo_set_GMA_palette(struct alifb_info *info)
{
	//if(info->gma_info[info->cur_region_id].valid)
	{
        #ifndef CONFIG_RPC_HLD_GMA
		int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
		gma_pal_attr_t pars;		
		memset((void *)&pars, 0, sizeof(pars));
        
        pars.pal_type = (unsigned char)(info->plt_type);         // GE_PAL_RGB or GE_PAL_YCBCR
	    pars.rgb_order = info->rgb_order;        // enum GE_RGB_ORDER
	    pars.alpha_range =info->alpha_range;      // enum GE_ALPHA_RANGE
	    pars.alpha_pol = info->alpha_pol;        // enum GE_ALPHA_POLARITY
        if(info->plt_alpha_level == PLT_ALPHA_16 )
            gma_m36f_set_pallette (fb_idx, info->palette_buffer, info->pallete_size, NULL);
        else
            gma_m36f_set_pallette (fb_idx, info->palette_buffer, info->pallete_size, &pars);
        #else
		struct gma_device *dev = NULL;
		int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
		struct pallette_pars pars;
		
		dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;	
		
		memset((void *)&pars, 0, sizeof(pars));
		pars.type = info->plt_type;
		pars.alpha_level = info->plt_alpha_level;
		pars.pallette_buf = (UINT32)__VMTSALI(info->palette_buffer);
		pars.color_num = info->pallete_size;
		gma_set_pallette(dev, &pars);
        FB_PRF("gma set pallette, buffer 0x%x type %d num %d\n", 
            pars.pallette_buf, pars.type, pars.color_num);
        #endif
	}
}

void ali_vpo_set_GMA_alpha(struct alifb_info *info, unsigned char alpha)
{
    #ifdef CONFIG_RPC_HLD_GMA
	struct gma_device *dev = NULL;
	int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;	

	dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;	
	gma_io_control(dev, GMA_IO_SET_GLOBAL_ALPHA, alpha);
    #else
	int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;	
    
    gma_m36f_ioctl (fb_idx, GE_IO_SET_GLOBAL_ALPHA, alpha);
    #endif
    FB_PRF("gma%d set global alpha %d\n", fb_idx, alpha);
}

void ali_gma_set_dbg_flag(struct alifb_info *info, unsigned long flag)
{
    #ifdef CONFIG_RPC_HLD_GMA
	struct gma_device *dev = NULL;
	int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;	

	dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;	
	gma_io_control(dev, GMA_IO_SET_DBG_FLAG, flag);
    #endif
}

void ali_vpo_set_region_by(struct alifb_info *info, unsigned char set_region_by)
{
	struct gma_device *dev = NULL;
	int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;	

    #ifdef CONFIG_RPC_HLD_GMA
	dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;	
	gma_io_control(dev, GMA_IO_SET_REGION_BY, set_region_by);
    #endif
}

void ali_osd_set_enhance_pars(struct alifb_info *info, int change_flag, int value)
{
	struct gma_device *dev = NULL;
	int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;	
	GMA_ENH_PARS enh_pars;
	
    #ifdef CONFIG_RPC_HLD_GMA
	dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;	

	enh_pars.enhance_flag = change_flag;
	enh_pars.enhance_grade = value;
	gma_io_control(dev, GMA_IO_SET_ENHANCE_PAR, (UINT32)(&enh_pars));
    FB_PRF("gma%d set enhance %d %d\n", fb_idx, change_flag, value);
    #endif
}

#ifdef CONFIG_RPC_HLD_GMA
int ali_vpo_set_full_screen(struct alifb_info *info, int src_width, int src_height)
{
	struct gma_device *dev = NULL;
	struct vpo_io_get_info vpo_info;
	struct scale_pars pars;	
	int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;

	dev = fb_idx == 0 ? g_gma_dev0 : g_gma_dev1;	

    memset((void *)&vpo_info, 0, sizeof(vpo_info));
	vpo_ioctl(g_vpo_dev, VPO_IO_GET_INFO, (UINT32)(&vpo_info));

	memset((void *)&pars, 0, sizeof(pars));
	pars.scale_type = GMA_SCALE_FILTER;
	pars.h_src = src_width;	
	pars.v_src = src_height;	
    pars.h_dst = vpo_info.des_width;
	pars.v_dst = vpo_info.des_height;

    if((vpo_info.des_width == 3840) || (vpo_info.des_width == 4096))
    {
        pars.h_dst /= 2;
    }

	FB_PRF("%s : src_w %d src_h %d dst_w %d dst_h %d\n"
		, __FUNCTION__, pars.h_src, pars.v_src, pars.h_dst, pars.v_dst);

	if(gma_scale(dev, &pars) != RET_SUCCESS){
		return -1;
	}	

	return 0;
}
#else
void ali_set_GMA_IOCtl(struct alifb_info *info, unsigned long dwParam)
{
	int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;	
    gma_m36f_ioctl (fb_idx, GE_IO_SET_DISPLAY_RECT, dwParam);  
}

void ali_get_GMA_IOCtl(struct alifb_info *info, unsigned long dwParam)
{
    int fb_idx = info->DE_layer == DE_LAYER2 ? 0 : 1;
    struct gma_rect *rect_parm = (struct gma_rect *)dwParam;
    gma_m36f_ioctl (fb_idx, GE_IO_GET_DISPLAY_RECT, (uint32)rect_parm);
    
}
#endif


