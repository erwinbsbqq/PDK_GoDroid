#define Sharping 0
#define Smoothing 1

#define OpenGL_ES 0

// maximum size supported by this shader
const int MaxKernelSize = 25;

// array of offsets for accessing the base image
//uniform vec2 Offset[MaxKernelSize];

// size of kernel (width * height) for this execution

// final scaling value

// image to be convolved
uniform sampler2D s_texture;
varying vec2 v_texCoord;

void main()
{
	int i;
	vec4 sum = vec4(0.0);
#if OpenGL_ES
        const mediump float step_w = 2.0/512.0;
        const mediump float step_h = 2.0/512.0;
#else
        const float step_w = 2.0/512.0;
        const float step_h = 2.0/512.0;
#endif


#if OpenGL_ES
	mediump vec2 offset[25];
        mediump float kernel[25];
#else
	vec2 offset[25];
        float kernel[25];
#endif
	vec4 ScaleFactor = vec4(0.003663);
	int KernelSize = 25;
	
        offset[0] = vec2(-2*step_w,  2*step_h);
        offset[1] = vec2(-2*step_w,  1*step_h);
        offset[2] = vec2(-2*step_w,  0*step_h);
        offset[3] = vec2(-2*step_w, -1*step_h);
        offset[4] = vec2(-2*step_w, -2*step_h);

        offset[5] = vec2(-1*step_w,  2*step_h);
        offset[6] = vec2(-1*step_w,  1*step_h);
        offset[7] = vec2(-1*step_w,  0*step_h);
        offset[8] = vec2(-1*step_w, -1*step_h);
        offset[9] = vec2(-1*step_w, -2*step_h);

        offset[10]= vec2( 0*step_w,  2*step_h);
        offset[11]= vec2( 0*step_w,  1*step_h);
        offset[12]= vec2( 0*step_w,  0*step_h);
        offset[13]= vec2( 0*step_w, -1*step_h);
        offset[14]= vec2( 0*step_w, -2*step_h);

        offset[15]= vec2( 1*step_w,  2*step_h);
        offset[16]= vec2( 1*step_w,  1*step_h);
        offset[17]= vec2( 1*step_w,  0*step_h);
        offset[18]= vec2( 1*step_w, -1*step_h);
        offset[19]= vec2( 1*step_w, -2*step_h);
        
        offset[20]= vec2( 2*step_w,  2*step_h);
        offset[21]= vec2( 2*step_w,  1*step_h);
        offset[22]= vec2( 2*step_w,  0*step_h);
        offset[23]= vec2( 2*step_w, -1*step_h);
        offset[24]= vec2( 2*step_w, -2*step_h);

        kernel[0] = kernel[4] = kernel[20]= kernel[24]= 1.0;
        kernel[5] = kernel[15]= kernel[1] = kernel[21]= kernel[3] = kernel[23]= kernel[9] = kernel[19]= 4.0;
        kernel[10]= kernel[14]= kernel[2] = kernel[22]= 7.0;
        kernel[6] = kernel[16]= kernel[8] = kernel[18]= 16.0;
        kernel[11]= kernel[7] = kernel[17]= kernel[13]= 26.0;
        kernel[12]= 41.0;        

	for (i = 0; i < KernelSize; i++)
	{
        	vec4 tmp = texture2D(s_texture, v_texCoord + offset[i]);
        	sum += tmp * kernel[i];
	}
    	gl_FragColor = ScaleFactor * sum;
	//gl_FragColor = vec4(0.1,0.1,0.9,1.0);
	//gl_FragColor = texture2D(s_texture, v_texCoord);
}

 

