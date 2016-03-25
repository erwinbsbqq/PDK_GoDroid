#ifndef __DRIVERS_VIDEO_ALI_ACCEL_H
#define __DRIVERS_VIDEO_ALI_ACCEL_H

#ifdef CONFIG_RPC_HLD_GMA
#define M36_GE_REG_BASEADDR  0x1800A000
#define M36_GE_REG_LEN	0x100
#else
#define M36_GE_REG_BASEADDR  0x18000000
#define M36_GE_REG_LEN	0xE000
#endif

void alifb_fillrect_accel(struct fb_info *p, const struct fb_fillrect *rect);
void alifb_copyarea_accel(struct fb_info *p, const struct fb_copyarea *area);
void alifb_imageblit_accel(struct fb_info *p, const struct fb_image *image);
void alifb_scaleblit_accel(struct fb_info *p, const struct alifbio_flush_GMA_rect_pars *pars);
#endif
