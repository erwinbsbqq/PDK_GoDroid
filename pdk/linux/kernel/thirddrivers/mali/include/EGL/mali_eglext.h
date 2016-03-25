/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2008-2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _EGLEXT_H_
#define _EGLEXT_H_

#include <EGL/eglext.h>

#if defined(__SYMBIAN32__)
/**
 * Defined in internal\include\khronos\GLES2\eglext.h
 * We are using \epoc32\include\GLES2\gl2ext.h
 */
#ifndef EGL_KHR_lock_surface2
#define EGL_KHR_lock_surface2 1
#define EGL_BITMAP_PIXEL_SIZE_KHR		        0x3110
#endif
#ifndef EGL_SYNC_FENCE_KHR
#define EGL_SYNC_FENCE_KHR                      0x30F9
#endif
#ifndef EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR
#define EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR    0x30F0
#endif
#ifndef EGL_SYNC_CONDITION_KHR
#define EGL_SYNC_CONDITION_KHR			        0x30F8
#endif
#endif  /* __SYMBIAN32__ */

#endif /* _EGLEXT_H_ */
