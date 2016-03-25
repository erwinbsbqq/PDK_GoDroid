precision mediump float;

uniform sampler2D s_textureSRC1;
uniform sampler2D s_textureSRC2;
varying vec2 v_texCoord;


void main()
{
	vec4 texel0, texel1;
	
	texel0 = texture2D(s_textureSRC1, v_texCoord);
	texel1 = texture2D(s_textureSRC2, v_texCoord);
	
  //gl_FragColor = mix(texel0, texel1, texel0.a);
  
  gl_FragColor.r = texel0.r;
  gl_FragColor.g = texel1.r;
  
  gl_FragColor.b = 0.0;
  gl_FragColor.a = 0.5;
} 

 

