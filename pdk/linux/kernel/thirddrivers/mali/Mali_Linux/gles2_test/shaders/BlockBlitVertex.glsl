/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009 - 2011 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */


attribute vec4 a_position;
attribute vec2 a_texCoord;
uniform  mat4 a_matrix;


varying vec2 v_texCoord;
void main()
{
	
	gl_Position = vec4(a_matrix * a_position);
	
	
	v_texCoord = a_texCoord;
}