/*****
ALi mali implementation for video texturing (substitution of PPV) (ALiDEVEfilter)

Gernel macro for debug message

20131004 yashi 

*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#define	DEBUG_PRINTF	printf
#define	DEBUG_PRINTF

#define	DEBUG_GL_ERROR(str)	DEBUG_PRINTF("glGetError in %s : %s = %8x \n", __FUNCTION__, #str, glGetError() );
#define	DEBUG_FUNC_ERROR(func, str) DEBUG_PRINTF("%s in %s : %s = %8x \n", #func, __FUNCTION__, #str, func() );


//from EGL_helpers
#define ASSERT_EGL_EQUAL(input, expected, str) {if( (input) == (expected)) { printf(str"\n"); return EGL_FALSE;} }
#define ASSERT_EGL_NOT_EQUAL(input, expected, str)  {if( (input) != (expected)) { printf(str"\n"); return EGL_FALSE;} }

#define MALLOC( size )			_mali_sys_malloc( (size) )

/*
===================
GL	ERROR CODE --by glGlError
===================
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505

===================
FramebufferStatus CODE --by glCheckFramebufferStatus
===================
#define GL_FRAMEBUFFER_COMPLETE                      0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS         0x8CD9
#define GL_FRAMEBUFFER_UNSUPPORTED                   0x8CDD


===================
GL	ERROR CODE --by eglGlError
===================
#define EGL_SUCCESS			0x3000
#define EGL_NOT_INITIALIZED		0x3001
#define EGL_BAD_ACCESS			0x3002
#define EGL_BAD_ALLOC			0x3003
#define EGL_BAD_ATTRIBUTE		0x3004
#define EGL_BAD_CONFIG			0x3005
#define EGL_BAD_CONTEXT			0x3006
#define EGL_BAD_CURRENT_SURFACE		0x3007
#define EGL_BAD_DISPLAY			0x3008
#define EGL_BAD_MATCH			0x3009
#define EGL_BAD_NATIVE_PIXMAP		0x300A
#define EGL_BAD_NATIVE_WINDOW		0x300B
#define EGL_BAD_PARAMETER		0x300C
#define EGL_BAD_SURFACE			0x300D
#define EGL_CONTEXT_LOST		0x300E	//EGL 1.1 - IMG_power_management 

*/


#if 1

//From Mali egl

typedef void* mali_egl_image;

/**
 * @defgroup MaliEGLImageErrorCodes
 * @{
 */
#define MALI_EGL_IMAGE_SUCCESS        0x4001
#define MALI_EGL_IMAGE_BAD_IMAGE      0x4002
#define MALI_EGL_IMAGE_BAD_LOCK       0x4003
#define MALI_EGL_IMAGE_BAD_MAP        0x4004
#define MALI_EGL_IMAGE_BAD_ACCESS     0x4005
#define MALI_EGL_IMAGE_BAD_ATTRIBUTE  0x4006
#define MALI_EGL_IMAGE_IN_USE         0x4007
#define MALI_EGL_IMAGE_BAD_PARAMETER  0x4008
#define MALI_EGL_IMAGE_BAD_POINTER    0x4009
#define MALI_EGL_IMAGE_SYNC_TIMEOUT   0x4010
#define MALI_EGL_IMAGE_BAD_VERSION    0x4011
#define MALI_EGL_IMAGE_OUT_OF_MEMORY  0x4012
/** @} */

#define MALI_EGL_IMAGE_PLANE 0x00FB

#define MALI_EGL_IMAGE_PLANE_Y   0x0100
#define MALI_EGL_IMAGE_PLANE_U   0x0101
#define MALI_EGL_IMAGE_PLANE_V   0x0102
#define MALI_EGL_IMAGE_PLANE_UV  0x0103
#define MALI_EGL_IMAGE_PLANE_YUV 0x0104
#define MALI_EGL_IMAGE_PLANE_RGB MALI_EGL_IMAGE_PLANE_Y

#endif


//#include <shared/mali_egl_image.h>