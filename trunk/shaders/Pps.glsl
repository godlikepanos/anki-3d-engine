#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/photoshop_filters.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

uniform sampler2D msDepthFai;
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
vec3 gammaCorrection(in float gamma, in vec3 col)
{
	return pow(col, vec3(1.0 / gamma));
}

//==============================================================================
vec3 gammaCorrectionRgb(in vec3 gamma, in vec3 col)
{
	return pow(col, 1.0 / gamma);
}

//==============================================================================
void main(void)
{
	fColor = texture2D(isFai, vTexCoords).rgb;

#if defined(HDR_ENABLED)
	vec3 hdr = texture2D(ppsHdrFai, vTexCoords).rgb;
	fColor += hdr;
#endif

#if defined(SSAO_ENABLED)
	float ssao = texture2D(ppsSsaoFai, vTexCoords).r;
	fColor *= ssao;
#endif

	/*float fog = 1.0 - readFromTextureAndLinearizeDepth(msDepthFai, 
		vTexCoords, 0.1, 10.0);
	fColor *= ;*/

	//fColor = BlendHardLight(vec3(0.6, 0.62, 0.4), fColor);
	fColor = gammaCorrectionRgb(vec3(0.9, 0.92, 0.75), fColor);
}

