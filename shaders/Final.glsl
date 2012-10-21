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
	fFragColor = vec3(texture2D(rasterImage, vTexCoords).r);
#endif
}
