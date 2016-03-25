precision mediump float;
uniform sampler2D s_texture;
uniform sampler2D s_textureSRC1;
//uniform sampler2D s_textureSRC2;
varying vec2 v_texCoord;


void main()
{
	gl_FragColor = texture2D(s_textureSRC1, v_texCoord);
} 

 

