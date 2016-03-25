/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009-2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _MALI_SURFACE_H_
#define _MALI_SURFACE_H_


#include <mali_system.h>
#include <base/mali_memory.h>
#include <base/mali_dependency_system.h>
#include <shared/mali_shared_mem_ref.h>
#include <shared/mali_surface_specifier.h>

/* surface flags enum. Bitwise combinations of these go into the flags field of the various constructors */
enum mali_surface_flags
{
	MALI_SURFACE_FLAGS_NONE = 0,         			/* No special flags present, this is the default behavior */
	MALI_SURFACE_FLAG_DONT_MOVE = 1,			/* The surface memory is referenced externally, and must never be moved. This is a property of pixmaps and window surfaces */
	MALI_SURFACE_FLAG_COMBINED_MIPMAP_CUBE_TARGET  = 2,   	/* Can be ignored by non-GLES contexts. Surfaces big enough for all cubemap's faces are flagged like this */
	MALI_SURFACE_FLAG_COMBINED_MIPMAP_OTHER_TARGET = 3	/* Can be ignored by non-GLES contexts. Surfaces are flagged like this when they contain the last three mipmap levels*/
};

enum mali_surface_event
{
	MALI_SURFACE_EVENT_CPU_ACCESS,        /* CPU about to read/write to the surface. Called as a part of the _mali_surface_map function */
	MALI_SURFACE_EVENT_CPU_ACCESS_DONE,   /* CPU done read/writing to the surface. Called as a part of the _mali_surface_unmap function */

	MALI_SURFACE_EVENT_GPU_WRITE,         /* GPU about to write to the surface. Called as a part of the framebuilder flush/swap functions */
	MALI_SURFACE_EVENT_GPU_WRITE_DONE,    /* GPU done writing to the surface. Called as a part of the PP callback function 
	                                         NB: Currently called on frame complete, not flush complete. We may want to change this.
											 As a consequence: if glFlush'ing a frame twice, you will end up with 2 GPU write, then 2 GPU write done, in that order. 
											 Also note that the ROTATE_ON_FLUSH property on a framebuilder will likely save us here. Since all flushes are swaps. */
	MALI_SURFACE_EVENT_DESTROY,           /* Used by the mali_image, for reasons unknown. Triggered when the surface is freed */
	MALI_SURFACE_EVENT_COPY_ON_WRITE,     /* Called when triggering a "copy on write". Used by VG to force the texture descriptor into the new copy */

	/* TODO: NOT YET SUPPORTED EVENTS: */
	MALI_SURFACE_EVENT_GPU_READ,          /* Do we want separate events for read/write, or can we just merge them into the "access" moniker, like with the CPU? */
	MALI_SURFACE_EVENT_AQUIRE_BUFFER,     /* Called when doing the first drawcall to the surface from a framebuilder. Useful for the "aquire buffer" 
	                                         functionality in the EGL backend. */ 

	MALI_SURFACE_EVENT_COUNT              /* The number of events defined. Keep this at the end of the list */
};

/* function pointer definitions for os_lock/unlock  */
struct mali_surface;
typedef void (*mali_surface_eventfunc)(struct mali_surface* surface, enum mali_surface_event event, void *data);

/**
 * This structure contains all metadata regarding a surface in mali memory.
 * A surface can be rendered to and used as a texture.
 */
typedef struct mali_surface
{
	/* The following values can be read and modified after locking the surface. */
	struct mali_shared_mem_ref     *mem_ref;       /**< Texture data in a mali memory block*/
	u32                            mem_offset;     /**< Offset in mali memory block where the texture data starts. */

	struct mali_frame_builder      *owner;         /**< This holds outstanding writes.
	                                                    Flushing the owner will cause the surface contents to be correct.
														To flush it out, call _mali_surface_clear_owner. */

	/* These are constant values and will never change during the life-time of the surface.
	 * Reading these without holding the access lock is completely safe */
	struct mali_surface_specifier  format;         /**< Contains all required information about the format and layout */
	u32                            datasize;       /**< The bytesize of the mali memory this surface requires. Autocalculated on create */
	mali_base_ctx_handle           base_ctx;       /**< A pointer to the base context used when allocating this surface */
	enum mali_surface_flags        flags;         	/**< Special behavior flags for this surface. See the flags enum for details */

	/* The following values are atomic and can be modified from anywhere */
	mali_mutex_handle              access_lock;    /**< A mutex used to prevent concurrent (CPU) access on the structure members. */
	mali_atomic_int                ref_count;      /**< Reference count. Increased per API context using the surface. */
	mali_ds_resource_handle        ds_resource;    /**< Dependency resource. Consumers can depend on surfaces */
	u64                            timestamp;      /**< Keeps track of the last Copy-on-Write operation; it is an autoincremented number, not an actual timestamp */

	/* callback pointers (should never be read or written to directly - use the API functions) */
	mali_surface_eventfunc         eventfunc[MALI_SURFACE_EVENT_COUNT];
	void*                          eventdata[MALI_SURFACE_EVENT_COUNT];

} mali_surface;

/**
 * Whenever we have to do a deep CoW on a mali_surface we need those values
 * describing which data needs to go from where to where
 */
typedef struct mali_surface_deep_cow_descriptor
{
	struct mali_shared_mem_ref *src_mem_ref;   /**< The source data */
	u32                         mem_offset;    /**< Memory offset */
	u32                         data_size;     /**< Data size */
	struct mali_shared_mem_ref *dest_mem_ref;  /**< The destination data */
} mali_surface_deep_cow_descriptor;

/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * Allocates a new empty surface.
 * @note This constructor will not allocate any mali memory area.
 * @param flags Surface flags modifying the surface behavior. See the flags struct for details.
 * @param format A struct containing the format and layout of this surface.
 * @param base_ctx a pointer to the base context; required to allocate mali memory
 * @return A pointer to the new surface, NULL on error
 */
MALI_IMPORT struct mali_surface* _mali_surface_alloc_empty(enum mali_surface_flags flags, const mali_surface_specifier* format, mali_base_ctx_handle base_ctx);

/**
 * Allocates a new surface and dynamically allocates memory for it.
 * @note This constructor will implicitly allocate a mali memory area matching the specifications.
 * @param flags Surface flags modifying the surface behavior. See the flags struct for details.
 * @param format A struct containing the format and layout of this surface
 * @param mem_offset Size of additional memory allocated in the same chunk, placed before surface data (usually VG's TD)
 * @param base_ctx a pointer to the base context; required to allocate mali memory
 * @return A pointer to the new surface, NULL on error
 * Note: It is a logical error to use MALI_SURFACE_FLAG_DONT_MOVE with this constructor.
 */
MALI_IMPORT struct mali_surface* _mali_surface_alloc(enum mali_surface_flags flags, const mali_surface_specifier* format, u32 mem_offset, mali_base_ctx_handle base_ctx);

/**
 * Allocates a new surface with the same properties as another surface. This is good if you need to clone a surface.
 * @param surface the surface to clone
 * @param copy_content if MALI_TRUE, the function will also copy the surface data. This may result in the input surface being flushed.
 * @return A pointer to the new surface, NULL on error
 * Note: This constructor will clear the MALI_SURFACE_FLAG_DONT_MOVE flag on the copy, if set.
 */
MALI_IMPORT struct mali_surface* _mali_surface_alloc_surface( struct mali_surface* surface, mali_bool copy_content);


/**
 * Allocates a new surface based on the given mali memory area.
 * @note This is a transfer of ownership of the mali_shared_mem_ref, which means that the surface will take one of the owner
 *       reference counts of the shared mem ref.
 * @param flags Surface flags modifying the surface behavior. See the flags struct for details.
 * @param format A struct containing the format and layout of this surface
 * @param mem_ref The memory reference to take ownership of
 * @param offset Any offset to the given mem_ref. Useful for miplevel 10+
 * @param base_ctx a pointer to the base context; required to allocate mali memory
 * @return A pointer to the new buffer, NULL on error
 */
MALI_IMPORT struct mali_surface* _mali_surface_alloc_ref( enum mali_surface_flags flags, const mali_surface_specifier* format, struct mali_shared_mem_ref *mem_ref,
                                                          u32 offset, mali_base_ctx_handle base_ctx );

/**
 * Release the memory previously allocated for a surface.
 * @param buffer A pointer to the surface buffer to be released
 */
MALI_IMPORT void _mali_surface_free( mali_surface* buffer );

/**
 * Increases the ref count
 * @param buffer The surface which get an increased reference counter.
 */
MALI_STATIC_FORCE_INLINE void _mali_surface_addref( mali_surface *buffer )
{
  MALI_DEBUG_ASSERT_POINTER( buffer );
  MALI_DEBUG_ASSERT( 0 < _mali_sys_atomic_get( &buffer->ref_count ), ("inconsistent ref count (ref_count=%d)", _mali_sys_atomic_get( &buffer->ref_count )));

  _mali_sys_atomic_inc( &buffer->ref_count );

}

/**
 * Decreases the ref count
 * @param buffer The surface which get a decreased reference counter.
 */
MALI_STATIC_FORCE_INLINE void _mali_surface_deref( mali_surface *buffer )
{
	MALI_DEBUG_ASSERT_POINTER( buffer );
	MALI_DEBUG_ASSERT( 0 < _mali_sys_atomic_get( &buffer->ref_count ), ("inconsistent ref count"));

	if ( 0 == _mali_sys_atomic_dec_and_return( &buffer->ref_count ) )
	{
		_mali_surface_free( buffer );
	}
}

/**
 * Locks the surface, mutex style. As long as you hold the lock you can read and modify any surface member
 * @param buffer The surface to lock
 */
MALI_IMPORT void _mali_surface_access_lock( struct mali_surface *buffer );

/**
 * Unlocks the surface again.
 * @param buffer The surface to unlock
 */
MALI_IMPORT void _mali_surface_access_unlock( struct mali_surface *buffer );

/**
 * @brief Clears out the surface owner, resetting it to NULL
 * @param buffer The surface to flush the owner of
 * @note This is a blocking operation. Try to use the dependency system instead of this function.
 */
MALI_IMPORT MALI_CHECK_RESULT mali_err_code _mali_surface_clear_owner( struct mali_surface *buffer );

/**
 * @brief Maps up the surface memory.
 * @param buffer The surface to map up.
 * @param flag Requested minimum capabilities for the buffer, as given to _mali_mem_ptr_map_area
 * @return a pointer to the mapped surface. Refer to the surface members for this memory layout.
 * @note You must hold the surface access before calling this function, and not release it until unmapped.
 * @note Modifications to the mapped memory will not be recognized unless you hold the writelock while doing so
 * @note There may be outstanding deferred writes to the surface. Clearing the surface owner will ensure these are flushed.
 */
MALI_IMPORT void* _mali_surface_map( struct mali_surface *buffer, u32 flag );

/**
 * @brief unmaps the surface memory again
 * @param buffer the surface to release
 */
MALI_IMPORT void _mali_surface_unmap( struct mali_surface *buffer );

/**
 * @brief Creates a sync handle
 * @param surface pointer to surface
 * @return MALI_TRUE on success, MALI_FALSE on error
 *
 * @note this is a placeholder for when the actual sync lock handle is moved into
 * the mali_surface functionality
 * @the sync lock will be created when the mem_ref is allocated / set
 */
MALI_STATIC_INLINE mali_bool _mali_surface_create_sync_handle( struct mali_surface *surface )
{
	MALI_IGNORE(surface);
	return MALI_TRUE;
}

/**
 * @brief Locks a sync handle
 * @param surface pointer to surface for which to lock
 * @return MALI_TRUE if sync handle was locked successfully, MALI_FALSE else
 */
MALI_IMPORT mali_bool _mali_surface_lock_sync_handle( mali_surface *surface );

/**
 * @brief Unlocks a sync handle
 * @param surface pointer to surface for which to unlock
 * @return MALI_TRUE on success, MALI_FALSE else
 */
MALI_IMPORT mali_bool _mali_surface_unlock_sync_handle( mali_surface *surface );

/**
 * @brief Clears all dependencies on a surface. This function is called before a write operation if there are outstanding
 *        read operations on the surface, effectively removing the read operations.
 * @note  Currently, the function will replace the memory of the surface, effectively moving the surface.
 *        Potential future developments for this function may involve more complex solutions, but for now a simple
 *        copy does the trick for what we need to do. This function is as such not compatible with surfaces
 *        that cannot be moved in mali memory, like an EGL Pixmap.
 *        Conceptually, this function works like mali_surface_alloc_surface, except it doesn't create a new surface, it only
 *        replaces the content of the given surface.
 * @param surface The surface to be cleared of dependencies. The function will modify this surface.
 * @param deep_cow_desc Set to NULL to only reallocate without data copy, otherwise use this to handle deep cow later
 * @note  Mali will replace the entire surface when writing to it; so for deferred writes and complete overrides, the parameter
 *        @a deep_cow_desc should be NULL.
 *        The only case where we need data preservation is when doing a partial update, e.g. through a direct write (GLES mipmap
 *        updates, VG partial clear) or using writeback_dirty behaviour.
 * @return The function will return the ds_resource_handle of the now cleared-of-dependencies surface. Since that
 *         typically means replacing the dependency system module, the dependency system itself must be told that it has
 *         been replaced.  That is what this parameter is for. For external non-depsystem callers, the return parameter
 *         must be checked for not being NULL, as that means the function ran out of memory.
 * @note  Regarding Locking:
 *
 *        This function may perform a Copy on Write. That involves moving the surface around in memory, which in turn mean
 *        that the memref is replaced. That is a BIG problem as far as access goes, as that may crash code accessing the
 *        surface internals from other threads. Of course, we have the access lock for just this. But this function can
 *        not take the access lock. Because the callers are in need of holding the access lock long enough to ensure
 *        that any addrefing later happen on the proper memref object. As such, it is the responsibility of the callers
 *        to ensure that the access lock is called.
 *
 *        This function will assert that the access lock is taken.
 */
MALI_IMPORT mali_ds_resource_handle _mali_surface_clear_dependencies(mali_surface* surface, mali_surface_deep_cow_descriptor *deep_cow_desc);


/**
 * @brief Trigger a given event on this surface. This function is invoked by the driver when an event occurs. 
 *        Users of the event system should not call this directly (though you can if you want to simulate events). 
 * @param surf - The surface to trigger an event on. 
 * @param event - The event to trigger. Check out the ENUM section for details. 
 * @note  If the event callback is NULL, no actions will occur.
 **/
MALI_STATIC_INLINE void _mali_surface_trigger_event(mali_surface* surf, enum mali_surface_event event)
{
	MALI_DEBUG_ASSERT_POINTER(surf);
	MALI_DEBUG_ASSERT(event < MALI_SURFACE_EVENT_COUNT, ("Event out of range: wanted [0-%d] - found %d", MALI_SURFACE_EVENT_COUNT-1, event));
	if(surf->eventfunc[event]) surf->eventfunc[event](surf, event, surf->eventdata[event]);
}

/**
 * @brief Set up a surface event handler 
 *        NULL means no callback. Overwriting an existing callback will trigger an assert, as this is not handled. 
 * @param surf - The surface to set up callbacks on
 * @param event - The event to listen in to
 * @param callback - The function pointer to call when the event is triggered
 * @param data - A generic handle block passed into the callback functions along with the callback.  
 */
MALI_STATIC_INLINE void _mali_surface_set_event_callback(mali_surface* surf, enum mali_surface_event event, mali_surface_eventfunc callback, void* data)
{
	MALI_DEBUG_ASSERT_POINTER(surf);
	MALI_DEBUG_ASSERT(event < MALI_SURFACE_EVENT_COUNT, ("Event out of range: wanted [0-%d] - found %d", MALI_SURFACE_EVENT_COUNT-1, event));
	MALI_DEBUG_ASSERT(surf->eventfunc[event] == NULL ||  /* no callback set */
	                  callback == NULL ||  /* setting it to NULL right now */
	                  (surf->eventfunc[event] == callback), /* or callback function is the same as it already is */
					  ("Event %d already had a callback registered. There can only be one at a time right now", event));
	surf->eventfunc[event] = callback;
	surf->eventdata[event] = data;
}


#if MALI_INSTRUMENTED || defined(DEBUG)
/**
 * Writes the content of the surface to a .ppm file in the current directory.
 *
 * @param surface The surface to write to the file
 * @param filename Name of file to write the surface to
 */
void _mali_surface_fprintf( struct mali_surface *surface, const char *filename );


/**
 * Dumps the content of a mali_surface to two files. A .hex file containing the actual image data,
 * and a .hex.metadata file containing the constraints and pointer values of the image.
 * This function is used for debugging purposes only.
 *
 * @param surface The surface to dump
 * @param filename_prefix The name of the file, minus file ending.
 *  Setting this to "foo" will yield one "foo.hex" file and one "foo.metadata" file
 **/
void _mali_surface_dump( struct mali_surface *surface, const char *filename_prefix );
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MALI_SURFACE_H_ */
