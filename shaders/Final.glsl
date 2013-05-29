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
	lowp vec3 col = textureLod(rasterImage, vTexCoords, 0.0).rgb;
	fFragColor = col;
}
