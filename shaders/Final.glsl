// The final pass

#pragma anki start vertexShader

#pragma anki include shaders/SimpleVert.glsl

#pragma anki start fragmentShader
#pragma anki include shaders/CommonFrag.glsl

uniform lowp sampler2D rasterImage;

in vec2 vTexCoords;
layout(location = 0) out lowp vec3 fFragColor;

void main()
{
#if 1
	lowp vec3 col = texture(rasterImage, vTexCoords).rgb;
	fFragColor = col;
#else
	uvec2 msAll = texture(rasterImage, vTexCoords).rg;
	vec4 diffuseAndSpec = unpackUnorm4x8(msAll[0]);
	fFragColor = vec3(diffuseAndSpec.rgb);
#endif
}
