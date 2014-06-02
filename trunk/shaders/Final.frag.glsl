// The final pass

#pragma anki type frag

#define DEFAULT_FLOAT_PRECISION lowp
#pragma anki include shaders/Common.glsl
#pragma anki include shaders/Pack.glsl

layout(binding = 0) uniform sampler2D uRasterImage;

layout(location = 0) in highp vec2 inTexCoords;
layout(location = 0) out vec3 outFragColor;

void main()
{
	vec3 col = textureRt(uRasterImage, inTexCoords).rgb;
	//vec3 col = vec3((2.0 * 0.5) / (500.0 + 0.5 - textureRt(uRasterImage, inTexCoords).r * (500.0 - 0.5)));
	outFragColor = col;
}
