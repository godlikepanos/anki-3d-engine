#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/photoshop_filters.glsl"

uniform sampler2D isFai;
uniform sampler2D ppsHdrFai;
uniform sampler2D ppsSsaoFai;

in vec2 vTexCoords;

layout(location = 0) out vec3 fColor;

//==============================================================================
vec3 grayScale(in vec3 col)
{
	float grey = (col.r + col.g + col.b) * 0.333333333; // aka: / 3.0
	return vec3(grey);
}

//==============================================================================
vec3 saturation(in vec3 col, in float factor)
{
	const vec3 lumCoeff = vec3(0.2125, 0.7154, 0.0721);

	vec3 intensity = vec3(dot(col, lumCoeff));
	return mix(intensity, col, factor);
}

//==============================================================================
void main(void)
{
	fColor = texture2D(isFai, vTexCoords).rgb;

	/*const float gamma = 0.7;
	color.r = pow(color.r, 1.0 / gamma);
	color.g = pow(color.g, 1.0 / gamma);
	color.b = pow(color.b, 1.0 / gamma);*/

#if defined(HDR_ENABLED)
	vec3 hdr = texture2D(ppsHdrFai, vTexCoords).rgb;
	fColor += hdr;
#endif

#if defined(SSAO_ENABLED)
	float ssao = texture2D(ppsSsaoFai, vTexCoords).r;
	fColor *= ssao;
#endif

	fColor = BlendHardLight(vec3(0.6, 0.62, 0.4), fColor);
}

