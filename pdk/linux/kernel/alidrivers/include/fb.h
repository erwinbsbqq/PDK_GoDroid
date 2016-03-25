/* linux/arch/arm/plat-s3c/include/plat/fb.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C - FB platform data definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __PLAT_S3C_FB_H
#define __PLAT_S3C_FB_H __FILE__

#define FB_SWAP_WORD	(1 << 24)
#define FB_SWAP_HWORD	(1 << 16)
#define FB_SWAP_BYTE	(1 << 8)
#define FB_SWAP_BIT	(1 << 0)

struct platform_device;
struct clk;

struct s3c_platform_fb {
	int		hw_ver;
	char		clk_name[16];
	int		nr_wins;
	int		nr_buffers[5];
	int		default_win;
	int		swap;
	phys_addr_t	pmem_start[5]; /* starting physical address of memory region */
	size_t		pmem_size[5]; /* size of memory region */
	void            *lcd;
	void		(*cfg_gpio)(struct platform_device *dev);
	int		(*backlight_on)(struct platform_device *dev);
	int		(*backlight_onoff)(struct platform_device *dev, int onoff);
	int		(*reset_lcd)(struct platform_device *dev);
	int		(*clk_on)(struct platform_device *pdev, struct clk **s3cfb_clk);
	int		(*clk_off)(struct platform_device *pdev, struct clk **clk);
};

extern void s3c_fb_set_platdata(struct s3c_platform_fb *fimd);

/* defined by architecture to configure gpio */
extern void s3cfb_cfg_gpio(struct platform_device *pdev);
extern int s3cfb_backlight_on(struct platform_device *pdev);
extern int s3cfb_backlight_onoff(struct platform_device *pdev, int onoff);
extern int s3cfb_reset_lcd(struct platform_device *pdev);
extern int s3cfb_clk_on(struct platform_device *pdev, struct clk **s3cfb_clk);
extern int s3cfb_clk_off(struct platform_device *pdev, struct clk **clk);
extern void s3cfb_get_clk_name(char *clk_name);

#endif /* __PLAT_S3C_FB_H */
