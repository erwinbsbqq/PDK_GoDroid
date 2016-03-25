/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2001-2002, 2007-2010 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */
 
 
 precision mediump float;
/*
#if OpenGL_ES
#define	OPENGL_MEDIUMP	mediump 
#else
#define	OPENGL_MEDIUMP	mediump
#endif
*/


uniform sampler2D s_texture;
varying vec2 v_texCoord;
uniform  vec4 v_PaintColor;


void main()
{
        	
	
	gl_FragData[0] = texture2D(s_texture, v_texCoord );
	//gl_FragData[0] = vec4(0, 0.8, 0.8, 1.0);
	gl_FragData[0].rgb = gl_FragData[0].rgb * v_PaintColor.rgb;
  
   	
}
