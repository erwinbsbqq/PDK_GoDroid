/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009-2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#ifndef MALI_SURFACE_SPECIFIER_H
#define MALI_SURFACE_SPECIFIER_H

#include <mali_system.h>
#include <shared/mali_pixel_format.h>
#include <shared/m200_texel_format.h>

typedef enum
{
	MALI_SURFACE_COLORSPACE_lRGB = 0,
	MALI_SURFACE_COLORSPACE_sRGB = 1
} mali_surface_colorspace;

typedef enum
{
	MALI_SURFACE_ALPHAFORMAT_NONPRE = 0,
	MALI_SURFACE_ALPHAFORMAT_PRE = 1
} mali_surface_alphaformat;

typedef struct mali_surface_specifier
{
	u16 width;                                /* The width of the surface in pixels. */
	u16 height;                               /* The height of the surface in pixels. */
	u16 pitch;                                /* The size of one pixel row in bytes, or 0 to compute from the width and format. Must be 0 for non-linear layouts*/
	mali_pixel_format pixel_format;           /* The pixel format of this surface. If MALI_PIXEL_FORMAT_NONE, not writable */
	m200_texel_format texel_format;           /* The texel format of this surface. If M200_TEXEL_FORMAT_NO_TEXTURE, not readable */
	mali_pixel_layout pixel_layout;           /* The pixel layout for this surface. Should always be specified */
	m200_texture_addressing_mode texel_layout;/* The texel layout for this surface. Should always be specified */
	mali_bool red_blue_swap;                  /* Whether the red and blue componets are swapped. RGBA -> BGRA */
	mali_bool reverse_order;                  /* Whether the components have reverse order. RGBA -> ABGR */
	mali_bool premultiplied_alpha;            /* Whether alpha component is premultiplied in color components or not */
	mali_surface_colorspace colorspace;       /* Which color space the surface color components resides */
	mali_bool alpha_to_one;                   /* Whether alpha component is to be expanded to ones */
} mali_surface_specifier;

/**
 * Fills out a surface format structure
 * @param output The struct to fill out. This is a pure output parameter.
 * @param width  The width of the surface in pixels.
 * @param height The height of the surface in pixels.
 * @param pitch  The size of one pixel row in bytes, or 0 to compute from the width and format. Must be 0 for non-linear layouts
 * @param pformat The required pixel format
 * @param tformat The required texel format
 * @param playoutThe required pixel layout
 * @param tlayout The required texel layout
 * @param rbswap The red/blue swap flag for this format
 * @param revorder The reverse order flag for this format
 * @param alpha_to_one Indicates whether to expand alpha component to one.
 * NOTE: This function will always specify all fields in the surface format struct given.
 */
MALI_STATIC_INLINE void _mali_surface_specifier_ex(  mali_surface_specifier* output,
													 u16 width, u16 height, u16 pitch,
													 mali_pixel_format pformat, m200_texel_format tformat,
													 mali_pixel_layout playout, m200_texture_addressing_mode tlayout,
													 mali_bool rbswap, mali_bool revorder,
													 mali_surface_colorspace colorspace, mali_surface_alphaformat alpha_format, mali_bool alpha_to_one )
{
	output->width = width;
	output->height = height;
	output->pitch = pitch;
	output->pixel_format = pformat;
	output->texel_format = tformat;
	output->pixel_layout = playout;
	output->texel_layout = tlayout;
	output->red_blue_swap = rbswap;
	output->reverse_order = revorder;
	output->colorspace = colorspace;
	output->premultiplied_alpha = alpha_format;
	output->alpha_to_one = alpha_to_one;
}

/**
 * Also fills out a format structure, but makes a guess on texel format, based on pixel format. Usually, this is good enough
 * @param output The struct to fill out. This is a pure output parameter.
 * @param width  The width of the surface in pixels.
 * @param height The height of the surface in pixels.
 * @param pitch  The size of one pixel row in bytes, or 0 to compute from the width and format. Must be 0 for non-linear layouts
 * @param pformat The required pixel format
 * @param playout The required pixel layout
 * @param rbswap The red/blue swap flag for this format
 * @param revorder The reverse order flag for this format
 * NOTE: This function will always specify all fields in the surface format struct given.
 */
MALI_STATIC_INLINE void _mali_surface_specifier(  mali_surface_specifier* output,
												  u16 width, u16 height, u16 pitch,
												  mali_pixel_format pformat, mali_pixel_layout playout,
												  mali_bool rbswap, mali_bool revorder )
{
	_mali_surface_specifier_ex(output, width, height, pitch,
							   pformat, _mali_pixel_to_texel_format(pformat),
							   playout, _mali_pixel_layout_to_texel_layout(playout), rbswap, revorder,
							   MALI_SURFACE_COLORSPACE_sRGB, MALI_SURFACE_ALPHAFORMAT_NONPRE, MALI_FALSE );
}

/**
 * Clones a mali_surface_specifier
 * @param output The struct to fill out. This is a pure output parameter.
 * @param input  The struct to be copied.
 * NOTE: This function will always specify all fields in the surface format struct given.
 */

MALI_STATIC_INLINE void _mali_surface_specifier_clone(  mali_surface_specifier* output, const mali_surface_specifier* input )
{
	MALI_DEBUG_ASSERT_POINTER(output);
	MALI_DEBUG_ASSERT_POINTER(input);
	_mali_surface_specifier_ex(output, 
							   input->width, input->height, input->pitch,
							   input->pixel_format, input->texel_format,
							   input->pixel_layout, input->texel_layout,
							   input->red_blue_swap, input->reverse_order,
							   input->colorspace,input->premultiplied_alpha?MALI_SURFACE_ALPHAFORMAT_PRE:MALI_SURFACE_ALPHAFORMAT_NONPRE,
							   input->alpha_to_one );
}

/**
 * returns the amount of bits per pixel available in the given surface format
 * @param format The surface format struct on which to calculate bpp size
 * @returns the number of bits per pixel.
 */
MALI_IMPORT u32 _mali_surface_specifier_bpp(const mali_surface_specifier* format);

/**
 * compares two formats for format and layout only
 * @param format_l The left input for compare
 * @param format_r The right input for compare
 * @returns MALI_TRUE if the 2 input formats are the same for both format and layout. MALI_FALSE otherwise.
 */
MALI_STATIC_INLINE mali_bool _mali_surface_specifier_cmp_format_layout(const mali_surface_specifier* format_l, const mali_surface_specifier* format_r)
{
	MALI_DEBUG_ASSERT_POINTER(format_l);
	MALI_DEBUG_ASSERT_POINTER(format_r);
	if(format_l->pixel_format != format_r->pixel_format) return MALI_FALSE;
	if(format_l->texel_format != format_r->texel_format) return MALI_FALSE;
	if(format_l->pixel_layout != format_r->pixel_layout) return MALI_FALSE;
	if(format_l->texel_layout != format_r->texel_layout) return MALI_FALSE;

	return MALI_TRUE;
}

/**
 * compares two formats
 * @param format_l The left input for compare
 * @param format_r The right input for compare
 * @returns MALI_TRUE if the 2 input formats are the same. MALI_FALSE otherwise.
 */
MALI_STATIC_INLINE mali_bool _mali_surface_specifier_cmp(const mali_surface_specifier* format_l, const mali_surface_specifier* format_r)
{
	MALI_DEBUG_ASSERT_POINTER(format_l);
	MALI_DEBUG_ASSERT_POINTER(format_r);
	if(_mali_surface_specifier_cmp_format_layout(format_l, format_r)==MALI_FALSE) return MALI_FALSE;
	if(format_l->red_blue_swap != format_r->red_blue_swap) return MALI_FALSE;
	if(format_l->reverse_order != format_r->reverse_order) return MALI_FALSE;
	if(format_l->premultiplied_alpha != format_r->premultiplied_alpha) return MALI_FALSE;
	if(format_l->colorspace != format_r->colorspace) return MALI_FALSE;

	return MALI_TRUE;
}

#endif /* MALI_SURFACE_SPECIFIER_H */
