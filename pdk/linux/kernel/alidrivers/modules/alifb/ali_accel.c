/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: ali_accel.c
 *  (I)
 *  Description: graphic accelerator
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
#include <ali_video_common.h>
#include <ali_decv_plugin_common.h>
#include <asm/io.h>
#include <asm/div64.h>

#include "ali_fb.h"
#include "ali_accel.h"

void alifb_fillrect_accel(struct fb_info *p, const struct fb_fillrect *rect)
{
	cfb_fillrect(p, rect);
}

void alifb_copyarea_accel(struct fb_info *p, const struct fb_copyarea *area)
{
	cfb_copyarea(p, area);	
}

void alifb_imageblit_accel(struct fb_info *p, const struct fb_image *image) 
{
	cfb_imageblit(p, image);
}

