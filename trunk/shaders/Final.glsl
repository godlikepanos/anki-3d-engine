// The final pass

#pragma anki start vertexShader

#pragma anki include shaders/SimpleVert.glsl

#pragma anki start fragmentShader
#pragma anki include shaders/CommonFrag.glsl
#pragma anki include shaders/Pack.glsl

uniform lowp sampler2D rasterImage;

in vec2 vTexCoords;
layout(location = 0) out lowp vec3 fFragColor;

void main()
{
	lowp vec3 col = texture(rasterImage, vTexCoords).rgb;
	//lowp vec3 col = vec3(readAndUnpackNormal(rasterImage, vTexCoords) * 0.5 + 0.5);
	fFragColor = col;
}
