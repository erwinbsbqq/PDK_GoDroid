precision mediump float;

uniform sampler2D s_texture;
varying vec2 v_texCoord;

void main()
{
	gl_FragColor = texture2D(s_texture, v_texCoord);
	//gl_FragColor.r = 0.3;
  //gl_FragColor.g = 0.1;
  
  //gl_FragColor.b = 0.0;
  //gl_FragColor.a = 0.5;
} 

 

