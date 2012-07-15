// The final pass

#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

#if 1
uniform usampler2D rasterImage;
#else
uniform sampler2D rasterImage;
#endif

in vec2 vTexCoords;
layout(location = 0) out vec3 fFragColor;

void main()
{
#if 1
	uvec2 a = texture2D(rasterImage, vTexCoords).rg;
	vec4 diffAndSpec = unpackUnorm4x8(a.x);
	fFragColor = diffAndSpec.rgb;
#else
	fFragColor = texture2D(rasterImage, vTexCoords).rgb;
#endif
	//fFragColor = texture2D(rasterImage, vTexCoords).rgb;
}
