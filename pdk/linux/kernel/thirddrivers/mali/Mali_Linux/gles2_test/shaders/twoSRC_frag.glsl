precision mediump float;
uniform sampler2D s_texture;
uniform sampler2D s_textureSRC1;
uniform sampler2D s_textureSRC2;
varying vec2 v_texCoord;


void main()
{
	vec4 dst;
	vec4 src1 = texture2D(s_textureSRC1, v_texCoord);
	vec4 src2 = texture2D(s_textureSRC2, v_texCoord);	
	src1.a = 0.5;
	dst.r = src1.r*src1.a + src2.r*(1.0-src1.a);
	dst.g = src1.g*src1.a + src2.g*(1.0-src1.a);
	dst.b = src1.b*src1.a + src2.b*(1.0-src1.a);
	dst.a = src1.a;	
	gl_FragColor = vec4(dst.b,dst.g,dst.r,dst.a);
} 

 

