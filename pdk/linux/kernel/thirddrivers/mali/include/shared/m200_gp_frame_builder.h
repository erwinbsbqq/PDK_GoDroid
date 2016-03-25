/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2006-2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef M200_GP_FRAME_BUILDER_H
#define M200_GP_FRAME_BUILDER_H

#if !defined(USING_MALI200) && !defined(USING_MALI400)
#error This header is for Mali200/400 only
#endif

#include <mali_system.h>
#include <base/pp/mali_pp_job.h>
#include <base/gp/mali_gp_job.h>
#include <base/mali_memory.h>
#include <base/mali_dependency_system.h>
#include <mali_instrumented_context_types.h>
#include <regs/MALI200/mali200_core.h>
#include <regs/MALIGP2/mali_gp_plbu_cmd.h>
#include <shared/mali_mem_pool.h>

/* helpers to calculate the supersample scaling factors */
#define SUPERSAMPLE_SCALE_X( factor ) (1 << ((factor) >> 1))
#define SUPERSAMPLE_SCALE_Y( factor ) (1 << (( (factor) >> 1) + ((factor) & 0x1)))

/* Output parameter bitflags, used by _mali_frame_builder_set_output calls).
 * For a detailed explanation what each do, check the doxygen for _mali_frame_builder_set_output*/
#define MALI_OUTPUT_COLOR (1<<0)
#define MALI_OUTPUT_DEPTH (1<<1)
#define MALI_OUTPUT_STENCIL (1<<2)
#define MALI_OUTPUT_MRT (1<<3)
#define MALI_OUTPUT_WRITE_DIRTY_PIXELS_ONLY (1<<4)
#define MALI_OUTPUT_DOWNSAMPLE_X_2X (1<<5)
#define MALI_OUTPUT_DOWNSAMPLE_X_4X (1<<6)
#define MALI_OUTPUT_DOWNSAMPLE_X_8X (1<<7)
#define MALI_OUTPUT_DOWNSAMPLE_Y_2X (1<<8)
#define MALI_OUTPUT_DOWNSAMPLE_Y_4X (1<<9)
#define MALI_OUTPUT_DOWNSAMPLE_Y_8X (1<<10)
#define MALI_OUTPUT_DOWNSAMPLE_Y_16X (1<<11)
#define MALI_OUTPUT_FSAA_4X (MALI_OUTPUT_DOWNSAMPLE_X_2X | MALI_OUTPUT_DOWNSAMPLE_Y_2X)
#define MALI_OUTPUT_FSAA_16X (MALI_OUTPUT_DOWNSAMPLE_X_4X | MALI_OUTPUT_DOWNSAMPLE_Y_4X)

/* The place where we usually find the color attachment */
#define MALI_DEFAULT_COLOR_WBIDX (0)

/* forward declarations */
typedef struct mali_frame_builder mali_frame_builder;
struct mali_surface;

/* callback parameter datatype for mali_frame_cb_func */
typedef u32 mali_frame_cb_param;

/* The frame callback function type, which can be added to the current frame using
 * _mali_frame_builder_add_frame_callback()
 */
typedef void (*mali_frame_cb_func)(mali_frame_cb_param cb_param);

typedef u64 mali_tilelist_cmd; /**< one tile list entry */

/**
 * Framebuilder allocation properties.
 * These can be combined into a bitfield for the framebuilder constructor.
 */
enum mali_frame_builder_properties
{
	/** No flags present. This is the default behavior if no flags are specified. */
	MALI_FRAME_BUILDER_PROPS_NONE                 = 0,

	/** If set, the screen-origo is the bottom-left corner. If not, it is in the top-left corner. */
	MALI_FRAME_BUILDER_PROPS_ORIGO_LOWER_LEFT     = (1<<0),

	/* If set, automatic readbacks will occur. 
	 * The readbacks will occur when calling frame_builder_write_lock and the owner of the 
	 * surfaces attached do not match the framebuilder being locked. 
	 * Currently, this flag is only used by GLES FBO framebuilders. 
	 */
	MALI_FRAME_BUILDER_PROPS_AUTOMATIC_READBACKS = (1<<1),

	/**
	 * If set, the framebuilder will allocate a PLBU heap.
	 * If it is known that the framebuilder will only be used to draw very simple geometry,
	 * this flag should not be set.
	 */
	MALI_FRAME_BUILDER_PROPS_ALLOCATE_HEAP        = (1<<2),

	/**
	 * If set, calls to _mali_frame_builder_flush will behave as calling _mali_frame_builder_swap.
	 * The intended use for this is for FBOs with multiple internal frames.
	 */
	MALI_FRAME_BUILDER_PROPS_ROTATE_ON_FLUSH      = (1<<3)
};

/**
 *	Framebuilder types available in the driver. Should you add anything to this enum, make
 *  sure you update the timeline profiling tool as well.
 */
enum mali_frame_builder_type
{
	MALI_FRAME_BUILDER_TYPE_UNKNOWN                 = 0,
	MALI_FRAME_BUILDER_TYPE_EGL_SURFACE             = 1,  /* deprecated, replaced by the 3 differentiating WINDOW/PIXMAP/PBUFFER values */
	MALI_FRAME_BUILDER_TYPE_GLES_FRAMEBUFFER_OBJECT = 2,
	MALI_FRAME_BUILDER_TYPE_GLES_TEXTURE_UPLOAD     = 3,
	MALI_FRAME_BUILDER_TYPE_VG_RENDERCHAIN          = 4,
	MALI_FRAME_BUILDER_TYPE_VG_MASKBUFFER           = 5,
	MALI_FRAME_BUILDER_TYPE_EGL_WINDOW              = 6,
	MALI_FRAME_BUILDER_TYPE_EGL_PBUFFER             = 7,
	MALI_FRAME_BUILDER_TYPE_EGL_PIXMAP              = 8
};

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Allocate and initialize a new frame builder
 * @param type Type of the framebuilder (see #mali_frame_builder_type above)
 * @param base_ctx The base context with which to perform all Mali resource (pp/gp jobs, mali mem) allocations
 * @param frame_count The number of internal frames. Recommended value is 3
 * @param properties A combination of properties for the frame builder. See #mali_frame_builder_properties.
 * @return The new frame_builder or NULL if the allocation failed
 */
MALI_IMPORT mali_frame_builder *_mali_frame_builder_alloc( enum mali_frame_builder_type type,
                                                           mali_base_ctx_handle base_ctx,
                                                           u32 frame_count,
                                                           enum mali_frame_builder_properties properties );

/**
 * Deinitialize and release all frame builder resources
 * @param frame_builder The frame builder to release
 */
MALI_IMPORT void _mali_frame_builder_free( mali_frame_builder *frame_builder );

/**
 * Release the frame builder to its initial state
 * @param frame_builder The frame builder to reset
 */
MALI_IMPORT void _mali_frame_builder_reset( mali_frame_builder *frame_builder );

/**
 * Release the frame builder to a clean state. Same as reset, but will output the clearcolor if flushed. 
 * @param frame_builder The frame builder to clean
 */
MALI_IMPORT void _mali_frame_builder_clean( mali_frame_builder *frame_builder );

/**
 * Per-draw-call init of the frame builder. Must be called to signal incoming modifications to the frame.
 * @param frame_builder The frame builder to use
 * @return MALI_ERR_NO_ERROR If the initialization was successful, otherwise an error explaining what went wrong.
 */
MALI_IMPORT mali_err_code _mali_frame_builder_use( mali_frame_builder *frame_builder );

/**
 * Flush the actual pixel rendering, but make it possible to continue building the frame afterwards
 * @param frame_builder The frame builder defining the current frame and its physical rendertargets
 * @return MALI_ERR_NO_ERROR If the initialization was successful, otherwise an error explaining what went wrong.
 * @note Frame state might be corrupted if the function fails, meaning that the frame builder should be reset in this case.
 */
MALI_IMPORT mali_err_code _mali_frame_builder_flush( mali_frame_builder *frame_builder );

/**
 * Flush the actual pixel rendering and swap out the frame, swapping in the next frame.   
 * @param frame_builder The frame builder defining the current frame and its physical rendertargets
 * @return MALI_ERR_NO_ERROR If the initialization was successful, otherwise an error explaining what went wrong.
 * @note Frame state might be corrupted if the function fails, meaning that the frame builder should be reset in this case.
 *       Before calling this function, register callbacks (if needed) when the frame is done using _mali_frame_builder_add_callback() or
 *       _mali_frame_builder_add_callback_threadsafe() depending on thread-safety concerns.
 */
MALI_IMPORT mali_err_code _mali_frame_builder_swap( mali_frame_builder *frame_builder );

/**
 * Checks if a frame builder is in a modified state
 * @param frame_builder The frame builder to check if modified
 * @return MALI_TRUE if modified, MALI FALSE if not
 */
MALI_IMPORT mali_bool _mali_frame_builder_is_modified( mali_frame_builder *frame_builder );


/**
 * Add a mem_handle to the frame garbage collection. This will be released when it is guaranteed that the frame will
 * not be re-used (i.e. when it is reset or it has been "swapped out" through _mali_frame_builder_swap().
 * @param frame_builder The frame builder defining the frame managing the memory
 * @param mem_handle The memory to add to the garbage collection
 */
MALI_IMPORT void _mali_frame_builder_add_frame_mem( mali_frame_builder *frame_builder,
                                                    mali_mem_handle mem_handle );

/**
 * Wait until the GPU is no longer rendering to the surface used by the current frame.
 *
 * If the MALI_FRAME_BUILDER_PROPS_ROTATE_ON_FLUSH property is set, this will wait for
 * all frames to complete.
 *
 * @param frame_builder The frame builder that owns the frame that is potentially being rendered
 */
MALI_IMPORT void _mali_frame_builder_wait( mali_frame_builder *frame_builder );

/**
 * Wait until the GPU is no longer rendering to the current frame.
 *
 * This is different from #_mali_frame_builder_wait only if the
 * MALI_FRAME_BUILDER_PROPS_ROTATE_ON_FLUSH property is set. If so, this function will only
 * wait for a single frame to complete.
 *
 * @param frame_builder The frame builder that owns the frame that is potentially being rendered
 */
MALI_IMPORT void _mali_frame_builder_wait_frame( mali_frame_builder *frame_builder );

/**
 * Wait until the GPU is no longer rendering to the surface used by the current frame.
 *
 * This function will always wait for all frames to complete.
 *
 * @param frame_builder The frame builder that owns the frame that is potentially being rendered
 */
MALI_IMPORT void _mali_frame_builder_wait_all( mali_frame_builder *frame_builder );


/**
 * Attach a surface to a frame builder as an output buffer.
 * This operation will addref the given surface, and replace (deref) whatever is already present
 * at the given wbx_id. The specified surface will be written to at the time of flush. 
 * The size of the surface will decide the framebuilder dimension at the time of drawcalls. 
 * Do not modify attachment dimensions midframe, but feel free to change surface pointers as you see fit. 
 *
 * @param frame_builder The frame builder to update
 * @param wbx_id The writeback unit ID responsible for writing to this surface. 
 *              Legal range is 0 to MALI200_WRITEBACK_UNIT_COUNT exclusive. 
 * @param surface The mali_surface this framebuilder should write to. 
 *              Can be NULL, in which case the writeback unit is disabled. 
 * @param usage This is a bitmask of usage information. Valid bits are as given:
 *
 *         MALI_OUTPUT_COLOR 
 *         MALI_OUTPUT_DEPTH 
 *         MALI_OUTPUT_STENCIL 
 *            Buffer usage bits. These are not mutually exclusive, but must match the surface format. 
 *            Color can not be set if depth or stencil is set. At least one of these must be set. 
 *
 *         MALI_OUTPUT_MRT
 *            The WBX will output in MRT mode. This will output all 4 subsamples to the memory 
 *            buffer given by the surface instead of the regular output. The surface->mem_ref given 
 *            in the surface need to contain a memory block that is (at least) 4x as large as the 
 *            surface->datatype, plus any offset. The offset to the individual subsample outputs is 
 *            given as the surface->offset plus respectively: 
 *             - 0
 *             - 1*surface->datasize
 *             - 2*surface->datasize 
 *             - 3*surface->datasize. 
 *            Each of these offsets need to be aligned to the surface alignment requirements. 
 *            This flag is typically only used by the incremental rendering subsystem, and can be 
 *            ignored for average users of the framebuilder. 
 *
 *         MALI_OUTPUT_DOWNSAMPLE_X_2X
 *         MALI_OUTPUT_DOWNSAMPLE_X_4X
 *         MALI_OUTPUT_DOWNSAMPLE_X_8X
 *         MALI_OUTPUT_DOWNSAMPLE_Y_2X
 *         MALI_OUTPUT_DOWNSAMPLE_Y_4X
 *         MALI_OUTPUT_DOWNSAMPLE_Y_8X
 *         MALI_OUTPUT_DOWNSAMPLE_Y_16X      -- note: there is no matching dowscale for the X axis
 *         MALI_OUTPUT_FSAA_4X   => same as MALI_OUTPUT_DOWNSAMPLE_X_2X | MALI_OUTPUT_DOWNSAMPLE_Y_2X 
 *         MALI_OUTPUT_FSAA_16X  => same as MALI_OUTPUT_DOWNSAMPLE_X_4X | MALI_OUTPUT_DOWNSAMPLE_Y_4X
 *            Specifies supersampling downscale. 
 *            The WBx unit will downsample the outputs in both X and Y dimensions at the requested factor,
 *            or in one of the directions at the specified factor, before outputting the results. The flags
 *            setting both factors effectively set both axis flags at one. Only one of these bits per
 *            axis can be specified. The output surfaces specified with either of these flags should 
 *            be sized accordingly smaller. F.ex, if you are rendering to a 20x20 surface with 
 *            MALI_OUTPUT_DOWNSAMPLE_4X set, then the framebuilder internally will set up an output 
 *            of 80x80 pixels (pre-downscale), and querying the framebuilder for size will return 80x80. 
 *            Other surfaces without downsample set should be sized 80x80 as well. This flag is not 
 *            mutually exclusive with the MRT flag. Typically, 16x FSAA is created by setting
 *            MALI_OUTPUT_FSAA_4X as well as enabling 4x multisampling.
 *
 *         MALI_OUTPUT_WRITE_DIRTY_PIXELS_ONLY
 *            Sets the WBx_DIRTY_BIT_ENABLE HW flag. This flag ensures that only pixels written to 
 *            in the tile buffer are written back to the framebuffer. This prevents background pixels
 *            from being written, but breaks badly for blended geometry and antialiasing. Use conciously. 
 *
 * @note If the surface set to the framebuilder don't match up in size (modified for downscale), an assert is 
 *       raised on the next drawcall. Do not change the output resolution between the first drawcall and swap. 
 *
 */
MALI_IMPORT void _mali_frame_builder_set_output( mali_frame_builder *frame_builder,
                                                 u32 wbx_id,
                                                 struct mali_surface *surface,
                                                 u32 usage
                                               );

/**
 * Retrieve a the output buffer previously set. 
 * @param frame_builder The framebuilder to retrieve from
 * @param wbx_id The index to get the surface from. Legal range is 0 to MALI200_WRITEBACK_UNIT_COUNT exclusive.
 * @param usage [out] Optional parameter. Will return the usage flag this surface was set with. NULL if not needed. 
 * @note Caller is responsible for addrefing the surface if used for long time storage. 
 *       Pointer is otherwise valid until the next set. 
 */
MALI_IMPORT struct mali_surface* _mali_frame_builder_get_output(mali_frame_builder *frame_builder, u32 wbx_id, u32* usage);

/**
 * Set a callback to be executed when the current output buffer needs to be locked down.
 * Can implicitly add a dependency to PP consumer, so that PP can not draw into the buffer before this dependency is met.
 * @param frame_builder The framebuilder to set lock output callback on
 * @param cb_func Callback function to execute
 * @param cb_param Callback parameter
 * @param ds_consumer_pp_render PP consumer ds handle returned
 */
MALI_IMPORT void _mali_frame_builder_set_lock_output_callback( mali_frame_builder *frame_builder, mali_frame_cb_func cb_func, mali_frame_cb_param cb_param, mali_ds_consumer_handle *ds_consumer_pp_render );

/**
 * Set a callback to be executed when an output buffer is required
 * @param frame_builder The framebuilder to set aquire output callback on
 * @param cb_func Callback function to execute
 * @param cb_param Callback parameter
 */
MALI_IMPORT void _mali_frame_builder_set_aquire_output_callback( mali_frame_builder *frame_builder, mali_frame_cb_func cb_func, mali_frame_cb_param cb_param );

/**
 * Aquire a new output buffer.
 * Depends on the aquire output buffer callback being set
 * @param frame_builder The framebuilder to aquire a new output buffer into
 */
MALI_IMPORT void _mali_frame_builder_aquire_output( mali_frame_builder *frame_builder );

/**
 * Set the frame builder's color, depth or stencil clear value.
 * @param frame_builder The frame builder to set a clear value of
 * @param buffer_type   The type of buffer to set the clear value for. Can
 *                      be MALI_OUTPUT_COLOR, MALI_OUTPUT_DEPTHor
 *                      MALI_OUTPUT_STENCIL
 * @param clear_value   The clear value to set on the frame builders buffer_type
 *                      buffer
 */
MALI_IMPORT void _mali_frame_builder_set_clear_value( mali_frame_builder *frame_builder, u32 buffer_type, u64 clear_value );

/**
 * Retrieve the frame builders color, depth or stencil clear value
 * @param frame_builder The frame builder to retrieve the clear value of
 * @param buffer_type   The type of the buffer to retrieve the clear value of
 * @return The clear value of the frame builder buffer of type buffer_type
 */
MALI_IMPORT u64 _mali_frame_builder_get_clear_value( mali_frame_builder *frame_builder, u32 buffer_type );

/**
 * Retrieves the error status of the last rendering job that failed since the
 * last call to this function. If no jobs have have failed then
 * MALI_JOB_STATUS_FINISHED is returned.
 *
 * @param frame_builder The frame builder to check for job completion errors
 * @return The error completion status of the last job that failed since this
 *         function was last called or MALI_JOB_STATUS_FINISHED if no jobs
 *         have failed
 */
MALI_IMPORT mali_job_completion_status _mali_frame_builder_get_framebuilder_completion_status( mali_frame_builder *frame_builder );


/* Mali200 frame builder specific functions ---------------------------------------------------------------- */

/**
 * Update the fragment stack requirements for the current frame
 * @param frame_builder The frame builder defining the frame
 * @param stack_start The initial stack position required for a specific shader
 * @param stack_size The stack size required for a specific shader
 */
MALI_IMPORT void _mali_frame_builder_update_fragment_stack( mali_frame_builder *frame_builder, u32 stack_start, u32 stack_size );

/**
 * Signal that the current gp_job can be started.
 * @param frame_builder The frame builder defining the frame
 * @return MALI_ERR_NO_ERROR if the frame_builders job finalization was successful, otherwise an error explaining the failure.
 * @note This function does not guarantee that the job is started immediately. Actual startup
 *       might be postponed until _mali_frame_builder_flush() / _mali_frame_builder_swap()
 */
MALI_IMPORT mali_err_code _mali_frame_builder_flush_gp( mali_frame_builder *frame_builder);

/**
 * Get the tile list block scale corresponding to bits [8:9] of GP_PLB_CONF_REG_PARAMS
 * @param frame_builder The frame builder defining the frame
 * @return A tile list block scale value in the range [ 0, 3]
 */
MALI_IMPORT u32 _mali_frame_builder_get_tile_list_block_scale( mali_frame_builder *frame_builder );


/**
 * Set the GP2 scissor parameters as described in 3.15.3 (cmd 7) in the M200 TRM
 *
 * If #buffer is @c NULL, then any PLBU commands will be written immediately
 * to the current frame's PLBU command list.
 *
 * If #buffer is not @c NULL, then any PLBU commands will be written to #buffer, and
 * #index will be incremented once per written command. The caller is responsible for
 * committing this to the PLBU command list.
 *
 * @param frame_builder The frame builder defining the frame
 * @param         left   The left coordinate of the scissorbox
 * @param         top    The top coordinate  of the scissorbox
 * @param         right  The right coordinate of the scissorbox
 * @param         bottom The bottom coordinate of the scissorbox
 * @param[in,out] buffer Pointer to an array of PLBU commands. May be @c NULL (see above).
 * @param[in,out] index  Index of first free entry in the array of PLBU commands. May be @c NULL (see above)
 * @param         buffer_len  The total number of elements in #buffer
 *
 * @return MALI_ERR_NO_ERROR if the frame_builders job finalization was successful, otherwise an error explaining the failure.
 */
MALI_IMPORT mali_err_code _mali_frame_builder_scissor( mali_frame_builder *frame_builder,
                                                       u32 left, u32 top, u32 right, u32 bottom,
                                                       mali_gp_plbu_cmd * const buffer, u32 * const index, const u32 buffer_len );

/**
 * Set the GP2 viewport parameters as described in 3.11.6 - .9 in the M200 TRM
 *
 * If #buffer is @c NULL, then any PLBU commands will be written immediately
 * to the current frame's PLBU command list.
 *
 * If #buffer is not @c NULL, then any PLBU commands will be written to #buffer, and
 * #index will be incremented once per written command. The caller is responsible for
 * committing this to the PLBU command list.
 *
 * @param frame_builder       The frame builder defining the frame
 * @param         left        The left coordinate of the viewport
 * @param         top         The top coordinate  of the viewport
 * @param         right       The right coordinate of the viewport
 * @param         bottom      The bottom coordinate of the viewport
 * @param[in,out] buffer      Pointer to an array of PLBU commands. May be @c NULL (see above).
 * @param[in,out] index       Index of first free entry in the array of PLBU commands. May be @c NULL (see above)
 * @param         buffer_len  The total number of elements in #buffer
 *
 * @return MALI_ERR_NO_ERROR if the frame_builders job finalization was successful, otherwise an error explaining the failure.
 */
MALI_IMPORT mali_err_code _mali_frame_builder_viewport( mali_frame_builder *frame_builder,
                                                        float left, float top, float right, float bottom,
                                                        mali_gp_plbu_cmd * const buffer, u32 * const index, const u32 buffer_len );

/**
 * Get the handle to the base_ctx used for allocations of base resources
 * @param frame_builder The frame builder to query
 * @return A handle to a base ctx
 */
MALI_IMPORT mali_base_ctx_handle _mali_frame_builder_get_base_ctx( mali_frame_builder *frame_builder );

/**
 * Add memory to the "garbage collector" which will release the memory when the current gp_job has finished.
 * @param frame_builder The frame builder defining the frame
 * @param mem_handle The memory to add to the list.
 */
MALI_IMPORT void _mali_frame_builder_add_gp_job_mem( mali_frame_builder *frame_builder,
                                                     mali_mem_handle mem_handle);

/**
 * Sets the split count.
 * The new split count will take effect when the next frame starts (i.e. it will not affect the current frame)
 * @param frame_builder The frame builder to update
 * @param split_cnt The new split count
 */
MALI_IMPORT void _mali_frame_builder_set_split_count( mali_frame_builder *frame_builder, u32 split_cnt );

/**
 * Set the number of subpixel precision bits (if not set, the default value is used)
 * @param frame_builder The frame builder defining the frame
 * @param num_bits The number of subpixel bits - must be in the range [0, 127]
 */
MALI_IMPORT void _mali_frame_builder_set_subpixel_bits( mali_frame_builder *frame_builder, u32 num_bits );

/**
 * Returns whether incremental rendering is requested or not. Will only return true
 * if all conditions for correct incremental rendering are true.
 * @param frame_builder The frame builder defining the frame
 */
MALI_IMPORT mali_bool _mali_frame_builder_incremental_rendering_requested( mali_frame_builder *frame_builder );

/**
 * @brief Locks all the surfaces of the frame-builder and clears it if all attachments are dirty
 * @param frame_builder The frame-builder to lock the surfaces of
 * @return MALI_ERR_NO_ERROR if successful, an error-code if not
 * NOTE: This function will replace mali_frame_builder_use. Currently, VG still use the old version, this will change
 */
MALI_IMPORT MALI_CHECK_RESULT mali_err_code _mali_frame_builder_write_lock( mali_frame_builder *frame_builder );

/**
 * @brief Unlocks all the surfaces of the frame-builder
 * @param frame_builder The frame-builder to unlock the surfaces of
 */
MALI_IMPORT void _mali_frame_builder_write_unlock( mali_frame_builder *frame_builder );

/*
 * @brief Sets in the framebuilder that the next flush should trigger incremental rendering
 * @param frame_builder The frame-builder to set
 * @param value == MALI_TRUE, next flush will trigger incremental rendering, else not
 */
MALI_IMPORT void _mali_frame_set_inc_render_on_flush( mali_frame_builder* frame_builder, mali_bool value );

#if HARDWARE_ISSUE_7320
/**
 * Get a vertex shader commandlist command for flushing the vertex storer.
 * Depending on the presence of hardware issue 7320, this can be just a
 * flush command or a call of a subroutine containing a workaround.
 */
MALI_IMPORT mali_addr _mali_frame_builder_get_flush_subroutine( mali_frame_builder *frame_builder );
#endif

#if HARDWARE_ISSUE_4126
/* When rendering multiple drawcalls in one job, Mali200 R0P1 will have an issue if the number of vertex shader
 * instructions in a drawcall is exactly 6 instructions longer than the number of instructions in the previous drawcall.
 * To avoid this issue, the frame builder need to inject dummy jobs into the command lists, which is what this function does.
 * This function should be called just after calling _mali_frame_builder_use or _mali_frame_builder_write_lock,
 * and before every drawcall added. Example (typical):
 *
 *   _mali_frame_builder_write_lock()
 *   _mali_frame_builder_workaround_for_bug_4126()
 *   add_some_drawcall_to_frame()
 *   _mali_frame_builder_workaround_for_bug_4126()
 *   add_some_drawcall_to_frame()
 *   _mali_frame_builder_write_unlock()
 *
 * @param frame_builder The framebuilder you've just created a frame for, but not yet added your drawcall to
 * @param num_vshader_instruction_in_next_drawcall The number of vertex shader instructions in your next drawcall.
 */
MALI_IMPORT mali_err_code _mali_frame_builder_workaround_for_bug_4126( mali_frame_builder* frame_builder, u32 num_vshader_instructions_in_next_drawcall);
#endif /* HW issue 4126 */

/**
 * @brief Adds a dependency between the given surface and the current frame in the framebuilder
 * @param frame_builder The current frame in the frame builder will be considered
 * @param read_surface The surface will be added as a resource read dependency
 * @return MALI_ERR_NO_ERROR, or MALI_OUT_OF_MEMORY on oom situations.
 * @note Should only be called inside a mali_framebuilder_writelock/unlock pair.
 *
 */
MALI_IMPORT mali_err_code _mali_frame_builder_add_surface_read_dependency(mali_frame_builder* frame_builder, struct mali_surface* read_surface);

#if MALI_TEST_API

MALI_IMPORT void
_mali_frame_builder_set_incremental_render_threshold( mali_frame_builder *frame_builder,
                                                      u32                 threshold );

#endif /* MALI_TEST_API */

#ifdef __cplusplus
}
#endif

#endif /* !defined(M200_GP_FRAME_BUILDER_H */

