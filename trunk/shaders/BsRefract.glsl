#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform sampler2D fai;

varying vec2 texCoords;

void main()
{
	gl_FragData[0] = texture2D(fai, texCoords);
}
