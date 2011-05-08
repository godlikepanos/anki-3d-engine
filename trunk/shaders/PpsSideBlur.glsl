// The final pass

#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform sampler2D tex;
in vec2 vTexCoords;
layout(location = 0) out vec3 fFragColor;


void main()
{
	fFragColor = vec3(0.0, 0.0, texture2D(tex, vTexCoords).r);
}
