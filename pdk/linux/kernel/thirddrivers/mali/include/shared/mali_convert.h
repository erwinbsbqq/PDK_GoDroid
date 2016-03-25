/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2007, 2009-2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
#ifndef MALI_CONVERT_H
#define MALI_CONVERT_H

#include "mali_system.h"
#include "shared/mali_surface_specifier.h"
#include "m200_texel_format.h"

struct mali_surface;

typedef struct
{
	u32 sx, sy, dx, dy, width, height;
} mali_convert_rectangle;

typedef struct
{
	const void *src_ptr;
	void *dst_ptr;
	void *dst_nonpre_ptr;

	s32 src_pitch;
	s32 dst_pitch;
	s32 dst_nonpre_pitch;

	mali_surface_specifier src_format;
	mali_surface_specifier dst_format;

	mali_convert_rectangle rect;

	mali_bool src_is_malimem;
	mali_bool dst_is_malimem;

	mali_bool alpha_clamp;
} mali_convert_request;

enum mali_convert_pixel_format
{
	/* 16 bit addressed formats */
	MALI_CONVERT_PIXEL_FORMAT_R5G6B5,
	MALI_CONVERT_PIXEL_FORMAT_R4G4B4A4,
	MALI_CONVERT_PIXEL_FORMAT_R5G5B5A1,

	/* 16 bit per component formats */
	MALI_CONVERT_PIXEL_FORMAT_L16A16,
	MALI_CONVERT_PIXEL_FORMAT_R16G16B16A16,

	/* byte addressed formats */
	MALI_CONVERT_PIXEL_FORMAT_R8G8B8,
	MALI_CONVERT_PIXEL_FORMAT_R8G8B8A8,
	MALI_CONVERT_PIXEL_FORMAT_L8,
	MALI_CONVERT_PIXEL_FORMAT_L8A8,
	MALI_CONVERT_PIXEL_FORMAT_A8
};

typedef enum mali_convert_method
{
	MALI_CONVERT_8BITS,
	MALI_CONVERT_16BITS,
	MALI_CONVERT_PACKED
} mali_convert_method;

/**
 * Get the number of bytes required to store the given format.
 *
 * @param format The format for which we want to obtain the size in bytes.
 *
 * @return The size of the format in bytes.
 */
MALI_IMPORT int _mali_convert_pixel_format_get_size(
	enum mali_convert_pixel_format format);

/**
 * Get the method to be used to convert from the given format.
 *
 * @param format The format for which we want to obtain the size in bytes.
 *
 * @return The method to be used.
 */
MALI_IMPORT mali_convert_method _mali_convert_pixel_format_get_convert_method(
	enum mali_convert_pixel_format format);

/* 8bit stuff */
MALI_IMPORT void _mali_convert_get_8bit_byte_indices(
	int index[4],
	enum mali_convert_pixel_format format);

/* 16bit stuff */
MALI_IMPORT void _mali_convert_get_16bit_shifts(
	int shift[4],
	enum mali_convert_pixel_format format);

MALI_IMPORT void _mali_convert_get_16bit_component_size(
	int size[4],
	enum mali_convert_pixel_format format);

MALI_IMPORT void _mali_convert_8bit_to_rgba8888(
	u8 *dst,
	const u8 *src,
	int count,
	enum mali_convert_pixel_format src_format);

MALI_IMPORT void _mali_convert_16bit_to_rgba8888(
	u8 *dst,
	const u16 *src,
	int count,
	enum mali_convert_pixel_format src_format);

MALI_IMPORT void _mali_convert_rgba8888_to_8bit(
	u8 *dst,
	const u8 *src,
	int count,
	enum mali_convert_pixel_format src_format);

MALI_IMPORT void _mali_convert_rgba8888_to_16bit(
	u16 *dst,
	const u8 *src,
	int count,
	enum mali_convert_pixel_format src_format);

/**
 * Convert a 32bit IEEE floating point to its binary representation
 * @param f The floating point to convert to its binary representation
 * @return A 32 bit unsigned integer containing the binary representation of
 * the floating point
 */
MALI_STATIC_FORCE_INLINE u32 _mali_convert_fp32_to_binary(float f)
{
	union {
		float f;
		u32   i;
	} v;
	v.f = f;
	return v.i;
}

/**
 * 8 bit linear to block interleaved conversion.
 * 
 * @param dst
 * @param src
 * @param width - Unaligned texture width
 * @param height - Unaligned texture height
 * @param src_pitch - Source pitch
 */
void _mali_convert_tex8_l_to_tex8_b(
	u8*               dest,
	const u8*         src,
	u32               width,
	u32               height,
	s32               src_pitch );


/**
 * 16 bit linear to block interleaved conversion.
 *
 * @param dst
 * @param src
 * @param width - Unaligned texture width
 * @param height - Unaligned texture height
 * @param src_pitch - Source pitch
 */
void _mali_convert_tex16_l_to_tex16_b(
	u8*               dest,
	const u8*         src,
	u32               width,
	u32               height,
	s32               src_pitch );

/**
 * 24 bit linear to block interleaved conversion.
 *
 * @param dst
 * @param src
 * @param width - Unaligned texture width
 * @param height - Unaligned texture height
 * @param src_pitch - Source pitch
 */
void _mali_convert_tex24_l_to_tex24_b(
	u8*               dest,
	const u8*         src,
	u32               width,
	u32               height,
	s32               src_pitch );

/**
 * 32 bit linear to block interleaved conversion.
 *
 * @param dst
 * @param src
 * @param width - Unaligned texture width
 * @param height - Unaligned texture height
 * @param src_pitch - Source pitch
 */
void _mali_convert_tex32_l_to_tex32_b(
	u8*               dest,
	const u8*         src,
	u32               width,
	u32               height,
	s32               src_pitch );

/**
 * 32 bit linear to block interleaved conversion
 * wich forces alpha to one.
 *
 * @param dst
 * @param dst_nonpre - For normalized storage. Will be ignored if NULL.
 * @param src
 * @param width - Unaligned texture width
 * @param height - Unaligned texture height
 * @param src_pitch - Source pitch
 */
void _mali_convert_tex32_l_to_tex32_b_alpha_to_one(
	u8*               dst,
	u8*               dst_nonpre,
	const u8*         src,
	u32               width,
	u32               height,
	s32               src_pitch,
	mali_bool         rev_order );

/**
 * 32 bit linear to block interleaved conversion with
 * alpha premultiplication and normalized storage
 *
 * @param dst_pre - Storing premultiplied swizzled texture
 * @param dst_nonpre - Storing swizzled original data
 * @param src
 * @param width - Unaligned texture width
 * @param height - Unaligned texture height
 * @param src_pitch - Source pitch
 * @param rev_order - Specifies reverse order (XYZA or AZYX)
 */
void _mali_convert_tex32_l_to_tex32_b_premult(
	u8*          dst_pre,
	u8*          dst_nonpre,
	const u8*    src,
	u32          width,
	u32          height,
	s32          src_pitch,
	mali_bool    rev_order );


/**
 * Conversion function handling all possible mali_surface_specifier conversions.
 * Function is based on src and dst mali_surface_specifiers. It supports any sub-
 * region.
 *
 * @note Slow fallbacks for unaligned src pointers.
 *
 * @param convert_request - Structure having the conversion parameters
 * @return MALI_TRUE if successfully converted, MALI_FALSE otherwise.
 */
MALI_IMPORT mali_bool _mali_convert_texture(
	mali_convert_request *convert_request );

/**
 * Initializes a conversion request.
 *
 * @param convert_request
 * @param dst_ptr
 * @param dst_pitch
 * @param dst_format - The format of the destination buffer
 * @param src_ptr
 * @param src_pitch
 * @param src_format - The format of the original source buffer
 * @param rect - The source and destination conversion areas
 * @param nonpre_dst_ptr - VG specific: An additional nonpre destination buffer for cases where
 *                         both premult and nonpre buffers are needed. If NULL, it is ignored.
 * @param nonpre_dst_pitch - VG specific: Pitch of additional nonpre destination buffer. Ignored if
                         nonpre_dst_ptr is NULL
 * @param alpha_clamp - Do alpha clamping when premultiplying alpha.
 */
MALI_IMPORT void _mali_convert_request_initialize(
	mali_convert_request *convert_request,
	void *dst_ptr,
	s32 dst_pitch,
	const mali_surface_specifier *dst_format,
	const void *src_ptr,
	s32 src_pitch,
	const mali_surface_specifier *src_format,
	void *dst_nonpre_ptr,
	s32 dst_nonpre_pitch,
	const mali_convert_rectangle *rect,
	mali_bool src_is_malimem,
	mali_bool dst_is_malimem,
	mali_bool alpha_clamp );

/**
 * LUT used to speed up conversion of lRGBA to sRGBA (calculated by make_conversion_lut.c)
 */
extern const u8 mali_convert_linear_to_nonlinear_lut[256];


/**
 * LUT used to speed up conversion of sRGBA to lRGBA (calculated by make_conversion_lut.c).
 */
extern const u8 mali_convert_nonlinear_to_linear_lut[256];


/**
 * Divides out the alpha from a premultiplied color component.
 * @param component premultiplied 8-bit color component
 * @param alpha original 8-bit alpha component
 * @return non-premult 8-bit color component
 */
MALI_STATIC_INLINE u32 _mali_convert_from_premult( u32 component, u32 alpha )
{
	return (alpha ? (component*0xFF + (alpha/2)) / alpha : 0);
}


/**
 * premultiply a color component with alpha.
 * @param component 4-bit color component
 * @param alpha 4-bit alpha component
 * @return premultiplied 4-bit color component
 */
MALI_STATIC_INLINE u16 _mali_convert_to_premult16( u32 component, u32 alpha )
{
	/* This is a trick which does ~round(value/15.0f) for inputs in range 0...15*15 */
	u32 ca = component*alpha;
	return (((ca+1)>>4) + ca + 8)>>4;
}

/**
 * premultiply a color component with alpha.
 * @param component 8-bit color component
 * @param alpha 8-bit alpha component
 * @return premultiplied 8-bit color component
 */
MALI_STATIC_INLINE u32 _mali_convert_to_premult( u32 component, u32 alpha )
{
	/* This is a trick which does ~round(value/255.0f) for inputs in range 0...255*255 */
	u32 ca = component*alpha;
	return ((ca>>8) + ca + 128)>>8;
}

/**
 * generic pixel packing for 16-bit 565 format
 */
MALI_STATIC_INLINE u32 _mali_convert_pack_565( u32 a, u32 b , u32 c )
{
	MALI_DEBUG_ASSERT( a <= 0xff && b <= 0xff && c <= 0xff, ("_mali_convert_pack_565 components out of range") );
	return ((a>>3)<<11) | ((b>>2)<<5) | (c>>3);
}


/**
 * generic pixel packing for 16-bit 4444 format
 */
MALI_STATIC_INLINE u32 _mali_convert_pack_4444( u32 a, u32 b, u32 c, u32 d )
{
	MALI_DEBUG_ASSERT( a <= 0xff && b <= 0xff && c <= 0xff && d <= 0xff, ("_mali_convert_pack_4444 components out of range") );
	return ((a>>4) << 12) | ((b>>4) << 8) | ((c>>4) << 4) | ((d)>>4);
}


/**
 * generic pixel packing for 16-bit 1555 format
 */
MALI_STATIC_INLINE u32 _mali_convert_pack_1555( u32 a, u32 b, u32 c, u32 d )
{
	MALI_DEBUG_ASSERT( a <= 0xff && b <= 0xff && c <= 0xff && d <= 0xff, ("_mali_convert_pack_1555 components out of range") );
	return ((b>>3) << 10) | ((c>>3) << 5) | (d>>3) | ( a >= 0x7F ? 1<<15 : 0 );
}

/**
 * generic pixel packing for  16-bit 5551 format
 */
MALI_STATIC_INLINE u32 _mali_convert_pack_5551( u32 a, u32 b, u32 c, u32 d )
{
	MALI_DEBUG_ASSERT( a <= 0xff && b <= 0xff && c <= 0xff && d <= 0xff, ("_mali_convert_pack_5551 components out of range") );
	return ((a>>3) << 11) | ((b>>3) << 6) | ((c>>3) << 1) | ( d >= 0x7F ? 1 : 0 );
}


/**
 * generic pixel packing for 32-bit 8888 format
 */
MALI_STATIC_INLINE u32 _mali_convert_pack_8888( u32 a, u32 b, u32 c, u32 d )
{
	MALI_DEBUG_ASSERT( a <= 0xff && b <= 0xff && c <= 0xff && d <= 0xff, ("_mali_convert_pack_8888 components out of range") );
	return ((a<<24) | (b<<16) | (c<<8) | d);
}


/**
 * Bit replication for expanding 4 bit values to 8-bit.
 * @note The value is replicated.
 * @param a		value to bit-replicate
 */
MALI_STATIC_INLINE u32 _mali_convert_replicate_4_bits( u32 a )
{
	return ((a << 4) | a);
}

/**
 * Bit replication for expanding 5 bit values to 8-bit.
 * @note The 3 MSBs are replicated.
 * @param a		value to bit-replicate
 */
MALI_STATIC_INLINE u32 _mali_convert_replicate_5_bits( u32 a )
{
	return ((a << 3) | (a >> 2));
}

/**
 * Bit replication for expanding 6 bit values to 8-bit.
 * @note The 2 MSBs are replicated.
 * @param a		value to bit-replicate
 */
MALI_STATIC_INLINE u32 _mali_convert_replicate_6_bits( u32 a )
{
	return ((a << 2) | (a >> 4));
}


/**
 * pixel conversion flags
 */
#define MALI_CONVERT_SRC_LINEAR             1
#define MALI_CONVERT_SRC_PREMULT            2
#define MALI_CONVERT_DST_LINEAR             4
#define MALI_CONVERT_DST_PREMULT            8
#define MALI_CONVERT_SRC_LUMINANCE         16
#define MALI_CONVERT_DST_LUMINANCE         32


/**
 * setup conversion rules used when converting a pixel from src to dest format.
 * @param src source image format
 * @param dest destination image format
 * @return conversion rules as bit flags
 */
MALI_IMPORT u32 _mali_convert_setup_conversion_rules( const mali_surface_specifier *src, const mali_surface_specifier *dest );

#define ALPHA_TO_ONE_BIT 2048
#define REVERSE_ORDER_BIT 1024
#define RED_BLUE_SWAP_BIT 512
#define PACK_SURFSPEC( surfspec ) \
        (((surfspec)->texel_format & 255) \
      | (((surfspec)->red_blue_swap & 1) << 9) \
      | (((surfspec)->reverse_order & 1) << 10) \
      | (((surfspec)->alpha_to_one & 1) << 11))

/**
 * Convert 8-bit r,g,b,a color channels to texel in format
 * @param surfspec format of output pixel
 * @return texel in format
 */
MALI_STATIC_INLINE u32 _mali_convert_color_channels_to_texel( const mali_surface_specifier *surfspec, const u32 *color )
{
#define _8_TO_4(x) ((x * 3855 + 32768) >> 16) /* this is equal to floor(x/255.f * 15.f + 0.5f) for x in [0..255] */

	u32 r, g, b, a, texel = 0;
	r = color[0]; g = color[1];
	b = color[2]; a = color[3];

	/* convert to texel */
	switch ( PACK_SURFSPEC( surfspec ) )
	{
		case M200_TEXEL_FORMAT_ARGB_8888|ALPHA_TO_ONE_BIT|RED_BLUE_SWAP_BIT: /* XBGR */
			a = 255;
			/* fall through */
		case M200_TEXEL_FORMAT_ARGB_8888|RED_BLUE_SWAP_BIT: /* ABGR */
			texel = _mali_convert_pack_8888( a, b, g, r );
			break;

		case M200_TEXEL_FORMAT_VIRTUAL_RGB888|RED_BLUE_SWAP_BIT:
		case M200_TEXEL_FORMAT_xRGB_8888|RED_BLUE_SWAP_BIT: /* XBGR */
			texel = _mali_convert_pack_8888( a, b, g, r );
			break;
		case M200_TEXEL_FORMAT_ARGB_8888|REVERSE_ORDER_BIT|ALPHA_TO_ONE_BIT: /* BGRX */
			a = 255;
			/* fall through */
		case M200_TEXEL_FORMAT_ARGB_8888|REVERSE_ORDER_BIT: /* BGRA */
			texel = _mali_convert_pack_8888( b, g, r, a );
			break;

		case M200_TEXEL_FORMAT_ARGB_8888|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT|ALPHA_TO_ONE_BIT: /* RGBX */
			a = 255;
			/* fall through */
		case M200_TEXEL_FORMAT_ARGB_8888|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT: /* RGBA */
			texel = _mali_convert_pack_8888( r, g, b, a );
			break;

		case M200_TEXEL_FORMAT_xRGB_8888: /* XRGB */
		case M200_TEXEL_FORMAT_xRGB_8888|ALPHA_TO_ONE_BIT: /* XRGB */
		case M200_TEXEL_FORMAT_ARGB_8888|ALPHA_TO_ONE_BIT: /* XRGB */
		case M200_TEXEL_FORMAT_VIRTUAL_RGB888: /* RGB is RGBx*/
			a = 255;
			/* fall through */
		case M200_TEXEL_FORMAT_ARGB_8888: /* ARGB */
			texel = _mali_convert_pack_8888( a, r, g, b );
			break;

		case M200_TEXEL_FORMAT_RGB_565|RED_BLUE_SWAP_BIT|ALPHA_TO_ONE_BIT: /* BGR */
		case M200_TEXEL_FORMAT_RGB_565|RED_BLUE_SWAP_BIT:
			texel = _mali_convert_pack_565( b, g, r );
			break;

		case M200_TEXEL_FORMAT_RGB_565|ALPHA_TO_ONE_BIT: /* RGB */
		case M200_TEXEL_FORMAT_RGB_565:
			texel = _mali_convert_pack_565( r, g, b );
			break;

		case M200_TEXEL_FORMAT_ARGB_1555|RED_BLUE_SWAP_BIT: /* ABGR */
			texel = _mali_convert_pack_1555( a, b, g, r );
			break;

		case M200_TEXEL_FORMAT_ARGB_1555|REVERSE_ORDER_BIT: /* BGRA */
			texel = _mali_convert_pack_5551( b, g, r, a );
			break;

		case M200_TEXEL_FORMAT_ARGB_1555: /* ARGB */
			texel = _mali_convert_pack_1555( a, r, g, b );
			break;

		case M200_TEXEL_FORMAT_ARGB_1555|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT: /* RGBA */
			texel = _mali_convert_pack_5551( r, g, b, a );
			break;

		case M200_TEXEL_FORMAT_ARGB_4444|RED_BLUE_SWAP_BIT: /* ABGR */
			a = _8_TO_4(a);
			g = _8_TO_4(g);
			b = _8_TO_4(b);
			r = _8_TO_4(r);
			texel = _mali_convert_pack_4444( a<<4, b<<4, g<<4, r<<4 );
			break;

		case M200_TEXEL_FORMAT_ARGB_4444|REVERSE_ORDER_BIT: /* BGRA */
			a = _8_TO_4(a);
			g = _8_TO_4(g);
			b = _8_TO_4(b);
			r = _8_TO_4(r);
			texel = _mali_convert_pack_4444( b<<4, g<<4, r<<4, a<<4 );
			break;

		case M200_TEXEL_FORMAT_ARGB_4444: /* ARGB */
			a = _8_TO_4(a);
			g = _8_TO_4(g);
			b = _8_TO_4(b);
			r = _8_TO_4(r);
			texel = _mali_convert_pack_4444( a<<4, r<<4, g<<4, b<<4 );
			break;

		case M200_TEXEL_FORMAT_ARGB_4444|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT: /* RGBA */
			a = _8_TO_4(a);
			g = _8_TO_4(g);
			b = _8_TO_4(b);
			r = _8_TO_4(r);
			texel = _mali_convert_pack_4444( r<<4, g<<4, b<<4, a<<4 );
			break;

		case M200_TEXEL_FORMAT_L_8|ALPHA_TO_ONE_BIT:
		case M200_TEXEL_FORMAT_L_8:
			texel = r;
			break;

		case M200_TEXEL_FORMAT_A_8:
		case M200_TEXEL_FORMAT_I_8:
			texel = a;
			break;

		case M200_TEXEL_FORMAT_I_4:
			texel = _8_TO_4(a);
			break;

		case M200_TEXEL_FORMAT_I_1:
			texel = ( a > 127 ? 1 : 0 );
			break;

		case M200_TEXEL_FORMAT_L_1|ALPHA_TO_ONE_BIT:
		case M200_TEXEL_FORMAT_L_1:
			texel = ( r > 127 ? 1 : 0 );
			break;
		default:
			MALI_DEBUG_ASSERT( 0, ("Invalid format for packing color channels to texel: 0x%X\n", PACK_SURFSPEC( surfspec ) ) );
	}

	return texel;
#undef _8_TO_4

}


/**
 * Convert a texel to 8-bit r,g,b,a color channels
 * @note Luminance formats are expanded to rgb = Luminance and alpha = 1 in this function.
 * @param surfspec Surface specifier for this texel conversion
 * @param texel	   Texel to convert
 * @param color    Output color channels [r,g,b,a]
 */
MALI_STATIC_INLINE void _mali_convert_texel_to_color_channels( const mali_surface_specifier *surfspec, u32 texel, u32 *color )
{
	u32 r, g, b, a;

	MALI_DEBUG_ASSERT_POINTER( color );
	MALI_DEBUG_ASSERT_POINTER( surfspec );

	/* unpack texel */
	switch ( PACK_SURFSPEC( surfspec ) )
	{
		case M200_TEXEL_FORMAT_ARGB_8888|ALPHA_TO_ONE_BIT|RED_BLUE_SWAP_BIT: /* XBGR */
			texel |= 0xFF000000;
			/* fall through */
		case M200_TEXEL_FORMAT_xRGB_8888|RED_BLUE_SWAP_BIT|ALPHA_TO_ONE_BIT : /* XRGB */
		case M200_TEXEL_FORMAT_xRGB_8888|RED_BLUE_SWAP_BIT:
		case M200_TEXEL_FORMAT_ARGB_8888|RED_BLUE_SWAP_BIT: /* ABGR */
		case M200_TEXEL_FORMAT_VIRTUAL_RGB888|RED_BLUE_SWAP_BIT:
		case M200_TEXEL_FORMAT_VIRTUAL_RGB888|RED_BLUE_SWAP_BIT|ALPHA_TO_ONE_BIT:
			a = texel >> 24;
			b = (texel >> 16) & 0xFF;
			g = (texel >> 8) & 0xFF;
			r = texel & 0xFF;
			break;
		case M200_TEXEL_FORMAT_ARGB_8888|REVERSE_ORDER_BIT|ALPHA_TO_ONE_BIT: /* BGRX */
			texel |= 0xFF;
			/* fall through */
		case M200_TEXEL_FORMAT_ARGB_8888|REVERSE_ORDER_BIT: /* BGRA */
			b = texel >> 24;
			g = (texel >> 16) & 0xFF;
			r = (texel >> 8) & 0xFF;
			a = texel & 0xFF;
			break;

		case M200_TEXEL_FORMAT_ARGB_8888|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT|ALPHA_TO_ONE_BIT: /* RGBX */
			texel |= 0xFF;
			/* fall through */
		case M200_TEXEL_FORMAT_ARGB_8888|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT: /* RGBA */
			r = texel >> 24;
			g = (texel >> 16) & 0xFF;
			b = (texel >> 8) & 0xFF;
			a = texel & 0xFF;
			break;

		case M200_TEXEL_FORMAT_xRGB_8888: /* XRGB */
		case M200_TEXEL_FORMAT_xRGB_8888|ALPHA_TO_ONE_BIT: /* XRGB */
		case M200_TEXEL_FORMAT_ARGB_8888|ALPHA_TO_ONE_BIT: /* XRGB */
			texel |= 0xFF000000;
			/* fall through */
		case M200_TEXEL_FORMAT_ARGB_8888: /* ARGB */
		case M200_TEXEL_FORMAT_VIRTUAL_RGB888: /* RGB is RGBx*/
		case M200_TEXEL_FORMAT_VIRTUAL_RGB888|ALPHA_TO_ONE_BIT: 
			a = texel >> 24;
			r = (texel >> 16) & 0xFF;
			g = (texel >> 8) & 0xFF;
			b = texel & 0xFF;
			break;

		case M200_TEXEL_FORMAT_RGB_565|RED_BLUE_SWAP_BIT:
		case M200_TEXEL_FORMAT_RGB_565|ALPHA_TO_ONE_BIT|RED_BLUE_SWAP_BIT: /* BGR */
			b = _mali_convert_replicate_5_bits( (texel >> 11) & 0x1F );
			g = _mali_convert_replicate_6_bits( (texel >> 5) & 0x3F );
			r = _mali_convert_replicate_5_bits( texel & 0x1F );
			a = 255;
			break;

		case M200_TEXEL_FORMAT_RGB_565|ALPHA_TO_ONE_BIT: /* RGB */
		case M200_TEXEL_FORMAT_RGB_565:
			r = _mali_convert_replicate_5_bits( (texel >> 11) & 0x1F );
			g = _mali_convert_replicate_6_bits( (texel >> 5) & 0x3F );
			b = _mali_convert_replicate_5_bits( texel & 0x1F );
			a = 255;
			break;

		case M200_TEXEL_FORMAT_ARGB_1555|RED_BLUE_SWAP_BIT|ALPHA_TO_ONE_BIT: /* ABGR */
			texel |= (1 << 15);
			/* fallthrough */
		case M200_TEXEL_FORMAT_ARGB_1555|RED_BLUE_SWAP_BIT: /* ABGR */
			a = (u8)((texel >> 15) * 255);
			b = _mali_convert_replicate_5_bits( (texel >> 10) & 0x1F );
			g = _mali_convert_replicate_5_bits( (texel >> 5) & 0x1F );
			r = _mali_convert_replicate_5_bits( texel & 0x1F );
			break;

		case M200_TEXEL_FORMAT_ARGB_1555|REVERSE_ORDER_BIT|ALPHA_TO_ONE_BIT: /* BGRA */
			texel |= 0x1;
			/* fallthrough */
		case M200_TEXEL_FORMAT_ARGB_1555|REVERSE_ORDER_BIT: /* BGRA */
			b = _mali_convert_replicate_5_bits( (texel >> 11) & 0x1F );
			g = _mali_convert_replicate_5_bits( (texel >> 6) & 0x1F );
			r = _mali_convert_replicate_5_bits( (texel >> 1) & 0x1F );
			a = (texel & 0x1) * 255;
			break;

		case M200_TEXEL_FORMAT_ARGB_1555|ALPHA_TO_ONE_BIT: /* RGBA */
			texel |= (1 << 15);
			/* fallthrough */
		case M200_TEXEL_FORMAT_ARGB_1555: /* ARGB */
			a = (u8)((texel >> 15) * 255);
			r = _mali_convert_replicate_5_bits( (texel >> 10) & 0x1F );
			g = _mali_convert_replicate_5_bits( (texel >> 5) & 0x1F );
			b = _mali_convert_replicate_5_bits( texel & 0x1F );
			break;

		case M200_TEXEL_FORMAT_ARGB_1555|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT|ALPHA_TO_ONE_BIT: /* RGBA */
			texel |= 0x1;
			/* fallthrough */
		case M200_TEXEL_FORMAT_ARGB_1555|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT: /* RGBA */
			r = _mali_convert_replicate_5_bits( (texel >> 11) & 0x1F );
			g = _mali_convert_replicate_5_bits( (texel >> 6) & 0x1F );
			b = _mali_convert_replicate_5_bits( (texel >> 1) & 0x1F );
			a = (texel & 0x1) * 255;
			break;

		case M200_TEXEL_FORMAT_ARGB_4444|RED_BLUE_SWAP_BIT|ALPHA_TO_ONE_BIT: /* ABGR */
			texel |= (0x0F<<12);
			/* fallthrough */
		case M200_TEXEL_FORMAT_ARGB_4444|RED_BLUE_SWAP_BIT: /* ABGR */
			a = _mali_convert_replicate_4_bits( (texel >> 12) & 0x0F );
			b = _mali_convert_replicate_4_bits( (texel >> 8) & 0x0F );
			g = _mali_convert_replicate_4_bits( (texel >> 4) & 0x0F );
			r = _mali_convert_replicate_4_bits( texel & 0x0F );
			break;

		case M200_TEXEL_FORMAT_ARGB_4444|REVERSE_ORDER_BIT|ALPHA_TO_ONE_BIT: /* BGRA */
			texel |= 0x0F;
			/* fallthrough */
		case M200_TEXEL_FORMAT_ARGB_4444|REVERSE_ORDER_BIT: /* BGRA */
			b = _mali_convert_replicate_4_bits( (texel >> 12) & 0x0F );
			g = _mali_convert_replicate_4_bits( (texel >> 8) & 0x0F );
			r = _mali_convert_replicate_4_bits( (texel >> 4) & 0x0F );
			a = _mali_convert_replicate_4_bits( texel & 0x0F );
			break;

		case M200_TEXEL_FORMAT_ARGB_4444|ALPHA_TO_ONE_BIT: /* RGBA */
			texel |= (0x0F<<12);
			/* fallthrough */
		case M200_TEXEL_FORMAT_ARGB_4444: /* ARGB */
			a = _mali_convert_replicate_4_bits( (texel >> 12) & 0x0F );
			r = _mali_convert_replicate_4_bits( (texel >> 8) & 0x0F );
			g = _mali_convert_replicate_4_bits( (texel >> 4) & 0x0F );
			b = _mali_convert_replicate_4_bits( texel & 0x0F );
			break;

		case M200_TEXEL_FORMAT_ARGB_4444|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT|ALPHA_TO_ONE_BIT: /* RGBA */
			texel |= 0x0F;
			/* fallthrough */
		case M200_TEXEL_FORMAT_ARGB_4444|REVERSE_ORDER_BIT|RED_BLUE_SWAP_BIT: /* RGBA */
			r = _mali_convert_replicate_4_bits( (texel >> 12) & 0x0F );
			g = _mali_convert_replicate_4_bits( (texel >> 8) & 0x0F );
			b = _mali_convert_replicate_4_bits( (texel >> 4) & 0x0F );
			a = _mali_convert_replicate_4_bits( texel & 0x0F );
			break;

		case M200_TEXEL_FORMAT_L_8|ALPHA_TO_ONE_BIT:
		case M200_TEXEL_FORMAT_L_8:
			MALI_DEBUG_ASSERT( texel <= 0xFF, ("This is not an 8-bit luminance value: %X\n", texel) );
			r = g = b = texel & 0xFF;
			a = 0xFF;
			break;

		case M200_TEXEL_FORMAT_A_8:
			MALI_DEBUG_ASSERT( texel <= 0xFF, ("This is not an 8-bit alpha value: %X\n", texel) );
			r = g = b = 0;
			a = texel & 0xFF;
			break;

		case M200_TEXEL_FORMAT_I_8:
		case M200_TEXEL_FORMAT_I_8|ALPHA_TO_ONE_BIT:
			MALI_DEBUG_ASSERT( texel <= 0xFF, ("This is not an 8-bit intensity value: %X\n", texel) );
			r = g = b = 0xFF;
			a = texel & 0xFF;
			break;

		case M200_TEXEL_FORMAT_I_4:
			MALI_DEBUG_ASSERT( texel <= 0x0F, ("This is not a 4-bit intensity value: %X\n", texel) );
			r = g = b = 0xFF;
			a = _mali_convert_replicate_4_bits( texel & 0x0F );
			break;

		case M200_TEXEL_FORMAT_I_1:
			MALI_DEBUG_ASSERT( texel <= 1, ("This is not a 1-bit intensity value: %X\n", texel) );
			r = g = b = 0xFF;
			a = (texel & 1) * 255;
			break;

		case M200_TEXEL_FORMAT_L_1|ALPHA_TO_ONE_BIT:
		case M200_TEXEL_FORMAT_L_1:
			r = g = b = (texel & 0x1) * 255;
			a = 0xFF;
			break;

		default:
			r = g = b = a = 0;
			MALI_DEBUG_ASSERT( 0, ("Invalid unpack format 0x%X\n", PACK_SURFSPEC( surfspec ) ) );
	}

	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = a;

#undef ALPHA_TO_ONE_BIT
#undef REVERSE_ORDER_BIT
#undef RED_BLUE_SWAP_BIT

}
#undef PACK_SURFSPEC

/**
 * Convert texel in src format to dest format
 * @param dest			Format of destination texel
 * @param conv_rules	texel conversion rules (use _vg200_setup_convertion_rules() to setup rules)
 * @param color         Output converted texel.
 */
MALI_STATIC_INLINE void _mali_convert_texel_internal( const mali_surface_specifier *dest, u32 conv_rules, u32 *color )
{
	u32 r, g, b, a, lum;

	r = color[0]; g = color[1];
	b = color[2]; a = color[3];

#define MC_MIN( a, b ) ((a) > (b) ? (b) : (a))

	/* normalize src to sRGBA */
	if ( conv_rules & MALI_CONVERT_SRC_PREMULT )
	{
		/* un-premultiply */
		r = _mali_convert_from_premult( MC_MIN( r, a ), a );
		g = _mali_convert_from_premult( MC_MIN( g, a ), a );
		b = _mali_convert_from_premult( MC_MIN( b, a ), a );
	}

	if ( M200_TEXEL_FORMAT_ARGB_1555 == (dest->texel_format) )
	{
		a = ( a > 127 ? 255 : 0 );
	}
	/* convert to destination format */
	if ( MALI_CONVERT_DST_LUMINANCE == (conv_rules & (MALI_CONVERT_DST_LUMINANCE|MALI_CONVERT_SRC_LUMINANCE)) )
	{
		if ( !(conv_rules & MALI_CONVERT_SRC_LINEAR) )
		{
			/* sRGB -> linear */
			r = mali_convert_nonlinear_to_linear_lut[r];
			g = mali_convert_nonlinear_to_linear_lut[g];
			b = mali_convert_nonlinear_to_linear_lut[b];
		}

		/* linear -> lL. See 3.4.2 Color space defininitions in the OpenVG 1.0.1 spec for an explanation of the three constants. */
		lum = (r*54 + g*182 + b*19) / 255;

		if ( !(conv_rules & MALI_CONVERT_DST_LINEAR) && (M200_TEXEL_FORMAT_L_1 != dest->texel_format) )
		{
			/* lL -> sL */
			lum = mali_convert_linear_to_nonlinear_lut[lum];
		}
		r = g = b = lum;
		a = 255;
	}
	else
	{
		if ( MALI_CONVERT_SRC_LINEAR == (conv_rules & (MALI_CONVERT_SRC_LINEAR | MALI_CONVERT_DST_LINEAR)) )
		{
			/* linear -> sRGB */
			r = mali_convert_linear_to_nonlinear_lut[r];
			g = mali_convert_linear_to_nonlinear_lut[g];
			b = mali_convert_linear_to_nonlinear_lut[b];
		}
		else if ( MALI_CONVERT_DST_LINEAR == (conv_rules & (MALI_CONVERT_SRC_LINEAR | MALI_CONVERT_DST_LINEAR)) )
		{
			/* sRGB -> linear */
			r = mali_convert_nonlinear_to_linear_lut[r];
			g = mali_convert_nonlinear_to_linear_lut[g];
			b = mali_convert_nonlinear_to_linear_lut[b];
		}

		if ( conv_rules & MALI_CONVERT_DST_PREMULT )
		{
			/* premultiply */
			r = _mali_convert_to_premult( r, a );
			g = _mali_convert_to_premult( g, a );
			b = _mali_convert_to_premult( b, a );
		}
	}
#undef MC_MIN
	color[0] = r; color[1] = g;
	color[2] = b; color[3] = a;
}


/**
 * Convert texel in src format to dest format
 * @param src           Source texel format
 * @param dest          Destination texel format
 * @param texel         Texel in src format
 * @param conv_rules    Texel conversion rules (use _mali_convert_setup_convertion_rules() to setup rules)
 * @return              Texel converted to dest format
 */
MALI_STATIC_INLINE u32 _mali_convert_texel( const mali_surface_specifier *src, const mali_surface_specifier *dest, u32 texel, u32 conv_rules )
{
	u32 color[4];
	color[0] = color[1] = color[2] = color[3] = 0;

	/* unpack texel from src format to 8-bit color channels */
	_mali_convert_texel_to_color_channels( src, texel, color );

	_mali_convert_texel_internal(dest, conv_rules, color );

	/* pack channels to dest format */
	return  _mali_convert_color_channels_to_texel( dest, color );
}

/* LUT for linear to block-interleaved indexing */
extern const u8 mali_convert_block_interleave_lut[16 * 16];

/** calculate start address of a 16x16 block */
#define MALI_CONVERT_BLOCK_START_ADDR( _X, _Y, _WIDTH_ALIGNED ) \
	((((_Y)/16)*((_WIDTH_ALIGNED)/16) + ((_X)/16)) * 16*16)


/** calculate offset within a 16x16 block */
#define MALI_CONVERT_BLOCK_OFFSET( _X, _Y )  \
	(mali_convert_block_interleave_lut[(((_Y)%16)*16) + ((_X)%16)])


/** calculate 16x16 blocked address */
#define MALI_CONVERT_BLOCKED_ADDRESS( _X, _Y, _WIDTH_ALIGNED ) \
	(MALI_CONVERT_BLOCK_START_ADDR(_X,_Y,_WIDTH_ALIGNED) + MALI_CONVERT_BLOCK_OFFSET(_X, _Y))


/** check that a block address are within texture bounds */
#define MALI_CONVERT_CHECK_BLOCK_ADDR( _ADDR, _WIDTH, _HEIGHT, _X, _Y ) \
	MALI_DEBUG_ASSERT( ((_ADDR) < (MALI_ALIGN(_WIDTH, 16)*MALI_ALIGN(_HEIGHT, 16))), ("block address %d out-of-bounds %d. size %dx%d, pos %dx%d", _ADDR, MALI_ALIGN(_WIDTH, 16)*MALI_ALIGN(_HEIGHT, 16), (_WIDTH), (_HEIGHT), (_X), (_Y)))

#endif /* MALI_CONVERT_H */
