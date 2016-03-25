/*
 * gfxdrv.h
 *
 *  Created on: 2011-8-7
 *      Author: christian.xie
 */

#ifndef GFXDRV_H_
#define GFXDRV_H_

typedef enum {
	ERR_OK = 0,
	ERR_INVALIED_PARA,	
	ERR_SYS,
	ERR_OTHER
	
}error_code;

typedef enum {
	// only alpha component per pixel, color defined per operation
	surface_format_MONO,
	// full 32 bit color
	surface_format_ARGB8888
}gfxdrv_surface_format;

typedef struct {
	// dimensions in pixels
	unsigned int width, height;
	// offset in bytes between two consecutive rows
	unsigned int pitch;
	// format of pixels
	gfxdrv_surface_format format;
	// physical address used by GFX HW
	void *physical_address;
	// virtual address used by user space code
	void *virtual_address;
	}gfxdrv_surface;

typedef struct {
		unsigned int x, y, width, height;
	}gfxdrv_rect;

error_code gfxdrv_init(void);
error_code gfxdrv_exit(void);
//Fill a region of an ARGB8888 surface with givern color value
error_code gfxdrv_color_fill(const gfxdrv_surface *dest,const gfxdrv_rect *dest_rect,unsigned int argb_color);

//Blend a color rectangle over a region of an ARGB8888 surface (the alpha factor is encoded in co
error_code gfxdrv_color_blend(const gfxdrv_surface *dest,const gfxdrv_rect *dest_rect,unsigned int argb_color);

//Copy a region of any surface to an ARGB8888 surfac
error_code gfxdrv_copy(const gfxdrv_surface *dest,unsigned int dest_x, unsigned int dest_y, \
                               const gfxdrv_surface *src,const gfxdrv_rect *src_rect, \
                               unsigned int src_color); // used when source surface in MONO format
                               
//Blend a region of any surface to an ARGB8888 surface, with scaling on-the-fly
error_code gfxdrv_scale(const gfxdrv_surface *dest,const gfxdrv_rect *dest_rect,const gfxdrv_surface *src,const gfxdrv_rect *src_rect);
error_code gfxdrv_blend(const gfxdrv_surface *dest,unsigned int dest_x, unsigned int dest_y, \
                         const gfxdrv_surface *src,const gfxdrv_rect *src_rect, unsigned int src_color,unsigned char src_alpha); 
                         // used when source surface in MONO format
                                
//Copy a region of any surface to an ARGB8888 surface, source surface format must be same as dest format
error_code gfxdrv_direct_copy(const gfxdrv_surface *dest,unsigned int dest_x, unsigned int dest_y, \
                               const gfxdrv_surface *src,const gfxdrv_rect *src_rect);
#endif /* SURFACE_H_ */
