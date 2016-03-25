/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009-2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _M200_INCREMENTAL_RENDERING_H_
#define _M200_INCREMENTAL_RENDERING_H_

#include <mali_system.h>
#include "m200_gp_frame_builder.h"

/**
 * A capability bit-mask encoding which buffers/data should be preserved when performing incremental
 * rendering.
 *
 * The buffers that are requested through this mask will be written to off-screen memory using a
 * writeback unit and will be read back in through a textured quad.
 *
 * If you request that multisampling should be preserved then all four sub-samples of each pixel
 * of each buffer will be written to off-screen memory and read back as four textured quads, each
 * with three of the subsample writes disabled.
 *
 * If your request that supersampling should be preserved then each buffer will be written to
 * off-screen memory with downsampling turned off and read back as a textured quad.
 */
typedef enum mali_incremental_render_capabilities
{
	MALI_INCREMENTAL_RENDER_NO_CAPABILITY           = 0,
	MALI_INCREMENTAL_RENDER_PRESERVE_COLOR_BUFFER   = (1<<0), /**< True if you wish the color buffer to be preserved */
	MALI_INCREMENTAL_RENDER_PRESERVE_DEPTH_BUFFER   = (1<<1), /**< True if you wish the depth buffer to be preserved */
	MALI_INCREMENTAL_RENDER_PRESERVE_STENCIL_BUFFER = (1<<2), /**< True if you wish the stencil buffer to be preserved */
	MALI_INCREMENTAL_RENDER_PRESERVE_MULTISAMPLING  = (1<<3), /**< True if you wish the multisampling data to be preserved */
/*	MALI_INCREMENTAL_RENDER_PRESERVE_SUPERSAMPLING  = (1<<4) */ /**< Supersampling is always preserved. It's hard to not do so actually. */
} mali_incremental_render_capabilities;



/**
 * Performs incremental rendering and a readback on the given frame builder. This function is
 * synchronous so when it returns all geometry that have previously been added to the frame builder
 * for processing (draw calls, etc) will have been rendered off-screen and deleted. In addition the
 * color, depth and stencil buffers will have been added to the frame builder as textured quads so
 * that they will be included in the final frame.
 *
 * In other words the following is done in an incremental rendering cycle:
 *  1. render to texture
 *  2. reset the frame builder
 *  3. read textures back in again
 *
 * @note Calling this function will reset the frame_builder, including calling all callbacks that
 *       have been added to it
 * @note It is allowed to add more geometry to the frame builder for the current frame after
 *       calling this function. That geometry will then be rendered on top of the textured quad
 *       containing the results of previous draw calls, etc.
 * @note Calling this function will effectively erase the GP Vertex Shader and PLBU command list,
 *       which means that the VS/PLBU state represented by these will be reset. If you need to
 *       continue rendering with this state after calling this function then you must add commands
 *       to the VS and PLBU command lists to recreate it
 * @note If a failure occurs then the framebuffer results are undefined
 *
 * @param frame_builder          - The frame builder to perform incremental rendering on, thereby
 *                                 resolving all previously registered draw calls/geometry
 * @param capabilities           - A bit-mask containing information on which buffers to preserve
 * @return An error code if something went wrong
 */
MALI_IMPORT MALI_CHECK_RESULT mali_err_code _mali_incremental_render( mali_frame_builder                   *frame_builder,
                                                                      mali_incremental_render_capabilities  capabilities );
#endif /* _M200_INCREMENTAL_RENDERING_H_ */

