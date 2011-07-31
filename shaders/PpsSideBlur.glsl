// The final pass

#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

in vec2 vTexCoords;

uniform sampler2D tex;
uniform float factor = 1.0;

layout(location = 0) out vec3 fFragColor;


void main()
{
	float f = texture2D(tex, vTexCoords).r;
	fFragColor = vec3(0.0, 0.0, f * factor);
}
