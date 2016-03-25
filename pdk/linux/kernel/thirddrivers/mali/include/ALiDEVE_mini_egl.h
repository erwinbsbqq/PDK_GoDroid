/*****
ALi mali implementation for video texturing (substitution of PPV) (ALiDEVEfilter)

Mini egl layer 

20131004 yashi 

*****/

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/fbdev_window.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


//enable extension format
/* EGLImage */
#define EXTENSION_EGL_IMAGE_OES_ENABLE                    1
/* EXT_texture_format_BGRA8888 */
#define EXTENSION_BGRA8888_ENABLE                         1


typedef struct
{
	fbdev_window *fbwin;
	
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	EGLConfig* configs;
	EGLConfig config;
}ALiEGLhandle;


typedef struct
{
	GLuint myfbo, renderbuffer, fb_current;
	GLuint uiProgram;
}ALiFBOhandle;

typedef struct
{
	unsigned char *pBuf;
	EGLImageKHR image_khr;
	GLuint Tex_id;
	EGLNativePixmapType pPixmap;
	
	int	width;
	int	height;
	int bpp;
}ALiEGLImage;


EGLBoolean ALi_egl_init(ALiEGLhandle *pHandle);

//From Mali GENC-009827-2-0 --EGL_YUV_IMAGE_SUPPORT
EGLNativePixmapType ALi_create_pixmap(int width, int height, int red, int green, int blue, int alpha, int luminance, unsigned char *pData);
EGLNativePixmapType ALi_assign_buf_pixmap(EGLNativePixmapType pInputPixmap, int width, int height, int red, int green, int blue, int alpha, int luminance, unsigned char *pData);


GLuint ALi_create_texture( EGLImageKHR egl_image );

GLuint ALi_create_egl_image( ALiEGLImage* pEGLimage, unsigned char *pBuf, int width, int height, int bpp, ALiEGLhandle *pEGLhandle);
GLuint ALi_copy_data_egl_image( ALiEGLImage* pEGLimage, unsigned char *pBuf, int width, int height, int bpp, ALiEGLhandle *pEGLhandle);

GLuint ALi_egl_fill_image( EGLImageKHR image_khr, EGLint *pAttribs, int disp_width, int disp_height, int input_bpp, unsigned char *pColor);


