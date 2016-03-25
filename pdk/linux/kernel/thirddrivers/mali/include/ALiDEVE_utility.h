/*****
ALi mali implementation for video texturing (substitution of PPV) (ALiDEVEfilter)

Utility func inculding math / Opengles (FBO / shaders ) 

20131004 yashi 

*****/


#define	ALPHA_OUTPUT_TEST

typedef enum{
	ONE_BYTE  = 0x0000,
	TWO_BYTE  = 0x0001,
	THREE_BYTE= 0x0002,
	FOUR_BYTE = 0x0003,	
	
}Data_Type;

/*
==BlendingFactorDest ==
GL_ZERO                           
GL_ONE                            
GL_SRC_COLOR                      
GL_ONE_MINUS_SRC_COLOR            
GL_SRC_ALPHA                      
GL_ONE_MINUS_SRC_ALPHA            
GL_DST_ALPHA                      
GL_ONE_MINUS_DST_ALPHA            

== BlendingFactorSrc ==
GL_ZERO
GL_ONE
GL_DST_COLOR                      
GL_ONE_MINUS_DST_COLOR            
GL_SRC_ALPHA_SATURATE             
*/
/*
typedef struct _DEVE_info_struct
{
	bmp_struct	*pInputData1;	//input data 1	
	bmp_struct	*pInputData2;	//input data 2
	bmp_struct	*pOutputData;	//output data 

	unsigned int blend_mode_src;
	unsigned int blend_mode_dst;
	float	rotate_angle;	//should be degrees;
	float	scale_factor;	
	
	float sx;	//starting position of source image;fixed 0,0
	float sy;

	float dx;	//blit position on destination (surface);
	float dy;

}DEVE_info_struct;
*/

//Each component described in ALi_DEVE_mini_egl.h
typedef struct{
	ALiEGLhandle *pEGLhandle;
	ALiFBOhandle *pFBOhandle;
	
	ALiEGLImage* pEGLimage_Src1;
	//ALiEGLImage* pEGLimage_Src2;	//not used
	ALiEGLImage* pEGLimage_Out;
	
	GLfloat *p_matrix;
	
	int disp_width;
	int disp_height;
	
	int	src_width;
	int	src_height;
	
	int input_bpp;
	
}ALiDEVEhandle;


//print log function
#if 0 
#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line);

#endif

void printShaderInfoLog(GLuint obj);
void printProgramInfoLog(GLuint obj);


//texture function

char *textFileRead(char *fn);
int textFileWrite(char *fn, char *s);

void setShaders(GLuint* uiProgram, char* pVertex, char* pFrag);
//char* load_shader(char *sFilename);

EGLBoolean create_new_FBO(GLuint *fb, GLuint *renderbuffer, int width, int height);
GLuint CreateSimpleTexture2D( GLuint tex_format, int width, int height, void* pInputData);

//math function
void matrix_op(GLfloat *p_matrix, float sx, float sy, float tx, float ty, float angle);
void denoise_kernel_setting(GLfloat *p_offset, GLfloat *p_kernel, float step_w, float step_h);

//render function
void render_quad(GLuint uiProgram, GLfloat *p_matrix, int textId, float spx, float spy);
void render_quad_yuv420(GLuint uiProgram, GLfloat *p_matrix, int textId1,  int textId2);