// The final pass

#pragma anki start vertexShader

#pragma anki include shaders/SimpleVert.glsl

#pragma anki start fragmentShader

uniform sampler2D rasterImage;

in vec2 vTexCoords;
layout(location = 0) out vec3 fFragColor;

void main()
{
#if 1
	vec3 col = texture2D(rasterImage, vTexCoords).rgb;
	fFragColor = col;
#else
	uvec2 msAll = texture2D(rasterImage, vTexCoords).rg;
	vec4 diffuseAndSpec = unpackUnorm4x8(msAll[0]);
	fFragColor = vec3(diffuseAndSpec.rgb);
#endif
}
