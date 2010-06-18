#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform sampler2D fai;

varying vec2 texCoords;



void main()
{
	vec4 _color = texture2D( fai, texCoords );
	if( _color.a == 0.0 ) discard;

	gl_FragData[0].rgb = _color.rgb;
}
