#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

uniform sampler2D fai; ///< Its the IS FAI
uniform float exposure;

in vec2 vTexCoords;

layout(location = 0) out vec3 fColor;

void main()
{
	vec3 color = texture2D(fai, vTexCoords).rgb;
	float luminance = dot(vec3(0.30, 0.59, 0.11), color);
	const float brightMax = 4.0;
	float yd = exposure * (exposure / brightMax + 1.0) /
		(exposure + 1.0) * luminance;
	color *= yd;
	fColor = color;
}
