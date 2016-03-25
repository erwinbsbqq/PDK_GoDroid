/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2006-2007, 2009-2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef __M200_TD_HEADER__
#define __M200_TD_HEADER__

#include <mali_system.h>
#include "m200_texel_format.h"

#ifdef __cplusplus
extern "C" {
#endif


/** size of Mali 200 texture descriptor in 32-bit words */
#define M200_TD_SIZE (512/32)
/** Expands the given parameters to the ones needed by _m200_td_set, done like this to be calculated in compile-time and to avoid
 *  too complicated usage of function:
 *    clear_mask The mask used to clear the field we are setting a value for
 *    index Which word we are to change
 *    field The mask (starting from 0), used to assert-check that we are not setting anything that will affect other values in the word
 *    left_index Passing of the left_index
 *    right_index Passing of the right_index
 *    right_index_small How many bits to right-shift in this word
 */
#define MALI_TD_SET_PARAMETER_EXPAND( left_index, right_index ) ~( ( ( 1 << ( left_index - right_index + 1 ) ) - 1 ) << ( right_index - ( ( right_index >> 5 ) << 5  ) ) ), ( left_index >> 5 ), ( ( 1 << ( left_index - right_index + 1 ) ) - 1 ), left_index, right_index, ( right_index - ( ( right_index >> 5 ) << 5  ) )

/**
 * @brief
These macros have been generated from the functions that are defined at the bottom of this file
 *
 */
#define MALI_TD_SET_TEXEL_FORMAT( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 5 >> 5 ) == ( 0 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 0, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffffffc0 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 0 ] = ( (dest)[ 0 ] & 0xffffffc0 ) | ( (val) << 0 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 5, 0 ) == (val), ( "Value set in MALI_TD_SET_TEXEL_FORMAT is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXEL_ORDER_INVERT( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 6 >> 5 ) == ( 6 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 0, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 0 ] = ( (dest)[ 0 ] & 0xffffffbf ) | ( (val) << 6 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 6, 6 ) == (val), ( "Value set in MALI_TD_SET_TEXEL_ORDER_INVERT is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXEL_RED_BLUE_SWAP( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 7 >> 5 ) == ( 7 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 0, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 0 ] = ( (dest)[ 0 ] & 0xffffff7f ) | ( (val) << 7 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 7, 7 ) == (val), ( "Value set in MALI_TD_SET_TEXEL_RED_BLUE_SWAP is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXEL_BIAS_SELECT( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 9 >> 5 ) == ( 8 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 0, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffc & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 0 ] = ( (dest)[ 0 ] & 0xfffffcff ) | ( (val) << 8 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 9, 8 ) == (val), ( "Value set in MALI_TD_SET_TEXEL_BIAS_SELECT is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXEL_TOGGLE_MSB( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 10 >> 5 ) == ( 10 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 0, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 0 ] = ( (dest)[ 0 ] & 0xfffffbff ) | ( (val) << 10 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 10, 10 ) == (val), ( "Value set in MALI_TD_SET_TEXEL_TOGGLE_MSB is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_PALETTE_FORMAT( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 12 >> 5 ) == ( 11 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 0, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffc & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 0 ] = ( (dest)[ 0 ] & 0xffffe7ff ) | ( (val) << 11 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 12, 11 ) == (val), ( "Value set in MALI_TD_SET_PALETTE_FORMAT is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_PALETTE_ADDRESS( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 38 >> 5 ) - 1 ) == ( 13 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 1, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 0, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 0 ] = ( (dest)[ 0 ] & 0x00001fff ) | ( ( (val) & 0x0007ffff ) << 13 ); \
	(dest)[ 1 ] = ( (dest)[ 1 ] & 0xffffff80 ) | ( (val) >> 19 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 38, 13 ) == (val), ( "Value set in MALI_TD_SET_PALETTE_ADDRESS is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXTURE_FORMAT( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 41 >> 5 ) == ( 39 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 1, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffff8 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 1 ] = ( (dest)[ 1 ] & 0xfffffc7f ) | ( (val) << 7 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 41, 39 ) == (val), ( "Value set in MALI_TD_SET_TEXTURE_FORMAT is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXTURE_DIMENSIONALITY( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 43 >> 5 ) == ( 42 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 1, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffc & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 1 ] = ( (dest)[ 1 ] & 0xfffff3ff ) | ( (val) << 10 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 43, 42 ) == (val), ( "Value set in MALI_TD_SET_TEXTURE_DIMENSIONALITY is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_LAMBDA_LOW_CLAMP( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 51 >> 5 ) == ( 44 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 1, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffffff00 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 1 ] = ( (dest)[ 1 ] & 0xfff00fff ) | ( (val) << 12 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 51, 44 ) == (val), ( "Value set in MALI_TD_SET_LAMBDA_LOW_CLAMP is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_LAMBDA_HIGH_CLAMP( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 59 >> 5 ) == ( 52 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 1, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffffff00 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 1 ] = ( (dest)[ 1 ] & 0xf00fffff ) | ( (val) << 20 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 59, 52 ) == (val), ( "Value set in MALI_TD_SET_LAMBDA_HIGH_CLAMP is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_LAMBDA_BIAS( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 68 >> 5 ) - 1 ) == ( 60 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 1, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffe00 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 1 ] = ( (dest)[ 1 ] & 0x0fffffff ) | ( ( (val) & 0x0000000f ) << 28 ); \
	(dest)[ 2 ] = ( (dest)[ 2 ] & 0xffffffe0 ) | ( (val) >> 4 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 68, 60 ) == (val), ( "Value set in MALI_TD_SET_LAMBDA_BIAS is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_ANISOTROPY_LOG2( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 71 >> 5 ) == ( 69 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffff8 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 2 ] = ( (dest)[ 2 ] & 0xffffff1f ) | ( (val) << 5 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 71, 69 ) == (val), ( "Value set in MALI_TD_SET_ANISOTROPY_LOG2 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAPPING_MODE( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 74 >> 5 ) == ( 73 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffc & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 2 ] = ( (dest)[ 2 ] & 0xfffff9ff ) | ( (val) << 9 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 74, 73 ) == (val), ( "Value set in MALI_TD_SET_MIPMAPPING_MODE is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_POINT_SAMPLE_MINIFY( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 75 >> 5 ) == ( 75 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 2 ] = ( (dest)[ 2 ] & 0xfffff7ff ) | ( (val) << 11 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 75, 75 ) == (val), ( "Value set in MALI_TD_SET_POINT_SAMPLE_MINIFY is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_POINT_SAMPLE_MAGNIFY( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 76 >> 5 ) == ( 76 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 2 ] = ( (dest)[ 2 ] & 0xffffefff ) | ( (val) << 12 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 76, 76 ) == (val), ( "Value set in MALI_TD_SET_POINT_SAMPLE_MAGNIFY is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_WRAP_MODE_S( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 79 >> 5 ) == ( 77 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffff8 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 2 ] = ( (dest)[ 2 ] & 0xffff1fff ) | ( (val) << 13 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 79, 77 ) == (val), ( "Value set in MALI_TD_SET_WRAP_MODE_S is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_WRAP_MODE_T( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 82 >> 5 ) == ( 80 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffff8 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 2 ] = ( (dest)[ 2 ] & 0xfff8ffff ) | ( (val) << 16 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 82, 80 ) == (val), ( "Value set in MALI_TD_SET_WRAP_MODE_T is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_WRAP_MODE_R( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 85 >> 5 ) == ( 83 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffff8 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 2 ] = ( (dest)[ 2 ] & 0xffc7ffff ) | ( (val) << 19 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 85, 83 ) == (val), ( "Value set in MALI_TD_SET_WRAP_MODE_R is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXTURE_DIMENSION_S( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 98 >> 5 ) - 1 ) == ( 86 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 3, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffffe000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 2 ] = ( (dest)[ 2 ] & 0x003fffff ) | ( ( (val) & 0x000003ff ) << 22 ); \
	(dest)[ 3 ] = ( (dest)[ 3 ] & 0xfffffff8 ) | ( (val) >> 10 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 98, 86 ) == (val), ( "Value set in MALI_TD_SET_TEXTURE_DIMENSION_S is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXTURE_DIMENSION_T( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 111 >> 5 ) == ( 99 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 3, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffffe000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 3 ] = ( (dest)[ 3 ] & 0xffff0007 ) | ( (val) << 3 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 111, 99 ) == (val), ( "Value set in MALI_TD_SET_TEXTURE_DIMENSION_T is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXTURE_DIMENSION_R( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 124 >> 5 ) == ( 112 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 3, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffffe000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 3 ] = ( (dest)[ 3 ] & 0xe000ffff ) | ( (val) << 16 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 124, 112 ) == (val), ( "Value set in MALI_TD_SET_TEXTURE_DIMENSION_R is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_BORDER_COLOR_RED( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 140 >> 5 ) - 1 ) == ( 125 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 4, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 3, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffff0000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 3 ] = ( (dest)[ 3 ] & 0x1fffffff ) | ( ( (val) & 0x00000007 ) << 29 ); \
	(dest)[ 4 ] = ( (dest)[ 4 ] & 0xffffe000 ) | ( (val) >> 3 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 140, 125 ) == (val), ( "Value set in MALI_TD_SET_BORDER_COLOR_RED is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_BORDER_COLOR_GREEN( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 156 >> 5 ) == ( 141 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 4, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffff0000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 4 ] = ( (dest)[ 4 ] & 0xe0001fff ) | ( (val) << 13 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 156, 141 ) == (val), ( "Value set in MALI_TD_SET_BORDER_COLOR_GREEN is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_BORDER_COLOR_BLUE( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 172 >> 5 ) - 1 ) == ( 157 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 5, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 4, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffff0000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 4 ] = ( (dest)[ 4 ] & 0x1fffffff ) | ( ( (val) & 0x00000007 ) << 29 ); \
	(dest)[ 5 ] = ( (dest)[ 5 ] & 0xffffe000 ) | ( (val) >> 3 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 172, 157 ) == (val), ( "Value set in MALI_TD_SET_BORDER_COLOR_BLUE is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_BORDER_COLOR_ALPHA( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 188 >> 5 ) == ( 173 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 5, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffff0000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 5 ] = ( (dest)[ 5 ] & 0xe0001fff ) | ( (val) << 13 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 188, 173 ) == (val), ( "Value set in MALI_TD_SET_BORDER_COLOR_ALPHA is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_SHADOW_MAPPING_AMBIENT( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 204 >> 5 ) - 1 ) == ( 189 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 6, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 5, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xffff0000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 5 ] = ( (dest)[ 5 ] & 0x1fffffff ) | ( ( (val) & 0x00000007 ) << 29 ); \
	(dest)[ 6 ] = ( (dest)[ 6 ] & 0xffffe000 ) | ( (val) >> 3 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 204, 189 ) == (val), ( "Value set in MALI_TD_SET_SHADOW_MAPPING_AMBIENT is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXTURE_ADDRESSING_MODE( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 206 >> 5 ) == ( 205 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 6, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffc & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 6 ] = ( (dest)[ 6 ] & 0xffff9fff ) | ( (val) << 13 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 206, 205 ) == (val), ( "Value set in MALI_TD_SET_TEXTURE_ADDRESSING_MODE is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_BORDER_TEXEL_ENABLE( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 207 >> 5 ) == ( 207 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 6, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 6 ] = ( (dest)[ 6 ] & 0xffff7fff ) | ( (val) << 15 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 207, 207 ) == (val), ( "Value set in MALI_TD_SET_BORDER_TEXEL_ENABLE is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_SHADOW_MAPPING_CMP_FUNC( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 210 >> 5 ) == ( 208 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 6, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffff8 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 6 ] = ( (dest)[ 6 ] & 0xfff8ffff ) | ( (val) << 16 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 210, 208 ) == (val), ( "Value set in MALI_TD_SET_SHADOW_MAPPING_CMP_FUNC is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_SHADOW_MAPPING_CPY_RGB( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 211 >> 5 ) == ( 211 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 6, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 6 ] = ( (dest)[ 6 ] & 0xfff7ffff ) | ( (val) << 19 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 211, 211 ) == (val), ( "Value set in MALI_TD_SET_SHADOW_MAPPING_CPY_RGB is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_SHADOW_MAPPING_CPY_ALPHA( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 212 >> 5 ) == ( 212 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 6, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 6 ] = ( (dest)[ 6 ] & 0xffefffff ) | ( (val) << 20 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 212, 212 ) == (val), ( "Value set in MALI_TD_SET_SHADOW_MAPPING_CPY_ALPHA is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_TEXTURE_STACKING_ENABLE( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 213 >> 5 ) == ( 213 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 6, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 6 ] = ( (dest)[ 6 ] & 0xffdfffff ) | ( (val) << 21 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 213, 213 ) == (val), ( "Value set in MALI_TD_SET_TEXTURE_STACKING_ENABLE is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_0( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 247 >> 5 ) - 1 ) == ( 222 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 7, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 6, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 6 ] = ( (dest)[ 6 ] & 0x3fffffff ) | ( ( (val) & 0x00000003 ) << 30 ); \
	(dest)[ 7 ] = ( (dest)[ 7 ] & 0xff000000 ) | ( (val) >> 2 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 247, 222 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_0 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_1( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 273 >> 5 ) - 1 ) == ( 248 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 8, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 7, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 7 ] = ( (dest)[ 7 ] & 0x00ffffff ) | ( ( (val) & 0x000000ff ) << 24 ); \
	(dest)[ 8 ] = ( (dest)[ 8 ] & 0xfffc0000 ) | ( (val) >> 8 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 273, 248 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_1 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_2( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 299 >> 5 ) - 1 ) == ( 274 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 9, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 8, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 8 ] = ( (dest)[ 8 ] & 0x0003ffff ) | ( ( (val) & 0x00003fff ) << 18 ); \
	(dest)[ 9 ] = ( (dest)[ 9 ] & 0xfffff000 ) | ( (val) >> 14 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 299, 274 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_2 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_3( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 325 >> 5 ) - 1 ) == ( 300 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 10, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 9, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 9 ] = ( (dest)[ 9 ] & 0x00000fff ) | ( ( (val) & 0x000fffff ) << 12 ); \
	(dest)[ 10 ] = ( (dest)[ 10 ] & 0xffffffc0 ) | ( (val) >> 20 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 325, 300 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_3 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_4( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 351 >> 5 ) == ( 326 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 10, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 10 ] = ( (dest)[ 10 ] & 0x0000003f ) | ( (val) << 6 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 351, 326 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_4 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_5( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 377 >> 5 ) == ( 352 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 11, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 11 ] = ( (dest)[ 11 ] & 0xfc000000 ) | ( (val) << 0 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 377, 352 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_5 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_6( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 403 >> 5 ) - 1 ) == ( 378 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 12, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 11, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 11 ] = ( (dest)[ 11 ] & 0x03ffffff ) | ( ( (val) & 0x0000003f ) << 26 ); \
	(dest)[ 12 ] = ( (dest)[ 12 ] & 0xfff00000 ) | ( (val) >> 6 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 403, 378 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_6 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_7( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 429 >> 5 ) - 1 ) == ( 404 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 13, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 12, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 12 ] = ( (dest)[ 12 ] & 0x000fffff ) | ( ( (val) & 0x00000fff ) << 20 ); \
	(dest)[ 13 ] = ( (dest)[ 13 ] & 0xffffc000 ) | ( (val) >> 12 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 429, 404 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_7 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_8( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 455 >> 5 ) - 1 ) == ( 430 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 14, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 13, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 13 ] = ( (dest)[ 13 ] & 0x00003fff ) | ( ( (val) & 0x0003ffff ) << 14 ); \
	(dest)[ 14 ] = ( (dest)[ 14 ] & 0xffffff00 ) | ( (val) >> 18 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 455, 430 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_8 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_9( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( ( 481 >> 5 ) - 1 ) == ( 456 >> 5 ), ("Value is split into two words that are not in sequence" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 15, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT_RANGE( 14, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 14 ] = ( (dest)[ 14 ] & 0x000000ff ) | ( ( (val) & 0x00ffffff ) << 8 ); \
	(dest)[ 15 ] = ( (dest)[ 15 ] & 0xfffffffc ) | ( (val) >> 24 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 481, 456 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_9 is different from what was retrieved\n" ) ); \
} while( 0 );
#define MALI_TD_SET_MIPMAP_ADDRESS_10( dest, val ) do { \
	MALI_DEBUG_ASSERT( ( 507 >> 5 ) == ( 482 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 15, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == ( 0xfc000000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[ 15 ] = ( (dest)[ 15 ] & 0xf0000003 ) | ( (val) << 2 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 507, 482 ) == (val), ( "Value set in MALI_TD_SET_MIPMAP_ADDRESS_10 is different from what was retrieved\n" ) ); \
} while( 0 );


/** MALI200 doesn't support texture stride for linear textures
 *  Texture stride will this only be set when target platform is MALI400
 */
#ifdef MALI200_HWVER

#define MALI_TD_SET_TEXTURE_TOGGLE_STRIDE(dest, val)
#define MALI_TD_SET_TEXTURE_STRIDE(dest, val)

#elif defined(MALI400_HWVER)

#define MALI_TD_SET_TEXTURE_TOGGLE_STRIDE(dest, val) do { \
	MALI_DEBUG_ASSERT( ( 72 >> 5 ) == ( 72 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 2, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == (0xfffffffe & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[2] = ( (dest)[ 2 ] & ~0x00000100 ) | ( ( val ) << 8 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 72, 72 ) == (val), ( "Valule set in MALI_TD_SET_TEXTURE_TOGGLE_PITCH is different from what was retrieved\n" ) ); \
} while( 0 );

#define MALI_TD_SET_TEXTURE_STRIDE(dest, val) do { \
	MALI_DEBUG_ASSERT( ( 31 >> 5 ) == ( 16 >> 5 ), ("Value is split into two words, should use split into two functions" ) ); \
	MALI_DEBUG_ASSERT_RANGE( 0, 0, M200_TD_SIZE - 1 ); \
	MALI_DEBUG_ASSERT( 0 == (0xffff0000 & (val) ), ("Bitfield given is too large for the field") ); \
	(dest)[0] = ( (dest)[ 0 ] & 0x0000ffff ) | ( ( val ) << 16 ); \
	MALI_DEBUG_ASSERT( _m200_td_get( (dest), 31, 16 ) == (val), ( "Value set in MALI_TD_SET_TEXTURE_PITCH is different from what was retrieved\n" ) ); \
} while( 0 );

#endif


#define MALI_TD_GET_TEXEL_FORMAT( dest ) _m200_td_get( dest, 5, 0 )
#define MALI_TD_GET_TEXEL_ORDER_INVERT( dest ) _m200_td_get( dest, 6, 6 )
#define MALI_TD_GET_TEXEL_RED_BLUE_SWAP( dest ) _m200_td_get( dest, 7, 7 )
#define MALI_TD_GET_TEXEL_BIAS_SELECT( dest ) _m200_td_get( dest, 9, 8 )
#define MALI_TD_GET_TEXEL_TOGGLE_MSB( dest ) _m200_td_get( dest, 10, 10 )
#define MALI_TD_GET_PALETTE_FORMAT( dest ) _m200_td_get( dest, 12, 11 )
#define MALI_TD_GET_TEXTURE_STRIDE ( dest ) _m200_td_get( dest, 31, 16 )
#define MALI_TD_GET_PALETTE_ADDRESS( dest ) _m200_td_get( dest, 38, 13 )
#define MALI_TD_GET_TEXTURE_FORMAT( dest ) _m200_td_get( dest, 41, 39 )
#define MALI_TD_GET_TEXTURE_DIMENSIONALITY( dest ) _m200_td_get( dest, 43, 42 )
#define MALI_TD_GET_LAMBDA_LOW_CLAMP( dest ) _m200_td_get( dest, 51, 44 )
#define MALI_TD_GET_LAMBDA_HIGH_CLAMP( dest ) _m200_td_get( dest, 59, 52 )
#define MALI_TD_GET_LAMBDA_BIAS( dest ) _m200_td_get( dest, 68, 60 )
#define MALI_TD_GET_ANISOTROPY_LOG2( dest ) _m200_td_get( dest, 71, 69 )
#define MALI_TD_GET_TEXTURE_TOGGLE_STRIDE ( dest ) _m200_td_get( dest, 72, 72 )
#define MALI_TD_GET_MIPMAPPING_MODE( dest ) _m200_td_get( dest, 74, 73 )
#define MALI_TD_GET_POINT_SAMPLE_MINIFY( dest ) _m200_td_get( dest, 75, 75 )
#define MALI_TD_GET_POINT_SAMPLE_MAGNIFY( dest ) _m200_td_get( dest, 76, 76 )
#define MALI_TD_GET_WRAP_MODE_S( dest ) _m200_td_get( dest, 79, 77 )
#define MALI_TD_GET_WRAP_MODE_T( dest ) _m200_td_get( dest, 82, 80 )
#define MALI_TD_GET_WRAP_MODE_R( dest ) _m200_td_get( dest, 85, 83 )
#define MALI_TD_GET_TEXTURE_DIMENSION_S( dest ) _m200_td_get( dest, 98, 86 )
#define MALI_TD_GET_TEXTURE_DIMENSION_T( dest ) _m200_td_get( dest, 111, 99 )
#define MALI_TD_GET_TEXTURE_DIMENSION_R( dest ) _m200_td_get( dest, 124, 112 )
#define MALI_TD_GET_BORDER_COLOR_RED( dest ) _m200_td_get( dest, 140, 125 )
#define MALI_TD_GET_BORDER_COLOR_GREEN( dest ) _m200_td_get( dest, 156, 141 )
#define MALI_TD_GET_BORDER_COLOR_BLUE( dest ) _m200_td_get( dest, 172, 157 )
#define MALI_TD_GET_BORDER_COLOR_ALPHA( dest ) _m200_td_get( dest, 188, 173 )
#define MALI_TD_GET_SHADOW_MAPPING_AMBIENT( dest ) _m200_td_get( dest, 204, 189 )
#define MALI_TD_GET_TEXTURE_ADDRESSING_MODE( dest ) _m200_td_get( dest, 206, 205 )
#define MALI_TD_GET_BORDER_TEXEL_ENABLE( dest ) _m200_td_get( dest, 207, 207 )
#define MALI_TD_GET_SHADOW_MAPPING_CMP_FUNC( dest ) _m200_td_get( dest, 210, 208 )
#define MALI_TD_GET_SHADOW_MAPPING_CPY_RGB( dest ) _m200_td_get( dest, 211, 211 )
#define MALI_TD_GET_SHADOW_MAPPING_CPY_ALPHA( dest ) _m200_td_get( dest, 212, 212 )
#define MALI_TD_GET_TEXTURE_STACKING_ENABLE( dest ) _m200_td_get( dest, 213, 213 )
#define MALI_TD_GET_MIPMAP_ADDRESS_0( dest ) _m200_td_get( dest, 247, 222 )
#define MALI_TD_GET_MIPMAP_ADDRESS_1( dest ) _m200_td_get( dest, 273, 248 )
#define MALI_TD_GET_MIPMAP_ADDRESS_2( dest ) _m200_td_get( dest, 299, 274 )
#define MALI_TD_GET_MIPMAP_ADDRESS_3( dest ) _m200_td_get( dest, 325, 300 )
#define MALI_TD_GET_MIPMAP_ADDRESS_4( dest ) _m200_td_get( dest, 351, 326 )
#define MALI_TD_GET_MIPMAP_ADDRESS_5( dest ) _m200_td_get( dest, 377, 352 )
#define MALI_TD_GET_MIPMAP_ADDRESS_6( dest ) _m200_td_get( dest, 403, 378 )
#define MALI_TD_GET_MIPMAP_ADDRESS_7( dest ) _m200_td_get( dest, 429, 404 )
#define MALI_TD_GET_MIPMAP_ADDRESS_8( dest ) _m200_td_get( dest, 455, 430 )
#define MALI_TD_GET_MIPMAP_ADDRESS_9( dest ) _m200_td_get( dest, 481, 456 )
#define MALI_TD_GET_MIPMAP_ADDRESS_10( dest ) _m200_td_get( dest, 507, 482 )

/** Mali 200 texture descriptor data block */
typedef u32 m200_td[M200_TD_SIZE];

/** If all compare enums are the same (like in 110, we can just create a compare enum instead of this */
typedef enum m200_shadow_mapping_compare_func {
	M200_SHADOW_MAPPING_COMPARE_FUNC_ALWAYS_FAIL = 0,
	M200_SHADOW_MAPPING_COMPARE_FUNC_LESS_THAN = 1,
	M200_SHADOW_MAPPING_COMPARE_FUNC_EQUAL = 2,
	M200_SHADOW_MAPPING_COMPARE_FUNC_LESS_THAN_OR_EQUAL = 3,
	M200_SHADOW_MAPPING_COMPARE_FUNC_GREATER_THAN = 4,
	M200_SHADOW_MAPPING_COMPARE_FUNC_NOT_EQUAL = 5,
	M200_SHADOW_MAPPING_COMPARE_FUNC_GREATER_THAN_OR_EQUAL = 6,
	M200_SHADOW_MAPPING_COMPARE_FUNC_ALWAYS_SUCCEED = 7
} m200_shadow_mapping_compare_function;

/** */
typedef enum m200_texture_format {
	M200_TEXTURE_FORMAT_NORMALIZED = 0,
	M200_TEXTURE_FORMAT_NON_NORMALIZED = 1,
	M200_TEXTURE_FORMAT_NORMALIZED_SHADOW_MAP = 2,
	M200_TEXTURE_FORMAT_NON_NORMALIZED_SHADOW_MAP = 3,
	M200_TEXTURE_FORMAT_CUBE_MAP = 4
	/*Perlin noise not implemented
	M200_TEXTURE_FORMAT_PERLIN_NOISE = 5,
	enums 6-7 reserved for future expansion */
} m200_texture_format;

/** */
typedef enum m200_texel_bias {
	M200_TEXEL_BIAS_NONE = 0,
	M200_TEXEL_BIAS_X_MINUS_HALF = 1,
	M200_TEXEL_BIAS_X_MINUS_HALF_X2 = 2,
	M200_TEXEL_BIAS_ONE_MINUS_X = 3
} m200_texel_bias;

/** */
typedef enum m200_palette_format {
	M200_PALETTE_FORMAT_RGB_565 = 0,
	M200_PALETTE_FORMAT_ARGB_1555 = 1,
	M200_PALETTE_FORMAT_ARGB_4444 = 2
	/* Not implemented
	M200_PALETTE_FORMAT_ARGB_8888 = 3, */
} m200_palette_format;

/** */
typedef enum m200_texture_addressing_mode {
	M200_TEXTURE_ADDRESSING_MODE_LINEAR = 0,
	M200_TEXTURE_ADDRESSING_MODE_2D_INTERLEAVED = 1,
	M200_TEXTURE_ADDRESSING_MODE_3D_INTERLEAVED = 2,
	M200_TEXTURE_ADDRESSING_MODE_16X16_BLOCKED = 3,
	M200_TEXTURE_ADDRESSING_MODE_INVALID = 4
} m200_texture_addressing_mode;

/** */
typedef enum m200_mipmap_mode {
	M200_MIPMAP_MODE_NEAREST = 0,
	M200_MIPMAP_MODE_DITHER = 1,
	M200_MIPMAP_MODE_PERFORMANCE_TRILINEAR = 2,
	M200_MIPMAP_MODE_QUALITY_TRILINEAR = 3
} m200_mipmap_mode;

/** */
typedef enum m200_texture_wrap_mode {
	M200_TEXTURE_WRAP_MODE_REPEAT = 0,
	M200_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE = 1,
	M200_TEXTURE_WRAP_MODE_CLAMP_TO_ZERO_ONE = 2,
	M200_TEXTURE_WRAP_MODE_CLAMP_TO_BORDER = 3,
	M200_TEXTURE_WRAP_MODE_MIRRORED_REPEAT = 4,
	M200_TEXTURE_WRAP_MODE_MIRRORED_CLAMP_TO_EDGE = 5,
	M200_TEXTURE_WRAP_MODE_MIRRORED_CLAMP_TO_MINUS_1_1 = 6,
	M200_TEXTURE_WRAP_MODE_MIRRORED_CLAMP_TO_BORDER = 7
} m200_texture_wrap_mode;

/** */
typedef enum m200_texture_dimensionality {
	M200_TEXTURE_DIMENSIONALITY_1D = 0,
	M200_TEXTURE_DIMENSIONALITY_2D = 1,
	M200_TEXTURE_DIMENSIONALITY_3D = 2
	/* **not implemented** supported for Perlin noise only
	M200_TEXTURE_DIMENSIONALITY_4D = 3
	*/
} m200_texture_dimensionality;

/**
 * Set default values for all fields in texture descriptor.
 * Fields that are not explicitly listed below defaults to zero.
 *	texel_format = M200_TEXEL_FORMAT_xRGB_8888
 *	bias_select = M200_TEXEL_BIAS_NONE
 *	paletted_format = M200_PALETTE_FORMAT_RGB_565
 *	texture_format = M200_TEXTURE_FORMAT_NORMALIZED
 *	texture_dimensionality = M200_TEXTURE_DIMENSIONALITY_2D
 *	mipmapping_mode = M200_MIPMAP_MODE_NEAREST
 *	wrap_mode_s = M200_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE
 *	wrap_mode_t = M200_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE
 *	wrap_mode_r = M200_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE
 *	texture_dimension_s = 1
 *	texture_dimension_t = 1
 *	texture_dimension_r = 1
 *	addressing_mode = M200_TEXTURE_ADDRESSING_MODE_LINEAR
 *	shadowmap_compare_func = M200_SHADOW_MAPPING_COMPARE_FUNC_ALWAYS_FAIL
 * @param dest
 */
MALI_IMPORT void m200_texture_descriptor_set_defaults( m200_td dest );

MALI_IMPORT u32 _m200_td_get(m200_td dest, u32 left_index, u32 right_index );

#ifdef __cplusplus
}
#endif

#ifdef DEBUG
/**
 * print textur descriptor to stdout
 * @param desc tex. desc. to print
 */
MALI_IMPORT void m200_td_print( m200_td dest );
#else
#define m200_td_print( desc )	((void)0)
#endif

#if 0 /* Code needed to regenerate macros for setting and getting */
void print_it( char *name, int left_index, int right_index )
{
	if ( ( left_index >> 5 ) != ( right_index >> 5 ) )
	{
		u32 right_split_index = ( ( left_index >> 5 ) << 5 ) - 1;
		u32 left_split_index = ( ( left_index >> 5 ) << 5 );
		u32 right_clear_mask = ~( ( ( 1 << 	( right_split_index - right_index + 1 ) ) - 1 ) << ( right_index - 	( ( right_index >> 5 ) << 5 ) ) );
		u32 left_clear_mask = ~( ( ( 1 << ( left_index - left_split_index + 1 ) ) - 1 ) );
		u32 right_word = ( right_index >> 5 );
		u32 left_word = ( left_index >> 5 );
		u32 right_field = ( ( 1 << ( right_split_index - right_index + 1 ) ) - 1 );
		u32 left_field = ( ( 1 << ( left_index - left_split_index + 1 ) ) - 1 );
		u32 field = ( ( 1 << ( left_index - right_index + 1 ) ) - 1 );
		u32 right_index_small = ( right_index - ( ( right_index >> 5 ) << 5  ) );
		u32 left_shift_small = right_split_index - right_index + 1;
#if 0
		printf("#ifdef MALI_TEST_API\n");
#endif
		printf("#define MALI_TD_SET_%s( dest, val ) do { \\\n", name );
		printf("\tMALI_DEBUG_ASSERT( ( ( %d >> 5 ) - 1 ) == ( %d >> 5 ), (\"Value is split into two words that are not in sequence\" ) ); \\\n", left_index, right_index );
		printf("\tMALI_DEBUG_ASSERT_RANGE( %d, 0, M200_TD_SIZE - 1 ); \\\n", left_word );
		printf("\tMALI_DEBUG_ASSERT_RANGE( %d, 0, M200_TD_SIZE - 1 ); \\\n", right_word);
		printf("\tMALI_DEBUG_ASSERT( 0 == ( 0x%08x & (val) ), (\"Bitfield given is too large for the field\") ); \\\n", ~field );
		printf("\t(dest)[ %d ] = ( (dest)[ %d ] & 0x%08x ) | ( ( (val) & 0x%08x ) << %d ); \\\n", right_word, right_word, right_clear_mask, right_field, right_index_small );
		printf("\t(dest)[ %d ] = ( (dest)[ %d ] & 0x%08x ) | ( (val) >> %d ); \\\n", left_word, left_word, left_clear_mask, left_shift_small );
		printf("\tMALI_DEBUG_ASSERT( _m200_td_get( (dest), %d, %d ) == (val), ( \"Value set in MALI_TD_SET_%s is different from what was retrieved\\n\" ) ); \\\n", left_index, right_index, name );
		printf("} while( 0 ); \n");
#if 0
		printf("#else\n");
		printf("\t#define MALI_TD_SET_%s( dest, (val) ) \\\n", name );
		printf("\t\t(dest)[ %d ] = ( (dest)[ %d ] & 0x%08x ) | ( ( (val) & 0x%08x ) << %d ) \\\n", right_word, right_word, right_clear_mask, right_field, right_index_small );
		printf("\t\t(dest)[ %d ] = ( (dest)[ %d ] & 0x%08x ) | ( (val) >> %d )\n", left_word, left_word, left_clear_mask, left_shift_small);
		printf("#endif\n");
#endif
	}
	else
	{
		u32 clear_mask = ~( ( ( 1 << ( left_index - right_index + 1 ) ) - 1 ) << ( right_index - ( ( right_index >> 5 ) << 5  ) ) );
		u32 index = 	( left_index >> 5 );
		u32 field = ( ( 1 << ( left_index - right_index + 1 ) ) - 1 );
		u32 right_index_small = ( right_index - ( ( right_index >> 5 ) << 5  ) );
#if 0
		printf("#ifdef MALI_TEST_API\n");
#endif
		printf("#define MALI_TD_SET_%s( dest, val ) do { \\\n", name );
		printf("\tMALI_DEBUG_ASSERT( ( %d >> 5 ) == ( %d >> 5 ), (\"Value is split into two words, should use split into two functions\" ) ); \\\n", left_index, right_index );
		printf("\tMALI_DEBUG_ASSERT_RANGE( %d, 0, M200_TD_SIZE - 1 ); \\\n", index );
		printf("\tMALI_DEBUG_ASSERT( 0 == ( 0x%08x & (val) ), (\"Bitfield given is too large for the field\") ); \\\n", ~field );
		printf("\t(dest)[ %d ] = ( (dest)[ %d ] & 0x%08x ) | ( (val) << %d ); \\\n", index, index, clear_mask, right_index_small );
		printf("\tMALI_DEBUG_ASSERT( _m200_td_get( (dest), %d, %d ) == (val), ( \"Value set in MALI_TD_SET_%s is different from what was retrieved\\n\" ) ); \\\n", left_index, right_index, name );
		printf("} while( 0 ); \n");
#if 0
		printf("#else\n");
		printf("\t#define MALI_TD_SET_%s( dest, (val) ) \\\n", name );
		printf("\t\t(dest)[ %d ] = ( (dest)[ %d ] & 0x%08x ) | ( (val) << %d )\n", index, index, clear_mask, right_index_small );
		printf("#endif\n");
#endif
	}
}
static int used[ 508 ];
static int forste = 1;
void print_it2( char *name, int left_index, int right_index )
{

	int i;

	if ( forste == 1 )
	{
		forste = 0;
		for ( i = 0; i <= 507; i++ )
		{
			used[ i ] = 0;
		}
	}
	for ( i = right_index; i <= left_index; i++ )
	{
		used[ i ]++;
	}
	printf("#define MALI_TD_GET_%s( dest ) _m200_td_get( dest, %d, %d )\n", name, left_index, right_index );
}
void print_all_of_them( )
{
	print_it( "TEXEL_FORMAT", 5, 0 );
	print_it( "TEXEL_ORDER_INVERT", 6, 6 );
	print_it( "TEXEL_RED_BLUE_SWAP", 7, 7 );
	print_it( "TEXEL_BIAS_SELECT", 9, 8 );
	print_it( "TEXEL_TOGGLE_MSB", 10, 10 );
	print_it( "PALETTE_FORMAT", 12, 11 );
	print_it( "TEXTURE_STRIDE", 31, 16 );
	print_it( "PALETTE_ADDRESS", 38, 13 );
	print_it( "TEXTURE_FORMAT", 41, 39 );
	print_it( "TEXTURE_DIMENSIONALITY", 43, 42 );
	print_it( "LAMBDA_LOW_CLAMP", 51, 44 );
	print_it( "LAMBDA_HIGH_CLAMP", 59, 52 );
	print_it( "LAMBDA_BIAS", 68, 60 );
	print_it( "ANISOTROPY_LOG2", 71, 69 );
	print_it( "TEXTURE_TOGGLE_STRIDE", 72, 72 );
	print_it( "MIPMAPPING_MODE", 74, 73 );
	print_it( "POINT_SAMPLE_MINIFY", 75, 75 );
	print_it( "POINT_SAMPLE_MAGNIFY", 76, 76 );
	print_it( "WRAP_MODE_S", 79, 77 );
	print_it( "WRAP_MODE_T", 82, 80 );
	print_it( "WRAP_MODE_R", 85, 83 );
	print_it( "TEXTURE_DIMENSION_S", 98, 86 );
	print_it( "TEXTURE_DIMENSION_T", 111, 99 );
	print_it( "TEXTURE_DIMENSION_R", 124, 112 );
	print_it( "BORDER_COLOR_RED", 140, 125 );
	print_it( "BORDER_COLOR_GREEN", 156, 141 );
	print_it( "BORDER_COLOR_BLUE", 172, 157 );
	print_it( "BORDER_COLOR_ALPHA", 188, 173 );
	print_it( "SHADOW_MAPPING_AMBIENT", 204, 189 );
	print_it( "TEXTURE_ADDRESSING_MODE", 206, 205 );
	print_it( "BORDER_TEXEL_ENABLE", 207, 207 );
	print_it( "SHADOW_MAPPING_CMP_FUNC", 210, 208 );
	print_it( "SHADOW_MAPPING_CPY_RGB", 211, 211 );
	print_it( "SHADOW_MAPPING_CPY_ALPHA", 212, 212 );
	print_it( "TEXTURE_STACKING_ENABLE", 213, 213 );
	print_it( "MIPMAP_ADDRESS_0", 247, 222 );
	print_it( "MIPMAP_ADDRESS_1", 273, 248 );
	print_it( "MIPMAP_ADDRESS_2", 299, 274 );
	print_it( "MIPMAP_ADDRESS_3", 325, 300 );
	print_it( "MIPMAP_ADDRESS_4", 351, 326);
	print_it( "MIPMAP_ADDRESS_5", 377, 352 );
	print_it( "MIPMAP_ADDRESS_6", 403, 378 );
	print_it( "MIPMAP_ADDRESS_7", 429, 404 );
	print_it( "MIPMAP_ADDRESS_8", 455, 430 );
	print_it( "MIPMAP_ADDRESS_9", 481, 456 );
	print_it( "MIPMAP_ADDRESS_10", 507, 482 );
#if 0
	{
		int i;
		for ( i = 0; i <= 507; i++ )
		{
			if ( used[ i ] == 0 )
			{
				_mali_sys_printf("word %d is reserved perhaps?\n", i );

			}
			else if ( used[ i ] > 1 )
			{
				_mali_sys_printf("word %d has been used several times\n", i );
			}
		}
	}
#endif
fflush(stdout);
while( 1 );
}
#endif

#endif /* __M200_TD_HEADER__ */
