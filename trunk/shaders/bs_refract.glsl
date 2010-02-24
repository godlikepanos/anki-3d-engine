//
#pragma anki vertShaderBegins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki fragShaderBegins

#pragma anki uniform fai 0
uniform sampler2D fai;

varying vec2 tex_coords;



void main()
{
	vec4 _color = texture2D( fai, tex_coords );
	if( _color.a == 0.0 ) discard;

	gl_FragData[0].rgb = _color.rgb;
}
