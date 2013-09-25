// The final pass

#pragma anki start vertexShader

#pragma anki include shaders/SimpleVert.glsl

#pragma anki start fragmentShader
#define DEFAULT_FLOAT_PRECISION lowp
#pragma anki include shaders/Common.glsl
#pragma anki include shaders/Pack.glsl

uniform sampler2D rasterImage;

in highp vec2 vTexCoords;
layout(location = 0) out vec3 fFragColor;

void main()
{
	vec3 col = textureFai(rasterImage, vTexCoords).rgb;
	//lowp vec3 col = vec3(readAndUnpackNormal(rasterImage, vTexCoords) * 0.5 + 0.5);
	fFragColor = col;
}
