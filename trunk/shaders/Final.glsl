// The final pass

#pragma anki start vertexShader

#pragma anki include shaders/SimpleVert.glsl

#pragma anki start fragmentShader

#if 0
uniform usampler2D rasterImage;
#else
uniform sampler2D rasterImage;
#endif

in vec2 vTexCoords;
layout(location = 0) out vec3 fFragColor;

void main()
{
#if 0
	uvec2 a = texture2D(rasterImage, vTexCoords).rg;
	vec4 diffAndSpec = unpackUnorm4x8(a.x);
	fFragColor = diffAndSpec.rgb;
#else
	vec3 col = texture2D(rasterImage, vTexCoords).rgb;
	fFragColor = col;
#endif
	//fFragColor = texture2D(rasterImage, vTexCoords).rgb;
}
